/* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause */
/*
 * Copyright (c) 2016 MediaTek Inc.
 */
/******************************************************************************
 *[File]             dbg_mt7663.c
 *[Version]          v1.0
 *[Revision Date]    2019-04-09
 *[Author]
 *[Description]
 *    The program provides WIFI FALCON MAC Debug APIs
 *[Copyright]
 *    Copyright (C) 2015 MediaTek Incorporation. All Rights Reserved.
 ******************************************************************************/

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
#if (CFG_SUPPORT_RA_GEN == 1)
static char *RATE_TBLE[] = {"B", "G", "N", "N_2SS", "AC", "AC_2SS", "BG",
					"N/A"};

static char *RA_STATUS_TBLE[] = {"INVALID", "POWER_SAVING", "SLEEP", "STANDBY",
						"RUNNING", "N/A"};
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
#if (CFG_SUPPORT_RA_GEN == 1)
int32_t mt7663_show_stat_info(struct ADAPTER *prAdapter,
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
	uint8_t ucDbdcIdx, ucSkipAr, ucStaIdx, ucNss;
	static uint32_t u4TotalTxCnt[CFG_STAT_DBG_PEER_NUM] = {0};
	static uint32_t	u4TotalFailCnt[CFG_STAT_DBG_PEER_NUM] = {0};
	static uint32_t u4Rate1TxCnt[CFG_STAT_DBG_PEER_NUM] = {0};
	static uint32_t	u4Rate1FailCnt[CFG_STAT_DBG_PEER_NUM] = {0};
	static uint32_t au4RxMpduCnt[ENUM_BAND_NUM] = {0};
	static uint32_t au4FcsError[ENUM_BAND_NUM] = {0};
	static uint32_t au4RxFifoCnt[ENUM_BAND_NUM] = {0};
	static uint32_t au4AmpduTxSfCnt[ENUM_BAND_NUM] = {0};
	static uint32_t au4AmpduTxAckSfCnt[ENUM_BAND_NUM] = {0};
	struct RX_CTRL *prRxCtrl;
	uint32_t u4InstantRxPer[ENUM_BAND_NUM];
	struct PARAM_CUSTOM_SW_CTRL_STRUCT rSwCtrlInfo;
	uint32_t rStatus = WLAN_STATUS_SUCCESS;
	int16_t i2Wf0AvgPwr = 0, i2Wf1AvgPwr = 0;
	uint32_t u4BufLen = 0;
	uint8_t ucRaTableNum = sizeof(RATE_TBLE) / sizeof(char *);
	uint8_t ucRaStatusNum = sizeof(RA_STATUS_TBLE) / sizeof(char *);
	uint8_t ucAggRange[AGG_RANGE_SEL_NUM] = {0};
	uint32_t u4RangeCtrl_0 = 0, u4RangeCtrl_1 = 0;
	enum AGG_RANGE_TYPE_T eRangeType = ENUM_AGG_RANGE_TYPE_TX;
	uint16_t u2Idx = 0;

	ucSkipAr = prQueryStaStatistics->ucSkipAr;
	prRxCtrl = &prAdapter->rRxCtrl;
	ucNss = prAdapter->rWifiVar.ucNSS;

	if (ucSkipAr) {
		u2Idx = nicGetStatIdxInfo(prAdapter,
			(uint8_t)(prHwWlanInfo->u4Index));

		if (u2Idx == 0xFFFF)
			return i4BytesWritten;
	}

	if (ucSkipAr) {
		u4TotalTxCnt[u2Idx] += prQueryStaStatistics->u4TransmitCount;
		u4TotalFailCnt[u2Idx] += prQueryStaStatistics->
							u4TransmitFailCount;
		u4Rate1TxCnt[u2Idx] += prQueryStaStatistics->u4Rate1TxCnt;
		u4Rate1FailCnt[u2Idx] += prQueryStaStatistics->u4Rate1FailCnt;
	}

	if (ucSkipAr) {
		u4Per = (u4Rate1TxCnt[u2Idx] == 0) ?
			(0) : (1000 * (u4Rate1FailCnt[u2Idx]) /
			(u4Rate1TxCnt[u2Idx]));

		u4InstantPer = (prQueryStaStatistics->u4Rate1TxCnt == 0) ?
			(0) : (1000 * (prQueryStaStatistics->u4Rate1FailCnt) /
				(prQueryStaStatistics->u4Rate1TxCnt));
	} else {
		u4Per = (prQueryStaStatistics->u4Rate1TxCnt == 0) ?
			(0) : (1000 * (prQueryStaStatistics->u4Rate1FailCnt) /
				(prQueryStaStatistics->u4Rate1TxCnt));

		u4InstantPer = (prQueryStaStatistics->ucPer == 0) ?
			(0) : (prQueryStaStatistics->ucPer);
	}

	for (ucDbdcIdx = 0; ucDbdcIdx < ENUM_BAND_NUM; ucDbdcIdx++) {
		au4RxMpduCnt[ucDbdcIdx] += g_arMibInfo[ucDbdcIdx].u4RxMpduCnt;
		au4FcsError[ucDbdcIdx] += g_arMibInfo[ucDbdcIdx].u4FcsError;
		au4RxFifoCnt[ucDbdcIdx] += g_arMibInfo[ucDbdcIdx].u4RxFifoFull;
		au4AmpduTxSfCnt[ucDbdcIdx] +=
			g_arMibInfo[ucDbdcIdx].u4AmpduTxSfCnt;
		au4AmpduTxAckSfCnt[ucDbdcIdx] +=
			g_arMibInfo[ucDbdcIdx].u4AmpduTxAckSfCnt;

		u4RxPer[ucDbdcIdx] =
		    ((au4RxMpduCnt[ucDbdcIdx] + au4FcsError[ucDbdcIdx]) == 0) ?
			(0) : (1000 * au4FcsError[ucDbdcIdx] /
				(au4RxMpduCnt[ucDbdcIdx] +
				au4FcsError[ucDbdcIdx]));

		u4AmpduPer[ucDbdcIdx] =
		    (au4AmpduTxSfCnt[ucDbdcIdx] == 0) ?
			(0) : (1000 * (au4AmpduTxSfCnt[ucDbdcIdx] -
				au4AmpduTxAckSfCnt[ucDbdcIdx]) /
				au4AmpduTxSfCnt[ucDbdcIdx]);

		u4InstantRxPer[ucDbdcIdx] =
			((prQueryStaStatistics->rMibInfo[ucDbdcIdx].u4RxMpduCnt
			+ prQueryStaStatistics->rMibInfo[ucDbdcIdx].u4FcsError)
			== 0) ?
				(0) : (1000 * prQueryStaStatistics->
				rMibInfo[ucDbdcIdx].u4FcsError /
				(prQueryStaStatistics->rMibInfo[ucDbdcIdx].
				u4RxMpduCnt +
				prQueryStaStatistics->rMibInfo[ucDbdcIdx].
				u4FcsError));
	}

	rRssi = RCPI_TO_dBm(prQueryStaStatistics->ucRcpi);
	u2LinkSpeed = (prQueryStaStatistics->u2LinkSpeed == 0) ? 0 :
					prQueryStaStatistics->u2LinkSpeed / 2;

	if (ucSkipAr) {
		i4BytesWritten += kalScnprintf(pcCommand + i4BytesWritten,
			i4TotalLen - i4BytesWritten,
			"%-20s%s%d(%d)\n", "\nWlanIdx(BackupIdx)", "  = ",
			prHwWlanInfo->u4Index, u2Idx);
	} else {
		i4BytesWritten += kalScnprintf(pcCommand + i4BytesWritten,
			i4TotalLen - i4BytesWritten,
			"%-20s%s%d\n", "\nWlanIdx", "  = ",
			prHwWlanInfo->u4Index);
	}

	/* =========== Group 0x0001 =========== */
	if (u4StatGroup & 0x0001) {
		i4BytesWritten += kalScnprintf(pcCommand + i4BytesWritten,
			i4TotalLen - i4BytesWritten,
			"%s", "----- STA Stat (Group 0x01) -----\n");

		i4BytesWritten += kalScnprintf(pcCommand + i4BytesWritten,
			i4TotalLen - i4BytesWritten,
			"%-20s%s%d\n", "CurrTemperature", " = ",
			prQueryStaStatistics->ucTemperature);

		i4BytesWritten += kalScnprintf(pcCommand + i4BytesWritten,
			i4TotalLen - i4BytesWritten,
			"%-20s%s%d\n", "Tx Total cnt", " = ",
			ucSkipAr ? (u4TotalTxCnt[u2Idx]) :
				(prQueryStaStatistics->u4TransmitCount));

		i4BytesWritten += kalScnprintf(pcCommand + i4BytesWritten,
			i4TotalLen - i4BytesWritten,
			"%-20s%s%d\n", "Tx Fail Cnt", " = ",
			ucSkipAr ? (u4TotalFailCnt[u2Idx]) :
				(prQueryStaStatistics->u4TransmitFailCount));

		i4BytesWritten += kalScnprintf(pcCommand + i4BytesWritten,
			i4TotalLen - i4BytesWritten,
			"%-20s%s%d\n", "Rate1 Tx Cnt", " = ",
			ucSkipAr ? (u4Rate1TxCnt[u2Idx]) :
				(prQueryStaStatistics->u4Rate1TxCnt));

		if (ucSkipAr)
			i4BytesWritten += kalScnprintf(
				pcCommand + i4BytesWritten,
				i4TotalLen - i4BytesWritten,
				"%-20s%s%d, PER = %d.%1d%%, instant PER = %d.%1d%%\n",
				"Rate1 Fail Cnt", " = ",
				u4Rate1FailCnt[u2Idx], u4Per/10, u4Per%10,
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
			u4TotalTxCnt[u2Idx] = 0;
			u4TotalFailCnt[u2Idx] = 0;
			u4Rate1TxCnt[u2Idx] = 0;
			u4Rate1FailCnt[u2Idx] = 0;
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
				"RX with CRC", " = ", au4FcsError[ucDbdcIdx],
				u4RxPer[ucDbdcIdx]/10, u4RxPer[ucDbdcIdx]%10,
				u4InstantRxPer[ucDbdcIdx]/10,
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

		/* get Beacon RSSI */
		rStatus = kalIoctl(prAdapter->prGlueInfo,
				   wlanoidQueryRssi, &rRssi,
				   sizeof(rRssi), TRUE, TRUE, TRUE,
				   &u4BufLen);
		if (rStatus != WLAN_STATUS_SUCCESS)
			DBGLOG(REQ, WARN, "unable to retrieve rssi\n");

		rSwCtrlInfo.u4Data = 0;
		rSwCtrlInfo.u4Id = CMD_SW_DBGCTL_ADVCTL_GET_ID + 1;
#if 0
		rStatus = kalIoctl(prAdapter->prGlueInfo,
				   wlanoidQuerySwCtrlRead, &rSwCtrlInfo,
				   sizeof(rSwCtrlInfo), TRUE, TRUE, TRUE,
				   &u4BufLen);
#endif
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
				"%-20s%s%d %d %d %d\n",
				"Tx Response RSSI", " = ",
				RCPI_TO_dBm(
				    prHwWlanInfo->rWtblRxCounter.ucRxRcpi0),
				RCPI_TO_dBm(
				    prHwWlanInfo->rWtblRxCounter.ucRxRcpi1),
				RCPI_TO_dBm(
				    prHwWlanInfo->rWtblRxCounter.ucRxRcpi2),
				RCPI_TO_dBm(
				    prHwWlanInfo->rWtblRxCounter.ucRxRcpi3));
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
			int8_t txpwr, pos_txpwr;

			txpwr = TX_VECTOR_GET_TX_PWR(
				&prQueryStaStatistics->rTxVector[ucDbdcIdx]);

			if (prAdapter->rWifiVar.fgDbDcModeEn)
				i4BytesWritten += kalScnprintf(
					pcCommand + i4BytesWritten,
					i4TotalLen - i4BytesWritten,
					"[DBDC_%d] :\n", ucDbdcIdx);

			i4BytesWritten += nicTxGetVectorInfo(
				pcCommand + i4BytesWritten,
				i4TotalLen - i4BytesWritten,
				&prQueryStaStatistics->rTxVector[ucDbdcIdx]);

			if (prQueryStaStatistics->rTxVector[ucDbdcIdx]
			    .u4TxV[0] == 0xFFFFFFFF)
				i4BytesWritten += kalScnprintf(
					pcCommand + i4BytesWritten,
					i4TotalLen - i4BytesWritten,
					"%-20s%s%s\n", "Chip Out TX Power",
					" = ", "N/A");
			else {
				pos_txpwr = (txpwr < 0) ?
					(~txpwr + 1) : (txpwr);
				i4BytesWritten += kalScnprintf(
					pcCommand + i4BytesWritten,
					i4TotalLen - i4BytesWritten,
					"%-20s%s%c%d.%1d dBm\n",
					"Chip Out TX Power", " = ",
					(txpwr < 0) ? '-' : '+',
					(pos_txpwr / 2),
					5 * (pos_txpwr % 2));
			}

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
		/* RA info */
		i4BytesWritten += kalScnprintf(pcCommand + i4BytesWritten,
			i4TotalLen - i4BytesWritten,
			"%s", "----- RA Info (Group 0x20) -----\n");

		i4BytesWritten += kalScnprintf(pcCommand + i4BytesWritten,
			i4TotalLen - i4BytesWritten, "%-20s%s%d\n", "LinkSpeed",
			" = ", u2LinkSpeed);

		if (!prQueryStaStatistics->ucSkipAr) {
			i4BytesWritten += kalScnprintf(
				pcCommand + i4BytesWritten,
				i4TotalLen - i4BytesWritten,
				"%-20s%s%s\n", "RateTable", " = ",
				prQueryStaStatistics->ucArTableIdx <
				    (ucRaTableNum - 1) ?
				    RATE_TBLE[
					prQueryStaStatistics->ucArTableIdx] :
					RATE_TBLE[ucRaTableNum - 1]);

			if (wlanGetStaIdxByWlanIdx(prAdapter,
			    (uint8_t)(prHwWlanInfo->u4Index), &ucStaIdx) ==
			    WLAN_STATUS_SUCCESS){
				i4BytesWritten += kalScnprintf(
					pcCommand + i4BytesWritten,
					i4TotalLen - i4BytesWritten,
					"%-20s%s%d\n", "2G Support 256QAM TX",
					" = ",
					((prAdapter->arStaRec[ucStaIdx].u4Flags
					 & MTK_SYNERGY_CAP_SUPPORT_24G_MCS89) ||
					(prQueryStaStatistics->
					 ucDynamicGband256QAMState == 2)) ?
					1 : 0);
			}

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
					"%-20s%s%d -> %d\n", "Train Down",
					" = ",
					(uint16_t)
					(prQueryStaStatistics->u2TrainDown
						& BITS(0, 7)),
					(uint16_t)
					((prQueryStaStatistics->u2TrainDown >>
						8) & BITS(0, 7)));

				i4BytesWritten += kalScnprintf(
					pcCommand + i4BytesWritten,
					i4TotalLen - i4BytesWritten,
					"%-20s%s%d -> %d\n", "Train Up", " = ",
					(uint16_t)
					(prQueryStaStatistics->u2TrainUp
						& BITS(0, 7)),
					(uint16_t)
					((prQueryStaStatistics->u2TrainUp >> 8)
						& BITS(0, 7)));
			}

			if (prQueryStaStatistics->fgIsForceTxStream == 0)
				i4BytesWritten += kalScnprintf(
					pcCommand + i4BytesWritten,
					i4TotalLen - i4BytesWritten,
					"%-20s%s%s\n", "Force Tx Stream",
					" = ", "N/A");
			else
				i4BytesWritten += kalScnprintf(
					pcCommand + i4BytesWritten,
					i4TotalLen - i4BytesWritten,
					"%-20s%s%d\n", "Force Tx Stream", " = ",
					prQueryStaStatistics->
						fgIsForceTxStream);

			i4BytesWritten += kalScnprintf(
				pcCommand + i4BytesWritten,
				i4TotalLen - i4BytesWritten,
				"%-20s%s%d\n", "Force SE off", " = ",
				prQueryStaStatistics->fgIsForceSeOff);
		}

		i4BytesWritten += kalScnprintf(pcCommand + i4BytesWritten,
			i4TotalLen - i4BytesWritten,
			"%-20s%s%d\n", "RunningCnt", " = ",
			prQueryStaStatistics->u2RaRunningCnt);

		prQueryStaStatistics->ucRaStatus &= ~0x80;
		i4BytesWritten += kalScnprintf(pcCommand + i4BytesWritten,
			i4TotalLen - i4BytesWritten,
			"%-20s%s%s\n", "Status", " = ",
			prQueryStaStatistics->ucRaStatus < (ucRaStatusNum - 1) ?
			RA_STATUS_TBLE[prQueryStaStatistics->ucRaStatus] :
			RA_STATUS_TBLE[ucRaStatusNum - 1]);

		i4BytesWritten += kalScnprintf(pcCommand + i4BytesWritten,
			i4TotalLen - i4BytesWritten,
			"%-20s%s%d\n", "MaxAF", " = ",
			prHwWlanInfo->rWtblPeerCap.ucAmpduFactor);

		i4BytesWritten += kalScnprintf(pcCommand + i4BytesWritten,
			i4TotalLen - i4BytesWritten,
			"%-20s%s0x%x\n", "SpeIdx", " = ",
			prHwWlanInfo->rWtblPeerCap.ucSpatialExtensionIndex);

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
		u4RangeCtrl_0 = prQueryStaStatistics->u4AggRangeCtrl_0;
		u4RangeCtrl_1 = prQueryStaStatistics->u4AggRangeCtrl_1;
		eRangeType = (enum AGG_RANGE_TYPE_T)
					prQueryStaStatistics->ucRangeType;

		ucAggRange[0] = (((u4RangeCtrl_0) & AGG_RANGE_SEL_0_MASK) >>
						AGG_RANGE_SEL_0_OFFSET);
		ucAggRange[1] = (((u4RangeCtrl_0) & AGG_RANGE_SEL_1_MASK) >>
						AGG_RANGE_SEL_1_OFFSET);
		ucAggRange[2] = (((u4RangeCtrl_0) & AGG_RANGE_SEL_2_MASK) >>
						AGG_RANGE_SEL_2_OFFSET);
		ucAggRange[3] = (((u4RangeCtrl_0) & AGG_RANGE_SEL_3_MASK) >>
						AGG_RANGE_SEL_3_OFFSET);
		ucAggRange[4] = (((u4RangeCtrl_1) & AGG_RANGE_SEL_4_MASK) >>
						AGG_RANGE_SEL_4_OFFSET);
		ucAggRange[5] = (((u4RangeCtrl_1) & AGG_RANGE_SEL_5_MASK) >>
						AGG_RANGE_SEL_5_OFFSET);
		ucAggRange[6] = (((u4RangeCtrl_1) & AGG_RANGE_SEL_6_MASK) >>
						AGG_RANGE_SEL_6_OFFSET);

		/* Tx Agg */
		i4BytesWritten += kalScnprintf(
			pcCommand + i4BytesWritten,
			i4TotalLen - i4BytesWritten,
			"%s%s%s", "------ ",
			(eRangeType > ENUM_AGG_RANGE_TYPE_TX) ? (
				(eRangeType == ENUM_AGG_RANGE_TYPE_TRX) ?
				("TRX") : ("RX")) : ("TX"),
				" AGG (Group 0x40) -----\n");

		if (eRangeType == ENUM_AGG_RANGE_TYPE_TRX) {
			i4BytesWritten += kalScnprintf(
				pcCommand + i4BytesWritten,
				i4TotalLen - i4BytesWritten,
				"%-6s%8d%5d%1s%2d%5d%1s%2d%5d%3s",
				" TX  :", ucAggRange[0] + 1,
				ucAggRange[0] + 2, "~", ucAggRange[1] + 1,
				ucAggRange[1] + 2, "~", ucAggRange[2] + 1,
				ucAggRange[2] + 2, "~64\n");

			for (ucDbdcIdx = 0; ucDbdcIdx < ENUM_BAND_NUM;
			     ucDbdcIdx++) {
				i4BytesWritten += kalScnprintf(
					pcCommand + i4BytesWritten,
					i4TotalLen - i4BytesWritten,
					"DBDC%d:", ucDbdcIdx);
				i4BytesWritten += kalScnprintf(
					pcCommand + i4BytesWritten,
					i4TotalLen - i4BytesWritten,
					"%8d%8d%8d%8d\n",
					g_arMibInfo[ucDbdcIdx].
					au2TxRangeAmpduCnt[0],
					g_arMibInfo[ucDbdcIdx].
					au2TxRangeAmpduCnt[1],
					g_arMibInfo[ucDbdcIdx].
					au2TxRangeAmpduCnt[2],
					g_arMibInfo[ucDbdcIdx].
					au2TxRangeAmpduCnt[3]);

				if (!prAdapter->rWifiVar.fgDbDcModeEn)
					break;
			}

			i4BytesWritten += kalScnprintf(
				pcCommand + i4BytesWritten,
				i4TotalLen - i4BytesWritten,
				"%-6s%8d%5d%1s%2d%5d%1s%2d%5d%3s",
				" RX  :", ucAggRange[3] + 1,
				ucAggRange[3] + 2, "~", ucAggRange[4] + 1,
				ucAggRange[4] + 2, "~", ucAggRange[5] + 1,
				ucAggRange[5] + 2, "~64\n");

			for (ucDbdcIdx = 0; ucDbdcIdx < ENUM_BAND_NUM;
			     ucDbdcIdx++) {
				i4BytesWritten += kalScnprintf(
					pcCommand + i4BytesWritten,
					i4TotalLen - i4BytesWritten,
					"DBDC%d:", ucDbdcIdx);
				i4BytesWritten += kalScnprintf(
					pcCommand + i4BytesWritten,
					i4TotalLen - i4BytesWritten,
					"%8d%8d%8d%8d\n",
					g_arMibInfo[ucDbdcIdx].
					au2TxRangeAmpduCnt[4],
					g_arMibInfo[ucDbdcIdx].
					au2TxRangeAmpduCnt[5],
					g_arMibInfo[ucDbdcIdx].
					au2TxRangeAmpduCnt[6],
					g_arMibInfo[ucDbdcIdx].
					au2TxRangeAmpduCnt[7]);

				if (!prAdapter->rWifiVar.fgDbDcModeEn)
					break;
			}
		} else {
			i4BytesWritten += kalScnprintf(
			pcCommand + i4BytesWritten, i4TotalLen - i4BytesWritten,
				"%-6s%8d%5d%1s%2d%5d%1s%2d%5d%1s%2d%5d%1s%2d%5d%1s%2d%5d%1s%2d%5d%3s",
				"Range:", ucAggRange[0] + 1,
				ucAggRange[0] + 2, "~", ucAggRange[1] + 1,
				ucAggRange[1] + 2, "~", ucAggRange[2] + 1,
				ucAggRange[2] + 2, "~", ucAggRange[3] + 1,
				ucAggRange[3] + 2, "~", ucAggRange[4] + 1,
				ucAggRange[4] + 2, "~", ucAggRange[5] + 1,
				ucAggRange[5] + 2, "~", ucAggRange[6] + 1,
				ucAggRange[6] + 2, "~64\n");

			for (ucDbdcIdx = 0; ucDbdcIdx < ENUM_BAND_NUM;
			     ucDbdcIdx++) {
				i4BytesWritten += kalScnprintf(
					pcCommand + i4BytesWritten,
					i4TotalLen - i4BytesWritten,
					"DBDC%d:", ucDbdcIdx);
				i4BytesWritten += kalScnprintf(
					pcCommand + i4BytesWritten,
					i4TotalLen - i4BytesWritten,
					"%8d%8d%8d%8d%8d%8d%8d%8d\n",
					g_arMibInfo[ucDbdcIdx].
					au2TxRangeAmpduCnt[0],
					g_arMibInfo[ucDbdcIdx].
					au2TxRangeAmpduCnt[1],
					g_arMibInfo[ucDbdcIdx].
					au2TxRangeAmpduCnt[2],
					g_arMibInfo[ucDbdcIdx].
					au2TxRangeAmpduCnt[3],
					g_arMibInfo[ucDbdcIdx].
					au2TxRangeAmpduCnt[4],
					g_arMibInfo[ucDbdcIdx].
					au2TxRangeAmpduCnt[5],
					g_arMibInfo[ucDbdcIdx].
					au2TxRangeAmpduCnt[6],
					g_arMibInfo[ucDbdcIdx].
					au2TxRangeAmpduCnt[7]);

				if (!prAdapter->rWifiVar.fgDbDcModeEn)
					break;
			}
		}
	}

	kalMemZero(g_arMibInfo, sizeof(g_arMibInfo));

	return i4BytesWritten;
}
#endif
#endif /* MT7663 */
