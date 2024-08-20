/* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause */
/*
 * Copyright (c) 2016 MediaTek Inc.
 */
/******************************************************************************
 *[File]             dbg_connac.c
 *[Version]          v1.0
 *[Revision Date]    2015-09-08
 *[Author]
 *[Description]
 *    The program provides WIFI MAC Debug APIs
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

/*******************************************************************************
 *                              F U N C T I O N S
 *******************************************************************************
 */

void halShowPseInfo(IN struct ADAPTER *prAdapter)
{
	uint32_t pse_buf_ctrl = 0, pg_sz, pg_num;
	uint32_t pse_stat = 0, pg_flow_ctrl[16] = {0};
	uint32_t fpg_cnt, ffa_cnt, fpg_head, fpg_tail;
	uint32_t max_q, min_q, rsv_pg, used_pg;
	uint32_t i, page_offset, addr, value = 0;
	uint32_t txd_payload_info[16] = {0};

	HAL_MCR_RD(prAdapter, PSE_PBUF_CTRL, &pse_buf_ctrl);
	HAL_MCR_RD(prAdapter, PSE_QUEUE_EMPTY, &pse_stat);
	HAL_MCR_RD(prAdapter, PSE_FREEPG_CNT, &pg_flow_ctrl[0]);
	HAL_MCR_RD(prAdapter, PSE_FREEPG_HEAD_TAIL, &pg_flow_ctrl[1]);
	HAL_MCR_RD(prAdapter, PSE_PG_HIF0_GROUP, &pg_flow_ctrl[2]);
	HAL_MCR_RD(prAdapter, PSE_HIF0_PG_INFO, &pg_flow_ctrl[3]);
	HAL_MCR_RD(prAdapter, PSE_PG_HIF1_GROUP, &pg_flow_ctrl[4]);
	HAL_MCR_RD(prAdapter, PSE_HIF1_PG_INFO, &pg_flow_ctrl[5]);
	HAL_MCR_RD(prAdapter, PSE_PG_CPU_GROUP, &pg_flow_ctrl[6]);
	HAL_MCR_RD(prAdapter, PSE_CPU_PG_INFO, &pg_flow_ctrl[7]);
	HAL_MCR_RD(prAdapter, PSE_PG_LMAC0_GROUP, &pg_flow_ctrl[8]);
	HAL_MCR_RD(prAdapter, PSE_LMAC0_PG_INFO, &pg_flow_ctrl[9]);
	HAL_MCR_RD(prAdapter, PSE_PG_LMAC1_GROUP, &pg_flow_ctrl[10]);
	HAL_MCR_RD(prAdapter, PSE_LMAC1_PG_INFO, &pg_flow_ctrl[11]);
	HAL_MCR_RD(prAdapter, PSE_PG_LMAC2_GROUP, &pg_flow_ctrl[12]);
	HAL_MCR_RD(prAdapter, PSE_LMAC2_PG_INFO, &pg_flow_ctrl[13]);
	HAL_MCR_RD(prAdapter, PSE_PG_PLE_GROUP, &pg_flow_ctrl[14]);
	HAL_MCR_RD(prAdapter, PSE_PLE_PG_INFO, &pg_flow_ctrl[15]);

	/* Configuration Info */
	DBGLOG(HAL, INFO, "PSE Configuration Info:\n");

	HAL_MCR_RD(prAdapter, PSE_GC, &value);
	DBGLOG(HAL, INFO, "\tGC(0x82068000): 0x%08x\n", value);
	HAL_MCR_RD(prAdapter, PSE_INT_STS, &value);
	DBGLOG(HAL, INFO, "\tINT_STS(0x82068024): 0x%08x\n", value);
	HAL_MCR_RD(prAdapter, PSE_INT_ERR_STS, &value);
	DBGLOG(HAL, INFO, "\tINT_ERR_STS(0x82068028): 0x%08x\n", value);

	DBGLOG(HAL, INFO, "\tPacket Buffer Control(0x82068014): 0x%08x\n",
		pse_buf_ctrl);
	pg_sz = (pse_buf_ctrl & (0x1 << 31)) >> 31;
	DBGLOG(HAL, INFO,
		"\t\tPage Size=%d(%d bytes per page)\n",
		pg_sz, (pg_sz == 1 ? 256 : 128));
	page_offset  = (pse_buf_ctrl & 0x3FFFFFF) >> 17;
	DBGLOG(HAL, INFO,
		 "\t\tPage Offset=%d(in unit of 2KB)\n", page_offset);
	pg_num = (pse_buf_ctrl & PSE_TOTAL_PAGE_NUM_MASK);
	DBGLOG(HAL, INFO, "\t\tAvailable Total Page=%d pages\n", pg_num);
	/* Page Flow Control */
	DBGLOG(HAL, INFO, "PSE Page Flow Control:\n");

	DBGLOG(HAL, INFO,
		"\tFree page counter(0x82068100): 0x%08x\n", pg_flow_ctrl[0]);
	fpg_cnt = pg_flow_ctrl[0] & 0xfff;
	DBGLOG(HAL, INFO,
		"\t\tThe toal page number of free=0x%03x\n", fpg_cnt);
	ffa_cnt = (pg_flow_ctrl[0] & (0xfff << 16)) >> 16;
	DBGLOG(HAL, INFO,
		"\t\tThe free page numbers of free for all=0x%03x\n", ffa_cnt);
	DBGLOG(HAL, INFO,
		"\tFree page head and tail(0x82068104): 0x%08x\n",
		pg_flow_ctrl[1]);
	fpg_head = pg_flow_ctrl[1] & 0xfff;
	fpg_tail = (pg_flow_ctrl[1] & (0xfff << 16)) >> 16;
	DBGLOG(HAL, INFO,
		"\t\tThe tail/head page of free page list=0x%03x/0x%03x\n",
		fpg_tail, fpg_head);
	DBGLOG(HAL, INFO,
		"\tReserved page counter of HIF0 group(0x82068110): 0x%08x\n",
		pg_flow_ctrl[2]);
	DBGLOG(HAL, INFO,
		"\tHIF0 group page status(0x82068114): 0x%08x\n",
		pg_flow_ctrl[3]);
	min_q = pg_flow_ctrl[2] & 0xfff;
	max_q = (pg_flow_ctrl[2] & (0xfff << 16)) >> 16;
	DBGLOG(HAL, INFO,
		"\t\tThe max/min quota pages of HIF0 group=0x%03x/0x%03x\n",
		max_q, min_q);
	rsv_pg = pg_flow_ctrl[3] & 0xfff;
	used_pg = (pg_flow_ctrl[3] & (0xfff << 16)) >> 16;
	DBGLOG(HAL, INFO,
		"\t\tThe used/reserved pages of HIF0 group=0x%03x/0x%03x\n",
		used_pg, rsv_pg);
	DBGLOG(HAL, INFO,
		"\tReserved page counter of HIF1 group(0x82068118): 0x%08x\n",
		pg_flow_ctrl[4]);
	DBGLOG(HAL, INFO,
		"\tHIF1 group page status(0x8206811c): 0x%08x\n",
		pg_flow_ctrl[5]);
	min_q = pg_flow_ctrl[4] & 0xfff;
	max_q = (pg_flow_ctrl[4] & (0xfff << 16)) >> 16;
	DBGLOG(HAL, INFO,
		"\t\tThe max/min quota pages of HIF1 group=0x%03x/0x%03x\n",
		max_q, min_q);
	rsv_pg = pg_flow_ctrl[5] & 0xfff;
	used_pg = (pg_flow_ctrl[5] & (0xfff << 16)) >> 16;
	DBGLOG(HAL, INFO,
		"\t\tThe used/reserved pages of HIF1 group=0x%03x/0x%03x\n",
		used_pg, rsv_pg);
	DBGLOG(HAL, INFO,
		"\tReserved page counter of CPU group(0x82068150): 0x%08x\n",
		pg_flow_ctrl[6]);
	DBGLOG(HAL, INFO,
		"\tCPU group page status(0x82068154): 0x%08x\n",
		pg_flow_ctrl[7]);
	min_q = pg_flow_ctrl[6] & 0xfff;
	max_q = (pg_flow_ctrl[6] & (0xfff << 16)) >> 16;
	DBGLOG(HAL, INFO,
		"\t\tThe max/min quota pages of CPU group=0x%03x/0x%03x\n",
		max_q, min_q);
	rsv_pg = pg_flow_ctrl[7] & 0xfff;
	used_pg = (pg_flow_ctrl[7] & (0xfff << 16)) >> 16;
	DBGLOG(HAL, INFO,
		"\t\tThe used/reserved pages of CPU group=0x%03x/0x%03x\n",
		used_pg, rsv_pg);
	DBGLOG(HAL, INFO,
		"\tReserved page counter of LMAC0 group(0x82068170): 0x%08x\n",
		pg_flow_ctrl[8]);
	DBGLOG(HAL, INFO,
		"\tLMAC0 group page status(0x82068174): 0x%08x\n",
		pg_flow_ctrl[9]);
	min_q = pg_flow_ctrl[8] & 0xfff;
	max_q = (pg_flow_ctrl[8] & (0xfff << 16)) >> 16;
	DBGLOG(HAL, INFO,
		"\t\tThe max/min quota pages of LMAC0 group=0x%03x/0x%03x\n",
		max_q, min_q);
	rsv_pg = pg_flow_ctrl[9] & 0xfff;
	used_pg = (pg_flow_ctrl[9] & (0xfff << 16)) >> 16;
	DBGLOG(HAL, INFO,
		"\t\tThe used/reserved pages of LMAC0 group=0x%03x/0x%03x\n",
		used_pg, rsv_pg);
	DBGLOG(HAL, INFO,
		"\tReserved page counter of LMAC1 group(0x82068178): 0x%08x\n",
		pg_flow_ctrl[10]);
	DBGLOG(HAL, INFO,
		"\tLMAC1 group page status(0x8206817c): 0x%08x\n",
		pg_flow_ctrl[11]);
	min_q = pg_flow_ctrl[10] & 0xfff;
	max_q = (pg_flow_ctrl[10] & (0xfff << 16)) >> 16;
	DBGLOG(HAL, INFO,
		"\t\tThe max/min quota pages of LMAC1 group=0x%03x/0x%03x\n",
		max_q, min_q);
	rsv_pg = pg_flow_ctrl[11] & 0xfff;
	used_pg = (pg_flow_ctrl[11] & (0xfff << 16)) >> 16;
	DBGLOG(HAL, INFO,
		"\t\tThe used/reserved pages of LMAC1 group=0x%03x/0x%03x\n",
		used_pg, rsv_pg);
	DBGLOG(HAL, INFO,
		"\tReserved page counter of LMAC2 group(0x82068180): 0x%08x\n",
		pg_flow_ctrl[12]);
	DBGLOG(HAL, INFO,
		"\tLMAC2 group page status(0x82068184): 0x%08x\n",
		pg_flow_ctrl[13]);
	min_q = pg_flow_ctrl[12] & 0xfff;
	max_q = (pg_flow_ctrl[12] & (0xfff << 16)) >> 16;
	DBGLOG(HAL, INFO,
		"\t\tThe max/min quota pages of LMAC2 group=0x%03x/0x%03x\n",
		max_q, min_q);
	rsv_pg = pg_flow_ctrl[13] & 0xfff;
	used_pg = (pg_flow_ctrl[13] & (0xfff << 16)) >> 16;
	DBGLOG(HAL, INFO,
		"\t\tThe used/reserved pages of LMAC2 group=0x%03x/0x%03x\n",
		used_pg, rsv_pg);
	DBGLOG(HAL, INFO,
		"\tReserved page counter of PLE group(0x82068190): 0x%08x\n",
		pg_flow_ctrl[14]);
	DBGLOG(HAL, INFO,
		"\tPLE group page status(0x82068194): 0x%08x\n",
		pg_flow_ctrl[15]);
	min_q = pg_flow_ctrl[14] & 0xfff;
	max_q = (pg_flow_ctrl[14] & (0xfff << 16)) >> 16;
	DBGLOG(HAL, INFO,
		"\t\tThe max/min quota pages of PLE group=0x%03x/0x%03x\n",
		max_q, min_q);
	rsv_pg = pg_flow_ctrl[15] & 0xfff;
	used_pg = (pg_flow_ctrl[15] & (0xfff << 16)) >> 16;
	DBGLOG(HAL, INFO,
		"\t\tThe used/reserved pages of PLE group=0x%03x/0x%03x\n",
		used_pg, rsv_pg);
	/* Queue Empty Status */
	DBGLOG(HAL, INFO, "PSE Queue Empty Status:\n");
	DBGLOG(HAL, INFO,
		"\tQUEUE_EMPTY(0x820680b0): 0x%08x\n", pse_stat);
	DBGLOG(HAL, INFO,
		"\t\tCPU Q0/1/2/3 empty=%d/%d/%d/%d\n",
		 pse_stat & 0x1, ((pse_stat & 0x2) >> 1),
		 ((pse_stat & 0x4) >> 2), ((pse_stat & 0x8) >> 3));
	DBGLOG(HAL, INFO,
		"\t\tHIF Q0/1 empty=%d/%d\n",
		 ((pse_stat & (0x1 << 16)) >> 16),
		 ((pse_stat & (0x1 << 17)) >> 17));
	DBGLOG(HAL, INFO,
		"\t\tLMAC TX Q empty=%d\n",
		 ((pse_stat & (0x1 << 24)) >> 24));
	DBGLOG(HAL, INFO,
		"\t\tRLS_Q empty=%d\n",
		 ((pse_stat & (0x1 << 31)) >> 31));
	DBGLOG(HAL, INFO, "Nonempty Q info:\n");

	for (i = 0; i < 31; i++) {
		if (((pse_stat & (0x1 << i)) >> i) == 0) {
			uint32_t hfid, tfid, pktcnt, fl_que_ctrl[3] = {0};

			if (i < 4) {
				DBGLOG(HAL, INFO,
						 "\tCPU Q%d: ", i);
				fl_que_ctrl[0] |= (0x1 << 14);
				fl_que_ctrl[0] |= (i << 8);
			} else if (i == 16) {
				DBGLOG(HAL, INFO, "\tHIF Q0: ");
				fl_que_ctrl[0] |= (0x0 << 14);
				fl_que_ctrl[0] |= (0x0 << 8);
			} else if (i == 17) {
				DBGLOG(HAL, INFO, "\tHIF  Q1: ");
				fl_que_ctrl[0] |= (0x0 << 14);
				fl_que_ctrl[0] |= (0x1 << 8);
			} else if (i == 24) {
				DBGLOG(HAL, INFO, "\tLMAC TX Q: ");
				fl_que_ctrl[0] |= (0x2 << 14);
				fl_que_ctrl[0] |= (0x0 << 8);
			} else if (i == 31) {
				DBGLOG(HAL, INFO, "\tRLS Q: ");
				fl_que_ctrl[0] |= (0x3 << 14);
				fl_que_ctrl[0] |= (i << 8);
			} else
				continue;

			fl_que_ctrl[0] |= (0x1 << 31);
			HAL_MCR_WR(prAdapter, PSE_FL_QUE_CTRL_0,
				fl_que_ctrl[0]);
			HAL_MCR_RD(prAdapter, PSE_FL_QUE_CTRL_2,
				&fl_que_ctrl[1]);
			HAL_MCR_RD(prAdapter, PSE_FL_QUE_CTRL_3,
				&fl_que_ctrl[2]);
			hfid = fl_que_ctrl[1] & 0xfff;
			tfid = (fl_que_ctrl[1] & 0xfff << 16) >> 16;
			pktcnt = fl_que_ctrl[2] & 0xfff;
			DBGLOG(HAL, INFO,
				"tail/head fid = 0x%03x/0x%03x, pkt cnt = %x\n",
				tfid, hfid, pktcnt);
			halGetPsePayload(prAdapter, hfid, txd_payload_info);
		}
	}

	for (i = 0; i < PSE_PEEK_CR_NUM; i++) {
		addr = PSE_PEEK_CR_0 + PSE_PEEK_CR_OFFSET * i;
		HAL_MCR_RD(prAdapter, addr, &value);
		DBGLOG(HAL, INFO, "PSE_PEEK_CR_%u[0x%08x/0x%08x]\n",
		       i, addr, value);
	}

	for (i = 0; i < PSE_ENDEQ_NUM; i++) {
		addr = PSE_ENQ_0 + PSE_ENDEQ_OFFSET * i;
		HAL_MCR_RD(prAdapter, addr, &value);
		DBGLOG(HAL, INFO, "PSE_ENQ_%u[0x%08x/0x%08x]\n",
		       i, addr, value);
	}

	for (i = 0; i < PSE_ENDEQ_NUM; i++) {
		addr = PSE_DEQ_0 + PSE_ENDEQ_OFFSET * i;
		HAL_MCR_RD(prAdapter, addr, &value);
		DBGLOG(HAL, INFO, "PSE_DEQ_%u[0x%08x/0x%08x]\n",
		       i, addr, value);
	}
}

