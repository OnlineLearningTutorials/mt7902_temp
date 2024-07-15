/* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause */
/*
 * Copyright (c) 2017 MediaTek Inc.
 */
/*! \file   "twt_req_fsm.c"
*   \brief  FSM for TWT Requesting STA negotiation
*/


/*******************************************************************************
*                         C O M P I L E R   F L A G S
********************************************************************************
*/

/*******************************************************************************
*                    E X T E R N A L   R E F E R E N C E S
********************************************************************************
*/
#include "precomp.h"

/*******************************************************************************
*                              C O N S T A N T S
********************************************************************************
*/

/*******************************************************************************
*                             D A T A   T Y P E S
********************************************************************************
*/

/*******************************************************************************
*                            P U B L I C   D A T A
********************************************************************************
*/

/*******************************************************************************
*                           P R I V A T E   D A T A
********************************************************************************
*/

static uint8_t *apucDebugTWTReqState[TWT_REQ_STATE_NUM] = {
	(uint8_t *) DISP_STRING("TWT_REQ_STATE_IDLE"),
	(uint8_t *) DISP_STRING("TWT_REQ_STATE_REQTX"),
	(uint8_t *) DISP_STRING("TWT_REQ_STATE_WAIT_RSP"),
	(uint8_t *) DISP_STRING("TWT_REQ_STATE_SUSPENDING"),
	(uint8_t *) DISP_STRING("TWT_REQ_STATE_SUSPENDED"),
	(uint8_t *) DISP_STRING("TWT_REQ_STATE_RESUMING"),
	(uint8_t *) DISP_STRING("TWT_REQ_STATE_TEARING_DOWN"),
};

#if (CFG_SUPPORT_TWT_HOTSPOT == 1)
static uint8_t *apucDebugTWTRespState[TWT_HOTSPOT_RESP_STATE_NUM] = {
	(uint8_t *) DISP_STRING("TWT_HOTSPOT_RESP_STATE_IDLE"),
	(uint8_t *) DISP_STRING("TWT_HOTSPOT_RESP_STATE_RECEIVE_SETUP"),
	(uint8_t *) DISP_STRING("TWT_HOTSPOT_RESP_STATE_SETUP_RESPONSE"),
	(uint8_t *) DISP_STRING("TWT_HOTSPOT_RESP_STATE_RECEIVE_TEARDOWN"),
	(uint8_t *) DISP_STRING("TWT_HOTSPOT_RESP_STATE_SEND_TEARDOWN_TO_STA"),
	(uint8_t *) DISP_STRING("TWT_HOTSPOT_RESP_STATE_DISCONNECT"),
	(uint8_t *) DISP_STRING("TWT_HOTSPOT_RESP_STATE_IDLE_BY_FORCE")
};
#endif

/*******************************************************************************
*                                 M A C R O S
********************************************************************************
*/

/*******************************************************************************
*                   F U N C T I O N   D E C L A R A T I O N S
********************************************************************************
*/
static uint32_t
twtReqFsmSendEvent(
	struct ADAPTER *prAdapter,
	struct STA_RECORD *prStaRec,
	uint8_t ucTWTFlowId,
	enum ENUM_MSG_ID eMsgId);

static uint32_t
twtReqFsmSendEventRxInfoFrm(
	struct ADAPTER *prAdapter,
	struct STA_RECORD *prStaRec,
	uint8_t ucTWTFlowId,
	struct _NEXT_TWT_INFO_T *prNextTWTInfo);

/*******************************************************************************
*                              F U N C T I O N S
********************************************************************************
*/

