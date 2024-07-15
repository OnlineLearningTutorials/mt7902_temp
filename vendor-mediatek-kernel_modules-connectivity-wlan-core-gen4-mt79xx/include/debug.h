/* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause */
/*
 * Copyright (c) 2016 MediaTek Inc.
 */
/*
 ** Id: include/debug.h
 */

/*! \file   debug.h
 *    \brief  Definition of SW debugging level.
 *
 *    In this file, it describes the definition of various SW debugging levels
 *    and assert functions.
 */

#ifndef _DEBUG_H
#define _DEBUG_H

/*******************************************************************************
 *                         C O M P I L E R   F L A G S
 *******************************************************************************
 */
#ifndef BUILD_QA_DBG
#define BUILD_QA_DBG 0
#endif

#define DBG_DISABLE_ALL_LOG             0

/*******************************************************************************
 *                    E X T E R N A L   R E F E R E N C E S
 *******************************************************************************
 */
#include "gl_typedef.h"

extern uint8_t aucDebugModule[];
extern uint32_t au4LogLevel[];

extern void set_logtoomuch_enable(int value) __attribute__((weak));
extern int get_logtoomuch_enable(void) __attribute__((weak));

extern struct MIB_INFO_STAT g_arMibInfo[ENUM_BAND_NUM];

/*******************************************************************************
 *                              C O N S T A N T S
 *******************************************************************************
 */
/* Define debug category (class):
 * (1) ERROR (2) WARN (3) STATE (4) EVENT (5) TRACE (6) INFO (7) LOUD (8) TEMP
 */
#define DBG_CLASS_ERROR         BIT(0)
#define DBG_CLASS_WARN          BIT(1)
#define DBG_CLASS_STATE         BIT(2)
#define DBG_CLASS_EVENT         BIT(3)
#define DBG_CLASS_TRACE         BIT(4)
#define DBG_CLASS_INFO          BIT(5)
#define DBG_CLASS_LOUD          BIT(6)
#define DBG_CLASS_TEMP          BIT(7)
#define DBG_CLASS_MASK          BITS(0, 7)

#define DBG_LOG_LEVEL_DEFAULT \
	(DBG_CLASS_ERROR | \
	DBG_CLASS_WARN | \
	DBG_CLASS_STATE | \
	DBG_CLASS_EVENT | \
	DBG_CLASS_INFO)
#define DBG_LOG_LEVEL_MORE \
	(DBG_LOG_LEVEL_DEFAULT | \
	DBG_CLASS_TRACE)
#define DBG_LOG_LEVEL_EXTREME \
	(DBG_LOG_LEVEL_MORE | \
	DBG_CLASS_LOUD)

#if defined(LINUX)
#define DBG_PRINTF_64BIT_DEC    "lld"
#else /* Windows */
#define DBG_PRINTF_64BIT_DEC    "I64d"
#endif
#define DBG_ALL_MODULE_IDX      0xFFFFFFFF

#define DEG_HIF_ALL             BIT(0)
#define DEG_HIF_HOST_CSR        BIT(1)
#define DEG_HIF_PDMA            BIT(2)
#define DEG_HIF_DMASCH          BIT(3)
#define DEG_HIF_PSE             BIT(4)
#define DEG_HIF_PLE             BIT(5)
#define DEG_HIF_MAC             BIT(6)
#define DEG_HIF_PHY             BIT(7)

#define DEG_HIF_DEFAULT_DUMP					\
	(DEG_HIF_HOST_CSR | DEG_HIF_PDMA | DEG_HIF_DMASCH |	\
	 DEG_HIF_PSE | DEG_HIF_PLE)

#define HIF_CHK_TX_HANG         BIT(1)
#define HIF_DRV_SER             BIT(2)

#define DUMP_MEM_SIZE 64

#if CFG_SUPPORT_ADVANCE_CONTROL
#define CMD_SW_DBGCTL_ADVCTL_SET_ID 0xa1260000
#define CMD_SW_DBGCTL_ADVCTL_GET_ID 0xb1260000
#endif

#define CFG_STAT_DBG_PEER_NUM		10
#define AGG_RANGE_SEL_4BYTE_NUM		4

#define AGG_RANGE_SEL_0_MASK		BITS(0, 7)
#define AGG_RANGE_SEL_0_OFFSET		0
#define AGG_RANGE_SEL_1_MASK		BITS(8, 15)
#define AGG_RANGE_SEL_1_OFFSET		8
#define AGG_RANGE_SEL_2_MASK		BITS(16, 23)
#define AGG_RANGE_SEL_2_OFFSET		16
#define AGG_RANGE_SEL_3_MASK		BITS(24, 31)
#define AGG_RANGE_SEL_3_OFFSET		24
#define AGG_RANGE_SEL_4_MASK		AGG_RANGE_SEL_0_MASK
#define AGG_RANGE_SEL_4_OFFSET		AGG_RANGE_SEL_0_OFFSET
#define AGG_RANGE_SEL_5_MASK		AGG_RANGE_SEL_1_MASK
#define AGG_RANGE_SEL_5_OFFSET		AGG_RANGE_SEL_1_OFFSET
#define AGG_RANGE_SEL_6_MASK		AGG_RANGE_SEL_2_MASK
#define AGG_RANGE_SEL_6_OFFSET		AGG_RANGE_SEL_2_OFFSET

/*******************************************************************************
 *                             D A T A   T Y P E S
 *******************************************************************************
 */
