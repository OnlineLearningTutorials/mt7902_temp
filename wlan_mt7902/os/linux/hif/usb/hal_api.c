/* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause */
/*
 * Copyright (c) 2016 MediaTek Inc.
 */
/******************************************************************************
*[File]             hif_api.c
*[Version]          v1.0
*[Revision Date]    2015-09-08
*[Author]
*[Description]
*    The program provides USB HIF APIs
*[Copyright]
*    Copyright (C) 2015 MediaTek Incorporation. All Rights Reserved.
******************************************************************************/

/*******************************************************************************
*                         C O M P I L E R   F L A G S
********************************************************************************
*/

/*******************************************************************************
*                    E X T E R N A L   R E F E R E N C E S
********************************************************************************
*/
#include "precomp.h"

#include <linux/usb.h>
#include <linux/mutex.h>

#include <linux/mm.h>
#ifndef CONFIG_X86
#include <asm/memory.h>
#endif

#include "mt66xx_reg.h"
#include "hal_wfsys_reset_mt7961.h"

/*******************************************************************************
*                              C O N S T A N T S
********************************************************************************
*/

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
static const uint16_t arTcToUSBEP[USB_TC_NUM] = {
	USB_DATA_BULK_OUT_EP4,
	USB_DATA_BULK_OUT_EP5,
	USB_DATA_BULK_OUT_EP6,
	USB_DATA_BULK_OUT_EP7,
	USB_DATA_BULK_OUT_EP8,
	USB_DATA_BULK_OUT_EP9,

	/* Second HW queue */
#if NIC_TX_ENABLE_SECOND_HW_QUEUE
	USB_DATA_BULK_OUT_EP9,
	USB_DATA_BULK_OUT_EP9,
	USB_DATA_BULK_OUT_EP9,
	USB_DATA_BULK_OUT_EP9,
#endif
};

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

/*----------------------------------------------------------------------------*/
/*!
* @brief Verify the CHIP ID
*
* @param prAdapter      a pointer to adapter private data structure.
*
*
* @retval TRUE          CHIP ID is the same as the setting compiled
* @retval FALSE         CHIP ID is different from the setting compiled
*/
/*----------------------------------------------------------------------------*/
u_int8_t halVerifyChipID(IN struct ADAPTER *prAdapter)
{
	uint32_t u4CIR = 0;
	struct mt66xx_chip_info *prChipInfo;

	if (prAdapter == NULL) {
		DBGLOG(HAL, ERROR, "prAdapter is NULL error\n");
		return FALSE;
	}

	if (prAdapter->fgIsReadRevID)
		return TRUE;

	prChipInfo = prAdapter->chip_info;

	HAL_MCR_RD(prAdapter, prChipInfo->top_hcr, &u4CIR);
	DBGLOG(INIT, TRACE, "Chip ID: 0x%4x\n", u4CIR);

	if (u4CIR != prChipInfo->chip_id)
		return FALSE;

	HAL_MCR_RD(prAdapter, prChipInfo->top_hvr, &u4CIR);
	DBGLOG(INIT, TRACE, "Revision ID: 0x%4x\n", u4CIR);

	prAdapter->ucRevID = (uint8_t) (u4CIR & 0xF);
	prAdapter->fgIsReadRevID = TRUE;
	return TRUE;
}

uint32_t
halRxWaitResponse(IN struct ADAPTER *prAdapter, IN uint8_t ucPortIdx, OUT uint8_t *pucRspBuffer,
		  IN uint32_t u4MaxRespBufferLen, OUT uint32_t *pu4Length)
{
	struct GL_HIF_INFO *prHifInfo;
	uint32_t u4Status = WLAN_STATUS_SUCCESS;
	struct RX_CTRL *prRxCtrl;
	u_int8_t ret = FALSE;
	struct BUS_INFO *prBusInfo;

	DEBUGFUNC("halRxWaitResponse");

	if (prAdapter == NULL) {
		DBGLOG(HAL, ERROR, "prAdapter is NULL error\n");
		return WLAN_STATUS_FAILURE;
	}
	if (pucRspBuffer == NULL) {
		DBGLOG(HAL, ERROR, "pucRspBuffer is NULL error\n");
		return WLAN_STATUS_FAILURE;
	}

	prHifInfo = &prAdapter->prGlueInfo->rHifInfo;
	prRxCtrl = &prAdapter->rRxCtrl;
	prBusInfo = prAdapter->chip_info->bus_info;

	if (prBusInfo->asicUsbEventEpDetected)
		ucPortIdx = prBusInfo->asicUsbEventEpDetected(prAdapter);
	else {
		if (prHifInfo->fgEventEpDetected == FALSE) {
			/* NOTE: This is temporary compatiable code with old/new CR4 FW to detect
			 *       which EVENT endpoint that.
			 *       CR4 FW is using. If the new EP4IN-using CR4 FW works without
			 *       any issue for a while,
			 *       this code block will be removed.
			 */
			if (prAdapter->fgIsCr4FwDownloaded) {
				ucPortIdx = USB_DATA_EP_IN;
				ret = kalDevPortRead(prAdapter->prGlueInfo, ucPortIdx,
					ALIGN_4(u4MaxRespBufferLen) + LEN_USB_RX_PADDING_CSO,
					prRxCtrl->pucRxCoalescingBufPtr, HIF_RX_COALESCING_BUFFER_SIZE);

				if (ret == TRUE) {
					prHifInfo->eEventEpType = EVENT_EP_TYPE_DATA_EP;
				} else {
					ucPortIdx = USB_EVENT_EP_IN;
					ret = kalDevPortRead(prAdapter->prGlueInfo, ucPortIdx,
						ALIGN_4(u4MaxRespBufferLen) + LEN_USB_RX_PADDING_CSO,
						prRxCtrl->pucRxCoalescingBufPtr, HIF_RX_COALESCING_BUFFER_SIZE);
				}
				prHifInfo->fgEventEpDetected = TRUE;

				kalMemCopy(pucRspBuffer, prRxCtrl->pucRxCoalescingBufPtr, u4MaxRespBufferLen);
				*pu4Length = u4MaxRespBufferLen;

				if (ret == FALSE)
					u4Status = WLAN_STATUS_FAILURE;
				return u4Status;
			}

			ucPortIdx = USB_EVENT_EP_IN;
		} else {
			if (prHifInfo->eEventEpType == EVENT_EP_TYPE_DATA_EP)
				if (prAdapter->fgIsCr4FwDownloaded)
					ucPortIdx = USB_DATA_EP_IN;
				else
					ucPortIdx = USB_EVENT_EP_IN;
			else
				ucPortIdx = USB_EVENT_EP_IN;
		}
	}
	ret = kalDevPortRead(prAdapter->prGlueInfo, ucPortIdx,
		ALIGN_4(u4MaxRespBufferLen) + LEN_USB_RX_PADDING_CSO,
		prRxCtrl->pucRxCoalescingBufPtr, HIF_RX_COALESCING_BUFFER_SIZE);

	kalMemCopy(pucRspBuffer, prRxCtrl->pucRxCoalescingBufPtr, u4MaxRespBufferLen);
	*pu4Length = u4MaxRespBufferLen;

	if (ret == FALSE)
		u4Status = WLAN_STATUS_FAILURE;

	return u4Status;
}

uint32_t halTxUSBSendCmd(IN struct GLUE_INFO *prGlueInfo, IN uint8_t ucTc, IN struct CMD_INFO *prCmdInfo)
{
	struct GL_HIF_INFO *prHifInfo = &prGlueInfo->rHifInfo;
	uint32_t u4Status = WLAN_STATUS_SUCCESS;
	struct USB_REQ *prUsbReq;
	struct BUF_CTRL *prBufCtrl;
	uint16_t u2OverallBufferLength = 0;
	unsigned long flags;
	uint8_t ucQueIdx;
	struct mt66xx_chip_info *prChipInfo;
	int ret;
	struct TX_DESC_OPS_T *prTxDescOps;

	if (!(prHifInfo->state == USB_STATE_LINK_UP ||
		prHifInfo->state == USB_STATE_PRE_RESUME ||
		prHifInfo->state == USB_STATE_PRE_SUSPEND)) {
		DBGLOG(HAL, ERROR, "TX CMD CID[0x%X] invalid state %d!\n",
		       prCmdInfo->ucCID, prHifInfo->state);
		return WLAN_STATUS_FAILURE;
	}

	prUsbReq = glUsbDequeueReq(prHifInfo, &prHifInfo->rTxCmdFreeQ, &prHifInfo->rTxCmdQLock);
	if (prUsbReq == NULL) {
		DBGLOG(HAL, ERROR, "TX CMD CID[0x%X] SEQ[%d] no URB!\n",
		       prCmdInfo->ucCID, prCmdInfo->ucCmdSeqNum);
		return WLAN_STATUS_RESOURCES;
	}

	prBufCtrl = prUsbReq->prBufCtrl;

	if ((TFCB_FRAME_PAD_TO_DW(prCmdInfo->u4TxdLen + prCmdInfo->u4TxpLen) + LEN_USB_UDMA_TX_TERMINATOR) >
	    prBufCtrl->u4BufSize) {
		DBGLOG(HAL, ERROR, "TX CMD CID[0x%X] buffer underflow!\n",
		       prCmdInfo->ucCID);
		glUsbEnqueueReq(prHifInfo, &prHifInfo->rTxCmdFreeQ, prUsbReq,
				&prHifInfo->rTxCmdQLock, FALSE);
		return WLAN_STATUS_RESOURCES;
	}

	DBGLOG(HAL, INFO, "TX CMD CID[0x%X] URB[0x%p] SEQ[%d]\n", prCmdInfo->ucCID,
	       prUsbReq->prUrb, prCmdInfo->ucCmdSeqNum);

	prChipInfo = prGlueInfo->prAdapter->chip_info;
	prTxDescOps = prChipInfo->prTxDescOps;
	HAL_WRITE_HIF_TXD(prChipInfo, prBufCtrl->pucBuf,
			(prCmdInfo->u4TxdLen + prCmdInfo->u4TxpLen),
				TXD_PKT_FORMAT_COMMAND);
	u2OverallBufferLength += prChipInfo->u2HifTxdSize;

	if (prCmdInfo->u4TxdLen) {
		memcpy((prBufCtrl->pucBuf + u2OverallBufferLength), prCmdInfo->pucTxd, prCmdInfo->u4TxdLen);
		u2OverallBufferLength += prCmdInfo->u4TxdLen;
	}

	if (prCmdInfo->u4TxpLen) {
		memcpy((prBufCtrl->pucBuf + u2OverallBufferLength), prCmdInfo->pucTxp, prCmdInfo->u4TxpLen);
		u2OverallBufferLength += prCmdInfo->u4TxpLen;
	}

	if (prChipInfo->is_support_cr4 && prTxDescOps->nic_txd_queue_idx_op) {
		void *prTxDesc = (void *)(prBufCtrl->pucBuf
					+ prChipInfo->u2HifTxdSize);
		ucQueIdx = prTxDescOps->nic_txd_queue_idx_op(
			prTxDesc, 0, FALSE);
		/* For H2CDMA Tx CMD mapping
		 * Mapping port1 queue0~3 to queue28~31,
		 * and CR4 will unmask this.
		 */
		prTxDescOps->nic_txd_queue_idx_op(
			prTxDesc,
			(ucQueIdx | USB_TX_CMD_QUEUE_MASK),
			TRUE);
	}

	/* DBGLOG_MEM32(SW4, INFO, prBufCtrl->pucBuf, 32); */
	memset(prBufCtrl->pucBuf + u2OverallBufferLength, 0,
	       ((TFCB_FRAME_PAD_TO_DW(u2OverallBufferLength) - u2OverallBufferLength) + LEN_USB_UDMA_TX_TERMINATOR));
	prBufCtrl->u4WrIdx = TFCB_FRAME_PAD_TO_DW(u2OverallBufferLength) + LEN_USB_UDMA_TX_TERMINATOR;

	prUsbReq->prPriv = (void *) prCmdInfo;
	usb_fill_bulk_urb(prUsbReq->prUrb,
			  prHifInfo->udev,
			  usb_sndbulkpipe(prHifInfo->udev, arTcToUSBEP[ucTc]),
			  (void *)prUsbReq->prBufCtrl->pucBuf,
			  prBufCtrl->u4WrIdx, halTxUSBSendCmdComplete, (void *)prUsbReq);

#if CFG_USB_CONSISTENT_DMA
	prUsbReq->prUrb->transfer_flags |= URB_NO_TRANSFER_DMA_MAP;
#endif
	spin_lock_irqsave(&prHifInfo->rTxCmdQLock, flags);
	ret = glUsbSubmitUrb(prHifInfo, prUsbReq->prUrb, SUBMIT_TYPE_TX_CMD);
	if (ret) {
		DBGLOG(HAL, ERROR,
			"glUsbSubmitUrb() reports error (%d) [%s] (EP%d OUT)\n",
			ret, __func__, arTcToUSBEP[ucTc]);
		list_add_tail(&prUsbReq->list, &prHifInfo->rTxCmdFreeQ);
		spin_unlock_irqrestore(&prHifInfo->rTxCmdQLock, flags);
		return WLAN_STATUS_FAILURE;
	}
	list_add_tail(&prUsbReq->list, &prHifInfo->rTxCmdSendingQ);
	spin_unlock_irqrestore(&prHifInfo->rTxCmdQLock, flags);

	if (wlanIsChipRstRecEnabled(prGlueInfo->prAdapter)
			&& wlanIsChipNoAck(prGlueInfo->prAdapter)) {
		wlanChipRstPreAct(prGlueInfo->prAdapter);
		DBGLOG(HAL, ERROR, "usb trigger whole reset\n");
		HAL_WIFI_FUNC_CHIP_RESET(prGlueInfo->prAdapter);
	}
	return u4Status;
}

