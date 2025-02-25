// SPDX-License-Identifier: ISC
/* Copyright (C) 2020 MediaTek Inc. */

#include <linux/firmware.h>
#include <linux/fs.h>
#include "besra.h"
#include "mcu.h"
#include "mac.h"
#include "eeprom.h"

struct besra_patch_hdr {
	char build_date[16];
	char platform[4];
	__be32 hw_sw_ver;
	__be32 patch_ver;
	__be16 checksum;
	u16 reserved;
	struct {
		__be32 patch_ver;
		__be32 subsys;
		__be32 feature;
		__be32 n_region;
		__be32 crc;
		u32 reserved[11];
	} desc;
} __packed;

struct besra_patch_sec {
	__be32 type;
	__be32 offs;
	__be32 size;
	union {
		__be32 spec[13];
		struct {
			__be32 addr;
			__be32 len;
			__be32 sec_key_idx;
			__be32 align_len;
			u32 reserved[9];
		} info;
	};
} __packed;

struct besra_fw_trailer {
	u8 chip_id;
	u8 eco_code;
	u8 n_region;
	u8 format_ver;
	u8 format_flag;
	u8 reserved[2];
	char fw_ver[10];
	char build_date[15];
	u32 crc;
} __packed;

struct besra_fw_region {
	__le32 decomp_crc;
	__le32 decomp_len;
	__le32 decomp_blk_sz;
	u8 reserved[4];
	__le32 addr;
	__le32 len;
	u8 feature_set;
	u8 reserved1[15];
} __packed;

#define MCU_PATCH_ADDRESS		0x200000

#define HE_PHY(p, c)			u8_get_bits(c, IEEE80211_HE_PHY_##p)
#define HE_MAC(m, c)			u8_get_bits(c, IEEE80211_HE_MAC_##m)

static u8
besra_mcu_get_sta_nss(u16 mcs_map)
{
	u8 nss;

	for (nss = 8; nss > 0; nss--) {
		u8 nss_mcs = (mcs_map >> (2 * (nss - 1))) & 3;

		if (nss_mcs != IEEE80211_VHT_MCS_NOT_SUPPORTED)
			break;
	}

	return nss - 1;
}

static void
besra_mcu_set_sta_he_mcs(struct ieee80211_sta *sta, __le16 *he_mcs,
			  u16 mcs_map)
{
	struct besra_sta *msta = (struct besra_sta *)sta->drv_priv;
	enum nl80211_band band = msta->vif->phy->mt76->chandef.chan->band;
	const u16 *mask = msta->vif->bitrate_mask.control[band].he_mcs;
	int nss, max_nss = sta->rx_nss > 3 ? 4 : sta->rx_nss;

	for (nss = 0; nss < max_nss; nss++) {
		int mcs;

		switch ((mcs_map >> (2 * nss)) & 0x3) {
		case IEEE80211_HE_MCS_SUPPORT_0_11:
			mcs = GENMASK(11, 0);
			break;
		case IEEE80211_HE_MCS_SUPPORT_0_9:
			mcs = GENMASK(9, 0);
			break;
		case IEEE80211_HE_MCS_SUPPORT_0_7:
			mcs = GENMASK(7, 0);
			break;
		default:
			mcs = 0;
		}

		mcs = mcs ? fls(mcs & mask[nss]) - 1 : -1;

		switch (mcs) {
		case 0 ... 7:
			mcs = IEEE80211_HE_MCS_SUPPORT_0_7;
			break;
		case 8 ... 9:
			mcs = IEEE80211_HE_MCS_SUPPORT_0_9;
			break;
		case 10 ... 11:
			mcs = IEEE80211_HE_MCS_SUPPORT_0_11;
			break;
		default:
			mcs = IEEE80211_HE_MCS_NOT_SUPPORTED;
			break;
		}
		mcs_map &= ~(0x3 << (nss * 2));
		mcs_map |= mcs << (nss * 2);
	}

	*he_mcs = cpu_to_le16(mcs_map);
}

static void
besra_mcu_set_sta_vht_mcs(struct ieee80211_sta *sta, __le16 *vht_mcs,
			   const u16 *mask)
{
	u16 mcs_map = le16_to_cpu(sta->vht_cap.vht_mcs.rx_mcs_map);
	int nss, max_nss = sta->rx_nss > 3 ? 4 : sta->rx_nss;
	u16 mcs;

	for (nss = 0; nss < max_nss; nss++, mcs_map >>= 2) {
		switch (mcs_map & 0x3) {
		case IEEE80211_VHT_MCS_SUPPORT_0_9:
			mcs = GENMASK(9, 0);
			break;
		case IEEE80211_VHT_MCS_SUPPORT_0_8:
			mcs = GENMASK(8, 0);
			break;
		case IEEE80211_VHT_MCS_SUPPORT_0_7:
			mcs = GENMASK(7, 0);
			break;
		default:
			mcs = 0;
		}

		vht_mcs[nss] = cpu_to_le16(mcs & mask[nss]);
	}
}

static void
besra_mcu_set_sta_ht_mcs(struct ieee80211_sta *sta, u8 *ht_mcs,
			  const u8 *mask)
{
	int nss, max_nss = sta->rx_nss > 3 ? 4 : sta->rx_nss;

	for (nss = 0; nss < max_nss; nss++)
		ht_mcs[nss] = sta->ht_cap.mcs.rx_mask[nss] & mask[nss];
}

static int
besra_mcu_parse_response(struct mt76_dev *mdev, int cmd,
			  struct sk_buff *skb, int seq)
{
	struct besra_mcu_rxd *rxd;
	struct besra_mcu_uni_event *event;
	int mcu_cmd = FIELD_GET(__MCU_CMD_FIELD_ID, cmd);
	int ret = 0;

	if (!skb) {
		dev_err(mdev->dev, "Message %08x (seq %d) timeout\n",
			cmd, seq);
		return -ETIMEDOUT;
	}

	rxd = (struct besra_mcu_rxd *)skb->data;
	if (seq != rxd->seq)
		return -EAGAIN;

	if (cmd == MCU_CMD(PATCH_SEM_CONTROL)) {
		skb_pull(skb, sizeof(*rxd) - 4);
		ret = *skb->data;
	} else if (cmd == MCU_EXT_CMD(THERMAL_CTRL)) {
		skb_pull(skb, sizeof(*rxd) + 4);
		ret = le32_to_cpu(*(__le32 *)skb->data);
	} else if ((rxd->option & MCU_UNI_CMD_EVENT) &&
		    rxd->eid == MCU_UNI_EVENT_RESULT) {
		skb_pull(skb, sizeof(*rxd));
		event = (struct besra_mcu_uni_event *)skb->data;
		ret = le32_to_cpu(event->status);
		/* skip invalid event */
		if (mcu_cmd != event->cid)
			ret = -EAGAIN;
	} else {
		skb_pull(skb, sizeof(struct besra_mcu_rxd));
	}

	return ret;
}

static int
besra_mcu_send_message(struct mt76_dev *mdev, struct sk_buff *skb,
			int cmd, int *wait_seq)
{
	struct besra_dev *dev = container_of(mdev, struct besra_dev, mt76);
	int txd_len, mcu_cmd = FIELD_GET(__MCU_CMD_FIELD_ID, cmd);
	struct besra_uni_txd *uni_txd;
	struct besra_mcu_txd *mcu_txd;
	enum mt76_mcuq_id qid;
	__le32 *txd;
	u32 val;
	u8 seq;

	/* TODO: make dynamic based on msg type */
	mdev->mcu.timeout = 20 * HZ;

	seq = ++dev->mt76.mcu.msg_seq & 0xf;
	if (!seq)
		seq = ++dev->mt76.mcu.msg_seq & 0xf;

	if (cmd == MCU_CMD(FW_SCATTER)) {
		qid = MT_MCUQ_FWDL;
		goto exit;
	}

	txd_len = cmd & __MCU_CMD_FIELD_UNI ? sizeof(*uni_txd) : sizeof(*mcu_txd);
	txd = (__le32 *)skb_push(skb, txd_len);
	if (test_bit(MT76_STATE_MCU_RUNNING, &dev->mphy.state))
		qid = MT_MCUQ_WA;
	else
		qid = MT_MCUQ_WM;

	val = FIELD_PREP(MT_TXD0_TX_BYTES, skb->len) |
	      FIELD_PREP(MT_TXD0_PKT_FMT, MT_TX_TYPE_CMD) |
	      FIELD_PREP(MT_TXD0_Q_IDX, MT_TX_MCU_PORT_RX_Q0);
	txd[0] = cpu_to_le32(val);

	val = FIELD_PREP(MT_TXD1_HDR_FORMAT, MT_HDR_FORMAT_CMD);
	txd[1] = cpu_to_le32(val);

	if (cmd & __MCU_CMD_FIELD_UNI) {
		uni_txd = (struct besra_uni_txd *)txd;
		uni_txd->len = cpu_to_le16(skb->len - sizeof(uni_txd->txd));
		uni_txd->cid = cpu_to_le16(mcu_cmd);
		uni_txd->s2d_index = MCU_S2D_H2CN;
		uni_txd->pkt_type = MCU_PKT_ID;
		uni_txd->seq = seq;

		if (cmd & __MCU_CMD_FIELD_QUERY)
			uni_txd->option = MCU_CMD_UNI_QUERY_ACK;
		else
			uni_txd->option = MCU_CMD_UNI_EXT_ACK;

		if ((cmd & __MCU_CMD_FIELD_WA) && (cmd & __MCU_CMD_FIELD_WM))
			uni_txd->s2d_index = MCU_S2D_H2CN;
		else if (cmd & __MCU_CMD_FIELD_WA)
			uni_txd->s2d_index = MCU_S2D_H2C;
		else if (cmd & __MCU_CMD_FIELD_WM)
			uni_txd->s2d_index = MCU_S2D_H2N;

		goto exit;
	}

	mcu_txd = (struct besra_mcu_txd *)txd;
	mcu_txd->len = cpu_to_le16(skb->len - sizeof(mcu_txd->txd));
	mcu_txd->pq_id = cpu_to_le16(MCU_PQ_ID(MT_TX_PORT_IDX_MCU,
					       MT_TX_MCU_PORT_RX_Q0));
	mcu_txd->pkt_type = MCU_PKT_ID;
	mcu_txd->seq = seq;

	mcu_txd->cid = FIELD_GET(__MCU_CMD_FIELD_ID, cmd);
	mcu_txd->set_query = MCU_Q_NA;
	mcu_txd->ext_cid = FIELD_GET(__MCU_CMD_FIELD_EXT_ID, cmd);
	if (mcu_txd->ext_cid) {
		mcu_txd->ext_cid_ack = 1;

		/* do not use Q_SET for efuse */
		if (cmd & __MCU_CMD_FIELD_QUERY)
			mcu_txd->set_query = MCU_Q_QUERY;
		else
			mcu_txd->set_query = MCU_Q_SET;
	}

	if (cmd & __MCU_CMD_FIELD_WA)
		mcu_txd->s2d_index = MCU_S2D_H2C;
	else
		mcu_txd->s2d_index = MCU_S2D_H2N;

exit:
	if (wait_seq)
		*wait_seq = seq;

	return mt76_tx_queue_skb_raw(dev, mdev->q_mcu[qid], skb, 0);
}

int besra_mcu_wa_cmd(struct besra_dev *dev, int cmd, u32 a1, u32 a2, u32 a3)
{
	struct {
		__le32 args[3];
	} req = {
		.args = {
			cpu_to_le32(a1),
			cpu_to_le32(a2),
			cpu_to_le32(a3),
		},
	};

	return mt76_mcu_send_msg(&dev->mt76, cmd, &req, sizeof(req), false);
}

static void
besra_mcu_csa_finish(void *priv, u8 *mac, struct ieee80211_vif *vif)
{
	if (vif->csa_active)
		ieee80211_csa_finish(vif);
}

static void
besra_mcu_rx_thermal_notify(struct besra_dev *dev, struct sk_buff *skb)
{
	struct mt76_phy *mphy = &dev->mt76.phy;
	struct besra_mcu_thermal_notify *t;
	struct besra_phy *phy;

	t = (struct besra_mcu_thermal_notify *)skb->data;
	if (t->ctrl.ctrl_id != THERMAL_PROTECT_ENABLE)
		return;

	mphy = mt76_dev_phy_by_band(&dev->mt76, t->ctrl.ctrl_id);

	phy = (struct besra_phy *)mphy->priv;
	phy->throttle_state = t->ctrl.duty.duty_cycle;
}

static void
besra_mcu_rx_radar_detected(struct besra_dev *dev, struct sk_buff *skb)
{
	struct mt76_phy *mphy = &dev->mt76.phy;
	struct besra_mcu_rdd_report *r;

	r = (struct besra_mcu_rdd_report *)skb->data;

	mphy = mt76_dev_phy_by_band(&dev->mt76, r->band_idx);

	if (r->band_idx == MT_RX_SEL2)
		cfg80211_background_radar_event(mphy->hw->wiphy,
						&dev->rdd2_chandef,
						GFP_ATOMIC);
	else
		ieee80211_radar_detected(mphy->hw);
	dev->hw_pattern++;
}

static void
besra_mcu_rx_log_message(struct besra_dev *dev, struct sk_buff *skb)
{
#define UNI_EVENT_FW_LOG_FORMAT 0
	struct besra_mcu_rxd *rxd = (struct besra_mcu_rxd *)skb->data;
	const char *data = (char *)&rxd[1] + 4, *type;
	struct tlv *tlv = (struct tlv *)data;
	int len;

	if (le16_to_cpu(tlv->tag) != UNI_EVENT_FW_LOG_FORMAT)
		return;

	data += sizeof(*tlv) + 4;
	len = le16_to_cpu(tlv->len) - sizeof(*tlv) - 4;

	switch (rxd->s2d_index) {
	case 0:
		if (besra_debugfs_rx_log(dev, data, len))
			return;

		type = "WM";
		break;
	case 2:
		type = "WA";
		break;
	default:
		type = "unknown";
		break;
	}

	wiphy_info(mt76_hw(dev)->wiphy, "%s: %.*s", type, len, data);
}

static void
besra_mcu_cca_finish(void *priv, u8 *mac, struct ieee80211_vif *vif)
{
	if (!vif->color_change_active)
		return;

	ieee80211_color_change_finish(vif);
}

static void
besra_mcu_ie_countdown(struct besra_dev *dev, struct sk_buff *skb)
{
#define UNI_EVENT_IE_COUNTDOWN_CSA 0
#define UNI_EVENT_IE_COUNTDOWN_BCC 1
	struct header {
		u8 band;
		u8 rsv[3];
	};
	struct mt76_phy *mphy = &dev->mt76.phy;
	struct besra_mcu_rxd *rxd = (struct besra_mcu_rxd *)skb->data;
	const char *data = (char *)&rxd[1], *tail;
	struct header *hdr = (struct header *) data;
	struct tlv *tlv = (struct tlv *)(data + 4);
	int len;

	if ((hdr->band && !dev->phy.band_idx) && dev->mt76.phy2)
		mphy = dev->mt76.phy2;

	tail = skb->data + le16_to_cpu(rxd->len);
	while (data + sizeof(struct tlv) < tail && le16_to_cpu(tlv->len)) {
		switch (le16_to_cpu(tlv->tag)) {
		case UNI_EVENT_IE_COUNTDOWN_CSA:
			ieee80211_iterate_active_interfaces_atomic(mphy->hw,
				IEEE80211_IFACE_ITER_RESUME_ALL,
				besra_mcu_csa_finish, mphy->hw);
			break;
		case UNI_EVENT_IE_COUNTDOWN_BCC:
			ieee80211_iterate_active_interfaces_atomic(mphy->hw,
				IEEE80211_IFACE_ITER_RESUME_ALL,
				besra_mcu_cca_finish, mphy->hw);
			break;
		}

		data += le16_to_cpu(tlv->len);
		tlv = (struct tlv *)data;
	}
}

#if 0
static void
besra_mcu_rx_ext_event(struct besra_dev *dev, struct sk_buff *skb)
{
	struct besra_mcu_rxd *rxd = (struct besra_mcu_rxd *)skb->data;

	switch (rxd->ext_eid) {
	case MCU_EXT_EVENT_THERMAL_PROTECT:
		besra_mcu_rx_thermal_notify(dev, skb);
		break;
	case MCU_EXT_EVENT_RDD_REPORT:
		besra_mcu_rx_radar_detected(dev, skb);
		break;
	case MCU_EXT_EVENT_CSA_NOTIFY:
		besra_mcu_rx_csa_notify(dev, skb);
		break;
	case MCU_EXT_EVENT_BCC_NOTIFY:
		ieee80211_iterate_active_interfaces_atomic(dev->mt76.hw,
				IEEE80211_IFACE_ITER_RESUME_ALL,
				besra_mcu_cca_finish, dev);
		break;
	default:
		break;
	}
}

static void
besra_mcu_rx_unsolicited_event(struct besra_dev *dev, struct sk_buff *skb)
{
	struct besra_mcu_rxd *rxd = (struct besra_mcu_rxd *)skb->data;

	switch (rxd->eid) {
	case MCU_EVENT_EXT:
		besra_mcu_rx_ext_event(dev, skb);
		break;
	default:
		break;
	}
	dev_kfree_skb(skb);
}
#endif

static void
besra_mcu_uni_rx_unsolicited_event(struct besra_dev *dev, struct sk_buff *skb)
{
	struct besra_mcu_rxd *rxd = (struct besra_mcu_rxd *)skb->data;

	switch (rxd->eid) {
	case MCU_UNI_EVENT_FW_LOG_2_HOST:
		besra_mcu_rx_log_message(dev, skb);
		break;
	case MCU_UNI_EVENT_IE_COUNTDOWN:
		besra_mcu_ie_countdown(dev, skb);
		break;
	default:
		break;
	}
	dev_kfree_skb(skb);
}

void besra_mcu_rx_event(struct besra_dev *dev, struct sk_buff *skb)
{
	struct besra_mcu_rxd *rxd = (struct besra_mcu_rxd *)skb->data;

	if (rxd->option & MCU_UNI_CMD_UNSOLICITED_EVENT) {
		besra_mcu_uni_rx_unsolicited_event(dev, skb);
		return;
	}

	/* TODO: to be changed to uni cmd */
	/* if (!(rxd->option & MCU_UNI_CMD_UNI_EVENT) && */
	/*     (rxd->ext_eid == MCU_EXT_EVENT_THERMAL_PROTECT || */
	/*      rxd->ext_eid == MCU_EXT_EVENT_ASSERT_DUMP || */
	/*      rxd->ext_eid == MCU_EXT_EVENT_PS_SYNC || */
	/*      rxd->ext_eid == MCU_EXT_EVENT_BCC_NOTIFY || */
	/*      !rxd->seq)) { */
	/* 		besra_mcu_rx_unsolicited_event(dev, skb); */
	/* 		return; */
	/* } */

	mt76_mcu_rx_event(&dev->mt76, skb);
}

static struct tlv *
besra_mcu_add_uni_tlv(struct sk_buff *skb, int tag, int len)
{
	struct tlv *ptlv, tlv = {
		.tag = cpu_to_le16(tag),
		.len = cpu_to_le16(len),
	};

	ptlv = skb_put(skb, len);
	memcpy(ptlv, &tlv, sizeof(tlv));

	return ptlv;
}

