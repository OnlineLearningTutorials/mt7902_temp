/* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause */
/*
 * Copyright (c) 2016 MediaTek Inc.
 */
/******************************************************************************
 *[File]             dbg_pdma.c
 *[Version]          v1.0
 *[Revision Date]    2015-09-08
 *[Author]
 *[Description]
 *    The program provides PDMA HIF APIs
 *[Copyright]
 *    Copyright (C) 2015 MediaTek Incorporation. All Rights Reserved.
 ******************************************************************************/

/*******************************************************************************
 *                         C O M P I L E R   F L A G S
 *******************************************************************************
 */

/*******************************************************************************
 *                    E X T E R N A L   R E F E R E N C E S
 *******************************************************************************
 */
#include "precomp.h"

#include "pse.h"
#include "wf_ple.h"
#include "host_csr.h"
#include "dma_sch.h"
#include "mt_dmac.h"

/*******************************************************************************
 *                              C O N S T A N T S
 *******************************************************************************
 */

/*******************************************************************************
 *                             D A T A   T Y P E S
 *******************************************************************************
 */

/*******************************************************************************
 *                            P U B L I C   D A T A
 *******************************************************************************
 */

/*******************************************************************************
 *                           P R I V A T E   D A T A
 *******************************************************************************
 */

/*******************************************************************************
 *                                 M A C R O S
 *******************************************************************************
 */

/*******************************************************************************
 *                   F U N C T I O N   D E C L A R A T I O N S
 *******************************************************************************
 */
static void halCheckHifState(struct ADAPTER *prAdapter);
static void halDumpHifDebugLog(struct ADAPTER *prAdapter);
static bool halIsTxHang(struct ADAPTER *prAdapter);

/*******************************************************************************
 *                              F U N C T I O N S
 *******************************************************************************
 */

void halPrintHifDbgInfo(IN struct ADAPTER *prAdapter)
{
	halCheckHifState(prAdapter);
	halDumpHifDebugLog(prAdapter);
}

static void halCheckHifState(struct ADAPTER *prAdapter)
{
	uint32_t u4DebugLevel = 0;
	if (prAdapter->u4HifChkFlag & HIF_CHK_TX_HANG) {
		if (halIsTxHang(prAdapter)) {
			DBGLOG(HAL, ERROR,
			       "Tx timeout, set hif debug info flag\n");
			wlanGetDriverDbgLevel(DBG_TX_IDX, &u4DebugLevel);
			if (u4DebugLevel & DBG_CLASS_TRACE)
				prAdapter->u4HifDbgFlag |= DEG_HIF_ALL;
			else
				halShowLitePleInfo(prAdapter);
		}
	}

	if (prAdapter->u4HifChkFlag & HIF_DRV_SER)
		halSetDrvSer(prAdapter);

	prAdapter->u4HifChkFlag = 0;
}

static void halDumpHifDebugLog(struct ADAPTER *prAdapter)
{
	struct GLUE_INFO *prGlueInfo = NULL;
	struct GL_HIF_INFO *prHifInfo = NULL;
	uint32_t u4Value = 0;
	bool fgIsClkEn = false;

	ASSERT(prAdapter);
	prGlueInfo = prAdapter->prGlueInfo;
	ASSERT(prGlueInfo);
	prHifInfo = &prGlueInfo->rHifInfo;

	/* Only dump all hif log once */
	if (prAdapter->u4HifDbgFlag & DEG_HIF_ALL) {
		if (!prAdapter->fgEnHifDbgInfo) {
			prAdapter->u4HifDbgFlag = 0;
			return;
		}
		prAdapter->fgEnHifDbgInfo = false;
	}

	/* Avoid register checking */
	prHifInfo->fgIsDumpLog = true;

	if (prAdapter->u4HifDbgFlag & (DEG_HIF_ALL | DEG_HIF_HOST_CSR))
		fgIsClkEn = halShowHostCsrInfo(prAdapter);
	else {
		HAL_MCR_WR(prAdapter, HOST_CSR_DRIVER_OWN_INFO, 0x00030000);
		kalUdelay(1);
		HAL_MCR_RD(prAdapter, HOST_CSR_DRIVER_OWN_INFO, &u4Value);

		/* check clock is enabled */
		fgIsClkEn = ((u4Value & BIT(17)) != 0) &&
			((u4Value & BIT(16)) != 0);
	}

	if (!fgIsClkEn)
		return;

	if (prAdapter->u4HifDbgFlag & (DEG_HIF_ALL | DEG_HIF_PDMA))
		halShowPdmaInfo(prAdapter);

	if (prAdapter->u4HifDbgFlag & (DEG_HIF_ALL | DEG_HIF_DMASCH))
		halShowDmaschInfo(prAdapter);

	if (prAdapter->u4HifDbgFlag & (DEG_HIF_ALL | DEG_HIF_PSE))
		halShowPseInfo(prAdapter);

	if (prAdapter->u4HifDbgFlag & (DEG_HIF_ALL | DEG_HIF_PLE))
		halShowPleInfo(prAdapter, TRUE);

	if (prAdapter->u4HifDbgFlag & (DEG_HIF_ALL | DEG_HIF_MAC))
		haldumpMacInfo(prAdapter);

	if (prAdapter->u4HifDbgFlag & (DEG_HIF_ALL | DEG_HIF_PHY))
		haldumpPhyInfo(prAdapter);

	prHifInfo->fgIsDumpLog = false;
	prAdapter->u4HifDbgFlag = 0;
}

