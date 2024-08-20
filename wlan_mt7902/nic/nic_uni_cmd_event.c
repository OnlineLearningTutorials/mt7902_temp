/* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause */
/*
 * Copyright (c) 2016 MediaTek Inc.
 */
/*
 *
 */

/*! \file   nic_uni_cmd_event.c
 *    \brief  Callback functions for Command packets.
 *
 *	Various Event packet handlers which will be setup in the callback
 *  function of a command packet.
 */


/*******************************************************************************
 *                         C O M P I L E R   F L A G S
 *******************************************************************************
 */

/*******************************************************************************
 *                    E X T E R N A L   R E F E R E N C E S
 *******************************************************************************
 */
#include "precomp.h"
#include "gl_ate_agent.h"

/*******************************************************************************
 *                              C O N S T A N T S
 *******************************************************************************
 */

/*******************************************************************************
 *                             D A T A   T Y P E S
 *******************************************************************************
 */
typedef uint32_t (*PROCESS_LEGACY_TO_UNI_FUNCTION) (struct ADAPTER *,
	struct WIFI_UNI_SETQUERY_INFO *);

typedef void(*PROCESS_RX_UNI_EVENT_FUNCTION) (struct ADAPTER *,
	struct WIFI_UNI_EVENT *);

/*******************************************************************************
 *                            P U B L I C   D A T A
 *******************************************************************************
 */

static PROCESS_LEGACY_TO_UNI_FUNCTION arUniCmdTable[CMD_ID_END] = {
	[0 ... CMD_ID_END - 1] = NULL,
	[CMD_ID_SCAN_REQ_V2] = nicUniCmdScanReqV2,
	[CMD_ID_SCAN_CANCEL] = nicUniCmdScanCancel,
	[CMD_ID_BSS_ACTIVATE_CTRL] = nicUniCmdBssActivateCtrl,
	[CMD_ID_GET_SET_CUSTOMER_CFG] = nicUniCmdCustomerCfg,
	[CMD_ID_CHIP_CONFIG] = nicUniCmdChipCfg,
	[CMD_ID_REMOVE_STA_RECORD] = nicUniCmdRemoveStaRec,
	[CMD_ID_INDICATE_PM_BSS_CREATED] = nicUniCmdBcnContent,
	[CMD_ID_INDICATE_PM_BSS_ABORT] = nicUniCmdPmDisable,
	[CMD_ID_INDICATE_PM_BSS_CONNECTED] = nicUniCmdPmEnable,
	[CMD_ID_UPDATE_BEACON_CONTENT] = nicUniCmdBcnContent,
	[CMD_ID_SET_BSS_INFO] = nicUniCmdSetBssInfo,
	[CMD_ID_UPDATE_STA_RECORD] = nicUniCmdUpdateStaRec,
	[CMD_ID_CH_PRIVILEGE] = nicUniCmdChPrivilege,
	[CMD_ID_SET_RX_FILTER] = nicUniCmdSetRxFilter,
	[CMD_ID_SET_DBDC_PARMS] = nicUniCmdSetMbmc,
	[CMD_ID_NIC_POWER_CTRL] = nicUniCmdPowerCtrl,
	[CMD_ID_SET_DOMAIN_INFO] = nicUniCmdSetDomain,
	[CMD_ID_LOG_UI_INFO] = nicUniCmdWsysFwLogUI,
	[CMD_ID_BASIC_CONFIG] = nicUniCmdWsysFwBasicConfig,
	[CMD_ID_SET_SUSPEND_MODE] = nicUniCmdSetSuspendMode,
	[CMD_ID_SET_WOWLAN] = nicUniCmdSetWOWLAN,
	[CMD_ID_POWER_SAVE_MODE] = nicUniCmdPowerSaveMode,
	[CMD_ID_LAYER_0_EXT_MAGIC_NUM] = nicUniCmdExtCommon,
	[CMD_ID_UPDATE_WMM_PARMS] = nicUniCmdUpdateEdcaSet,
	[CMD_ID_ACCESS_REG] = nicUniCmdAccessReg,
	[CMD_ID_SET_BSS_RLM_PARAM] = nicUniCmdSetBssRlm,
	[CMD_ID_MQM_UPDATE_MU_EDCA_PARMS] = nicUniCmdUpdateMuEdca,
	[CMD_ID_SET_IP_ADDRESS] = nicUniCmdOffloadIPV4,
	[CMD_ID_SET_IPV6_ADDRESS] = nicUniCmdOffloadIPV6,
	[CMD_ID_GET_LTE_CHN] = nicUniCmdGetIdcChnl,
	[CMD_ID_GET_STA_STATISTICS] = nicUniCmdNotSupport,
	[CMD_ID_GET_STATISTICS] = nicUniCmdNotSupport,
	[CMD_ID_PERF_IND] = nicUniCmdNotSupport,
};

static PROCESS_LEGACY_TO_UNI_FUNCTION arUniExtCmdTable[EXT_CMD_ID_END] = {
	[0 ... EXT_CMD_ID_END - 1] = NULL,
	[EXT_CMD_ID_TWT_AGRT_UPDATE] = nicUniCmdTwtArgtUpdate,
	[EXT_CMD_ID_STAREC_UPDATE] = nicUniCmdStaRecUpdateExt,
	[EXT_CMD_ID_BF_ACTION] = nicUniCmdBFAction,
	[EXT_CMD_ID_SER] = nicUniCmdSerAction,
	[EXT_CMD_ID_GET_MAC_INFO] = nicUniCmdGetTsf,
};

static PROCESS_RX_UNI_EVENT_FUNCTION arUniEventTable[UNI_EVENT_ID_NUM] = {
	[0 ... UNI_EVENT_ID_NUM - 1] = NULL,
	[UNI_EVENT_ID_SCAN_DONE] = nicUniEventScanDone,
	[UNI_EVENT_ID_CNM] = nicUniEventChMngrHandleChEvent,
	[UNI_EVENT_ID_MBMC] = nicUniEventMbmcHandleEvent,
	[UNI_EVENT_ID_STATUS_TO_HOST] = nicUniEventStatusToHost,
	[UNI_EVENT_ID_BA_OFFLOAD] = nicUniEventBaOffload,
	[UNI_EVENT_ID_SLEEP_NOTIFY] = nicUniEventSleepNotify,
	[UNI_EVENT_ID_BEACON_TIMEOUT] = nicUniEventBeaconTimeout,
	[UNI_EVENT_ID_UPDATE_COEX_PHYRATE] = nicUniEventUpdateCoex,
	[UNI_EVENT_ID_IDC] = nicUniEventIdc,
	[UNI_EVENT_ID_BSS_IS_ABSENCE] = nicUniEventBssIsAbsence,
	[UNI_EVENT_ID_PS_SYNC] = nicUniEventPsSync,
	[UNI_EVENT_ID_SAP] = nicUniEventSap,
};

/*******************************************************************************
 *                           P R I V A T E   D A T A
 *******************************************************************************
 */

/*******************************************************************************
 *                                 M A C R O S
 *******************************************************************************
 */

/* This is for the usage of assign a 2 byte Wcid to WcidL and WcidH */
#define WCID_SET_H_L(_HnVer, _L, _u2Value) \
	do { \
		_HnVer = (uint8_t)(((_u2Value) >> 8) & 0x3); \
		_L = (uint8_t)((_u2Value) & 0xff); \
	} while (0)

/*******************************************************************************
 *                   F U N C T I O N   D E C L A R A T I O N S
 *******************************************************************************
 */

/*******************************************************************************
 *                              F U N C T I O N S
 *******************************************************************************
 */

struct WIFI_UNI_CMD_ENTRY *nicUniCmdAllocEntry(struct ADAPTER *ad,
	uint8_t ucid, uint32_t max_cmd_len,
	PFN_CMD_DONE_HANDLER pfCmdDoneHandler,
	PFN_CMD_TIMEOUT_HANDLER pfCmdTimeoutHandler)
{
	struct WIFI_UNI_CMD_ENTRY *entry;

	/* alloc entry + cmd_buf together */
	entry = (struct WIFI_UNI_CMD_ENTRY *) cnmMemAlloc(ad, RAM_TYPE_BUF,
			sizeof(*entry) + max_cmd_len);

	if (!entry)
		return NULL;

	kalMemZero(entry, sizeof(*entry) + max_cmd_len);
	entry->pucInfoBuffer = ((uint8_t *) entry) + sizeof(*entry);
	entry->u4SetQueryInfoLen = max_cmd_len;
	entry->pfCmdDoneHandler = pfCmdDoneHandler;
	entry->pfCmdTimeoutHandler = pfCmdTimeoutHandler;
	entry->ucUCID = ucid;

	return entry;
}

