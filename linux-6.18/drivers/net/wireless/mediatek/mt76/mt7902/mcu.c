// SPDX-License-Identifier: ISC
/* Copyright (C) 2020 MediaTek Inc. */

#include <linux/fs.h>
#include <linux/firmware.h>
#include "mt7902.h"
#include "mcu.h"
#include "../mt76_connac2_mac.h"
#include "../mt792x_trace.h"

#define DEBUG


#define MT_STA_BFER			BIT(0)
#define MT_STA_BFEE			BIT(1)

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
	if (seq != rxd->seq) {
		dev_info(mdev->dev, "MCU seq mismatch: expected %d, got %d (cmd %08x)\n",
			seq, rxd->seq, cmd);
		return -EAGAIN;
	}

	if (cmd == MCU_CMD(PATCH_SEM_CONTROL) ||
	    cmd == MCU_CMD(PATCH_FINISH_REQ)) {
		skb_pull(skb, sizeof(*rxd) - 4);
		ret = *skb->data;
		dev_info(mdev->dev, "MCU Patch status: %d (cmd %08x)\n", ret, cmd);
	} else if (cmd == MCU_EXT_CMD(THERMAL_CTRL)) {
		skb_pull(skb, sizeof(*rxd) + 4);
		ret = le32_to_cpu(*(__le32 *)skb->data);
		dev_info(mdev->dev, "MCU Thermal info: %d\n", ret);
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
		if (ret)
			dev_warn(mdev->dev, "MCU UNI_CMD %08x failed with status %d\n", cmd, ret);

		/* skip invalid event */
		if (mcu_cmd != event->cid) {
			dev_info(mdev->dev, "MCU CID mismatch: cmd %x, event cid %x\n", mcu_cmd, event->cid);
			ret = -EAGAIN;
		}
	} else if (cmd == MCU_CE_QUERY(REG_READ)) {
		struct mt76_connac_mcu_reg_event *event;

		skb_pull(skb, sizeof(*rxd));
		event = (struct mt76_connac_mcu_reg_event *)skb->data;
		ret = (int)le32_to_cpu(event->val);
		dev_info(mdev->dev, "MCU Reg Read result: 0x%08x\n", ret);
	} else if (cmd == MCU_EXT_CMD(WF_RF_PIN_CTRL)) {
		struct mt7902_wf_rf_pin_ctrl_event *event;

		skb_pull(skb, sizeof(*rxd));
		event = (struct mt7902_wf_rf_pin_ctrl_event *)skb->data;
		ret = (int)event->result;
		dev_info(mdev->dev, "MCU RF Pin Ctrl result: %d\n", ret);
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

	dev_info(dev->mt76.dev, "Requesting eFuse read at offset 0x%04x (Block base: 0x%04x)\n", 
		offset, le32_to_cpu(req.addr));
	ret = mt76_mcu_send_and_get_msg(&dev->mt76, MCU_EXT_QUERY(EFUSE_ACCESS),
					&req, sizeof(req), true, &skb);
	if (ret) {
		dev_err(dev->mt76.dev, "Failed to read eFuse at offset 0x%x: %d\n", offset, ret);
		return ret;
	}

	// 2. Validate response pointer and length
	if (!skb || skb->len < sizeof(*res)) {
		dev_err(dev->mt76.dev, "Invalid eFuse response for offset 0x%x\n", offset);
		dev_kfree_skb(skb);
		return -EINVAL;
	}

	res = (struct mt7902_mcu_eeprom_info *)skb->data;
	*val = res->data[offset % MT7902_EEPROM_BLOCK_SIZE];

	dev_info(dev->mt76.dev, "eFuse[0x%x] = 0x%02x\n", offset, *val);
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

	/* Check if the buffer is large enough before pulling */
	if (skb->len < sizeof(struct mt76_connac2_mcu_rxd) + sizeof(*event)) {
		dev_warn_ratelimited(dev->mt76.dev, "MCU: Malformed LP event (len %d)\n", skb->len);
		return;
	}

	skb_pull(skb, sizeof(struct mt76_connac2_mcu_rxd));
	event = (struct mt7902_mcu_lp_event *)skb->data;

	/* 1. Log the state change */
	/* We use dev_dbg here to avoid spamming dmesg during frequent sleep/wake cycles */
	dev_info(dev->mt76.dev, "MCU LP Event: Chip reported state 0x%02x\n", event->state);

	/* 2. Log specific transitions for easier grepping */
	if (event->state == 0) {
		dev_info(dev->mt76.dev, "MCU LP: Chip is AWAKE / ACTIVE\n");
	} else {
		dev_info(dev->mt76.dev, "MCU LP: Chip is entering SLEEP / LOW POWER (state %d)\n", event->state);
	}

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
	if (mt7902_disable_clc || mt76_is_usb(&dev->mt76)) {
		dev_info(mdev->dev, "CLC loading skipped (disabled or USB device)\n");
		return 0;
	}

	if (mt76_is_mmio(&dev->mt76)) {
		ret = mt7902_mcu_read_eeprom(dev, MT_EE_HW_TYPE, &hw_encap);
		if (ret) {
			dev_err(mdev->dev, "Failed to read EEPROM for HW_TYPE: %d\n", ret);
			return ret;
		}
		hw_encap = u8_get_bits(hw_encap, MT_EE_HW_TYPE_ENCAP);
		dev_info(mdev->dev, "Hardware Encap type: %d\n", hw_encap);
	}

	dev_info(mdev->dev, "Requesting firmware for CLC parsing: %s\n", fw_name);
	ret = request_firmware(&fw, fw_name, mdev->dev);
	if (ret) {
		dev_err(mdev->dev, "Failed to request firmware %s: %d\n", fw_name, ret);
		return ret;
	}

	if (!fw || !fw->data || fw->size < sizeof(*hdr)) {
		//dev_err(mdev->dev, "Invalid firmware\n");
		dev_err(mdev->dev, "Invalid firmware binary size: %zu\n", fw ? fw->size : 0);
		ret = -EINVAL;
		goto out;
	}

	hdr = (const void *)(fw->data + fw->size - sizeof(*hdr));
	dev_info(mdev->dev, "FW Trailer found: %d regions, version - hdr->ver\n", hdr->n_region);
	for (i = 0; i < hdr->n_region; i++) {
		region = (const void *)((const u8 *)hdr -
					(hdr->n_region - i) * sizeof(*region));
		len = le32_to_cpu(region->len);

		/* check if we have valid buffer size */
		if (offset + len > fw->size) {
			//dev_err(mdev->dev, "Invalid firmware region\n");
			dev_err(mdev->dev, "Region %d exceeds firmware size!\n", i);
			ret = -EINVAL;
			goto out;
		}

		if ((region->feature_set & FW_FEATURE_NON_DL) &&
		    region->type == FW_TYPE_CLC) {
			clc_base = (u8 *)(fw->data + offset);
			dev_info(mdev->dev, "CLC region found at offset 0x%x (len %d)\n", offset, len);
			break;
		}
		offset += len;
	}

	if (!clc_base) {
		dev_warn(mdev->dev, "No CLC region found in firmware binary\n");
		goto out;
	}

	for (offset = 0; offset < len; offset += le32_to_cpu(clc->len)) {
		clc = (const struct mt7902_clc *)(clc_base + offset);

		/* do not init buf again if chip reset triggered */
		if (phy->clc[clc->idx]) {
			dev_info(mdev->dev, "CLC idx %d already initialized, skipping\n", clc->idx);
			continue;
		}

		/* header content sanity */
		if (clc->idx == MT7902_CLC_POWER &&
		    u8_get_bits(clc->type, MT_EE_HW_TYPE_ENCAP) != hw_encap) {
		    dev_info(mdev->dev, "CLC Power Encap mismatch (clc:%d vs hw:%d), skipping idx %d\n", 
				u8_get_bits(clc->type, MT_EE_HW_TYPE_ENCAP), hw_encap, clc->idx);
			
			continue;
		}

		dev_info(mdev->dev, "Duplicating CLC element: idx %d, len %d\n", clc->idx, le32_to_cpu(clc->len));
		phy->clc[clc->idx] = devm_kmemdup(mdev->dev, clc,
						  le32_to_cpu(clc->len),
						  GFP_KERNEL);

		if (!phy->clc[clc->idx]) {
			dev_err(mdev->dev, "Failed to allocate memory for CLC idx %d\n", clc->idx);
			ret = -ENOMEM;
			goto out;
		}
	}
	dev_info(mdev->dev, "Sending CLC configuration to MCU (Indoor mode)\n");
	ret = mt7902_mcu_set_clc(dev, "00", ENVIRON_INDOOR);
	if (ret)
		dev_err(mdev->dev, "mt7902_mcu_set_clc failed: %d\n", ret);

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
	struct mt7902_phy_cap {
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

	cap = (struct mt7902_phy_cap *)skb->data;

	// 1. Log the raw values for fundamental capabilities
	dev_info(dev->dev, "PHY Cap: NSS=%d, MaxBW=%d, HW_Path=0x%x\n", 
		cap->nss, cap->max_bw, cap->hw_path);

	// 2. Log Protocol support (HT/VHT/HE)
	dev_info(dev->dev, "Protocols: HT:%d VHT:%d HE:%d\n", 
		cap->ht, cap->vht, cap->he);

	dev->phy.antenna_mask = BIT(cap->nss) - 1;
	dev->phy.chainmask = dev->phy.antenna_mask;

	// 3. Log calculated masks
	dev_info(dev->dev, "Antenna Mask: 0x%x, Chainmask: 0x%x\n", 
		dev->phy.antenna_mask, dev->phy.chainmask);

	dev->phy.cap.has_2ghz = cap->hw_path & BIT(WF0_24G);
	dev->phy.cap.has_5ghz = cap->hw_path & BIT(WF0_5G);

	// 4. Log final band availability
	dev_info(dev->dev, "Bands Detected: 2.4GHz: %s, 5GHz: %s\n",
		 dev->phy.cap.has_2ghz ? "Yes" : "No",
		 dev->phy.cap.has_5ghz ? "Yes" : "No");

	// 5. Check for 160MHz support (MaxBW usually: 2=80MHz, 3=160MHz)
	if (cap->max_bw >= 3)
		dev_info(dev->dev, "160MHz bandwidth support enabled\n");

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

	dev_info(phy->dev->dev, "Querying NIC capabilities from MCU\n");
	ret = mt76_mcu_send_and_get_msg(phy->dev, MCU_CE_CMD(GET_NIC_CAPAB),
					NULL, 0, true, &skb);
	if (ret) {
		dev_err(phy->dev->dev, "Failed to get NIC capability msg: %d\n", ret);
		return ret;
	}

	hdr = (struct mt76_connac_cap_hdr *)skb->data;
	if (skb->len < sizeof(*hdr)) {
		dev_err(phy->dev->dev, "NIC cap response too short: %d\n", skb->len);
		ret = -EINVAL;
		goto out;
	}

	dev_info(phy->dev->dev, "MCU reported %d capability elements\n", le16_to_cpu(hdr->n_element));
	skb_pull(skb, sizeof(*hdr));

	for (i = 0; i < le16_to_cpu(hdr->n_element); i++) {
		struct tlv_hdr {
			__le32 type;
			__le32 len;
		} __packed * tlv = (struct tlv_hdr *)skb->data;
		int len;
		u32 type;

		if (skb->len < sizeof(*tlv))
			break;

		skb_pull(skb, sizeof(*tlv));

		len = le32_to_cpu(tlv->len);
		type = le32_to_cpu(tlv->type);
		if (skb->len < len) {
			dev_warn(phy->dev->dev, "TLV type %d has invalid length %d\n", type, len);
			break;
		}

		switch (le32_to_cpu(tlv->type)) {
		case MT_NIC_CAP_6G:
			phy->cap.has_6ghz = skb->data[0];
			dev_info(phy->dev->dev, "Hardware 6GHz support: %s\n", 
				 phy->cap.has_6ghz ? "Yes" : "No");
			break;
		case MT_NIC_CAP_MAC_ADDR:
			memcpy(phy->macaddr, (void *)skb->data, ETH_ALEN);
			dev_info(phy->dev->dev, "Firmware MAC address: %pM\n", phy->macaddr);
			break;
		case MT_NIC_CAP_PHY:
			dev_info(phy->dev->dev, "Parsing PHY capabilities\n");
			mt7902_mcu_parse_phy_cap(phy->dev, skb);
			break;
		case MT_NIC_CAP_TX_RESOURCE:
			dev_info(phy->dev->dev, "Parsing TX resources\n");
			if (mt76_is_sdio(phy->dev))
				mt7902_mcu_parse_tx_resource(phy->dev,
							     skb);
			break;
		case MT_NIC_CAP_CHIP_CAP:
			memcpy(&mphy->chip_cap, (void *)skb->data, sizeof(u64));
			dev_info(phy->dev->dev, "Chip Capability Mask: 0x%llx\n", mphy->chip_cap);
			break;
		default:
			dev_info(phy->dev->dev, "Unknown TLV type: 0x%x\n", type);
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
	int ret;

	dev_info(dev->mt76.dev, "%s MCU-to-Host logging (ctrl: 0x%02x)\n", 
		ctrl ? "Enabling" : "Disabling", ctrl);
	ret = mt76_mcu_send_msg(&dev->mt76, MCU_CE_CMD(FWLOG_2_HOST),
				 &data, sizeof(data), false);

	// 2. Log transmission status
	if (ret) {
		dev_err(dev->mt76.dev, "Failed to send FWLOG_2_HOST command: %d\n", ret);
	} else {
		dev_info(dev->mt76.dev, "FWLOG_2_HOST command sent to queue\n");
	}

	return ret;
	//return mt76_mcu_send_msg(&dev->mt76, MCU_CE_CMD(FWLOG_2_HOST),
	//			 &data, sizeof(data), false);
}

int mt7902_firmware_state(struct mt792x_dev *dev, bool wa)
{
	printk(KERN_DEBUG "mcu.c - mt7902_firmware_state(dev, wa: %d)", wa);
	u32 state = FIELD_PREP(MT_TOP_MISC_FW_STATE,
			       wa ? FW_STATE_RDY : FW_STATE_FW_DOWNLOAD);
	u32 val;

	dev_info(dev->mt76.dev, "Waiting for firmware state: 0x%08x (wa: %d)\n", state, wa);
	if (!mt76_poll_msec(dev, MT_TOP_MISC, MT_TOP_MISC_FW_STATE,
			    state, 1000)) {
		//dev_err(dev->mt76.dev, "Timeout for initializing firmware\n");
		val = mt76_rr(dev, MT_TOP_MISC);
		dev_err(dev->mt76.dev, "Firmware state timeout! Register 0x%x = 0x%08x (Expected bits: 0x%08x)\n", 
			MT_TOP_MISC, val, state);

		// 3. Check for common 'dead bus' indicator
		if (val == 0xffffffff)
			dev_err(dev->mt76.dev, "PCIe link lost during firmware state check\n");
			
		return -EIO;
	}
	return 0;
}

int mt7902_load_firmware(struct mt792x_dev *dev)
{
	printk(KERN_DEBUG "mcu.c - mt7902_load_firmware(dev)");
	int ret;
    
	// 1. Check current state - helps identify if the chip was already running
	dev_info(dev->mt76.dev, "Checking firmware state before download\n");
	
    /* make sure fw is download state */
	if (mt7902_firmware_state(dev, false)) {
		/* restart firmware once */
		dev_warn(dev->mt76.dev, "Firmware already running, attempting restart...\n");
		mt76_connac_mcu_restart(&dev->mt76);
		ret = mt7902_firmware_state(dev, false);
		if (ret) {
			//dev_err(dev->mt76.dev,
			//	"Firmware is not ready for download\n");
			dev_err(dev->mt76.dev, "Firmware is not ready for download (state: %d)\n", ret);
			return ret;
		}
	}

	// 2. Load the ROM Patch
	dev_info(dev->mt76.dev, "Loading patch: %s\n", mt792x_patch_name(dev));
	ret = mt76_connac2_load_patch(&dev->mt76, mt792x_patch_name(dev));
	if (ret) {
		dev_err(dev->mt76.dev, "Failed to load patch (%s): %d\n", mt792x_patch_name(dev), ret);
		return ret;
	}

	if (mt76_is_sdio(&dev->mt76)) {
		dev_info(dev->mt76.dev, "Performing SDIO PM control handshake\n");
		/* activate again */
		ret = __mt792x_mcu_fw_pmctrl(dev);
		if (!ret)
			ret = __mt792x_mcu_drv_pmctrl(dev);
	}

	dev_info(dev->mt76.dev, "Loading RAM: %s\n", mt792x_ram_name(dev));
	ret = mt76_connac2_load_ram(&dev->mt76, mt792x_ram_name(dev), NULL);
	if (ret) {
		dev_err(dev->mt76.dev, "Failed to load RAM (%s): %d\n", mt792x_ram_name(dev), ret);
		return ret;
	}

	dev_info(dev->mt76.dev, "Waiting for N9 Firmware Ready signal...\n");
	if (!mt76_poll_msec(dev, MT_CONN_ON_MISC, MT_TOP_MISC2_FW_N9_RDY,
			    MT_TOP_MISC2_FW_N9_RDY, 1500)) {
		//dev_err(dev->mt76.dev, "Timeout for initializing firmware\n");
		u32 status = mt76_rr(dev, MT_CONN_ON_MISC);
		dev_err(dev->mt76.dev, "Timeout for initializing firmware! Register 0x%x = 0x%08x\n", 
			MT_CONN_ON_MISC, status);

		return -EIO;
	}

#ifdef CONFIG_PM2
	dev->mt76.hw->wiphy->wowlan = &mt76_connac_wowlan_support;
#endif /* CONFIG_PM */

	dev_info(dev->mt76.dev, "Firmware init done\n");

	return 0;
}

int mt7902_run_firmware(struct mt792x_dev *dev)
{
	printk(KERN_DEBUG "mcu.c - mt7902_run_firmware(dev)");
	int err;

	// 1. Monitor the heavy lifting: Loading the binary into chip RAM
	dev_info(dev->mt76.dev, "Loading firmware binaries into RAM...\n");
	err = mt7902_load_firmware(dev);
	if (err) {
		dev_err(dev->mt76.dev, "mt7902_load_firmware failed: %d\n", err);
		return err;
	}

	// 2. Capabilities check: Ensures the loaded FW supports this hardware revision
	dev_info(dev->mt76.dev, "Requesting NIC capability from MCU\n");
	err = mt7902_mcu_get_nic_capability(&dev->phy);
	if (err) {
		dev_err(dev->mt76.dev, "MCU failed to report capabilities: %d\n", err);
		return err;
	}

	set_bit(MT76_STATE_MCU_RUNNING, &dev->mphy.state);
	dev_info(dev->mt76.dev, "MT76_STATE_MCU_RUNNING bit set\n");

	// 3. CLC (Calibration Log/Control) loading
	dev_info(dev->mt76.dev, "Loading CLC: %s\n", mt792x_ram_name(dev));
	err = mt7902_load_clc(dev, mt792x_ram_name(dev));
	if (err) {
		dev_err(dev->mt76.dev, "CLC load failed: %d\n", err);
		return err;
	}

	// 4. Final step: Enabling the MCU to send its own debug logs to the host dmesg
	dev_info(dev->mt76.dev, "Enabling MCU firmware-to-host logging\n");
	err = mt7902_mcu_fw_log_2_host(dev, 1);
	if (err)
		dev_warn(dev->mt76.dev, "Failed to enable MCU host logging: %d\n", err);

	dev_info(dev->mt76.dev, "MT7902 firmware running and initialized\n");
	return 0;

	//return mt7902_mcu_fw_log_2_host(dev, 1);
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

	int err;

	/* 1. Log the LED action being requested */
	dev_info(dev->mt76.dev, "LED: Sending MCU Radio Ctrl (ID: 0x%02x)\n", value);

	/* 
	 * Note: 'false' means asynchronous. We only know if the message 
	 * was *sent* to the queue, not if the firmware actually processed it.
	 */
	err = mt76_mcu_send_msg(&dev->mt76, MCU_EXT_CMD(ID_RADIO_ON_OFF_CTRL),
				&req, sizeof(req), false);

	if (err) {
		dev_warn(dev->mt76.dev, "LED: Failed to queue Radio Ctrl 0x%02x (err: %d)\n", 
			 value, err);
	}

	return err;

	//return mt76_mcu_send_msg(&dev->mt76, MCU_EXT_CMD(ID_RADIO_ON_OFF_CTRL),
	//			&req, sizeof(req), false);
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

	dev_info(dev->mt76.dev, "TX_CONF: Updating EDCA. VIF_idx=%d WMM_idx=%d QoS=%d HE=%d\n",
		mvif->bss_conf.mt76.idx, mvif->bss_conf.mt76.wmm_idx, 
		vif->bss_conf.qos, vif->bss_conf.he_support);

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

		dev_info(dev->mt76.dev, "TX_CONF: AC%d -> AIFS=%d TXOP=%d CWmin/max=%d/%d\n",
			ac, q->aifs, q->txop, le16_to_cpu(e->cw_min), le16_to_cpu(e->cw_max));
	}

	ret = mt76_mcu_send_msg(&dev->mt76, MCU_CE_CMD(SET_EDCA_PARMS), &req,
				sizeof(req), false);
	if (ret) {
		dev_err(dev->mt76.dev, "TX_CONF: Failed to send EDCA params (%d)\n", ret);
		return ret;
	}

	if (!vif->bss_conf.he_support)
		return 0;

	// ret = mt76_mcu_send_msg(&dev->mt76, MCU_CE_CMD(SET_EDCA_PARMS), &req,
	// 			sizeof(req), false);
	// if (ret)
	// 	return ret;

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
		dev_info(dev->mt76.dev, "TX_CONF: HE-MU AC%d -> AIFSN=%d CWmin/max=%d/%d Timer=%d\n",
			ac, e->aifsn, e->cw_min, e->cw_max, e->timer);
	}

	ret = mt76_mcu_send_msg(&dev->mt76, MCU_CE_CMD(SET_MU_EDCA_PARMS),
				 &req_mu, sizeof(req_mu), false);
	if (ret)
		dev_err(dev->mt76.dev, "TX_CONF: Failed to send MU-EDCA params (%d)\n", ret);

	return ret;

	// return mt76_mcu_send_msg(&dev->mt76, MCU_CE_CMD(SET_MU_EDCA_PARMS),
	// 			 &req_mu, sizeof(req_mu), false);
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
	int err;
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

	/* --- DEBUG LOGS START --- */
	dev_info(dev->mt76.dev, 
		"MCU ChanInfo: Cmd=%d Freq=%dMHz(Ch:%d) BW=%d Band=%d Reason=%d\n",
		cmd, freq1, req.center_ch, req.bw, req.channel_band, req.switch_reason);

	dev_info(dev->mt76.dev, 
		"MCU ChanInfo: Streams(TX:%d RX_mask:0x%x) BandIdx=%d\n",
		req.tx_streams_num, req.rx_streams, req.band_idx);
	/* --- DEBUG LOGS END --- */

	/* 1. Log the High-Level intent */
	dev_info(dev->mt76.dev, "CHAN_INFO: [Cmd: %s (%d)] Freq: %d MHz, BW: %d, Band: %d\n",
		(cmd == MCU_EXT_CMD(CHANNEL_SWITCH)) ? "SWITCH" : "SET_RX", 
		cmd, freq1, req.bw, req.channel_band);

	/* 2. Log the specific Hardware/Firmware mapping */
	// This helps catch issues where the 'control channel' doesn't match the 'center channel' (common in 80/160MHz)
	dev_info(dev->mt76.dev, "CHAN_INFO: HW_Control_Ch: %d, HW_Center_Ch: %d, Reason: %d\n",
		req.control_ch, req.center_ch, req.switch_reason);

	/* 3. Log Antenna/Stream state */
	// If RX mask is 0 here, the radio will be deaf.
	dev_info(dev->mt76.dev, "CHAN_INFO: Antenna Mask -> TX_Streams: %d, RX_Streams/Mask: 0x%x\n",
		req.tx_streams_num, req.rx_streams);

	/* 4. Log 80+80 or 160MHz specific info */
	if (chandef->width == NL80211_CHAN_WIDTH_80P80) {
		dev_info(dev->mt76.dev, "CHAN_INFO: 80+80 Mode Active! Center_Ch2: %d\n", 
			req.center_ch2);
	}

	err = mt76_mcu_send_msg(&dev->mt76, cmd, &req, sizeof(req), true);
	
	if (err)
		dev_err(dev->mt76.dev, "MCU ChanInfo: Failed to send command %d (err=%d)\n", cmd, err);
	
	return err;


	//return mt76_mcu_send_msg(&dev->mt76, cmd, &req, sizeof(req), true);
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
	int ret;

	// 1. Log the intent and mode
	dev_info(dev->mt76.dev, "Setting MCU eFuse mode: %d, format: %d\n", 
		req.buffer_mode, req.format);

	ret = mt76_mcu_send_msg(&dev->mt76, MCU_EXT_CMD(EFUSE_BUFFER_MODE),
				 &req, sizeof(req), true);

	// 2. Capture the result of the handshake
	if (ret) {
		dev_err(dev->mt76.dev, "Failed to set MCU eFuse mode: %d\n", ret);
	} else {
		dev_info(dev->mt76.dev, "MCU eFuse mode set successfully\n");
	}

	return ret;

	//return mt76_mcu_send_msg(&dev->mt76, MCU_EXT_CMD(EFUSE_BUFFER_MODE),
	//			 &req, sizeof(req), true);
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

	dev_info(dev->mt76.dev, "Searching CLC rules for %c%c in idx %d (Type: %d)\n", 
		alpha2[0], alpha2[1], idx, dev->phy.power_type);

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

		dev_info(dev->mt76.dev, "Found matching CLC rule: %c%c Type: %c%c Len: %d\n",
			rule->alpha2[0], rule->alpha2[1], rule->type[0], rule->type[1], len);


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
		if (ret < 0) {
			dev_err(dev->mt76.dev, "MCU rejected CLC rule %c%c: %d\n", 
				alpha2[0], alpha2[1], ret);
			return ret;
		}

		if (ret_skb) {
			struct mt7902_clc_info_tlv *info;

			info = (struct mt7902_clc_info_tlv *)(ret_skb->data + 4);
			dev->phy.clc_chan_conf = info->chan_conf;
			dev_info(dev->mt76.dev, "MCU CLC feedback: chan_conf=0x%x\n", info->chan_conf);
			dev_kfree_skb(ret_skb);
		}

		valid_cnt++;
	}

	if (!valid_cnt) {
		dev_info(dev->mt76.dev, "No valid rules found for country %c%c\n", 
			alpha2[0], alpha2[1]);
		return -ENOENT;
	}

	return 0;
}

