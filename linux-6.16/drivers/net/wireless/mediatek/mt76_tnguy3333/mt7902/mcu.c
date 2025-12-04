// SPDX-License-Identifier: ISC
/* Copyright (C) 2020 MediaTek Inc. */

#include <linux/fs.h>
#include <linux/firmware.h>
#include "mt7902.h"
#include "mcu.h"
#include "mt76_connac2_mac.h"
#include "mt792x_trace.h"

#define MT_STA_BFER			BIT(0)
#define MT_STA_BFEE			BIT(1)

#define HE_PHY(p, c)			u8_get_bits(c, IEEE80211_HE_PHY_##p)
#define HE_MAC(m, c)			u8_get_bits(c, IEEE80211_HE_MAC_##m)


static bool mt7902_disable_clc;
module_param_named(disable_clc, mt7902_disable_clc, bool, 0644);
MODULE_PARM_DESC(disable_clc, "disable CLC support");

int mt7902_mcu_parse_response(struct mt76_dev *mdev, int cmd,
			      struct sk_buff *skb, int seq)
{
	printk(KERN_DEBUG "mcu.c - mt7902_mcu_parse_response(mdev, cmd: %x, skb, seq: %d)", cmd, seq);
	int mcu_cmd = FIELD_GET(__MCU_CMD_FIELD_ID, cmd);
	struct mt76_connac2_mcu_rxd *rxd;
	int ret = 0;

	if (!skb) {
		dev_err(mdev->dev, "Message %08x (seq %d) timeout\n",
			cmd, seq);
		mt792x_reset(mdev);

		return -ETIMEDOUT;
	}

	rxd = (struct mt76_connac2_mcu_rxd *)skb->data;
	if (seq != rxd->seq)
		return -EAGAIN;

	if (cmd == MCU_CMD(PATCH_SEM_CONTROL) ||
	    cmd == MCU_CMD(PATCH_FINISH_REQ)) {
		skb_pull(skb, sizeof(*rxd) - 4);
		ret = *skb->data;
	} else if (cmd == MCU_EXT_CMD(THERMAL_CTRL)) {
		skb_pull(skb, sizeof(*rxd) + 4);
		ret = le32_to_cpu(*(__le32 *)skb->data);
	} else if (cmd == MCU_UNI_CMD(DEV_INFO_UPDATE) ||
		   cmd == MCU_UNI_CMD(BSS_INFO_UPDATE) ||
		   cmd == MCU_UNI_CMD(STA_REC_UPDATE) ||
		   cmd == MCU_UNI_CMD(HIF_CTRL) ||
		   cmd == MCU_UNI_CMD(OFFLOAD) ||
		   cmd == MCU_UNI_CMD(SUSPEND)) {
		struct mt76_connac_mcu_uni_event *event;

		skb_pull(skb, sizeof(*rxd));
		event = (struct mt76_connac_mcu_uni_event *)skb->data;
		ret = le32_to_cpu(event->status);
		/* skip invalid event */
		if (mcu_cmd != event->cid)
			ret = -EAGAIN;
	} else if (cmd == MCU_CE_QUERY(REG_READ)) {
		struct mt76_connac_mcu_reg_event *event;

		skb_pull(skb, sizeof(*rxd));
		event = (struct mt76_connac_mcu_reg_event *)skb->data;
		ret = (int)le32_to_cpu(event->val);
	} else if (cmd == MCU_EXT_CMD(WF_RF_PIN_CTRL)) {
		struct mt7902_wf_rf_pin_ctrl_event *event;

		skb_pull(skb, sizeof(*rxd));
		event = (struct mt7902_wf_rf_pin_ctrl_event *)skb->data;
		ret = (int)event->result;
	} else {
		skb_pull(skb, sizeof(struct mt76_connac2_mcu_rxd));
	}

	return ret;
}
EXPORT_SYMBOL_GPL(mt7902_mcu_parse_response);

static int mt7902_mcu_read_eeprom(struct mt792x_dev *dev, u32 offset, u8 *val)
{
	printk(KERN_DEBUG "mcu.c - mt7902_mcu_read_eeprom(dev, offset: 0x%08x, val: 0x%02x)", offset, val);
	struct mt7902_mcu_eeprom_info *res, req = {
		.addr = cpu_to_le32(round_down(offset,
				    MT7902_EEPROM_BLOCK_SIZE)),
	};
	struct sk_buff *skb;
	int ret;

	ret = mt76_mcu_send_and_get_msg(&dev->mt76, MCU_EXT_QUERY(EFUSE_ACCESS),
					&req, sizeof(req), true, &skb);
	if (ret)
		return ret;

	res = (struct mt7902_mcu_eeprom_info *)skb->data;
	*val = res->data[offset % MT7902_EEPROM_BLOCK_SIZE];
	dev_kfree_skb(skb);

	return 0;
}

#ifdef CONFIG_PM

static int
mt7902_mcu_set_ipv6_ns_filter(struct mt76_dev *dev,
			      struct ieee80211_vif *vif, bool suspend)
{
	printk(KERN_DEBUG "mcu.c - mt7902_mcu_set_ipv6_ns_filter");
	struct mt792x_vif *mvif = (struct mt792x_vif *)vif->drv_priv;
	struct {
		struct {
			u8 bss_idx;
			u8 pad[3];
		} __packed hdr;
		struct mt76_connac_arpns_tlv arpns;
	} req = {
		.hdr = {
			.bss_idx = mvif->bss_conf.mt76.idx,
		},
		.arpns = {
			.tag = cpu_to_le16(UNI_OFFLOAD_OFFLOAD_ND),
			.len = cpu_to_le16(sizeof(struct mt76_connac_arpns_tlv)),
			.mode = suspend,
		},
	};

	return mt76_mcu_send_msg(dev, MCU_UNI_CMD_OFFLOAD, &req, sizeof(req),
				 true);
}

void mt7902_mcu_set_suspend_iter(void *priv, u8 *mac, struct ieee80211_vif *vif)
{
	printk(KERN_DEBUG "mcu.c - mt7902_mcu_set_suspend_iter");
	if (IS_ENABLED(CONFIG_IPV6)) {
		struct mt76_phy *phy = priv;

		mt7902_mcu_set_ipv6_ns_filter(phy->dev, vif,
					      !test_bit(MT76_STATE_RUNNING,
					      &phy->state));
	}

	mt76_connac_mcu_set_suspend_iter(priv, mac, vif);
}

#endif /* CONFIG_PM */

static void
mt7902_mcu_uni_roc_event(struct mt792x_dev *dev, struct sk_buff *skb)
{
	printk(KERN_DEBUG "mcu.c - mt7902_mcu_uni_roc_event");
	struct mt7902_roc_grant_tlv *grant;
	struct mt76_connac2_mcu_rxd *rxd;
	int duration;

	rxd = (struct mt76_connac2_mcu_rxd *)skb->data;
	grant = (struct mt7902_roc_grant_tlv *)(rxd->tlv + 4);

	/* should never happen */
	WARN_ON_ONCE((le16_to_cpu(grant->tag) != UNI_EVENT_ROC_GRANT));

	if (grant->reqtype == MT7902_ROC_REQ_ROC)
		ieee80211_ready_on_channel(dev->mt76.phy.hw);

	dev->phy.roc_grant = true;
	wake_up(&dev->phy.roc_wait);
	duration = le32_to_cpu(grant->max_interval);
	mod_timer(&dev->phy.roc_timer,
		  jiffies + msecs_to_jiffies(duration));
}

static void
mt7902_mcu_scan_event(struct mt792x_dev *dev, struct sk_buff *skb)
{
	printk(KERN_DEBUG "mcu.c - mt7902_mcu_scan_event");
	struct mt76_phy *mphy = &dev->mt76.phy;
	struct mt792x_phy *phy = mphy->priv;

	spin_lock_bh(&dev->mt76.lock);
	__skb_queue_tail(&phy->scan_event_list, skb);
	spin_unlock_bh(&dev->mt76.lock);

	ieee80211_queue_delayed_work(mphy->hw, &phy->scan_work,
				     MT792x_HW_SCAN_TIMEOUT);
}

static void
mt7902_mcu_connection_loss_iter(void *priv, u8 *mac,
				struct ieee80211_vif *vif)
{
	printk(KERN_DEBUG "mcu.c - mt7902_mcu_connection_loss_iter");
	struct mt76_vif_link *mvif = (struct mt76_vif_link *)vif->drv_priv;
	struct mt76_connac_beacon_loss_event *event = priv;

	if (mvif->idx != event->bss_idx)
		return;

	if (!(vif->driver_flags & IEEE80211_VIF_BEACON_FILTER) ||
	    vif->type != NL80211_IFTYPE_STATION)
		return;

	ieee80211_connection_loss(vif);
}

static void
mt7902_mcu_connection_loss_event(struct mt792x_dev *dev, struct sk_buff *skb)
{
	printk(KERN_DEBUG "mcu.c - mt7902_mcu_connection_loss_event");
	struct mt76_connac_beacon_loss_event *event;
	struct mt76_phy *mphy = &dev->mt76.phy;

	skb_pull(skb, sizeof(struct mt76_connac2_mcu_rxd));
	event = (struct mt76_connac_beacon_loss_event *)skb->data;

	ieee80211_iterate_active_interfaces_atomic(mphy->hw,
					IEEE80211_IFACE_ITER_RESUME_ALL,
					mt7902_mcu_connection_loss_iter, event);
}

static void
mt7902_mcu_debug_msg_event(struct mt792x_dev *dev, struct sk_buff *skb)
{
	printk(KERN_DEBUG "mcu.c - mt7902_mcu_debug_msg_event");
	struct mt7902_debug_msg {
		__le16 id;
		u8 type;
		u8 flag;
		__le32 value;
		__le16 len;
		u8 content[512];
	} __packed * msg;

	skb_pull(skb, sizeof(struct mt76_connac2_mcu_rxd));
	msg = (struct mt7902_debug_msg *)skb->data;

	if (msg->type == 3) { /* fw log */
		u16 len = min_t(u16, le16_to_cpu(msg->len), 512);
		int i;

		for (i = 0 ; i < len; i++) {
			if (!msg->content[i])
				msg->content[i] = ' ';
		}
		wiphy_info(mt76_hw(dev)->wiphy, "%.*s", len, msg->content);
	}
}

static void
mt7902_mcu_low_power_event(struct mt792x_dev *dev, struct sk_buff *skb)
{
	printk(KERN_DEBUG "mcu.c - mt7902_mcu_low_power_event(dev, skb)");
	struct mt7902_mcu_lp_event {
		u8 state;
		u8 reserved[3];
	} __packed * event;

	skb_pull(skb, sizeof(struct mt76_connac2_mcu_rxd));
	event = (struct mt7902_mcu_lp_event *)skb->data;

	trace_lp_event(dev, event->state);
}

static void
mt7902_mcu_tx_done_event(struct mt792x_dev *dev, struct sk_buff *skb)
{
	printk(KERN_DEBUG "mcu.c - mt7902_mcu_tx_done_event");
	struct mt7902_mcu_tx_done_event *event;

	skb_pull(skb, sizeof(struct mt76_connac2_mcu_rxd));
	event = (struct mt7902_mcu_tx_done_event *)skb->data;

	mt7902_mac_add_txs(dev, event->txs);
}

static void
mt7902_mcu_rssi_monitor_iter(void *priv, u8 *mac,
			     struct ieee80211_vif *vif)
{
	printk(KERN_DEBUG "mcu.c - mt7902_mcu_rssi_monitor_iter");
	struct mt792x_vif *mvif = (struct mt792x_vif *)vif->drv_priv;
	struct mt76_connac_rssi_notify_event *event = priv;
	enum nl80211_cqm_rssi_threshold_event nl_event;
	s32 rssi = le32_to_cpu(event->rssi[mvif->bss_conf.mt76.idx]);

	if (!rssi)
		return;

	if (!(vif->driver_flags & IEEE80211_VIF_SUPPORTS_CQM_RSSI))
		return;

	if (rssi > vif->bss_conf.cqm_rssi_thold)
		nl_event = NL80211_CQM_RSSI_THRESHOLD_EVENT_HIGH;
	else
		nl_event = NL80211_CQM_RSSI_THRESHOLD_EVENT_LOW;

	ieee80211_cqm_rssi_notify(vif, nl_event, rssi, GFP_KERNEL);
}

static void
mt7902_mcu_rssi_monitor_event(struct mt792x_dev *dev, struct sk_buff *skb)
{
	printk(KERN_DEBUG "mcu.c - mt7902_mcu_rssi_monitor_event");
	struct mt76_connac_rssi_notify_event *event;

	skb_pull(skb, sizeof(struct mt76_connac2_mcu_rxd));
	event = (struct mt76_connac_rssi_notify_event *)skb->data;

	ieee80211_iterate_active_interfaces_atomic(mt76_hw(dev),
						   IEEE80211_IFACE_ITER_RESUME_ALL,
						   mt7902_mcu_rssi_monitor_iter, event);
}

