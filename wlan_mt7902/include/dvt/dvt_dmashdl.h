/*******************************************************************************
 *
 * This file is provided under a dual license.  When you use or
 * distribute this software, you may choose to be licensed under
 * version 2 of the GNU General Public License ("GPLv2 License")
 * or BSD License.
 *
 * GPLv2 License
 *
 * Copyright(C) 2016 MediaTek Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See http://www.gnu.org/licenses/gpl-2.0.html for more details.
 *
 * BSD LICENSE
 *
 * Copyright(C) 2016 MediaTek Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *  * Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 ******************************************************************************/

/*****************************************************************************
 *============================================================================
 *                               HISTORY
 * Log
 *
 * 10 05 2018 Kai.lin
 * commit id bece10b96739cf7ca196930c313eab8ddcef45b3
 * Porting from 7668
 *
 ****************************************************************************/


#ifndef __DVT_DMASHDL_H__
#define __DVT_DMASHDL_H__

#if (CFG_SUPPORT_DMASHDL_SYSDVT)
/*******************************************************************************
 *                              C O N S T A N T S
 *******************************************************************************
 */
#define WF_HIF_DMASHDL_TOP_BASE		(0x7C026000)
#define WF_HIF_DMASHDL_TOP_WACPU_REFILL_ADDR          \
			(WF_HIF_DMASHDL_TOP_BASE + 0x00) /* 6000 */
#define WF_HIF_DMASHDL_TOP_SW_CONTROL_ADDR            \
			(WF_HIF_DMASHDL_TOP_BASE + 0x04) /* 6004 */
#define WF_HIF_DMASHDL_TOP_OPTIONAL_CONTROL_ADDR      \
			(WF_HIF_DMASHDL_TOP_BASE + 0x08) /* 6008 */
#define WF_HIF_DMASHDL_TOP_PAGE_SETTING_ADDR          \
			(WF_HIF_DMASHDL_TOP_BASE + 0x0C) /* 600C */
#define WF_HIF_DMASHDL_TOP_REFILL_CONTROL_ADDR        \
			(WF_HIF_DMASHDL_TOP_BASE + 0x10) /* 6010 */
#define WF_HIF_DMASHDL_TOP_CONTROL_SIGNAL_ADDR        \
			(WF_HIF_DMASHDL_TOP_BASE + 0x18) /* 6018 */
#define WF_HIF_DMASHDL_TOP_PACKET_MAX_SIZE_ADDR       \
			(WF_HIF_DMASHDL_TOP_BASE + 0x1c) /* 601C */
#define WF_HIF_DMASHDL_TOP_GROUP0_CONTROL_ADDR        \
			(WF_HIF_DMASHDL_TOP_BASE + 0x20) /* 6020 */
#define WF_HIF_DMASHDL_TOP_GROUP1_CONTROL_ADDR        \
			(WF_HIF_DMASHDL_TOP_BASE + 0x24) /* 6024 */
#define WF_HIF_DMASHDL_TOP_GROUP2_CONTROL_ADDR        \
			(WF_HIF_DMASHDL_TOP_BASE + 0x28) /* 6028 */
#define WF_HIF_DMASHDL_TOP_GROUP3_CONTROL_ADDR        \
			(WF_HIF_DMASHDL_TOP_BASE + 0x2C) /* 602C */
#define WF_HIF_DMASHDL_TOP_GROUP4_CONTROL_ADDR        \
			(WF_HIF_DMASHDL_TOP_BASE + 0x30) /* 6030 */
#define WF_HIF_DMASHDL_TOP_GROUP15_CONTROL_ADDR       \
			(WF_HIF_DMASHDL_TOP_BASE + 0x5C) /* 605C */
#define WF_HIF_DMASHDL_TOP_HIF_SCHEDULER_SETTING0_ADDR\
			(WF_HIF_DMASHDL_TOP_BASE + 0xB0) /* 60B0 */
#define WF_HIF_DMASHDL_TOP_HIF_SCHEDULER_SETTING1_ADDR\
			(WF_HIF_DMASHDL_TOP_BASE + 0xB4) /* 60B4 */
#define WF_HIF_DMASHDL_TOP_QUEUE_MAPPING0_ADDR        \
			(WF_HIF_DMASHDL_TOP_BASE + 0xd0) /* 60D0 */
#define WF_HIF_DMASHDL_TOP_QUEUE_MAPPING1_ADDR        \
			(WF_HIF_DMASHDL_TOP_BASE + 0xd4) /* 60D4 */
#define WF_HIF_DMASHDL_TOP_QUEUE_MAPPING2_ADDR        \
			(WF_HIF_DMASHDL_TOP_BASE + 0xd8) /* 60D8 */
