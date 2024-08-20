/* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause */
/*
 * Copyright (c) 2021 MediaTek Inc.
 */

#ifndef __SSUSB_EPCTL_CSR_REGS_H__
#define __SSUSB_EPCTL_CSR_REGS_H__

#ifdef __cplusplus
extern "C" {
#endif

/* ************************************************************************** */
/* */
/* SSUSB_EPCTL_CSR CR Definitions */
/* */
/* ************************************************************************** */

#define SSUSB_EPCTL_CSR_BASE 0x74011800

#define SSUSB_EPCTL_CSR_EP_RST_OPT_ADDR \
	(SSUSB_EPCTL_CSR_BASE + 0x090) /* 1890 */

#ifdef __cplusplus
}
#endif

#endif /* __SSUSB_EPCTL_CSR_REGS_H__ */
