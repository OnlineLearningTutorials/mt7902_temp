/* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause */
/*
 * Copyright (c) 2017 MediaTek Inc.
 */
/*! \file   "twt_planner.c"
*   \brief  TWT Planner to determine TWT negotiation policy
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

/*******************************************************************************
*                                 M A C R O S
********************************************************************************
*/

/*******************************************************************************
*                   F U N C T I O N   D E C L A R A T I O N S
********************************************************************************
*/

/*******************************************************************************
*                              F U N C T I O N S
********************************************************************************
*/

static uint32_t
_twtPlannerDrvAgrtAdd(struct _TWT_PLANNER_T *prTWTPlanner,
	uint8_t ucBssIdx, uint8_t ucFlowId,
	struct _TWT_PARAMS_T *prTWTParams, uint8_t ucIdx)
{
	struct _TWT_AGRT_T *prTWTAgrt = &(prTWTPlanner->arTWTAgrtTbl[ucIdx]);

	prTWTAgrt->fgValid = TRUE;
	prTWTAgrt->ucBssIdx = ucBssIdx;
	prTWTAgrt->ucFlowId = ucFlowId;
	prTWTAgrt->ucAgrtTblIdx = ucIdx;
	kalMemCopy(&(prTWTAgrt->rTWTAgrt), prTWTParams,
		sizeof(struct _TWT_PARAMS_T));

	return WLAN_STATUS_SUCCESS;
}

static uint32_t
_twtPlannerDrvAgrtDel(
	struct _TWT_PLANNER_T *prTWTPlanner, uint8_t ucIdx)
{
	struct _TWT_AGRT_T *prTWTAgrt = &(prTWTPlanner->arTWTAgrtTbl[ucIdx]);

	kalMemSet(prTWTAgrt, 0, sizeof(struct _TWT_AGRT_T));

	return WLAN_STATUS_SUCCESS;
}

static uint32_t
_twtPlannerDrvAgrtModify(
	struct _TWT_PLANNER_T *prTWTPlanner,
	struct _NEXT_TWT_INFO_T *prNextTWTInfo,
	uint64_t u8CurTsf, uint8_t ucIdx,
	struct _TWT_PARAMS_T *prTWTParams)
{
	struct _TWT_AGRT_T *prTWTAgrt = &(prTWTPlanner->arTWTAgrtTbl[ucIdx]);

	if (!prNextTWTInfo || !prTWTParams)
		return WLAN_STATUS_FAILURE;

	if (prNextTWTInfo->ucNextTWTSize == NEXT_TWT_SUBFIELD_64_BITS) {
		prTWTAgrt->rTWTAgrt.u8TWT = prNextTWTInfo->u8NextTWT;
	} else if (prNextTWTInfo->ucNextTWTSize == NEXT_TWT_SUBFIELD_32_BITS) {
		prTWTAgrt->rTWTAgrt.u8TWT =
			((u8CurTsf & ((uint64_t)0xFFFFFFFF << 32)) |
			prNextTWTInfo->u8NextTWT);
	} else if (prNextTWTInfo->ucNextTWTSize == NEXT_TWT_SUBFIELD_48_BITS) {
		prTWTAgrt->rTWTAgrt.u8TWT =
			((u8CurTsf & ((uint64_t)0xFFFF << 48)) |
			prNextTWTInfo->u8NextTWT);
	} else {
		/* Zero bit Next TWT is not acceptable */
		return WLAN_STATUS_FAILURE;
	}

	kalMemCopy(prTWTParams, &(prTWTAgrt->rTWTAgrt),
		sizeof(struct _TWT_PARAMS_T));

	return WLAN_STATUS_SUCCESS;
}

static uint32_t
_twtPlannerDrvAgrtGet(
	struct _TWT_PLANNER_T *prTWTPlanner,
	uint8_t ucIdx, struct _TWT_PARAMS_T *prTWTParams)
{
	struct _TWT_AGRT_T *prTWTAgrt = &(prTWTPlanner->arTWTAgrtTbl[ucIdx]);

	if (!prTWTParams)
		return WLAN_STATUS_FAILURE;

	kalMemCopy(prTWTParams, &(prTWTAgrt->rTWTAgrt),
		sizeof(struct _TWT_PARAMS_T));

	return WLAN_STATUS_SUCCESS;
}

static uint8_t
twtPlannerDrvAgrtFind(struct ADAPTER *prAdapter, uint8_t ucBssIdx,
	uint8_t ucFlowId, uint8_t *pucFlowId)
{
	uint8_t i;
	struct _TWT_PLANNER_T *prTWTPlanner = &(prAdapter->rTWTPlanner);
	struct _TWT_AGRT_T *prTWTAgrt = &(prTWTPlanner->arTWTAgrtTbl[0]);
	uint8_t uCnt = 0;

	for (i = 0; i < TWT_AGRT_MAX_NUM; i++, prTWTAgrt++) {
		if (prTWTAgrt->fgValid == TRUE &&
			prTWTAgrt->ucFlowId == ucFlowId &&
			prTWTAgrt->ucBssIdx == ucBssIdx)
			break;
	}

	if ((i >= TWT_AGRT_MAX_NUM) && (pucFlowId != NULL)) {
		/*
		** This might be the case that we are in WFA logo
		** and QCOM reply TWT flow ID not identical to our
		** requested TWT flow ID
		*/

		/* Check if there exists only 1 valid TWT agrt */
		prTWTAgrt = &(prTWTPlanner->arTWTAgrtTbl[0]);

		for (i = 0; i < TWT_AGRT_MAX_NUM; i++, prTWTAgrt++) {
			if (prTWTAgrt->fgValid == TRUE)
				uCnt++;
		}

		/* Yes, only 1 valid TWT agrt exists,
		** take its TWT flow ID if the input ucFlowId == 0
		*/
		if ((uCnt == 1) && (ucFlowId == 0)) {
			prTWTAgrt = &(prTWTPlanner->arTWTAgrtTbl[0]);

			for (i = 0; i < TWT_AGRT_MAX_NUM; i++, prTWTAgrt++) {
				if ((prTWTAgrt->fgValid == TRUE) &&
					(prTWTAgrt->ucBssIdx == ucBssIdx)) {
					*pucFlowId = prTWTAgrt->ucFlowId;

					DBGLOG(TWT_PLANNER, WARN,
						"Gotcha, agrt bss %u flow %u->%u\n",
						ucBssIdx, ucFlowId, *pucFlowId);

					break;
				}
			}
		}
	}

	return i;
}

uint32_t
twtPlannerDrvAgrtAdd(struct ADAPTER *prAdapter,
	uint8_t ucBssIdx, uint8_t ucFlowId,
	struct _TWT_PARAMS_T *prTWTParams, uint8_t *pucIdx)
{
	uint8_t ucIdx;
	uint32_t rStatus = WLAN_STATUS_FAILURE;
	struct _TWT_PLANNER_T *prTWTPlanner = &(prAdapter->rTWTPlanner);
	struct _TWT_AGRT_T *prTWTAgrt = &(prTWTPlanner->arTWTAgrtTbl[0]);

	for (ucIdx = 0; ucIdx < TWT_AGRT_MAX_NUM; ucIdx++, prTWTAgrt++) {
		if (prTWTAgrt->fgValid == FALSE)
			break;
	}

	if (ucIdx < TWT_AGRT_MAX_NUM) {
		_twtPlannerDrvAgrtAdd(prTWTPlanner, ucBssIdx,
			ucFlowId, prTWTParams, ucIdx);
		*pucIdx = ucIdx;
		rStatus = WLAN_STATUS_SUCCESS;
	}

	return rStatus;
}

uint32_t
twtPlannerDrvAgrtModify(
	struct ADAPTER *prAdapter,
	uint8_t ucBssIdx, uint8_t ucFlowId,
	struct _NEXT_TWT_INFO_T *prNextTWTInfo,
	uint8_t *pucIdx, struct _TWT_PARAMS_T *prTWTParams)
{
	uint8_t ucIdx;
	uint64_t u8CurTsf;
	struct _TWT_PLANNER_T *prTWTPlanner = &(prAdapter->rTWTPlanner);
	uint32_t rStatus;
	uint8_t ucFlowId_real = ucFlowId;

	ucIdx = twtPlannerDrvAgrtFind(
		prAdapter, ucBssIdx, ucFlowId, &ucFlowId_real);
	if (ucIdx >= TWT_AGRT_MAX_NUM) {
		DBGLOG(TWT_PLANNER, ERROR, "Can't find agrt bss %u flow %u\n",
			ucBssIdx, ucFlowId);
		return WLAN_STATUS_FAILURE;
	}

	/* TODO: get current TSF from FW */
	u8CurTsf = 0;

	rStatus = _twtPlannerDrvAgrtModify(prTWTPlanner, prNextTWTInfo,
		u8CurTsf, ucIdx, prTWTParams);
	if (rStatus == WLAN_STATUS_SUCCESS)
		*pucIdx = ucIdx;

	return rStatus;
}

uint32_t
twtPlannerDrvAgrtGet(struct ADAPTER *prAdapter,
	uint8_t ucBssIdx, uint8_t ucFlowId,
	uint8_t *pucIdx, struct _TWT_PARAMS_T *prTWTParams)
{
	uint8_t ucIdx;
	struct _TWT_PLANNER_T *prTWTPlanner = &(prAdapter->rTWTPlanner);
	uint32_t rStatus;
	uint8_t ucFlowId_real = ucFlowId;

	ucIdx = twtPlannerDrvAgrtFind(
		prAdapter, ucBssIdx, ucFlowId, &ucFlowId_real);
	if (ucIdx >= TWT_AGRT_MAX_NUM) {
		DBGLOG(TWT_PLANNER, ERROR, "Can't find agrt bss %u flow %u\n",
			ucBssIdx, ucFlowId);
		return WLAN_STATUS_FAILURE;
	}

	rStatus = _twtPlannerDrvAgrtGet(prTWTPlanner, ucIdx, prTWTParams);
	if (rStatus == WLAN_STATUS_SUCCESS)
		*pucIdx = ucIdx;

	return rStatus;
}

bool
twtPlannerIsDrvAgrtExisting(struct ADAPTER *prAdapter)
{
	bool ret = FALSE;
	uint8_t i;
	struct _TWT_PLANNER_T *prTWTPlanner = &(prAdapter->rTWTPlanner);
	struct _TWT_AGRT_T *prTWTAgrt = &(prTWTPlanner->arTWTAgrtTbl[0]);

	for (i = 0; i < TWT_AGRT_MAX_NUM; i++, prTWTAgrt++) {
		if (prTWTAgrt->fgValid == TRUE) {
			ret = TRUE;
			break;
		}
	}

	return ret;
}

