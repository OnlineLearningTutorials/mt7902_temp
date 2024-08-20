/* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause */
/*
 * Copyright (c) 2016 MediaTek Inc.
 */
/*
 ** Id: //Department/DaVinci/TRUNK/WiFi_P2P_Driver/include/nic/p2p_mac.h#2
 */

/*! \file   "p2p_mac.h"
 *  \brief  Brief description.
 *
 *  Detail description.
 */

#ifndef _P2P_MAC_H
#define _P2P_MAC_H

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

#define ACTION_PUBLIC_WIFI_DIRECT                   9
#define ACTION_GAS_INITIAL_REQUEST                 10
#define ACTION_GAS_INITIAL_RESPONSE               11
#define ACTION_GAS_COMEBACK_REQUEST           12
#define ACTION_GAS_COMEBACK_RESPONSE         13

/* P2P 4.2.8.1 - P2P Public Action Frame Type. */
#define P2P_PUBLIC_ACTION_GO_NEGO_REQ               0
#define P2P_PUBLIC_ACTION_GO_NEGO_RSP               1
#define P2P_PUBLIC_ACTION_GO_NEGO_CFM               2
#define P2P_PUBLIC_ACTION_INVITATION_REQ            3
#define P2P_PUBLIC_ACTION_INVITATION_RSP            4
#define P2P_PUBLIC_ACTION_DEV_DISCOVER_REQ          5
#define P2P_PUBLIC_ACTION_DEV_DISCOVER_RSP          6
#define P2P_PUBLIC_ACTION_PROV_DISCOVERY_REQ        7
#define P2P_PUBLIC_ACTION_PROV_DISCOVERY_RSP        8

/* P2P 4.2.9.1 - P2P Action Frame Type */
#define P2P_ACTION_NOTICE_OF_ABSENCE                0
#define P2P_ACTION_P2P_PRESENCE_REQ                 1
#define P2P_ACTION_P2P_PRESENCE_RSP                 2
#define P2P_ACTION_GO_DISCOVER_REQ                  3

#define P2P_PUBLIC_ACTION_FRAME_LEN (WLAN_MAC_MGMT_HEADER_LEN + 8)
#define P2P_ACTION_FRAME_LEN         (WLAN_MAC_MGMT_HEADER_LEN + 7)

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

/* --------------- WPS Data Element Definitions --------------- */
/* P2P 4.2.2 - General WSC Attribute */
/* ID(2 octet) + Length(2 octets) */
#define WSC_ATTRI_HDR_LEN                                   4
#define WSC_ATTRI_MAX_LEN_VERSION                           1
#define WSC_ATTRI_MAX_LEN_DEVICE_PASSWORD_ID                2
#define WSC_ATTRI_LEN_CONFIG_METHOD                         2

/* --------------- WFA P2P IE --------------- */
/* P2P 4.1.1 - P2P IE format */
#define P2P_OUI_TYPE_LEN                            4
/* == OFFSET_OF(IE_P2P_T, aucP2PAttributes[0]) */
#define P2P_IE_OUI_HDR (ELEM_HDR_LEN + P2P_OUI_TYPE_LEN)

/* P2P 4.1.1 - General P2P Attribute */
#define P2P_ATTRI_HDR_LEN  3	/* ID(1 octet) + Length(2 octets) */

/* P2P 4.1.1 - P2P Attribute ID definitions */
#define P2P_ATTRI_ID_STATUS                                 0
#define P2P_ATTRI_ID_REASON_CODE                            1
#define P2P_ATTRI_ID_P2P_CAPABILITY                         2
#define P2P_ATTRI_ID_P2P_DEV_ID                             3
#define P2P_ATTRI_ID_GO_INTENT                              4
#define P2P_ATTRI_ID_CFG_TIMEOUT                            5
#define P2P_ATTRI_ID_LISTEN_CHANNEL                         6
#define P2P_ATTRI_ID_P2P_GROUP_BSSID                        7
#define P2P_ATTRI_ID_EXT_LISTEN_TIMING                      8
#define P2P_ATTRI_ID_INTENDED_P2P_IF_ADDR                   9
#define P2P_ATTRI_ID_P2P_MANAGEABILITY                      10
#define P2P_ATTRI_ID_CHANNEL_LIST                           11
#define P2P_ATTRI_ID_NOTICE_OF_ABSENCE                      12
#define P2P_ATTRI_ID_P2P_DEV_INFO                           13
#define P2P_ATTRI_ID_P2P_GROUP_INFO                         14
#define P2P_ATTRI_ID_P2P_GROUP_ID                           15
#define P2P_ATTRI_ID_P2P_INTERFACE                          16
#define P2P_ATTRI_ID_OPERATING_CHANNEL                      17
#define P2P_ATTRI_ID_INVITATION_FLAG                        18
#define P2P_ATTRI_ID_VENDOR_SPECIFIC                        221

