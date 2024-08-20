/* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause */
/*
 * Copyright (c) 2016 MediaTek Inc.
 */

/*! \file  dbg_comm.h
*    \brief This file contains the info of the dbg common function
*/


#ifndef _DBG_COMMON_H_
#define _DBG_COMMON_H_

struct wfdma_group_info {
	char name[20];
	u_int32_t hw_desc_base;
};

#if (CFG_SUPPORT_CONNAC3X == 0)
enum _ENUM_WFDMA_TYPE_T {
	WFDMA_TYPE_HOST = 0,
	WFDMA_TYPE_WM
};
#endif

void show_wfdma_interrupt_info(
	IN struct ADAPTER *prAdapter,
	IN enum _ENUM_WFDMA_TYPE_T enum_wfdma_type);

void show_wfdma_glo_info(
	IN struct ADAPTER *prAdapter,
	IN enum _ENUM_WFDMA_TYPE_T enum_wfdma_type);

void show_wfdma_dbg_probe_info(
	IN struct ADAPTER *prAdapter,
	IN enum _ENUM_WFDMA_TYPE_T enum_wfdma_type);

#endif /* _DBG_COMMON_H_ */
