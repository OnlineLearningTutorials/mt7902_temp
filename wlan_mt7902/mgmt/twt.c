/* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause */
/*
 * Copyright (c) 2017 MediaTek Inc.
 */
/*! \file   "twt.c"
*   \brief  Functions for processing TWT related elements and frames.
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

static void twtFillTWTElement(
	struct _IE_TWT_T *prTWTBuf,
	uint8_t ucTWTFlowId,
	struct _TWT_PARAMS_T *prTWTParams);

static void twtParseTWTElement(
	struct _IE_TWT_T *prTWTIE,
	struct _TWT_PARAMS_T *prTWTParams);

/*******************************************************************************
*                              F U N C T I O N S
********************************************************************************
*/
/*----------------------------------------------------------------------------*/
/*!
* \brief Send TWT Setup frame (S1G action frame)
*
* \param[in] prAdapter ADAPTER structure
*            prStaRec station record structure
*
* \return none
*/
/*----------------------------------------------------------------------------*/
uint32_t twtSendSetupFrame(
	struct ADAPTER *prAdapter,
	struct STA_RECORD *prStaRec,
	uint8_t ucTWTFlowId,
	struct _TWT_PARAMS_T *prTWTParams,
	PFN_TX_DONE_HANDLER pfTxDoneHandler)
{
	struct MSDU_INFO *prMsduInfo;
	struct _ACTION_TWT_SETUP_FRAME *prTxFrame;
	struct BSS_INFO *prBssInfo;
	uint16_t u2EstimatedFrameLen;
	struct _IE_TWT_T *prTWTBuf;

	ASSERT(prAdapter);
	ASSERT(prStaRec);
	ASSERT(prTWTParams);

	prBssInfo = GET_BSS_INFO_BY_INDEX(prAdapter, prStaRec->ucBssIndex);

	ASSERT(prBssInfo);

	/* Calculate MSDU buffer length */
	u2EstimatedFrameLen = MAC_TX_RESERVED_FIELD
		+ sizeof(struct _ACTION_TWT_SETUP_FRAME);

	/* Alloc MSDU_INFO */
	prMsduInfo = (struct MSDU_INFO *)
			cnmMgtPktAlloc(prAdapter, u2EstimatedFrameLen);

	if (!prMsduInfo) {
		DBGLOG(TWT_REQUESTER, WARN,
			"No MSDU_INFO_T for sending TWT Setup Frame.\n");
		return WLAN_STATUS_RESOURCES;
	}

	kalMemZero(prMsduInfo->prPacket, u2EstimatedFrameLen);

	prTxFrame = prMsduInfo->prPacket;

	/* Fill frame ctrl */
	prTxFrame->u2FrameCtrl = MAC_FRAME_ACTION;

	COPY_MAC_ADDR(prTxFrame->aucDestAddr, prStaRec->aucMacAddr);
	COPY_MAC_ADDR(prTxFrame->aucSrcAddr, prBssInfo->aucOwnMacAddr);
	COPY_MAC_ADDR(prTxFrame->aucBSSID, prBssInfo->aucBSSID);

	/* Compose the frame body's frame */
	prTxFrame->ucCategory = CATEGORY_S1G_ACTION;
	prTxFrame->ucAction = ACTION_S1G_TWT_SETUP;

	prTWTBuf = &(prTxFrame->rTWT);
	twtFillTWTElement(prTWTBuf, ucTWTFlowId, prTWTParams);

	/* Update information of MSDU_INFO_T */
	TX_SET_MMPDU(prAdapter,
			prMsduInfo,
			prBssInfo->ucBssIndex,
			prStaRec->ucIndex,
			WLAN_MAC_MGMT_HEADER_LEN,
			sizeof(struct _ACTION_TWT_SETUP_FRAME),
			pfTxDoneHandler, MSDU_RATE_MODE_AUTO);

	/* Enqueue the frame to send this action frame */
	nicTxEnqueueMsdu(prAdapter, prMsduInfo);

	return WLAN_STATUS_SUCCESS;
}

uint32_t twtSendTeardownFrame(
	struct ADAPTER *prAdapter,
	struct STA_RECORD *prStaRec,
	uint8_t ucTWTFlowId,
	PFN_TX_DONE_HANDLER pfTxDoneHandler)
{
	struct MSDU_INFO *prMsduInfo;
	struct _ACTION_TWT_TEARDOWN_FRAME *prTxFrame;
	struct BSS_INFO *prBssInfo;
	uint16_t u2EstimatedFrameLen;

	ASSERT(prAdapter);
	ASSERT(prStaRec);

	prBssInfo = GET_BSS_INFO_BY_INDEX(prAdapter, prStaRec->ucBssIndex);

	ASSERT(prBssInfo);

	/* Calculate MSDU buffer length */
	u2EstimatedFrameLen = MAC_TX_RESERVED_FIELD +
		sizeof(struct _ACTION_TWT_TEARDOWN_FRAME);

	/* Alloc MSDU_INFO */
	prMsduInfo = (struct MSDU_INFO *) cnmMgtPktAlloc(
		prAdapter, u2EstimatedFrameLen);

	if (!prMsduInfo) {
		DBGLOG(TWT_REQUESTER, WARN,
			"No MSDU_INFO_T for sending TWT Teardown Frame.\n");
		return WLAN_STATUS_RESOURCES;
	}

	kalMemZero(prMsduInfo->prPacket, u2EstimatedFrameLen);

	prTxFrame = prMsduInfo->prPacket;

	/* Fill frame ctrl */
	prTxFrame->u2FrameCtrl = MAC_FRAME_ACTION;

	COPY_MAC_ADDR(prTxFrame->aucDestAddr, prStaRec->aucMacAddr);
	COPY_MAC_ADDR(prTxFrame->aucSrcAddr, prBssInfo->aucOwnMacAddr);
	COPY_MAC_ADDR(prTxFrame->aucBSSID, prBssInfo->aucBSSID);

	/* Compose the frame body's frame */
	prTxFrame->ucCategory = CATEGORY_S1G_ACTION;
	prTxFrame->ucAction = ACTION_S1G_TWT_TEARDOWN;
	prTxFrame->ucTWTFlow = ucTWTFlowId;

	/* Update information of MSDU_INFO_T */
	TX_SET_MMPDU(prAdapter,
			prMsduInfo,
			prBssInfo->ucBssIndex,
			prStaRec->ucIndex,
			WLAN_MAC_MGMT_HEADER_LEN,
			sizeof(struct _ACTION_TWT_TEARDOWN_FRAME),
			pfTxDoneHandler, MSDU_RATE_MODE_AUTO);

	/* Enqueue the frame to send this action frame */
	nicTxEnqueueMsdu(prAdapter, prMsduInfo);

	return WLAN_STATUS_SUCCESS;
}

