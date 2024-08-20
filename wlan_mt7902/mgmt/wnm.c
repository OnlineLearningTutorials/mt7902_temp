/* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause */
/*
 * Copyright (c) 2016 MediaTek Inc.
 */
/*
 ** Id: //Department/DaVinci/TRUNK/MT6620_5931_WiFi_Driver/mgmt/wnm.c#1
 */

/*! \file   "wnm.c"
 *    \brief  This file includes the 802.11v default vale and functions.
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

#define WNM_MAX_TOD_ERROR 0
#define WNM_MAX_TOA_ERROR 0
#define MICRO_TO_10NANO(x) ((x)*100)
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
#if CFG_SUPPORT_802_11V_TIMING_MEASUREMENT
static uint8_t ucTimingMeasToken;
#endif
static uint8_t ucBtmMgtToken = 1;

/*******************************************************************************
 *                                 M A C R O S
 *******************************************************************************
 */

/*******************************************************************************
 *                   F U N C T I O N   D E C L A R A T I O N S
 *******************************************************************************
 */
#if CFG_SUPPORT_802_11V_TIMING_MEASUREMENT
static uint32_t
wnmRunEventTimgingMeasTxDone(IN struct ADAPTER *prAdapter,
			     IN struct MSDU_INFO *prMsduInfo,
			     IN enum ENUM_TX_RESULT_CODE rTxDoneStatus);

static void wnmComposeTimingMeasFrame(IN struct ADAPTER *prAdapter,
				      IN struct STA_RECORD *prStaRec,
				      IN PFN_TX_DONE_HANDLER pfTxDoneHandler);

static void wnmTimingMeasRequest(IN struct ADAPTER *prAdapter,
				 IN struct SW_RFB *prSwRfb);
#endif
/*******************************************************************************
 *                              F U N C T I O N S
 *******************************************************************************
 */

/*----------------------------------------------------------------------------*/
/*!
 *
 * \brief This routine is called to process the 802.11v wnm category action
 * frame.
 *
 *
 * \note
 *      Called by: Handle Rx mgmt request
 */
/*----------------------------------------------------------------------------*/
void wnmWNMAction(IN struct ADAPTER *prAdapter, IN struct SW_RFB *prSwRfb)
{
	struct WLAN_ACTION_FRAME *prRxFrame;

	ASSERT(prAdapter);
	ASSERT(prSwRfb);

	prRxFrame = (struct WLAN_ACTION_FRAME *)prSwRfb->pvHeader;

	DBGLOG(WNM, TRACE, "WNM action frame: %d from " MACSTR "\n",
	       prRxFrame->ucAction, MAC2STR(prRxFrame->aucSrcAddr));

	switch (prRxFrame->ucAction) {
#if CFG_SUPPORT_802_11V_TIMING_MEASUREMENT
	case ACTION_WNM_TIMING_MEASUREMENT_REQUEST:
		wnmTimingMeasRequest(prAdapter, prSwRfb);
		break;
#endif
#if CFG_AP_80211V_SUPPORT
	case ACTION_WNM_BSS_TRANSITION_MANAGEMENT_RSP:
		wnmMulAPAgentRecvBTMResponse(prAdapter, prSwRfb);
		break;
#endif /* CFG_AP_80211V_SUPPORT */
#if CFG_SUPPORT_802_11V_BSS_TRANSITION_MGT
	case ACTION_WNM_BSS_TRANSITION_MANAGEMENT_REQ:
#endif
	case ACTION_WNM_NOTIFICATION_REQUEST:
	default:
		DBGLOG(RX, INFO,
		       "WNM: action frame %d, try to send to supplicant\n",
		       prRxFrame->ucAction);
		aisFuncValidateRxActionFrame(prAdapter, prSwRfb);
		break;
	}
}

#if CFG_SUPPORT_802_11V_TIMING_MEASUREMENT
/*----------------------------------------------------------------------------*/
/*!
 *
 * \brief This routine is called to report timing measurement data.
 *
 */
/*----------------------------------------------------------------------------*/
void wnmReportTimingMeas(IN struct ADAPTER *prAdapter, IN uint8_t ucStaRecIndex,
			 IN uint32_t u4ToD, IN uint32_t u4ToA)
{
	struct STA_RECORD *prStaRec;

	prStaRec = cnmGetStaRecByIndex(prAdapter, ucStaRecIndex);

	if ((!prStaRec) || (!prStaRec->fgIsInUse))
		return;

	DBGLOG(WNM, TRACE, "WNM: wnmReportTimingMeas: u4ToD %x u4ToA %x", u4ToD,
	       u4ToA);

	if (!prStaRec->rWNMTimingMsmt.ucTrigger)
		return;

	prStaRec->rWNMTimingMsmt.u4ToD = MICRO_TO_10NANO(u4ToD);
	prStaRec->rWNMTimingMsmt.u4ToA = MICRO_TO_10NANO(u4ToA);
}

/*----------------------------------------------------------------------------*/
/*!
 * @brief This function will handle TxDone(TimingMeasurement) Event.
 *
 * @param[in] prAdapter      Pointer to the Adapter structure.
 * @param[in] prMsduInfo     Pointer to the MSDU_INFO_T.
 * @param[in] rTxDoneStatus  Return TX status of the Timing Measurement frame.
 *
 * @retval WLAN_STATUS_SUCCESS
 */
