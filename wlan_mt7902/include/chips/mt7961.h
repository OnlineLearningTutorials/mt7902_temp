/******************************************************************************
 *
 * This file is provided under a dual license.  When you use or
 * distribute this software, you may choose to be licensed under
 * version 2 of the GNU General Public License ("GPLv2 License")
 * or BSD License.
 *
 * GPLv2 License
 *
 * Copyright(C) 2016 MediaTek Inc.
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
 * Copyright(C) 2016 MediaTek Inc. All rights reserved.
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

/*! \file  mt7961.h
*    \brief This file contains the info of MT7961
*/

#if defined(MT7961) || defined(MT7922) || defined(MT7902)

#ifndef _MT7961_H
#define _MT7961_H

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

#define CONNAC2x_CONN_CFG_ON_BUS_MCU_STAT_ADDR \
	(CONNAC2x_CONN_CFG_ON_BASE + 0x00)
#define CONNAC2x_CONN_CFG_ON_MCU_STAT_CLK_DETECT_BUS_CLR_PULSE_MASK   0x1
#define CONNAC2x_CONN_CFG_ON_MCU_STAT_CLK_DETECT_BUS_CLR_PULSE_SHFT     0

#define CONNAC2x_CONN_CFG_ON_WF_BAND0_LPCTL_ADDR \
	(CONNAC2x_CONN_CFG_ON_BASE + 0x10)
#define CONNAC2x_CONN_CFG_ON_HOST_WF_B0_AP_OWNER_STATE_SYNC_MASK      0x4
#define CONNAC2x_CONN_CFG_ON_HOST_WF_B0_AP_OWNER_STATE_SYNC_SHFT        2
#define CONNAC2x_CONN_CFG_ON_WF_B0_AP_HOST_CLR_FW_OWN_HS_PULSE_MASK   0x2
#define CONNAC2x_CONN_CFG_ON_WF_B0_AP_HOST_CLR_FW_OWN_HS_PULSE_SHFT     1
#define CONNAC2x_CONN_CFG_ON_WF_B0_AP_HOST_SET_FW_OWN_HS_PULSE_MASK   0x1
#define CONNAC2x_CONN_CFG_ON_WF_B0_AP_HOST_SET_FW_OWN_HS_PULSE_SHFT     0

#define CONNAC2x_CONN_CFG_ON_CONN_ON_MISC_ADDR \
	(CONNAC2x_CONN_CFG_ON_BASE + 0xF0)
#define CONNAC2x_CONN_CFG_ON_CONN_ON_MISC_DRV_FM_STAT_SYNC_SHFT         0

#define CONNAC2x_CONN_CFG_ON_CONN_INFRA_ON_BUS_TIMEOUT_IRQ_ADDR \
	(CONNAC2x_CONN_CFG_ON_BASE + 0x014C)

#define CONNAC2x_CONN_CFG_ON_WF_TOP_RGU_ON_DEBUG_PORT_ADDR \
	(CONNAC2x_CONN_CFG_ON_BASE + 0x0214)

#define CONNAC2x_CONN_CFG_ON_WF_MAILBOX_DBG_ADDR \
	(CONNAC2x_CONN_CFG_ON_BASE + 0x0260)

#define CONNAC2x_CONN_CFG_ON_N13_MCU_MAILBOX_DBG_ADDR \
	(CONNAC2x_CONN_CFG_ON_BASE + 0x0264)

#define CONN_INFRA_CFG_BASE	0x7C001000
#define CONN_CFG_CONN_HW_VER_ADDR \
	(CONN_INFRA_CFG_BASE + 0x000) /* 1000 */

#define MT7961_CHIP_ID                 (0x7961)
#define MT7961_SW_SYNC0                CONNAC2x_CONN_CFG_ON_CONN_ON_MISC_ADDR
#define MT7961_SW_SYNC0_RDY_OFFSET \
	CONNAC2x_CONN_CFG_ON_CONN_ON_MISC_DRV_FM_STAT_SYNC_SHFT
#define MT7961_PATCH_START_ADDR        (0x00900000)
#define MT7961_TOP_CFG_BASE			CONN_CFG_BASE
#define MT7961_TX_DESC_APPEND_LENGTH   32
#define MT7961_RX_DESC_LENGTH   24
#define MT7961_ARB_AC_MODE_ADDR (0x820E315C)

#define MT7961_A_DIE_VER_ADDR 0x70010020
#define MT7961_A_DIE_VER_BIT  BIT(7)
#define MT7961_A_DIE_7921     0
#define MT7961_A_DIE_7920     BIT(7)
#define MT7961_A_DIE_7921_FLAVOR 0x1
#define MT7961_A_DIE_7920_FLAVOR 0x1a

#define MT7961_BT_FW_VER_ADDR 0x7C812004
#define MT7961_BT_FW_VER_MASK 0xFF

/*------------------------------------------------------------------------------
 * MACRO for debug MCU
 *------------------------------------------------------------------------------
 */
#define CURRENT_PC 0x3F
#define PC_LOG_IDX 0x20
#define PC_LOG_NUM 32

/*------------------------------------------------------------------------------
 * MACRO for WTBL INFO GET
 *------------------------------------------------------------------------------
 */

#define MT7961_WIFI_LWTBL_BASE 0x820d4200
#define MT7961_WIFI_UWTBL_BASE 0x820c4094