/** bss info **/
struct besra_he_obss_narrow_bw_ru_data {
	bool tolerated;
};

static inline u8 besra_get_band(enum nl80211_band band)
{
	static const u8 convert_to_fw[] = {
		[NL80211_BAND_2GHZ] = CMD_BAND_24G,
		[NL80211_BAND_5GHZ] = CMD_BAND_5G,
		[NL80211_BAND_6GHZ] = CMD_BAND_6G,
	};

	if (band >= ARRAY_SIZE(convert_to_fw))
		return 0;

	return convert_to_fw[band];
}

static void
besra_mcu_bss_rfch_tlv(struct sk_buff *skb, struct ieee80211_vif *vif,
			struct besra_phy *phy)
{
	struct cfg80211_chan_def *chandef = &phy->mt76->chandef;
	struct bss_rlm_tlv *ch;
	struct tlv *tlv;
	int freq1 = chandef->center_freq1;

	tlv = besra_mcu_add_uni_tlv(skb, UNI_BSS_INFO_RLM, sizeof(*ch));

	ch = (struct bss_rlm_tlv *)tlv;
	ch->control_channel = chandef->chan->hw_value;
	ch->center_chan = ieee80211_frequency_to_channel(freq1);
	ch->bw = mt76_connac_chan_bw(chandef);
	ch->tx_streams = hweight8(phy->mt76->antenna_mask);
	ch->rx_streams = hweight8(phy->mt76->antenna_mask);
	ch->band = besra_get_band(chandef->chan->band);

	if (chandef->width == NL80211_CHAN_WIDTH_80P80) {
		int freq2 = chandef->center_freq2;

		ch->center_chan2 = ieee80211_frequency_to_channel(freq2);
	}
}

static void
besra_mcu_bss_ra_tlv(struct sk_buff *skb, struct ieee80211_vif *vif,
		      struct besra_phy *phy)
{
	struct bss_ra_tlv *ra;
	struct tlv *tlv;

	tlv = besra_mcu_add_uni_tlv(skb, UNI_BSS_INFO_RA, sizeof(*ra));

	ra = (struct bss_ra_tlv *)tlv;
	ra->short_preamble = true;
}

static void
besra_mcu_bss_he_tlv(struct sk_buff *skb, struct ieee80211_vif *vif,
		      struct besra_phy *phy)
{
#define DEFAULT_HE_PE_DURATION		4
#define DEFAULT_HE_DURATION_RTS_THRES	1023
	const struct ieee80211_sta_he_cap *cap;
	struct bss_info_uni_he *he;
	struct tlv *tlv;

	cap = mt76_connac_get_he_phy_cap(phy->mt76, vif);

	tlv = besra_mcu_add_uni_tlv(skb, UNI_BSS_INFO_HE_BASIC, sizeof(*he));

	he = (struct bss_info_uni_he *)tlv;
	he->he_pe_duration = vif->bss_conf.htc_trig_based_pkt_ext;
	if (!he->he_pe_duration)
		he->he_pe_duration = DEFAULT_HE_PE_DURATION;

	he->he_rts_thres = cpu_to_le16(vif->bss_conf.frame_time_rts_th);
	if (!he->he_rts_thres)
		he->he_rts_thres = cpu_to_le16(DEFAULT_HE_DURATION_RTS_THRES);

	he->max_nss_mcs[CMD_HE_MCS_BW80] = cap->he_mcs_nss_supp.tx_mcs_80;
	he->max_nss_mcs[CMD_HE_MCS_BW160] = cap->he_mcs_nss_supp.tx_mcs_160;
	he->max_nss_mcs[CMD_HE_MCS_BW8080] = cap->he_mcs_nss_supp.tx_mcs_80p80;
}

static void
besra_mcu_bss_bmc_tlv(struct sk_buff *skb, struct besra_phy *phy)
{
	struct bss_rate_tlv *bmc;
	struct cfg80211_chan_def *chandef = &phy->mt76->chandef;
	enum nl80211_band band = chandef->chan->band;
	struct tlv *tlv;

	tlv = besra_mcu_add_uni_tlv(skb, UNI_BSS_INFO_RATE, sizeof(*bmc));

	bmc = (struct bss_rate_tlv *)tlv;
	if (band == NL80211_BAND_2GHZ) {
		bmc->short_preamble = true;
	} else {
		bmc->bc_trans = cpu_to_le16(0x8080);
		bmc->mc_trans = cpu_to_le16(0x8080);
		bmc->bc_fixed_rate = 1;
		bmc->mc_fixed_rate = 1;
		bmc->short_preamble = 1;
	}
}

static void
besra_mcu_bss_txcmd_tlv(struct sk_buff *skb, bool en)
{
	struct bss_txcmd_tlv *txcmd;
	struct tlv *tlv;

	tlv = besra_mcu_add_uni_tlv(skb, UNI_BSS_INFO_TXCMD, sizeof(*txcmd));

	txcmd = (struct bss_txcmd_tlv *)tlv;
	txcmd->txcmd_mode = en;
}

static void
besra_mcu_bss_mld_tlv(struct sk_buff *skb)
{
	struct bss_mld_tlv *mld;
	struct tlv *tlv;

	tlv = besra_mcu_add_uni_tlv(skb, UNI_BSS_INFO_MLD, sizeof(*mld));

	mld = (struct bss_mld_tlv *)tlv;
	mld->group_mld_id = 0xff;
	mld->remap_idx = 0xff;
}

static void
besra_mcu_bss_sec_tlv(struct sk_buff *skb, struct ieee80211_vif *vif)
{
	struct mt76_vif *mvif = (struct mt76_vif *)vif->drv_priv;
	struct bss_sec_tlv *sec;
	struct tlv *tlv;

	tlv = besra_mcu_add_uni_tlv(skb, UNI_BSS_INFO_SEC, sizeof(*sec));

	sec = (struct bss_sec_tlv *)tlv;
	sec->cipher = mvif->cipher;
}

static int
besra_mcu_muar_config(struct besra_phy *phy, struct ieee80211_vif *vif,
		       bool bssid, bool enable)
{
	struct besra_dev *dev = phy->dev;
	struct besra_vif *mvif = (struct besra_vif *)vif->drv_priv;
	u32 idx = mvif->mt76.omac_idx - REPEATER_BSSID_START;
	u32 mask = phy->omac_mask >> 32 & ~BIT(idx);
	const u8 *addr = vif->addr;
	struct {
		u8 mode;
		u8 force_clear;
		u8 clear_bitmap[8];
		u8 entry_count;
		u8 write;
		u8 band;

		u8 index;
		u8 bssid;
		u8 addr[ETH_ALEN];
	} __packed req = {
		.mode = !!mask || enable,
		.entry_count = 1,
		.write = 1,
		.band = phy->band_idx,
		.index = idx * 2 + bssid,
	};

	if (bssid)
		addr = vif->bss_conf.bssid;

	if (enable)
		ether_addr_copy(req.addr, addr);

	return mt76_mcu_send_msg(&dev->mt76, MCU_EXT_CMD(MUAR_UPDATE), &req,
				 sizeof(req), true);
}

static int
besra_mcu_bss_basic_tlv(struct sk_buff *skb,
			struct ieee80211_vif *vif,
			struct ieee80211_sta *sta,
			struct mt76_phy *phy, u16 wlan_idx,
			bool enable)
{
	struct mt76_vif *mvif = (struct mt76_vif *)vif->drv_priv;
	struct cfg80211_chan_def *chandef = &phy->chandef;
	struct mt76_connac_bss_basic_tlv *bss;
	struct tlv *tlv;
	u32 type;
	int idx;

	switch (vif->type) {
	case NL80211_IFTYPE_MESH_POINT:
	case NL80211_IFTYPE_AP:
	case NL80211_IFTYPE_MONITOR:
		type = CONNECTION_INFRA_AP;
		break;
	case NL80211_IFTYPE_STATION:
		if (enable) {
			rcu_read_lock();
			if (!sta)
				sta = ieee80211_find_sta(vif,
							 vif->bss_conf.bssid);
			/* TODO: enable BSS_INFO_UAPSD & BSS_INFO_PM */
			if (sta) {
				struct mt76_wcid *wcid;

				wcid = (struct mt76_wcid *)sta->drv_priv;
				wlan_idx = wcid->idx;
			}
			rcu_read_unlock();
		}
		type = CONNECTION_INFRA_STA;
		break;
	case NL80211_IFTYPE_ADHOC:
		type = CONNECTION_IBSS_ADHOC;
		break;
	default:
		WARN_ON(1);
		break;
	}

	tlv = besra_mcu_add_uni_tlv(skb, UNI_BSS_INFO_BASIC, sizeof(*bss));

	bss = (struct mt76_connac_bss_basic_tlv *)tlv;
	bss->bcn_interval = cpu_to_le16(vif->bss_conf.beacon_int);
	bss->dtim_period = vif->bss_conf.dtim_period;
	bss->bmc_tx_wlan_idx = cpu_to_le16(wlan_idx);
	bss->sta_idx = cpu_to_le16(wlan_idx);
	bss->conn_type = cpu_to_le32(type);
	bss->omac_idx = mvif->omac_idx;
	bss->band_idx = mvif->band_idx;
	bss->wmm_idx = mvif->wmm_idx;
	bss->conn_state = !enable;
	bss->active = enable;

	idx = mvif->omac_idx > EXT_BSSID_START ? HW_BSSID_0 : mvif->omac_idx;
	bss->hw_bss_idx = idx;

	if (vif->type != NL80211_IFTYPE_MONITOR) {
		memcpy(bss->bssid, vif->bss_conf.bssid, ETH_ALEN);
		bss->bcn_interval = cpu_to_le16(vif->bss_conf.beacon_int);
		bss->dtim_period = vif->bss_conf.dtim_period;
		bss->phymode = mt76_connac_get_phy_mode(phy, vif,
							chandef->chan->band, NULL);
	} else {
		memcpy(bss->bssid, phy->macaddr, ETH_ALEN);
	}

	if (chandef->chan->band == NL80211_BAND_6GHZ)
		bss->phymode_ext |= BIT(0);

	return 0;
}

static struct sk_buff *
__besra_mcu_alloc_bss_req(struct mt76_dev *dev, struct mt76_vif *mvif, int len)
{
	struct bss_req_hdr hdr = {
		.bss_idx = mvif->idx,
	};
	struct sk_buff *skb;

	skb = mt76_mcu_msg_alloc(dev, NULL, len);
	if (!skb)
		return ERR_PTR(-ENOMEM);

	skb_put_data(skb, &hdr, sizeof(hdr));

	return skb;
}

int besra_mcu_add_bss_info(struct besra_phy *phy,
			    struct ieee80211_vif *vif, int enable)
{
	struct besra_vif *mvif = (struct besra_vif *)vif->drv_priv;
	struct besra_dev *dev = phy->dev;
	struct sk_buff *skb;

	if (mvif->mt76.omac_idx >= REPEATER_BSSID_START) {
		besra_mcu_muar_config(phy, vif, false, enable);
		besra_mcu_muar_config(phy, vif, true, enable);
	}

	skb = __besra_mcu_alloc_bss_req(&dev->mt76, &mvif->mt76,
					BESRA_BSS_UPDATE_MAX_SIZE);
	if (IS_ERR(skb))
		return PTR_ERR(skb);

	/* bss_basic must be first */
	besra_mcu_bss_basic_tlv(skb, vif, NULL, phy->mt76,
				mvif->sta.wcid.idx, enable);
	besra_mcu_bss_sec_tlv(skb, vif);

	if (vif->type == NL80211_IFTYPE_MONITOR)
		goto out;

	if (enable) {
		besra_mcu_bss_rfch_tlv(skb, vif, phy);
		besra_mcu_bss_bmc_tlv(skb, phy);
		besra_mcu_bss_ra_tlv(skb, vif, phy);
		besra_mcu_bss_txcmd_tlv(skb, true);

		if (vif->bss_conf.he_support)
			besra_mcu_bss_he_tlv(skb, vif, phy);

		/* all besra ic need this tlv, no matter it supports EHT or not */
		besra_mcu_bss_mld_tlv(skb);
	}
out:
	return mt76_mcu_skb_send_msg(&dev->mt76, skb,
				     MCU_WMWA_UNI_CMD(BSS_INFO_UPDATE), true);
}

int besra_mcu_sta_ba(struct mt76_dev *dev, struct mt76_vif *mvif,
			   struct ieee80211_ampdu_params *params,
			   bool enable, bool tx)
{
	struct mt76_wcid *wcid = (struct mt76_wcid *)params->sta->drv_priv;
	struct uni_sta_rec_ba *ba;
	struct sk_buff *skb;
	struct tlv *tlv;

	skb = mt76_connac_mcu_alloc_sta_req(dev, mvif, wcid);
	if (IS_ERR(skb))
		return PTR_ERR(skb);

	tlv = mt76_connac_mcu_add_tlv(skb, STA_REC_BA, sizeof(*ba));

	ba = (struct uni_sta_rec_ba *)tlv;
	ba->ba_type = tx ? MT_BA_TYPE_ORIGINATOR : MT_BA_TYPE_RECIPIENT;
	ba->winsize = cpu_to_le16(params->buf_size);
	ba->ssn = cpu_to_le16(params->ssn);
	ba->ba_en = enable << params->tid;
	ba->amsdu = params->amsdu;
	ba->tid = params->tid;

	return mt76_mcu_skb_send_msg(dev, skb,
				     MCU_WMWA_UNI_CMD(STA_REC_UPDATE), true);
}

/** starec & wtbl **/
int besra_mcu_add_tx_ba(struct besra_dev *dev,
			 struct ieee80211_ampdu_params *params,
			 bool enable)
{
	struct besra_sta *msta = (struct besra_sta *)params->sta->drv_priv;
	struct besra_vif *mvif = msta->vif;

	if (enable && !params->amsdu)
		msta->wcid.amsdu = false;

	return besra_mcu_sta_ba(&dev->mt76, &mvif->mt76, params,
				enable, true);
}

int besra_mcu_add_rx_ba(struct besra_dev *dev,
			 struct ieee80211_ampdu_params *params,
			 bool enable)
{
	struct besra_sta *msta = (struct besra_sta *)params->sta->drv_priv;
	struct besra_vif *mvif = msta->vif;

	return besra_mcu_sta_ba(&dev->mt76, &mvif->mt76, params,
				enable, false);
}

static void
besra_mcu_sta_he_tlv(struct sk_buff *skb, struct ieee80211_sta *sta,
		      struct ieee80211_vif *vif)
{
	struct ieee80211_he_cap_elem *elem = &sta->he_cap.he_cap_elem;
	struct ieee80211_he_mcs_nss_supp mcs_map;
	struct sta_rec_he_v2 *he;
	struct tlv *tlv;
	int i = 0;

	if (!sta->he_cap.has_he)
		return;

	tlv = mt76_connac_mcu_add_tlv(skb, STA_REC_HE_V2, sizeof(*he));

	he = (struct sta_rec_he_v2 *)tlv;
	for (i = 0; i < 11; i++){
		if (i < 6)
			he->he_mac_cap[i] = cpu_to_le16(elem->mac_cap_info[i]);
		he->he_phy_cap[i] = cpu_to_le16(elem->phy_cap_info[i]);
	}

	mcs_map = sta->he_cap.he_mcs_nss_supp;
	switch (sta->bandwidth) {
	case IEEE80211_STA_RX_BW_160:
		if (elem->phy_cap_info[0] &
		    IEEE80211_HE_PHY_CAP0_CHANNEL_WIDTH_SET_80PLUS80_MHZ_IN_5G)
			besra_mcu_set_sta_he_mcs(sta,
						  &he->max_nss_mcs[CMD_HE_MCS_BW8080],
						  le16_to_cpu(mcs_map.rx_mcs_80p80));

		besra_mcu_set_sta_he_mcs(sta,
					  &he->max_nss_mcs[CMD_HE_MCS_BW160],
					  le16_to_cpu(mcs_map.rx_mcs_160));
		fallthrough;
	default:
		besra_mcu_set_sta_he_mcs(sta,
					  &he->max_nss_mcs[CMD_HE_MCS_BW80],
					  le16_to_cpu(mcs_map.rx_mcs_80));
		break;
	}

	he->pkt_ext = 2;
}

static void
besra_mcu_sta_he_6g_tlv(struct sk_buff *skb, struct ieee80211_sta *sta,
		      struct ieee80211_vif *vif)
{
	struct sta_rec_he_6g_capa *he_6g;
	struct tlv *tlv;

	if (!sta->he_6ghz_capa.capa)
		return;

	tlv = mt76_connac_mcu_add_tlv(skb, STA_REC_HE_6G, sizeof(*he_6g));

	he_6g = (struct sta_rec_he_6g_capa *)tlv;
	he_6g->capa = cpu_to_le16(sta->he_6ghz_capa.capa);

}

static void
besra_mcu_sta_muru_tlv(struct sk_buff *skb, struct ieee80211_sta *sta,
			struct ieee80211_vif *vif)
{
	struct besra_vif *mvif = (struct besra_vif *)vif->drv_priv;
	struct ieee80211_he_cap_elem *elem = &sta->he_cap.he_cap_elem;
	struct sta_rec_muru *muru;
	struct tlv *tlv;

	if (vif->type != NL80211_IFTYPE_STATION &&
	    vif->type != NL80211_IFTYPE_AP)
		return;

	tlv = mt76_connac_mcu_add_tlv(skb, STA_REC_MURU, sizeof(*muru));

	muru = (struct sta_rec_muru *)tlv;

	muru->cfg.mimo_dl_en = mvif->cap.he_mu_ebfer ||
			       mvif->cap.vht_mu_ebfer ||
			       mvif->cap.vht_mu_ebfee;
	muru->cfg.mimo_ul_en = true;
	muru->cfg.ofdma_dl_en = true;

	muru->mimo_dl.vht_mu_bfee =
		!!(sta->vht_cap.cap & IEEE80211_VHT_CAP_MU_BEAMFORMEE_CAPABLE);

	if (sta->vht_cap.vht_supported)
		muru->mimo_dl.vht_mu_bfee =
			!!(sta->vht_cap.cap & IEEE80211_VHT_CAP_MU_BEAMFORMEE_CAPABLE);

	if (!sta->he_cap.has_he)
		return;

	muru->mimo_dl.partial_bw_dl_mimo =
		HE_PHY(CAP6_PARTIAL_BANDWIDTH_DL_MUMIMO, elem->phy_cap_info[6]);

	muru->cfg.mimo_ul_en = true;
	muru->mimo_ul.full_ul_mimo =
		HE_PHY(CAP2_UL_MU_FULL_MU_MIMO, elem->phy_cap_info[2]);
	muru->mimo_ul.partial_ul_mimo =
		HE_PHY(CAP2_UL_MU_PARTIAL_MU_MIMO, elem->phy_cap_info[2]);

	muru->cfg.ofdma_dl_en = true;
	muru->ofdma_dl.punc_pream_rx =
		HE_PHY(CAP1_PREAMBLE_PUNC_RX_MASK, elem->phy_cap_info[1]);
	muru->ofdma_dl.he_20m_in_40m_2g =
		HE_PHY(CAP8_20MHZ_IN_40MHZ_HE_PPDU_IN_2G, elem->phy_cap_info[8]);
	muru->ofdma_dl.he_20m_in_160m =
		HE_PHY(CAP8_20MHZ_IN_160MHZ_HE_PPDU, elem->phy_cap_info[8]);
	muru->ofdma_dl.he_80m_in_160m =
		HE_PHY(CAP8_80MHZ_IN_160MHZ_HE_PPDU, elem->phy_cap_info[8]);