/*----------------------------------------------------------------------------*/
static uint32_t
wnmRunEventTimgingMeasTxDone(IN struct ADAPTER *prAdapter,
			     IN struct MSDU_INFO *prMsduInfo,
			     IN enum ENUM_TX_RESULT_CODE rTxDoneStatus)
{
	struct STA_RECORD *prStaRec;

	ASSERT(prAdapter);
	ASSERT(prMsduInfo);

	DBGLOG(WNM, LOUD, "WNM: EVENT-TX DONE: Current Time = %ld\n",
	       kalGetTimeTick());

	prStaRec = cnmGetStaRecByIndex(prAdapter, prMsduInfo->ucStaRecIndex);

	if ((!prStaRec) || (!prStaRec->fgIsInUse))
		return WLAN_STATUS_SUCCESS; /* For the case of replying ERROR
					     * STATUS CODE
					     */

	DBGLOG(WNM, TRACE,
	       "WNM: wnmRunEventTimgingMeasTxDone: ucDialog %d ucFollowUp %d u4ToD %x u4ToA %x",
	       prStaRec->rWNMTimingMsmt.ucDialogToken,
	       prStaRec->rWNMTimingMsmt.ucFollowUpDialogToken,
	       prStaRec->rWNMTimingMsmt.u4ToD, prStaRec->rWNMTimingMsmt.u4ToA);

	prStaRec->rWNMTimingMsmt.ucFollowUpDialogToken =
		prStaRec->rWNMTimingMsmt.ucDialogToken;
	prStaRec->rWNMTimingMsmt.ucDialogToken = ++ucTimingMeasToken;

	wnmComposeTimingMeasFrame(prAdapter, prStaRec, NULL);

	return WLAN_STATUS_SUCCESS;

} /* end of wnmRunEventTimgingMeasTxDone() */

/*----------------------------------------------------------------------------*/
/*!
 * @brief This function will compose the Timing Measurement frame.
 *
 * @param[in] prAdapter              Pointer to the Adapter structure.
 * @param[in] prStaRec               Pointer to the STA_RECORD_T.
 *
 * @return (none)
 */
/*----------------------------------------------------------------------------*/
static void wnmComposeTimingMeasFrame(IN struct ADAPTER *prAdapter,
				      IN struct STA_RECORD *prStaRec,
				      IN PFN_TX_DONE_HANDLER pfTxDoneHandler)
{
	struct MSDU_INFO *prMsduInfo;
	struct BSS_INFO *prBssInfo;
	struct ACTION_UNPROTECTED_WNM_TIMING_MEAS_FRAME *prTxFrame;
	uint16_t u2PayloadLen;

	prBssInfo = &prAdapter->rWifiVar.arBssInfo[prStaRec->ucNetTypeIndex];
	ASSERT(prBssInfo);

	prMsduInfo = (struct MSDU_INFO *)cnmMgtPktAlloc(
		prAdapter, MAC_TX_RESERVED_FIELD + PUBLIC_ACTION_MAX_LEN);

	if (!prMsduInfo)
		return;

	prTxFrame = (struct ACTION_UNPROTECTED_WNM_TIMING_MEAS_FRAME
			     *)((uint32_t)(prMsduInfo->prPacket) +
				MAC_TX_RESERVED_FIELD);

	prTxFrame->u2FrameCtrl = MAC_FRAME_ACTION;

	COPY_MAC_ADDR(prTxFrame->aucDestAddr, prStaRec->aucMacAddr);
	COPY_MAC_ADDR(prTxFrame->aucSrcAddr, prBssInfo->aucOwnMacAddr);
	COPY_MAC_ADDR(prTxFrame->aucBSSID, prBssInfo->aucBSSID);

	prTxFrame->ucCategory = CATEGORY_UNPROTECTED_WNM_ACTION;
	prTxFrame->ucAction = ACTION_UNPROTECTED_WNM_TIMING_MEASUREMENT;

	/* 3 Compose the frame body's frame. */
	prTxFrame->ucDialogToken = prStaRec->rWNMTimingMsmt.ucDialogToken;
	prTxFrame->ucFollowUpDialogToken =
		prStaRec->rWNMTimingMsmt.ucFollowUpDialogToken;
	prTxFrame->u4ToD = prStaRec->rWNMTimingMsmt.u4ToD;
	prTxFrame->u4ToA = prStaRec->rWNMTimingMsmt.u4ToA;
	prTxFrame->ucMaxToDErr = WNM_MAX_TOD_ERROR;
	prTxFrame->ucMaxToAErr = WNM_MAX_TOA_ERROR;

	u2PayloadLen = 2 + ACTION_UNPROTECTED_WNM_TIMING_MEAS_LEN;

	/* 4 Update information of MSDU_INFO_T */
	TX_SET_MMPDU(prAdapter, prMsduInfo, prStaRec->ucBssIndex,
		     prStaRec->ucIndex, WLAN_MAC_MGMT_HEADER_LEN,
		     WLAN_MAC_MGMT_HEADER_LEN + u2PayloadLen, pfTxDoneHandler,
		     MSDU_RATE_MODE_AUTO);

	DBGLOG(WNM, TRACE,
	       "WNM: wnmComposeTimingMeasFrame: ucDialogToken %d ucFollowUpDialogToken %d u4ToD %x u4ToA %x\n",
	       prTxFrame->ucDialogToken, prTxFrame->ucFollowUpDialogToken,
	       prTxFrame->u4ToD, prTxFrame->u4ToA);

	/* 4 Enqueue the frame to send this action frame. */
	nicTxEnqueueMsdu(prAdapter, prMsduInfo);

	return;

} /* end of wnmComposeTimingMeasFrame() */

/*----------------------------------------------------------------------------*/
/*!
 *
 * \brief This routine is called to process the 802.11v timing measurement
 * request.
 *
 *
 * \note
 *      Handle Rx mgmt request
 */
