/* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause */
/*
 * Copyright (c) 2016 MediaTek Inc.
 */
/*
 Module Name:
 hif_pdma.h
 */

#ifndef __HIF_PDMA_H__
#define __HIF_PDMA_H__

#include <linux/list_sort.h>

/*******************************************************************************
 *                              C O N S T A N T S
 *******************************************************************************
 */
#define NUM_OF_WFDMA1_TX_RING			0
#define NUM_OF_WFDMA1_RX_RING			0

#if (CFG_SUPPORT_CONNAC2X == 1 || CFG_SUPPORT_CONNAC3X == 1)
#undef NUM_OF_WFDMA1_TX_RING
#define NUM_OF_WFDMA1_TX_RING			1  /* WA CMD Ring */
#undef NUM_OF_WFDMA1_RX_RING
#define NUM_OF_WFDMA1_RX_RING			5
#endif /* CFG_SUPPORT_CONNAC2X == 1 || CFG_SUPPORT_CONNAC3X == 1 */

#define NUM_OF_TX_RING				(4+NUM_OF_WFDMA1_TX_RING)
#define NUM_OF_RX_RING				(2+NUM_OF_WFDMA1_RX_RING)

/* Max Tx ring size */
#if defined(MT7922)
#define TX_RING_SIZE				2048
#elif defined(MT7961) || defined(MT7902)
#define TX_RING_SIZE				512
#elif (CFG_SUPPORT_CONNAC3X == 1)
#define TX_RING_SIZE				1024
#else
#define TX_RING_SIZE				256
#endif

#if defined(MT7922)
/* Max Rx ring size */
#define RX_RING_SIZE				1024

/* Data Rx ring */
#define RX_RING0_SIZE				1024
#else
/* Max Rx ring size */
#define RX_RING_SIZE				256

/* Data Rx ring */
#define RX_RING0_SIZE				256
#endif

/* Event/MSDU_report Rx ring */
#define RX_RING1_SIZE				16

/* TXD_SIZE = TxD + TxInfo */
#define TXD_SIZE					16
#define RXD_SIZE					16

#define RX_BUFFER_AGGRESIZE			3840
#define RX_BUFFER_NORMSIZE			3840
#define TX_BUFFER_NORMSIZE			3840

#ifndef HIF_NUM_OF_QM_RX_PKT_NUM
#define HIF_NUM_OF_QM_RX_PKT_NUM			2048
#endif
#define HIF_IST_LOOP_COUNT					32
/* Min msdu count to trigger Tx during INT polling state */
#define HIF_IST_TX_THRESHOLD				1

#define HIF_TX_BUFF_COUNT_TC0				4096
#define HIF_TX_BUFF_COUNT_TC1				4096
#define HIF_TX_BUFF_COUNT_TC2				4096
#define HIF_TX_BUFF_COUNT_TC3				4096
#define HIF_TX_BUFF_COUNT_TC4				(TX_RING_SIZE - 1)
#define HIF_TX_BUFF_COUNT_TC5				4096

/* enable/disable TX resource control */
#define HIF_TX_RESOURCE_CTRL                1
/* enable/disable TX resource control PLE */
#define HIF_TX_RESOURCE_CTRL_PLE            0


#define HIF_TX_PAGE_SIZE_IN_POWER_OF_2		11
/* in unit of bytes */
#define HIF_TX_PAGE_SIZE					2048

#define HIF_EXTRA_IO_BUFFER_SIZE			0

#define HIF_TX_COALESCING_BUFFER_SIZE		(TX_BUFFER_NORMSIZE)
#define HIF_RX_COALESCING_BUFFER_SIZE		(RX_BUFFER_AGGRESIZE)

#define HIF_CR4_FWDL_SECTION_NUM			1
#define HIF_IMG_DL_STATUS_PORT_IDX			1

#define HIF_TX_INIT_CMD_PORT				TX_RING_FWDL_IDX_3