	muru->ofdma_ul.t_frame_dur =
		HE_MAC(CAP1_TF_MAC_PAD_DUR_MASK, elem->mac_cap_info[1]);
	muru->ofdma_ul.mu_cascading =
		HE_MAC(CAP2_MU_CASCADING, elem->mac_cap_info[2]);
	muru->ofdma_ul.uo_ra =
		HE_MAC(CAP3_OFDMA_RA, elem->mac_cap_info[3]);
}

static void
besra_mcu_sta_ht_tlv(struct sk_buff *skb, struct ieee80211_sta *sta)
{
	struct sta_rec_ht *ht;
	struct tlv *tlv;

	if (!sta->ht_cap.ht_supported)
		return;

	tlv = mt76_connac_mcu_add_tlv(skb, STA_REC_HT, sizeof(*ht));

	ht = (struct sta_rec_ht *)tlv;
	ht->ht_cap = cpu_to_le16(sta->ht_cap.cap);
}

static void
besra_mcu_sta_vht_tlv(struct sk_buff *skb, struct ieee80211_sta *sta)
{
	struct sta_rec_vht *vht;
	struct tlv *tlv;

	if (!sta->vht_cap.vht_supported)
		return;

	tlv = mt76_connac_mcu_add_tlv(skb, STA_REC_VHT, sizeof(*vht));

	vht = (struct sta_rec_vht *)tlv;
	vht->vht_cap = cpu_to_le32(sta->vht_cap.cap);
	vht->vht_rx_mcs_map = sta->vht_cap.vht_mcs.rx_mcs_map;
	vht->vht_tx_mcs_map = sta->vht_cap.vht_mcs.tx_mcs_map;
}

static void
besra_mcu_sta_amsdu_tlv(struct besra_dev *dev, struct sk_buff *skb,
			 struct ieee80211_vif *vif, struct ieee80211_sta *sta)
{
	struct besra_sta *msta = (struct besra_sta *)sta->drv_priv;
	struct sta_rec_amsdu *amsdu;
	struct tlv *tlv;

	if (vif->type != NL80211_IFTYPE_STATION &&
	    vif->type != NL80211_IFTYPE_AP)
		return;

	if (!sta->max_amsdu_len)
	    return;

	tlv = mt76_connac_mcu_add_tlv(skb, STA_REC_HW_AMSDU, sizeof(*amsdu));
	amsdu = (struct sta_rec_amsdu *)tlv;
	amsdu->max_amsdu_num = 8;
	amsdu->amsdu_en = true;
	msta->wcid.amsdu = true;

	switch (sta->max_amsdu_len) {
	case IEEE80211_MAX_MPDU_LEN_VHT_11454:
		amsdu->max_mpdu_size =
			IEEE80211_VHT_CAP_MAX_MPDU_LENGTH_11454;
		return;
	case IEEE80211_MAX_MPDU_LEN_HT_7935:
	case IEEE80211_MAX_MPDU_LEN_VHT_7991:
		amsdu->max_mpdu_size = IEEE80211_VHT_CAP_MAX_MPDU_LENGTH_7991;
		return;
	default:
		amsdu->max_mpdu_size = IEEE80211_VHT_CAP_MAX_MPDU_LENGTH_3895;
		return;
	}
}

#if 0
static int
besra_mcu_sta_wtbl_tlv(struct besra_dev *dev, struct sk_buff *skb,
			struct ieee80211_vif *vif, struct ieee80211_sta *sta)
{
	struct besra_vif *mvif = (struct besra_vif *)vif->drv_priv;
	struct besra_sta *msta;
	struct wtbl_req_hdr *wtbl_hdr;
	struct mt76_wcid *wcid;
	struct tlv *tlv;

	msta = sta ? (struct besra_sta *)sta->drv_priv : &mvif->sta;
	wcid = sta ? &msta->wcid : NULL;

	tlv = mt76_connac_mcu_add_tlv(skb, STA_REC_WTBL, sizeof(struct tlv));
	wtbl_hdr = mt76_connac_mcu_alloc_wtbl_req(&dev->mt76, &msta->wcid,
						  WTBL_RESET_AND_SET, tlv,
						  &skb);
	if (IS_ERR(wtbl_hdr))
		return PTR_ERR(wtbl_hdr);

	mt76_connac_mcu_wtbl_generic_tlv(&dev->mt76, skb, vif, sta, tlv,
					 wtbl_hdr);
	mt76_connac_mcu_wtbl_hdr_trans_tlv(skb, vif, wcid, tlv, wtbl_hdr);
	if (sta)
		mt76_connac_mcu_wtbl_ht_tlv(&dev->mt76, skb, sta, tlv,
					    wtbl_hdr, mvif->cap.ldpc);

	return 0;
}
#endif

static inline bool
besra_is_ebf_supported(struct besra_phy *phy, struct ieee80211_vif *vif,
			struct ieee80211_sta *sta, bool bfee)
{
	struct besra_vif *mvif = (struct besra_vif *)vif->drv_priv;
	int tx_ant = hweight8(phy->mt76->chainmask) - 1;

	if (vif->type != NL80211_IFTYPE_STATION &&
	    vif->type != NL80211_IFTYPE_AP)
		return false;

	if (!bfee && tx_ant < 2)
		return false;

	if (sta->he_cap.has_he) {
		struct ieee80211_he_cap_elem *pe = &sta->he_cap.he_cap_elem;

		if (bfee)
			return mvif->cap.he_su_ebfee &&
			       HE_PHY(CAP3_SU_BEAMFORMER, pe->phy_cap_info[3]);
		else
			return mvif->cap.he_su_ebfer &&
			       HE_PHY(CAP4_SU_BEAMFORMEE, pe->phy_cap_info[4]);
	}

	if (sta->vht_cap.vht_supported) {
		u32 cap = sta->vht_cap.cap;

		if (bfee)
			return mvif->cap.vht_su_ebfee &&
			       (cap & IEEE80211_VHT_CAP_SU_BEAMFORMER_CAPABLE);
		else
			return mvif->cap.vht_su_ebfer &&
			       (cap & IEEE80211_VHT_CAP_SU_BEAMFORMEE_CAPABLE);
	}

	return false;
}

static void
besra_mcu_sta_sounding_rate(struct sta_rec_bf *bf)
{
	bf->sounding_phy = MT_PHY_TYPE_OFDM;
	bf->ndp_rate = 0;				/* mcs0 */
	bf->ndpa_rate = BESRA_CFEND_RATE_DEFAULT;	/* ofdm 24m */
	bf->rept_poll_rate = BESRA_CFEND_RATE_DEFAULT;	/* ofdm 24m */
}

static void
besra_mcu_sta_bfer_ht(struct ieee80211_sta *sta, struct besra_phy *phy,
		       struct sta_rec_bf *bf)
{
	struct ieee80211_mcs_info *mcs = &sta->ht_cap.mcs;
	u8 n = 0;

	bf->tx_mode = MT_PHY_TYPE_HT;

	if ((mcs->tx_params & IEEE80211_HT_MCS_TX_RX_DIFF) &&
	    (mcs->tx_params & IEEE80211_HT_MCS_TX_DEFINED))
		n = FIELD_GET(IEEE80211_HT_MCS_TX_MAX_STREAMS_MASK,
			      mcs->tx_params);
	else if (mcs->rx_mask[3])
		n = 3;
	else if (mcs->rx_mask[2])
		n = 2;
	else if (mcs->rx_mask[1])
		n = 1;

	bf->nrow = hweight8(phy->mt76->chainmask) - 1;
	bf->ncol = min_t(u8, bf->nrow, n);
	bf->ibf_ncol = n;
}

static void
besra_mcu_sta_bfer_vht(struct ieee80211_sta *sta, struct besra_phy *phy,
			struct sta_rec_bf *bf, bool explicit)
{
	struct ieee80211_sta_vht_cap *pc = &sta->vht_cap;
	struct ieee80211_sta_vht_cap *vc = &phy->mt76->sband_5g.sband.vht_cap;
	u16 mcs_map = le16_to_cpu(pc->vht_mcs.rx_mcs_map);
	u8 nss_mcs = besra_mcu_get_sta_nss(mcs_map);
	u8 tx_ant = hweight8(phy->mt76->chainmask) - 1;

	bf->tx_mode = MT_PHY_TYPE_VHT;

	if (explicit) {
		u8 sts, snd_dim;

		besra_mcu_sta_sounding_rate(bf);

		sts = FIELD_GET(IEEE80211_VHT_CAP_BEAMFORMEE_STS_MASK,
				pc->cap);
		snd_dim = FIELD_GET(IEEE80211_VHT_CAP_SOUNDING_DIMENSIONS_MASK,
				    vc->cap);
		bf->nrow = min_t(u8, min_t(u8, snd_dim, sts), tx_ant);
		bf->ncol = min_t(u8, nss_mcs, bf->nrow);
		bf->ibf_ncol = bf->ncol;

		if (sta->bandwidth == IEEE80211_STA_RX_BW_160)
			bf->nrow = 1;
	} else {
		bf->nrow = tx_ant;
		bf->ncol = min_t(u8, nss_mcs, bf->nrow);
		bf->ibf_ncol = nss_mcs;

		if (sta->bandwidth == IEEE80211_STA_RX_BW_160)
			bf->ibf_nrow = 1;
	}
}

static void
besra_mcu_sta_bfer_he(struct ieee80211_sta *sta, struct ieee80211_vif *vif,
		       struct besra_phy *phy, struct sta_rec_bf *bf)
{
	struct ieee80211_sta_he_cap *pc = &sta->he_cap;
	struct ieee80211_he_cap_elem *pe = &pc->he_cap_elem;
	const struct ieee80211_sta_he_cap *vc =
		mt76_connac_get_he_phy_cap(phy->mt76, vif);
	const struct ieee80211_he_cap_elem *ve = &vc->he_cap_elem;
	u16 mcs_map = le16_to_cpu(pc->he_mcs_nss_supp.rx_mcs_80);
	u8 nss_mcs = besra_mcu_get_sta_nss(mcs_map);
	u8 snd_dim, sts;

	bf->tx_mode = MT_PHY_TYPE_HE_SU;

	besra_mcu_sta_sounding_rate(bf);

	bf->trigger_su = HE_PHY(CAP6_TRIG_SU_BEAMFORMING_FB,
				pe->phy_cap_info[6]);
	bf->trigger_mu = HE_PHY(CAP6_TRIG_MU_BEAMFORMING_PARTIAL_BW_FB,
				pe->phy_cap_info[6]);
	snd_dim = HE_PHY(CAP5_BEAMFORMEE_NUM_SND_DIM_UNDER_80MHZ_MASK,
			 ve->phy_cap_info[5]);
	sts = HE_PHY(CAP4_BEAMFORMEE_MAX_STS_UNDER_80MHZ_MASK,
		     pe->phy_cap_info[4]);
	bf->nrow = min_t(u8, snd_dim, sts);
	bf->ncol = min_t(u8, nss_mcs, bf->nrow);
	bf->ibf_ncol = bf->ncol;

	if (sta->bandwidth != IEEE80211_STA_RX_BW_160)
		return;

	/* go over for 160MHz and 80p80 */
	if (pe->phy_cap_info[0] &
	    IEEE80211_HE_PHY_CAP0_CHANNEL_WIDTH_SET_160MHZ_IN_5G) {
		mcs_map = le16_to_cpu(pc->he_mcs_nss_supp.rx_mcs_160);
		nss_mcs = besra_mcu_get_sta_nss(mcs_map);

		bf->ncol_bw160 = nss_mcs;
	}

	if (pe->phy_cap_info[0] &
	    IEEE80211_HE_PHY_CAP0_CHANNEL_WIDTH_SET_80PLUS80_MHZ_IN_5G) {
		mcs_map = le16_to_cpu(pc->he_mcs_nss_supp.rx_mcs_80p80);
		nss_mcs = besra_mcu_get_sta_nss(mcs_map);

		if (bf->ncol_bw160)
			bf->ncol_bw160 = min_t(u8, bf->ncol_bw160, nss_mcs);
		else
			bf->ncol_bw160 = nss_mcs;
	}

	snd_dim = HE_PHY(CAP5_BEAMFORMEE_NUM_SND_DIM_ABOVE_80MHZ_MASK,
			 ve->phy_cap_info[5]);
	sts = HE_PHY(CAP4_BEAMFORMEE_MAX_STS_ABOVE_80MHZ_MASK,
		     pe->phy_cap_info[4]);

	bf->nrow_bw160 = min_t(int, snd_dim, sts);
}

static void
besra_mcu_sta_bfer_tlv(struct besra_dev *dev, struct sk_buff *skb,
		       struct ieee80211_vif *vif, struct ieee80211_sta *sta)
{
	struct besra_vif *mvif = (struct besra_vif *)vif->drv_priv;
	struct besra_phy *phy = mvif->phy;
	int tx_ant = hweight8(phy->mt76->chainmask) - 1;
	struct sta_rec_bf *bf;
	struct tlv *tlv;
	const u8 matrix[4][4] = {
		{0, 0, 0, 0},
		{1, 1, 0, 0},	/* 2x1, 2x2, 2x3, 2x4 */
		{2, 4, 4, 0},	/* 3x1, 3x2, 3x3, 3x4 */
		{3, 5, 6, 0}	/* 4x1, 4x2, 4x3, 4x4 */
	};
	bool ebf;

	if (!(sta->ht_cap.ht_supported || sta->he_cap.has_he))
		return;

	ebf = besra_is_ebf_supported(phy, vif, sta, false);
	if (!ebf && !dev->ibf)
		return;

	tlv = mt76_connac_mcu_add_tlv(skb, STA_REC_BF, sizeof(*bf));
	bf = (struct sta_rec_bf *)tlv;

	/* he: eBF only, in accordance with spec
	 * vht: support eBF and iBF
	 * ht: iBF only, since mac80211 lacks of eBF support
	 */
	if (sta->he_cap.has_he && ebf)
		besra_mcu_sta_bfer_he(sta, vif, phy, bf);
	else if (sta->vht_cap.vht_supported)
		besra_mcu_sta_bfer_vht(sta, phy, bf, ebf);
	else if (sta->ht_cap.ht_supported)
		besra_mcu_sta_bfer_ht(sta, phy, bf);
	else
		return;

	bf->bf_cap = ebf ? ebf : dev->ibf << 1;
	bf->bw = sta->bandwidth;
	bf->ibf_dbw = sta->bandwidth;
	bf->ibf_nrow = tx_ant;

	if (!ebf && sta->bandwidth <= IEEE80211_STA_RX_BW_40 && !bf->ncol)
		bf->ibf_timeout = 0x48;
	else
		bf->ibf_timeout = 0x18;

	if (ebf && bf->nrow != tx_ant)
		bf->mem_20m = matrix[tx_ant][bf->ncol];
	else
		bf->mem_20m = matrix[bf->nrow][bf->ncol];

	switch (sta->bandwidth) {
	case IEEE80211_STA_RX_BW_160:
	case IEEE80211_STA_RX_BW_80:
		bf->mem_total = bf->mem_20m * 2;
		break;
	case IEEE80211_STA_RX_BW_40:
		bf->mem_total = bf->mem_20m;
		break;
	case IEEE80211_STA_RX_BW_20:
	default:
		break;
	}
}

static void
besra_mcu_sta_bfee_tlv(struct besra_dev *dev, struct sk_buff *skb,
			struct ieee80211_vif *vif, struct ieee80211_sta *sta)
{
	struct besra_vif *mvif = (struct besra_vif *)vif->drv_priv;
	struct besra_phy *phy = mvif->phy;
	int tx_ant = hweight8(phy->mt76->chainmask) - 1;
	struct sta_rec_bfee *bfee;
	struct tlv *tlv;
	u8 nrow = 0;

	if (!(sta->vht_cap.vht_supported || sta->he_cap.has_he))
		return;

	if (!besra_is_ebf_supported(phy, vif, sta, true))
		return;

	tlv = mt76_connac_mcu_add_tlv(skb, STA_REC_BFEE, sizeof(*bfee));
	bfee = (struct sta_rec_bfee *)tlv;

	if (sta->he_cap.has_he) {
		struct ieee80211_he_cap_elem *pe = &sta->he_cap.he_cap_elem;

		nrow = HE_PHY(CAP5_BEAMFORMEE_NUM_SND_DIM_UNDER_80MHZ_MASK,
			      pe->phy_cap_info[5]);
	} else if (sta->vht_cap.vht_supported) {
		struct ieee80211_sta_vht_cap *pc = &sta->vht_cap;

		nrow = FIELD_GET(IEEE80211_VHT_CAP_SOUNDING_DIMENSIONS_MASK,
				 pc->cap);
	}

	/* reply with identity matrix to avoid 2x2 BF negative gain */
	bfee->fb_identity_matrix = (nrow == 1 && tx_ant == 2);
}

static void
besra_mcu_sta_phy_tlv(struct besra_dev *dev, struct sk_buff *skb,
		      struct ieee80211_vif *vif, struct ieee80211_sta *sta)
{
	struct besra_vif *mvif = (struct besra_vif *)vif->drv_priv;
	struct cfg80211_chan_def *chandef = &mvif->phy->mt76->chandef;
	enum nl80211_band band = chandef->chan->band;
	struct mt76_phy *mphy = &dev->mphy;
	struct sta_rec_phy *phy;
	struct tlv *tlv;
	u8 af, mm;

	if (!sta->ht_cap.ht_supported && !sta->he_6ghz_capa.capa)
		return;

	tlv = mt76_connac_mcu_add_tlv(skb, STA_REC_PHY, sizeof(*phy));

	phy = (struct sta_rec_phy *)tlv;
	phy->phy_type = mt76_connac_get_phy_mode_v2(mphy, vif, band, sta);
	phy->basic_rate = cpu_to_le16((u16)vif->bss_conf.basic_rates);
	if (sta->ht_cap.ht_supported) {
		af = sta->ht_cap.ampdu_factor;
		mm = sta->ht_cap.ampdu_density;
	} else {
		af = le16_get_bits(sta->he_6ghz_capa.capa,
				   IEEE80211_HE_6GHZ_CAP_MAX_AMPDU_LEN_EXP);
		mm = le16_get_bits(sta->he_6ghz_capa.capa,
				   IEEE80211_HE_6GHZ_CAP_MIN_MPDU_START);
	}

	phy->ampdu = FIELD_PREP(IEEE80211_HT_AMPDU_PARM_FACTOR, af) |
		     FIELD_PREP(IEEE80211_HT_AMPDU_PARM_DENSITY, mm);
}

static void
besra_mcu_sta_hdr_trans_tlv(struct besra_dev *dev, struct sk_buff *skb,
			 struct ieee80211_vif *vif, struct ieee80211_sta *sta)
{
	struct sta_rec_hdr_trans *hdr_trans;
	struct tlv *tlv;

	tlv = mt76_connac_mcu_add_tlv(skb, STA_REC_HDR_TRANS, sizeof(*hdr_trans));
	hdr_trans = (struct sta_rec_hdr_trans*) tlv;
	hdr_trans->dis_rx_hdr_tran = 0;
	if (vif->type == NL80211_IFTYPE_STATION)
		hdr_trans->to_ds = true;
	else
		hdr_trans->from_ds = true;