void halTxUSBSendCmdComplete(struct urb *urb)
{
	struct USB_REQ *prUsbReq = urb->context;
	struct GL_HIF_INFO *prHifInfo = prUsbReq->prHifInfo;
	struct GLUE_INFO *prGlueInfo = prHifInfo->prGlueInfo;
	unsigned long flags;

#if CFG_USB_TX_HANDLE_IN_HIF_THREAD
	spin_lock_irqsave(&prHifInfo->rTxCmdQLock, flags);
	list_del_init(&prUsbReq->list);
	list_add_tail(&prUsbReq->list, &prHifInfo->rTxCmdCompleteQ);
	spin_unlock_irqrestore(&prHifInfo->rTxCmdQLock, flags);

	kalSetIntEvent(prGlueInfo);
#else
	spin_lock_irqsave(&prHifInfo->rTxCmdQLock, flags);
	list_del_init(&prUsbReq->list);
	spin_unlock_irqrestore(&prHifInfo->rTxCmdQLock, flags);

	halTxUSBProcessCmdComplete(prGlueInfo->prAdapter, prUsbReq);
#endif
}

void halTxUSBProcessCmdComplete(IN struct ADAPTER *prAdapter, struct USB_REQ *prUsbReq)
{
	struct urb *urb = prUsbReq->prUrb;
	uint32_t u4SentDataSize;
	struct GL_HIF_INFO *prHifInfo = prUsbReq->prHifInfo;

	if (urb->status != 0) {
		DBGLOG(TX, ERROR, "[%s] send CMD fail (status = %d)\n", __func__, urb->status);
		/* TODO: handle error */
	}

	DBGLOG(HAL, INFO, "TX CMD DONE: URB[0x%p]\n", urb);

	glUsbEnqueueReq(prHifInfo, &prHifInfo->rTxCmdFreeQ, prUsbReq, &prHifInfo->rTxCmdQLock, FALSE);

	u4SentDataSize = urb->actual_length - LEN_USB_UDMA_TX_TERMINATOR;
	nicTxReleaseResource_PSE(prAdapter, TC4_INDEX,
		halTxGetCmdPageCount(prAdapter, u4SentDataSize, TRUE), TRUE);

#if CFG_SUPPORT_MULTITHREAD
	/* TX Commands */
	if (kalGetTxPendingCmdCount(prAdapter->prGlueInfo))
		kalSetTxCmdEvent2Hif(prAdapter->prGlueInfo);
#endif
}

void halTxCancelSendingCmd(IN struct ADAPTER *prAdapter, IN struct CMD_INFO *prCmdInfo)
{
	struct USB_REQ *prUsbReq, *prNext;
	unsigned long flags;
	struct urb *urb = NULL;
	struct GL_HIF_INFO *prHifInfo = &prAdapter->prGlueInfo->rHifInfo;

	spin_lock_irqsave(&prHifInfo->rTxCmdQLock, flags);
	list_for_each_entry_safe(prUsbReq, prNext, &prHifInfo->rTxCmdSendingQ, list) {
		if (prUsbReq->prPriv == (void *) prCmdInfo) {
			list_del_init(&prUsbReq->list);
			urb = prUsbReq->prUrb;
			break;
		}
	}
	spin_unlock_irqrestore(&prHifInfo->rTxCmdQLock, flags);

	if (urb) {
		prCmdInfo->pfHifTxCmdDoneCb = NULL;
		usb_kill_urb(urb);
	}
}

void halTxCancelAllSending(IN struct ADAPTER *prAdapter)
{
	struct GLUE_INFO *prGlueInfo;
	struct USB_REQ *prUsbReq, *prUsbReqNext;
	struct GL_HIF_INFO *prHifInfo;
#if CFG_USB_TX_AGG
	uint8_t ucTc;
#endif

	if (prAdapter == NULL) {
		DBGLOG(HAL, ERROR, "prAdapter is NULL error\n");
		return;
	}

	prGlueInfo = prAdapter->prGlueInfo;
	prHifInfo = &prGlueInfo->rHifInfo;

	list_for_each_entry_safe(prUsbReq, prUsbReqNext, &prHifInfo->rTxCmdSendingQ, list) {
		usb_kill_urb(prUsbReq->prUrb);
	}

#if CFG_USB_TX_AGG
	for (ucTc = 0; ucTc < USB_TC_NUM; ++ucTc)
		usb_kill_anchored_urbs(&prHifInfo->rTxDataAnchor[ucTc]);
#else
	usb_kill_anchored_urbs(&prHifInfo->rTxDataAnchor);
#endif
}

void halCancelTxRx(IN struct ADAPTER *prAdapter)
{
	/* stop TX BULK OUT URB */
	halTxCancelAllSending(prAdapter);

	/* stop RX BULK IN URB */
	halDisableInterrupt(prAdapter);
}

#if CFG_CHIP_RESET_SUPPORT
uint32_t halToggleWfsysRst(IN struct ADAPTER *prAdapter)
{
	struct mt66xx_chip_info *prChipInfo;
	struct BUS_INFO *prBusInfo;
	struct CHIP_DBG_OPS *prChipDbg;

	prChipInfo = prAdapter->chip_info;
	prBusInfo = prChipInfo->bus_info;
	prChipDbg = prChipInfo->prDebugOps;

	/* Although we've already executed show_mcu_debug_info() in
	 * glResetTriggerCommon(), it may not be a successful operation at that
	 * time in USB mode due to the context is soft_irq which forbids the
	 * execution of usb_control_msg(). Since we don't prefer to refactor it
	 * at this stage, this is a workaround by performing it again here
	 * where the context is kernel thread from workqueue.
	 */
	if (prChipDbg->show_mcu_debug_info)
		prChipDbg->show_mcu_debug_info(prAdapter, NULL, 0,
					       DBG_MCU_DBG_ALL, NULL);

	/* set USB EP_RST_OPT as reset scope excludes toggle bit,
	 * sequence number, etc.
	 */
	if (prBusInfo->asicUsbEpctlRstOpt)
		prBusInfo->asicUsbEpctlRstOpt(prAdapter, FALSE);

	/* assert WF L0.5 reset */
	if (prChipInfo->asicWfsysRst)
		prChipInfo->asicWfsysRst(prAdapter, TRUE);

	/* wait 2 ticks of 32K */
	kalUdelay(64);

	/* de-assert WF L0.5 reset */
	if (prChipInfo->asicWfsysRst)
		prChipInfo->asicWfsysRst(prAdapter, FALSE);

	if (prChipInfo->asicPollWfsysSwInitDone)
		if (!prChipInfo->asicPollWfsysSwInitDone(prAdapter)) {
			DBGLOG(HAL, ERROR, "L0.5 reset polling sw init done fail\n");
			return WLAN_STATUS_FAILURE;
		}

	return WLAN_STATUS_SUCCESS;
}
#endif /* CFG_CHIP_RESET_SUPPORT */

#if CFG_USB_TX_AGG
uint32_t halTxUSBSendAggData(IN struct GL_HIF_INFO *prHifInfo, IN uint8_t ucTc, IN struct USB_REQ *prUsbReq)
{
	struct GLUE_INFO *prGlueInfo = prHifInfo->prGlueInfo;
	struct BUF_CTRL *prBufCtrl = prUsbReq->prBufCtrl;
	uint32_t u4Status = WLAN_STATUS_SUCCESS;
	int ret;

	memset(prBufCtrl->pucBuf + prBufCtrl->u4WrIdx, 0, LEN_USB_UDMA_TX_TERMINATOR);
	prBufCtrl->u4WrIdx += LEN_USB_UDMA_TX_TERMINATOR;

	if (prHifInfo->state != USB_STATE_LINK_UP) {
		/* No need to dequeue prUsbReq because LINK is not up */
		prBufCtrl->u4WrIdx = 0;
		return WLAN_STATUS_FAILURE;
	}

	list_del_init(&prUsbReq->list);

	usb_fill_bulk_urb(prUsbReq->prUrb,
			  prHifInfo->udev,
			  usb_sndbulkpipe(prHifInfo->udev, arTcToUSBEP[ucTc]),
			  (void *)prBufCtrl->pucBuf, prBufCtrl->u4WrIdx, halTxUSBSendDataComplete, (void *)prUsbReq);
#if CFG_USB_CONSISTENT_DMA
	prUsbReq->prUrb->transfer_flags |= URB_NO_TRANSFER_DMA_MAP;
#endif

	usb_anchor_urb(prUsbReq->prUrb, &prHifInfo->rTxDataAnchor[ucTc]);
	ret = glUsbSubmitUrb(prHifInfo, prUsbReq->prUrb, SUBMIT_TYPE_TX_DATA);
	if (ret) {
		DBGLOG(HAL, ERROR,
			"glUsbSubmitUrb() reports error (%d) [%s] (EP%d OUT)\n",
			ret, __func__, arTcToUSBEP[ucTc]);
		halTxUSBProcessMsduDone(prGlueInfo, prUsbReq);
		prBufCtrl->u4WrIdx = 0;
		usb_unanchor_urb(prUsbReq->prUrb);
		list_add_tail(&prUsbReq->list, &prHifInfo->rTxDataCompleteQ);
#if CFG_USB_TX_HANDLE_IN_HIF_THREAD
		kalSetIntEvent(prGlueInfo);
#else
		/*tasklet_hi_schedule(&prGlueInfo->rTxCompleteTask);*/
		tasklet_schedule(&prGlueInfo->rTxCompleteTask);
#endif
		return WLAN_STATUS_FAILURE;
	}

	return u4Status;
}
#endif