#define UMAC_FID_FAULT	0xFFF
static int8_t *sta_ctrl_reg[] = {"ENABLE", "*DISABLE", "*PAUSE"};
static struct EMPTY_QUEUE_INFO Queue_Empty_info[] = {
	{"CPU Q0",  ENUM_UMAC_CPU_PORT_1,     ENUM_UMAC_CTX_Q_0},
	{"CPU Q1",  ENUM_UMAC_CPU_PORT_1,     ENUM_UMAC_CTX_Q_1},
	{"CPU Q2",  ENUM_UMAC_CPU_PORT_1,     ENUM_UMAC_CTX_Q_2},
	{"CPU Q3",  ENUM_UMAC_CPU_PORT_1,     ENUM_UMAC_CTX_Q_3},
	{NULL, 0, 0}, {NULL, 0, 0}, {NULL, 0, 0}, {NULL, 0, 0},
	{"ALTX Q0", ENUM_UMAC_LMAC_PORT_2,    ENUM_UMAC_LMAC_PLE_TX_Q_ALTX_0},
	{"BMC Q0",  ENUM_UMAC_LMAC_PORT_2,    ENUM_UMAC_LMAC_PLE_TX_Q_BMC_0},
	{"BCN Q0",  ENUM_UMAC_LMAC_PORT_2,    ENUM_UMAC_LMAC_PLE_TX_Q_BNC_0},
	{"PSMP Q0", ENUM_UMAC_LMAC_PORT_2,    ENUM_UMAC_LMAC_PLE_TX_Q_PSMP_0},
	{"ALTX Q1", ENUM_UMAC_LMAC_PORT_2,    ENUM_UMAC_LMAC_PLE_TX_Q_ALTX_1},
	{"BMC Q1",  ENUM_UMAC_LMAC_PORT_2,    ENUM_UMAC_LMAC_PLE_TX_Q_BMC_1},
	{"BCN Q1",  ENUM_UMAC_LMAC_PORT_2,    ENUM_UMAC_LMAC_PLE_TX_Q_BNC_1},
	{"PSMP Q1", ENUM_UMAC_LMAC_PORT_2,    ENUM_UMAC_LMAC_PLE_TX_Q_PSMP_1},
	{"NAF Q",   ENUM_UMAC_LMAC_PORT_2,    ENUM_UMAC_LMAC_PLE_TX_Q_NAF},
	{"NBCN Q",  ENUM_UMAC_LMAC_PORT_2,    ENUM_UMAC_LMAC_PLE_TX_Q_NBCN},
	{NULL, 0, 0}, {NULL, 0, 0}, {NULL, 0, 0},
	{NULL, 0, 0}, {NULL, 0, 0}, {NULL, 0, 0},
	{NULL, 0, 0}, {NULL, 0, 0}, {NULL, 0, 0},
	{NULL, 0, 0}, {NULL, 0, 0}, {NULL, 0, 0},
	{"RLS Q",   ENUM_PLE_CTRL_PSE_PORT_3, ENUM_UMAC_PLE_CTRL_P3_Q_0X1E},
	{"RLS2 Q",  ENUM_PLE_CTRL_PSE_PORT_3, ENUM_UMAC_PLE_CTRL_P3_Q_0X1F}
};

