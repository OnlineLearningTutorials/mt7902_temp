/* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause */
/*
 * Copyright (c) 2021 MediaTek Inc.
 */
/*! \file  bellwether.h
*    \brief This file contains the info of cmm_asic_connac3x
*/

#ifndef _CMM_ASIC_CONNAC3X_H
#define _CMM_ASIC_CONNAC3X_H

#if (CFG_SUPPORT_CONNAC3X == 1)

#define CONN_INFRA_REMAPPING_OFFSET                    0x64000000
#define CONNAC3X_WFDMA_DISP_MAX_CNT_MASK               0x000000FF
#define CONNAC3X_WFDMA_DISP_BASE_PTR_MASK               0xFFFF0000

#define CONNAC3X_MCU_WPDMA_0_BASE                      0x54000000
#define CONNAC3X_MCU_WPDMA_1_BASE                      0x55000000
#define CONNAC3X_HOST_WPDMA_0_BASE                     0x7c024000
#define CONNAC3X_HOST_WPDMA_1_BASE                     0x7c025000
#define CONNAC3X_HOST_DMASHDL                          0x7c026000

#define CONNAC3X_WPDMA_GLO_CFG(__BASE)                ((__BASE) + 0x0208)
#define CONNAC3X_WPDMA_GLO_CFG_EXT0(__BASE)           ((__BASE) + 0x02B0)
#define CONNAC3X_TX_RING_EXT_CTRL_BASE(__BASE)        ((__BASE) + 0x0600)
#define CONNAC3X_TX_RING_DISP_MAX_CNT	  4
#define CONNAC3X_RX_RING_DISP_MAX_CNT	  4

#define CONNAC3X_RX_RING_CIDX(__BASE)                 ((__BASE) + 0x0508)
#define CONNAC3X_HOST_DMASHDL_SW_CONTROL(__BASE)      ((__BASE) + 0x0004)

/* OMIT_TX_INFO[28]*/
#define CONNAC3X_WPDMA1_GLO_CFG_OMIT_TX_INFO           0x10000000
/* OMIT_RX_INFO[27]*/
#define CONNAC3X_WPDMA1_GLO_CFG_OMIT_RX_INFO           0x08000000
/* OMIT_RX_INFO_PFET2[21]*/
#define CONNAC3X_WPDMA1_GLO_CFG_OMIT_RX_INFO_PFET2     0x00200000
/* FW_DWLD_Bypass_dmashdl[9] */
#define CONNAC3X_WPDMA1_GLO_CFG_FW_DWLD_Bypass_dmashdl 0x00000200
/* RX_DMA_BUSY[3] */
#define CONNAC3X_WPDMA1_GLO_CFG_RX_DMA_BUSY   0x00000008
/* RX_DMA_EN[2] */
#define CONNAC3X_WPDMA1_GLO_CFG_RX_DMA_EN     0x00000004
/* TX_DMA_BUSY[1] */
#define CONNAC3X_WPDMA1_GLO_CFG_TX_DMA_BUSY   0x00000002
/* TX_DMA_EN[0] */
#define CONNAC3X_WPDMA1_GLO_CFG_TX_DMA_EN     0x00000001
/* TX_DMASHDL_ENABLE[6] */
#define CONNAC3X_WPDMA1_GLO_CFG_EXT0_TX_DMASHDL_EN     0x00000040
/* DMASHDL_BYPASS[28] */
#define CONNAC3X_HIF_DMASHDL_BYPASS_EN                 0x10000000

#define CONNAC3X_NIC_TX_PSE_HEADER_LENGTH			16
#define CONNAC3X_RX_INIT_EVENT_LENGTH                           8

#define CONNAC3X_WFDMA_DUMMY_CR		(CONNAC3X_MCU_WPDMA_0_BASE + 0x120)
#define CONNAC3X_WFDMA_NEED_REINIT_BIT	BIT(1)

#define Connac3x_CONN_CFG_ON_BASE	0x7C060000
#define Connac3x_CONN_CFG_ON_CONN_ON_MISC_ADDR \
	(Connac3x_CONN_CFG_ON_BASE + 0xF0)
#define Connac3x_CONN_CFG_ON_CONN_ON_MISC_DRV_FM_STAT_SYNC_SHFT         0

#define WFSYS_CPUPCR_ADDR (Connac3x_CONN_CFG_ON_BASE + 0x0204)
#define WFSYS_LP_ADDR (Connac3x_CONN_CFG_ON_BASE + 0x0208)

#define CONNAC3X_CONN_HIF_ON_ADDR_REMAP23              0x7010
#define CONNAC3X_HOST_EXT_CONN_HIF_WRAP                0x7c027000
#define CONNAC3X_MCU_INT_CONN_HIF_WRAP                 0x57000000
#define CONNAC3X_MAX_WFDMA_COUNT                       2

/*******************************************************************************
*                              C O N S T A N T S
********************************************************************************
*/
#define CONNAC3X_WPDMA_EXT_INT_STA(__BASE)            ((__BASE) + 0x0010)
#define CONNAC3X_WPDMA_EXT_INT_MASK(__BASE)           ((__BASE) + 0x0014)
#define CONNAC3X_WPDMA_HIF_RST(__BASE)                ((__BASE) + 0x0100)
#define CONNAC3X_WPDMA_HOST2MCU_SW_INT_SET(__BASE)    ((__BASE) + 0x0108)
#define CONNAC3X_WPDMA_MCU2HOST_SW_INT_STA(__BASE)    ((__BASE) + 0x01F0)
#define CONNAC3X_WPDMA_MCU2HOST_SW_INT_MASK(__BASE)   ((__BASE) + 0x01F4)
#define CONNAC3X_WPDMA_INT_STA(__BASE)                ((__BASE) + 0x0200)
#define CONNAC3X_WPDMA_INT_MASK(__BASE)               ((__BASE) + 0x0204)
#define CONNAC3X_WPDMA_RST_DTX_PTR(__BASE)            ((__BASE) + 0x020C)
#define CONNAC3X_WPDMA_RST_DRX_PTR(__BASE)            ((__BASE) + 0x0280)


#define CONNAC3X_TX_RING_BASE(__BASE)                 ((__BASE) + 0x0300)
#define CONNAC3X_TX_RING_PTR(__BASE)                  ((__BASE) + 0x0300)
#define CONNAC3X_TX_RING_CNT(__BASE)                  ((__BASE) + 0x0304)
#define CONNAC3X_TX_RING_CIDX(__BASE)                 ((__BASE) + 0x0308)
#define CONNAC3X_TX_RING_DIDX(__BASE)                 ((__BASE) + 0x030C)

#define CONNAC3X_RX_RING_BASE(__BASE)                 ((__BASE) + 0x0500)
#define CONNAC3X_RX_RING_PTR(__BASE)                  ((__BASE) + 0x0500)
#define CONNAC3X_RX_RING_CNT(__BASE)                  ((__BASE) + 0x0504)
#define CONNAC3X_RX_RING_DIDX(__BASE)                 ((__BASE) + 0x050C)
#define CONNAC3X_RX_RING_EXT_CTRL_BASE(__BASE)        ((__BASE) + 0x0680)

