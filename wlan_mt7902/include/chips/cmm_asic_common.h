/* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause */
/*
 * Copyright (c) 2016 MediaTek Inc.
 */
/*! \file  cmm_connac_common.h
 *    \brief This file contains the info of CONNAC
 */

#ifndef _CMM_ASIC_COMMON_H
#define _CMM_ASIC_COMMON_H

/*******************************************************************************
 *                              C O N S T A N T S
 *******************************************************************************
 */

/*******************************************************************************
 *                  F U N C T I O N   D E C L A R A T I O N S
 *******************************************************************************
 */
uint32_t asicGetFwDlInfo(struct ADAPTER *prAdapter,
	char *pcBuf, int i4TotalLen);
uint32_t asicGetChipID(struct ADAPTER *prAdapter);
void fillNicTxDescAppend(IN struct ADAPTER *prAdapter,
	IN struct MSDU_INFO *prMsduInfo,
	OUT uint8_t *prTxDescBuffer);
void fillTxDescAppendByHostV2(IN struct ADAPTER *prAdapter,
	IN struct MSDU_INFO *prMsduInfo, IN uint16_t u4MsduId,
	IN dma_addr_t rDmaAddr, IN uint32_t u4Idx, IN u_int8_t fgIsLast,
	OUT uint8_t *pucBuffer);
void halDumpTxdInfo(IN struct ADAPTER *prAdapter, uint32_t *tmac_info);
void asicWakeUpWiFi(IN struct ADAPTER *prAdapter);
#endif /* _CMM_ASIC_COMMON_H */

