/* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause */
/*
 * Copyright (c) 2016 MediaTek Inc.
 */
/*! \file   cmm_asic_common.c
 *    \brief  Internal driver stack will export the required procedures
 *            here for GLUE Layer.
 *
 *    This file contains all routines which are exported from MediaTek 802.11
 *    Wireless LAN driver stack to GLUE Layer.
 */

/*******************************************************************************
 *                         C O M P I L E R   F L A G S
 *******************************************************************************
 */

/*******************************************************************************
 *                    E X T E R N A L   R E F E R E N C E S
 *******************************************************************************
 */
#include "precomp.h"
#include "mt_dmac.h"

/*******************************************************************************
 *                              C O N S T A N T S
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
 *                              F U N C T I O N S
 *******************************************************************************
 */
uint32_t asicGetFwDlInfo(struct ADAPTER *prAdapter,
			 char *pcBuf, int i4TotalLen)
{
	struct TAILER_COMMON_FORMAT_T *prComTailer;
	uint32_t u4Offset = 0;
	uint8_t aucBuf[32];

	prComTailer = &prAdapter->rVerInfo.rCommonTailer;

	kalMemZero(aucBuf, sizeof(aucBuf));
	kalMemCopy(aucBuf, prComTailer->aucRamVersion,
			sizeof(prComTailer->aucRamVersion));
	u4Offset += snprintf(pcBuf + u4Offset, i4TotalLen - u4Offset,
			     "Tailer Ver[%u:%u] %s (%s) info %u:E%u\n",
			     prComTailer->ucFormatVer,
			     prComTailer->ucFormatFlag,
			     aucBuf,
			     prComTailer->aucRamBuiltDate,
			     prComTailer->ucChipInfo,
			     prComTailer->ucEcoCode + 1);

	if (prComTailer->ucFormatFlag) {
		u4Offset += snprintf(pcBuf + u4Offset, i4TotalLen - u4Offset,
				     "Release manifest: %s\n",
				     prAdapter->rVerInfo.aucReleaseManifest);
	}
	return u4Offset;
}

uint32_t asicGetChipID(struct ADAPTER *prAdapter)
{
	struct mt66xx_chip_info *prChipInfo;
	uint32_t u4ChipID = 0;

	ASSERT(prAdapter);
	prChipInfo = prAdapter->chip_info;
	ASSERT(prChipInfo);

	/* Compose chipID from chip ip version
	 *
	 * BIT(30, 31) : Coding type, 00: compact, 01: index table
	 * BIT(24, 29) : IP config (6 bits)
	 * BIT(8, 23)  : IP version
	 * BIT(0, 7)   : A die info
	 */

	u4ChipID = (0x0 << 30) |
		   ((prChipInfo->u4ChipIpConfig & 0x3F) << 24) |
		   ((prChipInfo->u4ChipIpVersion & 0xF0000000) >>  8) |
		   ((prChipInfo->u4ChipIpVersion & 0x000F0000) >>  0) |
		   ((prChipInfo->u4ChipIpVersion & 0x00000F00) <<  4) |
		   ((prChipInfo->u4ChipIpVersion & 0x0000000F) <<  8) |
		   (prChipInfo->u2ADieChipVersion & 0xFF);

	log_dbg(HAL, INFO, "ChipID = [0x%08x]\n", u4ChipID);
	return u4ChipID;
}

void fillNicTxDescAppend(IN struct ADAPTER *prAdapter,
			 IN struct MSDU_INFO *prMsduInfo,
			 OUT uint8_t *prTxDescBuffer)
{
	struct mt66xx_chip_info *prChipInfo = prAdapter->chip_info;
	union HW_MAC_TX_DESC_APPEND *prHwTxDescAppend;

	/* Fill TxD append */
	prHwTxDescAppend = (union HW_MAC_TX_DESC_APPEND *)
			   prTxDescBuffer;
	kalMemZero(prHwTxDescAppend, prChipInfo->txd_append_size);
}

