/* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause */
/*
 * Copyright (c) 2016 MediaTek Inc.
 */
/*
 * Id: @(#)
 */

/*! \file   "cnm_scan.h"
 *   \brief
 *
 */


#ifndef _CNM_SCAN_H
#define _CNM_SCAN_H

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
#define SCN_CHANNEL_DWELL_TIME_MIN_MSEC		12
#define SCN_CHANNEL_DWELL_TIME_EXT_MSEC		98

#define SCN_TOTAL_PROBEREQ_NUM_FOR_FULL		3
#define SCN_SPECIFIC_PROBEREQ_NUM_FOR_FULL	1

#define SCN_TOTAL_PROBEREQ_NUM_FOR_PARTIAL	2
#define SCN_SPECIFIC_PROBEREQ_NUM_FOR_PARTIAL	1

/* Used by partial scan */
#define SCN_INTERLACED_CHANNEL_GROUPS_NUM	3

#define SCN_PARTIAL_SCAN_NUM			3

#define SCN_PARTIAL_SCAN_IDLE_MSEC		100

#define SCN_P2P_FULL_SCAN_PARAM			0

/*******************************************************************************
 *                             D A T A   T Y P E S
 *******************************************************************************
 */
/* The type of Scan Source */
enum ENUM_SCN_REQ_SOURCE {
	SCN_REQ_SOURCE_HEM = 0,
	SCN_REQ_SOURCE_NET_FSM,
	SCN_REQ_SOURCE_ROAMING,	/* ROAMING Module is independent of AIS FSM */
	SCN_REQ_SOURCE_OBSS,	/* 2.4G OBSS scan */
	SCN_REQ_SOURCE_NUM
};

enum ENUM_SCAN_PROFILE {
	SCAN_PROFILE_FULL = 0,
	SCAN_PROFILE_PARTIAL,
	SCAN_PROFILE_VOIP,
	SCAN_PROFILE_FULL_2G4,
	SCAN_PROFILE_NUM
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
 *                   F U N C T I O N   D E C L A R A T I O N S
 *******************************************************************************
 */

/*******************************************************************************
 *                              F U N C T I O N S
 *******************************************************************************
 */
#if 0
void cnmScanInit(void);

void cnmScanRunEventScanRequest(IN struct MSG_HDR *prMsgHdr);

u_int8_t cnmScanRunEventScanAbort(IN struct MSG_HDR *prMsgHdr);

void cnmScanProfileSelection(void);

void cnmScanProcessStart(void);

void cnmScanProcessStop(void);

void cnmScanRunEventReqAISAbsDone(IN struct MSG_HDR *prMsgHdr);

void cnmScanRunEventCancelAISAbsDone(IN struct MSG_HDR *prMsgHdr);

void cnmScanPartialScanTimeout(uint32_t u4Param);

void cnmScanRunEventScnFsmComplete(IN struct MSG_HDR *prMsgHdr);
#endif

#endif /* _CNM_SCAN_H */
