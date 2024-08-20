/* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause */
/*
 * Copyright (c) 2016 MediaTek Inc.
 */
/*! \file   soc3_0.c
*    \brief  Internal driver stack will export
*    the required procedures here for GLUE Layer.
*
*    This file contains all routines which are exported
     from MediaTek 802.11 Wireless LAN driver stack to GLUE Layer.
*/

#ifdef SOC3_0

/*******************************************************************************
*                         C O M P I L E R   F L A G S
********************************************************************************
*/
#define CFG_SUPPORT_VCODE_VDFS 0

/*******************************************************************************
*                    E X T E R N A L   R E F E R E N C E S
********************************************************************************
*/

#include "coda/soc3_0/wf_wfdma_host_dma0.h"
#include "coda/soc3_0/wf_wfdma_host_dma1.h"

#if (CFG_DOWNLOAD_DYN_MEMORY_MAP == 1)
#include "coda/soc3_0/conn_infra_cfg.h"
#endif

#include "precomp.h"
#include "gl_rst.h"
#include "soc3_0.h"

#include <linux/platform_device.h>

#if (CFG_SUPPORT_VCODE_VDFS == 1)
#include <linux/pm_qos.h>
#endif /*#ifndef CFG_SUPPORT_VCODE_VDFS*/

/*******************************************************************************
*                              C O N S T A N T S
********************************************************************************
*/

#define CONN_MCU_CONFG_BASE                0x88000000
#define CONN_MCU_CONFG_COM_REG0_ADDR       (CONN_MCU_CONFG_BASE + 0x200)

#define PATCH_SEMAPHORE_COMM_REG 0
#define PATCH_SEMAPHORE_COMM_REG_PATCH_DONE 1	/* bit0 is for patch. */

#define SW_WORKAROUND_FOR_WFDMA_ISSUE_HWITS00009838 1

/* this is workaround for AXE, do not sync back to trunk*/


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
#if (CFG_SUPPORT_VCODE_VDFS == 1)
static struct pm_qos_request wifi_req;
#endif /*#if (CFG_SUPPORT_VCODE_VDFS == 1)*/

uint8_t *apucsoc3_0FwName[] = {
	(uint8_t *) CFG_FW_FILENAME "_SOC3_0",
	NULL
};

struct ECO_INFO soc3_0_eco_table[] = {
	/* HW version,  ROM version,    Factory version */
	{0x00, 0x00, 0x0}	/* End of table */
};

