/* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause */
/*
 * Copyright (c) 2016 MediaTek Inc.
 */
#ifndef _P2P_DEV_STATE_H
#define _P2P_DEV_STATE_H

u_int8_t
p2pDevStateInit_IDLE(IN struct ADAPTER *prAdapter,
		IN struct P2P_CHNL_REQ_INFO *prChnlReqInfo,
		OUT enum ENUM_P2P_DEV_STATE *peNextState);

void p2pDevStateAbort_IDLE(IN struct ADAPTER *prAdapter);

u_int8_t
p2pDevStateInit_REQING_CHANNEL(IN struct ADAPTER *prAdapter,
		IN uint8_t ucBssIdx,
		IN struct P2P_CHNL_REQ_INFO *prChnlReqInfo,
		OUT enum ENUM_P2P_DEV_STATE *peNextState);

void
p2pDevStateAbort_REQING_CHANNEL(IN struct ADAPTER *prAdapter,
		IN struct P2P_CHNL_REQ_INFO *prChnlReqInfo,
		IN enum ENUM_P2P_DEV_STATE eNextState);

void
p2pDevStateInit_CHNL_ON_HAND(IN struct ADAPTER *prAdapter,
		IN struct BSS_INFO *prP2pBssInfo,
		IN struct P2P_DEV_FSM_INFO *prP2pDevFsmInfo,
		IN struct P2P_CHNL_REQ_INFO *prChnlReqInfo);

void
p2pDevStateAbort_CHNL_ON_HAND(IN struct ADAPTER *prAdapter,
		IN struct BSS_INFO *prP2pBssInfo,
		IN struct P2P_DEV_FSM_INFO *prP2pDevFsmInfo,
		IN struct P2P_CHNL_REQ_INFO *prChnlReqInfo,
		IN enum ENUM_P2P_DEV_STATE eNextState);

void p2pDevStateInit_SCAN(IN struct ADAPTER *prAdapter,
		IN uint8_t ucBssIndex,
		IN struct P2P_SCAN_REQ_INFO *prScanReqInfo);

void p2pDevStateAbort_SCAN(IN struct ADAPTER *prAdapter,
		IN struct P2P_DEV_FSM_INFO *prP2pDevFsmInfo);

u_int8_t
p2pDevStateInit_OFF_CHNL_TX(IN struct ADAPTER *prAdapter,
		IN struct P2P_DEV_FSM_INFO *prP2pDevFsmInfo,
		IN struct P2P_CHNL_REQ_INFO *prChnlReqInfo,
		IN struct P2P_MGMT_TX_REQ_INFO *prP2pMgmtTxInfo,
		OUT enum ENUM_P2P_DEV_STATE *peNextState);

void
p2pDevStateAbort_OFF_CHNL_TX(IN struct ADAPTER *prAdapter,
		IN struct P2P_DEV_FSM_INFO *prP2pDevFsmInfo,
		IN struct P2P_MGMT_TX_REQ_INFO *prP2pMgmtTxInfo,
		IN struct P2P_CHNL_REQ_INFO *prChnlReqInfo,
		IN enum ENUM_P2P_DEV_STATE eNextState);

#endif