void twtPlannerInit(IN struct _TWT_PLANNER_T *pTWTPlanner)
{
	ASSERT(pTWTPlanner);

	kalMemSet(&(pTWTPlanner->arTWTAgrtTbl[0]), 0,
		TWT_AGRT_MAX_NUM * sizeof(struct _TWT_AGRT_T));
}

static struct _TWT_FLOW_T *twtPlannerFlowFindById(
	struct STA_RECORD *prStaRec, uint8_t ucFlowId)
{
	struct _TWT_FLOW_T *prTWTFlow = NULL;

	ASSERT(prStaRec);

	if (ucFlowId >= TWT_MAX_FLOW_NUM) {
		DBGLOG(TWT_PLANNER, ERROR, "Invalid TWT flow id %u\n",
			ucFlowId);
		return NULL;
	}

	prTWTFlow = &(prStaRec->arTWTFlow[ucFlowId]);

	return prTWTFlow;
}

static uint32_t
twtPlannerSendReqStart(
	struct ADAPTER *prAdapter,
	struct STA_RECORD *prStaRec,
	uint8_t ucTWTFlowId)
{
	struct _MSG_TWT_REQFSM_START_T *prTWTReqFsmStartMsg;

	prTWTReqFsmStartMsg = cnmMemAlloc(prAdapter, RAM_TYPE_MSG,
		sizeof(struct _MSG_TWT_REQFSM_START_T));
	if (prTWTReqFsmStartMsg) {
		prTWTReqFsmStartMsg->rMsgHdr.eMsgId = MID_TWT_REQ_FSM_START;
		prTWTReqFsmStartMsg->prStaRec = prStaRec;
		prTWTReqFsmStartMsg->ucTWTFlowId = ucTWTFlowId;

		mboxSendMsg(prAdapter,
			MBOX_ID_0,
			(struct MSG_HDR *) prTWTReqFsmStartMsg,
			MSG_SEND_METHOD_BUF);
	} else
		return WLAN_STATUS_RESOURCES;

	return WLAN_STATUS_SUCCESS;
}

static uint32_t
twtPlannerSendReqTeardown(struct ADAPTER *prAdapter,
				struct STA_RECORD *prStaRec,
				uint8_t ucTWTFlowId)
{
	struct _MSG_TWT_REQFSM_TEARDOWN_T *prTWTReqFsmTeardownMsg;

	prTWTReqFsmTeardownMsg = cnmMemAlloc(prAdapter, RAM_TYPE_MSG,
		sizeof(struct _MSG_TWT_REQFSM_TEARDOWN_T));
	if (prTWTReqFsmTeardownMsg) {
		prTWTReqFsmTeardownMsg->rMsgHdr.eMsgId =
			MID_TWT_REQ_FSM_TEARDOWN;
		prTWTReqFsmTeardownMsg->prStaRec = prStaRec;
		prTWTReqFsmTeardownMsg->ucTWTFlowId = ucTWTFlowId;

		mboxSendMsg(prAdapter, MBOX_ID_0,
			(struct MSG_HDR *) prTWTReqFsmTeardownMsg,
			MSG_SEND_METHOD_BUF);
	} else
		return WLAN_STATUS_RESOURCES;

	return WLAN_STATUS_SUCCESS;
}

static uint32_t
twtPlannerSendReqSuspend(struct ADAPTER *prAdapter,
				struct STA_RECORD *prStaRec,
				uint8_t ucTWTFlowId)
{
	struct _MSG_TWT_REQFSM_SUSPEND_T *prTWTReqFsmSuspendMsg;

	prTWTReqFsmSuspendMsg = cnmMemAlloc(prAdapter, RAM_TYPE_MSG,
		sizeof(struct _MSG_TWT_REQFSM_SUSPEND_T));
	if (prTWTReqFsmSuspendMsg) {
		prTWTReqFsmSuspendMsg->rMsgHdr.eMsgId =
			MID_TWT_REQ_FSM_SUSPEND;
		prTWTReqFsmSuspendMsg->prStaRec = prStaRec;
		prTWTReqFsmSuspendMsg->ucTWTFlowId = ucTWTFlowId;

		mboxSendMsg(prAdapter, MBOX_ID_0,
			(struct MSG_HDR *) prTWTReqFsmSuspendMsg,
			MSG_SEND_METHOD_BUF);
	} else
		return WLAN_STATUS_RESOURCES;

	return WLAN_STATUS_SUCCESS;
}

static uint32_t
twtPlannerSendReqResume(struct ADAPTER *prAdapter,
				struct STA_RECORD *prStaRec,
				uint8_t ucTWTFlowId,
				uint64_t u8NextTWT,
				uint8_t ucNextTWTSize)
{
	struct _MSG_TWT_REQFSM_RESUME_T *prTWTReqFsmResumeMsg;

	prTWTReqFsmResumeMsg = cnmMemAlloc(prAdapter, RAM_TYPE_MSG,
		sizeof(struct _MSG_TWT_REQFSM_RESUME_T));
	if (prTWTReqFsmResumeMsg) {
		prTWTReqFsmResumeMsg->rMsgHdr.eMsgId =
			MID_TWT_REQ_FSM_RESUME;
		prTWTReqFsmResumeMsg->prStaRec = prStaRec;
		prTWTReqFsmResumeMsg->ucTWTFlowId = ucTWTFlowId;
		prTWTReqFsmResumeMsg->u8NextTWT = u8NextTWT;
		prTWTReqFsmResumeMsg->ucNextTWTSize = ucNextTWTSize;

		mboxSendMsg(prAdapter, MBOX_ID_0,
			(struct MSG_HDR *) prTWTReqFsmResumeMsg,
			MSG_SEND_METHOD_BUF);
	} else
		return WLAN_STATUS_RESOURCES;

	return WLAN_STATUS_SUCCESS;
}

static uint32_t
twtPlannerAddAgrtTbl(
	struct ADAPTER *prAdapter,
	struct BSS_INFO *prBssInfo,
	struct STA_RECORD *prStaRec,
	struct _TWT_PARAMS_T *prTWTParams,
	uint8_t ucFlowId,
	uint8_t fgIsOid,
	PFN_CMD_DONE_HANDLER pfCmdDoneHandler,
	PFN_CMD_TIMEOUT_HANDLER pfCmdTimeoutHandler)
{
	uint8_t ucAgrtTblIdx;
	uint32_t rWlanStatus = WLAN_STATUS_SUCCESS;
	struct _EXT_CMD_TWT_ARGT_UPDATE_T *prTWTAgrtUpdate;

	rWlanStatus = twtPlannerDrvAgrtAdd(prAdapter,
		prBssInfo ? prBssInfo->ucBssIndex : 0,
		ucFlowId, prTWTParams, &ucAgrtTblIdx);
	if (rWlanStatus) {
		DBGLOG(TWT_PLANNER, ERROR,
			"Agreement table is full\n");
		return WLAN_STATUS_FAILURE;
	}

	prTWTAgrtUpdate = cnmMemAlloc(
		prAdapter,
		RAM_TYPE_MSG,
		sizeof(struct _EXT_CMD_TWT_ARGT_UPDATE_T));
	if (!prTWTAgrtUpdate) {
		DBGLOG(
		TWT_PLANNER,
		ERROR,
		"Allocate _EXT_CMD_TWT_ARGT_UPDATE_T ==> FAILED.\n");
		return WLAN_STATUS_FAILURE;
	}

	prTWTAgrtUpdate->ucAgrtTblIdx = ucAgrtTblIdx;
	prTWTAgrtUpdate->ucAgrtCtrlFlag = TWT_AGRT_CTRL_ADD;
	prTWTAgrtUpdate->ucOwnMacId =
		(prBssInfo) ? prBssInfo->ucOwnMacIndex : 0;
	prTWTAgrtUpdate->ucFlowId = ucFlowId;
	prTWTAgrtUpdate->u2PeerIdGrpId =
		(prStaRec) ? CPU_TO_LE16(prStaRec->ucWlanIndex) : 1;
	prTWTAgrtUpdate->ucAgrtSpDuration = prTWTParams->ucMinWakeDur;
	prTWTAgrtUpdate->ucBssIndex = prBssInfo ? prBssInfo->ucBssIndex : 0;
	prTWTAgrtUpdate->u4AgrtSpStartTsfLow =
		CPU_TO_LE32(prTWTParams->u8TWT & 0xFFFFFFFF);
	prTWTAgrtUpdate->u4AgrtSpStartTsfHigh =
		CPU_TO_LE32((uint32_t)(prTWTParams->u8TWT >> 32));
	prTWTAgrtUpdate->u2AgrtSpWakeIntvlMantissa =
		CPU_TO_LE16(prTWTParams->u2WakeIntvalMantiss);
	prTWTAgrtUpdate->ucAgrtSpWakeIntvlExponent =
		prTWTParams->ucWakeIntvalExponent;
	prTWTAgrtUpdate->ucIsRoleAp = 0;  /* STA role */

	prTWTAgrtUpdate->ucAgrtParaBitmap =
	((prTWTParams->fgProtect << TWT_AGRT_PARA_BITMAP_PROTECT_OFFSET) |
	((!prTWTParams->fgUnannounced) << TWT_AGRT_PARA_BITMAP_ANNCE_OFFSET) |
	(prTWTParams->fgTrigger << TWT_AGRT_PARA_BITMAP_TRIGGER_OFFSET));

	prTWTAgrtUpdate->ucGrpMemberCnt = 0;

	rWlanStatus = wlanSendSetQueryExtCmd(prAdapter,
				CMD_ID_LAYER_0_EXT_MAGIC_NUM,
				EXT_CMD_ID_TWT_AGRT_UPDATE,
				TRUE,
				FALSE,
				fgIsOid,
				pfCmdDoneHandler,
				pfCmdTimeoutHandler,
				sizeof(struct _EXT_CMD_TWT_ARGT_UPDATE_T),
				(uint8_t *) (prTWTAgrtUpdate),
				NULL, 0);

	cnmMemFree(prAdapter, prTWTAgrtUpdate);

	return rWlanStatus;
}