uint32_t twtSendInfoFrame(
	struct ADAPTER *prAdapter,
	struct STA_RECORD *prStaRec,
	uint8_t ucTWTFlowId,
	struct _NEXT_TWT_INFO_T *prNextTWTInfo,
	PFN_TX_DONE_HANDLER pfTxDoneHandler)
{
	struct MSDU_INFO *prMsduInfo;
	struct _ACTION_TWT_INFO_FRAME *prTxFrame;
	uint32_t u4Pos =
		OFFSET_OF(struct _ACTION_TWT_INFO_FRAME, aucNextTWT[0]);
	struct BSS_INFO *prBssInfo;
	uint16_t u2EstimatedFrameLen;

	ASSERT(prAdapter);
	ASSERT(prStaRec);

	prBssInfo = GET_BSS_INFO_BY_INDEX(prAdapter, prStaRec->ucBssIndex);

	ASSERT(prBssInfo);

	/* Calculate MSDU buffer length */
	u2EstimatedFrameLen = MAC_TX_RESERVED_FIELD +
		sizeof(struct _ACTION_TWT_INFO_FRAME) +
		twtGetNextTWTByteCnt(prNextTWTInfo->ucNextTWTSize);

	/* Alloc MSDU_INFO */
	prMsduInfo = (struct MSDU_INFO *) cnmMgtPktAlloc(
		prAdapter, u2EstimatedFrameLen);

	if (!prMsduInfo) {
		DBGLOG(TWT_REQUESTER, WARN,
			"No MSDU_INFO_T for sending TWT Info Frame.\n");
		return WLAN_STATUS_RESOURCES;
	}

	kalMemZero(prMsduInfo->prPacket, u2EstimatedFrameLen);

	prTxFrame = prMsduInfo->prPacket;

	/* Fill frame ctrl */
	prTxFrame->u2FrameCtrl = MAC_FRAME_ACTION;

	COPY_MAC_ADDR(prTxFrame->aucDestAddr, prStaRec->aucMacAddr);
	COPY_MAC_ADDR(prTxFrame->aucSrcAddr, prBssInfo->aucOwnMacAddr);
	COPY_MAC_ADDR(prTxFrame->aucBSSID, prBssInfo->aucBSSID);

	/* Compose the frame body's frame */
	prTxFrame->ucCategory = CATEGORY_S1G_ACTION;
	prTxFrame->ucAction = ACTION_S1G_TWT_INFORMATION;
	prTxFrame->ucNextTWTCtrl = (ucTWTFlowId & TWT_INFO_FLOW_ID) |
		((prNextTWTInfo->ucNextTWTSize & TWT_INFO_NEXT_TWT_SIZE) <<
		TWT_INFO_NEXT_TWT_SIZE_OFFSET);

	switch (prNextTWTInfo->ucNextTWTSize) {
	case NEXT_TWT_SUBFIELD_64_BITS:
	{
		uint64_t *pu8NextTWT =
			(uint64_t *)(((uint8_t *)prTxFrame) + u4Pos);
		*pu8NextTWT = CPU_TO_LE64(prNextTWTInfo->u8NextTWT);
		break;
	}

	case NEXT_TWT_SUBFIELD_32_BITS:
	{
		uint32_t *pu4NextTWT =
			(uint32_t *)(((uint8_t *)prTxFrame) + u4Pos);
		*pu4NextTWT = CPU_TO_LE32(
			(uint32_t)(prNextTWTInfo->u8NextTWT & 0xFFFFFFFF));
		break;
	}

	case NEXT_TWT_SUBFIELD_48_BITS:
	{
		uint8_t *pucMem = ((uint8_t *)prTxFrame) + u4Pos;
		/* little endian placement */
		*pucMem = prNextTWTInfo->u8NextTWT & 0xFF;
		*(pucMem + 1) = (prNextTWTInfo->u8NextTWT >> 8) & 0xFF;
		*(pucMem + 2) = (prNextTWTInfo->u8NextTWT >> 16) & 0xFF;
		*(pucMem + 3) = (prNextTWTInfo->u8NextTWT >> 24) & 0xFF;
		*(pucMem + 4) = (prNextTWTInfo->u8NextTWT >> 32) & 0xFF;
		*(pucMem + 5) = (prNextTWTInfo->u8NextTWT >> 40) & 0xFF;
		break;
	}

	default:
		break;
	}

	/* Update information of MSDU_INFO_T */
	TX_SET_MMPDU(prAdapter,
			prMsduInfo,
			prBssInfo->ucBssIndex,
			prStaRec->ucIndex,
			WLAN_MAC_MGMT_HEADER_LEN,
			(sizeof(struct _ACTION_TWT_INFO_FRAME) +
				prNextTWTInfo->ucNextTWTSize),
			pfTxDoneHandler, MSDU_RATE_MODE_AUTO);

	/* Enqueue the frame to send this action frame */
	nicTxEnqueueMsdu(prAdapter, prMsduInfo);

	return WLAN_STATUS_SUCCESS;
}

static void twtFillTWTElement(
	struct _IE_TWT_T *prTWTBuf,
	uint8_t ucTWTFlowId,
	struct _TWT_PARAMS_T *prTWTParams)
{
	ASSERT(prTWTBuf);
	ASSERT(prTWTParams);

	/* Add TWT element */
	prTWTBuf->ucId = ELEM_ID_TWT;
	prTWTBuf->ucLength = sizeof(struct _IE_TWT_T) - ELEM_HDR_LEN;

	/* Request Type */
	prTWTBuf->u2ReqType |= SET_TWT_RT_REQUEST(prTWTParams->fgReq) |
		SET_TWT_RT_SETUP_CMD(prTWTParams->ucSetupCmd) |
		SET_TWT_RT_TRIGGER(prTWTParams->fgTrigger) |
		TWT_REQ_TYPE_IMPLICIT_LAST_BCAST_PARAM |
		SET_TWT_RT_FLOW_TYPE(prTWTParams->fgUnannounced) |
		SET_TWT_RT_FLOW_ID(ucTWTFlowId) |
		SET_TWT_RT_WAKE_INTVAL_EXP(prTWTParams->ucWakeIntvalExponent) |
		SET_TWT_RT_PROTECTION(prTWTParams->fgProtect);

