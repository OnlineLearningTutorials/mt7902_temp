/* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause */
/*
 * Copyright (c) 2016 MediaTek Inc.
 */
/*
 ** Id: //Department/DaVinci/BRANCHES/MT6620_WIFI_DRIVER_V2_3/nic/nic.c#4
 */

/*! \file   nic.c
 *  \brief  Functions that provide operation in NIC's (Network Interface Card)
 *          point of view.
 *    This file includes functions which unite multiple hal(Hardware) operations
 *    and also take the responsibility of Software Resource Management in order
 *    to keep the synchronization with Hardware Manipulation.
 */


/*******************************************************************************
 *                         C O M P I L E R   F L A G S
 *******************************************************************************
 */

/*******************************************************************************
 *                    E X T E R N A L   R E F E R E N C E S
 *******************************************************************************
 */
#include "precomp.h"

/*******************************************************************************
 *                              C O N S T A N T S
 *******************************************************************************
 */
const uint8_t aucPhyCfg2PhyTypeSet[PHY_CONFIG_NUM] = {
	PHY_TYPE_SET_802_11ABG,	/* PHY_CONFIG_802_11ABG */
	PHY_TYPE_SET_802_11BG,	/* PHY_CONFIG_802_11BG */
	PHY_TYPE_SET_802_11G,	/* PHY_CONFIG_802_11G */
	PHY_TYPE_SET_802_11A,	/* PHY_CONFIG_802_11A */
	PHY_TYPE_SET_802_11B,	/* PHY_CONFIG_802_11B */
	PHY_TYPE_SET_802_11ABGN,	/* PHY_CONFIG_802_11ABGN */
	PHY_TYPE_SET_802_11BGN,	/* PHY_CONFIG_802_11BGN */
	PHY_TYPE_SET_802_11AN,	/* PHY_CONFIG_802_11AN */
	PHY_TYPE_SET_802_11GN,	/* PHY_CONFIG_802_11GN */
	PHY_TYPE_SET_802_11AC,
	PHY_TYPE_SET_802_11ANAC,
	PHY_TYPE_SET_802_11ABGNAC,
#if (CFG_SUPPORT_802_11AX == 1)
	PHY_TYPE_SET_802_11ABGNACAX,
#endif
};

/*******************************************************************************
 *                             D A T A   T Y P E S
 *******************************************************************************
 */

/*******************************************************************************
 *                            P U B L I C   D A T A
 *******************************************************************************
 */

static struct INT_EVENT_MAP arIntEventMapTable[] = {
	{WHISR_ABNORMAL_INT | WHISR_WDT_INT, INT_EVENT_ABNORMAL},
	{WHISR_D2H_SW_INT, INT_EVENT_SW_INT},
	{WHISR_TX_DONE_INT, INT_EVENT_TX},
	{(WHISR_RX0_DONE_INT | WHISR_RX1_DONE_INT), INT_EVENT_RX}
};

static const uint8_t ucIntEventMapSize = (sizeof(
			arIntEventMapTable) / sizeof(struct INT_EVENT_MAP));

static IST_EVENT_FUNCTION apfnEventFuncTable[] = {
	nicProcessAbnormalInterrupt,	/*!< INT_EVENT_ABNORMAL */
	nicProcessSoftwareInterrupt,	/*!< INT_EVENT_SW_INT   */
	nicProcessTxInterrupt,	/*!< INT_EVENT_TX       */
	nicProcessRxInterrupt,	/*!< INT_EVENT_RX       */
};

struct ECO_INFO g_eco_info = {0xFF};
/*******************************************************************************
 *                           P R I V A T E   D A T A
 *******************************************************************************
 */
#if defined(_HIF_USB)
struct TIMER rSerSyncTimer = {
	.rLinkEntry = {0}
};
#endif

/*******************************************************************************
 *                                 M A C R O S
 *******************************************************************************
 */
/*! This macro is used to reduce coding errors inside nicAllocateAdapterMemory()
 * and also enhance the readability.
 */
#define LOCAL_NIC_ALLOCATE_MEMORY(pucMem, u4Size, eMemType, pucComment) \
{ \
	pucMem = (uint8_t *)kalMemAlloc(u4Size, eMemType); \
	if (pucMem == (uint8_t *)NULL) { \
		DBGLOG(INIT, ERROR, \
			"Could not allocate %u bytes for %s.\n", \
			u4Size, (char *) pucComment); \
		break; \
	} \
	ASSERT(((unsigned long)pucMem % 4) == 0); \
	DBGLOG(INIT, TRACE, "Alloc %u bytes, addr = 0x%p for %s.\n", \
		u4Size, (void *) pucMem, (char *) pucComment); \
}

/*******************************************************************************
 *                   F U N C T I O N   D E C L A R A T I O N S
 *******************************************************************************
 */

/*******************************************************************************
 *                              F U N C T I O N S
 *******************************************************************************
 */

/*----------------------------------------------------------------------------*/
/*!
 * @brief This routine is responsible for the allocation of the data structures
 *        inside the Adapter structure, include:
 *        1. SW_RFB_Ts
 *        2. Common coalescing buffer for TX PATH.
 *
 * @param prAdapter Pointer of Adapter Data Structure
 *
 * @retval WLAN_STATUS_SUCCESS - Has enough memory.
 * @retval WLAN_STATUS_RESOURCES - Memory is not enough.
 */
/*----------------------------------------------------------------------------*/
uint32_t nicAllocateAdapterMemory(IN struct ADAPTER
				  *prAdapter)
{
	uint32_t status = WLAN_STATUS_RESOURCES;
	struct RX_CTRL *prRxCtrl;
	struct TX_CTRL *prTxCtrl;

	DEBUGFUNC("nicAllocateAdapterMemory");

	ASSERT(prAdapter);
	prRxCtrl = &prAdapter->rRxCtrl;
	prTxCtrl = &prAdapter->rTxCtrl;

	do {
		/* 4 <0> Reset all Memory Handler */
#if CFG_DBG_MGT_BUF
		prAdapter->u4MemFreeDynamicCount = 0;
		prAdapter->u4MemAllocDynamicCount = 0;
#endif
		prAdapter->pucMgtBufCached = (uint8_t *) NULL;
		prRxCtrl->pucRxCached = (uint8_t *) NULL;

		/* 4 <1> Memory for Management Memory Pool and CMD_INFO_T */
		/* Allocate memory for the struct CMD_INFO
		 * and its MGMT memory pool.
		 */
		prAdapter->u4MgtBufCachedSize = MGT_BUFFER_SIZE;
#ifdef CFG_PREALLOC_MEMORY
		prAdapter->pucMgtBufCached = preallocGetMem(MEM_ID_NIC_ADAPTER);
#else
		LOCAL_NIC_ALLOCATE_MEMORY(prAdapter->pucMgtBufCached,
			prAdapter->u4MgtBufCachedSize, PHY_MEM_TYPE,
			"COMMON MGMT MEMORY POOL");
#endif
		/* 4 <2> Memory for RX Descriptor */
		/* Initialize the number of rx buffers
		 * we will have in our queue.
		 */
		/* <TODO> We may setup ucRxPacketDescriptors by GLUE Layer,
		 * and using this variable directly.
		 */
		/* Allocate memory for the SW receive structures. */
		prRxCtrl->u4RxCachedSize = CFG_RX_MAX_PKT_NUM * ALIGN_4(
						   sizeof(struct SW_RFB));

		LOCAL_NIC_ALLOCATE_MEMORY(prRxCtrl->pucRxCached,
			prRxCtrl->u4RxCachedSize,
			VIR_MEM_TYPE, "struct SW_RFB");

		/* 4 <3> Memory for TX DEscriptor */
		prTxCtrl->u4TxCachedSize = CFG_TX_MAX_PKT_NUM * ALIGN_4(
						   sizeof(struct MSDU_INFO));

		LOCAL_NIC_ALLOCATE_MEMORY(prTxCtrl->pucTxCached,
			prTxCtrl->u4TxCachedSize,
			VIR_MEM_TYPE, "struct MSDU_INFO");

		/* 4 <4> Memory for Common Coalescing Buffer */

		/* Get valid buffer size based on config & host capability */
		prAdapter->u4CoalescingBufCachedSize =
			halGetValidCoalescingBufSize(prAdapter);

		/* Allocate memory for the common coalescing buffer. */
#ifdef CFG_PREALLOC_MEMORY
		prAdapter->pucCoalescingBufCached =
			preallocGetMem(MEM_ID_IO_BUFFER);
#else
		prAdapter->pucCoalescingBufCached = kalAllocateIOBuffer(
				prAdapter->u4CoalescingBufCachedSize);
#endif
		if (prAdapter->pucCoalescingBufCached == NULL) {
			DBGLOG(INIT, ERROR,
			       "Could not allocate %u bytes for coalescing buffer.\n",
			       prAdapter->u4CoalescingBufCachedSize);
			break;
		}

		/* <5> Memory for HIF */
		if (halAllocateIOBuffer(prAdapter) != WLAN_STATUS_SUCCESS)
			break;

#if CFG_DBG_MGT_BUF
		LINK_INITIALIZE(&prAdapter->rMemTrackLink);
#endif
		status = WLAN_STATUS_SUCCESS;

	} while (FALSE);

	if (status != WLAN_STATUS_SUCCESS)
		nicReleaseAdapterMemory(prAdapter);

	return status;

}				/* end of nicAllocateAdapterMemory() */

/*----------------------------------------------------------------------------*/
/*!
 * @brief This routine is responsible for releasing the allocated memory by
 *        nicAllocatedAdapterMemory().
 *
 * @param prAdapter Pointer of Adapter Data Structure
 *
 * @return (none)
 */
/*----------------------------------------------------------------------------*/
void nicReleaseAdapterMemory(IN struct ADAPTER *prAdapter)
{
	struct TX_CTRL *prTxCtrl;
	struct RX_CTRL *prRxCtrl;
	uint32_t u4Idx;

	ASSERT(prAdapter);
	prTxCtrl = &prAdapter->rTxCtrl;
	prRxCtrl = &prAdapter->rRxCtrl;

	/* 4 <5> Memory for HIF */
	halReleaseIOBuffer(prAdapter);

	/* 4 <4> Memory for Common Coalescing Buffer */
	if (prAdapter->pucCoalescingBufCached) {
#ifndef CFG_PREALLOC_MEMORY
		kalReleaseIOBuffer((void *)
				   prAdapter->pucCoalescingBufCached,
				   prAdapter->u4CoalescingBufCachedSize);
#endif
		prAdapter->pucCoalescingBufCached = (uint8_t *) NULL;
	}

	/* 4 <3> Memory for TX Descriptor */
	if (prTxCtrl->pucTxCached) {
		kalMemFree((void *) prTxCtrl->pucTxCached, VIR_MEM_TYPE,
			   prTxCtrl->u4TxCachedSize);
		prTxCtrl->pucTxCached = (uint8_t *) NULL;
	}
	/* 4 <2> Memory for RX Descriptor */
	if (prRxCtrl->pucRxCached) {
		kalMemFree((void *) prRxCtrl->pucRxCached, VIR_MEM_TYPE,
			   prRxCtrl->u4RxCachedSize);
		prRxCtrl->pucRxCached = (uint8_t *) NULL;
	}
	/* 4 <1> Memory for Management Memory Pool */
	if (prAdapter->pucMgtBufCached) {
#ifndef CFG_PREALLOC_MEMORY
		kalMemFree((void *) prAdapter->pucMgtBufCached,
			   PHY_MEM_TYPE, prAdapter->u4MgtBufCachedSize);
#endif
		prAdapter->pucMgtBufCached = (uint8_t *) NULL;
	}

	/* Memory for TX Desc Template */
	for (u4Idx = 0; u4Idx < CFG_STA_REC_NUM; u4Idx++)
		nicTxFreeDescTemplate(prAdapter,
				      &prAdapter->arStaRec[u4Idx]);

#if CFG_DBG_MGT_BUF
	do {
		u_int8_t fgUnfreedMem = FALSE;
		struct BUF_INFO *prBufInfo;

		/* Dynamic allocated memory from OS */
		if (prAdapter->u4MemFreeDynamicCount !=
		    prAdapter->u4MemAllocDynamicCount)
			fgUnfreedMem = TRUE;

		/* MSG buffer */
		prBufInfo = &prAdapter->rMsgBufInfo;
		if (prBufInfo->u4AllocCount != (prBufInfo->u4FreeCount +
						prBufInfo->u4AllocNullCount))
			fgUnfreedMem = TRUE;

		/* MGT buffer */
		prBufInfo = &prAdapter->rMgtBufInfo;
		if (prBufInfo->u4AllocCount != (prBufInfo->u4FreeCount +
						prBufInfo->u4AllocNullCount))
			fgUnfreedMem = TRUE;

		/* Check if all allocated memories are free */
		if (fgUnfreedMem) {
			DBGLOG(MEM, ERROR,
				"Unequal memory alloc/free count!\n");

			qmDumpQueueStatus(prAdapter, NULL, 0);
			cnmDumpMemoryStatus(prAdapter, NULL, 0);
		}

		if (!wlanIsChipNoAck(prAdapter)) {
			/* Skip this ASSERT if chip is no ACK */
			if (prAdapter->u4MemFreeDynamicCount !=
					prAdapter->u4MemAllocDynamicCount) {
				struct MEM_TRACK *prMemTrack = NULL;

				DBGLOG(MEM, ERROR, "----- Memory Leak -----\n");
				LINK_FOR_EACH_ENTRY(prMemTrack,
						&prAdapter->rMemTrackLink,
						rLinkEntry,
						struct MEM_TRACK) {
					DBGLOG(MEM, ERROR,
						"file:line %s, cmd id: %u, where: %u\n",
						prMemTrack->pucFileAndLine,
						prMemTrack->u2CmdIdAndWhere &
							0x00FF,
						(prMemTrack->u2CmdIdAndWhere &
							0xFF00) >> 8);
				}
			}
			ASSERT(prAdapter->u4MemFreeDynamicCount ==
			       prAdapter->u4MemAllocDynamicCount);
		}
	} while (FALSE);
#endif

}

/*----------------------------------------------------------------------------*/
/*!
 * @brief disable global interrupt
 *
 * @param prAdapter pointer to the Adapter handler
 *
 * @return (none)
 */
/*----------------------------------------------------------------------------*/
void nicDisableInterrupt(IN struct ADAPTER *prAdapter)
{
	halDisableInterrupt(prAdapter);
}

/*----------------------------------------------------------------------------*/
/*!
 * @brief enable global interrupt
 *
 * @param prAdapter pointer to the Adapter handler
 *
 * @return (none)
 */
/*----------------------------------------------------------------------------*/
void nicEnableInterrupt(IN struct ADAPTER *prAdapter)
{
	halEnableInterrupt(prAdapter);

}				/* end of nicEnableInterrupt() */

#if 0				/* CFG_SDIO_INTR_ENHANCE */
/*----------------------------------------------------------------------------*/
/*!
 * @brief Read interrupt status from hardware
 *
 * @param prAdapter pointer to the Adapter handler
 * @param the interrupts
 *
 * @return N/A
 *
 */
/*----------------------------------------------------------------------------*/
void nicSDIOReadIntStatus(IN struct ADAPTER *prAdapter,
			  OUT uint32_t *pu4IntStatus)
{
	struct ENHANCE_MODE_DATA_STRUCT *prSDIOCtrl;

	DEBUGFUNC("nicSDIOReadIntStatus");

	ASSERT(prAdapter);
	ASSERT(pu4IntStatus);

	prSDIOCtrl = prAdapter->prSDIOCtrl;
	ASSERT(prSDIOCtrl);

	HAL_PORT_RD(prAdapter,
		    MCR_WHISR,
		    sizeof(struct ENHANCE_MODE_DATA_STRUCT),
		    (uint8_t *) prSDIOCtrl,
		    sizeof(struct ENHANCE_MODE_DATA_STRUCT));

	if (kalIsCardRemoved(prAdapter->prGlueInfo) == TRUE
	    || fgIsBusAccessFailed == TRUE) {
		*pu4IntStatus = 0;
		return;
	}

	/* workaround */
	if ((prSDIOCtrl->u4WHISR & WHISR_TX_DONE_INT) == 0
	    && (prSDIOCtrl->rTxInfo.au4WTSR[0]
		| prSDIOCtrl->rTxInfo.au4WTSR[1]
		| prSDIOCtrl->rTxInfo.au4WTSR[2]
		| prSDIOCtrl->rTxInfo.au4WTSR[3]
		| prSDIOCtrl->rTxInfo.au4WTSR[4]
		| prSDIOCtrl->rTxInfo.au4WTSR[5]
		| prSDIOCtrl->rTxInfo.au4WTSR[6]
		| prSDIOCtrl->rTxInfo.au4WTSR[7])) {
		prSDIOCtrl->u4WHISR |= WHISR_TX_DONE_INT;
	}

	if ((prSDIOCtrl->u4WHISR & BIT(31)) == 0 &&
	    HAL_GET_MAILBOX_READ_CLEAR(prAdapter) == TRUE &&
	    (prSDIOCtrl->u4RcvMailbox0 != 0
	     || prSDIOCtrl->u4RcvMailbox1 != 0)) {
		prSDIOCtrl->u4WHISR |= BIT(31);
	}

	*pu4IntStatus = prSDIOCtrl->u4WHISR;

}				/* end of nicSDIOReadIntStatus() */
#endif

#if 0				/*defined(_HIF_PCIE) */
void nicPCIEReadIntStatus(IN struct ADAPTER *prAdapter,
			  OUT uint32_t *pu4IntStatus)
{
	uint32_t u4RegValue;

	*pu4IntStatus = 0;

	HAL_MCR_RD(prAdapter, WPDMA_INT_STA, &u4RegValue);

	if (HAL_IS_RX_DONE_INTR(u4RegValue))
		*pu4IntStatus |= WHISR_RX0_DONE_INT;

	if (HAL_IS_TX_DONE_INTR(u4RegValue))
		*pu4IntStatus |= WHISR_TX_DONE_INT;

	/* clear interrupt */
	HAL_MCR_WR(prAdapter, WPDMA_INT_STA, u4RegValue);

}
#endif

/*----------------------------------------------------------------------------*/
/*!
 * @brief The function used to read interrupt status and then invoking
 *        dispatching procedure for the appropriate functions
 *        corresponding to specific interrupt bits
 *
 * @param prAdapter pointer to the Adapter handler
 *
 * @retval WLAN_STATUS_SUCCESS
 * @retval WLAN_STATUS_ADAPTER_NOT_READY
 */
/*----------------------------------------------------------------------------*/
uint32_t nicProcessIST(IN struct ADAPTER *prAdapter)
{
	uint32_t u4Status = WLAN_STATUS_SUCCESS;
	uint32_t u4IntStatus = 0;
	uint32_t i;

	ASSERT(prAdapter);

	if (prAdapter->rAcpiState == ACPI_STATE_D3) {
		DBGLOG(REQ, WARN,
		       "Fail in set nicProcessIST! (Adapter not ready). ACPI=D%d, Radio=%d\n",
		       prAdapter->rAcpiState, prAdapter->fgIsRadioOff);
		return WLAN_STATUS_ADAPTER_NOT_READY;
	}

	for (i = 0; i < prAdapter->rWifiVar.u4HifIstLoopCount;
	     i++) {

		HAL_READ_INT_STATUS(prAdapter, &u4IntStatus);
		/* DBGLOG(INIT, TRACE, ("u4IntStatus: 0x%x\n", u4IntStatus)); */

		if (u4IntStatus == 0) {
			if (i == 0)
				u4Status = WLAN_STATUS_NOT_INDICATING;
			break;
		}

		nicProcessIST_impl(prAdapter, u4IntStatus);
	}

	return u4Status;
}				/* end of nicProcessIST() */

/*----------------------------------------------------------------------------*/
/*!
 * @brief The function used to dispatch the appropriate functions for specific
 *        interrupt bits
 *
 * @param prAdapter   pointer to the Adapter handler
 *        u4IntStatus interrupt status bits
 *
 * @retval WLAN_STATUS_SUCCESS
 * @retval WLAN_STATUS_ADAPTER_NOT_READY
 */
/*----------------------------------------------------------------------------*/
uint32_t nicProcessIST_impl(IN struct ADAPTER *prAdapter,
			    IN uint32_t u4IntStatus)
{
	uint32_t u4IntCount = 0;
	struct INT_EVENT_MAP *prIntEventMap = NULL;

	ASSERT(prAdapter);

	prAdapter->u4IntStatus = u4IntStatus;

	/* Process each of the interrupt status consequently */
	prIntEventMap = &arIntEventMapTable[0];
	for (u4IntCount = 0; u4IntCount < ucIntEventMapSize;
	     prIntEventMap++, u4IntCount++) {
		if (prIntEventMap->u4Int & prAdapter->u4IntStatus) {
			if (0) {
				/* ignore */
			} else if (apfnEventFuncTable[prIntEventMap->u4Event] !=
				   NULL) {
				apfnEventFuncTable[
				prIntEventMap->u4Event] (prAdapter);
			} else {
				DBGLOG(INTR, WARN,
					"Empty INTR handler! ISAR bit#: %u, event:%u, func: 0x%p\n",
				  prIntEventMap->u4Int,
				  prIntEventMap->u4Event,
				  apfnEventFuncTable[prIntEventMap->u4Event]);

				/* to trap any NULL interrupt handler */
				ASSERT(0);
			}
			prAdapter->u4IntStatus &= ~prIntEventMap->u4Int;
		}
	}

	return WLAN_STATUS_SUCCESS;
}				/* end of nicProcessIST_impl() */

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
u_int8_t nicVerifyChipID(IN struct ADAPTER *prAdapter)
{
	return halVerifyChipID(prAdapter);
}

/*----------------------------------------------------------------------------*/
/*!
 * @brief Initialize the MCR to the appropriate init value, and verify the init
 *        value
 *
 * @param prAdapter      a pointer to adapter private data structure.
 *
 * @return -
 */
/*----------------------------------------------------------------------------*/
void nicMCRInit(IN struct ADAPTER *prAdapter)
{

	ASSERT(prAdapter);

	/* 4 <0> Initial value */
}

void nicHifInit(IN struct ADAPTER *prAdapter)
{

	ASSERT(prAdapter);
#if 0
	/* reset event */
	nicPutMailbox(prAdapter, 0, 0x52455345);	/* RESE */
	nicPutMailbox(prAdapter, 1, 0x545F5746);	/* T_WF */
	nicSetSwIntr(prAdapter, BIT(16));
#endif
}

/*----------------------------------------------------------------------------*/
/*!
 * @brief Initialize the Adapter soft variable
 *
 * @param prAdapter pointer to the Adapter handler
 *
 * @return (none)
 *
 */
/*----------------------------------------------------------------------------*/
uint32_t nicInitializeAdapter(IN struct ADAPTER *prAdapter)
{
	uint32_t u4Status = WLAN_STATUS_SUCCESS;

	ASSERT(prAdapter);

	prAdapter->fgIsIntEnableWithLPOwnSet = FALSE;
	prAdapter->fgIsReadRevID = FALSE;

	do {
		if (!nicVerifyChipID(prAdapter)) {
			u4Status = WLAN_STATUS_FAILURE;
			break;
		}
		/* 4 <1> MCR init */
		nicMCRInit(prAdapter);

		HAL_HIF_INIT(prAdapter);

		/* 4 <2> init FW HIF */
		nicHifInit(prAdapter);
	} while (FALSE);

	return u4Status;
}

#if defined(_HIF_SPI)
/*----------------------------------------------------------------------------*/
/*!
 * \brief Restore the SPI Mode Select to default mode,
 *        this is important while driver is unload, and this must be last mcr
 *        since the operation will let the hif use 8bit mode access
 *
 * \param[in] prAdapter      a pointer to adapter private data structure.
 * \param[in] eGPIO2_Mode    GPIO2 operation mode
 *
 * \return (none)
 */
/*----------------------------------------------------------------------------*/
void nicRestoreSpiDefMode(IN struct ADAPTER *prAdapter)
{
	ASSERT(prAdapter);

	HAL_MCR_WR(prAdapter, MCR_WCSR, SPICSR_8BIT_MODE_DATA);

}
#endif

/*----------------------------------------------------------------------------*/
/*!
 * @brief Process rx interrupt. When the rx
 *        Interrupt is asserted, it means there are frames in queue.
 *
 * @param prAdapter      Pointer to the Adapter structure.
 *
 * @return (none)
 */
/*----------------------------------------------------------------------------*/
void nicProcessAbnormalInterrupt(IN struct ADAPTER
				 *prAdapter)
{
	if (halIsHifStateSuspend(prAdapter))
		DBGLOG(RX, WARN, "suspend Abnormal\n");

	prAdapter->prGlueInfo->IsrAbnormalCnt++;

	halProcessAbnormalInterrupt(prAdapter);
}

/*----------------------------------------------------------------------------*/
/*!
 * @brief .
 *
 * @param prAdapter  Pointer to the Adapter structure.
 *
 * @return (none)
 */
/*----------------------------------------------------------------------------*/
void nicProcessFwOwnBackInterrupt(IN struct ADAPTER
				  *prAdapter)
{

}				/* end of nicProcessFwOwnBackInterrupt() */

/*----------------------------------------------------------------------------*/
/*!
 * @brief .
 *
 * @param prAdapter  Pointer to the Adapter structure.
 *
 * @return (none)
 */
/*----------------------------------------------------------------------------*/
void nicProcessSoftwareInterrupt(IN struct ADAPTER
				 *prAdapter)
{
	prAdapter->prGlueInfo->IsrSoftWareCnt++;
	if (halIsHifStateSuspend(prAdapter))
		DBGLOG(RX, WARN, "suspend SW INT\n");
	halProcessSoftwareInterrupt(prAdapter);
}				/* end of nicProcessSoftwareInterrupt() */

