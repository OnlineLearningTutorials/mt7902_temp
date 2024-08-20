/* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause */
/*
 * Copyright (c) 2016 MediaTek Inc.
 */
/*! \file   mt7663.c
 *  \brief  Internal driver stack will export the required procedures here
 *          for GLUE Layer.
 *
 *    This file contains all routines which are exported from MediaTek 802.11
 *    Wireless LAN driver stack to GLUE Layer.
 */

#ifdef MT7663

/*******************************************************************************
 *                         C O M P I L E R   F L A G S
 *******************************************************************************
 */

/*******************************************************************************
 *                    E X T E R N A L   R E F E R E N C E S
 *******************************************************************************
 */
#include "precomp.h"

#include "mt7663.h"

/*******************************************************************************
 *                              C O N S T A N T S
 *******************************************************************************
 */

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

#if defined(_HIF_PCIE)
static void mt7663InitPcieInt(struct GLUE_INFO *prGlueInfo)
{
	HAL_MCR_WR(prGlueInfo->prAdapter, MT_PCIE_IRQ_ENABLE, 1);
}
#endif /* _HIF_PCIE */

/*******************************************************************************
 *                            P U B L I C   D A T A
 *******************************************************************************
 */
struct ECO_INFO mt7663_eco_table[] = {
	/* HW version,  ROM version,    Factory version */
	{0x00, 0x00, 0x0A, 0x01},	/* E1 */
	{0x10, 0x01, 0x0A, 0x02},	/* E2 */
	{0x00, 0x00, 0x00, 0x00}	/* End of table */
};

#if defined(_HIF_PCIE)
struct PCIE_CHIP_CR_MAPPING mt7663_bus2chip_cr_mapping[] = {
	/* chip addr, bus addr, range */
	{0x80000000, 0x00002000, 0x00001000}, /* MCU_CFG */

	{0x50000000, 0x00004000, 0x00004000}, /* CONN_HIF (PDMA) */
	{0x50002000, 0x00005000, 0x00001000}, /* CONN_HIF (Reserved) */
	{0x5000A000, 0x00006000, 0x00001000}, /* CONN_HIF (DMASHDL) */
	{0x000E0000, 0x00007000, 0x00001000}, /* CONN_HIF_ON (HOST_CSR) */

	{0x82060000, 0x00008000, 0x00004000}, /* WF_UMAC_TOP (PLE) */
	{0x82068000, 0x0000C000, 0x00002000}, /* WF_UMAC_TOP (PSE) */
	{0x8206C000, 0x0000E000, 0x00002000}, /* WF_UMAC_TOP (PP) */
	{0x82070000, 0x00010000, 0x00010000}, /* WF_PHY */
	{0x820F0000, 0x00020000, 0x00010000}, /* WF_LMAC_TOP */
	{0x820E0000, 0x00030000, 0x00010000}, /* WF_LMAC_TOP (WF_WTBL) */

	{0x81000000, 0x00050000, 0x00010000}, /* BTSYS_OFF */
	{0x80070000, 0x00060000, 0x00010000}, /* GPSSYS */
	{0x40000000, 0x00070000, 0x00010000}, /* WF_SYSRAM */
	{0x00300000, 0x00080000, 0x00010000}, /* MCU_SYSRAM */

	{0x80010000, 0x000A1000, 0x00001000}, /* CONN_MCU_DMA */
	{0x80030000, 0x000A2000, 0x00001000}, /* CONN_MCU_BTIF0 */
	{0x81030000, 0x000A3000, 0x00001000}, /* CONN_MCU_CFG_ON */
	{0x80050000, 0x000A4000, 0x00001000}, /* CONN_UART_PTA */
	{0x81040000, 0x000A5000, 0x00001000}, /* CONN_MCU_CIRQ */
	{0x81050000, 0x000A6000, 0x00001000}, /* CONN_MCU_GPT */
	{0x81060000, 0x000A7000, 0x00001000}, /* CONN_PTA */
	{0x81080000, 0x000A8000, 0x00001000}, /* CONN_MCU_WDT */
	{0x81090000, 0x000A9000, 0x00001000}, /* CONN_MCU_PDA */
	{0x810A0000, 0x000AA000, 0x00001000}, /* CONN_RDD_AHB_WRAP0 */
	{0x810B0000, 0x000AB000, 0x00001000}, /* BTSYS_ON */
	{0x810C0000, 0x000AC000, 0x00001000}, /* CONN_RBIST_TOP */
	{0x810D0000, 0x000AD000, 0x00001000}, /* CONN_RDD_AHB_WRAP0 */
	{0x820D0000, 0x000AE000, 0x00001000}, /* WFSYS_ON */
	{0x60000000, 0x000AF000, 0x00001000}, /* CONN_MCU_PDA */

