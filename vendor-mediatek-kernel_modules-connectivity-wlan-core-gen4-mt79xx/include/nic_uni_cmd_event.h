/* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause */
/*
 * Copyright (c) 2016 MediaTek Inc.
 */
/*
 *
 */

/*! \file   "nic_uni_cmd_event.h"
 *  \brief This file contains the declairation file of the WLAN OID processing
 *	 routines of Windows driver for MediaTek Inc.
 *   802.11 Wireless LAN Adapters.
 */

#ifndef _NIC_UNI_CMD_EVENT_H
#define _NIC_UNI_CMD_EVENT_H

/*******************************************************************************
 *                         C O M P I L E R   F L A G S
 *******************************************************************************
 */

/*******************************************************************************
 *                    E X T E R N A L   R E F E R E N C E S
 *******************************************************************************
 */
#include "gl_vendor.h"
#include "wsys_cmd_handler_fw.h"

/*******************************************************************************
 *                              C O N S T A N T S
 *******************************************************************************
 */

#define CMD_FLAG_UNI_CMD_MASK                   (0x40)
#define CMD_FLAG_UNI_CMD_OFFSET                 (6)

#define UNI_CMD_OPT_BIT_0_ACK        BIT(0) /* for SET command */
#define UNI_CMD_OPT_BIT_1_UNI_CMD    BIT(1) /* 1: unified cmd, 0:original cmd */
#define UNI_CMD_OPT_BIT_2_SET_QUERY  BIT(2) /* 1: set, 0:query */
#define UNI_CMD_OPT_BIT_1_UNI_EVENT  BIT(1)
#define UNI_CMD_OPT_BIT_2_UNSOLICIT_EVENT  BIT(2)


#define UNI_CMD_MSG_HEADER_LEN (sizeof(WIFI_UNI_CMD))
#define UNI_CMD_MAX_INBAND_CMD_LEN (1600 - UNI_CMD_MSG_HEADER_LEN)

#define MAX_UNI_EVENT_FAIL_TAG_COUNT 10

/* Customer Config Cmd for Gen4m */
#define UNICMD_CHIP_CONFIG_RESP_SIZE       320

/* TODO: uni cmd */
#define MLD_LINK_MAX 3
#define MLD_GROUP_NONE 0xff
#define OM_REMAP_IDX_NONE 0xff

/* UNI_CMD_SUSPEND_WOW_WAKEUP_PORT */
#define UNI_CMD_MAX_TCP_UDP_PORT 10

/* UNI_CMD_TWT_ARGT_UPDATE */
#define UNI_TWT_GRP_MAX_MEMBER_CNT  8

/* UNI_CMD_EDCA_AC_PARM */
#define UNI_CMD_EDCA_AIFS_BIT		(1 << 0)
#define UNI_CMD_EDCA_WIN_MIN_BIT	(1 << 1)
#define UNI_CMD_EDCA_WIN_MAX_BIT	(1 << 2)
#define UNI_CMD_EDCA_TXOP_BIT		(1 << 3)
#define UNI_CMD_EDCA_ALL_BITS	\
	(UNI_CMD_EDCA_AIFS_BIT | UNI_CMD_EDCA_WIN_MIN_BIT | \
	 UNI_CMD_EDCA_WIN_MAX_BIT | UNI_CMD_EDCA_TXOP_BIT)

/* UNI_EVENT_UPDATE_COEX_PHYRATE */
#define UNI_BSS_INFO_NUM 5

/* UNI_EVENT_MD_SAFE_CHN */
#define UNI_WIFI_CH_MASK_IDX_NUM 4

/*******************************************************************************
 *                             D A T A   T Y P E S
 *******************************************************************************
 */

/*******************************************************************************
 *                                 COMMAND
 *******************************************************************************
 */

struct WIFI_UNI_CMD_INFO {
	uint16_t u2InfoBufLen;
	uint8_t *pucInfoBuffer;
	uint16_t u2CID;
	uint8_t  ucPktTypeID;
	uint8_t  ucS2DIndex;
	uint8_t  ucOption;
};

struct WIFI_UNI_CMD {
	uint16_t u2TxByteCount;	/* Max value is over 2048 */
	uint16_t u2PQ_ID;	/* Must be 0x8000 (Port1, Queue 0) */

	uint8_t ucWlanIdx;
	uint8_t ucHeaderFormat;
	uint8_t ucHeaderPadding;
	uint8_t ucPktFt: 2;
	uint8_t ucOwnMAC: 6;
	uint32_t au4Reserved1[6];

	uint16_t u2Length;
	uint16_t u2CID;

	uint8_t aucReserved[1];
	uint8_t ucPktTypeID;	/* Must be 0xA0 (CMD Packet) */
	uint8_t ucFragNum;
	uint8_t ucSeqNum;

	uint16_t u2Checksum;
	uint8_t ucS2DIndex;	/* Index for Src to Dst in CMD usage */
	uint8_t ucOption;	/* CID option */

	uint8_t aucReserved2[4];
	uint8_t aucBuffer[0];
};

struct WIFI_UNI_SETQUERY_INFO {
	/* legacy cmd*/
	IN uint8_t ucCID;
	IN uint8_t ucExtCID;
	IN uint8_t fgSetQuery;
	IN uint8_t fgNeedResp;
	IN uint8_t fgIsOid;
	IN PFN_CMD_DONE_HANDLER pfCmdDoneHandler;
	IN PFN_CMD_TIMEOUT_HANDLER pfCmdTimeoutHandler;
	IN uint32_t u4SetQueryInfoLen;
	IN uint8_t *pucInfoBuffer;
	IN void *pvSetQueryBuffer;
	IN uint32_t u4SetQueryBufferLen;

	/* uni cmds */
	OUT struct LINK rUniCmdList;
};

struct WIFI_UNI_CMD_ENTRY {
	struct LINK_ENTRY rLinkEntry;

	uint8_t ucUCID;
	PFN_CMD_DONE_HANDLER pfCmdDoneHandler;
	PFN_CMD_TIMEOUT_HANDLER pfCmdTimeoutHandler;
	uint32_t u4SetQueryInfoLen;
	uint8_t *pucInfoBuffer;
};

enum ENUM_UNI_CMD_ID {
	UNI_CMD_ID_DEVINFO		= 0x01, /* Update DEVINFO */
	UNI_CMD_ID_BSSINFO		= 0x02, /* Update BSSINFO */
	UNI_CMD_ID_STAREC_INFO		= 0x03, /* Update STAREC */
	UNI_CMD_ID_EDCA_SET		= 0x04, /* Update EDCA Set */
	UNI_CMD_ID_SUSPEND		= 0x05, /* Suspend */
	UNI_CMD_ID_OFFLOAD		= 0x06, /* Offload feature config */
	UNI_CMD_ID_HIF_CTRL		= 0x07, /* hif control */
	UNI_CMD_ID_BAND_CONFIG		= 0x08, /* Band Config */
	UNI_CMD_ID_REPT_MUAR		= 0x09, /* Repeater Mode Muar Config */
	UNI_CMD_ID_NORM_MUAR		= 0x0A, /* Normal Mode Muar Config */
	UNI_CMD_ID_WSYS_CONFIG		= 0x0B, /* WSYS Configuration */
	UNI_CMD_ID_ROAMING		= 0x0C, /* Roaming FSM switch */
	UNI_CMD_ID_ACCESS_REG		= 0x0D, /* Access Register */
	UNI_CMD_ID_CHIP_CONFIG		= 0x0E, /* Custom Chip Configuration*/
	UNI_CMD_ID_POWER_CTRL		= 0x0F, /* NIC Power control */
	UNI_CMD_ID_CFG_SMESH		= 0x10, /* Smesh Config */
	UNI_CMD_ID_RRM_11K		= 0x11, /* 802.11K RRM*/
	UNI_CMD_ID_RX_HDR_TRAN		= 0x12, /* Rx header translation */
	UNI_CMD_ID_SER			= 0x13, /* SER */
	UNI_CMD_ID_TWT			= 0x14, /* 80211AX TWT*/
	UNI_CMD_ID_SET_DOMAIN_INFO	= 0x15, /* Set Domain info */
	UNI_CMD_ID_IDC			= 0x17, /* IDC */
	UNI_CMD_ID_SCAN_REQ		= 0x16, /* Scan Request */
	UNI_CMD_ID_ECC_OPER		= 0x18, /* ECC Operation */
	UNI_CMD_ID_RDD_ON_OFF_CTRL	= 0x19, /* RDD On/Off Control */
	UNI_CMD_ID_GET_MAC_INFO	= 0x1A, /* Get MAC info */
	UNI_CMD_ID_TDLS		= 0x1B, /* TDLS */
	UNI_CMD_ID_TXCMD_CTRL		= 0x1D, /* Txcmd ctrl */
	UNI_CMD_ID_SET_DROP_PACKET_CFG	= 0x1E,  /* Set Packet Drop cfg */
	UNI_CMD_ID_BA_OFFLOAD		= 0x1F, /* BA Offload */
	UNI_CMD_ID_P2P			= 0x20, /* P2P */
	UNI_CMD_ID_SMART_GEAR		= 0x21, /* Smart Gear */
	UNI_CMD_ID_MIB			= 0x22, /* Get MIB counter */
	UNI_CMD_ID_GET_STATISTICS	= 0x23, /* Get Statistics */
	UNI_CMD_ID_SNIFFER_MODE	= 0x24, /* Sniffer Mode */
	UNI_CMD_ID_SR			= 0x25, /* SR */
	UNI_CMD_ID_SCS			= 0x26, /* SCS */
	UNI_CMD_ID_CNM			= 0x27, /*CNM*/
	UNI_CMD_ID_MBMC		= 0x28, /*MBMC*/
	UNI_CMD_ID_DVT			= 0x29, /* DVT */
	UNI_CMD_ID_GPIO		= 0x2A, /* GPIO setting*/
	UNI_CMD_ID_TXPOWER		= 0x2B, /* RLM Tx Power */
	UNI_CMD_ID_POWER_LIMIT		= 0x2C, /* Tx Power Limit*/
	UNI_CMD_ID_RA			= 0x2F, /* RA */
	UNI_CMD_ID_MURU		= 0x31, /* MURU */
	UNI_CMD_ID_TESTMODE_RX_STAT	= 0x32, /* testmode Rx statistic */
	UNI_CMD_ID_BF			= 0x33, /* BF */
	UNI_CMD_ID_VOW			= 0x37, /* VOW */
	UNI_CMD_ID_PP			= 0x38, /* PP */
	UNI_CMD_ID_TPC			= 0x39, /* TPC */
	UNI_CMD_ID_MEC			= 0x3A, /* MEC */
	UNI_CMD_ID_FR_TABLE		= 0x40, /* Set Fixed Rate TBL */
	UNI_CMD_ID_RSSI_MONITOR	= 0x41, /* Set monitoring RSSI range */
	UNI_CMD_ID_TEST_TR_PARAM	= 0x42,
					/* Set/Get testmode tx/rx parameter */
	UNI_CMD_ID_MQM_UPDATE_MU_EDCA_PARMS = 0x43, /* MU */
	UNI_CMD_ID_FRM_IND_FROM_HOST = 0x45, /* Support Host connect indicate*/
};

struct UNI_CMD_DEVINFO {
	/* fixed field */
	uint8_t ucOwnMacIdx;
	uint8_t ucDbdcIdx;
	uint8_t aucPadding[2];

	/* tlv */
	uint8_t aucTlvBuffer[0];
} __KAL_ATTRIB_PACKED__;

/* DevInfo command Tag */
enum ENUM_UNI_CMD_DEVINFO_TAG {
	UNI_CMD_DEVINFO_TAG_ACTIVE = 0,
	UNI_CMD_DEVINFO_TAG_NUM
};

/* DevInfo information (Tag0) */
struct UNI_CMD_DEVINFO_ACTIVE {
	uint16_t u2Tag;                   /* Tag = 0x00 */
	uint16_t u2Length;
	uint8_t ucActive;
	uint8_t aucPadding[1];
	uint8_t aucOwnMacAddr[6];
} __KAL_ATTRIB_PACKED__;

struct UNI_CMD_BSSINFO {
	/* fixed field */
	uint8_t ucBssInfoIdx;
	uint8_t aucPadding[3];

	/* tlv */
	uint8_t aucTlvBuffer[0];
} __KAL_ATTRIB_PACKED__;

/* BssInfo command Tag */
enum ENUM_UNI_CMD_BSSINFO_TAG {
	UNI_CMD_BSSINFO_TAG_BASIC = 0,
	UNI_CMD_BSSINFO_TAG_RA = 1,
	UNI_CMD_BSSINFO_TAG_RLM = 2,
	UNI_CMD_BSSINFO_TAG_PROTECT = 3,
	UNI_CMD_BSSINFO_TAG_BSS_COLOR = 4,
	UNI_CMD_BSSINFO_TAG_HE = 5,
	UNI_CMD_BSSINFO_TAG_11V_MBSSID = 6,
	UNI_CMD_BSSINFO_TAG_BCN_CONTENT = 7,
	UNI_CMD_BSSINFO_TAG_BCN_CSA = 8,
	UNI_CMD_BSSINFO_TAG_BCN_BCC = 9,
	UNI_CMD_BSSINFO_TAG_BCN_MBSSID = 0xA,
	UNI_CMD_BSSINFO_TAG_RATE = 0xB,
	UNI_CMD_BSSINFO_TAG_WAPI = 0xC,
	UNI_CMD_BSSINFO_TAG_SAP = 0xD,
	UNI_CMD_BSSINFO_TAG_P2P = 0xE,
	UNI_CMD_BSSINFO_TAG_QBSS = 0xF,
	UNI_CMD_BSSINFO_TAG_SEC = 0x10,
	UNI_CMD_BSSINFO_TAG_BCN_PROT = 0x11,
	UNI_CMD_BSSINFO_TAG_TXCMD = 0x12,
	UNI_CMD_BSSINFO_TAG_UAPSD = 0x13,
	UNI_CMD_BSSINFO_TAG_WMM_PS_TEST = 0x14,
	UNI_CMD_BSSINFO_TAG_POWER_SAVE = 0x15,
	UNI_CMD_BSSINFO_TAG_PM_ENABLE = 0x16,
	UNI_CMD_BSSINFO_TAG_IFS_TIME = 0x17,
	UNI_CMD_BSSINFO_TAG_STA_IOT = 0x18,
	UNI_CMD_BSSINFO_TAG_OFFLOAD_PKT = 0x19,
	UNI_CMD_BSSINFO_TAG_MLD = 0x1A,
	UNI_CMD_BSSINFO_TAG_PM_DISABLE = 0x1B,
	UNI_CMD_BSSINFO_NUM
};

typedef uint32_t(*PFN_UNI_CMD_BSSINFO_TAG_HANDLER) (IN struct ADAPTER
	*ad, IN uint8_t *buf, IN struct CMD_SET_BSS_INFO *cmd);

struct UNI_CMD_BSSINFO_TAG_HANDLE {
	uint32_t u4Size;
	PFN_UNI_CMD_BSSINFO_TAG_HANDLER pfHandler;
};

/* BssInfo basic information (Tag0) */
struct UNI_CMD_BSSINFO_BASIC {
	uint16_t u2Tag;          /* Tag = 0x00 */
	uint16_t u2Length;
	uint8_t  ucActive;
	uint8_t  ucOwnMacIdx;
	uint8_t  ucHwBSSIndex;
	uint8_t  ucDbdcIdx;
	uint32_t u4ConnectionType;
	uint8_t  ucConnectionState;
	uint8_t  ucWmmIdx;
	uint8_t  aucBSSID[6];
	uint16_t u2BcMcWlanidx;
		/* indicate which wlan-idx used for MC/BC transmission. */
	uint16_t u2BcnInterval;
	uint8_t  ucDtimPeriod;
	uint8_t  ucPhyMode;
	uint16_t u2StaRecIdxOfAP;
	uint16_t u2NonHTBasicPhyType;
	uint8_t  ucPhyModeExt;
	uint8_t  aucPadding[1];
} __KAL_ATTRIB_PACKED__;

/* BssInfo RA information (Tag1) */
struct UNI_CMD_BSSINFO_RA {
	uint16_t  u2Tag;                 /* Tag = 0x01 */
	uint16_t  u2Length;
	uint8_t  fgShortPreamble;
	uint8_t  fgTestbedForceShortGI;
	uint8_t  fgTestbedForceGreenField;
	uint8_t   ucHtMode;
	uint8_t  fgSeOff;
	uint8_t   ucAntennaIndex;
	uint16_t  u2MaxPhyRate;
	uint8_t   ucForceTxStream;
	uint8_t   aucPadding[3];
} __KAL_ATTRIB_PACKED__;

/* BssInfo RLM information (Tag2) */
struct UNI_CMD_BSSINFO_RLM {
	uint16_t u2Tag;  /* Tag = 0x02 */
	uint16_t u2Length;
	uint8_t  ucPrimaryChannel;
	uint8_t  ucCenterChannelSeg0;
	uint8_t  ucCenterChannelSeg1;
	uint8_t  ucBandwidth;
	uint8_t  ucTxStream;
	uint8_t  ucRxStream;
	uint8_t  ucHtOpInfo1; /* for mobile segment */
	uint8_t  ucSCO;    /* for mobile segment */
} __KAL_ATTRIB_PACKED__;

/*the enum elements syncs with the definition of host driver*/
enum ENUM_PROTECTION_MODE_T {
	/* 11n */
	HT_NON_MEMBER_PROTECT = BIT(1),
	HT_BW20_PROTECT = BIT(2),
	HT_NON_HT_MIXMODE_PROTECT = BIT(3),
	LEGACY_ERP_PROTECT = BIT(5),
	VEND_LONG_NAV_PROTECT = BIT(6),
	VEND_GREEN_FIELD_PROTECT = BIT(7),
	VEND_RIFS_PROTECT = BIT(8),
};

/* BssInfo protection information (Tag3) */
struct UNI_CMD_BSSINFO_PROTECT {
	uint16_t u2Tag;  /* Tag = 0x03 */
	uint16_t u2Length;
	uint32_t u4ProtectMode;
} __KAL_ATTRIB_PACKED__;

/* BssInfo bss color information (Tag4) */
struct UNI_CMD_BSSINFO_BSS_COLOR {
	uint16_t u2Tag;  /* Tag = 0x4 */
	uint16_t u2Length;
	uint8_t fgEnable;
	uint8_t  ucBssColor;
	uint8_t  aucPadding[2];
} __KAL_ATTRIB_PACKED__;