#define WF_HIF_DMASHDL_TOP_QUEUE_MAPPING3_ADDR        \
			(WF_HIF_DMASHDL_TOP_BASE + 0xdc) /* 60DC */
#define WF_HIF_DMASHDL_TOP_STATUS_RD_ADDR             \
			(WF_HIF_DMASHDL_TOP_BASE + 0x100) /* 6100 */
#define WF_HIF_DMASHDL_TOP_STATUS_RD_GP0_ADDR         \
			(WF_HIF_DMASHDL_TOP_BASE + 0x140) /* 6140 */
#define WF_HIF_DMASHDL_TOP_STATUS_RD_GP15_ADDR        \
			(WF_HIF_DMASHDL_TOP_BASE + 0x17C) /* 617C */
#define WF_HIF_DMASHDL_TOP_RD_GROUP_PKT_CNT0_ADDR     \
			(WF_HIF_DMASHDL_TOP_BASE + 0x180) /* 6180 */
#define WF_HIF_DMASHDL_TOP_CPU_QUOTA_SET_ADDR         \
			(WF_HIF_DMASHDL_TOP_BASE + 0x98) /* 6098 */
#define WF_HIF_DMASHDL_TOP_RD_GROUP_PKT_CNT7_ADDR     \
			(WF_HIF_DMASHDL_TOP_BASE + 0x19c) /* 619C */

#define WF_HIF_DMASHDL_TOP_OPTIONAL_CONTROL_CR_HIF_ACK_CNT_TH_MASK  \
	0x00FF0000                /* CR_HIF_ACK_CNT_TH[23..16] */
#define WF_HIF_DMASHDL_TOP_OPTIONAL_CONTROL_CR_HIF_ACK_CNT_TH_SHFT  \
			16
#define WF_HIF_DMASHDL_TOP_OPTIONAL_CONTROL_CR_HIF_GUP_ACT_MAP_MASK \
	0x0000FFFF                /* CR_HIF_GUP_ACT_MAP[15..0] */
#define WF_HIF_DMASHDL_TOP_OPTIONAL_CONTROL_CR_HIF_GUP_ACT_MAP_SHFT \
			0
#define WF_HIF_DMASHDL_TOP_PAGE_SETTING_SRC_CNT_PRI_EN_MASK         \
			0x00100000
#define WF_HIF_DMASHDL_TOP_PAGE_SETTING_GROUP_SEQUENCE_ORDER_TYPE_MASK \
	0x00010000                /* GROUP_SEQUENCE_ORDER_TYPE[16] */
#define WF_HIF_DMASHDL_TOP_CONTROL_SIGNAL_CR_WACPU_MODE_EN_MASK     \
			0x00000001                /* CR_WACPU_MODE_EN[0] */
#define WF_HIF_DMASHDL_TOP_WACPU_REFILL_WACPU_EXCUTE_MASK           \
			0x80000000                /* WACPU_EXCUTE[31] */
#define WF_HIF_DMASHDL_TOP_WACPU_REFILL_WACPU_RETRUN_MODE_SHFT      \
			24
#define WF_HIF_DMASHDL_TOP_WACPU_REFILL_WACPU_RETRUN_QID_SHFT       \
			16
#define WF_HIF_DMASHDL_TOP_WACPU_REFILL_WACPU_RETURN_PAGE_CNT_SHFT  \
			0
#define WF_HIF_DMASHDL_TOP_CPU_QUOTA_SET_EXCUTE_MASK                \
			0x80000000                /* EXCUTE[31] */
#define WF_HIF_DMASHDL_TOP_CPU_QUOTA_SET_CPU_RETRUN_MODE_SHFT       \
			24
#define WF_HIF_DMASHDL_TOP_CPU_QUOTA_SET_CPU_RETRUN_GID_SHFT        \
			16
#define WF_HIF_DMASHDL_TOP_CPU_QUOTA_SET_RETURN_PAGE_CNT_SHFT       \
			0
#define WF_HIF_DMASHDL_TOP_STATUS_RD_FREE_PAGE_CNT_MASK             \
			0x0FFF0000                /* FREE_PAGE_CNT[27..16] */
#define WF_HIF_DMASHDL_TOP_STATUS_RD_FREE_PAGE_CNT_SHFT             \
			16
#define WF_HIF_DMASHDL_TOP_STATUS_RD_FFA_CNT_MASK                   \
			0x00000FFF                /* FFA_CNT[11..0] */
#define WF_HIF_DMASHDL_TOP_STATUS_RD_FFA_CNT_SHFT                   \
			0
#define WF_HIF_DMASHDL_TOP_STATUS_RD_GP0_G0_RSV_CNT_MASK            \
			0x0FFF0000                /* G0_RSV_CNT[27..16] */