#if CFG_SUPPORT_DBDC
#define HIF_TX_MSDU_TOKEN_NUM				(TX_RING_SIZE * 3)
#else
#define HIF_TX_MSDU_TOKEN_NUM				(TX_RING_SIZE * 2)
#endif

#define HIF_TX_PAYLOAD_LENGTH				72

#define HIF_TX_MAX_FRAG					3 /* MUST be odd */

#define HIF_MSDU_REPORT_RETURN_TIMEOUT		10	/* sec */
#define HIF_SER_TIMEOUT				10000	/* msec */

#define MT_RINGREG_DIFF		0x10
#define MT_RINGREG_EXT_DIFF	0x04

#define MT_TX_RING_BASE		WPDMA_TX_RING0_CTRL0
#define MT_TX_RING_PTR		WPDMA_TX_RING0_CTRL0
#define MT_TX_RING_CNT		WPDMA_TX_RING0_CTRL1
#define MT_TX_RING_CIDX		WPDMA_TX_RING0_CTRL2
#define MT_TX_RING_DIDX		WPDMA_TX_RING0_CTRL3

#define MT_RX_RING_BASE		WPDMA_RX_RING0_CTRL0
#define MT_RX_RING_PTR		WPDMA_RX_RING0_CTRL0
#define MT_RX_RING_CNT		WPDMA_RX_RING0_CTRL1
#define MT_RX_RING_CIDX		WPDMA_RX_RING0_CTRL2
#define MT_RX_RING_DIDX		WPDMA_RX_RING0_CTRL3

#define MT_RING_CIDX_MASK	0x00000FFF
#define MT_RING_DIDX_MASK	0x00000FFF
#define MT_RING_CNT_MASK	0x00000FFF

#define DMA_LOWER_32BITS_MASK   0x00000000FFFFFFFF
#define DMA_HIGHER_4BITS_MASK   0x0000000F
#define DMA_BITS_OFFSET		32

#define DMA_DONE_WAITING_TIME   10
#define DMA_DONE_WAITING_COUNT  100

#define MT_TX_RING_BASE_EXT WPDMA_TX_RING0_BASE_PTR_EXT
#define MT_RX_RING_BASE_EXT WPDMA_RX_RING0_BASE_PTR_EXT

#define PDMA_DUMMY_RESET_VALUE  0x0F
#define PDMA_DUMMY_MAGIC_NUM    0x13

#define TXD_DW1_RMVL            BIT(10)
#define TXD_DW1_VLAN            BIT(11)
#define TXD_DW1_ETYP            BIT(12)
#define TXD_DW1_AMSDU_C         BIT(20)

#define HIF_DEADFEED_VALUE      0xdeadfeed

/*******************************************************************************
 *                                 M A C R O S
 *******************************************************************************
 */

#define INC_RING_INDEX(_idx, _RingSize)		\
{ \
	(_idx) = (_idx+1) % (_RingSize); \
	KAL_MB_W(); \
}

#define RTMP_IO_READ32(_A, _R, _pV) \
{ \
	(*(_pV) = readl((void *)((_A)->CSRBaseAddress + (_R)))); \
}

#define RTMP_IO_WRITE32(_A, _R, _V) \
{ \
	writel(_V, (void *)((_A)->CSRBaseAddress + (_R))); \
}

/*******************************************************************************
 *                             D A T A   T Y P E S
 *******************************************************************************
 */

/* sw defined tx ring idx */
enum ENUM_TX_RING_IDX {
	TX_RING_DATA0_IDX_0 = 0,
	TX_RING_DATA1_IDX_1,
	TX_RING_CMD_IDX_2,
	TX_RING_FWDL_IDX_3,
	TX_RING_WA_CMD_IDX_4,
};

