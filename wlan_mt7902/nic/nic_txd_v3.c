/* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause */
/*
 * Copyright (c) 2016 MediaTek Inc.
 */
/*
 ** Id: //Department/DaVinci/BRANCHES/MT6620_WIFI_DRIVER_v3_3/nic/nic_tx.c#2
 */

/*! \file   nic_tx.c
 *    \brief  Functions that provide TX operation in NIC Layer.
 *
 *    This file provides TX functions which are responsible for both Hardware
 *    and Software Resource Management and keep their Synchronization.
 */


#if (CFG_SUPPORT_CONNAC3X == 1)
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

/*******************************************************************************
 *                                 M A C R O S
 *******************************************************************************
 */

/*******************************************************************************
 *                  F U N C T I O N   D E C L A R A T I O N S
 *******************************************************************************
 */

/*******************************************************************************
 *                              F U N C T I O N S
 *******************************************************************************
 */

uint8_t nic_txd_v3_long_format_op(
	void *prTxDesc,
	uint8_t fgSet)
{
	return TRUE;
}

uint8_t nic_txd_v3_tid_op(
	void *prTxDesc,
	uint8_t ucTid,
	uint8_t fgSet)
{
	if (fgSet)
		HAL_MAC_CONNAC3X_TXD_SET_TID_MGMT_TYPE(
			(struct HW_MAC_CONNAC3X_TX_DESC *)prTxDesc, ucTid);
	return HAL_MAC_CONNAC3X_TXD_GET_TID_MGMT_TYPE(
			(struct HW_MAC_CONNAC3X_TX_DESC *)prTxDesc);
}

uint8_t nic_txd_v3_queue_idx_op(
	void *prTxDesc,
	uint8_t ucQueIdx,
	uint8_t fgSet)
{
	if (fgSet)
		HAL_MAC_CONNAC3X_TXD_SET_QUEUE_INDEX(
			(struct HW_MAC_CONNAC3X_TX_DESC *)prTxDesc, ucQueIdx);
	return HAL_MAC_CONNAC3X_TXD_GET_QUEUE_INDEX(
			(struct HW_MAC_CONNAC3X_TX_DESC *)prTxDesc);
}

#if (CFG_TCP_IP_CHKSUM_OFFLOAD == 1)
void nic_txd_v3_chksum_op(
	void *prTxDesc,
	uint8_t ucChksumFlag)
{
	if ((ucChksumFlag & TX_CS_IP_GEN))
		HAL_MAC_CONNAC3X_TXD_SET_IP_CHKSUM(
			(struct HW_MAC_CONNAC3X_TX_DESC *)prTxDesc);
	if ((ucChksumFlag & TX_CS_TCP_UDP_GEN))
		HAL_MAC_CONNAC3X_TXD_SET_TCP_UDP_CHKSUM(
			(struct HW_MAC_CONNAC3X_TX_DESC *)prTxDesc);
}
#endif /* CFG_TCP_IP_CHKSUM_OFFLOAD == 1 */

void nic_txd_v3_header_format_op(
	void *prTxDesc,
	struct MSDU_INFO *prMsduInfo)
{
	if (!prMsduInfo->fgIs802_11) {
		if (prMsduInfo->fgIs802_3)
			HAL_MAC_CONNAC3X_TXD_UNSET_ETHERNET_II(
				(struct HW_MAC_CONNAC3X_TX_DESC *)prTxDesc);
		if (prMsduInfo->fgIsVlanExists)
			HAL_MAC_CONNAC3X_TXD_SET_VLAN(
				(struct HW_MAC_CONNAC3X_TX_DESC *)prTxDesc);
	}
}

