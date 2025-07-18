/* SPDX-License-Identifier: ISC */
/* Copyright (C) 2020 MediaTek Inc. */

#ifndef __MT7902_REGS_H
#define __MT7902_REGS_H

#include "../mt792x_regs.h"

/* TMAC: band 0(0x820e4000), band 1(0x820f4000), band 3(0x830e4000) */
/* WF DMA TOP: band 0(0x820e7000),band 1(0x820f7000),band 3(0x830e7000) */
/* ETBF: band 0(0x820ea000), band 1(0x820fa000), band 3(0x830ea000) */
/* LPON: band 0(0x820eb000), band 1(0x820fb000), band 3(0x830eb000) */
/* MIB: band 0(0x820ed000), band 1(0x820fd000) band 3(0x830ed000)*/
/* These counters are (mostly?) clear-on-read.  So, some should not
 * be read at all in case firmware is already reading them.  These
 * are commented with 'DNR' below.  The DNR stats will be read by querying
 * the firmware API for the appropriate message.  For counters the driver
 * does read, the driver should accumulate the counters.
 */

#define MT_MDP_BASE			0x820cc000
#define MT_MDP(ofs)			(MT_MDP_BASE + (ofs))

#define MT_MDP_DCR0			MT_MDP(0x000)
#define MT_MDP_DCR0_DAMSDU_EN		BIT(15)
#define MT_MDP_DCR0_RX_HDR_TRANS_EN	BIT(19)

#define MT_MDP_DCR1			MT_MDP(0x004)
#define MT_MDP_DCR1_MAX_RX_LEN		GENMASK(15, 3)

#define MT_MDP_BNRCFR0(_band)		MT_MDP(0x090 + ((_band) << 8))
#define MT_MDP_RCFR0_MCU_RX_MGMT	GENMASK(5, 4)
#define MT_MDP_RCFR0_MCU_RX_CTL_NON_BAR	GENMASK(7, 6)
#define MT_MDP_RCFR0_MCU_RX_CTL_BAR	GENMASK(9, 8)

#define MT_MDP_BNRCFR1(_band)		MT_MDP(0x094 + ((_band) << 8))
#define MT_MDP_RCFR1_MCU_RX_BYPASS	GENMASK(23, 22)
#define MT_MDP_RCFR1_RX_DROPPED_UCAST	GENMASK(28, 27)
#define MT_MDP_RCFR1_RX_DROPPED_MCAST	GENMASK(30, 29)
#define MT_MDP_TO_HIF			0
#define MT_MDP_TO_WM			1

#define MT_WFDMA0_HOST_INT_ENA		MT_WFDMA0(0x204)
#define HOST_TX_DONE_INT_ENA8		BIT(12)
#define HOST_TX_DONE_INT_ENA9		BIT(13)
#define HOST_TX_DONE_INT_ENA10		BIT(14)
#define HOST_TX_DONE_INT_ENA11		BIT(15)
#define HOST_TX_DONE_INT_ENA12		BIT(16)
#define HOST_TX_DONE_INT_ENA13		BIT(17)
#define HOST_TX_DONE_INT_ENA14		BIT(18)
#define HOST_RX_DONE_INT_ENA4		BIT(22)
#define HOST_RX_DONE_INT_ENA5		BIT(23)
#define HOST_TX_DONE_INT_ENA16		BIT(26)
#define HOST_TX_DONE_INT_ENA17		BIT(27)

/* WFDMA interrupt */
#define MT_INT_RX_DONE_DATA		HOST_RX_DONE_INT_ENA2
#define MT_INT_RX_DONE_WM		HOST_RX_DONE_INT_ENA0
#define MT_INT_RX_DONE_WM2		HOST_RX_DONE_INT_ENA4
#define MT_INT_RX_DONE_ALL		(MT_INT_RX_DONE_DATA | \
					 MT_INT_RX_DONE_WM | \
					 MT_INT_RX_DONE_WM2)
#define MT_INT_TX_DONE_MCU_WM		HOST_TX_DONE_INT_ENA17
#define MT_INT_TX_DONE_FWDL		HOST_TX_DONE_INT_ENA16
#define MT_INT_TX_DONE_BAND0		HOST_TX_DONE_INT_ENA0

#define MT_INT_TX_DONE_MCU		(MT_INT_TX_DONE_MCU_WM |	\
					 MT_INT_TX_DONE_FWDL)
#define MT_INT_TX_DONE_ALL		(MT_INT_TX_DONE_MCU_WM |	\
					 MT_INT_TX_DONE_BAND0 |	\
					GENMASK(18, 4))

#define MT_RX_DATA_RING_BASE		MT_WFDMA0(0x520)

#define MT_INFRA_CFG_BASE		0xfe000
#define MT_INFRA(ofs)			(MT_INFRA_CFG_BASE + (ofs))

#define MT_HIF_REMAP_L1			0xfe418
#define MT_HIF_REMAP_L1_MASK		GENMASK(15, 0)
#define MT_HIF_REMAP_L1_OFFSET		GENMASK(15, 0)
#define MT_HIF_REMAP_L1_BASE		GENMASK(31, 16)
#define MT_HIF_REMAP_BASE_L1		0x40000

#define MT_WFSYS_SW_RST_B		0x18000140

#define MT_WTBLON_TOP_WDUCR		MT_WTBLON_TOP(0x200)
#define MT_WTBLON_TOP_WDUCR_GROUP	GENMASK(2, 0)

#define MT_WTBL_UPDATE			MT_WTBLON_TOP(0x230)
#define MT_WTBL_UPDATE_WLAN_IDX		GENMASK(9, 0)
#define MT_WTBL_UPDATE_ADM_COUNT_CLEAR	BIT(12)

#endif