/* BssInfo HE information (Tag5) */
struct UNI_CMD_BSSINFO_HE {
	uint16_t u2Tag;  /* Tag = 0x05 */
	uint16_t u2Length;
	uint16_t u2TxopDurationRtsThreshold;
	uint8_t  ucDefaultPEDuration;
	uint8_t fgErSuDisable; /* for mobile segment */
	uint16_t au2MaxNssMcs[3];
	uint8_t  aucPadding[2];
} __KAL_ATTRIB_PACKED__;

/* BssInfo 11v MBSSID information (Tag6) */
struct UNI_CMD_BSSINFO_11V_MBSSID {
	uint16_t u2Tag;  /* Tag = 0x06 */
	uint16_t u2Length;
	uint8_t  ucMaxBSSIDIndicator;
	uint8_t  ucMBSSIDIndex;
	uint8_t  aucPadding[2];
} __KAL_ATTRIB_PACKED__;

struct UNI_CMD_BSSINFO_BCN_CONTENT {
	uint16_t u2Tag;       /* Tag = 0x07 */
	uint16_t u2Length;
	uint16_t u2TimIeOffset;
	uint16_t u2CsaIeOffset;
	uint16_t u2BccIeOffset;
	uint8_t  ucAction;
	uint8_t  aucPktContentType;
	uint16_t u2PktLength;
	uint8_t  aucPktContent[0];
} __KAL_ATTRIB_PACKED__;

enum BCN_CONTENT_ACTION {
	BCN_ACTION_DISABLE = 0,
	BCN_ACTION_ENABLE = 1,
	UPDATE_PROBE_RSP = 2,
};


/* BssInfo BCN CSA information (Tag8) */
struct UNI_CMD_BSSINFO_BCN_CSA {
	uint16_t u2Tag;       /* Tag = 0x08 */
	uint16_t u2Length;
	uint8_t  ucCsaCount;
	uint8_t  aucPadding[3];
} __KAL_ATTRIB_PACKED__;

/* BssInfo BCN BCC information (Tag9) */
struct UNI_CMD_BSSINFO_BCN_BCC {
	uint16_t u2Tag;       /* Tag = 0x9 */
	uint16_t u2Length;
	uint8_t  ucBccCount;
	uint8_t  aucPadding[3];
} __KAL_ATTRIB_PACKED__;

/* BssInfo BCN Mbssid-index ie information (Tag10) */
struct UNI_CMD_BSSINFO_BCN_MBSSID {
	uint16_t u2Tag;       /* Tag = 0xA */
	uint16_t u2Length;
	uint32_t u4Dot11vMbssidBitmap;
	uint16_t u2MbssidIeOffset[32];
} __KAL_ATTRIB_PACKED__;

/* BssInfo RATE information (Tag11) */
struct UNI_CMD_BSSINFO_RATE {
	uint16_t u2Tag;  /* Tag = 0x0B */
	uint16_t u2Length;
	uint16_t u2OperationalRateSet; /* for mobile segment */
	uint16_t u2BSSBasicRateSet;    /* for mobile segment */
	uint16_t u2BcRate; /* for WA */
	uint16_t u2McRate; /* for WA */
	uint8_t  ucPreambleMode; /* for WA */
	uint8_t  aucPadding[3];
} __KAL_ATTRIB_PACKED__;

/* BssInfo WAPI information (Tag12) */
struct UNI_CMD_BSSINFO_WAPI {
	uint16_t u2Tag;  /* Tag = 0x0C */
	uint16_t u2Length;
	uint8_t fgWapiMode;
	uint8_t  aucPadding[3];
} __KAL_ATTRIB_PACKED__;

/* BssInfo SAP information (Tag13) */
struct UNI_CMD_BSSINFO_SAP {
	uint16_t u2Tag;  /* Tag = 0x0D */
	uint16_t u2Length;
	uint8_t fgIsHiddenSSID;
	uint8_t  aucPadding[2];
	uint8_t  ucSSIDLen;
	uint8_t  aucSSID[32];
} __KAL_ATTRIB_PACKED__;

/* BssInfo P2P information (Tag14) */
struct UNI_CMD_BSSINFO_P2P {
	uint16_t u2Tag;  /* Tag = 0x0E */
	uint16_t u2Length;
	uint32_t  u4PrivateData;
} __KAL_ATTRIB_PACKED__;

/* BssInfo QBSS information (Tag15) */
struct UNI_CMD_BSSINFO_QBSS {
	uint16_t u2Tag;  /* Tag = 0x0F */
	uint16_t u2Length;
	uint8_t  ucIsQBSS;
	uint8_t  aucPadding[3];
} __KAL_ATTRIB_PACKED__;

/* BssInfo Security information (Tag16) */
struct UNI_CMD_BSSINFO_SEC {
	uint16_t u2Tag;  /* Tag = 0x10 */
	uint16_t u2Length;
	uint8_t  ucAuthMode;/**<
	  *     Auth Mode             | Value | Note          |
	  *     --------------------  | ------|-------------- |
	  *     AUTH_MODE_OPEN        | 0     | -             |
	  *     AUTH_MODE_SHARED      | 1     | Shared key    |
	  *     AUTH_MODE_AUTO_SWITCH | 2     | open system or shared key |
	  *     AUTH_MODE_WPA         | 3     | -             |
	  *     AUTH_MODE_WPA_PSK     | 4     | -             |
	  *     AUTH_MODE_WPA_NONE    | 5     | For Ad hoc    |
	  *     AUTH_MODE_WPA2        | 6     | -             |
	  *     AUTH_MODE_WPA2_PSK    | 7     | -             |
	  *     AUTH_MODE_WPA2_FT     | 8     | 802.11r       |
	  *     AUTH_MODE_WPA2_FT_PSK | 9     | 802.11r       |
	  *     AUTH_MODE_WPA_OSEN    | 10    | -             |
	  *     AUTH_MODE_WPA3_SAE    | 11    | -             |
	  */
	uint8_t  ucEncStatus;
	uint8_t  ucCipherSuit; /* wa */
	uint8_t  aucPadding[1];
} __KAL_ATTRIB_PACKED__;

/* BssInfo BCN Prot. information (Tag 0x11) */
struct UNI_CMD_BSSINFO_BCN_PROT {
	uint16_t u2Tag; /* Tag = 0x11 */
	uint16_t u2Length;
	uint8_t aucBcnProtPN[16];
	uint8_t ucBcnProtEnabled;  /* 0: off, 1: SW mode, 2:HW mode */
	uint8_t ucBcnProtCipherId;
	uint8_t aucBcnProtKey[32];
	uint8_t ucBcnProtKeyId;
	uint8_t aucReserved[3];
} __KAL_ATTRIB_PACKED__;

/* TxCMD Mode information (Tag 0x12) */
struct UNI_CMD_BSSINFO_TXCMD {
	uint16_t u2Tag;  /* Tag = 0x12 */
	uint16_t u2Length;
	uint8_t fgUseTxCMD;
	uint8_t  aucPadding[3];
} __KAL_ATTRIB_PACKED__;

/* BssInfo UAPSD information (Tag 0x13) */
struct UNI_CMD_BSSINFO_UAPSD {
	uint16_t u2Tag;
	uint16_t u2Length;
	uint8_t  ucBmpDeliveryAC;
	uint8_t  ucBmpTriggerAC;
	uint8_t  aucPadding[2];
} __KAL_ATTRIB_PACKED__;

/* BssInfo WMM PS test information (Tag 0x14) */
struct UNI_CMD_BSSINFO_WMM_PS_TEST {
	uint16_t u2Tag;
	uint16_t u2Length;
	uint8_t  ucIsEnterPsAtOnce;
	uint8_t  ucIsDisableUcTrigger;
	uint8_t  aucPadding[2];
} __KAL_ATTRIB_PACKED__;

/* BssInfo Power Save information (Tag 0x15) */
struct UNI_CMD_BSSINFO_POWER_SAVE {
	uint16_t u2Tag;
	uint16_t u2Length;
	uint8_t  ucPsProfile;/**<
	  * Power Save Mode                 | Value | Note                     |
	  * --------------------            | ------|--------------            |
	  * ENUM_PSP_CONTINUOUS_ACTIVE      | 0     | Leave power save mode    |
	  * ENUM_PSP_CONTINUOUS_POWER_SAVE  | 1     | Enter power save mode    |
	  * ENUM_PSP_FAST_SWITCH            | 2     | Fast switch mode         |
	  * ENUM_PSP_TWT                    | 3     | twt                      |
	  * ENUM_PSP_TWT_SP                 | 4     | twt sp                   |
	  */
	uint8_t  aucPadding[3];
} __KAL_ATTRIB_PACKED__;

/* BssInfo STA connection information (Tag 0x16) */
struct UNI_CMD_BSSINFO_STA_PM_ENABLE {
	uint16_t u2Tag;
	uint16_t u2Length;
	uint16_t u2BcnInterval;
	uint8_t  ucDtimPeriod;
	uint8_t  aucPadding[1];
} __KAL_ATTRIB_PACKED__;

/* BssInfo IFS time information (Tag 0x17) */
struct UNI_CMD_BSSINFO_IFS_TIME {
	uint16_t u2Tag;
	uint16_t u2Length;
	uint8_t fgSlotValid;
	uint8_t fgSifsValid;
	uint8_t fgRifsValid;
	uint8_t fgEifsValid;
	uint16_t u2SlotTime;
	uint16_t u2SifsTime;
	uint16_t u2RifsTime;
	uint16_t u2EifsTime;
	uint8_t fgEifsCckValid;
	uint8_t aucPadding[1];
	uint16_t u2EifsCckTime;
} __KAL_ATTRIB_PACKED__;

/* BssInfo Mobile need information (Tag 0x18) */
struct UNI_CMD_BSSINFO_IOT {
	uint16_t u2Tag; /* Tag = 0x18 */
	uint16_t u2Length;
	uint8_t ucIotApBmp;
	uint8_t aucReserved[3];
} __KAL_ATTRIB_PACKED__;

/* BssInfo IFS time information (Tag 0x19) */
struct UNI_CMD_BSSINFO_OFFLOAD_PKT {
	uint16_t u2Tag;
	uint16_t u2Length;
	uint8_t  ucTxType;
	uint8_t  ucTxMode;
	uint8_t  ucTxInterval;
	uint8_t  fgEnable;
	uint16_t u2Wcid;
	uint16_t u2OffloadPktLength;
	uint8_t  aucPktContent[0];
} __KAL_ATTRIB_PACKED__;

enum ENUM_UNSOLICIT_TX {
	UNSOLICIT_TX_PROBE_RSP = 0,
	UNSOLICIT_TX_FILS_DISC = 1,
	UNSOLICIT_TX_QOS_NULL  = 2 /* packet injector */
};

/* BssInfo MLD information (Tag 0x1A) */
struct UNI_CMD_BSSINFO_MLD {
	uint16_t u2Tag;   /* Tag = 0x1A */
	uint16_t u2Length;
	uint8_t  ucGroupMldId; /* mld_addr in mat_table index, legacy=0xff */
	uint8_t  ucOwnMldId; /* own_mac_addr in mat_table idex */
	uint8_t  aucOwnMldAddr[MAC_ADDR_LEN]; /* legacy=don't care */
	uint8_t  ucOmRemapIdx;
	/* for AGG: 0~15, 0xFF means this is a legacy BSS,
	 * no need to do remapping
	 */
	uint8_t  aucReserved[3];
} __KAL_ATTRIB_PACKED__;

/* BssInfo STA PM disable (Tag 0x1B) */
struct UNI_CMD_BSSINFO_STA_PM_DISABLE {
	uint16_t u2Tag;
	uint16_t u2Length;
} __KAL_ATTRIB_PACKED__;

/* Common part of CMD_STAREC */
struct UNI_CMD_STAREC {
	/* Fixed field*/
	uint8_t ucBssInfoIdx;
	uint8_t ucWlanIdxL;
	uint8_t aucPadding[4];
	uint8_t ucWlanIdxHnVer;
	uint8_t aucPadding2[1];

	/* TLV */
	uint8_t aucTlvBuffer[0];
} __KAL_ATTRIB_PACKED__;

/*  STA record TLV tag */
enum ENUM_UNI_CMD_STAREC_TAG {
	UNI_CMD_STAREC_TAG_BASIC		= 0x00,
	UNI_CMD_STAREC_TAG_RA			= 0x01,
	UNI_CMD_STAREC_TAG_RA_COMMON_INFO	= 0x02,
	UNI_CMD_STAREC_TAG_RA_UPDATE		= 0x03,
	UNI_CMD_STAREC_TAG_BF			= 0x04,
	UNI_CMD_STAREC_TAG_MAUNAL_ASSOC		= 0x05,
	UNI_CMD_STAREC_TAG_BA			= 0x06,
	UNI_CMD_STAREC_TAG_STATE_CHANGED	= 0x07,
	UNI_CMD_STAREC_TAG_HT_BASIC		= 0x09,
	UNI_CMD_STAREC_TAG_VHT_BASIC		= 0x0a,
	UNI_CMD_STAREC_TAG_AP_PS		= 0x0b,
	UNI_CMD_STAREC_TAG_INSTALL_KEY		= 0x0c,
	UNI_CMD_STAREC_TAG_WTBL			= 0x0d,
	UNI_CMD_STAREC_TAG_HW_AMSDU		= 0x0f,
	UNI_CMD_STAREC_TAG_AAD_OM		= 0x10,
	UNI_CMD_STAREC_TAG_INSTALL_KEY_V2	= 0x11,
	UNI_CMD_STAREC_TAG_MURU			= 0x12,
	UNI_CMD_STAREC_TAG_BFEE			= 0x14,
	UNI_CMD_STAREC_TAG_PHY_INFO		= 0x15,
	UNI_CMD_STAREC_TAG_BA_OFFLOAD		= 0x16,
	UNI_CMD_STAREC_TAG_HE_6G_CAP		= 0x17,
	UNI_CMD_STAREC_TAG_INSTALL_DEFAULT_KEY	= 0x18,
	UNI_CMD_STAREC_TAG_HE_BASIC		= 0x19,
	UNI_CMD_STAREC_TAG_MLD_SETUP		= 0x20,
	UNI_CMD_STAREC_TAG_EHT_MLD		= 0x21,
	UNI_CMD_STAREC_TAG_EHT_BASIC		= 0x22,
	UNI_CMD_STAREC_TAG_MLD_TEARDOWN		= 0x23,
	UNI_CMD_STAREC_TAG_UAPSD		= 0x24,
	UNI_CMD_STAREC_TAG_REMOVE		= 0x25,

	UNI_CMD_STAREC_TAG_MAX_NUM
};

typedef uint32_t(*PFN_UNI_CMD_STAREC_TAG_HANDLER) (IN struct ADAPTER
	*ad, IN uint8_t *buf, IN struct CMD_UPDATE_STA_RECORD *cmd);

struct UNI_CMD_STAREC_TAG_HANDLE {
	uint32_t u4Size;
	PFN_UNI_CMD_STAREC_TAG_HANDLER pfHandler;
};

struct UNI_CMD_STAREC_BASIC {
	/* Basic STA record (Group0) */
	uint16_t u2Tag;		/* Tag = 0x00 */
	uint16_t u2Length;
	uint32_t u4ConnectionType;
	uint8_t	ucConnectionState;
	uint8_t	ucIsQBSS;
	uint16_t u2AID;
	uint8_t	aucPeerMacAddr[6];
	/* This is used especially for 7615 to indicate this STAREC is
	 * to create new one or simply update
	 * In some case host may send new STAREC without delete old STAREC
	 * in advance. (ex: lost de-auth or get assoc twice)
	 * We need extra info to know if this is a brand new STAREC or not
	 * Consider backward compatibility, we check bit 0 in this reserve.
	 * Only the bit 0 is on, N9 go new way to update STAREC if bit 1 is
	 * on too.
	 * If neither bit match, N9 go orinal way to update STAREC.
	 */
	uint16_t u2ExtraInfo;
} __KAL_ATTRIB_PACKED__;

struct UNI_CMD_STAREC_STATE_INFO {
	uint16_t u2Tag;
	uint16_t u2Length;
	uint8_t ucStaState;
	uint32_t u4Flags;
	uint8_t ucVhtOpMode; /* VHT operting mode, bit 7: Rx NSS Type,
			      * hbit 4-6: Rx NSS, bit 0-1: Channel Width
			      */
	uint8_t ucActionType;
	uint8_t aucReserve[1];
} __KAL_ATTRIB_PACKED__;

struct UNI_CMD_TXBF_PFMU_STA_INFO {
	uint16_t   u2PfmuId;       /* 0xFFFF means no access right for PFMU */
	uint8_t    fgSU_MU;        /* 0 : SU, 1 : MU */
	uint8_t    u1TxBfCap;      /* 0 : ITxBf, 1 : ETxBf */
	uint8_t    ucSoundingPhy;  /* 0: legacy, 1: OFDM, 2: HT, 4: VHT */
	uint8_t    ucNdpaRate;
	uint8_t    ucNdpRate;
	uint8_t    ucReptPollRate;
	uint8_t    ucTxMode;       /* 0: legacy, 1: OFDM, 2: HT, 4: VHT */
	uint8_t    ucNc;
	uint8_t    ucNr;
	uint8_t    ucCBW;          /* 0 : 20M, 1 : 40M, 2 : 80M, 3 : 80 + 80M */
	uint8_t    ucTotMemRequire;
	uint8_t    ucMemRequire20M;
	uint8_t    ucMemRow0;
	uint8_t    ucMemCol0;
	uint8_t    ucMemRow1;
	uint8_t    ucMemCol1;
	uint8_t    ucMemRow2;
	uint8_t    ucMemCol2;
	uint8_t    ucMemRow3;
	uint8_t    ucMemCol3;
	uint16_t   u2SmartAnt;
	uint8_t    ucSEIdx;
	uint8_t    ucAutoSoundingCtrl;
	    /* Bit7: low traffic indicator,
	     * Bit6: Stop sounding for this entry, Bit5~0: postpone sounding
	     */
	uint8_t    uciBfTimeOut;
	uint8_t    uciBfDBW;
	uint8_t    uciBfNcol;
	uint8_t    uciBfNrow;
	uint8_t    nr_bw160;
	uint8_t	  nc_bw160;
	uint8_t    ru_start_idx;
	uint8_t    ru_end_idx;
	uint8_t   trigger_su;
	uint8_t   trigger_mu;
	uint8_t   ng16_su;
	uint8_t   ng16_mu;
	uint8_t   codebook42_su;
	uint8_t   codebook75_mu;
	uint8_t    he_ltf;
} __KAL_ATTRIB_PACKED__;

struct UNI_CMD_STAREC_BF {
	uint16_t u2Tag;      /* Tag = 0x04 */
	uint16_t u2Length;
	struct UNI_CMD_TXBF_PFMU_STA_INFO  rTxBfPfmuInfo;
} __KAL_ATTRIB_PACKED__;

