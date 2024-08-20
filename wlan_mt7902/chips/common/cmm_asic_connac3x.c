/* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause */
/*
 * Copyright (c) 2016 MediaTek Inc.
 */
/*! \file   cmm_asic_connac3x.c
*    \brief  Internal driver stack will export the required procedures here for
* GLUE Layer.
*
*    This file contains all routines which are exported from MediaTek 802.11
* Wireless
*    LAN driver stack to GLUE Layer.
*/

/*******************************************************************************
*                         C O M P I L E R   F L A G S
********************************************************************************
*/

#if (CFG_SUPPORT_CONNAC3X == 1)

/*******************************************************************************
*                    E X T E R N A L   R E F E R E N C E S
********************************************************************************
*/

#include "precomp.h"
#include "wlan_lib.h"

/*******************************************************************************
*                              C O N S T A N T S
********************************************************************************
*/
#define CONNAC3X_HIF_DMASHDL_BASE 0x52000000

/*******************************************************************************
*                            P U B L I C   D A T A
********************************************************************************
*/

/*******************************************************************************
*                           P R I V A T E   D A T A
********************************************************************************
*/

#define USB_ACCESS_RETRY_LIMIT           1

/*******************************************************************************
*                              F U N C T I O N S
********************************************************************************
*/
void asicConnac3xCapInit(
	struct ADAPTER *prAdapter)
{
	struct GLUE_INFO *prGlueInfo;
	struct mt66xx_chip_info *prChipInfo;
	struct BUS_INFO *prBusInfo = NULL;
	uint32_t u4HostWpdamBase = 0;

	ASSERT(prAdapter);
	if (prAdapter->chip_info->is_support_wfdma1)
		u4HostWpdamBase = CONNAC3X_HOST_WPDMA_1_BASE;
	else
		u4HostWpdamBase = CONNAC3X_HOST_WPDMA_0_BASE;

	prGlueInfo = prAdapter->prGlueInfo;
	prChipInfo = prAdapter->chip_info;
	prBusInfo = prChipInfo->bus_info;

	prChipInfo->u2HifTxdSize = 0;
	prChipInfo->u2TxInitCmdPort = 0;
	prChipInfo->u2TxFwDlPort = 0;
	prChipInfo->fillHifTxDesc = NULL;
	prChipInfo->u2CmdTxHdrSize = sizeof(struct CONNAC3X_WIFI_CMD);
	prChipInfo->asicFillInitCmdTxd = asicConnac3xFillInitCmdTxd;
	prChipInfo->asicFillCmdTxd = asicConnac3xFillCmdTxd;
#ifdef CFG_SUPPORT_UNIFIED_COMMAND
	prChipInfo->u2UniCmdTxHdrSize = sizeof(struct CONNAC3X_WIFI_UNI_CMD);
	prChipInfo->asicFillUniCmdTxd = asicConnac3xFillUniCmdTxd;
#endif
	prChipInfo->u2RxSwPktBitMap = CONNAC3X_RX_STATUS_PKT_TYPE_SW_BITMAP;
	prChipInfo->u2RxSwPktEvent = CONNAC3X_RX_STATUS_PKT_TYPE_SW_EVENT;
	prChipInfo->u2RxSwPktFrame = CONNAC3X_RX_STATUS_PKT_TYPE_SW_FRAME;
	prChipInfo->u4ExtraTxByteCount = 0;
	asicConnac3xInitTxdHook(prChipInfo->prTxDescOps);
	asicConnac3xInitRxdHook(prChipInfo->prRxDescOps);
#if (CFG_SUPPORT_MSP == 1)
	prChipInfo->asicRxProcessRxvforMSP = asicConnac3xRxProcessRxvforMSP;
#endif /* CFG_SUPPORT_MSP == 1 */
	prChipInfo->asicRxGetRcpiValueFromRxv =
			asicConnac3xRxGetRcpiValueFromRxv;
#if (CFG_SUPPORT_PERF_IND == 1)
	prChipInfo->asicRxPerfIndProcessRXV = asicConnac3xRxPerfIndProcessRXV;
#endif
#if (CFG_CHIP_RESET_SUPPORT == 1) && (CFG_WMT_RESET_API_SUPPORT == 0)
	prChipInfo->rst_L0_notify_step2 = conn2_rst_L0_notify_step2;
#endif
#if CFG_SUPPORT_WIFI_SYSDVT
	prAdapter->u2TxTest = TX_TEST_UNLIMITIED;
	prAdapter->u2TxTestCount = 0;
	prAdapter->ucTxTestUP = TX_TEST_UP_UNDEF;
#endif /* CFG_SUPPORT_WIFI_SYSDVT */

	switch (prGlueInfo->u4InfType) {
#if defined(_HIF_PCIE) || defined(_HIF_AXI)
	case MT_DEV_INF_PCIE:
	case MT_DEV_INF_AXI:

		prChipInfo->u2TxInitCmdPort =
			TX_RING_CMD_IDX_2; /* Ring17 for CMD */
		prChipInfo->u2TxFwDlPort =
			TX_RING_FWDL_IDX_3; /* Ring16 for FWDL */
		prChipInfo->ucPacketFormat = TXD_PKT_FORMAT_TXD;
		prChipInfo->u4HifDmaShdlBaseAddr = CONNAC3X_HIF_DMASHDL_BASE;

		HAL_MCR_WR(prAdapter,
				CONNAC3X_BN0_IRQ_ENA_ADDR,
				BIT(0));

		if (prChipInfo->is_support_asic_lp) {
			HAL_MCR_WR(prAdapter,
				CONNAC3X_WPDMA_MCU2HOST_SW_INT_MASK
					(u4HostWpdamBase),
				BITS(0, 15));
		}
		break;
#endif /* _HIF_PCIE */
#if defined(_HIF_USB)
	case MT_DEV_INF_USB:
		prChipInfo->u2HifTxdSize = USB_HIF_TXD_LEN;
		prChipInfo->fillHifTxDesc = fillUsbHifTxDesc;
		prChipInfo->u2TxInitCmdPort = USB_DATA_BULK_OUT_EP8;
		prChipInfo->u2TxFwDlPort = USB_DATA_BULK_OUT_EP4;
		if (prChipInfo->is_support_wacpu)
			prChipInfo->ucPacketFormat = TXD_PKT_FORMAT_TXD;
		else
			prChipInfo->ucPacketFormat =
						TXD_PKT_FORMAT_TXD_PAYLOAD;
		prChipInfo->u4ExtraTxByteCount =
				EXTRA_TXD_SIZE_FOR_TX_BYTE_COUNT;
		prChipInfo->u4HifDmaShdlBaseAddr = USB_HIF_DMASHDL_BASE;

#if (CFG_ENABLE_FW_DOWNLOAD == 1)
		prChipInfo->asicEnableFWDownload = asicConnac3xEnableUsbFWDL;
#endif /* CFG_ENABLE_FW_DOWNLOAD == 1 */

		break;
#endif /* _HIF_USB */
	default:
		break;
	}
}

static void asicConnac3xFillInitCmdTxdInfo(
	struct ADAPTER *prAdapter,
	struct WIFI_CMD_INFO *prCmdInfo,
	u_int8_t *pucSeqNum)
{
	struct INIT_HIF_TX_HEADER *prInitHifTxHeader;
	struct HW_MAC_CONNAC3X_TX_DESC *prInitHifTxHeaderPadding;
	uint32_t u4TxdLen = sizeof(struct HW_MAC_CONNAC3X_TX_DESC);

	prInitHifTxHeaderPadding =
		(struct HW_MAC_CONNAC3X_TX_DESC *) (prCmdInfo->pucInfoBuffer);
	prInitHifTxHeader = (struct INIT_HIF_TX_HEADER *)
		(prCmdInfo->pucInfoBuffer + u4TxdLen);

	HAL_MAC_CONNAC3X_TXD_SET_TX_BYTE_COUNT(prInitHifTxHeaderPadding,
		prCmdInfo->u2InfoBufLen);
	if (!prCmdInfo->ucCID)
		HAL_MAC_CONNAC3X_TXD_SET_PKT_FORMAT(prInitHifTxHeaderPadding,
						INIT_PKT_FT_PDA_FWDL)
	else
		HAL_MAC_CONNAC3X_TXD_SET_PKT_FORMAT(prInitHifTxHeaderPadding,
						INIT_PKT_FT_CMD)
	HAL_MAC_CONNAC3X_TXD_SET_HEADER_FORMAT(prInitHifTxHeaderPadding,
						HEADER_FORMAT_COMMAND);
	HAL_MAC_CONNAC3X_TXD_SET_QUEUE_INDEX(prInitHifTxHeaderPadding, 0x20);
	prInitHifTxHeader->rInitWifiCmd.ucCID = prCmdInfo->ucCID;
	prInitHifTxHeader->rInitWifiCmd.ucPktTypeID = prCmdInfo->ucPktTypeID;
	prInitHifTxHeader->rInitWifiCmd.ucSeqNum =
		nicIncreaseCmdSeqNum(prAdapter);
	prInitHifTxHeader->u2TxByteCount =
		prCmdInfo->u2InfoBufLen - u4TxdLen;

	if (pucSeqNum)
		*pucSeqNum = prInitHifTxHeader->rInitWifiCmd.ucSeqNum;

	DBGLOG(INIT, INFO, "TX CMD: ID[0x%02X] SEQ[%u] LEN[%u]\n",
			prInitHifTxHeader->rInitWifiCmd.ucCID,
			prInitHifTxHeader->rInitWifiCmd.ucSeqNum,
			prCmdInfo->u2InfoBufLen);
}


