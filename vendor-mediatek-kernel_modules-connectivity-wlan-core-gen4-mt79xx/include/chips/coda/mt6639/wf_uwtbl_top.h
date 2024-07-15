/* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause */
/*
 * Copyright (c) 2021 MediaTek Inc.
 */

#ifndef __WF_UWTBL_TOP_REGS_H__
#define __WF_UWTBL_TOP_REGS_H__

#include "hal_common.h"

#ifdef __cplusplus
extern "C" {
#endif


/* ************************************************************************** */
/*  */
/* WF_UWTBL_TOP CR Definitions */
/*  */
/* ************************************************************************** */

#define WF_UWTBL_TOP_BASE                                      0x820c4000

#define WF_UWTBL_TOP_KTVLBR0_ADDR \
	(WF_UWTBL_TOP_BASE + 0x0000) /* 4000 */
#define WF_UWTBL_TOP_UCR_ADDR \
	(WF_UWTBL_TOP_BASE + 0x0100) /* 4100 */
#define WF_UWTBL_TOP_WDUCR_ADDR \
	(WF_UWTBL_TOP_BASE + 0x0104) /* 4104 */
#define WF_UWTBL_TOP_KTCR_ADDR \
	(WF_UWTBL_TOP_BASE + 0x0108) /* 4108 */
#define WF_UWTBL_TOP_WIUCR_ADDR \
	(WF_UWTBL_TOP_BASE + 0x0110) /* 4110 */
#define WF_UWTBL_TOP_WMUDR_ADDR \
	(WF_UWTBL_TOP_BASE + 0x0114) /* 4114 */
#define WF_UWTBL_TOP_WMUMR_ADDR \
	(WF_UWTBL_TOP_BASE + 0x0118) /* 4118 */
#define WF_UWTBL_TOP_PICR0_ADDR \
	(WF_UWTBL_TOP_BASE + 0x0120) /* 4120 */
#define WF_UWTBL_TOP_PICR1_ADDR \
	(WF_UWTBL_TOP_BASE + 0x0124) /* 4124 */
#define WF_UWTBL_TOP_ITCR_ADDR \
	(WF_UWTBL_TOP_BASE + 0x0130) /* 4130 */
#define WF_UWTBL_TOP_ITDR0_ADDR \
	(WF_UWTBL_TOP_BASE + 0x0138) /* 4138 */
#define WF_UWTBL_TOP_ITDR1_ADDR \
	(WF_UWTBL_TOP_BASE + 0x013C) /* 413C */
#define WF_UWTBL_TOP_DFR_ADDR \
	(WF_UWTBL_TOP_BASE + 0x1FE0) /* 5FE0 */
#define WF_UWTBL_TOP_DMY0_ADDR \
	(WF_UWTBL_TOP_BASE + 0x1FF0) /* 5FF0 */
#define WF_UWTBL_TOP_DMY1_ADDR \
	(WF_UWTBL_TOP_BASE + 0x1FF4) /* 5FF4 */


#define WF_UWTBL_TOP_WDUCR_GROUP_ADDR \
	WF_UWTBL_TOP_WDUCR_ADDR
#define WF_UWTBL_TOP_WDUCR_GROUP_MASK                          0x0000003F
	/* GROUP[5..0] */
#define WF_UWTBL_TOP_WDUCR_GROUP_SHFT                          0

#define WF_UWTBL_TOP_WDUCR_TARGET_ADDR \
	WF_UWTBL_TOP_WDUCR_ADDR
#define WF_UWTBL_TOP_WDUCR_TARGET_MASK                         0x80000000
	/* TARGET[31] */
#define WF_UWTBL_TOP_WDUCR_TARGET_SHFT                         31

#ifdef __cplusplus
}
#endif

#endif /* __WF_UWTBL_TOP_REGS_H__ */