struct UNI_CMD_STAREC_HT_INFO {
	uint16_t u2Tag;
	uint16_t u2Length;
	uint16_t u2HtCap;
	uint16_t u2HtExtendedCap;
} __KAL_ATTRIB_PACKED__;

struct UNI_CMD_STAREC_VHT_INFO {
	uint16_t u2Tag;
	uint16_t u2Length;
	uint32_t u4VhtCap;
	uint16_t u2VhtRxMcsMap;
	uint16_t u2VhtTxMcsMap;
	uint8_t	ucRTSBWSig; /* muru use only, ignored */
	uint8_t	aucReserve[3];
} __KAL_ATTRIB_PACKED__;

struct UNI_CMD_STAREC_PHY_INFO {
	uint16_t u2Tag;
	uint16_t u2Length;
	uint16_t u2BSSBasicRateSet;
	uint8_t ucDesiredPhyTypeSet;
	uint8_t ucAmpduParam;
	uint8_t ucRtsPolicy;
	uint8_t ucRCPI;
	uint8_t aucReserve[2];
} __KAL_ATTRIB_PACKED__;

/* Update RA Info */
struct UNI_CMD_STAREC_RA_INFO {
	uint16_t      u2Tag;
	uint16_t      u2Length;
	uint16_t      u2DesiredNonHTRateSet;
	uint8_t       aucRxMcsBitmask[10];
} __KAL_ATTRIB_PACKED__;

/* Update BA_OFFLOAD Info */
struct UNI_CMD_STAREC_BA_OFFLOAD_INFO {
	uint16_t      u2Tag;
	uint16_t      u2Length;
	uint8_t       ucTxAmpdu;
	uint8_t       ucRxAmpdu;
	uint8_t       ucTxAmsduInAmpdu;
	uint8_t       ucRxAmsduInAmpdu;
	uint32_t      u4TxMaxAmsduInAmpduLen;
	uint16_t      u2TxBaSize;
	uint16_t      u2RxBaSize;
} __KAL_ATTRIB_PACKED__;

struct UNI_CMD_STAREC_HE_BASIC {
	uint16_t u2Tag;
	uint16_t u2Length;

	uint8_t	    aucHeMacCapInfo[6];
	uint8_t	    aucHePhyCapInfo[11];
	uint8_t	    ucPktExt;
	/*0: BW80, 1: BW160, 2: BW8080*/
	uint16_t     au2RxMaxNssMcs[3];
} __KAL_ATTRIB_PACKED__;

/* Update HE 6g Info */
struct UNI_CMD_STAREC_HE_6G_CAP {
	uint16_t u2Tag;
	uint16_t u2Length;
	uint16_t u2He6gBandCapInfo;
	uint8_t aucReserve[2];
} __KAL_ATTRIB_PACKED__;

struct UNI_CMD_STAREC_AMSDU {
	uint16_t  u2Tag;		/* Tag = 0x05 */
	uint16_t  u2Length;
	uint8_t   ucMaxAmsduNum;
	uint8_t   ucMaxMpduSize;
	uint8_t   ucAmsduEnable;
	uint8_t   acuReserve[1];
} __KAL_ATTRIB_PACKED__;

/* mld starec setup (Tag 0x20) */
struct UNI_CMD_STAREC_MLD_SETUP {
	uint16_t  u2Tag;                 /* Tag = 0x20 */
	uint16_t  u2Length;
	uint8_t   aucPeerMldAddr[MAC_ADDR_LEN];
	uint16_t  u2PrimaryMldId;
	uint16_t  u2SecondMldId;
	uint16_t  u2SetupWlanId;
	uint8_t   ucLinkNumber;
	uint8_t   audPaddings[3];
	uint8_t   aucLinkInfo[0];
} __KAL_ATTRIB_PACKED__;

struct UNI_CMD_STAREC_LINK_INFO {
	uint16_t  u2WlanIdx;
	uint8_t   ucBssIdx;
	uint8_t   aucPadding[1];
} __KAL_ATTRIB_PACKED__;

/* starec MLD level information (Tag 0x21) */
struct UNI_CMD_STAREC_EHT_MLD {
	uint16_t  u2Tag;		/* Tag = 0x21 */
	uint16_t  u2Length;
	uint8_t   fgNSEP;
	uint8_t  ucEmlmrBitmap;
	uint8_t  ucEmlsrBitmap;
	uint8_t   afgStrCapBitmap[MLD_LINK_MAX];
	uint8_t   aucPadding[2];
} __KAL_ATTRIB_PACKED__;

/* starec link level EHT information (Tag 0x22) */
struct UNI_CMD_STAREC_EHT_BASIC {
	uint16_t  u2Tag;		/* Tag = 0x22 */
	uint16_t  u2Length;
	uint8_t   ucTidBitmap;
	uint8_t   aucPadding[3];
} __KAL_ATTRIB_PACKED__;

/* MLD STAREC teardown (Tag 0x23) */
struct UNI_CMD_STAREC_MLD_TEARDOWN {
	uint16_t  u2Tag;                 /* Tag = 0x23 */
	uint16_t  u2Length;
} __KAL_ATTRIB_PACKED__;

/* Update UAPSD Info */
struct UNI_CMD_STAREC_UAPSD_INFO {
	uint16_t      u2Tag;
	uint16_t      u2Length;
	uint8_t       fgIsUapsdSupported;
	uint8_t       ucUapsdAc;
	uint8_t       ucUapsdSp;
	uint8_t       aucReserve[1];
} __KAL_ATTRIB_PACKED__;

/* Remove STAREC */
struct UNI_CMD_STAREC_REMOVE_INFO {
	uint16_t      u2Tag;
	uint16_t      u2Length;
	uint8_t       ucActionType; /* ENUM_STA_REC_CMD_ACTION_T */
	uint8_t       aucReserve[3];
} __KAL_ATTRIB_PACKED__;

/* EDCA set command (0x04) */
struct UNI_CMD_EDCA {
	/* fixed field */
	uint8_t ucBssInfoIdx;
	uint8_t aucPadding[3];

	/* tlv */
	uint8_t aucTlvBuffer[0];
} __KAL_ATTRIB_PACKED__;

/* EDCA set command Tag */
enum ENUM_UNI_CMD_EDCA_TAG {
	UNI_CMD_EDCA_TAG_AC_PARM = 0,
	UNI_CMD_EDCA_TAG_NUM
};

/* EDCA AC Parameters (Tag0) */
struct UNI_CMD_EDCA_AC_PARM {
	uint16_t u2Tag;
	uint16_t u2Length;
	uint8_t ucAcIndex;
	uint8_t ucValidBitmap;
	/**<
	  *      Define            | BIT | Note              |
	  *      ------------------|-----|------------------ |
	  *      MASK_AIFS_SET     | 0   | 0x01, AIFSN       |
	  *      MASK_WINMIN_SET   | 1   | 0x02, CW min      |
	  *      MASK_WINMAX_SET   | 2   | 0x04, CW max      |
	  *      MASK_TXOP_SET     | 3   | 0x08, TXOP Limit  |
	  */
	uint8_t ucCWmin;
	uint8_t ucCWmax;
	uint16_t u2TxopLimit;
	uint8_t ucAifsn;
	uint8_t aucPadding[1];
} __KAL_ATTRIB_PACKED__;

struct UNI_CMD_SUSPEND {
	/*fixed field*/
	uint8_t ucBssInfoIdx;
	uint8_t aucPadding[3];

	/* tlv */
	uint8_t aucTlvBuffer[0]; /**< the TLVs included in this field:
    *
    * TAG                             | ID  | structure
    * -------------                   | ----| -------------
    * UNI_CMD_SUSPEND_MODE_SETTING    | 0x0 | UNI_CMD_SUSPEND_MODE_SETTING_T
    * UNI_CMD_SUSPEND_WOW_CTRL        | 0x1 | UNI_CMD_SUSPEND_WOW_CTRL_T
    * UNI_CMD_SUSPEND_WOW_GPIO_PARAM  | 0x2 | UNI_CMD_SUSPEND_WOW_GPIO_PARAM_T
    * UNI_CMD_SUSPEND_WOW_WAKEUP_PORT | 0x3 | UNI_CMD_SUSPEND_WOW_WAKEUP_PORT_T
    * UNI_CMD_SUSPEND_WOW_PATTERN     | 0x4 | UNI_CMD_SUSPEND_WOW_PATTERN_T
    */
} __KAL_ATTRIB_PACKED__;

/* Suspend command Tag */
enum ENUM_UNI_CMD_SUSPEND_TAG {
	UNI_CMD_SUSPEND_TAG_MODE_SETTING = 0,
	UNI_CMD_SUSPEND_TAG_WOW_CTRL = 1,
	UNI_CMD_SUSPEND_TAG_WOW_GPIO_PARAM = 2,
	UNI_CMD_SUSPEND_TAG_WOW_WAKEUP_PORT = 3,
	UNI_CMD_SUSPEND_TAG_WOW_PATTERN = 4,
	UNI_CMD_SUSPEND_TAG_NUM
};

/* suspend mode setting (Tag0) */
struct UNI_CMD_SUSPEND_MODE_SETTING {
	uint16_t u2Tag;                   /* Tag = 0x00 */
	uint16_t u2Length;

	uint8_t ucScreenStatus;
	uint8_t ucMdtim;
	uint8_t ucWowSuspend;
	uint8_t aucPadding[1];

	uint8_t aucReserved[4];
} __KAL_ATTRIB_PACKED__;

struct UNI_CMD_SUSPEND_WOW_CTRL {
	uint16_t u2Tag;                   /* Tag = 0x01 */
	uint16_t u2Length;

	uint8_t ucCmd;
	uint8_t ucDetectType;
	uint8_t ucWakeupHif;
	uint8_t aucPadding[1];

	uint8_t aucReserved[4];
} __KAL_ATTRIB_PACKED__;

struct UNI_CMD_SUSPEND_WOW_GPIO_PARAM {
	uint16_t u2Tag;                   /* Tag = 0x02 */
	uint16_t u2Length;

	uint8_t ucGpioPin;
	uint8_t ucTriggerLvl;
	uint8_t aucPadding[2];

	uint32_t u4GpioInterval;

	uint8_t aucReserved[4];
} __KAL_ATTRIB_PACKED__;

struct UNI_CMD_SUSPEND_WOW_WAKEUP_PORT {
	uint16_t u2Tag;                   /* Tag = 0x03 */
	uint16_t u2Length;

	uint8_t ucIPv4UdpPortCnt;
	uint8_t ucIPv4TcpPortCnt;
	uint8_t ucIPv6UdpPortCnt;
	uint8_t ucIPv6TcpPortCnt;

	uint16_t ausIPv4UdpPort[UNI_CMD_MAX_TCP_UDP_PORT];
	uint16_t ausIPv4TcpPort[UNI_CMD_MAX_TCP_UDP_PORT];
	uint16_t ausIPv6UdpPort[UNI_CMD_MAX_TCP_UDP_PORT];
	uint16_t ausIPv6TcpPort[UNI_CMD_MAX_TCP_UDP_PORT];

	uint8_t aucReserved[4];
} __KAL_ATTRIB_PACKED__;

struct UNI_CMD_OFFLOAD {
	/*fixed field*/
	uint8_t ucBssInfoIdx;
	uint8_t aucPadding[3];

	/* tlv */
	uint8_t aucTlvBuffer[0]; /**< the TLVs included in this field:
    *
    *   TAG                             | ID  | structure
    *   -------------                   | ----| -------------
    *   UNI_CMD_OFFLOAD_ARPNS_IPV4      | 0x0 | UNI_CMD_OFFLOAD_ARPNS_IPV4_T
    *   UNI_CMD_OFFLOAD_ARPNS_IPV6      | 0x1 | UNI_CMD_OFFLOAD_ARPNS_IPV6_T
    *   UNI_CMD_OFFLOAD_GTK_REKEY       | 0x2 | UNI_CMD_OFFLOAD_GTK_REKEY_T
    *   UNI_CMD_OFFLOAD_BMC_RPY_DETECT  | 0x3 | UNI_CMD_OFFLOAD_BMC_RPY_DETECT_T
    */
};

enum ENUM_UNI_CMD_OFFLOAD_TAG {
	UNI_CMD_OFFLOAD_TAG_ARPNS_IPV4 = 0,
	UNI_CMD_OFFLOAD_TAG_ARPNS_IPV6 = 1,
	UNI_CMD_OFFLOAD_TAG_GTK_REKEY = 2,
	UNI_CMD_OFFLOAD_TAG_BMC_RPY_DETECT = 3,
	UNI_CMD_OFFLOAD_TAG_NUM
};

struct IPV4_ADDRESS_V0 {
	uint8_t aucIpAddr[IPV4_ADDR_LEN];
};

struct IPV4_ADDRESS {
	uint8_t aucIpAddr[IPV4_ADDR_LEN];
	uint8_t aucIpMask[IPV4_ADDR_LEN];
};

struct UNI_CMD_OFFLOAD_ARPNS_IPV4 {
	uint16_t u2Tag;                   /* Tag = 0x00 */
	uint16_t u2Length;

	uint8_t ucEnable;
	uint8_t ucIpv4AddressCount;
	uint8_t ucVersion;
	uint8_t aucPadding[1];

	struct IPV4_ADDRESS arIpv4NetAddress[0];
} __KAL_ATTRIB_PACKED__;

struct IPV6_ADDRESS {
	uint8_t aucIpAddr[IPV6_ADDR_LEN];
};

struct UNI_CMD_OFFLOAD_ARPNS_IPV6 {
	uint16_t u2Tag;                   /* Tag = 0x01 */
	uint16_t u2Length;

	uint8_t ucEnable;
	uint8_t ucIpv6AddressCount;
	uint8_t aucPadding[2];

	struct IPV6_ADDRESS arIpv6NetAddress[0];
} __KAL_ATTRIB_PACKED__;

struct UNI_CMD_OFFLOAD_GTK_REKEY {
	uint16_t u2Tag;                   /* Tag = 0x02 */
	uint16_t u2Length;

	uint8_t aucKek[16];
	uint8_t aucKck[16];
	uint8_t aucReplayCtr[8];

	uint8_t ucRekeyMode;
	uint8_t ucCurKeyId;
	uint8_t ucOption;
	uint8_t aucPadding[1];

	uint32_t u4Proto;
	uint32_t u4PairwiseCipher;
	uint32_t u4GroupCipher;
	uint32_t u4KeyMgmt;
	uint32_t u4MgmtGroupCipher;

	uint8_t aucReserved[4];
} __KAL_ATTRIB_PACKED__;

struct UNI_CMD_OFFLOAD_BMC_RPY_DETECT {
	uint16_t u2Tag;                   /* Tag = 0x03 */
	uint16_t u2Length;

	uint8_t aucBMCOffloadKeyRscPN[8];

	uint8_t ucMode;
	uint8_t ucKeyId;
	uint8_t aucPadding[2];

	uint8_t aucReserved[4];
} __KAL_ATTRIB_PACKED__;

struct UNI_CMD_BAND_CONFIG {
	/*fixed field*/
	uint8_t ucDbdcIdx;
	uint8_t aucPadding[3];

	/* tlv */
	uint8_t aucTlvBuffer[0]; /**< the TLVs included in this field:
*
* TAG                                | ID  | structure
* -------------                      | ----| -------------
* UNI_CMD_BAND_CONFIG_RADIO_ONOFF    |0x0| UNI_CMD_BAND_CONFIG_RADIO_ONOFF_T
* UNI_CMD_BAND_CONFIG_RXV_CTRL       |0x1| UNI_CMD_BAND_CONFIG_RXV_CTRL_T
* UNI_CMD_BAND_CONFIG_SET_RX_FILTER  |0x2| UNI_CMD_BAND_CONFIG_SET_RX_FILTER_T
* UNI_CMD_BAND_CONFIG_DROP_CTRL_FRAME|0x3| UNI_CMD_BAND_CONFIG_DROP_CTRL_FRAME_T
* UNI_CMD_BAND_CONFIG_AGG_AC_LIMIT   |0x4| UNI_CMD_BAND_CONFIG_AGG_AC_LIMIT_T
* UNI_CMD_BAND_CONFIG_EDCCA_ENABLE   |0x5|
*				      UNI_CMD_BAND_CONFIG_EDCCA_ENABLE_CTRL_T
* UNI_CMD_BAND_CONFIG_EDCCA_THRESHOLD|0x6|
*				      UNI_CMD_BAND_CONFIG_EDCCA_THRESHOLD_CTRL_T
*/
} __KAL_ATTRIB_PACKED__;

/* Band config Tag */
enum ENUM_UNI_CMD_BAND_CONFIG_TAG {
	UNI_CMD_BAND_CONFIG_TAG_RADIO_ONOFF = 0,
	UNI_CMD_BAND_CONFIG_TAG_RXV_CTRL = 1,
	UNI_CMD_BAND_CONFIG_TAG_SET_RX_FILTER = 2,
	UNI_CMD_BAND_CONFIG_TAG_DROP_CTRL_FRAME = 3,
	UNI_CMD_BAND_CONFIG_TAG_AGG_AC_LIMIT = 4,
	UNI_CMD_BAND_CONFIG_TAG_EDCCA_ENABLE = 5,
	UNI_CMD_BAND_CONFIG_TAG_EDCCA_THRESHOLD = 6,
	UNI_CMD_BAND_CONFIG_MAX_NUM
};

struct UNI_CMD_BAND_CONFIG_RADIO_ONOFF {
	uint16_t u2Tag;
	uint16_t u2Length;
	uint8_t fgRadioOn;
	uint8_t  aucPadding[3];
} __KAL_ATTRIB_PACKED__;

struct UNI_CMD_BAND_CONFIG_RXV_CTRL {
	uint16_t u2Tag;
	uint16_t u2Length;
	uint8_t  ucRxvOfRxEnable;
	uint8_t  ucRxvOfTxEnable;
	uint8_t  aucPadding[2];
} __KAL_ATTRIB_PACKED__;

struct UNI_CMD_BAND_CONFIG_SET_RX_FILTER {
	uint16_t u2Tag;
	uint16_t u2Length;
	uint32_t u4RxPacketFilter;
} __KAL_ATTRIB_PACKED__;

struct UNI_CMD_BAND_CONFIG_DROP_CTRL_FRAME {
	uint16_t u2Tag;
	uint16_t u2Length;
	uint8_t ucDropRts;
	uint8_t ucDropCts;
	uint8_t ucDropUnwantedCtrl;
	uint8_t aucReserved[1];
} __KAL_ATTRIB_PACKED__;