int mt7902_mcu_set_clc(struct mt792x_dev *dev, u8 *alpha2,
		       enum environment_cap env_cap)
{
	printk(KERN_DEBUG "mcu.c - mt7902_mcu_set_clc(dev, alpha2: 0x%02x, env_cap)", alpha2);
	struct mt792x_phy *phy = (struct mt792x_phy *)&dev->phy;
	int i, ret;

	dev_info(dev->mt76.dev, "Pushing CLC tables for country %c%c (Env: %d)\n", 
		alpha2[0], alpha2[1], env_cap);

	/* submit all clc config */
	for (i = 0; i < ARRAY_SIZE(phy->clc); i++) {
		// Skip empty tables to avoid unnecessary MCU traffic
		if (!phy->clc[i])
			continue;

		ret = __mt7902_mcu_set_clc(dev, alpha2, env_cap,
					   phy->clc[i], i);

		/* If no country found, set "00" as default */
		if (ret == -ENOENT) {
			dev_warn(dev->mt76.dev, 
				 "Country %c%c not found in CLC idx %d. Falling back to '00'\n", 
				 alpha2[0], alpha2[1], i);
			ret = __mt7902_mcu_set_clc(dev, "00",
						   ENVIRON_INDOOR,
						   phy->clc[i], i);
		}
		if (ret < 0) {
			dev_err(dev->mt76.dev, "Failed to set CLC idx %d: %d\n", i, ret);
			return ret;
		}
		dev_info(dev->mt76.dev, "CLC table idx %d updated successfully\n", i);
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

	int ret;

	/* 1. Log the parsed intent */
	dev_info(dev->mt76.dev, 
		"RX_FILTER: Mode=%s(0x%x) FIF_Mask=0x%08x Bit_Op=0x%x Bit_Map=0x%08x\n",
		(data.mode == 1) ? "SET" : "CLEAR", data.mode, 
		fif, bit_op, bit_map);
	/* 2. Detailed bit breakdown for common filters */
	if (fif & BIT(5)) dev_info(dev->mt76.dev, "RX_FILTER: [Control Frames Enabled]\n");
	if (fif & BIT(6)) dev_info(dev->mt76.dev, "RX_FILTER: [Other BSS (Promiscuous) Enabled]\n");
	if (fif & BIT(2)) dev_info(dev->mt76.dev, "RX_FILTER: [FCS Failures Enabled]\n");

	/* 
	 * Note: This command is asynchronous (wait = false). 
	 * We only know if it was sent, not if the firmware successfully changed the filter.
	 */
	ret = mt76_mcu_send_msg(&dev->mt76, MCU_CE_CMD(SET_RX_FILTER),
				 &data, sizeof(data), false);

	if (ret) {
		dev_err(dev->mt76.dev, "RX_FILTER: Failed to queue MCU command (err=%d)\n", ret);
	}

	return ret;

	// return mt76_mcu_send_msg(&dev->mt76, MCU_CE_CMD(SET_RX_FILTER),
	// 			 &data, sizeof(data), false);
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
}

int mt7902_mcu_add_dev_info(struct mt76_phy *phy,
			    struct ieee80211_bss_conf *bss_conf,
                struct mt76_vif_link *mvif, bool enable)
{
	printk(KERN_DEBUG "mcu.c - mt7902_mcu_add_dev_info(phy, bss_conf, mvif, enable: %d)", enable);
	struct mt76_dev *dev = phy->dev;
	int err;
	struct {
		struct req_hdr { 
			u8 omac_idx;
			u8 band_idx;
			__le16 tlv_num;
			u8 is_tlv_append;
			u8 rsv[3];
		} __packed hdr;
		struct req_tlv {
			__le16 tag;
			__le16 len;
			u8 active;
			u8 band_idx;
			u8 omac_addr[ETH_ALEN];
		} __packed tlv;
	} data = {
		.hdr = {
			.omac_idx = mvif->omac_idx,
			.band_idx = mvif->band_idx,
			.tlv_num = cpu_to_le16(1),
			.is_tlv_append = 1,
		},
		.tlv = {
			.tag = cpu_to_le16(DEV_INFO_ACTIVE),
			.len = cpu_to_le16(sizeof(struct req_tlv)),
			.active = enable,
			.band_idx = mvif->band_idx,
		},
	};
	
