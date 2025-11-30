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
/******************************************************************************
 *[File]             dbg_soc5_0.c
 *[Version]          v1.0
 *[Revision Date]    2020-05-22
 *[Author]
 *[Description]
 *    The program provides WIFI FALCON MAC Debug APIs
 *[Copyright]
 *    Copyright (C) 2015 MediaTek Incorporation. All Rights Reserved.
 ******************************************************************************/

#ifdef SOC5_0

#include "precomp.h"
#include "soc5_0.h"
#include "coda/soc5_0/wf_wfdma_host_dma0.h"
#include "coda/soc5_0/wf_hif_dmashdl_top.h"
#include "coda/soc5_0/wf_ple_top.h"
#include "coda/soc5_0/wf_pse_top.h"

/*******************************************************************************
 *                         C O M P I L E R   F L A G S
 *******************************************************************************
 */

/*******************************************************************************
 *                              C O N S T A N T S
 *******************************************************************************
 */

#define WF_WFDMA_MCU_DMA0_WPDMA_TX_RING0_CTRL0_ADDR \
				(CONNAC2X_MCU_WPDMA_0_BASE + 0x300)
#define WF_WFDMA_MCU_DMA0_WPDMA_TX_RING2_CTRL0_ADDR \
				(CONNAC2X_MCU_WPDMA_0_BASE + 0x320)
#define WF_WFDMA_MCU_DMA0_WPDMA_TX_RING3_CTRL0_ADDR \
				(CONNAC2X_MCU_WPDMA_0_BASE + 0x330)

#define WF_WFDMA_MCU_DMA0_WPDMA_RX_RING0_CTRL0_ADDR \
				(CONNAC2X_MCU_WPDMA_0_BASE + 0x500)
#define WF_WFDMA_MCU_DMA0_WPDMA_RX_RING1_CTRL0_ADDR \
				(CONNAC2X_MCU_WPDMA_0_BASE + 0x510)
#define WF_WFDMA_MCU_DMA0_WPDMA_RX_RING2_CTRL0_ADDR \
				(CONNAC2X_MCU_WPDMA_0_BASE + 0x520)
#define WF_WFDMA_MCU_DMA0_WPDMA_RX_RING3_CTRL0_ADDR \
				(CONNAC2X_MCU_WPDMA_0_BASE + 0x530)
#define WF_WFDMA_MCU_DMA0_WPDMA_RX_RING4_CTRL0_ADDR \
				(CONNAC2X_MCU_WPDMA_0_BASE + 0x540)
#define WF_WFDMA_MCU_DMA0_WPDMA_RX_RING5_CTRL0_ADDR \
				(CONNAC2X_MCU_WPDMA_0_BASE + 0x550)

/*******************************************************************************
 *                             D A T A   T Y P E S
 *******************************************************************************
 */
enum _ENUM_WFDMA_TYPE_T {
	WFDMA_TYPE_HOST = 0,
	WFDMA_TYPE_WM
};

struct wfdma_group_info {
	char name[20];
	u_int32_t hw_desc_base;
};

struct wfdma_group_info wfmda_host_tx_group[] = {
	{"P0T0:AP DATA0", WF_WFDMA_HOST_DMA0_WPDMA_TX_RING0_CTRL0_ADDR},
	{"P0T1:AP DATA1", WF_WFDMA_HOST_DMA0_WPDMA_TX_RING1_CTRL0_ADDR},
	{"P0T16:FWDL", WF_WFDMA_HOST_DMA0_WPDMA_TX_RING16_CTRL0_ADDR},
	{"P0T17:AP CMD", WF_WFDMA_HOST_DMA0_WPDMA_TX_RING17_CTRL0_ADDR},
};

struct wfdma_group_info wfmda_host_rx_group[] = {
	{"P0R0:AP EVENT", WF_WFDMA_HOST_DMA0_WPDMA_RX_RING0_CTRL0_ADDR},
	{"P0R2:AP DATA0", WF_WFDMA_HOST_DMA0_WPDMA_RX_RING2_CTRL0_ADDR},
	{"P0R3:AP DATA1", WF_WFDMA_HOST_DMA0_WPDMA_RX_RING3_CTRL0_ADDR},
	{"P0R4:AP TDONE0", WF_WFDMA_HOST_DMA0_WPDMA_RX_RING4_CTRL0_ADDR},
	{"P0R5:AP TDONE1", WF_WFDMA_HOST_DMA0_WPDMA_RX_RING5_CTRL0_ADDR},
};