/* Maximum Length of P2P Attributes */
#define P2P_ATTRI_MAX_LEN_STATUS                            1	/* 0 */
#define P2P_ATTRI_MAX_LEN_REASON_CODE                       1	/* 1 */
#define P2P_ATTRI_MAX_LEN_P2P_CAPABILITY                    2	/* 2 */
#define P2P_ATTRI_MAX_LEN_P2P_DEV_ID                        6	/* 3 */
#define P2P_ATTRI_MAX_LEN_GO_INTENT                         1	/* 4 */
#define P2P_ATTRI_MAX_LEN_CFG_TIMEOUT                       2	/* 5 */
#define P2P_ATTRI_MAX_LEN_LISTEN_CHANNEL                    5	/* 6 */
#define P2P_ATTRI_MAX_LEN_P2P_GROUP_BSSID                   6	/* 7 */
#define P2P_ATTRI_MAX_LEN_EXT_LISTEN_TIMING                 4	/* 8 */
#define P2P_ATTRI_MAX_LEN_INTENDED_P2P_IF_ADDR              6	/* 9 */
#define P2P_ATTRI_MAX_LEN_P2P_MANAGEABILITY                 1	/* 10 */
/* #define P2P_ATTRI_MAX_LEN_CHANNEL_LIST 3 + (n* (2 + num_of_ch)) *//* 11 */
#define P2P_ATTRI_LEN_CHANNEL_LIST                                  3	/* 11 */
#define P2P_ATTRI_LEN_CHANNEL_ENTRY                                  2	/* 11 */

#define P2P_MAXIMUM_ATTRIBUTE_LEN                   251

/* P2P 4.1.2 - P2P Status definitions */
#define P2P_STATUS_SUCCESS                                  0
#define P2P_STATUS_FAIL_INFO_IS_CURRENTLY_UNAVAILABLE   1
#define P2P_STATUS_FAIL_INCOMPATIBLE_PARAM                  2
#define P2P_STATUS_FAIL_LIMIT_REACHED                       3
#define P2P_STATUS_FAIL_INVALID_PARAM                       4
#define P2P_STATUS_FAIL_UNABLE_ACCOMMODATE_REQ              5
#define P2P_STATUS_FAIL_PREVIOUS_PROTOCOL_ERR               6
#define P2P_STATUS_FAIL_NO_COMMON_CHANNELS                  7
#define P2P_STATUS_FAIL_UNKNOWN_P2P_GROUP                   8
#define P2P_STATUS_FAIL_SAME_INTENT_VALUE_15                9
#define P2P_STATUS_FAIL_INCOMPATIBLE_PROVISION_METHOD       10
#define P2P_STATUS_FAIL_REJECTED_BY_USER                    11

