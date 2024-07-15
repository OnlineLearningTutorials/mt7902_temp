/* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause */
/*
 * Copyright (c) 2016 MediaTek Inc.
 */
/*! \file   mt7922.c
*    \brief  Internal driver stack will export
*    the required procedures here for GLUE Layer.
*
*    This file contains all routines which are exported
     from MediaTek 802.11 Wireless LAN driver stack to GLUE Layer.
*/

#ifdef MT7922

/*******************************************************************************
*                         C O M P I L E R   F L A G S
********************************************************************************
*/

/*******************************************************************************
*                    E X T E R N A L   R E F E R E N C E S
********************************************************************************
*/

#include "coda/mt7961/wf_wfdma_host_dma0.h"
#include "coda/mt7961/wf_cr_sw_def.h"
#include "precomp.h"
#include "mt7961.h"
#include "mt7922.h"
#include "hal_dmashdl_mt7961.h"
#include "hal_wfsys_reset_mt7961.h"

/*******************************************************************************
*                              C O N S T A N T S
********************************************************************************
*/

#define CONN_MCU_CONFG_BASE 0x88000000
#define CONN_MCU_CONFG_COM_REG0_ADDR (CONN_MCU_CONFG_BASE + 0x200)

#define PATCH_SEMAPHORE_COMM_REG 0
#define PATCH_SEMAPHORE_COMM_REG_PATCH_DONE 1 /* bit0 is for patch. */

#define SW_WORKAROUND_FOR_WFDMA_ISSUE_HWITS00009838 1
#define SW_WORKAROUND_FOR_WFDMA_ISSUE_HWITS00015048 1

#define RX_DATA_RING_BASE_IDX 2

#define FLAVOR_VERSION_DEFAULT (1)

/*******************************************************************************
*                                 M A C R O S
********************************************************************************
*/
#define MT7922_BUS2CHIP_MEM_MAP_TBLE_SIZE    \
	(sizeof(mt7922_bus2chip_cr_mapping) /	\
	 sizeof(mt7922_bus2chip_cr_mapping[0]))

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

struct ECO_INFO mt7922_eco_table[] = {
	/* HW version,  ROM version, Factory version, EcoVer */
	{0x00, 0x00, 0xA, 0x1},	/* E1 */
	{0x01, 0x01, 0xA, 0x2},	/* E2 */
	{0x00, 0x00, 0x0, 0x0}	/* End of table */
};

