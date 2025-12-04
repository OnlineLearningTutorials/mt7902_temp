// SPDX-License-Identifier: ISC
/* Copyright (C) 2020 MediaTek Inc. */

#include <linux/etherdevice.h>
#include <linux/platform_device.h>
#include <linux/pci.h>
#include <linux/module.h>
#include <net/ipv6.h>
#include "mt7902.h"
#include "mcu.h"

static int
mt7902_init_he_caps(struct mt792x_phy *phy, enum nl80211_band band,
		    struct ieee80211_sband_iftype_data *data)
{
	printk(KERN_DEBUG "main.c - mt7902_init_he_caps(phy, band: %d, data)", band);
	int i, idx = 0;
	int nss = hweight8(phy->mt76->chainmask);
	u16 mcs_map = 0;

	for (i = 0; i < 8; i++) {
		if (i < nss)
			mcs_map |= (IEEE80211_HE_MCS_SUPPORT_0_11 << (i * 2));
		else
			mcs_map |= (IEEE80211_HE_MCS_NOT_SUPPORTED << (i * 2));
	}

	for (i = 0; i < NUM_NL80211_IFTYPES; i++) {
		struct ieee80211_sta_he_cap *he_cap = &data[idx].he_cap;
		struct ieee80211_he_cap_elem *he_cap_elem =
				&he_cap->he_cap_elem;
		struct ieee80211_he_mcs_nss_supp *he_mcs =
				&he_cap->he_mcs_nss_supp;

		switch (i) {
		case NL80211_IFTYPE_STATION:
		case NL80211_IFTYPE_AP:
			break;
		default:
			continue;
		}

		data[idx].types_mask = BIT(i);
		he_cap->has_he = true;

		he_cap_elem->mac_cap_info[0] =
			IEEE80211_HE_MAC_CAP0_HTC_HE;
		he_cap_elem->mac_cap_info[3] =
			IEEE80211_HE_MAC_CAP3_OMI_CONTROL |
			IEEE80211_HE_MAC_CAP3_MAX_AMPDU_LEN_EXP_EXT_3;
		he_cap_elem->mac_cap_info[4] =
			IEEE80211_HE_MAC_CAP4_AMSDU_IN_AMPDU;

		if (band == NL80211_BAND_2GHZ)
			he_cap_elem->phy_cap_info[0] =
				IEEE80211_HE_PHY_CAP0_CHANNEL_WIDTH_SET_40MHZ_IN_2G;
		else
			he_cap_elem->phy_cap_info[0] =
				IEEE80211_HE_PHY_CAP0_CHANNEL_WIDTH_SET_40MHZ_80MHZ_IN_5G;

		he_cap_elem->phy_cap_info[1] =
			IEEE80211_HE_PHY_CAP1_LDPC_CODING_IN_PAYLOAD;
		he_cap_elem->phy_cap_info[2] =
			IEEE80211_HE_PHY_CAP2_NDP_4x_LTF_AND_3_2US |
			IEEE80211_HE_PHY_CAP2_STBC_TX_UNDER_80MHZ |
			IEEE80211_HE_PHY_CAP2_STBC_RX_UNDER_80MHZ |
			IEEE80211_HE_PHY_CAP2_UL_MU_FULL_MU_MIMO |
			IEEE80211_HE_PHY_CAP2_UL_MU_PARTIAL_MU_MIMO;

		switch (i) {
		case NL80211_IFTYPE_AP:
			he_cap_elem->mac_cap_info[2] |=
				IEEE80211_HE_MAC_CAP2_BSR;
			he_cap_elem->mac_cap_info[4] |=
				IEEE80211_HE_MAC_CAP4_BQR;
			he_cap_elem->mac_cap_info[5] |=
				IEEE80211_HE_MAC_CAP5_OM_CTRL_UL_MU_DATA_DIS_RX;
			he_cap_elem->phy_cap_info[3] |=
				IEEE80211_HE_PHY_CAP3_DCM_MAX_CONST_TX_QPSK |
				IEEE80211_HE_PHY_CAP3_DCM_MAX_CONST_RX_QPSK;
			he_cap_elem->phy_cap_info[6] |=
				IEEE80211_HE_PHY_CAP6_PARTIAL_BW_EXT_RANGE |
				IEEE80211_HE_PHY_CAP6_PPE_THRESHOLD_PRESENT;
			he_cap_elem->phy_cap_info[9] |=
				IEEE80211_HE_PHY_CAP9_TX_1024_QAM_LESS_THAN_242_TONE_RU |
				IEEE80211_HE_PHY_CAP9_RX_1024_QAM_LESS_THAN_242_TONE_RU;

			if (is_mt7922(phy->mt76->dev)) {
				he_cap_elem->phy_cap_info[0] |=
					IEEE80211_HE_PHY_CAP0_CHANNEL_WIDTH_SET_160MHZ_IN_5G;
			}
			break;
		case NL80211_IFTYPE_STATION:
			he_cap_elem->mac_cap_info[1] |=
				IEEE80211_HE_MAC_CAP1_TF_MAC_PAD_DUR_16US;

			if (band == NL80211_BAND_2GHZ)
				he_cap_elem->phy_cap_info[0] |=
					IEEE80211_HE_PHY_CAP0_CHANNEL_WIDTH_SET_RU_MAPPING_IN_2G;
			else
				he_cap_elem->phy_cap_info[0] |=
					IEEE80211_HE_PHY_CAP0_CHANNEL_WIDTH_SET_RU_MAPPING_IN_5G;

			he_cap_elem->phy_cap_info[1] |=
				IEEE80211_HE_PHY_CAP1_DEVICE_CLASS_A |
				IEEE80211_HE_PHY_CAP1_HE_LTF_AND_GI_FOR_HE_PPDUS_0_8US;
			he_cap_elem->phy_cap_info[3] |=
				IEEE80211_HE_PHY_CAP3_DCM_MAX_CONST_TX_QPSK |
				IEEE80211_HE_PHY_CAP3_DCM_MAX_CONST_RX_QPSK;
			he_cap_elem->phy_cap_info[4] |=
				IEEE80211_HE_PHY_CAP4_SU_BEAMFORMEE |
				IEEE80211_HE_PHY_CAP4_BEAMFORMEE_MAX_STS_UNDER_80MHZ_4;
			he_cap_elem->phy_cap_info[5] |=
				IEEE80211_HE_PHY_CAP5_NG16_SU_FEEDBACK |
				IEEE80211_HE_PHY_CAP5_NG16_MU_FEEDBACK;
			he_cap_elem->phy_cap_info[6] |=
				IEEE80211_HE_PHY_CAP6_CODEBOOK_SIZE_42_SU |
				IEEE80211_HE_PHY_CAP6_CODEBOOK_SIZE_75_MU |
				IEEE80211_HE_PHY_CAP6_TRIG_CQI_FB |
				IEEE80211_HE_PHY_CAP6_PARTIAL_BW_EXT_RANGE |
				IEEE80211_HE_PHY_CAP6_PPE_THRESHOLD_PRESENT;
			he_cap_elem->phy_cap_info[7] |=
				IEEE80211_HE_PHY_CAP7_POWER_BOOST_FACTOR_SUPP |
				IEEE80211_HE_PHY_CAP7_HE_SU_MU_PPDU_4XLTF_AND_08_US_GI;
			he_cap_elem->phy_cap_info[8] |=
				IEEE80211_HE_PHY_CAP8_20MHZ_IN_40MHZ_HE_PPDU_IN_2G |
				IEEE80211_HE_PHY_CAP8_DCM_MAX_RU_484;
			he_cap_elem->phy_cap_info[9] |=
				IEEE80211_HE_PHY_CAP9_LONGER_THAN_16_SIGB_OFDM_SYM |
				IEEE80211_HE_PHY_CAP9_NON_TRIGGERED_CQI_FEEDBACK |
				IEEE80211_HE_PHY_CAP9_TX_1024_QAM_LESS_THAN_242_TONE_RU |
				IEEE80211_HE_PHY_CAP9_RX_1024_QAM_LESS_THAN_242_TONE_RU |
				IEEE80211_HE_PHY_CAP9_RX_FULL_BW_SU_USING_MU_WITH_COMP_SIGB |
				IEEE80211_HE_PHY_CAP9_RX_FULL_BW_SU_USING_MU_WITH_NON_COMP_SIGB;

			if (is_mt7922(phy->mt76->dev)) {
				he_cap_elem->phy_cap_info[0] |=
					IEEE80211_HE_PHY_CAP0_CHANNEL_WIDTH_SET_160MHZ_IN_5G;
				he_cap_elem->phy_cap_info[8] |=
					IEEE80211_HE_PHY_CAP8_20MHZ_IN_160MHZ_HE_PPDU |
					IEEE80211_HE_PHY_CAP8_80MHZ_IN_160MHZ_HE_PPDU;
			}
			break;
		}

		he_mcs->rx_mcs_80 = cpu_to_le16(mcs_map);
		he_mcs->tx_mcs_80 = cpu_to_le16(mcs_map);
		if (is_mt7922(phy->mt76->dev)) {
			he_mcs->rx_mcs_160 = cpu_to_le16(mcs_map);
			he_mcs->tx_mcs_160 = cpu_to_le16(mcs_map);
		}

		memset(he_cap->ppe_thres, 0, sizeof(he_cap->ppe_thres));
		if (he_cap_elem->phy_cap_info[6] &
		    IEEE80211_HE_PHY_CAP6_PPE_THRESHOLD_PRESENT) {
			mt76_connac_gen_ppe_thresh(he_cap->ppe_thres, nss, band);
		} else {
			he_cap_elem->phy_cap_info[9] |=
				u8_encode_bits(IEEE80211_HE_PHY_CAP9_NOMINAL_PKT_PADDING_16US,
					       IEEE80211_HE_PHY_CAP9_NOMINAL_PKT_PADDING_MASK);
		}

		if (band == NL80211_BAND_6GHZ) {
			struct ieee80211_supported_band *sband =
				&phy->mt76->sband_5g.sband;
			struct ieee80211_sta_vht_cap *vht_cap = &sband->vht_cap;
			struct ieee80211_sta_ht_cap *ht_cap = &sband->ht_cap;
			u32 exp;
			u16 cap;

			cap = u16_encode_bits(ht_cap->ampdu_density,
					IEEE80211_HE_6GHZ_CAP_MIN_MPDU_START);
			exp = u32_get_bits(vht_cap->cap,
				IEEE80211_VHT_CAP_MAX_A_MPDU_LENGTH_EXPONENT_MASK);
			cap |= u16_encode_bits(exp,
					IEEE80211_HE_6GHZ_CAP_MAX_AMPDU_LEN_EXP);
			exp = u32_get_bits(vht_cap->cap,
					   IEEE80211_VHT_CAP_MAX_MPDU_MASK);
			cap |= u16_encode_bits(exp,
					IEEE80211_HE_6GHZ_CAP_MAX_MPDU_LEN);
			if (vht_cap->cap & IEEE80211_VHT_CAP_TX_ANTENNA_PATTERN)
				cap |= IEEE80211_HE_6GHZ_CAP_TX_ANTPAT_CONS;
			if (vht_cap->cap & IEEE80211_VHT_CAP_RX_ANTENNA_PATTERN)
				cap |= IEEE80211_HE_6GHZ_CAP_RX_ANTPAT_CONS;

			data[idx].he_6ghz_capa.capa = cpu_to_le16(cap);
		}
		idx++;
	}

