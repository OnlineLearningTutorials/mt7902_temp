/* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause */
/*
 * Copyright (c) 2016 MediaTek Inc.
 */
/*
 ** gl_vendor.c
 **
 **
 */

/*******************************************************************************
 *                         C O M P I L E R   F L A G S
 *******************************************************************************
 */

/*******************************************************************************
 *                    E X T E R N A L   R E F E R E N C E S
 *******************************************************************************
 */
#include "gl_os.h"
#include "debug.h"
#include "wlan_lib.h"
#include "gl_wext.h"
#include "precomp.h"
#include <linux/can/netlink.h>
#include <net/netlink.h>
#include <net/cfg80211.h>
#include "gl_cfg80211.h"
#include "gl_vendor.h"
#include "wlan_oid.h"

#if KERNEL_VERSION(3, 16, 0) <= LINUX_VERSION_CODE

/*******************************************************************************
 *                              C O N S T A N T S
 *******************************************************************************
 */

/*******************************************************************************
 *                             D A T A   T Y P E S
 *******************************************************************************
 */

/*******************************************************************************
 *                            P U B L I C   D A T A
 *******************************************************************************
 */
uint8_t g_GetResultsBufferedCnt;
uint8_t g_GetResultsCmdCnt;

const struct nla_policy nal_parse_wifi_setband[
	QCA_WLAN_VENDOR_ATTR_MAX + 1] = {
	[QCA_WLAN_VENDOR_ATTR_SETBAND_VALUE] = {.type = NLA_U32},
	[QCA_WLAN_VENDOR_ATTR_SETBAND_MASK] = {.type = NLA_U32},
};

const struct nla_policy nla_parse_wifi_attribute[
	WIFI_ATTRIBUTE_MAX + 1] = {
	[WIFI_ATTRIBUTE_BAND] = {.type = NLA_U32},
#if KERNEL_VERSION(5, 9, 0) <= CFG80211_VERSION_CODE
	[WIFI_ATTRIBUTE_PNO_RANDOM_MAC_OUI] = NLA_POLICY_MIN_LEN(0),
#elif KERNEL_VERSION(5, 4, 0) <= CFG80211_VERSION_CODE
	[WIFI_ATTRIBUTE_PNO_RANDOM_MAC_OUI] = {.type = NLA_MIN_LEN, .len = 0 },
#else
	[WIFI_ATTRIBUTE_PNO_RANDOM_MAC_OUI] = {.type = NLA_BINARY},
#endif
	[WIFI_ATTRIBUTE_COUNTRY_CODE] = {.type = NLA_STRING},
#if CFG_SUPPORT_OLD_VENDOR_HAL
	[WIFI_ATTRIBUTE_RSSI_MONITOR_MAX_RSSI] = {.type = NLA_U32},
	[WIFI_ATTRIBUTE_RSSI_MONITOR_MIN_RSSI] = {.type = NLA_U32},
	[WIFI_ATTRIBUTE_RSSI_MONITOR_START]    = {.type = NLA_U32},
#endif
	[WIFI_ATTRIBUTE_ROAMING_BLACKLIST_NUM] = {.type = NLA_U32},
#if KERNEL_VERSION(5, 9, 0) <= CFG80211_VERSION_CODE
	[WIFI_ATTRIBUTE_ROAMING_BLACKLIST_BSSID] = NLA_POLICY_MIN_LEN(0),
#elif KERNEL_VERSION(5, 4, 0) <= CFG80211_VERSION_CODE
	[WIFI_ATTRIBUTE_ROAMING_BLACKLIST_BSSID] = {
		.type = NLA_MIN_LEN, .len = 0 },
#else
	[WIFI_ATTRIBUTE_ROAMING_BLACKLIST_BSSID] = {.type = NLA_BINARY},
#endif
	[WIFI_ATTRIBUTE_ROAMING_WHITELIST_NUM] = {.type = NLA_U32},
#if KERNEL_VERSION(5, 9, 0) <= CFG80211_VERSION_CODE
	[WIFI_ATTRIBUTE_ROAMING_WHITELIST_SSID] = NLA_POLICY_MIN_LEN(0),
#elif KERNEL_VERSION(5, 4, 0) <= CFG80211_VERSION_CODE
	[WIFI_ATTRIBUTE_ROAMING_WHITELIST_SSID] = {
		.type = NLA_MIN_LEN, .len = 0 },
#else
	[WIFI_ATTRIBUTE_ROAMING_WHITELIST_SSID] = {.type = NLA_BINARY},
#endif
	[WIFI_ATTRIBUTE_ROAMING_STATE] = {.type = NLA_U32},
	[WIFI_ATTRIBUTE_TX_POWER_SCENARIO] = {.type = NLA_U32},
};

#if !CFG_SUPPORT_OLD_VENDOR_HAL
const struct nla_policy nla_parse_wifi_rssi_monitor[
		WIFI_ATTRIBUTE_RSSI_MONITOR_ATTRIBUTE_MAX + 1] = {
	[WIFI_ATTRIBUTE_RSSI_MONITOR_MAX_RSSI] = {.type = NLA_U32},
	[WIFI_ATTRIBUTE_RSSI_MONITOR_MIN_RSSI] = {.type = NLA_U32},
	[WIFI_ATTRIBUTE_RSSI_MONITOR_START]    = {.type = NLA_U32},
};
#endif

const struct nla_policy nla_get_version_policy[
		LOGGER_ATTRIBUTE_MAX + 1] = {
#if KERNEL_VERSION(5, 9, 0) <= CFG80211_VERSION_CODE
	[LOGGER_ATTRIBUTE_DRIVER_VER] = NLA_POLICY_MIN_LEN(0),
	[LOGGER_ATTRIBUTE_FW_VER] = NLA_POLICY_MIN_LEN(0),
#elif KERNEL_VERSION(5, 4, 0) <= CFG80211_VERSION_CODE
	[LOGGER_ATTRIBUTE_DRIVER_VER] = { .type = NLA_MIN_LEN, .len = 0 },
	[LOGGER_ATTRIBUTE_FW_VER] = { .type = NLA_MIN_LEN, .len = 0 },
#else
	[LOGGER_ATTRIBUTE_DRIVER_VER] = { .type = NLA_UNSPEC },
	[LOGGER_ATTRIBUTE_FW_VER] = { .type = NLA_UNSPEC },
#endif
};

const struct nla_policy nla_parse_offloading_policy[
		 MKEEP_ALIVE_ATTRIBUTE_PERIOD_MSEC + 1] = {
	[MKEEP_ALIVE_ATTRIBUTE_ID] = {.type = NLA_U8},
	[MKEEP_ALIVE_ATTRIBUTE_IP_PKT] = {.type = NLA_BINARY},
	[MKEEP_ALIVE_ATTRIBUTE_IP_PKT_LEN] = {.type = NLA_U16},
#if KERNEL_VERSION(5, 9, 0) <= CFG80211_VERSION_CODE
	[MKEEP_ALIVE_ATTRIBUTE_SRC_MAC_ADDR] =
		NLA_POLICY_EXACT_LEN_WARN(MAC_ADDR_LEN),
	[MKEEP_ALIVE_ATTRIBUTE_DST_MAC_ADDR] =
		NLA_POLICY_EXACT_LEN_WARN(MAC_ADDR_LEN),
#else
	[MKEEP_ALIVE_ATTRIBUTE_SRC_MAC_ADDR] = {
		.type = NLA_BINARY, .len = MAC_ADDR_LEN},
	[MKEEP_ALIVE_ATTRIBUTE_DST_MAC_ADDR] = {
		.type = NLA_BINARY, .len = MAC_ADDR_LEN},
#endif
	[MKEEP_ALIVE_ATTRIBUTE_PERIOD_MSEC] = {.type = NLA_U32},
};

const struct nla_policy nla_get_preferred_freq_list_policy[
		WIFI_VENDOR_ATTR_PREFERRED_FREQ_LIST_MAX + 1] = {
	[WIFI_VENDOR_ATTR_PREFERRED_FREQ_LIST_IFACE_TYPE] = {.type = NLA_U32},
#if KERNEL_VERSION(5, 9, 0) <= CFG80211_VERSION_CODE
	[WIFI_VENDOR_ATTR_PREFERRED_FREQ_LIST_GET] = NLA_POLICY_MIN_LEN(0),
#endif
};

const struct nla_policy nla_get_acs_policy[
		WIFI_VENDOR_ATTR_ACS_MAX + 1] = {
	[WIFI_VENDOR_ATTR_ACS_HW_MODE] = { .type = NLA_U8 },
	[WIFI_VENDOR_ATTR_ACS_HT_ENABLED] = { .type = NLA_FLAG },
	[WIFI_VENDOR_ATTR_ACS_HT40_ENABLED] = { .type = NLA_FLAG },
	[WIFI_VENDOR_ATTR_ACS_VHT_ENABLED] = { .type = NLA_FLAG },
	[WIFI_VENDOR_ATTR_ACS_CHWIDTH] = { .type = NLA_U16 },
#if KERNEL_VERSION(5, 9, 0) <= CFG80211_VERSION_CODE
	[WIFI_VENDOR_ATTR_ACS_CH_LIST] = NLA_POLICY_MIN_LEN(0),
	[WIFI_VENDOR_ATTR_ACS_FREQ_LIST] = NLA_POLICY_MIN_LEN(0),
#elif KERNEL_VERSION(5, 4, 0) <= CFG80211_VERSION_CODE
	[WIFI_VENDOR_ATTR_ACS_CH_LIST] = { .type = NLA_MIN_LEN, .len = 0 },
	[WIFI_VENDOR_ATTR_ACS_FREQ_LIST] = { .type = NLA_MIN_LEN, .len = 0 },
#else
	[WIFI_VENDOR_ATTR_ACS_CH_LIST] = { .type = NLA_UNSPEC },
	[WIFI_VENDOR_ATTR_ACS_FREQ_LIST] = { .type = NLA_UNSPEC },
#endif
};

const struct nla_policy nla_get_apf_policy[
		APF_ATTRIBUTE_MAX + 1] = {
	[APF_ATTRIBUTE_VERSION] = {.type = NLA_U32},
	[APF_ATTRIBUTE_MAX_LEN] = {.type = NLA_U32},
#if KERNEL_VERSION(5, 9, 0) <= CFG80211_VERSION_CODE
	[APF_ATTRIBUTE_PROGRAM] = NLA_POLICY_MIN_LEN(0),
#elif KERNEL_VERSION(5, 4, 0) <= CFG80211_VERSION_CODE
	[APF_ATTRIBUTE_PROGRAM] = {.type = NLA_MIN_LEN, .len = 0},
#else
	[APF_ATTRIBUTE_PROGRAM] = {.type = NLA_UNSPEC},
#endif
	[APF_ATTRIBUTE_PROGRAM_LEN] = {.type = NLA_U32},
};

/*******************************************************************************
 *                           P R I V A T E   D A T A
 *******************************************************************************
 */

/*******************************************************************************
 *                                 M A C R O S
 *******************************************************************************
 */

/*******************************************************************************
 *                   F U N C T I O N   D E C L A R A T I O N S
 *******************************************************************************
 */

