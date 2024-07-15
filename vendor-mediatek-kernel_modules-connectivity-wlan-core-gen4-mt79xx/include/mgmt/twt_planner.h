/* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause */
/*
 * Copyright (c) 2017 MediaTek Inc.
 */
#ifndef _TWT_PLANNER_H
#define _TWT_PLANNER_H

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

struct _TWT_FLOW_T {
	struct _TWT_PARAMS_T rTWTParams;
	struct _TWT_PARAMS_T rTWTPeerParams;
	u_int64_t u8NextTWT;
};

struct _TWT_AGRT_T {
	u_int8_t fgValid;
	u_int8_t ucAgrtTblIdx;
	u_int8_t ucBssIdx;
	u_int8_t ucFlowId;
	struct _TWT_PARAMS_T rTWTAgrt;
};

struct _TWT_PLANNER_T {
	struct _TWT_AGRT_T arTWTAgrtTbl[TWT_AGRT_MAX_NUM];
};

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

#define TSF_OFFSET_FOR_EMU	   (1 * 1000 * 1000)	/* after 1 sec */
#define TSF_OFFSET_FOR_AGRT_ADD	   (5 * 1000 * 1000)	/* after 5 sec */
#define TSF_OFFSET_FOR_AGRT_RESUME (5 * 1000 * 1000)	/* after 5 sec */

/* Definitions for action control of TWT params */
enum {
	TWT_PARAM_ACTION_NONE = 0,
	TWT_PARAM_ACTION_ADD_BYPASS = 1, /* bypass nego & add an agrt */
	TWT_PARAM_ACTION_DEL_BYPASS = 2, /* bypass proto & del an agrt */
	TWT_PARAM_ACTION_MOD_BYPASS = 3, /* bypass proto & modify an agrt */
	TWT_PARAM_ACTION_ADD = 4,
	TWT_PARAM_ACTION_DEL = 5,
	TWT_PARAM_ACTION_SUSPEND = 6,
	TWT_PARAM_ACTION_RESUME = 7,
	TWT_PARAM_ACTION_MAX
};

#define IS_TWT_PARAM_ACTION_ADD_BYPASS(ucCtrlAction) \
	((ucCtrlAction) == TWT_PARAM_ACTION_ADD_BYPASS)
#define IS_TWT_PARAM_ACTION_DEL_BYPASS(ucCtrlAction) \
	((ucCtrlAction) == TWT_PARAM_ACTION_DEL_BYPASS)
#define IS_TWT_PARAM_ACTION_MOD_BYPASS(ucCtrlAction) \
	((ucCtrlAction) == TWT_PARAM_ACTION_MOD_BYPASS)
#define IS_TWT_PARAM_ACTION_ADD(ucCtrlAction) \
	((ucCtrlAction) == TWT_PARAM_ACTION_ADD)
#define IS_TWT_PARAM_ACTION_DEL(ucCtrlAction) \
	((ucCtrlAction) == TWT_PARAM_ACTION_DEL)
#define IS_TWT_PARAM_ACTION_SUSPEND(ucCtrlAction) \
	((ucCtrlAction) == TWT_PARAM_ACTION_SUSPEND)
#define IS_TWT_PARAM_ACTION_RESUME(ucCtrlAction) \
	((ucCtrlAction) == TWT_PARAM_ACTION_RESUME)

/*******************************************************************************
*                  F U N C T I O N   D E C L A R A T I O N S
********************************************************************************
*/
void twtPlannerSetParams(
	struct ADAPTER *prAdapter,
	struct MSG_HDR *prMsgHdr);

uint32_t twtPlannerReset(
	struct ADAPTER *prAdapter,
	struct BSS_INFO *prBssInfo);

void twtPlannerRxNegoResult(
	struct ADAPTER *prAdapter,
	struct MSG_HDR *prMsgHdr);

void twtPlannerSuspendDone(
	struct ADAPTER *prAdapter,
	struct MSG_HDR *prMsgHdr);

void twtPlannerTeardownDone(
	struct ADAPTER *prAdapter,
	struct MSG_HDR *prMsgHdr);

void twtPlannerResumeDone(
	struct ADAPTER *prAdapter,
	struct MSG_HDR *prMsgHdr);

void twtPlannerRxInfoFrm(
	struct ADAPTER *prAdapter,
	struct MSG_HDR *prMsgHdr);

void twtPlannerGetTsfDone(
	struct ADAPTER *prAdapter,
	struct CMD_INFO *prCmdInfo,
	uint8_t *pucEventBuf);

#if (CFG_SUPPORT_TWT_HOTSPOT == 1)
void twtHotspotPlannerSetParams(
	struct ADAPTER *prAdapter,
	struct MSG_HDR *prMsgHdr);

void twtHotspotPlannerSetupAgrtToFW(
	struct ADAPTER *prAdapter,
	struct MSG_HDR *prMsgHdr);

void twtHotspotPlannerTeardownToFW(
	struct ADAPTER *prAdapter,
	struct MSG_HDR *prMsgHdr);

uint32_t twtHotspotPlannerGetCurrentTSF(
	struct ADAPTER *prAdapter,
	struct BSS_INFO *prBssInfo,
	void *pvSetBuffer,
	uint32_t u4SetBufferLen);

void twtHotspotPlannerGetTsfDone(
	struct ADAPTER *prAdapter,
	struct CMD_INFO *prCmdInfo,
	uint8_t *pucEventBuf);

uint32_t
twtHotspotPlannerAddAgrtTbl(
	struct ADAPTER *prAdapter,
	struct STA_RECORD *prStaRec,
	uint8_t ucFlowId,
	uint8_t fgIsOid,
	PFN_CMD_DONE_HANDLER pfCmdDoneHandler,
	PFN_CMD_TIMEOUT_HANDLER pfCmdTimeoutHandler);

uint32_t
twtHotspotPlannerTeardownSta(
	struct ADAPTER *prAdapter,
	struct STA_RECORD *prStaRec,
	uint8_t ucFlowId,
	uint8_t fgIsOid,
	PFN_CMD_DONE_HANDLER pfCmdDoneHandler,
	PFN_CMD_TIMEOUT_HANDLER pfCmdTimeoutHandler);
#endif

/*******************************************************************************
*                              F U N C T I O N S
********************************************************************************
*/

#endif /* _TWT_PLANNER_H */
