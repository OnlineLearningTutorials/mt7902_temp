/* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause */
/*
 * Copyright (c) 2016 MediaTek Inc.
 */

/*! \file  mt7915.h
*    \brief This file contains the info of MT7915
*/

#ifdef MT7915

#ifndef _MT7915_H
#define _MT7915_H

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
#define CONNAC2X_TOP_FVR 0x70010208
#define CONNAC2x_CONN_CFG_ON_BASE	0x7C060000
#define CONNAC2x_CONN_CFG_ON_CONN_ON_MISC_ADDR \
	(CONNAC2x_CONN_CFG_ON_BASE + 0xF0)
#define CONNAC2x_CONN_CFG_ON_CONN_ON_MISC_DRV_FM_STAT_SYNC_SHFT         0

/*
 * 0 is MT7915 E1 RX padding rule
 * 1 is 8 bytes RX padding rule for MT7915e2 and chips after MT7915e2
 */
#define CONNAC2X_WFDMA_RX_CSO_OPTION BIT(8)

#define MT7915_CHIP_ID                 (0x7915)
#define MT7915_SW_SYNC0                CONNAC2x_CONN_CFG_ON_CONN_ON_MISC_ADDR
#define MT7915_SW_SYNC0_RDY_OFFSET \
	CONNAC2x_CONN_CFG_ON_CONN_ON_MISC_DRV_FM_STAT_SYNC_SHFT
#define MT7915_PATCH_START_ADDR        (0x00200000)
#define MT7915_TOP_CFG_BASE			CONN_CFG_BASE
#define MT7915_TX_DESC_APPEND_LENGTH        44
#define MT7915_RX_DESC_LENGTH               24
#define MT7915_ARB_AC_MODE_ADDR (0x820e3020)

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

void mt7915_show_ple_info(
	struct ADAPTER *prAdapter,
	u_int8_t fgDumpTxd);
void mt7915_show_pse_info(
	struct ADAPTER *prAdapter);

/*******************************************************************************
*                              F U N C T I O N S
********************************************************************************
*/

#endif /* _MT7915_H */
#endif  /* MT7915 */
