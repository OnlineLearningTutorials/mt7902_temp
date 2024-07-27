/* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause */
/*
 * Copyright (c) 2016 MediaTek Inc.
 */

/*! \file  mt7902.h
*    \brief This file contains the info of MT7902
*/

#ifdef MT7902

#ifndef _MT7902_H
#define _MT7902_H

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

#define MT7902_UDMA_CONN_INFRA_STATUS_SEL   0x74000A4C
#define MT7902_UDMA_CONN_INFRA_STATUS_VAL   0xD8000000
#define MT7902_UDMA_CONN_INFRA_STATUS       0x74000A48

#define MT7902_CHIP_ID                 (0x7902)
#define MT7902_SW_SYNC0                CONNAC2x_CONN_CFG_ON_CONN_ON_MISC_ADDR
#define MT7902_SW_SYNC0_RDY_OFFSET \
	CONNAC2x_CONN_CFG_ON_CONN_ON_MISC_DRV_FM_STAT_SYNC_SHFT
#define MT7902_PATCH_START_ADDR        (0x00900000)
#define MT7902_TOP_CFG_BASE			CONN_CFG_BASE
#define MT7902_TX_DESC_APPEND_LENGTH   32
#define MT7902_RX_DESC_LENGTH   24
#define MT7902_ARB_AC_MODE_ADDR (0x820E315C)

#define MT7902_A_DIE_VER_ADDR 0x70010020
#define MT7902_A_DIE_VER_BIT  BIT(7)

#define MT7902_BT_FW_VER_ADDR 0x7C812004
#define MT7902_BT_FW_VER_MASK 0xFF

/*------------------------------------------------------------------------------
 * MACRO for WTBL INFO GET
 *------------------------------------------------------------------------------
 */

#define MT7902_WIFI_LWTBL_BASE 0x820d4200
#define MT7902_WIFI_UWTBL_BASE 0x820c4094


/*------------------------------------------------------------------------------
 * MACRO for WFDMA INFO GET
 *------------------------------------------------------------------------------
 */
#define MT7902_WFDMA_COUNT 1

/*------------------------------------------------------------------------------
 * MACRO for debug MCU
 *------------------------------------------------------------------------------
 */
#define CURRENT_PC 0x3F
#define PC_LOG_IDX 0x20
#define PC_LOG_NUM 32


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
void mt7902_show_ple_info(
	struct ADAPTER *prAdapter,
	u_int8_t fgDumpTxd);

void mt7902_show_wfdma_info(
	IN struct ADAPTER *prAdapter);

#if defined(_HIF_SDIO)
u_int8_t mt7902_sdio_show_mcu_debug_info(
	struct ADAPTER *prAdapter,
	IN uint8_t *pucBuf, IN uint32_t u4Max,
	IN uint8_t ucFlag, OUT uint32_t *pu4Length);
#endif

#if (CFG_SUPPORT_DEBUG_SOP == 1)

#if defined(_HIF_PCIE)
void pcie_mt7902_dump_power_debug_cr(struct ADAPTER *prAdapter);
void pcie_mt7902_dump_wfsys_debug_cr(struct ADAPTER *prAdapter,
	uint8_t ucCase);
void pcie_mt7902_dump_bgfsys_debug_cr(struct ADAPTER *prAdapter,
	uint8_t ucCase);
void pcie_mt7902_dump_conninfra_debug_cr(struct ADAPTER *prAdapter,
	uint8_t ucCase);
#endif /* #if defined(_HIF_PCIE) */

#if defined(_HIF_USB)
void usb_mt7902_dump_power_debug_cr(struct ADAPTER *prAdapter);
void usb_mt7902_dump_wfsys_debug_cr(struct ADAPTER *prAdapter,
	uint8_t ucCase);
void usb_mt7902_dump_bgfsys_debug_cr(struct ADAPTER *prAdapter);
void usb_mt7902_dump_conninfra_debug_cr(struct ADAPTER *prAdapter,
	uint8_t ucCase);
#endif /* #if defined(_HIF_USB) */

#if defined(_HIF_SDIO)
void sdio_mt7902_dump_power_debug_cr(struct ADAPTER *prAdapter);
void sdio_mt7902_dump_wfsys_debug_cr(struct ADAPTER *prAdapter,
	uint8_t ucCase);
void sdio_mt7902_dump_conninfra_debug_cr(struct ADAPTER *prAdapter,
	uint8_t ucCase);
#endif /* #if defined(_HIF_SDIO) */

u_int8_t mt7902_show_debug_sop_info(struct ADAPTER *prAdapter,
	uint8_t ucCase);
#endif


/*******************************************************************************
*                              F U N C T I O N S
********************************************************************************
*/

#endif /* _MT7902_H */

#endif  /* MT7902 */