	struct mt76_wcid *wcid;
	wcid = (struct mt76_wcid *)sta->drv_priv;
	if (!wcid)
		return;

	if (test_bit(MT_WCID_FLAG_4ADDR, &wcid->flags)) {
		hdr_trans->to_ds = true;
		hdr_trans->from_ds = true;
	}
}

static enum mcu_mmps_mode
besra_mcu_get_mmps_mode(enum ieee80211_smps_mode smps)
{
	switch (smps) {
	case IEEE80211_SMPS_OFF:
		return MCU_MMPS_DISABLE;
	case IEEE80211_SMPS_STATIC:
		return MCU_MMPS_STATIC;
	case IEEE80211_SMPS_DYNAMIC:
		return MCU_MMPS_DYNAMIC;
	default:
		return MCU_MMPS_DISABLE;
	}
}

int besra_mcu_set_fixed_rate_ctrl(struct besra_dev *dev,
				   struct ieee80211_vif *vif,
				   struct ieee80211_sta *sta,
				   void *data, u32 field)
{
	struct besra_vif *mvif = (struct besra_vif *)vif->drv_priv;
	struct besra_sta *msta = (struct besra_sta *)sta->drv_priv;
	struct sta_phy *phy = data;
	struct sta_rec_ra_fixed *ra;
	struct sk_buff *skb;
	struct tlv *tlv;

	skb = mt76_connac_mcu_alloc_sta_req(&dev->mt76, &mvif->mt76,
					    &msta->wcid);
	if (IS_ERR(skb))
		return PTR_ERR(skb);

	tlv = mt76_connac_mcu_add_tlv(skb, STA_REC_RA_UPDATE, sizeof(*ra));
	ra = (struct sta_rec_ra_fixed *)tlv;

	switch (field) {
	case RATE_PARAM_AUTO:
		break;
	case RATE_PARAM_FIXED:
	case RATE_PARAM_FIXED_MCS:
	case RATE_PARAM_FIXED_GI:
	case RATE_PARAM_FIXED_HE_LTF:
		if (phy)
			ra->phy = *phy;
		break;
	case RATE_PARAM_MMPS_UPDATE:
		ra->mmps_mode = besra_mcu_get_mmps_mode(sta->smps_mode);
		break;
	default:
		break;
	}
	ra->field = cpu_to_le32(field);

	return mt76_mcu_skb_send_msg(&dev->mt76, skb,
				     MCU_WMWA_UNI_CMD(STA_REC_UPDATE), true);
}

int besra_mcu_add_smps(struct besra_dev *dev, struct ieee80211_vif *vif,
			struct ieee80211_sta *sta)
{
#if 0
	struct besra_vif *mvif = (struct besra_vif *)vif->drv_priv;
	struct besra_sta *msta = (struct besra_sta *)sta->drv_priv;
	struct wtbl_req_hdr *wtbl_hdr;
	struct tlv *sta_wtbl;
	struct sk_buff *skb;
	int ret;

	skb = mt76_connac_mcu_alloc_sta_req(&dev->mt76, &mvif->mt76,
					    &msta->wcid);
	if (IS_ERR(skb))
		return PTR_ERR(skb);

	sta_wtbl = mt76_connac_mcu_add_tlv(skb, STA_REC_WTBL,
					   sizeof(struct tlv));
	wtbl_hdr = mt76_connac_mcu_alloc_wtbl_req(&dev->mt76, &msta->wcid,
						  WTBL_SET, sta_wtbl, &skb);
	if (IS_ERR(wtbl_hdr))
		return PTR_ERR(wtbl_hdr);

	mt76_connac_mcu_wtbl_smps_tlv(skb, sta, sta_wtbl, wtbl_hdr);

	ret = mt76_mcu_skb_send_msg(&dev->mt76, skb,
				    MCU_EXT_CMD(STA_REC_UPDATE), true);
	if (ret)
		return ret;
#endif
	printk("Remove smps tag in wtbl. Todo: check it is corrected or not");

	return besra_mcu_set_fixed_rate_ctrl(dev, vif, sta, NULL,
					      RATE_PARAM_MMPS_UPDATE);
}

static int
besra_mcu_add_rate_ctrl_fixed(struct besra_dev *dev,
			       struct ieee80211_vif *vif,
			       struct ieee80211_sta *sta)
{
	struct besra_vif *mvif = (struct besra_vif *)vif->drv_priv;
	struct cfg80211_chan_def *chandef = &mvif->phy->mt76->chandef;
	struct cfg80211_bitrate_mask *mask = &mvif->bitrate_mask;
	enum nl80211_band band = chandef->chan->band;
	struct sta_phy phy = {};
	int ret, nrates = 0;

#define __sta_phy_bitrate_mask_check(_mcs, _gi, _he)				\
	do {									\
		u8 i, gi = mask->control[band]._gi;				\
		gi = (_he) ? gi : gi == NL80211_TXRATE_FORCE_SGI;		\
		for (i = 0; i <= sta->bandwidth; i++) {				\
			phy.sgi |= gi << (i << (_he));				\
			phy.he_ltf |= mask->control[band].he_ltf << (i << (_he));\
		}								\
		for (i = 0; i < ARRAY_SIZE(mask->control[band]._mcs); i++) {	\
			if (!mask->control[band]._mcs[i])			\
				continue;					\
			nrates += hweight16(mask->control[band]._mcs[i]);	\
			phy.mcs = ffs(mask->control[band]._mcs[i]) - 1;		\
		}								\
	} while (0)

	if (sta->he_cap.has_he) {
		__sta_phy_bitrate_mask_check(he_mcs, he_gi, 1);
	} else if (sta->vht_cap.vht_supported) {
		__sta_phy_bitrate_mask_check(vht_mcs, gi, 0);
	} else if (sta->ht_cap.ht_supported) {
		__sta_phy_bitrate_mask_check(ht_mcs, gi, 0);
	} else {
		nrates = hweight32(mask->control[band].legacy);
		phy.mcs = ffs(mask->control[band].legacy) - 1;
	}
#undef __sta_phy_bitrate_mask_check

	/* fall back to auto rate control */
	if (mask->control[band].gi == NL80211_TXRATE_DEFAULT_GI &&
	    mask->control[band].he_gi == GENMASK(7, 0) &&
	    mask->control[band].he_ltf == GENMASK(7, 0) &&
	    nrates != 1)
		return 0;

	/* fixed single rate */
	if (nrates == 1) {
		ret = besra_mcu_set_fixed_rate_ctrl(dev, vif, sta, &phy,
						     RATE_PARAM_FIXED_MCS);
		if (ret)
			return ret;
	}

	/* fixed GI */
	if (mask->control[band].gi != NL80211_TXRATE_DEFAULT_GI ||
	    mask->control[band].he_gi != GENMASK(7, 0)) {
		struct besra_sta *msta = (struct besra_sta *)sta->drv_priv;
		u32 addr;

		/* firmware updates only TXCMD but doesn't take WTBL into
		 * account, so driver should update here to reflect the
		 * actual txrate hardware sends out.
		 */
		addr = besra_mac_wtbl_lmac_addr(dev, msta->wcid.idx, 7);
		if (sta->he_cap.has_he)
			mt76_rmw_field(dev, addr, GENMASK(31, 24), phy.sgi);
		else
			mt76_rmw_field(dev, addr, GENMASK(15, 12), phy.sgi);

		ret = besra_mcu_set_fixed_rate_ctrl(dev, vif, sta, &phy,
						     RATE_PARAM_FIXED_GI);
		if (ret)
			return ret;
	}

	/* fixed HE_LTF */
	if (mask->control[band].he_ltf != GENMASK(7, 0)) {
		ret = besra_mcu_set_fixed_rate_ctrl(dev, vif, sta, &phy,
						     RATE_PARAM_FIXED_HE_LTF);
		if (ret)
			return ret;
	}

	return 0;
}

static void
besra_mcu_sta_rate_ctrl_tlv(struct sk_buff *skb, struct besra_dev *dev,
			     struct ieee80211_vif *vif, struct ieee80211_sta *sta)
{
	struct besra_vif *mvif = (struct besra_vif *)vif->drv_priv;
	struct mt76_phy *mphy = mvif->phy->mt76;
	struct cfg80211_chan_def *chandef = &mphy->chandef;
	struct cfg80211_bitrate_mask *mask = &mvif->bitrate_mask;
	enum nl80211_band band = chandef->chan->band;
	struct sta_rec_ra *ra;
	struct tlv *tlv;
	u32 supp_rate = sta->supp_rates[band];
	u32 cap = sta->wme ? STA_CAP_WMM : 0;

	tlv = mt76_connac_mcu_add_tlv(skb, STA_REC_RA, sizeof(*ra));
	ra = (struct sta_rec_ra *)tlv;

	ra->valid = true;
	ra->auto_rate = true;
	ra->phy_mode = mt76_connac_get_phy_mode(mphy, vif, band, sta);
	ra->channel = chandef->chan->hw_value;
	ra->bw = sta->bandwidth;
	ra->phy.bw = sta->bandwidth;
	ra->mmps_mode = besra_mcu_get_mmps_mode(sta->smps_mode);

	if (supp_rate) {
		supp_rate &= mask->control[band].legacy;
		ra->rate_len = hweight32(supp_rate);

		if (band == NL80211_BAND_2GHZ) {
			ra->supp_mode = MODE_CCK;
			ra->supp_cck_rate = supp_rate & GENMASK(3, 0);

			if (ra->rate_len > 4) {
				ra->supp_mode |= MODE_OFDM;
				ra->supp_ofdm_rate = supp_rate >> 4;
			}
		} else {
			ra->supp_mode = MODE_OFDM;
			ra->supp_ofdm_rate = supp_rate;
		}
	}

	if (sta->ht_cap.ht_supported) {
		ra->supp_mode |= MODE_HT;
		ra->af = sta->ht_cap.ampdu_factor;
		ra->ht_gf = !!(sta->ht_cap.cap & IEEE80211_HT_CAP_GRN_FLD);

		cap |= STA_CAP_HT;
		if (sta->ht_cap.cap & IEEE80211_HT_CAP_SGI_20)
			cap |= STA_CAP_SGI_20;
		if (sta->ht_cap.cap & IEEE80211_HT_CAP_SGI_40)
			cap |= STA_CAP_SGI_40;
		if (sta->ht_cap.cap & IEEE80211_HT_CAP_TX_STBC)
			cap |= STA_CAP_TX_STBC;
		if (sta->ht_cap.cap & IEEE80211_HT_CAP_RX_STBC)
			cap |= STA_CAP_RX_STBC;
		if (mvif->cap.ht_ldpc &&
		    (sta->ht_cap.cap & IEEE80211_HT_CAP_LDPC_CODING))
			cap |= STA_CAP_LDPC;

		besra_mcu_set_sta_ht_mcs(sta, ra->ht_mcs,
					  mask->control[band].ht_mcs);
		ra->supp_ht_mcs = *(__le32 *)ra->ht_mcs;
	}

	if (sta->vht_cap.vht_supported) {
		u8 af;

		ra->supp_mode |= MODE_VHT;
		af = FIELD_GET(IEEE80211_VHT_CAP_MAX_A_MPDU_LENGTH_EXPONENT_MASK,
			       sta->vht_cap.cap);
		ra->af = max_t(u8, ra->af, af);

		cap |= STA_CAP_VHT;
		if (sta->vht_cap.cap & IEEE80211_VHT_CAP_SHORT_GI_80)
			cap |= STA_CAP_VHT_SGI_80;
		if (sta->vht_cap.cap & IEEE80211_VHT_CAP_SHORT_GI_160)
			cap |= STA_CAP_VHT_SGI_160;
		if (sta->vht_cap.cap & IEEE80211_VHT_CAP_TXSTBC)
			cap |= STA_CAP_VHT_TX_STBC;
		if (sta->vht_cap.cap & IEEE80211_VHT_CAP_RXSTBC_1)
			cap |= STA_CAP_VHT_RX_STBC;
		if (mvif->cap.vht_ldpc &&
		    (sta->vht_cap.cap & IEEE80211_VHT_CAP_RXLDPC))
			cap |= STA_CAP_VHT_LDPC;

		besra_mcu_set_sta_vht_mcs(sta, ra->supp_vht_mcs,
					   mask->control[band].vht_mcs);
	}

	if (sta->he_cap.has_he) {
		ra->supp_mode |= MODE_HE;
		cap |= STA_CAP_HE;

		if (sta->he_6ghz_capa.capa)
			ra->af = le16_get_bits(sta->he_6ghz_capa.capa,
					       IEEE80211_HE_6GHZ_CAP_MAX_AMPDU_LEN_EXP);
	}

	ra->sta_cap = cpu_to_le32(cap);
}

int besra_mcu_add_rate_ctrl(struct besra_dev *dev, struct ieee80211_vif *vif,
			     struct ieee80211_sta *sta, bool changed)
{
	struct besra_vif *mvif = (struct besra_vif *)vif->drv_priv;
	struct besra_sta *msta = (struct besra_sta *)sta->drv_priv;
	struct sk_buff *skb;
	int ret;

	skb = mt76_connac_mcu_alloc_sta_req(&dev->mt76, &mvif->mt76,
					    &msta->wcid);
	if (IS_ERR(skb))
		return PTR_ERR(skb);

	/* firmware rc algorithm refers to sta_rec_he for HE control.
	 * once dev->rc_work changes the settings driver should also
	 * update sta_rec_he here.
	 */
	if (changed)
		besra_mcu_sta_he_tlv(skb, sta, vif);

	/* sta_rec_ra accommodates BW, NSS and only MCS range format
	 * i.e 0-{7,8,9} for VHT.
	 */
	besra_mcu_sta_rate_ctrl_tlv(skb, dev, vif, sta);

	ret = mt76_mcu_skb_send_msg(&dev->mt76, skb,
				    MCU_WMWA_UNI_CMD(STA_REC_UPDATE), true);
	if (ret)
		return ret;

	/* sta_rec_ra_fixed accommodates single rate, (HE)GI and HE_LTE,
	 * and updates as peer fixed rate parameters, which overrides
	 * sta_rec_ra and firmware rate control algorithm.
	 */
	return besra_mcu_add_rate_ctrl_fixed(dev, vif, sta);
}

static int
besra_mcu_add_group(struct besra_dev *dev, struct ieee80211_vif *vif,
		     struct ieee80211_sta *sta)
{
#define MT_STA_BSS_GROUP		1
	struct besra_vif *mvif = (struct besra_vif *)vif->drv_priv;
	struct besra_sta *msta;
	struct {
		/* fixed field */
		u8 __rsv1[4];
		/* TLV */
		__le16 tag;
		__le16 len;
		__le16 wlan_idx;
		u8 __rsv2[2];
		__le32 action;
		__le32 val;
		u8 __rsv3[8];
	} __packed req = {
		.tag = cpu_to_le16(UNI_VOW_DRR_CTRL),
		.len = cpu_to_le16(sizeof(req) - 4),
		.action = cpu_to_le32(MT_STA_BSS_GROUP),
		.val = cpu_to_le32(mvif->mt76.idx % 16),
	};

	msta = sta ? (struct besra_sta *)sta->drv_priv : &mvif->sta;
	req.wlan_idx = cpu_to_le16(msta->wcid.idx);

	return mt76_mcu_send_msg(&dev->mt76, MCU_WM_UNI_CMD(VOW), &req,
				 sizeof(req), true);
}

int besra_mcu_add_sta(struct besra_dev *dev, struct ieee80211_vif *vif,
		       struct ieee80211_sta *sta, bool enable)
{
	struct besra_vif *mvif = (struct besra_vif *)vif->drv_priv;
	struct besra_sta *msta;
	struct sk_buff *skb;
	int ret;

	msta = sta ? (struct besra_sta *)sta->drv_priv : &mvif->sta;

	skb = mt76_connac_mcu_alloc_sta_req(&dev->mt76, &mvif->mt76,
					    &msta->wcid);
	if (IS_ERR(skb))
		return PTR_ERR(skb);

	/* starec basic */
	mt76_connac_mcu_sta_basic_tlv(skb, vif, sta, enable, true);
	if (!enable)
		goto out;

	/* tag order is in accordance with firmware dependency. */
	if (sta) {
		/* starec phy */
		if (mt76_chip(&dev->mt76) != 0x7902)
			besra_mcu_sta_phy_tlv(dev, skb, vif, sta);
		/* starec bfer */
		besra_mcu_sta_bfer_tlv(dev, skb, vif, sta);
		/* starec ht */
		besra_mcu_sta_ht_tlv(skb, sta);
		/* starec vht */
		besra_mcu_sta_vht_tlv(skb, sta);
		/* starec uapsd */
		mt76_connac_mcu_sta_uapsd(skb, vif, sta);
		/* starec amsdu */
		besra_mcu_sta_amsdu_tlv(dev, skb, vif, sta);
		/* starec he */
		besra_mcu_sta_he_tlv(skb, sta, vif);
		/* starec he 6g*/
		besra_mcu_sta_he_6g_tlv(skb, sta, vif);
		/* starec muru */
		besra_mcu_sta_muru_tlv(skb, sta, vif);
		/* starec bfee */
		besra_mcu_sta_bfee_tlv(dev, skb, vif, sta);
		/* starec hdr trans */
		besra_mcu_sta_hdr_trans_tlv(dev, skb, vif, sta);
	}

	ret = besra_mcu_add_group(dev, vif, sta);
	if (ret) {
		dev_kfree_skb(skb);
		return ret;
	}
out:
	return mt76_mcu_skb_send_msg(&dev->mt76, skb,
				     MCU_WMWA_UNI_CMD(STA_REC_UPDATE), true);
}

static int
besra_mcu_sta_key_tlv(struct mt76_wcid *wcid,
		      struct mt76_connac_sta_key_conf *sta_key_conf,
		      struct sk_buff *skb,
		      struct ieee80211_key_conf *key,
		      enum set_key_cmd cmd)
{
	struct uni_sta_rec_sec *sec;
	struct tlv *tlv;

	tlv = mt76_connac_mcu_add_tlv(skb, STA_REC_KEY_V2, sizeof(*sec));
	sec = (struct uni_sta_rec_sec *)tlv;
	sec->add = cmd;