/* P2P 4.1.14 - CTWindow and OppPS Parameters definitions */
#define P2P_CTW_OPPPS_PARAM_OPPPS_FIELD                     BIT(7)
#define P2P_CTW_OPPPS_PARAM_CTWINDOW_MASK                   BITS(0, 6)
/* Action frame categories (IEEE 802.11-2007, 7.3.1.11, Table 7-24) */
#define WLAN_ACTION_SPECTRUM_MGMT 0
#define WLAN_ACTION_QOS 1
#define WLAN_ACTION_DLS 2
#define WLAN_ACTION_BLOCK_ACK 3
#define WLAN_ACTION_PUBLIC 4
#define WLAN_ACTION_RADIO_MEASUREMENT 5
#define WLAN_ACTION_FT 6
#define WLAN_ACTION_HT 7
#define WLAN_ACTION_SA_QUERY 8
#define WLAN_ACTION_PROTECTED_DUAL 9
#define WLAN_ACTION_WNM 10
#define WLAN_ACTION_UNPROTECTED_WNM 11
#define WLAN_ACTION_TDLS 12
#define WLAN_ACTION_SELF_PROTECTED 15
#define WLAN_ACTION_WMM 17 /* WMM Specification 1.1 */
#define WLAN_ACTION_VENDOR_SPECIFIC 127
#define P2P_IE_VENDOR_TYPE 0x506f9a09
#define WFD_IE_VENDOR_TYPE 0x506f9a0a

/* Public action codes */
#define WLAN_PA_20_40_BSS_COEX 0
#define WLAN_PA_VENDOR_SPECIFIC 9
#define WLAN_PA_GAS_INITIAL_REQ 10
#define WLAN_PA_GAS_INITIAL_RESP 11
#define WLAN_PA_GAS_COMEBACK_REQ 12
#define WLAN_PA_GAS_COMEBACK_RESP 13
#define WLAN_TDLS_DISCOVERY_RESPONSE 14

/* P2P public action frames */
enum ENUM_P2P_ACTION_FRAME_TYPE {
	P2P_GO_NEG_REQ       = 0,
	P2P_GO_NEG_RESP      = 1,
	P2P_GO_NEG_CONF      = 2,
	P2P_INVITATION_REQ   = 3,
	P2P_INVITATION_RESP  = 4,
	P2P_DEV_DISC_REQ     = 5,
	P2P_DEV_DISC_RESP    = 6,
	P2P_PROV_DISC_REQ    = 7,
	P2P_PROV_DISC_RESP   = 8
};

/* --------------- WFA P2P IE and Attributes --------------- */

/* P2P 4.1.1 - P2P Information Element */
struct IE_P2P {
	uint8_t ucId;		/* Element ID */
	uint8_t ucLength;	/* Length */
	uint8_t aucOui[3];	/* OUI */
	uint8_t ucOuiType;	/* OUI Type */
	uint8_t aucP2PAttributes[1];	/* P2P Attributes */
} __KAL_ATTRIB_PACKED__;

/* P2P 4.1.1 - General WSC Attribute */
struct WSC_ATTRIBUTE {
	uint16_t u2Id;		/* Attribute ID */
	uint16_t u2Length;	/* Length */
	uint8_t aucBody[1];	/* Body field */
} __KAL_ATTRIB_PACKED__;

/* P2P 4.1.2 - P2P Status Attribute */
struct P2P_ATTRI_STATUS {
	uint8_t ucId;		/* Attribute ID */
	uint16_t u2Length;	/* Length */
	uint8_t ucStatusCode;	/* Status Code */
} __KAL_ATTRIB_PACKED__;

/* P2P 4.1.10 - Extended Listen Timing Attribute */
struct P2P_ATTRI_EXT_LISTEN_TIMING {
	uint8_t ucId;		/* Attribute ID */
	uint16_t u2Length;	/* Length */
	uint16_t u2AvailPeriod;	/* Availability Period */
	uint16_t u2AvailInterval;	/* Availability Interval */
} __KAL_ATTRIB_PACKED__;

/* P2P 4.2.8.2 P2P Public Action Frame Format */
struct P2P_PUBLIC_ACTION_FRAME {
	/* MAC header */
	uint16_t u2FrameCtrl;	/* Frame Control */
	uint16_t u2Duration;	/* Duration */
	uint8_t aucDestAddr[MAC_ADDR_LEN];	/* DA */
	uint8_t aucSrcAddr[MAC_ADDR_LEN];	/* SA */
	uint8_t aucBSSID[MAC_ADDR_LEN];	/* BSSID */
	uint16_t u2SeqCtrl;	/* Sequence Control */
	/* P2P Public Action Frame Body */
	uint8_t ucCategory;	/* Category, 0x04 */
	uint8_t ucAction;	/* Action Value, 0x09 */
	uint8_t aucOui[3];	/* 0x50, 0x6F, 0x9A */
	uint8_t ucOuiType;	/* 0x09 */
	/* GO Nego Req/Rsp/Cfm, P2P Invittion Req/Rsp, */
	uint8_t ucOuiSubtype;
	/* Device Discovery Req/Rsp */
	uint8_t ucDialogToken;	/* Dialog Token. */
	uint8_t aucInfoElem[1];	/* P2P IE, WSC IE. */
} __KAL_ATTRIB_PACKED__;