static void halDumpTxRing(IN struct GLUE_INFO *prGlueInfo,
			  IN uint16_t u2Port, IN uint32_t u4Idx)
{
	struct GL_HIF_INFO *prHifInfo = &prGlueInfo->rHifInfo;
	struct RTMP_TX_RING *prTxRing;
	struct TXD_STRUCT *pTxD;

	if (u2Port >= NUM_OF_TX_RING || u4Idx >= TX_RING_SIZE) {
		DBGLOG(HAL, INFO, "Dump fail u2Port[%u] u4Idx[%u]\n",
		       u2Port, u4Idx);
		return;
	}

	prTxRing = &prHifInfo->TxRing[u2Port];

	pTxD = (struct TXD_STRUCT *) prTxRing->Cell[u4Idx].AllocVa;

	log_dbg(SW4, INFO, "TX Ring[%u] Idx[%04u] SDP0[0x%08x] SDL0[%u] LS[%u] B[%u] DDONE[%u] SDP0_EXT[%u]\n",
		u2Port, u4Idx, pTxD->SDPtr0, pTxD->SDLen0, pTxD->LastSec0,
		pTxD->Burst, pTxD->DMADONE, pTxD->SDPtr0Ext);
}

uint32_t halDumpHifStatus(IN struct ADAPTER *prAdapter,
	IN uint8_t *pucBuf, IN uint32_t u4Max)
{
	struct GLUE_INFO *prGlueInfo = prAdapter->prGlueInfo;
	struct GL_HIF_INFO *prHifInfo = &prGlueInfo->rHifInfo;
	uint32_t u4Idx, u4DmaIdx = 0, u4CpuIdx = 0, u4MaxCnt = 0;
	uint32_t u4Len = 0;
	struct RTMP_TX_RING *prTxRing;
	struct RTMP_RX_RING *prRxRing;

	LOGBUF(pucBuf, u4Max, u4Len, "\n------<Dump HIF Status>------\n");

	for (u4Idx = 0; u4Idx < NUM_OF_TX_RING; u4Idx++) {
		prTxRing = &prHifInfo->TxRing[u4Idx];
		kalDevRegRead(prGlueInfo, prTxRing->hw_cnt_addr, &u4MaxCnt);
		kalDevRegRead(prGlueInfo, prTxRing->hw_cidx_addr, &u4CpuIdx);
		kalDevRegRead(prGlueInfo, prTxRing->hw_didx_addr, &u4DmaIdx);

		u4MaxCnt &= MT_RING_CNT_MASK;
		u4CpuIdx &= MT_RING_CIDX_MASK;
		u4DmaIdx &= MT_RING_DIDX_MASK;

		LOGBUF(pucBuf, u4Max, u4Len,
			"TX[%u] SZ[%04u] CPU[%04u/%04u] DMA[%04u/%04u] SW_UD[%04u] Used[%u]\n",
			u4Idx, u4MaxCnt, prTxRing->TxCpuIdx,
			u4CpuIdx, prTxRing->TxDmaIdx,
			u4DmaIdx, prTxRing->TxSwUsedIdx, prTxRing->u4UsedCnt);

		if (u4Idx == TX_RING_DATA0_IDX_0) {
			halDumpTxRing(prGlueInfo, u4Idx, prTxRing->TxCpuIdx);
			halDumpTxRing(prGlueInfo, u4Idx, u4CpuIdx);
			halDumpTxRing(prGlueInfo, u4Idx, u4DmaIdx);
			halDumpTxRing(prGlueInfo, u4Idx, prTxRing->TxSwUsedIdx);
		}

		if (u4Idx == TX_RING_DATA1_IDX_1) {
			halDumpTxRing(prGlueInfo, u4Idx, prTxRing->TxCpuIdx);
			halDumpTxRing(prGlueInfo, u4Idx, u4CpuIdx);
			halDumpTxRing(prGlueInfo, u4Idx, u4DmaIdx);
			halDumpTxRing(prGlueInfo, u4Idx, prTxRing->TxSwUsedIdx);
		}
	}

	for (u4Idx = 0; u4Idx < NUM_OF_RX_RING; u4Idx++) {
		prRxRing = &prHifInfo->RxRing[u4Idx];

		kalDevRegRead(prGlueInfo, prRxRing->hw_cnt_addr, &u4MaxCnt);
		kalDevRegRead(prGlueInfo, prRxRing->hw_cidx_addr, &u4CpuIdx);
		kalDevRegRead(prGlueInfo, prRxRing->hw_didx_addr, &u4DmaIdx);

		u4MaxCnt &= MT_RING_CNT_MASK;
		u4CpuIdx &= MT_RING_CIDX_MASK;
		u4DmaIdx &= MT_RING_DIDX_MASK;

		LOGBUF(pucBuf, u4Max, u4Len,
		       "RX[%u] SZ[%04u] CPU[%04u/%04u] DMA[%04u/%04u]\n",
		       u4Idx, u4MaxCnt, prRxRing->RxCpuIdx, u4CpuIdx,
		       prRxRing->RxDmaIdx, u4DmaIdx);
	}

	LOGBUF(pucBuf, u4Max, u4Len, "MSDU Tok: Free[%u] Used[%u]\n",
		halGetMsduTokenFreeCnt(prGlueInfo->prAdapter),
		prGlueInfo->rHifInfo.rTokenInfo.u4UsedCnt);
	LOGBUF(pucBuf, u4Max, u4Len, "Pending QLen Normal[%u] Sec[%u]\n",
		prGlueInfo->i4TxPendingFrameNum,
		prGlueInfo->i4TxPendingSecurityFrameNum);

	LOGBUF(pucBuf, u4Max, u4Len, "---------------------------------\n\n");

	return u4Len;
}

