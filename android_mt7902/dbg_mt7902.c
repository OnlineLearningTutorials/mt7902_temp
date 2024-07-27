/* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause */
/*
 * Copyright (c) 2016 MediaTek Inc.
 */
/******************************************************************************
 *[File]             dbg_mt7902.c
 *[Version]          v1.0
 *[Revision Date]    2021-03-15
 *[Author]
 *[Description]
 *    The program provides WIFI FALCON MAC Debug APIs
 *[Copyright]
 *    Copyright (C) 2015 MediaTek Incorporation. All Rights Reserved.
 ******************************************************************************/

#if defined(MT7902)
/*******************************************************************************
 *                         C O M P I L E R   F L A G S
 *******************************************************************************
 */

/*******************************************************************************
 *                    E X T E R N A L   R E F E R E N C E S
 *******************************************************************************
 */
#include "precomp.h"
#include "wf_ple.h"
#include "coda/mt7902/wf_ple_top.h"
#include "coda/mt7902/wf_wfdma_host_dma0.h"
#include "mt7902.h"
#include "dbg_comm.h"

/*******************************************************************************
 *                              C O N S T A N T S
 *******************************************************************************
 */

/*******************************************************************************
 *                             D A T A   T Y P E S
 *******************************************************************************
 */
 #if (CFG_SUPPORT_DEBUG_SOP == 1)
struct MT7902_DEBUG_SOP_INFO {
	u_int32_t	*wfsys_sleep_status;
	uint8_t		wfsys_sleep_cr_num;
	u_int32_t	*wfsys_csr_status;
	uint8_t		wfsys_csr_cr_num;
	u_int32_t	*wfsys_status;
	uint8_t		wfsys_cr_num;
	u_int32_t	*bgfsys_bus_status;
	uint8_t		bgfsys_bus_cr_num;
	u_int32_t	*bgfsys_status;
	uint8_t		bgfsys_cr_num;
	u_int32_t	*conninfra_status;
	uint8_t		conninfra_cr_num;
#if defined(_HIF_PCIE)
	u_int32_t	*conninfra_bus_cr;
#else
	u_int32_t	*conninfra_bus_status;
#endif
	uint8_t		conninfra_bus_cr_num;
	u_int32_t	*conninfra_signal_status;
	uint8_t		conninfra_signal_num;
};
#endif

/*******************************************************************************
 *                            P U B L I C   D A T A
 *******************************************************************************
 */

/*******************************************************************************
 *                           P R I V A T E   D A T A
 *******************************************************************************
 */
 static struct EMPTY_QUEUE_INFO ple_queue_empty_info[] = {
	{"CPU Q0", MCU_Q0_INDEX, ENUM_UMAC_CTX_Q_0},
	{"CPU Q1", ENUM_UMAC_CPU_PORT_1, ENUM_UMAC_CTX_Q_1},
	{"CPU Q2", ENUM_UMAC_CPU_PORT_1, ENUM_UMAC_CTX_Q_2},
	{"CPU Q3", ENUM_UMAC_CPU_PORT_1, ENUM_UMAC_CTX_Q_3},
	{NULL, 0, 0},
	{NULL, 0, 0},
	{NULL, 0, 0},
	{NULL, 0, 0}, /* 4~7 not defined */
	{"ALTX Q0", ENUM_UMAC_LMAC_PORT_2,
	 ENUM_UMAC_LMAC_PLE_TX_Q_ALTX_0}, /* Q16 */
	{"BMC Q0", ENUM_UMAC_LMAC_PORT_2, ENUM_UMAC_LMAC_PLE_TX_Q_BMC_0},
	{"BCN Q0", ENUM_UMAC_LMAC_PORT_2, ENUM_UMAC_LMAC_PLE_TX_Q_BNC_0},
	{"PSMP Q0", ENUM_UMAC_LMAC_PORT_2, ENUM_UMAC_LMAC_PLE_TX_Q_PSMP_0},
	{"ALTX Q1", ENUM_UMAC_LMAC_PORT_2, ENUM_UMAC_LMAC_PLE_TX_Q_ALTX_1},
	{"BMC Q1", ENUM_UMAC_LMAC_PORT_2, ENUM_UMAC_LMAC_PLE_TX_Q_BMC_1},
	{"BCN Q1", ENUM_UMAC_LMAC_PORT_2, ENUM_UMAC_LMAC_PLE_TX_Q_BNC_1},
	{"PSMP Q1", ENUM_UMAC_LMAC_PORT_2, ENUM_UMAC_LMAC_PLE_TX_Q_PSMP_1},
	{"NAF Q", ENUM_UMAC_LMAC_PORT_2, ENUM_UMAC_LMAC_PLE_TX_Q_NAF},
	{"NBCN Q", ENUM_UMAC_LMAC_PORT_2, ENUM_UMAC_LMAC_PLE_TX_Q_NBCN},
	{NULL, 0, 0},
	{NULL, 0, 0},
	{NULL, 0, 0},
	{NULL, 0, 0},
	{NULL, 0, 0},
	{NULL, 0, 0},
	{NULL, 0, 0},
	{NULL, 0, 0},
	{NULL, 0, 0},
	{NULL, 0, 0},
	{NULL, 0, 0},
	{NULL, 0, 0}, /* 18~29 not defined */
	{"RLS Q", ENUM_PLE_CTRL_PSE_PORT_3, ENUM_UMAC_PLE_CTRL_P3_Q_0X1E},
	{"RLS2 Q", ENUM_PLE_CTRL_PSE_PORT_3, ENUM_UMAC_PLE_CTRL_P3_Q_0X1F} };

struct wfdma_group_info wfmda_host_tx_group[] = {
	{"T0:DATA0", WF_WFDMA_HOST_DMA0_WPDMA_TX_RING0_CTRL0_ADDR},
	{"T1:DATA1", WF_WFDMA_HOST_DMA0_WPDMA_TX_RING1_CTRL0_ADDR},
	{"T2:DATA2", WF_WFDMA_HOST_DMA0_WPDMA_TX_RING2_CTRL0_ADDR},
	{"T3:DATA3", WF_WFDMA_HOST_DMA0_WPDMA_TX_RING3_CTRL0_ADDR},
	{"T4:DATA4", WF_WFDMA_HOST_DMA0_WPDMA_TX_RING4_CTRL0_ADDR},
	{"T5:DATA5", WF_WFDMA_HOST_DMA0_WPDMA_TX_RING5_CTRL0_ADDR},
	{"T6:DATA6", WF_WFDMA_HOST_DMA0_WPDMA_TX_RING6_CTRL0_ADDR},
	{"T15:CMD", WF_WFDMA_HOST_DMA0_WPDMA_TX_RING15_CTRL0_ADDR},
	{"T16:FWDL", WF_WFDMA_HOST_DMA0_WPDMA_TX_RING16_CTRL0_ADDR},
};

struct wfdma_group_info wfmda_host_rx_group[] = {
	{"R0:TXFREEDONE(BN0)", WF_WFDMA_HOST_DMA0_WPDMA_RX_RING0_CTRL0_ADDR},
	{"R1:TXFREEDONE(BN1)", WF_WFDMA_HOST_DMA0_WPDMA_RX_RING1_CTRL0_ADDR},
	{"R2:DATA(BN0)", WF_WFDMA_HOST_DMA0_WPDMA_RX_RING2_CTRL0_ADDR},
	{"R3:DATA(BN1)", WF_WFDMA_HOST_DMA0_WPDMA_RX_RING3_CTRL0_ADDR},
};

static u_int8_t *sta_ctrl_reg[] = {"ENABLE", "DISABLE", "PAUSE"};

#if (CFG_SUPPORT_DEBUG_SOP == 1)
static u_int32_t mt7902_wfsys_sleep_status_sel[] = {
#if defined(_HIF_PCIE)
	0x00100000, 0x00108421, 0x00184210, 0x001BDEF7, 0x001EF7BD
#else
	0x80000000, 0x80008421, 0x80084210, 0x800BDEF7, 0x800EF7BD
#endif
};

static u_int32_t mt7902_wfsys_csr_status_sel[] = {
#if defined(_HIF_PCIE)
	0x00010001, 0x00020001, 0x00030001, 0x00040001, 0x00050001,
	0x00060001, 0x00070001, 0x00080001, 0x00090001, 0x000A0001,
	0x000B0001, 0x000C0001, 0x000D0001, 0x000E0001, 0x000F0001,
	0x00100001, 0x00010002, 0x00010003
#else
	0xC8010001, 0xC8020001, 0xC8030001, 0xC8040001, 0xC8050001,
	0xC8060001, 0xC8070001, 0xC8080001, 0xC8090001, 0xC80A0001,
	0xC80B0001, 0xC80C0001, 0xC80D0001, 0xC80E0001, 0xC80F0001,
	0xC8100001, 0xC8010002, 0xC8010003
#endif
};

static u_int32_t mt7902_wfsys_status_sel[] = {
#if defined(_HIF_PCIE)
	0x00100000, 0x00108421, 0x00184210, 0x00194A52, 0x001BDEF7,
	0x001C6318, 0x001E739C, 0x001EF7BD
#else
	0x80000000, 0x80008421, 0x80084210, 0x80094A52, 0x800BDEF7,
	0x800C6318, 0x800E739C, 0x800EF7BD
#endif
};

static u_int32_t mt7902_bgfsys_bus_status_sel[] = {
	0x00767501, 0x00787701, 0x007A7901, 0x007C7B01, 0x007E7D01,
	0x00807F01, 0x00828101, 0x00848301, 0x00868501, 0x00888701,
	0x008A8901, 0x008C8B01, 0x008E8D01, 0x00908F01, 0x00929101,
	0x00949301, 0x00969501, 0x00989701, 0x009A9901, 0x009C9B01,
	0x009E9D01, 0x00A09F01, 0x00A2A101, 0x00A4A301, 0x00A6A501,
	0x00A8A701, 0x00AAA901, 0x00ACAB01, 0x00AEAD01, 0x00B0AF01
};

static u_int32_t mt7902_bgfsys_status_sel[] = {
	0x00000080, 0x00000081, 0x00000082, 0x00000083, 0x00000084,
	0x00000085, 0x00000086, 0x00000087, 0x00000088, 0x00000089,
	0x0000008a, 0x0000008b, 0x0000008c, 0x0000008d, 0x0000008e,
	0x0000008f, 0x00000090, 0x00000091, 0x00000092, 0x00000093,
	0x000000c0, 0x000000c1, 0x000000c2, 0x000000c3, 0x000000c4,
	0x000000c5, 0x000000c6, 0x000000c7, 0x000000c8, 0x000000d0,
	0x000000d1, 0x000000d2, 0x000000d3, 0x000000d4, 0x000000d5,
	0x000000d6, 0x000000d7, 0x000000d8, 0x000000d9, 0x000000da,
	0x000000db, 0x000000dc, 0x000000dd, 0x000000de, 0x000000e0,
	0x000000e1, 0x000000e2, 0x000000e3
};

