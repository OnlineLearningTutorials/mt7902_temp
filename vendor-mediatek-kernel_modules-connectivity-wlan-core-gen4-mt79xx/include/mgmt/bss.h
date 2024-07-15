/* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause */
/*
 * Copyright (c) 2016 MediaTek Inc.
 */

/*! \file   "bss.h"
 *    \brief  In this file we define the function prototype used in BSS/IBSS.
 *
 *    The file contains the function declarations and defines
 *						for used in BSS/IBSS.
 */


#ifndef _BSS_H
#define _BSS_H

/*******************************************************************************
 *                         C O M P I L E R   F L A G S
 *******************************************************************************
 */

/*******************************************************************************
 *                    E X T E R N A L   R E F E R E N C E S
 *******************************************************************************
 */
#include "wlan_def.h"
extern const uint8_t *apucNetworkType[NETWORK_TYPE_NUM];

/*******************************************************************************
 *                              C O N S T A N T S
 *******************************************************************************
 */
#define BSS_DEFAULT_NUM         KAL_BSS_NUM

/* Define how many concurrent operation networks. */
#define BSS_P2P_NUM             KAL_P2P_NUM

#if (BSS_P2P_NUM > MAX_BSSID_NUM)
#error Exceed HW capability (KAL_BSS_NUM or KAL_P2P_NUM)!!
#endif

/* NOTE(Kevin): change define for george */
/* #define MAX_LEN_TIM_PARTIAL_BMP     (((MAX_ASSOC_ID + 1) + 7) / 8) */
/* Required bits = (MAX_ASSOC_ID + 1) */
#define MAX_LEN_TIM_PARTIAL_BMP                     ((CFG_STA_REC_NUM + 7) / 8)
/* reserve length greater than maximum size of STA_REC */
/* obsoleted: Assume we only use AID:1~15 */

/* CTRL FLAGS for Probe Response */
#define BSS_PROBE_RESP_USE_P2P_DEV_ADDR             BIT(0)
#define BSS_PROBE_RESP_INCLUDE_P2P_IE               BIT(1)

#define MAX_BSS_INDEX           HW_BSSID_NUM
#define P2P_DEV_BSS_INDEX       MAX_BSS_INDEX

#define IS_BSS_ALIVE(_prAdapter, _prBssInfo) \
	(_prBssInfo->fgIsInUse && \
	_prBssInfo->fgIsNetActive && \
	(_prBssInfo->eConnectionState == MEDIA_STATE_CONNECTED || \
	(_prBssInfo->eCurrentOPMode == OP_MODE_ACCESS_POINT && \
	IS_NET_PWR_STATE_ACTIVE(_prAdapter, \
	_prBssInfo->ucBssIndex))))

#define IS_BSS_NOT_ALIVE(_prAdapter, _prBssInfo) \
	(!IS_BSS_ALIVE(_prAdapter, _prBssInfo))

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
#define IS_BSS_INDEX_VALID(_ucBssIndex)     ((_ucBssIndex) <= P2P_DEV_BSS_INDEX)

#define GET_BSS_INFO_BY_INDEX(_prAdapter, _ucBssIndex) \
		((_prAdapter)->aprBssInfo[(_ucBssIndex)])

#define bssAssignAssocID(_prStaRec)         ((_prStaRec)->ucIndex + 1)

/*******************************************************************************
 *                   F U N C T I O N   D E C L A R A T I O N S
 *******************************************************************************
 */
/*----------------------------------------------------------------------------*/
/* Routines for all Operation Modes                                           */
/*----------------------------------------------------------------------------*/
struct STA_RECORD *
bssCreateStaRecFromBssDesc(IN struct ADAPTER *prAdapter,
			   IN enum ENUM_STA_TYPE eStaType, IN uint8_t uBssIndex,
			   IN struct BSS_DESC *prBssDesc);

#if (CFG_SUPPORT_SUPPLICANT_SME == 1)
struct STA_RECORD *
bssUpdateStaRecFromCfgAssoc(IN struct ADAPTER *prAdapter,
				IN struct BSS_DESC *prBssDesc,
				IN struct STA_RECORD *prStaRec);
#endif

void bssComposeNullFrame(IN struct ADAPTER *prAdapter,
			 IN uint8_t *pucBuffer, IN struct STA_RECORD *prStaRec);

void
bssComposeQoSNullFrame(IN struct ADAPTER *prAdapter,
		       IN uint8_t *pucBuffer, IN struct STA_RECORD *prStaRec,
		       IN uint8_t ucUP, IN u_int8_t fgSetEOSP);

uint32_t
bssSendNullFrame(IN struct ADAPTER *prAdapter,
		 IN struct STA_RECORD *prStaRec,
		 IN PFN_TX_DONE_HANDLER pfTxDoneHandler);

uint32_t
bssSendQoSNullFrame(IN struct ADAPTER *prAdapter,
		    IN struct STA_RECORD *prStaRec, IN uint8_t ucUP,
		    IN PFN_TX_DONE_HANDLER pfTxDoneHandler);

void bssDumpBssInfo(IN struct ADAPTER *prAdapter,
		    IN uint8_t ucBssIndex);

void bssDetermineApBssInfoPhyTypeSet(IN struct ADAPTER
				     *prAdapter, IN u_int8_t fgIsPureAp,
				     OUT struct BSS_INFO *prBssInfo);