static uint32_t
twtPlannerResumeAgrtTbl(struct ADAPTER *prAdapter,
			struct BSS_INFO *prBssInfo, struct STA_RECORD *prStaRec,
			uint8_t ucFlowId, uint8_t fgIsOid,
			PFN_CMD_DONE_HANDLER pfCmdDoneHandler,
			PFN_CMD_TIMEOUT_HANDLER pfCmdTimeoutHandler)
{
	uint8_t ucAgrtTblIdx;
	uint32_t rWlanStatus = WLAN_STATUS_SUCCESS;
	struct _EXT_CMD_TWT_ARGT_UPDATE_T *prTWTAgrtUpdate;
	struct _TWT_PARAMS_T rTWTParams;

	rWlanStatus = twtPlannerDrvAgrtGet(prAdapter,
		prBssInfo ? prBssInfo->ucBssIndex : 0,
		ucFlowId, &ucAgrtTblIdx, &rTWTParams);
	if (rWlanStatus) {
		DBGLOG(TWT_PLANNER, ERROR, "No agrt to resume Bss %u flow %u\n",
			prBssInfo ? prBssInfo->ucBssIndex : 0, ucFlowId);
		return WLAN_STATUS_FAILURE;
	}

	prTWTAgrtUpdate = cnmMemAlloc(prAdapter, RAM_TYPE_MSG,
		sizeof(struct _EXT_CMD_TWT_ARGT_UPDATE_T));
	if (!prTWTAgrtUpdate) {
		DBGLOG(TWT_PLANNER, ERROR,
			"Allocate _EXT_CMD_TWT_ARGT_UPDATE_T ==> FAILED.\n");
		return WLAN_STATUS_FAILURE;
	}

	prTWTAgrtUpdate->ucAgrtTblIdx = ucAgrtTblIdx;
	prTWTAgrtUpdate->ucAgrtCtrlFlag = TWT_AGRT_CTRL_ADD;
	prTWTAgrtUpdate->ucOwnMacId = (prBssInfo) ?
		prBssInfo->ucOwnMacIndex : 0;
	prTWTAgrtUpdate->ucFlowId = ucFlowId;
	prTWTAgrtUpdate->u2PeerIdGrpId = (prStaRec) ?
		CPU_TO_LE16(prStaRec->ucWlanIndex) : 1;
	prTWTAgrtUpdate->ucAgrtSpDuration = rTWTParams.ucMinWakeDur;
	prTWTAgrtUpdate->ucBssIndex = prBssInfo ? prBssInfo->ucBssIndex : 0;
	prTWTAgrtUpdate->u4AgrtSpStartTsfLow =
		CPU_TO_LE32(rTWTParams.u8TWT & 0xFFFFFFFF);
	prTWTAgrtUpdate->u4AgrtSpStartTsfHigh =
		CPU_TO_LE32((uint32_t)(rTWTParams.u8TWT >> 32));
	prTWTAgrtUpdate->u2AgrtSpWakeIntvlMantissa =
		CPU_TO_LE16(rTWTParams.u2WakeIntvalMantiss);
	prTWTAgrtUpdate->ucAgrtSpWakeIntvlExponent =
		rTWTParams.ucWakeIntvalExponent;
	prTWTAgrtUpdate->ucIsRoleAp = 0;  /* STA role */
	prTWTAgrtUpdate->ucAgrtParaBitmap =
	    ((rTWTParams.fgProtect << TWT_AGRT_PARA_BITMAP_PROTECT_OFFSET) |
	    ((!rTWTParams.fgUnannounced) << TWT_AGRT_PARA_BITMAP_ANNCE_OFFSET) |
	    (rTWTParams.fgTrigger << TWT_AGRT_PARA_BITMAP_TRIGGER_OFFSET));
	prTWTAgrtUpdate->ucGrpMemberCnt = 0;

	rWlanStatus = wlanSendSetQueryExtCmd(prAdapter,
			CMD_ID_LAYER_0_EXT_MAGIC_NUM,
			EXT_CMD_ID_TWT_AGRT_UPDATE,
			TRUE,
			FALSE,
			fgIsOid,
			pfCmdDoneHandler,
			pfCmdTimeoutHandler,
			sizeof(struct _EXT_CMD_TWT_ARGT_UPDATE_T),
			(uint8_t *) (prTWTAgrtUpdate),
			NULL, 0);

	cnmMemFree(prAdapter, prTWTAgrtUpdate);

	return rWlanStatus;
}

static uint32_t
twtPlannerModifyAgrtTbl(struct ADAPTER *prAdapter,
			struct BSS_INFO *prBssInfo, struct STA_RECORD *prStaRec,
			struct _NEXT_TWT_INFO_T *prNextTWTInfo,
			uint8_t ucFlowId, uint8_t fgIsOid,
			PFN_CMD_DONE_HANDLER pfCmdDoneHandler,
			PFN_CMD_TIMEOUT_HANDLER pfCmdTimeoutHandler)
{
	uint8_t ucAgrtTblIdx;
	uint32_t rWlanStatus = WLAN_STATUS_SUCCESS;
	struct _EXT_CMD_TWT_ARGT_UPDATE_T *prTWTAgrtUpdate;
	struct _TWT_PARAMS_T rTWTParams;

	/* Handle driver agreement table */
	rWlanStatus = twtPlannerDrvAgrtModify(prAdapter,
		prBssInfo ? prBssInfo->ucBssIndex : 0,
		ucFlowId, prNextTWTInfo, &ucAgrtTblIdx, &rTWTParams);

	if (rWlanStatus) {
		DBGLOG(TWT_PLANNER, ERROR, "No agrt to modify Bss %u flow %u\n",
			prBssInfo ? prBssInfo->ucBssIndex : 0, ucFlowId);
		return WLAN_STATUS_FAILURE;
	}

	/* Handle FW agreement table */
	prTWTAgrtUpdate = cnmMemAlloc(prAdapter, RAM_TYPE_MSG,
		sizeof(struct _EXT_CMD_TWT_ARGT_UPDATE_T));
	if (!prTWTAgrtUpdate) {
		DBGLOG(TWT_PLANNER, ERROR,
			"Allocate _EXT_CMD_TWT_ARGT_UPDATE_T ==> FAILED.\n");
		return WLAN_STATUS_FAILURE;
	}

	prTWTAgrtUpdate->ucAgrtTblIdx = ucAgrtTblIdx;
	prTWTAgrtUpdate->ucAgrtCtrlFlag = TWT_AGRT_CTRL_MODIFY;
	prTWTAgrtUpdate->ucOwnMacId = (prBssInfo) ?
		prBssInfo->ucOwnMacIndex : 0;
	prTWTAgrtUpdate->ucFlowId = ucFlowId;
	prTWTAgrtUpdate->u2PeerIdGrpId = (prStaRec) ?
		CPU_TO_LE16(prStaRec->ucWlanIndex) : 1;
	prTWTAgrtUpdate->ucAgrtSpDuration = rTWTParams.ucMinWakeDur;
	prTWTAgrtUpdate->u4AgrtSpStartTsfLow =
		CPU_TO_LE32(rTWTParams.u8TWT & 0xFFFFFFFF);
	prTWTAgrtUpdate->u4AgrtSpStartTsfHigh =
		CPU_TO_LE32((uint32_t)(rTWTParams.u8TWT >> 32));
	prTWTAgrtUpdate->ucGrpMemberCnt = 0;
	prTWTAgrtUpdate->ucBssIndex = prBssInfo ? prBssInfo->ucBssIndex : 0;

	rWlanStatus = wlanSendSetQueryExtCmd(prAdapter,
			CMD_ID_LAYER_0_EXT_MAGIC_NUM,
			EXT_CMD_ID_TWT_AGRT_UPDATE,
			TRUE,
			FALSE,
			fgIsOid,
			pfCmdDoneHandler,
			pfCmdTimeoutHandler,
			sizeof(struct _EXT_CMD_TWT_ARGT_UPDATE_T),
			(uint8_t *) (prTWTAgrtUpdate),
			NULL, 0);

	cnmMemFree(prAdapter, prTWTAgrtUpdate);

	return rWlanStatus;
}

static uint32_t
twtPlannerDelAgrtTbl(struct ADAPTER *prAdapter,
			struct BSS_INFO *prBssInfo, struct STA_RECORD *prStaRec,
			uint8_t ucFlowId, uint8_t fgIsOid,
			PFN_CMD_DONE_HANDLER pfCmdDoneHandler,
			PFN_CMD_TIMEOUT_HANDLER pfCmdTimeoutHandler,
			uint8_t fgDelDrvEntry)
{
	uint8_t ucAgrtTblIdx;
	uint32_t rWlanStatus = WLAN_STATUS_SUCCESS;
	struct _TWT_PLANNER_T *prTWTPlanner = &(prAdapter->rTWTPlanner);
	struct _EXT_CMD_TWT_ARGT_UPDATE_T *prTWTAgrtUpdate;
	uint8_t ucFlowId_real = ucFlowId;

	/* Find and delete the agreement entry in the driver */
	ucAgrtTblIdx = twtPlannerDrvAgrtFind(prAdapter,
		prBssInfo ? prBssInfo->ucBssIndex : 0, ucFlowId,
		&ucFlowId_real);

	if (ucAgrtTblIdx >= TWT_AGRT_MAX_NUM) {
		DBGLOG(TWT_PLANNER, ERROR,
			"Cannot find the flow %u to be deleted\n", ucFlowId);
		return WLAN_STATUS_FAILURE;

	}

	ucFlowId = ucFlowId_real;

	if (fgDelDrvEntry)
		_twtPlannerDrvAgrtDel(prTWTPlanner, ucAgrtTblIdx);

	/* Send cmd to delete agreement entry in FW */
	prTWTAgrtUpdate = cnmMemAlloc(prAdapter, RAM_TYPE_MSG,
		sizeof(struct _EXT_CMD_TWT_ARGT_UPDATE_T));
	if (!prTWTAgrtUpdate) {
		DBGLOG(TWT_PLANNER, ERROR,
			"Alloc _EXT_CMD_TWT_ARGT_UPDATE_T for del FAILED.\n");
		return WLAN_STATUS_FAILURE;
	}

	prTWTAgrtUpdate->ucAgrtTblIdx = ucAgrtTblIdx;
	prTWTAgrtUpdate->ucAgrtCtrlFlag = TWT_AGRT_CTRL_DELETE;
	prTWTAgrtUpdate->ucOwnMacId = (prBssInfo) ?
		prBssInfo->ucOwnMacIndex : 0;
	prTWTAgrtUpdate->ucFlowId = ucFlowId;
	prTWTAgrtUpdate->u2PeerIdGrpId = (prStaRec) ?
		CPU_TO_LE16(prStaRec->ucWlanIndex) : 1;
	prTWTAgrtUpdate->ucIsRoleAp = 0;  /* STA role */
	prTWTAgrtUpdate->ucBssIndex = prBssInfo ? prBssInfo->ucBssIndex : 0;

	rWlanStatus = wlanSendSetQueryExtCmd(prAdapter,
			CMD_ID_LAYER_0_EXT_MAGIC_NUM,
			EXT_CMD_ID_TWT_AGRT_UPDATE,
			TRUE,
			FALSE,
			fgIsOid,
			pfCmdDoneHandler,
			pfCmdTimeoutHandler,
			sizeof(struct _EXT_CMD_TWT_ARGT_UPDATE_T),
			(uint8_t *) (prTWTAgrtUpdate),
			NULL, 0);

	cnmMemFree(prAdapter, prTWTAgrtUpdate);

	return rWlanStatus;
}

