/* SPDX-License-Identifier: ISC */
/* Copyright (C) 2020 MediaTek Inc. */

#ifndef __BESRA_REGS_H
#define __BESRA_REGS_H

struct __map {
	u32 phys;
	u32 mapped;
	u32 size;
};

struct __base {
	u32 band_base[__MT_MAX_BAND];
};

/* used to differentiate between generations */
struct besra_reg_desc {
	const struct __base *base;
	const struct __map *map;
	u32 map_size;
};

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

#define __BASE(_id, _band)		(dev->reg.base[(_id)].band_base[(_band)])

/* MCU WFDMA0 */
#define MT_MCU_WFDMA0_BASE		0x2000
#define MT_MCU_WFDMA0(ofs)		(MT_MCU_WFDMA0_BASE + (ofs))

#define MT_MCU_WFDMA0_DUMMY_CR		MT_MCU_WFDMA0(0x120)

/* MCU WFDMA1 */
#define MT_MCU_WFDMA1_BASE		0x3000
#define MT_MCU_WFDMA1(ofs)		(MT_MCU_WFDMA1_BASE + (ofs))

#define MT_MCU_INT_EVENT		0x2108
#define MT_MCU_INT_EVENT_DMA_STOPPED	BIT(0)
#define MT_MCU_INT_EVENT_DMA_INIT	BIT(1)
#define MT_MCU_INT_EVENT_SER_TRIGGER	BIT(2)
#define MT_MCU_INT_EVENT_RESET_DONE	BIT(3)

/* PLE */
#define MT_PLE_BASE			0x820c0000
#define MT_PLE(ofs)			(MT_PLE_BASE + (ofs))

#define MT_FL_Q_EMPTY			MT_PLE(0x360)
#define MT_FL_Q0_CTRL			MT_PLE(0x3e0)
#define MT_FL_Q2_CTRL			MT_PLE(0x3e8)
#define MT_FL_Q3_CTRL			MT_PLE(0x3ec)

#define MT_PLE_FREEPG_CNT		MT_PLE(0x380)
#define MT_PLE_FREEPG_HEAD_TAIL		MT_PLE(0x384)
#define MT_PLE_PG_HIF_GROUP		MT_PLE(0x00c)
#define MT_PLE_HIF_PG_INFO		MT_PLE(0x388)

#define MT_PLE_AC_QEMPTY(ac, n)		MT_PLE(0x600 +	0x80 * (ac) + ((n) << 2))
#define MT_PLE_AMSDU_PACK_MSDU_CNT(n)	MT_PLE(0x10e0 + ((n) << 2))

#define MT_PSE_BASE			0x820c8000
#define MT_PSE(ofs)			(MT_PSE_BASE + (ofs))

/* WF MDP TOP */
#define MT_MDP_BASE			0x820cc000
#define MT_MDP(ofs)			(MT_MDP_BASE + (ofs))

#define MT_MDP_DCR0			MT_MDP(0x000)
#define MT_MDP_DCR0_DAMSDU_EN		BIT(15)

#define MT_MDP_DCR1			MT_MDP(0x004)
#define MT_MDP_DCR1_MAX_RX_LEN		GENMASK(15, 3)

#define MT_MDP_DCR2			MT_MDP(0x8e8)
#define MT_MDP_DCR2_RX_TRANS_SHORT	BIT(2)

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

/* TMAC: band 0(0x820e4000), band 1(0x820f4000), band 3(0x830e4000) */
#define MT_WF_TMAC_BASE(_band)		__BASE(WF_TMAC_BASE, (_band))
#define MT_WF_TMAC(_band, ofs)		(MT_WF_TMAC_BASE(_band) + (ofs))

#define MT_TMAC_TCR0(_band)		MT_WF_TMAC(_band, 0)
#define MT_TMAC_TCR0_TX_BLINK		GENMASK(7, 6)
#define MT_TMAC_TCR0_TBTT_STOP_CTRL	BIT(25)

#define MT_TMAC_CDTR(_band)		MT_WF_TMAC(_band, 0x0c8)
#define MT_TMAC_ODTR(_band)		MT_WF_TMAC(_band, 0x0cc)
#define MT_TIMEOUT_VAL_PLCP		GENMASK(15, 0)
#define MT_TIMEOUT_VAL_CCA		GENMASK(31, 16)

#define MT_TMAC_ATCR(_band)		MT_WF_TMAC(_band, 0x00c)
#define MT_TMAC_ATCR_TXV_TOUT		GENMASK(7, 0)

#define MT_TMAC_TRCR0(_band)		MT_WF_TMAC(_band, 0x010)
#define MT_TMAC_TRCR0_TR2T_CHK		GENMASK(8, 0)
#define MT_TMAC_TRCR0_I2T_CHK		GENMASK(24, 16)

#define MT_TMAC_ICR0(_band)		MT_WF_TMAC(_band, 0x014)
#define MT_IFS_EIFS_OFDM		GENMASK(8, 0)
#define MT_IFS_RIFS			GENMASK(14, 10)
#define MT_IFS_SIFS			GENMASK(22, 16)
#define MT_IFS_SLOT			GENMASK(30, 24)

#define MT_TMAC_ICR1(_band)		MT_WF_TMAC(_band, 0x018)
#define MT_IFS_EIFS_CCK			GENMASK(8, 0)

#define MT_TMAC_CTCR0(_band)		MT_WF_TMAC(_band, 0x114)
#define MT_TMAC_CTCR0_INS_DDLMT_REFTIME		GENMASK(5, 0)
#define MT_TMAC_CTCR0_INS_DDLMT_EN		BIT(17)
#define MT_TMAC_CTCR0_INS_DDLMT_VHT_SMPDU_EN	BIT(18)

#define MT_TMAC_TFCR0(_band)		MT_WF_TMAC(_band, 0x0e4)

/* WF DMA TOP: band 0(0x820e7000),band 1(0x820f7000),band 3(0x830e7000) */
#define MT_WF_DMA_BASE(_band)		__BASE(WF_DMA_BASE, (_band))
#define MT_WF_DMA(_band, ofs)		(MT_WF_DMA_BASE(_band) + (ofs))