/*----------------------------------------------------------------------------*/
static void wnmTimingMeasRequest(IN struct ADAPTER *prAdapter,
				 IN struct SW_RFB *prSwRfb)
{
	struct ACTION_WNM_TIMING_MEAS_REQ_FRAME *prRxFrame = NULL;
	struct STA_RECORD *prStaRec;

	prRxFrame =
		(struct ACTION_WNM_TIMING_MEAS_REQ_FRAME *)prSwRfb->pvHeader;
	if (!prRxFrame)
		return;

	prStaRec = cnmGetStaRecByIndex(prAdapter, prSwRfb->ucStaRecIdx);
	if ((!prStaRec) || (!prStaRec->fgIsInUse))
		return;

	DBGLOG(WNM, TRACE,
	       "WNM: Received Timing Measuremen Request from " MACSTR "\n",
	       MAC2STR(prStaRec->aucMacAddr));

	/* reset timing msmt */
	prStaRec->rWNMTimingMsmt.fgInitiator = TRUE;
	prStaRec->rWNMTimingMsmt.ucTrigger = prRxFrame->ucTrigger;
	if (!prRxFrame->ucTrigger)
		return;
	prStaRec->rWNMTimingMsmt.ucDialogToken = ++ucTimingMeasToken;
	prStaRec->rWNMTimingMsmt.ucFollowUpDialogToken = 0;
	wnmComposeTimingMeasFrame(prAdapter, prStaRec,
				  wnmRunEventTimgingMeasTxDone);
}

#if WNM_UNIT_TEST
void wnmTimingMeasUnitTest1(struct ADAPTER *prAdapter, uint8_t ucStaRecIndex)
{
	struct STA_RECORD *prStaRec;

	prStaRec = cnmGetStaRecByIndex(prAdapter, ucStaRecIndex);
	if ((!prStaRec) || (!prStaRec->fgIsInUse))
		return;
	DBGLOG(WNM, INFO,
	       "WNM: Test Timing Measuremen Request from " MACSTR "\n",
	       MAC2STR(prStaRec->aucMacAddr));
	prStaRec->rWNMTimingMsmt.fgInitiator = TRUE;
	prStaRec->rWNMTimingMsmt.ucTrigger = 1;
	prStaRec->rWNMTimingMsmt.ucDialogToken = ++ucTimingMeasToken;
	prStaRec->rWNMTimingMsmt.ucFollowUpDialogToken = 0;
	wnmComposeTimingMeasFrame(prAdapter, prStaRec,
				  wnmRunEventTimgingMeasTxDone);
}
#endif

#endif /* CFG_SUPPORT_802_11V_TIMING_MEASUREMENT */

uint8_t wnmGetBtmToken(void)
{
	return ucBtmMgtToken++;
}

static uint32_t wnmBTMQueryTxDone(IN struct ADAPTER *prAdapter,
				  IN struct MSDU_INFO *prMsduInfo,
				  IN enum ENUM_TX_RESULT_CODE rTxDoneStatus)
{
	DBGLOG(WNM, INFO, "BTM: Query Frame Tx Done, Status %d\n",
	       rTxDoneStatus);
	return WLAN_STATUS_SUCCESS;
}

static uint32_t wnmBTMResponseTxDone(IN struct ADAPTER *prAdapter,
				     IN struct MSDU_INFO *prMsduInfo,
				     IN enum ENUM_TX_RESULT_CODE rTxDoneStatus)
{
	uint8_t ucBssIndex = 0;
	struct BSS_TRANSITION_MGT_PARAM_T *prBtm;
	struct AIS_FSM_INFO *prAisFsmInfo;

	ASSERT(prMsduInfo);

	ucBssIndex =
		prMsduInfo->ucBssIndex;
	prBtm =
		aisGetBTMParam(prAdapter, ucBssIndex);
	prAisFsmInfo =
		aisGetAisFsmInfo(prAdapter, ucBssIndex);

	DBGLOG(WNM, INFO, "BTM: Response Frame Tx Done Status %d\n",
	       rTxDoneStatus);
	if (prBtm->fgPendingResponse &&
	    prAisFsmInfo->eCurrentState == AIS_STATE_SEARCH) {
		prBtm->fgPendingResponse = FALSE;
		aisFsmSteps(prAdapter, AIS_STATE_REQ_CHANNEL_JOIN,
			ucBssIndex);
	}
	return WLAN_STATUS_SUCCESS;
}

/*----------------------------------------------------------------------------*/
/*!
 * @brief This function will compose the BTM Response frame.
 *
 * @param[in] prAdapter              Pointer to the Adapter structure.
 * @param[in] prStaRec               Pointer to the STA_RECORD_T.
 *
 * @return (none)
 */