void nic_txd_v3_fill_by_pkt_option(
	struct MSDU_INFO *prMsduInfo,
	void *prTxD)
{
	struct HW_MAC_CONNAC3X_TX_DESC *prTxDesc =
				(struct HW_MAC_CONNAC3X_TX_DESC *)prTxD;
	uint32_t u4PktOption = prMsduInfo->u4Option;
	u_int8_t fgProtected = FALSE;

	/* Skip this function if no options is set */
	if (!u4PktOption)
		return;

	/* Fields in DW0 and DW1 (Short Format) */
	if (u4PktOption & MSDU_OPT_NO_ACK)
		HAL_MAC_CONNAC3X_TXD_SET_NO_ACK(prTxDesc);

	if (u4PktOption & MSDU_OPT_PROTECTED_FRAME) {
		/* DBGLOG(RSN, INFO, "MSDU_OPT_PROTECTED_FRAME\n"); */
		HAL_MAC_CONNAC3X_TXD_SET_PROTECTION(prTxDesc);
		fgProtected = TRUE;
	}

	switch (HAL_MAC_CONNAC3X_TXD_GET_HEADER_FORMAT(prTxDesc)) {
	case HEADER_FORMAT_802_11_ENHANCE_MODE:
		if (u4PktOption & MSDU_OPT_EOSP)
			HAL_MAC_CONNAC3X_TXD_SET_EOSP(prTxDesc);

		if (u4PktOption & MSDU_OPT_AMSDU)
			HAL_MAC_CONNAC3X_TXD_SET_AMSDU(prTxDesc);
		break;

	case HEADER_FORMAT_NON_802_11:
		if (u4PktOption & MSDU_OPT_EOSP)
			HAL_MAC_CONNAC3X_TXD_SET_EOSP(prTxDesc);

		if (u4PktOption & MSDU_OPT_MORE_DATA)
			HAL_MAC_CONNAC3X_TXD_SET_MORE_DATA(prTxDesc);

		if (u4PktOption & MSDU_OPT_REMOVE_VLAN)
			HAL_MAC_CONNAC3X_TXD_SET_REMOVE_VLAN(prTxDesc);
		break;

	case HEADER_FORMAT_802_11_NORMAL_MODE:
		if (fgProtected && prMsduInfo->prPacket) {
			struct WLAN_MAC_HEADER *prWlanHeader =
			    (struct WLAN_MAC_HEADER *)
			    ((unsigned long) (prMsduInfo->prPacket)
				+ MAC_TX_RESERVED_FIELD);

			prWlanHeader->u2FrameCtrl |= MASK_FC_PROTECTED_FRAME;
		}
		break;

	default:
		break;
	}

	/* Fields in DW2~6 (Long Format) */
	if (u4PktOption & MSDU_OPT_NO_AGGREGATE)
		HAL_MAC_CONNAC3X_TXD_SET_BA_DISABLE(prTxDesc);

	if (u4PktOption & MSDU_OPT_FORCE_RTS)
		HAL_MAC_CONNAC3X_TXD_SET_FORCE_RTS_CTS(prTxDesc);

	/* SW field */
	if (u4PktOption & MSDU_OPT_SW_DURATION)
		HAL_MAC_CONNAC3X_TXD_SET_DURATION_CONTROL_BY_SW(prTxDesc);

	if (u4PktOption & MSDU_OPT_SW_PS_BIT)
		HAL_MAC_CONNAC3X_TXD_SET_SW_PM_CONTROL(prTxDesc);

	if (u4PktOption & MSDU_OPT_SW_HTC)
		HAL_MAC_CONNAC3X_TXD_SET_HTC_EXIST(prTxDesc);

	if (u4PktOption & MSDU_OPT_MANUAL_SN) {
		HAL_MAC_CONNAC3X_TXD_SET_TXD_SN_VALID(prTxDesc);
		HAL_MAC_CONNAC3X_TXD_SET_SEQUENCE_NUMBER
			(prTxDesc, prMsduInfo->u2SwSN);
	}

}

