/* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause */
/*
 * Copyright (c) 2016 MediaTek Inc.
 */
/*! \file   mt7933.c
*    \brief  Internal driver stack will export
*    the required procedures here for GLUE Layer.
*
*    This file contains all routines which are exported
     from MediaTek 802.11 Wireless LAN driver stack to GLUE Layer.
*/

#ifdef MT7933

/*******************************************************************************
*                         C O M P I L E R   F L A G S
********************************************************************************
*/

/*******************************************************************************
*                    E X T E R N A L   R E F E R E N C E S
********************************************************************************
*/

#include "coda/mt7933/wf_wfdma_host_dma0.h"
#include "coda/mt7933/wf_wfdma_ext_wrap_csr.h"

#if (CFG_DOWNLOAD_DYN_MEMORY_MAP == 1)
#include "coda/mt7933/conn_infra_cfg.h"
#endif

#include "precomp.h"
#include "mt7933.h"
#include <linux/platform_device.h>
/*******************************************************************************
*                              C O N S T A N T S
********************************************************************************
*/

#define CONN_MCU_CONFG_BASE 0x88000000
#define CONN_MCU_CONFG_COM_REG0_ADDR (CONN_MCU_CONFG_BASE + 0x200)

#define PATCH_SEMAPHORE_COMM_REG 0
#define PATCH_SEMAPHORE_COMM_REG_PATCH_DONE 1 /* bit0 is for patch. */

#define SW_WORKAROUND_FOR_WFDMA_ISSUE_HWITS00009838 0 /* Buzzard Flag */
#define SW_WORKAROUND_FOR_WFDMA_ISSUE_HWITS00015048 0 /* Buzzard Flag */

#define RX_DATA_RING_BASE_IDX 0 /* mockingbird data ring is at RX RING 0 */
#define RX_EVT_RING_BASE_IDX 1 /* mockingbird data ring is at RX RING 0 */

#define MT7933_FWDL_TX_RING_IDX         3
#define MT7933_CMD_TX_RING_IDX          2 /* Direct to WM */
#define MT7933_DATA0_TXD_IDX 0 /* Band_0 TXD to WA, modified from 7915 */

/*******************************************************************************
*                                 M A C R O S
********************************************************************************
*/
#define MT7933_BUS2CHIP_MEM_MAP_TBLE_SIZE    \
	(sizeof(mt7933_bus2chip_cr_mapping) /	\
	 sizeof(mt7933_bus2chip_cr_mapping[0]))

/*******************************************************************************
*                   F U N C T I O N   D E C L A R A T I O N S
********************************************************************************
*/

/*******************************************************************************
*                              F U N C T I O N S
********************************************************************************
*/

/*******************************************************************************
*                            P U B L I C   D A T A
********************************************************************************
*/

struct ECO_INFO mt7933_eco_table[] = {
	/* HW version,  ROM version,    Factory version */
	{0x00, 0x00, 0xA, 0x1},	/* E1 */
	{0x00, 0x00, 0x0, 0x0}	/* End of table */
};

#if defined(_HIF_PCIE) || defined(_HIF_AXI)
struct PCIE_CHIP_CR_MAPPING mt7933_bus2chip_cr_mapping[] = {
	/* chip addr, bus addr, range */
	{0x54000000, 0x02000, 0x1000},  /* WFDMA PCIE0 MCU DMA0 */
	{0x55000000, 0x03000, 0x1000},  /* WFDMA PCIE0 MCU DMA1 */
	{0x56000000, 0x04000, 0x1000},  /* WFDMA reserved */
	{0x57000000, 0x05000, 0x1000},  /* WFDMA MCU wrap CR */
	{0x58000000, 0x06000, 0x1000},  /* WFDMA PCIE1 MCU DMA0 (MEM_DMA) */
	{0x59000000, 0x07000, 0x1000},  /* WFDMA PCIE1 MCU DMA1 */
	{0x820c0000, 0x08000, 0x4000},  /* WF_UMAC_TOP (PLE) */
	{0x820c8000, 0x0c000, 0x2000},  /* WF_UMAC_TOP (PSE) */
	{0x820cc000, 0x0e000, 0x2000},  /* WF_UMAC_TOP (PP) */
	{0x820e0000, 0x20000, 0x0400},  /* WF_LMAC_TOP BN0 (WF_CFG) */
	{0x820e1000, 0x20400, 0x0200},  /* WF_LMAC_TOP BN0 (WF_TRB) */
	{0x820e2000, 0x20800, 0x0400},  /* WF_LMAC_TOP BN0 (WF_AGG) */
	{0x820e3000, 0x20c00, 0x0400},  /* WF_LMAC_TOP BN0 (WF_ARB) */
	{0x820e4000, 0x21000, 0x0400},  /* WF_LMAC_TOP BN0 (WF_TMAC) */
	{0x820e5000, 0x21400, 0x0800},  /* WF_LMAC_TOP BN0 (WF_RMAC) */
	{0x820ce000, 0x21c00, 0x0200},  /* WF_LMAC_TOP (WF_SEC) */
	{0x820e7000, 0x21e00, 0x0200},  /* WF_LMAC_TOP BN0 (WF_DMA) */
	{0x820cf000, 0x22000, 0x1000},  /* WF_LMAC_TOP (WF_PF) */
	{0x820e9000, 0x23400, 0x0200},  /* WF_LMAC_TOP BN0 (WF_WTBLOFF) */
	{0x820ea000, 0x24000, 0x0200},  /* WF_LMAC_TOP BN0 (WF_ETBF) */
	{0x820eb000, 0x24200, 0x0400},  /* WF_LMAC_TOP BN0 (WF_LPON) */
	{0x820ec000, 0x24600, 0x0200},  /* WF_LMAC_TOP BN0 (WF_INT) */
	{0x820ed000, 0x24800, 0x0800},  /* WF_LMAC_TOP BN0 (WF_MIB) */
	{0x820ca000, 0x26000, 0x2000},  /* WF_LMAC_TOP BN0 (WF_MUCOP) */
	{0x820d0000, 0x30000, 0x10000}, /* WF_LMAC_TOP (WF_WTBLON) */
	{0x40000000, 0x70000, 0x10000}, /* WF_UMAC_SYSRAM */
	{0x00400000, 0x80000, 0x10000}, /* WF_MCU_SYSRAM */
	{0x00410000, 0x90000, 0x10000}, /* WF_MCU_SYSRAM (configure register) */
	{0x820b0000, 0xae000, 0x1000},  /* [APB2] WFSYS_ON */
	{0x80020000, 0xb0000, 0x10000}, /* WF_TOP_MISC_OFF */
	{0x81020000, 0xc0000, 0x10000}, /* WF_TOP_MISC_ON */
	{0x7c020000, 0xd0000, 0x10000}, /* CONN_INFRA, wfdma */
	{0x7c060000, 0xe0000, 0x10000}, /* CONN_INFRA, conn_host_csr_top */
	{0x7c000000, 0xf0000, 0x10000}, /* CONN_INFRA */
};

static void mt7933HalPrintHifDbgInfo(
	struct ADAPTER *prAdapter)
{

}
void mt7933_icapRiseVcoreClockRate(void)
{
	DBGLOG(HAL, INFO, "%s need to implement!\n", __func__);
}

void mt7933_icapDownVcoreClockRate(void)
{
	DBGLOG(HAL, INFO, "%s need to implement!\n", __func__);
}

static void mt7933EnableInterrupt(
	struct ADAPTER *prAdapter)
{
	struct mt66xx_chip_info *prChipInfo;
	union WPDMA_INT_MASK IntMask;
	uint32_t u4HostWpdamBase = 0;

	prChipInfo = prAdapter->chip_info;

	if (prChipInfo->is_support_wfdma1)
		u4HostWpdamBase = CONNAC2X_HOST_WPDMA_1_BASE;
	else
		u4HostWpdamBase = CONNAC2X_HOST_WPDMA_0_BASE;

	prAdapter->fgIsIntEnable = TRUE;

	HAL_MCR_RD(prAdapter,
		WF_WFDMA_HOST_DMA0_HOST_INT_ENA_ADDR, &IntMask.word);
	IntMask.word = 0;
	IntMask.field_conn2x_single.wfdma0_rx_done_0 = 1;
	IntMask.field_conn2x_single.wfdma0_rx_done_1 = 1;
	IntMask.field_conn2x_single.wfdma0_tx_done_0 = 1;
	IntMask.field_conn2x_single.wfdma0_tx_done_2 = 1;
	IntMask.field_conn2x_single.wfdma0_tx_done_3 = 1;
	IntMask.field_conn2x_single.wfdma0_mcu2host_sw_int_en = 1;

	IntMask.field_conn2x_single.wfdma0_rx_coherent = 0;
	IntMask.field_conn2x_single.wfdma0_tx_coherent = 0;
	HAL_MCR_WR(prAdapter,
		WF_WFDMA_HOST_DMA0_HOST_INT_ENA_ADDR, IntMask.word);

	if (prChipInfo->is_support_asic_lp)
		HAL_MCR_WR_FIELD(prAdapter,
				 CONNAC2X_WPDMA_MCU2HOST_SW_INT_MASK
				 (u4HostWpdamBase),
				 BITS(0, 15),
				 0,
				 BITS(0, 15));
} /* end of nicEnableInterrupt() */

static void mt7933DisableInterrupt(
	struct ADAPTER *prAdapter)
{
	struct GLUE_INFO *prGlueInfo = NULL;
	union WPDMA_INT_MASK IntMask;

	ASSERT(prAdapter);

	prGlueInfo = prAdapter->prGlueInfo;

	IntMask.word = 0;

	HAL_MCR_WR(prAdapter,
		WF_WFDMA_HOST_DMA0_HOST_INT_ENA_ADDR, IntMask.word);
	HAL_MCR_RD(prAdapter,
		WF_WFDMA_HOST_DMA0_HOST_INT_ENA_ADDR, &IntMask.word);

	prAdapter->fgIsIntEnable = FALSE;
}

/* as long as the hw addr for each ring is fixed
 * supposed the setting should be fine for
 * ring# & address pair
 */
static uint8_t mt7933SetRxRingHwAddr(
	struct RTMP_RX_RING *prRxRing,
	struct BUS_INFO *prBusInfo,
	uint32_t u4SwRingIdx)
{
	uint32_t offset = 0;

	/*
	 * defined in hif_pdma.h enumeration
	 * RX_RING_DATA_IDX_0   (RX_Ring0) - Band0 Rx Data
	 * RX_RING_EVT_IDX_1    (RX_Ring0) - Rx Event
	*/
	switch (u4SwRingIdx) {
	case RX_RING_DATA_IDX_0:
		/*
		 * in buzzard, RX Ring 2 is data ring, so need offset
		 * in mockingbird, Rx Ring 0 is data ring
		 */
		/* offset = RX_DATA_RING_BASE_IDX * MT_RINGREG_DIFF; */
		break;
	case RX_RING_EVT_IDX_1:
		offset = RX_EVT_RING_BASE_IDX * MT_RINGREG_DIFF;
		break;
	default:
		/*
		 * Can mockingbird just skip this and make all other stuff as
		 * RX ring 1, because all of the rest operation should
		 * not be data
		 */
		DBGLOG(HAL, INFO, "%s config ring [%d] failed\n",
			__func__, u4SwRingIdx);
		return FALSE;
	}

	prRxRing->hw_desc_base = prBusInfo->host_rx_ring_base + offset;
	prRxRing->hw_cidx_addr = prBusInfo->host_rx_ring_cidx_addr + offset;
	prRxRing->hw_didx_addr = prBusInfo->host_rx_ring_didx_addr + offset;
	prRxRing->hw_cnt_addr = prBusInfo->host_rx_ring_cnt_addr + offset;

	return TRUE;
}

