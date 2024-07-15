/* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause */
/*
 * Copyright (c) 2016 MediaTek Inc.
 */
/*! \file   mt7961.c
*    \brief  Internal driver stack will export
*    the required procedures here for GLUE Layer.
*
*    This file contains all routines which are exported
     from MediaTek 802.11 Wireless LAN driver stack to GLUE Layer.
*/

#if defined(MT7961) || defined(MT7922) || defined(MT7902)

/*******************************************************************************
*                         C O M P I L E R   F L A G S
********************************************************************************
*/

/*******************************************************************************
*                    E X T E R N A L   R E F E R E N C E S
********************************************************************************
*/

#include "coda/mt7961/wf_wfdma_host_dma0.h"
#include "coda/mt7961/wf_cr_sw_def.h"
#include "precomp.h"
#include "mt7961.h"
#include "hal_dmashdl_mt7961.h"
#include "hal_wfsys_reset_mt7961.h"
#include "hif.h"

/*******************************************************************************
*                              C O N S T A N T S
********************************************************************************
*/

#define CONN_MCU_CONFG_BASE 0x88000000
#define CONN_MCU_CONFG_COM_REG0_ADDR (CONN_MCU_CONFG_BASE + 0x200)

#define PATCH_SEMAPHORE_COMM_REG 0
#define PATCH_SEMAPHORE_COMM_REG_PATCH_DONE 1 /* bit0 is for patch. */

#define SW_WORKAROUND_FOR_WFDMA_ISSUE_HWITS00009838 1
#define SW_WORKAROUND_FOR_WFDMA_ISSUE_HWITS00015048 1

#define RX_DATA_RING_BASE_IDX 2

#define PCIE_LOW_POWER_CTRL_ADDR 0x74030194
#define PCIE_LTR_VALUES_ADDR 0x740301A4
#define PCIE_LOW_POWER_CTRL_DIS_L11_L12 BITS(11, 10)
/*******************************************************************************
*                                 M A C R O S
********************************************************************************
*/
#define MT7961_BUS2CHIP_MEM_MAP_TBLE_SIZE    \
	(sizeof(mt7961_bus2chip_cr_mapping) /	\
	 sizeof(mt7961_bus2chip_cr_mapping[0]))

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

struct ECO_INFO mt7961_eco_table[] = {
	/* HW version,  ROM version, Factory version, EcoVer */
	{0x00, 0x00, 0xA, 0x1},	/* E1 */
	{0x10, 0x01, 0xA, 0x2},	/* E2 */
	{0x00, 0x00, 0x0, 0x0}	/* End of table */
};

#if defined(_HIF_PCIE) || defined(_HIF_AXI)
struct PCIE_CHIP_CR_MAPPING mt7961_bus2chip_cr_mapping[] = {
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
	{0x74030000, 0x10000, 0x10000}, /* PCIE_MAC_IREG */
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
	{0x00400000, 0x80000, 0x10000}, /* WF_MCU_SYSRAM */
	{0x00410000, 0x90000, 0x10000}, /* WF_MCU_SYSRAM (configure register) */
	{0x820f0000, 0xa0000, 0x0400},  /* WF_LMAC_TOP BN1 (WF_CFG) */
	{0x820f1000, 0xa0600, 0x0200},  /* WF_LMAC_TOP BN1 (WF_TRB) */
	{0x820f2000, 0xa0800, 0x0400},  /* WF_LMAC_TOP BN1 (WF_AGG) */
	{0x820f3000, 0xa0c00, 0x0400},  /* WF_LMAC_TOP BN1 (WF_ARB) */
	{0x820f4000, 0xa1000, 0x0400},  /* WF_LMAC_TOP BN1 (WF_TMAC) */
	{0x820f5000, 0xa1400, 0x0800},  /* WF_LMAC_TOP BN1 (WF_RMAC) */
	{0x820f7000, 0xa1e00, 0x0200},  /* WF_LMAC_TOP BN1 (WF_DMA) */
	{0x820f9000, 0xa3400, 0x0200},  /* WF_LMAC_TOP BN1 (WF_WTBLOFF) */
	{0x820fa000, 0xa4000, 0x0200},  /* WF_LMAC_TOP BN1 (WF_ETBF) */
	{0x820fb000, 0xa4200, 0x0400},  /* WF_LMAC_TOP BN1 (WF_LPON) */
	{0x820fc000, 0xa4600, 0x0200},  /* WF_LMAC_TOP BN1 (WF_INT) */
	{0x820fd000, 0xa4800, 0x0800},  /* WF_LMAC_TOP BN1 (WF_MIB) */
	{0x820c4000, 0xa8000, 0x4000},  /* WF_LMAC_TOP BN1 (WF_MUCOP) */
	{0x820b0000, 0xae000, 0x1000},  /* [APB2] WFSYS_ON */
	{0x80020000, 0xb0000, 0x10000}, /* WF_TOP_MISC_OFF */
	{0x81020000, 0xc0000, 0x10000}, /* WF_TOP_MISC_ON */
	{0x7c020000, 0xd0000, 0x10000}, /* CONN_INFRA, wfdma */
	{0x7c060000, 0xe0000, 0x10000}, /* CONN_INFRA, conn_host_csr_top */
	{0x7c000000, 0xf0000, 0x10000}, /* CONN_INFRA */
};

void mt7961EnableInterrupt(
	struct ADAPTER *prAdapter)
{
	struct mt66xx_chip_info *prChipInfo;
	union WPDMA_INT_MASK IntMask;
	uint32_t u4HostWpdamBase = 0;
#if CFG_CHIP_RESET_SUPPORT
	uint32_t u4IntEna;
#endif

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
	IntMask.field_conn2x_single.wfdma0_rx_done_2 = 1;
	IntMask.field_conn2x_single.wfdma0_rx_done_3 = 1;
	IntMask.field_conn2x_single.wfdma0_rx_done_4 = 1;
	IntMask.field_conn2x_single.wfdma0_rx_done_5 = 1;
	IntMask.field_conn2x_single.wfdma0_tx_done_0 = 1;
	IntMask.field_conn2x_single.wfdma0_tx_done_16 = 1;
	IntMask.field_conn2x_single.wfdma0_tx_done_17 = 1;
	IntMask.field_conn2x_single.wfdma0_mcu2host_sw_int_en = 1;

	IntMask.field_conn2x_single.wfdma0_rx_coherent = 0;
	IntMask.field_conn2x_single.wfdma0_tx_coherent = 0;

	/* TX-done-int can be handled in TX-direct patch & TX-MSDU-report
	 * Disable this for lower CPU usage under Tput test
	 */
	if (HAL_IS_TX_DIRECT())
		IntMask.field_conn2x_single.wfdma0_tx_done_0 =
		kalIsTputMode(prAdapter, PKT_PATH_ALL, MAX_BSSID_NUM)?0:1;

	HAL_MCR_WR(prAdapter,
		WF_WFDMA_HOST_DMA0_HOST_INT_ENA_ADDR, IntMask.word);

	if (prChipInfo->is_support_asic_lp)
		HAL_MCR_WR_FIELD(prAdapter,
				 CONNAC2X_WPDMA_MCU2HOST_SW_INT_MASK
				 (u4HostWpdamBase),
				 BITS(0, 15),
				 0,
				 BITS(0, 15));

#if CFG_CHIP_RESET_SUPPORT
	/* WF WDT interrupt to host via PCIE */
	if (prAdapter->eWfsysResetState == WFSYS_RESET_STATE_IDLE &&
			prChipInfo->fgIsSupportL0p5Reset == TRUE) {
		HAL_MCR_RD(prAdapter, PCIE_MAC_IREG_IMASK_HOST_ADDR, &u4IntEna);
		u4IntEna |= PCIE_MAC_IREG_IMASK_HOST_WDT_INT_MASK;
		HAL_MCR_WR(prAdapter, PCIE_MAC_IREG_IMASK_HOST_ADDR, u4IntEna);
	}
#endif

	DBGLOG(HAL, TRACE, "%s [0x%08x]\n", __func__, IntMask.word);
} /* end of nicEnableInterrupt() */

void mt7961DisableInterrupt(
	struct ADAPTER *prAdapter)
{
	struct GLUE_INFO *prGlueInfo = NULL;
	union WPDMA_INT_MASK IntMask;
#if CFG_CHIP_RESET_SUPPORT
	uint32_t u4IntEna = 0;
#endif

	ASSERT(prAdapter);

	prGlueInfo = prAdapter->prGlueInfo;

	IntMask.word = 0;

	HAL_MCR_WR(prAdapter,
		WF_WFDMA_HOST_DMA0_HOST_INT_ENA_ADDR, IntMask.word);
	HAL_MCR_RD(prAdapter,
		WF_WFDMA_HOST_DMA0_HOST_INT_ENA_ADDR, &IntMask.word);

#if CFG_CHIP_RESET_SUPPORT
    if (prAdapter->chip_info->fgIsSupportL0p5Reset == TRUE) {
		/* WF WDT interrupt to host via PCIE */
		HAL_MCR_RD(prAdapter, PCIE_MAC_IREG_IMASK_HOST_ADDR, &u4IntEna);
		u4IntEna &= ~PCIE_MAC_IREG_IMASK_HOST_WDT_INT_MASK;
		HAL_MCR_WR(prAdapter, PCIE_MAC_IREG_IMASK_HOST_ADDR, u4IntEna);
	}
#endif
	prAdapter->fgIsIntEnable = FALSE;

	DBGLOG(HAL, TRACE, "%s\n", __func__);
}

uint8_t mt7961SetRxRingHwAddr(
	struct RTMP_RX_RING *prRxRing,
	struct BUS_INFO *prBusInfo,
	uint32_t u4SwRingIdx)
{
	uint32_t offset = 0;

	/*
	 * RX_RING_EVT_IDX_1    (RX_Ring0) - Rx Event
	 * RX_RING_DATA_IDX_0   (RX_Ring2) - Band0 Rx Data
	 * WFDMA0_RX_RING_IDX_2 (RX_Ring3) - Band1 Rx Data
	 * WFDMA0_RX_RING_IDX_3 (RX_Ring4) - Band0 Tx Free Done Event
	 * WFDMA1_RX_RING_IDX_0 (RX_Ring5) - Band1 Tx Free Done Event
	*/
	switch (u4SwRingIdx) {
	case RX_RING_EVT_IDX_1:
		offset = 0;
		break;
	case RX_RING_DATA_IDX_0:
		offset = RX_DATA_RING_BASE_IDX * MT_RINGREG_DIFF;
		break;
	case WFDMA0_RX_RING_IDX_2:
	case WFDMA0_RX_RING_IDX_3:
	case WFDMA1_RX_RING_IDX_0:
		offset = (u4SwRingIdx + 1) * MT_RINGREG_DIFF;
		break;
	default:
		return FALSE;
	}

	prRxRing->hw_desc_base = prBusInfo->host_rx_ring_base + offset;
	prRxRing->hw_cidx_addr = prBusInfo->host_rx_ring_cidx_addr + offset;
	prRxRing->hw_didx_addr = prBusInfo->host_rx_ring_didx_addr + offset;
	prRxRing->hw_cnt_addr = prBusInfo->host_rx_ring_cnt_addr + offset;

	return TRUE;
}