#define MT_DMA_DCR0(_band)		MT_WF_DMA(_band, 0x000)
#define MT_DMA_DCR0_MAX_RX_LEN		GENMASK(15, 3)
#define MT_DMA_DCR0_RXD_G5_EN		BIT(23)

/* ETBF: band 0(0x820ea000), band 1(0x820fa000), band 3(0x830ea000) */
#define MT_WF_ETBF_BASE(_band)		__BASE(WF_ETBF_BASE, (_band))
#define MT_WF_ETBF(_band, ofs)		(MT_WF_ETBF_BASE(_band) + (ofs))

#define MT_ETBF_TX_NDP_BFRP(_band)	MT_WF_ETBF(_band, 0x040)
#define MT_ETBF_TX_FB_CPL		GENMASK(31, 16)
#define MT_ETBF_TX_FB_TRI		GENMASK(15, 0)

#define MT_ETBF_RX_FB_CONT(_band)	MT_WF_ETBF(_band, 0x068)
#define MT_ETBF_RX_FB_BW		GENMASK(7, 6)
#define MT_ETBF_RX_FB_NC		GENMASK(5, 3)
#define MT_ETBF_RX_FB_NR		GENMASK(2, 0)

#define MT_ETBF_TX_APP_CNT(_band)	MT_WF_ETBF(_band, 0x0f0)
#define MT_ETBF_TX_IBF_CNT		GENMASK(31, 16)
#define MT_ETBF_TX_EBF_CNT		GENMASK(15, 0)

#define MT_ETBF_RX_FB_CNT(_band)	MT_WF_ETBF(_band, 0x0f8)
#define MT_ETBF_RX_FB_ALL		GENMASK(31, 24)
#define MT_ETBF_RX_FB_HE		GENMASK(23, 16)
#define MT_ETBF_RX_FB_VHT		GENMASK(15, 8)
#define MT_ETBF_RX_FB_HT		GENMASK(7, 0)

/* LPON: band 0(0x820eb000), band 1(0x820fb000), band 3(0x830eb000) */
#define MT_WF_LPON_BASE(_band)		__BASE(WF_LPON_BASE, (_band))
#define MT_WF_LPON(_band, ofs)		(MT_WF_LPON_BASE(_band) + (ofs))

#define MT_LPON_UTTR0(_band)		MT_WF_LPON(_band, 0x360)
#define MT_LPON_UTTR1(_band)		MT_WF_LPON(_band, 0x364)
#define MT_LPON_FRCR(_band)		MT_WF_LPON(_band, 0x37c)

#define MT_LPON_TCR(_band, n)		MT_WF_LPON(_band, 0x0a8 + (((n) * 4) << 4))
#define MT_LPON_TCR_SW_MODE		GENMASK(1, 0)
#define MT_LPON_TCR_SW_WRITE		BIT(0)
#define MT_LPON_TCR_SW_ADJUST		BIT(1)
#define MT_LPON_TCR_SW_READ		GENMASK(1, 0)

/* MIB: band 0(0x820ed000), band 1(0x820fd000) band 3(0x830ed000)*/
/* These counters are (mostly?) clear-on-read.  So, some should not
 * be read at all in case firmware is already reading them.  These
 * are commented with 'DNR' below.  The DNR stats will be read by querying
 * the firmware API for the appropriate message.  For counters the driver
 * does read, the driver should accumulate the counters.
 */
#define MT_WF_MIB_BASE(_band)		__BASE(WF_MIB_BASE, (_band))
#define MT_WF_MIB(_band, ofs)		(MT_WF_MIB_BASE(_band) + (ofs))

#define MT_MIB_SDR0(_band)		MT_WF_MIB(_band, 0x010)
#define MT_MIB_SDR0_BERACON_TX_CNT_MASK	GENMASK(15, 0)

#define MT_MIB_SDR3(_band)		MT_WF_MIB(_band, 0x698)
#define MT_MIB_SDR3_FCS_ERR_MASK	GENMASK(31, 16)

#define MT_MIB_SDR4(_band)		MT_WF_MIB(_band, 0x788)
#define MT_MIB_SDR4_RX_FIFO_FULL_MASK	GENMASK(15, 0)

/* rx mpdu counter, full 32 bits */
#define MT_MIB_SDR5(_band)		MT_WF_MIB(_band, 0x780)

#define MT_MIB_SDR6(_band)		MT_WF_MIB(_band, 0x020)
#define MT_MIB_SDR6_CHANNEL_IDL_CNT_MASK	GENMASK(15, 0)

#define MT_MIB_SDR7(_band)		MT_WF_MIB(_band, 0x5a8)
#define MT_MIB_SDR7_RX_VECTOR_MISMATCH_CNT_MASK	GENMASK(15, 0)

#define MT_MIB_SDR8(_band)		MT_WF_MIB(_band, 0x78c)
#define MT_MIB_SDR8_RX_DELIMITER_FAIL_CNT_MASK	GENMASK(15, 0)

/* aka CCA_NAV_TX_TIME */
#define MT_MIB_SDR9_DNR(_band)		MT_WF_MIB(_band, 0x024)
#define MT_MIB_SDR9_CCA_BUSY_TIME_MASK	GENMASK(23, 0)

#define MT_MIB_SDR10_DNR(_band)		MT_WF_MIB(_band, 0x76c)
#define MT_MIB_SDR10_MRDY_COUNT_MASK	GENMASK(31, 0)

#define MT_MIB_SDR11(_band)		MT_WF_MIB(_band, 0x790)
#define MT_MIB_SDR11_RX_LEN_MISMATCH_CNT_MASK	GENMASK(15, 0)

/* tx ampdu cnt, full 32 bits */
#define MT_MIB_SDR12(_band)		MT_WF_MIB(_band, 0x558)

#define MT_MIB_SDR13(_band)		MT_WF_MIB(_band, 0x560)
#define MT_MIB_SDR13_TX_STOP_Q_EMPTY_CNT_MASK	GENMASK(15, 0)

/* counts all mpdus in ampdu, regardless of success */
#define MT_MIB_SDR14(_band)		MT_WF_MIB(_band, 0x564)
#define MT_MIB_SDR14_TX_MPDU_ATTEMPTS_CNT_MASK	GENMASK(31, 0)

