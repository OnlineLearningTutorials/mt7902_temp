/* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause */
/*
 * Copyright (c) 2016 MediaTek Inc.
 */
#ifndef _P2P_ROLE_STATE_H
#define _P2P_ROLE_STATE_H

void
p2pRoleStateInit_IDLE(IN struct ADAPTER *prAdapter,
		IN struct P2P_ROLE_FSM_INFO *prP2pRoleFsmInfo,
		IN struct BSS_INFO *prP2pBssInfo);

void
p2pRoleStateAbort_IDLE(IN struct ADAPTER *prAdapter,
		IN struct P2P_ROLE_FSM_INFO *prP2pRoleFsmInfo,
		IN struct P2P_CHNL_REQ_INFO *prP2pChnlReqInfo);

void p2pRoleStateInit_SCAN(IN struct ADAPTER *prAdapter,
		IN uint8_t ucBssIndex,
		IN struct P2P_SCAN_REQ_INFO *prScanReqInfo);

void p2pRoleStateAbort_SCAN(IN struct ADAPTER *prAdapter,
		IN struct P2P_ROLE_FSM_INFO *prP2pRoleFsmInfo);

void
p2pRoleStateInit_REQING_CHANNEL(IN struct ADAPTER *prAdapter,
		IN uint8_t ucBssIdx,
		IN struct P2P_CHNL_REQ_INFO *prChnlReqInfo);

void
p2pRoleStateAbort_REQING_CHANNEL(IN struct ADAPTER *prAdapter,
		IN struct BSS_INFO *prP2pRoleBssInfo,
		IN struct P2P_ROLE_FSM_INFO *prP2pRoleFsmInfo,
		IN enum ENUM_P2P_ROLE_STATE eNextState);

void
p2pRoleStateInit_AP_CHNL_DETECTION(IN struct ADAPTER *prAdapter,
		IN uint8_t ucBssIndex,
		IN struct P2P_SCAN_REQ_INFO *prScanReqInfo,
		IN struct P2P_CONNECTION_REQ_INFO *prConnReqInfo);

void
p2pRoleStateAbort_AP_CHNL_DETECTION(IN struct ADAPTER *prAdapter,
		IN uint8_t ucBssIndex,
		IN struct P2P_CONNECTION_REQ_INFO *prP2pConnReqInfo,
		IN struct P2P_CHNL_REQ_INFO *prChnlReqInfo,
		IN struct P2P_SCAN_REQ_INFO *prP2pScanReqInfo,
		IN enum ENUM_P2P_ROLE_STATE eNextState);

void
p2pRoleStateInit_GC_JOIN(IN struct ADAPTER *prAdapter,
		IN struct P2P_ROLE_FSM_INFO *prP2pRoleFsmInfo,
		IN struct P2P_CHNL_REQ_INFO *prChnlReqInfo);

void
p2pRoleStateAbort_GC_JOIN(IN struct ADAPTER *prAdapter,
		IN struct P2P_ROLE_FSM_INFO *prP2pRoleFsmInfo,
		IN struct P2P_JOIN_INFO *prJoinInfo,
		IN enum ENUM_P2P_ROLE_STATE eNextState);

#if (CFG_SUPPORT_DFS_MASTER == 1)
void
p2pRoleStateInit_DFS_CAC(IN struct ADAPTER *prAdapter,
		IN uint8_t ucBssIdx,
		IN struct P2P_CHNL_REQ_INFO *prChnlReqInfo);

void
p2pRoleStateAbort_DFS_CAC(IN struct ADAPTER *prAdapter,
		IN struct BSS_INFO *prP2pRoleBssInfo,
		IN struct P2P_ROLE_FSM_INFO *prP2pRoleFsmInfo,
		IN enum ENUM_P2P_ROLE_STATE eNextState);

void
p2pRoleStateInit_SWITCH_CHANNEL(IN struct ADAPTER *prAdapter,
		IN uint8_t ucBssIdx,
		IN struct P2P_CHNL_REQ_INFO *prChnlReqInfo);

void
p2pRoleStateAbort_SWITCH_CHANNEL(IN struct ADAPTER *prAdapter,
		IN struct BSS_INFO *prP2pRoleBssInfo,
		IN struct P2P_ROLE_FSM_INFO *prP2pRoleFsmInfo,
		IN enum ENUM_P2P_ROLE_STATE eNextState);

void
p2pRoleStatePrepare_To_DFS_CAC_STATE(IN struct ADAPTER *prAdapter,
		IN struct BSS_INFO *prBssInfo,
		IN enum ENUM_CHANNEL_WIDTH rChannelWidth,
		IN struct P2P_CONNECTION_REQ_INFO *prConnReqInfo,
		OUT struct P2P_CHNL_REQ_INFO *prChnlReqInfo);

#endif

void
p2pRoleStatePrepare_To_REQING_CHANNEL_STATE(IN struct ADAPTER *prAdapter,
		IN struct BSS_INFO *prBssInfo,
		IN struct P2P_CONNECTION_REQ_INFO *prConnReqInfo,
		OUT struct P2P_CHNL_REQ_INFO *prChnlReqInfo);

u_int8_t
p2pRoleStateInit_OFF_CHNL_TX(IN struct ADAPTER *prAdapter,
		IN struct P2P_ROLE_FSM_INFO *prP2pRoleFsmInfo,
		IN struct P2P_CHNL_REQ_INFO *prChnlReqInfo,
		IN struct P2P_MGMT_TX_REQ_INFO *prP2pMgmtTxInfo,
		OUT enum ENUM_P2P_ROLE_STATE *peNextState);

void
p2pRoleStateAbort_OFF_CHNL_TX(IN struct ADAPTER *prAdapter,
		IN struct P2P_ROLE_FSM_INFO *prP2pRoleFsmInfo,
		IN struct P2P_MGMT_TX_REQ_INFO *prP2pMgmtTxInfo,
		IN struct P2P_CHNL_REQ_INFO *prChnlReqInfo,
		IN enum ENUM_P2P_ROLE_STATE eNextState);

#endif