	return idx;
}

void mt7902_set_stream_he_caps(struct mt792x_phy *phy)
{
	printk(KERN_DEBUG "main.c - mt7902_set_stream_he_caps(phy)");
	struct ieee80211_sband_iftype_data *data;
	struct ieee80211_supported_band *band;
	int n;

	if (phy->mt76->cap.has_2ghz) {
		data = phy->iftype[NL80211_BAND_2GHZ];
		n = mt7902_init_he_caps(phy, NL80211_BAND_2GHZ, data);

		band = &phy->mt76->sband_2g.sband;
		_ieee80211_set_sband_iftype_data(band, data, n);
	}

	if (phy->mt76->cap.has_5ghz) {
		data = phy->iftype[NL80211_BAND_5GHZ];
		n = mt7902_init_he_caps(phy, NL80211_BAND_5GHZ, data);

		band = &phy->mt76->sband_5g.sband;
		_ieee80211_set_sband_iftype_data(band, data, n);
	}
	if (phy->mt76->cap.has_6ghz) {
		data = phy->iftype[NL80211_BAND_6GHZ];
		n = mt7902_init_he_caps(phy, NL80211_BAND_6GHZ, data);

		band = &phy->mt76->sband_6g.sband;
		_ieee80211_set_sband_iftype_data(band, data, n);
	}
}

int __mt7902_start(struct mt792x_phy *phy)
{
	printk(KERN_DEBUG "main.c - __mt792x_start(phy)");
	struct mt76_phy *mphy = phy->mt76;
	int err;

	err = mt76_connac_mcu_set_mac_enable(mphy->dev, 0, true, false);
	if (err)
		return err;

	err = mt76_connac_mcu_set_channel_domain(mphy);
	if (err)
		return err;

	err = mt7902_mcu_set_chan_info(phy, MCU_EXT_CMD(SET_RX_PATH));
	if (err)
		return err;

	err = mt7902_set_tx_sar_pwr(mphy->hw, NULL);
	if (err)
		return err;

	mt792x_mac_reset_counters(phy);
	set_bit(MT76_STATE_RUNNING, &mphy->state);

	ieee80211_queue_delayed_work(mphy->hw, &mphy->mac_work,
				     MT792x_WATCHDOG_TIME);
	if (mt76_is_mmio(mphy->dev)) {
		err = mt7902_mcu_radio_led_ctrl(phy->dev, EXT_CMD_RADIO_LED_CTRL_ENABLE);
		if (err)
			return err;

		err = mt7902_mcu_radio_led_ctrl(phy->dev, EXT_CMD_RADIO_ON_LED);
		if (err)
			return err;
	}

	if (phy->chip_cap & MT792x_CHIP_CAP_WF_RF_PIN_CTRL_EVT_EN) {
		mt7902_mcu_wf_rf_pin_ctrl(phy, WF_RF_PIN_INIT);
		wiphy_rfkill_start_polling(mphy->hw->wiphy);
	}

	return 0;
}
EXPORT_SYMBOL_GPL(__mt7902_start);

static int mt7902_start(struct ieee80211_hw *hw)
{
	printk(KERN_DEBUG "main.c - mt792x_start(hw)");
	struct mt792x_phy *phy = mt792x_hw_phy(hw);
	int err;

	mt792x_mutex_acquire(phy->dev);
	err = __mt7902_start(phy);
	mt792x_mutex_release(phy->dev);

	return err;
}

static void mt7902_stop(struct ieee80211_hw *hw, bool suspend)
{
	printk(KERN_DEBUG "main.c - mt7902_stop");
	struct mt792x_dev *dev = mt792x_hw_dev(hw);
	int err = 0;

	if (mt76_is_mmio(&dev->mt76)) {
		mt792x_mutex_acquire(dev);
		err = mt7902_mcu_radio_led_ctrl(dev, EXT_CMD_RADIO_OFF_LED);
		mt792x_mutex_release(dev);
		if (err)
			return;
	}

	mt792x_stop(hw, false);
}

static inline int get_free_idx(u32 mask, u8 start, u8 end)
{
	return ffs(~mask & GENMASK(end, start));
}

static int get_omac_idx(enum nl80211_iftype type, u64 mask)
{
	int i;

	switch (type) {
	case NL80211_IFTYPE_MESH_POINT:
	case NL80211_IFTYPE_ADHOC:
	case NL80211_IFTYPE_STATION:
		/* prefer hw bssid slot 1-3 */
		i = get_free_idx(mask, HW_BSSID_1, HW_BSSID_3);
		if (i)
			return i - 1;

		if (type != NL80211_IFTYPE_STATION)
			break;

		i = get_free_idx(mask, EXT_BSSID_1, EXT_BSSID_MAX);
		if (i)
			return i - 1;

		if (~mask & BIT(HW_BSSID_0))
			return HW_BSSID_0;

		break;
	case NL80211_IFTYPE_MONITOR:
	case NL80211_IFTYPE_AP:
		/* ap uses hw bssid 0 and ext bssid */
		if (~mask & BIT(HW_BSSID_0))
			return HW_BSSID_0;

		i = get_free_idx(mask, EXT_BSSID_1, EXT_BSSID_MAX);
		if (i)
			return i - 1;

		break;
	default:
		WARN_ON(1);
		break;
	}

	return -1;
}

// static void mt7902_init_bitrate_mask(struct ieee80211_vif *vif)
// {
// 	struct mt792x_vif *mvif = (struct mt792x_vif *)vif->drv_priv;
// 	int i;

// 	for (i = 0; i < ARRAY_SIZE(mvif->deflink.bitrate_mask.control); i++) {
// 		mvif->bitrate_mask.control[i].gi = NL80211_TXRATE_DEFAULT_GI;
// 		mvif->bitrate_mask.control[i].he_gi = 0xff;
// 		mvif->bitrate_mask.control[i].he_ltf = 0xff;
// 		mvif->bitrate_mask.control[i].legacy = GENMASK(31, 0);
// 		memset(mvif->bitrate_mask.control[i].ht_mcs, 0xff,
// 		       sizeof(mvif->bitrate_mask.control[i].ht_mcs));
// 		memset(mvif->bitrate_mask.control[i].vht_mcs, 0xff,
// 		       sizeof(mvif->bitrate_mask.control[i].vht_mcs));
// 		memset(mvif->bitrate_mask.control[i].he_mcs, 0xff,
// 		       sizeof(mvif->bitrate_mask.control[i].he_mcs));
// 	}
// }


static int
mt7902_add_interface(struct ieee80211_hw *hw, struct ieee80211_vif *vif)
{
	printk(KERN_DEBUG "main.c - mt7902_add_interface(hw, vif)");
	struct mt792x_vif *mvif = (struct mt792x_vif *)vif->drv_priv;
	struct mt792x_dev *dev = mt792x_hw_dev(hw);
	struct mt792x_phy *phy = mt792x_hw_phy(hw);
	struct mt76_txq *mtxq;
	bool ext_phy = phy != &dev->phy;
	int idx, ret = 0;

	mt792x_mutex_acquire(dev);

	// mt76_testmode_reset(phy->mt76, true);

	// if(vif->type == NL80211_IFTYPE_MONITOR &&
	// 	is_zero_ether_addr(vif->addr))
	// 	phy->mt76->roc_vif = vif;

	mvif->bss_conf.mt76.idx = __ffs64(~dev->mt76.vif_mask);
	if (mvif->bss_conf.mt76.idx >= MT792x_MAX_INTERFACES) {
		ret = -ENOSPC;
		goto out;
	}

	// idx = get_omac_idx(vif->type, phy->omac_mask);
	// if(idx < 0) {
	// 	ret = -ENOSPC;
	// 	goto out;
	// }

	mvif->bss_conf.mt76.omac_idx = mvif->bss_conf.mt76.idx;
	mvif->phy = phy;
	mvif->bss_conf.vif = mvif;
	mvif->bss_conf.mt76.band_idx = 0; // phy->mt76->band_idx;
	// mvif->bss_conf.mt76.wcid = &mvif->sta.deflink.wcid;
	mvif->bss_conf.mt76.wmm_idx = mvif->bss_conf.mt76.idx % MT76_CONNAC_MAX_WMM_SETS;

	// mvif->bss_conf.mt76.wmm_idx = vif->type != NL80211_IFTYPE_AP;
	// if(ext_phy)
	// 	mvif->bss_conf.mt76.wmm_idx += 2;

	 // ret = mt76_connac_mcu_uni_add_dev(&dev->mphy, &vif->bss_conf,
	 // 				  &mvif->bss_conf.mt76,
	 // 				  &mvif->sta.deflink.wcid, true);

    	//ret = mt7902_mcu_add_dev_info(phy, vif, true);
	ret = mt7902_mcu_add_dev_info(&dev->mphy, &vif->bss_conf, &mvif->bss_conf.mt76,
					  true);
    printk(KERN_INFO "add_interface: add dev info, ret=%d\n", ret);
	if (ret)
		goto out;

	// ret = mt7902_mcu_set_radio_en(phy, mvif, true);
	// if (ret)
	// 	goto out;

	dev->mt76.vif_mask |= BIT_ULL(mvif->bss_conf.mt76.idx);
	phy->omac_mask |= BIT_ULL(mvif->bss_conf.mt76.omac_idx);

	idx = MT792x_WTBL_RESERVED - mvif->bss_conf.mt76.idx;

	// idx = mt76_wcid_alloc(dev->mt76.wcid_mask, mt7902_wtbl_size(dev));
	// if(idx < 0) {
	// 	ret = -ENOSPC;
	// 	goto out;
	// }

	INIT_LIST_HEAD(&mvif->sta.deflink.wcid.poll_list);
	mvif->sta.deflink.wcid.idx = idx;
	mvif->sta.deflink.wcid.tx_info |= MT_WCID_TX_INFO_SET;
	mt76_wcid_init(&mvif->sta.deflink.wcid, mvif->bss_conf.mt76.band_idx);

	mt7902_mac_wtbl_update(dev, idx,
			       MT_WTBL_UPDATE_ADM_COUNT_CLEAR);

	ewma_rssi_init(&mvif->bss_conf.rssi);

	rcu_assign_pointer(dev->mt76.wcid[idx], &mvif->sta.deflink.wcid);
	if (vif->txq) {
		mtxq = (struct mt76_txq *)vif->txq->drv_priv;
		mtxq->wcid = idx;
	}

	// if(vif->type != NL80211_IFTYPE_AP && 
	// 	(!mvif->bss_conf.mt76.omac_idx || mvif->bss_conf.mt76.omac_idx > 3))
	// 	vif->offload_flags = 0;
	// vif->offload_flags |= IEEE80211_OFFLOAD_ENCAP_4ADDR;

	// mt7902_init_bitrate_mask(vif);
	// memset(&mvif->cap, -1, sizeof(mvif->cap));

	// mt7902_mcu_add_bss_info(phy, vif, true);
	// mt7902_mcu_add_sta(dev, vif, NULL, CONN_STATE_PORT_SECURE, true);


	vif->driver_flags |= IEEE80211_VIF_BEACON_FILTER;
	if (phy->chip_cap & MT792x_CHIP_CAP_RSSI_NOTIFY_EVT_EN)
		vif->driver_flags |= IEEE80211_VIF_SUPPORTS_CQM_RSSI;

	INIT_WORK(&mvif->csa_work, mt7902_csa_work);
	timer_setup(&mvif->csa_timer, mt792x_csa_timer, 0);  
out:
	mt792x_mutex_release(dev);

	return ret;
}