struct UNI_CMD_BAND_CONFIG_AGG_AC_LIMIT {
	uint16_t u2Tag;
	uint16_t u2Length;
	uint8_t ucWmmIdx;
	uint8_t ucAc;
	uint16_t u2AggLimit;
} __KAL_ATTRIB_PACKED__;

/* EDCCA OnOff Control (Tag5) */
struct UNI_CMD_BAND_CONFIG_EDCCA_ENABLE_CTRL {
	uint16_t u2Tag;    /* Tag = 0x05 */
	uint16_t u2Length;
	uint8_t fgEDCCAEnable;
	uint8_t aucPadding[3];
} __KAL_ATTRIB_PACKED__;

/* EDCCA Threshold Control (Tag6) */
struct UNI_CMD_BAND_CONFIG_EDCCA_THRESHOLD_CTRL {
	uint16_t u2Tag;    /* Tag = 0x06 */
	uint16_t u2Length;
	uint8_t u1EDCCAThreshold[3];
	uint8_t fginit;
} __KAL_ATTRIB_PACKED__;

/* WSYS Config set command (0x0B) */
struct UNI_CMD_WSYS_CONFIG {
	/* fixed field */
	uint8_t ucReserved[4];

	/* tlv */
	uint8_t aucTlvBuffer[0];/**< the TLVs included in this field:
*
* TAG                                 |ID |structure
* ------------------------------------|---|-------------
* UNI_CMD_WSYS_CONFIG_FW_LOG_CTRL     |0x0|UNI_CMD_FW_LOG_CTRL_BASIC_T
* UNI_CMD_WSYS_CONFIG_FW_DBG_CTRL     |0x1|UNI_CMD_FW_DBG_CTRL_T
* UNI_CMD_WSYS_CONFIG_FW_LOG_UI_CTRL  |0x2|UNI_CMD_WSYS_CONFIG_FW_LOG_UI_CTRL_T
* UNI_CMD_WSYS_CONFIG_FW_BASIC_CONFIG |0x3|UNI_CMD_WSYS_CONFIG_FW_BASIC_CONFIG_T
* UNI_CMD_HOSTREPORT_TX_LATENCY_CONFIG|0x4|
*				     UNI_CMD_WSYS_CONFIG_HOSTREPORT_TX_LATENCY_T
*/
}  __KAL_ATTRIB_PACKED__;

/* WSYS Config set command TLV List */
enum ENUM_UNI_CMD_WSYS_CONFIG_TAG {
	UNI_CMD_WSYS_CONFIG_TAG_FW_LOG_CTRL = 0,
	UNI_CMD_WSYS_CONFIG_TAG_FW_DBG_CTRL = 1,
	UNI_CMD_WSYS_CONFIG_TAG_FW_LOG_UI_CTRL = 2,
	UNI_CMD_WSYS_CONFIG_TAG_FW_BASIC_CONFIG = 3,
	UNI_CMD_WSYS_CONFIG_TAG_HOSTREPORT_TX_LATENCY_CONFIG = 4,
	UNI_CMD_WSYS_CONFIG_TAG_NUM
};

/* FW Log UI Setting (Tag2) */
struct UNI_CMD_WSYS_CONFIG_FW_LOG_UI_CTRL {
	uint16_t u2Tag;
	uint16_t u2Length;
	uint32_t ucVersion; /* default is 1 */
	uint32_t ucLogLevel;/* 0: Default, 1: More, 2: Extreme */
	uint8_t  aucReserved[4];
} __KAL_ATTRIB_PACKED__;

/* FW Debug Level Setting (Tag3) */
struct UNI_CMD_WSYS_CONFIG_FW_BASIC_CONFIG {
	uint16_t u2Tag;
	uint16_t u2Length;
	uint16_t u2RxChecksum;   /* bit0: IP, bit1: UDP, bit2: TCP */
	uint16_t u2TxChecksum;   /* bit0: IP, bit1: UDP, bit2: TCP */
	uint8_t ucCtrlFlagAssertPath;
	uint8_t aucPadding[3];
} __KAL_ATTRIB_PACKED__;

/* register access command (0x0D) */
struct UNI_CMD_ACCESS_REG {
	/* fixed field */
	uint8_t ucReserved[4];

	/* tlv */
	uint8_t aucTlvBuffer[0];/**< the TLVs included in this field:
	*
	*   TAG                           | ID  | structure
	*   ------------------------------|-----|--------------
	*   UNI_CMD_ACCESS_REG_BASIC      | 0x0 | UNI_CMD_ACCESS_REG_BASIC_T
	*   UNI_CMD_ACCESS_RF_REG_BASIC   | 0x1 | UNI_CMD_ACCESS_RF_REG_BASIC_T
	*/
} __KAL_ATTRIB_PACKED__;

/* Register access command TLV List */
enum ENUM_UNI_CMD_ACCESS_REG_TAG {
	UNI_CMD_ACCESS_REG_TAG_BASIC = 0,
	UNI_CMD_ACCESS_REG_TAG_RF_REG_BASIC,
	UNI_CMD_ACCESS_REG_TAG_NUM
};

struct UNI_CMD_ACCESS_REG_BASIC {
	uint16_t u2Tag;
	uint16_t u2Length;
	uint32_t u4Addr;
	uint32_t u4Value;
} __KAL_ATTRIB_PACKED__;

struct UNI_CMD_ACCESS_RF_REG_BASIC {
	uint16_t u2Tag;
	uint16_t u2Length;
	uint16_t u2WifiStream;
	uint16_t u2Reserved;
	uint32_t u4Addr;
	uint32_t u4Value;
} __KAL_ATTRIB_PACKED__;

/* Chip Config set command (0x0E) */
struct UNI_CMD_CHIP_CONFIG {
	/* fixed field */
	uint8_t ucReserved[4];

	/* tlv */
	uint8_t aucTlvBuffer[0];/**< the TLVs included in this field:
*
* TAG                                | ID | structure
* -----------------------------------|----|--------------
* UNI_CMD_CHIP_CONFIG_SW_DBG_CTRL    | 0x0| UNI_CMD_CHIP_CONFIG_SW_DBG_CTRL_T
* UNI_CMD_CHIP_CONFIG_CUSTOMER_CFG   | 0x1| UNI_CMD_CHIP_CONFIG_CUSTOMER_CFG_T
* UNI_CMD_CHIP_CONFIG_CHIP_CFG       | 0x2| UNI_CMD_CHIP_CONFIG_CHIP_CFG_T
* UNI_CMD_CHIP_CONFIG_NIC_CAPABILITY | 0x3| UNI_CMD_CHIP_CONFIG_NIC_CAPABILITY_T
*/
} __KAL_ATTRIB_PACKED__;

/* Chip Config set command TLV List */
enum ENUM_UNI_CMD_CHIP_CONFIG_TAG {
	UNI_CMD_CHIP_CONFIG_TAG_SW_DBG_CTRL = 0,
	UNI_CMD_CHIP_CONFIG_TAG_CUSTOMER_CFG = 1,
	UNI_CMD_CHIP_CONFIG_TAG_CHIP_CFG = 2,
	UNI_CMD_CHIP_CONFIG_TAG_NIC_CAPABILITY = 3,
	UNI_CMD_CHIP_CONFIG_TAG_NUM
};

/* SW DBG control Setting (Tag0) */
struct UNI_CMD_CHIP_CONFIG_SW_DBG_CTRL {
	uint16_t u2Tag; /* Tag = 0x00 */
	uint16_t u2Length;
	uint32_t u4Id;
	uint32_t u4Data;
} __KAL_ATTRIB_PACKED__;

/* Customer Configuration Setting (Tag1) */
struct UNI_CMD_CHIP_CONFIG_CUSTOMER_CFG {
	uint16_t u2Tag;
	uint16_t u2Length;
	uint16_t cmdBufferLen;
	uint8_t itemNum;
	uint8_t aucPadding[1];
	uint8_t aucbuffer[MAX_CMD_BUFFER_LENGTH];
} __KAL_ATTRIB_PACKED__;

/* get or set chip configuration (Tag2) */
struct UNI_CMD_CHIP_CONFIG_CHIP_CFG {
	uint16_t u2Tag;
	uint16_t u2Length;
	uint8_t aucbuffer[0];
} __KAL_ATTRIB_PACKED__;

struct UNI_CMD_CHIP_CONFIG_CHIP_CFG_RESP {
	uint16_t u2Id;
	uint8_t ucType;
	uint8_t ucRespType;
	uint16_t u2MsgSize;
	uint8_t aucReserved0[2];
	uint8_t aucCmd[UNICMD_CHIP_CONFIG_RESP_SIZE];
} __KAL_ATTRIB_PACKED__;

/* Get Connsys Cpability (Tag3) */
struct UNI_CMD_CHIP_CONFIG_NIC_CAPABILITY {
	uint16_t u2Tag;
	uint16_t u2Length;
} __KAL_ATTRIB_PACKED__;

struct UNI_CMD_POWER_CTRL {
	/* fixed field */
	uint8_t ucReserved[4];

	/* tlv */
	uint8_t aucTlvBuffer[0];/**< the TLVs included in this field:
	*
	*   TAG                      | ID  | structure
	*   -------------------------|-----|--------------
	*   UNI_CMD_POWER_OFF        | 0x0 | UNI_CMD_POWER_OFF_T
	*/
} __KAL_ATTRIB_PACKED__;

enum ENUM_UNI_CMD_POWER_CTRL_TAG {
	UNI_CMD_POWER_CTRL_TAG_OFF = 0,
	UNI_CMD_POWER_CTRL_TAG_NUM
};

struct UNI_CMD_POWER_OFF {
	uint16_t u2Tag;
	uint16_t u2Length;
	uint8_t ucPowerMode;
	uint8_t aucReserved[3];
} __KAL_ATTRIB_PACKED__;

/* Rx header translation command (0x12) */
struct UNI_CMD_SER {
	/* fixed field */
	uint8_t aucReserved[4];

	/* tlv */
	uint8_t aucTlvBuffer[0]; /**<the TLVs includer in this field:
	*
	*  TAG                    | ID   | structure
	*  -------------          | -----| -------------
	*  UNI_CMD_SER_QUERY      | 0x0  | UNI_CMD_SER_QUERY_T
	*  UNI_CMD_SER_ENABLE     | 0x1  | UNI_CMD_SER_ENABLE_T
	*  UNI_CMD_SER_SET        | 0x2  | UNI_CMD_SER_SET_T
	*  UNI_CMD_SER_TRIGGER    | 0x3  | UNI_CMD_SER_TRIGGER_T
	*/
} __KAL_ATTRIB_PACKED__;

/* SER command TLV List */
enum ENUM_UNI_CMD_SER_TAG {
	UNI_CMD_SER_TAG_QUERY = 0,
	UNI_CMD_SER_TAG_ENABLE = 1,
	UNI_CMD_SER_TAG_SET = 2,
	UNI_CMD_SER_TAG_TRIGGER = 3,
	UNI_CMD_SER_TAG_L0P5_CTRL = 4,
	UNI_CMD_SER_TAG_NUM
};

/* Show ser (Tag0) */
struct UNI_CMD_SER_QUERY {
	uint16_t u2Tag;
	uint16_t u2Length;
} __KAL_ATTRIB_PACKED__;

/* Enable/disable ser (Tag1) */
struct UNI_CMD_SER_ENABLE {
	uint16_t u2Tag;
	uint16_t u2Length;
	uint8_t fgEnable;
	uint8_t aucPadding[3];
} __KAL_ATTRIB_PACKED__;

/* config ser (Tag2) */
struct UNI_CMD_SER_SET {
	uint16_t u2Tag;
	uint16_t u2Length;
	uint32_t u4EnableMask;
} __KAL_ATTRIB_PACKED__;

/* trigger ser recovery (Tag3) */
struct UNI_CMD_SER_TRIGGER {
	uint16_t u2Tag;
	uint16_t u2Length;
	uint8_t ucTriggerMethod;
	uint8_t ucDbdcIdx;
	uint8_t aucPadding[2];
} __KAL_ATTRIB_PACKED__;

struct UNI_CMD_TWT {
	/*fixed field*/
	uint8_t ucBssInfoIdx;
	uint8_t aucPadding[3];
	/* tlv */
	uint8_t aucTlvBuffer[0]; /**< the TLVs included in this field:
	*
	*  TAG                         | ID   | structure
	*  -------------               | -----| -------------
	*  UNI_CMD_TWT_AGRT_UPDATE       | 0x0  | UNI_CMD_TWT_ARGT_UPDATE_T
	*/
} __KAL_ATTRIB_PACKED__;

enum ENUM_UNI_CMD_TWT_TAG {
	UNI_CMD_TWT_TAG_AGRT_UPDATE = 0,
	UNI_CMD_TWT_TAG_NUM
};

struct UNI_CMD_TWT_ARGT_UPDATE {
	uint16_t    u2Tag;    /* Tag = 0x00 */
	uint16_t    u2Length;
	uint8_t      ucAgrtTblIdx;
	uint8_t      ucAgrtCtrlFlag;
	uint8_t      ucOwnMacId;
	uint8_t      ucFlowId;
	uint16_t     u2PeerIdGrpId;
	uint8_t      ucAgrtSpDuration;
	uint8_t      ucBssIndex;
	uint32_t     u4AgrtSpStartTsf_low;
	uint32_t     u4AgrtSpStartTsf_high;
	uint16_t     u2AgrtSpWakeIntvlMantissa;
	uint8_t      ucAgrtSpWakeIntvlExponent;
	uint8_t      fgIsRoleAp;
	uint8_t      ucAgrtParaBitmap;
	uint8_t      ucReserved_a;
	uint16_t     u2Reserved_b;
	uint8_t      ucGrpMemberCnt;
	uint8_t      ucReserved_c;
	uint16_t     u2Reserved_d;
	uint16_t     au2StaList[UNI_TWT_GRP_MAX_MEMBER_CNT];
} __KAL_ATTRIB_PACKED__;

struct UNI_CMD_DOMAIN_SET_INFO {
	/* fixed field */
	uint32_t u4CountryCode;
	uint8_t  uc2G4Bandwidth; /* CONFIG_BW_20_40M or CONFIG_BW_20M */
	uint8_t  uc5GBandwidth;  /* CONFIG_BW_20_40M or CONFIG_BW_20M */
	uint8_t  uc6GBandwidth;
	uint8_t  aucReserved[1];
	/* tlv */
	uint8_t  aucTlvBuffer[0];
/**< the TLVs included in this field:
*
* TAG                          | ID   | structure
* -------------                | ---- | -------------
* ENUM_UNI_CMD_DOMAIN_SUBBAND  | 0x01 | UNI_CMD_DOMAIN_SET_INFO_DOMAIN_SUBBAND_T
* ENUM_UNI_CMD_DOMAIN_ACTCHNL  | 0x02 |
*			UNI_CMD_DOMAIN_SET_INFO_DOMAIN_ACTIVE_CHANNEL_LIST_T
*/
} __KAL_ATTRIB_PACKED__;

enum ENUM_UNI_CMD_DOMAIN_TAG {
	UNI_CMD_DOMAIN_TAG_SUBBAND = 1,
	UNI_CMD_DOMAIN_TAG_ACTCHNL = 2,
	UNI_CMD_DOMAIN_TAG_NUM
};

struct UNI_CMD_DOMAIN_SET_INFO_DOMAIN_SUBBAND {
	uint16_t u2Tag;                   /* Tag = 0x01 */
	uint16_t u2Length;
	uint16_t u2IsSetPassiveScan;
	uint8_t  aucReserved[1];
	uint8_t  ucSubBandNum;
	uint8_t  aucSubBandInfoBuffer[0]; /* UNI_CMD_DOMAIN_SUBBAND_INFO */
} __KAL_ATTRIB_PACKED__;

struct UNI_CMD_DOMAIN_SUBBAND_INFO {
	uint8_t ucRegClass;
	uint8_t ucBand;
	uint8_t ucChannelSpan;
	uint8_t ucFirstChannelNum;
	uint8_t ucNumChannels;
	uint8_t fgDfs;         /* Type: BOOLEAN (fgDfsNeeded) */
	uint8_t aucReserved[2];
} __KAL_ATTRIB_PACKED__;

struct UNI_CMD_DOMAIN_SET_INFO_DOMAIN_ACTIVE_CHANNEL_LIST {
	uint16_t u2Tag;                   /* Tag = 0x02 */
	uint16_t u2Length;
	uint8_t  u1ActiveChNum2g;
	uint8_t  u1ActiveChNum5g;
	uint8_t  u1ActiveChNum6g;
	uint8_t  aucReserved[1];
	uint8_t  aucActChnlListBuffer[0]; /* DOMAIN_CHANNEL_T */
} __KAL_ATTRIB_PACKED__;

/* IDC command (0x17) */
struct UNI_CMD_IDC {
	uint8_t ucReserved[4];

	/* tlv */
	uint8_t aucTlvBuffer[0];/**< the TLVs included in this field:
* TAG                        | ID  | structure
* ---------------------------|-----|--------------
* UNI_CMD_ID_GET_IDC_CHN    | 0x00 | UNI_CMD_GET_IDC_CHN_T
* UNI_CMD_ID_ALWAYS_SCAN_PARAM_SETTING |0x01|UNI_CMD_ALWAYS_SCAN_PARAM_SETTING_T
* UNI_CMD_ID_CCCI_MSG       | 0x02 | UNI_CMD_CCCI_MSG_T
* UNI_CMD_ID_3WIRE_GROUP    | 0x03 | UNI_CMD_3WIRE_GROUP_T
*/

} __KAL_ATTRIB_PACKED__;

/* IDC config Tag */
enum ENUM_UNI_CMD_IDC_TAG {
	UNI_CMD_IDC_TAG_GET_IDC_CHN = 0,
	UNI_CMD_IDC_TAG_ALWAYS_SCAN_PARAM_SETTING = 1,
	UNI_CMD_IDC_TAG_CCCI_MSG = 2,
	UNI_CMD_IDC_TAG_3WIRE_GROUP = 3,
	UNI_CMD_IDC_TAG_NUM
};

/* IDC Setting (Tag0) */
struct UNI_CMD_GET_IDC_CHN {
	uint16_t u2Tag;
	uint16_t u2Length;
} __KAL_ATTRIB_PACKED__;

/* IDC Setting (Tag1) */
struct UNI_CMD_ALWAYS_SCAN_PARAM_SETTING {
	uint16_t   u2Tag;
	uint16_t   u2Length;
	uint8_t  fgAlwaysScanEnable;
	uint8_t     aucReserved[3];
} __KAL_ATTRIB_PACKED__;

struct UNI_CMD_SCAN {
	/* fixed field */
	uint8_t ucSeqNum;
	uint8_t ucBssIndex;
	uint8_t aucPadding[2];

	/* tlv */
	uint8_t aucTlvBuffer[0];
} __KAL_ATTRIB_PACKED__;