#define CONNAC3X_WFDMA1_RX_RING_BASE(__BASE)          ((__BASE) + 0x0500)
#define CONNAC3X_WFDMA1_RX_RING_PTR(__BASE)           ((__BASE) + 0x0500)
#define CONNAC3X_WFDMA1_RX_RING_CNT(__BASE)           ((__BASE) + 0x0504)
#define CONNAC3X_WFDMA1_RX_RING_CIDX(__BASE)          ((__BASE) + 0x0508)
#define CONNAC3X_WFDMA1_RX_RING_DIDX(__BASE)          ((__BASE) + 0x050C)
#define CONNAC3X_WFDMA1_RX_RING_EXT_CTRL_BASE(__BASE) ((__BASE) + 0x0680)

#define CONNAC3X_FWDL_TX_RING_IDX         16
#define CONNAC3X_CMD_TX_RING_IDX          17 /* Direct to WM */
#define CONNAC3X_DATA0_TXD_IDX            18 /* Band_0 TXD to WA */
#define CONNAC3X_DATA1_TXD_IDX            19 /* Band_1 TXD to WA */
#define CONNAC3X_CMD_TX_WA_RING_IDX       20 /* WA relay to WM */


/* WPDMA_INT_STA (0x50000000+0x200) */
#define CONNAC3X_WFDMA_TX_DONE_INT17             BIT(27)
#define CONNAC3X_WFDMA_TX_DONE_INT16             BIT(26)
#define CONNAC3X_WFDMA_RX_DONE_INT5              BIT(23)
#define CONNAC3X_WFDMA_RX_DONE_INT4              BIT(22)
#define CONNAC3X_TX_COHERENT_INT                 BIT(21)
#define CONNAC3X_RX_COHERENT_INT                 BIT(20)
#define CONNAC3X_WFDMA_TX_DONE_INT6              BIT(10)
#define CONNAC3X_WFDMA_TX_DONE_INT5              BIT(9)
#define CONNAC3X_WFDMA_TX_DONE_INT4              BIT(8)
#define CONNAC3X_WFDMA_TX_DONE_INT3              BIT(7)
#define CONNAC3X_WFDMA_TX_DONE_INT2              BIT(6)
#define CONNAC3X_WFDMA_TX_DONE_INT1              BIT(5)
#define CONNAC3X_WFDMA_TX_DONE_INT0              BIT(4)
#define CONNAC3X_WFDMA_RX_DONE_INT3              BIT(3)
#define CONNAC3X_WFDMA_RX_DONE_INT2              BIT(2)
#define CONNAC3X_WFDMA_RX_DONE_INT1              BIT(1)
#define CONNAC3X_WFDMA_RX_DONE_INT0              BIT(0)

/* EXT WPDMA_INT_STA (0x53000000+0x10) */
#define CONNAC3X_EXT_WFDMA1_TX_DONE_INT19        BIT(31)
#define CONNAC3X_EXT_WFDMA1_TX_DONE_INT18        BIT(30)
#define CONNAC3X_EXT_WFDMA1_TX_DONE_INT17        BIT(27)
#define CONNAC3X_EXT_WFDMA1_TX_DONE_INT16        BIT(26)
#define CONNAC3X_EXT_WFDMA0_TX_COHERENT_INT      BIT(23)
#define CONNAC3X_EXT_WFDMA0_RX_COHERENT_INT      BIT(22)
#define CONNAC3X_EXT_WFDMA1_TX_COHERENT_INT      BIT(21)
#define CONNAC3X_EXT_WFDMA1_RX_COHERENT_INT      BIT(20)
#define CONNAC3X_EXT_WFDMA0_RX_DONE_INT3         BIT(19)
#define CONNAC3X_EXT_WFDMA0_RX_DONE_INT2         BIT(18)
#define CONNAC3X_EXT_WFDMA0_RX_DONE_INT1         BIT(17)
#define CONNAC3X_EXT_WFDMA0_RX_DONE_INT0         BIT(16)
#define CONNAC3X_EXT_WFDMA1_TX_DONE_INT20        BIT(15)
#define CONNAC3X_EXT_WFDMA1_TX_DONE_INT6         BIT(10)
#define CONNAC3X_EXT_WFDMA1_TX_DONE_INT5         BIT(9)
#define CONNAC3X_EXT_WFDMA1_TX_DONE_INT4         BIT(8)
#define CONNAC3X_EXT_WFDMA1_TX_DONE_INT3         BIT(7)
#define CONNAC3X_EXT_WFDMA1_TX_DONE_INT2         BIT(6)
#define CONNAC3X_EXT_WFDMA1_TX_DONE_INT1         BIT(5)
#define CONNAC3X_EXT_WFDMA1_TX_DONE_INT0         BIT(4)
#define CONNAC3X_EXT_WFDMA1_RX_DONE_INT2         BIT(2)
#define CONNAC3X_EXT_WFDMA1_RX_DONE_INT1         BIT(1)
#define CONNAC3X_EXT_WFDMA1_RX_DONE_INT0         BIT(0)


#define CONNAC3X_HOST_CSR_TOP_BASE     (0x7c060000)
#define CONNAC3X_BN0_LPCTL_ADDR        (CONNAC3X_HOST_CSR_TOP_BASE + 0x10)
#define CONNAC3X_BN0_IRQ_STAT_ADDR     (CONNAC3X_HOST_CSR_TOP_BASE + 0x14)
#define CONNAC3X_BN0_IRQ_ENA_ADDR      (CONNAC3X_HOST_CSR_TOP_BASE + 0x18)
#define CONNAC3X_MAILBOX_DBG_ADDR      (0x18060260)

#if defined(_HIF_USB)
#define CONNAC3X_UDMA_BASE        0x74000000
#define CONNAC3X_UDMA_TX_QSEL     (CONNAC3X_UDMA_BASE + 0x08) /* 0008 */
#define CONNAC3X_UDMA_RESET       (CONNAC3X_UDMA_BASE + 0x14) /* 0014 */
#define CONNAC3X_UDMA_WLCFG_1     (CONNAC3X_UDMA_BASE + 0x0C) /* 000c */
#define CONNAC3X_UDMA_WLCFG_0     (CONNAC3X_UDMA_BASE + 0x18) /* 0018 */

#define CONNAC3X_UDMA_WLCFG_0_WL_TX_BUSY_MASK           (0x1 << 31)
#define CONNAC3X_UDMA_WLCFG_0_WL_TX_EN(p)               (((p) & 0x1) << 23)
#define CONNAC3X_UDMA_WLCFG_0_WL_RX_EN(p)               (((p) & 0x1) << 22)
#define CONNAC3X_UDMA_WLCFG_0_TICK_1US_EN_MASK          (0x1 << 20)
#define CONNAC3X_UDMA_WLCFG_0_TICK_1US_EN(p)            (((p) & 0x1) << 20)
#define CONNAC3X_UDMA_WLCFG_0_WL_RX_FLUSH_MASK          (0x1 << 19)
#define CONNAC3X_UDMA_WLCFG_0_WL_RX_MPSZ_PAD0(p)        (((p) & 0x1) << 18)
#define CONNAC3X_UDMA_WLCFG_0_WL_TX_TMOUT_FUNC_EN_MASK  (0x1 << 16)
#define CONNAC3X_UDMA_WLCFG_1_WL_TX_TMOUT_LMT_MASK      (0xFFFFF << 8)
#define CONNAC3X_UDMA_WLCFG_1_WL_TX_TMOUT_LMT(p)        (((p) & 0xFFFFF) << 8)