void nicSetSwIntr(IN struct ADAPTER *prAdapter,
		  IN uint32_t u4SwIntrBitmap)
{
	/* NOTE:
	 *  SW interrupt in HW bit 16 is mapping to SW bit 0
	 *  (shift 16bit in HW transparancy)
	 *  SW interrupt valid from b0~b15
	 */
	ASSERT((u4SwIntrBitmap & BITS(0, 15)) == 0);
	/* DBGLOG(INIT, TRACE, ("u4SwIntrBitmap: 0x%08x\n", u4SwIntrBitmap)); */

	HAL_MCR_WR(prAdapter, MCR_WSICR, u4SwIntrBitmap);
}

/*----------------------------------------------------------------------------*/
/*!
 * @brief This procedure is used to dequeue from prAdapter->rPendingCmdQueue
 *        with specified sequential number
 *
 * @param    prAdapter   Pointer of ADAPTER_T
 *           ucSeqNum    Sequential Number
 *
 * @retval - P_CMD_INFO_T
 */
/*----------------------------------------------------------------------------*/
struct CMD_INFO *nicGetPendingCmdInfo(IN struct ADAPTER
				      *prAdapter, IN uint8_t ucSeqNum)
{
	struct QUE *prCmdQue;
	struct QUE rTempCmdQue;
	struct QUE *prTempCmdQue = &rTempCmdQue;
	struct QUE_ENTRY *prQueueEntry = (struct QUE_ENTRY *) NULL;
	struct CMD_INFO *prCmdInfo = (struct CMD_INFO *) NULL;

	KAL_SPIN_LOCK_DECLARATION();

	ASSERT(prAdapter);

	KAL_ACQUIRE_SPIN_LOCK(prAdapter, SPIN_LOCK_CMD_PENDING);

	prCmdQue = &prAdapter->rPendingCmdQueue;
	QUEUE_MOVE_ALL(prTempCmdQue, prCmdQue);

	QUEUE_REMOVE_HEAD(prTempCmdQue, prQueueEntry,
			  struct QUE_ENTRY *);
	while (prQueueEntry) {
		prCmdInfo = (struct CMD_INFO *) prQueueEntry;

		if (prCmdInfo->ucCmdSeqNum == ucSeqNum)
			break;

		QUEUE_INSERT_TAIL(prCmdQue, prQueueEntry);

		prCmdInfo = NULL;

		QUEUE_REMOVE_HEAD(prTempCmdQue, prQueueEntry,
				  struct QUE_ENTRY *);
	}
	QUEUE_CONCATENATE_QUEUES(prCmdQue, prTempCmdQue);

	KAL_RELEASE_SPIN_LOCK(prAdapter, SPIN_LOCK_CMD_PENDING);

	return prCmdInfo;
}

/*----------------------------------------------------------------------------*/
/*!
 * @brief This procedure is used to dequeue from
 *        prAdapter->rTxCtrl.rTxMgmtTxingQueue
 *        with specified sequential number
 *
 * @param    prAdapter   Pointer of ADAPTER_T
 *           ucSeqNum    Sequential Number
 *
 * @retval - P_MSDU_INFO_T
 */
/*----------------------------------------------------------------------------*/
struct MSDU_INFO *nicGetPendingTxMsduInfo(
	IN struct ADAPTER *prAdapter, IN uint8_t ucWlanIndex,
	IN uint8_t ucPID)
{
	struct QUE *prTxingQue;
	struct QUE rTempQue;
	struct QUE *prTempQue = &rTempQue;
	struct QUE_ENTRY *prQueueEntry = (struct QUE_ENTRY *) NULL;
	struct MSDU_INFO *prMsduInfo = (struct MSDU_INFO *) NULL;

	KAL_SPIN_LOCK_DECLARATION();

	ASSERT(prAdapter);

	KAL_ACQUIRE_SPIN_LOCK(prAdapter, SPIN_LOCK_TXING_MGMT_LIST);

	prTxingQue = &(prAdapter->rTxCtrl.rTxMgmtTxingQueue);
	QUEUE_MOVE_ALL(prTempQue, prTxingQue);

	QUEUE_REMOVE_HEAD(prTempQue, prQueueEntry,
			  struct QUE_ENTRY *);
	while (prQueueEntry) {
		prMsduInfo = (struct MSDU_INFO *) prQueueEntry;

		if ((prMsduInfo->ucPID == ucPID)
		    && (prMsduInfo->ucWlanIndex == ucWlanIndex))
			break;

		QUEUE_INSERT_TAIL(prTxingQue, prQueueEntry);

		prMsduInfo = NULL;

		QUEUE_REMOVE_HEAD(prTempQue, prQueueEntry,
				  struct QUE_ENTRY *);
	}
	QUEUE_CONCATENATE_QUEUES(prTxingQue, prTempQue);

	KAL_RELEASE_SPIN_LOCK(prAdapter, SPIN_LOCK_TXING_MGMT_LIST);

	if (prMsduInfo) {
		DBGLOG(TX, TRACE,
		       "Get Msdu WIDX:PID[%u:%u] SEQ[%u] from Pending Q\n",
		       prMsduInfo->ucWlanIndex, prMsduInfo->ucPID,
		       prMsduInfo->ucTxSeqNum);
	} else {
		DBGLOG(TX, WARN,
		       "Cannot get Target Msdu WIDX:PID[%u:%u] from Pending Q\n",
		       ucWlanIndex, ucPID);
	}

	return prMsduInfo;
}

void nicFreePendingTxMsduInfo(IN struct ADAPTER *prAdapter,
	IN uint8_t ucIndex, IN enum ENUM_REMOVE_BY_MSDU_TPYE ucFreeType)
{
	struct QUE *prTxingQue;
	struct QUE rTempQue;
	struct QUE *prTempQue = &rTempQue;
	struct QUE_ENTRY *prQueueEntry = (struct QUE_ENTRY *) NULL;
	struct MSDU_INFO *prMsduInfoListHead = (struct MSDU_INFO *)
					       NULL;
	struct MSDU_INFO *prMsduInfoListTail = (struct MSDU_INFO *)
					       NULL;
	struct MSDU_INFO *prMsduInfo = (struct MSDU_INFO *) NULL;
	uint8_t ucRemoveByIndex = 255;

	KAL_SPIN_LOCK_DECLARATION();

	ASSERT(prAdapter);

	if (ucFreeType >= ENUM_REMOVE_BY_MSDU_TPYE_NUM) {
		DBGLOG(TX, WARN, "Wrong remove type: %d\n", ucFreeType);
		return;
	}

	KAL_ACQUIRE_SPIN_LOCK(prAdapter, SPIN_LOCK_TXING_MGMT_LIST);

	prTxingQue = &(prAdapter->rTxCtrl.rTxMgmtTxingQueue);
	QUEUE_MOVE_ALL(prTempQue, prTxingQue);

	QUEUE_REMOVE_HEAD(prTempQue, prQueueEntry,
			  struct QUE_ENTRY *);

	while (prQueueEntry) {
		prMsduInfo = (struct MSDU_INFO *) prQueueEntry;

		switch (ucFreeType) {
		case MSDU_REMOVE_BY_WLAN_INDEX:
			ucRemoveByIndex = prMsduInfo->ucWlanIndex;
			break;
		case MSDU_REMOVE_BY_BSS_INDEX:
			ucRemoveByIndex = prMsduInfo->ucBssIndex;
			break;
		default:
			break;
		}
		if (ucRemoveByIndex == ucIndex) {
			DBGLOG(TX, TRACE,
			       "%s: Get Msdu WIDX:PID[%u:%u] SEQ[%u] from Pending Q\n",
			       __func__, prMsduInfo->ucWlanIndex,
			       prMsduInfo->ucPID,
			       prMsduInfo->ucTxSeqNum);

			if (prMsduInfoListHead == NULL) {
				prMsduInfoListHead =
					prMsduInfoListTail = prMsduInfo;
			} else {
				QM_TX_SET_NEXT_MSDU_INFO(
					prMsduInfoListTail, prMsduInfo);
				prMsduInfoListTail = prMsduInfo;
			}
		} else {
			QUEUE_INSERT_TAIL(prTxingQue, prQueueEntry);

			prMsduInfo = NULL;
		}

		QUEUE_REMOVE_HEAD(prTempQue, prQueueEntry,
				  struct QUE_ENTRY *);
	}
	QUEUE_CONCATENATE_QUEUES(prTxingQue, prTempQue);

	KAL_RELEASE_SPIN_LOCK(prAdapter, SPIN_LOCK_TXING_MGMT_LIST);

	/* free */
	if (prMsduInfoListHead) {
		nicTxFreeMsduInfoPacket(prAdapter, prMsduInfoListHead);
		nicTxReturnMsduInfo(prAdapter, prMsduInfoListHead);
	}
}

/*----------------------------------------------------------------------------*/
/*!
 * @brief This procedure is used to retrieve a CMD sequence number atomically
 *
 * @param    prAdapter   Pointer of ADAPTER_T
 *
 * @retval - UINT_8
 */
/*----------------------------------------------------------------------------*/
uint8_t nicIncreaseCmdSeqNum(IN struct ADAPTER *prAdapter)
{
	uint8_t ucRetval;

	KAL_SPIN_LOCK_DECLARATION();

	ASSERT(prAdapter);

	KAL_ACQUIRE_SPIN_LOCK(prAdapter, SPIN_LOCK_CMD_SEQ_NUM);

	prAdapter->ucCmdSeqNum++;
	ucRetval = prAdapter->ucCmdSeqNum;

	KAL_RELEASE_SPIN_LOCK(prAdapter, SPIN_LOCK_CMD_SEQ_NUM);

	return ucRetval;
}

/*----------------------------------------------------------------------------*/
/*!
 * @brief This procedure is used to retrieve a TX sequence number atomically
 *
 * @param    prAdapter   Pointer of ADAPTER_T
 *
 * @retval - UINT_8
 */
/*----------------------------------------------------------------------------*/
uint8_t nicIncreaseTxSeqNum(IN struct ADAPTER *prAdapter)
{
	uint8_t ucRetval;

	KAL_SPIN_LOCK_DECLARATION();

	ASSERT(prAdapter);

	KAL_ACQUIRE_SPIN_LOCK(prAdapter, SPIN_LOCK_TX_SEQ_NUM);

	ucRetval = prAdapter->ucTxSeqNum;

	prAdapter->ucTxSeqNum++;

	KAL_RELEASE_SPIN_LOCK(prAdapter, SPIN_LOCK_TX_SEQ_NUM);

	return ucRetval;
}

/*----------------------------------------------------------------------------*/
/*!
 * @brief This utility function is used to handle
 *        media state change event
 *
 * @param
 *
 * @retval
 */
/*----------------------------------------------------------------------------*/
uint32_t
nicMediaStateChange(IN struct ADAPTER *prAdapter,
		    IN uint8_t ucBssIndex,
		    IN struct EVENT_CONNECTION_STATUS *prConnectionStatus)
{
	struct GLUE_INFO *prGlueInfo;
	struct AIS_FSM_INFO *prAisFsmInfo;

	ASSERT(prAdapter);
	prGlueInfo = prAdapter->prGlueInfo;

	switch (GET_BSS_INFO_BY_INDEX(prAdapter,
				      ucBssIndex)->eNetworkType) {
	case NETWORK_TYPE_AIS:
		prAisFsmInfo = aisGetAisFsmInfo(prAdapter, ucBssIndex);
		if (prConnectionStatus->ucMediaStatus ==
		    MEDIA_STATE_DISCONNECTED) {	/* disconnected */
			if (kalGetMediaStateIndicated(prGlueInfo,
				ucBssIndex) !=
			    MEDIA_STATE_DISCONNECTED ||
			    prAisFsmInfo->eCurrentState == AIS_STATE_JOIN) {

				kalIndicateStatusAndComplete(prGlueInfo,
					WLAN_STATUS_MEDIA_DISCONNECT, NULL,
					0, ucBssIndex);

				prAdapter->rWlanInfo.u4SysTime =
					kalGetTimeTick();
			}

			/* reset buffered link quality information */
			prAdapter->fgIsLinkQualityValid = FALSE;
			prAdapter->fgIsLinkRateValid = FALSE;
		} else if (prConnectionStatus->ucMediaStatus ==
			   MEDIA_STATE_CONNECTED) {	/* connected */
			struct PARAM_BSSID_EX *prCurrBssid =
				aisGetCurrBssId(prAdapter, ucBssIndex);

			prAdapter->rWlanInfo.u4SysTime = kalGetTimeTick();

			/* fill information for association result */
			prCurrBssid->rSsid.u4SsidLen =
				prConnectionStatus->ucSsidLen;
			kalMemCopy(
				prCurrBssid->rSsid.aucSsid,
				prConnectionStatus->aucSsid,
				prConnectionStatus->ucSsidLen);
			kalMemCopy(prCurrBssid->arMacAddress,
				   prConnectionStatus->aucBssid, MAC_ADDR_LEN);
			prCurrBssid->u4Privacy =
				prConnectionStatus->ucEncryptStatus;/* @FIXME */
			prCurrBssid->rRssi = 0; /* @FIXME */
			prCurrBssid->eNetworkTypeInUse =
				PARAM_NETWORK_TYPE_AUTOMODE;/* @FIXME */
			prCurrBssid->
			rConfiguration.u4BeaconPeriod
				= prConnectionStatus->u2BeaconPeriod;
			prCurrBssid->
			rConfiguration.u4ATIMWindow
				= prConnectionStatus->u2ATIMWindow;
			prCurrBssid->
				rConfiguration.u4DSConfig =
				prConnectionStatus->u4FreqInKHz;
			prAdapter->rWlanInfo.ucNetworkType[ucBssIndex] =
				prConnectionStatus->ucNetworkType;
			prCurrBssid->eOpMode
				= (enum ENUM_PARAM_OP_MODE)
					prConnectionStatus->ucInfraMode;

			/* always indicate to OS according to
			 * MSDN (re-association/roaming)
			 */
			if (kalGetMediaStateIndicated(prGlueInfo,
				ucBssIndex) !=
			    MEDIA_STATE_CONNECTED) {
				kalIndicateStatusAndComplete(prGlueInfo,
					WLAN_STATUS_MEDIA_CONNECT, NULL,
					0, ucBssIndex);
			} else {
				/* connected -> connected : roaming ? */
				kalIndicateStatusAndComplete(prGlueInfo,
					WLAN_STATUS_ROAM_OUT_FIND_BEST,
					NULL,
					0, ucBssIndex);
			}
		}
		break;

#if CFG_ENABLE_BT_OVER_WIFI
	case NETWORK_TYPE_BOW:
		break;
#endif

#if CFG_ENABLE_WIFI_DIRECT
	case NETWORK_TYPE_P2P:
		break;
#endif
	default:
		ASSERT(0);
	}

	return WLAN_STATUS_SUCCESS;
}				/* nicMediaStateChange */

/*----------------------------------------------------------------------------*/
/*!
 * @brief This utility function is used to generate a join failure event to OS
 *
 * @param
 *
 * @retval
 */
/*----------------------------------------------------------------------------*/
uint32_t nicMediaJoinFailure(IN struct ADAPTER *prAdapter,
			     IN uint8_t ucBssIndex, IN uint32_t rStatus)
{
	struct GLUE_INFO *prGlueInfo;

	ASSERT(prAdapter);
	prGlueInfo = prAdapter->prGlueInfo;

	switch (GET_BSS_INFO_BY_INDEX(prAdapter,
				      ucBssIndex)->eNetworkType) {
	case NETWORK_TYPE_AIS:
		kalIndicateStatusAndComplete(prGlueInfo, rStatus, NULL, 0,
			ucBssIndex);

		break;

	case NETWORK_TYPE_BOW:
	case NETWORK_TYPE_P2P:
	default:
		break;
	}

	return WLAN_STATUS_SUCCESS;
}				/* end of nicMediaJoinFailure() */

/*----------------------------------------------------------------------------*/
/*!
 * @brief This utility function is used to convert between
 *        frequency and channel number
 *
 * @param u4ChannelNum
 *
 * @retval - Frequency in unit of KHz, 0 for invalid channel number
 */
/*----------------------------------------------------------------------------*/
uint32_t nicChannelNum2Freq(uint32_t u4ChannelNum, enum ENUM_BAND eBand)
{
	uint32_t u4ChannelInMHz;

#if (CFG_SUPPORT_WIFI_6G == 1)
	if (eBand == BAND_6G)
		u4ChannelInMHz = 5950 + u4ChannelNum * 5;
	else if (u4ChannelNum >= 1 && u4ChannelNum <= 13)
#else
	if (u4ChannelNum >= 1 && u4ChannelNum <= 13)
#endif
		u4ChannelInMHz = 2412 + (u4ChannelNum - 1) * 5;
	else if (u4ChannelNum == 14)
		u4ChannelInMHz = 2484;
	else if (u4ChannelNum == 133)
		u4ChannelInMHz = 3665;	/* 802.11y */
	else if (u4ChannelNum == 137)
		u4ChannelInMHz = 3685;	/* 802.11y */
	else if ((u4ChannelNum >= 34 && u4ChannelNum <= 181)
		 || (u4ChannelNum == 16))
		u4ChannelInMHz = 5000 + u4ChannelNum * 5;
	else if (u4ChannelNum >= 182 && u4ChannelNum <= 196)
		u4ChannelInMHz = 4000 + u4ChannelNum * 5;
	else if (u4ChannelNum == 201)
		u4ChannelInMHz = 2730;
	else if (u4ChannelNum == 202)
		u4ChannelInMHz = 2498;
	else
		u4ChannelInMHz = 0;

	return 1000 * u4ChannelInMHz;
}

/*----------------------------------------------------------------------------*/
/*!
 * @brief This utility function is used to convert between
 *        frequency and channel number
 *
 * @param u4FreqInKHz
 *
 * @retval - Frequency Number, 0 for invalid freqency
 */
/*----------------------------------------------------------------------------*/
uint32_t nicFreq2ChannelNum(uint32_t u4FreqInKHz)
{
#if (CFG_SUPPORT_WIFI_6G == 1)
	uint32_t u4FreqInMHz = 0;
#endif

	switch (u4FreqInKHz) {
	case 2412000:
		return 1;
	case 2417000:
		return 2;
	case 2422000:
		return 3;
	case 2427000:
		return 4;
	case 2432000:
		return 5;
	case 2437000:
		return 6;
	case 2442000:
		return 7;
	case 2447000:
		return 8;
	case 2452000:
		return 9;
	case 2457000:
		return 10;
	case 2462000:
		return 11;
	case 2467000:
		return 12;
	case 2472000:
		return 13;
	case 2484000:
		return 14;
	case 3665000:
		return 133;	/* 802.11y */
	case 3685000:
		return 137;	/* 802.11y */
	case 4915000:
		return 183;
	case 4920000:
		return 184;
	case 4925000:
		return 185;
	case 4930000:
		return 186;
	case 4935000:
		return 187;
	case 4940000:
		return 188;
	case 4945000:
		return 189;
	case 4960000:
		return 192;
	case 4980000:
		return 196;
	case 5170000:
		return 34;
	case 5180000:
		return 36;
	case 5190000:
		return 38;
	case 5200000:
		return 40;
	case 5210000:
		return 42;
	case 5220000:
		return 44;
	case 5230000:
		return 46;
	case 5240000:
		return 48;
	case 5250000:
		return 50;
	case 5260000:
		return 52;
	case 5270000:
		return 54;
	case 5280000:
		return 56;
	case 5290000:
		return 58;
	case 5300000:
		return 60;
	case 5310000:
		return 62;
	case 5320000:
		return 64;
	case 5500000:
		return 100;
	case 5510000:
		return 102;
	case 5520000:
		return 104;
	case 5530000:
		return 106;
	case 5540000:
		return 108;
	case 5550000:
		return 110;
	case 5560000:
		return 112;
	case 5570000:
		return 114;
	case 5580000:
		return 116;
	case 5590000:
		return 118;
	case 5600000:
		return 120;
	case 5610000:
		return 122;
	case 5620000:
		return 124;
	case 5630000:
		return 126;
	case 5640000:
		return 128;
	case 5660000:
		return 132;
	case 5670000:
		return 134;
	case 5680000:
		return 136;
	case 5690000:
		return 138;
	case 5700000:
		return 140;
	case 5710000:
		return 142;
	case 5720000:
		return 144;
	case 5745000:
		return 149;
	case 5755000:
		return 151;
	case 5765000:
		return 153;
	case 5775000:
		return 155;
	case 5785000:
		return 157;
	case 5795000:
		return 159;
	case 5805000:
		return 161;
	case 5825000:
		return 165;
	case 5845000:
		return 169;
	case 5865000:
		return 173;
	default:
#if (CFG_SUPPORT_WIFI_6G == 1)
		if (u4FreqInKHz % 5000 == 0) {
			u4FreqInMHz = u4FreqInKHz/1000;
			if ((u4FreqInMHz > 5950) && (u4FreqInMHz <= 7115))
				return ((u4FreqInMHz - 5950) / 5);
		}
#endif
		DBGLOG(BSS, INFO, "Return Invalid Channelnum = 0.\n");
		return 0;
	}
}

uint8_t nicGetVhtS1(uint8_t ucPrimaryChannel,
		    uint8_t ucBandwidth)
{
	/* find S1 (central channel 42, 58, 106, 122, and 155) */

	if ((ucBandwidth == VHT_OP_CHANNEL_WIDTH_80)
	    || (ucBandwidth == VHT_OP_CHANNEL_WIDTH_80P80)) {

		if (ucPrimaryChannel >= 36 && ucPrimaryChannel <= 48)
			return 42;
		else if (ucPrimaryChannel >= 52 && ucPrimaryChannel <= 64)
			return 58;
		else if (ucPrimaryChannel >= 100 && ucPrimaryChannel <= 112)
			return 106;
		else if (ucPrimaryChannel >= 116 && ucPrimaryChannel <= 128)
			return 122;
		else if (ucPrimaryChannel >= 132 && ucPrimaryChannel <= 144)
			return 138;
		else if (ucPrimaryChannel >= 149 && ucPrimaryChannel <= 161)
			return 155;

	} else if (ucBandwidth == VHT_OP_CHANNEL_WIDTH_160) {

		if (ucPrimaryChannel >= 36 && ucPrimaryChannel <= 64)
			return 50;
		else if (ucPrimaryChannel >= 100 && ucPrimaryChannel <= 128)
			return 114;

	} else {

		return 0;
	}

	return 0;

}

#if (CFG_SUPPORT_WIFI_6G == 1)
uint8_t nicGetHe6gS1(uint8_t ucPrimaryChannel,
		    uint8_t ucBandwidth)
{
	if ((ucBandwidth == CW_80MHZ)
	    || (ucBandwidth == CW_80P80MHZ)) {

		if (ucPrimaryChannel >= 1 && ucPrimaryChannel <= 13)
			return 7;
		else if (ucPrimaryChannel >= 17 && ucPrimaryChannel <= 29)
			return 23;
		else if (ucPrimaryChannel >= 33 && ucPrimaryChannel <= 45)
			return 39;
		else if (ucPrimaryChannel >= 49 && ucPrimaryChannel <= 61)
			return 55;
		else if (ucPrimaryChannel >= 65 && ucPrimaryChannel <= 77)
			return 71;
		else if (ucPrimaryChannel >= 81 && ucPrimaryChannel <= 93)
			return 87;
		else if (ucPrimaryChannel >= 97 && ucPrimaryChannel <= 109)
			return 103;
		else if (ucPrimaryChannel >= 113 && ucPrimaryChannel <= 125)
			return 119;
		else if (ucPrimaryChannel >= 129 && ucPrimaryChannel <= 141)
			return 135;
		else if (ucPrimaryChannel >= 145 && ucPrimaryChannel <= 157)
			return 151;
		else if (ucPrimaryChannel >= 161 && ucPrimaryChannel <= 173)
			return 167;
		else if (ucPrimaryChannel >= 177 && ucPrimaryChannel <= 189)
			return 183;
		else if (ucPrimaryChannel >= 193 && ucPrimaryChannel <= 205)
			return 199;
		else if (ucPrimaryChannel >= 209 && ucPrimaryChannel <= 221)
			return 215;
		else if (ucPrimaryChannel >= 225 && ucPrimaryChannel <= 237)
			return 231;
		else if (ucPrimaryChannel >= 241 && ucPrimaryChannel <= 253)
			return 249;
	} else if (ucBandwidth == CW_160MHZ) {

		if (ucPrimaryChannel >= 1 && ucPrimaryChannel <= 29)
			return 15;
		else if (ucPrimaryChannel >= 33 && ucPrimaryChannel <= 61)
			return 47;
		else if (ucPrimaryChannel >= 65 && ucPrimaryChannel <= 93)
			return 79;
		else if (ucPrimaryChannel >= 97 && ucPrimaryChannel <= 125)
			return 111;
		else if (ucPrimaryChannel >= 129 && ucPrimaryChannel <= 157)
			return 143;
		else if (ucPrimaryChannel >= 161 && ucPrimaryChannel <= 189)
			return 175;
		else if (ucPrimaryChannel >= 193 && ucPrimaryChannel <= 221)
			return 207;
	} else {

		return 0;
	}
	return 0;
}
#endif

/* firmware command wrapper */
/* NETWORK (WIFISYS) */
/*----------------------------------------------------------------------------*/
/*!
 * @brief This utility function is used to activate WIFISYS for specified
 *        network
 *
 * @param prAdapter          Pointer of ADAPTER_T
 *        eNetworkTypeIdx    Index of network type
 *
 * @retval -
 */
