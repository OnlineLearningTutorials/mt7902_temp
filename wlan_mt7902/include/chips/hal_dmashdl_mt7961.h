/* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause */
/*
 * Copyright (c) 2019 MediaTek Inc.
 */
/*! \file   hal_dmashdl_mt7961.h
*    \brief  DMASHDL HAL API for MT7961
*
*    This file contains all routines which are exported
     from MediaTek 802.11 Wireless LAN driver stack to GLUE Layer.
*/

#ifndef _HAL_DMASHDL_MT7961_H
#define _HAL_DMASHDL_MT7961_H

/*******************************************************************************
*                         C O M P I L E R   F L A G S
********************************************************************************
*/

/*******************************************************************************
*                    E X T E R N A L   R E F E R E N C E S
********************************************************************************
*/

/*******************************************************************************
*                              C O N S T A N T S
********************************************************************************
*/
/* align MT7663 setting first */
#if defined(_HIF_PCIE) || defined(_HIF_AXI)

/* 1: 3rd arbitration makes decision based on group priority in current slot.
 * 0: 3rd arbitration makes decision based on fixed user-defined priority.
 */
#define MT7961_DMASHDL_SLOT_ARBITER_EN                 (0)
#define MT7961_DMASHDL_PKT_PLE_MAX_PAGE                (0x1)
/* Buzzard CMD packet flow control is controlled by WFDMA, not DMASHDL.
 * So, CMD packet (group 15) related CRs in DMASHDL are ignored.
 */
