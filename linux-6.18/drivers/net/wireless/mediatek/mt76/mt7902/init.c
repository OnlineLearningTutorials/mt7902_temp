// SPDX-License-Identifier: ISC
/* Copyright (C) 2020 MediaTek Inc. */

#include <linux/etherdevice.h>
#include <linux/hwmon.h>
#include <linux/hwmon-sysfs.h>
#include <linux/thermal.h>
#include <linux/firmware.h>
#include "mt7902.h"
#include "../mt76_connac2_mac.h"
#include "mcu.h"

#define DEBUG


static ssize_t mt7902_thermal_temp_show(struct device *dev,
					struct device_attribute *attr,
					char *buf)
{
	printk(KERN_DEBUG "init.c - mt7902_thermal_temp_show(dev, attr, buf)");
	switch (to_sensor_dev_attr(attr)->index) {
	case 0: {
		struct mt792x_phy *phy = dev_get_drvdata(dev);
		struct mt792x_dev *mdev = phy->dev;
		int temperature;

		mt792x_mutex_acquire(mdev);
		temperature = mt7902_mcu_get_temperature(phy);
		mt792x_mutex_release(mdev);

		if (temperature < 0)
			return temperature;
		/* display in millidegree Celsius */
		printk(KERN_DEBUG "init.c - mt7902_thermal_temp_show(dev, attr, buf) - temp: %u", temperature);

		return sprintf(buf, "%u\n", temperature * 1000);
	}
	default:
		return -EINVAL;
	}
}
static SENSOR_DEVICE_ATTR_RO(temp1_input, mt7902_thermal_temp, 0);

static struct attribute *mt7902_hwmon_attrs[] = {
	&sensor_dev_attr_temp1_input.dev_attr.attr,
	NULL,
};
ATTRIBUTE_GROUPS(mt7902_hwmon);

static int mt7902_thermal_init(struct mt792x_phy *phy)
{
	printk(KERN_DEBUG "init.c - mt7902_thermal_init(phy)");
	struct wiphy *wiphy = phy->mt76->hw->wiphy;
	struct device *hwmon;
	const char *name;

	dev_info(phy->mt76->dev->dev, "Initializing thermal sensor (HWMON)\n");
	if (!IS_REACHABLE(CONFIG_HWMON)) {
		dev_warn(phy->mt76->dev->dev, "HWMON support not reachable in kernel config; temperature monitoring disabled\n");
		return 0;
	}

	name = devm_kasprintf(&wiphy->dev, GFP_KERNEL, "mt7902_%s",
			      wiphy_name(wiphy));
	if (!name) {
		dev_err(phy->mt76->dev->dev, "Failed to allocate memory for thermal device name\n");
		return -ENOMEM;
	}
	else 
		printk(KERN_DEBUG "init.c - mt7902_thermal_init(phy) - %s", name);

	dev_info(phy->mt76->dev->dev, "Registering thermal device as: %s\n", name);
	hwmon = devm_hwmon_device_register_with_groups(&wiphy->dev, name, phy,
						       mt7902_hwmon_groups);
	if (IS_ERR(hwmon)) {
		dev_err(phy->mt76->dev->dev, "Failed to register HWMON device: %ld\n", PTR_ERR(hwmon));
		return PTR_ERR(hwmon);
	}

	dev_info(phy->mt76->dev->dev, "Thermal sensor registered successfully at /sys/class/hwmon/\n");
	return 0;
	//return PTR_ERR_OR_ZERO(hwmon);
}