/*----------------------------------------------------------------------------*/
void wnmSendBTMResponseFrame(IN struct ADAPTER *prAdapter,
			     IN struct STA_RECORD *prStaRec)
{
	struct MSDU_INFO *prMsduInfo = NULL;
	struct BSS_INFO *prBssInfo = NULL;
	struct ACTION_BTM_RSP_FRAME *prTxFrame = NULL;
	uint16_t u2PayloadLen = 0;
	struct BSS_TRANSITION_MGT_PARAM_T *prBtmParam;
	uint8_t *pucOptInfo = NULL;

	if (!prStaRec) {
		DBGLOG(WNM, INFO, "BTM: No station record found\n");
		return;
	}

	prBssInfo = GET_BSS_INFO_BY_INDEX(prAdapter, prStaRec->ucBssIndex);
	ASSERT(prBssInfo);
	prBtmParam =
		aisGetBTMParam(prAdapter, prStaRec->ucBssIndex);

	/* 1 Allocate MSDU Info */
	prMsduInfo = (struct MSDU_INFO *)cnmMgtPktAlloc(
		prAdapter, MAC_TX_RESERVED_FIELD + PUBLIC_ACTION_MAX_LEN);
	if (!prMsduInfo)
		return;
	prTxFrame = (struct ACTION_BTM_RSP_FRAME
			     *)((unsigned long)(prMsduInfo->prPacket) +
				MAC_TX_RESERVED_FIELD);

	/* 2 Compose The Mac Header. */
	prTxFrame->u2FrameCtrl = MAC_FRAME_ACTION;

	COPY_MAC_ADDR(prTxFrame->aucDestAddr, prStaRec->aucMacAddr);
	COPY_MAC_ADDR(prTxFrame->aucSrcAddr, prBssInfo->aucOwnMacAddr);
	COPY_MAC_ADDR(prTxFrame->aucBSSID, prBssInfo->aucBSSID);

	prTxFrame->ucCategory = CATEGORY_WNM_ACTION;
	prTxFrame->ucAction = ACTION_WNM_BSS_TRANSITION_MANAGEMENT_RSP;

	/* 3 Compose the frame body's frame. */
	prTxFrame->ucDialogToken = prBtmParam->ucDialogToken;
	prBtmParam->ucDialogToken = 0;	/* reset dialog token */
	prTxFrame->ucStatusCode = prBtmParam->ucStatusCode;
	prTxFrame->ucBssTermDelay = prBtmParam->ucTermDelay;
	pucOptInfo = &prTxFrame->aucOptInfo[0];
	if (prBtmParam->ucStatusCode == BSS_TRANSITION_MGT_STATUS_ACCEPT) {
		COPY_MAC_ADDR(pucOptInfo, prBtmParam->aucTargetBssid);
		pucOptInfo += MAC_ADDR_LEN;
		u2PayloadLen += MAC_ADDR_LEN;
	}
	if (prBtmParam->u2OurNeighborBssLen > 0) {
		kalMemCopy(pucOptInfo, prBtmParam->pucOurNeighborBss,
			   prBtmParam->u2OurNeighborBssLen);
		kalMemFree(prBtmParam->pucOurNeighborBss, VIR_MEM_TYPE,
			   prBtmParam->u2OurNeighborBssLen);
		prBtmParam->u2OurNeighborBssLen = 0;
		u2PayloadLen += prBtmParam->u2OurNeighborBssLen;
	}

	/* 4 Update information of MSDU_INFO_T */
	TX_SET_MMPDU(prAdapter, prMsduInfo, prStaRec->ucBssIndex,
		     prStaRec->ucIndex, WLAN_MAC_MGMT_HEADER_LEN,
		     OFFSET_OF(struct ACTION_BTM_RSP_FRAME, aucOptInfo) +
			     u2PayloadLen,
		     wnmBTMResponseTxDone, MSDU_RATE_MODE_AUTO);

	/* 5 Enqueue the frame to send this action frame. */
	nicTxEnqueueMsdu(prAdapter, prMsduInfo);
}				/* end of wnmComposeBTMResponseFrame() */

/*----------------------------------------------------------------------------*/
/*!
 * @brief This function will compose the BTM Query frame.
 *
 * @param[in] prAdapter              Pointer to the Adapter structure.
 * @param[in] prStaRec               Pointer to the STA_RECORD_T.
 *
 * @return (none)
 */
/*----------------------------------------------------------------------------*/
void wnmSendBTMQueryFrame(IN struct ADAPTER *prAdapter,
			  IN struct STA_RECORD *prStaRec)
{
	struct MSDU_INFO *prMsduInfo = NULL;
	struct BSS_INFO *prBssInfo = NULL;
	struct ACTION_BTM_QUERY_FRAME *prTxFrame = NULL;
	struct BSS_TRANSITION_MGT_PARAM_T *prBtmParam;

	prBssInfo = GET_BSS_INFO_BY_INDEX(prAdapter, prStaRec->ucBssIndex);
	ASSERT(prBssInfo);
	prBtmParam =
		aisGetBTMParam(prAdapter, prStaRec->ucBssIndex);

	/* 1 Allocate MSDU Info */
	prMsduInfo = (struct MSDU_INFO *)cnmMgtPktAlloc(
		prAdapter, MAC_TX_RESERVED_FIELD + PUBLIC_ACTION_MAX_LEN);
	if (!prMsduInfo)
		return;
	prTxFrame = (struct ACTION_BTM_QUERY_FRAME
			     *)((unsigned long)(prMsduInfo->prPacket) +
				MAC_TX_RESERVED_FIELD);

	/* 2 Compose The Mac Header. */
	prTxFrame->u2FrameCtrl = MAC_FRAME_ACTION;
	COPY_MAC_ADDR(prTxFrame->aucDestAddr, prStaRec->aucMacAddr);
	COPY_MAC_ADDR(prTxFrame->aucSrcAddr, prBssInfo->aucOwnMacAddr);
	COPY_MAC_ADDR(prTxFrame->aucBSSID, prBssInfo->aucBSSID);
	prTxFrame->ucCategory = CATEGORY_WNM_ACTION;
	prTxFrame->ucAction = ACTION_WNM_BSS_TRANSITION_MANAGEMENT_QUERY;

	/* 3 Compose the frame body's frame. */
	prTxFrame->ucDialogToken = prBtmParam->ucDialogToken;
	prTxFrame->ucQueryReason = prBtmParam->ucQueryReason;
	if (prBtmParam->u2OurNeighborBssLen > 0) {
		kalMemCopy(prTxFrame->pucNeighborBss,
			   prBtmParam->pucOurNeighborBss,
			   prBtmParam->u2OurNeighborBssLen);
		kalMemFree(prBtmParam->pucOurNeighborBss, VIR_MEM_TYPE,
			   prBtmParam->u2OurNeighborBssLen);
		prBtmParam->u2OurNeighborBssLen = 0;
	}

	/* 4 Update information of MSDU_INFO_T */
	TX_SET_MMPDU(prAdapter, prMsduInfo, prStaRec->ucBssIndex,
		     prStaRec->ucIndex, WLAN_MAC_MGMT_HEADER_LEN,
		     WLAN_MAC_MGMT_HEADER_LEN + 4 +
			     prBtmParam->u2OurNeighborBssLen,
		     wnmBTMQueryTxDone, MSDU_RATE_MODE_AUTO);

	/* 5 Enqueue the frame to send this action frame. */
	nicTxEnqueueMsdu(prAdapter, prMsduInfo);
}				/* end of wnmComposeBTMQueryFrame() */