#define CONNAC3X_UDMA_TX_TIMEOUT_LIMIT			(50000)

#define CONNAC3X_WFDMA_HOST_CONFIG_ADDR                 0x7c027030
/*
 * 0: command packet forward to TX ring 17 (WMCPU)
 * 1: forward to TX ring 20 (WACPU)
 */
#define CONNAC3X_WFDMA_HOST_CONFIG_USB_CMDPKT_DST_MASK  (0x1 << 7)
#define CONNAC3X_WFDMA_HOST_CONFIG_USB_CMDPKT_DST(p)    (((p) & 0x1) << 7)
#define CONNAC3X_USB_CMDPKT2WM	0
#define CONNAC3X_USB_CMDPKT2WA	1

#define CONNAC3X_WFDMA_HOST_CONFIG_USB_RXEVT_EP4_EN  (0x1 << 6)

#define CONNAC3X_LEN_USB_RX_PADDING_CSO          (4)	/*HW design spec */
#endif /* _HIF_USB */

#define CONN_INFRA_CFG_AP2WF_BUS_ADDR                          0x7C500000

/*------------------------------------------------------------------------*/
/* Rx descriptor field related information                                */
/*------------------------------------------------------------------------*/
/* DW 0 */
#define CONNAC3X_RXD_RX_BYTE_COUNT_MASK           BITS(0, 15)
#define CONNAC3X_RXD_RX_BYTE_COUNT_OFFSET         0

#define CONNAC3X_RXD_PKT_TYPE_MASK                BITS(27, 31)
#define CONNAC3X_RXD_PKT_TYPE_OFFSET              27

#define HAL_CONNAC3X_RXD_GET_PKT_TYPE(_prHwMacRxDesc)	\
	(((_prHwMacRxDesc)->u2DW0 &\
	CONNAC3X_RXD_PKT_TYPE_MASK) >> CONNAC3X_RXD_PKT_TYPE_OFFSET)

/*------------------------------------------------------------------------------
 * MACRO for WTBL INFO GET
 *------------------------------------------------------------------------------
 */

/* CONNAC3X */
#define CONNAC3X_WIFI_LWTBL_BASE 0x820d4000
#define CONNAC3X_WIFI_LWTBL_GROUP_MASK 0x00000007 /* GROUP[2..0] */
#define CONNAC3X_WIFI_LWTBL_GROUP_SHFT 0

#define CONNAC3X_WIFI_UWTBL_BASE 0x820c4000
#define CONNAC3X_WIFI_UWTBL_TARGET_MASK 0x80000000 /* TARGET[31] */
#define CONNAC3X_WIFI_UWTBL_TARGET_SHFT 31
#define CONNAC3X_WIFI_UWTBL_GROUP_MASK 0x0000000F /* GROUP[3..0] */
#define CONNAC3X_WIFI_UWTBL_GROUP_SHFT 0


/* UWTBL DW 5 */
#define CONNAC3X_WTBL_KEY_LINK_DW_KEY_LOC0_MASK BITS(0, 10)
#define CONNAC3X_WTBL_KEY_LINK_DW_KEY_LOC0_OFFSET 0

#define CONNAC3X_WTBL_KEY_LINK_DW_KEY_LOC1_MASK BITS(16, 26)
#define CONNAC3X_WTBL_KEY_LINK_DW_KEY_LOC1_OFFSET 16



#define CONNAC3X_LWTBL_CONFIG(_pAd, _lmacWtblDUAddr, _wlanIdx) \
	HAL_MCR_WR(_pAd, _lmacWtblDUAddr, \
		((_wlanIdx >> 7) & CONNAC3X_WIFI_LWTBL_GROUP_MASK) \
		<< CONNAC3X_WIFI_LWTBL_GROUP_SHFT)

#define CONNAC3X_LWTBL_IDX2BASE(_lmacWtblDUAddr, _wlanIdx, _DW) \
	((_lmacWtblDUAddr & 0xFFFF0000) | 0x8000 | \
	((_wlanIdx & 0x7F) << 8) | (_DW & 0x3F) << 2)

#define CONNAC3X_UWTBL_CONFIG(_pAd, _umacWtblDUAddr, _wlanIdx) \
	HAL_MCR_WR(_pAd, _umacWtblDUAddr, \
		((_wlanIdx >> 7) & CONNAC3X_WIFI_UWTBL_GROUP_MASK) \
		<< CONNAC3X_WIFI_UWTBL_GROUP_SHFT)

#define CONNAC3X_UWTBL_IDX2BASE(_umacWtblDUAddr, _wlanIdx, _DW) \
	((_umacWtblDUAddr & 0XFFFFC000) | 0x2000 | \
	((_wlanIdx & 0x7F) << 6) | (_DW & 0xF) << 2)

#define CONNAC3X_KEYTBL_CONFIG(_pAd, _umacWtblDUAddr, _key_loc) \
	HAL_MCR_WR(_pAd, _umacWtblDUAddr, \
		(CONNAC3X_WIFI_UWTBL_TARGET_MASK | \
		(((_key_loc >> 7) & CONNAC3X_WIFI_UWTBL_GROUP_MASK) \
		<< CONNAC3X_WIFI_UWTBL_GROUP_SHFT)))

#define CONNAC3X_KEYTBL_IDX2BASE(_umacWtblDUAddr, _key_loc, _DW) \
	((_umacWtblDUAddr & 0XFFFFC000) | 0x2000 | \
	((_key_loc & 0x7F) << 6) | (_DW & 0xF) << 2)

/*------------------------------------------------------------------------------
 * MACRO for decision of RXV source (RXD or RX_RPT)
 *------------------------------------------------------------------------------
 */
#define CONNAC3X_RXV_FROM_RX_RPT(_prAdapter)	\
	((_prAdapter)->chip_info->get_rxv_from_rxrpt)

/*------------------------------------------------------------------------------
 * MACRO for CONNAC3X RXVECTOR Parsing
 *------------------------------------------------------------------------------
 */
/* P-RXVector, 1st Cycle */
#define CONNAC3X_RX_VT_RX_RATE_MASK         BITS(0, 6)
#define CONNAC3X_RX_VT_RX_RATE_OFFSET       0
#define CONNAC3X_RX_VT_NSTS_MASK            BITS(7, 9)
#define CONNAC3X_RX_VT_NSTS_OFFSET          7
#define CONNAC3X_RX_VT_LDPC                 BIT(11)

