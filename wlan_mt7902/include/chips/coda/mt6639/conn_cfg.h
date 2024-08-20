/* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause */
/*
 * Copyright (c) 2021 MediaTek Inc.
 */

#ifndef __CONN_CFG_REGS_H__
#define __CONN_CFG_REGS_H__

#include "hal_common.h"

#ifdef __cplusplus
extern "C" {
#endif


/* ************************************************************************** */
/*  */
/* CONN_CFG CR Definitions */
/*  */
/* ************************************************************************** */

#define CONN_CFG_BASE            (0x18011000 + CONN_INFRA_REMAPPING_OFFSET)

#define CONN_CFG_IP_VERSION_ADDR \
	(CONN_CFG_BASE + 0x000) /* 1000 */
#define CONN_CFG_CFG_VERSION_ADDR \
	(CONN_CFG_BASE + 0x004) /* 1004 */
#define CONN_CFG_STRAP_STATUS_ADDR \
	(CONN_CFG_BASE + 0x010) /* 1010 */
#define CONN_CFG_EFUSE_ADDR \
	(CONN_CFG_BASE + 0x020) /* 1020 */
#define CONN_CFG_PLL_STATUS_ADDR \
	(CONN_CFG_BASE + 0x030) /* 1030 */
#define CONN_CFG_BUS_STATUS_ADDR \
	(CONN_CFG_BASE + 0x034) /* 1034 */
#define CONN_CFG_CFG_RSV0_ADDR \
	(CONN_CFG_BASE + 0x040) /* 1040 */
#define CONN_CFG_CFG_RSV1_ADDR \
	(CONN_CFG_BASE + 0x044) /* 1044 */
#define CONN_CFG_CMDBT_FETCH_START_ADDR0_ADDR \
	(CONN_CFG_BASE + 0x050) /* 1050 */
#define CONN_CFG_CONN_INFRA_SYSRAM_CTRL_0_ADDR \
	(CONN_CFG_BASE + 0x060) /* 1060 */
#define CONN_CFG_CONN_INFRA_SYSRAM_CTRL_1_ADDR \
	(CONN_CFG_BASE + 0x064) /* 1064 */
#define CONN_CFG_CONN_INFRA_SYSRAM_CTRL_2_ADDR \
	(CONN_CFG_BASE + 0x068) /* 1068 */
#define CONN_CFG_CONN_INFRA_SYSRAM_CTRL_3_ADDR \
	(CONN_CFG_BASE + 0x06C) /* 106C */
#define CONN_CFG_EMI_CTL_0_ADDR \
	(CONN_CFG_BASE + 0x100) /* 1100 */
#define CONN_CFG_EMI_CTL_1_ADDR \
	(CONN_CFG_BASE + 0x104) /* 1104 */
#define CONN_CFG_EMI_CTL_TOP_ADDR \
	(CONN_CFG_BASE + 0x110) /* 1110 */
#define CONN_CFG_EMI_CTL_WF_ADDR \
	(CONN_CFG_BASE + 0x114) /* 1114 */
#define CONN_CFG_EMI_CTL_BT_ADDR \
	(CONN_CFG_BASE + 0x118) /* 1118 */
#define CONN_CFG_EMI_CTL_GPS_ADDR \
	(CONN_CFG_BASE + 0x11C) /* 111C */
#define CONN_CFG_EMI_CTL_GPS_L1_ADDR \
	(CONN_CFG_BASE + 0X120) /* 1120 */
#define CONN_CFG_EMI_CTL_GPS_L5_ADDR \
	(CONN_CFG_BASE + 0X124) /* 1124 */
#define CONN_CFG_EMI_PROBE_ADDR \
	(CONN_CFG_BASE + 0x130) /* 1130 */
#define CONN_CFG_EMI_PROBE_1_ADDR \
	(CONN_CFG_BASE + 0x134) /* 1134 */

#ifdef __cplusplus
}
#endif

#endif /* __CONN_CFG_REGS_H__ */