#if defined(_HIF_PCIE) || defined(_HIF_AXI)
struct PCIE_CHIP_CR_MAPPING soc3_0_bus2chip_cr_mapping[] = {
	/* chip addr, bus addr, range */
	{0x54000000, 0x02000, 0x1000}, /* WFDMA PCIE0 MCU DMA0 */
	{0x55000000, 0x03000, 0x1000}, /* WFDMA PCIE0 MCU DMA1 */
	{0x56000000, 0x04000, 0x1000}, /* WFDMA reserved */
	{0x57000000, 0x05000, 0x1000}, /* WFDMA MCU wrap CR */
	{0x58000000, 0x06000, 0x1000}, /* WFDMA PCIE1 MCU DMA0 (MEM_DMA) */
	{0x59000000, 0x07000, 0x1000}, /* WFDMA PCIE1 MCU DMA1 */
	{0x820c0000, 0x08000, 0x4000}, /* WF_UMAC_TOP (PLE) */
	{0x820c8000, 0x0c000, 0x2000}, /* WF_UMAC_TOP (PSE) */
	{0x820cc000, 0x0e000, 0x2000}, /* WF_UMAC_TOP (PP) */
	{0x820e0000, 0x20000, 0x0400}, /* WF_LMAC_TOP BN0 (WF_CFG) */
	{0x820e1000, 0x20400, 0x0200}, /* WF_LMAC_TOP BN0 (WF_TRB) */
	{0x820e2000, 0x20800, 0x0400}, /* WF_LMAC_TOP BN0 (WF_AGG) */
	{0x820e3000, 0x20c00, 0x0400}, /* WF_LMAC_TOP BN0 (WF_ARB) */
	{0x820e4000, 0x21000, 0x0400}, /* WF_LMAC_TOP BN0 (WF_TMAC) */
	{0x820e5000, 0x21400, 0x0800}, /* WF_LMAC_TOP BN0 (WF_RMAC) */
	{0x820ce000, 0x21c00, 0x0200}, /* WF_LMAC_TOP (WF_SEC) */
	{0x820e7000, 0x21e00, 0x0200}, /* WF_LMAC_TOP BN0 (WF_DMA) */
	{0x820cf000, 0x22000, 0x1000}, /* WF_LMAC_TOP (WF_PF) */
	{0x820e9000, 0x23400, 0x0200}, /* WF_LMAC_TOP BN0 (WF_WTBLOFF) */
	{0x820ea000, 0x24000, 0x0200}, /* WF_LMAC_TOP BN0 (WF_ETBF) */
	{0x820eb000, 0x24200, 0x0400}, /* WF_LMAC_TOP BN0 (WF_LPON) */
	{0x820ec000, 0x24600, 0x0200}, /* WF_LMAC_TOP BN0 (WF_INT) */
	{0x820ed000, 0x24800, 0x0800}, /* WF_LMAC_TOP BN0 (WF_MIB) */
	{0x820ca000, 0x26000, 0x2000}, /* WF_LMAC_TOP BN0 (WF_MUCOP) */
	{0x820d0000, 0x30000, 0x10000}, /* WF_LMAC_TOP (WF_WTBLON) */
	{0x40000000, 0x70000, 0x10000}, /* WF_UMAC_SYSRAM */
	{0x00400000, 0x80000, 0x10000}, /* WF_MCU_SYSRAM */
	{0x00410000, 0x90000, 0x10000}, /* WF_MCU_SYSRAM (configure register) */
	{0x820f0000, 0xa0000, 0x0400}, /* WF_LMAC_TOP BN1 (WF_CFG) */
	{0x820f1000, 0xa0600, 0x0200}, /* WF_LMAC_TOP BN1 (WF_TRB) */
	{0x820f2000, 0xa0800, 0x0400}, /* WF_LMAC_TOP BN1 (WF_AGG) */
	{0x820f3000, 0xa0c00, 0x0400}, /* WF_LMAC_TOP BN1 (WF_ARB) */
	{0x820f4000, 0xa1000, 0x0400}, /* WF_LMAC_TOP BN1 (WF_TMAC) */
	{0x820f5000, 0xa1400, 0x0800}, /* WF_LMAC_TOP BN1 (WF_RMAC) */
	{0x820f7000, 0xa1e00, 0x0200}, /* WF_LMAC_TOP BN1 (WF_DMA) */
	{0x820f9000, 0xa3400, 0x0200}, /* WF_LMAC_TOP BN1 (WF_WTBLOFF) */
	{0x820fa000, 0xa4000, 0x0200}, /* WF_LMAC_TOP BN1 (WF_ETBF) */
	{0x820fb000, 0xa4200, 0x0400}, /* WF_LMAC_TOP BN1 (WF_LPON) */
	{0x820fc000, 0xa4600, 0x0200}, /* WF_LMAC_TOP BN1 (WF_INT) */
	{0x820fd000, 0xa4800, 0x0800}, /* WF_LMAC_TOP BN1 (WF_MIB) */
	{0x820cc000, 0xa5000, 0x2000}, /* WF_LMAC_TOP BN1 (WF_MUCOP) */
	{0x820c4000, 0xa8000, 0x4000}, /* WF_LMAC_TOP (WF_UWTBL)  */
	{0x820b0000, 0xae000, 0x1000}, /* [APB2] WFSYS_ON */
	{0x80020000, 0xb0000, 0x10000}, /* WF_TOP_MISC_OFF */
	{0x81020000, 0xc0000, 0x10000}, /* WF_TOP_MISC_ON */
#if (CFG_DOWNLOAD_DYN_MEMORY_MAP == 1)
	{0x7c500000, 0x50000, 0x10000}, /* CONN_INFRA, dyn mem map */
#endif
	{0x7c020000, 0xd0000, 0x10000}, /* CONN_INFRA, wfdma */
	{0x7c060000, 0xe0000, 0x10000}, /* CONN_INFRA, conn_host_csr_top */
	{0x7c000000, 0xf0000, 0x10000}, /* CONN_INFRA */
	{0x0, 0x0, 0x0} /* End */
};
#elif defined(_HIF_AXI)
struct PCIE_CHIP_CR_MAPPING soc3_0_bus2chip_cr_mapping[] = {
	/* chip addr, bus addr, range */
	{0x54000000, 0x402000, 0x1000}, /* WFDMA PCIE0 MCU DMA0 */
	{0x55000000, 0x403000, 0x1000}, /* WFDMA PCIE0 MCU DMA1 */
	{0x56000000, 0x404000, 0x1000}, /* WFDMA reserved */
	{0x57000000, 0x405000, 0x1000}, /* WFDMA MCU wrap CR */
	{0x58000000, 0x406000, 0x1000}, /* WFDMA PCIE1 MCU DMA0 (MEM_DMA) */
	{0x59000000, 0x407000, 0x1000}, /* WFDMA PCIE1 MCU DMA1 */
	{0x820c0000, 0x408000, 0x4000}, /* WF_UMAC_TOP (PLE) */
	{0x820c8000, 0x40c000, 0x2000}, /* WF_UMAC_TOP (PSE) */
	{0x820cc000, 0x40e000, 0x2000}, /* WF_UMAC_TOP (PP) */
	{0x820e0000, 0x420000, 0x0400}, /* WF_LMAC_TOP BN0 (WF_CFG) */
	{0x820e1000, 0x420400, 0x0200}, /* WF_LMAC_TOP BN0 (WF_TRB) */
	{0x820e2000, 0x420800, 0x0400}, /* WF_LMAC_TOP BN0 (WF_AGG) */
	{0x820e3000, 0x420c00, 0x0400}, /* WF_LMAC_TOP BN0 (WF_ARB) */
	{0x820e4000, 0x421000, 0x0400}, /* WF_LMAC_TOP BN0 (WF_TMAC) */
	{0x820e5000, 0x421400, 0x0800}, /* WF_LMAC_TOP BN0 (WF_RMAC) */
	{0x820ce000, 0x421c00, 0x0200}, /* WF_LMAC_TOP (WF_SEC) */
	{0x820e7000, 0x421e00, 0x0200}, /* WF_LMAC_TOP BN0 (WF_DMA) */
	{0x820cf000, 0x422000, 0x1000}, /* WF_LMAC_TOP (WF_PF) */
	{0x820e9000, 0x423400, 0x0200}, /* WF_LMAC_TOP BN0 (WF_WTBLOFF) */
	{0x820ea000, 0x424000, 0x0200}, /* WF_LMAC_TOP BN0 (WF_ETBF) */
	{0x820eb000, 0x424200, 0x0400}, /* WF_LMAC_TOP BN0 (WF_LPON) */
	{0x820ec000, 0x424600, 0x0200}, /* WF_LMAC_TOP BN0 (WF_INT) */
	{0x820ed000, 0x424800, 0x0800}, /* WF_LMAC_TOP BN0 (WF_MIB) */
	{0x820ca000, 0x426000, 0x2000}, /* WF_LMAC_TOP BN0 (WF_MUCOP) */
	{0x820d0000, 0x430000, 0x10000}, /* WF_LMAC_TOP (WF_WTBLON) */
	{0x40000000, 0x470000, 0x10000}, /* WF_UMAC_SYSRAM */
	{0x00400000, 0x480000, 0x10000}, /* WF_MCU_SYSRAM */
	{0x00410000, 0x490000, 0x10000}, /* WF_MCU_SYSRAM (config register) */
	{0x820f0000, 0x4a0000, 0x0400}, /* WF_LMAC_TOP BN1 (WF_CFG) */
	{0x820f1000, 0x4a0600, 0x0200}, /* WF_LMAC_TOP BN1 (WF_TRB) */
	{0x820f2000, 0x4a0800, 0x0400}, /* WF_LMAC_TOP BN1 (WF_AGG) */
	{0x820f3000, 0x4a0c00, 0x0400}, /* WF_LMAC_TOP BN1 (WF_ARB) */
	{0x820f4000, 0x4a1000, 0x0400}, /* WF_LMAC_TOP BN1 (WF_TMAC) */
	{0x820f5000, 0x4a1400, 0x0800}, /* WF_LMAC_TOP BN1 (WF_RMAC) */
	{0x820f7000, 0x4a1e00, 0x0200}, /* WF_LMAC_TOP BN1 (WF_DMA) */
	{0x820f9000, 0x4a3400, 0x0200}, /* WF_LMAC_TOP BN1 (WF_WTBLOFF) */
	{0x820fa000, 0x4a4000, 0x0200}, /* WF_LMAC_TOP BN1 (WF_ETBF) */
	{0x820fb000, 0x4a4200, 0x0400}, /* WF_LMAC_TOP BN1 (WF_LPON) */
	{0x820fc000, 0x4a4600, 0x0200}, /* WF_LMAC_TOP BN1 (WF_INT) */
	{0x820fd000, 0x4a4800, 0x0800}, /* WF_LMAC_TOP BN1 (WF_MIB) */
	{0x820cc000, 0x4a5000, 0x2000}, /* WF_LMAC_TOP BN1 (WF_MUCOP) */
	{0x820c4000, 0x4a8000, 0x4000}, /* WF_LMAC_TOP (WF_UWTBL)  */
	{0x820b0000, 0x4ae000, 0x1000}, /* [APB2] WFSYS_ON */
	{0x80020000, 0x4b0000, 0x10000}, /* WF_TOP_MISC_OFF */
	{0x81020000, 0x4c0000, 0x10000}, /* WF_TOP_MISC_ON */
	{0x7c500000, 0x500000, 0x10000}, /* CONN_INFRA, dyn mem map */
	{0x7c020000, 0x20000, 0x10000}, /* CONN_INFRA, wfdma */
	{0x7c060000, 0x60000, 0x10000}, /* CONN_INFRA, conn_host_csr_top */
	{0x7c000000, 0x00000, 0x10000}, /* CONN_INFRA, conn_infra_on */
	{0x0, 0x0, 0x0} /* End */
};
#endif
#if defined(_HIF_PCIE) || defined(_HIF_AXI)
static bool soc3_0WfdmaAllocRxRing(
	struct GLUE_INFO *prGlueInfo,
	bool fgAllocMem)
{
	if (!halWpdmaAllocRxRing(prGlueInfo, WFDMA0_RX_RING_IDX_2,
			RX_RING1_SIZE, RXD_SIZE, RX_BUFFER_AGGRESIZE,
			fgAllocMem)) {
		DBGLOG(HAL, ERROR, "AllocWfdmaRxRing fail\n");
		return false;
	}
	if (!halWpdmaAllocRxRing(prGlueInfo, WFDMA0_RX_RING_IDX_3,
			RX_RING1_SIZE, RXD_SIZE, RX_BUFFER_AGGRESIZE,
			fgAllocMem)) {
		DBGLOG(HAL, ERROR, "AllocWfdmaRxRing fail\n");
		return false;
	}
	if (!halWpdmaAllocRxRing(prGlueInfo, WFDMA1_RX_RING_IDX_0,
			RX_RING1_SIZE, RXD_SIZE, RX_BUFFER_AGGRESIZE,
			fgAllocMem)) {
		DBGLOG(HAL, ERROR, "AllocWfdmaRxRing fail\n");
		return false;
	}
	if (!halWpdmaAllocRxRing(prGlueInfo, WFDMA1_RX_RING_IDX_1,
			RX_RING_SIZE, RXD_SIZE, RX_BUFFER_AGGRESIZE,
			fgAllocMem)) {
		DBGLOG(HAL, ERROR, "AllocWfdmaRxRing fail\n");
		return false;
	}
	if (!halWpdmaAllocRxRing(prGlueInfo, WFDMA1_RX_RING_IDX_2,
			RX_RING_SIZE, RXD_SIZE, RX_BUFFER_AGGRESIZE,
			fgAllocMem)) {
		DBGLOG(HAL, ERROR, "AllocWfdmaRxRing fail\n");
		return false;
	}
	return true;
}

#endif /*_HIF_PCIE || _HIF_AXI */

void soc3_0asicConnac2xProcessTxInterrupt(IN struct ADAPTER *prAdapter)
{
	struct GL_HIF_INFO *prHifInfo = &prAdapter->prGlueInfo->rHifInfo;
	union WPDMA_INT_STA_STRUCT rIntrStatus;

	rIntrStatus = (union WPDMA_INT_STA_STRUCT)prHifInfo->u4IntStatus;
	if (rIntrStatus.field_conn2x_ext.wfdma1_tx_done_16)
		halWpdmaProcessCmdDmaDone(prAdapter->prGlueInfo,
			TX_RING_FWDL_IDX_3);

	if (rIntrStatus.field_conn2x_ext.wfdma1_tx_done_17)
		halWpdmaProcessCmdDmaDone(prAdapter->prGlueInfo,
			TX_RING_CMD_IDX_2);

	if (rIntrStatus.field_conn2x_ext.wfdma1_tx_done_0) {
		halWpdmaProcessDataDmaDone(prAdapter->prGlueInfo,
			TX_RING_DATA0_IDX_0);
#if CFG_SUPPORT_MULTITHREAD
		if (!HAL_IS_TX_DIRECT())
			kalSetTxEvent2Hif(prAdapter->prGlueInfo);
#endif
	}

}

void soc3_0asicConnac2xProcessRxInterrupt(
	struct ADAPTER *prAdapter)
{
	struct GL_HIF_INFO *prHifInfo = &prAdapter->prGlueInfo->rHifInfo;
	union WPDMA_INT_STA_STRUCT rIntrStatus;

	rIntrStatus = (union WPDMA_INT_STA_STRUCT)prHifInfo->u4IntStatus;
	if (rIntrStatus.field_conn2x_ext.wfdma1_rx_done_0 ||
		(prAdapter->u4NoMoreRfb & BIT(WFDMA1_RX_RING_IDX_0)))
		halRxReceiveRFBs(prAdapter, WFDMA1_RX_RING_IDX_0, FALSE);

	if (rIntrStatus.field_conn2x_ext.wfdma0_rx_done_0 ||
		(prAdapter->u4NoMoreRfb & BIT(RX_RING_DATA_IDX_0)))
		halRxReceiveRFBs(prAdapter, RX_RING_DATA_IDX_0, TRUE);