static void asicConnac3xFillCmdTxdInfo(
	struct ADAPTER *prAdapter,
	struct WIFI_CMD_INFO *prCmdInfo,
	u_int8_t *pucSeqNum)
{
	struct CONNAC3X_WIFI_CMD *prWifiCmd;
	uint32_t u4TxdLen = sizeof(struct HW_MAC_CONNAC3X_TX_DESC);

	prWifiCmd = (struct CONNAC3X_WIFI_CMD *)prCmdInfo->pucInfoBuffer;

	HAL_MAC_CONNAC3X_TXD_SET_TX_BYTE_COUNT(
		(struct HW_MAC_CONNAC3X_TX_DESC *)prWifiCmd,
		prCmdInfo->u2InfoBufLen);
	HAL_MAC_CONNAC3X_TXD_SET_PKT_FORMAT(
		(struct HW_MAC_CONNAC3X_TX_DESC *)prWifiCmd,
		INIT_PKT_FT_CMD);
	HAL_MAC_CONNAC3X_TXD_SET_HEADER_FORMAT(
		(struct HW_MAC_CONNAC3X_TX_DESC *)prWifiCmd,
		HEADER_FORMAT_COMMAND);
	HAL_MAC_CONNAC3X_TXD_SET_QUEUE_INDEX(
		(struct HW_MAC_CONNAC3X_TX_DESC *)prWifiCmd, 0x20);

	prWifiCmd->ucCID = prCmdInfo->ucCID;
	prWifiCmd->ucExtenCID = prCmdInfo->ucExtCID;
	prWifiCmd->ucPktTypeID = prCmdInfo->ucPktTypeID;
	prWifiCmd->ucSetQuery = prCmdInfo->ucSetQuery;
	prWifiCmd->ucSeqNum = nicIncreaseCmdSeqNum(prAdapter);
	prWifiCmd->ucS2DIndex = prCmdInfo->ucS2DIndex;
	prWifiCmd->u2Length = prCmdInfo->u2InfoBufLen - u4TxdLen;

	if (pucSeqNum)
		*pucSeqNum = prWifiCmd->ucSeqNum;

	if (aucDebugModule[DBG_TX_IDX] & DBG_CLASS_TRACE)
		DBGLOG(TX, TRACE,
			"TX CMD: ID[0x%02X] SEQ[%u] SET[%u] LEN[%u]\n",
			prWifiCmd->ucCID, prWifiCmd->ucSeqNum,
			prWifiCmd->ucSetQuery, prCmdInfo->u2InfoBufLen);
	else
		DBGLOG_LIMITED(TX, INFO,
			"TX CMD: ID[0x%02X] SEQ[%u] SET[%u] LEN[%u]\n",
			prWifiCmd->ucCID, prWifiCmd->ucSeqNum,
			prWifiCmd->ucSetQuery, prCmdInfo->u2InfoBufLen);
}

#ifdef CFG_SUPPORT_UNIFIED_COMMAND
static void asicConnac3xFillUniCmdTxdInfo(
	struct ADAPTER *prAdapter,
	struct WIFI_UNI_CMD_INFO *prCmdInfo,
	u_int8_t *pucSeqNum)
{
	struct CONNAC3X_WIFI_UNI_CMD *prWifiCmd;
	uint32_t u4TxdLen = sizeof(struct HW_MAC_CONNAC3X_TX_DESC);

	prWifiCmd = (struct CONNAC3X_WIFI_UNI_CMD *)prCmdInfo->pucInfoBuffer;

	HAL_MAC_CONNAC3X_TXD_SET_TX_BYTE_COUNT(
		(struct HW_MAC_CONNAC3X_TX_DESC *)prWifiCmd,
		prCmdInfo->u2InfoBufLen);
	HAL_MAC_CONNAC3X_TXD_SET_PKT_FORMAT(
		(struct HW_MAC_CONNAC3X_TX_DESC *)prWifiCmd,
		INIT_PKT_FT_CMD);
	HAL_MAC_CONNAC3X_TXD_SET_HEADER_FORMAT(
		(struct HW_MAC_CONNAC3X_TX_DESC *)prWifiCmd,
		HEADER_FORMAT_COMMAND);
	HAL_MAC_CONNAC3X_TXD_SET_QUEUE_INDEX(
		(struct HW_MAC_CONNAC3X_TX_DESC *)prWifiCmd, 0x20);

	prWifiCmd->u2CID = prCmdInfo->u2CID;
	prWifiCmd->ucPktTypeID = prCmdInfo->ucPktTypeID;
	prWifiCmd->ucSeqNum = nicIncreaseCmdSeqNum(prAdapter);
	prWifiCmd->ucS2DIndex = prCmdInfo->ucS2DIndex;
	prWifiCmd->u2Length = prCmdInfo->u2InfoBufLen - u4TxdLen;
	prWifiCmd->ucOption = prCmdInfo->ucOption;

	if (pucSeqNum)
		*pucSeqNum = prWifiCmd->ucSeqNum;

	DBGLOG(TX, TRACE, "TX CMD: ID[0x%04X] SEQ[%u] OPT[0x%02X] LEN[%u]\n",
			prWifiCmd->u2CID, prWifiCmd->ucSeqNum,
			prWifiCmd->ucOption, prCmdInfo->u2InfoBufLen);
}

#endif

void asicConnac3xFillInitCmdTxd(
	struct ADAPTER *prAdapter,
	struct WIFI_CMD_INFO *prCmdInfo,
	u_int16_t *pu2BufInfoLen,
	u_int8_t *pucSeqNum, OUT void **pCmdBuf)
{
	struct INIT_HIF_TX_HEADER *prInitHifTxHeader;
	uint32_t u4TxdLen = sizeof(struct HW_MAC_CONNAC3X_TX_DESC);

	/* We don't need to append TXD while sending fw frames. */
	if (!prCmdInfo->ucCID && pCmdBuf)
		*pCmdBuf = prCmdInfo->pucInfoBuffer;
	else {
		prInitHifTxHeader = (struct INIT_HIF_TX_HEADER *)
			(prCmdInfo->pucInfoBuffer + u4TxdLen);
		asicConnac3xFillInitCmdTxdInfo(
			prAdapter,
			prCmdInfo,
			pucSeqNum);
		if (pCmdBuf)
			*pCmdBuf =
				&prInitHifTxHeader->rInitWifiCmd.aucBuffer[0];
	}
}

void asicConnac3xWfdmaDummyCrRead(
	struct ADAPTER *prAdapter,
	u_int8_t *pfgResult)
{
	u_int32_t u4RegValue = 0;

	HAL_MCR_RD(prAdapter,
		CONNAC3X_WFDMA_DUMMY_CR,
		&u4RegValue);
	*pfgResult = (u4RegValue &
		CONNAC3X_WFDMA_NEED_REINIT_BIT)
		== 0 ? TRUE : FALSE;
}


void asicConnac3xWfdmaDummyCrWrite(
	struct ADAPTER *prAdapter)
{
	u_int32_t u4RegValue = 0;

	HAL_MCR_RD(prAdapter,
		CONNAC3X_WFDMA_DUMMY_CR,
		&u4RegValue);
	u4RegValue |= CONNAC3X_WFDMA_NEED_REINIT_BIT;

	HAL_MCR_WR(prAdapter,
		CONNAC3X_WFDMA_DUMMY_CR,
		u4RegValue);
}
void asicConnac3xWfdmaReInit(
	struct ADAPTER *prAdapter)
{
	u_int8_t fgResult;
	struct BUS_INFO *prBusInfo;

	prBusInfo = prAdapter->chip_info->bus_info;

	/*WFDMA re-init flow after chip deep sleep*/
	asicConnac3xWfdmaDummyCrRead(prAdapter, &fgResult);
	if (fgResult) {
#if defined(_HIF_PCIE) || defined(_HIF_AXI)

#if 0 /* Original Driver re-init Host WFDAM flow */
	DBGLOG(INIT, INFO, "WFDMA host sw-reinit due to deep sleep\n");
	halWpdmaInitRing(prAdapter->prGlueInfo, false);
#else /* Do Driver re-init Host WFDMA flow with FW bk/sr solution */
	{
		struct GL_HIF_INFO *prHifInfo;
		uint32_t u4Idx;

		DBGLOG(INIT, TRACE, "WFDMA reinit after bk/sr(deep sleep)\n");
		prHifInfo = &prAdapter->prGlueInfo->rHifInfo;
		for (u4Idx = 0; u4Idx < NUM_OF_TX_RING; u4Idx++) {
			prHifInfo->TxRing[u4Idx].TxSwUsedIdx = 0;
			prHifInfo->TxRing[u4Idx].u4UsedCnt = 0;
			prHifInfo->TxRing[u4Idx].TxCpuIdx = 0;
		}

		if (halWpdmaGetRxDmaDoneCnt(prAdapter->prGlueInfo,
			RX_RING_EVT_IDX_1)) {
			prAdapter->u4NoMoreRfb |= BIT(RX_RING_EVT_IDX_1);
		}
	}
#endif
	/* Write sleep mode magic num to dummy reg */
	if (prBusInfo->setDummyReg)
		prBusInfo->setDummyReg(prAdapter->prGlueInfo);

#elif defined(_HIF_USB)
		if (prAdapter->chip_info->is_support_asic_lp &&
				prAdapter->chip_info->asicUsbInit)
			prAdapter->chip_info->asicUsbInit(prAdapter,
							  prAdapter->chip_info);
#endif /* defined(_HIF_PCIE) || defined(_HIF_AXI) */
		asicConnac3xWfdmaDummyCrWrite(prAdapter);
	}
}

void asicConnac3xFillCmdTxd(
	struct ADAPTER *prAdapter,
	struct WIFI_CMD_INFO *prCmdInfo,
	u_int8_t *pucSeqNum,
	void **pCmdBuf)
{
	struct CONNAC3X_WIFI_CMD *prWifiCmd;

	/* 2. Setup common CMD Info Packet */
	prWifiCmd = (struct CONNAC3X_WIFI_CMD *)prCmdInfo->pucInfoBuffer;
	asicConnac3xFillCmdTxdInfo(prAdapter, prCmdInfo, pucSeqNum);
	if (pCmdBuf)
		*pCmdBuf = &prWifiCmd->aucBuffer[0];
}

#ifdef CFG_SUPPORT_UNIFIED_COMMAND
void asicConnac3xFillUniCmdTxd(
	struct ADAPTER *prAdapter,
	struct WIFI_UNI_CMD_INFO *prCmdInfo,
	u_int8_t *pucSeqNum,
	void **pCmdBuf)
{
	struct CONNAC3X_WIFI_UNI_CMD *prWifiCmd;

	/* 2. Setup common CMD Info Packet */
	prWifiCmd = (struct CONNAC3X_WIFI_UNI_CMD *)prCmdInfo->pucInfoBuffer;
	asicConnac3xFillUniCmdTxdInfo(prAdapter, prCmdInfo, pucSeqNum);
	if (pCmdBuf)
		*pCmdBuf = &prWifiCmd->aucBuffer[0];
}
#endif

#if defined(_HIF_PCIE) || defined(_HIF_AXI)
uint32_t asicConnac3xWfdmaCfgAddrGet(
	struct GLUE_INFO *prGlueInfo,
	u_int8_t ucDmaIdx)
{
	struct BUS_INFO *prBusInfo = prGlueInfo->prAdapter->chip_info->bus_info;

	if (ucDmaIdx == 0)
		return CONNAC3X_WPDMA_GLO_CFG(prBusInfo->host_dma0_base);
	else
		return CONNAC3X_WPDMA_GLO_CFG(prBusInfo->host_dma1_base);
}