void halShowPleInfo(IN struct ADAPTER *prAdapter,
	u_int8_t fgDumpTxd)
{
	uint32_t ple_buf_ctrl[3] = {0}, pg_sz, pg_num, bit_field_1, bit_field_2;
	uint32_t ple_stat[17] = {0}, pg_flow_ctrl[6] = {0};
	uint32_t sta_pause[4] = {0}, dis_sta_map[4] = {0};
	uint32_t fpg_cnt, ffa_cnt, fpg_head, fpg_tail, hif_max_q, hif_min_q;
	uint32_t rpg_hif, upg_hif, cpu_max_q, cpu_min_q, rpg_cpu, upg_cpu;
	uint32_t i, j, addr, value = 0;
	uint32_t txd_info[16] = {0};

	HAL_MCR_RD(prAdapter, PLE_PBUF_CTRL, &ple_buf_ctrl[0]);
	HAL_MCR_RD(prAdapter, PLE_RELEASE_CTRL, &ple_buf_ctrl[1]);
	HAL_MCR_RD(prAdapter, PLE_HIF_REPORT, &ple_buf_ctrl[2]);
	HAL_MCR_RD(prAdapter, PLE_QUEUE_EMPTY, &ple_stat[0]);
	HAL_MCR_RD(prAdapter, PLE_AC0_QUEUE_EMPTY_0, &ple_stat[1]);
	HAL_MCR_RD(prAdapter, PLE_AC0_QUEUE_EMPTY_1, &ple_stat[2]);
	HAL_MCR_RD(prAdapter, PLE_AC0_QUEUE_EMPTY_2, &ple_stat[3]);
	HAL_MCR_RD(prAdapter, PLE_AC0_QUEUE_EMPTY_3, &ple_stat[4]);
	HAL_MCR_RD(prAdapter, PLE_AC1_QUEUE_EMPTY_0, &ple_stat[5]);
	HAL_MCR_RD(prAdapter, PLE_AC1_QUEUE_EMPTY_1, &ple_stat[6]);
	HAL_MCR_RD(prAdapter, PLE_AC1_QUEUE_EMPTY_2, &ple_stat[7]);
	HAL_MCR_RD(prAdapter, PLE_AC1_QUEUE_EMPTY_3, &ple_stat[8]);
	HAL_MCR_RD(prAdapter, PLE_AC2_QUEUE_EMPTY_0, &ple_stat[9]);
	HAL_MCR_RD(prAdapter, PLE_AC2_QUEUE_EMPTY_1, &ple_stat[10]);
	HAL_MCR_RD(prAdapter, PLE_AC2_QUEUE_EMPTY_2, &ple_stat[11]);
	HAL_MCR_RD(prAdapter, PLE_AC2_QUEUE_EMPTY_3, &ple_stat[12]);
	HAL_MCR_RD(prAdapter, PLE_AC3_QUEUE_EMPTY_0, &ple_stat[13]);
	HAL_MCR_RD(prAdapter, PLE_AC3_QUEUE_EMPTY_1, &ple_stat[14]);
	HAL_MCR_RD(prAdapter, PLE_AC3_QUEUE_EMPTY_2, &ple_stat[15]);
	HAL_MCR_RD(prAdapter, PLE_AC3_QUEUE_EMPTY_3, &ple_stat[16]);
	HAL_MCR_RD(prAdapter, PLE_FREEPG_CNT, &pg_flow_ctrl[0]);
	HAL_MCR_RD(prAdapter, PLE_FREEPG_HEAD_TAIL, &pg_flow_ctrl[1]);
	HAL_MCR_RD(prAdapter, PLE_PG_HIF_GROUP, &pg_flow_ctrl[2]);
	HAL_MCR_RD(prAdapter, PLE_HIF_PG_INFO, &pg_flow_ctrl[3]);
	HAL_MCR_RD(prAdapter, PLE_PG_CPU_GROUP, &pg_flow_ctrl[4]);
	HAL_MCR_RD(prAdapter, PLE_CPU_PG_INFO, &pg_flow_ctrl[5]);
	HAL_MCR_RD(prAdapter, DIS_STA_MAP0, &dis_sta_map[0]);
	HAL_MCR_RD(prAdapter, DIS_STA_MAP1, &dis_sta_map[1]);
	HAL_MCR_RD(prAdapter, DIS_STA_MAP2, &dis_sta_map[2]);
	HAL_MCR_RD(prAdapter, DIS_STA_MAP3, &dis_sta_map[3]);
	HAL_MCR_RD(prAdapter, STATION_PAUSE0, &sta_pause[0]);
	HAL_MCR_RD(prAdapter, STATION_PAUSE1, &sta_pause[1]);
	HAL_MCR_RD(prAdapter, STATION_PAUSE2, &sta_pause[2]);
	HAL_MCR_RD(prAdapter, STATION_PAUSE3, &sta_pause[3]);
	/* Configuration Info */
	DBGLOG(HAL, INFO, "PLE Configuration Info:\n");

	HAL_MCR_RD(prAdapter, PLE_GC, &value);
	DBGLOG(HAL, INFO, "\tGC(0x82060000): 0x%08x\n", value);
	HAL_MCR_RD(prAdapter, PLE_INT_STS, &value);
	DBGLOG(HAL, INFO, "\tINT_STS(0x82060024): 0x%08x\n", value);
	HAL_MCR_RD(prAdapter, PLE_INT_ERR_STS, &value);
	DBGLOG(HAL, INFO, "\tINT_ERR_STS(0x82060028): 0x%08x\n", value);
	HAL_MCR_RD(prAdapter, PLE_QUEUE_CMD_ERR_STS, &value);
	DBGLOG(HAL, INFO, "\tQUEUE_CMD_ERR_STS(0x82060550): 0x%08x\n", value);

	DBGLOG(HAL, INFO,
		"\tPacket Buffer Control(0x82060014): 0x%08x\n",
		ple_buf_ctrl[0]);
	pg_sz = (ple_buf_ctrl[0] & (0x1 << 31)) >> 31;
	DBGLOG(HAL, INFO,
		"\t\tPage Size=%d(%d bytes per page)\n",
		pg_sz, (pg_sz == 1 ? 128 : 64));
	DBGLOG(HAL, INFO,
		"\t\tPage Offset=%d(in unit of 2KB)\n",
		(ple_buf_ctrl[0] & (0xf << 17)) >> 17);
	pg_num = (ple_buf_ctrl[0] & 0xfff);
	DBGLOG(HAL, INFO,
		"\t\tTotal Page=%d pages\n", (ple_buf_ctrl[0] & 0xfff));
	DBGLOG(HAL, INFO,
		"\tRelease Control(0x82060030): 0x%08x\n", ple_buf_ctrl[1]);
	bit_field_1 = (ple_buf_ctrl[1] & 0x1f);
	bit_field_2 = ((ple_buf_ctrl[1] & (0x3 << 6)) >> 6);
	DBGLOG(HAL, INFO,
		"\t\tNormalTx Release Pid/Qid=%d/%d\n",
		bit_field_2, bit_field_1);
	bit_field_1 = ((ple_buf_ctrl[1] & (0x1f << 8)) >> 8);
	bit_field_2 = ((ple_buf_ctrl[1] & (0x3 << 14)) >> 14);
	DBGLOG(HAL, INFO,
		"\t\tDropTx Release Pid/Qid=%d/%d\n", bit_field_2, bit_field_1);
	bit_field_1 = ((ple_buf_ctrl[1] & (0x1f << 16)) >> 16);
	bit_field_2 = ((ple_buf_ctrl[1] & (0x3 << 22)) >> 22);
	DBGLOG(HAL, INFO,
		"\t\tBCN0 Release Pid/Qid=%d/%d\n", bit_field_2, bit_field_1);
	bit_field_1 = ((ple_buf_ctrl[1] & (0x1f << 24)) >> 24);
	bit_field_2 = ((ple_buf_ctrl[1] & (0x3 << 30)) >> 30);
	DBGLOG(HAL, INFO,
		"\t\tBCN1 Release Pid/Qid=%d/%d\n", bit_field_2, bit_field_1);
	DBGLOG(HAL, INFO,
		"\tHIF Report Control(0x82060034): 0x%08x\n", ple_buf_ctrl[2]);
	bit_field_1 = ((ple_buf_ctrl[2] & (0x1 << 1)) >> 1);
	DBGLOG(HAL, INFO,
		"\t\tHostReportQSel/HostReportDisable=%d/%d\n",
			  (ple_buf_ctrl[2] & 0x1), bit_field_1);
	/* Page Flow Control */
	DBGLOG(HAL, INFO, "PLE Page Flow Control:\n");
	DBGLOG(HAL, INFO,
		"\tFree page counter(0x82060100): 0x%08x\n", pg_flow_ctrl[0]);
	fpg_cnt = pg_flow_ctrl[0] & 0xfff;
	DBGLOG(HAL, INFO,
		"\t\tThe toal page number of free=0x%03x\n", fpg_cnt);
	ffa_cnt = (pg_flow_ctrl[0] & (0xfff << 16)) >> 16;
	DBGLOG(HAL, INFO,
		"\t\tThe free page numbers of free for all=0x%03x\n", ffa_cnt);
	DBGLOG(HAL, INFO,
		"\tFree page head and tail(0x82060104): 0x%08x\n",
		pg_flow_ctrl[1]);
	fpg_head = pg_flow_ctrl[1] & 0xfff;
	fpg_tail = (pg_flow_ctrl[1] & (0xfff << 16)) >> 16;
	DBGLOG(HAL, INFO,
		"\t\tThe tail/head page of free page list=0x%03x/0x%03x\n",
		fpg_tail, fpg_head);
	DBGLOG(HAL, INFO,
		"\tReserved page counter of HIF group(0x82060110): 0x%08x\n",
		pg_flow_ctrl[2]);
	DBGLOG(HAL, INFO,
		"\tHIF group page status(0x82060114): 0x%08x\n",
		pg_flow_ctrl[3]);
	hif_min_q = pg_flow_ctrl[2] & 0xfff;
	hif_max_q = (pg_flow_ctrl[2] & (0xfff << 16)) >> 16;
	DBGLOG(HAL, INFO,
		"\t\tThe max/min quota pages of HIF group=0x%03x/0x%03x\n",
		hif_max_q, hif_min_q);
	rpg_hif = pg_flow_ctrl[3] & 0xfff;
	upg_hif = (pg_flow_ctrl[3] & (0xfff << 16)) >> 16;
	DBGLOG(HAL, INFO,
		"\t\tThe used/reserved pages of HIF group=0x%03x/0x%03x\n",
		upg_hif, rpg_hif);
	DBGLOG(HAL, INFO,
		"\tReserved page counter of CPU group(0x82060150): 0x%08x\n",
		pg_flow_ctrl[4]);
	DBGLOG(HAL, INFO,
		"\tCPU group page status(0x82060154): 0x%08x\n",
		pg_flow_ctrl[5]);
	cpu_min_q = pg_flow_ctrl[4] & 0xfff;
	cpu_max_q = (pg_flow_ctrl[4] & (0xfff << 16)) >> 16;
	DBGLOG(HAL, INFO,
		"\t\tThe max/min quota pages of CPU group=0x%03x/0x%03x\n",
		cpu_max_q, cpu_min_q);
	rpg_cpu = pg_flow_ctrl[5] & 0xfff;
	upg_cpu = (pg_flow_ctrl[5] & (0xfff << 16)) >> 16;
	DBGLOG(HAL, INFO,
		"\t\tThe used/reserved pages of CPU group=0x%03x/0x%03x\n",
		upg_cpu, rpg_cpu);

	if (((ple_stat[0] & (0x1 << 24)) >> 24) == 0) {
		DBGLOG(HAL, INFO,
			"\tAC0_QUEUE_EMPTY0(0x82060300): 0x%08x\n",
			ple_stat[1]);
		DBGLOG(HAL, INFO,
			"\tAC1_QUEUE_EMPTY0(0x82060310): 0x%08x\n",
			ple_stat[5]);
		DBGLOG(HAL, INFO,
			"\tAC2_QUEUE_EMPTY0(0x82060320): 0x%08x\n",
			ple_stat[9]);
		DBGLOG(HAL, INFO,
			"\tAC3_QUEUE_EMPTY0(0x82060330): 0x%08x\n",
			ple_stat[13]);

		for (j = 0; j < 16; j = j + 4) {
			if (j % 4 == 0)
				DBGLOG(HAL, INFO, "\tNonempty AC%d Q of STA#:",
					j / 4);

			for (i = 0; i < 32; i++) {
				if (((ple_stat[j + 1] & (0x1 << i)) >> i) == 0)
					DBGLOG(HAL, INFO, "%d ",
						i + (j % 4) * 32);
			}
		}

		DBGLOG(HAL, INFO, "\n");
	}

	DBGLOG(HAL, INFO, "Nonempty Q info:\n");

	for (i = 0; i < 31; i++) {
		if (((ple_stat[0] & (0x1 << i)) >> i) == 0) {
			uint32_t hfid, tfid, pktcnt, fl_que_ctrl[3] = {0};

			if (Queue_Empty_info[i].QueueName != NULL) {
				DBGLOG(HAL, INFO, "\t%s: ",
					Queue_Empty_info[i].QueueName);
				fl_que_ctrl[0] |= (0x1 << 31);
				fl_que_ctrl[0] |=
					(Queue_Empty_info[i].Portid << 14);
				fl_que_ctrl[0] |=
					(Queue_Empty_info[i].Queueid << 8);
			} else
				continue;

			HAL_MCR_WR(prAdapter,
				PLE_FL_QUE_CTRL_0, fl_que_ctrl[0]);
			HAL_MCR_RD(prAdapter,
				PLE_FL_QUE_CTRL_2, &fl_que_ctrl[1]);
			HAL_MCR_RD(prAdapter,
				PLE_FL_QUE_CTRL_3, &fl_que_ctrl[2]);
			hfid = fl_que_ctrl[1] & 0xfff;
			tfid = (fl_que_ctrl[1] & 0xfff << 16) >> 16;
			pktcnt = fl_que_ctrl[2] & 0xfff;
			DBGLOG(HAL, INFO,
				"tail/head fid = 0x%03x/0x%03x, pkt cnt = %x\n",
				 tfid, hfid, pktcnt);
		}
	}

	for (j = 0; j < 16; j = j + 4) { /* show AC Q info */
		for (i = 0; i < 32; i++) {
			if (((ple_stat[j + 1] & (0x1 << i)) >> i) == 0) {
				uint32_t hfid, tfid, pktcnt,
					ac_num = j / 4, ctrl = 0;
				uint32_t sta_num = i + (j % 4) * 32,
					fl_que_ctrl[3] = {0};

				DBGLOG(HAL, INFO, "\tSTA%d AC%d: ",
					sta_num, ac_num);
				fl_que_ctrl[0] |= (0x1 << 31);
				fl_que_ctrl[0] |= (0x2 << 14);
				fl_que_ctrl[0] |= (ac_num << 8);
				fl_que_ctrl[0] |= sta_num;
				HAL_MCR_WR(prAdapter,
					PLE_FL_QUE_CTRL_0, fl_que_ctrl[0]);
				HAL_MCR_RD(prAdapter,
					PLE_FL_QUE_CTRL_2, &fl_que_ctrl[1]);
				HAL_MCR_RD(prAdapter,
					PLE_FL_QUE_CTRL_3, &fl_que_ctrl[2]);
				hfid = fl_que_ctrl[1] & 0xfff;
				tfid = (fl_que_ctrl[1] & 0xfff << 16) >> 16;
				pktcnt = fl_que_ctrl[2] & 0xfff;
				DBGLOG(HAL, INFO,
				"tail/head fid = 0x%03x/0x%03x, pkt cnt = %x",
				tfid, hfid, pktcnt);
				if (fgDumpTxd) {
					halGetPleTxdInfo(prAdapter,
						hfid, txd_info);
					halDumpTxdInfo(prAdapter, txd_info);
				}
				if (((sta_pause[j % 4] & 0x1 << i) >> i) == 1)
					ctrl = 2;

				if (((dis_sta_map[j % 4] & 0x1 << i) >> i) == 1)
					ctrl = 1;

				DBGLOG(HAL, INFO, " ctrl = %s",
					sta_ctrl_reg[ctrl]);
			}
		}
	}

	for (i = 0; i < PLE_PEEK_CR_NUM; i++) {
		addr = PLE_PEEK_CR_0 + PLE_PEEK_CR_OFFSET * i;
		HAL_MCR_RD(prAdapter, addr, &value);
		DBGLOG(HAL, INFO, "PLE_PEEK_CR_%u[0x%08x/0x%08x]\n",
		       i, addr, value);
	}
}