#if CFG_SUPPORT_HOST_RX_WM_EVENT_FROM_PSE
#define MT7961_HOST_RX_WM_EVENT_FROM_PSE_RX_RING4_SIZE	32
#endif

/*------------------------------------------------------------------------------
 * MACRO for WFDMA INFO GET
 *------------------------------------------------------------------------------
 */
#define MT7961_WFDMA_COUNT 1

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
void mt7961_show_ple_info(
	struct ADAPTER *prAdapter,
	u_int8_t fgDumpTxd);

void mt7961_show_pse_info(
	struct ADAPTER *prAdapter);

void mt7961_show_wfdma_info(
	IN struct ADAPTER *prAdapter);

#if defined(_HIF_SDIO)
u_int8_t sdio_show_mcu_debug_info(
	struct ADAPTER *prAdapter,
	IN uint8_t *pucBuf, IN uint32_t u4Max,
	IN uint8_t ucFlag, OUT uint32_t *pu4Length);
u_int8_t mt7961_get_sdio_debug_info(
	struct ADAPTER *prAdapter);
#elif defined(_HIF_USB)
u_int8_t usb_show_mcu_debug_info(
	struct ADAPTER *prAdapter,
	IN uint8_t *pucBuf, IN uint32_t u4Max,
	IN uint8_t ucFlag, OUT uint32_t *pu4Length);
#elif defined(_HIF_PCIE)
u_int8_t pcie_show_mcu_debug_info(
	struct ADAPTER *prAdapter,
	IN uint8_t *pucBuf, IN uint32_t u4Max,
	IN uint8_t ucFlag, OUT uint32_t *pu4Length);

bool pcie_show_csr_info(IN struct ADAPTER *prAdapter);
#endif

#if (CFG_SUPPORT_DEBUG_SOP == 1)
#if defined(_HIF_USB)
void usb_mt7961_dump_subsys_debug_cr(struct ADAPTER *prAdapter);
void usb_mt7961_dump_conninfra_debug_cr(struct ADAPTER *prAdapter);
#elif defined(_HIF_PCIE)
void pcie_mt7961_dump_subsys_debug_cr(struct ADAPTER *prAdapter);
void pcie_mt7961_dump_conninfra_debug_cr(struct ADAPTER *prAdapter);
#endif
u_int8_t mt7961_show_debug_sop_info(struct ADAPTER *prAdapter,
	uint8_t ucCase);
#endif


#if defined(_HIF_PCIE) || defined(_HIF_AXI)

void mt7961EnableInterrupt(
	struct ADAPTER *prAdapter);

void mt7961DisableInterrupt(
	struct ADAPTER *prAdapter);

uint8_t mt7961SetRxRingHwAddr(
	struct RTMP_RX_RING *prRxRing,
	struct BUS_INFO *prBusInfo,
	uint32_t u4SwRingIdx);

void wlanBuzzardInitPcieInt(
	struct GLUE_INFO *prGlueInfo);

#if CFG_SUPPORT_PCIE_ASPM_IMPROVE
void mt7961ConfigPcieASPM(
	struct GLUE_INFO *prGlueInfo,
	bool fgActivate);

void mt7961SetCTSbyRate(
	struct GLUE_INFO *prGlueInfo,
	struct MSDU_INFO *prMsduInfo,
	void *prTxDesc);
#endif

bool mt7961LiteWfdmaAllocRxRing(
	struct GLUE_INFO *prGlueInfo,
	bool fgAllocMem);

void mt7961Connac2xProcessTxInterrupt(
	struct ADAPTER *prAdapter);

void mt7961Connac2xProcessRxInterrupt(
	struct ADAPTER *prAdapter);

void mt7961WfdmaTxRingExtCtrl(
	struct GLUE_INFO *prGlueInfo,
	struct RTMP_TX_RING *tx_ring,
	uint32_t index);

void mt7961WfdmaRxRingExtCtrl(
	struct GLUE_INFO *prGlueInfo,
	struct RTMP_RX_RING *rx_ring,
	uint32_t index);

void mt7961Connac2xWfdmaManualPrefetch(
	struct GLUE_INFO *prGlueInfo);

void mt7961ReadIntStatus(
	struct ADAPTER *prAdapter,
	uint32_t *pu4IntStatus);

uint8_t mt7961CheckPortForRxEventFromPse(IN struct ADAPTER *prAdapter,
				IN uint8_t u2Port);
#if (CFG_COALESCING_INTERRUPT == 1)
uint32_t mt7961setWfdmaCoalescingInt(
	struct ADAPTER *prAdapter,
	u_int8_t fgEnable);
#endif

#endif /* _HIF_PCIE || _HIF_AXI */

void mt7961DumpSerDummyCR(
	struct ADAPTER *prAdapter);

uint32_t mt7961GetFwVer(struct ADAPTER *prAdapter);

uint32_t mt7961DownloadBtPatch(IN struct ADAPTER *prAdapter);

void mt7961SerInit(IN struct ADAPTER *prAdapter,
		   IN const u_int8_t fgAtResetFlow);

/*******************************************************************************
*                              F U N C T I O N S
********************************************************************************
*/

#endif /* _MT7961_H */

#endif  /* defined(MT7961) || defined(MT7922) || defined(MT7902) */