/*----------------------------------------------------------------------------*/
/*!
* @brief In this function, we'll compose the Tx descriptor of the MSDU.
*
* @param prAdapter              Pointer to the Adapter structure.
* @param prMsduInfo             Pointer to the Msdu info
* @param prTxDesc               Pointer to the Tx descriptor buffer
*
* @retval VOID
*/
/*----------------------------------------------------------------------------*/
void nic_txd_v3_compose(
	struct ADAPTER *prAdapter,
	struct MSDU_INFO *prMsduInfo,
	u_int32_t u4TxDescLength,
	u_int8_t fgIsTemplate,
	u_int8_t *prTxDescBuffer)
{
	struct HW_MAC_CONNAC3X_TX_DESC *prTxDesc;
	struct STA_RECORD *prStaRec;
	struct BSS_INFO *prBssInfo;
	u_int32_t u4TxDescAndPaddingLength;
	u_int8_t ucWmmQueSet, ucTarQueue, ucTarPort;
	uint8_t ucEtherTypeOffsetInWord;
	uint8_t *apucPktType[ENUM_PKT_FLAG_NUM] = {
		(uint8_t *) DISP_STRING("INVALID"),
		(uint8_t *) DISP_STRING("802_3"),
		(uint8_t *) DISP_STRING("1X"),
		(uint8_t *) DISP_STRING("PROTECTED_1X"),
		(uint8_t *) DISP_STRING("NON_PROTECTED_1X"),
		(uint8_t *) DISP_STRING("VLAN_EXIST"),
		(uint8_t *) DISP_STRING("DHCP"),
		(uint8_t *) DISP_STRING("ARP"),
		(uint8_t *) DISP_STRING("ICMP"),
		(uint8_t *) DISP_STRING("TDLS"),
		(uint8_t *) DISP_STRING("DNS")
	};

	prTxDesc = (struct HW_MAC_CONNAC3X_TX_DESC *) prTxDescBuffer;
	prBssInfo = GET_BSS_INFO_BY_INDEX(prAdapter, prMsduInfo->ucBssIndex);
	prStaRec = cnmGetStaRecByIndex(prAdapter, prMsduInfo->ucStaRecIndex);

	u4TxDescAndPaddingLength = u4TxDescLength + NIC_TX_DESC_PADDING_LENGTH;

	kalMemZero(prTxDesc, u4TxDescAndPaddingLength);

	ucTarPort = nicTxGetTxDestPortIdxByTc(prMsduInfo->ucTC);

	/** DW0 **/
	/* Packet Format */
	ucWmmQueSet = prBssInfo->ucWmmQueSet;
	if (fgIsTemplate != TRUE
		&& prMsduInfo->ucPacketType == TX_PACKET_TYPE_DATA
		&& ucWmmQueSet != prMsduInfo->ucWmmQueSet) {
		DBGLOG(RSN, ERROR,
			"ucStaRecIndex:%x ucWmmQueSet mismatch[%d,%d]\n",
			prMsduInfo->ucStaRecIndex,
			ucWmmQueSet, prMsduInfo->ucWmmQueSet);
	}

	ucTarQueue = nicTxGetTxDestQIdxByTc(prMsduInfo->ucTC);
	if (ucTarPort == PORT_INDEX_LMAC)
		ucTarQueue +=
			(ucWmmQueSet * WMM_AC_INDEX_NUM);

	HAL_MAC_CONNAC3X_TXD_SET_QUEUE_INDEX(prTxDesc, ucTarQueue);

	/* Packet Format */
	HAL_MAC_CONNAC3X_TXD_SET_PKT_FORMAT(
		prTxDesc, prMsduInfo->ucPacketFormat);

	/* Ether-type offset */
	if (prMsduInfo->fgIs802_3) {
		ucEtherTypeOffsetInWord = ((ETHER_HEADER_LEN - ETHER_TYPE_LEN)
				+ prAdapter->chip_info->pse_header_length) >> 1;

		HAL_MAC_CONNAC3X_TXD_SET_ETHER_TYPE_OFFSET(
			prTxDesc, ucEtherTypeOffsetInWord);
	}

	/** DW1 **/
	/* MLDID-legacy */
	{
		prMsduInfo->ucWlanIndex = nicTxGetWlanIdx(prAdapter,
			prMsduInfo->ucBssIndex, prMsduInfo->ucStaRecIndex);
		HAL_MAC_CONNAC3X_TXD_SET_MLD_ID(
			prTxDesc, prMsduInfo->ucWlanIndex);
	}

	/* MLDID-by TID */
	/* TODO: ADD MLDID by TID */

	/* Header format */
	if (prMsduInfo->fgIs802_11) {
		HAL_MAC_CONNAC3X_TXD_SET_HEADER_FORMAT(
			prTxDesc, HEADER_FORMAT_802_11_NORMAL_MODE);
		HAL_MAC_CONNAC3X_TXD_SET_802_11_HEADER_LENGTH(
			prTxDesc, (prMsduInfo->ucMacHeaderLength >> 1));
		HAL_MAC_CONNAC3X_TXD_SET_HEADER_PADDING(
			prTxDesc, NIC_TX_DESC_HEADER_PADDING_TAIL_PAD);
	} else {
		HAL_MAC_CONNAC3X_TXD_SET_HEADER_FORMAT(
			prTxDesc, HEADER_FORMAT_NON_802_11);
		HAL_MAC_CONNAC3X_TXD_SET_ETHERNET_II(prTxDesc);
		HAL_MAC_CONNAC3X_TXD_SET_HEADER_PADDING(
			prTxDesc, NIC_TX_DESC_HEADER_PADDING_PAD_HEAD_PAD);
	}
	HAL_MAC_CONNAC3X_TXD_SET_HEADER_PADDING(
		prTxDesc, NIC_TX_DESC_HEADER_PADDING_LENGTH);

	/* TID mgmt type */
	if (prMsduInfo->fgIs802_11) {
		HAL_MAC_CONNAC3X_TXD_SET_TID_MGMT_TYPE(prTxDesc,
			TYPE_NORMAL_MANAGEMENT);
	} else {
		HAL_MAC_CONNAC3X_TXD_SET_TID_MGMT_TYPE(prTxDesc,
			prMsduInfo->ucUserPriority);
	}

	/* Own MAC */
	HAL_MAC_CONNAC3X_TXD_SET_OWN_MAC_INDEX(
		prTxDesc, prBssInfo->ucOwnMacIndex);

	/** DW2 **/
	/* Type */
	if (prMsduInfo->fgIs802_11) {
		struct WLAN_MAC_HEADER *prWlanHeader =
			(struct WLAN_MAC_HEADER *)
			((unsigned long) (prMsduInfo->prPacket) +
			MAC_TX_RESERVED_FIELD);

		HAL_MAC_CONNAC3X_TXD_SET_TYPE(
			prTxDesc,
			(prWlanHeader->u2FrameCtrl & MASK_FC_TYPE) >> 2);

		HAL_MAC_CONNAC3X_TXD_SET_SUB_TYPE(
			prTxDesc,
			(prWlanHeader->u2FrameCtrl & MASK_FC_SUBTYPE)
				>> OFFSET_OF_FC_SUBTYPE);
	} else {
		HAL_MAC_CONNAC3X_TXD_SET_TYPE(
			prTxDesc,
			MAC_FRAME_TYPE_DATA >> 2);

		HAL_MAC_CONNAC3X_TXD_SET_SUB_TYPE(
			prTxDesc,
			prStaRec->fgIsQoS ?
				MAC_FRAME_QOS_DATA >> 4 : MAC_FRAME_DATA >> 4);
	}

	/* Remaining TX time */
	if (!(prMsduInfo->u4Option & MSDU_OPT_MANUAL_LIFE_TIME))
		prMsduInfo->u4RemainingLifetime =
			nicTxGetRemainingTxTimeByTc(prMsduInfo->ucTC);
	HAL_MAC_CONNAC3X_TXD_SET_REMAINING_LIFE_TIME_IN_MS(
		prTxDesc, prMsduInfo->u4RemainingLifetime);

	/* Power Offset */
	HAL_MAC_CONNAC3X_TXD_SET_POWER_OFFSET(
		prTxDesc, prMsduInfo->cPowerOffset);

	/* OM MAP */
	/* HAL_MAC_CONNAC3X_TXD_SET_OM_MAP(prTxDesc); */

	/** DW3 **/
	/* Tx count limit */
	if (!(prMsduInfo->u4Option & MSDU_OPT_MANUAL_RETRY_LIMIT)) {
		/* Note: BMC packet retry limit is set to unlimited */
		prMsduInfo->ucRetryLimit =
			nicTxGetTxCountLimitByTc(prMsduInfo->ucTC);
	}
	HAL_MAC_CONNAC3X_TXD_SET_REMAINING_TX_COUNT(
		prTxDesc, prMsduInfo->ucRetryLimit);

	/* BMC packet */
	if (prMsduInfo->ucStaRecIndex == STA_REC_INDEX_BMCAST) {
		HAL_MAC_CONNAC3X_TXD_SET_BMC(prTxDesc);
		/* Must set No ACK to mask retry bit in FC */
		HAL_MAC_CONNAC3X_TXD_SET_NO_ACK(prTxDesc);
	}

	/* Protection */
	if (secIsProtectedFrame(prAdapter, prMsduInfo, prStaRec)) {
		/* Update Packet option, */
		/* PF bit will be set in nicTxFillDescByPktOption() */
		if ((prStaRec
			&& prStaRec->fgTransmitKeyExist) || fgIsTemplate) {
			DBGLOG_LIMITED(RSN, TRACE,
				"Set MSDU_OPT_PROTECTED_FRAME\n");
			nicTxConfigPktOption(
				prMsduInfo, MSDU_OPT_PROTECTED_FRAME, TRUE);

			if (prMsduInfo->fgIs802_1x &&
			    prMsduInfo->fgIs802_1x_NonProtected) {
				nicTxConfigPktOption(
					prMsduInfo,
					MSDU_OPT_PROTECTED_FRAME, FALSE);
				DBGLOG(RSN, LOUD,
					"Pairwise EAPoL not protect!\n");
			}
		} else if (prMsduInfo->ucStaRecIndex
				== STA_REC_INDEX_BMCAST) {/* BMC packet */
			nicTxConfigPktOption(
				prMsduInfo, MSDU_OPT_PROTECTED_FRAME, TRUE);
			DBGLOG(RSN, LOUD, "Protect BMC frame!\n");
		}
	}

	/* Update Packet option */
	nic_txd_v3_fill_by_pkt_option(prMsduInfo, prTxDesc);

	/** DW5 **/
	/* PID */
	if (prMsduInfo->pfTxDoneHandler) {
		prMsduInfo->ucPID = nicTxAssignPID(
				prAdapter, prMsduInfo->ucWlanIndex);

		DBGLOG(TX, INFO, "TX[%s] PID[%d]\n",
			apucPktType[prMsduInfo->ucPktType], prMsduInfo->ucPID);
		HAL_MAC_CONNAC3X_TXD_SET_PID(prTxDesc, prMsduInfo->ucPID);
		HAL_MAC_CONNAC3X_TXD_SET_TXS_TO_MCU(prTxDesc);
	} else if (prAdapter->rWifiVar.ucDataTxDone == 2) {
		/* Log mode: only TxS to FW, no event to driver */
		HAL_MAC_CONNAC3X_TXD_SET_PID(
			prTxDesc, NIC_TX_DESC_PID_RESERVED);
		HAL_MAC_CONNAC3X_TXD_SET_TXS_TO_MCU(prTxDesc);
	}

	/* TODO: ADD ForceLink(DW5 BIT15) */

	/** DW6 **/
	/* Disable MLD to link address translation */
	if (prMsduInfo->ucPacketType == TX_PACKET_TYPE_MGMT)
		HAL_MAC_CONNAC3X_TXD_SET_DIS_MAT(prTxDesc);

	/* Msdu count */
	HAL_MAC_CONNAC3X_TXD_SET_MSDU_COUNT(prTxDesc, 1);

	/** DW7 **/
	HAL_MAC_CONNAC3X_TXD_SET_TXD_LENGTH(prTxDesc, TXD_LEN_1_PAGE);

	/* Fix rate */
	DBGLOG(TX, INFO, "Rate mode[%d]\n", prMsduInfo->ucRateMode);
	/* TODO: Revise rate configuration for low rate */
	switch (prMsduInfo->ucRateMode) {
	case MSDU_RATE_MODE_MANUAL_DESC:
		HAL_MAC_CONNAC3X_TXD_SET_FIXED_RATE_ENABLE(prTxDesc);
		/* TODO: by band configure */
		HAL_MAC_CONNAC3X_TXD_SET_FIXED_RATE_IDX(prTxDesc, 0);
	case MSDU_RATE_MODE_MANUAL_CR:
	case MSDU_RATE_MODE_AUTO:
	default:
		break;
	}
}