typedef uint32_t(*PFN_UNI_CMD_SCAN_TAG_HANDLER) (IN struct ADAPTER
	*ad, IN uint8_t *buf, IN struct CMD_SCAN_REQ_V2 *cmd);

struct UNI_CMD_SCAN_TAG_HANDLE {
	uint32_t u4Size;
	PFN_UNI_CMD_SCAN_TAG_HANDLER pfHandler;
};

enum ENUM_UNI_CMD_SCAN_TAG {
	UNI_CMD_SCAN_TAG_SCAN_REQ             = 1,
	UNI_CMD_SCAN_TAG_SCAN_CANCEL          = 2,
	UNI_CMD_SCAN_TAG_SCHED_SCAN_REQ       = 3,
	UNI_CMD_SCAN_TAG_SCHED_SCAN_ENABLE    = 4,
	/** Reserved for Sched Scan / Custom Scan */

	UNI_CMD_SCAN_TAG_SCAN_SSID            = 10,
	UNI_CMD_SCAN_TAG_SCAN_BSSID           = 11,
	UNI_CMD_SCAN_TAG_SCAN_CHANNEL         = 12,
	UNI_CMD_SCAN_TAG_SCAN_IE              = 13,
	UNI_CMD_SCAN_TAG_SCAN_MISC            = 14,
	UNI_CMD_SCAN_TAG_SCAN_SSID_MATCH_SETS = 15,

	UNI_CMD_SCAN_TAG_NUM
};

struct UNI_CMD_SCAN_REQ {
	uint16_t u2Tag;                   /* Tag = 0x01 */
	uint16_t u2Length;

	uint8_t ucScanType;
	uint8_t ucNumProbeReq;
	uint8_t ucScnFuncMask;
	uint8_t aucPadding[1];
	uint16_t u2ChannelMinDwellTime;
	uint16_t u2ChannelDwellTime;
	uint16_t u2TimeoutValue;
	uint16_t u2ProbeDelayTime;
} __KAL_ATTRIB_PACKED__;

struct UNI_CMD_SCAN_CANCEL {
	uint16_t u2Tag;                   /* Tag = 0x02 */
	uint16_t u2Length;

	uint8_t ucIsExtChannel;     /* For P2P channel extension. */
	uint8_t aucReserved[3];
} __KAL_ATTRIB_PACKED__;

struct UNI_CMD_SCAN_SSID {
	uint16_t u2Tag;                   /* Tag = 0x0a */
	uint16_t u2Length;

	uint8_t ucSSIDType;
	uint8_t ucSSIDNum;
	uint8_t aucReserved[2];
	uint8_t aucSsidBuffer[0]; /* PARAM_SSID_T */
} __KAL_ATTRIB_PACKED__;

struct UNI_CMD_SCAN_BSSID {
	uint16_t u2Tag;                   /* Tag = 0x0b */
	uint16_t u2Length;

	uint8_t aucBssid[MAC_ADDR_LEN];
	uint8_t ucBssidMatchCh;
	uint8_t ucBssidMatchSsidInd;

} __KAL_ATTRIB_PACKED__;

struct UNI_CMD_SCAN_CHANNEL_INFO {
	uint16_t u2Tag;                   /* Tag = 0x0c */
	uint16_t u2Length;

	uint8_t ucChannelType;
	uint8_t ucChannelListNum;
	uint8_t aucPadding[2];
	uint8_t aucChnlInfoBuffer[0];

} __KAL_ATTRIB_PACKED__;

struct UNI_CMD_SCAN_IE {
	uint16_t u2Tag;                   /* Tag = 0x0d */
	uint16_t u2Length;

	uint16_t u2IELen;
	uint8_t  aucPadding[2];
	uint8_t  aucIEBuffer[0];  /* depends on u2IELen */

} __KAL_ATTRIB_PACKED__;

struct UNI_CMD_SCAN_MISC {
	uint16_t u2Tag;                   /* Tag = 0x0e */
	uint16_t u2Length;

	uint8_t aucRandomMac[MAC_ADDR_LEN];
	uint8_t ucShortSSIDNum;
	uint8_t aucReserved[1];

} __KAL_ATTRIB_PACKED__;

struct UNI_CMD_SCAN_SCHED_SCAN_REQ {
	uint16_t u2Tag;                   /* Tag = 0x03 */
	uint16_t u2Length;

	uint8_t  ucVersion;
	uint8_t  fgStopAfterIndication;
	uint8_t  ucMspEntryNum;
	uint8_t  ucScnFuncMask;
	uint16_t au2MspList[10];
} __KAL_ATTRIB_PACKED__;

struct UNI_CMD_SCAN_SCHED_SCAN_ENABLE {
	uint16_t u2Tag;                   /* Tag = 0x04 */
	uint16_t u2Length;

	uint8_t  ucSchedScanAct;  /* ENUM_SCHED_SCAN_ACT */
	uint8_t  aucReserved[3];

} __KAL_ATTRIB_PACKED__;

struct UNI_CMD_SCAN_SSID_MATCH_SETS {
	uint16_t u2Tag;                   /* Tag = 0x15 */
	uint16_t u2Length;

	uint8_t  ucMatchSsidNum;
	uint8_t  aucReserved[3];
	uint8_t  aucMatchSsidBuffer[0]; /* SCAN_SCHED_SSID_MATCH_SETS_T */
} __KAL_ATTRIB_PACKED__;

/* Get mac info command (0x1A) */
struct UNI_CMD_GET_MAC_INFO {
	/* fixed field */
	uint8_t ucReserved[4];

	/* tlv */
	uint8_t aucTlvBuffer[0];/**< the TLVs included in this field:
	*
	*   TAG                      | ID  | structure
	*   -------------------------|-----|--------------
	*   UNI_CMD_MAC_INFO_TSF     | 0x0 | UNI_CMD_MAC_INFO_TSF_T
	*/
} __KAL_ATTRIB_PACKED__;

/* Get mac info command TLV List */
enum ENUM_UNI_CMD_MAC_INFO_TAG {
	UNI_CMD_MAC_INFO_TAG_TSF = 0,
	UNI_CMD_MAC_INFO_TAG_NUM
};

/* Get tsf time (Tag0) */
struct UNI_CMD_MAC_INFO_TSF {
	uint16_t u2Tag;
	uint16_t u2Length;
	uint8_t ucDbdcIdx;
	uint8_t ucHwBssidIndex;
	uint8_t aucPadding[2];
} __KAL_ATTRIB_PACKED__;

struct UNI_CMD_PKT_DROP {
	/* fixed field */
	uint8_t ucReserved[4];

	/* tlv */
	uint8_t aucTlvBuffer[0];
/**< the TLVs included in this field:
*
*   TAG                                  | ID   | structure
*   -------------------------------------| ---- | -----------------------------
*   ENUM_UNI_CMD_ID_SET_DROP_PACKET_CFG  | 0x00 | UNI_CMD_SET_DROP_PACKET_PARAM
*/
} __KAL_ATTRIB_PACKED__;

enum ENUM_UNI_CMD_PKT_DROP_TAG {
	UNI_CMD_PKT_DROP_TAG_CFG                    = 1,
	UNI_CMD_PKT_DROP_TAG_NUM
};

struct UNI_CMD_PKT_DROP_SETTING {
	uint16_t u2Tag;
	uint16_t u2Length;
	uint8_t ucMagicCode;
	uint8_t ucCmdBufferLen;    /* buffer length */
	uint8_t aucReserved[24];
} __KAL_ATTRIB_PACKED__;

struct UNI_CMD_CNM {
	/* fixed field */
	uint8_t ucReserved[4];

	/* tlv */
	uint8_t aucTlvBuffer[0];/**< the TLVs included in this field:
*
*   TAG                              | ID  | structure
*   ---------------------------------|-----|--------------
*   UNI_CMD_CNM_CH_PRIVILEGE_REQ     | 0x0 | UNI_CMD_CNM_CH_PRIVILEGE_REQ_T
*   UNI_CMD_CNM_CH_PRIVILEGE_ABORT   | 0x1 | UNI_CMD_CNM_CH_PRIVILEGE_ABORT_T
*   UNI_CMD_CNM_GET_INFO             | 0x2 | UNI_CMD_CNM_GET_INFO_T
*/
} __KAL_ATTRIB_PACKED__;

enum ENUM_UNI_CMD_CNM_TAG {
	UNI_CMD_CNM_TAG_CH_PRIVILEGE_REQ = 0,
	UNI_CMD_CNM_TAG_CH_PRIVILEGE_ABORT = 1,
	UNI_CMD_CNM_TAG_GET_INFO = 2,
	UNI_CMD_CNM_TAG_NUM
};

struct UNI_CMD_CNM_CH_PRIVILEGE_REQ {
	uint16_t        u2Tag;
	uint16_t        u2Length;
	uint8_t         ucBssIndex;
	uint8_t         ucTokenID;
	uint8_t         ucPrimaryChannel;
	uint8_t         ucRfSco;
	uint8_t         ucRfBand;
	uint8_t         ucRfChannelWidth;   /* To support 80/160MHz bandwidth */
	uint8_t         ucRfCenterFreqSeg1; /* To support 80/160MHz bandwidth */
	uint8_t         ucRfCenterFreqSeg2; /* To support 80/160MHz bandwidth */
	uint8_t         ucRfChannelWidthFromAP;
		/* record original 20 /40 /80/160MHz bandwidth from AP's IE */
	uint8_t         ucRfCenterFreqSeg1FromAP;
	uint8_t         ucRfCenterFreqSeg2FromAP;
	uint8_t         ucReqType;          /* ENUM_CH_REQ_TYPE_T */
	uint32_t        u4MaxInterval;      /* In unit of ms */
	uint8_t         ucDBDCBand;
	uint8_t         aucReserved[3];
} __KAL_ATTRIB_PACKED__;

struct UNI_CMD_CNM_CH_PRIVILEGE_ABORT {
	uint16_t         u2Tag;
	uint16_t         u2Length;
	uint8_t          ucBssIndex;
	uint8_t          ucTokenID;
	uint8_t          ucDBDCBand;
	uint8_t          aucReserved[5];
} __KAL_ATTRIB_PACKED__;

struct UNI_CMD_CNM_GET_INFO {
	uint16_t         u2Tag;
	uint16_t         u2Length;
} __KAL_ATTRIB_PACKED__;

struct UNI_CMD_MBMC {
	/* fixed field */
	uint8_t ucReserved[4];

	/* tlv */
	uint8_t aucTlvBuffer[0];
	/**< the TLVs included in this field:
	*
	*   TAG                              | ID  | structure
	*   ---------------------------------|-----|--------------
	*   UNI_CMD_MBMC_TAG_SETTING         | 0x0 | UNI_CMD_MBMC_SETTING
	*/
} __KAL_ATTRIB_PACKED__;

enum ENUM_UNI_CMD_MBMC_TAG {
	UNI_CMD_MBMC_TAG_SETTING = 0,
	UNI_CMD_MBMC_TAG_MAX_NUM
} __KAL_ATTRIB_PACKED__;

struct UNI_CMD_MBMC_SETTING {
	uint16_t u2Tag;
	uint16_t u2Length;
	uint8_t ucMbmcEn;
	uint8_t ucRfBand;
	uint8_t aucReserved[2];
} __KAL_ATTRIB_PACKED__;

struct UNI_CMD_RA {
	/* fixed field */
	uint8_t aucReserved[4];

	/* tlv */
	uint8_t aucTlvBuffer[0]; /**< the TLVs included in this field:
	*
	*   TAG                                        | ID   |
	*   -------------------------  | --   |
	*   UNI_CMD_RA_GET_RU_RA_INFO    | 0x0 |
	*   UNI_CMD_RA_GET_HERA_RELATED_INFO    | 0x1 |
	*   UNI_CMD_RA_GET_MU_RA_INFO           | 0x2 |
	*   UNI_CMD_RA_HERA_STBC_PRIORITY       | 0x3 |
	*   UNI_CMD_RA_OPTION_CTRL              | 0x4 |
	*   UNI_CMD_RA_SET_VHT_1024_QAM     | 0x5 |
	*   UNI_CMD_RA_CFG_PTEC_PER_PPDU        | 0x6 |
	*   UNI_CMD_RA_CFG_MU_INIT_RATE_INTV    | 0x7 |
	*   UNI_CMD_RA__CFG_MU_DIS_SWITCH_SU    | 0x8 |
	*   UNI_CMD_RA_SET_VHT_RATE_FOR_2G      | 0x9 |
	*   UNI_CMD_RA_FIX_RATE_WO_STA_UPDATE   | 0xA |
	*   UNI_CMD_RA_SUPPORT_MCS_CAP_CTRL | 0xB |
	*   UNI_CMD_RA_DBG_CTRL             | 0xC |
	*   UNI_CMD_RA_GET_TX_RATE              | 0xD |
	*   UNI_CMD_RA_SET_MAX_PHY_RATE     | 0xE |
	*   UNI_CMD_RA_SET_FIXED_RATE          | 0xF |
	*   UNI_CMD_RA_SET_FIXED_RATE_UL_TRIG    | 0x10|
	*   UNI_CMD_RA_SET_AUTO_RATE     | 0x11|
	*/
} __KAL_ATTRIB_PACKED__;


/* RA Tag */
enum ENUM_UNI_CMD_RA_TAG {
	UNI_CMD_RA_TAG_GET_RU_RA_INFO = 0x0,
	UNI_CMD_RA_TAG_GET_HERA_RELATED_INFO = 0x01,
	UNI_CMD_RA_TAG_GET_MU_RA_INFO = 0x02,
	UNI_CMD_RA_TAG_HERA_STBC_PRIORITY = 0x03,
	UNI_CMD_RA_TAG_OPTION_CTRL = 0x04,
	UNI_CMD_RA_TAG_SET_VHT_1024_QAM = 0x05,
	UNI_CMD_RA_TAG_CFG_PTEC_PER_PPDU = 0x06,
	UNI_CMD_RA_TAG_CFG_MU_INIT_RATE_INTV = 0x07,
	UNI_CMD_RA_TAG_CFG_MU_DIS_SWITCH_SU = 0x08,
	UNI_CMD_RA_TAG_SET_VHT_RATE_FOR_2G = 0x09,
	UNI_CMD_RA_TAG_FIX_RATE_WO_STA_UPDATE = 0x0A,
	UNI_CMD_RA_TAG_SUPPORT_MCS_CAP_CTRL = 0x0B,
	UNI_CMD_RA_TAG_DBG_CTRL = 0x0C,
	UNI_CMD_RA_TAG_GET_TX_RATE = 0x0D,
	UNI_CMD_RA_TAG_SET_MAX_PHY_RATE = 0x0E,
	UNI_CMD_RA_TAG_SET_FIXED_RATE = 0x0F,
	UNI_CMD_RA_TAG_SET_FIXED_RATE_UL_TRIG = 0x10,
	UNI_CMD_RA_TAG_SET_AUTO_RATE = 0x11,
	UNI_CMD_RA_TAG_NUM
};

enum ENUM_UNI_CMD_RA_FIXED_RATE_VER {
	UNI_CMD_RA_FIXED_RATE_VER1 = 0x0,
	UNI_CMD_RA_FIXED_RATE_VER_MAX_NUM
};

/*RA fixed rate Parameters (Tag 0x0F) */
struct UNI_CMD_RA_SET_FIXED_RATE {
	uint16_t u2Tag;
	uint16_t u2Length;

	/* tag specific part */

	uint16_t u2Version;
	uint8_t  aucBuffer[0];
};

struct UNI_CMD_RA_SET_FIXED_RATE_V1 {
	uint16_t u2WlanIdx;
	uint16_t u2HeLtf;
	uint16_t u2ShortGi;
	uint8_t u1PhyMode;
	uint8_t u1Stbc;
	uint8_t u1Bw;
	uint8_t u1Ecc;
	uint8_t u1Mcs;
	uint8_t u1Nss;
	uint8_t u1Spe;
	uint8_t u1ShortPreamble;
} __KAL_ATTRIB_PACKED__;

struct UNI_CMD_RA_SET_AUTO_RATE {
	uint16_t u2Tag;
	uint16_t u2Length;

	/* tag specific part */
	uint16_t u2WlanIdx;
	uint8_t u1AutoRateEn;
	uint8_t u1Mode;
} __KAL_ATTRIB_PACKED__;

/* BF command (0x33) */
struct UNI_CMD_BF {
	/* fixed field */
	uint8_t aucReserved[4];

	/* tlv */
	uint8_t aucTlvBuffer[0]; /**<the TLVs includer in this field:
*
*  TAG                                 | ID   | structure
*  ------------------------------------| -----| -------------
*  UNI_CMD_BF_SOUNDING_OFF             | 0x0  | N/A
*  UNI_CMD_BF_SOUNDING_ON              | 0x1  | UNI_CMD_BF_SND_T
*  UNI_CMD_BF_DATA_PACKET_APPLY        | 0x2  | UNI_CMD_BF_APPLY_CTRL_T
*  UNI_CMD_BF_PFMU_MEM_ALLOCATE        | 0x3  | UNI_CMD_BF_MEM_ALLOC_CTRL_T
*  UNI_CMD_BF_PFMU_MEM_RELEASE         | 0x4  | UNI_CMD_BF_MEM_ALLOC_CTRL_T
*  UNI_CMD_BF_PFMU_TAG_READ            | 0x5  | UNI_CMD_BF_PFMU_TAG_RW_T
*  UNI_CMD_BF_PFMU_TAG_WRITE           | 0x6  | UNI_CMD_BF_PFMU_TAG_RW_T
*  UNI_CMD_BF_PROFILE_READ             | 0x7  | UNI_CMD_BF_PFMU_DATA_R_T
*  UNI_CMD_BF_PROFILE_WRITE            | 0x8  | UNI_CMD_BF_PFMU_DATA_W_T
*  UNI_CMD_BF_PFMU_MEM_ALLOC_MAP_READ  | 0x9  | N/A
*  UNI_CMD_BF_AID_SET                  | 0xA  | UNI_CMD_PEER_AID_T
*  UNI_CMD_BF_STA_REC_READ             | 0xB  | UNI_CMD_BF_STAREC_READ_T
*  UNI_CMD_BF_PHASE_CALIBRATION        | 0xC  | UNI_CMD_IBF_PHASE_CAL_CTRL_T
*  UNI_CMD_BF_IBF_PHASE_COMP           | 0xD  | UNI_CMD_IBF_PHASE_COMP_T
*  UNI_CMD_BF_LNA_GAIN_CONFIG          | 0xE  | UNI_CMD_IBF_LNA_GAIN_T
*  UNI_CMD_BF_PROFILE_WRITE_20M_ALL    | 0xF  | UNI_CMD_BF_PFMU_DATA_ALL_W_T
*  UNI_CMD_BF_APCLIENT_CLUSTER         | 0x10 | UNI_CMD_APCLIENT_BF_T
*  UNI_CMD_BF_HW_ENABLE_STATUS_UPDATE  | 0x11 |
*					   UNI_CMD_BF_HW_ENABLE_STATUS_UPDATE_T
*  UNI_CMD_BF_BFEE_HW_CTRL             | 0x12 | UNI_CMD_BF_BFEE_CTRL_T
*  UNI_CMD_BF_PFMU_SW_TAG_WRITE        | 0x13 | UNI_CMD_ETXBF_PFMU_SW_TAG_T
*  UNI_CMD_BF_MOD_EN_CTRL              | 0x14 | UNI_CMD_BF_MOD_EN_CTRL_T
*  UNI_CMD_BF_CONFIG                   | 0x15 | UNI_CMD_BF_CONFIG_T
*  UNI_CMD_BF_PFMU_DATA_WRITE          | 0x16 |
*                                          UNI_CMD_ETXBf_PFMU_FULL_DIM_DATA_W_T
*  UNI_CMD_BF_FBRPT_DBG_INFO_READ      | 0x17 | UNI_CMD_TXBF_FBRPT_DBG_INFO_T
*  UNI_CMD_BF_CMD_TXSND_INFO           | 0x18 | UNI_CMD_BF_SND_CMD_T
*  UNI_CMD_BF_CMD_PLY_INFO             | 0x19 | UNI_CMD_BF_PLY_CMD_T
*  UNI_CMD_BF_CMD_MU_METRIC            | 0x1A | UNI_CMD_HERA_MU_METRIC_T
*  UNI_CMD_BF_CMD_TXCMD                | 0x1B | UNI_CMD_BF_TXCMD_CMD_T
*  UNI_CMD_BF_CMD_CFG_PHY              | 0x1C | UNI_CMD_BF_CFG_BF_PHY_T
*  UNI_CMD_BF_CMD_SND_CNT              | 0x1D | UNI_CMD_BF_SND_CNT_CMD_T
*/
} __KAL_ATTRIB_PACKED__;