#if defined(_HIF_PCIE) || defined(_HIF_AXI)
struct PCIE_CHIP_CR_MAPPING mt7922_bus2chip_cr_mapping[] = {
	/* chip addr, bus addr, range */
	{0x54000000, 0x02000, 0x1000},  /* WFDMA PCIE0 MCU DMA0 */
	{0x55000000, 0x03000, 0x1000},  /* WFDMA PCIE0 MCU DMA1 */
	{0x56000000, 0x04000, 0x1000},  /* WFDMA reserved */
	{0x57000000, 0x05000, 0x1000},  /* WFDMA MCU wrap CR */
	{0x58000000, 0x06000, 0x1000},  /* WFDMA PCIE1 MCU DMA0 (MEM_DMA) */
	{0x59000000, 0x07000, 0x1000},  /* WFDMA PCIE1 MCU DMA1 */
	{0x820c0000, 0x08000, 0x4000},  /* WF_UMAC_TOP (PLE) */
	{0x820c8000, 0x0c000, 0x2000},  /* WF_UMAC_TOP (PSE) */
	{0x820cc000, 0x0e000, 0x2000},  /* WF_UMAC_TOP (PP) */
	{0x74030000, 0x10000, 0x10000}, /* PCIE_MAC_IREG */
	{0x820e0000, 0x20000, 0x0400},  /* WF_LMAC_TOP BN0 (WF_CFG) */
	{0x820e1000, 0x20400, 0x0200},  /* WF_LMAC_TOP BN0 (WF_TRB) */
	{0x820e2000, 0x20800, 0x0400},  /* WF_LMAC_TOP BN0 (WF_AGG) */
	{0x820e3000, 0x20c00, 0x0400},  /* WF_LMAC_TOP BN0 (WF_ARB) */
	{0x820e4000, 0x21000, 0x0400},  /* WF_LMAC_TOP BN0 (WF_TMAC) */
	{0x820e5000, 0x21400, 0x0800},  /* WF_LMAC_TOP BN0 (WF_RMAC) */
	{0x820ce000, 0x21c00, 0x0200},  /* WF_LMAC_TOP (WF_SEC) */
	{0x820e7000, 0x21e00, 0x0200},  /* WF_LMAC_TOP BN0 (WF_DMA) */
	{0x820cf000, 0x22000, 0x1000},  /* WF_LMAC_TOP (WF_PF) */
	{0x820e9000, 0x23400, 0x0200},  /* WF_LMAC_TOP BN0 (WF_WTBLOFF) */
	{0x820ea000, 0x24000, 0x0200},  /* WF_LMAC_TOP BN0 (WF_ETBF) */
	{0x820eb000, 0x24200, 0x0400},  /* WF_LMAC_TOP BN0 (WF_LPON) */
	{0x820ec000, 0x24600, 0x0200},  /* WF_LMAC_TOP BN0 (WF_INT) */
	{0x820ed000, 0x24800, 0x0800},  /* WF_LMAC_TOP BN0 (WF_MIB) */
	{0x820ca000, 0x26000, 0x2000},  /* WF_LMAC_TOP BN0 (WF_MUCOP) */
	{0x820d0000, 0x30000, 0x10000}, /* WF_LMAC_TOP (WF_WTBLON) */
	{0x88000000, 0x40000, 0x10000}, /* CONN_MCU_CONFG_LS */
	{0x40000000, 0x70000, 0x10000}, /* WF_UMAC_SYSRAM */
	{0x00400000, 0x80000, 0x10000}, /* WF_MCU_SYSRAM */
	{0x00410000, 0x90000, 0x10000}, /* WF_MCU_SYSRAM (configure register) */
	{0x820f0000, 0xa0000, 0x0400},  /* WF_LMAC_TOP BN1 (WF_CFG) */
	{0x820f1000, 0xa0600, 0x0200},  /* WF_LMAC_TOP BN1 (WF_TRB) */
	{0x820f2000, 0xa0800, 0x0400},  /* WF_LMAC_TOP BN1 (WF_AGG) */
	{0x820f3000, 0xa0c00, 0x0400},  /* WF_LMAC_TOP BN1 (WF_ARB) */
	{0x820f4000, 0xa1000, 0x0400},  /* WF_LMAC_TOP BN1 (WF_TMAC) */
	{0x820f5000, 0xa1400, 0x0800},  /* WF_LMAC_TOP BN1 (WF_RMAC) */
	{0x820f7000, 0xa1e00, 0x0200},  /* WF_LMAC_TOP BN1 (WF_DMA) */
	{0x820f9000, 0xa3400, 0x0200},  /* WF_LMAC_TOP BN1 (WF_WTBLOFF) */
	{0x820fa000, 0xa4000, 0x0200},  /* WF_LMAC_TOP BN1 (WF_ETBF) */
	{0x820fb000, 0xa4200, 0x0400},  /* WF_LMAC_TOP BN1 (WF_LPON) */
	{0x820fc000, 0xa4600, 0x0200},  /* WF_LMAC_TOP BN1 (WF_INT) */
	{0x820fd000, 0xa4800, 0x0800},  /* WF_LMAC_TOP BN1 (WF_MIB) */
	{0x820cc000, 0xa5000, 0x2000},  /* WF_LMAC_TOP BN1 (WF_MUCOP) */
	{0x820c4000, 0xa8000, 0x4000},  /* WF_LMAC_TOP BN1 (WF_MUCOP) */
	{0x820b0000, 0xae000, 0x1000},  /* [APB2] WFSYS_ON */
	{0x80020000, 0xb0000, 0x10000}, /* WF_TOP_MISC_OFF */
	{0x81020000, 0xc0000, 0x10000}, /* WF_TOP_MISC_ON */
	{0x7c020000, 0xd0000, 0x10000}, /* CONN_INFRA, wfdma */
	{0x7c060000, 0xe0000, 0x10000}, /* CONN_INFRA, conn_host_csr_top */
	{0x7c000000, 0xf0000, 0x10000}, /* CONN_INFRA */
};