/* counts all successfully tx'd mpdus in ampdu */
#define MT_MIB_SDR15(_band)		MT_WF_MIB(_band, 0x568)
#define MT_MIB_SDR15_TX_MPDU_SUCCESS_CNT_MASK	GENMASK(31, 0)

/* in units of 'us' */
#define MT_MIB_SDR16_DNR(_band)		MT_WF_MIB(_band, 0x7fc)
#define MT_MIB_SDR16_PRIMARY_CCA_BUSY_TIME_MASK	GENMASK(23, 0)

#define MT_MIB_SDR17_DNR(_band)		MT_WF_MIB(_band, 0x800)
#define MT_MIB_SDR17_SECONDARY_CCA_BUSY_TIME_MASK	GENMASK(23, 0)

#define MT_MIB_SDR18(_band)		MT_WF_MIB(_band, 0x030)
#define MT_MIB_SDR18_PRIMARY_ENERGY_DETECT_TIME_MASK	GENMASK(23, 0)

/* units are us */
#define MT_MIB_SDR19_DNR(_band)		MT_WF_MIB(_band, 0x5ac)
#define MT_MIB_SDR19_CCK_MDRDY_TIME_MASK	GENMASK(23, 0)

#define MT_MIB_SDR20_DNR(_band)		MT_WF_MIB(_band, 0x5b0)
#define MT_MIB_SDR20_OFDM_VHT_MDRDY_TIME_MASK	GENMASK(23, 0)

#define MT_MIB_SDR21_DNR(_band)		MT_WF_MIB(_band, 0x5b4)
#define MT_MIB_SDR20_GREEN_MDRDY_TIME_MASK	GENMASK(23, 0)

/* rx ampdu count, 32-bit */
#define MT_MIB_SDR22(_band)		MT_WF_MIB(_band, 0x770)

/* rx ampdu bytes count, 32-bit */
#define MT_MIB_SDR23(_band)		MT_WF_MIB(_band, 0x774)

/* rx ampdu valid subframe count */
#define MT_MIB_SDR24(_band)		MT_WF_MIB(_band, 0x778)
#define MT_MIB_SDR24_RX_AMPDU_SF_CNT_MASK	GENMASK(31, 0)

/* rx ampdu valid subframe bytes count, 32bits */
#define MT_MIB_SDR25(_band)		MT_WF_MIB(_band, 0x77c)

/* remaining windows protected stats */
#define MT_MIB_SDR27(_band)		MT_WF_MIB(_band, 0x080)
#define MT_MIB_SDR27_TX_RWP_FAIL_CNT_MASK	GENMASK(15, 0)

#define MT_MIB_SDR28(_band)		MT_WF_MIB(_band, 0x084)
#define MT_MIB_SDR28_TX_RWP_NEED_CNT_MASK	GENMASK(15, 0)

#define MT_MIB_SDR29(_band)		MT_WF_MIB(_band, 0x650)
#define MT_MIB_SDR29_RX_PFDROP_CNT_MASK	GENMASK(15, 0)

#define MT_MIB_SDRVEC(_band)		MT_WF_MIB(_band, 0x5a8)
#define MT_MIB_SDR30_RX_VEC_QUEUE_OVERFLOW_DROP_CNT_MASK	GENMASK(31, 16)

/* rx blockack count, 32 bits */
#define MT_MIB_SDR31(_band)		MT_WF_MIB(_band, 0x55c)

#define MT_MIB_SDR32(_band)		MT_WF_MIB(_band, 0x7a8)
#define MT_MIB_SDR32_TX_PKT_EBF_CNT_MASK	GENMASK(15, 0)

#define MT_MIB_SDR33(_band)		MT_WF_MIB(_band, 0x088)
#define MT_MIB_SDR32_TX_PKT_IBF_CNT_MASK	GENMASK(31, 16)

#define MT_MIB_SDRMUBF(_band)		MT_WF_MIB(_band, 0x7ac)
#define MT_MIB_MU_BF_TX_CNT		GENMASK(15, 0)

/* 36, 37 both DNR */

#define MT_MIB_DR8(_band)		MT_WF_MIB(_band, 0x56c)
#define MT_MIB_DR9(_band)		MT_WF_MIB(_band, 0x570)
#define MT_MIB_DR11(_band)		MT_WF_MIB(_band, 0x574)

#define MT_MIB_MB_SDR0(_band, n)	MT_WF_MIB(_band, 0x688 + (n))
#define MT_MIB_RTS_RETRIES_COUNT_MASK	GENMASK(31, 16)
#define MT_MIB_RTS_COUNT_MASK		GENMASK(15, 0)

#define MT_MIB_MB_SDR1(_band, n)	MT_WF_MIB(_band, 0x690 + (n))
#define MT_MIB_BA_MISS_COUNT_MASK	GENMASK(15, 0)
#define MT_MIB_ACK_FAIL_COUNT_MASK	GENMASK(31, 16)

#define MT_MIB_MB_SDR2(_band, n)	MT_WF_MIB(_band, 0x518 + (n))
#define MT_MIB_MB_BFTF(_band, n)	MT_WF_MIB(_band, 0x510 + (n))

#define MT_TX_AGG_CNT(_band, n)		MT_WF_MIB(_band, 0x7dc + ((n) << 2))
#define MT_TX_AGG_CNT2(_band, n)	MT_WF_MIB(_band, 0x7ec + ((n) << 2))
#define MT_MIB_ARNG(_band, n)		MT_WF_MIB(_band, 0x0b0 + ((n) << 2))
#define MT_MIB_ARNCR_RANGE(val, n)	(((val) >> ((n) << 3)) & GENMASK(7, 0))

/* WTBLON TOP */
#define MT_WTBLON_TOP_BASE		0x820d4000
#define MT_WTBLON_TOP(ofs)		(MT_WTBLON_TOP_BASE + (ofs))
#define MT_WTBLON_TOP_WDUCR		MT_WTBLON_TOP(0x370)
#define MT_WTBLON_TOP_WDUCR_GROUP	GENMASK(4, 0)