static uint32_t
twtPlannerTeardownAgrtTbl(struct ADAPTER *prAdapter,
			struct STA_RECORD *prStaRec,
			uint8_t fgIsOid,
			PFN_CMD_DONE_HANDLER pfCmdDoneHandler,
			PFN_CMD_TIMEOUT_HANDLER pfCmdTimeoutHandler)
{
	uint32_t rWlanStatus = WLAN_STATUS_SUCCESS;
	struct _EXT_CMD_TWT_ARGT_UPDATE_T *prTWTAgrtUpdate;

	/* Send cmd to teardown this STA in FW */
	prTWTAgrtUpdate = cnmMemAlloc(prAdapter, RAM_TYPE_MSG,
		sizeof(struct _EXT_CMD_TWT_ARGT_UPDATE_T));
	if (!prTWTAgrtUpdate) {
		DBGLOG(TWT_PLANNER, ERROR,
			"Alloc _EXT_CMD_TWT_ARGT_UPDATE_T for del FAILED.\n");
		return WLAN_STATUS_FAILURE;
	}

	/* Don't care about other fields of the cmd */
	prTWTAgrtUpdate->ucAgrtCtrlFlag = TWT_AGRT_CTRL_TEARDOWN;
	prTWTAgrtUpdate->u2PeerIdGrpId = (prStaRec) ?
		CPU_TO_LE16(prStaRec->ucWlanIndex) : 1;
	prTWTAgrtUpdate->ucIsRoleAp = 0;  /* STA role */
	prTWTAgrtUpdate->ucBssIndex = prStaRec ? prStaRec->ucBssIndex : 0;

	rWlanStatus = wlanSendSetQueryExtCmd(prAdapter,
			CMD_ID_LAYER_0_EXT_MAGIC_NUM,
			EXT_CMD_ID_TWT_AGRT_UPDATE,
			TRUE,
			FALSE,
			fgIsOid,
			pfCmdDoneHandler,
			pfCmdTimeoutHandler,
			sizeof(struct _EXT_CMD_TWT_ARGT_UPDATE_T),
			(uint8_t *) (prTWTAgrtUpdate),
			NULL, 0);

	cnmMemFree(prAdapter, prTWTAgrtUpdate);

	return rWlanStatus;
}

uint32_t twtPlannerReset(
	struct ADAPTER *prAdapter, struct BSS_INFO *prBssInfo)
{
	uint32_t rWlanStatus = WLAN_STATUS_SUCCESS;
	struct STA_RECORD *prStaRec;
	struct _EXT_CMD_TWT_ARGT_UPDATE_T *prTWTAgrtUpdate;

	/* If no agrt exits, don't bother resetting */
	if (twtPlannerIsDrvAgrtExisting(prAdapter) == FALSE)
		return rWlanStatus;

	prTWTAgrtUpdate = cnmMemAlloc(prAdapter, RAM_TYPE_MSG,
		sizeof(struct _EXT_CMD_TWT_ARGT_UPDATE_T));
	if (!prTWTAgrtUpdate) {
		DBGLOG(TWT_PLANNER, ERROR,
			"Alloc _EXT_CMD_TWT_ARGT_UPDATE_T for reset FAILED.\n");
		return WLAN_STATUS_FAILURE;
	}

	/* send cmd to reset FW agreement table */
	ASSERT(prBssInfo);
	prStaRec = prBssInfo ? prBssInfo->prStaRecOfAP : NULL;

	prTWTAgrtUpdate->ucAgrtCtrlFlag = TWT_AGRT_CTRL_RESET;
	prTWTAgrtUpdate->u2PeerIdGrpId = (prStaRec) ?
		CPU_TO_LE16(prStaRec->ucWlanIndex) : 1;
	prTWTAgrtUpdate->ucIsRoleAp = 0;  /* STA role */
	prTWTAgrtUpdate->ucBssIndex = prBssInfo ? prBssInfo->ucBssIndex : 0;

	rWlanStatus = wlanSendSetQueryExtCmd(prAdapter,
			CMD_ID_LAYER_0_EXT_MAGIC_NUM,
			EXT_CMD_ID_TWT_AGRT_UPDATE,
			TRUE,
			FALSE,
			FALSE,
			NULL,
			NULL,
			sizeof(struct _EXT_CMD_TWT_ARGT_UPDATE_T),
			(uint8_t *) (prTWTAgrtUpdate),
			NULL, 0);


	/* reset driver agreement table */
	memset(&(prAdapter->rTWTPlanner), 0, sizeof(prAdapter->rTWTPlanner));

#if (CFG_DISABLE_SCAN_UNDER_TWT == 1)
	/* Enable scan after TWT agrt reset */
	prAdapter->fgEnOnlineScan = TRUE;
#endif

	cnmMemFree(prAdapter, prTWTAgrtUpdate);

	return rWlanStatus;
}

uint64_t twtPlannerAdjustNextTWT(struct ADAPTER *prAdapter,
	uint8_t ucBssIdx, uint8_t ucFlowId,
	uint64_t u8NextTWTOrig)
{
	uint8_t ucAgrtTblIdx;
	struct _TWT_PARAMS_T rTWTParams = {0};
	uint64_t u8Diff;
	uint32_t u4WakeIntvl;

	twtPlannerDrvAgrtGet(prAdapter, ucBssIdx, ucFlowId,
		&ucAgrtTblIdx, &rTWTParams);

	u4WakeIntvl = rTWTParams.u2WakeIntvalMantiss <<
		rTWTParams.ucWakeIntvalExponent;
	u8Diff = u8NextTWTOrig - rTWTParams.u8TWT;
	/* TODO: move div_u64 to os-dependent file */
	return (rTWTParams.u8TWT +
		(div_u64(u8Diff, u4WakeIntvl) + 1) * u4WakeIntvl);
}

void twtPlannerGetTsfDone(
	struct ADAPTER *prAdapter,
	struct CMD_INFO *prCmdInfo,
	uint8_t *pucEventBuf)
{
	struct EXT_EVENT_MAC_INFO_T *prEventMacInfo;
	struct _TWT_GET_TSF_CONTEXT_T *prGetTsfCtxt;
	struct BSS_INFO *prBssInfo;
	struct STA_RECORD *prStaRec;
	struct TSF_RESULT_T *prTsfResult;
	uint64_t u8CurTsf;

	ASSERT(prAdapter);
	ASSERT(prCmdInfo);
	ASSERT(pucEventBuf);

	if (!pucEventBuf) {
		DBGLOG(TWT_PLANNER, ERROR, "pucEventBuf is NULL.\n");
		return;
	}
	if (!prCmdInfo->pvInformationBuffer) {
		DBGLOG(TWT_PLANNER, ERROR,
			"prCmdInfo->pvInformationBuffer is NULL.\n");
		return;
	}

	prEventMacInfo = (struct EXT_EVENT_MAC_INFO_T *) (pucEventBuf);
	prGetTsfCtxt = (struct _TWT_GET_TSF_CONTEXT_T *)
		prCmdInfo->pvInformationBuffer;

	ASSERT(prGetTsfCtxt);

	if (prGetTsfCtxt->ucBssIdx < MAX_BSSID_NUM)
		prBssInfo = GET_BSS_INFO_BY_INDEX(
			prAdapter, prGetTsfCtxt->ucBssIdx);
	else {
		DBGLOG(TWT_PLANNER, ERROR,
			"prGetTsfCtxt->ucBssIdx is NULL.\n");
		return;
	}
	ASSERT(prBssInfo);
	prStaRec = prBssInfo->prStaRecOfAP;
	ASSERT(prStaRec);

	prTsfResult = &(prEventMacInfo->rMacInfoResult.rTsfResult);
	u8CurTsf = LE32_TO_CPU(prTsfResult->u4TsfBitsLow) |
		(((uint64_t)(LE32_TO_CPU(prTsfResult->u4TsfBitsHigh))) << 32);

	switch (prGetTsfCtxt->ucReason) {
	case TWT_GET_TSF_FOR_ADD_AGRT_BYPASS:
		prGetTsfCtxt->rTWTParams.u8TWT = u8CurTsf + TSF_OFFSET_FOR_EMU;
		twtPlannerAddAgrtTbl(prAdapter, prBssInfo,
				prStaRec, &(prGetTsfCtxt->rTWTParams),
				prGetTsfCtxt->ucTWTFlowId,
				prGetTsfCtxt->fgIsOid,
				NULL, NULL);
		break;

	case TWT_GET_TSF_FOR_ADD_AGRT:
	{
		struct _TWT_PARAMS_T *prTWTParams;
		struct _TWT_FLOW_T *prTWTFlow = twtPlannerFlowFindById(prStaRec,
					prGetTsfCtxt->ucTWTFlowId);

		if (prTWTFlow == NULL) {
			DBGLOG(TWT_PLANNER, ERROR, "prTWTFlow is NULL.\n");

			kalMemFree(prGetTsfCtxt,
				VIR_MEM_TYPE, sizeof(*prGetTsfCtxt));

			return;
		}

		prGetTsfCtxt->rTWTParams.u8TWT =
			u8CurTsf + TSF_OFFSET_FOR_AGRT_ADD;

		prTWTParams = &(prTWTFlow->rTWTParams);

		kalMemCopy(prTWTParams, &(prGetTsfCtxt->rTWTParams),
			sizeof(struct _TWT_PARAMS_T));

		/* Start the process to nego for a new agreement */
		twtPlannerSendReqStart(prAdapter,
			prStaRec, prGetTsfCtxt->ucTWTFlowId);

		break;
	}
	case TWT_GET_TSF_FOR_RESUME_AGRT:
	{
		uint8_t ucNextTWTSize = NEXT_TWT_SUBFIELD_64_BITS;
		uint64_t u8NextTWT = u8CurTsf + TSF_OFFSET_FOR_AGRT_RESUME;

		/* Adjust next TWT if 'Flexible TWT Sched' is not supported */
		if (!HE_IS_MAC_CAP_FLEXIBLE_TWT_SHDL(
			prStaRec->ucHeMacCapInfo)) {
			u8NextTWT = twtPlannerAdjustNextTWT(prAdapter,
					prBssInfo->ucBssIndex,
					prGetTsfCtxt->ucTWTFlowId,
					u8NextTWT);
		}

		/* Start the process to resume this TWT agreement */
		twtPlannerSendReqResume(prAdapter,
			prStaRec, prGetTsfCtxt->ucTWTFlowId,
			u8NextTWT, ucNextTWTSize);

		break;
	}

	default:
		DBGLOG(TWT_PLANNER, ERROR,
			"Unknown reason to get TSF %u\n",
			prGetTsfCtxt->ucReason);
		break;
	}

	/* free memory */
	kalMemFree(prGetTsfCtxt, VIR_MEM_TYPE, sizeof(*prGetTsfCtxt));
}

