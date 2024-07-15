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


#ifndef _NIC_RXD_V2_H
#define _NIC_RXD_V2_H

#if (CFG_SUPPORT_CONNAC2X == 1)
/*******************************************************************************
 *                         C O M P I L E R   F L A G S
 *******************************************************************************
 */

/*******************************************************************************
 *                    E X T E R N A L   R E F E R E N C E S
 *******************************************************************************
 */

uint16_t nic_rxd_v2_get_rx_byte_count(
	void *prRxStatus);
uint8_t nic_rxd_v2_get_packet_type(
	void *prRxStatus);
uint8_t nic_rxd_v2_get_wlan_idx(
	void *prRxStatus);
uint8_t nic_rxd_v2_get_sec_mode(
	void *prRxStatus);
uint8_t nic_rxd_v2_get_sw_class_error_bit(
	void *prRxStatus);
uint8_t nic_rxd_v2_get_ch_num(
	void *prRxStatus);
uint8_t nic_rxd_v2_get_rf_band(
	void *prRxStatus);
uint8_t nic_rxd_v2_get_tcl(
	void *prRxStatus);
uint8_t nic_rxd_v2_get_ofld(
	void *prRxStatus);
void nic_rxd_v2_fill_rfb(
	struct ADAPTER *prAdapter,
	struct SW_RFB *prSwRfb);
u_int8_t nic_rxd_v2_sanity_check(
	struct ADAPTER *prAdapter,
	struct SW_RFB *prSwRfb);

uint8_t nic_rxd_v2_get_HdrTrans(
	void *prRxStatus);

#if CFG_SUPPORT_WAKEUP_REASON_DEBUG
void nic_rxd_v2_check_wakeup_reason(
	struct ADAPTER *prAdapter,
	struct SW_RFB *prSwRfb);
#endif /* CFG_SUPPORT_WAKEUP_REASON_DEBUG */

#if (CFG_SUPPORT_SNIFFER_RADIOTAP == 1)
uint8_t nic_rxd_v2_fill_radiotap(
	struct ADAPTER *prAdapter,
	struct SW_RFB *prSwRfb);
#endif

#endif /* CFG_SUPPORT_CONNAC2X == 1 */
#endif /* _NIC_RXD_V2_H */