static void
mt7902_mcu_rx_unsolicited_event(struct mt792x_dev *dev, struct sk_buff *skb)
{
	//printk(KERN_DEBUG "mcu.c - mt7902_mcu_rx_unsolicited_event(dev, skb)");
	struct mt76_connac2_mcu_rxd *rxd;

	rxd = (struct mt76_connac2_mcu_rxd *)skb->data;

	//printk(KERN_DEBUG "mcu.c - mt7902_mcu_rx_unsolicited_event - rxd->eid : %x", rxd->eid);
	switch (rxd->eid) {
	case MCU_EVENT_BSS_BEACON_LOSS:
		mt7902_mcu_connection_loss_event(dev, skb);
		break;
	case MCU_EVENT_SCHED_SCAN_DONE:
	case MCU_EVENT_SCAN_DONE:
		mt7902_mcu_scan_event(dev, skb);
		return;
	case MCU_EVENT_DBG_MSG:
		mt7902_mcu_debug_msg_event(dev, skb);
		break;
	case MCU_EVENT_COREDUMP:
		dev->fw_assert = true;
		mt76_connac_mcu_coredump_event(&dev->mt76, skb,
					       &dev->coredump);
		return;
	case MCU_EVENT_LP_INFO:
		mt7902_mcu_low_power_event(dev, skb);
		break;
	case MCU_EVENT_TX_DONE:
		mt7902_mcu_tx_done_event(dev, skb);
		break;
	case MCU_EVENT_RSSI_NOTIFY:
		mt7902_mcu_rssi_monitor_event(dev, skb);
		break;
	default:
		break;
	}
	dev_kfree_skb(skb);
}

static void
mt7902_mcu_uni_rx_unsolicited_event(struct mt792x_dev *dev,
				    struct sk_buff *skb)
{
	printk(KERN_DEBUG "mcu.c - mt7902_mcu_uni_rx_unsolicited_event(dev, skb)");
	struct mt76_connac2_mcu_rxd *rxd;

	rxd = (struct mt76_connac2_mcu_rxd *)skb->data;
	printk(KERN_DEBUG "mcu.c - mt7902_mcu_uni_rx_unsolicited_event - rxd->eid: %x", rxd->eid);
	switch (rxd->eid) {
	case MCU_UNI_EVENT_ROC:
		mt7902_mcu_uni_roc_event(dev, skb);
		break;
	default:
		break;
	}
	dev_kfree_skb(skb);
}

void mt7902_mcu_rx_event(struct mt792x_dev *dev, struct sk_buff *skb)
{
	//printk(KERN_DEBUG "mcu.c - mt7902_mcu_rx_event(dev, skb)");
	struct mt76_connac2_mcu_rxd *rxd;

	if (skb_linearize(skb))
		return;

	rxd = (struct mt76_connac2_mcu_rxd *)skb->data;

	if (rxd->option & MCU_UNI_CMD_UNSOLICITED_EVENT) {
		mt7902_mcu_uni_rx_unsolicited_event(dev, skb);
		return;
	}

	if (rxd->eid == 0x6) {
		mt76_mcu_rx_event(&dev->mt76, skb);
		return;
	}

	if (rxd->ext_eid == MCU_EXT_EVENT_RATE_REPORT ||
	    rxd->eid == MCU_EVENT_BSS_BEACON_LOSS ||
	    rxd->eid == MCU_EVENT_SCHED_SCAN_DONE ||
	    rxd->eid == MCU_EVENT_RSSI_NOTIFY ||
	    rxd->eid == MCU_EVENT_SCAN_DONE ||
	    rxd->eid == MCU_EVENT_TX_DONE ||
	    rxd->eid == MCU_EVENT_DBG_MSG ||
	    rxd->eid == MCU_EVENT_COREDUMP ||
	    rxd->eid == MCU_EVENT_LP_INFO ||
	    !rxd->seq)
		mt7902_mcu_rx_unsolicited_event(dev, skb);
	else
		mt76_mcu_rx_event(&dev->mt76, skb);
}

/** starec & wtbl **/
int mt7902_mcu_uni_tx_ba(struct mt792x_dev *dev,
			 struct ieee80211_ampdu_params *params,
			 bool enable)
{
	printk(KERN_DEBUG "mcu.c - mt7902_mcu_uni_tx_ba");
	struct mt792x_sta *msta = (struct mt792x_sta *)params->sta->drv_priv;

	if (enable && !params->amsdu)
		msta->deflink.wcid.amsdu = false;

	printk(KERN_INFO "uni_tx_ba sta rec update\n");

	return mt76_connac_mcu_sta_ba(&dev->mt76, &msta->vif->bss_conf.mt76, params,
				      MCU_EXT_CMD(STA_REC_UPDATE),
				      enable, true);
}

int mt7902_mcu_uni_rx_ba(struct mt792x_dev *dev,
			 struct ieee80211_ampdu_params *params,
			 bool enable)
{
	printk(KERN_DEBUG "mcu.c - mt7902_mcu_uni_rx_ba");
	struct mt792x_sta *msta = (struct mt792x_sta *)params->sta->drv_priv;

	printk(KERN_INFO "uni_rx_ba sta rec update\n");

	return mt76_connac_mcu_sta_ba(&dev->mt76, &msta->vif->bss_conf.mt76, params,
				      MCU_EXT_CMD(STA_REC_UPDATE),
				      enable, false);
}

static int mt7902_load_clc(struct mt792x_dev *dev, const char *fw_name)
{
	printk(KERN_DEBUG "mcu.c - mt7902_load_clc(dev, fw_name: %s)",fw_name);
	const struct mt76_connac2_fw_trailer *hdr;
	const struct mt76_connac2_fw_region *region;
	const struct mt7902_clc *clc;
	struct mt76_dev *mdev = &dev->mt76;
	struct mt792x_phy *phy = &dev->phy;
	const struct firmware *fw;
	int ret, i, len, offset = 0;
	u8 *clc_base = NULL, hw_encap = 0;

	dev->phy.clc_chan_conf = 0xff;
	if (mt7902_disable_clc ||
	    mt76_is_usb(&dev->mt76))
		return 0;

	if (mt76_is_mmio(&dev->mt76)) {
		ret = mt7902_mcu_read_eeprom(dev, MT_EE_HW_TYPE, &hw_encap);
		if (ret)
			return ret;
		hw_encap = u8_get_bits(hw_encap, MT_EE_HW_TYPE_ENCAP);
	}

	ret = request_firmware(&fw, fw_name, mdev->dev);
	if (ret)
		return ret;

	if (!fw || !fw->data || fw->size < sizeof(*hdr)) {
		dev_err(mdev->dev, "Invalid firmware\n");
		ret = -EINVAL;
		goto out;
	}

	hdr = (const void *)(fw->data + fw->size - sizeof(*hdr));
	for (i = 0; i < hdr->n_region; i++) {
		region = (const void *)((const u8 *)hdr -
					(hdr->n_region - i) * sizeof(*region));
		len = le32_to_cpu(region->len);

		/* check if we have valid buffer size */
		if (offset + len > fw->size) {
			dev_err(mdev->dev, "Invalid firmware region\n");
			ret = -EINVAL;
			goto out;
		}

		if ((region->feature_set & FW_FEATURE_NON_DL) &&
		    region->type == FW_TYPE_CLC) {
			clc_base = (u8 *)(fw->data + offset);
			break;
		}
		offset += len;
	}

	if (!clc_base)
		goto out;

	for (offset = 0; offset < len; offset += le32_to_cpu(clc->len)) {
		clc = (const struct mt7902_clc *)(clc_base + offset);

		/* do not init buf again if chip reset triggered */
		if (phy->clc[clc->idx])
			continue;

		/* header content sanity */
		if (clc->idx == MT7902_CLC_POWER &&
		    u8_get_bits(clc->type, MT_EE_HW_TYPE_ENCAP) != hw_encap)
			continue;

		phy->clc[clc->idx] = devm_kmemdup(mdev->dev, clc,
						  le32_to_cpu(clc->len),
						  GFP_KERNEL);

		if (!phy->clc[clc->idx]) {
			ret = -ENOMEM;
			goto out;
		}
	}
	ret = mt7902_mcu_set_clc(dev, "00", ENVIRON_INDOOR);
out:
	release_firmware(fw);

	return ret;
}

static void mt7902_mcu_parse_tx_resource(struct mt76_dev *dev,
					 struct sk_buff *skb)
{
	printk(KERN_DEBUG "mcu.c - mt7902_mcu_parse_tx_resource");
	struct mt76_sdio *sdio = &dev->sdio;
	struct mt7902_tx_resource {
		__le32 version;
		__le32 pse_data_quota;
		__le32 pse_mcu_quota;
		__le32 ple_data_quota;
		__le32 ple_mcu_quota;
		__le16 pse_page_size;
		__le16 ple_page_size;
		u8 pp_padding;
		u8 pad[3];
	} __packed * tx_res;

	tx_res = (struct mt7902_tx_resource *)skb->data;
	sdio->sched.pse_data_quota = le32_to_cpu(tx_res->pse_data_quota);
	sdio->pse_mcu_quota_max = le32_to_cpu(tx_res->pse_mcu_quota);
	/* The mcu quota usage of this function itself must be taken into consideration */
	sdio->sched.pse_mcu_quota =
		sdio->sched.pse_mcu_quota ? sdio->pse_mcu_quota_max : sdio->pse_mcu_quota_max - 1;
	sdio->sched.ple_data_quota = le32_to_cpu(tx_res->ple_data_quota);
	sdio->sched.pse_page_size = le16_to_cpu(tx_res->pse_page_size);
	sdio->sched.deficit = tx_res->pp_padding;
}

static void mt7902_mcu_parse_phy_cap(struct mt76_dev *dev,
				     struct sk_buff *skb)
{
	printk(KERN_DEBUG "mcu.c - mt7902_mcu_parse_phy_cap(dev, skb)");
	struct mt792x_phy_cap {
		u8 ht;
		u8 vht;
		u8 _5g;
		u8 max_bw;
		u8 nss;
		u8 dbdc;
		u8 tx_ldpc;
		u8 rx_ldpc;
		u8 tx_stbc;
		u8 rx_stbc;
		u8 hw_path;
		u8 he;
	} __packed * cap;

	enum {
		WF0_24G,
		WF0_5G
	};

	cap = (struct mt792x_phy_cap *)skb->data;

	dev->phy.antenna_mask = BIT(cap->nss) - 1;
	dev->phy.chainmask = dev->phy.antenna_mask;
	dev->phy.cap.has_2ghz = cap->hw_path & BIT(WF0_24G);
	dev->phy.cap.has_5ghz = cap->hw_path & BIT(WF0_5G);
}

static int mt7902_mcu_get_nic_capability(struct mt792x_phy *mphy)
{
	printk(KERN_DEBUG "mcu.c - mt7902_mcu_get_nic_capability(mphy)");
	struct mt76_connac_cap_hdr {
		__le16 n_element;
		u8 rsv[2];
	} __packed * hdr;
	struct sk_buff *skb;
	struct mt76_phy *phy = mphy->mt76;
	int ret, i;

	ret = mt76_mcu_send_and_get_msg(phy->dev, MCU_CE_CMD(GET_NIC_CAPAB),
					NULL, 0, true, &skb);
	if (ret)
		return ret;

	hdr = (struct mt76_connac_cap_hdr *)skb->data;
	if (skb->len < sizeof(*hdr)) {
		ret = -EINVAL;
		goto out;
	}

	skb_pull(skb, sizeof(*hdr));

	for (i = 0; i < le16_to_cpu(hdr->n_element); i++) {
		struct tlv_hdr {
			__le32 type;
			__le32 len;
		} __packed * tlv = (struct tlv_hdr *)skb->data;
		int len;

		if (skb->len < sizeof(*tlv))
			break;

		skb_pull(skb, sizeof(*tlv));

		len = le32_to_cpu(tlv->len);
		if (skb->len < len)
			break;

		switch (le32_to_cpu(tlv->type)) {
		case MT_NIC_CAP_6G:
			// printk(KERN_DEBUG "mcu.c - mt7902_mcu_get_nic_capability - MT_NIC_CAP_6G");
			phy->cap.has_6ghz = skb->data[0];
			break;
		case MT_NIC_CAP_MAC_ADDR:
			// printk(KERN_DEBUG "mcu.c - mt7902_mcu_get_nic_capability - MT_NIC_CAP_MAC_ADDR");
			memcpy(phy->macaddr, (void *)skb->data, ETH_ALEN);
			break;
		case MT_NIC_CAP_PHY:
			// printk(KERN_DEBUG "mcu.c - mt7902_mcu_get_nic_capability - MT_NIC_CAP_PHY");
			mt7902_mcu_parse_phy_cap(phy->dev, skb);
			break;
		case MT_NIC_CAP_TX_RESOURCE:
			// printk(KERN_DEBUG "mcu.c - mt7902_mcu_get_nic_capability - MT_NIC_CAP_TX_RESOURCE");
			if (mt76_is_sdio(phy->dev))
				mt7902_mcu_parse_tx_resource(phy->dev,
							     skb);
			break;
		case MT_NIC_CAP_CHIP_CAP:
			// printk(KERN_DEBUG "mcu.c - mt7902_mcu_get_nic_capability - MT_NIC_CAP_CHIP_CAP");
			memcpy(&mphy->chip_cap, (void *)skb->data, sizeof(u64));
			break;
		default:
			break;
		}
		skb_pull(skb, len);
	}
out:
	dev_kfree_skb(skb);

	return ret;
}

int mt7902_mcu_fw_log_2_host(struct mt792x_dev *dev, u8 ctrl)
{
	printk(KERN_DEBUG "mcu.c - mt7902_mcu_fw_log_2_host(dev, ctrl: 0x%02x)", ctrl);
	struct {
		u8 ctrl_val;
		u8 pad[3];
	} data = {
		.ctrl_val = ctrl
	};

	return mt76_mcu_send_msg(&dev->mt76, MCU_CE_CMD(FWLOG_2_HOST),
				 &data, sizeof(data), false);
}