#define MT_WTBL_UPDATE			MT_WTBLON_TOP(0x380)
#define MT_WTBL_UPDATE_WLAN_IDX		GENMASK(11, 0)
#define MT_WTBL_UPDATE_ADM_COUNT_CLEAR	BIT(14)
#define MT_WTBL_UPDATE_BUSY		BIT(31)

/* WTBL */
#define MT_WTBL_BASE			0x820d8000
#define MT_WTBL_LMAC_ID			GENMASK(14, 8)
#define MT_WTBL_LMAC_DW			GENMASK(7, 2)
#define MT_WTBL_LMAC_OFFS(_id, _dw)	(MT_WTBL_BASE | \
					 FIELD_PREP(MT_WTBL_LMAC_ID, _id) | \
					 FIELD_PREP(MT_WTBL_LMAC_DW, _dw))

/* AGG: band 0(0x820e2000), band 1(0x820f2000), band 3(0x830e2000) */
#define MT_WF_AGG_BASE(_band)		__BASE(WF_AGG_BASE, (_band))
#define MT_WF_AGG(_band, ofs)		(MT_WF_AGG_BASE(_band) + (ofs))

#define MT_AGG_AWSCR0(_band, _n)	MT_WF_AGG(_band, (0x030 + (_n) * 4))
#define MT_AGG_PCR0(_band, _n)		MT_WF_AGG(_band, (0x040 + (_n) * 4))
#define MT_AGG_PCR0_MM_PROT		BIT(0)
#define MT_AGG_PCR0_GF_PROT		BIT(1)
#define MT_AGG_PCR0_BW20_PROT		BIT(2)
#define MT_AGG_PCR0_BW40_PROT		BIT(4)
#define MT_AGG_PCR0_BW80_PROT		BIT(6)
#define MT_AGG_PCR0_ERP_PROT		GENMASK(12, 8)
#define MT_AGG_PCR0_VHT_PROT		BIT(13)
#define MT_AGG_PCR0_PTA_WIN_DIS		BIT(15)

#define MT_AGG_PCR1_RTS0_NUM_THRES	GENMASK(31, 23)
#define MT_AGG_PCR1_RTS0_LEN_THRES	GENMASK(19, 0)

#define MT_AGG_ACR0(_band)		MT_WF_AGG(_band, 0x054)
#define MT_AGG_ACR_CFEND_RATE		GENMASK(13, 0)
#define MT_AGG_ACR_BAR_RATE		GENMASK(29, 16)

#define MT_AGG_MRCR(_band)		MT_WF_AGG(_band, 0x068)
#define MT_AGG_MRCR_BAR_CNT_LIMIT		GENMASK(15, 12)
#define MT_AGG_MRCR_LAST_RTS_CTS_RN		BIT(6)
#define MT_AGG_MRCR_RTS_FAIL_LIMIT		GENMASK(11, 7)
#define MT_AGG_MRCR_TXCMD_RTS_FAIL_LIMIT	GENMASK(28, 24)

#define MT_AGG_ATCR1(_band)		MT_WF_AGG(_band, 0x1a8)
#define MT_AGG_ATCR3(_band)		MT_WF_AGG(_band, 0x080)

/* ARB: band 0(0x820e3000), band 1(0x820f3000), band 3(0x830e3000) */
#define MT_WF_ARB_BASE(_band)		__BASE(WF_ARB_BASE, (_band))
#define MT_WF_ARB(_band, ofs)		(MT_WF_ARB_BASE(_band) + (ofs))

#define MT_ARB_SCR(_band)		MT_WF_ARB(_band, 0x000)
#define MT_ARB_SCR_TX_DISABLE		BIT(8)
#define MT_ARB_SCR_RX_DISABLE		BIT(9)

#define MT_ARB_DRNGR0(_band, _n)	MT_WF_ARB(_band, (0x1e0 + (_n) * 4))

/* RMAC: band 0(0x820e5000), band 1(0x820f5000), band 3(0x830e5000), */
#define MT_WF_RMAC_BASE(_band)		__BASE(WF_RMAC_BASE, (_band))
#define MT_WF_RMAC(_band, ofs)		(MT_WF_RMAC_BASE(_band) + (ofs))

#define MT_WF_RFCR(_band)		MT_WF_RMAC(_band, 0x000)
#define MT_WF_RFCR_DROP_STBC_MULTI	BIT(0)
#define MT_WF_RFCR_DROP_FCSFAIL		BIT(1)
#define MT_WF_RFCR_DROP_VERSION		BIT(3)
#define MT_WF_RFCR_DROP_PROBEREQ	BIT(4)
#define MT_WF_RFCR_DROP_MCAST		BIT(5)
#define MT_WF_RFCR_DROP_BCAST		BIT(6)
#define MT_WF_RFCR_DROP_MCAST_FILTERED	BIT(7)
#define MT_WF_RFCR_DROP_A3_MAC		BIT(8)
#define MT_WF_RFCR_DROP_A3_BSSID	BIT(9)
#define MT_WF_RFCR_DROP_A2_BSSID	BIT(10)
#define MT_WF_RFCR_DROP_OTHER_BEACON	BIT(11)
#define MT_WF_RFCR_DROP_FRAME_REPORT	BIT(12)
#define MT_WF_RFCR_DROP_CTL_RSV		BIT(13)
#define MT_WF_RFCR_DROP_CTS		BIT(14)
#define MT_WF_RFCR_DROP_RTS		BIT(15)
#define MT_WF_RFCR_DROP_DUPLICATE	BIT(16)
#define MT_WF_RFCR_DROP_OTHER_BSS	BIT(17)
#define MT_WF_RFCR_DROP_OTHER_UC	BIT(18)
#define MT_WF_RFCR_DROP_OTHER_TIM	BIT(19)
#define MT_WF_RFCR_DROP_NDPA		BIT(20)
#define MT_WF_RFCR_DROP_UNWANTED_CTL	BIT(21)

