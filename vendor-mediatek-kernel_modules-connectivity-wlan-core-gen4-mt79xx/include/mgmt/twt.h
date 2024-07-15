/* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause */
/*
 * Copyright (c) 2017 MediaTek Inc.
 */
#ifndef _TWT_H
#define _TWT_H

/*******************************************************************************
*                         C O M P I L E R   F L A G S
********************************************************************************
*/

/*******************************************************************************
*                    E X T E R N A L   R E F E R E N C E S
********************************************************************************
*/

/*******************************************************************************
*                              C O N S T A N T S
********************************************************************************
*/
#define TWT_MAX_FLOW_NUM        8
#define TWT_MAX_WAKE_INTVAL_EXP (TWT_REQ_TYPE_TWT_WAKE_INTVAL_EXP >> \
	TWT_REQ_TYPE_TWT_WAKE_INTVAL_EXP_OFFSET)

#define TWT_ROLE_STA		0
#define TWT_ROLE_AP			1
#define TWT_ROLE_APCLI		2
#define TWT_ROLE_HOTSPOT	3

#if (CFG_SUPPORT_TWT_HOTSPOT == 1)
#define TWT_HOTSPOT_TSF_ALIGNMENT_EN 1

/* 16TU = 16*1024usec*/
#define TWT_HOTSPOT_TSF_ALIGNMNET_UINT		(16 * 1024)
#endif

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
/* Macros for setting request type bit fields in TWT IE */
#define SET_TWT_RT_REQUEST(fgReq) \
	(((fgReq) << TWT_REQ_TYPE_TWT_REQUEST_OFFSET) & \
		TWT_REQ_TYPE_TWT_REQUEST)

#define SET_TWT_RT_SETUP_CMD(ucSetupCmd) \
	(((ucSetupCmd) << TWT_REQ_TYPE_TWT_SETUP_COMMAND_OFFSET) & \
		TWT_REQ_TYPE_TWT_SETUP_COMMAND)

#define SET_TWT_RT_TRIGGER(fgTrigger) \
	(((fgTrigger) << TWT_REQ_TYPE_TRIGGER_OFFSET) & TWT_REQ_TYPE_TRIGGER)

#define SET_TWT_RT_FLOW_TYPE(fgUnannounced) \
	(((fgUnannounced) << TWT_REQ_TYPE_FLOWTYPE_OFFSET) & \
		TWT_REQ_TYPE_FLOWTYPE)

#define SET_TWT_RT_FLOW_ID(ucTWTFlowId) \
	(((ucTWTFlowId) << TWT_REQ_TYPE_TWT_FLOW_IDENTIFIER_OFFSET) & \
		TWT_REQ_TYPE_TWT_FLOW_IDENTIFIER)

#define SET_TWT_RT_WAKE_INTVAL_EXP(ucWakeIntvlExponent) \
	(((ucWakeIntvlExponent) << TWT_REQ_TYPE_TWT_WAKE_INTVAL_EXP_OFFSET) & \
		TWT_REQ_TYPE_TWT_WAKE_INTVAL_EXP)

#define SET_TWT_RT_PROTECTION(fgProtect) \
	(((fgProtect) << TWT_REQ_TYPE_TWT_PROTECTION_OFFSET) & \
		TWT_REQ_TYPE_TWT_PROTECTION)

/* Macros for getting request type bit fields in TWT IE */
#define GET_TWT_RT_REQUEST(u2ReqType) \
	(((u2ReqType) & TWT_REQ_TYPE_TWT_REQUEST) >> \
		TWT_REQ_TYPE_TWT_REQUEST_OFFSET)

#define GET_TWT_RT_SETUP_CMD(u2ReqType) \
	(((u2ReqType) & TWT_REQ_TYPE_TWT_SETUP_COMMAND) >> \
		TWT_REQ_TYPE_TWT_SETUP_COMMAND_OFFSET)

#define GET_TWT_RT_TRIGGER(u2ReqType) \
	(((u2ReqType) & TWT_REQ_TYPE_TRIGGER) >> TWT_REQ_TYPE_TRIGGER_OFFSET)

#define GET_TWT_RT_FLOW_TYPE(u2ReqType) \
	(((u2ReqType) & TWT_REQ_TYPE_FLOWTYPE) >> TWT_REQ_TYPE_FLOWTYPE_OFFSET)