	if (rIntrStatus.field_conn2x_ext.wfdma0_rx_done_1 ||
		(prAdapter->u4NoMoreRfb & BIT(RX_RING_EVT_IDX_1)))
		halRxReceiveRFBs(prAdapter, RX_RING_EVT_IDX_1, TRUE);

	if (rIntrStatus.field_conn2x_ext.wfdma0_rx_done_2 ||
		(prAdapter->u4NoMoreRfb & BIT(WFDMA0_RX_RING_IDX_2)))
		halRxReceiveRFBs(prAdapter, WFDMA0_RX_RING_IDX_2, TRUE);

	if (rIntrStatus.field_conn2x_ext.wfdma0_rx_done_3 ||
		(prAdapter->u4NoMoreRfb & BIT(WFDMA0_RX_RING_IDX_3)))
		halRxReceiveRFBs(prAdapter, WFDMA0_RX_RING_IDX_3, TRUE);
}

void soc3_0asicConnac2xWfdmaManualPrefetch(
	struct GLUE_INFO *prGlueInfo)
{
	struct ADAPTER *prAdapter = prGlueInfo->prAdapter;
	u_int32_t val = 0;

	HAL_MCR_RD(prAdapter, WF_WFDMA_HOST_DMA0_WPDMA_GLO_CFG_ADDR, &val);
	/* disable prefetch offset calculation auto-mode */
	val &=
	~WF_WFDMA_HOST_DMA0_WPDMA_GLO_CFG_CSR_DISP_BASE_PTR_CHAIN_EN_MASK;
	HAL_MCR_WR(prAdapter, WF_WFDMA_HOST_DMA0_WPDMA_GLO_CFG_ADDR, val);

	HAL_MCR_RD(prAdapter, WF_WFDMA_HOST_DMA1_WPDMA_GLO_CFG_ADDR, &val);
	/* disable prefetch offset calculation auto-mode */
	val &=
	~WF_WFDMA_HOST_DMA1_WPDMA_GLO_CFG_CSR_DISP_BASE_PTR_CHAIN_EN_MASK;
	HAL_MCR_WR(prAdapter,
		WF_WFDMA_HOST_DMA1_WPDMA_GLO_CFG_ADDR, val);

	HAL_MCR_WR(prAdapter,
		WF_WFDMA_HOST_DMA0_WPDMA_RX_RING0_EXT_CTRL_ADDR, 0x00000004);
	HAL_MCR_WR(prAdapter,
		WF_WFDMA_HOST_DMA0_WPDMA_RX_RING1_EXT_CTRL_ADDR, 0x00400004);
	HAL_MCR_WR(prAdapter,
		WF_WFDMA_HOST_DMA0_WPDMA_RX_RING2_EXT_CTRL_ADDR, 0x00800004);
	HAL_MCR_WR(prAdapter,
		WF_WFDMA_HOST_DMA0_WPDMA_RX_RING3_EXT_CTRL_ADDR, 0x00c00004);

#if (SW_WORKAROUND_FOR_WFDMA_ISSUE_HWITS00009838 == 1)
	HAL_MCR_WR(prAdapter,
		WF_WFDMA_HOST_DMA0_WPDMA_RX_RING4_EXT_CTRL_ADDR, 0x01000004);
#endif

	HAL_MCR_WR(prAdapter,
		WF_WFDMA_HOST_DMA1_WPDMA_TX_RING0_EXT_CTRL_ADDR, 0x01400004);
	HAL_MCR_WR(prAdapter,
		WF_WFDMA_HOST_DMA1_WPDMA_TX_RING1_EXT_CTRL_ADDR, 0x01800004);
	HAL_MCR_WR(prAdapter,
		WF_WFDMA_HOST_DMA1_WPDMA_TX_RING2_EXT_CTRL_ADDR, 0x01c00004);

	HAL_MCR_WR(prAdapter,
		WF_WFDMA_HOST_DMA1_WPDMA_TX_RING3_EXT_CTRL_ADDR, 0x02000004);
	HAL_MCR_WR(prAdapter,
		WF_WFDMA_HOST_DMA1_WPDMA_TX_RING4_EXT_CTRL_ADDR, 0x02400004);
	HAL_MCR_WR(prAdapter,
		WF_WFDMA_HOST_DMA1_WPDMA_TX_RING5_EXT_CTRL_ADDR, 0x02800004);
	HAL_MCR_WR(prAdapter,
		WF_WFDMA_HOST_DMA1_WPDMA_TX_RING6_EXT_CTRL_ADDR, 0x02c00004);

#if (SW_WORKAROUND_FOR_WFDMA_ISSUE_HWITS00009838 == 1)
	HAL_MCR_WR(prAdapter,
		WF_WFDMA_HOST_DMA1_WPDMA_TX_RING7_EXT_CTRL_ADDR, 0x03000004);
#endif

	HAL_MCR_WR(prAdapter,
		WF_WFDMA_HOST_DMA1_WPDMA_TX_RING16_EXT_CTRL_ADDR, 0x03400004);
	HAL_MCR_WR(prAdapter,
		WF_WFDMA_HOST_DMA1_WPDMA_TX_RING17_EXT_CTRL_ADDR, 0x03800004);

#if (SW_WORKAROUND_FOR_WFDMA_ISSUE_HWITS00009838 == 1)
	HAL_MCR_WR(prAdapter,
		WF_WFDMA_HOST_DMA1_WPDMA_TX_RING18_EXT_CTRL_ADDR, 0x03c00004);
#endif

	HAL_MCR_WR(prAdapter,
		WF_WFDMA_HOST_DMA1_WPDMA_RX_RING0_EXT_CTRL_ADDR, 0x04800004);
	HAL_MCR_WR(prAdapter,
		WF_WFDMA_HOST_DMA1_WPDMA_RX_RING1_EXT_CTRL_ADDR, 0x04C00004);

#if (SW_WORKAROUND_FOR_WFDMA_ISSUE_HWITS00009838 == 1)
	HAL_MCR_WR(prAdapter,
		WF_WFDMA_HOST_DMA1_WPDMA_RX_RING2_EXT_CTRL_ADDR, 0x05000004);
#endif

	/* reset dma idx */
	HAL_MCR_WR(prAdapter,
		WF_WFDMA_HOST_DMA0_WPDMA_RST_DTX_PTR_ADDR, 0xFFFFFFFF);
	HAL_MCR_WR(prAdapter,
		WF_WFDMA_HOST_DMA1_WPDMA_RST_DTX_PTR_ADDR, 0xFFFFFFFF);
}

struct BUS_INFO soc3_0_bus_info = {
#if defined(_HIF_PCIE) || defined(_HIF_AXI)
	.top_cfg_base = SOC3_0_TOP_CFG_BASE,

	/* host_dma0 for TXP */
	.host_dma0_base = CONNAC2X_HOST_WPDMA_0_BASE,
	/* host_dma1 for TXD and host cmd to WX_CPU */
	.host_dma1_base = CONNAC2X_HOST_WPDMA_1_BASE,
	.host_ext_conn_hif_wrap_base = CONNAC2X_HOST_EXT_CONN_HIF_WRAP,
	.host_int_status_addr =
		CONNAC2X_WPDMA_EXT_INT_STA(CONNAC2X_HOST_EXT_CONN_HIF_WRAP),

	.host_int_txdone_bits = (CONNAC2X_EXT_WFDMA1_TX_DONE_INT0
				| CONNAC2X_EXT_WFDMA1_TX_DONE_INT1
				| CONNAC2X_EXT_WFDMA1_TX_DONE_INT2
				| CONNAC2X_EXT_WFDMA1_TX_DONE_INT3
				| CONNAC2X_EXT_WFDMA1_TX_DONE_INT4
				| CONNAC2X_EXT_WFDMA1_TX_DONE_INT5
				| CONNAC2X_EXT_WFDMA1_TX_DONE_INT6
				| CONNAC2X_EXT_WFDMA1_TX_DONE_INT16
				| CONNAC2X_EXT_WFDMA1_TX_DONE_INT17),
	.host_int_rxdone_bits = (CONNAC2X_EXT_WFDMA1_RX_DONE_INT0
				| CONNAC2X_EXT_WFDMA0_RX_DONE_INT0
				| CONNAC2X_EXT_WFDMA0_RX_DONE_INT1
				| CONNAC2X_EXT_WFDMA0_RX_DONE_INT2
				| CONNAC2X_EXT_WFDMA0_RX_DONE_INT3
				),

	.host_tx_ring_base =
		CONNAC2X_TX_RING_BASE(CONNAC2X_HOST_WPDMA_1_BASE),
	.host_tx_ring_ext_ctrl_base =
		CONNAC2X_TX_RING_EXT_CTRL_BASE(CONNAC2X_HOST_WPDMA_1_BASE),
	.host_tx_ring_cidx_addr =
		CONNAC2X_TX_RING_CIDX(CONNAC2X_HOST_WPDMA_1_BASE),
	.host_tx_ring_didx_addr =
		CONNAC2X_TX_RING_DIDX(CONNAC2X_HOST_WPDMA_1_BASE),
	.host_tx_ring_cnt_addr =
		CONNAC2X_TX_RING_CNT(CONNAC2X_HOST_WPDMA_1_BASE),

