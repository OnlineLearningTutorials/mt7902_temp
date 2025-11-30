/******************************************************************************
 *
 * This file is provided under a dual license.  When you use or
 * distribute this software, you may choose to be licensed under
 * version 2 of the GNU General Public License ("GPLv2 License")
 * or BSD License.
 *
 * GPLv2 License
 *
 * Copyright(C) 2019 MediaTek Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See http://www.gnu.org/licenses/gpl-2.0.html for more details.
 *
 * BSD LICENSE
 *
 * Copyright(C) 2019 MediaTek Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *  * Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *****************************************************************************/
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