#endif /*_HIF_PCIE || _HIF_AXI */

void mt7922_icapRiseVcoreClockRate(void)
{
	DBGLOG(HAL, INFO, "%s need to implement!\n", __func__);
}

void mt7922_icapDownVcoreClockRate(void)
{
	DBGLOG(HAL, INFO, "%s need to implement!\n", __func__);
}

void mt7922ConstructFirmwarePrio(struct GLUE_INFO *prGlueInfo,
	uint8_t **apucNameTable, uint8_t **apucName,
	uint8_t *pucNameIdx, uint8_t ucMaxNameIdx)
{
	uint8_t sub_idx = 0;
	uint32_t chip_id = 0;
	struct ADAPTER *prAdapter = NULL;
	struct mt66xx_chip_info *prChipInfo = NULL;
	uint32_t u4FlavorVer = FLAVOR_VERSION_DEFAULT;

	if (prGlueInfo == NULL) {
		DBGLOG(INIT, ERROR, "prGlueInfo is NULL.\n");
		return;
	}

	prAdapter = prGlueInfo->prAdapter;
	if (prAdapter == NULL) {
		DBGLOG(INIT, ERROR, "prAdapter is NULL.\n");
		return;
	}

	prChipInfo = prAdapter->chip_info;
	if (prChipInfo == NULL) {
		DBGLOG(INIT, ERROR, "prChipInfo is NULL.\n");
		return;
	}

	chip_id = prChipInfo->chip_id;

	for (sub_idx = 0; apucNameTable[sub_idx]; sub_idx++) {
		if (((*pucNameIdx) + 3) < ucMaxNameIdx) {
			/* Type 1. WIFI_RAM_CODE_MTxxxx_x.bin */
			snprintf(*(apucName + (*pucNameIdx)),
				CFG_FW_NAME_MAX_LEN, "%s%x_%x.bin",
				apucNameTable[sub_idx], chip_id, u4FlavorVer);
			(*pucNameIdx) += 1;

			/* Type 2. WIFI_RAM_CODE_MTxxxx_x */
			snprintf(*(apucName + (*pucNameIdx)),
				CFG_FW_NAME_MAX_LEN, "%s%x_%x",
				apucNameTable[sub_idx], chip_id, u4FlavorVer);
			(*pucNameIdx) += 1;

			/* Type 3. WIFI_RAM_CODE_MTxxxx.bin */
			snprintf(*(apucName + (*pucNameIdx)),
					CFG_FW_NAME_MAX_LEN, "%s%x.bin",
					apucNameTable[sub_idx], chip_id);
			(*pucNameIdx) += 1;

			/* Type 4. WIFI_RAM_CODE_MTxxxx */
			snprintf(*(apucName + (*pucNameIdx)),
				CFG_FW_NAME_MAX_LEN, "%s%x",
					apucNameTable[sub_idx], chip_id);
			(*pucNameIdx) += 1;
		} else {
			/* the table is not large enough */
			DBGLOG(INIT, ERROR,
				"kalFirmwareImageMapping >> file name array is not enough.\n");
			ASSERT(0);
		}
	}
}

void mt7922ConstructPatchName(struct GLUE_INFO *prGlueInfo,
	uint8_t **apucName, uint8_t *pucNameIdx)
{
	struct ADAPTER *prAdapter = NULL;
	struct mt66xx_chip_info *prChipInfo = NULL;
	uint32_t u4FlavorVer = FLAVOR_VERSION_DEFAULT;

	if (prGlueInfo == NULL) {
		DBGLOG(INIT, ERROR, "prGlueInfo is NULL.\n");
		return;
	}

	prAdapter = prGlueInfo->prAdapter;
	if (prAdapter == NULL) {
		DBGLOG(INIT, ERROR, "prAdapter is NULL.\n");
		return;
	}

	prChipInfo = prAdapter->chip_info;
	if (prChipInfo == NULL) {
		DBGLOG(INIT, ERROR, "prChipInfo is NULL.\n");
		return;
	}

	snprintf(apucName[(*pucNameIdx)],
		CFG_FW_NAME_MAX_LEN, "WIFI_MT%x_patch_mcu_%x_%x_hdr.bin",
		prChipInfo->chip_id, u4FlavorVer,
		mt7961GetFwVer(prAdapter));
}