	prTWTBuf->u8TWT = CPU_TO_LE64(prTWTParams->u8TWT);
	prTWTBuf->ucMinWakeDur = prTWTParams->ucMinWakeDur;
	prTWTBuf->u2WakeIntvalMantiss =
		CPU_TO_LE16(prTWTParams->u2WakeIntvalMantiss);
}

static void twtParseTWTElement(
	struct _IE_TWT_T *prTWTIE,
	struct _TWT_PARAMS_T *prTWTParams)
{
	uint16_t u2ReqType;

	u2ReqType = LE16_TO_CPU(prTWTIE->u2ReqType);

	prTWTParams->fgReq = GET_TWT_RT_REQUEST(u2ReqType);
	prTWTParams->ucSetupCmd = GET_TWT_RT_SETUP_CMD(u2ReqType);
	prTWTParams->fgTrigger = GET_TWT_RT_TRIGGER(u2ReqType);
	prTWTParams->fgUnannounced = GET_TWT_RT_FLOW_TYPE(u2ReqType);

	prTWTParams->ucWakeIntvalExponent =
		GET_TWT_RT_WAKE_INTVAL_EXP(u2ReqType);

	prTWTParams->fgProtect = GET_TWT_RT_PROTECTION(u2ReqType);
	prTWTParams->u8TWT = LE64_TO_CPU(prTWTIE->u8TWT);
	prTWTParams->ucMinWakeDur = prTWTIE->ucMinWakeDur;

	prTWTParams->u2WakeIntvalMantiss =
		LE16_TO_CPU(prTWTIE->u2WakeIntvalMantiss);
}

uint8_t twtGetTxSetupFlowId(
	struct MSDU_INFO *prMsduInfo)
{
	uint8_t ucFlowId;
	struct _ACTION_TWT_SETUP_FRAME *prTxFrame;

	ASSERT(prMsduInfo);

	prTxFrame = (struct _ACTION_TWT_SETUP_FRAME *)(prMsduInfo->prPacket);
	ucFlowId = GET_TWT_RT_FLOW_ID(prTxFrame->rTWT.u2ReqType);

	return ucFlowId;
}

uint8_t twtGetTxTeardownFlowId(
	struct MSDU_INFO *prMsduInfo)
{
	uint8_t ucFlowId;
	struct _ACTION_TWT_TEARDOWN_FRAME *prTxFrame;

	ASSERT(prMsduInfo);

	prTxFrame = (struct _ACTION_TWT_TEARDOWN_FRAME *)(prMsduInfo->prPacket);
	ucFlowId = (prTxFrame->ucTWTFlow & TWT_TEARDOWN_FLOW_ID);

	return ucFlowId;
}

uint8_t twtGetTxInfoFlowId(
	struct MSDU_INFO *prMsduInfo)
{
	uint8_t ucFlowId;
	struct _ACTION_TWT_INFO_FRAME *prTxFrame;

	ASSERT(prMsduInfo);

	prTxFrame = (struct _ACTION_TWT_INFO_FRAME *)(prMsduInfo->prPacket);
	ucFlowId = GET_TWT_INFO_FLOW_ID(prTxFrame->ucNextTWTCtrl);

	return ucFlowId;
}

uint8_t twtGetRxSetupFlowId(
	struct _IE_TWT_T *prTWTIE)
{
	uint16_t u2ReqType;

	ASSERT(prTWTIE);

	u2ReqType = LE16_TO_CPU(prTWTIE->u2ReqType);

	return GET_TWT_RT_FLOW_ID(u2ReqType);
}

void twtProcessS1GAction(
	struct ADAPTER *prAdapter,
	struct SW_RFB *prSwRfb)
{
	struct WLAN_ACTION_FRAME *prRxFrame;
	struct _ACTION_TWT_SETUP_FRAME *prRxSetupFrame = NULL;
	struct _ACTION_TWT_TEARDOWN_FRAME *prRxTeardownFrame = NULL;
	struct _ACTION_TWT_INFO_FRAME *prRxInfoFrame = NULL;
	struct STA_RECORD *prStaRec;
	struct RX_DESC_OPS_T *prRxDescOps;
#if (CFG_SUPPORT_TWT_HOTSPOT == 1)
	uint8_t ucTWTHotspotRejectSetupReq = 0;
	struct _TWT_HOTSPOT_STA_NODE *prTWTHotspotStaNode = NULL;
	struct _MSG_TWT_HOTSPOT_PARAMS_SET_T *prTWTHotspotParamSetMsg = NULL;
	struct _TWT_HOTSPOT_CTRL_T *prTWTHotspotCtrl = NULL;
	struct _TWT_PARAMS_T *prTWTHotspotParam = NULL;
#endif

	uint8_t ucTWTFlowId = 0;
	uint32_t u4Offset;
	uint16_t u2ReqType;

	ASSERT(prAdapter);
	ASSERT(prSwRfb);

	prRxDescOps = prAdapter->chip_info->prRxDescOps;
	ASSERT(prRxDescOps->nic_rxd_get_rx_byte_count);
	ASSERT(prRxDescOps->nic_rxd_get_pkt_type);
	ASSERT(prRxDescOps->nic_rxd_get_wlan_idx);

	prRxFrame = (struct WLAN_ACTION_FRAME *) prSwRfb->pvHeader;
	prStaRec = cnmGetStaRecByIndex(prAdapter, prSwRfb->ucStaRecIdx);
	if (!prStaRec) {
		DBGLOG(TWT_REQUESTER, WARN,
		"Received an S1G Action: wlanIdx[%d] w/o corresponding staRec\n"
		, prRxDescOps->nic_rxd_get_wlan_idx(prSwRfb->prRxStatus));
		return;
	}