static void mt7902_roc_iter(void *priv, u8 *mac,
			    struct ieee80211_vif *vif)
{
	printk(KERN_DEBUG "main.c - mt7902_roc_iter");
	struct mt792x_vif *mvif = (struct mt792x_vif *)vif->drv_priv;
	struct mt792x_phy *phy = priv;

	mt7902_mcu_abort_roc(phy, mvif, phy->roc_token_id);
}

void mt7902_roc_abort_sync(struct mt792x_dev *dev)
{
	printk(KERN_DEBUG "main.c - mt7902_roc_abort_sync");
	struct mt792x_phy *phy = &dev->phy;

	timer_delete_sync(&phy->roc_timer);
	cancel_work_sync(&phy->roc_work);
	if (test_and_clear_bit(MT76_STATE_ROC, &phy->mt76->state))
		ieee80211_iterate_interfaces(mt76_hw(dev),
					     IEEE80211_IFACE_ITER_RESUME_ALL,
					     mt7902_roc_iter, (void *)phy);
}
EXPORT_SYMBOL_GPL(mt7902_roc_abort_sync);

void mt7902_roc_work(struct work_struct *work)
{
	printk(KERN_DEBUG "main.c - mt7902_roc_work");
	struct mt792x_phy *phy;

	phy = (struct mt792x_phy *)container_of(work, struct mt792x_phy,
						roc_work);

	if (!test_and_clear_bit(MT76_STATE_ROC, &phy->mt76->state))
		return;

	mt792x_mutex_acquire(phy->dev);
	ieee80211_iterate_active_interfaces(phy->mt76->hw,
					    IEEE80211_IFACE_ITER_RESUME_ALL,
					    mt7902_roc_iter, phy);
	mt792x_mutex_release(phy->dev);
	ieee80211_remain_on_channel_expired(phy->mt76->hw);
}

static int mt7902_abort_roc(struct mt792x_phy *phy, struct mt792x_vif *vif)
{
	printk(KERN_DEBUG "main.c - mt7902_abort_roc");
	int err = 0;

	timer_delete_sync(&phy->roc_timer);
	cancel_work_sync(&phy->roc_work);

	mt792x_mutex_acquire(phy->dev);
	if (test_and_clear_bit(MT76_STATE_ROC, &phy->mt76->state))
		err = mt7902_mcu_abort_roc(phy, vif, phy->roc_token_id);
	mt792x_mutex_release(phy->dev);

	return err;
}

static int mt7902_set_roc(struct mt792x_phy *phy,
			  struct mt792x_vif *vif,
			  struct ieee80211_channel *chan,
			  int duration,
			  enum mt7902_roc_req type)
{
	printk(KERN_DEBUG "main.c - mt7902_set_roc");
	int err;

	if (test_and_set_bit(MT76_STATE_ROC, &phy->mt76->state))
		return -EBUSY;

	phy->roc_grant = false;

	err = mt7902_mcu_set_roc(phy, vif, chan, duration, type,
				 ++phy->roc_token_id);
	if (err < 0) {
		clear_bit(MT76_STATE_ROC, &phy->mt76->state);
		goto out;
	}

	if (!wait_event_timeout(phy->roc_wait, phy->roc_grant, HZ)) {
		mt7902_mcu_abort_roc(phy, vif, phy->roc_token_id);
		clear_bit(MT76_STATE_ROC, &phy->mt76->state);
		err = -ETIMEDOUT;
	}

out:
	return err;
}

static int mt7902_remain_on_channel(struct ieee80211_hw *hw,
				    struct ieee80211_vif *vif,
				    struct ieee80211_channel *chan,
				    int duration,
				    enum ieee80211_roc_type type)
{
	printk(KERN_DEBUG "main.c - mt7902_remain_on_channel");
	struct mt792x_vif *mvif = (struct mt792x_vif *)vif->drv_priv;
	struct mt792x_phy *phy = mt792x_hw_phy(hw);
	int err;

	mt792x_mutex_acquire(phy->dev);
	err = mt7902_set_roc(phy, mvif, chan, duration, MT7902_ROC_REQ_ROC);
	mt792x_mutex_release(phy->dev);

	return err;
}

static int mt7902_cancel_remain_on_channel(struct ieee80211_hw *hw,
					   struct ieee80211_vif *vif)
{
	printk(KERN_DEBUG "main.c - mt7902_cancel_remain_on_channel");
	struct mt792x_vif *mvif = (struct mt792x_vif *)vif->drv_priv;
	struct mt792x_phy *phy = mt792x_hw_phy(hw);

	return mt7902_abort_roc(phy, mvif);
}

int mt7902_set_channel(struct mt76_phy *mphy)
{
	printk(KERN_DEBUG "main.c - mt7902_set_channel(mphy)");
	struct mt792x_phy *phy = mphy->priv;
	struct mt792x_dev *dev = phy->dev;
	int ret;

	mt76_connac_pm_wake(mphy, &dev->pm);
	ret = mt7902_mcu_set_chan_info(phy, MCU_EXT_CMD(CHANNEL_SWITCH));
	if (ret)
		goto out;

	mt792x_mac_set_timeing(phy);
	mt792x_mac_reset_counters(phy);
	phy->noise = 0;

out:
	mt76_connac_power_save_sched(mphy, &dev->pm);

	ieee80211_queue_delayed_work(mphy->hw, &mphy->mac_work,
				     MT792x_WATCHDOG_TIME);

	return ret;
}
EXPORT_SYMBOL_GPL(mt7902_set_channel);

