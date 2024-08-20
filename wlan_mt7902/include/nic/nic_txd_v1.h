/* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause */
/*
 * Copyright (c) 2016 MediaTek Inc.
 */
/*
 ** Id: //Department/DaVinci/BRANCHES/
 *			MT6620_WIFI_DRIVER_V2_3/include/nic/nic_tx.h#1
 */

/*! \file   nic_tx.h
 *    \brief  Functions that provide TX operation in NIC's point of view.
 *
 *    This file provides TX functions which are responsible for both Hardware
 *    and Software Resource Management and keep their Synchronization.
 *
 */


#ifndef _NIC_TXD_V1_H
#define _NIC_TXD_V1_H

/*******************************************************************************
 *                         C O M P I L E R   F L A G S
 *******************************************************************************
 */

/*******************************************************************************
 *                    E X T E R N A L   R E F E R E N C E S
 *******************************************************************************
 */

uint8_t nic_txd_v1_long_format_op(
	void *prTxDesc,
	uint8_t fgSet);
uint8_t nic_txd_v1_tid_op(
	void *prTxDesc,
	uint8_t ucTid,
	uint8_t fgSet);
uint8_t nic_txd_v1_queue_idx_op(
	void *prTxDesc,
	uint8_t ucQueIdx,
	uint8_t fgSet);
#if (CFG_TCP_IP_CHKSUM_OFFLOAD == 1)
void nic_txd_v1_chksum_op(
	void *prTxDesc,
	uint8_t ucChksumFlag);
#endif /* CFG_TCP_IP_CHKSUM_OFFLOAD == 1 */
void nic_txd_v1_header_format_op(
	void *prTxDesc,
	struct MSDU_INFO *prMsduInfo);

void nic_txd_v1_fill_by_pkt_option(
	struct MSDU_INFO *prMsduInfo,
	void *prTxD);

void nic_txd_v1_compose(
	struct ADAPTER *prAdapter,
	struct MSDU_INFO *prMsduInfo,
	u_int32_t u4TxDescLength,
	u_int8_t fgIsTemplate,
	u_int8_t *prTxDescBuffer);
void nic_txd_v1_compose_security_frame(
	struct ADAPTER *prAdapter,
	struct CMD_INFO *prCmdInfo,
	uint8_t *prTxDescBuffer,
	uint8_t *pucTxDescLength);
void nic_txd_v1_set_pkt_fixed_rate_option_full(
	struct MSDU_INFO *prMsduInfo,
	uint16_t u2RateCode,
	uint8_t ucBandwidth,
	u_int8_t fgShortGI,
	u_int8_t fgLDPC,
	u_int8_t fgDynamicBwRts,
	u_int8_t fgBeamforming,
	uint8_t ucAntennaIndex);
void nic_txd_v1_set_pkt_fixed_rate_option(
	struct MSDU_INFO *prMsduInfo,
	uint16_t u2RateCode,
	uint8_t ucBandwidth,
	u_int8_t fgShortGI,
	u_int8_t fgDynamicBwRts);
void nic_txd_v1_set_hw_amsdu_template(
	struct ADAPTER *prAdapter,
	struct STA_RECORD *prStaRec,
	uint8_t ucTid,
	u_int8_t fgSet);
void nic_txd_v1_change_data_port_by_ac(
	struct STA_RECORD *prStaRec,
	uint8_t ucAci,
	u_int8_t fgToMcu);
#endif /* _NIC_TXD_V1_H */