/* Define debug module index */
enum ENUM_DBG_MODULE {
	DBG_INIT_IDX = 0,	/* 0x00 *//* For driver initial */
	DBG_HAL_IDX,		/* 0x01 *//* For HAL(HW) Layer */
	DBG_INTR_IDX,		/* 0x02 *//* For Interrupt */
	DBG_REQ_IDX,		/* 0x03 */
	DBG_TX_IDX,		/* 0x04 */
	DBG_RX_IDX,		/* 0x05 */
	DBG_RFTEST_IDX,		/* 0x06 *//* For RF test mode */
	DBG_EMU_IDX,		/* 0x07 *//* Developer specific */
	DBG_SW1_IDX,		/* 0x08 *//* Developer specific */
	DBG_SW2_IDX,		/* 0x09 *//* Developer specific */
	DBG_SW3_IDX,		/* 0x0A *//* Developer specific */
	DBG_SW4_IDX,		/* 0x0B *//* Developer specific */
	DBG_HEM_IDX,		/* 0x0C *//* HEM */
	DBG_AIS_IDX,		/* 0x0D *//* AIS */
	DBG_RLM_IDX,		/* 0x0E *//* RLM */
	DBG_MEM_IDX,		/* 0x0F *//* RLM */
	DBG_CNM_IDX,		/* 0x10 *//* CNM */
	DBG_RSN_IDX,		/* 0x11 *//* RSN */
	DBG_BSS_IDX,		/* 0x12 *//* BSS */
	DBG_SCN_IDX,		/* 0x13 *//* SCN */
	DBG_SAA_IDX,		/* 0x14 *//* SAA */
	DBG_AAA_IDX,		/* 0x15 *//* AAA */
	DBG_P2P_IDX,		/* 0x16 *//* P2P */
	DBG_QM_IDX,		/* 0x17 *//* QUE_MGT */
	DBG_SEC_IDX,		/* 0x18 *//* SEC */
	DBG_BOW_IDX,		/* 0x19 *//* BOW */
	DBG_WAPI_IDX,		/* 0x1A *//* WAPI */
	DBG_ROAMING_IDX,	/* 0x1B *//* ROAMING */
	DBG_TDLS_IDX,		/* 0x1C *//* TDLS *//* CFG_SUPPORT_TDLS */
	DBG_PF_IDX,		/* 0x1D *//* PF */
	DBG_OID_IDX,
	DBG_NIC_IDX,
	DBG_WNM_IDX,
	DBG_WMM_IDX,
	DBG_TWT_REQUESTER_IDX,
	DBG_TWT_PLANNER_IDX,
	DBG_TWT_RESPONDER_IDX,
	DBG_RRM_IDX,
#if (CFG_TX_RSRC_WMM_ENHANCE == 1)
	DBG_HIF_WMM_ENHANCE_IDX,
#endif
#if CFG_SUPPORT_NAN
	DBG_NAN_IDX,
#endif
	DBG_MODULE_NUM		/* Notice the XLOG check */
};
enum ENUM_DBG_ASSERT_CTRL_LEVEL {
	DBG_ASSERT_CTRL_LEVEL_ERROR,
	DBG_ASSERT_CTRL_LEVEL_WARN,
	DBG_ASSERT_CTRL_LEVEL_LITE
};
enum ENUM_DBG_ASSERT_PATH {
	DBG_ASSERT_PATH_WIFI,
	DBG_ASSERT_PATH_WMT
};
enum ENUM_MCU_DBG_FLAG {
	DBG_MCU_DBG_CURRENT_PC,
	DBG_MCU_DBG_ALL
};

enum FAIL_SCENARIO {
	SLEEP = 0,
	SLAVENORESP,
};

#if (CFG_SUPPORT_CONNAC3X == 1)
struct wfdma_group_info {
	char name[20];
	u_int32_t hw_desc_base;
	bool dump_ring_content;

	uint32_t cnt;
	uint32_t cidx;
	uint32_t didx;
};

struct pse_group_info {
	char name[20];
	u_int32_t quota_addr;
	u_int32_t pg_info_addr;
};

struct CODA_CR_INFO {
	uint32_t u4Addr;
	uint32_t u4Mask;
	uint32_t u4Shift;
};

enum ENUM_DMASHDL_GROUP_IDX {
	ENUM_DMASHDL_GROUP_0 = 0,
	ENUM_DMASHDL_GROUP_1,
	ENUM_DMASHDL_GROUP_2,
	ENUM_DMASHDL_GROUP_3,
	ENUM_DMASHDL_GROUP_4,
	ENUM_DMASHDL_GROUP_5,
	ENUM_DMASHDL_GROUP_6,
	ENUM_DMASHDL_GROUP_7,
	ENUM_DMASHDL_GROUP_8,
	ENUM_DMASHDL_GROUP_9,
	ENUM_DMASHDL_GROUP_10,
	ENUM_DMASHDL_GROUP_11,
	ENUM_DMASHDL_GROUP_12,
	ENUM_DMASHDL_GROUP_13,
	ENUM_DMASHDL_GROUP_14,
	ENUM_DMASHDL_GROUP_15,
	ENUM_DMASHDL_GROUP_NUM
};

struct DMASHDL_CFG {
	u_int8_t fgSlotArbiterEn;
	uint16_t u2PktPleMaxPage;
	uint16_t u2PktPseMaxPage;
	u_int8_t afgRefillEn[ENUM_DMASHDL_GROUP_NUM];
	uint16_t au2MaxQuota[ENUM_DMASHDL_GROUP_NUM];
	uint16_t au2MinQuota[ENUM_DMASHDL_GROUP_NUM];
	uint8_t aucQueue2Group[ENUM_DMASHDL_GROUP_NUM * 2];
	uint8_t aucPriority2Group[ENUM_DMASHDL_GROUP_NUM];
	uint16_t u2HifAckCntTh;
	uint16_t u2HifGupActMap;
	uint32_t u4GroupNum;