/*----------------------------------------------------------------------------*/
/*!
* @brief The Core FSM engine of TWT Requester Module.
*
* @param[in] prStaRec           Pointer to the STA_RECORD_T
* @param[in] eNextState         The value of Next State
* @param[in] prRetainedSwRfb     SW_RFB_T for JOIN Success
*
* @return (none)
*/
/*----------------------------------------------------------------------------*/
void
twtReqFsmSteps(
	struct ADAPTER *prAdapter,
	struct STA_RECORD *prStaRec,
	enum _ENUM_TWT_REQUESTER_STATE_T eNextState,
	uint8_t ucTWTFlowId,
	void *pParam)
{
	uint32_t rStatus;
	enum _ENUM_TWT_REQUESTER_STATE_T ePreState;
	uint8_t fgIsTransition;

	ASSERT(prAdapter);
	ASSERT(prStaRec);

	do {
		if (((uint32_t)prStaRec->aeTWTReqState >= TWT_REQ_STATE_NUM) ||
		    ((uint32_t)eNextState >= TWT_REQ_STATE_NUM)) {
			DBGLOG(TWT_RESPONDER, ERROR,
			       "Invalid State: [%d] -> [%d]\n",
			       (uint32_t)prStaRec->aeTWTReqState,
			       (uint32_t)eNextState);
			return;
		}

		DBGLOG(TWT_REQUESTER, STATE,
		"[TWT_REQ] Flow %d TRANSITION: [%s] -> [%s]\n",
		ucTWTFlowId,
		apucDebugTWTReqState[(uint32_t)prStaRec->aeTWTReqState],
		apucDebugTWTReqState[(uint32_t)eNextState]);

		ePreState = prStaRec->aeTWTReqState;

		prStaRec->aeTWTReqState = eNextState;
		fgIsTransition = (uint8_t) FALSE;

		switch (prStaRec->aeTWTReqState) {
		case TWT_REQ_STATE_IDLE:
			/* Notify TWT Planner of the negotiation result */
			if (ePreState == TWT_REQ_STATE_WAIT_RSP) {
				twtReqFsmSendEvent(prAdapter, prStaRec,
					ucTWTFlowId, MID_TWT_REQ_IND_RESULT);
				/* TODO: how to handle failures */
			} else if (ePreState == TWT_REQ_STATE_TEARING_DOWN) {
				twtReqFsmSendEvent(prAdapter, prStaRec,
					ucTWTFlowId,
					MID_TWT_REQ_IND_TEARDOWN_DONE);
			} else if (ePreState == TWT_REQ_STATE_RESUMING) {
				twtReqFsmSendEvent(prAdapter, prStaRec,
					ucTWTFlowId,
					MID_TWT_REQ_IND_RESUME_DONE);
			}
			break;

		case TWT_REQ_STATE_REQTX:
		{
			struct _TWT_PARAMS_T *prTWTParams =
				(struct _TWT_PARAMS_T *)pParam;
			ASSERT(prTWTParams);
			rStatus = twtSendSetupFrame(
				prAdapter, prStaRec, ucTWTFlowId,
				prTWTParams, twtReqFsmRunEventTxDone);
			if (rStatus != WLAN_STATUS_SUCCESS) {
				eNextState = TWT_REQ_STATE_IDLE;
				fgIsTransition = TRUE;
			}
			break;
		}

		case TWT_REQ_STATE_WAIT_RSP:
			break;

		case TWT_REQ_STATE_TEARING_DOWN:
			rStatus = twtSendTeardownFrame(
				prAdapter, prStaRec, ucTWTFlowId,
				twtReqFsmRunEventTxDone);
			if (rStatus != WLAN_STATUS_SUCCESS) {
				eNextState = TWT_REQ_STATE_IDLE;
				fgIsTransition = TRUE;
			}
			break;

		case TWT_REQ_STATE_SUSPENDING:
		{
			struct _NEXT_TWT_INFO_T rNextTWTInfo = {0};

			rStatus = twtSendInfoFrame(
				prAdapter, prStaRec, ucTWTFlowId, &rNextTWTInfo,
				twtReqFsmRunEventTxDone);
			if (rStatus != WLAN_STATUS_SUCCESS) {
				eNextState = TWT_REQ_STATE_IDLE;
				fgIsTransition = TRUE;
			}
			break;
		}

		case TWT_REQ_STATE_RESUMING:
		{
			struct _NEXT_TWT_INFO_T *prNextTWTInfo =
				(struct _NEXT_TWT_INFO_T *)pParam;
			rStatus = twtSendInfoFrame(
				prAdapter, prStaRec, ucTWTFlowId, prNextTWTInfo,
				twtReqFsmRunEventTxDone);
			if (rStatus != WLAN_STATUS_SUCCESS) {
				eNextState = TWT_REQ_STATE_IDLE;
				fgIsTransition = TRUE;
			}

			break;
		}

		case TWT_REQ_STATE_SUSPENDED:
			twtReqFsmSendEvent(prAdapter, prStaRec,
				ucTWTFlowId, MID_TWT_REQ_IND_SUSPEND_DONE);
			break;

		case TWT_REQ_STATE_RX_TEARDOWN:
			twtReqFsmSendEvent(prAdapter, prStaRec,
				ucTWTFlowId, MID_TWT_REQ_IND_TEARDOWN_DONE);
			break;

		case TWT_REQ_STATE_RX_INFOFRM:
		{
			struct _NEXT_TWT_INFO_T *prNextTWTInfo =
				(struct _NEXT_TWT_INFO_T *)pParam;
			twtReqFsmSendEventRxInfoFrm(prAdapter, prStaRec,
				ucTWTFlowId, prNextTWTInfo);
			break;
		}

		default:
			DBGLOG(TWT_REQUESTER, ERROR,
				"Unknown TWT_REQUESTER STATE\n");
			ASSERT(0);
			break;
		}

	} while (fgIsTransition);
}