/* C-RXC Vector, 1st Cycle */
#define CONNAC3X_RX_VT_STBC_MASK            BITS(0, 1)
#define CONNAC3X_RX_VT_STBC_OFFSET          0
#define CONNAC3X_RX_VT_RX_MODE_MASK         BITS(4, 7)
#define CONNAC3X_RX_VT_RX_MODE_OFFSET       4
#define CONNAC3X_RX_VT_FR_MODE_MASK         BITS(8, 10)
#define CONNAC3X_RX_VT_FR_MODE_OFFSET       8
#define CONNAC3X_RX_VT_SHORT_GI_MASK		BITS(13, 14)
#define CONNAC3X_RX_VT_SHORT_GI_OFFSET		13
#define CONNAC3X_RX_VT_GROUP_ID_MASK        BITS(22, 27)
#define CONNAC3X_RX_VT_GROUP_ID_OFFSET      22

/* C-RXC Vector, 4th Cycle */
#define CONNAC3X_RX_VT_RCPI0_MASK             BITS(0, 7)
#define CONNAC3X_RX_VT_RCPI0_OFFSET           0
#define CONNAC3X_RX_VT_RCPI1_MASK             BITS(8, 15)
#define CONNAC3X_RX_VT_RCPI1_OFFSET           8
#define CONNAC3X_RX_VT_RCPI2_MASK             BITS(16, 23)
#define CONNAC3X_RX_VT_RCPI2_OFFSET           16
#define CONNAC3X_RX_VT_RCPI3_MASK             BITS(24, 31)
#define CONNAC3X_RX_VT_RCPI3_OFFSET           24

#define CONNAC3X_HAL_RX_VECTOR_GET_RX_VECTOR(_prHwRxVector, _ucIdx) \
	((_prHwRxVector)->u4RxVector[_ucIdx])

#define CONNAC3X_HAL_RXV_GET_RCPI0_RXRPT(_RxvDw6)	\
	(((_RxvDw6) & CONNAC3X_RX_VT_RCPI0_MASK) >> CONNAC3X_RX_VT_RCPI0_OFFSET)

#define CONNAC3X_HAL_RXV_GET_RCPI1_RXRPT(_RxvDw6)	\
	(((_RxvDw6) & CONNAC3X_RX_VT_RCPI1_MASK) >> CONNAC3X_RX_VT_RCPI1_OFFSET)

#define CONNAC3X_HAL_RXV_GET_RCPI2_RXRPT(_RxvDw6)	\
	(((_RxvDw6) & CONNAC3X_RX_VT_RCPI2_MASK) >> CONNAC3X_RX_VT_RCPI2_OFFSET)

#define CONNAC3X_HAL_RXV_GET_RCPI3_RXRPT(_RxvDw6)	\
	(((_RxvDw6) & CONNAC3X_RX_VT_RCPI3_MASK) >> CONNAC3X_RX_VT_RCPI3_OFFSET)

#define CONNAC3X_HAL_RXV_GET_NUM_RX_RXRPT(_RxvDw2)	\
	(((_RxvDw2) & CONNAC3X_RX_VT_NUM_RX_MASK) >>	\
	CONNAC3X_RX_VT_NUM_RX_OFFSET)


#if defined(_HIF_PCIE) || defined(_HIF_AXI)
#define HAL_IS_CONNAC3X_EXT_TX_DONE_INTR(u4IntrStatus, __u4IntrBits) \
	((u4IntrStatus & (__u4IntrBits)) ? TRUE : FALSE)

#define HAL_IS_CONNAC3X_EXT_RX_DONE_INTR(u4IntrStatus, __u4IntrBits) \
	((u4IntrStatus & (__u4IntrBits)) ? TRUE : FALSE)
#endif /* defined(_HIF_PCIE) || defined(_HIF_AXI) */


/*------------------------------------------------------------------------------
 * MACRO for CONNAC3X WTBL TX RATE
 *------------------------------------------------------------------------------
 */
#define CONNAC3X_HW_TX_RATE_TO_MCS(_x)         (((_x) & BITS(0, 5)) >> 0)
#define CONNAC3X_HW_TX_RATE_TO_MODE(_x)        (((_x) & BITS(6, 9)) >> 6)
#define CONNAC3X_HW_TX_RATE_TO_NSS(_x)         (((_x) & BITS(10, 13)) >> 10)
#define CONNAC3X_HW_TX_RATE_TO_STBC(_x)        (((_x) & BIT(14)) >> 14)

/*------------------------------------------------------------------------------
 * MACRO for CONNAC3X TXV
 *------------------------------------------------------------------------------
 */
#define CONNAC3X_TXV_GET_TX_RATE(_x)	((_x)->u4TxV[2] & 0x7f)
#define CONNAC3X_TXV_GET_TX_LDPC(_x)	(((_x)->u4TxV[2] & (0x1 << 7)) >> 7)
#define CONNAC3X_TXV_GET_TX_STBC(_x)	(((_x)->u4TxV[0] & (0x3 << 6)) >> 6)
#define CONNAC3X_TXV_GET_TX_FRMODE(_x)	(((_x)->u4TxV[0] & (0x7 << 8)) >> 8)
#define CONNAC3X_TXV_GET_TX_MODE(_x)	(((_x)->u4TxV[0] & (0xf << 12)) >> 12)
#define CONNAC3X_TXV_GET_TX_NSTS(_x)	(((_x)->u4TxV[2] & (0x7 << 8)) >> 8)
#define CONNAC3X_TXV_GET_TX_PWR(_x)	(((_x)->u4TxV[0] & (0xff << 16)) >> 16)
#define CONNAC3X_TXV_GET_TX_SGI(_x)	(((_x)->u4TxV[1] & (0x3 << 26)) >> 26)

/*******************************************************************************
*                             D A T A   T Y P E S
********************************************************************************
*/

struct CONNAC3X_WIFI_CMD {
	struct HW_MAC_CONNAC3X_TX_DESC rWifiCmdTxD;

	uint16_t u2Length;
	uint16_t u2PqId;

	uint8_t ucCID;
	uint8_t ucPktTypeID;	/* Must be 0x20 (CMD Packet) */
	uint8_t ucSetQuery;
	uint8_t ucSeqNum;

	/* padding fields, hw may auto modify this field */
	uint8_t ucD2B0Rev;
	uint8_t ucExtenCID;	/* Extend CID */
	uint8_t ucS2DIndex;	/* Index for Src to Dst in CMD usage */
	uint8_t ucExtCmdOption;	/* Extend CID option */

	uint8_t ucCmdVersion;
	uint8_t ucReserved2[3];
	uint32_t au4Reserved3[4];	/* padding fields */

	uint8_t aucBuffer[0];
};

#ifdef CFG_SUPPORT_UNIFIED_COMMAND
struct CONNAC3X_WIFI_UNI_CMD {
	struct HW_MAC_CONNAC3X_TX_DESC rWifiCmdTxD;

	uint16_t u2Length;
	uint16_t u2CID;

	uint8_t aucReserved[1];
	uint8_t ucPktTypeID;	/* Must be 0xA0 (CMD Packet) */
	uint8_t ucFragNum;
	uint8_t ucSeqNum;

	uint16_t u2Checksum;
	uint8_t ucS2DIndex;	/* Index for Src to Dst in CMD usage */
	uint8_t ucOption;	/* CID option */

	uint8_t aucReserved2[4];
	uint8_t aucBuffer[0];
};
#endif