static u_int32_t mt7902_conninfra_status_sel[] = {
#if defined(_HIF_PCIE)
	0x00000000, 0x00000001, 0x00000002, 0x00000003, 0x00000004,
	0x00000005
#else
	0xD0000000, 0xD0000001, 0xD0000002, 0xD0000003, 0xD0000004,
	0xD0000005
#endif
};

#if defined(_HIF_PCIE)
static u_int32_t mt7902_conninfra_bus_cr[] = {
	0xF1514, 0xF1504, 0xF1564, 0xF1554, 0xF1544, 0xF1524,
	0xF1534, 0xFE2A0, 0xFE2A8, 0xFE2A4, 0xFF408, 0xFF40C,
	0xFF410, 0xFF414, 0xFF418, 0xFF41C, 0xFF420, 0xFF424,
	0xFF428, 0xFF42C, 0xFF430, 0xE0414, 0xE0418, 0xE042C,
	0xE0430, 0xE041C, 0xE0420, 0xE0410, 0xFE370
};
#else
static u_int32_t mt7902_conninfra_bus_status_sel[] = {
	0xA8000000, 0xB0000000, 0x88000000, 0x90000000, 0x98000000,
	0xA0000000
};
#endif

static u_int32_t mt7902_conn_infra_signal_status_sel[] = {
#if defined(_HIF_PCIE)
	0x00010001, 0x00020001, 0x00010002, 0x00020002, 0x00030002,
	0x00010003, 0x00020003, 0x00030003, 0x00010004, 0x00020004,
	0x00010005
#else
	0xC0010001, 0xC0020001, 0xC0010002, 0xC0020002, 0xC0030002,
	0xC0010003, 0xC0020003, 0xC0030003, 0xC0010004, 0xC0020004,
	0xC0010005
#endif
};

static struct MT7902_DEBUG_SOP_INFO mt7902_debug_sop_info[] = {
	{mt7902_wfsys_sleep_status_sel,
	sizeof(mt7902_wfsys_sleep_status_sel)/sizeof(u_int32_t),
	mt7902_wfsys_csr_status_sel,
	sizeof(mt7902_wfsys_csr_status_sel)/sizeof(u_int32_t),
	mt7902_wfsys_status_sel,
	sizeof(mt7902_wfsys_status_sel)/sizeof(u_int32_t),
	mt7902_bgfsys_bus_status_sel,
	sizeof(mt7902_bgfsys_bus_status_sel)/sizeof(u_int32_t),
	mt7902_bgfsys_status_sel,
	sizeof(mt7902_bgfsys_status_sel)/sizeof(u_int32_t),
	mt7902_conninfra_status_sel,
	sizeof(mt7902_conninfra_status_sel)/sizeof(u_int32_t),
#if defined(_HIF_PCIE)
	mt7902_conninfra_bus_cr,
	sizeof(mt7902_conninfra_bus_cr)/sizeof(u_int32_t),
#else
	mt7902_conninfra_bus_status_sel,
	sizeof(mt7902_conninfra_bus_status_sel)/sizeof(u_int32_t),
#endif
	mt7902_conn_infra_signal_status_sel,
	sizeof(mt7902_conn_infra_signal_status_sel)/sizeof(u_int32_t),
	}
};
#endif /* (CFG_SUPPORT_DEBUG_SOP == 1) */


/*******************************************************************************
 *                                 M A C R O S
 *******************************************************************************
 */

/*******************************************************************************
 *                   F U N C T I O N   D E C L A R A T I O N S
 *******************************************************************************
 */

