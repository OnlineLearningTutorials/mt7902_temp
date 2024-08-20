/* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause */
/*
 * Copyright (c) 2016 MediaTek Inc.
 */
/*
 ** Id: //Department/DaVinci/BRANCHES/
 * MT6620_WIFI_DRIVER_V2_3/include/pwr_mgt.h#1
 */

/*! \file   "pwr_mgt.h"
 *    \brief  In this file we define the STATE and EVENT for Power
 *            Management FSM.
 *
 *    The SCAN FSM is responsible for performing SCAN behavior when the Arbiter
 *    enter ARB_STATE_SCAN. The STATE and EVENT for SCAN FSM are defined here
 *    with detail description.
 */

#ifndef _PWR_MGT_H
#define _PWR_MGT_H
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
#define PM_UAPSD_AC0                        (BIT(0))
#define PM_UAPSD_AC1                        (BIT(1))
#define PM_UAPSD_AC2                        (BIT(2))
#define PM_UAPSD_AC3                        (BIT(3))

#define PM_UAPSD_ALL \
		(PM_UAPSD_AC0 | PM_UAPSD_AC1 | PM_UAPSD_AC2 | PM_UAPSD_AC3)
#define PM_UAPSD_NONE                       0

#define LP_OWN_BACK_TOTAL_DELAY_MS      2048	/* exponential of 2 */
#define LP_OWN_BACK_LOOP_DELAY_MS       1	/* exponential of 2 */
#define LP_OWN_REQ_CLR_INTERVAL_MS	200
#define LP_OWN_BACK_FAILED_RETRY_CNT    5
#define LP_OWN_BACK_FAILED_LOG_SKIP_MS  2000
#define LP_OWN_BACK_FAILED_RESET_CNT    5
#define LP_OWN_BACK_FAILED_DBGCR_POLL_ROUND	5
#define LP_DBGCR_POLL_ROUND				1
#define LP_OWN_BACK_LOOP_DELAY_MIN_US   900
#define LP_OWN_BACK_LOOP_DELAY_MAX_US   1000
#define POLL_MAILBOX_TIMEOUT_MS		2048
#define ISSUE_SW_INT_INTERVAL_MS	200
#define CLR_MAILBOX_ACK_LOOP_DELAY_MIN_US   900
#define CLR_MAILBOX_ACK_LOOP_DELAY_MAX_US   1000



/*******************************************************************************
 *                             D A T A   T Y P E S
 *******************************************************************************
 */
struct PM_PROFILE_SETUP_INFO {
	/* Profile setup */
	/* 0: AC_BE, 1: AC_BK, 2: AC_VI, 3: AC_VO */
	uint8_t ucBmpDeliveryAC;
	/* 0: AC_BE, 1: AC_BK, 2: AC_VI, 3: AC_VO */
	uint8_t ucBmpTriggerAC;

	uint8_t ucUapsdSp;	/* Number of triggered packets in UAPSD */

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
#if !CFG_ENABLE_FULL_PM
#define ACQUIRE_POWER_CONTROL_FROM_PM(_prAdapter)
#define RECLAIM_POWER_CONTROL_TO_PM(_prAdapter, _fgEnableGINT_in_IST)
#else
#define ACQUIRE_POWER_CONTROL_FROM_PM(_prAdapter) \
	{ \
			nicpmSetDriverOwn(_prAdapter); \
	}

#define RECLAIM_POWER_CONTROL_TO_PM(_prAdapter, _fgEnableGINT_in_IST) \
	{ \
			nicpmSetFWOwn(_prAdapter, _fgEnableGINT_in_IST); \
	}
#endif

/*******************************************************************************
 *                   F U N C T I O N   D E C L A R A T I O N S
 *******************************************************************************
 */

/*******************************************************************************
 *                              F U N C T I O N S
 *******************************************************************************
 */

#endif /* _PWR_MGT_H */