union WTBL_LMAC_DW0 {
	struct {
		uint32_t addr_4:8;
		uint32_t addr_5:8;
		uint32_t muar_idx:6;
		uint32_t rc_a1:1;
		uint32_t kid:2;
		uint32_t rc_id:1;
		uint32_t fd:1;
		uint32_t td:1;
		uint32_t rv:1;
		uint32_t rc_a2:1;
		uint32_t wpi_flg:1;
		uint32_t pad:1;
	} field;
	uint32_t word;
};

union WTBL_LMAC_DW1 {
	struct {
		uint32_t addr_0;
	} field;
	uint32_t word;
};

enum WTBL_LMAC_CIPHER_SUIT {
	WTBL_CIPHER_NONE = 0,
	WTBL_CIPHER_WEP_40 = 1,
	WTBL_CIPHER_TKIP_MIC = 2,
	WTBL_CIPHER_TKIP_NO_MIC = 3,
	WTBL_CIPHER_CCMP_128_PMF = 4,
	WTBL_CIPHER_WEP_104 = 5,
	WTBL_CIPHER_BIP_CMAC_128 = 6,
	WTBL_CIPHER_WEP_128 = 7,
	WTBL_CIPHER_WPI_128 = 8,
	WTBL_CIPHER_CCMP_128_CCX = 9,
	WTBL_CIPHER_CCMP_256 = 10,
	WTBL_CIPHER_GCMP_128 = 11,
	WTBL_CIPHER_GCMP_256 = 12,
	WTBL_CIPHER_GCMP_WPI_128 = 13,
};

union WTBL_LMAC_DW2 {
	struct {
		uint32_t aid12:12;
		uint32_t gid_su:1;
		uint32_t spp_en:1;
		uint32_t wpi_even:1;
		uint32_t aad_om:1;
		uint32_t cipher_suit:5;
		uint32_t cipher_suit_igtk:2;
		uint32_t rsvd:1;
		uint32_t sw:1;
		uint32_t ul:1;
		uint32_t tx_ps:1;
		uint32_t qos:1;
		uint32_t ht:1;
		uint32_t vht:1;
		uint32_t he:1;
		uint32_t mesh:1;
	} field;
	uint32_t word;
};

union WTBL_LMAC_DW3 {
	struct {
		uint32_t wmm_q:2;
		uint32_t rxd_dup_mode:2;
		uint32_t vlan_2e_th:1;
		uint32_t pad:3;
		uint32_t pfmu_index:8;
		uint32_t bf_rsvd:8;
		uint32_t ribf:1;
		uint32_t bf_rsvd2:4;
		uint32_t tebf:1;
		uint32_t tebf_vht:1;
		uint32_t tebf_he:1;
	} field;

	struct {
		uint32_t wmm_q:2;
		uint32_t rxd_dup_mode:2;
		uint32_t vlan_2e_th:1;
		uint32_t beam_chg:1;
		uint32_t ba_mode:2;
		uint32_t pfmu_index:8;
		uint32_t ulpf_index:8;
		uint32_t ribf:1;
		uint32_t ulpf:1;
		uint32_t ign_fbk:1;
		uint32_t bf_rsvd2:2;
		uint32_t tebf:1;
		uint32_t tebf_vht:1;
		uint32_t tebf_he:1;
	} field_v2;
	uint32_t word;
};

union WTBL_LMAC_DW4 {
	struct {
		uint32_t ant_id_sts0:3;
		uint32_t ant_id_sts1:3;
		uint32_t ant_id_sts2:3;
		uint32_t ant_id_sts3:3;
		uint32_t ant_id_sts4:3;
		uint32_t ant_id_sts5:3;
		uint32_t ant_id_sts6:3;
		uint32_t ant_id_sts7:3;
		uint32_t cascad:1;
		uint32_t ldpc_ht:1;
		uint32_t ldpc_vht:1;
		uint32_t ldpc_he:1;
		uint32_t dis_rhtr:1;
		uint32_t all_ack:1;
		uint32_t drop:1;
		uint32_t ack_en:1;
	} field;
	uint32_t word;
};

union WTBL_LMAC_DW5 {
	struct {
		uint32_t af:3;
		uint32_t af_he:2;
		uint32_t rts:1;
		uint32_t smps:1;
		uint32_t dyn_bw:1;
		uint32_t mmss:3;
		uint32_t usr:1;
		uint32_t sr_r:2;
		uint32_t beam_chg:1;
		uint32_t sr_abort:1;
		uint32_t tx_power_offset:6;
		uint32_t mpdu_size:2;
		uint32_t pe:2;
		uint32_t doppl:1;
		uint32_t txop_ps_cap:1;
		uint32_t du_i_psm:1;
		uint32_t i_psm:1;
		uint32_t psm:1;
		uint32_t skip_tx:1;
	} field;

	struct {
		uint32_t af:3;
		uint32_t af_he:2;
		uint32_t rts:1;
		uint32_t smps:1;
		uint32_t dyn_bw:1;
		uint32_t mmss:3;
		uint32_t usr:1;
		uint32_t sr_r:3;
		uint32_t sr_abort:1;
		uint32_t tx_power_offset:6;
		uint32_t mpdu_size:2;
		uint32_t pe:2;
		uint32_t doppl:1;
		uint32_t txop_ps_cap:1;
		uint32_t du_i_psm:1;
		uint32_t i_psm:1;
		uint32_t psm:1;
		uint32_t skip_tx:1;
	} field_v2;
	uint32_t word;
};

union WTBL_LMAC_DW6 {
	struct {
		uint32_t ba_win_size_tid0:4;
		uint32_t ba_win_size_tid1:4;
		uint32_t ba_win_size_tid2:4;
		uint32_t ba_win_size_tid3:4;
		uint32_t ba_win_size_tid4:4;
		uint32_t ba_win_size_tid5:4;
		uint32_t ba_win_size_tid6:4;
		uint32_t ba_win_size_tid7:4;
	} field;
	uint32_t word;
};

union WTBL_LMAC_DW7 {
	struct {
		uint32_t cb_rn:3;
		uint32_t dbnss_en:1;
		uint32_t bafen:1;
		uint32_t rdg_ba:1;
		uint32_t r:1;
		uint32_t spe_idx:5;
		uint32_t g2:1;
		uint32_t g4:1;
		uint32_t g8:1;
		uint32_t g16:1;
		uint32_t g2_ltf:2;
		uint32_t g4_ltf:2;
		uint32_t g8_ltf:2;
		uint32_t g16_ltf:2;
		uint32_t g2_he:2;
		uint32_t g4_he:2;
		uint32_t g8_he:2;
		uint32_t g16_he:2;
	} field;
	uint32_t word;
};

union WTBL_LMAC_DW8 {
	struct {
		uint32_t rts_fail_cnt_ac0:5;
		uint32_t rts_fail_cnt_ac1:5;
		uint32_t rts_fail_cnt_ac2:5;
		uint32_t rts_fail_cnt_ac3:5;
		uint32_t partial_aid:9;
		uint32_t pad:2;
		uint32_t chk_per:1;
	} field;
	uint32_t word;
};