/*******************************************************************************
 *                              F U N C T I O N S
 *******************************************************************************
 */
 void mt7902_show_ple_info(
	struct ADAPTER *prAdapter,
	u_int8_t fgDumpTxd)
{
	u_int32_t ple_buf_ctrl = 0, pg_sz, pg_num;
	u_int32_t ple_stat[25] = {0}, pg_flow_ctrl[10] = {0};
	u_int32_t sta_pause[6] = {0}, dis_sta_map[6] = {0};
	u_int32_t fpg_cnt, ffa_cnt, fpg_head, fpg_tail, hif_max_q, hif_min_q;
	u_int32_t rpg_hif, upg_hif, cpu_max_q, cpu_min_q, rpg_cpu, upg_cpu;
	u_int32_t ple_err = 0, ple_err1 = 0;
	u_int32_t i, j;
#if 0
	u_int32_t ple_txcmd_stat;
#endif

	HAL_MCR_RD(prAdapter, WF_PLE_TOP_PBUF_CTRL_ADDR, &ple_buf_ctrl);
	HAL_MCR_RD(prAdapter, WF_PLE_TOP_QUEUE_EMPTY_ADDR, &ple_stat[0]);
	HAL_MCR_RD(prAdapter, WF_PLE_TOP_AC0_QUEUE_EMPTY0_ADDR, &ple_stat[1]);
#if 0
	HAL_MCR_RD(prAdapter, WF_PLE_TOP_AC0_QUEUE_EMPTY1_ADDR, &ple_stat[2]);
	HAL_MCR_RD(prAdapter, WF_PLE_TOP_AC0_QUEUE_EMPTY2_ADDR, &ple_stat[3]);
	HAL_MCR_RD(prAdapter, WF_PLE_TOP_AC0_QUEUE_EMPTY3_ADDR, &ple_stat[4]);
	HAL_MCR_RD(prAdapter, WF_PLE_TOP_AC0_QUEUE_EMPTY4_ADDR, &ple_stat[5]);
	HAL_MCR_RD(prAdapter, WF_PLE_TOP_AC0_QUEUE_EMPTY5_ADDR, &ple_stat[6]);
#endif
	HAL_MCR_RD(prAdapter, WF_PLE_TOP_AC1_QUEUE_EMPTY0_ADDR, &ple_stat[7]);
#if 0
	HAL_MCR_RD(prAdapter, WF_PLE_TOP_AC1_QUEUE_EMPTY1_ADDR, &ple_stat[8]);
	HAL_MCR_RD(prAdapter, WF_PLE_TOP_AC1_QUEUE_EMPTY2_ADDR, &ple_stat[9]);
	HAL_MCR_RD(prAdapter, WF_PLE_TOP_AC1_QUEUE_EMPTY3_ADDR, &ple_stat[10]);
	HAL_MCR_RD(prAdapter, WF_PLE_TOP_AC1_QUEUE_EMPTY4_ADDR, &ple_stat[11]);
	HAL_MCR_RD(prAdapter, WF_PLE_TOP_AC1_QUEUE_EMPTY5_ADDR, &ple_stat[12]);
#endif
	HAL_MCR_RD(prAdapter, WF_PLE_TOP_AC2_QUEUE_EMPTY0_ADDR, &ple_stat[13]);
#if 0
	HAL_MCR_RD(prAdapter, WF_PLE_TOP_AC2_QUEUE_EMPTY1_ADDR, &ple_stat[14]);
	HAL_MCR_RD(prAdapter, WF_PLE_TOP_AC2_QUEUE_EMPTY2_ADDR, &ple_stat[15]);
	HAL_MCR_RD(prAdapter, WF_PLE_TOP_AC2_QUEUE_EMPTY3_ADDR, &ple_stat[16]);
	HAL_MCR_RD(prAdapter, WF_PLE_TOP_AC2_QUEUE_EMPTY4_ADDR, &ple_stat[17]);
	HAL_MCR_RD(prAdapter, WF_PLE_TOP_AC2_QUEUE_EMPTY5_ADDR, &ple_stat[18]);
#endif
	HAL_MCR_RD(prAdapter, WF_PLE_TOP_AC3_QUEUE_EMPTY0_ADDR, &ple_stat[19]);
#if 0
	HAL_MCR_RD(prAdapter, WF_PLE_TOP_AC3_QUEUE_EMPTY1_ADDR, &ple_stat[20]);
	HAL_MCR_RD(prAdapter, WF_PLE_TOP_AC3_QUEUE_EMPTY2_ADDR, &ple_stat[21]);
	HAL_MCR_RD(prAdapter, WF_PLE_TOP_AC3_QUEUE_EMPTY3_ADDR, &ple_stat[22]);
	HAL_MCR_RD(prAdapter, WF_PLE_TOP_AC3_QUEUE_EMPTY4_ADDR, &ple_stat[23]);
	HAL_MCR_RD(prAdapter, WF_PLE_TOP_AC3_QUEUE_EMPTY5_ADDR, &ple_stat[24]);
#endif
#if 0
	HAL_MCR_RD(prAdapter, WF_PLE_TOP_TXCMD_QUEUE_EMPTY_ADDR,
		   &ple_txcmd_stat);
#endif
	HAL_MCR_RD(prAdapter, WF_PLE_TOP_FREEPG_CNT_ADDR, &pg_flow_ctrl[0]);
	HAL_MCR_RD(prAdapter, WF_PLE_TOP_FREEPG_HEAD_TAIL_ADDR,
		   &pg_flow_ctrl[1]);
	HAL_MCR_RD(prAdapter, WF_PLE_TOP_PG_HIF_GROUP_ADDR, &pg_flow_ctrl[2]);
	HAL_MCR_RD(prAdapter, WF_PLE_TOP_HIF_PG_INFO_ADDR, &pg_flow_ctrl[3]);
	HAL_MCR_RD(prAdapter, WF_PLE_TOP_PG_CPU_GROUP_ADDR, &pg_flow_ctrl[4]);
	HAL_MCR_RD(prAdapter, WF_PLE_TOP_CPU_PG_INFO_ADDR, &pg_flow_ctrl[5]);
	HAL_MCR_RD(prAdapter, WF_PLE_TOP_PG_HIF_TXCMD_GROUP_ADDR,
		   &pg_flow_ctrl[6]);
	HAL_MCR_RD(prAdapter, WF_PLE_TOP_HIF_TXCMD_PG_INFO_ADDR,
		   &pg_flow_ctrl[7]);
	HAL_MCR_RD(prAdapter, WF_PLE_TOP_PG_HIF_WMTXD_GROUP_ADDR,
		   &pg_flow_ctrl[8]);
	HAL_MCR_RD(prAdapter, WF_PLE_TOP_HIF_WMTXD_PG_INFO_ADDR,
		   &pg_flow_ctrl[9]);
	HAL_MCR_RD(prAdapter, WF_PLE_TOP_DIS_STA_MAP0_ADDR, &dis_sta_map[0]);
#if 0
	HAL_MCR_RD(prAdapter, WF_PLE_TOP_DIS_STA_MAP1_ADDR, &dis_sta_map[1]);
	HAL_MCR_RD(prAdapter, WF_PLE_TOP_DIS_STA_MAP2_ADDR, &dis_sta_map[2]);
	HAL_MCR_RD(prAdapter, WF_PLE_TOP_DIS_STA_MAP3_ADDR, &dis_sta_map[3]);
	HAL_MCR_RD(prAdapter, WF_PLE_TOP_DIS_STA_MAP4_ADDR, &dis_sta_map[4]);
	HAL_MCR_RD(prAdapter, WF_PLE_TOP_DIS_STA_MAP5_ADDR, &dis_sta_map[5]);
#endif
	HAL_MCR_RD(prAdapter, WF_PLE_TOP_STATION_PAUSE0_ADDR, &sta_pause[0]);
#if 0
	HAL_MCR_RD(prAdapter, WF_PLE_TOP_STATION_PAUSE1_ADDR, &sta_pause[1]);
	HAL_MCR_RD(prAdapter, WF_PLE_TOP_STATION_PAUSE2_ADDR, &sta_pause[2]);
	HAL_MCR_RD(prAdapter, WF_PLE_TOP_STATION_PAUSE3_ADDR, &sta_pause[3]);
	HAL_MCR_RD(prAdapter, WF_PLE_TOP_STATION_PAUSE4_ADDR, &sta_pause[4]);
	HAL_MCR_RD(prAdapter, WF_PLE_TOP_STATION_PAUSE5_ADDR, &sta_pause[5]);
#endif
	HAL_MCR_RD(prAdapter, WF_PLE_TOP_INT_N9_ERR_STS_ADDR, &ple_err);
	HAL_MCR_RD(prAdapter, WF_PLE_TOP_INT_N9_ERR_STS_1_ADDR, &ple_err1);

	/* Configuration Info */
	DBGLOG(HAL, INFO, "PLE Configuration Info:\n");

	DBGLOG(HAL, INFO, "\tPacket Buffer Control(0x%08x): 0x%08x\n",
		WF_PLE_TOP_PBUF_CTRL_ADDR,
		ple_buf_ctrl);
	pg_sz = (ple_buf_ctrl & WF_PLE_TOP_PBUF_CTRL_PAGE_SIZE_CFG_MASK) >>
		WF_PLE_TOP_PBUF_CTRL_PAGE_SIZE_CFG_SHFT;
	DBGLOG(HAL, INFO, "\t\tPage Size=%d(%d bytes per page)\n", pg_sz,
	       (pg_sz == 1 ? 128 : 64));
	DBGLOG(HAL, INFO, "\t\tPage Offset=%d(in unit of 2KB)\n",
	       (ple_buf_ctrl & WF_PLE_TOP_PBUF_CTRL_PBUF_OFFSET_MASK) >>
		       WF_PLE_TOP_PBUF_CTRL_PBUF_OFFSET_SHFT);
	pg_num = (ple_buf_ctrl & WF_PLE_TOP_PBUF_CTRL_TOTAL_PAGE_NUM_MASK) >>
		 WF_PLE_TOP_PBUF_CTRL_TOTAL_PAGE_NUM_SHFT;
	DBGLOG(HAL, INFO, "\t\tTotal Page=%d pages\n", pg_num);

	/* Page Flow Control */
	DBGLOG(HAL, INFO, "PLE Page Flow Control:\n");
	DBGLOG(HAL, INFO, "\tFree page counter(0x%08x): 0x%08x\n",
		WF_PLE_TOP_FREEPG_CNT_ADDR,
		pg_flow_ctrl[0]);
	fpg_cnt = (pg_flow_ctrl[0] & WF_PLE_TOP_FREEPG_CNT_FREEPG_CNT_MASK) >>
		WF_PLE_TOP_FREEPG_CNT_FREEPG_CNT_SHFT;
	DBGLOG(HAL, INFO, "\t\tThe toal page number of free=0x%03x\n", fpg_cnt);
	ffa_cnt = (pg_flow_ctrl[0] & WF_PLE_TOP_FREEPG_CNT_FFA_CNT_MASK) >>
		  WF_PLE_TOP_FREEPG_CNT_FFA_CNT_SHFT;
	DBGLOG(HAL, INFO, "\t\tThe free page numbers of free for all=0x%03x\n",
	       ffa_cnt);
	DBGLOG(HAL, INFO, "\tFree page head and tail(0x%08x): 0x%08x\n",
		WF_PLE_TOP_FREEPG_HEAD_TAIL_ADDR,
		pg_flow_ctrl[1]);
	fpg_head = (pg_flow_ctrl[1] &
		    WF_PLE_TOP_FREEPG_HEAD_TAIL_FREEPG_HEAD_MASK) >>
		   WF_PLE_TOP_FREEPG_HEAD_TAIL_FREEPG_HEAD_SHFT;
	fpg_tail = (pg_flow_ctrl[1] &
		    WF_PLE_TOP_FREEPG_HEAD_TAIL_FREEPG_TAIL_MASK) >>
		   WF_PLE_TOP_FREEPG_HEAD_TAIL_FREEPG_TAIL_SHFT;
	DBGLOG(HAL, INFO,
	       "\t\tThe tail/head page of free page list=0x%03x/0x%03x\n",
	       fpg_tail, fpg_head);
	DBGLOG(HAL, INFO,
		"\tReserved page counter of HIF group(0x%08x): 0x%08x\n",
		WF_PLE_TOP_PG_HIF_GROUP_ADDR,
		pg_flow_ctrl[2]);
	DBGLOG(HAL, INFO, "\tHIF group page status(0x%08x): 0x%08x\n",
		WF_PLE_TOP_HIF_PG_INFO_ADDR,
		pg_flow_ctrl[3]);
	hif_min_q = (pg_flow_ctrl[2] &
		     WF_PLE_TOP_PG_HIF_GROUP_HIF_MIN_QUOTA_MASK) >>
		    WF_PLE_TOP_PG_HIF_GROUP_HIF_MIN_QUOTA_SHFT;
	hif_max_q = (pg_flow_ctrl[2] &
		     WF_PLE_TOP_PG_HIF_GROUP_HIF_MAX_QUOTA_MASK) >>
		    WF_PLE_TOP_PG_HIF_GROUP_HIF_MAX_QUOTA_SHFT;
	DBGLOG(HAL, INFO,
	       "\t\tThe max/min quota pages of HIF group=0x%03x/0x%03x\n",
	       hif_max_q, hif_min_q);
	rpg_hif = (pg_flow_ctrl[3] & WF_PLE_TOP_HIF_PG_INFO_HIF_RSV_CNT_MASK) >>
		  WF_PLE_TOP_HIF_PG_INFO_HIF_RSV_CNT_SHFT;
	upg_hif = (pg_flow_ctrl[3] & WF_PLE_TOP_HIF_PG_INFO_HIF_SRC_CNT_MASK) >>
		  WF_PLE_TOP_HIF_PG_INFO_HIF_SRC_CNT_SHFT;
	DBGLOG(HAL, INFO,
	       "\t\tThe used/reserved pages of HIF group=0x%03x/0x%03x\n",
	       upg_hif, rpg_hif);

	DBGLOG(HAL, INFO,
	"\tReserved page counter of HIF_TXCMD group(0x%08x): 0x%08x\n",
	WF_PLE_TOP_PG_HIF_TXCMD_GROUP_ADDR,
	pg_flow_ctrl[6]);
	DBGLOG(HAL, INFO, "\tHIF_TXCMD group page status(0x%08x): 0x%08x\n",
		WF_PLE_TOP_HIF_TXCMD_PG_INFO_ADDR,
		pg_flow_ctrl[7]);
	cpu_min_q = (pg_flow_ctrl[6] &
		     WF_PLE_TOP_PG_HIF_TXCMD_GROUP_HIF_TXCMD_MIN_QUOTA_MASK) >>
		    WF_PLE_TOP_PG_HIF_TXCMD_GROUP_HIF_TXCMD_MIN_QUOTA_SHFT;
	cpu_max_q = (pg_flow_ctrl[6] &
		     WF_PLE_TOP_PG_HIF_TXCMD_GROUP_HIF_TXCMD_MAX_QUOTA_MASK) >>
		    WF_PLE_TOP_PG_HIF_TXCMD_GROUP_HIF_TXCMD_MAX_QUOTA_SHFT;
	DBGLOG(HAL, INFO,
	       "\t\tThe max/min quota pages of HIF_TXCMD group=0x%03x/0x%03x\n",
	       cpu_max_q, cpu_min_q);
	rpg_cpu = (pg_flow_ctrl[7] &
		   WF_PLE_TOP_HIF_TXCMD_PG_INFO_HIF_TXCMD_SRC_CNT_MASK) >>
		  WF_PLE_TOP_HIF_TXCMD_PG_INFO_HIF_TXCMD_SRC_CNT_SHFT;
	upg_cpu = (pg_flow_ctrl[7] &
		   WF_PLE_TOP_HIF_TXCMD_PG_INFO_HIF_TXCMD_RSV_CNT_MASK) >>
		  WF_PLE_TOP_HIF_TXCMD_PG_INFO_HIF_TXCMD_RSV_CNT_SHFT;
	DBGLOG(HAL, INFO,
	       "\t\tThe used/reserved pages of HIF_TXCMD group=0x%03x/0x%03x\n",
	       upg_cpu, rpg_cpu);

	DBGLOG(HAL, INFO,
		"\tReserved page counter of CPU group(0x%08x): 0x%08x\n",
		WF_PLE_TOP_PG_CPU_GROUP_ADDR,
		pg_flow_ctrl[4]);
	DBGLOG(HAL, INFO, "\tCPU group page status(0x%08x): 0x%08x\n",
		WF_PLE_TOP_CPU_PG_INFO_ADDR,
	       pg_flow_ctrl[5]);
	cpu_min_q = (pg_flow_ctrl[4] &
		     WF_PLE_TOP_PG_CPU_GROUP_CPU_MIN_QUOTA_MASK) >>
		    WF_PLE_TOP_PG_CPU_GROUP_CPU_MIN_QUOTA_SHFT;
	cpu_max_q = (pg_flow_ctrl[4] &
		     WF_PLE_TOP_PG_CPU_GROUP_CPU_MAX_QUOTA_MASK) >>
		    WF_PLE_TOP_PG_CPU_GROUP_CPU_MAX_QUOTA_SHFT;
	DBGLOG(HAL, INFO,
	       "\t\tThe max/min quota pages of CPU group=0x%03x/0x%03x\n",
	       cpu_max_q, cpu_min_q);
	rpg_cpu = (pg_flow_ctrl[5] & WF_PLE_TOP_CPU_PG_INFO_CPU_RSV_CNT_MASK) >>
		  WF_PLE_TOP_CPU_PG_INFO_CPU_RSV_CNT_SHFT;
	upg_cpu = (pg_flow_ctrl[5] & WF_PLE_TOP_CPU_PG_INFO_CPU_SRC_CNT_MASK) >>
		  WF_PLE_TOP_CPU_PG_INFO_CPU_SRC_CNT_SHFT;
	DBGLOG(HAL, INFO,
	       "\t\tThe used/reserved pages of CPU group=0x%03x/0x%03x\n",
	       upg_cpu, rpg_cpu);

	DBGLOG(HAL, INFO,
		"\tReserved page counter of HIF_WMTXD group(0x%08x): 0x%08x\n",
		WF_PLE_TOP_PG_HIF_WMTXD_GROUP_ADDR,
		pg_flow_ctrl[8]);
	DBGLOG(HAL, INFO, "\tHIF_WMTXD group page status(0x%08x): 0x%08x\n",
		WF_PLE_TOP_HIF_WMTXD_PG_INFO_ADDR,
	       pg_flow_ctrl[9]);
	cpu_min_q = (pg_flow_ctrl[8] &
		     WF_PLE_TOP_PG_HIF_WMTXD_GROUP_HIF_WMTXD_MIN_QUOTA_MASK) >>
		    WF_PLE_TOP_PG_HIF_WMTXD_GROUP_HIF_WMTXD_MIN_QUOTA_SHFT;
	cpu_max_q = (pg_flow_ctrl[8] &
		     WF_PLE_TOP_PG_HIF_WMTXD_GROUP_HIF_WMTXD_MAX_QUOTA_MASK) >>
		    WF_PLE_TOP_PG_HIF_WMTXD_GROUP_HIF_WMTXD_MAX_QUOTA_SHFT;
	DBGLOG(HAL, INFO,
	       "\t\tThe max/min quota pages of HIF_WMTXD group=0x%03x/0x%03x\n",
	       cpu_max_q, cpu_min_q);
	rpg_cpu = (pg_flow_ctrl[9] &
		   WF_PLE_TOP_HIF_WMTXD_PG_INFO_HIF_WMTXD_RSV_CNT_MASK) >>
		  WF_PLE_TOP_HIF_WMTXD_PG_INFO_HIF_WMTXD_RSV_CNT_SHFT;
	upg_cpu = (pg_flow_ctrl[9] &
		   WF_PLE_TOP_HIF_WMTXD_PG_INFO_HIF_WMTXD_SRC_CNT_MASK) >>
		  WF_PLE_TOP_HIF_WMTXD_PG_INFO_HIF_WMTXD_SRC_CNT_SHFT;
	DBGLOG(HAL, INFO,
	       "\t\tThe used/reserved pages of HIF_WMTXD group=0x%03x/0x%03x\n",
	       upg_cpu, rpg_cpu);

	if ((ple_stat[0] & WF_PLE_TOP_QUEUE_EMPTY_ALL_AC_EMPTY_MASK) == 0) {
		for (j = 0; j < 24; j = j + 6) {
			if (j % 6 == 0) {
				DBGLOG(HAL, INFO,
					"\tNonempty AC%d Q of STA#: ", j / 6);
			}

			for (i = 0; i < 32; i++) {
				if (((ple_stat[j + 1] & (0x1 << i)) >> i) ==
				    0) {
					DBGLOG(HAL, INFO, "%d ",
						i + (j % 6) * 32);
				}
			}
		}

		DBGLOG(HAL, INFO, "\n");
	}

	DBGLOG(HAL, INFO, "Nonempty Q info:\n");

	for (i = 0; i < 31; i++) {
		if (((ple_stat[0] & (0x1 << i)) >> i) == 0) {
			uint32_t hfid, tfid, pktcnt, fl_que_ctrl[3] = {0};

			if (ple_queue_empty_info[i].QueueName != NULL) {
				DBGLOG(HAL, INFO, "\t%s: ",
					ple_queue_empty_info[i].QueueName);
				fl_que_ctrl[0] |=
					WF_PLE_TOP_FL_QUE_CTRL_0_EXECUTE_MASK;
				fl_que_ctrl[0] |=
				(ple_queue_empty_info[i].Portid
				 << WF_PLE_TOP_FL_QUE_CTRL_0_Q_BUF_PID_SHFT);
				fl_que_ctrl[0] |=
				(ple_queue_empty_info[i].Queueid
				 << WF_PLE_TOP_FL_QUE_CTRL_0_Q_BUF_QID_SHFT);
			} else
				continue;

			HAL_MCR_WR(prAdapter, WF_PLE_TOP_FL_QUE_CTRL_0_ADDR,
				   fl_que_ctrl[0]);
			HAL_MCR_RD(prAdapter, WF_PLE_TOP_FL_QUE_CTRL_2_ADDR,
				   &fl_que_ctrl[1]);
			HAL_MCR_RD(prAdapter, WF_PLE_TOP_FL_QUE_CTRL_3_ADDR,
				   &fl_que_ctrl[2]);
			hfid = (fl_que_ctrl[1] &
				WF_PLE_TOP_FL_QUE_CTRL_2_QUEUE_HEAD_FID_MASK) >>
			       WF_PLE_TOP_FL_QUE_CTRL_2_QUEUE_HEAD_FID_SHFT;
			tfid = (fl_que_ctrl[1] &
				WF_PLE_TOP_FL_QUE_CTRL_2_QUEUE_TAIL_FID_MASK) >>
			       WF_PLE_TOP_FL_QUE_CTRL_2_QUEUE_TAIL_FID_SHFT;
			pktcnt =
				(fl_que_ctrl[2] &
				 WF_PLE_TOP_FL_QUE_CTRL_3_QUEUE_PKT_NUM_MASK) >>
				WF_PLE_TOP_FL_QUE_CTRL_3_QUEUE_PKT_NUM_SHFT;
			DBGLOG(HAL, INFO,
			"tail/head fid = 0x%03x/0x%03x, pkt cnt = 0x%03x\n",
				tfid, hfid, pktcnt);
			if (pktcnt > 0 && fgDumpTxd)
				connac2x_show_txd_Info(
					prAdapter, hfid);
		}
	}

	for (j = 0; j < 24; j = j + 6) { /* show AC Q info */
		for (i = 0; i < 32; i++) {
			if (((ple_stat[j + 1] & (0x1 << i)) >> i) == 0) {
				uint32_t hfid, tfid, pktcnt, ac_num = j / 6,
							   ctrl = 0;
				uint32_t sta_num = i + (j % 6) * 32,
				       fl_que_ctrl[3] = {0};
				uint32_t wmmidx = 0;

				DBGLOG(HAL, INFO, "\tSTA%d AC%d: ", sta_num,
				       ac_num);

				fl_que_ctrl[0] |=
					WF_PLE_TOP_FL_QUE_CTRL_0_EXECUTE_MASK;
				fl_que_ctrl[0] |=
				(ENUM_UMAC_LMAC_PORT_2
				 << WF_PLE_TOP_FL_QUE_CTRL_0_Q_BUF_PID_SHFT);
				fl_que_ctrl[0] |=
				(ac_num
				 << WF_PLE_TOP_FL_QUE_CTRL_0_Q_BUF_QID_SHFT);
				fl_que_ctrl[0] |=
				(sta_num
				 << WF_PLE_TOP_FL_QUE_CTRL_0_Q_BUF_WLANID_SHFT);
				HAL_MCR_WR(prAdapter,
					   WF_PLE_TOP_FL_QUE_CTRL_0_ADDR,
					   fl_que_ctrl[0]);
				HAL_MCR_RD(prAdapter,
					   WF_PLE_TOP_FL_QUE_CTRL_2_ADDR,
					   &fl_que_ctrl[1]);
				HAL_MCR_RD(prAdapter,
					   WF_PLE_TOP_FL_QUE_CTRL_3_ADDR,
					   &fl_que_ctrl[2]);
				hfid = (fl_que_ctrl[1] &
				WF_PLE_TOP_FL_QUE_CTRL_2_QUEUE_HEAD_FID_MASK) >>
				WF_PLE_TOP_FL_QUE_CTRL_2_QUEUE_HEAD_FID_SHFT;
				tfid = (fl_que_ctrl[1] &
				WF_PLE_TOP_FL_QUE_CTRL_2_QUEUE_TAIL_FID_MASK) >>
				WF_PLE_TOP_FL_QUE_CTRL_2_QUEUE_TAIL_FID_SHFT;

				pktcnt =
				(fl_que_ctrl[2] &
				WF_PLE_TOP_FL_QUE_CTRL_3_QUEUE_PKT_NUM_MASK) >>
				WF_PLE_TOP_FL_QUE_CTRL_3_QUEUE_PKT_NUM_SHFT;
				DBGLOG(HAL, INFO,
				"tail/head fid = 0x%03x/0x%03x, pkt cnt = %x",
				tfid, hfid, pktcnt);

				if (((sta_pause[j % 6] & 0x1 << i) >> i) == 1)
					ctrl = 2;

				if (((dis_sta_map[j % 6] & 0x1 << i) >> i) == 1)
					ctrl = 1;

				DBGLOG(HAL, INFO, " ctrl = %s",
						   sta_ctrl_reg[ctrl]);
				DBGLOG(HAL, INFO, " (wmmidx=%d)\n",
					wmmidx);
				if (pktcnt > 0 && fgDumpTxd)
					connac2x_show_txd_Info(
						prAdapter, hfid);
			}
		}
	}
#if 0
	if (~ple_txcmd_stat) {
		DBGLOG(HAL, INFO, "Nonempty TXCMD Q info:\n");
		for (i = 0; i < 31; i++) {
			if (((ple_txcmd_stat & (0x1 << i)) >> i) == 0) {
				uint32_t hfid, tfid;
				uint32_t pktcnt, fl_que_ctrl[3] = {0};

				if (ple_txcmd_queue_empty_info[i].QueueName !=
				    NULL) {
					DBGLOG(HAL, INFO, "\t%s: ",
					       ple_txcmd_queue_empty_info[i]
						       .QueueName);
					fl_que_ctrl[0] |=
					WF_PLE_TOP_FL_QUE_CTRL_0_EXECUTE_MASK;
					fl_que_ctrl[0] |=
						(ple_txcmd_queue_empty_info[i]
							 .Portid
				<< WF_PLE_TOP_FL_QUE_CTRL_0_Q_BUF_PID_SHFT);
					fl_que_ctrl[0] |=
						(ple_txcmd_queue_empty_info[i]
							 .Queueid
				 << WF_PLE_TOP_FL_QUE_CTRL_0_Q_BUF_QID_SHFT);
				} else
					continue;

				HAL_MCR_WR(prAdapter,
					   WF_PLE_TOP_FL_QUE_CTRL_0_ADDR,
					   fl_que_ctrl[0]);
				HAL_MCR_RD(prAdapter,
					   WF_PLE_TOP_FL_QUE_CTRL_2_ADDR,
					   &fl_que_ctrl[1]);
				HAL_MCR_RD(prAdapter,
					   WF_PLE_TOP_FL_QUE_CTRL_3_ADDR,
					   &fl_que_ctrl[2]);
				hfid = (fl_que_ctrl[1] &
				WF_PLE_TOP_FL_QUE_CTRL_2_QUEUE_HEAD_FID_MASK) >>
				WF_PLE_TOP_FL_QUE_CTRL_2_QUEUE_HEAD_FID_SHFT;
				tfid = (fl_que_ctrl[1] &
				WF_PLE_TOP_FL_QUE_CTRL_2_QUEUE_TAIL_FID_MASK) >>
				WF_PLE_TOP_FL_QUE_CTRL_2_QUEUE_TAIL_FID_SHFT;
				pktcnt =
				(fl_que_ctrl[2] &
				WF_PLE_TOP_FL_QUE_CTRL_3_QUEUE_PKT_NUM_MASK) >>
				WF_PLE_TOP_FL_QUE_CTRL_3_QUEUE_PKT_NUM_SHFT;
				DBGLOG(HAL, INFO, "tail/head fid =");
				DBGLOG(HAL, INFO, "0x%03x/0x%03x,", tfid, hfid);
				DBGLOG(HAL, INFO, "pkt cnt = 0x%03x\n", pktcnt);
			}
		}
	}
#endif

	DBGLOG(HAL, INFO, "WF_PLE_TOP_INT_N9_ERR_STS=0x%08x\n", ple_err);
	DBGLOG(HAL, INFO, "WF_PLE_TOP_INT_N9_ERR_STS_1=0x%08x\n", ple_err1);
}

