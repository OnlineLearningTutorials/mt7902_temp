/* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause */
/*
 * Copyright (c) 2019 MediaTek Inc.
 */
/*! \file   hal_wfsys_reset_mt7961.h
*    \brief  WFSYS reset HAL API for MT7961
*
*    This file contains all routines which are exported
     from MediaTek 802.11 Wireless LAN driver stack to GLUE Layer.
*/

#ifndef _HAL_WFSYS_RESET_MT7961_H
#define _HAL_WFSYS_RESET_MT7961_H

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

#if defined(_HIF_PCIE)

#define CONN_INFRA_BUS_CR_BASE                     0x7C00E000

/* E24C */
#define CONN_INFRA_BUS_CR_PCIE2AP_REMAP_2_ADDR                  \
        (CONN_INFRA_BUS_CR_BASE + 0x24C)
#define CONN_INFRA_BUS_CR_PCIE2AP_REMAP_2_DEFAULT  0x18451844
#define R_PCIE2AP_PUBLIC_REMAPPING_4_BUS_ADDR      0x40000

#define PCIE_MAC_IREG_BASE                         0x74030000

/* 0188 */
#define PCIE_MAC_IREG_IMASK_HOST_ADDR                           \
        (PCIE_MAC_IREG_BASE + 0x0188)

/* 018C */
#define PCIE_MAC_IREG_ISTATUS_HOST_ADDR                         \
        (PCIE_MAC_IREG_BASE + 0x018C)
#define PCIE_MAC_IREG_IMASK_HOST_WDT_INT_MASK      0x00000100
#define PCIE_MAC_IREG_ISTATUS_HOST_WDT_INT_MASK    0x00000100

#define CONN_INFRA_RGU_BASE                        0x7C000000

/* 0140 */
#define CONN_INFRA_RGU_WFSYS_SW_RST_B_ADDR                      \
        (CONN_INFRA_RGU_BASE + 0x140)

/* WFSYS_SW_INIT_DONE[4] */
#define CONN_INFRA_RGU_WFSYS_SW_RST_B_WFSYS_SW_INIT_DONE_MASK   \
        0x00000010

#define MMIO_READ_FAIL                             0xFFFFFFFF

#define WF_WFDMA_HOST_DMA0_WPDMA_GLO_CFG_TX_DMA_BUSY_MASK 0x2
#define WF_WFDMA_HOST_DMA0_WPDMA_GLO_CFG_RX_DMA_BUSY_MASK 0x8

#define WF_WFDMA_INT_WRAP_CSR_WFDMA_HIF_MISC_ADDR 0x57000044
#define WF_WFDMA_INT_WRAP_CSR_WFDMA_HIF_MISC_HIF_BUSY_MASK 0x1

#define WF_WFDMA_EXT_WRAP_CSR_WFDMA_HIF_MISC_ADDR 0x7C027044
#define WF_WFDMA_EXT_WRAP_CSR_WFDMA_HIF_MISC_HIF_BUSY_MASK 0x1


#endif /* defined(_HIF_PCIE) */

/*******************************************************************************
*                             D A T A   T Y P E S
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
*                   F U N C T I O N   D E C L A R A T I O N S
********************************************************************************
*/

/*******************************************************************************
*                              F U N C T I O N S
********************************************************************************
*/

u_int8_t mt7961HalCbtopRguWfRst(struct ADAPTER *prAdapter,
				u_int8_t fgAssertRst);

u_int8_t mt7961HalPollWfsysSwInitDone(struct ADAPTER *prAdapter);


#if defined(_HIF_PCIE)

u_int8_t mt7961HalPollWfdmaIdle(struct ADAPTER *prAdapter);

#endif /* defined(_HIF_PCIE) */

#if defined(_HIF_USB)

u_int8_t mt7961HalUsbEpctlRstOpt(struct ADAPTER *prAdapter,
				 u_int8_t fgIsRstScopeIncludeToggleBit);

#endif    /* defined(_HIF_USB) */

#if defined(_HIF_SDIO)
u_int8_t mt7961HalSetNoBTFwOwnEn(IN int32_t i4Enable);

#endif /* defined(_HIF_SDIO) */

#endif /* _HAL_WFSYS_RESET_MT7961_H */
