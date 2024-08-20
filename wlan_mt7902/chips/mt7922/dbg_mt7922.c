/* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause */
/*
 * Copyright (c) 2016 MediaTek Inc.
 */
/******************************************************************************
 *[File]             dbg_mt7922.c
 *[Version]          v1.0
 *[Revision Date]    2021-03-10
 *[Author]
 *[Description]
 *    The program provides WIFI FALCON MAC Debug APIs
 *[Copyright]
 *    Copyright (C) 2015 MediaTek Incorporation. All Rights Reserved.
 ******************************************************************************/

#if defined(MT7922)
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

/*******************************************************************************
 *                             D A T A   T Y P E S
 *******************************************************************************
 */
#if (CFG_SUPPORT_DEBUG_SOP == 1)
struct MT7922_DEBUG_SOP_INFO {
	u_int32_t	*wfsys_sleep_status;
	uint8_t		wfsys_sleep_cr_num;
	u_int32_t	*wfsys_status;
	uint8_t		wfsys_cr_num;
	u_int32_t	*bgfsys_bus_status;
	uint8_t		bgfsys_bus_cr_num;
	u_int32_t	*bgfsys_status;
	uint8_t		bgfsys_cr_num;
	u_int32_t	*conninfra_status;
	uint8_t		conninfra_cr_num;
	u_int32_t	*conninfra_bus_cr;
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
#if (CFG_SUPPORT_DEBUG_SOP == 1)
static u_int32_t mt7922_wfsys_sleep_status_sel[] = {
	0x00100000, 0x00108421, 0x00184210, 0x001BDEF7, 0x001EF7BD};

static u_int32_t mt7922_wfsys_status_sel[] = {
	0xD0010001, 0xD0020001, 0xD0030001, 0xD0040001, 0xD0050001,
	0xD0060001, 0xD0070001, 0xD0080001, 0xD0090001, 0xD00A0001,
	0xD00B0001, 0xD00C0001, 0xD00D0001, 0xD00E0001, 0xD00F0001,
	0xD0100001, 0xD0010002, 0xD0010003
};

static u_int32_t mt7922_bgfsys_bus_status_sel[] = {
	0x00767501, 0x00787701, 0x007A7901, 0x007C7B01, 0x007E7D01,
	0x00807F01, 0x00828101, 0x00848301, 0x00868501, 0x00888701,
	0x008A8901, 0x008C8B01, 0x008E8D01, 0x00908F01, 0x00929101,
	0x00949301, 0x00969501, 0x00989701, 0x009A9901, 0x009C9B01,
	0x009E9D01, 0x00A09F01, 0x00A2A101, 0x00A4A301, 0x00A6A501,
	0x00A8A701, 0x00AAA901, 0x00ACAB01, 0x00AEAD01, 0x00B0AF01
};

static u_int32_t mt7922_bgfsys_status_sel[] = {
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

static u_int32_t mt7922_conninfra_status_sel[] = {
	0x00000000, 0x00000001, 0x00000002, 0x00000003, 0x00000004,
	0x00000005};

static u_int32_t mt7922_conninfra_bus_cr[] = {
	0xF1514, 0xF1504, 0xF1564, 0xF1554, 0xF1544, 0xF1524,
	0xF1534, 0xFE2A0, 0xFE2A8, 0xFE2A4, 0xFF408, 0xFF40C,
	0xFF410, 0xFF414, 0xFF418, 0xFF41C, 0xFF420, 0xFF424,
	0xFF428, 0xFF42C, 0xFF430, 0xE0414, 0xE0418, 0xE042C,
	0xE0430, 0xE041C, 0xE0420, 0xE0410, 0xFE370
};

static u_int32_t mt7922_conn_infra_signal_status_sel[] = {
	0x00010001, 0x00020001, 0x00010002, 0x00020002, 0x00030002,
	0x00010003, 0x00020003, 0x00030003, 0x00010004, 0x00020004,
	0x00010005
};

static struct MT7922_DEBUG_SOP_INFO mt7922_debug_sop_info[] = {
	{mt7922_wfsys_sleep_status_sel,
	sizeof(mt7922_wfsys_sleep_status_sel)/sizeof(u_int32_t),
	mt7922_wfsys_status_sel,
	sizeof(mt7922_wfsys_status_sel)/sizeof(u_int32_t),
	mt7922_bgfsys_bus_status_sel,
	sizeof(mt7922_bgfsys_bus_status_sel)/sizeof(u_int32_t),
	mt7922_bgfsys_status_sel,
	sizeof(mt7922_bgfsys_status_sel)/sizeof(u_int32_t),
	mt7922_conninfra_status_sel,
	sizeof(mt7922_conninfra_status_sel)/sizeof(u_int32_t),
	mt7922_conninfra_bus_cr,
	sizeof(mt7922_conninfra_bus_cr)/sizeof(u_int32_t),
	mt7922_conn_infra_signal_status_sel,
	sizeof(mt7922_conn_infra_signal_status_sel)/sizeof(u_int32_t),
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
#if (CFG_SUPPORT_DEBUG_SOP == 1)
void pcie_mt7922_dump_wfsys_debug_cr(struct ADAPTER *prAdapter,
	uint8_t ucCase)
{
	uint8_t i = 0;
	uint32_t u4Val = 0;

	if (ucCase == SLAVENORESP) {
		DBGLOG(HAL, INFO,
		 "--WFSYS sel CR: 0x18060164, status CR: 0x18060168 loop start--\n"
		);
		for (i = 0; i < mt7922_debug_sop_info->wfsys_cr_num; i++) {
			HAL_MCR_WR(prAdapter,
			  CONNAC25_PCIE_WF_DEBUG_SEL,
			  mt7922_debug_sop_info->wfsys_status[i]);

			HAL_MCR_RD(prAdapter,
			  CONNAC25_PCIE_WF_DEBUG_STATUS, &u4Val);

			DBGLOG(HAL, INFO,
			  "WFSYS sel: 0x%08x, Val: 0x%08x\n",
			  mt7922_debug_sop_info->wfsys_status[i], u4Val);
		}
		DBGLOG(HAL, INFO,
		 "--WFSYS sel CR: 0x18060164, status CR: 0x18060168 loop end--\n"
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
	for (i = 0; i < mt7922_debug_sop_info->wfsys_sleep_cr_num; i++) {
		HAL_MCR_WR(prAdapter,
		  CONNAC2X_PCIE_DEBUG_SEL,
		  mt7922_debug_sop_info->wfsys_sleep_status[i]);

		HAL_MCR_RD(prAdapter,
		  CONNAC2X_PCIE_DEBUG_STATUS, &u4Val);

		DBGLOG(HAL, INFO,
		  "WFSYS sleep sel: 0x%08x, Val: 0x%08x\n",
		  mt7922_debug_sop_info->wfsys_sleep_status[i], u4Val);
	}
	DBGLOG(HAL, INFO,
	 "--WFSYS sel CR: 0x18060094, status CR: 0x1806021C loop end--\n"
	);
}

void pcie_mt7922_dump_bgfsys_debug_cr(struct ADAPTER *prAdapter,
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
		for (i = 0; i < mt7922_debug_sop_info->bgfsys_bus_cr_num; i++) {
			HAL_MCR_WR(prAdapter,
			  CONNAC25_PCIE_BGF_BUS_SEL,
			  mt7922_debug_sop_info->bgfsys_bus_status[i]);

			HAL_MCR_RD(prAdapter,
			  CONNAC25_PCIE_BGF_BUS_STATUS, &u4Val);

			DBGLOG(HAL, INFO,
			  "BGFSYS bus sel: 0x%08x, Val: 0x%08x\n",
			  mt7922_debug_sop_info->bgfsys_bus_status[i], u4Val);
		}
		DBGLOG(HAL, INFO,
		 "--BGF sel CR: 0x180600A8, status CR: 0x1806022C loop end--\n"
		);
	}
	DBGLOG(HAL, INFO,
	 "--BGF sel CR: 0x180600AC, status CR: 0x1806023C loop start--\n"
	);
	for (i = 0; i < mt7922_debug_sop_info->bgfsys_cr_num; i++) {
		HAL_MCR_WR(prAdapter,
		  CONNAC25_PCIE_BGF_DBG_SEL,
		  mt7922_debug_sop_info->bgfsys_status[i]);

		HAL_MCR_RD(prAdapter,
		  CONNAC25_PCIE_BGF_DBG_STATUS, &u4Val);

		DBGLOG(HAL, INFO,
		  "BGFSYS sel: 0x%08x, Val: 0x%08x\n",
		  mt7922_debug_sop_info->bgfsys_status[i], u4Val);
	}
	DBGLOG(HAL, INFO,
	 "--BGF sel CR: 0x180600AC, status CR: 0x1806023C loop end--\n"
	);
}

void pcie_mt7922_dump_conninfra_debug_cr(struct ADAPTER *prAdapter,
	uint8_t ucCase)
{
	uint8_t i = 0;
	uint32_t u4Val = 0;

	DBGLOG(HAL, INFO,
	 "--CONNINFRA sel CR: 0x1806015C, status CR: 0x180602C8 loop start--\n"
	);
	for (i = 0; i < mt7922_debug_sop_info->conninfra_cr_num; i++) {
		HAL_MCR_WR(prAdapter, CONNAC25_PCIE_CONNINFRA_CLK_SEL,
		  mt7922_debug_sop_info->conninfra_status[i]);

		HAL_MCR_RD(prAdapter, CONNAC25_PCIE_CONNINFRA_CLK_STATUS,
		  &u4Val);

		DBGLOG(HAL, INFO, "CONNINFRA sel: 0x%08x, Val: 0x%08x\n",
		  mt7922_debug_sop_info->conninfra_status[i], u4Val);
	}
	DBGLOG(HAL, INFO,
	 "--CONNINFRA sel CR: 0x1806015C, status CR: 0x180602C8 loop end--\n"
	);

	if (ucCase == SLAVENORESP) {
		HAL_MCR_RD(prAdapter, CONNAC25_PCIE_CONNINFRA_STRAP_STATUS,
		  &u4Val);
		DBGLOG(HAL, INFO, "CONNINFRA STRAP: 0x%08x, Val: 0x%08x\n",
		  CONNAC25_PCIE_CONNINFRA_STRAP_STATUS, u4Val);

		for (i = 0; i < mt7922_debug_sop_info->conninfra_bus_cr_num;
			i++) {
			HAL_MCR_RD(prAdapter,
			  mt7922_debug_sop_info->conninfra_bus_cr[i],
			  &u4Val);

			DBGLOG(HAL, INFO,
			  "CONNINFRA bus cr: 0x%08x, Val: 0x%08x\n",
			  mt7922_debug_sop_info->conninfra_bus_cr[i], u4Val);
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
		for (i = 0; i < mt7922_debug_sop_info->conninfra_signal_num;
			i++) {
			HAL_MCR_WR(prAdapter,
			  CONNAC2X_PCIE_CONN_INFRA_BUS_SEL,
			  mt7922_debug_sop_info->conninfra_signal_status[i]);

			HAL_MCR_RD(prAdapter,
			  CONNAC2X_PCIE_CONN_INFRA_BUS_STATUS,
			  &u4Val);

			DBGLOG(HAL, INFO,
			  "CONNINFRA sel: 0x%08x, Val: 0x%08x\n",
			  mt7922_debug_sop_info->conninfra_signal_status[i],
			  u4Val);
		}
		DBGLOG(HAL, INFO,
		 "--CONNINFRA sel CR: 0x18060138, status CR: 0x18060150 loop end--\n"
		);
	}
}

u_int8_t mt7922_show_debug_sop_info(struct ADAPTER *prAdapter,
	uint8_t ucCase)
{
	uint32_t u4Val = 0;

	switch (ucCase) {
	case SLEEP:
		/* Check power status */
		DBGLOG(HAL, ERROR, "Sleep Fail!\n");
		HAL_MCR_RD(prAdapter, CONNAC25_PCIE_POWER_STATUS, &u4Val);
		DBGLOG(HAL, INFO, "Power status CR: 0x%08x, Val: 0x%08x\n",
			CONNAC25_PCIE_POWER_STATUS, u4Val);
		pcie_mt7922_dump_wfsys_debug_cr(prAdapter, ucCase);
		pcie_mt7922_dump_bgfsys_debug_cr(prAdapter, ucCase);
		pcie_mt7922_dump_conninfra_debug_cr(prAdapter, ucCase);
		break;
	case SLAVENORESP:
		DBGLOG(HAL, ERROR, "Slave no response!\n");
		pcie_mt7922_dump_wfsys_debug_cr(prAdapter, ucCase);
		pcie_mt7922_dump_bgfsys_debug_cr(prAdapter, ucCase);
		pcie_mt7922_dump_conninfra_debug_cr(prAdapter, ucCase);
		break;
	default:
		break;
	}

	return TRUE;
}
#endif /* (CFG_SUPPORT_DEBUG_SOP == 1) */

#endif /* defined(MT7922) */