static void show_wfdma_ring_info(
	IN struct ADAPTER *prAdapter,
	IN enum _ENUM_WFDMA_TYPE_T enum_wfdma_type)
{

	uint32_t idx;
	uint32_t group_cnt;
	uint32_t u4DmaCfgCrAddr;
	struct wfdma_group_info *group;
	uint32_t u4_hw_desc_base_value = 0;
	uint32_t u4_hw_cnt_value = 0;
	uint32_t u4_hw_cidx_value = 0;
	uint32_t u4_hw_didx_value = 0;
	uint32_t queue_cnt;

	if (enum_wfdma_type != WFDMA_TYPE_HOST) {
		DBGLOG(HAL, WARN, "MT7902 only support Host WFDMA\n");
		return;
	}

	/* Dump All Ring Info */
	DBGLOG(HAL, INFO, "TRX Ring Configuration\n");
	DBGLOG(HAL, INFO, "%4s %20s %10s %12s %8s %8s %8s %8s\n",
		"Idx", "Attr", "Reg", "Base", "Cnt", "CIDX", "DIDX", "QCnt");


	/* Dump TX Ring */
	group_cnt = sizeof(wfmda_host_tx_group) /
	sizeof(struct wfdma_group_info);

	for (idx = 0; idx < group_cnt; idx++) {
		group = &wfmda_host_tx_group[idx];

		u4DmaCfgCrAddr = group->hw_desc_base;

		HAL_MCR_RD(prAdapter, u4DmaCfgCrAddr, &u4_hw_desc_base_value);
		HAL_MCR_RD(prAdapter, u4DmaCfgCrAddr+0x04, &u4_hw_cnt_value);
		HAL_MCR_RD(prAdapter, u4DmaCfgCrAddr+0x08, &u4_hw_cidx_value);
		HAL_MCR_RD(prAdapter, u4DmaCfgCrAddr+0x0c, &u4_hw_didx_value);

		queue_cnt = (u4_hw_cidx_value >= u4_hw_didx_value) ?
			(u4_hw_cidx_value - u4_hw_didx_value) :
			(u4_hw_cidx_value - u4_hw_didx_value + u4_hw_cnt_value);

		DBGLOG(HAL, INFO,
		       "%4d %20s 0x%08X 0x%10X 0x%06X 0x%06X 0x%06X 0x%06X\n",
		       idx, group->name, u4DmaCfgCrAddr, u4_hw_desc_base_value,
		       u4_hw_cnt_value, u4_hw_cidx_value, u4_hw_didx_value,
		       queue_cnt);
	}


	/* Dump RX Ring */
	group_cnt = sizeof(wfmda_host_rx_group) /
	sizeof(struct wfdma_group_info);

	for (idx = 0; idx < group_cnt; idx++) {
		group = &wfmda_host_rx_group[idx];

		u4DmaCfgCrAddr = group->hw_desc_base;

		HAL_MCR_RD(prAdapter, u4DmaCfgCrAddr, &u4_hw_desc_base_value);
		HAL_MCR_RD(prAdapter, u4DmaCfgCrAddr+0x04, &u4_hw_cnt_value);
		HAL_MCR_RD(prAdapter, u4DmaCfgCrAddr+0x08, &u4_hw_cidx_value);
		HAL_MCR_RD(prAdapter, u4DmaCfgCrAddr+0x0c, &u4_hw_didx_value);

		queue_cnt = (u4_hw_didx_value > u4_hw_cidx_value) ?
			(u4_hw_didx_value - u4_hw_cidx_value - 1) :
			(u4_hw_didx_value - u4_hw_cidx_value
			+ u4_hw_cnt_value - 1);

		DBGLOG(HAL, INFO,
		       "%4d %20s 0x%08X 0x%10X 0x%06X 0x%06X 0x%06X 0x%06X\n",
		       idx, group->name, u4DmaCfgCrAddr, u4_hw_desc_base_value,
		       u4_hw_cnt_value, u4_hw_cidx_value, u4_hw_didx_value,
		       queue_cnt);
	}

}

