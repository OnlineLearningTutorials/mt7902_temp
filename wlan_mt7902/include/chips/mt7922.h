/* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause */
/*
 * Copyright (c) 2016 MediaTek Inc.
 */

/*! \file  mt7922.h
*    \brief This file contains the info of MT7922
*/

#ifdef MT7922

#ifndef _MT7922_H
#define _MT7922_H

/*******************************************************************************
*                         C O M P I L E R   F L A G S
********************************************************************************
*/

/*******************************************************************************
*                    E X T E R N A L   R E F E R E N C E S
********************************************************************************
*/

/*******************************************************************************
*                              C O N S T A N T S
********************************************************************************
*/
/*TODO: To use correct ID after FPGA uses correct ID @20170927*/

#define CONNAC2X_TOP_HCR 0x70010200
#define CONNAC2X_TOP_HVR 0x70010204
#define CONNAC2X_TOP_FVR 0x88000004
#define CONNAC2x_CONN_CFG_ON_BASE	0x7C060000
#define CONNAC2x_CONN_CFG_ON_CONN_ON_MISC_ADDR \
	(CONNAC2x_CONN_CFG_ON_BASE + 0xF0)
#define CONNAC2x_CONN_CFG_ON_CONN_ON_MISC_DRV_FM_STAT_SYNC_SHFT         0

#define MT7922_CHIP_ID                 (0x7922)
#define MT7922_SW_SYNC0                CONNAC2x_CONN_CFG_ON_CONN_ON_MISC_ADDR
#define MT7922_SW_SYNC0_RDY_OFFSET \
	CONNAC2x_CONN_CFG_ON_CONN_ON_MISC_DRV_FM_STAT_SYNC_SHFT
#define MT7922_PATCH_START_ADDR        (0x00900000)
#define MT7922_TOP_CFG_BASE			CONN_CFG_BASE
#define MT7922_TX_DESC_APPEND_LENGTH   32
#define MT7922_RX_DESC_LENGTH   24
#define MT7922_ARB_AC_MODE_ADDR (0x820E315C)

#define MT7922_A_DIE_VER_ADDR 0x70010020
#define MT7922_A_DIE_VER_BIT  BIT(7)

#define MT7922_BT_FW_VER_ADDR 0x7C812004
#define MT7922_BT_FW_VER_MASK 0xFF

/*------------------------------------------------------------------------------
 * MACRO for WTBL INFO GET
 *------------------------------------------------------------------------------
 */

#define MT7922_WIFI_LWTBL_BASE 0x820d4200
#define MT7922_WIFI_UWTBL_BASE 0x820c4094

#if CFG_SUPPORT_HOST_RX_WM_EVENT_FROM_PSE
#define MT7922_HOST_RX_WM_EVENT_FROM_PSE_RX_RING4_SIZE	32
#endif

/*------------------------------------------------------------------------------
 * MACRO for WFDMA INFO GET
 *------------------------------------------------------------------------------
 */
#define MT7922_WFDMA_COUNT 1

/*******************************************************************************
*                         D A T A   T Y P E S
********************************************************************************
*/

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

/*******************************************************************************
*                  F U N C T I O N   D E C L A R A T I O N S
********************************************************************************
*/
#if (CFG_SUPPORT_DEBUG_SOP == 1)
void pcie_mt7922_dump_wfsys_debug_cr(struct ADAPTER *prAdapter,
	uint8_t ucCase);
void pcie_mt7922_dump_bgfsys_debug_cr(struct ADAPTER *prAdapter,
	uint8_t ucCase);
void pcie_mt7922_dump_conninfra_debug_cr(struct ADAPTER *prAdapter,
	uint8_t ucCase);
u_int8_t mt7922_show_debug_sop_info(struct ADAPTER *prAdapter,
	uint8_t ucCase);
#endif

/*******************************************************************************
*                              F U N C T I O N S
********************************************************************************
*/

#endif /* _MT7922_H */

#endif  /* MT7922 */