uint32_t asicConnac3xWfdmaIntRstDtxPtrAddrGet(
	struct GLUE_INFO *prGlueInfo,
	u_int8_t ucDmaIdx)
{
	struct BUS_INFO *prBusInfo = prGlueInfo->prAdapter->chip_info->bus_info;

	if (ucDmaIdx == 0)
		return CONNAC3X_WPDMA_RST_DTX_PTR(prBusInfo->host_dma0_base);
	else
		return CONNAC3X_WPDMA_RST_DTX_PTR(prBusInfo->host_dma1_base);
}

uint32_t asicConnac3xWfdmaIntRstDrxPtrAddrGet(
	struct GLUE_INFO *prGlueInfo,
	u_int8_t ucDmaIdx)
{
	struct BUS_INFO *prBusInfo = prGlueInfo->prAdapter->chip_info->bus_info;

	if (ucDmaIdx == 0)
		return CONNAC3X_WPDMA_RST_DRX_PTR(prBusInfo->host_dma0_base);
	else
		return CONNAC3X_WPDMA_RST_DRX_PTR(prBusInfo->host_dma1_base);
}

uint32_t asicConnac3xWfdmaHifRstAddrGet(
	struct GLUE_INFO *prGlueInfo,
	u_int8_t ucDmaIdx)
{
	struct BUS_INFO *prBusInfo = prGlueInfo->prAdapter->chip_info->bus_info;

	if (ucDmaIdx == 0)
		return CONNAC3X_WPDMA_HIF_RST(prBusInfo->host_dma0_base);
	else
		return CONNAC3X_WPDMA_HIF_RST(prBusInfo->host_dma1_base);
}

void asicConnac3xWfdmaControl(
	struct GLUE_INFO *prGlueInfo,
	u_int8_t ucDmaIdx,
	u_int8_t enable)
{
	struct ADAPTER *prAdapter = prGlueInfo->prAdapter;
	union WPDMA_GLO_CFG_STRUCT GloCfg;
	uint32_t u4DmaCfgCr;
	uint32_t u4DmaRstDtxPtrCr;
	uint32_t u4DmaRstDrxPtrCr;

	ASSERT(ucDmaIdx < CONNAC3X_MAX_WFDMA_COUNT);
	u4DmaCfgCr = asicConnac3xWfdmaCfgAddrGet(prGlueInfo, ucDmaIdx);
	u4DmaRstDtxPtrCr =
		asicConnac3xWfdmaIntRstDtxPtrAddrGet(prGlueInfo, ucDmaIdx);
	u4DmaRstDrxPtrCr =
		asicConnac3xWfdmaIntRstDrxPtrAddrGet(prGlueInfo, ucDmaIdx);

	HAL_MCR_RD(prAdapter, u4DmaCfgCr, &GloCfg.word);
	if (enable == TRUE) {
		GloCfg.field_conn3x.pdma_bt_size = 3;
		GloCfg.field_conn3x.tx_wb_ddone = 1;
		GloCfg.field_conn3x.fifo_little_endian = 1;
		GloCfg.field_conn3x.clk_gate_dis = 1;
		GloCfg.field_conn3x.omit_tx_info = 1;
		if (ucDmaIdx == 1)
			GloCfg.field_conn3x.omit_rx_info = 1;
		GloCfg.field_conn3x.csr_disp_base_ptr_chain_en = 1;
		GloCfg.field_conn3x.omit_rx_info_pfet2 = 1;
	} else {
		GloCfg.field_conn3x.tx_dma_en = 0;
		GloCfg.field_conn3x.rx_dma_en = 0;
		GloCfg.field_conn3x.csr_disp_base_ptr_chain_en = 0;
		GloCfg.field_conn3x.omit_tx_info = 0;
		GloCfg.field_conn3x.omit_rx_info = 0;
		GloCfg.field_conn3x.omit_rx_info_pfet2 = 0;
	}
	HAL_MCR_WR(prAdapter, u4DmaCfgCr, GloCfg.word);

	if (!enable) {
		asicConnac3xWfdmaWaitIdle(prGlueInfo, ucDmaIdx, 100, 1000);
		/* Reset DMA Index */
		HAL_MCR_WR(prAdapter, u4DmaRstDtxPtrCr, 0xFFFFFFFF);
		HAL_MCR_WR(prAdapter, u4DmaRstDrxPtrCr, 0xFFFFFFFF);
	}
}

u_int8_t asicConnac3xWfdmaWaitIdle(
	struct GLUE_INFO *prGlueInfo,
	u_int8_t index,
	uint32_t round,
	uint32_t wait_us)
{
	uint32_t i = 0;
	uint32_t u4RegAddr = 0;
	union WPDMA_GLO_CFG_STRUCT GloCfg;
	struct BUS_INFO *prBusInfo = prGlueInfo->prAdapter->chip_info->bus_info;
	struct ADAPTER *prAdapter = prGlueInfo->prAdapter;

	if (index == 0)
		u4RegAddr = prBusInfo->host_dma0_base;
	else if (index == 1)
		u4RegAddr = prBusInfo->host_dma1_base;
	else {
		DBGLOG(HAL, ERROR, "Unknown wfdma index(=%d)\n", index);
		return FALSE;
	}

	do {
		HAL_MCR_RD(prAdapter, u4RegAddr, &GloCfg.word);
		if ((GloCfg.field.TxDMABusy == 0) &&
		    (GloCfg.field.RxDMABusy == 0)) {
			DBGLOG(HAL, TRACE, "==>  DMAIdle, GloCfg=0x%x\n",
			       GloCfg.word);
			return TRUE;
		}
		kalUdelay(wait_us);
	} while ((i++) < round);

	DBGLOG(HAL, INFO, "==>  DMABusy, GloCfg=0x%x\n", GloCfg.word);

	return FALSE;
}

void asicConnac3xWfdmaTxRingExtCtrl(
	struct GLUE_INFO *prGlueInfo,
	struct RTMP_TX_RING *tx_ring,
	u_int32_t index)
{
	struct BUS_INFO *prBusInfo;
	uint32_t ext_offset = 0;
	struct ADAPTER *prAdapter = prGlueInfo->prAdapter;
	struct mt66xx_chip_info *prChipInfo;

	prChipInfo = prGlueInfo->prAdapter->chip_info;
	prBusInfo = prGlueInfo->prAdapter->chip_info->bus_info;

	if (index == TX_RING_CMD_IDX_2)
		ext_offset = prBusInfo->tx_ring_cmd_idx * 4;
	else if (index == TX_RING_FWDL_IDX_3)
		ext_offset = prBusInfo->tx_ring_fwdl_idx * 4;
	else if (prChipInfo->is_support_wacpu) {
		if (index == TX_RING_DATA0_IDX_0)
			ext_offset = prBusInfo->tx_ring0_data_idx * 4;
		if (index == TX_RING_DATA1_IDX_1)
			ext_offset = prBusInfo->tx_ring1_data_idx * 4;
		if (index == TX_RING_WA_CMD_IDX_4)
			ext_offset = prBusInfo->tx_ring_wa_cmd_idx * 4;
	} else
		ext_offset = index * 4;

	tx_ring->hw_desc_base_ext =
		prBusInfo->host_tx_ring_ext_ctrl_base + ext_offset;
	HAL_MCR_WR(prAdapter, tx_ring->hw_desc_base_ext,
		   CONNAC3X_TX_RING_DISP_MAX_CNT);
}

void asicConnac3xWfdmaRxRingExtCtrl(
	struct GLUE_INFO *prGlueInfo,
	struct RTMP_RX_RING *rx_ring,
	u_int32_t index)
{
	struct ADAPTER *prAdapter;
	struct mt66xx_chip_info *prChipInfo;
	struct BUS_INFO *prBusInfo;
	uint32_t ext_offset;

	prAdapter = prGlueInfo->prAdapter;
	prChipInfo = prAdapter->chip_info;
	prBusInfo = prChipInfo->bus_info;

	switch (index) {
	case RX_RING_EVT_IDX_1:
		ext_offset = 0;
		break;
	case RX_RING_DATA_IDX_0:
		ext_offset = (index + 2) * 4;
		break;
	case RX_RING_DATA1_IDX_2:
	case RX_RING_TXDONE0_IDX_3:
	case RX_RING_TXDONE1_IDX_4:
		ext_offset = (index + 1) * 4;
		break;
	case RX_RING_WAEVT0_IDX_5:
	case RX_RING_WAEVT1_IDX_6:
		ext_offset = (index - 4) * 4;
		break;
	default:
		DBGLOG(RX, ERROR, "Error index=%d\n", index);
		return;
	}

	rx_ring->hw_desc_base_ext =
		prBusInfo->host_rx_ring_ext_ctrl_base + ext_offset;

	HAL_MCR_WR(prAdapter, rx_ring->hw_desc_base_ext,
		   CONNAC3X_RX_RING_DISP_MAX_CNT);
}

void asicConnac3xEnablePlatformIRQ(IN struct ADAPTER *prAdapter)
{
	struct GL_HIF_INFO *prHifInfo = NULL;

	ASSERT(prAdapter);

	prAdapter->fgIsIntEnable = TRUE;

	prHifInfo = &prAdapter->prGlueInfo->rHifInfo;
	enable_irq(prHifInfo->u4IrqId);
}

void asicConnac3xDisablePlatformIRQ(IN struct ADAPTER *prAdapter)
{
	struct GL_HIF_INFO *prHifInfo = NULL;

	ASSERT(prAdapter);

	prHifInfo = &prAdapter->prGlueInfo->rHifInfo;
	disable_irq_nosync(prHifInfo->u4IrqId);

	prAdapter->fgIsIntEnable = FALSE;
}

#if defined(_HIF_AXI)
void asicConnac3xDisablePlatformSwIRQ(IN struct ADAPTER *prAdapter)
{
	struct GL_HIF_INFO *prHifInfo = NULL;

	ASSERT(prAdapter);

	prHifInfo = &prAdapter->prGlueInfo->rHifInfo;
	disable_irq_nosync(prHifInfo->u4IrqId_1);
}
#endif

void asicConnac3xDisableExtInterrupt(
	struct ADAPTER *prAdapter)
{
	struct GLUE_INFO *prGlueInfo = NULL;
	union WPDMA_INT_MASK IntMask;

	ASSERT(prAdapter);

	prGlueInfo = prAdapter->prGlueInfo;

	IntMask.word = 0;

	HAL_MCR_WR(prAdapter,
		CONNAC3X_WPDMA_EXT_INT_MASK(CONNAC3X_HOST_EXT_CONN_HIF_WRAP),
		IntMask.word);
	HAL_MCR_RD(prAdapter,
		CONNAC3X_WPDMA_EXT_INT_MASK(CONNAC3X_HOST_EXT_CONN_HIF_WRAP),
		&IntMask.word);

	prAdapter->fgIsIntEnable = FALSE;

	DBGLOG(HAL, TRACE, "%s\n", __func__);

}