	switch (prRxFrame->ucAction) {
	case ACTION_S1G_TWT_SETUP:
		prRxSetupFrame =
			(struct _ACTION_TWT_SETUP_FRAME *) prSwRfb->pvHeader;
		if (prStaRec->ucStaState != STA_STATE_3 ||
			prSwRfb->u2PacketLen <
				sizeof(struct _ACTION_TWT_SETUP_FRAME)) {
			DBGLOG(TWT_REQUESTER, WARN,
				"Received improper TWT Setup frame\n");
			return;
		}

#if (CFG_SUPPORT_TWT_HOTSPOT == 1)
		u2ReqType = prRxSetupFrame->rTWT.u2ReqType;

		if ((u2ReqType & TWT_REQ_TYPE_TWT_REQUEST)) {
			/* TWT hotspot receives STA's TWT setup request */
			/* Check if TWT hotspot is enabled */
			if (IS_FEATURE_ENABLED(
				prAdapter->rWifiVar.ucTWTHotSpotSupport)) {

				if ((u2ReqType & TWT_REQ_TYPE_TRIGGER) ||
				(!(u2ReqType & TWT_REQ_TYPE_FLOWTYPE))) {
				/* Accepts only non-trigger & unannounce */
				DBGLOG(TWT_REQUESTER, ERROR,
					"Reject TWT Setup params tr[%lu] ua[%lu]\n",
					(u2ReqType & TWT_REQ_TYPE_TRIGGER),
					(u2ReqType & TWT_REQ_TYPE_FLOWTYPE));

					ucTWTHotspotRejectSetupReq = TRUE;
				}

				if (ucTWTHotspotRejectSetupReq == FALSE) {
					/* Get free flow id */
					twtHotspotGetFreeFlowId(
						prAdapter,
						prStaRec,
						&ucTWTFlowId);

					if (ucTWTFlowId !=
						TWT_HOTSPOT_NO_MORE_FLOW_ID) {
					/* Accept, next to
					* send TWT setup resp frame
					*/
						ucTWTHotspotRejectSetupReq
							= FALSE;

						prStaRec->ucTWTFlowId
							= ucTWTFlowId;
					} else {
						/* No more flow ID avail,
						*  just reject
						*/
						ucTWTHotspotRejectSetupReq
							= TRUE;
					}
				}
			} else {
				/* TWT hotspot not enabled, just reject */
				ucTWTHotspotRejectSetupReq = TRUE;
			}

			if (ucTWTHotspotRejectSetupReq == FALSE) {
				twtHotspotGetFreeStaNode(
					prAdapter,
					prStaRec,
					&prTWTHotspotStaNode);

				if (prTWTHotspotStaNode == NULL) {
					/* No sta node avail,
					* just reject
					*/
					ucTWTHotspotRejectSetupReq
						= TRUE;
				} else {
					prTWTHotspotStaNode->flow_id
						= ucTWTFlowId;
					prTWTHotspotStaNode->prStaRec
						= prStaRec;
				}

				DBGLOG(TWT_RESPONDER, ERROR,
					"[TWT_RESP]IsReject=%d\n",
					ucTWTHotspotRejectSetupReq);
			}

			/* If this is a reject, prTWTHotspotStaNode = NULL */
			prStaRec->prTWTHotspotStaNode
				= prTWTHotspotStaNode;

			prTWTHotspotParamSetMsg = cnmMemAlloc(
				prAdapter, RAM_TYPE_MSG,
				sizeof(struct _MSG_TWT_HOTSPOT_PARAMS_SET_T));

			if (prTWTHotspotParamSetMsg) {
				prTWTHotspotCtrl =
					&prTWTHotspotParamSetMsg->rTWTCtrl;
				prTWTHotspotCtrl->ucBssIdx
					= prStaRec->ucBssIndex;
				prTWTHotspotCtrl->ucCtrlAction
					= TWT_PARAM_ACTION_ADD;
				prTWTHotspotCtrl->ucTWTFlowId = ucTWTFlowId;
				prTWTHotspotCtrl->ucIsReject
					= ucTWTHotspotRejectSetupReq;
				prTWTHotspotCtrl->ucDialogToken =
					prRxSetupFrame->ucDialogToken;
				prTWTHotspotCtrl->prStaRec = prStaRec;

				DBGLOG(TWT_RESPONDER, ERROR,
					"[TWT_RESP]B=%d lA=%d F=%d RJ=%d Tkn=%d\n",
					prTWTHotspotCtrl->ucBssIdx,
					prTWTHotspotCtrl->ucCtrlAction,
					prTWTHotspotCtrl->ucTWTFlowId,
					prTWTHotspotCtrl->ucIsReject,
					prTWTHotspotCtrl->ucDialogToken);

				prTWTHotspotParam =
					&prTWTHotspotCtrl->rTWTParams;
				prTWTHotspotParam->fgReq = FALSE;
				prTWTHotspotParam->fgTrigger =
					(u2ReqType & TWT_REQ_TYPE_TRIGGER) ?
						TRUE : FALSE;
				prTWTHotspotParam->fgProtect =
					(u2ReqType &
					TWT_REQ_TYPE_TWT_PROTECTION) ?
						TRUE : FALSE;
				prTWTHotspotParam->fgUnannounced =
					(u2ReqType & TWT_REQ_TYPE_FLOWTYPE) ?
						TRUE : FALSE;

				/* 1st stage Accept(Reject)
				* is determined over here
				*/
				prTWTHotspotParam->ucSetupCmd =
					(ucTWTHotspotRejectSetupReq) ?
						TWT_SETUP_CMD_REJECT :
						TWT_SETUP_CMD_ACCEPT;

				prTWTHotspotParam->ucMinWakeDur =
					prRxSetupFrame->rTWT.ucMinWakeDur;
				prTWTHotspotParam->ucWakeIntvalExponent =
					(u2ReqType &
					TWT_REQ_TYPE_TWT_WAKE_INTVAL_EXP) >>
					TWT_REQ_TYPE_TWT_WAKE_INTVAL_EXP_OFFSET;
				prTWTHotspotParam->u2WakeIntvalMantiss =
				prRxSetupFrame->rTWT.u2WakeIntvalMantiss;
				prTWTHotspotParam->u8TWT =
					prRxSetupFrame->rTWT.u8TWT;

				kalMemCopy(
					&prStaRec->TWTHotspotCtrl,
					prTWTHotspotCtrl,
					sizeof(struct _TWT_HOTSPOT_CTRL_T));

				if (ucTWTHotspotRejectSetupReq == FALSE) {
					/* Prepare to get
					*  the nearest target tsf
					*/
					prTWTHotspotStaNode->agrt_sp_duration =
						prTWTHotspotParam->
							ucMinWakeDur;
					prTWTHotspotStaNode->
						agrt_sp_wake_intvl_exponent =
						prTWTHotspotParam->
							ucWakeIntvalExponent;
					prTWTHotspotStaNode->
						agrt_sp_wake_intvl_mantissa =
						prRxSetupFrame->
						rTWT.u2WakeIntvalMantiss;
					prTWTHotspotStaNode->
						agrt_sp_start_tsf =
							prTWTHotspotParam->
								u8TWT;
					prTWTHotspotStaNode->
						schedule_sp_start_tsf = 0;
				}

				prTWTHotspotParamSetMsg->rMsgHdr.eMsgId =
					MID_TWT_RESP_PARAMS_SET;

				mboxSendMsg(prAdapter, MBOX_ID_0,
				(struct MSG_HDR *) prTWTHotspotParamSetMsg,
				MSG_SEND_METHOD_BUF);
			}
		} else {
#endif

		/* Parse TWT element */
		ucTWTFlowId = twtGetRxSetupFlowId(&(prRxSetupFrame->rTWT));
		twtParseTWTElement(&(prRxSetupFrame->rTWT),
			&(prStaRec->arTWTFlow[ucTWTFlowId].rTWTPeerParams));

		/* Notify TWT Requester FSM upon reception of a TWT response */
		u2ReqType = prRxSetupFrame->rTWT.u2ReqType;
		if (!(u2ReqType & TWT_REQ_TYPE_TWT_REQUEST)) {
			twtReqFsmRunEventRxSetup(prAdapter,
				prSwRfb, prStaRec, ucTWTFlowId);
		}

#if (CFG_SUPPORT_TWT_HOTSPOT == 1)
		}
#endif

		break;

	case ACTION_S1G_TWT_TEARDOWN:
		prRxTeardownFrame = (struct _ACTION_TWT_TEARDOWN_FRAME *)
			prSwRfb->pvHeader;
		if (prStaRec->ucStaState != STA_STATE_3 ||
			prSwRfb->u2PacketLen <
				sizeof(struct _ACTION_TWT_TEARDOWN_FRAME)) {
			DBGLOG(TWT_REQUESTER, WARN,
				"Received improper TWT Teardown frame\n");
			return;
		}

		ucTWTFlowId = prRxTeardownFrame->ucTWTFlow;

#if (CFG_SUPPORT_TWT_HOTSPOT == 1)
		prTWTHotspotStaNode = prStaRec->prTWTHotspotStaNode;

		if (prTWTHotspotStaNode != NULL) {
			/* if we're over here, this is Hotspot's
			* STA's teardown request
			*/
			if (prTWTHotspotStaNode->flow_id == ucTWTFlowId) {
				/* Proceed the teardown operation for
				* this Hotsopt's STA:
				* 1. send teardown command to F/W
				* 2. reset  this Hotsopt's STA station record
				*/
				twtHotspotRespFsmSteps(
					prAdapter,
					prStaRec,
					TWT_HOTSPOT_RESP_STATE_RECEIVE_TEARDOWN,
					ucTWTFlowId,
					NULL);
			} else {
				/* incorrect flow id from sta */
				DBGLOG(TWT_RESPONDER, ERROR,
					"[TWT_RESP]Received TWT Teardown flow id = %d\n",
					ucTWTFlowId);

				return;
			}
		}		else		{
			/* We are TWT STA */
			/* Notify TWT Requester FSM */
			twtReqFsmRunEventRxTeardown(
				prAdapter, prSwRfb, prStaRec,
				ucTWTFlowId);
		}
#else
		/* Notify TWT Requester FSM */
		twtReqFsmRunEventRxTeardown(
			prAdapter, prSwRfb, prStaRec, ucTWTFlowId);
#endif

		break;

	case ACTION_S1G_TWT_INFORMATION:
	{
		uint8_t ucNextTWTSize = 0;
		uint8_t *pucMem;
		struct _NEXT_TWT_INFO_T rNextTWTInfo;

		prRxInfoFrame = (struct _ACTION_TWT_INFO_FRAME *)
			prSwRfb->pvHeader;
		if (prStaRec->ucStaState != STA_STATE_3 ||
			prSwRfb->u2PacketLen <
				sizeof(struct _ACTION_TWT_INFO_FRAME)) {
			DBGLOG(TWT_REQUESTER, WARN,
				"Received improper TWT Info frame\n");
			return;
		}

		ucTWTFlowId = GET_TWT_INFO_FLOW_ID(
			prRxInfoFrame->ucNextTWTCtrl);
		ucNextTWTSize = GET_TWT_INFO_NEXT_TWT_SIZE(
			prRxInfoFrame->ucNextTWTCtrl);

		u4Offset = OFFSET_OF(struct _ACTION_TWT_INFO_FRAME,
			aucNextTWT[0]);
		pucMem = ((uint8_t *)prRxInfoFrame) + u4Offset;

		if (ucNextTWTSize == NEXT_TWT_SUBFIELD_64_BITS &&
			prSwRfb->u2PacketLen >=
			(sizeof(struct _ACTION_TWT_INFO_FRAME) + 8)) {
			rNextTWTInfo.u8NextTWT =
				LE64_TO_CPU(*((uint64_t *)pucMem));
		} else if (ucNextTWTSize == NEXT_TWT_SUBFIELD_32_BITS &&
			prSwRfb->u2PacketLen >=
			(sizeof(struct _ACTION_TWT_INFO_FRAME) + 4)) {
			rNextTWTInfo.u8NextTWT =
				LE32_TO_CPU(*((uint32_t *)pucMem));
		} else if (ucNextTWTSize == NEXT_TWT_SUBFIELD_48_BITS &&
			prSwRfb->u2PacketLen >=
			(sizeof(struct _ACTION_TWT_INFO_FRAME) + 6)) {
			rNextTWTInfo.u8NextTWT =
				GET_48_BITS_NEXT_TWT_FROM_PKT(pucMem);
		} else {
			DBGLOG(TWT_REQUESTER, WARN,
				"TWT Info frame with imcorrect size\n");
			return;
		}

		rNextTWTInfo.ucNextTWTSize = ucNextTWTSize;

		/* Notify TWT Requester FSM */
		twtReqFsmRunEventRxInfoFrm(
			prAdapter, prSwRfb, prStaRec, ucTWTFlowId,
			&rNextTWTInfo);

		break;
	}
	default:
		break;
	}
}

