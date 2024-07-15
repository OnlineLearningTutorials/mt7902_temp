/* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause */
/*
 * Copyright (c) 2016 MediaTek Inc.
 */
/*
 ** Id:
 */

/*! \file   "roaming_fsm.c"
 *    \brief  This file defines the FSM for Roaming MODULE.
 *
 *    This file defines the FSM for Roaming MODULE.
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

#if CFG_SUPPORT_ROAMING
/*******************************************************************************
 *                              C O N S T A N T S
 *******************************************************************************
 */

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
static uint8_t *apucDebugRoamingState[ROAMING_STATE_NUM] = {
	(uint8_t *) DISP_STRING("IDLE"),
	(uint8_t *) DISP_STRING("DECISION"),
	(uint8_t *) DISP_STRING("DISCOVERY"),
	(uint8_t *) DISP_STRING("REQ_CAND_LIST"),
	(uint8_t *) DISP_STRING("ROAM")
};


/*******************************************************************************
 *                                 M A C R O S
 *******************************************************************************
 */

/*******************************************************************************
 *                   F U N C T I O N   D E C L A R A T I O N S
 *******************************************************************************
 */

/*******************************************************************************
 *                              F U N C T I O N S
 *******************************************************************************
 */
static void roamingWaitCandidateTimeout(IN struct ADAPTER *prAdapter,
	unsigned long ulParamPtr)
{
	uint8_t ucBssIndex = (uint8_t) ulParamPtr;

	DBGLOG(ROAMING, INFO,
	       "[%d] Time out, Waiting for neighbor response",
	       ucBssIndex);

	roamingFsmSteps(prAdapter, ROAMING_STATE_DISCOVERY, ucBssIndex);
}
/*----------------------------------------------------------------------------*/
/*!
 * @brief Initialize the value in ROAMING_FSM_INFO_T for ROAMING FSM operation
 *
 * @param [IN P_ADAPTER_T] prAdapter
 *
 * @return (none)
 */
/*----------------------------------------------------------------------------*/
void roamingFsmInit(IN struct ADAPTER *prAdapter, IN uint8_t ucBssIndex)
{
	struct ROAMING_INFO *prRoamingFsmInfo;
	struct CONNECTION_SETTINGS *prConnSettings;

	DBGLOG(ROAMING, LOUD,
	       "[%d]->roamingFsmInit(): Current Time = %d\n",
	       ucBssIndex,
	       kalGetTimeTick());

	prRoamingFsmInfo =
		aisGetRoamingInfo(prAdapter, ucBssIndex);
	prConnSettings =
		aisGetConnSettings(prAdapter, ucBssIndex);

	/* 4 <1> Initiate FSM */
	prRoamingFsmInfo->fgIsEnableRoaming =
		prConnSettings->fgIsEnableRoaming;
	prRoamingFsmInfo->eCurrentState = ROAMING_STATE_IDLE;
	prRoamingFsmInfo->rRoamingDiscoveryUpdateTime = 0;
	prRoamingFsmInfo->fgDrvRoamingAllow = TRUE;
	cnmTimerInitTimer(prAdapter, &prRoamingFsmInfo->rWaitCandidateTimer,
			  (PFN_MGMT_TIMEOUT_FUNC) roamingWaitCandidateTimeout,
			  (unsigned long) ucBssIndex);
}				/* end of roamingFsmInit() */

/*----------------------------------------------------------------------------*/
/*!
 * @brief Uninitialize the value in AIS_FSM_INFO_T for AIS FSM operation
 *
 * @param [IN P_ADAPTER_T] prAdapter
 *
 * @return (none)
 */