void asicConnac3xLowPowerOwnRead(
	struct ADAPTER *prAdapter,
	u_int8_t *pfgResult)
{
	struct mt66xx_chip_info *prChipInfo;

	prChipInfo = prAdapter->chip_info;

	if (prChipInfo->is_support_asic_lp) {
		u_int32_t u4RegValue;

		HAL_MCR_RD(prAdapter,
				CONNAC3X_BN0_LPCTL_ADDR,
				&u4RegValue);
		*pfgResult = (u4RegValue &
				PCIE_LPCR_AP_HOST_OWNER_STATE_SYNC)
				== 0 ? TRUE : FALSE;
	} else
		*pfgResult = TRUE;
}

void asicConnac3xLowPowerOwnSet(
	struct ADAPTER *prAdapter,
	u_int8_t *pfgResult)
{
	struct mt66xx_chip_info *prChipInfo;

	prChipInfo = prAdapter->chip_info;

	if (prChipInfo->is_support_asic_lp) {
		u_int32_t u4RegValue = 0;

		HAL_MCR_WR(prAdapter,
				CONNAC3X_BN0_LPCTL_ADDR,
				PCIE_LPCR_HOST_SET_OWN);
		HAL_MCR_RD(prAdapter,
				CONNAC3X_BN0_LPCTL_ADDR,
				&u4RegValue);
		*pfgResult = (u4RegValue &
			PCIE_LPCR_AP_HOST_OWNER_STATE_SYNC) == 0x4;
	} else
		*pfgResult = TRUE;
}

void asicConnac3xLowPowerOwnClear(
	struct ADAPTER *prAdapter,
	u_int8_t *pfgResult)
{
	struct mt66xx_chip_info *prChipInfo;

	prChipInfo = prAdapter->chip_info;

	if (prChipInfo->is_support_asic_lp) {
		u_int32_t u4RegValue = 0;

		HAL_MCR_WR(prAdapter,
			CONNAC3X_BN0_LPCTL_ADDR,
			PCIE_LPCR_HOST_CLR_OWN);
		HAL_MCR_RD(prAdapter,
			CONNAC3X_BN0_LPCTL_ADDR,
			&u4RegValue);
		*pfgResult = (u4RegValue &
				PCIE_LPCR_AP_HOST_OWNER_STATE_SYNC) == 0;
	} else
		*pfgResult = TRUE;
}

void asicConnac3xProcessSoftwareInterrupt(
	struct ADAPTER *prAdapter)
{
	struct GLUE_INFO *prGlueInfo;
	struct GL_HIF_INFO *prHifInfo;
	struct ERR_RECOVERY_CTRL_T *prErrRecoveryCtrl;
	uint32_t u4Status = 0;
	uint32_t u4HostWpdamBase = 0;

	if (prAdapter->prGlueInfo == NULL) {
		DBGLOG(HAL, ERROR, "prGlueInfo is NULL\n");
		return;
	}

	prGlueInfo = prAdapter->prGlueInfo;
	prHifInfo = &prGlueInfo->rHifInfo;
	prErrRecoveryCtrl = &prHifInfo->rErrRecoveryCtl;

	if (prAdapter->chip_info->is_support_wfdma1)
		u4HostWpdamBase = CONNAC3X_HOST_WPDMA_1_BASE;
	else
		u4HostWpdamBase = CONNAC3X_HOST_WPDMA_0_BASE;

	kalDevRegRead(prGlueInfo,
		CONNAC3X_WPDMA_MCU2HOST_SW_INT_STA(u4HostWpdamBase),
		&u4Status);

	prErrRecoveryCtrl->u4BackupStatus = u4Status;
	if (u4Status & ERROR_DETECT_MASK) {
		prErrRecoveryCtrl->u4Status = u4Status;
		kalDevRegWrite(prGlueInfo,
			CONNAC3X_WPDMA_MCU2HOST_SW_INT_STA(u4HostWpdamBase),
			u4Status);
		halHwRecoveryFromError(prAdapter);
	} else {
		kalDevRegWrite(prGlueInfo,
			CONNAC3X_WPDMA_MCU2HOST_SW_INT_STA(u4HostWpdamBase),
			u4Status);
		DBGLOG(HAL, TRACE, "undefined SER status[0x%x].\n", u4Status);
	}
}


void asicConnac3xSoftwareInterruptMcu(
	struct ADAPTER *prAdapter, u_int32_t intrBitMask)
{
	struct GLUE_INFO *prGlueInfo;
	uint32_t u4McuWpdamBase = 0;

	if (prAdapter == NULL || prAdapter->prGlueInfo == NULL) {
		DBGLOG(HAL, ERROR, "prAdapter or prGlueInfo is NULL\n");
		return;
	}

	prGlueInfo = prAdapter->prGlueInfo;
	if (prAdapter->chip_info->is_support_wfdma1)
		u4McuWpdamBase = CONNAC3X_MCU_WPDMA_1_BASE;
	else
		u4McuWpdamBase = CONNAC3X_MCU_WPDMA_0_BASE;
	kalDevRegWrite(prGlueInfo,
		CONNAC3X_WPDMA_HOST2MCU_SW_INT_SET(u4McuWpdamBase),
		intrBitMask);
}


void asicConnac3xHifRst(
	struct GLUE_INFO *prGlueInfo)
{
	uint32_t u4HifRstCr;

	u4HifRstCr = asicConnac3xWfdmaHifRstAddrGet(prGlueInfo, 0);
	/* Reset dmashdl and wpdma */
	kalDevRegWrite(prGlueInfo, u4HifRstCr, 0x00000000);
	kalDevRegWrite(prGlueInfo, u4HifRstCr, 0x00000030);

	u4HifRstCr = asicConnac3xWfdmaHifRstAddrGet(prGlueInfo, 1);
	/* Reset dmashdl and wpdma */
	kalDevRegWrite(prGlueInfo, u4HifRstCr, 0x00000000);
	kalDevRegWrite(prGlueInfo, u4HifRstCr, 0x00000030);
}
#endif /* _HIF_PCIE */

#if defined(_HIF_USB)
/*
 * tx_ring config
 * 1.1tx_ring_ext_ctrl
 * 7c025600[31:0]: 0x00800004 (ring 0 BASE_PTR & max_cnt for EP4)
 * 7c025604[31:0]: 0x00c00004 (ring 1 BASE_PTR & max_cnt for EP5)
 * 7c025608[31:0]: 0x01000004 (ring 2 BASE_PTR & max_cnt for EP6)
 * 7c02560c[31:0]: 0x01400004 (ring 3 BASE_PTR & max_cnt for EP7)
 * 7c025610[31:0]: 0x01800004 (ring 4 BASE_PTR & max_cnt for EP9)
 * 7c025640[31:0]: 0x02800004 (ring 16 BASE_PTR & max_cnt for EP4/FWDL)
 * 7c02563C[31:0]: 0x02c00004 (ring 15 BASE_PTR & max_cnt for EP8/WMCPU)
 * 7c025650[31:0]: 0x03800004 (ring 20 BASE_PTR & max_cnt for EP8/WACPU)
 *
 * WFDMA_GLO_CFG Setting
 * 2.1 WFDMA_GLO_CFG: 7c025208[28][27]=2'b11;
 * 2.2 WFDMA_GLO_CFG: 7c025208[20]=1'b1;
 * 2.3 WFDMA_GLO_CFG: 7c025208[9]=1'b1;
 *
 * 3.	trx_dma_en:
 * 3.1 WFDMA_GLO_CFG: 7c025208[2][0]=1'b1;
 */
void asicConnac3xWfdmaInitForUSB(
	struct ADAPTER *prAdapter,
	struct mt66xx_chip_info *prChipInfo)
{
	struct BUS_INFO *prBusInfo;
	uint32_t idx;
	uint32_t u4WfdmaAddr, u4WfdmaCr;

	prBusInfo = prChipInfo->bus_info;

	/* HAL_MCR_RD(prAdapter, 0x7c00e400, &u4WfdmaCr); */
	/* HAL_MCR_WR(prAdapter, 0x7c00e400, 0xFF); */

	HAL_MCR_RD(prAdapter, 0x7c021100, &u4WfdmaCr);
	HAL_MCR_WR(prAdapter, 0x7c021100, 0x0);

	if (prChipInfo->is_support_wfdma1) {
		u4WfdmaAddr =
		CONNAC3X_TX_RING_EXT_CTRL_BASE(CONNAC3X_HOST_WPDMA_1_BASE);
	} else {
		u4WfdmaAddr =
		CONNAC3X_TX_RING_EXT_CTRL_BASE(CONNAC3X_HOST_WPDMA_0_BASE);
	}
	/*
	 * HOST_DMA1_WPDMA_TX_RING0_EXT_CTRL ~ HOST_DMA1_WPDMA_TX_RING4_EXT_CTRL
	 */
	for (idx = 0; idx < USB_TX_EPOUT_NUM; idx++) {
		HAL_MCR_RD(prAdapter, u4WfdmaAddr + (idx*4), &u4WfdmaCr);
		u4WfdmaCr &= ~CONNAC3X_WFDMA_DISP_MAX_CNT_MASK;
		u4WfdmaCr |= CONNAC3X_TX_RING_DISP_MAX_CNT;
		u4WfdmaCr &= ~CONNAC3X_WFDMA_DISP_BASE_PTR_MASK;
		u4WfdmaCr |= (0x008 + 0x4 * idx)<<20;
		HAL_MCR_WR(prAdapter, u4WfdmaAddr + (idx*4), u4WfdmaCr);
	}

	/* HOST_DMA1_WPDMA_TX_RING16_EXT_CTRL_ADDR */
	HAL_MCR_RD(prAdapter, u4WfdmaAddr + 0x40, &u4WfdmaCr);
	u4WfdmaCr &= ~CONNAC3X_WFDMA_DISP_MAX_CNT_MASK;
	u4WfdmaCr |= CONNAC3X_TX_RING_DISP_MAX_CNT;
	u4WfdmaCr &= ~CONNAC3X_WFDMA_DISP_BASE_PTR_MASK;
	u4WfdmaCr |= 0x02800000;
	HAL_MCR_WR(prAdapter, u4WfdmaAddr + 0x40, u4WfdmaCr);

	/* HOST_DMA1_WPDMA_TX_RING15_EXT_CTRL_ADDR */
	HAL_MCR_RD(prAdapter, u4WfdmaAddr + 0x3c, &u4WfdmaCr);
	u4WfdmaCr &= ~CONNAC3X_WFDMA_DISP_MAX_CNT_MASK;
	u4WfdmaCr |= CONNAC3X_TX_RING_DISP_MAX_CNT;
	u4WfdmaCr &= ~CONNAC3X_WFDMA_DISP_BASE_PTR_MASK;
	u4WfdmaCr |= 0x02c00000;
	HAL_MCR_WR(prAdapter, u4WfdmaAddr + 0x3c, u4WfdmaCr);