static void wlanBuzzardInitPcieInt(
	struct GLUE_INFO *prGlueInfo)
{

	DBGLOG(HAL, ERROR, "%s Buzzard change do nothing\n", __func__);
#if 0
       /* PCIE interrupt DMA end enable  */
	HAL_MCR_WR(prGlueInfo->prAdapter,
		0x10188,
		0x000000FF);
#endif
}

/* change from Buzzard
 * from caller halWpdmaAllocRing there are data0 & EVT ring
 * just alloc different ring allocation here
 */
#if 0
static bool mt7933LiteWfdmaAllocRxRing(
	struct GLUE_INFO *prGlueInfo,
	bool fgAllocMem)
{
#if 0
	/* Band1 Data Rx path */
	if (!halWpdmaAllocRxRing(prGlueInfo,
			WFDMA0_RX_RING_IDX_2, RX_RING0_SIZE,
			RXD_SIZE, CFG_RX_MAX_PKT_SIZE, fgAllocMem)) {
		DBGLOG(HAL, ERROR, "AllocRxRing[0] fail\n");
		return false;
	}
	/* Band0 Tx Free Done Event */
	if (!halWpdmaAllocRxRing(prGlueInfo,
			WFDMA0_RX_RING_IDX_3, RX_RING1_SIZE,
			RXD_SIZE, RX_BUFFER_AGGRESIZE, fgAllocMem)) {
		DBGLOG(HAL, ERROR, "AllocRxRing[1] fail\n");
		return false;
	}
	/* Band1 Tx Free Done Event */
	if (!halWpdmaAllocRxRing(prGlueInfo,
			WFDMA1_RX_RING_IDX_0, RX_RING1_SIZE,
			RXD_SIZE, RX_BUFFER_AGGRESIZE, fgAllocMem)) {
		DBGLOG(HAL, ERROR, "AllocRxRing[1] fail\n");
		return false;
	}
#endif
	return true;
}
#endif

static void mt7933Connac2xProcessTxInterrupt(
	struct ADAPTER *prAdapter)
{
	struct GL_HIF_INFO *prHifInfo = &prAdapter->prGlueInfo->rHifInfo;
	union WPDMA_INT_STA_STRUCT rIntrStatus;

	/*
	 * enum ENUM_TX_RING_IDX {
	 * TX_RING_DATA0_IDX_0 = 0,
	 * TX_RING_DATA1_IDX_1,
	 * TX_RING_CMD_IDX_2,
	 * TX_RING_FWDL_IDX_3,
	 * TX_RING_WA_CMD_IDX_4,
	 */
	rIntrStatus = (union WPDMA_INT_STA_STRUCT)prHifInfo->u4IntStatus;
	/*
	 * after FWDL tx_done_3 become TXD offloading
	 */
	if (rIntrStatus.field_conn2x_single.wfdma0_tx_done_3)
		halWpdmaProcessCmdDmaDone(
			prAdapter->prGlueInfo, TX_RING_FWDL_IDX_3);

	if (rIntrStatus.field_conn2x_single.wfdma0_tx_done_2)
		halWpdmaProcessCmdDmaDone(
			prAdapter->prGlueInfo, TX_RING_CMD_IDX_2);

	if (rIntrStatus.field_conn2x_single.wfdma0_tx_done_0) {
		halWpdmaProcessDataDmaDone(
			prAdapter->prGlueInfo, TX_RING_DATA0_IDX_0);
#if CFG_SUPPORT_MULTITHREAD
		if (!HAL_IS_TX_DIRECT())
			kalSetTxEvent2Hif(prAdapter->prGlueInfo);
#endif
	}
}

static void mt7933Connac2xProcessRxInterrupt(
	struct ADAPTER *prAdapter)
{
	struct GL_HIF_INFO *prHifInfo = &prAdapter->prGlueInfo->rHifInfo;
	union WPDMA_INT_STA_STRUCT rIntrStatus;

	/*
	 * enum ENUM_RX_RING_IDX {
	 * RX_RING_DATA_IDX_0 = 0,  Rx Data
	 * RX_RING_EVT_IDX_1,
	 * WFDMA0_RX_RING_IDX_2, Band0 TxFreeDoneEvent
	 * WFDMA0_RX_RING_IDX_3,  Band1 TxFreeDoneEvent
	 * WFDMA1_RX_RING_IDX_0,  WM Event
	 */
	/* may have problem, buzzard use kalDevReadData
	 * for TxFreeDoneEvent, and kalDevPortRead for Event Packet.
	 * BUT in mockingbird, its the same Ring.
	 * may have problem here?
	 */
	rIntrStatus = (union WPDMA_INT_STA_STRUCT)prHifInfo->u4IntStatus;
	if (rIntrStatus.field_conn2x_single.wfdma0_rx_done_0)
		halRxReceiveRFBs(prAdapter, RX_RING_DATA_IDX_0, TRUE);

	if (rIntrStatus.field_conn2x_single.wfdma0_rx_done_1)
		halRxReceiveRFBs(prAdapter, RX_RING_EVT_IDX_1, FALSE);

}

static void mt7933ReadIntStatus(
	struct ADAPTER *prAdapter,
	uint32_t *pu4IntStatus)
{
	uint32_t u4RegValue;
	struct GL_HIF_INFO *prHifInfo = &prAdapter->prGlueInfo->rHifInfo;
	struct BUS_INFO *prBusInfo = prAdapter->chip_info->bus_info;

	*pu4IntStatus = 0;

	HAL_MCR_RD(prAdapter,
		WF_WFDMA_HOST_DMA0_HOST_INT_STA_ADDR, &u4RegValue);

	if (HAL_IS_CONNAC2X_EXT_RX_DONE_INTR(u4RegValue,
				       prBusInfo->host_int_rxdone_bits))
	*pu4IntStatus |= WHISR_RX0_DONE_INT;

	if (HAL_IS_CONNAC2X_EXT_TX_DONE_INTR(u4RegValue,
				       prBusInfo->host_int_txdone_bits))
	*pu4IntStatus |= WHISR_TX_DONE_INT;

	if (u4RegValue & CONNAC_MCU_SW_INT)
	*pu4IntStatus |= WHISR_D2H_SW_INT;

	prHifInfo->u4IntStatus = u4RegValue;

	/* clear interrupt */
	HAL_MCR_WR(prAdapter,
		WF_WFDMA_HOST_DMA0_HOST_INT_STA_ADDR, u4RegValue);

	/* Buzzard HWITS */
#if (SW_WORKAROUND_FOR_WFDMA_ISSUE_HWITS00015048 == 1)
	if (u4RegValue & CONNAC2X_WFDMA_RX_DONE_INT5)
	HAL_MCR_WR(prAdapter,
		WF_WFDMA_HOST_DMA0_HOST_INT_STA_EXT_ADDR,
		(1 <<
		WF_WFDMA_HOST_DMA0_HOST_INT_STA_EXT_rx_done_int_sts_5_SHFT));
#endif
}

#endif /*_HIF_PCIE || _HIF_AXI */

#if 0 /* TODO */
void mt7961DumpSerDummyCR(
	struct ADAPTER *prAdapter)
{
	uint32_t u4MacVal;

	DBGLOG(HAL, INFO, "%s\n", __func__);

	DBGLOG(HAL, INFO, "=====Dump Start====\n");

	HAL_MCR_RD(prAdapter, WF_SW_DEF_CR_SER_STATUS_ADDR, &u4MacVal);
	DBGLOG(HAL, INFO, "SER STATUS[0x%08x]: 0x%08x\n",
		WF_SW_DEF_CR_SER_STATUS_ADDR, u4MacVal);

	HAL_MCR_RD(prAdapter, WF_SW_DEF_CR_PLE_STATUS_ADDR, &u4MacVal);
	DBGLOG(HAL, INFO, "PLE STATUS[0x%08x]: 0x%08x\n",
		WF_SW_DEF_CR_PLE_STATUS_ADDR, u4MacVal);

	HAL_MCR_RD(prAdapter, WF_SW_DEF_CR_PLE1_STATUS_ADDR, &u4MacVal);
	DBGLOG(HAL, INFO, "PLE1 STATUS[0x%08x]: 0x%08x\n",
		WF_SW_DEF_CR_PLE1_STATUS_ADDR, u4MacVal);

	HAL_MCR_RD(prAdapter, WF_SW_DEF_CR_PLE_AMSDU_STATUS_ADDR, &u4MacVal);
	DBGLOG(HAL, INFO, "PLE AMSDU STATUS[0x%08x]: 0x%08x\n",
		WF_SW_DEF_CR_PLE_AMSDU_STATUS_ADDR, u4MacVal);

	HAL_MCR_RD(prAdapter, WF_SW_DEF_CR_PSE_STATUS_ADDR, &u4MacVal);
	DBGLOG(HAL, INFO, "PSE STATUS[0x%08x]: 0x%08x\n",
		WF_SW_DEF_CR_PSE_STATUS_ADDR, u4MacVal);

	HAL_MCR_RD(prAdapter, WF_SW_DEF_CR_PSE1_STATUS_ADDR, &u4MacVal);
	DBGLOG(HAL, INFO, "PSE1 STATUS[0x%08x]: 0x%08x\n",
		WF_SW_DEF_CR_PSE1_STATUS_ADDR, u4MacVal);

	HAL_MCR_RD(prAdapter, WF_SW_DEF_CR_LAMC_WISR6_BN0_STATUS_ADDR,
			&u4MacVal);
	DBGLOG(HAL, INFO, "LMAC WISR6 BN0 STATUS[0x%08x]: 0x%08x\n",
		WF_SW_DEF_CR_LAMC_WISR6_BN0_STATUS_ADDR, u4MacVal);

	HAL_MCR_RD(prAdapter, WF_SW_DEF_CR_LAMC_WISR6_BN1_STATUS_ADDR,
			&u4MacVal);
	DBGLOG(HAL, INFO, "LMAC WISR6 BN1 STATUS[0x%08x]: 0x%08x\n",
		WF_SW_DEF_CR_LAMC_WISR6_BN1_STATUS_ADDR, u4MacVal);

	HAL_MCR_RD(prAdapter, WF_SW_DEF_CR_LAMC_WISR7_BN0_STATUS_ADDR,
			&u4MacVal);
	DBGLOG(HAL, INFO, "LMAC WISR7 BN0 STATUS[0x%08x]: 0x%08x\n",
		WF_SW_DEF_CR_LAMC_WISR7_BN0_STATUS_ADDR, u4MacVal);

	HAL_MCR_RD(prAdapter, WF_SW_DEF_CR_LAMC_WISR7_BN1_STATUS_ADDR,
			&u4MacVal);
	DBGLOG(HAL, INFO, "LMAC WISR7 BN1 STATUS[0x%08x]: 0x%08x\n",
		WF_SW_DEF_CR_LAMC_WISR7_BN1_STATUS_ADDR, u4MacVal);

	DBGLOG(HAL, INFO, "=====Dump End====\n");

}