void halShowDmaschInfo(IN struct ADAPTER *prAdapter)
{
	struct mt66xx_chip_info *prChipInfo;
	uint32_t value = 0;
	uint32_t ple_pkt_max_sz;
	uint32_t pse_pkt_max_sz;
	uint32_t max_quota;
	uint32_t min_quota;
	uint32_t rsv_cnt;
	uint32_t src_cnt;
	uint32_t pse_rsv_cnt = 0;
	uint32_t pse_src_cnt = 0;
	uint32_t odd_group_pktin_cnt = 0;
	uint32_t odd_group_ask_cnt = 0;
	uint32_t pktin_cnt;
	uint32_t ask_cnt;
	uint32_t total_src_cnt = 0;
	uint32_t total_rsv_cnt = 0;
	uint32_t ffa_cnt;
	uint32_t free_pg_cnt;
	uint32_t Group_Mapping_Q[16] = {0};
	uint32_t qmapping_addr = MT_HIF_DMASHDL_Q_MAP0;
	uint32_t status_addr = MT_HIF_DMASHDL_STATUS_RD_GP0;
	uint32_t quota_addr = MT_HIF_DMASHDL_GROUP0_CTRL;
	uint32_t pkt_cnt_addr = MT_HIF_DMASHDLRD_GP_PKT_CNT_0;
	uint32_t mapping_mask = 0xf;
	uint32_t mapping_offset = 0;
	uint32_t mapping_qidx;
	uint32_t groupidx = 0;
	uint8_t idx = 0;
	u_int8_t pktin_int_refill_ena;
	u_int8_t pdma_add_int_refill_ena;
	u_int8_t ple_add_int_refill_ena;
	u_int8_t ple_sub_ena;
	u_int8_t hif_ask_sub_ena;
	u_int8_t ple_txd_gt_max_size_flag_clr;
	uint32_t ple_rpg_hif;
	uint32_t ple_upg_hif;
	uint32_t pse_rpg_hif = 0;
	uint32_t pse_upg_hif = 0;
	uint8_t is_mismatch = FALSE;
	uint32_t u4BaseAddr;

	prChipInfo = prAdapter->chip_info;
	u4BaseAddr = prChipInfo->u4HifDmaShdlBaseAddr;
	for (mapping_qidx = 0; mapping_qidx < 32; mapping_qidx++) {
		uint32_t mapping_group;

		idx = 0;

		if (mapping_qidx == 0) {
			qmapping_addr = MT_HIF_DMASHDL_Q_MAP0;
			mapping_mask = 0xf;
			mapping_offset = 0;
		} else if ((mapping_qidx % 8) == 0) {
			qmapping_addr += 0x4;
			mapping_mask = 0xf;
			mapping_offset = 0;
		} else {
			mapping_offset += 4;
			mapping_mask = 0xf << mapping_offset;
		}

		HAL_MCR_RD(prAdapter, qmapping_addr, &value);
		mapping_group = (value & mapping_mask) >> mapping_offset;
		Group_Mapping_Q[mapping_group] |= 1 << mapping_qidx;
	}

	DBGLOG(HAL, INFO, "Dma scheduler info:\n");
	HAL_MCR_RD(prAdapter, MT_HIF_DMASHDL_CTRL_SIGNAL, &value);
	pktin_int_refill_ena =
		(value & DMASHDL_PKTIN_INT_REFILL_ENA) ? TRUE : FALSE;
	pdma_add_int_refill_ena =
		(value & DMASHDL_PDMA_ADD_INT_REFILL_ENA) ? TRUE : FALSE;
	ple_add_int_refill_ena =
		(value & DMASHDL_PLE_ADD_INT_REFILL_ENA) ? TRUE : FALSE;
	ple_sub_ena = (value & DMASHDL_PLE_SUB_ENA) ? TRUE : FALSE;
	hif_ask_sub_ena = (value & DMASHDL_HIF_ASK_SUB_ENA) ? TRUE : FALSE;
	ple_txd_gt_max_size_flag_clr =
		(value & DMASHDL_PLE_TXD_GT_MAX_SIZE_FLAG_CLR) ? TRUE : FALSE;
	DBGLOG(HAL, INFO, "DMASHDL Ctrl Signal(0x5000A018): 0x%08x\n", value);
	DBGLOG(HAL, INFO, "\tple_txd_gt_max_size_flag_clr(BIT0) = %d\n",
		ple_txd_gt_max_size_flag_clr);
	DBGLOG(HAL, INFO, "\thif_ask_sub_ena(BIT16) = %d\n", hif_ask_sub_ena);
	DBGLOG(HAL, INFO, "\tple_sub_ena(BIT17) = %d\n", ple_sub_ena);
	DBGLOG(HAL, INFO, "\tple_add_int_refill_ena(BIT29) = %d\n",
		ple_add_int_refill_ena);
	DBGLOG(HAL, INFO, "\tpdma_add_int_refill_ena(BIT30) = %d\n",
		pdma_add_int_refill_ena);
	DBGLOG(HAL, INFO, "\tpktin_int_refill(BIT31)_ena = %d\n",
		pktin_int_refill_ena);
	HAL_MCR_RD(prAdapter, MT_HIF_DMASHDL_PKT_MAX_SIZE, &value);
	ple_pkt_max_sz = GET_PLE_PKT_MAX_SIZE_NUM(value);
	pse_pkt_max_sz = GET_PSE_PKT_MAX_SIZE_NUM(value);
	DBGLOG(HAL, INFO,
		"DMASHDL Packet_max_size(0x5000A01c): 0x%08x\n", value);
	DBGLOG(HAL, INFO,
		"PLE/PSE packet max size=0x%03x/0x%03x\n",
		 ple_pkt_max_sz, pse_pkt_max_sz);
	HAL_MCR_RD(prAdapter, MT_HIF_DMASHDL_ERROR_FLAG_CTRL, &value);
	DBGLOG(HAL, INFO, "DMASHDL ERR FLAG CTRL(0x5000A09c): 0x%08x\n", value);
	HAL_MCR_RD(prAdapter, MT_HIF_DMASHDL_STATUS_RD, &value);
	ffa_cnt = (value & DMASHDL_FFA_CNT_MASK) >> DMASHDL_FFA_CNT_OFFSET;
	free_pg_cnt = (value & DMASHDL_FREE_PG_CNT_MASK) >>
		DMASHDL_FREE_PG_CNT_OFFSET;
	DBGLOG(HAL, INFO, "DMASHDL Status_RD(0x5000A100): 0x%08x\n", value);
	DBGLOG(HAL, INFO,
		"free page cnt = 0x%03x, ffa cnt = 0x%03x\n",
		free_pg_cnt, ffa_cnt);
	HAL_MCR_RD(prAdapter,
		CONN_HIF_DMASHDL_REFILL_CONTROL(u4BaseAddr), &value);
	DBGLOG(HAL, INFO,
		"DMASHDL ReFill Control(0x5000A010): 0x%08x\n", value);

	for (groupidx = 0; groupidx < 16; groupidx++) {
		DBGLOG(HAL, INFO, "Group %d info:", groupidx);
		HAL_MCR_RD(prAdapter, status_addr, &value);
		rsv_cnt = (value & DMASHDL_RSV_CNT_MASK) >>
			DMASHDL_RSV_CNT_OFFSET;
		src_cnt = (value & DMASHDL_SRC_CNT_MASK) >>
			DMASHDL_SRC_CNT_OFFSET;
		DBGLOG(HAL, INFO,
			"\tDMASHDL Status_RD_GP%d(0x%08x): 0x%08x\n",
			groupidx, status_addr, value);
		HAL_MCR_RD(prAdapter, quota_addr, &value);
		max_quota = (value & DMASHDL_MAX_QUOTA_MASK) >>
			DMASHDL_MAX_QUOTA_OFFSET;
		min_quota = (value & DMASHDL_MIN_QUOTA_MASK) >>
			DMASHDL_MIN_QUOTA_OFFSET;
		DBGLOG(HAL, INFO,
			"\tDMASHDL Group%d control(0x%08x): 0x%08x\n",
			groupidx, quota_addr, value);

		if ((groupidx & 0x1) == 0) {
			HAL_MCR_RD(prAdapter, pkt_cnt_addr, &value);
			DBGLOG(HAL, INFO,
			"\tDMASHDL RD_group_pkt_cnt_%d(0x%08x): 0x%08x\n",
			groupidx / 2, pkt_cnt_addr, value);
			odd_group_pktin_cnt = GET_ODD_GROUP_PKT_IN_CNT(value);
			odd_group_ask_cnt = GET_ODD_GROUP_ASK_CNT(value);
			pktin_cnt = GET_EVEN_GROUP_PKT_IN_CNT(value);
			ask_cnt = GET_EVEN_GROUP_ASK_CNT(value);
		} else {
			pktin_cnt = odd_group_pktin_cnt;
			ask_cnt = odd_group_ask_cnt;
		}

		DBGLOG(HAL, INFO,
			"\trsv_cnt = 0x%03x, src_cnt = 0x%03x\n",
			rsv_cnt, src_cnt);
		DBGLOG(HAL, INFO,
			"\tmax/min quota = 0x%03x/ 0x%03x\n",
			max_quota, min_quota);
		DBGLOG(HAL, INFO,
			"\tpktin_cnt = 0x%02x, ask_cnt = 0x%02x",
			pktin_cnt, ask_cnt);

		if (hif_ask_sub_ena && pktin_cnt != ask_cnt) {
			DBGLOG(HAL, INFO, ", mismatch!");
			is_mismatch = TRUE;
		}

		/* Group15 is for PSE */
		if (groupidx == 15 && Group_Mapping_Q[groupidx] == 0) {
			pse_src_cnt = src_cnt;
			pse_rsv_cnt = rsv_cnt;
			break;
		}

		DBGLOG(HAL, INFO, "\tMapping Qidx: 0x%x",
		       Group_Mapping_Q[groupidx]);

		total_src_cnt += src_cnt;
		total_rsv_cnt += rsv_cnt;
		status_addr = status_addr + 4;
		quota_addr = quota_addr + 4;

		if (groupidx & 0x1)
			pkt_cnt_addr = pkt_cnt_addr + 4;
	}

	DBGLOG(HAL, INFO, "\nCounter Check:\n");
	HAL_MCR_RD(prAdapter, PLE_HIF_PG_INFO, &value);
	ple_rpg_hif = value & 0xfff;
	ple_upg_hif = (value & (0xfff << 16)) >> 16;
	DBGLOG(HAL, INFO,
	"PLE:\n\tThe used/reserved pages of PLE HIF group=0x%03x/0x%03x\n",
	ple_upg_hif, ple_rpg_hif);
	HAL_MCR_RD(prAdapter, PSE_HIF1_PG_INFO, &value);
	pse_rpg_hif = value & 0xfff;
	pse_upg_hif = (value & (0xfff << 16)) >> 16;
	DBGLOG(HAL, INFO,
	"PSE:\n\tThe used/reserved pages of PSE HIF group=0x%03x/0x%03x\n",
	pse_upg_hif, pse_rpg_hif);
	DBGLOG(HAL, INFO,
		"DMASHDL:\n\tThe total used pages of group0~14=0x%03x",
		 total_src_cnt);

	if (ple_upg_hif != total_src_cnt) {
		DBGLOG(HAL, INFO, ", mismatch!");
		is_mismatch = TRUE;
	}

	DBGLOG(HAL, INFO, "\n");
	DBGLOG(HAL, INFO,
		"\tThe total reserved pages of group0~14=0x%03x\n",
		total_rsv_cnt);
	DBGLOG(HAL, INFO,
		"\tThe total ffa pages of group0~14=0x%03x\n",
		ffa_cnt);
	DBGLOG(HAL, INFO,
		"\tThe total free pages of group0~14=0x%03x", free_pg_cnt);

	if (free_pg_cnt != total_rsv_cnt + ffa_cnt) {
		DBGLOG(HAL, INFO,
			", mismatch(total_rsv_cnt + ffa_cnt in DMASHDL)");
		is_mismatch = TRUE;
	}

	if (free_pg_cnt != ple_rpg_hif) {
		DBGLOG(HAL, INFO, ", mismatch(reserved pages in PLE)");
		is_mismatch = TRUE;
	}

	DBGLOG(HAL, INFO, "\n");
	DBGLOG(HAL, INFO, "\tThe used pages of group15=0x%03x", pse_src_cnt);

	if (pse_upg_hif != pse_src_cnt) {
		DBGLOG(HAL, INFO, ", mismatch!");
		is_mismatch = TRUE;
	}

	DBGLOG(HAL, INFO, "\n");
	DBGLOG(HAL, INFO,
		"\tThe reserved pages of group15=0x%03x", pse_rsv_cnt);

	if (pse_rpg_hif != pse_rsv_cnt) {
		DBGLOG(HAL, INFO, ", mismatch!");
		is_mismatch = TRUE;
	}

	DBGLOG(HAL, INFO, "\n");

	if (!is_mismatch)
		DBGLOG(HAL, INFO, "DMASHDL: no counter mismatch\n");
}