static int mt7902_set_key(struct ieee80211_hw *hw, enum set_key_cmd cmd,
			  struct ieee80211_vif *vif, struct ieee80211_sta *sta,
			  struct ieee80211_key_conf *key)
{
	printk(KERN_DEBUG "main.c - mt7902_set_key(hw, cmd, vif, sta, key)");
	struct mt792x_dev *dev = mt792x_hw_dev(hw);
	struct mt792x_vif *mvif = (struct mt792x_vif *)vif->drv_priv;
	struct mt792x_sta *msta = sta ? (struct mt792x_sta *)sta->drv_priv :
				  &mvif->sta;
	struct mt76_wcid *wcid = &msta->deflink.wcid;
	u8 *wcid_keyidx = &wcid->hw_key_idx;
	int idx = key->keyidx, err = 0;
	printk(KERN_DEBUG "main.c - mt7902_set_key - wdid_keyidx: %x, idx: %d", wcid_keyidx, idx);


/**
 * enum nl80211_iftype - (virtual) interface types
 *
 * @NL80211_IFTYPE_UNSPECIFIED: unspecified type, driver decides
 * @NL80211_IFTYPE_ADHOC: independent BSS member
 * @NL80211_IFTYPE_STATION: managed BSS member
 * @NL80211_IFTYPE_AP: access point
 * @NL80211_IFTYPE_AP_VLAN: VLAN interface for access points; VLAN interfaces
 *	are a bit special in that they must always be tied to a pre-existing
 *	AP type interface.
 * @NL80211_IFTYPE_WDS: wireless distribution interface
 * @NL80211_IFTYPE_MONITOR: monitor interface receiving all frames
 * @NL80211_IFTYPE_MESH_POINT: mesh point
 * @NL80211_IFTYPE_P2P_CLIENT: P2P client
 * @NL80211_IFTYPE_P2P_GO: P2P group owner
 * @NL80211_IFTYPE_P2P_DEVICE: P2P device interface type, this is not a netdev
 *	and therefore can't be created in the normal ways, use the
 *	%NL80211_CMD_START_P2P_DEVICE and %NL80211_CMD_STOP_P2P_DEVICE
 *	commands to create and destroy one
 * @NL80211_IFTYPE_OCB: Outside Context of a BSS
 *	This mode corresponds to the MIB variable dot11OCBActivated=true
 * @NL80211_IFTYPE_NAN: NAN device interface type (not a netdev)
 * @NL80211_IFTYPE_MAX: highest interface type number currently defined
 * @NUM_NL80211_IFTYPES: number of defined interface types
 *
 * These values are used with the %NL80211_ATTR_IFTYPE
 * to set the type of an interface.
 *
 * /
enum nl80211_iftype {
	NL80211_IFTYPE_UNSPECIFIED,
	NL80211_IFTYPE_ADHOC,
	NL80211_IFTYPE_STATION,
	NL80211_IFTYPE_AP,
	NL80211_IFTYPE_AP_VLAN,
	NL80211_IFTYPE_WDS,
	NL80211_IFTYPE_MONITOR,
	NL80211_IFTYPE_MESH_POINT,
	NL80211_IFTYPE_P2P_CLIENT,
	NL80211_IFTYPE_P2P_GO,
	NL80211_IFTYPE_P2P_DEVICE,
	NL80211_IFTYPE_OCB,
	NL80211_IFTYPE_NAN,

	/* keep last * /
	NUM_NL80211_IFTYPES,
	NL80211_IFTYPE_MAX = NUM_NL80211_IFTYPES - 1
};


/**
 * struct ieee80211_key_conf - key information
 *
 * This key information is given by mac80211 to the driver by
 * the set_key() callback in &struct ieee80211_ops.
 *
 * @hw_key_idx: To be set by the driver, this is the key index the driver
 *	wants to be given when a frame is transmitted and needs to be
 *	encrypted in hardware.
 * @cipher: The key's cipher suite selector.
 * @tx_pn: PN used for TX keys, may be used by the driver as well if it
 *	needs to do software PN assignment by itself (e.g. due to TSO)
 * @flags: key flags, see &enum ieee80211_key_flags.
 * @keyidx: the key index (0-7)
 * @keylen: key material length
 * @key: key material. For ALG_TKIP the key is encoded as a 256-bit (32 byte)
 * 	data block:
 * 	- Temporal Encryption Key (128 bits)
 * 	- Temporal Authenticator Tx MIC Key (64 bits)
 * 	- Temporal Authenticator Rx MIC Key (64 bits)
 * @icv_len: The ICV length for this key type
 * @iv_len: The IV length for this key type
 * @link_id: the link ID, 0 for non-MLO, or -1 for pairwise keys
 * /
struct ieee80211_key_conf {
	atomic64_t tx_pn;
	u32 cipher;
	u8 icv_len;
	u8 iv_len;
	u8 hw_key_idx;
	s8 keyidx;
	u16 flags;
	s8 link_id;
	u8 keylen;
	u8 key[];
};


#define SUITE(oui, id)	(((oui) << 8) | (id))

/* cipher suite selectors * /
#define WLAN_CIPHER_SUITE_USE_GROUP	SUITE(0x000FAC, 0)
#define WLAN_CIPHER_SUITE_WEP40		SUITE(0x000FAC, 1)
#define WLAN_CIPHER_SUITE_TKIP		SUITE(0x000FAC, 2)
/* reserved: 				SUITE(0x000FAC, 3) * /
#define WLAN_CIPHER_SUITE_CCMP		SUITE(0x000FAC, 4)
#define WLAN_CIPHER_SUITE_WEP104	SUITE(0x000FAC, 5)
#define WLAN_CIPHER_SUITE_AES_CMAC	SUITE(0x000FAC, 6)
#define WLAN_CIPHER_SUITE_GCMP		SUITE(0x000FAC, 8)
#define WLAN_CIPHER_SUITE_GCMP_256	SUITE(0x000FAC, 9)
#define WLAN_CIPHER_SUITE_CCMP_256	SUITE(0x000FAC, 10)
#define WLAN_CIPHER_SUITE_BIP_GMAC_128	SUITE(0x000FAC, 11)
#define WLAN_CIPHER_SUITE_BIP_GMAC_256	SUITE(0x000FAC, 12)
#define WLAN_CIPHER_SUITE_BIP_CMAC_256	SUITE(0x000FAC, 13)

#define WLAN_CIPHER_SUITE_SMS4		SUITE(0x001472, 1)



	/* The hardware does not support per-STA RX GTK, fallback
	 * to software mode for these.
	 */
	printk(KERN_DEBUG "main.c - mt7902_set_key - vif->type: %d", vif->type);
	printk(KERN_DEBUG "main.c - mt7902_set_key - key->cipher: %x", key->cipher);
	

	if ((vif->type == NL80211_IFTYPE_ADHOC ||
	     vif->type == NL80211_IFTYPE_MESH_POINT) &&
	    (key->cipher == WLAN_CIPHER_SUITE_TKIP ||
	     key->cipher == WLAN_CIPHER_SUITE_CCMP) &&
	    !(key->flags & IEEE80211_KEY_FLAG_PAIRWISE))
		return -EOPNOTSUPP;

	// Returning not supp here forces software decryption, because hardware decryption is currently not working
	// if(key->cipher == WLAN_CIPHER_SUITE_CCMP){
	// 	printk(KERN_INFO "set_key: falling back to software decryption, returning not supp for PTK/GTK\n");
	// 	return -EOPNOTSUPP;
	// }

	/* fall back to sw encryption for unsupported ciphers */
	switch (key->cipher) {
	case WLAN_CIPHER_SUITE_AES_CMAC:
		key->flags |= IEEE80211_KEY_FLAG_GENERATE_MMIE;
		wcid_keyidx = &wcid->hw_key_idx2;
		break;
	case WLAN_CIPHER_SUITE_WEP40:
	case WLAN_CIPHER_SUITE_WEP104:
		if (!mvif->wep_sta)
			return -EOPNOTSUPP;
		break;
	case WLAN_CIPHER_SUITE_TKIP:
	case WLAN_CIPHER_SUITE_CCMP:
	case WLAN_CIPHER_SUITE_CCMP_256:
	case WLAN_CIPHER_SUITE_GCMP:
	case WLAN_CIPHER_SUITE_GCMP_256:
	case WLAN_CIPHER_SUITE_SMS4:
		break;
	default:
		return -EOPNOTSUPP;
	}

	mt792x_mutex_acquire(dev);

	if (cmd == SET_KEY) {
		*wcid_keyidx = idx;
	} else {
		if (idx == *wcid_keyidx)
			*wcid_keyidx = -1;

		/* For security issue we don't trigger the key deletion when
		 * reassociating. But we should trigger the deletion process
		 * to avoid using incorrect cipher after disconnection,
		 */
		if (vif->type != NL80211_IFTYPE_STATION || vif->cfg.assoc)
			goto out;
	}
	printk(KERN_DEBUG "main.c - mt7902_set_key - mt76_wcid_key_setup");
	
	mt76_wcid_key_setup(&dev->mt76, wcid, key);
	err = mt76_connac_mcu_add_key(&dev->mt76, vif, &msta->deflink.bip,
				      key, MCU_EXT_CMD(STA_REC_UPDATE),
				      &msta->deflink.wcid, cmd);
	printk(KERN_INFO "add key, ret = %d\n", err);
	if (err)
		goto out;

	if (key->cipher == WLAN_CIPHER_SUITE_WEP104 ||
	    key->cipher == WLAN_CIPHER_SUITE_WEP40)
		err = mt76_connac_mcu_add_key(&dev->mt76, vif,
					      &mvif->wep_sta->deflink.bip,
					      key, MCU_EXT_CMD(STA_REC_UPDATE),
					      &mvif->wep_sta->deflink.wcid, cmd);
out:
	mt792x_mutex_release(dev);

	return err;
}

static void
mt7902_pm_interface_iter(void *priv, u8 *mac, struct ieee80211_vif *vif)
{
	printk(KERN_DEBUG "main.c - mt7902_pm_interface_iter");
	struct mt792x_dev *dev = priv;
	struct ieee80211_hw *hw = mt76_hw(dev);
	bool pm_enable = dev->pm.enable;
	int err;

	err = mt7902_mcu_set_beacon_filter(dev, vif, pm_enable);
	if (err < 0)
		return;

	if (pm_enable) {
		vif->driver_flags |= IEEE80211_VIF_BEACON_FILTER;
		ieee80211_hw_set(hw, CONNECTION_MONITOR);
	} else {
		vif->driver_flags &= ~IEEE80211_VIF_BEACON_FILTER;
		__clear_bit(IEEE80211_HW_CONNECTION_MONITOR, hw->flags);
	}
}

static void
mt7902_sniffer_interface_iter(void *priv, u8 *mac, struct ieee80211_vif *vif)
{
	printk(KERN_DEBUG "main.c - mt7902_sniffer_interface_iter");
	struct mt792x_dev *dev = priv;
	struct ieee80211_hw *hw = mt76_hw(dev);
	struct mt76_connac_pm *pm = &dev->pm;
	bool monitor = !!(hw->conf.flags & IEEE80211_CONF_MONITOR);

	mt7902_mcu_set_sniffer(dev, vif, monitor);
	pm->enable = pm->enable_user && !monitor;
	pm->ds_enable = pm->ds_enable_user && !monitor;

	mt76_connac_mcu_set_deep_sleep(&dev->mt76, pm->ds_enable);

	if (monitor)
		mt7902_mcu_set_beacon_filter(dev, vif, false);
}

void mt7902_set_runtime_pm(struct mt792x_dev *dev)
{
	printk(KERN_DEBUG "main.c - mt7902_set_runtime_pm");
	struct ieee80211_hw *hw = mt76_hw(dev);
	struct mt76_connac_pm *pm = &dev->pm;
	bool monitor = !!(hw->conf.flags & IEEE80211_CONF_MONITOR);

	pm->enable = pm->enable_user && !monitor;
	ieee80211_iterate_active_interfaces(hw,
					    IEEE80211_IFACE_ITER_RESUME_ALL,
					    mt7902_pm_interface_iter, dev);
	pm->ds_enable = pm->ds_enable_user && !monitor;
	mt76_connac_mcu_set_deep_sleep(&dev->mt76, pm->ds_enable);
}

static int mt7902_config(struct ieee80211_hw *hw, u32 changed)
{
	printk(KERN_DEBUG "main.c - mt7902_config(hw, changed: 0x%x)", changed);
	struct mt792x_dev *dev = mt792x_hw_dev(hw);
	struct mt792x_phy *phy = mt792x_hw_phy(hw);
	int ret = 0;

	if (changed & IEEE80211_CONF_CHANGE_CHANNEL) {
		ret = mt76_update_channel(phy->mt76);
		if (ret)
			return ret;
	}

	mt792x_mutex_acquire(dev);

	if (changed & IEEE80211_CONF_CHANGE_POWER) {
		ret = mt7902_set_tx_sar_pwr(hw, NULL);
		if (ret)
			goto out;
	}

	if (changed & IEEE80211_CONF_CHANGE_MONITOR) {
		ieee80211_iterate_active_interfaces(hw,
						    IEEE80211_IFACE_ITER_RESUME_ALL,
						    mt7902_sniffer_interface_iter, dev);
	}

out:
	mt792x_mutex_release(dev);

	return ret;
}