uint32_t mt7961GetFlavorVer(
	struct ADAPTER *prAdapter)
{
	uint32_t u4Val;
	uint32_t flavor_ver;

	if (wlanAccessRegister(prAdapter, MT7961_A_DIE_VER_ADDR, &u4Val, 0, 0)
	    != WLAN_STATUS_SUCCESS) {
		DBGLOG(INIT, ERROR, "get Bounding Info failed. set as 0x1\n");
		flavor_ver = 0x1;
	} else {
		DBGLOG(INIT, TRACE, "get Bounding Info = 0x%x\n", u4Val);

		if ((u4Val & MT7961_A_DIE_VER_BIT) == MT7961_A_DIE_7920)
			flavor_ver = 0x1a;
		else
			flavor_ver = 0x1;
	}

	return flavor_ver;
}
#endif

void mt7933ConstructFirmwarePrio(struct GLUE_INFO *prGlueInfo,
	uint8_t **apucNameTable, uint8_t **apucName,
	uint8_t *pucNameIdx, uint8_t ucMaxNameIdx)
{
	uint8_t sub_idx = 0;
	uint32_t chip_id = 0;
	struct ADAPTER *prAdapter = NULL;
	struct mt66xx_chip_info *prChipInfo = NULL;

	if (prGlueInfo == NULL) {
		DBGLOG(INIT, ERROR, "prGlueInfo is NULL.\n");
		return;
	}

	prAdapter = prGlueInfo->prAdapter;
	if (prAdapter == NULL) {
		DBGLOG(INIT, ERROR, "prAdapter is NULL.\n");
		return;
	}

	prChipInfo = prAdapter->chip_info;
	if (prChipInfo == NULL) {
		DBGLOG(INIT, ERROR, "prChipInfo is NULL.\n");
		return;
	}

	chip_id = prChipInfo->chip_id;

	for (sub_idx = 0; apucNameTable[sub_idx]; sub_idx++) {
		if (((*pucNameIdx) + 3) < ucMaxNameIdx) {
#if 0
			/* Type 1. WIFI_RAM_CODE_MTxxxx_x.bin */
			snprintf(*(apucName + (*pucNameIdx)),
				CFG_FW_NAME_MAX_LEN, "%s%x_%x.bin",
				apucNameTable[sub_idx], chip_id, flavor_ver);
			(*pucNameIdx) += 1;

			/* Type 2. WIFI_RAM_CODE_MTxxxx_x */
			snprintf(*(apucName + (*pucNameIdx)),
				CFG_FW_NAME_MAX_LEN, "%s%x_%x",
				apucNameTable[sub_idx], chip_id, flavor_ver);
			(*pucNameIdx) += 1;

#endif
			/* Type 3. WIFI_RAM_CODE_MTxxxx.bin */
			snprintf(*(apucName + (*pucNameIdx)),
					CFG_FW_NAME_MAX_LEN, "%s%x.bin",
					apucNameTable[sub_idx], chip_id);
			(*pucNameIdx) += 1;

			/* Type 4. WIFI_RAM_CODE_MTxxxx */
			snprintf(*(apucName + (*pucNameIdx)),
				CFG_FW_NAME_MAX_LEN, "%s%x",
					apucNameTable[sub_idx], chip_id);
			(*pucNameIdx) += 1;
		} else {
			/* the table is not large enough */
			DBGLOG(INIT, ERROR,
				"kalFirmwareImageMapping >> file name array is not enough.\n");
			ASSERT(0);
		}
	}
}

void mt7933ConstructPatchName(struct GLUE_INFO *prGlueInfo,
	uint8_t **apucName, uint8_t *pucNameIdx)
{
	struct ADAPTER *prAdapter = NULL;
	struct mt66xx_chip_info *prChipInfo = NULL;

	if (prGlueInfo == NULL) {
		DBGLOG(INIT, ERROR, "prGlueInfo is NULL.\n");
		return;
	}

	prAdapter = prGlueInfo->prAdapter;
	if (prAdapter == NULL) {
		DBGLOG(INIT, ERROR, "prAdapter is NULL.\n");
		return;
	}

	prChipInfo = prAdapter->chip_info;
	if (prChipInfo == NULL) {
		DBGLOG(INIT, ERROR, "prChipInfo is NULL.\n");
		return;
	}

	snprintf(apucName[(*pucNameIdx)],
		CFG_FW_NAME_MAX_LEN, "mt%x_patch_e%x_hdr.bin",
		prChipInfo->chip_id,
		wlanGetEcoVersion(prGlueInfo->prAdapter));
}

uint32_t mt7933WlanDownloadSection(IN struct ADAPTER *prAdapter,
			     IN uint32_t u4Addr, IN uint32_t u4Len,
			     IN uint32_t u4DataMode, IN uint8_t *pucStartPtr,
			     IN enum ENUM_IMG_DL_IDX_T eDlIdx)
{
	uint32_t u4ImgSecSize, u4Offset;
	uint8_t *pucSecBuf;
	uint32_t u4Val;

	if (wlanImageSectionConfig(prAdapter, u4Addr, u4Len,
				   u4DataMode, eDlIdx) != WLAN_STATUS_SUCCESS) {
		DBGLOG(INIT, ERROR,
		       "Firmware download configuration failed!\n");
		return WLAN_STATUS_FAILURE;
	}

	/*MT7933 temp patch, IP change */
	kalDevRegRead(prAdapter->prGlueInfo,
		WF_WFDMA_HOST_DMA0_PDA_CONFG_PDA_FWDL_EN_ADDR, &u4Val);
	u4Val |= BIT(31);
	kalDevRegWrite(prAdapter->prGlueInfo,
		WF_WFDMA_HOST_DMA0_PDA_CONFG_PDA_FWDL_EN_ADDR, u4Val);


	for (u4Offset = 0; u4Offset < u4Len;
	     u4Offset += CMD_PKT_SIZE_FOR_IMAGE) {
		if (u4Offset + CMD_PKT_SIZE_FOR_IMAGE < u4Len)
			u4ImgSecSize = CMD_PKT_SIZE_FOR_IMAGE;
		else
			u4ImgSecSize = u4Len - u4Offset;

		pucSecBuf = (uint8_t *) pucStartPtr + u4Offset;
		if (wlanImageSectionDownload(prAdapter, u4ImgSecSize,
					     pucSecBuf) !=
					     WLAN_STATUS_SUCCESS) {
			DBGLOG(INIT, ERROR,
				"Firmware scatter download failed!\n");
			return WLAN_STATUS_FAILURE;
		}
	}
	return WLAN_STATUS_SUCCESS;
}

uint32_t mt7933WlanDownloadSectionV2(IN struct ADAPTER *prAdapter,
		IN uint32_t u4DataMode,
		IN enum ENUM_IMG_DL_IDX_T eDlIdx,
		struct patch_dl_target *target)
{
	uint32_t u4ImgSecSize, u4Offset;
	uint8_t *pucSecBuf;
	uint32_t num_of_region, i;
	uint32_t u4Status = WLAN_STATUS_SUCCESS;
	uint32_t u4Val;

	num_of_region = target->num_of_region;
	if (num_of_region < 0) {
		DBGLOG(INIT, ERROR,
			"Firmware download num_of_region < 0 !\n");
		u4Status = WLAN_STATUS_FAILURE;
		goto out;
	}


	for (i = 0; i < num_of_region; i++) {
		struct patch_dl_buf *region;

		region = &target->patch_region[i];
		if (region->img_ptr == NULL)
			continue;

		/* 2. config PDA */
		if (wlanImageSectionConfig(prAdapter, region->img_dest_addr,
			region->img_size, u4DataMode, eDlIdx) !=
			WLAN_STATUS_SUCCESS) {
			DBGLOG(INIT, ERROR,
				"Firmware download configuration failed!\n");
			u4Status = WLAN_STATUS_FAILURE;
			goto out;
		}

		/*MT7933 temp patch, IP change */

		kalDevRegRead(prAdapter->prGlueInfo,
			WF_WFDMA_HOST_DMA0_PDA_CONFG_PDA_FWDL_EN_ADDR, &u4Val);
		u4Val |= BIT(31);
		kalDevRegWrite(prAdapter->prGlueInfo,
			WF_WFDMA_HOST_DMA0_PDA_CONFG_PDA_FWDL_EN_ADDR, u4Val);

		/* 3. image scatter */
		for (u4Offset = 0; u4Offset < region->img_size;
			u4Offset += CMD_PKT_SIZE_FOR_IMAGE) {
			if (u4Offset + CMD_PKT_SIZE_FOR_IMAGE <
				region->img_size)
				u4ImgSecSize = CMD_PKT_SIZE_FOR_IMAGE;
			else
				u4ImgSecSize = region->img_size - u4Offset;

			pucSecBuf = (uint8_t *) region->img_ptr + u4Offset;
			if (wlanImageSectionDownload(prAdapter, u4ImgSecSize,
					pucSecBuf) !=
					WLAN_STATUS_SUCCESS) {
				DBGLOG(INIT, ERROR,
					"Firmware scatter download failed!\n");

				u4Status = WLAN_STATUS_FAILURE;
				goto out;
			}
		}

		/* MT7933 temp patch, IP change */
		/* wiat PDA idle then stop PDA */
		do {
			kalDevRegRead(prAdapter->prGlueInfo,
			WF_WFDMA_HOST_DMA0_PDA_DWLD_STATE_ADDR, &u4Val);
			if (u4Val &
			WF_WFDMA_HOST_DMA0_PDA_DWLD_STATE_PDA_FWDL_FINISH_MASK)
				break;
			kalUdelay(1000);
		} while (1);

		kalDevRegRead(prAdapter->prGlueInfo,
		WF_WFDMA_HOST_DMA0_PDA_CONFG_PDA_FWDL_EN_ADDR, &u4Val);
		u4Val &= ~BIT(31);
		kalDevRegWrite(prAdapter->prGlueInfo,
		WF_WFDMA_HOST_DMA0_PDA_CONFG_PDA_FWDL_EN_ADDR, u4Val);

	}

out:
	return u4Status;
}

struct BUS_INFO mt7933_bus_info = {
#if defined(_HIF_PCIE) || defined(_HIF_AXI)
	.top_cfg_base = MT7933_TOP_CFG_BASE,

	/* host_dma0 for TXP */
	.host_dma0_base = WF_WFDMA_HOST_DMA0_BASE,
	.host_int_status_addr = WF_WFDMA_HOST_DMA0_HOST_INT_STA_ADDR,

	.host_int_txdone_bits =
		(CONNAC2X_WFDMA_TX_DONE_INT0 |
		CONNAC2X_WFDMA_TX_DONE_INT2 | CONNAC2X_WFDMA_TX_DONE_INT3),
	.host_int_rxdone_bits =
		(CONNAC2X_WFDMA_RX_DONE_INT0 | CONNAC2X_WFDMA_RX_DONE_INT1),

	.host_tx_ring_base = WF_WFDMA_HOST_DMA0_WPDMA_TX_RING0_CTRL0_ADDR,
#if 0 /* no control register here */
	.host_tx_ring_ext_ctrl_base =
		WF_WFDMA_HOST_DMA0_WPDMA_TX_RING0_EXT_CTRL_ADDR,
#endif
	.host_tx_ring_cidx_addr = WF_WFDMA_HOST_DMA0_WPDMA_TX_RING0_CTRL2_ADDR,
	.host_tx_ring_didx_addr = WF_WFDMA_HOST_DMA0_WPDMA_TX_RING0_CTRL3_ADDR,
	.host_tx_ring_cnt_addr = WF_WFDMA_HOST_DMA0_WPDMA_TX_RING0_CTRL1_ADDR,