#define MT_WF_RFCR1(_band)		MT_WF_RMAC(_band, 0x004)
#define MT_WF_RFCR1_DROP_ACK		BIT(4)
#define MT_WF_RFCR1_DROP_BF_POLL	BIT(5)
#define MT_WF_RFCR1_DROP_BA		BIT(6)
#define MT_WF_RFCR1_DROP_CFEND		BIT(7)
#define MT_WF_RFCR1_DROP_CFACK		BIT(8)

#define MT_WF_RMAC_MIB_AIRTIME0(_band)	MT_WF_RMAC(_band, 0x0380)
#define MT_WF_RMAC_MIB_RXTIME_CLR	BIT(31)

/* WFDMA0 */
#define MT_WFDMA0_BASE			0xd4000
#define MT_WFDMA0(ofs)			(MT_WFDMA0_BASE + (ofs))

#define MT_WFDMA0_RST			MT_WFDMA0(0x100)
#define MT_WFDMA0_RST_LOGIC_RST		BIT(4)
#define MT_WFDMA0_RST_DMASHDL_ALL_RST	BIT(5)

#define MT_WFDMA0_BUSY_ENA		MT_WFDMA0(0x13c)
#define MT_WFDMA0_BUSY_ENA_TX_FIFO0	BIT(0)
#define MT_WFDMA0_BUSY_ENA_TX_FIFO1	BIT(1)
#define MT_WFDMA0_BUSY_ENA_RX_FIFO	BIT(2)

#define MT_WFDMA0_GLO_CFG		MT_WFDMA0(0x208)
#define MT_WFDMA0_GLO_CFG_TX_DMA_EN	BIT(0)
#define MT_WFDMA0_GLO_CFG_RX_DMA_EN	BIT(2)
#define MT_WFDMA0_GLO_CFG_OMIT_TX_INFO	BIT(28)
#define MT_WFDMA0_GLO_CFG_OMIT_RX_INFO	BIT(27)
#define MT_WFDMA0_GLO_CFG_OMIT_RX_INFO_PFET2	BIT(21)

#define MT_WFDMA0_RST_DTX_PTR		MT_WFDMA0(0x20c)
#define MT_WFDMA0_PRI_DLY_INT_CFG0	MT_WFDMA0(0x2f0)
#define MT_WFDMA0_PRI_DLY_INT_CFG1	MT_WFDMA0(0x2f4)
#define MT_WFDMA0_PRI_DLY_INT_CFG2	MT_WFDMA0(0x2f8)

/* WFDMA1 */
#define MT_WFDMA1_BASE			0xd5000
#define MT_WFDMA1(ofs)			(MT_WFDMA1_BASE + (ofs))

#define MT_WFDMA1_RST			MT_WFDMA1(0x100)
#define MT_WFDMA1_RST_LOGIC_RST		BIT(4)
#define MT_WFDMA1_RST_DMASHDL_ALL_RST	BIT(5)

#define MT_WFDMA1_BUSY_ENA		MT_WFDMA1(0x13c)
#define MT_WFDMA1_BUSY_ENA_TX_FIFO0	BIT(0)
#define MT_WFDMA1_BUSY_ENA_TX_FIFO1	BIT(1)
#define MT_WFDMA1_BUSY_ENA_RX_FIFO	BIT(2)

#define MT_WFDMA1_GLO_CFG		MT_WFDMA1(0x208)
#define MT_WFDMA1_GLO_CFG_TX_DMA_EN	BIT(0)
#define MT_WFDMA1_GLO_CFG_RX_DMA_EN	BIT(2)
#define MT_WFDMA1_GLO_CFG_OMIT_TX_INFO	BIT(28)
#define MT_WFDMA1_GLO_CFG_OMIT_RX_INFO	BIT(27)
#define MT_WFDMA1_GLO_CFG_OMIT_RX_INFO_PFET2	BIT(21)

#define MT_WFDMA1_RST_DTX_PTR		MT_WFDMA1(0x20c)
#define MT_WFDMA1_PRI_DLY_INT_CFG0	MT_WFDMA1(0x2f0)

/* WFDMA CSR */
#define MT_WFDMA_EXT_CSR_BASE		0xd7000
#define MT_WFDMA_EXT_CSR(ofs)		(MT_WFDMA_EXT_CSR_BASE + (ofs))

#define MT_WFDMA_HOST_CONFIG		MT_WFDMA_EXT_CSR(0x30)
#define MT_WFDMA_HOST_CONFIG_PDMA_BAND	BIT(0)

#define MT_WFDMA_EXT_CSR_HIF_MISC	MT_WFDMA_EXT_CSR(0x44)
#define MT_WFDMA_EXT_CSR_HIF_MISC_BUSY	BIT(0)

#define MT_PCIE_RECOG_ID		0xd7090
#define MT_PCIE_RECOG_ID_MASK		GENMASK(30, 0)
#define MT_PCIE_RECOG_ID_SEM		BIT(31)

/* WFDMA0 PCIE1 */
#define MT_WFDMA0_PCIE1_BASE		0xd8000
#define MT_WFDMA0_PCIE1(ofs)		(MT_WFDMA0_PCIE1_BASE + (ofs))

#define MT_WFDMA0_PCIE1_BUSY_ENA	MT_WFDMA0_PCIE1(0x13c)
#define MT_WFDMA0_PCIE1_BUSY_ENA_TX_FIFO0	BIT(0)
#define MT_WFDMA0_PCIE1_BUSY_ENA_TX_FIFO1	BIT(1)
#define MT_WFDMA0_PCIE1_BUSY_ENA_RX_FIFO	BIT(2)

/* WFDMA1 PCIE1 */
#define MT_WFDMA1_PCIE1_BASE		0xd9000
#define MT_WFDMA1_PCIE1(ofs)		(MT_WFDMA1_PCIE1_BASE + (ofs))

#define MT_WFDMA1_PCIE1_BUSY_ENA	MT_WFDMA1_PCIE1(0x13c)
#define MT_WFDMA1_PCIE1_BUSY_ENA_TX_FIFO0	BIT(0)
#define MT_WFDMA1_PCIE1_BUSY_ENA_TX_FIFO1	BIT(1)
#define MT_WFDMA1_PCIE1_BUSY_ENA_RX_FIFO	BIT(2)