static void mt7902_configure_filter(struct ieee80211_hw *hw,
				    unsigned int changed_flags,
				    unsigned int *total_flags,
				    u64 multicast)
{
	printk(KERN_DEBUG "main.c - mt7902_configure_filter(hw, changed_flags, total_flags, multicast: 0x%x)", multicast);
#define MT7902_FILTER_FCSFAIL    BIT(2)
#define MT7902_FILTER_CONTROL    BIT(5)
#define MT7902_FILTER_OTHER_BSS  BIT(6)
#define MT7902_FILTER_ENABLE     BIT(31)

	struct mt792x_dev *dev = mt792x_hw_dev(hw);
	u32 flags = MT7902_FILTER_ENABLE;

#define MT7902_FILTER(_fif, _type) do {			\
		if (*total_flags & (_fif))		\
			flags |= MT7902_FILTER_##_type;	\
	} while (0)

	MT7902_FILTER(FIF_FCSFAIL, FCSFAIL);
	MT7902_FILTER(FIF_CONTROL, CONTROL);
	MT7902_FILTER(FIF_OTHER_BSS, OTHER_BSS);

	mt792x_mutex_acquire(dev);
	mt7902_mcu_set_rxfilter(dev, flags, 0, 0);
	mt792x_mutex_release(dev);

	*total_flags &= (FIF_OTHER_BSS | FIF_FCSFAIL | FIF_CONTROL);
}

static void
mt7902_update_bss_color(struct ieee80211_hw *hw,
			struct ieee80211_vif *vif,
			struct cfg80211_he_bss_color *bss_color)
{
	struct mt792x_dev *dev = mt7902_hw_dev(hw);

	switch (vif->type) {
	case NL80211_IFTYPE_AP: {
		struct mt792x_vif *mvif = (struct mt792x_vif *)vif->drv_priv;

		if (mvif->bss_conf.mt76.omac_idx > HW_BSSID_MAX)
			return;
		fallthrough;
	}
	case NL80211_IFTYPE_STATION:
		mt7902_mcu_update_bss_color(dev, vif, bss_color);
		break;
	default:
		break;
	}
} 

static void mt7902_bss_info_changed(struct ieee80211_hw *hw,
				    struct ieee80211_vif *vif,
				    struct ieee80211_bss_conf *info,
				    u64 changed)
{
	printk(KERN_DEBUG "main.c - mt7902_bss_info_changed(hw, vif, info, changed: 0x%x)", changed);
	struct mt792x_phy *phy = mt792x_hw_phy(hw);
	struct mt792x_dev *dev = mt792x_hw_dev(hw);
	int set_bss_info = -1, set_sta = -1;

	mt792x_mutex_acquire(dev);

	/*
	 * station mode uses BSSID to map the wlan entry to a peer,
	 * and then peer references bss_info_rfch to set bandwidth cap.
	 */
	// if (changed & BSS_CHANGED_BSSID &&
	//     vif->type == NL80211_IFTYPE_STATION) {
	// 	bool join = !is_zero_ether_addr(info->bssid);
			
	// 	mt7902_mcu_add_bss_info(phy, vif, join);
	// 	mt7902_mcu_add_sta(dev, vif, NULL, CONN_STATE_PORT_SECURE, join);
	// }
		
	// if (changed & BSS_CHANGED_ASSOC) {
	// 	mt7902_mcu_add_bss_info(phy, vif, true);
	// 	mt7902_mcu_add_obss_spr(dev, vif, info->he_obss_pd.enable);
	// }

	// if (changed & BSS_CHANGED_BEACON_ENABLED &&
	//     info->enable_beacon &&
	//     vif->type != NL80211_IFTYPE_AP)
	// 	set_bss_info = set_sta = 1;

	// if (set_bss_info == 1)
	// 	mt7902_mcu_add_bss_info(phy, vif, true);
	// if (set_sta == 1)
	// 	mt7902_mcu_add_sta(dev, vif, NULL, CONN_STATE_PORT_SECURE, false);

	// if (changed & BSS_CHANGED_ERP_CTS_PROT)
	// 	mt7902_mac_enable_rtscts(dev, vif, info->use_cts_prot);

	if (changed & BSS_CHANGED_ERP_SLOT) {
		int slottime = info->use_short_slot ? 9 : 20;

		if (slottime != phy->slottime) {
			phy->slottime = slottime;
			mt7902_mac_set_timing(phy);
		}
	}

	// if(changed & BSS_CHANGED_BEACON_ENABLED && info->enable_beacon) {
	// 	mt7902_mcu_add_bss_info(phy, vif, true);
	// 	mt7902_mcu_add_sta(dev, vif, NULL, CONN_STATE_PORT_SECURE, true);
	// }

	/* ensure that enable txcmd_mode after bss_info */
	if (changed & (BSS_CHANGED_QOS | BSS_CHANGED_BEACON_ENABLED))
		mt7902_mcu_set_tx(dev, vif);

	
	if (changed & BSS_CHANGED_CQM)
		mt7902_mcu_set_rssimonitor(dev, vif);

	if (changed & BSS_CHANGED_ASSOC) {
		mt7902_mcu_sta_update(dev, NULL, vif, true,
				      MT76_STA_INFO_STATE_ASSOC);
		mt7902_mcu_set_beacon_filter(dev, vif, vif->cfg.assoc);
	}

	if (changed & BSS_CHANGED_HE_OBSS_PD)
		mt7902_mcu_add_obss_spr(dev, vif, &info->he_obss_pd);

	if (changed & BSS_CHANGED_HE_BSS_COLOR)
		mt7902_update_bss_color(hw, vif, &info->he_bss_color);

	if (changed & (BSS_CHANGED_BEACON |
		       BSS_CHANGED_BEACON_ENABLED))
		mt7902_mcu_add_beacon(hw, vif, changed);

	mt792x_mutex_release(dev);
}

static void
mt7902_calc_vif_num(void *priv, u8 *mac, struct ieee80211_vif *vif)
{
	printk(KERN_DEBUG "main.c - mt7902_calc_vif_num(priv, mac, vif) - vif->type: %d", vif->type);
	u32 *num = priv;

	if (!priv)
		return;

	switch (vif->type) {
	case NL80211_IFTYPE_STATION:
	case NL80211_IFTYPE_P2P_CLIENT:
	case NL80211_IFTYPE_AP:
	case NL80211_IFTYPE_P2P_GO:
		*num += 1;
		break;
	default:
		break;
	}
}

static void
mt7902_regd_set_6ghz_power_type(struct ieee80211_vif *vif, bool is_add)
{
	printk(KERN_DEBUG "main.c - mt7902_regd_set_6ghz_power_type(vif, is_add: %d)", is_add);
	struct mt792x_vif *mvif = (struct mt792x_vif *)vif->drv_priv;
	struct mt792x_phy *phy = mvif->phy;
	struct mt792x_dev *dev = phy->dev;
	u32 valid_vif_num = 0;

	ieee80211_iterate_active_interfaces(mt76_hw(dev),
					    IEEE80211_IFACE_ITER_RESUME_ALL,
					    mt7902_calc_vif_num, &valid_vif_num);

	if (valid_vif_num > 1) {
		phy->power_type = MT_AP_DEFAULT;
		goto out;
	}

	if (!is_add)
		vif->bss_conf.power_type = IEEE80211_REG_UNSET_AP;

	printk(KERN_DEBUG "main.c - mt7902_regd_set_6ghz_power_type - vif->bss_conf.power_type: %d", vif->bss_conf.power_type);

	switch (vif->bss_conf.power_type) {
	case IEEE80211_REG_SP_AP:
		phy->power_type = MT_AP_SP;
		break;
	case IEEE80211_REG_VLP_AP:
		phy->power_type = MT_AP_VLP;
		break;
	case IEEE80211_REG_LPI_AP:
		phy->power_type = MT_AP_LPI;
		break;
	case IEEE80211_REG_UNSET_AP:
		phy->power_type = MT_AP_UNSET;
		break;
	default:
		phy->power_type = MT_AP_DEFAULT;
		break;
	}

out:
	mt7902_mcu_set_clc(dev, dev->mt76.alpha2, dev->country_ie_env);
}

int mt7902_mac_sta_add(struct mt76_dev *mdev, struct ieee80211_vif *vif,
		       struct ieee80211_sta *sta)
{
	printk(KERN_DEBUG "main.c - mt7902_mac_sta_add");
	struct mt792x_dev *dev = container_of(mdev, struct mt792x_dev, mt76);
	struct mt792x_sta *msta = (struct mt792x_sta *)sta->drv_priv;
	struct mt792x_vif *mvif = (struct mt792x_vif *)vif->drv_priv;
	int ret, idx;

	idx = mt76_wcid_alloc(dev->mt76.wcid_mask, MT792x_WTBL_STA - 1);
	if (idx < 0)
		return -ENOSPC;

	INIT_LIST_HEAD(&msta->deflink.wcid.poll_list);
	msta->vif = mvif;
	msta->deflink.wcid.sta = 1;
	msta->deflink.wcid.idx = idx;
	msta->deflink.wcid.phy_idx = mvif->bss_conf.mt76.band_idx;
	msta->deflink.wcid.tx_info |= MT_WCID_TX_INFO_SET;
	msta->deflink.last_txs = jiffies;
	msta->deflink.sta = msta;

	ret = mt76_connac_pm_wake(&dev->mphy, &dev->pm);
	if (ret)
		return ret;

	if (vif->type == NL80211_IFTYPE_STATION)
		mvif->wep_sta = msta;

	mt7902_mac_wtbl_update(dev, idx,
			       MT_WTBL_UPDATE_ADM_COUNT_CLEAR);

	ret = mt7902_mcu_sta_update(dev, sta, vif, true,
				    MT76_STA_INFO_STATE_NONE);
	if (ret)
		return ret;

	mt7902_regd_set_6ghz_power_type(vif, true);

	mt76_connac_power_save_sched(&dev->mphy, &dev->pm);

	return 0;
}
EXPORT_SYMBOL_GPL(mt7902_mac_sta_add);

int mt7902_mac_sta_event(struct mt76_dev *mdev, struct ieee80211_vif *vif,
			 struct ieee80211_sta *sta, enum mt76_sta_event ev)
{
	printk(KERN_DEBUG "main.c - mt7902_mac_sta_event(mdev, vif, sta, ev: %d)", ev);
	struct mt792x_dev *dev = container_of(mdev, struct mt792x_dev, mt76);
	struct mt792x_sta *msta = (struct mt792x_sta *)sta->drv_priv;
	struct mt792x_vif *mvif = (struct mt792x_vif *)vif->drv_priv;

	if (ev != MT76_STA_EVENT_ASSOC)
	    return 0;

	mt792x_mutex_acquire(dev);