void haldumpMacInfo(struct ADAPTER *prAdapter)
{
	uint32_t i = 0, j = 0;
	uint32_t value = 0, index = 0, flag = 0, queue = 0;

	DBGLOG(HAL, INFO, "Print 0x820F3190 5*20 times\n");
	for (i = 0; i < 5; i++) {
		for (j = 0; j < 20; j++) {
			HAL_MCR_RD(prAdapter, 0x820F3190, &value);
			DBGLOG(HAL, INFO, "0x820F3190: 0x%08x\n", value);
		}
		kalMdelay(1);
	}

	for (j = 0; j < 20; j++) {
		HAL_MCR_RD(prAdapter, 0x820FD020, &value);
		DBGLOG(HAL, INFO, "slot idle: 0x820FD020: 0x%08x\n", value);
		HAL_MCR_RD(prAdapter, 0x820F4128, &value);
		DBGLOG(HAL, INFO,
		       "TX state machine/CCA: 0x820F4128 = 0x%08x\n", value);
		HAL_MCR_RD(prAdapter, 0x820F20D0, &value);
		DBGLOG(HAL, INFO,
		       "AGG state machine band0: 0x820F20D0 = 0x%08x\n", value);
		HAL_MCR_RD(prAdapter, 0x820F20D4, &value);
		DBGLOG(HAL, INFO,
		       "AGG state machine band1: 0x820F20D4 = 0x%08x\n", value);
		/* 1: empty, 0: non-empty */
		HAL_MCR_RD(prAdapter, 0x82060220, &value);
		DBGLOG(HAL, INFO, "queue empty: 0x82060220: 0x%08x\n", value);
		HAL_MCR_RD(prAdapter, 0x820603EC, &value);
		DBGLOG(HAL, INFO,
			"PLE MACTX CurState: 0x820603EC: 0x%08x\n", value);
		kalMdelay(1);
	}

	HAL_MCR_RD(prAdapter, 0x820F4124, &value);
	DBGLOG(HAL, INFO, "TXV count: 0x820F4124 = %08x\n", value);

	/* Band 0 TXV1-TXV7 */
	for (j = 0x820F4130; j < 0x820F4148; j += 4) {
		HAL_MCR_RD(prAdapter, j, &value);
		DBGLOG(HAL, INFO, "0x%08x: 0x%08x\n", j, value);
		kalMdelay(1);
	}

	/* Band 1 TXV1-TXV7 */
	for (j = 0x820F414C; j < 0x820F4164; j += 4) {
		HAL_MCR_RD(prAdapter, j, &value);
		DBGLOG(HAL, INFO, "0x%08x: 0x%08x\n", j, value);
		kalMdelay(1);
	}

	HAL_MCR_RD(prAdapter, 0x820F409C, &value);
	DBGLOG(HAL, INFO, "Dump CR: 0x820F409C = %08x\n", value);

	HAL_MCR_RD(prAdapter, 0x820F409C, &value);
	DBGLOG(HAL, INFO, "Dump CR: 0x820F409C = %08x\n", value);

	HAL_MCR_RD(prAdapter, 0x820F3080, &value);
	DBGLOG(HAL, INFO, "Dump CR: 0x820F3080	= %08x\n", value);

	DBGLOG(HAL, INFO, "Dump ARB CR: 820F3000~820F33FF\n");
	for (index = 0x820f3000; index < 0x820f33ff; index += 4) {
		HAL_MCR_RD(prAdapter, index, &value);
		DBGLOG(HAL, INFO, "0x%08x: 0x%08x\n", index, value);
	}

	DBGLOG(HAL, INFO, "Dump AGG CR: 820F000~820F21FF\n");
	for (index = 0x820f2000; index < 0x820f21ff; index += 4) {
		HAL_MCR_RD(prAdapter, index, &value);
		DBGLOG(HAL, INFO, "0x%08x: 0x%08x\n", index, value);
	}

	DBGLOG(HAL, INFO, "Dump TRB\n");
	HAL_MCR_WR(prAdapter, 0x80025104, 0x02020202);
	flag = 0x01010000;
	for (i = 0; i < 64; i++) {
		HAL_MCR_WR(prAdapter, 0x80025104, flag);
		HAL_MCR_RD(prAdapter, 0x820f0024, &value);
		DBGLOG(HAL, INFO, "write flag = 0x%08x, 0x820f0024: 0x%08x\n",
		       flag, value);
		flag += 0x02020202;
	}

	DBGLOG(HAL, INFO, "Dump ARB\n");
	for (i = 0; i < 20; i++) {
		HAL_MCR_RD(prAdapter, 0x802f3190, &value);
		DBGLOG(HAL, INFO, "0x802f3190: 0x%08x\n", value);
	}

	HAL_MCR_WR(prAdapter, 0x820f082C, 0xf);
	HAL_MCR_WR(prAdapter, 0x80025100, 0x1f);
	HAL_MCR_WR(prAdapter, 0x80025104, 0x04040404);

	HAL_MCR_WR(prAdapter, 0x80025108, 0x41414040);
	HAL_MCR_RD(prAdapter, 0x820f0024, &value);
	DBGLOG(HAL, INFO, "0x820f0024: 0x%08x\n", value);
	HAL_MCR_RD(prAdapter, 0x820f20d0, &value);
	DBGLOG(HAL, INFO, "0x820f20d0: 0x%08x\n", value);
	HAL_MCR_RD(prAdapter, 0x820f20d4, &value);
	DBGLOG(HAL, INFO, "0x820f20d4: 0x%08x\n", value);

	queue = 0;
	flag = 0x00000101;
	for (i = 0; i < 25; i++) {
		HAL_MCR_WR(prAdapter, 0x820f3060, queue);
		flag = 0x00000101;
		for (j = 0; j < 8; j++) {
			HAL_MCR_WR(prAdapter, 0x80025108, flag);
			HAL_MCR_RD(prAdapter, 0x820f0024, &value);
			DBGLOG(HAL, INFO,
			       "write queue = 0x%08x flag = 0x%08x, 0x820f0024: 0x%08x\n",
			       queue, flag, value);
			flag += 0x02020202;
		}
		queue += 0x01010101;
	}

	queue = 0x01010000;
	flag = 0x04040505;
	HAL_MCR_WR(prAdapter, 0x820f3060, queue);
	for (i = 0; i < 3; i++) {
		HAL_MCR_WR(prAdapter, 0x80025104, flag);
		HAL_MCR_RD(prAdapter, 0x820f0024, &value);
		DBGLOG(HAL, INFO, "write flag = 0x%08x, 0x820f0024: 0x%08x\n",
		       flag, value);
		flag += 0x02020202;
	}

	flag = 0x00000101;
	HAL_MCR_WR(prAdapter, 0x820f3060, 0); /* BSSID = 0 */
	for (i = 0; i < 128; i++) {
		HAL_MCR_WR(prAdapter, 0x80025108, flag);
		HAL_MCR_RD(prAdapter, 0x820f0024, &value);
		DBGLOG(HAL, INFO, "write flag = 0x%08x, 0x820f0024: 0x%08x\n",
		       flag, value);
		flag += 0x02020202;
	}

	DBGLOG(HAL, INFO, "Dump AGG\n");
	HAL_MCR_WR(prAdapter, 0x80025104, 0x05050505);
	flag = 0x01010000;
	for (i = 0; i < 64; i++) {
		HAL_MCR_WR(prAdapter, 0x80025104, flag);
		HAL_MCR_RD(prAdapter, 0x820f0024, &value);
		DBGLOG(HAL, INFO, "write flag = 0x%08x, 0x820f0024: 0x%08x\n",
		       flag, value);
		flag += 0x02020202;
	}

	DBGLOG(HAL, INFO, "Dump DMA\n");
	HAL_MCR_WR(prAdapter, 0x80025104, 0x06060606);
	flag = 0x01010000;
	for (i = 0; i < 64; i++) {
		HAL_MCR_WR(prAdapter, 0x80025104, flag);
		HAL_MCR_RD(prAdapter, 0x820f0024, &value);
		DBGLOG(HAL, INFO, "write flag = 0x%08x, 0x820f0024: 0x%08x\n",
		       flag, value);
		flag += 0x02020202;
	}

	DBGLOG(HAL, INFO, "Dump TMAC\n");
	HAL_MCR_WR(prAdapter, 0x80025104, 0x07070707);
	flag = 0x01010000;
	for (i = 0; i < 33; i++) {
		HAL_MCR_WR(prAdapter, 0x80025104, flag);
		HAL_MCR_RD(prAdapter, 0x820f0024, &value);
		DBGLOG(HAL, INFO, "write flag = 0x%08x, 0x820f0024: 0x%08x\n",
		       flag, value);
		flag += 0x02020202;
	}
}

