/* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause */
/*
 * Copyright (c) 2016 MediaTek Inc.
 */
/*
 ** Id: //Department/DaVinci/TRUNK/WiFi_P2P_Driver/mgmt/p2p_fsm.c#61
 */

/*! \file   "p2p_fsm.c"
 *  \brief  This file defines the FSM for P2P Module.
 *
 *  This file defines the FSM for P2P Module.
 */


/******************************************************************************
 *                         C O M P I L E R   F L A G S
 ******************************************************************************
 */

/******************************************************************************
 *                    E X T E R N A L   R E F E R E N C E S
 ******************************************************************************
 */
#include "precomp.h"

#if CFG_ENABLE_WIFI_DIRECT

/******************************************************************************
 *                              C O N S T A N T S
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
 *                                 M A C R O S
 ******************************************************************************
 */

/******************************************************************************
 *                   F U N C T I O N   D E C L A R A T I O N S
 ******************************************************************************
 */

/******************************************************************************
 *                              F U N C T I O N S
 ******************************************************************************
 */
static u_int8_t p2pFsmUseRoleIf(IN struct ADAPTER *prAdapter,
		uint8_t ucBssIdx)
{
	u_int8_t fgUseRoleInterface = FALSE;
	struct BSS_INFO *prBssInfo = (struct BSS_INFO *) NULL;

	prBssInfo = GET_BSS_INFO_BY_INDEX(prAdapter, ucBssIdx);

	if (prBssInfo == NULL) {
		DBGLOG(P2P, ERROR, "can not get bss info by bssIdx: %d",
				ucBssIdx);
		return FALSE;
	}

	if (ucBssIdx != prAdapter->ucP2PDevBssIdx) {
		if (IS_NET_ACTIVE(prAdapter, ucBssIdx)) {
			fgUseRoleInterface = TRUE;
			if (prBssInfo->eIftype != IFTYPE_P2P_CLIENT &&
				prBssInfo->eIftype != IFTYPE_P2P_GO &&
				!p2pFuncIsAPMode(
				prAdapter->rWifiVar
				.prP2PConnSettings
				[prBssInfo->u4PrivateData])) {
				DBGLOG(P2P, TRACE,
					"force use dev interface.\n");
				fgUseRoleInterface = FALSE;
			}
		} else {
			fgUseRoleInterface = FALSE;
		}
	} else {
		fgUseRoleInterface = FALSE;
	}

	DBGLOG(P2P, TRACE, "bss[%d %d], role: %d, use_role_if: %d\n",
			ucBssIdx,
			IS_NET_ACTIVE(prAdapter, ucBssIdx),
			prBssInfo->eIftype,
			fgUseRoleInterface);

	return fgUseRoleInterface;
}

void p2pFsmRunEventScanRequest(IN struct ADAPTER *prAdapter,
		IN struct MSG_HDR *prMsgHdr)
{
	struct MSG_P2P_SCAN_REQUEST *prP2pScanReqMsg =
		(struct MSG_P2P_SCAN_REQUEST *) NULL;

	do {
		ASSERT_BREAK((prAdapter != NULL) && (prMsgHdr != NULL));
		if ((prAdapter == NULL) || (prMsgHdr == NULL))
			break;

		prP2pScanReqMsg = (struct MSG_P2P_SCAN_REQUEST *) prMsgHdr;

		prAdapter->prP2pInfo->eConnState = P2P_CNN_NORMAL;

		if (prP2pScanReqMsg->ucBssIdx == prAdapter->ucP2PDevBssIdx)
			p2pDevFsmRunEventScanRequest(prAdapter, prMsgHdr);
		else
			p2pRoleFsmRunEventScanRequest(prAdapter, prMsgHdr);
	} while (FALSE);

	/*
	 * if (prAdapter && prMsgHdr)
	 *	cnmMemFree(prAdapter, prMsgHdr);
	 */
}				/* p2pDevFsmRunEventScanRequest */

/*----------------------------------------------------------------------------*/
/*!
 * \brief    This function is call when channel is granted
 *             by CNM module from FW.
 *
 * \param[in] prAdapter  Pointer of ADAPTER_T
 *
 * \return none
 */