	.host_rx_ring_base =
		CONNAC2X_RX_RING_BASE(CONNAC2X_HOST_WPDMA_0_BASE),
	.host_rx_ring_ext_ctrl_base =
		CONNAC2X_RX_RING_EXT_CTRL_BASE(CONNAC2X_HOST_WPDMA_0_BASE),
	.host_rx_ring_cidx_addr =
		CONNAC2X_RX_RING_CIDX(CONNAC2X_HOST_WPDMA_0_BASE),
	.host_rx_ring_didx_addr =
		CONNAC2X_RX_RING_DIDX(CONNAC2X_HOST_WPDMA_0_BASE),
	.host_rx_ring_cnt_addr =
		CONNAC2X_RX_RING_CNT(CONNAC2X_HOST_WPDMA_0_BASE),

	.host_wfdma1_rx_ring_base =
		CONNAC2X_WFDMA1_RX_RING_BASE(CONNAC2X_HOST_WPDMA_1_BASE),
	.host_wfdma1_rx_ring_cidx_addr =
		CONNAC2X_WFDMA1_RX_RING_CIDX(CONNAC2X_HOST_WPDMA_1_BASE),
	.host_wfdma1_rx_ring_didx_addr =
		CONNAC2X_WFDMA1_RX_RING_DIDX(CONNAC2X_HOST_WPDMA_1_BASE),
	.host_wfdma1_rx_ring_cnt_addr =
		CONNAC2X_WFDMA1_RX_RING_CNT(CONNAC2X_HOST_WPDMA_1_BASE),
	.host_wfdma1_rx_ring_ext_ctrl_base =
		CONNAC2X_WFDMA1_RX_RING_EXT_CTRL_BASE(
			CONNAC2X_HOST_WPDMA_1_BASE),

	.bus2chip = soc3_0_bus2chip_cr_mapping,
#if defined(_HIF_PCIE)
	.max_static_map_addr = 0x000f0000,
#elif defined(_HIF_AXI)
			.max_static_map_addr = 0x00700000,
#endif

	.tx_ring_fwdl_idx = CONNAC2X_FWDL_TX_RING_IDX,
	.tx_ring_cmd_idx = CONNAC2X_CMD_TX_RING_IDX,
	.tx_ring0_data_idx = 0,
	.tx_ring1_data_idx = 1,
	.fw_own_clear_addr = CONNAC2X_BN0_IRQ_STAT_ADDR,
	.fw_own_clear_bit = PCIE_LPCR_FW_CLR_OWN,
	.fgCheckDriverOwnInt = FALSE,
	.u4DmaMask = 32,
	.pdmaSetup = asicConnac2xWpdmaConfig,
	.pdmaStop = NULL,
	.pdmaPollingIdle = NULL,
	.enableInterrupt = asicConnac2xEnableExtInterrupt,
	.disableInterrupt = asicConnac2xDisableExtInterrupt,
	.processTxInterrupt = soc3_0asicConnac2xProcessTxInterrupt,
	.processRxInterrupt = soc3_0asicConnac2xProcessRxInterrupt,
	.tx_ring_ext_ctrl = asicConnac2xWfdmaTxRingExtCtrl,
	.rx_ring_ext_ctrl = asicConnac2xWfdmaRxRingExtCtrl,
	/* null wfdmaManualPrefetch if want to disable manual mode */
	.wfdmaManualPrefetch = soc3_0asicConnac2xWfdmaManualPrefetch,
	.lowPowerOwnRead = asicConnac2xLowPowerOwnRead,
	.lowPowerOwnSet = asicConnac2xLowPowerOwnSet,
	.lowPowerOwnClear = asicConnac2xLowPowerOwnClear,
	.wakeUpWiFi = asicWakeUpWiFi,
	.processSoftwareInterrupt = asicConnac2xProcessSoftwareInterrupt,
	.softwareInterruptMcu = asicConnac2xSoftwareInterruptMcu,
	.hifRst = asicConnac2xHifRst,

	.initPcieInt = NULL,
	.devReadIntStatus = asicConnac2xReadExtIntStatus,
	.DmaShdlInit = NULL,
	.DmaShdlReInit = NULL,
	.wfdmaAllocRxRing = soc3_0WfdmaAllocRxRing,
#endif			/*_HIF_PCIE || _HIF_AXI */
};

#if CFG_ENABLE_FW_DOWNLOAD
struct FWDL_OPS_T soc3_0_fw_dl_ops = {
	.constructFirmwarePrio = NULL,
	.constructPatchName = NULL,
	.downloadPatch = wlanDownloadPatch,
	.downloadFirmware = wlanConnacFormatDownload,
#if (CFG_DOWNLOAD_DYN_MEMORY_MAP == 1)
	.downloadByDynMemMap = soc3_0_DownloadByDynMemMap,
#else
	.downloadByDynMemMap = NULL,
#endif
	.getFwInfo = wlanGetConnacFwInfo,
	.getFwDlInfo = asicGetFwDlInfo,
};
#endif			/* CFG_ENABLE_FW_DOWNLOAD */

struct TX_DESC_OPS_T soc3_0_TxDescOps = {
	.fillNicAppend = fillNicTxDescAppend,
	.fillHifAppend = fillTxDescAppendByHostV2,
	.fillTxByteCount = fillConnac2xTxDescTxByteCount,
};

struct RX_DESC_OPS_T soc3_0_RxDescOps = {
};

struct CHIP_DBG_OPS soc3_0_debug_ops = {
	.showPdmaInfo = soc3_0_show_wfdma_info,
	.showPseInfo = soc3_0_show_pse_info,
	.showPleInfo = soc3_0_show_ple_info,
	.showTxdInfo = connac2x_show_txd_Info,
	.showWtblInfo = connac2x_show_wtbl_info,
	.showUmacFwtblInfo = connac2x_show_umac_wtbl_info,
	.showCsrInfo = NULL,
	.showDmaschInfo = NULL,
	.showHifInfo = NULL,
	.printHifDbgInfo = NULL,
	.show_rx_rate_info = connac2x_show_rx_rate_info,
	.show_rx_rssi_info = connac2x_show_rx_rssi_info,
	.show_stat_info = connac2x_show_stat_info,
	.show_mcu_debug_info = NULL,
	.show_conninfra_debug_info = NULL,
};

#if CFG_SUPPORT_QA_TOOL
struct ATE_OPS_T soc3_0_AteOps = {
	/*ICapStart phase out , wlan_service instead*/
	.setICapStart = connacSetICapStart,
	/*ICapStatus phase out , wlan_service instead*/
	.getICapStatus = connacGetICapStatus,
	/*CapIQData phase out , wlan_service instead*/
	.getICapIQData = connacGetICapIQData,
	.getRbistDataDumpEvent = nicExtEventICapIQData,
	.icapRiseVcoreClockRate = soc3_0_icapRiseVcoreClockRate,
	.icapDownVcoreClockRate = soc3_0_icapDownVcoreClockRate,
	.u4EnBitWidth = 0, /*32 bit*/
	.u4Architech = 1,  /*1:on-the-fly*/
	.u4PhyIdx = 0,
#if (CFG_MTK_ANDROID_EMI == 1)
	.u4EmiStartAddress =
	    (uint32_t) (gConEmiPhyBase & 0xFFFFFFFF);
	.u4EmiEndAddress =
	    (uint32_t) ((gConEmiPhyBase + gConEmiSize) & 0xFFFFFFFF);
	.u4EmiMsbAddress =
	    (uint32_t) ((((uint64_t) gConEmiPhyBase) >> 32) & 0xFFFFFFFF);
#else
	.u4EmiStartAddress = 0,
	.u4EmiEndAddress = 0,
	.u4EmiMsbAddress = 0,
#endif
};
#endif


/* Litien code refine to support multi chip */
struct mt66xx_chip_info mt66xx_chip_info_soc3_0 = {
	.bus_info = &soc3_0_bus_info,
#if CFG_ENABLE_FW_DOWNLOAD
	.fw_dl_ops = &soc3_0_fw_dl_ops,
#endif				/* CFG_ENABLE_FW_DOWNLOAD */
#if CFG_SUPPORT_QA_TOOL
	.prAteOps = &soc3_0_AteOps,
#endif /*CFG_SUPPORT_QA_TOOL*/
	.prDebugOps = &soc3_0_debug_ops,
	.prTxDescOps = &soc3_0_TxDescOps,
	.prRxDescOps = &soc3_0_RxDescOps,
	.chip_id = SOC3_0_CHIP_ID,
	.should_verify_chip_id = FALSE,
	.sw_sync0 = SOC3_0_SW_SYNC0,
	.sw_ready_bits = WIFI_FUNC_NO_CR4_READY_BITS,
	.sw_ready_bit_offset = SOC3_0_SW_SYNC0_RDY_OFFSET,
	.patch_addr = SOC3_0_PATCH_START_ADDR,
	.is_support_cr4 = FALSE,
	.is_support_wacpu = FALSE,
	.txd_append_size = SOC3_0_TX_DESC_APPEND_LENGTH,
	.rxd_size = SOC3_0_RX_DESC_LENGTH,