void mt7902_show_wfdma_info(
	IN struct ADAPTER *prAdapter)
{
	/* Dump Host WFMDA info */
	DBGLOG(HAL, INFO, "==============================\n");
	DBGLOG(HAL, INFO, "HOST WFMDA Configuration:\n");
	DBGLOG(HAL, INFO, "==============================\n");
	show_wfdma_interrupt_info(prAdapter, WFDMA_TYPE_HOST);
	show_wfdma_glo_info(prAdapter, WFDMA_TYPE_HOST);
	show_wfdma_ring_info(prAdapter, WFDMA_TYPE_HOST);
	show_wfdma_dbg_probe_info(prAdapter, WFDMA_TYPE_HOST);

	/* MT7902 doesn't have MCU WFDMA */
#if 0
	/* Dump FW WFDMA info */
	DBGLOG(HAL, INFO, "==============================\n");
	DBGLOG(HAL, INFO, "WM WFMDA Configuration:\n");
	DBGLOG(HAL, INFO, "==============================\n");
	show_wfdma_interrupt_info(prAdapter, WFDMA_TYPE_WM);
	show_wfdma_glo_info(prAdapter, WFDMA_TYPE_WM);
	show_wfdma_ring_info(prAdapter, WFDMA_TYPE_WM);
	show_wfdma_dbg_probe_info(prAdapter, WFDMA_TYPE_WM);
#endif
}

#if defined(_HIF_SDIO)
u_int8_t mt7902_sdio_read_wifi_mcu(IN struct ADAPTER *prAdapter,
	IN uint8_t ucPcLogSel, OUT uint32_t *pu4RetVal)
{
	uint32_t u4Val = 0;

	if (pu4RetVal == NULL) {
		DBGLOG(INIT, WARN, "pu4RetVal should not be NULL!!!\n");
		return FALSE;
	}

	HAL_MCR_RD(prAdapter, MCR_DB_COMDBGCR_SEL, &u4Val);
	u4Val = PC_IDX_SWH(u4Val, ucPcLogSel, CONNAC2X_SDIO_MCU_PC_LOG_MASK,
		CONNAC2X_SDIO_MCU_PC_LOG_SHIFT);
	HAL_MCR_WR(prAdapter, MCR_DB_COMDBGCR_SEL, u4Val);
	HAL_MCR_RD(prAdapter, MCR_DB_COMDBGCR, pu4RetVal);

	return TRUE;
}


u_int8_t mt7902_sdio_show_mcu_debug_info(struct ADAPTER *prAdapter,
	IN uint8_t *pucBuf, IN uint32_t u4Max, IN uint8_t ucFlag,
	OUT uint32_t *pu4Length)
{
	uint32_t u4Val = 0;
	uint8_t i = 0;

	if (pucBuf) {
		LOGBUF(pucBuf, u4Max, *pu4Length, "\n");
		LOGBUF(pucBuf, u4Max, *pu4Length,
			"----<Dump MCU Debug Information>----\n");
	}
	/* Set mode to 3'b01:con1_debug_rdata (wf_mcu) */
	HAL_MCR_WR(prAdapter, MCR_WF_MIXED_DEBUG_SEL, SDIO_CON1_DBG_RDATA);

	/* Enable SDIO mcu debug function. */
	HAL_MCR_RD(prAdapter, MCR_DB_COMDBGCR_SEL, &u4Val);
	u4Val |= SDIO_CTRL_EN;
	u4Val &= CONNAC2X_SDIO_WM_MONITER_SEL;
	u4Val &= CONNAC2X_SDIO_PC_MONITER_SEL;
	HAL_MCR_WR(prAdapter, MCR_DB_COMDBGCR_SEL, u4Val);

	mt7902_sdio_read_wifi_mcu(prAdapter, CURRENT_PC, &u4Val);

	DBGLOG(INIT, INFO, "Current PC LOG: 0x%08x\n", u4Val);
	if (pucBuf)
		LOGBUF(pucBuf, u4Max, *pu4Length,
		"Current PC LOG: 0x%08x\n", u4Val);

	/*
	 * Prevent dump log too much, because 7961 cmd res
	 * is not sufficient, so will wakeup hif_thread() to dump
	 * debug info frequently.
	 */
	if (ucFlag != DBG_MCU_DBG_CURRENT_PC) {
		mt7902_sdio_read_wifi_mcu(prAdapter, PC_LOG_IDX, &u4Val);
		DBGLOG(INIT, INFO, "PC log contorl=0x%08x\n", u4Val);
		if (pucBuf)
			LOGBUF(pucBuf, u4Max, *pu4Length,
			"PC log contorl=0x%08x\n", u4Val);

		for (i = 0; i < PC_LOG_NUM; i++) {
			mt7902_sdio_read_wifi_mcu(prAdapter, i, &u4Val);
			DBGLOG(INIT, INFO, "PC log(%d)=0x%08x\n", i, u4Val);
			if (pucBuf)
				LOGBUF(pucBuf, u4Max, *pu4Length,
				"PC log(%d)=0x%08x\n", i, u4Val);
		}
		/* Switch to LR. */
		HAL_MCR_RD(prAdapter, MCR_DB_COMDBGCR_SEL, &u4Val);
		u4Val |= CONNAC2X_SDIO_LR_MONITER_SEL;
		HAL_MCR_WR(prAdapter, MCR_DB_COMDBGCR_SEL, u4Val);

		mt7902_sdio_read_wifi_mcu(prAdapter, PC_LOG_IDX, &u4Val);
		DBGLOG(INIT, INFO, "LR log contorl=0x%08x\n", u4Val);
		if (pucBuf)
			LOGBUF(pucBuf, u4Max, *pu4Length,
			"LR log contorl=0x%08x\n", u4Val);

		for (i = 0; i < PC_LOG_NUM; i++) {
			mt7902_sdio_read_wifi_mcu(prAdapter, i, &u4Val);
			DBGLOG(INIT, INFO, "LR log(%d)=0x%08x\n", i, u4Val);
			if (pucBuf)
				LOGBUF(pucBuf, u4Max, *pu4Length,
				"LR log(%d)=0x%08x\n", i, u4Val);
		}
	}

	return TRUE;
}
#endif /* defined(_HIF_SDIO)  */