#if (CFG_SUPPORT_CONNAC3X == 0)
static char *q_idx_mcu_str[] = {"RQ0", "RQ1", "RQ2", "RQ3", "Invalid"};
static char *pkt_ft_str[] = {"cut_through", "store_forward",
	"cmd", "PDA_FW_Download"};
static char *hdr_fmt_str[] = {
	"Non-80211-Frame",
	"Command-Frame",
	"Normal-80211-Frame",
	"enhanced-80211-Frame",
};
static char *p_idx_str[] = {"LMAC", "MCU"};
static char *q_idx_lmac_str[] = {"WMM0_AC0", "WMM0_AC1", "WMM0_AC2", "WMM0_AC3",
	"WMM1_AC0", "WMM1_AC1", "WMM1_AC2", "WMM1_AC3",
	"WMM2_AC0", "WMM2_AC1", "WMM2_AC2", "WMM2_AC3",
	"WMM3_AC0", "WMM3_AC1", "WMM3_AC2", "WMM3_AC3",
	"Band0_ALTX", "Band0_BMC", "Band0_BNC", "Band0_PSMP",
	"Band1_ALTX", "Band1_BMC", "Band1_BNC", "Band1_PSMP",
	"Invalid"};
#endif

void halGetPsePayload(
	IN struct ADAPTER *prAdapter, uint32_t fid, uint32_t *result) {
	uint32_t u4Start_txd = 0;
	uint32_t u4High_txd = 0;
	uint32_t u4Remap_txd = 0;
	uint32_t u4Pse_offset = 0;
	uint32_t i = 0, value = 0;

	HAL_MCR_RD(prAdapter, 0x82060014, &value);
	u4Pse_offset = (value & 0x3FF0000) << 10;
	u4High_txd = (u4Pse_offset >> 16);
	HAL_MCR_WR(prAdapter, 0x0000700C, u4High_txd);
	u4Start_txd = 0x40000000 + u4Pse_offset + (fid << 7);
	u4Remap_txd = 0x000D0000 + (u4Start_txd & 0xFFFF);
	for (i = 0; i < 16; i++)
		HAL_MCR_RD(prAdapter, u4Remap_txd + (i * 4), &result[i]);
	DBGLOG(HAL, INFO, "Dump fid=%d PSE payload\n", fid);
	dumpMemory32(result, 64);
}