static uint32_t
twtReqFsmSendEvent(
	struct ADAPTER *prAdapter,
	struct STA_RECORD *prStaRec,
	uint8_t ucTWTFlowId,
	enum ENUM_MSG_ID eMsgId)
{
	struct _MSG_TWT_REQFSM_IND_RESULT_T *prTWTFsmResultMsg;

	prTWTFsmResultMsg = cnmMemAlloc(prAdapter, RAM_TYPE_MSG,
		sizeof(struct _MSG_TWT_REQFSM_IND_RESULT_T));
	if (prTWTFsmResultMsg) {
		prTWTFsmResultMsg->rMsgHdr.eMsgId = eMsgId;
		prTWTFsmResultMsg->prStaRec = prStaRec;
		prTWTFsmResultMsg->ucTWTFlowId = ucTWTFlowId;

		mboxSendMsg(prAdapter,
			MBOX_ID_0,
			(struct MSG_HDR *) prTWTFsmResultMsg,
			MSG_SEND_METHOD_BUF);
	} else
		return WLAN_STATUS_RESOURCES;

	return WLAN_STATUS_SUCCESS;
}

static uint32_t
twtReqFsmSendEventRxInfoFrm(
	struct ADAPTER *prAdapter,
	struct STA_RECORD *prStaRec,
	uint8_t ucTWTFlowId,
	struct _NEXT_TWT_INFO_T *prNextTWTInfo)
{
	struct _MSG_TWT_REQFSM_IND_INFOFRM_T *prTWTFsmInfoFrmMsg;

	prTWTFsmInfoFrmMsg = cnmMemAlloc(prAdapter, RAM_TYPE_MSG,
		sizeof(struct _MSG_TWT_REQFSM_IND_INFOFRM_T));
	if (prTWTFsmInfoFrmMsg) {
		prTWTFsmInfoFrmMsg->rMsgHdr.eMsgId = MID_TWT_REQ_IND_INFOFRM;
		prTWTFsmInfoFrmMsg->prStaRec = prStaRec;
		prTWTFsmInfoFrmMsg->ucTWTFlowId = ucTWTFlowId;
		kalMemCopy(&(prTWTFsmInfoFrmMsg->rNextTWTInfo), prNextTWTInfo,
			sizeof(struct _NEXT_TWT_INFO_T));

		mboxSendMsg(prAdapter, MBOX_ID_0,
			(struct MSG_HDR *) prTWTFsmInfoFrmMsg,
			MSG_SEND_METHOD_BUF);
	} else
		return WLAN_STATUS_RESOURCES;

	return WLAN_STATUS_SUCCESS;
}

/*----------------------------------------------------------------------------*/
/*!
* @brief This function will handle the Start Event to TWT FSM.
*
* @param[in] prMsgHdr   Message of Request for a particular STA.
*
* @return (none)
*/
/*----------------------------------------------------------------------------*/
void twtReqFsmRunEventStart(
	struct ADAPTER *prAdapter,
	struct MSG_HDR *prMsgHdr)
{
	struct _MSG_TWT_REQFSM_START_T *prTWTReqFsmStartMsg;
	struct STA_RECORD *prStaRec;
	struct _TWT_PARAMS_T *prTWTParams;
	uint8_t ucTWTFlowId;

	ASSERT(prAdapter);
	ASSERT(prMsgHdr);

	prTWTReqFsmStartMsg = (struct _MSG_TWT_REQFSM_START_T *) prMsgHdr;
	prStaRec = prTWTReqFsmStartMsg->prStaRec;
	ucTWTFlowId = prTWTReqFsmStartMsg->ucTWTFlowId;
	prTWTParams = &(prStaRec->arTWTFlow[ucTWTFlowId].rTWTParams);

	if ((!prStaRec) || (prStaRec->fgIsInUse == FALSE)) {
		cnmMemFree(prAdapter, prMsgHdr);
		return;
	}

	ASSERT(prStaRec);
	ASSERT(prTWTParams);

	DBGLOG(TWT_REQUESTER, LOUD,
		"EVENT-START: TWT Requester FSM %d\n", ucTWTFlowId);

	cnmMemFree(prAdapter, prMsgHdr);

	/* Validation of TWT Requester Start Event */
	if (!IS_AP_STA(prStaRec)) {
		DBGLOG(TWT_REQUESTER, ERROR,
			"EVENT-START: Invalid Type %d\n",
			prStaRec->eStaType);

		/* TODO: Notify TWT Planner */

		return;
	}