	.pse_header_length = CONNAC2X_NIC_TX_PSE_HEADER_LENGTH,
	.init_event_size = CONNAC2X_RX_INIT_EVENT_LENGTH,
	.eco_info = soc3_0_eco_table,
	.isNicCapV1 = FALSE,
	.top_hcr = CONNAC2X_TOP_HCR,
	.top_hvr = CONNAC2X_TOP_HVR,
	.top_fvr = CONNAC2X_TOP_FVR,
	.arb_ac_mode_addr = SOC3_0_ARB_AC_MODE_ADDR,
	.asicCapInit = asicConnac2xCapInit,
#if CFG_ENABLE_FW_DOWNLOAD
	.asicEnableFWDownload = NULL,
#endif				/* CFG_ENABLE_FW_DOWNLOAD */
	.asicGetChipID = asicGetChipID,
	.downloadBufferBin = wlanConnacDownloadBufferBin,
	.is_support_hw_amsdu = TRUE,
	.is_support_asic_lp = TRUE,
	.is_support_wfdma1 = TRUE,
	.is_support_nvram_fragment = TRUE,
	.asicWfdmaReInit = asicConnac2xWfdmaReInit,
	.asicWfdmaReInit_handshakeInit = asicConnac2xWfdmaDummyCrWrite,
	.group5_size = sizeof(struct HW_MAC_RX_STS_GROUP_5),
	.u4LmacWtblDUAddr = CONNAC2X_WIFI_LWTBL_BASE,
	.u4UmacWtblDUAddr = CONNAC2X_WIFI_UWTBL_BASE,
	.wmmcupwron = hifWmmcuPwrOn,
	.wmmcupwroff = hifWmmcuPwrOff,
	.triggerfwassert = soc3_0_Trigger_fw_assert,
#if (CFG_SUPPORT_CONNINFRA == 1)
	.trigger_wholechiprst = soc3_0_Trigger_whole_chip_rst,
	.sw_interrupt_handler = soc3_0_Sw_interrupt_handler
#endif

#if CFG_CHIP_RESET_SUPPORT
	.asicWfsysRst = NULL,
	.asicPollWfsysSwInitDone = NULL,
#endif

	/* leave it to project owner */
	.uc2G4HeCapMaxAmpduLenExp = 0,
	.uc5GHeCapMaxAmpduLenExp = 0,
};

struct mt66xx_hif_driver_data mt66xx_driver_data_soc3_0 = {
	.chip_info = &mt66xx_chip_info_soc3_0,
};