#if (CFG_SUPPORT_DEBUG_SOP == 1)
#if defined(_HIF_PCIE)
void pcie_mt7902_dump_power_debug_cr(struct ADAPTER *prAdapter)
{
	uint32_t u4Val = 0;

	HAL_MCR_RD(prAdapter, CONNAC25_PCIE_POWER_STATUS, &u4Val);
	DBGLOG(HAL, INFO, "Power status CR: 0x%08x, Val: 0x%08x\n",
		CONNAC25_PCIE_POWER_STATUS, u4Val);
}

void pcie_mt7902_dump_wfsys_debug_cr(struct ADAPTER *prAdapter,
	uint8_t ucCase)
{
	uint8_t i = 0;
	uint32_t u4Val = 0;

	if (ucCase == SLAVENORESP) {
		DBGLOG(HAL, INFO,
		 "--WFSYS sel CR: 0x18060164, status CR: 0x18060168 loop start--\n"
		);
		for (i = 0; i < mt7902_debug_sop_info->wfsys_csr_cr_num; i++) {
			HAL_MCR_WR(prAdapter,
			  CONNAC25_PCIE_WF_DEBUG_SEL,
			  mt7902_debug_sop_info->wfsys_csr_status[i]);

			HAL_MCR_RD(prAdapter,
			  CONNAC25_PCIE_WF_DEBUG_STATUS, &u4Val);

			DBGLOG(HAL, INFO,
			  "WFSYS sel: 0x%08x, Val: 0x%08x\n",
			  mt7902_debug_sop_info->wfsys_csr_status[i], u4Val);
		}
		DBGLOG(HAL, INFO,
		 "--WFSYS sel CR: 0x18060164, status CR: 0x18060168 loop end--\n"
		);
		DBGLOG(HAL, INFO,
		 "--WFSYS sel CR: 0x18060094, status CR: 0x1806021C loop start--\n"
		);
		for (i = 0; i < mt7902_debug_sop_info->wfsys_cr_num; i++) {
			HAL_MCR_WR(prAdapter,
			  CONNAC2X_PCIE_DEBUG_SEL,
			  mt7902_debug_sop_info->wfsys_status[i]);

			HAL_MCR_RD(prAdapter,
			  CONNAC2X_PCIE_DEBUG_STATUS, &u4Val);

			DBGLOG(HAL, INFO,
			  "WFSYS sel: 0x%08x, Val: 0x%08x\n",
			  mt7902_debug_sop_info->wfsys_status[i], u4Val);
		}
		DBGLOG(HAL, INFO,
		 "--WFSYS sel CR: 0x18060094, status CR: 0x1806021C loop end--\n"
		);
		HAL_MCR_RD(prAdapter, 0x88000444, &u4Val);
		DBGLOG(HAL, INFO, "WFSYS BUS CR: 0x88000444, Val: 0x%08x\n",
			u4Val);

		HAL_MCR_RD(prAdapter, 0x88000430, &u4Val);
		DBGLOG(HAL, INFO, "WFSYS BUS CR: 0x88000430, Val: 0x%08x\n",
			u4Val);

		HAL_MCR_RD(prAdapter, 0x8800044C, &u4Val);
		DBGLOG(HAL, INFO, "WFSYS BUS CR: 0x8800044C, Val: 0x%08x\n",
			u4Val);

		HAL_MCR_RD(prAdapter, 0x88000450, &u4Val);
		DBGLOG(HAL, INFO, "WFSYS BUS CR: 0x88000450, Val: 0x%08x\n",
			u4Val);
	}
	DBGLOG(HAL, INFO,
	 "--WFSYS sel CR: 0x18060094, status CR: 0x1806021C loop start--\n"
	);
	for (i = 0; i < mt7902_debug_sop_info->wfsys_sleep_cr_num; i++) {
		HAL_MCR_WR(prAdapter,
		  CONNAC2X_PCIE_DEBUG_SEL,
		  mt7902_debug_sop_info->wfsys_sleep_status[i]);

		HAL_MCR_RD(prAdapter,
		  CONNAC2X_PCIE_DEBUG_STATUS, &u4Val);

		DBGLOG(HAL, INFO,
		  "WFSYS sleep sel: 0x%08x, Val: 0x%08x\n",
		  mt7902_debug_sop_info->wfsys_sleep_status[i], u4Val);
	}
	DBGLOG(HAL, INFO,
	 "--WFSYS sel CR: 0x18060094, status CR: 0x1806021C loop end--\n"
	);
}

void pcie_mt7902_dump_bgfsys_debug_cr(struct ADAPTER *prAdapter,
	uint8_t ucCase)
{
	uint8_t i = 0;
	uint32_t u4Val = 0;

	if (ucCase == SLEEP) {
		HAL_MCR_WR(prAdapter, CONNAC25_PCIE_BGF_DBG_SEL, 0x0081);
		HAL_MCR_RD(prAdapter, CONNAC25_PCIE_BGF_DBG_STATUS, &u4Val);

		DBGLOG(HAL, INFO, "BGFSYS sel: 0x0081, Val: 0x%08x\n", u4Val);

		HAL_MCR_WR(prAdapter,
		  CONNAC2X_PCIE_DBG_BGF_MCU_SEL_CR,
		  BGF_MCU_PC_LOG_SEL);

		HAL_MCR_RD(prAdapter,
		  CONNAC2X_PCIE_BGF_MCU_PC_DBG_STS, &u4Val);
		DBGLOG(HAL, INFO, "BGFSYS MCU CR: 0x%08x, Val: 0x%08x\n",
		  CONNAC2X_PCIE_BGF_MCU_PC_DBG_STS, u4Val);

		HAL_MCR_RD(prAdapter,
		  CONNAC2X_PCIE_MCU_BGF_ON_DBG_STS, &u4Val);

		DBGLOG(HAL, INFO, "BGFSYS MCU CR: 0x%08x, Val: 0x%08x\n",
		  CONNAC2X_PCIE_MCU_BGF_ON_DBG_STS, u4Val);
	}
	if (ucCase == SLAVENORESP) {
		/* For pin mux setting. */
		HAL_MCR_WR(prAdapter, 0xE00A0, 0xA8);
		DBGLOG(HAL, INFO,
		 "--BGF sel CR: 0x180600A8, status CR: 0x1806022C loop start--\n"
		);
		for (i = 0; i < mt7902_debug_sop_info->bgfsys_bus_cr_num; i++) {
			HAL_MCR_WR(prAdapter,
			  CONNAC25_PCIE_BGF_BUS_SEL,
			  mt7902_debug_sop_info->bgfsys_bus_status[i]);

			HAL_MCR_RD(prAdapter,
			  CONNAC25_PCIE_BGF_BUS_STATUS, &u4Val);

			DBGLOG(HAL, INFO,
			  "BGFSYS bus sel: 0x%08x, Val: 0x%08x\n",
			  mt7902_debug_sop_info->bgfsys_bus_status[i], u4Val);
		}
		DBGLOG(HAL, INFO,
		 "--BGF sel CR: 0x180600A8, status CR: 0x1806022C loop end--\n"
		);
	}
	DBGLOG(HAL, INFO,
	 "--BGF sel CR: 0x180600AC, status CR: 0x1806023C loop start--\n"
	);
	for (i = 0; i < mt7902_debug_sop_info->bgfsys_cr_num; i++) {
		HAL_MCR_WR(prAdapter,
		  CONNAC25_PCIE_BGF_DBG_SEL,
		  mt7902_debug_sop_info->bgfsys_status[i]);

		HAL_MCR_RD(prAdapter,
		  CONNAC25_PCIE_BGF_DBG_STATUS, &u4Val);

		DBGLOG(HAL, INFO,
		  "BGFSYS sel: 0x%08x, Val: 0x%08x\n",
		  mt7902_debug_sop_info->bgfsys_status[i], u4Val);
	}
	DBGLOG(HAL, INFO,
	 "--BGF sel CR: 0x180600AC, status CR: 0x1806023C loop end--\n"
	);
}