	twtReqFsmSteps(prAdapter, prStaRec,
		TWT_REQ_STATE_REQTX, ucTWTFlowId, prTWTParams);
}

/*----------------------------------------------------------------------------*/
/*!
* @brief This function will handle the Teardown Event to TWT FSM.
*
* @param[in] prMsgHdr   Message of Request for a particular STA.
*
* @return (none)
*/
/*----------------------------------------------------------------------------*/
void twtReqFsmRunEventTeardown(
	struct ADAPTER *prAdapter, struct MSG_HDR *prMsgHdr)
{
	struct _MSG_TWT_REQFSM_TEARDOWN_T *prTWTReqFsmTeardownMsg;
	struct STA_RECORD *prStaRec;
	uint8_t ucTWTFlowId;

	ASSERT(prAdapter);
	ASSERT(prMsgHdr);

	prTWTReqFsmTeardownMsg = (struct _MSG_TWT_REQFSM_TEARDOWN_T *) prMsgHdr;
	prStaRec = prTWTReqFsmTeardownMsg->prStaRec;
	ucTWTFlowId = prTWTReqFsmTeardownMsg->ucTWTFlowId;

	if ((!prStaRec) || (prStaRec->fgIsInUse == FALSE)) {
		cnmMemFree(prAdapter, prMsgHdr);
		return;
	}

	ASSERT(prStaRec);

	DBGLOG(TWT_REQUESTER, LOUD, "EVENT-TEARDOWN: TWT Requester FSM %d\n",
		ucTWTFlowId);

	cnmMemFree(prAdapter, prMsgHdr);

	/* Validation of TWT Requester Teardown Event */
	if (!IS_AP_STA(prStaRec)) {
		DBGLOG(TWT_REQUESTER, ERROR, "Invalid STA Type %d\n",
			prStaRec->eStaType);

		/* TODO: Notify TWT Planner */

		return;
	}

	twtReqFsmSteps(prAdapter, prStaRec, TWT_REQ_STATE_TEARING_DOWN,
		ucTWTFlowId, NULL);
}

/*----------------------------------------------------------------------------*/
/*!
* @brief This function will handle the Suspend Event to TWT FSM.
*
* @param[in] prMsgHdr   Message of Request for a particular STA.
*
* @return (none)
*/
/*----------------------------------------------------------------------------*/
void twtReqFsmRunEventSuspend(
	struct ADAPTER *prAdapter, struct MSG_HDR *prMsgHdr)
{
	struct _MSG_TWT_REQFSM_SUSPEND_T *prTWTReqFsmSuspendMsg;
	struct STA_RECORD *prStaRec;
	uint8_t ucTWTFlowId;

	ASSERT(prAdapter);
	ASSERT(prMsgHdr);

	prTWTReqFsmSuspendMsg = (struct _MSG_TWT_REQFSM_SUSPEND_T *) prMsgHdr;
	prStaRec = prTWTReqFsmSuspendMsg->prStaRec;
	ucTWTFlowId = prTWTReqFsmSuspendMsg->ucTWTFlowId;

	if ((!prStaRec) || (prStaRec->fgIsInUse == FALSE)) {
		cnmMemFree(prAdapter, prMsgHdr);
		return;
	}

	ASSERT(prStaRec);

	DBGLOG(TWT_REQUESTER, LOUD, "EVENT-SUSPEND: TWT Requester FSM %d\n",
		ucTWTFlowId);

	cnmMemFree(prAdapter, prMsgHdr);

	/* Validation of TWT Requester Suspend Event */
	if (!IS_AP_STA(prStaRec)) {
		DBGLOG(TWT_REQUESTER, ERROR, "Invalid STA Type %d\n",
			prStaRec->eStaType);

		/* TODO: Notify TWT Planner */

		return;
	}

	twtReqFsmSteps(prAdapter, prStaRec, TWT_REQ_STATE_SUSPENDING,
		ucTWTFlowId, NULL);
}

/*----------------------------------------------------------------------------*/
/*!
* @brief This function will handle the Suspend Event to TWT FSM.
*
* @param[in] prMsgHdr   Message of Request for a particular STA.
*
* @return (none)
*/
/*----------------------------------------------------------------------------*/
void twtReqFsmRunEventResume(
	struct ADAPTER *prAdapter, struct MSG_HDR *prMsgHdr)
{
	struct _MSG_TWT_REQFSM_RESUME_T *prTWTReqFsmResumeMsg;
	struct STA_RECORD *prStaRec;
	uint8_t ucTWTFlowId;
	struct _NEXT_TWT_INFO_T rNextTWTInfo;