	.host_rx_ring_base = WF_WFDMA_HOST_DMA0_WPDMA_RX_RING0_CTRL0_ADDR,
#if 0 /* no control register here */
	.host_rx_ring_ext_ctrl_base =
		WF_WFDMA_HOST_DMA0_WPDMA_TX_RING0_EXT_CTRL_ADDR,
#endif
	.host_rx_ring_cidx_addr = WF_WFDMA_HOST_DMA0_WPDMA_RX_RING0_CTRL2_ADDR,
	.host_rx_ring_didx_addr = WF_WFDMA_HOST_DMA0_WPDMA_RX_RING0_CTRL3_ADDR,
	.host_rx_ring_cnt_addr = WF_WFDMA_HOST_DMA0_WPDMA_RX_RING0_CTRL1_ADDR,

	.max_static_map_addr = 0x000f0000,

	.fw_own_clear_addr = CONNAC2X_BN0_IRQ_STAT_ADDR,
	.fw_own_clear_bit = PCIE_LPCR_FW_CLR_OWN,
	.fgCheckDriverOwnInt = FALSE,
	.u4DmaMask = 32,
	.pdmaSetup = asicConnac2xWpdmaConfig,
	.pdmaStop = NULL,
	.pdmaPollingIdle = NULL,
	/* mockingbird has no ext ctrl CR */
	.tx_ring_ext_ctrl = NULL, /* asicConnac2xWfdmaTxRingExtCtrl, */
	.rx_ring_ext_ctrl = NULL, /* asicConnac2xWfdmaRxRingExtCtrl, */
	/* null wfdmaManualPrefetch if want to disable manual mode */
	/* mockingbird has no prefetch function */
	.wfdmaManualPrefetch = NULL,
	.lowPowerOwnRead = asicConnac2xLowPowerOwnRead,
	.lowPowerOwnSet = asicConnac2xLowPowerOwnSet,
	.lowPowerOwnClear = asicConnac2xLowPowerOwnClear,
	.wakeUpWiFi = asicWakeUpWiFi,
	.processSoftwareInterrupt = asicConnac2xProcessSoftwareInterrupt,
	.softwareInterruptMcu = asicConnac2xSoftwareInterruptMcu,
	.hifRst = asicConnac2xHifRst,

	.initPcieInt = wlanBuzzardInitPcieInt,
	.bus2chip = mt7933_bus2chip_cr_mapping,
	.bus2chip_tbl_size = MT7933_BUS2CHIP_MEM_MAP_TBLE_SIZE,
	.enableInterrupt = mt7933EnableInterrupt,
	.disableInterrupt = mt7933DisableInterrupt,
	.devReadIntStatus = mt7933ReadIntStatus,
	.processTxInterrupt = mt7933Connac2xProcessTxInterrupt,
	.processRxInterrupt = mt7933Connac2xProcessRxInterrupt,
	.DmaShdlInit = NULL,
	.DmaShdlReInit = NULL,
	.setRxRingHwAddr = mt7933SetRxRingHwAddr,
	.wfdmaAllocRxRing = NULL, /* mt7933LiteWfdmaAllocRxRing, */
	.tx_ring_fwdl_idx = MT7933_FWDL_TX_RING_IDX,
	.tx_ring_cmd_idx = MT7933_CMD_TX_RING_IDX,
	.tx_ring0_data_idx = MT7933_DATA0_TXD_IDX,
	/* Mockingbird do not have ring1 in host */
	.tx_ring1_data_idx = MT7933_DATA0_TXD_IDX,
	.skip_tx_ring = BIT(TX_RING_DATA1_IDX_1),
#endif /*_HIF_PCIE || _HIF_AXI */

};

#if CFG_ENABLE_FW_DOWNLOAD
struct FWDL_OPS_T mt7933_fw_dl_ops = {
	.constructFirmwarePrio = mt7933ConstructFirmwarePrio,
	.constructPatchName = mt7933ConstructPatchName,
	.downloadPatch = wlanDownloadPatch,
	.downloadFirmware = wlanConnacFormatDownload,
	.getFwInfo = wlanGetConnacFwInfo,
	.getFwDlInfo = asicGetFwDlInfo,
	.downloadSection = mt7933WlanDownloadSection,
	.downloadSectionV2 = mt7933WlanDownloadSectionV2,
};
#endif /* CFG_ENABLE_FW_DOWNLOAD */

struct TX_DESC_OPS_T mt7933TxDescOps = {
	.fillNicAppend = fillNicTxDescAppend,
	.fillHifAppend = fillTxDescAppendByHostV2,
	.fillTxByteCount = fillConnac2xTxDescTxByteCount,
};

struct RX_DESC_OPS_T mt7933RxDescOps = {};

struct CHIP_DBG_OPS mt7933DebugOps = {
	.showPdmaInfo = mt7933_show_wfdma_info,
	.showPseInfo = mt7933_show_pse_info,
	.showPleInfo = mt7933_show_ple_info,
	.showTxdInfo = connac2x_show_txd_Info,
	.showWtblInfo = connac2x_show_wtbl_info,
	.showUmacFwtblInfo = connac2x_show_umac_wtbl_info,
	.showCsrInfo = mt7933HalShowHostCsrInfo,
	.showDmaschInfo = NULL,
	.showHifInfo = mt7933HalPrintHifDbgInfo,
	.printHifDbgInfo = mt7933HalPrintHifDbgInfo,
	.show_rx_rate_info = connac2x_show_rx_rate_info,
	.show_rx_rssi_info = connac2x_show_rx_rssi_info,
	.show_stat_info = connac2x_show_stat_info,
	.show_mcu_debug_info = NULL,
	.show_conninfra_debug_info = NULL,
};
#if CFG_SUPPORT_QA_TOOL
struct ATE_OPS_T mt7933_AteOps = {
	/*ICapStart phase out , wlan_service instead*/
	.setICapStart = connacSetICapStart,
	/*ICapStatus phase out , wlan_service instead*/
	.getICapStatus = connacGetICapStatus,
	/*CapIQData phase out , wlan_service instead*/
	.getICapIQData = connacGetICapIQData,
	.getRbistDataDumpEvent = nicExtEventICapIQData,
	.icapRiseVcoreClockRate = mt7933_icapRiseVcoreClockRate,
	.icapDownVcoreClockRate = mt7933_icapDownVcoreClockRate,
	.u4EnBitWidth = 3, /*64 bit*/
	.u4Architech = 0,  /*0:on-chip*/
	.u4PhyIdx = 0,
	.u4EmiStartAddress = 0,
	.u4EmiEndAddress = 0,
	.u4EmiMsbAddress = 0,
};
#endif


struct WIFI_CFG_NVRAM_STRUCT {
	uint8_t ucQoS;
	uint8_t ucStaHt;
	uint8_t ucStaVht;
	uint8_t ucStaHe;
	uint8_t ucApHe;
	uint8_t ucP2pGoHe;
	uint8_t ucP2pGcHe;

	uint8_t ucApHt;
	uint8_t ucApVht;
	uint8_t ucP2pGoHt;
	uint8_t ucP2pGoVht;
	uint8_t ucP2pGcHt;
	uint8_t ucP2pGcVht;

	uint8_t ucAmpduRx;
	uint8_t ucAmpduTx;
	uint8_t ucAmsduInAmpduRx;
	uint8_t ucAmsduInAmpduTx;
	uint8_t ucHtAmsduInAmpduRx;
	uint8_t ucHtAmsduInAmpduTx;
	uint8_t ucVhtAmsduInAmpduRx;
	uint8_t ucVhtAmsduInAmpduTx;

	uint8_t ucTspec;
	uint8_t ucUapsd;
	uint8_t ucStaUapsd;
	uint8_t ucApUapsd;
	uint8_t ucP2pUapsd;
	uint8_t ucRegP2pIfAtProbe;
	uint8_t ucP2pShareMacAddr;
	uint8_t ucTxShortGI;
	uint8_t ucRxShortGI;
	uint8_t ucTxLdpc;
	uint8_t ucRxLdpc;
	uint8_t ucTxStbc;
	uint8_t ucRxStbc;
	uint8_t ucRxStbcNss;
	uint8_t ucTxGf;
	uint8_t ucRxGf;
	uint8_t ucMCS32;

	uint8_t ucHeAmsduInAmpduRx;
	uint8_t ucHeAmsduInAmpduTx;
	uint8_t ucTrigMacPadDur;
	uint8_t fgEnableSR;

	uint8_t ucTWTRequester;
	uint8_t ucTWTResponder;
	uint8_t ucTWTStaBandBitmap;

	uint8_t ucSigTaRts;
	uint8_t ucTxopPsTx;
	uint8_t ucDynBwRts;
	uint8_t ucStaHtBfee;
	uint8_t ucStaVhtBfee;
	uint8_t ucStaVhtMuBfee;
	uint8_t ucStaHtBfer;
	uint8_t ucStaVhtBfer;

	uint8_t ucStaHeBfee;
	uint8_t ucHeOMCtrl;
	uint8_t fgEnShowHETrigger;

	uint8_t ucDataTxDone;
	uint8_t ucDataTxRateMode;
	uint8_t u4DataTxRateCode;
	uint8_t ucApWpsMode;
	uint8_t ucThreadScheduling;
	uint8_t ucThreadPriority;
	int8_t cThreadNice;
	uint32_t u4MaxForwardBufferCount;

	uint8_t ucApChannel;
	uint8_t ucApSco;
	uint8_t ucP2pGoSco;
	uint8_t ucStaBandwidth;
	uint8_t ucSta2gBandwidth;
	uint8_t ucSta5gBandwidth;
	uint8_t ucP2p2gBandwidth;
	uint8_t ucP2p5gBandwidth;
	uint8_t ucApBandwidth;
	uint8_t ucAp2gBandwidth;
	uint8_t ucAp5gBandwidth;

	uint8_t ucApChnlDefFromCfg;
	uint8_t ucApAllowHtVhtTkip;

	uint8_t ucNSS;
	uint8_t ucAp5gNSS;
	uint8_t ucAp2gNSS;
	uint8_t ucGo5gNSS;
	uint8_t ucGo2gNSS;

	uint8_t ucRxMaxMpduLen;
	uint32_t u4TxMaxAmsduInAmpduLen;
	uint8_t ucTcRestrict;
	uint32_t u4MaxTxDeQLimit;
	uint8_t ucAlwaysResetUsedRes;

	uint8_t ucMtkOui;
	uint32_t u4MtkOuiCap;
	uint8_t aucMtkFeature[4];
	uint8_t ucGbandProbe256QAM;

	uint8_t ucVhtIeIn2g;

	uint8_t ucCmdRsvResource;
	uint32_t u4MgmtQueueDelayTimeout;
	uint8_t u4HifIstLoopCount;
	uint8_t u4Rx2OsLoopCount;
	uint8_t u4HifTxloopCount;
	uint8_t u4TxRxLoopCount;
	uint8_t u4TxFromOsLoopCount;
	uint8_t u4TxIntThCount;

	uint32_t u4NetifStopTh;
	uint32_t u4NetifStartTh;

	uint8_t ucTxBaSize;
	uint8_t ucRxHtBaSize;
	uint8_t ucRxVhtBaSize;
	uint16_t u2RxHeBaSize;
	uint16_t u2TxHeBaSize;

