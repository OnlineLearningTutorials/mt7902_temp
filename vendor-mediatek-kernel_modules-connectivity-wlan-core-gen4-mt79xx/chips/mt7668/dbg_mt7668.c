/* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause */
/*
 * Copyright (c) 2016 MediaTek Inc.
 */
/******************************************************************************
 *[File]             dbg_mt7668.c
 *[Version]          v1.0
 *[Revision Date]    2019-04-09
 *[Author]
 *[Description]
 *    The program provides WIFI FALCON MAC Debug APIs
 *[Copyright]
 *    Copyright (C) 2015 MediaTek Incorporation. All Rights Reserved.
 ******************************************************************************/

#ifdef MT7668
/*******************************************************************************
 *                         C O M P I L E R   F L A G S
 *******************************************************************************
 */

/*******************************************************************************
 *                    E X T E R N A L   R E F E R E N C E S
 *******************************************************************************
 */
#include "precomp.h"

/*******************************************************************************
 *                              C O N S T A N T S
 *******************************************************************************
 */

/*******************************************************************************
 *                             D A T A   T Y P E S
 *******************************************************************************
 */

/*******************************************************************************
 *                            P U B L I C   D A T A
 *******************************************************************************
 */

/*******************************************************************************
 *                           P R I V A T E   D A T A
 *******************************************************************************
 */