#if (CFG_SUPPORT_TWT_HOTSPOT == 1)
u_int8_t
twtHotspotIsDuplicatedFlowId(
	struct ADAPTER *prAdapter,
	struct STA_RECORD *prStaRec,
	u_int8_t ucTWTFlowId)
{
	return 0;
}

void
twtHotspotGetFreeFlowId(
	struct ADAPTER *prAdapter,
	struct STA_RECORD *prStaRec,
	u_int8_t *p_ucTWTFlowId)
{
	struct BSS_INFO *prBssInfo;
	u_int8_t i = 0;

	ASSERT(prAdapter);
	ASSERT(prStaRec);

	prBssInfo = GET_BSS_INFO_BY_INDEX(prAdapter, prStaRec->ucBssIndex);

	ASSERT(prBssInfo);

	if (p2pFuncIsAPMode(prAdapter->rWifiVar
		.prP2PConnSettings[prBssInfo->u4PrivateData])) {
		for (i = 0; i < TWT_MAX_FLOW_NUM; i++) {
			if ((prBssInfo->twt_flow_id_bitmap & (1 << i)) == 0) {
				*p_ucTWTFlowId = i;

				prBssInfo->twt_flow_id_bitmap |= (1 << i);

				DBGLOG(TWT_RESPONDER, ERROR,
					"[TWT_RESP]BSS_INFO[%d] hotspot TWT flow id bitmap[%x]\n",
					prBssInfo->ucBssIndex,
					prBssInfo->twt_flow_id_bitmap);

				return;
			}
		}

		*p_ucTWTFlowId = TWT_HOTSPOT_NO_MORE_FLOW_ID;

		DBGLOG(TWT_RESPONDER, ERROR,
				"[TWT_RESP]Hotspot no more TWT connectivity\n");
	}
}