/*******************************************************************************
 *                              F U N C T I O N S
 *******************************************************************************
 */
int mtk_cfg80211_NLA_PUT(struct sk_buff *skb, int attrtype,
			 int attrlen, const void *data)
{
	if (unlikely(nla_put(skb, attrtype, attrlen, data) < 0))
		return 0;
	return 1;
}

int mtk_cfg80211_nla_put_type(struct sk_buff *skb,
			      enum ENUM_NLA_PUT_DATE_TYPE type, int attrtype,
			      const void *value)
{
	u8 u8data = 0;
	u16 u16data = 0;
	u32 u32data = 0;
	u64 u64data = 0;

	switch (type) {
	case NLA_PUT_DATE_U8:
		u8data = *(u8 *)value;
		return mtk_cfg80211_NLA_PUT(skb, attrtype, sizeof(u8),
					    &u8data);
	case NLA_PUT_DATE_U16:
		u16data = *(u16 *)value;
		return mtk_cfg80211_NLA_PUT(skb, attrtype, sizeof(u16),
					    &u16data);
	case NLA_PUT_DATE_U32:
		u32data = *(u32 *)value;
		return mtk_cfg80211_NLA_PUT(skb, attrtype, sizeof(u32),
					    &u32data);
	case NLA_PUT_DATE_U64:
		u64data = *(u64 *)value;
		return mtk_cfg80211_NLA_PUT(skb, attrtype, sizeof(u64),
					    &u64data);
	default:
		break;
	}

	return 0;
}

int mtk_cfg80211_vendor_get_channel_list(struct wiphy *wiphy,
					 struct wireless_dev *wdev,
					 const void *data, int data_len)
{
	struct GLUE_INFO *prGlueInfo;
	struct nlattr *attr;
	uint32_t band = 0;
	uint8_t ucNumOfChannel, i, j;
	struct RF_CHANNEL_INFO *aucChannelList;
	uint32_t num_channels;
	uint32_t channels[MAX_CHN_NUM];
	struct sk_buff *skb;
	uint16_t u2CountryCode;

	ASSERT(wiphy && wdev);
	if ((data == NULL) || !data_len)
		return -EINVAL;

	DBGLOG(REQ, TRACE, "data_len=%d, iftype=%d\n", data_len, wdev->iftype);

	attr = (struct nlattr *)data;
	if (attr->nla_type == WIFI_ATTRIBUTE_BAND)
		band = nla_get_u32(attr);

	DBGLOG(REQ, TRACE, "Get channel list for band: %d\n", band);

	prGlueInfo = wlanGetGlueInfo();
	if (!prGlueInfo)
		return -EFAULT;

	aucChannelList = (struct RF_CHANNEL_INFO *)
		kalMemAlloc(sizeof(struct RF_CHANNEL_INFO)*MAX_CHN_NUM,
			VIR_MEM_TYPE);
	if (!aucChannelList) {
		DBGLOG(REQ, ERROR,
			"Can not alloc memory for rf channel info\n");
		return -ENOMEM;
	}
	kalMemZero(aucChannelList,
		sizeof(struct RF_CHANNEL_INFO)*MAX_CHN_NUM);

	switch (band) {
	case 1: /* 2.4G band */
		rlmDomainGetChnlList(prGlueInfo->prAdapter, BAND_2G4, TRUE,
			     MAX_CHN_NUM, &ucNumOfChannel, aucChannelList);
		break;
	case 2: /* 5G band without DFS channels */
		rlmDomainGetChnlList(prGlueInfo->prAdapter, BAND_5G, TRUE,
			     MAX_CHN_NUM, &ucNumOfChannel, aucChannelList);
		break;
	case 4: /* 5G band DFS channels only */
		rlmDomainGetDfsChnls(prGlueInfo->prAdapter, MAX_CHN_NUM,
				     &ucNumOfChannel, aucChannelList);
		break;
	default:
		ucNumOfChannel = 0;
		break;
	}

	kalMemZero(channels, sizeof(channels));
	u2CountryCode = prGlueInfo->prAdapter->rWifiVar.u2CountryCode;
	for (i = 0, j = 0; i < ucNumOfChannel; i++) {
		/* We need to report frequency list to HAL */
		channels[j] =
			nicChannelNum2Freq(
				aucChannelList[i].ucChannelNum,
				aucChannelList[i].eBand) / 1000;
		if (channels[j] == 0)
			continue;
		else if ((u2CountryCode == COUNTRY_CODE_TW) &&
			 (channels[j] >= 5180 && channels[j] <= 5260)) {
			/* Taiwan NCC has resolution to follow FCC spec
			 * to support 5G Band 1/2/3/4
			 * (CH36~CH48, CH52~CH64, CH100~CH140, CH149~CH165)
			 * Filter CH36~CH52 for compatible with some old
			 * devices.
			 */
			DBGLOG(REQ, TRACE, "skip channels[%d]=%d, country=%d\n",
			       j, channels[j], u2CountryCode);
			continue;
		} else {
			DBGLOG(REQ, TRACE, "channels[%d] = %d\n", j,
			       channels[j]);
			j++;
		}
	}
	num_channels = j;
	DBGLOG(REQ, INFO, "Get channel list for band: %d, num_channels=%d\n",
	       band, num_channels);

	kalMemFree(aucChannelList, VIR_MEM_TYPE,
		sizeof(struct RF_CHANNEL_INFO)*MAX_CHN_NUM);

	skb = cfg80211_vendor_cmd_alloc_reply_skb(wiphy, sizeof(channels));
	if (!skb) {
		DBGLOG(REQ, ERROR, "Allocate skb failed\n");
		return -ENOMEM;
	}

	if (unlikely(nla_put_u32(skb, WIFI_ATTRIBUTE_NUM_CHANNELS,
				 num_channels) < 0))
		goto nla_put_failure;

	if (unlikely(nla_put(skb, WIFI_ATTRIBUTE_CHANNEL_LIST,
			     (sizeof(uint32_t) * num_channels), channels) < 0))
		goto nla_put_failure;

	return cfg80211_vendor_cmd_reply(skb);

nla_put_failure:
	kfree_skb(skb);
	return -EFAULT;
}

int mtk_cfg80211_vendor_set_country_code(struct wiphy
		*wiphy, struct wireless_dev *wdev, const void *data,
		int data_len)
{
	struct GLUE_INFO *prGlueInfo;
	uint32_t rStatus;
	uint32_t u4BufLen;
	struct nlattr *attr;
	uint8_t country[2] = {0};

	ASSERT(wiphy && wdev);
	if ((data == NULL) || (data_len == 0))
		return -EINVAL;

	DBGLOG(REQ, INFO,
	       "vendor command: data_len=%d, iftype=%d\n", data_len,
	       wdev->iftype);

	attr = (struct nlattr *)data;
	if (attr->nla_type == WIFI_ATTRIBUTE_COUNTRY_CODE) {
		country[0] = *((uint8_t *)nla_data(attr));
		country[1] = *((uint8_t *)nla_data(attr) + 1);
	}

	DBGLOG(REQ, INFO, "Set country code: %c%c\n", country[0],
	       country[1]);

	prGlueInfo = wlanGetGlueInfo();
	if (!prGlueInfo)
		return -EFAULT;

	rStatus = kalIoctl(prGlueInfo, wlanoidSetCountryCode,
			   country, 2, FALSE, FALSE, TRUE, &u4BufLen);
	if (rStatus != WLAN_STATUS_SUCCESS) {
		DBGLOG(REQ, ERROR, "Set country code error: %x\n", rStatus);
		return -EFAULT;
	}

	return 0;
}

int mtk_cfg80211_vendor_set_scan_mac_oui(struct wiphy *wiphy,
	struct wireless_dev *wdev, const void *data, int data_len)
{
	struct GLUE_INFO *prGlueInfo = NULL;
	uint32_t rStatus = WLAN_STATUS_SUCCESS;
	struct nlattr *attr;
	uint32_t i = 0;
	struct PARAM_BSS_MAC_OUI rParamMacOui;
	uint32_t u4BufLen = 0;
	struct NETDEV_PRIVATE_GLUE_INFO *prNetDevPrivate = NULL;

	ASSERT(wiphy);
	ASSERT(wdev);

	if (data == NULL || data_len <= 0) {
		log_dbg(REQ, ERROR, "data error(len=%d)\n", data_len);
		return -EINVAL;
	}

	prGlueInfo = wlanGetGlueInfo();
	if (!prGlueInfo) {
		log_dbg(REQ, ERROR, "Invalid glue info\n");
		return -EFAULT;
	}
	prNetDevPrivate =
		(struct NETDEV_PRIVATE_GLUE_INFO *) netdev_priv(wdev->netdev);
	if (!prNetDevPrivate) {
		log_dbg(REQ, ERROR, "Invalid net device private\n");
		return -EFAULT;
	}
	rParamMacOui.ucBssIndex = prNetDevPrivate->ucBssIdx;

	attr = (struct nlattr *)data;
	kalMemZero(rParamMacOui.ucMacOui, MAC_OUI_LEN);
	if (nla_type(attr) != WIFI_ATTRIBUTE_PNO_RANDOM_MAC_OUI) {
		log_dbg(REQ, ERROR, "Set MAC oui type error(%u)\n",
			nla_type(attr));
		return -EINVAL;
	}

	if (nla_len(attr) != MAC_OUI_LEN) {
		log_dbg(REQ, ERROR, "Set MAC oui length error(%u), %u needed\n",
			nla_len(attr), MAC_OUI_LEN);
		return -EINVAL;
	}

	for (i = 0; i < MAC_OUI_LEN; i++)
		rParamMacOui.ucMacOui[i] = *((uint8_t *)nla_data(attr) + i);

	log_dbg(REQ, INFO, "Set MAC oui: %02x-%02x-%02x\n",
		rParamMacOui.ucMacOui[0], rParamMacOui.ucMacOui[1],
		rParamMacOui.ucMacOui[2]);

	rStatus = kalIoctl(prGlueInfo, wlanoidSetScanMacOui,
		&rParamMacOui, sizeof(rParamMacOui),
		FALSE, FALSE, FALSE, &u4BufLen);
	if (rStatus != WLAN_STATUS_SUCCESS) {
		log_dbg(REQ, ERROR, "Set MAC oui error: 0x%X\n", rStatus);
		return -EFAULT;
	}

	return 0;
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief This routine is to answer FWK that we can support FW Roaming.
 *
 * \param[in] wiphy wiphy for AIS STA.
 *
 * \param[in] wdev (not used here).
 *
 * \param[in] data (not used here).
 *
 * \param[in] data_len (not used here).
 *
 * \retval TRUE Success.
 *
 * \note we use cfg80211_vendor_cmd_reply to send the max number of our
 *       blacklist and whiltlist directly without receiving any data
 *       from the upper layer.
 */
/*----------------------------------------------------------------------------*/
int mtk_cfg80211_vendor_get_roaming_capabilities(struct wiphy *wiphy,
	struct wireless_dev *wdev, const void *data, int data_len)
{
	uint32_t maxNumOfList[2] = { MAX_FW_ROAMING_BLACKLIST_SIZE,
				     MAX_FW_ROAMING_WHITELIST_SIZE };
	struct sk_buff *skb;