int mt7902_firmware_state(struct mt792x_dev *dev, bool wa)
{
	printk(KERN_DEBUG "mcu.c - mt7902_firmware_state(dev, wa: %d)", wa);
	u32 state = FIELD_PREP(MT_TOP_MISC_FW_STATE,
			       wa ? FW_STATE_RDY : FW_STATE_FW_DOWNLOAD);

	if (!mt76_poll_msec(dev, MT_TOP_MISC, MT_TOP_MISC_FW_STATE,
			    state, 1000)) {
		dev_err(dev->mt76.dev, "Timeout for initializing firmware\n");
		return -EIO;
	}
	return 0;
}

int mt7902_load_firmware(struct mt792x_dev *dev)
{
	printk(KERN_DEBUG "mcu.c - mt7902_load_firmware(dev)");
	int ret;
    
    /* make sure fw is download state */
	if (mt7902_firmware_state(dev, false)) {
		/* restart firmware once */
		mt76_connac_mcu_restart(&dev->mt76);
		ret = mt7902_firmware_state(dev, false);
		if (ret) {
			dev_err(dev->mt76.dev,
				"Firmware is not ready for download\n");
			return ret;
		}
	}

	printk(KERN_DEBUG "mcu.c - mt7902_load_firmware - load_patch");
	ret = mt76_connac2_load_patch(&dev->mt76, mt792x_patch_name(dev));
	if (ret)
		return ret;

	if (mt76_is_sdio(&dev->mt76)) {
		/* activate again */
		ret = __mt792x_mcu_fw_pmctrl(dev);
		if (!ret)
			ret = __mt792x_mcu_drv_pmctrl(dev);
	}

	printk(KERN_DEBUG "mcu.c - mt7902_load_firmware - load_ram");
	ret = mt76_connac2_load_ram(&dev->mt76, mt792x_ram_name(dev), NULL);
	if (ret)
		return ret;

	if (!mt76_poll_msec(dev, MT_CONN_ON_MISC, MT_TOP_MISC2_FW_N9_RDY,
			    MT_TOP_MISC2_FW_N9_RDY, 1500)) {
		dev_err(dev->mt76.dev, "Timeout for initializing firmware\n");

		return -EIO;
	}

#ifdef CONFIG_PM2
	dev->mt76.hw->wiphy->wowlan = &mt76_connac_wowlan_support;
#endif /* CONFIG_PM */

	dev_dbg(dev->mt76.dev, "Firmware init done\n");

	return 0;
}

int mt7902_run_firmware(struct mt792x_dev *dev)
{
	printk(KERN_DEBUG "mcu.c - mt7902_run_firmware(dev)");
	int err;

	err = mt7902_load_firmware(dev);
	if (err)
		return err;

	err = mt7902_mcu_get_nic_capability(&dev->phy);
	if (err)
		return err;

	set_bit(MT76_STATE_MCU_RUNNING, &dev->mphy.state);
	err = mt7902_load_clc(dev, mt792x_ram_name(dev));
	if (err)
		return err;

	return mt7902_mcu_fw_log_2_host(dev, 1);
}
EXPORT_SYMBOL_GPL(mt7902_run_firmware);

int mt7902_mcu_radio_led_ctrl(struct mt792x_dev *dev, u8 value)
{
	printk(KERN_DEBUG "mcu.c - mt7902_mcu_radio_led_ctrl(dev, value: 0x%02x)", value);
	struct {
		u8 ctrlid;
		u8 rsv[3];
	} __packed req = {
		.ctrlid = value,
	};

	return mt76_mcu_send_msg(&dev->mt76, MCU_EXT_CMD(ID_RADIO_ON_OFF_CTRL),
				&req, sizeof(req), false);
}
EXPORT_SYMBOL_GPL(mt7902_mcu_radio_led_ctrl);

int mt7902_mcu_set_tx(struct mt792x_dev *dev, struct ieee80211_vif *vif)
{
	printk(KERN_DEBUG "mcu.c - mt7902_mcu_set_tx(dev, vif)");
	struct mt792x_vif *mvif = (struct mt792x_vif *)vif->drv_priv;
	struct edca {
		__le16 cw_min;
		__le16 cw_max;
		__le16 txop;
		__le16 aifs;
		u8 guardtime;
		u8 acm;
	} __packed;
	struct mt7902_mcu_tx {
		struct edca edca[IEEE80211_NUM_ACS];
		u8 bss_idx;
		u8 qos;
		u8 wmm_idx;
		u8 pad;
	} __packed req = {
		.bss_idx = mvif->bss_conf.mt76.idx,
		.qos = vif->bss_conf.qos,
		.wmm_idx = mvif->bss_conf.mt76.wmm_idx,
	};
	struct mu_edca {
		u8 cw_min;
		u8 cw_max;
		u8 aifsn;
		u8 acm;
		u8 timer;
		u8 padding[3];
	};
	struct mt7902_mcu_mu_tx {
		u8 ver;
		u8 pad0;
		__le16 len;
		u8 bss_idx;
		u8 qos;
		u8 wmm_idx;
		u8 pad1;
		struct mu_edca edca[IEEE80211_NUM_ACS];
		u8 pad3[32];
	} __packed req_mu = {
		.bss_idx = mvif->bss_conf.mt76.idx,
		.qos = vif->bss_conf.qos,
		.wmm_idx = mvif->bss_conf.mt76.wmm_idx,
	};
	static const int to_aci[] = { 1, 0, 2, 3 };
	int ac, ret;

	for (ac = 0; ac < IEEE80211_NUM_ACS; ac++) {
		struct ieee80211_tx_queue_params *q = &mvif->bss_conf.queue_params[ac];
		struct edca *e = &req.edca[to_aci[ac]];

		e->aifs = cpu_to_le16(q->aifs);
		e->txop = cpu_to_le16(q->txop);

		if (q->cw_min)
			e->cw_min = cpu_to_le16(q->cw_min);
		else
			e->cw_min = cpu_to_le16(5);

		if (q->cw_max)
			e->cw_max = cpu_to_le16(q->cw_max);
		else
			e->cw_max = cpu_to_le16(10);
	}

	ret = mt76_mcu_send_msg(&dev->mt76, MCU_CE_CMD(SET_EDCA_PARMS), &req,
				sizeof(req), false);
	if (ret)
		return ret;

	if (!vif->bss_conf.he_support)
		return 0;

	for (ac = 0; ac < IEEE80211_NUM_ACS; ac++) {
		struct ieee80211_he_mu_edca_param_ac_rec *q;
		struct mu_edca *e;

		if (!mvif->bss_conf.queue_params[ac].mu_edca)
			break;

		q = &mvif->bss_conf.queue_params[ac].mu_edca_param_rec;
		e = &(req_mu.edca[to_aci[ac]]);

		e->cw_min = q->ecw_min_max & 0xf;
		e->cw_max = (q->ecw_min_max & 0xf0) >> 4;
		e->aifsn = q->aifsn;
		e->timer = q->mu_edca_timer;
	}

	return mt76_mcu_send_msg(&dev->mt76, MCU_CE_CMD(SET_MU_EDCA_PARMS),
				 &req_mu, sizeof(req_mu), false);
}

int mt7902_mcu_set_roc(struct mt792x_phy *phy, struct mt792x_vif *vif,
		       struct ieee80211_channel *chan, int duration,
		       enum mt7902_roc_req type, u8 token_id)
{
	printk(KERN_DEBUG "mcu.c - mt7902_mcu_set_roc");
	int center_ch = ieee80211_frequency_to_channel(chan->center_freq);
	struct mt792x_dev *dev = phy->dev;
	struct {
		struct {
			u8 rsv[4];
		} __packed hdr;
		struct roc_acquire_tlv {
			__le16 tag;
			__le16 len;
			u8 bss_idx;
			u8 tokenid;
			u8 control_channel;
			u8 sco;
			u8 band;
			u8 bw;
			u8 center_chan;
			u8 center_chan2;
			u8 bw_from_ap;
			u8 center_chan_from_ap;
			u8 center_chan2_from_ap;
			u8 reqtype;
			__le32 maxinterval;
			u8 dbdcband;
			u8 rsv[3];
		} __packed roc;
	} __packed req = {
		.roc = {
			.tag = cpu_to_le16(UNI_ROC_ACQUIRE),
			.len = cpu_to_le16(sizeof(struct roc_acquire_tlv)),
			.tokenid = token_id,
			.reqtype = type,
			.maxinterval = cpu_to_le32(duration),
			.bss_idx = vif->bss_conf.mt76.idx,
			.control_channel = chan->hw_value,
			.bw = CMD_CBW_20MHZ,
			.bw_from_ap = CMD_CBW_20MHZ,
			.center_chan = center_ch,
			.center_chan_from_ap = center_ch,
			.dbdcband = 0xff, /* auto */
		},
	};

	if (chan->hw_value < center_ch)
		req.roc.sco = 1; /* SCA */
	else if (chan->hw_value > center_ch)
		req.roc.sco = 3; /* SCB */

	switch (chan->band) {
	case NL80211_BAND_6GHZ:
		req.roc.band = 3;
		break;
	case NL80211_BAND_5GHZ:
		req.roc.band = 2;
		break;
	default:
		req.roc.band = 1;
		break;
	}

	return mt76_mcu_send_msg(&dev->mt76, MCU_UNI_CMD(ROC),
				 &req, sizeof(req), false);
}

int mt7902_mcu_abort_roc(struct mt792x_phy *phy, struct mt792x_vif *vif,
			 u8 token_id)
{
	printk(KERN_DEBUG "mcu.c - mt7902_mcu_abort_roc");
	struct mt792x_dev *dev = phy->dev;
	struct {
		struct {
			u8 rsv[4];
		} __packed hdr;
		struct roc_abort_tlv {
			__le16 tag;
			__le16 len;
			u8 bss_idx;
			u8 tokenid;
			u8 dbdcband;
			u8 rsv[5];
		} __packed abort;
	} __packed req = {
		.abort = {
			.tag = cpu_to_le16(UNI_ROC_ABORT),
			.len = cpu_to_le16(sizeof(struct roc_abort_tlv)),
			.tokenid = token_id,
			.bss_idx = vif->bss_conf.mt76.idx,
			.dbdcband = 0xff, /* auto*/
		},
	};

	return mt76_mcu_send_msg(&dev->mt76, MCU_UNI_CMD(ROC),
				 &req, sizeof(req), false);
}

int mt7902_mcu_set_chan_info(struct mt792x_phy *phy, int cmd)
{
	printk(KERN_DEBUG "mcu.c - mt7902_mcu_set_chan_info(phy, cmd: %d)", cmd);
	struct mt792x_dev *dev = phy->dev;
	struct cfg80211_chan_def *chandef = &phy->mt76->chandef;
	int freq1 = chandef->center_freq1;
	struct {
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
		u8 rsv1[57];
	} __packed req = {
		.control_ch = chandef->chan->hw_value,
		.center_ch = ieee80211_frequency_to_channel(freq1),
		.bw = mt76_connac_chan_bw(chandef),
		.tx_streams_num = hweight8(phy->mt76->antenna_mask),
		.rx_streams = phy->mt76->antenna_mask,
		.band_idx = phy != &dev->phy,
	};

	if (chandef->chan->band == NL80211_BAND_6GHZ)
		req.channel_band = 2;
	else
		req.channel_band = chandef->chan->band;

	if (cmd == MCU_EXT_CMD(SET_RX_PATH) ||
	    dev->mt76.hw->conf.flags & IEEE80211_CONF_MONITOR)
		req.switch_reason = CH_SWITCH_NORMAL;
	else if (phy->mt76->offchannel)
		req.switch_reason = CH_SWITCH_SCAN_BYPASS_DPD;
	else if (!cfg80211_reg_can_beacon(dev->mt76.hw->wiphy, chandef,
					  NL80211_IFTYPE_AP))
		req.switch_reason = CH_SWITCH_DFS;
	else
		req.switch_reason = CH_SWITCH_NORMAL;

	if (cmd == MCU_EXT_CMD(CHANNEL_SWITCH))
		req.rx_streams = hweight8(req.rx_streams);

	if (chandef->width == NL80211_CHAN_WIDTH_80P80) {
		int freq2 = chandef->center_freq2;

		req.center_ch2 = ieee80211_frequency_to_channel(freq2);
	}
	// printk(KERN_DEBUG "mcu.c - mt7902_mcu_set_chan_info - req.control_ch : %x", req.control_ch);
	// printk(KERN_DEBUG "mcu.c - mt7902_mcu_set_chan_info - req.center_ch : %x", req.center_ch);
	// printk(KERN_DEBUG "mcu.c - mt7902_mcu_set_chan_info - req.bw : %x", req.bw);
	// printk(KERN_DEBUG "mcu.c - mt7902_mcu_set_chan_info - req.tx_streams_num : %x", req.tx_streams_num);
	// printk(KERN_DEBUG "mcu.c - mt7902_mcu_set_chan_info - req.rx_streams : %x", req.rx_streams);
	// printk(KERN_DEBUG "mcu.c - mt7902_mcu_set_chan_info - req.band_idx : %x", req.band_idx);
	// printk(KERN_DEBUG "mcu.c - mt7902_mcu_set_chan_info - req.channel_band : %x", req.channel_band);
	// printk(KERN_DEBUG "mcu.c - mt7902_mcu_set_chan_info - req.switch_reason : %x", req.switch_reason);
	// printk(KERN_DEBUG "mcu.c - mt7902_mcu_set_chan_info - req.center_ch2 : %x", req.center_ch2);
	

	return mt76_mcu_send_msg(&dev->mt76, cmd, &req, sizeof(req), true);
}