/* BF cmd tags */
enum ENUM_UNI_CMD_BF_TAG {
	UNI_CMD_BF_TAG_SOUNDING_OFF = 0x00,
	UNI_CMD_BF_TAG_SOUNDING_ON = 0x01,
	UNI_CMD_BF_TAG_DATA_PACKET_APPLY = 0x02,
	UNI_CMD_BF_TAG_PFMU_MEM_ALLOCATE = 0x03,
	UNI_CMD_BF_TAG_PFMU_MEM_RELEASE = 0x04,
	UNI_CMD_BF_TAG_PFMU_TAG_READ = 0x05,
	UNI_CMD_BF_TAG_PFMU_TAG_WRITE = 0x06,
	UNI_CMD_BF_TAG_PROFILE_READ = 0x7,
	UNI_CMD_BF_TAG_PROFILE_WRITE = 0x8,
	UNI_CMD_BF_TAG_PFMU_MEM_ALLOC_MAP_READ = 0x09,
	UNI_CMD_BF_TAG_AID_SET = 0x0a,
	UNI_CMD_BF_TAG_STA_REC_READ = 0x0b,
	UNI_CMD_BF_TAG_PHASE_CALIBRATION = 0x0c,
	UNI_CMD_BF_TAG_IBF_PHASE_COMP = 0x0d,
	UNI_CMD_BF_TAG_LNA_GAIN_CONFIG = 0x0e,
	UNI_CMD_BF_TAG_PROFILE_WRITE_20M_ALL = 0x0f,
	UNI_CMD_BF_TAG_APCLIENT_CLUSTER = 0x10,
	UNI_CMD_BF_TAG_HW_ENABLE_STATUS_UPDATE = 0x11,
	UNI_CMD_BF_TAG_BFEE_HW_CTRL = 0x12,
	UNI_CMD_BF_TAG_PFMU_SW_TAG_WRITE = 0x13,
	UNI_CMD_BF_TAG_MOD_EN_CTRL = 0x14,
	UNI_CMD_BF_TAG_CONFIG = 0x15,
	UNI_CMD_BF_TAG_PFMU_DATA_WRITE = 0x16,
	UNI_CMD_BF_TAG_FBRPT_DBG_INFO_READ = 0x17,
	UNI_CMD_BF_TAG_CMD_TXSND_INFO = 0x18,
	UNI_CMD_BF_TAG_CMD_PLY_INFO = 0x19,
	UNI_CMD_BF_TAG_CMD_MU_METRIC = 0x1a,
	UNI_CMD_BF_TAG_CMD_TXCMD = 0x1b,
	UNI_CMD_BF_TAG_CMD_CFG_PHY = 0x1c,
	UNI_CMD_BF_TAG_CMD_SND_CNT = 0x1d,
	UNI_CMD_BF_TAG_CMD_NUM
};

struct UNI_CMD_BF_SND {
	uint16_t u2Tag;
	uint16_t u2Length;
	uint8_t u1SuMuSndMode;
	uint8_t u1StaNum;
	uint8_t au1Reserved[2];
	uint16_t u2WlanId[4];
	uint32_t u4SndIntv;
} __KAL_ATTRIB_PACKED__;

/* BF read BF StaRec (Tag11) */
struct UNI_CMD_BF_STAREC_READ {
	uint16_t u2Tag;
	uint16_t u2Length;
	uint16_t u2WlanIdx;
	uint8_t  au1Reserved[2];
} __KAL_ATTRIB_PACKED__;

struct UNI_CMD_MQM_UPDATE_MU_EDCA {
	/*fixed field*/
	uint8_t aucReserved[4];

	/* tlv */
	uint8_t aucTlvBuffer[0]; /**< the TLVs included in this field:
	*
	*   TAG                              | ID  | structure
	*   ---------------------------------| ----| -------------
	*   UNI_CMD_MQM_UPDATE_MU_EDCA_PARMS | 0x0 |UNI_CMD_MQM_UPDATE_MU_EDCA_T
	*/
} __KAL_ATTRIB_PACKED__;

enum ENUM_UNI_CMD_MQM_UPDATE_MU_EDCA_TAG {
	UNI_CMD_MQM_UPDATE_MU_EDCA_TAG_PARMS = 0x0,
	UNI_CMD_MQM_UPDATE_MU_EDCA_TAG_NUM
} __KAL_ATTRIB_PACKED__;

struct UNI_CMD_MU_EDCA_PARAMS {
	uint8_t ucECWmin;
	uint8_t ucECWmax;
	uint8_t ucAifsn;
	uint8_t ucIsACMSet;
	uint8_t ucMUEdcaTimer;
	uint8_t aucPadding[3];
} __KAL_ATTRIB_PACKED__;

struct UNI_CMD_UPDATE_MU_EDCA {
	uint16_t u2Tag;
	uint16_t u2Length;

	/* tag specific part */
	uint8_t  ucBssIndex;
	uint8_t  fgIsQBSS;
	uint8_t  ucWmmSet;
	uint8_t  ucReserved;
	struct UNI_CMD_MU_EDCA_PARAMS arMUEdcaParams[4];
} __KAL_ATTRIB_PACKED__;

struct UNI_CMD_ID_FRM_IND_FROM_HOST {
	/*fixed field*/
	uint8_t aucPadding[4];
	/* tlv */
	uint8_t aucTlvBuffer[0]; /**< the TLVs included in this field:
    *
    *  TAG                          | ID   | structure
    *  -------------                | -----| -------------
    *  UNI_CMD_ID_FRM_IND_FROM_HOST | 0x0  | UNI_CMD_ID_FRM_IND_FROM_HOST_PARM_T
    */
} __KAL_ATTRIB_PACKED__;

/* RDD set command Tag */
enum ENUM_UNI_CMD_FRM_IND_FROM_HOST_TAG {
	UNI_CMD_FRM_IND_FROM_HOST_TAG_PARM = 0,
	UNI_CMD_FRM_IND_FROM_HOST_TAG_NUM
};

struct UNI_CMD_FRM_IND_FROM_HOST_PARM {
	uint8_t  ucCmdVer;
	uint8_t  aucPadding0[1];
	uint16_t u2CmdLen;       /* cmd size including common part and body. */
	uint8_t  ucStaIdx;
	uint8_t  ucBssIdx;
	uint8_t  ucTransmitType;
	uint8_t  ucProtocolType;
	uint8_t  ucProtocolSubType;
	uint8_t  ucRateValid;
	uint8_t  aucPadding1[2];
	uint32_t u4Rate;
	uint32_t u4Len;
	uint8_t  TxS;
	uint8_t  aucPadding3[3];
	uint8_t  aucPadding4[64];
} __KAL_ATTRIB_PACKED__;

/*******************************************************************************
 *                                 Event
 *******************************************************************************
 */

struct WIFI_UNI_EVENT {
	uint16_t u2PacketLength;
	uint16_t u2PacketType;	/* Must be filled with 0xE000 (EVENT Packet) */

	uint8_t ucEID;
	uint8_t ucSeqNum;
	uint8_t ucOption;
	uint8_t aucReserved[1];

	uint8_t ucExtenEID;
	uint8_t aucReserved2[2];
	uint8_t ucS2DIndex;

	uint8_t aucBuffer[0];
};

/*
 * TLV element structure should start with a 2-byte Tag field and a 2-byte
 * length field and pad to 4-byte alignment. The u2Length field indicate
 * the length of the whole TLV including tag and length field.
 */
struct TAG_HDR {
	uint16_t u2Tag;
	uint16_t u2Length;
	uint8_t aucBuffer[0];
};

enum ENUM_UNI_EVENT_ID {
	UNI_EVENT_ID_CMD_RESULT      = 0x01,
				/* Generic event for return cmd status */
	UNI_EVENT_ID_BMC_RPY_DT      = 0x02,
	UNI_EVENT_ID_HIF_CTRL	     = 0x03,
	UNI_EVENT_ID_FW_LOG_2_HOST   = 0x04,
	UNI_EVENT_ID_ROAMING	      = 0x05,
	UNI_EVENT_ID_ACCESS_REG      = 0x06,
	UNI_EVENT_ID_CHIP_CONFIG     = 0x07,
	UNI_EVENT_ID_SMESH_INFO      = 0x08,
	UNI_EVENT_ID_IE_COUNTDOWN    = 0x09,
	UNI_EVENT_ID_ASSERT_DUMP     = 0x0A,
	UNI_EVENT_ID_SLEEP_NOTIFY    = 0x0b,
	UNI_EVENT_ID_BEACON_TIMEOUT  = 0x0C,
	UNI_EVENT_ID_PS_SYNC	     = 0x0D,
	UNI_EVENT_ID_SCAN_DONE	     = 0x0E,
	UNI_EVENT_ID_ECC_CAL	     = 0x10,
	UNI_EVENT_ID_RDD	     = 0x11,
	UNI_EVENT_ID_ADD_KEY_DONE    = 0x12,
	UNI_EVENT_ID_OBSS_UPDATE     = 0x13,
	UNI_EVENT_ID_SER	     = 0x14,
	UNI_EVENT_ID_IDC	     = 0x17,
	UNI_EVENT_ID_MAC_INFO	     = 0x1A,
	UNI_EVENT_ID_TDLS	     = 0x1B,
	UNI_EVENT_ID_SAP	     = 0x1C,
	UNI_EVENT_ID_TXCMD_CTRL      = 0x1D,
	UNI_EVENT_ID_P2P	     = 0x1F,
	UNI_EVENT_ID_EDCCA	     = 0x21,
	UNI_EVENT_ID_MIB	     = 0x22,
	UNI_EVENT_ID_STATISTICS      = 0x23,
	UNI_EVENT_ID_SR	     = 0x25,
	UNI_EVENT_ID_SCS	     = 0x26,
	UNI_EVENT_ID_CNM	     = 0x27,
	UNI_EVENT_ID_MBMC	     = 0x28,
	UNI_EVENT_ID_BSS_IS_ABSENCE  = 0x29,
	UNI_EVENT_ID_TXPOWER	     = 0x2A,
	UNI_EVENT_ID_WSYS_CONFIG     = 0x2B,
	UNI_EVENT_ID_BA_OFFLOAD      = 0x2C,
	UNI_EVENT_ID_STATUS_TO_HOST  = 0x2D,
	UNI_EVENT_ID_RA	     = 0x2F,
	UNI_EVENT_ID_TESTMODE_RX_STAT_INFO  = 0x32,
	UNI_EVENT_ID_BF	     = 0X33,
	UNI_EVENT_ID_SDVT_STAT	     = 0x34,
	UNI_EVENT_ID_VOW	     = 0x37,
	UNI_EVENT_ID_TPC	     = 0x38,
	UNI_EVENT_ID_MEC	     = 0x3A,
	UNI_EVENT_ID_RSSI_MONITOR    = 0x41,
	UNI_EVENT_ID_TEST_TR_PARAM   = 0x42,
	UNI_EVENT_ID_CHIP_CAPABILITY = 0x43,
	UNI_EVENT_ID_UPDATE_COEX_PHYRATE = 0x44,

	UNI_EVENT_ID_NUM
};

struct UNI_EVENT_CMD_RESULT {
	uint16_t u2CID;
	uint8_t aucReserved[2];
	uint32_t u4Status;
} __KAL_ATTRIB_PACKED__;

struct UNI_EVENT_ACCESS_REG {
	/* fixed field */
	uint8_t ucReserved[4];

	/* tlv */
	uint8_t aucTlvBuffer[0];/**< the TLVs included in this field:
	*
	*   TAG                           | ID | structure
	*   ------------------------------|----|--------------------------------
	*   UNI_EVENT_ACCESS_REG_BASIC    | 0x0| UNI_EVENT_ACCESS_REG_BASIC_T
	*   UNI_EVENT_ACCESS_RF_REG_BASIC | 0x1| UNI_EVENT_ACCESS_RF_REG_BASIC_T
	*/
} __KAL_ATTRIB_PACKED__;

/* Register access event Tag */
enum ENUM_UNI_EVENT_ACCESS_REG_TAG {
	UNI_EVENT_ACCESS_REG_TAG_BASIC = 0,
	UNI_EVENT_ACCESS_REG_TAG_RF_REG_BASIC,
	UNI_EVENT_ACCESS_REG_TAG_NUM
} __KAL_ATTRIB_PACKED__;

/* Access Register (Tag0) */
struct UNI_EVENT_ACCESS_REG_BASIC {
	uint16_t u2Tag;
	uint16_t u2Length;
	uint32_t u4Addr;
	uint32_t u4Value;
} __KAL_ATTRIB_PACKED__;

/* Access RF address (Tag1) */
struct UNI_EVENT_ACCESS_RF_REG_BASIC {
	uint16_t u2Tag;
	uint16_t u2Length;
	uint16_t u2WifiStream;
	uint16_t u2Reserved;
	uint32_t u4Addr;
	uint32_t u4Value;
} __KAL_ATTRIB_PACKED__;

struct UNI_EVENT_CHIP_CONFIG {
	/* fixed field */
	uint16_t u2TotalElementNum;
	uint8_t aucPadding[2];
	/* tlv */

	uint8_t aucTlvBuffer[0];
} __KAL_ATTRIB_PACKED__;

enum ENUM_UNI_EVENT_CHIP_CONFIG_TAG {
	UNI_EVENT_CHIP_CONFIG_TAG_SW_DBG_CTRL,
	UNI_EVENT_CHIP_CONFIG_TAG_CUSTOMER_CFG,
	UNI_EVENT_CHIP_CONFIG_TAG_CHIP_CFG,
	UNI_EVENT_CHIP_CONFIG_TAG_NUM
};

struct UNI_EVENT_SLEEP_NOTIFY {
	/*fixed field*/
	uint8_t ucBssIndex;
	uint8_t aucPadding[3];

	/* tlv */
	uint8_t aucTlvBuffer[0]; /**< the TLVs included in this field:
	*
	*   TAG                             | ID  | structure
	*   -------------                   | ----| -------------
	*   UNI_EVENT_SLEEP_INFO          | 0x0 | UNI_EVENT_SLEEP_INFO_T
	*/
} __KAL_ATTRIB_PACKED__;

/* Sleep Notify event Tag */
enum ENUM_UNI_EVENT_SLEEP_NOTYFY_TAG {
	UNI_EVENT_SLEEP_NOTYFY_TAG_SLEEP_INFO = 0,
	UNI_EVENT_SLEEP_NOTYFY_TAG_NUM
};

struct UNI_EVENT_SLEEP_INFO {
	uint16_t u2Tag;                   /* Tag = 0x00 */
	uint16_t u2Length;

	uint8_t ucSleepyState;
	uint8_t aucPadding[3];
} __KAL_ATTRIB_PACKED__;

struct UNI_EVENT_BEACON_TIMEOUT {
	/* fixed field */
	uint8_t ucBssIndex;
	uint8_t aucPadding[3];

	/* tlv */
	uint8_t aucTlvBuffer[0];
} __KAL_ATTRIB_PACKED__;

/* Beacon Timeout event Tag */
enum ENUM_UNI_EVENT_BEACON_TIMEOUT_TAG {
	UNI_EVENT_BEACON_TIMEOUT_TAG_INFO = 0,
	UNI_EVENT_BEACON_TIMEOUT_TAG_NUM
};

struct UNI_EVENT_BEACON_TIMEOUT_INFO {
	uint16_t u2Tag;    /* Tag = 0x00 */
	uint16_t u2Length;
	uint8_t ucReasonCode;
	uint8_t aucPadding[3];
} __KAL_ATTRIB_PACKED__;

struct UNI_EVENT_PS_SYNC {
	/*fixed field*/
	uint8_t ucBssIndex;
	uint8_t aucPadding[3];

	/* tlv */
	uint8_t aucTlvBuffer[0]; /**< the TLVs included in this field:
	*
	*   TAG                             | ID  | structure
	*   -------------                   | ----| -------------
	*   UNI_EVENT_CLIENT_PS_INFO          | 0x0 | UNI_EVENT_CLIENT_PS_INFO_T
	*/
} __KAL_ATTRIB_PACKED__;

enum ENUM_UNI_EVENT_PS_SYNC_TAG {
	UNI_EVENT_PS_SYNC_TAG_CLIENT_PS_INFO = 0,
	UNI_EVENT_PS_SYNC_TAG_NUM
};

/* PS SYNC (Tag0) */
struct UNI_EVENT_CLIENT_PS_INFO {
	uint16_t u2Tag;
	uint16_t u2Length;

	uint8_t ucPsBit;
	uint8_t aucPadding[1];
	uint16_t ucWtblIndex;
	uint8_t ucBufferSize;
	uint8_t aucReserved[3];
} __KAL_ATTRIB_PACKED__;