/* P2P 4.2.9.1 -  General Action Frame Format. */
struct P2P_ACTION_FRAME {
	/* MAC header */
	uint16_t u2FrameCtrl;	/* Frame Control */
	uint16_t u2Duration;	/* Duration */
	uint8_t aucDestAddr[MAC_ADDR_LEN];	/* DA */
	uint8_t aucSrcAddr[MAC_ADDR_LEN];	/* SA */
	uint8_t aucBSSID[MAC_ADDR_LEN];	/* BSSID */
	uint16_t u2SeqCtrl;	/* Sequence Control */
	/* P2P Action Frame Body */
	uint8_t ucCategory;	/* 0x7F */
	uint8_t aucOui[3];	/* 0x50, 0x6F, 0x9A */
	uint8_t ucOuiType;	/* 0x09 */
	uint8_t ucOuiSubtype;	/*  */
	uint8_t ucDialogToken;
	uint8_t aucInfoElem[1];
} __KAL_ATTRIB_PACKED__;

/* P2P C.1 GAS Public Action Initial Request Frame Format */
struct GAS_PUBLIC_ACTION_INITIAL_REQUEST_FRAME {
	/* MAC header */
	uint16_t u2FrameCtrl;	/* Frame Control */
	uint16_t u2Duration;	/* Duration */
	uint8_t aucDestAddr[MAC_ADDR_LEN];	/* DA */
	uint8_t aucSrcAddr[MAC_ADDR_LEN];	/* SA */
	uint8_t aucBSSID[MAC_ADDR_LEN];	/* BSSID */
	/* P2P Public Action Frame Body */
	uint16_t u2SeqCtrl;	/* Sequence Control */
	uint8_t ucCategory;	/* Category, 0x04 */
	uint8_t ucAction;	/* Action Value, 0x09 */
	uint8_t ucDialogToken;	/* Dialog Token. */
	uint8_t aucInfoElem[1];	/* Advertisement IE. */
} __KAL_ATTRIB_PACKED__;

/* P2P C.2 GAS Public Action Initial Response Frame Format */
struct GAS_PUBLIC_ACTION_INITIAL_RESPONSE_FRAME {
	/* MAC header */
	uint16_t u2FrameCtrl;	/* Frame Control */
	uint16_t u2Duration;	/* Duration */
	uint8_t aucDestAddr[MAC_ADDR_LEN];	/* DA */
	uint8_t aucSrcAddr[MAC_ADDR_LEN];	/* SA */
	uint8_t aucBSSID[MAC_ADDR_LEN];	/* BSSID */
	uint16_t u2SeqCtrl;	/* Sequence Control */
	/* P2P Public Action Frame Body */
	uint8_t ucCategory;	/* Category, 0x04 */
	uint8_t ucAction;	/* Action Value, 0x09 */
	uint8_t ucDialogToken;	/* Dialog Token. */
	uint16_t u2StatusCode;	/* Initial Response. */
	uint16_t u2ComebackDelay;
	/* Initial Response. *//* In unit of TU. */
	uint8_t aucInfoElem[1];	/* Advertisement IE. */
} __KAL_ATTRIB_PACKED__;