#define MT7961_DMASHDL_PKT_PSE_MAX_PAGE                (0x0)
#define MT7961_DMASHDL_GROUP_0_REFILL_EN               (1)
#define MT7961_DMASHDL_GROUP_1_REFILL_EN               (1)
#define MT7961_DMASHDL_GROUP_2_REFILL_EN               (0)
#define MT7961_DMASHDL_GROUP_3_REFILL_EN               (0)
#define MT7961_DMASHDL_GROUP_4_REFILL_EN               (0)
#define MT7961_DMASHDL_GROUP_5_REFILL_EN               (0)
#define MT7961_DMASHDL_GROUP_6_REFILL_EN               (0)
#define MT7961_DMASHDL_GROUP_7_REFILL_EN               (0)
#define MT7961_DMASHDL_GROUP_8_REFILL_EN               (0)
#define MT7961_DMASHDL_GROUP_9_REFILL_EN               (0)
#define MT7961_DMASHDL_GROUP_10_REFILL_EN              (0)
#define MT7961_DMASHDL_GROUP_11_REFILL_EN              (0)
#define MT7961_DMASHDL_GROUP_12_REFILL_EN              (0)
#define MT7961_DMASHDL_GROUP_13_REFILL_EN              (0)
#define MT7961_DMASHDL_GROUP_14_REFILL_EN              (0)
#define MT7961_DMASHDL_GROUP_15_REFILL_EN              (0)
#define MT7961_DMASHDL_GROUP_0_MAX_QUOTA               (0xFFF)
#define MT7961_DMASHDL_GROUP_1_MAX_QUOTA               (0xFFF)
#define MT7961_DMASHDL_GROUP_2_MAX_QUOTA               (0x0)
#define MT7961_DMASHDL_GROUP_3_MAX_QUOTA               (0x0)
#define MT7961_DMASHDL_GROUP_4_MAX_QUOTA               (0x0)
#define MT7961_DMASHDL_GROUP_5_MAX_QUOTA               (0x0)
#define MT7961_DMASHDL_GROUP_6_MAX_QUOTA               (0x0)
#define MT7961_DMASHDL_GROUP_7_MAX_QUOTA               (0x0)
#define MT7961_DMASHDL_GROUP_8_MAX_QUOTA               (0x0)
#define MT7961_DMASHDL_GROUP_9_MAX_QUOTA               (0x0)
#define MT7961_DMASHDL_GROUP_10_MAX_QUOTA              (0x0)
#define MT7961_DMASHDL_GROUP_11_MAX_QUOTA              (0x0)
#define MT7961_DMASHDL_GROUP_12_MAX_QUOTA              (0x0)
#define MT7961_DMASHDL_GROUP_13_MAX_QUOTA              (0x0)
#define MT7961_DMASHDL_GROUP_14_MAX_QUOTA              (0x0)
#define MT7961_DMASHDL_GROUP_15_MAX_QUOTA              (0x0)
#define MT7961_DMASHDL_GROUP_0_MIN_QUOTA               (0x3)
#define MT7961_DMASHDL_GROUP_1_MIN_QUOTA               (0x3)
#define MT7961_DMASHDL_GROUP_2_MIN_QUOTA               (0x0)
#define MT7961_DMASHDL_GROUP_3_MIN_QUOTA               (0x0)
#define MT7961_DMASHDL_GROUP_4_MIN_QUOTA               (0x0)
#define MT7961_DMASHDL_GROUP_5_MIN_QUOTA               (0x0)
#define MT7961_DMASHDL_GROUP_6_MIN_QUOTA               (0x0)
#define MT7961_DMASHDL_GROUP_7_MIN_QUOTA               (0x0)
#define MT7961_DMASHDL_GROUP_8_MIN_QUOTA               (0x0)
#define MT7961_DMASHDL_GROUP_9_MIN_QUOTA               (0x0)
#define MT7961_DMASHDL_GROUP_10_MIN_QUOTA              (0x0)
#define MT7961_DMASHDL_GROUP_11_MIN_QUOTA              (0x0)
#define MT7961_DMASHDL_GROUP_12_MIN_QUOTA              (0x0)
#define MT7961_DMASHDL_GROUP_13_MIN_QUOTA              (0x0)
#define MT7961_DMASHDL_GROUP_14_MIN_QUOTA              (0x0)
#define MT7961_DMASHDL_GROUP_15_MIN_QUOTA              (0x0)
#define MT7961_DMASHDL_QUEUE_0_TO_GROUP                (0x0)   /* LMAC AC00 */
#define MT7961_DMASHDL_QUEUE_1_TO_GROUP                (0x0)   /* LMAC AC01 */
#define MT7961_DMASHDL_QUEUE_2_TO_GROUP                (0x0)   /* LMAC AC02 */
#define MT7961_DMASHDL_QUEUE_3_TO_GROUP                (0x0)   /* LMAC AC03 */
#define MT7961_DMASHDL_QUEUE_4_TO_GROUP                (0x1)   /* LMAC AC10 */
#define MT7961_DMASHDL_QUEUE_5_TO_GROUP                (0x1)   /* LMAC AC11 */
#define MT7961_DMASHDL_QUEUE_6_TO_GROUP                (0x1)   /* LMAC AC12 */
#define MT7961_DMASHDL_QUEUE_7_TO_GROUP                (0x1)   /* LMAC AC13 */
#define MT7961_DMASHDL_QUEUE_8_TO_GROUP                (0x0)   /* LMAC AC20 */
#define MT7961_DMASHDL_QUEUE_9_TO_GROUP                (0x0)   /* LMAC AC21 */
#define MT7961_DMASHDL_QUEUE_10_TO_GROUP               (0x0)   /* LMAC AC22 */
#define MT7961_DMASHDL_QUEUE_11_TO_GROUP               (0x0)   /* LMAC AC23 */
#define MT7961_DMASHDL_QUEUE_12_TO_GROUP               (0x0)   /* LMAC AC30 */
#define MT7961_DMASHDL_QUEUE_13_TO_GROUP               (0x0)   /* LMAC AC31 */
#define MT7961_DMASHDL_QUEUE_14_TO_GROUP               (0x0)   /* LMAC AC32 */
#define MT7961_DMASHDL_QUEUE_15_TO_GROUP               (0x0)   /* LMAC AC33 */
#define MT7961_DMASHDL_QUEUE_16_TO_GROUP               (0x0)   /* ALTX */
#define MT7961_DMASHDL_QUEUE_17_TO_GROUP               (0x0)   /* BMC */
#define MT7961_DMASHDL_QUEUE_18_TO_GROUP               (0x0)   /* BCN */
#define MT7961_DMASHDL_QUEUE_19_TO_GROUP               (0x1)   /* HW Reserved */
#define MT7961_DMASHDL_QUEUE_20_TO_GROUP               (0x1)   /* HW Reserved */
#define MT7961_DMASHDL_QUEUE_21_TO_GROUP               (0x1)   /* HW Reserved */
#define MT7961_DMASHDL_QUEUE_22_TO_GROUP               (0x1)   /* HW Reserved */
#define MT7961_DMASHDL_QUEUE_23_TO_GROUP               (0x1)   /* HW Reserved */
#define MT7961_DMASHDL_QUEUE_24_TO_GROUP               (0x0)   /* NAF */
#define MT7961_DMASHDL_QUEUE_25_TO_GROUP               (0x0)   /* NBCN */
#define MT7961_DMASHDL_QUEUE_26_TO_GROUP               (0x0)   /* FIXFID */
#define MT7961_DMASHDL_QUEUE_27_TO_GROUP               (0x1)   /* Reserved */
#define MT7961_DMASHDL_QUEUE_28_TO_GROUP               (0x1)   /* Reserved */
#define MT7961_DMASHDL_QUEUE_29_TO_GROUP               (0x1)   /* Reserved */
#define MT7961_DMASHDL_QUEUE_30_TO_GROUP               (0x1)   /* Reserved */
#define MT7961_DMASHDL_QUEUE_31_TO_GROUP               (0x1)   /* Reserved */
#define MT7961_DMASHDL_PRIORITY0_GROUP                 (0x0)
#define MT7961_DMASHDL_PRIORITY1_GROUP                 (0x1)
#define MT7961_DMASHDL_PRIORITY2_GROUP                 (0x2)
#define MT7961_DMASHDL_PRIORITY3_GROUP                 (0x3)
#define MT7961_DMASHDL_PRIORITY4_GROUP                 (0x4)
#define MT7961_DMASHDL_PRIORITY5_GROUP                 (0x5)
#define MT7961_DMASHDL_PRIORITY6_GROUP                 (0x6)
#define MT7961_DMASHDL_PRIORITY7_GROUP                 (0x7)
#define MT7961_DMASHDL_PRIORITY8_GROUP                 (0x8)
#define MT7961_DMASHDL_PRIORITY9_GROUP                 (0x9)
#define MT7961_DMASHDL_PRIORITY10_GROUP                (0xA)
#define MT7961_DMASHDL_PRIORITY11_GROUP                (0xB)
#define MT7961_DMASHDL_PRIORITY12_GROUP                (0xC)
#define MT7961_DMASHDL_PRIORITY13_GROUP                (0xD)
#define MT7961_DMASHDL_PRIORITY14_GROUP                (0xE)
#define MT7961_DMASHDL_PRIORITY15_GROUP                (0xF)
#ifdef MT7922
/* PLE Quota 0x3D0*/
#define MT7961_DMASHDL_DBDC_5G_MAX_QUOTA               (0x1E8)
#define MT7961_DMASHDL_DBDC_2G_MAX_QUOTA               (0x1E8)
#define MT7961_DMASHDL_DBDC_5G_6G_MAX_QUOTA            (0x1E8)
#else
/*TODO: fine tune*/
#define MT7961_DMASHDL_DBDC_5G_MAX_QUOTA               (0xFFF)
#define MT7961_DMASHDL_DBDC_2G_MAX_QUOTA               (0xFFF)
#define MT7961_DMASHDL_DBDC_5G_6G_MAX_QUOTA            (0xFFF)
#endif