    memcpy(data.tlv.omac_addr, bss_conf->addr, ETH_ALEN);

    /* 1. Log the Registration Details */
	dev_info(dev->dev, "MCU DevInfo: %s OMAC_idx=%d Band_idx=%d Addr=%pM\n",
		enable ? "Enabling" : "Disabling",
		data.hdr.omac_idx, data.hdr.band_idx, data.tlv.omac_addr);

	/* 2. Send and track return value */
	err = mt76_mcu_send_msg(dev, MCU_EXT_CMD(DEV_INFO_UPDATE),
				 &data, sizeof(data), true);

	if (err) {
		dev_err(dev->dev, "MCU DevInfo: Update failed for %pM (err=%d)\n", 
			bss_conf->addr, err);
	} else {
		dev_info(dev->dev, "MCU DevInfo: Successfully %s %pM\n",
			enable ? "activated" : "deactivated", bss_conf->addr);
	}

	return err;

	//return mt76_mcu_send_msg(dev, MCU_EXT_CMD(DEV_INFO_UPDATE),
	//			 &data, sizeof(data), true);
}

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
}

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

	skb = __mt76_connac_mcu_alloc_sta_req(&dev->mt76, &mvif->bss_conf.mt76, NULL,
					      MT7902_BSS_UPDATE_MAX_SIZE);
	if (IS_ERR(skb))
		return PTR_ERR(skb);

	/* bss_omac must be first */
	if (enable)
		mt76_connac_mcu_bss_omac_tlv(skb, vif);

	mt76_connac_mcu_bss_basic_tlv(skb, vif, NULL, phy->mt76,
				      mvif->sta.deflink.wcid.idx, enable);

	if (vif->type == NL80211_IFTYPE_MONITOR)
		goto out;

	if (enable) {
		mt7902_mcu_bss_rfch_tlv(skb, vif, phy);
		mt7902_mcu_bss_bmc_tlv(skb, phy);
		mt7902_mcu_bss_ra_tlv(skb, vif, phy);
		mt7902_mcu_bss_hw_amsdu_tlv(skb);

		if (vif->bss_conf.he_support)
			mt7902_mcu_bss_he_tlv(skb, vif, phy);

		/* all mt7902 ic need this tlv, no matter it supports EHT or not */
		// mt7902_mcu_bss_mld_tlv(skb);
	}
out:
	return mt76_mcu_skb_send_msg(&dev->mt76, skb,
				     MCU_EXT_CMD(BSS_INFO_UPDATE), true);
}