#define GET_TWT_RT_FLOW_ID(u2ReqType) \
	(((u2ReqType) & TWT_REQ_TYPE_TWT_FLOW_IDENTIFIER) >> \
		TWT_REQ_TYPE_TWT_FLOW_IDENTIFIER_OFFSET)

#define GET_TWT_RT_WAKE_INTVAL_EXP(u2ReqType) \
	(((u2ReqType) & TWT_REQ_TYPE_TWT_WAKE_INTVAL_EXP) >> \
		TWT_REQ_TYPE_TWT_WAKE_INTVAL_EXP_OFFSET)

#define GET_TWT_RT_PROTECTION(u2ReqType) \
	(((u2ReqType) & TWT_REQ_TYPE_TWT_PROTECTION) >> \
		TWT_REQ_TYPE_TWT_PROTECTION_OFFSET)

/* Macros to set TWT info field */
#define SET_TWT_INFO_FLOW_ID(ucNextTWTCtrl) \
	(((ucNextTWTCtrl) << TWT_INFO_FLOW_ID_OFFSET) & TWT_INFO_FLOW_ID)

#define SET_TWT_INFO_RESP_REQUESTED(ucNextTWTCtrl) \
	(((ucNextTWTCtrl) << TWT_INFO_RESP_REQUESTED_OFFSET) & \
	TWT_INFO_RESP_REQUESTED)

#define SET_TWT_INFO_NEXT_TWT_REQ(ucNextTWTCtrl) \
	(((ucNextTWTCtrl) << TWT_INFO_NEXT_TWT_REQ_OFFSET) & \
	TWT_INFO_NEXT_TWT_REQ)

#define SET_TWT_INFO_NEXT_TWT_SIZE(ucNextTWTCtrl) \
	(((ucNextTWTCtrl) << TWT_INFO_NEXT_TWT_SIZE_OFFSET) & \
	TWT_INFO_NEXT_TWT_SIZE)

#define SET_TWT_INFO_BCAST_RESCHED(ucNextTWTCtrl) \
	(((ucNextTWTCtrl) << TWT_INFO_BCAST_RESCHED_OFFSET) & \
	TWT_INFO_BCAST_RESCHED)

/* Macros to get TWT info field */
#define GET_TWT_INFO_FLOW_ID(ucNextTWTCtrl) \
	(((ucNextTWTCtrl) & TWT_INFO_FLOW_ID) >> TWT_INFO_FLOW_ID_OFFSET)

#define GET_TWT_INFO_RESP_REQUESTED(ucNextTWTCtrl) \
	(((ucNextTWTCtrl) & TWT_INFO_RESP_REQUESTED) >> \
	TWT_INFO_RESP_REQUESTED_OFFSET)

#define GET_TWT_INFO_NEXT_TWT_REQ(ucNextTWTCtrl) \
	(((ucNextTWTCtrl) & TWT_INFO_NEXT_TWT_REQ) >> \
	TWT_INFO_NEXT_TWT_REQ_OFFSET)

#define GET_TWT_INFO_NEXT_TWT_SIZE(ucNextTWTCtrl) \
	(((ucNextTWTCtrl) & TWT_INFO_NEXT_TWT_SIZE) >> \
	TWT_INFO_NEXT_TWT_SIZE_OFFSET)

#define GET_TWT_INFO_BCAST_RESCHED(ucNextTWTCtrl) \
	(((ucNextTWTCtrl) & TWT_INFO_BCAST_RESCHED) >> \
	TWT_INFO_BCAST_RESCHED_OFFSET)

/* Next TWT from the packet should be little endian */
#define GET_48_BITS_NEXT_TWT_FROM_PKT(pMem) \
	((u_int64_t)(*((u_int8_t *)(pMem))) | \
	((u_int64_t)(*(((u_int8_t *)(pMem)) + 1)) << 8) | \
	((u_int64_t)(*(((u_int8_t *)(pMem)) + 2)) << 16) | \
	((u_int64_t)(*(((u_int8_t *)(pMem)) + 3)) << 24) | \
	((u_int64_t)(*(((u_int8_t *)(pMem)) + 4)) << 32) | \
	((u_int64_t)(*(((u_int8_t *)(pMem)) + 5)) << 40))

/*******************************************************************************
*                  F U N C T I O N   D E C L A R A T I O N S
********************************************************************************
*/