	if (cmd == SET_KEY) {
		struct uni_sec_key *sec_key;
		u8 cipher;

		cipher = mt76_connac_mcu_get_cipher(key->cipher);
		if (cipher == MCU_CIPHER_NONE)
			return -EOPNOTSUPP;

		sec_key = &sec->key[0];
		sec_key->cipher_len = sizeof(*sec_key);

		if (cipher == MCU_CIPHER_BIP_CMAC_128) {
			sec_key->wlan_idx = cpu_to_le16(wcid->idx);
			sec_key->cipher_id = MCU_CIPHER_AES_CCMP;
			sec_key->key_id = sta_key_conf->keyidx;
			sec_key->key_len = 16;
			memcpy(sec_key->key, sta_key_conf->key, 16);

			sec_key = &sec->key[1];
			sec_key->wlan_idx = cpu_to_le16(wcid->idx);
			sec_key->cipher_id = MCU_CIPHER_BIP_CMAC_128;
			sec_key->cipher_len = sizeof(*sec_key);
			sec_key->key_len = 16;
			memcpy(sec_key->key, key->key, 16);
			sec->n_cipher = 2;
		} else {
			sec_key->wlan_idx = cpu_to_le16(wcid->idx);
			sec_key->cipher_id = cipher;
			sec_key->key_id = key->keyidx;
			sec_key->key_len = key->keylen;
			memcpy(sec_key->key, key->key, key->keylen);

			if (cipher == MCU_CIPHER_TKIP) {
				/* Rx/Tx MIC keys are swapped */
				memcpy(sec_key->key + 16, key->key + 24, 8);
				memcpy(sec_key->key + 24, key->key + 16, 8);
			}

			/* store key_conf for BIP batch update */
			if (cipher == MCU_CIPHER_AES_CCMP) {
				memcpy(sta_key_conf->key, key->key, key->keylen);
				sta_key_conf->keyidx = key->keyidx;
			}

			sec->n_cipher = 1;
		}
	} else {
		sec->n_cipher = 0;
	}

	return 0;
}

int besra_mcu_add_key(struct mt76_dev *dev, struct ieee80211_vif *vif,
		      struct mt76_connac_sta_key_conf *sta_key_conf,
		      struct ieee80211_key_conf *key, int mcu_cmd,
		      struct mt76_wcid *wcid, enum set_key_cmd cmd)
{
	struct mt76_vif *mvif = (struct mt76_vif *)vif->drv_priv;
	struct sk_buff *skb;
	int ret;

	skb = mt76_connac_mcu_alloc_sta_req(dev, mvif, wcid);
	if (IS_ERR(skb))
		return PTR_ERR(skb);

	ret = besra_mcu_sta_key_tlv(wcid, sta_key_conf, skb, key, cmd);
	if (ret)
		return ret;

	return mt76_mcu_skb_send_msg(dev, skb, mcu_cmd, true);
}

int besra_mcu_add_dev_info(struct besra_phy *phy,
			    struct ieee80211_vif *vif, bool enable)
{
	struct besra_dev *dev = phy->dev;
	struct besra_vif *mvif = (struct besra_vif *)vif->drv_priv;
	struct {
		struct req_hdr {
			u8 omac_idx;
			u8 dbdc_idx;
			u8 __rsv[2];
		} __packed hdr;
		struct req_tlv {
			__le16 tag;
			__le16 len;
			u8 active;
			u8 __rsv;
			u8 omac_addr[ETH_ALEN];
		} __packed tlv;
	} data = {
		.hdr = {
			.omac_idx = mvif->mt76.omac_idx,
			.dbdc_idx = mvif->mt76.band_idx,
		},
		.tlv = {
			.tag = cpu_to_le16(DEV_INFO_ACTIVE),
			.len = cpu_to_le16(sizeof(struct req_tlv)),
			.active = enable,
		},
	};

	if (mvif->mt76.omac_idx >= REPEATER_BSSID_START)
		return besra_mcu_muar_config(phy, vif, false, enable);

	memcpy(data.tlv.omac_addr, vif->addr, ETH_ALEN);
	return mt76_mcu_send_msg(&dev->mt76, MCU_WMWA_UNI_CMD(DEV_INFO_UPDATE),
				 &data, sizeof(data), true);
}

static void
besra_mcu_beacon_cntdwn(struct ieee80211_vif *vif, struct sk_buff *rskb,
			 struct sk_buff *skb,
			 struct ieee80211_mutable_offsets *offs)
{
	struct bss_bcn_cntdwn_tlv *info;
	struct tlv *tlv;
	u16 tag;

	if (!offs->cntdwn_counter_offs[0])
		return;

	tag = vif->csa_active ? UNI_BSS_INFO_BCN_CSA : UNI_BSS_INFO_BCN_BCC;

	tlv = besra_mcu_add_uni_tlv(rskb, tag, sizeof(*info));

	info = (struct bss_bcn_cntdwn_tlv *)tlv;
	info->cnt = skb->data[offs->cntdwn_counter_offs[0]];
}

static void
besra_mcu_beacon_cont(struct besra_dev *dev, struct ieee80211_vif *vif,
		       struct sk_buff *rskb, struct sk_buff *skb,
		       struct bss_bcn_content_tlv *bcn,
		       struct ieee80211_mutable_offsets *offs)
{
	struct mt76_wcid *wcid = &dev->mt76.global_wcid;
	u8 *buf;

	bcn->pkt_len = cpu_to_le16(MT_TXD_SIZE + skb->len);
	bcn->tim_ie_pos = cpu_to_le16(offs->tim_offset);

	if (offs->cntdwn_counter_offs[0]) {
		u16 offset = offs->cntdwn_counter_offs[0];

		if (vif->csa_active)
			bcn->csa_ie_pos = cpu_to_le16(offset - 4);
		if (vif->color_change_active)
			bcn->bcc_ie_pos = cpu_to_le16(offset - 3);
	}

	buf = (u8 *)bcn + sizeof(*bcn) - MAX_BEACON_SIZE;
	besra_mac_write_txwi(dev, (__le32 *)buf, skb, wcid, 0, NULL,
			     true);
	memcpy(buf + MT_TXD_SIZE, skb->data, skb->len);
}

static void
besra_mcu_beacon_check_caps(struct besra_phy *phy, struct ieee80211_vif *vif,
			     struct sk_buff *skb)
{
	struct besra_vif *mvif = (struct besra_vif *)vif->drv_priv;
	struct besra_vif_cap *vc = &mvif->cap;
	const struct ieee80211_he_cap_elem *he;
	const struct ieee80211_vht_cap *vht;
	const struct ieee80211_ht_cap *ht;
	struct ieee80211_mgmt *mgmt = (struct ieee80211_mgmt *)skb->data;
	const u8 *ie;
	u32 len, bc;

	/* Check missing configuration options to allow AP mode in mac80211
	 * to remain in sync with hostapd settings, and get a subset of
	 * beacon and hardware capabilities.
	 */
	if (WARN_ON_ONCE(skb->len <= (mgmt->u.beacon.variable - skb->data)))
		return;

	memset(vc, 0, sizeof(*vc));

	len = skb->len - (mgmt->u.beacon.variable - skb->data);

	ie = cfg80211_find_ie(WLAN_EID_HT_CAPABILITY, mgmt->u.beacon.variable,
			      len);
	if (ie && ie[1] >= sizeof(*ht)) {
		ht = (void *)(ie + 2);
		vc->ht_ldpc |= !!(le16_to_cpu(ht->cap_info) &
				  IEEE80211_HT_CAP_LDPC_CODING);
	}

	ie = cfg80211_find_ie(WLAN_EID_VHT_CAPABILITY, mgmt->u.beacon.variable,
			      len);
	if (ie && ie[1] >= sizeof(*vht)) {
		u32 pc = phy->mt76->sband_5g.sband.vht_cap.cap;

		vht = (void *)(ie + 2);
		bc = le32_to_cpu(vht->vht_cap_info);

		vc->vht_ldpc |= !!(bc & IEEE80211_VHT_CAP_RXLDPC);
		vc->vht_su_ebfer =
			(bc & IEEE80211_VHT_CAP_SU_BEAMFORMER_CAPABLE) &&
			(pc & IEEE80211_VHT_CAP_SU_BEAMFORMER_CAPABLE);
		vc->vht_su_ebfee =
			(bc & IEEE80211_VHT_CAP_SU_BEAMFORMEE_CAPABLE) &&
			(pc & IEEE80211_VHT_CAP_SU_BEAMFORMEE_CAPABLE);
		vc->vht_mu_ebfer =
			(bc & IEEE80211_VHT_CAP_MU_BEAMFORMER_CAPABLE) &&
			(pc & IEEE80211_VHT_CAP_MU_BEAMFORMER_CAPABLE);
		vc->vht_mu_ebfee =
			(bc & IEEE80211_VHT_CAP_MU_BEAMFORMEE_CAPABLE) &&
			(pc & IEEE80211_VHT_CAP_MU_BEAMFORMEE_CAPABLE);
	}

	ie = cfg80211_find_ext_ie(WLAN_EID_EXT_HE_CAPABILITY,
				  mgmt->u.beacon.variable, len);
	if (ie && ie[1] >= sizeof(*he) + 1) {
		const struct ieee80211_sta_he_cap *pc =
			mt76_connac_get_he_phy_cap(phy->mt76, vif);
		const struct ieee80211_he_cap_elem *pe = &pc->he_cap_elem;

		he = (void *)(ie + 3);

		vc->he_ldpc =
			HE_PHY(CAP1_LDPC_CODING_IN_PAYLOAD, pe->phy_cap_info[1]);
		vc->he_su_ebfer =
			HE_PHY(CAP3_SU_BEAMFORMER, he->phy_cap_info[3]) &&
			HE_PHY(CAP3_SU_BEAMFORMER, pe->phy_cap_info[3]);
		vc->he_su_ebfee =
			HE_PHY(CAP4_SU_BEAMFORMEE, he->phy_cap_info[4]) &&
			HE_PHY(CAP4_SU_BEAMFORMEE, pe->phy_cap_info[4]);
		vc->he_mu_ebfer =
			HE_PHY(CAP4_MU_BEAMFORMER, he->phy_cap_info[4]) &&
			HE_PHY(CAP4_MU_BEAMFORMER, pe->phy_cap_info[4]);
	}
}

int besra_mcu_add_beacon(struct ieee80211_hw *hw,
			  struct ieee80211_vif *vif, int en)
{
	struct besra_dev *dev = besra_hw_dev(hw);
	struct besra_phy *phy = besra_hw_phy(hw);
	struct besra_vif *mvif = (struct besra_vif *)vif->drv_priv;
	struct ieee80211_mutable_offsets offs;
	struct ieee80211_tx_info *info;
	struct sk_buff *skb, *rskb;
	struct tlv *tlv;
	struct bss_bcn_content_tlv *bcn;
	u8 phy_idx = besra_get_phy_id(phy);

	rskb = __besra_mcu_alloc_bss_req(&dev->mt76, &mvif->mt76,
					 BESRA_BEACON_UPDATE_SIZE);
	if (IS_ERR(rskb))
		return PTR_ERR(rskb);

	tlv = besra_mcu_add_uni_tlv(rskb,
				    UNI_BSS_INFO_BCN_CONTENT, sizeof(*bcn));
	bcn = (struct bss_bcn_content_tlv *)tlv;
	bcn->enable = en;

	if (!en)
		goto out;

	skb = ieee80211_beacon_get_template(hw, vif, &offs);
	if (!skb)
		return -EINVAL;

	if (skb->len > MAX_BEACON_SIZE - MT_TXD_SIZE) {
		dev_err(dev->mt76.dev, "Bcn size limit exceed\n");
		dev_kfree_skb(skb);
		return -EINVAL;
	}

	info = IEEE80211_SKB_CB(skb);
	info->hw_queue |= FIELD_PREP(MT_TX_HW_QUEUE_PHY, phy_idx);

	besra_mcu_beacon_check_caps(phy, vif, skb);

	besra_mcu_beacon_cont(dev, vif, rskb, skb, bcn, &offs);
	/* TODO: subtag - 11v MBSSID */
	besra_mcu_beacon_cntdwn(vif, rskb, skb, &offs);
	dev_kfree_skb(skb);
out:
	return mt76_mcu_skb_send_msg(&phy->dev->mt76, rskb,
				     MCU_WMWA_UNI_CMD(BSS_INFO_UPDATE), true);
}

static int besra_driver_own(struct besra_dev *dev, u8 band)
{
	mt76_wr(dev, MT_TOP_LPCR_HOST_BAND(band), MT_TOP_LPCR_HOST_DRV_OWN);
	if (!mt76_poll_msec(dev, MT_TOP_LPCR_HOST_BAND(band),
			    MT_TOP_LPCR_HOST_FW_OWN_STAT, 0, 500)) {
		dev_err(dev->mt76.dev, "Timeout for driver own\n");
		return -EIO;
	}

	/* clear irq when the driver own success */
	mt76_wr(dev, MT_TOP_LPCR_HOST_BAND_IRQ_STAT(band),
		MT_TOP_LPCR_HOST_BAND_STAT);

	return 0;
}

static int besra_load_patch(struct besra_dev *dev)
{
	const struct besra_patch_hdr *hdr;
	const struct firmware *fw = NULL;
	int i, ret, sem;

	sem = mt76_connac_mcu_patch_sem_ctrl(&dev->mt76, 1);
	switch (sem) {
	case PATCH_IS_DL:
		return 0;
	case PATCH_NOT_DL_SEM_SUCCESS:
		break;
	default:
		dev_err(dev->mt76.dev, "Failed to get patch semaphore\n");
		return -EAGAIN;
	}

	ret = request_firmware(&fw, MT7902_ROM_PATCH,
			       dev->mt76.dev);
	if (ret)
		goto out;

	if (!fw || !fw->data || fw->size < sizeof(*hdr)) {
		dev_err(dev->mt76.dev, "Invalid firmware\n");
		ret = -EINVAL;
		goto out;
	}

	hdr = (const struct besra_patch_hdr *)(fw->data);

	dev_info(dev->mt76.dev, "HW/SW Version: 0x%x, Build Time: %.16s\n",
		 be32_to_cpu(hdr->hw_sw_ver), hdr->build_date);

	for (i = 0; i < be32_to_cpu(hdr->desc.n_region); i++) {
		struct besra_patch_sec *sec;
		const u8 *dl;
		u32 len, addr;

		sec = (struct besra_patch_sec *)(fw->data + sizeof(*hdr) +
						  i * sizeof(*sec));
		if ((be32_to_cpu(sec->type) & PATCH_SEC_TYPE_MASK) !=
		    PATCH_SEC_TYPE_INFO) {
			ret = -EINVAL;
			goto out;
		}

		addr = be32_to_cpu(sec->info.addr);
		len = be32_to_cpu(sec->info.len);
		dl = fw->data + be32_to_cpu(sec->offs);

		ret = mt76_connac_mcu_init_download(&dev->mt76, addr, len,
						    DL_MODE_NEED_RSP);
		if (ret) {
			dev_err(dev->mt76.dev, "Download request failed\n");
			goto out;
		}

		ret = __mt76_mcu_send_firmware(&dev->mt76, MCU_CMD(FW_SCATTER),
					       dl, len, 4096);
		if (ret) {
			dev_err(dev->mt76.dev, "Failed to send patch\n");
			goto out;
		}
	}

	ret = mt76_connac_mcu_start_patch(&dev->mt76);
	if (ret)
		dev_err(dev->mt76.dev, "Failed to start patch\n");

out:
	sem = mt76_connac_mcu_patch_sem_ctrl(&dev->mt76, 0);
	switch (sem) {
	case PATCH_REL_SEM_SUCCESS:
		break;
	default:
		ret = -EAGAIN;
		dev_err(dev->mt76.dev, "Failed to release patch semaphore\n");
		break;
	}
	release_firmware(fw);

	return ret;
}

static int
besra_mcu_send_ram_firmware(struct besra_dev *dev,
			     const struct besra_fw_trailer *hdr,
			     const u8 *data, bool is_wa)
{
	int i, offset = 0;
	u32 override = 0, option = 0;

	for (i = 0; i < hdr->n_region; i++) {
		const struct besra_fw_region *region;
		int err;
		u32 len, addr, mode;

		region = (const struct besra_fw_region *)((const u8 *)hdr -
			 (hdr->n_region - i) * sizeof(*region));
		mode = mt76_connac_mcu_gen_dl_mode(&dev->mt76,
						   region->feature_set, is_wa);
		len = le32_to_cpu(region->len);
		addr = le32_to_cpu(region->addr);

		if (region->feature_set & FW_FEATURE_OVERRIDE_ADDR)
			override = addr;

		err = mt76_connac_mcu_init_download(&dev->mt76, addr, len,
						    mode);
		if (err) {
			dev_err(dev->mt76.dev, "Download request failed\n");
			return err;
		}

		err = __mt76_mcu_send_firmware(&dev->mt76, MCU_CMD(FW_SCATTER),
					       data + offset, len, 4096);
		if (err) {
			dev_err(dev->mt76.dev, "Failed to send firmware.\n");
			return err;
		}

		offset += len;
	}

	if (override)
		option |= FW_START_OVERRIDE;

	if (is_wa)
		option |= FW_START_WORKING_PDA_CR4;

	return mt76_connac_mcu_start_firmware(&dev->mt76, override, option);
}

static int besra_load_ram(struct besra_dev *dev)
{
	const struct besra_fw_trailer *hdr;
	const struct firmware *fw;
	int ret;

	ret = request_firmware(&fw, MT7902_FIRMWARE_WM,
			       dev->mt76.dev);
	if (ret)
		return ret;

	if (!fw || !fw->data || fw->size < sizeof(*hdr)) {
		dev_err(dev->mt76.dev, "Invalid firmware\n");
		ret = -EINVAL;
		goto out;
	}

	hdr = (const struct besra_fw_trailer *)(fw->data + fw->size -
					sizeof(*hdr));

	dev_info(dev->mt76.dev, "WM Firmware Version: %.10s, Build Time: %.15s\n",
		 hdr->fw_ver, hdr->build_date);

	ret = besra_mcu_send_ram_firmware(dev, hdr, fw->data, false);
	if (ret) {
		dev_err(dev->mt76.dev, "Failed to start WM firmware\n");
		goto out;
	}

	release_firmware(fw);

	ret = request_firmware(&fw, MT7902_FIRMWARE_WA,
			       dev->mt76.dev);
	if (ret)
		return ret;

	if (!fw || !fw->data || fw->size < sizeof(*hdr)) {
		dev_err(dev->mt76.dev, "Invalid firmware\n");
		ret = -EINVAL;
		goto out;
	}

	hdr = (const struct besra_fw_trailer *)(fw->data + fw->size -
					sizeof(*hdr));

	dev_info(dev->mt76.dev, "WA Firmware Version: %.10s, Build Time: %.15s\n",
		 hdr->fw_ver, hdr->build_date);

	ret = besra_mcu_send_ram_firmware(dev, hdr, fw->data, true);
	if (ret) {
		dev_err(dev->mt76.dev, "Failed to start WA firmware\n");
		goto out;
	}

	snprintf(dev->mt76.hw->wiphy->fw_version,
		 sizeof(dev->mt76.hw->wiphy->fw_version),
		 "%.10s-%.15s", hdr->fw_ver, hdr->build_date);

out:
	release_firmware(fw);

	return ret;
}

static int
besra_firmware_state(struct besra_dev *dev, bool wa)
{
	u32 state = FIELD_PREP(MT_TOP_MISC_FW_STATE,
			       wa ? FW_STATE_RDY : FW_STATE_FW_DOWNLOAD);

	if (!mt76_poll_msec(dev, MT_TOP_MISC, MT_TOP_MISC_FW_STATE,
			    state, 1000)) {
		dev_err(dev->mt76.dev, "Timeout for initializing firmware\n");
		return -EIO;
	}
	return 0;
}