void wlanBuzzardInitPcieInt(
	struct GLUE_INFO *prGlueInfo)
{
       /* PCIE interrupt DMA end enable  */
	HAL_MCR_WR(prGlueInfo->prAdapter,
		0x10188,
		0x000000FF);
}

#if CFG_SUPPORT_PCIE_ASPM_IMPROVE

void mt7961ConfigPcieASPM(
	struct GLUE_INFO *prGlueInfo,
	bool fgActivate)
{
	struct GL_HIF_INFO *prHifInfo = &prGlueInfo->rHifInfo;

	if (fgActivate) {
		/*
		 *	Backup original setting then
		 *	disable L1.1, L1.2 and set LTR to 0
		 */
		HAL_MCR_RD(prGlueInfo->prAdapter,
			PCIE_LTR_VALUES_ADDR, &prHifInfo->u4PcieLTR);
		HAL_MCR_RD(prGlueInfo->prAdapter,
			PCIE_LOW_POWER_CTRL_ADDR, &prHifInfo->u4PcieASPM);
		HAL_MCR_WR(prGlueInfo->prAdapter,
			PCIE_LTR_VALUES_ADDR, 0);
		HAL_MCR_WR(prGlueInfo->prAdapter,
			PCIE_LOW_POWER_CTRL_ADDR,
			prHifInfo->u4PcieASPM |
				PCIE_LOW_POWER_CTRL_DIS_L11_L12);
	} else {
		/* Restore original setting*/
		HAL_MCR_WR(prGlueInfo->prAdapter,
			PCIE_LTR_VALUES_ADDR, prHifInfo->u4PcieLTR);
		HAL_MCR_WR(prGlueInfo->prAdapter,
			PCIE_LOW_POWER_CTRL_ADDR, prHifInfo->u4PcieASPM);
	}
}

#define CONNAC2X_TX_DESC_FIXDE_RATE_TX_MODE_MASK BITS(9, 6)

void mt7961SetCTSbyRate(
	struct GLUE_INFO *prGlueInfo,
	struct MSDU_INFO *prMsduInfo, void *prTxDesc)
{
	uint32_t u4RateCode;
	uint32_t u4TxMode;

	if (prMsduInfo->ucRateMode != MSDU_RATE_MODE_MANUAL_DESC)
		return;

	u4RateCode = prMsduInfo->u4FixedRateOption;
	u4RateCode =
		(u4RateCode & CONNAC2X_TX_DESC_FIXDE_RATE_MASK) >>
			CONNAC2X_TX_DESC_FIXDE_RATE_OFFSET;
	u4TxMode = u4RateCode & CONNAC2X_TX_DESC_FIXDE_RATE_TX_MODE_MASK;
	if (u4TxMode <= TX_MODE_OFDM) {
		HAL_MAC_CONNAC2X_TXD_SET_FORCE_RTS_CTS(
			(struct HW_MAC_CONNAC2X_TX_DESC *)prTxDesc);
	}
}

#endif

bool mt7961LiteWfdmaAllocRxRing(
	struct GLUE_INFO *prGlueInfo,
	bool fgAllocMem)
{
	/* Band1 Data Rx path */
	if (!halWpdmaAllocRxRing(prGlueInfo,
			WFDMA0_RX_RING_IDX_2, RX_RING0_SIZE,
			RXD_SIZE, CFG_RX_MAX_PKT_SIZE, fgAllocMem)) {
		DBGLOG(HAL, ERROR, "AllocRxRing[0] fail\n");
		return false;
	}
	/* Band0 Tx Free Done Event */
#if CFG_SUPPORT_HOST_RX_WM_EVENT_FROM_PSE
	if (!halWpdmaAllocRxRing(prGlueInfo,
			WFDMA0_RX_RING_IDX_3,
			MT7961_HOST_RX_WM_EVENT_FROM_PSE_RX_RING4_SIZE,
			RXD_SIZE, CFG_RX_MAX_PKT_SIZE, fgAllocMem)) {
		DBGLOG(HAL, ERROR, "AllocRxRing[1] fail\n");
		return false;
	}
#else
	if (!halWpdmaAllocRxRing(prGlueInfo,
			WFDMA0_RX_RING_IDX_3, RX_RING1_SIZE,
			RXD_SIZE, CFG_RX_MAX_PKT_SIZE, fgAllocMem)) {
		DBGLOG(HAL, ERROR, "AllocRxRing[1] fail\n");
		return false;
	}
#endif
	/* Band1 Tx Free Done Event */
	if (!halWpdmaAllocRxRing(prGlueInfo,
			WFDMA1_RX_RING_IDX_0, RX_RING1_SIZE,
			RXD_SIZE, CFG_RX_MAX_PKT_SIZE, fgAllocMem)) {
		DBGLOG(HAL, ERROR, "AllocRxRing[1] fail\n");
		return false;
	}
	return true;
}

void mt7961Connac2xProcessTxInterrupt(
	struct ADAPTER *prAdapter)
{
	struct GL_HIF_INFO *prHifInfo = &prAdapter->prGlueInfo->rHifInfo;
	union WPDMA_INT_STA_STRUCT rIntrStatus;

	rIntrStatus = (union WPDMA_INT_STA_STRUCT)prHifInfo->u4IntStatus;
	if (rIntrStatus.field_conn2x_single.wfdma0_tx_done_16)
		halWpdmaProcessCmdDmaDone(
			prAdapter->prGlueInfo, TX_RING_FWDL_IDX_3);

	if (rIntrStatus.field_conn2x_single.wfdma0_tx_done_17)
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

	if (rIntrStatus.field_conn2x_single.wfdma0_tx_done_1) {
		halWpdmaProcessDataDmaDone(
			prAdapter->prGlueInfo, TX_RING_DATA1_IDX_1);
#if CFG_SUPPORT_MULTITHREAD
		if (!HAL_IS_TX_DIRECT())
			kalSetTxEvent2Hif(prAdapter->prGlueInfo);
#endif
	}
}

void mt7961Connac2xProcessRxInterrupt(
	struct ADAPTER *prAdapter)
{
	struct GL_HIF_INFO *prHifInfo = &prAdapter->prGlueInfo->rHifInfo;
	union WPDMA_INT_STA_STRUCT rIntrStatus;

	rIntrStatus = (union WPDMA_INT_STA_STRUCT)prHifInfo->u4IntStatus;
	if (rIntrStatus.field_conn2x_single.wfdma0_rx_done_0)
		halRxReceiveRFBs(prAdapter, RX_RING_EVT_IDX_1, FALSE);

	if (rIntrStatus.field_conn2x_single.wfdma0_rx_done_2)
		halRxReceiveRFBs(prAdapter, RX_RING_DATA_IDX_0, TRUE);

	if (rIntrStatus.field_conn2x_single.wfdma0_rx_done_3)
		halRxReceiveRFBs(prAdapter, WFDMA0_RX_RING_IDX_2, TRUE);

#if CFG_SUPPORT_HOST_RX_WM_EVENT_FROM_PSE
	if (rIntrStatus.field_conn2x_single.wfdma0_rx_done_4)
		halRxReceiveRFBs(prAdapter, WFDMA0_RX_RING_IDX_3, FALSE);
#else
	if (rIntrStatus.field_conn2x_single.wfdma0_rx_done_4)
		halRxReceiveRFBs(prAdapter, WFDMA0_RX_RING_IDX_3, TRUE);
#endif

	if (rIntrStatus.field_conn2x_single.wfdma0_rx_done_5)
		halRxReceiveRFBs(prAdapter, WFDMA1_RX_RING_IDX_0, TRUE);
}

void mt7961WfdmaTxRingExtCtrl(
	struct GLUE_INFO *prGlueInfo,
	struct RTMP_TX_RING *tx_ring,
	uint32_t index)
{
	struct BUS_INFO *prBusInfo;
	uint32_t u4Offset = 0;

	prBusInfo = prGlueInfo->prAdapter->chip_info->bus_info;

	/*
	 * TX_RING_DATA0_IDX_0   (TX_Ring0)  - Band0 Tx Data
	 * TX_RING_DATA1_IDX_1   (TX_Ring1)  - Band1 Tx Data
	 * TX_RING_FWDL_IDX_3    (TX_Ring16) - FW download
	 * TX_RING_CMD_IDX_2     (TX_Ring17) - FW CMD
	 */
	switch (index) {
	case TX_RING_DATA0_IDX_0:
		u4Offset = HW_WFDMA0_TX_RING_IDX_0 * MT_RINGREG_EXT_DIFF;
		break;

	case TX_RING_DATA1_IDX_1:
		u4Offset = HW_WFDMA0_TX_RING_IDX_1 * MT_RINGREG_EXT_DIFF;
		break;

	case TX_RING_FWDL_IDX_3:
		u4Offset = HW_WFDMA0_TX_RING_IDX_16 * MT_RINGREG_EXT_DIFF;
		break;

	case TX_RING_CMD_IDX_2:
		u4Offset = HW_WFDMA0_TX_RING_IDX_17 * MT_RINGREG_EXT_DIFF;
		break;

	default:
		return;
	}

	tx_ring->hw_desc_base_ext =
		prBusInfo->host_tx_ring_ext_ctrl_base + u4Offset;
}