int mt7902_mcu_set_eeprom(struct mt792x_dev *dev)
{
	printk(KERN_DEBUG "mcu.c - mt7902_mcu_set_eeprom(dev)");
	struct req_hdr {
		u8 buffer_mode;
		u8 format;
		__le16 len;
	} __packed req = {
		.buffer_mode = EE_MODE_EFUSE,
		.format = EE_FORMAT_WHOLE,
	};

	return mt76_mcu_send_msg(&dev->mt76, MCU_EXT_CMD(EFUSE_BUFFER_MODE),
				 &req, sizeof(req), true);
}
EXPORT_SYMBOL_GPL(mt7902_mcu_set_eeprom);

int mt7902_mcu_uni_bss_ps(struct mt792x_dev *dev, struct ieee80211_vif *vif)
{
	printk(KERN_DEBUG "mcu.c - mt7902_mcu_uni_bss_ps");
	printk(KERN_INFO "uni bss ps\n");
	struct mt792x_vif *mvif = (struct mt792x_vif *)vif->drv_priv;
	struct {
		struct {
			u8 bss_idx;
			u8 pad[3];
		} __packed hdr;
		struct ps_tlv {
			__le16 tag;
			__le16 len;
			u8 ps_state; /* 0: device awake
				      * 1: static power save
				      * 2: dynamic power saving
				      * 3: enter TWT power saving
				      * 4: leave TWT power saving
				      */
			u8 pad[3];
		} __packed ps;
	} __packed ps_req = {
		.hdr = {
			.bss_idx = mvif->bss_conf.mt76.idx,
		},
		.ps = {
			.tag = cpu_to_le16(UNI_BSS_INFO_PS),
			.len = cpu_to_le16(sizeof(struct ps_tlv)),
			.ps_state = vif->cfg.ps ? 2 : 0,
		},
	};

	if (vif->type != NL80211_IFTYPE_STATION)
		return -EOPNOTSUPP;

	return mt76_mcu_send_msg(&dev->mt76, MCU_UNI_CMD(BSS_INFO_UPDATE),
				 &ps_req, sizeof(ps_req), true);
}


static int
mt7902_mcu_uni_bss_bcnft(struct mt792x_dev *dev, struct ieee80211_vif *vif,
			 bool enable)
{
	struct mt792x_vif *mvif = (struct mt792x_vif *)vif->drv_priv;
	struct {
		struct {
			u8 bss_idx;
			u8 pad[3];
		} __packed hdr;
		struct bcnft_tlv {
			__le16 tag;
			__le16 len;
			__le16 bcn_interval;
			u8 dtim_period;
			u8 pad;
		} __packed bcnft;
	} __packed bcnft_req = {
		.hdr = {
			.bss_idx = mvif->bss_conf.mt76.idx,
		},
		.bcnft = {
			.tag = cpu_to_le16(UNI_BSS_INFO_BCNFT),
			.len = cpu_to_le16(sizeof(struct bcnft_tlv)),
			.bcn_interval = cpu_to_le16(vif->bss_conf.beacon_int),
			.dtim_period = vif->bss_conf.dtim_period,
		},
	};

	if (vif->type != NL80211_IFTYPE_STATION)
		return 0;

	return mt76_mcu_send_msg(&dev->mt76, MCU_UNI_CMD(BSS_INFO_UPDATE),
				 &bcnft_req, sizeof(bcnft_req), true);
}

int
mt7902_mcu_set_bss_pm(struct mt792x_dev *dev, struct ieee80211_vif *vif,
		      bool enable)
{
	printk(KERN_DEBUG "mcu.c - mt7902_mcu_set_bss_pm");
	struct mt792x_vif *mvif = (struct mt792x_vif *)vif->drv_priv;
	struct {
		u8 bss_idx;
		u8 dtim_period;
		__le16 aid;
		__le16 bcn_interval;
		__le16 atim_window;
		u8 uapsd;
		u8 bmc_delivered_ac;
		u8 bmc_triggered_ac;
		u8 pad;
	} req = {
		.bss_idx = mvif->bss_conf.mt76.idx,
		.aid = cpu_to_le16(vif->cfg.aid),
		.dtim_period = vif->bss_conf.dtim_period,
		.bcn_interval = cpu_to_le16(vif->bss_conf.beacon_int),
	};
	struct {
		u8 bss_idx;
		u8 pad[3];
	} req_hdr = {
		.bss_idx = mvif->bss_conf.mt76.idx,
	};
	int err;

	err = mt76_mcu_send_msg(&dev->mt76, MCU_CE_CMD(SET_BSS_ABORT),
				&req_hdr, sizeof(req_hdr), false);
	if (err < 0 || !enable)
		return err;

	return mt76_mcu_send_msg(&dev->mt76, MCU_CE_CMD(SET_BSS_CONNECTED),
				 &req, sizeof(req), false);
}

int mt7902_mcu_sta_update(struct mt792x_dev *dev, struct ieee80211_sta *sta,
			  struct ieee80211_vif *vif, bool enable,
			  enum mt76_sta_info_state state)
{
	printk(KERN_DEBUG "mcu.c - mt7902_mcu_sta_update");
	struct mt792x_vif *mvif = (struct mt792x_vif *)vif->drv_priv;
	int rssi = -ewma_rssi_read(&mvif->bss_conf.rssi);
	struct mt76_sta_cmd_info info = {
		.sta = sta,
		.vif = vif,
		.enable = enable,
		.cmd = MCU_EXT_CMD(STA_REC_UPDATE),
		.state = state,
		.offload_fw = true,
		.rcpi = to_rcpi(rssi),
	};
	struct mt792x_sta *msta;

	msta = sta ? (struct mt792x_sta *)sta->drv_priv : NULL;
	info.wcid = msta ? &msta->deflink.wcid : &mvif->sta.deflink.wcid;
	info.newly = msta ? state != MT76_STA_INFO_STATE_ASSOC : true;

	return mt76_connac_mcu_sta_cmd(&dev->mphy, &info);
} 

int mt7902_mcu_set_beacon_filter(struct mt792x_dev *dev,
				 struct ieee80211_vif *vif,
				 bool enable)
{
	printk(KERN_DEBUG "mcu.c - mt7902_mcu_set_beacon_filter(dev, vif, enable: %d)", enable);
#define MT7902_FIF_BIT_CLR		BIT(1)
#define MT7902_FIF_BIT_SET		BIT(0)
	int err;

	if (enable) {
		err = mt7902_mcu_uni_bss_bcnft(dev, vif, true);
		if (err)
			return err;

		err = mt7902_mcu_set_rxfilter(dev, 0,
					      MT7902_FIF_BIT_SET,
					      MT_WF_RFCR_DROP_OTHER_BEACON);
		if (err)
			return err;

		return 0;
	}

	err = mt7902_mcu_set_bss_pm(dev, vif, false);
	if (err)
		return err;

	err = mt7902_mcu_set_rxfilter(dev, 0,
				      MT7902_FIF_BIT_CLR,
				      MT_WF_RFCR_DROP_OTHER_BEACON);
	if (err)
		return err;

	return 0;
}

int mt7902_get_txpwr_info(struct mt792x_dev *dev, struct mt7902_txpwr *txpwr)
{
	printk(KERN_DEBUG "mcu.c - mt7902_get_txpwr_info");
	struct mt7902_txpwr_event *event;
	struct mt7902_txpwr_req req = {
		.dbdc_idx = 0,
	};
	struct sk_buff *skb;
	int ret;

	ret = mt76_mcu_send_and_get_msg(&dev->mt76, MCU_CE_CMD(GET_TXPWR),
					&req, sizeof(req), true, &skb);
	if (ret)
		return ret;

	event = (struct mt7902_txpwr_event *)skb->data;
	WARN_ON(skb->len != le16_to_cpu(event->len));
	memcpy(txpwr, &event->txpwr, sizeof(event->txpwr));

	dev_kfree_skb(skb);

	return 0;
}

int mt7902_mcu_set_sniffer(struct mt792x_dev *dev, struct ieee80211_vif *vif,
			   bool enable)
{
	printk(KERN_DEBUG "mcu.c - mt7902_mcu_set_sniffer");
	struct mt76_vif_link *mvif = (struct mt76_vif_link *)vif->drv_priv;
	struct {
		struct {
			u8 band_idx;
			u8 pad[3];
		} __packed hdr;
		struct sniffer_enable_tlv {
			__le16 tag;
			__le16 len;
			u8 enable;
			u8 pad[3];
		} __packed enable;
	} req = {
		.hdr = {
			.band_idx = mvif->band_idx,
		},
		.enable = {
			.tag = cpu_to_le16(0),
			.len = cpu_to_le16(sizeof(struct sniffer_enable_tlv)),
			.enable = enable,
		},
	};

	return mt76_mcu_send_msg(&dev->mt76, MCU_UNI_CMD(SNIFFER), &req, sizeof(req),
				 true);
}

int mt7902_mcu_config_sniffer(struct mt792x_vif *vif,
			      struct ieee80211_chanctx_conf *ctx)
{
	printk(KERN_DEBUG "mcu.c - mt7902_mcu_config_sniffer");
	struct cfg80211_chan_def *chandef = &ctx->def;
	int freq1 = chandef->center_freq1, freq2 = chandef->center_freq2;
	static const u8 ch_band[] = {
		[NL80211_BAND_2GHZ] = 1,
		[NL80211_BAND_5GHZ] = 2,
		[NL80211_BAND_6GHZ] = 3,
	};
	static const u8 ch_width[] = {
		[NL80211_CHAN_WIDTH_20_NOHT] = 0,
		[NL80211_CHAN_WIDTH_20] = 0,
		[NL80211_CHAN_WIDTH_40] = 0,
		[NL80211_CHAN_WIDTH_80] = 1,
		[NL80211_CHAN_WIDTH_160] = 2,
		[NL80211_CHAN_WIDTH_80P80] = 3,
		[NL80211_CHAN_WIDTH_5] = 4,
		[NL80211_CHAN_WIDTH_10] = 5,
		[NL80211_CHAN_WIDTH_320] = 6,
	};
	struct {
		struct {
			u8 band_idx;
			u8 pad[3];
		} __packed hdr;
		struct config_tlv {
			__le16 tag;
			__le16 len;
			u16 aid;
			u8 ch_band;
			u8 bw;
			u8 control_ch;
			u8 sco;
			u8 center_ch;
			u8 center_ch2;
			u8 drop_err;
			u8 pad[3];
		} __packed tlv;
	} __packed req = {
		.hdr = {
			.band_idx = vif->bss_conf.mt76.band_idx,
		},
		.tlv = {
			.tag = cpu_to_le16(1),
			.len = cpu_to_le16(sizeof(req.tlv)),
			.control_ch = chandef->chan->hw_value,
			.center_ch = ieee80211_frequency_to_channel(freq1),
			.drop_err = 1,
		},
	};
	if (chandef->chan->band < ARRAY_SIZE(ch_band))
		req.tlv.ch_band = ch_band[chandef->chan->band];
	if (chandef->width < ARRAY_SIZE(ch_width))
		req.tlv.bw = ch_width[chandef->width];

	if (freq2)
		req.tlv.center_ch2 = ieee80211_frequency_to_channel(freq2);

	if (req.tlv.control_ch < req.tlv.center_ch)
		req.tlv.sco = 1; /* SCA */
	else if (req.tlv.control_ch > req.tlv.center_ch)
		req.tlv.sco = 3; /* SCB */

	return mt76_mcu_send_msg(vif->phy->mt76->dev, MCU_UNI_CMD(SNIFFER),
				 &req, sizeof(req), true);
}

int
mt7902_mcu_uni_add_beacon_offload(struct mt792x_dev *dev,
				  struct ieee80211_hw *hw,
				  struct ieee80211_vif *vif,
				  bool enable)
{
	printk(KERN_DEBUG "mcu.c - mt7902_mcu_uni_add_beacon_offload");
	printk(KERN_INFO "uni beacon offload\n");
	struct mt792x_vif *mvif = (struct mt792x_vif *)vif->drv_priv;
	struct mt76_wcid *wcid = &dev->mt76.global_wcid;
	struct ieee80211_mutable_offsets offs;
	struct {
		struct req_hdr {
			u8 bss_idx;
			u8 pad[3];
		} __packed hdr;
		struct bcn_content_tlv {
			__le16 tag;
			__le16 len;
			__le16 tim_ie_pos;
			__le16 csa_ie_pos;
			__le16 bcc_ie_pos;
			/* 0: disable beacon offload
			 * 1: enable beacon offload
			 * 2: update probe respond offload
			 */
			u8 enable;
			/* 0: legacy format (TXD + payload)
			 * 1: only cap field IE
			 */
			u8 type;
			__le16 pkt_len;
			u8 pkt[512];
		} __packed beacon_tlv;
	} req = {
		.hdr = {
			.bss_idx = mvif->bss_conf.mt76.idx,
		},
		.beacon_tlv = {
			.tag = cpu_to_le16(UNI_BSS_INFO_BCN_CONTENT),
			.len = cpu_to_le16(sizeof(struct bcn_content_tlv)),
			.enable = enable,
		},
	};
	struct sk_buff *skb;

	/* support enable/update process only
	 * disable flow would be handled in bss stop handler automatically
	 */
	if (!enable)
		return -EOPNOTSUPP;

	skb = ieee80211_beacon_get_template(mt76_hw(dev), vif, &offs, 0);
	if (!skb)
		return -EINVAL;

	if (skb->len > 512 - MT_TXD_SIZE) {
		dev_err(dev->mt76.dev, "beacon size limit exceed\n");
		dev_kfree_skb(skb);
		return -EINVAL;
	}

	mt76_connac2_mac_write_txwi(&dev->mt76, (__le32 *)(req.beacon_tlv.pkt),
				    skb, wcid, NULL, 0, 0, BSS_CHANGED_BEACON);
	memcpy(req.beacon_tlv.pkt + MT_TXD_SIZE, skb->data, skb->len);
	req.beacon_tlv.pkt_len = cpu_to_le16(MT_TXD_SIZE + skb->len);
	req.beacon_tlv.tim_ie_pos = cpu_to_le16(MT_TXD_SIZE + offs.tim_offset);

	if (offs.cntdwn_counter_offs[0]) {
		u16 csa_offs;

		csa_offs = MT_TXD_SIZE + offs.cntdwn_counter_offs[0] - 4;
		req.beacon_tlv.csa_ie_pos = cpu_to_le16(csa_offs);
	}
	dev_kfree_skb(skb);

	return mt76_mcu_send_msg(&dev->mt76, MCU_UNI_CMD(BSS_INFO_UPDATE),
				 &req, sizeof(req), true);
}