	ASSERT(wiphy);

	DBGLOG(REQ, INFO,
		"Get roaming capabilities: max black/whitelist=%d/%d",
		maxNumOfList[0], maxNumOfList[1]);

	skb = cfg80211_vendor_cmd_alloc_reply_skb(wiphy, sizeof(maxNumOfList));
	if (!skb) {
		DBGLOG(REQ, ERROR, "Allocate skb failed\n");
		return -ENOMEM;
	}

	if (unlikely(nla_put(skb, WIFI_ATTRIBUTE_ROAMING_CAPABILITIES,
				sizeof(uint32_t), &maxNumOfList[0]) < 0))
		goto nla_put_failure;
	if (unlikely(nla_put(skb, WIFI_ATTRIBUTE_ROAMING_CAPABILITIES,
				sizeof(uint32_t), &maxNumOfList[1]) < 0))
		goto nla_put_failure;

	return cfg80211_vendor_cmd_reply(skb);

nla_put_failure:
	kfree_skb(skb);
	return -EFAULT;
}


/*----------------------------------------------------------------------------*/
/*!
 * \brief This routine is to receive the black/whiltelist. from FWK.
 *
 * \param[in] wiphy wiphy for AIS STA.
 *
 * \param[in] wdev (not used here).
 *
 * \param[in] data BSSIDs in the FWK blact&whitelist.
 *
 * \param[in] data_len the byte-length of the FWK blact&whitelist.
 *
 * \retval TRUE Success.
 *
 * \note we iterate each BSSID in 'data' and put it into driver blacklist.
 *       For now, whiltelist doesn't be implemented by the FWK currently.
 */
/*----------------------------------------------------------------------------*/
int mtk_cfg80211_vendor_config_roaming(struct wiphy *wiphy,
	       struct wireless_dev *wdev, const void *data, int data_len)
{
	struct GLUE_INFO *prGlueInfo = NULL;
	struct nlattr *attrlist;
	struct AIS_BLACKLIST_ITEM *prBlackList;
	struct BSS_DESC *prBssDesc = NULL;
	uint32_t len_shift = 0;
	uint32_t numOfList[2] = { 0 };
	uint8_t *aucBSSID = NULL;
	int i;

	DBGLOG(REQ, INFO,
	       "Receives roaming blacklist & whitelist with data_len=%d\n",
	       data_len);
	ASSERT(wiphy);
	ASSERT(wdev);
	if ((data == NULL) || (data_len == 0))
		return -EINVAL;

	prGlueInfo = (struct GLUE_INFO *) wiphy_priv(wiphy);
	if (!prGlueInfo)
		return -EINVAL;

	if (prGlueInfo->u4FWRoamingEnable == 0) {
		DBGLOG(REQ, INFO,
		       "FWRoaming is disabled (FWRoamingEnable=%d)\n",
		       prGlueInfo->u4FWRoamingEnable);
		return WLAN_STATUS_SUCCESS;
	}

	attrlist = (struct nlattr *)((uint8_t *) data);

	/* get the number of blacklist and copy those mac addresses from HAL */
	if (attrlist->nla_type ==
	    WIFI_ATTRIBUTE_ROAMING_BLACKLIST_NUM) {
		numOfList[0] = nla_get_u32(attrlist);
		len_shift += NLA_ALIGN(attrlist->nla_len);
	}
	DBGLOG(REQ, INFO, "Get the number of blacklist=%d\n",
	       numOfList[0]);

	if (numOfList[0] > MAX_FW_ROAMING_BLACKLIST_SIZE)
		return -EINVAL;

	/*Refresh all the FWKBlacklist */
	aisRefreshFWKBlacklist(prGlueInfo->prAdapter);

	/* Start to receive blacklist mac addresses and set to FWK blacklist */
	attrlist = (struct nlattr *)((uint8_t *) data + len_shift);
	for (i = 0; i < numOfList[0]; i++) {
		if (attrlist->nla_type ==
		    WIFI_ATTRIBUTE_ROAMING_BLACKLIST_BSSID) {
			prBssDesc =
				scanSearchBssDescByBssid(prGlueInfo->prAdapter,
							nla_data(attrlist));
			len_shift += NLA_ALIGN(attrlist->nla_len);
			attrlist =
				(struct nlattr *)((uint8_t *) data + len_shift);

			if (prBssDesc == NULL) {
				aucBSSID = nla_data(attrlist);
				DBGLOG(REQ, ERROR, "No found blacklist BSS="
					MACSTR "\n",
					MAC2STR(aucBSSID));
				continue;
			}

			prBlackList = aisAddBlacklist(prGlueInfo->prAdapter,
						      prBssDesc);

			if (prBlackList) {
				prBlackList->fgIsInFWKBlacklist = TRUE;
				DBGLOG(REQ, INFO,
					"Gets roaming blacklist SSID=%s addr="
					MACSTR "\n",
					prBssDesc->aucSSID,
					MAC2STR(prBssDesc->aucBSSID));
			} else {
				DBGLOG(REQ, ERROR,
					"prBlackList is NULL, return -EINVAL!");
				return -EINVAL;
			}
		}
	}

	return WLAN_STATUS_SUCCESS;
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief This routine is to turn on/off FW Roaming.
 *
 * \param[in] wiphy wiphy for AIS STA.
 *
 * \param[in] wdev (not used here).
 *
 * \param[in] data 1 for ON / 0 for OFF.
 *
 * \param[in] data_len the byte-length of the data.
 *
 * \retval TRUE Success.
 *
 * \note we only receive the data and make the interface available to FWK.
 *       For now, this SUBCMD woundn't be sent from the FWK currently.
 */
/*----------------------------------------------------------------------------*/
int mtk_cfg80211_vendor_enable_roaming(struct wiphy *wiphy,
	       struct wireless_dev *wdev, const void *data, int data_len)
{
	struct GLUE_INFO *prGlueInfo = NULL;
	struct nlattr *attr;

	ASSERT(wiphy);	/* change to if (wiphy == NULL) then return? */
	ASSERT(wdev);	/* change to if (wiphy == NULL) then return? */

	prGlueInfo = (struct GLUE_INFO *) wiphy_priv(wiphy);
	if (!prGlueInfo)
		return -EFAULT;

	attr = (struct nlattr *)data;
	if (attr->nla_type == WIFI_ATTRIBUTE_ROAMING_STATE)
		prGlueInfo->u4FWRoamingEnable = nla_get_u32(attr);

	DBGLOG(REQ, INFO, "FWK set FWRoamingEnable = %d\n",
	       prGlueInfo->u4FWRoamingEnable);