/*----------------------------------------------------------------------------*/
/*!
 * @brief Compare two struct timeval
 *
 * @param prTs1          a pointer to timeval
 * @param prTs2          a pointer to timeval
 *
 *
 * @retval 0             two time value is equal
 * @retval 1             prTs1 value > prTs2 value
 * @retval -1            prTs1 value < prTs2 value
 */
/*----------------------------------------------------------------------------*/
#if KERNEL_VERSION(5, 0, 0) <= LINUX_VERSION_CODE
int halTimeCompare(struct timespec64 *prTs1, struct timespec64 *prTs2)
#else
int halTimeCompare(struct timeval *prTs1, struct timeval *prTs2)
#endif
{
	if (prTs1->tv_sec > prTs2->tv_sec)
		return 1;
	else if (prTs1->tv_sec < prTs2->tv_sec)
		return -1;
	/* sec part is equal */
#if KERNEL_VERSION(5, 0, 0) <= LINUX_VERSION_CODE
	else if (prTs1->tv_nsec > prTs2->tv_nsec)
		return 1;
	else if (prTs1->tv_nsec < prTs2->tv_nsec)
		return -1;
#else
	else if (prTs1->tv_usec > prTs2->tv_usec)
		return 1;
	else if (prTs1->tv_usec < prTs2->tv_usec)
		return -1;
#endif
	return 0;
}

/*----------------------------------------------------------------------------*/
/*!
 * @brief Checking tx hang
 *
 * @param prAdapter      a pointer to adapter private data structure.
 *
 * @retval true          tx is hang because msdu report too long
 */