/*----------------------------------------------------------------------------*/
uint32_t nicActivateNetwork(IN struct ADAPTER *prAdapter,
			    IN uint8_t ucBssIndex)
{
	struct CMD_BSS_ACTIVATE_CTRL rCmdActivateCtrl;
	struct BSS_INFO *prBssInfo;
	/*	const UINT_8 aucZeroMacAddr[] = NULL_MAC_ADDR; */

	ASSERT(prAdapter);
	ASSERT(ucBssIndex <= prAdapter->ucHwBssIdNum);

	/* Enable tx hang detect */
	prAdapter->u4TxHangFlag |= BIT(ucBssIndex);

	prBssInfo = GET_BSS_INFO_BY_INDEX(prAdapter, ucBssIndex);

	prBssInfo->fg40mBwAllowed = FALSE;
	prBssInfo->fgAssoc40mBwAllowed = FALSE;

	rCmdActivateCtrl.ucBssIndex = ucBssIndex;
	rCmdActivateCtrl.ucActive = 1;
	rCmdActivateCtrl.ucNetworkType = (uint8_t)
					 prBssInfo->eNetworkType;
	rCmdActivateCtrl.ucOwnMacAddrIndex =
		prBssInfo->ucOwnMacIndex;
	COPY_MAC_ADDR(rCmdActivateCtrl.aucBssMacAddr,
		      prBssInfo->aucOwnMacAddr);

	prBssInfo->ucBMCWlanIndex =
		secPrivacySeekForBcEntry(prAdapter, prBssInfo->ucBssIndex,
					 prBssInfo->aucOwnMacAddr,
					 STA_REC_INDEX_NOT_FOUND,
					 CIPHER_SUITE_NONE, 0xFF);
	rCmdActivateCtrl.ucBMCWlanIndex = prBssInfo->ucBMCWlanIndex;

	kalMemZero(&rCmdActivateCtrl.ucReserved,
		   sizeof(rCmdActivateCtrl.ucReserved));

#if 1				/* DBG */
	DBGLOG(RSN, INFO,
	       "[wlan index]=%d OwnMac%d=" MACSTR " BSSID=" MACSTR
	       " BMCIndex = %d NetType=%d\n",
	       ucBssIndex,
	       prBssInfo->ucOwnMacIndex,
	       MAC2STR(prBssInfo->aucOwnMacAddr),
	       MAC2STR(prBssInfo->aucBSSID),
	       prBssInfo->ucBMCWlanIndex, prBssInfo->eNetworkType);
#endif

	/* Free the pending msdu in rTxMgmtTxingQueue.
	 * Move this action from "deactive" to "active" to avoid the KE issue.
	 * Deactive remove pending msdu but HIF thread use it after deactive.
	 */
#if (CFG_NOT_CLR_FREE_MSDU_IN_ACTIVE_NETWORK == 0)
	nicFreePendingTxMsduInfo(prAdapter, ucBssIndex,
		MSDU_REMOVE_BY_BSS_INDEX);
#endif
	kalClearSecurityFramesByBssIdx(prAdapter->prGlueInfo,
				       ucBssIndex);

	return wlanSendSetQueryCmd(prAdapter,
				   CMD_ID_BSS_ACTIVATE_CTRL,
				   TRUE,
				   FALSE,
				   FALSE,
				   NULL, NULL,
				   sizeof(struct CMD_BSS_ACTIVATE_CTRL),
				   (uint8_t *)&rCmdActivateCtrl, NULL, 0);
}

/*----------------------------------------------------------------------------*/
/*!
 * @brief This utility function is used to deactivate WIFISYS for specified
 *        network
 *
 * @param prAdapter          Pointer of ADAPTER_T
 *        eNetworkTypeIdx    Index of network type
 *
 * @retval -
 */
/*----------------------------------------------------------------------------*/
uint32_t nicDeactivateNetwork(IN struct ADAPTER *prAdapter,
			      IN uint8_t ucBssIndex)
{
	uint32_t u4Status;
	struct CMD_BSS_ACTIVATE_CTRL rCmdActivateCtrl;
	struct BSS_INFO *prBssInfo;

	ASSERT(prAdapter);
	ASSERT(ucBssIndex <= prAdapter->ucHwBssIdNum);

	/* Disable tx hang detect */
	prAdapter->u4TxHangFlag &= ~BIT(ucBssIndex);

	prBssInfo = GET_BSS_INFO_BY_INDEX(prAdapter, ucBssIndex);

	/* FW only supports BMCWlan index 0 ~ 31.
	 * it always checks BMCWlan index validity and triggers
	 * assertion if BMCWlan index is invalid.
	 */
	if (prBssInfo->ucBMCWlanIndex == WTBL_RESERVED_ENTRY) {
		DBGLOG(RSN, WARN,
		       "Network may be deactivated already, ignore\n");
		return WLAN_STATUS_NOT_ACCEPTED;
	}

	kalMemZero(&rCmdActivateCtrl,
		   sizeof(struct CMD_BSS_ACTIVATE_CTRL));

	rCmdActivateCtrl.ucBssIndex = ucBssIndex;
	rCmdActivateCtrl.ucActive = 0;
	rCmdActivateCtrl.ucNetworkType =
		(uint8_t)prBssInfo->eNetworkType;
	rCmdActivateCtrl.ucOwnMacAddrIndex =
		prBssInfo->ucOwnMacIndex;
	rCmdActivateCtrl.ucBMCWlanIndex =
		prBssInfo->ucBMCWlanIndex;

	DBGLOG(RSN, INFO,
	       "[wlan index]=%d OwnMac=" MACSTR " BSSID=" MACSTR
	       " BMCIndex = %d NetType=%d\n",
	       ucBssIndex,
	       MAC2STR(prBssInfo->aucOwnMacAddr),
	       MAC2STR(prBssInfo->aucBSSID),
	       prBssInfo->ucBMCWlanIndex, prBssInfo->eNetworkType);

	u4Status = wlanSendSetQueryCmd(prAdapter,
				       CMD_ID_BSS_ACTIVATE_CTRL,
				       TRUE,
				       FALSE,
				       FALSE,
				       NULL,
				       NULL,
				       sizeof(struct CMD_BSS_ACTIVATE_CTRL),
				       (uint8_t *)&rCmdActivateCtrl, NULL, 0);

	secRemoveBssBcEntry(prAdapter, prBssInfo, FALSE);

	/* free all correlated station records */
	cnmStaFreeAllStaByNetwork(prAdapter, ucBssIndex,
				  STA_REC_EXCLUDE_NONE);
	if (HAL_IS_TX_DIRECT(prAdapter))
		nicTxDirectClearBssAbsentQ(prAdapter, ucBssIndex);
	else
		qmFreeAllByBssIdx(prAdapter, ucBssIndex);

	cnmFreeWmmIndex(prAdapter, prBssInfo);
	return u4Status;
}

/* BSS-INFO */
/*----------------------------------------------------------------------------*/
/*!
 * @brief This utility function is used to sync bss info with firmware
 *        when a new BSS has been connected or disconnected
 *
 * @param prAdapter          Pointer of ADAPTER_T
 *        ucBssIndex         Index of BSS-INFO
 *
 * @retval -
 */
/*----------------------------------------------------------------------------*/
uint32_t nicUpdateBss(IN struct ADAPTER *prAdapter,
		      IN uint8_t ucBssIndex)
{
	uint32_t u4Status = WLAN_STATUS_NOT_ACCEPTED;
	struct BSS_INFO *prBssInfo;
	struct CMD_SET_BSS_INFO rCmdSetBssInfo;

	ASSERT(prAdapter);
	ASSERT(ucBssIndex <= prAdapter->ucHwBssIdNum);

	prBssInfo = prAdapter->aprBssInfo[ucBssIndex];

	kalMemZero(&rCmdSetBssInfo,
		   sizeof(struct CMD_SET_BSS_INFO));

	rCmdSetBssInfo.ucBssIndex = ucBssIndex;
	rCmdSetBssInfo.ucConnectionState = (uint8_t)
					   prBssInfo->eConnectionState;
	rCmdSetBssInfo.ucCurrentOPMode = (uint8_t)
					 prBssInfo->eCurrentOPMode;
	rCmdSetBssInfo.ucSSIDLen = (uint8_t) prBssInfo->ucSSIDLen;
	kalMemCopy(rCmdSetBssInfo.aucSSID, prBssInfo->aucSSID,
		   prBssInfo->ucSSIDLen);
	COPY_MAC_ADDR(rCmdSetBssInfo.aucBSSID, prBssInfo->aucBSSID);
	rCmdSetBssInfo.ucIsQBSS = (uint8_t) prBssInfo->fgIsQBSS;
	rCmdSetBssInfo.ucNonHTBasicPhyType =
		prBssInfo->ucNonHTBasicPhyType;
	rCmdSetBssInfo.u2OperationalRateSet =
		prBssInfo->u2OperationalRateSet;
	rCmdSetBssInfo.u2BSSBasicRateSet =
		prBssInfo->u2BSSBasicRateSet;
	rCmdSetBssInfo.u2HwDefaultFixedRateCode =
		prBssInfo->u2HwDefaultFixedRateCode;
	rCmdSetBssInfo.ucPhyTypeSet = prBssInfo->ucPhyTypeSet;
	rCmdSetBssInfo.u4PrivateData = prBssInfo->u4PrivateData;
#if	CFG_SUPPORT_DBDC
	/*
	 *To do: In fact, this is not used anymore and could be removed now.
	 *But command structure change has driver and firmware dependency.
	 *So currently using ENUM_BAND_AUTO is a temporary solution.
	 */
	rCmdSetBssInfo.ucDBDCBand = ENUM_BAND_AUTO;
#endif
	rCmdSetBssInfo.ucWmmSet = prBssInfo->ucWmmQueSet;
	rCmdSetBssInfo.ucNss = prBssInfo->ucOpTxNss; /* Backward compatible */

	if (prBssInfo->fgBcDefaultKeyExist) {
		if (prBssInfo->wepkeyWlanIdx <
		    prAdapter->ucTxDefaultWlanIndex)
			rCmdSetBssInfo.ucBMCWlanIndex =
				prBssInfo->wepkeyWlanIdx;
		else if (prBssInfo->ucBMCWlanIndexSUsed[
			prBssInfo->ucBcDefaultKeyIdx])
			rCmdSetBssInfo.ucBMCWlanIndex =
				prBssInfo->ucBMCWlanIndexS[
				prBssInfo->ucBcDefaultKeyIdx];
	} else
		rCmdSetBssInfo.ucBMCWlanIndex = prBssInfo->ucBMCWlanIndex;

	DBGLOG(RSN, TRACE, "Update BSS BMC WlanIdx %u\n",
	       rCmdSetBssInfo.ucBMCWlanIndex);

#if CFG_ENABLE_WIFI_DIRECT
	rCmdSetBssInfo.ucHiddenSsidMode =
		prBssInfo->eHiddenSsidType;
#endif
	rlmFillSyncCmdParam(&rCmdSetBssInfo.rBssRlmParam,
			    prBssInfo);

	rCmdSetBssInfo.ucWapiMode = (uint8_t) FALSE;

	if (IS_BSS_AIS(prBssInfo)) {
		struct CONNECTION_SETTINGS *prConnSettings =
			aisGetConnSettings(prAdapter, ucBssIndex);
#if CFG_SUPPORT_PASSPOINT
		/* mapping OSEN to WPA2,
		 * due to firmware no need to know current is OSEN
		 */
		if (prConnSettings->eAuthMode == AUTH_MODE_WPA_OSEN)
			rCmdSetBssInfo.ucAuthMode = AUTH_MODE_WPA2;
		else
#endif
		rCmdSetBssInfo.ucAuthMode = (uint8_t) prConnSettings->eAuthMode;
		rCmdSetBssInfo.ucEncStatus = (uint8_t)
					     prConnSettings->eEncStatus;
		rCmdSetBssInfo.ucWapiMode = (uint8_t)
					    prConnSettings->fgWapiMode;
	}
#if CFG_ENABLE_BT_OVER_WIFI
	else if (IS_BSS_BOW(prBssInfo)) {
		rCmdSetBssInfo.ucAuthMode = (uint8_t) AUTH_MODE_WPA2_PSK;
		rCmdSetBssInfo.ucEncStatus = (uint8_t)
					     ENUM_ENCRYPTION3_KEY_ABSENT;
	}
#endif
	else {
#if CFG_ENABLE_WIFI_DIRECT
		if (prAdapter->fgIsP2PRegistered) {
			if (kalP2PGetCcmpCipher(prAdapter->prGlueInfo,
				(uint8_t) prBssInfo->u4PrivateData)) {
				rCmdSetBssInfo.ucAuthMode =
				(uint8_t) AUTH_MODE_WPA2_PSK;
				rCmdSetBssInfo.ucEncStatus =
					(uint8_t)
					ENUM_ENCRYPTION3_ENABLED;
			} else if (kalP2PGetTkipCipher(prAdapter->prGlueInfo,
					(uint8_t) prBssInfo->u4PrivateData)) {
				rCmdSetBssInfo.ucAuthMode =
					(uint8_t) AUTH_MODE_WPA_PSK;
				rCmdSetBssInfo.ucEncStatus =
					(uint8_t) ENUM_ENCRYPTION2_ENABLED;
			} else if (kalP2PGetWepCipher(prAdapter->prGlueInfo,
					(uint8_t) prBssInfo->u4PrivateData)) {
				rCmdSetBssInfo.ucAuthMode =
					(uint8_t) AUTH_MODE_OPEN;
				rCmdSetBssInfo.ucEncStatus =
					(uint8_t) ENUM_ENCRYPTION1_ENABLED;
			} else {
				rCmdSetBssInfo.ucAuthMode =
					(uint8_t) AUTH_MODE_OPEN;
				rCmdSetBssInfo.ucEncStatus =
					(uint8_t) ENUM_ENCRYPTION_DISABLED;
			}
			/* Need the probe response to detect the PBC overlap */
			rCmdSetBssInfo.ucIsApMode =
				p2pFuncIsAPMode(
					prAdapter->rWifiVar.prP2PConnSettings[
					prBssInfo->u4PrivateData]);

		}
#else
		rCmdSetBssInfo.ucAuthMode = (uint8_t) AUTH_MODE_WPA2_PSK;
		rCmdSetBssInfo.ucEncStatus = (uint8_t)
					     ENUM_ENCRYPTION3_KEY_ABSENT;
#endif
	}
	rCmdSetBssInfo.ucDisconnectDetectThreshold = 0;

	if (IS_BSS_AIS(prBssInfo) &&
	    (prBssInfo->eCurrentOPMode == OP_MODE_INFRASTRUCTURE) &&
	    (prBssInfo->prStaRecOfAP != NULL)) {
		rCmdSetBssInfo.ucStaRecIdxOfAP =
			prBssInfo->prStaRecOfAP->ucIndex;

		cnmAisInfraConnectNotify(prAdapter);
	}
#if CFG_ENABLE_WIFI_DIRECT
	else if ((prAdapter->fgIsP2PRegistered) &&
		 (prBssInfo->eNetworkType == NETWORK_TYPE_P2P) &&
		 (prBssInfo->eCurrentOPMode == OP_MODE_INFRASTRUCTURE)
		 && (prBssInfo->prStaRecOfAP != NULL)) {
		rCmdSetBssInfo.ucStaRecIdxOfAP =
			prBssInfo->prStaRecOfAP->ucIndex;
	}
#endif

#if CFG_ENABLE_BT_OVER_WIFI
/* disabled for BOW to finish ucBssIndex migration */
	else if (prBssInfo->eNetworkType == NETWORK_TYPE_BOW &&
		 prBssInfo->eCurrentOPMode == OP_MODE_BOW
		 && prBssInfo->prStaRecOfAP != NULL) {
		rCmdSetBssInfo.ucStaRecIdxOfAP =
			prBssInfo->prStaRecOfAP->ucIndex;
	}
#endif
	else
		rCmdSetBssInfo.ucStaRecIdxOfAP = STA_REC_INDEX_NOT_FOUND;

#if (CFG_SUPPORT_802_11AX == 1)
	memcpy(rCmdSetBssInfo.ucHeOpParams, prBssInfo->ucHeOpParams,
			HE_OP_BYTE_NUM);
	rCmdSetBssInfo.ucBssColorInfo = prBssInfo->ucBssColorInfo;
	rCmdSetBssInfo.u2HeBasicMcsSet =
		CPU_TO_LE16(prBssInfo->u2HeBasicMcsSet);
#endif

#if (CFG_SUPPORT_802_11V_MBSSID == 1)
	rCmdSetBssInfo.ucMaxBSSIDIndicator = prBssInfo->ucMaxBSSIDIndicator;
	rCmdSetBssInfo.ucMBSSIDIndex = prBssInfo->ucMBSSIDIndex;
#endif

	DBGLOG(BSS, INFO,
	       "Update Bss[%u] ConnState[%u] OPmode[%u] BSSID[" MACSTR
	       "] AuthMode[%u] EncStatus[%u]\n", ucBssIndex,
	       prBssInfo->eConnectionState,
	       prBssInfo->eCurrentOPMode, MAC2STR(prBssInfo->aucBSSID),
	       rCmdSetBssInfo.ucAuthMode,
	       rCmdSetBssInfo.ucEncStatus);

	u4Status = wlanSendSetQueryCmd(prAdapter,
				       CMD_ID_SET_BSS_INFO,
				       TRUE,
				       FALSE,
				       FALSE,
				       NULL, NULL,
				       sizeof(struct CMD_SET_BSS_INFO),
				       (uint8_t *)&rCmdSetBssInfo, NULL, 0);

	/* if BSS-INFO is going to be disconnected state,
	 * free all correlated station records
	 */
	if (prBssInfo->eConnectionState ==
	    MEDIA_STATE_DISCONNECTED) {
		/* clear client list */
		bssInitializeClientList(prAdapter, prBssInfo);

#if DBG
		DBGLOG(BSS, TRACE, "nicUpdateBss for disconnect state\n");
#endif
		/* free all correlated station records */
		cnmStaFreeAllStaByNetwork(prAdapter, ucBssIndex,
					  STA_REC_EXCLUDE_NONE);
		if (HAL_IS_TX_DIRECT(prAdapter))
			nicTxDirectClearBssAbsentQ(prAdapter, ucBssIndex);
		else
			qmFreeAllByBssIdx(prAdapter, ucBssIndex);
		kalClearSecurityFramesByBssIdx(prAdapter->prGlueInfo,
					       ucBssIndex);
#if CFG_ENABLE_GTK_FRAME_FILTER
		if (prBssInfo->prIpV4NetAddrList)
			FREE_IPV4_NETWORK_ADDR_LIST(
				prBssInfo->prIpV4NetAddrList);
#endif
#if CFG_SUPPORT_DBDC
		cnmDbdcRuntimeCheckDecision(prAdapter,
						ucBssIndex,
						FALSE);
#endif
	}

	return u4Status;
}

/* BSS-INFO Indication (PM) */
/*----------------------------------------------------------------------------*/
/*!
 * @brief This utility function is used to indicate PM that
 *        a BSS has been created. (for AdHoc / P2P-GO)
 *
 * @param prAdapter          Pointer of ADAPTER_T
 *        ucBssIndex         Index of BSS-INFO
 *
 * @retval -
 */
/*----------------------------------------------------------------------------*/
uint32_t nicPmIndicateBssCreated(IN struct ADAPTER
				 *prAdapter, IN uint8_t ucBssIndex)
{
	struct BSS_INFO *prBssInfo;
	struct CMD_INDICATE_PM_BSS_CREATED rCmdIndicatePmBssCreated;

	ASSERT(prAdapter);
	ASSERT(ucBssIndex <= prAdapter->ucHwBssIdNum);

	prBssInfo = GET_BSS_INFO_BY_INDEX(prAdapter, ucBssIndex);

	memset(&rCmdIndicatePmBssCreated, 0,
		sizeof(struct CMD_INDICATE_PM_BSS_CREATED));
	rCmdIndicatePmBssCreated.ucBssIndex = ucBssIndex;
	rCmdIndicatePmBssCreated.ucDtimPeriod =
		prBssInfo->ucDTIMPeriod;
	rCmdIndicatePmBssCreated.u2BeaconInterval =
		prBssInfo->u2BeaconInterval;
	rCmdIndicatePmBssCreated.u2AtimWindow =
		prBssInfo->u2ATIMWindow;

	return wlanSendSetQueryCmd(prAdapter,
	   CMD_ID_INDICATE_PM_BSS_CREATED,
	   TRUE,
	   FALSE,
	   FALSE,
	   NULL,
	   NULL,
	   sizeof(struct CMD_INDICATE_PM_BSS_CREATED),
	   (uint8_t *)&rCmdIndicatePmBssCreated, NULL, 0);
}

/*----------------------------------------------------------------------------*/
/*!
 * @brief This utility function is used to indicate PM that
 *        a BSS has been connected
 *
 * @param prAdapter          Pointer of ADAPTER_T
 *        eNetworkTypeIdx    Index of BSS-INFO
 *
 * @retval -
 */
/*----------------------------------------------------------------------------*/
uint32_t nicPmIndicateBssConnected(IN struct ADAPTER
				   *prAdapter, IN uint8_t ucBssIndex)
{
	struct BSS_INFO *prBssInfo;
	struct CMD_INDICATE_PM_BSS_CONNECTED
		rCmdIndicatePmBssConnected;

	ASSERT(prAdapter);
	ASSERT(ucBssIndex <= prAdapter->ucHwBssIdNum);

	prBssInfo = GET_BSS_INFO_BY_INDEX(prAdapter, ucBssIndex);

	memset(&rCmdIndicatePmBssConnected, 0,
		sizeof(struct CMD_INDICATE_PM_BSS_CONNECTED));
	rCmdIndicatePmBssConnected.ucBssIndex = ucBssIndex;
	rCmdIndicatePmBssConnected.ucDtimPeriod =
		prBssInfo->ucDTIMPeriod;
	rCmdIndicatePmBssConnected.u2AssocId = prBssInfo->u2AssocId;
	rCmdIndicatePmBssConnected.u2BeaconInterval =
		prBssInfo->u2BeaconInterval;
	rCmdIndicatePmBssConnected.u2AtimWindow =
		prBssInfo->u2ATIMWindow;

	rCmdIndicatePmBssConnected.ucBmpDeliveryAC =
		prBssInfo->rPmProfSetupInfo.ucBmpDeliveryAC;
	rCmdIndicatePmBssConnected.ucBmpTriggerAC =
		prBssInfo->rPmProfSetupInfo.ucBmpTriggerAC;

	/* rCmdIndicatePmBssConnected.ucBmpDeliveryAC, */
	/* rCmdIndicatePmBssConnected.ucBmpTriggerAC); */

	if ((GET_BSS_INFO_BY_INDEX(prAdapter,
		ucBssIndex)->eNetworkType == NETWORK_TYPE_AIS)
#if CFG_ENABLE_WIFI_DIRECT
	    || ((GET_BSS_INFO_BY_INDEX(prAdapter,
				ucBssIndex)->eNetworkType == NETWORK_TYPE_P2P)
		&& (prAdapter->fgIsP2PRegistered))
#endif
	   ) {
		if (prBssInfo->eCurrentOPMode == OP_MODE_INFRASTRUCTURE &&
		    prBssInfo->prStaRecOfAP) {
			uint8_t ucUapsd = wmmCalculateUapsdSetting(prAdapter,
				ucBssIndex);

			/* should sync Tspec uapsd settings */
			rCmdIndicatePmBssConnected.ucBmpDeliveryAC =
				(ucUapsd >> 4) & 0xf;
			rCmdIndicatePmBssConnected.ucBmpTriggerAC =
				ucUapsd & 0xf;
			rCmdIndicatePmBssConnected.fgIsUapsdConnection =
				(uint8_t) prBssInfo->prStaRecOfAP->
				fgIsUapsdSupported;
		} else {
			rCmdIndicatePmBssConnected.fgIsUapsdConnection =
				0;	/* @FIXME */
		}
	} else {
		rCmdIndicatePmBssConnected.fgIsUapsdConnection = 0;
	}

	return wlanSendSetQueryCmd(prAdapter,
	   CMD_ID_INDICATE_PM_BSS_CONNECTED,
	   TRUE,
	   FALSE,
	   FALSE,
	   NULL,
	   NULL,
	   sizeof(struct CMD_INDICATE_PM_BSS_CONNECTED),
	   (uint8_t *)&rCmdIndicatePmBssConnected, NULL, 0);
}

/*----------------------------------------------------------------------------*/
/*!
 * @brief This utility function is used to indicate PM that
 *        a BSS has been disconnected
 *
 * @param prAdapter          Pointer of ADAPTER_T
 *        ucBssIndex         Index of BSS-INFO
 *
 * @retval -
 */
/*----------------------------------------------------------------------------*/
uint32_t nicPmIndicateBssAbort(IN struct ADAPTER *prAdapter,
			       IN uint8_t ucBssIndex)
{
	struct CMD_INDICATE_PM_BSS_ABORT rCmdIndicatePmBssAbort;

	ASSERT(prAdapter);
	ASSERT(ucBssIndex <= prAdapter->ucHwBssIdNum);

	memset(&rCmdIndicatePmBssAbort, 0,
		sizeof(struct CMD_INDICATE_PM_BSS_ABORT));
	rCmdIndicatePmBssAbort.ucBssIndex = ucBssIndex;

	return wlanSendSetQueryCmd(prAdapter,
				   CMD_ID_INDICATE_PM_BSS_ABORT,
				   TRUE,
				   FALSE,
				   FALSE,
				   NULL,
				   NULL,
				   sizeof(struct CMD_INDICATE_PM_BSS_ABORT),
				   (uint8_t *)&rCmdIndicatePmBssAbort, NULL, 0);
}

/*----------------------------------------------------------------------------*/
/*!
 * @brief This utility function is used to set power save bit map
 *
 *
 * @param prAdapter          Pointer of ADAPTER_T
 *        ucBssIndex         Index of BSS-INFO
 *        ucSet              enter power save or not(1 PS, 0 not PS)
 *        ucCaller           index of bit map for caller
 * @retval -
 */