void fillTxDescAppendByHostV2(IN struct ADAPTER *prAdapter,
	IN struct MSDU_INFO *prMsduInfo, IN uint16_t u4MsduId,
	IN dma_addr_t rDmaAddr, IN uint32_t u4Idx,
	IN u_int8_t fgIsLast,
	OUT uint8_t *pucBuffer)
{
	union HW_MAC_TX_DESC_APPEND *prHwTxDescAppend;
	struct TXD_PTR_LEN *prPtrLen;
	uint64_t u8Addr = (uint64_t)rDmaAddr;

	prHwTxDescAppend = (union HW_MAC_TX_DESC_APPEND *)
		(pucBuffer + NIC_TX_DESC_LONG_FORMAT_LENGTH);
	prHwTxDescAppend->CONNAC_APPEND.au2MsduId[u4Idx] =
		u4MsduId | TXD_MSDU_ID_VLD;
	prPtrLen = &prHwTxDescAppend->CONNAC_APPEND.arPtrLen[u4Idx >> 1];

	if ((u4Idx & 1) == 0) {
		prPtrLen->u4Ptr0 = (uint32_t)u8Addr;
		prPtrLen->u2Len0 =
			(prMsduInfo->u2FrameLength & TXD_LEN_MASK_V2) |
			((u8Addr >> TXD_ADDR2_OFFSET) & TXD_ADDR2_MASK);
		prPtrLen->u2Len0 |= TXD_LEN_ML_V2;
	} else {
		prPtrLen->u4Ptr1 = (uint32_t)u8Addr;
		prPtrLen->u2Len1 =
			(prMsduInfo->u2FrameLength & TXD_LEN_MASK_V2) |
			((u8Addr >> TXD_ADDR2_OFFSET) & TXD_ADDR2_MASK);
		prPtrLen->u2Len1 |= TXD_LEN_ML_V2;
	}
}

static char *q_idx_mcu_str[] = {"RQ0", "RQ1", "RQ2", "RQ3", "Invalid"};
static char *pkt_ft_str[] = {"cut_through", "store_forward",
	"cmd", "PDA_FW_Download"};
static char *hdr_fmt_str[] = {
	"Non-80211-Frame",
	"Command-Frame",
	"Normal-80211-Frame",
	"enhanced-80211-Frame",
};
static char *p_idx_str[] = {"LMAC", "MCU"};
static char *q_idx_lmac_str[] = {"WMM0_AC0", "WMM0_AC1", "WMM0_AC2", "WMM0_AC3",
	"WMM1_AC0", "WMM1_AC1", "WMM1_AC2", "WMM1_AC3",
	"WMM2_AC0", "WMM2_AC1", "WMM2_AC2", "WMM2_AC3",
	"WMM3_AC0", "WMM3_AC1", "WMM3_AC2", "WMM3_AC3",
	"Band0_ALTX", "Band0_BMC", "Band0_BNC", "Band0_PSMP",
	"Band1_ALTX", "Band1_BMC", "Band1_BNC", "Band1_PSMP",
	"Invalid"};