/*----------------------------------------------------------------------------*/
void roamingFsmUninit(IN struct ADAPTER *prAdapter, IN uint8_t ucBssIndex)
{
	struct ROAMING_INFO *prRoamingFsmInfo;

	DBGLOG(ROAMING, LOUD,
	       "[%d]->roamingFsmUninit(): Current Time = %d\n",
	       ucBssIndex,
	       kalGetTimeTick());

	prRoamingFsmInfo =
		aisGetRoamingInfo(prAdapter, ucBssIndex);

	prRoamingFsmInfo->eCurrentState = ROAMING_STATE_IDLE;
	cnmTimerStopTimer(prAdapter, &prRoamingFsmInfo->rWaitCandidateTimer);
}				/* end of roamingFsmUninit() */

/*----------------------------------------------------------------------------*/
/*!
 * @brief Send commands to firmware
 *
 * @param [IN P_ADAPTER_T]       prAdapter
 *        [IN P_ROAMING_PARAM_T] prParam
 *
 * @return (none)
 */
/*----------------------------------------------------------------------------*/
void roamingFsmSendCmd(IN struct ADAPTER *prAdapter,
	IN struct CMD_ROAMING_TRANSIT *prTransit)
{
	struct ROAMING_INFO *prRoamingFsmInfo;
	uint32_t rStatus;
	uint8_t ucBssIndex = prTransit->ucBssidx;

	DBGLOG(ROAMING, LOUD,
	       "[%d]->roamingFsmSendCmd(): Current Time = %d\n",
	       ucBssIndex,
	       kalGetTimeTick());

	prRoamingFsmInfo =
		aisGetRoamingInfo(prAdapter, ucBssIndex);

	rStatus = wlanSendSetQueryCmd(prAdapter,	/* prAdapter */
				      CMD_ID_ROAMING_TRANSIT,	/* ucCID */
				      TRUE,	/* fgSetQuery */
				      FALSE,	/* fgNeedResp */
				      FALSE,	/* fgIsOid */
				      NULL,	/* pfCmdDoneHandler */
				      NULL,	/* pfCmdTimeoutHandler */
				      sizeof(struct CMD_ROAMING_TRANSIT),
				      /* u4SetQueryInfoLen */
				      (uint8_t *)prTransit, /* pucInfoBuffer */
				      NULL,	/* pvSetQueryBuffer */
				      0	/* u4SetQueryBufferLen */
				     );

	/* ASSERT(rStatus == WLAN_STATUS_PENDING); */
}				/* end of roamingFsmSendCmd() */

/*----------------------------------------------------------------------------*/
/*!
 * @brief Update the recent time when ScanDone occurred
 *
 * @param [IN P_ADAPTER_T] prAdapter
 *
 * @return none
 */
/*----------------------------------------------------------------------------*/
void roamingFsmScanResultsUpdate(
	IN struct ADAPTER *prAdapter,
	IN uint8_t ucBssIndex)
{
	DBGLOG(ROAMING, LOUD,
		"[%d]->roamingFsmScanResultsUpdate(): Current Time = %d\n",
		ucBssIndex, kalGetTimeTick());

	/* try driver roaming */
	if (scanCheckNeedDriverRoaming(prAdapter, ucBssIndex)) {
		DBGLOG(ROAMING, INFO, "Request driver roaming");

		aisFsmIsRequestPending(prAdapter,
			AIS_REQUEST_ROAMING_SEARCH, TRUE, ucBssIndex);
		aisFsmIsRequestPending(prAdapter,
			AIS_REQUEST_ROAMING_CONNECT, TRUE, ucBssIndex);

		aisFsmInsertRequest(prAdapter,
			AIS_REQUEST_ROAMING_CONNECT, ucBssIndex);
	}
}				/* end of roamingFsmScanResultsUpdate() */

/*----------------------------------------------------------------------------*/
/*
 * @brief Check if need to do scan for roaming
 *
 * @param[out] fgIsNeedScan Set to TRUE if need to scan since
 *		there is roaming candidate in current scan result
 *		or skip roaming times > limit times
 * @return
 */