void pcie_mt7902_dump_conninfra_debug_cr(struct ADAPTER *prAdapter,
	uint8_t ucCase)
{
	uint8_t i = 0;
	uint32_t u4Val = 0;

	DBGLOG(HAL, INFO,
	 "--CONNINFRA sel CR: 0x1806015C, status CR: 0x180602C8 loop start--\n"
	);
	for (i = 0; i < mt7902_debug_sop_info->conninfra_cr_num; i++) {
		HAL_MCR_WR(prAdapter, CONNAC25_PCIE_CONNINFRA_CLK_SEL,
		  mt7902_debug_sop_info->conninfra_status[i]);

		HAL_MCR_RD(prAdapter, CONNAC25_PCIE_CONNINFRA_CLK_STATUS,
		  &u4Val);

		DBGLOG(HAL, INFO, "CONNINFRA sel: 0x%08x, Val: 0x%08x\n",
		  mt7902_debug_sop_info->conninfra_status[i], u4Val);
	}
	DBGLOG(HAL, INFO,
	 "--CONNINFRA sel CR: 0x1806015C, status CR: 0x180602C8 loop end--\n"
	);

	if (ucCase == SLAVENORESP) {
		HAL_MCR_RD(prAdapter, CONNAC25_PCIE_CONNINFRA_STRAP_STATUS,
		  &u4Val);
		DBGLOG(HAL, INFO, "CONNINFRA STRAP: 0x%08x, Val: 0x%08x\n",
		  CONNAC25_PCIE_CONNINFRA_STRAP_STATUS, u4Val);

		for (i = 0; i < mt7902_debug_sop_info->conninfra_bus_cr_num;
			i++) {
			HAL_MCR_RD(prAdapter,
			  mt7902_debug_sop_info->conninfra_bus_cr[i],
			  &u4Val);

			DBGLOG(HAL, INFO,
			  "CONNINFRA bus cr: 0x%08x, Val: 0x%08x\n",
			  mt7902_debug_sop_info->conninfra_bus_cr[i], u4Val);
		}
		/* off domain */
		HAL_MCR_WR(prAdapter, 0xE0000, 0x1);
		HAL_MCR_RD(prAdapter, 0xE0000, &u4Val);
		DBGLOG(HAL, INFO,
		  "CONNINFRA off domain cr: 0x18060000, Val: 0x%08x\n", u4Val);
		HAL_MCR_RD(prAdapter, 0xE02D4, &u4Val);
		if (!(u4Val & BIT(0)))
			DBGLOG(HAL, INFO, "CONNINFRA off domain bus hang!!!\n");

		HAL_MCR_RD(prAdapter, 0xF1000, &u4Val);
		DBGLOG(HAL, INFO,
		  "CONNINFRA off domain cr: 0x18001000, Val: 0x%08x\n", u4Val);

		DBGLOG(HAL, INFO,
		 "--CONNINFRA sel CR: 0x18060138, status CR: 0x18060150 loop start--\n"
		);
		for (i = 0; i < mt7902_debug_sop_info->conninfra_signal_num;
			i++) {
			HAL_MCR_WR(prAdapter,
			  CONNAC2X_PCIE_CONN_INFRA_BUS_SEL,
			  mt7902_debug_sop_info->conninfra_signal_status[i]);

			HAL_MCR_RD(prAdapter,
			  CONNAC2X_PCIE_CONN_INFRA_BUS_STATUS,
			  &u4Val);

			DBGLOG(HAL, INFO,
			  "CONNINFRA sel: 0x%08x, Val: 0x%08x\n",
			  mt7902_debug_sop_info->conninfra_signal_status[i],
			  u4Val);
		}
		DBGLOG(HAL, INFO,
		 "--CONNINFRA sel CR: 0x18060138, status CR: 0x18060150 loop end--\n"
		);
	}
}
#endif /*#if defined(_HIF_PCIE) */

#if defined(_HIF_USB)
void usb_mt7902_dump_power_debug_cr(struct ADAPTER *prAdapter)
{
	uint32_t u4Val = 0;
	u_int8_t fgStatus = FALSE;

	HAL_UHW_WR(prAdapter, CONNAC2X_UDMA_CONN_INFRA_STATUS_SEL,
		0x9F1E0000, &fgStatus);
	HAL_UHW_RD(prAdapter, CONNAC2X_UDMA_CONN_INFRA_STATUS,
		&u4Val, &fgStatus);
	DBGLOG(HAL, INFO, "Power status sel CR:0x%08x, status CR:0x%08x\n",
		CONNAC2X_UDMA_CONN_INFRA_STATUS_SEL,
		CONNAC2X_UDMA_CONN_INFRA_STATUS);
	DBGLOG(HAL, INFO, "Power status sel: 0x9F1E0000, Val: 0x%08x\n",
		u4Val);
}

void usb_mt7902_dump_wfsys_debug_cr(struct ADAPTER *prAdapter,
	uint8_t ucCase)
{
	uint8_t i = 0;
	uint32_t u4Val = 0;
	u_int8_t fgStatus = FALSE;

	if (ucCase == SLAVENORESP) {
		DBGLOG(HAL, INFO,
		 "--WFSYS sel CR: 0x74000A4C, status CR: 0x74000A48 loop start--\n"
		);
		for (i = 0; i < mt7902_debug_sop_info->wfsys_csr_cr_num; i++) {
			HAL_UHW_WR(prAdapter, CONNAC2X_UDMA_CONN_INFRA_CLK_SEL,
			  mt7902_debug_sop_info->wfsys_csr_status[i],
			  &fgStatus);
			HAL_UHW_RD(prAdapter,
			  CONNAC2X_UDMA_CONN_INFRA_CLK_STATUS,
			  &u4Val, &fgStatus);

			DBGLOG(HAL, INFO,
			  "WFSYS sel: 0x%08x, Val: 0x%08x\n",
			  mt7902_debug_sop_info->wfsys_csr_status[i], u4Val);
		}
		DBGLOG(HAL, INFO,
		 "--WFSYS sel CR: 0x74000A4C, status CR: 0x74000A48 loop end--\n"
		);
		DBGLOG(HAL, INFO,
		 "--WFSYS sel CR: 0x74000A14, status CR: 0x74000A10 loop start--\n"
		);
		for (i = 0; i < mt7902_debug_sop_info->wfsys_cr_num; i++) {
			HAL_UHW_WR(prAdapter, CONNAC2X_UDMA_DBG_SEL,
			  mt7902_debug_sop_info->wfsys_status[i],
			  &fgStatus);
			HAL_UHW_RD(prAdapter,
			  CONNAC2X_UDMA_DBG_STATUS,
			  &u4Val, &fgStatus);

			DBGLOG(HAL, INFO,
			  "WFSYS sel: 0x%08x, Val: 0x%08x\n",
			  mt7902_debug_sop_info->wfsys_status[i], u4Val);
		}
		DBGLOG(HAL, INFO,
		 "--WFSYS sel CR: 0x74000A14, status CR: 0x74000A10 loop end--\n"
		);
	}

	DBGLOG(HAL, INFO,
	 "--WFSYS sel CR: 0x74000A14, status CR: 0x74000A10 loop start--\n"
	);
	for (i = 0; i < mt7902_debug_sop_info->wfsys_sleep_cr_num; i++) {
		HAL_UHW_WR(prAdapter, CONNAC2X_UDMA_DBG_SEL,
		  mt7902_debug_sop_info->wfsys_sleep_status[i],
		  &fgStatus);
		HAL_UHW_RD(prAdapter, CONNAC2X_UDMA_DBG_STATUS,
		  &u4Val, &fgStatus);

		DBGLOG(HAL, INFO,
		  "WFSYS sleep sel: 0x%08x, Val: 0x%08x\n",
		  mt7902_debug_sop_info->wfsys_sleep_status[i], u4Val);
	}
	DBGLOG(HAL, INFO,
	 "--WFSYS sel CR: 0x74000A14, status CR: 0x74000A10 loop end--\n"
	);
}

void usb_mt7902_dump_bgfsys_debug_cr(struct ADAPTER *prAdapter)
{
	uint8_t i = 0;
	uint32_t u4Val = 0;
	u_int8_t fgStatus = FALSE;

	DBGLOG(HAL, INFO,
	 "--BGF sel CR: 0x74000A44, status CR: 0x74000A40 loop start--\n"
	);
	for (i = 0; i < mt7902_debug_sop_info->bgfsys_cr_num; i++) {
		HAL_UHW_WR(prAdapter, CONNAC25_UDMA_BGF_DBG_SEL,
		  mt7902_debug_sop_info->bgfsys_status[i],
		  &fgStatus);
		HAL_UHW_RD(prAdapter, CONNAC25_UDMA_BGF_DBG_STATUS,
		  &u4Val, &fgStatus);

		DBGLOG(HAL, INFO,
		  "BGFSYS sel: 0x%08x, Val: 0x%08x\n",
		  mt7902_debug_sop_info->bgfsys_status[i], u4Val);
	}
	DBGLOG(HAL, INFO,
	 "--BGF sel CR: 0x74000A44, status CR: 0x74000A40 loop end--\n"
	);
}

void usb_mt7902_dump_conninfra_debug_cr(struct ADAPTER *prAdapter,
	uint8_t ucCase)
{
	uint8_t i = 0;
	uint32_t u4Val = 0;
	u_int8_t fgStatus = FALSE;

	DBGLOG(HAL, INFO,
	 "--CONNINFRA sel CR: 0x74000A4C, status CR: 0x74000A48 loop start--\n"
	);
	for (i = 0; i < mt7902_debug_sop_info->conninfra_cr_num; i++) {
		HAL_UHW_WR(prAdapter, CONNAC2X_UDMA_CONN_INFRA_CLK_SEL,
		  mt7902_debug_sop_info->conninfra_status[i],
		  &fgStatus);
		HAL_UHW_RD(prAdapter, CONNAC2X_UDMA_CONN_INFRA_CLK_STATUS,
		  &u4Val, &fgStatus);

		DBGLOG(HAL, INFO, "CONNINFRA sel: 0x%08x, Val: 0x%08x\n",
		  mt7902_debug_sop_info->conninfra_status[i], u4Val);
	}
	if (ucCase == SLAVENORESP) {
		HAL_UHW_WR(prAdapter, CONNAC2X_UDMA_CONN_INFRA_CLK_SEL,
		  0xD8000000, &fgStatus);
		HAL_UHW_RD(prAdapter, CONNAC2X_UDMA_CONN_INFRA_CLK_STATUS,
		  &u4Val, &fgStatus);

		DBGLOG(HAL, INFO,
		  "CONNINFRA STRAP sel CR:0x%08x, status CR:0x%08x\n",
		  CONNAC2X_UDMA_CONN_INFRA_CLK_SEL,
		  CONNAC2X_UDMA_CONN_INFRA_CLK_STATUS);

		DBGLOG(HAL, INFO,
		  "CONNINFRA STRAP sel: 0xD8000000, Val: 0x%08x\n",
		  u4Val);

		for (i = 0; i < mt7902_debug_sop_info->conninfra_bus_cr_num;
			i++) {
			HAL_UHW_WR(prAdapter,
			  CONNAC2X_UDMA_CONN_INFRA_CLK_SEL,
			  mt7902_debug_sop_info->conninfra_bus_status[i],
			  &fgStatus);
			HAL_UHW_RD(prAdapter,
			  CONNAC2X_UDMA_CONN_INFRA_CLK_STATUS,
			  &u4Val, &fgStatus);

			DBGLOG(HAL, INFO,
			  "CONNINFRA bus sel: 0x%08x, Val: 0x%08x\n",
			  mt7902_debug_sop_info->conninfra_bus_status[i],
			  u4Val);
		}

		for (i = 0; i < mt7902_debug_sop_info->conninfra_signal_num;
			i++) {
			HAL_UHW_WR(prAdapter, CONNAC2X_UDMA_CONN_INFRA_CLK_SEL,
			  mt7902_debug_sop_info->conninfra_signal_status[i],
			  &fgStatus);
			HAL_UHW_RD(prAdapter,
			  CONNAC2X_UDMA_CONN_INFRA_CLK_STATUS,
			  &u4Val, &fgStatus);

			DBGLOG(HAL, INFO,
			  "CONNINFRA sel: 0x%08x, Val: 0x%08x\n",
			  mt7902_debug_sop_info->conninfra_signal_status[i],
			  u4Val);
		}
	}
	DBGLOG(HAL, INFO,
	 "--CONNINFRA sel CR: 0x74000A4C, status CR: 0x74000A48 loop end--\n"
	);
}
#endif /* #if defined(_HIF_USB) */