	if (vif->type == NL80211_IFTYPE_STATION && !sta->tdls){
		//mt76_connac_mcu_uni_add_bss(&dev->mphy, vif, &mvif->sta.deflink.wcid,
		//			    true, mvif->bss_conf.mt76.ctx);
		u32 ret = mt7902_mcu_add_bss_info(mvif->phy, vif, true);
		printk(KERN_INFO "mac_sta_event: add bss info ret = %d\n", ret);
	}

	ewma_avg_signal_init(&msta->deflink.avg_ack_signal);

	mt7902_mac_wtbl_update(dev, msta->deflink.wcid.idx,
			       MT_WTBL_UPDATE_ADM_COUNT_CLEAR);
	memset(msta->deflink.airtime_ac, 0, sizeof(msta->deflink.airtime_ac));

	mt7902_mcu_sta_update(dev, sta, vif, true, MT76_STA_INFO_STATE_ASSOC);

	mt792x_mutex_release(dev);

	return 0;
}
EXPORT_SYMBOL_GPL(mt7902_mac_sta_event);

void mt7902_mac_sta_remove(struct mt76_dev *mdev, struct ieee80211_vif *vif,
			   struct ieee80211_sta *sta)
{
	printk(KERN_DEBUG "main.c - mt7902_mac_sta_remove");
	struct mt792x_dev *dev = container_of(mdev, struct mt792x_dev, mt76);
	struct mt792x_sta *msta = (struct mt792x_sta *)sta->drv_priv;

	mt7902_roc_abort_sync(dev);
	mt76_connac_free_pending_tx_skbs(&dev->pm, &msta->deflink.wcid);
	mt76_connac_pm_wake(&dev->mphy, &dev->pm);

	mt7902_mcu_sta_update(dev, sta, vif, false, MT76_STA_INFO_STATE_NONE);
	mt7902_mac_wtbl_update(dev, msta->deflink.wcid.idx,
			       MT_WTBL_UPDATE_ADM_COUNT_CLEAR);

	if (vif->type == NL80211_IFTYPE_STATION) {
		struct mt792x_vif *mvif = (struct mt792x_vif *)vif->drv_priv;

		mvif->wep_sta = NULL;
		ewma_rssi_init(&mvif->bss_conf.rssi);
		if (!sta->tdls){
			//mt76_connac_mcu_uni_add_bss(&dev->mphy, vif,
			//			    &mvif->sta.deflink.wcid, false,
			//			    mvif->bss_conf.mt76.ctx);
			u32 ret = mt7902_mcu_add_bss_info(mvif->phy, vif, true);
			printk(KERN_INFO "mac_sta_remove: add bss info ret = %d\n", ret);
		}
	}

	spin_lock_bh(&dev->mt76.sta_poll_lock);
	if (!list_empty(&msta->deflink.wcid.poll_list))
		list_del_init(&msta->deflink.wcid.poll_list);
	spin_unlock_bh(&dev->mt76.sta_poll_lock);

	mt7902_regd_set_6ghz_power_type(vif, false);

	mt76_connac_power_save_sched(&dev->mphy, &dev->pm);
}
EXPORT_SYMBOL_GPL(mt7902_mac_sta_remove);

static int mt7902_set_rts_threshold(struct ieee80211_hw *hw, u32 val)
{
	printk(KERN_DEBUG "main.c - mt7902_set_rts_threshold");
	struct mt792x_dev *dev = mt792x_hw_dev(hw);

	mt792x_mutex_acquire(dev);
	mt76_connac_mcu_set_rts_thresh(&dev->mt76, val, 0);
	mt792x_mutex_release(dev);

	return 0;
}

static int
mt7902_ampdu_action(struct ieee80211_hw *hw, struct ieee80211_vif *vif,
		    struct ieee80211_ampdu_params *params)
{
	printk(KERN_DEBUG "main.c - mt7902_ampdu_action");
	enum ieee80211_ampdu_mlme_action action = params->action;
	struct mt792x_dev *dev = mt792x_hw_dev(hw);
	struct ieee80211_sta *sta = params->sta;
	struct ieee80211_txq *txq = sta->txq[params->tid];
	struct mt792x_sta *msta = (struct mt792x_sta *)sta->drv_priv;
	u16 tid = params->tid;
	u16 ssn = params->ssn;
	struct mt76_txq *mtxq;
	int ret = 0;

	if (!txq)
		return -EINVAL;

	mtxq = (struct mt76_txq *)txq->drv_priv;

	mt792x_mutex_acquire(dev);
	switch (action) {
	case IEEE80211_AMPDU_RX_START:
		mt76_rx_aggr_start(&dev->mt76, &msta->deflink.wcid, tid, ssn,
				   params->buf_size);
		mt7902_mcu_uni_rx_ba(dev, params, true);
		break;
	case IEEE80211_AMPDU_RX_STOP:
		mt76_rx_aggr_stop(&dev->mt76, &msta->deflink.wcid, tid);
		mt7902_mcu_uni_rx_ba(dev, params, false);
		break;
	case IEEE80211_AMPDU_TX_OPERATIONAL:
		mtxq->aggr = true;
		mtxq->send_bar = false;
		mt7902_mcu_uni_tx_ba(dev, params, true);
		break;
	case IEEE80211_AMPDU_TX_STOP_FLUSH:
	case IEEE80211_AMPDU_TX_STOP_FLUSH_CONT:
		mtxq->aggr = false;
		clear_bit(tid, &msta->deflink.wcid.ampdu_state);
		mt7902_mcu_uni_tx_ba(dev, params, false);
		break;
	case IEEE80211_AMPDU_TX_START:
		set_bit(tid, &msta->deflink.wcid.ampdu_state);
		ret = IEEE80211_AMPDU_TX_START_IMMEDIATE;
		break;
	case IEEE80211_AMPDU_TX_STOP_CONT:
		mtxq->aggr = false;
		clear_bit(tid, &msta->deflink.wcid.ampdu_state);
		mt7902_mcu_uni_tx_ba(dev, params, false);
		ieee80211_stop_tx_ba_cb_irqsafe(vif, sta->addr, tid);
		break;
	}
	mt792x_mutex_release(dev);

	return ret;
}

static int mt7902_sta_state(struct ieee80211_hw *hw,
			    struct ieee80211_vif *vif,
			    struct ieee80211_sta *sta,
			    enum ieee80211_sta_state old_state,
			    enum ieee80211_sta_state new_state)
{
	printk(KERN_DEBUG "main.c - mt792x_sta_state(hw, vif, sta, old_state: %d, new_state: %d)", old_state, new_state);
	struct mt792x_dev *dev = mt792x_hw_dev(hw);

	if (dev->pm.ds_enable) {
		mt792x_mutex_acquire(dev);
		mt76_connac_sta_state_dp(&dev->mt76, old_state, new_state);
		mt792x_mutex_release(dev);
	}

	return mt76_sta_state(hw, vif, sta, old_state, new_state);
}

void mt7902_scan_work(struct work_struct *work)
{
	printk(KERN_DEBUG "main.c - mt7902_scan_work(work)");
	struct mt792x_phy *phy;

	phy = (struct mt792x_phy *)container_of(work, struct mt792x_phy,
						scan_work.work);

	while (true) {
		struct mt76_connac2_mcu_rxd *rxd;
		struct sk_buff *skb;

		spin_lock_bh(&phy->dev->mt76.lock);
		skb = __skb_dequeue(&phy->scan_event_list);
		spin_unlock_bh(&phy->dev->mt76.lock);

		if (!skb)
			break;

		rxd = (struct mt76_connac2_mcu_rxd *)skb->data;
		if (rxd->eid == MCU_EVENT_SCHED_SCAN_DONE) {
			ieee80211_sched_scan_results(phy->mt76->hw);
		} else if (test_and_clear_bit(MT76_HW_SCANNING,
					      &phy->mt76->state)) {
			struct cfg80211_scan_info info = {
				.aborted = false,
			};

			ieee80211_scan_completed(phy->mt76->hw, &info);
		}
		dev_kfree_skb(skb);
	}
}

static int
mt7902_hw_scan(struct ieee80211_hw *hw, struct ieee80211_vif *vif,
	       struct ieee80211_scan_request *req)
{
	printk(KERN_DEBUG "main.c - mt7902_hw_scan(hw, vif, req)");
	struct mt792x_dev *dev = mt792x_hw_dev(hw);
	struct mt76_phy *mphy = hw->priv;
	int err;

	mt792x_mutex_acquire(dev);
	err = mt76_connac_mcu_hw_scan(mphy, vif, req);
	mt792x_mutex_release(dev);

	return err;
}

static void
mt7902_cancel_hw_scan(struct ieee80211_hw *hw, struct ieee80211_vif *vif)
{
	printk(KERN_DEBUG "main.c - mt7902_cancel_hw_scan");
	struct mt792x_dev *dev = mt792x_hw_dev(hw);
	struct mt76_phy *mphy = hw->priv;

	mt792x_mutex_acquire(dev);
	mt76_connac_mcu_cancel_hw_scan(mphy, vif);
	mt792x_mutex_release(dev);
}

static int
mt7902_start_sched_scan(struct ieee80211_hw *hw, struct ieee80211_vif *vif,
			struct cfg80211_sched_scan_request *req,
			struct ieee80211_scan_ies *ies)
{
	printk(KERN_DEBUG "main.c - mt792x_start_sched_scan");
	struct mt792x_dev *dev = mt792x_hw_dev(hw);
	struct mt76_phy *mphy = hw->priv;
	int err;

	mt792x_mutex_acquire(dev);

	err = mt76_connac_mcu_sched_scan_req(mphy, vif, req);
	if (err < 0)
		goto out;

	err = mt76_connac_mcu_sched_scan_enable(mphy, vif, true);
out:
	mt792x_mutex_release(dev);

	return err;
}

static int
mt7902_stop_sched_scan(struct ieee80211_hw *hw, struct ieee80211_vif *vif)
{
	printk(KERN_DEBUG "main.c - mt7902_stop_sched_scan");
	struct mt792x_dev *dev = mt792x_hw_dev(hw);
	struct mt76_phy *mphy = hw->priv;
	int err;

	mt792x_mutex_acquire(dev);
	err = mt76_connac_mcu_sched_scan_enable(mphy, vif, false);
	mt792x_mutex_release(dev);

	return err;
}