	return WLAN_STATUS_SUCCESS;
}

int mtk_cfg80211_vendor_get_rtt_capabilities(
	struct wiphy *wiphy, struct wireless_dev *wdev,
	const void *data, int data_len)
{
	struct GLUE_INFO *prGlueInfo = NULL;
	int32_t i4Status = -EINVAL;
	struct PARAM_WIFI_RTT_CAPABILITIES rRttCapabilities;
	struct sk_buff *skb;

	DBGLOG(REQ, TRACE, "vendor command\n");

	ASSERT(wiphy);
	ASSERT(wdev);
	prGlueInfo = (struct GLUE_INFO *) wiphy_priv(wiphy);

	skb = cfg80211_vendor_cmd_alloc_reply_skb(wiphy,
			sizeof(rRttCapabilities));
	if (!skb) {
		DBGLOG(REQ, ERROR, "%s allocate skb failed:%x\n",
		       __func__, i4Status);
		return -ENOMEM;
	}

	kalMemZero(&rRttCapabilities, sizeof(rRttCapabilities));

	/* RTT Capabilities return from driver not firmware */
	rRttCapabilities.rtt_one_sided_supported = 0;
	rRttCapabilities.rtt_ftm_supported = 1;
	rRttCapabilities.lci_support = 1;
	rRttCapabilities.lcr_support = 1;
	rRttCapabilities.preamble_support = 0x07;
	rRttCapabilities.bw_support = 0x1c;

	if (unlikely(nla_put(skb, RTT_ATTRIBUTE_CAPABILITIES,
			     sizeof(rRttCapabilities), &rRttCapabilities) < 0))
		goto nla_put_failure;

	i4Status = cfg80211_vendor_cmd_reply(skb);
	return i4Status;

nla_put_failure:
	kfree_skb(skb);
	return i4Status;
}

int mtk_cfg80211_vendor_llstats_get_info(
	struct wiphy *wiphy, struct wireless_dev *wdev,
	const void *data, int data_len)
{
	int32_t i4Status = -EINVAL;
	struct WIFI_RADIO_STAT *pRadioStat = NULL;
	struct sk_buff *skb = NULL;
	uint32_t u4BufLen = 0;

	ASSERT(wiphy);
	ASSERT(wdev);

	u4BufLen = sizeof(struct WIFI_RADIO_STAT) + sizeof(
			   struct WIFI_IFACE_STAT);
	pRadioStat = kalMemAlloc(u4BufLen, VIR_MEM_TYPE);
	if (!pRadioStat) {
		DBGLOG(REQ, ERROR, "%s kalMemAlloc pRadioStat failed\n",
		       __func__);
		i4Status = -ENOMEM;
		goto nla_put_failure;
	}
	kalMemZero(pRadioStat, u4BufLen);

	skb = cfg80211_vendor_cmd_alloc_reply_skb(wiphy, u4BufLen);
	if (!skb) {
		DBGLOG(REQ, TRACE, "%s allocate skb failed:%x\n", __func__,
		       i4Status);
		i4Status = -ENOMEM;
		goto nla_put_failure;
	}

#if 0
	rStatus = kalIoctl(prGlueInfo,
			   wlanoidQueryStatistics,
			   &rRadioStat,
			   sizeof(rRadioStat),
			   TRUE,
			   TRUE,
			   TRUE,
			   TRUE,
			   &u4BufLen);
#endif
	/* only for test */
	pRadioStat->radio = 10;
	pRadioStat->on_time = 11;
	pRadioStat->tx_time = 12;
	pRadioStat->num_channels = 4;

	/*NLA_PUT(skb, LSTATS_ATTRIBUTE_STATS, u4BufLen, pRadioStat);*/
	if (unlikely(nla_put(skb, LSTATS_ATTRIBUTE_STATS, u4BufLen,
			     pRadioStat) < 0))
		goto nla_put_failure;

	i4Status = cfg80211_vendor_cmd_reply(skb);
	kalMemFree(pRadioStat, VIR_MEM_TYPE, u4BufLen);
	if (i4Status != 0)
		DBGLOG(REQ, TRACE, "skb reply error:%x\n",
		       i4Status);
	return -1; /* not support LLS now*/
	/* return i4Status; */

nla_put_failure:
	if (skb != NULL)
		kfree_skb(skb);
	if (pRadioStat != NULL)
		kalMemFree(pRadioStat, VIR_MEM_TYPE, u4BufLen);
	return i4Status;
}

int mtk_cfg80211_vendor_set_band(struct wiphy *wiphy,
				 struct wireless_dev *wdev,
				 const void *data, int data_len)
{
	struct GLUE_INFO *prGlueInfo = NULL;
	struct nlattr *attr;
	uint8_t setBand = 0;
	enum ENUM_BAND band;

	ASSERT(wiphy);
	ASSERT(wdev);

	DBGLOG(REQ, INFO, "%s()\n", __func__);

	if ((data == NULL) || !data_len)
		goto nla_put_failure;

	DBGLOG(REQ, TRACE,
	       "vendor command: data_len=%d, data=0x%x 0x%x\n",
	       data_len, *((uint32_t *) data), *((uint32_t *) data + 1));

	attr = (struct nlattr *)data;
	setBand = nla_get_u32(attr);
	prGlueInfo = (struct GLUE_INFO *) wiphy_priv(wiphy);
	ASSERT(prGlueInfo);

	DBGLOG(REQ, INFO, "Vendor Set Band value=%d\n", setBand);

	if (setBand == QCA_SETBAND_5G)
		band = BAND_5G;
	else if (setBand == QCA_SETBAND_2G)
		band = BAND_2G4;
	else
		band = BAND_NULL;

	prGlueInfo->prAdapter->aePreferBand[NETWORK_TYPE_AIS] =
		band;
	return 0;

nla_put_failure:
	return -1;
}

#if CFG_SUPPORT_MBO
const struct nla_policy
qca_roaming_param_policy[QCA_ATTR_ROAMING_PARAM_MAX + 1] = {
	[QCA_ATTR_ROAMING_SUBCMD] = {.type = NLA_U32},
	[QCA_ATTR_ROAMING_REQ_ID] = {.type = NLA_U32},
	[QCA_ATTR_ROAMING_PARAM_SET_BSSID_PARAMS_NUM_BSSID] = {.type = NLA_U32},
	[QCA_ATTR_ROAMING_PARAM_SET_BSSID_PARAMS] = {.type = NLA_NESTED},
	[QCA_ATTR_ROAMING_PARAM_SET_BSSID_PARAMS_BSSID] = {
					.type = NLA_BINARY,
					.len = MAC_ADDR_LEN},
};

#define SET_BSSID_PARAMS_NUM_BSSID \
		QCA_ATTR_ROAMING_PARAM_SET_BSSID_PARAMS_NUM_BSSID
#define SET_BSSID_PARAMS \
		QCA_ATTR_ROAMING_PARAM_SET_BSSID_PARAMS
#define SET_BSSID_PARAMS_BSSID \
		QCA_ATTR_ROAMING_PARAM_SET_BSSID_PARAMS_BSSID

int mtk_cfg80211_vendor_set_roaming_param(struct wiphy *wiphy,
				 struct wireless_dev *wdev,
				 const void *data, int data_len)
{
	struct GLUE_INFO *prGlueInfo = NULL;
	struct nlattr *tb[QCA_ATTR_ROAMING_PARAM_MAX + 1];
	struct nlattr *tb2[QCA_ATTR_ROAMING_PARAM_MAX + 1];
	struct nlattr *attr;
	uint32_t rStatus, u4BufLen, cmd_type, count, index;
	int tmp;
	uint8_t i = 0;

	ASSERT(wiphy);
	ASSERT(wdev);

	prGlueInfo = (struct GLUE_INFO *) wiphy_priv(wiphy);
	if (!prGlueInfo)
		return -EFAULT;

	if ((data == NULL) || (data_len == 0))
		goto fail;

	if (NLA_PARSE(tb, QCA_ATTR_ROAMING_PARAM_MAX,
			data, data_len, qca_roaming_param_policy)) {
		DBGLOG(REQ, ERROR, "Wrong ROAM ATTR.\n");
		goto fail;
	}

	/* Parse and fetch Command Type*/
	if (!tb[QCA_ATTR_ROAMING_SUBCMD]) {
		DBGLOG(REQ, ERROR, "Invalid roam cmd type\n");
		goto fail;
	}

	cmd_type = nla_get_u32(tb[QCA_ATTR_ROAMING_SUBCMD]);
	if (cmd_type == QCA_ATTR_ROAM_SUBCMD_SET_BLACKLIST_BSSID) {
		struct PARAM_BSS_DISALLOWED_LIST request = {};

		/* Parse and fetch number of blacklist BSSID */
		if (!tb[SET_BSSID_PARAMS_NUM_BSSID]) {
			DBGLOG(REQ, ERROR, "Invlaid num of blacklist bssid\n");
			goto fail;
		}
		count = nla_get_u32(tb[SET_BSSID_PARAMS_NUM_BSSID]);
		if (count > MAX_FW_ROAMING_BLACKLIST_SIZE) {
			DBGLOG(REQ, ERROR, "Count %u exceeds\n", count);
			goto fail;
		}
		request.u4NumBssDisallowed = count;
		i = 0;
		if (count && tb[SET_BSSID_PARAMS]) {
			nla_for_each_nested(attr, tb[SET_BSSID_PARAMS], tmp) {
				char *bssid = NULL;

				if (i == count) {
					DBGLOG(REQ, ERROR, "Excess num\n");
					break;
				}
				if (NLA_PARSE(tb2,
					QCA_ATTR_ROAMING_PARAM_MAX,
					nla_data(attr), nla_len(attr),
					qca_roaming_param_policy)) {
					DBGLOG(REQ, ERROR, "Wrong ROAM ATTR\n");
					goto fail;
				}
				/* Parse and fetch MAC address */
				if (!tb2[SET_BSSID_PARAMS_BSSID]) {
					DBGLOG(REQ, ERROR, "addr failed\n");
					goto fail;
				}
				bssid = nla_data(tb2[SET_BSSID_PARAMS_BSSID]);
				index = i * MAC_ADDR_LEN;
				COPY_MAC_ADDR(&request.aucList[index], bssid);
				DBGLOG(REQ, INFO, "disallow #%d " MACSTR "\n",
					i, MAC2STR(bssid));
				i++;
			}
		}
		if (i != count)
			DBGLOG(REQ, ERROR, "Count %u, expected %u\n", i, count);

		rStatus = kalIoctl(prGlueInfo, wlanoidBssDisallowedList,
				   &request,
				   sizeof(struct PARAM_BSS_DISALLOWED_LIST),
				   FALSE, FALSE, FALSE, &u4BufLen);

		if (rStatus != WLAN_STATUS_SUCCESS) {
			DBGLOG(REQ, ERROR, "disallowed error:%x\n", rStatus);
			return -EFAULT;
		}
	} else {
		DBGLOG(REQ, INFO, "unhandled cmd_type %d\n", cmd_type);
		goto fail;
	}

	return WLAN_STATUS_SUCCESS;
fail:
	return -EINVAL;
}

#undef SET_BSSID_PARAMS_NUM_BSSID
#undef SET_BSSID_PARAMS
#undef SET_BSSID_PARAMS_BSSID

#endif

int mtk_cfg80211_vendor_set_roaming_policy(
	struct wiphy *wiphy, struct wireless_dev *wdev,
	const void *data, int data_len)
{
	struct GLUE_INFO *prGlueInfo = NULL;
	uint32_t rStatus = WLAN_STATUS_SUCCESS;
	struct nlattr *attr;
	uint32_t setRoaming = 0;
	uint32_t u4BufLen = 0;
	int32_t i4Status = -EINVAL;

	ASSERT(wiphy);
	ASSERT(wdev);

	if ((data == NULL) || !data_len)
		goto nla_put_failure;

	attr = (struct nlattr *)data;
	setRoaming = nla_get_u32(attr);
	prGlueInfo = (struct GLUE_INFO *) wiphy_priv(wiphy);
	ASSERT(prGlueInfo);

	DBGLOG(REQ, INFO,
	       "vendor command: data_len=%d, data=0x%x 0x%x, roaming policy=%d\n",
	       data_len, *((uint32_t *) data), *((uint32_t *) data + 1),
	       setRoaming);

	rStatus = kalIoctl(prGlueInfo,
			   wlanoidSetDrvRoamingPolicy,
			   &setRoaming, sizeof(uint32_t), FALSE, FALSE, TRUE,
			   &u4BufLen);

	return rStatus;

nla_put_failure:
	return i4Status;

}

int mtk_cfg80211_vendor_set_rssi_monitoring(
	struct wiphy *wiphy, struct wireless_dev *wdev,
	const void *data, int data_len)
{
	uint32_t rStatus = WLAN_STATUS_SUCCESS;
	uint32_t u4BufLen = 0;
	struct GLUE_INFO *prGlueInfo = NULL;

	int32_t i4Status = -EINVAL;
	struct PARAM_RSSI_MONITOR_T rRSSIMonitor;
	struct nlattr *attr[WIFI_ATTRIBUTE_RSSI_MONITOR_START + 1];
	uint32_t i = 0;

	ASSERT(wiphy);
	ASSERT(wdev);

	DBGLOG(REQ, TRACE, "vendor command: data_len=%d\n",
	       data_len);
	kalMemZero(&rRSSIMonitor,
		   sizeof(struct PARAM_RSSI_MONITOR_T));
	if ((data == NULL) || !data_len)
		goto nla_put_failure;
	kalMemZero(attr, sizeof(struct nlattr *) *
		   (WIFI_ATTRIBUTE_RSSI_MONITOR_START + 1));
	if (NLA_PARSE_NESTED(attr,
			     WIFI_ATTRIBUTE_RSSI_MONITOR_START,
			     (struct nlattr *)(data - NLA_HDRLEN),
#if KERNEL_VERSION(5, 4, 0) <= LINUX_VERSION_CODE
				 nla_parse_wifi_rssi_monitor) < 0) {
#else
				 nla_parse_wifi_attribute) < 0) {
#endif
		DBGLOG(REQ, ERROR, "%s nla_parse_nested failed\n",
		       __func__);
		goto nla_put_failure;
	}

	for (i = WIFI_ATTRIBUTE_RSSI_MONITOR_MAX_RSSI;
	     i <= WIFI_ATTRIBUTE_RSSI_MONITOR_START; i++) {
		if (attr[i]) {
			switch (i) {
			case WIFI_ATTRIBUTE_RSSI_MONITOR_MAX_RSSI:
				rRSSIMonitor.max_rssi_value =
					nla_get_u32(attr[i]);
				break;
			case WIFI_ATTRIBUTE_RSSI_MONITOR_MIN_RSSI:
				rRSSIMonitor.min_rssi_value
					= nla_get_u32(attr[i]);
				break;
			case WIFI_ATTRIBUTE_RSSI_MONITOR_START:
				rRSSIMonitor.enable = nla_get_u32(attr[i]);
				break;
			}
		}
	}

	DBGLOG(REQ, TRACE,
	       "mMax_rssi=%d, mMin_rssi=%d enable=%d\n",
	       rRSSIMonitor.max_rssi_value, rRSSIMonitor.min_rssi_value,
	       rRSSIMonitor.enable);

	prGlueInfo = (struct GLUE_INFO *) wiphy_priv(wiphy);
	ASSERT(prGlueInfo);