/* P2P C.3-1 GAS Public Action Comeback Request Frame Format */
struct GAS_PUBLIC_ACTION_COMEBACK_REQUEST_FRAME {
	/* MAC header */
	uint16_t u2FrameCtrl;	/* Frame Control */
	uint16_t u2Duration;	/* Duration */
	uint8_t aucDestAddr[MAC_ADDR_LEN];	/* DA */
	uint8_t aucSrcAddr[MAC_ADDR_LEN];	/* SA */
	uint8_t aucBSSID[MAC_ADDR_LEN];	/* BSSID */
	uint16_t u2SeqCtrl;	/* Sequence Control */
	/* P2P Public Action Frame Body */
	uint8_t ucCategory;	/* Category, 0x04 */
	uint8_t ucAction;	/* Action Value, 0x09 */
	uint8_t ucDialogToken;	/* Dialog Token. */
} __KAL_ATTRIB_PACKED__;

/* P2P C.3-2 GAS Public Action Comeback Response Frame Format */
struct GAS_PUBLIC_ACTION_COMEBACK_RESPONSE_FRAME {
	/* MAC header */
	uint16_t u2FrameCtrl;	/* Frame Control */
	uint16_t u2Duration;	/* Duration */
	uint8_t aucDestAddr[MAC_ADDR_LEN];	/* DA */
	uint8_t aucSrcAddr[MAC_ADDR_LEN];	/* SA */
	uint8_t aucBSSID[MAC_ADDR_LEN];	/* BSSID */
	uint16_t u2SeqCtrl;	/* Sequence Control */
	/* P2P Public Action Frame Body */
	uint8_t ucCategory;	/* Category, 0x04 */
	uint8_t ucAction;	/* Action Value, 0x09 */
	uint8_t ucDialogToken;	/* Dialog Token. */
	uint16_t u2StatusCode;	/* Comeback Response. */
	uint8_t ucFragmentID;	/*Comeback Response. */
	uint16_t u2ComebackDelay;	/* Comeback Response. */
	uint8_t aucInfoElem[1];	/* Advertisement IE. */
} __KAL_ATTRIB_PACKED__;

struct P2P_SD_VENDER_SPECIFIC_CONTENT {
	/* Service Discovery Vendor-specific Content. */
	uint8_t ucOuiSubtype;	/* 0x09 */
	uint16_t u2ServiceUpdateIndicator;
	uint8_t aucServiceTLV[1];
} __KAL_ATTRIB_PACKED__;

struct P2P_SERVICE_REQUEST_TLV {
	uint16_t u2Length;
	uint8_t ucServiceProtocolType;
	uint8_t ucServiceTransID;
	uint8_t aucQueryData[1];
} __KAL_ATTRIB_PACKED__;

struct P2P_SERVICE_RESPONSE_TLV {
	uint16_t u2Length;
	uint8_t ucServiceProtocolType;
	uint8_t ucServiceTransID;
	uint8_t ucStatusCode;
	uint8_t aucResponseData[1];
} __KAL_ATTRIB_PACKED__;

/* P2P 4.1.1 - General P2P Attribute */
struct P2P_ATTRIBUTE {
	uint8_t ucId;		/* Attribute ID */
	uint16_t u2Length;	/* Length */
	uint8_t aucBody[1];	/* Body field */
} __KAL_ATTRIB_PACKED__;

/* P2P 4.1.14 - Notice of Absence Attribute */
struct P2P_ATTRI_NOA {
	uint8_t ucId;		/* Attribute ID */
	uint16_t u2Length;	/* Length */
	uint8_t ucIndex;		/* Index */
	uint8_t ucCTWOppPSParam;	/* CTWindow and OppPS Parameters */
	uint8_t aucNoADesc[1];	/* NoA Descriptor */
} __KAL_ATTRIB_PACKED__;

struct NOA_DESCRIPTOR {
	uint8_t ucCountType;	/* Count/Type */
	uint32_t u4Duration;	/* Duration */
	uint32_t u4Interval;	/* Interval */
	uint32_t u4StartTime;	/* Start Time */
} __KAL_ATTRIB_PACKED__;
struct CHANNEL_ENTRY_FIELD {
	uint8_t ucRegulatoryClass;	/* Regulatory Class */
	uint8_t ucNumberOfChannels;	/* Number Of Channels */
	uint8_t aucChannelList[1];	/* Channel List */
} __KAL_ATTRIB_PACKED__;

#endif