static int
mt7902_set_antenna(struct ieee80211_hw *hw, u32 tx_ant, u32 rx_ant)
{
	printk(KERN_DEBUG "main.c - mt7902_set_antenna");
	struct mt792x_dev *dev = mt792x_hw_dev(hw);
	struct mt792x_phy *phy = mt792x_hw_phy(hw);
	int max_nss = hweight8(hw->wiphy->available_antennas_tx);

	if (!tx_ant || tx_ant != rx_ant || ffs(tx_ant) > max_nss)
		return -EINVAL;

	if ((BIT(hweight8(tx_ant)) - 1) != tx_ant)
		return -EINVAL;

	mt792x_mutex_acquire(dev);

	phy->mt76->antenna_mask = tx_ant;
	phy->mt76->chainmask = tx_ant;

	mt76_set_stream_caps(phy->mt76, true);
	mt7902_set_stream_he_caps(phy);

	mt792x_mutex_release(dev);

	return 0;
}

#ifdef CONFIG_PM
/*
static int mt7902_suspend(struct ieee80211_hw *hw,
			  struct cfg80211_wowlan *wowlan)
{
	struct mt792x_dev *dev = mt792x_hw_dev(hw);
	struct mt792x_phy *phy = mt792x_hw_phy(hw);

	cancel_delayed_work_sync(&phy->scan_work);
	cancel_delayed_work_sync(&phy->mt76->mac_work);

	cancel_delayed_work_sync(&dev->pm.ps_work);
	mt76_connac_free_pending_tx_skbs(&dev->pm, NULL);

	mt792x_mutex_acquire(dev);

	clear_bit(MT76_STATE_RUNNING, &phy->mt76->state);
	ieee80211_iterate_active_interfaces(hw,
					    IEEE80211_IFACE_ITER_RESUME_ALL,
					    mt7902_mcu_set_suspend_iter,
					    &dev->mphy);

	mt792x_mutex_release(dev);

	return 0;
}

static int mt7902_resume(struct ieee80211_hw *hw)
{
	struct mt792x_dev *dev = mt792x_hw_dev(hw);
	struct mt792x_phy *phy = mt792x_hw_phy(hw);

	mt792x_mutex_acquire(dev);

	set_bit(MT76_STATE_RUNNING, &phy->mt76->state);
	ieee80211_iterate_active_interfaces(hw,
					    IEEE80211_IFACE_ITER_RESUME_ALL,
					    mt76_connac_mcu_set_suspend_iter,
					    &dev->mphy);

	ieee80211_queue_delayed_work(hw, &phy->mt76->mac_work,
				     MT792x_WATCHDOG_TIME);

	mt792x_mutex_release(dev);

	return 0;
}

static void mt7902_set_rekey_data(struct ieee80211_hw *hw,
				  struct ieee80211_vif *vif,
				  struct cfg80211_gtk_rekey_data *data)
{
	struct mt792x_dev *dev = mt792x_hw_dev(hw);

	mt792x_mutex_acquire(dev);
	mt76_connac_mcu_update_gtk_rekey(hw, vif, data);
	mt792x_mutex_release(dev);
}*/
#endif /* CONFIG_PM */

static void mt7902_sta_set_decap_offload(struct ieee80211_hw *hw,
					 struct ieee80211_vif *vif,
					 struct ieee80211_sta *sta,
					 bool enabled)
{
	printk(KERN_DEBUG "main.c - mt792x_sta_set_decap_offload");
	struct mt792x_sta *msta = (struct mt792x_sta *)sta->drv_priv;
	struct mt792x_dev *dev = mt792x_hw_dev(hw);

	if (!msta->deflink.wcid.sta)
		return;

	mt792x_mutex_acquire(dev);

	if (enabled)
		set_bit(MT_WCID_FLAG_HDR_TRANS, &msta->deflink.wcid.flags);
	else
		clear_bit(MT_WCID_FLAG_HDR_TRANS, &msta->deflink.wcid.flags);

	u32 ret = mt76_connac_mcu_sta_update_hdr_trans(&dev->mt76, vif, &msta->deflink.wcid,
					     MCU_EXT_CMD(STA_REC_UPDATE));

	printk(KERN_INFO "decap_offload - update_hdr_trans sta rec update, ret = %d\n", ret);

	mt792x_mutex_release(dev);
}

#if IS_ENABLED(CONFIG_IPV6)
/*
static void mt7902_ipv6_addr_change(struct ieee80211_hw *hw,
				    struct ieee80211_vif *vif,
				    struct inet6_dev *idev)
{
	struct mt792x_vif *mvif = (struct mt792x_vif *)vif->drv_priv;
	struct mt792x_dev *dev = mt792x_hw_dev(hw);
	struct inet6_ifaddr *ifa;
	struct in6_addr ns_addrs[IEEE80211_BSS_ARP_ADDR_LIST_LEN];
	struct sk_buff *skb;
	u8 i, idx = 0;

	struct {
		struct {
			u8 bss_idx;
			u8 pad[3];
		} __packed hdr;
		struct mt76_connac_arpns_tlv arpns;
	} req_hdr = {
		.hdr = {
			.bss_idx = mvif->bss_conf.mt76.idx,
		},
		.arpns = {
			.tag = cpu_to_le16(UNI_OFFLOAD_OFFLOAD_ND),
			.mode = 2,
			.option = 1,
		},
	};

	read_lock_bh(&idev->lock);
	list_for_each_entry(ifa, &idev->addr_list, if_list) {
		if (ifa->flags & IFA_F_TENTATIVE)
			continue;
		ns_addrs[idx] = ifa->addr;
		if (++idx >= IEEE80211_BSS_ARP_ADDR_LIST_LEN)
			break;
	}
	read_unlock_bh(&idev->lock);

	if (!idx)
		return;

	req_hdr.arpns.ips_num = idx;
	req_hdr.arpns.len = cpu_to_le16(sizeof(struct mt76_connac_arpns_tlv)
					+ idx * sizeof(struct in6_addr));
	skb = __mt76_mcu_msg_alloc(&dev->mt76, &req_hdr,
			sizeof(req_hdr) + idx * sizeof(struct in6_addr),
			sizeof(req_hdr), GFP_ATOMIC);
	if (!skb)
		return;

	for (i = 0; i < idx; i++)
		skb_put_data(skb, &ns_addrs[i].in6_u, sizeof(struct in6_addr));

	skb_queue_tail(&dev->ipv6_ns_list, skb);

	ieee80211_queue_work(dev->mt76.hw, &dev->ipv6_ns_work);
}*/
#endif

int mt7902_set_tx_sar_pwr(struct ieee80211_hw *hw,
			  const struct cfg80211_sar_specs *sar)
{
	printk(KERN_DEBUG "main.c - mt7902_set_tx_sar_pwr(hw, sar)");
	struct mt76_phy *mphy = hw->priv;

	if (sar) {
		int err = mt76_init_sar_power(hw, sar);

		if (err)
			return err;
	}
	mt792x_init_acpi_sar_power(mt792x_hw_phy(hw), !sar);

	return mt76_connac_mcu_set_rate_txpower(mphy);
}

static int mt7902_set_sar_specs(struct ieee80211_hw *hw,
				const struct cfg80211_sar_specs *sar)
{
	printk(KERN_DEBUG "main.c - mt7902_set_sar_specs");
	struct mt792x_dev *dev = mt792x_hw_dev(hw);
	int err;

	mt792x_mutex_acquire(dev);
	err = mt7902_mcu_set_clc(dev, dev->mt76.alpha2,
				 dev->country_ie_env);
	if (err < 0)
		goto out;

	err = mt7902_set_tx_sar_pwr(hw, sar);
out:
	mt792x_mutex_release(dev);

	return err;
}

static void
mt7902_channel_switch_beacon(struct ieee80211_hw *hw,
			     struct ieee80211_vif *vif,
			     struct cfg80211_chan_def *chandef)
{
	//struct mt792x_dev *dev = mt792x_hw_dev(hw);

	//mt792x_mutex_acquire(dev);
	//mt7902_mcu_uni_add_beacon_offload(dev, hw, vif, true);
	//mt792x_mutex_release(dev);
}

static int
mt7902_start_ap(struct ieee80211_hw *hw, struct ieee80211_vif *vif,
		struct ieee80211_bss_conf *link_conf)
{
	printk(KERN_DEBUG "main.c - mt792x_start_ap");
	struct mt792x_vif *mvif = (struct mt792x_vif *)vif->drv_priv;
	struct mt792x_dev *dev = mt792x_hw_dev(hw);
	int err;

	mt792x_mutex_acquire(dev);

	//err = mt76_connac_mcu_uni_add_bss(phy->mt76, vif, &mvif->sta.deflink.wcid,
	//				  true, mvif->bss_conf.mt76.ctx);
	
	err = mt7902_mcu_add_bss_info(mvif->phy, vif, true);
	printk(KERN_INFO "start_ap: add bss info, ret = %d\n", err);

	if (err)
		goto out;

	err = mt7902_mcu_set_bss_pm(dev, vif, true);
	if (err)
		goto out;

	err = mt7902_mcu_sta_update(dev, NULL, vif, true,
				    MT76_STA_INFO_STATE_NONE);
out:
	mt792x_mutex_release(dev);

	return err;
}

static void
mt7902_stop_ap(struct ieee80211_hw *hw, struct ieee80211_vif *vif,
	       struct ieee80211_bss_conf *link_conf)
{
	printk(KERN_DEBUG "main.c - mt7902_stop_ap");
	struct mt792x_vif *mvif = (struct mt792x_vif *)vif->drv_priv;
	struct mt792x_dev *dev = mt792x_hw_dev(hw);
	int err;

	mt792x_mutex_acquire(dev);

	err = mt7902_mcu_set_bss_pm(dev, vif, false);
	if (err)
		goto out;

	//mt76_connac_mcu_uni_add_bss(phy->mt76, vif, &mvif->sta.deflink.wcid, false,
	//			    mvif->bss_conf.mt76.ctx);

	err = mt7902_mcu_add_bss_info(mvif->phy, vif, false);
	printk(KERN_INFO "stop_ap: add bss info, ret = %d\n", err);

out:
	mt792x_mutex_release(dev);
}

static int
mt7902_add_chanctx(struct ieee80211_hw *hw,
		   struct ieee80211_chanctx_conf *ctx)
{
	printk(KERN_DEBUG "main.c - mt7902_add_chanctx");
	struct mt792x_dev *dev = mt792x_hw_dev(hw);

	dev->new_ctx = ctx;
	return 0;
}

static void
mt7902_remove_chanctx(struct ieee80211_hw *hw,
		      struct ieee80211_chanctx_conf *ctx)
{
	printk(KERN_DEBUG "main.c - mt7902_remove_chanctx");
	struct mt792x_dev *dev = mt792x_hw_dev(hw);

