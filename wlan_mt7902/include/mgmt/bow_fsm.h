/* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause */
/*
 * Copyright (c) 2016 MediaTek Inc.
 */
/*
** Id: //Department/DaVinci/BRANCHES/MT6620_WIFI_DRIVER_V2_3/include/mgmt/bow_fsm.h#1
*/

/*! \file   bow_fsm.h
*    \brief  Declaration of functions and finite state machine for BOW Module.
*
*    Declaration of functions and finite state machine for BOW Module.
*/


#ifndef _BOW_FSM_H
#define _BOW_FSM_H

/*******************************************************************************
*                         C O M P I L E R   F L A G S
********************************************************************************
*/

/*******************************************************************************
*                    E X T E R N A L   R E F E R E N C E S
********************************************************************************
*/

/*******************************************************************************
*                              C O N S T A N T S
********************************************************************************
*/

#define BOW_BG_SCAN_INTERVAL_MIN_SEC        2	/* 30 // exponential to 960 */
#define BOW_BG_SCAN_INTERVAL_MAX_SEC        2	/* 960 // 16min */

#define BOW_DELAY_TIME_OF_DISCONNECT_SEC    10

#define BOW_BEACON_TIMEOUT_COUNT_STARTING   10
#define BOW_BEACON_TIMEOUT_GUARD_TIME_SEC   1	/* Second */

#define BOW_BEACON_MAX_TIMEOUT_TU           100
#define BOW_BEACON_MIN_TIMEOUT_TU           5
#define BOW_BEACON_MAX_TIMEOUT_VALID        TRUE
#define BOW_BEACON_MIN_TIMEOUT_VALID        TRUE

#define BOW_BMC_MAX_TIMEOUT_TU              100
#define BOW_BMC_MIN_TIMEOUT_TU              5
#define BOW_BMC_MAX_TIMEOUT_VALID           TRUE
#define BOW_BMC_MIN_TIMEOUT_VALID           TRUE

#define BOW_JOIN_CH_GRANT_THRESHOLD         10
#define BOW_JOIN_CH_REQUEST_INTERVAL        2000

/*******************************************************************************
*                             D A T A   T Y P E S
********************************************************************************
*/

enum ENUM_BOW_STATE {
	BOW_STATE_IDLE = 0,
	BOW_STATE_SEARCH,
	BOW_STATE_SCAN,
	BOW_STATE_ONLINE_SCAN,
	BOW_STATE_LOOKING_FOR,
	BOW_STATE_WAIT_FOR_NEXT_SCAN,
	BOW_STATE_REQ_CHANNEL_JOIN,
	BOW_STATE_REQ_CHANNEL_ALONE,
	BOW_STATE_REQ_CHANNEL_MERGE,
	BOW_STATE_JOIN,
	BOW_STATE_IBSS_ALONE,
	BOW_STATE_IBSS_MERGE,
	BOW_STATE_NORMAL_TR,
	BOW_STATE_NUM
};

struct BOW_FSM_INFO {
	/* Channel Privilege */
	u_int8_t fgIsChannelRequested;
	u_int8_t fgIsChannelGranted;
	uint32_t u4ChGrantedInterval;

	uint8_t ucPrimaryChannel;
	enum ENUM_BAND eBand;
	uint16_t u2BeaconInterval;

	struct STA_RECORD *prTargetStaRec;
	struct BSS_DESC *prTargetBssDesc;	/* For destination */

	uint8_t aucPeerAddress[6];
	uint8_t ucBssIndex;	/* Assume there is only 1 BSS for BOW */
	uint8_t ucRole;		/* Initiator or responder */
	uint8_t ucAvailableAuthTypes;	/* Used for AUTH_MODE_AUTO_SWITCH */

	u_int8_t fgSupportQoS;

	/* Sequence number of requested message. */
	uint8_t ucSeqNumOfChReq;
	uint8_t ucSeqNumOfReqMsg;
	uint8_t ucSeqNumOfScnMsg;
	uint8_t ucSeqNumOfScanReq;
	uint8_t ucSeqNumOfCancelMsg;

	/* Timer */
	struct TIMER rStartingBeaconTimer;	/* For device discovery time of each discovery request from user. */
	struct TIMER rChGrantedTimer;

	/* can be deleted? */
	struct TIMER rIndicationOfDisconnectTimer;

};

/*******************************************************************************
*                            P U B L I C   D A T A
********************************************************************************
*/

/*******************************************************************************
*                           P R I V A T E   D A T A
********************************************************************************
*/

/*******************************************************************************
*                                 M A C R O S
********************************************************************************
*/

#define bowChangeMediaState(_prBssInfo, _eNewMediaState) \
	(_prBssInfo->eConnectionState = (_eNewMediaState))
	/* (_prAdapter->rWifiVar.arBssInfo[NETWORK_TYPE_BOW_INDEX].eConnectionState = (_eNewMediaState)); */

/*******************************************************************************
*                  F U N C T I O N   D E C L A R A T I O N S
********************************************************************************
*/

/*******************************************************************************
*                              F U N C T I O N S
********************************************************************************
*/

#endif