/*----------------------------------------------------------------------------*/
void p2pFsmRunEventChGrant(IN struct ADAPTER *prAdapter,
		IN struct MSG_HDR *prMsgHdr)
{
	struct MSG_CH_GRANT *prMsgChGrant = (struct MSG_CH_GRANT *) NULL;
	struct BSS_INFO *prP2pBssInfo = (struct BSS_INFO *) NULL;

	do {
		ASSERT_BREAK((prAdapter != NULL) && (prMsgHdr != NULL));

		prMsgChGrant = (struct MSG_CH_GRANT *) prMsgHdr;

		prP2pBssInfo =
			GET_BSS_INFO_BY_INDEX(prAdapter,
				prMsgChGrant->ucBssIndex);

		prAdapter->prP2pInfo->eConnState = P2P_CNN_NORMAL;
		prAdapter->prP2pInfo->ucExtendChanFlag = 0;

		DBGLOG(P2P, TRACE, "P2P Run Event Channel Grant\n");

#if ((CFG_SISO_SW_DEVELOP == 1) || (CFG_SUPPORT_SPE_IDX_CONTROL == 1))
		/* Driver record granted CH in BSS info */
		prP2pBssInfo->fgIsGranted = TRUE;
		prP2pBssInfo->eBandGranted = prMsgChGrant->eRfBand;
		prP2pBssInfo->ucPrimaryChannelGranted =
			prMsgChGrant->ucPrimaryChannel;
#endif

		switch (prP2pBssInfo->eCurrentOPMode) {
		case OP_MODE_P2P_DEVICE:
			ASSERT(prP2pBssInfo->ucBssIndex
				== prAdapter->ucP2PDevBssIdx);

			p2pDevFsmRunEventChnlGrant(prAdapter,
				prMsgHdr,
				prAdapter->rWifiVar.prP2pDevFsmInfo);
			break;
		case OP_MODE_INFRASTRUCTURE:
		case OP_MODE_ACCESS_POINT:
			ASSERT(prP2pBssInfo->ucBssIndex
				< prAdapter->ucP2PDevBssIdx);

			p2pRoleFsmRunEventChnlGrant(prAdapter, prMsgHdr,
				P2P_ROLE_INDEX_2_ROLE_FSM_INFO(prAdapter,
					prP2pBssInfo->u4PrivateData));
			break;
		default:
			ASSERT(FALSE);
			break;
		}
	} while (FALSE);
}				/* p2pFsmRunEventChGrant */

void p2pFsmRunEventNetDeviceRegister(IN struct ADAPTER *prAdapter,
		IN struct MSG_HDR *prMsgHdr)
{
	struct MSG_P2P_NETDEV_REGISTER *prNetDevRegisterMsg =
		(struct MSG_P2P_NETDEV_REGISTER *) NULL;

	DBGLOG(P2P, TRACE, "p2pFsmRunEventNetDeviceRegister\n");

	prNetDevRegisterMsg = (struct MSG_P2P_NETDEV_REGISTER *) prMsgHdr;

	if (prNetDevRegisterMsg->fgIsEnable) {
		p2pSetMode((prNetDevRegisterMsg->ucMode == 1) ? TRUE : FALSE);
		if (p2pLaunch(prAdapter->prGlueInfo))
			ASSERT(prAdapter->fgIsP2PRegistered);
	} else {
		if (prAdapter->fgIsP2PRegistered)
			p2pRemove(prAdapter->prGlueInfo);
	}

	cnmMemFree(prAdapter, prMsgHdr);
}				/* p2pFsmRunEventNetDeviceRegister */

void p2pFsmRunEventUpdateMgmtFrame(IN struct ADAPTER *prAdapter,
		IN struct MSG_HDR *prMsgHdr)
{
	struct MSG_P2P_MGMT_FRAME_UPDATE *prP2pMgmtFrameUpdateMsg =
		(struct MSG_P2P_MGMT_FRAME_UPDATE *) NULL;

	DBGLOG(P2P, TRACE, "p2pFsmRunEventUpdateMgmtFrame\n");

	prP2pMgmtFrameUpdateMsg = (struct MSG_P2P_MGMT_FRAME_UPDATE *) prMsgHdr;

	switch (prP2pMgmtFrameUpdateMsg->eBufferType) {
	case ENUM_FRAME_TYPE_EXTRA_IE_BEACON:
		break;
	case ENUM_FRAME_TYPE_EXTRA_IE_ASSOC_RSP:
		break;
	case ENUM_FRAME_TYPE_EXTRA_IE_PROBE_RSP:
		break;
	case ENUM_FRAME_TYPE_PROBE_RSP_TEMPLATE:
		break;
	case ENUM_FRAME_TYPE_BEACON_TEMPLATE:
		break;
	default:
		break;
	}

	cnmMemFree(prAdapter, prMsgHdr);
}				/* p2pFsmRunEventUpdateMgmtFrame */