struct UNI_EVENT_SCAN_DONE {
	/* fixed field */
	uint8_t ucSeqNum;
	uint8_t aucPadding[3];

	/* tlv */
	uint8_t aucTlvBuffer[0];
} __KAL_ATTRIB_PACKED__;

enum ENUM_UNI_EVENT_SCAN_DONE_TAG {
	UNI_EVENT_SCAN_DONE_TAG_BASIC      = 0,
	UNI_EVENT_SCAN_DONE_TAG_SPARSECHNL = 1,
	UNI_EVENT_SCAN_DONE_TAG_CHNLINFO   = 2,
	UNI_EVENT_SCAN_DONE_TAG_NLO        = 3,
	UNI_EVENT_SCAN_DONE_TAG_NUM
};

struct UNI_EVENT_SCAN_DONE_BASIC {
	uint16_t u2Tag;    /* Tag = 0x00 */
	uint16_t u2Length;

	uint8_t  ucCompleteChanCount;
	uint8_t  ucCurrentState;
	uint8_t  ucScanDoneVersion;
	uint8_t  fgIsPNOenabled;
	uint32_t u4ScanDurBcnCnt;
} __KAL_ATTRIB_PACKED__;

struct UNI_EVENT_SCAN_DONE_SPARSECHNL {
	uint16_t u2Tag;    /* Tag = 0x01 */
	uint16_t u2Length;

	uint8_t ucSparseChannelValid;
	uint8_t ucBand;
	uint8_t ucChannelNum;
	uint8_t ucSparseChannelArrayValidNum;
} __KAL_ATTRIB_PACKED__;

struct UNI_EVENT_SCAN_DONE_CHNLINFO {
	uint16_t u2Tag;    /* Tag = 0x02 */
	uint16_t u2Length;

	uint8_t ucNumOfChnl;
	uint8_t aucReserved[3];
	uint8_t aucChnlInfoBuffer[0];
} __KAL_ATTRIB_PACKED__;

struct UNI_EVENT_CHNLINFO {
	uint8_t ucChannelNum;
	uint8_t ucChannelBAndPCnt;
	uint8_t ucChannelMDRDYCnt;
	uint8_t aucPadding[1];
	uint16_t u2ChannelIdleTime;
	uint16_t u2ChannelScanTime;
} __KAL_ATTRIB_PACKED__;

struct UNI_EVENT_SCAN_DONE_NLO {
	uint16_t u2Tag;    /* Tag = 0x03 */
	uint16_t u2Length;

	uint8_t  ucStatus;
	uint8_t  aucReserved[3];
} __KAL_ATTRIB_PACKED__;

struct UNI_EVENT_IDC {
	/* fixed field */
	uint8_t aucPadding[4];

	/* tlv */
	uint8_t aucTlvBuffer[0];/**< the TLVs included in this field:
	*   TAG                             | ID  | structure
	*   -------------                   | ----| -------------
	*   UNI_EVENT_MD_SAFE_CHN   | 0x0 | UNI_EVENT_MD_SAFE_CHN_T
	*   UNI_EVENT_CCCI_MSG         | 0x1 | UNI_EVENT_CCCI_MSG_T
	*
	*/
} __KAL_ATTRIB_PACKED__;

enum ENUM_UNI_EVENT_IDC_TAG {
	UNI_EVENT_IDC_TAG_MD_SAFE_CHN = 0,
	UNI_EVENT_IDC_TAG_CCCI_MSG = 1,
	UNI_EVENT_IDC_TAG_NUM
};

struct UNI_EVENT_MD_SAFE_CHN {
	uint16_t u2Tag;
	uint16_t u2Length;
	uint8_t ucVersion;
	uint8_t aucReserved1[3];    /* 4 byte alignment */
	uint32_t u4Flags;            /* Bit0: valid */
	uint32_t u4SafeChannelBitmask[UNI_WIFI_CH_MASK_IDX_NUM];
					/* WIFI_CH_MASK_IDX_NUM = 4 */
} __KAL_ATTRIB_PACKED__;

struct UNI_EVENT_MAC_IFNO {
	/* fixed field */
	uint8_t aucReserved[4];

	/* tlv */
	uint8_t aucTlvBuffer[0];
} __KAL_ATTRIB_PACKED__;

/* Mac info event Tag */
enum ENUM_UNI_EVENT_MAC_INFO_TAG {
	UNI_EVENT_MAC_INFO_TAG_TSF  = 0,
	UNI_EVENT_MAC_INFO_TAG_NUM
};

/* Beacon timeout reason (Tag0) */
struct UNI_EVENT_MAC_INFO_TSF {
	uint16_t   u2Tag;    /* Tag = 0x00 */
	uint16_t   u2Length;
	uint8_t    ucDbdcIdx;
	uint8_t    ucHwBssidIndex;
	uint8_t    aucPadding[2];
	uint32_t   u4TsfBit0_31;
	uint32_t   u4TsfBit63_32;
} __KAL_ATTRIB_PACKED__;

struct UNI_EVENT_SAP {
	/* fixed field */
	uint8_t aucPadding[4];

	/* tlv */
	uint8_t aucTlvBuffer[0];
} __KAL_ATTRIB_PACKED__;

/* SAP event Tag */
enum ENUM_UNI_EVENT_SAP_TAG {
	UNI_EVENT_SAP_TAG_AGING_TIMEOUT = 0,
	UNI_EVENT_SAP_TAG_UPDATE_STA_FREE_QUOTA = 1,
	UNI_EVENT_SAP_TAG_NUM
};

struct UNI_EVENT_SAP_AGING_TIMEOUT {
	uint16_t u2Tag;    /* Tag = 0x00 */
	uint16_t u2Length;
	uint16_t u2StaRecIdx;
	uint8_t aucPadding[2];
} __KAL_ATTRIB_PACKED__;

struct UNI_EVENT_UPDATE_STA_FREE_QUOTA {
	uint16_t u2Tag;
	uint16_t u2Length;
	uint16_t u2StaRecIdx;
	uint8_t  ucUpdateMode;
	uint8_t  ucFreeQuota;
} __KAL_ATTRIB_PACKED__;

struct UNI_EVENT_CNM {
	/* fixed field */
	uint8_t aucPadding[4];

	/* tlv */
	uint8_t aucTlvBuffer[0];/**< the TLVs included in this field:
*
*   TAG                              | ID  | structure
*   ---------------------------------|-----|--------------
*   UNI_EVENT_CNM_CH_PRIVILEGE_GRANT | 0x0 | UNI_EVENT_CH_PRIVILEGE_GRANT_T
*   UNI_EVENT_CNM_GET_CHANNEL_INFO   | 0x1 | UNI_EVENT_CNM_GET_CHANNEL_INFO_T
*   UNI_EVENT_CNM_GET_BSS_INFO       | 0x2 | UNI_EVENT_CNM_GET_BSS_INFO_T
*   UNI_EVENT_CNM_OPMODE_CHANGE      | 0x3 | UNI_EVENT_CNM_OPMODE_CHANGE_T
*/
} __KAL_ATTRIB_PACKED__;

/** channel privilege command TLV List */
enum ENUM_UNI_EVENT_CNM_TAG {
	UNI_EVENT_CNM_TAG_CH_PRIVILEGE_GRANT = 0,
	UNI_EVENT_CNM_TAG_GET_CHANNEL_INFO = 1,
	UNI_EVENT_CNM_TAG_GET_BSS_INFO = 2,
	UNI_EVENT_CNM_TAG_OPMODE_CHANGE = 3,
	UNI_EVENT_CNM_TAG_NUM
} __KAL_ATTRIB_PACKED__;

struct UNI_EVENT_CNM_CH_PRIVILEGE_GRANT {
	uint16_t         u2Tag;
	uint16_t         u2Length;
	uint8_t          ucBssIndex;
	uint8_t          ucTokenID;
	uint8_t          ucStatus;
	uint8_t          ucPrimaryChannel;
	uint8_t          ucRfSco;
	uint8_t          ucRfBand;
	uint8_t          ucRfChannelWidth;   /* To support 80/160MHz bandwidth*/
	uint8_t          ucRfCenterFreqSeg1; /* To support 80/160MHz bandwidth*/
	uint8_t          ucRfCenterFreqSeg2; /* To support 80/160MHz bandwidth*/
	uint8_t          ucReqType;
	uint8_t          ucDBDCBand;         /* ENUM_CMD_REQ_DBDC_BAND_T */
	uint8_t          aucReserved[1];
	uint32_t         u4GrantInterval;    /* In unit of ms */
} __KAL_ATTRIB_PACKED__;

struct UNI_EVENT_CNM_GET_CHANNEL_INFO {
	uint16_t     u2Tag;
	uint16_t     u2Length;
	uint8_t      ucDBDCBand;
	uint8_t      fgIsCnmTimelineEnabled;
	uint8_t      ucOpChNum;
	uint8_t      aucReserved[1];

	uint8_t      aucChnlInfo[0];
} __KAL_ATTRIB_PACKED__;

struct UNI_EVENT_CNM_GET_BSS_INFO {
	uint16_t     u2Tag;
	uint16_t     u2Length;
	uint8_t      ucBssNum;
	uint8_t      aucReserved[3];

	uint8_t      aucBssInfo[0];
} __KAL_ATTRIB_PACKED__;

struct UNI_EVENT_CNM_CHANNEL_INFO {
	uint8_t      ucPriChannel;
	uint8_t      ucChBw;
	uint8_t      ucChSco;
	uint8_t      ucChannelS1;
	uint8_t      ucChannelS2;
	uint8_t      ucChBssNum;
	uint16_t     u2ChBssBitmapList;
} __KAL_ATTRIB_PACKED__;

struct UNI_EVENT_CNM_BSS_INFO {
	uint8_t      ucBssType;
	uint8_t      ucBssInuse;
	uint8_t      ucBssActive;
	uint8_t      ucBssConnectState;
	uint8_t      ucBssPriChannel;
	uint8_t      ucBssDBDCBand;
	uint8_t      ucBssOMACIndex;
	uint8_t      ucBssOMACDBDCBand;
} __KAL_ATTRIB_PACKED__;

struct UNI_EVENT_CNM_OPMODE_CHANGE {
	uint16_t         u2Tag;
	uint16_t         u2Length;
	uint16_t         u2BssBitmap;
	uint8_t          ucEnable;
	uint8_t          ucOpTxNss;
	uint8_t          ucOpRxNss;
	uint8_t          ucReason;
	uint8_t          aucReserved[2];
} __KAL_ATTRIB_PACKED__;

struct UNI_EVENT_MBMC {
	/* fixed field */
	uint8_t ucReserved[4];

	/* tlv */
	uint8_t aucTlvBuffer[0];

	/**< the TLVs included in this field:
	*
	*   TAG                              | ID  | structure
	*   ---------------------------------|-----|--------------
	*   UNI_EVENT_MBMC_TAG_SWITCH_DONE   | 0x0 | UNI_EVENT_MBMC_SWITCH_DONE
	*/
} __KAL_ATTRIB_PACKED__;

/** MBMC Event TLV List */
enum ENUM_UNI_EVENT_MBMC_TAG {
	UNI_EVENT_MBMC_TAG_SWITCH_DONE = 0,
	UNI_EVENT_MBMC_TAG_NUM
} __KAL_ATTRIB_PACKED__;

struct UNI_EVENT_MBMC_SWITCH_DONE {
	uint16_t u2Tag;
	uint16_t u2Length;
	uint8_t  aucReserved[4];
} __KAL_ATTRIB_PACKED__;

struct UNI_EVENT_BSS_IS_ABSENCE {
	/* fixed field */
	uint8_t ucBssIndex;
	uint8_t aucPadding[3];

	/* tlv */
	uint8_t aucTlvBuffer[0];
} __KAL_ATTRIB_PACKED__;

/* BSS Absence or Presence Event Tag */
enum ENUM_UNI_EVENT_BSS_IS_ABSENCE_TAG {
	UNI_EVENT_BSS_IS_ABSENCE_TAG_INFO = 0,
	UNI_EVENT_BSS_IS_ABSENCE_TAG_NUM
};

struct UNI_EVENT_BSS_IS_ABSENCE_INFO {
	uint16_t     u2Tag;
	uint16_t     u2Length;
	/* Event Body */
	uint8_t	    ucIsAbsent;
	uint8_t	    ucBssFreeQuota;
	uint8_t	    aucReserved[2];
} __KAL_ATTRIB_PACKED__;

struct UNI_EVENT_STATUS_TO_HOST {
	/* fixed field */
	uint8_t aucPadding[4];

	/* tlv */
	uint8_t aucTlvBuffer[0];
} __KAL_ATTRIB_PACKED__;

struct UNI_EVENT_BA_OFFLOAD {
	/* fixed field */
	uint8_t aucPadding[4];

	/* tlv */
	uint8_t aucTlvBuffer[0];
} __KAL_ATTRIB_PACKED__;

enum ENUM_UNI_EVENT_BA_OFFLOAD_TAG {
	UNI_EVENT_BA_OFFLOAD_TAG_RX_ADDBA  = 0,
	UNI_EVENT_BA_OFFLOAD_TAG_RX_DELBA  = 1,
	UNI_EVENT_BA_OFFLOAD_TAG_TX_ADDBA  = 2,
	UNI_EVENT_BA_OFFLOAD_TAG_NUM
};

struct UNI_EVENT_RX_ADDBA {
	uint16_t u2Tag;
	uint16_t u2Length;
	uint16_t u2WlanIdx;
	uint16_t u2WinSize;
	uint16_t u2BATimeoutValue;
	uint16_t u2BAStartSeqCtrl;
	uint8_t	ucDialogToken;
	uint8_t	ucTid;
	uint8_t	aucReserved[2];
} __KAL_ATTRIB_PACKED__;

struct UNI_EVENT_RX_DELBA {
	uint16_t u2Tag;
	uint16_t u2Length;
	uint16_t u2WlanIdx;
	uint8_t	ucTid;
	uint8_t	aucReserved[1];
} __KAL_ATTRIB_PACKED__;

struct UNI_EVENT_TX_ADDBA {
	uint16_t u2Tag;
	uint16_t u2Length;
	uint16_t u2WlanIdx;
	uint16_t u2WinSize;
	uint16_t u2SSN;
	uint8_t	ucTid;
	/* AMSDU in AMPDU is enabled or not (TID bitmap)*/
	uint8_t	ucAmsduEnBitmap;
	/* Max AMSDU length */
	uint8_t u4MaxMpduLen;
	/*
	 * Note: If length of a packet < u4MinMpduLen then it shall not be
	 * aggregated in an AMSDU
	 */
	/* Min MPDU length to be AMSDU */
	uint8_t u4MinMpduLen;
	/*
	 * AMSDU count/length limits by count *OR* length
	 * Count: MPDU count in an AMSDU shall not exceed ucMaxMpduCount
	 * Length: AMSDU length shall not exceed u4MaxMpduLen
	 */
	/* Max MPDU count in an AMSDU */
	uint8_t	ucMaxMpduCount;
	uint8_t	aucReserved[15];
} __KAL_ATTRIB_PACKED__;

/* status to host event tag */
enum ENUM_UNI_EVENT_STATUS_TO_HOST_TAG {
	UNI_EVENT_STATUS_TO_HOST_TAG_TX_DONE  = 0,
	UNI_EVENT_STATUS_TO_HOST_TAG_NUM
};

struct UNI_EVENT_TX_DONE {
	uint16_t     u2Tag;
	uint16_t     u2Length;

	/** HW PID */
	uint8_t      ucPacketSeq;
	/** TX_RESULT_xx */
	uint8_t      ucStatus;
	/** Packet sequence number */
	uint16_t     u2SequenceNumber;

	/** WLAN index (WTBL) */
	uint8_t      ucWlanIndex;
	/** TX count (includes retry count) */
	uint8_t      ucTxCount;
	/** TX success rate (see right page) */
	uint16_t     u2TxRate;

	/** TXS_WITH_ADVANCED_INFO or TXS_IS_EXIST */
	uint8_t      ucFlag;
	/** Packet TID */
	uint8_t      ucTid;
	/** Response rate */
	uint8_t      ucRspRate;
	/** The last transmission rate index from WLAN table */
	uint8_t      ucRateTableIdx;

	/** TX Bandwidth, the bandwidth used to transmit this PPDU */
	uint8_t      ucBandwidth;
	uint8_t      ucTxPower; /** TX Power dBm */
	/** (SW) the packet is flushed to tx done, here is the reason */
	uint8_t      ucFlushReason;
	uint8_t      aucReserved0[1];

	/** Unit is 32us. Transmitted by UMAC till UMAC gets tx status */
	uint32_t     u4TxDelay;
	/** local TSF when 1st bit of MAC header TX from MAC to PHY */
	uint32_t     u4Timestamp;
	/** (SW) Reference to _ENUM_TXS_APPLIED_FLAG_T */
	uint32_t     u4AppliedFlag;
} __KAL_ATTRIB_PACKED__;

/* BF event (0x33) */
struct UNI_EVENT_BF {
	/* fixed field */
	uint8_t au1Reserved[4];

	/* tlv */
	uint8_t au1TlvBuffer[0];
} __KAL_ATTRIB_PACKED__;

/* BF event tags */
enum ENUM_UNI_EVENT_BF_TAG {
	UNI_EVENT_BF_TAG_PFMU = 0x5,
	UNI_EVENT_BF_TAG_PFMU_DATA = 0x7,
	UNI_EVENT_BF_TAG_PFMU_MEM_ALLOC_MAP = 0x9,
	UNI_EVENT_BF_TAG_STAREC = 0xB,
	UNI_EVENT_BF_TAG_CAL_PHASE = 0xC,
	UNI_EVENT_BF_TAG_FBK_INFO = 0x17,
	UNI_EVENT_BF_TAG_TXSND_INFO = 0x18,
	UNI_EVENT_BF_TAG_PLY_INFO = 0x19,
	UNI_EVENT_BF_TAG_METRIC_INFO = 0x1A,
	UNI_EVENT_BF_TAG_TXCMD_CFG_INFO = 0x1B,
	UNI_EVENT_BF_TAG_SND_CNT_INFO = 0x1D,
	UNI_EVENT_BF_TAG_NUM
};

struct UNI_EVENT_BF_STA_REC {
	uint16_t u2Tag;
	uint16_t u2Length;

	struct TXBF_PFMU_STA_INFO rTxBfPfmuInfo;
} __KAL_ATTRIB_PACKED__;

struct UNI_EVENT_CHIP_CAPABILITY {
	/* fixed field */
	uint16_t u2TotalElementNum;
	uint8_t aucPadding[2];
	/* tlv */

	uint8_t aucTlvBuffer[0];
} __KAL_ATTRIB_PACKED__;

