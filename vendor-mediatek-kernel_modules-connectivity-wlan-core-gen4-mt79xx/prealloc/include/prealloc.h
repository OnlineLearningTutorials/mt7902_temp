/* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause */
/*
 * Copyright (c) 2017 MediaTek Inc.
 */
/*! \file   "prealloc.h"
*   \brief  This file contains the declairation of memory preallocation module
*/

/*******************************************************************************
*                         C O M P I L E R   F L A G S
********************************************************************************
*/

/*******************************************************************************
*                    E X T E R N A L   R E F E R E N C E S
********************************************************************************
*/
#ifndef _PREALLOC_H
#define _PREALLOC_H

/*******************************************************************************
*                              C O N S T A N T S
********************************************************************************
*/

/*******************************************************************************
*                             D A T A   T Y P E S
********************************************************************************
*/
enum ENUM_MEM_ID {
	MEM_ID_NIC_ADAPTER,
	MEM_ID_IO_BUFFER,
#if defined(_HIF_SDIO)
	MEM_ID_IO_CTRL,
	MEM_ID_RX_DATA,
#endif
#if defined(_HIF_USB)
	MEM_ID_TX_CMD,
	MEM_ID_TX_DATA_FFA,
	MEM_ID_TX_DATA,
	MEM_ID_RX_EVENT,
	MEM_ID_RX_DATA,
#if CFG_CHIP_RESET_SUPPORT
	MEM_ID_RX_WDT,
#endif
#endif

	MEM_ID_NUM, /* END, Do not modify */
};

/*******************************************************************************
*                            P U B L I C   D A T A
********************************************************************************
*/

/*******************************************************************************
*                           P R I V A T E   D A T A
********************************************************************************
*/

/*******************************************************************************
*                                 M A C R O S
********************************************************************************
*/
#define MP_Dbg(_Fmt...)  pr_info("[wlan][MemPrealloc] " _Fmt)
#define MP_Info(_Fmt...)  pr_info("[wlan][MemPrealloc] " _Fmt)
#define MP_Err(_Fmt...) pr_info("[wlan][MemPrealloc] " _Fmt)


/*******************************************************************************
*                   F U N C T I O N   D E C L A R A T I O N S
********************************************************************************
*/
void *preallocGetMem(enum ENUM_MEM_ID memId);

#endif /* _PREALLOC_H */