/*----------------------------------------------------------------------------*/
/*!
 *
 * \brief This routine is called to process the 802.11v BTM request.
 *
 *
 * \note
 *      Handle Rx mgmt request
 */
/*----------------------------------------------------------------------------*/
void wnmRecvBTMRequest(IN struct ADAPTER *prAdapter, IN struct SW_RFB *prSwRfb)
{
	struct ACTION_BTM_REQ_FRAME *prRxFrame = NULL;
	struct BSS_TRANSITION_MGT_PARAM_T *prBtmParam;
	uint8_t *pucOptInfo = NULL;
	uint8_t ucRequestMode = 0;
	uint16_t u2TmpLen = 0;
	uint16_t u2SubIElen = 0;
	struct MSG_AIS_BSS_TRANSITION_T *prMsg = NULL;
	enum WNM_AIS_BSS_TRANSITION eTransType = BSS_TRANSITION_NO_MORE_ACTION;
	uint8_t ucBssIndex = secGetBssIdxByRfb(prAdapter, prSwRfb);

	prRxFrame = (struct ACTION_BTM_REQ_FRAME *) prSwRfb->pvHeader;
	if (!prRxFrame)
		return;
	if (prSwRfb->u2PacketLen <
		OFFSET_OF(struct ACTION_BTM_REQ_FRAME, aucOptInfo)) {
		DBGLOG(WNM, WARN,
		       "BTM: Request frame length is less than a standard BTM frame\n");
		return;
	}
	prMsg = (struct MSG_AIS_BSS_TRANSITION_T *)cnmMemAlloc(
		prAdapter, RAM_TYPE_MSG,
		sizeof(struct MSG_AIS_BSS_TRANSITION_T));
	if (!prMsg) {
		DBGLOG(WNM, WARN, "BTM: Msg Hdr is NULL\n");
		return;
	}

	prBtmParam =
		aisGetBTMParam(prAdapter, ucBssIndex);
	kalMemZero(prMsg, sizeof(*prMsg));
	prBtmParam->ucRequestMode = prRxFrame->ucRequestMode;
	prMsg->ucToken = prRxFrame->ucDialogToken;
	prBtmParam->u2DisassocTimer = prRxFrame->u2DisassocTimer;
	prBtmParam->ucDialogToken = prRxFrame->ucDialogToken;
	pucOptInfo = &prRxFrame->aucOptInfo[0];
	ucRequestMode = prBtmParam->ucRequestMode;
	u2TmpLen = OFFSET_OF(struct ACTION_BTM_REQ_FRAME, aucOptInfo);
	if (ucRequestMode & BTM_REQ_MODE_DISC_IMM)
		eTransType = BSS_TRANSITION_REQ_ROAMING;
	if (ucRequestMode & BTM_REQ_MODE_BSS_TERM_INCLUDE) {
		struct SUB_IE_BSS_TERM_DURATION *prBssTermDuration =
			(struct SUB_IE_BSS_TERM_DURATION *)pucOptInfo;

		prBtmParam->u2TermDuration = prBssTermDuration->u2Duration;
		kalMemCopy(prBtmParam->aucTermTsf,
			   prBssTermDuration->aucTermTsf, 8);
		pucOptInfo += sizeof(*prBssTermDuration);
		u2TmpLen += sizeof(*prBssTermDuration);
		eTransType = BSS_TRANSITION_REQ_ROAMING;
	}
	if (ucRequestMode & BTM_REQ_MODE_ESS_DISC_IMM) {
		u2SubIElen = pucOptInfo[0];
		if (prSwRfb->u2PacketLen < u2TmpLen + u2SubIElen
				|| u2SubIElen > 255) {
			DBGLOG(WNM, WARN,
		       "BTM: Request frame length is less than a standard BTM frame\n");
			return;
		}
		kalMemCopy(prBtmParam->aucSessionURL, &pucOptInfo[1],
			   u2SubIElen);
		prBtmParam->ucSessionURLLen = u2SubIElen;
		u2TmpLen += u2SubIElen;
		pucOptInfo += u2SubIElen + 1;
		eTransType = BSS_TRANSITION_DISASSOC;
	}
	if (ucRequestMode & BTM_REQ_MODE_CAND_INCLUDED_BIT) {
		if (!(ucRequestMode & BTM_REQ_MODE_ESS_DISC_IMM))
			eTransType = BSS_TRANSITION_REQ_ROAMING;
		if (prSwRfb->u2PacketLen > u2TmpLen) {
			prMsg->u2CandListLen = prSwRfb->u2PacketLen - u2TmpLen;
			prMsg->pucCandList = pucOptInfo;
			prMsg->ucValidityInterval =
				prRxFrame->ucValidityInterval;
		} else
			DBGLOG(WNM, WARN,
			       "BTM: Candidate Include bit is set, but no candidate list\n");
	}

	DBGLOG(WNM, INFO,
	       "BTM: Req %d, VInt %d, DiscTimer %d, Token %d, TransType %d\n",
	       prBtmParam->ucRequestMode, prRxFrame->ucValidityInterval,
	       prBtmParam->u2DisassocTimer, prMsg->ucToken, eTransType);

	prMsg->eTransitionType = eTransType;
	prMsg->rMsgHdr.eMsgId = MID_WNM_AIS_BSS_TRANSITION;
	prMsg->ucBssIndex = ucBssIndex;
	/* if BTM Request is dest for broadcast, don't send BTM Response */
	if (kalMemCmp(prRxFrame->aucDestAddr, "\xff\xff\xff\xff\xff\xff",
		      MAC_ADDR_LEN))
		prMsg->fgNeedResponse = TRUE;
	else
		prMsg->fgNeedResponse = FALSE;
	mboxSendMsg(prAdapter, MBOX_ID_0, (struct MSG_HDR *)prMsg,
		    MSG_SEND_METHOD_BUF);
}