#if CFG_SUPPORT_WIFI_DL_BT_PATCH
void mt7922ConstructBtPatchName(struct GLUE_INFO *prGlueInfo,
	uint8_t **apucName, uint8_t *pucNameIdx)
{
	struct ADAPTER *prAdapter = NULL;
	struct mt66xx_chip_info *prChipInfo = NULL;
	uint32_t u4FlavorVer = FLAVOR_VERSION_DEFAULT;

	if (prGlueInfo == NULL) {
		DBGLOG(INIT, ERROR, "prGlueInfo is NULL.\n");
		return;
	}

	prAdapter = prGlueInfo->prAdapter;
	if (prAdapter == NULL) {
		DBGLOG(INIT, ERROR, "prAdapter is NULL.\n");
		return;
	}

	prChipInfo = prAdapter->chip_info;
	if (prChipInfo == NULL) {
		DBGLOG(INIT, ERROR, "prChipInfo is NULL.\n");
		return;
	}

	snprintf(apucName[(*pucNameIdx)],
		CFG_FW_NAME_MAX_LEN, "BT_RAM_CODE_MT%x_%x_%x_hdr.bin",
		prChipInfo->chip_id, u4FlavorVer,
		mt7961GetFwVer(prAdapter));
}
#endif

uint32_t mt7922ConstructBufferBinFileName(struct ADAPTER *prAdapter,
					  uint8_t *aucEeprom)
{
	struct mt66xx_chip_info *prChipInfo;
	uint32_t u4FlavorVer = FLAVOR_VERSION_DEFAULT;

	if (prAdapter == NULL) {
		DBGLOG(INIT, ERROR, "prAdapter == NULL\n");
		return WLAN_STATUS_FAILURE;
	}

	prChipInfo = prAdapter->chip_info;

	if (prChipInfo == NULL) {
		DBGLOG(INIT, ERROR, "prChipInfo == NULL\n");
		return WLAN_STATUS_FAILURE;
	}

	snprintf(aucEeprom, 32, "EEPROM_MT%x_%x.bin",
		 prChipInfo->chip_id, u4FlavorVer);

	return WLAN_STATUS_SUCCESS;
}

u_int8_t mt7922GetRxDbgInfoSrc(struct ADAPTER *prAdapter)
{
	return TRUE;
}

struct BUS_INFO mt7922_bus_info = {
#if defined(_HIF_PCIE) || defined(_HIF_AXI)
	.top_cfg_base = MT7922_TOP_CFG_BASE,

	/* host_dma0 for TXP */
	.host_dma0_base = WF_WFDMA_HOST_DMA0_BASE,
	.host_int_status_addr = WF_WFDMA_HOST_DMA0_HOST_INT_STA_ADDR,

	.host_int_txdone_bits =
		(CONNAC2X_WFDMA_TX_DONE_INT0 | CONNAC2X_WFDMA_TX_DONE_INT1 |
		CONNAC2X_WFDMA_TX_DONE_INT2 | CONNAC2X_WFDMA_TX_DONE_INT3 |
		CONNAC2X_WFDMA_TX_DONE_INT4 | CONNAC2X_WFDMA_TX_DONE_INT5 |
		CONNAC2X_WFDMA_TX_DONE_INT6 | CONNAC2X_WFDMA_TX_DONE_INT16 |
		CONNAC2X_WFDMA_TX_DONE_INT17),
	.host_int_rxdone_bits =
		(CONNAC2X_WFDMA_RX_DONE_INT0 | CONNAC2X_WFDMA_RX_DONE_INT0 |
		 CONNAC2X_WFDMA_RX_DONE_INT2 | CONNAC2X_WFDMA_RX_DONE_INT3 |
		 CONNAC2X_WFDMA_RX_DONE_INT4 | CONNAC2X_WFDMA_RX_DONE_INT5),

