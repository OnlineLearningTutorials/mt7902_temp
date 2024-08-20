/* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause */
/*
 * Copyright (c) 2017 MediaTek Inc.
 */
#ifndef _TWT_REQ_FSM_H
#define _TWT_REQ_FSM_H

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

/*******************************************************************************
*                             D A T A   T Y P E S
********************************************************************************
*/

enum _ENUM_TWT_REQUESTER_STATE_T {
	TWT_REQ_STATE_IDLE = 0,
	TWT_REQ_STATE_REQTX,
	TWT_REQ_STATE_WAIT_RSP,
	TWT_REQ_STATE_SUSPENDING,
	TWT_REQ_STATE_SUSPENDED,
	TWT_REQ_STATE_RESUMING,
	TWT_REQ_STATE_TEARING_DOWN,
	TWT_REQ_STATE_RX_TEARDOWN,
	TWT_REQ_STATE_RX_INFOFRM,
	TWT_REQ_STATE_NUM
};

#if (CFG_SUPPORT_TWT_HOTSPOT == 1)
enum _ENUM_TWT_HOTSPOT_RESPONDER_STATE_T {
	TWT_HOTSPOT_RESP_STATE_IDLE = 0,
	TWT_HOTSPOT_RESP_STATE_RECEIVE_SETUP,
	TWT_HOTSPOT_RESP_STATE_SETUP_RESPONSE,
	TWT_HOTSPOT_RESP_STATE_RECEIVE_TEARDOWN,
	TWT_HOTSPOT_RESP_STATE_SEND_TEARDOWN_TO_STA,
	TWT_HOTSPOT_RESP_STATE_DISCONNECT,
	TWT_HOTSPOT_RESP_STATE_IDLE_BY_FORCE,
	TWT_HOTSPOT_RESP_STATE_NUM
};
#endif

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
*                  F U N C T I O N   D E C L A R A T I O N S
********************************************************************************
*/

u_int32_t
twtReqFsmRunEventTxDone(
	struct ADAPTER *prAdapter,
	struct MSDU_INFO *prMsduInfo,
	enum ENUM_TX_RESULT_CODE rTxDoneStatus);

void twtReqFsmRunEventRxSetup(
	struct ADAPTER *prAdapter,
	struct SW_RFB *prSwRfb,
	struct STA_RECORD *prStaRec,
	u_int8_t ucTWTFlowId);

void twtReqFsmRunEventRxTeardown(
	struct ADAPTER *prAdapter,
	struct SW_RFB *prSwRfb,
	struct STA_RECORD *prStaRec,
	u_int8_t ucTWTFlowId);

void twtReqFsmRunEventRxInfoFrm(
	struct ADAPTER *prAdapter,
	struct SW_RFB *prSwRfb,
	struct STA_RECORD *prStaRec,
	u_int8_t ucTWTFlowId,
	struct _NEXT_TWT_INFO_T *prNextTWTInfo);

void twtReqFsmRunEventStart(
	struct ADAPTER *prAdapter,
	struct MSG_HDR *prMsgHdr);

void twtReqFsmRunEventTeardown(
	struct ADAPTER *prAdapter,
	struct MSG_HDR *prMsgHdr);

void twtReqFsmRunEventSuspend(
	struct ADAPTER *prAdapter,
	struct MSG_HDR *prMsgHdr);

void twtReqFsmRunEventResume(
	struct ADAPTER *prAdapter,
	struct MSG_HDR *prMsgHdr);

#if (CFG_SUPPORT_TWT_HOTSPOT == 1)
uint32_t
twtHotspotRespFsmSendEvent(
	struct ADAPTER *prAdapter,
	struct STA_RECORD *prStaRec,
	uint8_t ucTWTFlowId,
	enum ENUM_MSG_ID eMsgId);

void
twtHotspotRespFsmSteps(
	struct ADAPTER *prAdapter,
	struct STA_RECORD *prStaRec,
	enum _ENUM_TWT_HOTSPOT_RESPONDER_STATE_T eNextState,
	uint8_t ucTWTFlowId,
	void *pParam);

void
twtHotspotRespFsmRunEventRxSetup(
	struct ADAPTER *prAdapter,
	void *pParam);

u_int32_t
twtHotspotRespFsmRunEventTxDone(
	struct ADAPTER *prAdapter,
	struct MSDU_INFO *prMsduInfo,
	enum ENUM_TX_RESULT_CODE rTxDoneStatus);
#endif

/*******************************************************************************
*                              F U N C T I O N S
********************************************************************************
*/

#endif /* _TWT_REQ_FSM_H */