union WTBL_LMAC_DW9 {
	struct {
		uint32_t rx_avg_mpdu_size:14;
		uint32_t pad:4;
		uint32_t pritx_dcm:1;
		uint32_t pritx_er160:1;
		uint32_t pritx_ersu:1;
		uint32_t fcap:2;
		uint32_t mpdu_fail_cnt:3;
		uint32_t mpdu_ok_cnt:3;
		uint32_t rate_idx:3;
	} field;

	struct {
		uint32_t rx_avg_mpdu_size:14;
		uint32_t pad:2;
		uint32_t pritx_sw_mode:1;
		uint32_t pritx_plr:1;
		uint32_t pritx_dcm:1;
		uint32_t pritx_er160:1;
		uint32_t pritx_ersu:1;
		uint32_t fcap:2;
		uint32_t mpdu_fail_cnt:3;
		uint32_t mpdu_ok_cnt:3;
		uint32_t rate_idx:3;
	} field_v2;
	uint32_t word;
};

union WTBL_LMAC_DW10 {
	struct {
		uint32_t rate1:14;
		uint32_t pad:2;
		uint32_t rate2:14;
		uint32_t pad2:2;
	} field;
	uint32_t word;
};

union WTBL_LMAC_DW11 {
	struct {
		uint32_t rate3:14;
		uint32_t pad:2;
		uint32_t rate4:14;
		uint32_t pad2:2;
	} field;
	uint32_t word;
};

union WTBL_LMAC_DW12 {
	struct {
		uint32_t rate5:14;
		uint32_t pad:2;
		uint32_t rate6:14;
		uint32_t pad2:2;
	} field;
	uint32_t word;
};

union WTBL_LMAC_DW13 {
	struct {
		uint32_t rate7:14;
		uint32_t pad:2;
		uint32_t rate8:14;
		uint32_t pad2:2;
	} field;
	uint32_t word;
};

union WTBL_LMAC_DW14 {
	struct {
		uint32_t rate_1_tx_cnt:16;
		uint32_t rate_1_fail_cnt:16;
	} field;
	uint32_t word;
};

union WTBL_LMAC_DW15 {
	struct {
		uint32_t rate_2_ok_cnt:16;
		uint32_t rate_3_ok_cnt:16;
	} field;
	uint32_t word;
};

union WTBL_LMAC_DW16 {
	struct {
		uint32_t current_bw_tx_cnt:16;
		uint32_t current_bw_fail_cnt:16;
	} field;
	uint32_t word;
};

union WTBL_LMAC_DW17 {
	struct {
		uint32_t other_bw_tx_cnt:16;
		uint32_t other_bw_fail_cnt:16;
	} field;
	uint32_t word;
};

union WTBL_LMAC_DW18 {
	struct {
		uint32_t rts_ok_cnt:16;
		uint32_t rts_fail_cnt:16;
	} field;
	uint32_t word;
};

union WTBL_LMAC_DW19 {
	struct {
		uint32_t data_retry_cnt:16;
		uint32_t mgnt_retry_cnt:16;
	} field;
	uint32_t word;
};

union WTBL_LMAC_DW20 {
	uint32_t word;
};

union WTBL_LMAC_DW21 {
	uint32_t word;
};

union WTBL_LMAC_DW22 {
	uint32_t word;
};

union WTBL_LMAC_DW23 {
	uint32_t word;
};

union WTBL_LMAC_DW24 {
	uint32_t word;
};

union WTBL_LMAC_DW25 {
	uint32_t word;
};

union WTBL_LMAC_DW26 {
	uint32_t word;
};

union WTBL_LMAC_DW27 {
	uint32_t word;
};

union WTBL_LMAC_DW28 {
	struct {
		uint32_t usr_rssi:9;
		uint32_t usr_snr:6;
		uint32_t pad:1;
		uint32_t rapid_reaction_rate:11;
		uint32_t pad2:3;
		uint32_t ht_amsdu:1;
		uint32_t amsdu_cros_lg:1;
	} field;

	struct {
		uint32_t om_info:12;
		uint32_t rxd_dup_om_chg:1;
		uint32_t pad:19;
	} field_v2;
	uint32_t word;
};

union WTBL_LMAC_DW29 {
	struct {
		uint32_t resp_rcpi_0:8;
		uint32_t resp_rcpi_1:8;
		uint32_t resp_rcpi_2:8;
		uint32_t resp_rcpi_3:8;
	} field;

	struct {
		uint32_t usr_rssi:9;
		uint32_t usr_snr:6;
		uint32_t pad:1;
		uint32_t rapid_reaction_rate:11;
		uint32_t pad2:3;
		uint32_t ht_amsdu:1;
		uint32_t amsdu_cros_lg:1;
	} field_2;
	uint32_t word;
};

union WTBL_LMAC_DW30 {
	struct {
		uint32_t resp_rcpi_4:8;
		uint32_t resp_rcpi_5:8;
		uint32_t resp_rcpi_6:8;
		uint32_t resp_rcpi_7:8;
	} field;

	struct {
		uint32_t resp_rcpi_0:8;
		uint32_t resp_rcpi_1:8;
		uint32_t resp_rcpi_2:8;
		uint32_t resp_rcpi_3:8;
	} field_v2;
	uint32_t word;
};

union WTBL_LMAC_DW31 {
	struct {
		uint32_t snr_rx0:6;
		uint32_t snr_rx1:6;
		uint32_t snr_rx2:6;
		uint32_t snr_rx3:6;
		uint32_t pad:8;
	} field;
	uint32_t word;
};

union WTBL_LMAC_DW32 {
	struct {
		uint32_t snr_rx4:6;
		uint32_t snr_rx5:6;
		uint32_t snr_rx6:6;
		uint32_t snr_rx7:6;
		uint32_t pad:8;
	} field;
	uint32_t word;
};

struct wtbl_rx_stat {
	union WTBL_LMAC_DW28 wtbl_d28;
	union WTBL_LMAC_DW29 wtbl_d29;
	union WTBL_LMAC_DW30 wtbl_d30;
	union WTBL_LMAC_DW31 wtbl_d31;
	union WTBL_LMAC_DW32 wtbl_d32;
};

struct wtbl_adm_ctrl {
	union WTBL_LMAC_DW20 wtbl_d20;
	union WTBL_LMAC_DW21 wtbl_d21;
	union WTBL_LMAC_DW22 wtbl_d22;
	union WTBL_LMAC_DW23 wtbl_d23;
	union WTBL_LMAC_DW24 wtbl_d24;
	union WTBL_LMAC_DW25 wtbl_d25;
	union WTBL_LMAC_DW26 wtbl_d26;
	union WTBL_LMAC_DW27 wtbl_d27;
};

struct wtbl_ppdu_cnt {
	union WTBL_LMAC_DW19 wtbl_d19;
};

struct wtbl_auto_rate_cnt {
	union WTBL_LMAC_DW14 wtbl_d14;
	union WTBL_LMAC_DW15 wtbl_d15;
	union WTBL_LMAC_DW16 wtbl_d16;
	union WTBL_LMAC_DW17 wtbl_d17;
	union WTBL_LMAC_DW18 wtbl_d18;
};