/*----------------------------------------------------------------------------*/
static u_int8_t roamingFsmIsNeedScan(
	IN struct ADAPTER *prAdapter,
	IN uint8_t ucBssIndex)
{
	struct AIS_SPECIFIC_BSS_INFO *asbi = NULL;
	struct LINK *prEssLink = NULL;
	u_int8_t fgIsNeedScan = TRUE;

	asbi = aisGetAisSpecBssInfo(prAdapter, ucBssIndex);
	if (asbi == NULL) {
		DBGLOG(ROAMING, WARN, "ais specific bss info is NULL\n");
		return TRUE;
	}

	prEssLink = &asbi->rCurEssLink;

#if CFG_SUPPORT_ROAMING_SKIP_ONE_AP
	/*
	 * Start skip roaming scan mechanism if only one ESSID AP
	 */
	if (prEssLink->u4NumElem == 1) {
		struct BSS_DESC *prBssDesc;

		/* Get current BssDesc */
		prBssDesc = aisGetTargetBssDesc(prAdapter, ucBssIndex);
		if (prBssDesc) {
			DBGLOG(ROAMING, INFO,
				"roamingFsmSteps: RCPI:%d RoamSkipTimes:%d\n",
				prBssDesc->ucRCPI, asbi->ucRoamSkipTimes);
			if (prBssDesc->ucRCPI > 90) {
				/* Set parameters related to Good Area */
				asbi->ucRoamSkipTimes = 3;
				asbi->fgGoodRcpiArea = TRUE;
				asbi->fgPoorRcpiArea = FALSE;
			} else {
				if (asbi->fgGoodRcpiArea) {
					asbi->ucRoamSkipTimes--;
				} else if (prBssDesc->ucRCPI > 67) {
					/*Set parameters related to Poor Area*/
					if (!asbi->fgPoorRcpiArea) {
						asbi->ucRoamSkipTimes = 2;
						asbi->fgPoorRcpiArea = TRUE;
						asbi->fgGoodRcpiArea = FALSE;
					} else {
						asbi->ucRoamSkipTimes--;
					}
				} else {
					asbi->fgPoorRcpiArea = FALSE;
					asbi->fgGoodRcpiArea = FALSE;
					asbi->ucRoamSkipTimes--;
				}
			}

			if (asbi->ucRoamSkipTimes == 0) {
				asbi->ucRoamSkipTimes = 3;
				asbi->fgPoorRcpiArea = FALSE;
				asbi->fgGoodRcpiArea = FALSE;
				DBGLOG(ROAMING, INFO, "Need Scan\n");
			} else {
				struct CMD_ROAMING_SKIP_ONE_AP cmd = {0};

				cmd.fgIsRoamingSkipOneAP = 1;

				wlanSendSetQueryCmd(prAdapter,
				    CMD_ID_SET_ROAMING_SKIP,
				    TRUE,
				    FALSE,
				    FALSE, NULL, NULL,
				    sizeof(struct CMD_ROAMING_SKIP_ONE_AP),
				    (uint8_t *)&cmd, NULL, 0);

				fgIsNeedScan = FALSE;
			}
		} else {
			DBGLOG(ROAMING, WARN, "Target BssDesc is NULL\n");
		}
	}
#endif
	return fgIsNeedScan;
}

/*----------------------------------------------------------------------------*/
/*!
 * @brief The Core FSM engine of ROAMING for AIS Infra.
 *
 * @param [IN P_ADAPTER_T]          prAdapter
 *        [IN ENUM_ROAMING_STATE_T] eNextState Enum value of next AIS STATE
 *
 * @return (none)
 */