#define WF_HIF_DMASHDL_TOP_STATUS_RD_GP0_G0_RSV_CNT_SHFT            \
			16
#define WF_HIF_DMASHDL_TOP_STATUS_RD_GP0_G0_SRC_CNT_MASK            \
			0x00000FFF                /* G0_SRC_CNT[11..0] */
#define WF_HIF_DMASHDL_TOP_STATUS_RD_GP0_G0_SRC_CNT_SHFT            \
			0

/*******************************************************************************
 *			M A C R O S
 *******************************************************************************
 */
/* HIF_RESET_SW_LOGIC */
#define HIF_RESET_SW_LOGIC(prGlueInfo)                             \
	{                                                              \
		kalDevRegWrite(prGlueInfo,                                 \
			WF_HIF_DMASHDL_TOP_SW_CONTROL_ADDR, 1);                \
		kalDevRegWrite(prGlueInfo,                                 \
			WF_HIF_DMASHDL_TOP_SW_CONTROL_ADDR, 0);                \
	}

#define HIF_CPU_RTN_CNT(prGlueInfo, ucGroup, ucRtnCnt)                  \
	kalDevRegWrite(prGlueInfo, WF_HIF_DMASHDL_TOP_CPU_QUOTA_SET_ADDR,   \
	WF_HIF_DMASHDL_TOP_CPU_QUOTA_SET_EXCUTE_MASK |                      \
	0x1 << WF_HIF_DMASHDL_TOP_CPU_QUOTA_SET_CPU_RETRUN_MODE_SHFT |      \
	ucGroup << WF_HIF_DMASHDL_TOP_CPU_QUOTA_SET_CPU_RETRUN_GID_SHFT |   \
	ucRtnCnt << WF_HIF_DMASHDL_TOP_CPU_QUOTA_SET_RETURN_PAGE_CNT_SHFT)

/* HIF_ADD_RSV_CNT */
#define HIF_ADD_RSV_CNT(prGlueInfo, ucQueue, ucRsvCnt)                  \
	kalDevRegWrite(prGlueInfo, WF_HIF_DMASHDL_TOP_WACPU_REFILL_ADDR,    \
	WF_HIF_DMASHDL_TOP_WACPU_REFILL_WACPU_EXCUTE_MASK |                 \
	0x1 << WF_HIF_DMASHDL_TOP_WACPU_REFILL_WACPU_RETRUN_MODE_SHFT |     \
	ucQueue << WF_HIF_DMASHDL_TOP_WACPU_REFILL_WACPU_RETRUN_QID_SHFT |  \
	ucRsvCnt << WF_HIF_DMASHDL_TOP_WACPU_REFILL_WACPU_RETURN_PAGE_CNT_SHFT)

/* HIF_SUB_SRC_CNT */
#define HIF_SUB_SRC_CNT(prGlueInfo, ucQueue, ucSrcCnt)                  \
	kalDevRegWrite(prGlueInfo, WF_HIF_DMASHDL_TOP_WACPU_REFILL_ADDR,    \
	WF_HIF_DMASHDL_TOP_WACPU_REFILL_WACPU_EXCUTE_MASK |                 \
	0x2 << WF_HIF_DMASHDL_TOP_WACPU_REFILL_WACPU_RETRUN_MODE_SHFT |     \
	ucQueue << WF_HIF_DMASHDL_TOP_WACPU_REFILL_WACPU_RETRUN_QID_SHFT |  \
	ucSrcCnt << WF_HIF_DMASHDL_TOP_WACPU_REFILL_WACPU_RETURN_PAGE_CNT_SHFT)

/* HIF_ADD_SRC_CNT */
#define HIF_ADD_SRC_CNT(prGlueInfo, ucQueue, ucSrcCnt)                  \
	kalDevRegWrite(prGlueInfo, WF_HIF_DMASHDL_TOP_WACPU_REFILL_ADDR,    \
	WF_HIF_DMASHDL_TOP_WACPU_REFILL_WACPU_EXCUTE_MASK |                 \
	0x3 << WF_HIF_DMASHDL_TOP_WACPU_REFILL_WACPU_RETRUN_MODE_SHFT |     \
	ucQueue << WF_HIF_DMASHDL_TOP_WACPU_REFILL_WACPU_RETRUN_QID_SHFT |  \
	ucSrcCnt << WF_HIF_DMASHDL_TOP_WACPU_REFILL_WACPU_RETURN_PAGE_CNT_SHFT)

#define DMASHDL_DVT_SET_ITEM(pAd, item)             \
	do {                                            \
		if (pAd->auto_dvt)                          \
			pAd->auto_dvt->dmashdl.dvt_item = item; \
	} while (0)
#define DMASHDL_DVT_GET_ITEM(pAd)                   \
	((pAd->auto_dvt)?pAd->auto_dvt->dmashdl.dvt_item:0)