#if CFG_SUPPORT_WFD
void p2pFsmRunEventWfdSettingUpdate(IN struct ADAPTER *prAdapter,
		IN struct MSG_HDR *prMsgHdr)
{
	struct WFD_CFG_SETTINGS *prWfdCfgSettings =
		(struct WFD_CFG_SETTINGS *) NULL;
	struct MSG_WFD_CONFIG_SETTINGS_CHANGED *prMsgWfdCfgSettings =
		(struct MSG_WFD_CONFIG_SETTINGS_CHANGED *) NULL;
	uint32_t i;

	/* WLAN_STATUS rStatus =  WLAN_STATUS_SUCCESS; */

	DBGLOG(P2P, INFO, "p2pFsmRunEventWfdSettingUpdate\n");

	do {
		ASSERT_BREAK((prAdapter != NULL));

		if (prMsgHdr != NULL) {
			prMsgWfdCfgSettings =
				(struct MSG_WFD_CONFIG_SETTINGS_CHANGED *)
					prMsgHdr;
			prWfdCfgSettings =
				prMsgWfdCfgSettings->prWfdCfgSettings;
		} else {
			prWfdCfgSettings =
				&prAdapter->rWifiVar.rWfdConfigureSettings;
		}

		DBGLOG(P2P, INFO,
				"WFD Enalbe %x info %x state %x flag %x adv %x\n",
				prWfdCfgSettings->ucWfdEnable,
				prWfdCfgSettings->u2WfdDevInfo,
				(uint32_t) prWfdCfgSettings->u4WfdState,
				(uint32_t) prWfdCfgSettings->u4WfdFlag,
				(uint32_t) prWfdCfgSettings->u4WfdAdvancedFlag);

		if (prWfdCfgSettings->ucWfdEnable == 0)
			for (i = 0; i < KAL_P2P_NUM; i++) {
				if (prAdapter->prGlueInfo->prP2PInfo[i])
					prAdapter->prGlueInfo->prP2PInfo[i]
						->u2WFDIELen = 0;
			}

#if CFG_ENABLE_PER_STA_STATISTICS_LOG
		if (prAdapter->rWifiVar.aprP2pRoleFsmInfo[0]) {
			/* Assume role 0 */
			struct P2P_ROLE_FSM_INFO *prP2pRoleFsmInfo =
				(struct P2P_ROLE_FSM_INFO *)
				prAdapter->rWifiVar.aprP2pRoleFsmInfo[0];

			if (prWfdCfgSettings->ucWfdEnable == 1)
				cnmTimerStartTimer(prAdapter,
					&(prP2pRoleFsmInfo
					->rP2pRoleFsmGetStatisticsTimer),
					(3 * P2P_ROLE_GET_STATISTICS_TIME));
			else
				cnmTimerStopTimer(prAdapter,
					&prP2pRoleFsmInfo
					->rP2pRoleFsmGetStatisticsTimer);
		}
#endif

	} while (FALSE);

	if (prMsgHdr)
		cnmMemFree(prAdapter, prMsgHdr);
}

/* p2pFsmRunEventWfdSettingUpdate */

#endif /* CFG_SUPPORT_WFD */


/*----------------------------------------------------------------------------*/
/*!
 * \brief    This function is used to handle scan done event
 *             during Device Discovery.
 *
 * \param[in] prAdapter  Pointer of ADAPTER_T
 *
 * \return none
 */