/*----------------------------------------------------------------------------*/
void
nicPowerSaveInfoMap(IN struct ADAPTER *prAdapter,
		    IN uint8_t ucBssIndex,
		    IN enum PARAM_POWER_MODE ePowerMode,
		    IN enum POWER_SAVE_CALLER ucCaller)
{
	uint32_t u4Flag;

	/* max caller is 24 */
	if (ucCaller >= PS_CALLER_MAX_NUM)
		ASSERT(0);

	u4Flag = prAdapter->rWlanInfo.u4PowerSaveFlag[ucBssIndex];

	if (ucCaller == PS_CALLER_WOW) {
		/* For WOW,
		 * must switch to sleep mode for low power
		 */
		if (ePowerMode == Param_PowerModeCAM) {
			u4Flag &= ~BIT(ucCaller);
			/* WOW disable, if other callers request to CAM --> sync to FW */
			if ((u4Flag & PS_CALLER_ACTIVE) != 0)
				u4Flag |= PS_SYNC_WITH_FW;
		} else {
			/* WOW enable */
			u4Flag |= BIT(ucCaller);
			u4Flag |= PS_SYNC_WITH_FW;
		}
	} else {
		/* set send command flag */
		if (ePowerMode != Param_PowerModeCAM) {
			u4Flag &= ~BIT(ucCaller);
			if ((u4Flag & PS_CALLER_ACTIVE) == 0)
				u4Flag |= PS_SYNC_WITH_FW;
		} else {
			if ((u4Flag & PS_CALLER_ACTIVE) == 0)
				u4Flag |= PS_SYNC_WITH_FW;
			u4Flag |= BIT(ucCaller);
		}
	}

	DBGLOG(NIC, INFO,
		"Flag=0x%04x, Caller=%d, PM=%d, PSFlag[%d]=0x%04x\n",
		u4Flag, ucCaller, ePowerMode, ucBssIndex,
		prAdapter->rWlanInfo.u4PowerSaveFlag[ucBssIndex]);

	prAdapter->rWlanInfo.u4PowerSaveFlag[ucBssIndex] = u4Flag;
}

/*----------------------------------------------------------------------------*/
/*!
 * @brief This utility function is used to set power save profile
 *
 *
 * @param prAdapter          Pointer of ADAPTER_T
 *        ucBssIndex         Index of BSS-INFO
 *        ucSet              enter power save or not(1 PS, 0 not PS)
 *        fgEnCmdEvent       Enable the functions when command done and timeout
 *        ucCaller           index of bit map for caller
 *
 * @retval WLAN_STATUS_SUCCESS
 * @retval WLAN_STATUS_PENDING
 * @retval WLAN_STATUS_FAILURE
 * @retval WLAN_STATUS_NOT_SUPPORTED
 */
/*----------------------------------------------------------------------------*/
uint32_t
nicConfigPowerSaveProfile(IN struct ADAPTER *prAdapter,
			  IN uint8_t ucBssIndex,
			  IN enum PARAM_POWER_MODE ePwrMode,
			  IN u_int8_t fgEnCmdEvent,
			  IN enum POWER_SAVE_CALLER ucCaller)
{
	DEBUGFUNC("nicConfigPowerSaveProfile");
	DBGLOG(INIT, TRACE,
		"ucBssIndex:%d, ePwrMode:%d, fgEnCmdEvent:%d\n",
		ucBssIndex, ePwrMode, fgEnCmdEvent);

	ASSERT(prAdapter);

	if (ucBssIndex >= prAdapter->ucHwBssIdNum) {
		ASSERT(0);
		return WLAN_STATUS_NOT_SUPPORTED;
	}

	nicPowerSaveInfoMap(prAdapter, ucBssIndex, ePwrMode, ucCaller);

	prAdapter->rWlanInfo.arPowerSaveMode[ucBssIndex].ucBssIndex
		= ucBssIndex;
	prAdapter->rWlanInfo.arPowerSaveMode[ucBssIndex].ucPsProfile
		= (uint8_t) ePwrMode;

	if (PS_SYNC_WITH_FW
		& prAdapter->rWlanInfo.u4PowerSaveFlag[ucBssIndex]) {
		uint32_t rWlanStatus = WLAN_STATUS_SUCCESS;

		prAdapter->rWlanInfo.u4PowerSaveFlag[ucBssIndex]
			&= ~PS_SYNC_WITH_FW;

		DBGLOG(NIC, TRACE,
			"SYNC_WITH_FW u4PowerSaveFlag[%d]=0x%04x\n",
			ucBssIndex,
			prAdapter->rWlanInfo.u4PowerSaveFlag[ucBssIndex]);

		rWlanStatus = wlanSendSetQueryCmd(prAdapter,	/* prAdapter */
			CMD_ID_POWER_SAVE_MODE,		/* ucCID */
			TRUE,	/* fgSetQuery */
			FALSE,	/* fgNeedResp */
			fgEnCmdEvent,	/* fgIsOid */

			/* pfCmdDoneHandler */
			(fgEnCmdEvent ? nicCmdEventSetCommon : NULL),

			/* pfCmdTimeoutHandler */
			(fgEnCmdEvent ? nicOidCmdTimeoutCommon : NULL),

			/* u4SetQueryInfoLen */
			sizeof(struct CMD_PS_PROFILE),

			/* pucInfoBuffer */
			(uint8_t *) &(prAdapter->rWlanInfo
				.arPowerSaveMode[ucBssIndex]),

			/* pvSetQueryBuffer */
			NULL,

			/* u4SetQueryBufferLen */
			0
			);

		if (fgEnCmdEvent)
			return rWlanStatus;
	}
	return WLAN_STATUS_SUCCESS;
} /* end of nicConfigPowerSaveProfile */

uint32_t
nicConfigProcSetCamCfgWrite(IN struct ADAPTER *prAdapter,
	IN u_int8_t enabled, IN uint8_t ucBssIndex)
{
	enum PARAM_POWER_MODE ePowerMode;
	struct CMD_PS_PROFILE rPowerSaveMode;

	if ((!prAdapter))
		return WLAN_STATUS_FAILURE;

	if (ucBssIndex >= BSS_DEFAULT_NUM)
		return WLAN_STATUS_FAILURE;
	memset(&rPowerSaveMode, 0, sizeof(struct CMD_PS_PROFILE));
	rPowerSaveMode.ucBssIndex = ucBssIndex;

	if (enabled) {
		prAdapter->rWlanInfo.fgEnSpecPwrMgt = TRUE;
		ePowerMode = Param_PowerModeCAM;
		rPowerSaveMode.ucPsProfile = (uint8_t) ePowerMode;
		DBGLOG(INIT, INFO, "Enable CAM BssIndex:%d, PowerMode:%d\n",
		       ucBssIndex, rPowerSaveMode.ucPsProfile);
	} else {
		prAdapter->rWlanInfo.fgEnSpecPwrMgt = FALSE;
		rPowerSaveMode.ucPsProfile =
			prAdapter->rWlanInfo.arPowerSaveMode[ucBssIndex].
			ucPsProfile;
		DBGLOG(INIT, INFO,
		       "Disable CAM BssIndex:%d, PowerMode:%d\n",
		       ucBssIndex, rPowerSaveMode.ucPsProfile);
	}

	return wlanSendSetQueryCmd(prAdapter,
				   CMD_ID_POWER_SAVE_MODE,
				   TRUE,
				   FALSE,
				   FALSE,
				   NULL,
				   NULL,
				   sizeof(struct CMD_PS_PROFILE),
				   (uint8_t *) &rPowerSaveMode,
				   NULL, 0);
}

uint32_t nicEnterCtiaMode(IN struct ADAPTER *prAdapter,
			  u_int8_t fgEnterCtia, u_int8_t fgEnCmdEvent)
{
	struct CMD_SW_DBG_CTRL rCmdSwCtrl;
	/* CMD_ACCESS_REG rCmdAccessReg; */
	uint32_t rWlanStatus;
	uint8_t ucBssIdx;

	DEBUGFUNC("nicEnterCtiaMode");
	DBGLOG(INIT, TRACE, "nicEnterCtiaMode: %d\n", fgEnterCtia);

	ASSERT(prAdapter);

	rWlanStatus = WLAN_STATUS_SUCCESS;
	memset(&rCmdSwCtrl, 0, sizeof(struct CMD_SW_DBG_CTRL));

	if (fgEnterCtia) {
		/* 0. Enter CTIA mode */
		prAdapter->fgInCtiaMode = TRUE;

		/* 1. Disable On-Lin Scan */
		prAdapter->fgEnOnlineScan = FALSE;

		/* 2. Disable FIFO FULL no ack */
		/* 3. Disable Roaming */
		/* 4. Disalbe auto tx power */
		rCmdSwCtrl.u4Id = 0xa0100003;
		rCmdSwCtrl.u4Data = 0x0;
		wlanSendSetQueryCmd(prAdapter,
			CMD_ID_SW_DBG_CTRL,
			TRUE,
			FALSE,
			FALSE, NULL, NULL,
			sizeof(struct CMD_SW_DBG_CTRL),
			(uint8_t *)&rCmdSwCtrl, NULL, 0);

		/* 2. Keep at CAM mode */
		for (ucBssIdx = 0; ucBssIdx < KAL_AIS_NUM; ucBssIdx++) {
			enum PARAM_POWER_MODE ePowerMode;

			prAdapter->u4CtiaPowerMode = 0;
			prAdapter->fgEnCtiaPowerMode = TRUE;

			ePowerMode = Param_PowerModeCAM;
			rWlanStatus = nicConfigPowerSaveProfile(prAdapter,
				ucBssIdx,
				ePowerMode, fgEnCmdEvent, PS_CALLER_CTIA);
		}

		/* 5. Disable Beacon Timeout Detection */
		prAdapter->fgDisBcnLostDetection = TRUE;
	} else {
		/* 1. Enaable On-Lin Scan */
		prAdapter->fgEnOnlineScan = TRUE;

		/* 2. Enable FIFO FULL no ack */
		/* 3. Enable Roaming */
		/* 4. Enable auto tx power */
		/*  */

		rCmdSwCtrl.u4Id = 0xa0100003;
		rCmdSwCtrl.u4Data = 0x1;
		wlanSendSetQueryCmd(prAdapter,
			CMD_ID_SW_DBG_CTRL,
			TRUE,
			FALSE,
			FALSE, NULL, NULL,
			sizeof(struct CMD_SW_DBG_CTRL),
			(uint8_t *)&rCmdSwCtrl, NULL, 0);

		/* 2. Keep at Fast PS */
		for (ucBssIdx = 0; ucBssIdx < KAL_AIS_NUM; ucBssIdx++) {
			enum PARAM_POWER_MODE ePowerMode;

			prAdapter->u4CtiaPowerMode = 2;
			prAdapter->fgEnCtiaPowerMode = TRUE;

			ePowerMode = Param_PowerModeFast_PSP;
			rWlanStatus = nicConfigPowerSaveProfile(prAdapter,
				ucBssIdx,
				ePowerMode, fgEnCmdEvent, PS_CALLER_CTIA);
		}

		/* 5. Enable Beacon Timeout Detection */
		prAdapter->fgDisBcnLostDetection = FALSE;

		/* 6. Leave CTIA mode  */
		prAdapter->fgInCtiaMode = FALSE;
	}

	return rWlanStatus;
}				/* end of nicEnterCtiaMode() */

uint32_t nicEnterCtiaModeOfScan(IN struct ADAPTER
	*prAdapter, u_int8_t fgEnterCtia, u_int8_t fgEnCmdEvent)
{
	uint32_t rWlanStatus;

	ASSERT(prAdapter);
	DBGLOG(INIT, INFO, "nicEnterCtiaModeOfScan: %d\n",
	       fgEnterCtia);

	rWlanStatus = WLAN_STATUS_SUCCESS;

	if (fgEnterCtia) {
		/* Disable On-Line Scan */
		prAdapter->fgEnOnlineScan = FALSE;
	} else {
		/* Enable On-Line Scan */
		prAdapter->fgEnOnlineScan = TRUE;
	}

	return rWlanStatus;
}

uint32_t nicEnterCtiaModeOfRoaming(IN struct ADAPTER
	*prAdapter, u_int8_t fgEnterCtia, u_int8_t fgEnCmdEvent)
{
	struct CMD_SW_DBG_CTRL rCmdSwCtrl;
	uint32_t rWlanStatus;

	ASSERT(prAdapter);
	DBGLOG(INIT, INFO, "nicEnterCtiaModeOfRoaming: %d\n",
	       fgEnterCtia);

	rWlanStatus = WLAN_STATUS_SUCCESS;
	memset(&rCmdSwCtrl, 0, sizeof(struct CMD_SW_DBG_CTRL));

	if (fgEnterCtia) {
		/* Disable Roaming */
		rCmdSwCtrl.u4Id = 0x55660000;
		rCmdSwCtrl.u4Data = 0x0;
		wlanSendSetQueryCmd(prAdapter,
			CMD_ID_SW_DBG_CTRL,
			TRUE,
			FALSE,
			FALSE, NULL, NULL,
			sizeof(struct CMD_SW_DBG_CTRL),
			(uint8_t *) &rCmdSwCtrl, NULL, 0);
	} else {
		/* Enable Roaming */
		rCmdSwCtrl.u4Id = 0x55660000;
		rCmdSwCtrl.u4Data = 0x1;
		wlanSendSetQueryCmd(prAdapter,
			CMD_ID_SW_DBG_CTRL,
			TRUE,
			FALSE,
			FALSE, NULL, NULL,
			sizeof(struct CMD_SW_DBG_CTRL),
			(uint8_t *) &rCmdSwCtrl, NULL, 0);
	}

	return rWlanStatus;
}

uint32_t nicEnterCtiaModeOfCAM(IN struct ADAPTER *prAdapter,
			       u_int8_t fgEnterCtia, u_int8_t fgEnCmdEvent)
{
	uint32_t rWlanStatus;
	uint8_t ucBssIdx;

	ASSERT(prAdapter);
	DBGLOG(INIT, INFO, "nicEnterCtiaModeOfCAM: %d\n",
	       fgEnterCtia);

	rWlanStatus = WLAN_STATUS_SUCCESS;

	if (fgEnterCtia) {
		/* Keep at CAM mode */
		for (ucBssIdx = 0; ucBssIdx < KAL_AIS_NUM; ucBssIdx++) {
			enum PARAM_POWER_MODE ePowerMode;

			prAdapter->u4CtiaPowerMode = 0;
			prAdapter->fgEnCtiaPowerMode = TRUE;

			ePowerMode = Param_PowerModeCAM;
			rWlanStatus = nicConfigPowerSaveProfile(prAdapter,
				ucBssIdx,
				ePowerMode, fgEnCmdEvent, PS_CALLER_CTIA_CAM);
		}
	} else {
		/* Keep at Fast PS */
		for (ucBssIdx = 0; ucBssIdx < KAL_AIS_NUM; ucBssIdx++) {
			enum PARAM_POWER_MODE ePowerMode;

			prAdapter->u4CtiaPowerMode = 2;
			prAdapter->fgEnCtiaPowerMode = TRUE;

			ePowerMode = Param_PowerModeFast_PSP;
			rWlanStatus = nicConfigPowerSaveProfile(prAdapter,
				ucBssIdx,
				ePowerMode, fgEnCmdEvent, PS_CALLER_CTIA_CAM);
		}
	}

	return rWlanStatus;
}

uint32_t nicEnterCtiaModeOfBCNTimeout(IN struct ADAPTER
	*prAdapter, u_int8_t fgEnterCtia, u_int8_t fgEnCmdEvent)
{
	uint32_t rWlanStatus;

	ASSERT(prAdapter);
	DBGLOG(INIT, INFO, "nicEnterCtiaModeOfBCNTimeout: %d\n",
	       fgEnterCtia);

	rWlanStatus = WLAN_STATUS_SUCCESS;

	if (fgEnterCtia) {
		/* Disable Beacon Timeout Detection */
		prAdapter->fgDisBcnLostDetection = TRUE;
	} else {
		/* Enable Beacon Timeout Detection */
		prAdapter->fgDisBcnLostDetection = FALSE;
	}

	return rWlanStatus;
}

uint32_t nicEnterCtiaModeOfAutoTxPower(IN struct ADAPTER
	*prAdapter, u_int8_t fgEnterCtia, u_int8_t fgEnCmdEvent)
{
	struct CMD_SW_DBG_CTRL rCmdSwCtrl;
	uint32_t rWlanStatus;

	ASSERT(prAdapter);
	DBGLOG(INIT, INFO, "nicEnterCtiaModeOfAutoTxPower: %d\n",
	       fgEnterCtia);

	rWlanStatus = WLAN_STATUS_SUCCESS;
	memset(&rCmdSwCtrl, 0, sizeof(struct CMD_SW_DBG_CTRL));

	if (fgEnterCtia) {
		/* Disalbe auto tx power */
		rCmdSwCtrl.u4Id = 0x55670003;
		rCmdSwCtrl.u4Data = 0x0;
		wlanSendSetQueryCmd(prAdapter,
			CMD_ID_SW_DBG_CTRL,
			TRUE,
			FALSE,
			FALSE,
			NULL,
			NULL,
			sizeof(struct CMD_SW_DBG_CTRL),
			(uint8_t *) &rCmdSwCtrl,
			NULL, 0);
	} else {
		/* Enable auto tx power */
		rCmdSwCtrl.u4Id = 0x55670003;
		rCmdSwCtrl.u4Data = 0x1;
		wlanSendSetQueryCmd(prAdapter,
			CMD_ID_SW_DBG_CTRL,
			TRUE,
			FALSE,
			FALSE,
			NULL,
			NULL,
			sizeof(struct CMD_SW_DBG_CTRL),
			(uint8_t *) &rCmdSwCtrl,
			NULL, 0);
	}

	return rWlanStatus;
}

uint32_t nicEnterCtiaModeOfFIFOFullNoAck(IN struct ADAPTER
		*prAdapter, u_int8_t fgEnterCtia, u_int8_t fgEnCmdEvent)
{
	struct CMD_SW_DBG_CTRL rCmdSwCtrl;
	uint32_t rWlanStatus;

	ASSERT(prAdapter);
	DBGLOG(INIT, INFO, "nicEnterCtiaModeOfFIFOFullNoAck: %d\n",
	       fgEnterCtia);

	rWlanStatus = WLAN_STATUS_SUCCESS;
	memset(&rCmdSwCtrl, 0, sizeof(struct CMD_SW_DBG_CTRL));

	if (fgEnterCtia) {
		/* Disable FIFO FULL no ack */
		rCmdSwCtrl.u4Id = 0x55680000;
		rCmdSwCtrl.u4Data = 0x0;
		wlanSendSetQueryCmd(prAdapter,
			CMD_ID_SW_DBG_CTRL,
			TRUE,
			FALSE,
			FALSE,
			NULL,
			NULL,
			sizeof(struct CMD_SW_DBG_CTRL),
			(uint8_t *) &rCmdSwCtrl,
			NULL, 0);
	} else {
		/* Enable FIFO FULL no ack */
		rCmdSwCtrl.u4Id = 0x55680000;
		rCmdSwCtrl.u4Data = 0x1;
		wlanSendSetQueryCmd(prAdapter,
			CMD_ID_SW_DBG_CTRL,
			TRUE,
			FALSE,
			FALSE,
			NULL,
			NULL,
			sizeof(struct CMD_SW_DBG_CTRL),
			(uint8_t *) &rCmdSwCtrl,
			NULL, 0);
	}

	return rWlanStatus;
}

uint32_t nicEnterTPTestMode(IN struct ADAPTER *prAdapter,
			    IN uint8_t ucFuncMask)
{
	struct CMD_SW_DBG_CTRL rCmdSwCtrl;
	uint32_t rWlanStatus;
	uint8_t ucBssIdx;
	struct BSS_INFO *prBssInfo;

	ASSERT(prAdapter);

	rWlanStatus = WLAN_STATUS_SUCCESS;
	memset(&rCmdSwCtrl, 0, sizeof(struct CMD_SW_DBG_CTRL));

	if (ucFuncMask) {
		/* 1. Disable On-Lin Scan */
		if (ucFuncMask & TEST_MODE_DISABLE_ONLINE_SCAN)
			prAdapter->fgEnOnlineScan = FALSE;

		/* 2. Disable Roaming */
		if (ucFuncMask & TEST_MODE_DISABLE_ROAMING) {
			rCmdSwCtrl.u4Id = 0xa0210000;
			rCmdSwCtrl.u4Data = 0x0;
			wlanSendSetQueryCmd(prAdapter, CMD_ID_SW_DBG_CTRL, TRUE,
				FALSE, FALSE,
				NULL, NULL, sizeof(struct CMD_SW_DBG_CTRL),
				(uint8_t *)&rCmdSwCtrl, NULL, 0);
		}
		/* 3. Keep at CAM mode */
		if (ucFuncMask & TEST_MODE_FIXED_CAM_MODE)
			for (ucBssIdx = 0; ucBssIdx < prAdapter->ucHwBssIdNum;
			     ucBssIdx++) {
				prBssInfo =
					GET_BSS_INFO_BY_INDEX(prAdapter,
						ucBssIdx);
				if (prBssInfo->fgIsInUse
				    && (prBssInfo->eCurrentOPMode
				    == OP_MODE_INFRASTRUCTURE))
					nicConfigPowerSaveProfile(prAdapter,
						ucBssIdx, Param_PowerModeCAM,
						FALSE, PS_CALLER_TP);
			}

		/* 4. Disable Beacon Timeout Detection */
		if (ucFuncMask & TEST_MODE_DISABLE_BCN_LOST_DET)
			prAdapter->fgDisBcnLostDetection = TRUE;
	} else {
		/* 1. Enaable On-Lin Scan */
		prAdapter->fgEnOnlineScan = TRUE;

		/* 2. Enable Roaming */
		rCmdSwCtrl.u4Id = 0xa0210000;
		rCmdSwCtrl.u4Data = 0x1;
		wlanSendSetQueryCmd(prAdapter, CMD_ID_SW_DBG_CTRL, TRUE,
				    FALSE, FALSE,
				    NULL, NULL, sizeof(struct CMD_SW_DBG_CTRL),
				    (uint8_t *)&rCmdSwCtrl, NULL, 0);

		/* 3. Keep at Fast PS */
		for (ucBssIdx = 0; ucBssIdx < prAdapter->ucHwBssIdNum;
		     ucBssIdx++) {
			prBssInfo = GET_BSS_INFO_BY_INDEX(prAdapter, ucBssIdx);
			if (prBssInfo->fgIsInUse
			    && (prBssInfo->eCurrentOPMode
						== OP_MODE_INFRASTRUCTURE))
				nicConfigPowerSaveProfile(prAdapter, ucBssIdx,
					Param_PowerModeFast_PSP,
					FALSE, PS_CALLER_TP);
		}

		/* 4. Enable Beacon Timeout Detection */
		prAdapter->fgDisBcnLostDetection = FALSE;
	}

	return rWlanStatus;
}

/*----------------------------------------------------------------------------*/
/*!
 * @brief This utility function is used to indicate firmware domain
 *        for beacon generation parameters
 *
 * @param prAdapter          Pointer of ADAPTER_T
 *        eIeUpdMethod,      Update Method
 *        ucBssIndex         Index of BSS-INFO
 *        u2Capability       Capability
 *        aucIe              Pointer to buffer of IEs
 *        u2IELen            Length of IEs
 *
 * @retval - WLAN_STATUS_SUCCESS
 *           WLAN_STATUS_FAILURE
 *           WLAN_STATUS_PENDING
 *           WLAN_STATUS_INVALID_DATA
 */