#if CFG_AP_80211V_SUPPORT
static void wnmMulAPAgentBTMRequestDisassocTimerFunc(
				IN struct ADAPTER *prAdapter, unsigned long ulParamPtr)
{
	struct STA_RECORD *prStaRec = NULL;
	struct BSS_INFO *prBssInfo = NULL;

	DBGLOG(WNM, INFO, "BTM: Request Disassociation Imminent\n");
	prStaRec = (struct STA_RECORD *) ulParamPtr;
	prBssInfo = GET_BSS_INFO_BY_INDEX(prAdapter, prStaRec->ucBssIndex);

	if (prStaRec->fgIsInUse)
		p2pFuncDisconnect(prAdapter,
			prBssInfo, prStaRec,
			TRUE,
			REASON_CODE_DISASSOC_INACTIVITY);
}

static uint32_t wnmMulAPAgentBTMRequestTxDone(IN struct ADAPTER *prAdapter,
				  IN struct MSDU_INFO *prMsduInfo,
				  IN enum ENUM_TX_RESULT_CODE rTxDoneStatus)
{
	struct ACTION_BTM_REQ_FRAME *prTxFrame = NULL;
	struct BSS_INFO *prBssInfo = NULL;
	struct STA_RECORD *prStaRec = NULL;
	int32_t u4DisassocTime = 0;

	DBGLOG(WNM, INFO,
		"BTM: Request Frame Tx Done, Status %d\n", rTxDoneStatus);
	if (rTxDoneStatus != TX_RESULT_SUCCESS)
		return WLAN_STATUS_FAILURE;

	prTxFrame = (struct ACTION_BTM_REQ_FRAME *)
		((unsigned long)(prMsduInfo->prPacket) +
				MAC_TX_RESERVED_FIELD);
	prBssInfo = GET_BSS_INFO_BY_INDEX(prAdapter, prMsduInfo->ucBssIndex);
	prStaRec = cnmGetStaRecByIndex(prAdapter, prMsduInfo->ucStaRecIndex);

	/* DisassocTimer */
	u4DisassocTime = prTxFrame->u2DisassocTimer;
	if ((prTxFrame->ucRequestMode & BIT(2)) && (u4DisassocTime != 0)) {
		u4DisassocTime *= prBssInfo->u2BeaconInterval;
		if (timerPendingTimer(&prStaRec->rBTMReqDisassocTimer)) {
			cnmTimerStopTimer(prAdapter,
				&prStaRec->rBTMReqDisassocTimer);
			DBGLOG(WNM, INFO, "BTM: update DisassocTimer\n");
		}
		cnmTimerInitTimer(prAdapter, &prStaRec->rBTMReqDisassocTimer,
				(PFN_MGMT_TIMEOUT_FUNC)
				wnmMulAPAgentBTMRequestDisassocTimerFunc,
				(unsigned long) prStaRec);
		cnmTimerStartTimer(prAdapter,
			&prStaRec->rBTMReqDisassocTimer, u4DisassocTime);
		DBGLOG(WNM, INFO,
			"BTM: disconnect after %d ms\n", u4DisassocTime);
	}

	return WLAN_STATUS_SUCCESS;
}


/*----------------------------------------------------------------------------*/
/*!
 * @brief This function will compose the BTM Request frame.
 *
 * @param[in] prAdapter              Pointer to the Adapter structure.
 * @param[in] prStaRec               Pointer to the STA_RECORD_T.
 * @param[in] prSetBtmReqInfo        Pointer to the PARAM_CUSTOM_BTM_REQ_STRUCT.
 *
 * @return (none)
 */