/*----------------------------------------------------------------------------*/
static bool halIsTxHang(struct ADAPTER *prAdapter)
{
	struct MSDU_TOKEN_INFO *prTokenInfo;
	struct MSDU_TOKEN_ENTRY *prToken;
	struct MSDU_INFO *prMsduInfo;
#if KERNEL_VERSION(5, 0, 0) <= LINUX_VERSION_CODE
	struct timespec64 rNowTs, rTime, rLongest, rTimeout;
#else
	struct timeval rNowTs, rTime, rLongest, rTimeout;
#endif
	uint32_t u4Idx = 0, u4TokenId = 0;
	bool fgIsTimeout = false;
	struct WIFI_VAR *prWifiVar;

	ASSERT(prAdapter);
	ASSERT(prAdapter->prGlueInfo);

	prTokenInfo = &prAdapter->prGlueInfo->rHifInfo.rTokenInfo;
	prWifiVar = &prAdapter->rWifiVar;

	rTimeout.tv_sec = prWifiVar->ucMsduReportTimeout;
	rLongest.tv_sec = 0;
#if KERNEL_VERSION(5, 0, 0) <= LINUX_VERSION_CODE
	rTimeout.tv_nsec = 0;
	rLongest.tv_nsec = 0;
	ktime_get_real_ts64(&rNowTs);
#else
	rTimeout.tv_usec = 0;
	rLongest.tv_usec = 0;
	do_gettimeofday(&rNowTs);
#endif

	for (u4Idx = 0; u4Idx < HIF_TX_MSDU_TOKEN_NUM; u4Idx++) {
		prToken = &prTokenInfo->arToken[u4Idx];
		prMsduInfo = prToken->prMsduInfo;
		if (!prToken->fgInUsed || !prMsduInfo)
			continue;

		/* check tx hang is enabled */
		if ((prAdapter->u4TxHangFlag &
		      BIT(prMsduInfo->ucBssIndex)) == 0)
			continue;

		/* Ignore now time < token time */
		if (halTimeCompare(&rNowTs, &prToken->rTs) < 0)
			continue;

#if KERNEL_VERSION(5, 0, 0) <= LINUX_VERSION_CODE
		rTime = timespec64_sub(rNowTs, prToken->rTs);
#else
		rTime.tv_sec = rNowTs.tv_sec - prToken->rTs.tv_sec;
		rTime.tv_usec = rNowTs.tv_usec;
		if (prToken->rTs.tv_usec > rNowTs.tv_usec) {
			rTime.tv_sec -= 1;
			rTime.tv_usec += SEC_TO_USEC(1);
		}
		rTime.tv_usec -= prToken->rTs.tv_usec;
#endif

		if (halTimeCompare(&rTime, &rTimeout) >= 0)
			fgIsTimeout = true;

		/* rTime > rLongest */
		if (halTimeCompare(&rTime, &rLongest) > 0) {
			rLongest.tv_sec = rTime.tv_sec;
#if KERNEL_VERSION(5, 0, 0) <= LINUX_VERSION_CODE
			rLongest.tv_nsec = rTime.tv_nsec;
#else
			rLongest.tv_usec = rTime.tv_usec;
#endif
			u4TokenId = u4Idx;
		}
	}

	if (fgIsTimeout) {
		DBGLOG(HAL, INFO, "TokenId[%u] timeout[sec:%ld, usec:%ld]\n",
#if KERNEL_VERSION(5, 0, 0) <= LINUX_VERSION_CODE
		       u4TokenId, rLongest.tv_sec, rLongest.tv_nsec/1000);
#else
		       u4TokenId, rLongest.tv_sec, rLongest.tv_usec);
#endif
		prToken = &prTokenInfo->arToken[u4TokenId];
		if (prToken->prPacket)
			DBGLOG_MEM32(HAL, INFO, prToken->prPacket, 64);
	}

	return fgIsTimeout;
}

void kalDumpTxRing(struct GLUE_INFO *prGlueInfo,
		   struct RTMP_TX_RING *prTxRing,
		   uint32_t u4Num, bool fgDumpContent)
{
	struct GL_HIF_INFO *prHifInfo = NULL;
	struct HIF_MEM_OPS *prMemOps;
	struct RTMP_DMACB *pTxCell;
	struct TXD_STRUCT *pTxD;
	uint32_t u4DumpLen = 64;

	ASSERT(prGlueInfo);
	prHifInfo = &prGlueInfo->rHifInfo;
	prMemOps = &prHifInfo->rMemOps;

	if (u4Num >= TX_RING_SIZE)
		return;

	pTxCell = &prTxRing->Cell[u4Num];
	pTxD = (struct TXD_STRUCT *) pTxCell->AllocVa;

	if (!pTxD)
		return;

	DBGLOG(HAL, INFO, "Tx Dese Num[%u]\n", u4Num);
	DBGLOG_MEM32(HAL, INFO, pTxD, sizeof(struct TXD_STRUCT));

	if (!fgDumpContent)
		return;

	DBGLOG(HAL, INFO, "Tx Contents\n");
	if (prMemOps->dumpTx)
		prMemOps->dumpTx(prHifInfo, prTxRing, u4Num, u4DumpLen);
	DBGLOG(HAL, INFO, "\n\n");
}