/*----------------------------------------------------------------------------*/
uint32_t
nicUpdateBeaconIETemplate(IN struct ADAPTER *prAdapter,
			  IN enum ENUM_IE_UPD_METHOD eIeUpdMethod,
			  IN uint8_t ucBssIndex, IN uint16_t u2Capability,
			  IN uint8_t *aucIe, IN uint16_t u2IELen)
{
	struct CMD_BEACON_TEMPLATE_UPDATE *prCmdBcnUpdate;
	uint16_t u2CmdBufLen = 0, cmd_size;
	struct GLUE_INFO *prGlueInfo;
	struct CMD_INFO *prCmdInfo;
	struct mt66xx_chip_info *prChipInfo;

	DEBUGFUNC("wlanUpdateBeaconIETemplate");
	DBGLOG(INIT, LOUD, "\n");

	ASSERT(prAdapter);
	prGlueInfo = prAdapter->prGlueInfo;
	prChipInfo = prAdapter->chip_info;

	if (u2IELen > MAX_IE_LENGTH)
		return WLAN_STATUS_INVALID_DATA;

	if (eIeUpdMethod == IE_UPD_METHOD_UPDATE_RANDOM
	    || eIeUpdMethod == IE_UPD_METHOD_UPDATE_ALL) {
		u2CmdBufLen = OFFSET_OF(struct CMD_BEACON_TEMPLATE_UPDATE,
					aucIE) + u2IELen;
	} else if (eIeUpdMethod == IE_UPD_METHOD_DELETE_ALL) {
		u2CmdBufLen = OFFSET_OF(struct CMD_BEACON_TEMPLATE_UPDATE,
					u2IELen);
#if CFG_SUPPORT_P2P_GO_OFFLOAD_PROBE_RSP
	} else if (eIeUpdMethod == IE_UPD_METHOD_UPDATE_PROBE_RSP) {
		u2CmdBufLen = OFFSET_OF(struct CMD_BEACON_TEMPLATE_UPDATE,
					aucIE) + u2IELen;
		DBGLOG(NIC, INFO,
		       "update for probe response offload to firmware\n");
#endif
	} else {
		DBGLOG(INIT, ERROR, "Unknown IeUpdMethod.\n");
		return WLAN_STATUS_FAILURE;
	}

	/* prepare command info */
	cmd_size = prChipInfo->u2CmdTxHdrSize + u2CmdBufLen;
	prCmdInfo = cmdBufAllocateCmdInfo(prAdapter, cmd_size);
	if (!prCmdInfo) {
		DBGLOG(INIT, ERROR, "Allocate CMD_INFO_T ==> FAILED.\n");
		return WLAN_STATUS_FAILURE;
	}

	/* Setup common CMD Info Packet */
	prCmdInfo->eCmdType = COMMAND_TYPE_NETWORK_IOCTL;
	prCmdInfo->u2InfoBufLen = cmd_size;
	prCmdInfo->pfCmdDoneHandler = NULL;	/* @FIXME */
	prCmdInfo->pfCmdTimeoutHandler = NULL;	/* @FIXME */
	prCmdInfo->fgIsOid = FALSE;
	prCmdInfo->ucCID = CMD_ID_UPDATE_BEACON_CONTENT;
	prCmdInfo->fgSetQuery = TRUE;
	prCmdInfo->fgNeedResp = FALSE;
	prCmdInfo->u4SetInfoLen = u2CmdBufLen;
	prCmdInfo->pvInformationBuffer = NULL;
	prCmdInfo->u4InformationBufferLength = 0;

	/* Setup WIFI_CMD_T (no payload) */
	NIC_FILL_CMD_TX_HDR(prAdapter,
		prCmdInfo->pucInfoBuffer,
		prCmdInfo->u2InfoBufLen,
		prCmdInfo->ucCID,
		CMD_PACKET_TYPE_ID,
		&prCmdInfo->ucCmdSeqNum,
		prCmdInfo->fgSetQuery,
		&prCmdBcnUpdate, FALSE, 0, S2D_INDEX_CMD_H2N,
		prCmdInfo->fgNeedResp);

	/* fill beacon updating command */
	prCmdBcnUpdate->ucUpdateMethod = (uint8_t) eIeUpdMethod;
	prCmdBcnUpdate->ucBssIndex = ucBssIndex;
	prCmdBcnUpdate->u2Capability = u2Capability;
	prCmdBcnUpdate->u2IELen = u2IELen;
	if (u2IELen > 0)
		kalMemCopy(prCmdBcnUpdate->aucIE, aucIe, u2IELen);
	/* insert into prCmdQueue */
	kalEnqueueCommand(prGlueInfo,
			  (struct QUE_ENTRY *) prCmdInfo);

	/* wakeup txServiceThread later */
	GLUE_SET_EVENT(prGlueInfo);
	return WLAN_STATUS_PENDING;
}

/*----------------------------------------------------------------------------*/
/*!
 * @brief This utility function is used to initialization PHY related
 *        varaibles
 *
 * @param prAdapter  Pointer of ADAPTER_T
 *
 * @retval none
 */
/*----------------------------------------------------------------------------*/
void nicSetAvailablePhyTypeSet(IN struct ADAPTER *prAdapter)
{
	ASSERT(prAdapter);

	if (prAdapter->rWifiVar.eDesiredPhyConfig >= PHY_CONFIG_NUM) {
		ASSERT(0);
		return;
	}

	prAdapter->rWifiVar.ucAvailablePhyTypeSet =
		aucPhyCfg2PhyTypeSet[
		prAdapter->rWifiVar.eDesiredPhyConfig];

	if (prAdapter->rWifiVar.ucAvailablePhyTypeSet &
	    PHY_TYPE_BIT_ERP)
		prAdapter->rWifiVar.eNonHTBasicPhyType2G4 =
			PHY_TYPE_ERP_INDEX;
	/* NOTE(Kevin): Because we don't have N only mode, TBD */
	else
		prAdapter->rWifiVar.eNonHTBasicPhyType2G4 =
			PHY_TYPE_HR_DSSS_INDEX;

}

/*----------------------------------------------------------------------------*/
/*!
 * @brief This utility function is used to update WMM Parms
 *
 * @param prAdapter          Pointer of ADAPTER_T
 *        ucBssIndex         Index of BSS-INFO
 *
 * @retval -
 */
/*----------------------------------------------------------------------------*/
uint32_t nicQmUpdateWmmParms(IN struct ADAPTER *prAdapter,
			     IN uint8_t ucBssIndex)
{
	struct BSS_INFO *prBssInfo;
	struct CMD_UPDATE_WMM_PARMS rCmdUpdateWmmParms;
	struct mt66xx_chip_info *prChipInfo;

	ASSERT(prAdapter);
	prChipInfo = prAdapter->chip_info;

	DBGLOG(QM, INFO, "Update WMM parameters for BSS[%u]\n",
	       ucBssIndex);

	DBGLOG(QM, EVENT, "sizeof(struct AC_QUE_PARMS): %zu\n",
	       sizeof(struct AC_QUE_PARMS));
	DBGLOG(QM, EVENT, "sizeof(CMD_UPDATE_WMM_PARMS): %zu\n",
	       sizeof(struct CMD_UPDATE_WMM_PARMS));
	DBGLOG(QM, EVENT, "u2CmdTxHdrSize: %u\n",
	       prChipInfo->u2CmdTxHdrSize);

	prBssInfo = GET_BSS_INFO_BY_INDEX(prAdapter, ucBssIndex);
	memset(&rCmdUpdateWmmParms, 0, sizeof(struct CMD_UPDATE_WMM_PARMS));
	rCmdUpdateWmmParms.ucBssIndex = (uint8_t) ucBssIndex;
	kalMemCopy(&rCmdUpdateWmmParms.arACQueParms[0],
		   &prBssInfo->arACQueParms[0],
		   (sizeof(struct AC_QUE_PARMS) * AC_NUM));

	rCmdUpdateWmmParms.fgIsQBSS = prBssInfo->fgIsQBSS;
	rCmdUpdateWmmParms.ucWmmSet = (uint8_t)
				      prBssInfo->ucWmmQueSet;

	return wlanSendSetQueryCmd(prAdapter,
				   CMD_ID_UPDATE_WMM_PARMS,
				   TRUE,
				   FALSE,
				   FALSE,
				   NULL, NULL,
				   sizeof(struct CMD_UPDATE_WMM_PARMS),
				   (uint8_t *)&rCmdUpdateWmmParms, NULL, 0);
}

#if (CFG_SUPPORT_802_11AX == 1)
uint32_t nicQmUpdateMUEdcaParams(IN struct ADAPTER *prAdapter,
	IN uint8_t ucBssIndex)
{
	struct BSS_INFO *prBssInfo;
	struct _CMD_MQM_UPDATE_MU_EDCA_PARMS_T rCmdUpdateMUEdcaParms;

	ASSERT(prAdapter);

	DBGLOG(QM, INFO, "Update MU EDCA parameters for BSS[%u]\n", ucBssIndex);

	prBssInfo = GET_BSS_INFO_BY_INDEX(prAdapter, ucBssIndex);
	memset(&rCmdUpdateMUEdcaParms, 0,
		sizeof(struct _CMD_MQM_UPDATE_MU_EDCA_PARMS_T));
	rCmdUpdateMUEdcaParms.ucBssIndex = (uint8_t) ucBssIndex;

	if (prAdapter->fgMuEdcaOverride) {

		enum ENUM_WMM_ACI eAci;
		struct _CMD_MU_EDCA_PARAMS_T *prMUEdca;

		for (eAci = 0; eAci < WMM_AC_INDEX_NUM; eAci++) {

			prMUEdca =
				&(rCmdUpdateMUEdcaParms.arMUEdcaParams[eAci]);

			prMUEdca->ucECWmin = 15;
			prMUEdca->ucECWmax = 15;
			prMUEdca->ucAifsn = 0;
			prMUEdca->ucIsACMSet = 0;
			prMUEdca->ucMUEdcaTimer = 0xff;
		}
	} else {
		kalMemCopy(&rCmdUpdateMUEdcaParms.arMUEdcaParams[0],
			&prBssInfo->arMUEdcaParams[0],
			(sizeof(struct _CMD_MU_EDCA_PARAMS_T) * AC_NUM));
	}

	rCmdUpdateMUEdcaParms.fgIsQBSS = prBssInfo->fgIsQBSS;
	rCmdUpdateMUEdcaParms.ucWmmSet = (uint8_t) prBssInfo->ucWmmQueSet;

	return wlanSendSetQueryCmd(prAdapter,
				CMD_ID_MQM_UPDATE_MU_EDCA_PARMS,
				TRUE,
				FALSE,
				FALSE,
				NULL, NULL,
				sizeof(struct _CMD_MQM_UPDATE_MU_EDCA_PARMS_T),
				(uint8_t *)&rCmdUpdateMUEdcaParms, NULL, 0);
}

uint32_t nicRlmUpdateSRParams(IN struct ADAPTER *prAdapter,
	IN uint8_t ucBssIndex)
{
	struct BSS_INFO *prBssInfo;
	struct _CMD_RLM_UPDATE_SR_PARMS_T rCmdUpdateSRParms;

	ASSERT(prAdapter);

	DBGLOG(RLM, INFO, "Update Spatial Reuse parameters for BSS[%u]\n",
		ucBssIndex);

	prBssInfo = GET_BSS_INFO_BY_INDEX(prAdapter, ucBssIndex);

	memset(&rCmdUpdateSRParms, 0,
		sizeof(struct _CMD_RLM_UPDATE_SR_PARMS_T));
	rCmdUpdateSRParms.ucBssIndex = ucBssIndex;
	rCmdUpdateSRParms.ucSRControl = prBssInfo->ucSRControl;
	rCmdUpdateSRParms.ucNonSRGObssPdMaxOffset =
		prBssInfo->ucNonSRGObssPdMaxOffset;
	rCmdUpdateSRParms.ucSRGObssPdMinOffset =
		prBssInfo->ucSRGObssPdMinOffset;
	rCmdUpdateSRParms.ucSRGObssPdMaxOffset =
		prBssInfo->ucSRGObssPdMaxOffset;
	rCmdUpdateSRParms.u4SRGBSSColorBitmapLow = CPU_TO_LE32(
		(uint32_t)(prBssInfo->u8SRGBSSColorBitmap & 0xFFFFFFFF));
	rCmdUpdateSRParms.u4SRGBSSColorBitmapHigh = CPU_TO_LE32(
		(uint32_t)(prBssInfo->u8SRGBSSColorBitmap >> 32));
	rCmdUpdateSRParms.u4SRGPartialBSSIDBitmapLow = CPU_TO_LE32(
		(uint32_t)(prBssInfo->u8SRGPartialBSSIDBitmap & 0xFFFFFFFF));
	rCmdUpdateSRParms.u4SRGPartialBSSIDBitmapHigh = CPU_TO_LE32(
		(uint32_t)(prBssInfo->u8SRGPartialBSSIDBitmap >> 32));

	return wlanSendSetQueryCmd(prAdapter,
				CMD_ID_RLM_UPDATE_SR_PARAMS,
				TRUE,
				FALSE,
				FALSE,
				NULL, NULL,
				sizeof(struct _CMD_RLM_UPDATE_SR_PARMS_T),
				(uint8_t *)&rCmdUpdateSRParms, NULL, 0);
}
#endif

/*----------------------------------------------------------------------------*/
/*!
 * @brief This utility function is used to update TX power gain corresponding to
 *        each band/modulation combination
 *
 * @param prAdapter          Pointer of ADAPTER_T
 *        prTxPwrParam       Pointer of TX power parameters
 *
 * @retval WLAN_STATUS_PENDING
 *         WLAN_STATUS_FAILURE
 */
/*----------------------------------------------------------------------------*/
uint32_t nicUpdateTxPower(IN struct ADAPTER *prAdapter,
			  IN struct CMD_TX_PWR *prTxPwrParam)
{
	DEBUGFUNC("nicUpdateTxPower");

	ASSERT(prAdapter);

	return wlanSendSetQueryCmd(prAdapter,
				   CMD_ID_SET_TX_PWR,
				   TRUE,
				   FALSE, FALSE, NULL, NULL,
				   sizeof(struct CMD_TX_PWR),
				   (uint8_t *) prTxPwrParam, NULL, 0);
}

/*----------------------------------------------------------------------------*/
/*!
 * @brief This utility function is used to set auto tx power parameter
 *
 * @param prAdapter          Pointer of ADAPTER_T
 *        prTxPwrParam       Pointer of Auto TX power parameters
 *
 * @retval WLAN_STATUS_PENDING
 *         WLAN_STATUS_FAILURE
 */
/*----------------------------------------------------------------------------*/
uint32_t nicSetAutoTxPower(IN struct ADAPTER *prAdapter,
			   IN struct CMD_AUTO_POWER_PARAM *prAutoPwrParam)
{
	DEBUGFUNC("nicSetAutoTxPower");

	ASSERT(prAdapter);

	return wlanSendSetQueryCmd(prAdapter,
				   CMD_ID_SET_AUTOPWR_CTRL,
				   TRUE,
				   FALSE,
				   FALSE,
				   NULL, NULL,
				   sizeof(struct CMD_AUTO_POWER_PARAM),
				   (uint8_t *) prAutoPwrParam, NULL, 0);
}

/*----------------------------------------------------------------------------*/
/*!
 * @brief This utility function is used to update TX power gain corresponding to
 *        each band/modulation combination
 *
 * @param prAdapter          Pointer of ADAPTER_T
 *        prTxPwrParam       Pointer of TX power parameters
 *
 * @retval WLAN_STATUS_PENDING
 *         WLAN_STATUS_FAILURE
 */
/*----------------------------------------------------------------------------*/
uint32_t nicSetAutoTxPowerControl(IN struct ADAPTER
	*prAdapter, IN struct CMD_TX_PWR *prTxPwrParam)
{
	DEBUGFUNC("nicUpdateTxPower");

	ASSERT(prAdapter);

	return wlanSendSetQueryCmd(prAdapter,
				   CMD_ID_SET_TX_PWR,
				   TRUE,
				   FALSE, FALSE, NULL, NULL,
				   sizeof(struct CMD_TX_PWR),
				   (uint8_t *) prTxPwrParam, NULL, 0);
}

/*----------------------------------------------------------------------------*/
/*!
 * @brief This utility function is used to update power offset around 5GHz band
 *
 * @param prAdapter          Pointer of ADAPTER_T
 *        pr5GPwrOffset      Pointer of 5GHz power offset parameter
 *
 * @retval WLAN_STATUS_PENDING
 *         WLAN_STATUS_FAILURE
 */
/*----------------------------------------------------------------------------*/
uint32_t nicUpdate5GOffset(IN struct ADAPTER *prAdapter,
			   IN struct CMD_5G_PWR_OFFSET *pr5GPwrOffset)
{
#if 0				/* It is not needed anymore */
	DEBUGFUNC("nicUpdate5GOffset");

	ASSERT(prAdapter);

	return wlanSendSetQueryCmd(prAdapter,
				   CMD_ID_SET_5G_PWR_OFFSET,
				   TRUE,
				   FALSE,
				   FALSE, NULL, NULL,
				   sizeof(struct CMD_5G_PWR_OFFSET),
				   (uint8_t *) pr5GPwrOffset, NULL, 0);
#else
	return 0;
#endif
}

/*----------------------------------------------------------------------------*/
/*!
 * @brief This utility function is used to update DPD calibration result
 *
 * @param prAdapter          Pointer of ADAPTER_T
 *        pr5GPwrOffset      Pointer of parameter for DPD calibration result
 *
 * @retval WLAN_STATUS_PENDING
 *         WLAN_STATUS_FAILURE
 */
/*----------------------------------------------------------------------------*/
uint32_t nicUpdateDPD(IN struct ADAPTER *prAdapter,
		      IN struct CMD_PWR_PARAM *prDpdCalResult)
{
	DEBUGFUNC("nicUpdateDPD");

	ASSERT(prAdapter);

	return wlanSendSetQueryCmd(prAdapter,
				   CMD_ID_SET_PWR_PARAM,
				   TRUE,
				   FALSE,
				   FALSE, NULL, NULL,
				   sizeof(struct CMD_PWR_PARAM),
				   (uint8_t *) prDpdCalResult, NULL, 0);
}

/*----------------------------------------------------------------------------*/
/*!
 * @brief This utility function starts system service such as timer and
 *        memory pools
 *
 * @param prAdapter          Pointer of ADAPTER_T
 *
 * @retval none
 */
/*----------------------------------------------------------------------------*/
void nicInitSystemService(IN struct ADAPTER *prAdapter,
				   IN const u_int8_t bAtResetFlow)
{
	ASSERT(prAdapter);

	/* <1> Initialize MGMT Memory pool and STA_REC */
	if (!bAtResetFlow) {
		cnmMemInit(prAdapter);
		cnmStaRecInit(prAdapter);
	}

	cmdBufInitialize(prAdapter);

	if (!bAtResetFlow) {
		/* <2> Mailbox Initialization */
		mboxInitialize(prAdapter);
	}

	/* <3> Timer Initialization */
	cnmTimerInitialize(prAdapter);
}

/*----------------------------------------------------------------------------*/
/*!
 * @brief This utility function reset some specific system service,
 *        such as STA-REC
 *
 * @param prAdapter          Pointer of ADAPTER_T
 *
 * @retval none
 */
/*----------------------------------------------------------------------------*/
void nicResetSystemService(IN struct ADAPTER *prAdapter)
{
	ASSERT(prAdapter);
}

/*----------------------------------------------------------------------------*/
/*!
 * @brief This utility function is used to update WMM Parms
 *
 * @param prAdapter          Pointer of ADAPTER_T
 *
 * @retval none
 */
/*----------------------------------------------------------------------------*/
void nicUninitSystemService(IN struct ADAPTER *prAdapter)
{
	ASSERT(prAdapter);

	/* Timer Destruction */
	cnmTimerDestroy(prAdapter);

	/* Mailbox Destruction */
	mboxDestroy(prAdapter);

	/* CmdBuf Destruction */
	cmdBufUninit(prAdapter);
}

/*----------------------------------------------------------------------------*/
/*!
 * @brief This utility function is used to update WMM Parms
 *
 * @param prAdapter          Pointer of ADAPTER_T
 *
 * @retval none
 */
/*----------------------------------------------------------------------------*/
void nicInitMGMT(IN struct ADAPTER *prAdapter,
		 IN struct REG_INFO *prRegInfo)
{
	uint8_t i;

	ASSERT(prAdapter);

	/* CNM Module - initialization */
	cnmInit(prAdapter);

	wmmInit(prAdapter);

	/* RLM Module - initialization */
	rlmFsmEventInit(prAdapter);

	/* SCN Module - initialization */
	scnInit(prAdapter);

	for (i = 0; i < KAL_AIS_NUM; i++) {
		/* AIS Module - intiailization */
		aisFsmInit(prAdapter, i);
		aisInitializeConnectionSettings(prAdapter,
			prRegInfo,
			i);
#if CFG_SUPPORT_ROAMING
		/* Roaming Module - intiailization */
		roamingFsmInit(prAdapter, i);
#endif /* CFG_SUPPORT_ROAMING */
	}

#if CFG_SUPPORT_SWCR
	swCrDebugInit(prAdapter);
#endif /* CFG_SUPPORT_SWCR */

}

/*----------------------------------------------------------------------------*/
/*!
 * @brief This utility function is used to update WMM Parms
 *
 * @param prAdapter          Pointer of ADAPTER_T
 *
 * @retval none
 */
/*----------------------------------------------------------------------------*/
void nicUninitMGMT(IN struct ADAPTER *prAdapter)
{
	uint8_t i;

	ASSERT(prAdapter);

#if CFG_SUPPORT_SWCR
	swCrDebugUninit(prAdapter);
#endif /* CFG_SUPPORT_SWCR */

	for (i = 0; i < KAL_AIS_NUM; i++) {
#if CFG_SUPPORT_ROAMING
		/* Roaming Module - unintiailization */
		roamingFsmUninit(prAdapter, i);
#endif /* CFG_SUPPORT_ROAMING */

		/* AIS Module - unintiailization */
		aisFsmUninit(prAdapter, i);
	}

	/* SCN Module - unintiailization */
	scnUninit(prAdapter);

	wmmUnInit(prAdapter);

	/* RLM Module - uninitialization */
	rlmFsmEventUninit(prAdapter);

	/* CNM Module - uninitialization */
	cnmUninit(prAdapter);

}

/*----------------------------------------------------------------------------*/
/*!
 * @brief This function is invoked to buffer scan result
 *
 * @param prAdapter          Pointer to the Adapter structure.
 * @param rMacAddr           BSSID
 * @param prSsid             Pointer to SSID
 * @param u2CapInfo          Capability settings
 * @param rRssi              Received Strength (-10 ~ -200 dBm)
 * @param eNetworkType       Network Type (a/b/g)
 * @param prConfiguration    Network Parameter
 * @param eOpMode            Infra/Ad-Hoc
 * @param rSupportedRates    Supported basic rates
 * @param u2IELength         IE Length
 * @param pucIEBuf           Pointer to Information Elements(IEs)
 *
 * @return (none)
 */
