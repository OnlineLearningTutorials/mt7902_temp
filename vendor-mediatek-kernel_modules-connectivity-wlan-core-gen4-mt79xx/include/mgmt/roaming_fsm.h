/* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause */
/*
 * Copyright (c) 2016 MediaTek Inc.
 */
/*
 * Id:
 */

/*! \file   "roaming_fsm.h"
 *    \brief  This file defines the FSM for Roaming MODULE.
 *
 *    This file defines the FSM for Roaming MODULE.
 */


#ifndef _ROAMING_FSM_H
#define _ROAMING_FSM_H

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
/* Roaming Discovery interval, SCAN result need to be updated */
#define ROAMING_DISCOVERY_TIMEOUT_SEC               5	/* Seconds. */
#if CFG_SUPPORT_ROAMING_SKIP_ONE_AP
#define ROAMING_ONE_AP_SKIP_TIMES		3
#endif

/* #define ROAMING_NO_SWING_RCPI_STEP                  5 //rcpi */
/*******************************************************************************
 *                             D A T A   T Y P E S
 *******************************************************************************
 */
enum ENUM_ROAMING_FAIL_REASON {
	ROAMING_FAIL_REASON_CONNLIMIT = 0,
	ROAMING_FAIL_REASON_NOCANDIDATE,
	ROAMING_FAIL_REASON_NUM
};

/* events of roaming between driver and firmware */
enum ENUM_ROAMING_EVENT {
	ROAMING_EVENT_START = 0,
	ROAMING_EVENT_DISCOVERY,
	ROAMING_EVENT_ROAM,
	ROAMING_EVENT_FAIL,
	ROAMING_EVENT_ABORT,
	ROAMING_EVENT_NUM
};

enum ENUM_ROAMING_REASON {
	ROAMING_REASON_POOR_RCPI = 0,
	ROAMING_REASON_GOOD_RCPI,
	ROAMING_REASON_TX_ERR, /*Lowest rate, high PER*/
	ROAMING_REASON_RETRY,
	ROAMING_REASON_NUM
};

struct CMD_ROAMING_TRANSIT {
	uint16_t	u2Event;
	uint16_t	u2Data;
	uint16_t	u2RcpiLowThreshold;
	uint8_t	ucIsSupport11B;
	uint8_t	ucBssidx;
	enum ENUM_ROAMING_REASON	eReason;
	uint32_t	u4RoamingTriggerTime; /*sec in mcu*/
	uint8_t aucReserved2[8];
};


struct CMD_ROAMING_CTRL {
	uint8_t fgEnable;
	uint8_t ucRcpiAdjustStep;
	uint16_t u2RcpiLowThr;
	uint8_t ucRoamingRetryLimit;
	uint8_t ucRoamingStableTimeout;
	uint8_t aucReserved[2];
};

#if CFG_SUPPORT_ROAMING_SKIP_ONE_AP
struct CMD_ROAMING_SKIP_ONE_AP {
	uint8_t	  fgIsRoamingSkipOneAP;
	uint8_t	  aucReserved[3];
	uint8_t	  aucReserved2[8];
};
#endif

enum ENUM_ROAMING_STATE {
	ROAMING_STATE_IDLE = 0,
	ROAMING_STATE_DECISION,
	ROAMING_STATE_DISCOVERY,
	ROAMING_STATE_REQ_CAND_LIST,
	ROAMING_STATE_ROAM,
	ROAMING_STATE_NUM
};

struct ROAMING_INFO {
	u_int8_t fgIsEnableRoaming;

	enum ENUM_ROAMING_STATE eCurrentState;

	OS_SYSTIME rRoamingDiscoveryUpdateTime;
#if CFG_SUPPORT_DRIVER_ROAMING
	OS_SYSTIME rRoamingLastDecisionTime;
#endif

	u_int8_t fgDrvRoamingAllow;
	struct TIMER rWaitCandidateTimer;
	enum ENUM_ROAMING_REASON eReason;
	uint8_t ucPER;
};

enum ROAM_TYPE {
	ROAM_TYPE_RCPI,
	ROAM_TYPE_PER,

	ROAM_TYPE_NUM
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
void roamingFsmInit(IN struct ADAPTER *prAdapter,
	IN uint8_t ucBssIndex);

void roamingFsmUninit(IN struct ADAPTER *prAdapter,
	IN uint8_t ucBssIndex);

void roamingFsmSendCmd(IN struct ADAPTER *prAdapter,
	IN struct CMD_ROAMING_TRANSIT *prTransit);

void roamingFsmScanResultsUpdate(IN struct ADAPTER *prAdapter,
	IN uint8_t ucBssIndex);

void roamingFsmSteps(IN struct ADAPTER *prAdapter,
	IN enum ENUM_ROAMING_STATE eNextState,
	IN uint8_t ucBssIndex);

void roamingFsmRunEventStart(IN struct ADAPTER *prAdapter,
	IN uint8_t ucBssIndex);

void roamingFsmRunEventDiscovery(IN struct ADAPTER *prAdapter,
	IN struct CMD_ROAMING_TRANSIT *prTransit);

void roamingFsmRunEventRoam(IN struct ADAPTER *prAdapter,
	IN uint8_t ucBssIndex);

void roamingFsmRunEventFail(IN struct ADAPTER *prAdapter,
	IN uint32_t u4Reason,
	IN uint8_t ucBssIndex);

void roamingFsmRunEventAbort(IN struct ADAPTER *prAdapter,
	IN uint8_t ucBssIndex);

uint32_t roamingFsmProcessEvent(IN struct ADAPTER *prAdapter,
	IN struct CMD_ROAMING_TRANSIT *prTransit);

#endif /* _ROAMING_FSM_H */