#if (CFG_DOWNLOAD_DYN_MEMORY_MAP == 1)
uint32_t soc3_0_DownloadByDynMemMap(IN struct ADAPTER *prAdapter,
	IN uint32_t u4Addr, IN uint32_t u4Len,
	IN uint8_t *pucStartPtr, IN enum ENUM_IMG_DL_IDX_T eDlIdx)
{
	uint32_t u4Val = 0;
	struct GL_HIF_INFO *prHifInfo = &prAdapter->prGlueInfo->rHifInfo;
	uint32_t u4Idx = 0;
	uint32_t u4NonZeroMemCnt = 0;

	if ((eDlIdx == IMG_DL_IDX_PATCH) || (eDlIdx == IMG_DL_IDX_N9_FW)) {
#if defined(SOC3_0)
/*#pragma message("wlanDownloadSectionByDynMemMap()::SOC3_0")*/
#if defined(_HIF_AXI)
		/* AXI goes over here */
#endif

#if defined(_HIF_PCIE)
		HAL_MCR_RD(prAdapter,
			CONN_INFRA_CFG_PCIE2AP_REMAP_2_ADDR, &u4Val);

		DBGLOG(INIT, WARN, "[MJ]ORIG(0x%08x) = 0x%08x\n",
			CONN_INFRA_CFG_PCIE2AP_REMAP_2_ADDR, u4Val);

		/*
		 * 0x18=0x7C
		 * 0x18001120[31:0] = 0x00100000
		 * 0x18500000  (AP) is 0x00100000(MCU)
		 * 0x18001198[31:16] = 0x1850
		 * 0x18001198 = 0x1850|Value[15:0]

		u4Val = ((~BITS(31,16) & u4Val) | ((0x1850) << 16));
		*/

		/* this hard code is verified with DE for SOC3_0 */
		u4Val = CONN_INFRA_CFG_PCIE2AP_REMAP_2_ADDR_DE_HARDCODE;

		HAL_MCR_WR(prAdapter,
			CONN_INFRA_CFG_PCIE2AP_REMAP_2_ADDR, u4Val);

		HAL_MCR_RD(prAdapter,
			CONN_INFRA_CFG_PCIE2AP_REMAP_2_ADDR, &u4Val);

		DBGLOG(INIT, WARN, "[MJ]NEW(0x%08x) = 0x%08x\n",
			CONN_INFRA_CFG_PCIE2AP_REMAP_2_ADDR, u4Val);

#if 1
		/* PCIe workable version by bytes of u4Len in one movement */
		/* for PCIe WLAN drv, use 0x7C000000 based;
		 * for AXI, use 0x18000000 based
		 */
		HAL_MCR_WR(prAdapter,
			CONN_INFRA_CFG_AP2WF_REMAP_1_ADDR,
			u4Addr);

		if (eDlIdx == IMG_DL_IDX_PATCH) {
			do {
				u4Val = 0;

				HAL_MCR_RD(prAdapter,
				(CONN_INFRA_CFG_AP2WF_BUS_ADDR +
				(u4Idx * 4)),	&u4Val);

				if (u4Val != 0)
					u4NonZeroMemCnt++;
			} while (u4Idx++ < 20);

			if (u4NonZeroMemCnt != 0) {
				DBGLOG(INIT, WARN,
					"[MJ]ROM patch exists(%d)!\n",
					u4NonZeroMemCnt);

				return WLAN_STATUS_NOT_ACCEPTED;
			}
		}

		/* because
		* soc3_0_bus2chip_cr_mapping =
		*	{0x7c500000, 0x50000, 0x10000}
		*/
		kalMemCopy((void *)(prHifInfo->CSRBaseAddress+0x50000),
			(void *)pucStartPtr, u4Len);
#else
		/* PCIe workable version by 4 bytes in one movement */
		/* for PCIe WLAN drv, use 0x7C000000 based;
		 * for AXI, use 0x18000000 based
		 */
		HAL_MCR_WR(prAdapter,
			CONN_INFRA_CFG_AP2WF_REMAP_1_ADDR,
			u4Addr);

		for (u4Offset = 0; u4Len > 0; u4Offset += u4RemapSize) {
			if (u4Len > 4)
				u4RemapSize = 4;
			else
				u4RemapSize = u4Len;

			u4Len -= u4RemapSize;

			HAL_MCR_WR(prAdapter,
			(CONN_INFRA_CFG_AP2WF_BUS_ADDR + u4Offset),
			(uint32_t)
			(*(uint32_t *)((uint8_t *)pucStartPtr + u4Offset)));

			HAL_MCR_RD(prAdapter,
			(CONN_INFRA_CFG_AP2WF_BUS_ADDR + u4Offset), &u4Val);

			/* You can uncomment it to see what is downloaded
			*if (u4Idx++ < 20) {
			*DBGLOG(INIT, WARN, "[MJ]0x%08x(%08x) = 0x%08x\n",
			*(0x7C500000 + u4Offset), u4Val,
			*(uint32_t)(*(uint32_t *)
			*((uint8_t *)pucStartPtr + u4Offset)));
			*}
			*/
		}
#endif
#endif /* _HIF_PCIE */
#endif /* SOC3_0 */
	} else {
		return WLAN_STATUS_NOT_SUPPORTED;
	}

	return WLAN_STATUS_SUCCESS;
}
#endif
int wf_ioremap_read(size_t addr, unsigned int *val)
{
	void *vir_addr = NULL;

	vir_addr = ioremap_nocache(addr, 0x10);
	if (!vir_addr) {
		DBGLOG(INIT, ERROR, "%s: Cannot remap address.\n", __func__);
		return -1;
	}

	*val = readl(vir_addr);
	iounmap(vir_addr);
	DBGLOG(INIT, ERROR, "Read CONSYS 0x%08x=0x%08x.\n", addr, *val);

	return 0;
}
int wf_ioremap_write(phys_addr_t addr, unsigned int val)
{
	void *vir_addr = NULL;

	vir_addr = ioremap_nocache(addr, 0x10);
	if (!vir_addr) {
		DBGLOG(INIT, ERROR, "%s: Cannot remap address.\n", __func__);
		return -1;
	}

	writel(val, vir_addr);
	iounmap(vir_addr);
	DBGLOG(INIT, ERROR, "Write CONSYS 0x%08x=0x%08x.\n", addr, val);

	return 0;
}
int soc3_0_Trigger_fw_assert(void)
{
	int ret = 0;
	int value;

	wf_ioremap_read(WF_TRIGGER_AP2CONN_EINT, &value);
	value &= 0xFFFFFF7F;
	ret = wf_ioremap_write(WF_TRIGGER_AP2CONN_EINT, value);
	udelay(1000);
	wf_ioremap_read(WF_TRIGGER_AP2CONN_EINT, &value);
	value |= 0x80;
	ret = wf_ioremap_write(WF_TRIGGER_AP2CONN_EINT, value);
	return ret;
}
int wf_pwr_on_consys_mcu(void)
{
	int check;
	int value = 0;
	int ret = 0;
	int conninfra_hang_ret = 0;
	unsigned int polling_count;

	DBGLOG(INIT, INFO, "wmmcu power-on start.\n");
	/* Wakeup conn_infra off write 0x180601A4[0] = 1'b1 */
	wf_ioremap_read(CONN_INFRA_WAKEUP_WF_ADDR, &value);
	value |= 0x00000001;
	wf_ioremap_write(CONN_INFRA_WAKEUP_WF_ADDR, value);

	/* Check AP2CONN slpprot ready
	 * (polling "10 times" and each polling interval is "1ms")
	 * Address: 0x1806_0184[5]
	 * Data: 1'b0
	 * Action: polling
	 */
	wf_ioremap_read(CONN_INFRA_ON2OFF_SLP_PROT_ACK_ADDR, &value);
	check = 0;
	polling_count = 0;
	while ((value & 0x00000020) != 0) {
		if (polling_count > 10) {
			check = -1;
			ret = -1;
			break;
		}
		udelay(1000);
		wf_ioremap_read(CONN_INFRA_ON2OFF_SLP_PROT_ACK_ADDR, &value);
		polling_count++;
	}
	if (check != 0) {
		DBGLOG(INIT, ERROR, "Polling  AP2CONN slpprot ready fail.\n");
		return ret;
	}

	/* Check CONNSYS version ID
	 * (polling "10 times" and each polling interval is "1ms")
	 * Address: 0x1800_1000[31:0]
	 * Data: 0x2001_0000
	 * Action: polling
	 */
	wf_ioremap_read(CONN_HW_VER_ADDR, &value);
	check = 0;
	polling_count = 0;
	while (value != CONNSYS_VERSION_ID) {
		if (polling_count > 10) {
			check = -1;
			ret = -1;
			break;
		}
		udelay(1000);
		wf_ioremap_read(CONN_HW_VER_ADDR, &value);
		polling_count++;
	}
	if (check != 0) {
		DBGLOG(INIT, ERROR, "Polling CONNSYS version ID fail.\n");
		return ret;
	}

	/* Assert CONNSYS WM CPU SW reset write 0x18000010[0] = 1'b0*/
	wf_ioremap_read(WFSYS_CPU_SW_RST_B_ADDR, &value);
	value &= 0xFFFFFFFE;
	wf_ioremap_write(WFSYS_CPU_SW_RST_B_ADDR, value);
#if (CFG_SUPPORT_CONNINFRA == 1)
	/* bus clock ctrl */
	conninfra_bus_clock_ctrl(CONNDRV_TYPE_WIFI, CONNINFRA_BUS_CLOCK_ALL, 1);
#endif
	/* Turn on wfsys_top_on
	 * 0x18000000[31:16] = 0x57460000,
	 * 0x18000000[7] = 1'b1
	 */
	wf_ioremap_read(WFSYS_ON_TOP_PWR_CTL_ADDR, &value);
	value &= 0x0000FF7F;
	value |= 0x57460080;
	wf_ioremap_write(WFSYS_ON_TOP_PWR_CTL_ADDR, value);
	/* Polling wfsys_rgu_off_hreset_rst_b
	 * (polling "10 times" and each polling interval is "0.5ms")
	 * Address: 0x1806_02CC[2] (TOP_DBG_DUMMY_3_CONNSYS_PWR_STATUS[2])
	 * Data: 1'b1
	 * Action: polling
	 */

	wf_ioremap_read(TOP_DBG_DUMMY_3_CONNSYS_PWR_STATUS_ADDR, &value);
	check = 0;
	polling_count = 0;
	while ((value & 0x00000004) == 0) {
		if (polling_count > 10) {
			check = -1;
			ret = -1;
			break;
		}
		udelay(500);
		wf_ioremap_read(TOP_DBG_DUMMY_3_CONNSYS_PWR_STATUS_ADDR,
				&value);
		polling_count++;
	}
	if (check != 0) {
		DBGLOG(INIT, ERROR,
			"Polling wfsys rgu off fail. (0x%x)\n",
			value);
		return ret;
	}
#if (CFG_SUPPORT_CONNINFRA == 1)
	if (!conninfra_reg_readable()) {
		DBGLOG(HAL, ERROR,
			"conninfra_reg_readable fail\n");

		conninfra_hang_ret = conninfra_is_bus_hang();

		if (conninfra_hang_ret > 0) {

			DBGLOG(HAL, ERROR,
				"conninfra_is_bus_hang, Chip reset\n");
		}
		return -1;
	}
#endif
	/* Turn off "conn_infra to wfsys"/ wfsys to conn_infra" bus
	 * sleep protect 0x18001620[0] = 1'b0
	 */
	wf_ioremap_read(CONN_INFRA_WF_SLP_CTRL_R_ADDR, &value);
	value &= 0xFFFFFFFE;
	wf_ioremap_write(CONN_INFRA_WF_SLP_CTRL_R_ADDR, value);
	/* Polling WF_SLP_CTRL ready
	 * (polling "100 times" and each polling interval is "0.5ms")
	 * Address: 0x1800_1620[3] (CONN_INFRA_WF_SLP_CTRL_R_OFFSET[3])
	 * Data: 1'b0
	 * Action: polling
	 */
	wf_ioremap_read(CONN_INFRA_WF_SLP_CTRL_R_ADDR, &value);
	check = 0;
	polling_count = 0;
	while ((value & 0x00000008) != 0) {
		if (polling_count > 100) {
			check = -1;
			ret = -1;
			break;
		}
		udelay(500);
		wf_ioremap_read(CONN_INFRA_WF_SLP_CTRL_R_ADDR, &value);
		polling_count++;
		DBGLOG(INIT, ERROR, "WF_SLP_CTRL (0x%x) (%d)\n",
			value,
			polling_count);
	}
	if (check != 0) {
		DBGLOG(INIT, ERROR,
			"Polling WFSYS TO CONNINFRA SLEEP PROTECT fail. (0x%x)\n",
			value);
		return ret;
	}
	/* Turn off "wf_dma to conn_infra" bus sleep protect
	 * 0x18001624[0] = 1'b0
	 */
	wf_ioremap_read(CONN_INFRA_WFDMA_SLP_CTRL_R_ADDR, &value);
	value &= 0xFFFFFFFE;
	wf_ioremap_write(CONN_INFRA_WFDMA_SLP_CTRL_R_ADDR, value);
	/* Polling wfsys_rgu_off_hreset_rst_b
	 * (polling "100 times" and each polling interval is "0.5ms")
	 * Address: 0x1800_1624[3] (CONN_INFRA_WFDMA_SLP_CTRL_R_OFFSET[3])
	 * Data: 1'b0
	 * Action: polling
	 */
	wf_ioremap_read(CONN_INFRA_WFDMA_SLP_CTRL_R_ADDR, &value);
	check = 0;
	polling_count = 0;
	while ((value & 0x00000008) != 0) {
		if (polling_count > 100) {
			check = -1;
			ret = -1;
			break;
		}
		udelay(500);
		wf_ioremap_read(CONN_INFRA_WFDMA_SLP_CTRL_R_ADDR, &value);
		polling_count++;
		DBGLOG(INIT, ERROR, "WFDMA_SLP_CTRL (0x%x) (%d)\n",
			value,
			polling_count);
	}
	if (check != 0) {
		DBGLOG(INIT, ERROR,
			"Polling WFDMA TO CONNINFRA SLEEP PROTECT RDY fail. (0x%x)\n",
			value);
		return ret;
	}

	/*WF_VDNR_EN_ADDR 0x1800_E06C[0] =1'b1*/
	wf_ioremap_read(WF_VDNR_EN_ADDR, &value);
	value |= 0x00000001;
	wf_ioremap_write(WF_VDNR_EN_ADDR, value);

	/* Check WFSYS version ID (Polling) */
	wf_ioremap_read(WFSYS_VERSION_ID_ADDR, &value);
	check = 0;
	polling_count = 0;
	while (value != CONNSYS_VERSION_ID) {
		if (polling_count > 10) {
			check = -1;
			ret = -1;
			break;
		}
		udelay(500);
		wf_ioremap_read(WFSYS_VERSION_ID_ADDR, &value);
		polling_count++;
	}
	if (check != 0) {
		DBGLOG(INIT, ERROR, "Polling CONNSYS version ID fail.\n");
		return ret;
	}
	/* Setup CONNSYS firmware in EMI */
#if 0/*(CFG_POWER_ON_DOWNLOAD_EMI_ROM_PATCH == 1)*/
	soc3_0_wlanPowerOnInit(ENUM_WLAN_POWER_ON_DOWNLOAD_EMI);
#endif

	/* Default value update 2: EMI entry address */
	wf_ioremap_write(CONN_CFG_AP2WF_REMAP_1_ADDR, CONN_MCU_CONFG_HS_BASE);
	wf_ioremap_write(WF_DYNAMIC_BASE+MCU_EMI_ENTRY_OFFSET, 0x00000000);
	wf_ioremap_write(WF_DYNAMIC_BASE+WF_EMI_ENTRY_OFFSET, 0x00000000);

	/* Reset WFSYS semaphore 0x18000018[0] = 1'b0 */
	wf_ioremap_read(WFSYS_SW_RST_B_ADDR, &value);
	value &= 0xFFFFFFFE;
	wf_ioremap_write(WFSYS_SW_RST_B_ADDR, value);

	/* De-assert WFSYS CPU SW reset 0x18000010[0] = 1'b1 */
	wf_ioremap_read(WFSYS_CPU_SW_RST_B_ADDR, &value);
	value |= 0x00000001;
	wf_ioremap_write(WFSYS_CPU_SW_RST_B_ADDR, value);

	/* Check CONNSYS power-on completion
	 * Polling "100 times" and each polling interval is "0.5ms"
	 * Polling 0x81021604[31:0] = 0x00001D1E
	 */
	wf_ioremap_read(WF_ROM_CODE_INDEX_ADDR, &value);
	check = 0;
	polling_count = 0;
	while (value != CONNSYS_ROM_DONE_CHECK) {
		if (polling_count > 100) {
			check = -1;
			ret = -1;
			break;
		}
		udelay(1000);
		wf_ioremap_read(WF_ROM_CODE_INDEX_ADDR, &value);
		polling_count++;
	}
	if (check != 0) {
		DBGLOG(INIT, ERROR,
			"Check CONNSYS power-on completion fail.\n");
		return ret;
	}
#if (CFG_SUPPORT_CONNINFRA == 1)
	conninfra_config_setup();

	/* bus clock ctrl */
	conninfra_bus_clock_ctrl(CONNDRV_TYPE_WIFI, CONNINFRA_BUS_CLOCK_ALL, 0);
#endif
	/* Disable conn_infra off domain force on 0x180601A4[0] = 1'b0 */
	wf_ioremap_read(CONN_INFRA_WAKEUP_WF_ADDR, &value);
	value &= 0xFFFFFFFE;
	wf_ioremap_write(CONN_INFRA_WAKEUP_WF_ADDR, value);
	DBGLOG(INIT, INFO, "wmmcu power-on done.\n");
	return ret;
}

