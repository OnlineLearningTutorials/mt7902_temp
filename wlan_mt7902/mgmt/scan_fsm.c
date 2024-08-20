/* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause */
/*
 * Copyright (c) 2016 MediaTek Inc.
 */
/*
 * Id: //Department/DaVinci/BRANCHES/MT6620_WIFI_DRIVER_V2_3/mgmt/scan_fsm.c#2
 */

/*! \file   "scan_fsm.c"
 *    \brief  This file defines the state transition function for SCAN FSM.
 *
 *    The SCAN FSM is part of SCAN MODULE and responsible for performing basic
 *    SCAN behavior as metioned in IEEE 802.11 2007 11.1.3.1 & 11.1.3.2.
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

/*******************************************************************************
 *                           P R I V A T E   D A T A
 *******************************************************************************
 */
static uint8_t *apucDebugScanState[SCAN_STATE_NUM] = {
	(uint8_t *) DISP_STRING("IDLE"),
	(uint8_t *) DISP_STRING("SCANNING"),
};

/*******************************************************************************
 *                                 M A C R O S
 *******************************************************************************
 */
#if (CFG_SUPPORT_WIFI_6G == 1)
#define SCN_GET_EBAND_BY_CH_NUM(_ucChNum) \
	((_ucChNum <= HW_CHNL_NUM_MAX_2G4) ? BAND_2G4 : \
	(_ucChNum > HW_CHNL_NUM_MAX_5G) ? BAND_6G : \
	BAND_5G)
#else
#define SCN_GET_EBAND_BY_CH_NUM(_ucChNum) \
	((_ucChNum <= HW_CHNL_NUM_MAX_2G4) ? BAND_2G4 :	BAND_5G)
#endif


/*******************************************************************************
 *                   F U N C T I O N   D E C L A R A T I O N S
 *******************************************************************************
 */

/*******************************************************************************
 *                              F U N C T I O N S
 *******************************************************************************
 */
/*----------------------------------------------------------------------------*/
/*!
 * \brief
 *
 * \param[in]
 *
 * \return none
 */