	struct CODA_CR_INFO rPlePacketMaxSize;
	struct CODA_CR_INFO rPsePacketMaxSize;
	struct CODA_CR_INFO rGroup0RefillDisable;
	struct CODA_CR_INFO rGroup0ControlMaxQuota;
	struct CODA_CR_INFO rGroup0ControlMinQuota;
	struct CODA_CR_INFO rQueueMapping0Queue0;
	struct CODA_CR_INFO rGroup0MaxQuota;
	struct CODA_CR_INFO rPageSettingGroupSeqOrderType;
	struct CODA_CR_INFO rSchdulerSetting0Priority0Group;
	struct CODA_CR_INFO rStatusRdGp0RsvCnt;
	struct CODA_CR_INFO rStatusRdGp0SrcCnt;
	struct CODA_CR_INFO rRdGroupPktCnt0;
	struct CODA_CR_INFO rOptionalControlCrHifAckCntTh;
	struct CODA_CR_INFO rOptionalControlCrHifGupActMap;
	struct CODA_CR_INFO rErrorFlagCtrl;
	struct CODA_CR_INFO rStatusRdFfaCnt;
	struct CODA_CR_INFO rStatusRdFreePageCnt;
	struct CODA_CR_INFO rHifPgInfoHifRsvCnt;
	struct CODA_CR_INFO rHifPgInfoHifSrcCnt;
};

struct PLE_TOP_CR {
	struct CODA_CR_INFO rAc0QueueEmpty0;
	struct CODA_CR_INFO rAc1QueueEmpty0;
	struct CODA_CR_INFO rAc2QueueEmpty0;
	struct CODA_CR_INFO rAc3QueueEmpty0;
	struct CODA_CR_INFO rCpuPgInfo;
	struct CODA_CR_INFO rCpuPgInfoCpuRsvCnt;
	struct CODA_CR_INFO rCpuPgInfoCpuSrcCnt;
	struct CODA_CR_INFO rDisStaMap0;
	struct CODA_CR_INFO rFlQueCtrl0;
	struct CODA_CR_INFO rFlQueCtrl0Execute;
	struct CODA_CR_INFO rFlQueCtrl0QBufPid;
	struct CODA_CR_INFO rFlQueCtrl0QBufQid;
	struct CODA_CR_INFO rFlQueCtrl0QBufWlanid;
	struct CODA_CR_INFO rFlQueCtrl2;
	struct CODA_CR_INFO rFlQueCtrl2QueueHeadFid;
	struct CODA_CR_INFO rFlQueCtrl2QueueTailFid;
	struct CODA_CR_INFO rFlQueCtrl3;
	struct CODA_CR_INFO rFlQueCtrl3QueuePktNum;
	struct CODA_CR_INFO rFreepgCnt;
	struct CODA_CR_INFO rFreepgCntFfaCnt;
	struct CODA_CR_INFO rFreepgCntFreepgCnt;
	struct CODA_CR_INFO rFreepgHeadTail;
	struct CODA_CR_INFO rFreepgHeadTailFreepgHead;
	struct CODA_CR_INFO rFreepgHeadTailFreepgTail;
	struct CODA_CR_INFO rFsmPeekCr00;
	struct CODA_CR_INFO rFsmPeekCr01;
	struct CODA_CR_INFO rFsmPeekCr02;
	struct CODA_CR_INFO rFsmPeekCr03;
	struct CODA_CR_INFO rFsmPeekCr04;
	struct CODA_CR_INFO rFsmPeekCr05;
	struct CODA_CR_INFO rFsmPeekCr06;
	struct CODA_CR_INFO rFsmPeekCr07;
	struct CODA_CR_INFO rFsmPeekCr08;
	struct CODA_CR_INFO rFsmPeekCr09;
	struct CODA_CR_INFO rFsmPeekCr10;
	struct CODA_CR_INFO rFsmPeekCr11;
	struct CODA_CR_INFO rHifPgInfo;
	struct CODA_CR_INFO rHifPgInfoHifRsvCnt;
	struct CODA_CR_INFO rHifPgInfoHifSrcCnt;
	struct CODA_CR_INFO rHifTxcmdPgInfo;
	struct CODA_CR_INFO rHifTxcmdPgInfoHifTxcmdRsvCnt;
	struct CODA_CR_INFO rHifTxcmdPgInfoHifTxcmdSrcCnt;
	struct CODA_CR_INFO rHifWmtxdPgInfo;
	struct CODA_CR_INFO rHifWmtxdPgInfoHifWmtxdRsvCnt;
	struct CODA_CR_INFO rHifWmtxdPgInfoHifWmtxdSrcCnt;
	struct CODA_CR_INFO rIntN9ErrSts;
	struct CODA_CR_INFO rIntN9ErrSts1;
	struct CODA_CR_INFO rPbufCtrl;
	struct CODA_CR_INFO rPbufCtrlPageSizeCfg;
	struct CODA_CR_INFO rPbufCtrlPbufOffset;
	struct CODA_CR_INFO rPbufCtrlTotalPageNum;
	struct CODA_CR_INFO rPgCpuGroup;
	struct CODA_CR_INFO rPgCpuGroupCpuMaxQuota;
	struct CODA_CR_INFO rPgCpuGroupCpuMinQuota;
	struct CODA_CR_INFO rPgHifGroup;
	struct CODA_CR_INFO rPgHifGroupHifMaxQuota;
	struct CODA_CR_INFO rPgHifGroupHifMinQuota;
	struct CODA_CR_INFO rPgHifTxcmdGroup;
	struct CODA_CR_INFO rPgHifTxcmdGroupHifTxcmdMaxQuota;
	struct CODA_CR_INFO rPgHifTxcmdGroupHifTxcmdMinQuota;
	struct CODA_CR_INFO rPgHifWmtxdGroup;
	struct CODA_CR_INFO rPgHifWmtxdGroupHifWmtxdMaxQuota;
	struct CODA_CR_INFO rPgHifWmtxdGroupHifWmtxdMinQuota;
	struct CODA_CR_INFO rQueueEmpty;
	struct CODA_CR_INFO rQueueEmptyAllAcEmpty;
	struct CODA_CR_INFO rStationPause0;
	struct CODA_CR_INFO rTxdQueueEmpty;
	struct CODA_CR_INFO rToN9IntToggle;
};