/* sw defined rx ring idx */
enum ENUM_RX_RING_IDX {
	RX_RING_DATA_IDX_0 = 0,  /*Rx Data */
	RX_RING_EVT_IDX_1,
#if (CFG_SUPPORT_CONNAC3X == 0)
	WFDMA0_RX_RING_IDX_2,  /* Band0 TxFreeDoneEvent */
	WFDMA0_RX_RING_IDX_3,  /* Band1 TxFreeDoneEvent  */
	WFDMA1_RX_RING_IDX_0,  /* WM Event */
	WFDMA1_RX_RING_IDX_1,  /*WA Band 0 Event*/
	WFDMA1_RX_RING_IDX_2,  /*WA Band 1 Event*/
#else
	RX_RING_DATA1_IDX_2,
	RX_RING_TXDONE0_IDX_3,
	RX_RING_TXDONE1_IDX_4,
	RX_RING_DATA2_IDX_5,
	RX_RING_TXDONE2_IDX_6,
	RX_RING_WAEVT0_IDX_5,
	RX_RING_WAEVT1_IDX_6,
#endif
	RX_RING_MAX,
};

/* hw defined tx ring idx */
enum ENUM_HW_WFDMA0_TX_RING_IDX {
	HW_WFDMA0_TX_RING_IDX_0 = 0,
	HW_WFDMA0_TX_RING_IDX_1,
	HW_WFDMA0_TX_RING_IDX_2,
	HW_WFDMA0_TX_RING_IDX_3,
	HW_WFDMA0_TX_RING_IDX_4,
	HW_WFDMA0_TX_RING_IDX_5,
	HW_WFDMA0_TX_RING_IDX_6,
	HW_WFDMA0_TX_RING_IDX_7,
	HW_WFDMA0_TX_RING_IDX_8,
	HW_WFDMA0_TX_RING_IDX_9,
	HW_WFDMA0_TX_RING_IDX_10,
	HW_WFDMA0_TX_RING_IDX_11,
	HW_WFDMA0_TX_RING_IDX_12,
	HW_WFDMA0_TX_RING_IDX_13,
	HW_WFDMA0_TX_RING_IDX_14,
	HW_WFDMA0_TX_RING_IDX_15,
	HW_WFDMA0_TX_RING_IDX_16,
	HW_WFDMA0_TX_RING_IDX_17
};

/* hw defined rx ring idx */
enum ENUM_HW_WFDMA0_RX_RING_IDX {
	HW_WFDMA0_RX_RING_IDX_0 = 0,
	HW_WFDMA0_RX_RING_IDX_1,
	HW_WFDMA0_RX_RING_IDX_2,
	HW_WFDMA0_RX_RING_IDX_3,
	HW_WFDMA0_RX_RING_IDX_4,
	HW_WFDMA0_RX_RING_IDX_5,
	HW_WFDMA0_RX_RING_IDX_6,
	HW_WFDMA0_RX_RING_IDX_7,
	HW_WFDMA0_RX_RING_IDX_8,
	HW_WFDMA0_RX_RING_IDX_9,
};

/* ============================================================================
 * PCI/RBUS TX / RX Frame Descriptors format
 *
 * Memory Layout
 *
 * 1. Tx Descriptor
 * TxD (12 bytes) + TXINFO (4 bytes)
 * 2. Packet Buffer
 * TXWI + 802.11
 * 31                                                                         0
 * +--------------------------------------------------------------------------+
 * |                                   SDP0[31:0]                             |
 * +-+--+---------------------+-+--+------------------------------------------+
 * |D |L0|       SDL0[13:0]              |B|L1|                    SDL1[13:0] |
 * +-+--+---------------------+-+--+------------------------------------------+
 * |                                   SDP1[31:0]                             |
 * +--------------------------------------------------------------------------+
 * |                                  TX / RX INFO                            |
 * +--------------------------------------------------------------------------+
 * =========================================================================
 */
/*
 *  TX descriptor format for Tx Data/Mgmt Rings
 */
struct TXD_STRUCT {
	/* Word 0 */
	uint32_t SDPtr0;

	/* Word 1 */
	uint32_t SDLen1:14;
	uint32_t LastSec1:1;
	uint32_t Burst:1;
	uint32_t SDLen0:14;
	uint32_t LastSec0:1;
	uint32_t DMADONE:1;