	ASSERT(prAdapter);
	ASSERT(prMsgHdr);

	prTWTReqFsmResumeMsg = (struct _MSG_TWT_REQFSM_RESUME_T *) prMsgHdr;
	prStaRec = prTWTReqFsmResumeMsg->prStaRec;
	ucTWTFlowId = prTWTReqFsmResumeMsg->ucTWTFlowId;
	rNextTWTInfo.u8NextTWT = prTWTReqFsmResumeMsg->u8NextTWT;
	rNextTWTInfo.ucNextTWTSize = prTWTReqFsmResumeMsg->ucNextTWTSize;

	if ((!prStaRec) || (prStaRec->fgIsInUse == FALSE)) {
		cnmMemFree(prAdapter, prMsgHdr);
		return;
	}

	ASSERT(prStaRec);

	DBGLOG(TWT_REQUESTER, LOUD, "EVENT-RESUME: TWT Requester FSM %d\n",
		ucTWTFlowId);

	cnmMemFree(prAdapter, prMsgHdr);

	/* Validation of TWT Requester Teardown Event */
	if (!IS_AP_STA(prStaRec)) {
		DBGLOG(TWT_REQUESTER, ERROR, "Invalid STA Type %d\n",
			prStaRec->eStaType);

		/* TODO: Notify TWT Planner */

		return;
	}

	twtReqFsmSteps(prAdapter, prStaRec, TWT_REQ_STATE_RESUMING,
		ucTWTFlowId, (void *)&rNextTWTInfo);
}

uint32_t
twtReqFsmRunEventTxDone(
	struct ADAPTER *prAdapter,
	struct MSDU_INFO *prMsduInfo,
	enum ENUM_TX_RESULT_CODE rTxDoneStatus)
{
	struct STA_RECORD *prStaRec;
	enum _ENUM_TWT_REQUESTER_STATE_T eNextState;
	uint8_t ucTWTFlowId;

	ASSERT(prMsduInfo);

	prStaRec = cnmGetStaRecByIndex(prAdapter, prMsduInfo->ucStaRecIndex);
	if (!prStaRec) {
		DBGLOG(TWT_REQUESTER, ERROR,
			"EVENT-TXDONE: No valid STA Record\n");
		return WLAN_STATUS_INVALID_PACKET;
	}

	if (rTxDoneStatus)
		DBGLOG(TWT_REQUESTER, INFO,
			"EVENT-TX DONE [status: %d][seq: %d]: Current Time = %d\n",
		   rTxDoneStatus, prMsduInfo->ucTxSeqNum, kalGetTimeTick());

	/* Next state is set to current state
	 *by default and check Tx done status to transition if possible
	 */
	eNextState = prStaRec->aeTWTReqState;

	switch (prStaRec->aeTWTReqState) {
	case TWT_REQ_STATE_REQTX:

		if (rTxDoneStatus == TX_RESULT_SUCCESS)
			eNextState = TWT_REQ_STATE_WAIT_RSP;
		else
			eNextState = TWT_REQ_STATE_IDLE;

		ucTWTFlowId = twtGetTxSetupFlowId(prMsduInfo);
		twtReqFsmSteps(prAdapter,
			prStaRec, eNextState, ucTWTFlowId, NULL);

		break;

	case TWT_REQ_STATE_TEARING_DOWN:

		if (rTxDoneStatus == TX_RESULT_SUCCESS)
			eNextState = TWT_REQ_STATE_IDLE;

		ucTWTFlowId = twtGetTxTeardownFlowId(prMsduInfo);
		twtReqFsmSteps(prAdapter, prStaRec, eNextState,
			ucTWTFlowId, NULL);

		break;

	case TWT_REQ_STATE_SUSPENDING:
		if (rTxDoneStatus == TX_RESULT_SUCCESS)
			eNextState = TWT_REQ_STATE_SUSPENDED;

		ucTWTFlowId = twtGetTxInfoFlowId(prMsduInfo);
		twtReqFsmSteps(prAdapter, prStaRec, eNextState,
			ucTWTFlowId, NULL);

		break;

	case TWT_REQ_STATE_RESUMING:
		if (rTxDoneStatus == TX_RESULT_SUCCESS)
			eNextState = TWT_REQ_STATE_IDLE;

		ucTWTFlowId = twtGetTxInfoFlowId(prMsduInfo);
		twtReqFsmSteps(prAdapter, prStaRec, eNextState,
			ucTWTFlowId, NULL);

		break;

	default:
		break;		/* Ignore other cases */
	}

	return WLAN_STATUS_SUCCESS;
}