void nic_txd_v3_compose_security_frame(
	struct ADAPTER *prAdapter,
	struct CMD_INFO *prCmdInfo,
	uint8_t *prTxDescBuffer,
	uint8_t *pucTxDescLength)
{
	struct HW_MAC_CONNAC3X_TX_DESC *prTxDesc
		= (struct HW_MAC_CONNAC3X_TX_DESC *) prTxDescBuffer;
	uint8_t ucTxDescAndPaddingLength
		= NIC_TX_DESC_LONG_FORMAT_LENGTH + NIC_TX_DESC_PADDING_LENGTH;
	struct BSS_INFO *prBssInfo;
	uint8_t ucTid = 0;
	uint8_t ucTempTC = TC4_INDEX;
	void *prNativePacket;
	uint8_t ucEtherTypeOffsetInWord;
	struct MSDU_INFO *prMsduInfo;

	prMsduInfo = prCmdInfo->prMsduInfo;
	prBssInfo = GET_BSS_INFO_BY_INDEX(prAdapter, prMsduInfo->ucBssIndex);
	prNativePacket = prMsduInfo->prPacket;

	ASSERT(prNativePacket);

	kalMemZero(prTxDesc, ucTxDescAndPaddingLength);

	/* WLAN index */
	prMsduInfo->ucWlanIndex =
		nicTxGetWlanIdx(prAdapter,
			prMsduInfo->ucBssIndex, prMsduInfo->ucStaRecIndex);

	/* UC to a connected peer */
	HAL_MAC_CONNAC3X_TXD_SET_MLD_ID(prTxDesc, prMsduInfo->ucWlanIndex);

	/* Tx byte count */
	HAL_MAC_CONNAC3X_TXD_SET_TX_BYTE_COUNT(prTxDesc,
		ucTxDescAndPaddingLength + prCmdInfo->u2InfoBufLen);

	/* Ether-type offset */
	ucEtherTypeOffsetInWord =
		((ETHER_HEADER_LEN - ETHER_TYPE_LEN) +
		prAdapter->chip_info->pse_header_length)
		>> 1;

	HAL_MAC_CONNAC3X_TXD_SET_ETHER_TYPE_OFFSET(prTxDesc,
		ucEtherTypeOffsetInWord);

	/* queue index */
	HAL_MAC_CONNAC3X_TXD_SET_QUEUE_INDEX(prTxDesc,
		nicTxGetTxDestQIdxByTc(ucTempTC));

	/* Header format */
	HAL_MAC_CONNAC3X_TXD_SET_HEADER_FORMAT(prTxDesc,
		HEADER_FORMAT_NON_802_11);

	/* Update Packet option */
	nic_txd_v3_fill_by_pkt_option(prMsduInfo, prTxDesc);

	if (!GLUE_TEST_PKT_FLAG(prNativePacket, ENUM_PKT_802_3)) {
		/* Set EthernetII */
		HAL_MAC_CONNAC3X_TXD_SET_ETHERNET_II(prTxDesc);
	}
	/* Header Padding */
	HAL_MAC_CONNAC3X_TXD_SET_HEADER_PADDING(prTxDesc,
		NIC_TX_DESC_HEADER_PADDING_LENGTH);

	/* TID mgmt type */
	HAL_MAC_CONNAC3X_TXD_SET_TID_MGMT_TYPE(prTxDesc, ucTid);

	/* Remaining TX time */
	HAL_MAC_CONNAC3X_TXD_SET_REMAINING_LIFE_TIME_IN_MS(prTxDesc,
		nicTxGetRemainingTxTimeByTc(ucTempTC));

	/* Tx count limit */
	HAL_MAC_CONNAC3X_TXD_SET_REMAINING_TX_COUNT(prTxDesc,
		nicTxGetTxCountLimitByTc(ucTempTC));

	HAL_MAC_CONNAC3X_TXD_SET_FIXED_RATE_ENABLE(prTxDesc);

	/* Packet Format */
	HAL_MAC_CONNAC3X_TXD_SET_PKT_FORMAT(prTxDesc, TXD_PKT_FORMAT_COMMAND);

	/* Own MAC */
	HAL_MAC_CONNAC3X_TXD_SET_OWN_MAC_INDEX(prTxDesc,
		prBssInfo->ucOwnMacIndex);

	/* PID */
	if (prMsduInfo->pfTxDoneHandler) {
		prMsduInfo->ucPID =
			nicTxAssignPID(prAdapter, prMsduInfo->ucWlanIndex);
		HAL_MAC_CONNAC3X_TXD_SET_PID(prTxDesc, prMsduInfo->ucPID);
		HAL_MAC_CONNAC3X_TXD_SET_TXS_TO_MCU(prTxDesc);
	}

	if (pucTxDescLength)
		*pucTxDescLength = ucTxDescAndPaddingLength;
}