	{0x80020000, 0x000B0000, 0x00010000}, /* CONN_TOP_MISC_OFF */
	{0x81020000, 0x000C0000, 0x00010000}, /* CONN_TOP_MISC_ON */
	{0x7c030000, 0x000F0000, 0x00010000}, /* CONN_TOP_MISC_ON */

	{0x0, 0x0, 0x0}
};
#endif /* _HIF_PCIE */

struct BUS_INFO mt7663_bus_info = {
#if defined(_HIF_PCIE)
	.top_cfg_base = MT7663_TOP_CFG_BASE,
	.host_tx_ring_base = MT_TX_RING_BASE,
	.host_tx_ring_ext_ctrl_base = MT_TX_RING_BASE_EXT,
	.host_tx_ring_cidx_addr = MT_TX_RING_CIDX,
	.host_tx_ring_didx_addr = MT_TX_RING_DIDX,
	.host_tx_ring_cnt_addr = MT_TX_RING_CNT,

	.host_rx_ring_base = MT_RX_RING_BASE,
	.host_rx_ring_ext_ctrl_base = MT_RX_RING_BASE_EXT,
	.host_rx_ring_cidx_addr = MT_RX_RING_CIDX,
	.host_rx_ring_didx_addr = MT_RX_RING_DIDX,
	.host_rx_ring_cnt_addr = MT_RX_RING_CNT,
	.bus2chip = mt7663_bus2chip_cr_mapping,
	.tx_ring_fwdl_idx = 3,
	.tx_ring_cmd_idx = 15,
	.tx_ring0_data_idx = 0,
	.tx_ring1_data_idx = 0,
	.fw_own_clear_addr = WPDMA_INT_STA,
	.fw_own_clear_bit = WPDMA_FW_CLR_OWN_INT,
	.max_static_map_addr = 0x00040000,
	.fgCheckDriverOwnInt = FALSE,
	.u4DmaMask = 36,

	.pdmaSetup = asicPdmaConfig,
	.pdmaStop = NULL,
	.pdmaPollingIdle = NULL,
	.updateTxRingMaxQuota = NULL,
	.enableInterrupt = asicEnableInterrupt,
	.disableInterrupt = asicDisableInterrupt,
	.lowPowerOwnRead = asicLowPowerOwnRead,
	.lowPowerOwnSet = asicLowPowerOwnSet,
	.lowPowerOwnClear = asicLowPowerOwnClearPCIe,
	.wakeUpWiFi = asicWakeUpWiFi,
	.isValidRegAccess = NULL,
	.getMailboxStatus = asicGetMailboxStatus,
	.setDummyReg = asicSetDummyReg,
	.checkDummyReg = asicCheckDummyReg,
	.tx_ring_ext_ctrl = asicPdmaTxRingExtCtrl,
	.rx_ring_ext_ctrl = asicPdmaRxRingExtCtrl,
	.hifRst = NULL,
	.initPcieInt = mt7663InitPcieInt,
	.DmaShdlInit = asicPcieDmaShdlInit,
	.DmaShdlReInit = NULL,
#endif /* _HIF_PCIE */
#if defined(_HIF_USB)
	.u4UdmaWlCfg_0_Addr = CONNAC_UDMA_WLCFG_0,
	.u4UdmaWlCfg_1_Addr = CONNAC_UDMA_WLCFG_1,
	.u4UdmaWlCfg_0 =
		(UDMA_WLCFG_0_TX_EN(1) |
		UDMA_WLCFG_0_RX_EN(1) |
		UDMA_WLCFG_0_RX_MPSZ_PAD0(1) |
		UDMA_WLCFG_0_1US_TIMER_EN(1)),
	.u4UdmaTxTimeout = UDMA_TX_TIMEOUT_LIMIT,
	.u4device_vender_request_in = DEVICE_VENDOR_REQUEST_IN,
	.u4device_vender_request_out = DEVICE_VENDOR_REQUEST_OUT,
	.fgIsSupportWdtEp = FALSE,
	.asicUsbSuspend = asicUsbSuspend,
	.asicUsbResume = NULL,
	.asicUsbEventEpDetected = asicUsbEventEpDetected,
	.asicUsbRxByteCount = NULL,
	.DmaShdlInit = asicUsbDmaShdlInit,
	.DmaShdlReInit = NULL,
	.asicUdmaRxFlush = NULL,
#if CFG_CHIP_RESET_SUPPORT
	.asicUsbEpctlRstOpt = NULL,
#endif
#endif /* _HIF_USB */
#if defined(_HIF_SDIO)
	.halTxGetFreeResource = halTxGetFreeResource_v1,
	.halTxReturnFreeResource = halTxReturnFreeResource_v1,
	.halRestoreTxResource = halRestoreTxResource_v1,
	.halUpdateTxDonePendingCount = halUpdateTxDonePendingCount_v1,
#endif /* _HIF_SDIO */
};