/* CHIP CAPABILITY Tag */
enum ENUM_UNI_EVENT_CHIP_CAPABILITY_TAG {
	UNI_EVENT_CHIP_CAPABILITY_TAG_TX_RESOURCE = 0x0,
	UNI_EVENT_CHIP_CAPABILITY_TAG_TX_EFUSEADDRESS = 0x1,
	UNI_EVENT_CHIP_CAPABILITY_TAG_COEX_FEATURE = 0x2,
	UNI_EVENT_CHIP_CAPABILITY_TAG_SINGLE_SKU = 0x3,
	UNI_EVENT_CHIP_CAPABILITY_TAG_CSUM_OFFLOAD = 0x4,
	UNI_EVENT_CHIP_CAPABILITY_TAG_HW_VERSION = 0x5,
	UNI_EVENT_CHIP_CAPABILITY_TAG_SW_VERSION = 0x6,
	UNI_EVENT_CHIP_CAPABILITY_TAG_MAC_ADDR = 0x7,
	UNI_EVENT_CHIP_CAPABILITY_TAG_PHY_CAP = 0x8,
	UNI_EVENT_CHIP_CAPABILITY_TAG_MAC_CAP = 0x9,
	UNI_EVENT_CHIP_CAPABILITY_TAG_FRAME_BUF_CAP = 0xa,
	UNI_EVENT_CHIP_CAPABILITY_TAG_BEAMFORM_CAP = 0xb,
	UNI_EVENT_CHIP_CAPABILITY_TAG_LOCATION_CAP = 0xc,
	UNI_EVENT_CHIP_CAPABILITY_TAG_MUMIMO_CAP = 0xd,
	UNI_EVENT_CHIP_CAPABILITY_TAG_BUFFER_MODE_INFO = 0xe,
	UNI_EVENT_CHIP_CAPABILITY_TAG_R_MODE_CAP = 0xf,
	UNI_EVENT_CHIP_CAPABILITY_TAG_CMD_ID_SUPPORT_LIST = 0x10,
	UNI_EVENT_CHIP_CAPABILITY_TAG_CMD_EXTID_SUPPORT_LIST = 0x11,
	UNI_EVENT_CHIP_CAPABILITY_TAG_CMD_TAGID_SUPPORT_LIST_24 = 0x12,
	UNI_EVENT_CHIP_CAPABILITY_TAG_CMD_TAGID_SUPPORT_LIST_56 = 0x13,
	UNI_EVENT_CHIP_CAPABILITY_TAG_HW_ADIE_VERSION = 0x14,
	UNI_EVENT_CHIP_CAPABILITY_TAG_ATCMD_FEATURE_SUPPORT_LIST = 0x15,
	UNI_EVENT_CHIP_CAPABILITY_TAG_ANT_SWAP = 0x16,
	UNI_EVENT_CHIP_CAPABILITY_TAG_WFDMA_REALLOC = 0x17,
	UNI_EVENT_CHIP_CAPABILITY_TAG_HOST_STATUS_ADDRESS = 0x19,
	UNI_EVENT_CHIP_CAPABILITY_TAG_FAST_PATH = 0x1A,
	UNI_EVENT_CHIP_CAPABILITY_TAG_PSE_RX_QUOTA = 0x1B,
	UNI_EVENT_CHIP_CAPABILITY_TAG_NUM
};

struct UNI_EVENT_UPDATE_COEX {
	/* fixed field */
	uint8_t ucReserved[4];

	/* tlv */
	uint8_t aucTlvBuffer[0];/**< the TLVs included in this field:
*
*   TAG                              | ID  | structure
*   ---------------------------------|-----|--------------
*   EVENT_UPDATE_COEX_PHYRATE        | 0x0 | EVENT_UPDATE_COEX_PHYRATE
*					(Should always be the last TLV element)
*/
} __KAL_ATTRIB_PACKED__;

/* FW Log 2 Host event Tag */
enum ENUM_UNI_EVENT_UPDATE_COEX_TAG {
	UNI_EVENT_UPDATE_COEX_TAG_PHYRATE = 0,
	UNI_EVENT_UPDATE_COEX_TAG_NUM
};

/* FW Log with Format (Tag0) */
struct UNI_EVENT_UPDATE_COEX_PHYRATE {
	uint16_t u2Tag;
	uint16_t u2Length;
	uint32_t u4Flags;
	uint32_t au4PhyRateLimit[UNI_BSS_INFO_NUM];
} __KAL_ATTRIB_PACKED__;

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

#define NIC_FILL_UNI_CMD_TX_HDR(__prAd, __pucInfoBuffer, __u2InfoBufLen, \
	__u2CID, __ucPktTypeID, __pucSeqNum, __ppCmdBuf, \
	__ucS2DIndex, __ucOption) \
{ \
	struct mt66xx_chip_info *__prChipInfo; \
	struct WIFI_UNI_CMD_INFO __wifi_cmd_info; \
	__prChipInfo = __prAd->chip_info; \
	__wifi_cmd_info.pucInfoBuffer = __pucInfoBuffer; \
	__wifi_cmd_info.u2InfoBufLen = __u2InfoBufLen; \
	__wifi_cmd_info.u2CID = __u2CID; \
	__wifi_cmd_info.ucPktTypeID = __ucPktTypeID; \
	__wifi_cmd_info.ucS2DIndex = __ucS2DIndex; \
	__wifi_cmd_info.ucOption = __ucOption; \
	ASSERT(__prChipInfo->asicFillUniCmdTxd); \
	__prChipInfo->asicFillUniCmdTxd(__prAd, &(__wifi_cmd_info), \
		__pucSeqNum, (void **)__ppCmdBuf); \
}

#define GET_UNI_CMD_DATA_LEN(_cmd) \
	(((struct CMD_INFO *)(_cmd))->u4SetInfoLen)

#define GET_UNI_CMD_DATA(_cmd) \
	(((struct CMD_INFO *)(_cmd))->pucSetInfoBuffer)

#define GET_UNI_EVENT_DATA_LEN(_evt) \
	(((struct WIFI_UNI_EVENT *)(_evt))->u2PacketLength - \
				sizeof(struct WIFI_UNI_EVENT))

#define GET_UNI_EVENT_DATA(_evt) \
	(((struct WIFI_UNI_EVENT *)(_evt))->aucBuffer)

#define GET_UNI_EVENT_SEQ(_evt) \
	(((struct WIFI_UNI_EVENT *)(_evt))->ucSeqNum)

#define GET_UNI_EVENT_ID(_evt) \
	(((struct WIFI_UNI_EVENT *)(_evt))->ucEID)

#define GET_UNI_EVENT_EXT_ID(_evt) \
	(((struct WIFI_UNI_EVENT *)(_evt))->ucExtenEID)

#define GET_UNI_EVENT_OPTION(_evt) \
	(((struct WIFI_UNI_EVENT *)(_evt))->ucOption)

#define IS_UNI_EVENT(_evt) \
	((GET_UNI_EVENT_OPTION(_evt) & UNI_CMD_OPT_BIT_1_UNI_EVENT) > 0 ? \
	TRUE : FALSE)

#define IS_UNI_UNSOLICIT_EVENT(_evt) (IS_UNI_EVENT(_evt) && \
	((GET_UNI_EVENT_OPTION(_evt) & UNI_CMD_OPT_BIT_2_UNSOLICIT_EVENT) > 0 \
	? TRUE : FALSE))


#define TAG_ID(fp)	(((struct TAG_HDR *) fp)->u2Tag)
#define TAG_LEN(fp)	(((struct TAG_HDR *) fp)->u2Length)
#define TAG_DATA(fp)	(((struct TAG_HDR *) fp)->aucBuffer)
#define TAG_HDR_LEN	sizeof(struct TAG_HDR)

#define TAG_FOR_EACH(_pucTlvBuf, _i32TlvBufLen, _u2Offset) \
for ((_u2Offset) = 0;	\
	((((_u2Offset) + 2) <= (_i32TlvBufLen)) && \
	(((_u2Offset) + TAG_LEN(_pucTlvBuf)) <= (_i32TlvBufLen))); \
	(_u2Offset) += TAG_LEN(_pucTlvBuf), (_pucTlvBuf) += TAG_LEN(_pucTlvBuf))


/*******************************************************************************
 *                   F U N C T I O N   D E C L A R A T I O N S
 *******************************************************************************
 */

uint32_t wlanSendSetQueryCmdHelper(IN struct ADAPTER *prAdapter,
		    uint8_t ucCID,
		    uint8_t ucExtCID,
		    u_int8_t fgSetQuery,
		    u_int8_t fgNeedResp,
		    u_int8_t fgIsOid,
		    PFN_CMD_DONE_HANDLER pfCmdDoneHandler,
		    PFN_CMD_TIMEOUT_HANDLER pfCmdTimeoutHandler,
		    uint32_t u4SetQueryInfoLen,
		    uint8_t *pucInfoBuffer, OUT void *pvSetQueryBuffer,
		    IN uint32_t u4SetQueryBufferLen,
		    enum EUNM_CMD_SEND_METHOD eMethod);

uint32_t wlanSendSetQueryUniCmd(IN struct ADAPTER *prAdapter,
			uint8_t ucUCID,
			u_int8_t fgSetQuery,
			u_int8_t fgNeedResp,
			u_int8_t fgIsOid,
			PFN_CMD_DONE_HANDLER pfCmdDoneHandler,
			PFN_CMD_TIMEOUT_HANDLER pfCmdTimeoutHandler,
			uint32_t u4SetQueryInfoLen,
			uint8_t *pucInfoBuffer, OUT void *pvSetQueryBuffer,
			IN uint32_t u4SetQueryBufferLen);

uint32_t wlanSendSetQueryUniCmdAdv(IN struct ADAPTER *prAdapter,
			uint8_t ucUCID,
			u_int8_t fgSetQuery,
			u_int8_t fgNeedResp,
			u_int8_t fgIsOid,
			PFN_CMD_DONE_HANDLER pfCmdDoneHandler,
			PFN_CMD_TIMEOUT_HANDLER pfCmdTimeoutHandler,
			uint32_t u4SetQueryInfoLen,
			uint8_t *pucInfoBuffer, OUT void *pvSetQueryBuffer,
			IN uint32_t u4SetQueryBufferLen,
			enum EUNM_CMD_SEND_METHOD eMethod);

/*******************************************************************************
 *                   Command
 *******************************************************************************
 */

uint32_t nicUniCmdExtCommon(struct ADAPTER *ad,
		struct WIFI_UNI_SETQUERY_INFO *info);
uint32_t nicUniCmdNotSupport(struct ADAPTER *ad,
		struct WIFI_UNI_SETQUERY_INFO *info);
uint32_t nicUniCmdScanReqV2(struct ADAPTER *ad,
		struct WIFI_UNI_SETQUERY_INFO *info);
uint32_t nicUniCmdScanCancel(struct ADAPTER *ad,
		struct WIFI_UNI_SETQUERY_INFO *info);
uint32_t nicUniCmdBssActivateCtrl(struct ADAPTER *ad,
		struct WIFI_UNI_SETQUERY_INFO *info);
uint32_t nicUniCmdCustomerCfg(struct ADAPTER *ad,
		struct WIFI_UNI_SETQUERY_INFO *info);
uint32_t nicUniCmdChipCfg(struct ADAPTER *ad,
		struct WIFI_UNI_SETQUERY_INFO *info);
uint32_t nicUniCmdSetRxFilter(struct ADAPTER *ad,
		struct WIFI_UNI_SETQUERY_INFO *info);
uint32_t nicUniCmdSetMbmc(struct ADAPTER *ad,
		struct WIFI_UNI_SETQUERY_INFO *info);
uint32_t nicUniCmdPowerCtrl(struct ADAPTER *ad,
		struct WIFI_UNI_SETQUERY_INFO *info);
uint32_t nicUniCmdSetDomain(struct ADAPTER *ad,
		struct WIFI_UNI_SETQUERY_INFO *info);
uint32_t nicUniCmdRemoveStaRec(struct ADAPTER *ad,
		struct WIFI_UNI_SETQUERY_INFO *info);
uint32_t nicUniCmdBcnContent(struct ADAPTER *ad,
		struct WIFI_UNI_SETQUERY_INFO *info);
uint32_t nicUniCmdPmDisable(struct ADAPTER *ad,
		struct WIFI_UNI_SETQUERY_INFO *info);
uint32_t nicUniCmdPmEnable(struct ADAPTER *ad,
		struct WIFI_UNI_SETQUERY_INFO *info);
uint32_t nicUniCmdSetBssInfo(struct ADAPTER *ad,
		struct WIFI_UNI_SETQUERY_INFO *info);
uint32_t nicUniCmdSetBssRlm(struct ADAPTER *ad,
		struct WIFI_UNI_SETQUERY_INFO *info);
uint32_t nicUniCmdUpdateStaRec(struct ADAPTER *ad,
		struct WIFI_UNI_SETQUERY_INFO *info);
uint32_t nicUniCmdChPrivilege(struct ADAPTER *ad,
		struct WIFI_UNI_SETQUERY_INFO *info);
uint32_t nicUniCmdWsysFwLogUI(struct ADAPTER *ad,
		struct WIFI_UNI_SETQUERY_INFO *info);
uint32_t nicUniCmdWsysFwBasicConfig(struct ADAPTER *ad,
		struct WIFI_UNI_SETQUERY_INFO *info);
uint32_t nicUniCmdSetSuspendMode(struct ADAPTER *ad,
		struct WIFI_UNI_SETQUERY_INFO *info);
uint32_t nicUniCmdSetWOWLAN(struct ADAPTER *ad,
		struct WIFI_UNI_SETQUERY_INFO *info);
uint32_t nicUniCmdPowerSaveMode(struct ADAPTER *ad,
		struct WIFI_UNI_SETQUERY_INFO *info);
uint32_t nicUniCmdTwtArgtUpdate(struct ADAPTER *ad,
		struct WIFI_UNI_SETQUERY_INFO *info);
uint32_t nicUniCmdStaRecUpdateExt(struct ADAPTER *ad,
		struct WIFI_UNI_SETQUERY_INFO *info);
uint32_t nicUniCmdBFAction(struct ADAPTER *ad,
		struct WIFI_UNI_SETQUERY_INFO *info);
uint32_t nicUniCmdSerAction(struct ADAPTER *ad,
		struct WIFI_UNI_SETQUERY_INFO *info);
uint32_t nicUniCmdGetTsf(struct ADAPTER *ad,
		struct WIFI_UNI_SETQUERY_INFO *info);
uint32_t nicUniCmdUpdateEdcaSet(struct ADAPTER *ad,
		struct WIFI_UNI_SETQUERY_INFO *info);
uint32_t nicUniCmdAccessReg(struct ADAPTER *ad,
		struct WIFI_UNI_SETQUERY_INFO *info);
uint32_t nicUniCmdUpdateMuEdca(struct ADAPTER *ad,
		struct WIFI_UNI_SETQUERY_INFO *info);
uint32_t nicUniCmdOffloadIPV4(struct ADAPTER *ad,
		struct WIFI_UNI_SETQUERY_INFO *info);
uint32_t nicUniCmdOffloadIPV6(struct ADAPTER *ad,
		struct WIFI_UNI_SETQUERY_INFO *info);
uint32_t nicUniCmdGetIdcChnl(struct ADAPTER *ad,
		struct WIFI_UNI_SETQUERY_INFO *info);

/*******************************************************************************
 *                   Event
 *******************************************************************************
 */

void nicRxProcessUniEventPacket(IN struct ADAPTER *prAdapter,
			     IN OUT struct SW_RFB *prSwRfb);
void nicUniCmdEventSetCommon(IN struct ADAPTER
	*prAdapter, IN struct CMD_INFO *prCmdInfo,
	IN uint8_t *pucEventBuf);
void nicUniCmdTimeoutCommon(IN struct ADAPTER *prAdapter,
			    IN struct CMD_INFO *prCmdInfo);

/*******************************************************************************
 *                   Solicited Event
 *******************************************************************************
 */

void nicUniCmdEventQueryCfgRead(IN struct ADAPTER *prAdapter,
	IN struct CMD_INFO *prCmdInfo, IN uint8_t *pucEventBuf);
void nicUniEventQueryChipConfig(IN struct ADAPTER *prAdapter,
	IN struct CMD_INFO *prCmdInfo, IN uint8_t *pucEventBuf);
void nicUniEventQueryIdcChnl(IN struct ADAPTER *prAdapter,
		IN struct CMD_INFO *prCmdInfo,
		IN uint8_t *pucEventBuf);
void nicUniEventBFStaRec(IN struct ADAPTER *prAdapter,
	IN struct CMD_INFO *prCmdInfo, IN uint8_t *pucEventBuf);
void nicUniCmdEventQueryMcrRead(IN struct ADAPTER *prAdapter,
	IN struct CMD_INFO *prCmdInfo, IN uint8_t *pucEventBuf);
void nicUniCmdEventGetTsfDone(IN struct ADAPTER *prAdapter,
	IN struct CMD_INFO *prCmdInfo, IN uint8_t *pucEventBuf);

/*******************************************************************************
 *                   Unsolicited Event
 *******************************************************************************
 */

void nicUniEventScanDone(struct ADAPTER *ad,
	struct WIFI_UNI_EVENT *evt);
void nicUniEventChMngrHandleChEvent(struct ADAPTER *ad,
	struct WIFI_UNI_EVENT *evt);
void nicUniEventMbmcHandleEvent(struct ADAPTER *ad,
	struct WIFI_UNI_EVENT *evt);
void nicUniEventStatusToHost(struct ADAPTER *ad,
	struct WIFI_UNI_EVENT *evt);
void nicUniEventBaOffload(struct ADAPTER *ad,
	struct WIFI_UNI_EVENT *evt);
void nicUniEventSleepNotify(struct ADAPTER *ad,
	struct WIFI_UNI_EVENT *evt);
void nicUniEventBeaconTimeout(struct ADAPTER *ad,
	struct WIFI_UNI_EVENT *evt);
void nicUniEventUpdateCoex(struct ADAPTER *ad,
	struct WIFI_UNI_EVENT *evt);
void nicUniEventIdc(struct ADAPTER *ad,
	struct WIFI_UNI_EVENT *evt);
void nicUniEventBssIsAbsence(struct ADAPTER *ad,
	struct WIFI_UNI_EVENT *evt);
void nicUniEventPsSync(struct ADAPTER *ad,
	struct WIFI_UNI_EVENT *evt);
void nicUniEventSap(struct ADAPTER *ad,
	struct WIFI_UNI_EVENT *evt);

/*******************************************************************************
 *                              F U N C T I O N S
 *******************************************************************************
 */


#endif /* _NIC_CMD_EVENT_H */