static
int __mt7902_mcu_set_clc(struct mt792x_dev *dev, u8 *alpha2,
			 enum environment_cap env_cap,
			 struct mt7902_clc *clc,
			 u8 idx)
{
	printk(KERN_DEBUG "mcu.c - __mt7902_mcu_set_clc(dev, alpha2: 0x%02x, env_cap, clc, idx: 0x%02x)", alpha2, idx);
#define CLC_CAP_EVT_EN BIT(0)
#define CLC_CAP_DTS_EN BIT(1)
	struct sk_buff *skb, *ret_skb = NULL;
	struct {
		u8 ver;
		u8 pad0;
		__le16 len;
		u8 idx;
		u8 env;
		u8 acpi_conf;
		u8 cap;
		u8 alpha2[2];
		u8 type[2];
		u8 env_6g;
		u8 mtcl_conf;
		u8 rsvd[62];
	} __packed req = {
		.ver = 1,
		.idx = idx,
		.env = env_cap,
		.env_6g = dev->phy.power_type,
		.acpi_conf = mt792x_acpi_get_flags(&dev->phy),
		.mtcl_conf = mt792x_acpi_get_mtcl_conf(&dev->phy, alpha2),
	};
	int ret, valid_cnt = 0;
	u32 buf_len = 0;
	u8 *pos;

	if (!clc)
		return 0;

	if (dev->phy.chip_cap & MT792x_CHIP_CAP_CLC_EVT_EN)
		req.cap |= CLC_CAP_EVT_EN;
	if (mt76_find_power_limits_node(&dev->mt76))
		req.cap |= CLC_CAP_DTS_EN;

	buf_len = le32_to_cpu(clc->len) - sizeof(*clc);
	pos = clc->data;
	while (buf_len > 16) {
		struct mt7902_clc_rule *rule = (struct mt7902_clc_rule *)pos;
		u16 len = le16_to_cpu(rule->len);
		u16 offset = len + sizeof(*rule);

		pos += offset;
		buf_len -= offset;
		if (rule->alpha2[0] != alpha2[0] ||
		    rule->alpha2[1] != alpha2[1])
			continue;

		memcpy(req.alpha2, rule->alpha2, 2);
		memcpy(req.type, rule->type, 2);

		req.len = cpu_to_le16(sizeof(req) + len);
		skb = __mt76_mcu_msg_alloc(&dev->mt76, &req,
					   le16_to_cpu(req.len),
					   sizeof(req), GFP_KERNEL);
		if (!skb)
			return -ENOMEM;
		skb_put_data(skb, rule->data, len);

		ret = mt76_mcu_skb_send_and_get_msg(&dev->mt76, skb,
						    MCU_CE_CMD(SET_CLC),
						    !!(req.cap & CLC_CAP_EVT_EN),
						    &ret_skb);
		if (ret < 0)
			return ret;

		if (ret_skb) {
			struct mt7902_clc_info_tlv *info;

			info = (struct mt7902_clc_info_tlv *)(ret_skb->data + 4);
			dev->phy.clc_chan_conf = info->chan_conf;
			dev_kfree_skb(ret_skb);
		}

		valid_cnt++;
	}

	if (!valid_cnt)
		return -ENOENT;

	return 0;
}

int mt7902_mcu_set_clc(struct mt792x_dev *dev, u8 *alpha2,
		       enum environment_cap env_cap)
{
	printk(KERN_DEBUG "mcu.c - mt7902_mcu_set_clc(dev, alpha2: 0x%02x, env_cap)", alpha2);
	struct mt792x_phy *phy = (struct mt792x_phy *)&dev->phy;
	int i, ret;

	/* submit all clc config */
	for (i = 0; i < ARRAY_SIZE(phy->clc); i++) {
		ret = __mt7902_mcu_set_clc(dev, alpha2, env_cap,
					   phy->clc[i], i);

		/* If no country found, set "00" as default */
		if (ret == -ENOENT)
			ret = __mt7902_mcu_set_clc(dev, "00",
						   ENVIRON_INDOOR,
						   phy->clc[i], i);
		if (ret < 0)
			return ret;
	}
	return 0;
}

int mt7902_mcu_get_temperature(struct mt792x_phy *phy)
{
	printk(KERN_DEBUG "mcu.c - mt7902_mcu_get_temperature(phy)");
	struct mt792x_dev *dev = phy->dev;
	struct {
		u8 ctrl_id;
		u8 action;
		u8 band_idx;
		u8 rsv[5];
	} req = {
		.ctrl_id = THERMAL_SENSOR_TEMP_QUERY,
		.band_idx = phy->mt76->band_idx,
	};

	return mt76_mcu_send_msg(&dev->mt76, MCU_EXT_CMD(THERMAL_CTRL), &req,
				 sizeof(req), true);
}

int mt7902_mcu_wf_rf_pin_ctrl(struct mt792x_phy *phy, u8 action)
{
	printk(KERN_DEBUG "mcu.c - mt7902_mcu_wf_rf_pin_ctrl");
	struct mt792x_dev *dev = phy->dev;
	struct {
		u8 action;
		u8 value;
	} req = {
		.action = action,
		.value = 0,
	};

	return mt76_mcu_send_msg(&dev->mt76, MCU_EXT_CMD(WF_RF_PIN_CTRL), &req,
				 sizeof(req), action ? true : false);
}

int mt7902_mcu_set_rxfilter(struct mt792x_dev *dev, u32 fif,
			    u8 bit_op, u32 bit_map)
{
	printk(KERN_DEBUG "mcu.c - mt7902_mcu_set_rxfilter(dev, fif: 0x%08x, bit_op: 0x%02x, bit_map: 0x%08x)", fif, bit_op, bit_map);
	struct {
		u8 rsv[4];
		u8 mode;
		u8 rsv2[3];
		__le32 fif;
		__le32 bit_map; /* bit_* for bitmap update */
		u8 bit_op;
		u8 pad[51];
	} __packed data = {
		.mode = fif ? 1 : 2,
		.fif = cpu_to_le32(fif),
		.bit_map = cpu_to_le32(bit_map),
		.bit_op = bit_op,
	};

	return mt76_mcu_send_msg(&dev->mt76, MCU_CE_CMD(SET_RX_FILTER),
				 &data, sizeof(data), false);
}

int mt7902_mcu_set_rssimonitor(struct mt792x_dev *dev, struct ieee80211_vif *vif)
{
	printk(KERN_DEBUG "mcu.c - mt7902_mcu_set_rssimonitor");
	struct mt792x_vif *mvif = (struct mt792x_vif *)vif->drv_priv;
	struct {
		u8 enable;
		s8 cqm_rssi_high;
		s8 cqm_rssi_low;
		u8 bss_idx;
		u16 duration;
		u8 rsv2[2];
	} __packed data = {
		.enable = vif->cfg.assoc,
		.cqm_rssi_high = vif->bss_conf.cqm_rssi_thold + vif->bss_conf.cqm_rssi_hyst,
		.cqm_rssi_low = vif->bss_conf.cqm_rssi_thold - vif->bss_conf.cqm_rssi_hyst,
		.bss_idx = mvif->bss_conf.mt76.idx,
	};

	return mt76_mcu_send_msg(&dev->mt76, MCU_CE_CMD(RSSI_MONITOR),
				 &data, sizeof(data), false);
};

int mt7902_mcu_add_dev_info(struct mt76_phy *phy,
			    struct ieee80211_bss_conf *bss_conf,
                struct mt76_vif_link *mvif, bool enable)
{
	printk(KERN_DEBUG "mcu.c - mt7902_mcu_add_dev_info(phy, bss_conf, mvif, enable: %d)", enable);
	struct mt76_dev *dev = phy->dev;
	//struct mt792x_vif *mvif = (struct mt792x_vif *)vif->drv_priv;
	struct {
		struct req_hdr { 
			u8 omac_idx;
			u8 band_idx;
			__le16 tlv_num;
			u8 is_tlv_append;
			//u8 dbdc_idx;
			u8 rsv[3];
		} __packed hdr;
		struct req_tlv {
			__le16 tag;
			__le16 len;
			u8 active;
			u8 band_idx;
			//u8 __rsv;
			u8 omac_addr[ETH_ALEN];
		} __packed tlv;
	} data = {
		.hdr = {
			.omac_idx = mvif->omac_idx,
			.band_idx = mvif->band_idx,
			.tlv_num = cpu_to_le16(1),
			.is_tlv_append = 1,
			//.dbdc_idx = mvif->bss_conf.mt76.band_idx
		},
		.tlv = {
			.tag = cpu_to_le16(DEV_INFO_ACTIVE),
			.len = cpu_to_le16(sizeof(struct req_tlv)),
			.active = enable,
			.band_idx = mvif->band_idx,
		},
	};

	// if(mvif->bss_conf.mt76.omac_idx >= REPEATER_BSSID_START)
	// 	return mt7902_mcu_muar_config(phy, vif, false, enable);
	
    memcpy(data.tlv.omac_addr, bss_conf->addr, ETH_ALEN);
	// printk(KERN_DEBUG "mcu.c - mt7902_mcu_add_dev_info - data.hdr.omac_idx : %x", data.hdr.omac_idx);
	// printk(KERN_DEBUG "mcu.c - mt7902_mcu_add_dev_info - data.hdr.band_idx : %x", data.hdr.band_idx);
	// printk(KERN_DEBUG "mcu.c - mt7902_mcu_add_dev_info - data.hdr.is_tlv_append : %x", data.hdr.is_tlv_append);
	// printk(KERN_DEBUG "mcu.c - mt7902_mcu_add_dev_info - data.tlv.active : %x", data.tlv.active);
	// printk(KERN_DEBUG "mcu.c - mt7902_mcu_add_dev_info - data.tlv.band_idx : %x", data.tlv.band_idx);

	return mt76_mcu_send_msg(dev, MCU_EXT_CMD(DEV_INFO_UPDATE),
				 &data, sizeof(data), true);
};

/** bss info **/
struct mt7902_he_obss_narrow_bw_ru_data {
	bool tolerated;
};

static void mt7902_check_he_obss_narrow_bw_ru_iter(struct wiphy *wiphy,
						   struct cfg80211_bss *bss,
						   void *_data)
{
	printk(KERN_DEBUG "mcu.c - mt7902_check_he_obss_narrow_bw_ru_iter");
	struct mt7902_he_obss_narrow_bw_ru_data *data = _data;
	const struct element *elem;

	rcu_read_lock();
	elem = ieee80211_bss_get_elem(bss, WLAN_EID_EXT_CAPABILITY);

	if (!elem || elem->datalen <= 10 ||
	    !(elem->data[10] &
	      WLAN_EXT_CAPA10_OBSS_NARROW_BW_RU_TOLERANCE_SUPPORT))
		data->tolerated = false;

	rcu_read_unlock();
};

static bool mt7902_check_he_obss_narrow_bw_ru(struct ieee80211_hw *hw,
					      struct ieee80211_vif *vif)
{
	printk(KERN_DEBUG "mcu.c - mt7902_check_he_obss_narrow_bw_ru");
	struct mt7902_he_obss_narrow_bw_ru_data iter_data = {
		.tolerated = true,
	};

	if (!(vif->bss_conf.chanreq.oper.chan->flags & IEEE80211_CHAN_RADAR))
		return false;

	cfg80211_bss_iter(hw->wiphy, &vif->bss_conf.chanreq.oper,
			  mt7902_check_he_obss_narrow_bw_ru_iter,
			  &iter_data);

	/*
	 * If there is at least one AP on radar channel that cannot
	 * tolerate 26-tone RU UL OFDMA transmissions using HE TB PPDU.
	 */
	return !iter_data.tolerated;
};

static inline u8 mt7902_get_band(enum nl80211_band band)
{
	static const u8 convert_to_fw[] = {
		[NL80211_BAND_2GHZ] = CMD_BAND_24G,
		[NL80211_BAND_5GHZ] = CMD_BAND_5G,
		[NL80211_BAND_6GHZ] = CMD_BAND_6G,
	};

	if (band >= ARRAY_SIZE(convert_to_fw))
		return 0;

	return convert_to_fw[band];
};