void twtProcessS1GAction(
	struct ADAPTER *prAdapter,
	struct SW_RFB *prSwRfb);

uint32_t twtSendSetupFrame(
	struct ADAPTER *prAdapter,
	struct STA_RECORD *prStaRec,
	u_int8_t ucTWTFlowId,
	struct _TWT_PARAMS_T *prTWTParams,
	PFN_TX_DONE_HANDLER pfTxDoneHandler);

uint32_t twtSendTeardownFrame(
	struct ADAPTER *prAdapter,
	struct STA_RECORD *prStaRec,
	u_int8_t ucTWTFlowId,
	PFN_TX_DONE_HANDLER pfTxDoneHandler);

uint32_t twtSendInfoFrame(
	struct ADAPTER *prAdapter,
	struct STA_RECORD *prStaRec,
	u_int8_t ucTWTFlowId,
	struct _NEXT_TWT_INFO_T *prNextTWTInfo,
	PFN_TX_DONE_HANDLER pfTxDoneHandler);

u_int8_t twtGetTxSetupFlowId(
	struct MSDU_INFO *prMsduInfo);

u_int8_t twtGetTxTeardownFlowId(
	struct MSDU_INFO *prMsduInfo);

uint8_t twtGetTxInfoFlowId(
	struct MSDU_INFO *prMsduInfo);

static inline u_int8_t twtGetNextTWTByteCnt(u_int8_t ucNextTWTSize)
{
	return (ucNextTWTSize == NEXT_TWT_SUBFIELD_64_BITS) ? 8 :
		((ucNextTWTSize == NEXT_TWT_SUBFIELD_32_BITS) ? 4 :
		((ucNextTWTSize == NEXT_TWT_SUBFIELD_48_BITS) ? 6 : 0));
}

#if (CFG_SUPPORT_TWT_HOTSPOT == 1)
u_int8_t
twtHotspotIsDuplicatedFlowId(
	struct ADAPTER *prAdapter,
	struct STA_RECORD *prStaRec,
	u_int8_t ucTWTFlowId);

void
twtHotspotGetFreeFlowId(
	struct ADAPTER *prAdapter,
	struct STA_RECORD *prStaRec,
	u_int8_t *p_ucTWTFlowId);

void
twtHotspotReturnFlowId(
	struct ADAPTER *prAdapter,
	struct STA_RECORD *prStaRec,
	u_int8_t ucTWTFlowId);

void
twtHotspotGetStaRecIndexByFlowId(
	struct ADAPTER *prAdapter,
	u_int8_t ucBssIdx,
	u_int8_t ucTWTFlowId,
	u_int8_t *p_ucIndex);

void
twtHotspotGetStaRecByFlowId(
	struct ADAPTER *prAdapter,
	u_int8_t ucBssIdx,
	u_int8_t ucTWTFlowId,
	struct STA_RECORD **pprStaRec
);

void
twtHotspotGetFreeStaNodeIndex(
	struct ADAPTER *prAdapter,
	struct STA_RECORD *prStaRec,
	u_int8_t *p_ucIndex);

void
twtHotspotGetFreeStaNode(
	struct ADAPTER *prAdapter,
	struct STA_RECORD *prStaRec,
	struct _TWT_HOTSPOT_STA_NODE **pprTWTHotspotStaNode);

void
twtHotspotResetStaNode(
	struct ADAPTER *prAdapter,
	struct STA_RECORD *prStaRec);

u_int32_t
twtHotspotAlignDuration(
	u_int32_t sp_duration,
	u_int32_t alignment);

void
twtHotspotGetNearestTargetTSF(
	struct ADAPTER *prAdapter,
	struct STA_RECORD *prStaRec,
	struct _TWT_HOTSPOT_STA_NODE *prTWTHotspotStaNode,
	u_int64_t u8CurrentTsf);

uint32_t
twtHotspotSendSetupRespFrame(
	struct ADAPTER *prAdapter,
	struct STA_RECORD *prStaRec,
	u_int8_t ucTWTFlowId,
	u_int8_t ucDialogToken,
	struct _TWT_PARAMS_T *prTWTParams,
	PFN_TX_DONE_HANDLER pfTxDoneHandler);
#endif

/*******************************************************************************
*                              F U N C T I O N S
********************************************************************************
*/

#endif /* _TWT_H */
