/* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause */
/*
 * Copyright (c) 2016 MediaTek Inc.
 */
/*
 ** Id: //Department/DaVinci/TRUNK/WiFi_P2P_Driver/include/mgmt/p2p_fsm.h#23
 */

/*! \file   p2p_fsm.h
 *  \brief  Declaration of functions and finite state machine for P2P Module.
 *
 *  Declaration of functions and finite state machine for P2P Module.
 */


#ifndef _P2P_FSM_H
#define _P2P_FSM_H

void p2pFsmRunEventScanRequest(IN struct ADAPTER *prAdapter,
		IN struct MSG_HDR *prMsgHdr);

void p2pFsmRunEventChGrant(IN struct ADAPTER *prAdapter,
		IN struct MSG_HDR *prMsgHdr);

void p2pFsmRunEventNetDeviceRegister(IN struct ADAPTER *prAdapter,
		IN struct MSG_HDR *prMsgHdr);

void p2pFsmRunEventUpdateMgmtFrame(IN struct ADAPTER *prAdapter,
		IN struct MSG_HDR *prMsgHdr);

#if CFG_SUPPORT_WFD
void p2pFsmRunEventWfdSettingUpdate(IN struct ADAPTER *prAdapter,
		IN struct MSG_HDR *prMsgHdr);
#endif

void p2pFsmRunEventScanDone(IN struct ADAPTER *prAdapter,
		IN struct MSG_HDR *prMsgHdr);

void p2pFsmRunEventMgmtFrameTx(IN struct ADAPTER *prAdapter,
		IN struct MSG_HDR *prMsgHdr);

void p2pFsmRunEventTxCancelWait(IN struct ADAPTER *prAdapter,
		IN struct MSG_HDR *prMsgHdr);

struct BSS_DESC *p2pGetTargetBssDesc(
	IN struct ADAPTER *prAdapter,
	IN uint8_t ucBssIndex);

#endif /* _P2P_FSM_H */
