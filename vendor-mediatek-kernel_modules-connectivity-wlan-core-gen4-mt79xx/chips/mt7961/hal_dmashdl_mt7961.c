/* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause */
/*
 * Copyright (c) 2019 MediaTek Inc.
 */
/*! \file   hal_dmashdl_mt7961.c
*    \brief  DMASHDL HAL API for MT7961
*
*    This file contains all routines which are exported
     from MediaTek 802.11 Wireless LAN driver stack to GLUE Layer.
*/

#if defined(MT7961) || defined(MT7922)
#if defined(_HIF_PCIE) || defined(_HIF_AXI) || defined(_HIF_USB)

/*******************************************************************************
*                         C O M P I L E R   F L A G S
********************************************************************************
*/

/*******************************************************************************
*                    E X T E R N A L   R E F E R E N C E S
********************************************************************************
*/

#include "precomp.h"
#include "mt7961.h"
#include "coda/mt7961/wf_hif_dmashdl_top.h"
#include "hal_dmashdl_mt7961.h"

/*******************************************************************************
*                              C O N S T A N T S
********************************************************************************
*/

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


/*******************************************************************************
*                            P U B L I C   D A T A
********************************************************************************
*/

struct MT7961_DMASHDL_CFG rMT7961DmashdlCfg = {
	.fgSlotArbiterEn = MT7961_DMASHDL_SLOT_ARBITER_EN,

	.u2PktPleMaxPage = MT7961_DMASHDL_PKT_PLE_MAX_PAGE,

	.u2PktPseMaxPage = MT7961_DMASHDL_PKT_PSE_MAX_PAGE,

	.afgRefillEn = {
		MT7961_DMASHDL_GROUP_0_REFILL_EN,
		MT7961_DMASHDL_GROUP_1_REFILL_EN,
		MT7961_DMASHDL_GROUP_2_REFILL_EN,
		MT7961_DMASHDL_GROUP_3_REFILL_EN,
		MT7961_DMASHDL_GROUP_4_REFILL_EN,
		MT7961_DMASHDL_GROUP_5_REFILL_EN,
		MT7961_DMASHDL_GROUP_6_REFILL_EN,
		MT7961_DMASHDL_GROUP_7_REFILL_EN,
		MT7961_DMASHDL_GROUP_8_REFILL_EN,
		MT7961_DMASHDL_GROUP_9_REFILL_EN,
		MT7961_DMASHDL_GROUP_10_REFILL_EN,
		MT7961_DMASHDL_GROUP_11_REFILL_EN,
		MT7961_DMASHDL_GROUP_12_REFILL_EN,
		MT7961_DMASHDL_GROUP_13_REFILL_EN,
		MT7961_DMASHDL_GROUP_14_REFILL_EN,
		MT7961_DMASHDL_GROUP_15_REFILL_EN,
	},

	.au2MaxQuota = {
		MT7961_DMASHDL_GROUP_0_MAX_QUOTA,
		MT7961_DMASHDL_GROUP_1_MAX_QUOTA,
		MT7961_DMASHDL_GROUP_2_MAX_QUOTA,
		MT7961_DMASHDL_GROUP_3_MAX_QUOTA,
		MT7961_DMASHDL_GROUP_4_MAX_QUOTA,
		MT7961_DMASHDL_GROUP_5_MAX_QUOTA,
		MT7961_DMASHDL_GROUP_6_MAX_QUOTA,
		MT7961_DMASHDL_GROUP_7_MAX_QUOTA,
		MT7961_DMASHDL_GROUP_8_MAX_QUOTA,
		MT7961_DMASHDL_GROUP_9_MAX_QUOTA,
		MT7961_DMASHDL_GROUP_10_MAX_QUOTA,
		MT7961_DMASHDL_GROUP_11_MAX_QUOTA,
		MT7961_DMASHDL_GROUP_12_MAX_QUOTA,
		MT7961_DMASHDL_GROUP_13_MAX_QUOTA,
		MT7961_DMASHDL_GROUP_14_MAX_QUOTA,
		MT7961_DMASHDL_GROUP_15_MAX_QUOTA,
	},

