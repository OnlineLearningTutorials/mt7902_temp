/* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause */
/*
 * Copyright (c) 2016 MediaTek Inc.
 */
/*
 ** Id: //Department/DaVinci/TRUNK/WiFi_P2P_Driver/include/mgmt/p2p_rlm_obss.h#1
 */

/*! \file   "rlm_obss.h"
 *  \brief
 */

#ifndef _P2P_RLM_OBSS_H
#define _P2P_RLM_OBSS_H

/******************************************************************************
 *                         C O M P I L E R   F L A G S
 ******************************************************************************
 */

/******************************************************************************
 *                    E X T E R N A L   R E F E R E N C E S
 ******************************************************************************
 */

/******************************************************************************
 *                              C O N S T A N T S
 ******************************************************************************
 */

/******************************************************************************
 *                                 M A C R O S
 ******************************************************************************
 */

/******************************************************************************
 *                             D A T A   T Y P E S
 ******************************************************************************
 */

/******************************************************************************
 *                            P U B L I C   D A T A
 ******************************************************************************
 */

/******************************************************************************
 *                           P R I V A T E   D A T A
 ******************************************************************************
 */

/******************************************************************************
 *                  F U N C T I O N   D E C L A R A T I O N S
 ******************************************************************************
 */

void rlmRspGenerateObssScanIE(struct ADAPTER *prAdapter,
		struct MSDU_INFO *prMsduInfo);

void rlmProcessPublicAction(struct ADAPTER *prAdapter, struct SW_RFB *prSwRfb);

void rlmHandleObssStatusEventPkt(struct ADAPTER *prAdapter,
		struct EVENT_AP_OBSS_STATUS *prObssStatus);

uint8_t rlmObssChnlLevel(struct BSS_INFO *prBssInfo,
		enum ENUM_BAND eBand, uint8_t ucPriChannel,
		enum ENUM_CHNL_EXT eExtend);

void rlmObssScanExemptionRsp(struct ADAPTER *prAdapter,
		struct BSS_INFO *prBssInfo, struct SW_RFB *prSwRfb);

#endif