/*----------------------------------------------------------------------------*/
void roamingFsmSteps(IN struct ADAPTER *prAdapter,
	IN enum ENUM_ROAMING_STATE eNextState,
	IN uint8_t ucBssIndex)
{
	struct ROAMING_INFO *prRoamingFsmInfo;
	u_int8_t fgIsTransition = (u_int8_t) FALSE;
#if CFG_SUPPORT_NCHO
	uint32_t u4ScnResultsTimeout =
		ROAMING_DISCOVERY_TIMEOUT_SEC;
	uint32_t u4ReqScan = FALSE;
#endif
	uint32_t u4State, u4NextState, u4PreState;


	prRoamingFsmInfo =
		aisGetRoamingInfo(prAdapter, ucBssIndex);
	u4State = prRoamingFsmInfo->eCurrentState;
	u4NextState = eNextState;

	do {
		if (u4State >= ROAMING_STATE_NUM ||
			u4NextState >= ROAMING_STATE_NUM) {
			DBGLOG(ROAMING, STATE, "Invalid stat eNextState[%d]\n",
				u4NextState);
			return;
		}
		/* Do entering Next State */
		DBGLOG(ROAMING, STATE,
		       "[ROAMING%d] TRANSITION: [%s] -> [%s]\n",
		       ucBssIndex,
		       apucDebugRoamingState[u4State],
		       apucDebugRoamingState[u4NextState]);

		/* NOTE(Kevin): This is the only place to
		 *    change the eCurrentState(except initial)
		 */
		u4PreState = u4State;
		u4State = u4NextState;
		prRoamingFsmInfo->eCurrentState = eNextState;

		fgIsTransition = (u_int8_t) FALSE;

		/* Do tasks of the State that we just entered */
		switch (prRoamingFsmInfo->eCurrentState) {
		/* NOTE(Kevin): we don't have to rearrange the sequence of
		 *   following switch case. Instead I would like to use a common
		 *   lookup table of array of function pointer
		 *   to speed up state search.
		 */
		case ROAMING_STATE_IDLE:
			break;
		case ROAMING_STATE_DECISION:
#if CFG_SUPPORT_DRIVER_ROAMING
			GET_CURRENT_SYSTIME(
				&prRoamingFsmInfo->rRoamingLastDecisionTime);
#endif
			prRoamingFsmInfo->eReason = ROAMING_REASON_POOR_RCPI;
			break;

		case ROAMING_STATE_DISCOVERY: {
#if CFG_SUPPORT_NCHO
			if (prAdapter->rNchoInfo.fgECHOEnabled == TRUE) {
				u4ScnResultsTimeout =
					prAdapter->rNchoInfo.u4RoamScanPeriod;
				DBGLOG(ROAMING, TRACE,
					"NCHO u4ScnResultsTimeout is %d\n",
				       u4ScnResultsTimeout);
			}

			if (CHECK_FOR_TIMEOUT(kalGetTimeTick(),
			      prRoamingFsmInfo->rRoamingDiscoveryUpdateTime,
			      SEC_TO_SYSTIME(u4ScnResultsTimeout))) {
				DBGLOG(ROAMING, LOUD,
					"DiscoveryUpdateTime Timeout");
				u4ReqScan =  TRUE;
			} else {
				DBGLOG(ROAMING, LOUD,
					"DiscoveryUpdateTime Updated");
				u4ReqScan = FALSE;
			}
			aisFsmRunEventRoamingDiscovery(prAdapter, u4ReqScan,
				ucBssIndex);
#else
			OS_SYSTIME rCurrentTime;
			u_int8_t fgIsNeedScan = FALSE;

			cnmTimerStopTimer(
				prAdapter,
				&prRoamingFsmInfo->rWaitCandidateTimer);

			GET_CURRENT_SYSTIME(&rCurrentTime);
			if (CHECK_FOR_TIMEOUT(rCurrentTime,
			      prRoamingFsmInfo->rRoamingDiscoveryUpdateTime,
			      SEC_TO_SYSTIME(ROAMING_DISCOVERY_TIMEOUT_SEC))) {
				DBGLOG(ROAMING, LOUD,
					"roamingFsmSteps: DiscoveryUpdateTime Timeout\n");

				fgIsNeedScan = roamingFsmIsNeedScan(prAdapter,
								ucBssIndex);
			}
			aisFsmRunEventRoamingDiscovery(
				prAdapter, fgIsNeedScan, ucBssIndex);

#endif /* CFG_SUPPORT_NCHO */
		}
		break;
		case ROAMING_STATE_REQ_CAND_LIST:
		{
#if CFG_SUPPORT_802_11K
			struct BSS_INFO *prBssInfo =
				aisGetAisBssInfo(prAdapter,
				ucBssIndex);
			struct BSS_DESC *prBssDesc =
				aisGetTargetBssDesc(prAdapter,
				ucBssIndex);
			/* if AP supports Neighbor AP report, then it can used
			 * to assist roaming candicate selection
			 */
			if (prBssInfo && prBssInfo->prStaRecOfAP) {
				if (prBssDesc &&
				    (prBssDesc->aucRrmCap[0] &
				     BIT(RRM_CAP_INFO_NEIGHBOR_REPORT_BIT))) {
					aisSendNeighborRequest(prAdapter,
						ucBssIndex);
					cnmTimerStartTimer(
						prAdapter,
						&prRoamingFsmInfo
							 ->rWaitCandidateTimer,
						100);
				}
			}
#endif
			fgIsTransition = TRUE;
			eNextState = ROAMING_STATE_DISCOVERY;
			u4NextState = eNextState;
			break;
		}
		case ROAMING_STATE_ROAM:
			break;

		default:
			ASSERT(0); /* Make sure we have handle all STATEs */
		}
	} while (fgIsTransition);

	return;

}				/* end of roamingFsmSteps() */