struct wtbl_auto_rate_tb {
	union WTBL_LMAC_DW10 wtbl_d10;
	union WTBL_LMAC_DW11 wtbl_d11;
	union WTBL_LMAC_DW12 wtbl_d12;
	union WTBL_LMAC_DW13 wtbl_d13;
};

struct wtbl_tx_rx_cap {
	union WTBL_LMAC_DW2 wtbl_d2;
	union WTBL_LMAC_DW3 wtbl_d3;
	union WTBL_LMAC_DW4 wtbl_d4;
	union WTBL_LMAC_DW5 wtbl_d5;
	union WTBL_LMAC_DW6 wtbl_d6;
	union WTBL_LMAC_DW7 wtbl_d7;
	union WTBL_LMAC_DW8 wtbl_d8;
	union WTBL_LMAC_DW9 wtbl_d9;
};

struct wtbl_basic_info {
	union WTBL_LMAC_DW0 wtbl_d0;
	union WTBL_LMAC_DW1 wtbl_d1;
};

struct fwtbl_lmac_struct {
	struct wtbl_basic_info peer_basic_info;
	struct wtbl_tx_rx_cap trx_cap;
	struct wtbl_auto_rate_tb auto_rate_tb;
	struct wtbl_auto_rate_cnt auto_rate_counters;
	struct wtbl_ppdu_cnt ppdu_counters;
	struct wtbl_adm_ctrl adm_ctrl;
	struct wtbl_rx_stat rx_stat;
};

union WTBL_UMAC_DW0 {
	struct {
		uint32_t pn0;
	} field;
	uint32_t word;
};

union WTBL_UMAC_DW1 {
	struct {
		uint32_t pn1:16;
		uint32_t com_sn:12;
		uint32_t pad:4;
	} field;
	uint32_t word;
};

union WTBL_UMAC_DW2 {
	struct {
		uint32_t ac0_sn:12;
		uint32_t ac1_sn:12;
		uint32_t ac2_sn:8;
	} field;
	uint32_t word;
};

union WTBL_UMAC_DW3 {
	struct {
		uint32_t ac2_sn:4;
		uint32_t ac3_sn:12;
		uint32_t ac4_sn:12;
		uint32_t ac5_sn:4;
	} field;
	uint32_t word;
};

union WTBL_UMAC_DW4 {
	struct {
		uint32_t ac5_sn:8;
		uint32_t ac6_sn:12;
		uint32_t ac7_sn:12;
	} field;
	uint32_t word;
};

union WTBL_UMAC_DW5 {
	struct {
		uint32_t key_loc0:11;
		uint32_t pad:5;
		uint32_t key_loc1:11;
		uint32_t qos:1;
		uint32_t ht:1;
		uint32_t pad2:3;
	} field;
	uint32_t word;
};

union WTBL_UMAC_DW6 {
	struct {
		uint32_t hw_amsdu_cfg:10;
		uint32_t pad:22;
	} field;
	uint32_t word;
};

union WTBL_UMAC_DW7 {
	struct {
		uint32_t pad:32;
	} field;
	uint32_t word;
};

struct wtbl_key_tb {
	union WTBL_UMAC_DW7 wtbl_d7;
};

struct wtbl_keylink_amsdu {
	union WTBL_UMAC_DW5 wtbl_d5;
	union WTBL_UMAC_DW6 wtbl_d6;
};

struct wtbl_serial_num {
	union WTBL_UMAC_DW0 wtbl_d0;
	union WTBL_UMAC_DW1 wtbl_d1;
	union WTBL_UMAC_DW2 wtbl_d2;
	union WTBL_UMAC_DW3 wtbl_d3;
	union WTBL_UMAC_DW4 wtbl_d4;
};

struct fwtbl_umac_struct {
	struct wtbl_serial_num serial_no;
	struct wtbl_keylink_amsdu klink_amsdu;
	struct wtbl_key_tb key_tb;
};

/*******************************************************************************
*                  F U N C T I O N   D E C L A R A T I O N S
********************************************************************************
*/
void asicConnac3xCapInit(
	struct ADAPTER *prAdapter);
void asicConnac3xFillInitCmdTxd(
	struct ADAPTER *prAdapter,
	struct WIFI_CMD_INFO *prCmdInfo,
	uint16_t *pu2BufInfoLen,
	uint8_t *pucSeqNum,
	void **pCmdBuf);
void asicConnac3xFillCmdTxd(
	struct ADAPTER *prAdapter,
	struct WIFI_CMD_INFO *prCmdInfo,
	uint8_t *pucSeqNum,
	void **pCmdBuf);
#ifdef CFG_SUPPORT_UNIFIED_COMMAND
void asicConnac3xFillUniCmdTxd(
	struct ADAPTER *prAdapter,
	struct WIFI_UNI_CMD_INFO *prCmdInfo,
	uint8_t *pucSeqNum,
	void **pCmdBuf);
#endif
#if defined(_HIF_PCIE) || defined(_HIF_AXI)
uint32_t asicConnac3xWfdmaCfgAddrGet(
	struct GLUE_INFO *prGlueInfo,
	u_int8_t ucDmaIdx);
uint32_t asicConnac3xWfdmaIntRstDtxPtrAddrGet(
	struct GLUE_INFO *prGlueInfo,
	u_int8_t ucDmaIdx);
uint32_t asicConnac3xWfdmaIntRstDrxPtrAddrGet(
	struct GLUE_INFO *prGlueInfo,
	u_int8_t ucDmaIdx);
uint32_t asicConnac3xWfdmaHifRstAddrGet(
	struct GLUE_INFO *prGlueInfo,
	u_int8_t ucDmaIdx);
void asicConnac3xWfdmaStop(struct GLUE_INFO *prGlueInfo, u_int8_t enable);
u_int8_t asicConnac3xWfdmaPollingAllIdle(struct GLUE_INFO *prGlueInfo);
uint8_t asicConnac3xWfdmaWaitIdle(
	struct GLUE_INFO *prGlueInfo,
	uint8_t index,
	uint32_t round,
	uint32_t wait_us);
void asicConnac3xWfdmaTxRingExtCtrl(
	struct GLUE_INFO *prGlueInfo,
	struct RTMP_TX_RING *tx_ring,
	uint32_t index);
void asicConnac3xWfdmaRxRingExtCtrl(
	struct GLUE_INFO *prGlueInfo,
	struct RTMP_RX_RING *rx_ring,
	uint32_t index);
void asicConnac3xEnablePlatformIRQ(
	struct ADAPTER *prAdapter);
void asicConnac3xDisablePlatformIRQ(
	struct ADAPTER *prAdapter);
void asicConnac3xDisablePlatformSwIRQ(
	struct ADAPTER *prAdapter);
void asicConnac3xEnableExtInterrupt(
	struct ADAPTER *prAdapter);
void asicConnac3xLowPowerOwnRead(
	struct ADAPTER *prAdapter,
	uint8_t *pfgResult);
void asicConnac3xLowPowerOwnSet(
	struct ADAPTER *prAdapter,
	uint8_t *pfgResult);
void asicConnac3xLowPowerOwnClear(
	struct ADAPTER *prAdapter,
	uint8_t *pfgResult);