int wf_pwr_off_consys_mcu(void)
{
	int check;
	int value;
	int ret = 0;
	int conninfra_hang_ret = 0;
	int polling_count;

	DBGLOG(INIT, INFO, "wmmcu power-off start.\n");
	/* Wakeup conn_infra off write 0x180601A4[0] = 1'b1 */
	wf_ioremap_read(CONN_INFRA_WAKEUP_WF_ADDR, &value);
	value |= 0x00000001;
	wf_ioremap_write(CONN_INFRA_WAKEUP_WF_ADDR, value);

	/* Check AP2CONN slpprot ready
	 * (polling "10 times" and each polling interval is "1ms")
	 * Address: 0x1806_0184[5]
	 * Data: 1'b0
	 * Action: polling
	 */
	wf_ioremap_read(CONN_INFRA_ON2OFF_SLP_PROT_ACK_ADDR, &value);
	check = 0;
	polling_count = 0;
	while ((value & 0x00000020) != 0) {
		if (polling_count > 10) {
			check = -1;
			ret = -1;
			break;
		}
		udelay(1000);
		wf_ioremap_read(CONN_INFRA_ON2OFF_SLP_PROT_ACK_ADDR, &value);
		polling_count++;
	}
	if (check != 0) {
		DBGLOG(INIT, ERROR, "Polling  AP2CONN slpprot ready fail.\n");
		return ret;
	}
#if (CFG_SUPPORT_CONNINFRA == 1)
	if (!conninfra_reg_readable()) {
		DBGLOG(HAL, ERROR,
			"conninfra_reg_readable fail\n");

		conninfra_hang_ret = conninfra_is_bus_hang();

		if (conninfra_hang_ret > 0) {

			DBGLOG(HAL, ERROR,
				"conninfra_is_bus_hang, Chip reset\n");
		}
		return -1;
	}
#endif
	/* Check CONNSYS version ID
	 * (polling "10 times" and each polling interval is "1ms")
	 * Address: 0x1800_1000[31:0]
	 * Data: 0x2001_0000
	 * Action: polling
	 */
	wf_ioremap_read(CONN_HW_VER_ADDR, &value);
	check = 0;
	polling_count = 0;
	while (value != CONNSYS_VERSION_ID) {
		if (polling_count > 10) {
			check = -1;
			ret = -1;
			break;
		}
		udelay(1000);
		wf_ioremap_read(CONN_HW_VER_ADDR, &value);
		polling_count++;
	}
	if (check != 0) {
		DBGLOG(INIT, ERROR, "Polling CONNSYS version ID fail.\n");
		return ret;
	}
	/* Turn on "conn_infra to wfsys"/ wfsys to conn_infra" bus sleep protect
	 * 0x18001620[0] = 1'b1
	 */
	wf_ioremap_read(CONN_INFRA_WF_SLP_CTRL_R_ADDR, &value);
	value |= 0x00000001;
	wf_ioremap_write(CONN_INFRA_WF_SLP_CTRL_R_ADDR, value);

	/* Polling WF_SLP_CTRL ready
	 * (polling "100 times" and each polling interval is "0.5ms")
	 * Address: 0x1800_1620[3] (CONN_INFRA_WF_SLP_CTRL_R_OFFSET[3])
	 * Data: 1'b1
	 * Action: polling
	 */
	wf_ioremap_read(CONN_INFRA_WF_SLP_CTRL_R_ADDR, &value);
	check = 0;
	polling_count = 0;
	while ((value & 0x00000008) == 0) {
		if (polling_count > 100) {
			check = -1;
			ret = -1;
			break;
		}
		udelay(500);
		wf_ioremap_read(CONN_INFRA_WF_SLP_CTRL_R_ADDR, &value);
		polling_count++;
		DBGLOG(INIT, ERROR, "WF_SLP_CTRL (0x%x) (%d)\n",
			value,
			polling_count);
	}
	if (check != 0) {
		DBGLOG(INIT, ERROR,
			"Polling WFSYS TO CONNINFRA SLEEP PROTECT fail. (0x%x)\n",
			value);
		return ret;
	}
	/* Turn off "wf_dma to conn_infra" bus sleep protect
	 * 0x18001624[0] = 1'b1
	 */
	wf_ioremap_read(CONN_INFRA_WFDMA_SLP_CTRL_R_ADDR, &value);
	value |= 0x00000001;
	wf_ioremap_write(CONN_INFRA_WFDMA_SLP_CTRL_R_ADDR, value);

	/* Polling wfsys_rgu_off_hreset_rst_b
	 * (polling "100 times" and each polling interval is "0.5ms")
	 * Address: 0x1800_1624[3] (CONN_INFRA_WFDMA_SLP_CTRL_R_OFFSET[3])
	 * Data: 1'b1
	 * Action: polling
	 */
	wf_ioremap_read(CONN_INFRA_WFDMA_SLP_CTRL_R_ADDR, &value);
	check = 0;
	polling_count = 0;
	while ((value & 0x00000008) == 0) {
		if (polling_count > 100) {
			check = -1;
			ret = -1;
			break;
		}
		udelay(500);
		wf_ioremap_read(CONN_INFRA_WFDMA_SLP_CTRL_R_ADDR, &value);
		polling_count++;
		DBGLOG(INIT, ERROR, "WFDMA_SLP_CTRL (0x%x) (%d)\n",
			value,
			polling_count);
	}
	if (check != 0) {
		DBGLOG(INIT, ERROR,
			"Polling WFDMA TO CONNINFRA SLEEP PROTECT RDY fail. (0x%x)\n",
			value);
	}
#if (CFG_SUPPORT_CONNINFRA == 1)
	/* bus clock ctrl */
	conninfra_bus_clock_ctrl(CONNDRV_TYPE_WIFI, CONNINFRA_BUS_CLOCK_ALL, 0);
#endif
	/* Turn off wfsys_top_on
	 * 0x18000000[31:16] = 0x57460000,
	 * 0x18000000[7] = 1'b0
	 */
	wf_ioremap_read(WFSYS_ON_TOP_PWR_CTL_ADDR, &value);
	value &= 0x0000FF7F;
	value |= 0x57460000;
	wf_ioremap_write(WFSYS_ON_TOP_PWR_CTL_ADDR, value);
	/* Polling wfsys_rgu_off_hreset_rst_b
	 * (polling "10 times" and each polling interval is "0.5ms")
	 * Address: 0x1806_02CC[2] (TOP_DBG_DUMMY_3_CONNSYS_PWR_STATUS[2])
	 * Data: 1'b0
	 * Action: polling
	 */

	wf_ioremap_read(TOP_DBG_DUMMY_3_CONNSYS_PWR_STATUS_ADDR, &value);
	check = 0;
	polling_count = 0;
	while ((value & 0x00000004) != 0) {
		if (polling_count > 10) {
			check = -1;
			ret = -1;
			break;
		}
		udelay(500);
		wf_ioremap_read(TOP_DBG_DUMMY_3_CONNSYS_PWR_STATUS_ADDR,
				&value);
		polling_count++;
	}
	if (check != 0) {
		DBGLOG(INIT, ERROR,
			"Polling wfsys rgu off fail. (0x%x)\n",
			value);
		return ret;
	}
	/* Toggle WFSYS EMI request 0x18001c14[0] = 1'b1 -> 1'b0 */
	wf_ioremap_read(CONN_INFRA_WFSYS_EMI_REQ_ADDR, &value);
	value |= 0x00000001;
	wf_ioremap_write(CONN_INFRA_WFSYS_EMI_REQ_ADDR, value);
	value &= 0xFFFFFFFE;
	wf_ioremap_write(CONN_INFRA_WFSYS_EMI_REQ_ADDR, value);

	/* Reset WFSYS semaphore 0x18000018[0] = 1'b0 */
	wf_ioremap_read(WFSYS_SW_RST_B_ADDR, &value);
	value &= 0xFFFFFFFE;
	wf_ioremap_write(WFSYS_SW_RST_B_ADDR, value);

	/*Disable A-die top_ck_en (use common API)(clear driver & FW resource)*/
	conninfra_adie_top_ck_en_off(CONNSYS_ADIE_CTL_FW_WIFI);

	soc3_0_DumpBusHangdebuglog();
	/* Disable conn_infra off domain force on 0x180601A4[0] = 1'b0 */
	wf_ioremap_read(CONN_INFRA_WAKEUP_WF_ADDR, &value);
	value &= 0xFFFFFFFE;
	wf_ioremap_write(CONN_INFRA_WAKEUP_WF_ADDR, value);
	return ret;
}