struct PSE_TOP_CR {
	struct CODA_CR_INFO rFlQueCtrl0;
	struct CODA_CR_INFO rFlQueCtrl0Execute;
	struct CODA_CR_INFO rFlQueCtrl0QBufPid;
	struct CODA_CR_INFO rFlQueCtrl0QBufQid;
	struct CODA_CR_INFO rFlQueCtrl2;
	struct CODA_CR_INFO rFlQueCtrl2QueueHeadFid;
	struct CODA_CR_INFO rFlQueCtrl2QueueTailFid;
	struct CODA_CR_INFO rFlQueCtrl3;
	struct CODA_CR_INFO rFlQueCtrl3QueuePktNum;
	struct CODA_CR_INFO rFreepgCnt;
	struct CODA_CR_INFO rFreepgCntFfaCnt;
	struct CODA_CR_INFO rFreepgCntFreepgCnt;
	struct CODA_CR_INFO rFreepgHeadTail;
	struct CODA_CR_INFO rFreepgHeadTailFreepgHead;
	struct CODA_CR_INFO rFreepgHeadTailFreepgTail;
	struct CODA_CR_INFO rFsmPeekCr00;
	struct CODA_CR_INFO rFsmPeekCr01;
	struct CODA_CR_INFO rFsmPeekCr02;
	struct CODA_CR_INFO rFsmPeekCr03;
	struct CODA_CR_INFO rFsmPeekCr04;
	struct CODA_CR_INFO rFsmPeekCr05;
	struct CODA_CR_INFO rFsmPeekCr06;
	struct CODA_CR_INFO rFsmPeekCr07;
	struct CODA_CR_INFO rFsmPeekCr08;
	struct CODA_CR_INFO rFsmPeekCr09;
	struct CODA_CR_INFO rHif0PgInfoHif0RsvCnt;
	struct CODA_CR_INFO rHif0PgInfoHif0SrcCnt;
	struct CODA_CR_INFO rIntN9Err1Sts;
	struct CODA_CR_INFO rIntN9ErrSts;
	struct CODA_CR_INFO rPbufCtrl;
	struct CODA_CR_INFO rPbufCtrlPageSizeCfg;
	struct CODA_CR_INFO rPbufCtrlPbufOffset;
	struct CODA_CR_INFO rPbufCtrlTotalPageNum;
	struct CODA_CR_INFO rPgHif0GroupHif0MaxQuota;
	struct CODA_CR_INFO rPgHif0GroupHif0MinQuota;
	struct CODA_CR_INFO rQueueEmpty;
	struct CODA_CR_INFO rQueueEmptyCpuQ0Empty;
	struct CODA_CR_INFO rQueueEmptyCpuQ1Empty;
	struct CODA_CR_INFO rQueueEmptyCpuQ2Empty;
	struct CODA_CR_INFO rQueueEmptyCpuQ3Empty;
	struct CODA_CR_INFO rQueueEmptyHif0Empty;
	struct CODA_CR_INFO rQueueEmptyHif10Empty;
	struct CODA_CR_INFO rQueueEmptyHif11Empty;
	struct CODA_CR_INFO rQueueEmptyHif1Empty;
	struct CODA_CR_INFO rQueueEmptyHif2Empty;
	struct CODA_CR_INFO rQueueEmptyHif3Empty;
	struct CODA_CR_INFO rQueueEmptyHif4Empty;
	struct CODA_CR_INFO rQueueEmptyHif5Empty;
	struct CODA_CR_INFO rQueueEmptyHif6Empty;
	struct CODA_CR_INFO rQueueEmptyHif7Empty;
	struct CODA_CR_INFO rQueueEmptyHif8Empty;
	struct CODA_CR_INFO rQueueEmptyHif9Empty;
	struct CODA_CR_INFO rQueueEmptyLmacTxQueueEmpty;
	struct CODA_CR_INFO rQueueEmptyMdpRxQueueEmpty;
	struct CODA_CR_INFO rQueueEmptyMdpRxioc1QueueEmpty;
	struct CODA_CR_INFO rQueueEmptyMdpRxiocQueueEmpty;
	struct CODA_CR_INFO rQueueEmptyMdpTx1QueueEmpty;
	struct CODA_CR_INFO rQueueEmptyMdpTxQueueEmpty;
	struct CODA_CR_INFO rQueueEmptyMdpTxioc1QueueEmpty;
	struct CODA_CR_INFO rQueueEmptyMdpTxiocQueueEmpty;
	struct CODA_CR_INFO rQueueEmptyRlsQEmtpy;
	struct CODA_CR_INFO rQueueEmptySecRxQueueEmpty;
	struct CODA_CR_INFO rQueueEmptySecTx1QueueEmpty;
	struct CODA_CR_INFO rQueueEmptySecTxQueueEmpty;
	struct CODA_CR_INFO rQueueEmptySfdParkQueueEmpty;
};

struct PP_TOP_CR {
	struct CODA_CR_INFO rDbgCtrl;
	struct CODA_CR_INFO rDbgCs0;
	struct CODA_CR_INFO rDbgCs1;
	struct CODA_CR_INFO rDbgCs2;
};

enum _ENUM_WFDMA_TYPE_T {
	WFDMA_TYPE_HOST = 0,
	WFDMA_TYPE_WM
};

#endif /* #if (CFG_SUPPORT_CONNAC3X == 1) */

