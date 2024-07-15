/* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause */
/*
 * Copyright (c) 2016 MediaTek Inc.
 */
/*
 ** Id: //Department/DaVinci/BRANCHES/
 *			MT6620_WIFI_DRIVER_v3_3/include/nic/nic_tx.h#1
 */

/*! \file   nic_tx.h
 *    \brief  Functions that provide TX operation in NIC's point of view.
 *
 *    This file provides TX functions which are responsible for both Hardware
 *    and Software Resource Management and keep their Synchronization.
 *
 */


#ifndef _NIC_RXD_v3_H
#define _NIC_RXD_v3_H

#if (CFG_SUPPORT_CONNAC3X == 1)
/*******************************************************************************
 *                         C O M P I L E R   F L A G S
 *******************************************************************************
 */

/*******************************************************************************
 *                    E X T E R N A L   R E F E R E N C E S
 *******************************************************************************
 */

struct HW_MAC_RX_STS_GROUP_1 {
	uint8_t aucPN[16];		/* DW12 - DW15 */
};

struct HW_MAC_RX_STS_GROUP_2 {
	uint32_t u4Timestamp;		/* DW16 */
	uint32_t u4CRC;			/* DW17 */
	uint32_t aucReserved[2];	/* DW18 - DW19 */
};

struct HW_MAC_RX_STS_GROUP_4 {
	/* For HDR_TRAN */
	uint16_t u2FrameCtl;		/* DW8 */
	uint8_t aucTA[6];		/* DW8 - DW9 */
	uint16_t u2SeqFrag;		/* DW 6 */
	uint16_t u2Qos;			/* DW 6 */
	uint32_t u4HTC;			/* DW 7 */
};

struct HW_MAC_RX_STS_GROUP_3 {
	/*!  RX Vector Info */
	uint32_t u4RxVector[6];	/* DW 14~19 */
};

struct HW_MAC_RX_STS_GROUP_3_V2 {
	/*  PRXVector Info */
	uint32_t u4RxVector[2];		/* DW20 - DW21 */
	uint16_t u2RxInfo;		/* DW22 */
	uint16_t u2Reserved;		/* DW22 */
	uint32_t u4Rcpi;		/* DW23 */
};

struct HW_MAC_RX_STS_GROUP_5 {
	uint32_t u4RxVector[24];
};

struct tx_free_done_rpt {
	uint32_t dw0;
	uint32_t dw1;
	void *arIdSets[0];
};

uint16_t nic_rxd_v3_get_rx_byte_count(
	void *prRxStatus);
uint8_t nic_rxd_v3_get_packet_type(
	void *prRxStatus);
uint8_t nic_rxd_v3_get_wlan_idx(
	void *prRxStatus);
uint8_t nic_rxd_v3_get_sec_mode(
	void *prRxStatus);
uint8_t nic_rxd_v3_get_sw_class_error_bit(
	void *prRxStatus);
uint8_t nic_rxd_v3_get_ch_num(
	void *prRxStatus);
uint8_t nic_rxd_v3_get_rf_band(
	void *prRxStatus);
uint8_t nic_rxd_v3_get_tcl(
	void *prRxStatus);
uint8_t nic_rxd_v3_get_ofld(
	void *prRxStatus);
void nic_rxd_v3_fill_rfb(
	struct ADAPTER *prAdapter,
	struct SW_RFB *prSwRfb);
u_int8_t nic_rxd_v3_sanity_check(
	struct ADAPTER *prAdapter,
	struct SW_RFB *prSwRfb);

uint8_t nic_rxd_v3_get_HdrTrans(
	void *prRxStatus);

#if CFG_SUPPORT_WAKEUP_REASON_DEBUG
void nic_rxd_v3_check_wakeup_reason(
	struct ADAPTER *prAdapter,
	struct SW_RFB *prSwRfb);
#endif /* CFG_SUPPORT_WAKEUP_REASON_DEBUG */
void nic_rxd_v3_handle_host_rpt(struct ADAPTER *prAdapter,
	struct SW_RFB *prSwRfb,
	struct QUE *prFreeQueue);

#if (CFG_SUPPORT_SNIFFER_RADIOTAP == 1)
uint8_t nic_rxd_v3_fill_radiotap(
	struct ADAPTER *prAdapter,
	struct SW_RFB *prSwRfb);
#endif

#endif /* CFG_SUPPORT_CONNAC3X == 1 */
#endif /* _NIC_RXD_v3_H */