#elif defined(_HIF_USB)

/* 1: 3rd arbitration makes decision based on group priority in current slot.
 * 0: 3rd arbitration makes decision based on fixed user-defined priority.
 */
#define MT7961_DMASHDL_SLOT_ARBITER_EN                 (0)
#define MT7961_DMASHDL_PKT_PLE_MAX_PAGE                (0x1)
/* Buzzard CMD packet flow control is controlled by WFDMA, not DMASHDL.
 * So, CMD packet (group 15) related CRs in DMASHDL are ignored.
 */
#define MT7961_DMASHDL_PKT_PSE_MAX_PAGE                (0x0)
#define MT7961_DMASHDL_GROUP_0_REFILL_EN               (1)
#define MT7961_DMASHDL_GROUP_1_REFILL_EN               (1)
#define MT7961_DMASHDL_GROUP_2_REFILL_EN               (1)
#define MT7961_DMASHDL_GROUP_3_REFILL_EN               (1)
#define MT7961_DMASHDL_GROUP_4_REFILL_EN               (1)
#define MT7961_DMASHDL_GROUP_5_REFILL_EN               (0)
#define MT7961_DMASHDL_GROUP_6_REFILL_EN               (0)
#define MT7961_DMASHDL_GROUP_7_REFILL_EN               (0)
#define MT7961_DMASHDL_GROUP_8_REFILL_EN               (0)
#define MT7961_DMASHDL_GROUP_9_REFILL_EN               (0)
#define MT7961_DMASHDL_GROUP_10_REFILL_EN              (0)
#define MT7961_DMASHDL_GROUP_11_REFILL_EN              (0)
#define MT7961_DMASHDL_GROUP_12_REFILL_EN              (0)
#define MT7961_DMASHDL_GROUP_13_REFILL_EN              (0)
#define MT7961_DMASHDL_GROUP_14_REFILL_EN              (0)
#define MT7961_DMASHDL_GROUP_15_REFILL_EN              (0)
#define MT7961_DMASHDL_GROUP_0_MAX_QUOTA               (0xFFF)
#define MT7961_DMASHDL_GROUP_1_MAX_QUOTA               (0xFFF)
#define MT7961_DMASHDL_GROUP_2_MAX_QUOTA               (0xFFF)
#define MT7961_DMASHDL_GROUP_3_MAX_QUOTA               (0xFFF)
#define MT7961_DMASHDL_GROUP_4_MAX_QUOTA               (0xFFF)
#define MT7961_DMASHDL_GROUP_5_MAX_QUOTA               (0x0)
#define MT7961_DMASHDL_GROUP_6_MAX_QUOTA               (0x0)
#define MT7961_DMASHDL_GROUP_7_MAX_QUOTA               (0x0)
#define MT7961_DMASHDL_GROUP_8_MAX_QUOTA               (0x0)
#define MT7961_DMASHDL_GROUP_9_MAX_QUOTA               (0x0)
#define MT7961_DMASHDL_GROUP_10_MAX_QUOTA              (0x0)
#define MT7961_DMASHDL_GROUP_11_MAX_QUOTA              (0x0)
#define MT7961_DMASHDL_GROUP_12_MAX_QUOTA              (0x0)
#define MT7961_DMASHDL_GROUP_13_MAX_QUOTA              (0x0)
#define MT7961_DMASHDL_GROUP_14_MAX_QUOTA              (0x0)
#define MT7961_DMASHDL_GROUP_15_MAX_QUOTA              (0x0)
#define MT7961_DMASHDL_GROUP_0_MIN_QUOTA               (0x3)
#define MT7961_DMASHDL_GROUP_1_MIN_QUOTA               (0x3)
#define MT7961_DMASHDL_GROUP_2_MIN_QUOTA               (0x3)
#define MT7961_DMASHDL_GROUP_3_MIN_QUOTA               (0x3)
#define MT7961_DMASHDL_GROUP_4_MIN_QUOTA               (0x3)
#define MT7961_DMASHDL_GROUP_5_MIN_QUOTA               (0x0)
#define MT7961_DMASHDL_GROUP_6_MIN_QUOTA               (0x0)
#define MT7961_DMASHDL_GROUP_7_MIN_QUOTA               (0x0)
#define MT7961_DMASHDL_GROUP_8_MIN_QUOTA               (0x0)
#define MT7961_DMASHDL_GROUP_9_MIN_QUOTA               (0x0)
#define MT7961_DMASHDL_GROUP_10_MIN_QUOTA              (0x0)
#define MT7961_DMASHDL_GROUP_11_MIN_QUOTA              (0x0)
#define MT7961_DMASHDL_GROUP_12_MIN_QUOTA              (0x0)
#define MT7961_DMASHDL_GROUP_13_MIN_QUOTA              (0x0)
#define MT7961_DMASHDL_GROUP_14_MIN_QUOTA              (0x0)
#define MT7961_DMASHDL_GROUP_15_MIN_QUOTA              (0x0)
#define MT7961_DMASHDL_QUEUE_0_TO_GROUP                (0x0)   /* LMAC AC00 */
#define MT7961_DMASHDL_QUEUE_1_TO_GROUP                (0x1)   /* LMAC AC01 */
#define MT7961_DMASHDL_QUEUE_2_TO_GROUP                (0x2)   /* LMAC AC02 */
#define MT7961_DMASHDL_QUEUE_3_TO_GROUP                (0x3)   /* LMAC AC03 */
#define MT7961_DMASHDL_QUEUE_4_TO_GROUP                (0x4)   /* LMAC AC10 */
#define MT7961_DMASHDL_QUEUE_5_TO_GROUP                (0x4)   /* LMAC AC11 */
#define MT7961_DMASHDL_QUEUE_6_TO_GROUP                (0x4)   /* LMAC AC12 */
#define MT7961_DMASHDL_QUEUE_7_TO_GROUP                (0x4)   /* LMAC AC13 */
#define MT7961_DMASHDL_QUEUE_8_TO_GROUP                (0x4)   /* LMAC AC20 */
#define MT7961_DMASHDL_QUEUE_9_TO_GROUP                (0x4)   /* LMAC AC21 */
#define MT7961_DMASHDL_QUEUE_10_TO_GROUP               (0x4)   /* LMAC AC22 */
#define MT7961_DMASHDL_QUEUE_11_TO_GROUP               (0x4)   /* LMAC AC23 */
#define MT7961_DMASHDL_QUEUE_12_TO_GROUP               (0x4)   /* LMAC AC30 */
#define MT7961_DMASHDL_QUEUE_13_TO_GROUP               (0x4)   /* LMAC AC31 */
#define MT7961_DMASHDL_QUEUE_14_TO_GROUP               (0x4)   /* LMAC AC32 */
#define MT7961_DMASHDL_QUEUE_15_TO_GROUP               (0x4)   /* LMAC AC33 */
#define MT7961_DMASHDL_QUEUE_16_TO_GROUP               (0x4)   /* ALTX */
#define MT7961_DMASHDL_QUEUE_17_TO_GROUP               (0x4)   /* BMC */
#define MT7961_DMASHDL_QUEUE_18_TO_GROUP               (0x4)   /* BCN */
#define MT7961_DMASHDL_QUEUE_19_TO_GROUP               (0x5)   /* HW Reserved */
#define MT7961_DMASHDL_QUEUE_20_TO_GROUP               (0x5)   /* HW Reserved */
#define MT7961_DMASHDL_QUEUE_21_TO_GROUP               (0x5)   /* HW Reserved */
#define MT7961_DMASHDL_QUEUE_22_TO_GROUP               (0x5)   /* HW Reserved */
#define MT7961_DMASHDL_QUEUE_23_TO_GROUP               (0x5)   /* HW Reserved */
#define MT7961_DMASHDL_QUEUE_24_TO_GROUP               (0x4)   /* NAF */
#define MT7961_DMASHDL_QUEUE_25_TO_GROUP               (0x4)   /* NBCN */
#define MT7961_DMASHDL_QUEUE_26_TO_GROUP               (0x4)   /* FIXFID */
#define MT7961_DMASHDL_QUEUE_27_TO_GROUP               (0x5)   /* Reserved */
#define MT7961_DMASHDL_QUEUE_28_TO_GROUP               (0x5)   /* Reserved */
#define MT7961_DMASHDL_QUEUE_29_TO_GROUP               (0x5)   /* Reserved */
#define MT7961_DMASHDL_QUEUE_30_TO_GROUP               (0x5)   /* Reserved */
#define MT7961_DMASHDL_QUEUE_31_TO_GROUP               (0x5)   /* Reserved */
#define MT7961_DMASHDL_PRIORITY0_GROUP                 (0x3)
#define MT7961_DMASHDL_PRIORITY1_GROUP                 (0x2)
#define MT7961_DMASHDL_PRIORITY2_GROUP                 (0x1)
#define MT7961_DMASHDL_PRIORITY3_GROUP                 (0x0)
#define MT7961_DMASHDL_PRIORITY4_GROUP                 (0x4)
#define MT7961_DMASHDL_PRIORITY5_GROUP                 (0x5)
#define MT7961_DMASHDL_PRIORITY6_GROUP                 (0x6)
#define MT7961_DMASHDL_PRIORITY7_GROUP                 (0x7)
#define MT7961_DMASHDL_PRIORITY8_GROUP                 (0x8)
#define MT7961_DMASHDL_PRIORITY9_GROUP                 (0x9)
#define MT7961_DMASHDL_PRIORITY10_GROUP                (0xA)
#define MT7961_DMASHDL_PRIORITY11_GROUP                (0xB)
#define MT7961_DMASHDL_PRIORITY12_GROUP                (0xC)
#define MT7961_DMASHDL_PRIORITY13_GROUP                (0xD)
#define MT7961_DMASHDL_PRIORITY14_GROUP                (0xE)
#define MT7961_DMASHDL_PRIORITY15_GROUP                (0xF)
#if (CFG_WIFI_FWDL_UMAC_RESERVE_SIZE_PARA == 128)
/* PSE quota 169*/
#define MT7961_DMASHDL_DBDC_5G_MAX_QUOTA               (0x38)
#define MT7961_DMASHDL_DBDC_2G_MAX_QUOTA               (0x1C)
#define MT7961_DMASHDL_DBDC_5G_6G_MAX_QUOTA            (0x2A)
#else
/* PSE quota 212*/
#define MT7961_DMASHDL_DBDC_5G_MAX_QUOTA               (0x46)
#define MT7961_DMASHDL_DBDC_2G_MAX_QUOTA               (0x24)
#define MT7961_DMASHDL_DBDC_5G_6G_MAX_QUOTA            (0x35)
#endif
#endif /* defined(_HIF_PCIE) || defined(_HIF_AXI) */