void nicUniCmdFreeEntry(struct ADAPTER *ad, struct WIFI_UNI_CMD_ENTRY *entry)
{
	if (entry)
		cnmMemFree(ad, entry);
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief command packet generation utility
 *
 * \param[in] prAdapter          Pointer to the Adapter structure.
 * \param[in] ucCID              Command ID
 * \param[in] fgSetQuery         Set or Query
 * \param[in] fgNeedResp         Need for response
 * \param[in] pfCmdDoneHandler   Function pointer when command is done
 * \param[in] u4SetQueryInfoLen  The length of the set/query buffer
 * \param[in] pucInfoBuffer      Pointer to set/query buffer
 *
 *
 * \retval WLAN_STATUS_PENDING
 * \retval WLAN_STATUS_FAILURE
 */
/*----------------------------------------------------------------------------*/
uint32_t
wlanSendSetQueryCmdHelper(IN struct ADAPTER *prAdapter,
		    uint8_t ucCID,
		    uint8_t ucExtCID,
		    u_int8_t fgSetQuery,
		    u_int8_t fgNeedResp,
		    u_int8_t fgIsOid,
		    PFN_CMD_DONE_HANDLER pfCmdDoneHandler,
		    PFN_CMD_TIMEOUT_HANDLER pfCmdTimeoutHandler,
		    uint32_t u4SetQueryInfoLen,
		    uint8_t *pucInfoBuffer, OUT void *pvSetQueryBuffer,
		    IN uint32_t u4SetQueryBufferLen,
		    enum EUNM_CMD_SEND_METHOD eMethod)
{
	struct WIFI_UNI_SETQUERY_INFO info;
	struct LINK *link;
	struct WIFI_UNI_CMD_ENTRY *entry, *next;
	uint32_t status = 0;

	if (kalIsResetting()) {
		DBGLOG(INIT, WARN, "Chip resetting, skip\n");
		return WLAN_STATUS_NOT_ACCEPTED;
	}

	if (!arUniCmdTable[ucCID])
		return wlanSendSetQueryCmdAdv(prAdapter,
			ucCID, ucExtCID, fgSetQuery, fgNeedResp, fgIsOid,
			pfCmdDoneHandler, pfCmdTimeoutHandler,
			u4SetQueryInfoLen, pucInfoBuffer,
			pvSetQueryBuffer, u4SetQueryBufferLen,
			eMethod);

	/* prepare info */
	info.ucCID = ucCID;
	info.ucExtCID = ucExtCID;
	info.fgSetQuery = fgSetQuery;
	info.fgNeedResp = fgNeedResp;
	info.fgIsOid = fgIsOid;
	info.pfCmdDoneHandler = pfCmdDoneHandler;
	info.pfCmdTimeoutHandler = pfCmdTimeoutHandler;
	info.u4SetQueryInfoLen = u4SetQueryInfoLen;
	info.pucInfoBuffer = pucInfoBuffer;
	info.pvSetQueryBuffer = pvSetQueryBuffer;
	info.u4SetQueryBufferLen = u4SetQueryBufferLen;
	LINK_INITIALIZE(&info.rUniCmdList);

	/* collect unified cmd info */
	status = arUniCmdTable[ucCID](prAdapter, &info);
	if (status != WLAN_STATUS_SUCCESS)
		goto done;

	link = &info.rUniCmdList;

	LINK_FOR_EACH_ENTRY_SAFE(entry, next,
		link, rLinkEntry, struct WIFI_UNI_CMD_ENTRY) {

		DBGLOG(REQ, TRACE,
			"UCMD[0x%x] SET[%d] RSP[%d] OID[%d] LEN[%d]\n",
			entry->ucUCID, fgSetQuery, fgNeedResp, fgIsOid,
			entry->u4SetQueryInfoLen);
		status = wlanSendSetQueryUniCmdAdv(prAdapter,
			entry->ucUCID, fgSetQuery, fgNeedResp, fgIsOid,
			entry->pfCmdDoneHandler, entry->pfCmdTimeoutHandler,
			entry->u4SetQueryInfoLen, entry->pucInfoBuffer,
			pvSetQueryBuffer, u4SetQueryBufferLen, eMethod);
	}
done:
	/* clear before return, in case uni handler already insert any entry */
	LINK_FOR_EACH_ENTRY_SAFE(entry, next,
		link, rLinkEntry, struct WIFI_UNI_CMD_ENTRY) {

		LINK_REMOVE_KNOWN_ENTRY(link, entry);
		/* releaes info */
		nicUniCmdFreeEntry(prAdapter, entry);
	}

	return status;
}

uint32_t wlanSendSetQueryUniCmd(IN struct ADAPTER *prAdapter,
			uint8_t ucUCID,
			u_int8_t fgSetQuery,
			u_int8_t fgNeedResp,
			u_int8_t fgIsOid,
			PFN_CMD_DONE_HANDLER pfCmdDoneHandler,
			PFN_CMD_TIMEOUT_HANDLER pfCmdTimeoutHandler,
			uint32_t u4SetQueryInfoLen,
			uint8_t *pucInfoBuffer, OUT void *pvSetQueryBuffer,
			IN uint32_t u4SetQueryBufferLen)
{
	return wlanSendSetQueryUniCmdAdv(prAdapter,
			ucUCID, fgSetQuery, fgNeedResp, fgIsOid,
			pfCmdDoneHandler, pfCmdTimeoutHandler,
			u4SetQueryInfoLen, pucInfoBuffer,
			pvSetQueryBuffer, u4SetQueryBufferLen,
			CMD_SEND_METHOD_ENQUEUE);
}

/*----------------------------------------------------------------------------*/
/*!
* \brief command packet generation utility
*
* \param[in] prAdapter	   Pointer to the Adapter structure.
* \param[in] ucCID		   Command ID
* \param[in] fgSetQuery	   Set or Query
* \param[in] fgNeedResp	   Need for response
* \param[in] pfCmdDoneHandler   Function pointer when command is done
* \param[in] u4SetQueryInfoLen  The length of the set/query buffer
* \param[in] pucInfoBuffer	   Pointer to set/query buffer
*
*
* \retval WLAN_STATUS_PENDING
* \retval WLAN_STATUS_FAILURE
*/
/*----------------------------------------------------------------------------*/
uint32_t
wlanSendSetQueryUniCmdAdv(IN struct ADAPTER *prAdapter,
	      uint8_t ucUCID,
	      u_int8_t fgSetQuery,
	      u_int8_t fgNeedResp,
	      u_int8_t fgIsOid,
	      PFN_CMD_DONE_HANDLER pfCmdDoneHandler,
	      PFN_CMD_TIMEOUT_HANDLER pfCmdTimeoutHandler,
	      uint32_t u4SetQueryInfoLen,
	      uint8_t *pucInfoBuffer, OUT void *pvSetQueryBuffer,
	      IN uint32_t u4SetQueryBufferLen,
	      enum EUNM_CMD_SEND_METHOD eMethod)
{
	struct GLUE_INFO *prGlueInfo;
	struct CMD_INFO *prCmdInfo;
	uint8_t *pucCmfBuf;
	struct mt66xx_chip_info *prChipInfo;
	uint16_t cmd_size;
	uint8_t ucOption;
	uint32_t status = WLAN_STATUS_PENDING;

	if (kalIsResetting()) {
		DBGLOG(INIT, WARN, "Chip resetting, skip\n");
		return WLAN_STATUS_NOT_ACCEPTED;
	}

	prGlueInfo = prAdapter->prGlueInfo;
	prChipInfo = prAdapter->chip_info;
	cmd_size = prChipInfo->u2UniCmdTxHdrSize + u4SetQueryInfoLen;
	prCmdInfo = cmdBufAllocateCmdInfo(prAdapter, cmd_size);

	if (!prCmdInfo) {
		DBGLOG(INIT, ERROR, "Allocate CMD_INFO_T ==> FAILED.\n");
		return WLAN_STATUS_RESOURCES;
	}

	/* Setup common CMD Info Packet */
	prCmdInfo->eCmdType = COMMAND_TYPE_NETWORK_IOCTL;
	prCmdInfo->u2InfoBufLen = cmd_size;
	prCmdInfo->pfCmdDoneHandler = pfCmdDoneHandler;
	prCmdInfo->pfCmdTimeoutHandler = pfCmdTimeoutHandler;
	prCmdInfo->fgIsOid = fgIsOid;
	prCmdInfo->ucCID = ucUCID;
	prCmdInfo->fgSetQuery = fgSetQuery;
	prCmdInfo->fgNeedResp = fgNeedResp;
	prCmdInfo->u4SetInfoLen = u4SetQueryInfoLen;
	prCmdInfo->pvInformationBuffer = pvSetQueryBuffer;
	prCmdInfo->u4InformationBufferLength = u4SetQueryBufferLen;
	ucOption = UNI_CMD_OPT_BIT_1_UNI_CMD;
	if (fgSetQuery) /* it is a SET command */
		ucOption |= (prCmdInfo->fgNeedResp ? UNI_CMD_OPT_BIT_0_ACK : 0);
	ucOption |= (fgIsOid ? UNI_CMD_OPT_BIT_0_ACK : 0);
	ucOption |= (fgSetQuery ? UNI_CMD_OPT_BIT_2_SET_QUERY : 0);

	/* TODO: uni cmd, fragment */
	/* Setup WIFI_CMD_T (no payload) */
	NIC_FILL_UNI_CMD_TX_HDR(prAdapter,
		prCmdInfo->pucInfoBuffer,
		prCmdInfo->u2InfoBufLen,
		prCmdInfo->ucCID,
		CMD_PACKET_TYPE_ID,
		&prCmdInfo->ucCmdSeqNum,
		&pucCmfBuf, S2D_INDEX_CMD_H2N,
		ucOption);
	prCmdInfo->pucSetInfoBuffer = pucCmfBuf;
	if (u4SetQueryInfoLen > 0 && pucInfoBuffer != NULL)
		kalMemCopy(pucCmfBuf, pucInfoBuffer, u4SetQueryInfoLen);

	switch (eMethod) {
	case CMD_SEND_METHOD_ENQUEUE:
		/* insert into prCmdQueue */
		kalEnqueueCommand(prGlueInfo,
				  (struct QUE_ENTRY *) prCmdInfo);

		/* wakeup txServiceThread later */
		GLUE_SET_EVENT(prGlueInfo);
		break;
	case CMD_SEND_METHOD_REQ_RESOURCE:
		status = wlanSendCommand(prAdapter, prCmdInfo);
		cmdBufFreeCmdInfo(prAdapter, prCmdInfo);
		break;
	case CMD_SEND_METHOD_TX:
		status = nicTxCmd(prAdapter, prCmdInfo, TC4_INDEX);
		cmdBufFreeCmdInfo(prAdapter, prCmdInfo);
		break;
	}

	return status;
}

uint32_t nicUniCmdExtCommon(struct ADAPTER *ad,
	struct WIFI_UNI_SETQUERY_INFO *info)
{
	if (info->ucCID != CMD_ID_LAYER_0_EXT_MAGIC_NUM)
		return WLAN_STATUS_NOT_ACCEPTED;

	if (!arUniExtCmdTable[info->ucExtCID])
		return WLAN_STATUS_NOT_SUPPORTED;

	return arUniExtCmdTable[info->ucExtCID](ad, info);
}

uint32_t nicUniCmdNotSupport(struct ADAPTER *ad,
		struct WIFI_UNI_SETQUERY_INFO *info)
{
	DBGLOG(INIT, ERROR, "CID[0x%x] not support!!\n", info->ucCID);
	return WLAN_STATUS_SUCCESS;
}

uint32_t nicUniCmdScanTagScanReq(struct ADAPTER *ad,
	uint8_t *buf, struct CMD_SCAN_REQ_V2 *cmd)
{
	struct UNI_CMD_SCAN_REQ *tag = (struct UNI_CMD_SCAN_REQ *)buf;

	tag->u2Tag = UNI_CMD_SCAN_TAG_SCAN_REQ;
	tag->u2Length = sizeof(*tag);
	tag->ucScanType = cmd->ucScanType;
	tag->ucNumProbeReq = cmd->ucNumProbeReq;
	tag->ucScnFuncMask = cmd->ucScnFuncMask;
	tag->u2ChannelMinDwellTime = cmd->u2ChannelMinDwellTime;
	tag->u2ChannelDwellTime = cmd->u2ChannelDwellTime;
	tag->u2TimeoutValue = cmd->u2TimeoutValue;
	tag->u2ProbeDelayTime = cmd->u2ProbeDelayTime;
	return tag->u2Length;
}

uint32_t nicUniCmdScanTagScanSsid(struct ADAPTER *ad, uint8_t *buf,
	struct CMD_SCAN_REQ_V2 *cmd)
{
	struct UNI_CMD_SCAN_SSID *tag = (struct UNI_CMD_SCAN_SSID *)buf;
	uint8_t i;
	uint8_t *pos = tag->aucSsidBuffer;
	uint8_t ssid_num = min_t(int, cmd->ucSSIDNum, 4);
	uint8_t ssid_ext_num = min_t(int, cmd->ucSSIDExtNum, 6);
	uint16_t len = sizeof(*tag) +
		(ssid_num + ssid_ext_num) * sizeof(struct PARAM_SSID);

	tag->u2Tag = UNI_CMD_SCAN_TAG_SCAN_SSID;
	tag->u2Length = len;
	tag->ucSSIDType = cmd->ucSSIDType;
	tag->ucSSIDNum = ssid_num + ssid_ext_num;
	for (i = 0; i < ssid_num; i++) {
		kalMemCopy(pos, &cmd->arSSID[i], sizeof(struct PARAM_SSID));
		pos += sizeof(struct PARAM_SSID);
	}
	for (i = 0; i < ssid_ext_num; i++) {
		kalMemCopy(pos, &cmd->arSSIDExtend[i],
					sizeof(struct PARAM_SSID));
		pos += sizeof(struct PARAM_SSID);
	}
	return tag->u2Length;
}

uint32_t nicUniCmdScanTagScanBssid(struct ADAPTER *ad, uint8_t *buf,
	struct CMD_SCAN_REQ_V2 *cmd)
{
	struct UNI_CMD_SCAN_BSSID *tag = (struct UNI_CMD_SCAN_BSSID *)buf;

	tag->u2Tag = UNI_CMD_SCAN_TAG_SCAN_BSSID;
	tag->u2Length = sizeof(*tag);
	kalMemCopy(tag->aucBssid, cmd->aucBSSID, MAC_ADDR_LEN);

	/* TODO: uni cmd */
	tag->ucBssidMatchCh = 0;
	tag->ucBssidMatchSsidInd = 0;

	return tag->u2Length;
}

uint32_t nicUniCmdScanTagScanChnlInfo(struct ADAPTER *ad, uint8_t *buf,
	struct CMD_SCAN_REQ_V2 *cmd)
{
	struct UNI_CMD_SCAN_CHANNEL_INFO *tag =
		(struct UNI_CMD_SCAN_CHANNEL_INFO *)buf;
	uint8_t i;
	uint8_t *pos = tag->aucChnlInfoBuffer;
	uint8_t chnl_num = min_t(int, cmd->ucChannelListNum, 32);
	uint8_t chnl_ext_num = min_t(int, cmd->ucChannelListExtNum, 32);
	uint16_t len = sizeof(*tag) +
		(chnl_num + chnl_ext_num) * sizeof(struct CHANNEL_INFO);

	tag->u2Tag = UNI_CMD_SCAN_TAG_SCAN_CHANNEL;
	tag->u2Length = len;
	tag->ucChannelType = cmd->ucChannelType;
	tag->ucChannelListNum = chnl_num + chnl_ext_num;
	for (i = 0; i < chnl_num; i++) {
		kalMemCopy(pos, &cmd->arChannelList[i],
			sizeof(struct CHANNEL_INFO));
		pos += sizeof(struct CHANNEL_INFO);
	}
	for (i = 0; i < chnl_ext_num; i++) {
		kalMemCopy(pos, &cmd->arChannelListExtend[i],
			sizeof(struct CHANNEL_INFO));
		pos += sizeof(struct CHANNEL_INFO);
	}
	return tag->u2Length;
}

uint32_t nicUniCmdScanTagScanIe(struct ADAPTER *ad, uint8_t *buf,
	struct CMD_SCAN_REQ_V2 *cmd)
{
	struct UNI_CMD_SCAN_IE *tag = (struct UNI_CMD_SCAN_IE *)buf;

	tag->u2Tag = UNI_CMD_SCAN_TAG_SCAN_IE;
	tag->u2Length = sizeof(*tag) + cmd->u2IELen;
	tag->u2IELen = cmd->u2IELen;
	kalMemCopy(tag->aucIEBuffer, cmd->aucIE, cmd->u2IELen);

	return tag->u2Length;
}

uint32_t nicUniCmdScanTagScanMisc(struct ADAPTER *ad, uint8_t *buf,
	struct CMD_SCAN_REQ_V2 *cmd)
{
	struct UNI_CMD_SCAN_MISC *tag = (struct UNI_CMD_SCAN_MISC *)buf;

	tag->u2Tag = UNI_CMD_SCAN_TAG_SCAN_MISC;
	tag->u2Length = sizeof(*tag);
	kalMemCopy(tag->aucRandomMac, cmd->aucRandomMac, MAC_ADDR_LEN);

	/* TODO: uni cmd */
	tag->ucShortSSIDNum = 0;

	return tag->u2Length;
}

struct UNI_CMD_SCAN_TAG_HANDLE arSetScanReqTable[] = {
	{sizeof(struct UNI_CMD_SCAN_REQ), nicUniCmdScanTagScanReq},
	{sizeof(struct UNI_CMD_SCAN_SSID) + sizeof(struct PARAM_SSID) * 10,
	 nicUniCmdScanTagScanSsid},
	{sizeof(struct UNI_CMD_SCAN_BSSID), nicUniCmdScanTagScanBssid},
	{sizeof(struct UNI_CMD_SCAN_CHANNEL_INFO) +
	 sizeof(struct CHANNEL_INFO) * 64,
	 nicUniCmdScanTagScanChnlInfo},
	{sizeof(struct UNI_CMD_SCAN_IE) + 600, nicUniCmdScanTagScanIe},
	{sizeof(struct UNI_CMD_SCAN_MISC), nicUniCmdScanTagScanMisc},
};

uint32_t nicUniCmdScanReqV2(struct ADAPTER *ad,
	struct WIFI_UNI_SETQUERY_INFO *info)
{
	struct CMD_SCAN_REQ_V2 *cmd =
		(struct CMD_SCAN_REQ_V2 *)info->pucInfoBuffer;
	struct UNI_CMD_SCAN *uni_cmd;
	struct WIFI_UNI_CMD_ENTRY *entry;
	uint8_t *pos;
	uint32_t max_cmd_len = 0;
	int i;

	if (info->ucCID != CMD_ID_SCAN_REQ_V2 ||
	    info->u4SetQueryInfoLen != sizeof(*cmd))
		return WLAN_STATUS_NOT_ACCEPTED;

	max_cmd_len += sizeof(struct UNI_CMD_SCAN);
	for (i = 0; i < ARRAY_SIZE(arSetScanReqTable); i++)
		max_cmd_len += arSetScanReqTable[i].u4Size;

	entry = nicUniCmdAllocEntry(ad, UNI_CMD_ID_SCAN_REQ,
			max_cmd_len, NULL, NULL);

	if (!entry)
		return WLAN_STATUS_RESOURCES;

	pos = entry->pucInfoBuffer;
	uni_cmd = (struct UNI_CMD_SCAN *) pos;
	uni_cmd->ucSeqNum = cmd->ucSeqNum;
	uni_cmd->ucBssIndex = cmd->ucBssIndex;
	pos += sizeof(*uni_cmd);
	for (i = 0; i < ARRAY_SIZE(arSetScanReqTable); i++)
		pos += arSetScanReqTable[i].pfHandler(ad, pos, cmd);
	entry->u4SetQueryInfoLen = pos - entry->pucInfoBuffer;

	ASSERT(entry->u4SetQueryInfoLen <= max_cmd_len);

	LINK_INSERT_TAIL(&info->rUniCmdList, &entry->rLinkEntry);

	return WLAN_STATUS_SUCCESS;
}

uint32_t nicUniCmdBssInfoConnType(struct ADAPTER *ad, struct BSS_INFO *bssinfo)
{
	if (bssinfo->eNetworkType == NETWORK_TYPE_AIS) {
		return CONNECTION_INFRA_STA;
	} else if (bssinfo->eNetworkType == NETWORK_TYPE_P2P) {
		if (bssinfo->eCurrentOPMode == OP_MODE_INFRASTRUCTURE) {
			return CONNECTION_P2P_GC;
		} else if (bssinfo->eCurrentOPMode == OP_MODE_ACCESS_POINT) {
#if CFG_ENABLE_WIFI_DIRECT
			if (ad->fgIsP2PRegistered &&
			    !p2pFuncIsAPMode(ad->rWifiVar.prP2PConnSettings[
					bssinfo->u4PrivateData])) {
				return CONNECTION_P2P_GO;
			}
#endif
			return CONNECTION_INFRA_AP;
		} else if (bssinfo->eCurrentOPMode == OP_MODE_P2P_DEVICE) {
			return CONNECTION_P2P_DEVICE;
		}
	}

	return 0;
}

uint32_t nicUniCmdBssInfoPhyMode(uint8_t ucPhyTypeSet)
{
	uint32_t phy_mode = 0;

	if (ucPhyTypeSet & PHY_TYPE_BIT_HR_DSSS)
		phy_mode |= WMODE_B;

	if (ucPhyTypeSet & PHY_TYPE_BIT_ERP)
		phy_mode |= WMODE_G;

	if (ucPhyTypeSet & PHY_TYPE_BIT_OFDM)
		phy_mode |= WMODE_A;

	if (ucPhyTypeSet & PHY_TYPE_BIT_HT) {
		phy_mode |= WMODE_GN;
		phy_mode |= WMODE_AN;
	}

	if (ucPhyTypeSet & PHY_TYPE_BIT_VHT)
		phy_mode |= WMODE_AC;

#if (CFG_SUPPORT_802_11AX == 1)
	if (ucPhyTypeSet & PHY_TYPE_BIT_HE) {
		phy_mode |= WMODE_AX_24G;
		phy_mode |= WMODE_AX_5G;
		phy_mode |= WMODE_AX_6G;
	}
#endif
	return phy_mode;
}

uint32_t nicUniCmdBssActivateCtrl(struct ADAPTER *ad,
		struct WIFI_UNI_SETQUERY_INFO *info)
{
	struct CMD_BSS_ACTIVATE_CTRL *cmd;
	struct UNI_CMD_DEVINFO *dev_cmd;
	struct UNI_CMD_BSSINFO *bss_cmd;
	struct UNI_CMD_DEVINFO_ACTIVE *dev_active_tag;
	struct UNI_CMD_BSSINFO_BASIC *bss_basic_tag;
	struct UNI_CMD_BSSINFO_MLD *bss_mld_tag;
	struct WIFI_UNI_CMD_ENTRY *dev_entry = NULL, *bss_entry = NULL;
	uint32_t max_cmd_len;
	struct BSS_INFO *bss;
	uint32_t phy_mode;

	cmd = (struct CMD_BSS_ACTIVATE_CTRL *) info->pucInfoBuffer;
	bss = GET_BSS_INFO_BY_INDEX(ad, cmd->ucBssIndex);

	if (info->ucCID != CMD_ID_BSS_ACTIVATE_CTRL ||
	    info->u4SetQueryInfoLen != sizeof(*cmd) ||
	    bss->eNetworkType != NETWORK_TYPE_AIS)
		return WLAN_STATUS_NOT_ACCEPTED;

	/* update devinfo */
	max_cmd_len = sizeof(struct UNI_CMD_DEVINFO) +
		      sizeof(struct UNI_CMD_DEVINFO_ACTIVE);
	dev_entry = nicUniCmdAllocEntry(ad, UNI_CMD_ID_DEVINFO,
			max_cmd_len, NULL, NULL);
	if (!dev_entry)
		goto fail;
	dev_cmd = (struct UNI_CMD_DEVINFO *) dev_entry->pucInfoBuffer;
	dev_cmd->ucOwnMacIdx = cmd->ucOwnMacAddrIndex;
	dev_cmd->ucDbdcIdx = ENUM_BAND_AUTO;
	dev_active_tag = (struct UNI_CMD_DEVINFO_ACTIVE *)dev_cmd->aucTlvBuffer;
	dev_active_tag->u2Tag = UNI_CMD_DEVINFO_TAG_ACTIVE;
	dev_active_tag->u2Length = sizeof(*dev_active_tag);
	dev_active_tag->ucActive = cmd->ucActive;
	COPY_MAC_ADDR(dev_active_tag->aucOwnMacAddr, cmd->aucBssMacAddr);

	/* update bssinfo */
	max_cmd_len = sizeof(struct UNI_CMD_BSSINFO) +
		      sizeof(struct UNI_CMD_BSSINFO_BASIC) +
		      (cmd->ucActive ? sizeof(struct UNI_CMD_BSSINFO_MLD) : 0);
	bss_entry = nicUniCmdAllocEntry(ad, UNI_CMD_ID_BSSINFO,
			max_cmd_len, NULL, NULL);
	if (!bss_entry)
		goto fail;

	bss_cmd = (struct UNI_CMD_BSSINFO *) bss_entry->pucInfoBuffer;
	bss_cmd->ucBssInfoIdx = cmd->ucBssIndex;
	bss_basic_tag = (struct UNI_CMD_BSSINFO_BASIC *)bss_cmd->aucTlvBuffer;
	bss_basic_tag->u2Tag = UNI_CMD_BSSINFO_TAG_BASIC;
	bss_basic_tag->u2Length = sizeof(*bss_basic_tag);
	bss_basic_tag->ucActive = cmd->ucActive;
	bss_basic_tag->ucDbdcIdx = ENUM_BAND_AUTO;
	bss_basic_tag->ucOwnMacIdx = cmd->ucOwnMacAddrIndex;
	bss_basic_tag->ucHwBSSIndex = cmd->ucOwnMacAddrIndex;
	bss_basic_tag->u4ConnectionType = nicUniCmdBssInfoConnType(ad, bss);
	bss_basic_tag->ucConnectionState = bss->eConnectionState;
	bss_basic_tag->ucWmmIdx = bss->ucWmmQueSet;
	COPY_MAC_ADDR(bss_basic_tag->aucBSSID, bss->aucBSSID);
	bss_basic_tag->u2BcMcWlanidx = cmd->ucBMCWlanIndex;
	bss_basic_tag->u2BcnInterval = bss->u2BeaconInterval;
	bss_basic_tag->ucDtimPeriod = bss->ucDTIMPeriod;
	bss_basic_tag->u2StaRecIdxOfAP = STA_REC_INDEX_NOT_FOUND;
	bss_basic_tag->u2NonHTBasicPhyType = bss->ucNonHTBasicPhyType;
	phy_mode = nicUniCmdBssInfoPhyMode(bss->ucPhyTypeSet);
	bss_basic_tag->ucPhyMode = phy_mode & 0xff;
	bss_basic_tag->ucPhyModeExt = (phy_mode >> 8) & 0xff;

	DBGLOG(INIT, INFO,
		"%s DevInfo[OMAC=%d, DBDC=%d], BssInfo%d[DBDC=%d, OMAC=%d, ConnType=%d, ConnState=%d, BcIdx=%d, PhyMode=0x%x, PhyModeEx=0x%x]\n",
		cmd->ucActive ? "Activate" : "Deactivate",
		dev_cmd->ucOwnMacIdx, dev_cmd->ucDbdcIdx,
		bss_cmd->ucBssInfoIdx, bss_basic_tag->ucDbdcIdx,
		bss_basic_tag->ucOwnMacIdx, bss_basic_tag->u4ConnectionType,
		bss_basic_tag->ucConnectionState, bss_basic_tag->u2BcMcWlanidx,
		bss_basic_tag->ucPhyMode, bss_basic_tag->ucPhyModeExt);

	if (cmd->ucActive) {
		bss_mld_tag = (struct UNI_CMD_BSSINFO_MLD *)
			(bss_cmd->aucTlvBuffer + sizeof(*bss_basic_tag));
		bss_mld_tag->u2Tag = UNI_CMD_BSSINFO_TAG_MLD;
		bss_mld_tag->u2Length = sizeof(*bss_mld_tag);
		bss_mld_tag->ucGroupMldId = MLD_GROUP_NONE;
		bss_mld_tag->ucOwnMldId = bss->ucBssIndex;
		COPY_MAC_ADDR(bss_mld_tag->aucOwnMldAddr, bss->aucBSSID);
		bss_mld_tag->ucOmRemapIdx = OM_REMAP_IDX_NONE;

		DBGLOG(INIT, INFO,
			"BssInfo%d[GroupMldId=%d, OwnMldId=%d, OmRemapIdx=%d, OwnMldAddr="
			MACSTR "]\n",
			bss_cmd->ucBssInfoIdx,
			bss_mld_tag->ucGroupMldId,
			bss_mld_tag->ucOwnMldId,
			bss_mld_tag->ucOmRemapIdx,
			MAC2STR(bss_mld_tag->aucOwnMldAddr));
	}

	if (cmd->ucActive) {
		/* activate devinfo first */
		LINK_INSERT_TAIL(&info->rUniCmdList, &dev_entry->rLinkEntry);
		LINK_INSERT_TAIL(&info->rUniCmdList, &bss_entry->rLinkEntry);
	} else {
		/* deactivate bssinfo first */
		LINK_INSERT_TAIL(&info->rUniCmdList, &bss_entry->rLinkEntry);
		LINK_INSERT_TAIL(&info->rUniCmdList, &dev_entry->rLinkEntry);
	}

	return WLAN_STATUS_SUCCESS;

fail:
	nicUniCmdFreeEntry(ad, dev_entry);
	nicUniCmdFreeEntry(ad, bss_entry);
	return WLAN_STATUS_RESOURCES;
}

uint32_t nicUniCmdCustomerCfg(struct ADAPTER *ad,
		struct WIFI_UNI_SETQUERY_INFO *info)
{
	struct CMD_HEADER *cmd;
	struct UNI_CMD_CHIP_CONFIG *uni_cmd;
	struct UNI_CMD_CHIP_CONFIG_CUSTOMER_CFG *tag;
	struct WIFI_UNI_CMD_ENTRY *entry;
	uint32_t max_cmd_len = sizeof(struct UNI_CMD_CHIP_CONFIG) +
			       sizeof(struct UNI_CMD_CHIP_CONFIG_CUSTOMER_CFG);

	if (info->ucCID != CMD_ID_GET_SET_CUSTOMER_CFG ||
	    info->u4SetQueryInfoLen != sizeof(*cmd))
		return WLAN_STATUS_NOT_ACCEPTED;

	cmd = (struct CMD_HEADER *) info->pucInfoBuffer;
	entry = nicUniCmdAllocEntry(ad, UNI_CMD_ID_CHIP_CONFIG, max_cmd_len,
			info->fgSetQuery ? NULL : nicUniCmdEventQueryCfgRead,
			info->fgSetQuery ? NULL : nicUniCmdTimeoutCommon);

	if (!entry)
		return WLAN_STATUS_RESOURCES;

	uni_cmd = (struct UNI_CMD_CHIP_CONFIG *) entry->pucInfoBuffer;
	tag = (struct UNI_CMD_CHIP_CONFIG_CUSTOMER_CFG *) uni_cmd->aucTlvBuffer;
	tag->u2Tag = UNI_CMD_CHIP_CONFIG_TAG_CUSTOMER_CFG;
	tag->u2Length = sizeof(*tag);
	tag->cmdBufferLen = cmd->cmdBufferLen;
	tag->itemNum = cmd->itemNum;
	kalMemCopy(tag->aucbuffer, cmd->buffer, cmd->cmdBufferLen);

	LINK_INSERT_TAIL(&info->rUniCmdList, &entry->rLinkEntry);

	return WLAN_STATUS_SUCCESS;
}

uint32_t nicUniCmdChipCfg(struct ADAPTER *ad,
		struct WIFI_UNI_SETQUERY_INFO *info)
{
	struct CMD_CHIP_CONFIG *cmd;
	struct UNI_CMD_CHIP_CONFIG *uni_cmd;
	struct UNI_CMD_CHIP_CONFIG_CHIP_CFG *tag;
	struct UNI_CMD_CHIP_CONFIG_CHIP_CFG_RESP *resp;
	struct WIFI_UNI_CMD_ENTRY *entry;
	uint32_t max_cmd_len = sizeof(struct UNI_CMD_CHIP_CONFIG) +
			       sizeof(struct UNI_CMD_CHIP_CONFIG_CHIP_CFG) +
			       sizeof(struct UNI_CMD_CHIP_CONFIG_CHIP_CFG_RESP);

	if (info->ucCID != CMD_ID_CHIP_CONFIG ||
	    info->u4SetQueryInfoLen != sizeof(*cmd))
		return WLAN_STATUS_NOT_ACCEPTED;

	cmd = (struct CMD_CHIP_CONFIG *) info->pucInfoBuffer;
	entry = nicUniCmdAllocEntry(ad, UNI_CMD_ID_CHIP_CONFIG, max_cmd_len,
			info->fgSetQuery ? nicUniCmdEventSetCommon :
			nicUniEventQueryChipConfig,
			nicUniCmdTimeoutCommon);

	if (!entry)
		return WLAN_STATUS_RESOURCES;

	uni_cmd = (struct UNI_CMD_CHIP_CONFIG *) entry->pucInfoBuffer;
	tag = (struct UNI_CMD_CHIP_CONFIG_CHIP_CFG *) uni_cmd->aucTlvBuffer;
	tag->u2Tag = UNI_CMD_CHIP_CONFIG_TAG_CHIP_CFG;
	tag->u2Length = sizeof(*tag) + sizeof(*resp);
	resp = (struct UNI_CMD_CHIP_CONFIG_CHIP_CFG_RESP *) tag->aucbuffer;
	resp->u2Id = cmd->u2Id;
	resp->u2MsgSize = cmd->u2MsgSize;
	resp->ucType = cmd->ucType;
	resp->ucRespType = cmd->ucRespType;
	kalMemCopy(resp->aucCmd, cmd->aucCmd, cmd->u2MsgSize);

	LINK_INSERT_TAIL(&info->rUniCmdList, &entry->rLinkEntry);

	return WLAN_STATUS_SUCCESS;
}

uint32_t nicUniCmdSetRxFilter(struct ADAPTER *ad,
		struct WIFI_UNI_SETQUERY_INFO *info)
{
	struct CMD_RX_PACKET_FILTER *cmd;
	struct UNI_CMD_BAND_CONFIG *uni_cmd;
	struct UNI_CMD_BAND_CONFIG_SET_RX_FILTER *tag;
	struct WIFI_UNI_CMD_ENTRY *entry;
	uint32_t max_cmd_len = sizeof(struct UNI_CMD_BAND_CONFIG) +
			       sizeof(struct UNI_CMD_BAND_CONFIG_SET_RX_FILTER);

	if (info->ucCID != CMD_ID_SET_RX_FILTER ||
	    info->u4SetQueryInfoLen != sizeof(*cmd))
		return WLAN_STATUS_NOT_ACCEPTED;

	cmd = (struct CMD_RX_PACKET_FILTER *) info->pucInfoBuffer;
	entry = nicUniCmdAllocEntry(ad, UNI_CMD_ID_BAND_CONFIG,
				max_cmd_len, nicUniCmdEventSetCommon,
				nicUniCmdTimeoutCommon);
	if (!entry)
		return WLAN_STATUS_RESOURCES;

	uni_cmd = (struct UNI_CMD_BAND_CONFIG *) entry->pucInfoBuffer;
	uni_cmd->ucDbdcIdx = ENUM_BAND_ALL;
	tag = (struct UNI_CMD_BAND_CONFIG_SET_RX_FILTER *)
		uni_cmd->aucTlvBuffer;
	tag->u2Tag = UNI_CMD_BAND_CONFIG_TAG_SET_RX_FILTER;
	tag->u2Length = sizeof(*tag);
	tag->u4RxPacketFilter = cmd->u4RxPacketFilter;

	LINK_INSERT_TAIL(&info->rUniCmdList, &entry->rLinkEntry);

	return WLAN_STATUS_SUCCESS;
}

uint32_t nicUniCmdSetMbmc(struct ADAPTER *ad,
		struct WIFI_UNI_SETQUERY_INFO *info)
{
	struct CMD_DBDC_SETTING *cmd;
	struct UNI_CMD_MBMC *uni_cmd;
	struct UNI_CMD_MBMC_SETTING *tag;
	struct WIFI_UNI_CMD_ENTRY *entry;
	uint32_t max_cmd_len = sizeof(struct UNI_CMD_MBMC) +
			       sizeof(struct UNI_CMD_MBMC_SETTING);

	if (info->ucCID != CMD_ID_SET_DBDC_PARMS ||
	    info->u4SetQueryInfoLen != sizeof(*cmd))
		return WLAN_STATUS_NOT_ACCEPTED;

	cmd = (struct CMD_DBDC_SETTING *) info->pucInfoBuffer;
	entry = nicUniCmdAllocEntry(ad, UNI_CMD_ID_MBMC,
					max_cmd_len, NULL, NULL);

	if (!entry)
		return WLAN_STATUS_RESOURCES;

	uni_cmd = (struct UNI_CMD_MBMC *) entry->pucInfoBuffer;
	tag = (struct UNI_CMD_MBMC_SETTING *) uni_cmd->aucTlvBuffer;
	tag->u2Tag = UNI_CMD_MBMC_TAG_SETTING;
	tag->u2Length = sizeof(*tag);
	tag->ucMbmcEn = cmd->ucDbdcEn;
	tag->ucRfBand = 0; /* unused */

	LINK_INSERT_TAIL(&info->rUniCmdList, &entry->rLinkEntry);

	return WLAN_STATUS_SUCCESS;
}

uint32_t nicUniCmdPowerCtrl(struct ADAPTER *ad,
		struct WIFI_UNI_SETQUERY_INFO *info)
{
	struct CMD_NIC_POWER_CTRL *cmd;
	struct UNI_CMD_POWER_CTRL *uni_cmd;
	struct UNI_CMD_POWER_OFF *tag;
	struct WIFI_UNI_CMD_ENTRY *entry;
	uint32_t max_cmd_len = sizeof(struct UNI_CMD_POWER_CTRL) +
			       sizeof(struct UNI_CMD_POWER_OFF);

	if (info->ucCID != CMD_ID_NIC_POWER_CTRL ||
	    info->u4SetQueryInfoLen != sizeof(*cmd))
		return WLAN_STATUS_NOT_ACCEPTED;

	cmd = (struct CMD_NIC_POWER_CTRL *) info->pucInfoBuffer;
	entry = nicUniCmdAllocEntry(ad, UNI_CMD_ID_POWER_CTRL,
					max_cmd_len, NULL, NULL);

	if (!entry)
		return WLAN_STATUS_RESOURCES;

	uni_cmd = (struct UNI_CMD_POWER_CTRL *) entry->pucInfoBuffer;
	tag = (struct UNI_CMD_POWER_OFF *) uni_cmd->aucTlvBuffer;
	tag->u2Tag = UNI_CMD_POWER_CTRL_TAG_OFF;
	tag->u2Length = sizeof(*tag);
	tag->ucPowerMode = cmd->ucPowerMode;

	LINK_INSERT_TAIL(&info->rUniCmdList, &entry->rLinkEntry);

	return WLAN_STATUS_SUCCESS;
}

uint32_t nicUniCmdSetDomainV1(struct ADAPTER *ad,
		struct WIFI_UNI_SETQUERY_INFO *info)
{
	struct CMD_SET_DOMAIN_INFO *cmd;
	struct UNI_CMD_DOMAIN_SET_INFO *uni_cmd;
	struct UNI_CMD_DOMAIN_SET_INFO_DOMAIN_SUBBAND *tag;
	struct UNI_CMD_DOMAIN_SUBBAND_INFO *sub;
	struct WIFI_UNI_CMD_ENTRY *entry;
	uint32_t max_cmd_len = sizeof(struct UNI_CMD_DOMAIN_SET_INFO) +
		sizeof(struct UNI_CMD_DOMAIN_SET_INFO_DOMAIN_SUBBAND) +
		sizeof(struct UNI_CMD_DOMAIN_SUBBAND_INFO) * MAX_SUBBAND_NUM;
	uint8_t i;

	cmd = (struct CMD_SET_DOMAIN_INFO *) info->pucInfoBuffer;
	entry = nicUniCmdAllocEntry(ad, UNI_CMD_ID_SET_DOMAIN_INFO,
					max_cmd_len, NULL, NULL);

	if (!entry)
		return WLAN_STATUS_RESOURCES;

	uni_cmd = (struct UNI_CMD_DOMAIN_SET_INFO *) entry->pucInfoBuffer;
	uni_cmd->u4CountryCode = cmd->u2CountryCode;
	uni_cmd->uc2G4Bandwidth = cmd->uc2G4Bandwidth;
	uni_cmd->uc5GBandwidth = cmd->uc5GBandwidth;
	uni_cmd->uc6GBandwidth = cmd->uc5GBandwidth; /* TODO: uni cmd, 6g */

	tag = (struct UNI_CMD_DOMAIN_SET_INFO_DOMAIN_SUBBAND *)
		uni_cmd->aucTlvBuffer;
	tag->u2Tag = UNI_CMD_DOMAIN_TAG_SUBBAND;
	tag->u2Length = sizeof(*tag) +
		sizeof(struct UNI_CMD_DOMAIN_SUBBAND_INFO) * MAX_SUBBAND_NUM;

	tag->u2IsSetPassiveScan = cmd->u2IsSetPassiveScan;
	tag->ucSubBandNum = MAX_SUBBAND_NUM;
	sub = (struct UNI_CMD_DOMAIN_SUBBAND_INFO *)tag->aucSubBandInfoBuffer;
	for (i = 0; i < MAX_SUBBAND_NUM; i++) {
		sub->ucRegClass = cmd->rSubBand[i].ucRegClass;
		sub->ucBand = cmd->rSubBand[i].ucBand;
		sub->ucChannelSpan = cmd->rSubBand[i].ucChannelSpan;
		sub->ucFirstChannelNum = cmd->rSubBand[i].ucFirstChannelNum;
		sub->ucNumChannels = cmd->rSubBand[i].ucNumChannels;
		sub->fgDfs = FALSE; /* unused */
		sub++;
	}

	LINK_INSERT_TAIL(&info->rUniCmdList, &entry->rLinkEntry);

	return WLAN_STATUS_SUCCESS;
}

uint32_t nicUniCmdSetDomainV2(struct ADAPTER *ad,
		struct WIFI_UNI_SETQUERY_INFO *info)
{
	struct CMD_SET_DOMAIN_INFO_V2 *cmd;
	struct UNI_CMD_DOMAIN_SET_INFO *uni_cmd;
	struct UNI_CMD_DOMAIN_SET_INFO_DOMAIN_ACTIVE_CHANNEL_LIST *tag;
	struct CMD_DOMAIN_CHANNEL *chl_dst, *chl_src;
	struct WIFI_UNI_CMD_ENTRY *entry;
	uint32_t max_cmd_len;
	uint8_t i;
	uint8_t valid_channel_count = 0;

	cmd = (struct CMD_SET_DOMAIN_INFO_V2 *) info->pucInfoBuffer;
	valid_channel_count = cmd->arActiveChannels.u1ActiveChNum2g +
			      cmd->arActiveChannels.u1ActiveChNum5g;
	max_cmd_len = sizeof(struct UNI_CMD_DOMAIN_SET_INFO) +
	  sizeof(struct UNI_CMD_DOMAIN_SET_INFO_DOMAIN_ACTIVE_CHANNEL_LIST) +
	  (valid_channel_count * sizeof(struct CMD_DOMAIN_CHANNEL));
	entry = nicUniCmdAllocEntry(ad, UNI_CMD_ID_SET_DOMAIN_INFO,
					max_cmd_len, NULL, NULL);

	if (!entry)
		return WLAN_STATUS_RESOURCES;

	uni_cmd = (struct UNI_CMD_DOMAIN_SET_INFO *) entry->pucInfoBuffer;
	uni_cmd->u4CountryCode = cmd->u4CountryCode;
	uni_cmd->uc2G4Bandwidth = cmd->uc2G4Bandwidth;
	uni_cmd->uc5GBandwidth = cmd->uc5GBandwidth;
	uni_cmd->uc6GBandwidth = cmd->uc5GBandwidth; /* TODO: uni cmd, 6g */

	tag = (struct UNI_CMD_DOMAIN_SET_INFO_DOMAIN_ACTIVE_CHANNEL_LIST *)
		uni_cmd->aucTlvBuffer;
	tag->u2Tag = UNI_CMD_DOMAIN_TAG_ACTCHNL;
	tag->u2Length = sizeof(*tag) +
			sizeof(struct CMD_DOMAIN_CHANNEL) * valid_channel_count;
	tag->u1ActiveChNum2g = cmd->arActiveChannels.u1ActiveChNum2g;
	tag->u1ActiveChNum5g = cmd->arActiveChannels.u1ActiveChNum5g;

	chl_dst = (struct CMD_DOMAIN_CHANNEL *) tag->aucActChnlListBuffer;
	chl_src = cmd->arActiveChannels.arChannels;
	for (i = 0; i < valid_channel_count; i++) {
		kalMemCopy(chl_dst, chl_src, sizeof(*chl_dst));
		chl_dst++;
		chl_src++;
	}

	LINK_INSERT_TAIL(&info->rUniCmdList, &entry->rLinkEntry);

	return WLAN_STATUS_SUCCESS;
}

uint32_t nicUniCmdSetDomain(struct ADAPTER *ad,
		struct WIFI_UNI_SETQUERY_INFO *info)
{
	if (info->ucCID != CMD_ID_SET_DOMAIN_INFO)
		return WLAN_STATUS_NOT_ACCEPTED;

	if (info->u4SetQueryInfoLen == sizeof(struct CMD_SET_DOMAIN_INFO))
		return nicUniCmdSetDomainV1(ad, info);
	else
		return nicUniCmdSetDomainV2(ad, info);
}

uint32_t nicUniCmdScanCancel(struct ADAPTER *ad,
		struct WIFI_UNI_SETQUERY_INFO *info)
{
	struct CMD_SCAN_CANCEL *cmd;
	struct UNI_CMD_SCAN *uni_cmd;
	struct UNI_CMD_SCAN_CANCEL *tag;
	struct WIFI_UNI_CMD_ENTRY *entry;
	uint32_t max_cmd_len = sizeof(struct UNI_CMD_SCAN) +
			       sizeof(struct UNI_CMD_SCAN_CANCEL);

	if (info->ucCID != CMD_ID_SCAN_CANCEL ||
	    info->u4SetQueryInfoLen != sizeof(*cmd))
		return WLAN_STATUS_NOT_ACCEPTED;

	cmd = (struct CMD_SCAN_CANCEL *) info->pucInfoBuffer;
	entry = nicUniCmdAllocEntry(ad, UNI_CMD_ID_SCAN_REQ,
					max_cmd_len, NULL, NULL);

	if (!entry)
		return WLAN_STATUS_RESOURCES;

	uni_cmd = (struct UNI_CMD_SCAN *) entry->pucInfoBuffer;
	uni_cmd->ucSeqNum = cmd->ucSeqNum;
	uni_cmd->ucBssIndex = 0; /* unused */
	tag = (struct UNI_CMD_SCAN_CANCEL *) uni_cmd->aucTlvBuffer;
	tag->u2Tag = UNI_CMD_SCAN_TAG_SCAN_CANCEL;
	tag->u2Length = sizeof(*tag);
	tag->ucIsExtChannel = cmd->ucIsExtChannel;

	LINK_INSERT_TAIL(&info->rUniCmdList, &entry->rLinkEntry);

	return WLAN_STATUS_SUCCESS;
}

uint32_t nicUniCmdWsysFwLogUI(struct ADAPTER *ad,
		struct WIFI_UNI_SETQUERY_INFO *info)
{
	struct CMD_EVENT_LOG_UI_INFO *cmd;
	struct UNI_CMD_WSYS_CONFIG *uni_cmd;
	struct UNI_CMD_WSYS_CONFIG_FW_LOG_UI_CTRL *tag;
	struct WIFI_UNI_CMD_ENTRY *entry;
	uint32_t max_cmd_len = sizeof(struct UNI_CMD_WSYS_CONFIG) +
			      sizeof(struct UNI_CMD_WSYS_CONFIG_FW_LOG_UI_CTRL);

	if (info->ucCID != CMD_ID_LOG_UI_INFO ||
	    info->u4SetQueryInfoLen != sizeof(*cmd))
		return WLAN_STATUS_NOT_ACCEPTED;

	cmd = (struct CMD_EVENT_LOG_UI_INFO *) info->pucInfoBuffer;
	entry = nicUniCmdAllocEntry(ad, UNI_CMD_ID_WSYS_CONFIG,
		  max_cmd_len, nicUniCmdEventSetCommon, nicUniCmdTimeoutCommon);

	if (!entry)
		return WLAN_STATUS_RESOURCES;

	uni_cmd = (struct UNI_CMD_WSYS_CONFIG *) entry->pucInfoBuffer;
	tag = (struct UNI_CMD_WSYS_CONFIG_FW_LOG_UI_CTRL *)
						uni_cmd->aucTlvBuffer;
	tag->u2Tag = UNI_CMD_WSYS_CONFIG_TAG_FW_LOG_UI_CTRL;
	tag->u2Length = sizeof(*tag);
	tag->ucVersion = cmd->ucVersion;
	tag->ucLogLevel = cmd->ucLogLevel;

	LINK_INSERT_TAIL(&info->rUniCmdList, &entry->rLinkEntry);

	return WLAN_STATUS_SUCCESS;
}

uint32_t nicUniCmdWsysFwBasicConfig(struct ADAPTER *ad,
		struct WIFI_UNI_SETQUERY_INFO *info)
{
	struct CMD_BASIC_CONFIG *cmd;
	struct UNI_CMD_WSYS_CONFIG *uni_cmd;
	struct UNI_CMD_WSYS_CONFIG_FW_BASIC_CONFIG *tag;
	struct WIFI_UNI_CMD_ENTRY *entry;
	uint32_t max_cmd_len = sizeof(struct UNI_CMD_WSYS_CONFIG) +
			     sizeof(struct UNI_CMD_WSYS_CONFIG_FW_BASIC_CONFIG);

	if (info->ucCID != CMD_ID_BASIC_CONFIG ||
	    info->u4SetQueryInfoLen != sizeof(*cmd))
		return WLAN_STATUS_NOT_ACCEPTED;

	cmd = (struct CMD_BASIC_CONFIG *) info->pucInfoBuffer;
	entry = nicUniCmdAllocEntry(ad, UNI_CMD_ID_WSYS_CONFIG,
		  max_cmd_len, nicUniCmdEventSetCommon, nicUniCmdTimeoutCommon);

	if (!entry)
		return WLAN_STATUS_RESOURCES;

	uni_cmd = (struct UNI_CMD_WSYS_CONFIG *) entry->pucInfoBuffer;
	tag = (struct UNI_CMD_WSYS_CONFIG_FW_BASIC_CONFIG *)
						uni_cmd->aucTlvBuffer;
	tag->u2Tag = UNI_CMD_WSYS_CONFIG_TAG_FW_BASIC_CONFIG;
	tag->u2Length = sizeof(*tag);
	tag->u2RxChecksum = cmd->rCsumOffload.u2RxChecksum;
	tag->u2TxChecksum = cmd->rCsumOffload.u2TxChecksum;
	tag->ucCtrlFlagAssertPath = cmd->ucCtrlFlagAssertPath;

	LINK_INSERT_TAIL(&info->rUniCmdList, &entry->rLinkEntry);

	return WLAN_STATUS_SUCCESS;
}

uint32_t nicUniCmdSetSuspendMode(struct ADAPTER *ad,
		struct WIFI_UNI_SETQUERY_INFO *info)
{
	struct CMD_SUSPEND_MODE_SETTING *cmd;
	struct UNI_CMD_SUSPEND *uni_cmd;
	struct UNI_CMD_SUSPEND_MODE_SETTING *tag;
	struct WIFI_UNI_CMD_ENTRY *entry;
	uint32_t max_cmd_len = sizeof(struct UNI_CMD_SUSPEND) +
			       sizeof(struct UNI_CMD_SUSPEND_MODE_SETTING);

	if (info->ucCID != CMD_ID_SET_SUSPEND_MODE ||
	    info->u4SetQueryInfoLen != sizeof(*cmd))
		return WLAN_STATUS_NOT_ACCEPTED;

	cmd = (struct CMD_SUSPEND_MODE_SETTING *) info->pucInfoBuffer;
	entry = nicUniCmdAllocEntry(ad, UNI_CMD_ID_SUSPEND,
		  max_cmd_len, nicUniCmdEventSetCommon, nicUniCmdTimeoutCommon);

	if (!entry)
		return WLAN_STATUS_RESOURCES;

	uni_cmd = (struct UNI_CMD_SUSPEND *) entry->pucInfoBuffer;
	uni_cmd->ucBssInfoIdx = cmd->ucBssIndex;
	tag = (struct UNI_CMD_SUSPEND_MODE_SETTING *) uni_cmd->aucTlvBuffer;
	tag->u2Tag = UNI_CMD_SUSPEND_TAG_MODE_SETTING;
	tag->u2Length = sizeof(*tag);
	tag->ucScreenStatus = cmd->ucEnableSuspendMode;
	tag->ucMdtim = cmd->ucMdtim;
	tag->ucWowSuspend = FALSE; /*unset */

	LINK_INSERT_TAIL(&info->rUniCmdList, &entry->rLinkEntry);

	return WLAN_STATUS_SUCCESS;
}

void nicUniCmdWowEventSetCb(IN struct ADAPTER *prAdapter,
			IN struct CMD_INFO *prCmdInfo, IN uint8_t *pucEventBuf)
{
	if (prCmdInfo->ucCID == UNI_CMD_ID_SUSPEND) {
		DBGLOG(INIT, STATE, "CMD_ID_SET_WOWLAN cmd done\n");
		prAdapter->fgSetWowDone = TRUE;
	}
}

uint32_t nicUniCmdSetWOWLAN(struct ADAPTER *ad,
		struct WIFI_UNI_SETQUERY_INFO *info)
{
	struct CMD_WOWLAN_PARAM *cmd;
	struct UNI_CMD_SUSPEND *uni_cmd;
	struct UNI_CMD_SUSPEND_WOW_CTRL *ctrl_tag;
	struct UNI_CMD_SUSPEND_WOW_GPIO_PARAM *gpio_tag;
	struct UNI_CMD_SUSPEND_WOW_WAKEUP_PORT *port_tag;
	struct WIFI_UNI_CMD_ENTRY *entry;
	uint32_t max_cmd_len = sizeof(struct UNI_CMD_WSYS_CONFIG) +
			       sizeof(struct UNI_CMD_SUSPEND_WOW_CTRL) +
			       sizeof(struct UNI_CMD_SUSPEND_WOW_GPIO_PARAM) +
			       sizeof(struct UNI_CMD_SUSPEND_WOW_WAKEUP_PORT);
	uint8_t *pos;

	if (info->ucCID != CMD_ID_SET_WOWLAN ||
	    info->u4SetQueryInfoLen != sizeof(*cmd))
		return WLAN_STATUS_NOT_ACCEPTED;

	cmd = (struct CMD_WOWLAN_PARAM *) info->pucInfoBuffer;
	entry = nicUniCmdAllocEntry(ad, UNI_CMD_ID_SUSPEND,
		  max_cmd_len, nicUniCmdWowEventSetCb, nicUniCmdTimeoutCommon);

	if (!entry)
		return WLAN_STATUS_RESOURCES;

	uni_cmd = (struct UNI_CMD_SUSPEND *) entry->pucInfoBuffer;
	uni_cmd->ucBssInfoIdx = 0; /* unused */
	pos = uni_cmd->aucTlvBuffer;

	ctrl_tag = (struct UNI_CMD_SUSPEND_WOW_CTRL *) pos;
	ctrl_tag->u2Tag = UNI_CMD_SUSPEND_TAG_WOW_CTRL;
	ctrl_tag->u2Length = sizeof(*ctrl_tag);
	ctrl_tag->ucCmd = cmd->ucCmd;
	ctrl_tag->ucDetectType = cmd->ucDetectType;
	ctrl_tag->ucWakeupHif = cmd->astWakeHif[0].ucWakeupHif;
	pos += sizeof(*ctrl_tag);

	gpio_tag = (struct UNI_CMD_SUSPEND_WOW_GPIO_PARAM *) pos;
	gpio_tag->u2Tag = UNI_CMD_SUSPEND_TAG_WOW_GPIO_PARAM;
	gpio_tag->u2Length = sizeof(*gpio_tag);
	gpio_tag->ucGpioPin = cmd->astWakeHif[0].ucGpioPin;
	gpio_tag->ucTriggerLvl = cmd->astWakeHif[0].ucTriggerLvl;
	gpio_tag->u4GpioInterval = cmd->astWakeHif[0].u4GpioInterval;
	pos += sizeof(*gpio_tag);

	port_tag = (struct UNI_CMD_SUSPEND_WOW_WAKEUP_PORT *) pos;
	port_tag->u2Tag = UNI_CMD_SUSPEND_TAG_WOW_WAKEUP_PORT;
	port_tag->u2Length = sizeof(*port_tag);
	port_tag->ucIPv4UdpPortCnt = cmd->stWowPort.ucIPv4UdpPortCnt;
	port_tag->ucIPv4TcpPortCnt = cmd->stWowPort.ucIPv4TcpPortCnt;
	port_tag->ucIPv4UdpPortCnt = cmd->stWowPort.ucIPv4UdpPortCnt;
	port_tag->ucIPv4TcpPortCnt = cmd->stWowPort.ucIPv4TcpPortCnt;
	kalMemCopy(port_tag->ausIPv4UdpPort, cmd->stWowPort.ausIPv4UdpPort,
		sizeof(port_tag->ausIPv4UdpPort));
	kalMemCopy(port_tag->ausIPv4TcpPort, cmd->stWowPort.ausIPv4TcpPort,
		sizeof(port_tag->ausIPv4TcpPort));
	kalMemCopy(port_tag->ausIPv6UdpPort, cmd->stWowPort.ausIPv6UdpPort,
		sizeof(port_tag->ausIPv6UdpPort));
	kalMemCopy(port_tag->ausIPv6TcpPort, cmd->stWowPort.ausIPv6TcpPort,
		sizeof(port_tag->ausIPv6TcpPort));
	pos += sizeof(*port_tag);

	LINK_INSERT_TAIL(&info->rUniCmdList, &entry->rLinkEntry);

	return WLAN_STATUS_SUCCESS;
}

void nicUniCmdNicCapability(struct ADAPTER *prAdapter)
{
	struct UNI_CMD_CHIP_CONFIG *uni_cmd;
	struct UNI_CMD_CHIP_CONFIG_NIC_CAPABILITY *tag;
	uint32_t max_cmd_len = sizeof(struct UNI_CMD_CHIP_CONFIG) +
			      sizeof(struct UNI_CMD_CHIP_CONFIG_NIC_CAPABILITY);

	uni_cmd = (struct UNI_CMD_CHIP_CONFIG *)
		cnmMemAlloc(prAdapter, RAM_TYPE_BUF, max_cmd_len);
	tag = (struct UNI_CMD_CHIP_CONFIG_NIC_CAPABILITY *)
			uni_cmd->aucTlvBuffer;
	tag->u2Tag = UNI_CMD_CHIP_CONFIG_TAG_NIC_CAPABILITY;
	tag->u2Length = sizeof(*tag);

	wlanSendSetQueryUniCmdAdv(
		prAdapter, UNI_CMD_ID_CHIP_CONFIG, FALSE,
		TRUE, FALSE, NULL, NULL, max_cmd_len,
		(uint8_t *) uni_cmd, NULL, 0,
		CMD_SEND_METHOD_REQ_RESOURCE);

	cnmMemFree(prAdapter, uni_cmd);
}

void nicUniCmdEventQueryNicCapabilityV2(IN struct ADAPTER *ad,
				     IN struct WIFI_UNI_EVENT *evt)
{
	int32_t tags_len;
	uint8_t *tag;
	uint16_t offset = 0;
	uint32_t fixed_len = sizeof(struct UNI_EVENT_CHIP_CAPABILITY);
	uint32_t data_len = GET_UNI_EVENT_DATA_LEN(evt);
	uint8_t *data = GET_UNI_EVENT_DATA(evt);

	/* copy tag to legacy event */
	tags_len = data_len - fixed_len;
	tag = data + fixed_len;
	TAG_FOR_EACH(tag, tags_len, offset) {
		DBGLOG(NIC, TRACE, "Tag(%d, %d)\n", TAG_ID(tag), TAG_LEN(tag));
		nicParsingNicCapV2(ad, TAG_ID(tag), TAG_DATA(tag));
	}
}

uint32_t wlanQueryNicCapabilityV2(IN struct ADAPTER *prAdapter)
{
	uint32_t u4RxPktLength;
	uint8_t *prEventBuff;
	struct WIFI_UNI_EVENT *prEvent;
	struct mt66xx_chip_info *prChipInfo;
	uint32_t chip_id;
	uint32_t u4Time;

	ASSERT(prAdapter);
	prChipInfo = prAdapter->chip_info;
	chip_id = prChipInfo->chip_id;

	/* Get Nic resource information from FW */
	if (!prChipInfo->isNicCapV1
	    || (prAdapter->u4FwFeatureFlag0 &
		FEATURE_FLAG0_NIC_CAPABILITY_V2)) {

		DBGLOG(INIT, INFO, "Support NIC_CAPABILITY_V2 feature\n");

		/* get nic capability */
		nicUniCmdNicCapability(prAdapter);

		/*
		 * receive UNI_EVENT_ID_CHIP_CONFIG
		 */
		/* allocate event buffer */
		prEventBuff = cnmMemAlloc(prAdapter, RAM_TYPE_BUF,
					  CFG_RX_MAX_PKT_SIZE);
		if (!prEventBuff) {
			DBGLOG(INIT, WARN, "event buffer alloc failed!\n");
			return WLAN_STATUS_FAILURE;
		}

		/* get event */
		u4Time = kalGetTimeTick();
		while (TRUE) {
			if (nicRxWaitResponse(prAdapter,
					      1,
					      prEventBuff,
					      CFG_RX_MAX_PKT_SIZE,
					      &u4RxPktLength)
			    != WLAN_STATUS_SUCCESS) {
				DBGLOG(INIT, WARN,
				       "wait for event failed!\n");

				/* free event buffer */
				cnmMemFree(prAdapter, prEventBuff);

				return WLAN_STATUS_FAILURE;
			}

			if (CHECK_FOR_TIMEOUT(kalGetTimeTick(), u4Time,
				MSEC_TO_SYSTIME(3000))) {
				DBGLOG(HAL, ERROR,
					"Query nic capability timeout\n");
				return WLAN_STATUS_FAILURE;
			}

			/* header checking .. */
			if ((NIC_RX_GET_U2_SW_PKT_TYPE(prEventBuff) &
				prChipInfo->u2RxSwPktBitMap) !=
				prChipInfo->u2RxSwPktEvent) {
				DBGLOG(INIT, WARN,
				       "skip unexpected Rx pkt type[0x%04x]\n",
				       NIC_RX_GET_U2_SW_PKT_TYPE(prEventBuff));

				continue;
			}

			prEvent = (struct WIFI_UNI_EVENT *)
				(prEventBuff + prChipInfo->rxd_size);

			if (!IS_UNI_EVENT(prEvent) ||
			    prEvent->ucEID != UNI_EVENT_ID_CHIP_CAPABILITY) {
				DBGLOG(INIT, WARN,
				       "skip unexpected event ID[0x%02x] option:%d\n",
					prEvent->ucEID,
					GET_UNI_EVENT_OPTION(prEvent));

				continue;
			} else {
				/* hit */
				break;
			}

		}

		nicUniCmdEventQueryNicCapabilityV2(prAdapter, prEvent);

		/*
		 * free event buffer
		 */
		cnmMemFree(prAdapter, prEventBuff);
	}

	/* Fill capability for different Chip version */
	if (chip_id == HQA_CHIP_ID_6632) {
		/* 6632 only */
		prAdapter->fgIsSupportBufferBinSize16Byte = TRUE;
		prAdapter->fgIsSupportDelayCal = FALSE;
		prAdapter->fgIsSupportGetFreeEfuseBlockCount = FALSE;
		prAdapter->fgIsSupportQAAccessEfuse = FALSE;
		prAdapter->fgIsSupportPowerOnSendBufferModeCMD = FALSE;
		prAdapter->fgIsSupportGetTxPower = FALSE;
	} else {
		prAdapter->fgIsSupportBufferBinSize16Byte = FALSE;
		prAdapter->fgIsSupportDelayCal = TRUE;
		prAdapter->fgIsSupportGetFreeEfuseBlockCount = TRUE;
		prAdapter->fgIsSupportQAAccessEfuse = TRUE;
		prAdapter->fgIsSupportPowerOnSendBufferModeCMD = TRUE;
		prAdapter->fgIsSupportGetTxPower = TRUE;
	}

	return WLAN_STATUS_SUCCESS;
}

uint32_t nicUniCmdRemoveStaRec(struct ADAPTER *ad,
		struct WIFI_UNI_SETQUERY_INFO *info)
{
	struct CMD_REMOVE_STA_RECORD *cmd;
	struct UNI_CMD_STAREC *uni_cmd;
	struct UNI_CMD_STAREC_REMOVE_INFO *tag;
	struct WIFI_UNI_CMD_ENTRY *entry;
	uint32_t max_cmd_len = sizeof(struct UNI_CMD_STAREC) +
			       sizeof(struct UNI_CMD_STAREC_REMOVE_INFO);
	uint8_t widx;

	if (info->ucCID != CMD_ID_REMOVE_STA_RECORD ||
	    info->u4SetQueryInfoLen != sizeof(*cmd))
		return WLAN_STATUS_NOT_ACCEPTED;

	cmd = (struct CMD_REMOVE_STA_RECORD *) info->pucInfoBuffer;
	if (cmd->ucActionType == STA_REC_CMD_ACTION_STA ||
	    cmd->ucActionType == STA_REC_CMD_ACTION_BSS_EXCLUDE_STA) {
		/* don't use cnmGetStaRecByIndex becayse starec is not in-use */
		if (cmd->ucStaIndex < CFG_STA_REC_NUM)
			widx = ad->arStaRec[cmd->ucStaIndex].ucWlanIndex;
		else
			return WLAN_STATUS_INVALID_DATA;
	} else {
		/* remove by bss, widx is not checked */
		widx = 0;
	}

	entry = nicUniCmdAllocEntry(ad, UNI_CMD_ID_STAREC_INFO,
			max_cmd_len, NULL, NULL);
	if (!entry)
		return WLAN_STATUS_RESOURCES;

	uni_cmd = (struct UNI_CMD_STAREC *) entry->pucInfoBuffer;
	uni_cmd->ucBssInfoIdx = cmd->ucBssIndex;
	WCID_SET_H_L(uni_cmd->ucWlanIdxHnVer, uni_cmd->ucWlanIdxL, widx);

	tag = (struct UNI_CMD_STAREC_REMOVE_INFO *) uni_cmd->aucTlvBuffer;
	tag->u2Tag = UNI_CMD_STAREC_TAG_REMOVE;
	tag->u2Length = sizeof(*tag);
	tag->ucActionType = cmd->ucActionType;

	LINK_INSERT_TAIL(&info->rUniCmdList, &entry->rLinkEntry);

	return WLAN_STATUS_SUCCESS;
}

uint32_t nicUniCmdBcnContent(struct ADAPTER *ad,
		struct WIFI_UNI_SETQUERY_INFO *info)
{
	struct UNI_CMD_BSSINFO *uni_cmd;
	struct UNI_CMD_BSSINFO_BCN_CONTENT *tag;
	struct WIFI_UNI_CMD_ENTRY *entry;
	struct BSS_INFO *bss;
	struct WLAN_BEACON_FRAME *bcn;
	uint32_t max_cmd_len = sizeof(struct UNI_CMD_BSSINFO) +
			       sizeof(struct UNI_CMD_BSSINFO_BCN_CONTENT);
	uint16_t bcn_len;
	uint8_t bss_idx;

	if (info->ucCID != CMD_ID_INDICATE_PM_BSS_CREATED ||
	    info->ucCID != CMD_ID_UPDATE_BEACON_CONTENT)
		return WLAN_STATUS_NOT_ACCEPTED;

	if (info->ucCID == CMD_ID_INDICATE_PM_BSS_CREATED)
		bss_idx = ((struct CMD_INDICATE_PM_BSS_CREATED *)
				info->pucInfoBuffer)->ucBssIndex;
	else
		bss_idx = ((struct CMD_BEACON_TEMPLATE_UPDATE *)
				info->pucInfoBuffer)->ucBssIndex;
	bss = GET_BSS_INFO_BY_INDEX(ad, bss_idx);
	if (!bss->prBeacon)
		return WLAN_STATUS_INVALID_DATA;

	bcn = (struct WLAN_BEACON_FRAME *)bss->prBeacon->prPacket;
	bcn_len = ALIGN_4(bss->prBeacon->u2FrameLength -
		OFFSET_OF(struct WLAN_BEACON_FRAME, u2CapInfo));
	max_cmd_len += bcn_len;
	entry = nicUniCmdAllocEntry(ad, UNI_CMD_ID_BSSINFO,
			max_cmd_len, NULL, NULL);
	if (!entry)
		return WLAN_STATUS_RESOURCES;

	uni_cmd = (struct UNI_CMD_BSSINFO *) entry->pucInfoBuffer;
	uni_cmd->ucBssInfoIdx = bss_idx;
	tag = (struct UNI_CMD_BSSINFO_BCN_CONTENT *) uni_cmd->aucTlvBuffer;
	tag->u2Tag = UNI_CMD_BSSINFO_TAG_BCN_CONTENT;
	tag->u2Length = sizeof(*tag) + bcn_len;
	tag->ucAction = BCN_ACTION_ENABLE;
	tag->u2PktLength = bcn_len;
	/* the aucPktContent field only include
	 * capablity field and followed IEs
	 */
	tag->aucPktContentType = 1;
	kalMemCopy(tag->aucPktContent, &bcn->u2CapInfo, bcn_len);

	LINK_INSERT_TAIL(&info->rUniCmdList, &entry->rLinkEntry);

	return WLAN_STATUS_SUCCESS;
}

uint32_t nicUniCmdPmDisable(struct ADAPTER *ad,
		struct WIFI_UNI_SETQUERY_INFO *info)
{
	struct CMD_INDICATE_PM_BSS_ABORT *cmd;
	struct UNI_CMD_BSSINFO *uni_cmd;
	struct UNI_CMD_BSSINFO_STA_PM_DISABLE *tag;
	struct WIFI_UNI_CMD_ENTRY *entry;
	uint32_t max_cmd_len = sizeof(struct UNI_CMD_BSSINFO) +
			       sizeof(struct UNI_CMD_BSSINFO_STA_PM_DISABLE);

	if (info->ucCID != CMD_ID_INDICATE_PM_BSS_ABORT ||
	    info->u4SetQueryInfoLen != sizeof(*cmd))
		return WLAN_STATUS_NOT_ACCEPTED;

	cmd = (struct CMD_INDICATE_PM_BSS_ABORT *) info->pucInfoBuffer;
	entry = nicUniCmdAllocEntry(ad, UNI_CMD_ID_BSSINFO,
			max_cmd_len, NULL, NULL);
	if (!entry)
		return WLAN_STATUS_RESOURCES;

	uni_cmd = (struct UNI_CMD_BSSINFO *) entry->pucInfoBuffer;
	uni_cmd->ucBssInfoIdx = cmd->ucBssIndex;
	tag = (struct UNI_CMD_BSSINFO_STA_PM_DISABLE *) uni_cmd->aucTlvBuffer;
	tag->u2Tag = UNI_CMD_BSSINFO_TAG_PM_DISABLE;
	tag->u2Length = sizeof(*tag);

	LINK_INSERT_TAIL(&info->rUniCmdList, &entry->rLinkEntry);

	return WLAN_STATUS_SUCCESS;
}

uint32_t nicUniCmdPmEnable(struct ADAPTER *ad,
		struct WIFI_UNI_SETQUERY_INFO *info)
{
	struct CMD_INDICATE_PM_BSS_CONNECTED *cmd;
	struct UNI_CMD_BSSINFO *uni_cmd;
	struct UNI_CMD_BSSINFO_STA_PM_ENABLE *tag;
	struct WIFI_UNI_CMD_ENTRY *entry;
	uint32_t max_cmd_len = sizeof(struct UNI_CMD_BSSINFO) +
			       sizeof(struct UNI_CMD_BSSINFO_STA_PM_ENABLE);

	if (info->ucCID != CMD_ID_INDICATE_PM_BSS_CONNECTED ||
	    info->u4SetQueryInfoLen != sizeof(*cmd))
		return WLAN_STATUS_NOT_ACCEPTED;

	cmd = (struct CMD_INDICATE_PM_BSS_CONNECTED *) info->pucInfoBuffer;
	entry = nicUniCmdAllocEntry(ad, UNI_CMD_ID_BSSINFO,
			max_cmd_len, NULL, NULL);
	if (!entry)
		return WLAN_STATUS_RESOURCES;

	uni_cmd = (struct UNI_CMD_BSSINFO *) entry->pucInfoBuffer;
	uni_cmd->ucBssInfoIdx = cmd->ucBssIndex;
	tag = (struct UNI_CMD_BSSINFO_STA_PM_ENABLE *) uni_cmd->aucTlvBuffer;
	tag->u2Tag = UNI_CMD_BSSINFO_TAG_PM_ENABLE;
	tag->u2Length = sizeof(*tag);
	tag->u2BcnInterval = cmd->u2BeaconInterval;
	tag->ucDtimPeriod = cmd->ucDtimPeriod;

	LINK_INSERT_TAIL(&info->rUniCmdList, &entry->rLinkEntry);

	return WLAN_STATUS_SUCCESS;
}

uint32_t nicUniCmdBssInfoTagBasic(struct ADAPTER *ad,
	uint8_t *buf, struct CMD_SET_BSS_INFO *cmd)
{
	struct UNI_CMD_BSSINFO_BASIC *tag =
		(struct UNI_CMD_BSSINFO_BASIC *)buf;
	struct BSS_INFO *bss = GET_BSS_INFO_BY_INDEX(ad, cmd->ucBssIndex);
	uint32_t phy_mode;

	tag->u2Tag = UNI_CMD_BSSINFO_TAG_BASIC;
	tag->u2Length = sizeof(*tag);
	tag->ucActive = IS_BSS_ACTIVE(bss);
	tag->ucOwnMacIdx = bss->ucOwnMacIndex;
	tag->ucHwBSSIndex = bss->ucOwnMacIndex;
	tag->ucDbdcIdx = cmd->ucDBDCBand;
	tag->u4ConnectionType = nicUniCmdBssInfoConnType(ad, bss);
	tag->ucConnectionState = bss->eConnectionState;
	tag->ucWmmIdx = cmd->ucWmmSet;
	COPY_MAC_ADDR(tag->aucBSSID, cmd->aucBSSID);
	tag->u2BcMcWlanidx = cmd->ucBMCWlanIndex;
	tag->u2BcnInterval = bss->u2BeaconInterval;
	tag->ucDtimPeriod = bss->ucDTIMPeriod;
	tag->u2StaRecIdxOfAP = secGetWlanIdxByStaIdx(ad, cmd->ucStaRecIdxOfAP);
	tag->u2NonHTBasicPhyType = cmd->ucNonHTBasicPhyType;
	phy_mode = nicUniCmdBssInfoPhyMode(cmd->ucPhyTypeSet);
	tag->ucPhyMode = phy_mode & 0xff;
	tag->ucPhyModeExt = (phy_mode >> 8) & 0xff;

	return tag->u2Length;
}

uint32_t nicUniCmdSetBssRlmImpl(struct ADAPTER *ad,
	uint8_t *buf, struct CMD_SET_BSS_RLM_PARAM *cmd)
{
	struct UNI_CMD_BSSINFO_RLM *rlm_tag;
	struct UNI_CMD_BSSINFO_PROTECT *prot_tag;
	struct UNI_CMD_BSSINFO_IFS_TIME *ifs_tag;
	uint8_t *pos = buf;

	rlm_tag = (struct UNI_CMD_BSSINFO_RLM *) pos;
	rlm_tag->u2Tag = UNI_CMD_BSSINFO_TAG_RLM;
	rlm_tag->u2Length = sizeof(*rlm_tag);
	rlm_tag->ucPrimaryChannel = cmd->ucPrimaryChannel;
	rlm_tag->ucCenterChannelSeg0 = cmd->ucVhtChannelFrequencyS1;
	rlm_tag->ucCenterChannelSeg1 = cmd->ucVhtChannelFrequencyS2;
	rlm_tag->ucTxStream = cmd->ucTxNss;
	rlm_tag->ucRxStream = cmd->ucRxNss;
	rlm_tag->ucHtOpInfo1 = cmd->ucHtOpInfo1;
	rlm_tag->ucSCO = cmd->ucRfSco;
	rlm_tag->ucBandwidth = BW_20;
	switch (cmd->ucVhtChannelWidth) {
	case VHT_OP_CHANNEL_WIDTH_80P80:
		rlm_tag->ucBandwidth = BW_8080;
		break;
	case VHT_OP_CHANNEL_WIDTH_160:
		rlm_tag->ucBandwidth = BW_160;
		break;
	case VHT_OP_CHANNEL_WIDTH_80:
		rlm_tag->ucBandwidth = BW_80;
		break;
	case VHT_OP_CHANNEL_WIDTH_20_40:
		if (cmd->ucRfSco != CHNL_EXT_SCN)
			rlm_tag->ucBandwidth = BW_40;
		break;
	default:
		break;
	}
	pos += sizeof(*rlm_tag);

	prot_tag = (struct UNI_CMD_BSSINFO_PROTECT *) pos;
	prot_tag->u2Tag = UNI_CMD_BSSINFO_TAG_PROTECT;
	prot_tag->u2Length = sizeof(*prot_tag);
	if (cmd->ucErpProtectMode)
		prot_tag->u4ProtectMode |= LEGACY_ERP_PROTECT;
	switch (cmd->ucHtProtectMode) {
	case HT_PROTECT_MODE_NON_MEMBER:
		prot_tag->u4ProtectMode |= HT_NON_MEMBER_PROTECT;
		break;
	case HT_PROTECT_MODE_20M:
		prot_tag->u4ProtectMode |= HT_BW20_PROTECT;
		break;
	case HT_PROTECT_MODE_NON_HT:
		prot_tag->u4ProtectMode |= HT_NON_HT_MIXMODE_PROTECT;
		break;
	default:
		break;
	}
	if (cmd->ucGfOperationMode == GF_MODE_PROTECT)
		prot_tag->u4ProtectMode |= VEND_GREEN_FIELD_PROTECT;
	pos += sizeof(*prot_tag);

	ifs_tag = (struct UNI_CMD_BSSINFO_IFS_TIME *) pos;
	ifs_tag->u2Tag = UNI_CMD_BSSINFO_TAG_IFS_TIME;
	ifs_tag->u2Length = sizeof(*ifs_tag);
	ifs_tag->fgSlotValid = TRUE;
	if (cmd->ucUseShortSlotTime)
		ifs_tag->u2SlotTime = 9;
	else
		ifs_tag->u2SlotTime = 20;
	pos += sizeof(*ifs_tag);

	return pos - buf;
}

uint32_t nicUniCmdBssInfoTagRlm(struct ADAPTER *ad,
	uint8_t *buf, struct CMD_SET_BSS_INFO *cmd)
{
	return nicUniCmdSetBssRlmImpl(ad, buf, &cmd->rBssRlmParam);
}

uint32_t nicUniCmdBssInfoTagRate(struct ADAPTER *ad,
	uint8_t *buf, struct CMD_SET_BSS_INFO *cmd)
{
	struct UNI_CMD_BSSINFO_RATE *tag = (struct UNI_CMD_BSSINFO_RATE *)buf;

	tag->u2Tag = UNI_CMD_BSSINFO_TAG_RATE;
	tag->u2Length = sizeof(*tag);
	tag->u2OperationalRateSet = cmd->u2OperationalRateSet;
	tag->u2BSSBasicRateSet = cmd->u2BSSBasicRateSet;

	return tag->u2Length;
}

uint32_t nicUniCmdBssInfoTagSec(struct ADAPTER *ad,
	uint8_t *buf, struct CMD_SET_BSS_INFO *cmd)
{
	struct UNI_CMD_BSSINFO_SEC *tag = (struct UNI_CMD_BSSINFO_SEC *)buf;

	tag->u2Tag = UNI_CMD_BSSINFO_TAG_SEC;
	tag->u2Length = sizeof(*tag);
	tag->ucAuthMode = cmd->ucAuthMode;
	tag->ucEncStatus = cmd->ucEncStatus;

	return tag->u2Length;
}

uint32_t nicUniCmdBssInfoTagQbss(struct ADAPTER *ad,
	uint8_t *buf, struct CMD_SET_BSS_INFO *cmd)
{
	struct UNI_CMD_BSSINFO_QBSS *tag = (struct UNI_CMD_BSSINFO_QBSS *)buf;

	tag->u2Tag = UNI_CMD_BSSINFO_TAG_QBSS;
	tag->u2Length = sizeof(*tag);
	tag->ucIsQBSS = cmd->ucIsQBSS;
	return tag->u2Length;
}

uint32_t nicUniCmdBssInfoTagSap(struct ADAPTER *ad,
	uint8_t *buf, struct CMD_SET_BSS_INFO *cmd)
{
	struct UNI_CMD_BSSINFO_SAP *tag = (struct UNI_CMD_BSSINFO_SAP *)buf;

	tag->u2Tag = UNI_CMD_BSSINFO_TAG_SAP;
	tag->u2Length = sizeof(*tag);
	tag->fgIsHiddenSSID = cmd->ucHiddenSsidMode != ENUM_HIDDEN_SSID_NONE;
	COPY_SSID(tag->aucSSID, tag->ucSSIDLen, cmd->aucSSID, cmd->ucSSIDLen);

	return tag->u2Length;
}

uint32_t nicUniCmdBssInfoTagP2p(struct ADAPTER *ad,
	uint8_t *buf, struct CMD_SET_BSS_INFO *cmd)
{
	struct UNI_CMD_BSSINFO_P2P *tag = (struct UNI_CMD_BSSINFO_P2P *)buf;

	tag->u2Tag = UNI_CMD_BSSINFO_TAG_P2P;
	tag->u2Length = sizeof(*tag);
	tag->u4PrivateData = cmd->u4PrivateData;

	return tag->u2Length;
}

#if (CFG_SUPPORT_802_11AX == 1)
uint32_t nicUniCmdBssInfoTagHe(struct ADAPTER *ad,
	uint8_t *buf, struct CMD_SET_BSS_INFO *cmd)
{
	struct UNI_CMD_BSSINFO_HE *tag = (struct UNI_CMD_BSSINFO_HE *)buf;
	struct BSS_INFO *bss = GET_BSS_INFO_BY_INDEX(ad, cmd->ucBssIndex);

	tag->u2Tag = UNI_CMD_BSSINFO_TAG_HE;
	tag->u2Length = sizeof(*tag);
	tag->u2TxopDurationRtsThreshold =
		((bss->ucHeOpParams[0] &
			HE_OP_PARAM0_TXOP_DUR_RTS_THRESHOLD_MASK) >>
			HE_OP_PARAM0_TXOP_DUR_RTS_THRESHOLD_SHFT) |
		(((bss->ucHeOpParams[1] &
			HE_OP_PARAM1_TXOP_DUR_RTS_THRESHOLD_MASK) >>
			HE_OP_PARAM1_TXOP_DUR_RTS_THRESHOLD_SHFT) << 4);
	tag->au2MaxNssMcs[0] = CPU_TO_LE16(bss->u2HeBasicMcsSet);
	tag->au2MaxNssMcs[1] = CPU_TO_LE16(bss->u2HeBasicMcsSet);
	tag->au2MaxNssMcs[2] = CPU_TO_LE16(bss->u2HeBasicMcsSet);
	tag->ucDefaultPEDuration = ((bss->ucHeOpParams[0] &
		HE_OP_PARAM0_DEFAULT_PE_DUR_MASK) >>
		HE_OP_PARAM0_DEFAULT_PE_DUR_SHFT);
	tag->fgErSuDisable = (bss->ucHeOpParams[2] &
		(1 << HE_OP_PARAM2_ER_SU_DISABLE_SHFT)) ? TRUE : FALSE;

	return tag->u2Length;
}

uint32_t nicUniCmdBssInfoTagBssColor(struct ADAPTER *ad,
	uint8_t *buf, struct CMD_SET_BSS_INFO *cmd)
{
	struct UNI_CMD_BSSINFO_BSS_COLOR *tag =
		(struct UNI_CMD_BSSINFO_BSS_COLOR *)buf;
	struct BSS_INFO *bss = GET_BSS_INFO_BY_INDEX(ad, cmd->ucBssIndex);

	tag->u2Tag = UNI_CMD_BSSINFO_TAG_BSS_COLOR;
	tag->u2Length = sizeof(*tag);
	tag->fgEnable =	(bss->ucBssColorInfo & HE_OP_BSSCOLOR_BSS_COLOR_DISABLE)
			? FALSE : TRUE;
	tag->ucBssColor = ((bss->ucBssColorInfo & HE_OP_BSSCOLOR_BSS_COLOR_MASK)
			>> HE_OP_BSSCOLOR_BSS_COLOR_SHFT);
	return tag->u2Length;
}

#endif

uint32_t nicUniCmdBssInfoTagMBSSID(struct ADAPTER *ad,
	uint8_t *buf, struct CMD_SET_BSS_INFO *cmd)
{
	struct UNI_CMD_BSSINFO_11V_MBSSID *tag =
		(struct UNI_CMD_BSSINFO_11V_MBSSID *)buf;

	tag->u2Tag = UNI_CMD_BSSINFO_TAG_11V_MBSSID;
	tag->u2Length = sizeof(*tag);
	tag->ucMaxBSSIDIndicator = cmd->ucMaxBSSIDIndicator;
	tag->ucMBSSIDIndex = cmd->ucMBSSIDIndex;

	return tag->u2Length;
}

uint32_t nicUniCmdBssInfoTagWapi(struct ADAPTER *ad,
	uint8_t *buf, struct CMD_SET_BSS_INFO *cmd)
{
	struct UNI_CMD_BSSINFO_WAPI *tag = (struct UNI_CMD_BSSINFO_WAPI *)buf;

	tag->u2Tag = UNI_CMD_BSSINFO_TAG_WAPI;
	tag->u2Length = sizeof(*tag);
	tag->fgWapiMode = cmd->ucWapiMode;

	return tag->u2Length;
}

uint32_t nicUniCmdBssInfoTagMld(struct ADAPTER *ad,
	uint8_t *buf, struct CMD_SET_BSS_INFO *cmd)
{
	struct UNI_CMD_BSSINFO_MLD *tag = (struct UNI_CMD_BSSINFO_MLD *)buf;
	struct BSS_INFO *bss = GET_BSS_INFO_BY_INDEX(ad, cmd->ucBssIndex);

	tag->u2Tag = UNI_CMD_BSSINFO_TAG_MLD;
	tag->u2Length = sizeof(*tag);
	tag->ucGroupMldId = MLD_GROUP_NONE;
	tag->ucOwnMldId = bss->ucBssIndex;
	COPY_MAC_ADDR(tag->aucOwnMldAddr, cmd->aucBSSID);
	tag->ucOmRemapIdx = OM_REMAP_IDX_NONE;

	DBGLOG(NIC, INFO,
		"[%d] GroupMldId: %d, OwnMldId: %d, OmRemapIdx: %d, OwnMldAddr: "
		MACSTR "\n",
		cmd->ucBssIndex,
		tag->ucGroupMldId,
		tag->ucOwnMldId,
		tag->ucOmRemapIdx,
		MAC2STR(tag->aucOwnMldAddr));

	return tag->u2Length;
}

struct UNI_CMD_BSSINFO_TAG_HANDLE arSetBssInfoTable[] = {
	{sizeof(struct UNI_CMD_BSSINFO_BASIC), nicUniCmdBssInfoTagBasic},
	{sizeof(struct UNI_CMD_BSSINFO_RLM) +
	 sizeof(struct UNI_CMD_BSSINFO_PROTECT)	+
	 sizeof(struct UNI_CMD_BSSINFO_IFS_TIME), nicUniCmdBssInfoTagRlm},
	{sizeof(struct UNI_CMD_BSSINFO_RATE), nicUniCmdBssInfoTagRate},
	{sizeof(struct UNI_CMD_BSSINFO_SEC), nicUniCmdBssInfoTagSec},
	{sizeof(struct UNI_CMD_BSSINFO_QBSS), nicUniCmdBssInfoTagQbss},
	{sizeof(struct UNI_CMD_BSSINFO_SAP), nicUniCmdBssInfoTagSap},
	{sizeof(struct UNI_CMD_BSSINFO_P2P), nicUniCmdBssInfoTagP2p},
#if (CFG_SUPPORT_802_11AX == 1)
	{sizeof(struct UNI_CMD_BSSINFO_HE), nicUniCmdBssInfoTagHe},
	{sizeof(struct UNI_CMD_BSSINFO_BSS_COLOR), nicUniCmdBssInfoTagBssColor},
#endif
	{sizeof(struct UNI_CMD_BSSINFO_11V_MBSSID), nicUniCmdBssInfoTagMBSSID},
	{sizeof(struct UNI_CMD_BSSINFO_WAPI), nicUniCmdBssInfoTagWapi},
	{sizeof(struct UNI_CMD_BSSINFO_MLD), nicUniCmdBssInfoTagMld}
};

uint32_t nicUniCmdSetBssInfo(struct ADAPTER *ad,
		struct WIFI_UNI_SETQUERY_INFO *info)
{
	struct CMD_SET_BSS_INFO *cmd;
	struct UNI_CMD_BSSINFO *uni_cmd;
	struct WIFI_UNI_CMD_ENTRY *entry;
	uint8_t *pos;
	uint32_t max_cmd_len = 0;
	int i;

	if (info->ucCID != CMD_ID_SET_BSS_INFO ||
	    info->u4SetQueryInfoLen != sizeof(*cmd))
		return WLAN_STATUS_NOT_ACCEPTED;

	cmd = (struct CMD_SET_BSS_INFO *) info->pucInfoBuffer;
	max_cmd_len += sizeof(struct UNI_CMD_BSSINFO);
	for (i = 0; i < ARRAY_SIZE(arSetBssInfoTable); i++)
		max_cmd_len += arSetBssInfoTable[i].u4Size;
	entry = nicUniCmdAllocEntry(ad, UNI_CMD_ID_BSSINFO,
			max_cmd_len, NULL, NULL);

	if (!entry)
		return WLAN_STATUS_RESOURCES;

	pos = entry->pucInfoBuffer;
	uni_cmd = (struct UNI_CMD_BSSINFO *) pos;
	uni_cmd->ucBssInfoIdx = cmd->ucBssIndex;
	pos += sizeof(*uni_cmd);
	for (i = 0; i < ARRAY_SIZE(arSetBssInfoTable); i++)
		pos += arSetBssInfoTable[i].pfHandler(ad, pos, cmd);
	entry->u4SetQueryInfoLen = pos - entry->pucInfoBuffer;

	ASSERT(entry->u4SetQueryInfoLen == max_cmd_len);

	LINK_INSERT_TAIL(&info->rUniCmdList, &entry->rLinkEntry);

	return WLAN_STATUS_SUCCESS;
}

uint32_t nicUniCmdSetBssRlm(struct ADAPTER *ad,
		struct WIFI_UNI_SETQUERY_INFO *info)
{
	struct CMD_SET_BSS_RLM_PARAM *cmd;
	struct UNI_CMD_BSSINFO *uni_cmd;
	struct WIFI_UNI_CMD_ENTRY *entry;
	uint32_t max_cmd_len = sizeof(struct UNI_CMD_BSSINFO) +
			       sizeof(struct UNI_CMD_BSSINFO_RLM) +
			       sizeof(struct UNI_CMD_BSSINFO_PROTECT) +
			       sizeof(struct UNI_CMD_BSSINFO_IFS_TIME);
	uint8_t *pos;

	if (info->ucCID != CMD_ID_SET_BSS_RLM_PARAM ||
	    info->u4SetQueryInfoLen != sizeof(*cmd))
		return WLAN_STATUS_NOT_ACCEPTED;

	cmd = (struct CMD_SET_BSS_RLM_PARAM *) info->pucInfoBuffer;
	entry = nicUniCmdAllocEntry(ad, UNI_CMD_ID_BSSINFO, max_cmd_len,
			NULL, NULL);
	if (!entry)
		return WLAN_STATUS_RESOURCES;

	uni_cmd = (struct UNI_CMD_BSSINFO *) entry->pucInfoBuffer;
	uni_cmd->ucBssInfoIdx = cmd->ucBssIndex;
	pos = uni_cmd->aucTlvBuffer;

	nicUniCmdSetBssRlmImpl(ad, uni_cmd->aucTlvBuffer, cmd);

	LINK_INSERT_TAIL(&info->rUniCmdList, &entry->rLinkEntry);

	return WLAN_STATUS_SUCCESS;
}

uint32_t nicUniCmdPowerSaveMode(struct ADAPTER *ad,
		struct WIFI_UNI_SETQUERY_INFO *info)
{
	struct CMD_PS_PROFILE *cmd;
	struct UNI_CMD_BSSINFO *uni_cmd;
	struct UNI_CMD_BSSINFO_POWER_SAVE *tag;
	struct WIFI_UNI_CMD_ENTRY *entry;
	uint32_t max_cmd_len = sizeof(struct UNI_CMD_BSSINFO) +
			       sizeof(struct UNI_CMD_BSSINFO_POWER_SAVE);

	if (info->ucCID != CMD_ID_POWER_SAVE_MODE ||
	    info->u4SetQueryInfoLen != sizeof(*cmd))
		return WLAN_STATUS_NOT_ACCEPTED;

	cmd = (struct CMD_PS_PROFILE *) info->pucInfoBuffer;
	entry = nicUniCmdAllocEntry(ad, UNI_CMD_ID_BSSINFO, max_cmd_len,
			info->fgIsOid ? nicUniCmdEventSetCommon : NULL,
			info->fgIsOid ? nicUniCmdTimeoutCommon : NULL);
	if (!entry)
		return WLAN_STATUS_RESOURCES;

	uni_cmd = (struct UNI_CMD_BSSINFO *) entry->pucInfoBuffer;
	uni_cmd->ucBssInfoIdx = cmd->ucBssIndex;
	tag = (struct UNI_CMD_BSSINFO_POWER_SAVE *) uni_cmd->aucTlvBuffer;
	tag->u2Tag = UNI_CMD_BSSINFO_TAG_POWER_SAVE;
	tag->u2Length = sizeof(*tag);
	tag->ucPsProfile = cmd->ucPsProfile;

	LINK_INSERT_TAIL(&info->rUniCmdList, &entry->rLinkEntry);

	return WLAN_STATUS_SUCCESS;
}

uint32_t nicUniCmdTwtArgtUpdate(struct ADAPTER *ad,
		struct WIFI_UNI_SETQUERY_INFO *info)
{
#if (CFG_SUPPORT_TWT == 1)
	struct _EXT_CMD_TWT_ARGT_UPDATE_T *cmd;
	struct UNI_CMD_TWT *uni_cmd;
	struct UNI_CMD_TWT_ARGT_UPDATE *tag;
	struct WIFI_UNI_CMD_ENTRY *entry;
	uint32_t max_cmd_len = sizeof(struct UNI_CMD_TWT) +
			       sizeof(struct UNI_CMD_TWT_ARGT_UPDATE);

	if (info->ucCID != CMD_ID_LAYER_0_EXT_MAGIC_NUM ||
	    info->ucExtCID != EXT_CMD_ID_TWT_AGRT_UPDATE ||
	    info->u4SetQueryInfoLen != sizeof(*cmd))
		return WLAN_STATUS_NOT_ACCEPTED;

	cmd = (struct _EXT_CMD_TWT_ARGT_UPDATE_T *) info->pucInfoBuffer;
	entry = nicUniCmdAllocEntry(ad, UNI_CMD_ID_TWT, max_cmd_len,
			NULL, NULL);
	if (!entry)
		return WLAN_STATUS_RESOURCES;

	uni_cmd = (struct UNI_CMD_TWT *) entry->pucInfoBuffer;
	uni_cmd->ucBssInfoIdx = cmd->ucBssIndex;
	tag = (struct UNI_CMD_TWT_ARGT_UPDATE *) uni_cmd->aucTlvBuffer;
	tag->u2Tag = UNI_CMD_TWT_TAG_AGRT_UPDATE;
	tag->u2Length = sizeof(*tag);
	tag->ucAgrtTblIdx = cmd->ucAgrtTblIdx;
	tag->ucAgrtCtrlFlag = cmd->ucAgrtCtrlFlag;
	tag->ucBssIndex = cmd->ucBssIndex;
	tag->ucOwnMacId = cmd->ucOwnMacId;
	tag->ucFlowId = cmd->ucFlowId;
	tag->u2PeerIdGrpId = cmd->u2PeerIdGrpId;
	tag->ucAgrtSpDuration = cmd->ucAgrtSpDuration;
	tag->u4AgrtSpStartTsf_low = cmd->u4AgrtSpStartTsfLow;
	tag->u4AgrtSpStartTsf_high = cmd->u4AgrtSpStartTsfHigh;
	tag->u2AgrtSpWakeIntvlMantissa = cmd->u2AgrtSpWakeIntvlMantissa;
	tag->ucAgrtSpWakeIntvlExponent = cmd->ucAgrtSpWakeIntvlExponent;
	tag->fgIsRoleAp = cmd->ucIsRoleAp;
	tag->ucAgrtParaBitmap = cmd->ucAgrtParaBitmap;
	tag->ucReserved_a = cmd->ucReserved_a;
	tag->u2Reserved_b = cmd->u2Reserved_b;
	tag->ucGrpMemberCnt = cmd->ucGrpMemberCnt;
	tag->ucReserved_c = cmd->ucReserved_c;
	tag->u2Reserved_d = cmd->u2Reserved_d;
	kalMemCopy(tag->au2StaList, cmd->au2StaList, sizeof(tag->au2StaList));

	LINK_INSERT_TAIL(&info->rUniCmdList, &entry->rLinkEntry);

	return WLAN_STATUS_SUCCESS;
#else
	return WLAN_STATUS_NOT_SUPPORTED;
#endif
}

uint32_t nicUniCmdStaRecUpdateExt(struct ADAPTER *ad,
		struct WIFI_UNI_SETQUERY_INFO *info)
{
	struct CMD_STAREC_UPDATE *cmd;
	struct CMD_STAREC_BF *bf;
	struct UNI_CMD_STAREC *uni_cmd;
	struct UNI_CMD_STAREC_BF *tag;
	struct WIFI_UNI_CMD_ENTRY *entry;
	uint32_t max_cmd_len = sizeof(struct UNI_CMD_STAREC) +
			       sizeof(struct UNI_CMD_STAREC_BF);

	if (info->ucCID != CMD_ID_LAYER_0_EXT_MAGIC_NUM ||
	    info->ucExtCID != EXT_CMD_ID_STAREC_UPDATE)
		return WLAN_STATUS_NOT_ACCEPTED;

	cmd = (struct CMD_STAREC_UPDATE *) info->pucInfoBuffer;
	bf = (struct CMD_STAREC_BF *) cmd->aucBuffer;

	if (bf->u2Tag != STA_REC_BF)
		return WLAN_STATUS_NOT_ACCEPTED;

	entry = nicUniCmdAllocEntry(ad, UNI_CMD_ID_STAREC_INFO, max_cmd_len,
			nicUniCmdEventSetCommon, nicUniCmdTimeoutCommon);
	if (!entry)
		return WLAN_STATUS_RESOURCES;

	uni_cmd = (struct UNI_CMD_STAREC *) entry->pucInfoBuffer;
	uni_cmd->ucBssInfoIdx = cmd->ucBssIndex;
	WCID_SET_H_L(uni_cmd->ucWlanIdxHnVer, uni_cmd->ucWlanIdxL,
		cmd->ucWlanIdx);

	tag = (struct UNI_CMD_STAREC_BF *) uni_cmd->aucTlvBuffer;
	tag->u2Tag = UNI_CMD_STAREC_TAG_BF;
	tag->u2Length = sizeof(*tag);
	tag->rTxBfPfmuInfo.u2PfmuId = bf->rTxBfPfmuInfo.u2PfmuId;
	tag->rTxBfPfmuInfo.fgSU_MU = bf->rTxBfPfmuInfo.fgSU_MU;
	tag->rTxBfPfmuInfo.u1TxBfCap = bf->rTxBfPfmuInfo.u1TxBfCap;
	tag->rTxBfPfmuInfo.ucSoundingPhy = bf->rTxBfPfmuInfo.ucSoundingPhy;
	tag->rTxBfPfmuInfo.ucNdpaRate = bf->rTxBfPfmuInfo.ucNdpaRate;
	tag->rTxBfPfmuInfo.ucNdpRate = bf->rTxBfPfmuInfo.ucNdpRate;
	tag->rTxBfPfmuInfo.ucReptPollRate = bf->rTxBfPfmuInfo.ucReptPollRate;
	tag->rTxBfPfmuInfo.ucTxMode = bf->rTxBfPfmuInfo.ucTxMode;
	tag->rTxBfPfmuInfo.ucNc = bf->rTxBfPfmuInfo.ucNc;
	tag->rTxBfPfmuInfo.ucNr = bf->rTxBfPfmuInfo.ucNr;
	tag->rTxBfPfmuInfo.ucCBW = bf->rTxBfPfmuInfo.ucCBW;
	tag->rTxBfPfmuInfo.ucTotMemRequire = bf->rTxBfPfmuInfo.ucTotMemRequire;
	tag->rTxBfPfmuInfo.ucMemRequire20M = bf->rTxBfPfmuInfo.ucMemRequire20M;
	tag->rTxBfPfmuInfo.ucMemRow0 = bf->rTxBfPfmuInfo.ucMemRow0;
	tag->rTxBfPfmuInfo.ucMemCol0 = bf->rTxBfPfmuInfo.ucMemCol0;
	tag->rTxBfPfmuInfo.ucMemRow1 = bf->rTxBfPfmuInfo.ucMemRow1;
	tag->rTxBfPfmuInfo.ucMemCol1 = bf->rTxBfPfmuInfo.ucMemCol1;
	tag->rTxBfPfmuInfo.ucMemRow2 = bf->rTxBfPfmuInfo.ucMemRow2;
	tag->rTxBfPfmuInfo.ucMemCol2 = bf->rTxBfPfmuInfo.ucMemCol2;
	tag->rTxBfPfmuInfo.ucMemRow3 = bf->rTxBfPfmuInfo.ucMemRow3;
	tag->rTxBfPfmuInfo.ucMemCol3 = bf->rTxBfPfmuInfo.ucMemCol3;
	tag->rTxBfPfmuInfo.u2SmartAnt = bf->rTxBfPfmuInfo.u2SmartAnt;
	tag->rTxBfPfmuInfo.ucSEIdx = bf->rTxBfPfmuInfo.ucSEIdx;
	tag->rTxBfPfmuInfo.ucAutoSoundingCtrl =
		bf->rTxBfPfmuInfo.ucAutoSoundingCtrl;
	tag->rTxBfPfmuInfo.uciBfTimeOut = bf->rTxBfPfmuInfo.uciBfTimeOut;
	tag->rTxBfPfmuInfo.uciBfDBW = bf->rTxBfPfmuInfo.uciBfDBW;
	tag->rTxBfPfmuInfo.uciBfNcol = bf->rTxBfPfmuInfo.uciBfNcol;
	tag->rTxBfPfmuInfo.uciBfNrow = bf->rTxBfPfmuInfo.uciBfNrow;
	tag->rTxBfPfmuInfo.nr_bw160 = bf->rTxBfPfmuInfo.u1NrBw160;
	tag->rTxBfPfmuInfo.nc_bw160 = bf->rTxBfPfmuInfo.u1NcBw160;
	tag->rTxBfPfmuInfo.ru_start_idx = bf->rTxBfPfmuInfo.u1RuStartIdx;
	tag->rTxBfPfmuInfo.ru_end_idx = bf->rTxBfPfmuInfo.u1RuEndIdx;
	tag->rTxBfPfmuInfo.trigger_su = bf->rTxBfPfmuInfo.fgTriggerSu;
	tag->rTxBfPfmuInfo.trigger_mu = bf->rTxBfPfmuInfo.fgTriggerMu;
	tag->rTxBfPfmuInfo.ng16_su = bf->rTxBfPfmuInfo.fgNg16Su;
	tag->rTxBfPfmuInfo.ng16_mu = bf->rTxBfPfmuInfo.fgNg16Mu;
	tag->rTxBfPfmuInfo.codebook42_su = bf->rTxBfPfmuInfo.fgCodebook42Su;
	tag->rTxBfPfmuInfo.codebook75_mu = bf->rTxBfPfmuInfo.fgCodebook75Mu;
	tag->rTxBfPfmuInfo.he_ltf = bf->rTxBfPfmuInfo.u1HeLtf;

	LINK_INSERT_TAIL(&info->rUniCmdList, &entry->rLinkEntry);

	return WLAN_STATUS_SUCCESS;
}

uint32_t nicUniCmdBFAction(struct ADAPTER *ad,
		struct WIFI_UNI_SETQUERY_INFO *info)
{
	union CMD_TXBF_ACTION *cmd;
	struct UNI_CMD_BF *uni_cmd;
	struct UNI_CMD_BF_SND *tag;
	struct WIFI_UNI_CMD_ENTRY *entry;
	uint32_t max_cmd_len = sizeof(struct UNI_CMD_BF) +
			       sizeof(struct UNI_CMD_BF_SND);
	uint32_t bf_id;
	uint8_t i;

	if (info->ucCID != CMD_ID_LAYER_0_EXT_MAGIC_NUM ||
	    info->ucExtCID != EXT_CMD_ID_BF_ACTION)
		return WLAN_STATUS_NOT_ACCEPTED;

	cmd = (union CMD_TXBF_ACTION *) info->pucInfoBuffer;

	bf_id =	cmd->rTxBfSoundingStart.ucCmdCategoryID;
	if (bf_id != BF_SOUNDING_ON)
		return WLAN_STATUS_NOT_ACCEPTED;

	entry = nicUniCmdAllocEntry(ad, UNI_CMD_ID_BF, max_cmd_len,
			nicUniCmdEventSetCommon, nicUniCmdTimeoutCommon);
	if (!entry)
		return WLAN_STATUS_RESOURCES;

	uni_cmd = (struct UNI_CMD_BF *) entry->pucInfoBuffer;
	tag = (struct UNI_CMD_BF_SND *) uni_cmd->aucTlvBuffer;
	tag->u2Tag = UNI_CMD_BF_TAG_SOUNDING_ON;
	tag->u2Length = sizeof(*tag);
	tag->u1SuMuSndMode = cmd->rTxBfSoundingStart.ucSuMuSndMode;
	tag->u1StaNum = cmd->rTxBfSoundingStart.ucStaNum;
	for (i = 0; i < 4; i++)
		tag->u2WlanId[i] = cmd->rTxBfSoundingStart.ucWlanId[i];
	tag->u4SndIntv = cmd->rTxBfSoundingStart.u4SoundingInterval;

	LINK_INSERT_TAIL(&info->rUniCmdList, &entry->rLinkEntry);

	return WLAN_STATUS_SUCCESS;
}

uint32_t nicUniCmdSerAction(struct ADAPTER *ad,
		struct WIFI_UNI_SETQUERY_INFO *info)
{
	struct EXT_CMD_SER_T *cmd;
	struct UNI_CMD_SER *uni_cmd;
	struct WIFI_UNI_CMD_ENTRY *entry;
	uint32_t max_cmd_len = sizeof(struct UNI_CMD_SER);

	if (info->ucCID != CMD_ID_LAYER_0_EXT_MAGIC_NUM ||
	    info->ucExtCID != EXT_CMD_ID_SER)
		return WLAN_STATUS_NOT_ACCEPTED;

	cmd = (struct EXT_CMD_SER_T *) info->pucInfoBuffer;
	switch (cmd->ucAction) {
	case SER_ACTION_QUERY:
		max_cmd_len += sizeof(struct UNI_CMD_SER_QUERY);
		break;
	case SER_ACTION_SET:
		max_cmd_len += sizeof(struct UNI_CMD_SER_ENABLE);
		break;
	case SER_ACTION_SET_ENABLE_MASK:
		max_cmd_len += sizeof(struct UNI_CMD_SER_SET);
		break;
	case SER_ACTION_RECOVER:
		max_cmd_len += sizeof(struct UNI_CMD_SER_TRIGGER);
		break;
	default:
		DBGLOG(NIC, ERROR, "unknown action %d\n", cmd->ucAction);
		return WLAN_STATUS_NOT_ACCEPTED;
	}

	entry = nicUniCmdAllocEntry(ad, UNI_CMD_ID_SER, max_cmd_len,
			NULL, nicUniCmdTimeoutCommon);
	if (!entry)
		return WLAN_STATUS_RESOURCES;

	uni_cmd = (struct UNI_CMD_SER *) entry->pucInfoBuffer;
	switch (cmd->ucAction) {
	case SER_ACTION_QUERY: {
		struct UNI_CMD_SER_QUERY *tag =
			(struct UNI_CMD_SER_QUERY *)uni_cmd->aucTlvBuffer;

		tag->u2Tag = UNI_CMD_SER_TAG_QUERY;
		tag->u2Length = sizeof(*tag);
	}
		break;
	case SER_ACTION_SET: {
		struct UNI_CMD_SER_ENABLE *tag =
			(struct UNI_CMD_SER_ENABLE *)uni_cmd->aucTlvBuffer;

		tag->u2Tag = UNI_CMD_SER_TAG_ENABLE;
		tag->u2Length = sizeof(*tag);
		tag->fgEnable = cmd->ucSerSet;
	}
		break;
	case SER_ACTION_SET_ENABLE_MASK: {
		struct UNI_CMD_SER_SET *tag =
			(struct UNI_CMD_SER_SET *)uni_cmd->aucTlvBuffer;

		tag->u2Tag = UNI_CMD_SER_TAG_SET;
		tag->u2Length = sizeof(*tag);
		tag->u4EnableMask = cmd->ucSerSet;
	}
		break;
	case SER_ACTION_RECOVER: {
		struct UNI_CMD_SER_TRIGGER *tag =
			(struct UNI_CMD_SER_TRIGGER *)uni_cmd->aucTlvBuffer;

		tag->u2Tag = UNI_CMD_SER_TAG_TRIGGER;
		tag->u2Length = sizeof(*tag);
		tag->ucTriggerMethod = cmd->ucSerSet;
		tag->ucDbdcIdx = cmd->ucDbdcIdx;
	}
		break;
	}

	LINK_INSERT_TAIL(&info->rUniCmdList, &entry->rLinkEntry);

	return WLAN_STATUS_SUCCESS;
}

uint32_t nicUniCmdGetTsf(struct ADAPTER *ad,
		struct WIFI_UNI_SETQUERY_INFO *info)
{
	struct _EXT_CMD_GET_MAC_INFO_T *cmd;
	struct UNI_CMD_GET_MAC_INFO *uni_cmd;
	struct UNI_CMD_MAC_INFO_TSF *tag;
	struct WIFI_UNI_CMD_ENTRY *entry;
	uint32_t max_cmd_len = sizeof(struct UNI_CMD_GET_MAC_INFO) +
			       sizeof(struct UNI_CMD_MAC_INFO_TSF);

	if (info->ucCID != CMD_ID_LAYER_0_EXT_MAGIC_NUM ||
	    info->ucExtCID != EXT_CMD_ID_GET_MAC_INFO)
		return WLAN_STATUS_NOT_ACCEPTED;

	cmd = (struct _EXT_CMD_GET_MAC_INFO_T *) info->pucInfoBuffer;
	entry = nicUniCmdAllocEntry(ad, UNI_CMD_ID_GET_MAC_INFO, max_cmd_len,
			nicUniCmdEventGetTsfDone, NULL);
	if (!entry)
		return WLAN_STATUS_RESOURCES;

	uni_cmd = (struct UNI_CMD_GET_MAC_INFO *) entry->pucInfoBuffer;
	tag = (struct UNI_CMD_MAC_INFO_TSF *) uni_cmd->aucTlvBuffer;
	tag->u2Tag = UNI_CMD_MAC_INFO_TAG_TSF;
	tag->u2Length = sizeof(*tag);
	tag->ucDbdcIdx = ENUM_BAND_AUTO;
	tag->ucHwBssidIndex = cmd->rExtraArgument.rTsfArg.ucHwBssidIndex;

	LINK_INSERT_TAIL(&info->rUniCmdList, &entry->rLinkEntry);

	return WLAN_STATUS_SUCCESS;
}

uint32_t nicUniCmdStaRecConnType(struct ADAPTER *ad, uint32_t legacy_sta_type)
{
	if (legacy_sta_type == STA_TYPE_LEGACY_AP)
		return CONNECTION_INFRA_AP;
	else if (legacy_sta_type == STA_TYPE_LEGACY_CLIENT)
		return CONNECTION_INFRA_STA;
	else if (legacy_sta_type == STA_TYPE_P2P_GO)
		return CONNECTION_P2P_GO;
	else if (legacy_sta_type == STA_TYPE_P2P_GC)
		return CONNECTION_P2P_GC;

	DBGLOG(NIC, ERROR, "wrong sta_type=%d\n", legacy_sta_type);
	return CONNECTION_INFRA_STA;
}

uint32_t nicUniCmdStaRecTagBasic(struct ADAPTER *ad,
	uint8_t *buf, struct CMD_UPDATE_STA_RECORD *cmd)
{
	struct UNI_CMD_STAREC_BASIC *tag = (struct UNI_CMD_STAREC_BASIC *)buf;

	tag->u2Tag = UNI_CMD_STAREC_TAG_BASIC;
	tag->u2Length = sizeof(*tag);
	tag->u4ConnectionType = nicUniCmdStaRecConnType(ad, cmd->ucStaType);
	tag->ucConnectionState = STATE_CONNECTED;
	tag->ucIsQBSS = cmd->ucIsQoS;
	tag->u2AID = cmd->u2AssocId;
	COPY_MAC_ADDR(tag->aucPeerMacAddr, cmd->aucMacAddr);
	tag->u2ExtraInfo = STAREC_COMMON_EXTRAINFO_V2 |
			   STAREC_COMMON_EXTRAINFO_NEWSTAREC;

	return tag->u2Length;
}

uint32_t nicUniCmdStaRecTagState(struct ADAPTER *ad,
	uint8_t *buf, struct CMD_UPDATE_STA_RECORD *cmd)
{
	struct UNI_CMD_STAREC_STATE_INFO *tag =
		(struct UNI_CMD_STAREC_STATE_INFO *)buf;

	tag->u2Tag = UNI_CMD_STAREC_TAG_STATE_CHANGED;
	tag->u2Length = sizeof(*tag);
	tag->ucStaState = cmd->ucStaState;
	tag->u4Flags = cmd->u4Flags;
	tag->ucVhtOpMode = cmd->ucVhtOpMode;
	tag->ucActionType = STA_REC_CMD_ACTION_STA;

	return tag->u2Length;
}

uint32_t nicUniCmdStaRecTagPhyInfo(struct ADAPTER *ad,
	uint8_t *buf, struct CMD_UPDATE_STA_RECORD *cmd)
{
	struct UNI_CMD_STAREC_PHY_INFO *tag =
		(struct UNI_CMD_STAREC_PHY_INFO *)buf;

	tag->u2Tag = UNI_CMD_STAREC_TAG_PHY_INFO;
	tag->u2Length = sizeof(*tag);
	tag->u2BSSBasicRateSet = cmd->u2BSSBasicRateSet;
	tag->ucDesiredPhyTypeSet = cmd->ucDesiredPhyTypeSet;
	tag->ucAmpduParam = cmd->ucAmpduParam;
	tag->ucRtsPolicy = cmd->ucRtsPolicy;
	tag->ucRCPI = cmd->ucRCPI;

	return tag->u2Length;
}

uint32_t nicUniCmdStaRecTagHtInfo(struct ADAPTER *ad,
	uint8_t *buf, struct CMD_UPDATE_STA_RECORD *cmd)
{
	struct UNI_CMD_STAREC_HT_INFO *tag =
		(struct UNI_CMD_STAREC_HT_INFO *)buf;

	if (cmd->u2HtCapInfo == 0 && cmd->u2HtExtendedCap == 0)
		return 0;

	tag->u2Tag = UNI_CMD_STAREC_TAG_HT_BASIC;
	tag->u2Length = sizeof(*tag);
	tag->u2HtCap = cmd->u2HtCapInfo;
	tag->u2HtExtendedCap = cmd->u2HtExtendedCap;

	return tag->u2Length;
}

uint32_t nicUniCmdStaRecTagVhtInfo(struct ADAPTER *ad,
	uint8_t *buf, struct CMD_UPDATE_STA_RECORD *cmd)
{
	struct UNI_CMD_STAREC_VHT_INFO *tag =
		(struct UNI_CMD_STAREC_VHT_INFO *)buf;

	if (cmd->u4VhtCapInfo == 0)
		return 0;

	tag->u2Tag = UNI_CMD_STAREC_TAG_VHT_BASIC;
	tag->u2Length = sizeof(*tag);
	tag->u4VhtCap = cmd->u4VhtCapInfo;
	tag->u2VhtRxMcsMap = cmd->u2VhtRxMcsMap;
	tag->u2VhtTxMcsMap = cmd->u2VhtTxMcsMap;

	return tag->u2Length;
}

#if (CFG_SUPPORT_802_11AX == 1)
uint32_t nicUniCmdStaRecTagHeBasic(struct ADAPTER *ad,
	uint8_t *buf, struct CMD_UPDATE_STA_RECORD *cmd)
{
	struct UNI_CMD_STAREC_HE_BASIC *tag =
		(struct UNI_CMD_STAREC_HE_BASIC *)buf;
	static uint8_t zero[HE_PHY_CAP_BYTE_NUM];

	if (!kalMemCmp(cmd->ucHeMacCapInfo, zero, HE_MAC_CAP_BYTE_NUM) &&
	    !kalMemCmp(cmd->ucHePhyCapInfo, zero, HE_PHY_CAP_BYTE_NUM))
		return 0;

	tag->u2Tag = UNI_CMD_STAREC_TAG_HE_BASIC;
	tag->u2Length = sizeof(*tag);
	kalMemCopy(tag->aucHeMacCapInfo, cmd->ucHeMacCapInfo,
			sizeof(tag->aucHeMacCapInfo));
	kalMemCopy(tag->aucHePhyCapInfo, cmd->ucHePhyCapInfo,
			sizeof(tag->aucHePhyCapInfo));
	tag->ucPktExt = 2; /* mobile */
	tag->au2RxMaxNssMcs[0] = cmd->u2HeRxMcsMapBW80;
	tag->au2RxMaxNssMcs[1] = cmd->u2HeRxMcsMapBW160;
	tag->au2RxMaxNssMcs[2] = cmd->u2HeRxMcsMapBW80P80;

	return tag->u2Length;
}

uint32_t nicUniCmdStaRecTagHe6gCap(struct ADAPTER *ad,
	uint8_t *buf, struct CMD_UPDATE_STA_RECORD *cmd)
{
#if 0
	struct UNI_CMD_STAREC_HE_6G_CAP *tag =
		(struct UNI_CMD_STAREC_HE_6G_CAP *)buf;

	/* TODO: uni cmd */

	tag->u2Tag = UNI_CMD_STAREC_TAG_HE_6G_CAP;
	tag->u2Length = sizeof(*tag);
	tag->u2He6gBandCapInfo = 0;
	return tag->u2Length;
#else
	return 0;
#endif
}
#endif

uint32_t nicUniCmdStaRecTagRA(struct ADAPTER *ad,
	uint8_t *buf, struct CMD_UPDATE_STA_RECORD *cmd)
{
	struct UNI_CMD_STAREC_RA_INFO *tag =
		(struct UNI_CMD_STAREC_RA_INFO *)buf;

	tag->u2Tag = UNI_CMD_STAREC_TAG_RA;
	tag->u2Length = sizeof(*tag);
	tag->u2DesiredNonHTRateSet = cmd->u2DesiredNonHTRateSet;
	kalMemCopy(tag->aucRxMcsBitmask, cmd->aucRxMcsBitmask, 10);

	return tag->u2Length;
}

uint32_t nicUniCmdStaRecTagBAOffload(struct ADAPTER *ad,
	uint8_t *buf, struct CMD_UPDATE_STA_RECORD *cmd)
{
	struct UNI_CMD_STAREC_BA_OFFLOAD_INFO *tag =
		(struct UNI_CMD_STAREC_BA_OFFLOAD_INFO *)buf;

	tag->u2Tag = UNI_CMD_STAREC_TAG_BA_OFFLOAD;
	tag->u2Length = sizeof(*tag);
	tag->ucTxAmpdu = cmd->ucTxAmpdu;
	tag->ucRxAmpdu = cmd->ucRxAmpdu;
	tag->ucTxAmsduInAmpdu = cmd->ucTxAmsduInAmpdu;
	tag->ucRxAmsduInAmpdu = cmd->ucRxAmsduInAmpdu;
	tag->u4TxMaxAmsduInAmpduLen = cmd->u4TxMaxAmsduInAmpduLen;

	if (cmd->ucDesiredPhyTypeSet & PHY_TYPE_SET_802_11AX) {
		tag->u2TxBaSize = cmd->rBaSize.rHeBaSize.u2TxBaSize;
		tag->u2RxBaSize = cmd->rBaSize.rHeBaSize.u2RxBaSize;
	} else {
		tag->u2TxBaSize = cmd->rBaSize.rHtVhtBaSize.ucTxBaSize;
		tag->u2RxBaSize = cmd->rBaSize.rHtVhtBaSize.ucRxBaSize;
	}
	return tag->u2Length;
}

uint32_t nicUniCmdStaRecTagUapsd(struct ADAPTER *ad,
	uint8_t *buf, struct CMD_UPDATE_STA_RECORD *cmd)
{
	struct UNI_CMD_STAREC_UAPSD_INFO *tag =
		(struct UNI_CMD_STAREC_UAPSD_INFO *)buf;

	tag->u2Tag = UNI_CMD_STAREC_TAG_UAPSD;
	tag->u2Length = sizeof(*tag);
	tag->fgIsUapsdSupported = cmd->ucIsUapsdSupported;
	tag->ucUapsdAc = cmd->ucUapsdAc;
	tag->ucUapsdSp = cmd->ucUapsdSp;
	return tag->u2Length;
}

struct UNI_CMD_STAREC_TAG_HANDLE arUpdateStaRecTable[] = {
	{sizeof(struct UNI_CMD_STAREC_BASIC), nicUniCmdStaRecTagBasic},
	{sizeof(struct UNI_CMD_STAREC_HT_INFO), nicUniCmdStaRecTagHtInfo},
	{sizeof(struct UNI_CMD_STAREC_VHT_INFO), nicUniCmdStaRecTagVhtInfo},
#if (CFG_SUPPORT_802_11AX == 1)
	{sizeof(struct UNI_CMD_STAREC_HE_BASIC), nicUniCmdStaRecTagHeBasic},
	{sizeof(struct UNI_CMD_STAREC_HE_6G_CAP), nicUniCmdStaRecTagHe6gCap},
#endif
	{sizeof(struct UNI_CMD_STAREC_STATE_INFO), nicUniCmdStaRecTagState},
	{sizeof(struct UNI_CMD_STAREC_PHY_INFO), nicUniCmdStaRecTagPhyInfo},
	{sizeof(struct UNI_CMD_STAREC_RA_INFO), nicUniCmdStaRecTagRA},
	{sizeof(struct UNI_CMD_STAREC_BA_OFFLOAD_INFO),
		nicUniCmdStaRecTagBAOffload},
	{sizeof(struct UNI_CMD_STAREC_UAPSD_INFO), nicUniCmdStaRecTagUapsd}
};

void nicUniCmdStaRecHandleEventPkt(IN struct ADAPTER
	*prAdapter, IN struct CMD_INFO *prCmdInfo,
	IN uint8_t *pucEventBuf)
{
	struct UNI_CMD_STAREC *uni_cmd =
		(struct UNI_CMD_STAREC *) GET_UNI_CMD_DATA(prCmdInfo);
	struct UNI_EVENT_CMD_RESULT *evt =
		(struct UNI_EVENT_CMD_RESULT *)pucEventBuf;

	DBGLOG(NIC, TRACE,
		"cmd_result:ucCID=0x%x, status=%d, wlanidx=%d\n",
		evt->u2CID, evt->u4Status, uni_cmd->ucWlanIdxL);

	if (evt->u2CID == UNI_CMD_ID_STAREC_INFO && evt->u4Status == 0) {
		struct STA_RECORD *prStaRec = cnmGetStaRecByIndex(prAdapter,
			secGetStaIdxByWlanIdx(prAdapter, uni_cmd->ucWlanIdxL));

		qmActivateStaRec(prAdapter, prStaRec);
	}
}

uint32_t nicUniCmdUpdateStaRec(struct ADAPTER *ad,
		struct WIFI_UNI_SETQUERY_INFO *info)
{
	struct CMD_UPDATE_STA_RECORD *cmd;
	struct UNI_CMD_STAREC *uni_cmd;
	struct WIFI_UNI_CMD_ENTRY *entry;
	struct BSS_INFO *bss;
	uint8_t *pos;
	uint32_t max_cmd_len = 0;
	int i;

	if (info->ucCID != CMD_ID_UPDATE_STA_RECORD ||
	    info->u4SetQueryInfoLen != sizeof(*cmd))
		return WLAN_STATUS_NOT_ACCEPTED;

	cmd = (struct CMD_UPDATE_STA_RECORD *) info->pucInfoBuffer;
	bss = GET_BSS_INFO_BY_INDEX(ad, cmd->ucBssIndex);
	max_cmd_len += sizeof(struct UNI_CMD_STAREC);
	for (i = 0; i < ARRAY_SIZE(arUpdateStaRecTable); i++)
		max_cmd_len += arUpdateStaRecTable[i].u4Size;
	entry = nicUniCmdAllocEntry(ad, UNI_CMD_ID_STAREC_INFO, max_cmd_len,
		cmd->ucNeedResp ? nicUniCmdStaRecHandleEventPkt : NULL,	NULL);
	if (!entry)
		return WLAN_STATUS_RESOURCES;

	pos = entry->pucInfoBuffer;
	uni_cmd = (struct UNI_CMD_STAREC *) pos;
	uni_cmd->ucBssInfoIdx = cmd->ucBssIndex;
	WCID_SET_H_L(uni_cmd->ucWlanIdxHnVer, uni_cmd->ucWlanIdxL,
		cmd->ucWlanIndex);
	pos += sizeof(*uni_cmd);
	for (i = 0; i < ARRAY_SIZE(arUpdateStaRecTable); i++)
		pos += arUpdateStaRecTable[i].pfHandler(ad, pos, cmd);
	entry->u4SetQueryInfoLen = pos - entry->pucInfoBuffer;

	LINK_INSERT_TAIL(&info->rUniCmdList, &entry->rLinkEntry);

	return WLAN_STATUS_SUCCESS;
}

uint32_t nicUniCmdChPrivilege(struct ADAPTER *ad,
		struct WIFI_UNI_SETQUERY_INFO *info)
{
	struct CMD_CH_PRIVILEGE *cmd;
	struct UNI_CMD_CNM *uni_cmd;
	struct WIFI_UNI_CMD_ENTRY *entry;
	uint32_t max_cmd_len = sizeof(struct UNI_CMD_CNM);

	if (info->ucCID != CMD_ID_CH_PRIVILEGE ||
	    info->u4SetQueryInfoLen != sizeof(*cmd))
		return WLAN_STATUS_NOT_ACCEPTED;

	cmd = (struct CMD_CH_PRIVILEGE *) info->pucInfoBuffer;
	max_cmd_len += cmd->ucAction == CMD_CH_ACTION_ABORT ?
		sizeof(struct UNI_CMD_CNM_CH_PRIVILEGE_ABORT) :
		sizeof(struct UNI_CMD_CNM_CH_PRIVILEGE_REQ);
	entry = nicUniCmdAllocEntry(ad, UNI_CMD_ID_CNM,
			max_cmd_len, NULL, NULL);
	if (!entry)
		return WLAN_STATUS_RESOURCES;

	uni_cmd = (struct UNI_CMD_CNM *) entry->pucInfoBuffer;
	if (cmd->ucAction == CMD_CH_ACTION_ABORT) {
		struct UNI_CMD_CNM_CH_PRIVILEGE_ABORT *tag =
			(struct UNI_CMD_CNM_CH_PRIVILEGE_ABORT *)
			uni_cmd->aucTlvBuffer;

		tag->u2Tag = UNI_CMD_CNM_TAG_CH_PRIVILEGE_ABORT;
		tag->u2Length = sizeof(*tag);
		tag->ucBssIndex = cmd->ucBssIndex;
		tag->ucTokenID = cmd->ucTokenID;
		tag->ucDBDCBand = cmd->ucDBDCBand;
	} else {
		struct UNI_CMD_CNM_CH_PRIVILEGE_REQ *tag =
			(struct UNI_CMD_CNM_CH_PRIVILEGE_REQ *)
			uni_cmd->aucTlvBuffer;

		tag->u2Tag = UNI_CMD_CNM_TAG_CH_PRIVILEGE_REQ;
		tag->u2Length = sizeof(*tag);
		tag->ucBssIndex = cmd->ucBssIndex;
		tag->ucTokenID = cmd->ucTokenID;
		tag->ucPrimaryChannel = cmd->ucPrimaryChannel;
		tag->ucRfSco = cmd->ucRfSco;
		tag->ucRfBand = cmd->ucRfBand;
		tag->ucRfChannelWidth = cmd->ucRfChannelWidth;
		tag->ucRfCenterFreqSeg1 = cmd->ucRfCenterFreqSeg1;
		tag->ucRfCenterFreqSeg2 = cmd->ucRfCenterFreqSeg2;
		tag->ucRfChannelWidthFromAP = cmd->ucRfChannelWidth;
		tag->ucRfCenterFreqSeg1FromAP = cmd->ucRfCenterFreqSeg1;
		tag->ucRfCenterFreqSeg2FromAP = cmd->ucRfCenterFreqSeg2;
		tag->ucReqType = cmd->ucReqType;
		tag->u4MaxInterval = cmd->u4MaxInterval;
		tag->ucDBDCBand = cmd->ucDBDCBand;
	}

	LINK_INSERT_TAIL(&info->rUniCmdList, &entry->rLinkEntry);

	return WLAN_STATUS_SUCCESS;
}

uint32_t nicUniCmdAccessReg(struct ADAPTER *ad,
		struct WIFI_UNI_SETQUERY_INFO *info)
{
	struct CMD_ACCESS_REG *cmd;
	struct UNI_CMD_ACCESS_REG *uni_cmd;
	struct UNI_CMD_ACCESS_REG_BASIC *tag;
	struct WIFI_UNI_CMD_ENTRY *entry;
	uint32_t max_cmd_len = sizeof(struct UNI_CMD_ACCESS_REG) +
			       sizeof(struct UNI_CMD_ACCESS_REG_BASIC);

	if (info->ucCID != CMD_ID_ACCESS_REG ||
	    info->u4SetQueryInfoLen != sizeof(*cmd))
		return WLAN_STATUS_NOT_ACCEPTED;

	cmd = (struct CMD_ACCESS_REG *) info->pucInfoBuffer;

	if (info->fgSetQuery)
		entry = nicUniCmdAllocEntry(ad, UNI_CMD_ID_ACCESS_REG,
				max_cmd_len, nicUniCmdEventSetCommon,
				nicUniCmdTimeoutCommon);
	else
		entry = nicUniCmdAllocEntry(ad, UNI_CMD_ID_ACCESS_REG,
				max_cmd_len, nicUniCmdEventQueryMcrRead,
				nicUniCmdTimeoutCommon);

	if (!entry)
		return WLAN_STATUS_RESOURCES;

	uni_cmd = (struct UNI_CMD_ACCESS_REG *) entry->pucInfoBuffer;
	tag = (struct UNI_CMD_ACCESS_REG_BASIC *) uni_cmd->aucTlvBuffer;
	tag->u2Tag = UNI_CMD_ACCESS_REG_TAG_BASIC;
	tag->u2Length = sizeof(*tag);
	tag->u4Addr = cmd->u4Address;
	tag->u4Value = cmd->u4Data;

	LINK_INSERT_TAIL(&info->rUniCmdList, &entry->rLinkEntry);

	return WLAN_STATUS_SUCCESS;
}

uint32_t nicUniCmdUpdateEdcaSet(struct ADAPTER *ad,
		struct WIFI_UNI_SETQUERY_INFO *info)
{
	struct CMD_UPDATE_WMM_PARMS *cmd;
	struct UNI_CMD_EDCA *uni_cmd;
	struct UNI_CMD_EDCA_AC_PARM *tag;
	struct WIFI_UNI_CMD_ENTRY *entry;
	uint32_t max_cmd_len = sizeof(struct UNI_CMD_EDCA) +
			       sizeof(struct UNI_CMD_EDCA_AC_PARM) * AC_NUM;
	uint8_t i;

	if (info->ucCID != CMD_ID_UPDATE_WMM_PARMS ||
	    info->u4SetQueryInfoLen != sizeof(*cmd))
		return WLAN_STATUS_NOT_ACCEPTED;

	cmd = (struct CMD_UPDATE_WMM_PARMS *) info->pucInfoBuffer;
	entry = nicUniCmdAllocEntry(ad, UNI_CMD_ID_EDCA_SET,
					max_cmd_len, NULL, NULL);

	if (!entry)
		return WLAN_STATUS_RESOURCES;

	uni_cmd = (struct UNI_CMD_EDCA *) entry->pucInfoBuffer;
	uni_cmd->ucBssInfoIdx = cmd->ucBssIndex;
	tag = (struct UNI_CMD_EDCA_AC_PARM *) uni_cmd->aucTlvBuffer;
	for (i = 0; i < AC_NUM; ++i) {
		tag->u2Tag = UNI_CMD_EDCA_TAG_AC_PARM;
		tag->u2Length = sizeof(*tag);
		tag->ucAcIndex = i;
		tag->ucValidBitmap = UNI_CMD_EDCA_ALL_BITS;
		tag->ucCWmin = (uint8_t) ffs(cmd->arACQueParms[i].u2CWmin + 1)
			- 1;
		tag->ucCWmax = (uint8_t) ffs(cmd->arACQueParms[i].u2CWmax + 1)
			- 1;
		tag->u2TxopLimit = cmd->arACQueParms[i].u2TxopLimit;
		tag->ucAifsn = (uint8_t) cmd->arACQueParms[i].u2Aifsn;
		tag++;
	}

	LINK_INSERT_TAIL(&info->rUniCmdList, &entry->rLinkEntry);

	return WLAN_STATUS_SUCCESS;
}

uint32_t nicUniCmdUpdateMuEdca(struct ADAPTER *ad,
		struct WIFI_UNI_SETQUERY_INFO *info)
{
#if (CFG_SUPPORT_802_11AX == 1)
	struct _CMD_MQM_UPDATE_MU_EDCA_PARMS_T *cmd;
	struct UNI_CMD_MQM_UPDATE_MU_EDCA *uni_cmd;
	struct UNI_CMD_UPDATE_MU_EDCA *tag;
	struct WIFI_UNI_CMD_ENTRY *entry;
	uint32_t max_cmd_len = sizeof(struct UNI_CMD_MQM_UPDATE_MU_EDCA) +
			       sizeof(struct UNI_CMD_UPDATE_MU_EDCA);
	uint8_t i;

	if (info->ucCID != CMD_ID_MQM_UPDATE_MU_EDCA_PARMS ||
	    info->u4SetQueryInfoLen != sizeof(*cmd))
		return WLAN_STATUS_NOT_ACCEPTED;

	cmd = (struct _CMD_MQM_UPDATE_MU_EDCA_PARMS_T *) info->pucInfoBuffer;
	entry = nicUniCmdAllocEntry(ad, UNI_CMD_ID_MQM_UPDATE_MU_EDCA_PARMS,
			max_cmd_len, NULL, NULL);
	if (!entry)
		return WLAN_STATUS_RESOURCES;

	uni_cmd = (struct UNI_CMD_MQM_UPDATE_MU_EDCA *) entry->pucInfoBuffer;
	tag = (struct UNI_CMD_UPDATE_MU_EDCA *) uni_cmd->aucTlvBuffer;
	tag->u2Tag = UNI_CMD_MQM_UPDATE_MU_EDCA_TAG_PARMS;
	tag->u2Length = sizeof(*tag);
	tag->ucBssIndex = cmd->ucBssIndex;
	tag->fgIsQBSS = cmd->fgIsQBSS;
	tag->ucWmmSet = cmd->ucWmmSet;

	for (i = 0; i < 4; i++) {
		tag->arMUEdcaParams[i].ucECWmin =
			cmd->arMUEdcaParams[i].ucECWmin;
		tag->arMUEdcaParams[i].ucECWmax =
			cmd->arMUEdcaParams[i].ucECWmax;
		tag->arMUEdcaParams[i].ucAifsn =
			cmd->arMUEdcaParams[i].ucAifsn;
		tag->arMUEdcaParams[i].ucIsACMSet =
			cmd->arMUEdcaParams[i].ucIsACMSet;
		tag->arMUEdcaParams[i].ucMUEdcaTimer =
			cmd->arMUEdcaParams[i].ucMUEdcaTimer;
	}

	LINK_INSERT_TAIL(&info->rUniCmdList, &entry->rLinkEntry);

	return WLAN_STATUS_SUCCESS;
#else
	return WLAN_STATUS_NOT_SUPPORTED;
#endif
}

uint32_t nicUniCmdOffloadIPV4(struct ADAPTER *ad,
		struct WIFI_UNI_SETQUERY_INFO *info)
{
	struct CMD_SET_NETWORK_ADDRESS_LIST *cmd;
	struct UNI_CMD_OFFLOAD *uni_cmd;
	struct UNI_CMD_OFFLOAD_ARPNS_IPV4 *tag;
	struct IPV4_ADDRESS *ipv4;
	struct WIFI_UNI_CMD_ENTRY *entry;
	uint32_t max_cmd_len;
	uint8_t i;

	if (info->ucCID != CMD_ID_SET_IP_ADDRESS)
		return WLAN_STATUS_NOT_ACCEPTED;

	cmd = (struct CMD_SET_NETWORK_ADDRESS_LIST *) info->pucInfoBuffer;
	max_cmd_len = sizeof(struct UNI_CMD_OFFLOAD) +
		      sizeof(struct UNI_CMD_OFFLOAD_ARPNS_IPV4) +
		      sizeof(struct IPV4_ADDRESS) * cmd->ucAddressCount;
	entry = nicUniCmdAllocEntry(ad, UNI_CMD_ID_OFFLOAD,
		max_cmd_len, nicUniCmdEventSetCommon, nicUniCmdTimeoutCommon);
	if (!entry)
		return WLAN_STATUS_RESOURCES;

	uni_cmd = (struct UNI_CMD_OFFLOAD *) entry->pucInfoBuffer;
	uni_cmd->ucBssInfoIdx = cmd->ucBssIndex;
	tag = (struct UNI_CMD_OFFLOAD_ARPNS_IPV4 *) uni_cmd->aucTlvBuffer;
	tag->u2Tag = UNI_CMD_OFFLOAD_TAG_ARPNS_IPV4;
	tag->u2Length = sizeof(*tag) +
			sizeof(struct IPV4_ADDRESS) * cmd->ucAddressCount;
	tag->ucIpv4AddressCount = cmd->ucAddressCount;
	tag->ucVersion = cmd->ucVersion;
	ipv4 = tag->arIpv4NetAddress;
	for (i = 0; i < cmd->ucAddressCount; i++) {
		kalMemCopy(ipv4, &cmd->arNetAddress[i], sizeof(*ipv4));
		ipv4++;
	}

	LINK_INSERT_TAIL(&info->rUniCmdList, &entry->rLinkEntry);

	return WLAN_STATUS_SUCCESS;
}

uint32_t nicUniCmdOffloadIPV6(struct ADAPTER *ad,
		struct WIFI_UNI_SETQUERY_INFO *info)
{
	struct CMD_IPV6_NETWORK_ADDRESS_LIST *cmd;
	struct UNI_CMD_OFFLOAD *uni_cmd;
	struct UNI_CMD_OFFLOAD_ARPNS_IPV6 *tag;
	struct IPV6_ADDRESS *ipv6;
	struct WIFI_UNI_CMD_ENTRY *entry;
	uint32_t max_cmd_len;
	uint8_t i;

	if (info->ucCID != CMD_ID_SET_IPV6_ADDRESS)
		return WLAN_STATUS_NOT_ACCEPTED;

	cmd = (struct CMD_IPV6_NETWORK_ADDRESS_LIST *) info->pucInfoBuffer;
	max_cmd_len = sizeof(struct UNI_CMD_OFFLOAD) +
		      sizeof(struct UNI_CMD_OFFLOAD_ARPNS_IPV6) +
		      sizeof(struct IPV6_ADDRESS) * cmd->ucAddressCount;
	entry = nicUniCmdAllocEntry(ad, UNI_CMD_ID_OFFLOAD,
		max_cmd_len, nicUniCmdEventSetCommon, nicUniCmdTimeoutCommon);
	if (!entry)
		return WLAN_STATUS_RESOURCES;

	uni_cmd = (struct UNI_CMD_OFFLOAD *) entry->pucInfoBuffer;
	uni_cmd->ucBssInfoIdx = cmd->ucBssIndex;
	tag = (struct UNI_CMD_OFFLOAD_ARPNS_IPV6 *) uni_cmd->aucTlvBuffer;
	tag->u2Tag = UNI_CMD_OFFLOAD_TAG_ARPNS_IPV6;
	tag->u2Length = sizeof(*tag) +
			sizeof(struct IPV6_ADDRESS) * cmd->ucAddressCount;
	tag->ucIpv6AddressCount = cmd->ucAddressCount;
	ipv6 = tag->arIpv6NetAddress;
	for (i = 0; i < cmd->ucAddressCount; i++) {
		kalMemCopy(ipv6, &cmd->arNetAddress[i], sizeof(*ipv6));
		ipv6++;
	}

	LINK_INSERT_TAIL(&info->rUniCmdList, &entry->rLinkEntry);

	return WLAN_STATUS_SUCCESS;
}

uint32_t nicUniCmdGetIdcChnl(struct ADAPTER *ad,
		struct WIFI_UNI_SETQUERY_INFO *info)
{
	struct CMD_GET_LTE_SAFE_CHN *cmd;
	struct UNI_CMD_IDC *uni_cmd;
	struct UNI_CMD_GET_IDC_CHN *tag;
	struct WIFI_UNI_CMD_ENTRY *entry;
	uint32_t max_cmd_len = sizeof(struct UNI_CMD_IDC) +
			       sizeof(struct UNI_CMD_GET_IDC_CHN);

	if (info->ucCID != CMD_ID_GET_LTE_CHN ||
	    info->u4SetQueryInfoLen != sizeof(*cmd))
		return WLAN_STATUS_NOT_ACCEPTED;

	cmd = (struct CMD_GET_LTE_SAFE_CHN *) info->pucInfoBuffer;
	entry = nicUniCmdAllocEntry(ad, UNI_CMD_ID_IDC,
		max_cmd_len, nicUniEventQueryIdcChnl, nicUniCmdTimeoutCommon);
	if (!entry)
		return WLAN_STATUS_RESOURCES;

	uni_cmd = (struct UNI_CMD_IDC *) entry->pucInfoBuffer;
	tag = (struct UNI_CMD_GET_IDC_CHN *) uni_cmd->aucTlvBuffer;
	tag->u2Tag = UNI_CMD_IDC_TAG_GET_IDC_CHN;
	tag->u2Length = sizeof(*tag);

	LINK_INSERT_TAIL(&info->rUniCmdList, &entry->rLinkEntry);

	return WLAN_STATUS_SUCCESS;
}

/*******************************************************************************
 *                                 Event
 *******************************************************************************
 */

void nicRxProcessUniEventPacket(IN struct ADAPTER *prAdapter,
			     IN OUT struct SW_RFB *prSwRfb)
{
	struct mt66xx_chip_info *prChipInfo;
	struct CMD_INFO *prCmdInfo;
	struct WIFI_UNI_EVENT *prEvent;

	ASSERT(prAdapter);
	ASSERT(prSwRfb);
	prChipInfo = prAdapter->chip_info;
	prEvent = (struct WIFI_UNI_EVENT *)
			(prSwRfb->pucRecvBuff + prChipInfo->rxd_size);

	if (IS_UNI_UNSOLICIT_EVENT(prEvent)) {
		if (arUniEventTable[GET_UNI_EVENT_ID(prEvent)])
			arUniEventTable[GET_UNI_EVENT_ID(prEvent)](
				prAdapter, prEvent);
	} else {
		prCmdInfo = nicGetPendingCmdInfo(prAdapter,
						 prEvent->ucSeqNum);

		if (prCmdInfo != NULL) {
			if (prCmdInfo->pfCmdDoneHandler)
				prCmdInfo->pfCmdDoneHandler(
					prAdapter, prCmdInfo,
					prEvent->aucBuffer);
			else if (prCmdInfo->fgIsOid)
				kalOidComplete(
					prAdapter->prGlueInfo,
					prCmdInfo->fgSetQuery,
					0,
					WLAN_STATUS_SUCCESS);

			/* return prCmdInfo */
			cmdBufFreeCmdInfo(prAdapter, prCmdInfo);
		} else {
			DBGLOG(RX, INFO,
				"UNHANDLED RX EVENT: ID[0x%02X] SEQ[%u] LEN[%u]\n",
			  prEvent->ucEID, prEvent->ucSeqNum,
			  prEvent->u2PacketLength);
		}
	}

	/* Reset Chip NoAck flag */
	if (prAdapter->fgIsChipNoAck) {
		DBGLOG_LIMITED(RX, WARN,
		       "Got response from chip, clear NoAck flag!\n");
		WARN_ON(TRUE);
	}
	prAdapter->ucOidTimeoutCount = 0;
	prAdapter->fgIsChipNoAck = FALSE;

	nicRxReturnRFB(prAdapter, prSwRfb);
}

void nicUniCmdEventSetCommon(IN struct ADAPTER
	*prAdapter, IN struct CMD_INFO *prCmdInfo,
	IN uint8_t *pucEventBuf)
{
	struct UNI_EVENT_CMD_RESULT *evt =
		(struct UNI_EVENT_CMD_RESULT *)pucEventBuf;

	DBGLOG(NIC, TRACE,
		"cmd_result:ucCID=0x%x, status=%d, prCmdInfo:ucCID=%d\n",
		evt->u2CID, evt->u4Status, prCmdInfo->ucCID);

	nicCmdEventSetCommon(prAdapter, prCmdInfo, pucEventBuf);
}

void nicUniCmdTimeoutCommon(IN struct ADAPTER *prAdapter,
			    IN struct CMD_INFO *prCmdInfo)
{
	ASSERT(prAdapter);

	DBGLOG(NIC, TRACE,
		"prCmdInfo:ucCID=0x%x isOid=%d timeout\n",
		prCmdInfo->ucCID, prCmdInfo->fgIsOid);

	nicOidCmdTimeoutCommon(prAdapter, prCmdInfo);
}

void nicUniCmdEventQueryCfgRead(IN struct ADAPTER *prAdapter,
	IN struct CMD_INFO *prCmdInfo, IN uint8_t *pucEventBuf)
{
	struct UNI_EVENT_CHIP_CONFIG *evt =
		(struct UNI_EVENT_CHIP_CONFIG *)pucEventBuf;
	struct UNI_CMD_CHIP_CONFIG_CUSTOMER_CFG *tag =
		(struct UNI_CMD_CHIP_CONFIG_CUSTOMER_CFG *) evt->aucTlvBuffer;
	struct CMD_HEADER legacy;

	legacy.itemNum = tag->itemNum;
	legacy.cmdBufferLen = tag->cmdBufferLen;
	kalMemCopy(legacy.buffer, tag->aucbuffer, tag->cmdBufferLen);

	nicCmdEventQueryCfgRead(prAdapter, prCmdInfo, (uint8_t *)&legacy);
}

void nicUniEventQueryChipConfig(IN struct ADAPTER *prAdapter,
	IN struct CMD_INFO *prCmdInfo, IN uint8_t *pucEventBuf)
{
	struct UNI_EVENT_CHIP_CONFIG *evt =
		(struct UNI_EVENT_CHIP_CONFIG *)pucEventBuf;
	struct UNI_CMD_CHIP_CONFIG_CHIP_CFG *tag =
		(struct UNI_CMD_CHIP_CONFIG_CHIP_CFG *) evt->aucTlvBuffer;
	struct UNI_CMD_CHIP_CONFIG_CHIP_CFG_RESP *resp =
		(struct UNI_CMD_CHIP_CONFIG_CHIP_CFG_RESP *) tag->aucbuffer;
	struct CMD_CHIP_CONFIG legacy;

	legacy.u2Id = resp->u2Id;
	legacy.ucType = resp->ucType;
	legacy.ucRespType = resp->ucRespType;
	legacy.u2MsgSize = resp->u2MsgSize;
	kalMemCopy(legacy.aucCmd, resp->aucCmd, resp->u2MsgSize);

	nicCmdEventQueryChipConfig(prAdapter, prCmdInfo, (uint8_t *)&legacy);
}

void nicUniEventQueryIdcChnl(IN struct ADAPTER *prAdapter,
		IN struct CMD_INFO *prCmdInfo,
		IN uint8_t *pucEventBuf)
{
	struct UNI_EVENT_IDC *evt = (struct UNI_EVENT_IDC *)pucEventBuf;
	struct UNI_EVENT_MD_SAFE_CHN *tag =
		(struct UNI_EVENT_MD_SAFE_CHN *) evt->aucTlvBuffer;
	struct EVENT_LTE_SAFE_CHN legacy;

	legacy.ucVersion = tag->ucVersion;
	legacy.u4Flags = tag->u4Flags;
	kalMemCopy(legacy.rLteSafeChn.au4SafeChannelBitmask,
		   tag->u4SafeChannelBitmask,
		   sizeof(uint32_t) * ENUM_SAFE_CH_MASK_MAX_NUM);

	nicCmdEventQueryLteSafeChn(prAdapter, prCmdInfo, (uint8_t *)&legacy);
}

void nicUniEventBFStaRec(IN struct ADAPTER *prAdapter,
	IN struct CMD_INFO *prCmdInfo, IN uint8_t *pucEventBuf)
{
	struct UNI_EVENT_BF *evt =
		(struct UNI_EVENT_BF *)pucEventBuf;
	struct UNI_EVENT_BF_STA_REC *tag =
		(struct UNI_EVENT_BF_STA_REC *) evt->au1TlvBuffer;
	struct TXBF_PFMU_STA_INFO *info =
		(struct TXBF_PFMU_STA_INFO *) &tag->rTxBfPfmuInfo;

	DBGLOG(RFTEST, INFO,
		 "====================================== BF StaRec ========================================\n"
		 "rStaRecBf.u2PfmuId	  = %d\n"
		 "rStaRecBf.fgSU_MU	  = %d\n"
		 "rStaRecBf.u1TxBfCap	  = %d\n"
		 "rStaRecBf.ucSoundingPhy = %d\n"
		 "rStaRecBf.ucNdpaRate	  = %d\n"
		 "rStaRecBf.ucNdpRate	  = %d\n"
		 "rStaRecBf.ucReptPollRate= %d\n"
		 "rStaRecBf.ucTxMode	  = %d\n"
		 "rStaRecBf.ucNc	  = %d\n"
		 "rStaRecBf.ucNr	  = %d\n"
		 "rStaRecBf.ucCBW	  = %d\n"
		 "rStaRecBf.ucTotMemRequire = %d\n"
		 "rStaRecBf.ucMemRequire20M = %d\n"
		 "rStaRecBf.ucMemRow0	  = %d\n"
		 "rStaRecBf.ucMemCol0	  = %d\n"
		 "rStaRecBf.ucMemRow1	  = %d\n"
		 "rStaRecBf.ucMemCol1	  = %d\n"
		 "rStaRecBf.ucMemRow2	  = %d\n"
		 "rStaRecBf.ucMemCol2	  = %d\n"
		 "rStaRecBf.ucMemRow3	  = %d\n"
		 "rStaRecBf.ucMemCol3	  = %d\n",
		 info->u2PfmuId,
		 info->fgSU_MU,
		 info->u1TxBfCap,
		 info->ucSoundingPhy,
		 info->ucNdpaRate,
		 info->ucNdpRate,
		 info->ucReptPollRate,
		 info->ucTxMode,
		 info->ucNc,
		 info->ucNr,
		 info->ucCBW,
		 info->ucTotMemRequire,
		 info->ucMemRequire20M,
		 info->ucMemRow0,
		 info->ucMemCol0,
		 info->ucMemRow1,
		 info->ucMemCol1,
		 info->ucMemRow2,
		 info->ucMemCol2,
		 info->ucMemRow3,
		 info->ucMemCol3);
	DBGLOG(RFTEST, INFO,
		 "rStaRecBf.u2SmartAnt	  = 0x%x\n"
		 "rStaRecBf.ucSEIdx	  = %d\n"
		 "rStaRecBf.ucAutoSoundingCtrl = %d\n"
		 "rStaRecBf.uciBfTimeOut  = 0x%x\n"
		 "rStaRecBf.uciBfDBW	  = %d\n"
		 "rStaRecBf.uciBfNcol	  = %d\n"
		 "rStaRecBf.uciBfNrow	  = %d\n"
		 "rStaRecBf.nr_bw160	  = %d\n"
		 "rStaRecBf.nc_bw160	  = %d\n"
		 "rStaRecBf.ru_start_idx  = %d\n"
		 "rStaRecBf.ru_end_idx	  = %d\n"
		 "rStaRecBf.trigger_su	  = %d\n"
		 "rStaRecBf.trigger_mu	  = %d\n"
		 "rStaRecBf.ng16_su	  = %d\n"
		 "rStaRecBf.ng16_mu	  = %d\n"
		 "rStaRecBf.codebook42_su = %d\n"
		 "rStaRecBf.codebook75_mu = %d\n"
		 "rStaRecBf.he_ltf	      = %d\n"
		 "=======================================================================================\n",
		 info->u2SmartAnt,
		 info->ucSEIdx,
		 info->ucAutoSoundingCtrl,
		 info->uciBfTimeOut,
		 info->uciBfDBW,
		 info->uciBfNcol,
		 info->uciBfNrow,
		 info->u1NrBw160,
		 info->u1NcBw160,
		 info->u1RuStartIdx,
		 info->u1RuEndIdx,
		 info->fgTriggerSu,
		 info->fgTriggerMu,
		 info->fgNg16Su,
		 info->fgNg16Mu,
		 info->fgCodebook42Su,
		 info->fgCodebook75Mu,
		 info->u1HeLtf);
	nicCmdEventSetCommon(prAdapter, prCmdInfo, pucEventBuf);
}

void nicUniCmdEventQueryMcrRead(IN struct ADAPTER *prAdapter,
	IN struct CMD_INFO *prCmdInfo, IN uint8_t *pucEventBuf)
{
	struct UNI_EVENT_ACCESS_REG *evt =
		(struct UNI_EVENT_ACCESS_REG *)pucEventBuf;
	struct UNI_EVENT_ACCESS_REG_BASIC *tag =
		(struct UNI_EVENT_ACCESS_REG_BASIC *) evt->aucTlvBuffer;
	struct CMD_ACCESS_REG legacy;

	legacy.u4Address = tag->u4Addr;
	legacy.u4Data = tag->u4Value;

	nicCmdEventQueryMcrRead(prAdapter, prCmdInfo, (uint8_t *)&legacy);
}

void nicUniCmdEventGetTsfDone(IN struct ADAPTER *prAdapter,
	IN struct CMD_INFO *prCmdInfo, IN uint8_t *pucEventBuf)
{
	struct UNI_EVENT_MAC_IFNO *evt =
		(struct UNI_EVENT_MAC_IFNO *)pucEventBuf;
	struct UNI_EVENT_MAC_INFO_TSF *tag =
		(struct UNI_EVENT_MAC_INFO_TSF *) evt->aucTlvBuffer;
	struct EXT_EVENT_MAC_INFO_T legacy;

	legacy.rMacInfoResult.rTsfResult.u4TsfBitsLow = tag->u4TsfBit0_31;
	legacy.rMacInfoResult.rTsfResult.u4TsfBitsHigh = tag->u4TsfBit63_32;

	twtPlannerGetTsfDone(prAdapter, prCmdInfo, (uint8_t *)&legacy);
}

void nicUniEventScanDone(struct ADAPTER *ad, struct WIFI_UNI_EVENT *evt)
{
	int32_t tags_len;
	uint8_t *tag;
	uint16_t offset = 0;
	uint32_t fixed_len = sizeof(struct UNI_EVENT_SCAN_DONE);
	uint32_t data_len = GET_UNI_EVENT_DATA_LEN(evt);
	uint8_t *data = GET_UNI_EVENT_DATA(evt);
	uint32_t fail_cnt = 0;
	int i;
	struct UNI_EVENT_SCAN_DONE *scan_done;
	struct EVENT_SCAN_DONE legacy;

	scan_done = (struct UNI_EVENT_SCAN_DONE *) data;
	legacy.ucSeqNum = scan_done->ucSeqNum;

	tags_len = data_len - fixed_len;
	tag = data + fixed_len;
	TAG_FOR_EACH(tag, tags_len, offset) {
		DBGLOG(SCN, INFO, "Tag(%d, %d)\n", TAG_ID(tag), TAG_LEN(tag));

		switch (TAG_ID(tag)) {
		case UNI_EVENT_SCAN_DONE_TAG_BASIC: {
			struct UNI_EVENT_SCAN_DONE_BASIC *basic =
				(struct UNI_EVENT_SCAN_DONE_BASIC *) tag;

			legacy.ucCompleteChanCount = basic->ucCompleteChanCount;
			legacy.ucCurrentState = basic->ucCurrentState;
			legacy.ucScanDoneVersion = basic->ucScanDoneVersion;
			legacy.fgIsPNOenabled = basic->fgIsPNOenabled;
			legacy.u4ScanDurBcnCnt = basic->u4ScanDurBcnCnt;
		}
			break;
		case UNI_EVENT_SCAN_DONE_TAG_SPARSECHNL: {
			struct UNI_EVENT_SCAN_DONE_SPARSECHNL *sparse =
				(struct UNI_EVENT_SCAN_DONE_SPARSECHNL *) tag;

			legacy.ucSparseChannelValid =
				sparse->ucSparseChannelValid;
			legacy.rSparseChannel.ucBand = sparse->ucBand;
			legacy.rSparseChannel.ucChannelNum =
				sparse->ucChannelNum;
			legacy.ucSparseChannelArrayValidNum =
				sparse->ucSparseChannelArrayValidNum;
		}
			break;
		case UNI_EVENT_SCAN_DONE_TAG_CHNLINFO: {
			struct UNI_EVENT_SCAN_DONE_CHNLINFO *chnlinfo =
				(struct UNI_EVENT_SCAN_DONE_CHNLINFO *) tag;
			struct UNI_EVENT_CHNLINFO *chnl =
				(struct UNI_EVENT_CHNLINFO *)
				chnlinfo->aucChnlInfoBuffer;

			ASSERT(chnlinfo->ucNumOfChnl <
				SCAN_DONE_EVENT_MAX_CHANNEL_NUM);
			ASSERT(chnlinfo->u2Length == sizeof(*chnlinfo) +
				chnlinfo->ucNumOfChnl * sizeof(*chnl));
			for (i = 0; i < chnlinfo->ucNumOfChnl; i++, chnl++) {
				legacy.aucChannelNum[i] =
					chnl->ucChannelNum;
				legacy.au2ChannelIdleTime[i] =
					chnl->u2ChannelIdleTime;
				legacy.aucChannelBAndPCnt[i] =
					chnl->ucChannelBAndPCnt;
				legacy.aucChannelMDRDYCnt[i] =
					chnl->ucChannelMDRDYCnt;
			}
		}
			break;
		case UNI_EVENT_SCAN_DONE_TAG_NLO:{
			/* TODO: uni cmd */
		}
			break;
		default:
			fail_cnt++;
			ASSERT(fail_cnt < MAX_UNI_EVENT_FAIL_TAG_COUNT)
			DBGLOG(SCN, WARN, "invalid tag = %d\n", TAG_ID(tag));
			break;
		}
	}

	scnEventScanDone(ad, &legacy, TRUE);
}

void nicUniEventChMngrHandleChEvent(struct ADAPTER *ad,
	struct WIFI_UNI_EVENT *evt)
{
	int32_t tags_len;
	uint8_t *tag;
	uint16_t offset = 0;
	uint32_t fixed_len = sizeof(struct UNI_EVENT_CNM);
	uint32_t data_len = GET_UNI_EVENT_DATA_LEN(evt);
	uint8_t *data = GET_UNI_EVENT_DATA(evt);
	uint32_t fail_cnt = 0;

	tags_len = data_len - fixed_len;
	tag = data + fixed_len;
	TAG_FOR_EACH(tag, tags_len, offset) {
		DBGLOG(CNM, INFO, "Tag(%d, %d)\n", TAG_ID(tag), TAG_LEN(tag));

		switch (TAG_ID(tag)) {
		case UNI_EVENT_CNM_TAG_CH_PRIVILEGE_GRANT: {
			struct UNI_EVENT_CNM_CH_PRIVILEGE_GRANT *grant =
				(struct UNI_EVENT_CNM_CH_PRIVILEGE_GRANT *)tag;
			struct EVENT_CH_PRIVILEGE *legacy;
			struct WIFI_EVENT *prEvent;

			prEvent = (struct WIFI_EVENT *)kalMemAlloc(
					sizeof(struct WIFI_EVENT) +
					sizeof(struct EVENT_CH_PRIVILEGE),
					VIR_MEM_TYPE);
			if (!prEvent) {
				DBGLOG(NIC, ERROR,
				       "Allocate prEvent failed!\n");
				return;
			}

			legacy = (struct EVENT_CH_PRIVILEGE *)
				prEvent->aucBuffer;

			legacy->ucBssIndex = grant->ucBssIndex;
			legacy->ucTokenID = grant->ucTokenID;
			legacy->ucStatus = grant->ucStatus;
			legacy->ucPrimaryChannel = grant->ucPrimaryChannel;
			legacy->ucRfSco = grant->ucRfSco;
			legacy->ucRfBand = grant->ucRfBand;
			legacy->ucRfChannelWidth = grant->ucRfChannelWidth;
			legacy->ucRfCenterFreqSeg1 = grant->ucRfCenterFreqSeg1;
			legacy->ucRfCenterFreqSeg2 = grant->ucRfCenterFreqSeg2;
			legacy->ucReqType = grant->ucReqType;
			legacy->ucDBDCBand = grant->ucDBDCBand;
			legacy->u4GrantInterval = grant->u4GrantInterval;

			cnmChMngrHandleChEvent(ad, prEvent);

			kalMemFree(prEvent, VIR_MEM_TYPE,
				sizeof(struct WIFI_EVENT) +
				sizeof(struct EVENT_CH_PRIVILEGE));
		}
			break;
		case UNI_EVENT_CNM_TAG_GET_CHANNEL_INFO:
			break;
		case UNI_EVENT_CNM_TAG_GET_BSS_INFO:
			break;
		case UNI_EVENT_CNM_TAG_OPMODE_CHANGE: {
			struct UNI_EVENT_CNM_OPMODE_CHANGE *opmode =
				(struct UNI_EVENT_CNM_OPMODE_CHANGE *)tag;
			struct EVENT_OPMODE_CHANGE *legacy;
			struct WIFI_EVENT *prEvent;

			prEvent = (struct WIFI_EVENT *)kalMemAlloc(
					sizeof(struct WIFI_EVENT) +
					sizeof(struct EVENT_OPMODE_CHANGE),
					VIR_MEM_TYPE);
			if (!prEvent) {
				DBGLOG(NIC, ERROR,
				       "Allocate prEvent failed!\n");
				return;
			}

			legacy = (struct EVENT_OPMODE_CHANGE *)
				prEvent->aucBuffer;

			legacy->ucBssBitmap = (uint8_t) opmode->u2BssBitmap;
			legacy->ucEnable = opmode->ucEnable;
			legacy->ucOpTxNss = opmode->ucOpTxNss;
			legacy->ucOpRxNss = opmode->ucOpRxNss;
			legacy->ucReason = opmode->ucReason;

			cnmOpmodeEventHandler(ad, prEvent);

			kalMemFree(prEvent, VIR_MEM_TYPE,
				sizeof(struct WIFI_EVENT) +
				sizeof(struct EVENT_OPMODE_CHANGE));
		}
			break;
		default:
			fail_cnt++;
			ASSERT(fail_cnt < MAX_UNI_EVENT_FAIL_TAG_COUNT)
			DBGLOG(CNM, WARN, "invalid tag = %d\n", TAG_ID(tag));
			break;
		}
	}
}

void nicUniEventMbmcHandleEvent(struct ADAPTER *ad, struct WIFI_UNI_EVENT *evt)
{
	int32_t tags_len;
	uint8_t *tag;
	uint16_t offset = 0;
	uint32_t fixed_len = sizeof(struct UNI_EVENT_MBMC);
	uint32_t data_len = GET_UNI_EVENT_DATA_LEN(evt);
	uint8_t *data = GET_UNI_EVENT_DATA(evt);
	uint32_t fail_cnt = 0;

	tags_len = data_len - fixed_len;
	tag = data + fixed_len;
	TAG_FOR_EACH(tag, tags_len, offset) {
		DBGLOG(CNM, INFO, "Tag(%d, %d)\n", TAG_ID(tag), TAG_LEN(tag));

		switch (TAG_ID(tag)) {
		case UNI_EVENT_MBMC_TAG_SWITCH_DONE: {
			struct WIFI_EVENT rEvent;

			rEvent.ucSeqNum = evt->ucSeqNum;
			cnmDbdcEventHwSwitchDone(ad, &rEvent);
		}
			break;
		default:
			fail_cnt++;
			ASSERT(fail_cnt < MAX_UNI_EVENT_FAIL_TAG_COUNT)
			DBGLOG(CNM, WARN, "invalid tag = %d\n", TAG_ID(tag));
			break;
		}
	}
}

void nicUniEventStatusToHost(struct ADAPTER *ad, struct WIFI_UNI_EVENT *evt)
{
	int32_t tags_len;
	uint8_t *tag;
	uint16_t offset = 0;
	uint32_t fixed_len = sizeof(struct UNI_EVENT_STATUS_TO_HOST);
	uint32_t data_len = GET_UNI_EVENT_DATA_LEN(evt);
	uint8_t *data = GET_UNI_EVENT_DATA(evt);
	uint32_t fail_cnt = 0;

	tags_len = data_len - fixed_len;
	tag = data + fixed_len;
	TAG_FOR_EACH(tag, tags_len, offset) {
		DBGLOG(NIC, TRACE, "Tag(%d, %d)\n", TAG_ID(tag), TAG_LEN(tag));

		switch (TAG_ID(tag)) {
		case UNI_EVENT_STATUS_TO_HOST_TAG_TX_DONE:{
			struct UNI_EVENT_TX_DONE *tx =
				(struct UNI_EVENT_TX_DONE *) tag;
			struct EVENT_TX_DONE *legacy;
			struct WIFI_EVENT *prEvent;

			prEvent = (struct WIFI_EVENT *)kalMemAlloc(
					sizeof(struct WIFI_EVENT) +
					sizeof(struct EVENT_TX_DONE),
					VIR_MEM_TYPE);
			if (!prEvent) {
				DBGLOG(NIC, ERROR,
				       "Allocate prEvent failed!\n");
				return;
			}

			legacy = (struct EVENT_TX_DONE *)prEvent->aucBuffer;

			legacy->ucPacketSeq = tx->ucPacketSeq;
			legacy->ucStatus = tx->ucStatus;
			legacy->u2SequenceNumber = tx->u2SequenceNumber;
			legacy->ucWlanIndex = tx->ucWlanIndex;
			legacy->ucTxCount = tx->ucTxCount;
			legacy->u2TxRate = tx->u2TxRate;

			legacy->ucFlag = tx->ucFlag;
			legacy->ucTid = tx->ucTid;
			legacy->ucRspRate = tx->ucRspRate;
			legacy->ucRateTableIdx = tx->ucRateTableIdx;

			legacy->ucBandwidth = tx->ucBandwidth;
			legacy->ucTxPower = tx->ucTxPower;
			legacy->ucFlushReason = tx->ucFlushReason;

			legacy->u4TxDelay = tx->u4TxDelay;
			legacy->u4Timestamp = tx->u4Timestamp;
			legacy->u4AppliedFlag = tx->u4AppliedFlag;

			nicTxProcessTxDoneEvent(ad, prEvent);

			kalMemFree(prEvent, VIR_MEM_TYPE,
				sizeof(struct WIFI_EVENT) +
				sizeof(struct EVENT_TX_DONE));
		}
			break;
		default:
			fail_cnt++;
			ASSERT(fail_cnt < MAX_UNI_EVENT_FAIL_TAG_COUNT)
			DBGLOG(NIC, WARN, "invalid tag = %d\n", TAG_ID(tag));
			break;
		}
	}
}

void nicUniEventBaOffload(struct ADAPTER *ad, struct WIFI_UNI_EVENT *evt)
{
	int32_t tags_len;
	uint8_t *tag;
	uint16_t offset = 0;
	uint32_t fixed_len = sizeof(struct UNI_EVENT_BA_OFFLOAD);
	uint32_t data_len = GET_UNI_EVENT_DATA_LEN(evt);
	uint8_t *data = GET_UNI_EVENT_DATA(evt);
	uint32_t fail_cnt = 0;

	tags_len = data_len - fixed_len;
	tag = data + fixed_len;
	TAG_FOR_EACH(tag, tags_len, offset) {
		DBGLOG(NIC, TRACE, "Tag(%d, %d)\n", TAG_ID(tag), TAG_LEN(tag));

		switch (TAG_ID(tag)) {
		case UNI_EVENT_BA_OFFLOAD_TAG_RX_ADDBA:{
			struct UNI_EVENT_RX_ADDBA *ba =
				(struct UNI_EVENT_RX_ADDBA *) tag;
			struct EVENT_RX_ADDBA *legacy;
			struct WIFI_EVENT *prEvent;

			prEvent = (struct WIFI_EVENT *)kalMemAlloc(
					sizeof(struct WIFI_EVENT) +
					sizeof(struct EVENT_RX_ADDBA),
					VIR_MEM_TYPE);
			if (!prEvent) {
				DBGLOG(NIC, ERROR,
				       "Allocate prEvent failed!\n");
				return;
			}

			legacy = (struct EVENT_RX_ADDBA *)prEvent->aucBuffer;

			legacy->ucStaRecIdx =
				secGetStaIdxByWlanIdx(ad, ba->u2WlanIdx);
			legacy->ucDialogToken = ba->ucDialogToken;
			legacy->u2BATimeoutValue = ba->u2BATimeoutValue;
			legacy->u2BAStartSeqCtrl = ba->u2BAStartSeqCtrl;
			legacy->u2BAParameterSet =
				(ba->ucTid << BA_PARAM_SET_TID_MASK_OFFSET) |
				(ba->u2WinSize <<
					BA_PARAM_SET_BUFFER_SIZE_MASK_OFFSET);

			qmHandleEventRxAddBa(ad, prEvent);

			kalMemFree(prEvent, VIR_MEM_TYPE,
				sizeof(struct WIFI_EVENT) +
				sizeof(struct EVENT_RX_ADDBA));
		}
			break;
		case UNI_EVENT_BA_OFFLOAD_TAG_RX_DELBA:{
			struct UNI_EVENT_RX_DELBA *ba =
				(struct UNI_EVENT_RX_DELBA *) tag;
			struct EVENT_RX_DELBA *legacy;
			struct WIFI_EVENT *prEvent;

			prEvent = (struct WIFI_EVENT *)kalMemAlloc(
					sizeof(struct WIFI_EVENT) +
					sizeof(struct EVENT_RX_DELBA),
					VIR_MEM_TYPE);
			if (!prEvent) {
				DBGLOG(NIC, ERROR,
				       "Allocate prEvent failed!\n");
				return;
			}

			legacy = (struct EVENT_RX_DELBA *)prEvent->aucBuffer;

			legacy->ucStaRecIdx =
				secGetStaIdxByWlanIdx(ad, ba->u2WlanIdx);
			legacy->ucTid = ba->ucTid;
			qmHandleEventRxDelBa(ad, prEvent);

			kalMemFree(prEvent, VIR_MEM_TYPE,
				sizeof(struct WIFI_EVENT) +
				sizeof(struct EVENT_RX_DELBA));
		}
			break;
		case UNI_EVENT_BA_OFFLOAD_TAG_TX_ADDBA:{
			struct UNI_EVENT_TX_ADDBA *ba =
				(struct UNI_EVENT_TX_ADDBA *) tag;
			struct EVENT_TX_ADDBA *legacy;
			struct WIFI_EVENT *prEvent;

			prEvent = (struct WIFI_EVENT *)kalMemAlloc(
					sizeof(struct WIFI_EVENT) +
					sizeof(struct EVENT_TX_ADDBA),
					VIR_MEM_TYPE);
			if (!prEvent) {
				DBGLOG(NIC, ERROR,
				       "Allocate prEvent failed!\n");
				return;
			}

			legacy = (struct EVENT_TX_ADDBA *)prEvent->aucBuffer;

			legacy->ucStaRecIdx =
				secGetStaIdxByWlanIdx(ad, ba->u2WlanIdx);
			legacy->ucTid = ba->ucTid;
			/* WARN: ba1024 win size is truncated, it's okay
			 * now because qmHandleEventTxAddBa doesn't use it
			 */
			legacy->ucWinSize = ba->u2WinSize;
			legacy->ucAmsduEnBitmap = ba->ucAmsduEnBitmap;
			legacy->u2SSN = ba->u2SSN;
			legacy->ucMaxMpduCount = ba->ucMaxMpduCount;
			legacy->u4MaxMpduLen = ba->u4MaxMpduLen;
			legacy->u4MinMpduLen = ba->u4MinMpduLen;

			qmHandleEventTxAddBa(ad, prEvent);

			kalMemFree(prEvent, VIR_MEM_TYPE,
				sizeof(struct WIFI_EVENT) +
				sizeof(struct EVENT_TX_ADDBA));
		}
			break;
		default:
			fail_cnt++;
			ASSERT(fail_cnt < MAX_UNI_EVENT_FAIL_TAG_COUNT)
			DBGLOG(NIC, WARN, "invalid tag = %d\n", TAG_ID(tag));
			break;
		}
	}
}

void nicUniEventSleepNotify(struct ADAPTER *ad, struct WIFI_UNI_EVENT *evt)
{
	int32_t tags_len;
	uint8_t *tag;
	uint16_t offset = 0;
	uint32_t fixed_len = sizeof(struct UNI_EVENT_SLEEP_NOTIFY);
	uint32_t data_len = GET_UNI_EVENT_DATA_LEN(evt);
	uint8_t *data = GET_UNI_EVENT_DATA(evt);
	uint32_t fail_cnt = 0;

	tags_len = data_len - fixed_len;
	tag = data + fixed_len;
	TAG_FOR_EACH(tag, tags_len, offset) {
		DBGLOG(NIC, TRACE, "Tag(%d, %d)\n", TAG_ID(tag), TAG_LEN(tag));

		switch (TAG_ID(tag)) {
		case UNI_EVENT_SLEEP_NOTYFY_TAG_SLEEP_INFO:{
			struct UNI_EVENT_SLEEP_INFO *info =
				(struct UNI_EVENT_SLEEP_INFO *) tag;
			struct EVENT_SLEEPY_INFO *legacy;
			struct WIFI_EVENT *prEvent;

			prEvent = (struct WIFI_EVENT *)kalMemAlloc(
					sizeof(struct WIFI_EVENT) +
					sizeof(struct EVENT_SLEEPY_INFO),
					VIR_MEM_TYPE);
			if (!prEvent) {
				DBGLOG(NIC, ERROR,
				       "Allocate prEvent failed!\n");
				return;
			}

			legacy = (struct EVENT_SLEEPY_INFO *)prEvent->aucBuffer;

			legacy->ucSleepyState = info->ucSleepyState;

			nicEventSleepyNotify(ad, prEvent);

			kalMemFree(prEvent, VIR_MEM_TYPE,
				sizeof(struct WIFI_EVENT) +
				sizeof(struct EVENT_SLEEPY_INFO));
		}
			break;
		default:
			fail_cnt++;
			ASSERT(fail_cnt < MAX_UNI_EVENT_FAIL_TAG_COUNT)
			DBGLOG(NIC, WARN, "invalid tag = %d\n", TAG_ID(tag));
			break;
		}
	}
}

void nicUniEventBeaconTimeout(struct ADAPTER *ad, struct WIFI_UNI_EVENT *evt)
{
	int32_t tags_len;
	uint8_t *tag;
	uint16_t offset = 0;
	uint32_t fixed_len = sizeof(struct UNI_EVENT_BEACON_TIMEOUT);
	uint32_t data_len = GET_UNI_EVENT_DATA_LEN(evt);
	uint8_t *data = GET_UNI_EVENT_DATA(evt);
	uint32_t fail_cnt = 0;
	struct UNI_EVENT_BEACON_TIMEOUT *timeout;
	struct EVENT_BSS_BEACON_TIMEOUT *legacy;
	struct WIFI_EVENT *prEvent;

	prEvent = (struct WIFI_EVENT *)kalMemAlloc(
			sizeof(struct WIFI_EVENT) +
			sizeof(struct EVENT_BSS_BEACON_TIMEOUT),
			VIR_MEM_TYPE);
	if (!prEvent) {
		DBGLOG(NIC, ERROR, "Allocate prEvent failed!\n");
		return;
	}

	legacy = (struct EVENT_BSS_BEACON_TIMEOUT *)prEvent->aucBuffer;

	timeout = (struct UNI_EVENT_BEACON_TIMEOUT *) data;
	legacy->ucBssIndex = timeout->ucBssIndex;

	tags_len = data_len - fixed_len;
	tag = data + fixed_len;
	TAG_FOR_EACH(tag, tags_len, offset) {
		DBGLOG(NIC, TRACE, "Tag(%d, %d)\n", TAG_ID(tag), TAG_LEN(tag));

		switch (TAG_ID(tag)) {
		case UNI_EVENT_BEACON_TIMEOUT_TAG_INFO:{
			struct UNI_EVENT_BEACON_TIMEOUT_INFO *info =
				(struct UNI_EVENT_BEACON_TIMEOUT_INFO *) tag;

			legacy->ucReasonCode = info->ucReasonCode;
			nicEventBeaconTimeout(ad, prEvent);

		}
			break;
		default:
			fail_cnt++;
			ASSERT(fail_cnt < MAX_UNI_EVENT_FAIL_TAG_COUNT)
			DBGLOG(NIC, WARN, "invalid tag = %d\n", TAG_ID(tag));
			break;
		}
	}
	kalMemFree(prEvent, VIR_MEM_TYPE,
		sizeof(struct WIFI_EVENT) +
		sizeof(struct EVENT_BSS_BEACON_TIMEOUT));
}

void nicUniEventUpdateCoex(struct ADAPTER *ad, struct WIFI_UNI_EVENT *evt)
{
	int32_t tags_len;
	uint8_t *tag;
	uint16_t offset = 0;
	uint32_t fixed_len = sizeof(struct UNI_EVENT_UPDATE_COEX);
	uint32_t data_len = GET_UNI_EVENT_DATA_LEN(evt);
	uint8_t *data = GET_UNI_EVENT_DATA(evt);
	uint32_t fail_cnt = 0;
	uint8_t i;

	tags_len = data_len - fixed_len;
	tag = data + fixed_len;
	TAG_FOR_EACH(tag, tags_len, offset) {
		DBGLOG(NIC, TRACE, "Tag(%d, %d)\n", TAG_ID(tag), TAG_LEN(tag));

		switch (TAG_ID(tag)) {
		case UNI_EVENT_UPDATE_COEX_TAG_PHYRATE:{
			struct UNI_EVENT_UPDATE_COEX_PHYRATE *phyrate =
				(struct UNI_EVENT_UPDATE_COEX_PHYRATE *) tag;
			struct EVENT_UPDATE_COEX_PHYRATE *legacy;
			struct WIFI_EVENT *prEvent;

			prEvent = (struct WIFI_EVENT *)kalMemAlloc(
				    sizeof(struct WIFI_EVENT) +
				    sizeof(struct EVENT_UPDATE_COEX_PHYRATE),
				    VIR_MEM_TYPE);
			if (!prEvent) {
				DBGLOG(NIC, ERROR,
				       "Allocate prEvent failed!\n");
				return;
			}

			legacy = (struct EVENT_UPDATE_COEX_PHYRATE *)
				prEvent->aucBuffer;

			for (i = 0; i < MAX_BSSID_NUM + 1; i++) {
				legacy->au4PhyRateLimit[i] =
					phyrate->au4PhyRateLimit[i];
			}
			nicEventUpdateCoexPhyrate(ad, prEvent);

			kalMemFree(prEvent, VIR_MEM_TYPE,
				sizeof(struct WIFI_EVENT) +
				sizeof(struct EVENT_UPDATE_COEX_PHYRATE));
		}
			break;
		default:
			fail_cnt++;
			ASSERT(fail_cnt < MAX_UNI_EVENT_FAIL_TAG_COUNT)
			DBGLOG(NIC, WARN, "invalid tag = %d\n", TAG_ID(tag));
			break;
		}
	}
}

void nicUniEventIdc(struct ADAPTER *ad, struct WIFI_UNI_EVENT *evt)
{
	int32_t tags_len;
	uint8_t *tag;
	uint16_t offset = 0;
	uint32_t fixed_len = sizeof(struct UNI_EVENT_IDC);
	uint32_t data_len = GET_UNI_EVENT_DATA_LEN(evt);
	uint8_t *data = GET_UNI_EVENT_DATA(evt);
	uint32_t fail_cnt = 0;

	tags_len = data_len - fixed_len;
	tag = data + fixed_len;
	TAG_FOR_EACH(tag, tags_len, offset) {
		DBGLOG(NIC, TRACE, "Tag(%d, %d)\n", TAG_ID(tag), TAG_LEN(tag));

		switch (TAG_ID(tag)) {
		case UNI_EVENT_IDC_TAG_MD_SAFE_CHN:{
			struct UNI_EVENT_MD_SAFE_CHN *chn =
				(struct UNI_EVENT_MD_SAFE_CHN *) tag;
			struct EVENT_LTE_SAFE_CHN *legacy;
			struct WIFI_EVENT *prEvent;

			prEvent = (struct WIFI_EVENT *)kalMemAlloc(
					sizeof(struct WIFI_EVENT) +
					sizeof(struct EVENT_LTE_SAFE_CHN),
					VIR_MEM_TYPE);
			if (!prEvent) {
				DBGLOG(NIC, ERROR,
				       "Allocate prEvent failed!\n");
				return;
			}

			legacy = (struct EVENT_LTE_SAFE_CHN *)
				prEvent->aucBuffer;

			legacy->ucVersion = chn->ucVersion;
			legacy->u4Flags = chn->u4Flags;
			kalMemCopy(legacy->rLteSafeChn.au4SafeChannelBitmask,
				chn->u4SafeChannelBitmask,
				sizeof(uint32_t) * ENUM_SAFE_CH_MASK_MAX_NUM);

#if CFG_SUPPORT_IDC_CH_SWITCH
			cnmIdcDetectHandler(ad, prEvent);
#endif

			kalMemFree(prEvent, VIR_MEM_TYPE,
				sizeof(struct WIFI_EVENT) +
				sizeof(struct EVENT_LTE_SAFE_CHN));
		}
			break;
		case UNI_EVENT_IDC_TAG_CCCI_MSG:
			break;
		default:
			fail_cnt++;
			ASSERT(fail_cnt < MAX_UNI_EVENT_FAIL_TAG_COUNT)
			DBGLOG(NIC, WARN, "invalid tag = %d\n", TAG_ID(tag));
			break;
		}
	}
}

void nicUniEventBssIsAbsence(struct ADAPTER *ad, struct WIFI_UNI_EVENT *evt)
{
	int32_t tags_len;
	uint8_t *tag;
	uint16_t offset = 0;
	uint32_t fixed_len = sizeof(struct UNI_EVENT_BSS_IS_ABSENCE);
	uint32_t data_len = GET_UNI_EVENT_DATA_LEN(evt);
	uint8_t *data = GET_UNI_EVENT_DATA(evt);
	uint32_t fail_cnt = 0;
	struct UNI_EVENT_BSS_IS_ABSENCE *absence;

	absence = (struct UNI_EVENT_BSS_IS_ABSENCE *) data;

	tags_len = data_len - fixed_len;
	tag = data + fixed_len;
	TAG_FOR_EACH(tag, tags_len, offset) {
		DBGLOG(NIC, TRACE, "Tag(%d, %d)\n", TAG_ID(tag), TAG_LEN(tag));

		switch (TAG_ID(tag)) {
		case UNI_EVENT_BSS_IS_ABSENCE_TAG_INFO: {
			struct UNI_EVENT_BSS_IS_ABSENCE_INFO *info =
				(struct UNI_EVENT_BSS_IS_ABSENCE_INFO *) tag;
			struct EVENT_BSS_ABSENCE_PRESENCE *legacy;
			struct WIFI_EVENT *prEvent;

			prEvent = (struct WIFI_EVENT *)kalMemAlloc(
				    sizeof(struct WIFI_EVENT) +
				    sizeof(struct EVENT_BSS_ABSENCE_PRESENCE),
				    VIR_MEM_TYPE);
			if (!prEvent) {
				DBGLOG(NIC, ERROR,
				       "Allocate prEvent failed!\n");
				return;
			}

			legacy = (struct EVENT_BSS_ABSENCE_PRESENCE *)
				prEvent->aucBuffer;

			legacy->ucBssIndex = absence->ucBssIndex;
			legacy->ucIsAbsent = info->ucIsAbsent;
			legacy->ucBssFreeQuota = info->ucBssFreeQuota;

			qmHandleEventBssAbsencePresence(ad, prEvent);

			kalMemFree(prEvent, VIR_MEM_TYPE,
				sizeof(struct WIFI_EVENT) +
				sizeof(struct EVENT_BSS_ABSENCE_PRESENCE));
		}
			break;
		default:
			fail_cnt++;
			ASSERT(fail_cnt < MAX_UNI_EVENT_FAIL_TAG_COUNT)
			DBGLOG(NIC, WARN, "invalid tag = %d\n", TAG_ID(tag));
			break;
		}
	}
}

void nicUniEventPsSync(struct ADAPTER *ad, struct WIFI_UNI_EVENT *evt)
{
	int32_t tags_len;
	uint8_t *tag;
	uint16_t offset = 0;
	uint32_t fixed_len = sizeof(struct UNI_EVENT_PS_SYNC);
	uint32_t data_len = GET_UNI_EVENT_DATA_LEN(evt);
	uint8_t *data = GET_UNI_EVENT_DATA(evt);
	uint32_t fail_cnt = 0;

	tags_len = data_len - fixed_len;
	tag = data + fixed_len;
	TAG_FOR_EACH(tag, tags_len, offset) {
		DBGLOG(NIC, TRACE, "Tag(%d, %d)\n", TAG_ID(tag), TAG_LEN(tag));

		switch (TAG_ID(tag)) {
		case UNI_EVENT_PS_SYNC_TAG_CLIENT_PS_INFO: {
			struct UNI_EVENT_CLIENT_PS_INFO *ps =
				(struct UNI_EVENT_CLIENT_PS_INFO *) tag;
			struct EVENT_STA_CHANGE_PS_MODE *legacy;
			struct WIFI_EVENT *prEvent;

			prEvent = (struct WIFI_EVENT *)kalMemAlloc(
					sizeof(struct WIFI_EVENT) +
					sizeof(struct EVENT_STA_CHANGE_PS_MODE),
					VIR_MEM_TYPE);
			if (!prEvent) {
				DBGLOG(NIC, ERROR,
				       "Allocate prEvent failed!\n");
				return;
			}

			legacy = (struct EVENT_STA_CHANGE_PS_MODE *)
				prEvent->aucBuffer;

			legacy->ucStaRecIdx = ps->ucWtblIndex;
			legacy->ucIsInPs = ps->ucPsBit;
			legacy->ucFreeQuota = ps->ucBufferSize;

			qmHandleEventStaChangePsMode(ad, prEvent);

			kalMemFree(prEvent, VIR_MEM_TYPE,
				sizeof(struct WIFI_EVENT) +
				sizeof(struct EVENT_STA_CHANGE_PS_MODE));
		}
			break;
		default:
			fail_cnt++;
			ASSERT(fail_cnt < MAX_UNI_EVENT_FAIL_TAG_COUNT)
			DBGLOG(NIC, WARN, "invalid tag = %d\n", TAG_ID(tag));
			break;
		}
	}
}

void nicUniEventSap(struct ADAPTER *ad, struct WIFI_UNI_EVENT *evt)
{
	int32_t tags_len;
	uint8_t *tag;
	uint16_t offset = 0;
	uint32_t fixed_len = sizeof(struct UNI_EVENT_SAP);
	uint32_t data_len = GET_UNI_EVENT_DATA_LEN(evt);
	uint8_t *data = GET_UNI_EVENT_DATA(evt);
	uint32_t fail_cnt = 0;

	tags_len = data_len - fixed_len;
	tag = data + fixed_len;
	TAG_FOR_EACH(tag, tags_len, offset) {
		DBGLOG(NIC, TRACE, "Tag(%d, %d)\n", TAG_ID(tag), TAG_LEN(tag));

		switch (TAG_ID(tag)) {
		case UNI_EVENT_SAP_TAG_AGING_TIMEOUT:
			/* TODO: uni cmd */
			break;
		case UNI_EVENT_SAP_TAG_UPDATE_STA_FREE_QUOTA: {
			struct UNI_EVENT_UPDATE_STA_FREE_QUOTA *quota =
				(struct UNI_EVENT_UPDATE_STA_FREE_QUOTA *) tag;
			struct EVENT_STA_UPDATE_FREE_QUOTA *legacy;
			struct WIFI_EVENT *prEvent;

			prEvent = (struct WIFI_EVENT *)kalMemAlloc(
				    sizeof(struct WIFI_EVENT) +
				    sizeof(struct EVENT_STA_UPDATE_FREE_QUOTA),
				    VIR_MEM_TYPE);
			if (!prEvent) {
				DBGLOG(NIC, ERROR,
				       "Allocate prEvent failed!\n");
				return;
			}

			legacy = (struct EVENT_STA_UPDATE_FREE_QUOTA *)
				prEvent->aucBuffer;

			legacy->ucStaRecIdx = quota->u2StaRecIdx;
			legacy->ucUpdateMode = quota->ucUpdateMode;
			legacy->ucFreeQuota = quota->ucFreeQuota;

			qmHandleEventStaUpdateFreeQuota(ad, prEvent);

			kalMemFree(prEvent, VIR_MEM_TYPE,
				sizeof(struct WIFI_EVENT) +
				sizeof(struct EVENT_STA_UPDATE_FREE_QUOTA));
		}
			break;
		default:
			fail_cnt++;
			ASSERT(fail_cnt < MAX_UNI_EVENT_FAIL_TAG_COUNT)
			DBGLOG(NIC, WARN, "invalid tag = %d\n", TAG_ID(tag));
			break;
		}
	}
}