	.host_tx_ring_base = WF_WFDMA_HOST_DMA0_WPDMA_TX_RING0_CTRL0_ADDR,
	.host_tx_ring_ext_ctrl_base =
		WF_WFDMA_HOST_DMA0_WPDMA_TX_RING0_EXT_CTRL_ADDR,
	.host_tx_ring_cidx_addr = WF_WFDMA_HOST_DMA0_WPDMA_TX_RING0_CTRL2_ADDR,
	.host_tx_ring_didx_addr = WF_WFDMA_HOST_DMA0_WPDMA_TX_RING0_CTRL3_ADDR,
	.host_tx_ring_cnt_addr = WF_WFDMA_HOST_DMA0_WPDMA_TX_RING0_CTRL1_ADDR,

	.host_rx_ring_base = WF_WFDMA_HOST_DMA0_WPDMA_RX_RING0_CTRL0_ADDR,
	.host_rx_ring_ext_ctrl_base =
		WF_WFDMA_HOST_DMA0_WPDMA_RX_RING0_EXT_CTRL_ADDR,
	.host_rx_ring_cidx_addr = WF_WFDMA_HOST_DMA0_WPDMA_RX_RING0_CTRL2_ADDR,
	.host_rx_ring_didx_addr = WF_WFDMA_HOST_DMA0_WPDMA_RX_RING0_CTRL3_ADDR,
	.host_rx_ring_cnt_addr = WF_WFDMA_HOST_DMA0_WPDMA_RX_RING0_CTRL1_ADDR,

	.bus2chip = mt7922_bus2chip_cr_mapping,
	.bus2chip_tbl_size = MT7922_BUS2CHIP_MEM_MAP_TBLE_SIZE,
	.max_static_map_addr = 0x00100000,

	.tx_ring_fwdl_idx = CONNAC2X_FWDL_TX_RING_IDX,
	.tx_ring_cmd_idx = CONNAC2X_CMD_TX_RING_IDX,
	.tx_ring0_data_idx = 0,
	.tx_ring1_data_idx = 1,
	.fw_own_clear_addr = CONNAC2X_BN0_IRQ_STAT_ADDR,
	.fw_own_clear_bit = PCIE_LPCR_FW_CLR_OWN,
	.fgCheckDriverOwnInt = FALSE,
	.u4DmaMask = 32,
	.pdmaSetup = asicConnac2xWpdmaConfig,
	.pdmaStop = asicConnac2xWfdmaStop,
	.pdmaPollingIdle = asicConnac2xWfdmaPollingAllIdle,
	.enableInterrupt = mt7961EnableInterrupt,
	.disableInterrupt = mt7961DisableInterrupt,
	.processTxInterrupt = mt7961Connac2xProcessTxInterrupt,
	.processRxInterrupt = mt7961Connac2xProcessRxInterrupt,
	.tx_ring_ext_ctrl = mt7961WfdmaTxRingExtCtrl,
	.rx_ring_ext_ctrl = mt7961WfdmaRxRingExtCtrl,
	.rx_evt_ring_buf_size = RX_BUFFER_AGGRESIZE,
	/* null wfdmaManualPrefetch if want to disable manual mode */
	.wfdmaManualPrefetch = mt7961Connac2xWfdmaManualPrefetch,
	.lowPowerOwnRead = asicConnac2xLowPowerOwnRead,
	.lowPowerOwnSet = asicConnac2xLowPowerOwnSet,
	.lowPowerOwnClear = asicConnac2xLowPowerOwnClear,
	.wakeUpWiFi = asicWakeUpWiFi,
	.getSoftwareInterrupt = asicConnac2xGetSoftwareInterrupt,
	.processSoftwareInterrupt = asicConnac2xProcessSoftwareInterrupt,
	.softwareInterruptMcu = asicConnac2xSoftwareInterruptMcu,
	.hifRst = asicConnac2xHifRst,