/*----------------------------------------------------------------------------*/
/*!
 * @brief Transit to Decision state after join completion
 *
 * @param [IN P_ADAPTER_T] prAdapter
 *
 * @return none
 */
/*----------------------------------------------------------------------------*/
void roamingFsmRunEventStart(IN struct ADAPTER *prAdapter,
	IN uint8_t ucBssIndex)
{
	struct ROAMING_INFO *prRoamingFsmInfo;
	enum ENUM_ROAMING_STATE eNextState;
	struct BSS_INFO *prAisBssInfo;
	struct CMD_ROAMING_TRANSIT rTransit;

	prRoamingFsmInfo =
		aisGetRoamingInfo(prAdapter, ucBssIndex);

	/* Check Roaming Conditions */
	if (!(prRoamingFsmInfo->fgIsEnableRoaming))
		return;


	prAisBssInfo = aisGetAisBssInfo(prAdapter,
		ucBssIndex);
	if (prAisBssInfo->eCurrentOPMode != OP_MODE_INFRASTRUCTURE)
		return;

	DBGLOG(ROAMING, EVENT,
	       "[%d] EVENT-ROAMING START: Current Time = %d\n",
	       ucBssIndex,
	       kalGetTimeTick());

	/* IDLE, ROAM -> DECISION */
	/* Errors as DECISION, DISCOVERY -> DECISION */
	if (!(prRoamingFsmInfo->eCurrentState == ROAMING_STATE_IDLE
	      || prRoamingFsmInfo->eCurrentState == ROAMING_STATE_ROAM))
		return;

	eNextState = ROAMING_STATE_DECISION;
	if (eNextState != prRoamingFsmInfo->eCurrentState) {
		memset(&rTransit, 0, sizeof(struct CMD_ROAMING_TRANSIT));
		rTransit.u2Event = ROAMING_EVENT_START;
		rTransit.u2Data = prAisBssInfo->ucBssIndex;
		rTransit.ucBssidx = ucBssIndex;
		roamingFsmSendCmd(prAdapter,
			(struct CMD_ROAMING_TRANSIT *) &rTransit);

		/* Step to next state */
		roamingFsmSteps(prAdapter, eNextState, ucBssIndex);
	}
}				/* end of roamingFsmRunEventStart() */

/*----------------------------------------------------------------------------*/
/*!
 * @brief Transit to Discovery state when deciding to find a candidate
 *
 * @param [IN P_ADAPTER_T] prAdapter
 *
 * @return none
 */