	/*Word2 */
	uint32_t SDPtr1;

	/*Word3 */
	uint16_t SDPtr0Ext;
	uint16_t SDPtr1Ext;
};

/*
 *  Rx descriptor format for Rx Rings
 */
struct RXD_STRUCT {
	/* Word 0 */
	uint32_t SDPtr0;

	/* Word 1 */
	uint32_t SDLen1:14;
	uint32_t LastSec1:1;
	uint32_t Burst:1;
	uint32_t SDLen0:14;
	uint32_t LastSec0:1;
	uint32_t DMADONE:1;

	/* Word 2 */
	uint32_t SDPtr1;

	/* Word 3 */
	uint32_t RXINFO;
};

/*
 *	Data buffer for DMA operation, the buffer must be contiguous
 *	physical memory Both DMA to / from CPU use the same structure.
 */
struct RTMP_DMABUF {
	unsigned long AllocSize;
	void *AllocVa;		/* TxBuf virtual address */
	phys_addr_t AllocPa;		/* TxBuf physical address */
};

/*
 *	Control block (Descriptor) for all ring descriptor DMA operation,
 *	buffer must be contiguous physical memory. NDIS_PACKET stored the
 *	binding Rx packet descriptor which won't be released, driver has to
 *	wait until upper layer return the packet before giveing up this rx
 *	ring descriptor to ASIC. NDIS_BUFFER is assocaited pair to describe
 *	the packet buffer. For Tx, NDIS_PACKET stored the tx packet descriptor
 *  which driver should ACK upper layer when the tx is physically done or
 *  failed.
 */
struct RTMP_DMACB {
	unsigned long AllocSize;	/* Control block size */
	void *AllocVa;			/* Control block virtual address */
	phys_addr_t AllocPa;	        /* Control block physical address */
	void *pPacket;
	void *pBuffer;
	phys_addr_t PacketPa;
	struct RTMP_DMABUF DmaBuf;	/* Associated DMA buffer structure */
	struct MSDU_TOKEN_ENTRY *prToken;
};

struct RTMP_TX_RING {
	struct RTMP_DMACB Cell[TX_RING_SIZE];
	uint32_t TxCpuIdx;
	uint32_t TxDmaIdx;
	uint32_t u4BufSize;
	uint32_t TxSwUsedIdx;
	uint32_t u4UsedCnt;
	uint32_t hw_desc_base;
	uint32_t hw_desc_base_ext;
	uint32_t hw_cidx_addr;
	uint32_t hw_didx_addr;
	uint32_t hw_cnt_addr;
	spinlock_t rTxDmaQLock;
};

struct RTMP_RX_RING {
	struct RTMP_DMACB Cell[RX_RING_SIZE];
	uint32_t RxCpuIdx;
	uint32_t RxDmaIdx;
	uint32_t u4BufSize;
	uint32_t u4RingSize;
	u_int8_t fgRxSegPkt;
	uint32_t hw_desc_base;
	uint32_t hw_desc_base_ext;
	uint32_t hw_cidx_addr;
	uint32_t hw_didx_addr;
	uint32_t hw_cnt_addr;
	u_int8_t fgSwRead;
	bool fgIsDumpLog;
#if (CFG_SUPPORT_PDMA_SCATTER == 1)
	void *pvPacket;
	uint32_t u4PacketLen;
#endif
};

struct PCIE_CHIP_CR_MAPPING {
	uint32_t u4ChipAddr;
	uint32_t u4BusAddr;
	uint32_t u4Range;
};

