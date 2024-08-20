/* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause */
/*
 * Copyright (c) 2016 MediaTek Inc.
 */
/*
 * Id: //Department/DaVinci/BRANCHES/MT6620_WIFI_DRIVER_V2_3/
 *							include/mgmt/aa_fsm.h#1
 */

/*! \file   aa_fsm.h
 *    \brief  Declaration of functions and finite state machine for
 *							SAA/AAA Module.
 *
 *    Declaration of functions and finite state machine for SAA/AAA Module.
 */

#ifndef _AA_FSM_H
#define _AA_FSM_H

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
/* Retry interval for retransmiting authentication-request MMPDU. */
#define TX_AUTHENTICATION_RETRY_TIMEOUT_TU          100	/* TU. */

/* Retry interval for retransmiting association-request MMPDU. */
#define TX_ASSOCIATION_RETRY_TIMEOUT_TU             100	/* TU. */

/* Wait for a response to a transmitted authentication-request MMPDU. */
#define DOT11_AUTHENTICATION_RESPONSE_TIMEOUT_TU    512	/* TU. */

/* Wait for a response to a transmitted association-request MMPDU. */
#define DOT11_ASSOCIATION_RESPONSE_TIMEOUT_TU       512	/* TU. */

/* Wait for a response to a transmitted SAE authentication MMPDU. */
/* 2000 msec is the default value on 802.11-REVmd-D0.5 */
#define DOT11_RSNA_SAE_RETRANS_PERIOD_TU	2000

/* The maximum time to wait for JOIN process complete. */
/* Beacon Interval, 20 * 100TU = 2 sec. */
#define JOIN_FAILURE_TIMEOUT_BEACON_INTERVAL        20

/* Retry interval for next JOIN request. */
#define JOIN_RETRY_INTERVAL_SEC                     10	/* Seconds */

/* Maximum Retry Count for accept a JOIN request. */
#define JOIN_MAX_RETRY_FAILURE_COUNT                2	/* Times */

#define TX_AUTHENTICATION_RESPONSE_TIMEOUT_TU        512 /* TU. */

#define TX_ASSOCIATE_TIMEOUT_TU        2048 /* TU. */

#if CFG_AP_80211KVR_INTERFACE
#define SAP_CHAN_NOISE_GET_INFO_PERIOD        5000 /* ms */
#endif

/*******************************************************************************
 *                             D A T A   T Y P E S
 *******************************************************************************
 */
enum ENUM_AA_STATE {
	AA_STATE_IDLE = 0,
	SAA_STATE_SEND_AUTH1,
	SAA_STATE_WAIT_AUTH2,
	SAA_STATE_SEND_AUTH3,
	SAA_STATE_WAIT_AUTH4,
	SAA_STATE_SEND_ASSOC1,
	SAA_STATE_WAIT_ASSOC2,
	AAA_STATE_SEND_AUTH2,
	AAA_STATE_SEND_AUTH4,	/* We may not use,			     */
				/* because P2P GO didn't support WEP and 11r */
	AAA_STATE_SEND_ASSOC2,
	AA_STATE_RESOURCE,	/* A state for debugging the case of	     */
				/* out of msg buffer.			     */
	AA_STATE_NUM
};

#if (CFG_SUPPORT_SUPPLICANT_SME == 1)
enum ENUM_AA_SENT {
	AA_SENT_NONE = 0,
	AA_SENT_AUTH1, /* = auth transaction SN */
	AA_SENT_AUTH2,
	AA_SENT_AUTH3,
	AA_SENT_AUTH4,
	AA_SENT_ASSOC1, /* req */
	AA_SENT_ASSOC2, /* resp */
	AA_SENT_RESOURCE, /* A state for debugging the case of	 */
			/* out of msg buffer. */
	AA_SENT_NUM
};
#endif