/*----------------------------------------------------------------------------*/
void roamingFsmRunEventDiscovery(IN struct ADAPTER *prAdapter,
	IN struct CMD_ROAMING_TRANSIT *prTransit)
{
	struct ROAMING_INFO *prRoamingFsmInfo;
	enum ENUM_ROAMING_STATE eNextState;
	uint8_t ucBssIndex = prTransit->ucBssidx;

	prRoamingFsmInfo =
		aisGetRoamingInfo(prAdapter, ucBssIndex);

	/* Check Roaming Conditions */
	if (!(prRoamingFsmInfo->fgIsEnableRoaming))
		return;

	DBGLOG(ROAMING, EVENT,
	       "[%d] EVENT-ROAMING DISCOVERY: Current Time = %d\n",
	       ucBssIndex,
	       kalGetTimeTick());

	/* DECISION -> DISCOVERY */
	/* Errors as IDLE, DISCOVERY, ROAM -> DISCOVERY */
	if (prRoamingFsmInfo->eCurrentState !=
	    ROAMING_STATE_DECISION)
		return;

	eNextState = ROAMING_STATE_REQ_CAND_LIST;
	/* DECISION -> DISCOVERY */
	if (eNextState != prRoamingFsmInfo->eCurrentState) {
		struct BSS_INFO *prAisBssInfo;
		struct BSS_DESC *prBssDesc;
		uint8_t arBssid[PARAM_MAC_ADDR_LEN];
		struct PARAM_SSID rSsid;
		struct AIS_FSM_INFO *prAisFsmInfo;
		struct CONNECTION_SETTINGS *prConnSettings;

		kalMemZero(&rSsid, sizeof(struct PARAM_SSID));
		prAisFsmInfo = aisGetAisFsmInfo(prAdapter, ucBssIndex);
		prConnSettings =
			aisGetConnSettings(prAdapter, ucBssIndex);

		/* sync. rcpi with firmware */
		prAisBssInfo =
			&(prAdapter->rWifiVar.arBssInfoPool[NETWORK_TYPE_AIS]);
		prBssDesc = prAisFsmInfo->prTargetBssDesc;
		if (prBssDesc) {
			COPY_SSID(rSsid.aucSsid, rSsid.u4SsidLen,
				  prBssDesc->aucSSID, prBssDesc->ucSSIDLen);
			COPY_MAC_ADDR(arBssid, prBssDesc->aucBSSID);
		} else  {
			COPY_SSID(rSsid.aucSsid, rSsid.u4SsidLen,
				  prConnSettings->aucSSID,
				  prConnSettings->ucSSIDLen);
			COPY_MAC_ADDR(arBssid, prConnSettings->aucBSSID);
		}
		prBssDesc = scanSearchBssDescByBssidAndSsid(prAdapter,
				arBssid, TRUE, &rSsid);
		if (prBssDesc) {
			prBssDesc->ucRCPI = (uint8_t)(prTransit->u2Data & 0xff);
			DBGLOG(ROAMING, INFO, "ucRCPI %u\n",
				prBssDesc->ucRCPI);
		}

		/* Save roaming reason code and PER value for AP selection */
		prRoamingFsmInfo->eReason = prTransit->eReason;
		if (prTransit->eReason == ROAMING_REASON_TX_ERR) {
			prRoamingFsmInfo->ucPER =
				(prTransit->u2Data >> 8) & 0xff;
			DBGLOG(ROAMING, INFO, "ucPER %u\n",
				prRoamingFsmInfo->ucPER);
		} else {
			prRoamingFsmInfo->ucPER = 0;
		}
		roamingFsmSteps(prAdapter, eNextState, ucBssIndex);
	}
}				/* end of roamingFsmRunEventDiscovery() */

/*----------------------------------------------------------------------------*/
/*!
 * @brief Transit to Roam state after Scan Done
 *
 * @param [IN P_ADAPTER_T] prAdapter
 *
 * @return none
 */