void mt7961WfdmaRxRingExtCtrl(
	struct GLUE_INFO *prGlueInfo,
	struct RTMP_RX_RING *rx_ring,
	uint32_t index)
{
	struct BUS_INFO *prBusInfo;
	uint32_t u4Offset = 0;

	prBusInfo = prGlueInfo->prAdapter->chip_info->bus_info;

	/*
	 * RX_RING_EVT_IDX_1    (RX_Ring0) - Rx Event
	 * RX_RING_DATA_IDX_0   (RX_Ring2) - Band0 Rx Data
	 * WFDMA0_RX_RING_IDX_2 (RX_Ring3) - Band1 Rx Data
	 * WFDMA0_RX_RING_IDX_3 (RX_Ring4) - Band0 Tx Free Done Event
	 * WFDMA1_RX_RING_IDX_0 (RX_Ring5) - Band1 Tx Free Done Event
	*/
	switch (index) {
	case RX_RING_EVT_IDX_1:
		u4Offset = HW_WFDMA0_RX_RING_IDX_0 * MT_RINGREG_EXT_DIFF;
		break;

	case RX_RING_DATA_IDX_0:
		u4Offset = HW_WFDMA0_RX_RING_IDX_2 * MT_RINGREG_EXT_DIFF;
		break;

	case WFDMA0_RX_RING_IDX_2:
		u4Offset = HW_WFDMA0_RX_RING_IDX_3 * MT_RINGREG_EXT_DIFF;
		break;

	case WFDMA0_RX_RING_IDX_3:
		u4Offset = HW_WFDMA0_RX_RING_IDX_4 * MT_RINGREG_EXT_DIFF;
		break;

	case WFDMA1_RX_RING_IDX_0:
		u4Offset = HW_WFDMA0_RX_RING_IDX_5 * MT_RINGREG_EXT_DIFF;
		break;

	default:
		return;
	}

	rx_ring->hw_desc_base_ext =
		prBusInfo->host_rx_ring_ext_ctrl_base + u4Offset;
}

void mt7961Connac2xWfdmaManualPrefetch(
	struct GLUE_INFO *prGlueInfo)
{
	struct ADAPTER *prAdapter = prGlueInfo->prAdapter;
	u_int32_t val = 0;

	HAL_MCR_RD(prAdapter, WF_WFDMA_HOST_DMA0_WPDMA_GLO_CFG_ADDR, &val);
	/* disable prefetch offset calculation auto-mode */
	val &=
	~WF_WFDMA_HOST_DMA0_WPDMA_GLO_CFG_CSR_DISP_BASE_PTR_CHAIN_EN_MASK;
	HAL_MCR_WR(prAdapter, WF_WFDMA_HOST_DMA0_WPDMA_GLO_CFG_ADDR, val);

	HAL_MCR_WR(prAdapter, WF_WFDMA_HOST_DMA0_WPDMA_RX_RING0_EXT_CTRL_ADDR,
	     0x00000004);
	HAL_MCR_WR(prAdapter, WF_WFDMA_HOST_DMA0_WPDMA_RX_RING2_EXT_CTRL_ADDR,
	     0x00400004);
	HAL_MCR_WR(prAdapter, WF_WFDMA_HOST_DMA0_WPDMA_RX_RING3_EXT_CTRL_ADDR,
	     0x00800004);
	HAL_MCR_WR(prAdapter, WF_WFDMA_HOST_DMA0_WPDMA_RX_RING4_EXT_CTRL_ADDR,
	     0x00c00004);
	HAL_MCR_WR(prAdapter, WF_WFDMA_HOST_DMA0_WPDMA_RX_RING5_EXT_CTRL_ADDR,
	     0x01000004);

	HAL_MCR_WR(prAdapter, WF_WFDMA_HOST_DMA0_WPDMA_TX_RING0_EXT_CTRL_ADDR,
	     0x01400004);
	HAL_MCR_WR(prAdapter, WF_WFDMA_HOST_DMA0_WPDMA_TX_RING1_EXT_CTRL_ADDR,
	     0x01800004);
	HAL_MCR_WR(prAdapter, WF_WFDMA_HOST_DMA0_WPDMA_TX_RING2_EXT_CTRL_ADDR,
	     0x01c00004);

	HAL_MCR_WR(prAdapter, WF_WFDMA_HOST_DMA0_WPDMA_TX_RING3_EXT_CTRL_ADDR,
	     0x02000004);
	HAL_MCR_WR(prAdapter, WF_WFDMA_HOST_DMA0_WPDMA_TX_RING4_EXT_CTRL_ADDR,
	     0x02400004);
	HAL_MCR_WR(prAdapter, WF_WFDMA_HOST_DMA0_WPDMA_TX_RING5_EXT_CTRL_ADDR,
	     0x02800004);
	HAL_MCR_WR(prAdapter, WF_WFDMA_HOST_DMA0_WPDMA_TX_RING6_EXT_CTRL_ADDR,
	     0x02c00004);

	HAL_MCR_WR(prAdapter, WF_WFDMA_HOST_DMA0_WPDMA_TX_RING16_EXT_CTRL_ADDR,
	     0x03400004);
	HAL_MCR_WR(prAdapter, WF_WFDMA_HOST_DMA0_WPDMA_TX_RING17_EXT_CTRL_ADDR,
	     0x03800004);

	/* reset dma idx */
	HAL_MCR_WR(prAdapter,
		WF_WFDMA_HOST_DMA0_WPDMA_RST_DTX_PTR_ADDR, 0xFFFFFFFF);
}

void mt7961ReadIntStatus(
	struct ADAPTER *prAdapter,
	uint32_t *pu4IntStatus)
{
	uint32_t u4RegValue = 0;
	struct GL_HIF_INFO *prHifInfo = &prAdapter->prGlueInfo->rHifInfo;
	struct BUS_INFO *prBusInfo = prAdapter->chip_info->bus_info;
	union WPDMA_INT_STA_STRUCT *prIntrStatus;
#if CFG_CHIP_RESET_SUPPORT
	uint32_t u4IntSta = 0;
#endif

	*pu4IntStatus = 0;
	prIntrStatus = (union WPDMA_INT_STA_STRUCT *)&u4RegValue;

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

	if (prAdapter->u4NoMoreRfb)
		*pu4IntStatus |= WHISR_RX0_DONE_INT;

	if (prAdapter->u4NoMoreRfb & BIT(RX_RING_EVT_IDX_1)) {
		prIntrStatus->field_conn2x_single.wfdma0_rx_done_0 = 1;
		DBGLOG(HAL, ERROR, "retry process RX_RING_EVT_IDX_1\n");
	}

	if (prAdapter->u4NoMoreRfb & BIT(RX_RING_DATA_IDX_0)) {
		prIntrStatus->field_conn2x_single.wfdma0_rx_done_2 = 1;
		DBGLOG(HAL, ERROR, "retry process RX_RING_DATA_IDX_0\n");
	}

	if (prAdapter->u4NoMoreRfb & BIT(WFDMA0_RX_RING_IDX_2)) {
		prIntrStatus->field_conn2x_single.wfdma0_rx_done_3 = 1;
		DBGLOG(HAL, ERROR, "retry process WFDMA0_RX_RING_IDX_2\n");
	}

	if (prAdapter->u4NoMoreRfb & BIT(WFDMA0_RX_RING_IDX_3)) {
		prIntrStatus->field_conn2x_single.wfdma0_rx_done_4 = 1;
		DBGLOG(HAL, ERROR, "retry process WFDMA0_RX_RING_IDX_3\n");
	}

	if (prAdapter->u4NoMoreRfb & BIT(WFDMA1_RX_RING_IDX_0)) {
		prIntrStatus->field_conn2x_single.wfdma0_rx_done_5 = 1;
		DBGLOG(HAL, ERROR, "retry process WFDMA1_RX_RING_IDX_0\n");
	}

	prHifInfo->u4IntStatus = u4RegValue;

	/* clear interrupt */
	HAL_MCR_WR(prAdapter,
		WF_WFDMA_HOST_DMA0_HOST_INT_STA_ADDR, u4RegValue);

#if (SW_WORKAROUND_FOR_WFDMA_ISSUE_HWITS00015048 == 1)
	if (u4RegValue & CONNAC2X_WFDMA_RX_DONE_INT5)
	HAL_MCR_WR(prAdapter,
		WF_WFDMA_HOST_DMA0_HOST_INT_STA_EXT_ADDR,
		(1 <<
		WF_WFDMA_HOST_DMA0_HOST_INT_STA_EXT_rx_done_int_sts_5_SHFT));
#endif

#if CFG_CHIP_RESET_SUPPORT
	/* WF WDT interrupt to host via PCIE */
	if (prAdapter->eWfsysResetState == WFSYS_RESET_STATE_IDLE &&
			prAdapter->chip_info->fgIsSupportL0p5Reset == TRUE) {
		HAL_MCR_RD(prAdapter, PCIE_MAC_IREG_ISTATUS_HOST_ADDR,
			&u4IntSta);

		if (u4IntSta & PCIE_MAC_IREG_ISTATUS_HOST_WDT_INT_MASK)
			*pu4IntStatus |= WHISR_WDT_INT;

		HAL_MCR_WR(prAdapter, PCIE_MAC_IREG_ISTATUS_HOST_ADDR,
			u4IntSta);
	}
#endif
}

#if CFG_SUPPORT_HOST_RX_WM_EVENT_FROM_PSE
uint8_t mt7961CheckPortForRxEventFromPse(IN struct ADAPTER *prAdapter,
				IN uint8_t u2Port)
{
	if (u2Port == RX_RING_EVT_IDX_1 &&
			prAdapter->fgIsFwDownloaded == TRUE)
		return WFDMA0_RX_RING_IDX_3;
	else
		return u2Port;
}
#endif
#endif /*_HIF_PCIE || _HIF_AXI */

