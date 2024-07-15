/* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause */
/*
 * Copyright (c) 2009 MediaTek Inc.
 */

#ifndef _WF_CR_SW_DEF_H
#define _WF_CR_SW_DEF_H

/*******************************************************************************
* M A C R O S
********************************************************************************
*/


/******************************************************************************
*
*                     MCU_SYSRAM SW CR Definitions
*
******************************************************************************
*/
#define WF_SW_DEF_CR_BASE                0x0041F200

#define WF_SW_DEF_CR_WACPU_STAT_ADDR \
	(WF_SW_DEF_CR_BASE + 0x000) /* F200 */
#define WF_SW_DEF_CR_WACPU_SLEEP_STAT_ADDR \
	(WF_SW_DEF_CR_BASE + 0x004) /* F204 */
#define WF_SW_DEF_CR_WM2WA_ACTION_ADDR \
	(WF_SW_DEF_CR_BASE + 0x008) /* F208 */
#define WF_SW_DEF_CR_WA2WM_ACTION_ADDR \
	(WF_SW_DEF_CR_BASE + 0x00C) /* F20C */
#define WF_SW_DEF_CR_LP_DBG0_ADDR \
	(WF_SW_DEF_CR_BASE + 0x010) /* F210 */
#define WF_SW_DEF_CR_LP_DBG1_ADDR \
	(WF_SW_DEF_CR_BASE + 0x014) /* F214 */
#define WF_SW_DEF_CR_SER_STATUS_ADDR \
	(WF_SW_DEF_CR_BASE + 0x040) /* F240 */
#define WF_SW_DEF_CR_PLE_STATUS_ADDR \
	(WF_SW_DEF_CR_BASE + 0x044) /* F244 */
#define WF_SW_DEF_CR_PLE1_STATUS_ADDR \
	(WF_SW_DEF_CR_BASE + 0x048) /* F248 */
#define WF_SW_DEF_CR_PLE_AMSDU_STATUS_ADDR \
	(WF_SW_DEF_CR_BASE + 0x04C) /* F24C */
#define WF_SW_DEF_CR_PSE_STATUS_ADDR \
	(WF_SW_DEF_CR_BASE + 0x050) /* F250 */
#define WF_SW_DEF_CR_PSE1_STATUS_ADDR \
	(WF_SW_DEF_CR_BASE + 0x054) /* F254 */
#define WF_SW_DEF_CR_LAMC_WISR6_BN0_STATUS_ADDR \
	(WF_SW_DEF_CR_BASE + 0x058) /* F258 */
#define WF_SW_DEF_CR_LAMC_WISR6_BN1_STATUS_ADDR \
	(WF_SW_DEF_CR_BASE + 0x05C) /* F25C */
#define WF_SW_DEF_CR_LAMC_WISR7_BN0_STATUS_ADDR \
	(WF_SW_DEF_CR_BASE + 0x060) /* F260 */
#define WF_SW_DEF_CR_LAMC_WISR7_BN1_STATUS_ADDR \
	(WF_SW_DEF_CR_BASE + 0x064) /* F264 */
#define WF_SW_DEF_CR_USB_MCU_EVENT_ADD \
	(WF_SW_DEF_CR_BASE + 0x070) /* F270 */
#define WF_SW_DEF_CR_USB_HOST_ACK_ADDR \
	(WF_SW_DEF_CR_BASE + 0x074) /* F274 */

/*
* ---WF_SW_DEF_CR_WACPU_SLEEP_STAT_ADDR (0x0041F200 + 0x004)---
* SLEEP_STATUS[0] - (RW) 0: Awake, 1: sleep
* GATING_STATUS[1] - (RW) 0:Idle, 1: Gating
* RESERVED5[31..2] - (RO) Reserved bits
*/
#define WF_SW_DEF_CR_WACPU_SLEEP_STAT_SLEEP_ADDR \
	WF_SW_DEF_CR_WACPU_SLEEP_STAT_ADDR
#define WF_SW_DEF_CR_WACPU_SLEEP_STAT_SLEEP_MASK    0x00000001
#define WF_SW_DEF_CR_WACPU_SLEEP_STAT_SLEEP_SHFT    0
#define WF_SW_DEF_CR_WACPU_SLEEP_STAT_GATING_ADDR \
	WF_SW_DEF_CR_WACPU_SLEEP_STAT_ADDR
#define WF_SW_DEF_CR_WACPU_SLEEP_STAT_GATING_MASK   0x00000002
#define WF_SW_DEF_CR_WACPU_SLEEP_STAT_GATING_SHFT   1


#endif /* _WF_CR_SW_DEF_H */