/*----------------------------------------------------------------------------*/
void wnmMulAPAgentSendBTMRequestFrame(
		IN struct ADAPTER *prAdapter,
		IN struct STA_RECORD *prStaRec,
		IN struct PARAM_CUSTOM_BTM_REQ_STRUCT *prSetBtmReqInfo)
{
	struct MSDU_INFO *prMsduInfo = NULL;
	struct BSS_INFO *prBssInfo = NULL;
	struct ACTION_BTM_REQ_FRAME *prTxFrame = NULL;
	struct IE_NEIGHBOR_REPORT *prNeighborRpt = NULL;
	struct SUB_IE_BSS_CAND_PREFERENCE *prCandidatePrefer = NULL;
	uint16_t u2TxFrameLen = 500;
	uint32_t u4UrlLen = 0;
	uint32_t u4SessionInfoLen = 0;
	uint16_t u2NeighborListLen = 0;
	uint8_t *prOptionInfo = NULL;
	uint8_t ucIndex = 0;

	prBssInfo = GET_BSS_INFO_BY_INDEX(prAdapter, prStaRec->ucBssIndex);
	if (!prBssInfo) {
		DBGLOG(WNM, INFO, "WNM: prBssInfo rec is NULL\n");
		return;
	}

	if (!prStaRec) {
		DBGLOG(WNM, INFO, "WNM: sta rec is NULL\n");
		return;
	}

	if (!prStaRec) {
		DBGLOG(WNM, INFO, "WNM: sta rec is NULL\n");
		return;
	}

	/* 1 Allocate MSDU Info */
	prMsduInfo = (struct MSDU_INFO *)cnmMgtPktAlloc(
		prAdapter, MAC_TX_RESERVED_FIELD + u2TxFrameLen);
	if (!prMsduInfo)
		return;
	prTxFrame = (struct ACTION_BTM_REQ_FRAME *)
		((unsigned long)(prMsduInfo->prPacket) +
				MAC_TX_RESERVED_FIELD);

	/* 2 Compose The Mac Header. */
	prTxFrame->u2FrameCtrl = MAC_FRAME_ACTION;
	COPY_MAC_ADDR(prTxFrame->aucDestAddr, prStaRec->aucMacAddr);
	COPY_MAC_ADDR(prTxFrame->aucSrcAddr, prBssInfo->aucOwnMacAddr);
	COPY_MAC_ADDR(prTxFrame->aucBSSID, prBssInfo->aucBSSID);
	prTxFrame->ucCategory = CATEGORY_WNM_ACTION;
	prTxFrame->ucAction = ACTION_WNM_BSS_TRANSITION_MANAGEMENT_REQ;

	/* 3 Compose the frame body's frame. */
	prTxFrame->ucDialogToken = wnmGetBtmToken();
	prTxFrame->ucRequestMode = 0;
	prTxFrame->u2DisassocTimer = prSetBtmReqInfo->u2DisassocTimer;
	prTxFrame->ucValidityInterval = prSetBtmReqInfo->ucValidityInterval;

	prOptionInfo = &prTxFrame->aucOptInfo[0];
	if (prSetBtmReqInfo->ucEssImm) {
		prTxFrame->ucRequestMode |= BIT(4);
		u4UrlLen = kalStrLen(prSetBtmReqInfo->aucSessionUrl);
		*prOptionInfo = u4UrlLen;
		prOptionInfo += 1;
		kalMemCopy(prOptionInfo,
			&prSetBtmReqInfo->aucSessionUrl, u4UrlLen);
		u4SessionInfoLen = u4UrlLen + 1;
		prOptionInfo += u4UrlLen;
	}

	for (ucIndex = 0; ucIndex < prSetBtmReqInfo->ucTargetBSSIDCnt;
			ucIndex++) {
		prTxFrame->ucRequestMode |= BIT(0);
		prNeighborRpt = (struct IE_NEIGHBOR_REPORT *) prOptionInfo;
		prNeighborRpt->ucId = ELEM_ID_NEIGHBOR_REPORT;
		prNeighborRpt->ucLength = 6 + 4 + 1 + 1 + 1;
		COPY_MAC_ADDR(prNeighborRpt->aucBSSID,
			prSetBtmReqInfo->ucTargetBSSIDList[ucIndex].mMac);
		prNeighborRpt->u4BSSIDInfo =
			prSetBtmReqInfo->ucTargetBSSIDList[ucIndex].u4BSSIDInfo;
		prNeighborRpt->ucOperClass =
			prSetBtmReqInfo->ucTargetBSSIDList[ucIndex].ucOperClass;
		prNeighborRpt->ucChnlNumber =
			prSetBtmReqInfo->ucTargetBSSIDList[ucIndex].ucChannel;
		prNeighborRpt->ucPhyType =
			prSetBtmReqInfo->ucTargetBSSIDList[ucIndex].ucPhyType;

		/* subelement */
		prCandidatePrefer = (struct SUB_IE_BSS_CAND_PREFERENCE *)
			&prNeighborRpt->aucSubElem[0];
		prCandidatePrefer->ucSubId =
			ELEM_ID_NR_BSS_TRANSITION_CAND_PREF;
		prCandidatePrefer->ucLength = 1;
		prCandidatePrefer->ucPreference =
			prSetBtmReqInfo
			->ucTargetBSSIDList[ucIndex].ucPreference;
		prNeighborRpt->ucLength += (2 + prCandidatePrefer->ucLength);

		prOptionInfo += (2 + prNeighborRpt->ucLength);
		u2NeighborListLen += (2 + prNeighborRpt->ucLength);
	}

	if (prSetBtmReqInfo->ucAbridged)
		prTxFrame->ucRequestMode |= BIT(1);

	if (prTxFrame->u2DisassocTimer != 0)
		prTxFrame->ucRequestMode |= BIT(2);

	/* 4 Update information of MSDU_INFO_T */
	TX_SET_MMPDU(prAdapter, prMsduInfo, prStaRec->ucBssIndex,
		     prStaRec->ucIndex, WLAN_MAC_MGMT_HEADER_LEN,
		     WLAN_MAC_MGMT_HEADER_LEN + 7
		     + u4SessionInfoLen + u2NeighborListLen,
		     wnmMulAPAgentBTMRequestTxDone,
		     MSDU_RATE_MODE_AUTO);

	/* 5 Enqueue the frame to send this action frame. */
	nicTxEnqueueMsdu(prAdapter, prMsduInfo);
}				/* end of wnmComposeBTMQueryFrame() */