	rStatus = kalIoctl(prGlueInfo,
			   wlanoidRssiMonitor,
			   &rRSSIMonitor, sizeof(struct PARAM_RSSI_MONITOR_T),
			   FALSE, FALSE, TRUE, &u4BufLen);
	return rStatus;

nla_put_failure:
	return i4Status;
}

int mtk_cfg80211_vendor_packet_keep_alive_start(
	struct wiphy *wiphy, struct wireless_dev *wdev,
	const void *data, int data_len)
{
	struct GLUE_INFO *prGlueInfo = NULL;
	struct PARAM_PACKET_KEEPALIVE_T *prPkt = NULL;
	struct nlattr *attr[MKEEP_ALIVE_ATTRIBUTE_MAX];
	uint32_t rStatus = WLAN_STATUS_SUCCESS;
	uint16_t u2IpPktLen = 0;
	uint32_t u4BufLen = 0;
	uint8_t ucBssIndex = 0, ucIdx = 0;
	int32_t i4Status = -EINVAL;

	ASSERT(wiphy);
	ASSERT(wdev);
	if ((data == NULL) || !data_len)
		goto nla_put_failure;

	ucBssIndex = wlanGetBssIdx(wdev->netdev);
	DBGLOG(REQ, TRACE, "vendor command: data_len=%d\n",
	       data_len);
	prPkt = (struct PARAM_PACKET_KEEPALIVE_T *)
		kalMemAlloc(sizeof(struct PARAM_PACKET_KEEPALIVE_T),
			    VIR_MEM_TYPE);
	if (!prPkt) {
		DBGLOG(REQ, ERROR,
		       "Can not alloc memory for struct PARAM_PACKET_KEEPALIVE_T\n");
		return -ENOMEM;
	}
	kalMemZero(prPkt, sizeof(struct PARAM_PACKET_KEEPALIVE_T));
	kalMemZero(attr, sizeof(struct nlattr *) * (MKEEP_ALIVE_ATTRIBUTE_MAX));

	prPkt->fgEnable = TRUE; /*start packet keep alive*/
	prPkt->reserved[0] = ucBssIndex;
	if (NLA_PARSE_NESTED(attr,
			     MKEEP_ALIVE_ATTRIBUTE_PERIOD_MSEC,
			     (struct nlattr *)(data - NLA_HDRLEN),
			     nla_parse_offloading_policy) < 0) {
		DBGLOG(REQ, ERROR, "%s nla_parse_nested failed\n",
		       __func__);
		goto nla_put_failure;
	}

	for (ucIdx = MKEEP_ALIVE_ATTRIBUTE_ID;
	     ucIdx <= MKEEP_ALIVE_ATTRIBUTE_PERIOD_MSEC; ucIdx++) {
		if (attr[ucIdx]) {
			switch (ucIdx) {
			case MKEEP_ALIVE_ATTRIBUTE_ID:
				prPkt->index = nla_get_u8(attr[ucIdx]);
				break;
			case MKEEP_ALIVE_ATTRIBUTE_IP_PKT_LEN:
				prPkt->u2IpPktLen = nla_get_u16(attr[ucIdx]);
				break;
			case MKEEP_ALIVE_ATTRIBUTE_IP_PKT:
				u2IpPktLen = prPkt->u2IpPktLen <= 256
					? prPkt->u2IpPktLen : 256;
				kalMemCopy(prPkt->pIpPkt, nla_data(attr[ucIdx]),
					u2IpPktLen);
				break;
			case MKEEP_ALIVE_ATTRIBUTE_SRC_MAC_ADDR:
				kalMemCopy(prPkt->ucSrcMacAddr,
				   nla_data(attr[ucIdx]), sizeof(uint8_t) * 6);
				break;
			case MKEEP_ALIVE_ATTRIBUTE_DST_MAC_ADDR:
				kalMemCopy(prPkt->ucDstMacAddr,
				   nla_data(attr[ucIdx]), sizeof(uint8_t) * 6);
				break;
			case MKEEP_ALIVE_ATTRIBUTE_PERIOD_MSEC:
				prPkt->u4PeriodMsec = nla_get_u32(attr[ucIdx]);
				break;
			}
		}
	}

	DBGLOG(REQ, INFO,
		"BssIdx=%d fgEnable=%d, index=%d, u2IpPktLen=%d u4PeriodMsec=%d\n",
		prPkt->reserved[0], prPkt->fgEnable, prPkt->index,
		prPkt->u2IpPktLen, prPkt->u4PeriodMsec);
	DBGLOG(REQ, TRACE, "prPkt->pIpPkt=0x%02x%02x%02x%02x\n",
		prPkt->pIpPkt[0], prPkt->pIpPkt[1],
		prPkt->pIpPkt[2], prPkt->pIpPkt[3]);
	DBGLOG(REQ, TRACE, "%02x%02x%02x%02x, %02x%02x%02x%02x\n",
		prPkt->pIpPkt[4], prPkt->pIpPkt[5],
		prPkt->pIpPkt[6], prPkt->pIpPkt[7],
		prPkt->pIpPkt[8], prPkt->pIpPkt[9],
		prPkt->pIpPkt[10], prPkt->pIpPkt[11]);
	DBGLOG(REQ, TRACE, "%02x%02x%02x%02x\n",
		prPkt->pIpPkt[12], prPkt->pIpPkt[13],
		prPkt->pIpPkt[14], prPkt->pIpPkt[15]);
	DBGLOG(REQ, TRACE,
		"prPkt->srcMAC=%02x:%02x:%02x:%02x:%02x:%02x\n",
		prPkt->ucSrcMacAddr[0], prPkt->ucSrcMacAddr[1],
		prPkt->ucSrcMacAddr[2], prPkt->ucSrcMacAddr[3],
		prPkt->ucSrcMacAddr[4], prPkt->ucSrcMacAddr[5]);
	DBGLOG(REQ, TRACE, "dstMAC=%02x:%02x:%02x:%02x:%02x:%02x\n",
		prPkt->ucDstMacAddr[0], prPkt->ucDstMacAddr[1],
		prPkt->ucDstMacAddr[2], prPkt->ucDstMacAddr[3],
		prPkt->ucDstMacAddr[4], prPkt->ucDstMacAddr[5]);

	prGlueInfo = (struct GLUE_INFO *) wiphy_priv(wiphy);
	ASSERT(prGlueInfo);

	rStatus = kalIoctl(prGlueInfo,
			   wlanoidPacketKeepAlive,
			   prPkt, sizeof(struct PARAM_PACKET_KEEPALIVE_T),
			   FALSE, FALSE, TRUE, &u4BufLen);
	kalMemFree(prPkt, VIR_MEM_TYPE,
		   sizeof(struct PARAM_PACKET_KEEPALIVE_T));
	return rStatus;

nla_put_failure:
	if (prPkt != NULL)
		kalMemFree(prPkt, VIR_MEM_TYPE,
			   sizeof(struct PARAM_PACKET_KEEPALIVE_T));
	return i4Status;
}

int mtk_cfg80211_vendor_packet_keep_alive_stop(
	struct wiphy *wiphy, struct wireless_dev *wdev,
	const void *data, int data_len)
{
	uint32_t rStatus = WLAN_STATUS_SUCCESS;
	uint32_t u4BufLen = 0;
	struct GLUE_INFO *prGlueInfo = NULL;

	int32_t i4Status = -EINVAL;
	struct PARAM_PACKET_KEEPALIVE_T *prPkt = NULL;
	struct nlattr *attr;

	ASSERT(wiphy);
	ASSERT(wdev);
	if ((data == NULL) || !data_len)
		goto nla_put_failure;

	DBGLOG(REQ, TRACE, "vendor command: data_len=%d\n",
	       data_len);
	prPkt = (struct PARAM_PACKET_KEEPALIVE_T *)
		kalMemAlloc(sizeof(struct PARAM_PACKET_KEEPALIVE_T),
			    VIR_MEM_TYPE);
	if (!prPkt) {
		DBGLOG(REQ, ERROR,
		       "Can not alloc memory for PARAM_PACKET_KEEPALIVE_T\n");
		return -ENOMEM;
	}
	kalMemZero(prPkt, sizeof(struct PARAM_PACKET_KEEPALIVE_T));

	prPkt->fgEnable = FALSE;  /*stop packet keep alive*/
	attr = (struct nlattr *)data;
	if (attr->nla_type == MKEEP_ALIVE_ATTRIBUTE_ID)
		prPkt->index = nla_get_u8(attr);

	DBGLOG(REQ, INFO, "fgEnable=%d, index=%d\n",
	       prPkt->fgEnable, prPkt->index);

	prGlueInfo = (struct GLUE_INFO *) wiphy_priv(wiphy);
	ASSERT(prGlueInfo);

	rStatus = kalIoctl(prGlueInfo,
			   wlanoidPacketKeepAlive,
			   prPkt, sizeof(struct PARAM_PACKET_KEEPALIVE_T),
			   FALSE, FALSE, TRUE, &u4BufLen);
	kalMemFree(prPkt, VIR_MEM_TYPE,
		   sizeof(struct PARAM_PACKET_KEEPALIVE_T));
	return rStatus;

nla_put_failure:
	if (prPkt != NULL)
		kalMemFree(prPkt, VIR_MEM_TYPE,
			   sizeof(struct PARAM_PACKET_KEEPALIVE_T));
	return i4Status;
}

int mtk_cfg80211_vendor_get_version(struct wiphy *wiphy,
				    struct wireless_dev *wdev,
				    const void *data, int data_len)
{
#define STR(x) STR_HELPER(x)
	struct GLUE_INFO *prGlueInfo = NULL;
	struct sk_buff *skb = NULL;
	struct nlattr *attrlist = NULL;
	char aucVersionBuf[256];
	uint16_t u2CopySize = 0;
	uint16_t u2Len = 0;

	ASSERT(wiphy);
	ASSERT(wdev);

	if ((data == NULL) || !data_len)
		return -ENOMEM;

	kalMemZero(aucVersionBuf, 256);
	attrlist = (struct nlattr *)((uint8_t *) data);
	if (attrlist->nla_type == LOGGER_ATTRIBUTE_DRIVER_VER) {
		char aucDriverVersionStr[] = STR(NIC_DRIVER_MAJOR_VERSION) "_"
					     STR(NIC_DRIVER_MINOR_VERSION) "_"
					     STR(NIC_DRIVER_SERIAL_VERSION) "-"
					     STR(DRIVER_BUILD_DATE);

		u2Len = kalStrLen(aucDriverVersionStr);
		DBGLOG(REQ, TRACE, "Get driver version len: %d\n", u2Len);
		u2CopySize = (u2Len >= 256) ? 255 : u2Len;
		if (u2CopySize > 0)
			kalMemCopy(aucVersionBuf, &aucDriverVersionStr[0],
				u2CopySize);
	} else if (attrlist->nla_type == LOGGER_ATTRIBUTE_FW_VER) {
		struct ADAPTER *prAdapter;

		prGlueInfo = (struct GLUE_INFO *) wiphy_priv(wiphy);
		ASSERT(prGlueInfo);
		prAdapter = prGlueInfo->prAdapter;
		if (prAdapter) {
			u2Len = kalStrLen(
					prAdapter->rVerInfo.aucReleaseManifest);
			DBGLOG(REQ, TRACE,
				"Get FW manifest version len: %d\n", u2Len);
			u2CopySize = (u2Len >= 256) ? 255 : u2Len;
			if (u2CopySize > 0)
				kalMemCopy(aucVersionBuf,
					prAdapter->rVerInfo.aucReleaseManifest,
					u2CopySize);
		}
	}

	DBGLOG(REQ, TRACE, "Get version(%d)=[%s]\n", u2CopySize, aucVersionBuf);

	if (u2CopySize == 0)
		return -EFAULT;

	skb = cfg80211_vendor_cmd_alloc_reply_skb(wiphy, u2CopySize);
	if (!skb) {
		DBGLOG(REQ, ERROR, "Allocate skb failed\n");
		return -ENOMEM;
	}

	if (unlikely(nla_put_nohdr(skb, u2CopySize, &aucVersionBuf[0]) < 0))
		goto nla_put_failure;

	return cfg80211_vendor_cmd_reply(skb);

nla_put_failure:
	kfree_skb(skb);
	return -EFAULT;
}

int mtk_cfg80211_vendor_get_supported_feature_set(struct wiphy *wiphy,
		struct wireless_dev *wdev, const void *data, int data_len)
{
	uint64_t u8FeatureSet;
	struct GLUE_INFO *prGlueInfo;
	struct sk_buff *skb;