	uint8_t ucExtraTxDone;
	uint8_t ucTxDbg;
	uint32_t au4TcPageCount[TC_NUM];
	uint8_t ucTxMsduQueue;

	uint32_t au4MinReservedTcResource[TC_NUM];
	uint32_t au4GuaranteedTcResource[TC_NUM];
	uint32_t u4TimeToAdjustTcResource;
	uint32_t u4TimeToUpdateQueLen;
	uint32_t u4QueLenMovingAverage;
	uint32_t u4ExtraReservedTcResource;

	uint32_t u4StatsLogTimeout;
	uint32_t u4StatsLogDuration;
	uint8_t ucDhcpTxDone;
	uint8_t ucArpTxDone;
	uint8_t ucMacAddrOverride;
	uint8_t aucOverrideMacAddr[WLAN_CFG_VALUE_LEN_MAX];
	uint8_t ucCtiaMode;
	uint8_t ucTpTestMode;

	uint8_t eDbdcMode;

	uint8_t ucEfuseBufferModeCal;

	uint8_t ucCalTimingCtrl;
	uint8_t ucWow;
	uint8_t ucOffload;
	uint8_t ucAdvPws; /* enable LP multiple DTIM function, default enable */
	uint8_t ucWowOnMdtim; /* multiple DTIM if WOW enable, default 1 */
	uint8_t ucWowOffMdtim; /* multiple DTIM if WOW disable, default 3 */

	struct WOW_CTRL rWowCtrl;

	uint8_t u4SwTestMode;
	uint8_t	ucCtrlFlagAssertPath;
	uint8_t	ucCtrlFlagDebugLevel;
	uint8_t u4ScanCtrl;
	uint8_t ucScanChannelListenTime;
	uint32_t u4WakeLockRxTimeout;
	uint32_t u4WakeLockThreadWakeup;
	uint32_t u4RegP2pIfAtProbe; /* register p2p interface during probe */
	/* p2p group interface use the same mac addr as p2p device interface */
	uint8_t ucSmartRTS;
	enum PARAM_POWER_MODE ePowerMode;
	uint32_t u4UapsdAcBmp;
	uint32_t u4MaxSpLen;
	/* 0: enable online scan, non-zero: disable online scan */
	uint32_t fgDisOnlineScan;
	uint32_t fgDisBcnLostDetection;
	uint32_t fgDisRoaming;		/* 0:enable roaming 1:disable */
	uint32_t u4AisRoamingNumber;
	uint32_t fgEnArpFilter;
	uint8_t	 uDeQuePercentEnable;
	uint32_t u4DeQuePercentVHT80Nss1;
	uint32_t u4DeQuePercentVHT40Nss1;
	uint32_t u4DeQuePercentVHT20Nss1;
	uint32_t u4DeQuePercentHT40Nss1;
	uint32_t u4DeQuePercentHT20Nss1;
	uint8_t fgTdlsBufferSTASleep; /* Support TDLS 5.5.4.2 optional case */
	uint8_t fgChipResetRecover;
	uint32_t u4PerfMonUpdatePeriod;
	uint32_t u4PerfMonTpTh[PERF_MON_TP_MAX_THRESHOLD];
	uint32_t	u4BoostCpuTh;
	uint8_t fgEnableSerL0;
	enum ENUM_FEATURE_OPTION_IN_SER eEnableSerL0p5;
	enum ENUM_FEATURE_OPTION_IN_SER eEnableSerL1;
	uint8_t fgForceSTSNum;

	uint8_t ucChannelSwtichColdownTime;
	uint8_t fgCrossBandSwitchEn;

	uint8_t fgPerfIndicatorEn;

	uint8_t ucSpeIdxCtrl;	/* 0: WF0, 1: WF1, 2: duplicate */


	uint8_t ucLowLatencyModeScan;
	uint8_t ucLowLatencyModeReOrder;
	uint8_t ucLowLatencyModePower;

	uint32_t u4MTU; /* net device maximum transmission unit */

	uint32_t ucGROFlushTimeout; /* Flush packet timeout (ms) */
	uint32_t ucGROEnableTput; /* Threshold of enable GRO Tput */

	uint8_t ucMsduReportTimeout;


	uint32_t u4PerHighThreshole;
	uint32_t u4TxLowRateThreshole;
	uint32_t u4RxLowRateThreshole;
	uint32_t u4ReportEventInterval;
	uint32_t u4TrafficThreshold;

	uint8_t u4ExtendedRange;

	uint8_t fgReuseRSNIE;
};

struct WIFI_CFG_NVRAM_STRUCT mt7933_cfg_data  = {
	.ucQoS = 1,
	.ucStaHt = 1,
	.ucStaVht =1,
#if (CFG_SUPPORT_802_11AX == 1)
	.ucStaHe = 1,
	.ucApHe = 0,
	.ucP2pGoHe = 0,
	.ucP2pGcHe = 0,
#endif
	.ucApHt = 1,
	.ucApVht = 1,
	.ucP2pGoHt = 1,
	.ucP2pGoVht = 1,
	.ucP2pGcHt = 1,
	.ucP2pGcVht = 1,
	.ucAmpduRx = 1,
	.ucAmpduTx = 1,
	.ucAmsduInAmpduRx = 1,
	.ucAmsduInAmpduTx = 1,
	.ucHtAmsduInAmpduRx = 0,
	.ucHtAmsduInAmpduTx = 0,
	.ucVhtAmsduInAmpduRx = 1,
	.ucVhtAmsduInAmpduTx = 1,

	.ucTspec = 0,

	.ucUapsd = 1,
	.ucStaUapsd = 0,
	.ucApUapsd = 0,
	.ucP2pUapsd = 1,
#if (CFG_ENABLE_WIFI_DIRECT && CFG_MTK_ANDROID_WMT)
	.u4RegP2pIfAtProbe = 1,
#else
	.u4RegP2pIfAtProbe = 0,
#endif
	.ucP2pShareMacAddr = 0,
	.ucTxShortGI = 1,
	.ucRxShortGI = 1,

	.ucTxLdpc = 1,
	.ucRxLdpc = 1,

	.ucTxStbc = 1,
	.ucRxStbc = 1,
	.ucRxStbcNss = 1,

	.ucTxGf = 1,
	.ucRxGf = 1,

	.ucMCS32 = 0,
#if (CFG_SUPPORT_802_11AX == 1)
	.ucHeAmsduInAmpduRx = 1,
	.ucHeAmsduInAmpduTx = 1,
	.ucTrigMacPadDur = HE_CAP_TRIGGER_PAD_DURATION_16,
	.fgEnableSR = 0,
#endif

#if (CFG_SUPPORT_TWT == 1)
	.ucTWTRequester = 1,
	.ucTWTResponder = 0,
	.ucTWTStaBandBitmap = BAND_2G4|BAND_5G,
#endif

	.ucSigTaRts = 0 ,
	.ucTxopPsTx = 0,
	.ucDynBwRts = 0,

	.ucStaHtBfee = 0,
	.ucStaVhtBfee = 1,
	.ucStaVhtMuBfee = 1,
	.ucStaHtBfer = 0,
	.ucStaVhtBfer = 0,

#if (CFG_SUPPORT_802_11AX == 1)
	.ucStaHeBfee = 1,
	.ucHeOMCtrl = 1,
#endif

	/* 0: disabled
	 * 1: Tx done event to driver
	 * 2: Tx status to FW only
	 */
	.ucDataTxDone = 0,
	.ucDataTxRateMode = DATA_RATE_MODE_AUTO,
	.u4DataTxRateCode = 0x0,

	.ucApWpsMode = 0,
	.ucThreadScheduling = 0,
	.ucThreadPriority = WLAN_THREAD_TASK_PRIORITY,
	.cThreadNice = WLAN_THREAD_TASK_NICE,

	.u4MaxForwardBufferCount = QM_FWD_PKT_QUE_THRESHOLD,

	/* AP channel setting
	 * 0: auto
	 */
	.ucApChannel = 0,

	/*
	 * 0: SCN
	 * 1: SCA
	 * 2: RES
	 * 3: SCB
	 */
	.ucApSco = 0,
	.ucP2pGoSco = 0,

	/* Max bandwidth setting
	 * 0: 20Mhz
	 * 1: 40Mhz
	 * 2: 80Mhz
	 * 3: 160Mhz
	 * 4: 80+80Mhz
	 * Note: For VHT STA, BW 80Mhz is a must!
	 */
	.ucStaBandwidth = MAX_BW_160MHZ,
	.ucSta2gBandwidth = MAX_BW_20MHZ,
	.ucSta5gBandwidth = MAX_BW_80MHZ,
	/* GC,GO */
	.ucP2p2gBandwidth = MAX_BW_20MHZ,
	.ucP2p5gBandwidth = MAX_BW_80MHZ,
	.ucApBandwidth = MAX_BW_160MHZ,
	.ucAp2gBandwidth = MAX_BW_20MHZ,
	.ucAp5gBandwidth = MAX_BW_80MHZ,
	.ucApChnlDefFromCfg = 1,
	.ucApAllowHtVhtTkip = 0,

	.ucNSS = 1,
	.ucAp5gNSS = 1,
	.ucAp2gNSS = 1,
	.ucGo5gNSS = 1,
	.ucGo2gNSS = 1,

	/* Max Rx MPDU length setting
	 * 0: 3k
	 * 1: 8k
	 * 2: 11k
	 */
	.ucRxMaxMpduLen = VHT_CAP_INFO_MAX_MPDU_LEN_3K,
	/* Max Tx AMSDU in AMPDU length *in BYTES* */
	.u4TxMaxAmsduInAmpduLen = 8192,

	.ucTcRestrict = 0xFF,
	/* Max Tx dequeue limit: 0 => auto */
	.u4MaxTxDeQLimit = 0x0,
	.ucAlwaysResetUsedRes = 0x0,

#if CFG_SUPPORT_MTK_SYNERGY
	.ucMtkOui = 1,
	.u4MtkOuiCap = 0,
	.aucMtkFeature[0] = 0xff,
	.aucMtkFeature[1] = 0xff,
	.aucMtkFeature[2] = 0xff,
	.aucMtkFeature[3] = 0xff,
	.ucGbandProbe256QAM = 1,
#endif
#if CFG_SUPPORT_VHT_IE_IN_2G
	.ucVhtIeIn2g = 1,
#endif
	.ucCmdRsvResource = QM_CMD_RESERVED_THRESHOLD,
	.u4MgmtQueueDelayTimeout =
		QM_MGMT_QUEUED_TIMEOUT, /* ms */

	/* Performance related */
	.u4HifIstLoopCount = CFG_IST_LOOP_COUNT,
	.u4Rx2OsLoopCount = 4,
	.u4HifTxloopCount = 1,
	.u4TxFromOsLoopCount = 1,
	.u4TxRxLoopCount = 1,
	.u4TxIntThCount = HIF_IST_TX_THRESHOLD,

	.u4NetifStopTh =
		CFG_TX_STOP_NETIF_PER_QUEUE_THRESHOLD,
	.u4NetifStartTh =
		CFG_TX_START_NETIF_PER_QUEUE_THRESHOLD,
	.ucTxBaSize = 16,
	.ucRxHtBaSize = 16,
	.ucRxVhtBaSize = 16,
#if (CFG_SUPPORT_802_11AX == 1)
	.u2RxHeBaSize = 16,
	.u2TxHeBaSize = 16,
#endif