void
twtHotspotReturnFlowId(
	struct ADAPTER *prAdapter,
	struct STA_RECORD *prStaRec,
	u_int8_t ucTWTFlowId)
{
	struct BSS_INFO *prBssInfo;
	u_int8_t i = 0;

	ASSERT(prAdapter);
	ASSERT(prStaRec);

	prBssInfo = GET_BSS_INFO_BY_INDEX(prAdapter, prStaRec->ucBssIndex);

	ASSERT(prBssInfo);

	if (p2pFuncIsAPMode(prAdapter->rWifiVar
		.prP2PConnSettings[prBssInfo->u4PrivateData])) {
		i = prBssInfo->twt_flow_id_bitmap;
		prBssInfo->twt_flow_id_bitmap &= (~(1 << ucTWTFlowId));

		DBGLOG(TWT_RESPONDER, ERROR,
				"[TWT_RESP]BSS_INFO[%d] TWT flow id bitmap [%x]->[%x]\n",
				prBssInfo->ucBssIndex,
				i,
				prBssInfo->twt_flow_id_bitmap);
	}
}

void
twtHotspotGetStaRecIndexByFlowId(
	struct ADAPTER *prAdapter,
	u_int8_t ucBssIdx,
	u_int8_t ucTWTFlowId,
	u_int8_t *p_ucIndex)
{
	struct BSS_INFO *prBssInfo;
	u_int8_t i = 0;

	ASSERT(prAdapter);

	prBssInfo = GET_BSS_INFO_BY_INDEX(prAdapter, ucBssIdx);

	ASSERT(prBssInfo);

	if (p2pFuncIsAPMode(prAdapter->rWifiVar
		.prP2PConnSettings[prBssInfo->u4PrivateData])) {
		for (i = 0; i < TWT_MAX_FLOW_NUM; i++) {
			if ((prBssInfo->arTWTSta[i].used == 1) &&
				(prBssInfo->arTWTSta[i].flow_id ==
					ucTWTFlowId)) {
				*p_ucIndex = i;

				return;
			}
		}

		*p_ucIndex = 0xFF;
	}
}

void
twtHotspotGetStaRecByFlowId(
	struct ADAPTER *prAdapter,
	u_int8_t ucBssIdx,
	u_int8_t ucTWTFlowId,
	struct STA_RECORD **pprStaRec
)
{
	struct BSS_INFO *prBssInfo;
	u_int8_t i = 0;

	ASSERT(prAdapter);

	prBssInfo = GET_BSS_INFO_BY_INDEX(prAdapter, ucBssIdx);

	ASSERT(prBssInfo);

	if (p2pFuncIsAPMode(prAdapter->rWifiVar
		.prP2PConnSettings[prBssInfo->u4PrivateData])) {
		for (i = 0; i < TWT_MAX_FLOW_NUM; i++) {
			if ((prBssInfo->arTWTSta[i].used == 1) &&
				(prBssInfo->arTWTSta[i].flow_id ==
					ucTWTFlowId)) {
				*pprStaRec = prBssInfo->arTWTSta[i].prStaRec;

				return;
			}
		}

		*pprStaRec = NULL;
	}
}

void
twtHotspotGetFreeStaNodeIndex(
	struct ADAPTER *prAdapter,
	struct STA_RECORD *prStaRec,
	u_int8_t *p_ucIndex)
{
	struct BSS_INFO *prBssInfo;
	u_int8_t i = 0;

	ASSERT(prAdapter);
	ASSERT(prStaRec);

	prBssInfo = GET_BSS_INFO_BY_INDEX(prAdapter, prStaRec->ucBssIndex);

	ASSERT(prBssInfo);

	if (p2pFuncIsAPMode(prAdapter->rWifiVar
		.prP2PConnSettings[prBssInfo->u4PrivateData])) {

		for (i = 0; i < TWT_MAX_FLOW_NUM; i++) {
			if (prBssInfo->arTWTSta[i].used == 0) {
				prBssInfo->arTWTSta[i].used = 1;

				*p_ucIndex = i;

				DBGLOG(TWT_RESPONDER, ERROR,
					"[TWT_RESP]BSS_INFO[%d] hotspot TWT sta node[%d]\n",
					prBssInfo->ucBssIndex,
					i);

				return;
			}
		}

		*p_ucIndex = TWT_HOTSPOT_NO_MORE_FLOW_ID;

		DBGLOG(TWT_RESPONDER, ERROR,
					"[TWT_RESP]Hotspot no more TWT sta nodes\n");
	}
}

void
twtHotspotGetFreeStaNode(
	struct ADAPTER *prAdapter,
	struct STA_RECORD *prStaRec,
	struct _TWT_HOTSPOT_STA_NODE **pprTWTHotspotStaNode)
{
	struct BSS_INFO *prBssInfo;
	u_int8_t i = 0;

	ASSERT(prAdapter);
	ASSERT(prStaRec);

	prBssInfo = GET_BSS_INFO_BY_INDEX(prAdapter, prStaRec->ucBssIndex);

	ASSERT(prBssInfo);