int8_t bssGetRxNss(IN struct ADAPTER *prAdapter,
	IN struct BSS_DESC *prBssDesc);
#if CFG_SUPPORT_IOT_AP_BLACKLIST
uint32_t bssGetIotApAction(IN struct ADAPTER *prAdapter,
	IN struct BSS_DESC *prBssDesc);
#endif
/*----------------------------------------------------------------------------*/
/* Routines for both IBSS(AdHoc) and BSS(AP)                                  */
/*----------------------------------------------------------------------------*/
void bssGenerateExtSuppRate_IE(IN struct ADAPTER *prAdapter,
			       IN struct MSDU_INFO *prMsduInfo);

void
bssBuildBeaconProbeRespFrameCommonIEs(IN struct MSDU_INFO
				      *prMsduInfo,
				      IN struct BSS_INFO *prBssInfo,
				      IN uint8_t *pucDestAddr);

void
bssComposeBeaconProbeRespFrameHeaderAndFF(
	IN uint8_t *pucBuffer,
	IN uint8_t *pucDestAddr,
	IN uint8_t *pucOwnMACAddress,
	IN uint8_t *pucBSSID, IN uint16_t u2BeaconInterval,
	IN uint16_t u2CapInfo);

uint32_t
bssSendBeaconProbeResponse(IN struct ADAPTER *prAdapter,
				IN uint8_t uBssIndex, IN uint8_t *pucDestAddr,
				IN uint32_t u4ControlFlags);

uint32_t bssProcessProbeRequest(IN struct ADAPTER *prAdapter,
				IN struct SW_RFB *prSwRfb);

void bssInitializeClientList(IN struct ADAPTER *prAdapter,
				IN struct BSS_INFO *prBssInfo);

void bssAddClient(IN struct ADAPTER *prAdapter,
				IN struct BSS_INFO *prBssInfo,
				IN struct STA_RECORD *prStaRec);

u_int8_t bssRemoveClient(IN struct ADAPTER *prAdapter,
				IN struct BSS_INFO *prBssInfo,
				IN struct STA_RECORD *prStaRec);

struct STA_RECORD *bssRemoveClientByMac(IN struct ADAPTER *prAdapter,
				IN struct BSS_INFO *prBssInfo,
				IN uint8_t *pucMac);

struct STA_RECORD *bssGetClientByMac(IN struct ADAPTER *prAdapter,
				IN struct BSS_INFO *prBssInfo,
				IN uint8_t *pucMac);

struct STA_RECORD *bssRemoveHeadClient(IN struct ADAPTER *prAdapter,
				IN struct BSS_INFO *prBssInfo);

uint32_t bssGetClientCount(IN struct ADAPTER *prAdapter,
				IN struct BSS_INFO *prBssInfo);

void bssDumpClientList(IN struct ADAPTER *prAdapter,
				IN struct BSS_INFO *prBssInfo);

void bssCheckClientList(IN struct ADAPTER *prAdapter,
				IN struct BSS_INFO *prBssInfo);

/*----------------------------------------------------------------------------*/
/* Routines for IBSS(AdHoc) only                                              */
/*----------------------------------------------------------------------------*/
void
ibssProcessMatchedBeacon(IN struct ADAPTER *prAdapter,
			 IN struct BSS_INFO *prBssInfo,
			 IN struct BSS_DESC *prBssDesc, IN uint8_t ucRCPI);

uint32_t ibssCheckCapabilityForAdHocMode(
		IN struct ADAPTER *prAdapter,
		IN struct BSS_DESC *prBssDesc,
		IN uint8_t uBssIndex);

void ibssInitForAdHoc(IN struct ADAPTER *prAdapter,
		      IN struct BSS_INFO *prBssInfo);

uint32_t bssUpdateBeaconContent(IN struct ADAPTER
				*prAdapter, IN uint8_t uBssIndex);

/*----------------------------------------------------------------------------*/
/* Routines for BSS(AP) only                                                  */
/*----------------------------------------------------------------------------*/
void bssInitForAP(IN struct ADAPTER *prAdapter,
		  IN struct BSS_INFO *prBssInfo, IN u_int8_t fgIsRateUpdate);

void bssUpdateDTIMCount(IN struct ADAPTER *prAdapter,
			IN uint8_t uBssIndex);

void bssSetTIMBitmap(IN struct ADAPTER *prAdapter,
		     IN struct BSS_INFO *prBssInfo, IN uint16_t u2AssocId);

/*link function to p2p module for txBcnIETable*/

/* WMM-2.2.2 WMM ACI to AC coding */
enum ENUM_ACI {
	ACI_BE = 0,
	ACI_BK = 1,
	ACI_VI = 2,
	ACI_VO = 3,
	ACI_NUM
};

enum ENUM_AC_PRIORITY {
	AC_BK_PRIORITY = 0,
	AC_BE_PRIORITY,
	AC_VI_PRIORITY,
	AC_VO_PRIORITY
};

#if (CFG_SUPPORT_HE_ER == 1)
struct EVENT_ER_TX_MODE {
	uint8_t ucBssInfoIdx;
	uint8_t ucErMode;
};

void bssProcessErTxModeEvent(IN struct ADAPTER *prAdapter,
	IN struct WIFI_EVENT *prEvent);
#endif

#endif /* _BSS_H */