static uint32_t
twtPlannerGetCurrentTSF(
	struct ADAPTER *prAdapter,
	struct BSS_INFO *prBssInfo,
	void *pvSetBuffer,
	uint32_t u4SetBufferLen)
{
	uint32_t rWlanStatus = WLAN_STATUS_SUCCESS;
	struct _EXT_CMD_GET_MAC_INFO_T *prMacInfoCmd;
	struct _EXTRA_ARG_TSF_T *prTsfArg;

	prMacInfoCmd = cnmMemAlloc(prAdapter, RAM_TYPE_MSG,
		sizeof(struct _EXT_CMD_GET_MAC_INFO_T));
	if (!prMacInfoCmd) {
		DBGLOG(TWT_PLANNER, ERROR,
			"Alloc _EXT_CMD_GET_MAC_INFO_T FAILED.\n");
		return WLAN_STATUS_FAILURE;
	}

	prTsfArg = &(prMacInfoCmd->rExtraArgument.rTsfArg);
	prMacInfoCmd->u2MacInfoId = CPU_TO_LE16(MAC_INFO_TYPE_TSF);
	prTsfArg->ucHwBssidIndex = prBssInfo->ucOwnMacIndex;

	rWlanStatus = wlanSendSetQueryExtCmd(prAdapter,
					CMD_ID_LAYER_0_EXT_MAGIC_NUM,
					EXT_CMD_ID_GET_MAC_INFO,
					FALSE,
					TRUE,
					FALSE,
					twtPlannerGetTsfDone,
					NULL,
					sizeof(struct _EXT_CMD_GET_MAC_INFO_T),
					(uint8_t *) (prMacInfoCmd),
					pvSetBuffer, u4SetBufferLen);

	cnmMemFree(prAdapter, prMacInfoCmd);

	return rWlanStatus;
}

void twtPlannerSetParams(
	struct ADAPTER *prAdapter, struct MSG_HDR *prMsgHdr)
{
	struct _MSG_TWT_PARAMS_SET_T *prTWTParamSetMsg;
	struct _TWT_CTRL_T rTWTCtrl, *prTWTCtrl = &rTWTCtrl;
	struct BSS_INFO *prBssInfo;
	struct STA_RECORD *prStaRec;
	uint8_t ucBssIdx, ucFlowId;
	uint8_t ucFlowId_real;

	ASSERT(prAdapter);
	ASSERT(prMsgHdr);

	prTWTParamSetMsg = (struct _MSG_TWT_PARAMS_SET_T *) prMsgHdr;
	kalMemCopy(prTWTCtrl, &prTWTParamSetMsg->rTWTCtrl, sizeof(*prTWTCtrl));

	cnmMemFree(prAdapter, prMsgHdr);

	/* Find the BSS info */
	ucBssIdx = prTWTCtrl->ucBssIdx;
	prBssInfo = GET_BSS_INFO_BY_INDEX(prAdapter, ucBssIdx);

	if (prBssInfo->eCurrentOPMode != OP_MODE_INFRASTRUCTURE ||
		prBssInfo->eConnectionState != MEDIA_STATE_CONNECTED) {
		DBGLOG(TWT_PLANNER, ERROR,
			"Current op mode %d connection state %d\n",
			prBssInfo->eCurrentOPMode, prBssInfo->eConnectionState);
		return;
	}

	/* Get the STA Record */
	prStaRec = prBssInfo->prStaRecOfAP;
	if (!prStaRec) {
		DBGLOG(TWT_PLANNER, ERROR, "No AP STA Record\n");
		return;
	}

	/* If bypassing TWT nego, this ctrl param is treated as a TWT agrt*/
	if (IS_TWT_PARAM_ACTION_ADD_BYPASS(prTWTCtrl->ucCtrlAction)) {
		struct _TWT_GET_TSF_CONTEXT_T *prGetTsfCtxt =
			kalMemAlloc(sizeof(struct _TWT_GET_TSF_CONTEXT_T),
				VIR_MEM_TYPE);
		if (prGetTsfCtxt == NULL) {
			DBGLOG(TWT_PLANNER, ERROR, "mem alloc failed\n");
			return;
		}

		prGetTsfCtxt->ucReason = TWT_GET_TSF_FOR_ADD_AGRT_BYPASS;
		prGetTsfCtxt->ucBssIdx = ucBssIdx;
		prGetTsfCtxt->ucTWTFlowId = prTWTCtrl->ucTWTFlowId;
		prGetTsfCtxt->fgIsOid = FALSE;
		kalMemCopy(&(prGetTsfCtxt->rTWTParams),
				&(prTWTCtrl->rTWTParams),
				sizeof(struct _TWT_PARAMS_T));
		twtPlannerGetCurrentTSF(prAdapter,
			prBssInfo, prGetTsfCtxt, sizeof(*prGetTsfCtxt));

		return;
	}

#if (CFG_SUPPORT_TWT_HOTSPOT_AC == 1)
	if (!IS_FEATURE_ENABLED(prAdapter->rWifiVar.ucTWTRequester)) {
		DBGLOG(TWT_PLANNER, ERROR,
			"User config of TWT req %u\n",
			prAdapter->rWifiVar.ucTWTRequester);

		return;
	}

	if (!HE_IS_MAC_CAP_TWT_RSP(prStaRec->ucHeMacCapInfo) &&
		(prStaRec->ucTWTHospotSupport == FALSE))	{
		DBGLOG(TWT_PLANNER, ERROR,
			"Peer cap 0x%x remote AP of TWT hotspot support %d\n",
			prStaRec->ucHeMacCapInfo[0],
			prStaRec->ucTWTHospotSupport);

		return;
	}
#else
	/* Check if peer has TWT responder capability and local config */
	if (!HE_IS_MAC_CAP_TWT_RSP(prStaRec->ucHeMacCapInfo) ||
		!IS_FEATURE_ENABLED(prAdapter->rWifiVar.ucTWTRequester)) {
		DBGLOG(TWT_PLANNER, ERROR,
			"Peer cap 0x%x user config of TWT req %u\n",
			prStaRec->ucHeMacCapInfo[0],
			prAdapter->rWifiVar.ucTWTRequester);

		return;
	}
#endif

	/* For COEX concern, suppose only 5G is allowed */
	if ((prAdapter->rWifiVar.ucTWTStaBandBitmap & prBssInfo->eBand)
		!= prBssInfo->eBand) {
		DBGLOG(TWT_PLANNER, ERROR,
			"TWT BAND support bitmaps(%u)!=%u\n",
			prAdapter->rWifiVar.ucTWTStaBandBitmap,
			prBssInfo->eBand);

		return;
	}

	/* No TWT activity under CTIA mode */
	if (prAdapter->fgInCtiaMode == TRUE) {
		DBGLOG(TWT_PLANNER, ERROR,
			"No TWT under CTIA(%d)n", prAdapter->fgInCtiaMode);

		return;
	}

	ucFlowId = prTWTCtrl->ucTWTFlowId;

	ucFlowId_real = ucFlowId;

	switch (prTWTCtrl->ucCtrlAction) {
	case TWT_PARAM_ACTION_ADD:
		if (twtPlannerDrvAgrtFind(
			prAdapter, ucBssIdx, ucFlowId, &ucFlowId_real)
				>= TWT_AGRT_MAX_NUM) {

			struct _TWT_GET_TSF_CONTEXT_T *prGetTsfCtxt =
				kalMemAlloc(
					sizeof(struct _TWT_GET_TSF_CONTEXT_T),
					VIR_MEM_TYPE);
			if (prGetTsfCtxt == NULL) {
				DBGLOG(TWT_PLANNER, ERROR,
					"mem alloc failed\n");
				return;
			}

			prGetTsfCtxt->ucReason = TWT_GET_TSF_FOR_ADD_AGRT;
			prGetTsfCtxt->ucBssIdx = ucBssIdx;
			prGetTsfCtxt->ucTWTFlowId = prTWTCtrl->ucTWTFlowId;
			prGetTsfCtxt->fgIsOid = FALSE;
			kalMemCopy(&(prGetTsfCtxt->rTWTParams),
					&(prTWTCtrl->rTWTParams),
					sizeof(struct _TWT_PARAMS_T));
			twtPlannerGetCurrentTSF(prAdapter, prBssInfo,
				prGetTsfCtxt, sizeof(*prGetTsfCtxt));

			return;
		}

		DBGLOG(TWT_PLANNER, ERROR,
			"BSS %u TWT flow %u already exists\n",
			ucBssIdx, ucFlowId);
		break;

	case TWT_PARAM_ACTION_DEL:
		if (twtPlannerDrvAgrtFind(
			prAdapter, ucBssIdx, ucFlowId, &ucFlowId_real)
				< TWT_AGRT_MAX_NUM) {
			/* Start the process to tear down this TWT agreement */
			ucFlowId = ucFlowId_real;

			twtPlannerSendReqTeardown(prAdapter,
				prStaRec, ucFlowId);
		} else {
			DBGLOG(TWT_PLANNER, ERROR,
				"BSS %u TWT flow %u doesn't exist\n",
				ucBssIdx, ucFlowId);
		}
		break;

	case TWT_PARAM_ACTION_SUSPEND:
		if (twtPlannerDrvAgrtFind(
			prAdapter, ucBssIdx, ucFlowId, &ucFlowId_real)
				< TWT_AGRT_MAX_NUM) {
			/* Start the process to suspend this TWT agreement */
			ucFlowId = ucFlowId_real;

			twtPlannerSendReqSuspend(prAdapter,
				prStaRec, ucFlowId);
		} else {
			DBGLOG(TWT_PLANNER, ERROR,
				"BSS %u TWT flow %u doesn't exist\n",
				ucBssIdx, ucFlowId);
		}
		break;

	case TWT_PARAM_ACTION_RESUME:
		if (twtPlannerDrvAgrtFind(
			prAdapter, ucBssIdx, ucFlowId, &ucFlowId_real)
				< TWT_AGRT_MAX_NUM) {
			struct _TWT_GET_TSF_CONTEXT_T *prGetTsfCtxt =
				kalMemAlloc(
					sizeof(struct _TWT_GET_TSF_CONTEXT_T),
					VIR_MEM_TYPE);
			if (prGetTsfCtxt == NULL) {
				DBGLOG(TWT_PLANNER, ERROR,
					"mem alloc failed\n");
				return;
			}

			ucFlowId = ucFlowId_real;

			prGetTsfCtxt->ucReason = TWT_GET_TSF_FOR_RESUME_AGRT;
			prGetTsfCtxt->ucBssIdx = ucBssIdx;
			prGetTsfCtxt->ucTWTFlowId = prTWTCtrl->ucTWTFlowId;
			prGetTsfCtxt->fgIsOid = FALSE;
			twtPlannerGetCurrentTSF(prAdapter, prBssInfo,
				prGetTsfCtxt, sizeof(*prGetTsfCtxt));
		} else {
			DBGLOG(TWT_PLANNER, ERROR,
				"BSS %u TWT flow %u doesn't exist\n",
				ucBssIdx, ucFlowId);
		}

		break;

	default:
		DBGLOG(TWT_PLANNER, ERROR,
			"Action %u not supported\n", prTWTCtrl->ucCtrlAction);
		break;
	}
}