	/* Tx Buffer Management */
	.ucExtraTxDone = 1,
	.ucTxDbg = 0,

	.ucTxMsduQueue = 0,

#if QM_ADAPTIVE_TC_RESOURCE_CTRL
	.au4MinReservedTcResource[TC0_INDEX] = QM_MIN_RESERVED_TC0_RESOURCE,
	.au4MinReservedTcResource[TC1_INDEX] = QM_MIN_RESERVED_TC1_RESOURCE,
	.au4MinReservedTcResource[TC2_INDEX] = QM_MIN_RESERVED_TC2_RESOURCE,
	.au4MinReservedTcResource[TC3_INDEX] = QM_MIN_RESERVED_TC3_RESOURCE,
	.au4MinReservedTcResource[TC4_INDEX] = QM_MIN_RESERVED_TC4_RESOURCE,

	.au4GuaranteedTcResource[TC0_INDEX] = QM_GUARANTEED_TC0_RESOURCE,
	.au4GuaranteedTcResource[TC1_INDEX] = QM_GUARANTEED_TC1_RESOURCE,
	.au4GuaranteedTcResource[TC2_INDEX] = QM_GUARANTEED_TC2_RESOURCE,
	.au4GuaranteedTcResource[TC3_INDEX] = QM_GUARANTEED_TC3_RESOURCE,
	.au4GuaranteedTcResource[TC4_INDEX] = QM_GUARANTEED_TC4_RESOURCE,

	.u4TimeToAdjustTcResource = QM_INIT_TIME_TO_ADJUST_TC_RSC,
	.u4TimeToUpdateQueLen = QM_INIT_TIME_TO_UPDATE_QUE_LEN,
	.u4QueLenMovingAverage = QM_QUE_LEN_MOVING_AVE_FACTOR,
	.u4ExtraReservedTcResource = QM_EXTRA_RESERVED_RESOURCE_WHEN_BUSY,
#endif

	/* Stats log */
	.u4StatsLogTimeout = WLAN_TX_STATS_LOG_TIMEOUT,
	.u4StatsLogDuration = WLAN_TX_STATS_LOG_DURATION,

	.ucDhcpTxDone = 1,
	.ucArpTxDone = 1,

	.ucMacAddrOverride = 0,

	.ucCtiaMode = 0,

	/* Combine ucTpTestMode and ucSigmaTestMode in one flag */
	/* ucTpTestMode == 0, for normal driver */
	/* ucTpTestMode == 1, for pure throughput test mode (ex: RvR) */
	/* ucTpTestMode == 2, for sigma TGn/TGac/PMF */
	/* ucTpTestMode == 3, for sigma WMM PS */
	.ucTpTestMode = 0,

#if CFG_SUPPORT_DBDC
	.eDbdcMode = ENUM_DBDC_MODE_DYNAMIC,
#endif /*CFG_SUPPORT_DBDC*/
#if (CFG_EFUSE_BUFFER_MODE_DELAY_CAL == 1)
	.ucEfuseBufferModeCal = 0,
#endif
	.ucCalTimingCtrl = 0,
	.ucWow = 0,
	.ucOffload = 0,
	.ucAdvPws = 1,
	.ucWowOnMdtim = 1,
	.ucWowOffMdtim = 3,

#if CFG_WOW_SUPPORT
	.rWowCtrl.fgWowEnable = 1,
	.rWowCtrl.ucScenarioId = 0,
	.rWowCtrl.ucBlockCount = 1,
	.rWowCtrl.astWakeHif[0].ucWakeupHif =
					ENUM_HIF_TYPE_GPIO,
	.rWowCtrl.astWakeHif[0].ucGpioPin = 0xFF,
  	.rWowCtrl.astWakeHif[0].ucTriggerLvl = 3,
	.rWowCtrl.astWakeHif[0].u4GpioInterval = 0,
#endif

	/* SW Test Mode: Mainly used for Sigma */
	.u4SwTestMode = ENUM_SW_TEST_MODE_NONE,
	.ucCtrlFlagAssertPath = DBG_ASSERT_PATH_DEFAULT,
	.ucCtrlFlagDebugLevel = DBG_ASSERT_CTRL_LEVEL_DEFAULT,
	.u4ScanCtrl = SCN_CTRL_DEFAULT_SCAN_CTRL,
	.ucScanChannelListenTime = 0,

	/* Wake lock related configuration */
	.u4WakeLockRxTimeout = WAKE_LOCK_RX_TIMEOUT,
	.u4WakeLockThreadWakeup = WAKE_LOCK_THREAD_WAKEUP_TIMEOUT,

	.ucSmartRTS = 0,
	.ePowerMode = Param_PowerModeMax,

#if 1
	/* add more cfg from RegInfo */
	.u4UapsdAcBmp = 0,
	.u4MaxSpLen = 0,
	.fgDisOnlineScan = 0,
	.fgDisBcnLostDetection = 0,
	.fgDisRoaming = 0,
	.u4AisRoamingNumber = KAL_AIS_NUM,
	.fgEnArpFilter = 1,
#endif

	/* Driver Flow Control Dequeue Quota. Now is only used by DBDC */
	.uDeQuePercentEnable = 1,
 	.u4DeQuePercentVHT80Nss1 = QM_DEQUE_PERCENT_VHT80_NSS1,
	.u4DeQuePercentVHT40Nss1 = QM_DEQUE_PERCENT_VHT40_NSS1,
	.u4DeQuePercentVHT20Nss1 = QM_DEQUE_PERCENT_VHT20_NSS1,
	.u4DeQuePercentHT40Nss1 = QM_DEQUE_PERCENT_HT40_NSS1,
 	.u4DeQuePercentHT20Nss1 = QM_DEQUE_PERCENT_HT20_NSS1,

	/* Support TDLS 5.5.4.2 optional case */
	.fgTdlsBufferSTASleep = 0,
	/* Support USB Whole chip reset recover */
	.fgChipResetRecover = 0,
	.u4PerfMonUpdatePeriod = PERF_MON_UPDATE_INTERVAL,

	.u4PerfMonTpTh[0] = 20,
	.u4PerfMonTpTh[1] = 50,
	.u4PerfMonTpTh[2] = 100,
	.u4PerfMonTpTh[3] = 180,
	.u4PerfMonTpTh[4] = 250,
	.u4PerfMonTpTh[5] = 300,
	.u4PerfMonTpTh[6] = 400,
	.u4PerfMonTpTh[7] = 500,
	.u4PerfMonTpTh[8] = 600,
	.u4PerfMonTpTh[9] = 700,
	.u4BoostCpuTh = 1,

	/* for SER */
	.fgEnableSerL0 = 0,
	.eEnableSerL0p5 = FEATURE_OPT_SER_DISABLE,
	.eEnableSerL1 = FEATURE_OPT_SER_ENABLE,

	/*
	 * For Certification purpose,forcibly set
	 * "Compressed Steering Number of Beamformer Antennas Supported" to our
	 * own capability.
	 */
	.fgForceSTSNum = 0,
#if CFG_SUPPORT_IDC_CH_SWITCH
	.ucChannelSwtichColdownTime = 60,/*Second*/
	.fgCrossBandSwitchEn = 0,
#endif
#if CFG_SUPPORT_PERF_IND
	.fgPerfIndicatorEn = 1,
#endif
#if CFG_SUPPORT_SPE_IDX_CONTROL
	.ucSpeIdxCtrl = 2,
#endif

#if CFG_SUPPORT_LOWLATENCY_MODE
	.ucLowLatencyModeScan = 1,
	.ucLowLatencyModeReOrder = 1,
	.ucLowLatencyModePower = 1,
#endif /* CFG_SUPPORT_LOWLATENCY_MODE */

	.u4MTU = 0,

#if CFG_SUPPORT_RX_GRO
	.ucGROFlushTimeout = 1,
	.ucGROEnableTput = 6250000,
#endif
	.ucMsduReportTimeout = NIC_MSDU_REPORT_DUMP_TIMEOUT,

#if CFG_SUPPORT_DATA_STALL
	.u4PerHighThreshole = EVENT_PER_HIGH_THRESHOLD,
	.u4TxLowRateThreshole = EVENT_TX_LOW_RATE_THRESHOLD,
	.u4RxLowRateThreshole = EVENT_RX_LOW_RATE_THRESHOLD,
	.u4ReportEventInterval = REPORT_EVENT_INTERVAL,
	.u4TrafficThreshold = TRAFFIC_RHRESHOLD,
#endif
#if ((CFG_SUPPORT_802_11AX == 1) && (CFG_SUPPORT_WIFI_SYSDVT == 1))
	.fgEnShowHETrigger = FEATURE_DISABLE,
#endif

#if CFG_SUPPORT_HE_ER
	.u4ExtendedRange = 1,
#endif

	.fgReuseRSNIE = 0,
};

void mt7933LoadCfgSetting(
	struct ADAPTER *prAdapter)
{
	struct WIFI_VAR *prWifiVar = &prAdapter->rWifiVar;
#if QM_ADAPTIVE_TC_RESOURCE_CTRL
	struct QUE_MGT *prQM = &prAdapter->rQM;
#endif


	prWifiVar->ucQoS = mt7933_cfg_data.ucQoS;
	prWifiVar->ucStaHt = mt7933_cfg_data.ucStaHt;
	prWifiVar->ucStaVht = mt7933_cfg_data.ucStaVht;

#if (CFG_SUPPORT_802_11AX == 1)
	prWifiVar->ucStaHe = mt7933_cfg_data.ucStaHe;
	prWifiVar->ucApHe = mt7933_cfg_data.ucApHe;
	prWifiVar->ucP2pGoHe = mt7933_cfg_data.ucP2pGoHe;
	prWifiVar->ucP2pGcHe = mt7933_cfg_data.ucP2pGcHe;
#endif

	prWifiVar->ucApHt = mt7933_cfg_data.ucApHt;
	prWifiVar->ucApVht = mt7933_cfg_data.ucApVht;

	prWifiVar->ucP2pGoHt = mt7933_cfg_data.ucP2pGoHt;
	prWifiVar->ucP2pGoVht = mt7933_cfg_data.ucP2pGoVht;

	prWifiVar->ucP2pGcHt = mt7933_cfg_data.ucP2pGcHt;
	prWifiVar->ucP2pGcVht = mt7933_cfg_data.ucP2pGcVht;

	prWifiVar->ucAmpduRx = mt7933_cfg_data.ucAmpduRx;
	prWifiVar->ucAmpduTx = mt7933_cfg_data.ucAmpduTx;

	prWifiVar->ucAmsduInAmpduRx = mt7933_cfg_data.ucAmsduInAmpduRx;
	prWifiVar->ucAmsduInAmpduTx = mt7933_cfg_data.ucAmsduInAmpduTx;
	prWifiVar->ucHtAmsduInAmpduRx = mt7933_cfg_data.ucHtAmsduInAmpduRx;
	prWifiVar->ucHtAmsduInAmpduTx = mt7933_cfg_data.ucHtAmsduInAmpduTx;
	prWifiVar->ucVhtAmsduInAmpduRx = mt7933_cfg_data.ucVhtAmsduInAmpduRx;
	prWifiVar->ucVhtAmsduInAmpduTx = mt7933_cfg_data.ucVhtAmsduInAmpduTx;