	.au2MinQuota = {
		MT7961_DMASHDL_GROUP_0_MIN_QUOTA,
		MT7961_DMASHDL_GROUP_1_MIN_QUOTA,
		MT7961_DMASHDL_GROUP_2_MIN_QUOTA,
		MT7961_DMASHDL_GROUP_3_MIN_QUOTA,
		MT7961_DMASHDL_GROUP_4_MIN_QUOTA,
		MT7961_DMASHDL_GROUP_5_MIN_QUOTA,
		MT7961_DMASHDL_GROUP_6_MIN_QUOTA,
		MT7961_DMASHDL_GROUP_7_MIN_QUOTA,
		MT7961_DMASHDL_GROUP_8_MIN_QUOTA,
		MT7961_DMASHDL_GROUP_9_MIN_QUOTA,
		MT7961_DMASHDL_GROUP_10_MIN_QUOTA,
		MT7961_DMASHDL_GROUP_11_MIN_QUOTA,
		MT7961_DMASHDL_GROUP_12_MIN_QUOTA,
		MT7961_DMASHDL_GROUP_13_MIN_QUOTA,
		MT7961_DMASHDL_GROUP_14_MIN_QUOTA,
		MT7961_DMASHDL_GROUP_15_MIN_QUOTA,
	},

	.aucQueue2Group = {
		MT7961_DMASHDL_QUEUE_0_TO_GROUP,
		MT7961_DMASHDL_QUEUE_1_TO_GROUP,
		MT7961_DMASHDL_QUEUE_2_TO_GROUP,
		MT7961_DMASHDL_QUEUE_3_TO_GROUP,
		MT7961_DMASHDL_QUEUE_4_TO_GROUP,
		MT7961_DMASHDL_QUEUE_5_TO_GROUP,
		MT7961_DMASHDL_QUEUE_6_TO_GROUP,
		MT7961_DMASHDL_QUEUE_7_TO_GROUP,
		MT7961_DMASHDL_QUEUE_8_TO_GROUP,
		MT7961_DMASHDL_QUEUE_9_TO_GROUP,
		MT7961_DMASHDL_QUEUE_10_TO_GROUP,
		MT7961_DMASHDL_QUEUE_11_TO_GROUP,
		MT7961_DMASHDL_QUEUE_12_TO_GROUP,
		MT7961_DMASHDL_QUEUE_13_TO_GROUP,
		MT7961_DMASHDL_QUEUE_14_TO_GROUP,
		MT7961_DMASHDL_QUEUE_15_TO_GROUP,
		MT7961_DMASHDL_QUEUE_16_TO_GROUP,
		MT7961_DMASHDL_QUEUE_17_TO_GROUP,
		MT7961_DMASHDL_QUEUE_18_TO_GROUP,
		MT7961_DMASHDL_QUEUE_19_TO_GROUP,
		MT7961_DMASHDL_QUEUE_20_TO_GROUP,
		MT7961_DMASHDL_QUEUE_21_TO_GROUP,
		MT7961_DMASHDL_QUEUE_22_TO_GROUP,
		MT7961_DMASHDL_QUEUE_23_TO_GROUP,
		MT7961_DMASHDL_QUEUE_24_TO_GROUP,
		MT7961_DMASHDL_QUEUE_25_TO_GROUP,
		MT7961_DMASHDL_QUEUE_26_TO_GROUP,
		MT7961_DMASHDL_QUEUE_27_TO_GROUP,
		MT7961_DMASHDL_QUEUE_28_TO_GROUP,
		MT7961_DMASHDL_QUEUE_29_TO_GROUP,
		MT7961_DMASHDL_QUEUE_30_TO_GROUP,
		MT7961_DMASHDL_QUEUE_31_TO_GROUP,
	},

	.aucPriority2Group = {
		MT7961_DMASHDL_PRIORITY0_GROUP,
		MT7961_DMASHDL_PRIORITY1_GROUP,
		MT7961_DMASHDL_PRIORITY2_GROUP,
		MT7961_DMASHDL_PRIORITY3_GROUP,
		MT7961_DMASHDL_PRIORITY4_GROUP,
		MT7961_DMASHDL_PRIORITY5_GROUP,
		MT7961_DMASHDL_PRIORITY6_GROUP,
		MT7961_DMASHDL_PRIORITY7_GROUP,
		MT7961_DMASHDL_PRIORITY8_GROUP,
		MT7961_DMASHDL_PRIORITY9_GROUP,
		MT7961_DMASHDL_PRIORITY10_GROUP,
		MT7961_DMASHDL_PRIORITY11_GROUP,
		MT7961_DMASHDL_PRIORITY12_GROUP,
		MT7961_DMASHDL_PRIORITY13_GROUP,
		MT7961_DMASHDL_PRIORITY14_GROUP,
		MT7961_DMASHDL_PRIORITY15_GROUP,
	},
};