static void
mt7902_regd_channel_update(struct wiphy *wiphy, struct mt792x_dev *dev)
{
	printk(KERN_DEBUG "init.c - mt7902_regd_channel_update(wiphy, dev)");
#define IS_UNII_INVALID(idx, sfreq, efreq) \
	(!(dev->phy.clc_chan_conf & BIT(idx)) && (cfreq) >= (sfreq) && (cfreq) <= (efreq))
	struct ieee80211_supported_band *sband;
	struct mt76_dev *mdev = &dev->mt76;
	struct device_node *np, *band_np;
	struct ieee80211_channel *ch;
	int i, cfreq;


	dev_info(mdev->dev, "Starting channel update. Firmware clc_chan_conf mask: 0x%02x\n", 
		dev->phy.clc_chan_conf);
	np = mt76_find_power_limits_node(mdev);
	if (np)
		dev_info(mdev->dev, "Found DTS power limits node\n");


	sband = wiphy->bands[NL80211_BAND_5GHZ];
	band_np = np ? of_get_child_by_name(np, "txpower-5g") : NULL;
	for (i = 0; i < sband->n_channels; i++) {
		ch = &sband->channels[i];
		cfreq = ch->center_freq;

		if (np && (!band_np || !mt76_find_channel_node(band_np, ch))) {
			dev_info(mdev->dev, "Disabling 5G ch %d (%d MHz): Not in DTS\n", ch->hw_value, cfreq);
			ch->flags |= IEEE80211_CHAN_DISABLED;
			continue;
		}

		/* UNII-4 */
		if (IS_UNII_INVALID(0, 5845, 5925)) {
			dev_info(mdev->dev, "Disabling 5G ch %d (%d MHz): UNII-4 mask bit 0 is clear\n", ch->hw_value, cfreq);
			ch->flags |= IEEE80211_CHAN_DISABLED;
		}
	}

	sband = wiphy->bands[NL80211_BAND_6GHZ];
	if (!sband)
		return;

	band_np = np ? of_get_child_by_name(np, "txpower-6g") : NULL;
	for (i = 0; i < sband->n_channels; i++) {
		ch = &sband->channels[i];
		cfreq = ch->center_freq;

		if (np && (!band_np || !mt76_find_channel_node(band_np, ch))) {
			dev_info(mdev->dev, "Disabling 6G ch %d (%d MHz): Not in DTS\n", ch->hw_value, cfreq);
			ch->flags |= IEEE80211_CHAN_DISABLED;
			continue;
		}

		/* UNII-5/6/7/8 */
		if (IS_UNII_INVALID(1, 5925, 6425) ||
		    IS_UNII_INVALID(2, 6425, 6525) ||
		    IS_UNII_INVALID(3, 6525, 6875) ||
		    IS_UNII_INVALID(4, 6875, 7125)) {
			dev_info(mdev->dev, "Disabling 6G ch %d (%d MHz): UNII mask restriction\n", ch->hw_value, cfreq);
			ch->flags |= IEEE80211_CHAN_DISABLED;
		}
			
	}
}

void mt7902_regd_update(struct mt792x_dev *dev)
{
	printk(KERN_DEBUG "init.c - mt7902_regd_update(dev)");
	struct mt76_dev *mdev = &dev->mt76;
	struct ieee80211_hw *hw = mdev->hw;
	struct wiphy *wiphy = hw->wiphy;

	dev_info(mdev->dev, "Regulatory hardware update for country: %c%c (ENV: %d)\n",
		 mdev->alpha2[0], mdev->alpha2[1], dev->country_ie_env);


	//mt7902_mcu_set_clc(dev, mdev->alpha2, dev->country_ie_env);
	if (mt7902_mcu_set_clc(dev, mdev->alpha2, dev->country_ie_env))
		dev_err(mdev->dev, "Failed to set CLC for region %c%c\n", 
			mdev->alpha2[0], mdev->alpha2[1]);

	dev_info(mdev->dev, "Recalculating channel list for %c%c\n", 
		mdev->alpha2[0], mdev->alpha2[1]);
	mt7902_regd_channel_update(wiphy, dev);
	//mt76_connac_mcu_set_channel_domain(hw->priv);
	if (mt76_connac_mcu_set_channel_domain(hw->priv))
		dev_warn(mdev->dev, "MCU failed to acknowledge channel domain update\n");

	dev_info(mdev->dev, "Updating TX SAR power limits\n");
	mt7902_set_tx_sar_pwr(hw, NULL);
	dev_info(mdev->dev, "Regulatory hardware update complete\n");
}
EXPORT_SYMBOL_GPL(mt7902_regd_update);

static void
mt7902_regd_notifier(struct wiphy *wiphy,
		     struct regulatory_request *request)
{
	printk(KERN_DEBUG "init.c - mt7902_regd_notifier(wiphy, request)");
	struct ieee80211_hw *hw = wiphy_to_ieee80211_hw(wiphy);
	struct mt792x_dev *dev = mt792x_hw_dev(hw);
	struct mt76_connac_pm *pm = &dev->pm;

	dev_info(dev->mt76.dev, "Regulatory update: Country %c%c (Initiator: %d, Region: %d)\n",
		 request->alpha2[0], request->alpha2[1], 
		 request->initiator, request->dfs_region);

