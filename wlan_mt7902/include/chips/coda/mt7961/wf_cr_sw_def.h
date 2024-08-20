/* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause */
/*
 * Copyright (c) 2016 MediaTek Inc.
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
#define WF_SW_DEF_CR_BASE                0x00401A00

#define WF_SW_DEF_CR_SER_STATUS_ADDR \
	(WF_SW_DEF_CR_BASE + 0x000) /* E00 */
#define WF_SW_DEF_CR_PLE_STATUS_ADDR \
	(WF_SW_DEF_CR_BASE + 0x004) /* E04 */
#define WF_SW_DEF_CR_PLE1_STATUS_ADDR \
	(WF_SW_DEF_CR_BASE + 0x008) /* E08 */
#define WF_SW_DEF_CR_PLE_AMSDU_STATUS_ADDR \
	(WF_SW_DEF_CR_BASE + 0x00C) /* E0C */
#define WF_SW_DEF_CR_PSE_STATUS_ADDR \
	(WF_SW_DEF_CR_BASE + 0x010) /* E10 */
#define WF_SW_DEF_CR_PSE1_STATUS_ADDR \
	(WF_SW_DEF_CR_BASE + 0x014) /* E14 */
#define WF_SW_DEF_CR_LAMC_WISR6_BN0_STATUS_ADDR \
	(WF_SW_DEF_CR_BASE + 0x018) /* E18 */
#define WF_SW_DEF_CR_LAMC_WISR6_BN1_STATUS_ADDR \
	(WF_SW_DEF_CR_BASE + 0x01C) /* E1C */
#define WF_SW_DEF_CR_LAMC_WISR7_BN0_STATUS_ADDR \
	(WF_SW_DEF_CR_BASE + 0x020) /* E20 */
#define WF_SW_DEF_CR_LAMC_WISR7_BN1_STATUS_ADDR \
	(WF_SW_DEF_CR_BASE + 0x024) /* E24 */
#define WF_SW_DEF_CR_USB_MCU_EVENT_ADD \
	(WF_SW_DEF_CR_BASE + 0x028) /* E28 */
#define WF_SW_DEF_CR_USB_HOST_ACK_ADDR \
	(WF_SW_DEF_CR_BASE + 0x02C) /* E2C */
#define WF_SW_DEF_CR_ICAP_SPECTRUM_MODE_ADDR \
		(WF_SW_DEF_CR_BASE + 0x030) /* E30 */

#endif /* _WF_CR_SW_DEF_H */