uint32_t halTxUSBSendData(IN struct GLUE_INFO *prGlueInfo, IN struct MSDU_INFO *prMsduInfo)
{
	struct GL_HIF_INFO *prHifInfo = &prGlueInfo->rHifInfo;
	struct mt66xx_chip_info *prChipInfo;
	uint32_t u4Status = WLAN_STATUS_SUCCESS;
	struct USB_REQ *prUsbReq;
	struct BUF_CTRL *prBufCtrl;
	uint32_t u4PaddingLength;
	struct sk_buff *skb;
	uint8_t ucTc;
	uint8_t *pucBuf;
	uint32_t u4Length;
	uint32_t u4TotalLen;
#if CFG_USB_TX_AGG
	unsigned long flags;
#else
	int ret;
#endif
#if (CFG_SUPPORT_TX_SG == 1)
	int i;
#endif /* CFG_SUPPORT_TX_SG */

	prChipInfo = prGlueInfo->prAdapter->chip_info;
	skb = (struct sk_buff *)prMsduInfo->prPacket;
	pucBuf = skb->data;
	u4Length = skb->len;
	u4TotalLen = u4Length + prChipInfo->u2HifTxdSize;
	ucTc = USB_TRANS_MSDU_TC(prMsduInfo);
#if (CFG_SUPPORT_DMASHDL_SYSDVT)
	if (prMsduInfo->ucPktType == ENUM_PKT_ICMP) {
		if (DMASHDL_DVT_QUEUE_MAPPING_TYPE1(prGlueInfo->prAdapter)
		|| DMASHDL_DVT_QUEUE_MAPPING_TYPE2(prGlueInfo->prAdapter)) {
			/* send ping packets to each EP for DMASHDL DVT */
			ucTc = prMsduInfo->ucTarQueue % TC_NUM;
			/* skip TC4, TC4=>EP8 is reserved for CMD */
			if (ucTc == TC4_INDEX)
				ucTc = TC_NUM;
			DMASHDL_DVT_INC_PING_PKT_CNT(prGlueInfo->prAdapter,
				prMsduInfo->ucTarQueue);
		}
	}
#endif /* CFG_SUPPORT_DMASHDL_SYSDVT */

#if CFG_USB_TX_AGG
	spin_lock_irqsave(&prHifInfo->rTxDataQLock, flags);

	if (list_empty(&prHifInfo->rTxDataFreeQ[ucTc])) {
		if (glUsbBorrowFfaReq(prHifInfo, ucTc) == FALSE) {
			spin_unlock_irqrestore(&prHifInfo->rTxDataQLock, flags);
			DBGLOG(HAL, ERROR, "run out of rTxDataFreeQ #1!!\n");
			wlanProcessQueuedMsduInfo(prGlueInfo->prAdapter, prMsduInfo);
			return WLAN_STATUS_RESOURCES;
		}
	}
	prUsbReq = list_entry(prHifInfo->rTxDataFreeQ[ucTc].next, struct USB_REQ, list);
	prBufCtrl = prUsbReq->prBufCtrl;

	if (prHifInfo->u4AggRsvSize[ucTc] < ALIGN_4(u4TotalLen))
		DBGLOG(HAL, ERROR, "u4AggRsvSize[%hhu] count FAIL (%u, %u)\n",
		       ucTc, prHifInfo->u4AggRsvSize[ucTc], u4TotalLen);
	prHifInfo->u4AggRsvSize[ucTc] -= ALIGN_4(u4TotalLen);

	if (prBufCtrl->u4WrIdx + ALIGN_4(u4TotalLen) + LEN_USB_UDMA_TX_TERMINATOR > prBufCtrl->u4BufSize) {
		halTxUSBSendAggData(prHifInfo, ucTc, prUsbReq);

		if (list_empty(&prHifInfo->rTxDataFreeQ[ucTc])) {
			if (glUsbBorrowFfaReq(prHifInfo, ucTc) == FALSE) {
				spin_unlock_irqrestore(&prHifInfo->rTxDataQLock,
							flags);
				DBGLOG(HAL, ERROR, "run out of rTxDataFreeQ #2!!\n");
				wlanProcessQueuedMsduInfo(prGlueInfo->prAdapter, prMsduInfo);
				return WLAN_STATUS_FAILURE;
			}
		}

		prUsbReq = list_entry(prHifInfo->rTxDataFreeQ[ucTc].next, struct USB_REQ, list);
		prBufCtrl = prUsbReq->prBufCtrl;
	}

	HAL_WRITE_HIF_TXD(prChipInfo, prBufCtrl->pucBuf + prBufCtrl->u4WrIdx,
				u4Length, TXD_PKT_FORMAT_TXD_PAYLOAD);
	prBufCtrl->u4WrIdx += prChipInfo->u2HifTxdSize;
	memcpy(prBufCtrl->pucBuf + prBufCtrl->u4WrIdx, pucBuf,
		u4Length - skb->data_len);
	prBufCtrl->u4WrIdx += (u4Length - skb->data_len);
#if (CFG_SUPPORT_TX_SG == 1)
	for (i = 0; i < skb_shinfo(skb)->nr_frags; i++) {
		const skb_frag_t *frag = &skb_shinfo(skb)->frags[i];

		memcpy(prBufCtrl->pucBuf + prBufCtrl->u4WrIdx,
			skb_frag_address(frag),
			skb_frag_size(frag));
		prBufCtrl->u4WrIdx += skb_frag_size(frag);
	}
#endif /* CFG_SUPPORT_TX_SG */

	u4PaddingLength = (ALIGN_4(u4TotalLen) - u4TotalLen);
	if (u4PaddingLength) {
		memset(prBufCtrl->pucBuf + prBufCtrl->u4WrIdx, 0, u4PaddingLength);
		prBufCtrl->u4WrIdx += u4PaddingLength;
	}

	if (!prMsduInfo->pfTxDoneHandler)
		QUEUE_INSERT_TAIL(&prUsbReq->rSendingDataMsduInfoList, (struct QUE_ENTRY *) prMsduInfo);

	if (usb_anchor_empty(&prHifInfo->rTxDataAnchor[ucTc]))
		halTxUSBSendAggData(prHifInfo, ucTc, prUsbReq);

	spin_unlock_irqrestore(&prHifInfo->rTxDataQLock, flags);
#else
	prUsbReq = glUsbDequeueReq(prHifInfo, &prHifInfo->rTxDataFreeQ,
					&prHifInfo->rTxDataQLock);
	if (prUsbReq == NULL) {
		DBGLOG(HAL, ERROR, "run out of rTxDataFreeQ!!\n");
		wlanProcessQueuedMsduInfo(prGlueInfo->prAdapter, prMsduInfo);
		return WLAN_STATUS_RESOURCES;
	}

	prBufCtrl = prUsbReq->prBufCtrl;
	prBufCtrl->u4WrIdx = 0;

	HAL_WRITE_HIF_TXD(prChipInfo, prBufCtrl->pucBuf, u4Length,
			TXD_PKT_FORMAT_TXD_PAYLOAD);
	prBufCtrl->u4WrIdx += prChipInfo->u2HifTxdSize;

	memcpy(prBufCtrl->pucBuf + prChipInfo->u2HifTxdSize, pucBuf, u4Length);
	prBufCtrl->u4WrIdx += u4Length;

	u4PaddingLength = (ALIGN_4(u4TotalLen) - u4TotalLen);
	if (u4PaddingLength) {
		memset(prBufCtrl->pucBuf + prBufCtrl->u4WrIdx, 0, u4PaddingLength);
		prBufCtrl->u4WrIdx += u4PaddingLength;
	}

	memset(prBufCtrl->pucBuf + prBufCtrl->u4WrIdx, 0, LEN_USB_UDMA_TX_TERMINATOR);
	prBufCtrl->u4WrIdx += LEN_USB_UDMA_TX_TERMINATOR;

	if (!prMsduInfo->pfTxDoneHandler)
		QUEUE_INSERT_TAIL(&prUsbReq->rSendingDataMsduInfoList, (struct QUE_ENTRY *) prMsduInfo);

	*((uint8_t *)&prUsbReq->prPriv) = ucTc;
	usb_fill_bulk_urb(prUsbReq->prUrb,
			  prHifInfo->udev,
			  usb_sndbulkpipe(prHifInfo->udev, arTcToUSBEP[ucTc]),
			  (void *)prUsbReq->prBufCtrl->pucBuf,
			  prBufCtrl->u4WrIdx, halTxUSBSendDataComplete, (void *)prUsbReq);
#if CFG_USB_CONSISTENT_DMA
	prUsbReq->prUrb->transfer_flags |= URB_NO_TRANSFER_DMA_MAP;
#endif

	usb_anchor_urb(prUsbReq->prUrb, &prHifInfo->rTxDataAnchor);
	ret = glUsbSubmitUrb(prHifInfo, prUsbReq->prUrb, SUBMIT_TYPE_TX_DATA);
	if (ret) {
		DBGLOG(HAL, ERROR,
			"glUsbSubmitUrb() reports error (%d) [%s] (EP%d OUT)\n",
			ret, __func__, arTcToUSBEP[ucTc]);
		halTxUSBProcessMsduDone(prHifInfo->prGlueInfo, prUsbReq);
		prBufCtrl->u4WrIdx = 0;
		usb_unanchor_urb(prUsbReq->prUrb);
		glUsbEnqueueReq(prHifInfo, &prHifInfo->rTxDataFreeQ, prUsbReq,
					&prHifInfo->rTxDataQLock, FALSE);
		return WLAN_STATUS_FAILURE;
	}
#endif

	if (wlanIsChipRstRecEnabled(prGlueInfo->prAdapter)
			&& wlanIsChipNoAck(prGlueInfo->prAdapter)) {
		wlanChipRstPreAct(prGlueInfo->prAdapter);
		DBGLOG(HAL, ERROR, "usb trigger whole reset\n");
		HAL_WIFI_FUNC_CHIP_RESET(prGlueInfo->prAdapter);
	}
	return u4Status;
}

uint32_t halTxUSBKickData(IN struct GLUE_INFO *prGlueInfo)
{
#if CFG_USB_TX_AGG
	struct GL_HIF_INFO *prHifInfo = &prGlueInfo->rHifInfo;
	struct USB_REQ *prUsbReq;
	struct BUF_CTRL *prBufCtrl;
	uint8_t ucTc;
	unsigned long flags;

	spin_lock_irqsave(&prHifInfo->rTxDataQLock, flags);

	for (ucTc = TC0_INDEX; ucTc < USB_TC_NUM; ucTc++) {
		if (list_empty(&prHifInfo->rTxDataFreeQ[ucTc]))
			continue;

		prUsbReq = list_entry(prHifInfo->rTxDataFreeQ[ucTc].next, struct USB_REQ, list);
		prBufCtrl = prUsbReq->prBufCtrl;

		if (prBufCtrl->u4WrIdx)
			halTxUSBSendAggData(prHifInfo, ucTc, prUsbReq);
	}

	spin_unlock_irqrestore(&prHifInfo->rTxDataQLock, flags);
#endif

	return WLAN_STATUS_SUCCESS;
}

void halTxUSBSendDataComplete(struct urb *urb)
{
	struct USB_REQ *prUsbReq = urb->context;
	struct GL_HIF_INFO *prHifInfo = prUsbReq->prHifInfo;
	struct GLUE_INFO *prGlueInfo = prHifInfo->prGlueInfo;

	glUsbEnqueueReq(prHifInfo, &prHifInfo->rTxDataCompleteQ, prUsbReq, &prHifInfo->rTxDataQLock, FALSE);

#if CFG_USB_TX_HANDLE_IN_HIF_THREAD
	kalSetIntEvent(prGlueInfo);
#else
	/*tasklet_hi_schedule(&prGlueInfo->rTxCompleteTask);*/
	tasklet_schedule(&prGlueInfo->rTxCompleteTask);
#endif
}

void halTxUSBProcessMsduDone(IN struct GLUE_INFO *prGlueInfo, struct USB_REQ *prUsbReq)
{
	uint8_t ucTc;
	struct QUE rFreeQueue;
	struct QUE *prFreeQueue;
	struct urb *urb = prUsbReq->prUrb;
	uint32_t u4SentDataSize;

	ucTc = *((uint8_t *)&prUsbReq->prPriv) & TC_MASK;

	prFreeQueue = &rFreeQueue;
	QUEUE_INITIALIZE(prFreeQueue);
	QUEUE_MOVE_ALL((prFreeQueue), (&(prUsbReq->rSendingDataMsduInfoList)));
	if (g_pfTxDataDoneCb)
		g_pfTxDataDoneCb(prGlueInfo, prFreeQueue);

	u4SentDataSize = urb->actual_length - LEN_USB_UDMA_TX_TERMINATOR;
	nicTxReleaseResource_PSE(prGlueInfo->prAdapter, ucTc,
		halTxGetDataPageCount(prGlueInfo->prAdapter, u4SentDataSize,
			TRUE), TRUE);
}

void halTxUSBProcessDataComplete(IN struct ADAPTER *prAdapter, struct USB_REQ *prUsbReq)
{
	uint8_t ucTc;
	u_int8_t fgFfa;
	struct urb *urb = prUsbReq->prUrb;
	struct GL_HIF_INFO *prHifInfo = prUsbReq->prHifInfo;
#if CFG_USB_TX_AGG
	struct BUF_CTRL *prBufCtrl = prUsbReq->prBufCtrl;
#endif
	unsigned long flags;

	ucTc = *((uint8_t *)&prUsbReq->prPriv) & TC_MASK;
	fgFfa =  *((uint8_t *)&prUsbReq->prPriv) & FFA_MASK;

	if (urb->status != 0) {
		DBGLOG(TX, ERROR, "[%s] send DATA fail (status = %d)\n", __func__, urb->status);
		/* TODO: handle error */
	}

	halTxUSBProcessMsduDone(prAdapter->prGlueInfo, prUsbReq);

	spin_lock_irqsave(&prHifInfo->rTxDataQLock, flags);
#if CFG_USB_TX_AGG
	prBufCtrl->u4WrIdx = 0;

	if ((fgFfa == FALSE) || list_empty(&prHifInfo->rTxDataFreeQ[ucTc]))
		list_add_tail(&prUsbReq->list, &prHifInfo->rTxDataFreeQ[ucTc]);
	else
		list_add_tail(&prUsbReq->list, &prHifInfo->rTxDataFfaQ);

	if (usb_anchor_empty(&prHifInfo->rTxDataAnchor[ucTc])) {
		prUsbReq = list_entry(prHifInfo->rTxDataFreeQ[ucTc].next, struct USB_REQ, list);
		prBufCtrl = prUsbReq->prBufCtrl;

		if (prBufCtrl->u4WrIdx != 0)
			halTxUSBSendAggData(prHifInfo, ucTc, prUsbReq);	/* TODO */
	}
#else
	list_add_tail(&prUsbReq->list, &prHifInfo->rTxDataFreeQ);
#endif
	spin_unlock_irqrestore(&prHifInfo->rTxDataQLock, flags);

	if (!HAL_IS_TX_DIRECT(prAdapter)) {
		if (kalGetTxPendingCmdCount(prAdapter->prGlueInfo) > 0 || wlanGetTxPendingFrameCount(prAdapter) > 0)
			kalSetEvent(prAdapter->prGlueInfo);
#if CFG_SUPPORT_MULTITHREAD
		kalSetTxEvent2Hif(prAdapter->prGlueInfo);
#endif
	}
}