/*----------------------------------------------------------------------------*/
void roamingFsmRunEventRoam(IN struct ADAPTER *prAdapter,
	IN uint8_t ucBssIndex)
{
	struct ROAMING_INFO *prRoamingFsmInfo;
	enum ENUM_ROAMING_STATE eNextState;
	struct CMD_ROAMING_TRANSIT rTransit;

	prRoamingFsmInfo =
		aisGetRoamingInfo(prAdapter, ucBssIndex);

	/* Check Roaming Conditions */
	if (!(prRoamingFsmInfo->fgIsEnableRoaming))
		return;


	DBGLOG(ROAMING, EVENT,
	       "[%d] EVENT-ROAMING ROAM: Current Time = %d\n",
	       ucBssIndex,
	       kalGetTimeTick());

	/* IDLE, ROAM -> DECISION */
	/* Errors as IDLE, DECISION, ROAM -> ROAM */
	if (prRoamingFsmInfo->eCurrentState !=
	    ROAMING_STATE_DISCOVERY)
		return;

	memset(&rTransit, 0, sizeof(struct CMD_ROAMING_TRANSIT));

	eNextState = ROAMING_STATE_ROAM;
	/* DISCOVERY -> ROAM */
	if (eNextState != prRoamingFsmInfo->eCurrentState) {
		rTransit.u2Event = ROAMING_EVENT_ROAM;
		rTransit.ucBssidx = ucBssIndex;
		roamingFsmSendCmd(prAdapter,
			(struct CMD_ROAMING_TRANSIT *) &rTransit);

		/* Step to next state */
		roamingFsmSteps(prAdapter, eNextState, ucBssIndex);
	}
}				/* end of roamingFsmRunEventRoam() */

/*----------------------------------------------------------------------------*/
/*!
 * @brief Transit to Decision state as being failed to find out any candidate
 *
 * @param [IN P_ADAPTER_T] prAdapter
 *
 * @return none
 */
/*----------------------------------------------------------------------------*/
void roamingFsmRunEventFail(IN struct ADAPTER *prAdapter,
	IN uint32_t u4Param, IN uint8_t ucBssIndex)
{
	struct ROAMING_INFO *prRoamingFsmInfo;
	enum ENUM_ROAMING_STATE eNextState;
	struct CMD_ROAMING_TRANSIT rTransit;

	prRoamingFsmInfo =
		aisGetRoamingInfo(prAdapter, ucBssIndex);

	/* Check Roaming Conditions */
	if (!(prRoamingFsmInfo->fgIsEnableRoaming))
		return;


	DBGLOG(ROAMING, STATE,
	       "[%d] EVENT-ROAMING FAIL: reason %x Current Time = %d\n",
	       ucBssIndex,
	       u4Param, kalGetTimeTick());

	/* IDLE, ROAM -> DECISION */
	/* Errors as IDLE, DECISION, DISCOVERY -> DECISION */
	if (prRoamingFsmInfo->eCurrentState != ROAMING_STATE_ROAM)
		return;

	eNextState = ROAMING_STATE_DECISION;
	/* ROAM -> DECISION */
	if (eNextState != prRoamingFsmInfo->eCurrentState) {
		memset(&rTransit, 0, sizeof(struct CMD_ROAMING_TRANSIT));
		rTransit.u2Event = ROAMING_EVENT_FAIL;
		rTransit.u2Data = (uint16_t) (u4Param & 0xffff);
		rTransit.ucBssidx = ucBssIndex;
		roamingFsmSendCmd(prAdapter,
			(struct CMD_ROAMING_TRANSIT *) &rTransit);

		/* Step to next state */
		roamingFsmSteps(prAdapter, eNextState, ucBssIndex);
	}
}				/* end of roamingFsmRunEventFail() */

/*----------------------------------------------------------------------------*/
/*!
 * @brief Transit to Idle state as beging aborted by other moduels, AIS
 *
 * @param [IN P_ADAPTER_T] prAdapter
 *
 * @return none
 */