	ASSERT(wiphy);
	ASSERT(wdev);

	prGlueInfo = wlanGetGlueInfo();
	if (!prGlueInfo)
		return -EFAULT;

	u8FeatureSet = wlanGetSupportedFeatureSet(prGlueInfo);

	skb = cfg80211_vendor_cmd_alloc_reply_skb(wiphy, sizeof(u8FeatureSet));
	if (!skb) {
		DBGLOG(REQ, ERROR, "Allocate skb failed\n");
		return -ENOMEM;
	}

	if (unlikely(
	    nla_put_nohdr(skb, sizeof(u8FeatureSet), &u8FeatureSet) < 0)) {
		DBGLOG(REQ, ERROR, "nla_put_nohdr failed\n");
		goto nla_put_failure;
	}

	DBGLOG(REQ, TRACE, "supported feature set=0x%llx\n", u8FeatureSet);

	return cfg80211_vendor_cmd_reply(skb);

nla_put_failure:
	kfree_skb(skb);
	return -EFAULT;
}

int mtk_cfg80211_vendor_event_rssi_beyond_range(
	struct wiphy *wiphy, struct wireless_dev *wdev, int rssi)
{
	struct sk_buff *skb;
	struct PARAM_RSSI_MONITOR_EVENT rRSSIEvt;
	struct BSS_INFO *prAisBssInfo;
	struct GLUE_INFO *prGlueInfo = NULL;
	struct ADAPTER *prAdapter;
	uint8_t ucBssIndex = AIS_DEFAULT_INDEX;

	ASSERT(wiphy);
	ASSERT(wdev);

	prGlueInfo = (struct GLUE_INFO *) wiphy_priv(wiphy);
	ASSERT(prGlueInfo);

	DBGLOG(REQ, TRACE, "vendor command rssi=%d\n", rssi);
	kalMemZero(&rRSSIEvt,
		   sizeof(struct PARAM_RSSI_MONITOR_EVENT));

#if KERNEL_VERSION(4, 4, 0) <= LINUX_VERSION_CODE
	skb = cfg80211_vendor_event_alloc(wiphy, wdev,
				  sizeof(struct PARAM_RSSI_MONITOR_EVENT),
				  WIFI_EVENT_RSSI_MONITOR, GFP_KERNEL);
#else
	skb = cfg80211_vendor_event_alloc(wiphy,
				  sizeof(struct PARAM_RSSI_MONITOR_EVENT),
				  WIFI_EVENT_RSSI_MONITOR, GFP_KERNEL);
#endif /* KERNEL_VERSION(4, 4, 0) <= LINUX_VERSION_CODE */

	if (!skb) {
		DBGLOG(REQ, ERROR, "%s allocate skb failed\n", __func__);
		return -ENOMEM;
	}

	prAdapter = prGlueInfo->prAdapter;
	prAisBssInfo = aisGetAisBssInfo(prAdapter, ucBssIndex);
	kalMemCopy(rRSSIEvt.BSSID, prAisBssInfo->aucBSSID,
		   sizeof(uint8_t) * MAC_ADDR_LEN);

	rRSSIEvt.version = 1; /* RSSI_MONITOR_EVT_VERSION = 1 */
	if (rssi > PARAM_WHQL_RSSI_MAX_DBM)
		rssi = PARAM_WHQL_RSSI_MAX_DBM;
	else if (rssi < -120)
		rssi = -120;
	rRSSIEvt.rssi = (int8_t)rssi;
	DBGLOG(REQ, INFO,
	       "RSSI Event: version=%d, rssi=%d, BSSID=" MACSTR "\n",
	       rRSSIEvt.version, rRSSIEvt.rssi, MAC2STR(rRSSIEvt.BSSID));

	/*NLA_PUT_U32(skb, GOOGLE_RSSI_MONITOR_EVENT, rssi);*/
	{
		/* unsigned int __tmp = rssi; */

		if (unlikely(nla_put(skb, WIFI_EVENT_RSSI_MONITOR,
				     sizeof(struct PARAM_RSSI_MONITOR_EVENT),
				     &rRSSIEvt) < 0))
			goto nla_put_failure;
	}

	cfg80211_vendor_event(skb, GFP_KERNEL);
	return 0;

nla_put_failure:
	kfree_skb(skb);
	return -ENOMEM;
}

int mtk_cfg80211_vendor_set_tx_power_scenario(struct wiphy *wiphy,
		struct wireless_dev *wdev, const void *data, int data_len)
{
#if CFG_SUPPORT_DYNAMIC_PWR_LIMIT
	struct PARAM_TX_PWR_CTRL_IOCTL rPwrCtrlParam = { 0 };
	struct GLUE_INFO *prGlueInfo;
	struct nlattr *attr;
	struct sk_buff *skb;
	uint32_t u4Scenario;
	uint32_t rStatus = WLAN_STATUS_SUCCESS;
	uint32_t u4SetInfoLen = 0;
	uint8_t index = 0;
	char name[] = { "_G_Scenario" };

	ASSERT(wiphy);
	ASSERT(wdev);

#if CFG_ENABLE_UNIFY_WIPHY
	prGlueInfo = (struct GLUE_INFO *) wiphy_priv(wiphy);
#else	/* CFG_ENABLE_UNIFY_WIPHY */
	if (wdev == gprWdev)	/* wlan0 */
		prGlueInfo = (struct GLUE_INFO *) wiphy_priv(wiphy);
	else
		prGlueInfo = *((struct GLUE_INFO **) wiphy_priv(wiphy));
#endif	/* CFG_ENABLE_UNIFY_WIPHY */

	if (!prGlueInfo)
		return -EFAULT;

	attr = (struct nlattr *)data;
	if (attr->nla_type == WIFI_ATTRIBUTE_TX_POWER_SCENARIO)
		u4Scenario = nla_get_u32(attr);
	else
		return -EINVAL;

	if (u4Scenario == UINT_MAX) {
		index = 0;
	} else if ((u4Scenario >= 0) && (u4Scenario <= 4)) {
		index = u4Scenario + 1;
	} else {
		DBGLOG(REQ, ERROR, "invalid scenario index: %u\n", u4Scenario);
		return -EINVAL;
	}

	rPwrCtrlParam.fgApplied = (index == 0) ? FALSE : TRUE;
	rPwrCtrlParam.name = name;
	rPwrCtrlParam.index = index;

	DBGLOG(REQ, INFO,
	       "applied=[%d], name=[%s], index=[%u], setting=[%s], UINT_MAX=[%u], iftype=[%d]\n",
	       rPwrCtrlParam.fgApplied,
	       rPwrCtrlParam.name,
	       rPwrCtrlParam.index,
	       rPwrCtrlParam.newSetting,
	       UINT_MAX,
	       wdev->iftype);

	rStatus = kalIoctl(prGlueInfo,
		 wlanoidTxPowerControl,
		 (void *)&rPwrCtrlParam,
		 sizeof(struct PARAM_TX_PWR_CTRL_IOCTL),
		 FALSE,
		 FALSE,
		 TRUE,
		 &u4SetInfoLen);

	skb = cfg80211_vendor_cmd_alloc_reply_skb(wiphy, sizeof(rStatus));
	if (!skb) {
		DBGLOG(REQ, ERROR, "Allocate skb failed\n");
		return -ENOMEM;
	}

	if (unlikely(
	    nla_put_nohdr(skb, sizeof(rStatus), &rStatus) < 0)) {
		DBGLOG(REQ, ERROR, "nla_put_nohdr failed\n");
		goto errHandleLabel;
	}

	DBGLOG(REQ, INFO, "rStatus=0x%x\n", rStatus);