#define MT7961_DMASHDL_QUEUE_NUM        32
#define MT7961_DMASHDL_PRIORITY_NUM     16

/*******************************************************************************
*                         D A T A   T Y P E S
********************************************************************************
*/

enum ENUM_MT7961_DMASHDL_GROUP_IDX {
	ENUM_MT7961_DMASHDL_GROUP_0 = 0,
	ENUM_MT7961_DMASHDL_GROUP_1,
	ENUM_MT7961_DMASHDL_GROUP_2,
	ENUM_MT7961_DMASHDL_GROUP_3,
	ENUM_MT7961_DMASHDL_GROUP_4,
	ENUM_MT7961_DMASHDL_GROUP_5,
	ENUM_MT7961_DMASHDL_GROUP_6,
	ENUM_MT7961_DMASHDL_GROUP_7,
	ENUM_MT7961_DMASHDL_GROUP_8,
	ENUM_MT7961_DMASHDL_GROUP_9,
	ENUM_MT7961_DMASHDL_GROUP_10,
	ENUM_MT7961_DMASHDL_GROUP_11,
	ENUM_MT7961_DMASHDL_GROUP_12,
	ENUM_MT7961_DMASHDL_GROUP_13,
	ENUM_MT7961_DMASHDL_GROUP_14,
	ENUM_MT7961_DMASHDL_GROUP_15,
	ENUM_MT7961_DMASHDL_GROUP_NUM
};