struct MSDU_TOKEN_ENTRY {
	uint32_t u4Token;
	u_int8_t fgInUsed;
#if KERNEL_VERSION(5, 0, 0) <= LINUX_VERSION_CODE
	struct timespec64 rTs;	/* token tx timestamp */
#else
	struct timeval rTs;	/* token tx timestamp */
#endif
	uint32_t u4CpuIdx;	/* tx ring cell index */
	struct MSDU_INFO *prMsduInfo;
	void *prPacket;
	phys_addr_t rDmaAddr;
	uint32_t u4DmaLength;
	phys_addr_t rPktDmaAddr;
	uint32_t u4PktDmaLength;
	uint16_t u2Port; /* tx ring number */
#if (CFG_SUPPORT_TX_SG == 1)
	void *rPktDmaVAddr_nr[HIF_TX_MAX_FRAG];
	phys_addr_t rPktDmaAddr_nr[HIF_TX_MAX_FRAG];
	uint32_t u4PktDmaLength_nr[HIF_TX_MAX_FRAG];
	uint8_t nr_frags;
	uint32_t len_frags;
#endif
};

struct MSDU_TOKEN_INFO {
	uint32_t u4UsedCnt;
	struct MSDU_TOKEN_ENTRY *aprTokenStack[HIF_TX_MSDU_TOKEN_NUM];
	spinlock_t rTokenLock;
	struct MSDU_TOKEN_ENTRY arToken[HIF_TX_MSDU_TOKEN_NUM];
};

struct TX_CMD_REQ {
	struct CMD_INFO *prCmdInfo;
	uint8_t ucTC;
	struct list_head list;
};

struct TX_DATA_REQ {
	struct MSDU_INFO *prMsduInfo;
	struct list_head list;
};

struct AMSDU_MAC_TX_DESC {
	uint16_t u2TxByteCount;
	uint16_t u2DW0;
	uint32_t u4DW1;
	uint32_t u4DW2:31;
	uint32_t u4FR:1;
	uint32_t u4DW3;
	uint32_t u4DW4;
	uint32_t u4DW5_1:9;
	uint32_t u4TXS:2;
	uint32_t u4DW5_2:21;
	uint32_t u4DW6;
	uint32_t u4DW7;
};


struct ERR_RECOVERY_CTRL_T {
	uint8_t eErrRecovState;
	uint32_t u4Status;
#if (CFG_SUPPORT_CONNAC3X == 1)
	uint32_t u4BackupStatus;
#endif
};

/*******************************************************************************
*                   F U N C T I O N   D E C L A R A T I O N S
********************************************************************************
*/

u_int8_t halSerHappendInSuspend(IN struct ADAPTER *prAdapter);
void halSerPollDoneInSuspend(IN struct ADAPTER *prAdapter);
void halHifRst(struct GLUE_INFO *prGlueInfo);
bool halWpdmaAllocRing(struct GLUE_INFO *prGlueInfo, bool fgAllocMem);
void halWpdmaFreeRing(struct GLUE_INFO *prGlueInfo);
void halWpdmaInitRing(struct GLUE_INFO *prGlueInfo);
void halWpdmaInitTxRing(IN struct GLUE_INFO *prGlueInfo);
void halWpdmaInitRxRing(IN struct GLUE_INFO *prGlueInfo);
void halWpdmaProcessCmdDmaDone(IN struct GLUE_INFO *prGlueInfo,
			       IN uint16_t u2Port);
void halWpdmaProcessDataDmaDone(IN struct GLUE_INFO *prGlueInfo,
				IN uint16_t u2Port);
uint32_t halWpdmaGetRxDmaDoneCnt(IN struct GLUE_INFO *prGlueInfo,
				 IN uint8_t ucRingNum);
void halInitMsduTokenInfo(IN struct ADAPTER *prAdapter);
void halUninitMsduTokenInfo(IN struct ADAPTER *prAdapter);
uint32_t halGetMsduTokenFreeCnt(IN struct ADAPTER *prAdapter);
struct MSDU_TOKEN_ENTRY *halGetMsduTokenEntry(IN struct ADAPTER *prAdapter,
					      uint32_t u4TokenNum);