static int
mt7902_mcu_bss_basic_tlv(struct sk_buff *skb,
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

	tlv = mt76_connac_mcu_add_tlv(skb, UNI_BSS_INFO_BASIC, sizeof(*bss));

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



static void
mt7902_mcu_bss_sec_tlv(struct sk_buff *skb, struct ieee80211_vif *vif)
{
	struct mt76_vif *mvif = (struct mt76_vif *)vif->drv_priv;
	struct bss_sec_tlv *sec;
	struct tlv *tlv;

	tlv = mt76_connac_mcu_add_tlv(skb, UNI_BSS_INFO_SEC, sizeof(*sec));

	sec = (struct bss_sec_tlv *)tlv;
	sec->cipher = mvif->cipher;
}


static void
mt7902_mcu_bss_rfch_tlv(struct sk_buff *skb, struct ieee80211_vif *vif,
			struct mt792x_phy *phy)
{
	struct cfg80211_chan_def *chandef = &phy->mt76->chandef;
	struct bss_info_rf_ch *ch;
	struct tlv *tlv;
	int freq1 = chandef->center_freq1;

	tlv = mt76_connac_mcu_add_tlv(skb, BSS_INFO_RF_CH, sizeof(*ch));

	ch = (struct bss_info_rf_ch *)tlv;
	ch->pri_ch = chandef->chan->hw_value;
	ch->center_ch0 = ieee80211_frequency_to_channel(freq1);
	ch->bw = mt76_connac_chan_bw(chandef);
	//ch->tx_streams = hweight8(phy->mt76->antenna_mask);
	//ch->rx_streams = hweight8(phy->mt76->antenna_mask);
	//ch->band = mt7902_get_band(chandef->chan->band);

	if (chandef->width == NL80211_CHAN_WIDTH_80P80) {
		int freq2 = chandef->center_freq2;

		ch->center_ch1 = ieee80211_frequency_to_channel(freq2);
	}

	if (vif->bss_conf.he_support && vif->type == NL80211_IFTYPE_STATION) {
		struct mt76_phy *mphy = phy->mt76;

		ch->he_ru26_block =
			mt7902_check_he_obss_narrow_bw_ru(mphy->hw, vif);
		ch->he_all_disable = false;
	} else {
		ch->he_all_disable = true;
	}
}




static void
mt7902_mcu_bss_ra_tlv(struct sk_buff *skb, struct ieee80211_vif *vif,
		      struct mt792x_phy *phy)
{
	int max_nss = hweight8(phy->mt76->antenna_mask);
	struct bss_info_ra *ra;
	struct tlv *tlv;

	tlv = mt76_connac_mcu_add_tlv(skb, BSS_INFO_RA, sizeof(*ra));

	ra = (struct bss_info_ra *)tlv;
	ra->op_mode = vif->type == NL80211_IFTYPE_AP;
	ra->adhoc_en = vif->type == NL80211_IFTYPE_ADHOC;
	ra->short_preamble = true;
	ra->tx_streams = max_nss;
	ra->rx_streams = max_nss;
	ra->algo = 4;
	ra->train_up_rule = 2;
	ra->train_up_high_thres = 110;
	ra->train_up_rule_rssi = -70;
	ra->low_traffic_thres = 2;
	ra->phy_cap = cpu_to_le32(0xfdf);
	ra->interval = cpu_to_le32(500);
	ra->fast_interval = cpu_to_le32(100);
}

static void
mt7902_mcu_bss_he_tlv(struct sk_buff *skb, struct ieee80211_vif *vif,
		      struct mt792x_phy *phy)
{
#define DEFAULT_HE_PE_DURATION		4
#define DEFAULT_HE_DURATION_RTS_THRES	1023
	const struct ieee80211_sta_he_cap *cap;
	struct bss_info_he *he;
	struct tlv *tlv;

	cap = mt76_connac_get_he_phy_cap(phy->mt76, vif);

	tlv = mt76_connac_mcu_add_tlv(skb, BSS_INFO_HE_BASIC, sizeof(*he));

	he = (struct bss_info_he *)tlv;
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
mt7902_mcu_bss_hw_amsdu_tlv(struct sk_buff *skb)
{
#define TXD_CMP_MAP1		GENMASK(15, 0)
#define TXD_CMP_MAP2		(GENMASK(31, 0) & ~BIT(23))
	struct bss_info_hw_amsdu *amsdu;
	struct tlv *tlv;

	tlv = mt76_connac_mcu_add_tlv(skb, BSS_INFO_HW_AMSDU, sizeof(*amsdu));

	amsdu = (struct bss_info_hw_amsdu *)tlv;
	amsdu->cmp_bitmap_0 = cpu_to_le32(TXD_CMP_MAP1);
	amsdu->cmp_bitmap_1 = cpu_to_le32(TXD_CMP_MAP2);
	amsdu->trig_thres = cpu_to_le16(2);
	amsdu->enable = true;
}

static void
mt7902_mcu_bss_bmc_tlv(struct sk_buff *skb, struct mt792x_phy *phy)
{
	struct bss_info_bmc_rate *bmc;
	struct cfg80211_chan_def *chandef = &phy->mt76->chandef;
	enum nl80211_band band = chandef->chan->band;
	struct tlv *tlv;

	tlv = mt76_connac_mcu_add_tlv(skb, BSS_INFO_BMC_RATE, sizeof(*bmc));

	bmc = (struct bss_info_bmc_rate *)tlv;
	if (band == NL80211_BAND_2GHZ) {
		bmc->short_preamble = true;
	} else {
		bmc->bc_trans = cpu_to_le16(0x2000);
		bmc->mc_trans = cpu_to_le16(0x2080);
	}
}

int mt7902_mcu_add_bss_info(struct mt792x_phy *phy,
			    struct ieee80211_vif *vif, int enable)
{
	printk(KERN_DEBUG "mcu.c - mt7902_mcu_add_bss_info(phy, vif, enable: %d)", enable);
	struct mt792x_vif *mvif = (struct mt792x_vif *)vif->drv_priv;
	struct mt792x_dev *dev = phy->dev;
	struct sk_buff *skb;


	// if (mvif->omac_idx >= REPEATER_BSSID_START) {
	// 	mt7902_mcu_muar_config(phy, vif, false, enable);
	// 	mt7902_mcu_muar_config(phy, vif, true, enable);
	// }
	
	skb = __mt76_connac_mcu_alloc_sta_req(&dev->mt76, &mvif->bss_conf.mt76, NULL,
					      MT7902_BSS_UPDATE_MAX_SIZE);
	if (IS_ERR(skb))
		return PTR_ERR(skb);

	/* bss_basic must be first */
	mt76_connac_mcu_bss_basic_tlv(skb, vif, NULL, phy->mt76,
				      mvif->sta.deflink.wcid.idx, enable);
	//mt7902_mcu_bss_basic_tlv(skb, vif, NULL, phy->mt76,
	//			mvif->sta.deflink.wcid.idx, enable);
	mt7902_mcu_bss_sec_tlv(skb, vif);        //7925

	if (vif->type == NL80211_IFTYPE_MONITOR)
		goto out;

	if (enable) {
		mt7902_mcu_bss_rfch_tlv(skb, vif, phy);  //7915
		mt7902_mcu_bss_bmc_tlv(skb, phy);   //7925
		mt7902_mcu_bss_ra_tlv(skb, vif, phy);   //7915
		mt7902_mcu_bss_hw_amsdu_tlv(skb);

		mt7902_mcu_bss_txcmd_tlv(skb, true);   //7996

		// if (vif->bss_conf.he_support)
		// 	mt7902_mcu_bss_he_tlv(skb, vif, phy);   //7925

		/* all mt7902 ic need this tlv, no matter it supports EHT or not */
		mt7902_mcu_bss_mld_tlv(skb);   //7925
	}
out:
	return mt76_mcu_skb_send_msg(&dev->mt76, skb,
				     MCU_EXT_CMD(BSS_INFO_UPDATE), true);
};

static void
mt7902_mcu_bss_txcmd_tlv(struct sk_buff *skb, bool en)
{
	printk(KERN_DEBUG "mcu.c - mt7902_mcu_bss_txcmd_tlv");
	struct bss_txcmd_tlv *txcmd;
	struct tlv *tlv;

	tlv = mt76_connac_mcu_add_tlv(skb, UNI_BSS_INFO_TXCMD, sizeof(*txcmd));

	txcmd = (struct bss_txcmd_tlv *)tlv;
	txcmd->txcmd_mode = en;
}


static void
mt7902_mcu_bss_mld_tlv(struct sk_buff *skb)
{
	struct bss_mld_tlv *mld;
	struct tlv *tlv;

	tlv = mt76_connac_mcu_add_tlv(skb, UNI_BSS_INFO_MLD, sizeof(*mld));

	mld = (struct bss_mld_tlv *)tlv;
	mld->group_mld_id = 0xff;
	mld->remap_idx = 0xff;
}

/*
static struct tlv *
mt7902_mcu_add_uni_tlv(struct sk_buff *skb, int tag, int len)
{
	printk(KERN_DEBUG "mcu.c - mt7902_mcu_add_uni_tlv");
	struct tlv *ptlv, tlv = {
		.tag = cpu_to_le16(tag),
		.len = cpu_to_le16(len),
	};

	ptlv = skb_put(skb, len);
	memcpy(ptlv, &tlv, sizeof(tlv));

	return ptlv;
}
*/



int mt7902_mcu_add_sta(struct mt792x_dev *dev, struct ieee80211_vif *vif,
		       struct ieee80211_sta *sta, int conn_state, bool newly)
{
	printk(KERN_DEBUG "mcu.c - mt7902_mcu_add_sta(dev, vif, sta, enable: %d)", newly);

	
	struct mt792x_vif *mvif = (struct mt792x_vif *)vif->drv_priv;
	struct mt792x_sta *msta;
	struct sk_buff *skb;
	struct ieee80211_link_sta *link_sta;
	int ret;

	msta = sta ? (struct mt792x_sta *)sta->drv_priv : &mvif->sta;
	link_sta = sta ? &sta->deflink : NULL;

	skb = mt76_connac_mcu_alloc_sta_req(&dev->mt76, &mvif->bss_conf.mt76,
					    &msta->deflink.wcid);
	if (IS_ERR(skb))
		return PTR_ERR(skb);

	
	//conn_state = enable ? CONN_STATE_PORT_SECURE : CONN_STATE_DISCONNECT;

	/* starec basic */
	mt76_connac_mcu_sta_basic_tlv(&dev->mt76, skb, &vif->bss_conf, link_sta, conn_state, newly);
	//if (!enable)
	//	goto out;

	/* tag order is in accordance with firmware dependency. */
	if (sta) {
		// starec phy   physical layer information
		//if (mt76_chip(&dev->mt76) != 0x7902)
		//	mt7902_mcu_sta_phy_tlv(dev, skb, vif, sta);  

		// starec bfer   Beamforming Reference Information
		mt7902_mcu_sta_bfer_tlv(dev, skb, vif, sta);
		// starec ht High Thoughput Information
		mt7902_mcu_sta_ht_tlv(skb, sta);
		// starec vht Very High Thoughput Information
		mt7902_mcu_sta_vht_tlv(skb, sta);
		// starec uapsd   U-APSD (Unscheduled Automatic Power Save Delivery) for a specific station (STA).
		mt76_connac_mcu_sta_uapsd(skb, vif, sta);
		// starec amsdu A-MSDU (Aggregated MAC Service Data Unit) information
		mt7902_mcu_sta_amsdu_tlv(dev, skb, vif, sta);
		// starec he  High Efficiency information
		mt7902_mcu_sta_he_tlv(skb, sta, vif);
		// starec he 6g*  High Efficiency Information for 6 GHz channel
		mt7902_mcu_sta_he_6g_tlv(skb, sta, vif);
		// starec muru   Multi-User Resource Unit (MU-RU) information
		mt7902_mcu_sta_muru_tlv(skb, sta, vif);
		// starec bfee     Beamforming Feedback (BFEE) information
		mt7902_mcu_sta_bfee_tlv(dev, skb, vif, sta);
		// starec hdr trans       header translation
		mt7902_mcu_sta_hdr_trans_tlv(dev, skb, vif, sta);
	}

	ret = mt7902_mcu_add_group(dev, vif, sta);
	if (ret) {
		dev_kfree_skb(skb);
		return ret;
	} 
out:
	return mt76_mcu_skb_send_msg(&dev->mt76, skb,
				     MCU_EXT_CMD(STA_REC_UPDATE), true);
	
} 

static void
mt7902_mcu_sta_bfer_tlv(struct mt792x_dev *dev, struct sk_buff *skb,
		       struct ieee80211_vif *vif, struct ieee80211_sta *sta)
{
	struct mt792x_vif *mvif = (struct mt792x_vif *)vif->drv_priv;
	struct mt792x_phy *phy = mvif->phy;
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

	if (!(sta->deflink.ht_cap.ht_supported || sta->deflink.he_cap.has_he))
		return;

	// ebf = mt7902_is_ebf_supported(phy, vif, sta, false);
	// if (!ebf && !dev->ibf)
	// 	return;

	tlv = mt76_connac_mcu_add_tlv(skb, STA_REC_BF, sizeof(*bf));
	bf = (struct sta_rec_bf *)tlv;

	/* he: eBF only, in accordance with spec
	 * vht: support eBF and iBF
	 * ht: iBF only, since mac80211 lacks of eBF support
	 */
	if (sta->deflink.he_cap.has_he && ebf)
		mt7902_mcu_sta_bfer_he(sta, vif, phy, bf);
	// else if (sta->deflink.vht_cap.vht_supported)
		// mt7902_mcu_sta_bfer_vht(sta, phy, bf, ebf);
	// else if (sta->deflink.ht_cap.ht_supported)
	// 	mt7902_mcu_sta_bfer_ht(sta, phy, bf);
	else
		return; 

	// bf->bf_cap = ebf ? ebf : dev->ibf << 1;
	bf->bw = sta->deflink.bandwidth;
	bf->ibf_dbw = sta->deflink.bandwidth;
	bf->ibf_nrow = tx_ant;

	if (!ebf && sta->deflink.bandwidth <= IEEE80211_STA_RX_BW_40 && !bf->ncol)
		bf->ibf_timeout = 0x48;
	else
		bf->ibf_timeout = 0x18;

	if (ebf && bf->nrow != tx_ant)
		bf->mem_20m = matrix[tx_ant][bf->ncol];
	else
		bf->mem_20m = matrix[bf->nrow][bf->ncol];

	switch (sta->deflink.bandwidth) {
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
mt7902_mcu_sta_ht_tlv(struct sk_buff *skb, struct ieee80211_sta *sta)
{
	struct sta_rec_ht *ht;
	struct tlv *tlv;

	if (!sta->deflink.ht_cap.ht_supported)
		return;

	tlv = mt76_connac_mcu_add_tlv(skb, STA_REC_HT, sizeof(*ht));

	ht = (struct sta_rec_ht *)tlv;
	ht->ht_cap = cpu_to_le16(sta->deflink.ht_cap.cap);
}

static void
mt7902_mcu_sta_vht_tlv(struct sk_buff *skb, struct ieee80211_sta *sta)
{
	struct sta_rec_vht *vht;
	struct tlv *tlv;

	if (!sta->deflink.vht_cap.vht_supported)
		return;

	tlv = mt76_connac_mcu_add_tlv(skb, STA_REC_VHT, sizeof(*vht));

	vht = (struct sta_rec_vht *)tlv;
	vht->vht_cap = cpu_to_le32(sta->deflink.vht_cap.cap);
	vht->vht_rx_mcs_map = sta->deflink.vht_cap.vht_mcs.rx_mcs_map;
	vht->vht_tx_mcs_map = sta->deflink.vht_cap.vht_mcs.tx_mcs_map;
}


static void
mt7902_mcu_sta_amsdu_tlv(struct mt792x_dev *dev, struct sk_buff *skb,
			 struct ieee80211_vif *vif, struct ieee80211_sta *sta)
{
	struct mt792x_sta *msta = (struct mt792x_sta *)sta->drv_priv;
	struct sta_rec_amsdu *amsdu;
	struct tlv *tlv;

	if (vif->type != NL80211_IFTYPE_STATION &&
	    vif->type != NL80211_IFTYPE_AP)
		return;

	if (!sta->deflink.agg.max_amsdu_len)
	    return;

	tlv = mt76_connac_mcu_add_tlv(skb, STA_REC_HW_AMSDU, sizeof(*amsdu));
	amsdu = (struct sta_rec_amsdu *)tlv;
	amsdu->max_amsdu_num = 8;
	amsdu->amsdu_en = true;
	msta->deflink.wcid.amsdu = true;

	switch (sta->deflink.agg.max_amsdu_len) {
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


static void
mt7902_mcu_set_sta_he_mcs(struct ieee80211_sta *sta, __le16 *he_mcs,
			  u16 mcs_map)
{
	struct mt792x_sta *msta = (struct mt792x_sta *)sta->drv_priv;
	enum nl80211_band band = msta->vif->phy->mt76->chandef.chan->band;
	// const u16 *mask = msta->vif->bitrate_mask.control[band].he_mcs;
	int nss, max_nss = sta->deflink.rx_nss > 3 ? 4 : sta->deflink.rx_nss;

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

		// mcs = mcs ? fls(mcs & mask[nss]) - 1 : -1;

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
mt7902_mcu_sta_he_tlv(struct sk_buff *skb, struct ieee80211_sta *sta,
		      struct ieee80211_vif *vif)
{
	struct ieee80211_he_cap_elem *elem = &sta->deflink.he_cap.he_cap_elem;
	struct ieee80211_he_mcs_nss_supp mcs_map;
	struct sta_rec_he_v2 *he;
	struct tlv *tlv;
	int i = 0;

	if (!sta->deflink.he_cap.has_he)
		return;

	tlv = mt76_connac_mcu_add_tlv(skb, STA_REC_HE_V2, sizeof(*he));

	he = (struct sta_rec_he_v2 *)tlv;
	for (i = 0; i < 11; i++){
		if (i < 6)
			he->he_mac_cap[i] = cpu_to_le16(elem->mac_cap_info[i]);
		he->he_phy_cap[i] = cpu_to_le16(elem->phy_cap_info[i]);
	}

	mcs_map = sta->deflink.he_cap.he_mcs_nss_supp;
	switch (sta->deflink.bandwidth) {
	case IEEE80211_STA_RX_BW_160:
		if (elem->phy_cap_info[0] &
		    IEEE80211_HE_PHY_CAP0_CHANNEL_WIDTH_SET_80PLUS80_MHZ_IN_5G)
			mt7902_mcu_set_sta_he_mcs(sta,
						  &he->max_nss_mcs[CMD_HE_MCS_BW8080],
						  le16_to_cpu(mcs_map.rx_mcs_80p80));

		mt7902_mcu_set_sta_he_mcs(sta,
					  &he->max_nss_mcs[CMD_HE_MCS_BW160],
					  le16_to_cpu(mcs_map.rx_mcs_160));
		fallthrough;
	default:
		mt7902_mcu_set_sta_he_mcs(sta,
					  &he->max_nss_mcs[CMD_HE_MCS_BW80],
					  le16_to_cpu(mcs_map.rx_mcs_80));
		break;
	}

	he->pkt_ext = 2;
}


static void
mt7902_mcu_sta_he_6g_tlv(struct sk_buff *skb, struct ieee80211_sta *sta,
		      struct ieee80211_vif *vif)
{
	struct sta_rec_he_6g_capa *he_6g;
	struct tlv *tlv;

	if (!sta->deflink.he_6ghz_capa.capa)
		return;

	tlv = mt76_connac_mcu_add_tlv(skb, STA_REC_HE_6G, sizeof(*he_6g));

	he_6g = (struct sta_rec_he_6g_capa *)tlv;
	he_6g->capa = cpu_to_le16(sta->deflink.he_6ghz_capa.capa);

}


static void
mt7902_mcu_sta_muru_tlv(struct sk_buff *skb, struct ieee80211_sta *sta,
			struct ieee80211_vif *vif)
{
	struct mt792x_vif *mvif = (struct mt792x_vif *)vif->drv_priv;
	struct ieee80211_he_cap_elem *elem = &sta->deflink.he_cap.he_cap_elem;
	struct sta_rec_muru *muru;
	struct tlv *tlv;

	if (vif->type != NL80211_IFTYPE_STATION &&
	    vif->type != NL80211_IFTYPE_AP)
		return;

	tlv = mt76_connac_mcu_add_tlv(skb, STA_REC_MURU, sizeof(*muru));

	muru = (struct sta_rec_muru *)tlv;

	// muru->cfg.mimo_dl_en = mvif->cap.he_mu_ebfer ||
	// 		       mvif->cap.vht_mu_ebfer ||
	// 		       mvif->cap.vht_mu_ebfee;
	muru->cfg.mimo_ul_en = true;
	muru->cfg.ofdma_dl_en = true;

	muru->mimo_dl.vht_mu_bfee =
		!!(sta->deflink.vht_cap.cap & IEEE80211_VHT_CAP_MU_BEAMFORMEE_CAPABLE);

	if (sta->deflink.vht_cap.vht_supported)
		muru->mimo_dl.vht_mu_bfee =
			!!(sta->deflink.vht_cap.cap & IEEE80211_VHT_CAP_MU_BEAMFORMEE_CAPABLE);

	if (!sta->deflink.he_cap.has_he)
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
mt7902_mcu_sta_bfee_tlv(struct mt792x_dev *dev, struct sk_buff *skb,
			struct ieee80211_vif *vif, struct ieee80211_sta *sta)
{
	struct mt792x_vif *mvif = (struct mt792x_vif *)vif->drv_priv;
	struct mt792x_phy *phy = mvif->phy;
	int tx_ant = hweight8(phy->mt76->chainmask) - 1;
	struct sta_rec_bfee *bfee;
	struct tlv *tlv;
	u8 nrow = 0;

	if (!(sta->deflink.vht_cap.vht_supported || sta->deflink.he_cap.has_he))
		return;

	// if (!mt7902_is_ebf_supported(phy, vif, sta, true))
	// 	return;

	tlv = mt76_connac_mcu_add_tlv(skb, STA_REC_BFEE, sizeof(*bfee));
	bfee = (struct sta_rec_bfee *)tlv;

	if (sta->deflink.he_cap.has_he) {
		struct ieee80211_he_cap_elem *pe = &sta->deflink.he_cap.he_cap_elem;

		nrow = HE_PHY(CAP5_BEAMFORMEE_NUM_SND_DIM_UNDER_80MHZ_MASK,
			      pe->phy_cap_info[5]);
	} else if (sta->deflink.vht_cap.vht_supported) {
		struct ieee80211_sta_vht_cap *pc = &sta->deflink.vht_cap;

		nrow = FIELD_GET(IEEE80211_VHT_CAP_SOUNDING_DIMENSIONS_MASK,
				 pc->cap);
	}

	/* reply with identity matrix to avoid 2x2 BF negative gain */
	bfee->fb_identity_matrix = (nrow == 1 && tx_ant == 2);
}


static void
mt7902_mcu_sta_hdr_trans_tlv(struct mt792x_dev *dev, struct sk_buff *skb,
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

static int
mt7902_mcu_add_group(struct mt792x_dev *dev, struct ieee80211_vif *vif,
		     struct ieee80211_sta *sta)
{
#define MT_STA_BSS_GROUP		1
	struct mt792x_vif *mvif = (struct mt792x_vif *)vif->drv_priv;
	struct mt792x_sta *msta;
	struct {
		__le32 action;
		u8 wlan_idx_lo;
		u8 status;
		u8 wlan_idx_hi;
		u8 rsv0[5];
		__le32 val;
		u8 rsv1[8];
	} __packed req = {
		.action = cpu_to_le32(MT_STA_BSS_GROUP),
		.val = cpu_to_le32(mvif->bss_conf.mt76.idx % 16),
	};

	msta = sta ? (struct mt792x_sta *)sta->drv_priv : &mvif->sta;
	req.wlan_idx_lo = to_wcid_lo(msta->deflink.wcid.idx);
	req.wlan_idx_hi = to_wcid_hi(msta->deflink.wcid.idx);

	return mt76_mcu_send_msg(&dev->mt76, MCU_EXT_CMD(SET_DRR_CTRL), &req,
				 sizeof(req), true);

	/*
	struct {
		// fixed field 
		u8 __rsv1[4];
		// TLV 
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

	msta = sta ? (struct mt792x_sta *)sta->drv_priv : &mvif->sta;
	req.wlan_idx = cpu_to_le16(msta->wcid.idx);

	return mt76_mcu_send_msg(&dev->mt76, MCU_EXT_CMD(VOW), &req,
				 sizeof(req), true); */
}








int mt7902_mcu_add_obss_spr(struct mt792x_dev *dev, struct ieee80211_vif *vif,
			    bool enable)
{
#define MT_SPR_ENABLE		1
	struct mt792x_vif *mvif = (struct mt792x_vif *)vif->drv_priv;
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
		.band_idx = mvif->bss_conf.mt76.band_idx,
		.val = cpu_to_le32(enable),
	};

	return mt76_mcu_send_msg(&dev->mt76, MCU_EXT_CMD(SET_SPR), &req,
				 sizeof(req), true);
}

static struct sk_buff *
__mt7902_mcu_alloc_bss_req(struct mt76_dev *dev, struct mt76_vif *mvif, int len)
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


int mt7902_mcu_update_bss_color(struct mt792x_dev *dev, struct ieee80211_vif *vif,
				struct cfg80211_he_bss_color *he_bss_color)
{
	//int len = sizeof(struct bss_req_hdr) + sizeof(struct bss_color_tlv);
	int len = sizeof(struct sta_req_hdr) + sizeof(struct bss_info_color);
	struct mt792x_vif *mvif = (struct mt792x_vif *)vif->drv_priv;
	struct bss_info_color *bss_color;
	struct sk_buff *skb;
	struct tlv *tlv;

	//skb = __mt7902_mcu_alloc_bss_req(&dev->mt76, &mvif->mt76, len);
	skb = __mt76_connac_mcu_alloc_sta_req(&dev->mt76, &mvif->bss_conf.mt76, NULL, len);
	if (IS_ERR(skb))
		return PTR_ERR(skb);

	//tlv = mt76_connac_mcu_add_tlv(skb, UNI_BSS_INFO_BSS_COLOR, sizeof(*bss_color));
	tlv = mt76_connac_mcu_add_tlv(skb, BSS_INFO_BSS_COLOR, sizeof(*bss_color));
	//bss_color = (struct bss_color_tlv *)tlv;
	bss_color = (struct bss_info_color *)tlv;
	//bss_color->enable = he_bss_color->enabled;
	bss_color->disable = !he_bss_color->enabled;
	bss_color->color = he_bss_color->color;

	// return mt76_mcu_skb_send_msg(&dev->mt76, skb,
	// 			     MCU_EXT_CMD(BSS_INFO_UPDATE), true);

	return mt76_mcu_skb_send_msg(&dev->mt76, skb,
				     MCU_WMWA_UNI_CMD(BSS_INFO_UPDATE), true);
}


static int
mt7902_mcu_muar_config(struct mt792x_phy *phy, struct ieee80211_vif *vif,
		       bool bssid, bool enable)
{
	struct mt792x_dev *dev = phy->dev;
	struct mt792x_vif *mvif = (struct mt792x_vif *)vif->drv_priv;
	u32 idx = mvif->bss_conf.mt76.omac_idx - REPEATER_BSSID_START;
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
		.band = mvif->bss_conf.mt76.band_idx,
		.index = idx * 2 + bssid,
	};

	if (bssid)
		addr = vif->bss_conf.bssid;

	if (enable)
		ether_addr_copy(req.addr, addr);

	return mt76_mcu_send_msg(&dev->mt76, MCU_EXT_CMD(MUAR_UPDATE), &req,
				 sizeof(req), true);
}


static struct tlv *
mt7902_mcu_add_nested_subtlv(struct sk_buff *skb, int sub_tag, int sub_len,
			     __le16 *sub_ntlv, __le16 *len)
{
	struct tlv *ptlv, tlv = {
		.tag = cpu_to_le16(sub_tag),
		.len = cpu_to_le16(sub_len),
	};

	ptlv = skb_put_zero(skb, sub_len);
	memcpy(ptlv, &tlv, sizeof(tlv));

	le16_add_cpu(sub_ntlv, 1);
	le16_add_cpu(len, sub_len);

	return ptlv;
}




static void
mt7902_mcu_beacon_cntdwn(struct ieee80211_vif *vif, struct sk_buff *rskb,
			 struct sk_buff *skb, struct bss_info_bcn *bcn,
			 struct ieee80211_mutable_offsets *offs)
{
	struct bss_info_bcn_cntdwn *info;
	struct tlv *tlv;
	int sub_tag;

	if (!offs->cntdwn_counter_offs[0])
		return;

	sub_tag = vif->bss_conf.csa_active ? BSS_INFO_BCN_CSA : BSS_INFO_BCN_BCC;
	tlv = mt7902_mcu_add_nested_subtlv(rskb, sub_tag, sizeof(*info),
					   &bcn->sub_ntlv, &bcn->len);
	info = (struct bss_info_bcn_cntdwn *)tlv;
	info->cnt = skb->data[offs->cntdwn_counter_offs[0]];
}

static void
mt7902_mcu_beacon_mbss(struct sk_buff *rskb, struct sk_buff *skb,
		       struct ieee80211_vif *vif, struct bss_info_bcn *bcn,
		       struct ieee80211_mutable_offsets *offs)
{
	struct bss_info_bcn_mbss *mbss;
	const struct element *elem;
	struct tlv *tlv;

	if (!vif->bss_conf.bssid_indicator)
		return;

	tlv = mt7902_mcu_add_nested_subtlv(rskb, BSS_INFO_BCN_MBSSID,
					   sizeof(*mbss), &bcn->sub_ntlv,
					   &bcn->len);

	mbss = (struct bss_info_bcn_mbss *)tlv;
	mbss->offset[0] = cpu_to_le16(offs->tim_offset);
	mbss->bitmap = cpu_to_le32(1);

	for_each_element_id(elem, WLAN_EID_MULTIPLE_BSSID,
			    &skb->data[offs->mbssid_off],
			    skb->len - offs->mbssid_off) {
		const struct element *sub_elem;

		if (elem->datalen < 2)
			continue;

		for_each_element(sub_elem, elem->data + 1, elem->datalen - 1) {
			const struct ieee80211_bssid_index *idx;
			const u8 *idx_ie;

			if (sub_elem->id || sub_elem->datalen < 4)
				continue; /* not a valid BSS profile */

			/* Find WLAN_EID_MULTI_BSSID_IDX
			 * in the merged nontransmitted profile
			 */
			idx_ie = cfg80211_find_ie(WLAN_EID_MULTI_BSSID_IDX,
						  sub_elem->data,
						  sub_elem->datalen);
			if (!idx_ie || idx_ie[1] < sizeof(*idx))
				continue;

			idx = (void *)(idx_ie + 2);
			if (!idx->bssid_index || idx->bssid_index > 31)
				continue;

			mbss->offset[idx->bssid_index] =
				cpu_to_le16(idx_ie - skb->data);
			mbss->bitmap |= cpu_to_le32(BIT(idx->bssid_index));
		}
	}
}

static void
mt7902_mcu_beacon_cont(struct mt792x_dev *dev, struct ieee80211_vif *vif,
		       struct sk_buff *rskb, struct sk_buff *skb,
		       struct bss_info_bcn *bcn,
		       struct ieee80211_mutable_offsets *offs)
{
	struct mt76_wcid *wcid = &dev->mt76.global_wcid;
	struct bss_info_bcn_cont *cont;
	struct tlv *tlv;
	u8 *buf;
	int len = sizeof(*cont) + MT_TXD_SIZE + skb->len;

	len = (len & 0x3) ? ((len | 0x3) + 1) : len;
	tlv = mt7902_mcu_add_nested_subtlv(rskb, BSS_INFO_BCN_CONTENT,
					   len, &bcn->sub_ntlv, &bcn->len);

	cont = (struct bss_info_bcn_cont *)tlv;
	cont->pkt_len = cpu_to_le16(MT_TXD_SIZE + skb->len);
	cont->tim_ofs = cpu_to_le16(offs->tim_offset);

	if (offs->cntdwn_counter_offs[0]) {
		u16 offset = offs->cntdwn_counter_offs[0];

		if (vif->bss_conf.csa_active)
			cont->csa_ofs = cpu_to_le16(offset - 4);
		if (vif->bss_conf.color_change_active)
			cont->bcc_ofs = cpu_to_le16(offset - 3);
	}

	buf = (u8 *)tlv + sizeof(*cont);
	mt7902_mac_write_txwi(&dev->mt76, (__le32 *)buf, skb, wcid, 0, NULL,
			      0, BSS_CHANGED_BEACON);
	memcpy(buf + MT_TXD_SIZE, skb->data, skb->len);
}




int mt7902_mcu_add_beacon(struct ieee80211_hw *hw,
			  struct ieee80211_vif *vif, int en)
{
	struct mt792x_dev *dev = mt7902_hw_dev(hw);
	struct mt792x_phy *phy = mt7902_hw_phy(hw);
	struct mt792x_vif *mvif = (struct mt792x_vif *)vif->drv_priv;
	struct ieee80211_mutable_offsets offs;
	struct ieee80211_tx_info *info;
	struct sk_buff *skb, *rskb;
	struct tlv *tlv;
	struct bss_info_bcn *bcn;
	//u8 phy_idx = mt7902_get_phy_id(phy);
	int len = MT7902_MAX_BSS_OFFLOAD_SIZE;
	bool ext_phy = phy != &dev->phy;

	if(vif->bss_conf.nontransmitted)
		return 0;

	rskb = __mt76_connac_mcu_alloc_sta_req(&dev->mt76, &mvif->bss_conf.mt76, NULL, len);
	//rskb = __mt7902_mcu_alloc_bss_req(&dev->mt76, &mvif->mt76, mt7902_BEACON_UPDATE_SIZE);
	if (IS_ERR(rskb))
		return PTR_ERR(rskb);

	//tlv = mt7902_mcu_add_uni_tlv(rskb,
	//			    UNI_BSS_INFO_BCN_CONTENT, sizeof(*bcn));
	tlv = mt76_connac_mcu_add_tlv(rskb, BSS_INFO_OFFLOAD, sizeof(*bcn));
	bcn = (struct bss_info_bcn *)tlv;
	//bcn = (struct bss_bcn_content_tlv *)tlv;
	bcn->enable = en;

	if (!en)
		goto out;

	skb = ieee80211_beacon_get_template(hw, vif, &offs, 0);
	if (!skb) {
		dev_kfree_skb(rskb);
		return -EINVAL;
	}

	//if (skb->len > MAX_BEACON_SIZE - MT_TXD_SIZE) {
	if(skb->len > MT7902_MAX_BEACON_SIZE) {
		dev_err(dev->mt76.dev, "Bcn size limit exceed\n");
		dev_kfree_skb(skb);
		return -EINVAL;
	}

	info = IEEE80211_SKB_CB(skb);
	info->hw_queue |= FIELD_PREP(MT_TX_HW_QUEUE_PHY, ext_phy);

	//mt7902_mcu_beacon_check_caps(phy, vif, skb);

	//mt7902_mcu_beacon_cont(dev, vif, rskb, skb, bcn, &offs);
	/* TODO: subtag - 11v MBSSID */
	//mt7902_mcu_beacon_cntdwn(vif, rskb, skb, &offs);

	mt7902_mcu_beacon_cntdwn(vif, rskb, skb, bcn, &offs);
	mt7902_mcu_beacon_mbss(rskb, skb, vif, bcn, &offs);
	mt7902_mcu_beacon_cont(dev, vif, rskb, skb, bcn, &offs);
	dev_kfree_skb(skb);
out:
	return mt76_mcu_skb_send_msg(&phy->dev->mt76, rskb,
				     MCU_EXT_CMD(BSS_INFO_UPDATE), true);
}



// static inline bool
// mt7902_is_ebf_supported(struct mt792x_phy *phy, struct ieee80211_vif *vif,
// 			struct ieee80211_sta *sta, bool bfee)
// {
// 	struct mt792x_vif *mvif = (struct mt792x_vif *)vif->drv_priv;
// 	int tx_ant = hweight8(phy->mt76->chainmask) - 1;

// 	if (vif->type != NL80211_IFTYPE_STATION &&
// 	    vif->type != NL80211_IFTYPE_AP)
// 		return false;

// 	if (!bfee && tx_ant < 2)
// 		return false;

// 	if (sta->he_cap.has_he) {
// 		struct ieee80211_he_cap_elem *pe = &sta->he_cap.he_cap_elem;

// 		if (bfee)
// 			return mvif->cap.he_su_ebfee &&
// 			       HE_PHY(CAP3_SU_BEAMFORMER, pe->phy_cap_info[3]);
// 		else
// 			return mvif->cap.he_su_ebfer &&
// 			       HE_PHY(CAP4_SU_BEAMFORMEE, pe->phy_cap_info[4]);
// 	}

// 	if (sta->vht_cap.vht_supported) {
// 		u32 cap = sta->vht_cap.cap;

// 		if (bfee)
// 			return mvif->cap.vht_su_ebfee &&
// 			       (cap & IEEE80211_VHT_CAP_SU_BEAMFORMER_CAPABLE);
// 		else
// 			return mvif->cap.vht_su_ebfer &&
// 			       (cap & IEEE80211_VHT_CAP_SU_BEAMFORMEE_CAPABLE);
// 	}

// 	return false;
// }


// static void
// mt7902_mcu_sta_bfer_ht(struct ieee80211_sta *sta, struct mt792x_phy *phy,
// 		       struct sta_rec_bf *bf)
// {
// 	struct ieee80211_mcs_info *mcs = &sta->ht_cap.mcs;
// 	u8 n = 0;

// 	bf->tx_mode = MT_PHY_TYPE_HT;

// 	if ((mcs->tx_params & IEEE80211_HT_MCS_TX_RX_DIFF) &&
// 	    (mcs->tx_params & IEEE80211_HT_MCS_TX_DEFINED))
// 		n = FIELD_GET(IEEE80211_HT_MCS_TX_MAX_STREAMS_MASK,
// 			      mcs->tx_params);
// 	else if (mcs->rx_mask[3])
// 		n = 3;
// 	else if (mcs->rx_mask[2])
// 		n = 2;
// 	else if (mcs->rx_mask[1])
// 		n = 1;

// 	bf->nrow = hweight8(phy->mt76->chainmask) - 1;
// 	bf->ncol = min_t(u8, bf->nrow, n);
// 	bf->ibf_ncol = n;
// }






// static void
// mt7902_mcu_sta_bfer_vht(struct ieee80211_sta *sta, struct mt792x_phy *phy,
// 			struct sta_rec_bf *bf, bool explicit)
// {
// 	struct ieee80211_sta_vht_cap *pc = &sta->vht_cap;
// 	struct ieee80211_sta_vht_cap *vc = &phy->mt76->sband_5g.sband.vht_cap;
// 	u16 mcs_map = le16_to_cpu(pc->vht_mcs.rx_mcs_map);
// 	u8 nss_mcs = mt7902_mcu_get_sta_nss(mcs_map);
// 	u8 tx_ant = hweight8(phy->mt76->chainmask) - 1;

// 	bf->tx_mode = MT_PHY_TYPE_VHT;

// 	if (explicit) {
// 		u8 sts, snd_dim;

// 		mt7902_mcu_sta_sounding_rate(bf);

// 		sts = FIELD_GET(IEEE80211_VHT_CAP_BEAMFORMEE_STS_MASK,
// 				pc->cap);
// 		snd_dim = FIELD_GET(IEEE80211_VHT_CAP_SOUNDING_DIMENSIONS_MASK,
// 				    vc->cap);
// 		bf->nrow = min_t(u8, min_t(u8, snd_dim, sts), tx_ant);
// 		bf->ncol = min_t(u8, nss_mcs, bf->nrow);
// 		bf->ibf_ncol = bf->ncol;

// 		if (sta->bandwidth == IEEE80211_STA_RX_BW_160)
// 			bf->nrow = 1;
// 	} else {
// 		bf->nrow = tx_ant;
// 		bf->ncol = min_t(u8, nss_mcs, bf->nrow);
// 		bf->ibf_ncol = nss_mcs;

// 		if (sta->bandwidth == IEEE80211_STA_RX_BW_160)
// 			bf->ibf_nrow = 1;
// 	}
// }

int mt7902_mcu_set_radio_en(struct mt792x_phy *phy, struct mt792x_vif *mvif, bool enable)
{
	struct {
		u8 band_idx;
		u8 _rsv[3];

		__le16 tag;
		__le16 len;
		u8 enable;
		u8 _rsv2[3];
	} __packed req = {
		.band_idx = mvif->bss_conf.mt76.band_idx,
		.tag = cpu_to_le16(UNI_BAND_CONFIG_RADIO_ENABLE),
		.len = cpu_to_le16(sizeof(req) - 4),
		.enable = enable,
	};

	return mt76_mcu_send_msg(&phy->dev->mt76, MCU_WM_UNI_CMD(BAND_CONFIG),
				 &req, sizeof(req), true);
}