/*----------------------------------------------------------------------------*/
void p2pFsmRunEventScanDone(IN struct ADAPTER *prAdapter,
		IN struct MSG_HDR *prMsgHdr)
{
	struct MSG_SCN_SCAN_DONE *prScanDoneMsg =
		(struct MSG_SCN_SCAN_DONE *) NULL;
	struct BSS_INFO *prP2pBssInfo = (struct BSS_INFO *) NULL;

	prScanDoneMsg = (struct MSG_SCN_SCAN_DONE *) prMsgHdr;

	prP2pBssInfo =
		GET_BSS_INFO_BY_INDEX(prAdapter, prScanDoneMsg->ucBssIndex);

	if (prAdapter->fgIsP2PRegistered == FALSE) {
		DBGLOG(P2P, TRACE,
			"P2P BSS Info is removed, break p2pFsmRunEventScanDone\n");

		cnmMemFree(prAdapter, prMsgHdr);
		return;
	}

	DBGLOG(P2P, TRACE, "P2P Scan Done Event\n");

	switch (prP2pBssInfo->eCurrentOPMode) {
	case OP_MODE_P2P_DEVICE:
		ASSERT(prP2pBssInfo->ucBssIndex == prAdapter->ucP2PDevBssIdx);
		p2pDevFsmRunEventScanDone(prAdapter,
			prMsgHdr,
			prAdapter->rWifiVar.prP2pDevFsmInfo);
		break;
	case OP_MODE_INFRASTRUCTURE:
	case OP_MODE_ACCESS_POINT:
		ASSERT(prP2pBssInfo->ucBssIndex < prAdapter->ucP2PDevBssIdx);
		p2pRoleFsmRunEventScanDone(prAdapter, prMsgHdr,
			P2P_ROLE_INDEX_2_ROLE_FSM_INFO(prAdapter,
				prP2pBssInfo->u4PrivateData));
		break;
	default:
		ASSERT(FALSE);
		break;
	}
}				/* p2pFsmRunEventScanDone */

void p2pFsmRunEventMgmtFrameTx(IN struct ADAPTER *prAdapter,
		IN struct MSG_HDR *prMsgHdr)
{
	struct MSG_MGMT_TX_REQUEST *prMgmtTxMsg =
			(struct MSG_MGMT_TX_REQUEST *) NULL;

	do {
		if ((prAdapter == NULL) || (prMsgHdr == NULL))
			break;

		prMgmtTxMsg = (struct MSG_MGMT_TX_REQUEST *) prMsgHdr;

		if (p2pFsmUseRoleIf(prAdapter, prMgmtTxMsg->ucBssIdx)) {
			p2pRoleFsmRunEventMgmtTx(prAdapter, prMsgHdr);
		} else {
			prMgmtTxMsg->ucBssIdx = prAdapter->ucP2PDevBssIdx;
			p2pDevFsmRunEventMgmtTx(prAdapter, prMsgHdr);
		}
	} while (FALSE);
}				/* p2pFsmRunEventMgmtFrameTx */

void p2pFsmRunEventTxCancelWait(IN struct ADAPTER *prAdapter,
		IN struct MSG_HDR *prMsgHdr)
{
	struct MSG_CANCEL_TX_WAIT_REQUEST *prCancelTxWaitMsg =
			(struct MSG_CANCEL_TX_WAIT_REQUEST *) NULL;

	do {
		if ((prAdapter == NULL) || (prMsgHdr == NULL))
			break;

		prCancelTxWaitMsg =
				(struct MSG_CANCEL_TX_WAIT_REQUEST *) prMsgHdr;

		if (p2pFsmUseRoleIf(prAdapter, prCancelTxWaitMsg->ucBssIdx)) {
			p2pRoleFsmRunEventTxCancelWait(prAdapter, prMsgHdr);
		} else {
			prCancelTxWaitMsg->ucBssIdx = prAdapter->ucP2PDevBssIdx;
			p2pDevFsmRunEventTxCancelWait(prAdapter, prMsgHdr);
		}
	} while (FALSE);

}				/* p2pFsmRunEventTxCancelWait */

struct BSS_DESC *p2pGetTargetBssDesc(
	IN struct ADAPTER *prAdapter,
	IN uint8_t ucBssIndex) {

	uint8_t i = 0;

	for (i = 0 ; i < BSS_P2P_NUM; i++) {
		if (!prAdapter->rWifiVar.aprP2pRoleFsmInfo[i])
			continue;

		if (prAdapter->rWifiVar.aprP2pRoleFsmInfo[i]->ucBssIndex
			== ucBssIndex)
			break;
	}

	if (i >= BSS_P2P_NUM)
		return NULL;

	return prAdapter->rWifiVar.aprP2pRoleFsmInfo[i]
		->rJoinInfo.prTargetBssDesc;
}

#endif /* CFG_ENABLE_WIFI_DIRECT */