/* WFDMA COMMON */
#define __RXQ(q)			((q) + __MT_MCUQ_MAX)
#define __TXQ(q)			(__RXQ(q) + __MT_RXQ_MAX)

#define MT_RXQ_VALID(q)			(dev->mt76.q_rx[q].ndesc)

#define MT_Q_ID(q)			(dev->q_id[(q)])
#define MT_Q_BASE(q)			((dev->wfdma_mask >> (q)) & 0x1 ?	\
					 MT_WFDMA1_BASE : MT_WFDMA0_BASE)

#define MT_MCUQ_ID(q)			MT_Q_ID(q)
#define MT_TXQ_ID(q)			MT_Q_ID(__TXQ(q))
#define MT_RXQ_ID(q)			MT_Q_ID(__RXQ(q))

#define MT_MCUQ_RING_BASE(q)		(MT_Q_BASE(q) + 0x300)
#define MT_TXQ_RING_BASE(q)		(MT_Q_BASE(__TXQ(q)) + 0x300)
#define MT_RXQ_RING_BASE(q)		(MT_Q_BASE(__RXQ(q)) + 0x500)

#define MT_MCUQ_EXT_CTRL(q)		(MT_Q_BASE(q) +	0x600 +	\
					 MT_MCUQ_ID(q)* 0x4)
#define MT_RXQ_EXT_CTRL(q)		(MT_Q_BASE(__RXQ(q)) + 0x680 +	\
					 MT_RXQ_ID(q)* 0x4)
#define MT_TXQ_EXT_CTRL(q)		(MT_Q_BASE(__TXQ(q)) + 0x600 +	\
					 MT_TXQ_ID(q)* 0x4)

#define MT_INT_SOURCE_CSR		MT_WFDMA0(0x200)
#define MT_INT_MASK_CSR			MT_WFDMA0(0x204)

#define MT_INT1_SOURCE_CSR		MT_WFDMA0_PCIE1(0x200)
#define MT_INT1_MASK_CSR		MT_WFDMA0_PCIE1(0x204)

#define MT_INT_RX_DONE_BAND0		BIT(22)
#define MT_INT_RX_DONE_BAND1		BIT(23)
#define MT_INT_RX_DONE_BAND2		BIT(13)
#define MT_INT_RX_DONE_WM		BIT(0)
#define MT_INT_RX_DONE_WA		BIT(1)
#define MT_INT_RX_DONE_WA_MAIN		BIT(2)
#define MT_INT_RX_DONE_WA_EXT		BIT(2)
#define MT_INT_RX_DONE_WA_TRI		BIT(2)
#define MT_INT_MCU_CMD			BIT(29)

#define MT_INT_RX(q)			(dev->q_int_mask[__RXQ(q)])
#define MT_INT_TX_MCU(q)		(dev->q_int_mask[(q)])

#define MT_INT_RX_DONE_MCU		(MT_INT_RX(MT_RXQ_MCU) |	\
					 MT_INT_RX(MT_RXQ_MCU_WA))

#define MT_INT_BAND0_RX_DONE		(MT_INT_RX(MT_RXQ_MAIN) |	\
					 MT_INT_RX(MT_RXQ_MAIN_WA))

#define MT_INT_BAND1_RX_DONE		(MT_INT_RX(MT_RXQ_EXT) |	\
					 MT_INT_RX(MT_RXQ_EXT_WA) |	\
					 MT_INT_RX(MT_RXQ_MAIN_WA))

#define MT_INT_BAND2_RX_DONE		(MT_INT_RX(MT_RXQ_TRI) |	\
					 MT_INT_RX(MT_RXQ_TRI_WA) |	\
					 MT_INT_RX(MT_RXQ_MAIN_WA))

#define MT_INT_RX_DONE_ALL		(MT_INT_RX_DONE_MCU |		\
					 MT_INT_BAND0_RX_DONE |		\
					 MT_INT_BAND1_RX_DONE)

#define MT_INT_TX_DONE_FWDL		BIT(26)
#define MT_INT_TX_DONE_MCU_WM		BIT(27)
#define MT_INT_TX_DONE_MCU_WA		BIT(25)
#define MT_INT_TX_DONE_BAND0		BIT(30)
#define MT_INT_TX_DONE_BAND1		BIT(31)
#define MT_INT_TX_DONE_BAND2		BIT(16)

#define MT_INT_TX_DONE_MCU		(MT_INT_TX_MCU(MT_MCUQ_WA) |	\
					 MT_INT_TX_MCU(MT_MCUQ_WM) |	\
					 MT_INT_TX_MCU(MT_MCUQ_FWDL))

#define MT_MCU_CMD			MT_WFDMA0(0x1f0)
#define MT_MCU_CMD_STOP_DMA_FW_RELOAD	BIT(1)
#define MT_MCU_CMD_STOP_DMA		BIT(2)
#define MT_MCU_CMD_RESET_DONE		BIT(3)
#define MT_MCU_CMD_RECOVERY_DONE	BIT(4)
#define MT_MCU_CMD_NORMAL_STATE		BIT(5)
#define MT_MCU_CMD_ERROR_MASK		GENMASK(5, 1)

/* TOP RGU */
#define MT_TOP_RGU_BASE			0x18000000
#define MT_TOP_PWR_CTRL			(MT_TOP_RGU_BASE + (0x0))
#define MT_TOP_PWR_KEY			(0x5746 << 16)
#define MT_TOP_PWR_SW_RST		BIT(0)
#define MT_TOP_PWR_SW_PWR_ON		GENMASK(3, 2)
#define MT_TOP_PWR_HW_CTRL		BIT(4)
#define MT_TOP_PWR_PWR_ON		BIT(7)

#define MT_TOP_RGU_SYSRAM_PDN		(MT_TOP_RGU_BASE + 0x050)
#define MT_TOP_RGU_SYSRAM_SLP		(MT_TOP_RGU_BASE + 0x054)
#define MT_TOP_WFSYS_PWR		(MT_TOP_RGU_BASE + 0x010)
#define MT_TOP_PWR_EN_MASK		BIT(7)
#define MT_TOP_PWR_ACK_MASK		BIT(6)
#define MT_TOP_PWR_KEY_MASK		GENMASK(31, 16)