/*----------------------------------------------------------------------------*/
void roamingFsmRunEventAbort(IN struct ADAPTER *prAdapter,
	IN uint8_t ucBssIndex)
{
	struct ROAMING_INFO *prRoamingFsmInfo;
	enum ENUM_ROAMING_STATE eNextState;
	struct CMD_ROAMING_TRANSIT rTransit;

	prRoamingFsmInfo =
		aisGetRoamingInfo(prAdapter, ucBssIndex);

	/* Check Roaming Conditions */
	if (!(prRoamingFsmInfo->fgIsEnableRoaming))
		return;


	DBGLOG(ROAMING, EVENT,
	       "[%d] EVENT-ROAMING ABORT: Current Time = %d\n",
	       ucBssIndex,
	       kalGetTimeTick());

	eNextState = ROAMING_STATE_IDLE;
	/* IDLE, DECISION, DISCOVERY, ROAM -> IDLE */
	if (eNextState != prRoamingFsmInfo->eCurrentState) {
		memset(&rTransit, 0, sizeof(struct CMD_ROAMING_TRANSIT));
		rTransit.u2Event = ROAMING_EVENT_ABORT;
		rTransit.ucBssidx = ucBssIndex;
		roamingFsmSendCmd(prAdapter,
			(struct CMD_ROAMING_TRANSIT *) &rTransit);

		/* Step to next state */
		roamingFsmSteps(prAdapter, eNextState, ucBssIndex);
	}
}				/* end of roamingFsmRunEventAbort() */

/*----------------------------------------------------------------------------*/
/*!
 * @brief Process events from firmware
 *
 * @param [IN P_ADAPTER_T]       prAdapter
 *        [IN P_ROAMING_PARAM_T] prParam
 *
 * @return none
 */
/*----------------------------------------------------------------------------*/
uint32_t roamingFsmProcessEvent(IN struct ADAPTER *prAdapter,
	IN struct CMD_ROAMING_TRANSIT *prTransit)
{
	uint8_t ucBssIndex = prTransit->ucBssidx;
#if (CFG_SUPPORT_SUPPLICANT_SME == 1)
	int32_t rssi;
#endif

	struct net_device *prNetDev = NULL;

	DBGLOG(ROAMING, LOUD,
	       "[%d] ROAMING Process Events: Current Time = %d\n",
	       ucBssIndex,
	       kalGetTimeTick());
	prNetDev = (struct net_device *)wlanGetNetInterfaceByBssIdx(
		prAdapter->prGlueInfo, ucBssIndex);
	if (!prNetDev)
		prNetDev = prAdapter->prGlueInfo->prDevHandler;

	if (prTransit->u2Event == ROAMING_EVENT_DISCOVERY) {
		DBGLOG(ROAMING, STATE,
			"ROAMING_EVENT_DISCOVERY Data[%u] RCPI[%u] PER[%u] Thr[%u] Reason[%d] Time[%u]\n",
			prTransit->u2Data,
			(prTransit->u2Data) & 0xff,      /* L[8], RCPI */
			(prTransit->u2Data >> 8) & 0xff, /* H[8], PER */
			prTransit->u2RcpiLowThreshold,
			prTransit->eReason,
			prTransit->u4RoamingTriggerTime);
#if (CFG_SUPPORT_SUPPLICANT_SME == 1)
		if (prTransit->eReason == ROAMING_REASON_POOR_RCPI) {
			rssi = (uint8_t)(prTransit->u2Data & 0xff)/2 - 110;
			kalIndicateCqmRssiNotify(prNetDev,
				NL80211_CQM_RSSI_THRESHOLD_EVENT_LOW,
				rssi);
		}
		if (prTransit->eReason == ROAMING_REASON_TX_ERR) {
			/* TBD */
			/*
			 * kalIndicateCqmTxeNotify(
			 *	prNetDev,
			 *	prConnSettings->aucBSSID, 0, 0, 0);
			 */
		}
#else
		roamingFsmRunEventDiscovery(prAdapter, prTransit);
#endif
	}

	return WLAN_STATUS_SUCCESS;
}

#endif
