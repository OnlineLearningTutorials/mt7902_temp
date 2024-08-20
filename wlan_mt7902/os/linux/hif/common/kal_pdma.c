/* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause */
/*
 * Copyright (c) 2016 MediaTek Inc.
 */
/******************************************************************************
 *[File]             kal_pdma.c
 *[Version]          v1.0
 *[Revision Date]    2010-03-01
 *[Author]
 *[Description]
 *    The program provides PCIE HIF driver
 *[Copyright]
 *    Copyright (C) 2010 MediaTek Incorporation. All Rights Reserved.
 ******************************************************************************/


/*******************************************************************************
 *                         C O M P I L E R   F L A G S
 *******************************************************************************
 */

/*******************************************************************************
 *                    E X T E R N A L   R E F E R E N C E S
 *******************************************************************************
 */

#include "gl_os.h"

#include "hif_pdma.h"

#include "precomp.h"

#include <linux/mm.h>
#ifndef CONFIG_X86
#include <asm/memory.h>
#endif

#include "mt66xx_reg.h"

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
static bool kalDevWriteCmdByQueue(struct GLUE_INFO *prGlueInfo,
				  struct CMD_INFO *prCmdInfo, uint8_t ucTC);
static bool kalDevWriteDataByQueue(struct GLUE_INFO *prGlueInfo,
				   struct MSDU_INFO *prMsduInfo);
static bool kalDevKickMsduData(struct GLUE_INFO *prGlueInfo);
static bool kalDevKickAmsduData(struct GLUE_INFO *prGlueInfo);

/*******************************************************************************
 *                              F U N C T I O N S
 *******************************************************************************
 */

/*----------------------------------------------------------------------------*/
/*!
 * \brief Check connsys is dead or not
 *
 * \param[in] prGlueInfo Pointer to the GLUE_INFO_T structure.
 * \param[in] u4Register Register address
 * \param[in] pu4Value   Pointer to read value
 *
 * \retval TRUE          connsys is dead
 * \retval FALSE         connsys is alive
 */