	if (prChipInfo->is_support_wacpu) {
		/* HOST_DMA1_WPDMA_TX_RING20_EXT_CTRL_ADDR */
		HAL_MCR_RD(prAdapter, u4WfdmaAddr + 0x50, &u4WfdmaCr);
		u4WfdmaCr &= ~CONNAC3X_WFDMA_DISP_MAX_CNT_MASK;
		u4WfdmaCr |= CONNAC3X_TX_RING_DISP_MAX_CNT;
		u4WfdmaCr &= ~CONNAC3X_WFDMA_DISP_BASE_PTR_MASK;
		u4WfdmaCr |= 0x03800000;
		HAL_MCR_WR(prAdapter, u4WfdmaAddr + 0x50, u4WfdmaCr);
	}

	if (prChipInfo->is_support_wfdma1) {
		u4WfdmaAddr =
			CONNAC3X_WPDMA_GLO_CFG(CONNAC3X_HOST_WPDMA_1_BASE);
		HAL_MCR_RD(prAdapter, u4WfdmaAddr, &u4WfdmaCr);
		u4WfdmaCr |=
			(CONNAC3X_WPDMA1_GLO_CFG_OMIT_TX_INFO |
			 CONNAC3X_WPDMA1_GLO_CFG_OMIT_RX_INFO |
			 CONNAC3X_WPDMA1_GLO_CFG_FW_DWLD_Bypass_dmashdl |
			 CONNAC3X_WPDMA1_GLO_CFG_RX_DMA_EN |
			 CONNAC3X_WPDMA1_GLO_CFG_TX_DMA_EN);
		HAL_MCR_WR(prAdapter, u4WfdmaAddr, u4WfdmaCr);

		/* Enable WFDMA0 RX for receiving data frame */
		u4WfdmaAddr =
			CONNAC3X_WPDMA_GLO_CFG(CONNAC3X_HOST_WPDMA_0_BASE);
		HAL_MCR_RD(prAdapter, u4WfdmaAddr, &u4WfdmaCr);
		u4WfdmaCr |=
			(CONNAC3X_WPDMA1_GLO_CFG_RX_DMA_EN);
		HAL_MCR_WR(prAdapter, u4WfdmaAddr, u4WfdmaCr);
	} else {
		u4WfdmaAddr =
			CONNAC3X_WPDMA_GLO_CFG(CONNAC3X_HOST_WPDMA_0_BASE);
		HAL_MCR_RD(prAdapter, u4WfdmaAddr, &u4WfdmaCr);
		u4WfdmaCr &= ~(CONNAC3X_WPDMA1_GLO_CFG_OMIT_RX_INFO);
		u4WfdmaCr |=
			(CONNAC3X_WPDMA1_GLO_CFG_OMIT_TX_INFO |
			 CONNAC3X_WPDMA1_GLO_CFG_OMIT_RX_INFO_PFET2 |
			 CONNAC3X_WPDMA1_GLO_CFG_FW_DWLD_Bypass_dmashdl |
			 CONNAC3X_WPDMA1_GLO_CFG_RX_DMA_EN |
			 CONNAC3X_WPDMA1_GLO_CFG_TX_DMA_EN);
		HAL_MCR_WR(prAdapter, u4WfdmaAddr, u4WfdmaCr);

	}

	prChipInfo->is_support_dma_shdl = wlanCfgGetUint32(prAdapter,
				    "DmaShdlEnable",
				    FEATURE_ENABLED);
	if (!prChipInfo->is_support_dma_shdl) {
		/*
		 *	To disable 0x7C0252B0[6] DMASHDL
		 */
		if (prChipInfo->is_support_wfdma1) {
			u4WfdmaAddr = CONNAC3X_WPDMA_GLO_CFG_EXT0(
					CONNAC3X_HOST_WPDMA_1_BASE);
		} else {
			u4WfdmaAddr = CONNAC3X_WPDMA_GLO_CFG_EXT0(
					CONNAC3X_HOST_WPDMA_0_BASE);
		}
		HAL_MCR_RD(prAdapter, u4WfdmaAddr, &u4WfdmaCr);
		u4WfdmaCr &= ~CONNAC3X_WPDMA1_GLO_CFG_EXT0_TX_DMASHDL_EN;
		HAL_MCR_WR(prAdapter, u4WfdmaAddr, u4WfdmaCr);

		/*
		 *	[28]DMASHDL_BYPASS
		 *	DMASHDL host ask and quota control function bypass
		 *	0: Disable
		 *	1: Enable
		 */
		u4WfdmaAddr = CONNAC3X_HOST_DMASHDL_SW_CONTROL(
					CONNAC3X_HOST_DMASHDL);
		HAL_MCR_RD(prAdapter, u4WfdmaAddr, &u4WfdmaCr);
		u4WfdmaCr |= CONNAC3X_HIF_DMASHDL_BYPASS_EN;
		HAL_MCR_WR(prAdapter, u4WfdmaAddr, u4WfdmaCr);
	}

	if (prChipInfo->asicUsbInit_ic_specific)
		prChipInfo->asicUsbInit_ic_specific(prAdapter, prChipInfo);
}

uint8_t asicConnac3xUsbEventEpDetected(IN struct ADAPTER *prAdapter)
{
	return USB_DATA_EP_IN;
}

void asicConnac3xEnableUsbCmdTxRing(
	struct ADAPTER *prAdapter,
	u_int8_t ucDstRing)
{
	struct GLUE_INFO *prGlueInfo;
	struct BUS_INFO *prBusInfo;
	uint32_t u4Value = 0;
	uint32_t u4WfdmaValue = 0;
	uint32_t i = 0;
	uint32_t dma_base;

	ASSERT(prAdapter);

	prGlueInfo = prAdapter->prGlueInfo;
	prBusInfo = prAdapter->chip_info->bus_info;
	if (prAdapter->chip_info->is_support_wfdma1)
		dma_base = CONNAC3X_HOST_WPDMA_1_BASE;
	else
		dma_base = CONNAC3X_HOST_WPDMA_0_BASE;

	HAL_MCR_RD(prAdapter, CONNAC3X_WFDMA_HOST_CONFIG_ADDR, &u4WfdmaValue);
	u4WfdmaValue &= ~CONNAC3X_WFDMA_HOST_CONFIG_USB_CMDPKT_DST_MASK;
	u4WfdmaValue |= CONNAC3X_WFDMA_HOST_CONFIG_USB_CMDPKT_DST(ucDstRing);
	do {
		HAL_MCR_RD(prAdapter, CONNAC3X_WPDMA_GLO_CFG(dma_base),
			&u4Value);
		if ((u4Value & CONNAC3X_WPDMA1_GLO_CFG_TX_DMA_BUSY) == 0) {
			u4Value &= ~CONNAC3X_WPDMA1_GLO_CFG_TX_DMA_EN;
			HAL_MCR_WR(prAdapter, CONNAC3X_WPDMA_GLO_CFG(dma_base),
				u4Value);
			break;
		}
		kalUdelay(1000);
	} while ((i++) < 100);
	if (i > 100)
		DBGLOG(HAL, ERROR, "WFDMA1 TX keep busy....\n");
	else {
		HAL_MCR_WR(prAdapter,
				CONNAC3X_WFDMA_HOST_CONFIG_ADDR,
				u4WfdmaValue);
		HAL_MCR_RD(prAdapter, CONNAC3X_WPDMA_GLO_CFG(dma_base),
			&u4Value);
		u4Value |= CONNAC3X_WPDMA1_GLO_CFG_TX_DMA_EN;
		HAL_MCR_WR(prAdapter, CONNAC3X_WPDMA_GLO_CFG(dma_base),
			u4Value);
	}
}

#if CFG_ENABLE_FW_DOWNLOAD
void asicConnac3xEnableUsbFWDL(
	struct ADAPTER *prAdapter,
	u_int8_t fgEnable)
{
	struct GLUE_INFO *prGlueInfo;
	struct BUS_INFO *prBusInfo;
	struct mt66xx_chip_info *prChipInfo;

	uint32_t u4Value = 0;

	ASSERT(prAdapter);

	prGlueInfo = prAdapter->prGlueInfo;
	prChipInfo = prAdapter->chip_info;
	prBusInfo = prChipInfo->bus_info;

	HAL_MCR_RD(prAdapter, prBusInfo->u4UdmaTxQsel, &u4Value);
	if (fgEnable)
		u4Value |= FW_DL_EN;
	else
		u4Value &= ~FW_DL_EN;

	HAL_MCR_WR(prAdapter, prBusInfo->u4UdmaTxQsel, u4Value);

	if (prChipInfo->is_support_wacpu) {
		/* command packet forward to TX ring 17 (WMCPU) or
		 *    TX ring 20 (WACPU)
		 */
		asicConnac3xEnableUsbCmdTxRing(prAdapter,
			fgEnable ? CONNAC3X_USB_CMDPKT2WM :
				   CONNAC3X_USB_CMDPKT2WA);
	}
}
#endif /* CFG_ENABLE_FW_DOWNLOAD */

void asicConnac3xUdmaRxFlush(
	struct ADAPTER *prAdapter,
	u_int8_t bEnable)
{
	struct BUS_INFO *prBusInfo;
	uint32_t u4Value;

	prBusInfo = prAdapter->chip_info->bus_info;

	HAL_MCR_RD(prAdapter, prBusInfo->u4UdmaWlCfg_0_Addr,
		   &u4Value);
	if (bEnable)
		u4Value |= UDMA_WLCFG_0_RX_FLUSH_MASK;
	else
		u4Value &= ~UDMA_WLCFG_0_RX_FLUSH_MASK;
	HAL_MCR_WR(prAdapter, prBusInfo->u4UdmaWlCfg_0_Addr,
		   u4Value);
}

uint16_t asicConnac3xUsbRxByteCount(
	struct ADAPTER *prAdapter,
	struct BUS_INFO *prBusInfo,
	uint8_t *pRXD)
{

	uint16_t u2RxByteCount;
	uint8_t ucPacketType;

	ucPacketType = HAL_MAC_CONNAC3X_RX_STATUS_GET_PKT_TYPE(
		(struct HW_MAC_CONNAC3X_RX_DESC *)pRXD);
	u2RxByteCount = HAL_MAC_CONNAC3X_RX_STATUS_GET_RX_BYTE_CNT(
		(struct HW_MAC_CONNAC3X_RX_DESC *)pRXD);

	u2RxByteCount = ALIGN_16(u2RxByteCount) + 12;

	return u2RxByteCount;
}

#endif /* _HIF_USB */