	memcpy(dev->mt76.alpha2, request->alpha2, sizeof(dev->mt76.alpha2));
	dev->mt76.region = request->dfs_region;
	dev->country_ie_env = request->country_ie_env;

	if (request->initiator == NL80211_REGDOM_SET_BY_USER) {
		dev_info(dev->mt76.dev, "Regulatory set by user. Alpha2: %c%c\n", 
			dev->mt76.alpha2[0], dev->mt76.alpha2[1]);
		if (dev->mt76.alpha2[0] == '0' && dev->mt76.alpha2[1] == '0')
			wiphy->regulatory_flags &= ~REGULATORY_COUNTRY_IE_IGNORE;
		else
			wiphy->regulatory_flags |= REGULATORY_COUNTRY_IE_IGNORE;
	}

	if (pm->suspended) {
		dev_warn(dev->mt76.dev, "Regulatory update received while suspended, skipping\n");
		return;
	}

	dev->regd_in_progress = true;
	dev_info(dev->mt76.dev, "Updating hardware with new regulatory rules\n");
	mt792x_mutex_acquire(dev);
	mt7902_regd_update(dev);
	mt792x_mutex_release(dev);

	dev->regd_in_progress = false;
	dev_info(dev->mt76.dev, "Regulatory update complete. Waking up waiters.\n");
	wake_up(&dev->wait);
}

int mt7902_mac_init(struct mt792x_dev *dev)
{
	printk(KERN_DEBUG "init.c - mt7902_mac_init(dev)");
	int i, ret;

	dev_info(dev->mt76.dev, "Initializing MAC engine (MDP/DCR settings)\n");
	mt76_rmw_field(dev, MT_MDP_DCR1, MT_MDP_DCR1_MAX_RX_LEN, 1536);
	/* enable hardware de-agg */
	mt76_set(dev, MT_MDP_DCR0, MT_MDP_DCR0_DAMSDU_EN);
	/* enable hardware rx header translation */
	mt76_set(dev, MT_MDP_DCR0, MT_MDP_DCR0_RX_HDR_TRANS_EN);

	dev_info(dev->mt76.dev, "Hardware de-agg and header translation enabled\n");

	dev_info(dev->mt76.dev, "Clearing %d WTBL entries\n", MT792x_WTBL_SIZE);
	for (i = 0; i < MT792x_WTBL_SIZE; i++)
		mt7902_mac_wtbl_update(dev, i,
				       MT_WTBL_UPDATE_ADM_COUNT_CLEAR);
	for (i = 0; i < 2; i++) {
		dev_info(dev->mt76.dev, "Initializing MAC band %d\n", i);
		mt792x_mac_init_band(dev, i);
	}

	dev_info(dev->mt76.dev, "Setting RTS threshold via MCU\n");
	ret = mt76_connac_mcu_set_rts_thresh(&dev->mt76, 0x92b, 0);
	if (ret) {
		dev_err(dev->mt76.dev, "Failed to set RTS threshold: %d\n", ret);
	} else {
		dev_info(dev->mt76.dev, "MAC initialization sequence completed\n");
	}

	return ret;
	//return mt76_connac_mcu_set_rts_thresh(&dev->mt76, 0x92b, 0);
}
EXPORT_SYMBOL_GPL(mt7902_mac_init);

static int __mt7902_init_hardware(struct mt792x_dev *dev)
{
	printk(KERN_DEBUG "init.c - __mt7902_init_hardware(dev)");
	int ret;

	/* force firmware operation mode into normal state,
	 * which should be set before firmware download stage.
	 */
	dev_info(dev->mt76.dev, "Setting SWDEF_MODE to NORMAL\n");
	mt76_wr(dev, MT_SWDEF_MODE, MT_SWDEF_NORMAL_MODE);
	dev_info(dev->mt76.dev, "Starting MCU initialization (Firmware download/start)\n");
	ret = mt792x_mcu_init(dev);
	if (ret) {
		dev_err(dev->mt76.dev, "MCU init failed: %d\n", ret);
		goto out;
	}

	dev_info(dev->mt76.dev, "Applying EEPROM overrides\n");
	ret = mt76_eeprom_override(&dev->mphy);
	if (ret) {
		dev_err(dev->mt76.dev, "EEPROM override failed: %d\n", ret);
		goto out;
	}

	dev_info(dev->mt76.dev, "Pushing EEPROM data to MCU\n");
	ret = mt7902_mcu_set_eeprom(dev);
	if (ret) {
		dev_err(dev->mt76.dev, "MCU set EEPROM failed: %d\n", ret);
		goto out;
	}

	dev_info(dev->mt76.dev, "Starting MAC initialization\n");
	ret = mt7902_mac_init(dev);
	if (ret) {
		dev_err(dev->mt76.dev, "MAC init failed: %d\n", ret);
	}

out:
	if (ret == 0)
		dev_info(dev->mt76.dev, "__mt7902_init_hardware sequence completed successfully\n");

	return ret;
}