uint32_t halRxUSBEnqueueRFB(
	IN struct ADAPTER *prAdapter,
	IN uint8_t *pucBuf,
	IN uint32_t u4Length,
	IN uint32_t u4MinRfbCnt,
	IN struct list_head *prCompleteQ)
{
	struct GLUE_INFO *prGlueInfo;
	struct mt66xx_chip_info *prChipInfo;
	struct RX_CTRL *prRxCtrl;
	struct SW_RFB *prSwRfb = (struct SW_RFB *) NULL;
	void *prRxStatus;
	uint32_t u4RemainCount;
	uint16_t u2RxByteCount;
	uint8_t *pucRxFrame;
	uint32_t u4EnqCnt = 0;
	struct BUS_INFO *prBusInfo;
#if CFG_TCP_IP_CHKSUM_OFFLOAD
	uint32_t *pu4HwAppendDW;
#endif /* CFG_TCP_IP_CHKSUM_OFFLOAD */
	struct RX_DESC_OPS_T *prRxDescOps;

	KAL_SPIN_LOCK_DECLARATION();

	if (prAdapter == NULL) {
		DBGLOG(HAL, ERROR, "prAdapter is NULL error\n");
		return 0;
	}
	prGlueInfo = prAdapter->prGlueInfo;
	prRxCtrl = &prAdapter->rRxCtrl;
	prChipInfo = prAdapter->chip_info;
	prBusInfo = prChipInfo->bus_info;
	prRxDescOps = prChipInfo->prRxDescOps;
	if (prRxDescOps->nic_rxd_get_rx_byte_count == NULL) {
		DBGLOG(HAL, ERROR, "nic_rxd_get_rx_byte_count is NULL error\n");
		return 0;
	}
	if (prRxDescOps->nic_rxd_get_pkt_type == NULL) {
		DBGLOG(HAL, ERROR, "nic_rxd_get_pkt_type is NULL error\n");
		return 0;
	}

	pucRxFrame = pucBuf;
	u4RemainCount = u4Length;
	while (u4RemainCount > 4) {
		/*
		 * For different align support.
		 * Ex. We need to do 8byte align for 7915u.
		 */
		if (prBusInfo->asicUsbRxByteCount)
			u2RxByteCount = prBusInfo->asicUsbRxByteCount(prAdapter,
				prBusInfo, pucRxFrame);
		else {
			u2RxByteCount =
				prRxDescOps->nic_rxd_get_rx_byte_count(
								pucRxFrame);
			u2RxByteCount = ALIGN_4(u2RxByteCount)
				+ LEN_USB_RX_PADDING_CSO;
		}

		if (u2RxByteCount <= CFG_RX_MAX_PKT_SIZE) {
			prSwRfb = NULL;
			KAL_ACQUIRE_SPIN_LOCK(prAdapter, SPIN_LOCK_RX_FREE_QUE);
			if (prRxCtrl->rFreeSwRfbList.u4NumElem > u4MinRfbCnt)
				QUEUE_REMOVE_HEAD(&prRxCtrl->rFreeSwRfbList, prSwRfb, struct SW_RFB *);
			KAL_RELEASE_SPIN_LOCK(prAdapter, SPIN_LOCK_RX_FREE_QUE);

			if (!prSwRfb)
				return u4Length - u4RemainCount;

			kalMemCopy(prSwRfb->pucRecvBuff, pucRxFrame, u2RxByteCount);

			prRxStatus = prSwRfb->prRxStatus;
			ASSERT(prRxStatus);

			prSwRfb->ucPacketType =
				prRxDescOps->nic_rxd_get_pkt_type(prRxStatus);
			/* DBGLOG(RX, TRACE, ("ucPacketType = %d\n", prSwRfb->ucPacketType)); */
#if CFG_TCP_IP_CHKSUM_OFFLOAD
			pu4HwAppendDW = (uint32_t *) prRxStatus;
			pu4HwAppendDW +=
				(ALIGN_4(u2RxByteCount - LEN_USB_RX_PADDING_CSO)
									>> 2);
			prSwRfb->u4TcpUdpIpCksStatus = *pu4HwAppendDW;
#endif /* CFG_TCP_IP_CHKSUM_OFFLOAD */

			if (HAL_IS_RX_DIRECT(prAdapter)) {
				switch (prSwRfb->ucPacketType) {
				case RX_PKT_TYPE_RX_DATA:
					spin_lock_bh(&prGlueInfo->rSpinLock[SPIN_LOCK_RX_DIRECT]);
					nicRxProcessDataPacket(
						prAdapter, prSwRfb);
					spin_unlock_bh(&prGlueInfo->rSpinLock[SPIN_LOCK_RX_DIRECT]);
					break;
				default:
					KAL_ACQUIRE_SPIN_LOCK(prAdapter, SPIN_LOCK_RX_QUE);
					QUEUE_INSERT_TAIL(&prRxCtrl->rReceivedRfbList, &prSwRfb->rQueEntry);
					KAL_RELEASE_SPIN_LOCK(prAdapter, SPIN_LOCK_RX_QUE);
					u4EnqCnt++;
					break;
				}
			} else {
				KAL_ACQUIRE_SPIN_LOCK(prAdapter, SPIN_LOCK_RX_QUE);
				QUEUE_INSERT_TAIL(&prRxCtrl->rReceivedRfbList, &prSwRfb->rQueEntry);
				KAL_RELEASE_SPIN_LOCK(prAdapter, SPIN_LOCK_RX_QUE);
				u4EnqCnt++;
			}
			RX_INC_CNT(prRxCtrl, RX_MPDU_TOTAL_COUNT);
		} else {
			DBGLOG(RX, WARN, "Rx byte count:%u exceeds SW_RFB max length:%u\n!",
				u2RxByteCount, CFG_RX_MAX_PKT_SIZE);
			DBGLOG_MEM32(RX, WARN, pucRxFrame,
				     prChipInfo->rxd_size);
			break;
		}

		u4RemainCount -= u2RxByteCount;
		pucRxFrame += u2RxByteCount;
	}

	if (u4EnqCnt) {
		set_bit(GLUE_FLAG_RX_BIT, &(prGlueInfo->ulFlag));
		wake_up_interruptible(&(prGlueInfo->waitq));
	}
	return u4Length;
}

uint32_t halRxUSBReceiveEvent(IN struct ADAPTER *prAdapter, IN u_int8_t fgFillUrb)
{
	struct GLUE_INFO *prGlueInfo = prAdapter->prGlueInfo;
	struct GL_HIF_INFO *prHifInfo = &prGlueInfo->rHifInfo;
	struct USB_REQ *prUsbReq;
	uint32_t u4Status = WLAN_STATUS_SUCCESS;
	int ret;

	while (1) {
		prUsbReq = glUsbDequeueReq(prHifInfo, &prHifInfo->rRxEventFreeQ, &prHifInfo->rRxEventQLock);
		if (prUsbReq == NULL)
			return WLAN_STATUS_RESOURCES;

		usb_anchor_urb(prUsbReq->prUrb, &prHifInfo->rRxEventAnchor);

		prUsbReq->prBufCtrl->u4ReadSize = 0;
		if (prHifInfo->eEventEpType == EVENT_EP_TYPE_INTR && fgFillUrb) {
			usb_fill_int_urb(prUsbReq->prUrb,
				prHifInfo->udev,
				usb_rcvintpipe(prHifInfo->udev,
						USB_EVENT_EP_IN),
				(void *)prUsbReq->prBufCtrl->pucBuf,
				prUsbReq->prBufCtrl->u4BufSize,
				halRxUSBReceiveEventComplete,
				(void *)prUsbReq,
				1);
		} else if (prHifInfo->eEventEpType == EVENT_EP_TYPE_BULK) {
			usb_fill_bulk_urb(prUsbReq->prUrb,
				prHifInfo->udev,
				usb_rcvbulkpipe(prHifInfo->udev,
							USB_EVENT_EP_IN),
				(void *)prUsbReq->prBufCtrl->pucBuf,
				prUsbReq->prBufCtrl->u4BufSize,
				halRxUSBReceiveEventComplete,
				(void *)prUsbReq);
		}
		ret = glUsbSubmitUrb(prHifInfo, prUsbReq->prUrb,
					SUBMIT_TYPE_RX_EVENT);
		if (ret) {
			DBGLOG(HAL, ERROR,
				"glUsbSubmitUrb() reports error (%d) [%s] (EP%d IN)\n",
				ret, __func__, (USB_EVENT_EP_IN & 0x0F));
			usb_unanchor_urb(prUsbReq->prUrb);
			glUsbEnqueueReq(prHifInfo, &prHifInfo->rRxEventFreeQ, prUsbReq,
					&prHifInfo->rRxEventQLock, FALSE);
			break;
		}
	}

	return u4Status;
}

void halRxUSBReceiveEventComplete(struct urb *urb)
{
	struct USB_REQ *prUsbReq = urb->context;
	struct GL_HIF_INFO *prHifInfo = prUsbReq->prHifInfo;
	struct GLUE_INFO *prGlueInfo = prHifInfo->prGlueInfo;

	if (!(prHifInfo->state == USB_STATE_LINK_UP ||
			prHifInfo->state == USB_STATE_PRE_RESUME ||
			prHifInfo->state == USB_STATE_PRE_SUSPEND)) {
		glUsbEnqueueReq(prHifInfo, &prHifInfo->rRxEventFreeQ, prUsbReq, &prHifInfo->rRxEventQLock, FALSE);
		return;
	}

	/* Hif power off wifi, drop rx packets and continue polling RX packets until RX path empty */
	if (prGlueInfo->ulFlag & GLUE_FLAG_HALT) {
		glUsbEnqueueReq(prHifInfo, &prHifInfo->rRxEventFreeQ, prUsbReq, &prHifInfo->rRxEventQLock, FALSE);
		halRxUSBReceiveEvent(prGlueInfo->prAdapter, FALSE);
		return;
	}

	if (urb->status == -ESHUTDOWN || urb->status == -ENOENT) {
		glUsbEnqueueReq(prHifInfo, &prHifInfo->rRxEventFreeQ, prUsbReq, &prHifInfo->rRxEventQLock, FALSE);
		DBGLOG(RX, ERROR, "USB device shutdown skip Rx [%s]\n", __func__);
		return;
	}

#if CFG_USB_RX_HANDLE_IN_HIF_THREAD
	DBGLOG(RX, TRACE, "[%s] Rx URB[0x%p] Len[%u] Sts[%u]\n", __func__, urb, urb->actual_length, urb->status);

	glUsbEnqueueReq(prHifInfo, &prHifInfo->rRxEventCompleteQ, prUsbReq, &prHifInfo->rRxEventQLock, FALSE);

	kalSetIntEvent(prGlueInfo);
#else
	if (urb->status == 0) {
		glUsbEnqueueReq(prHifInfo, &prHifInfo->rRxEventCompleteQ, prUsbReq, &prHifInfo->rRxEventQLock, FALSE);

		/*tasklet_hi_schedule(&prGlueInfo->rRxTask);*/
		tasklet_schedule(&prGlueInfo->rRxTask);
	} else {
		DBGLOG(RX, ERROR, "[%s] receive EVENT fail (status = %d)\n", __func__, urb->status);
		glUsbEnqueueReq(prHifInfo, &prHifInfo->rRxEventFreeQ, prUsbReq, &prHifInfo->rRxEventQLock, FALSE);

		halRxUSBReceiveEvent(prGlueInfo->prAdapter, FALSE);
	}
#endif
}

#if CFG_CHIP_RESET_SUPPORT
uint32_t halRxUSBReceiveWdt(IN struct ADAPTER *prAdapter)
{
	struct GLUE_INFO *prGlueInfo = prAdapter->prGlueInfo;
	struct GL_HIF_INFO *prHifInfo = &prGlueInfo->rHifInfo;
	struct USB_REQ *prUsbReq;
	uint32_t u4Status = WLAN_STATUS_SUCCESS;
	int ret;

	while (1) {
		prUsbReq = glUsbDequeueReq(prHifInfo, &prHifInfo->rRxWdtFreeQ,
					   &prHifInfo->rRxWdtQLock);
		if (prUsbReq == NULL)
			return WLAN_STATUS_RESOURCES;

		usb_anchor_urb(prUsbReq->prUrb, &prHifInfo->rRxWdtAnchor);

		prUsbReq->prBufCtrl->u4ReadSize = 0;

		usb_fill_int_urb(prUsbReq->prUrb,
				 prHifInfo->udev,
				 usb_rcvintpipe(prHifInfo->udev,
						USB_WDT_EP_IN),
				 (void *)prUsbReq->prBufCtrl->pucBuf,
				 prUsbReq->prBufCtrl->u4BufSize,
				 halRxUSBReceiveWdtComplete,
				 (void *)prUsbReq,
				 1);

		ret = glUsbSubmitUrb(prHifInfo, prUsbReq->prUrb,
				     SUBMIT_TYPE_RX_WDT);
		if (ret) {
			DBGLOG(HAL, ERROR,
			       "glUsbSubmitUrb() reports error (%d)\n", ret);

			usb_unanchor_urb(prUsbReq->prUrb);
			glUsbEnqueueReq(prHifInfo, &prHifInfo->rRxWdtFreeQ,
					prUsbReq, &prHifInfo->rRxWdtQLock,
					FALSE);
			break;
		}
	}

	return u4Status;
}

void halRxUSBReceiveWdtComplete(struct urb *urb)
{
	struct USB_REQ *prUsbReq = urb->context;
	struct GL_HIF_INFO *prHifInfo = prUsbReq->prHifInfo;
	struct GLUE_INFO *prGlueInfo = prHifInfo->prGlueInfo;
	struct mt66xx_chip_info *prChipInfo;
	struct BUS_INFO *prBusInfo;

	prChipInfo = prGlueInfo->prAdapter->chip_info;
	prBusInfo = prChipInfo->bus_info;

	if (urb->status == -ESHUTDOWN || urb->status == -ENOENT) {
		glUsbEnqueueReq(prHifInfo, &prHifInfo->rRxWdtFreeQ, prUsbReq,
				&prHifInfo->rRxWdtQLock, FALSE);
		DBGLOG(RX, ERROR, "USB device shutdown skip Rx\n");
		return;
	}

	if (urb->status == 0) {
		glUsbEnqueueReq(prHifInfo, &prHifInfo->rRxWdtCompleteQ,
				prUsbReq, &prHifInfo->rRxWdtQLock, FALSE);

		/*tasklet_hi_schedule(&prGlueInfo->rRxTask);*/
		tasklet_schedule(&prGlueInfo->rRxTask);
	} else {
		DBGLOG(RX, ERROR, "receive WDT fail (status = %d)\n",
		       urb->status);
		glUsbEnqueueReq(prHifInfo, &prHifInfo->rRxWdtFreeQ, prUsbReq,
				&prHifInfo->rRxWdtQLock, FALSE);

		halRxUSBReceiveWdt(prGlueInfo->prAdapter);
	}
}
#endif /* CFG_CHIP_RESET_SUPPORT */

uint32_t halRxUSBReceiveData(IN struct ADAPTER *prAdapter)
{
	struct GLUE_INFO *prGlueInfo;
	struct GL_HIF_INFO *prHifInfo;
	struct USB_REQ *prUsbReq;
	uint32_t u4Status = WLAN_STATUS_SUCCESS;
	int ret;

	if (!prAdapter) {
		DBGLOG(INIT, WARN, "Adapter NULL\n");
		return WLAN_STATUS_FAILURE;
	}
	prGlueInfo = prAdapter->prGlueInfo;

	if (!prGlueInfo) {
		DBGLOG(INIT, WARN, "GlueInfo NULL\n");
		return WLAN_STATUS_FAILURE;
	}
	prHifInfo = &prGlueInfo->rHifInfo;

	while (1) {
		prUsbReq = glUsbDequeueReq(prHifInfo, &prHifInfo->rRxDataFreeQ, &prHifInfo->rRxDataQLock);
		if (prUsbReq == NULL)
			return WLAN_STATUS_RESOURCES;

		usb_anchor_urb(prUsbReq->prUrb, &prHifInfo->rRxDataAnchor);

		prUsbReq->prBufCtrl->u4ReadSize = 0;
		usb_fill_bulk_urb(prUsbReq->prUrb,
				  prHifInfo->udev,
				  usb_rcvbulkpipe(prHifInfo->udev, USB_DATA_EP_IN),
				  (void *)prUsbReq->prBufCtrl->pucBuf,
				  prUsbReq->prBufCtrl->u4BufSize, halRxUSBReceiveDataComplete, (void *)prUsbReq);
		ret = glUsbSubmitUrb(prHifInfo, prUsbReq->prUrb,
					SUBMIT_TYPE_RX_DATA);
		if (ret) {
			DBGLOG(HAL, ERROR,
				"glUsbSubmitUrb() reports error (%d) [%s] (EP%d IN)\n",
				ret, __func__, (USB_EVENT_EP_IN & 0x0F));
			usb_unanchor_urb(prUsbReq->prUrb);
			glUsbEnqueueReq(prHifInfo, &prHifInfo->rRxDataFreeQ, prUsbReq, &prHifInfo->rRxDataQLock, FALSE);
			break;
		}
	}

	return u4Status;
}