void halDumpTxdInfo(IN struct ADAPTER *prAdapter, uint32_t *tmac_info)
{
	struct TMAC_TXD_S *txd_s;
	struct TMAC_TXD_0 *txd_0;
	struct TMAC_TXD_1 *txd_1;
	uint8_t q_idx = 0;

	txd_s = (struct TMAC_TXD_S *)tmac_info;
	txd_0 = &txd_s->TxD0;
	txd_1 = &txd_s->TxD1;

	DBGLOG(HAL, INFO, "TMAC_TXD Fields:\n");
	DBGLOG(HAL, INFO, "\tTMAC_TXD_0:\n");
	DBGLOG(HAL, INFO, "\t\tPortID=%d(%s)\n",
			txd_0->p_idx, p_idx_str[txd_0->p_idx]);

	if (txd_0->p_idx == P_IDX_LMAC)
		q_idx = txd_0->q_idx % 0x18;
	else
		q_idx = ((txd_0->q_idx == TxQ_IDX_MCU_PDA) ?
			txd_0->q_idx : (txd_0->q_idx % 0x4));

	DBGLOG(HAL, INFO, "\t\tQueID=0x%x(%s %s)\n", txd_0->q_idx,
			 (txd_0->p_idx == P_IDX_LMAC ? "LMAC" : "MCU"),
			 txd_0->p_idx == P_IDX_LMAC ?
				q_idx_lmac_str[q_idx] : q_idx_mcu_str[q_idx]);
	DBGLOG(HAL, INFO, "\t\tTxByteCnt=%d\n", txd_0->TxByteCount);
	DBGLOG(HAL, INFO, "\t\tIpChkSumOffload=%d\n", txd_0->IpChkSumOffload);
	DBGLOG(HAL, INFO, "\t\tUdpTcpChkSumOffload=%d\n",
						txd_0->UdpTcpChkSumOffload);
	DBGLOG(HAL, INFO, "\t\tEthTypeOffset=%d\n", txd_0->EthTypeOffset);

	DBGLOG(HAL, INFO, "\tTMAC_TXD_1:\n");
	DBGLOG(HAL, INFO, "\t\twlan_idx=%d\n", txd_1->wlan_idx);
	DBGLOG(HAL, INFO, "\t\tHdrFmt=%d(%s)\n",
			 txd_1->hdr_format, hdr_fmt_str[txd_1->hdr_format]);
	DBGLOG(HAL, INFO, "\t\tHdrInfo=0x%x\n", txd_1->hdr_info);

	switch (txd_1->hdr_format) {
	case TMI_HDR_FT_NON_80211:
		DBGLOG(HAL, INFO,
			"\t\t\tMRD=%d, EOSP=%d, RMVL=%d, VLAN=%d, ETYP=%d\n",
			txd_1->hdr_info & (1 << TMI_HDR_INFO_0_BIT_MRD),
			txd_1->hdr_info & (1 << TMI_HDR_INFO_0_BIT_EOSP),
			txd_1->hdr_info & (1 << TMI_HDR_INFO_0_BIT_RMVL),
			txd_1->hdr_info & (1 << TMI_HDR_INFO_0_BIT_VLAN),
			txd_1->hdr_info & (1 << TMI_HDR_INFO_0_BIT_ETYP));
		break;

	case TMI_HDR_FT_CMD:
		DBGLOG(HAL, INFO, "\t\t\tRsvd=0x%x\n", txd_1->hdr_info);
		break;

	case TMI_HDR_FT_NOR_80211:
		DBGLOG(HAL, INFO, "\t\t\tHeader Len=%d(WORD)\n",
				 txd_1->hdr_info & TMI_HDR_INFO_2_MASK_LEN);
		break;

	case TMI_HDR_FT_ENH_80211:
		DBGLOG(HAL, INFO, "\t\t\tEOSP=%d, AMS=%d\n",
			txd_1->hdr_info & (1 << TMI_HDR_INFO_3_BIT_EOSP),
			txd_1->hdr_info & (1 << TMI_HDR_INFO_3_BIT_AMS));
		break;
	}

	DBGLOG(HAL, INFO, "\t\tTxDFormatType=%d(%s format)\n", txd_1->ft,
		(txd_1->ft == TMI_FT_LONG ?
		"Long - 8 DWORD" : "Short - 3 DWORD"));
	DBGLOG(HAL, INFO, "\t\ttxd_len=%d page(%d DW)\n",
		txd_1->txd_len == 0 ? 1 : 2, (txd_1->txd_len + 1) * 16);
	DBGLOG(HAL, INFO,
		"\t\tHdrPad=%d(Padding Mode: %s, padding bytes: %d)\n",
		txd_1->hdr_pad,
		((txd_1->hdr_pad & (TMI_HDR_PAD_MODE_TAIL << 1)) ?
		"tail" : "head"), (txd_1->hdr_pad & 0x1 ? 2 : 0));
	DBGLOG(HAL, INFO, "\t\tUNxV=%d\n", txd_1->UNxV);
	DBGLOG(HAL, INFO, "\t\tamsdu=%d\n", txd_1->amsdu);
	DBGLOG(HAL, INFO, "\t\tTID=%d\n", txd_1->tid);
	DBGLOG(HAL, INFO, "\t\tpkt_ft=%d(%s)\n",
			 txd_1->pkt_ft, pkt_ft_str[txd_1->pkt_ft]);
	DBGLOG(HAL, INFO, "\t\town_mac=%d\n", txd_1->OwnMacAddr);

	if (txd_s->TxD1.ft == TMI_FT_LONG) {
		struct TMAC_TXD_L *txd_l = (struct TMAC_TXD_L *)tmac_info;
		struct TMAC_TXD_2 *txd_2 = &txd_l->TxD2;
		struct TMAC_TXD_3 *txd_3 = &txd_l->TxD3;
		struct TMAC_TXD_4 *txd_4 = &txd_l->TxD4;
		struct TMAC_TXD_5 *txd_5 = &txd_l->TxD5;
		struct TMAC_TXD_6 *txd_6 = &txd_l->TxD6;

		DBGLOG(HAL, INFO, "\tTMAC_TXD_2:\n");
		DBGLOG(HAL, INFO, "\t\tsub_type=%d\n", txd_2->sub_type);
		DBGLOG(HAL, INFO, "\t\tfrm_type=%d\n", txd_2->frm_type);
		DBGLOG(HAL, INFO, "\t\tNDP=%d\n", txd_2->ndp);
		DBGLOG(HAL, INFO, "\t\tNDPA=%d\n", txd_2->ndpa);
		DBGLOG(HAL, INFO, "\t\tSounding=%d\n", txd_2->sounding);
		DBGLOG(HAL, INFO, "\t\tRTS=%d\n", txd_2->rts);
		DBGLOG(HAL, INFO, "\t\tbc_mc_pkt=%d\n", txd_2->bc_mc_pkt);
		DBGLOG(HAL, INFO, "\t\tBIP=%d\n", txd_2->bip);
		DBGLOG(HAL, INFO, "\t\tDuration=%d\n", txd_2->duration);
		DBGLOG(HAL, INFO, "\t\tHE(HTC Exist)=%d\n", txd_2->htc_vld);
		DBGLOG(HAL, INFO, "\t\tFRAG=%d\n", txd_2->frag);
		DBGLOG(HAL, INFO, "\t\tReamingLife/MaxTx time=%d\n",
			txd_2->max_tx_time);
		DBGLOG(HAL, INFO, "\t\tpwr_offset=%d\n", txd_2->pwr_offset);
		DBGLOG(HAL, INFO, "\t\tba_disable=%d\n", txd_2->ba_disable);
		DBGLOG(HAL, INFO, "\t\ttiming_measure=%d\n",
			txd_2->timing_measure);
		DBGLOG(HAL, INFO, "\t\tfix_rate=%d\n", txd_2->fix_rate);
		DBGLOG(HAL, INFO, "\tTMAC_TXD_3:\n");
		DBGLOG(HAL, INFO, "\t\tNoAck=%d\n", txd_3->no_ack);
		DBGLOG(HAL, INFO, "\t\tPF=%d\n", txd_3->protect_frm);
		DBGLOG(HAL, INFO, "\t\ttx_cnt=%d\n", txd_3->tx_cnt);
		DBGLOG(HAL, INFO, "\t\tremain_tx_cnt=%d\n",
			txd_3->remain_tx_cnt);
		DBGLOG(HAL, INFO, "\t\tsn=%d\n", txd_3->sn);
		DBGLOG(HAL, INFO, "\t\tpn_vld=%d\n", txd_3->pn_vld);
		DBGLOG(HAL, INFO, "\t\tsn_vld=%d\n", txd_3->sn_vld);
		DBGLOG(HAL, INFO, "\tTMAC_TXD_4:\n");
		DBGLOG(HAL, INFO, "\t\tpn_low=0x%x\n", txd_4->pn_low);
		DBGLOG(HAL, INFO, "\tTMAC_TXD_5:\n");
		DBGLOG(HAL, INFO, "\t\ttx_status_2_host=%d\n",
			txd_5->tx_status_2_host);
		DBGLOG(HAL, INFO, "\t\ttx_status_2_mcu=%d\n",
			txd_5->tx_status_2_mcu);
		DBGLOG(HAL, INFO, "\t\ttx_status_fmt=%d\n",
			txd_5->tx_status_fmt);

		if (txd_5->tx_status_2_host || txd_5->tx_status_2_mcu)
			DBGLOG(HAL, INFO, "\t\tpid=%d\n", txd_5->pid);

		if (txd_2->fix_rate)
			DBGLOG(HAL, INFO,
				"\t\tda_select=%d\n", txd_5->da_select);

		DBGLOG(HAL, INFO, "\t\tpwr_mgmt=0x%x\n", txd_5->pwr_mgmt);
		DBGLOG(HAL, INFO, "\t\tpn_high=0x%x\n", txd_5->pn_high);

		if (txd_2->fix_rate) {
			DBGLOG(HAL, INFO, "\tTMAC_TXD_6:\n");
			DBGLOG(HAL, INFO, "\t\tfix_rate_mode=%d\n",
				txd_6->fix_rate_mode);
			DBGLOG(HAL, INFO, "\t\tGI=%d(%s)\n", txd_6->gi,
				(txd_6->gi == 0 ? "LONG" : "SHORT"));
			DBGLOG(HAL, INFO, "\t\tldpc=%d(%s)\n", txd_6->ldpc,
				(txd_6->ldpc == 0 ? "BCC" : "LDPC"));
			DBGLOG(HAL, INFO, "\t\tTxBF=%d\n", txd_6->TxBF);
			DBGLOG(HAL, INFO, "\t\ttx_rate=0x%x\n", txd_6->tx_rate);
			DBGLOG(HAL, INFO, "\t\tant_id=%d\n", txd_6->ant_id);
			DBGLOG(HAL, INFO, "\t\tdyn_bw=%d\n", txd_6->dyn_bw);
			DBGLOG(HAL, INFO, "\t\tbw=%d\n", txd_6->bw);
		}
	}
}

#if defined(_HIF_PCIE) || defined(_HIF_AXI)
void asicWakeUpWiFi(IN struct ADAPTER *prAdapter)
{
	u_int8_t fgResult;

	ASSERT(prAdapter);

	HAL_LP_OWN_RD(prAdapter, &fgResult);

	if (fgResult) {
		prAdapter->fgIsFwOwn = FALSE;
		DBGLOG(HAL, WARN,
			"Already DriverOwn, set flag only\n");
	} else
		HAL_LP_OWN_CLR(prAdapter, &fgResult);
}
#endif /* _HIF_PCIE || _HIF_AXI */

