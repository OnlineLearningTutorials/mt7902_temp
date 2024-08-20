/* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause */
/*
 * Copyright (c) 2021 MediaTek Inc.
 */

#ifndef __CBTOP_RGU_REGS_H__
#define __CBTOP_RGU_REGS_H__

#ifdef __cplusplus
extern "C" {
#endif

/* ************************************************************************** */
/* */
/* CBTOP_RGU CR Definitions */
/* */
/* ************************************************************************** */

#define CBTOP_RGU_BASE 0x70002000

#define CBTOP_RGU_WF_SUBSYS_RST_ADDR (CBTOP_RGU_BASE + 0x600)	      /* 2600 */
#define CBTOP_RGU_WF_SUBSYS_RST_BYPASS_WFDMA_SLP_PROT_MASK                     \
	0x00000040 /* BYPASS_WFDMA_SLP_PROT[6] */
#define CBTOP_RGU_WF_SUBSYS_RST_WF_WHOLE_PATH_RST_MASK                         \
	0x00000001 /* WF_WHOLE_PATH_RST[0] */

#ifdef __cplusplus
}
#endif

#endif /* __CBTOP_RGU_REGS_H__ */