static int besra_load_firmware(struct besra_dev *dev)
{
	int ret;

	/* make sure fw is download state */
	if (besra_firmware_state(dev, false)) {
		/* restart firmware once */
		__mt76_mcu_restart(&dev->mt76);
		ret = besra_firmware_state(dev, false);
		if (ret) {
			dev_err(dev->mt76.dev,
				"Firmware is not ready for download\n");
			return ret;
		}
	}

	ret = besra_load_patch(dev);
	if (ret)
		return ret;

	ret = besra_load_ram(dev);
	if (ret)
		return ret;

	ret = besra_firmware_state(dev, true);
	if (ret)
		return ret;

	mt76_queue_tx_cleanup(dev, dev->mt76.q_mcu[MT_MCUQ_FWDL], false);

	dev_dbg(dev->mt76.dev, "Firmware init done\n");

	return 0;
}

int besra_mcu_fw_log_2_host(struct besra_dev *dev, u8 type, u8 ctrl)
{
	struct {
		u8 _rsv[4];

		__le16 tag;
		__le16 len;
		u8 ctrl;
		u8 interval;
		u8 _rsv2[2];
	} __packed data = {
		.tag = cpu_to_le16(UNI_WSYS_CONFIG_FW_LOG_CTRL),
		.len = cpu_to_le16(sizeof(data) - 4),
		.ctrl = ctrl,
	};

	if (type == MCU_FW_LOG_WA)
		return mt76_mcu_send_msg(&dev->mt76, MCU_WA_UNI_CMD(WSYS_CONFIG),
					 &data, sizeof(data), true);

	return mt76_mcu_send_msg(&dev->mt76, MCU_WM_UNI_CMD(WSYS_CONFIG), &data,
				 sizeof(data), true);
}

int besra_mcu_fw_dbg_ctrl(struct besra_dev *dev, u32 module, u8 level)
{
	struct {
		u8 _rsv[4];

		__le16 tag;
		__le16 len;
		__le32 module_idx;
		u8 level;
		u8 _rsv2[3];
	} data = {
		.tag = cpu_to_le16(UNI_WSYS_CONFIG_FW_DBG_CTRL),
		.len = cpu_to_le16(sizeof(data) - 4),
		.module_idx = cpu_to_le32(module),
		.level = level,
	};

	return mt76_mcu_send_msg(&dev->mt76, MCU_WM_UNI_CMD(WSYS_CONFIG), &data,
				 sizeof(data), false);
}

int besra_mcu_muru_debug_set(struct besra_dev *dev, bool enabled)
{
	struct {
		__le32 cmd;
		u8 enable;
	} data = {
		.cmd = cpu_to_le32(MURU_SET_TXC_TX_STATS_EN),
		.enable = enabled,
	};

	return mt76_mcu_send_msg(&dev->mt76, MCU_EXT_CMD(MURU_CTRL), &data,
				sizeof(data), false);
}

int besra_mcu_muru_debug_get(struct besra_phy *phy, void *ms)
{
	struct besra_dev *dev = phy->dev;
	struct sk_buff *skb;
	struct besra_mcu_muru_stats *mu_stats =
				(struct besra_mcu_muru_stats *)ms;
	int ret;

	struct {
		__le32 cmd;
		u8 band_idx;
	} req = {
		.cmd = cpu_to_le32(MURU_GET_TXC_TX_STATS),
		.band_idx = phy->band_idx,
	};

	ret = mt76_mcu_send_and_get_msg(&dev->mt76, MCU_EXT_CMD(MURU_CTRL),
					&req, sizeof(req), true, &skb);
	if (ret)
		return ret;

	memcpy(mu_stats, skb->data, sizeof(struct besra_mcu_muru_stats));
	dev_kfree_skb(skb);

	return 0;
}

static int besra_mcu_set_mwds(struct besra_dev *dev, bool enabled)
{
	struct {
		u8 enable;
		u8 _rsv[3];
	} __packed req = {
		.enable = enabled
	};

	return mt76_mcu_send_msg(&dev->mt76, MCU_WA_EXT_CMD(MWDS_SUPPORT), &req,
				 sizeof(req), false);
}

int besra_mcu_set_muru_ctrl(struct besra_dev *dev, u32 cmd, u32 val)
{
	struct {
		__le32 cmd;
		u8 val[4];
	} __packed req = {
		.cmd = cpu_to_le32(cmd),
	};

	put_unaligned_le32(val, req.val);

	return mt76_mcu_send_msg(&dev->mt76, MCU_EXT_CMD(MURU_CTRL), &req,
				 sizeof(req), false);
}

static int
besra_mcu_init_rx_airtime(struct besra_dev *dev)
{
#define RX_AIRTIME_FEATURE_CTRL		1
#define RX_AIRTIME_BITWISE_CTRL		2
#define RX_AIRTIME_CLEAR_EN	1
	struct {
		__le16 field;
		__le16 sub_field;
		__le32 set_status;
		__le32 get_status;
		u8 _rsv[12];

		bool airtime_en;
		bool mibtime_en;
		bool earlyend_en;
		u8 _rsv1[9];

		bool airtime_clear;
		bool mibtime_clear;
		u8 _rsv2[98];
	} __packed req = {
		.field = cpu_to_le16(RX_AIRTIME_BITWISE_CTRL),
		.sub_field = cpu_to_le16(RX_AIRTIME_CLEAR_EN),
		.airtime_clear = true,
	};
	int ret;

	ret = mt76_mcu_send_msg(&dev->mt76, MCU_EXT_CMD(RX_AIRTIME_CTRL), &req,
				sizeof(req), true);
	if (ret)
		return ret;

	req.field = cpu_to_le16(RX_AIRTIME_FEATURE_CTRL);
	req.sub_field = cpu_to_le16(RX_AIRTIME_CLEAR_EN);
	req.airtime_en = true;

	return mt76_mcu_send_msg(&dev->mt76, MCU_EXT_CMD(RX_AIRTIME_CTRL), &req,
				 sizeof(req), true);
}

static int
besra_load_rom(struct besra_dev *dev, bool is_sram)
{
#define MCU_FIRMWARE_ROM_ADDR		0x00800000
#define MCU_FIRMWARE_ROM_SRAM_ADDR	0xE0048000

	const struct firmware *fw;
	int ret;
	u32 val, ofs = 0;

	ret = request_firmware(&fw, (is_sram ? MT7902_FIRMWARE_ROM_SRAM :
			       MT7902_FIRMWARE_ROM),
			       dev->mt76.dev);
	if (ret) {
		dev_err(dev->mt76.dev, "request rom binary failed\n");
		return ret;
	}

	if (!fw || !fw->data ) {
		dev_err(dev->mt76.dev, "Invalid firmware\n");
		ret = -EINVAL;
		goto out;
	}

	val = mt76_rr(dev, MT_INFRA_BUS_ON_REMAP_WF_5_4);

	mt76_wr(dev, MT_INFRA_BUS_ON_REMAP_WF_5_4,
		FIELD_GET(MT_INFRA_BUS_ON_REMAP_WF_4_MASK, val) |
		FIELD_PREP(MT_INFRA_BUS_ON_REMAP_WF_5_MASK, 0x1850));

	while (true) {
		u32 size;

		if (ofs >= fw->size)
			break;

		if ((ofs + 0x10000) <= fw->size)
			size = 0x10000;
		else
			size = fw->size - ofs;

		mt76_wr(dev, MT_MCU_BUS_REMAP,
			((is_sram ? MCU_FIRMWARE_ROM_SRAM_ADDR : MCU_FIRMWARE_ROM_ADDR) + ofs));
		mt76_wr_copy(dev, 0x50000, (void *) (fw->data + ofs), size);

		ofs += 0x10000;
	}

	mt76_wr(dev, MT_INFRA_BUS_ON_REMAP_WF_5_4, val);

out:
	release_firmware(fw);

	return ret;

}

int besra_rom_start(struct besra_dev *dev)
{
#define WF_IDLE			0xBE11
#define WF_STATE_MASK		GENMASK(15, 0)

	int ret;
	u32 val;

	ret = !mt76_poll_msec(dev, MT_TOP_CFG_ON_ROM_IDX,
			      MT_TOP_CFG_ON_ROM_STATE_MASK,
			      FIELD_PREP(MT_TOP_CFG_ON_ROM_STATE_MASK,
					 MT_TOP_CFG_ON_ROM_IDLE), 200);
	if (!ret)
		return ret;

	mt76_rmw(dev, MT_INFRA_RGU_RGU_ON_SW_RST_B,
		 MT_INFRA_RGU_RGU_ON_SW_RST_B_MASK, 0);

	ret = besra_load_rom(dev, false);
	if (ret)
		return ret;

	ret = besra_load_rom(dev, true);
	if (ret)
		return ret;

	mt76_rmw(dev, MT_INFRA_RGU_RGU_ON_SW_RST_B,
		 MT_INFRA_RGU_RGU_ON_SW_RST_B_MASK, 1);

	ret = !mt76_poll_msec(dev, MT_TOP_CFG_ON_ROM_IDX,
			      MT_TOP_CFG_ON_ROM_STATE_MASK,
			      FIELD_PREP(MT_TOP_CFG_ON_ROM_STATE_MASK,
					 MT_TOP_CFG_ON_ROM_IDLE), 200);
	if (ret)
		return ret;

	val = mt76_rr(dev, MT_HIF_REMAP_L1);

	mt76_wr(dev, MT_HIF_REMAP_L1,
		FIELD_GET(MT_HIF_REMAP_L1_OFFSET, val) | FIELD_PREP(MT_HIF_REMAP_L1_BASE, 0x1805));
	if (!mt76_poll_msec(dev, 0x54A68, WF_STATE_MASK,
		    FIELD_PREP(WF_STATE_MASK, WF_IDLE), 1000)) {
		dev_err(dev->mt76.dev, "timeout for wf idle\n");
		ret = -EIO;
	}

	mt76_wr(dev, MT_HIF_REMAP_L1, val);

	return ret;
}

int besra_mcu_init(struct besra_dev *dev)
{
	static const struct mt76_mcu_ops besra_mcu_ops = {
		.headroom = sizeof(struct besra_mcu_txd),
		.mcu_skb_send_msg = besra_mcu_send_message,
		.mcu_parse_response = besra_mcu_parse_response,
		.mcu_restart = mt76_connac_mcu_restart,
	};
	int ret;

	dev->mt76.mcu_ops = &besra_mcu_ops;

	/* force firmware operation mode into normal state,
	 * which should be set before firmware download stage.
	 */
	mt76_wr(dev, MT_SWDEF_MODE, MT_SWDEF_NORMAL_MODE);

	ret = besra_driver_own(dev, 0);
	if (ret)
		return ret;
	/* set driver own for band1 when two hif exist */
	if (dev->hif2) {
		ret = besra_driver_own(dev, 1);
		if (ret)
			return ret;
	}

	ret = besra_load_firmware(dev);
	if (ret)
		return ret;

	set_bit(MT76_STATE_MCU_RUNNING, &dev->mphy.state);
	ret = besra_mcu_fw_log_2_host(dev, MCU_FW_LOG_WM, 0);
	if (ret)
		return ret;

	ret = besra_mcu_fw_log_2_host(dev, MCU_FW_LOG_WA, 0);
	if (ret)
		return ret;

	ret = besra_mcu_set_mwds(dev, 1);
	if (ret)
		return ret;

	ret = besra_mcu_set_muru_ctrl(dev, MURU_SET_PLATFORM_TYPE,
				       MURU_PLATFORM_TYPE_PERF_LEVEL_2);
	if (ret)
		return ret;

	ret = besra_mcu_init_rx_airtime(dev);
	if (ret)
		return ret;

	return besra_mcu_wa_cmd(dev, MCU_WA_PARAM_CMD(SET),
				 MCU_WA_PARAM_RED, 0, 0);
}

void besra_mcu_exit(struct besra_dev *dev)
{
	__mt76_mcu_restart(&dev->mt76);
	if (besra_firmware_state(dev, false)) {
		dev_err(dev->mt76.dev, "Failed to exit mcu\n");
		return;
	}

	mt76_wr(dev, MT_TOP_LPCR_HOST_BAND(0), MT_TOP_LPCR_HOST_FW_OWN);
	if (dev->hif2)
		mt76_wr(dev, MT_TOP_LPCR_HOST_BAND(1),
			MT_TOP_LPCR_HOST_FW_OWN);
	skb_queue_purge(&dev->mt76.mcu.res_q);
}

int besra_mcu_set_hdr_trans(struct besra_dev *dev, bool hdr_trans)
{
	struct {
		u8 __rsv[4];
	} __packed hdr;
	struct hdr_trans_blacklist *req_blacklist;
	struct hdr_trans_en *req_en;
	struct sk_buff *skb;
	struct tlv *tlv;
	int len = BESRA_HDR_TRANS_MAX_SIZE + sizeof(hdr);
	int ret;

	skb = mt76_mcu_msg_alloc(&dev->mt76, NULL, len);
	if (!skb)
		return ERR_PTR(-ENOMEM);

	skb_put_data(skb, &hdr, sizeof(hdr));

	tlv = besra_mcu_add_uni_tlv(skb, UNI_HDR_TRANS_EN, sizeof(*req_en));
	req_en = (struct hdr_trans_en *)tlv;
	req_en->enable = hdr_trans;

	tlv = besra_mcu_add_uni_tlv(skb, UNI_HDR_TRANS_VLAN,
				    sizeof(struct hdr_trans_vlan));

	if (hdr_trans) {
		tlv = besra_mcu_add_uni_tlv(skb, UNI_HDR_TRANS_BLACKLIST,
					    sizeof(*req_blacklist));
		req_blacklist = (struct hdr_trans_blacklist *)tlv;
		req_blacklist->enable = 1;
		req_blacklist->type = cpu_to_le16(ETH_P_PAE);
	}

	return mt76_mcu_skb_send_msg(&dev->mt76, skb,
				     MCU_WM_UNI_CMD(RX_HDR_TRANS), true);
}

int besra_mcu_set_tx(struct besra_dev *dev, struct ieee80211_vif *vif)
{
	struct besra_vif *mvif = (struct besra_vif *)vif->drv_priv;
	struct {
		u8 bss_idx;
		u8 __rsv[3];
	} __packed hdr = {
		.bss_idx = mvif->mt76.idx,
	};
	struct sk_buff *skb;
	int len = sizeof(hdr) + IEEE80211_NUM_ACS * sizeof(struct edca);
	int ac;

	skb = mt76_mcu_msg_alloc(&dev->mt76, NULL, len);
	if (!skb)
		return ERR_PTR(-ENOMEM);

	skb_put_data(skb, &hdr, sizeof(hdr));

	for (ac = 0; ac < IEEE80211_NUM_ACS; ac++) {
		struct ieee80211_tx_queue_params *q = &mvif->queue_params[ac];
		struct edca *e;
		struct tlv *tlv;

		tlv = besra_mcu_add_uni_tlv(skb, MCU_EDCA_AC_PARAM, sizeof(*e));

		e = (struct edca *)tlv;
		e->set = WMM_PARAM_SET;
		e->queue = ac + mvif->mt76.wmm_idx * BESRA_MAX_WMM_SETS;
		e->aifs = q->aifs;
		e->txop = cpu_to_le16(q->txop);

		if (q->cw_min)
			e->cw_min = fls(q->cw_min);
		else
			e->cw_min = 5;

		if (q->cw_max)
			e->cw_max = cpu_to_le16(fls(q->cw_max));
		else
			e->cw_max = cpu_to_le16(10);
	}
	return mt76_mcu_skb_send_msg(&dev->mt76, skb,
				     MCU_WM_UNI_CMD(EDCA), true);
}

int besra_mcu_set_fcc5_lpn(struct besra_dev *dev, int val)
{
	struct {
		__le32 tag;
		__le16 min_lpn;
		u8 rsv[2];
	} __packed req = {
		.tag = cpu_to_le32(0x1),
		.min_lpn = cpu_to_le16(val),
	};

	return mt76_mcu_send_msg(&dev->mt76, MCU_EXT_CMD(SET_RDD_TH), &req,
				 sizeof(req), true);
}

int besra_mcu_set_pulse_th(struct besra_dev *dev,
			    const struct besra_dfs_pulse *pulse)
{
	struct {
		__le32 tag;

		__le32 max_width;		/* us */
		__le32 max_pwr;			/* dbm */
		__le32 min_pwr;			/* dbm */
		__le32 min_stgr_pri;		/* us */
		__le32 max_stgr_pri;		/* us */
		__le32 min_cr_pri;		/* us */
		__le32 max_cr_pri;		/* us */
	} __packed req = {
		.tag = cpu_to_le32(0x3),

#define __req_field(field) .field = cpu_to_le32(pulse->field)
		__req_field(max_width),
		__req_field(max_pwr),
		__req_field(min_pwr),
		__req_field(min_stgr_pri),
		__req_field(max_stgr_pri),
		__req_field(min_cr_pri),
		__req_field(max_cr_pri),
#undef __req_field
	};

	return mt76_mcu_send_msg(&dev->mt76, MCU_EXT_CMD(SET_RDD_TH), &req,
				 sizeof(req), true);
}

int besra_mcu_set_radar_th(struct besra_dev *dev, int index,
			    const struct besra_dfs_pattern *pattern)
{
	struct {
		__le32 tag;
		__le16 radar_type;

		u8 enb;
		u8 stgr;
		u8 min_crpn;
		u8 max_crpn;
		u8 min_crpr;
		u8 min_pw;
		__le32 min_pri;
		__le32 max_pri;
		u8 max_pw;
		u8 min_crbn;
		u8 max_crbn;
		u8 min_stgpn;
		u8 max_stgpn;
		u8 min_stgpr;
		u8 rsv[2];
		__le32 min_stgpr_diff;
	} __packed req = {
		.tag = cpu_to_le32(0x2),
		.radar_type = cpu_to_le16(index),

#define __req_field_u8(field) .field = pattern->field
#define __req_field_u32(field) .field = cpu_to_le32(pattern->field)
		__req_field_u8(enb),
		__req_field_u8(stgr),
		__req_field_u8(min_crpn),
		__req_field_u8(max_crpn),
		__req_field_u8(min_crpr),
		__req_field_u8(min_pw),
		__req_field_u32(min_pri),
		__req_field_u32(max_pri),
		__req_field_u8(max_pw),
		__req_field_u8(min_crbn),
		__req_field_u8(max_crbn),
		__req_field_u8(min_stgpn),
		__req_field_u8(max_stgpn),
		__req_field_u8(min_stgpr),
		__req_field_u32(min_stgpr_diff),
#undef __req_field_u8
#undef __req_field_u32
	};

	return mt76_mcu_send_msg(&dev->mt76, MCU_EXT_CMD(SET_RDD_TH), &req,
				 sizeof(req), true);
}