/*----------------------------------------------------------------------------*/
static inline bool kalIsChipDead(struct GLUE_INFO *prGlueInfo,
				 uint32_t u4Register, uint32_t *pu4Value)
{
	struct GL_HIF_INFO *prHifInfo = NULL;
	uint32_t u4Value;
	uint32_t u4BusAddr;

	prHifInfo = &prGlueInfo->rHifInfo;

	if (*pu4Value != HIF_DEADFEED_VALUE)
		return false;

	if (!halChipToStaticMapBusAddr(prGlueInfo, CONN_CFG_CHIP_ID_ADDR,
				       &u4BusAddr)) {
		DBGLOG(HAL, ERROR, "Not exist CR read[0x%08x]\n", u4Register);
		return false;
	}

	RTMP_IO_READ32(prHifInfo, u4BusAddr, &u4Value);

	return u4Value == HIF_DEADFEED_VALUE;
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief Read a 32-bit device register
 *
 * \param[in] prGlueInfo Pointer to the GLUE_INFO_T structure.
 * \param[in] u4Register Register offset
 * \param[in] pu4Value   Pointer to variable used to store read value
 *
 * \retval TRUE          operation success
 * \retval FALSE         operation fail
 */
/*----------------------------------------------------------------------------*/
u_int8_t kalDevRegRead(IN struct GLUE_INFO *prGlueInfo,
	IN uint32_t u4Register, OUT uint32_t *pu4Value)
{
	struct GL_HIF_INFO *prHifInfo = NULL;
	struct ADAPTER *prAdapter = NULL;
	struct BUS_INFO *prBusInfo = NULL;
	uint32_t u4BusAddr = u4Register;

	ASSERT(prGlueInfo);
	ASSERT(pu4Value);

	prHifInfo = &prGlueInfo->rHifInfo;
	prAdapter = prGlueInfo->prAdapter;
	ASSERT(prAdapter);
	prBusInfo = prAdapter->chip_info->bus_info;

	if (!prHifInfo->fgIsDumpLog && prBusInfo->isValidRegAccess &&
	    !prBusInfo->isValidRegAccess(prAdapter, u4Register)) {
		/* Don't print log when resetting */
		if (!wlanIsChipNoAck(prAdapter)) {
			DBGLOG(HAL, ERROR,
			       "Invalid access! Get CR[0x%08x/0x%08x] value[0x%08x]\n",
			       u4Register, u4BusAddr, *pu4Value);
		}
		*pu4Value = HIF_DEADFEED_VALUE;
		return FALSE;
	}

	/* Static mapping */
	if (halChipToStaticMapBusAddr(prGlueInfo, u4Register, &u4BusAddr)) {
		RTMP_IO_READ32(prHifInfo, u4BusAddr, pu4Value);
		if (kalIsChipDead(prGlueInfo, u4Register, pu4Value)) {
			/* Don't print log when resetting */
			if (!wlanIsChipNoAck(prAdapter)) {
				DBGLOG(HAL, ERROR,
				       "Read register is deadfeed\n");
				GL_USER_DEFINE_RESET_TRIGGER(prAdapter,
							     RST_CR_ACCESS_FAIL,
						       RST_FLAG_DO_WHOLE_RESET);
			}
			return FALSE;
		}
	} else {
		DBGLOG(HAL, ERROR, "Not exist CR read[0x%08x]\n", u4Register);
	}

	return TRUE;
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief Write a 32-bit device register
 *
 * \param[in] prGlueInfo Pointer to the GLUE_INFO_T structure.
 * \param[in] u4Register Register offset
 * \param[in] u4Value    Value to be written
 *
 * \retval TRUE          operation success
 * \retval FALSE         operation fail
 */
/*----------------------------------------------------------------------------*/
u_int8_t kalDevRegWrite(IN struct GLUE_INFO *prGlueInfo,
	IN uint32_t u4Register, IN uint32_t u4Value)
{
	struct GL_HIF_INFO *prHifInfo = NULL;
	struct ADAPTER *prAdapter = NULL;
	struct BUS_INFO *prBusInfo = NULL;
	uint32_t u4BusAddr = u4Register;

	ASSERT(prGlueInfo);

	prHifInfo = &prGlueInfo->rHifInfo;
	prAdapter = prGlueInfo->prAdapter;
	ASSERT(prAdapter);
	prBusInfo = prAdapter->chip_info->bus_info;

	if (!prHifInfo->fgIsDumpLog && prBusInfo->isValidRegAccess &&
	    !prBusInfo->isValidRegAccess(prAdapter, u4Register)) {
		/* Don't print log when resetting */
		if (!wlanIsChipNoAck(prAdapter)) {
			DBGLOG(HAL, ERROR,
			       "Invalid access! Set CR[0x%08x/0x%08x] value[0x%08x]\n",
			       u4Register, u4BusAddr, u4Value);
		}
		return FALSE;
	}

	/* Static mapping */
	if (halChipToStaticMapBusAddr(prGlueInfo, u4Register, &u4BusAddr)) {
		RTMP_IO_WRITE32(prHifInfo, u4BusAddr, u4Value);
	} else {
		DBGLOG(HAL, ERROR, "Not exist CR write[0x%08x] value[0x%08x]\n",
		       u4Register, u4Value);
	}

	prHifInfo->u4HifCnt++;

	return TRUE;
}

static bool kalWaitRxDmaDone(struct GLUE_INFO *prGlueInfo,
			     struct RTMP_RX_RING *prRxRing,
			     struct RXD_STRUCT *pRxD,
			     uint16_t u2Port)
{
	uint32_t u4Count = 0;

	for (u4Count = 0; pRxD->DMADONE == 0; u4Count++) {
		kalDevRegRead(prGlueInfo, prRxRing->hw_didx_addr,
			      &prRxRing->RxDmaIdx);
		if (u4Count > DMA_DONE_WAITING_COUNT) {
			DBGLOG(HAL, INFO,
			       "Rx DMA done P[%u] DMA[%u] CPU[%u]\n",
			       u2Port, prRxRing->RxDmaIdx, prRxRing->RxCpuIdx);
			return false;
		}

		kalMdelay(DMA_DONE_WAITING_TIME);
	}
	return true;
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief Read device I/O port
 *
 * \param[in] prGlueInfo         Pointer to the GLUE_INFO_T structure.
 * \param[in] u2Port             I/O port offset
 * \param[in] u2Len              Length to be read
 * \param[out] pucBuf            Pointer to read buffer
 * \param[in] u2ValidOutBufSize  Length of the buffer valid to be accessed
 *
 * \retval TRUE          operation success
 * \retval FALSE         operation fail
 */
/*----------------------------------------------------------------------------*/
u_int8_t kalDevPortRead(IN struct GLUE_INFO *prGlueInfo,
	IN uint16_t u2Port, IN uint32_t u4Len,
	OUT uint8_t *pucBuf, IN uint32_t u4ValidOutBufSize)
{
	struct ADAPTER *prAdapter = NULL;
	struct GL_HIF_INFO *prHifInfo = NULL;
	struct HIF_MEM_OPS *prMemOps;
	struct RTMP_RX_RING *prRxRing;
	struct RTMP_DMACB *pRxCell;
	struct RXD_STRUCT *pRxD;
	struct RTMP_DMABUF *prDmaBuf;
	u_int8_t fgRet = TRUE;
	uint32_t u4CpuIdx = 0;

	ASSERT(prGlueInfo);
	ASSERT(pucBuf);
	ASSERT(u4Len <= u4ValidOutBufSize);

	prAdapter = prGlueInfo->prAdapter;
	prHifInfo = &prGlueInfo->rHifInfo;
	prMemOps = &prHifInfo->rMemOps;
	prRxRing = &prHifInfo->RxRing[u2Port];
	u4CpuIdx = prRxRing->RxCpuIdx;
	INC_RING_INDEX(u4CpuIdx, prRxRing->u4RingSize);

	pRxCell = &prRxRing->Cell[u4CpuIdx];
	pRxD = (struct RXD_STRUCT *)pRxCell->AllocVa;

	if (halWpdmaGetRxDmaDoneCnt(prGlueInfo, u2Port) == 0)
		return FALSE;

	if (!kalWaitRxDmaDone(prGlueInfo, prRxRing, pRxD, u2Port)) {
		if (!prRxRing->fgIsDumpLog) {
			prAdapter->u4HifDbgFlag |= DEG_HIF_DEFAULT_DUMP;
			halPrintHifDbgInfo(prAdapter);
		}
		prRxRing->fgIsDumpLog = true;
		return FALSE;
	}

	if (pRxD->LastSec0 == 0 || prRxRing->fgRxSegPkt) {
		/* Rx segmented packet */
		DBGLOG(HAL, WARN,
			"Skip Rx segmented packet, SDL0[%u] LS0[%u]\n",
			pRxD->SDLen0, pRxD->LastSec0);
		if (pRxD->LastSec0 == 1) {
			/* Last segmented packet */
			prRxRing->fgRxSegPkt = FALSE;
		} else {
			/* Segmented packet */
			prRxRing->fgRxSegPkt = TRUE;
		}

		fgRet = FALSE;
		goto skip;
	}

	if (pRxD->SDLen0 > u4Len) {
		DBGLOG(HAL, WARN,
			"Skip Rx packet, SDL0[%u] > SwRfb max len[%u]\n",
			pRxD->SDLen0, u4Len);
		goto skip;
	}

	prDmaBuf = &pRxCell->DmaBuf;
	if (prMemOps->copyEvent &&
	    !prMemOps->copyEvent(prHifInfo, pRxCell, pRxD,
				 prDmaBuf, pucBuf, pRxD->SDLen0)) {
		ASSERT(0);
		return FALSE;
	}

	pRxD->SDPtr0 = (uint64_t)prDmaBuf->AllocPa & DMA_LOWER_32BITS_MASK;
#ifdef CONFIG_PHYS_ADDR_T_64BIT
	pRxD->SDPtr1 = ((uint64_t)prDmaBuf->AllocPa >> DMA_BITS_OFFSET) &
		DMA_HIGHER_4BITS_MASK;
#else
	pRxD->SDPtr1 = 0;
#endif
skip:
	pRxD->SDLen0 = prRxRing->u4BufSize;
	pRxD->DMADONE = 0;

	prRxRing->RxCpuIdx = u4CpuIdx;
	prRxRing->fgSwRead = true;
	prRxRing->fgIsDumpLog = false;

	return fgRet;
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief Write device I/O port
 *
 * \param[in] prGlueInfo         Pointer to the GLUE_INFO_T structure.
 * \param[in] u2Port             I/O port offset
 * \param[in] u2Len              Length to be write
 * \param[in] pucBuf             Pointer to write buffer
 * \param[in] u2ValidInBufSize   Length of the buffer valid to be accessed
 *
 * \retval TRUE          operation success
 * \retval FALSE         operation fail
 */
/*----------------------------------------------------------------------------*/
u_int8_t
kalDevPortWrite(IN struct GLUE_INFO *prGlueInfo,
	IN uint16_t u2Port, IN uint32_t u4Len, IN uint8_t *pucBuf,
	IN uint32_t u4ValidInBufSize)
{
	struct GL_HIF_INFO *prHifInfo = NULL;
	struct HIF_MEM_OPS *prMemOps;
	struct RTMP_TX_RING *prTxRing;
	struct RTMP_DMACB *pTxCell;
	struct TXD_STRUCT *pTxD;
	void *pucDst = NULL;

	ASSERT(prGlueInfo);
	ASSERT(pucBuf);
	ASSERT(u4Len <= u4ValidInBufSize);

	prHifInfo = &prGlueInfo->rHifInfo;
	prMemOps = &prHifInfo->rMemOps;
	prTxRing = &prHifInfo->TxRing[u2Port];

#if (CFG_SUPPORT_CONNAC3X == 1)
	if (prTxRing->u4UsedCnt + 1 >= TX_RING_SIZE) {
		DBGLOG(HAL, INFO, "Force recycle port %d DMA resource.\n",
			u2Port);
		halWpdmaProcessCmdDmaDone(prGlueInfo, u2Port);
	}

	if (prTxRing->u4UsedCnt + 1 >= TX_RING_SIZE) {
		DBGLOG(HAL, ERROR, "Port %d TX resource is NOT enough.\n",
			u2Port);
		return FALSE;
	}
#endif

	if (prMemOps->allocRuntimeMem)
		pucDst = prMemOps->allocRuntimeMem(u4Len);

	kalDevRegRead(prGlueInfo, prTxRing->hw_cidx_addr, &prTxRing->TxCpuIdx);
	if (prTxRing->TxCpuIdx >= TX_RING_SIZE) {
		DBGLOG(HAL, ERROR, "Error TxCpuIdx[%u]\n", prTxRing->TxCpuIdx);
		if (prMemOps->freeBuf)
			prMemOps->freeBuf(pucDst, u4Len);
		return FALSE;
	}

	pTxCell = &prTxRing->Cell[prTxRing->TxCpuIdx];
	pTxD = (struct TXD_STRUCT *)pTxCell->AllocVa;

	pTxCell->pPacket = NULL;
	pTxCell->pBuffer = pucDst;

	if (prMemOps->copyCmd &&
	    !prMemOps->copyCmd(prHifInfo, pTxCell, pucDst,
			       pucBuf, u4Len, NULL, 0)) {
		if (prMemOps->freeBuf)
			prMemOps->freeBuf(pucDst, u4Len);
		ASSERT(0);
		return FALSE;
	}

	pTxD->LastSec0 = 1;
	pTxD->LastSec1 = 0;
	pTxD->SDLen0 = u4Len;
	pTxD->SDLen1 = 0;
	pTxD->SDPtr0 = (uint64_t)pTxCell->PacketPa & DMA_LOWER_32BITS_MASK;
#ifdef CONFIG_PHYS_ADDR_T_64BIT
	pTxD->SDPtr0Ext = ((uint64_t)pTxCell->PacketPa >> DMA_BITS_OFFSET) &
		DMA_HIGHER_4BITS_MASK;
#else
	pTxD->SDPtr0Ext = 0;
#endif
	pTxD->SDPtr1 = 0;
	pTxD->Burst = 0;
	pTxD->DMADONE = 0;

	/* Increase TX_CTX_IDX, but write to register later. */
	INC_RING_INDEX(prTxRing->TxCpuIdx, TX_RING_SIZE);

	prTxRing->u4UsedCnt++;

	kalDevRegWrite(prGlueInfo, prTxRing->hw_cidx_addr, prTxRing->TxCpuIdx);

	return TRUE;
}

void kalDevReadIntStatus(IN struct ADAPTER *prAdapter,
	OUT uint32_t *pu4IntStatus)
{
	uint32_t u4RegValue = 0;
	struct GL_HIF_INFO *prHifInfo = &prAdapter->prGlueInfo->rHifInfo;

	*pu4IntStatus = 0;

	HAL_MCR_RD(prAdapter, WPDMA_INT_STA, &u4RegValue);

	if (HAL_IS_RX_DONE_INTR(u4RegValue) || prAdapter->u4NoMoreRfb != 0)
		*pu4IntStatus |= WHISR_RX0_DONE_INT;

	if (HAL_IS_TX_DONE_INTR(u4RegValue))
		*pu4IntStatus |= WHISR_TX_DONE_INT;

	if (u4RegValue & CONNAC_MCU_SW_INT)
		*pu4IntStatus |= WHISR_D2H_SW_INT;

	prHifInfo->u4IntStatus = u4RegValue;

	/* clear interrupt */
	HAL_MCR_WR(prAdapter, WPDMA_INT_STA, u4RegValue);

}

u_int8_t kalDevWriteCmd(IN struct GLUE_INFO *prGlueInfo,
	IN struct CMD_INFO *prCmdInfo, IN uint8_t ucTC)
{
	struct GL_HIF_INFO *prHifInfo = NULL;

	ASSERT(prGlueInfo);
	prHifInfo = &prGlueInfo->rHifInfo;

	if (nicSerIsTxStop(prGlueInfo->prAdapter))
		return kalDevWriteCmdByQueue(prGlueInfo, prCmdInfo, ucTC);

	return halWpdmaWriteCmd(prGlueInfo, prCmdInfo, ucTC);
}

static bool kalDevWriteCmdByQueue(struct GLUE_INFO *prGlueInfo,
				  struct CMD_INFO *prCmdInfo, uint8_t ucTC)
{
	struct GL_HIF_INFO *prHifInfo = NULL;
	struct TX_CMD_REQ *prTxReq;

	ASSERT(prGlueInfo);
	prHifInfo = &prGlueInfo->rHifInfo;
	prTxReq = kalMemAlloc(sizeof(struct TX_CMD_REQ), PHY_MEM_TYPE);

	if (prTxReq == NULL) {
		DBGLOG(HAL, ERROR, "kmalloc() TX_CMD_REQ error\n");
		halWpdmaWriteCmd(prGlueInfo, prCmdInfo, ucTC);
		goto error;
	}

	prTxReq->prCmdInfo = prCmdInfo;
	prTxReq->ucTC = ucTC;
	list_add_tail(&prTxReq->list, &prHifInfo->rTxCmdQ);

error:

	return true;
}

bool kalDevKickCmd(IN struct GLUE_INFO *prGlueInfo)
{
	struct GL_HIF_INFO *prHifInfo = NULL;
	struct list_head *prCur, *prNext;
	struct TX_CMD_REQ *prTxReq;

	ASSERT(prGlueInfo);
	prHifInfo = &prGlueInfo->rHifInfo;

	list_for_each_safe(prCur, prNext, &prHifInfo->rTxCmdQ) {
		prTxReq = list_entry(prCur, struct TX_CMD_REQ, list);
		if (prTxReq->prCmdInfo)
			halWpdmaWriteCmd(prGlueInfo,
				prTxReq->prCmdInfo, prTxReq->ucTC);
		list_del(prCur);
		kfree(prTxReq);
	}

	return true;
}

static uint8_t kalGetSwAmsduNum(struct GLUE_INFO *prGlueInfo,
				struct MSDU_INFO *prMsduInfo)
{
	struct ADAPTER *prAdapter;
	struct sk_buff *prSkb;
	struct STA_RECORD *prStaRec;
	uint8_t ucTid, ucStaRecIndex;
	struct TX_DESC_OPS_T *prTxDescOps;

	ASSERT(prGlueInfo);
	ASSERT(prMsduInfo);

	prAdapter = prGlueInfo->prAdapter;
	prTxDescOps = prAdapter->chip_info->prTxDescOps;
	prSkb = (struct sk_buff *)prMsduInfo->prPacket;

	ucTid = prTxDescOps->nic_txd_tid_op((void *)prSkb->data, 0, FALSE);
	ucStaRecIndex = prMsduInfo->ucStaRecIndex;
	if (ucStaRecIndex >= CFG_STA_REC_NUM || ucTid >= TX_DESC_TID_NUM)
		return 0;

	prStaRec = cnmGetStaRecByIndex(prAdapter, ucStaRecIndex);
	if (!prStaRec || !(prStaRec->ucAmsduEnBitmap & BIT(ucTid)))
		return 0;

	return prStaRec->ucMaxMpduCount;
}

u_int8_t kalDevWriteData(IN struct GLUE_INFO *prGlueInfo,
	IN struct MSDU_INFO *prMsduInfo)
{
	struct GL_HIF_INFO *prHifInfo = NULL;
	struct mt66xx_chip_info *prChipInfo;

	ASSERT(prGlueInfo);

	prHifInfo = &prGlueInfo->rHifInfo;
	prChipInfo = prGlueInfo->prAdapter->chip_info;

	if (nicSerIsTxStop(prGlueInfo->prAdapter) ||
	    (prChipInfo->ucMaxSwAmsduNum > 1 &&
	     kalGetSwAmsduNum(prGlueInfo, prMsduInfo) > 1))
		return kalDevWriteDataByQueue(prGlueInfo, prMsduInfo);

	return halWpdmaWriteMsdu(prGlueInfo, prMsduInfo, NULL);
}

static bool kalDevWriteDataByQueue(IN struct GLUE_INFO *prGlueInfo,
				   IN struct MSDU_INFO *prMsduInfo)
{
	struct GL_HIF_INFO *prHifInfo = NULL;
	struct TX_DATA_REQ *prTxReq;

	ASSERT(prGlueInfo);
	prHifInfo = &prGlueInfo->rHifInfo;

	prTxReq = &prMsduInfo->rTxReq;
	prTxReq->prMsduInfo = prMsduInfo;
	list_add_tail(&prTxReq->list, &prHifInfo->rTxDataQ);
	prHifInfo->u4TxDataQLen++;

	return true;
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief Kick Tx data to device
 *
 * \param[in] prGlueInfo         Pointer to the GLUE_INFO_T structure.
 *
 * \retval TRUE          operation success
 * \retval FALSE         operation fail
 */
/*----------------------------------------------------------------------------*/
u_int8_t kalDevKickData(IN struct GLUE_INFO *prGlueInfo)
{
	struct mt66xx_chip_info *prChipInfo;

	ASSERT(prGlueInfo);

	prChipInfo = prGlueInfo->prAdapter->chip_info;

	if (prChipInfo->ucMaxSwAmsduNum > 1)
		return kalDevKickAmsduData(prGlueInfo);

	return kalDevKickMsduData(prGlueInfo);
}

static uint16_t kalGetPaddingSize(uint16_t u2TxByteCount)
{
	uint16_t u2Size = 0;

	if (u2TxByteCount & 3)
		u2Size = 4 - (u2TxByteCount & 3);
	return u2Size;
}

static uint16_t kalGetMoreSizeForAmsdu(uint32_t u4TxdDW1)
{
	/*
	 * ETYPE=0/VLAN=0/RMVL=X PLlength = PL length
	 * ETYPE=0/VLAN=1/RMVL=1 PLlength = PL length - 4
	 * ETYPE=0/VLAN=1/RMVL=0 PLlength = PL length + 6
	 * ETYPE=1/VLAN=0/RMVL=X PLlength = PL length + 8
	 * ETYPE=1/VLAN=1/RMVL=1 PLlength = PL length + 4
	 * ETYPE=1/VLAN=1/RMVL=0 PLlength = PL length + 14
	 */
	uint16_t u2Size = 0;

	if (u4TxdDW1 & TXD_DW1_ETYP)
		u2Size += 8;
	if (u4TxdDW1 & TXD_DW1_VLAN) {
		if (u4TxdDW1 & TXD_DW1_RMVL) {
			if (u2Size >= 4)
				u2Size -= 4;
		} else
			u2Size += 6;
	}
	return u2Size;
}

static bool kalDevKickMsduData(struct GLUE_INFO *prGlueInfo)
{
	struct GL_HIF_INFO *prHifInfo = NULL;
	struct BUS_INFO *prBusInfo = NULL;
	struct list_head *prCur, *prNext;
	struct TX_DATA_REQ *prTxReq;
	struct MSDU_INFO *prMsduInfo;
	bool fgRet = true;

	ASSERT(prGlueInfo);

	prHifInfo = &prGlueInfo->rHifInfo;
	prBusInfo = prGlueInfo->prAdapter->chip_info->bus_info;

	list_for_each_safe(prCur, prNext, &prHifInfo->rTxDataQ) {
		prTxReq = list_entry(prCur, struct TX_DATA_REQ, list);
		prMsduInfo = prTxReq->prMsduInfo;
		if (!prMsduInfo ||
		    !halWpdmaWriteMsdu(prGlueInfo, prMsduInfo, prCur)) {
			fgRet = false;
			break;
		}
	}

	return fgRet;
}
#if KERNEL_VERSION(5, 13, 0) <= LINUX_VERSION_CODE
static int kalAmsduTxDCmp(void *prPriv,
		const struct list_head *prList1,
		const struct list_head *prList2)
#else
static int kalAmsduTxDCmp(void *prPriv,
		struct list_head *prList1,
		struct list_head *prList2)
#endif
{
	struct TX_DATA_REQ *prTxReq1, *prTxReq2;
	struct sk_buff *prSkb1, *prSkb2;
	struct AMSDU_MAC_TX_DESC *prTxD1, *prTxD2;

	prTxReq1 = list_entry(prList1, struct TX_DATA_REQ, list);
	prTxReq2 = list_entry(prList2, struct TX_DATA_REQ, list);
	prSkb1 = (struct sk_buff *)prTxReq1->prMsduInfo->prPacket;
	prSkb2 = (struct sk_buff *)prTxReq2->prMsduInfo->prPacket;
	prTxD1 = (struct AMSDU_MAC_TX_DESC *)prSkb1->data;
	prTxD2 = (struct AMSDU_MAC_TX_DESC *)prSkb2->data;

	if (prTxD1->u2DW0 != prTxD2->u2DW0)
		return prTxD2->u2DW0 - prTxD1->u2DW0;

	return prTxD1->u4DW1 - prTxD2->u4DW1;
}

static bool kalIsAggregatedMsdu(struct GLUE_INFO *prGlueInfo,
				struct MSDU_INFO *prMsduInfo)
{
	struct sk_buff *prSkb;
	struct AMSDU_MAC_TX_DESC *prTxD;

	prSkb = (struct sk_buff *)prMsduInfo->prPacket;
	prTxD = (struct AMSDU_MAC_TX_DESC *)prSkb->data;

	if (prTxD->u4FR || prTxD->u4TXS)
		return false;
	return true;
}

static uint32_t kalGetNumOfAmsdu(struct GLUE_INFO *prGlueInfo,
				 struct list_head *prTarget,
				 struct list_head *prHead,
				 uint16_t *pu2Size)
{
	struct TX_DATA_REQ *prTxReq;
	struct MSDU_INFO *prMsduInfo;
	struct sk_buff *prSkb;
	struct AMSDU_MAC_TX_DESC *prTxD;
	struct STA_RECORD *prStaRec;
	struct list_head *prCur;
	uint16_t u2TotalSize, u2Size;
	uint32_t u4Cnt = 1;
	uint8_t ucStaRecIndex;

	prTxReq = list_entry(prTarget, struct TX_DATA_REQ, list);
	prMsduInfo = prTxReq->prMsduInfo;
	prSkb = (struct sk_buff *)prMsduInfo->prPacket;
	prTxD = (struct AMSDU_MAC_TX_DESC *)prSkb->data;

	ucStaRecIndex = prMsduInfo->ucStaRecIndex;
	prStaRec = cnmGetStaRecByIndex(prGlueInfo->prAdapter, ucStaRecIndex);
	if (!prStaRec)
		return 1;

	u2TotalSize = NIC_TX_DESC_LONG_FORMAT_LENGTH;
	u2TotalSize += prMsduInfo->u2FrameLength;
	u2TotalSize += kalGetMoreSizeForAmsdu(prTxD->u4DW1);

	for (prCur = prTarget->next; prCur != prHead; prCur = prCur->next) {
		if (u4Cnt >= kalGetSwAmsduNum(prGlueInfo, prMsduInfo) ||
		    kalAmsduTxDCmp((void *)prGlueInfo, prTarget, prCur))
			break;

		prTxReq = list_entry(prCur, struct TX_DATA_REQ, list);
		prMsduInfo = prTxReq->prMsduInfo;
		prSkb = (struct sk_buff *)prMsduInfo->prPacket;
		prTxD = (struct AMSDU_MAC_TX_DESC *)prSkb->data;

		u2Size = prMsduInfo->u2FrameLength;
		u2Size += kalGetMoreSizeForAmsdu(prTxD->u4DW1);
		u2Size += kalGetPaddingSize(u2TotalSize);
		if ((u2TotalSize + u2Size) > prStaRec->u4MaxMpduLen)
			break;

		u2TotalSize += u2Size;
		u4Cnt++;
	}

	if (u2TotalSize < prStaRec->u4MinMpduLen)
		return 1;

	*pu2Size = u2TotalSize;
	return u4Cnt;
}

static bool kalDevKickAmsduData(struct GLUE_INFO *prGlueInfo)
{
	struct GL_HIF_INFO *prHifInfo = NULL;
	struct BUS_INFO *prBusInfo = NULL;
	struct list_head *prHead, *prCur, *prNext;
	struct TX_DATA_REQ *prTxReq;
	struct MSDU_INFO *prMsduInfo;
	uint32_t u4Num = 0, u4Idx;
	uint16_t u2Size = 0;
	bool fgRet = true;

	ASSERT(prGlueInfo);

	prHifInfo = &prGlueInfo->rHifInfo;
	prBusInfo = prGlueInfo->prAdapter->chip_info->bus_info;

	prHead = &prHifInfo->rTxDataQ;
	list_for_each_safe(prCur, prNext, prHead) {
		prTxReq = list_entry(prCur, struct TX_DATA_REQ, list);
		prMsduInfo = prTxReq->prMsduInfo;
		if (!kalIsAggregatedMsdu(prGlueInfo, prMsduInfo)) {
			if (!halWpdmaWriteMsdu(prGlueInfo, prMsduInfo, prCur))
				return false;
		}
	}

	list_sort((void *)prGlueInfo, prHead, kalAmsduTxDCmp);

	for (prCur = prHead->next; prCur != prHead; prCur = prNext) {
		u4Num = kalGetNumOfAmsdu(prGlueInfo, prCur, prHead, &u2Size);
		prNext = prCur->next;
		if (u4Num > 1) {
			for (u4Idx = 1; u4Idx < u4Num; u4Idx++)
				prNext = prNext->next;
			fgRet = halWpdmaWriteAmsdu(prGlueInfo, prCur,
						   u4Num, u2Size);
		} else {
			prTxReq = list_entry(prCur, struct TX_DATA_REQ, list);
			prMsduInfo = prTxReq->prMsduInfo;
			fgRet = halWpdmaWriteMsdu(prGlueInfo, prMsduInfo,
						  prCur);
		}
		if (!fgRet)
			break;
	}

	return fgRet;
}

bool kalDevReadData(struct GLUE_INFO *prGlueInfo, uint16_t u2Port,
		    struct SW_RFB *prSwRfb)
{
	struct ADAPTER *prAdapter = NULL;
	struct GL_HIF_INFO *prHifInfo = NULL;
	struct HIF_MEM_OPS *prMemOps;
	struct RXD_STRUCT *pRxD;
	struct RTMP_RX_RING *prRxRing;
	struct RTMP_DMACB *pRxCell;
	struct RTMP_DMABUF *prDmaBuf;
	u_int8_t fgRet = TRUE;
	uint32_t u4CpuIdx = 0;
#if (CFG_SUPPORT_PDMA_SCATTER == 1)
	struct RTMP_DMACB *pRxCellScatter;
	struct RXD_STRUCT *pRxDScatter;
	uint32_t u4CpuIdxScatter = 0;
	uint8_t ucScatterCnt = 0;
	uint8_t *pucRecvBuff;
#endif

	ASSERT(prGlueInfo);

	prAdapter = prGlueInfo->prAdapter;
	prHifInfo = &prGlueInfo->rHifInfo;
	prMemOps = &prHifInfo->rMemOps;
	prRxRing = &prHifInfo->RxRing[u2Port];
	u4CpuIdx = prRxRing->RxCpuIdx;
	INC_RING_INDEX(u4CpuIdx, prRxRing->u4RingSize);

	pRxCell = &prRxRing->Cell[u4CpuIdx];
	pRxD = (struct RXD_STRUCT *)pRxCell->AllocVa;

#if 0
	/* Caller should check resource available ready */
	if (halWpdmaGetRxDmaDoneCnt(prGlueInfo, u2Port) == 0)
		return FALSE;
#endif

	if (!kalWaitRxDmaDone(prGlueInfo, prRxRing, pRxD, u2Port)) {
		if (!prRxRing->fgIsDumpLog) {
			prAdapter->u4HifDbgFlag |= DEG_HIF_DEFAULT_DUMP;
			halPrintHifDbgInfo(prAdapter);
		}
		prRxRing->fgIsDumpLog = true;
		return false;
	}

	if (pRxD->LastSec0 == 0 || prRxRing->fgRxSegPkt) {
		/* Rx segmented packet */
		DBGLOG(HAL, WARN,
			"Skip Rx segmented data packet, SDL0[%u] LS0[%u]\n",
			pRxD->SDLen0, pRxD->LastSec0);
#if (CFG_SUPPORT_PDMA_SCATTER == 1)
		if (prRxRing->fgRxSegPkt == FALSE) {
			u4CpuIdxScatter = u4CpuIdx;
			do {
				pRxCellScatter =
					&prRxRing->Cell[u4CpuIdxScatter];
				pRxDScatter = (struct RXD_STRUCT *)
					pRxCellScatter->AllocVa;
				ucScatterCnt++;

				if (pRxDScatter->LastSec0 == 1)
					break;

				INC_RING_INDEX(u4CpuIdxScatter,
					       prRxRing->u4RingSize);
			} while (TRUE);

			prRxRing->pvPacket = kalPacketAlloc(prGlueInfo,
					(ucScatterCnt * CFG_RX_MAX_MPDU_SIZE),
					&pucRecvBuff);
			prRxRing->u4PacketLen = 0;
		}
#endif
		if (pRxD->LastSec0 == 1) {
			/* Last segmented packet */
			prRxRing->fgRxSegPkt = FALSE;
		} else {
			/* Segmented packet */
			prRxRing->fgRxSegPkt = TRUE;
		}

		fgRet = false;
#if (CFG_SUPPORT_PDMA_SCATTER == 1)
		if (prRxRing->pvPacket == NULL)
#endif
		goto skip;
	}

	prDmaBuf = &pRxCell->DmaBuf;

	if (prMemOps->flushCache)
		prMemOps->flushCache(prHifInfo, prDmaBuf->AllocVa,
				     pRxD->SDLen0);

	if (prMemOps->copyRxData &&
	    !prMemOps->copyRxData(prHifInfo, pRxCell, prDmaBuf, prSwRfb))
		return FALSE;

	prSwRfb->pucRecvBuff = ((struct sk_buff *)prSwRfb->pvPacket)->data;
	prSwRfb->prRxStatus = (void *)prSwRfb->pucRecvBuff;

#if CFG_TCP_IP_CHKSUM_OFFLOAD
	prSwRfb->u4TcpUdpIpCksStatus = pRxD->RXINFO;
#endif /* CFG_TCP_IP_CHKSUM_OFFLOAD */

	pRxD->SDPtr0 = (uint64_t)prDmaBuf->AllocPa & DMA_LOWER_32BITS_MASK;
#ifdef CONFIG_PHYS_ADDR_T_64BIT
	pRxD->SDPtr1 = ((uint64_t)prDmaBuf->AllocPa >>
		DMA_BITS_OFFSET) & DMA_HIGHER_4BITS_MASK;
#else
	pRxD->SDPtr1 = 0;
#endif

#if (CFG_SUPPORT_PDMA_SCATTER == 1)
	if (fgRet == FALSE) {
		pucRecvBuff = ((struct sk_buff *)prRxRing->pvPacket)->data;
		pucRecvBuff += prRxRing->u4PacketLen;
		kalMemCopy(pucRecvBuff, prSwRfb->pucRecvBuff, pRxD->SDLen0);
		prRxRing->u4PacketLen += pRxD->SDLen0;

		if (prRxRing->fgRxSegPkt == FALSE) {
			kalPacketFree(prGlueInfo, prSwRfb->pvPacket);
			prSwRfb->pvPacket = prRxRing->pvPacket;
			prSwRfb->pucRecvBuff =
				((struct sk_buff *)prSwRfb->pvPacket)->data;
			prSwRfb->prRxStatus = (void *)prSwRfb->pucRecvBuff;
			prRxRing->pvPacket = NULL;
			fgRet = TRUE;
		}
	}
#endif
skip:
	pRxD->SDLen0 = prRxRing->u4BufSize;
	pRxD->DMADONE = 0;

	prRxRing->RxCpuIdx = u4CpuIdx;
	prRxRing->fgSwRead = true;
	prRxRing->fgIsDumpLog = false;

#if CFG_TCP_IP_CHKSUM_OFFLOAD
	if (fgRet)
		DBGLOG(RX, LOUD, "u4TcpUdpIpCksStatus[0x%02x]\n",
		       prSwRfb->u4TcpUdpIpCksStatus);
#endif /* CFG_TCP_IP_CHKSUM_OFFLOAD */

	return fgRet;
}


#if (CFG_SUPPORT_CONNAC3X == 1)
int wf_ioremap_read(phys_addr_t addr, unsigned int *val)
{
	void *vir_addr = NULL;

	vir_addr = ioremap(addr, 0x10);
	if (!vir_addr) {
		DBGLOG(INIT, ERROR, "%s: Cannot remap address[%pa].\n",
		       addr, __func__);
		return -1;
	}
	*val = readl(vir_addr);
	iounmap(vir_addr);
	DBGLOG(INIT, TRACE, "Read CONSYS 0x%08x=0x%08x.\n", addr, *val);
	return 0;
}
int wf_ioremap_write(phys_addr_t addr, unsigned int val)
{
	void *vir_addr = NULL;

	vir_addr = ioremap(addr, 0x10);
	if (!vir_addr) {
		DBGLOG(INIT, ERROR, "%s: Cannot remap address[%pa].\n",
		       addr, __func__);
		return -1;
	}
	writel(val, vir_addr);
	iounmap(vir_addr);
	DBGLOG(INIT, TRACE, "Write CONSYS 0x%08x=0x%08x.\n", addr, val);
	return 0;
}
#endif /* if (CFG_SUPPORT_CONNAC3X == 1) */