static int mt7902_init_hardware(struct mt792x_dev *dev)
{
	printk(KERN_DEBUG "init.c - mt7902_init_hardware(dev)");
	int ret, i;

	dev_info(dev->mt76.dev, "Hardware init started. Max retries: %d\n", MT792x_MCU_INIT_RETRY_COUNT);
	set_bit(MT76_STATE_INITIALIZED, &dev->mphy.state);

	for (i = 0; i < MT792x_MCU_INIT_RETRY_COUNT; i++) {
		if (i > 0)
			dev_warn(dev->mt76.dev, "Retrying hardware init... (Attempt %d/%d)\n", 
				 i + 1, MT792x_MCU_INIT_RETRY_COUNT);

		ret = __mt7902_init_hardware(dev);
		if (!ret) {
			// 2. Success log
			if (i > 0)
				dev_info(dev->mt76.dev, "Hardware init succeeded after %d retries\n", i);
			
			break;
		}

		dev_err(dev->mt76.dev, "Attempt %d failed with error: %d. Triggering reset.\n", i + 1, ret);
		mt792x_init_reset(dev);
	}

	if (i == MT792x_MCU_INIT_RETRY_COUNT) {
		dev_err(dev->mt76.dev, "hardware init failed\n");
		return ret;
	}

	return 0;
}

static void mt7902_init_work(struct work_struct *work)
{
	printk(KERN_DEBUG "init.c - mt7902_init_work(work)");
	struct mt792x_dev *dev = container_of(work, struct mt792x_dev,
					      init_work);
	int ret;

	dev_info(dev->mt76.dev, "Starting hardware initialization...\n");
	ret = mt7902_init_hardware(dev);
	if (ret) {
		dev_err(dev->mt76.dev, "mt7902_init_hardware failed: %d\n", ret);
		return;
	}

	dev_info(dev->mt76.dev, "Hardware initialization successful\n");

	mt76_set_stream_caps(&dev->mphy, true);
	mt7902_set_stream_he_caps(&dev->phy);
	mt792x_config_mac_addr_list(dev);

	dev_info(dev->mt76.dev, "Registering device with mt76 core\n");
	ret = mt76_register_device(&dev->mt76, true, mt76_rates,
				   ARRAY_SIZE(mt76_rates));
	if (ret) {
		dev_err(dev->mt76.dev, "register device failed\n");
		return;
	}

	ret = mt7902_init_debugfs(dev);
	if (ret) {
		dev_err(dev->mt76.dev, "register debugfs failed\n");
		return;
	}

	ret = mt7902_thermal_init(&dev->phy);
	if (ret) {
		dev_err(dev->mt76.dev, "thermal init failed\n");
		return;
	}

	/* we support chip reset now */
	dev->hw_init_done = true;

	dev_info(dev->mt76.dev, "Setting deep sleep: %s\n", dev->pm.ds_enable ? "on" : "off");
	mt76_connac_mcu_set_deep_sleep(&dev->mt76, dev->pm.ds_enable);
	dev_info(dev->mt76.dev, "MT7902 device registration complete\n");
}