struct CHIP_DBG_OPS {
	void (*showPdmaInfo)(struct ADAPTER *prAdapter);
	void (*showPseInfo)(struct ADAPTER *prAdapter);
	void (*showPleInfo)(struct ADAPTER *prAdapter, u_int8_t fgDumpTxd);
	void (*showTxdInfo)(struct ADAPTER *prAdapter, u_int32_t fid);
	bool (*showCsrInfo)(struct ADAPTER *prAdapter);
	void (*showDmaschInfo)(struct ADAPTER *prAdapter);
	int32_t (*showWtblInfo)(
		struct ADAPTER *prAdapter,
		uint32_t u4Index,
		char *pcCommand,
		int32_t i4TotalLen);
#if (CFG_SUPPORT_CONNAC2X == 1)
	int32_t (*showUmacFwtblInfo)(
		struct ADAPTER *prAdapter,
		uint32_t u4Index,
		char *pcCommand,
		int32_t i4TotalLen);
#endif /* CFG_SUPPORT_CONNAC2X == 1 */
	void (*showHifInfo)(struct ADAPTER *prAdapter);
	void (*printHifDbgInfo)(struct ADAPTER *prAdapter);
	int32_t (*show_rx_rate_info)(
		struct ADAPTER *prAdapter,
		char *pcCommand,
		int32_t i4TotalLen,
		uint8_t ucStaIdx);
	int32_t (*show_rx_rssi_info)(
		struct ADAPTER *prAdapter,
		char *pcCommand,
		int32_t i4TotalLen,
		uint8_t ucStaIdx);
	int32_t (*show_stat_info)(
		struct ADAPTER *prAdapter,
		char *pcCommand,
		int32_t i4TotalLen,
		struct PARAM_HW_WLAN_INFO *prHwWlanInfo,
		struct PARAM_GET_STA_STATISTICS *prQueryStaStatistics,
		uint8_t fgResetCnt,
		uint32_t u4StatGroup);
#if (CFG_SUPPORT_CONNAC3X == 1)
	void (*show_wfdma_dbg_probe_info)(IN struct ADAPTER *prAdapter,
		IN enum _ENUM_WFDMA_TYPE_T enum_wfdma_type);
	void (*show_wfdma_wrapper_info)(IN struct ADAPTER *prAdapter,
		IN enum _ENUM_WFDMA_TYPE_T enum_wfdma_type);
#endif
#if CFG_SUPPORT_LINK_QUALITY_MONITOR
	int (*get_rx_rate_info)(
		struct ADAPTER *prAdapter,
		uint32_t *pu4Rate,
		uint32_t *pu4Nsts,
		uint32_t *pu4RxMode,
		uint32_t *pu4FrMode,
		uint32_t *pu4Sgi,
		uint32_t *pu4GrpId);
#endif
	u_int8_t (*show_mcu_debug_info)(
		struct ADAPTER *prAdapter,
		IN uint8_t *pucBuf,
		IN uint32_t u4Max,
		IN uint8_t ucFlag,
		OUT uint32_t *pu4Length);
	u_int8_t (*show_conninfra_debug_info)(
		struct ADAPTER *prAdapter,
		IN uint8_t *pucBuf,
		IN uint32_t u4Max,
		IN uint8_t ucFlag,
		OUT uint32_t *pu4Length);
#if (CFG_SUPPORT_DEBUG_SOP == 1)
	u_int8_t (*show_debug_sop_info)(
		struct ADAPTER *prAdapter,
		uint8_t ucCase);
#endif
#if (CFG_SUPPORT_CONNAC3X == 1)
	void (*dumpPhyInfo)(struct ADAPTER *prAdapter);
	int32_t (*showMibInfo)(struct ADAPTER *prAdapter,
		uint32_t u4Index,
		char *pcCommand,
		int32_t i4TotalLen);
#endif
#if defined(_HIF_SDIO)
	u_int8_t (*get_sdio_debug_info)(
		struct ADAPTER *prAdapter);
#endif
};

enum PKT_PHASE {
	PHASE_XMIT_RCV,
	PHASE_ENQ_QM,
	PHASE_HIF_TX,
};

struct WLAN_DEBUG_INFO {
	u_int8_t fgVoE5_7Test:1;
	u_int8_t fgReserved:7;
};

#if MTK_WCN_HIF_SDIO
#define DBG_ASSERT_PATH_DEFAULT DBG_ASSERT_PATH_WMT
#else
#define DBG_ASSERT_PATH_DEFAULT DBG_ASSERT_PATH_WIFI
#endif
#define DBG_ASSERT_CTRL_LEVEL_DEFAULT DBG_ASSERT_CTRL_LEVEL_ERROR
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
/* Debug print format string for the OS system time */
#define OS_SYSTIME_DBG_FORMAT               "0x%08x"
/* Debug print argument for the OS system time */
#define OS_SYSTIME_DBG_ARGUMENT(systime)    (systime)
#if CFG_SHOW_FULL_MACADDR
/* Debug print format string for the MAC Address */
#define MACSTR		"%pM"
/* Debug print argument for the MAC Address */
#define MAC2STR(a)	a
#else
#define MACSTR          "%02x:%02x:**:**:**:%02x"
#define MAC2STR(a)   ((uint8_t *)a)[0], ((uint8_t *)a)[1], ((uint8_t *)a)[5]
#endif
#define PMKSTR "%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%03x%02x%02x"

#if CFG_SHOW_FULL_IPADDR
/* Debug print format string for the IPv4 Address */
#define IPV4STR		"%pI4"
/* Debug print argument for the IPv4 Address */
#define IPV4TOSTR(a)	a
/* Debug print format string for the IPv6 Address */
#define IPV6STR		"%pI6"
/* Debug print argument for the IPv6 Address */
#define IPV6TOSTR(a)	a
#else
#define IPV4STR		"%d.***.***.%d"
#define IPV4TOSTR(a)	((uint8_t *)a)[0], ((uint8_t *)a)[3]
#define IPV6STR		"%02x%02x:**:**:**:**:**:**:%02x%02x"
#define IPV6TOSTR(a)	((uint8_t *)a)[0], ((uint8_t *)a)[1], \
				((uint8_t *)a)[14], ((uint8_t *)a)[15]
#endif

#if CFG_SHOW_SSID
#define HIDE(_str) _str
#else
#define HIDE(_str) "***"
#endif

/* The pre-defined format to dump the varaible value with its name shown. */
#define DUMPVAR(variable, format)   (#variable " = " format "\n", variable)
/* The pre-defined format to dump the MAC type value with its name shown. */
#define DUMPMACADDR(addr)           (#addr " = " MACSTR "\n", MAC2STR(addr))
/* Debug print format string for the floating point */
#define FPSTR		"%u.%u"
/* Debug print argument for the floating point */
#define DIV2INT(_dividend, _divisor) \
		((_divisor) ? (_dividend) / (_divisor) : 0)