struct wfdma_group_info wfmda_wm_tx_group[] = {
	{"P0T0:AP EVENT", WF_WFDMA_MCU_DMA0_WPDMA_TX_RING0_CTRL0_ADDR},
	{"P0T2:DATA", WF_WFDMA_MCU_DMA0_WPDMA_TX_RING2_CTRL0_ADDR},
	{"P0T3:SW TX Command", WF_WFDMA_MCU_DMA0_WPDMA_TX_RING3_CTRL0_ADDR},
};

struct wfdma_group_info wfmda_wm_rx_group[] = {
	{"P0R0:FWDL", WF_WFDMA_MCU_DMA0_WPDMA_RX_RING0_CTRL0_ADDR},
	{"P0R1:AP CMD", WF_WFDMA_MCU_DMA0_WPDMA_RX_RING1_CTRL0_ADDR},
	{"P0R3:DATA", WF_WFDMA_MCU_DMA0_WPDMA_RX_RING3_CTRL0_ADDR},
	{"P0R4:TXDONE", WF_WFDMA_MCU_DMA0_WPDMA_RX_RING4_CTRL0_ADDR},
	{"P0R5:RPT", WF_WFDMA_MCU_DMA0_WPDMA_RX_RING5_CTRL0_ADDR},
};

/*******************************************************************************
*                   F U N C T I O N   D E C L A R A T I O N S
********************************************************************************
*/
static void show_wfdma_interrupt_info(IN struct ADAPTER *prAdapter,
		IN enum _ENUM_WFDMA_TYPE_T enum_wfdma_type);

static void show_wfdma_glo_info(IN struct ADAPTER *prAdapter,
		IN enum _ENUM_WFDMA_TYPE_T enum_wfdma_type);

static void show_wfdma_ring_info(IN struct ADAPTER *prAdapter,
		IN enum _ENUM_WFDMA_TYPE_T enum_wfdma_type);

static void show_wfdma_dbg_log(IN struct ADAPTER *prAdapter,
		IN enum _ENUM_WFDMA_TYPE_T enum_wfdma_type);

static void dump_dbg_value(IN struct ADAPTER *prAdapter,
		IN uint32_t pdma_base_cr,
		IN uint32_t set_value);

static void dump_wfdma_dbg_value(IN struct ADAPTER *prAdapter,
		IN enum _ENUM_WFDMA_TYPE_T enum_wfdma_type);

static void show_wfdma_axi_debug_log(IN struct ADAPTER *prAdapter,
		IN enum _ENUM_WFDMA_TYPE_T enum_wfdma_type);

/*******************************************************************************
 *                            P U B L I C   D A T A
 *******************************************************************************
 */

/*******************************************************************************
 *                           P R I V A T E   D A T A
 *******************************************************************************
 */

void soc5_0_show_wfdma_info(IN struct ADAPTER *prAdapter)
{
	if (!prAdapter)
		return;

	show_wfdma_dbg_log(prAdapter, WFDMA_TYPE_HOST);
	show_wfdma_dbg_log(prAdapter, WFDMA_TYPE_WM);

	dump_wfdma_dbg_value(prAdapter, WFDMA_TYPE_HOST);
	dump_wfdma_dbg_value(prAdapter, WFDMA_TYPE_WM);

	show_wfdma_axi_debug_log(prAdapter, WFDMA_TYPE_HOST);
	show_wfdma_axi_debug_log(prAdapter, WFDMA_TYPE_WM);
}

void soc5_0_show_ple_info(struct ADAPTER *prAdapter, u_int8_t fgDumpTxd)
{
	if (!prAdapter)
		return;
}

void soc5_0_show_pse_info(struct ADAPTER *prAdapter)
{
	if (!prAdapter)
		return;
}

bool soc5_0_show_host_csr_info(struct ADAPTER *prAdapter)
{
	if (!prAdapter)
		return false;

	return true;
}

static void show_wfdma_interrupt_info(IN struct ADAPTER *prAdapter,
		IN enum _ENUM_WFDMA_TYPE_T enum_wfdma_type)
{
	uint32_t u4IntStaBaseCrAddr = 0;
	uint32_t u4IntEnaBaseCrAddr = 0;
	uint32_t u4RegValue = 0;

	if (enum_wfdma_type == WFDMA_TYPE_HOST) {
		u4IntStaBaseCrAddr = WF_WFDMA_HOST_DMA0_HOST_INT_STA_ADDR;
		u4IntEnaBaseCrAddr = WF_WFDMA_HOST_DMA0_HOST_INT_ENA_ADDR;
	} else if (enum_wfdma_type == WFDMA_TYPE_WM) {
		u4IntStaBaseCrAddr = CONNAC2X_MCU_WPDMA_0_BASE + 0x200;
		u4IntEnaBaseCrAddr = CONNAC2X_MCU_WPDMA_0_BASE + 0x204;
	}

	HAL_MCR_RD(prAdapter, u4IntStaBaseCrAddr, &u4RegValue);
	DBGLOG(INIT, INFO, "%s_INT_STA(0x%08x):0x%08x\n",
			enum_wfdma_type == WFDMA_TYPE_HOST ? "HOST" : "MCU",
			u4IntStaBaseCrAddr,
			u4RegValue);

	HAL_MCR_RD(prAdapter, u4IntEnaBaseCrAddr, &u4RegValue);
	DBGLOG(INIT, INFO, "%s_INT_ENA(0x%08x):0x%08x\n",
			enum_wfdma_type == WFDMA_TYPE_HOST ? "HOST" : "MCU",
			u4IntEnaBaseCrAddr,
			u4RegValue);
}

