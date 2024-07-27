/* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause */
/*
 * Copyright (c) 2019 MediaTek Inc.
 */

#ifndef __WF_WFDMA_HOST_DMA0_REGS_H__
#define __WF_WFDMA_HOST_DMA0_REGS_H__

#ifdef __cplusplus
extern "C" {
#endif

/* ****************************************************************************
 */
/* */
/* WF_WFDMA_HOST_DMA0 CR Definitions */
/* */
/* ****************************************************************************
 */

#define WF_WFDMA_HOST_DMA0_BASE 0x7c024000
#define WF_WFDMA_EXT_WRAP_CSR_BASE 0x7c027000

#define WF_WFDMA_HOST_DMA0_WPDMA_DBG_IDX_ADDR                                  \
	(WF_WFDMA_HOST_DMA0_BASE + 0x124) /* 4124 */

#define WF_WFDMA_HOST_DMA0_WPDMA_DBG_PROBE_ADDR                                \
	(WF_WFDMA_HOST_DMA0_BASE + 0x128) /* 4128 */

#define WF_WFDMA_HOST_DMA0_HOST_INT_STA_ADDR                                   \
	(WF_WFDMA_HOST_DMA0_BASE + 0x200) /* 4200 */

#define WF_WFDMA_HOST_DMA0_HOST_INT_ENA_ADDR                                   \
	(WF_WFDMA_HOST_DMA0_BASE + 0x204) /* 4204 */

#define WF_WFDMA_HOST_DMA0_WPDMA_GLO_CFG_ADDR                                  \
	(WF_WFDMA_HOST_DMA0_BASE + 0x208) /* 4208 */
#define WF_WFDMA_HOST_DMA0_WPDMA_GLO_CFG_CSR_DISP_BASE_PTR_CHAIN_EN_MASK       \
	0x00008000 /* CSR_DISP_BASE_PTR_CHAIN_EN[15] */

#define WF_WFDMA_HOST_DMA0_WPDMA_RST_DTX_PTR_ADDR                              \
	(WF_WFDMA_HOST_DMA0_BASE + 0x20C) /* 420C */

#define WF_WFDMA_HOST_DMA0_WPDMA_INT_RX_PRI_SEL                                \
	(WF_WFDMA_HOST_DMA0_BASE + 0x298) /* 4298 */
#define WF_WFDMA_HOST_DMA0_WPDMA_INT_RX_RING2_PRI_SEL_MASK                     \
	0x00000004 /* WPDMA_INT_RX_PRI_SEL[2] */
#define WF_WFDMA_HOST_DMA0_WPDMA_INT_RX_RING3_PRI_SEL_MASK                     \
	0x00000008 /* WPDMA_INT_RX_PRI_SEL[3] */

#define WF_WFDMA_HOST_DMA0_WPDMA_GLO_CFG_EXT2_ADDR                             \
	(WF_WFDMA_HOST_DMA0_BASE + 0x2B8) /* 42B8 */
#define WF_WFDMA_HOST_DMA0_WPDMA_GLO_CFG_EXT2_CSR_TX_HALT_MODE_MASK            \
	0x00040000 /* CSR_TX_HALT_MODE[18] */

#define WF_WFDMA_HOST_DMA0_WPDMA_TX_RING0_CTRL0_ADDR                           \
	(WF_WFDMA_HOST_DMA0_BASE + 0x300) /* 4300 */

#define WF_WFDMA_HOST_DMA0_WPDMA_TX_RING0_CTRL1_ADDR                           \
	(WF_WFDMA_HOST_DMA0_BASE + 0x304) /* 4304 */

#define WF_WFDMA_HOST_DMA0_WPDMA_TX_RING0_CTRL2_ADDR                           \
	(WF_WFDMA_HOST_DMA0_BASE + 0x308) /* 4308 */

#define WF_WFDMA_HOST_DMA0_WPDMA_TX_RING0_CTRL3_ADDR                           \
	(WF_WFDMA_HOST_DMA0_BASE + 0x30c) /* 430C */

#define WF_WFDMA_HOST_DMA0_WPDMA_TX_RING1_CTRL0_ADDR                           \
	(WF_WFDMA_HOST_DMA0_BASE + 0x310) /* 4310 */

#define WF_WFDMA_HOST_DMA0_WPDMA_TX_RING2_CTRL0_ADDR                           \
	(WF_WFDMA_HOST_DMA0_BASE + 0x320) /* 4320 */

#define WF_WFDMA_HOST_DMA0_WPDMA_TX_RING3_CTRL0_ADDR                           \
	(WF_WFDMA_HOST_DMA0_BASE + 0x330) /* 4330 */

#define WF_WFDMA_HOST_DMA0_WPDMA_TX_RING4_CTRL0_ADDR                           \
	(WF_WFDMA_HOST_DMA0_BASE + 0x340) /* 4340 */

#define WF_WFDMA_HOST_DMA0_WPDMA_TX_RING5_CTRL0_ADDR                           \
	(WF_WFDMA_HOST_DMA0_BASE + 0x350) /* 4350 */

#define WF_WFDMA_HOST_DMA0_WPDMA_TX_RING6_CTRL0_ADDR                           \
	(WF_WFDMA_HOST_DMA0_BASE + 0x360) /* 4360 */

#define WF_WFDMA_HOST_DMA0_WPDMA_TX_RING15_CTRL0_ADDR                          \
	(WF_WFDMA_HOST_DMA0_BASE + 0x3F0) /* 43F0 */

#define WF_WFDMA_HOST_DMA0_WPDMA_TX_RING16_CTRL0_ADDR                          \
	(WF_WFDMA_HOST_DMA0_BASE + 0x400) /* 4400 */

#define WF_WFDMA_HOST_DMA0_WPDMA_TX_RING17_CTRL0_ADDR                          \
	(WF_WFDMA_HOST_DMA0_BASE + 0x410) /* 4410 */

#define WF_WFDMA_HOST_DMA0_WPDMA_RX_RING0_CTRL0_ADDR                           \
	(WF_WFDMA_HOST_DMA0_BASE + 0x500) /* 4500 */

#define WF_WFDMA_HOST_DMA0_WPDMA_RX_RING0_CTRL1_ADDR                           \
	(WF_WFDMA_HOST_DMA0_BASE + 0x504) /* 4504 */

#define WF_WFDMA_HOST_DMA0_WPDMA_RX_RING0_CTRL2_ADDR                           \
	(WF_WFDMA_HOST_DMA0_BASE + 0x508) /* 4508 */

#define WF_WFDMA_HOST_DMA0_WPDMA_RX_RING0_CTRL3_ADDR                           \
	(WF_WFDMA_HOST_DMA0_BASE + 0x50c) /* 450C */

#define WF_WFDMA_HOST_DMA0_WPDMA_RX_RING1_CTRL0_ADDR                           \
	(WF_WFDMA_HOST_DMA0_BASE + 0x510) /* 4510 */

#define WF_WFDMA_HOST_DMA0_WPDMA_RX_RING2_CTRL0_ADDR                           \
	(WF_WFDMA_HOST_DMA0_BASE + 0x520) /* 4520 */

#define WF_WFDMA_HOST_DMA0_WPDMA_RX_RING3_CTRL0_ADDR                           \
	(WF_WFDMA_HOST_DMA0_BASE + 0x530) /* 4530 */

#define WF_WFDMA_HOST_DMA0_WPDMA_RX_RING4_CTRL0_ADDR                           \
	(WF_WFDMA_HOST_DMA0_BASE + 0x540) /* 4540 */

#define WF_WFDMA_HOST_DMA0_WPDMA_RX_RING5_CTRL0_ADDR                           \
	(WF_WFDMA_HOST_DMA0_BASE + 0x550) /* 4550 */

#define WF_WFDMA_HOST_DMA0_WPDMA_TX_RING0_EXT_CTRL_ADDR                        \
	(WF_WFDMA_HOST_DMA0_BASE + 0x600) /* 4600 */

#define WF_WFDMA_HOST_DMA0_WPDMA_TX_RING1_EXT_CTRL_ADDR                        \
	(WF_WFDMA_HOST_DMA0_BASE + 0x604) /* 4604 */

#define WF_WFDMA_HOST_DMA0_WPDMA_TX_RING2_EXT_CTRL_ADDR                        \
	(WF_WFDMA_HOST_DMA0_BASE + 0x608) /* 4608 */

#define WF_WFDMA_HOST_DMA0_WPDMA_TX_RING3_EXT_CTRL_ADDR                        \
	(WF_WFDMA_HOST_DMA0_BASE + 0x60C) /* 460C */

#define WF_WFDMA_HOST_DMA0_WPDMA_TX_RING4_EXT_CTRL_ADDR                        \
	(WF_WFDMA_HOST_DMA0_BASE + 0x610) /* 4610 */

#define WF_WFDMA_HOST_DMA0_WPDMA_TX_RING5_EXT_CTRL_ADDR                        \
	(WF_WFDMA_HOST_DMA0_BASE + 0x614) /* 4614 */

#define WF_WFDMA_HOST_DMA0_WPDMA_TX_RING6_EXT_CTRL_ADDR                        \
	(WF_WFDMA_HOST_DMA0_BASE + 0x618) /* 4618 */

#define WF_WFDMA_HOST_DMA0_WPDMA_TX_RING15_EXT_CTRL_ADDR                       \
	(WF_WFDMA_HOST_DMA0_BASE + 0x63C) /* 463C */

#define WF_WFDMA_HOST_DMA0_WPDMA_TX_RING16_EXT_CTRL_ADDR                       \
	(WF_WFDMA_HOST_DMA0_BASE + 0x640) /* 4640 */

#define WF_WFDMA_HOST_DMA0_WPDMA_TX_RING17_EXT_CTRL_ADDR                       \
	(WF_WFDMA_HOST_DMA0_BASE + 0x644) /* 4644 */

#define WF_WFDMA_HOST_DMA0_WPDMA_RX_RING0_EXT_CTRL_ADDR                        \
	(WF_WFDMA_HOST_DMA0_BASE + 0x680) /* 4680 */

#define WF_WFDMA_HOST_DMA0_WPDMA_RX_RING1_EXT_CTRL_ADDR                        \
	(WF_WFDMA_HOST_DMA0_BASE + 0x684) /* 4684 */

#define WF_WFDMA_HOST_DMA0_WPDMA_RX_RING2_EXT_CTRL_ADDR                        \
	(WF_WFDMA_HOST_DMA0_BASE + 0x688) /* 4688 */

#define WF_WFDMA_HOST_DMA0_WPDMA_RX_RING3_EXT_CTRL_ADDR                        \
	(WF_WFDMA_HOST_DMA0_BASE + 0x68C) /* 468C */

#define WF_WFDMA_HOST_DMA0_WPDMA_RX_RING4_EXT_CTRL_ADDR                        \
	(WF_WFDMA_HOST_DMA0_BASE + 0x690) /* 4690 */

#define WF_WFDMA_HOST_DMA0_WPDMA_RX_RING5_EXT_CTRL_ADDR                        \
	(WF_WFDMA_HOST_DMA0_BASE + 0x694) /* 4694 */

#define WF_WFDMA_EXT_WRAP_CSR_WFDMA_HOST_CONFIG_ADDR                           \
	(WF_WFDMA_EXT_WRAP_CSR_BASE + 0x30) /* 7030 */
#define WF_WFDMA_EXT_WRAP_CSR_WFDMA_HOST_CONFIG_pcie_dly_rx_int_en_ADDR        \
	WF_WFDMA_EXT_WRAP_CSR_WFDMA_HOST_CONFIG_ADDR
#define WF_WFDMA_EXT_WRAP_CSR_WFDMA_HOST_CONFIG_pcie_dly_rx_int_en_MASK        \
	0x00000200				  /* pcie_dly_rx_int_en[9] */

#ifdef __cplusplus
}
#endif

#endif /* __WF_WFDMA_HOST_DMA0_REGS_H__ */
