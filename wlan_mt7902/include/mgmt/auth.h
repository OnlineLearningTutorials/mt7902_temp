/* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause */
/*
 * Copyright (c) 2016 MediaTek Inc.
 */
/*
 * Id: //Department/DaVinci/BRANCHES/MT6620_WIFI_DRIVER_V2_3/
 *							include/mgmt/auth.h#1
 */

/*! \file  auth.h
 *    \brief This file contains the authentication REQ/RESP of
 *	   IEEE 802.11 family for MediaTek 802.11 Wireless LAN Adapters.
 */


#ifndef _AUTH_H
#define _AUTH_H

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
/* Routines in auth.c                                                         */
/*----------------------------------------------------------------------------*/
void authAddIEChallengeText(IN struct ADAPTER *prAdapter,
			    IN OUT struct MSDU_INFO *prMsduInfo);

#if !CFG_SUPPORT_AAA
uint32_t authSendAuthFrame(IN struct ADAPTER *prAdapter,
			   IN struct STA_RECORD *prStaRec,
			   IN uint16_t u2TransactionSeqNum);
#else
uint32_t
authSendAuthFrame(IN struct ADAPTER *prAdapter,
			  IN struct STA_RECORD *prStaRec,
			  IN uint8_t uBssIndex,
			  IN struct SW_RFB *prFalseAuthSwRfb,
			  IN uint16_t u2TransactionSeqNum,
			  IN uint16_t u2StatusCode);
#endif /* CFG_SUPPORT_AAA */

uint32_t authCheckTxAuthFrame(IN struct ADAPTER *prAdapter,
			IN struct MSDU_INFO *prMsduInfo,
			IN uint16_t u2TransactionSeqNum);

uint32_t authCheckRxAuthFrameTransSeq(IN struct ADAPTER
			*prAdapter, IN struct SW_RFB *prSwRfb);

uint32_t
authCheckRxAuthFrameStatus(IN struct ADAPTER *prAdapter,
			   IN struct SW_RFB *prSwRfb,
			   IN uint16_t u2TransactionSeqNum,
			   OUT uint16_t *pu2StatusCode);

void authHandleIEChallengeText(struct ADAPTER *prAdapter,
			       struct SW_RFB *prSwRfb, struct IE_HDR *prIEHdr);

uint32_t authProcessRxAuth2_Auth4Frame(IN struct ADAPTER
				       *prAdapter, IN struct SW_RFB *prSwRfb);

uint32_t
authSendDeauthFrame(IN struct ADAPTER *prAdapter,
		    IN struct BSS_INFO *prBssInfo,
		    IN struct STA_RECORD *prStaRec,
		    IN struct SW_RFB *prClassErrSwRfb, IN uint16_t u2ReasonCode,
		    IN PFN_TX_DONE_HANDLER pfTxDoneHandler);

uint32_t authProcessRxDeauthFrame(IN struct SW_RFB *prSwRfb,
			IN uint8_t aucBSSID[], OUT uint16_t *pu2ReasonCode);

uint32_t
authProcessRxAuth1Frame(IN struct ADAPTER *prAdapter,
			IN struct SW_RFB *prSwRfb,
			IN uint8_t aucExpectedBSSID[],
			IN uint16_t u2ExpectedAuthAlgNum,
			IN uint16_t u2ExpectedTransSeqNum,
			OUT uint16_t *pu2ReturnStatusCode);
uint32_t
authProcessRxAuthFrame(IN struct ADAPTER *prAdapter,
			IN struct SW_RFB *prSwRfb,
			IN struct BSS_INFO *prBssInfo,
			OUT uint16_t *pu2ReturnStatusCode);

void authAddMDIE(IN struct ADAPTER *prAdapter,
		 IN OUT struct MSDU_INFO *prMsduInfo);

uint32_t authCalculateRSNIELen(struct ADAPTER *prAdapter, uint8_t ucBssIdx,
			       struct STA_RECORD *prStaRec);

void authAddRSNIE(IN struct ADAPTER *prAdapter,
		  IN OUT struct MSDU_INFO *prMsduInfo);

uint32_t authAddRSNIE_impl(IN struct ADAPTER *prAdapter,
		  IN OUT struct MSDU_INFO *prMsduInfo);

void authHandleFtIEs(struct ADAPTER *prAdapter, struct SW_RFB *prSwRfb,
		     struct IE_HDR *prIEHdr);

u_int8_t
authFloodingCheck(IN struct ADAPTER *prAdapter,
		IN struct BSS_INFO *prP2pBssInfo,
		IN struct SW_RFB *prSwRfb);

#endif /* _AUTH_H */