void halGetPleTxdInfo(
	IN struct ADAPTER *prAdapter, uint32_t fid, uint32_t *result) {
	uint32_t u4Start_txd = 0;
	uint32_t u4High_txd = 0;
	uint32_t u4Remap_txd = 0;
	uint32_t i = 0;

	u4Start_txd = 0x40000000 + (fid << 6);
	u4High_txd = (u4Start_txd >> 16);
	HAL_MCR_WR(prAdapter, 0x0000700C, u4High_txd);
	u4Remap_txd = 0x000D0000 + (fid << 6);
	for (i = 0; i < 16; i++) {
		HAL_MCR_RD(prAdapter,
			u4Remap_txd + (i * 4), &result[i]);
	}
	DBGLOG(HAL, INFO, "Dump fid=%d PLE TXD\n", fid);
	dumpMemory32(result, 64);
}

#if (CFG_SUPPORT_CONNAC3X == 0)
void halDumpTxdInfo(IN struct ADAPTER *prAdapter, uint32_t *tmac_info)
{
	struct TMAC_TXD_S *txd_s;
	struct TMAC_TXD_0 *txd_0;
	struct TMAC_TXD_1 *txd_1;
	uint8_t q_idx = 0;

	txd_s = (struct TMAC_TXD_S *)tmac_info;
	txd_0 = &txd_s->TxD0;
	txd_1 = &txd_s->TxD1;

	DBGLOG(HAL, INFO, "TMAC_TXD Fields:\n");
	DBGLOG(HAL, INFO, "\tTMAC_TXD_0:\n");
	DBGLOG(HAL, INFO, "\t\tPortID=%d(%s)\n",
			txd_0->p_idx, p_idx_str[txd_0->p_idx]);

	if (txd_0->p_idx == P_IDX_LMAC)
		q_idx = txd_0->q_idx % 0x18;
	else
		q_idx = ((txd_0->q_idx == TxQ_IDX_MCU_PDA) ?
			txd_0->q_idx : (txd_0->q_idx % 0x4));

	DBGLOG(HAL, INFO, "\t\tQueID=0x%x(%s %s)\n", txd_0->q_idx,
			 (txd_0->p_idx == P_IDX_LMAC ? "LMAC" : "MCU"),
			 txd_0->p_idx == P_IDX_LMAC ?
				q_idx_lmac_str[q_idx] : q_idx_mcu_str[q_idx]);
	DBGLOG(HAL, INFO, "\t\tTxByteCnt=%d\n", txd_0->TxByteCount);
	DBGLOG(HAL, INFO, "\tTMAC_TXD_1:\n");
	DBGLOG(HAL, INFO, "\t\twlan_idx=%d\n", txd_1->wlan_idx);
	DBGLOG(HAL, INFO, "\t\tHdrFmt=%d(%s)\n",
			 txd_1->hdr_format, hdr_fmt_str[txd_1->hdr_format]);
	DBGLOG(HAL, INFO, "\t\tHdrInfo=0x%x\n", txd_1->hdr_info);

	switch (txd_1->hdr_format) {
	case TMI_HDR_FT_NON_80211:
		DBGLOG(HAL, INFO,
			"\t\t\tMRD=%d, EOSP=%d, RMVL=%d, VLAN=%d, ETYP=%d\n",
			txd_1->hdr_info & (1 << TMI_HDR_INFO_0_BIT_MRD),
			txd_1->hdr_info & (1 << TMI_HDR_INFO_0_BIT_EOSP),
			txd_1->hdr_info & (1 << TMI_HDR_INFO_0_BIT_RMVL),
			txd_1->hdr_info & (1 << TMI_HDR_INFO_0_BIT_VLAN),
			txd_1->hdr_info & (1 << TMI_HDR_INFO_0_BIT_ETYP));
		break;

	case TMI_HDR_FT_CMD:
		DBGLOG(HAL, INFO, "\t\t\tRsvd=0x%x\n", txd_1->hdr_info);
		break;

	case TMI_HDR_FT_NOR_80211:
		DBGLOG(HAL, INFO, "\t\t\tHeader Len=%d(WORD)\n",
				 txd_1->hdr_info & TMI_HDR_INFO_2_MASK_LEN);
		break;

	case TMI_HDR_FT_ENH_80211:
		DBGLOG(HAL, INFO, "\t\t\tEOSP=%d, AMS=%d\n",
			txd_1->hdr_info & (1 << TMI_HDR_INFO_3_BIT_EOSP),
			txd_1->hdr_info & (1 << TMI_HDR_INFO_3_BIT_AMS));
		break;
	}

	DBGLOG(HAL, INFO, "\t\tTxDFormatType=%d(%s format)\n", txd_1->ft,
		(txd_1->ft == TMI_FT_LONG ?
		"Long - 8 DWORD" : "Short - 3 DWORD"));
	DBGLOG(HAL, INFO, "\t\ttxd_len=%d page(%d DW)\n",
		txd_1->txd_len == 0 ? 1 : 2, (txd_1->txd_len + 1) * 16);
	DBGLOG(HAL, INFO,
		"\t\tHdrPad=%d(Padding Mode: %s, padding bytes: %d)\n",
		txd_1->hdr_pad,
		((txd_1->hdr_pad & (TMI_HDR_PAD_MODE_TAIL << 1)) ?
		"tail" : "head"), (txd_1->hdr_pad & 0x1 ? 2 : 0));
	DBGLOG(HAL, INFO, "\t\tUNxV=%d\n", txd_1->UNxV);
	DBGLOG(HAL, INFO, "\t\tamsdu=%d\n", txd_1->amsdu);
	DBGLOG(HAL, INFO, "\t\tTID=%d\n", txd_1->tid);
	DBGLOG(HAL, INFO, "\t\tpkt_ft=%d(%s)\n",
			 txd_1->pkt_ft, pkt_ft_str[txd_1->pkt_ft]);
	DBGLOG(HAL, INFO, "\t\town_mac=%d\n", txd_1->OwnMacAddr);

	if (txd_s->TxD1.ft == TMI_FT_LONG) {
		struct TMAC_TXD_L *txd_l = (struct TMAC_TXD_L *)tmac_info;
		struct TMAC_TXD_2 *txd_2 = &txd_l->TxD2;
		struct TMAC_TXD_3 *txd_3 = &txd_l->TxD3;
		struct TMAC_TXD_4 *txd_4 = &txd_l->TxD4;
		struct TMAC_TXD_5 *txd_5 = &txd_l->TxD5;
		struct TMAC_TXD_6 *txd_6 = &txd_l->TxD6;

		DBGLOG(HAL, INFO, "\tTMAC_TXD_2:\n");
		DBGLOG(HAL, INFO, "\t\tsub_type=%d\n", txd_2->sub_type);
		DBGLOG(HAL, INFO, "\t\tfrm_type=%d\n", txd_2->frm_type);
		DBGLOG(HAL, INFO, "\t\tNDP=%d\n", txd_2->ndp);
		DBGLOG(HAL, INFO, "\t\tNDPA=%d\n", txd_2->ndpa);
		DBGLOG(HAL, INFO, "\t\tSounding=%d\n", txd_2->sounding);
		DBGLOG(HAL, INFO, "\t\tRTS=%d\n", txd_2->rts);
		DBGLOG(HAL, INFO, "\t\tbc_mc_pkt=%d\n", txd_2->bc_mc_pkt);
		DBGLOG(HAL, INFO, "\t\tBIP=%d\n", txd_2->bip);
		DBGLOG(HAL, INFO, "\t\tDuration=%d\n", txd_2->duration);
		DBGLOG(HAL, INFO, "\t\tHE(HTC Exist)=%d\n", txd_2->htc_vld);
		DBGLOG(HAL, INFO, "\t\tFRAG=%d\n", txd_2->frag);
		DBGLOG(HAL, INFO, "\t\tReamingLife/MaxTx time=%d\n",
			txd_2->max_tx_time);
		DBGLOG(HAL, INFO, "\t\tpwr_offset=%d\n", txd_2->pwr_offset);
		DBGLOG(HAL, INFO, "\t\tba_disable=%d\n", txd_2->ba_disable);
		DBGLOG(HAL, INFO, "\t\ttiming_measure=%d\n",
			txd_2->timing_measure);
		DBGLOG(HAL, INFO, "\t\tfix_rate=%d\n", txd_2->fix_rate);
		DBGLOG(HAL, INFO, "\tTMAC_TXD_3:\n");
		DBGLOG(HAL, INFO, "\t\tNoAck=%d\n", txd_3->no_ack);
		DBGLOG(HAL, INFO, "\t\tPF=%d\n", txd_3->protect_frm);
		DBGLOG(HAL, INFO, "\t\ttx_cnt=%d\n", txd_3->tx_cnt);
		DBGLOG(HAL, INFO, "\t\tremain_tx_cnt=%d\n",
			txd_3->remain_tx_cnt);
		DBGLOG(HAL, INFO, "\t\tsn=%d\n", txd_3->sn);
		DBGLOG(HAL, INFO, "\t\tpn_vld=%d\n", txd_3->pn_vld);
		DBGLOG(HAL, INFO, "\t\tsn_vld=%d\n", txd_3->sn_vld);
		DBGLOG(HAL, INFO, "\tTMAC_TXD_4:\n");
		DBGLOG(HAL, INFO, "\t\tpn_low=0x%x\n", txd_4->pn_low);
		DBGLOG(HAL, INFO, "\tTMAC_TXD_5:\n");
		DBGLOG(HAL, INFO, "\t\ttx_status_2_host=%d\n",
			txd_5->tx_status_2_host);
		DBGLOG(HAL, INFO, "\t\ttx_status_2_mcu=%d\n",
			txd_5->tx_status_2_mcu);
		DBGLOG(HAL, INFO, "\t\ttx_status_fmt=%d\n",
			txd_5->tx_status_fmt);

		if (txd_5->tx_status_2_host || txd_5->tx_status_2_mcu)
			DBGLOG(HAL, INFO, "\t\tpid=%d\n", txd_5->pid);

		if (txd_2->fix_rate)
			DBGLOG(HAL, INFO,
				"\t\tda_select=%d\n", txd_5->da_select);

		DBGLOG(HAL, INFO, "\t\tpwr_mgmt=0x%x\n", txd_5->pwr_mgmt);
		DBGLOG(HAL, INFO, "\t\tpn_high=0x%x\n", txd_5->pn_high);

		if (txd_2->fix_rate) {
			DBGLOG(HAL, INFO, "\tTMAC_TXD_6:\n");
			DBGLOG(HAL, INFO, "\t\tfix_rate_mode=%d\n",
				txd_6->fix_rate_mode);
			DBGLOG(HAL, INFO, "\t\tGI=%d(%s)\n", txd_6->gi,
				(txd_6->gi == 0 ? "LONG" : "SHORT"));
			DBGLOG(HAL, INFO, "\t\tldpc=%d(%s)\n", txd_6->ldpc,
				(txd_6->ldpc == 0 ? "BCC" : "LDPC"));
			DBGLOG(HAL, INFO, "\t\tTxBF=%d\n", txd_6->TxBF);
			DBGLOG(HAL, INFO, "\t\ttx_rate=0x%x\n", txd_6->tx_rate);
			DBGLOG(HAL, INFO, "\t\tant_id=%d\n", txd_6->ant_id);
			DBGLOG(HAL, INFO, "\t\tdyn_bw=%d\n", txd_6->dyn_bw);
			DBGLOG(HAL, INFO, "\t\tbw=%d\n", txd_6->bw);
		}
	}
}
#endif /* if (CFG_SUPPORT_CONNAC3X == 0) */