struct MSDU_TOKEN_ENTRY *halAcquireMsduToken(IN struct ADAPTER *prAdapter);
void halReturnMsduToken(IN struct ADAPTER *prAdapter, uint32_t u4TokenNum);
void halReturnTimeoutMsduToken(struct ADAPTER *prAdapter);
void halTxUpdateCutThroughDesc(struct GLUE_INFO *prGlueInfo,
			       struct MSDU_INFO *prMsduInfo,
			       struct MSDU_TOKEN_ENTRY *prFillToken,
			       struct MSDU_TOKEN_ENTRY *prDataToken,
			       uint32_t u4Idx, bool fgIsLast);
u_int8_t halIsStaticMapBusAddr(IN struct ADAPTER *prAdapter,
					IN uint32_t u4Addr);
u_int8_t halChipToStaticMapBusAddr(IN struct GLUE_INFO *prGlueInfo,
				   IN uint32_t u4ChipAddr,
				   OUT uint32_t *pu4BusAddr);
u_int8_t halGetDynamicMapReg(IN struct GLUE_INFO *prGlueInfo,
			     IN uint32_t u4ChipAddr,
			     OUT uint32_t *pu4Value);
u_int8_t halSetDynamicMapReg(IN struct GLUE_INFO *prGlueInfo,
			     IN uint32_t u4ChipAddr,
			     IN uint32_t u4Value);
void halConnacWpdmaConfig(struct GLUE_INFO *prGlueInfo, u_int8_t enable);
void halConnacEnableInterrupt(IN struct ADAPTER *prAdapter);
bool halWpdmaWriteCmd(struct GLUE_INFO *prGlueInfo,
		      struct CMD_INFO *prCmdInfo,
		      uint8_t ucTC);
bool halWpdmaWriteMsdu(struct GLUE_INFO *prGlueInfo,
		       struct MSDU_INFO *prMsduInfo,
		       struct list_head *prCurList);
bool halWpdmaWriteAmsdu(struct GLUE_INFO *prGlueInfo,
			struct list_head *prList,
			uint32_t u4Num, uint16_t u2Size);
void halWpdmaFreeMsdu(struct GLUE_INFO *prGlueInfo,
		      struct MSDU_INFO *prMsduInfo,
		      bool fgSetEvent);
void halWpdmaFreeMsduTasklet(unsigned long data);


bool kalDevReadData(struct GLUE_INFO *prGlueInfo, uint16_t u2Port,
		    struct SW_RFB *prSwRfb);
bool kalDevKickCmd(struct GLUE_INFO *prGlueInfo);

/* SER functions */
void halSetDrvSer(struct ADAPTER *prAdapter);
#if KERNEL_VERSION(4, 15, 0) <= LINUX_VERSION_CODE
void halHwRecoveryTimeout(struct timer_list *timer);
#else
void halHwRecoveryTimeout(unsigned long arg);
#endif
void halHwRecoveryFromError(IN struct ADAPTER *prAdapter);

/* Debug functions */
#if KERNEL_VERSION(5, 0, 0) <= LINUX_VERSION_CODE
int halTimeCompare(struct timespec64 *prTs1, struct timespec64 *prTs2);
#else
int halTimeCompare(struct timeval *prTs1, struct timeval *prTs2);
#endif
void halShowPdmaInfo(IN struct ADAPTER *prAdapter);
bool halShowHostCsrInfo(IN struct ADAPTER *prAdapter);
void kalDumpTxRing(struct GLUE_INFO *prGlueInfo,
		   struct RTMP_TX_RING *prTxRing,
		   uint32_t u4Num, bool fgDumpContent);
void kalDumpRxRing(struct GLUE_INFO *prGlueInfo,
		   struct RTMP_RX_RING *prRxRing,
		   uint32_t u4Num, bool fgDumpContent);
void haldumpPhyInfo(struct ADAPTER *prAdapter);
u_int8_t halProcessToken(IN struct ADAPTER *prAdapter,
	IN uint32_t u4Token,
	IN struct QUE *prFreeQueue);
#if (CFG_SUPPORT_CONNAC3X == 1)
int wf_ioremap_read(phys_addr_t addr, unsigned int *val);
int wf_ioremap_write(phys_addr_t addr, unsigned int val);
#endif
#endif /* HIF_PDMA_H__ */