/*----------------------------------------------------------------------------*/
void
nicAddScanResult(IN struct ADAPTER *prAdapter,
		 IN uint8_t rMacAddr[PARAM_MAC_ADDR_LEN],
		 IN struct PARAM_SSID *prSsid,
		 IN uint16_t u2CapInfo,
		 IN int32_t rRssi,
		 IN enum ENUM_PARAM_NETWORK_TYPE eNetworkType,
		 IN struct PARAM_802_11_CONFIG *prConfiguration,
		 IN enum ENUM_PARAM_OP_MODE eOpMode,
		 IN uint8_t rSupportedRates[PARAM_MAX_LEN_RATES_EX],
		 IN uint16_t u2IELength, IN uint8_t *pucIEBuf)
{
	u_int8_t bReplace;
	uint32_t i;
	uint32_t u4IdxWeakest = 0;
	int32_t rWeakestRssi;
	uint32_t u4BufferSize;
	/* Privicy setting 0: Open / 1: WEP/WPA/WPA2 enabled */
	uint32_t u4Privacy = u2CapInfo & CAP_INFO_PRIVACY ? 1 : 0;

	ASSERT(prAdapter);

	rWeakestRssi = (int32_t) INT_MAX;
	u4BufferSize = ARRAY_SIZE(
			       prAdapter->rWlanInfo.aucScanIEBuf);

	bReplace = FALSE;

	/* decide to replace or add */
	for (i = 0; i < prAdapter->rWlanInfo.u4ScanResultNum; i++) {
		uint8_t j;
		u_int8_t bUnMatch = TRUE;

		for (j = 0; j < KAL_AIS_NUM; j++) {
			struct PARAM_BSSID_EX *prCurrBssid;

			prCurrBssid = aisGetCurrBssId(prAdapter,
				j);
			if (EQUAL_MAC_ADDR
				(prAdapter->rWlanInfo.
				arScanResult[i].arMacAddress,
				prCurrBssid->arMacAddress)) {
				bUnMatch = FALSE;
				break;
			}
		}

		/* find weakest entry && not connected one */
		if (bUnMatch
		    && prAdapter->rWlanInfo.arScanResult[i].rRssi <
		    rWeakestRssi) {
			u4IdxWeakest = i;
			rWeakestRssi
				= prAdapter->rWlanInfo.arScanResult[i].rRssi;
		}

		if (prAdapter->rWlanInfo.arScanResult[i].eOpMode == eOpMode
		    &&
		    EQUAL_MAC_ADDR(&(prAdapter->rWlanInfo.
					arScanResult[i].arMacAddress), rMacAddr)
		    &&
		    (EQUAL_SSID
		     (prAdapter->rWlanInfo.arScanResult[i].rSsid.aucSsid,
		      prAdapter->rWlanInfo.arScanResult[i].rSsid.u4SsidLen,
		      prSsid->aucSsid, prSsid->u4SsidLen)
		     || prAdapter->rWlanInfo.arScanResult[i].rSsid.u4SsidLen ==
		     0)) {
			/* replace entry */
			bReplace = TRUE;

			/* free IE buffer then zero */
			nicFreeScanResultIE(prAdapter, i);
			kalMemZero(&(prAdapter->rWlanInfo.arScanResult[i]),
				   OFFSET_OF(struct PARAM_BSSID_EX, aucIEs));

			/* then fill buffer */
			prAdapter->rWlanInfo.arScanResult[i].u4Length =
				OFFSET_OF(struct PARAM_BSSID_EX, aucIEs)
					+ u2IELength;
			COPY_MAC_ADDR(
				prAdapter->rWlanInfo.arScanResult[i].
				arMacAddress,
				rMacAddr);
			COPY_SSID(
				prAdapter->rWlanInfo.arScanResult[i].rSsid.
				aucSsid,
				prAdapter->rWlanInfo.arScanResult[i].rSsid.
				u4SsidLen,
				prSsid->aucSsid, prSsid->u4SsidLen);
			prAdapter->rWlanInfo.arScanResult[i].u4Privacy
				= u4Privacy;
			prAdapter->rWlanInfo.arScanResult[i].rRssi = rRssi;
			prAdapter->rWlanInfo.arScanResult[i].eNetworkTypeInUse =
				eNetworkType;
			kalMemCopy(&
				(prAdapter->rWlanInfo.arScanResult[i].
				rConfiguration),
				prConfiguration,
				sizeof(struct PARAM_802_11_CONFIG));
			prAdapter->rWlanInfo.arScanResult[i].eOpMode = eOpMode;
			kalMemCopy((
				prAdapter->rWlanInfo.
				arScanResult[i].rSupportedRates),
				rSupportedRates, (sizeof(uint8_t) *
				PARAM_MAX_LEN_RATES_EX));
			prAdapter->rWlanInfo.arScanResult[i].u4IELength =
				(uint32_t) u2IELength;

			/* IE - allocate buffer and update pointer */
			if (u2IELength > 0) {
				if (ALIGN_4(u2IELength) +
				    prAdapter->rWlanInfo.u4ScanIEBufferUsage
						<= u4BufferSize) {
					kalMemCopy(&
						(prAdapter->rWlanInfo.
						aucScanIEBuf[prAdapter->
						rWlanInfo.
						u4ScanIEBufferUsage]),
						pucIEBuf,
						u2IELength);

					prAdapter->rWlanInfo.
					apucScanResultIEs[i]
						=	&(prAdapter->rWlanInfo.
						  aucScanIEBuf[prAdapter->
						  rWlanInfo.
						  u4ScanIEBufferUsage]);

					prAdapter->rWlanInfo.u4ScanIEBufferUsage
						+= ALIGN_4(u2IELength);
				} else {
					/* buffer is not enough */
					prAdapter->rWlanInfo.
					arScanResult[i].u4Length -= u2IELength;
					prAdapter->rWlanInfo.
					arScanResult[i].u4IELength = 0;
					prAdapter->rWlanInfo.
					apucScanResultIEs[i] = NULL;
				}
			} else {
				prAdapter->rWlanInfo.
				apucScanResultIEs[i] = NULL;
			}

			break;
		}
	}

	if (bReplace == FALSE) {
		if (prAdapter->rWlanInfo.u4ScanResultNum <
		    (CFG_MAX_NUM_BSS_LIST - 1)) {
			i = prAdapter->rWlanInfo.u4ScanResultNum;

			/* zero */
			kalMemZero(&(prAdapter->rWlanInfo.arScanResult[i]),
				   OFFSET_OF(struct PARAM_BSSID_EX, aucIEs));

			/* then fill buffer */
			prAdapter->rWlanInfo.arScanResult[i].u4Length =
				OFFSET_OF(struct PARAM_BSSID_EX, aucIEs)
					+ u2IELength;
			COPY_MAC_ADDR(
				prAdapter->rWlanInfo.arScanResult[i].
				arMacAddress,
				rMacAddr);
			COPY_SSID(
				prAdapter->rWlanInfo.arScanResult[i].
				rSsid.aucSsid,
				prAdapter->rWlanInfo.arScanResult[i].
				rSsid.u4SsidLen,
				prSsid->aucSsid, prSsid->u4SsidLen);
			prAdapter->rWlanInfo.arScanResult[i].
				u4Privacy = u4Privacy;
			prAdapter->rWlanInfo.arScanResult[i].rRssi = rRssi;
			prAdapter->rWlanInfo.arScanResult[i].eNetworkTypeInUse =
				eNetworkType;
			kalMemCopy(&
				(prAdapter->rWlanInfo.arScanResult[i].
				rConfiguration),
				prConfiguration,
				sizeof(struct PARAM_802_11_CONFIG));
			prAdapter->rWlanInfo.arScanResult[i].eOpMode = eOpMode;
			kalMemCopy((
				prAdapter->rWlanInfo.arScanResult[i].
				rSupportedRates),
				rSupportedRates, (sizeof(uint8_t) *
				PARAM_MAX_LEN_RATES_EX));
			prAdapter->rWlanInfo.arScanResult[i].u4IELength =
				(uint32_t) u2IELength;

			/* IE - allocate buffer and update pointer */
			if (u2IELength > 0) {
				if (ALIGN_4(u2IELength) +
				    prAdapter->rWlanInfo.u4ScanIEBufferUsage
							<= u4BufferSize) {
					kalMemCopy(&
						(prAdapter->rWlanInfo.
						aucScanIEBuf[prAdapter->
						rWlanInfo.
						u4ScanIEBufferUsage]),
						pucIEBuf,
						u2IELength);

					prAdapter->rWlanInfo.
						apucScanResultIEs[i]
						=	&(prAdapter->rWlanInfo.
						  aucScanIEBuf[prAdapter->
						  rWlanInfo.
						  u4ScanIEBufferUsage]);

					prAdapter->rWlanInfo.u4ScanIEBufferUsage
						+= ALIGN_4(u2IELength);
				} else {
					/* buffer is not enough */
					prAdapter->rWlanInfo.arScanResult[i].
					u4Length -= u2IELength;
					prAdapter->rWlanInfo.arScanResult[i].
					u4IELength = 0;
					prAdapter->rWlanInfo.
					apucScanResultIEs[i] = NULL;
				}
			} else {
				prAdapter->rWlanInfo.apucScanResultIEs[i]
					= NULL;
			}

			prAdapter->rWlanInfo.u4ScanResultNum++;
		} else if (rWeakestRssi != (int32_t) INT_MAX) {
			/* replace weakest one */
			i = u4IdxWeakest;

			/* free IE buffer then zero */
			nicFreeScanResultIE(prAdapter, i);
			kalMemZero(&(prAdapter->rWlanInfo.arScanResult[i]),
				   OFFSET_OF(struct PARAM_BSSID_EX, aucIEs));

			/* then fill buffer */
			prAdapter->rWlanInfo.arScanResult[i].u4Length =
				OFFSET_OF(struct PARAM_BSSID_EX, aucIEs)
					+ u2IELength;
			COPY_MAC_ADDR(
				prAdapter->rWlanInfo.arScanResult[i].
				arMacAddress,
				rMacAddr);
			COPY_SSID(
				prAdapter->rWlanInfo.arScanResult[i].
				rSsid.aucSsid,
				prAdapter->rWlanInfo.arScanResult[i].
				rSsid.u4SsidLen,
				prSsid->aucSsid, prSsid->u4SsidLen);
			prAdapter->rWlanInfo.arScanResult[i].u4Privacy
				= u4Privacy;
			prAdapter->rWlanInfo.arScanResult[i].rRssi = rRssi;
			prAdapter->rWlanInfo.arScanResult[i].eNetworkTypeInUse =
				eNetworkType;
			kalMemCopy(&(prAdapter->rWlanInfo.
				arScanResult[i].rConfiguration),
				prConfiguration,
				sizeof(struct PARAM_802_11_CONFIG));
			prAdapter->rWlanInfo.arScanResult[i].eOpMode = eOpMode;
			kalMemCopy((
				prAdapter->rWlanInfo.arScanResult[i].
				rSupportedRates),
				rSupportedRates, (sizeof(uint8_t) *
				PARAM_MAX_LEN_RATES_EX));
			prAdapter->rWlanInfo.arScanResult[i].u4IELength =
				(uint32_t) u2IELength;

			if (u2IELength > 0) {
				/* IE - allocate buffer and update pointer */
				if (ALIGN_4(u2IELength) +
				    prAdapter->rWlanInfo.u4ScanIEBufferUsage
							<= u4BufferSize) {
					kalMemCopy(&
						   (prAdapter->rWlanInfo.
						    aucScanIEBuf[
						    prAdapter->rWlanInfo.
						    u4ScanIEBufferUsage]),
						   pucIEBuf,
						   u2IELength);

					prAdapter->rWlanInfo.
					apucScanResultIEs[i]
						= &(prAdapter->rWlanInfo.
						  aucScanIEBuf[prAdapter->
						  rWlanInfo.
						  u4ScanIEBufferUsage]);

					prAdapter->rWlanInfo.u4ScanIEBufferUsage
						+= ALIGN_4(u2IELength);
				} else {
					/* buffer is not enough */
					prAdapter->rWlanInfo.arScanResult[i].
					u4Length -= u2IELength;
					prAdapter->rWlanInfo.arScanResult[i].
					u4IELength = 0;
					prAdapter->rWlanInfo.
					apucScanResultIEs[i] = NULL;
				}
			} else {
				prAdapter->rWlanInfo.apucScanResultIEs[i]
					= NULL;
			}
		}
	}
}

/*----------------------------------------------------------------------------*/
/*!
 * @brief This function is invoked to free IE buffer for dedicated scan result
 *
 * @param prAdapter          Pointer to the Adapter structure.
 * @param u4Idx              Index of Scan Result
 *
 * @return (none)
 */
/*----------------------------------------------------------------------------*/
void nicFreeScanResultIE(IN struct ADAPTER *prAdapter,
			 IN uint32_t u4Idx)
{
	uint32_t i;
	uint8_t *pucPivot, *pucMovePivot;
	uint32_t u4MoveSize, u4FreeSize, u4ReserveSize;

	ASSERT(prAdapter);
	ASSERT(u4Idx < CFG_MAX_NUM_BSS_LIST);

	if (prAdapter->rWlanInfo.arScanResult[u4Idx].u4IELength == 0
	    || prAdapter->rWlanInfo.apucScanResultIEs[u4Idx] == NULL) {
		return;
	}

	u4FreeSize = ALIGN_4(
		prAdapter->rWlanInfo.arScanResult[u4Idx].u4IELength);

	pucPivot = prAdapter->rWlanInfo.apucScanResultIEs[u4Idx];
	pucMovePivot = (uint8_t *) ((unsigned long) (
		prAdapter->rWlanInfo.apucScanResultIEs[u4Idx]) +
		u4FreeSize);

	u4ReserveSize = ((unsigned long) pucPivot) -
		(unsigned long) (&(prAdapter->rWlanInfo.aucScanIEBuf[0]));
	u4MoveSize = prAdapter->rWlanInfo.u4ScanIEBufferUsage -
		     u4ReserveSize - u4FreeSize;

	/* 1. rest of buffer to move forward */
	kalMemCopy(pucPivot, pucMovePivot, u4MoveSize);

	/* 1.1 modify pointers */
	for (i = 0; i < prAdapter->rWlanInfo.u4ScanResultNum; i++) {
		if (i != u4Idx) {
			if (prAdapter->rWlanInfo.apucScanResultIEs[i] >=
			    pucMovePivot) {
				prAdapter->rWlanInfo.apucScanResultIEs[i] =
					(uint8_t *) ((unsigned long) (
						prAdapter->rWlanInfo.
						apucScanResultIEs[i])
						- u4FreeSize);
			}
		}
	}

	/* 1.2 reset the freed one */
	prAdapter->rWlanInfo.arScanResult[u4Idx].u4IELength = 0;
	prAdapter->rWlanInfo.apucScanResultIEs[i] = NULL;

	/* 2. reduce IE buffer usage */
	prAdapter->rWlanInfo.u4ScanIEBufferUsage -= u4FreeSize;

}

/*----------------------------------------------------------------------------*/
/*!
 * @brief This function is to hack parameters for WLAN TABLE for
 *        fixed rate settings
 *
 * @param prAdapter          Pointer to the Adapter structure.
 * @param eRateSetting
 * @param pu2DesiredNonHTRateSet,
 * @param pu2BSSBasicRateSet,
 * @param pucMcsSet
 * @param pucSupMcs32
 * @param pu2HtCapInfo
 *
 * @return WLAN_STATUS_SUCCESS
 */