	.initPcieInt = wlanBuzzardInitPcieInt,
	.devReadIntStatus = mt7961ReadIntStatus,
	.DmaShdlInit = mt7961DmashdlInit,
	.DmaShdlReInit = mt7961DmashdlReInit,
	.setRxRingHwAddr = mt7961SetRxRingHwAddr,
	.wfdmaAllocRxRing = mt7961LiteWfdmaAllocRxRing,
#if CFG_SUPPORT_PCIE_ASPM_IMPROVE
	.configPcieASPM = mt7961ConfigPcieASPM,
	.setCTSbyRate = mt7961SetCTSbyRate,
#endif
#if CFG_SUPPORT_HOST_RX_WM_EVENT_FROM_PSE
#if defined(_HIF_PCIE)
	.checkPortForRxEventFromPse = mt7961CheckPortForRxEventFromPse,
#else
	.checkPortForRxEventFromPse = NULL,
#endif
#endif
#if (CFG_COALESCING_INTERRUPT == 1)
#if defined(_HIF_PCIE)
	.setWfdmaCoalescingInt = mt7961setWfdmaCoalescingInt,
#else
	.setWfdmaCoalescingInt = NULL,
#endif
#endif
	.updateTxRingMaxQuota = mt7961UpdateDmashdlQuota,
#endif /*_HIF_PCIE || _HIF_AXI */
};

#if CFG_ENABLE_FW_DOWNLOAD
struct FWDL_OPS_T mt7922_fw_dl_ops = {
	.constructFirmwarePrio = mt7922ConstructFirmwarePrio,
	.constructPatchName = mt7922ConstructPatchName,
	.downloadPatch = wlanDownloadPatch,
	.downloadFirmware = wlanConnacFormatDownload,
	.getFwInfo = wlanGetConnacFwInfo,
	.getFwDlInfo = asicGetFwDlInfo,
#if CFG_SUPPORT_WIFI_DL_BT_PATCH
	.constructBtPatchName = mt7922ConstructBtPatchName,
	.downloadBtPatch = mt7961DownloadBtPatch,
#endif
};
#endif /* CFG_ENABLE_FW_DOWNLOAD */

struct TX_DESC_OPS_T mt7922TxDescOps = {
	.fillNicAppend = fillNicTxDescAppend,
	.fillHifAppend = fillTxDescAppendByHostV2,
	.fillTxByteCount = fillConnac2xTxDescTxByteCount,
};

struct RX_DESC_OPS_T mt7922RxDescOps = {};

struct CHIP_DBG_OPS mt7922DebugOps = {
	.showPdmaInfo = mt7961_show_wfdma_info,
	.showPseInfo = mt7961_show_pse_info,
	.showPleInfo = mt7961_show_ple_info,
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
#if (CFG_SUPPORT_LINK_QUALITY_MONITOR == 1)
	.get_rx_rate_info = connac2x_get_rx_rate_info,
#endif
	.show_mcu_debug_info = NULL,
	.show_conninfra_debug_info = NULL,
#if (CFG_SUPPORT_DEBUG_SOP == 1)
	.show_debug_sop_info = mt7922_show_debug_sop_info,
#endif
};

#if CFG_SUPPORT_QA_TOOL
struct ATE_OPS_T mt7922_AteOps = {
	/*ICapStart phase out , wlan_service instead*/
	.setICapStart = connacSetICapStart,
	/*ICapStatus phase out , wlan_service instead*/
	.getICapStatus = connacGetICapStatus,
	/*CapIQData phase out , wlan_service instead*/
	.getICapIQData = connacGetICapIQData,
	.getRbistDataDumpEvent = nicExtEventICapIQData,
	.icapRiseVcoreClockRate = mt7922_icapRiseVcoreClockRate,
	.icapDownVcoreClockRate = mt7922_icapDownVcoreClockRate,
	.u4EnBitWidth = 4, /*no translate, keep 128 bit*/
	.u4Architech = 0,  /*0:on-chip*/
	.u4PhyIdx = 0,
	.u4EmiStartAddress = 0,
	.u4EmiEndAddress = 0,
	.u4EmiMsbAddress = 0,
};
#endif