/*----------------------------------------------------------------------------*/
/*!
 * \brief Set DMASHDL PLE_PACKET_MAX_SIZE
 *
 * \param prAdapter      Pointer of Adapter Data Structure
 *        u2MaxPage      Intended value for PLE_PACKET_MAX_SIZE
 *
 * \return (none)
 */
/*----------------------------------------------------------------------------*/
void mt7961HalDmashdlSetPlePktMaxPage(struct ADAPTER *prAdapter,
				      uint16_t u2MaxPage)
{
	uint32_t u4Val = 0;

	HAL_MCR_RD(prAdapter, WF_HIF_DMASHDL_TOP_PACKET_MAX_SIZE_ADDR, &u4Val);

	u4Val &= ~WF_HIF_DMASHDL_TOP_PACKET_MAX_SIZE_PLE_PACKET_MAX_SIZE_MASK;
	u4Val |= (u2MaxPage <<
		  WF_HIF_DMASHDL_TOP_PACKET_MAX_SIZE_PLE_PACKET_MAX_SIZE_SHFT) &
		 WF_HIF_DMASHDL_TOP_PACKET_MAX_SIZE_PLE_PACKET_MAX_SIZE_MASK;

	HAL_MCR_WR(prAdapter, WF_HIF_DMASHDL_TOP_PACKET_MAX_SIZE_ADDR, u4Val);
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief Set DMASHDL PSE_PACKET_MAX_SIZE
 *
 * \param prAdapter      Pointer of Adapter Data Structure
 *        u2MaxPage      Intended value for PSE_PACKET_MAX_SIZE
 *
 * \return (none)
 */
/*----------------------------------------------------------------------------*/
void mt7961HalDmashdlSetPsePktMaxPage(struct ADAPTER *prAdapter,
				      uint16_t u2MaxPage)
{
	uint32_t u4Val = 0;

	HAL_MCR_RD(prAdapter, WF_HIF_DMASHDL_TOP_PACKET_MAX_SIZE_ADDR, &u4Val);

	u4Val &= ~WF_HIF_DMASHDL_TOP_PACKET_MAX_SIZE_PSE_PACKET_MAX_SIZE_MASK;
	u4Val |= (u2MaxPage <<
		  WF_HIF_DMASHDL_TOP_PACKET_MAX_SIZE_PSE_PACKET_MAX_SIZE_SHFT) &
		 WF_HIF_DMASHDL_TOP_PACKET_MAX_SIZE_PSE_PACKET_MAX_SIZE_MASK;

	HAL_MCR_WR(prAdapter, WF_HIF_DMASHDL_TOP_PACKET_MAX_SIZE_ADDR, u4Val);
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief Set DMASHDL PLE_PACKET_MAX_SIZE and PSE_PACKET_MAX_SIZE
 *
 * \param prAdapter      Pointer of Adapter Data Structure
 *        u2PleMaxPage   Intended value for PLE_PACKET_MAX_SIZE
 *        u2PseMaxPage   Intended value for PSE_PACKET_MAX_SIZE
 *
 * \return (none)
 */
/*----------------------------------------------------------------------------*/
void mt7961HalDmashdlSetPktMaxPage(struct ADAPTER *prAdapter,
				   uint16_t u2PleMaxPage,
				   uint16_t u2PseMaxPage)
{
	uint32_t u4Val = 0;

	/* The implementation here tends to reduce IO control and hence no
	 * reuse of mt7961HalDmashdlSetPlePktMaxPage and
	 * mt7961HalDmashdlSetPsePktMaxPage is applied.
	 */
	u4Val |= (u2PleMaxPage <<
		  WF_HIF_DMASHDL_TOP_PACKET_MAX_SIZE_PLE_PACKET_MAX_SIZE_SHFT) &
		 WF_HIF_DMASHDL_TOP_PACKET_MAX_SIZE_PLE_PACKET_MAX_SIZE_MASK;
	u4Val |= (u2PseMaxPage <<
		  WF_HIF_DMASHDL_TOP_PACKET_MAX_SIZE_PSE_PACKET_MAX_SIZE_SHFT) &
		 WF_HIF_DMASHDL_TOP_PACKET_MAX_SIZE_PSE_PACKET_MAX_SIZE_MASK;

	HAL_MCR_WR(prAdapter, WF_HIF_DMASHDL_TOP_PACKET_MAX_SIZE_ADDR, u4Val);
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief Set DMASHDL refill of a specific group.
 *
 * \param prAdapter      Pointer of Adapter Data Structure
 *        ucGroup        Target group
 *        fgEnable       TRUE for refill enable and FALSE for refill disable
 *
 * \return (none)
 */
/*----------------------------------------------------------------------------*/
void mt7961HalDmashdlSetRefill(struct ADAPTER *prAdapter, uint8_t ucGroup,
			       u_int8_t fgEnable)
{
	uint32_t u4Val = 0, u4Mask = 0;

	if (ucGroup >= ENUM_MT7961_DMASHDL_GROUP_NUM)
		ASSERT(0);

	u4Mask = WF_HIF_DMASHDL_TOP_REFILL_CONTROL_GROUP0_REFILL_DISABLE_MASK
		<< ucGroup;

	HAL_MCR_RD(prAdapter, WF_HIF_DMASHDL_TOP_REFILL_CONTROL_ADDR, &u4Val);

	if (fgEnable)
		u4Val &= ~u4Mask;
	else
		u4Val |= u4Mask;

	HAL_MCR_WR(prAdapter, WF_HIF_DMASHDL_TOP_REFILL_CONTROL_ADDR, u4Val);
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief Set DMASHDL refill of all groups
 *
 * \param prAdapter      Pointer of Adapter Data Structure
 *        pfgEnable      Pointer to the beginning of the array of group refill
 *
 * \return (none)
 */
/*----------------------------------------------------------------------------*/
void mt7961HalDmashdlSetAllRefill(struct ADAPTER *prAdapter,
				  u_int8_t *pfgEnable)
{
	uint32_t u4Group = 0, u4Val = 0, u4Mask = 0;

	/* The implementation here tends to reduce IO control and hence no
	 * reuse of mt7961HalDmashdlSetRefill is applied.
	 */
	HAL_MCR_RD(prAdapter, WF_HIF_DMASHDL_TOP_REFILL_CONTROL_ADDR, &u4Val);

	for (u4Group = 0; u4Group < ENUM_MT7961_DMASHDL_GROUP_NUM; u4Group++) {
		u4Mask =
		    WF_HIF_DMASHDL_TOP_REFILL_CONTROL_GROUP0_REFILL_DISABLE_MASK
		    << u4Group;

		if (pfgEnable[u4Group])
			u4Val &= ~u4Mask;
		else
			u4Val |= u4Mask;
	}

	HAL_MCR_WR(prAdapter, WF_HIF_DMASHDL_TOP_REFILL_CONTROL_ADDR, u4Val);
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief Set DMASHDL max quota of a specific group
 *
 * \param prAdapter      Pointer of Adapter Data Structure
 *        ucGroup        Target group
 *        u2MaxQuota     Max quota
 *
 * \return (none)
 */
/*----------------------------------------------------------------------------*/
void mt7961HalDmashdlSetMaxQuota(struct ADAPTER *prAdapter, uint8_t ucGroup,
				 uint16_t u2MaxQuota)
{
	uint32_t u4Addr = 0, u4Val = 0;

	if (ucGroup >= ENUM_MT7961_DMASHDL_GROUP_NUM)
		ASSERT(0);

	u4Addr = WF_HIF_DMASHDL_TOP_GROUP0_CONTROL_ADDR + (ucGroup << 2);

	HAL_MCR_RD(prAdapter, u4Addr, &u4Val);

	u4Val &= ~WF_HIF_DMASHDL_TOP_GROUP0_CONTROL_GROUP0_MAX_QUOTA_MASK;
	u4Val |= (u2MaxQuota <<
		  WF_HIF_DMASHDL_TOP_GROUP0_CONTROL_GROUP0_MAX_QUOTA_SHFT) &
		 WF_HIF_DMASHDL_TOP_GROUP0_CONTROL_GROUP0_MAX_QUOTA_MASK;

	HAL_MCR_WR(prAdapter, u4Addr, u4Val);
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief Set DMASHDL min quota of a specific group
 *
 * \param prAdapter      Pointer of Adapter Data Structure
 *        ucGroup        Target group
 *        u2MinQuota     Min quota
 *
 * \return (none)
 */
/*----------------------------------------------------------------------------*/
void mt7961HalDmashdlSetMinQuota(struct ADAPTER *prAdapter, uint8_t ucGroup,
				 uint16_t u2MinQuota)
{
	uint32_t u4Addr = 0, u4Val = 0;

	if (ucGroup >= ENUM_MT7961_DMASHDL_GROUP_NUM)
		ASSERT(0);

	u4Addr = WF_HIF_DMASHDL_TOP_GROUP0_CONTROL_ADDR + (ucGroup << 2);

	HAL_MCR_RD(prAdapter, u4Addr, &u4Val);

	u4Val &= ~WF_HIF_DMASHDL_TOP_GROUP0_CONTROL_GROUP0_MIN_QUOTA_MASK;
	u4Val |= (u2MinQuota <<
		  WF_HIF_DMASHDL_TOP_GROUP0_CONTROL_GROUP0_MIN_QUOTA_SHFT) &
		 WF_HIF_DMASHDL_TOP_GROUP0_CONTROL_GROUP0_MIN_QUOTA_MASK;

	HAL_MCR_WR(prAdapter, u4Addr, u4Val);
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief Set DMASHDL max quota and min quota of all groups
 *
 * \param prAdapter      Pointer of Adapter Data Structure
 *        pu2MaxQuota    Pointer to the beginning of the array of group max
 *                       quota
 *        pu2MinQuota    Pointer to the beginning of the array of group min
 *                       quota
 *
 * \return (none)
 */
/*----------------------------------------------------------------------------*/
void mt7961HalDmashdlSetAllQuota(struct ADAPTER *prAdapter,
				 uint16_t *pu2MaxQuota, uint16_t *pu2MinQuota)
{
	uint32_t u4Group = 0, u4Addr = 0, u4Val = 0;

	/* The implementation here tends to reduce IO control and hence no
	 * reuse of mt7961HalDmashdlSetMaxQuota and mt7961HalDmashdlSetMinQuota
	 * is applied.
	 */
	for (u4Group = 0; u4Group < ENUM_MT7961_DMASHDL_GROUP_NUM; u4Group++) {
		u4Addr = WF_HIF_DMASHDL_TOP_GROUP0_CONTROL_ADDR +
			(u4Group << 2);

		u4Val = 0;
		u4Val |= (pu2MaxQuota[u4Group] <<
		      WF_HIF_DMASHDL_TOP_GROUP0_CONTROL_GROUP0_MAX_QUOTA_SHFT) &
			WF_HIF_DMASHDL_TOP_GROUP0_CONTROL_GROUP0_MAX_QUOTA_MASK;
		u4Val |= (pu2MinQuota[u4Group] <<
		      WF_HIF_DMASHDL_TOP_GROUP0_CONTROL_GROUP0_MIN_QUOTA_SHFT) &
			WF_HIF_DMASHDL_TOP_GROUP0_CONTROL_GROUP0_MIN_QUOTA_MASK;

		HAL_MCR_WR(prAdapter, u4Addr, u4Val);
	}
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief Set DMASHDL queue-to-group mapping of a specifi queue
 *
 * \param prAdapter      Pointer of Adapter Data Structure
 *        ucQueue        Target queue
 *        ucGroup        Group
 *
 * \return (none)
 */
/*----------------------------------------------------------------------------*/
void mt7961HalDmashdlSetQueueMapping(struct ADAPTER *prAdapter, uint8_t ucQueue,
				     uint8_t ucGroup)
{
	uint32_t u4Addr = 0, u4Val = 0, u4Mask = 0, u4Shft = 0;

	if (ucQueue >= MT7961_DMASHDL_QUEUE_NUM)
		ASSERT(0);

	if (ucGroup >= ENUM_MT7961_DMASHDL_GROUP_NUM)
		ASSERT(0);

	u4Addr = WF_HIF_DMASHDL_TOP_QUEUE_MAPPING0_ADDR +
		 ((ucQueue >> 3) << 2);
	u4Mask = WF_HIF_DMASHDL_TOP_QUEUE_MAPPING0_QUEUE0_MAPPING_MASK <<
		 ((ucQueue % 8) << 2);
	u4Shft = (ucQueue % 8) << 2;

	HAL_MCR_RD(prAdapter, u4Addr, &u4Val);

	u4Val &= ~u4Mask;
	u4Val |= (ucGroup << u4Shft) & u4Mask;

	HAL_MCR_WR(prAdapter, u4Addr, u4Val);
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief Set DMASHDL queue-to-group mappings of all queues
 *
 * \param prAdapter      Pointer of Adapter Data Structure
 *        pucGroup       Pointer to the beginning of the array of queue-to-group
 *                       mappings
 *
 * \return (none)
 */
/*----------------------------------------------------------------------------*/
void mt7961HalDmashdlSetAllQueueMapping(struct ADAPTER *prAdapter,
					uint8_t *pucGroup)
{
	uint32_t u4Addr, u4Val = 0, u4Mask, u4Shft;
	uint8_t ucQueue;

	/* The implementation here tends to reduce IO control and hence no
	 * reuse of mt7961HalDmashdlSetQueueMapping is applied.
	 */
	for (ucQueue = 0; ucQueue < MT7961_DMASHDL_QUEUE_NUM; ucQueue++) {
		if (ucQueue % 8 == 0)
			u4Val = 0;

		u4Mask = WF_HIF_DMASHDL_TOP_QUEUE_MAPPING0_QUEUE0_MAPPING_MASK
			<< ((ucQueue % 8) << 2);
		u4Shft = (ucQueue % 8) << 2;

		u4Val |= (pucGroup[ucQueue] << u4Shft) & u4Mask;

		if (ucQueue % 8 == 7) {
			u4Addr = WF_HIF_DMASHDL_TOP_QUEUE_MAPPING0_ADDR +
				((ucQueue >> 3) << 2);
			HAL_MCR_WR(prAdapter, u4Addr, u4Val);
		}
	}
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief Set DMASHDL slot arbiter
 *
 * \param prAdapter      Pointer of Adapter Data Structure
 *        fgEnable       TRUE for enable and FALSE for disable
 *
 * \return (none)
 */
/*----------------------------------------------------------------------------*/
void mt7961HalDmashdlSetSlotArbiter(struct ADAPTER *prAdapter,
				    u_int8_t fgEnable)
{
	uint32_t u4Val = 0;

	HAL_MCR_RD(prAdapter, WF_HIF_DMASHDL_TOP_PAGE_SETTING_ADDR, &u4Val);

	if (fgEnable)
		u4Val |=
		 WF_HIF_DMASHDL_TOP_PAGE_SETTING_GROUP_SEQUENCE_ORDER_TYPE_MASK;
	else
		u4Val &=
		~WF_HIF_DMASHDL_TOP_PAGE_SETTING_GROUP_SEQUENCE_ORDER_TYPE_MASK;

	HAL_MCR_WR(prAdapter, WF_HIF_DMASHDL_TOP_PAGE_SETTING_ADDR, u4Val);
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief Set DMASHDL user defined priority of a specific group
 *
 * \param prAdapter      Pointer of Adapter Data Structure
 *        ucPriority     Priority
 *        ucGroup        Group
 *
 * \return (none)
 */
/*----------------------------------------------------------------------------*/
void mt7961HalDmashdlSetUserDefinedPriority(struct ADAPTER *prAdapter,
					    uint8_t ucPriority,
					    uint8_t ucGroup)
{
	uint32_t u4Addr = 0, u4Val = 0, u4Mask = 0, u4Shft = 0;

	ASSERT(ucPriority < MT7961_DMASHDL_PRIORITY_NUM);
	ASSERT(ucGroup < ENUM_MT7961_DMASHDL_GROUP_NUM);

	u4Addr = WF_HIF_DMASHDL_TOP_HIF_SCHEDULER_SETTING0_ADDR +
		((ucPriority >> 3) << 2);
	u4Mask = WF_HIF_DMASHDL_TOP_HIF_SCHEDULER_SETTING0_PRIORITY0_GROUP_MASK
		 << ((ucPriority % 8) << 2);
	u4Shft = (ucPriority % 8) << 2;

	HAL_MCR_RD(prAdapter, u4Addr, &u4Val);

	u4Val &= ~u4Mask;
	u4Val |= (ucGroup << u4Shft) & u4Mask;

	HAL_MCR_WR(prAdapter, u4Addr, u4Val);
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief Set DMASHDL user defined priorities of all groups
 *
 * \param prAdapter      Pointer of Adapter Data Structure
 *        pucGroup       Pointer to the beginning of the array of group priority
 *
 * \return (none)
 */
/*----------------------------------------------------------------------------*/
void mt7961HalDmashdlSetAllUserDefinedPriority(struct ADAPTER *prAdapter,
					       uint8_t *pucGroup)
{
	uint32_t u4Addr, u4Val = 0, u4Mask, u4Shft;
	uint8_t ucPriority;

	/* The implementation here tends to reduce IO control and hence no
	 * reuse of mt7961HalDmashdlSetUserDefinedPriority is applied.
	 */
	for (ucPriority = 0; ucPriority < MT7961_DMASHDL_PRIORITY_NUM;
	     ucPriority++) {
		if (ucPriority % 8 == 0)
			u4Val = 0;

		u4Mask =
		  WF_HIF_DMASHDL_TOP_HIF_SCHEDULER_SETTING0_PRIORITY0_GROUP_MASK
			<< ((ucPriority % 8) << 2);
		u4Shft = (ucPriority % 8) << 2;

		u4Val |= (pucGroup[ucPriority] << u4Shft) & u4Mask;

		if (ucPriority % 8 == 7) {
			u4Addr = WF_HIF_DMASHDL_TOP_HIF_SCHEDULER_SETTING0_ADDR
				+ ((ucPriority >> 3) << 2);
			HAL_MCR_WR(prAdapter, u4Addr, u4Val);
		}
	}
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief Do DMASDHL init when WIFISYS is initialized at probe, L0.5 reset, etc.
 *
 * \param prAdapter      Pointer of Adapter Data Structure
 *
 * \return (none)
 */
/*----------------------------------------------------------------------------*/
void mt7961DmashdlInit(struct ADAPTER *prAdapter)
{
	mt7961HalDmashdlSetPktMaxPage(prAdapter,
				      rMT7961DmashdlCfg.u2PktPleMaxPage,
				      rMT7961DmashdlCfg.u2PktPseMaxPage);

	mt7961HalDmashdlSetAllRefill(prAdapter, rMT7961DmashdlCfg.afgRefillEn);

	mt7961HalDmashdlSetAllQuota(prAdapter, rMT7961DmashdlCfg.au2MaxQuota,
				    rMT7961DmashdlCfg.au2MinQuota);

	mt7961HalDmashdlSetAllQueueMapping(prAdapter,
					   rMT7961DmashdlCfg.aucQueue2Group);

	mt7961HalDmashdlSetAllUserDefinedPriority(prAdapter,
					   rMT7961DmashdlCfg.aucPriority2Group);

	mt7961HalDmashdlSetSlotArbiter(prAdapter,
				       rMT7961DmashdlCfg.fgSlotArbiterEn);
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief Although DMASHDL was init, we need to reinit it again due to falcon
 *        L1 reset, etc. The difference between mt7961DmashdlInit and
 *        mt7961DmashdlReInit is that we don't init CRs such as refill,
 *        min_quota, max_quota in mt7961DmashdlReInit, which are backup and
 *        restored in fw. The reason why some DMASHDL CRs are reinit by driver
 *        and some by fw is
 *        1. Some DMASHDL CRs shall be inited before fw releases UMAC reset
 *           in L1 procedure. Then, these CRs are backup and restored by fw.
 *        2. However, the backup and restore of each DMASHDL CR in fw needs
 *           wm DLM space. So, we save DLM space by reinit the remaining
 *           DMASHDL CRs in driver.
 *
 * \param prAdapter      Pointer of Adapter Data Structure
 *
 * \return (none)
 */
/*----------------------------------------------------------------------------*/
void mt7961DmashdlReInit(struct ADAPTER *prAdapter)
{
	mt7961HalDmashdlSetPktMaxPage(prAdapter,
				      rMT7961DmashdlCfg.u2PktPleMaxPage,
				      rMT7961DmashdlCfg.u2PktPseMaxPage);

	mt7961HalDmashdlSetAllQueueMapping(prAdapter,
					   rMT7961DmashdlCfg.aucQueue2Group);

	mt7961HalDmashdlSetAllUserDefinedPriority(prAdapter,
					   rMT7961DmashdlCfg.aucPriority2Group);

	mt7961HalDmashdlSetSlotArbiter(prAdapter,
				       rMT7961DmashdlCfg.fgSlotArbiterEn);
}

uint32_t mt7961UpdateDmashdlQuota (struct ADAPTER *prAdapter,
					    uint8_t ucWmmIndex, uint32_t u4MaxQuota)
{
	uint8_t ucGroupIdx, ucAcIdx;
	uint32_t idx;
	uint16_t u2MaxQuotaFinal;
	bool fgIsMaxQuotaInvalid = FALSE;

	ASSERT(prAdapter);
	if (u4MaxQuota > (DMASHDL_MAX_QUOTA_MASK >> DMASHDL_MAX_QUOTA_OFFSET))
		fgIsMaxQuotaInvalid = TRUE;

	for (idx= 0; idx < WMM_AC_INDEX_NUM; idx++) {
		ucAcIdx = idx + (ucWmmIndex * WMM_AC_INDEX_NUM);
		ucGroupIdx = rMT7961DmashdlCfg.aucQueue2Group[ucAcIdx];
		u2MaxQuotaFinal = u4MaxQuota;
		if (fgIsMaxQuotaInvalid){
			/* Set quota to default */
			u2MaxQuotaFinal = rMT7961DmashdlCfg.au2MaxQuota[ucGroupIdx];
		}

		if (u2MaxQuotaFinal) {
			DBGLOG(HAL, INFO,
				"ucWmmIndex,%u,ucGroupIdx,%u,u2MaxQuotaFinal,0x%x\n",
				ucWmmIndex, ucGroupIdx, u2MaxQuotaFinal);
			mt7961HalDmashdlSetMaxQuota(prAdapter, ucGroupIdx, u2MaxQuotaFinal);
		}
	}
	return WLAN_STATUS_SUCCESS;
}

uint32_t mt7961dmashdlQuotaDecision (struct ADAPTER *prAdapter,
			uint8_t ucWmmIndex)
{
	struct BSS_INFO *prBssInfo;
	uint8_t ucBssIndex;
	uint16_t u2MaxQuota = 0;
#if (CFG_SUPPORT_WIFI_6G == 1)
	u_int8_t fgIs5g = FALSE, fgIs6g = FALSE;
#endif
	enum ENUM_BAND eTargetBand = BAND_NULL;

	for (ucBssIndex = 0;
		ucBssIndex < prAdapter->ucHwBssIdNum; ucBssIndex++) {

		prBssInfo = prAdapter->aprBssInfo[ucBssIndex];

		if (IS_BSS_NOT_ALIVE(prAdapter, prBssInfo))
			continue;

		if (prBssInfo->eBand != BAND_2G4
		    && prBssInfo->eBand != BAND_5G
#if (CFG_SUPPORT_WIFI_6G == 1)
		    && prBssInfo->eBand != BAND_6G
#endif
		    )
			continue;

#if (CFG_SUPPORT_WIFI_6G == 1)
		if (prBssInfo->eBand == BAND_6G)
			fgIs6g = TRUE;
		else if (prBssInfo->eBand == BAND_5G)
			fgIs5g = TRUE;
#endif
		if (prBssInfo->ucWmmQueSet == ucWmmIndex)
			eTargetBand = prBssInfo->eBand;

	}

	if (eTargetBand != BAND_NULL) {
		if (eTargetBand == BAND_2G4) /* for 2G in case 2+6 or 2+5 */
			u2MaxQuota = MT7961_DMASHDL_DBDC_2G_MAX_QUOTA;
		else /* for 5G and 6G in case 2+6 or 2+5 */
			u2MaxQuota = MT7961_DMASHDL_DBDC_5G_MAX_QUOTA;
#if (CFG_SUPPORT_WIFI_6G == 1)
		if (fgIs6g && fgIs5g) /* for 5+6 case */
			u2MaxQuota = MT7961_DMASHDL_DBDC_5G_6G_MAX_QUOTA;
#endif
	}

	return u2MaxQuota;
}

#endif /* defined(_HIF_PCIE) || defined(_HIF_AXI) || defined(_HIF_USB) */
#endif /* defined(MT7961) || defined(MT7922) */