/*----------------------------------------------------------------------------*/
uint32_t
nicUpdateRateParams(IN struct ADAPTER *prAdapter,
		    IN enum ENUM_REGISTRY_FIXED_RATE eRateSetting,
		    IN uint8_t *pucDesiredPhyTypeSet,
		    IN uint16_t *pu2DesiredNonHTRateSet,
		    IN uint16_t *pu2BSSBasicRateSet,
		    IN uint8_t *pucMcsSet, IN uint8_t *pucSupMcs32,
		    IN uint16_t *pu2HtCapInfo)
{
	ASSERT(prAdapter);
	ASSERT(eRateSetting > FIXED_RATE_NONE
	       && eRateSetting < FIXED_RATE_NUM);

	switch (prAdapter->rWifiVar.eRateSetting) {
	case FIXED_RATE_1M:
		*pucDesiredPhyTypeSet = PHY_TYPE_BIT_HR_DSSS;
		*pu2DesiredNonHTRateSet = RATE_SET_BIT_1M;
		*pu2BSSBasicRateSet = RATE_SET_BIT_1M;
		*pucMcsSet = 0;
		*pucSupMcs32 = 0;
		*pu2HtCapInfo = 0;
		break;

	case FIXED_RATE_2M:
		*pucDesiredPhyTypeSet = PHY_TYPE_BIT_HR_DSSS;
		*pu2DesiredNonHTRateSet = RATE_SET_BIT_2M;
		*pu2BSSBasicRateSet = RATE_SET_BIT_2M;
		*pucMcsSet = 0;
		*pucSupMcs32 = 0;
		*pu2HtCapInfo = 0;
		break;

	case FIXED_RATE_5_5M:
		*pucDesiredPhyTypeSet = PHY_TYPE_BIT_HR_DSSS;
		*pu2DesiredNonHTRateSet = RATE_SET_BIT_5_5M;
		*pu2BSSBasicRateSet = RATE_SET_BIT_5_5M;
		*pucMcsSet = 0;
		*pucSupMcs32 = 0;
		*pu2HtCapInfo = 0;
		break;

	case FIXED_RATE_11M:
		*pucDesiredPhyTypeSet = PHY_TYPE_BIT_HR_DSSS;
		*pu2DesiredNonHTRateSet = RATE_SET_BIT_11M;
		*pu2BSSBasicRateSet = RATE_SET_BIT_11M;
		*pucMcsSet = 0;
		*pucSupMcs32 = 0;
		*pu2HtCapInfo = 0;
		break;

	case FIXED_RATE_6M:
		if ((*pucDesiredPhyTypeSet) & PHY_TYPE_BIT_ERP)
			*pucDesiredPhyTypeSet = PHY_TYPE_BIT_ERP;
		else if ((*pucDesiredPhyTypeSet) & PHY_TYPE_BIT_OFDM)
			*pucDesiredPhyTypeSet = PHY_TYPE_BIT_OFDM;

		*pu2DesiredNonHTRateSet = RATE_SET_BIT_6M;
		*pu2BSSBasicRateSet = RATE_SET_BIT_6M;
		*pucMcsSet = 0;
		*pucSupMcs32 = 0;
		*pu2HtCapInfo = 0;
		break;

	case FIXED_RATE_9M:
		if ((*pucDesiredPhyTypeSet) & PHY_TYPE_BIT_ERP)
			*pucDesiredPhyTypeSet = PHY_TYPE_BIT_ERP;
		else if ((*pucDesiredPhyTypeSet) & PHY_TYPE_BIT_OFDM)
			*pucDesiredPhyTypeSet = PHY_TYPE_BIT_OFDM;

		*pu2DesiredNonHTRateSet = RATE_SET_BIT_9M;
		*pu2BSSBasicRateSet = RATE_SET_BIT_9M;
		*pucMcsSet = 0;
		*pucSupMcs32 = 0;
		*pu2HtCapInfo = 0;
		break;

	case FIXED_RATE_12M:
		if ((*pucDesiredPhyTypeSet) & PHY_TYPE_BIT_ERP)
			*pucDesiredPhyTypeSet = PHY_TYPE_BIT_ERP;
		else if ((*pucDesiredPhyTypeSet) & PHY_TYPE_BIT_OFDM)
			*pucDesiredPhyTypeSet = PHY_TYPE_BIT_OFDM;

		*pu2DesiredNonHTRateSet = RATE_SET_BIT_12M;
		*pu2BSSBasicRateSet = RATE_SET_BIT_12M;
		*pucMcsSet = 0;
		*pucSupMcs32 = 0;
		*pu2HtCapInfo = 0;
		break;

	case FIXED_RATE_18M:
		if ((*pucDesiredPhyTypeSet) & PHY_TYPE_BIT_ERP)
			*pucDesiredPhyTypeSet = PHY_TYPE_BIT_ERP;
		else if ((*pucDesiredPhyTypeSet) & PHY_TYPE_BIT_OFDM)
			*pucDesiredPhyTypeSet = PHY_TYPE_BIT_OFDM;

		*pu2DesiredNonHTRateSet = RATE_SET_BIT_18M;
		*pu2BSSBasicRateSet = RATE_SET_BIT_18M;
		*pucMcsSet = 0;
		*pucSupMcs32 = 0;
		*pu2HtCapInfo = 0;
		break;

	case FIXED_RATE_24M:
		if ((*pucDesiredPhyTypeSet) & PHY_TYPE_BIT_ERP)
			*pucDesiredPhyTypeSet = PHY_TYPE_BIT_ERP;
		else if ((*pucDesiredPhyTypeSet) & PHY_TYPE_BIT_OFDM)
			*pucDesiredPhyTypeSet = PHY_TYPE_BIT_OFDM;

		*pu2DesiredNonHTRateSet = RATE_SET_BIT_24M;
		*pu2BSSBasicRateSet = RATE_SET_BIT_24M;
		*pucMcsSet = 0;
		*pucSupMcs32 = 0;
		*pu2HtCapInfo = 0;
		break;

	case FIXED_RATE_36M:
		if ((*pucDesiredPhyTypeSet) & PHY_TYPE_BIT_ERP)
			*pucDesiredPhyTypeSet = PHY_TYPE_BIT_ERP;
		else if ((*pucDesiredPhyTypeSet) & PHY_TYPE_BIT_OFDM)
			*pucDesiredPhyTypeSet = PHY_TYPE_BIT_OFDM;

		*pu2DesiredNonHTRateSet = RATE_SET_BIT_36M;
		*pu2BSSBasicRateSet = RATE_SET_BIT_36M;
		*pucMcsSet = 0;
		*pucSupMcs32 = 0;
		*pu2HtCapInfo = 0;
		break;

	case FIXED_RATE_48M:
		if ((*pucDesiredPhyTypeSet) & PHY_TYPE_BIT_ERP)
			*pucDesiredPhyTypeSet = PHY_TYPE_BIT_ERP;
		else if ((*pucDesiredPhyTypeSet) & PHY_TYPE_BIT_OFDM)
			*pucDesiredPhyTypeSet = PHY_TYPE_BIT_OFDM;

		*pu2DesiredNonHTRateSet = RATE_SET_BIT_48M;
		*pu2BSSBasicRateSet = RATE_SET_BIT_48M;
		*pucMcsSet = 0;
		*pucSupMcs32 = 0;
		*pu2HtCapInfo = 0;
		break;

	case FIXED_RATE_54M:
		if ((*pucDesiredPhyTypeSet) & PHY_TYPE_BIT_ERP)
			*pucDesiredPhyTypeSet = PHY_TYPE_BIT_ERP;
		else if ((*pucDesiredPhyTypeSet) & PHY_TYPE_BIT_OFDM)
			*pucDesiredPhyTypeSet = PHY_TYPE_BIT_OFDM;

		*pu2DesiredNonHTRateSet = RATE_SET_BIT_54M;
		*pu2BSSBasicRateSet = RATE_SET_BIT_54M;
		*pucMcsSet = 0;
		*pucSupMcs32 = 0;
		*pu2HtCapInfo = 0;
		break;

	case FIXED_RATE_MCS0_20M_800NS:
		*pucDesiredPhyTypeSet = PHY_TYPE_BIT_HT;
		*pu2DesiredNonHTRateSet = RATE_SET_BIT_HT_PHY;
		*pu2BSSBasicRateSet = RATE_SET_BIT_HT_PHY;
		*pucMcsSet = BIT(HT_RATE_MCS0_INDEX - 1);
		*pucSupMcs32 = 0;
		(*pu2HtCapInfo) &= ~(HT_CAP_INFO_SUP_CHNL_WIDTH
			| HT_CAP_INFO_SHORT_GI_20M | HT_CAP_INFO_SHORT_GI_40M |
			HT_CAP_INFO_HT_GF);
		break;

	case FIXED_RATE_MCS1_20M_800NS:
		*pucDesiredPhyTypeSet = PHY_TYPE_BIT_HT;
		*pu2DesiredNonHTRateSet = RATE_SET_BIT_HT_PHY;
		*pu2BSSBasicRateSet = RATE_SET_BIT_HT_PHY;
		*pucMcsSet = BIT(HT_RATE_MCS1_INDEX - 1);
		*pucSupMcs32 = 0;
		(*pu2HtCapInfo) &= ~(HT_CAP_INFO_SUP_CHNL_WIDTH
			| HT_CAP_INFO_SHORT_GI_20M | HT_CAP_INFO_SHORT_GI_40M |
			HT_CAP_INFO_HT_GF);
		break;

	case FIXED_RATE_MCS2_20M_800NS:
		*pucDesiredPhyTypeSet = PHY_TYPE_BIT_HT;
		*pu2DesiredNonHTRateSet = RATE_SET_BIT_HT_PHY;
		*pu2BSSBasicRateSet = RATE_SET_BIT_HT_PHY;
		*pucMcsSet = BIT(HT_RATE_MCS2_INDEX - 1);
		*pucSupMcs32 = 0;
		(*pu2HtCapInfo) &= ~(HT_CAP_INFO_SUP_CHNL_WIDTH
			| HT_CAP_INFO_SHORT_GI_20M | HT_CAP_INFO_SHORT_GI_40M |
			HT_CAP_INFO_HT_GF);
		break;

	case FIXED_RATE_MCS3_20M_800NS:
		*pucDesiredPhyTypeSet = PHY_TYPE_BIT_HT;
		*pu2DesiredNonHTRateSet = RATE_SET_BIT_HT_PHY;
		*pu2BSSBasicRateSet = RATE_SET_BIT_HT_PHY;
		*pucMcsSet = BIT(HT_RATE_MCS3_INDEX - 1);
		*pucSupMcs32 = 0;
		(*pu2HtCapInfo) &= ~(HT_CAP_INFO_SUP_CHNL_WIDTH
			| HT_CAP_INFO_SHORT_GI_20M | HT_CAP_INFO_SHORT_GI_40M |
			HT_CAP_INFO_HT_GF);
		break;

	case FIXED_RATE_MCS4_20M_800NS:
		*pucDesiredPhyTypeSet = PHY_TYPE_BIT_HT;
		*pu2DesiredNonHTRateSet = RATE_SET_BIT_HT_PHY;
		*pu2BSSBasicRateSet = RATE_SET_BIT_HT_PHY;
		*pucMcsSet = BIT(HT_RATE_MCS4_INDEX - 1);
		*pucSupMcs32 = 0;
		(*pu2HtCapInfo) &= ~(HT_CAP_INFO_SUP_CHNL_WIDTH
			| HT_CAP_INFO_SHORT_GI_20M | HT_CAP_INFO_SHORT_GI_40M |
			HT_CAP_INFO_HT_GF);
		break;

	case FIXED_RATE_MCS5_20M_800NS:
		*pucDesiredPhyTypeSet = PHY_TYPE_BIT_HT;
		*pu2DesiredNonHTRateSet = RATE_SET_BIT_HT_PHY;
		*pu2BSSBasicRateSet = RATE_SET_BIT_HT_PHY;
		*pucMcsSet = BIT(HT_RATE_MCS5_INDEX - 1);
		*pucSupMcs32 = 0;
		(*pu2HtCapInfo) &= ~(HT_CAP_INFO_SUP_CHNL_WIDTH
			| HT_CAP_INFO_SHORT_GI_20M | HT_CAP_INFO_SHORT_GI_40M |
			HT_CAP_INFO_HT_GF);
		break;

	case FIXED_RATE_MCS6_20M_800NS:
		*pucDesiredPhyTypeSet = PHY_TYPE_BIT_HT;
		*pu2DesiredNonHTRateSet = RATE_SET_BIT_HT_PHY;
		*pu2BSSBasicRateSet = RATE_SET_BIT_HT_PHY;
		*pucMcsSet = BIT(HT_RATE_MCS6_INDEX - 1);
		*pucSupMcs32 = 0;
		(*pu2HtCapInfo) &= ~(HT_CAP_INFO_SUP_CHNL_WIDTH
			| HT_CAP_INFO_SHORT_GI_20M | HT_CAP_INFO_SHORT_GI_40M |
			HT_CAP_INFO_HT_GF);
		break;

	case FIXED_RATE_MCS7_20M_800NS:
		*pucDesiredPhyTypeSet = PHY_TYPE_BIT_HT;
		*pu2DesiredNonHTRateSet = RATE_SET_BIT_HT_PHY;
		*pu2BSSBasicRateSet = RATE_SET_BIT_HT_PHY;
		*pucMcsSet = BIT(HT_RATE_MCS7_INDEX - 1);
		*pucSupMcs32 = 0;
		(*pu2HtCapInfo) &= ~(HT_CAP_INFO_SUP_CHNL_WIDTH
			| HT_CAP_INFO_SHORT_GI_20M | HT_CAP_INFO_SHORT_GI_40M |
			HT_CAP_INFO_HT_GF);
		break;

	case FIXED_RATE_MCS0_20M_400NS:
		*pucDesiredPhyTypeSet = PHY_TYPE_BIT_HT;
		*pu2DesiredNonHTRateSet = RATE_SET_BIT_HT_PHY;
		*pu2BSSBasicRateSet = RATE_SET_BIT_HT_PHY;
		*pucMcsSet = BIT(HT_RATE_MCS0_INDEX - 1);
		*pucSupMcs32 = 0;
		(*pu2HtCapInfo) &= ~(HT_CAP_INFO_SUP_CHNL_WIDTH |
			HT_CAP_INFO_SHORT_GI_40M | HT_CAP_INFO_HT_GF);
		(*pu2HtCapInfo) |= HT_CAP_INFO_SHORT_GI_20M;
		break;

	case FIXED_RATE_MCS1_20M_400NS:
		*pucDesiredPhyTypeSet = PHY_TYPE_BIT_HT;
		*pu2DesiredNonHTRateSet = RATE_SET_BIT_HT_PHY;
		*pu2BSSBasicRateSet = RATE_SET_BIT_HT_PHY;
		*pucMcsSet = BIT(HT_RATE_MCS1_INDEX - 1);
		*pucSupMcs32 = 0;
		(*pu2HtCapInfo) &= ~(HT_CAP_INFO_SUP_CHNL_WIDTH |
			HT_CAP_INFO_SHORT_GI_40M | HT_CAP_INFO_HT_GF);
		(*pu2HtCapInfo) |= HT_CAP_INFO_SHORT_GI_20M;
		break;

	case FIXED_RATE_MCS2_20M_400NS:
		*pucDesiredPhyTypeSet = PHY_TYPE_BIT_HT;
		*pu2DesiredNonHTRateSet = RATE_SET_BIT_HT_PHY;
		*pu2BSSBasicRateSet = RATE_SET_BIT_HT_PHY;
		*pucMcsSet = BIT(HT_RATE_MCS2_INDEX - 1);
		*pucSupMcs32 = 0;
		(*pu2HtCapInfo) &= ~(HT_CAP_INFO_SUP_CHNL_WIDTH |
			HT_CAP_INFO_SHORT_GI_40M | HT_CAP_INFO_HT_GF);
		(*pu2HtCapInfo) |= HT_CAP_INFO_SHORT_GI_20M;
		break;

	case FIXED_RATE_MCS3_20M_400NS:
		*pucDesiredPhyTypeSet = PHY_TYPE_BIT_HT;
		*pu2DesiredNonHTRateSet = RATE_SET_BIT_HT_PHY;
		*pu2BSSBasicRateSet = RATE_SET_BIT_HT_PHY;
		*pucMcsSet = BIT(HT_RATE_MCS3_INDEX - 1);
		*pucSupMcs32 = 0;
		(*pu2HtCapInfo) &= ~(HT_CAP_INFO_SUP_CHNL_WIDTH |
			HT_CAP_INFO_SHORT_GI_40M | HT_CAP_INFO_HT_GF);
		(*pu2HtCapInfo) |= HT_CAP_INFO_SHORT_GI_20M;
		break;

	case FIXED_RATE_MCS4_20M_400NS:
		*pucDesiredPhyTypeSet = PHY_TYPE_BIT_HT;
		*pu2DesiredNonHTRateSet = RATE_SET_BIT_HT_PHY;
		*pu2BSSBasicRateSet = RATE_SET_BIT_HT_PHY;
		*pucMcsSet = BIT(HT_RATE_MCS4_INDEX - 1);
		*pucSupMcs32 = 0;
		(*pu2HtCapInfo) &= ~(HT_CAP_INFO_SUP_CHNL_WIDTH |
			HT_CAP_INFO_SHORT_GI_40M | HT_CAP_INFO_HT_GF);
		(*pu2HtCapInfo) |= HT_CAP_INFO_SHORT_GI_20M;
		break;

	case FIXED_RATE_MCS5_20M_400NS:
		*pucDesiredPhyTypeSet = PHY_TYPE_BIT_HT;
		*pu2DesiredNonHTRateSet = RATE_SET_BIT_HT_PHY;
		*pu2BSSBasicRateSet = RATE_SET_BIT_HT_PHY;
		*pucMcsSet = BIT(HT_RATE_MCS5_INDEX - 1);
		*pucSupMcs32 = 0;
		(*pu2HtCapInfo) &= ~(HT_CAP_INFO_SUP_CHNL_WIDTH |
			HT_CAP_INFO_SHORT_GI_40M | HT_CAP_INFO_HT_GF);
		(*pu2HtCapInfo) |= HT_CAP_INFO_SHORT_GI_20M;
		break;

	case FIXED_RATE_MCS6_20M_400NS:
		*pucDesiredPhyTypeSet = PHY_TYPE_BIT_HT;
		*pu2DesiredNonHTRateSet = RATE_SET_BIT_HT_PHY;
		*pu2BSSBasicRateSet = RATE_SET_BIT_HT_PHY;
		*pucMcsSet = BIT(HT_RATE_MCS6_INDEX - 1);
		*pucSupMcs32 = 0;
		(*pu2HtCapInfo) &= ~(HT_CAP_INFO_SUP_CHNL_WIDTH |
			HT_CAP_INFO_SHORT_GI_40M | HT_CAP_INFO_HT_GF);
		(*pu2HtCapInfo) |= HT_CAP_INFO_SHORT_GI_20M;
		break;

	case FIXED_RATE_MCS7_20M_400NS:
		*pucDesiredPhyTypeSet = PHY_TYPE_BIT_HT;
		*pu2DesiredNonHTRateSet = RATE_SET_BIT_HT_PHY;
		*pu2BSSBasicRateSet = RATE_SET_BIT_HT_PHY;
		*pucMcsSet = BIT(HT_RATE_MCS7_INDEX - 1);
		*pucSupMcs32 = 0;
		(*pu2HtCapInfo) &= ~(HT_CAP_INFO_SUP_CHNL_WIDTH |
			HT_CAP_INFO_SHORT_GI_40M | HT_CAP_INFO_HT_GF);
		(*pu2HtCapInfo) |= HT_CAP_INFO_SHORT_GI_20M;
		break;

	case FIXED_RATE_MCS0_40M_800NS:
		*pucDesiredPhyTypeSet = PHY_TYPE_BIT_HT;
		*pu2DesiredNonHTRateSet = RATE_SET_BIT_HT_PHY;
		*pu2BSSBasicRateSet = RATE_SET_BIT_HT_PHY;
		*pucMcsSet = BIT(HT_RATE_MCS0_INDEX - 1);
		*pucSupMcs32 = 0;
		(*pu2HtCapInfo) &= ~(HT_CAP_INFO_SHORT_GI_20M |
			HT_CAP_INFO_SHORT_GI_40M | HT_CAP_INFO_HT_GF);
		(*pu2HtCapInfo) |= HT_CAP_INFO_SUP_CHNL_WIDTH;
		break;

	case FIXED_RATE_MCS1_40M_800NS:
		*pucDesiredPhyTypeSet = PHY_TYPE_BIT_HT;
		*pu2DesiredNonHTRateSet = RATE_SET_BIT_HT_PHY;
		*pu2BSSBasicRateSet = RATE_SET_BIT_HT_PHY;
		*pucMcsSet = BIT(HT_RATE_MCS1_INDEX - 1);
		*pucSupMcs32 = 0;
		(*pu2HtCapInfo) &= ~(HT_CAP_INFO_SHORT_GI_20M |
			HT_CAP_INFO_SHORT_GI_40M | HT_CAP_INFO_HT_GF);
		(*pu2HtCapInfo) |= HT_CAP_INFO_SUP_CHNL_WIDTH;
		break;

	case FIXED_RATE_MCS2_40M_800NS:
		*pucDesiredPhyTypeSet = PHY_TYPE_BIT_HT;
		*pu2DesiredNonHTRateSet = RATE_SET_BIT_HT_PHY;
		*pu2BSSBasicRateSet = RATE_SET_BIT_HT_PHY;
		*pucMcsSet = BIT(HT_RATE_MCS2_INDEX - 1);
		*pucSupMcs32 = 0;
		(*pu2HtCapInfo) &= ~(HT_CAP_INFO_SHORT_GI_20M |
			HT_CAP_INFO_SHORT_GI_40M | HT_CAP_INFO_HT_GF);
		(*pu2HtCapInfo) |= HT_CAP_INFO_SUP_CHNL_WIDTH;
		break;

	case FIXED_RATE_MCS3_40M_800NS:
		*pucDesiredPhyTypeSet = PHY_TYPE_BIT_HT;
		*pu2DesiredNonHTRateSet = RATE_SET_BIT_HT_PHY;
		*pu2BSSBasicRateSet = RATE_SET_BIT_HT_PHY;
		*pucMcsSet = BIT(HT_RATE_MCS3_INDEX - 1);
		*pucSupMcs32 = 0;
		(*pu2HtCapInfo) &= ~(HT_CAP_INFO_SHORT_GI_20M |
			HT_CAP_INFO_SHORT_GI_40M | HT_CAP_INFO_HT_GF);
		(*pu2HtCapInfo) |= HT_CAP_INFO_SUP_CHNL_WIDTH;
		break;

	case FIXED_RATE_MCS4_40M_800NS:
		*pucDesiredPhyTypeSet = PHY_TYPE_BIT_HT;
		*pu2DesiredNonHTRateSet = RATE_SET_BIT_HT_PHY;
		*pu2BSSBasicRateSet = RATE_SET_BIT_HT_PHY;
		*pucMcsSet = BIT(HT_RATE_MCS4_INDEX - 1);
		*pucSupMcs32 = 0;
		(*pu2HtCapInfo) &= ~(HT_CAP_INFO_SHORT_GI_20M |
			HT_CAP_INFO_SHORT_GI_40M | HT_CAP_INFO_HT_GF);
		(*pu2HtCapInfo) |= HT_CAP_INFO_SUP_CHNL_WIDTH;
		break;

	case FIXED_RATE_MCS5_40M_800NS:
		*pucDesiredPhyTypeSet = PHY_TYPE_BIT_HT;
		*pu2DesiredNonHTRateSet = RATE_SET_BIT_HT_PHY;
		*pu2BSSBasicRateSet = RATE_SET_BIT_HT_PHY;
		*pucMcsSet = BIT(HT_RATE_MCS5_INDEX - 1);
		*pucSupMcs32 = 0;
		(*pu2HtCapInfo) &= ~(HT_CAP_INFO_SHORT_GI_20M |
			HT_CAP_INFO_SHORT_GI_40M | HT_CAP_INFO_HT_GF);
		(*pu2HtCapInfo) |= HT_CAP_INFO_SUP_CHNL_WIDTH;
		break;

	case FIXED_RATE_MCS6_40M_800NS:
		*pucDesiredPhyTypeSet = PHY_TYPE_BIT_HT;
		*pu2DesiredNonHTRateSet = RATE_SET_BIT_HT_PHY;
		*pu2BSSBasicRateSet = RATE_SET_BIT_HT_PHY;
		*pucMcsSet = BIT(HT_RATE_MCS6_INDEX - 1);
		*pucSupMcs32 = 0;
		(*pu2HtCapInfo) &= ~(HT_CAP_INFO_SHORT_GI_20M |
			HT_CAP_INFO_SHORT_GI_40M | HT_CAP_INFO_HT_GF);
		(*pu2HtCapInfo) |= HT_CAP_INFO_SUP_CHNL_WIDTH;
		break;

	case FIXED_RATE_MCS7_40M_800NS:
		*pucDesiredPhyTypeSet = PHY_TYPE_BIT_HT;
		*pu2DesiredNonHTRateSet = RATE_SET_BIT_HT_PHY;
		*pu2BSSBasicRateSet = RATE_SET_BIT_HT_PHY;
		*pucMcsSet = BIT(HT_RATE_MCS7_INDEX - 1);
		*pucSupMcs32 = 0;
		(*pu2HtCapInfo) &= ~(HT_CAP_INFO_SHORT_GI_20M |
			HT_CAP_INFO_SHORT_GI_40M | HT_CAP_INFO_HT_GF);
		(*pu2HtCapInfo) |= HT_CAP_INFO_SUP_CHNL_WIDTH;
		break;

	case FIXED_RATE_MCS32_800NS:
		*pucDesiredPhyTypeSet = PHY_TYPE_BIT_HT;
		*pu2DesiredNonHTRateSet = RATE_SET_BIT_HT_PHY;
		*pu2BSSBasicRateSet = RATE_SET_BIT_HT_PHY;
		*pucMcsSet = 0;
		*pucSupMcs32 = 1;
		(*pu2HtCapInfo) &= ~(HT_CAP_INFO_SHORT_GI_20M |
			HT_CAP_INFO_SHORT_GI_40M | HT_CAP_INFO_HT_GF);
		(*pu2HtCapInfo) |= HT_CAP_INFO_SUP_CHNL_WIDTH;
		break;

	case FIXED_RATE_MCS0_40M_400NS:
		*pucDesiredPhyTypeSet = PHY_TYPE_BIT_HT;
		*pu2DesiredNonHTRateSet = RATE_SET_BIT_HT_PHY;
		*pu2BSSBasicRateSet = RATE_SET_BIT_HT_PHY;
		*pucMcsSet = BIT(HT_RATE_MCS0_INDEX - 1);
		*pucSupMcs32 = 0;
		(*pu2HtCapInfo) &= ~(HT_CAP_INFO_SHORT_GI_20M |
				     HT_CAP_INFO_HT_GF);
		(*pu2HtCapInfo) |= (HT_CAP_INFO_SUP_CHNL_WIDTH |
				    HT_CAP_INFO_SHORT_GI_40M);
		break;

	case FIXED_RATE_MCS1_40M_400NS:
		*pucDesiredPhyTypeSet = PHY_TYPE_BIT_HT;
		*pu2DesiredNonHTRateSet = RATE_SET_BIT_HT_PHY;
		*pu2BSSBasicRateSet = RATE_SET_BIT_HT_PHY;
		*pucMcsSet = BIT(HT_RATE_MCS1_INDEX - 1);
		*pucSupMcs32 = 0;
		(*pu2HtCapInfo) &= ~(HT_CAP_INFO_SHORT_GI_20M |
				     HT_CAP_INFO_HT_GF);
		(*pu2HtCapInfo) |= (HT_CAP_INFO_SUP_CHNL_WIDTH |
				    HT_CAP_INFO_SHORT_GI_40M);
		break;

	case FIXED_RATE_MCS2_40M_400NS:
		*pucDesiredPhyTypeSet = PHY_TYPE_BIT_HT;
		*pu2DesiredNonHTRateSet = RATE_SET_BIT_HT_PHY;
		*pu2BSSBasicRateSet = RATE_SET_BIT_HT_PHY;
		*pucMcsSet = BIT(HT_RATE_MCS2_INDEX - 1);
		*pucSupMcs32 = 0;
		(*pu2HtCapInfo) &= ~(HT_CAP_INFO_SHORT_GI_20M |
				     HT_CAP_INFO_HT_GF);
		(*pu2HtCapInfo) |= (HT_CAP_INFO_SUP_CHNL_WIDTH |
				    HT_CAP_INFO_SHORT_GI_40M);
		break;

	case FIXED_RATE_MCS3_40M_400NS:
		*pucDesiredPhyTypeSet = PHY_TYPE_BIT_HT;
		*pu2DesiredNonHTRateSet = RATE_SET_BIT_HT_PHY;
		*pu2BSSBasicRateSet = RATE_SET_BIT_HT_PHY;
		*pucMcsSet = BIT(HT_RATE_MCS3_INDEX - 1);
		*pucSupMcs32 = 0;
		(*pu2HtCapInfo) &= ~(HT_CAP_INFO_SHORT_GI_20M |
				     HT_CAP_INFO_HT_GF);
		(*pu2HtCapInfo) |= (HT_CAP_INFO_SUP_CHNL_WIDTH |
				    HT_CAP_INFO_SHORT_GI_40M);
		break;

	case FIXED_RATE_MCS4_40M_400NS:
		*pucDesiredPhyTypeSet = PHY_TYPE_BIT_HT;
		*pu2DesiredNonHTRateSet = RATE_SET_BIT_HT_PHY;
		*pu2BSSBasicRateSet = RATE_SET_BIT_HT_PHY;
		*pucMcsSet = BIT(HT_RATE_MCS4_INDEX - 1);
		*pucSupMcs32 = 0;
		(*pu2HtCapInfo) &= ~(HT_CAP_INFO_SHORT_GI_20M |
				     HT_CAP_INFO_HT_GF);
		(*pu2HtCapInfo) |= (HT_CAP_INFO_SUP_CHNL_WIDTH |
				    HT_CAP_INFO_SHORT_GI_40M);
		break;

	case FIXED_RATE_MCS5_40M_400NS:
		*pucDesiredPhyTypeSet = PHY_TYPE_BIT_HT;
		*pu2DesiredNonHTRateSet = RATE_SET_BIT_HT_PHY;
		*pu2BSSBasicRateSet = RATE_SET_BIT_HT_PHY;
		*pucMcsSet = BIT(HT_RATE_MCS5_INDEX - 1);
		*pucSupMcs32 = 0;
		(*pu2HtCapInfo) &= ~(HT_CAP_INFO_SHORT_GI_20M |
				     HT_CAP_INFO_HT_GF);
		(*pu2HtCapInfo) |= (HT_CAP_INFO_SUP_CHNL_WIDTH |
				    HT_CAP_INFO_SHORT_GI_40M);
		break;

	case FIXED_RATE_MCS6_40M_400NS:
		*pucDesiredPhyTypeSet = PHY_TYPE_BIT_HT;
		*pu2DesiredNonHTRateSet = RATE_SET_BIT_HT_PHY;
		*pu2BSSBasicRateSet = RATE_SET_BIT_HT_PHY;
		*pucMcsSet = BIT(HT_RATE_MCS6_INDEX - 1);
		*pucSupMcs32 = 0;
		(*pu2HtCapInfo) &= ~(HT_CAP_INFO_SHORT_GI_20M |
				     HT_CAP_INFO_HT_GF);
		(*pu2HtCapInfo) |= (HT_CAP_INFO_SUP_CHNL_WIDTH |
				    HT_CAP_INFO_SHORT_GI_40M);
		break;

	case FIXED_RATE_MCS7_40M_400NS:
		*pucDesiredPhyTypeSet = PHY_TYPE_BIT_HT;
		*pu2DesiredNonHTRateSet = RATE_SET_BIT_HT_PHY;
		*pu2BSSBasicRateSet = RATE_SET_BIT_HT_PHY;
		*pucMcsSet = BIT(HT_RATE_MCS7_INDEX - 1);
		*pucSupMcs32 = 0;
		(*pu2HtCapInfo) &= ~(HT_CAP_INFO_SHORT_GI_20M |
				     HT_CAP_INFO_HT_GF);
		(*pu2HtCapInfo) |= (HT_CAP_INFO_SUP_CHNL_WIDTH |
				    HT_CAP_INFO_SHORT_GI_40M);
		break;

	case FIXED_RATE_MCS32_400NS:
		*pucDesiredPhyTypeSet = PHY_TYPE_BIT_HT;
		*pu2DesiredNonHTRateSet = RATE_SET_BIT_HT_PHY;
		*pu2BSSBasicRateSet = RATE_SET_BIT_HT_PHY;
		*pucMcsSet = 0;
		*pucSupMcs32 = 1;
		(*pu2HtCapInfo) &= ~(HT_CAP_INFO_SHORT_GI_20M |
				     HT_CAP_INFO_HT_GF);
		(*pu2HtCapInfo) |= (HT_CAP_INFO_SUP_CHNL_WIDTH |
				    HT_CAP_INFO_SHORT_GI_40M);
		break;

	default:
		ASSERT(0);
	}

	return WLAN_STATUS_SUCCESS;
}

/*----------------------------------------------------------------------------*/
/*!
 * @brief This utility function is used to write the register
 *
 * @param u4Address         Register address
 *        u4Value           the value to be written
 *
 * @retval WLAN_STATUS_SUCCESS
 *         WLAN_STATUS_FAILURE
 */
/*----------------------------------------------------------------------------*/

uint32_t nicWriteMcr(IN struct ADAPTER *prAdapter,
		     IN uint32_t u4Address, IN uint32_t u4Value)
{
	struct CMD_ACCESS_REG rCmdAccessReg;

	rCmdAccessReg.u4Address = u4Address;
	rCmdAccessReg.u4Data = u4Value;

	return wlanSendSetQueryCmd(prAdapter,
				   CMD_ID_ACCESS_REG,
				   TRUE,
				   FALSE,
				   FALSE, NULL, NULL,
				   sizeof(struct CMD_ACCESS_REG),
				   (uint8_t *) &rCmdAccessReg, NULL, 0);

}

/*----------------------------------------------------------------------------*/
/*!
 * @brief This utility function is used to modify the auto rate parameters
 *
 * @param u4ArSysParam0  see description below
 *        u4ArSysParam1
 *        u4ArSysParam2
 *        u4ArSysParam3
 *
 *
 * @retval WLAN_STATUS_SUCCESS
 *         WLAN_STATUS_FAILURE
 *
 * @note
 *   ArSysParam0[0:3] -> auto rate version (0:disable 1:version1 2:version2)
 *   ArSysParam0[4:5]-> auto bw version (0:disable 1:version1 2:version2)
 *   ArSysParam0[6:7]-> auto gi version (0:disable 1:version1 2:version2)
 *   ArSysParam0[8:15]-> HT rate clear mask
 *   ArSysParam0[16:31]-> Legacy rate clear mask
 *   ArSysParam1[0:7]-> Auto Rate check weighting window
 *   ArSysParam1[8:15]-> Auto Rate v1 Force Rate down
 *   ArSysParam1[16:23]-> Auto Rate v1 PerH
 *   ArSysParam1[24:31]-> Auto Rate v1 PerL
 *
 *   Examples
 *   ArSysParam0 = 1,
 *   Enable auto rate version 1
 *
 *   ArSysParam0 = 983041,
 *   Enable auto rate version 1
 *   Remove CCK 1M, 2M, 5.5M, 11M
 *
 *   ArSysParam0 = 786433
 *   Enable auto rate version 1
 *   Remove CCK 5.5M 11M
 */
/*----------------------------------------------------------------------------*/

uint32_t
nicRlmArUpdateParms(IN struct ADAPTER *prAdapter,
		    IN uint32_t u4ArSysParam0,
		    IN uint32_t u4ArSysParam1, IN uint32_t u4ArSysParam2,
		    IN uint32_t u4ArSysParam3)
{
	uint8_t ucArVer, ucAbwVer, ucAgiVer;
	uint16_t u2HtClrMask;
	uint16_t u2LegacyClrMask;
	uint8_t ucArCheckWindow;
	uint8_t ucArPerL;
	uint8_t ucArPerH;
	uint8_t ucArPerForceRateDownPer;

	ucArVer = (uint8_t) (u4ArSysParam0 & BITS(0, 3));
	ucAbwVer = (uint8_t) ((u4ArSysParam0 & BITS(4, 5)) >> 4);
	ucAgiVer = (uint8_t) ((u4ArSysParam0 & BITS(6, 7)) >> 6);
	u2HtClrMask = (uint16_t) ((u4ArSysParam0 & BITS(8,
				   15)) >> 8);
	u2LegacyClrMask = (uint16_t) ((u4ArSysParam0 & BITS(16,
				       31)) >> 16);

#if 0
	ucArCheckWindow = (uint8_t) (u4ArSysParam1 & BITS(0, 7));
	ucArPerH = (uint8_t) ((u4ArSysParam1 & BITS(16, 23)) >> 16);
	ucArPerL = (uint8_t) ((u4ArSysParam1 & BITS(24, 31)) >> 24);
#endif

	ucArCheckWindow = (uint8_t) (u4ArSysParam1 & BITS(0, 7));
	ucArPerForceRateDownPer = (uint8_t) (((u4ArSysParam1 >> 8) &
					      BITS(0, 7)));
	ucArPerH = (uint8_t) (((u4ArSysParam1 >> 16) & BITS(0, 7)));
	ucArPerL = (uint8_t) (((u4ArSysParam1 >> 24) & BITS(0, 7)));

	DBGLOG(INIT, INFO, "ArParam %u %u %u %u\n", u4ArSysParam0,
	       u4ArSysParam1, u4ArSysParam2, u4ArSysParam3);
	DBGLOG(INIT, INFO, "ArVer %u AbwVer %u AgiVer %u\n",
	       ucArVer, ucAbwVer, ucAgiVer);
	DBGLOG(INIT, INFO, "HtMask %x LegacyMask %x\n", u2HtClrMask,
	       u2LegacyClrMask);
	DBGLOG(INIT, INFO,
	       "CheckWin %u RateDownPer %u PerH %u PerL %u\n",
	       ucArCheckWindow,
	       ucArPerForceRateDownPer, ucArPerH, ucArPerL);

#define SWCR_DATA_ADDR(MOD, ADDR) (0x90000000+(MOD<<8)+(ADDR))
#define SWCR_DATA_CMD(CATE, WRITE, INDEX, OPT0, OPT1) \
	((CATE<<24) | (WRITE<<23) | (INDEX<<16) | (OPT0 << 8) | OPT1)
#define SWCR_DATA0 0x0
#define SWCR_DATA1 0x4
#define SWCR_DATA2 0x8
#define SWCR_DATA3 0xC
#define SWCR_DATA4 0x10
#define SWCR_WRITE 1
#define SWCR_READ 0

	if (ucArVer > 0) {
		/* dummy = WiFi.WriteMCR(&h90000104, &h00000001) */
		/* dummy = WiFi.WriteMCR(&h90000100, &h00850000) */

		nicWriteMcr(prAdapter, SWCR_DATA_ADDR(1 /*MOD*/,
			SWCR_DATA1), 1);
		nicWriteMcr(prAdapter, SWCR_DATA_ADDR(1 /*MOD*/,
			SWCR_DATA0), SWCR_DATA_CMD(0, SWCR_WRITE, 5, 0, 0));
	} else {
		nicWriteMcr(prAdapter, SWCR_DATA_ADDR(1 /*MOD*/,
			SWCR_DATA1), 0);
		nicWriteMcr(prAdapter, SWCR_DATA_ADDR(1 /*MOD*/,
			SWCR_DATA0), SWCR_DATA_CMD(0, SWCR_WRITE, 5, 0, 0));
	}

	/* ucArVer 0: none 1:PER 2:Rcpi */
	nicWriteMcr(prAdapter, SWCR_DATA_ADDR(1 /*MOD*/,
		SWCR_DATA1), ucArVer);
	nicWriteMcr(prAdapter, SWCR_DATA_ADDR(1 /*MOD*/,
		SWCR_DATA0), SWCR_DATA_CMD(0, SWCR_WRITE, 7, 0, 0));

	/* Candidate rate Ht mask */
	nicWriteMcr(prAdapter, SWCR_DATA_ADDR(1 /*MOD*/,
		SWCR_DATA1), u2HtClrMask);
	nicWriteMcr(prAdapter, SWCR_DATA_ADDR(1 /*MOD*/,
		SWCR_DATA0), SWCR_DATA_CMD(0, SWCR_WRITE, 0x1c, 0, 0));

	/* Candidate rate legacy mask */
	nicWriteMcr(prAdapter, SWCR_DATA_ADDR(1 /*MOD*/,
		SWCR_DATA1), u2LegacyClrMask);
	nicWriteMcr(prAdapter, SWCR_DATA_ADDR(1 /*MOD*/,
		SWCR_DATA0), SWCR_DATA_CMD(0, SWCR_WRITE, 0x1d, 0, 0));

#if 0
	if (ucArCheckWindow != 0) {
		/* TX DONE MCS INDEX CHECK STA RATE DOWN TH */
		nicWriteMcr(prAdapter, SWCR_DATA_ADDR(1 /*MOD*/,
			SWCR_DATA1), ucArCheckWindow);
		nicWriteMcr(prAdapter, SWCR_DATA_ADDR(1 /*MOD*/,
			SWCR_DATA0), SWCR_DATA_CMD(0, SWCR_WRITE, 0x14, 0, 0));
		nicWriteMcr(prAdapter, SWCR_DATA_ADDR(1 /*MOD*/,
			SWCR_DATA1), ucArCheckWindow);
		nicWriteMcr(prAdapter, SWCR_DATA_ADDR(1 /*MOD*/,
			SWCR_DATA0), SWCR_DATA_CMD(0, SWCR_WRITE, 0xc, 0, 0));
	}

	if (ucArPerForceRateDownPer != 0) {
		nicWriteMcr(prAdapter, SWCR_DATA_ADDR(1 /*MOD*/,
			SWCR_DATA1), ucArPerForceRateDownPer);
		nicWriteMcr(prAdapter, SWCR_DATA_ADDR(1 /*MOD*/,
			SWCR_DATA0), SWCR_DATA_CMD(0, SWCR_WRITE, 0x18, 0, 0));
	}
	if (ucArPerH != 0) {
		nicWriteMcr(prAdapter, SWCR_DATA_ADDR(1 /*MOD*/,
			SWCR_DATA1), ucArPerH);
		nicWriteMcr(prAdapter, SWCR_DATA_ADDR(1 /*MOD*/,
			SWCR_DATA0), SWCR_DATA_CMD(0, SWCR_WRITE, 0x1, 0, 0));
	}
	if (ucArPerL != 0) {
		nicWriteMcr(prAdapter, SWCR_DATA_ADDR(1 /*MOD*/,
			SWCR_DATA1), ucArPerL);
		nicWriteMcr(prAdapter, SWCR_DATA_ADDR(1 /*MOD*/,
			SWCR_DATA0), SWCR_DATA_CMD(0, SWCR_WRITE, 0x2, 0, 0));
	}
#endif

	return WLAN_STATUS_SUCCESS;
}