static int
besra_mcu_background_chain_ctrl(struct besra_phy *phy,
				 struct cfg80211_chan_def *chandef,
				 int cmd)
{
	struct besra_dev *dev = phy->dev;
	struct mt76_phy *mphy = phy->mt76;
	struct ieee80211_channel *chan = mphy->chandef.chan;
	int freq = mphy->chandef.center_freq1;
	struct besra_mcu_background_chain_ctrl req = {
		.monitor_scan_type = 2, /* simple rx */
	};

	if (!chandef && cmd != CH_SWITCH_BACKGROUND_SCAN_STOP)
		return -EINVAL;

	if (!cfg80211_chandef_valid(&mphy->chandef))
		return -EINVAL;

	switch (cmd) {
	case CH_SWITCH_BACKGROUND_SCAN_START: {
		req.chan = chan->hw_value;
		req.central_chan = ieee80211_frequency_to_channel(freq);
		req.bw = mt76_connac_chan_bw(&mphy->chandef);
		req.monitor_chan = chandef->chan->hw_value;
		req.monitor_central_chan =
			ieee80211_frequency_to_channel(chandef->center_freq1);
		req.monitor_bw = mt76_connac_chan_bw(chandef);
		req.band_idx = phy->band_idx;
		req.scan_mode = 1;
		break;
	}
	case CH_SWITCH_BACKGROUND_SCAN_RUNNING:
		req.monitor_chan = chandef->chan->hw_value;
		req.monitor_central_chan =
			ieee80211_frequency_to_channel(chandef->center_freq1);
		req.band_idx = phy->band_idx;
		req.scan_mode = 2;
		break;
	case CH_SWITCH_BACKGROUND_SCAN_STOP:
		req.chan = chan->hw_value;
		req.central_chan = ieee80211_frequency_to_channel(freq);
		req.bw = mt76_connac_chan_bw(&mphy->chandef);
		req.tx_stream = hweight8(mphy->antenna_mask);
		req.rx_stream = mphy->antenna_mask;
		break;
	default:
		return -EINVAL;
	}
	req.band = chandef ? chandef->chan->band == NL80211_BAND_5GHZ : 1;

	return mt76_mcu_send_msg(&dev->mt76, MCU_EXT_CMD(OFFCH_SCAN_CTRL),
				 &req, sizeof(req), false);
}

int besra_mcu_rdd_background_enable(struct besra_phy *phy,
				     struct cfg80211_chan_def *chandef)
{
	struct besra_dev *dev = phy->dev;
	int err, region;

	if (!chandef) { /* disable offchain */
		err = besra_mcu_rdd_cmd(dev, RDD_STOP, MT_RX_SEL2,
					0, 0);
		if (err)
			return err;

		return besra_mcu_background_chain_ctrl(phy, NULL,
				CH_SWITCH_BACKGROUND_SCAN_STOP);
	}

	err = besra_mcu_background_chain_ctrl(phy, chandef,
					       CH_SWITCH_BACKGROUND_SCAN_START);
	if (err)
		return err;

	switch (dev->mt76.region) {
	case NL80211_DFS_ETSI:
		region = 0;
		break;
	case NL80211_DFS_JP:
		region = 2;
		break;
	case NL80211_DFS_FCC:
	default:
		region = 1;
		break;
	}

	return besra_mcu_rdd_cmd(dev, RDD_START, MT_RX_SEL2,
				 0, region);
}

int besra_mcu_set_chan_info(struct besra_phy *phy, int tag)
{
	static const u8 ch_band[] = {
		[NL80211_BAND_2GHZ] = 0,
		[NL80211_BAND_5GHZ] = 1,
		[NL80211_BAND_6GHZ] = 2,
	};
	struct besra_dev *dev = phy->dev;
	struct cfg80211_chan_def *chandef = &phy->mt76->chandef;
	int freq1 = chandef->center_freq1;
	u8 phy_idx = besra_get_phy_id(phy);
	u16 chainshift;
	struct {
		/* fixed field */
		u8 __rsv[4];

		/* TLV*/
		__u16 tag;
		__u16 len;
		u8 control_ch;
		u8 center_ch;
		u8 bw;
		u8 tx_streams_num;
		u8 rx_streams;	/* mask or num */
		u8 switch_reason;
		u8 band_idx;
		u8 center_ch2;	/* for 80+80 only */
		__le16 cac_case;
		u8 channel_band;
		u8 rsv0;
		__le32 outband_freq;
		u8 txpower_drop;
		u8 ap_bw;
		u8 ap_center_ch;
		u8 rsv1[53];
	} __packed req = {
		.tag = cpu_to_le16(tag),
		.len = cpu_to_le16(sizeof(req) - 4),
		.control_ch = chandef->chan->hw_value,
		.center_ch = ieee80211_frequency_to_channel(freq1),
		.bw = mt76_connac_chan_bw(chandef),
		.tx_streams_num = hweight8(phy->mt76->antenna_mask),
		.rx_streams = phy->mt76->antenna_mask,
		.band_idx = phy->band_idx,
		.channel_band = ch_band[chandef->chan->band],
	};

#ifdef CONFIG_NL80211_TESTMODE
	if (phy->mt76->test.tx_antenna_mask &&
	    (phy->mt76->test.state == MT76_TM_STATE_TX_FRAMES ||
	     phy->mt76->test.state == MT76_TM_STATE_RX_FRAMES ||
	     phy->mt76->test.state == MT76_TM_STATE_TX_CONT)) {
		req.tx_streams_num = fls(phy->mt76->test.tx_antenna_mask);
		req.rx_streams = phy->mt76->test.tx_antenna_mask;

		if (phy_idx == MT_MAIN_PHY)
			chainshift = 0;
		else if (phy_idx == MT_EXT_PHY)
			chainshift = dev->chain_shift_ext;
		else
			chainshift = dev->chain_shift_tri;

		req.rx_streams >>= chainshift;
	}
#endif

	if (tag == UNI_CHANNEL_RX_PATH ||
	    dev->mt76.hw->conf.flags & IEEE80211_CONF_MONITOR)
		req.switch_reason = CH_SWITCH_NORMAL;
	else if (phy->mt76->hw->conf.flags & IEEE80211_CONF_OFFCHANNEL)
		req.switch_reason = CH_SWITCH_SCAN_BYPASS_DPD;
	else if (!cfg80211_reg_can_beacon(phy->mt76->hw->wiphy, chandef,
					  NL80211_IFTYPE_AP))
		req.switch_reason = CH_SWITCH_DFS;
	else
		req.switch_reason = CH_SWITCH_NORMAL;

	if (tag == UNI_CHANNEL_SWITCH)
		req.rx_streams = hweight8(req.rx_streams);

	if (chandef->width == NL80211_CHAN_WIDTH_80P80) {
		int freq2 = chandef->center_freq2;

		req.center_ch2 = ieee80211_frequency_to_channel(freq2);
	}

	return mt76_mcu_send_msg(&dev->mt76, MCU_WMWA_UNI_CMD(CHANNEL_SWITCH),
				 &req, sizeof(req), true);
}

static int besra_mcu_set_eeprom_flash(struct besra_dev *dev)
{
#define MAX_PAGE_IDX_MASK	GENMASK(7, 5)
#define PAGE_IDX_MASK		GENMASK(4, 2)
#define PER_PAGE_SIZE		0x400
	struct besra_mcu_eeprom req = {
		.tag = cpu_to_le16(UNI_EFUSE_BUFFER_MODE),
		.buffer_mode = EE_MODE_BUFFER
	};
	u16 eeprom_size = BESRA_EEPROM_SIZE;
	u8 total = DIV_ROUND_UP(eeprom_size, PER_PAGE_SIZE);
	u8 *eep = (u8 *)dev->mt76.eeprom.data;
	int eep_len, i;

	for (i = 0; i < total; i++, eep += eep_len) {
		struct sk_buff *skb;
		int ret, msg_len;

		if (i == total - 1 && !!(eeprom_size % PER_PAGE_SIZE))
			eep_len = eeprom_size % PER_PAGE_SIZE;
		else
			eep_len = PER_PAGE_SIZE;

		msg_len = sizeof(req) + eep_len;
		skb = mt76_mcu_msg_alloc(&dev->mt76, NULL, msg_len);
		if (!skb)
			return -ENOMEM;

		req.len = cpu_to_le16(msg_len - 4);
		req.format = FIELD_PREP(MAX_PAGE_IDX_MASK, total - 1) |
			     FIELD_PREP(PAGE_IDX_MASK, i) | EE_FORMAT_WHOLE;
		req.buf_len = cpu_to_le16(eep_len);

		skb_put_data(skb, &req, sizeof(req));
		skb_put_data(skb, eep, eep_len);

		ret = mt76_mcu_skb_send_msg(&dev->mt76, skb,
					    MCU_WM_UNI_CMD(EFUSE_CTRL), true);
		if (ret)
			return ret;
	}

	return 0;
}

int besra_mcu_set_eeprom(struct besra_dev *dev)
{
	struct besra_mcu_eeprom req = {
		.tag = cpu_to_le16(UNI_EFUSE_BUFFER_MODE),
		.len = cpu_to_le16(sizeof(req) - 4),
		.buffer_mode = EE_MODE_EFUSE,
		.format = EE_FORMAT_WHOLE
	};

	if (dev->flash_mode)
		return besra_mcu_set_eeprom_flash(dev);

	return mt76_mcu_send_msg(&dev->mt76, MCU_WM_UNI_CMD(EFUSE_CTRL),
				 &req, sizeof(req), true);
}

int besra_mcu_get_eeprom(struct besra_dev *dev, u32 offset)
{
	struct besra_mcu_eeprom_info req = {
		.addr = cpu_to_le32(round_down(offset,
				    BESRA_EEPROM_BLOCK_SIZE)),
	};
	struct besra_mcu_eeprom_info *res;
	struct sk_buff *skb;
	int ret;
	u8 *buf;

	ret = mt76_mcu_send_and_get_msg(&dev->mt76, MCU_EXT_QUERY(EFUSE_ACCESS), &req,
				sizeof(req), true, &skb);
	if (ret)
		return ret;

	res = (struct besra_mcu_eeprom_info *)skb->data;
	buf = dev->mt76.eeprom.data + le32_to_cpu(res->addr);
	memcpy(buf, res->data, BESRA_EEPROM_BLOCK_SIZE);
	dev_kfree_skb(skb);

	return 0;
}

int besra_mcu_get_eeprom_free_block(struct besra_dev *dev, u8 *block_num)
{
	struct {
		u8 _rsv;
		u8 version;
		u8 die_idx;
		u8 _rsv2;
	} __packed req = {
		.version = 1,
	};
	struct sk_buff *skb;
	int ret;

	ret = mt76_mcu_send_and_get_msg(&dev->mt76, MCU_EXT_QUERY(EFUSE_FREE_BLOCK), &req,
					sizeof(req), true, &skb);
	if (ret)
		return ret;

	*block_num = *(u8 *)skb->data;
	dev_kfree_skb(skb);

	return 0;
}

static int besra_mcu_set_pre_cal(struct besra_dev *dev, u8 idx,
				  u8 *data, u32 len, int cmd)
{
	struct {
		u8 dir;
		u8 valid;
		__le16 bitmap;
		s8 precal;
		u8 action;
		u8 band;
		u8 idx;
		u8 rsv[4];
		__le32 len;
	} req = {};
	struct sk_buff *skb;

	skb = mt76_mcu_msg_alloc(&dev->mt76, NULL, sizeof(req) + len);
	if (!skb)
		return -ENOMEM;

	req.idx = idx;
	req.len = cpu_to_le32(len);
	skb_put_data(skb, &req, sizeof(req));
	skb_put_data(skb, data, len);

	return mt76_mcu_skb_send_msg(&dev->mt76, skb, cmd, false);
}

int besra_mcu_apply_group_cal(struct besra_dev *dev)
{
	u8 idx = 0, *cal = dev->cal, *eep = dev->mt76.eeprom.data;
	u32 total = MT_EE_CAL_GROUP_SIZE;

	if (1 || !(eep[MT_EE_DO_PRE_CAL] & MT_EE_WIFI_CAL_GROUP))
		return 0;

	/*
	 * Items: Rx DCOC, RSSI DCOC, Tx TSSI DCOC, Tx LPFG
	 * Tx FDIQ, Tx DCIQ, Rx FDIQ, Rx FIIQ, ADCDCOC
	 */
	while (total > 0) {
		int ret, len;

		len = min_t(u32, total, MT_EE_CAL_UNIT);

		ret = besra_mcu_set_pre_cal(dev, idx, cal, len,
					     MCU_EXT_CMD(GROUP_PRE_CAL_INFO));
		if (ret)
			return ret;

		total -= len;
		cal += len;
		idx++;
	}

	return 0;
}

static int besra_find_freq_idx(const u16 *freqs, int n_freqs, u16 cur)
{
	int i;

	for (i = 0; i < n_freqs; i++)
		if (cur == freqs[i])
			return i;

	return -1;
}

static int besra_dpd_freq_idx(u16 freq, u8 bw)
{
	static const u16 freq_list[] = {
		5180, 5200, 5220, 5240,
		5260, 5280, 5300, 5320,
		5500, 5520, 5540, 5560,
		5580, 5600, 5620, 5640,
		5660, 5680, 5700, 5745,
		5765, 5785, 5805, 5825
	};
	int offset_2g = ARRAY_SIZE(freq_list);
	int idx;

	if (freq < 4000) {
		if (freq < 2432)
			return offset_2g;
		if (freq < 2457)
			return offset_2g + 1;

		return offset_2g + 2;
	}

	if (bw == NL80211_CHAN_WIDTH_80P80 || bw == NL80211_CHAN_WIDTH_160)
		return -1;

	if (bw != NL80211_CHAN_WIDTH_20) {
		idx = besra_find_freq_idx(freq_list, ARRAY_SIZE(freq_list),
					   freq + 10);
		if (idx >= 0)
			return idx;

		idx = besra_find_freq_idx(freq_list, ARRAY_SIZE(freq_list),
					   freq - 10);
		if (idx >= 0)
			return idx;
	}

	return besra_find_freq_idx(freq_list, ARRAY_SIZE(freq_list), freq);
}

int besra_mcu_apply_tx_dpd(struct besra_phy *phy)
{
	struct besra_dev *dev = phy->dev;
	struct cfg80211_chan_def *chandef = &phy->mt76->chandef;
	u16 total = 2, center_freq = chandef->center_freq1;
	u8 *cal = dev->cal, *eep = dev->mt76.eeprom.data;
	int idx;

	if (1 || !(eep[MT_EE_DO_PRE_CAL] & MT_EE_WIFI_CAL_DPD))
		return 0;

	idx = besra_dpd_freq_idx(center_freq, chandef->width);
	if (idx < 0)
		return -EINVAL;

	/* Items: Tx DPD, Tx Flatness */
	idx = idx * 2;
	cal += MT_EE_CAL_GROUP_SIZE;

	while (total--) {
		int ret;

		cal += (idx * MT_EE_CAL_UNIT);
		ret = besra_mcu_set_pre_cal(dev, idx, cal, MT_EE_CAL_UNIT,
					     MCU_EXT_CMD(DPD_PRE_CAL_INFO));
		if (ret)
			return ret;

		idx++;
	}

	return 0;
}

int besra_mcu_get_chan_mib_info(struct besra_phy *phy, bool chan_switch)
{
	struct {
		struct {
			u8 band;
			u8 __rsv[3];
		} hdr;
		struct {
			__le16 tag;
			__le16 len;
			__le32 offs;
		} data[4];
	} __packed req = {
		.hdr.band = phy->band_idx,
	};
	/* strict order */
	static const u32 offs[] = {
		MIB_BUSY_TIME, MIB_TX_TIME, MIB_RX_TIME,
		MIB_OBSS_AIRTIME
	};
	struct mt76_channel_state *state = phy->mt76->chan_state;
	struct mt76_channel_state *state_ts = &phy->state_ts;
	struct besra_dev *dev = phy->dev;
	struct besra_mcu_mib *res;
	struct sk_buff *skb;
	int i, ret;

	for (i = 0; i < 4; i++) {
		req.data[i].tag = cpu_to_le16(UNI_CMD_MIB_DATA);
		req.data[i].len = cpu_to_le16(sizeof(req.data[i]));
		req.data[i].offs = cpu_to_le32(offs[i]);
	}

	ret = mt76_mcu_send_and_get_msg(&dev->mt76, MCU_WM_UNI_CMD(GET_MIB_INFO),
					&req, sizeof(req), true, &skb);
	if (ret)
		return ret;

	skb_pull(skb, sizeof(req.hdr));

	res = (struct besra_mcu_mib *)(skb->data);

	if (chan_switch)
		goto out;

#define __res_u64(s) le64_to_cpu(res[s].data)
	state->cc_busy += __res_u64(0) - state_ts->cc_busy;
	state->cc_tx += __res_u64(1) - state_ts->cc_tx;
	state->cc_bss_rx += __res_u64(2) - state_ts->cc_bss_rx;
	state->cc_rx += __res_u64(2) + __res_u64(3) - state_ts->cc_rx;

out:
	state_ts->cc_busy = __res_u64(0);
	state_ts->cc_tx = __res_u64(1);
	state_ts->cc_bss_rx = __res_u64(2);
	state_ts->cc_rx = __res_u64(2) + __res_u64(3);
#undef __res_u64

	dev_kfree_skb(skb);

	return 0;
}

int besra_mcu_get_temperature(struct besra_phy *phy)
{
	struct besra_dev *dev = phy->dev;
	struct {
		u8 ctrl_id;
		u8 action;
		u8 band;
		u8 rsv[5];
	} req = {
		.ctrl_id = THERMAL_SENSOR_TEMP_QUERY,
		.band = phy->band_idx,
	};

	return mt76_mcu_send_msg(&dev->mt76, MCU_EXT_CMD(THERMAL_CTRL), &req,
				 sizeof(req), true);
}

int besra_mcu_set_thermal_throttling(struct besra_phy *phy, u8 state)
{
	struct besra_dev *dev = phy->dev;
	struct {
		struct besra_mcu_thermal_ctrl ctrl;

		__le32 trigger_temp;
		__le32 restore_temp;
		__le16 sustain_time;
		u8 rsv[2];
	} __packed req = {
		.ctrl = {
			.band_idx = phy->band_idx,
		},
	};
	int level;

	if (!state) {
		req.ctrl.ctrl_id = THERMAL_PROTECT_DISABLE;
		goto out;
	}

	/* set duty cycle and level */
	for (level = 0; level < 4; level++) {
		int ret;

		req.ctrl.ctrl_id = THERMAL_PROTECT_DUTY_CONFIG;
		req.ctrl.duty.duty_level = level;
		req.ctrl.duty.duty_cycle = state;
		state /= 2;

		ret = mt76_mcu_send_msg(&dev->mt76, MCU_EXT_CMD(THERMAL_PROT),
					&req, sizeof(req.ctrl), false);
		if (ret)
			return ret;
	}

	/* set high-temperature trigger threshold */
	req.ctrl.ctrl_id = THERMAL_PROTECT_ENABLE;
	/* add a safety margin ~10 */
	req.restore_temp = cpu_to_le32(phy->throttle_temp[0] - 10);
	req.trigger_temp = cpu_to_le32(phy->throttle_temp[1]);
	req.sustain_time = cpu_to_le16(10);

out:
	req.ctrl.type.protect_type = 1;
	req.ctrl.type.trigger_type = 1;

	return mt76_mcu_send_msg(&dev->mt76, MCU_EXT_CMD(THERMAL_PROT),
				 &req, sizeof(req), false);
}