struct MT7961_DMASHDL_CFG {
	u_int8_t fgSlotArbiterEn;
	uint16_t u2PktPleMaxPage;
	uint16_t u2PktPseMaxPage;
	u_int8_t afgRefillEn[ENUM_MT7961_DMASHDL_GROUP_NUM];
	uint16_t au2MaxQuota[ENUM_MT7961_DMASHDL_GROUP_NUM];
	uint16_t au2MinQuota[ENUM_MT7961_DMASHDL_GROUP_NUM];
	uint8_t aucQueue2Group[32];
	uint8_t aucPriority2Group[16];
};

/*******************************************************************************
*                            P U B L I C   D A T A
********************************************************************************
*/

/*******************************************************************************
*                           P R I V A T E   D A T A
********************************************************************************
*/

/*******************************************************************************
*                                 M A C R O S
********************************************************************************
*/

/*******************************************************************************
*                  F U N C T I O N   D E C L A R A T I O N S
********************************************************************************
*/

/*******************************************************************************
*                              F U N C T I O N S
********************************************************************************
*/

void mt7961HalDmashdlSetPlePktMaxPage(struct ADAPTER *prAdapter,
				      uint16_t u2MaxPage);

void mt7961HalDmashdlSetPsePktMaxPage(struct ADAPTER *prAdapter,
				      uint16_t u2MaxPage);