	if (dev->new_ctx == ctx)
		dev->new_ctx = NULL;
}

static void
mt7902_change_chanctx(struct ieee80211_hw *hw,
		      struct ieee80211_chanctx_conf *ctx,
		      u32 changed)
{
	printk(KERN_DEBUG "main.c - mt7902_change_chanctx");
	struct mt792x_chanctx *mctx = (struct mt792x_chanctx *)ctx->drv_priv;
	struct mt792x_phy *phy = mt792x_hw_phy(hw);
	struct ieee80211_vif *vif;
	struct mt792x_vif *mvif;

	if (!mctx->bss_conf)
		return;

	mvif = container_of(mctx->bss_conf, struct mt792x_vif, bss_conf);
	vif = container_of((void *)mvif, struct ieee80211_vif, drv_priv);

	mt792x_mutex_acquire(phy->dev);
	if (vif->type == NL80211_IFTYPE_MONITOR)
		mt7902_mcu_config_sniffer(mvif, ctx);
	//else
	//	mt76_connac_mcu_uni_set_chctx(mvif->phy->mt76, &mvif->bss_conf.mt76, ctx);
	mt792x_mutex_release(phy->dev);
}

static void mt7902_mgd_prepare_tx(struct ieee80211_hw *hw,
				  struct ieee80211_vif *vif,
				  struct ieee80211_prep_tx_info *info)
{
	printk(KERN_DEBUG "main.c - mt7902_mgd_prepare_tx");
	struct mt792x_vif *mvif = (struct mt792x_vif *)vif->drv_priv;
	struct mt792x_dev *dev = mt792x_hw_dev(hw);
	u16 duration = info->duration ? info->duration :
		       jiffies_to_msecs(HZ);

	mt792x_mutex_acquire(dev);
	mt7902_set_roc(mvif->phy, mvif, mvif->bss_conf.mt76.ctx->def.chan, duration,
		       MT7902_ROC_REQ_JOIN);
	mt792x_mutex_release(dev);
}

static void mt7902_mgd_complete_tx(struct ieee80211_hw *hw,
				   struct ieee80211_vif *vif,
				   struct ieee80211_prep_tx_info *info)
{
	printk(KERN_DEBUG "main.c - mt7902_mgd_complete_tx");
	struct mt792x_vif *mvif = (struct mt792x_vif *)vif->drv_priv;

	mt7902_abort_roc(mvif->phy, mvif);
}

static int mt7902_switch_vif_chanctx(struct ieee80211_hw *hw,
				     struct ieee80211_vif_chanctx_switch *vifs,
				     int n_vifs,
				     enum ieee80211_chanctx_switch_mode mode)
{
	printk(KERN_DEBUG "main.c - mt7902_switch_vif_chanctx");
	return mt792x_assign_vif_chanctx(hw, vifs->vif, vifs->link_conf,
					 vifs->new_ctx);
}

void mt7902_csa_work(struct work_struct *work)
{
	printk(KERN_DEBUG "main.c - mt7902_csa_work");
	struct mt792x_vif *mvif;
	struct mt792x_dev *dev;
	struct ieee80211_vif *vif;
	int ret = 0;

	mvif = (struct mt792x_vif *)container_of(work, struct mt792x_vif,
						csa_work);
	dev = mvif->phy->dev;
	vif = container_of((void *)mvif, struct ieee80211_vif, drv_priv);

	mt792x_mutex_acquire(dev);
	//ret = mt76_connac_mcu_uni_set_chctx(mvif->phy->mt76, &mvif->bss_conf.mt76,
	//				    dev->new_ctx);
	mt792x_mutex_release(dev);

	ieee80211_chswitch_done(vif, !ret, 0);
}

static int mt7902_pre_channel_switch(struct ieee80211_hw *hw,
				     struct ieee80211_vif *vif,
				     struct ieee80211_channel_switch *chsw)
{
	printk(KERN_DEBUG "main.c - mt7902_pre_channel_switch");
	if (vif->type != NL80211_IFTYPE_STATION || !vif->cfg.assoc)
		return -EOPNOTSUPP;

	/* Avoid beacon loss due to the CAC(Channel Availability Check) time
	 * of the AP.
	 */
	if (!cfg80211_chandef_usable(hw->wiphy, &chsw->chandef,
				     IEEE80211_CHAN_RADAR))
		return -EOPNOTSUPP;

	return 0;
}

static void mt7902_channel_switch(struct ieee80211_hw *hw,
				  struct ieee80211_vif *vif,
				  struct ieee80211_channel_switch *chsw)
{
	printk(KERN_DEBUG "main.c - mt7902_channel_switch");
	struct mt792x_vif *mvif = (struct mt792x_vif *)vif->drv_priv;
	u16 beacon_interval = vif->bss_conf.beacon_int;

	mvif->csa_timer.expires = TU_TO_EXP_TIME(beacon_interval * chsw->count);
	add_timer(&mvif->csa_timer);
}

static void mt7902_abort_channel_switch(struct ieee80211_hw *hw,
					struct ieee80211_vif *vif,
					struct ieee80211_bss_conf *link_conf)
{
	printk(KERN_DEBUG "main.c - mt7902_abort_channel_switch");
	struct mt792x_vif *mvif = (struct mt792x_vif *)vif->drv_priv;

	timer_delete_sync(&mvif->csa_timer);
	cancel_work_sync(&mvif->csa_work);
}

static void mt7902_channel_switch_rx_beacon(struct ieee80211_hw *hw,
					    struct ieee80211_vif *vif,
					    struct ieee80211_channel_switch *chsw)
{
	printk(KERN_DEBUG "main.c - mt7902_channel_switch_rx_beacon");
	struct mt792x_dev *dev = mt792x_hw_dev(hw);
	struct mt792x_vif *mvif = (struct mt792x_vif *)vif->drv_priv;
	u16 beacon_interval = vif->bss_conf.beacon_int;

	if (cfg80211_chandef_identical(&chsw->chandef,
				       &dev->new_ctx->def) &&
				       chsw->count) {
		mod_timer(&mvif->csa_timer,
			  TU_TO_EXP_TIME(beacon_interval * chsw->count));
	}
}

static void mt7902_rfkill_poll(struct ieee80211_hw *hw)
{
	printk(KERN_DEBUG "main.c - mt7902_rfkill_poll");
	struct mt792x_phy *phy = mt792x_hw_phy(hw);
	int ret = 0;

	mt792x_mutex_acquire(phy->dev);
	ret = mt7902_mcu_wf_rf_pin_ctrl(phy, WF_RF_PIN_POLL);
	mt792x_mutex_release(phy->dev);

	wiphy_rfkill_set_hw_state(hw->wiphy, ret ? false : true);
}

const struct ieee80211_ops mt7902_ops = {
	.tx = mt792x_tx,
	.start = mt7902_start,
	.stop = mt7902_stop,
	.add_interface = mt7902_add_interface,
	.remove_interface = mt792x_remove_interface,
	.config = mt7902_config,
	.conf_tx = mt792x_conf_tx,
	.configure_filter = mt7902_configure_filter,
	.bss_info_changed = mt7902_bss_info_changed,
	.start_ap = mt7902_start_ap,
	.stop_ap = mt7902_stop_ap,
	.sta_state = mt7902_sta_state,
	.sta_pre_rcu_remove = mt76_sta_pre_rcu_remove,
	.set_key = mt7902_set_key,
	.sta_set_decap_offload = mt7902_sta_set_decap_offload,
#if IS_ENABLED(CONFIG_IPV6)
	//.ipv6_addr_change = mt7902_ipv6_addr_change,
#endif /* CONFIG_IPV6 */
	.ampdu_action = mt7902_ampdu_action,
	.set_rts_threshold = mt7902_set_rts_threshold,
	.wake_tx_queue = mt76_wake_tx_queue,
	.release_buffered_frames = mt76_release_buffered_frames,
	.channel_switch_beacon = mt7902_channel_switch_beacon,
	.get_txpower = mt76_get_txpower,
	.get_stats = mt792x_get_stats,
	.get_et_sset_count = mt792x_get_et_sset_count,
	.get_et_strings = mt792x_get_et_strings,
	.get_et_stats = mt792x_get_et_stats,
	.get_tsf = mt792x_get_tsf,
	.set_tsf = mt792x_set_tsf,
	.get_survey = mt76_get_survey,
	.get_antenna = mt76_get_antenna,
	.set_antenna = mt7902_set_antenna,
	.set_coverage_class = mt792x_set_coverage_class,
	.hw_scan = mt7902_hw_scan,
	.cancel_hw_scan = mt7902_cancel_hw_scan,
	.sta_statistics = mt792x_sta_statistics,
	.sched_scan_start = mt7902_start_sched_scan,
	.sched_scan_stop = mt7902_stop_sched_scan,
	CFG80211_TESTMODE_CMD(mt7902_testmode_cmd)
	CFG80211_TESTMODE_DUMP(mt7902_testmode_dump)
#ifdef CONFIG_PM
	//.suspend = mt7902_suspend,
	//.resume = mt7902_resume,
	//.set_wakeup = mt792x_set_wakeup,
	//.set_rekey_data = mt7902_set_rekey_data,
#endif /* CONFIG_PM */
	.flush = mt792x_flush,
	.set_sar_specs = mt7902_set_sar_specs,
	.rfkill_poll = mt7902_rfkill_poll,
	.remain_on_channel = mt7902_remain_on_channel,
	.cancel_remain_on_channel = mt7902_cancel_remain_on_channel,
	.add_chanctx = mt7902_add_chanctx,
	.remove_chanctx = mt7902_remove_chanctx,
	.change_chanctx = mt7902_change_chanctx,
	.assign_vif_chanctx = mt792x_assign_vif_chanctx,
	.unassign_vif_chanctx = mt792x_unassign_vif_chanctx,
	.mgd_prepare_tx = mt7902_mgd_prepare_tx,
	.mgd_complete_tx = mt7902_mgd_complete_tx,
	.switch_vif_chanctx = mt7902_switch_vif_chanctx,
	.pre_channel_switch = mt7902_pre_channel_switch,
	.channel_switch = mt7902_channel_switch,
	.abort_channel_switch = mt7902_abort_channel_switch,
	.channel_switch_rx_beacon = mt7902_channel_switch_rx_beacon,
};
EXPORT_SYMBOL_GPL(mt7902_ops);

MODULE_DESCRIPTION("MediaTek MT7902 core driver");
MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("Sean Wang <sean.wang@mediatek.com>");