static void show_wfdma_glo_info(IN struct ADAPTER *prAdapter,
		IN enum _ENUM_WFDMA_TYPE_T enum_wfdma_type)
{
	uint32_t u4DmaCfgCr = 0;
	uint32_t u4RegValue = 0;

	if (enum_wfdma_type == WFDMA_TYPE_HOST)
		u4DmaCfgCr = CONNAC2X_WPDMA_GLO_CFG(CONNAC2X_HOST_WPDMA_0_BASE);
	else if (enum_wfdma_type == WFDMA_TYPE_WM)
		u4DmaCfgCr = CONNAC2X_WPDMA_GLO_CFG(CONNAC2X_MCU_WPDMA_0_BASE);

	HAL_MCR_RD(prAdapter, u4DmaCfgCr, &u4RegValue);
	DBGLOG(INIT, INFO, "%s_GLO_CONFIG(0x%08x):0x%08x\n",
			enum_wfdma_type == WFDMA_TYPE_HOST ? "HOST" : "MCU",
			u4DmaCfgCr,
			u4RegValue);
}

static void show_wfdma_ring_info(IN struct ADAPTER *prAdapter,
		IN enum _ENUM_WFDMA_TYPE_T enum_wfdma_type)
{
	uint32_t idx;
	uint32_t group_cnt;
	uint32_t u4DmaCfgCrAddr = 0;
	struct wfdma_group_info *group;
	uint32_t u4_hw_desc_base_value = 0;
	uint32_t u4_hw_cnt_value = 0;
	uint32_t u4_hw_cidx_value = 0;
	uint32_t u4_hw_didx_value = 0;
	uint32_t queue_cnt;

	/* Dump All TX Ring Info */
	DBGLOG(HAL, INFO, "----------- TX Ring Config -----------\n");
	DBGLOG(HAL, INFO, "%4s %16s %8s %10s %6s %6s %6s %6s\n",
		"Idx", "Attr", "Reg", "Base", "Cnt", "CIDX", "DIDX", "QCnt");

	/* Dump TX Ring */
	if (enum_wfdma_type == WFDMA_TYPE_HOST)
		group_cnt = sizeof(wfmda_host_tx_group) /
		sizeof(struct wfdma_group_info);
	else
		group_cnt = sizeof(wfmda_wm_tx_group) /
		sizeof(struct wfdma_group_info);

	for (idx = 0; idx < group_cnt; idx++) {
		if (enum_wfdma_type == WFDMA_TYPE_HOST)
			group = &wfmda_host_tx_group[idx];
		else
			group = &wfmda_wm_tx_group[idx];

		u4DmaCfgCrAddr = group->hw_desc_base;

		HAL_MCR_RD(prAdapter, u4DmaCfgCrAddr, &u4_hw_desc_base_value);
		HAL_MCR_RD(prAdapter, u4DmaCfgCrAddr + 0x04, &u4_hw_cnt_value);
		HAL_MCR_RD(prAdapter, u4DmaCfgCrAddr + 0x08, &u4_hw_cidx_value);
		HAL_MCR_RD(prAdapter, u4DmaCfgCrAddr + 0x0c, &u4_hw_didx_value);

		queue_cnt = (u4_hw_cidx_value >= u4_hw_didx_value) ?
			(u4_hw_cidx_value - u4_hw_didx_value) :
			(u4_hw_cidx_value - u4_hw_didx_value + u4_hw_cnt_value);

		DBGLOG(HAL, INFO, "%4d %16s %8x %10x %6x %6x %6x %6x\n",
					idx,
					group->name,
					u4DmaCfgCrAddr, u4_hw_desc_base_value,
					u4_hw_cnt_value, u4_hw_cidx_value,
					u4_hw_didx_value, queue_cnt);

	}

	/* Dump All RX Ring Info */
	DBGLOG(HAL, INFO, "----------- RX Ring Config -----------\n");
	DBGLOG(HAL, INFO, "%4s %16s %8s %10s %6s %6s %6s %6s\n",
		"Idx", "Attr", "Reg", "Base", "Cnt", "CIDX", "DIDX", "QCnt");

	/* Dump RX Ring */
	if (enum_wfdma_type == WFDMA_TYPE_HOST)
		group_cnt = sizeof(wfmda_host_rx_group) /
		sizeof(struct wfdma_group_info);
	else
		group_cnt = sizeof(wfmda_wm_rx_group) /
		sizeof(struct wfdma_group_info);

	for (idx = 0; idx < group_cnt; idx++) {
		if (enum_wfdma_type == WFDMA_TYPE_HOST)
			group = &wfmda_host_rx_group[idx];
		else
			group = &wfmda_wm_rx_group[idx];

		u4DmaCfgCrAddr = group->hw_desc_base;

		HAL_MCR_RD(prAdapter, u4DmaCfgCrAddr, &u4_hw_desc_base_value);
		HAL_MCR_RD(prAdapter, u4DmaCfgCrAddr + 0x04, &u4_hw_cnt_value);
		HAL_MCR_RD(prAdapter, u4DmaCfgCrAddr + 0x08, &u4_hw_cidx_value);
		HAL_MCR_RD(prAdapter, u4DmaCfgCrAddr + 0x0c, &u4_hw_didx_value);

		queue_cnt = (u4_hw_didx_value > u4_hw_cidx_value) ?
			(u4_hw_didx_value - u4_hw_cidx_value - 1) :
			(u4_hw_didx_value - u4_hw_cidx_value
			+ u4_hw_cnt_value - 1);

		DBGLOG(HAL, INFO, "%4d %16s %8x %10x %6x %6x %6x %6x\n",
					idx,
					group->name,
					u4DmaCfgCrAddr, u4_hw_desc_base_value,
					u4_hw_cnt_value, u4_hw_cidx_value,
					u4_hw_didx_value, queue_cnt);
	}
}

