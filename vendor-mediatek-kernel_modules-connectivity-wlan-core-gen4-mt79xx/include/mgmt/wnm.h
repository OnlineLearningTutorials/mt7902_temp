/* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause */
/*
 * Copyright (c) 2016 MediaTek Inc.
 */
/*
 * Id: //Department/DaVinci/TRUNK/MT6620_5931_WiFi_Driver/include/mgmt/wnm.h#1
 */

/*! \file  wnm.h
 *    \brief This file contains the IEEE 802.11 family related 802.11v
 *		network management for MediaTek 802.11 Wireless LAN Adapters.
 */


#ifndef _WNM_H
#define _WNM_H

/*******************************************************************************
 *                         C O M P I L E R   F L A G S
 *******************************************************************************
 */

/*******************************************************************************
 *                    E X T E R N A L   R E F E R E N C E S
 *******************************************************************************
 */

/*******************************************************************************
 *                              C O N S T A N T S
 *******************************************************************************
 */

/*******************************************************************************
 *                         D A T A   T Y P E S
 *******************************************************************************
 */

struct TIMINGMSMT_PARAM {
	u_int8_t fgInitiator;
	uint8_t ucTrigger;
	uint8_t ucDialogToken;	/* Dialog Token */
	uint8_t ucFollowUpDialogToken;	/* Follow Up Dialog Token */
	uint32_t u4ToD;		/* Timestamp of Departure [10ns] */
	uint32_t u4ToA;		/* Timestamp of Arrival [10ns] */
};

struct BSS_TRANSITION_MGT_PARAM_T {
	/* for Query */
	uint8_t ucDialogToken;
	uint8_t ucQueryReason;
	/* for Request */
	uint8_t ucRequestMode;
	uint16_t u2DisassocTimer;
	uint16_t u2TermDuration;
	uint8_t aucTermTsf[8];
	uint8_t ucSessionURLLen;
	uint8_t aucSessionURL[255];
	/* for Respone */
	u_int8_t fgPendingResponse:1;
	u_int8_t fgUnsolicitedReq:1;
	u_int8_t fgReserved:6;
	uint8_t ucStatusCode;
	uint8_t ucTermDelay;
	uint8_t aucTargetBssid[MAC_ADDR_LEN];
	uint8_t *pucOurNeighborBss;
	uint16_t u2OurNeighborBssLen;
};

/*******************************************************************************
 *                            P U B L I C   D A T A
 *******************************************************************************
 */

/*******************************************************************************
 *                           P R I V A T E   D A T A
 *******************************************************************************
 */

/*******************************************************************************
 *                                 M A C R O S
 *******************************************************************************
 */
#define BTM_REQ_MODE_CAND_INCLUDED_BIT                  BIT(0)
#define BTM_REQ_MODE_ABRIDGED                           BIT(1)
#define BTM_REQ_MODE_DISC_IMM                           BIT(2)
#define BTM_REQ_MODE_BSS_TERM_INCLUDE                   BIT(3)
#define BTM_REQ_MODE_ESS_DISC_IMM                       BIT(4)

#define BSS_TRANSITION_MGT_STATUS_ACCEPT                0
#define BSS_TRANSITION_MGT_STATUS_UNSPECIFIED           1
#define BSS_TRANSITION_MGT_STATUS_NEED_SCAN             2
#define BSS_TRANSITION_MGT_STATUS_CAND_NO_CAPACITY      3
#define BSS_TRANSITION_MGT_STATUS_TERM_UNDESIRED        4
#define BSS_TRANSITION_MGT_STATUS_TERM_DELAY_REQUESTED  5
#define BSS_TRANSITION_MGT_STATUS_CAND_LIST_PROVIDED    6
#define BSS_TRANSITION_MGT_STATUS_CAND_NO_CANDIDATES    7
#define BSS_TRANSITION_MGT_STATUS_LEAVING_ESS           8

/* 802.11v: define Transtion and Transition Query reasons */
#define BSS_TRANSITION_BETTER_AP_FOUND                  6
#define BSS_TRANSITION_LOW_RSSI                         16
#define BSS_TRANSITION_INCLUDE_PREFER_CAND_LIST         19
#define BSS_TRANSITION_LEAVING_ESS                      20

/*******************************************************************************
 *                  F U N C T I O N   D E C L A R A T I O N S
 *******************************************************************************
 */

void wnmWNMAction(IN struct ADAPTER *prAdapter,
		  IN struct SW_RFB *prSwRfb);

void wnmReportTimingMeas(IN struct ADAPTER *prAdapter,
			 IN uint8_t ucStaRecIndex, IN uint32_t u4ToD,
			 IN uint32_t u4ToA);

#if WNM_UNIT_TEST
void wnmTimingMeasUnitTest1(struct ADAPTER *prAdapter,
			    uint8_t ucStaRecIndex);
#endif

void wnmRecvBTMRequest(IN struct ADAPTER *prAdapter, IN struct SW_RFB *prSwRfb);

void wnmSendBTMQueryFrame(IN struct ADAPTER *prAdapter,
			 IN struct STA_RECORD *prStaRec);

void wnmSendBTMResponseFrame(IN struct ADAPTER *prAdapter,
			 IN struct STA_RECORD *prStaRec);

uint8_t wnmGetBtmToken(void);
#if CFG_AP_80211V_SUPPORT
void wnmMulAPAgentSendBTMRequestFrame(
			IN struct ADAPTER *prAdapter,
			IN struct STA_RECORD *prStaRec,
			struct PARAM_CUSTOM_BTM_REQ_STRUCT *prSetBtmReqInfo);

void wnmMulAPAgentRecvBTMResponse(IN struct ADAPTER *prAdapter,
	IN struct SW_RFB *prSwRfb);
#endif /* CFG_AP_80211V_SUPPORT */

/*******************************************************************************
 *                              F U N C T I O N S
 *******************************************************************************
 */

#endif /* _WNM_H */