void halRxUSBReceiveDataComplete(struct urb *urb)
{
	struct USB_REQ *prUsbReq = urb->context;
	struct GL_HIF_INFO *prHifInfo = prUsbReq->prHifInfo;
	struct GLUE_INFO *prGlueInfo = prHifInfo->prGlueInfo;

	if (!(prHifInfo->state == USB_STATE_LINK_UP ||
			prHifInfo->state == USB_STATE_PRE_RESUME ||
			prHifInfo->state == USB_STATE_PRE_SUSPEND)) {
		glUsbEnqueueReq(prHifInfo, &prHifInfo->rRxDataFreeQ, prUsbReq, &prHifInfo->rRxDataQLock, FALSE);
		return;
	}

	/* Hif power off wifi, drop rx packets and continue polling RX packets until RX path empty */
	if (prGlueInfo->ulFlag & GLUE_FLAG_HALT) {
		glUsbEnqueueReq(prHifInfo, &prHifInfo->rRxDataFreeQ, prUsbReq, &prHifInfo->rRxDataQLock, FALSE);
		halRxUSBReceiveData(prGlueInfo->prAdapter);
		return;
	}

	if (urb->status == -ESHUTDOWN || urb->status == -ENOENT) {
		glUsbEnqueueReq(prHifInfo, &prHifInfo->rRxDataFreeQ, prUsbReq, &prHifInfo->rRxDataQLock, FALSE);
		DBGLOG(RX, ERROR, "USB device shutdown skip Rx [%s]\n", __func__);
		return;
	}

#if CFG_USB_RX_HANDLE_IN_HIF_THREAD
	glUsbEnqueueReq(prHifInfo, &prHifInfo->rRxDataCompleteQ, prUsbReq, &prHifInfo->rRxDataQLock, FALSE);

	kalSetIntEvent(prGlueInfo);
#else
	if (urb->status == 0) {
		glUsbEnqueueReq(prHifInfo, &prHifInfo->rRxDataCompleteQ, prUsbReq, &prHifInfo->rRxDataQLock, FALSE);

		/*tasklet_hi_schedule(&prGlueInfo->rRxTask);*/
		tasklet_schedule(&prGlueInfo->rRxTask);
	} else {
		DBGLOG(RX, ERROR, "[%s] receive DATA fail (status = %d)\n", __func__, urb->status);
		glUsbEnqueueReq(prHifInfo, &prHifInfo->rRxDataFreeQ, prUsbReq, &prHifInfo->rRxDataQLock, FALSE);

		halRxUSBReceiveData(prGlueInfo->prAdapter);
	}
#endif
}

void halRxUSBProcessEventDataComplete(IN struct ADAPTER *prAdapter,
	struct list_head *prCompleteQ, struct list_head *prFreeQ, uint32_t u4MinRfbCnt)
{
	struct USB_REQ *prUsbReq;
	struct urb *prUrb;
	struct BUF_CTRL *prBufCtrl;
	struct GLUE_INFO *prGlueInfo = prAdapter->prGlueInfo;
	struct GL_HIF_INFO *prHifInfo = &prGlueInfo->rHifInfo;
	uint8_t *pucBufAddr;
	uint32_t u4BufLen;
	static u_int8_t s_fgOutOfSwRfb = FALSE;
	static uint32_t s_u4OutOfSwRfbPrintLimit;

	/* lock with rRxDataQLock if processing queue is data queue */
	/* and vice versa                                           */
	spinlock_t *prLock =
		(prCompleteQ == &prHifInfo->rRxDataCompleteQ) ?
		(&prHifInfo->rRxDataQLock) :
		(&prHifInfo->rRxEventQLock);

	/* Process complete event/data */
	prUsbReq = glUsbDequeueReq(prHifInfo, prCompleteQ, prLock);
	while (prUsbReq) {
		prUrb = prUsbReq->prUrb;
		prBufCtrl = prUsbReq->prBufCtrl;

		DBGLOG(RX, LOUD, "[%s] Rx URB[0x%p] Len[%u] Sts[%u]\n", __func__,
			prUrb, prUrb->actual_length, prUrb->status);

		if (prUrb->status != 0) {
			DBGLOG(RX, ERROR, "[%s] receive EVENT/DATA fail (status = %d)\n", __func__, prUrb->status);

			glUsbEnqueueReq(prHifInfo, prFreeQ, prUsbReq, prLock,
					FALSE);
			prUsbReq = glUsbDequeueReq(prHifInfo, prCompleteQ,
						prLock);
			continue;
		}

#if CFG_CHIP_RESET_SUPPORT
		if (prAdapter->chip_info->fgIsSupportL0p5Reset) {
			spin_lock_bh(&prAdapter->rWfsysResetLock);
			/* During WFSYS_RESET_STATE_REINIT,
			* driver might need to receive
			* events like response
			* of EXT_CMD_ID_EFUSE_BUFFER_MODE.
			*/
			if ((prAdapter->eWfsysResetState
				!= WFSYS_RESET_STATE_IDLE) &&
			    (prAdapter->eWfsysResetState
				!= WFSYS_RESET_STATE_REINIT)) {
				spin_unlock_bh(&prAdapter->rWfsysResetLock);

				DBGLOG(RX, ERROR,
					"skip rx urb process due to L0.5 reset\n");

				glUsbEnqueueReq(prHifInfo, prFreeQ,
					prUsbReq, prLock, FALSE);
				prUsbReq = glUsbDequeueReq(prHifInfo,
					prCompleteQ, prLock);
				continue;
			}
			spin_unlock_bh(&prAdapter->rWfsysResetLock);
		}
#endif /* CFG_CHIP_RESET_SUPPORT */

		pucBufAddr = prBufCtrl->pucBuf + prBufCtrl->u4ReadSize;
		u4BufLen = prUrb->actual_length - prBufCtrl->u4ReadSize;

		prBufCtrl->u4ReadSize += halRxUSBEnqueueRFB(
				prAdapter,
				pucBufAddr,
				u4BufLen,
				u4MinRfbCnt,
				prCompleteQ);

		if (unlikely(prUrb->actual_length - prBufCtrl->u4ReadSize > 4)) {
			if (s_fgOutOfSwRfb == FALSE) {
				if ((long)jiffies - (long)s_u4OutOfSwRfbPrintLimit > 0) {
					DBGLOG(RX, WARN, "Out of SwRfb!\n");
					s_u4OutOfSwRfbPrintLimit = jiffies + MSEC_TO_JIFFIES(SW_RFB_LOG_LIMIT_MS);
				}
				s_fgOutOfSwRfb = TRUE;
			}
			glUsbEnqueueReq(prHifInfo, prCompleteQ, prUsbReq,
					prLock, TRUE);

			set_bit(GLUE_FLAG_RX_BIT, &prGlueInfo->ulFlag);
			wake_up_interruptible(&prGlueInfo->waitq);

			schedule_delayed_work(&prGlueInfo->rRxPktDeAggWork, MSEC_TO_JIFFIES(SW_RFB_RECHECK_MS));
			break;
		}

		if (unlikely(s_fgOutOfSwRfb == TRUE))
			s_fgOutOfSwRfb = FALSE;

		glUsbEnqueueReq(prHifInfo, prFreeQ, prUsbReq, prLock, FALSE);
		prUsbReq = glUsbDequeueReq(prHifInfo, prCompleteQ, prLock);
	}
}

#if CFG_CHIP_RESET_SUPPORT
void halRxUSBProcessWdtComplete(IN struct ADAPTER *prAdapter,
				struct list_head *prCompleteQ,
				struct list_head *prFreeQ, uint32_t u4MinRfbCnt)
{
	struct USB_REQ *prUsbReq;
	struct urb *prUrb;
	struct BUF_CTRL *prBufCtrl;
	struct GLUE_INFO *prGlueInfo = prAdapter->prGlueInfo;
	struct GL_HIF_INFO *prHifInfo = &prGlueInfo->rHifInfo;
	spinlock_t *prLock = &prHifInfo->rRxWdtQLock;
	uint8_t *pucBufAddr;
	uint32_t u4BufLen;

#define USB_WATCHDOG_TIMEOUT_EVENT        0xFF

	/* Process complete WDT interrupt */
	prUsbReq = glUsbDequeueReq(prHifInfo, prCompleteQ, prLock);
	while (prUsbReq) {
		prUrb = prUsbReq->prUrb;
		prBufCtrl = prUsbReq->prBufCtrl;

		DBGLOG(RX, LOUD, "Rx URB[0x%p] Len[%u] Sts[%u]\n",
			prUrb, prUrb->actual_length, prUrb->status);

		if (prUrb->status != 0) {
			DBGLOG(RX, ERROR, "receive WDT fail (status = %d)\n",
			       prUrb->status);

			glUsbEnqueueReq(prHifInfo, prFreeQ, prUsbReq, prLock,
					FALSE);
			prUsbReq = glUsbDequeueReq(prHifInfo, prCompleteQ,
						prLock);
			continue;
		}

#if CFG_CHIP_RESET_SUPPORT
		spin_lock_bh(&prAdapter->rWfsysResetLock);
		if (prAdapter->eWfsysResetState != WFSYS_RESET_STATE_IDLE) {
			spin_unlock_bh(&prAdapter->rWfsysResetLock);

			DBGLOG(RX, ERROR,
			       "skip rx urb process due to L0.5 reset\n");

			glUsbEnqueueReq(prHifInfo, prFreeQ, prUsbReq, prLock,
					FALSE);
			prUsbReq = glUsbDequeueReq(prHifInfo, prCompleteQ,
						prLock);
			continue;
		}
		spin_unlock_bh(&prAdapter->rWfsysResetLock);
#endif /* CFG_CHIP_RESET_SUPPORT */

		pucBufAddr = prBufCtrl->pucBuf + prBufCtrl->u4ReadSize;
		u4BufLen = prUrb->actual_length - prBufCtrl->u4ReadSize;

		if (u4BufLen == 1 &&
		    pucBufAddr[0] == USB_WATCHDOG_TIMEOUT_EVENT) {
			GL_DEFAULT_RESET_TRIGGER(prAdapter, RST_WDT);
		} else {
			DBGLOG(RX, ERROR, "receive unexpected WDT packet\n");
			dumpMemory8(pucBufAddr, u4BufLen);
			WARN_ON(1);
		}

		glUsbEnqueueReq(prHifInfo, prFreeQ, prUsbReq, prLock, FALSE);
		prUsbReq = glUsbDequeueReq(prHifInfo, prCompleteQ, prLock);
	}
}
#endif /* CFG_CHIP_RESET_SUPPORT */

/*----------------------------------------------------------------------------*/
/*!
* @brief enable global interrupt
*
* @param prAdapter pointer to the Adapter handler
*
* @return (none)
*/
/*----------------------------------------------------------------------------*/
void halEnableInterrupt(IN struct ADAPTER *prAdapter)
{
	struct GLUE_INFO *prGlueInfo;
	struct GL_HIF_INFO *prHifInfo;

	if (prAdapter == NULL) {
		DBGLOG(HAL, ERROR, "prAdapter is NULL error\n");
		return;
	}

	prGlueInfo = prAdapter->prGlueInfo;
	prHifInfo = &prGlueInfo->rHifInfo;

	halRxUSBReceiveData(prAdapter);
	if (prHifInfo->eEventEpType != EVENT_EP_TYPE_DATA_EP)
		halRxUSBReceiveEvent(prAdapter, TRUE);

#if CFG_CHIP_RESET_SUPPORT
	if (prAdapter->chip_info->fgIsSupportL0p5Reset &&
	    prAdapter->chip_info->bus_info->fgIsSupportWdtEp)
		halRxUSBReceiveWdt(prAdapter);
#endif

	glUdmaRxAggEnable(prGlueInfo, TRUE);
} /* end of halEnableInterrupt() */

/*----------------------------------------------------------------------------*/
/*!
* @brief disable global interrupt
*
* @param prAdapter pointer to the Adapter handler
*
* @return (none)
*/
/*----------------------------------------------------------------------------*/
void halDisableInterrupt(IN struct ADAPTER *prAdapter)
{
	struct GLUE_INFO *prGlueInfo;
	struct GL_HIF_INFO *prHifInfo;

	if (prAdapter == NULL) {
		DBGLOG(HAL, ERROR, "prAdapter is NULL error\n");
		return;
	}
	prGlueInfo = prAdapter->prGlueInfo;
	prHifInfo = &prGlueInfo->rHifInfo;

	usb_kill_anchored_urbs(&prHifInfo->rRxDataAnchor);
	usb_kill_anchored_urbs(&prHifInfo->rRxEventAnchor);
#if CFG_CHIP_RESET_SUPPORT
	if (prAdapter->chip_info->fgIsSupportL0p5Reset &&
	    prAdapter->chip_info->bus_info->fgIsSupportWdtEp)
		usb_kill_anchored_urbs(&prHifInfo->rRxWdtAnchor);
#endif

	if (!wlanIsChipNoAck(prAdapter))
		glUdmaRxAggEnable(prGlueInfo, FALSE);
	prAdapter->fgIsIntEnable = FALSE;
}