void fillConnac3xTxDescTxByteCount(
	struct ADAPTER *prAdapter,
	struct MSDU_INFO *prMsduInfo,
	void *prTxDesc)
{
	struct mt66xx_chip_info *prChipInfo;
	uint32_t u4TxByteCount = NIC_TX_DESC_LONG_FORMAT_LENGTH;

	ASSERT(prAdapter);
	ASSERT(prMsduInfo);
	ASSERT(prTxDesc);

	prChipInfo = prAdapter->chip_info;
	u4TxByteCount += prMsduInfo->u2FrameLength;

	if (prMsduInfo->ucPacketType == TX_PACKET_TYPE_DATA)
		u4TxByteCount += prChipInfo->u4ExtraTxByteCount;

	/* Calculate Tx byte count */
	HAL_MAC_CONNAC3X_TXD_SET_TX_BYTE_COUNT(
		(struct HW_MAC_CONNAC3X_TX_DESC *)prTxDesc, u4TxByteCount);
}

void asicConnac3xInitTxdHook(
	struct TX_DESC_OPS_T *prTxDescOps)
{
	ASSERT(prTxDescOps);
	prTxDescOps->nic_txd_long_format_op = nic_txd_v3_long_format_op;
	prTxDescOps->nic_txd_tid_op = nic_txd_v3_tid_op;
	prTxDescOps->nic_txd_queue_idx_op = nic_txd_v3_queue_idx_op;
#if (CFG_TCP_IP_CHKSUM_OFFLOAD == 1)
	prTxDescOps->nic_txd_chksum_op = nic_txd_v3_chksum_op;
#endif /* CFG_TCP_IP_CHKSUM_OFFLOAD == 1 */
	prTxDescOps->nic_txd_header_format_op = nic_txd_v3_header_format_op;
	prTxDescOps->nic_txd_fill_by_pkt_option = nic_txd_v3_fill_by_pkt_option;
	prTxDescOps->nic_txd_compose = nic_txd_v3_compose;
	prTxDescOps->nic_txd_compose_security_frame =
		nic_txd_v3_compose_security_frame;
	prTxDescOps->nic_txd_set_pkt_fixed_rate_option_full =
		nic_txd_v3_set_pkt_fixed_rate_option_full;
	prTxDescOps->nic_txd_set_pkt_fixed_rate_option =
		nic_txd_v3_set_pkt_fixed_rate_option;
	prTxDescOps->nic_txd_set_hw_amsdu_template =
		nic_txd_v3_set_hw_amsdu_template;
}

void asicConnac3xInitRxdHook(
	struct RX_DESC_OPS_T *prRxDescOps)
{
	ASSERT(prRxDescOps);
	prRxDescOps->nic_rxd_get_rx_byte_count = nic_rxd_v3_get_rx_byte_count;
	prRxDescOps->nic_rxd_get_pkt_type = nic_rxd_v3_get_packet_type;
	prRxDescOps->nic_rxd_get_wlan_idx = nic_rxd_v3_get_wlan_idx;
	prRxDescOps->nic_rxd_get_sec_mode = nic_rxd_v3_get_sec_mode;
	prRxDescOps->nic_rxd_get_sw_class_error_bit =
		nic_rxd_v3_get_sw_class_error_bit;
	prRxDescOps->nic_rxd_get_ch_num = nic_rxd_v3_get_ch_num;
	prRxDescOps->nic_rxd_get_rf_band = nic_rxd_v3_get_rf_band;
	prRxDescOps->nic_rxd_get_tcl = nic_rxd_v3_get_tcl;
	prRxDescOps->nic_rxd_get_ofld = nic_rxd_v3_get_ofld;
	prRxDescOps->nic_rxd_get_HdrTrans = nic_rxd_v3_get_HdrTrans;
	prRxDescOps->nic_rxd_fill_rfb = nic_rxd_v3_fill_rfb;
	prRxDescOps->nic_rxd_sanity_check = nic_rxd_v3_sanity_check;
	prRxDescOps->nic_rxd_check_wakeup_reason =
		nic_rxd_v3_check_wakeup_reason;
#if (CFG_SUPPORT_SNIFFER_RADIOTAP == 1)
	prRxDescOps->nic_rxd_fill_radiotap = nic_rxd_v3_fill_radiotap;
#endif
	prRxDescOps->nic_rxd_handle_host_rpt =
		nic_rxd_v3_handle_host_rpt;
}

#if (CFG_SUPPORT_MSP == 1)
void asicConnac3xRxProcessRxvforMSP(IN struct ADAPTER *prAdapter,
	  IN OUT struct SW_RFB *prRetSwRfb) {
	struct HW_MAC_RX_STS_GROUP_3_V2 *prGroup3;

	if (prRetSwRfb->ucStaRecIdx >= CFG_STA_REC_NUM) {
		DBGLOG(RX, LOUD,
		"prRetSwRfb->ucStaRecIdx(%d) >= CFG_STA_REC_NUM(%d)\n",
			prRetSwRfb->ucStaRecIdx, CFG_STA_REC_NUM);
		return;
	}

	if (CONNAC3X_RXV_FROM_RX_RPT(prAdapter))
		return;

	prGroup3 =
		(struct HW_MAC_RX_STS_GROUP_3_V2 *)prRetSwRfb->prRxStatusGroup3;
	if (prRetSwRfb->ucGroupVLD & BIT(RX_GROUP_VLD_3)) {
		/* P-RXV1[0:31] */
		prAdapter->arStaRec[
			prRetSwRfb->ucStaRecIdx].u4RxVector0 =
			CONNAC3X_HAL_RX_VECTOR_GET_RX_VECTOR(prGroup3, 0);
		prAdapter->arStaRec[
			prRetSwRfb->ucStaRecIdx].u4RxVector4 =
			CONNAC3X_HAL_RX_VECTOR_GET_RX_VECTOR(prGroup3, 1);
	} else {
		prAdapter->arStaRec[
			prRetSwRfb->ucStaRecIdx].u4RxVector0 = 0;
		prAdapter->arStaRec[
			prRetSwRfb->ucStaRecIdx].u4RxVector4 = 0;
	}

	if (prRetSwRfb->ucGroupVLD & BIT(RX_GROUP_VLD_5)) {
		/* C-B-0[0:31] */
		prAdapter->arStaRec[
			prRetSwRfb->ucStaRecIdx].u4RxVector1 =
			CONNAC3X_HAL_RX_VECTOR_GET_RX_VECTOR(
			prRetSwRfb->prRxStatusGroup5, 0);
		/* C-B-1[0:31] */
		prAdapter->arStaRec[
			prRetSwRfb->ucStaRecIdx].u4RxVector2 =
			CONNAC3X_HAL_RX_VECTOR_GET_RX_VECTOR(
			prRetSwRfb->prRxStatusGroup5, 2);
		/* C-B-2[0:31] */
		prAdapter->arStaRec[
			prRetSwRfb->ucStaRecIdx].u4RxVector3 =
			CONNAC3X_HAL_RX_VECTOR_GET_RX_VECTOR(
			prRetSwRfb->prRxStatusGroup5, 4);
		/* C-B-3[0:31] */
		prAdapter->arStaRec[
			prRetSwRfb->ucStaRecIdx].u4RxVector4 =
			CONNAC3X_HAL_RX_VECTOR_GET_RX_VECTOR(
			prRetSwRfb->prRxStatusGroup5, 6);
	} else {
		prAdapter->arStaRec[
			prRetSwRfb->ucStaRecIdx].u4RxVector1 = 0;
		prAdapter->arStaRec[
			prRetSwRfb->ucStaRecIdx].u4RxVector2 = 0;
		prAdapter->arStaRec[
			prRetSwRfb->ucStaRecIdx].u4RxVector3 = 0;
	}
}
#endif /* CFG_SUPPORT_MSP == 1 */

uint8_t asicConnac3xRxGetRcpiValueFromRxv(
	IN uint8_t ucRcpiMode,
	IN struct SW_RFB *prSwRfb,
	IN struct ADAPTER *prAdapter)
{
	uint8_t ucRcpi0, ucRcpi1, ucRcpi2, ucRcpi3;
	uint8_t ucRcpiValue = 0;
	/* falcon IP donot have this field 'ucRxNum' */
	/* uint8_t ucRxNum; */

	ASSERT(prSwRfb);

	if (ucRcpiMode >= RCPI_MODE_NUM) {
		DBGLOG(RX, WARN,
		       "Rcpi Mode = %d is invalid for getting uint8_t value from RXV\n",
		       ucRcpiMode);
		return 0;
	}

	ucRcpi0 = HAL_RX_STATUS_GET_RCPI0(
			  (struct HW_MAC_RX_STS_GROUP_3_V2 *)
			  prSwRfb->prRxStatusGroup3);
	ucRcpi1 = HAL_RX_STATUS_GET_RCPI1(
			  (struct HW_MAC_RX_STS_GROUP_3_V2 *)
			  prSwRfb->prRxStatusGroup3);
	ucRcpi2 = HAL_RX_STATUS_GET_RCPI2(
			  (struct HW_MAC_RX_STS_GROUP_3_V2 *)
			  prSwRfb->prRxStatusGroup3);
	ucRcpi3 = HAL_RX_STATUS_GET_RCPI3(
			  (struct HW_MAC_RX_STS_GROUP_3_V2 *)
			  prSwRfb->prRxStatusGroup3);

	/*If Rcpi is not available, set it to zero*/
	if (ucRcpi0 == RCPI_MEASUREMENT_NOT_AVAILABLE)
		ucRcpi0 = RCPI_LOW_BOUND;
	if (ucRcpi1 == RCPI_MEASUREMENT_NOT_AVAILABLE)
		ucRcpi1 = RCPI_LOW_BOUND;
	DBGLOG(RX, TRACE, "RCPI WF0:%d WF1:%d WF2:%d WF3:%d\n",
	       ucRcpi0, ucRcpi1, ucRcpi2, ucRcpi3);

	switch (ucRcpiMode) {
	case RCPI_MODE_WF0:
		ucRcpiValue = ucRcpi0;
		break;

	case RCPI_MODE_WF1:
		ucRcpiValue = ucRcpi1;
		break;

	case RCPI_MODE_WF2:
		ucRcpiValue = ucRcpi2;
		break;

	case RCPI_MODE_WF3:
		ucRcpiValue = ucRcpi3;
		break;

	case RCPI_MODE_AVG: /*Not recommended for CBW80+80*/
		ucRcpiValue = (ucRcpi0 + ucRcpi1) / 2;
		break;

	case RCPI_MODE_MAX:
		ucRcpiValue =
			(ucRcpi0 > ucRcpi1) ? (ucRcpi0) : (ucRcpi1);
		break;

	case RCPI_MODE_MIN:
		ucRcpiValue =
			(ucRcpi0 < ucRcpi1) ? (ucRcpi0) : (ucRcpi1);
		break;

	default:
		break;
	}

	return ucRcpiValue;
}