void twtPlannerRxNegoResult(
	struct ADAPTER *prAdapter,
	struct MSG_HDR *prMsgHdr)
{
	struct _MSG_TWT_REQFSM_IND_RESULT_T *prTWTFsmResultMsg;
	struct STA_RECORD *prStaRec;
	struct BSS_INFO *prBssInfo;
	uint8_t ucTWTFlowId;
	struct _TWT_PARAMS_T *prTWTResult, *prTWTParams;
	struct _TWT_FLOW_T *prTWTFlow;

	ASSERT(prAdapter);
	ASSERT(prMsgHdr);

	prTWTFsmResultMsg = (struct _MSG_TWT_REQFSM_IND_RESULT_T *) prMsgHdr;
	prStaRec = prTWTFsmResultMsg->prStaRec;
	ucTWTFlowId = prTWTFsmResultMsg->ucTWTFlowId;

	if ((!prStaRec) || (prStaRec->fgIsInUse == FALSE)) {
		cnmMemFree(prAdapter, prMsgHdr);
		return;
	}

	prBssInfo = GET_BSS_INFO_BY_INDEX(prAdapter, prStaRec->ucBssIndex);

	cnmMemFree(prAdapter, prMsgHdr);

	if (!IS_AP_STA(prStaRec)) {
		DBGLOG(TWT_PLANNER, ERROR,
			"Rx nego result: invalid STA Type %d\n",
			prStaRec->eStaType);
		return;
	}

	prTWTFlow = &(prStaRec->arTWTFlow[ucTWTFlowId]);
	prTWTResult = &(prTWTFlow->rTWTPeerParams);

	switch (prTWTResult->ucSetupCmd) {
	case TWT_SETUP_CMD_ACCEPT:
		/* Update agreement table */
		twtPlannerAddAgrtTbl(prAdapter, prBssInfo, prStaRec,
			prTWTResult, ucTWTFlowId, FALSE,
			NULL, NULL /* handle TWT cmd timeout? */);

#if (CFG_DISABLE_SCAN_UNDER_TWT == 1)
		/* Disable SCAN during TWT activity */
		prAdapter->fgEnOnlineScan = FALSE;
#endif

		break;

	case TWT_SETUP_CMD_ALTERNATE:
	case TWT_SETUP_CMD_DICTATE:
		/* Use AP's suggestions */
		prTWTParams = &(prTWTFlow->rTWTParams);
		kalMemCopy(prTWTParams,
			prTWTResult, sizeof(struct _TWT_PARAMS_T));
		prTWTParams->ucSetupCmd = TWT_SETUP_CMD_SUGGEST;
		prTWTParams->fgReq = 1;
		twtPlannerSendReqStart(prAdapter, prStaRec, ucTWTFlowId);
		break;

	case TWT_SETUP_CMD_REJECT:
		/* Clear TWT flow in StaRec */
		break;

	default:
		DBGLOG(TWT_PLANNER, ERROR,
			"Unknown setup command %u\n", prTWTResult->ucSetupCmd);
		ASSERT(0);
		break;

	}
}

void twtPlannerSuspendDone(
	struct ADAPTER *prAdapter, struct MSG_HDR *prMsgHdr)
{
	struct _MSG_TWT_REQFSM_IND_RESULT_T *prTWTFsmResultMsg;
	struct STA_RECORD *prStaRec;
	struct BSS_INFO *prBssInfo;
	uint8_t ucTWTFlowId;

	ASSERT(prAdapter);
	ASSERT(prMsgHdr);

	prTWTFsmResultMsg = (struct _MSG_TWT_REQFSM_IND_RESULT_T *) prMsgHdr;
	prStaRec = prTWTFsmResultMsg->prStaRec;
	ucTWTFlowId = prTWTFsmResultMsg->ucTWTFlowId;

	if ((!prStaRec) || (prStaRec->fgIsInUse == FALSE)) {
		cnmMemFree(prAdapter, prMsgHdr);
		return;
	}

	prBssInfo = GET_BSS_INFO_BY_INDEX(prAdapter, prStaRec->ucBssIndex);

	cnmMemFree(prAdapter, prMsgHdr);

	if (!IS_AP_STA(prStaRec)) {
		DBGLOG(TWT_PLANNER, ERROR,
			"Rx suspend result: invalid STA Type %d\n",
			prStaRec->eStaType);
		return;
	}

	/* Delete only FW TWT agreement entry */
	twtPlannerDelAgrtTbl(prAdapter, prBssInfo, prStaRec,
		ucTWTFlowId, FALSE,
		NULL, NULL /* handle TWT cmd timeout? */, FALSE);

	/* Teardown FW TWT agreement entry */
	twtPlannerTeardownAgrtTbl(prAdapter, prStaRec,
		FALSE, NULL, NULL /* handle TWT cmd timeout? */);

}

void twtPlannerResumeDone(
	struct ADAPTER *prAdapter, struct MSG_HDR *prMsgHdr)
{
	struct _MSG_TWT_REQFSM_IND_RESULT_T *prTWTFsmResultMsg;
	struct STA_RECORD *prStaRec;
	struct BSS_INFO *prBssInfo;
	uint8_t ucTWTFlowId;

	ASSERT(prAdapter);
	ASSERT(prMsgHdr);

	prTWTFsmResultMsg = (struct _MSG_TWT_REQFSM_IND_RESULT_T *) prMsgHdr;
	prStaRec = prTWTFsmResultMsg->prStaRec;
	ucTWTFlowId = prTWTFsmResultMsg->ucTWTFlowId;

	if ((!prStaRec) || (prStaRec->fgIsInUse == FALSE)) {
		cnmMemFree(prAdapter, prMsgHdr);
		return;
	}

	prBssInfo = GET_BSS_INFO_BY_INDEX(prAdapter, prStaRec->ucBssIndex);

	cnmMemFree(prAdapter, prMsgHdr);

	if (!IS_AP_STA(prStaRec)) {
		DBGLOG(TWT_PLANNER, ERROR,
			"Rx suspend result: invalid STA Type %d\n",
			prStaRec->eStaType);
		return;
	}

	/* Add back the FW TWT agreement entry */
	twtPlannerResumeAgrtTbl(prAdapter, prBssInfo, prStaRec,
		ucTWTFlowId, FALSE,
		NULL, NULL /* handle TWT cmd timeout? */);

}

void twtPlannerTeardownDone(
	struct ADAPTER *prAdapter, struct MSG_HDR *prMsgHdr)
{
	struct _MSG_TWT_REQFSM_IND_RESULT_T *prTWTFsmResultMsg;
	struct STA_RECORD *prStaRec;
	struct BSS_INFO *prBssInfo;
	uint8_t ucTWTFlowId;

	ASSERT(prAdapter);
	ASSERT(prMsgHdr);

	prTWTFsmResultMsg = (struct _MSG_TWT_REQFSM_IND_RESULT_T *) prMsgHdr;
	prStaRec = prTWTFsmResultMsg->prStaRec;
	ucTWTFlowId = prTWTFsmResultMsg->ucTWTFlowId;

	if ((!prStaRec) || (prStaRec->fgIsInUse == FALSE)) {
		cnmMemFree(prAdapter, prMsgHdr);
		return;
	}

	prBssInfo = GET_BSS_INFO_BY_INDEX(prAdapter, prStaRec->ucBssIndex);

	cnmMemFree(prAdapter, prMsgHdr);

	if (!IS_AP_STA(prStaRec)) {
		DBGLOG(TWT_PLANNER, ERROR,
			"Rx teardown result: invalid STA Type %d\n",
			prStaRec->eStaType);
		return;
	}

	/* Delete driver & FW TWT agreement entry */
	twtPlannerDelAgrtTbl(prAdapter, prBssInfo, prStaRec,
		ucTWTFlowId, FALSE,
		NULL, NULL /* handle TWT cmd timeout? */, TRUE);

	/* Teardown FW TWT agreement entry */
	twtPlannerTeardownAgrtTbl(prAdapter, prStaRec,
		FALSE, NULL, NULL /* handle TWT cmd timeout? */);

#if (CFG_DISABLE_SCAN_UNDER_TWT == 1)
	/* Enable SCAN after TWT agrt has been tear down */
	prAdapter->fgEnOnlineScan = TRUE;
#endif
}

void twtPlannerRxInfoFrm(
	struct ADAPTER *prAdapter, struct MSG_HDR *prMsgHdr)
{
	struct _MSG_TWT_REQFSM_IND_INFOFRM_T *prTWTFsmInfoFrmMsg;
	struct STA_RECORD *prStaRec;
	struct BSS_INFO *prBssInfo;
	uint8_t ucTWTFlowId;
	struct _NEXT_TWT_INFO_T rNextTWTInfo;

	ASSERT(prAdapter);
	ASSERT(prMsgHdr);

	prTWTFsmInfoFrmMsg = (struct _MSG_TWT_REQFSM_IND_INFOFRM_T *) prMsgHdr;
	prStaRec = prTWTFsmInfoFrmMsg->prStaRec;
	ucTWTFlowId = prTWTFsmInfoFrmMsg->ucTWTFlowId;

	if ((!prStaRec) || (prStaRec->fgIsInUse == FALSE)) {
		cnmMemFree(prAdapter, prMsgHdr);
		return;
	}

	prBssInfo = GET_BSS_INFO_BY_INDEX(prAdapter, prStaRec->ucBssIndex);

	if (!IS_AP_STA(prStaRec)) {
		DBGLOG(TWT_PLANNER, ERROR,
			"Rx info frame: invalid STA Type %d\n",
			prStaRec->eStaType);
		cnmMemFree(prAdapter, prMsgHdr);
		return;
	}

	kalMemCopy(&(rNextTWTInfo), &(prTWTFsmInfoFrmMsg->rNextTWTInfo),
		sizeof(struct _NEXT_TWT_INFO_T));

	cnmMemFree(prAdapter, prMsgHdr);

	/* Modify the TWT agreement entry */
	twtPlannerModifyAgrtTbl(prAdapter, prBssInfo, prStaRec,
		&rNextTWTInfo, ucTWTFlowId, FALSE,
		NULL, NULL /* handle TWT cmd timeout? */);

}

#if (CFG_SUPPORT_TWT_HOTSPOT == 1)
void twtHotspotPlannerSetParams(
	struct ADAPTER *prAdapter,
	struct MSG_HDR *prMsgHdr)
{
	struct _MSG_TWT_HOTSPOT_PARAMS_SET_T *prTWTParamSetMsg;
	struct _TWT_HOTSPOT_CTRL_T rTWTCtrl, *prTWTCtrl = &rTWTCtrl;
	struct _TWT_GET_TSF_CONTEXT_T *prGetTsfCtxt = NULL;
	struct BSS_INFO *prBssInfo = NULL;