enum ENUM_AA_FRM_TYPE {
	FRM_DISASSOC = 0,
	FRM_DEAUTH
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

/*******************************************************************************
 *                  F U N C T I O N   D E C L A R A T I O N S
 *******************************************************************************
 */

/*----------------------------------------------------------------------------*/
/* Routines in saa_fsm.c                                                      */
/*----------------------------------------------------------------------------*/
void
saaFsmSteps(IN struct ADAPTER *prAdapter,
	    IN struct STA_RECORD *prStaRec,
	    IN enum ENUM_AA_STATE eNextState,
	    IN struct SW_RFB *prRetainedSwRfb);

uint32_t
saaFsmSendEventJoinComplete(IN struct ADAPTER *prAdapter,
			    uint32_t rJoinStatus, struct STA_RECORD *prStaRec,
			    struct SW_RFB *prSwRfb);

void saaFsmRunEventStart(IN struct ADAPTER *prAdapter,
			 IN struct MSG_HDR *prMsgHdr);

uint32_t
saaFsmRunEventTxDone(IN struct ADAPTER *prAdapter,
		     IN struct MSDU_INFO *prMsduInfo,
		     IN enum ENUM_TX_RESULT_CODE rTxDoneStatus);

void saaFsmRunEventTxReqTimeOut(IN struct ADAPTER
				*prAdapter, IN unsigned long plParamPtr);

void saaFsmRunEventRxRespTimeOut(IN struct ADAPTER
				 *prAdapter, IN unsigned long ulParamPtr);

void saaFsmRunEventRxAuth(IN struct ADAPTER *prAdapter,
			  IN struct SW_RFB *prSwRfb);

uint32_t saaFsmRunEventRxAssoc(IN struct ADAPTER *prAdapter,
			       IN struct SW_RFB *prSwRfb);

uint32_t saaFsmRunEventRxDeauth(IN struct ADAPTER
				*prAdapter, IN struct SW_RFB *prSwRfb);

uint32_t saaFsmRunEventRxDisassoc(IN struct ADAPTER
				  *prAdapter, IN struct SW_RFB *prSwRfb);

void saaFsmRunEventAbort(IN struct ADAPTER *prAdapter,
			 IN struct MSG_HDR *prMsgHdr);

void saaChkDeauthfrmParamHandler(IN struct ADAPTER
				 *prAdapter, IN struct SW_RFB *prSwRfb,
				 IN struct STA_RECORD *prStaRec);

void
saaChkDisassocfrmParamHandler(IN struct ADAPTER *prAdapter,
			      IN struct WLAN_DISASSOC_FRAME *prDisassocFrame,
			      IN struct STA_RECORD *prStaRec,
			      IN struct SW_RFB *prSwRfb);

void
saaSendDisconnectMsgHandler(IN struct ADAPTER *prAdapter,
			    IN struct STA_RECORD *prStaRec,
			    IN struct BSS_INFO *prAisBssInfo,
			    IN enum ENUM_AA_FRM_TYPE eFrmType);

void saaFsmRunEventFTContinue(IN struct ADAPTER *prAdapter,
			      IN struct MSG_HDR *prMsgHdr);

#if (CFG_SUPPORT_SUPPLICANT_SME == 1)
void
saaSendAuthAssoc(IN struct ADAPTER *prAdapter,
			IN struct STA_RECORD *prStaRec);

/*Add for support WEP when enable wpa3*/
void saaSendAuthSeq3(IN struct ADAPTER *prAdapter,
			IN struct STA_RECORD *prStaRec);
#endif

/*----------------------------------------------------------------------------*/
/* Routines in aaa_fsm.c                                                      */
/*----------------------------------------------------------------------------*/
void aaaFsmRunEventRxAuth(IN struct ADAPTER *prAdapter,
			  IN struct SW_RFB *prSwRfb);

uint32_t aaaFsmRunEventRxAssoc(IN struct ADAPTER *prAdapter,
			       IN struct SW_RFB *prSwRfb);

uint32_t
aaaFsmRunEventTxDone(IN struct ADAPTER *prAdapter,
		     IN struct MSDU_INFO *prMsduInfo,
		     IN enum ENUM_TX_RESULT_CODE rTxDoneStatus);

#if CFG_AP_80211KVR_INTERFACE
void aaaMulAPAgentChanNoiseInitWorkHandler(
	struct work_struct *work);

void aaaMulAPAgentChanNoiseCollectionWorkHandler(
	struct work_struct *work);

void aaaMulAPAgentStaEventNotify(IN struct STA_RECORD *prStaRec,
	IN unsigned char *pucAddr, IN unsigned char fgIsConnected);

void aaaMulAPAgentUnassocStaMeasureTimeout(
	IN struct ADAPTER *prAdapter, unsigned long ulParamPtr);
#endif
#endif /* _AA_FSM_H */
