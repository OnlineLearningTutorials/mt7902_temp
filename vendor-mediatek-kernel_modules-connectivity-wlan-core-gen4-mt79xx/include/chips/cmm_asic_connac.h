/* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause */
/*
 * Copyright (c) 2016 MediaTek Inc.
 */
/*
 ** Id: //Department/DaVinci/BRANCHES/
 *			MT6620_WIFI_DRIVER_V2_3/include/chips/connac.h#1
 */

/*! \file  connac.h
 *    \brief This file contains the info of CONNAC
 */

#ifndef _CMM_ASIC_CONNAC_H
#define _CMM_ASIC_CONNAC_H

/*******************************************************************************
 *                              C O N S T A N T S
 *******************************************************************************
 */
#define CONNAC_CHIP_IP_VERSION			(0x10020300)
#define CONNAC_CHIP_IP_CONFIG			(0x1)
#define USB_HIF_TXD_LEN    4
#define NIC_TX_PSE_HEADER_LENGTH                4

/*******************************************************************************
 *                  F U N C T I O N   D E C L A R A T I O N S
 *******************************************************************************
 */
void asicCapInit(IN struct ADAPTER *prAdapter);
#if (CFG_SUPPORT_CONNAC3X == 0)
uint32_t asicGetFwDlInfo(struct ADAPTER *prAdapter,
	char *pcBuf, int i4TotalLen);
#endif
void asicEnableFWDownload(IN struct ADAPTER *prAdapter,
	IN u_int8_t fgEnable);
uint32_t asicGetChipID(struct ADAPTER *prAdapter);
#if (CFG_SUPPORT_CONNAC3X == 0)
void fillNicTxDescAppend(IN struct ADAPTER *prAdapter,
	IN struct MSDU_INFO *prMsduInfo,
	OUT uint8_t *prTxDescBuffer);
#endif
void fillNicTxDescAppendWithCR4(IN struct ADAPTER *prAdapter,
	IN struct MSDU_INFO *prMsduInfo,
	OUT uint8_t *prTxDescBuffer);
void fillTxDescAppendByHost(IN struct ADAPTER *prAdapter,
	IN struct MSDU_INFO *prMsduInfo, IN uint16_t u4MsduId,
	IN phys_addr_t rDmaAddr, IN uint32_t u4Idx, IN u_int8_t fgIsLast,
	OUT uint8_t *pucBuffer);
#if (CFG_SUPPORT_CONNAC3X == 0)
void fillTxDescAppendByHostV2(IN struct ADAPTER *prAdapter,
	IN struct MSDU_INFO *prMsduInfo, IN uint16_t u4MsduId,
	IN dma_addr_t rDmaAddr, IN uint32_t u4Idx, IN u_int8_t fgIsLast,
	OUT uint8_t *pucBuffer);
#endif
void fillTxDescAppendByCR4(IN struct ADAPTER *prAdapter,
	IN struct MSDU_INFO *prMsduInfo, IN uint16_t u4MsduId,
	IN phys_addr_t rDmaAddr, IN uint32_t u4Idx, IN u_int8_t fgIsLast,
	OUT uint8_t *pucBuffer);
void fillTxDescTxByteCount(IN struct ADAPTER *prAdapter,
	IN struct MSDU_INFO *prMsduInfo,
	void *prTxDesc);
void fillTxDescTxByteCountWithCR4(IN struct ADAPTER *prAdapter,
	IN struct MSDU_INFO *prMsduInfo,
	void *prTxDesc);

#if defined(_HIF_PCIE) || defined(_HIF_AXI)
/* DMS Scheduler Init */
void asicPcieDmaShdlInit(IN struct ADAPTER *prAdapter);
void asicPdmaLoopBackConfig(struct GLUE_INFO *prGlueInfo, u_int8_t fgEnable);
void asicPdmaConfig(struct GLUE_INFO *prGlueInfo, u_int8_t fgEnable);
uint32_t asicUpdatTxRingMaxQuota(IN struct ADAPTER *prAdapter,
	IN uint8_t ucWmmIndex, IN uint32_t u4MaxQuota);