	return cfg80211_vendor_cmd_reply(skb);

errHandleLabel:
	kfree_skb(skb);
#endif
	return -EOPNOTSUPP;
}

int mtk_cfg80211_vendor_get_preferred_freq_list(struct wiphy
		*wiphy, struct wireless_dev *wdev, const void *data,
		int data_len)
{
	struct GLUE_INFO *prGlueInfo;
	struct sk_buff *skb;
	struct nlattr *tb[WIFI_VENDOR_ATTR_PREFERRED_FREQ_LIST_LAST] = {};
	uint32_t freq_list[MAX_CHN_NUM] = {};
	uint32_t num_freq_list = 0;
	uint32_t rStatus = WLAN_STATUS_SUCCESS;
	enum CONN_MODE_IFACE_TYPE type;
	enum ENUM_IFTYPE eIftype;
	uint32_t i;

	ASSERT(wiphy);
	ASSERT(wdev);

	if ((data == NULL) || !data_len)
		return -EINVAL;

	prGlueInfo = wlanGetGlueInfo();
	if (!prGlueInfo)
		return -EFAULT;

	if (NLA_PARSE(tb, WIFI_VENDOR_ATTR_PREFERRED_FREQ_LIST_MAX,
			data, data_len, nla_get_preferred_freq_list_policy)) {
		DBGLOG(REQ, ERROR, "Invalid ATTR.\n");
		return -EINVAL;
	}

	type = nla_get_u32(tb[WIFI_VENDOR_ATTR_PREFERRED_FREQ_LIST_IFACE_TYPE]);

	DBGLOG(REQ, INFO, "type: %d\n", type);

	switch (type) {
	case CONN_MODE_IFACE_TYPE_STA:
		eIftype = IFTYPE_STATION;
		break;
	case CONN_MODE_IFACE_TYPE_SAP:
		eIftype = IFTYPE_AP;
		break;
	case CONN_MODE_IFACE_TYPE_P2P_GC:
		eIftype = IFTYPE_P2P_CLIENT;
		break;
	case CONN_MODE_IFACE_TYPE_P2P_GO:
		eIftype = IFTYPE_P2P_GO;
		break;
	default:
		eIftype = IFTYPE_NUM;
		break;
	}

	if (eIftype != IFTYPE_P2P_CLIENT && eIftype != IFTYPE_P2P_GO) {
		DBGLOG(REQ, ERROR, "Only support p2p gc/go type.\n");
		return -EINVAL;
	}

	rStatus = p2pFunGetPreferredFreqList(prGlueInfo->prAdapter, eIftype,
			freq_list, &num_freq_list);
	if (rStatus != WLAN_STATUS_SUCCESS) {
		DBGLOG(REQ, ERROR, "get preferred freq list failed.\n");
		return -EINVAL;
	}

	DBGLOG(P2P, INFO, "num. of preferred freq list = %d\n", num_freq_list);
	for (i = 0; i < num_freq_list; i++)
		DBGLOG(P2P, INFO, "dump preferred freq list[%d] = %d\n",
			i, freq_list[i]);

	skb = cfg80211_vendor_cmd_alloc_reply_skb(wiphy, sizeof(u32) +
			sizeof(uint32_t) * num_freq_list + NLMSG_HDRLEN);
	if (!skb) {
		DBGLOG(REQ, ERROR, "Allocate skb failed.\n");
		return -ENOMEM;
	}

	if (unlikely(nla_put_u32(skb,
			WIFI_VENDOR_ATTR_PREFERRED_FREQ_LIST_IFACE_TYPE,
			type) < 0)) {
		DBGLOG(REQ, ERROR, "put iface into skb failed.\n");
		goto nla_put_failure;
	}

	if (unlikely(nla_put(skb, WIFI_VENDOR_ATTR_PREFERRED_FREQ_LIST_GET,
			sizeof(uint32_t) * num_freq_list, freq_list) < 0)) {
		DBGLOG(REQ, ERROR, "put freq list into skb failed.\n");
		goto nla_put_failure;
	}

	return cfg80211_vendor_cmd_reply(skb);

nla_put_failure:
	kfree_skb(skb);
	return -EFAULT;
}

int mtk_cfg80211_vendor_acs(struct wiphy *wiphy,
		struct wireless_dev *wdev, const void *data, int data_len)
{
	struct GLUE_INFO *prGlueInfo;
	struct nlattr *tb[WIFI_VENDOR_ATTR_ACS_MAX + 1] = {};
	uint32_t rStatus = WLAN_STATUS_SUCCESS;
	bool ht_enabled, ht40_enabled, vht_enabled;
	uint8_t ch_width = 0;
	enum P2P_VENDOR_ACS_HW_MODE hw_mode;
	uint8_t *ch_list = NULL;
	uint8_t ch_list_count = 0;
	uint8_t i;
	uint32_t msg_size;
	struct MSG_P2P_ACS_REQUEST *prMsgAcsRequest;
	struct RF_CHANNEL_INFO *prRfChannelInfo;
	struct sk_buff *reply_skb;
	uint8_t role_idx;

	if (!wiphy || !wdev || !data || !data_len) {
		DBGLOG(REQ, ERROR, "input data null.\n");
		rStatus = -EINVAL;
		goto exit;
	}

#if CFG_ENABLE_UNIFY_WIPHY
	prGlueInfo = (struct GLUE_INFO *) wiphy_priv(wiphy);
#else	/* CFG_ENABLE_UNIFY_WIPHY */
	if (wdev == gprWdev)	/* wlan0 */
		prGlueInfo = (struct GLUE_INFO *) wiphy_priv(wiphy);
	else
		prGlueInfo = *((struct GLUE_INFO **) wiphy_priv(wiphy));
#endif	/* CFG_ENABLE_UNIFY_WIPHY */

	if (!prGlueInfo) {
		DBGLOG(REQ, ERROR, "get glue structure fail.\n");
		rStatus = -EFAULT;
		goto exit;
	}

	if (mtk_Netdev_To_RoleIdx(prGlueInfo, wdev->netdev, &role_idx) < 0) {
		DBGLOG(REQ, ERROR, "get role index fail.\n");
		rStatus = -EFAULT;
		goto exit;
	}

	if (NLA_PARSE(tb, WIFI_VENDOR_ATTR_ACS_MAX, data, data_len,
			nla_get_acs_policy)) {
		DBGLOG(REQ, ERROR, "parse acs attr fail.\n");
		rStatus = -EINVAL;
		goto exit;
	}

	if (!tb[WIFI_VENDOR_ATTR_ACS_HW_MODE]) {
		DBGLOG(REQ, ERROR, "attr hw_mode failed.\n");
		rStatus = -EINVAL;
		goto exit;
	}
	hw_mode = nla_get_u8(tb[WIFI_VENDOR_ATTR_ACS_HW_MODE]);

	if (tb[WIFI_VENDOR_ATTR_ACS_HT_ENABLED])
		ht_enabled =
			nla_get_flag(tb[WIFI_VENDOR_ATTR_ACS_HT_ENABLED]);
	else
		ht_enabled = 0;

	if (tb[WIFI_VENDOR_ATTR_ACS_HT40_ENABLED])
		ht40_enabled =
			nla_get_flag(tb[WIFI_VENDOR_ATTR_ACS_HT40_ENABLED]);
	else
		ht40_enabled = 0;

	if (tb[WIFI_VENDOR_ATTR_ACS_VHT_ENABLED])
		vht_enabled =
			nla_get_flag(tb[WIFI_VENDOR_ATTR_ACS_VHT_ENABLED]);
	else
		vht_enabled = 0;

	if (tb[WIFI_VENDOR_ATTR_ACS_CHWIDTH])
		ch_width = nla_get_u16(tb[WIFI_VENDOR_ATTR_ACS_CHWIDTH]);

	if (tb[WIFI_VENDOR_ATTR_ACS_CH_LIST]) {
		char *tmp = nla_data(tb[WIFI_VENDOR_ATTR_ACS_CH_LIST]);

		ch_list_count = nla_len(tb[WIFI_VENDOR_ATTR_ACS_CH_LIST]);
		if (ch_list_count) {
			ch_list = kalMemAlloc(sizeof(uint8_t) * ch_list_count,
					VIR_MEM_TYPE);
			if (ch_list == NULL) {
				DBGLOG(REQ, ERROR, "allocate ch_list fail.\n");
				rStatus = -ENOMEM;
				goto exit;
			}

			kalMemCopy(ch_list, tmp, ch_list_count);
		}
	} else if (tb[WIFI_VENDOR_ATTR_ACS_FREQ_LIST]) {
		uint32_t *freq =
			nla_data(tb[WIFI_VENDOR_ATTR_ACS_FREQ_LIST]);

		ch_list_count = nla_len(tb[WIFI_VENDOR_ATTR_ACS_FREQ_LIST]) /
				sizeof(uint32_t);
		if (ch_list_count) {
			ch_list = kalMemAlloc(sizeof(uint8_t) * ch_list_count,
					VIR_MEM_TYPE);
			if (ch_list == NULL) {
				DBGLOG(REQ, ERROR, "allocate ch_list fail.\n");
				rStatus = -ENOMEM;
				goto exit;
			}

			for (i = 0; i < ch_list_count; i++)
				ch_list[i] =
					ieee80211_frequency_to_channel(freq[i]);
		}
	}

	if (!ch_list_count) {
		DBGLOG(REQ, ERROR, "channel list count can NOT be 0\n");
		rStatus = -EINVAL;
		goto exit;
	}

	msg_size = sizeof(struct MSG_P2P_ACS_REQUEST) +
			(ch_list_count * sizeof(struct RF_CHANNEL_INFO));

	prMsgAcsRequest = cnmMemAlloc(prGlueInfo->prAdapter,
			RAM_TYPE_MSG, msg_size);

	if (prMsgAcsRequest == NULL) {
		DBGLOG(REQ, ERROR, "allocate msg acs req. fail.\n");
		rStatus = -ENOMEM;
		goto exit;
	}

	kalMemSet(prMsgAcsRequest, 0, msg_size);
	prMsgAcsRequest->rMsgHdr.eMsgId = MID_MNY_P2P_ACS;
	prMsgAcsRequest->ucRoleIdx = role_idx;
	prMsgAcsRequest->fgIsHtEnable = ht_enabled;
	prMsgAcsRequest->fgIsHt40Enable = ht40_enabled;
	prMsgAcsRequest->fgIsVhtEnable = vht_enabled;
	switch (ch_width) {
	case 20:
		prMsgAcsRequest->eChnlBw = MAX_BW_20MHZ;
		break;
	case 40:
		prMsgAcsRequest->eChnlBw = MAX_BW_40MHZ;
		break;
	case 80:
		prMsgAcsRequest->eChnlBw = MAX_BW_80MHZ;
		break;
	case 160:
		prMsgAcsRequest->eChnlBw = MAX_BW_160MHZ;
		break;
	default:
		DBGLOG(REQ, ERROR, "unsupport width: %d.\n", ch_width);
		prMsgAcsRequest->eChnlBw = MAX_BW_UNKNOWN;
		break;
	}
	prMsgAcsRequest->eHwMode = hw_mode;
	prMsgAcsRequest->u4NumChannel = ch_list_count;

	for (i = 0; i < ch_list_count; i++) {
		/* Translate Freq from MHz to channel number. */
		prRfChannelInfo =
			&(prMsgAcsRequest->arChannelListInfo[i]);

		prRfChannelInfo->ucChannelNum = ch_list[i];

		if (prRfChannelInfo->ucChannelNum <= 14)
			prRfChannelInfo->eBand = BAND_2G4;
		else
			prRfChannelInfo->eBand = BAND_5G;

		/* Iteration. */
		prRfChannelInfo++;
	}

	mboxSendMsg(prGlueInfo->prAdapter,
			MBOX_ID_0,
			(struct MSG_HDR *) prMsgAcsRequest,
			MSG_SEND_METHOD_BUF);

exit:
	if (ch_list)
		kalMemFree(ch_list, VIR_MEM_TYPE,
				sizeof(uint8_t) * ch_list_count);
	if (rStatus == WLAN_STATUS_SUCCESS) {
		reply_skb = cfg80211_vendor_cmd_alloc_reply_skb(wiphy,
				NLMSG_HDRLEN);
		if (reply_skb != NULL)
			return cfg80211_vendor_cmd_reply(reply_skb);
	}
	return rStatus;
}

int mtk_cfg80211_vendor_get_features(struct wiphy *wiphy,
		struct wireless_dev *wdev, const void *data, int data_len)
{
	struct sk_buff *reply_skb;
	uint8_t feature_flags[(NUM_VENDOR_FEATURES + 7) / 8] = {0};
	uint8_t i;

	ASSERT(wiphy);
	ASSERT(wdev);

#if CFG_AUTO_CHANNEL_SEL_SUPPORT
	feature_flags[(VENDOR_FEATURE_SUPPORT_HW_MODE_ANY / 8)] |=
			(1 << (VENDOR_FEATURE_SUPPORT_HW_MODE_ANY % 8));
#endif

	for (i = 0; i < ((NUM_VENDOR_FEATURES + 7) / 8); i++) {
		DBGLOG(REQ, INFO, "Dump feature flags[%d]=0x%x.\n", i,
				feature_flags[i]);
	}

	reply_skb = cfg80211_vendor_cmd_alloc_reply_skb(wiphy,
			sizeof(feature_flags) + NLMSG_HDRLEN);

	if (!reply_skb)
		goto nla_put_failure;