#if (CFG_SUPPORT_PERF_IND == 1)
void asicConnac3xRxPerfIndProcessRXV(IN struct ADAPTER *prAdapter,
			       IN struct SW_RFB *prSwRfb,
			       IN uint8_t ucBssIndex)
{
	struct HW_MAC_RX_STS_GROUP_3 *prRxStatusGroup3;
	uint8_t ucRCPI0 = 0, ucRCPI1 = 0;

	ASSERT(prAdapter);
	ASSERT(prSwRfb);
	/* REMOVE DATA RATE Parsing Logic:Workaround only for 6885*/
	/* Since MT6885 can not get Rx Data Rate dur to RXV HW Bug*/

	if (ucBssIndex >= BSSID_NUM)
		return;

	/* can't parse radiotap info if no rx vector */
	if (((prSwRfb->ucGroupVLD & BIT(RX_GROUP_VLD_2)) == 0)
		|| ((prSwRfb->ucGroupVLD & BIT(RX_GROUP_VLD_3)) == 0)) {
		return;
	}

	prRxStatusGroup3 = prSwRfb->prRxStatusGroup3;

	/* RCPI */
	ucRCPI0 = HAL_RX_STATUS_GET_RCPI0(prRxStatusGroup3);
	ucRCPI1 = HAL_RX_STATUS_GET_RCPI1(prRxStatusGroup3);

	/* Record peak rate to Traffic Indicator*/
	prAdapter->prGlueInfo->PerfIndCache.
		ucCurRxRCPI0[ucBssIndex] = ucRCPI0;
	prAdapter->prGlueInfo->PerfIndCache.
		ucCurRxRCPI1[ucBssIndex] = ucRCPI1;

}
#endif

#if (CFG_CHIP_RESET_SUPPORT == 1) && (CFG_WMT_RESET_API_SUPPORT == 0)
u_int8_t conn2_rst_L0_notify_step2(void)
{
	if (glGetRstReason() == RST_BT_TRIGGER) {
		typedef void (*p_bt_fun_type) (void);
		p_bt_fun_type bt_func;
		char *bt_func_name = "WF_rst_L0_notify_BT_step2";

		bt_func = (p_bt_fun_type)(uintptr_t)
				GLUE_LOOKUP_FUN(bt_func_name);
		if (bt_func) {
			bt_func();
		} else {
			DBGLOG(INIT, WARN, "[SER][L0] %s does not exist\n",
							bt_func_name);
			return FALSE;
		}
	} else {
		/* if wifi trigger, then wait bt ready notify */
		DBGLOG(INIT, WARN, "[SER][L0] not support..\n");
	}

	return TRUE;
}
#endif

#if (CFG_DOWNLOAD_DYN_MEMORY_MAP == 1)
uint32_t downloadImgByDynMemMap(IN struct ADAPTER *prAdapter,
	IN uint32_t u4Addr, IN uint32_t u4Len,
	IN uint8_t *pucStartPtr, IN enum ENUM_IMG_DL_IDX_T eDlIdx)
{
	struct BUS_INFO *prBusInfo;
	uint32_t u4ReMapReg = 0;
#if defined(_HIF_PCIE)
	uint32_t u4OrigValue = 0, u4ChangedValue = 0;
#endif

	if (eDlIdx != IMG_DL_IDX_PATCH && eDlIdx != IMG_DL_IDX_N9_FW)
		return WLAN_STATUS_NOT_SUPPORTED;

	DBGLOG(INIT, INFO, "u4Addr: 0x%x, u4Len: %u, eDlIdx: %u\n",
			u4Addr, u4Len, eDlIdx);

	prBusInfo = prAdapter->chip_info->bus_info;

#if defined(_HIF_PCIE)
	/* To access 0x1850_xxxx, 0x7C500000 will be used, whose bus address is
	 * 0x50000. By default, bus address 0x50000 will be remapped to
	 * 0x1845_xxxx, we need to change this setting to 0x1850_xxxx by
	 * register PCIE2AP_REMAP_2.
	 * Set R_PCIE2AP_PUBLIC_REMAPPING_5[31:16] as 0x1850.
	 */
	HAL_MCR_RD(prAdapter, prBusInfo->pcie2ap_remap_2,
			&u4OrigValue);
	u4ChangedValue = (u4OrigValue & BITS(15, 0)) | ((0x1850) << 16);
	HAL_MCR_WR(prAdapter, prBusInfo->pcie2ap_remap_2,
			u4ChangedValue);
#endif

	HAL_MCR_WR(prAdapter, prBusInfo->ap2wf_remap_1,
			u4Addr);

	if (!halChipToStaticMapBusAddr(prAdapter->chip_info,
			CONN_INFRA_CFG_AP2WF_BUS_ADDR,
			&u4ReMapReg)) {
		DBGLOG(INIT, ERROR, "map bus address fail.\n");
		return WLAN_STATUS_FAILURE;
	}

	RTMP_IO_MEM_COPY(&prAdapter->prGlueInfo->rHifInfo, u4ReMapReg,
				pucStartPtr, u4Len);

#if defined(_HIF_PCIE)
	HAL_MCR_WR(prAdapter, prBusInfo->pcie2ap_remap_2,
			u4OrigValue);
#endif

	return WLAN_STATUS_SUCCESS;
}
#endif

void asicConnac3xDmashdlSetPlePktMaxPage(struct ADAPTER *prAdapter,
				      uint16_t u2MaxPage)
{
	struct BUS_INFO *prBusInfo;
	struct DMASHDL_CFG *prCfg;
	uint32_t u4Val = 0;

	prBusInfo = prAdapter->chip_info->bus_info;
	prCfg = prBusInfo->prDmashdlCfg;

	HAL_MCR_RD(prAdapter, prCfg->rPlePacketMaxSize.u4Addr, &u4Val);

	u4Val &= ~prCfg->rPlePacketMaxSize.u4Mask;
	u4Val |= (u2MaxPage << prCfg->rPlePacketMaxSize.u4Shift) &
		prCfg->rPlePacketMaxSize.u4Mask;

	HAL_MCR_WR(prAdapter, prCfg->rPlePacketMaxSize.u4Addr, u4Val);
}

void asicConnac3xDmashdlSetPsePktMaxPage(struct ADAPTER *prAdapter,
				      uint16_t u2MaxPage)
{
	struct BUS_INFO *prBusInfo;
	struct DMASHDL_CFG *prCfg;
	uint32_t u4Val = 0;

	prBusInfo = prAdapter->chip_info->bus_info;
	prCfg = prBusInfo->prDmashdlCfg;

	HAL_MCR_RD(prAdapter, prCfg->rPsePacketMaxSize.u4Addr, &u4Val);

	u4Val &= ~prCfg->rPsePacketMaxSize.u4Mask;
	u4Val |= (u2MaxPage << prCfg->rPsePacketMaxSize.u4Shift) &
		 prCfg->rPsePacketMaxSize.u4Mask;

	HAL_MCR_WR(prAdapter, prCfg->rPsePacketMaxSize.u4Addr, u4Val);
}

void asicConnac3xDmashdlGetPktMaxPage(struct ADAPTER *prAdapter)
{
	struct BUS_INFO *prBusInfo;
	struct DMASHDL_CFG *prCfg;
	uint32_t u4Val = 0;
	uint32_t ple_pkt_max_sz;
	uint32_t pse_pkt_max_sz;

	prBusInfo = prAdapter->chip_info->bus_info;
	prCfg = prBusInfo->prDmashdlCfg;

	HAL_MCR_RD(prAdapter, prCfg->rPlePacketMaxSize.u4Addr, &u4Val);

	ple_pkt_max_sz = (u4Val & prCfg->rPlePacketMaxSize.u4Mask) >>
		prCfg->rPlePacketMaxSize.u4Shift;
	pse_pkt_max_sz = (u4Val & prCfg->rPsePacketMaxSize.u4Mask) >>
		prCfg->rPsePacketMaxSize.u4Shift;

	DBGLOG(HAL, INFO, "DMASHDL PLE_PACKET_MAX_SIZE (0x%08x): 0x%08x\n",
		prCfg->rPlePacketMaxSize.u4Addr, u4Val);
	DBGLOG(HAL, INFO, "PLE/PSE packet max size=0x%03x/0x%03x\n",
		ple_pkt_max_sz, pse_pkt_max_sz);

}
void asicConnac3xDmashdlSetRefill(struct ADAPTER *prAdapter, uint8_t ucGroup,
			       u_int8_t fgEnable)
{
	struct BUS_INFO *prBusInfo;
	struct DMASHDL_CFG *prCfg;
	uint32_t u4Mask;
	uint32_t u4Val = 0;

	prBusInfo = prAdapter->chip_info->bus_info;
	prCfg = prBusInfo->prDmashdlCfg;

	if (ucGroup >= prCfg->u4GroupNum)
		ASSERT(0);

	u4Mask = prCfg->rGroup0RefillDisable.u4Mask << ucGroup;

	HAL_MCR_RD(prAdapter, prCfg->rGroup0RefillDisable.u4Addr, &u4Val);

	if (fgEnable)
		u4Val &= ~u4Mask;
	else
		u4Val |= u4Mask;

	HAL_MCR_WR(prAdapter, prCfg->rGroup0RefillDisable.u4Addr, u4Val);
}

void asicConnac3xDmashdlGetRefill(struct ADAPTER *prAdapter)
{
	struct BUS_INFO *prBusInfo;
	struct DMASHDL_CFG *prCfg;
	uint32_t u4Val = 0;

	prBusInfo = prAdapter->chip_info->bus_info;
	prCfg = prBusInfo->prDmashdlCfg;

	HAL_MCR_RD(prAdapter, prCfg->rGroup0RefillDisable.u4Addr, &u4Val);
	DBGLOG(HAL, INFO, "DMASHDL ReFill Control (0x%08x): 0x%08x\n",
		prCfg->rGroup0RefillDisable.u4Addr, u4Val);
}

void asicConnac3xDmashdlSetMaxQuota(struct ADAPTER *prAdapter, uint8_t ucGroup,
				 uint16_t u2MaxQuota)
{
	struct BUS_INFO *prBusInfo;
	struct DMASHDL_CFG *prCfg;
	uint32_t u4Addr;
	uint32_t u4Val = 0;

	prBusInfo = prAdapter->chip_info->bus_info;
	prCfg = prBusInfo->prDmashdlCfg;

	if (ucGroup >= prCfg->u4GroupNum)
		ASSERT(0);

	u4Addr = prCfg->rGroup0ControlMaxQuota.u4Addr + (ucGroup << 2);

	HAL_MCR_RD(prAdapter, u4Addr, &u4Val);

	u4Val &= ~prCfg->rGroup0ControlMaxQuota.u4Mask;
	u4Val |= (u2MaxQuota << prCfg->rGroup0ControlMaxQuota.u4Shift) &
		prCfg->rGroup0ControlMaxQuota.u4Mask;

	HAL_MCR_WR(prAdapter, u4Addr, u4Val);
}

