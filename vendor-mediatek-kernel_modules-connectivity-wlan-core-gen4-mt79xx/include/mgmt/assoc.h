/* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause */
/*
 * Copyright (c) 2016 MediaTek Inc.
 */
/*
 * Id: //Department/DaVinci/BRANCHES/MT6620_WIFI_DRIVER_V2_3/
 *							include/mgmt/assoc.h#1
 */

/*! \file  assoc.h
 *    \brief This file contains the ASSOC REQ/RESP of
 *	   IEEE 802.11 family for MediaTek 802.11 Wireless LAN Adapters.
 */


#ifndef _ASSOC_H
#define _ASSOC_H

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
/* Routines in assoc.c                                                        */
/*----------------------------------------------------------------------------*/
uint32_t assocSendReAssocReqFrame(IN struct ADAPTER
				*prAdapter, IN struct STA_RECORD *prStaRec);

uint32_t assocCheckTxReAssocReqFrame(IN struct ADAPTER
				*prAdapter, IN struct MSDU_INFO *prMsduInfo);

uint32_t assocCheckTxReAssocRespFrame(IN struct ADAPTER
				*prAdapter, IN struct MSDU_INFO *prMsduInfo);

uint32_t
assocCheckRxReAssocRspFrameStatus(IN struct ADAPTER
				*prAdapter, IN struct SW_RFB *prSwRfb,
				OUT uint16_t *pu2StatusCode);

uint32_t assocSendDisAssocFrame(IN struct ADAPTER
				*prAdapter, IN struct STA_RECORD *prStaRec,
				IN uint16_t u2ReasonCode);

uint32_t
assocProcessRxDisassocFrame(IN struct ADAPTER *prAdapter,
				IN struct SW_RFB *prSwRfb,
				IN uint8_t aucBSSID[],
				OUT uint16_t *pu2ReasonCode);

uint32_t assocProcessRxAssocReqFrame(IN struct ADAPTER
				*prAdapter, IN struct SW_RFB *prSwRfb,
				OUT uint16_t *pu2StatusCode);

uint32_t assocSendReAssocRespFrame(IN struct ADAPTER
				*prAdapter, IN struct STA_RECORD *prStaRec);

uint16_t assocBuildCapabilityInfo(IN struct ADAPTER
				*prAdapter, IN struct STA_RECORD *prStaRec);

uint16_t assoc_get_nonwfa_vend_ie_len(struct ADAPTER *prAdapter,
	uint8_t ucBssIndex);

void assoc_build_nonwfa_vend_ie(struct ADAPTER *prAdapter,
	struct MSDU_INFO *prMsduInfo);

void assocGenerateMDIE(IN struct ADAPTER *prAdapter,
		       IN OUT struct MSDU_INFO *prMsduInfo);

#endif /* _ASSOC_H */