void mt7961HalDmashdlSetPktMaxPage(struct ADAPTER *prAdapter,
				   uint16_t u2PleMaxPage,
				   uint16_t u2PseMaxPage);

void mt7961HalDmashdlSetRefill(struct ADAPTER *prAdapter, uint8_t ucGroup,
			       u_int8_t fgEnable);

void mt7961HalDmashdlSetAllRefill(struct ADAPTER *prAdapter,
				  u_int8_t *pfgEnable);

void mt7961HalDmashdlSetMaxQuota(struct ADAPTER *prAdapter, uint8_t ucGroup,
				 uint16_t u2MaxQuota);

void mt7961HalDmashdlSetMinQuota(struct ADAPTER *prAdatper, uint8_t ucGroup,
				 uint16_t u2MinQuota);

void mt7961HalDmashdlSetAllQuota(struct ADAPTER *prAdapter,
				 uint16_t *pu2MaxQuota, uint16_t *pu2MinQuota);

void mt7961HalDmashdlSetQueueMapping(struct ADAPTER *prAdapter, uint8_t ucQueue,
				     uint8_t ucGroup);

void mt7961HalDmashdlSetAllQueueMapping(struct ADAPTER *prAdapter,
					uint8_t *pucGroup);

void mt7961HalDmashdlSetSlotArbiter(struct ADAPTER *prAdapter,
				    u_int8_t fgEnable);

void mt7961HalDmashdlSetUserDefinedPriority(struct ADAPTER *prAdapter,
					    uint8_t ucPriority,
					    uint8_t ucGroup);

void mt7961HalDmashdlSetAllUserDefinedPriority(struct ADAPTER *prAdapter,
					       uint8_t *pucGroup);

void mt7961DmashdlInit(struct ADAPTER *prAdapter);

void mt7961DmashdlReInit(struct ADAPTER *prAdapter);

#if defined(_HIF_PCIE) || defined(_HIF_AXI) || defined(_HIF_USB)
uint32_t mt7961UpdateDmashdlQuota(struct ADAPTER *prAdapter,
			uint8_t ucWmmIndex, uint32_t u4MaxQuota);

uint32_t mt7961dmashdlQuotaDecision (struct ADAPTER *prAdapter,
			uint8_t ucWmmIndex);
#endif
#endif /* _HAL_DMASHDL_MT7961_H */