void asicEnableInterrupt(IN struct ADAPTER *prAdapter);
void asicDisableInterrupt(IN struct ADAPTER *prAdapter);
void asicLowPowerOwnRead(IN struct ADAPTER *prAdapter, OUT u_int8_t *pfgResult);
void asicLowPowerOwnSet(IN struct ADAPTER *prAdapter, OUT u_int8_t *pfgResult);
void asicLowPowerOwnClear(IN struct ADAPTER *prAdapter,
	OUT u_int8_t *pfgResult);
void asicLowPowerOwnClearPCIe(IN struct ADAPTER *prAdapter,
	OUT u_int8_t *pfgResult);
#if (CFG_SUPPORT_CONNAC3X == 0)
void asicWakeUpWiFi(IN struct ADAPTER *prAdapter);
#endif
bool asicIsValidRegAccess(IN struct ADAPTER *prAdapter, IN uint32_t u4Register);
void asicGetMailboxStatus(IN struct ADAPTER *prAdapter, OUT uint32_t *pu4Val);
void asicSetDummyReg(struct GLUE_INFO *prGlueInfo);
void asicCheckDummyReg(struct GLUE_INFO *prGlueInfo);
void asicPdmaTxRingExtCtrl(
	struct GLUE_INFO *prGlueInfo,
	struct RTMP_TX_RING *tx_ring,
	uint32_t index);
void asicPdmaRxRingExtCtrl(
	struct GLUE_INFO *prGlueInfo,
	struct RTMP_RX_RING *rx_ring,
	uint32_t index);
#endif /* _HIF_PCIE */

#if defined(_HIF_USB)
/* DMS Scheduler Init */
void asicUsbDmaShdlInit(IN struct ADAPTER *prAdapter);
void asicUdmaTxTimeoutEnable(IN struct ADAPTER *prAdapter);
u_int8_t asicUsbSuspend(IN struct ADAPTER *prAdapter,
	IN struct GLUE_INFO *prGlueInfo);
uint8_t asicUsbEventEpDetected(IN struct ADAPTER *prAdapter);
void asicUdmaRxFlush(IN struct ADAPTER *prAdapter, IN u_int8_t bEnable);
void asicPdmaHifReset(IN struct ADAPTER *prAdapter, IN u_int8_t bRelease);
void fillUsbHifTxDesc(IN uint8_t **pDest, IN uint16_t *pInfoBufLen,
	IN uint8_t ucPacketType);
#endif /* _HIF_USB */

#if defined(_HIF_SDIO)
void fillSdioHifTxDesc(IN uint8_t **pDest, IN uint16_t *pInfoBufLen,
	IN uint8_t ucPacketType);
#endif /* _HIF_SDIO */

void asicFillInitCmdTxd(
	struct ADAPTER *prAdapter,
	struct WIFI_CMD_INFO *prCmdInfo,
	uint16_t *pu2BufInfoLen,
	u_int8_t *pucSeqNum,
	void **pCmdBuf);
void asicFillCmdTxd(
	struct ADAPTER *prAdapter,
	struct WIFI_CMD_INFO *prCmdInfo,
	u_int8_t *pucSeqNum,
	void **pCmdBuf);
void asicInitTxdHook(
	struct TX_DESC_OPS_T *prTxDescOps);
void asicInitRxdHook(
	struct RX_DESC_OPS_T *prRxDescOps);
#if (CFG_SUPPORT_MSP == 1)
void asicRxProcessRxvforMSP(
	IN struct ADAPTER *prAdapter,
	IN OUT struct SW_RFB *prRetSwRfb);
#endif /* CFG_SUPPORT_MSP == 1 */
uint8_t asicRxGetRcpiValueFromRxv(
	IN uint8_t ucRcpiMode,
	IN struct SW_RFB *prSwRfb,
	IN struct ADAPTER *prAdapter);
#if (CFG_SUPPORT_PERF_IND == 1)
void asicRxPerfIndProcessRXV(IN struct ADAPTER *prAdapter,
	IN struct SW_RFB *prSwRfb,
	IN uint8_t ucBssIndex);
#endif

#if (CFG_CHIP_RESET_SUPPORT == 1) && (CFG_WMT_RESET_API_SUPPORT == 0)
u_int8_t conn1_rst_L0_notify_step2(void);
#endif
#endif /* _CMM_ASIC_CONNAC_H */