	if (nla_put(reply_skb, WIFI_VENDOR_ATTR_FEATURE_FLAGS,
			sizeof(feature_flags), feature_flags))
		goto nla_put_failure;

	return cfg80211_vendor_cmd_reply(reply_skb);

nla_put_failure:
	kfree_skb(reply_skb);
	return -EINVAL;
}

int mtk_cfg80211_vendor_get_apf_capabilities(struct wiphy *wiphy,
	struct wireless_dev *wdev, const void *data, int data_len)
{
	uint32_t aucCapablilities[2] = {APF_VERSION, APF_MAX_PROGRAM_LEN};
	struct sk_buff *skb;

	ASSERT(wiphy);
	ASSERT(wdev);

	skb = cfg80211_vendor_cmd_alloc_reply_skb(wiphy,
		sizeof(aucCapablilities));

	if (!skb) {
		DBGLOG(REQ, ERROR, "Allocate skb failed\n");
		return -ENOMEM;
	}

	if (unlikely(nla_put(skb, APF_ATTRIBUTE_VERSION,
				sizeof(uint32_t), &aucCapablilities[0]) < 0))
		goto nla_put_failure;
	if (unlikely(nla_put(skb, APF_ATTRIBUTE_MAX_LEN,
				sizeof(uint32_t), &aucCapablilities[1]) < 0))
		goto nla_put_failure;

	DBGLOG(REQ, INFO, "apf capability - ver:%d, max program len: %d\n",
		APF_VERSION, APF_MAX_PROGRAM_LEN);

	return cfg80211_vendor_cmd_reply(skb);

nla_put_failure:
	kfree_skb(skb);
	return -EFAULT;
}

int mtk_cfg80211_vendor_driver_memory_dump(struct wiphy *wiphy,
	struct wireless_dev *wdev, const void *data, int data_len)
{
#if CFG_SUPPORT_LINK_QUALITY_MONITOR
	struct LINK_QUALITY_INFO_OUTPUT_DATA {
		uint16_t u2Tag01; /* cur tx rate */
		uint16_t u2Len01;
		uint32_t u4CurTxRate;
		uint16_t u2Tag02; /* tx total count */
		uint16_t u2Len02;
		uint64_t u8TxTotalCount;
		uint16_t u2Tag03; /* tx retry count */
		uint16_t u2Len03;
		uint64_t u8TxRetryCount;
		uint16_t u2Tag04; /* tx fail Count */
		uint16_t u2Len04;
		uint64_t u8TxFailCount;
		uint16_t u2Tag05; /* Rts fail count */
		uint16_t u2Len05;
		uint64_t u8TxRtsFailCount;
		uint16_t u2Tag06; /* Ack fail count */
		uint16_t u2Len06;
		uint64_t u8TxAckFailCount;
		uint16_t u2Tag07; /* cur rx rate */
		uint16_t u2Len07;
		uint32_t u4CurRxRate;
		uint16_t u2Tag08; /* Rx total count */
		uint16_t u2Len08;
		uint64_t u8RxTotalCount;
		uint16_t u2Tag09; /* Rx dup count */
		uint16_t u2Len09;
		uint32_t u4RxDupCount;
		uint16_t u2Tag10; /* Rx err count */
		uint16_t u2Len10;
		uint64_t u8RxErrCount;
		uint16_t u2Tag11; /* Idle slot count */
		uint16_t u2Len11;
		uint64_t u8IdleSlotCount;
	} __packed outputData = {
		.u2Tag01 = 1,  /* tag: 1, cur tx rate */
		.u2Len01 = 4,  /* len: 4, bytes */
		.u2Tag02 = 2,  /* tag: 2, tx total count */
		.u2Len02 = 8,  /* len: 8, bytes */
		.u2Tag03 = 3,  /* tag: 3, tx retry count */
		.u2Len03 = 8,  /* len: 8, bytes */
		.u2Tag04 = 4,  /* tag: 4, tx fail count */
		.u2Len04 = 8,  /* len: 8, bytes */
		.u2Tag05 = 5,  /* tag: 5, tx rts fail count */
		.u2Len05 = 8,  /* len: 8, bytes */
		.u2Tag06 = 6,  /* tag: 6, tx ack fail count */
		.u2Len06 = 8,  /* len: 8, bytes */
		.u2Tag07 = 7,  /* tag: 7, cur rx rate */
		.u2Len07 = 4,  /* len: 4, bytes */
		.u2Tag08 = 8,  /* tag: 8, rx total count */
		.u2Len08 = 8,  /* len: 8, bytes */
		.u2Tag09 = 9,  /* tag: 9, rx dup count */
		.u2Len09 = 4,  /* len: 4, bytes */
		.u2Tag10 = 10, /* tag: 10, rx err count */
		.u2Len10 = 8,  /* len: 8, bytes */
		.u2Tag11 = 11,
		.u2Len11 = 8
	};
	struct PARAM_GET_LINK_QUALITY_INFO rParam;
	struct NETDEV_PRIVATE_GLUE_INFO *prNetDevPrivate;
	struct WIFI_LINK_QUALITY_INFO rLinkQualityInfo;
	struct GLUE_INFO *prGlueInfo;
	uint32_t u4BufLen;
#endif
	struct sk_buff *skb = NULL;
	uint32_t *puBuffer = NULL;
	int32_t i4Status;
	uint16_t u2CopySize = 0;

	ASSERT(wiphy);
	ASSERT(wdev);
#if CFG_SUPPORT_LINK_QUALITY_MONITOR
	prNetDevPrivate =
		(struct NETDEV_PRIVATE_GLUE_INFO *) netdev_priv(wdev->netdev);
	if (!prNetDevPrivate) {
		DBGLOG(REQ, ERROR, "Invalid net device private\n");
		return -EFAULT;
	}
	rParam.ucBssIdx = 0; /* prNetDevPrivate->ucBssIdx; */
	rParam.prLinkQualityInfo = &rLinkQualityInfo;
	prGlueInfo = (struct GLUE_INFO *) wiphy_priv(wiphy);
	i4Status = kalIoctl(prGlueInfo, wlanoidGetLinkQualityInfo,
		 &rParam, sizeof(struct PARAM_GET_LINK_QUALITY_INFO),
		 TRUE, FALSE, FALSE, &u4BufLen);
	if (i4Status != WLAN_STATUS_SUCCESS) {
		DBGLOG(REQ, ERROR, "wlanoidGetLinkQualityInfo error\n");
		goto err_handle_label;
	}

	outputData.u4CurTxRate = rLinkQualityInfo.u4CurTxRate;
	outputData.u8TxTotalCount = rLinkQualityInfo.u8TxTotalCount;
	outputData.u8TxRetryCount = rLinkQualityInfo.u8TxRetryCount;
	outputData.u8TxFailCount = rLinkQualityInfo.u8TxFailCount;
	outputData.u8TxRtsFailCount = rLinkQualityInfo.u8TxRtsFailCount;
	outputData.u8TxAckFailCount = rLinkQualityInfo.u8TxAckFailCount;
	outputData.u4CurRxRate = rLinkQualityInfo.u4CurRxRate;
	outputData.u8RxTotalCount = rLinkQualityInfo.u8RxTotalCount;
	outputData.u4RxDupCount = rLinkQualityInfo.u4RxDupCount;
	outputData.u8RxErrCount = rLinkQualityInfo.u8RxErrCount;
	outputData.u8IdleSlotCount = rLinkQualityInfo.u8IdleSlotCount;

	DBGLOG(REQ, INFO,
	       "LQ: Tx(rate:%u, total:%lu, Rty:%lu, fail:%lu, RTSF:%lu, ACKF:%lu), Rx(rate:%u, total:%lu, dup:%u, error:%lu), Idle:%lu\n",
	       outputData.u4CurTxRate, /* tx rate, current tx link speed */
	       outputData.u8TxTotalCount, /* tx total packages */
	       outputData.u8TxRetryCount, /* tx retry count */
	       outputData.u8TxFailCount, /* tx fail count */
	       outputData.u8TxRtsFailCount, /* tx RTS fail count */
	       outputData.u8TxAckFailCount, /* tx ACK fail count */
	       outputData.u4CurRxRate, /* current rx rate */
	       outputData.u8RxTotalCount, /* rx total packages */
	       outputData.u4RxDupCount, /* rx duplicate package count */
	       outputData.u8RxErrCount, /* rx error count */
	       outputData.u8IdleSlotCount
	);

	u2CopySize = sizeof(struct LINK_QUALITY_INFO_OUTPUT_DATA);
	puBuffer = (uint32_t *)&outputData;
#endif

	skb = cfg80211_vendor_cmd_alloc_reply_skb(wiphy, u2CopySize);
	if (!skb) {
		DBGLOG(REQ, ERROR, "allocate skb failed\n");
		i4Status = -ENOMEM;
		return i4Status;
	}

	if (unlikely(nla_put_nohdr(skb, u2CopySize, puBuffer) < 0)) {
		DBGLOG(REQ, ERROR, "nla_put_nohdr failed: len=%u, ptr=%p\n",
		       u2CopySize, puBuffer);
		i4Status = -EINVAL;
		goto err_handle_label;
	}

	return cfg80211_vendor_cmd_reply(skb);

err_handle_label:
	if (skb != NULL)
		kfree_skb(skb);
	return i4Status;
}

#if CFG_SUPPORT_MAGIC_PKT_VENDOR_EVENT
int mtk_cfg80211_vendor_event_wowlan_magic_pkt(struct wiphy *wiphy,
				struct wireless_dev *wdev, uint32_t num)
{
	struct sk_buff *skb;

	ASSERT(wiphy);
	ASSERT(wdev);

	DBGLOG(REQ, INFO, "%s for vendor command %d\n", __func__, num);

#if KERNEL_VERSION(4, 4, 0) <= LINUX_VERSION_CODE
	skb = cfg80211_vendor_event_alloc(wiphy, wdev, sizeof(num),
			WIFI_EVENT_MAGIC_PACKET_RECEIVED, GFP_KERNEL);
#else
	skb = cfg80211_vendor_event_alloc(wiphy, sizeof(num),
			WIFI_EVENT_MAGIC_PACKET_RECEIVED, GFP_KERNEL);
#endif /* KERNEL_VERSION(4, 4, 0) <= LINUX_VERSION_CODE */

	if (!skb) {
		DBGLOG(REQ, ERROR, "%s allocate skb failed\n", __func__);
		return -ENOMEM;
	}

	/*NLA_PUT_U32(skb, WIFI_EVENT_MAGIC_PACKET_RECEIVED, num);*/
	{
		unsigned int __tmp = num;

		if (unlikely(nla_put(skb, WIFI_EVENT_MAGIC_PACKET_RECEIVED,
			sizeof(unsigned int), &__tmp) < 0))
			goto nla_put_failure;
	}

	cfg80211_vendor_event(skb, GFP_KERNEL);
	DBGLOG(REQ, INFO, "%s for vendor command done\n", __func__);
	return 0;

nla_put_failure:
	kfree_skb(skb);
	DBGLOG(REQ, INFO, "%s nla_put_fail!\n", __func__);
	return -ENOMEM;
}
#endif

#endif /* KERNEL_VERSION(3, 16, 0) <= LINUX_VERSION_CODE */