/*----------------------------------------------------------------------------*/
void scnFsmSteps(IN struct ADAPTER *prAdapter,
	IN enum ENUM_SCAN_STATE eNextState)
{
	struct SCAN_INFO *prScanInfo;
	struct SCAN_PARAM *prScanParam;
	struct MSG_HDR *prMsgHdr;

	u_int8_t fgIsTransition = (u_int8_t) FALSE;

	prScanInfo = &(prAdapter->rWifiVar.rScanInfo);
	prScanParam = &prScanInfo->rScanParam;

	do {
		if ((uint32_t)prScanInfo->eCurrentState < SCAN_STATE_NUM &&
			(uint32_t)eNextState < SCAN_STATE_NUM) {
			log_dbg(SCN, STATE, "[SCAN]TRANSITION: [%s] -> [%s]\n",
			apucDebugScanState[
				(uint32_t) prScanInfo->eCurrentState],
			apucDebugScanState[
				(uint32_t) eNextState]);
		}

		/* NOTE(Kevin): This is the only place to change the
		 * eCurrentState(except initial)
		 */
		prScanInfo->eCurrentState = eNextState;

		fgIsTransition = (u_int8_t) FALSE;

		switch (prScanInfo->eCurrentState) {
		case SCAN_STATE_IDLE:
			prScanParam->fg6gOobRnrParseEn = FALSE;
			/* check for pending scanning requests */
			if (!LINK_IS_EMPTY(&(prScanInfo->rPendingMsgList))) {
				/* load next message from pending list as
				 * scan parameters
				 */
				LINK_REMOVE_HEAD(&(prScanInfo->rPendingMsgList),
					prMsgHdr, struct MSG_HDR *);

#define __MSG_ID__ prMsgHdr->eMsgId
				if (__MSG_ID__ == MID_AIS_SCN_SCAN_REQ
					|| __MSG_ID__ == MID_BOW_SCN_SCAN_REQ
					|| __MSG_ID__ == MID_P2P_SCN_SCAN_REQ
					|| __MSG_ID__ == MID_RLM_SCN_SCAN_REQ) {
					scnFsmHandleScanMsg(prAdapter,
						(struct MSG_SCN_SCAN_REQ *)
						 prMsgHdr);

					eNextState = SCAN_STATE_SCANNING;
					fgIsTransition = TRUE;
				} else if (__MSG_ID__ == MID_AIS_SCN_SCAN_REQ_V2
					|| __MSG_ID__ == MID_BOW_SCN_SCAN_REQ_V2
					|| __MSG_ID__ == MID_P2P_SCN_SCAN_REQ_V2
					|| __MSG_ID__ == MID_RLM_SCN_SCAN_REQ_V2
					) {
					scnFsmHandleScanMsgV2(prAdapter,
						(struct MSG_SCN_SCAN_REQ_V2 *)
						 prMsgHdr);

					eNextState = SCAN_STATE_SCANNING;
					fgIsTransition = TRUE;
				} else {
					/* should not happen */
					ASSERT(0);
				}
#undef __MSG_ID__

				/* switch to next state */
				cnmMemFree(prAdapter, prMsgHdr);
			}
			break;

		case SCAN_STATE_SCANNING:
			/* Support AP Selection */
			prScanInfo->u4ScanUpdateIdx++;
			if (prScanParam->fgIsScanV2 == FALSE)
				scnSendScanReq(prAdapter);
			else
				scnSendScanReqV2(prAdapter);
			break;

		default:
			ASSERT(0);
			break;

		}
	} while (fgIsTransition);
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief        Generate CMD_ID_SCAN_REQ command
 *
 * Because CMD_ID_SCAN_REQ is deprecated,
 * wrap this command to CMD_ID_SCAN_REQ_V2
 *
 * \param[in] prAdapter   adapter
 *
 * \return none
 */
/*----------------------------------------------------------------------------*/
void scnSendScanReq(IN struct ADAPTER *prAdapter)
{
	log_dbg(SCN, WARN,
		"CMD_ID_SCAN_REQ is deprecated, use CMD_ID_SCAN_REQ_V2\n");
	scnSendScanReqV2(prAdapter);
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief        Generate CMD_ID_SCAN_REQ_V2 command
 *
 * \param[in]
 *
 * \return none
 */
/*----------------------------------------------------------------------------*/
void scnSendScanReqV2(IN struct ADAPTER *prAdapter)
{
	struct SCAN_INFO *prScanInfo;
	struct SCAN_PARAM *prScanParam;
	/* CMD_SCAN_REQ_V2 rCmdScanReq; */
	struct CMD_SCAN_REQ_V2 *prCmdScanReq;
	uint32_t i;

	ASSERT(prAdapter);

	prScanInfo = &(prAdapter->rWifiVar.rScanInfo);
	prScanParam = &(prScanInfo->rScanParam);

	prCmdScanReq = kalMemAlloc(
		sizeof(struct CMD_SCAN_REQ_V2), VIR_MEM_TYPE);
	if (!prCmdScanReq) {
		log_dbg(SCN, ERROR, "alloc CmdScanReq V2 fail\n");
		return;
	}
	/* send command packet for scan */
	kalMemZero(prCmdScanReq, sizeof(struct CMD_SCAN_REQ_V2));
	/* Modify channelList number from 32 to 54 */
	if (prScanParam->ucScnFuncMask & ENUM_SCN_USE_PADDING_AS_BSSID) {
		kalMemCopy(prCmdScanReq->aucExtBSSID,
			&prScanParam->aucBSSID[0][0],
			SCN_SSID_MAX_NUM*MAC_ADDR_LEN);
		prCmdScanReq->ucScnFuncMask |= ENUM_SCN_USE_PADDING_AS_BSSID;
		DBGLOG(SCN, INFO,
			"[56_2] Bssid! "MACSTR"\n",
		       MAC2STR(prCmdScanReq->aucExtBSSID[0]));
	} else {
		COPY_MAC_ADDR(prCmdScanReq->aucBSSID,
		&prScanParam->aucBSSID[0][0]);
	}
	if (!EQUAL_MAC_ADDR(prCmdScanReq->aucBSSID, "\xff\xff\xff\xff\xff\xff"))
		DBGLOG(SCN, INFO, "Include BSSID " MACSTR " in probe request\n",
		       MAC2STR(prCmdScanReq->aucBSSID));

	prCmdScanReq->ucSeqNum = prScanParam->ucSeqNum;
	prCmdScanReq->ucBssIndex = prScanParam->ucBssIndex;
	prCmdScanReq->ucScanType = (uint8_t) prScanParam->eScanType;
	prCmdScanReq->ucSSIDType = prScanParam->ucSSIDType;
	prCmdScanReq->auVersion[0] = 1;
	/* for 6G OOB scan */
	kalMemCopy(prCmdScanReq->ucBssidMatchCh, prScanParam->ucBssidMatchCh,
			CFG_SCAN_SSID_MAX_NUM);
	kalMemCopy(prCmdScanReq->ucBssidMatchSsidInd,
		prScanParam->ucBssidMatchSsidInd, CFG_SCAN_SSID_MAX_NUM);

	if (kalIsValidMacAddr(prScanParam->aucRandomMac)) {
		prCmdScanReq->ucScnFuncMask |= (ENUM_SCN_RANDOM_MAC_EN |
						ENUM_SCN_RANDOM_SN_EN);
		kalMemCopy(prCmdScanReq->aucRandomMac,
			prScanParam->aucRandomMac, MAC_ADDR_LEN);
	}
	if (prAdapter->rWifiVar.eDbdcMode == ENUM_DBDC_MODE_DISABLED)
		prCmdScanReq->ucScnFuncMask |= ENUM_SCN_DBDC_SCAN_DIS;

	/* Set SSID to scan request */
	if (prScanParam->ucSSIDNum <= SCAN_CMD_SSID_NUM) {
		prCmdScanReq->ucSSIDNum = prScanParam->ucSSIDNum;
		prCmdScanReq->ucSSIDExtNum = 0;
	} else if (prScanParam->ucSSIDNum <= CFG_SCAN_SSID_MAX_NUM) {
		prCmdScanReq->ucSSIDNum = SCAN_CMD_SSID_NUM;
		prCmdScanReq->ucSSIDExtNum = prScanParam->ucSSIDNum
			- SCAN_CMD_SSID_NUM;
	} else {
		log_dbg(SCN, WARN, "Too many SSID %u\n",
					prScanParam->ucSSIDNum);
		prCmdScanReq->ucSSIDNum = SCAN_CMD_SSID_NUM;
		prCmdScanReq->ucSSIDExtNum = SCAN_CMD_EXT_SSID_NUM;
	}

	for (i = 0; i < prCmdScanReq->ucSSIDNum; i++) {
		COPY_SSID(prCmdScanReq->arSSID[i].aucSsid,
			prCmdScanReq->arSSID[i].u4SsidLen,
			prScanParam->aucSpecifiedSSID[i],
			prScanParam->ucSpecifiedSSIDLen[i]);
		log_dbg(SCN, TRACE,
			"Ssid=%s, SsidLen=%d\n",
			prCmdScanReq->arSSID[i].aucSsid,
			prCmdScanReq->arSSID[i].u4SsidLen);
	}
	for (i = 0; i < prCmdScanReq->ucSSIDExtNum; i++) {
		COPY_SSID(prCmdScanReq->arSSIDExtend[i].aucSsid,
			prCmdScanReq->arSSIDExtend[i].u4SsidLen,
			prScanParam->aucSpecifiedSSID
			[prCmdScanReq->ucSSIDNum+i],
			prScanParam->ucSpecifiedSSIDLen
			[prCmdScanReq->ucSSIDNum+i]);
		log_dbg(SCN, TRACE,
			"Ssid=%s, SsidLen=%d\n",
			prCmdScanReq->arSSIDExtend[i].aucSsid,
			prCmdScanReq->arSSIDExtend[i].u4SsidLen);
	}

	prCmdScanReq->u2ProbeDelayTime
		= (uint8_t) prScanParam->u2ProbeDelayTime;
	prCmdScanReq->ucChannelType
		= (uint8_t) prScanParam->eScanChannel;

	/* Set channel info to scan request */
	if (prScanParam->eScanChannel == SCAN_CHANNEL_SPECIFIED) {
		if (prScanParam->ucChannelListNum <= SCAN_CMD_CHNL_NUM) {
			prCmdScanReq->ucChannelListNum =
				prScanParam->ucChannelListNum;
			prCmdScanReq->ucChannelListExtNum = 0;
		} else if (prScanParam->ucChannelListNum <=
				SCAN_CMD_CHNL_NUM + SCAN_CMD_EXT_CHNL_NUM) {
			prCmdScanReq->ucChannelListNum =
				SCAN_CMD_CHNL_NUM;
			prCmdScanReq->ucChannelListExtNum =
				prScanParam->ucChannelListNum -
				SCAN_CMD_CHNL_NUM;
		} else {
			log_dbg(SCN, WARN, "Too many Channel %u\n",
				prScanParam->ucChannelListNum);
			prCmdScanReq->ucChannelListNum = 0;
			prCmdScanReq->ucChannelListExtNum = 0;
			prCmdScanReq->ucChannelType = SCAN_CHANNEL_FULL;
		}

		for (i = 0; i < prCmdScanReq->ucChannelListNum; i++) {
			prCmdScanReq->arChannelList[i].ucBand
				= (uint8_t) prScanParam->arChnlInfoList[i]
					.eBand;

			prCmdScanReq->arChannelList[i].ucChannelNum
				= (uint8_t) prScanParam->arChnlInfoList[i]
					.ucChannelNum;
		}
		for (i = 0; i < prCmdScanReq->ucChannelListExtNum; i++) {
			prCmdScanReq->arChannelListExtend[i].ucBand
				= (uint8_t)prScanParam
					->arChnlInfoList
					[prCmdScanReq->ucChannelListNum+i]
					.eBand;

			prCmdScanReq->arChannelListExtend[i].ucChannelNum
				= (uint8_t) prScanParam
					->arChnlInfoList
					[prCmdScanReq->ucChannelListNum+i]
					.ucChannelNum;
		}
	}

	prCmdScanReq->u2ChannelDwellTime = prScanParam->u2ChannelDwellTime;
	prCmdScanReq->u2ChannelMinDwellTime =
		prScanParam->u2ChannelMinDwellTime;
	prCmdScanReq->u2TimeoutValue = prScanParam->u2TimeoutValue;

	if (prScanParam->u2IELen <= MAX_IE_LENGTH)
		prCmdScanReq->u2IELen = prScanParam->u2IELen;
	else
		prCmdScanReq->u2IELen = MAX_IE_LENGTH;

	if (prScanParam->u2IELen)
		kalMemCopy(prCmdScanReq->aucIE, prScanParam->aucIE,
			sizeof(uint8_t) * prCmdScanReq->u2IELen);

	scanLogCacheFlushAll(&(prScanInfo->rScanLogCache),
		LOG_SCAN_REQ_D2F, SCAN_LOG_MSG_MAX_LEN);
	scanReqLog(prCmdScanReq);

	wlanSendSetQueryCmd(prAdapter,
		CMD_ID_SCAN_REQ_V2,
		TRUE,
		FALSE,
		FALSE,
		NULL,
		NULL,
		sizeof(struct CMD_SCAN_REQ_V2),
		(uint8_t *)prCmdScanReq, NULL, 0);
	log_dbg(SCN, TRACE, "Send %zu bytes\n", sizeof(struct CMD_SCAN_REQ_V2));


	kalMemFree(prCmdScanReq, VIR_MEM_TYPE, sizeof(struct CMD_SCAN_REQ_V2));

}

/*----------------------------------------------------------------------------*/
/*!
 * \brief
 *
 * \param[in]
 *
 * \return none
 */
/*----------------------------------------------------------------------------*/
void scnFsmMsgStart(IN struct ADAPTER *prAdapter, IN struct MSG_HDR *prMsgHdr)
{
	struct SCAN_INFO *prScanInfo;
	struct SCAN_PARAM *prScanParam;

	ASSERT(prMsgHdr);

	prScanInfo = &(prAdapter->rWifiVar.rScanInfo);
	prScanParam = &prScanInfo->rScanParam;

	if (prScanInfo->eCurrentState == SCAN_STATE_IDLE) {
		if (prMsgHdr->eMsgId == MID_AIS_SCN_SCAN_REQ
			|| prMsgHdr->eMsgId == MID_BOW_SCN_SCAN_REQ
			|| prMsgHdr->eMsgId == MID_P2P_SCN_SCAN_REQ
			|| prMsgHdr->eMsgId == MID_RLM_SCN_SCAN_REQ) {
			scnFsmHandleScanMsg(prAdapter,
				(struct MSG_SCN_SCAN_REQ *) prMsgHdr);
		} else if (prMsgHdr->eMsgId == MID_AIS_SCN_SCAN_REQ_V2
			|| prMsgHdr->eMsgId == MID_BOW_SCN_SCAN_REQ_V2
			|| prMsgHdr->eMsgId == MID_P2P_SCN_SCAN_REQ_V2
			|| prMsgHdr->eMsgId == MID_RLM_SCN_SCAN_REQ_V2) {
			scnFsmHandleScanMsgV2(prAdapter,
				(struct MSG_SCN_SCAN_REQ_V2 *) prMsgHdr);
		} else {
			/* should not deliver to this function */
			ASSERT(0);
		}

		cnmMemFree(prAdapter, prMsgHdr);
		scnFsmSteps(prAdapter, SCAN_STATE_SCANNING);
	} else {
		LINK_INSERT_TAIL(&prScanInfo->rPendingMsgList,
			&prMsgHdr->rLinkEntry);
	}
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief
 *
 * \param[in]
 *
 * \return none
 */
/*----------------------------------------------------------------------------*/
void scnFsmMsgAbort(IN struct ADAPTER *prAdapter, IN struct MSG_HDR *prMsgHdr)
{
	struct MSG_SCN_SCAN_CANCEL *prScanCancel;
	struct SCAN_INFO *prScanInfo;
	struct SCAN_PARAM *prScanParam;
	struct CMD_SCAN_CANCEL rCmdScanCancel;

	ASSERT(prMsgHdr);

	prScanCancel = (struct MSG_SCN_SCAN_CANCEL *) prMsgHdr;
	prScanInfo = &(prAdapter->rWifiVar.rScanInfo);
	prScanParam = &prScanInfo->rScanParam;

	memset(&rCmdScanCancel, 0, sizeof(struct CMD_SCAN_CANCEL));

	/* TODO: remove condition (prScanParam->fgIsObssScan == TRUE) after remove fgIsObssScan */
	if (prScanInfo->eCurrentState != SCAN_STATE_IDLE) {
		if (((prScanCancel->ucSeqNum == prScanParam->ucSeqNum) ||
			(prScanParam->fgIsObssScan == TRUE)) &&
			prScanCancel->ucBssIndex == prScanParam->ucBssIndex) {
			enum ENUM_SCAN_STATUS eStatus = SCAN_STATUS_DONE;

			/* send cancel message to firmware domain */
			rCmdScanCancel.ucSeqNum = prScanParam->ucSeqNum;
			rCmdScanCancel.ucIsExtChannel
				= (uint8_t) prScanCancel->fgIsChannelExt;

			scanlog_dbg(LOG_SCAN_ABORT_REQ_D2F, INFO, "Scan Abort#%u to Q: isExtCh=%u",
				rCmdScanCancel.ucSeqNum,
				rCmdScanCancel.ucIsExtChannel);

			wlanSendSetQueryCmd(prAdapter,
				CMD_ID_SCAN_CANCEL,
				TRUE,
				FALSE,
				FALSE,
				NULL,
				NULL,
				sizeof(struct CMD_SCAN_CANCEL),
				(uint8_t *) &rCmdScanCancel,
				NULL,
				0);

			/* Full2Partial: ignore this statistics */
			if (prScanInfo->fgIsScanForFull2Partial) {
				prScanInfo->fgIsScanForFull2Partial = FALSE;
				prScanInfo->u4LastFullScanTime = 0;
				log_dbg(SCN, INFO,
					"Full2Partial: scan canceled(%u)\n",
					prScanParam->ucSeqNum);
			}

			eStatus = SCAN_STATUS_CANCELLED;
			scnFsmGenerateScanDoneMsg(prAdapter,
				prScanParam->ucSeqNum,
				prScanParam->ucBssIndex,
				eStatus);

			/* switch to next pending scan */
			scnFsmSteps(prAdapter, SCAN_STATE_IDLE);
		} else {
			scnFsmRemovePendingMsg(prAdapter,
				prScanCancel->ucSeqNum,
				prScanCancel->ucBssIndex);
		}
	}

	cnmMemFree(prAdapter, prMsgHdr);
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief            Scan Message Parsing (Legacy)
 *
 * \param[in]
 *
 * \return none
 */
/*----------------------------------------------------------------------------*/
void scnFsmHandleScanMsg(IN struct ADAPTER *prAdapter,
	IN struct MSG_SCN_SCAN_REQ *prScanReqMsg)
{
	struct SCAN_INFO *prScanInfo;
	struct SCAN_PARAM *prScanParam;
	uint32_t i;

	ASSERT(prAdapter);
	ASSERT(prScanReqMsg);

	prScanInfo = &(prAdapter->rWifiVar.rScanInfo);
	prScanParam = &prScanInfo->rScanParam;

	kalMemZero(prScanParam, sizeof(*prScanParam));
	prScanParam->eScanType = prScanReqMsg->eScanType;
	prScanParam->ucBssIndex = prScanReqMsg->ucBssIndex;
	prScanParam->ucSSIDType = prScanReqMsg->ucSSIDType;
	if (prScanParam->ucSSIDType
		& (SCAN_REQ_SSID_SPECIFIED | SCAN_REQ_SSID_P2P_WILDCARD)) {
		prScanParam->ucSSIDNum = 1;

		COPY_SSID(prScanParam->aucSpecifiedSSID[0],
			prScanParam->ucSpecifiedSSIDLen[0],
			prScanReqMsg->aucSSID, prScanReqMsg->ucSSIDLength);

		/* reset SSID length to zero for rest array entries */
		for (i = 1; i < SCN_SSID_MAX_NUM; i++)
			prScanParam->ucSpecifiedSSIDLen[i] = 0;
	} else {
		prScanParam->ucSSIDNum = 0;

		for (i = 0; i < SCN_SSID_MAX_NUM; i++)
			prScanParam->ucSpecifiedSSIDLen[i] = 0;
	}

	prScanParam->u2ProbeDelayTime = 0;
	prScanParam->eScanChannel = prScanReqMsg->eScanChannel;
	if (prScanParam->eScanChannel == SCAN_CHANNEL_SPECIFIED) {
		if (prScanReqMsg->ucChannelListNum
			<= MAXIMUM_OPERATION_CHANNEL_LIST) {
			prScanParam->ucChannelListNum
				= prScanReqMsg->ucChannelListNum;
		} else {
			prScanParam->ucChannelListNum
				= MAXIMUM_OPERATION_CHANNEL_LIST;
		}

		kalMemCopy(prScanParam->arChnlInfoList,
			prScanReqMsg->arChnlInfoList,
			sizeof(struct RF_CHANNEL_INFO)
				* prScanParam->ucChannelListNum);
	}

	if (prScanReqMsg->u2IELen <= MAX_IE_LENGTH)
		prScanParam->u2IELen = prScanReqMsg->u2IELen;
	else
		prScanParam->u2IELen = MAX_IE_LENGTH;

	if (prScanParam->u2IELen) {
		kalMemCopy(prScanParam->aucIE,
			prScanReqMsg->aucIE, prScanParam->u2IELen);
	}

	prScanParam->u2ChannelDwellTime = prScanReqMsg->u2ChannelDwellTime;
	prScanParam->u2TimeoutValue = prScanReqMsg->u2TimeoutValue;
	prScanParam->ucSeqNum = prScanReqMsg->ucSeqNum;

	if (prScanReqMsg->rMsgHdr.eMsgId == MID_RLM_SCN_SCAN_REQ)
		prScanParam->fgIsObssScan = TRUE;
	else
		prScanParam->fgIsObssScan = FALSE;

	prScanParam->fgIsScanV2 = FALSE;
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief            Scan Message Parsing - V2 with multiple SSID support
 *
 * \param[in]
 *
 * \return none
 */
/*----------------------------------------------------------------------------*/
void scnFsmHandleScanMsgV2(IN struct ADAPTER *prAdapter,
	IN struct MSG_SCN_SCAN_REQ_V2 *prScanReqMsg)
{
	struct SCAN_INFO *prScanInfo;
	struct SCAN_PARAM *prScanParam;
	uint32_t i;

	ASSERT(prAdapter);
	ASSERT(prScanReqMsg);
	ASSERT(prScanReqMsg->ucSSIDNum <= SCN_SSID_MAX_NUM);

	prScanInfo = &(prAdapter->rWifiVar.rScanInfo);
	prScanParam = &prScanInfo->rScanParam;

	kalMemZero(prScanParam, sizeof(*prScanParam));
	prScanParam->eScanType = prScanReqMsg->eScanType;
	prScanParam->ucBssIndex = prScanReqMsg->ucBssIndex;
	prScanParam->ucSSIDType = prScanReqMsg->ucSSIDType;
	prScanParam->ucSSIDNum = prScanReqMsg->ucSSIDNum;
	prScanParam->ucScnFuncMask |= prScanReqMsg->ucScnFuncMask;
	kalMemCopy(prScanParam->aucRandomMac, prScanReqMsg->aucRandomMac,
		MAC_ADDR_LEN);
	/* for 6G OOB scan */
	kalMemCopy(prScanParam->ucBssidMatchCh, prScanReqMsg->ucBssidMatchCh,
			CFG_SCAN_SSID_MAX_NUM);
	kalMemCopy(prScanParam->ucBssidMatchSsidInd,
		prScanReqMsg->ucBssidMatchSsidInd, CFG_SCAN_SSID_MAX_NUM);
	prScanParam->fg6gOobRnrParseEn = prScanReqMsg->fg6gOobRnrParseEn;

	if ((prScanParam->ucSSIDType & SCAN_REQ_SSID_SPECIFIED_ONLY) &&
		((prScanReqMsg->ucScnFuncMask &
		ENUM_SCN_USE_PADDING_AS_BSSID) == 0)) {
		prScanParam->ucSSIDNum = 1;
		kalMemZero(prScanParam->ucSpecifiedSSIDLen,
			   sizeof(prScanParam->ucSpecifiedSSIDLen));
		COPY_SSID(prScanParam->aucSpecifiedSSID[0],
			  prScanParam->ucSpecifiedSSIDLen[0],
			  &prScanReqMsg->prSsid[0].aucSsid[0],
			  prScanReqMsg->prSsid[0].u4SsidLen);
	} else {
		for (i = 0; i < prScanReqMsg->ucSSIDNum; i++) {
			COPY_SSID(prScanParam->aucSpecifiedSSID[i],
				  prScanParam->ucSpecifiedSSIDLen[i],
				  prScanReqMsg->prSsid[i].aucSsid,
				  (uint8_t) prScanReqMsg->prSsid[i].u4SsidLen);
		}
	}

	prScanParam->u2ProbeDelayTime = prScanReqMsg->u2ProbeDelay;
	prScanParam->eScanChannel = prScanReqMsg->eScanChannel;
	if (prScanParam->eScanChannel == SCAN_CHANNEL_SPECIFIED) {
		if (prScanReqMsg->ucChannelListNum
			<= MAXIMUM_OPERATION_CHANNEL_LIST) {
			prScanParam->ucChannelListNum
				= prScanReqMsg->ucChannelListNum;
		} else {
			prScanParam->ucChannelListNum
				= MAXIMUM_OPERATION_CHANNEL_LIST;
		}

		kalMemCopy(prScanParam->arChnlInfoList,
			prScanReqMsg->arChnlInfoList,
			sizeof(struct RF_CHANNEL_INFO)
				* prScanParam->ucChannelListNum);
	}

	if (prScanReqMsg->u2IELen <= MAX_IE_LENGTH)
		prScanParam->u2IELen = prScanReqMsg->u2IELen;
	else
		prScanParam->u2IELen = MAX_IE_LENGTH;

	if (prScanParam->u2IELen) {
		kalMemCopy(prScanParam->aucIE,
			prScanReqMsg->aucIE, prScanParam->u2IELen);
	}

	prScanParam->u2ChannelDwellTime = prScanReqMsg->u2ChannelDwellTime;
	prScanParam->u2ChannelMinDwellTime =
		prScanReqMsg->u2ChannelMinDwellTime;
	prScanParam->u2TimeoutValue = prScanReqMsg->u2TimeoutValue;
	prScanParam->ucSeqNum = prScanReqMsg->ucSeqNum;

	if (prScanReqMsg->rMsgHdr.eMsgId == MID_RLM_SCN_SCAN_REQ_V2)
		prScanParam->fgIsObssScan = TRUE;
	else
		prScanParam->fgIsObssScan = FALSE;

	prScanParam->fgIsScanV2 = TRUE;

	kalMemCopy(&prScanParam->aucBSSID[0][0],
		&prScanReqMsg->aucExtBssid[0][0],
		SCN_SSID_MAX_NUM*MAC_ADDR_LEN);
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief            Remove pending scan request
 *
 * \param[in]
 *
 * \return none
 */
/*----------------------------------------------------------------------------*/
void scnFsmRemovePendingMsg(IN struct ADAPTER *prAdapter, IN uint8_t ucSeqNum,
	IN uint8_t ucBssIndex)
{
	struct SCAN_INFO *prScanInfo;
	struct SCAN_PARAM *prScanParam;
	struct MSG_HDR *prPendingMsgHdr = NULL;
	struct MSG_HDR *prPendingMsgHdrNext = NULL;
	struct MSG_HDR *prRemoveMsgHdr = NULL;
	struct LINK_ENTRY *prRemoveLinkEntry = NULL;
	u_int8_t fgIsRemovingScan = FALSE;

	ASSERT(prAdapter);

	prScanInfo = &(prAdapter->rWifiVar.rScanInfo);
	prScanParam = &prScanInfo->rScanParam;

	/* traverse through rPendingMsgList for removal */
	LINK_FOR_EACH_ENTRY_SAFE(prPendingMsgHdr,
		prPendingMsgHdrNext, &(prScanInfo->rPendingMsgList),
		rLinkEntry, struct MSG_HDR) {

#define __MSG_ID__ prPendingMsgHdr->eMsgId
		if (__MSG_ID__ == MID_AIS_SCN_SCAN_REQ
		    || __MSG_ID__ == MID_BOW_SCN_SCAN_REQ
		    || __MSG_ID__ == MID_P2P_SCN_SCAN_REQ
		    || __MSG_ID__ == MID_RLM_SCN_SCAN_REQ) {
			struct MSG_SCN_SCAN_REQ *prScanReqMsg
				= (struct MSG_SCN_SCAN_REQ *)
					prPendingMsgHdr;

			if (ucSeqNum == prScanReqMsg->ucSeqNum
				&& ucBssIndex == prScanReqMsg->ucBssIndex) {
				prRemoveLinkEntry
					= &(prScanReqMsg->rMsgHdr.rLinkEntry);
				prRemoveMsgHdr = prPendingMsgHdr;
				fgIsRemovingScan = TRUE;
			}
		} else if (__MSG_ID__ == MID_AIS_SCN_SCAN_REQ_V2
			   || __MSG_ID__ == MID_BOW_SCN_SCAN_REQ_V2
			   || __MSG_ID__ == MID_P2P_SCN_SCAN_REQ_V2
			   || __MSG_ID__ == MID_RLM_SCN_SCAN_REQ_V2) {
			struct MSG_SCN_SCAN_REQ_V2 *prScanReqMsgV2
				= (struct MSG_SCN_SCAN_REQ_V2 *)
					prPendingMsgHdr;

			if (ucSeqNum == prScanReqMsgV2->ucSeqNum
				&& ucBssIndex == prScanReqMsgV2->ucBssIndex) {
				prRemoveLinkEntry
					= &(prScanReqMsgV2->rMsgHdr.rLinkEntry);
				prRemoveMsgHdr = prPendingMsgHdr;
				fgIsRemovingScan = TRUE;
			}
		}
#undef __MSG_ID__

		if (prRemoveLinkEntry) {
			if (fgIsRemovingScan == TRUE) {
				/* generate scan-done event for caller */
				scnFsmGenerateScanDoneMsg(prAdapter, ucSeqNum,
					ucBssIndex, SCAN_STATUS_CANCELLED);
			}

			/* remove from pending list */
			LINK_REMOVE_KNOWN_ENTRY(&(prScanInfo->rPendingMsgList),
				prRemoveLinkEntry);
			cnmMemFree(prAdapter, prRemoveMsgHdr);

			break;
		}
	}
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief
 *
 * \param[in]
 *
 * \return none
 */
/*----------------------------------------------------------------------------*/
void scnEventScanDone(IN struct ADAPTER *prAdapter,
	IN struct EVENT_SCAN_DONE *prScanDone, u_int8_t fgIsNewVersion)
{
	struct SCAN_INFO *prScanInfo;
	struct SCAN_PARAM *prScanParam;
	uint32_t u4ChCnt = 0;

	prScanInfo = &(prAdapter->rWifiVar.rScanInfo);
	prScanParam = &prScanInfo->rScanParam;

	if (fgIsNewVersion) {
		scanlog_dbg(LOG_SCAN_DONE_F2D, INFO, "scnEventScanDone Version%u!size of ScanDone%zu,ucCompleteChanCount[%u],ucCurrentState%u, u4ScanDurBcnCnt[%u],Seq[%u]\n",
			prScanDone->ucScanDoneVersion,
			sizeof(struct EVENT_SCAN_DONE),
			prScanDone->ucCompleteChanCount,
			prScanDone->ucCurrentState,
			prScanDone->u4ScanDurBcnCnt,
			prScanDone->ucSeqNum);

		scanLogCacheFlushBSS(&(prScanInfo->rScanLogCache.rBSSListFW),
			LOG_SCAN_DONE_F2D, SCAN_LOG_MSG_MAX_LEN);

		if (prScanDone->ucCurrentState != FW_SCAN_STATE_SCAN_DONE) {
			log_dbg(SCN, INFO, "FW Scan timeout!generate ScanDone event at State%d complete chan count%d ucChannelListNum%d\n",
				prScanDone->ucCurrentState,
				prScanDone->ucCompleteChanCount,
				prScanParam->ucChannelListNum);

		} else {
			log_dbg(SCN, TRACE, " scnEventScanDone at FW_SCAN_STATE_SCAN_DONE state\n");
		}
	} else {
		scanlog_dbg(LOG_SCAN_DONE_F2D, INFO, "Old scnEventScanDone Version\n");
	}

	/* buffer empty channel information */
	if (prScanDone->ucSparseChannelValid) {
		int num = 0;
		char strbuf[SCN_SCAN_DONE_PRINT_BUFFER_LENGTH];

		prScanInfo->fgIsSparseChannelValid = TRUE;
		prScanInfo->rSparseChannel.eBand
			= (enum ENUM_BAND) prScanDone->rSparseChannel.ucBand;
		prScanInfo->rSparseChannel.ucChannelNum
			= prScanDone->rSparseChannel.ucChannelNum;
		num = prScanInfo->ucSparseChannelArrayValidNum
			= prScanDone->ucSparseChannelArrayValidNum;
		log_dbg(SCN, INFO, "Country Code = %c%c, Detected_Channel_Num = %d\n",
			((prAdapter->rWifiVar.u2CountryCode
				& 0xff00) >> 8),
			(prAdapter->rWifiVar.u2CountryCode
				& 0x00ff), num);
#if (CFG_SUPPORT_WIFI_6G == 1)
#define print_info_ch(_Mod, _Clz, _Fmt, var) \
		do { \
			int written = 0; \
			int totalLen = SCN_SCAN_DONE_PRINT_BUFFER_LENGTH; \
			enum ENUM_BAND eBand = BAND_NULL; \
			for (u4ChCnt = 0; u4ChCnt < num; u4ChCnt++) { \
				eBand = \
				SCN_GET_EBAND_BY_CH_NUM( \
				prScanDone->var[u4ChCnt]); \
				prScanInfo->aeChannelBand[u4ChCnt] = eBand; \
				prScanInfo->var[u4ChCnt] \
				= prScanDone->var[u4ChCnt]; \
				nicRxdChNumTranslate(eBand, \
				&prScanInfo->var[u4ChCnt]); \
				written += kalSnprintf(strbuf + written, \
				totalLen - written, "%6d", \
				prScanInfo->var[u4ChCnt]); \
			} \
			log_dbg(_Mod, _Clz, _Fmt, strbuf); \
		} while (0)
#endif

#define print_info(_Mod, _Clz, _Fmt, var) \
		do { \
			int written = 0; \
		    int totalLen = SCN_SCAN_DONE_PRINT_BUFFER_LENGTH; \
			for (u4ChCnt = 0; u4ChCnt < num; u4ChCnt++) { \
				prScanInfo->var[u4ChCnt] \
					= prScanDone->var[u4ChCnt]; \
				written += kalSnprintf(strbuf + written, \
					totalLen - written, "%6d", \
					prScanInfo->var[u4ChCnt]); \
			} \
			log_dbg(_Mod, _Clz, _Fmt, strbuf); \
		} while (0)

#if (CFG_SUPPORT_WIFI_6G == 1)
		print_info_ch(SCN, INFO, "Channel  : %s\n", aucChannelNum);
#else
		print_info(SCN, INFO, "Channel  : %s\n", aucChannelNum);
#endif
		print_info(SCN, LOUD, "IdleTime : %s\n", au2ChannelIdleTime);
		print_info(SCN, LOUD, "MdrdyCnt : %s\n", aucChannelMDRDYCnt);
		print_info(SCN, INFO, "BAndPCnt : %s\n", aucChannelBAndPCnt);

#undef	print_scan_info
	} else {
		prScanInfo->fgIsSparseChannelValid = FALSE;
	}

	/* Full2Partial */
	if (prScanInfo->fgIsScanForFull2Partial &&
		prScanInfo->ucFull2PartialSeq == prScanDone->ucSeqNum) {
		uint32_t *pu4BitMap = &(prScanInfo->au4ChannelBitMap[0]);
#if (CFG_SUPPORT_WIFI_6G == 1)
		log_dbg(SCN, INFO,
			"Full2Partial(%u):%08X %08X %08X %08X %08X %08X %08X %08X %08X %08X %08X %08X %08X %08X %08X %08X\n",
			scanCountBits(prScanInfo->au4ChannelBitMap,
			sizeof(prScanInfo->au4ChannelBitMap)),
			pu4BitMap[7], pu4BitMap[6], pu4BitMap[5], pu4BitMap[4],
			pu4BitMap[3], pu4BitMap[2], pu4BitMap[1], pu4BitMap[0],
			pu4BitMap[15], pu4BitMap[14], pu4BitMap[13],
			pu4BitMap[12], pu4BitMap[11], pu4BitMap[10],
			pu4BitMap[9], pu4BitMap[8]);
#else
		log_dbg(SCN, INFO,
			"Full2Partial(%u):%08X %08X %08X %08X %08X %08X %08X %08X\n",
			scanCountBits(prScanInfo->au4ChannelBitMap,
			sizeof(prScanInfo->au4ChannelBitMap)),
			pu4BitMap[7], pu4BitMap[6], pu4BitMap[5], pu4BitMap[4],
			pu4BitMap[3], pu4BitMap[2], pu4BitMap[1], pu4BitMap[0]);
#endif

		prScanInfo->fgIsScanForFull2Partial = FALSE;
	}

	if (prScanInfo->eCurrentState == SCAN_STATE_SCANNING
		&& prScanDone->ucSeqNum == prScanParam->ucSeqNum) {
		scanRemoveBssDescsByPolicy(prAdapter,
		       SCN_RM_POLICY_EXCLUDE_CONNECTED | SCN_RM_POLICY_TIMEOUT);

		/* generate scan-done event for caller */
		scnFsmGenerateScanDoneMsg(prAdapter, prScanParam->ucSeqNum,
			prScanParam->ucBssIndex, SCAN_STATUS_DONE);

		/* switch to next pending scan */
		scnFsmSteps(prAdapter, SCAN_STATE_IDLE);
	} else {
		log_dbg(SCN, INFO, "Unexpected SCAN-DONE event: SeqNum = %d, Current State = %d\n",
			prScanDone->ucSeqNum,
			prScanInfo->eCurrentState);
	}
}	/* end of scnEventScanDone */

/*----------------------------------------------------------------------------*/
/*!
 * \brief
 *
 * \param[in]
 *
 * \return none
 */
/*----------------------------------------------------------------------------*/
void
scnFsmGenerateScanDoneMsg(IN struct ADAPTER *prAdapter,
	IN uint8_t ucSeqNum, IN uint8_t ucBssIndex,
	IN enum ENUM_SCAN_STATUS eScanStatus)
{
	struct SCAN_INFO *prScanInfo;
	struct SCAN_PARAM *prScanParam;
	struct MSG_SCN_SCAN_DONE *prScanDoneMsg;

	ASSERT(prAdapter);

	prScanInfo = &(prAdapter->rWifiVar.rScanInfo);
	prScanParam = &prScanInfo->rScanParam;

	prScanDoneMsg = (struct MSG_SCN_SCAN_DONE *) cnmMemAlloc(prAdapter,
		RAM_TYPE_MSG, sizeof(struct MSG_SCN_SCAN_DONE));
	if (!prScanDoneMsg) {
		ASSERT(0);	/* Can't indicate SCAN FSM Complete */
		return;
	}

	if (prScanParam->fgIsObssScan == TRUE) {
		prScanDoneMsg->rMsgHdr.eMsgId = MID_SCN_RLM_SCAN_DONE;
	} else {
		switch (GET_BSS_INFO_BY_INDEX(
			prAdapter, ucBssIndex)->eNetworkType) {
		case NETWORK_TYPE_AIS:
			prScanDoneMsg->rMsgHdr.eMsgId = MID_SCN_AIS_SCAN_DONE;
			break;

		case NETWORK_TYPE_P2P:
			prScanDoneMsg->rMsgHdr.eMsgId = MID_SCN_P2P_SCAN_DONE;
			break;

		case NETWORK_TYPE_BOW:
			prScanDoneMsg->rMsgHdr.eMsgId = MID_SCN_BOW_SCAN_DONE;
			break;

		default:
			log_dbg(SCN, LOUD,
				"Unexpected Network Type: %d\n",
				GET_BSS_INFO_BY_INDEX(
					prAdapter, ucBssIndex)->eNetworkType);
			ASSERT(0);
			break;
		}
	}

	prScanDoneMsg->ucSeqNum = ucSeqNum;
	prScanDoneMsg->ucBssIndex = ucBssIndex;
	prScanDoneMsg->eScanStatus = eScanStatus;

	mboxSendMsg(prAdapter, MBOX_ID_0,
		(struct MSG_HDR *) prScanDoneMsg, MSG_SEND_METHOD_BUF);

}	/* end of scnFsmGenerateScanDoneMsg() */

/*----------------------------------------------------------------------------*/
/*!
 * \brief        Query for most sparse channel
 *
 * \param[in]
 *
 * \return none
 */
/*----------------------------------------------------------------------------*/
u_int8_t scnQuerySparseChannel(IN struct ADAPTER *prAdapter,
	enum ENUM_BAND *prSparseBand, uint8_t *pucSparseChannel)
{
	struct SCAN_INFO *prScanInfo;

	ASSERT(prAdapter);

	prScanInfo = &(prAdapter->rWifiVar.rScanInfo);

	if (prScanInfo->fgIsSparseChannelValid == TRUE) {
		if (prSparseBand)
			*prSparseBand = prScanInfo->rSparseChannel.eBand;

		if (pucSparseChannel) {
			*pucSparseChannel
				= prScanInfo->rSparseChannel.ucChannelNum;
		}

		return TRUE;
	} else {
		return FALSE;
	}
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief        Event handler for schedule scan done event
 *
 * \param[in]
 *
 * \return none
 */
/*----------------------------------------------------------------------------*/
void scnEventSchedScanDone(IN struct ADAPTER *prAdapter,
	IN struct EVENT_SCHED_SCAN_DONE *prSchedScanDone)
{
	struct SCAN_INFO *prScanInfo;
	struct SCHED_SCAN_PARAM *prSchedScanParam;

	prScanInfo = &(prAdapter->rWifiVar.rScanInfo);
	prSchedScanParam = &prScanInfo->rSchedScanParam;

	if (prScanInfo->fgSchedScanning == TRUE) {
		scanlog_dbg(LOG_SCHED_SCAN_DONE_F2D, INFO, "scnEventSchedScanDone seq %u\n",
			prSchedScanDone->ucSeqNum);

		kalSchedScanResults(prAdapter->prGlueInfo);
	} else {
		scanlog_dbg(LOG_SCHED_SCAN_DONE_F2D, INFO, "Unexpected SCHEDSCANDONE event: Seq = %u, Current State = %d\n",
			prSchedScanDone->ucSeqNum, prScanInfo->eCurrentState);
	}
}

#if CFG_SUPPORT_SCHED_SCAN
/*----------------------------------------------------------------------------*/
/*!
 * \brief        handler for starting schedule scan
 *
 * \param[in]
 *
 * \return       TRUE if send sched scan successfully. FALSE otherwise
 */
/*----------------------------------------------------------------------------*/
u_int8_t
scnFsmSchedScanRequest(IN struct ADAPTER *prAdapter,
	IN struct PARAM_SCHED_SCAN_REQUEST *prRequest)
{
	struct SCAN_INFO *prScanInfo;
	struct SCHED_SCAN_PARAM *prSchedScanParam;
	struct CMD_SCHED_SCAN_REQ *prSchedScanCmd = NULL;
	struct SSID_MATCH_SETS *prMatchSets = NULL;
	struct PARAM_SSID *prSsid = NULL;
	uint32_t i;
	uint16_t u2IeLen;
	enum ENUM_BAND ePreferedChnl = BAND_NULL;
	struct BSS_INFO *prAisBssInfo;

	ASSERT(prAdapter);
	ASSERT(prRequest);
	ASSERT(prRequest->u4SsidNum <= CFG_SCAN_HIDDEN_SSID_MAX_NUM);
	ASSERT(prRequest->u4MatchSsidNum <= CFG_SCAN_SSID_MATCH_MAX_NUM);
	log_dbg(SCN, TRACE, "scnFsmSchedScanRequest\n");

	prAisBssInfo = aisGetAisBssInfo(prAdapter,
		prRequest->ucBssIndex);
	if (prAisBssInfo == NULL) {
		log_dbg(SCN, WARN, "prAisBssInfo is NULL\n");
		return FALSE;
	}

	prScanInfo = &(prAdapter->rWifiVar.rScanInfo);
	prSchedScanParam = &prScanInfo->rSchedScanParam;

	if (prScanInfo->fgSchedScanning) {
		log_dbg(SCN, WARN, "prScanInfo->fgSchedScanning = TRUE already scanning\n");

		return FALSE;
	}

	/* 0. allocate memory for schedule scan command */
	if (prRequest->u4IELength <= MAX_IE_LENGTH)
		u2IeLen = (uint16_t)prRequest->u4IELength;
	else
		u2IeLen = MAX_IE_LENGTH;

	prSchedScanCmd = (struct CMD_SCHED_SCAN_REQ *) cnmMemAlloc(prAdapter,
		RAM_TYPE_BUF, sizeof(struct CMD_SCHED_SCAN_REQ) + u2IeLen);
	if (!prSchedScanCmd) {
		log_dbg(SCN, ERROR, "alloc CMD_SCHED_SCAN_REQ (%zu+%u) fail\n",
			sizeof(struct CMD_SCHED_SCAN_REQ), u2IeLen);
		return FALSE;
	}
	kalMemZero(prSchedScanCmd, sizeof(struct CMD_SCHED_SCAN_REQ) + u2IeLen);
	prMatchSets = &(prSchedScanCmd->auMatchSsid[0]);
	prSsid = &(prSchedScanCmd->auSsid[0]);

	/* 1 Set Sched scan param parameters */
	prSchedScanParam->ucSeqNum++;
	prSchedScanParam->ucBssIndex = prAisBssInfo->ucBssIndex;
	prSchedScanParam->fgStopAfterIndication = FALSE;

	if (!IS_NET_ACTIVE(prAdapter, prAisBssInfo->ucBssIndex)) {
		SET_NET_ACTIVE(prAdapter, prAisBssInfo->ucBssIndex);
		/* sync with firmware */
		nicActivateNetwork(prAdapter,
			prAisBssInfo->ucBssIndex);
	}

	/* 2.1 Prepare command. Set FW struct SSID_MATCH_SETS */
	/* ssid in ssid list will be send in probe request in advance */
	prSchedScanCmd->ucSsidNum = prRequest->u4SsidNum;
	for (i = 0; i < prSchedScanCmd->ucSsidNum; i++) {
		kalMemCopy(&(prSsid[i]), &(prRequest->arSsid[i]),
			sizeof(struct PARAM_SSID));
		log_dbg(SCN, TRACE, "ssid set(%d) %s\n", i, prSsid[i].aucSsid);
	}

	prSchedScanCmd->ucMatchSsidNum = prRequest->u4MatchSsidNum;
	for (i = 0; i < prSchedScanCmd->ucMatchSsidNum; i++) {
		COPY_SSID(prMatchSets[i].aucSsid, prMatchSets[i].ucSsidLen,
			prRequest->arMatchSsid[i].aucSsid,
			prRequest->arMatchSsid[i].u4SsidLen);
		prMatchSets[i].i4RssiThresold = prRequest->ai4RssiThold[i];
		log_dbg(SCN, TRACE, "Match set(%d) %s, rssi>%d\n",
				i, prMatchSets[i].aucSsid,
				prMatchSets[i].i4RssiThresold);
	}

	/* 2.2 Prepare command. Set channel */

	ePreferedChnl
		= prAdapter->aePreferBand[NETWORK_TYPE_AIS];
	if (ePreferedChnl == BAND_2G4) {
		prSchedScanCmd->ucChannelType =
			SCHED_SCAN_CHANNEL_TYPE_2G4_ONLY;
		prSchedScanCmd->ucChnlNum = 0;
	} else if (ePreferedChnl == BAND_5G) {
		prSchedScanCmd->ucChannelType =
			SCHED_SCAN_CHANNEL_TYPE_5G_ONLY;
		prSchedScanCmd->ucChnlNum = 0;
	} else if (prRequest->ucChnlNum > 0 &&
		prRequest->ucChnlNum <=
			ARRAY_SIZE(prSchedScanCmd->aucChannel)) {
		prSchedScanCmd->ucChannelType =
			SCHED_SCAN_CHANNEL_TYPE_SPECIFIED;
		prSchedScanCmd->ucChnlNum = prRequest->ucChnlNum;
		for (i = 0; i < prRequest->ucChnlNum; i++) {
			prSchedScanCmd->aucChannel[i].ucChannelNum =
				prRequest->pucChannels[i];
			prSchedScanCmd->aucChannel[i].ucBand =
				(prSchedScanCmd->aucChannel[i].ucChannelNum <=
				HW_CHNL_NUM_MAX_2G4) ? BAND_2G4 : BAND_5G;
		}
	} else {
		prSchedScanCmd->ucChnlNum = 0;
		prSchedScanCmd->ucChannelType =
			SCHED_SCAN_CHANNEL_TYPE_DUAL_BAND;
	}

	prSchedScanCmd->ucSeqNum = prSchedScanParam->ucSeqNum;
	prSchedScanCmd->fgStopAfterIndication =
		prSchedScanParam->fgStopAfterIndication;
	prSchedScanCmd->u2IELen = u2IeLen;
	prSchedScanCmd->ucVersion = SCHED_SCAN_CMD_VERSION;
	if (prSchedScanCmd->u2IELen) {
		kalMemCopy(prSchedScanCmd->aucIE, prRequest->pucIE,
				prSchedScanCmd->u2IELen);
	}

	prSchedScanCmd->ucScnFuncMask |= prRequest->ucScnFuncMask;

	if (kalIsValidMacAddr(prRequest->aucRandomMac)) {
		prSchedScanCmd->ucScnFuncMask |=
			(ENUM_SCN_RANDOM_MAC_EN | ENUM_SCN_RANDOM_SN_EN);
		kalMemCopy(prSchedScanCmd->aucRandomMac,
			prRequest->aucRandomMac, MAC_ADDR_LEN);
	}

	scnSetSchedScanPlan(prAdapter, prSchedScanCmd);

	log_dbg(SCN, INFO, "V(%u)seq(%u)sz(%zu)chT(%u)chN(%u)ssid(%u)match(%u)IE(%u=>%u)MSP(%u)Func(0x%X)\n",
		prSchedScanCmd->ucVersion,
		prSchedScanCmd->ucSeqNum, sizeof(struct CMD_SCHED_SCAN_REQ),
		prSchedScanCmd->ucChannelType, prSchedScanCmd->ucChnlNum,
		prSchedScanCmd->ucSsidNum, prSchedScanCmd->ucMatchSsidNum,
		prRequest->u4IELength, prSchedScanCmd->u2IELen,
		prSchedScanCmd->ucMspEntryNum,
		prSchedScanCmd->ucScnFuncMask);

	/* 3. send command packet to FW */
	do {
		if (!scnFsmSchedScanSetCmd(prAdapter, prSchedScanCmd)) {
			log_dbg(SCN, TRACE, "scnFsmSchedScanSetCmd failed\n");
			break;
		}
		if (!scnFsmSchedScanSetAction(prAdapter,
				SCHED_SCAN_ACT_ENABLE)) {
			log_dbg(SCN, TRACE, "scnFsmSchedScanSetAction failed\n");
			break;
		}
		prScanInfo->fgSchedScanning = TRUE;
	} while (0);

	if (!prScanInfo->fgSchedScanning)
		nicDeactivateNetwork(prAdapter,
			prAisBssInfo->ucBssIndex);

	cnmMemFree(prAdapter, (void *) prSchedScanCmd);

	return prScanInfo->fgSchedScanning;
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief         handler for stopping scheduled scan
 *
 * \param[in]
 *
 * \return        TRUE if send stop command successfully. FALSE otherwise
 */
/*----------------------------------------------------------------------------*/
u_int8_t scnFsmSchedScanStopRequest(IN struct ADAPTER *prAdapter)
{
	uint8_t ucBssIndex = 0;

	ASSERT(prAdapter);

	ucBssIndex =
		prAdapter->rWifiVar.rScanInfo.rSchedScanParam.ucBssIndex;

	log_dbg(SCN, INFO, "ucBssIndex = %d\n", ucBssIndex);

	if (aisGetAisBssInfo(prAdapter,
		ucBssIndex) == NULL) {
		log_dbg(SCN, WARN,
			"prAisBssInfo%d is NULL\n",
			ucBssIndex);
		return FALSE;
	}

	if (!scnFsmSchedScanSetAction(prAdapter, SCHED_SCAN_ACT_DISABLE)) {
		log_dbg(SCN, TRACE, "scnFsmSchedScanSetAction failed\n");
		return FALSE;
	}

	prAdapter->rWifiVar.rScanInfo.fgSchedScanning = FALSE;

	return TRUE;
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief handler for setting schedule scan action
 * \param prAdapter       adapter
 * \param ucSchedScanAct  schedule scan action. set enable/disable to FW
 *
 * \return TRUE if send query command successfully. FALSE otherwise
 */
/*----------------------------------------------------------------------------*/
u_int8_t
scnFsmSchedScanSetAction(IN struct ADAPTER *prAdapter,
		 IN enum ENUM_SCHED_SCAN_ACT ucSchedScanAct)
{
	struct CMD_SET_SCHED_SCAN_ENABLE rCmdSchedScanAction;
	uint32_t rStatus;

	ASSERT(prAdapter);

	kalMemZero(&rCmdSchedScanAction,
		sizeof(struct CMD_SET_SCHED_SCAN_ENABLE));

	/* 0:enable, 1:disable */
	rCmdSchedScanAction.ucSchedScanAct = ucSchedScanAct;

	if (ucSchedScanAct == SCHED_SCAN_ACT_ENABLE) {
		scanlog_dbg(LOG_SCHED_SCAN_REQ_START_D2F, INFO, "sched scan action = %d\n",
			rCmdSchedScanAction.ucSchedScanAct);
	} else {
		scanlog_dbg(LOG_SCHED_SCAN_REQ_STOP_D2F, INFO, "sched scan action = %d\n",
			rCmdSchedScanAction.ucSchedScanAct);
	}

	rStatus = wlanSendSetQueryCmd(prAdapter,
			    CMD_ID_SET_SCAN_SCHED_ENABLE,
			    TRUE,
			    FALSE,
			    FALSE,
			    nicCmdEventSetCommon,
			    nicOidCmdTimeoutCommon,
			    sizeof(struct CMD_SET_SCHED_SCAN_ENABLE),
			    (uint8_t *)&rCmdSchedScanAction, NULL, 0);

	return (rStatus != WLAN_STATUS_FAILURE) ? TRUE : FALSE;
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief                 handler for setting schedule scan command
 * \param prAdapter       adapter
 * \param prSchedScanCmd  schedule scan command
 *
 * \return                TRUE if send query command successfully.
 *                        FAIL otherwise
 */
/*----------------------------------------------------------------------------*/
u_int8_t
scnFsmSchedScanSetCmd(IN struct ADAPTER *prAdapter,
		IN struct CMD_SCHED_SCAN_REQ *prSchedScanCmd)
{
	uint16_t u2IeSize = 0;
	uint32_t rStatus;

	ASSERT(prAdapter);

	log_dbg(SCN, TRACE, "--> %s()\n", __func__);

	if (prSchedScanCmd)
		u2IeSize = prSchedScanCmd->u2IELen;
	rStatus = wlanSendSetQueryCmd(prAdapter,
			    CMD_ID_SET_SCAN_SCHED_REQ,
			    TRUE,
			    FALSE,
			    FALSE,
			    nicCmdEventSetCommon,
			    nicOidCmdTimeoutCommon,
			    sizeof(struct CMD_SCHED_SCAN_REQ) + u2IeSize,
			    (uint8_t *) prSchedScanCmd, NULL, 0);

	return (rStatus != WLAN_STATUS_FAILURE) ? TRUE : FALSE;
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief                 Set schedule scan multiple scan plan (scan interval)
 * \param prAdapter       adapter
 * \param prSchedScanCmd  schedule scan command request
 *
 * \return                void
 */
/*----------------------------------------------------------------------------*/
void
scnSetSchedScanPlan(IN struct ADAPTER *prAdapter,
		IN struct CMD_SCHED_SCAN_REQ *prSchedScanCmd)
{
	/* Set Multiple Scan Plan here */
	log_dbg(SCN, TRACE, "--> %s()\n", __func__);

	ASSERT(prAdapter);

	prSchedScanCmd->ucMspEntryNum = 0;
	kalMemZero(prSchedScanCmd->au2MspList,
			sizeof(prSchedScanCmd->au2MspList));
}

#endif /* CFG_SUPPORT_SCHED_SCAN */