/* CONN INFRA BUS ON */
#define MT_INFRA_BUS_ON_BASE		0xFE000
#define MT_INFRA_BUS_ON(ofs)		(MT_INFRA_BUS_ON_BASE + (ofs))

#define MT_INFRA_BUS_ON_REMAP_WF_5_4	MT_INFRA_BUS_ON(0x418)
#define MT_INFRA_BUS_ON_REMAP_WF_5_MASK	GENMASK(31, 16)
#define MT_INFRA_BUS_ON_REMAP_WF_4_MASK	GENMASK(15, 0)

/* l1/l2 remap */
#define MT_HIF_REMAP_L1			0xfe418
#define MT_HIF_REMAP_L1_MASK		GENMASK(15, 0)
#define MT_HIF_REMAP_L1_OFFSET		GENMASK(15, 0)
#define MT_HIF_REMAP_L1_BASE		GENMASK(31, 16)
#define MT_HIF_REMAP_BASE_L1		0x40000

#define MT_HIF_REMAP_L2			0x1b4
#define MT_HIF_REMAP_L2_MASK		GENMASK(19, 0)
#define MT_HIF_REMAP_L2_OFFSET		GENMASK(11, 0)
#define MT_HIF_REMAP_L2_BASE		GENMASK(31, 12)
#define MT_HIF_REMAP_BASE_L2		0x1000

#define MT_INFRA_BASE			0x18000000
#define MT_WFSYS0_PHY_START		0x18400000
#define MT_WFSYS1_PHY_START		0x18800000
#define MT_WFSYS1_PHY_END		0x18bfffff
#define MT_CBTOP1_PHY_START		0x70000000
#define MT_CBTOP1_PHY_END		0x77ffffff
#define MT_CBTOP2_PHY_START		0xf0000000
#define MT_CBTOP2_PHY_END		0xffffffff
#define MT_INFRA_MCU_START		0x7c000000
#define MT_INFRA_MCU_END		0x7c3fffff
#define MT_CONN_INFRA_OFFSET(p)		((p) - MT_INFRA_BASE)

/* CONN INFRA RGU ON */
#define MT_INFRA_RGU_BASE		0xF0000
#define MT_INFRA_RGU(ofs)		(MT_INFRA_RGU_BASE + (ofs))

#define MT_INFRA_RGU_RGU_ON_SW_RST_B     MT_INFRA_RGU(0x120)
#define MT_INFRA_RGU_RGU_ON_SW_RST_B_MASK BIT(0)

/* FW MODE SYNC */
#define MT_SWDEF_MODE			0x9143c
#define MT_SWDEF_NORMAL_MODE		0
#define MT_SWDEF_ICAP_MODE		1
#define MT_SWDEF_SPECTRUM_MODE		2

#define MT_DIC_CMD_REG_BASE		0x41f000
#define MT_DIC_CMD_REG(ofs)		(MT_DIC_CMD_REG_BASE + (ofs))
#define MT_DIC_CMD_REG_CMD		MT_DIC_CMD_REG(0x10)

#define MT_CPU_UTIL_BASE		0x41f030
#define MT_CPU_UTIL(ofs)		(MT_CPU_UTIL_BASE + (ofs))
#define MT_CPU_UTIL_BUSY_PCT		MT_CPU_UTIL(0x00)
#define MT_CPU_UTIL_PEAK_BUSY_PCT	MT_CPU_UTIL(0x04)
#define MT_CPU_UTIL_IDLE_CNT		MT_CPU_UTIL(0x08)
#define MT_CPU_UTIL_PEAK_IDLE_CNT	MT_CPU_UTIL(0x0c)
#define MT_CPU_UTIL_CTRL		MT_CPU_UTIL(0x1c)

/* LED */
#define MT_LED_TOP_BASE			0x18013000
#define MT_LED_PHYS(_n)			(MT_LED_TOP_BASE + (_n))

#define MT_LED_CTRL(_n)			MT_LED_PHYS(0x00 + ((_n) * 4))
#define MT_LED_CTRL_KICK		BIT(7)
#define MT_LED_CTRL_BLINK_MODE		BIT(2)
#define MT_LED_CTRL_POLARITY		BIT(1)

#define MT_LED_TX_BLINK(_n)		MT_LED_PHYS(0x10 + ((_n) * 4))
#define MT_LED_TX_BLINK_ON_MASK		GENMASK(7, 0)
#define MT_LED_TX_BLINK_OFF_MASK        GENMASK(15, 8)

#define MT_LED_EN(_n)			MT_LED_PHYS(0x40 + ((_n) * 4))

#define MT_LED_GPIO_MUX2                0x70005058 /* GPIO 18 */
#define MT_LED_GPIO_MUX3                0x7000505C /* GPIO 26 */
#define MT_LED_GPIO_SEL_MASK            GENMASK(11, 8)

/* MT TOP */
#define MT_TOP_BASE			0xe0000
#define MT_TOP(ofs)			(MT_TOP_BASE + (ofs))

#define MT_TOP_LPCR_HOST_BAND(_band)	MT_TOP(0x10 + ((_band) * 0x10))
#define MT_TOP_LPCR_HOST_FW_OWN		BIT(0)
#define MT_TOP_LPCR_HOST_DRV_OWN	BIT(1)
#define MT_TOP_LPCR_HOST_FW_OWN_STAT	BIT(2)

#define MT_TOP_LPCR_HOST_BAND_IRQ_STAT(_band)	MT_TOP(0x14 + ((_band) * 0x10))
#define MT_TOP_LPCR_HOST_BAND_STAT	BIT(0)

#define MT_TOP_MISC			MT_TOP(0xf0)
#define MT_TOP_MISC_FW_STATE		GENMASK(2, 0)

#define MT_HW_BOUND			0x70010020
#define MT_HW_REV			0x70010204
#define MT_WF_SUBSYS_RST		0x70002600

#define MT_TOP_WFSYS_WAKEUP		MT_TOP(0x1a4)
#define MT_TOP_WFSYS_WAKEUP_MASK	BIT(0)