void kalDumpRxRing(struct GLUE_INFO *prGlueInfo,
		   struct RTMP_RX_RING *prRxRing,
		   uint32_t u4Num, bool fgDumpContent)
{
	struct GL_HIF_INFO *prHifInfo = NULL;
	struct HIF_MEM_OPS *prMemOps;
	struct RTMP_DMACB *pRxCell;
	struct RXD_STRUCT *pRxD;
	uint32_t u4DumpLen = 64;

	ASSERT(prGlueInfo);
	prHifInfo = &prGlueInfo->rHifInfo;
	prMemOps = &prHifInfo->rMemOps;

	if (u4Num >= prRxRing->u4RingSize)
		return;

	pRxCell = &prRxRing->Cell[u4Num];
	pRxD = (struct RXD_STRUCT *) pRxCell->AllocVa;

	if (!pRxD)
		return;

	DBGLOG(HAL, INFO, "Rx Dese Num[%u]\n", u4Num);
	DBGLOG_MEM32(HAL, INFO, pRxD, sizeof(struct RXD_STRUCT));

	if (!fgDumpContent)
		return;

	if (u4DumpLen > pRxD->SDLen0)
		u4DumpLen = pRxD->SDLen0;

	DBGLOG(HAL, INFO, "Rx Contents\n");
	if (prMemOps->dumpRx)
		prMemOps->dumpRx(prHifInfo, prRxRing, u4Num, u4DumpLen);
	DBGLOG(HAL, INFO, "\n\n");
}