#if defined(_HIF_SDIO)
void sdio_mt7902_dump_power_debug_cr(struct ADAPTER *prAdapter)
{
	uint32_t u4Val = 0;

	HAL_MCR_WR(prAdapter,
		MCR_WF_MIXED_DEBUG_SEL, SDIO_INFRA_CSR_RDATA);
	DBGLOG(HAL, INFO, "Switch 0x48 to 4(SDIO_INFRA_CSR_RDATA)\n");

	HAL_MCR_WR(prAdapter, MCR_DB_COMDBGCR_SEL, 0x9F1E0000);

	HAL_MCR_RD(prAdapter, MCR_DB_COMDBGCR, &u4Val);

	DBGLOG(HAL, INFO, "Power status sel CR:0x%08x, status CR:0x%08x\n",
		MCR_DB_COMDBGCR_SEL,
		MCR_DB_COMDBGCR);
	DBGLOG(HAL, INFO, "Power status sel: 0x9F1E0000, Val: 0x%08x\n",
		u4Val);
}

void sdio_mt7902_dump_wfsys_debug_cr(struct ADAPTER *prAdapter,
	uint8_t ucCase)
{
	uint8_t i = 0;
	uint32_t u4Val = 0;

	DBGLOG(HAL, INFO,
	 "--WFSYS sel CR: 0x44, status CR: 0x40 loop start--\n"
	);

	if (ucCase == SLAVENORESP) {
		HAL_MCR_WR(prAdapter, MCR_WF_MIXED_DEBUG_SEL,
			SDIO_INFRA_CSR_RDATA);
		DBGLOG(HAL, INFO, "Switch 0x48 to 4(SDIO_INFRA_CSR_RDATA)\n");

		for (i = 0; i < mt7902_debug_sop_info->wfsys_csr_cr_num; i++) {
			HAL_MCR_WR(prAdapter, MCR_DB_COMDBGCR_SEL,
			  mt7902_debug_sop_info->wfsys_csr_status[i]);

			HAL_MCR_RD(prAdapter, MCR_DB_COMDBGCR, &u4Val);

			DBGLOG(HAL, INFO,
			  "WFSYS sel: 0x%08x, Val: 0x%08x\n",
			  mt7902_debug_sop_info->wfsys_csr_status[i], u4Val);
		}
		HAL_MCR_WR(prAdapter, MCR_WF_MIXED_DEBUG_SEL,
			SDIO_CON_DBG_RDATA);
		DBGLOG(HAL, INFO, "Switch 0x48 to 0(SDIO_CON_DBG_RDATA)\n");

		for (i = 0; i < mt7902_debug_sop_info->wfsys_cr_num; i++) {
			HAL_MCR_WR(prAdapter, MCR_DB_COMDBGCR_SEL,
			  mt7902_debug_sop_info->wfsys_status[i]);

			HAL_MCR_RD(prAdapter, MCR_DB_COMDBGCR, &u4Val);

			DBGLOG(HAL, INFO,
			  "WFSYS sel: 0x%08x, Val: 0x%08x\n",
			  mt7902_debug_sop_info->wfsys_status[i], u4Val);
		}
		HAL_MCR_WR(prAdapter, MCR_WF_MIXED_DEBUG_SEL,
			SDIO_COM_DBG_RDATA);
		DBGLOG(HAL, INFO, "Switch 0x48 to 3(SDIO_COM_DBG_RDATA)\n");

		HAL_MCR_WR(prAdapter, MCR_DB_COMDBGCR_SEL,
			  0x0F0F0000);

		HAL_MCR_RD(prAdapter, MCR_DB_COMDBGCR, &u4Val);

		DBGLOG(HAL, INFO,
		  "WFSYS sel: 0x0F0F0000, Val: 0x%08x\n",
		  u4Val);
	}
	HAL_MCR_WR(prAdapter, MCR_WF_MIXED_DEBUG_SEL, SDIO_CON_DBG_RDATA);
	DBGLOG(HAL, INFO, "Switch 0x48 to 0(SDIO_CON_DBG_RDATA)\n");

	for (i = 0; i < mt7902_debug_sop_info->wfsys_sleep_cr_num; i++) {
		HAL_MCR_WR(prAdapter, MCR_DB_COMDBGCR_SEL,
		  mt7902_debug_sop_info->wfsys_sleep_status[i]);

		HAL_MCR_RD(prAdapter, MCR_DB_COMDBGCR, &u4Val);

		DBGLOG(HAL, INFO,
		  "WFSYS sleep sel: 0x%08x, Val: 0x%08x\n",
		  mt7902_debug_sop_info->wfsys_sleep_status[i], u4Val);
	}
	DBGLOG(HAL, INFO,
	 "--WFSYS sel CR: 0x44, status CR: 0x40 loop end--\n"
	);
}

void sdio_mt7902_dump_conninfra_debug_cr(struct ADAPTER *prAdapter,
	uint8_t ucCase)
{
	uint8_t i = 0;
	uint32_t u4Val = 0;

	DBGLOG(HAL, INFO,
	 "--CONNINFRA sel CR: 0x44, status CR: 0x40 loop start--\n"
	);
	HAL_MCR_WR(prAdapter, MCR_WF_MIXED_DEBUG_SEL, SDIO_INFRA_CSR_RDATA);
	DBGLOG(HAL, INFO, "Switch 0x48 to 4(SDIO_INFRA_CSR_RDATA)\n");

	for (i = 0; i < mt7902_debug_sop_info->conninfra_cr_num; i++) {
		HAL_MCR_WR(prAdapter, MCR_DB_COMDBGCR_SEL,
		  mt7902_debug_sop_info->conninfra_status[i]);

		HAL_MCR_RD(prAdapter, MCR_DB_COMDBGCR,
		  &u4Val);

		DBGLOG(HAL, INFO, "CONNINFRA sel: 0x%08x, Val: 0x%08x\n",
		  mt7902_debug_sop_info->conninfra_status[i], u4Val);
	}

	if (ucCase == SLAVENORESP) {
		HAL_MCR_WR(prAdapter, MCR_DB_COMDBGCR_SEL, 0xD8000000);

		HAL_MCR_RD(prAdapter, MCR_DB_COMDBGCR, &u4Val);

		DBGLOG(HAL, INFO,
		  "CONNINFRA STRAP sel: 0xD8000000, Val: 0x%08x\n",
		  u4Val);

		for (i = 0; i < mt7902_debug_sop_info->conninfra_bus_cr_num;
			i++) {
			HAL_MCR_WR(prAdapter, MCR_DB_COMDBGCR_SEL,
				mt7902_debug_sop_info->conninfra_bus_status[i]);

			HAL_MCR_RD(prAdapter, MCR_DB_COMDBGCR, &u4Val);

			DBGLOG(HAL, INFO,
			  "CONNINFRA bus sel: 0x%08x, Val: 0x%08x\n",
			  mt7902_debug_sop_info->conninfra_bus_status[i],
			  u4Val);
		}

		for (i = 0; i < mt7902_debug_sop_info->conninfra_signal_num;
			i++) {
			HAL_MCR_WR(prAdapter, MCR_DB_COMDBGCR_SEL,
			  mt7902_debug_sop_info->conninfra_signal_status[i]);

			HAL_MCR_RD(prAdapter, MCR_DB_COMDBGCR, &u4Val);

			DBGLOG(HAL, INFO,
			  "CONNINFRA sel: 0x%08x, Val: 0x%08x\n",
			  mt7902_debug_sop_info->conninfra_signal_status[i],
			  u4Val);
		}
	}
	DBGLOG(HAL, INFO,
	 "--CONNINFRA sel CR: 0x44, status CR: 0x40 loop end--\n"
	);
}
#endif /* #if defined(_HIF_SDIO)  */

u_int8_t mt7902_show_debug_sop_info(struct ADAPTER *prAdapter,
	uint8_t ucCase)
{

	switch (ucCase) {
	case SLEEP:
		/* Check power status */
		DBGLOG(HAL, ERROR, "Sleep Fail!\n");
#if defined(_HIF_PCIE)
		pcie_mt7902_dump_power_debug_cr(prAdapter);
		pcie_mt7902_dump_wfsys_debug_cr(prAdapter, ucCase);
		pcie_mt7902_dump_bgfsys_debug_cr(prAdapter, ucCase);
		pcie_mt7902_dump_conninfra_debug_cr(prAdapter, ucCase);
#elif defined(_HIF_USB)
		usb_mt7902_dump_power_debug_cr(prAdapter);
		usb_mt7902_dump_wfsys_debug_cr(prAdapter, ucCase);
		usb_mt7902_dump_bgfsys_debug_cr(prAdapter);
		usb_mt7902_dump_conninfra_debug_cr(prAdapter, ucCase);
#elif defined(_HIF_SDIO)
		sdio_mt7902_dump_power_debug_cr(prAdapter);
		sdio_mt7902_dump_wfsys_debug_cr(prAdapter, ucCase);
		sdio_mt7902_dump_conninfra_debug_cr(prAdapter, ucCase);
#endif
		break;
	case SLAVENORESP:
		DBGLOG(HAL, ERROR, "Slave no response!\n");
#if defined(_HIF_PCIE)
		pcie_mt7902_dump_wfsys_debug_cr(prAdapter, ucCase);
		pcie_mt7902_dump_bgfsys_debug_cr(prAdapter, ucCase);
		pcie_mt7902_dump_conninfra_debug_cr(prAdapter, ucCase);
#elif defined(_HIF_USB)
		usb_mt7902_dump_wfsys_debug_cr(prAdapter, ucCase);
		usb_mt7902_dump_conninfra_debug_cr(prAdapter, ucCase);
#elif defined(_HIF_SDIO)
		sdio_mt7902_dump_wfsys_debug_cr(prAdapter, ucCase);
		sdio_mt7902_dump_conninfra_debug_cr(prAdapter, ucCase);
#endif
		break;
	default:
		break;
	}

	return TRUE;
}
#endif /* (CFG_SUPPORT_DEBUG_SOP == 1)  */


#endif /* defined(MT7902) */