	prWifiVar->ucTspec = mt7933_cfg_data.ucTspec;

	prWifiVar->ucUapsd = mt7933_cfg_data.ucUapsd;
	prWifiVar->ucStaUapsd = mt7933_cfg_data.ucStaUapsd;
	prWifiVar->ucApUapsd = mt7933_cfg_data.ucApUapsd;
	prWifiVar->ucP2pUapsd = mt7933_cfg_data.ucP2pUapsd;
	prWifiVar->u4RegP2pIfAtProbe = mt7933_cfg_data.u4RegP2pIfAtProbe;
	prWifiVar->ucP2pShareMacAddr = mt7933_cfg_data.ucP2pShareMacAddr;

	prWifiVar->ucTxShortGI = mt7933_cfg_data.ucTxShortGI;
	prWifiVar->ucRxShortGI = mt7933_cfg_data.ucRxShortGI;

	prWifiVar->ucTxLdpc = mt7933_cfg_data.ucTxLdpc;
	prWifiVar->ucRxLdpc = mt7933_cfg_data.ucRxLdpc;

	prWifiVar->ucTxStbc = mt7933_cfg_data.ucTxStbc;
	prWifiVar->ucRxStbc = mt7933_cfg_data.ucRxStbc;
	prWifiVar->ucRxStbcNss = mt7933_cfg_data.ucRxStbcNss;

	prWifiVar->ucTxGf = mt7933_cfg_data.ucTxGf;
	prWifiVar->ucRxGf = mt7933_cfg_data.ucRxGf;

	prWifiVar->ucMCS32 = mt7933_cfg_data.ucMCS32;

#if (CFG_SUPPORT_802_11AX == 1)
	prWifiVar->ucHeAmsduInAmpduRx = mt7933_cfg_data.ucHeAmsduInAmpduRx;
	prWifiVar->ucHeAmsduInAmpduTx = mt7933_cfg_data.ucHeAmsduInAmpduTx;
	prWifiVar->ucTrigMacPadDur = mt7933_cfg_data.ucTrigMacPadDur;
	prWifiVar->fgEnableSR = mt7933_cfg_data.fgEnableSR;
#endif

#if (CFG_SUPPORT_TWT == 1)
	prWifiVar->ucTWTRequester = mt7933_cfg_data.ucTWTRequester;
	prWifiVar->ucTWTResponder = mt7933_cfg_data.ucTWTResponder;
	prWifiVar->ucTWTStaBandBitmap = mt7933_cfg_data.ucTWTStaBandBitmap;
#endif

	prWifiVar->ucSigTaRts = mt7933_cfg_data.ucSigTaRts;
	prWifiVar->ucDynBwRts = mt7933_cfg_data.ucDynBwRts;
	prWifiVar->ucTxopPsTx = mt7933_cfg_data.ucTxopPsTx;

	prWifiVar->ucStaHtBfee = mt7933_cfg_data.ucStaHtBfee;
	prWifiVar->ucStaVhtBfee = mt7933_cfg_data.ucStaVhtBfee;
	prWifiVar->ucStaVhtMuBfee = mt7933_cfg_data.ucStaVhtMuBfee;
	prWifiVar->ucStaHtBfer = mt7933_cfg_data.ucStaHtBfer;
	prWifiVar->ucStaVhtBfer = mt7933_cfg_data.ucStaVhtBfer;

#if (CFG_SUPPORT_802_11AX == 1)
	prWifiVar->ucStaHeBfee = mt7933_cfg_data.ucStaHeBfee;
	prWifiVar->ucHeOMCtrl = mt7933_cfg_data.ucHeOMCtrl;
#endif

	prWifiVar->ucDataTxDone = mt7933_cfg_data.ucDataTxDone;
	prWifiVar->ucDataTxRateMode = mt7933_cfg_data.ucDataTxRateMode ;
	prWifiVar->u4DataTxRateCode = mt7933_cfg_data.u4DataTxRateCode;

	prWifiVar->ucApWpsMode = mt7933_cfg_data.ucApWpsMode;
	DBGLOG(INIT, WARN, "ucApWpsMode = %u\n", prWifiVar->ucApWpsMode);

	prWifiVar->ucThreadScheduling = mt7933_cfg_data.ucThreadScheduling;
	prWifiVar->ucThreadPriority = mt7933_cfg_data.ucThreadPriority;
	prWifiVar->cThreadNice = mt7933_cfg_data.cThreadNice;

	prAdapter->rQM.u4MaxForwardBufferCount = mt7933_cfg_data.u4MaxForwardBufferCount;

	prWifiVar->ucApChannel = mt7933_cfg_data.ucApChannel;

	prWifiVar->ucApSco = mt7933_cfg_data.ucApSco;
	prWifiVar->ucP2pGoSco = mt7933_cfg_data.ucP2pGoSco;

	prWifiVar->ucStaBandwidth = mt7933_cfg_data.ucStaBandwidth;
	prWifiVar->ucSta2gBandwidth = mt7933_cfg_data.ucSta2gBandwidth;
	prWifiVar->ucSta5gBandwidth = mt7933_cfg_data.ucSta5gBandwidth;

	prWifiVar->ucP2p2gBandwidth = mt7933_cfg_data.ucP2p2gBandwidth;
	prWifiVar->ucP2p5gBandwidth = mt7933_cfg_data.ucP2p5gBandwidth;
	prWifiVar->ucApBandwidth = mt7933_cfg_data.ucApBandwidth;
	prWifiVar->ucAp2gBandwidth = mt7933_cfg_data.ucAp2gBandwidth;
	prWifiVar->ucAp5gBandwidth = mt7933_cfg_data.ucAp5gBandwidth;
	prWifiVar->ucApChnlDefFromCfg = mt7933_cfg_data.ucApChnlDefFromCfg;
	prWifiVar->ucApAllowHtVhtTkip = mt7933_cfg_data.ucApAllowHtVhtTkip;

	prWifiVar->ucNSS = mt7933_cfg_data.ucNSS;
	prWifiVar->ucAp5gNSS = mt7933_cfg_data.ucAp5gNSS;
	prWifiVar->ucAp2gNSS = mt7933_cfg_data.ucAp2gNSS;
	prWifiVar->ucGo5gNSS = mt7933_cfg_data.ucGo5gNSS;
	prWifiVar->ucGo2gNSS = mt7933_cfg_data.ucGo2gNSS;

	prWifiVar->ucRxMaxMpduLen = mt7933_cfg_data.ucRxMaxMpduLen;
	prWifiVar->u4TxMaxAmsduInAmpduLen = mt7933_cfg_data.u4TxMaxAmsduInAmpduLen;

	prWifiVar->ucTcRestrict = mt7933_cfg_data.ucTcRestrict;
	prWifiVar->u4MaxTxDeQLimit = mt7933_cfg_data.u4MaxTxDeQLimit;
	prWifiVar->ucAlwaysResetUsedRes = mt7933_cfg_data.ucAlwaysResetUsedRes;

#if CFG_SUPPORT_MTK_SYNERGY
	prWifiVar->ucMtkOui = mt7933_cfg_data.ucMtkOui;
	prWifiVar->u4MtkOuiCap = mt7933_cfg_data.u4MtkOuiCap;
	kalMemCopy(prWifiVar->aucMtkFeature, mt7933_cfg_data.aucMtkFeature, 4);

	prWifiVar->ucGbandProbe256QAM = mt7933_cfg_data.ucGbandProbe256QAM;
#endif
#if CFG_SUPPORT_VHT_IE_IN_2G
	prWifiVar->ucVhtIeIn2g = mt7933_cfg_data.ucVhtIeIn2g;
#endif
	prWifiVar->ucCmdRsvResource = mt7933_cfg_data.ucCmdRsvResource;
	prWifiVar->u4MgmtQueueDelayTimeout =
		mt7933_cfg_data.u4MgmtQueueDelayTimeout; /* ms */

	prWifiVar->u4HifIstLoopCount = mt7933_cfg_data.u4HifIstLoopCount;
	prWifiVar->u4Rx2OsLoopCount = mt7933_cfg_data.u4Rx2OsLoopCount;
	prWifiVar->u4HifTxloopCount = mt7933_cfg_data.u4HifTxloopCount;
	prWifiVar->u4TxFromOsLoopCount = mt7933_cfg_data.u4TxFromOsLoopCount;
	prWifiVar->u4TxRxLoopCount = mt7933_cfg_data.u4TxRxLoopCount;
	prWifiVar->u4TxIntThCount = mt7933_cfg_data.u4TxIntThCount;

	prWifiVar->u4NetifStopTh =
		mt7933_cfg_data.u4NetifStopTh;
	prWifiVar->u4NetifStartTh =
		mt7933_cfg_data.u4NetifStartTh;
	prWifiVar->ucTxBaSize = mt7933_cfg_data.ucTxBaSize;
	prWifiVar->ucRxHtBaSize = mt7933_cfg_data.ucRxHtBaSize;
	prWifiVar->ucRxVhtBaSize = mt7933_cfg_data.ucRxVhtBaSize;
#if (CFG_SUPPORT_802_11AX == 1)
	prWifiVar->u2RxHeBaSize = mt7933_cfg_data.u2RxHeBaSize;
	prWifiVar->u2TxHeBaSize = mt7933_cfg_data.u2TxHeBaSize;
#endif

	prWifiVar->ucExtraTxDone = mt7933_cfg_data.ucExtraTxDone;
	prWifiVar->ucTxDbg = mt7933_cfg_data.ucTxDbg;

	kalMemZero(prWifiVar->au4TcPageCount,
					sizeof(prWifiVar->au4TcPageCount));

	prWifiVar->au4TcPageCount[TC0_INDEX] = NIC_TX_PAGE_COUNT_TC0;
	prWifiVar->au4TcPageCount[TC1_INDEX] = NIC_TX_PAGE_COUNT_TC0;
	prWifiVar->au4TcPageCount[TC2_INDEX] = NIC_TX_PAGE_COUNT_TC0;
	prWifiVar->au4TcPageCount[TC3_INDEX] = NIC_TX_PAGE_COUNT_TC0;
	prWifiVar->au4TcPageCount[TC4_INDEX] = NIC_TX_PAGE_COUNT_TC0;
	prWifiVar->ucTxMsduQueue = mt7933_cfg_data.ucTxMsduQueue;

#if QM_ADAPTIVE_TC_RESOURCE_CTRL
	kalMemCopy(prQM->au4MinReservedTcResource,
		mt7933_cfg_data.au4MinReservedTcResource,
		sizeof(mt7933_cfg_data.au4MinReservedTcResource));

	kalMemCopy(prQM->au4GuaranteedTcResource,
		mt7933_cfg_data.au4GuaranteedTcResource,
		sizeof(mt7933_cfg_data.au4GuaranteedTcResource));

	prQM->u4TimeToAdjustTcResource = mt7933_cfg_data.u4TimeToAdjustTcResource;
	prQM->u4TimeToUpdateQueLen = mt7933_cfg_data.u4TimeToUpdateQueLen;
	prQM->u4QueLenMovingAverage = mt7933_cfg_data.u4QueLenMovingAverage;
	prQM->u4ExtraReservedTcResource = mt7933_cfg_data.u4ExtraReservedTcResource;
#endif

	prWifiVar->u4StatsLogTimeout = mt7933_cfg_data.u4StatsLogTimeout;
	prWifiVar->u4StatsLogDuration = mt7933_cfg_data.u4StatsLogDuration;