void halShowPdmaInfo(IN struct ADAPTER *prAdapter)
{
	uint32_t i = 0, u4Value = 0;
	uint32_t Base[6], Base_Ext[6], Cnt[6], Cidx[6], Didx[6];
	uint32_t offset, offset_ext, SwIdx;
	char buf[100] = {0};
	struct GL_HIF_INFO *prHifInfo = NULL;
	struct BUS_INFO *prBus_info;
	struct RTMP_TX_RING *prTxRing;
	struct RTMP_RX_RING *prRxRing;
	int ret;

	/* PDMA HOST_INT */
	HAL_MCR_RD(prAdapter, WPDMA_INT_STA, &u4Value);
	DBGLOG(HAL, INFO, "WPDMA HOST_INT:0x%08x = 0x%08x\n",
		WPDMA_INT_STA, u4Value);

	/* PDMA GLOBAL_CFG  */
	HAL_MCR_RD(prAdapter, WPDMA_GLO_CFG, &u4Value);
	DBGLOG(HAL, INFO, "WPDMA GLOBAL_CFG:0x%08x = 0x%08x\n",
		WPDMA_GLO_CFG, u4Value);

	HAL_MCR_RD(prAdapter, CONN_HIF_RST, &u4Value);
	DBGLOG(HAL, INFO, "WPDMA CONN_HIF_RST:0x%08x = 0x%08x\n",
		CONN_HIF_RST, u4Value);

	HAL_MCR_RD(prAdapter, MCU2HOST_SW_INT_STA, &u4Value);
	DBGLOG(HAL, INFO, "WPDMA MCU2HOST_SW_INT_STA:0x%08x = 0x%08x\n",
		MCU2HOST_SW_INT_STA, u4Value);

	/* PDMA Tx/Rx Ring  Info */
	prBus_info = prAdapter->chip_info->bus_info;
	DBGLOG(HAL, INFO, "Tx Ring configuration\n");
	DBGLOG(HAL, INFO, "%10s%12s%18s%10s%10s%10s\n",
		"Tx Ring", "Reg", "Base", "Cnt", "CIDX", "DIDX");

	for (i = 0; i < 4; i++) {
		if (i == 0) {
			offset = prBus_info->tx_ring0_data_idx *
						MT_RINGREG_DIFF;
			offset_ext = prBus_info->tx_ring0_data_idx *
					MT_RINGREG_EXT_DIFF;
		} else if (i == 1) {
			offset = prBus_info->tx_ring1_data_idx *
					MT_RINGREG_DIFF;
			offset_ext = prBus_info->tx_ring1_data_idx *
					MT_RINGREG_EXT_DIFF;
		} else if (i == 2) {
			offset = prBus_info->tx_ring_fwdl_idx * MT_RINGREG_DIFF;
			offset_ext = prBus_info->tx_ring_fwdl_idx *
					MT_RINGREG_EXT_DIFF;
		} else {
			offset = prBus_info->tx_ring_cmd_idx * MT_RINGREG_DIFF;
			offset_ext = prBus_info->tx_ring_cmd_idx *
					MT_RINGREG_EXT_DIFF;
		}

		HAL_MCR_RD(prAdapter, WPDMA_TX_RING0_CTRL0 + offset, &Base[i]);
		HAL_MCR_RD(prAdapter, WPDMA_TX_RING0_BASE_PTR_EXT + offset_ext,
				&Base_Ext[i]);
		HAL_MCR_RD(prAdapter, WPDMA_TX_RING0_CTRL1 + offset, &Cnt[i]);
		HAL_MCR_RD(prAdapter, WPDMA_TX_RING0_CTRL2 + offset, &Cidx[i]);
		HAL_MCR_RD(prAdapter, WPDMA_TX_RING0_CTRL3 + offset, &Didx[i]);

		ret = kalSnprintf(buf, sizeof(buf),
			"%10d  0x%08x  0x%016llx%10d%10d%10d",
			offset/MT_RINGREG_DIFF, WPDMA_TX_RING0_CTRL0 + offset,
			(Base[i] + ((uint64_t) Base_Ext[i] << 32)),
			Cnt[i], Cidx[i], Didx[i]);
		if (ret >= 0 || ret < sizeof(buf))
			DBGLOG(HAL, INFO, "%s\n", buf);
		else
			DBGLOG(INIT, ERROR,
					"[%u] kalSnprintf failed, ret: %d\n",
					__LINE__, ret);
	}

	DBGLOG(HAL, INFO, "Rx Ring configuration\n");
	DBGLOG(HAL, INFO, "%10s%12s%18s%10s%10s%10s\n",
		"Rx Ring", "Reg", "Base", "Cnt", "CIDX", "DIDX");
	HAL_MCR_RD(prAdapter, WPDMA_RX_RING0_CTRL0, &Base[4]);
	HAL_MCR_RD(prAdapter, WPDMA_RX_RING0_BASE_PTR_EXT, &Base_Ext[4]);
	HAL_MCR_RD(prAdapter, WPDMA_RX_RING0_CTRL1, &Cnt[4]);
	HAL_MCR_RD(prAdapter, WPDMA_RX_RING0_CTRL2, &Cidx[4]);
	HAL_MCR_RD(prAdapter, WPDMA_RX_RING0_CTRL3, &Didx[4]);
	ret = kalSnprintf(buf, sizeof(buf),
		"%10d  0x%08x  0x%016llx%10d%10d%10d",
		0, WPDMA_RX_RING0_CTRL0,
		(Base[4] + ((uint64_t)Base_Ext[4] << 32)),
		Cnt[4], Cidx[4], Didx[4]);
	if (ret >= 0 || ret < sizeof(buf))
		DBGLOG(HAL, INFO, "%s\n", buf);
	else
		DBGLOG(INIT, ERROR,
				"[%u] kalSnprintf failed, ret: %d\n",
				__LINE__, ret);

	HAL_MCR_RD(prAdapter, WPDMA_RX_RING0_CTRL0 + MT_RINGREG_DIFF, &Base[5]);
	HAL_MCR_RD(prAdapter, WPDMA_RX_RING0_BASE_PTR_EXT + MT_RINGREG_EXT_DIFF,
			&Base_Ext[5]);
	HAL_MCR_RD(prAdapter, WPDMA_RX_RING0_CTRL1 + MT_RINGREG_DIFF, &Cnt[5]);
	HAL_MCR_RD(prAdapter, WPDMA_RX_RING0_CTRL2 + MT_RINGREG_DIFF, &Cidx[5]);
	HAL_MCR_RD(prAdapter, WPDMA_RX_RING0_CTRL3 + MT_RINGREG_DIFF, &Didx[5]);
	ret = kalSnprintf(buf, sizeof(buf),
		"%10d  0x%08x  0x%016llx%10d%10d%10d",
		1, WPDMA_RX_RING0_CTRL0 + MT_RINGREG_DIFF,
		(Base[5] + ((uint64_t)Base_Ext[5] << 32)),
		Cnt[5], Cidx[5], Didx[5]);
	if (ret >= 0 || ret < sizeof(buf))
		DBGLOG(HAL, INFO, "%s\n", buf);
	else
		DBGLOG(INIT, ERROR,
				"[%u] kalSnprintf failed, ret: %d\n",
				__LINE__, ret);

	/* PDMA Tx/Rx descriptor & packet content */
	prHifInfo = &prAdapter->prGlueInfo->rHifInfo;

	for (i = 0; i < 3; i++) {
		DBGLOG(HAL, INFO, "Dump PDMA Tx Ring[%u]\n", i);
		prTxRing = &prHifInfo->TxRing[i];
		SwIdx = Didx[i];
		kalDumpTxRing(prAdapter->prGlueInfo, prTxRing,
			      SwIdx, true);
		SwIdx = Didx[i] == 0 ? Cnt[i] - 1 : Didx[i] - 1;
		kalDumpTxRing(prAdapter->prGlueInfo, prTxRing,
			      SwIdx, true);
	}

	for (i = 0; i < 2; i++) {
		DBGLOG(HAL, INFO, "Dump PDMA Rx Ring[%u]\n", i);
		prRxRing = &prHifInfo->RxRing[i];
		SwIdx = Didx[i+3];
		kalDumpRxRing(prAdapter->prGlueInfo, prRxRing,
			      SwIdx, true);
		SwIdx = Didx[i+3] == 0 ? Cnt[i+3] - 1 : Didx[i+3] - 1;
		kalDumpRxRing(prAdapter->prGlueInfo, prRxRing,
			      SwIdx, true);
	}

	/* PDMA Busy Status */
	HAL_MCR_RD(prAdapter, PDMA_DEBUG_BUSY_STATUS, &u4Value);
	DBGLOG(HAL, INFO, "PDMA busy status:0x%08x = 0x%08x\n",
		PDMA_DEBUG_STATUS, u4Value);
	HAL_MCR_RD(prAdapter, PDMA_DEBUG_HIF_BUSY_STATUS, &u4Value);
	DBGLOG(HAL, INFO, "CONN_HIF busy status:0x%08x = 0x%08x\n\n",
		PDMA_DEBUG_HIF_BUSY_STATUS, u4Value);

	/* PDMA Debug Flag Info */
	DBGLOG(HAL, INFO, "PDMA core dbg");
	for (i = 0; i < 24; i++) {
		u4Value = 256 + i;
		HAL_MCR_WR(prAdapter, PDMA_DEBUG_EN, u4Value);
		HAL_MCR_RD(prAdapter, PDMA_DEBUG_STATUS, &u4Value);
		DBGLOG(HAL, INFO, "Set:0x%02x, result = 0x%08x\n", i, u4Value);
		mdelay(1);
	}

	/* AXI Debug Flag */
	HAL_MCR_WR(prAdapter, AXI_DEBUG_DEBUG_EN, PDMA_AXI_DEBUG_FLAG);
	HAL_MCR_RD(prAdapter, CONN_HIF_DEBUG_STATUS, &u4Value);
	DBGLOG(HAL, INFO, "Set:0x%04x, pdma axi dbg:0x%08x",
	       PDMA_AXI_DEBUG_FLAG, u4Value);

	HAL_MCR_WR(prAdapter, AXI_DEBUG_DEBUG_EN, GALS_AXI_DEBUG_FLAG);
	HAL_MCR_RD(prAdapter, CONN_HIF_DEBUG_STATUS, &u4Value);
	DBGLOG(HAL, INFO, "Set:0x%04x, gals axi dbg:0x%08x",
	       GALS_AXI_DEBUG_FLAG, u4Value);

	HAL_MCR_WR(prAdapter, AXI_DEBUG_DEBUG_EN, MCU_AXI_DEBUG_FLAG);
	HAL_MCR_RD(prAdapter, CONN_HIF_DEBUG_STATUS, &u4Value);
	DBGLOG(HAL, INFO, "Set:0x%04x, mcu axi dbg:0x%08x",
	       MCU_AXI_DEBUG_FLAG, u4Value);

	/* Rbus Bridge Debug Flag */
	DBGLOG(HAL, INFO, "rbus dbg");
	HAL_MCR_WR(prAdapter, PDMA_DEBUG_EN, RBUS_DEBUG_FLAG);
	for (i = 0; i < 9; i++) {
		u4Value = i << 16;
		HAL_MCR_WR(prAdapter, AXI_DEBUG_DEBUG_EN, u4Value);
		HAL_MCR_RD(prAdapter, PDMA_DEBUG_STATUS, &u4Value);
		DBGLOG(HAL, INFO, "Set[19:16]:0x%02x, result = 0x%08x\n",
		       i, u4Value);
	}
	if (prAdapter->chip_info->prDebugOps->showHifInfo)
		prAdapter->chip_info->prDebugOps->showHifInfo(prAdapter);
}