int hifWmmcuPwrOn(void)
{
	int ret = 0;
	uint32_t u4Value = 0;

#if (CFG_SUPPORT_CONNINFRA == 1)
	/* conninfra power on */
	if (!kalIsWholeChipResetting()) {
	ret = conninfra_pwr_on(CONNDRV_TYPE_WIFI);
		if (ret == CONNINFRA_ERR_RST_ONGOING) {
			DBGLOG(INIT, ERROR,
				"Conninfra is doing whole chip reset.\n");
			return ret;
		}
		if (ret != 0) {
			DBGLOG(INIT, ERROR,
				"Conninfra pwr on fail.\n");
		return ret;
		}
	}
#endif
	/* wf driver power on */
	ret = wf_pwr_on_consys_mcu();
	if (ret != 0)
		return ret;
	/* set FW own after power on consys mcu to
	 * keep Driver/FW/HW state sync
	 */
	wf_ioremap_read(CONN_HOST_CSR_TOP_BASE_ADDR + 0x0010,
		&u4Value);

	if ((u4Value & BIT(2)) != BIT(2)) {
		DBGLOG(INIT, INFO, "0x%08x = 0x%08x, Set FW Own\n",
			CONN_HOST_CSR_TOP_BASE_ADDR + 0x0010,
			u4Value);

		wf_ioremap_write(CONN_HOST_CSR_TOP_BASE_ADDR + 0x0010,
			PCIE_LPCR_HOST_SET_OWN);
	}

	DBGLOG(INIT, INFO,
		"hifWmmcuPwrOn done\n");

	return ret;

	return ret;
}
int hifWmmcuPwrOff(void)
{
	int ret = 0;
	/* wf driver power off */
	ret = wf_pwr_off_consys_mcu();
	if (ret != 0)
		return ret;
#if (CFG_SUPPORT_CONNINFRA == 1)
	/*
	 * conninfra power off sequence
	 * conninfra will do conninfra power off self during whole chip reset.
	 */
	if (!kalIsWholeChipResetting()) {
	ret = conninfra_pwr_off(CONNDRV_TYPE_WIFI);
	if (ret != 0)
		return ret;
	}
#endif
	return ret;
}
#if (CFG_SUPPORT_CONNINFRA == 1)
int soc3_0_Trigger_whole_chip_rst(char *reason)
{
	return conninfra_trigger_whole_chip_rst(CONNDRV_TYPE_WIFI, reason);
}
void soc3_0_Sw_interrupt_handler(struct ADAPTER *prAdapter)
{
	int value;
	struct GL_HIF_INFO *prHifInfo = NULL;

	ASSERT(prAdapter);
	prHifInfo = &prAdapter->prGlueInfo->rHifInfo;

	if (!conninfra_reg_readable_no_lock()) {
		DBGLOG(HAL, ERROR,
			"conninfra_reg_readable fail\n");
		disable_irq_nosync(prHifInfo->u4IrqId_1);
#if (CFG_ANDORID_CONNINFRA_COREDUMP_SUPPORT == 1)
		g_eWfRstSource = WF_RST_SOURCE_FW;
#endif
		DBGLOG(HAL, ERROR,
			"FW trigger assert(0x%x).\n", value);
		fgIsResetting = TRUE;
		update_driver_reset_status(fgIsResetting);
		kalSetRstEvent();
		return;
	}
	HAL_MCR_WR(prAdapter,
		   CONN_INFRA_CFG_AP2WF_REMAP_1_ADDR,
		   CONN_MCU_CONFG_HS_BASE);
	HAL_MCR_RD(prAdapter,
		  (CONN_INFRA_CFG_AP2WF_BUS_ADDR + 0xc0),
		  &value);

	DBGLOG(HAL, TRACE, "SW INT happened!!!!!(0x%x)\n", value);
	HAL_MCR_WR(prAdapter,
		  (CONN_INFRA_CFG_AP2WF_BUS_ADDR + 0xc8),
		  value);

#ifdef CONFIG_MTK_CONNSYS_DEDICATED_LOG_PATH
	if (value & BIT(0))
		fw_log_wifi_irq_handler();
#endif

	if (value & BIT(1)) {
		if (kalIsResetting()) {
#if (CFG_ANDORID_CONNINFRA_COREDUMP_SUPPORT == 1)
			g_eWfRstSource = WF_RST_SOURCE_DRIVER;
#endif
			DBGLOG(HAL, ERROR,
				"Wi-Fi Driver trigger, need do complete(0x%x).\n",
				value);
			complete(&g_triggerComp);
		} else {
			if (get_wifi_process_status() == 1) {
				DBGLOG(HAL, ERROR,
					"Wi-Fi on/off process is ongoing, ignore interrupt(0x%x).\n",
					value);
				return;
}
#if (CFG_ANDORID_CONNINFRA_COREDUMP_SUPPORT == 1)
			g_eWfRstSource = WF_RST_SOURCE_FW;
#endif
			DBGLOG(HAL, ERROR,
				"FW trigger assert(0x%x).\n", value);
			fgIsResetting = TRUE;
			update_driver_reset_status(fgIsResetting);
			kalSetRstEvent();
		}
	}
	if (value & BIT(2)) {
		if (get_wifi_process_status() == 1) {
			DBGLOG(HAL, ERROR,
				"Wi-Fi on/off process is ongoing, ignore interrupt(0x%x).\n",
				value);
			return;
		}
#if (CFG_ANDORID_CONNINFRA_COREDUMP_SUPPORT == 1)
		g_eWfRstSource = WF_RST_SOURCE_FW;
#endif
		DBGLOG(HAL, ERROR,
			"FW trigger whole chip reset(0x%x).\n", value);
		fgIsResetting = TRUE;
		update_driver_reset_status(fgIsResetting);
		g_IsWfsysBusHang = TRUE;
		kalSetRstEvent();
	}
	if (value & BIT(3)) {
		if (get_wifi_process_status() == 1) {
			DBGLOG(HAL, ERROR,
				"Wi-Fi on/off process is ongoing, ignore interrupt(0x%x).\n",
				value);
			return;
		}
#if (CFG_ANDORID_CONNINFRA_COREDUMP_SUPPORT == 1)
		g_eWfRstSource = WF_RST_SOURCE_FW;
#endif
		g_fgRstRecover = TRUE;
		fgIsResetting = TRUE;
		update_driver_reset_status(fgIsResetting);
		kalSetRstEvent();
	}
}

#endif
void soc3_0_icapRiseVcoreClockRate(void)
{

#if (CFG_SUPPORT_VCODE_VDFS == 1)
	int value = 0;


	/*2 update Clork Rate*/
	/*0x1000123C[20]=1,218Mhz*/
	wf_ioremap_read(WF_CONN_INFA_BUS_CLOCK_RATE, &value);
	value |= 0x00010000;
	wf_ioremap_write(WF_CONN_INFA_BUS_CLOCK_RATE, value);


	/*Enable VCore to 0.725*/

	/*init*/
	if (!pm_qos_request_active(&wifi_req))
		pm_qos_add_request(&wifi_req, PM_QOS_VCORE_OPP,
						PM_QOS_VCORE_OPP_DEFAULT_VALUE);

	/*update Vcore*/
	pm_qos_update_request(&wifi_req, 0);

	DBGLOG(HAL, STATE, "icapRiseVcoreClockRate done\n");
#else
	DBGLOG(HAL, STATE, "icapRiseVcoreClockRate skip\n");
#endif  /*#ifndef CFG_BUILD_X86_PLATFORM*/

}
void soc3_0_icapDownVcoreClockRate(void)
{
#if (CFG_SUPPORT_VCODE_VDFS == 1)
	int value = 0;

	/*2 update Clork Rate*/
	/*0x1000123C[20]=0,156Mhz*/
	wf_ioremap_read(WF_CONN_INFA_BUS_CLOCK_RATE, &value);
	value &= ~(0x00010000);
	wf_ioremap_write(WF_CONN_INFA_BUS_CLOCK_RATE, value);

	/*restore to default value*/
	pm_qos_add_request(&wifi_req, PM_QOS_VCORE_OPP,
					PM_QOS_VCORE_OPP_DEFAULT_VALUE);

	/*disable VCore to normal setting*/
	DBGLOG(HAL, STATE, "icapDownVcoreClockRate done!\n");
#else
	DBGLOG(HAL, STATE, "icapDownVcoreClockRate skip\n");
#endif  /*#ifndef CFG_SUPPORT_VCODE_VDFS*/

}
#endif	/* soc3_0 */