/* Litien code refine to support multi chip */
struct mt66xx_chip_info mt66xx_chip_info_mt7922 = {
	.bus_info = &mt7922_bus_info,
#if CFG_ENABLE_FW_DOWNLOAD
	.fw_dl_ops = &mt7922_fw_dl_ops,
#endif /* CFG_ENABLE_FW_DOWNLOAD */
#if CFG_SUPPORT_QA_TOOL
	.prAteOps = &mt7922_AteOps,
#endif
	.prTxDescOps = &mt7922TxDescOps,
	.prRxDescOps = &mt7922RxDescOps,
	.prDebugOps = &mt7922DebugOps,
	.chip_id = MT7922_CHIP_ID,
	.should_verify_chip_id = FALSE,
	.sw_sync0 = MT7922_SW_SYNC0,
	.sw_ready_bits = WIFI_FUNC_NO_CR4_READY_BITS,
	.sw_ready_bit_offset = MT7922_SW_SYNC0_RDY_OFFSET,
	.patch_addr = MT7922_PATCH_START_ADDR,
	.is_support_cr4 = FALSE,
	.is_support_wacpu = FALSE,
	.txd_append_size = MT7922_TX_DESC_APPEND_LENGTH,
	.rxd_size = MT7922_RX_DESC_LENGTH,
	.pse_header_length = CONNAC2X_NIC_TX_PSE_HEADER_LENGTH,
	.init_event_size = CONNAC2X_RX_INIT_EVENT_LENGTH,
	.eco_info = mt7922_eco_table,
	.isNicCapV1 = FALSE,
	.is_support_efuse = TRUE,
	.top_hcr = CONNAC2X_TOP_HCR,
	.top_hvr = CONNAC2X_TOP_HVR,
	.top_fvr = CONNAC2X_TOP_FVR,
	.arb_ac_mode_addr = MT7922_ARB_AC_MODE_ADDR,
	.asicCapInit = asicConnac2xCapInit,
#if CFG_ENABLE_FW_DOWNLOAD
	.asicEnableFWDownload = NULL,
#endif /* CFG_ENABLE_FW_DOWNLOAD */
	.asicDumpSerDummyCR = NULL,
	.downloadBufferBin = wlanConnac2XDownloadBufferBin,
	.constructBufferBinFileName = mt7922ConstructBufferBinFileName,
	.is_support_hw_amsdu = TRUE,
	.is_support_asic_lp = TRUE,
	.is_support_wfdma1 = FALSE,
	.RxDbgInfoFromRxdGrp3 = mt7922GetRxDbgInfoSrc,
	.asicWfdmaReInit = asicConnac2xWfdmaReInit,
	.asicWfdmaReInit_handshakeInit = asicConnac2xWfdmaDummyCrWrite,
	.group5_size = sizeof(struct HW_MAC_RX_STS_GROUP_5),
	.u4LmacWtblDUAddr = MT7922_WIFI_LWTBL_BASE,
	.u4UmacWtblDUAddr = MT7922_WIFI_UWTBL_BASE,
	.u4SerSuspendSyncAddr = CONNAC2X_HOST_MAILBOX_WF_ADDR,

#if CFG_CHIP_RESET_SUPPORT
	.asicWfsysRst = mt7961HalCbtopRguWfRst,
	.asicPollWfsysSwInitDone = mt7961HalPollWfsysSwInitDone,
#endif
	.prTxPwrLimitFile = "TxPwrLimit_MT79x1.dat",
	.ucTxPwrLimitBatchSize = 8,

	/* buzzard capability:
	 * 1. MAC RX AMPDU max number is 256.
	 * 2. MAC RX MPDU max length is 11454.
	 * So, MAC RX AMPDU max length is 256 * 11454 = 2,932,224
	 *
	 * Set HE_CAP MAX_AMPDU_LEN_EXP to maximum which meets the condition
	 *     1. In 2.4G, 2 ^ (16 + MAX_AMPDU_LEN_EXP) - 1 <= 2,932,224
	 *     2. In 5G, 2 ^ (20 + MAX_AMPDU_LEN_EX) - 1 <= 2,932,224
	 */
	.uc2G4HeCapMaxAmpduLenExp = 3, /* 2 ^ (16 + 3) - 1 = 524287 */
	.uc5GHeCapMaxAmpduLenExp = 1, /* 2 ^ (20 + 1) - 1 = 2,097,151 */
	.loadCfgSetting = NULL,
#if defined(_HIF_PCIE) || defined(_HIF_AXI) || defined(_HIF_USB)
	.dmashdlQuotaDecision = mt7961dmashdlQuotaDecision,
#endif
};

struct mt66xx_hif_driver_data mt66xx_driver_data_mt7922 = {
	.chip_info = &mt66xx_chip_info_mt7922,
};

#endif /* MT7922 */