	prWifiVar->ucDhcpTxDone = mt7933_cfg_data.ucDhcpTxDone;
	prWifiVar->ucArpTxDone = mt7933_cfg_data.ucArpTxDone;

	prWifiVar->ucMacAddrOverride = mt7933_cfg_data.ucMacAddrOverride;

	prWifiVar->ucCtiaMode = mt7933_cfg_data.ucCtiaMode;

	prWifiVar->ucTpTestMode = mt7933_cfg_data.ucTpTestMode;

#if CFG_SUPPORT_DBDC
	prWifiVar->eDbdcMode = mt7933_cfg_data.eDbdcMode;
#endif
#if (CFG_EFUSE_BUFFER_MODE_DELAY_CAL == 1)
	prWifiVar->ucEfuseBufferModeCal = mt7933_cfg_data.ucEfuseBufferModeCal;
#endif
	prWifiVar->ucCalTimingCtrl = mt7933_cfg_data.ucCalTimingCtrl;
	prWifiVar->ucWow = mt7933_cfg_data.ucWow;
	prWifiVar->ucOffload = mt7933_cfg_data.ucOffload;
	prWifiVar->ucAdvPws = mt7933_cfg_data.ucAdvPws;
	prWifiVar->ucWowOnMdtim = mt7933_cfg_data.ucWowOnMdtim;
	prWifiVar->ucWowOffMdtim = mt7933_cfg_data.ucWowOffMdtim;

#if CFG_WOW_SUPPORT
	kalMemCopy(&(prAdapter->rWowCtrl),
			&(mt7933_cfg_data.rWowCtrl), sizeof(mt7933_cfg_data.rWowCtrl));
#endif

	prWifiVar->u4SwTestMode = mt7933_cfg_data.u4SwTestMode;
	prWifiVar->ucCtrlFlagAssertPath = mt7933_cfg_data.ucCtrlFlagAssertPath;
	prWifiVar->ucCtrlFlagDebugLevel = mt7933_cfg_data.ucCtrlFlagDebugLevel;
	prWifiVar->u4ScanCtrl = mt7933_cfg_data.u4ScanCtrl;
	prWifiVar->ucScanChannelListenTime = mt7933_cfg_data.ucScanChannelListenTime;

	prWifiVar->u4WakeLockRxTimeout = mt7933_cfg_data.u4WakeLockRxTimeout;
	prWifiVar->u4WakeLockThreadWakeup = mt7933_cfg_data.u4WakeLockThreadWakeup;

	prWifiVar->ucSmartRTS = mt7933_cfg_data.ucSmartRTS;
	prWifiVar->ePowerMode = mt7933_cfg_data.ePowerMode;

	prWifiVar->u4UapsdAcBmp = mt7933_cfg_data.u4UapsdAcBmp;
	prWifiVar->u4MaxSpLen = mt7933_cfg_data.u4MaxSpLen;
	prWifiVar->fgDisOnlineScan = mt7933_cfg_data.fgDisOnlineScan;
	prWifiVar->fgDisBcnLostDetection = mt7933_cfg_data.fgDisBcnLostDetection;
	prWifiVar->fgDisRoaming = mt7933_cfg_data.fgDisRoaming;
	prWifiVar->u4AisRoamingNumber = mt7933_cfg_data.u4AisRoamingNumber;
	prWifiVar->fgEnArpFilter = mt7933_cfg_data.fgEnArpFilter;

	prWifiVar->uDeQuePercentEnable = mt7933_cfg_data.uDeQuePercentEnable;
	prWifiVar->u4DeQuePercentVHT80Nss1 = mt7933_cfg_data.u4DeQuePercentVHT80Nss1;
	prWifiVar->u4DeQuePercentVHT40Nss1 = mt7933_cfg_data.u4DeQuePercentVHT40Nss1;
	prWifiVar->u4DeQuePercentVHT20Nss1 = mt7933_cfg_data.u4DeQuePercentVHT20Nss1;
	prWifiVar->u4DeQuePercentHT40Nss1 = mt7933_cfg_data.u4DeQuePercentHT40Nss1;
	prWifiVar->u4DeQuePercentHT20Nss1 = mt7933_cfg_data.u4DeQuePercentHT20Nss1;

	prWifiVar->fgTdlsBufferSTASleep = mt7933_cfg_data.fgTdlsBufferSTASleep;
	prWifiVar->fgChipResetRecover = mt7933_cfg_data.fgChipResetRecover;

	prWifiVar->u4PerfMonUpdatePeriod = mt7933_cfg_data.u4PerfMonUpdatePeriod;

	kalMemCopy(prWifiVar->u4PerfMonTpTh,
			mt7933_cfg_data.u4PerfMonTpTh, sizeof(mt7933_cfg_data.u4PerfMonTpTh));
	prWifiVar->u4BoostCpuTh = mt7933_cfg_data.u4BoostCpuTh;

	prWifiVar->fgEnableSerL0 = mt7933_cfg_data.fgEnableSerL0;
	prWifiVar->eEnableSerL0p5 = mt7933_cfg_data.eEnableSerL0p5;
	prWifiVar->eEnableSerL1 = mt7933_cfg_data.eEnableSerL1;
	prWifiVar->fgForceSTSNum = mt7933_cfg_data.fgForceSTSNum;
#if CFG_SUPPORT_IDC_CH_SWITCH
	prWifiVar->ucChannelSwtichColdownTime = mt7933_cfg_data.ucChannelSwtichColdownTime;/*Second*/
	prWifiVar->fgCrossBandSwitchEn = mt7933_cfg_data.fgCrossBandSwitchEn;
#endif
#if CFG_SUPPORT_PERF_IND
	prWifiVar->fgPerfIndicatorEn = mt7933_cfg_data.fgPerfIndicatorEn;
#endif
#if CFG_SUPPORT_SPE_IDX_CONTROL
	prWifiVar->ucSpeIdxCtrl = mt7933_cfg_data.ucSpeIdxCtrl;
#endif

#if CFG_SUPPORT_LOWLATENCY_MODE
	prWifiVar->ucLowLatencyModeScan = mt7933_cfg_data.ucLowLatencyModeScan;
	prWifiVar->ucLowLatencyModeReOrder = mt7933_cfg_data.ucLowLatencyModeReOrder;
	prWifiVar->ucLowLatencyModePower = mt7933_cfg_data.ucLowLatencyModePower;
#endif

	prWifiVar->u4MTU = mt7933_cfg_data.u4MTU;

#if CFG_SUPPORT_RX_GRO
	prWifiVar->ucGROFlushTimeout = mt7933_cfg_data.ucGROFlushTimeout;
	prWifiVar->ucGROEnableTput = mt7933_cfg_data.ucGROEnableTput;
#endif
	prWifiVar->ucMsduReportTimeout = mt7933_cfg_data.ucMsduReportTimeout;

#if CFG_SUPPORT_DATA_STALL
	prWifiVar->u4PerHighThreshole = mt7933_cfg_data.u4PerHighThreshole;
	prWifiVar->u4TxLowRateThreshole = mt7933_cfg_data.u4TxLowRateThreshole;
	prWifiVar->u4RxLowRateThreshole = mt7933_cfg_data.u4RxLowRateThreshole;
	prWifiVar->u4ReportEventInterval = mt7933_cfg_data.u4ReportEventInterval;
	prWifiVar->u4TrafficThreshold = mt7933_cfg_data.u4TrafficThreshold;
#endif
#if ((CFG_SUPPORT_802_11AX == 1) && (CFG_SUPPORT_WIFI_SYSDVT == 1))
	prWifiVar->fgEnShowHETrigger = mt7933_cfg_data.fgEnShowHETrigger;
#endif

#if CFG_SUPPORT_HE_ER
	prWifiVar->u4ExtendedRange = mt7933_cfg_data.u4ExtendedRange;
#endif
	prWifiVar->fgReuseRSNIE = mt7933_cfg_data.fgReuseRSNIE;
}

/* Litien code refine to support multi chip */
struct mt66xx_chip_info mt66xx_chip_info_mt7933 = {
	.bus_info = &mt7933_bus_info,
#if CFG_ENABLE_FW_DOWNLOAD
	.fw_dl_ops = &mt7933_fw_dl_ops,
#endif /* CFG_ENABLE_FW_DOWNLOAD */
#if CFG_SUPPORT_QA_TOOL
	.prAteOps = &mt7933_AteOps,
#endif
	.prTxDescOps = &mt7933TxDescOps,
	.prRxDescOps = &mt7933RxDescOps,
	.prDebugOps = &mt7933DebugOps,
	.chip_id = MT7933_CHIP_ID,
	.should_verify_chip_id = FALSE,
	.sw_sync0 = MT7933_SW_SYNC0,
	.sw_ready_bits = WIFI_FUNC_NO_CR4_READY_BITS,
	.sw_ready_bit_offset = MT7933_SW_SYNC0_RDY_OFFSET,
	.patch_addr = MT7933_PATCH_START_ADDR,
	.is_support_cr4 = FALSE,
	.is_support_wacpu = FALSE,
	.txd_append_size = MT7933_TX_DESC_APPEND_LENGTH,
	.rxd_size = MT7933_RX_DESC_LENGTH,
	.pse_header_length = CONNAC2X_NIC_TX_PSE_HEADER_LENGTH,
	.init_event_size = CONNAC2X_RX_INIT_EVENT_LENGTH,
	.eco_info = mt7933_eco_table,
	.isNicCapV1 = FALSE,
	.top_hcr = CONNAC2X_TOP_HCR,
	.top_hvr = CONNAC2X_TOP_HVR,
	.top_fvr = CONNAC2X_TOP_FVR,
	.arb_ac_mode_addr = MT7933_ARB_AC_MODE_ADDR,
	.asicCapInit = asicConnac2xCapInit,
#if CFG_ENABLE_FW_DOWNLOAD
	.asicEnableFWDownload = NULL,
#endif /* CFG_ENABLE_FW_DOWNLOAD */
	.asicDumpSerDummyCR = NULL, /* mt7961DumpSerDummyCR, */
	.downloadBufferBin = NULL, /* wlanConnac2XDownloadBufferBin, */
	.is_support_hw_amsdu = TRUE,
	.is_support_asic_lp = TRUE,
	.is_support_wfdma1 = FALSE,
	.asicWfdmaReInit = asicConnac2xWfdmaReInit,
	.asicWfdmaReInit_handshakeInit = asicConnac2xWfdmaDummyCrWrite,
	.group5_size = sizeof(struct HW_MAC_RX_STS_GROUP_5),
	.u4LmacWtblDUAddr = CONNAC2X_WIFI_LWTBL_BASE,
	.u4UmacWtblDUAddr = CONNAC2X_WIFI_UWTBL_BASE,

#if 0 /* CFG_CHIP_RESET_SUPPORT */
	.asicWfsysRst = mt7961HalCbtopRguWfRst,
	.asicPollWfsysSwInitDone = mt7961HalPollWfsysSwInitDone,
#endif

	/* leave it to project owner */
	.uc2G4HeCapMaxAmpduLenExp = 0,
	.uc5GHeCapMaxAmpduLenExp = 0,
	.loadCfgSetting = mt7933LoadCfgSetting,
};

struct mt66xx_hif_driver_data mt66xx_driver_data_mt7933 = {
	.chip_info = &mt66xx_chip_info_mt7933,
};

#endif /* MT7933 */