/*----------------------------------------------------------------------------*/
/*!
* \brief This routine is used to process the POWER OFF procedure.
*
* \param[in] pvAdapter Pointer to the Adapter structure.
*
* \return (none)
*/
/*----------------------------------------------------------------------------*/
u_int8_t halSetDriverOwn(IN struct ADAPTER *prAdapter)
{
	return TRUE;
}

/*----------------------------------------------------------------------------*/
/*!
* \brief This routine is used to process the POWER ON procedure.
*
* \param[in] pvAdapter Pointer to the Adapter structure.
*
* \return (none)
*/
/*----------------------------------------------------------------------------*/
void halSetFWOwn(IN struct ADAPTER *prAdapter, IN u_int8_t fgEnableGlobalInt)
{
}

void halWakeUpWiFi(IN struct ADAPTER *prAdapter)
{
	struct GL_HIF_INFO *prHifInfo;
	u_int8_t fgResult = FALSE;
	uint8_t ucCount = 0;

	DBGLOG(INIT, INFO, "Power on Wi-Fi....\n");

	prHifInfo = &prAdapter->prGlueInfo->rHifInfo;

	/* Remove check, because just check fw is dl state or not,
	*  but fw change to dl state is not only by wakeup cmd, and
	*  fw new cal flow need driver to notice fw wifi driver is
	*  insert, so it need issue this cmd always when wifi is
	*  insert.
	*/
	while (!fgResult) {
		HAL_WIFI_FUNC_POWER_ON(prAdapter);
		kalMdelay(50);
		HAL_WIFI_FUNC_READY_CHECK(prAdapter, WIFI_FUNC_INIT_DONE, &fgResult);

		ucCount++;

		if (ucCount >= 5) {
			DBGLOG(INIT, WARN, "Power on failed!!!\n");
			break;
		}
	}

	if (prHifInfo->state == USB_STATE_WIFI_OFF && fgResult)
		glUsbSetState(prHifInfo,
			USB_STATE_LINK_UP);

	prAdapter->fgIsFwOwn = FALSE;
}

void halEnableFWDownload(IN struct ADAPTER *prAdapter, IN u_int8_t fgEnable)
{
#if (CFG_UMAC_GENERATION >= 0x20)
	struct mt66xx_chip_info *prChipInfo;

	if (prAdapter == NULL) {
		DBGLOG(HAL, ERROR, "prAdapter is NULL error\n");
		return;
	}

	prChipInfo = prAdapter->chip_info;
	if (prChipInfo->asicEnableFWDownload) {
		prChipInfo->asicEnableFWDownload(prAdapter, fgEnable);
	} else {
		uint32_t u4Value = 0;

		HAL_MCR_RD(prAdapter, UDMA_TX_QSEL, &u4Value);

		if (fgEnable)
			u4Value |= FW_DL_EN;
		else
			u4Value &= ~FW_DL_EN;

		HAL_MCR_WR(prAdapter, UDMA_TX_QSEL, u4Value);
	}
#endif
}

void halDevInit(IN struct ADAPTER *prAdapter)
{
	struct GLUE_INFO *prGlueInfo;

	if (prAdapter == NULL) {
		DBGLOG(HAL, ERROR, "prAdapter is NULL error\n");
		return;
	}
	prGlueInfo = prAdapter->prGlueInfo;

	glUdmaRxAggEnable(prGlueInfo, FALSE);
	glUdmaTxRxEnable(prGlueInfo, TRUE);
}

u_int32_t halTxGetFreeCmdCnt(IN struct ADAPTER *prAdapter)
{
	struct GLUE_INFO *prGlueInfo;
	struct GL_HIF_INFO *prHifInfo;
	struct USB_REQ *prUsbReq, *prNext;
	unsigned long flags;
	u_int16_t u2Cnt = 0;

	if (prAdapter == NULL) {
		DBGLOG(HAL, ERROR, "prAdapter is NULL error\n");
		return 0;
	}
	prGlueInfo = prAdapter->prGlueInfo;
	prHifInfo = &prGlueInfo->rHifInfo;

	spin_lock_irqsave(&prHifInfo->rTxCmdQLock, flags);
	list_for_each_entry_safe(prUsbReq, prNext, &prHifInfo->rTxCmdFreeQ, list)
		u2Cnt++;
	spin_unlock_irqrestore(&prHifInfo->rTxCmdQLock, flags);
	return u2Cnt;
}

u_int8_t halTxIsDataBufEnough(IN struct ADAPTER *prAdapter, IN struct MSDU_INFO *prMsduInfo)
{
	struct GLUE_INFO *prGlueInfo = prAdapter->prGlueInfo;
	struct GL_HIF_INFO *prHifInfo = &prGlueInfo->rHifInfo;
#if CFG_USB_TX_AGG
	struct USB_REQ *prUsbReq;
	struct BUF_CTRL *prBufCtrl;
#endif
	uint8_t ucTc;
	struct sk_buff *skb;
	uint32_t u4Length;
	struct mt66xx_chip_info *prChipInfo;

	unsigned long flags;

	prChipInfo = prAdapter->chip_info;
	skb = (struct sk_buff *)prMsduInfo->prPacket;
	u4Length = skb->len;
	u4Length += prChipInfo->u2HifTxdSize;
	ucTc = USB_TRANS_MSDU_TC(prMsduInfo);

	spin_lock_irqsave(&prHifInfo->rTxDataQLock, flags);

#if CFG_USB_TX_AGG
	if (list_empty(&prHifInfo->rTxDataFreeQ[ucTc])) {
		if (glUsbBorrowFfaReq(prHifInfo, ucTc) == FALSE) {
			spin_unlock_irqrestore(&prHifInfo->rTxDataQLock, flags);
			return FALSE;
		}
	}

	prUsbReq = list_entry(prHifInfo->rTxDataFreeQ[ucTc].next, struct USB_REQ, list);
	prBufCtrl = prUsbReq->prBufCtrl;

	if (prHifInfo->rTxDataFreeQ[ucTc].next->next == &prHifInfo->rTxDataFreeQ[ucTc]) {
		/* length of rTxDataFreeQ equals 1 */
		if (prBufCtrl->u4WrIdx + ALIGN_4(u4Length) >
		    prBufCtrl->u4BufSize - prHifInfo->u4AggRsvSize[ucTc] - LEN_USB_UDMA_TX_TERMINATOR) {
			/* Buffer is not enough */
			if (glUsbBorrowFfaReq(prHifInfo, ucTc) == FALSE) {
				spin_unlock_irqrestore(&prHifInfo->rTxDataQLock,
							flags);
				return FALSE;
			}
		}
	}
	prHifInfo->u4AggRsvSize[ucTc] += ALIGN_4(u4Length);
#else
	if (list_empty(&prHifInfo->rTxDataFreeQ)) {
		spin_unlock_irqrestore(&prHifInfo->rTxDataQLock, flags);

		return FALSE;
	}
#endif

	spin_unlock_irqrestore(&prHifInfo->rTxDataQLock, flags);
	return TRUE;
}

uint8_t halTxRingDataSelect(IN struct ADAPTER *prAdapter,
	IN struct MSDU_INFO *prMsduInfo)
{
	return 0;
}

void halUpdateTxMaxQuota(IN struct ADAPTER *prAdapter)
{
	struct BUS_INFO *prBusInfo;
	uint32_t u4Ret;
	uint32_t u4Quota;
	bool fgRun;
	uint8_t ucWmmIndex;
	KAL_SPIN_LOCK_DECLARATION();

	prBusInfo = prAdapter->chip_info->bus_info;

	for (ucWmmIndex = 0; ucWmmIndex < prAdapter->ucWmmSetNum;
		ucWmmIndex++) {
		u4Ret = WLAN_STATUS_SUCCESS;

		KAL_ACQUIRE_SPIN_LOCK(prAdapter, SPIN_LOCK_UPDATE_WMM_QUOTA);
		u4Quota = prAdapter->rWmmQuotaReqCS[ucWmmIndex].u4Quota;
		fgRun = prAdapter->rWmmQuotaReqCS[ucWmmIndex].fgRun;
		KAL_RELEASE_SPIN_LOCK(prAdapter, SPIN_LOCK_UPDATE_WMM_QUOTA);

		if (fgRun) {
			if (prBusInfo->updateTxRingMaxQuota) {
				u4Ret = prBusInfo->updateTxRingMaxQuota(
					prAdapter, (uint16_t)ucWmmIndex, u4Quota);
			} else {
				DBGLOG(HAL, INFO,
					"updateTxRingMaxQuota not implemented\n");
				u4Ret = WLAN_STATUS_NOT_ACCEPTED;
			}
		}

		DBGLOG(HAL, INFO,
			"WmmQuota,Run,%u,Wmm,%u,Quota,0x%x,ret=0x%x\n",
			fgRun, ucWmmIndex, u4Quota, u4Ret);
		if (u4Ret != WLAN_STATUS_PENDING) {
			KAL_ACQUIRE_SPIN_LOCK(prAdapter,
				SPIN_LOCK_UPDATE_WMM_QUOTA);
			prAdapter->rWmmQuotaReqCS[ucWmmIndex].fgRun
				= false;
			KAL_RELEASE_SPIN_LOCK(prAdapter,
				SPIN_LOCK_UPDATE_WMM_QUOTA);
		}
	}
}

void halProcessTxInterrupt(IN struct ADAPTER *prAdapter)
{
#if CFG_USB_TX_HANDLE_IN_HIF_THREAD
	struct USB_REQ *prUsbReq;
	struct GL_HIF_INFO *prHifInfo = &prAdapter->prGlueInfo->rHifInfo;

	/* Process complete Tx cmd */
	prUsbReq = glUsbDequeueReq(prHifInfo, &prHifInfo->rTxCmdCompleteQ, &prHifInfo->rTxCmdQLock);
	while (prUsbReq) {
		halTxUSBProcessCmdComplete(prAdapter, prUsbReq);
		prUsbReq = glUsbDequeueReq(prHifInfo,
						&prHifInfo->rTxCmdCompleteQ,
						&prHifInfo->rTxCmdQLock);
	}

	/* Process complete Tx data */
	prUsbReq = glUsbDequeueReq(prHifInfo, &prHifInfo->rTxDataCompleteQ,
					&prHifInfo->rTxDataQLock);
	while (prUsbReq) {
		halTxUSBProcessDataComplete(prAdapter, prUsbReq);
		prUsbReq = glUsbDequeueReq(prHifInfo,
						&prHifInfo->rTxDataCompleteQ,
						&prHifInfo->rTxDataQLock);
	}
#endif
}

bool halHifSwInfoInit(IN struct ADAPTER *prAdapter)
{
	struct mt66xx_chip_info *prChipInfo;
	struct BUS_INFO *prBusInfo;

	prChipInfo = prAdapter->chip_info;
	prBusInfo = prChipInfo->bus_info;

	if (prBusInfo->DmaShdlInit)
		prBusInfo->DmaShdlInit(prAdapter);

	if (prChipInfo->asicUsbInit)
		prChipInfo->asicUsbInit(prAdapter, prChipInfo);

	if (prBusInfo->asicUdmaRxFlush)
		prBusInfo->asicUdmaRxFlush(prAdapter, FALSE);

	/* set USB EP_RST_OPT as reset scope excludes toggle
	 * bit,sequence number, etc.
	 */
	if (prBusInfo->asicUsbEpctlRstOpt)
		prBusInfo->asicUsbEpctlRstOpt(prAdapter,
					      FALSE);

	return true;
}

void halRxProcessMsduReport(IN struct ADAPTER *prAdapter, IN OUT struct SW_RFB *prSwRfb)
{

}

#if (CFG_SUPPORT_CONNAC3X == 1)
u_int8_t halProcessToken(IN struct ADAPTER *prAdapter,
	IN uint32_t u4Token,
	IN struct QUE *prFreeQueue)
{
	return 0;
}
#endif

static uint32_t halTxGetPageCount(IN struct ADAPTER *prAdapter,
	IN uint32_t u4FrameLength, IN u_int8_t fgIncludeDesc)
{
#if CFG_USB_TX_AGG
	struct mt66xx_chip_info *prChipInfo = prAdapter->chip_info;
	uint32_t u4RequiredBufferSize;
	uint32_t u4PageCount;
	uint32_t u4TxHeadRoomSize = NIC_TX_DESC_AND_PADDING_LENGTH + prChipInfo->txd_append_size;

	/* Frame Buffer
	 *  |<--Tx Descriptor-->|<--Tx descriptor padding-->|
	 *  <--802.3/802.11 Header-->|<--Header padding-->|<--Payload-->|
	 */

	if (fgIncludeDesc)
		u4RequiredBufferSize = u4FrameLength;
	else
		u4RequiredBufferSize = u4TxHeadRoomSize + u4FrameLength;

	u4RequiredBufferSize = ALIGN_4(u4RequiredBufferSize);

	if (NIC_TX_PAGE_SIZE_IS_POWER_OF_2)
		u4PageCount = (u4RequiredBufferSize + (NIC_TX_PAGE_SIZE - 1)) >> NIC_TX_PAGE_SIZE_IN_POWER_OF_2;
	else
		u4PageCount = (u4RequiredBufferSize + (NIC_TX_PAGE_SIZE - 1)) / NIC_TX_PAGE_SIZE;

	return u4PageCount;
#else
	return 1;
#endif
}

uint32_t halTxGetDataPageCount(IN struct ADAPTER *prAdapter,
	IN uint32_t u4FrameLength, IN u_int8_t fgIncludeDesc)
{
	return halTxGetPageCount(prAdapter, u4FrameLength, fgIncludeDesc);
}

uint32_t halTxGetCmdPageCount(IN struct ADAPTER *prAdapter,
	IN uint32_t u4FrameLength, IN u_int8_t fgIncludeDesc)
{
	return halTxGetPageCount(prAdapter, u4FrameLength, fgIncludeDesc);
}

uint32_t halTxPollingResource(IN struct ADAPTER *prAdapter, IN uint8_t ucTC)
{
	return WLAN_STATUS_SUCCESS;
}