#define MT_TOP_MCU_EMI_BASE		MT_TOP(0x1c4)
#define MT_TOP_MCU_EMI_BASE_MASK	GENMASK(19, 0)

#define MT_TOP_CONN_INFRA_WAKEUP	MT_TOP(0x1a0)
#define MT_TOP_CONN_INFRA_WAKEUP_MASK	BIT(0)

#define MT_TOP_WFSYS_RESET_STATUS	MT_TOP(0x2cc)
#define MT_TOP_WFSYS_RESET_STATUS_MASK	BIT(30)

/* SEMA */
#define MT_SEMA_BASE			0x18070000
#define MT_SEMA(ofs)			(MT_SEMA_BASE + (ofs))

#define MT_SEMA_RFSPI_STATUS		(MT_SEMA(0x2000) + (11 * 4))
#define MT_SEMA_RFSPI_RELEASE		(MT_SEMA(0x2200) + (11 * 4))
#define MT_SEMA_RFSPI_STATUS_MASK	BIT(1)

/* MCU BUS */
#define MT_MCU_BUS_BASE			0x00000
#define MT_MCU_BUS(ofs)			(MT_MCU_BUS_BASE + (ofs))

#define MT_MCU_BUS_TIMEOUT		MT_MCU_BUS(0xf0440)
#define MT_MCU_BUS_TIMEOUT_SET_MASK	GENMASK(7, 0)
#define MT_MCU_BUS_TIMEOUT_CG_EN_MASK	BIT(28)
#define MT_MCU_BUS_TIMEOUT_EN_MASK	BIT(31)

#define MT_MCU_BUS_REMAP		MT_MCU_BUS(0x120)

/* TOP CFG */
#define MT_TOP_CFG_BASE			0x184b0000
#define MT_TOP_CFG(ofs)			(MT_TOP_CFG_BASE + (ofs))

#define MT_TOP_CFG_IP_VERSION_ADDR	MT_TOP_CFG(0x010)

/* TOP CFG ON */
#define MT_TOP_CFG_ON_BASE		0xc1000
#define MT_TOP_CFG_ON(ofs)		(MT_TOP_CFG_ON_BASE + (ofs))

#define MT_TOP_CFG_ON_ROM_IDX		MT_TOP_CFG_ON(0x604)
#define MT_TOP_CFG_ON_ROM_IDLE		0x1D1E
#define MT_TOP_CFG_ON_ROM_STATE_MASK	GENMASK(15, 0)

/* SLP CTRL */
#define MT_SLP_BASE			0x184c3000
#define MT_SLP(ofs)			(MT_SLP_BASE + (ofs))

#define MT_SLP_STATUS			MT_SLP(0x00c)
#define MT_SLP_WFDMA2CONN_MASK		(BIT(21) | BIT(23))
#define MT_SLP_CTRL_EN_MASK		BIT(0)
#define MT_SLP_CTRL_BSY_MASK		BIT(1)

/* MCU BUS DBG */
#define MT_MCU_BUS_DBG_BASE		0x18500000
#define MT_MCU_BUS_DBG(ofs)		(MT_MCU_BUS_DBG_BASE + (ofs))

#define MT_MCU_BUS_DBG_TIMEOUT		MT_MCU_BUS_DBG(0x0)
#define MT_MCU_BUS_DBG_TIMEOUT_SET_MASK GENMASK(31, 16)
#define MT_MCU_BUS_DBG_TIMEOUT_CK_EN_MASK BIT(3)
#define MT_MCU_BUS_DBG_TIMEOUT_EN_MASK	BIT(2)

/* PCIE MAC */
#define MT_PCIE_MAC_BASE		0x10000
#define MT_PCIE_MAC(ofs)		(MT_PCIE_MAC_BASE + (ofs))
#define MT_PCIE_MAC_INT_ENABLE		MT_PCIE_MAC(0x188)

#define MT_PCIE1_MAC_BASE		0x70000
#define MT_PCIE1_MAC(ofs)		(MT_PCIE1_MAC_BASE + (ofs))

#define MT_PCIE1_MAC_INT_ENABLE		MT_PCIE1_MAC(0x188)

/* PP TOP */
#define MT_WF_PP_TOP_BASE		0x820cc000
#define MT_WF_PP_TOP(ofs)		(MT_WF_PP_TOP_BASE + (ofs))

#define MT_WF_PP_TOP_RXQ_WFDMA_CF_5	MT_WF_PP_TOP(0x0e8)
#define MT_WF_PP_TOP_RXQ_QID6_WFDMA_HIF_SEL_MASK	BIT(6)

#define MT_WF_IRPI_BASE			0x83000000
#define MT_WF_IRPI(ofs)			(MT_WF_IRPI_BASE + (ofs))

#define MT_WF_IRPI_NSS(phy, nss)	MT_WF_IRPI(0x1000 + ((phy) << 20) + ((nss) << 16))

/* PHY */
#define MT_WF_PHY_BASE			0x83080000
#define MT_WF_PHY(ofs)			(MT_WF_PHY_BASE + (ofs))

#define MT_WF_PHY_RX_CTRL1(_phy)	MT_WF_PHY(0x2004 + ((_phy) << 20))
#define MT_WF_PHY_RX_CTRL1_IPI_EN	GENMASK(2, 0)
#define MT_WF_PHY_RX_CTRL1_STSCNT_EN	GENMASK(11, 9)

#define MT_WF_PHY_RXTD12(_phy)	MT_WF_PHY(0x8230 + ((_phy) << 20))
#define MT_WF_PHY_RXTD12_IRPI_SW_CLR_ONLY	BIT(18)
#define MT_WF_PHY_RXTD12_IRPI_SW_CLR		BIT(29)

#define MT_MCU_WM_CIRQ_BASE			0x89010000
#define MT_MCU_WM_CIRQ(ofs)			(MT_MCU_WM_CIRQ_BASE + (ofs))
#define MT_MCU_WM_CIRQ_IRQ_MASK_CLR_ADDR	MT_MCU_WM_CIRQ(0x80)
#define MT_MCU_WM_CIRQ_IRQ_SOFT_ADDR		MT_MCU_WM_CIRQ(0xc0)

#endif
