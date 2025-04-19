// SPDX-License-Identifier: ISC
/* Copyright (C) 2020 MediaTek Inc. */

#include <linux/firmware.h>
#include "besra.h"
#include "eeprom.h"

static int besra_eeprom_load_precal(struct besra_dev *dev)
{
	struct mt76_dev *mdev = &dev->mt76;
	u8 *eeprom = mdev->eeprom.data;
	u32 val = eeprom[MT_EE_DO_PRE_CAL];

	if (!dev->flash_mode)
		return 0;

	if (val != (MT_EE_WIFI_CAL_DPD | MT_EE_WIFI_CAL_GROUP))
		return 0;

	val = MT_EE_CAL_GROUP_SIZE + MT_EE_CAL_DPD_SIZE;
	dev->cal = devm_kzalloc(mdev->dev, val, GFP_KERNEL);
	if (!dev->cal)
		return -ENOMEM;

	return mt76_get_of_eeprom(mdev, dev->cal, MT_EE_PRECAL_V2, val);
}

static int besra_check_eeprom(struct besra_dev *dev)
{
	u8 *eeprom = dev->mt76.eeprom.data;
	u16 val = get_unaligned_le16(eeprom);

	switch (val) {
	case 0x7915:
	case 0x7916:
	case 0x7986:
		return 0;
	default:
		return -EINVAL;
	}
}

static char *besra_eeprom_name(struct besra_dev *dev)
{
	return MT7902_EEPROM_DEFAULT;
}

static int
besra_eeprom_load_default(struct besra_dev *dev)
{
	u8 *eeprom = dev->mt76.eeprom.data;
	const struct firmware *fw = NULL;
	int ret;

	ret = request_firmware(&fw, besra_eeprom_name(dev), dev->mt76.dev);
	if (ret)
		return ret;

	if (!fw || !fw->data) {
		dev_err(dev->mt76.dev, "Invalid default bin\n");
		ret = -EINVAL;
		goto out;
	}

	memcpy(eeprom, fw->data, BESRA_EEPROM_SIZE);
	dev->flash_mode = true;

out:
	release_firmware(fw);

	return ret;
}

static int besra_eeprom_load(struct besra_dev *dev)
{
	int ret;

	ret = mt76_eeprom_init(&dev->mt76, BESRA_EEPROM_SIZE);
	if (ret < 0)
		return ret;

	if (ret) {
		dev->flash_mode = true;
	} else {
		u8 free_block_num;
		u32 block_num, i;

		besra_mcu_get_eeprom_free_block(dev, &free_block_num);
		/* efuse info not enough */
		if (free_block_num >= 29)
			return -EINVAL;

		/* read eeprom data from efuse */
		block_num = DIV_ROUND_UP(BESRA_EEPROM_SIZE,
					 BESRA_EEPROM_BLOCK_SIZE);
		for (i = 0; i < block_num; i++)
			besra_mcu_get_eeprom(dev,
					      i * BESRA_EEPROM_BLOCK_SIZE);
	}

	return besra_check_eeprom(dev);
}

static void besra_eeprom_parse_band_config(struct besra_phy *phy)
{
	struct besra_dev *dev = phy->dev;
	u8 *eeprom = dev->mt76.eeprom.data;
	u32 val;

	/* TODO:
	 * since default bin hasn't properly configured yet,
	 * currently hardcode band cap for bellwether */

	/* val = eeprom[MT_EE_WIFI_CONF + phy->band_idx]; */
	/* val = FIELD_GET(MT_EE_WIFI_CONF0_BAND_SEL, val); */

	/* switch (val) { */
	/* 	case MT_EE_V2_BAND_SEL_5GHZ: */
	/* 		phy->mt76->cap.has_5ghz = true; */
	/* 		return; */
	/* 	case MT_EE_V2_BAND_SEL_6GHZ: */
	/* 		phy->mt76->cap.has_6ghz = true; */
	/* 		return; */
	/* 	case MT_EE_V2_BAND_SEL_5GHZ_6GHZ: */
	/* 		phy->mt76->cap.has_5ghz = true; */
	/* 		phy->mt76->cap.has_6ghz = true; */
	/* 		return; */
	/* 	default */
	/* 		phy->mt76->cap.has_2ghz = true; */
	/* 		return; */
	/* } */

	if (phy->band_idx == 2)
		phy->mt76->cap.has_2ghz = true;
	else {
		phy->mt76->cap.has_5ghz = true;
		/* phy->mt76->cap.has_6ghz = true; */
	}
}

void besra_eeprom_parse_hw_cap(struct besra_dev *dev,
				struct besra_phy *phy)
{
	u8 nss, nss_band, nss_band_max, *eeprom = dev->mt76.eeprom.data;
	struct mt76_phy *mphy = phy->mt76;
	u8 phy_idx = besra_get_phy_id(phy);

	besra_eeprom_parse_band_config(phy);

	/* read tx/rx mask from eeprom */
	nss = FIELD_GET(MT_EE_WIFI_CONF0_TX_PATH,
			eeprom[MT_EE_WIFI_CONF + phy->band_idx]);