void twtReqFsmRunEventRxSetup(
	struct ADAPTER *prAdapter,
	struct SW_RFB *prSwRfb,
	struct STA_RECORD *prStaRec,
	uint8_t ucTWTFlowId)
{
#if (CFG_KEEPFULLPOWER_BEFORE_TWT_NEGO == 1)
	uint32_t rStatus = WLAN_STATUS_SUCCESS;
	uint32_t u4BufLen = 0;
	struct PARAM_CUSTOM_CHIP_CONFIG_STRUCT rChipConfigInfo = {0};
	struct GLUE_INFO *prGlueInfo = NULL;
#endif
	if (!IS_AP_STA(prStaRec))
		return;

	switch (prStaRec->aeTWTReqState) {
	case TWT_REQ_STATE_WAIT_RSP:
		/* transition to the IDLE state */
		twtReqFsmSteps(prAdapter,
			prStaRec, TWT_REQ_STATE_IDLE, ucTWTFlowId, NULL);

#if (CFG_KEEPFULLPOWER_BEFORE_TWT_NEGO == 1)
		/*
		 * WiFi 6E Cert 5.60.2 issue
		 * AP will send accept twt action frame
		 * when STA sleep then cause TWT nego fail
		 * Keep full power before TWT nego to
		 * let STAUT can rx the action frame,
		 * and restore KeepFullPwr setting here
		 */
		prGlueInfo = prAdapter->prGlueInfo;

		if (!prGlueInfo) {
			DBGLOG(TWT_REQUESTER, ERROR,
			"prGlueInfo is null\n");
			return;
		}

		rChipConfigInfo.ucType = CHIP_CONFIG_TYPE_WO_RESPONSE;
		rChipConfigInfo.u2MsgSize = 13;

		if (prGlueInfo->ucKeepFullPwrBackup == 1) {
			kalStrnCpy(rChipConfigInfo.aucCmd, "KeepFullPwr 1",
			   CHIP_CONFIG_RESP_SIZE - 1);
		} else {
			kalStrnCpy(rChipConfigInfo.aucCmd, "KeepFullPwr 0",
			   CHIP_CONFIG_RESP_SIZE - 1);
		}

		rChipConfigInfo.aucCmd[CHIP_CONFIG_RESP_SIZE - 1] = '\0';

		rStatus = kalIoctl(prGlueInfo, wlanoidSetChipConfig,
				   &rChipConfigInfo, sizeof(rChipConfigInfo),
				   FALSE, FALSE, TRUE, &u4BufLen);
#endif
		break;

	default:
		break;		/* Ignore other cases */
	}
}

void twtReqFsmRunEventRxTeardown(
	struct ADAPTER *prAdapter,
	struct SW_RFB *prSwRfb,
	struct STA_RECORD *prStaRec,
	uint8_t ucTWTFlowId)
{
	if (!IS_AP_STA(prStaRec))
		return;

	switch (prStaRec->aeTWTReqState) {
	case TWT_REQ_STATE_IDLE:
		/* transition to the RX TEARDOWN state */
		twtReqFsmSteps(prAdapter, prStaRec, TWT_REQ_STATE_RX_TEARDOWN,
			ucTWTFlowId, NULL);
		break;

	default:
		break;		/* Ignore other cases */
	}
}

void twtReqFsmRunEventRxInfoFrm(
	struct ADAPTER *prAdapter,
	struct SW_RFB *prSwRfb,
	struct STA_RECORD *prStaRec,
	uint8_t ucTWTFlowId,
	struct _NEXT_TWT_INFO_T *prNextTWTInfo)
{
	if (!IS_AP_STA(prStaRec))
		return;

	switch (prStaRec->aeTWTReqState) {
	case TWT_REQ_STATE_IDLE:
		/* transition to the RX Info frame state */
		twtReqFsmSteps(prAdapter, prStaRec, TWT_REQ_STATE_RX_INFOFRM,
			ucTWTFlowId, prNextTWTInfo);
		break;

	default:
		break;		/* Ignore other cases */
	}
}

#if (CFG_SUPPORT_TWT_HOTSPOT == 1)
uint32_t
twtHotspotRespFsmSendEvent(
	struct ADAPTER *prAdapter,
	struct STA_RECORD *prStaRec,
	uint8_t ucTWTFlowId,
	enum ENUM_MSG_ID eMsgId)
{
	struct _MSG_TWT_HOTSPOT_PARAMS_SET_T *prTWTHotspotParamSetMsg = NULL;

	prTWTHotspotParamSetMsg =
		cnmMemAlloc(
			prAdapter,
			RAM_TYPE_MSG,
			sizeof(struct _MSG_TWT_HOTSPOT_PARAMS_SET_T));

