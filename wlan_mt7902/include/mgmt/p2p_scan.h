/* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause */
/*
 * Copyright (c) 2016 MediaTek Inc.
 */
/*
 ** Id: //Department/DaVinci/TRUNK/WiFi_P2P_Driver/include/mgmt/p2p_scan.h#1
 */

/*! \file   "scan.h"
 *  \brief
 *
 */

#ifndef _P2P_SCAN_H
#define _P2P_SCAN_H

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

void scnEventReturnChannel(IN struct ADAPTER *prAdapter,
		IN uint8_t ucScnSeqNum);

u_int8_t scanUpdateP2pDeviceDesc(IN struct ADAPTER *prAdapter,
		IN struct BSS_DESC *prBssDesc);

void
scanP2pProcessBeaconAndProbeResp(IN struct ADAPTER *prAdapter,
		IN struct SW_RFB *prSwRfb,
		IN uint32_t *prStatus,
		IN struct BSS_DESC *prBssDesc,
		IN struct WLAN_BEACON_FRAME *prWlanBeaconFrame);

void scanRemoveAllP2pBssDesc(struct ADAPTER *prAdapter);

void scanRemoveP2pBssDesc(struct ADAPTER *prAdapter,
		struct BSS_DESC *prBssDesc);

struct BSS_DESC *scanP2pSearchDesc(IN struct ADAPTER *prAdapter,
		IN struct P2P_CONNECTION_REQ_INFO *prConnReqInfo);

#endif