	if (!nss || nss > 4)
		nss = 4;

	/* read tx/rx stream */
	nss_band = nss;

	if (dev->dbdc_support) {
		nss_band = FIELD_GET(MT_EE_WIFI_CONF_STREAM_NUM,
				     eeprom[MT_EE_WIFI_CONF + 2 + phy->band_idx]);

		nss_band_max = MT_EE_NSS_MAX_DBDC_MA7986;
	} else {
		nss_band_max = MT_EE_NSS_MAX_MA7986;
	}

	if (!nss_band || nss_band > nss_band_max)
		nss_band = nss_band_max;

	if (nss_band > nss) {
		dev_warn(dev->mt76.dev,
			 "nss mismatch, nss(%d) nss_band(%d) band(%d) phy_id(%d)\n",
			 nss, nss_band, phy->band_idx, phy_idx);
		nss = nss_band;
	}

	mphy->chainmask = BIT(nss) - 1;
	switch (phy_idx) {
	case MT_MAIN_PHY:
		dev->chain_shift_ext = hweight8(mphy->chainmask);
		dev->chain_shift_tri = dev->chain_shift_ext;
		break;
	case MT_EXT_PHY:
		mphy->chainmask <<= dev->chain_shift_ext;
		dev->chain_shift_tri += hweight8(mphy->chainmask);
		break;
	case MT_TRI_PHY:
		mphy->chainmask <<= dev->chain_shift_tri;
		break;
	default:
		break;
	}
	mphy->antenna_mask = BIT(nss_band) - 1;
	dev->chainmask |= mphy->chainmask;

	dev_err(dev->mt76.dev,
		 "nss info, nss(%d) nss_band(%d) band(%d) phy_id(%d) chainmask(%08x) antenna_mask(%02x) dev chainmask(%08x)\n",
		 nss, nss_band, phy->band_idx, phy_idx,
		 mphy->chainmask, mphy->antenna_mask, dev->chainmask);
}

int besra_eeprom_init(struct besra_dev *dev)
{
	int ret;

	ret = besra_eeprom_load(dev);
	if (ret < 0) {
		if (ret != -EINVAL)
			return ret;

		dev_warn(dev->mt76.dev, "eeprom load fail, use default bin\n");
		ret = besra_eeprom_load_default(dev);
		if (ret)
			return ret;
	}

	ret = besra_eeprom_load_precal(dev);
	if (ret)
		return ret;

	besra_eeprom_parse_hw_cap(dev, &dev->phy);
	memcpy(dev->mphy.macaddr, dev->mt76.eeprom.data + MT_EE_MAC_ADDR,
	       ETH_ALEN);

	mt76_eeprom_override(&dev->mphy);

	return 0;
}

int besra_eeprom_get_target_power(struct besra_dev *dev,
				   struct ieee80211_channel *chan,
				   u8 chain_idx)
{
	u8 *eeprom = dev->mt76.eeprom.data;
	int index, target_power;
	bool tssi_on;

	if (chain_idx > 3)
		return -EINVAL;

	tssi_on = besra_tssi_enabled(dev, chan->band);

	if (chan->band == NL80211_BAND_2GHZ) {
		index = MT_EE_TX0_POWER_2G_V2 + chain_idx * 3;
		target_power = eeprom[index];

		if (!tssi_on)
			target_power += eeprom[index + 1];
	} else {
		int group = besra_get_channel_group(chan->hw_value);

		index = MT_EE_TX0_POWER_5G_V2 + chain_idx * 12;
		target_power = eeprom[index + group];

		if (!tssi_on)
			target_power += eeprom[index + 8];
	}

	return target_power;
}

s8 besra_eeprom_get_power_delta(struct besra_dev *dev, int band)
{
	u8 *eeprom = dev->mt76.eeprom.data;
	u32 val;
	s8 delta;

	if (band == NL80211_BAND_2GHZ)
		val = eeprom[MT_EE_RATE_DELTA_2G_V2];
	else
		val = eeprom[MT_EE_RATE_DELTA_5G_V2];

	if (!(val & MT_EE_RATE_DELTA_EN))
		return 0;

	delta = FIELD_GET(MT_EE_RATE_DELTA_MASK, val);

	return val & MT_EE_RATE_DELTA_SIGN ? delta : -delta;
}

const u8 besra_sku_group_len[] = {
	[SKU_CCK] = 4,
	[SKU_OFDM] = 8,
	[SKU_HT_BW20] = 8,
	[SKU_HT_BW40] = 9,
	[SKU_VHT_BW20] = 12,
	[SKU_VHT_BW40] = 12,
	[SKU_VHT_BW80] = 12,
	[SKU_VHT_BW160] = 12,
	[SKU_HE_RU26] = 12,
	[SKU_HE_RU52] = 12,
	[SKU_HE_RU106] = 12,
	[SKU_HE_RU242] = 12,
	[SKU_HE_RU484] = 12,
	[SKU_HE_RU996] = 12,
	[SKU_HE_RU2x996] = 12
};