#if (CFG_SUPPORT_RA_GEN == 0)
static char *RATE_TBLE[] = {"B", "G", "N", "N_2SS", "AC", "AC_2SS", "N/A"};
#endif
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
#if (CFG_SUPPORT_RA_GEN == 0)
int32_t mt7668_show_stat_info(struct ADAPTER *prAdapter,
			char *pcCommand, int32_t i4TotalLen,
			struct PARAM_HW_WLAN_INFO *prHwWlanInfo,
			struct PARAM_GET_STA_STATISTICS *prQueryStaStatistics,
			uint8_t fgResetCnt, uint32_t u4StatGroup)
{
	int32_t i4BytesWritten = 0;
	int32_t rRssi;
	uint16_t u2LinkSpeed;
	uint32_t u4Per, u4RxPer[ENUM_BAND_NUM], u4AmpduPer[ENUM_BAND_NUM],
		 u4InstantPer;
	uint8_t ucDbdcIdx, ucStaIdx, ucNss;
	uint8_t ucSkipAr;
	static uint32_t u4TotalTxCnt, u4TotalFailCnt;
	static uint32_t u4Rate1TxCnt, u4Rate1FailCnt;
	static uint32_t au4RxMpduCnt[ENUM_BAND_NUM] = {0};
	static uint32_t au4FcsError[ENUM_BAND_NUM] = {0};
	static uint32_t au4RxFifoCnt[ENUM_BAND_NUM] = {0};
	static uint32_t au4AmpduTxSfCnt[ENUM_BAND_NUM] = {0};
	static uint32_t au4AmpduTxAckSfCnt[ENUM_BAND_NUM] = {0};
	struct RX_CTRL *prRxCtrl;
	uint32_t u4InstantRxPer[ENUM_BAND_NUM];
	struct PARAM_CUSTOM_SW_CTRL_STRUCT rSwCtrlInfo;
	uint32_t rStatus = WLAN_STATUS_SUCCESS;
	int16_t i2Wf0AvgPwr;
	int16_t i2Wf1AvgPwr;
	uint32_t u4BufLen = 0;

	ucSkipAr = prQueryStaStatistics->ucSkipAr;
	prRxCtrl = &prAdapter->rRxCtrl;
	ucNss = prAdapter->rWifiVar.ucNSS;

	if (ucSkipAr) {
		u4TotalTxCnt += prHwWlanInfo->rWtblTxCounter.u2CurBwTxCnt +
				prHwWlanInfo->rWtblTxCounter.u2OtherBwTxCnt;
		u4TotalFailCnt += prHwWlanInfo->rWtblTxCounter.u2CurBwFailCnt +
				prHwWlanInfo->rWtblTxCounter.u2OtherBwFailCnt;
		u4Rate1TxCnt += prHwWlanInfo->rWtblTxCounter.u2Rate1TxCnt;
		u4Rate1FailCnt += prHwWlanInfo->rWtblTxCounter.u2Rate1FailCnt;
	}

	if (ucSkipAr) {
		u4Per = (prHwWlanInfo->rWtblTxCounter.u2Rate1TxCnt == 0) ?
			(0) : (1000 * u4Rate1FailCnt / u4Rate1TxCnt);

		u4InstantPer =
			(prHwWlanInfo->rWtblTxCounter.u2Rate1TxCnt == 0) ? (0) :
			(1000 * (prHwWlanInfo->rWtblTxCounter.u2Rate1FailCnt) /
			(prHwWlanInfo->rWtblTxCounter.u2Rate1TxCnt));
	} else {
		u4Per = (prQueryStaStatistics->u4Rate1TxCnt == 0) ?
			(0) : (1000 * (prQueryStaStatistics->u4Rate1FailCnt) /
			(prQueryStaStatistics->u4Rate1TxCnt));

		u4InstantPer = (prQueryStaStatistics->ucPer == 0) ?
					(0) : (prQueryStaStatistics->ucPer);
	}

	for (ucDbdcIdx = 0; ucDbdcIdx < ENUM_BAND_NUM; ucDbdcIdx++) {
		au4RxMpduCnt[ucDbdcIdx] +=
		    prQueryStaStatistics->rMibInfo[ucDbdcIdx].u4RxMpduCnt;
		au4FcsError[ucDbdcIdx] +=
		    prQueryStaStatistics->rMibInfo[ucDbdcIdx].u4FcsError;
		au4RxFifoCnt[ucDbdcIdx] +=
		    prQueryStaStatistics->rMibInfo[ucDbdcIdx].u4RxFifoFull;
		au4AmpduTxSfCnt[ucDbdcIdx] +=
		    prQueryStaStatistics->rMibInfo[ucDbdcIdx].u4AmpduTxSfCnt;
		au4AmpduTxAckSfCnt[ucDbdcIdx] +=
		    prQueryStaStatistics->rMibInfo[ucDbdcIdx].u4AmpduTxAckSfCnt;

		u4RxPer[ucDbdcIdx] = ((au4RxMpduCnt[ucDbdcIdx] +
		    au4FcsError[ucDbdcIdx]) == 0) ? (0) :
		    (1000 * au4FcsError[ucDbdcIdx] /
		    (au4RxMpduCnt[ucDbdcIdx] + au4FcsError[ucDbdcIdx]));

		u4AmpduPer[ucDbdcIdx] =
		    (au4AmpduTxSfCnt[ucDbdcIdx] == 0) ? (0) :
		    (1000 * (au4AmpduTxSfCnt[ucDbdcIdx] -
		    au4AmpduTxAckSfCnt[ucDbdcIdx]) /
		    au4AmpduTxSfCnt[ucDbdcIdx]);

		u4InstantRxPer[ucDbdcIdx] =
		    ((prQueryStaStatistics->rMibInfo[ucDbdcIdx].u4RxMpduCnt +
		    prQueryStaStatistics->rMibInfo[ucDbdcIdx].u4FcsError) == 0)
		    ? (0) :
		    (1000 * prQueryStaStatistics->rMibInfo[ucDbdcIdx].u4FcsError
		    / (prQueryStaStatistics->rMibInfo[ucDbdcIdx].u4RxMpduCnt +
		    prQueryStaStatistics->rMibInfo[ucDbdcIdx].u4FcsError));
	}

	/* get Beacon RSSI */
	rStatus = kalIoctl(prAdapter->prGlueInfo, wlanoidQueryRssi, &rRssi,
			   sizeof(rRssi), TRUE, FALSE, TRUE, &u4BufLen);
	if (rStatus != WLAN_STATUS_SUCCESS)
		DBGLOG(REQ, WARN, "unable to retrieve rssi\n");

	u2LinkSpeed = (prQueryStaStatistics->u2LinkSpeed == 0) ?
				0 : prQueryStaStatistics->u2LinkSpeed / 2;

	/* =========== Group 0x0001 =========== */
	if (u4StatGroup & 0x0001) {
		i4BytesWritten += kalScnprintf(pcCommand + i4BytesWritten,
			i4TotalLen - i4BytesWritten,
			"%s", "\n----- STA Stat (Group 0x01) -----\n");

		i4BytesWritten += kalScnprintf(pcCommand + i4BytesWritten,
			i4TotalLen - i4BytesWritten,
			"%-20s%s%d\n", "CurrTemperature", " = ",
			prQueryStaStatistics->ucTemperature);

		i4BytesWritten += kalScnprintf(pcCommand + i4BytesWritten,
			i4TotalLen - i4BytesWritten,
			"%-20s%s%d\n", "Tx Total cnt", " = ",
			ucSkipAr ? (u4TotalTxCnt) :
				(prQueryStaStatistics->u4TransmitCount));

		i4BytesWritten += kalScnprintf(pcCommand + i4BytesWritten,
			i4TotalLen - i4BytesWritten,
			"%-20s%s%d\n", "Tx Fail Cnt", " = ",
			ucSkipAr ? (u4TotalFailCnt) :
				(prQueryStaStatistics->u4TransmitFailCount));

		i4BytesWritten += kalScnprintf(pcCommand + i4BytesWritten,
			i4TotalLen - i4BytesWritten,
			"%-20s%s%d\n", "Rate1 Tx Cnt", " = ",
			ucSkipAr ? (u4Rate1TxCnt) :
				(prQueryStaStatistics->u4Rate1TxCnt));

		if (ucSkipAr)
			i4BytesWritten += kalScnprintf(
				pcCommand + i4BytesWritten,
				i4TotalLen - i4BytesWritten,
				"%-20s%s%d, PER = %d.%1d%%, instant PER = %d.%1d%%\n",
				"Rate1 Fail Cnt", " = ",
				u4Rate1FailCnt, u4Per/10, u4Per%10,
				u4InstantPer/10, u4InstantPer%10);
		else
			i4BytesWritten += kalScnprintf(
				pcCommand + i4BytesWritten,
				i4TotalLen - i4BytesWritten,
				"%-20s%s%d, PER = %d.%1d%%, instant PER = %d%%\n",
				"Rate1 Fail Cnt", " = ",
				prQueryStaStatistics->u4Rate1FailCnt,
				u4Per/10, u4Per%10, u4InstantPer);

		if ((ucSkipAr) && (fgResetCnt)) {
			u4TotalTxCnt = 0;
			u4TotalFailCnt = 0;
			u4Rate1TxCnt = 0;
			u4Rate1FailCnt = 0;
		}
	}

	/* =========== Group 0x0002 =========== */
	if (u4StatGroup & 0x0002) {
		i4BytesWritten += kalScnprintf(pcCommand + i4BytesWritten,
			i4TotalLen - i4BytesWritten,
			"%s", "----- MIB Info (Group 0x02) -----\n");

		for (ucDbdcIdx = 0; ucDbdcIdx < ENUM_BAND_NUM; ucDbdcIdx++) {
			if (prAdapter->rWifiVar.fgDbDcModeEn)
				i4BytesWritten += kalScnprintf(
					pcCommand + i4BytesWritten,
					i4TotalLen - i4BytesWritten,
					"[DBDC_%d] :\n", ucDbdcIdx);

			i4BytesWritten += kalScnprintf(
				pcCommand + i4BytesWritten,
				i4TotalLen - i4BytesWritten,
				"%-20s%s%d\n", "RX Success", " = ",
				au4RxMpduCnt[ucDbdcIdx]);

			i4BytesWritten += kalScnprintf(
				pcCommand + i4BytesWritten,
				i4TotalLen - i4BytesWritten,
				"%-20s%s%d, PER = %d.%1d%%, instant PER = %d.%1d%%\n",
				"RX with CRC", " = ",
				au4FcsError[ucDbdcIdx], u4RxPer[ucDbdcIdx]/10,
				u4RxPer[ucDbdcIdx] % 10,
				u4InstantRxPer[ucDbdcIdx] / 10,
				u4InstantRxPer[ucDbdcIdx]%10);

			i4BytesWritten += kalScnprintf(
				pcCommand + i4BytesWritten,
				i4TotalLen - i4BytesWritten,
				"%-20s%s%d\n", "RX drop FIFO full", " = ",
				au4RxFifoCnt[ucDbdcIdx]);

			if (!prAdapter->rWifiVar.fgDbDcModeEn)
				break;
		}

		if (fgResetCnt) {
			kalMemZero(au4RxMpduCnt, sizeof(au4RxMpduCnt));
			kalMemZero(au4FcsError, sizeof(au4RxMpduCnt));
			kalMemZero(au4RxFifoCnt, sizeof(au4RxMpduCnt));
			kalMemZero(au4AmpduTxSfCnt, sizeof(au4RxMpduCnt));
			kalMemZero(au4AmpduTxAckSfCnt, sizeof(au4RxMpduCnt));
		}
	}

	/* =========== Group 0x0004 =========== */
	if (u4StatGroup & 0x0004) {
		i4BytesWritten += kalScnprintf(pcCommand + i4BytesWritten,
			i4TotalLen - i4BytesWritten,
			"%s", "----- Last Rx Info (Group 0x04) -----\n");

		rSwCtrlInfo.u4Data = 0;
		rSwCtrlInfo.u4Id = CMD_SW_DBGCTL_ADVCTL_GET_ID + 1;

		rStatus = kalIoctl(prAdapter->prGlueInfo,
			wlanoidQuerySwCtrlRead, &rSwCtrlInfo,
			sizeof(rSwCtrlInfo), TRUE, TRUE, TRUE, &u4BufLen);

		DBGLOG(REQ, LOUD, "rStatus %u, rSwCtrlInfo.u4Data 0x%x\n",
		       rStatus, rSwCtrlInfo.u4Data);
		if (rStatus == WLAN_STATUS_SUCCESS) {
			i2Wf0AvgPwr = rSwCtrlInfo.u4Data & 0xFFFF;
			i2Wf1AvgPwr = (rSwCtrlInfo.u4Data >> 16) & 0xFFFF;

			i4BytesWritten += kalScnprintf(
				pcCommand + i4BytesWritten,
				i4TotalLen - i4BytesWritten,
				"%-20s%s%d %d\n", "NOISE", " = ",
				i2Wf0AvgPwr, i2Wf1AvgPwr);
		}

		/* Last RX Rate */
		i4BytesWritten += nicGetRxRateInfo(prAdapter,
			pcCommand + i4BytesWritten, i4TotalLen - i4BytesWritten,
			(uint8_t)(prHwWlanInfo->u4Index));

		/* Last RX RSSI */
		i4BytesWritten += nicRxGetLastRxRssi(prAdapter,
			pcCommand + i4BytesWritten, i4TotalLen - i4BytesWritten,
			(uint8_t)(prHwWlanInfo->u4Index));

		/* Last TX Resp RSSI */
		if (ucNss > 2)
			i4BytesWritten += kalScnprintf(
				pcCommand + i4BytesWritten,
				i4TotalLen - i4BytesWritten,
				"%-20s%s%d %d %d %d\n", "Tx Response RSSI",
				" = ",
				RCPI_TO_dBm(prHwWlanInfo->
					rWtblRxCounter.ucRxRcpi0),
				RCPI_TO_dBm(prHwWlanInfo->
					rWtblRxCounter.ucRxRcpi1),
				RCPI_TO_dBm(prHwWlanInfo->
					rWtblRxCounter.ucRxRcpi2),
				RCPI_TO_dBm(prHwWlanInfo->
					rWtblRxCounter.ucRxRcpi3));
		else
			i4BytesWritten += kalScnprintf(
				pcCommand + i4BytesWritten,
				i4TotalLen - i4BytesWritten,
				"%-20s%s%d %d\n", "Tx Response RSSI", " = ",
				RCPI_TO_dBm(
				    prHwWlanInfo->rWtblRxCounter.ucRxRcpi0),
				RCPI_TO_dBm(
				    prHwWlanInfo->rWtblRxCounter.ucRxRcpi1));

		/* Last Beacon RSSI */
		i4BytesWritten += kalScnprintf(pcCommand + i4BytesWritten,
				i4TotalLen - i4BytesWritten,
				"%-20s%s%d\n", "Beacon RSSI", " = ", rRssi);
	}

	/* =========== Group 0x0008 =========== */
	if (u4StatGroup & 0x0008) {
		/* TxV */
		i4BytesWritten += kalScnprintf(pcCommand + i4BytesWritten,
			i4TotalLen - i4BytesWritten,
			"%s", "----- Last TX Info (Group 0x08) -----\n");

		for (ucDbdcIdx = 0; ucDbdcIdx < ENUM_BAND_NUM; ucDbdcIdx++) {
			if (prAdapter->rWifiVar.fgDbDcModeEn)
				i4BytesWritten += kalScnprintf(
					pcCommand + i4BytesWritten,
					i4TotalLen - i4BytesWritten,
					"[DBDC_%d] :\n", ucDbdcIdx);

			i4BytesWritten += nicTxGetVectorInfo(
				pcCommand + i4BytesWritten,
				i4TotalLen - i4BytesWritten,
				&prQueryStaStatistics->rTxVector[ucDbdcIdx]);

			if (prQueryStaStatistics->
			    rTxVector[ucDbdcIdx].u4TxV[0] == 0xFFFFFFFF)
				i4BytesWritten += kalScnprintf(
					pcCommand + i4BytesWritten,
					i4TotalLen - i4BytesWritten,
					"%-20s%s%s\n", "Chip Out TX Power",
					" = ", "N/A");
			else
				i4BytesWritten += kalScnprintf(
					pcCommand + i4BytesWritten,
					i4TotalLen - i4BytesWritten,
					"%-20s%s%ld.%1ld dBm\n",
					"Chip Out TX Power", " = ",
					TX_VECTOR_GET_TX_PWR(
					    &prQueryStaStatistics->
					    rTxVector[ucDbdcIdx]) >> 1,
					5 * (TX_VECTOR_GET_TX_PWR(
					    &prQueryStaStatistics->rTxVector[
					    ucDbdcIdx]) % 2));

			if (!prAdapter->rWifiVar.fgDbDcModeEn)
				break;
		}
	}

	/* =========== Group 0x0010 =========== */
	if (u4StatGroup & 0x0010) {
		/* RX Reorder */
		i4BytesWritten += kalScnprintf(pcCommand + i4BytesWritten,
			i4TotalLen - i4BytesWritten,
			"%s", "----- RX Reorder (Group 0x10) -----\n");
		i4BytesWritten += kalScnprintf(pcCommand + i4BytesWritten,
			i4TotalLen - i4BytesWritten,
			"%-20s%s%ld\n", "Rx reorder miss", " = ",
			RX_GET_CNT(prRxCtrl, RX_DATA_REORDER_MISS_COUNT));
		i4BytesWritten += kalScnprintf(pcCommand + i4BytesWritten,
			i4TotalLen - i4BytesWritten,
			"%-20s%s%ld\n", "Rx reorder within", " = ",
			RX_GET_CNT(prRxCtrl, RX_DATA_REORDER_WITHIN_COUNT));
		i4BytesWritten += kalScnprintf(pcCommand + i4BytesWritten,
			i4TotalLen - i4BytesWritten,
			"%-20s%s%ld\n", "Rx reorder ahead", " = ",
			RX_GET_CNT(prRxCtrl, RX_DATA_REORDER_AHEAD_COUNT));
		i4BytesWritten += kalScnprintf(pcCommand + i4BytesWritten,
			i4TotalLen - i4BytesWritten,
			"%-20s%s%ld\n", "Rx reorder behind", " = ",
			RX_GET_CNT(prRxCtrl, RX_DATA_REORDER_BEHIND_COUNT));
	}

	/* =========== Group 0x0020 =========== */
	if (u4StatGroup & 0x0020) {
		/* AR info */
		i4BytesWritten += kalScnprintf(pcCommand + i4BytesWritten,
				i4TotalLen - i4BytesWritten,
				"%s", "----- AR Info (Group 0x20) -----\n");

		/* Last TX Rate */
		i4BytesWritten += nicGetTxRateInfo(
				pcCommand + i4BytesWritten,
				i4TotalLen - i4BytesWritten, FALSE,
				prHwWlanInfo, prQueryStaStatistics);

		i4BytesWritten += kalScnprintf(pcCommand + i4BytesWritten,
				i4TotalLen - i4BytesWritten,
				"%-20s%s%d\n", "LinkSpeed", " = ", u2LinkSpeed);

		if (!prQueryStaStatistics->ucSkipAr) {
			i4BytesWritten += kalScnprintf(
				pcCommand + i4BytesWritten,
				i4TotalLen - i4BytesWritten,
				"%-20s%s%s\n", "RateTable", " = ",
				prQueryStaStatistics->ucArTableIdx < 6 ?
					RATE_TBLE[
					prQueryStaStatistics->ucArTableIdx] :
					RATE_TBLE[6]);

			if (wlanGetStaIdxByWlanIdx(prAdapter,
			    (uint8_t)(prHwWlanInfo->u4Index), &ucStaIdx) ==
				WLAN_STATUS_SUCCESS){
				i4BytesWritten += kalScnprintf(
					pcCommand + i4BytesWritten,
					i4TotalLen - i4BytesWritten,
					"%-20s%s%d\n", "2G Support 256QAM TX",
					" = ",
					(prAdapter->arStaRec[ucStaIdx].u4Flags &
					    MTK_SYNERGY_CAP_SUPPORT_24G_MCS89) ?
					    1 : 0);
			}

			i4BytesWritten += kalScnprintf(
				pcCommand + i4BytesWritten,
				i4TotalLen - i4BytesWritten,
				"%-20s%s%d%%\n", "Rate1 instantPer", " = ",
				u4InstantPer);

			if (prQueryStaStatistics->ucAvePer == 0xFF) {
				i4BytesWritten += kalScnprintf(
					pcCommand + i4BytesWritten,
					i4TotalLen - i4BytesWritten,
					"%-20s%s%s\n", "Train Down", " = ",
					"N/A");

				i4BytesWritten += kalScnprintf(
					pcCommand + i4BytesWritten,
					i4TotalLen - i4BytesWritten,
					"%-20s%s%s\n", "Train Up", " = ",
					"N/A");
			} else {
				i4BytesWritten += kalScnprintf(
					pcCommand + i4BytesWritten,
					i4TotalLen - i4BytesWritten,
					"%-20s%s%d -> %hd\n", "Train Down",
					" = ",
					(uint16_t)((prQueryStaStatistics
					    ->u2TrainDown) & BITS(0, 7)),
					(uint16_t)(((prQueryStaStatistics
					    ->u2TrainDown) >> 8) & BITS(0, 7)));

				i4BytesWritten += kalScnprintf(
					pcCommand + i4BytesWritten,
					i4TotalLen - i4BytesWritten,
					"%-20s%s%d -> %hd\n", "Train Up", " = ",
					(uint16_t)((prQueryStaStatistics->
						u2TrainUp) & BITS(0, 7)),
					(uint16_t)(((prQueryStaStatistics->
						u2TrainUp) >> 8) & BITS(0, 7)));
			}

			if (prQueryStaStatistics->fgIsForceTxStream == 0)
				i4BytesWritten += kalScnprintf(
				    pcCommand + i4BytesWritten,
				    i4TotalLen - i4BytesWritten,
				    "%-20s%s%s\n", "Force Tx Stream", " = ",
				    "N/A");
			else
				i4BytesWritten += kalScnprintf(
				    pcCommand + i4BytesWritten,
				    i4TotalLen - i4BytesWritten,
				    "%-20s%s%d\n", "Force Tx Stream", " = ",
				    prQueryStaStatistics->fgIsForceTxStream);

			i4BytesWritten += kalScnprintf(
				pcCommand + i4BytesWritten,
				i4TotalLen - i4BytesWritten,
				"%-20s%s%d\n", "Force SE off", " = ",
				prQueryStaStatistics->fgIsForceSeOff);
		}

		i4BytesWritten += kalScnprintf(pcCommand + i4BytesWritten,
			i4TotalLen - i4BytesWritten,
			"%-20s%s%d\n", "CBRN", " = ",
			prHwWlanInfo->rWtblPeerCap.ucChangeBWAfterRateN);

		/* Rate1~Rate8 */
		i4BytesWritten += nicGetTxRateInfo(
			pcCommand + i4BytesWritten, i4TotalLen - i4BytesWritten,
			TRUE, prHwWlanInfo, prQueryStaStatistics);
	}

	/* =========== Group 0x0040 =========== */
	if (u4StatGroup & 0x0040) {
		/* Tx Agg */
		i4BytesWritten += kalScnprintf(pcCommand + i4BytesWritten,
			i4TotalLen - i4BytesWritten,
			"%s", "------ TX AGG (Group 0x40) -----\n");

		i4BytesWritten += kalScnprintf(pcCommand + i4BytesWritten,
			i4TotalLen - i4BytesWritten,
			"%-12s%s", "Range:",
			"1     2~5     6~15    16~22    23~33    34~49    50~57    58~64\n");

		for (ucDbdcIdx = 0; ucDbdcIdx < ENUM_BAND_NUM; ucDbdcIdx++) {
			i4BytesWritten += kalScnprintf(
				pcCommand + i4BytesWritten,
				i4TotalLen - i4BytesWritten,
				"DBDC%d:", ucDbdcIdx);
			i4BytesWritten += kalScnprintf(
				pcCommand + i4BytesWritten,
				i4TotalLen - i4BytesWritten,
				"%7d%8d%9d%9d%9d%9d%9d%9d\n",
				prQueryStaStatistics->
					rMibInfo[ucDbdcIdx].
					au2TxRangeAmpduCnt[0],
				prQueryStaStatistics->
					rMibInfo[ucDbdcIdx].
					au2TxRangeAmpduCnt[1],
				prQueryStaStatistics->
					rMibInfo[ucDbdcIdx].
					au2TxRangeAmpduCnt[2],
				prQueryStaStatistics->
					rMibInfo[ucDbdcIdx].
					au2TxRangeAmpduCnt[3],
				prQueryStaStatistics->
					rMibInfo[ucDbdcIdx].
					au2TxRangeAmpduCnt[4],
				prQueryStaStatistics->
					rMibInfo[ucDbdcIdx].
					au2TxRangeAmpduCnt[5],
				prQueryStaStatistics->
					rMibInfo[ucDbdcIdx].
					au2TxRangeAmpduCnt[6],
				prQueryStaStatistics->
					rMibInfo[ucDbdcIdx].
					au2TxRangeAmpduCnt[7]);

			if (!prAdapter->rWifiVar.fgDbDcModeEn)
				break;
		}
	}

	return i4BytesWritten;
}
#endif
#endif /* MT7668 */