	if (p2pFuncIsAPMode(prAdapter->rWifiVar
		.prP2PConnSettings[prBssInfo->u4PrivateData])) {

		for (i = 0; i < TWT_MAX_FLOW_NUM; i++) {
			if (prBssInfo->arTWTSta[i].used == 0) {
				prBssInfo->arTWTSta[i].used = 1;

/* Caller:
*       struct _TWT_HOTSPOT_STA_NODE *prTWTHotspotStaNode = NULL;
*   Callee:
*       pprTWTHotspotStaNode => &prTWTHotspotStaNode
*       *pprTWTHotspotStaNode => prTWTHotspotStaNode
*/

				prBssInfo->arTWTSta[i].own_mac_idx =
					prBssInfo->ucOwnMacIndex;
				prBssInfo->arTWTSta[i].peer_id_grp_id =
					CPU_TO_LE16(prStaRec->ucWlanIndex);
				prBssInfo->arTWTSta[i].bss_idx =
					prBssInfo->ucBssIndex;

				*pprTWTHotspotStaNode = &prBssInfo->arTWTSta[i];

				DBGLOG(TWT_RESPONDER, ERROR,
					"[TWT_RESP]BSS_INFO[%d] hotspot TWT sta node[%d]\n",
					prBssInfo->ucBssIndex,
					i);

				return;
			}
		}

		*pprTWTHotspotStaNode = NULL;

		DBGLOG(TWT_RESPONDER, ERROR,
					"[TWT_RESP]Hotspot no more TWT sta nodes\n");
	}
}

void
twtHotspotResetStaNode(
	struct ADAPTER *prAdapter,
	struct STA_RECORD *prStaRec)
{
	struct BSS_INFO *prBssInfo;
	struct _TWT_HOTSPOT_STA_NODE *prTWTHotspotStaNode = NULL;

	ASSERT(prAdapter);
	ASSERT(prStaRec);

	prBssInfo = GET_BSS_INFO_BY_INDEX(prAdapter, prStaRec->ucBssIndex);

	ASSERT(prBssInfo);

	prTWTHotspotStaNode = prStaRec->prTWTHotspotStaNode;

	ASSERT(prTWTHotspotStaNode);

	if (p2pFuncIsAPMode(prAdapter->rWifiVar
		.prP2PConnSettings[prBssInfo->u4PrivateData])) {
		/* Reset hotspot sta node */
		prTWTHotspotStaNode->used = 0;

		prTWTHotspotStaNode->own_mac_idx = 0;
		prTWTHotspotStaNode->flow_id = 0;
		prTWTHotspotStaNode->peer_id_grp_id = 0;
		prTWTHotspotStaNode->agrt_sp_duration = 0;
		prTWTHotspotStaNode->bss_idx = 0;

		prTWTHotspotStaNode->schedule_sp_start_tsf = 0;
		prTWTHotspotStaNode->twt_assigned_tsf = 0;
		prTWTHotspotStaNode->agrt_sp_start_tsf = 0;
		prTWTHotspotStaNode->agrt_sp_wake_intvl_mantissa = 0;
		prTWTHotspotStaNode->agrt_sp_wake_intvl_exponent = 0;
		prTWTHotspotStaNode->prStaRec = NULL;

		/* Remove this hotspot sta node from schedule list */
		DlListDel(&prTWTHotspotStaNode->list);

		/* Finally, set station record's hotspot sta node to NULL */
		prStaRec->prTWTHotspotStaNode = NULL;
	}
}

u_int32_t
twtHotspotAlignDuration(
	u_int32_t sp_duration,
	u_int32_t alignment)
{
	u_int32_t sp_duration_alignment = 0;
	u_int32_t m = sp_duration % alignment;

#if (TWT_HOTSPOT_TSF_ALIGNMENT_EN == 1)
	sp_duration_alignment = sp_duration + (alignment - m);
#else
	sp_duration_alignment = sp_duration;
#endif /* TWT_TSF_ALIGNMENT_EN */

	return sp_duration_alignment;
}