	if (prTWTHotspotParamSetMsg) {
		prTWTHotspotParamSetMsg->rMsgHdr.eMsgId = eMsgId;
		prTWTHotspotParamSetMsg->rTWTCtrl.prStaRec = prStaRec;
		prTWTHotspotParamSetMsg->rTWTCtrl.ucTWTFlowId = ucTWTFlowId;

		mboxSendMsg(prAdapter,
			MBOX_ID_0,
			(struct MSG_HDR *) prTWTHotspotParamSetMsg,
			MSG_SEND_METHOD_BUF);
	} else
		return WLAN_STATUS_RESOURCES;

	return WLAN_STATUS_SUCCESS;
}

void
twtHotspotRespFsmSteps(
	struct ADAPTER *prAdapter,
	struct STA_RECORD *prStaRec,
	enum _ENUM_TWT_HOTSPOT_RESPONDER_STATE_T eNextState,
	uint8_t ucTWTFlowId,
	void *pParam)
{
	uint32_t rStatus;
	enum _ENUM_TWT_HOTSPOT_RESPONDER_STATE_T ePreState;
	uint8_t fgIsTransition;
	struct BSS_INFO *prBssInfo;

	prBssInfo = GET_BSS_INFO_BY_INDEX(prAdapter, prStaRec->ucBssIndex);

	ASSERT(prBssInfo);
	ASSERT(prAdapter);
	ASSERT(prStaRec);

	do {
		if ((uint32_t)(prBssInfo->aeTWTRespState) >=
			TWT_HOTSPOT_RESP_STATE_NUM ||
			(uint32_t)eNextState >=
			TWT_HOTSPOT_RESP_STATE_NUM) {
			DBGLOG(TWT_RESPONDER, ERROR,
				"Invalid stat eNextState[%d]\n", eNextState);
			return;
		}
		DBGLOG(TWT_RESPONDER, ERROR,
		"[TWT_RESP] Flow %d TRANSITION: [%s] -> [%s]\n",
		ucTWTFlowId,
		apucDebugTWTRespState[(uint32_t)(prBssInfo->aeTWTRespState)],
		apucDebugTWTRespState[(uint32_t)eNextState]);

		ePreState = prBssInfo->aeTWTRespState;

		prBssInfo->aeTWTRespState = eNextState;
		fgIsTransition = (uint8_t) FALSE;

		switch (prBssInfo->aeTWTRespState) {
		case TWT_HOTSPOT_RESP_STATE_IDLE:
			if (ePreState ==
				TWT_HOTSPOT_RESP_STATE_SETUP_RESPONSE) {
				if (prStaRec->TWTHotspotCtrl.ucIsReject
					== FALSE) {
					/* Send TWT agrt EXT_CMD to F/W */
					twtHotspotRespFsmSendEvent(
						prAdapter,
						prStaRec,
						ucTWTFlowId,
						MID_TWT_RESP_SETUP_AGRT_TO_FW);
				}
			} else if (
				(ePreState ==
				TWT_HOTSPOT_RESP_STATE_RECEIVE_TEARDOWN) ||
				(ePreState ==
				TWT_HOTSPOT_RESP_STATE_DISCONNECT)) {
				/* Reset the TWT hotspot station node */
				twtHotspotResetStaNode(
					prAdapter,
					prStaRec);

				/* Return the flow ID */
				twtHotspotReturnFlowId(
					prAdapter,
					prStaRec,
					ucTWTFlowId);
			}

			break;

		case TWT_HOTSPOT_RESP_STATE_RECEIVE_SETUP:
			/* We just receive STA's TWT setup request */
			eNextState = TWT_HOTSPOT_RESP_STATE_SETUP_RESPONSE;

			fgIsTransition = TRUE;

			break;

		case TWT_HOTSPOT_RESP_STATE_SETUP_RESPONSE:
		{
			/* Send the TWT setup response frame */
			struct _TWT_HOTSPOT_CTRL_T *pTWTHotspotCtrl =
				(struct _TWT_HOTSPOT_CTRL_T *) pParam;
			struct _TWT_PARAMS_T *prTWTParams =
				(struct _TWT_PARAMS_T *)
				(&pTWTHotspotCtrl->rTWTParams);

			ASSERT(prTWTParams);

			rStatus = twtHotspotSendSetupRespFrame(
				prAdapter, prStaRec, ucTWTFlowId,
				pTWTHotspotCtrl->ucDialogToken,
				prTWTParams, twtHotspotRespFsmRunEventTxDone);

			if (rStatus != WLAN_STATUS_SUCCESS) {
				eNextState = TWT_HOTSPOT_RESP_STATE_IDLE;

				fgIsTransition = TRUE;
			}

			break;
		}

		case TWT_HOTSPOT_RESP_STATE_RECEIVE_TEARDOWN:
		{
			/* TWT hotspot receives STA's teardown frame */
			twtHotspotRespFsmSendEvent(
				prAdapter,
				prStaRec,
				ucTWTFlowId,
				MID_TWT_RESP_TEARDOWN_TO_FW);

			break;
		}

		case TWT_HOTSPOT_RESP_STATE_SEND_TEARDOWN_TO_STA:
			break;

		case TWT_HOTSPOT_RESP_STATE_DISCONNECT:
		{
			/* TWT hotspot disconnect with this STA */
			twtHotspotRespFsmSendEvent(
				prAdapter,
				prStaRec,
				ucTWTFlowId,
				MID_TWT_RESP_TEARDOWN_TO_FW);

			break;
		}

		case TWT_HOTSPOT_RESP_STATE_IDLE_BY_FORCE:
		{
			fgIsTransition = (uint8_t) TRUE;

			eNextState = TWT_HOTSPOT_RESP_STATE_IDLE;

			break;
		}

		default:
			DBGLOG(TWT_RESPONDER, ERROR,
				"[TWT_RESP]Unknown TWT_RESPONDER STATE\n");
			ASSERT(0);

			break;
		}
	} while (fgIsTransition);
}