/*----------------------------------------------------------------------------*/
/*!
 *
 * \brief This routine is called to process the 802.11v BTM request.
 *
 *
 * \note
 *      Handle Rx mgmt request
 */
/*----------------------------------------------------------------------------*/
void wnmMulAPAgentRecvBTMResponse(IN struct ADAPTER *prAdapter,
		IN struct SW_RFB *prSwRfb)
{
	struct ACTION_BTM_RSP_FRAME *prRxFrame = NULL;
	uint8_t *pucOptInfo = NULL;
	uint16_t u2TmpLen = 0;
	int32_t i4Ret = 0;
	struct T_MULTI_AP_STA_STEERING_REPORT *prBtmReport = NULL;

	prRxFrame = (struct ACTION_BTM_RSP_FRAME *) prSwRfb->pvHeader;
	if (!prRxFrame)
		return;
	if (prSwRfb->u2PacketLen <
		OFFSET_OF(struct ACTION_BTM_RSP_FRAME, aucOptInfo)) {
		DBGLOG(WNM, WARN,
		       "BTM: Request frame length is less than a standard BTM frame\n");
		return;
	}

	prBtmReport = (struct T_MULTI_AP_STA_STEERING_REPORT *)
			kalMemAlloc(sizeof(
				struct T_MULTI_AP_STA_STEERING_REPORT),
			VIR_MEM_TYPE);
	if (prBtmReport == NULL) {
		DBGLOG(INIT, ERROR, "alloc memory fail\n");
		return;
	}

	kalMemZero(prBtmReport, sizeof(struct T_MULTI_AP_STA_STEERING_REPORT));
	COPY_MAC_ADDR(prBtmReport->mStaMac, prRxFrame->aucSrcAddr);
	COPY_MAC_ADDR(prBtmReport->mBssid, prRxFrame->aucBSSID);

	pucOptInfo = &prRxFrame->aucOptInfo[0];
	u2TmpLen = OFFSET_OF(struct ACTION_BTM_RSP_FRAME, aucOptInfo);

	prBtmReport->u8Status = prRxFrame->ucStatusCode;

	DBGLOG(WNM, INFO,
		"[SAP_Test] u2FrameCtrl = 0x%x\n", prRxFrame->u2FrameCtrl);
	DBGLOG(WNM, INFO,
		"[SAP_Test] u2Duration = %u\n", prRxFrame->u2Duration);
	DBGLOG(WNM, INFO,
		"[SAP_Test] aucDestAddr = " MACSTR "\n",
		MAC2STR(prRxFrame->aucDestAddr));
	DBGLOG(WNM, INFO,
		"[SAP_Test] aucSrcAddr = " MACSTR "\n",
		MAC2STR(prRxFrame->aucSrcAddr));
	DBGLOG(WNM, INFO,
		"[SAP_Test] aucBSSID = " MACSTR "\n",
		MAC2STR(prRxFrame->aucBSSID));
	DBGLOG(WNM, INFO,
		"[SAP_Test] u2SeqCtrl = %u\n", prRxFrame->u2SeqCtrl);
	DBGLOG(WNM, INFO,
		"[SAP_Test] ucCategory = %u\n", prRxFrame->ucCategory);
	DBGLOG(WNM, INFO,
		"[SAP_Test] ucAction = %u\n", prRxFrame->ucAction);
	DBGLOG(WNM, INFO,
		"[SAP_Test] ucDialogToken = %u\n", prRxFrame->ucDialogToken);
	DBGLOG(WNM, INFO,
		"[SAP_Test] ucStatusCode = %u\n", prRxFrame->ucStatusCode);
	DBGLOG(WNM, INFO,
		"[SAP_Test] ucBssTermDelay = %u\n", prRxFrame->ucBssTermDelay);
	if (prRxFrame->ucStatusCode == BSS_TRANSITION_MGT_STATUS_ACCEPT) {
		COPY_MAC_ADDR(prBtmReport->mDestBssid, pucOptInfo);
		pucOptInfo += MAC_ADDR_LEN;
		u2TmpLen += MAC_ADDR_LEN;
	}
	DBGLOG(WNM, INFO,
			"[SAP_Test] Target BSSID = " MACSTR "\n",
			MAC2STR(prBtmReport->mDestBssid));

	/* BSS Transition Candidate List Entries */
	while (prSwRfb->u2PacketLen > u2TmpLen) {
		DBGLOG_MEM8(WNM, INFO, pucOptInfo, pucOptInfo[1] + 2);
		pucOptInfo += pucOptInfo[1] + 2;
		u2TmpLen += pucOptInfo[1] + 2;
	}

	i4Ret = MulAPAgentMontorSendMsg(
		EV_WLAN_MULTIAP_STEERING_BTM_REPORT,
		prBtmReport, sizeof(*prBtmReport));
	if (i4Ret < 0)
		DBGLOG(AAA, ERROR,
			"EV_WLAN_MULTIAP_STEERING_BTM_REPORT nl send msg failed!\n");

	kalMemFree(prBtmReport,
		VIR_MEM_TYPE,
		sizeof(struct T_MULTI_AP_STA_STEERING_REPORT));
}
#endif /* CFG_AP_80211V_SUPPORT */