void halSerHifReset(IN struct ADAPTER *prAdapter)
{
	uint32_t i;

	/**
	 * usb_reset_endpoint - Reset an endpoint's state.
	 * @dev: the device whose endpoint is to be reset
	 * @epaddr: the endpoint's address.  Endpoint number for output,
	 *	endpoint number + USB_DIR_IN for input
	 *
	 * Resets any host-side endpoint state such as the toggle bit,
	 * sequence number or current window.
	 *
	 * void usb_reset_endpoint(struct usb_device *dev, unsigned int epaddr);
	 */

	/* SER new flow: just flush out ep out fifo,
	* not to reset ep out endpoint
	*/
#if 0
	/* reset ALL BULK OUT endpoints */
	for (i = USB_DATA_BULK_OUT_EP4; i <= USB_DATA_BULK_OUT_EP9; i++)
		usb_reset_endpoint(prAdapter->prGlueInfo->rHifInfo.udev, i);
#endif
	/* reset ALL BULK IN endpoints */
	for (i = USB_DATA_BULK_IN_EP4; i <= USB_DATA_BULK_IN_EP5; i++)
		usb_reset_endpoint(prAdapter->prGlueInfo->rHifInfo.udev,
					i | USB_DIR_IN);
}

void halProcessRxInterrupt(IN struct ADAPTER *prAdapter)
{
	struct GL_HIF_INFO *prHifInfo = &prAdapter->prGlueInfo->rHifInfo;

	/* Process complete data */
	halRxUSBProcessEventDataComplete(prAdapter, &prHifInfo->rRxDataCompleteQ,
		&prHifInfo->rRxDataFreeQ, USB_RX_DATA_RFB_RSV_CNT);
	halRxUSBReceiveData(prAdapter);

	if (prHifInfo->eEventEpType != EVENT_EP_TYPE_DATA_EP) {
		/* Process complete event */
		halRxUSBProcessEventDataComplete(prAdapter, &prHifInfo->rRxEventCompleteQ,
			&prHifInfo->rRxEventFreeQ, USB_RX_EVENT_RFB_RSV_CNT);
		halRxUSBReceiveEvent(prAdapter, FALSE);
	}

#if CFG_CHIP_RESET_SUPPORT
	if (prAdapter->chip_info->fgIsSupportL0p5Reset &&
	    prAdapter->chip_info->bus_info->fgIsSupportWdtEp) {
		halRxUSBProcessWdtComplete(prAdapter,
					   &prHifInfo->rRxWdtCompleteQ,
					   &prHifInfo->rRxWdtFreeQ,
					   USB_RX_WDT_RFB_RSV_CNT);

		halRxUSBReceiveWdt(prAdapter);
	}
#endif
}

uint32_t halDumpHifStatus(IN struct ADAPTER *prAdapter, IN uint8_t *pucBuf, IN uint32_t u4Max)
{
	uint32_t u4CpuIdx, u4DmaIdx, u4Int, u4GloCfg, u4Reg;
	uint32_t u4Len = 0;
	struct GLUE_INFO *prGlueInfo = prAdapter->prGlueInfo;
	uint8_t pBuffer[512] = {0};

	HAL_MCR_RD(prAdapter, 0x820b0118, &u4CpuIdx);
	HAL_MCR_RD(prAdapter, 0x820b011c, &u4DmaIdx);
	HAL_MCR_RD(prAdapter, 0x820b0220, &u4Int);
	HAL_MCR_RD(prAdapter, 0x820b0204, &u4GloCfg);

	LOGBUF(pucBuf, u4Max, u4Len, "\n");
	LOGBUF(pucBuf, u4Max, u4Len, "PDMA1R1 CPU[%u] DMA[%u] INT[0x%08x] CFG[0x%08x]\n", u4CpuIdx,
		u4DmaIdx, u4Int, u4GloCfg);

	HAL_MCR_RD(prAdapter, UDMA_WLCFG_0, &u4Reg);

	LOGBUF(pucBuf, u4Max, u4Len, "UDMA WLCFG[0x%08x]\n", u4Reg);

	LOGBUF(pucBuf, u4Max, u4Len, "\n");
	LOGBUF(pucBuf, u4Max, u4Len, "VenderID: %04x\n",
		glGetUsbDeviceVendorId(prGlueInfo->rHifInfo.udev));
	LOGBUF(pucBuf, u4Max, u4Len, "ProductID: %04x\n",
		glGetUsbDeviceProductId(prGlueInfo->rHifInfo.udev));

	glGetUsbDeviceManufacturerName(prGlueInfo->rHifInfo.udev, pBuffer,
		sizeof(pBuffer));
	LOGBUF(pucBuf, u4Max, u4Len, "Manufacturer: %s\n",
		pBuffer);

	glGetUsbDeviceProductName(prGlueInfo->rHifInfo.udev, pBuffer,
		sizeof(pBuffer));
	LOGBUF(pucBuf, u4Max, u4Len, "Product: %s\n", pBuffer);

	glGetUsbDeviceSerialNumber(prGlueInfo->rHifInfo.udev, pBuffer,
		sizeof(pBuffer));
	LOGBUF(pucBuf, u4Max, u4Len, "SerialNumber: %s\n",
		pBuffer);

	return u4Len;
}

void halGetCompleteStatus(IN struct ADAPTER *prAdapter, OUT uint32_t *pu4IntStatus)
{
#if CFG_USB_RX_HANDLE_IN_HIF_THREAD || CFG_USB_TX_HANDLE_IN_HIF_THREAD
	struct GL_HIF_INFO *prHifInfo = &prAdapter->prGlueInfo->rHifInfo;
#endif

	*pu4IntStatus = 0;

#if CFG_USB_RX_HANDLE_IN_HIF_THREAD
	if (!list_empty(&prHifInfo->rRxDataCompleteQ) || !list_empty(&prHifInfo->rRxEventCompleteQ))
		*pu4IntStatus |= WHISR_RX0_DONE_INT;
#endif

#if CFG_USB_TX_HANDLE_IN_HIF_THREAD
	if (!list_empty(&prHifInfo->rTxDataCompleteQ) || !list_empty(&prHifInfo->rTxCmdCompleteQ))
		*pu4IntStatus |= WHISR_TX_DONE_INT;
#endif
}

u_int8_t halIsPendingRx(IN struct ADAPTER *prAdapter)
{
#if CFG_USB_RX_HANDLE_IN_HIF_THREAD
	struct GL_HIF_INFO *prHifInfo = &prAdapter->prGlueInfo->rHifInfo;

	if (!list_empty(&prHifInfo->rRxDataCompleteQ) || !list_empty(&prHifInfo->rRxEventCompleteQ))
		return TRUE;
	else
		return FALSE;
#else
	return FALSE;
#endif
}

/*----------------------------------------------------------------------------*/
/*!
* @brief Send HIF_CTRL command to inform FW stop send packet/event to host
*	suspend = 1
*
* @param prAdapter      Pointer to the Adapter structure.
*
* @return (void)
*/
/*----------------------------------------------------------------------------*/
void halUSBPreSuspendCmd(IN struct ADAPTER *prAdapter)
{
	struct CMD_HIF_CTRL rCmdHifCtrl;
	uint32_t rStatus;

	kalMemSet(&rCmdHifCtrl, 0, sizeof(struct CMD_HIF_CTRL));

	rCmdHifCtrl.ucHifType = ENUM_HIF_TYPE_USB;
	rCmdHifCtrl.ucHifDirection = ENUM_HIF_TX;
	rCmdHifCtrl.ucHifStop = 1;
	rCmdHifCtrl.ucHifSuspend = 1;
	rCmdHifCtrl.u4WakeupHifType = ENUM_CMD_HIF_WAKEUP_TYPE_USB;

	rStatus = wlanSendSetQueryCmd(prAdapter,	/* prAdapter */
				      CMD_ID_HIF_CTRL,	/* ucCID */
				      TRUE,	/* fgSetQuery */
				      FALSE,	/* fgNeedResp */
				      FALSE,	/* fgIsOid */
				      NULL,	/* pfCmdDoneHandler */
				      NULL,	/* pfCmdTimeoutHandler */
				      sizeof(struct CMD_HIF_CTRL),	/* u4SetQueryInfoLen */
				      (uint8_t *)&rCmdHifCtrl,	/* pucInfoBuffer */
				      NULL,	/* pvSetQueryBuffer */
				      0	/* u4SetQueryBufferLen */
				     );

	if (rStatus != WLAN_STATUS_PENDING)
		DBGLOG(HAL, ERROR, "wlanSendSetQueryCmd error\n");
}

/*----------------------------------------------------------------------------*/
/*!
* @brief Send HIF_CTRL command to inform FW allow send packet/event to host
*	suspend = 0
*
* @param prAdapter      Pointer to the Adapter structure.
*
* @return (void)
*/
/*----------------------------------------------------------------------------*/
void halUSBPreResumeCmd(IN struct ADAPTER *prAdapter)
{
	struct CMD_HIF_CTRL rCmdHifCtrl;
	uint32_t rStatus;

	kalMemSet(&rCmdHifCtrl, 0, sizeof(struct CMD_HIF_CTRL));

	rCmdHifCtrl.ucHifType = ENUM_HIF_TYPE_USB;
	rCmdHifCtrl.ucHifDirection = ENUM_HIF_TX;
	rCmdHifCtrl.ucHifStop = 0;
	rCmdHifCtrl.ucHifSuspend = 0;
	rCmdHifCtrl.u4WakeupHifType = ENUM_CMD_HIF_WAKEUP_TYPE_USB;

	rStatus = wlanSendSetQueryCmd(prAdapter, /* prAdapter */
				CMD_ID_HIF_CTRL,	/* ucCID */
				TRUE,	/* fgSetQuery */
				FALSE,	/* fgNeedResp */
				FALSE,	/* fgIsOid */
				NULL,	/* nicEventHifCtrl */
				NULL,	/* pfCmdTimeoutHandler */
				sizeof(struct CMD_HIF_CTRL),
				(uint8_t *)&rCmdHifCtrl, /* pucInfoBuffer */
				NULL,	/* pvSetQueryBuffer */
				0		/* u4SetQueryBufferLen */
	    );

	if (rStatus != WLAN_STATUS_PENDING)
		DBGLOG(HAL, ERROR, "wlanSendSetQueryCmd error\n");
}

void halUSBPreSuspendDone(IN struct ADAPTER *prAdapter, IN struct CMD_INFO *prCmdInfo, IN uint8_t *pucEventBuf)
{
	unsigned long flags;
	struct GL_HIF_INFO *prHifInfo;

	if (prAdapter == NULL) {
		DBGLOG(HAL, ERROR, "prAdapter is NULL error\n");
		return;
	}
	prHifInfo = &prAdapter->prGlueInfo->rHifInfo;

	spin_lock_irqsave(&prHifInfo->rStateLock, flags);

	if (prHifInfo->state == USB_STATE_LINK_UP
		|| prHifInfo->state == USB_STATE_PRE_SUSPEND)
		prHifInfo->state = USB_STATE_SUSPEND;
	else
		DBGLOG(HAL, ERROR, "Previous USB state (%d)!\n",
			prHifInfo->state);

	spin_unlock_irqrestore(&prHifInfo->rStateLock, flags);
}

void halUSBPreSuspendTimeout(IN struct ADAPTER *prAdapter, IN struct CMD_INFO *prCmdInfo)
{
	unsigned long flags;
	struct GL_HIF_INFO *prHifInfo;

	if (prAdapter == NULL) {
		DBGLOG(HAL, ERROR, "prAdapter is NULL error\n");
		return;
	}
	prHifInfo = &prAdapter->prGlueInfo->rHifInfo;

	spin_lock_irqsave(&prHifInfo->rStateLock, flags);

	if (prHifInfo->state == USB_STATE_LINK_UP
		|| prHifInfo->state == USB_STATE_PRE_SUSPEND)
		prHifInfo->state = USB_STATE_PRE_SUSPEND_FAIL;
	else
		DBGLOG(HAL, ERROR, "Previous USB state (%d)!\n",
			prHifInfo->state);

	spin_unlock_irqrestore(&prHifInfo->rStateLock, flags);
}

uint32_t halGetValidCoalescingBufSize(IN struct ADAPTER *prAdapter)
{
	uint32_t u4BufSize;

	if (HIF_TX_COALESCING_BUFFER_SIZE > HIF_RX_COALESCING_BUFFER_SIZE)
		u4BufSize = HIF_TX_COALESCING_BUFFER_SIZE;
	else
		u4BufSize = HIF_RX_COALESCING_BUFFER_SIZE;

	return u4BufSize;
}

uint32_t halAllocateIOBuffer(IN struct ADAPTER *prAdapter)
{
	return WLAN_STATUS_SUCCESS;
}

uint32_t halReleaseIOBuffer(IN struct ADAPTER *prAdapter)
{
	return WLAN_STATUS_SUCCESS;
}

void halProcessAbnormalInterrupt(IN struct ADAPTER *prAdapter)
{

}

void halProcessSoftwareInterrupt(IN struct ADAPTER *prAdapter)
{

}

void halDeAggRxPktWorker(struct work_struct *work)
{
	struct GLUE_INFO *prGlueInfo = ENTRY_OF(work, struct GLUE_INFO, rRxPktDeAggWork);

	/*tasklet_hi_schedule(&prGlueInfo->rRxTask);*/
	tasklet_schedule(&prGlueInfo->rRxTask);
}

void halRxTasklet(unsigned long data)
{
	struct GLUE_INFO *prGlueInfo = (struct GLUE_INFO *)data;

	halProcessRxInterrupt(prGlueInfo->prAdapter);
}

