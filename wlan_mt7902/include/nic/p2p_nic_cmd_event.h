/* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause */
/*
 * Copyright (c) 2016 MediaTek Inc.
 */
/*
 ** Id:
 * //Department/DaVinci/TRUNK/WiFi_P2P_Driver/include/nic/p2p_nic_cmd_event.h#1
 */

/*! \file   p2p_nic_cmd_event.h
 *  \brief
 */

#ifndef _P2P_NIC_CMD_EVENT_H
#define _P2P_NIC_CMD_EVENT_H

/******************************************************************************
 *                         C O M P I L E R   F L A G S
 ******************************************************************************
 */

/******************************************************************************
 *                    E X T E R N A L   R E F E R E N C E S
 ******************************************************************************
 */

/******************************************************************************
 *                              C O N S T A N T S
 ******************************************************************************
 */

/******************************************************************************
 *                                 M A C R O S
 ******************************************************************************
 */

/******************************************************************************
 *                             D A T A   T Y P E S
 ******************************************************************************
 */

/******************************************************************************
 *                            P U B L I C   D A T A
 ******************************************************************************
 */

/******************************************************************************
 *                           P R I V A T E   D A T A
 ******************************************************************************
 */

/******************************************************************************
 *                  F U N C T I O N   D E C L A R A T I O N S
 ******************************************************************************
 */

struct EVENT_P2P_DEV_DISCOVER_RESULT {
/* UINT_8 aucCommunicateAddr[MAC_ADDR_LEN];  // Deprecated. */
	uint8_t aucDeviceAddr[MAC_ADDR_LEN];	/* Device Address. */
	uint8_t aucInterfaceAddr[MAC_ADDR_LEN];	/* Device Address. */
	uint8_t ucDeviceCapabilityBitmap;
	uint8_t ucGroupCapabilityBitmap;
	uint16_t u2ConfigMethod;	/* Configure Method. */
	struct P2P_DEVICE_TYPE rPriDevType;
	uint8_t ucSecDevTypeNum;
	struct P2P_DEVICE_TYPE arSecDevType[2];
	uint16_t u2NameLength;
	uint8_t aucName[32];
	uint8_t *pucIeBuf;
	uint16_t u2IELength;
	uint8_t aucBSSID[MAC_ADDR_LEN];
	/* TODO: Service Information or PasswordID valid? */
};

#endif