	ASSERT(prAdapter);
	ASSERT(prMsgHdr);

	prTWTParamSetMsg = (struct _MSG_TWT_HOTSPOT_PARAMS_SET_T *) prMsgHdr;
	kalMemCopy(prTWTCtrl, &prTWTParamSetMsg->rTWTCtrl, sizeof(*prTWTCtrl));

	cnmMemFree(prAdapter, prMsgHdr);

	if (prTWTCtrl->ucIsReject == TRUE) {
		/* It is a reject, no need to
		*  calculate the nearest target tsf
		*/
		DBGLOG(TWT_RESPONDER, ERROR,
				"[TWT_RESP]direct reject\n");

		twtHotspotRespFsmRunEventRxSetup(
			prAdapter,
			(void *)prTWTCtrl);
	} else {
		/* It is an accept, use current tsf for
		*  the nearest target tsf
		*/
		prGetTsfCtxt =
			kalMemAlloc(
				sizeof(
					struct _TWT_GET_TSF_CONTEXT_T),
					VIR_MEM_TYPE);

		if (prGetTsfCtxt == NULL) {
			DBGLOG(TWT_PLANNER, ERROR,
				"[TWT_RESP]mem alloc failed\n");

			return;
		}

		prGetTsfCtxt->ucReason = TWT_GET_TSF_FOR_ADD_AGRT;
		prGetTsfCtxt->ucBssIdx = prTWTCtrl->ucBssIdx;
		prGetTsfCtxt->ucTWTFlowId = prTWTCtrl->ucTWTFlowId;
		prGetTsfCtxt->fgIsOid = FALSE;

		prBssInfo = GET_BSS_INFO_BY_INDEX(
					prAdapter,
					prGetTsfCtxt->ucBssIdx);

		ASSERT(prBssInfo);

		kalMemCopy(
			&(prGetTsfCtxt->rTWTParams),
			&(prTWTCtrl->rTWTParams),
			sizeof(struct _TWT_PARAMS_T));

		DBGLOG(TWT_RESPONDER, ERROR,
				"[TWT_RESP]GetCurrentTSF\n");

		twtHotspotPlannerGetCurrentTSF(
			prAdapter, prBssInfo,
			prGetTsfCtxt, sizeof(*prGetTsfCtxt));
	}
}

void twtHotspotPlannerSetupAgrtToFW(
	struct ADAPTER *prAdapter,
	struct MSG_HDR *prMsgHdr)
{
	struct _MSG_TWT_HOTSPOT_PARAMS_SET_T *prTWTParamSetMsg;
	struct _TWT_HOTSPOT_CTRL_T rTWTCtrl, *prTWTCtrl = &rTWTCtrl;

	ASSERT(prAdapter);
	ASSERT(prMsgHdr);

	prTWTParamSetMsg = (struct _MSG_TWT_HOTSPOT_PARAMS_SET_T *) prMsgHdr;
	kalMemCopy(prTWTCtrl, &prTWTParamSetMsg->rTWTCtrl, sizeof(*prTWTCtrl));

	cnmMemFree(prAdapter, prMsgHdr);

	twtHotspotPlannerAddAgrtTbl(
			prAdapter,
			prTWTCtrl->prStaRec,
			prTWTCtrl->ucTWTFlowId,
			FALSE,
			NULL,
			NULL);
}

void twtHotspotPlannerTeardownToFW(
	struct ADAPTER *prAdapter,
	struct MSG_HDR *prMsgHdr)
{
	struct _MSG_TWT_HOTSPOT_PARAMS_SET_T *prTWTParamSetMsg;
	struct _TWT_HOTSPOT_CTRL_T rTWTCtrl, *prTWTCtrl = &rTWTCtrl;

	ASSERT(prAdapter);
	ASSERT(prMsgHdr);

	prTWTParamSetMsg = (struct _MSG_TWT_HOTSPOT_PARAMS_SET_T *) prMsgHdr;
	kalMemCopy(prTWTCtrl, &prTWTParamSetMsg->rTWTCtrl, sizeof(*prTWTCtrl));

	cnmMemFree(prAdapter, prMsgHdr);

	twtHotspotPlannerTeardownSta(
			prAdapter,
			prTWTCtrl->prStaRec,
			prTWTCtrl->ucTWTFlowId,
			FALSE,
			NULL,
			NULL);
}

uint32_t twtHotspotPlannerGetCurrentTSF(
	struct ADAPTER *prAdapter,
	struct BSS_INFO *prBssInfo,
	void *pvSetBuffer,
	uint32_t u4SetBufferLen)
{
	uint32_t rWlanStatus = WLAN_STATUS_SUCCESS;
	struct _EXT_CMD_GET_MAC_INFO_T *prMacInfoCmd;
	struct _EXTRA_ARG_TSF_T *prTsfArg;

	prMacInfoCmd = cnmMemAlloc(prAdapter, RAM_TYPE_MSG,
		sizeof(struct _EXT_CMD_GET_MAC_INFO_T));

	if (!prMacInfoCmd) {
		DBGLOG(TWT_RESPONDER, ERROR,
			"[TWT_RESP]Alloc _EXT_CMD_GET_MAC_INFO_T FAILED.\n");

		return WLAN_STATUS_FAILURE;
	}

	prTsfArg = &(prMacInfoCmd->rExtraArgument.rTsfArg);
	prMacInfoCmd->u2MacInfoId = CPU_TO_LE16(MAC_INFO_TYPE_TSF);
	prTsfArg->ucHwBssidIndex = prBssInfo->ucOwnMacIndex;

	rWlanStatus = wlanSendSetQueryExtCmd(prAdapter,
					CMD_ID_LAYER_0_EXT_MAGIC_NUM,
					EXT_CMD_ID_GET_MAC_INFO,
					FALSE,
					TRUE,
					FALSE,
					twtHotspotPlannerGetTsfDone,
					NULL,
					sizeof(struct _EXT_CMD_GET_MAC_INFO_T),
					(uint8_t *) (prMacInfoCmd),
					pvSetBuffer, u4SetBufferLen);

	cnmMemFree(prAdapter, prMacInfoCmd);

	return rWlanStatus;
}

void twtHotspotPlannerGetTsfDone(
	struct ADAPTER *prAdapter,
	struct CMD_INFO *prCmdInfo,
	uint8_t *pucEventBuf)
{
	struct EXT_EVENT_MAC_INFO_T *prEventMacInfo = NULL;
	struct _TWT_GET_TSF_CONTEXT_T *prGetTsfCtxt = NULL;
	struct STA_RECORD *prStaRec = NULL;
	struct TSF_RESULT_T *prTsfResult = NULL;
	uint64_t u8CurTsf = 0;
	struct _TWT_HOTSPOT_STA_NODE *prTWTHotspotStaNode = NULL;
	struct _TWT_HOTSPOT_CTRL_T *prTWTHotspotCtrl = NULL;
	struct _TWT_PARAMS_T *prTWTHotspotParam = NULL;

	ASSERT(prAdapter);
	ASSERT(prCmdInfo);
	ASSERT(pucEventBuf);

	if (!pucEventBuf) {
		DBGLOG(TWT_RESPONDER, ERROR,
			"[TWT_RESP]pucEventBuf is NULL.\n");

		return;
	}

	if (!prCmdInfo->pvInformationBuffer) {
		DBGLOG(TWT_RESPONDER, ERROR,
			"[TWT_RESP]prCmdInfo->pvInformationBuffer is NULL.\n");

		return;
	}

	prEventMacInfo = (struct EXT_EVENT_MAC_INFO_T *) (pucEventBuf);

	prGetTsfCtxt = (struct _TWT_GET_TSF_CONTEXT_T *)
		prCmdInfo->pvInformationBuffer;

	ASSERT(prGetTsfCtxt);

	if (prGetTsfCtxt->ucBssIdx >= MAX_BSSID_NUM) {
		DBGLOG(TWT_RESPONDER, ERROR,
		   "[TWT_RESP]prGetTsfCtxt->ucBssIdx is Invalid.\n");

		return;
	}
	twtHotspotGetStaRecByFlowId(
		prAdapter,
		prGetTsfCtxt->ucBssIdx,
		prGetTsfCtxt->ucTWTFlowId,
		&prStaRec);

	if (prStaRec != NULL) {
		prTsfResult = &(prEventMacInfo->rMacInfoResult.rTsfResult);

		u8CurTsf = LE32_TO_CPU(prTsfResult->u4TsfBitsLow) |
			(((uint64_t)(LE32_TO_CPU(prTsfResult->
							u4TsfBitsHigh))) << 32);

		DBGLOG(TWT_RESPONDER, ERROR,
			"[TWT_RESP]GetTSF u8CurTsf=%llx, TsfLow=%x, TsfHigh=%x\n",
			u8CurTsf,
			LE32_TO_CPU(prTsfResult->u4TsfBitsLow),
			LE32_TO_CPU(prTsfResult->u4TsfBitsHigh));

		prTWTHotspotStaNode = prStaRec->prTWTHotspotStaNode;

		prTWTHotspotCtrl = &prStaRec->TWTHotspotCtrl;

		prTWTHotspotParam = &prTWTHotspotCtrl->rTWTParams;

		twtHotspotGetNearestTargetTSF(
			prAdapter, prStaRec, prTWTHotspotStaNode, u8CurTsf);

		prTWTHotspotParam->u8TWT =
			prTWTHotspotStaNode->twt_assigned_tsf;

		DBGLOG(TWT_RESPONDER, ERROR,
			"[TWT_RESP]IsReject=%d SetupCmd=%d\n",
			prTWTHotspotCtrl->ucIsReject,
			prTWTHotspotParam->ucSetupCmd);

		twtHotspotRespFsmRunEventRxSetup(
			prAdapter,
			(void *)prTWTHotspotCtrl);
	}

	/* free memory */
	kalMemFree(prGetTsfCtxt, VIR_MEM_TYPE, sizeof(*prGetTsfCtxt));
}