void
twtHotspotRespFsmRunEventRxSetup(
	struct ADAPTER *prAdapter,
	void *pParam)
{
	struct STA_RECORD *prStaRec;
	struct BSS_INFO *prBssInfo;
	struct _TWT_HOTSPOT_CTRL_T *prTWTCtrl;

	prTWTCtrl = (struct _TWT_HOTSPOT_CTRL_T *)pParam;

	prStaRec = prTWTCtrl->prStaRec;

	prBssInfo = GET_BSS_INFO_BY_INDEX(prAdapter, prStaRec->ucBssIndex);

	ASSERT(prBssInfo);

	if (p2pFuncIsAPMode(prAdapter->rWifiVar
		.prP2PConnSettings[prBssInfo->u4PrivateData])) {
		switch (prBssInfo->aeTWTRespState) {
		case TWT_HOTSPOT_RESP_STATE_IDLE:
			/* transition to the TWT hotspot receive setup state */
			twtHotspotRespFsmSteps(
				prAdapter,
				prStaRec,
				TWT_HOTSPOT_RESP_STATE_RECEIVE_SETUP,
				prTWTCtrl->ucTWTFlowId,
				(void *)prTWTCtrl);

			break;

		default:
			break;
		}
	}
}

u_int32_t
twtHotspotRespFsmRunEventTxDone(
	struct ADAPTER *prAdapter,
	struct MSDU_INFO *prMsduInfo,
	enum ENUM_TX_RESULT_CODE rTxDoneStatus)
{
	struct STA_RECORD *prStaRec;
	enum _ENUM_TWT_HOTSPOT_RESPONDER_STATE_T eNextState;
	uint8_t ucTWTFlowId;
	struct BSS_INFO *prBssInfo;

	ASSERT(prMsduInfo);

	prStaRec = cnmGetStaRecByIndex(prAdapter, prMsduInfo->ucStaRecIndex);

	if (!prStaRec) {
		DBGLOG(TWT_RESPONDER, ERROR,
			"[TWT_RESP]EVENT-TXDONE: No valid STA Record\n");
		return WLAN_STATUS_INVALID_PACKET;
	}

	if (rTxDoneStatus)
		DBGLOG(TWT_RESPONDER, ERROR,
			"[TWT_RESP]EVENT-TX DONE [status: %d][seq: %d]: Current Time = %d\n",
		   rTxDoneStatus, prMsduInfo->ucTxSeqNum, kalGetTimeTick());

	prBssInfo = GET_BSS_INFO_BY_INDEX(prAdapter, prStaRec->ucBssIndex);

	ASSERT(prBssInfo);

	switch (prBssInfo->aeTWTRespState) {
	case TWT_HOTSPOT_RESP_STATE_SETUP_RESPONSE:
		eNextState = TWT_HOTSPOT_RESP_STATE_IDLE;

		ucTWTFlowId = twtGetTxSetupFlowId(prMsduInfo);

		twtHotspotRespFsmSteps(prAdapter,
			prStaRec, eNextState, ucTWTFlowId, NULL);

		break;

	default:
		break;		/* Ignore other cases */
	}

	return WLAN_STATUS_SUCCESS;
}
#endif