bool halShowHostCsrInfo(IN struct ADAPTER *prAdapter)
{
	uint32_t i = 0, u4Value = 0;
	bool fgIsDriverOwn = false;
	bool fgEnClock = false;

	DBGLOG(HAL, INFO, "Host CSR Configuration Info:\n\n");

	HAL_MCR_RD(prAdapter, HOST_CSR_BASE, &u4Value);
	DBGLOG(HAL, INFO, "Get 0x87654321: 0x%08x = 0x%08x\n",
		HOST_CSR_BASE, u4Value);

	HAL_MCR_RD(prAdapter, HOST_CSR_DRIVER_OWN_INFO, &u4Value);
	DBGLOG(HAL, INFO, "Driver own info: 0x%08x = 0x%08x\n",
		HOST_CSR_BASE, u4Value);
	fgIsDriverOwn = (u4Value & PCIE_LPCR_HOST_SET_OWN) == 0;

	for (i = 0; i < 5; i++) {
		HAL_MCR_RD(prAdapter, HOST_CSR_MCU_PORG_COUNT, &u4Value);
		DBGLOG(HAL, INFO,
			"MCU programming Counter info (no sync): 0x%08x = 0x%08x\n",
			HOST_CSR_MCU_PORG_COUNT, u4Value);
	}

	HAL_MCR_RD(prAdapter, HOST_CSR_RGU, &u4Value);
	DBGLOG(HAL, INFO, "RGU Info: 0x%08x = 0x%08x\n", HOST_CSR_RGU, u4Value);

	HAL_MCR_RD(prAdapter, HOST_CSR_HIF_BUSY_CORQ_WFSYS_ON, &u4Value);
	DBGLOG(HAL, INFO, "HIF_BUSY / CIRQ / WFSYS_ON info: 0x%08x = 0x%08x\n",
		HOST_CSR_HIF_BUSY_CORQ_WFSYS_ON, u4Value);

	HAL_MCR_RD(prAdapter, HOST_CSR_PINMUX_MON_FLAG, &u4Value);
	DBGLOG(HAL, INFO, "Pinmux/mon_flag info: 0x%08x = 0x%08x\n",
		HOST_CSR_PINMUX_MON_FLAG, u4Value);

	HAL_MCR_RD(prAdapter, HOST_CSR_MCU_PWR_STAT, &u4Value);
	DBGLOG(HAL, INFO, "Bit[5] mcu_pwr_stat: 0x%08x = 0x%08x\n",
		HOST_CSR_MCU_PWR_STAT, u4Value);

	HAL_MCR_RD(prAdapter, HOST_CSR_FW_OWN_SET, &u4Value);
	DBGLOG(HAL, INFO, "Bit[15] fw_own_stat: 0x%08x = 0x%08x\n",
		HOST_CSR_FW_OWN_SET, u4Value);

	HAL_MCR_RD(prAdapter, HOST_CSR_MCU_SW_MAILBOX_0, &u4Value);
	DBGLOG(HAL, INFO, "WF Mailbox[0]: 0x%08x = 0x%08x\n",
		HOST_CSR_MCU_SW_MAILBOX_0, u4Value);

	HAL_MCR_RD(prAdapter, HOST_CSR_MCU_SW_MAILBOX_1, &u4Value);
	DBGLOG(HAL, INFO, "MCU Mailbox[1]: 0x%08x = 0x%08x\n",
		HOST_CSR_MCU_SW_MAILBOX_1, u4Value);

	HAL_MCR_RD(prAdapter, HOST_CSR_MCU_SW_MAILBOX_2, &u4Value);
	DBGLOG(HAL, INFO, "BT Mailbox[2]: 0x%08x = 0x%08x\n",
		HOST_CSR_MCU_SW_MAILBOX_2, u4Value);

	HAL_MCR_RD(prAdapter, HOST_CSR_MCU_SW_MAILBOX_3, &u4Value);
	DBGLOG(HAL, INFO, "GPS Mailbox[3]: 0x%08x = 0x%08x\n",
		HOST_CSR_MCU_SW_MAILBOX_3, u4Value);

	HAL_MCR_RD(prAdapter, HOST_CSR_CONN_CFG_ON, &u4Value);
	DBGLOG(HAL, INFO, "Conn_cfg_on info: 0x%08x = 0x%08x\n",
		HOST_CSR_CONN_CFG_ON, u4Value);

	HAL_MCR_WR(prAdapter, HOST_CSR_DRIVER_OWN_INFO, 0x00030000);
	kalUdelay(1);
	HAL_MCR_RD(prAdapter, HOST_CSR_DRIVER_OWN_INFO, &u4Value);
	DBGLOG(HAL, INFO, "Bit[17]/[16], Get HCLK info: 0x%08x = 0x%08x\n",
		HOST_CSR_DRIVER_OWN_INFO, u4Value);

	/* check clock is enabled */
	fgEnClock = ((u4Value & BIT(17)) != 0) && ((u4Value & BIT(16)) != 0);

	return fgIsDriverOwn && fgEnClock;
}

void haldumpPhyInfo(struct ADAPTER *prAdapter)
{
	uint32_t i = 0, value = 0;

	for (i = 0; i < 20; i++) {
		HAL_MCR_RD(prAdapter, 0x82072644, &value);
		DBGLOG(HAL, INFO, "0x82072644: 0x%08x\n", value);
		HAL_MCR_RD(prAdapter, 0x82072644, &value);
		DBGLOG(HAL, INFO, "0x82072654: 0x%08x\n", value);
		kalMdelay(1);
	}
}