int mt7902_register_device(struct mt792x_dev *dev)
{
	printk(KERN_DEBUG "init.c - mt7902_register_device(dev)");
	struct ieee80211_hw *hw = mt76_hw(dev);
	int ret;

	dev_info(dev->mt76.dev, "Registering device (Bus: %s)\n", 
            mt76_is_sdio(&dev->mt76) ? "SDIO" : (mt76_is_usb(&dev->mt76) ? "USB" : "MMIO"));
	dev->phy.dev = dev;
	dev->phy.mt76 = &dev->mt76.phy;
	dev->mt76.phy.priv = &dev->phy;
	dev->mt76.tx_worker.fn = mt792x_tx_worker;

	INIT_DELAYED_WORK(&dev->pm.ps_work, mt792x_pm_power_save_work);
	INIT_WORK(&dev->pm.wake_work, mt792x_pm_wake_work);
	spin_lock_init(&dev->pm.wake.lock);
	mutex_init(&dev->pm.mutex);
	init_waitqueue_head(&dev->pm.wait);
	init_waitqueue_head(&dev->wait);
	if (mt76_is_sdio(&dev->mt76))
		init_waitqueue_head(&dev->mt76.sdio.wait);
	spin_lock_init(&dev->pm.txq_lock);
	INIT_DELAYED_WORK(&dev->mphy.mac_work, mt792x_mac_work);
	INIT_DELAYED_WORK(&dev->phy.scan_work, mt7902_scan_work);
	INIT_DELAYED_WORK(&dev->coredump.work, mt7902_coredump_work);
#if IS_ENABLED(CONFIG_IPV6)
	INIT_WORK(&dev->ipv6_ns_work, mt7902_set_ipv6_ns_work);
	skb_queue_head_init(&dev->ipv6_ns_list);
#endif
	skb_queue_head_init(&dev->phy.scan_event_list);
	skb_queue_head_init(&dev->coredump.msg_list);

	INIT_WORK(&dev->reset_work, mt7902_mac_reset_work);
	INIT_WORK(&dev->init_work, mt7902_init_work);

	INIT_WORK(&dev->phy.roc_work, mt7902_roc_work);
	timer_setup(&dev->phy.roc_timer, mt792x_roc_timer, 0);
	init_waitqueue_head(&dev->phy.roc_wait);

	dev->pm.idle_timeout = MT792x_PM_TIMEOUT;
	dev->pm.stats.last_wake_event = jiffies;
	dev->pm.stats.last_doze_event = jiffies;
	if (!mt76_is_usb(&dev->mt76)) {
		dev->pm.enable_user = true;
		dev->pm.enable = true;
		dev->pm.ds_enable_user = true;
		dev->pm.ds_enable = true;
	}

	if (!mt76_is_mmio(&dev->mt76))
		hw->extra_tx_headroom += MT_SDIO_TXD_SIZE + MT_SDIO_HDR_SIZE;

    dev_info(dev->mt76.dev, "Initializing ACPI SAR\n");
	mt792x_init_acpi_sar(dev);

	ret = mt792x_init_wcid(dev);
	if (ret)
		return ret;

    dev_info(dev->mt76.dev, "Initializing Wiphy\n");
	ret = mt792x_init_wiphy(hw);
	if (ret)
		return ret;

	hw->wiphy->reg_notifier = mt7902_regd_notifier;
	dev->mphy.sband_2g.sband.ht_cap.cap |=
			IEEE80211_HT_CAP_LDPC_CODING |
			IEEE80211_HT_CAP_MAX_AMSDU;
	dev->mphy.sband_5g.sband.ht_cap.cap |=
			IEEE80211_HT_CAP_LDPC_CODING |
			IEEE80211_HT_CAP_MAX_AMSDU;
	dev->mphy.sband_5g.sband.vht_cap.cap |=
			IEEE80211_VHT_CAP_MAX_MPDU_LENGTH_11454 |
			IEEE80211_VHT_CAP_MAX_A_MPDU_LENGTH_EXPONENT_MASK |
			IEEE80211_VHT_CAP_SU_BEAMFORMEE_CAPABLE |
			IEEE80211_VHT_CAP_MU_BEAMFORMEE_CAPABLE |
			(3 << IEEE80211_VHT_CAP_BEAMFORMEE_STS_SHIFT);
	if (is_mt7922(&dev->mt76))
		dev->mphy.sband_5g.sband.vht_cap.cap |=
			IEEE80211_VHT_CAP_SUPP_CHAN_WIDTH_160MHZ |
			IEEE80211_VHT_CAP_SHORT_GI_160;

	dev_info(dev->mt76.dev, "Chainmask: 0x%x, Antennas RX/TX: %d/%d\n", 
            dev->mphy.chainmask, hw->wiphy->available_antennas_rx, hw->wiphy->available_antennas_tx);

	dev->mphy.hw->wiphy->available_antennas_rx = dev->mphy.chainmask;
	dev->mphy.hw->wiphy->available_antennas_tx = dev->mphy.chainmask;

    dev_info(dev->mt76.dev, "Scheduling mt7902_init_work\n");
	queue_work(system_wq, &dev->init_work);

	return 0;
}
EXPORT_SYMBOL_GPL(mt7902_register_device);