void halShowLitePleInfo(IN struct ADAPTER *prAdapter)
{
	uint32_t pg_flow_ctrl[6] = {0};
	uint32_t rpg_hif, upg_hif, i, j;
	uint32_t ple_stat[17] = {0};
	uint32_t sta_pause[4] = {0};
	uint32_t dis_sta_map[4] = {0};

	HAL_MCR_RD(prAdapter, PLE_HIF_PG_INFO, &pg_flow_ctrl[3]);
	rpg_hif = pg_flow_ctrl[3] & 0xfff;
	upg_hif = (pg_flow_ctrl[3] & (0xfff << 16)) >> 16;
	DBGLOG(HAL, INFO,
	  "\t\tThe used/reserved pages of HIF group=0x%03x/0x%03x\n",
	  upg_hif, rpg_hif);

	HAL_MCR_RD(prAdapter, PLE_QUEUE_EMPTY, &ple_stat[0]);
	HAL_MCR_RD(prAdapter, PLE_AC0_QUEUE_EMPTY_0, &ple_stat[1]);
	HAL_MCR_RD(prAdapter, PLE_AC0_QUEUE_EMPTY_1, &ple_stat[2]);
	HAL_MCR_RD(prAdapter, PLE_AC0_QUEUE_EMPTY_2, &ple_stat[3]);
	HAL_MCR_RD(prAdapter, PLE_AC0_QUEUE_EMPTY_3, &ple_stat[4]);
	HAL_MCR_RD(prAdapter, PLE_AC1_QUEUE_EMPTY_0, &ple_stat[5]);
	HAL_MCR_RD(prAdapter, PLE_AC1_QUEUE_EMPTY_1, &ple_stat[6]);
	HAL_MCR_RD(prAdapter, PLE_AC1_QUEUE_EMPTY_2, &ple_stat[7]);
	HAL_MCR_RD(prAdapter, PLE_AC1_QUEUE_EMPTY_3, &ple_stat[8]);
	HAL_MCR_RD(prAdapter, PLE_AC2_QUEUE_EMPTY_0, &ple_stat[9]);
	HAL_MCR_RD(prAdapter, PLE_AC2_QUEUE_EMPTY_1, &ple_stat[10]);
	HAL_MCR_RD(prAdapter, PLE_AC2_QUEUE_EMPTY_2, &ple_stat[11]);
	HAL_MCR_RD(prAdapter, PLE_AC2_QUEUE_EMPTY_3, &ple_stat[12]);
	HAL_MCR_RD(prAdapter, PLE_AC3_QUEUE_EMPTY_0, &ple_stat[13]);
	HAL_MCR_RD(prAdapter, PLE_AC3_QUEUE_EMPTY_1, &ple_stat[14]);
	HAL_MCR_RD(prAdapter, PLE_AC3_QUEUE_EMPTY_2, &ple_stat[15]);
	HAL_MCR_RD(prAdapter, PLE_AC3_QUEUE_EMPTY_3, &ple_stat[16]);

	HAL_MCR_RD(prAdapter, STATION_PAUSE0, &sta_pause[0]);
	HAL_MCR_RD(prAdapter, STATION_PAUSE1, &sta_pause[1]);
	HAL_MCR_RD(prAdapter, STATION_PAUSE2, &sta_pause[2]);
	HAL_MCR_RD(prAdapter, STATION_PAUSE3, &sta_pause[3]);

	HAL_MCR_RD(prAdapter, DIS_STA_MAP0, &dis_sta_map[0]);
	HAL_MCR_RD(prAdapter, DIS_STA_MAP1, &dis_sta_map[1]);
	HAL_MCR_RD(prAdapter, DIS_STA_MAP2, &dis_sta_map[2]);
	HAL_MCR_RD(prAdapter, DIS_STA_MAP3, &dis_sta_map[3]);

	for (j = 0; j < 16; j = j + 4) { /* show AC Q info */
		for (i = 0; i < 32; i++) {
			if (((ple_stat[j + 1] & (0x1 << i)) >> i) == 0) {
				uint32_t hfid, tfid, pktcnt,
					ac_num = j / 4, ctrl = 0;
				uint32_t sta_num = i + (j % 4) * 32,
					fl_que_ctrl[3] = {0};
				fl_que_ctrl[0] |= (0x1 << 31);
				fl_que_ctrl[0] |= (0x2 << 14);
				fl_que_ctrl[0] |= (ac_num << 8);
				fl_que_ctrl[0] |= sta_num;
				HAL_MCR_WR(prAdapter,
					PLE_FL_QUE_CTRL_0, fl_que_ctrl[0]);
				HAL_MCR_RD(prAdapter,
					PLE_FL_QUE_CTRL_2, &fl_que_ctrl[1]);
				HAL_MCR_RD(prAdapter,
					PLE_FL_QUE_CTRL_3, &fl_que_ctrl[2]);
				hfid = fl_que_ctrl[1] & 0xfff;
				tfid = (fl_que_ctrl[1] & 0xfff << 16) >> 16;
				pktcnt = fl_que_ctrl[2] & 0xfff;

				if (((sta_pause[j % 4] & 0x1 << i) >> i) == 1)
					ctrl = 2;
				if (((dis_sta_map[j % 4] & 0x1 << i) >> i) == 1)
					ctrl = 1;
				DBGLOG(HAL, INFO,
					"STA%d AC%d:",
					 sta_num, ac_num);
				DBGLOG(HAL, INFO,
					" tail/head fid = 0x%03x/0x%03x,",
					tfid, hfid);
				DBGLOG(HAL, INFO,
					" pkt cnt = %x  ctrl = %s\n",
					pktcnt, sta_ctrl_reg[ctrl]);
			}
		}
	}
}

void halShowTxdInfo(
	struct ADAPTER *prAdapter,
	u_int32_t fid)
{
	uint32_t txd_info[16] = {0};

	halGetPleTxdInfo(prAdapter, fid, txd_info);
	halDumpTxdInfo(prAdapter, txd_info);
}