void nic_txd_v3_set_pkt_fixed_rate_option_full(struct MSDU_INFO
	*prMsduInfo,
	uint16_t u2RateCode,
	uint8_t ucBandwidth,
	u_int8_t fgShortGI,
	u_int8_t fgLDPC,
	u_int8_t fgDynamicBwRts, u_int8_t fgBeamforming,
	uint8_t ucAntennaIndex)
{
#define UNUSED(x) ((void)(x))

	UNUSED(prMsduInfo);
	UNUSED(u2RateCode);
	UNUSED(ucBandwidth);
	UNUSED(fgShortGI);
	UNUSED(fgLDPC);
	UNUSED(fgDynamicBwRts);
	UNUSED(fgBeamforming);
	UNUSED(ucAntennaIndex);

	/* Fixed Rate */
	prMsduInfo->ucRateMode = MSDU_RATE_MODE_MANUAL_DESC;
}

void nic_txd_v3_set_pkt_fixed_rate_option(
	struct MSDU_INFO *prMsduInfo,
	uint16_t u2RateCode,
	uint8_t ucBandwidth,
	u_int8_t fgShortGI,
	u_int8_t fgDynamicBwRts)
{
#define UNUSED(x) ((void)(x))

	UNUSED(prMsduInfo);
	UNUSED(u2RateCode);
	UNUSED(ucBandwidth);
	UNUSED(fgShortGI);
	UNUSED(fgDynamicBwRts);

	/* Fixed Rate */
	prMsduInfo->ucRateMode = MSDU_RATE_MODE_MANUAL_DESC;
}

void nic_txd_v3_set_hw_amsdu_template(
	struct ADAPTER *prAdapter,
	struct STA_RECORD *prStaRec,
	u_int8_t ucTid,
	IN u_int8_t fgSet)
{
	struct HW_MAC_CONNAC3X_TX_DESC *prTxDesc;

	DBGLOG(QM, INFO,
		"Update HW Amsdu field of TXD template for STA[%u] Tid[%u]\n",
		prStaRec->ucIndex, ucTid);

	if (prStaRec->aprTxDescTemplate[ucTid]) {
		prTxDesc =
			(struct HW_MAC_CONNAC3X_TX_DESC *)
			prStaRec->aprTxDescTemplate[ucTid];
		if (fgSet)
			HAL_MAC_CONNAC3X_TXD_SET_HW_AMSDU(prTxDesc);
		else
			HAL_MAC_CONNAC3X_TXD_UNSET_HW_AMSDU(prTxDesc);
	}
}
#endif /* CFG_SUPPORT_CONNAC3X == 1 */