void mt7961DumpSerDummyCR(
	struct ADAPTER *prAdapter)
{
	uint32_t u4MacVal = 0;

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

uint32_t mt7961GetFlavorVer(struct ADAPTER *prAdapter)
{
	uint32_t u4Val = 0;
	uint32_t flavor_ver = MT7961_A_DIE_7921_FLAVOR;

	if (prAdapter == NULL) {
		DBGLOG(INIT, ERROR, "prAdapter == NULL\n");
		return flavor_ver;
	}

	if (prAdapter->chip_info->u4ADieVer != 0xFFFFFFFF) {
		return prAdapter->chip_info->u4ADieVer;
	}

	if (prAdapter->fgIsFwDownloaded) {
		HAL_MCR_RD(prAdapter, MT7961_A_DIE_VER_ADDR, &u4Val);
	} else if (wlanAccessRegister(prAdapter, MT7961_A_DIE_VER_ADDR,
			&u4Val, 0, 0) != WLAN_STATUS_SUCCESS) {
		DBGLOG(INIT, ERROR, "get Bounding Info failed. set as 7921\n");
		u4Val = MT7961_A_DIE_7921;
	}

	if ((u4Val & MT7961_A_DIE_VER_BIT) == MT7961_A_DIE_7920)
		flavor_ver = MT7961_A_DIE_7920_FLAVOR;
	else
		flavor_ver = MT7961_A_DIE_7921_FLAVOR;

	prAdapter->chip_info->u4ADieVer = flavor_ver;

	return flavor_ver;
}

uint32_t mt7961GetFwVer(struct ADAPTER *prAdapter)
{
	uint32_t u4SwVer = 0;

	u4SwVer = nicGetChipSwVer() + 1;

	return u4SwVer;
}

void mt7961ConstructFirmwarePrio(struct GLUE_INFO *prGlueInfo,
	uint8_t **apucNameTable, uint8_t **apucName,
	uint8_t *pucNameIdx, uint8_t ucMaxNameIdx)
{
	uint8_t sub_idx = 0;
	uint8_t flavor_ver = 0;
	uint32_t chip_id = 0;
	struct ADAPTER *prAdapter = NULL;
	struct mt66xx_chip_info *prChipInfo = NULL;

	if (prGlueInfo == NULL) {
		DBGLOG(INIT, ERROR, "prGlueInfo is NULL.\n");
		return;
	}

	if ((pucNameIdx == NULL) || (apucName == NULL)) {
		DBGLOG(INIT, ERROR, "pucNameIdx or apucName are NULL.\n");
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
	flavor_ver = mt7961GetFlavorVer(prAdapter);

	for (sub_idx = 0; apucNameTable[sub_idx]; sub_idx++) {
		if (((*pucNameIdx) + 3) < ucMaxNameIdx) {
			/* Type 1. WIFI_RAM_CODE_MTxxxx_x.bin */
			if (snprintf(*(apucName + (*pucNameIdx)),
			    CFG_FW_NAME_MAX_LEN, "%s%x_%x.bin",
			    apucNameTable[sub_idx], chip_id, flavor_ver) < 0) {
				DBGLOG(INIT, ERROR, "gen type 1 fail\n");
				return;
			}
			(*pucNameIdx) += 1;

			/* Type 2. WIFI_RAM_CODE_MTxxxx_x */
			if (snprintf(*(apucName + (*pucNameIdx)),
			    CFG_FW_NAME_MAX_LEN, "%s%x_%x",
			    apucNameTable[sub_idx], chip_id, flavor_ver) < 0) {
				DBGLOG(INIT, ERROR, "gen type 2 fail\n");
				return;
			}
			(*pucNameIdx) += 1;

			/* Type 3. WIFI_RAM_CODE_MTxxxx.bin */
			if (snprintf(*(apucName + (*pucNameIdx)),
					CFG_FW_NAME_MAX_LEN, "%s%x.bin",
					apucNameTable[sub_idx], chip_id) < 0) {
				DBGLOG(INIT, ERROR, "gen type 3 fail\n");
				return;
			}
			(*pucNameIdx) += 1;

			/* Type 4. WIFI_RAM_CODE_MTxxxx */
			if (snprintf(*(apucName + (*pucNameIdx)),
				CFG_FW_NAME_MAX_LEN, "%s%x",
					apucNameTable[sub_idx], chip_id) < 0) {
				DBGLOG(INIT, ERROR, "gen type 4 fail\n");
				return;
			}
			(*pucNameIdx) += 1;
		} else {
			/* the table is not large enough */
			DBGLOG(INIT, ERROR,
				"kalFirmwareImageMapping >> file name array is not enough.\n");
			ASSERT(0);
		}
	}
}

void mt7961ConstructPatchName(struct GLUE_INFO *prGlueInfo,
	uint8_t **apucName, uint8_t *pucNameIdx)
{
	uint32_t u4FlavorVer;
	struct ADAPTER *prAdapter = NULL;
	struct mt66xx_chip_info *prChipInfo = NULL;

	if (prGlueInfo == NULL) {
		DBGLOG(INIT, ERROR, "prGlueInfo is NULL.\n");
		return;
	}

	if ((pucNameIdx == NULL) || (apucName == NULL)) {
		DBGLOG(INIT, ERROR, "pucNameIdx or apucName are NULL.\n");
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

	u4FlavorVer = mt7961GetFlavorVer(prAdapter);

	if (snprintf(apucName[(*pucNameIdx)],
		CFG_FW_NAME_MAX_LEN, "WIFI_MT%x_patch_mcu_%x_%x_hdr.bin",
		prChipInfo->chip_id, u4FlavorVer,
		mt7961GetFwVer(prAdapter)) < 0)
		DBGLOG(INIT, ERROR, "gen Patch File Name fail\n");
}

#if defined(_HIF_USB)
void mt7961Connac2xWfdmaInitForUSB(
	struct ADAPTER *prAdapter,
	struct mt66xx_chip_info *prChipInfo)
{
	/* Prevent USB first inband cmd read timeout due
	*  to endpoint not match.
	*/
	if (!prAdapter->fgIsFwDownloaded)
		asicConnac2xUsbRxEvtEP4Setting(prAdapter, TRUE);
}

uint8_t mt7961Connac2xUsbEventEpDetected(IN struct ADAPTER *prAdapter)
{
	struct GL_HIF_INFO *prHifInfo = NULL;
	struct GLUE_INFO *prGlueInfo = NULL;

	ASSERT(FALSE == 0);
	prGlueInfo = prAdapter->prGlueInfo;
	prHifInfo = &prGlueInfo->rHifInfo;

	if (prHifInfo->fgEventEpDetected == FALSE) {
		uint32_t u4Value = 0;
		uint8_t ucCurEp; /* current event packet from which endpoint */

		/* MT7961 use EP4IN for event packets */
		prHifInfo->fgEventEpDetected = TRUE;
		prHifInfo->eEventEpType = EVENT_EP_TYPE_DATA_EP;

		HAL_MCR_RD(prAdapter, CONNAC2X_WFDMA_HOST_CONFIG_ADDR,
			&u4Value);

		if (u4Value & CONNAC2X_WFDMA_HOST_CONFIG_USB_RXEVT_EP4_EN) {
			ucCurEp = USB_DATA_EP_IN;
		} else {
			ucCurEp = USB_EVENT_EP_IN;
			asicConnac2xUsbRxEvtEP4Setting(prAdapter, TRUE);
		}

		return ucCurEp;
	}

	if (prHifInfo->eEventEpType == EVENT_EP_TYPE_DATA_EP)
		return USB_DATA_EP_IN;
	else
		return USB_EVENT_EP_IN;
}

uint16_t mt7961Connac2xUsbRxByteCount(
	struct ADAPTER *prAdapter,
	struct BUS_INFO *prBusInfo,
	uint8_t *pRXD)
{

	uint16_t u2RxByteCount;
	uint8_t ucPacketType;

	ucPacketType = HAL_MAC_CONNAC2X_RX_STATUS_GET_PKT_TYPE(
		(struct HW_MAC_CONNAC2X_RX_DESC *)pRXD);
	u2RxByteCount = HAL_MAC_CONNAC2X_RX_STATUS_GET_RX_BYTE_CNT(
		(struct HW_MAC_CONNAC2X_RX_DESC *)pRXD);

	/* According to Barry's rule, it can be summarized as below formula:
	 * 1. packets from WFDMA
	   -> RX padding for 4B alignment
	 * 2. packets from UMAC
	 * -> RX padding for 8B alignment first,
				then extra 4B padding
	 * 3. MT7961 Rx data packets and event packets should are all from UMAC
	 *    because of HW limitation
	 */
#if CFG_SUPPORT_HOST_RX_WM_EVENT_FROM_PSE
	u2RxByteCount = ALIGN_8(u2RxByteCount) + LEN_USB_RX_PADDING_CSO;
#else
	if ((ucPacketType == RX_PKT_TYPE_RX_DATA) ||
		(ucPacketType == RX_PKT_TYPE_RX_REPORT))
		u2RxByteCount = ALIGN_8(u2RxByteCount)
			+ LEN_USB_RX_PADDING_CSO;
	else
		u2RxByteCount = ALIGN_4(u2RxByteCount);
#endif

	return u2RxByteCount;
}
#endif /*_HIF_USB */

#if CFG_SUPPORT_WIFI_DL_BT_PATCH
void mt7961ConstructBtPatchName(struct GLUE_INFO *prGlueInfo,
	uint8_t **apucName, uint8_t *pucNameIdx)
{
	uint32_t flavor_ver;
	struct ADAPTER *prAdapter = NULL;
	struct mt66xx_chip_info *prChipInfo = NULL;

	if (prGlueInfo == NULL) {
		DBGLOG(INIT, ERROR, "prGlueInfo is NULL.\n");
		return;
	}

	if ((pucNameIdx == NULL) || (apucName == NULL)) {
		DBGLOG(INIT, ERROR, "pucNameIdx or apucName are NULL.\n");
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
	flavor_ver = mt7961GetFlavorVer(prAdapter);
	if (snprintf(apucName[(*pucNameIdx)],
		CFG_FW_NAME_MAX_LEN, "BT_RAM_CODE_MT%x_%x_%x_hdr.bin",
		prChipInfo->chip_id, flavor_ver,
		mt7961GetFwVer(prAdapter)) < 0)
		DBGLOG(INIT, ERROR, "gen BT Patch File Name fail\n");
}

uint32_t wlanBtPatchSendSemaControl(IN struct ADAPTER *prAdapter,
				    IN uint32_t u4Addr,
				    OUT uint8_t *pucSeqNum)
{
	struct mt66xx_chip_info *prChipInfo = NULL;
	struct CMD_INFO *prCmdInfo;
	uint32_t u4Status = WLAN_STATUS_SUCCESS;
	struct INIT_CMD_BT_PATCH_SEMA_CTRL *prBtPatchSemaCtrl;

	if (prAdapter == NULL) {
		DBGLOG(INIT, ERROR, "prAdapter is NULL\n");
		return WLAN_STATUS_FAILURE;
	}

	prChipInfo = prAdapter->chip_info;
	if (prChipInfo == NULL) {
		DBGLOG(INIT, ERROR, "prChipInfo is NULL\n");
		return WLAN_STATUS_FAILURE;
	}

	/* 1. Allocate CMD Info Packet and its Buffer. */
	prCmdInfo = cmdBufAllocateCmdInfo(prAdapter,
		sizeof(struct INIT_HIF_TX_HEADER) +
		sizeof(struct INIT_HIF_TX_HEADER_PENDING_FOR_HW_32BYTES) +
		sizeof(struct INIT_CMD_BT_PATCH_SEMA_CTRL));

	if (!prCmdInfo) {
		DBGLOG(INIT, ERROR, "Allocate CMD_INFO_T ==> FAILED.\n");
		return WLAN_STATUS_FAILURE;
	}

	prCmdInfo->u2InfoBufLen = sizeof(struct INIT_HIF_TX_HEADER) +
		sizeof(struct INIT_HIF_TX_HEADER_PENDING_FOR_HW_32BYTES) +
		sizeof(struct INIT_CMD_BT_PATCH_SEMA_CTRL);

	prCmdInfo->ucCID = INIT_CMD_ID_BT_PATCH_SEMAPHORE_CONTROL;
	NIC_FILL_CMD_TX_HDR(prAdapter,
		prCmdInfo->pucInfoBuffer,
		prCmdInfo->u2InfoBufLen,
		prCmdInfo->ucCID,
		INIT_CMD_PDA_PACKET_TYPE_ID,
		pucSeqNum, FALSE,
		(void **)&prBtPatchSemaCtrl, TRUE, 0, S2D_INDEX_CMD_H2N,
		FALSE);

	/* Setup DOWNLOAD_BUF */
	kalMemZero(prBtPatchSemaCtrl,
		   sizeof(struct INIT_CMD_BT_PATCH_SEMA_CTRL));
	prBtPatchSemaCtrl->ucGetSemaphore = PATCH_GET_SEMA_CONTROL;
	prBtPatchSemaCtrl->u4Addr = u4Addr;

	/* 4. Send FW_Download command */
	if (nicTxInitCmd(prAdapter, prCmdInfo,
			 prChipInfo->u2TxInitCmdPort) != WLAN_STATUS_SUCCESS) {
		u4Status = WLAN_STATUS_FAILURE;
		DBGLOG(INIT, ERROR, "send Get BT Patch Sema fail\n");
	} else
		DBGLOG(INIT, INFO, "send Get BT Patch Sema success\n");

	/* 5. Free CMD Info Packet. */
	cmdBufFreeCmdInfo(prAdapter, prCmdInfo);

	return u4Status;
}

uint32_t wlanBtPatchRecvSemaResp(IN struct ADAPTER *prAdapter,
				 IN uint8_t ucCmdSeqNum,
				 OUT uint8_t *pucPatchStatus,
				 OUT uint32_t *u4RemapAddr)
{
	struct mt66xx_chip_info *prChipInfo;
	uint8_t *aucBuffer;
	uint32_t u4EventSize;
	struct INIT_WIFI_EVENT *prInitEvent;
	struct INIT_EVENT_BT_PATCH_SEMA_CTRL *prEventCmdResult;
	uint32_t u4RxPktLength = 0;
	uint32_t u4Status = WLAN_STATUS_FAILURE;

	ASSERT(prAdapter);
	prChipInfo = prAdapter->chip_info;

	if (kalIsCardRemoved(prAdapter->prGlueInfo) == TRUE
	    || fgIsBusAccessFailed == TRUE)
		return WLAN_STATUS_FAILURE;

	u4EventSize = prChipInfo->rxd_size + prChipInfo->init_event_size +
		sizeof(struct INIT_EVENT_BT_PATCH_SEMA_CTRL);
	aucBuffer = kalMemAlloc(u4EventSize, PHY_MEM_TYPE);
	if (aucBuffer == NULL) {
		DBGLOG(INIT, ERROR, "Alloc CMD buffer failed\n");
		return WLAN_STATUS_FAILURE;
	}

	if (nicRxWaitResponse(prAdapter, 0, aucBuffer, u4EventSize,
			      &u4RxPktLength) != WLAN_STATUS_SUCCESS) {

		DBGLOG(INIT, WARN, "Wait patch semaphore response fail\n");
		goto out;
	}

	prInitEvent = (struct INIT_WIFI_EVENT *)
		(aucBuffer + prChipInfo->rxd_size);

	if (prInitEvent->ucEID != INIT_EVENT_ID_BT_PATCH_SEMA_CTRL) {
		DBGLOG(INIT, WARN, "Unexpected EVENT ID, get 0x%0x\n",
		       prInitEvent->ucEID);
		goto out;
	}

	/* BT always response prInitEvent->ucSeqNum=0 */

	prEventCmdResult = (struct INIT_EVENT_BT_PATCH_SEMA_CTRL *)
		prInitEvent->aucBuffer;

	*pucPatchStatus = prEventCmdResult->ucStatus;
	*u4RemapAddr = prEventCmdResult->u4RemapAddr;

	u4Status = WLAN_STATUS_SUCCESS;
out:
	kalMemFree(aucBuffer, PHY_MEM_TYPE, u4EventSize);

	return u4Status;
}

uint32_t wlanImageSectionGetBtPatchInfo(IN struct ADAPTER *prAdapter,
	IN void *pvFwImageMapFile, IN uint32_t u4FwImageFileLength,
	OUT uint32_t *pu4DataMode, OUT struct patch_dl_target *target)
{
	struct PATCH_FORMAT_V2_T *prPatchFormat;
	struct PATCH_GLO_DESC *glo_desc;
	struct PATCH_SEC_MAP *sec_map;
	struct patch_dl_buf *region;
	uint32_t section_type;
	uint32_t num_of_region, i;
	uint32_t u4Status = WLAN_STATUS_FAILURE;
	uint8_t *img_ptr;
	uint8_t aucBuffer[32];
	uint32_t sec_info = 0;

	/* patch header */
	img_ptr = pvFwImageMapFile;
	prPatchFormat = (struct PATCH_FORMAT_V2_T *)img_ptr;

	/* Dump image information */
	kalMemZero(aucBuffer, 32);
	kalStrnCpy(aucBuffer, prPatchFormat->aucPlatform, 4);
	DBGLOG(INIT, INFO,
	       "PATCH INFO: platform[%s] HW/SW ver[0x%04X] ver[0x%04X]\n",
	       aucBuffer, prPatchFormat->u4SwHwVersion,
	       prPatchFormat->u4PatchVersion);

	kalStrnCpy(aucBuffer, prPatchFormat->aucBuildDate, 16);
	DBGLOG(INIT, INFO, "date[%s]\n", aucBuffer);

	if (prPatchFormat->u4PatchVersion != PATCH_VERSION_MAGIC_NUM) {
		DBGLOG(INIT, ERROR, "BT Patch format isn't V2\n");
		return WLAN_STATUS_FAILURE;
	}

	/* global descriptor */
	img_ptr += sizeof(struct PATCH_FORMAT_V2_T);
	glo_desc = (struct PATCH_GLO_DESC *)img_ptr;
	num_of_region = le2cpu32(glo_desc->section_num);
	DBGLOG(INIT, INFO,
			"\tPatch ver: 0x%x, Section num: 0x%x, subsys: 0x%x\n",
			glo_desc->patch_ver,
			num_of_region,
			le2cpu32(glo_desc->subsys));

	/* section map */
	img_ptr += sizeof(struct PATCH_GLO_DESC);

	/* XXX: Expect that PATCH only occupy one section */
	target->num_of_region = 1;
	target->patch_region = (struct patch_dl_buf *)kalMemAlloc(
				sizeof(struct patch_dl_buf), PHY_MEM_TYPE);

	if (!target->patch_region) {
		DBGLOG(INIT, WARN, "No memory to allocate.\n");
		return WLAN_STATUS_FAILURE;
	}

	region = &target->patch_region[0];
	region->img_ptr = NULL;
	for (i = 0; i < num_of_region; i++) {
		sec_map = (struct PATCH_SEC_MAP *)img_ptr;
		img_ptr += sizeof(struct PATCH_SEC_MAP);

		section_type = le2cpu32(sec_map->section_type);

		if ((section_type & PATCH_SEC_TYPE_MASK) !=
		     PATCH_SEC_TYPE_BIN_INFO)
			continue;

		region->bin_type = le2cpu32(sec_map->bin_info_spec.bin_type);
		/* only handle BT Patch */
		if (region->bin_type != FW_SECT_BINARY_TYPE_BT_PATCH)
			continue;

		region->img_dest_addr =
			le2cpu32(sec_map->bin_info_spec.dl_addr);
		/* PDA needs 16-byte aligned length */
		region->img_size =
			le2cpu32(sec_map->bin_info_spec.dl_size) +
			le2cpu32(sec_map->bin_info_spec.align_len);
		if (!(region->img_size % 16))
			DBGLOG(INIT, WARN,
			       "BT Patch is not 16-byte aligned\n");
		region->img_ptr = pvFwImageMapFile +
			le2cpu32(sec_map->section_offset);
		sec_info = le2cpu32(sec_map->bin_info_spec.sec_info);

		DBGLOG(INIT, INFO, "BT Patch addr=0x%x: size=%d, ptr=0x%p\n",
			region->img_dest_addr, region->img_size,
			region->img_ptr);

		u4Status = WLAN_STATUS_SUCCESS;
	}

	*pu4DataMode = wlanGetPatchDataModeV2(prAdapter, sec_info);

	if (region->img_ptr == NULL) {
		DBGLOG(INIT, ERROR, "Can't find the BT Patch\n");
		kalMemFree(target->patch_region, PHY_MEM_TYPE,
			sizeof(struct patch_dl_buf));
	}

	return u4Status;
}


int32_t wlanBtPatchIsDownloaded(IN struct ADAPTER *prAdapter,
				 IN uint32_t u4DestAddr, OUT uint32_t *u4BtAddr)
{
	uint8_t ucSeqNum = 0, ucPatchStatus;
	uint32_t rStatus;
	uint32_t u4Count;
	uint32_t u4RemapAddr;
	int32_t s4RetStatus = -1;

	ucPatchStatus = PATCH_STATUS_NO_SEMA_NEED_PATCH;
	u4Count = 0;

	while (ucPatchStatus == PATCH_STATUS_NO_SEMA_NEED_PATCH) {
		if (u4Count)
			kalMdelay(100);

		rStatus = wlanBtPatchSendSemaControl(prAdapter, u4DestAddr,
							&ucSeqNum);
		if (rStatus != WLAN_STATUS_SUCCESS) {
			DBGLOG(INIT, WARN,
			       "Send patch SEMA control CMD failed!!\n");
			goto out;
		}

		rStatus = wlanBtPatchRecvSemaResp(prAdapter, ucSeqNum,
						&ucPatchStatus, &u4RemapAddr);
		if (rStatus != WLAN_STATUS_SUCCESS) {
			DBGLOG(INIT, WARN,
			       "Recv patch SEMA control EVT failed!!\n");
			goto out;
		}

		u4Count++;

		if (u4Count > 50) {
			DBGLOG(INIT, WARN, "Patch status check timeout!!\n");
			goto out;
		}
	}

	if (ucPatchStatus != PATCH_STATUS_NO_NEED_TO_PATCH)
		*u4BtAddr = u4RemapAddr;

	s4RetStatus = (ucPatchStatus == PATCH_STATUS_NO_NEED_TO_PATCH);

out:
	return s4RetStatus;
}

uint32_t mt7961DownloadBtPatch(IN struct ADAPTER *prAdapter)
{
	uint32_t u4FwSize = 0;
	uint32_t u4Status = WLAN_STATUS_FAILURE;
	uint32_t u4DataMode;
	uint32_t u4RemapAddr;
	int32_t s4BtPatchCheck;
	struct patch_dl_target target;
	struct patch_dl_buf *region = NULL;
	void *prFwBuffer = NULL;

#if CFG_SUPPORT_COMPRESSION_FW_OPTION
	#pragma message("WARN: Download BT Patch doesn't support COMPRESSION")
#endif
#if CFG_DOWNLOAD_DYN_MEMORY_MAP
	#pragma message("WARN: Download BT Patch doesn't support DYN_MEM_MAP")
#endif
#if CFG_ROM_PATCH_NO_SEM_CTRL
	#pragma message("WARN: Download BT Patch doesn't support NO_SEM_CTRL")
#endif

	if (!prAdapter)
		return WLAN_STATUS_FAILURE;

	DBGLOG(INIT, INFO, "BT Patch download start\n");

	/* Always check BT Patch Download for L0.5 reset case */

	/* refer from wlanImageSectionDownloadStage */

	/* step.1 open the PATCH file */
	kalFirmwareImageMapping(prAdapter->prGlueInfo, &prFwBuffer,
				&u4FwSize, IMG_DL_IDX_BT_PATCH);
	if (prFwBuffer == NULL) {
		DBGLOG(INIT, WARN, "FW[%u] load error!\n",
		       IMG_DL_IDX_BT_PATCH);
		return WLAN_STATUS_FAILURE;
	}

	/* step 2. get Addr info. Refer from : wlanImageSectionDownloadStage */
	u4Status = wlanImageSectionGetBtPatchInfo(prAdapter,
			prFwBuffer, u4FwSize, &u4DataMode, &target);

	if (u4Status != WLAN_STATUS_SUCCESS) {
		DBGLOG(INIT, ERROR, "Can't find the BT Patch Section\n");
		goto out;
	}

	/* step 3. check BT doesn't download PATCH */
	region = &target.patch_region[0];
	s4BtPatchCheck = wlanBtPatchIsDownloaded(prAdapter,
				region->img_dest_addr, &u4RemapAddr);
	if (s4BtPatchCheck < 0) {
		DBGLOG(INIT, INFO, "Get BT Semaphore Fail\n");
		u4Status =  WLAN_STATUS_FAILURE;
		goto out;
	} else if (s4BtPatchCheck == 1) {
		DBGLOG(INIT, INFO, "No need to download patch\n");
		u4Status =  WLAN_STATUS_SUCCESS;
		goto out;
	}

	region->img_dest_addr = u4RemapAddr;

	/* step 4. download BT patch */
	u4Status = wlanDownloadSectionV2(prAdapter, u4DataMode,
					IMG_DL_IDX_BT_PATCH, &target);
	if (u4Status != WLAN_STATUS_SUCCESS) {
		DBGLOG(INIT, ERROR, "BT Patch download Fail\n");
		goto out;
	}

	/* step 5. send INIT_CMD_PATCH_FINISH */
	u4Status = wlanPatchSendComplete(prAdapter, PATCH_FNSH_TYPE_BT);

	if (u4Status != WLAN_STATUS_SUCCESS)
		DBGLOG(INIT, ERROR, "Send INIT_CMD_PATCH_FINISH Fail\n");
	else
		DBGLOG(INIT, INFO, "BT Patch download success\n");

out:
	if (target.patch_region != NULL) {
		/* This case is that the BT patch isn't downloaded this time.
		 * The original free action is in wlanDownloadSectionV2().
		 */
		kalMemFree(target.patch_region, PHY_MEM_TYPE,
			sizeof(struct patch_dl_buf) * target.num_of_region);
		target.patch_region = NULL;
		target.num_of_region = 0;
	}

	kalFirmwareImageUnmapping(prAdapter->prGlueInfo, NULL, prFwBuffer);

	return u4Status;
}
#endif /* CFG_SUPPORT_WIFI_DL_BT_PATCH */

uint32_t mt7961ConstructBufferBinFileName(struct ADAPTER *prAdapter,
					  uint8_t *aucEeprom)
{
	struct mt66xx_chip_info *prChipInfo;
	uint8_t flavor_ver = 0;

	if (prAdapter == NULL) {
		DBGLOG(INIT, ERROR, "prAdapter == NULL\n");
		return WLAN_STATUS_FAILURE;
	}

	if (aucEeprom == NULL) {
		DBGLOG(INIT, ERROR, "aucEeprom == NULL\n");
		return WLAN_STATUS_FAILURE;
	}

	prChipInfo = prAdapter->chip_info;

	if (prChipInfo == NULL) {
		DBGLOG(INIT, ERROR, "prChipInfo == NULL\n");
		return WLAN_STATUS_FAILURE;
	}

	flavor_ver = mt7961GetFlavorVer(prAdapter);

	if (snprintf(aucEeprom, 32, "EEPROM_MT%x_%x.bin",
		 prChipInfo->chip_id, flavor_ver) < 0) {
		DBGLOG(INIT, ERROR, "gen buffer bin file name fail\n");
		return WLAN_STATUS_FAILURE;
	}

	return WLAN_STATUS_SUCCESS;
}

u_int8_t mt7961GetRxvSrc(struct ADAPTER *prAdapter)
{
	if (nicIsEcoVerEqualOrLaterTo(prAdapter, ECO_VER_2))
		return FALSE;	/* From RXD */
	else
		return TRUE;	/* From RX_RPT */
}

u_int8_t mt7961GetRxDbgInfoSrc(struct ADAPTER *prAdapter)
{
	if (nicIsEcoVerEqualOrLaterTo(prAdapter, ECO_VER_2))
		return TRUE;	/* From group3 */
	else
		return FALSE;	/* From group5 */
}

#if (CFG_COALESCING_INTERRUPT == 1)
uint32_t mt7961setWfdmaCoalescingInt(struct ADAPTER *prAdapter,
					    u_int8_t fgEnable)
{
	uint32_t u4Addr;
	uint32_t u4Val = 0;
	struct BUS_INFO *prBusInfo;

	prBusInfo = prAdapter->chip_info->bus_info;
	u4Addr = WF_WFDMA_HOST_DMA0_WPDMA_INT_RX_PRI_SEL;
	HAL_MCR_RD(prAdapter, u4Addr, &u4Val);
	/*
	 * RX_RING_DATA_IDX_0   (RX_Ring2) - Band0 Rx Data
	 * WFDMA0_RX_RING_IDX_2 (RX_Ring3) - Band1 Rx Data
	*/

	u4Val &=
		~(WF_WFDMA_HOST_DMA0_WPDMA_INT_RX_RING2_PRI_SEL_MASK |
		WF_WFDMA_HOST_DMA0_WPDMA_INT_RX_RING3_PRI_SEL_MASK);

	if (fgEnable) {
		u4Val |=
			(WF_WFDMA_HOST_DMA0_WPDMA_INT_RX_RING2_PRI_SEL_MASK |
			WF_WFDMA_HOST_DMA0_WPDMA_INT_RX_RING3_PRI_SEL_MASK);
	}
	HAL_MCR_WR(prAdapter, u4Addr, u4Val);

	u4Val = 0;
	u4Addr =
	WF_WFDMA_EXT_WRAP_CSR_WFDMA_HOST_CONFIG_pcie_dly_rx_int_en_ADDR;
	HAL_MCR_RD(prAdapter, u4Addr, &u4Val);

	u4Val &=
	~WF_WFDMA_EXT_WRAP_CSR_WFDMA_HOST_CONFIG_pcie_dly_rx_int_en_MASK;
	if (fgEnable) {
		u4Val |=
		WF_WFDMA_EXT_WRAP_CSR_WFDMA_HOST_CONFIG_pcie_dly_rx_int_en_MASK;
	}
	HAL_MCR_WR(prAdapter, u4Addr, u4Val);

	return WLAN_STATUS_SUCCESS;
}
#endif

void mt7961SerInit(IN struct ADAPTER *prAdapter,
		   IN const u_int8_t fgAtResetFlow)
{
	if (!fgAtResetFlow) {
#if defined(_HIF_SDIO)
		/* Since MT7961 SDIO hw doesn't support L1 reset, if user
		 * enables L1 reset somehow, then driver explicitly reset this
		 * flag to monitor only.
		 */
		if (prAdapter->rWifiVar.eEnableSerL1 == FEATURE_OPT_SER_ENABLE)
			prAdapter->rWifiVar.eEnableSerL1 =
				FEATURE_OPT_SER_MONITOR;
#endif
	}
}

#if defined(_HIF_PCIE)
bool pcie_show_csr_info(IN struct ADAPTER *prAdapter)
{
	uint32_t i = 0, u4Value = 0;
	bool fgIsDriverOwn = false;
	struct mt66xx_chip_info *prChipInfo = NULL;

	if (!prAdapter) {
		DBGLOG(HAL, INFO, "prAdapter == NULL!\n");
		return FALSE;
	}
	prChipInfo = prAdapter->chip_info;

	DBGLOG(HAL, INFO, "PCIE Host CSR Configuration Info:\n\n");

	/* clk_detect: write 0x7C06_0000 bit0 = 0x1, read 0x7C06_0000*/
	HAL_MCR_WR(prAdapter, CONNAC2x_CONN_CFG_ON_BUS_MCU_STAT_ADDR,
		CONNAC2x_CONN_CFG_ON_MCU_STAT_CLK_DETECT_BUS_CLR_PULSE_MASK);
	HAL_MCR_RD(prAdapter, CONNAC2x_CONN_CFG_ON_BUS_MCU_STAT_ADDR, &u4Value);
	DBGLOG(HAL, INFO, "Get Bus Stat: 0x%08x = 0x%08x\n",
		CONNAC2x_CONN_CFG_ON_BUS_MCU_STAT_ADDR, u4Value);

	/* bus timeout irq idx: read 0x7C06_014C*/
	HAL_MCR_RD(prAdapter,
		CONNAC2x_CONN_CFG_ON_CONN_INFRA_ON_BUS_TIMEOUT_IRQ_ADDR, &u4Value);
	DBGLOG(HAL, INFO, "conn_infra on bus timeout irq: 0x%08x = 0x%08x\n",
		CONNAC2x_CONN_CFG_ON_CONN_INFRA_ON_BUS_TIMEOUT_IRQ_ADDR, u4Value);

	/* read own status */
	HAL_MCR_RD(prAdapter, CONNAC2x_CONN_CFG_ON_WF_BAND0_LPCTL_ADDR, &u4Value);
	DBGLOG(HAL, INFO, "Driver own info: 0x%08x = 0x%08x\n",
		CONNAC2x_CONN_CFG_ON_WF_BAND0_LPCTL_ADDR, u4Value);
	fgIsDriverOwn = ((u4Value &
		CONNAC2x_CONN_CFG_ON_HOST_WF_B0_AP_OWNER_STATE_SYNC_MASK) == 0);

	/* dump current PC*/
	if (prChipInfo &&
		prChipInfo->prDebugOps &&
		prChipInfo->prDebugOps->show_mcu_debug_info) {
		for (i = 0; i < 5; i++) {
			prChipInfo->prDebugOps->show_mcu_debug_info(prAdapter, NULL, 0,
				DBG_MCU_DBG_CURRENT_PC, NULL);
		}
	}

	HAL_MCR_RD(prAdapter, CONNAC2x_CONN_CFG_ON_WF_TOP_RGU_ON_DEBUG_PORT_ADDR, &u4Value);
	DBGLOG(HAL, INFO, "RGU Info: 0x%08x = 0x%08x\n",
		CONNAC2x_CONN_CFG_ON_WF_TOP_RGU_ON_DEBUG_PORT_ADDR, u4Value);

	HAL_MCR_RD(prAdapter, CONNAC2x_CONN_CFG_ON_WF_MAILBOX_DBG_ADDR, &u4Value);
	DBGLOG(HAL, INFO, "WF Mailbox[0]: 0x%08x = 0x%08x\n",
		CONNAC2x_CONN_CFG_ON_WF_MAILBOX_DBG_ADDR, u4Value);

	HAL_MCR_RD(prAdapter, CONNAC2x_CONN_CFG_ON_N13_MCU_MAILBOX_DBG_ADDR, &u4Value);
	DBGLOG(HAL, INFO, "N13 MCU Mailbox[1]: 0x%08x = 0x%08x\n",
		CONNAC2x_CONN_CFG_ON_N13_MCU_MAILBOX_DBG_ADDR, u4Value);

	/* clear FW own and check own status again*/
	HAL_MCR_WR(prAdapter, CONNAC2x_CONN_CFG_ON_WF_BAND0_LPCTL_ADDR,
		CONNAC2x_CONN_CFG_ON_WF_B0_AP_HOST_CLR_FW_OWN_HS_PULSE_MASK);
	kalUdelay(1);
	HAL_MCR_RD(prAdapter, CONNAC2x_CONN_CFG_ON_WF_BAND0_LPCTL_ADDR, &u4Value);
	DBGLOG(HAL, INFO, "Driver own info: 0x%08x = 0x%08x\n",
		CONNAC2x_CONN_CFG_ON_WF_BAND0_LPCTL_ADDR, u4Value);

	fgIsDriverOwn = (u4Value & CONNAC2x_CONN_CFG_ON_HOST_WF_B0_AP_OWNER_STATE_SYNC_MASK) == 0;

	/* read HW ID in off doamain. */
	HAL_MCR_RD(prAdapter, CONN_CFG_CONN_HW_VER_ADDR, &u4Value);
	DBGLOG(HAL, INFO, "HW ID: 0x%08x = 0x%08x\n",
		CONN_CFG_CONN_HW_VER_ADDR, u4Value);

	return fgIsDriverOwn;

}
#endif

#ifdef MT7961
struct BUS_INFO mt7961_bus_info = {
#if defined(_HIF_PCIE) || defined(_HIF_AXI)
	.top_cfg_base = MT7961_TOP_CFG_BASE,

	/* host_dma0 for TXP */
	.host_dma0_base = WF_WFDMA_HOST_DMA0_BASE,
	.host_int_status_addr = WF_WFDMA_HOST_DMA0_HOST_INT_STA_ADDR,

	.host_int_txdone_bits =
		(CONNAC2X_WFDMA_TX_DONE_INT0 | CONNAC2X_WFDMA_TX_DONE_INT1 |
		CONNAC2X_WFDMA_TX_DONE_INT2 | CONNAC2X_WFDMA_TX_DONE_INT3 |
		CONNAC2X_WFDMA_TX_DONE_INT4 | CONNAC2X_WFDMA_TX_DONE_INT5 |
		CONNAC2X_WFDMA_TX_DONE_INT6 | CONNAC2X_WFDMA_TX_DONE_INT16 |
		CONNAC2X_WFDMA_TX_DONE_INT17),
	.host_int_rxdone_bits =
		(CONNAC2X_WFDMA_RX_DONE_INT0 | CONNAC2X_WFDMA_RX_DONE_INT0 |
		 CONNAC2X_WFDMA_RX_DONE_INT2 | CONNAC2X_WFDMA_RX_DONE_INT3 |
		 CONNAC2X_WFDMA_RX_DONE_INT4 | CONNAC2X_WFDMA_RX_DONE_INT5),

	.host_tx_ring_base = WF_WFDMA_HOST_DMA0_WPDMA_TX_RING0_CTRL0_ADDR,
	.host_tx_ring_ext_ctrl_base =
		WF_WFDMA_HOST_DMA0_WPDMA_TX_RING0_EXT_CTRL_ADDR,
	.host_tx_ring_cidx_addr = WF_WFDMA_HOST_DMA0_WPDMA_TX_RING0_CTRL2_ADDR,
	.host_tx_ring_didx_addr = WF_WFDMA_HOST_DMA0_WPDMA_TX_RING0_CTRL3_ADDR,
	.host_tx_ring_cnt_addr = WF_WFDMA_HOST_DMA0_WPDMA_TX_RING0_CTRL1_ADDR,

	.host_rx_ring_base = WF_WFDMA_HOST_DMA0_WPDMA_RX_RING0_CTRL0_ADDR,
	.host_rx_ring_ext_ctrl_base =
		WF_WFDMA_HOST_DMA0_WPDMA_RX_RING0_EXT_CTRL_ADDR,
	.host_rx_ring_cidx_addr = WF_WFDMA_HOST_DMA0_WPDMA_RX_RING0_CTRL2_ADDR,
	.host_rx_ring_didx_addr = WF_WFDMA_HOST_DMA0_WPDMA_RX_RING0_CTRL3_ADDR,
	.host_rx_ring_cnt_addr = WF_WFDMA_HOST_DMA0_WPDMA_RX_RING0_CTRL1_ADDR,

	.bus2chip = mt7961_bus2chip_cr_mapping,
	.bus2chip_tbl_size = MT7961_BUS2CHIP_MEM_MAP_TBLE_SIZE,
	.max_static_map_addr = 0x000f0000,

	.tx_ring_fwdl_idx = CONNAC2X_FWDL_TX_RING_IDX,
	.tx_ring_cmd_idx = CONNAC2X_CMD_TX_RING_IDX,
	.tx_ring0_data_idx = 0,
	.tx_ring1_data_idx = 1,
	.fw_own_clear_addr = CONNAC2X_BN0_IRQ_STAT_ADDR,
	.fw_own_clear_bit = PCIE_LPCR_FW_CLR_OWN,
	.fgCheckDriverOwnInt = FALSE,
	.u4DmaMask = 32,
	.pdmaSetup = asicConnac2xWpdmaConfig,
	.pdmaStop = asicConnac2xWfdmaStop,
	.pdmaPollingIdle = asicConnac2xWfdmaPollingAllIdle,
	.enableInterrupt = mt7961EnableInterrupt,
	.disableInterrupt = mt7961DisableInterrupt,
	.processTxInterrupt = mt7961Connac2xProcessTxInterrupt,
	.processRxInterrupt = mt7961Connac2xProcessRxInterrupt,
	.tx_ring_ext_ctrl = mt7961WfdmaTxRingExtCtrl,
	.rx_ring_ext_ctrl = mt7961WfdmaRxRingExtCtrl,
	.rx_evt_ring_buf_size = RX_BUFFER_AGGRESIZE,
	/* null wfdmaManualPrefetch if want to disable manual mode */
	.wfdmaManualPrefetch = mt7961Connac2xWfdmaManualPrefetch,
	.lowPowerOwnRead = asicConnac2xLowPowerOwnRead,
	.lowPowerOwnSet = asicConnac2xLowPowerOwnSet,
	.lowPowerOwnClear = asicConnac2xLowPowerOwnClear,
	.wakeUpWiFi = asicWakeUpWiFi,
	.getSoftwareInterrupt = asicConnac2xGetSoftwareInterrupt,
	.processSoftwareInterrupt = asicConnac2xProcessSoftwareInterrupt,
	.softwareInterruptMcu = asicConnac2xSoftwareInterruptMcu,
	.hifRst = asicConnac2xHifRst,
	.initPcieInt = wlanBuzzardInitPcieInt,
	.devReadIntStatus = mt7961ReadIntStatus,
	.DmaShdlInit = mt7961DmashdlInit,
	.DmaShdlReInit = mt7961DmashdlReInit,
	.updateTxRingMaxQuota = mt7961UpdateDmashdlQuota,
	.setRxRingHwAddr = mt7961SetRxRingHwAddr,
	.wfdmaAllocRxRing = mt7961LiteWfdmaAllocRxRing,
#if CFG_SUPPORT_PCIE_ASPM_IMPROVE
	.configPcieASPM = mt7961ConfigPcieASPM,
	.setCTSbyRate = mt7961SetCTSbyRate,
#endif
#if CFG_SUPPORT_HOST_RX_WM_EVENT_FROM_PSE
#if defined(_HIF_PCIE)
	.checkPortForRxEventFromPse = mt7961CheckPortForRxEventFromPse,
#else
	.checkPortForRxEventFromPse = NULL,
#endif
#endif
#if (CFG_COALESCING_INTERRUPT == 1)
#if defined(_HIF_PCIE)
	.setWfdmaCoalescingInt = mt7961setWfdmaCoalescingInt,
#else
	.setWfdmaCoalescingInt = NULL,
#endif
#endif/* (CFG_COALESCING_INTERRUPT == 1) */
#endif /*_HIF_PCIE || _HIF_AXI */

#if defined(_HIF_USB)
	.u4UdmaWlCfg_0_Addr = CONNAC2X_UDMA_WLCFG_0,
	.u4UdmaWlCfg_1_Addr = CONNAC2X_UDMA_WLCFG_1,
	.u4UdmaWlCfg_0 =
	    (CONNAC2X_UDMA_WLCFG_0_WL_TX_EN(1) |
	     CONNAC2X_UDMA_WLCFG_0_WL_RX_EN(1) |
	     CONNAC2X_UDMA_WLCFG_0_WL_RX_MPSZ_PAD0(1) |
	     CONNAC2X_UDMA_WLCFG_0_TICK_1US_EN(1)),
	.u4UdmaTxQsel = CONNAC2X_UDMA_TX_QSEL,
	.u4UdmaConnInfraStatusSelAddr = CONNAC2X_UDMA_CONN_INFRA_STATUS_SEL,
	.u4UdmaConnInfraStatusSelVal = 0x00000000,
	.u4UdmaConnInfraStatusAddr = CONNAC2X_UDMA_CONN_INFRA_STATUS,
	.u4device_vender_request_in = DEVICE_VENDOR_REQUEST_IN_CONNAC2,
	.u4device_vender_request_out = DEVICE_VENDOR_REQUEST_OUT_CONNAC2,
	.fgIsSupportWdtEp = TRUE,
	.asicUsbEventEpDetected = mt7961Connac2xUsbEventEpDetected,
	.asicUsbRxByteCount = mt7961Connac2xUsbRxByteCount,
	.u4SuspendVer = SUSPEND_V2,
	.asicUsbSuspend = NULL, /*asicUsbSuspend*/
	.asicUsbResume = asicConnac2xUsbResume,
	.DmaShdlInit = mt7961DmashdlInit,
	.DmaShdlReInit = mt7961DmashdlReInit,
	.updateTxRingMaxQuota = mt7961UpdateDmashdlQuota,
	.asicUdmaRxFlush = asicConnac2xUdmaRxFlush,
#if CFG_CHIP_RESET_SUPPORT
	.asicUsbEpctlRstOpt = mt7961HalUsbEpctlRstOpt,
#endif
#endif
#if defined(_HIF_SDIO)
	.halTxGetFreeResource = halTxGetFreeResource_v1,
	.halTxReturnFreeResource = halTxReturnFreeResource_v1,
	.halRestoreTxResource = halRestoreTxResource_v1,
	.halUpdateTxDonePendingCount = halUpdateTxDonePendingCount_v1,
#endif /* _HIF_SDIO */
};

#if CFG_ENABLE_FW_DOWNLOAD
struct FWDL_OPS_T mt7961_fw_dl_ops = {
	.constructFirmwarePrio = mt7961ConstructFirmwarePrio,
	.constructPatchName = mt7961ConstructPatchName,
	.downloadPatch = wlanDownloadPatch,
	.downloadFirmware = wlanConnacFormatDownload,
	.getFwInfo = wlanGetConnacFwInfo,
	.getFwDlInfo = asicGetFwDlInfo,
#if CFG_SUPPORT_WIFI_DL_BT_PATCH
	.constructBtPatchName = mt7961ConstructBtPatchName,
	.downloadBtPatch = mt7961DownloadBtPatch,
#endif
};
#endif /* CFG_ENABLE_FW_DOWNLOAD */

struct TX_DESC_OPS_T mt7961TxDescOps = {
	.fillNicAppend = fillNicTxDescAppend,
	.fillHifAppend = fillTxDescAppendByHostV2,
	.fillTxByteCount = fillConnac2xTxDescTxByteCount,
};

struct RX_DESC_OPS_T mt7961RxDescOps = {};

struct CHIP_DBG_OPS mt7961DebugOps = {
	.showPdmaInfo = mt7961_show_wfdma_info,
	.showPseInfo = mt7961_show_pse_info,
	.showPleInfo = mt7961_show_ple_info,
	.showTxdInfo = connac2x_show_txd_Info,
	.showWtblInfo = connac2x_show_wtbl_info,
	.showUmacFwtblInfo = connac2x_show_umac_wtbl_info,
#if defined(_HIF_PCIE)
	.showCsrInfo = pcie_show_csr_info,
#else
	.showCsrInfo = NULL,
#endif
	.showDmaschInfo = NULL,
	.showHifInfo = NULL,
	.printHifDbgInfo = NULL,
	.show_rx_rate_info = connac2x_show_rx_rate_info,
	.show_rx_rssi_info = connac2x_show_rx_rssi_info,
	.show_stat_info = connac2x_show_stat_info,
#if (CFG_SUPPORT_LINK_QUALITY_MONITOR == 1)
	.get_rx_rate_info = connac2x_get_rx_rate_info,
#endif
#if defined(_HIF_SDIO)
	.show_mcu_debug_info = sdio_show_mcu_debug_info,
	.get_sdio_debug_info = mt7961_get_sdio_debug_info,
#elif defined(_HIF_USB)
	.show_mcu_debug_info = usb_show_mcu_debug_info,
#elif defined(_HIF_PCIE)
	.show_mcu_debug_info = pcie_show_mcu_debug_info,
#endif
	.show_conninfra_debug_info = NULL,
#if (CFG_SUPPORT_DEBUG_SOP == 1)
	.show_debug_sop_info = mt7961_show_debug_sop_info,
#endif
};

/* Litien code refine to support multi chip */
struct mt66xx_chip_info mt66xx_chip_info_mt7961 = {
	.bus_info = &mt7961_bus_info,
#if CFG_ENABLE_FW_DOWNLOAD
	.fw_dl_ops = &mt7961_fw_dl_ops,
#endif /* CFG_ENABLE_FW_DOWNLOAD */
	.prTxDescOps = &mt7961TxDescOps,
	.prRxDescOps = &mt7961RxDescOps,
	.prDebugOps = &mt7961DebugOps,
	.chip_id = MT7961_CHIP_ID,
	.should_verify_chip_id = FALSE,
	.sw_sync0 = MT7961_SW_SYNC0,
	.sw_ready_bits = WIFI_FUNC_NO_CR4_READY_BITS,
	.sw_ready_bit_offset = MT7961_SW_SYNC0_RDY_OFFSET,
	.patch_addr = MT7961_PATCH_START_ADDR,
	.is_support_cr4 = FALSE,
	.is_support_wacpu = FALSE,
	.txd_append_size = MT7961_TX_DESC_APPEND_LENGTH,
	.rxd_size = MT7961_RX_DESC_LENGTH,
	.pse_header_length = CONNAC2X_NIC_TX_PSE_HEADER_LENGTH,
	.init_event_size = CONNAC2X_RX_INIT_EVENT_LENGTH,
	.event_hdr_size = CONNAC2X_RX_EVENT_HDR_LENGTH,
	.eco_info = mt7961_eco_table,
	.isNicCapV1 = FALSE,
	.is_support_efuse = TRUE,
	.top_hcr = CONNAC2X_TOP_HCR,
	.top_hvr = CONNAC2X_TOP_HVR,
	.top_fvr = CONNAC2X_TOP_FVR,
	.arb_ac_mode_addr = MT7961_ARB_AC_MODE_ADDR,
	.asicCapInit = asicConnac2xCapInit,
#if CFG_ENABLE_FW_DOWNLOAD
	.asicEnableFWDownload = NULL,
#endif /* CFG_ENABLE_FW_DOWNLOAD */
#if defined(_HIF_USB) || defined(_HIF_SDIO)
	.asicDumpSerDummyCR = mt7961DumpSerDummyCR,
#endif
	.downloadBufferBin = wlanConnac2XDownloadBufferBin,
	.constructBufferBinFileName = mt7961ConstructBufferBinFileName,
	.is_support_hw_amsdu = TRUE,
	.is_support_asic_lp = TRUE,
	.is_support_wfdma1 = FALSE,
	.get_rxv_from_rxrpt = mt7961GetRxvSrc,
	.RxDbgInfoFromRxdGrp3 = mt7961GetRxDbgInfoSrc,
	.asicWfdmaReInit = asicConnac2xWfdmaReInit,
	.asicWfdmaReInit_handshakeInit = asicConnac2xWfdmaDummyCrWrite,
	.u4SerSuspendSyncAddr = CONNAC2X_HOST_MAILBOX_WF_ADDR,
#if defined(_HIF_USB)
	.asicUsbInit = asicConnac2xWfdmaInitForUSB,
	.asicUsbInit_ic_specific = mt7961Connac2xWfdmaInitForUSB,
	.u4SerUsbMcuEventAddr = WF_SW_DEF_CR_USB_MCU_EVENT_ADD,
	.u4SerUsbHostAckAddr = WF_SW_DEF_CR_USB_HOST_ACK_ADDR,
#endif
	.group5_size = sizeof(struct HW_MAC_RX_STS_GROUP_5),
	.u4LmacWtblDUAddr = MT7961_WIFI_LWTBL_BASE,
	.u4UmacWtblDUAddr = MT7961_WIFI_UWTBL_BASE,

#if CFG_CHIP_RESET_SUPPORT
	.asicWfsysRst = mt7961HalCbtopRguWfRst,
	.asicPollWfsysSwInitDone = mt7961HalPollWfsysSwInitDone,
#if defined(_HIF_SDIO)
        .asicSetNoBTFwOwnEn = mt7961HalSetNoBTFwOwnEn,
#endif
#endif
	.asicSerInit = mt7961SerInit,
	.prTxPwrLimitFile = "TxPwrLimit_MT79x1.dat",
	.ucTxPwrLimitBatchSize = 8,

	/* buzzard capability:
	 * 1. MAC RX AMPDU max number is 256.
	 * 2. MAC RX MPDU max length is 11454.
	 * So, MAC RX AMPDU max length is 256 * 11454 = 2,932,224
	 *
	 * Set HE_CAP MAX_AMPDU_LEN_EXP to maximum which meets the condition
	 *     1. In 2.4G, 2 ^ (16 + MAX_AMPDU_LEN_EXP) - 1 <= 2,932,224
	 *     2. In 5G, 2 ^ (20 + MAX_AMPDU_LEN_EX) - 1 <= 2,932,224
	 */
	.uc2G4HeCapMaxAmpduLenExp = 3, /* 2 ^ (16 + 3) - 1 = 524287 */
	.uc5GHeCapMaxAmpduLenExp = 1, /* 2 ^ (20 + 1) - 1 = 2,097,151 */
#if defined(_HIF_PCIE) || defined(_HIF_AXI) || defined(_HIF_USB)
	.dmashdlQuotaDecision= mt7961dmashdlQuotaDecision,
#endif

#if defined(_HIF_PCIE) || defined(_HIF_AXI)
	/* owner set true when feature is ready. */
	.fgIsSupportL0p5Reset = TRUE,
#elif defined(_HIF_USB)
	.fgIsSupportL0p5Reset = TRUE,
#elif defined(_HIF_SDIO)
	/* owner set true when feature is ready. */
	.fgIsSupportL0p5Reset = TRUE,
#endif
	.u4ADieVer = 0xFFFFFFFF,
	.loadCfgSetting = NULL,

#if ((CFG_SUPPORT_5G_TX_MCS_LIMIT == 1) && (CFG_SUPPORT_DBDC == 1))
	.fgIs5gTxMcsLimited = TRUE,
#endif
};

struct mt66xx_hif_driver_data mt66xx_driver_data_mt7961 = {
	.chip_info = &mt66xx_chip_info_mt7961,
};
#endif /* MT7961 */

#endif /* defined(MT7961) || defined(MT7922) || defined(MT7902) */
