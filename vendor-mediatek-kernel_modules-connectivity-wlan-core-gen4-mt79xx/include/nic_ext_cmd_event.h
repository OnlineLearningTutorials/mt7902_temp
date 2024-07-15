/* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause */
/*
 * Copyright (c) 2016 MediaTek Inc.
 */
/*! \file   "nic_ext_cmd_event.h"
 *  \brief This file contains the declairation file of the WLAN OID processing
 *	 routines of Windows driver for MediaTek Inc.
 *   802.11 Wireless LAN Adapters.
 */

#ifndef _NIC_EXT_CMD_EVENT_H
#define _NIC_EXT_CMD_EVENT_H

/*******************************************************************************
 *                         C O M P I L E R   F L A G S
 *******************************************************************************
 */
#if (CFG_SUPPORT_CONNAC2X == 1 || CFG_SUPPORT_CONNAC3X == 1)

/*******************************************************************************
 *                    E X T E R N A L   R E F E R E N C E S
 *******************************************************************************
 */
#include "gl_typedef.h"

/*******************************************************************************
 *                              C O N S T A N T S
 *******************************************************************************
 */
#define STAREC_COMMON_EXTRAINFO_V2		BIT(0)
#define STAREC_COMMON_EXTRAINFO_NEWSTAREC	BIT(1)

/*******************************************************************************
 *                            P U B L I C   D A T A
 *******************************************************************************
 */

/*******************************************************************************
 *                           P R I V A T E   D A T A
 *******************************************************************************
 */
struct STAREC_COMMON_T {
	/* Basic STA record (Group0) */
	uint16_t u2Tag;		/* Tag = 0x00 */
	uint16_t u2Length;
	uint32_t u4ConnectionType;
	uint8_t	ucConnectionState;
	uint8_t	ucIsQBSS;
	uint16_t u2AID;
	uint8_t	aucPeerMacAddr[6];
	uint16_t u2ExtraInfo;
} __KAL_ATTRIB_PACKED__;

struct CMD_STAREC_UPDATE_T {
	uint8_t	ucBssIndex;
	uint8_t	ucWlanIdx;
	uint16_t u2TotalElementNum;
	uint8_t	ucAppendCmdTLV;
	uint8_t ucMuarIdx;
	uint8_t	aucReserve[2];
	uint8_t	aucBuffer[];
} __KAL_ATTRIB_PACKED__;

struct STAREC_HANDLE_T {
	uint32_t StaRecTag;
	uint32_t StaRecTagLen;
	int32_t (*StaRecTagHandler)(
		struct ADAPTER *pAd, uint8_t *pMsgBuf, void *args);
};

#if (CFG_SUPPORT_DMASHDL_SYSDVT)
struct EXT_CMD_CR4_DMASHDL_DVT_T {
	uint8_t ucItemNo;
	uint8_t ucSubItemNo;
	uint8_t ucReserve[2];
};
#endif /* CFG_SUPPORT_DMASHDL_SYSDVT */

struct CMD_BSSINFO_UPDATE_T {
	uint8_t	ucBssIndex;
	uint8_t	ucReserve;
	uint16_t	u2TotalElementNum;
	uint32_t u4Reserve;
	uint8_t aucBuffer[];
} __KAL_ATTRIB_PACKED__;


/* TAG ID 0x00: */
struct BSSINFO_CONNECT_OWN_DEV_T {
	/* BSS connect to own dev (Tag0) */
	uint16_t u2Tag;		/* Tag = 0x00 */
	uint16_t u2Length;
	uint8_t ucHwBSSIndex;
	uint8_t ucOwnMacIdx;
	uint8_t aucReserve[2];
	uint32_t ucConnectionType;
	uint32_t u4Reserved;
} __KAL_ATTRIB_PACKED__;

/* TAG ID 0x01: */
struct BSSINFO_BASIC_T {
	/* Basic BSS information (Tag1) */
	uint16_t u2Tag;		/* Tag = 0x01 */
	uint16_t u2Length;
	uint32_t u4NetworkType;
	uint8_t ucActive;
	uint8_t ucReserve0;
	uint16_t u2BcnInterval;
	uint8_t aucBSSID[6];
	uint8_t ucWmmIdx;
	uint8_t ucDtimPeriod;
/* indicate which wlan-idx used for MC/BC transmission */
	uint8_t ucBcMcWlanidx;
	uint8_t ucCipherSuit;
	uint8_t acuReserve[6];
} __KAL_ATTRIB_PACKED__;


struct CMD_DEVINFO_UPDATE_T {
	uint8_t ucOwnMacIdx;
	uint8_t ucReserve;
	uint16_t u2TotalElementNum;
	uint32_t aucReserve;
	uint8_t aucBuffer[];
} __KAL_ATTRIB_PACKED__;

struct CMD_DEVINFO_ACTIVE_T {
	uint16_t u2Tag;		/* Tag = 0x00 */
	uint16_t u2Length;
	uint8_t ucActive;
	uint8_t ucDbdcIdx;
	uint8_t aucOwnMacAddr[6];
	uint8_t aucReserve[4];
} __KAL_ATTRIB_PACKED__;


/*******************************************************************************
 *                                 M A C R O S
 *******************************************************************************
 */

/*******************************************************************************
 *                   F U N C T I O N   D E C L A R A T I O N S
 *******************************************************************************
 */

uint32_t CmdExtStaRecUpdate2WA(
	struct ADAPTER *pAd,
	struct STA_RECORD *pStaRecCfg);

#if (CFG_SUPPORT_DMASHDL_SYSDVT)
uint32_t CmdExtDmaShdlDvt2WA(
	struct ADAPTER *pAd,
	uint8_t ucItemNo,
	uint8_t ucSubItemNo);
#endif /* CFG_SUPPORT_DMASHDL_SYSDVT */

uint32_t CmdExtBssInfoUpdate2WA(
	struct ADAPTER *pAd,
	uint8_t ucBssIndex);

uint32_t CmdExtDevInfoUpdate2WA(
	struct ADAPTER *pAd,
	uint8_t ucBssIndex);

#endif /* CFG_SUPPORT_CONNAC2X == 1 */

#endif /* _NIC_EXT_CMD_EVENT_H */
