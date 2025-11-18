/* SPDX-License-Identifier: ISC */
/* Copyright (C) 2020 MediaTek Inc. */

#ifndef __MT7902_REGS_H
#define __MT7902_REGS_H

#include "mt792x_regs.h"

// struct __map {
// 	u32 phys;
// 	u32 mapped;
// 	u32 size;
// };

// struct __base {
// 	u32 band_base[__MT_MAX_BAND];
// };

/* used to differentiate between generations */
// struct mt7902_reg_desc {
// 	const struct __base *base;
// 	const struct __map *map;
// 	u32 map_size;
// };

enum base_rev {
	WF_AGG_BASE,
	WF_MIB_BASE,
	WF_TMAC_BASE,
	WF_RMAC_BASE,
	WF_ARB_BASE,
	WF_LPON_BASE,
	WF_ETBF_BASE,
	WF_DMA_BASE,
	__MT_REG_BASE_MAX,
};


// #define __BASE(_id, _band)		(dev->reg.base[(_id)].band_base[(_band)])


#define MT_MDP_BASE			0x820cd000
#define MT_MDP(ofs)			(MT_MDP_BASE + (ofs))

#define MT_MDP_DCR0			MT_MDP(0x000)
#define MT_MDP_DCR0_DAMSDU_EN		BIT(15)
#define MT_MDP_DCR0_RX_HDR_TRANS_EN	BIT(19)

#define MT_MDP_DCR1			MT_MDP(0x004)
#define MT_MDP_DCR1_MAX_RX_LEN		GENMASK(15, 3)

#define MT_MDP_BNRCFR0(_band)		MT_MDP(0x070 + ((_band) << 8))
#define MT_MDP_RCFR0_MCU_RX_MGMT	GENMASK(5, 4)
#define MT_MDP_RCFR0_MCU_RX_CTL_NON_BAR	GENMASK(7, 6)
#define MT_MDP_RCFR0_MCU_RX_CTL_BAR	GENMASK(9, 8)

#define MT_MDP_BNRCFR1(_band)		MT_MDP(0x074 + ((_band) << 8))
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

#define MT_HIF_REMAP_L1			MT_INFRA(0x24c)
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


/* TRB: band 0(0x820e1000), band 1(0x820f1000) */
#define MT_WF_TRB_BASE(_band)		((_band) ? 0x820f1000 : 0x820e1000)
#define MT_WF_TRB(_band, ofs)		(MT_WF_TRB_BASE(_band) + (ofs))

#define MT_TRB_RXPSR0(_band)		MT_WF_TRB(_band, 0x03c)
#define MT_TRB_RXPSR0_RX_WTBL_PTR	GENMASK(25, 16)
#define MT_TRB_RXPSR0_RX_RMAC_PTR	GENMASK(9, 0)

// /* TMAC: band 0(0x820e4000), band 1(0x820f4000), band 3(0x830e4000) */
#define MT_WF_TMAC_BASE(_band)		0x820e4000 //__BASE(WF_TMAC_BASE, (_band))
#define MT_WF_TMAC(_band, ofs)		(MT_WF_TMAC_BASE(_band) + (ofs))

// #define MT_TMAC_TCR0(_band)		MT_WF_TMAC(_band, 0)
// #define MT_TMAC_TCR0_TX_BLINK		GENMASK(7, 6)
// #define MT_TMAC_TCR0_TBTT_STOP_CTRL	BIT(25)

// #define MT_TMAC_CDTR(_band)		MT_WF_TMAC(_band, 0x0c8)
// #define MT_TMAC_ODTR(_band)		MT_WF_TMAC(_band, 0x0cc)
// #define MT_TIMEOUT_VAL_PLCP		GENMASK(15, 0)
// #define MT_TIMEOUT_VAL_CCA		GENMASK(31, 16)

// #define MT_TMAC_ATCR(_band)		MT_WF_TMAC(_band, 0x00c)
// #define MT_TMAC_ATCR_TXV_TOUT		GENMASK(7, 0)

// #define MT_TMAC_TRCR0(_band)		MT_WF_TMAC(_band, 0x010)
// #define MT_TMAC_TRCR0_TR2T_CHK		GENMASK(8, 0)
// #define MT_TMAC_TRCR0_I2T_CHK		GENMASK(24, 16)

// #define MT_TMAC_ICR0(_band)		MT_WF_TMAC(_band, 0x014)
#define MT_IFS_EIFS_OFDM		GENMASK(8, 0)
// #define MT_IFS_RIFS			GENMASK(14, 10)
// #define MT_IFS_SIFS			GENMASK(22, 16)
// #define MT_IFS_SLOT			GENMASK(30, 24)

#define MT_TMAC_ICR1(_band)		MT_WF_TMAC(_band, 0x018)
#define MT_IFS_EIFS_CCK			GENMASK(8, 0)

// #define MT_TMAC_CTCR0(_band)		MT_WF_TMAC(_band, 0x114)
// #define MT_TMAC_CTCR0_INS_DDLMT_REFTIME		GENMASK(5, 0)
// #define MT_TMAC_CTCR0_INS_DDLMT_EN		BIT(17)
// #define MT_TMAC_CTCR0_INS_DDLMT_VHT_SMPDU_EN	BIT(18)

// #define MT_TMAC_TFCR0(_band)		MT_WF_TMAC(_band, 0x0e4)




// /* ARB: band 0(0x820e3000), band 1(0x820f3000), band 3(0x830e3000) */
// #define MT_WF_ARB_BASE(_band)		__BASE(WF_ARB_BASE, (_band))
// #define MT_WF_ARB(_band, ofs)		(MT_WF_ARB_BASE(_band) + (ofs))

// #define MT_ARB_SCR(_band)		MT_WF_ARB(_band, 0x000)
// #define MT_ARB_SCR_TX_DISABLE		BIT(8)
// #define MT_ARB_SCR_RX_DISABLE		BIT(9)

// #define MT_ARB_DRNGR0(_band, _n)	MT_WF_ARB(_band, (0x1e0 + (_n) * 4))



#endif