static void show_wfdma_dbg_log(IN struct ADAPTER *prAdapter,
		IN enum _ENUM_WFDMA_TYPE_T enum_wfdma_type)
{
	if (!prAdapter)
		return;

	show_wfdma_interrupt_info(prAdapter, enum_wfdma_type);
	show_wfdma_glo_info(prAdapter, enum_wfdma_type);
	show_wfdma_ring_info(prAdapter, enum_wfdma_type);
}

static void dump_dbg_value(IN struct ADAPTER *prAdapter,
		IN uint32_t pdma_base_cr,
		IN uint32_t set_value)
{
	uint32_t set_debug_cr, get_debug_cr;
	uint32_t get_debug_value = 0;

	set_debug_cr = pdma_base_cr + 0x124;
	get_debug_cr = pdma_base_cr + 0x128;

	HAL_MCR_WR(prAdapter, set_debug_cr, set_value);
	HAL_MCR_RD(prAdapter, get_debug_cr, &get_debug_value);

	DBGLOG(INIT, INFO, "set(0x%08x):0x%08x, get(0x%08x):0x%08x\n",
			set_debug_cr, set_value,
			get_debug_cr, get_debug_value);
}

static void dump_wfdma_dbg_value(IN struct ADAPTER *prAdapter,
		IN enum _ENUM_WFDMA_TYPE_T enum_wfdma_type)
{
	uint32_t pdma_base_cr;
	uint32_t i = 0;

	if (enum_wfdma_type == WFDMA_TYPE_HOST)
		pdma_base_cr = CONNAC2X_HOST_WPDMA_0_BASE;
	else
		pdma_base_cr = CONNAC2X_MCU_WPDMA_0_BASE;

	for (i = 0x100; i <= 0x154; i++)
		dump_dbg_value(prAdapter, pdma_base_cr, i);
}

static void show_wfdma_axi_debug_log(IN struct ADAPTER *prAdapter,
		IN enum _ENUM_WFDMA_TYPE_T enum_wfdma_type)
{
	uint32_t pdma_base_cr;
	uint32_t i = 0;

	if (enum_wfdma_type == WFDMA_TYPE_HOST)
		pdma_base_cr = CONNAC2X_HOST_EXT_CONN_HIF_WRAP;
	else
		pdma_base_cr = CONNAC2X_MCU_INT_CONN_HIF_WRAP;

	for (i = 0; i < 13; i++) {
		uint32_t target_cr = pdma_base_cr + 0x500 + (i * 4);
		uint32_t u4RegValue = 0;

		HAL_MCR_RD(prAdapter, target_cr, &u4RegValue);
		DBGLOG(INIT, INFO, "get(0x%08x):0x%08x\n",
			target_cr,
			u4RegValue);
	}
}

#endif /* SOC5_0 */