#define DMASHDL_DVT_SET_SUBITEM(pAd, subitem)       \
	do {                                            \
		if (pAd->auto_dvt)                          \
			pAd->auto_dvt->dmashdl.dvt_sub_item = subitem;\
	} while (0)
#define DMASHDL_DVT_GET_SUBITEM(pAd)                \
	((pAd->auto_dvt)?pAd->auto_dvt->dmashdl.dvt_sub_item:0)

#define DMASHDL_DVT_RESET(pAd)                      \
	do {                                            \
		if (pAd->auto_dvt)                          \
			memset(&(pAd->auto_dvt->dmashdl),       \
				0, sizeof(struct DMASHDL_TEST));    \
	} while (0)

#define DMASHDL_DVT_ALLOW_PING_ONLY(pAd)                         \
	(pAd->auto_dvt &&                                            \
		(pAd->auto_dvt->dmashdl.dvt_item == DMASHDL_DVT_ITEM_1 ||\
		pAd->auto_dvt->dmashdl.dvt_item == DMASHDL_DVT_ITEM_3  ||\
		pAd->auto_dvt->dmashdl.dvt_item == DMASHDL_DVT_ITEM_4  ||\
		(pAd->auto_dvt->dmashdl.dvt_item == DMASHDL_DVT_ITEM_5 &&\
		pAd->auto_dvt->dmashdl.dvt_sub_item == DMASHDL_DVT_SUBITEM_2)\
		))
#define DMASHDL_DVT_QUEUE_MAPPING_TYPE1(pAd)                     \
	(pAd->auto_dvt &&                                            \
		(pAd->auto_dvt->dmashdl.dvt_item == DMASHDL_DVT_ITEM_1 ||\
		pAd->auto_dvt->dmashdl.dvt_item == DMASHDL_DVT_ITEM_2  ||\
		pAd->auto_dvt->dmashdl.dvt_item == DMASHDL_DVT_ITEM_3  ||\
		pAd->auto_dvt->dmashdl.dvt_item == DMASHDL_DVT_ITEM_4))
#define DMASHDL_DVT_QUEUE_MAPPING_TYPE2(pAd)                     \
	(pAd->auto_dvt &&                                            \
		(pAd->auto_dvt->dmashdl.dvt_item == DMASHDL_DVT_ITEM_5 &&\
		pAd->auto_dvt->dmashdl.dvt_sub_item == DMASHDL_DVT_SUBITEM_2))
/* No check pAd->auto_dvt because                                    */
/* must call DMASHDL_DVT_QUEUE_MAPPING_TYPEX before call those MARCO */
#define DMASHDL_DVT_GET_MAPPING_QID(pAd)                    \
	(pAd->auto_dvt->dmashdl.dvt_queue_idx)
#define DMASHDL_DVT_SET_MAPPING_QID(pAd, qidx)              \
	(pAd->auto_dvt->dmashdl.dvt_queue_idx = (qidx))
#define DMASHDL_DVT_INC_PING_PKT_CNT(pAd, qidx)             \
	(pAd->auto_dvt->dmashdl.dvt_ping_nums[qidx]++)



/*******************************************************************************
 *                             D A T A   T Y P E S
 *******************************************************************************
 */
/* ================ DMASHDL DVT ================ */
enum _ENUM_DMASHDL_DVT_ITEM_T {
	DMASHDL_DVT_RESULT = 0,
	DMASHDL_DVT_ITEM_1 = 1,
	DMASHDL_DVT_ITEM_2,
	DMASHDL_DVT_ITEM_3,
	DMASHDL_DVT_ITEM_4,
	DMASHDL_DVT_ITEM_5,
	DMASHDL_DVT_ITEM_6,
	DMASHDL_DVT_ITEM_NUM
};

enum _ENUM_DMASHDL_DVT_SUBITEM_T {
	DMASHDL_DVT_SUBITEM_1 = 1,
	DMASHDL_DVT_SUBITEM_2 = 2,
};

struct DMASHDL_DVT_CMD_T {
	uint8_t ucItemNo;
	uint8_t ucSubItemNo;
	uint8_t ucArgNo;
	uint8_t ucReserve;
};

/*******************************************************************************
 *			F U N C T I O N   D E C L A R A T I O N S
 *******************************************************************************
 */
int32_t priv_driver_dmashdl_dvt_item(
	IN struct net_device *prNetDev,
	IN char *pcCommand,
	IN int32_t i4TotalLen);
int32_t priv_driver_show_dmashdl_allcr(
	IN struct net_device *prNetDev,
	IN char *pcCommand,
	IN int i4TotalLen);
#endif
#endif /* __DVT_DMASHDL_H__ */