void
twtHotspotGetNearestTargetTSF(
	struct ADAPTER *prAdapter,
	struct STA_RECORD *prStaRec,
	struct _TWT_HOTSPOT_STA_NODE *prTWTHotspotStaNode,
	u_int64_t u8CurrentTsf)
{
	struct BSS_INFO *prBssInfo;
	u_int16_t sp_duration = 0;
	u_int64_t u8twt_interval = 0;
	u_int64_t u8Temp = 0;
	u_int64_t u8Mod = 0;
	u_int8_t bFound = FALSE;
	struct _TWT_HOTSPOT_STA_NODE *curr_twt_node = NULL;
	struct _TWT_HOTSPOT_STA_NODE *next_twt_node = NULL;
	struct _TWT_HOTSPOT_STA_NODE *temp_twt_node = NULL;
	struct _TWT_HOTSPOT_STA_NODE *head_twt_node = NULL;

	ASSERT(prAdapter);
	ASSERT(prStaRec);

	prBssInfo = GET_BSS_INFO_BY_INDEX(
				prAdapter,
				prStaRec->ucBssIndex);

	ASSERT(prBssInfo);

	if (prTWTHotspotStaNode == NULL) {
		DBGLOG(TWT_RESPONDER, ERROR,
					"[TWT_RESP]NULL node\n");
		return;
	}
	if (p2pFuncIsAPMode(prAdapter->rWifiVar
		.prP2PConnSettings[prBssInfo->u4PrivateData])) {

		/* Build the whole schedule from
		* BssInfo's TWT schedule link
		*/
		sp_duration = prTWTHotspotStaNode->
						agrt_sp_duration << 8;

		if (DlListLen(&prBssInfo->twt_sch_link) == 0) {
			/* insert as the 1st node */
			prTWTHotspotStaNode->schedule_sp_start_tsf = 0;

			DlListAddTail(
				&prBssInfo->twt_sch_link,
				&prTWTHotspotStaNode->list);
		} else if (DlListLen(&prBssInfo->twt_sch_link) == 1) {
			curr_twt_node = DlListFirst(
				&prBssInfo->twt_sch_link,
				struct _TWT_HOTSPOT_STA_NODE,
				list);
			if (curr_twt_node == NULL) {
				DBGLOG(TWT_RESPONDER, ERROR,
					"[TWT_RESP]can't find curr_twt_node\n");
				return;
			}
			if (curr_twt_node->schedule_sp_start_tsf
				>= sp_duration) {
				/* insert before 1st node */
				prTWTHotspotStaNode->schedule_sp_start_tsf = 0;

				DlListAddTail(
					&curr_twt_node->list,
					&prTWTHotspotStaNode->list);
			} else {
				/* insert after 1st node */
				prTWTHotspotStaNode->schedule_sp_start_tsf =
					curr_twt_node->schedule_sp_start_tsf +
					twtHotspotAlignDuration(
					(curr_twt_node->agrt_sp_duration) << 8,
					TWT_HOTSPOT_TSF_ALIGNMNET_UINT);

				DlListAdd(
					&curr_twt_node->list,
					&prTWTHotspotStaNode->list);
			}
		} else {
			/* insert at proper place */
			head_twt_node = DlListFirst(
				&prBssInfo->twt_sch_link,
				struct _TWT_HOTSPOT_STA_NODE,
				list);

			DlListForEachSafe(
				temp_twt_node,
				next_twt_node,
				&prBssInfo->twt_sch_link,
				struct _TWT_HOTSPOT_STA_NODE,
				list) {
				curr_twt_node = temp_twt_node;

				/* space check before 1st node */
				if (curr_twt_node == head_twt_node) {
					if (curr_twt_node->
						schedule_sp_start_tsf >=
							sp_duration) {
					/* insert before head */
					prTWTHotspotStaNode->
						schedule_sp_start_tsf = 0;

					DlListAddTail(
						&curr_twt_node->list,
						&prTWTHotspotStaNode->list);

					bFound = TRUE;

					break;
					}
				}

				/* space check after 1st node
				* if current node is not the last node
				*/
				if ((&curr_twt_node->list)->Next !=
					(&prBssInfo->twt_sch_link)) {
					if (next_twt_node->
						schedule_sp_start_tsf -
						(curr_twt_node->
						schedule_sp_start_tsf +
						twtHotspotAlignDuration(
						curr_twt_node->
							agrt_sp_duration << 8,
						TWT_HOTSPOT_TSF_ALIGNMNET_UINT))
							>= sp_duration) {
						prTWTHotspotStaNode->
							schedule_sp_start_tsf =
						curr_twt_node->
							schedule_sp_start_tsf +
						twtHotspotAlignDuration(
						curr_twt_node->agrt_sp_duration
							<< 8,
						TWT_HOTSPOT_TSF_ALIGNMNET_UINT);

						DlListAdd(
						&curr_twt_node->list,
						&prTWTHotspotStaNode->list);

						bFound = TRUE;

						break;
					}
				}
			}

			/* insert as the tail node */
			if (!bFound) {
				prTWTHotspotStaNode->schedule_sp_start_tsf =
					curr_twt_node->schedule_sp_start_tsf +
					twtHotspotAlignDuration(
					curr_twt_node->agrt_sp_duration << 8,
					TWT_HOTSPOT_TSF_ALIGNMNET_UINT);

				DlListAddTail(
					&prBssInfo->twt_sch_link,
					&prTWTHotspotStaNode->list);
			}
		}

		/* Determine the real timestamp for this TWT activity */
		u8twt_interval = ((u_int64_t)(prTWTHotspotStaNode->
			agrt_sp_wake_intvl_mantissa)) <<
			prTWTHotspotStaNode->agrt_sp_wake_intvl_exponent;

		u8Temp = u8CurrentTsf -
			prTWTHotspotStaNode->schedule_sp_start_tsf;

		u8Mod = kal_mod64(u8Temp, u8twt_interval);

		prTWTHotspotStaNode->twt_assigned_tsf =
						u8CurrentTsf +
						(u8twt_interval - u8Mod) +
						u8twt_interval * 10;

		DBGLOG(TWT_RESPONDER, ERROR,
					"[TWT_RESP]twt_assigned_tsf=%llx\n",
					prTWTHotspotStaNode->twt_assigned_tsf);
		DBGLOG(TWT_RESPONDER, ERROR,
					"[TWT_RESP]u8Temp=%llx\n",
					u8Temp);
		DBGLOG(TWT_RESPONDER, ERROR,
					"[TWT_RESP]u8Mod=%llx\n",
					u8Mod);
	}
}

uint32_t
twtHotspotSendSetupRespFrame(
	struct ADAPTER *prAdapter,
	struct STA_RECORD *prStaRec,
	u_int8_t ucTWTFlowId,
	u_int8_t ucDialogToken,
	struct _TWT_PARAMS_T *prTWTParams,
	PFN_TX_DONE_HANDLER pfTxDoneHandler)
{
	struct MSDU_INFO *prMsduInfo;
	struct _ACTION_TWT_SETUP_FRAME *prTxFrame;
	struct BSS_INFO *prBssInfo;
	uint16_t u2EstimatedFrameLen;
	struct _IE_TWT_T *prTWTBuf;

	ASSERT(prAdapter);
	ASSERT(prStaRec);
	ASSERT(prTWTParams);

	prBssInfo = GET_BSS_INFO_BY_INDEX(prAdapter, prStaRec->ucBssIndex);

	ASSERT(prBssInfo);

	/* Calculate MSDU buffer length */
	u2EstimatedFrameLen = MAC_TX_RESERVED_FIELD
		+ sizeof(struct _ACTION_TWT_SETUP_FRAME);

	/* Alloc MSDU_INFO */
	prMsduInfo = (struct MSDU_INFO *)
			cnmMgtPktAlloc(prAdapter, u2EstimatedFrameLen);

	if (!prMsduInfo) {
		DBGLOG(TWT_RESPONDER, ERROR,
			"[TWT_RESP]No MSDU_INFO_T for sending TWT Setup Resp Frame.\n");

		return WLAN_STATUS_RESOURCES;
	}

	kalMemZero(prMsduInfo->prPacket, u2EstimatedFrameLen);

	prTxFrame = prMsduInfo->prPacket;

	/* Fill frame ctrl */
	prTxFrame->u2FrameCtrl = MAC_FRAME_ACTION;

	COPY_MAC_ADDR(prTxFrame->aucDestAddr, prStaRec->aucMacAddr);
	COPY_MAC_ADDR(prTxFrame->aucSrcAddr, prBssInfo->aucOwnMacAddr);
	COPY_MAC_ADDR(prTxFrame->aucBSSID, prBssInfo->aucBSSID);

	/* Compose the frame body's frame */
	prTxFrame->ucCategory = CATEGORY_S1G_ACTION;
	prTxFrame->ucAction = ACTION_S1G_TWT_SETUP;

	/* Must be the same as in the STA's setup req frame*/
	prTxFrame->ucDialogToken = ucDialogToken;

	prTWTBuf = &(prTxFrame->rTWT);
	twtFillTWTElement(prTWTBuf, ucTWTFlowId, prTWTParams);

	/* Update information of MSDU_INFO_T */
	TX_SET_MMPDU(prAdapter,
			prMsduInfo,
			prBssInfo->ucBssIndex,
			prStaRec->ucIndex,
			WLAN_MAC_MGMT_HEADER_LEN,
			sizeof(struct _ACTION_TWT_SETUP_FRAME),
			pfTxDoneHandler, MSDU_RATE_MODE_AUTO);

	/* Enqueue the frame to send this action frame */
	nicTxEnqueueMsdu(prAdapter, prMsduInfo);

	return WLAN_STATUS_SUCCESS;
}
#endif