#define DIV2DEC(_dividend, _divisor) \
		((_divisor) ? (((_dividend) * 100) / (_divisor)) % 100 : 0)
/* Basiclly, we just do renaming of KAL functions although they should
 * be defined as "Nothing to do" if DBG=0. But in some compiler, the macro
 * syntax does not support  #define LOG_FUNC(x,...)
 *
 * A caller shall not invoke these three macros when DBG=0.
 */
#define LOG_FUNC                kalPrint
#define LOG_FUNC_LIMITED	kalPrintLimited

/* If __FUNCTION__ is already defined by compiler, we just use it. */
#define DEBUGFUNC(_Func)
/* Disabled due to AOSP
 * #if defined(__FUNCTION__)
 *    #define DEBUGFUNC(_Func)
 * #else
 *    #define DEBUGFUNC(_Func) static const char __FUNCTION__[] = _Func;
 * #endif
 */
#if DBG_DISABLE_ALL_LOG
#define DBGLOG(_Module, _Class, _Fmt)
#define DBGLOG_LIMITED(_Module, _Class, _Fmt)
#define DBGLOG_MEM8(_Module, _Class, _StartAddr, _Length)
#define DBGLOG_MEM32(_Module, _Class, _StartAddr, _Length)
#else
#define DBGLOG(_Mod, _Clz, _Fmt, ...) \
	do { \
		if ((aucDebugModule[DBG_##_Mod##_IDX] & \
			 DBG_CLASS_##_Clz) == 0) \
			break; \
		LOG_FUNC("[%u]%s:(" #_Mod " " #_Clz ") " _Fmt, \
			 KAL_GET_CURRENT_THREAD_ID(), \
			 __func__, ##__VA_ARGS__); \
	} while (0)
#define DBGLOG_LIMITED(_Mod, _Clz, _Fmt, ...) \
	do { \
		if ((aucDebugModule[DBG_##_Mod##_IDX] & \
			 DBG_CLASS_##_Clz) == 0) \
			break; \
		LOG_FUNC_LIMITED("[%u]%s:(" #_Mod " " #_Clz ") " _Fmt, \
			 KAL_GET_CURRENT_THREAD_ID(), \
			 __func__, ##__VA_ARGS__); \
	} while (0)
#define DBGFWLOG(_Mod, _Clz, _Fmt, ...) \
	do { \
		if ((aucDebugModule[DBG_##_Mod##_IDX] & \
			 DBG_CLASS_##_Clz) == 0) \
			break; \
		wlanPrintFwLog(NULL, 0, DEBUG_MSG_TYPE_DRIVER, \
			 "[%u]%s:(" #_Mod " " #_Clz ") " _Fmt, \
			 KAL_GET_CURRENT_THREAD_ID(), \
			 __func__, ##__VA_ARGS__); \
	} while (0)
#define TOOL_PRINTLOG(_Mod, _Clz, _Fmt, ...) \
	do { \
		if ((aucDebugModule[DBG_##_Mod##_IDX] & \
			 DBG_CLASS_##_Clz) == 0) \
			break; \
		LOG_FUNC(_Fmt, ##__VA_ARGS__); \
	} while (0)
#define DBGLOG_MEM8(_Mod, _Clz, _Adr, _Len) \
	{ \
		if (aucDebugModule[DBG_##_Mod##_IDX] & DBG_CLASS_##_Clz) { \
			LOG_FUNC("%s:(" #_Mod " " #_Clz ")\n", __func__); \
			dumpMemory8((uint8_t *)(_Adr), (uint32_t)(_Len)); \
		} \
	}
#define DBGLOG_MEM32(_Mod, _Clz, _Adr, _Len) \
	{ \
		if (aucDebugModule[DBG_##_Mod##_IDX] & DBG_CLASS_##_Clz) { \
			LOG_FUNC("%s:(" #_Mod " " #_Clz ")\n", __func__); \
			dumpMemory32((uint32_t *)(_Adr), (uint32_t)(_Len)); \
		} \
	}
#endif
#define DISP_STRING(_str)       _str
#undef ASSERT
#undef ASSERT_REPORT
#if (BUILD_QA_DBG || DBG)
#define ASSERT_NOMEM() \
{ \
	LOG_FUNC("alloate memory failed at %s:%d\n", __FILE__, __LINE__); \
	kalSendAeeWarning("Wlan_Gen4 No Mem", "Memory Alloate Failed %s:%d",\
		__FILE__, __LINE__); \
}
#ifdef _lint
#define ASSERT(_exp) \
	{ \
		if (!(_exp)) { \
			do {} while (1); \
		} \
	}
#define ASSERT_REPORT(_exp, _fmt) \
	{ \
		LOG_FUNC("Assertion failed: %s:%d (%s)\n", \
			__FILE__, __LINE__, #_exp); \
		LOG_FUNC _fmt; \
		if (!(_exp)) { \
			do {} while (1); \
		} \
	}
#elif defined(WINDOWS_CE)
#define UNICODE_TEXT(_msg)  TEXT(_msg)
#define ASSERT(_exp) \
	{ \
		if (!(_exp)) { \
			TCHAR rUbuf[256]; \
			kalBreakPoint(); \
			_stprintf(rUbuf, TEXT("Assertion failed: %s:%d %s\n"), \
				  UNICODE_TEXT(__FILE__), __LINE__, \
				  UNICODE_TEXT(#_exp)); \
			MessageBox(NULL, rUbuf, TEXT("ASSERT!"), MB_OK); \
		} \
	}
#define ASSERT_REPORT(_exp, _fmt) \
	{ \
		if (!(_exp)) { \
			TCHAR rUbuf[256]; \
			kalBreakPoint(); \
			_stprintf(rUbuf, TEXT("Assertion failed: %s:%d %s\n"), \
				  UNICODE_TEXT(__FILE__), __LINE__, \
				  UNICODE_TEXT(#_exp)); \
			MessageBox(NULL, rUbuf, TEXT("ASSERT!"), MB_OK); \
		} \
	}
#else
#define ASSERT_NOMEM() \
{ \
	LOG_FUNC("alloate memory failed at %s:%d\n", __FILE__, __LINE__); \
	kalSendAeeWarning("Wlan_Gen4 No Mem", "Memory Alloate Failed %s:%d",\
		__FILE__, __LINE__); \
}

#define ASSERT(_exp) \
	{ \
		if (!(_exp)) { \
			LOG_FUNC("Assertion failed: %s:%d (%s)\n", \
				__FILE__, __LINE__, #_exp); \
			kalBreakPoint(); \
		} \
	}
#define ASSERT_REPORT(_exp, _fmt) \
	{ \
		if (!(_exp)) { \
			LOG_FUNC("Assertion failed: %s:%d (%s)\n", \
				__FILE__, __LINE__, #_exp); \
			LOG_FUNC _fmt; \
			kalBreakPoint(); \
		} \
	}
#endif /* WINDOWS_CE */
#else
#define ASSERT_NOMEM() {}
#define ASSERT(_exp) {}
#define ASSERT_REPORT(_exp, _fmt) {}
#endif /* BUILD_QA_DBG */
/* LOG function for print to buffer */
/* If buffer pointer is NULL, redirect to normal DBGLOG */
#define LOGBUF(_pucBuf, _maxLen, _curLen, _Fmt, ...) \
	{ \
		if (_pucBuf) \
			(_curLen) += kalSnprintf((_pucBuf) + (_curLen), \
			(_maxLen) - (_curLen), _Fmt, ##__VA_ARGS__); \
		else \
			DBGLOG(SW4, INFO, _Fmt, ##__VA_ARGS__); \
	}
/* The following macro is used for debugging packed structures. */
#ifndef DATA_STRUCT_INSPECTING_ASSERT
#define DATA_STRUCT_INSPECTING_ASSERT(expr) \
{ \
	switch (0) {case 0: case (expr): default:; } \
}
#endif

/* Name alias of debug functions to skip check patch*/
#define log_dbg			DBGLOG
#define log_limited_dbg		DBGLOG_LIMITED
#define log_fw_dbg		DBGFWLOG
#define log_mem8_dbg		DBGLOG_MEM8
#define log_mem32_dbg		DBGLOG_MEM32
#define log_tool_dbg		TOOL_PRINTLOG

#define STR_HELPER(s) #s
/*******************************************************************************
 *                  F U N C T I O N   D E C L A R A T I O N S
 *******************************************************************************
 */
void dumpMemory8(IN uint8_t *pucStartAddr,
		 IN uint32_t u4Length);
void dumpMemory32(IN uint32_t *pu4StartAddr,
		  IN uint32_t u4Length);
void wlanPrintFwLog(uint8_t *pucLogContent,
		    uint16_t u2MsgSize, uint8_t ucMsgType,
		    const uint8_t *pucFmt, ...);

void wlanDbgLogLevelInit(void);
void wlanDbgLogLevelUninit(void);
uint32_t wlanDbgLevelUiSupport(IN struct ADAPTER *prAdapter,
		uint32_t u4Version, uint32_t ucModule);
uint32_t wlanDbgGetLogLevelImpl(IN struct ADAPTER *prAdapter,
		uint32_t u4Version, uint32_t ucModule);
void wlanDbgSetLogLevelImpl(IN struct ADAPTER *prAdapter,
		uint32_t u4Version, uint32_t u4Module, uint32_t u4level);
void wlanDriverDbgLevelSync(void);
u_int8_t wlanDbgGetGlobalLogLevel(uint32_t u4Module, uint32_t *pu4Level);
u_int8_t wlanDbgSetGlobalLogLevel(uint32_t u4Module, uint32_t u4Level);

void wlanFillTimestamp(struct ADAPTER *prAdapter, void *pvPacket,
		       uint8_t ucPhase);

void halShowPseInfo(IN struct ADAPTER *prAdapter);
void halShowPleInfo(IN struct ADAPTER *prAdapter,
	u_int8_t fgDumpTxd);
void halShowDmaschInfo(IN struct ADAPTER *prAdapter);
void haldumpMacInfo(IN struct ADAPTER *prAdapter);
void halGetPleTxdInfo(IN struct ADAPTER *prAdapter,
		      uint32_t fid, uint32_t *result);
void halGetPsePayload(IN struct ADAPTER *prAdapter,
		      uint32_t fid, uint32_t *result);
#if (CFG_SUPPORT_CONNAC3X == 0)
void halDumpTxdInfo(IN struct ADAPTER *prAdapter, uint32_t *tmac_info);
#endif
void halShowLitePleInfo(IN struct ADAPTER *prAdapter);
void halShowTxdInfo(
	struct ADAPTER *prAdapter,
	u_int32_t fid);


#if (CFG_SUPPORT_CONNAC2X == 1)
void connac2x_show_txd_Info(
	struct ADAPTER *prAdapter,
	u_int32_t fid);
int32_t connac2x_show_wtbl_info(
	struct ADAPTER *prAdapter,
	uint32_t u4Index,
	char *pcCommand,
	int i4TotalLen);
int32_t connac2x_show_umac_wtbl_info(
	struct ADAPTER *prAdapter,
	uint32_t u4Index,
	char *pcCommand,
	int i4TotalLen);

int32_t connac2x_show_rx_rate_info(
	struct ADAPTER *prAdapter,
	char *pcCommand,
	int32_t i4TotalLen,
	uint8_t ucStaIdx);

int32_t connac2x_show_rx_rssi_info(
	struct ADAPTER *prAdapter,
	char *pcCommand,
	int32_t i4TotalLen,
	uint8_t ucStaIdx);

int32_t connac2x_show_stat_info(
	struct ADAPTER *prAdapter,
	char *pcCommand,
	int32_t i4TotalLen,
	struct PARAM_HW_WLAN_INFO *prHwWlanInfo,
	struct PARAM_GET_STA_STATISTICS *prQueryStaStatistics,
	uint8_t fgResetCnt,
	uint32_t u4StatGroup);

#if CFG_SUPPORT_LINK_QUALITY_MONITOR
int connac2x_get_rx_rate_info(
	struct ADAPTER *prAdapter,
	uint32_t *pu4Rate,
	uint32_t *pu4Nsts,
	uint32_t *pu4RxMode,
	uint32_t *pu4FrMode,
	uint32_t *pu4Sgi,
	uint32_t *pu4GrpId);
#endif

#endif /* CFG_SUPPORT_CONNAC2X == 1 */

#if (CFG_SUPPORT_CONNAC3X == 1)
void connac3x_show_txd_Info(
	struct ADAPTER *prAdapter,
	u_int32_t fid);
int32_t connac3x_show_wtbl_info(
	struct ADAPTER *prAdapter,
	uint32_t u4Index,
	char *pcCommand,
	int i4TotalLen);
int32_t connac3x_show_mib_info(
	struct ADAPTER *prAdapter,
	uint32_t u4Index,
	char *pcCommand,
	int i4TotalLen);
int32_t connac3x_show_rx_rate_info(
	struct ADAPTER *prAdapter,
	char *pcCommand,
	int32_t i4TotalLen,
	uint8_t ucStaIdx);

int32_t connac3x_show_rx_rssi_info(
	struct ADAPTER *prAdapter,
	char *pcCommand,
	int32_t i4TotalLen,
	uint8_t ucStaIdx);

int32_t connac3x_show_stat_info(
	struct ADAPTER *prAdapter,
	char *pcCommand,
	int32_t i4TotalLen,
	struct PARAM_HW_WLAN_INFO *prHwWlanInfo,
	struct PARAM_GET_STA_STATISTICS *prQueryStaStatistics,
	uint8_t fgResetCnt,
	uint32_t u4StatGroup);

void connac3x_show_wfdma_interrupt_info(
	struct ADAPTER *prAdapter,
	enum _ENUM_WFDMA_TYPE_T enum_wfdma_type,
	uint32_t u4DmaNum);

void connac3x_show_wfdma_glo_info(
	struct ADAPTER *prAdapter,
	enum _ENUM_WFDMA_TYPE_T enum_wfdma_type,
	uint32_t u4DmaNum);

void connac3x_show_wfdma_ring_info(
	struct ADAPTER *prAdapter,
	enum _ENUM_WFDMA_TYPE_T enum_wfdma_type);

void connac3x_show_wfdma_dbg_flag_log(
	struct ADAPTER *prAdapter,
	enum _ENUM_WFDMA_TYPE_T enum_wfdma_type,
	uint32_t u4DmaNum);

void connac3x_show_wfdma_info_by_type(
	struct ADAPTER *prAdapter,
	enum _ENUM_WFDMA_TYPE_T enum_wfdma_type,
	uint32_t u4DmaNum);

void connac3x_show_wfdma_info(IN struct ADAPTER *prAdapter);
void connac3x_show_dmashdl_info(IN struct ADAPTER *prAdapter);
void connac3x_show_ple_info(struct ADAPTER *prAdapter, u_int8_t fgDumpTxd);
void connac3x_show_pse_info(struct ADAPTER *prAdapter);
void connac3x_DumpWfsyscpupcr(struct ADAPTER *prAdapter);
void connac3x_DbgCrRead(
	struct ADAPTER *prAdapter, uint32_t addr, unsigned int *val);
void connac3x_DbgCrWrite(
	struct ADAPTER *prAdapter, uint32_t addr, unsigned int val);
void connac3x_dump_format_memory32(
	uint32_t *pu4StartAddr, uint32_t u4Count, char *aucInfo);
void connac3x_DumpCrRange(
	struct ADAPTER *prAdapter,
	uint32_t cr_start, uint32_t word_count, char *str);
#ifdef CFG_SUPPORT_LINK_QUALITY_MONITOR
int connac3x_get_rx_rate_info(
	struct ADAPTER *prAdapter,
	uint32_t *pu4Rate,
	uint32_t *pu4Nss,
	uint32_t *pu4RxMode,
	uint32_t *pu4FrMode,
	uint32_t *pu4Sgi);
#endif

#endif /* CFG_SUPPORT_CONNAC3X == 1 */

#if (CFG_SUPPORT_RA_GEN == 1)
int32_t mt7663_show_stat_info(struct ADAPTER *prAdapter,
			char *pcCommand, int32_t i4TotalLen,
			struct PARAM_HW_WLAN_INFO *prHwWlanInfo,
			struct PARAM_GET_STA_STATISTICS *prQueryStaStatistics,
			uint8_t fgResetCnt, uint32_t u4StatGroup);
#endif

#if (CFG_SUPPORT_RA_GEN == 0)
int32_t mt7668_show_stat_info(struct ADAPTER *prAdapter,
			char *pcCommand, int32_t i4TotalLen,
			struct PARAM_HW_WLAN_INFO *prHwWlanInfo,
			struct PARAM_GET_STA_STATISTICS *prQueryStaStatistics,
			uint8_t fgResetCnt, uint32_t u4StatGroup);

int32_t mt6632_show_stat_info(struct ADAPTER *prAdapter,
			char *pcCommand, int32_t i4TotalLen,
			struct PARAM_HW_WLAN_INFO *prHwWlanInfo,
			struct PARAM_GET_STA_STATISTICS *prQueryStaStatistics,
			uint8_t fgResetCnt, uint32_t u4StatGroup);
#endif
/*******************************************************************************
 *                              F U N C T I O N S
 *******************************************************************************
 */
#endif /* _DEBUG_H */