struct FWDL_OPS_T mt7663_fw_dl_ops = {
	.constructFirmwarePrio = NULL,
	.downloadPatch = wlanDownloadPatch,
	.downloadFirmware = wlanConnacFormatDownload,
	.downloadByDynMemMap = NULL,
	.getFwInfo = wlanGetConnacFwInfo,
	.getFwDlInfo = asicGetFwDlInfo,
};

struct TX_DESC_OPS_T mt7663TxDescOps = {
	.fillNicAppend = fillNicTxDescAppend,
	.fillHifAppend = fillTxDescAppendByHostV2,
	.fillTxByteCount = fillTxDescTxByteCount,
};

struct RX_DESC_OPS_T mt7663RxDescOps = {
};

#if CFG_SUPPORT_QA_TOOL
struct ATE_OPS_T mt7663AteOps = {
	.setICapStart = connacSetICapStart,
	.getICapStatus = connacGetICapStatus,
	.getICapIQData = connacGetICapIQData,
	.getRbistDataDumpEvent = nicExtEventICapIQData,
	.u4EnBitWidth = 1, /*96 bit*/
	.u4Architech = 0,  /*0:on-chip*/
	.u4PhyIdx = 0,
	.u4EmiStartAddress = 0,
	.u4EmiEndAddress = 0,
	.u4EmiMsbAddress = 0,
};
#endif

struct CHIP_DBG_OPS mt7663_debug_ops = {
#if defined(_HIF_PCIE) || defined(_HIF_AXI)
	.showPdmaInfo = halShowPdmaInfo,
	.showCsrInfo = halShowHostCsrInfo,
#else
	.showPdmaInfo = NULL,
	.showCsrInfo = NULL,
#endif
	.showPseInfo = halShowPseInfo,
	.showPleInfo = halShowPleInfo,
	.showTxdInfo = halShowTxdInfo,
	.showDmaschInfo = halShowDmaschInfo,
	.showWtblInfo = NULL,
	.showHifInfo = NULL,
	.printHifDbgInfo = halPrintHifDbgInfo,
#if (CFG_SUPPORT_RA_GEN == 1)
	.show_stat_info = mt7663_show_stat_info,
#else
	.show_stat_info = NULL,
#endif
	.show_mcu_debug_info = NULL,
	.show_conninfra_debug_info = NULL,
};

/* Litien code refine to support multi chip */
struct mt66xx_chip_info mt66xx_chip_info_mt7663 = {
	.bus_info = &mt7663_bus_info,
	.fw_dl_ops = &mt7663_fw_dl_ops,
	.prTxDescOps = &mt7663TxDescOps,
	.prRxDescOps = &mt7663RxDescOps,
#if CFG_SUPPORT_QA_TOOL
	.prAteOps = &mt7663AteOps,
#endif
	.prDebugOps = &mt7663_debug_ops,

	.chip_id = MT7663_CHIP_ID,
	.should_verify_chip_id = FALSE,
	.sw_sync0 = MT7663_SW_SYNC0,
	.sw_ready_bits = WIFI_FUNC_NO_CR4_READY_BITS,
	.sw_ready_bit_offset = MT7663_SW_SYNC0_RDY_OFFSET,
	.patch_addr = MT7663_PATCH_START_ADDR,
	.is_support_cr4 = FALSE,
	.txd_append_size = MT7663_TX_DESC_APPEND_LENGTH,
	.rxd_size = MT7663_RX_DESC_LENGTH,
	.pse_header_length = NIC_TX_PSE_HEADER_LENGTH,
	.init_event_size = MT7663_RX_INIT_EVENT_LENGTH,
	.event_hdr_size = MT7663_RX_EVENT_HDR_LENGTH,
	.eco_info = mt7663_eco_table,
	.isNicCapV1 = FALSE,
	.is_support_efuse = TRUE,

	.asicCapInit = asicCapInit,
	.asicEnableFWDownload = asicEnableFWDownload,
	.asicGetChipID = NULL,
	.downloadBufferBin = wlanConnacDownloadBufferBin,
	.showTaskStack = NULL,
	.is_support_hw_amsdu = TRUE,
	.ucMaxSwAmsduNum = 0,
	.workAround = 0,
	.prTxPwrLimitFile = "TxPwrLimit_MT76x3.dat",
	.ucTxPwrLimitBatchSize = 16,

	.top_hcr = TOP_HCR,
	.top_hvr = TOP_HVR,
	.top_fvr = TOP_FVR,
	.ucMaxSwapAntenna = 0,

#if CFG_CHIP_RESET_SUPPORT
	.asicWfsysRst = NULL,
	.asicPollWfsysSwInitDone = NULL,
#endif
	.loadCfgSetting = NULL,
};

struct mt66xx_hif_driver_data mt66xx_driver_data_mt7663 = {
	.chip_info = &mt66xx_chip_info_mt7663,
};

#endif /* MT7663 */