uint32_t
twtHotspotPlannerAddAgrtTbl(
	struct ADAPTER *prAdapter,
	struct STA_RECORD *prStaRec,
	uint8_t ucFlowId,
	uint8_t fgIsOid,
	PFN_CMD_DONE_HANDLER pfCmdDoneHandler,
	PFN_CMD_TIMEOUT_HANDLER pfCmdTimeoutHandler)
{
	uint32_t rWlanStatus = WLAN_STATUS_SUCCESS;
	struct _TWT_HOTSPOT_STA_NODE *prTWTHotspotStaNode = NULL;
	struct _EXT_CMD_TWT_ARGT_UPDATE_T *prTWTAgrtUpdate;

	prTWTHotspotStaNode = prStaRec->prTWTHotspotStaNode;

	if (!prTWTHotspotStaNode) {
		DBGLOG(
			TWT_RESPONDER,
			ERROR,
			"[TWT_RESP]No sta node in sta record for flow id = %d\n",
			ucFlowId);

		return WLAN_STATUS_FAILURE;
	}

	prTWTAgrtUpdate = cnmMemAlloc(
		prAdapter,
		RAM_TYPE_MSG,
		sizeof(struct _EXT_CMD_TWT_ARGT_UPDATE_T));

	if (!prTWTAgrtUpdate) {
		DBGLOG(
			TWT_RESPONDER,
			ERROR,
			"[TWT_RESP]Allocate _EXT_CMD_TWT_ARGT_UPDATE_T ==> FAILED.\n");

		return WLAN_STATUS_FAILURE;
	}

	if (prTWTHotspotStaNode->flow_id != ucFlowId) {
		DBGLOG(
			TWT_RESPONDER,
			ERROR,
			"[TWT_RESP]Flow id mismatch %d vs %d\n",
			prTWTHotspotStaNode->flow_id, ucFlowId);

		cnmMemFree(prAdapter, prTWTAgrtUpdate);

		return WLAN_STATUS_FAILURE;
	}

	prTWTAgrtUpdate->ucAgrtTblIdx = prTWTHotspotStaNode->agrt_tbl_idx;
	prTWTAgrtUpdate->ucAgrtCtrlFlag = TWT_AGRT_CTRL_ADD;
	prTWTAgrtUpdate->ucOwnMacId = prTWTHotspotStaNode->own_mac_idx;
	prTWTAgrtUpdate->ucFlowId = prTWTHotspotStaNode->flow_id;
	prTWTAgrtUpdate->u2PeerIdGrpId =
		(prStaRec) ? CPU_TO_LE16(prStaRec->ucWlanIndex) : 1;
	prTWTAgrtUpdate->ucAgrtSpDuration =
		prTWTHotspotStaNode->agrt_sp_duration;
	prTWTAgrtUpdate->ucBssIndex = prTWTHotspotStaNode->bss_idx;
	prTWTAgrtUpdate->u4AgrtSpStartTsfLow =
		CPU_TO_LE32(prTWTHotspotStaNode->twt_assigned_tsf & 0xFFFFFFFF);
	prTWTAgrtUpdate->u4AgrtSpStartTsfHigh =
		CPU_TO_LE32((uint32_t)
		(prTWTHotspotStaNode->twt_assigned_tsf >> 32));
	prTWTAgrtUpdate->u2AgrtSpWakeIntvlMantissa =
		CPU_TO_LE16(prTWTHotspotStaNode->agrt_sp_wake_intvl_mantissa);
	prTWTAgrtUpdate->ucAgrtSpWakeIntvlExponent =
	prTWTHotspotStaNode->agrt_sp_wake_intvl_exponent;
	prTWTAgrtUpdate->ucIsRoleAp = TWT_ROLE_HOTSPOT;  /* TWT hotspot role */
	prTWTAgrtUpdate->ucAgrtParaBitmap = 0;
	prTWTAgrtUpdate->ucGrpMemberCnt = 0;

	rWlanStatus = wlanSendSetQueryExtCmd(prAdapter,
		CMD_ID_LAYER_0_EXT_MAGIC_NUM,
		EXT_CMD_ID_TWT_AGRT_UPDATE,
		TRUE,
		FALSE,
		fgIsOid,
		pfCmdDoneHandler,
		pfCmdTimeoutHandler,
		sizeof(struct _EXT_CMD_TWT_ARGT_UPDATE_T),
		(uint8_t *) (prTWTAgrtUpdate),
		NULL, 0);

	DBGLOG(
		TWT_RESPONDER,
		ERROR,
		"[TWT_RESP]AgrtTblIdx = %d, OwnMacId = %d, FlowId = %d, PeerIdGrpId = %d, BssIdx = %d, Status=%x\n",
		prTWTAgrtUpdate->ucAgrtTblIdx,
		prTWTAgrtUpdate->ucOwnMacId,
		prTWTAgrtUpdate->ucFlowId,
		prTWTAgrtUpdate->u2PeerIdGrpId,
		prTWTAgrtUpdate->ucBssIndex,
		rWlanStatus);

	DBGLOG(
		TWT_RESPONDER,
		ERROR,
		"[TWT_RESP]SpDuration=%d, TsfLow=%x, TsfHigh=%x, Mantissa=%d, Exp=%d\n",
		prTWTAgrtUpdate->ucAgrtSpDuration,
		prTWTAgrtUpdate->u4AgrtSpStartTsfLow,
		prTWTAgrtUpdate->u4AgrtSpStartTsfHigh,
		prTWTAgrtUpdate->u2AgrtSpWakeIntvlMantissa,
		prTWTAgrtUpdate->ucAgrtSpWakeIntvlExponent);

	cnmMemFree(prAdapter, prTWTAgrtUpdate);

	return rWlanStatus;
}

uint32_t
twtHotspotPlannerTeardownSta(
	struct ADAPTER *prAdapter,
	struct STA_RECORD *prStaRec,
	uint8_t ucFlowId,
	uint8_t fgIsOid,
	PFN_CMD_DONE_HANDLER pfCmdDoneHandler,
	PFN_CMD_TIMEOUT_HANDLER pfCmdTimeoutHandler)
{
	uint32_t rWlanStatus = WLAN_STATUS_SUCCESS;
	struct _TWT_HOTSPOT_STA_NODE *prTWTHotspotStaNode = NULL;
	struct _EXT_CMD_TWT_ARGT_UPDATE_T *prTWTAgrtUpdate;

	prTWTHotspotStaNode = prStaRec->prTWTHotspotStaNode;

	if (prTWTHotspotStaNode == NULL) {
		/* TWT hotspot might receive teardown before disconnect */
		DBGLOG(
			TWT_PLANNER,
			ERROR,
			"[TWT_RESP]no sta node for flow id = %d\n",
			ucFlowId);

		twtHotspotRespFsmSteps(
			prAdapter,
			prStaRec,
			TWT_HOTSPOT_RESP_STATE_IDLE_BY_FORCE,
			ucFlowId,
			NULL);

		return WLAN_STATUS_SUCCESS;
	}

	prTWTAgrtUpdate = cnmMemAlloc(
		prAdapter,
		RAM_TYPE_MSG,
		sizeof(struct _EXT_CMD_TWT_ARGT_UPDATE_T));

	if (!prTWTAgrtUpdate) {
		DBGLOG(
			TWT_PLANNER,
			ERROR,
			"[TWT_RESP]Allocate _EXT_CMD_TWT_ARGT_UPDATE_T ==> FAILED.\n");

		return WLAN_STATUS_FAILURE;
	}

	if (prTWTHotspotStaNode->flow_id != ucFlowId) {
		DBGLOG(
			TWT_PLANNER,
			ERROR,
			"[TWT_RESP]Flow id mismatch %d vs %d\n",
			prTWTHotspotStaNode->flow_id, ucFlowId);

		cnmMemFree(prAdapter, prTWTAgrtUpdate);

		return WLAN_STATUS_FAILURE;
	}

	/* 1. send teardown command to F/W */
	/* 1-1. TWT_AGRT_CTRL_DELETE
	*          - deregister tsf timer & defect RRselection
	*/
	prTWTAgrtUpdate->ucAgrtTblIdx = prTWTHotspotStaNode->agrt_tbl_idx;
	prTWTAgrtUpdate->ucAgrtCtrlFlag = TWT_AGRT_CTRL_DELETE;
	prTWTAgrtUpdate->ucOwnMacId = prTWTHotspotStaNode->own_mac_idx;
	prTWTAgrtUpdate->ucFlowId = prTWTHotspotStaNode->flow_id;
	prTWTAgrtUpdate->u2PeerIdGrpId = (prStaRec) ?
		CPU_TO_LE16(prStaRec->ucWlanIndex) : 1;
	prTWTAgrtUpdate->ucIsRoleAp = TWT_ROLE_HOTSPOT;  /* TWT hotspot role */
	prTWTAgrtUpdate->ucBssIndex = prTWTHotspotStaNode->bss_idx;

	rWlanStatus = wlanSendSetQueryExtCmd(
		prAdapter,
		CMD_ID_LAYER_0_EXT_MAGIC_NUM,
		EXT_CMD_ID_TWT_AGRT_UPDATE,
		TRUE,
		FALSE,
		fgIsOid,
		pfCmdDoneHandler,
		pfCmdTimeoutHandler,
		sizeof(struct _EXT_CMD_TWT_ARGT_UPDATE_T),
		(uint8_t *) (prTWTAgrtUpdate),
		NULL, 0);

	DBGLOG(
		TWT_RESPONDER,
		ERROR,
		"[TWT_RESP]TWT_AGRT_CTRL_DELETE(%d) %d %d %d %d %d\n",
		rWlanStatus,
		prTWTAgrtUpdate->ucAgrtTblIdx,
		prTWTAgrtUpdate->ucBssIndex,
		prTWTAgrtUpdate->ucOwnMacId,
		prTWTAgrtUpdate->ucFlowId,
		prTWTAgrtUpdate->u2PeerIdGrpId);

	/* 1-2. TWT_AGRT_CTRL_TEARDOWN - reset STA to non-TWT STA */
	prTWTAgrtUpdate->ucAgrtCtrlFlag = TWT_AGRT_CTRL_TEARDOWN;

	rWlanStatus = wlanSendSetQueryExtCmd(
		prAdapter,
		CMD_ID_LAYER_0_EXT_MAGIC_NUM,
		EXT_CMD_ID_TWT_AGRT_UPDATE,
		TRUE,
		FALSE,
		fgIsOid,
		pfCmdDoneHandler,
		pfCmdTimeoutHandler,
		sizeof(struct _EXT_CMD_TWT_ARGT_UPDATE_T),
		(uint8_t *) (prTWTAgrtUpdate),
		NULL, 0);

	DBGLOG(
		TWT_RESPONDER,
		ERROR,
		"[TWT_RESP]TWT_AGRT_CTRL_TEARDOWN(%d) %d %d %d %d %d\n",
		rWlanStatus,
		prTWTAgrtUpdate->ucAgrtTblIdx,
		prTWTAgrtUpdate->ucBssIndex,
		prTWTAgrtUpdate->ucOwnMacId,
		prTWTAgrtUpdate->ucFlowId,
		prTWTAgrtUpdate->u2PeerIdGrpId);

	cnmMemFree(prAdapter, prTWTAgrtUpdate);

	/* 2. reset  this Hotsopt's STA station record */
	twtHotspotRespFsmSteps(
		prAdapter,
		prStaRec,
		TWT_HOTSPOT_RESP_STATE_IDLE,
		ucFlowId,
		NULL);

	return rWlanStatus;
}

#endif
