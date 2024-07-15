/* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause */
/*
 * Copyright (c) 2019 MediaTek Inc.
 */

#ifndef __WF_UMIB_TOP_REGS_H__
#define __WF_UMIB_TOP_REGS_H__

#include "hal_common.h"

#ifdef __cplusplus
extern "C" {
#endif


/* ************************************************************************** */
/*  */
/* WF_UMIB_TOP CR Definitions */
/*  */
/* ************************************************************************** */

#define WF_UMIB_TOP_BASE                                       0x820cd000

#define WF_UMIB_TOP_MCR_ADDR \
	(WF_UMIB_TOP_BASE + 0x01C) /* D01C */
#define WF_UMIB_TOP_MMCR_ADDR \
	(WF_UMIB_TOP_BASE + 0x0F0) /* D0F0 */
#define WF_UMIB_TOP_MVCR_ADDR \
	(WF_UMIB_TOP_BASE + 0x3FC) /* D3FC */
#define WF_UMIB_TOP_ANCR_ADDR \
	(WF_UMIB_TOP_BASE + 0x400) /* D400 */
#define WF_UMIB_TOP_B0BROCR_ADDR \
	(WF_UMIB_TOP_BASE + 0x480) /* D480 */
#define WF_UMIB_TOP_B0BRBCR_ADDR \
	(WF_UMIB_TOP_BASE + 0x4D0) /* D4D0 */
#define WF_UMIB_TOP_B0BRDCR_ADDR \
	(WF_UMIB_TOP_BASE + 0x520) /* D520 */
#define WF_UMIB_TOP_B0BRMCR_ADDR \
	(WF_UMIB_TOP_BASE + 0x530) /* D530 */
#define WF_UMIB_TOP_B0BRPDCR_ADDR \
	(WF_UMIB_TOP_BASE + 0x540) /* D540 */
#define WF_UMIB_TOP_B0ARCR_ADDR \
	(WF_UMIB_TOP_BASE + 0x550) /* D550 */
#define WF_UMIB_TOP_B0RPDCR_ADDR \
	(WF_UMIB_TOP_BASE + 0x560) /* D560 */
#define WF_UMIB_TOP_B0BRIPCR0_ADDR \
	(WF_UMIB_TOP_BASE + 0x564) /* D564 */
#define WF_UMIB_TOP_B0BRIPCR1_ADDR \
	(WF_UMIB_TOP_BASE + 0x574) /* D574 */
#define WF_UMIB_TOP_B0BRIPCR2_ADDR \
	(WF_UMIB_TOP_BASE + 0x584) /* D584 */
#define WF_UMIB_TOP_B0BRIPCR3_ADDR \
	(WF_UMIB_TOP_BASE + 0x594) /* D594 */
#define WF_UMIB_TOP_B0BRIPCR4_ADDR \
	(WF_UMIB_TOP_BASE + 0x5A4) /* D5A4 */
#define WF_UMIB_TOP_B1BROCR_ADDR \
	(WF_UMIB_TOP_BASE + 0x5B4) /* D5B4 */
#define WF_UMIB_TOP_B1BRBCR_ADDR \
	(WF_UMIB_TOP_BASE + 0x604) /* D604 */
#define WF_UMIB_TOP_B1BRDCR_ADDR \
	(WF_UMIB_TOP_BASE + 0x654) /* D654 */
#define WF_UMIB_TOP_B1BRMCR_ADDR \
	(WF_UMIB_TOP_BASE + 0x664) /* D664 */
#define WF_UMIB_TOP_B1BRPDCR_ADDR \
	(WF_UMIB_TOP_BASE + 0x674) /* D674 */
#define WF_UMIB_TOP_B1ARCR_ADDR \
	(WF_UMIB_TOP_BASE + 0x684) /* D684 */
#define WF_UMIB_TOP_B1RPDCR_ADDR \
	(WF_UMIB_TOP_BASE + 0x694) /* D694 */
#define WF_UMIB_TOP_B1BRIPCR0_ADDR \
	(WF_UMIB_TOP_BASE + 0x698) /* D698 */
#define WF_UMIB_TOP_B1BRIPCR1_ADDR \
	(WF_UMIB_TOP_BASE + 0x6A8) /* D6A8 */
#define WF_UMIB_TOP_B1BRIPCR2_ADDR \
	(WF_UMIB_TOP_BASE + 0x6B8) /* D6B8 */
#define WF_UMIB_TOP_B1BRIPCR3_ADDR \
	(WF_UMIB_TOP_BASE + 0x6C8) /* D6C8 */
#define WF_UMIB_TOP_B1BRIPCR4_ADDR \
	(WF_UMIB_TOP_BASE + 0x6D8) /* D6D8 */
#define WF_UMIB_TOP_B2BROCR_ADDR \
	(WF_UMIB_TOP_BASE + 0x6E8) /* D6E8 */
#define WF_UMIB_TOP_B2BRBCR_ADDR \
	(WF_UMIB_TOP_BASE + 0x738) /* D738 */
#define WF_UMIB_TOP_B2BRDCR_ADDR \
	(WF_UMIB_TOP_BASE + 0x788) /* D788 */
#define WF_UMIB_TOP_B2BRMCR_ADDR \
	(WF_UMIB_TOP_BASE + 0x798) /* D798 */
#define WF_UMIB_TOP_B2BRPDCR_ADDR \
	(WF_UMIB_TOP_BASE + 0x7A8) /* D7A8 */
#define WF_UMIB_TOP_B2ARCR_ADDR \
	(WF_UMIB_TOP_BASE + 0x7B8) /* D7B8 */
#define WF_UMIB_TOP_B2RPDCR_ADDR \
	(WF_UMIB_TOP_BASE + 0x7C8) /* D7C8 */
#define WF_UMIB_TOP_B2BRIPCR0_ADDR \
	(WF_UMIB_TOP_BASE + 0x7CC) /* D7CC */
#define WF_UMIB_TOP_B2BRIPCR1_ADDR \
	(WF_UMIB_TOP_BASE + 0x7DC) /* D7DC */
#define WF_UMIB_TOP_B2BRIPCR2_ADDR \
	(WF_UMIB_TOP_BASE + 0x7EC) /* D7EC */
#define WF_UMIB_TOP_B2BRIPCR3_ADDR \
	(WF_UMIB_TOP_BASE + 0x7FC) /* D7FC */
#define WF_UMIB_TOP_B2BRIPCR4_ADDR \
	(WF_UMIB_TOP_BASE + 0x80C) /* D80C */

#ifdef __cplusplus
}
#endif

#endif /* __WF_UMIB_TOP_REGS_H__ */