void asicConnac3xDmashdlSetMinQuota(struct ADAPTER *prAdapter, uint8_t ucGroup,
				 uint16_t u2MinQuota)
{
	struct BUS_INFO *prBusInfo;
	struct DMASHDL_CFG *prCfg;
	uint32_t u4Addr;
	uint32_t u4Val = 0;

	prBusInfo = prAdapter->chip_info->bus_info;
	prCfg = prBusInfo->prDmashdlCfg;

	if (ucGroup >= prCfg->u4GroupNum)
		ASSERT(0);

	u4Addr = prCfg->rGroup0ControlMinQuota.u4Addr + (ucGroup << 2);

	HAL_MCR_RD(prAdapter, u4Addr, &u4Val);

	u4Val &= ~prCfg->rGroup0ControlMinQuota.u4Mask;
	u4Val |= (u2MinQuota << prCfg->rGroup0ControlMinQuota.u4Shift) &
		prCfg->rGroup0ControlMinQuota.u4Mask;

	HAL_MCR_WR(prAdapter, u4Addr, u4Val);
}

void asicConnac3xDmashdlGetGroupControl(struct ADAPTER *prAdapter,
					uint8_t ucGroup)
{
	struct BUS_INFO *prBusInfo;
	struct DMASHDL_CFG *prCfg;
	uint32_t u4Addr;
	uint32_t u4Val = 0;
	uint32_t max_quota;
	uint32_t min_quota;

	prBusInfo = prAdapter->chip_info->bus_info;
	prCfg = prBusInfo->prDmashdlCfg;

	u4Addr = prCfg->rGroup0ControlMaxQuota.u4Addr + (ucGroup << 2);

	HAL_MCR_RD(prAdapter, u4Addr, &u4Val);

	max_quota = GET_DMASHDL_MAX_QUOTA_NUM(u4Val);
	min_quota = GET_DMASHDL_MIN_QUOTA_NUM(u4Val);
	DBGLOG(HAL, INFO, "\tDMASHDL Group%d control(0x%08x): 0x%08x\n",
		ucGroup, u4Addr, u4Val);
	DBGLOG(HAL, INFO, "\tmax/min quota = 0x%03x/ 0x%03x\n",
		max_quota, min_quota);

}
void asicConnac3xDmashdlSetQueueMapping(struct ADAPTER *prAdapter,
					uint8_t ucQueue,
					uint8_t ucGroup)
{
	struct BUS_INFO *prBusInfo;
	struct DMASHDL_CFG *prCfg;
	uint32_t u4Addr, u4Mask, u4Shft;
	uint32_t u4Val = 0;

	prBusInfo = prAdapter->chip_info->bus_info;
	prCfg = prBusInfo->prDmashdlCfg;

	if (ucQueue >= 32)
		ASSERT(0);

	if (ucGroup >= prCfg->u4GroupNum)
		ASSERT(0);

	u4Addr = prCfg->rQueueMapping0Queue0.u4Addr + ((ucQueue >> 3) << 2);
	u4Mask = prCfg->rQueueMapping0Queue0.u4Mask << ((ucQueue % 8) << 2);
	u4Shft = (ucQueue % 8) << 2;

	HAL_MCR_RD(prAdapter, u4Addr, &u4Val);

	u4Val &= ~u4Mask;
	u4Val |= (ucGroup << u4Shft) & u4Mask;

	HAL_MCR_WR(prAdapter, u4Addr, u4Val);
}

void asicConnac3xDmashdlSetSlotArbiter(struct ADAPTER *prAdapter,
				    u_int8_t fgEnable)
{
	struct BUS_INFO *prBusInfo;
	struct DMASHDL_CFG *prCfg;
	uint32_t u4Val = 0;

	prBusInfo = prAdapter->chip_info->bus_info;
	prCfg = prBusInfo->prDmashdlCfg;

	HAL_MCR_RD(prAdapter, prCfg->rPageSettingGroupSeqOrderType.u4Addr,
		   &u4Val);

	if (fgEnable)
		u4Val |= prCfg->rPageSettingGroupSeqOrderType.u4Mask;
	else
		u4Val &= ~prCfg->rPageSettingGroupSeqOrderType.u4Mask;

	HAL_MCR_WR(prAdapter, prCfg->rPageSettingGroupSeqOrderType.u4Addr,
		   u4Val);
}

void asicConnac3xDmashdlSetUserDefinedPriority(struct ADAPTER *prAdapter,
					    uint8_t ucPriority, uint8_t ucGroup)
{
	struct BUS_INFO *prBusInfo;
	struct DMASHDL_CFG *prCfg;
	uint32_t u4Addr, u4Mask, u4Shft;
	uint32_t u4Val = 0;

	prBusInfo = prAdapter->chip_info->bus_info;
	prCfg = prBusInfo->prDmashdlCfg;

	ASSERT(ucPriority < 16);
	ASSERT(ucGroup < prCfg->u4GroupNum);

	u4Addr = prCfg->rSchdulerSetting0Priority0Group.u4Addr +
		((ucPriority >> 3) << 2);
	u4Mask = prCfg->rSchdulerSetting0Priority0Group.u4Mask <<
		((ucPriority % 8) << 2);
	u4Shft = (ucPriority % 8) << 2;

	HAL_MCR_RD(prAdapter, u4Addr, &u4Val);

	u4Val &= ~u4Mask;
	u4Val |= (ucGroup << u4Shft) & u4Mask;

	HAL_MCR_WR(prAdapter, u4Addr, u4Val);
}

uint32_t asicConnac3xDmashdlGetRsvCount(struct ADAPTER *prAdapter,
					uint8_t ucGroup)
{
	struct BUS_INFO *prBusInfo;
	struct DMASHDL_CFG *prCfg;
	uint32_t u4Addr;
	uint32_t u4Val = 0;
	uint32_t rsv_cnt = 0;

	prBusInfo = prAdapter->chip_info->bus_info;
	prCfg = prBusInfo->prDmashdlCfg;

	u4Addr = prCfg->rStatusRdGp0RsvCnt.u4Addr + (ucGroup << 2);

	HAL_MCR_RD(prAdapter, u4Addr, &u4Val);

	rsv_cnt = (u4Val & prCfg->rStatusRdGp0RsvCnt.u4Mask) >>
		prCfg->rStatusRdGp0RsvCnt.u4Shift;

	DBGLOG(HAL, INFO, "\tDMASHDL Status_RD_GP%d(0x%08x): 0x%08x\n",
		ucGroup, u4Addr, u4Val);
	DBGLOG(HAL, TRACE, "\trsv_cnt = 0x%03x\n", rsv_cnt);
	return rsv_cnt;
}

uint32_t asicConnac3xDmashdlGetSrcCount(struct ADAPTER *prAdapter,
					uint8_t ucGroup)
{
	struct BUS_INFO *prBusInfo;
	struct DMASHDL_CFG *prCfg;
	uint32_t u4Addr;
	uint32_t u4Val = 0;
	uint32_t src_cnt = 0;

	prBusInfo = prAdapter->chip_info->bus_info;
	prCfg = prBusInfo->prDmashdlCfg;

	u4Addr = prCfg->rStatusRdGp0SrcCnt.u4Addr + (ucGroup << 2);

	HAL_MCR_RD(prAdapter, u4Addr, &u4Val);

	src_cnt = (u4Val & prCfg->rStatusRdGp0SrcCnt.u4Mask) >>
		prCfg->rStatusRdGp0SrcCnt.u4Shift;

	DBGLOG(HAL, TRACE, "\tsrc_cnt = 0x%03x\n", src_cnt);
	return src_cnt;
}

void asicConnac3xDmashdlGetPKTCount(struct ADAPTER *prAdapter, uint8_t ucGroup)
{
	struct BUS_INFO *prBusInfo;
	struct DMASHDL_CFG *prCfg;
	uint32_t u4Addr;
	uint32_t u4Val = 0;
	uint32_t pktin_cnt = 0;
	uint32_t ask_cnt = 0;

	prBusInfo = prAdapter->chip_info->bus_info;
	prCfg = prBusInfo->prDmashdlCfg;

	if ((ucGroup & 0x1) == 0)
		u4Addr = prCfg->rRdGroupPktCnt0.u4Addr + (ucGroup << 1);
	else
		u4Addr = prCfg->rRdGroupPktCnt0.u4Addr + ((ucGroup-1) << 1);

	HAL_MCR_RD(prAdapter, u4Addr, &u4Val);
	DBGLOG(HAL, INFO, "\tDMASHDL RD_group_pkt_cnt_%d(0x%08x): 0x%08x\n",
		ucGroup / 2, u4Addr, u4Val);
	if ((ucGroup & 0x1) == 0) {
		pktin_cnt = GET_EVEN_GROUP_PKT_IN_CNT(u4Val);
		ask_cnt = GET_EVEN_GROUP_ASK_CNT(u4Val);
	} else {
		pktin_cnt = GET_ODD_GROUP_PKT_IN_CNT(u4Val);
		ask_cnt = GET_ODD_GROUP_ASK_CNT(u4Val);
	}
	DBGLOG(HAL, INFO, "\tpktin_cnt = 0x%02x, ask_cnt = 0x%02x",
		pktin_cnt, ask_cnt);
}

void asicConnac3xDmashdlSetOptionalControl(struct ADAPTER *prAdapter,
		uint16_t u2HifAckCntTh, uint16_t u2HifGupActMap)
{
	struct BUS_INFO *prBusInfo;
	struct DMASHDL_CFG *prCfg;
	uint32_t u4Addr, u4Val = 0;

	prBusInfo = prAdapter->chip_info->bus_info;
	prCfg = prBusInfo->prDmashdlCfg;

	u4Addr = prCfg->rOptionalControlCrHifAckCntTh.u4Addr;

	HAL_MCR_RD(prAdapter, u4Addr, &u4Val);

	u4Val &= ~prCfg->rOptionalControlCrHifAckCntTh.u4Mask;
	u4Val |= (u2HifAckCntTh <<
		  prCfg->rOptionalControlCrHifAckCntTh.u4Shift);

	u4Val &= ~prCfg->rOptionalControlCrHifGupActMap.u4Mask;
	u4Val |= (u2HifGupActMap <<
		  prCfg->rOptionalControlCrHifGupActMap.u4Shift);

	HAL_MCR_WR(prAdapter, u4Addr, u4Val);
}

#endif /* CFG_SUPPORT_CONNAC3X == 1 */