/*----------------------------------------------------------------------------*/
/*!
 * @brief This function is called to update Link Quality information
 *
 * @param prAdapter      Pointer of Adapter Data Structure
 *        ucBssIndex
 *        prEventLinkQuality
 *        cRssi
 *        cLinkQuality
 *
 * @return none
 */
/*----------------------------------------------------------------------------*/
void nicUpdateLinkQuality(IN struct ADAPTER *prAdapter,
			  IN uint8_t ucBssIndex,
			  IN struct EVENT_LINK_QUALITY *prEventLinkQuality)
{
	int8_t cRssi;
	uint16_t u2AdjustRssi = 10;

	ASSERT(prAdapter);
	ASSERT(ucBssIndex <= prAdapter->ucHwBssIdNum);
	ASSERT(prEventLinkQuality);

	switch (GET_BSS_INFO_BY_INDEX(prAdapter,
				      ucBssIndex)->eNetworkType) {
	case NETWORK_TYPE_AIS:
		if (GET_BSS_INFO_BY_INDEX(prAdapter,
					  ucBssIndex)->eConnectionState ==
		    MEDIA_STATE_CONNECTED) {
			/* check is to prevent RSSI to be updated by
			 * incorrect initial RSSI from hardware
			 */
			/* buffer statistics for further query */
			if (prAdapter->fgIsLinkQualityValid == FALSE
			    || (kalGetTimeTick() -
			    prAdapter->rLinkQualityUpdateTime) >
			    CFG_LINK_QUALITY_VALID_PERIOD) {
				/* ranged from (-128 ~ 30) in unit of dBm */
				cRssi =
					prEventLinkQuality->rLq[ucBssIndex].
					cRssi;
				cRssi =
					(int8_t) (((int16_t)
					(cRssi) * u2AdjustRssi) / 10);
				DBGLOG(RLM, INFO,
					"Rssi=%d, NewRssi=%d\n",
					prEventLinkQuality->rLq[ucBssIndex].
					cRssi,
					cRssi);
				nicUpdateRSSI(prAdapter, ucBssIndex, cRssi,
					prEventLinkQuality->rLq[ucBssIndex].
					cLinkQuality);
			}

			if (prAdapter->fgIsLinkRateValid == FALSE
			    || (kalGetTimeTick() -
			    prAdapter->rLinkRateUpdateTime)
			    > CFG_LINK_QUALITY_VALID_PERIOD) {
				nicUpdateLinkSpeed(prAdapter, ucBssIndex,
					prEventLinkQuality->rLq[ucBssIndex].
					u2LinkSpeed);
			}

		}
		break;

#if 0
/* #if CFG_ENABLE_WIFI_DIRECT && CFG_SUPPORT_P2P_RSSI_QUERY */
	case NETWORK_TYPE_P2P:
		if (prAdapter->fgIsP2pLinkQualityValid == FALSE
		    || (kalGetTimeTick() - prAdapter->rP2pLinkQualityUpdateTime)
		    > CFG_LINK_QUALITY_VALID_PERIOD) {
			struct EVENT_LINK_QUALITY_EX *prEventLQEx =
				(struct EVENT_LINK_QUALITY_EX *)
				prEventLinkQuality;

			nicUpdateRSSI(prAdapter, ucBssIndex,
				prEventLQEx->cRssiP2P,
				prEventLQEx->cLinkQualityP2P);
		}
		break;
#endif
	default:
		break;

	}

}

/*----------------------------------------------------------------------------*/
/*!
 * @brief This function is called to update RSSI and Link Quality information
 *
 * @param prAdapter      Pointer of Adapter Data Structure
 *        ucBssIndex
 *        cRssi
 *        cLinkQuality
 *
 * @return none
 */
/*----------------------------------------------------------------------------*/
void nicUpdateRSSI(IN struct ADAPTER *prAdapter,
		   IN uint8_t ucBssIndex, IN int8_t cRssi,
		   IN int8_t cLinkQuality)
{
	ASSERT(prAdapter);
	ASSERT(ucBssIndex <= prAdapter->ucHwBssIdNum);

	switch (GET_BSS_INFO_BY_INDEX(prAdapter,
				      ucBssIndex)->eNetworkType) {
	case NETWORK_TYPE_AIS:
		if (GET_BSS_INFO_BY_INDEX(prAdapter,
					  ucBssIndex)->eConnectionState ==
		    MEDIA_STATE_CONNECTED) {
			prAdapter->fgIsLinkQualityValid = TRUE;
			prAdapter->rLinkQualityUpdateTime = kalGetTimeTick();

			prAdapter->rLinkQuality.cRssi = cRssi;
			prAdapter->rLinkQuality.cLinkQuality = cLinkQuality;
			/* indicate to glue layer */
			kalUpdateRSSI(prAdapter->prGlueInfo,
				      KAL_NETWORK_TYPE_AIS_INDEX,
				      prAdapter->rLinkQuality.cRssi,
				      prAdapter->rLinkQuality.cLinkQuality);
		}

		break;
#if CFG_ENABLE_WIFI_DIRECT && CFG_SUPPORT_P2P_RSSI_QUERY
	case NETWORK_TYPE_P2P:
		prAdapter->fgIsP2pLinkQualityValid = TRUE;
		prAdapter->rP2pLinkQualityUpdateTime = kalGetTimeTick();

		prAdapter->rP2pLinkQuality.cRssi = cRssi;
		prAdapter->rP2pLinkQuality.cLinkQuality = cLinkQuality;

		kalUpdateRSSI(prAdapter->prGlueInfo,
			      KAL_NETWORK_TYPE_P2P_INDEX, cRssi, cLinkQuality);
		break;
#endif
	default:
		break;

	}

}

/*----------------------------------------------------------------------------*/
/*!
 * @brief This function is called to update Link Quality information
 *
 * @param prAdapter      Pointer of Adapter Data Structure
 *        ucBssIndex
 *        prEventLinkQuality
 *        cRssi
 *        cLinkQuality
 *
 * @return none
 */
/*----------------------------------------------------------------------------*/
void nicUpdateLinkSpeed(IN struct ADAPTER *prAdapter,
			IN uint8_t ucBssIndex, IN uint16_t u2LinkSpeed)
{
	ASSERT(prAdapter);
	ASSERT(ucBssIndex <= prAdapter->ucHwBssIdNum);

	switch (GET_BSS_INFO_BY_INDEX(prAdapter,
				      ucBssIndex)->eNetworkType) {
	case NETWORK_TYPE_AIS:
		if (GET_BSS_INFO_BY_INDEX(prAdapter,
					  ucBssIndex)->eConnectionState ==
		    MEDIA_STATE_CONNECTED) {
			/* buffer statistics for further query */
			prAdapter->fgIsLinkRateValid = TRUE;
			prAdapter->rLinkRateUpdateTime = kalGetTimeTick();

			prAdapter->rLinkQuality.u2LinkSpeed = u2LinkSpeed;
		}
		break;

	default:
		break;

	}

}

#if CFG_SUPPORT_RDD_TEST_MODE
uint32_t nicUpdateRddTestMode(IN struct ADAPTER *prAdapter,
			      IN struct CMD_RDD_CH *prRddChParam)
{
	DEBUGFUNC("nicUpdateRddTestMode.\n");

	ASSERT(prAdapter);

	/* aisFsmScanRequest(prAdapter, NULL); */

	return wlanSendSetQueryCmd(prAdapter,
				   CMD_ID_SET_RDD_CH,
				   TRUE,
				   FALSE, FALSE, NULL, NULL,
				   sizeof(struct CMD_RDD_CH),
				   (uint8_t *) prRddChParam, NULL, 0);
}
#endif

/*----------------------------------------------------------------------------*/
/*!
 * @brief This function is called to apply network address setting to
 *        both OS side and firmware domain
 *
 * @param prAdapter      Pointer of Adapter Data Structure
 *
 * @return none
 */
/*----------------------------------------------------------------------------*/

uint32_t nicApplyNetworkAddress(IN struct ADAPTER
				*prAdapter)
{
	uint32_t i;

	ASSERT(prAdapter);

	/* copy to adapter */
	COPY_MAC_ADDR(prAdapter->rMyMacAddr,
		      prAdapter->rWifiVar.aucMacAddress);

	/* 4 <3> Update new MAC address to all 3 networks */
	COPY_MAC_ADDR(prAdapter->rWifiVar.aucDeviceAddress,
		      prAdapter->rMyMacAddr);
	prAdapter->rWifiVar.aucDeviceAddress[0] ^=
		MAC_ADDR_LOCAL_ADMIN;

	for (i = 0; i < KAL_P2P_NUM; i++) {
		COPY_MAC_ADDR(prAdapter->rWifiVar.aucInterfaceAddress[i],
			      prAdapter->rMyMacAddr);
		prAdapter->rWifiVar.aucInterfaceAddress[i][0] |= 0x2;
		prAdapter->rWifiVar.aucInterfaceAddress[i][0] ^=
			i << MAC_ADDR_LOCAL_ADMIN;
	}

#if CFG_ENABLE_WIFI_DIRECT
	if (prAdapter->fgIsP2PRegistered) {
		for (i = 0; i < prAdapter->ucHwBssIdNum; i++) {
			if (prAdapter->rWifiVar.arBssInfoPool[i].eNetworkType ==
			    NETWORK_TYPE_P2P) {
				COPY_MAC_ADDR(
					prAdapter->rWifiVar.arBssInfoPool[i].
					aucOwnMacAddr,
					prAdapter->rWifiVar.aucDeviceAddress);
			}
		}
	}
#endif

#if CFG_ENABLE_BT_OVER_WIFI
	for (i = 0; i < prAdapter->ucHwBssIdNum; i++) {
		if (prAdapter->rWifiVar.arBssInfoPool[i].eNetworkType ==
		    NETWORK_TYPE_BOW) {
			COPY_MAC_ADDR(
				prAdapter->rWifiVar.arBssInfoPool[i].
				aucOwnMacAddr,
				prAdapter->rWifiVar.aucDeviceAddress);
		}
	}
#endif

#if CFG_TEST_WIFI_DIRECT_GO
	if (prAdapter->rWifiVar.prP2pFsmInfo->eCurrentState ==
	    P2P_STATE_IDLE) {
		wlanEnableP2pFunction(prAdapter);

		wlanEnableATGO(prAdapter);
	}
#endif

	kalUpdateMACAddress(prAdapter->prGlueInfo,
			    prAdapter->rWifiVar.aucMacAddress);

	if (KAL_AIS_NUM > 1) {
		/* Generate from wlan0 MAC */
		COPY_MAC_ADDR(prAdapter->rWifiVar.aucMacAddress1,
			prAdapter->rWifiVar.aucMacAddress);
		/* Update wlan1 address */
		prAdapter->rWifiVar.aucMacAddress1[3] ^= BIT(1);
	}

	return WLAN_STATUS_SUCCESS;
}

#if 1
uint8_t nicGetChipHwVer(void)
{
	return g_eco_info.ucHwVer;
}

uint8_t nicGetChipSwVer(void)
{
	return g_eco_info.ucRomVer;
}

uint8_t nicGetChipFactoryVer(void)
{
	return g_eco_info.ucFactoryVer;
}

uint8_t nicSetChipHwVer(uint8_t value)
{
	g_eco_info.ucHwVer = value;
	return 0;
}

uint8_t nicSetChipSwVer(uint8_t value)
{
	g_eco_info.ucRomVer = value;
	return 0;
}

uint8_t nicSetChipFactoryVer(uint8_t value)
{
	g_eco_info.ucFactoryVer = value;
	return 0;
}

#else
uint8_t nicGetChipHwVer(void)
{
	return mtk_wcn_wmt_ic_info_get(WMTCHIN_HWVER) & BITS(0, 7);
}

uint8_t nicGetChipSwVer(void)
{
	return mtk_wcn_wmt_ic_info_get(WMTCHIN_FWVER) & BITS(0, 7);
}

uint8_t nicGetChipFactoryVer(void)
{
	return (mtk_wcn_wmt_ic_info_get(WMTCHIN_FWVER) & BITS(8,
			11)) >> 8;
}
#endif

uint8_t nicGetChipEcoVer(IN struct ADAPTER *prAdapter)
{
	struct ECO_INFO *prEcoInfo;
	uint8_t ucEcoVer;
	uint8_t ucCurSwVer, ucCurHwVer, ucCurFactoryVer;

	ucCurSwVer = nicGetChipSwVer();
	ucCurHwVer = nicGetChipHwVer();
	ucCurFactoryVer = nicGetChipFactoryVer();

	ucEcoVer = 0;

	while (TRUE) {
		/* Get ECO info from table */
		prEcoInfo = (struct ECO_INFO *) &
			    (prAdapter->chip_info->eco_info[ucEcoVer]);

		if ((prEcoInfo->ucRomVer == 0) &&
		    (prEcoInfo->ucHwVer == 0) &&
		    (prEcoInfo->ucFactoryVer == 0)) {

			/* last ECO info */
			if (ucEcoVer > 0)
				ucEcoVer--;

			/* End of table */
			break;
		}

		if ((prEcoInfo->ucRomVer == ucCurSwVer) &&
		    (prEcoInfo->ucHwVer == ucCurHwVer) &&
		    (prEcoInfo->ucFactoryVer == ucCurFactoryVer)) {
			break;
		}

		ucEcoVer++;
	}

#if 0
	DBGLOG(INIT, INFO,
	       "Cannot get ECO version for SwVer[0x%02x]HwVer[0x%02x]FactoryVer[0x%1x],recognize as latest version[E%u]\n",
	       ucCurSwVer, ucCurHwVer, ucCurFactoryVer,
	       prAdapter->chip_info->eco_info[ucEcoVer].ucEcoVer);
#endif
	return prAdapter->chip_info->eco_info[ucEcoVer].ucEcoVer;
}

u_int8_t nicIsEcoVerEqualTo(IN struct ADAPTER *prAdapter,
			    uint8_t ucEcoVer)
{
	if (ucEcoVer == prAdapter->chip_info->eco_ver)
		return TRUE;
	else
		return FALSE;
}

u_int8_t nicIsEcoVerEqualOrLaterTo(IN struct ADAPTER
				   *prAdapter, uint8_t ucEcoVer)
{
	if (ucEcoVer <= prAdapter->chip_info->eco_ver)
		return TRUE;
	else
		return FALSE;
}

void nicSerStopTxRx(IN struct ADAPTER *prAdapter)
{
#if defined(_HIF_USB)
	unsigned long ulFlags;

	/*TODO: multiple spinlocks seems risky.
	* http://www.linuxgrill.com/anonymous/fire/netfilter/
	*			    kernel-hacking-HOWTO-5.html
	*/
	/* 1. Make sure ucSerState is accessed sequentially.
	*  2. Two scenario for race condition:
	*    - When hif_thread is doing usb_submit_urb, SER occurs.
	*	hif_thread acquires the lock first,
	*	so nicSerSyncTimerHandler must wait hif_thread
	*	until it completes current usb_submit_urb.
	*	Then, nicSerSyncTimerHandler acquires the lock,
	*	change ucSerState to prevent subsequent usb_submit_urb and
	*	cancel ALL TX BULK OUT URB.
	*    - When SER is triggered and executed,
	*	hif_thread is prepared to do usb_submit_urb.
	*	nicSerSyncTimerHandler acquires the lock first,
	*	which guarantees ucSerState is accessed sequentially.
	*	Then, hif_thread acquires the lock, knows that SER is ongoing,
	*	and bypass usb_submit_urb.
	*/
	spin_lock_irqsave(&prAdapter->prGlueInfo->rHifInfo.rStateLock,
				ulFlags);
#endif
	DBGLOG(NIC, WARN, "[SER] host set STOP_TRX\n");
	prAdapter->ucSerState = SER_STOP_HOST_TX_RX;

	/* Force own to FW as ACK and stop HIF */
	prAdapter->fgWiFiInSleepyState = TRUE;

#if defined(_HIF_USB)
	spin_unlock_irqrestore(&prAdapter->prGlueInfo->rHifInfo.rStateLock,
				ulFlags);
#endif

}

void nicSerStopTx(IN struct ADAPTER *prAdapter)
{
	DBGLOG(NIC, WARN, "[SER] Stop HIF Tx!\n");

	prAdapter->ucSerState = SER_STOP_HOST_TX;
}

void nicSerStartTxRx(IN struct ADAPTER *prAdapter)
{
	DBGLOG(NIC, WARN, "[SER] Start HIF T/R!\n");

	prAdapter->ucSerState = SER_IDLE_DONE;
}

u_int8_t nicSerIsWaitingReset(IN struct ADAPTER *prAdapter)
{
	if (prAdapter->ucSerState == SER_STOP_HOST_TX_RX)
		return TRUE;
	else
		return FALSE;
}

u_int8_t nicSerIsTxStop(IN struct ADAPTER *prAdapter)
{
	switch (prAdapter->ucSerState) {
	case SER_STOP_HOST_TX:
	case SER_STOP_HOST_TX_RX:
	case SER_REINIT_HIF:
		return TRUE;

	case SER_IDLE_DONE:
	default:
		return FALSE;
	}
}

u_int8_t nicSerIsRxStop(IN struct ADAPTER *prAdapter)
{
	switch (prAdapter->ucSerState) {
	case SER_STOP_HOST_TX_RX:
	case SER_REINIT_HIF:
		return TRUE;

	case SER_STOP_HOST_TX:
	case SER_IDLE_DONE:
	default:
		return FALSE;
	}
}

void nicSerReInitBeaconFrame(IN struct ADAPTER *prAdapter)
{
	struct P2P_ROLE_FSM_INFO *prRoleP2pFsmInfo;

	prRoleP2pFsmInfo = P2P_ROLE_INDEX_2_ROLE_FSM_INFO(prAdapter,
			   0);
	if (prRoleP2pFsmInfo != NULL) {
		bssUpdateBeaconContent(prAdapter,
				       prRoleP2pFsmInfo->ucBssIndex);
		DBGLOG(NIC, INFO, "SER beacon frame is updated\n");
	}
}

#if defined(_HIF_USB)
void nicSerTimerHandler(IN struct ADAPTER *prAdapter,
	IN unsigned long plParamPtr)
{
	uint32_t u4TimeoutMs;

#if (CFG_SUPPORT_WFDMA_REALLOC == 1)
	/* Use a shorter timeout timer to speed up the procedure of
	 * wfdma reallocation in USB mode.
	 */
	if (prAdapter->fgIsSupportWfdmaRealloc &&
	    !prAdapter->fgWfdmaReallocDone)
		u4TimeoutMs = WIFI_SER_SYNC_TIMER_SHORT_TIMEOUT_IN_MS;
	else
#endif /* CFG_SUPPORT_WFDMA_REALLOC */
		u4TimeoutMs = WIFI_SER_SYNC_TIMER_NORMAL_TIMEOUT_IN_MS;

	halSerSyncTimerHandler(prAdapter);
	cnmTimerStartTimer(prAdapter, &rSerSyncTimer, u4TimeoutMs);
}
#endif

void nicSerInit(IN struct ADAPTER *prAdapter, IN const u_int8_t bAtResetFlow)
{
	if (prAdapter->chip_info->asicSerInit)
		prAdapter->chip_info->asicSerInit(prAdapter, bAtResetFlow);

#if CFG_CHIP_RESET_SUPPORT
	if (prAdapter->chip_info->fgIsSupportL0p5Reset) {
		if (!bAtResetFlow) {
			spin_lock_init(&prAdapter->rWfsysResetLock);
			glSetWfsysResetState(prAdapter, WFSYS_RESET_STATE_IDLE);
			INIT_WORK(&prAdapter->prGlueInfo->rWfsysResetWork,
				  WfsysResetHdlr);
		}

		if (prAdapter->rWifiVar.eEnableSerL0p5 ==
		    FEATURE_OPT_SER_DISABLE)
			wlanoidSerExtCmd(prAdapter, SER_ACTION_L0P5_CTRL,
					 SER_ACTION_L0P5_CTRL_PAUSE_WDT, 0);
	}
#endif

	/* if SER L1 is not enabled, disable this feature in FW */
	if (prAdapter->rWifiVar.eEnableSerL1 == FEATURE_OPT_SER_DISABLE)
		wlanoidSerExtCmd(prAdapter, SER_ACTION_SET,
				SER_SET_DISABLE, 0);

#if defined(_HIF_USB)
	/* if SER L1 sync CR address is specified, then init sync timer */
	if (prAdapter->chip_info->u4SerUsbMcuEventAddr != 0) {
		if (!bAtResetFlow) {
			cnmTimerInitTimer(prAdapter,
					  &rSerSyncTimer,
				     (PFN_MGMT_TIMEOUT_FUNC) nicSerTimerHandler,
					  (unsigned long) NULL);
		}

		cnmTimerStartTimer(prAdapter,
			&rSerSyncTimer,
			WIFI_SER_SYNC_TIMER_NORMAL_TIMEOUT_IN_MS);
	}
#endif /* _HIF_USB */
}

void nicSerDeInit(IN struct ADAPTER *prAdapter)
{
#if CFG_CHIP_RESET_SUPPORT
	if (prAdapter->chip_info->fgIsSupportL0p5Reset)
		flush_work(&prAdapter->prGlueInfo->rWfsysResetWork);
#endif

#if defined(_HIF_USB)
	cnmTimerStopTimer(prAdapter, &rSerSyncTimer);
#endif
}

#if (CFG_COALESCING_INTERRUPT == 1)
uint32_t nicSetCoalescingInt(IN struct ADAPTER *prAdapter,
			u_int8_t fgPktThEn, u_int8_t fgTmrThEn)
{
	struct CMD_PF_CF_COALESCING_INT rCmdSetCoalescingInt;
	struct CMD_PF_CF_COALESCING_INT *prCmdSetCoalescingInt;

	kalMemZero(&rCmdSetCoalescingInt,
		sizeof(struct CMD_PF_CF_COALESCING_INT));

	ASSERT(prAdapter);
	prCmdSetCoalescingInt = &rCmdSetCoalescingInt;
	prCmdSetCoalescingInt->ucPktThEn = fgPktThEn;
	prCmdSetCoalescingInt->ucTmrThEn = fgTmrThEn;
	prCmdSetCoalescingInt->u2MaxPkt =
		prAdapter->rWifiVar.u2CoalescingIntMaxPk;
	prCmdSetCoalescingInt->u2MaxTime =
		prAdapter->rWifiVar.u2CoalescingIntMaxTime;
	prCmdSetCoalescingInt->u2FilterMask =
		prAdapter->rWifiVar.u2CoalescingIntuFilterMask;

	return wlanSendSetQueryCmd(prAdapter,
	    CMD_ID_PF_CF_COALESCING_INT,
	    TRUE,
	    FALSE,
	    FALSE,
	    NULL,
	    NULL,
	    sizeof(struct CMD_PF_CF_COALESCING_INT),
	    (uint8_t *)prCmdSetCoalescingInt, NULL, 0);
}
#endif

#if (CFG_SUPPORT_WIFI_6G == 1)
void nicRxdChNumTranslate(
	IN enum ENUM_BAND eBand, IN uint8_t *pucHwChannelNum)
{
	if ((eBand == BAND_6G) && (pucHwChannelNum != NULL))
		*pucHwChannelNum = (((*pucHwChannelNum-181) << 2) + 1);

}
#endif