void asicConnac3xProcessSoftwareInterrupt(
	struct ADAPTER *prAdapter);
void asicConnac3xSoftwareInterruptMcu(
	struct ADAPTER *prAdapter, u_int32_t intrBitMask);
void asicConnac3xHifRst(
	struct GLUE_INFO *prGlueInfo);
#endif /* _HIF_PCIE */

void fillConnac3xTxDescTxByteCount(
	struct ADAPTER *prAdapter,
	struct MSDU_INFO *prMsduInfo,
	void *prTxDesc);
void asicConnac3xInitTxdHook(
	struct TX_DESC_OPS_T *prTxDescOps);
void asicConnac3xInitRxdHook(
	struct RX_DESC_OPS_T *prRxDescOps);
#if (CFG_SUPPORT_MSP == 1)
void asicConnac3xRxProcessRxvforMSP(IN struct ADAPTER *prAdapter,
	IN OUT struct SW_RFB *prRetSwRfb);
#endif /* CFG_SUPPORT_MSP == 1 */
uint8_t asicConnac3xRxGetRcpiValueFromRxv(
	IN uint8_t ucRcpiMode,
	IN struct SW_RFB *prSwRfb,
	IN struct ADAPTER *prAdapter);
#if (CFG_SUPPORT_PERF_IND == 1)
void asicConnac3xRxPerfIndProcessRXV(IN struct ADAPTER *prAdapter,
	IN struct SW_RFB *prSwRfb,
	IN uint8_t ucBssIndex);
#endif
void asicConnac3xWfdmaReInit(
	struct ADAPTER *prAdapter);
void asicConnac3xWfdmaDummyCrWrite(
	struct ADAPTER *prAdapter);

#if (CFG_CHIP_RESET_SUPPORT == 1) && (CFG_WMT_RESET_API_SUPPORT == 0)
u_int8_t conn2_rst_L0_notify_step2(void);
#endif

/*******************************************************************************
*                  D E B U G F U N C T I O N   D E C L A R A T I O N S
********************************************************************************
*/
uint32_t asic_connac3x_show_raw_wtbl_info(
	struct GLUE_INFO *prGlueInfo,
	int8_t *pcCommand,
	int32_t i4TotalLen,
	int32_t i4Argc,
	int32_t idx);

char *asic_connac3x_hw_rate_ofdm_str(
	u_int16_t ofdm_idx);

char *asic_connac3x_fwtbl_hw_rate_str(
	uint8_t mode,
	uint16_t rate_idx);

uint32_t asic_connac3x_show_txd_info(
	struct ADAPTER *prAdapter,
	int8_t *pcCommand,
	int32_t i4TotalLen,
	int32_t i4Argc,
	int32_t idx);
uint32_t asic_connac3x_show_umac_wtbl_info(
	struct ADAPTER *prAdapter,
	int8_t *pcCommand,
	int32_t i4TotalLen,
	int32_t i4Argc,
	int32_t idx);
u_int32_t asic_connac3x_show_rx_rate_info(
	struct ADAPTER *prAdapter,
	char *pcCommand,
	int32_t i4TotalLen,
	uint8_t ucStaIdx);
u_int32_t asic_connac3x_show_rx_rssi_info(
	struct ADAPTER *prAdapter,
	char *pcCommand,
	int32_t i4TotalLen,
	uint8_t ucStaIdx);
void asicConnac3xWfdmaControl(
	struct GLUE_INFO *prGlueInfo,
	u_int8_t ucDmaIdx,
	u_int8_t enable);

#if (CFG_DOWNLOAD_DYN_MEMORY_MAP == 1)
uint32_t downloadImgByDynMemMap(IN struct ADAPTER *prAdapter,
	IN uint32_t u4Addr, IN uint32_t u4Len,
	IN uint8_t *pucStartPtr, IN enum ENUM_IMG_DL_IDX_T eDlIdx);
#endif

void asicConnac3xDmashdlSetPlePktMaxPage(
	struct ADAPTER *prAdapter,
	uint16_t u2MaxPage);
void asicConnac3xDmashdlSetPsePktMaxPage(
	struct ADAPTER *prAdapter,
	uint16_t u2MaxPage);
void asicConnac3xDmashdlSetRefill(
	struct ADAPTER *prAdapter,
	uint8_t ucGroup,
	u_int8_t fgEnable);
void asicConnac3xDmashdlSetMaxQuota(
	struct ADAPTER *prAdapter,
	uint8_t ucGroup,
	uint16_t u2MaxQuota);
void asicConnac3xDmashdlSetMinQuota(
	struct ADAPTER *prAdatper,
	uint8_t ucGroup,
	uint16_t u2MinQuota);
void asicConnac3xDmashdlSetQueueMapping(
	struct ADAPTER *prAdapter,
	uint8_t ucQueue,
	uint8_t ucGroup);
void asicConnac3xDmashdlGetPktMaxPage(struct ADAPTER *prAdapter);
void asicConnac3xDmashdlGetRefill(struct ADAPTER *prAdapter);
void asicConnac3xDmashdlGetGroupControl(
	struct ADAPTER *prAdapter,
	uint8_t ucGroup);
void asicConnac3xDmashdlSetSlotArbiter(
	struct ADAPTER *prAdapter,
	u_int8_t fgEnable);
void asicConnac3xDmashdlSetUserDefinedPriority(
	struct ADAPTER *prAdapter,
	uint8_t ucPriority,
	uint8_t ucGroup);
uint32_t asicConnac3xDmashdlGetRsvCount(
	struct ADAPTER *prAdapter,
	uint8_t ucGroup);
uint32_t asicConnac3xDmashdlGetSrcCount(
	struct ADAPTER *prAdapter,
	uint8_t ucGroup);
void asicConnac3xDmashdlGetPKTCount(
	struct ADAPTER *prAdapter,
	uint8_t ucGroup);
void asicConnac3xDmashdlSetOptionalControl(
	struct ADAPTER *prAdapter,
	uint16_t u2HifAckCntTh,
	uint16_t u2HifGupActMap);

#if defined(_HIF_USB)
void asicConnac3xWfdmaInitForUSB(
	struct ADAPTER *prAdapter,
	struct mt66xx_chip_info *prChipInfo);
uint8_t asicConnac3xUsbEventEpDetected(
	struct ADAPTER *prAdapter);
void asicConnac3xEnableUsbCmdTxRing(
	struct ADAPTER *prAdapter,
	u_int8_t ucDstRing);
#if CFG_ENABLE_FW_DOWNLOAD
void asicConnac3xEnableUsbFWDL(
	struct ADAPTER *prAdapter,
	u_int8_t fgEnable);
#endif /* CFG_ENABLE_FW_DOWNLOAD */
void asicConnac3xUdmaRxFlush(
	struct ADAPTER *prAdapter,
	u_int8_t bEnable);
uint16_t asicConnac3xUsbRxByteCount(
	struct ADAPTER *prAdapter,
	struct BUS_INFO *prBusInfo,
	uint8_t *pRXD);
#endif /* _HIF_USB */

#endif /* CFG_SUPPORT_CONNAC3X == 1 */
#endif /* _CMM_ASIC_CONNAC3X_H */