void halTxCompleteTasklet(unsigned long data)
{
	struct GLUE_INFO *prGlueInfo = (struct GLUE_INFO *)data;
	struct GL_HIF_INFO *prHifInfo = &prGlueInfo->rHifInfo;
	struct USB_REQ *prUsbReq;

	/* Process complete Tx data */
	prUsbReq = glUsbDequeueReq(prHifInfo, &prHifInfo->rTxDataCompleteQ, &prHifInfo->rTxDataQLock);
	while (prUsbReq) {
		halTxUSBProcessDataComplete(prGlueInfo->prAdapter, prUsbReq);
		prUsbReq = glUsbDequeueReq(prHifInfo, &prHifInfo->rTxDataCompleteQ, &prHifInfo->rTxDataQLock);
	}
}

/* Hif power off wifi */
uint32_t halHifPowerOffWifi(IN struct ADAPTER *prAdapter)
{
	uint32_t rStatus = WLAN_STATUS_SUCCESS;

	DBGLOG(INIT, INFO, "Power off Wi-Fi!\n");

	/* Power off Wi-Fi */
	wlanSendNicPowerCtrlCmd(prAdapter, TRUE);

	rStatus = wlanCheckWifiFunc(prAdapter, FALSE);

	glUsbSetState(&prAdapter->prGlueInfo->rHifInfo, USB_STATE_WIFI_OFF);

	nicDisableInterrupt(prAdapter);

	wlanClearPendingInterrupt(prAdapter);

	halTxCancelAllSending(prAdapter);

	return rStatus;
}

void halPrintHifDbgInfo(IN struct ADAPTER *prAdapter)
{

}

u_int8_t halIsTxResourceControlEn(IN struct ADAPTER *prAdapter)
{
	return FALSE;
}

void halTxResourceResetHwTQCounter(IN struct ADAPTER *prAdapter)
{
}

static uint32_t halGetHifTxPageSize(IN struct ADAPTER *prAdapter)
{
	return HIF_TX_PAGE_SIZE;
}

uint32_t halGetHifTxCmdPageSize(IN struct ADAPTER *prAdapter)
{
	return halGetHifTxPageSize(prAdapter);
}

uint32_t halGetHifTxDataPageSize(IN struct ADAPTER *prAdapter)
{
	return halGetHifTxPageSize(prAdapter);
}

u_int8_t halSerHappendInSuspend(IN struct ADAPTER *prAdapter)
{
	uint32_t u4SerEvnt;

	if (!prAdapter->chip_info->u4SerSuspendSyncAddr)
		return FALSE;

	HAL_MCR_RD(prAdapter, prAdapter->chip_info->u4SerSuspendSyncAddr,
		   &u4SerEvnt);

	return !!(u4SerEvnt & ERROR_DETECT_L1_TRIGGER_IN_SUSPEND);
}

void halSerPollDoneInSuspend(IN struct ADAPTER *prAdapter)
{
	uint32_t u4SerEvnt;
	uint32_t u4Tick;

	if (!prAdapter->chip_info->u4SerSuspendSyncAddr)
		return;

	u4Tick = kalGetTimeTick();

	while (1) {
		HAL_MCR_RD(prAdapter,
			   prAdapter->chip_info->u4SerSuspendSyncAddr,
			   &u4SerEvnt);

		if (u4SerEvnt & ERROR_DETECT_L1_DONE_IN_SUSPEND) {
			DBGLOG(HAL, INFO, "[SER][L1] reset done in suspend\n");

			break;
		}

		kalMsleep(10);

		if (CHECK_FOR_TIMEOUT(kalGetTimeTick(), u4Tick,
			       MSEC_TO_SYSTIME(WIFI_SER_L1_RST_DONE_TIMEOUT))) {

			DBGLOG(HAL, ERROR,
			       "[SER][L1] reset timeout in suspend\n");
			break;
		}
	}

	/* clear SER bits */
	HAL_MCR_RD(prAdapter, prAdapter->chip_info->u4SerSuspendSyncAddr,
		   &u4SerEvnt);
	u4SerEvnt &= ~(ERROR_DETECT_L1_TRIGGER_IN_SUSPEND |
		       ERROR_DETECT_L1_DONE_IN_SUSPEND);
	HAL_MCR_WR(prAdapter, prAdapter->chip_info->u4SerSuspendSyncAddr,
		   u4SerEvnt);
}

uint32_t halSerGetMcuEvent(IN struct ADAPTER *prAdapter, IN u_int8_t fgClear)
{
	struct GLUE_INFO *prGlueInfo;
	struct mt66xx_chip_info *prChipInfo;
	struct GL_HIF_INFO *prHifInfo;
	uint32_t u4SerAction;
	enum usb_state state;

	prGlueInfo = prAdapter->prGlueInfo;
	prChipInfo = prAdapter->chip_info;
	prHifInfo = &prGlueInfo->rHifInfo;

	state = glUsbGetState(prHifInfo);
	if (!(state == USB_STATE_PRE_SUSPEND
		|| state == USB_STATE_PRE_RESUME
		|| state == USB_STATE_LINK_UP))
		return 0;

	/* get MCU SER event */
	if (!kalDevRegRead(prGlueInfo,
			   prChipInfo->u4SerUsbMcuEventAddr,
			   &u4SerAction)) {
		DBGLOG(NIC, WARN, "get MCU SER event fail\n");

		u4SerAction = 0;

		goto out;
	}

	if (u4SerAction && fgClear) {
		DBGLOG(NIC, INFO, "u4SerAction=0x%08X\n", u4SerAction);

		/* clear MCU SER event */
		kalDevRegWrite(prGlueInfo,
			       prChipInfo->u4SerUsbMcuEventAddr, 0);
	}

out:
	return u4SerAction;
}

void halSerSyncTimerHandler(IN struct ADAPTER *prAdapter)
{
	struct ERR_RECOVERY_CTRL_T *prErrRecoveryCtrl;
	uint32_t u4SerAction;
	struct mt66xx_chip_info *prChipInfo;
	struct BUS_INFO *prBusInfo;

	prChipInfo = prAdapter->chip_info;
	prBusInfo = prChipInfo->bus_info;
	prErrRecoveryCtrl = &prAdapter->prGlueInfo->rHifInfo.rErrRecoveryCtl;

	/* get MCU SER event */
	u4SerAction = halSerGetMcuEvent(prAdapter, TRUE);
	if (u4SerAction == 0)
		return;
	else {
		if ((prAdapter->rWifiVar.eEnableSerL1 !=
		     FEATURE_OPT_SER_ENABLE)
#if (CFG_SUPPORT_WFDMA_REALLOC == 1)
		    /* Since the execution of wfdma realloc relies on L1 reset,
		     * it self becomes one of the condition checks of whether
		     * perform L1 reset or not. Ex: If wfdma realloc is
		     * supported and not done yet, then the perform of L1 reset
		     * is allowed.
		     */
		    && !(prAdapter->fgIsSupportWfdmaRealloc &&
			 !prAdapter->fgWfdmaReallocDone)
#endif /* CFG_SUPPORT_WFDMA_REALLOC */
			) {
			if (prChipInfo->asicDumpSerDummyCR)
				prChipInfo->asicDumpSerDummyCR(prAdapter);

			DBGLOG(HAL, WARN,
			       "[SER][L1] Bypass L1 reset due to wifi.cfg\n");

			GL_DEFAULT_RESET_TRIGGER(prAdapter, RST_SER_L1_FAIL);

			return;
		}
	}

	switch (prErrRecoveryCtrl->eErrRecovState) {
	case ERR_RECOV_STOP_IDLE:
		if (u4SerAction == ERROR_DETECT_STOP_PDMA) {
			if (prChipInfo->asicDumpSerDummyCR)
				prChipInfo->asicDumpSerDummyCR(prAdapter);

			DBGLOG(HAL, INFO,
				"SER(E) Host stop HIF tx/rx operation\n");

			/* change SER FSM to SER_STOP_HOST_TX_RX */
			nicSerStopTxRx(prAdapter);
			/* stop TX BULK OUT URB */
			halTxCancelAllSending(prAdapter);
			/* stop RX BULK IN URB */
			halDisableInterrupt(prAdapter);

			DBGLOG(HAL, INFO,
			"SER(F) Host ACK HIF tx/rx stop operation done\n");

			/* Send Host stops TX/RX done response to mcu */
			kalDevRegWrite(prAdapter->prGlueInfo,
					prChipInfo->u4SerUsbHostAckAddr,
					MCU_INT_PDMA0_STOP_DONE);
			prErrRecoveryCtrl->eErrRecovState =
				ERR_RECOV_STOP_PDMA0;
		} else {
			/* do nothing */
		}
		break;

	case ERR_RECOV_STOP_PDMA0:
		if (u4SerAction == ERROR_DETECT_RESET_DONE) {
			DBGLOG(HAL, INFO, "SER(L) Host re-initialize WFDMA\n");
			DBGLOG(HAL, INFO, "SER(M) Host enable WFDMA\n");

			if (prChipInfo->asicUsbInit)
				prChipInfo->asicUsbInit(prAdapter, prChipInfo);

			DBGLOG(HAL, INFO,
				"SER(N) Host ACK WFDMA init done\n");
			/* Send Host stops TX/RX done response to mcu */
			kalDevRegWrite(prAdapter->prGlueInfo,
					prChipInfo->u4SerUsbHostAckAddr,
					MCU_INT_PDMA0_INIT_DONE);

			prErrRecoveryCtrl->eErrRecovState =
				ERR_RECOV_RESET_PDMA0;
		} else {
			/* do nothing */
		}
		break;
	case ERR_RECOV_RESET_PDMA0:
		if (u4SerAction == ERROR_DETECT_RECOVERY_DONE) {
			if (prBusInfo->DmaShdlReInit)
				prBusInfo->DmaShdlReInit(prAdapter);

			DBGLOG(HAL, INFO,
				"SER(Q) Host ACK MCU SER handle done\n");
			/* Send Host stops TX/RX done response to mcu */
			kalDevRegWrite(prAdapter->prGlueInfo,
				prChipInfo->u4SerUsbHostAckAddr,
				MCU_INT_PDMA0_RECOVERY_DONE);
			prErrRecoveryCtrl->eErrRecovState =
				ERR_RECOV_WAIT_MCU_NORMAL;
		} else {
			/* do nothing */
		}
		break;

	case ERR_RECOV_WAIT_MCU_NORMAL:
		if (u4SerAction == ERROR_DETECT_MCU_NORMAL_STATE) {
			/* update Beacon frame if operating in AP mode. */
			DBGLOG(HAL, INFO, "SER(T) Host re-initialize BCN\n");
			nicSerReInitBeaconFrame(prAdapter);

			DBGLOG(HAL, INFO,
				"SER(U) Host reset TX/RX endpoint\n");

			/* It's surprising that the toggle bit or sequence
			 * number of USB endpoints in some USB hosts cannot be
			 * reset by kernel API usb_reset_endpoint(). In order to
			 * prevent this IOT, we tend to do not reset toggle bit
			 * and sequence number on both device and host in some
			 * project like MT7961. In MT7961, we can choose that
			 * endpoints reset excludes toggle bit and sequence
			 * number through asicUsbEpctlRstOpt().
			 */
			if (!prBusInfo->asicUsbEpctlRstOpt)
				halSerHifReset(prAdapter);

			/* resume TX/RX */
			nicSerStartTxRx(prAdapter);

			halEnableInterrupt(prAdapter);

			prErrRecoveryCtrl->eErrRecovState = ERR_RECOV_STOP_IDLE;

#if (CFG_SUPPORT_WFDMA_REALLOC == 1)
			if (prAdapter->fgOidWfdmaReallocPend) {
				prAdapter->fgOidWfdmaReallocPend = FALSE;

				kalOidComplete(prAdapter->prGlueInfo,
					       TRUE, 0, WLAN_STATUS_SUCCESS);
			}
#endif /* CFG_SUPPORT_WFDMA_REALLOC */
		} else {
			/* do nothing */
		}
		break;
	}
}

/*----------------------------------------------------------------------------*/
/*!
* @brief Check if HIF state is LINK_UP or READY for USB TX/RX
*
* @param prAdapter      Pointer to the Adapter structure.
*
* @return (TRUE: ready, FALSE: not ready)
*/
/*----------------------------------------------------------------------------*/
bool halIsHifStateLinkup(IN struct ADAPTER *prAdapter)
{
	if (!prAdapter)
		return FALSE;

	if (!prAdapter->prGlueInfo)
		return FALSE;

	if (prAdapter->prGlueInfo->rHifInfo.state != USB_STATE_LINK_UP)
		return FALSE;

	return TRUE;
}

/*----------------------------------------------------------------------------*/
/*!
* @brief Check if HIF state is READY for upper layer cfg80211
*
* @param prAdapter      Pointer to the Adapter structure.
*
* @return (TRUE: ready, FALSE: not ready)
*/
/*----------------------------------------------------------------------------*/
bool halIsHifStateReady(IN struct ADAPTER *prAdapter, uint8_t *pucState)
{
	if (!prAdapter)
		return FALSE;

	if (!prAdapter->prGlueInfo)
		return FALSE;

	if (pucState)
		*pucState = prAdapter->prGlueInfo->rHifInfo.state;

	if (prAdapter->prGlueInfo->rHifInfo.state != USB_STATE_LINK_UP)
		return FALSE;

	return TRUE;
}

/*----------------------------------------------------------------------------*/
/*!
* @brief Check if HIF state is during supend process
*
* @param prAdapter      Pointer to the Adapter structure.
*
* @return (TRUE: suspend, reject the caller action. FALSE: not suspend)
*/
/*----------------------------------------------------------------------------*/
bool halIsHifStateSuspend(IN struct ADAPTER *prAdapter)
{
	enum usb_state state;

	if (!prAdapter)
		return FALSE;

	if (!prAdapter->prGlueInfo)
		return FALSE;

	state = prAdapter->prGlueInfo->rHifInfo.state;

	if (state == USB_STATE_SUSPEND)
		return TRUE;

	return FALSE;
}