#if 0
int besra_mcu_set_txpower_sku(struct besra_phy *phy)
{
	struct besra_dev *dev = phy->dev;
	struct mt76_phy *mphy = phy->mt76;
	struct ieee80211_hw *hw = mphy->hw;
	struct besra_sku_val {
		u8 format_id;
		u8 limit_type;
		u8 band;
		s8 val[BESRA_SKU_RATE_NUM];
	} __packed req = {
		.format_id = 4,
		.band = phy->band_idx,
	};
	struct mt76_power_limits limits_array;
	s8 *la = (s8 *)&limits_array;
	int i, idx, n_chains = hweight8(mphy->antenna_mask);
	int tx_power = hw->conf.power_level * 2;

	tx_power = mt76_get_sar_power(mphy, mphy->chandef.chan,
				      tx_power);
	tx_power -= mt76_tx_power_nss_delta(n_chains);
	tx_power = mt76_get_rate_power_limits(mphy, mphy->chandef.chan,
					      &limits_array, tx_power);
	mphy->txpower_cur = tx_power;

	for (i = 0, idx = 0; i < ARRAY_SIZE(besra_sku_group_len); i++) {
		u8 mcs_num, len = besra_sku_group_len[i];
		int j;

		if (i >= SKU_HT_BW20 && i <= SKU_VHT_BW160) {
			mcs_num = 10;

			if (i == SKU_HT_BW20 || i == SKU_VHT_BW20)
				la = (s8 *)&limits_array + 12;
		} else {
			mcs_num = len;
		}

		for (j = 0; j < min_t(u8, mcs_num, len); j++)
			req.val[idx + j] = la[j];

		la += mcs_num;
		idx += len;
	}

	return mt76_mcu_send_msg(&dev->mt76,
				 MCU_EXT_CMD(TX_POWER_FEATURE_CTRL), &req,
				 sizeof(req), true);
}
#endif

int besra_mcu_get_txpower_sku(struct besra_phy *phy, s8 *txpower, int len)
{
#define RATE_POWER_INFO	2
	struct besra_dev *dev = phy->dev;
	struct {
		u8 _rsv[4];

		__le16 tag;
		__le16 len;

		u8 format_id;
		u8 category;
		u8 band;
		u8 _rsv2;
	} __packed req = {
		.tag = cpu_to_le16(UNI_TXPOWER_SHOW_INFO),
		.len = cpu_to_le16(sizeof(req) - 4),
		.format_id = 7,
		.category = RATE_POWER_INFO,
		.band = phy->band_idx,
	};
	s8 res[BESRA_SKU_RATE_NUM][3];
	struct sk_buff *skb;
	int ret, i;

	ret = mt76_mcu_send_and_get_msg(&dev->mt76, MCU_WM_UNI_CMD(TXPOWER),
					&req, sizeof(req), true, &skb);
	if (ret)
		return ret;

	/* TODO: support EHT rates */
	skb_pull(skb, 4 + sizeof(struct tlv));
	memcpy(res, skb->data, sizeof(res));
	for (i = 0; i < len; i++)
		txpower[i] = res[i][req.band];

	dev_kfree_skb(skb);

	return 0;
}

int besra_mcu_set_test_param(struct besra_dev *dev, u8 param, bool test_mode,
			      u8 en)
{
	struct {
		u8 test_mode_en;
		u8 param_idx;
		u8 _rsv[2];

		u8 enable;
		u8 _rsv2[3];

		u8 pad[8];
	} __packed req = {
		.test_mode_en = test_mode,
		.param_idx = param,
		.enable = en,
	};

	return mt76_mcu_send_msg(&dev->mt76, MCU_EXT_CMD(ATE_CTRL), &req,
				 sizeof(req), false);
}

int besra_mcu_set_sku_en(struct besra_phy *phy, bool enable)
{
	struct besra_dev *dev = phy->dev;
	struct besra_sku {
		u8 format_id;
		u8 sku_enable;
		u8 band;
		u8 rsv;
	} __packed req = {
		.format_id = 0,
		.band = phy->band_idx,
		.sku_enable = enable,
	};

	return mt76_mcu_send_msg(&dev->mt76,
				 MCU_EXT_CMD(TX_POWER_FEATURE_CTRL), &req,
				 sizeof(req), true);
}

int besra_mcu_set_ser(struct besra_dev *dev, u8 action, u8 set, u8 band)
{
	struct {
		u8 action;
		u8 set;
		u8 band;
		u8 rsv;
	} req = {
		.action = action,
		.set = set,
		.band = band,
	};

	return mt76_mcu_send_msg(&dev->mt76, MCU_EXT_CMD(SET_SER_TRIGGER),
				 &req, sizeof(req), false);
}

int besra_mcu_set_txbf(struct besra_dev *dev, u8 action)
{
	struct {
		u8 action;
		union {
			struct {
				u8 snd_mode;
				u8 sta_num;
				u8 rsv;
				u8 wlan_idx[4];
				__le32 snd_period;	/* ms */
			} __packed snd;
			struct {
				bool ebf;
				bool ibf;
				u8 rsv;
			} __packed type;
			struct {
				u8 bf_num;
				u8 bf_bitmap;
				u8 bf_sel[8];
				u8 rsv[5];
			} __packed mod;
		};
	} __packed req = {
		.action = action,
	};

#define MT_BF_PROCESSING	4
	switch (action) {
	case MT_BF_SOUNDING_ON:
		req.snd.snd_mode = MT_BF_PROCESSING;
		break;
	case MT_BF_TYPE_UPDATE:
		req.type.ebf = true;
		req.type.ibf = dev->ibf;
		break;
	case MT_BF_MODULE_UPDATE:
		req.mod.bf_num = 2;
		req.mod.bf_bitmap = GENMASK(1, 0);
		break;
	default:
		return -EINVAL;
	}

	return mt76_mcu_send_msg(&dev->mt76, MCU_EXT_CMD(TXBF_ACTION), &req,
				 sizeof(req), true);
}

int besra_mcu_add_obss_spr(struct besra_dev *dev, struct ieee80211_vif *vif,
			    bool enable)
{
#define MT_SPR_ENABLE		1
	struct besra_vif *mvif = (struct besra_vif *)vif->drv_priv;
	struct {
		u8 action;
		u8 arg_num;
		u8 band_idx;
		u8 status;
		u8 drop_tx_idx;
		u8 sta_idx;	/* 256 sta */
		u8 rsv[2];
		__le32 val;
	} __packed req = {
		.action = MT_SPR_ENABLE,
		.arg_num = 1,
		.band_idx = mvif->mt76.band_idx,
		.val = cpu_to_le32(enable),
	};

	return mt76_mcu_send_msg(&dev->mt76, MCU_EXT_CMD(SET_SPR), &req,
				 sizeof(req), true);
}

int besra_mcu_get_rx_rate(struct besra_phy *phy, struct ieee80211_vif *vif,
			   struct ieee80211_sta *sta, struct rate_info *rate)
{
	struct besra_vif *mvif = (struct besra_vif *)vif->drv_priv;
	struct besra_sta *msta = (struct besra_sta *)sta->drv_priv;
	struct besra_dev *dev = phy->dev;
	struct mt76_phy *mphy = phy->mt76;
	struct {
		u8 category;
		u8 band;
		__le16 wcid;
	} __packed req = {
		.category = MCU_PHY_STATE_CONTENTION_RX_RATE,
		.band = mvif->mt76.band_idx,
		.wcid = cpu_to_le16(msta->wcid.idx),
	};
	struct ieee80211_supported_band *sband;
	struct besra_mcu_phy_rx_info *res;
	struct sk_buff *skb;
	int ret;
	bool cck = false;

	ret = mt76_mcu_send_and_get_msg(&dev->mt76, MCU_EXT_CMD(PHY_STAT_INFO),
					&req, sizeof(req), true, &skb);
	if (ret)
		return ret;

	res = (struct besra_mcu_phy_rx_info *)skb->data;

	rate->mcs = res->rate;
	rate->nss = res->nsts + 1;

	switch (res->mode) {
	case MT_PHY_TYPE_CCK:
		cck = true;
		fallthrough;
	case MT_PHY_TYPE_OFDM:
		if (mphy->chandef.chan->band == NL80211_BAND_5GHZ)
			sband = &mphy->sband_5g.sband;
		else if (mphy->chandef.chan->band == NL80211_BAND_6GHZ)
			sband = &mphy->sband_6g.sband;
		else
			sband = &mphy->sband_2g.sband;

		rate->mcs = mt76_get_rate(&dev->mt76, sband, rate->mcs, cck);
		rate->legacy = sband->bitrates[rate->mcs].bitrate;
		break;
	case MT_PHY_TYPE_HT:
	case MT_PHY_TYPE_HT_GF:
		if (rate->mcs > 31) {
			ret = -EINVAL;
			goto out;
		}

		rate->flags = RATE_INFO_FLAGS_MCS;
		if (res->gi)
			rate->flags |= RATE_INFO_FLAGS_SHORT_GI;
		break;
	case MT_PHY_TYPE_VHT:
		if (rate->mcs > 9) {
			ret = -EINVAL;
			goto out;
		}

		rate->flags = RATE_INFO_FLAGS_VHT_MCS;
		if (res->gi)
			rate->flags |= RATE_INFO_FLAGS_SHORT_GI;
		break;
	case MT_PHY_TYPE_HE_SU:
	case MT_PHY_TYPE_HE_EXT_SU:
	case MT_PHY_TYPE_HE_TB:
	case MT_PHY_TYPE_HE_MU:
		if (res->gi > NL80211_RATE_INFO_HE_GI_3_2 || rate->mcs > 11) {
			ret = -EINVAL;
			goto out;
		}
		rate->he_gi = res->gi;
		rate->flags = RATE_INFO_FLAGS_HE_MCS;
		break;
	default:
		ret = -EINVAL;
		goto out;
	}

	switch (res->bw) {
	case IEEE80211_STA_RX_BW_160:
		rate->bw = RATE_INFO_BW_160;
		break;
	case IEEE80211_STA_RX_BW_80:
		rate->bw = RATE_INFO_BW_80;
		break;
	case IEEE80211_STA_RX_BW_40:
		rate->bw = RATE_INFO_BW_40;
		break;
	default:
		rate->bw = RATE_INFO_BW_20;
		break;
	}

out:
	dev_kfree_skb(skb);

	return ret;
}

int besra_mcu_update_bss_color(struct besra_dev *dev, struct ieee80211_vif *vif,
				struct cfg80211_he_bss_color *he_bss_color)
{
	int len = sizeof(struct bss_req_hdr) + sizeof(struct bss_color_tlv);
	struct besra_vif *mvif = (struct besra_vif *)vif->drv_priv;
	struct bss_color_tlv *bss_color;
	struct sk_buff *skb;
	struct tlv *tlv;

	skb = __besra_mcu_alloc_bss_req(&dev->mt76, &mvif->mt76, len);
	if (IS_ERR(skb))
		return PTR_ERR(skb);

	tlv = mt76_connac_mcu_add_tlv(skb, UNI_BSS_INFO_BSS_COLOR,
				      sizeof(*bss_color));
	bss_color = (struct bss_color_tlv *)tlv;
	bss_color->enable = he_bss_color->enabled;
	bss_color->color = he_bss_color->color;

	return mt76_mcu_skb_send_msg(&dev->mt76, skb,
				     MCU_WMWA_UNI_CMD(BSS_INFO_UPDATE), true);
}

#define TWT_AGRT_TRIGGER	BIT(0)
#define TWT_AGRT_ANNOUNCE	BIT(1)
#define TWT_AGRT_PROTECT	BIT(2)

int besra_mcu_twt_agrt_update(struct besra_dev *dev,
			       struct besra_vif *mvif,
			       struct besra_twt_flow *flow,
			       int cmd)
{
	struct {
		u8 tbl_idx;
		u8 cmd;
		u8 own_mac_idx;
		u8 flowid; /* 0xff for group id */
		__le16 peer_id; /* specify the peer_id (msb=0)
				 * or group_id (msb=1)
				 */
		u8 duration; /* 256 us */
		u8 bss_idx;
		__le64 start_tsf;
		__le16 mantissa;
		u8 exponent;
		u8 is_ap;
		u8 agrt_params;
		u8 rsv[23];
	} __packed req = {
		.tbl_idx = flow->table_id,
		.cmd = cmd,
		.own_mac_idx = mvif->mt76.omac_idx,
		.flowid = flow->id,
		.peer_id = cpu_to_le16(flow->wcid),
		.duration = flow->duration,
		.bss_idx = mvif->mt76.idx,
		.start_tsf = cpu_to_le64(flow->tsf),
		.mantissa = flow->mantissa,
		.exponent = flow->exp,
		.is_ap = true,
	};

	if (flow->protection)
		req.agrt_params |= TWT_AGRT_PROTECT;
	if (!flow->flowtype)
		req.agrt_params |= TWT_AGRT_ANNOUNCE;
	if (flow->trigger)
		req.agrt_params |= TWT_AGRT_TRIGGER;

	return mt76_mcu_send_msg(&dev->mt76, MCU_EXT_CMD(TWT_AGRT_UPDATE),
				 &req, sizeof(req), true);
}


void besra_mcu_set_pm(void *priv, u8 *mac, struct ieee80211_vif *vif)
{
#define EXIT_PM_STATE	0
#define ENTER_PM_STATE	1
	struct ieee80211_hw *hw = priv;
	struct besra_dev *dev = besra_hw_dev(hw);
	struct besra_phy *phy = besra_hw_phy(hw);
	struct besra_vif *mvif = (struct besra_vif *)vif->drv_priv;
	struct bss_power_save *ps;
	struct sk_buff *skb;
	struct tlv *tlv;
	bool running = test_bit(MT76_STATE_RUNNING, &phy->mt76->state);

	skb = __besra_mcu_alloc_bss_req(&dev->mt76, &mvif->mt76,
					BESRA_BSS_UPDATE_MAX_SIZE);
	if (IS_ERR(skb))
		return;

	tlv = besra_mcu_add_uni_tlv(skb, UNI_BSS_INFO_PS, sizeof(*ps));
	ps = (struct bss_power_save *)tlv;
	ps->profile = running ? EXIT_PM_STATE : ENTER_PM_STATE;

	mt76_mcu_skb_send_msg(&dev->mt76, skb,
			      MCU_WMWA_UNI_CMD(BSS_INFO_UPDATE), true);
}

int besra_mcu_set_rts_thresh(struct besra_phy *phy, u32 val)
{
	struct {
		u8 band_idx;
		u8 _rsv[3];

		__le16 tag;
		__le16 len;
		__le32 len_thresh;
		__le32 pkt_thresh;
	} __packed req = {
		.band_idx = phy->band_idx,
		.tag = cpu_to_le16(UNI_BAND_CONFIG_RTS_THRESHOLD),
		.len = cpu_to_le16(sizeof(req) - 4),
		.len_thresh = cpu_to_le32(val),
		.pkt_thresh = cpu_to_le32(0x2),
	};

	return mt76_mcu_send_msg(&phy->dev->mt76, MCU_WM_UNI_CMD(BAND_CONFIG),
				 &req, sizeof(req), true);
}

int besra_mcu_set_radio_en(struct besra_phy *phy, bool enable)
{
	struct {
		u8 band_idx;
		u8 _rsv[3];

		__le16 tag;
		__le16 len;
		u8 enable;
		u8 _rsv2[3];
	} __packed req = {
		.band_idx = phy->band_idx,
		.tag = cpu_to_le16(UNI_BAND_CONFIG_RADIO_ENABLE),
		.len = cpu_to_le16(sizeof(req) - 4),
		.enable = enable,
	};

	return mt76_mcu_send_msg(&phy->dev->mt76, MCU_WM_UNI_CMD(BAND_CONFIG),
				 &req, sizeof(req), true);
}

int besra_mcu_set_edcca_thresh(struct besra_phy *phy)
{
	struct {
		u8 band_idx;
		u8 _rsv[3];

		__le16 tag;
		__le16 len;
		u8 thresh[3];
		u8 fginit;
	} __packed req = {
		.band_idx = phy->band_idx,
		.tag = cpu_to_le16(UNI_BAND_CONFIG_EDCCA_THRESHOLD),
		.len = cpu_to_le16(sizeof(req) - 4),
		.thresh = {0x7f, 0x7f, 0x7f},
		.fginit = 1,
	};

	return mt76_mcu_send_msg(&phy->dev->mt76, MCU_WM_UNI_CMD(BAND_CONFIG),
				 &req, sizeof(req), true);
}

int besra_mcu_set_edcca_en(struct besra_phy *phy, bool enable)
{
	struct {
		u8 band_idx;
		u8 _rsv[3];

		__le16 tag;
		__le16 len;
		u8 enable;
		u8 _rsv2[3];
	} __packed req = {
		.band_idx = phy->band_idx,
		.tag = cpu_to_le16(UNI_BAND_CONFIG_EDCCA_ENABLE),
		.len = cpu_to_le16(sizeof(req) - 4),
		.enable = enable,
	};

	return mt76_mcu_send_msg(&phy->dev->mt76, MCU_WM_UNI_CMD(BAND_CONFIG),
				 &req, sizeof(req), true);
}

int besra_mcu_rdd_cmd(struct besra_dev *dev, int cmd, u8 index,
		      u8 rx_sel, u8 val)
{
	struct {
		u8 _rsv[4];

		__le16 tag;
		__le16 len;

		u8 ctrl;
		u8 rdd_idx;
		u8 rdd_rx_sel;
		u8 val;
		u8 rsv[4];
	} __packed req = {
		.tag = cpu_to_le16(UNI_RDD_CTRL_PARM),
		.len = cpu_to_le16(sizeof(req) - 4),
		.ctrl = cmd,
		.rdd_idx = index,
		.rdd_rx_sel = rx_sel,
		.val = val,
	};

	/* Todo: check return value. return 0 is bellwether workaround.
		 Bellwether does not enable RDD.
	*/
	mt76_mcu_send_msg(&dev->mt76, MCU_WM_UNI_CMD(RDD_CTRL),
			  &req, sizeof(req), true);
	return 0;
}


int besra_mcu_wtbl_update_hdr_trans(struct besra_dev *dev,
			struct ieee80211_vif *vif, struct ieee80211_sta *sta)
{
	struct besra_vif *mvif = (struct besra_vif *)vif->drv_priv;
	struct besra_sta *msta;
	struct sk_buff *skb;

	msta = sta ? (struct besra_sta *)sta->drv_priv : &mvif->sta;

	skb = mt76_connac_mcu_alloc_sta_req(&dev->mt76, &mvif->mt76,
					    &msta->wcid);
	if (IS_ERR(skb))
		return PTR_ERR(skb);

	/* starec hdr trans */
	besra_mcu_sta_hdr_trans_tlv(dev, skb, vif, sta);
	return mt76_mcu_skb_send_msg(&dev->mt76, skb,
				     MCU_WMWA_UNI_CMD(STA_REC_UPDATE), true);
}

int besra_mcu_rf_regval(struct besra_dev *dev, u32 regidx, u32 *val, bool set)
{
	struct {
		__le32 idx;
		__le32 ofs;
		__le32 data;
	} __packed req = {
	.idx = cpu_to_le32(u32_get_bits(regidx, GENMASK(31, 28))),
		.ofs = cpu_to_le32(u32_get_bits(regidx, GENMASK(27, 0))),
		.data = set ? cpu_to_le32(*val) : 0,
	};
	struct sk_buff *skb;
	int ret;

	if (set)
		return mt76_mcu_send_msg(&dev->mt76, MCU_EXT_CMD(RF_REG_ACCESS),
					 &req, sizeof(req), false);

	ret = mt76_mcu_send_and_get_msg(&dev->mt76, MCU_EXT_QUERY(RF_REG_ACCESS),
					&req, sizeof(req), true, &skb);
	if (ret)
		return ret;

	*val = le32_to_cpu(*(__le32 *)(skb->data + 8));
	dev_kfree_skb(skb);

	return 0;
}
