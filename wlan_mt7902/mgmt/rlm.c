/* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause */
/*
 * Copyright (c) 2016 MediaTek Inc.
 */
/*
 ** Id: //Department/DaVinci/BRANCHES/MT6620_WIFI_DRIVER_V2_3/mgmt/rlm.c#3
 */

/*! \file   "rlm.c"
 *    \brief
 *
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

/*******************************************************************************
 *                              C O N S T A N T S
 *******************************************************************************
 */

/* Retry limit of sending operation notification frame */
#define OPERATION_NOTICATION_TX_LIMIT	2

/*******************************************************************************
 *                             D A T A   T Y P E S
 *******************************************************************************
 */
enum ENUM_OP_NOTIFY_STATE_T {
	OP_NOTIFY_STATE_KEEP = 0, /* Won't change OP mode */
	OP_NOTIFY_STATE_SENDING,  /* Sending OP notification frame */
	OP_NOTIFY_STATE_SUCCESS,  /* OP notification Tx success */
	OP_NOTIFY_STATE_FAIL,     /* OP notification Tx fail(over retry limit)*/
	OP_NOTIFY_STATE_NUM
};

/*******************************************************************************
 *                            P U B L I C   D A T A
 *******************************************************************************
 */

/*******************************************************************************
 *                           P R I V A T E   D A T A
 *******************************************************************************
 */
/*
** Should Not Force to BW 20 after Channel Switch.
** Enable for DFS Certification
*/
#ifdef CFG_DFS_CHSW_FORCE_BW20
u_int8_t g_fgHasChannelSwitchIE = FALSE;
#endif
u_int8_t g_fgHasStopTx = FALSE;

#if CFG_SUPPORT_CAL_RESULT_BACKUP_TO_HOST
struct RLM_CAL_RESULT_ALL_V2 g_rBackupCalDataAllV2;
#endif

#if CFG_SUPPORT_ONE_TIME_CAL
struct ONE_TIME_CAL_GROUP_DEF_ENTRY GROUP_FREQ_DEF_ARR[] = {
	{0, 2300, 2499},
	{1, 4850, 5149},
	{2, 5150, 5250},
	{3, 5251, 5330},
	{4, 5331, 5490},
	{5, 5491, 5570},
	{6, 5571, 5650},
	{7, 5651, 5734},
	{8, 5735, 5925},
#if (CFG_SUPPORT_WIFI_6G == 1)
	{9, 5926, 6065},
	{10, 6066, 6185},
	{11, 6186, 6305},
	{12, 6306, 6425},
	{13, 6426, 6545},
	{14, 6546, 6665},
	{15, 6666, 6785},
	{16, 6786, 6905},
	{17, 6906, 7025},
	{18, 7026, 0xFF},
#endif
};
#endif

/*******************************************************************************
 *                                 M A C R O S
 *******************************************************************************
 */

/*******************************************************************************
 *                   F U N C T I O N   D E C L A R A T I O N S
 *******************************************************************************
 */
static void rlmFillHtCapIE(struct ADAPTER *prAdapter,
			   struct BSS_INFO *prBssInfo,
			   struct MSDU_INFO *prMsduInfo);

static void rlmFillExtCapIE(struct ADAPTER *prAdapter,
			    struct BSS_INFO *prBssInfo,
			    struct MSDU_INFO *prMsduInfo);

static void rlmFillHtOpIE(struct ADAPTER *prAdapter, struct BSS_INFO *prBssInfo,
			  struct MSDU_INFO *prMsduInfo);

static uint8_t rlmRecIeInfoForClient(struct ADAPTER *prAdapter,
				     struct BSS_INFO *prBssInfo, uint8_t *pucIE,
				     uint16_t u2IELength);

static u_int8_t rlmRecBcnFromNeighborForClient(struct ADAPTER *prAdapter,
					       struct BSS_INFO *prBssInfo,
					       struct SW_RFB *prSwRfb,
					       uint8_t *pucIE,
					       uint16_t u2IELength);

static u_int8_t rlmRecBcnInfoForClient(struct ADAPTER *prAdapter,
				       struct BSS_INFO *prBssInfo,
				       struct SW_RFB *prSwRfb, uint8_t *pucIE,
				       uint16_t u2IELength);

static void rlmBssReset(struct ADAPTER *prAdapter, struct BSS_INFO *prBssInfo);

#if CFG_SUPPORT_802_11AC
static void rlmFillVhtCapIE(struct ADAPTER *prAdapter,
			    struct BSS_INFO *prBssInfo,
			    struct MSDU_INFO *prMsduInfo);
static void rlmFillVhtOpNotificationIE(struct ADAPTER *prAdapter,
				       struct BSS_INFO *prBssInfo,
				       struct MSDU_INFO *prMsduInfo,
				       u_int8_t fgIsMaxCap);

#endif

/* Operating BW/Nss change and notification */
static void rlmOpModeTxDoneHandler(struct ADAPTER *prAdapter,
				   struct MSDU_INFO *prMsduInfo,
				   uint8_t ucOpChangeType,
				   u_int8_t fgIsSuccess);
static void rlmChangeOwnOpInfo(struct ADAPTER *prAdapter,
			       struct BSS_INFO *prBssInfo);
static void rlmCompleteOpModeChange(struct ADAPTER *prAdapter,
				    struct BSS_INFO *prBssInfo,
				    u_int8_t fgIsSuccess);
static void rlmRollbackOpChangeParam(struct BSS_INFO *prBssInfo,
				     u_int8_t fgIsRollbackBw,
				     u_int8_t fgIsRollbackNss);
static uint8_t rlmGetOpModeBwByVhtAndHtOpInfo(struct BSS_INFO *prBssInfo);
static u_int8_t rlmCheckOpChangeParamValid(struct ADAPTER *prAdapter,
					   struct BSS_INFO *prBssInfo,
					   uint8_t ucChannelWidth,
					   uint8_t ucOpRxNss,
					   uint8_t ucOpTxNss);
static void rlmRecOpModeBwForClient(uint8_t ucVhtOpModeChannelWidth,
				    struct BSS_INFO *prBssInfo);

/*******************************************************************************
 *                              F U N C T I O N S
 *******************************************************************************
 */

/*----------------------------------------------------------------------------*/
/*!
 * \brief
 *
 * \param[in]
 *
 * \return none
 */
/*----------------------------------------------------------------------------*/
void rlmFsmEventInit(struct ADAPTER *prAdapter)
{
	ASSERT(prAdapter);

	/* Note: assume struct TIMER structures are reset to zero or stopped
	 * before invoking this function.
	 */

	/* Initialize OBSS FSM */
	rlmObssInit(prAdapter);

#if CFG_SUPPORT_PWR_LIMIT_COUNTRY
	rlmDomainCheckCountryPowerLimitTable(prAdapter);
#endif

#ifdef CFG_DFS_CHSW_FORCE_BW20
	g_fgHasChannelSwitchIE = FALSE;
#endif
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief
 *
 * \param[in]
 *
 * \return none
 */
/*----------------------------------------------------------------------------*/
void rlmFsmEventUninit(struct ADAPTER *prAdapter)
{
	struct BSS_INFO *prBssInfo;
	uint8_t i;

	ASSERT(prAdapter);

	for (i = 0; i < prAdapter->ucHwBssIdNum; i++) {
		prBssInfo = prAdapter->aprBssInfo[i];

		/* Note: all RLM timers will also be stopped.
		 *       Now only one OBSS scan timer.
		 */
		rlmBssReset(prAdapter, prBssInfo);
	}
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief For association request, power capability
 *
 * \param[in]
 *
 * \return none
 */
/*----------------------------------------------------------------------------*/
void rlmReqGeneratePowerCapIE(struct ADAPTER *prAdapter,
			      struct MSDU_INFO *prMsduInfo)
{
	uint8_t *pucBuffer;

	ASSERT(prAdapter);
	ASSERT(prMsduInfo);

	pucBuffer =
		(uint8_t *)(prMsduInfo->prPacket + prMsduInfo->u2FrameLength);

	POWER_CAP_IE(pucBuffer)->ucId = ELEM_ID_PWR_CAP;
	POWER_CAP_IE(pucBuffer)->ucLength = ELEM_MAX_LEN_POWER_CAP;
	POWER_CAP_IE(pucBuffer)->cMinTxPowerCap = RLM_MIN_TX_PWR;
	POWER_CAP_IE(pucBuffer)->cMaxTxPowerCap = RLM_MAX_TX_PWR;

	prMsduInfo->u2FrameLength += IE_SIZE(pucBuffer);
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief For association request, supported channels
 *
 * \param[in]
 *
 * \return none
 */
/*----------------------------------------------------------------------------*/
void rlmReqGenerateSupportedChIE(struct ADAPTER *prAdapter,
				 struct MSDU_INFO *prMsduInfo)
{
	uint8_t *pucBuffer;
	struct BSS_INFO *prBssInfo;
	struct RF_CHANNEL_INFO *par2gChannelList;
	struct RF_CHANNEL_INFO *par5gChannelList;
	uint8_t ucNumOf2gChannel = 0;
	uint8_t ucNumOf5gChannel = 0;
	uint8_t ucChIdx = 0;
	uint8_t ucIdx = 0;
	const uint16_t u2ChanList2gSize =
		sizeof(struct RF_CHANNEL_INFO) * MAX_2G_BAND_CHN_NUM;
	const uint16_t u2ChanList5gSize =
		sizeof(struct RF_CHANNEL_INFO) * MAX_5G_BAND_CHN_NUM;

	prBssInfo = GET_BSS_INFO_BY_INDEX(prAdapter, prMsduInfo->ucBssIndex);

	/* We should add supported channels IE in assoc/reassoc request
	 * if the spectrum management bit is set to 1 in Capability Info
	 * field, or the connection will be rejected by Marvell APs in
	 * some TGn items. (e.g. 5.2.3). Spectrum management related
	 * feature (802.11h) is for 5G band.
	 */
	if (!prBssInfo || prBssInfo->eBand != BAND_5G)
		return;

	par2gChannelList = (struct RF_CHANNEL_INFO *)kalMemAlloc(
			u2ChanList2gSize, VIR_MEM_TYPE);
	par5gChannelList = (struct RF_CHANNEL_INFO *)kalMemAlloc(
			u2ChanList5gSize, VIR_MEM_TYPE);
	if (!par2gChannelList || !par5gChannelList)
		goto end;
	kalMemZero(par2gChannelList, u2ChanList2gSize);
	kalMemZero(par5gChannelList, u2ChanList5gSize);

	pucBuffer =
		(uint8_t *)(prMsduInfo->prPacket + prMsduInfo->u2FrameLength);

	rlmDomainGetChnlList(prAdapter, BAND_2G4, TRUE, MAX_2G_BAND_CHN_NUM,
			     &ucNumOf2gChannel, par2gChannelList);
	rlmDomainGetChnlList(prAdapter, BAND_5G, TRUE, MAX_5G_BAND_CHN_NUM,
			     &ucNumOf5gChannel, par5gChannelList);

	SUP_CH_IE(pucBuffer)->ucId = ELEM_ID_SUP_CHS;
	SUP_CH_IE(pucBuffer)->ucLength =
		(ucNumOf2gChannel + ucNumOf5gChannel) * 2;

	for (ucIdx = 0; ucIdx < ucNumOf2gChannel; ucIdx++, ucChIdx += 2) {
		SUP_CH_IE(pucBuffer)->ucChannelNum[ucChIdx] =
			par2gChannelList[ucIdx].ucChannelNum;
		SUP_CH_IE(pucBuffer)->ucChannelNum[ucChIdx + 1] = 1;
	}

	for (ucIdx = 0; ucIdx < ucNumOf5gChannel; ucIdx++, ucChIdx += 2) {
		SUP_CH_IE(pucBuffer)->ucChannelNum[ucChIdx] =
			par5gChannelList[ucIdx].ucChannelNum;
		SUP_CH_IE(pucBuffer)->ucChannelNum[ucChIdx + 1] = 1;
	}

	prMsduInfo->u2FrameLength += IE_SIZE(pucBuffer);

end:
	if (par2gChannelList)
		kalMemFree(par2gChannelList, VIR_MEM_TYPE,
			u2ChanList2gSize);
	if (par5gChannelList)
		kalMemFree(par5gChannelList, VIR_MEM_TYPE,
			u2ChanList5gSize);
	return;
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief For probe request, association request
 *
 * \param[in]
 *
 * \return none
 */
/*----------------------------------------------------------------------------*/
void rlmReqGenerateHtCapIE(struct ADAPTER *prAdapter,
			   struct MSDU_INFO *prMsduInfo)
{
	struct BSS_INFO *prBssInfo;
	struct STA_RECORD *prStaRec;

	ASSERT(prAdapter);
	ASSERT(prMsduInfo);

	prBssInfo = prAdapter->aprBssInfo[prMsduInfo->ucBssIndex];
	if (!prBssInfo)
		return;

	prStaRec = cnmGetStaRecByIndex(prAdapter, prMsduInfo->ucStaRecIndex);

	if ((prAdapter->rWifiVar.ucAvailablePhyTypeSet &
	     PHY_TYPE_SET_802_11N) &&
	    (!prStaRec || (prStaRec->ucPhyTypeSet & PHY_TYPE_SET_802_11N)))
		rlmFillHtCapIE(prAdapter, prBssInfo, prMsduInfo);
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief For probe request, association request
 *
 * \param[in]
 *
 * \return none
 */
/*----------------------------------------------------------------------------*/
void rlmReqGenerateExtCapIE(struct ADAPTER *prAdapter,
			    struct MSDU_INFO *prMsduInfo)
{
	struct BSS_INFO *prBssInfo;
	struct STA_RECORD *prStaRec;
#if CFG_SUPPORT_PASSPOINT
	struct HS20_INFO *prHS20Info;
#endif

	ASSERT(prAdapter);
	ASSERT(prMsduInfo);

	prBssInfo = prAdapter->aprBssInfo[prMsduInfo->ucBssIndex];
	if (!prBssInfo)
		return;

#if CFG_SUPPORT_PASSPOINT
	prHS20Info = aisGetHS20Info(prAdapter,
		prBssInfo->ucBssIndex);
	if (!prHS20Info)
		return;
#endif

	prStaRec = cnmGetStaRecByIndex(prAdapter, prMsduInfo->ucStaRecIndex);

#if (CFG_SUPPORT_WIFI_6G == 1)
	if (!prStaRec || (prStaRec->ucPhyTypeSet &
		(PHY_TYPE_SET_802_11N | PHY_TYPE_SET_802_11AX)))
#else
	if ((prAdapter->rWifiVar.ucAvailablePhyTypeSet &
		PHY_TYPE_SET_802_11N) &&
		(!prStaRec || (prStaRec->ucPhyTypeSet & PHY_TYPE_SET_802_11N)))
#endif
		rlmFillExtCapIE(prAdapter, prBssInfo, prMsduInfo);
#if CFG_SUPPORT_PASSPOINT
	else if (prHS20Info->fgConnectHS20AP == TRUE)
		hs20FillExtCapIE(prAdapter, prBssInfo, prMsduInfo);
#endif /* CFG_SUPPORT_PASSPOINT */
}

#if (CFG_SUPPORT_SUPPLICANT_MBO == 1)
/*----------------------------------------------------------------------------*/
/*!
* \brief For probe request, association request
*
* \param[in]
*
* \return none
*/
/*----------------------------------------------------------------------------*/
void rlmReqGenerateSupOpClassIE(struct ADAPTER *prAdapter,
	struct MSDU_INFO *prMsduInfo)
{
	uint8_t *pucBuffer;

	ASSERT(prAdapter);
	ASSERT(prMsduInfo);

	if (prMsduInfo->ucBssIndex != KAL_NETWORK_TYPE_AIS_INDEX) {
		pr_info("[%s] prMsduInfo->ucBssIndex(%d) is not KAL_NETWORK_TYPE_AIS_INDEX\n",
			__func__, prMsduInfo->ucBssIndex);
		return;
	}

	pucBuffer = (uint8_t *)
		((unsigned long) prMsduInfo->prPacket +
		(unsigned long) prMsduInfo->u2FrameLength);

	if (prAdapter->prGlueInfo->u2SupOpClassIELen) {
		kalMemCopy(pucBuffer,
			&prAdapter->prGlueInfo->aucSupOpClassIE,
			prAdapter->prGlueInfo->u2SupOpClassIELen);
		prMsduInfo->u2FrameLength +=
			prAdapter->prGlueInfo->u2SupOpClassIELen;
	}
}

uint32_t rlmReqGetSupOpClassIELen(
	struct ADAPTER *prAdapter,
	uint8_t ucBssIndex,
	struct STA_RECORD *prStaRec)
{
	uint32_t u4IELen = 0;

	if (prAdapter->prGlueInfo->u2SupOpClassIELen)
		u4IELen =
			ELEM_HDR_LEN + prAdapter->prGlueInfo->u2SupOpClassIELen;

	return u4IELen;
}

/*----------------------------------------------------------------------------*/
/*!
* \brief For probe request, association request
*
* \param[in]
*
* \return none
*/
/*----------------------------------------------------------------------------*/
void rlmReqGenerateMboIE(struct ADAPTER *prAdapter,
	struct MSDU_INFO *prMsduInfo)
{
	uint8_t *pucBuffer;

	ASSERT(prAdapter);
	ASSERT(prMsduInfo);

	if (prMsduInfo->ucBssIndex != KAL_NETWORK_TYPE_AIS_INDEX) {
		pr_info("[%s] prMsduInfo->ucBssIndex(%d) is not KAL_NETWORK_TYPE_AIS_INDEX\n",
			__func__, prMsduInfo->ucBssIndex);
		return;
	}

	pucBuffer = (uint8_t *)
		((unsigned long) prMsduInfo->prPacket +
		(unsigned long) prMsduInfo->u2FrameLength);

	if (prAdapter->prGlueInfo->u2MboIELen) {
		kalMemCopy(pucBuffer,
			&prAdapter->prGlueInfo->aucMboIE,
			prAdapter->prGlueInfo->u2MboIELen);
		prMsduInfo->u2FrameLength +=
			prAdapter->prGlueInfo->u2MboIELen;
	}
}

uint32_t rlmReqGetMboIELen(
	struct ADAPTER *prAdapter,
	uint8_t ucBssIndex,
	struct STA_RECORD *prStaRec)
{
	uint32_t u4IELen = 0;

	if (prAdapter->prGlueInfo->u2MboIELen)
		u4IELen = ELEM_HDR_LEN + prAdapter->prGlueInfo->u2MboIELen;

	return u4IELen;
}
#endif /* CFG_SUPPORT_SUPPLICANT_MBO */

/*----------------------------------------------------------------------------*/
/*!
 * \brief For probe response (GO, IBSS) and association response
 *
 * \param[in]
 *
 * \return none
 */
/*----------------------------------------------------------------------------*/
void rlmRspGenerateHtCapIE(struct ADAPTER *prAdapter,
			   struct MSDU_INFO *prMsduInfo)
{
	struct BSS_INFO *prBssInfo;
	struct STA_RECORD *prStaRec;
	uint8_t ucPhyTypeSet;

	ASSERT(prAdapter);
	ASSERT(prMsduInfo);

	prBssInfo = prAdapter->aprBssInfo[prMsduInfo->ucBssIndex];
	if (!prBssInfo)
		return;

	if (!IS_BSS_ACTIVE(prBssInfo))
		return;

	prStaRec = cnmGetStaRecByIndex(prAdapter, prMsduInfo->ucStaRecIndex);

	/* Decide PHY type set source */
	if (prStaRec) {
		/* Get PHY type set from target STA */
		ucPhyTypeSet = prStaRec->ucPhyTypeSet;
	} else {
		/* Get PHY type set from current BSS */
		ucPhyTypeSet = prBssInfo->ucPhyTypeSet;
	}

	if (RLM_NET_IS_11N(prBssInfo) &&
	    (ucPhyTypeSet & PHY_TYPE_SET_802_11N) &&
		(!prBssInfo->fgIsWepCipherGroup))
		rlmFillHtCapIE(prAdapter, prBssInfo, prMsduInfo);
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief For probe response (GO, IBSS) and association response
 *
 * \param[in]
 *
 * \return none
 */
/*----------------------------------------------------------------------------*/
void rlmRspGenerateExtCapIE(struct ADAPTER *prAdapter,
			    struct MSDU_INFO *prMsduInfo)
{
	struct BSS_INFO *prBssInfo;
	struct STA_RECORD *prStaRec;
	uint8_t ucPhyTypeSet;

	ASSERT(prAdapter);
	ASSERT(prMsduInfo);

	prBssInfo = prAdapter->aprBssInfo[prMsduInfo->ucBssIndex];
	if (!prBssInfo)
		return;

	if (!IS_BSS_ACTIVE(prBssInfo))
		return;

	prStaRec = cnmGetStaRecByIndex(prAdapter, prMsduInfo->ucStaRecIndex);

	/* Decide PHY type set source */
	if (prStaRec) {
		/* Get PHY type set from target STA */
		ucPhyTypeSet = prStaRec->ucPhyTypeSet;
	} else {
		/* Get PHY type set from current BSS */
		ucPhyTypeSet = prBssInfo->ucPhyTypeSet;
	}

	if (RLM_NET_IS_11N(prBssInfo) && (ucPhyTypeSet & PHY_TYPE_SET_802_11N))
		rlmFillExtCapIE(prAdapter, prBssInfo, prMsduInfo);
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief For probe response (GO, IBSS) and association response
 *
 * \param[in]
 *
 * \return none
 */
/*----------------------------------------------------------------------------*/
void rlmRspGenerateHtOpIE(struct ADAPTER *prAdapter,
			  struct MSDU_INFO *prMsduInfo)
{
	struct BSS_INFO *prBssInfo;
	struct STA_RECORD *prStaRec;
	uint8_t ucPhyTypeSet;

	ASSERT(prAdapter);
	ASSERT(prMsduInfo);

	prStaRec = cnmGetStaRecByIndex(prAdapter, prMsduInfo->ucStaRecIndex);

	prBssInfo = prAdapter->aprBssInfo[prMsduInfo->ucBssIndex];
	if (!prBssInfo)
		return;

	if (!IS_BSS_ACTIVE(prBssInfo))
		return;

	/* Decide PHY type set source */
	if (prStaRec) {
		/* Get PHY type set from target STA */
		ucPhyTypeSet = prStaRec->ucPhyTypeSet;
	} else {
		/* Get PHY type set from current BSS */
		ucPhyTypeSet = prBssInfo->ucPhyTypeSet;
	}

	if (RLM_NET_IS_11N(prBssInfo) &&
	    (ucPhyTypeSet & PHY_TYPE_SET_802_11N) &&
		(!prBssInfo->fgIsWepCipherGroup))
		rlmFillHtOpIE(prAdapter, prBssInfo, prMsduInfo);
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief For probe response (GO, IBSS) and association response
 *
 * \param[in]
 *
 * \return none
 */
/*----------------------------------------------------------------------------*/
void rlmRspGenerateErpIE(struct ADAPTER *prAdapter,
			 struct MSDU_INFO *prMsduInfo)
{
	struct BSS_INFO *prBssInfo;
	struct STA_RECORD *prStaRec;
	struct IE_ERP *prErpIe;
	uint8_t ucPhyTypeSet;

	ASSERT(prAdapter);
	ASSERT(prMsduInfo);

	prStaRec = cnmGetStaRecByIndex(prAdapter, prMsduInfo->ucStaRecIndex);

	prBssInfo = prAdapter->aprBssInfo[prMsduInfo->ucBssIndex];
	if (!prBssInfo)
		return;

	if (!IS_BSS_ACTIVE(prBssInfo))
		return;

	/* Decide PHY type set source */
	if (prStaRec) {
		/* Get PHY type set from target STA */
		ucPhyTypeSet = prStaRec->ucPhyTypeSet;
	} else {
		/* Get PHY type set from current BSS */
		ucPhyTypeSet = prBssInfo->ucPhyTypeSet;
	}

	if (RLM_NET_IS_11GN(prBssInfo) && prBssInfo->eBand == BAND_2G4 &&
	    (ucPhyTypeSet & PHY_TYPE_SET_802_11GN)) {
		prErpIe = (struct IE_ERP *)(((uint8_t *)prMsduInfo->prPacket) +
					    prMsduInfo->u2FrameLength);

		/* Add ERP IE */
		prErpIe->ucId = ELEM_ID_ERP_INFO;
		prErpIe->ucLength = 1;

		prErpIe->ucERP = prBssInfo->fgObssErpProtectMode
					 ? ERP_INFO_USE_PROTECTION
					 : 0;

		if (prBssInfo->fgErpProtectMode)
			prErpIe->ucERP |= (ERP_INFO_NON_ERP_PRESENT |
					   ERP_INFO_USE_PROTECTION);

		/* Handle barker preamble */
		if (!prBssInfo->fgUseShortPreamble)
			prErpIe->ucERP |= ERP_INFO_BARKER_PREAMBLE_MODE;

		ASSERT(IE_SIZE(prErpIe) <= (ELEM_HDR_LEN + ELEM_MAX_LEN_ERP));

		prMsduInfo->u2FrameLength += IE_SIZE(prErpIe);
	}
}

#if CFG_SUPPORT_MTK_SYNERGY
/*----------------------------------------------------------------------------*/
/*!
 * \brief This function is used to generate MTK Vendor Specific OUI
 *
 * \param[in]
 *
 * \return none
 */
/*----------------------------------------------------------------------------*/
void rlmGenerateMTKOuiIE(struct ADAPTER *prAdapter,
			 struct MSDU_INFO *prMsduInfo)
{
	struct BSS_INFO *prBssInfo;
	uint8_t *pucBuffer;
	uint8_t aucMtkOui[] = VENDOR_OUI_MTK;

	ASSERT(prAdapter);
	ASSERT(prMsduInfo);

	if (prAdapter->rWifiVar.ucMtkOui == FEATURE_DISABLED)
		return;

	prBssInfo = prAdapter->aprBssInfo[prMsduInfo->ucBssIndex];
	if (!prBssInfo)
		return;

	pucBuffer = (uint8_t *)((unsigned long)prMsduInfo->prPacket +
				(unsigned long)prMsduInfo->u2FrameLength);

	MTK_OUI_IE(pucBuffer)->ucId = ELEM_ID_VENDOR;
	MTK_OUI_IE(pucBuffer)->ucLength = ELEM_MIN_LEN_MTK_OUI;
	MTK_OUI_IE(pucBuffer)->aucOui[0] = aucMtkOui[0];
	MTK_OUI_IE(pucBuffer)->aucOui[1] = aucMtkOui[1];
	MTK_OUI_IE(pucBuffer)->aucOui[2] = aucMtkOui[2];
	MTK_OUI_IE(pucBuffer)->aucCapability[0] =
		MTK_SYNERGY_CAP0 & (prAdapter->rWifiVar.aucMtkFeature[0]);
	MTK_OUI_IE(pucBuffer)->aucCapability[1] =
		MTK_SYNERGY_CAP1 & (prAdapter->rWifiVar.aucMtkFeature[1]);
	MTK_OUI_IE(pucBuffer)->aucCapability[2] =
		MTK_SYNERGY_CAP2 & (prAdapter->rWifiVar.aucMtkFeature[2]);
	MTK_OUI_IE(pucBuffer)->aucCapability[3] =
		MTK_SYNERGY_CAP3 & (prAdapter->rWifiVar.aucMtkFeature[3]);

	/* Disable the 2.4G 256QAM feature bit if chip doesn't support AC*/
	if (prAdapter->rWifiVar.ucHwNotSupportAC) {
		MTK_OUI_IE(pucBuffer)->aucCapability[0] &=
			~(MTK_SYNERGY_CAP_SUPPORT_24G_MCS89);
		DBGLOG(INIT, WARN,
		       "Disable 2.4G 256QAM support if N only chip\n");
	}

#if (CFG_SUPPORT_TWT_HOTSPOT_AC == 1)
	if (IS_FEATURE_ENABLED(
		prAdapter->rWifiVar.ucTWTHotSpotSupport)) {
		MTK_OUI_IE(pucBuffer)->aucCapability[0] |=
			MTK_SYNERGY_CAP_SUPPORT_TWT_HOTSPOT_AC;

		DBGLOG(INIT, WARN, "TWT hotspot supported\n");
	}
#endif

	prMsduInfo->u2FrameLength += IE_SIZE(pucBuffer);
	pucBuffer += IE_SIZE(pucBuffer);
} /* rlmGenerateMTKOuiIE */

/*----------------------------------------------------------------------------*/
/*!
 * @brief This function is used to check MTK Vendor Specific OUI
 *
 *
 * @return true:  correct MTK OUI
 *             false: incorrect MTK OUI
 */
/*----------------------------------------------------------------------------*/
u_int8_t rlmParseCheckMTKOuiIE(IN struct ADAPTER *prAdapter, IN uint8_t *pucBuf,
			       IN uint32_t *pu4Cap)
{
	uint8_t aucMtkOui[] = VENDOR_OUI_MTK;
	struct IE_MTK_OUI *prMtkOuiIE = (struct IE_MTK_OUI *)NULL;

	do {
		ASSERT_BREAK((prAdapter != NULL) && (pucBuf != NULL));

		prMtkOuiIE = (struct IE_MTK_OUI *)pucBuf;

		if (prAdapter->rWifiVar.ucMtkOui == FEATURE_DISABLED)
			break;
		else if (IE_LEN(pucBuf) < ELEM_MIN_LEN_MTK_OUI)
			break;
		else if (prMtkOuiIE->aucOui[0] != aucMtkOui[0] ||
			 prMtkOuiIE->aucOui[1] != aucMtkOui[1] ||
			 prMtkOuiIE->aucOui[2] != aucMtkOui[2])
			break;
		/* apply NvRam setting */
		prMtkOuiIE->aucCapability[0] =
			prMtkOuiIE->aucCapability[0] &
			(prAdapter->rWifiVar.aucMtkFeature[0]);
		prMtkOuiIE->aucCapability[1] =
			prMtkOuiIE->aucCapability[1] &
			(prAdapter->rWifiVar.aucMtkFeature[1]);
		prMtkOuiIE->aucCapability[2] =
			prMtkOuiIE->aucCapability[2] &
			(prAdapter->rWifiVar.aucMtkFeature[2]);
		prMtkOuiIE->aucCapability[3] =
			prMtkOuiIE->aucCapability[3] &
			(prAdapter->rWifiVar.aucMtkFeature[3]);

		/* Disable the 2.4G 256QAM feature bit if chip doesn't support
		 * AC. Otherwise, FW would choose wrong max rate of auto rate.
		 */
		if (prAdapter->rWifiVar.ucHwNotSupportAC) {
			prMtkOuiIE->aucCapability[0] &=
				~(MTK_SYNERGY_CAP_SUPPORT_24G_MCS89);
			DBGLOG(INIT, WARN,
			       "Disable 2.4G 256QAM support if N only chip\n");
		}

		kalMemCopy(pu4Cap, prMtkOuiIE->aucCapability, sizeof(uint32_t));

		return TRUE;
	} while (FALSE);

	return FALSE;
} /* rlmParseCheckMTKOuiIE */

#endif

/*----------------------------------------------------------------------------*/
/*!
 * \brief
 *
 * \param[in]
 *
 * \return none
 */
/*----------------------------------------------------------------------------*/
void rlmGenerateCsaIE(struct ADAPTER *prAdapter, struct MSDU_INFO *prMsduInfo)
{
	uint8_t *pucBuffer;

	ASSERT(prAdapter);
	ASSERT(prMsduInfo);

	if (prAdapter->rWifiVar.fgCsaInProgress) {

		pucBuffer =
			(uint8_t *)((unsigned long)prMsduInfo->prPacket +
				    (unsigned long)prMsduInfo->u2FrameLength);

		CSA_IE(pucBuffer)->ucId = ELEM_ID_CH_SW_ANNOUNCEMENT;
		CSA_IE(pucBuffer)->ucLength = 3;
		CSA_IE(pucBuffer)->ucChannelSwitchMode =
			prAdapter->rWifiVar.ucChannelSwitchMode;
		CSA_IE(pucBuffer)->ucNewChannelNum =
			prAdapter->rWifiVar.ucNewChannelNumber;
		CSA_IE(pucBuffer)->ucChannelSwitchCount =
			prAdapter->rWifiVar.ucChannelSwitchCount;

		prMsduInfo->u2FrameLength += IE_SIZE(pucBuffer);
		/* Fill Secondary channel offset IE */
		pucBuffer += IE_SIZE(pucBuffer);

		SEC_OFFSET_IE(pucBuffer)->ucId = ELEM_ID_SCO;
		SEC_OFFSET_IE(pucBuffer)->ucLength = 1;
		SEC_OFFSET_IE(pucBuffer)->ucSecondaryOffset =
			prAdapter->rWifiVar.ucSecondaryOffset;

		prMsduInfo->u2FrameLength += IE_SIZE(pucBuffer);

		/* Fill Wide Bandwidth Channel Switch IE */
		pucBuffer += IE_SIZE(pucBuffer);

		WIDE_BW_IE(pucBuffer)->ucId = ELEM_ID_WIDE_BAND_CHANNEL_SWITCH;
		WIDE_BW_IE(pucBuffer)->ucLength = 3;
		WIDE_BW_IE(pucBuffer)->ucNewChannelWidth =
			prAdapter->rWifiVar.ucNewChannelWidth;
		WIDE_BW_IE(pucBuffer)->ucChannelS1 =
			prAdapter->rWifiVar.ucNewChannelS1;
		WIDE_BW_IE(pucBuffer)->ucChannelS2 =
			prAdapter->rWifiVar.ucNewChannelS2;

		prMsduInfo->u2FrameLength += IE_SIZE(pucBuffer);
	}
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief
 *
 * \param[in]
 *
 * \return none
 */
/*----------------------------------------------------------------------------*/
static void rlmFillHtCapIE(struct ADAPTER *prAdapter,
			   struct BSS_INFO *prBssInfo,
			   struct MSDU_INFO *prMsduInfo)
{
	struct IE_HT_CAP *prHtCap;
	struct SUP_MCS_SET_FIELD *prSupMcsSet;
	u_int8_t fg40mAllowed;
	uint8_t ucIdx;

	ASSERT(prAdapter);
	ASSERT(prBssInfo);
	ASSERT(prMsduInfo);

	fg40mAllowed = prBssInfo->fgAssoc40mBwAllowed;

	prHtCap = (struct IE_HT_CAP *)(((uint8_t *)prMsduInfo->prPacket) +
				       prMsduInfo->u2FrameLength);

	/* Add HT capabilities IE */
	prHtCap->ucId = ELEM_ID_HT_CAP;
	prHtCap->ucLength = sizeof(struct IE_HT_CAP) - ELEM_HDR_LEN;

	prHtCap->u2HtCapInfo = HT_CAP_INFO_DEFAULT_VAL;

	if (IS_FEATURE_ENABLED(prAdapter->rWifiVar.ucRxShortGI))
		prHtCap->u2HtCapInfo |=
			(HT_CAP_INFO_SHORT_GI_20M | HT_CAP_INFO_SHORT_GI_40M);

	if (IS_FEATURE_ENABLED(prAdapter->rWifiVar.ucRxLdpc))
		prHtCap->u2HtCapInfo |= HT_CAP_INFO_LDPC_CAP;

	if (IS_FEATURE_ENABLED(prAdapter->rWifiVar.ucTxStbc) &&
			prBssInfo->ucOpTxNss >= 2)
		prHtCap->u2HtCapInfo |= HT_CAP_INFO_TX_STBC;

	if (IS_FEATURE_ENABLED(prAdapter->rWifiVar.ucRxStbc)) {

		uint8_t tempRxStbcNss;

		tempRxStbcNss = prAdapter->rWifiVar.ucRxStbcNss;
		tempRxStbcNss =
			(tempRxStbcNss >
			 wlanGetSupportNss(prAdapter, prBssInfo->ucBssIndex))
				? wlanGetSupportNss(prAdapter,
						    prBssInfo->ucBssIndex)
				: (tempRxStbcNss);
		if (tempRxStbcNss != prAdapter->rWifiVar.ucRxStbcNss) {
			DBGLOG(RLM, WARN, "Apply Nss:%d as RxStbcNss in HT Cap",
			       wlanGetSupportNss(prAdapter,
						 prBssInfo->ucBssIndex));
			DBGLOG(RLM, WARN,
			       " due to set RxStbcNss more than Nss is not appropriate.\n");
		}
		if (tempRxStbcNss == 1)
			prHtCap->u2HtCapInfo |= HT_CAP_INFO_RX_STBC_1_SS;
		else if (tempRxStbcNss == 2)
			prHtCap->u2HtCapInfo |= HT_CAP_INFO_RX_STBC_2_SS;
		else if (tempRxStbcNss >= 3)
			prHtCap->u2HtCapInfo |= HT_CAP_INFO_RX_STBC_3_SS;
	}

	if (IS_FEATURE_ENABLED(prAdapter->rWifiVar.ucRxGf))
		prHtCap->u2HtCapInfo |= HT_CAP_INFO_HT_GF;

	if (prAdapter->rWifiVar.ucRxMaxMpduLen > VHT_CAP_INFO_MAX_MPDU_LEN_3K)
		prHtCap->u2HtCapInfo |= HT_CAP_INFO_MAX_AMSDU_LEN;

	if (!fg40mAllowed)
		prHtCap->u2HtCapInfo &= ~(HT_CAP_INFO_SUP_CHNL_WIDTH |
					  HT_CAP_INFO_SHORT_GI_40M |
					  HT_CAP_INFO_DSSS_CCK_IN_40M);

	/* SM power saving (IEEE 802.11, 2016, 10.2.4)
	 * A non-AP HT STA may also use SM Power Save bits in the HT
	 * Capabilities element of its Association Request to achieve
	 * the same purpose. The latter allows the STA to use only a
	 * single receive chain immediately after association.
	 */
	if (prBssInfo->ucOpRxNss <
		wlanGetSupportNss(prAdapter, prBssInfo->ucBssIndex))
		prHtCap->u2HtCapInfo &=
			~HT_CAP_INFO_SM_POWER_SAVE; /*Set as static power save*/

	prHtCap->ucAmpduParam = AMPDU_PARAM_DEFAULT_VAL;

	prSupMcsSet = &prHtCap->rSupMcsSet;
	kalMemZero((void *)&prSupMcsSet->aucRxMcsBitmask[0],
		   SUP_MCS_RX_BITMASK_OCTET_NUM);

	for (ucIdx = 0;
	     ucIdx < wlanGetSupportNss(prAdapter, prBssInfo->ucBssIndex);
	     ucIdx++)
		prSupMcsSet->aucRxMcsBitmask[ucIdx] = BITS(0, 7);

	/* prSupMcsSet->aucRxMcsBitmask[0] = BITS(0, 7); */

	if (fg40mAllowed && IS_FEATURE_ENABLED(prAdapter->rWifiVar.ucMCS32))
		prSupMcsSet->aucRxMcsBitmask[32 / 8] = BIT(0); /* MCS32 */
	prSupMcsSet->u2RxHighestSupportedRate = SUP_MCS_RX_DEFAULT_HIGHEST_RATE;
	prSupMcsSet->u4TxRateInfo = SUP_MCS_TX_DEFAULT_VAL;

	prHtCap->u2HtExtendedCap = HT_EXT_CAP_DEFAULT_VAL;
	if (!fg40mAllowed ||
	    prBssInfo->eCurrentOPMode != OP_MODE_INFRASTRUCTURE)
		prHtCap->u2HtExtendedCap &=
			~(HT_EXT_CAP_PCO | HT_EXT_CAP_PCO_TRANS_TIME_NONE);

	prHtCap->u4TxBeamformingCap = TX_BEAMFORMING_CAP_DEFAULT_VAL;
	if ((prAdapter->rWifiVar.eDbdcMode == ENUM_DBDC_MODE_DISABLED) ||
	    (prBssInfo->eBand == BAND_5G)) {
		/* HT BFee has IOT issue
		 * only support HT BFee when force mode for testing
		 */
		if (IS_FEATURE_FORCE_ENABLED(prAdapter->rWifiVar.ucStaHtBfee))
			prHtCap->u4TxBeamformingCap = TX_BEAMFORMING_CAP_BFEE;
		if (IS_FEATURE_ENABLED(prAdapter->rWifiVar.ucStaHtBfer))
			prHtCap->u4TxBeamformingCap |= TX_BEAMFORMING_CAP_BFER;
	}

	prHtCap->ucAselCap = ASEL_CAP_DEFAULT_VAL;

	ASSERT(IE_SIZE(prHtCap) <= (ELEM_HDR_LEN + ELEM_MAX_LEN_HT_CAP));

	prMsduInfo->u2FrameLength += IE_SIZE(prHtCap);
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief
 *
 * \param[in]
 *
 * \return none
 */
/*----------------------------------------------------------------------------*/
static void rlmFillExtCapIE(struct ADAPTER *prAdapter,
			    struct BSS_INFO *prBssInfo,
			    struct MSDU_INFO *prMsduInfo)
{
#if CFG_SUPPORT_PASSPOINT
	struct IE_HS20_EXT_CAP_T *prHsExtCap;
	struct HS20_INFO *prHS20Info;
#else
	struct IE_EXT_CAP *prExtCap;
#endif
	u_int8_t fg40mAllowed, fgAppendVhtCap;
	struct STA_RECORD *prStaRec;
	struct WIFI_VAR *prWifiVar;

	ASSERT(prAdapter);
	ASSERT(prMsduInfo);

	prWifiVar = &prAdapter->rWifiVar;

	fg40mAllowed = prBssInfo->fgAssoc40mBwAllowed;

#if CFG_SUPPORT_PASSPOINT
	prHS20Info = aisGetHS20Info(prAdapter,
			prBssInfo->ucBssIndex);
	if (!prHS20Info)
		return;

	prHsExtCap =
		(struct IE_HS20_EXT_CAP_T *)(((uint8_t *)prMsduInfo->prPacket) +
					     prMsduInfo->u2FrameLength);
	prHsExtCap->ucId = ELEM_ID_EXTENDED_CAP;

	if (prHS20Info->fgConnectHS20AP == TRUE)
		prHsExtCap->ucLength = ELEM_MAX_LEN_EXT_CAP;
	else
		prHsExtCap->ucLength = 3 - ELEM_HDR_LEN;

	kalMemZero(prHsExtCap->aucCapabilities,
		   sizeof(prHsExtCap->aucCapabilities));

	prHsExtCap->aucCapabilities[0] = ELEM_EXT_CAP_DEFAULT_VAL;

	if (!fg40mAllowed)
		prHsExtCap->aucCapabilities[0] &=
			~ELEM_EXT_CAP_20_40_COEXIST_SUPPORT;

	if (prBssInfo->eCurrentOPMode != OP_MODE_INFRASTRUCTURE)
		prHsExtCap->aucCapabilities[0] &= ~ELEM_EXT_CAP_PSMP_CAP;

#if CFG_SUPPORT_802_11AC
	prStaRec = cnmGetStaRecByIndex(prAdapter, prMsduInfo->ucStaRecIndex);
	fgAppendVhtCap = FALSE;

	/* Check append rule */
	if (prAdapter->rWifiVar.ucAvailablePhyTypeSet
		& PHY_TYPE_SET_802_11AC) {
		/* Note: For AIS connecting state,
		 * structure in BSS_INFO will not be inited
		 *	 So, we check StaRec instead of BssInfo
		 */
		if (prStaRec) {
			if (prStaRec->ucPhyTypeSet & PHY_TYPE_SET_802_11AC)
				fgAppendVhtCap = TRUE;
		} else if (RLM_NET_IS_11AC(prBssInfo) &&
				((prBssInfo->eCurrentOPMode ==
				OP_MODE_INFRASTRUCTURE) ||
				(prBssInfo->eCurrentOPMode ==
				OP_MODE_ACCESS_POINT))) {
			fgAppendVhtCap = TRUE;
		}

	}

	if (fgAppendVhtCap) {
		if (prHsExtCap->ucLength < ELEM_MAX_LEN_EXT_CAP)
			prHsExtCap->ucLength = ELEM_MAX_LEN_EXT_CAP;

		SET_EXT_CAP(prHsExtCap->aucCapabilities, ELEM_MAX_LEN_EXT_CAP,
			    ELEM_EXT_CAP_OP_MODE_NOTIFICATION_BIT);
	}
#endif

	if (prHS20Info->fgConnectHS20AP == TRUE) {
		SET_EXT_CAP(prHsExtCap->aucCapabilities, ELEM_MAX_LEN_EXT_CAP,
			    ELEM_EXT_CAP_INTERWORKING_BIT);
		SET_EXT_CAP(prHsExtCap->aucCapabilities, ELEM_MAX_LEN_EXT_CAP,
			    ELEM_EXT_CAP_QOSMAPSET_BIT);
		SET_EXT_CAP(prHsExtCap->aucCapabilities, ELEM_MAX_LEN_EXT_CAP,
			    ELEM_EXT_CAP_BSS_TRANSITION_BIT);
		/* For R2 WNM-Notification */
		SET_EXT_CAP(prHsExtCap->aucCapabilities, ELEM_MAX_LEN_EXT_CAP,
			    ELEM_EXT_CAP_WNM_NOTIFICATION_BIT);
	}

#if CFG_SUPPORT_802_11V_BSS_TRANSITION_MGT
	prHsExtCap->ucLength = ELEM_MAX_LEN_EXT_CAP;
	SET_EXT_CAP(prHsExtCap->aucCapabilities, ELEM_MAX_LEN_EXT_CAP,
		    ELEM_EXT_CAP_BSS_TRANSITION_BIT);
#endif

#if (CFG_SUPPORT_802_11V_MBSSID == 1)
	prExtCap->ucLength = ELEM_MAX_LEN_EXT_CAP;
	SET_EXT_CAP(prHsExtCap->aucCapabilities, ELEM_MAX_LEN_EXT_CAP,
		    ELEM_EXT_CAP_MBSSID_BIT);
#endif

	ASSERT(IE_SIZE(prHsExtCap) <= (ELEM_HDR_LEN + ELEM_MAX_LEN_EXT_CAP));

	prMsduInfo->u2FrameLength += IE_SIZE(prHsExtCap);

#else
	/* Add Extended Capabilities IE */
	prExtCap = (struct IE_EXT_CAP *)(((uint8_t *)prMsduInfo->prPacket) +
					 prMsduInfo->u2FrameLength);

	prExtCap->ucId = ELEM_ID_EXTENDED_CAP;

	prExtCap->ucLength = 3 - ELEM_HDR_LEN;
	kalMemZero(prExtCap->aucCapabilities,
		   sizeof(prExtCap->aucCapabilities));

	prExtCap->aucCapabilities[0] = ELEM_EXT_CAP_DEFAULT_VAL;

	if (!fg40mAllowed)
		prExtCap->aucCapabilities[0] &=
			~ELEM_EXT_CAP_20_40_COEXIST_SUPPORT;

	if (prBssInfo->eCurrentOPMode != OP_MODE_INFRASTRUCTURE)
		prExtCap->aucCapabilities[0] &= ~ELEM_EXT_CAP_PSMP_CAP;

#if CFG_SUPPORT_802_11AC
	prStaRec = cnmGetStaRecByIndex(prAdapter, prMsduInfo->ucStaRecIndex);
	fgAppendVhtCap = FALSE;

	/* Check append rule */
	if (prAdapter->rWifiVar.ucAvailablePhyTypeSet
		& PHY_TYPE_SET_802_11AC) {
		/* Note: For AIS connecting state,
		 * structure in BSS_INFO will not be inited
		 *       So, we check StaRec instead of BssInfo
		 */
		if (prStaRec) {
			if (prStaRec->ucPhyTypeSet & PHY_TYPE_SET_802_11AC)
				fgAppendVhtCap = TRUE;
		} else if (RLM_NET_IS_11AC(prBssInfo) &&
				((prBssInfo->eCurrentOPMode ==
				OP_MODE_INFRASTRUCTURE) ||
				(prBssInfo->eCurrentOPMode ==
				OP_MODE_ACCESS_POINT))) {
			fgAppendVhtCap = TRUE;
		}
	}

	if (fgAppendVhtCap) {
		if (prExtCap->ucLength < ELEM_MAX_LEN_EXT_CAP)
			prExtCap->ucLength = ELEM_MAX_LEN_EXT_CAP;

		SET_EXT_CAP(prExtCap->aucCapabilities, ELEM_MAX_LEN_EXT_CAP,
			    ELEM_EXT_CAP_OP_MODE_NOTIFICATION_BIT);
	}
#endif

#if (CFG_SUPPORT_TWT == 1)
	if (IS_FEATURE_ENABLED(prWifiVar->ucTWTRequester))
		prExtCap->
			aucCapabilities[ELEM_EXT_CAP_TWT_REQUESTER_BIT >> 3] |=
			BIT(ELEM_EXT_CAP_TWT_REQUESTER_BIT % 8);
#endif

#if CFG_SUPPORT_802_11V_BSS_TRANSITION_MGT
	prExtCap->ucLength = ELEM_MAX_LEN_EXT_CAP;
	SET_EXT_CAP(prExtCap->aucCapabilities, ELEM_MAX_LEN_EXT_CAP,
				ELEM_EXT_CAP_BSS_TRANSITION_BIT);
#endif

#if (CFG_SUPPORT_802_11V_MBSSID == 1)
	prExtCap->ucLength = ELEM_MAX_LEN_EXT_CAP;
	SET_EXT_CAP(prExtCap->aucCapabilities, ELEM_MAX_LEN_EXT_CAP,
				ELEM_EXT_CAP_MBSSID_BIT);
#endif

	ASSERT(IE_SIZE(prExtCap) <= (ELEM_HDR_LEN + ELEM_MAX_LEN_EXT_CAP));

	prMsduInfo->u2FrameLength += IE_SIZE(prExtCap);
#endif
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief
 *
 * \param[in]
 *
 * \return none
 */
/*----------------------------------------------------------------------------*/
static void rlmFillHtOpIE(struct ADAPTER *prAdapter, struct BSS_INFO *prBssInfo,
			  struct MSDU_INFO *prMsduInfo)
{
	struct IE_HT_OP *prHtOp;
	uint16_t i;

	ASSERT(prAdapter);
	ASSERT(prBssInfo);
	ASSERT(prMsduInfo);

	prHtOp = (struct IE_HT_OP *)(((uint8_t *)prMsduInfo->prPacket) +
				     prMsduInfo->u2FrameLength);

	/* Add HT operation IE */
	prHtOp->ucId = ELEM_ID_HT_OP;
	prHtOp->ucLength = sizeof(struct IE_HT_OP) - ELEM_HDR_LEN;

	/* RIFS and 20/40 bandwidth operations are included */
	prHtOp->ucPrimaryChannel = prBssInfo->ucPrimaryChannel;
	prHtOp->ucInfo1 = prBssInfo->ucHtOpInfo1;

	/* Decide HT protection mode field */
	if (prBssInfo->eHtProtectMode == HT_PROTECT_MODE_NON_HT)
		prHtOp->u2Info2 = (uint8_t)HT_PROTECT_MODE_NON_HT;
	else if (prBssInfo->eObssHtProtectMode == HT_PROTECT_MODE_NON_MEMBER)
		prHtOp->u2Info2 = (uint8_t)HT_PROTECT_MODE_NON_MEMBER;
	else {
		/* It may be SYS_PROTECT_MODE_NONE or SYS_PROTECT_MODE_20M */
		prHtOp->u2Info2 = (uint8_t)prBssInfo->eHtProtectMode;
	}

	if (prBssInfo->eGfOperationMode != GF_MODE_NORMAL) {
		/* It may be GF_MODE_PROTECT or GF_MODE_DISALLOWED
		 * Note: it will also be set in ad-hoc network
		 */
		prHtOp->u2Info2 |= HT_OP_INFO2_NON_GF_HT_STA_PRESENT;
	}

	if (0 /* Regulatory class 16 */ &&
	    prBssInfo->eObssHtProtectMode == HT_PROTECT_MODE_NON_MEMBER) {
		/* (TBD) It is HT_PROTECT_MODE_NON_MEMBER, so require protection
		 * although it is possible to have no protection by spec.
		 */
		prHtOp->u2Info2 |= HT_OP_INFO2_OBSS_NON_HT_STA_PRESENT;
	}

	prHtOp->u2Info3 = prBssInfo->u2HtOpInfo3; /* To do: handle L-SIG TXOP */

	/* No basic MCSx are needed temporarily */
	for (i = 0; i < 16; i++)
		prHtOp->aucBasicMcsSet[i] = 0;

	ASSERT(IE_SIZE(prHtOp) <= (ELEM_HDR_LEN + ELEM_MAX_LEN_HT_OP));

	prMsduInfo->u2FrameLength += IE_SIZE(prHtOp);
}

#if CFG_SUPPORT_802_11AC

/*----------------------------------------------------------------------------*/
/*!
 * \brief For probe request, association request
 *
 * \param[in]
 *
 * \return none
 */
/*----------------------------------------------------------------------------*/
void rlmReqGenerateVhtCapIE(struct ADAPTER *prAdapter,
			    struct MSDU_INFO *prMsduInfo)
{
	struct BSS_INFO *prBssInfo;
	struct STA_RECORD *prStaRec;
	struct BSS_DESC *prBssDesc = NULL;

	ASSERT(prAdapter);
	ASSERT(prMsduInfo);

	prBssInfo = prAdapter->aprBssInfo[prMsduInfo->ucBssIndex];
	prStaRec = cnmGetStaRecByIndex(prAdapter, prMsduInfo->ucStaRecIndex);

	if (!prBssInfo || !prStaRec) {
		DBGLOG(RLM, ERROR, "prBssInfo=%p, prStaRec=%p, return!!\n",
			prBssInfo, prStaRec);
		return;
	}

	if (IS_BSS_AIS(prBssInfo))
		prBssDesc =
			aisGetTargetBssDesc(prAdapter, prMsduInfo->ucBssIndex);
#if CFG_ENABLE_WIFI_DIRECT
	else if (IS_BSS_P2P(prBssInfo))
		prBssDesc =
			p2pGetTargetBssDesc(prAdapter, prMsduInfo->ucBssIndex);
#endif

	if (!prBssDesc) {
		DBGLOG(RLM, ERROR, "prBssDesc is NULL, return!\n");
		return;
	}

	DBGLOG(RLM, TRACE,
		"eBand=%d,PhyType=0x%02x, AvailPhyType=0x%02x, VHTPresent=%d\n",
		prBssDesc->eBand,
		prStaRec->ucPhyTypeSet,
		prAdapter->rWifiVar.ucAvailablePhyTypeSet,
		prBssDesc->fgIsVHTPresent);

	if ((prBssDesc->eBand == BAND_5G) &&
		(prAdapter->rWifiVar.ucAvailablePhyTypeSet &
		PHY_TYPE_SET_802_11AC)
		&& (prStaRec->ucPhyTypeSet & PHY_TYPE_SET_802_11AC))
		rlmFillVhtCapIE(prAdapter, prBssInfo, prMsduInfo);
#if CFG_SUPPORT_VHT_IE_IN_2G
	else if ((prBssDesc->eBand == BAND_2G4) &&
			(prStaRec->ucPhyTypeSet & PHY_TYPE_SET_802_11N) &&
			((prAdapter->rWifiVar.ucVhtIeIn2g
				==  FEATURE_FORCE_ENABLED) ||
			((prAdapter->rWifiVar.ucVhtIeIn2g == FEATURE_ENABLED) &&
				(prBssDesc->fgIsVHTPresent)))) {
		DBGLOG(RLM, TRACE, "Add VHT IE in 2.4G\n");
		rlmFillVhtCapIE(prAdapter, prBssInfo, prMsduInfo);
	}
#endif
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief For probe response (GO, IBSS) and association response
 *
 * \param[in]
 *
 * \return none
 */
/*----------------------------------------------------------------------------*/
void rlmRspGenerateVhtCapIE(struct ADAPTER *prAdapter,
			    struct MSDU_INFO *prMsduInfo)
{
	struct BSS_INFO *prBssInfo;
	struct STA_RECORD *prStaRec;
	uint8_t ucPhyTypeSet;

	ASSERT(prAdapter);
	ASSERT(prMsduInfo);

	prBssInfo = prAdapter->aprBssInfo[prMsduInfo->ucBssIndex];
	if (!prBssInfo)
		return;

	if (!IS_BSS_ACTIVE(prBssInfo))
		return;

	prStaRec = cnmGetStaRecByIndex(prAdapter, prMsduInfo->ucStaRecIndex);

	/* Decide PHY type set source */
	if (prStaRec) {
		/* Get PHY type set from target STA */
		ucPhyTypeSet = prStaRec->ucPhyTypeSet;
	} else {
		/* Get PHY type set from current BSS */
		ucPhyTypeSet = prBssInfo->ucPhyTypeSet;
	}

	if (RLM_NET_IS_11AC(prBssInfo) &&
	    (ucPhyTypeSet & PHY_TYPE_SET_802_11AC))
		rlmFillVhtCapIE(prAdapter, prBssInfo, prMsduInfo);
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief
 *
 * \param[in]
 *
 * \return none
 */
/*----------------------------------------------------------------------------*/
void rlmRspGenerateVhtOpIE(struct ADAPTER *prAdapter,
			   struct MSDU_INFO *prMsduInfo)
{
	struct BSS_INFO *prBssInfo;
	struct STA_RECORD *prStaRec;
	uint8_t ucPhyTypeSet;

	ASSERT(prAdapter);
	ASSERT(prMsduInfo);

	prStaRec = cnmGetStaRecByIndex(prAdapter, prMsduInfo->ucStaRecIndex);

	prBssInfo = prAdapter->aprBssInfo[prMsduInfo->ucBssIndex];
	if (!prBssInfo)
		return;

	if (!IS_BSS_ACTIVE(prBssInfo))
		return;

	/* Decide PHY type set source */
	if (prStaRec) {
		/* Get PHY type set from target STA */
		ucPhyTypeSet = prStaRec->ucPhyTypeSet;
	} else {
		/* Get PHY type set from current BSS */
		ucPhyTypeSet = prBssInfo->ucPhyTypeSet;
	}

	if (RLM_NET_IS_11AC(prBssInfo) &&
	    (ucPhyTypeSet & PHY_TYPE_SET_802_11AC))
		rlmFillVhtOpIE(prAdapter, prBssInfo, prMsduInfo);
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief For probe request, association request
 *
 * \param[in]
 *
 * \return none
 */
/*----------------------------------------------------------------------------*/
void rlmReqGenerateVhtOpNotificationIE(struct ADAPTER *prAdapter,
				       struct MSDU_INFO *prMsduInfo)
{
	struct BSS_INFO *prBssInfo;
	struct STA_RECORD *prStaRec;

	ASSERT(prAdapter);
	ASSERT(prMsduInfo);

	prBssInfo = prAdapter->aprBssInfo[prMsduInfo->ucBssIndex];
	if (!prBssInfo)
		return;

	prStaRec = cnmGetStaRecByIndex(prAdapter, prMsduInfo->ucStaRecIndex);

	if ((prAdapter->rWifiVar.ucAvailablePhyTypeSet &
	     PHY_TYPE_SET_802_11AC) &&
	    (!prStaRec || (prStaRec->ucPhyTypeSet & PHY_TYPE_SET_802_11AC))) {
		/* Fill own capability in channel width field in OP mode element
		 * since we haven't filled in channel width info in BssInfo at
		 * current state
		 */
		rlmFillVhtOpNotificationIE(prAdapter, prBssInfo, prMsduInfo,
					   FALSE);
	}
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief
 *
 * \param[in]
 *
 * \return none
 */
/*----------------------------------------------------------------------------*/
void rlmRspGenerateVhtOpNotificationIE(struct ADAPTER *prAdapter,
				       struct MSDU_INFO *prMsduInfo)
{
	struct BSS_INFO *prBssInfo;
	struct STA_RECORD *prStaRec;
	uint8_t ucPhyTypeSet;

	ASSERT(prAdapter);
	ASSERT(prMsduInfo);

	prStaRec = cnmGetStaRecByIndex(prAdapter, prMsduInfo->ucStaRecIndex);

	prBssInfo = prAdapter->aprBssInfo[prMsduInfo->ucBssIndex];
	if (!prBssInfo)
		return;

	if (!IS_BSS_ACTIVE(prBssInfo))
		return;

	if (!prBssInfo->fgIsOpChangeRxNss)
		return;

	/* Decide PHY type set source */
	if (prStaRec) {
		/* Get PHY type set from target STA */
		ucPhyTypeSet = prStaRec->ucPhyTypeSet;
	} else {
		/* Get PHY type set from current BSS */
		ucPhyTypeSet = prBssInfo->ucPhyTypeSet;
	}

	if (RLM_NET_IS_11AC(prBssInfo) &&
	    (ucPhyTypeSet & PHY_TYPE_SET_802_11AC))
		rlmFillVhtOpNotificationIE(prAdapter, prBssInfo, prMsduInfo,
					   FALSE);
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief
 * add VHT operation notification IE for VHT-BW40 case specific
 * \param[in]
 *
 * \return none
 */
/*----------------------------------------------------------------------------*/
static void rlmFillVhtOpNotificationIE(struct ADAPTER *prAdapter,
				       struct BSS_INFO *prBssInfo,
				       struct MSDU_INFO *prMsduInfo,
				       u_int8_t fgIsOwnCap)
{
	struct IE_VHT_OP_MODE_NOTIFICATION *prVhtOpMode;
	uint8_t ucOpModeBw = VHT_OP_MODE_CHANNEL_WIDTH_20;

	ASSERT(prAdapter);
	ASSERT(prBssInfo);
	ASSERT(prMsduInfo);

	prVhtOpMode = (struct IE_VHT_OP_MODE_NOTIFICATION
			       *)(((uint8_t *)prMsduInfo->prPacket) +
				  prMsduInfo->u2FrameLength);

	kalMemZero((void *)prVhtOpMode,
		   sizeof(struct IE_VHT_OP_MODE_NOTIFICATION));

	prVhtOpMode->ucId = ELEM_ID_OP_MODE;
	prVhtOpMode->ucLength =
		sizeof(struct IE_VHT_OP_MODE_NOTIFICATION) - ELEM_HDR_LEN;

	DBGLOG(RLM, TRACE, "rlmFillVhtOpNotificationIE(%d) %u %u\n",
	       prBssInfo->ucBssIndex, fgIsOwnCap, prBssInfo->ucOpRxNss);

	if (fgIsOwnCap) {
#if CFG_SUPPORT_DBDC
		ucOpModeBw = cnmGetDbdcBwCapability(prAdapter,
						    prBssInfo->ucBssIndex);
#else
		ucOpModeBw = cnmGetBssMaxBw(prAdapter,
						    prBssInfo->ucBssIndex);
#endif
		/*handle 80P80 case*/
		if (ucOpModeBw >= MAX_BW_160MHZ) {
			ucOpModeBw = VHT_OP_MODE_CHANNEL_WIDTH_80;
			prVhtOpMode->ucOperatingMode |=
				VHT_OP_MODE_CHANNEL_WIDTH_80P80_160;
		}

		prVhtOpMode->ucOperatingMode |= ucOpModeBw;
		prVhtOpMode->ucOperatingMode |=
			(((prBssInfo->ucOpRxNss - 1)
				<< VHT_OP_MODE_RX_NSS_OFFSET) &
			 VHT_OP_MODE_RX_NSS);
	} else {

		ucOpModeBw = rlmGetOpModeBwByVhtAndHtOpInfo(prBssInfo);
		if (ucOpModeBw == VHT_OP_MODE_CHANNEL_WIDTH_160_80P80) {
			ucOpModeBw = VHT_OP_MODE_CHANNEL_WIDTH_80;
			prVhtOpMode->ucOperatingMode |=
				VHT_OP_MODE_CHANNEL_WIDTH_80P80_160;
		}

		prVhtOpMode->ucOperatingMode |= ucOpModeBw;
		prVhtOpMode->ucOperatingMode |=
			(((prBssInfo->ucOpRxNss - 1)
				<< VHT_OP_MODE_RX_NSS_OFFSET) &
			 VHT_OP_MODE_RX_NSS);
	}

	prMsduInfo->u2FrameLength += IE_SIZE(prVhtOpMode);
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief
 *
 * \param[in]
 *
 * \return none
 */
/*----------------------------------------------------------------------------*/
static void rlmFillVhtCapIE(struct ADAPTER *prAdapter,
			    struct BSS_INFO *prBssInfo,
			    struct MSDU_INFO *prMsduInfo)
{
	struct IE_VHT_CAP *prVhtCap;
	struct VHT_SUPPORTED_MCS_FIELD *prVhtSupportedMcsSet;
	uint8_t i;
	uint8_t ucMaxBw;
#if CFG_SUPPORT_BFEE
	struct STA_RECORD *prStaRec;
#endif

	ASSERT(prAdapter);
	ASSERT(prBssInfo);
	ASSERT(prMsduInfo);

	prVhtCap = (struct IE_VHT_CAP *)(((uint8_t *)prMsduInfo->prPacket) +
					 prMsduInfo->u2FrameLength);

	prVhtCap->ucId = ELEM_ID_VHT_CAP;
	prVhtCap->ucLength = sizeof(struct IE_VHT_CAP) - ELEM_HDR_LEN;
	prVhtCap->u4VhtCapInfo = VHT_CAP_INFO_DEFAULT_VAL;

	ucMaxBw = cnmGetBssMaxBw(prAdapter, prBssInfo->ucBssIndex);

	prVhtCap->u4VhtCapInfo |= (prAdapter->rWifiVar.ucRxMaxMpduLen &
				   VHT_CAP_INFO_MAX_MPDU_LEN_MASK);
#if CFG_SUPPORT_VHT_IE_IN_2G
	if (prBssInfo->eBand == BAND_2G4) {
		prVhtCap->u4VhtCapInfo |=
			VHT_CAP_INFO_MAX_SUP_CHANNEL_WIDTH_SET_NONE;
	} else {
#endif
		if (ucMaxBw == MAX_BW_160MHZ)
			prVhtCap->u4VhtCapInfo |=
				VHT_CAP_INFO_MAX_SUP_CHANNEL_WIDTH_SET_160;
		else if (ucMaxBw == MAX_BW_80_80_MHZ)
			prVhtCap->u4VhtCapInfo |=
		VHT_CAP_INFO_MAX_SUP_CHANNEL_WIDTH_SET_160_80P80;

		if (IS_FEATURE_ENABLED(prAdapter->rWifiVar.ucStaVhtBfee)) {
			prVhtCap->u4VhtCapInfo |= FIELD_VHT_CAP_INFO_BFEE;
#if CFG_SUPPORT_BFEE
		prStaRec = cnmGetStaRecByIndex(prAdapter,
					       prMsduInfo->ucStaRecIndex);

		if (prStaRec &&
		(prStaRec->ucVhtCapNumSoundingDimensions == 0x2) &&
		!prAdapter->rWifiVar.fgForceSTSNum) {
		/* For the compatibility with netgear R7000 AP */
			prVhtCap->u4VhtCapInfo |=
		(((uint32_t)prStaRec->ucVhtCapNumSoundingDimensions)
<< VHT_CAP_INFO_COMPRESSED_STEERING_NUMBER_OF_BEAMFORMER_ANTENNAS_SUP_OFF);
			DBGLOG(RLM, INFO, "Set VHT Cap BFEE STS CAP=%d\n",
				       prStaRec->ucVhtCapNumSoundingDimensions);
		} else {
		/* For 11ac cert. VHT-5.2.63C MU-BFee step3,
		 * it requires STAUT to set its maximum STS capability here
		 */
			prVhtCap->u4VhtCapInfo |=
VHT_CAP_INFO_COMPRESSED_STEERING_NUMBER_OF_BEAMFORMER_ANTENNAS_4_SUP;
			DBGLOG(RLM, TRACE, "Set VHT Cap BFEE STS CAP=%d\n",
				VHT_CAP_INFO_BEAMFORMEE_STS_CAP_MAX);
		}
/* DBGLOG(RLM, INFO, "VhtCapInfo=%x\n", prVhtCap->u4VhtCapInfo); */
#endif
		if (IS_FEATURE_ENABLED(prAdapter->rWifiVar.ucStaVhtMuBfee))
			prVhtCap->u4VhtCapInfo |=
				VHT_CAP_INFO_MU_BEAMFOMEE_CAPABLE;
		}

		if (IS_FEATURE_ENABLED(prAdapter->rWifiVar.ucStaVhtBfer))
			prVhtCap->u4VhtCapInfo |= FIELD_VHT_CAP_INFO_BFER;
#if CFG_SUPPORT_VHT_IE_IN_2G
	}
#endif
	if (IS_FEATURE_ENABLED(prAdapter->rWifiVar.ucRxShortGI)) {
		if (ucMaxBw >= MAX_BW_80MHZ)
			prVhtCap->u4VhtCapInfo |= VHT_CAP_INFO_SHORT_GI_80;

		if (ucMaxBw >= MAX_BW_160MHZ)
			prVhtCap->u4VhtCapInfo |=
				VHT_CAP_INFO_SHORT_GI_160_80P80;
	}

	if (IS_FEATURE_ENABLED(prAdapter->rWifiVar.ucRxLdpc))
		prVhtCap->u4VhtCapInfo |= VHT_CAP_INFO_RX_LDPC;

	if (IS_FEATURE_ENABLED(prAdapter->rWifiVar.ucRxStbc)) {
		uint8_t tempRxStbcNss;

		if (prAdapter->rWifiVar.u4SwTestMode ==
		    ENUM_SW_TEST_MODE_SIGMA_AC) {
			tempRxStbcNss = 1;
			DBGLOG(RLM, INFO,
			       "Set RxStbcNss to 1 for 11ac certification.\n");
		} else {
			tempRxStbcNss = prAdapter->rWifiVar.ucRxStbcNss;
			tempRxStbcNss =
				(tempRxStbcNss >
				 wlanGetSupportNss(prAdapter,
						   prBssInfo->ucBssIndex))
					? wlanGetSupportNss(
						  prAdapter,
						  prBssInfo->ucBssIndex)
					: (tempRxStbcNss);
			if (tempRxStbcNss != prAdapter->rWifiVar.ucRxStbcNss) {
				DBGLOG(RLM, WARN,
				       "Apply Nss:%d as RxStbcNss in VHT Cap",
				       wlanGetSupportNss(
					       prAdapter,
					       prBssInfo->ucBssIndex));
				DBGLOG(RLM, WARN,
				       "due to set RxStbcNss more than Nss is not appropriate.\n");
			}
		}
		prVhtCap->u4VhtCapInfo |=
			((tempRxStbcNss << VHT_CAP_INFO_RX_STBC_OFFSET) &
			 VHT_CAP_INFO_RX_STBC_MASK);
	}

	if (IS_FEATURE_ENABLED(prAdapter->rWifiVar.ucTxStbc) &&
			prBssInfo->ucOpTxNss >= 2)
		prVhtCap->u4VhtCapInfo |= VHT_CAP_INFO_TX_STBC;

	/*set MCS map */
	prVhtSupportedMcsSet = &prVhtCap->rVhtSupportedMcsSet;
	kalMemZero((void *)prVhtSupportedMcsSet,
		   sizeof(struct VHT_SUPPORTED_MCS_FIELD));

	for (i = 0; i < 8; i++) {
		uint8_t ucOffset = i * 2;
		uint8_t ucMcsMap;

		if (i < wlanGetSupportNss(prAdapter, prBssInfo->ucBssIndex))
			ucMcsMap = VHT_CAP_INFO_MCS_MAP_MCS9;
		else
			ucMcsMap = VHT_CAP_INFO_MCS_NOT_SUPPORTED;

		prVhtSupportedMcsSet->u2RxMcsMap |= (ucMcsMap << ucOffset);
		prVhtSupportedMcsSet->u2TxMcsMap |= (ucMcsMap << ucOffset);
	}

#if 0
	for (i = 0; i < wlanGetSupportNss(prAdapter,
		prBssInfo->ucBssIndex); i++) {
		uint8_t ucOffset = i * 2;

		prVhtSupportedMcsSet->u2RxMcsMap &=
			((VHT_CAP_INFO_MCS_MAP_MCS9 << ucOffset) &
			BITS(ucOffset, ucOffset + 1));
		prVhtSupportedMcsSet->u2TxMcsMap &=
			((VHT_CAP_INFO_MCS_MAP_MCS9 << ucOffset) &
			BITS(ucOffset, ucOffset + 1));
	}
#endif

	prVhtSupportedMcsSet->u2RxHighestSupportedDataRate =
		VHT_CAP_INFO_DEFAULT_HIGHEST_DATA_RATE;
	prVhtSupportedMcsSet->u2TxHighestSupportedDataRate =
		VHT_CAP_INFO_DEFAULT_HIGHEST_DATA_RATE;

	ASSERT(IE_SIZE(prVhtCap) <= (ELEM_HDR_LEN + ELEM_MAX_LEN_VHT_CAP));

	prMsduInfo->u2FrameLength += IE_SIZE(prVhtCap);
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief
 *
 * \param[in]
 *
 * \return none
 */
/*----------------------------------------------------------------------------*/
void rlmFillVhtOpIE(struct ADAPTER *prAdapter, struct BSS_INFO *prBssInfo,
		    struct MSDU_INFO *prMsduInfo)
{
	struct IE_VHT_OP *prVhtOp;

	ASSERT(prAdapter);
	ASSERT(prBssInfo);
	ASSERT(prMsduInfo);

	prVhtOp = (struct IE_VHT_OP *)(((uint8_t *)prMsduInfo->prPacket) +
				       prMsduInfo->u2FrameLength);

	/* Add HT operation IE */
	prVhtOp->ucId = ELEM_ID_VHT_OP;
	prVhtOp->ucLength = sizeof(struct IE_VHT_OP) - ELEM_HDR_LEN;

	ASSERT(IE_SIZE(prVhtOp) <= (ELEM_HDR_LEN + ELEM_MAX_LEN_VHT_OP));

	/* (UINT8)VHT_OP_CHANNEL_WIDTH_80; */
	prVhtOp->ucVhtOperation[0] = prBssInfo->ucVhtChannelWidth;
	prVhtOp->ucVhtOperation[1] = prBssInfo->ucVhtChannelFrequencyS1;
	prVhtOp->ucVhtOperation[2] = prBssInfo->ucVhtChannelFrequencyS2;

#if 0
	if (cnmGetBssMaxBw(prAdapter, prBssInfo->ucBssIndex) < MAX_BW_80MHZ) {
		prVhtOp->ucVhtOperation[0] = VHT_OP_CHANNEL_WIDTH_20_40;
		prVhtOp->ucVhtOperation[1] = 0;
		prVhtOp->ucVhtOperation[2] = 0;
	} else if (cnmGetBssMaxBw(prAdapter, prBssInfo->ucBssIndex) ==
			MAX_BW_80MHZ) {
		prVhtOp->ucVhtOperation[0] = VHT_OP_CHANNEL_WIDTH_80;
		prVhtOp->ucVhtOperation[1] =
			nicGetVhtS1(prBssInfo->ucPrimaryChannel);
		prVhtOp->ucVhtOperation[2] = 0;
	} else {
		/* TODO: BW80 + 80/160 support */
	}
#endif

	prVhtOp->u2VhtBasicMcsSet = prBssInfo->u2VhtBasicMcsSet;

	prMsduInfo->u2FrameLength += IE_SIZE(prVhtOp);
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief Get RxNss from VHT CAP IE.
 *
 * \param[in] prVhtCap
 *
 * \return ucRxNss
 */
/*----------------------------------------------------------------------------*/
uint8_t
rlmGetSupportRxNssInVhtCap(struct IE_VHT_CAP *prVhtCap)
{
	uint8_t ucRxNss = 1;

	if (!prVhtCap) {
		DBGLOG(RLM, TRACE, "null prVhtCap, assume RxNss=1\n");
		return ucRxNss;
	}

	if (((prVhtCap->rVhtSupportedMcsSet.u2RxMcsMap &
		VHT_CAP_INFO_MCS_2SS_MASK) >>
		VHT_CAP_INFO_MCS_2SS_OFFSET)
		!= VHT_CAP_INFO_MCS_NOT_SUPPORTED)
		ucRxNss = 2;
	if (((prVhtCap->rVhtSupportedMcsSet.u2RxMcsMap &
		VHT_CAP_INFO_MCS_3SS_MASK)
		>> VHT_CAP_INFO_MCS_3SS_OFFSET)
		!= VHT_CAP_INFO_MCS_NOT_SUPPORTED)
		ucRxNss = 3;
	if (((prVhtCap->rVhtSupportedMcsSet.u2RxMcsMap &
		VHT_CAP_INFO_MCS_4SS_MASK)
		>> VHT_CAP_INFO_MCS_4SS_OFFSET)
		!= VHT_CAP_INFO_MCS_NOT_SUPPORTED)
		ucRxNss = 4;
	if (((prVhtCap->rVhtSupportedMcsSet.u2RxMcsMap &
		VHT_CAP_INFO_MCS_5SS_MASK)
		>> VHT_CAP_INFO_MCS_5SS_OFFSET)
		!= VHT_CAP_INFO_MCS_NOT_SUPPORTED)
		ucRxNss = 5;
	if (((prVhtCap->rVhtSupportedMcsSet.u2RxMcsMap &
		VHT_CAP_INFO_MCS_6SS_MASK)
		>> VHT_CAP_INFO_MCS_6SS_OFFSET)
		!= VHT_CAP_INFO_MCS_NOT_SUPPORTED)
		ucRxNss = 6;
	if (((prVhtCap->rVhtSupportedMcsSet.u2RxMcsMap &
		VHT_CAP_INFO_MCS_7SS_MASK)
		>> VHT_CAP_INFO_MCS_7SS_OFFSET)
		!= VHT_CAP_INFO_MCS_NOT_SUPPORTED)
		ucRxNss = 7;
	if (((prVhtCap->rVhtSupportedMcsSet.u2RxMcsMap &
		VHT_CAP_INFO_MCS_8SS_MASK)
		>> VHT_CAP_INFO_MCS_8SS_OFFSET)
		!= VHT_CAP_INFO_MCS_NOT_SUPPORTED)
		ucRxNss = 8;

	return ucRxNss;
}

#endif

#if CFG_SUPPORT_802_11D
/*----------------------------------------------------------------------------*/
/*!
 * \brief
 *
 * \param[in]
 *
 * \return none
 */
/*----------------------------------------------------------------------------*/
void rlmGenerateCountryIE(struct ADAPTER *prAdapter,
		struct MSDU_INFO *prMsduInfo)
{
	struct BSS_INFO *prBssInfo =
		prAdapter->aprBssInfo[prMsduInfo->ucBssIndex];
	unsigned char *pucBuf =
		(((unsigned char *) prMsduInfo->prPacket) +
		prMsduInfo->u2FrameLength);

	if (prBssInfo->aucCountryStr[0] == 0)
		return;

	COUNTRY_IE(pucBuf)->ucId = ELEM_ID_COUNTRY_INFO;
	COUNTRY_IE(pucBuf)->ucLength = prBssInfo->ucCountryIELen;
	COUNTRY_IE(pucBuf)->aucCountryStr[0] = prBssInfo->aucCountryStr[0];
	COUNTRY_IE(pucBuf)->aucCountryStr[1] = prBssInfo->aucCountryStr[1];
	COUNTRY_IE(pucBuf)->aucCountryStr[2] = prBssInfo->aucCountryStr[2];
	kalMemCopy(COUNTRY_IE(pucBuf)->arCountryStr,
		prBssInfo->aucSubbandTriplet,
		prBssInfo->ucCountryIELen - 3);

	prMsduInfo->u2FrameLength += IE_SIZE(pucBuf);
}
#endif

#if CFG_SUPPORT_ONE_TIME_CAL
/*----------------------------------------------------------------------------*/
/*!
 * \brief Start One Time Cal command
 *
 * \param[in] prAdapter
 *
 * \return WLAN_STATUS
 */
/*----------------------------------------------------------------------------*/
uint32_t rlmOneTimeCalStart(
	IN struct ADAPTER *prAdapter
)
{
	struct ONE_TIME_CAL_FILE *prOneTimeCalFile;

	DBGLOG(RLM, INFO, "%s start\n", __func__);

	prAdapter->fgOneTimeCalOngoing = TRUE;

	/* Start one time cal timer */
	if (prAdapter->rOneTimeCalTimer.pfMgmtTimeOutFunc == NULL) {
		cnmTimerInitTimer(prAdapter,
			&prAdapter->rOneTimeCalTimer,
			(PFN_MGMT_TIMEOUT_FUNC) rlmOneTimeCalStop,
			(unsigned long) NULL);

		prOneTimeCalFile = &prAdapter->rOneTimeCalFile;
		kalMemZero(prOneTimeCalFile, sizeof(struct ONE_TIME_CAL_FILE));

		DBGLOG(RLM, INFO, "rOneTimeCalTimer start\n");
	}

	cnmTimerStartTimer(
		prAdapter, &prAdapter->rOneTimeCalTimer,
		SEC_TO_MSEC(ONE_TIME_CAL_GET_TIMEOUT_TH));

	/* Read the file and update file struct */
	if (rlmOneTimeCalReadFromFile(prAdapter) != WLAN_STATUS_SUCCESS) {
		DBGLOG(RLM, ERROR, "Fail to read from file\n");
		return WLAN_STATUS_FAILURE;
	}

	return WLAN_STATUS_SUCCESS;
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief Stop One Time Cal command
 *        Timeout function
 *
 * \param[in] prAdapter
 *            ulParamPtr
 *
 * \return void
 */
/*----------------------------------------------------------------------------*/
void rlmOneTimeCalStop(
	IN struct ADAPTER *prAdapter,
	IN unsigned long ulParamPtr
)
{
	struct ONE_TIME_CAL_FILE *prOneTimeCalFile;

	DBGLOG(RLM, INFO, "%s start\n", __func__);

	cnmTimerStopTimer(prAdapter, &prAdapter->rOneTimeCalTimer);

	prOneTimeCalFile = &prAdapter->rOneTimeCalFile;
	kalMemZero(prOneTimeCalFile, sizeof(struct ONE_TIME_CAL_FILE));

	prAdapter->fgOneTimeCalOngoing = FALSE;
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief Find the group index and channel index in array
 *
 * \param[in] prAdapter
 *            eBand
 *            ucChannel
 *       [out] pucGroup
 *             pucChIdx
 *
 * \return WLAN_STATUS
 */
/*----------------------------------------------------------------------------*/
uint32_t rlmCh2Group(
	IN struct ADAPTER *prAdapter,
	IN enum ONE_TIME_CAL_BAND eBand,
	IN uint8_t ucChannel,
	OUT uint8_t *pucGroup,
	OUT uint8_t *pucChIdx
)
{
	struct ieee80211_supported_band *prChList = NULL;
	struct wiphy *prWiphy;
	uint8_t ucChannelNum = 0;
	uint8_t ucChIdx = 0, ucGrpDefIdx = 0;
	uint16_t u2Freq = 0;

	DBGLOG(RLM, INFO, "%s start\n", __func__);
	DBGLOG(RLM, INFO, "Input eBand=%u, ucChannel=%u\n", eBand, ucChannel);

	prWiphy = priv_to_wiphy(prAdapter->prGlueInfo);

	/* Get the channel list by band */
	if (eBand == ONE_TIME_CAL_BAND_2G) {
		prChList = prWiphy->bands[KAL_BAND_2GHZ];
		ucChannelNum = prChList->n_channels;

	} else if (eBand == ONE_TIME_CAL_BAND_5G) {
		prChList = prWiphy->bands[KAL_BAND_5GHZ];
		ucChannelNum = prChList->n_channels;

#if (CFG_SUPPORT_WIFI_6G == 1)

	} else if (eBand == ONE_TIME_CAL_BAND_6G) {
		prChList = prWiphy->bands[KAL_BAND_6GHZ];
		ucChannelNum = prChList->n_channels;

#endif

	} else
		return WLAN_STATUS_FAILURE;


	/* Get the center freq of channel */
	for (ucChIdx = 0; ucChIdx < ucChannelNum; ucChIdx++) {

		if ((prChList->channels[ucChIdx]).hw_value == ucChannel) {
			u2Freq = (prChList->channels[ucChIdx]).center_freq;
			break;
		}
	}

	if (ucChIdx >= ucChannelNum) {
		DBGLOG(RLM, ERROR,
			"Error: find no center_freq for eBand %u ch%u\n",
			eBand, ucChannel);
		return WLAN_STATUS_FAILURE;
	}


	/* Get the group by center freq of channel */
	for (ucGrpDefIdx = 0;
		ucGrpDefIdx < ONE_TIME_CAL_GROUP_NUM; ucGrpDefIdx++) {

		if (u2Freq >= GROUP_FREQ_DEF_ARR[ucGrpDefIdx].ucFreqStart &&
			u2Freq <= GROUP_FREQ_DEF_ARR[ucGrpDefIdx].ucFreqEnd) {
			*pucGroup =
				GROUP_FREQ_DEF_ARR[ucGrpDefIdx].ucGroupIdx;
			break;
		}
	}

	if (*pucGroup >= ONE_TIME_CAL_GROUP_NUM) {
		DBGLOG(RLM, ERROR,
			"Error: find no group for eBand %u ch%u\n",
			eBand, ucChannel);
		return WLAN_STATUS_FAILURE;
	}


	/* Get the index of DPD cal in cal data array */
	if (eBand > ONE_TIME_CAL_BAND_2G)
		ucChIdx += ONE_TIME_CAL_CH_NUM_2G;
	if (eBand > ONE_TIME_CAL_BAND_5G)
		ucChIdx += ONE_TIME_CAL_CH_NUM_5G;

	*pucChIdx = ucChIdx;

	DBGLOG(RLM, INFO, "Find group %u and chIdx %u for eBand %u ch%u\n",
		*pucGroup, *pucChIdx, eBand, ucChannel);

	return WLAN_STATUS_SUCCESS;
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief Get/Set the stored data from/to file buffer
 *
 * \param[in] prOneTimeCalFileBlk
 *            eType
 *            paucFileBuf
 *            pu4FileBufOffset
 *            fgSet
 *
 * \return WLAN_STATUS
 */
/*----------------------------------------------------------------------------*/
uint32_t rlmOneTimeCalUpdateFileBuf(
	IN void *prOneTimeCalFileBlk,
	IN enum ONE_TIME_CAL_TYPE eType,
	IN uint8_t *paucFileBuf,
	IN uint32_t *pu4FileBufOffset,
	IN uint8_t fgSet
)
{
	struct ONE_TIME_CAL_BLK_INFO *prOneTimeCalFileBlkInfo;
	uint32_t u4BufLen = 0;
	uint32_t *pu4CacheMark;
	uint32_t *pu4Temperature;
	uint8_t *pucFileBlkCalData;

	DBGLOG(RLM, INFO, "%s start\n", __func__);


	/* Copy the block info */
	switch (eType) {

	case ONE_TIME_CAL_TYPE_COMMON:

		prOneTimeCalFileBlkInfo =
			&((struct ONE_TIME_CAL_FILE_BLK_COM *)
				prOneTimeCalFileBlk)->rOneTimeCalBlkInfo;
		break;

	case ONE_TIME_CAL_TYPE_GROUP:

		prOneTimeCalFileBlkInfo =
			&((struct ONE_TIME_CAL_FILE_BLK_GRP *)
				prOneTimeCalFileBlk)->rOneTimeCalBlkInfo;
		break;

	case ONE_TIME_CAL_TYPE_CHANNEL:

		prOneTimeCalFileBlkInfo =
			&((struct ONE_TIME_CAL_FILE_BLK_CH *)
				prOneTimeCalFileBlk)->rOneTimeCalBlkInfo;
		break;

	default:

		DBGLOG(RLM, ERROR, "Error eType\n");
		return WLAN_STATUS_FAILURE;

	}

	if (fgSet) {

		kalMemCopy(&paucFileBuf[*pu4FileBufOffset],
			prOneTimeCalFileBlkInfo,
			sizeof(struct ONE_TIME_CAL_BLK_INFO));
		*pu4FileBufOffset += sizeof(struct ONE_TIME_CAL_BLK_INFO);

	} else {

		kalMemCopy(prOneTimeCalFileBlkInfo,
			&paucFileBuf[*pu4FileBufOffset],
			sizeof(struct ONE_TIME_CAL_BLK_INFO));
		*pu4FileBufOffset += sizeof(struct ONE_TIME_CAL_BLK_INFO);

	}

	DBGLOG(RLM, INFO,
		"Copy BlkInfo with BufOffset=%u\n", *pu4FileBufOffset);



	/* Copy the cal data */
	switch (eType) {
	case ONE_TIME_CAL_TYPE_COMMON:

		u4BufLen = ONE_TIME_CAL_FILE_BLK_LEN_COM;

		pucFileBlkCalData =
			&((struct ONE_TIME_CAL_FILE_BLK_COM *)
				prOneTimeCalFileBlk)->aucCalData[0];

		break;

	case ONE_TIME_CAL_TYPE_GROUP:

		u4BufLen = ONE_TIME_CAL_FILE_BLK_LEN_GRP;

		pucFileBlkCalData =
			&((struct ONE_TIME_CAL_FILE_BLK_GRP *)
				prOneTimeCalFileBlk)->aucCalData[0];

		break;

	case ONE_TIME_CAL_TYPE_CHANNEL:

		u4BufLen = ONE_TIME_CAL_FILE_BLK_LEN_CH;

		pu4CacheMark =
			&((struct ONE_TIME_CAL_FILE_BLK_CH *)
				prOneTimeCalFileBlk)->u4CacheMark;

		pu4Temperature =
			&((struct ONE_TIME_CAL_FILE_BLK_CH *)
				prOneTimeCalFileBlk)->u4Temperature;

		pucFileBlkCalData =
			&((struct ONE_TIME_CAL_FILE_BLK_CH *)
				prOneTimeCalFileBlk)->aucCalData[0];

		if (fgSet) {

			kalMemCopy(&paucFileBuf[*pu4FileBufOffset],
				pu4CacheMark, sizeof(uint32_t));
			*pu4FileBufOffset += sizeof(uint32_t);

			kalMemCopy(&paucFileBuf[*pu4FileBufOffset],
				pu4Temperature, sizeof(uint32_t));
			*pu4FileBufOffset += sizeof(uint32_t);

		} else {

			kalMemCopy(pu4CacheMark,
				&paucFileBuf[*pu4FileBufOffset],
				sizeof(uint32_t));
			*pu4FileBufOffset += sizeof(uint32_t);

			kalMemCopy(pu4Temperature,
				&paucFileBuf[*pu4FileBufOffset],
				sizeof(uint32_t));
			*pu4FileBufOffset += sizeof(uint32_t);

		}

		DBGLOG(RLM, INFO,
			"Copy CacheMark and Temperature with uBufOffset=%u\n",
			*pu4FileBufOffset);

		break;

	default:

		DBGLOG(RLM, ERROR, "Error eType\n");

		return WLAN_STATUS_FAILURE;

	}

	DBGLOG(RLM, INFO, "Cal data length=%u\n", u4BufLen);


	if (fgSet) {

		kalMemCopy(&paucFileBuf[*pu4FileBufOffset], pucFileBlkCalData,
			u4BufLen);
		*pu4FileBufOffset += u4BufLen;

	} else {

		kalMemCopy(pucFileBlkCalData, &paucFileBuf[*pu4FileBufOffset],
			u4BufLen);
		*pu4FileBufOffset += u4BufLen;

	}

	DBGLOG(RLM, INFO, "Copy cal data with u4FileBufOffset=%u\n",
		*pu4FileBufOffset);


	return WLAN_STATUS_SUCCESS;
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief Get the range of file blocks of each file
 *
 * \param[in] ucFileNum
 *       [out] pucFileBlkNum
 *             pucBlkCtr
 *             pucBlkCtrMax
 *
 * \return WLAN_STATUS
 */
/*----------------------------------------------------------------------------*/
uint32_t rlmOneTimeCalGetFileBlkNum(
	IN uint8_t ucFileNum,
	OUT uint8_t *pucFileBlkNum,
	OUT uint8_t *pucBlkCtr,
	OUT uint8_t *pucBlkCtrMax
)
{
	DBGLOG(RLM, INFO, "%s start\n", __func__);

	switch (ucFileNum) {

	/* Common & Group */
	case 1:
		*pucBlkCtr = 0;
		*pucBlkCtrMax = 0;
		*pucFileBlkNum = ONE_TIME_CAL_BAND_NUM +
			ONE_TIME_CAL_GROUP_NUM * ONE_TIME_CAL_GRP_BLK_NUM;
		break;

	/* Channel 2G*/
	case 2:
		*pucBlkCtr = 0;
		*pucBlkCtrMax = ONE_TIME_CAL_CH_NUM_2G - 1;
		*pucFileBlkNum =
			(*pucBlkCtrMax - *pucBlkCtr + 1) *
			ONE_TIME_CAL_DPD_BLK_NUM;
		break;

	/* Channel 5G 36~112 116~165 */
	case 3:
	case 4:
		*pucBlkCtr =
			(ucFileNum - 3) * (ONE_TIME_CAL_CH_NUM_5G/2);
		*pucBlkCtrMax = (ucFileNum == 3) ?
			(ONE_TIME_CAL_CH_NUM_5G/2-1) :
			(ONE_TIME_CAL_CH_NUM_5G-1);
		*pucFileBlkNum =
			(*pucBlkCtrMax - *pucBlkCtr + 1) *
			ONE_TIME_CAL_DPD_BLK_NUM;
		*pucBlkCtr += ONE_TIME_CAL_CH_NUM_2G;
		*pucBlkCtrMax += ONE_TIME_CAL_CH_NUM_2G;
		break;

#if (CFG_SUPPORT_WIFI_6G == 1)

	case 5:
	case 6:
	case 7:
	case 8:
		*pucBlkCtr =
			(ucFileNum - 5) * (ONE_TIME_CAL_CH_NUM_6G/4);
		*pucBlkCtrMax = (ucFileNum == 8) ?
			(ONE_TIME_CAL_CH_NUM_6G) :
			(*pucBlkCtr + (ONE_TIME_CAL_CH_NUM_6G/4));
		*pucFileBlkNum = (*pucBlkCtrMax - *pucBlkCtr + 1) *
			ONE_TIME_CAL_DPD_BLK_NUM;
		*pucBlkCtr +=
			ONE_TIME_CAL_CH_NUM_2G + ONE_TIME_CAL_CH_NUM_5G;
		*pucBlkCtrMax +=
			ONE_TIME_CAL_CH_NUM_2G + ONE_TIME_CAL_CH_NUM_5G;
		break;

#endif

	default:
		DBGLOG(RLM, STATE, "Error ucFileNum=%u\n", ucFileNum);
		return WLAN_STATUS_FAILURE;

	}


	DBGLOG(RLM, INFO,
		"Get the pucBlkCtr=%u and pucBlkCtrMax=%u for file %u\n",
		pucBlkCtr, pucBlkCtrMax, ucFileNum);

	return WLAN_STATUS_SUCCESS;
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief Download the cal data in prAdapter->prOneTimeCalData from file
 *
 * \param[in] prAdapter
 *
 * \return WLAN_STATUS
 */
/*----------------------------------------------------------------------------*/
uint32_t rlmOneTimeCalReadFromFile(
	IN struct ADAPTER *prAdapter
)
{
	uint32_t u4ReadSize = 0;
	struct ONE_TIME_CAL_FILE *prOneTimeCalFile;
	void *prOneTimeCalFileBlk;
	uint8_t ucFileNum = 1, ucBlkCtr = 0, ucBlkCtr2 = 0;
	uint8_t ucBlkCtrMax = 0, ucFileBlkNum = 0;
	uint8_t aucFilePath[256];
	uint8_t *pucFileBuf;
	uint32_t u4FileBufOffset = 0;


	DBGLOG(RLM, INFO, "%s start\n", __func__);

	prOneTimeCalFile = &prAdapter->rOneTimeCalFile;

	pucFileBuf = &prAdapter->aucOneTimeCalFileBuf[0];


	/* Read each file */
	for (ucFileNum = 1; ucFileNum <= ONE_TIME_CAL_FILE_NUM; ucFileNum++) {

		/* Get the file path */
		kalMemZero(&aucFilePath[0], sizeof(aucFilePath));
		snprintf(aucFilePath, sizeof(aucFilePath),
			"%sone_time_cal_%d.bin",
			ONE_TIME_CAL_DATA_FILE_DIR, ucFileNum);

		DBGLOG(RLM, INFO, "Reading file path=%s\n", aucFilePath);

		kalMemZero(pucFileBuf, sizeof(prAdapter->aucOneTimeCalFileBuf));
		u4FileBufOffset = 0;


		/* Get the ragne of including file blocks */
		rlmOneTimeCalGetFileBlkNum(ucFileNum, &ucFileBlkNum,
			&ucBlkCtr, &ucBlkCtrMax);


		/* Read the file in buffer */
		if (kalReadToFile(&aucFilePath[0], pucFileBuf,
			sizeof(prAdapter->aucOneTimeCalFileBuf), &u4ReadSize)) {

			DBGLOG(RLM, ERROR, "Cannot read %s\n", aucFilePath);

			continue;
		}

		if (ucFileNum == 1) {

			/* one_time_cal_1.bin */
			/* Common + Group */

			for (ucBlkCtr = 0;
				ucBlkCtr < ONE_TIME_CAL_BAND_NUM; ucBlkCtr++) {

				prOneTimeCalFileBlk = &prOneTimeCalFile
					->rComCalData[ucBlkCtr];

				rlmOneTimeCalUpdateFileBuf(prOneTimeCalFileBlk,
					ONE_TIME_CAL_TYPE_COMMON,
					pucFileBuf, &u4FileBufOffset,
					FALSE);
			}

			DBGLOG(RLM, INFO,
				"Copy common cal data with %u\n",
				u4FileBufOffset);

			for (ucBlkCtr = 0;
				ucBlkCtr < ONE_TIME_CAL_GROUP_NUM; ucBlkCtr++) {

				for (ucBlkCtr2 = 0;
					ucBlkCtr2 < ONE_TIME_CAL_GRP_BLK_NUM;
					ucBlkCtr2++) {

					prOneTimeCalFileBlk = &prOneTimeCalFile
						->rGrpCalData
						[ucBlkCtr][ucBlkCtr2];

					rlmOneTimeCalUpdateFileBuf(
						prOneTimeCalFileBlk,
						ONE_TIME_CAL_TYPE_GROUP,
						pucFileBuf, &u4FileBufOffset,
						FALSE);

				}
			}

			DBGLOG(RLM, INFO,
				"Copy group cal data with %u\n",
				u4FileBufOffset);


		} else {

			/* one_time_cal_2.bin ~ one_time_cal_8.bin */
			/* Channel */

			for (ucBlkCtr = ucBlkCtr;
				ucBlkCtr <= ucBlkCtrMax; ucBlkCtr++) {

				for (ucBlkCtr2 = 0;
					ucBlkCtr2 < ONE_TIME_CAL_DPD_BLK_NUM;
					ucBlkCtr2++) {

					prOneTimeCalFileBlk =
						&prOneTimeCalFile
						->rDpdCalData
						[ucBlkCtr][ucBlkCtr2];

					rlmOneTimeCalUpdateFileBuf(
						prOneTimeCalFileBlk,
						ONE_TIME_CAL_TYPE_CHANNEL,
						pucFileBuf, &u4FileBufOffset,
						FALSE);

				}
			}

			DBGLOG(RLM, INFO,
				"Copy channel cal data with %u\n",
				u4FileBufOffset);

		}

	}


	DBGLOG(RLM, INFO, "Read all cal data from files\n");

	return WLAN_STATUS_SUCCESS;
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief Write the cal data in prAdapter->prOneTimeCalData into file
 *
 * \param[in] prAdapter
 *
 * \return WLAN_STATUS
 */
/*----------------------------------------------------------------------------*/
uint32_t rlmOneTimeCalWriteToFile(
	IN struct ADAPTER *prAdapter
)
{
	struct ONE_TIME_CAL_FILE *prOneTimeCalFile;
	void *prOneTimeCalFileBlk;
	uint8_t ucFileNum = 1, ucBlkCtr = 0, ucBlkCtr2 = 0, ucBlkCtrMax = 0;
	uint8_t ucFileBlkNum = 0;
	uint8_t aucFilePath[256];
	uint8_t *pucFileBuf;
	uint32_t u4FileBufOffset = 0;

	DBGLOG(RLM, INFO, "%s start\n", __func__);

	prOneTimeCalFile = &prAdapter->rOneTimeCalFile;

	pucFileBuf = &prAdapter->aucOneTimeCalFileBuf[0];


	/* Wrtie each files */
	for (ucFileNum = 1; ucFileNum <= ONE_TIME_CAL_FILE_NUM; ucFileNum++) {

		kalMemZero(pucFileBuf, sizeof(prAdapter->aucOneTimeCalFileBuf));
		u4FileBufOffset = 0;

		DBGLOG(RLM, INFO, "ucFileNum=%u\n", ucFileNum);


		if (ucFileNum == 1) {

			/* Common */
			for (ucBlkCtr = 0;
				ucBlkCtr < ONE_TIME_CAL_BAND_NUM; ucBlkCtr++) {

				prOneTimeCalFileBlk = &prOneTimeCalFile
					->rComCalData[ucBlkCtr];

				rlmOneTimeCalUpdateFileBuf(
					prOneTimeCalFileBlk,
					ONE_TIME_CAL_TYPE_COMMON,
					pucFileBuf, &u4FileBufOffset,
					TRUE);
			}

			DBGLOG(RLM, INFO,
				"Copy common cal data with %u\n",
				u4FileBufOffset);

			/* Group */
			for (ucBlkCtr = 0;
				ucBlkCtr < ONE_TIME_CAL_GROUP_NUM; ucBlkCtr++) {

				for (ucBlkCtr2 = 0;
					ucBlkCtr2 < ONE_TIME_CAL_GRP_BLK_NUM;
					ucBlkCtr2++) {

					prOneTimeCalFileBlk =
						&(prOneTimeCalFile
						->rGrpCalData
						[ucBlkCtr][ucBlkCtr2]);

					rlmOneTimeCalUpdateFileBuf(
						prOneTimeCalFileBlk,
						ONE_TIME_CAL_TYPE_GROUP,
						pucFileBuf, &u4FileBufOffset,
						TRUE);

				}
			}

			DBGLOG(RLM, INFO,
				"Copy group cal data with %u\n",
				u4FileBufOffset);

		} else {

			rlmOneTimeCalGetFileBlkNum(ucFileNum,
				&ucFileBlkNum, &ucBlkCtr, &ucBlkCtrMax);

			for (; ucBlkCtr <= ucBlkCtrMax; ucBlkCtr++) {

				for (ucBlkCtr2 = 0;
					ucBlkCtr2 < ONE_TIME_CAL_DPD_BLK_NUM;
					ucBlkCtr2++) {

					prOneTimeCalFileBlk =
						&prOneTimeCalFile
						->rDpdCalData
						[ucBlkCtr][ucBlkCtr2];

					rlmOneTimeCalUpdateFileBuf(
						prOneTimeCalFileBlk,
						ONE_TIME_CAL_TYPE_CHANNEL,
						pucFileBuf, &u4FileBufOffset,
						TRUE);

				}
			}

			DBGLOG(RLM, INFO,
				"Copy channel cal data with %u\n",
				u4FileBufOffset);

		}


		/* Wrtie the buffer into file */
		kalMemZero(&aucFilePath[0], sizeof(aucFilePath));
		snprintf(aucFilePath, sizeof(aucFilePath),
			"%sone_time_cal_%d.bin",
			ONE_TIME_CAL_DATA_FILE_DIR, ucFileNum);

		DBGLOG(RLM, INFO, "Writing file path=%s\n", aucFilePath);

		if (kalWriteToFile(&aucFilePath[0],
				FALSE, pucFileBuf, u4FileBufOffset) != 0) {
			DBGLOG(RLM, ERROR, "Write file fail\n");

			return WLAN_STATUS_FAILURE;
		}

	}


	return WLAN_STATUS_SUCCESS;
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief Set/Get one tiem cal frag into/from
 *            the block in prAdapter->prOneTimeCalData
 *
 * \param[in] prAdapter
 *            prOneTimeCalNewFrag
 *            fgSet
 *
 * \return WLAN_STATUS
 */
/*----------------------------------------------------------------------------*/
uint32_t rlmOneTimeCalUpdateStruct(
	IN struct ADAPTER *prAdapter,
	IN struct ONE_TIME_CAL_DATA_BLK *prNewBlk,
	IN uint8_t fgSet
)
{
	struct ONE_TIME_CAL_FILE *prOneTimeCalFile;
	void *prFileBlk;
	struct ONE_TIME_CAL_BLK_INFO *prFileBlkInfo;
	struct ONE_TIME_CAL_BLK_INFO *prNewBlkInfo;
	enum ONE_TIME_CAL_BAND eBand = 0;
	uint8_t ucCalType = 0, ucBlkSeqNumCur = 0, ucFragSeqNumCur = 0;
	uint8_t ucBlkSeqNum = 0, ucFragSeqNum = 0;
	uint8_t ucGroup = 0, ucChIdx = 0, ucChannel = 0;
	uint32_t u4CalParam = 0, u4FragBufLen = 0, u4BlkBufLen = 0;
	uint8_t *pucFileBlkCalData, *pucNewBlkCalData;
	uint16_t u2FileBlkCalDataOffset = 0;


	DBGLOG(RLM, INFO, "%s start\n", __func__);

	prOneTimeCalFile = &prAdapter->rOneTimeCalFile;


	/* The info from the new block */
	prNewBlkInfo = &prNewBlk->rOneTimeCalBlkInfo;
	ucCalType = prNewBlkInfo->ucCalType;
	u4CalParam = prNewBlkInfo->u4CalParam;
	ucBlkSeqNumCur = prNewBlkInfo->ucBlkSeqNum & BITS(0, 3);
	ucFragSeqNumCur = prNewBlkInfo->ucBlkFragSeqNum & BITS(0, 3);
	u4FragBufLen = prNewBlkInfo->u4BlkLen & BITS(0, 15);

	DBGLOG(RLM, INFO,
		"Type=0x%X, CalParam=0x%X, BlkSeqCur=0x%X, FragSeqCur=0x%X\n",
		ucCalType, u4CalParam, ucBlkSeqNumCur, ucFragSeqNumCur);


	pucNewBlkCalData = &prNewBlk->aucCalData[0];
	u2FileBlkCalDataOffset = ONE_TIME_CAL_DATA_BLK_LEN * ucFragSeqNumCur;

	DBGLOG(RLM, INFO, "FileBlkDataOffset=%u\n",
			u2FileBlkCalDataOffset);

	/* Clear new data block if set */
	if (!fgSet)
		kalMemZero(prNewBlk, sizeof(struct ONE_TIME_CAL_DATA_BLK));


	/* Find the mapping file block */
	switch (ucCalType) {

	case ONE_TIME_CAL_TYPE_COMMON:

		eBand = u4CalParam & BITS(0, 7);
		prFileBlk = &(prOneTimeCalFile->rComCalData[eBand]);

		prFileBlkInfo =
			&(((struct ONE_TIME_CAL_FILE_BLK_COM *)
				prFileBlk)->rOneTimeCalBlkInfo);

		pucFileBlkCalData =
			&(((struct ONE_TIME_CAL_FILE_BLK_COM *)
				prFileBlk)
				->aucCalData[u2FileBlkCalDataOffset]);

		if (fgSet && ucFragSeqNumCur == 0)
			kalMemZero(
				&(((struct ONE_TIME_CAL_FILE_BLK_COM *)
					prFileBlk)->aucCalData[0]),
				ONE_TIME_CAL_FILE_BLK_LEN_COM);

		break;

	case ONE_TIME_CAL_TYPE_GROUP:

		ucGroup = u4CalParam & BITS(0, 7);
		prFileBlk = &(prOneTimeCalFile
			->rGrpCalData[ucGroup][ucBlkSeqNumCur]);

		prFileBlkInfo =
			&((struct ONE_TIME_CAL_FILE_BLK_GRP *)prFileBlk)
				->rOneTimeCalBlkInfo;

		pucFileBlkCalData =
			&((struct ONE_TIME_CAL_FILE_BLK_GRP *)prFileBlk)
				->aucCalData[u2FileBlkCalDataOffset];

		if (fgSet && ucFragSeqNumCur == 0)
			kalMemZero(
				&((struct ONE_TIME_CAL_FILE_BLK_GRP *)prFileBlk)
					->aucCalData[0],
				ONE_TIME_CAL_FILE_BLK_LEN_GRP);

		break;

	case ONE_TIME_CAL_TYPE_CHANNEL:

		ucChannel = u4CalParam & BITS(0, 11);
		eBand = (u4CalParam & BITS(12, 15)) >> 12;

		if (rlmCh2Group(prAdapter, eBand, ucChannel,
			&ucGroup, &ucChIdx) != WLAN_STATUS_SUCCESS)
			return WLAN_STATUS_FAILURE;

		prFileBlk = &(prOneTimeCalFile
			->rDpdCalData[ucChIdx][ucBlkSeqNumCur]);

		prFileBlkInfo =
			&(((struct ONE_TIME_CAL_FILE_BLK_CH *)prFileBlk)
				->rOneTimeCalBlkInfo);

		pucFileBlkCalData =
			&(((struct ONE_TIME_CAL_FILE_BLK_CH *)prFileBlk)
				->aucCalData[u2FileBlkCalDataOffset]);


		if (fgSet) {

			((struct ONE_TIME_CAL_FILE_BLK_CH *)prFileBlk)
				->u4CacheMark = prNewBlk->u4CacheMark;
			((struct ONE_TIME_CAL_FILE_BLK_CH *)prFileBlk)
				->u4Temperature = prNewBlk->u4Temperature;

			if (ucFragSeqNumCur == 0)
				kalMemZero(
					&((struct ONE_TIME_CAL_FILE_BLK_CH *)
						prFileBlk)->aucCalData[0],
					ONE_TIME_CAL_FILE_BLK_LEN_CH);

		} else {

			prNewBlk->u4CacheMark =
				((struct ONE_TIME_CAL_FILE_BLK_CH *)prFileBlk)
					->u4CacheMark;
			prNewBlk->u4Temperature =
				((struct ONE_TIME_CAL_FILE_BLK_CH *)prFileBlk)
					->u4Temperature;

		}

		break;

	default:

		DBGLOG(RLM, ERROR, "Error ucCalType=%u\n", ucCalType);

		return WLAN_STATUS_FAILURE;

	}



	/* Copy the info and cal data */
	if (fgSet) {

		kalMemCopy(prFileBlkInfo, prNewBlkInfo,
			sizeof(struct ONE_TIME_CAL_BLK_INFO));

		if (u4FragBufLen > 0)
			kalMemCopy(pucFileBlkCalData, pucNewBlkCalData,
				u4FragBufLen);

	} else {

		ucBlkSeqNum =
			(prFileBlkInfo->ucBlkSeqNum & BITS(4, 7)) >> 4;
		ucFragSeqNum =
			(prFileBlkInfo->ucBlkFragSeqNum & BITS(4, 7)) >> 4;
		u4BlkBufLen =
			(prFileBlkInfo->u4BlkLen & BITS(16, 31)) >> 16;

		DBGLOG(RLM, INFO, "BlkSeq=%u, FragSeq=%u, BlkLen=%u\n",
			ucBlkSeqNum, ucFragSeqNum, u4BlkBufLen);

		if (ucFragSeqNumCur == ucFragSeqNum - 1)
			u4FragBufLen = u4BlkBufLen - u2FileBlkCalDataOffset;
		else
			u4FragBufLen = ONE_TIME_CAL_DATA_BLK_LEN;

		DBGLOG(RLM, INFO, "FragLen=0x%X\n", u4FragBufLen);

		/* Get from block to new block frag*/
		prNewBlkInfo->ucCalType = ucCalType;
		prNewBlkInfo->ucBlkFragSeqNum =
			(ucFragSeqNum << 4) + ucFragSeqNumCur;
		prNewBlkInfo->ucBlkSeqNum =
			(ucBlkSeqNum << 4) + ucBlkSeqNumCur;
		prNewBlkInfo->u4BlkLen =
			(u4BlkBufLen << 16) + u4FragBufLen;
		prNewBlkInfo->u4CalParam = u4CalParam;

		kalMemCopy(pucNewBlkCalData, pucFileBlkCalData, u4FragBufLen);

	}


	return WLAN_STATUS_SUCCESS;
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief Sending cal data to FW to set one time cal
 *        uint: one time cal frag
 *
 * \param[in] prAdapter
 *            eCalType
 *            u4CalParam
 *
 * \return WLAN_STATUS
 */
/*----------------------------------------------------------------------------*/
uint32_t rlmSendFWOneTimeCalData(
	IN struct ADAPTER *prAdapter,
	IN enum ONE_TIME_CAL_TYPE eCalType,
	IN uint32_t u4CalParam
)
{
	uint8_t ucFragSeqNum = 0, ucBlkSeqNum = 0;
	uint8_t ucFragCtr = 0, ucBlkCtr = 0;
	uint32_t u4FragBufLen = 0, u4BlkBufLen = 0;
	struct ONE_TIME_CAL_DATA_BLK *prCmdBlk;
	struct ONE_TIME_CAL_BLK_INFO *prCmdBlkInfo;

	DBGLOG(RLM, INFO, "%s start\n", __func__);
	DBGLOG(RLM, INFO, "CalType=%u, CalParam=%u\n", eCalType, u4CalParam);

	prCmdBlk = kalMemAlloc(
		sizeof(struct ONE_TIME_CAL_DATA_BLK), VIR_MEM_TYPE);

	if (!prCmdBlk) {
		DBGLOG(RLM, ERROR, "Alloc cmd block mem fail\n");
		return WLAN_STATUS_FAILURE;
	}

	kalMemZero(prCmdBlk, sizeof(struct ONE_TIME_CAL_DATA_BLK));

	prCmdBlkInfo = &prCmdBlk->rOneTimeCalBlkInfo;

	/* Get the 1st block to check total number of blocks */
	prCmdBlkInfo->ucCalType = eCalType;
	prCmdBlkInfo->u4CalParam = u4CalParam;
	prCmdBlkInfo->ucBlkSeqNum = 0;
	prCmdBlkInfo->ucBlkFragSeqNum = 0;
	prCmdBlkInfo->u4BlkLen =
		(ONE_TIME_CAL_DATA_BLK_LEN << 16) +
		ONE_TIME_CAL_DATA_BLK_LEN;

	if (rlmOneTimeCalUpdateStruct(prAdapter, prCmdBlk, FALSE)
			!= WLAN_STATUS_SUCCESS)
		return WLAN_STATUS_FAILURE;

	ucBlkSeqNum =
		((prCmdBlkInfo->ucBlkSeqNum) & BITS(4, 7)) >> 4;

	DBGLOG(RLM, INFO,
		"Get the 1st block to get BlkNum=%u\n", ucBlkSeqNum);

	for	(ucBlkCtr = 0; ucBlkCtr < ucBlkSeqNum; ucBlkCtr++) {

		/* Get the total number of frags from 1st frag */
		prCmdBlkInfo->ucCalType = eCalType;
		prCmdBlkInfo->u4CalParam = u4CalParam;
		prCmdBlkInfo->ucBlkSeqNum =
			(ucBlkSeqNum << 4) + ucBlkCtr;
		prCmdBlkInfo->ucBlkFragSeqNum = 0;
		prCmdBlkInfo->u4BlkLen =
			(ONE_TIME_CAL_DATA_BLK_LEN << 16) +
			ONE_TIME_CAL_DATA_BLK_LEN;

		if (rlmOneTimeCalUpdateStruct(prAdapter, prCmdBlk, FALSE)
				!= WLAN_STATUS_SUCCESS)
			return WLAN_STATUS_FAILURE;

		ucFragSeqNum =
			((prCmdBlkInfo->ucBlkFragSeqNum) & BITS(4, 7)) >> 4;

		DBGLOG(RLM, INFO, "Get FragNum=%u of block %u\n",
			ucFragSeqNum, ucFragSeqNum);



		for (ucFragCtr = 0; ucFragCtr < ucFragSeqNum; ucFragCtr++) {

			kalMemZero(prCmdBlk,
				sizeof(struct ONE_TIME_CAL_DATA_BLK));

			prCmdBlkInfo->ucCalType = eCalType;
			prCmdBlkInfo->u4CalParam = u4CalParam;
			prCmdBlkInfo->ucBlkSeqNum =
				(ucBlkSeqNum << 4) + ucBlkCtr;
			prCmdBlkInfo->ucBlkFragSeqNum =
				(ucFragSeqNum << 4) + ucFragCtr;
			prCmdBlkInfo->u4BlkLen =
				(ONE_TIME_CAL_DATA_BLK_LEN << 16) +
				ONE_TIME_CAL_DATA_BLK_LEN;

			if (rlmOneTimeCalUpdateStruct(prAdapter,
				prCmdBlk, FALSE) != WLAN_STATUS_SUCCESS)
				return WLAN_STATUS_FAILURE;

			u4BlkBufLen =
				(prCmdBlkInfo->u4BlkLen & BITS(16, 31))
				>> 16;

			if (ucFragCtr == ucFragSeqNum - 1)
				u4FragBufLen = u4BlkBufLen -
					ucFragCtr * ONE_TIME_CAL_DATA_BLK_LEN;
			else
				u4FragBufLen = ONE_TIME_CAL_DATA_BLK_LEN;


			DBGLOG(RLM, INFO, "BlkBufLen=%u, FragBufLen=%u\n",
				u4BlkBufLen, u4FragBufLen);

			prCmdBlkInfo->u4BlkLen =
				(u4BlkBufLen << 16) + u4FragBufLen;

			wlanSendSetQueryCmd(prAdapter,
				CMD_ID_ONE_TIME_CAL,
				TRUE,
				FALSE,
				FALSE,
				NULL,
				NULL,
				(uint32_t)
					(sizeof(struct ONE_TIME_CAL_DATA_BLK)),
				(uint8_t *)prCmdBlk,
				NULL,
				0);

		}

	}


	if (prCmdBlk)
		kalMemFree(prCmdBlk, VIR_MEM_TYPE,
			sizeof(struct ONE_TIME_CAL_DATA_BLK));


	return WLAN_STATUS_SUCCESS;
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief Handler function for one time cal get
 *
 * \param[in] prAdapter
 *            eBand
 *            ucChannel
 *
 * \return WLAN_STATUS
 */
/*----------------------------------------------------------------------------*/
uint32_t rlmOneTimeCalGetHandler(
	IN struct ADAPTER *prAdapter,
	IN enum ONE_TIME_CAL_BAND eBand,
	IN uint8_t ucChannel
)
{

	struct ONE_TIME_CAL_DATA_BLK *prCmdBlk;
	struct ONE_TIME_CAL_BLK_INFO *prCmdBlkInfo;
	uint8_t ucGroup = 0, ucChIdx = 0;

	DBGLOG(RLM, INFO, "%s start\n", __func__);

	prCmdBlk = kalMemAlloc(
		sizeof(struct ONE_TIME_CAL_DATA_BLK), VIR_MEM_TYPE);

	if (!prCmdBlk) {
		DBGLOG(RLM, ERROR, "Alloc cmd block mem fail\n");
		return WLAN_STATUS_FAILURE;
	}

	kalMemZero(prCmdBlk, sizeof(struct ONE_TIME_CAL_DATA_BLK));

	if (rlmCh2Group(prAdapter, eBand, ucChannel, &ucGroup, &ucChIdx)
		!= WLAN_STATUS_SUCCESS)
		return WLAN_STATUS_FAILURE;

	prCmdBlkInfo = &prCmdBlk->rOneTimeCalBlkInfo;

	prCmdBlkInfo->u4CalParam = ((uint32_t)eBand << 12) + ucChannel;
	prCmdBlkInfo->ucGroup = ucGroup;

	DBGLOG(RLM, INFO, "BlkNum=0x%X, CalParam=0x%X, Group=0x%X\n",
		prCmdBlkInfo->ucBlkSeqNum,
		prCmdBlkInfo->u4CalParam,
		prCmdBlkInfo->ucGroup);

	wlanSendSetQueryCmd(prAdapter,
			CMD_ID_ONE_TIME_CAL,
			TRUE,
			FALSE,
			FALSE,
			NULL,
			NULL,
			sizeof(struct ONE_TIME_CAL_DATA_BLK),
			(uint8_t *)prCmdBlk,
			NULL,
			0);

	if (prCmdBlk) {
		kalMemFree(prCmdBlk, VIR_MEM_TYPE,
			sizeof(struct ONE_TIME_CAL_DATA_BLK));
	}


	return WLAN_STATUS_SUCCESS;
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief Handler function for one time cal set
 *
 * \param[in] prAdapter
 *            eBand
 *            ucChannel
 *
 * \return WLAN_STATUS
 */
/*----------------------------------------------------------------------------*/
uint32_t rlmOneTimeCalSetHandler(
	IN struct ADAPTER *prAdapter,
	IN enum ONE_TIME_CAL_BAND eBand,
	IN uint8_t ucChannel
)
{
	uint8_t ucGroup = 0, ucChIdx = 0;
	uint32_t u4CalParam = 0;

	DBGLOG(RLM, INFO, "%s start\n", __func__);

	if (rlmCh2Group(prAdapter, eBand, ucChannel,
			&ucGroup, &ucChIdx) != WLAN_STATUS_SUCCESS)
		return WLAN_STATUS_FAILURE;


	/* Common */
	u4CalParam = eBand;
	if (rlmSendFWOneTimeCalData(prAdapter,
		ONE_TIME_CAL_TYPE_COMMON, u4CalParam) != WLAN_STATUS_SUCCESS)
		return WLAN_STATUS_FAILURE;

	DBGLOG(RLM, INFO, "Send FW common cal data for eBend %u\n", eBand);


	/* Group */
	u4CalParam = ucGroup;
	if (rlmSendFWOneTimeCalData(prAdapter,
		ONE_TIME_CAL_TYPE_GROUP, u4CalParam) != WLAN_STATUS_SUCCESS)
		return WLAN_STATUS_FAILURE;

	DBGLOG(RLM, INFO, "Send FW group cal data for group %u\n", ucGroup);


	/* Channel */
	u4CalParam = (eBand << 12) + ucChannel;
	if (rlmSendFWOneTimeCalData(prAdapter,
		ONE_TIME_CAL_TYPE_CHANNEL, u4CalParam) != WLAN_STATUS_SUCCESS)
		return WLAN_STATUS_FAILURE;

	DBGLOG(RLM, INFO, "Send FW channel cal data for eBend %u ch %u\n",
		eBand, ucChannel);


	return WLAN_STATUS_SUCCESS;
}
#endif

#if CFG_SUPPORT_CAL_RESULT_BACKUP_TO_HOST
uint32_t rlmCalBackup(struct ADAPTER *prAdapter, uint8_t ucReason,
		      uint8_t ucAction, uint8_t ucRomRam)
{
	uint32_t rStatus = WLAN_STATUS_FAILURE;
	struct GLUE_INFO *prGlueInfo = NULL;
	struct PARAM_CAL_BACKUP_STRUCT_V2 rCalBackupDataV2;
	uint32_t u4BufLen = 0;

	if (!prAdapter) {
		DBGLOG(NAN, ERROR, "prAdapter error!\n");
		return rStatus;
	}
	if (!prAdapter->prGlueInfo) {
		DBGLOG(NAN, ERROR, "prAdapter->prGlueInfo error!\n");
		return rStatus;
	}

	prGlueInfo = prAdapter->prGlueInfo;

	rCalBackupDataV2.ucReason = ucReason;
	rCalBackupDataV2.ucAction = ucAction;
	rCalBackupDataV2.ucNeedResp = 1;
	rCalBackupDataV2.ucFragNum = 0;
	rCalBackupDataV2.ucRomRam = ucRomRam;
	rCalBackupDataV2.u4ThermalValue = 0;
	rCalBackupDataV2.u4Address = 0;
	rCalBackupDataV2.u4Length = 0;
	rCalBackupDataV2.u4RemainLength = 0;

	if (ucReason == 0 && ucAction == 0) {
		DBGLOG(RFTEST, INFO, "RLM CMD : Get Thermal Temp from FW.\n");
		/* Step 1 : Get Thermal Temp from FW */

		rStatus = kalIoctl(prGlueInfo, wlanoidQueryCalBackupV2,
				   &rCalBackupDataV2,
				   sizeof(struct PARAM_CAL_BACKUP_STRUCT_V2),
				   TRUE, TRUE, TRUE, &u4BufLen);

		if (rStatus != WLAN_STATUS_SUCCESS) {
			DBGLOG(RFTEST, INFO,
			       "RLM CMD : Get Thermal Temp from FW Return Fail (0x%08x)!!!!!!!!!!!\n",
			       rStatus);
			return rStatus;
		}

		DBGLOG(RFTEST, INFO,
		       "CMD : Get Thermal Temp (%d) from FW. Finish!!!!!!!!!!!\n",
		       rCalBackupDataV2.u4ThermalValue);
	} else if (ucReason == 1 && ucAction == 2) {
		DBGLOG(RFTEST, INFO, "RLM CMD : Trigger FW Do All Cal.\n");
		/* Step 2 : Trigger All Cal Function */

		rStatus = kalIoctl(prGlueInfo, wlanoidSetCalBackup,
				   &rCalBackupDataV2,
				   sizeof(struct PARAM_CAL_BACKUP_STRUCT_V2),
				   FALSE, FALSE, TRUE, &u4BufLen);

		if (rStatus != WLAN_STATUS_SUCCESS) {
			DBGLOG(RFTEST, INFO,
			       "RLM CMD : Trigger FW Do All Cal Return Fail (0x%08x)!!!!!!!!!!!\n",
			       rStatus);
			return rStatus;
		}

		DBGLOG(RFTEST, INFO,
		       "CMD : Trigger FW Do All Cal. Finish!!!!!!!!!!!\n");
	} else if (ucReason == 0 && ucAction == 1) {
		DBGLOG(RFTEST, INFO,
		       "RLM CMD : Get Cal Data (%s) Size from FW.\n",
		       ucRomRam == 0 ? "ROM" : "RAM");
		/* Step 3 : Get Cal Data Size from FW */

		rStatus = kalIoctl(prGlueInfo, wlanoidQueryCalBackupV2,
				   &rCalBackupDataV2,
				   sizeof(struct PARAM_CAL_BACKUP_STRUCT_V2),
				   TRUE, TRUE, TRUE, &u4BufLen);

		if (rStatus != WLAN_STATUS_SUCCESS) {
			DBGLOG(RFTEST, INFO,
			       "RLM CMD : Get Cal Data (%s) Size from FW Return Fail (0x%08x)!!!!!!!!!!!\n",
			       ucRomRam == 0 ? "ROM" : "RAM", rStatus);
			return rStatus;
		}

		DBGLOG(RFTEST, INFO,
		       "CMD : Get Cal Data (%s) Size from FW. Finish!!!!!!!!!!!\n",
		       ucRomRam == 0 ? "ROM" : "RAM");
	} else if (ucReason == 2 && ucAction == 4) {
		DBGLOG(RFTEST, INFO,
		       "RLM CMD : Get Cal Data from FW (%s). Start!!!!!!!!!!!!!!!!\n",
		       ucRomRam == 0 ? "ROM" : "RAM");
		DBGLOG(RFTEST, INFO, "Thermal Temp = %d\n",
		       g_rBackupCalDataAllV2.u4ThermalInfo);
		/* Step 4 : Get Cal Data from FW */

		rStatus = kalIoctl(prGlueInfo, wlanoidQueryCalBackupV2,
				   &rCalBackupDataV2,
				   sizeof(struct PARAM_CAL_BACKUP_STRUCT_V2),
				   TRUE, TRUE, TRUE, &u4BufLen);
		if (rStatus != WLAN_STATUS_SUCCESS) {
			DBGLOG(RFTEST, INFO,
			       "RLM CMD : Get Cal Data (%s) Size from FW Return Fail (0x%08x)!!!!!!!!!!!\n",
			       ucRomRam == 0 ? "ROM" : "RAM", rStatus);
			return rStatus;
		}

		DBGLOG(RFTEST, INFO,
		       "CMD : Get Cal Data from FW (%s). Finish!!!!!!!!!!!\n",
		       ucRomRam == 0 ? "ROM" : "RAM");

		if (ucRomRam == 0) {
			DBGLOG(RFTEST, INFO,
			       "Check some of elements (0x%08x), (0x%08x), (0x%08x), (0x%08x), (0x%08x)\n",
			       g_rBackupCalDataAllV2.au4RomCalData[670],
			       g_rBackupCalDataAllV2.au4RomCalData[671],
			       g_rBackupCalDataAllV2.au4RomCalData[672],
			       g_rBackupCalDataAllV2.au4RomCalData[673],
			       g_rBackupCalDataAllV2.au4RomCalData[674]);
			DBGLOG(RFTEST, INFO,
			       "Check some of elements (0x%08x), (0x%08x), (0x%08x), (0x%08x), (0x%08x)\n",
			       g_rBackupCalDataAllV2.au4RomCalData[675],
			       g_rBackupCalDataAllV2.au4RomCalData[676],
			       g_rBackupCalDataAllV2.au4RomCalData[677],
			       g_rBackupCalDataAllV2.au4RomCalData[678],
			       g_rBackupCalDataAllV2.au4RomCalData[679]);
		}
	} else if (ucReason == 4 && ucAction == 6) {
		DBGLOG(RFTEST, INFO, "RLM CMD : Print Cal Data in FW (%s).\n",
		       ucRomRam == 0 ? "ROM" : "RAM");
		/* Debug Use : Print Cal Data in FW */

		rStatus = kalIoctl(prGlueInfo, wlanoidSetCalBackup,
				   &rCalBackupDataV2,
				   sizeof(struct PARAM_CAL_BACKUP_STRUCT_V2),
				   TRUE, TRUE, TRUE, &u4BufLen);

		if (rStatus != WLAN_STATUS_SUCCESS) {
			DBGLOG(RFTEST, INFO,
			       "RLM CMD : Print Cal Data in FW (%s) Return Fail (0x%08x)!!!!!!!!!!!\n",
			       ucRomRam == 0 ? "ROM" : "RAM", rStatus);
			return rStatus;
		}

		DBGLOG(RFTEST, INFO,
		       "CMD : Print Cal Data in FW (%s). Finish!!!!!!!!!!!\n",
		       ucRomRam == 0 ? "ROM" : "RAM");
	} else if (ucReason == 3 && ucAction == 5) {
		DBGLOG(RFTEST, INFO, "RLM CMD : Send Cal Data to FW (%s).\n",
		       ucRomRam == 0 ? "ROM" : "RAM");
		/* Send Cal Data to FW */

		rStatus = kalIoctl(prGlueInfo, wlanoidSetCalBackup,
				   &rCalBackupDataV2,
				   sizeof(struct PARAM_CAL_BACKUP_STRUCT_V2),
				   TRUE, TRUE, TRUE, &u4BufLen);

		if (rStatus != WLAN_STATUS_SUCCESS) {
			DBGLOG(RFTEST, INFO,
			       "RLM CMD : Send Cal Data to FW (%s) Return Fail (0x%08x)!!!!!!!!!!!\n",
			       ucRomRam == 0 ? "ROM" : "RAM", rStatus);
			return rStatus;
		}

		DBGLOG(RFTEST, INFO,
		       "CMD : Send Cal Data to FW (%s). Finish!!!!!!!!!!!\n",
		       ucRomRam == 0 ? "ROM" : "RAM");
	} else {
		DBGLOG(RFTEST, INFO,
		       "CMD : Undefined Reason (%d) and Action (%d) for Cal Backup in Host Side!\n",
		       ucReason, ucAction);

		return rStatus;
	}

	return rStatus;
}

uint32_t rlmTriggerCalBackup(struct ADAPTER *prAdapter,
			     u_int8_t fgIsCalDataBackuped)
{
	uint32_t rStatus = WLAN_STATUS_SUCCESS;

	if (!fgIsCalDataBackuped) {
		DBGLOG(RFTEST, INFO,
		       "======== Boot Time Wi-Fi Enable........\n");
		DBGLOG(RFTEST, INFO,
		       "Step 0 : Reset All Cal Data in Driver.\n");
		memset(&g_rBackupCalDataAllV2, 1,
		       sizeof(struct RLM_CAL_RESULT_ALL_V2));
		g_rBackupCalDataAllV2.u4MagicNum1 = 6632;
		g_rBackupCalDataAllV2.u4MagicNum2 = 6632;

		DBGLOG(RFTEST, INFO, "Step 1 : Get Thermal Temp from FW.\n");
		if (rlmCalBackup(prAdapter, 0, 0, 0) == WLAN_STATUS_FAILURE) {
			DBGLOG(RFTEST, INFO, "Step 1 : Return Failure.\n");
			return WLAN_STATUS_FAILURE;
		}

		DBGLOG(RFTEST, INFO,
		       "Step 2 : Get Rom Cal Data Size from FW.\n");
		if (rlmCalBackup(prAdapter, 0, 1, 0) == WLAN_STATUS_FAILURE) {
			DBGLOG(RFTEST, INFO, "Step 2 : Return Failure.\n");
			return WLAN_STATUS_FAILURE;
		}

		DBGLOG(RFTEST, INFO,
		       "Step 3 : Get Ram Cal Data Size from FW.\n");
		if (rlmCalBackup(prAdapter, 0, 1, 1) == WLAN_STATUS_FAILURE) {
			DBGLOG(RFTEST, INFO, "Step 3 : Return Failure.\n");
			return WLAN_STATUS_FAILURE;
		}

		DBGLOG(RFTEST, INFO, "Step 4 : Trigger FW Do Full Cal.\n");
		if (rlmCalBackup(prAdapter, 1, 2, 0) == WLAN_STATUS_FAILURE) {
			DBGLOG(RFTEST, INFO, "Step 4 : Return Failure.\n");
			return WLAN_STATUS_FAILURE;
		}
	} else {
		DBGLOG(RFTEST, INFO, "======== Normal Wi-Fi Enable........\n");
		DBGLOG(RFTEST, INFO, "Step 0 : Sent Rom Cal data to FW.\n");
		if (rlmCalBackup(prAdapter, 3, 5, 0) == WLAN_STATUS_FAILURE) {
			DBGLOG(RFTEST, INFO, "Step 0 : Return Failure.\n");
			return WLAN_STATUS_FAILURE;
		}

		DBGLOG(RFTEST, INFO, "Step 1 : Sent Ram Cal data to FW.\n");
		if (rlmCalBackup(prAdapter, 3, 5, 1) == WLAN_STATUS_FAILURE) {
			DBGLOG(RFTEST, INFO, "Step 1 : Return Failure.\n");
			return WLAN_STATUS_FAILURE;
		}
	}

	return rStatus;
}
#endif

void rlmModifyVhtBwPara(uint8_t *pucVhtChannelFrequencyS1,
			uint8_t *pucVhtChannelFrequencyS2,
			uint8_t *pucVhtChannelWidth)
{
	uint8_t i = 0, ucTempS = 0;

	if ((*pucVhtChannelFrequencyS1 != 0) &&
	    (*pucVhtChannelFrequencyS2 != 0)) {

		uint8_t ucBW160Inteval = 8;

		if (((*pucVhtChannelFrequencyS2 - *pucVhtChannelFrequencyS1) ==
		     ucBW160Inteval) ||
		    ((*pucVhtChannelFrequencyS1 - *pucVhtChannelFrequencyS2) ==
		     ucBW160Inteval)) {
			/*C160 case*/

			/* NEW spec should set central ch of bw80 at S1,
			 * set central ch of bw160 at S2
			 */
			for (i = 0; i < 2; i++) {

				if (i == 0)
					ucTempS = *pucVhtChannelFrequencyS1;
				else
					ucTempS = *pucVhtChannelFrequencyS2;

				if ((ucTempS == 50) || (ucTempS == 82)
				   || (ucTempS == 114) || (ucTempS == 163)
#if (CFG_SUPPORT_WIFI_6G == 1)
				   || (ucTempS == 15) || (ucTempS == 47)
				   || (ucTempS == 79) || (ucTempS == 111)
				   || (ucTempS == 143) || (ucTempS == 175)
				   || (ucTempS == 207)
#endif
				   )
					break;
			}

			if (ucTempS == 0) {
				DBGLOG(RLM, WARN,
				       "please check BW160 setting, find central freq fail\n");
				return;
			}

			*pucVhtChannelFrequencyS1 = ucTempS;
			*pucVhtChannelFrequencyS2 = 0;
			*pucVhtChannelWidth = CW_160MHZ;
		} else {
			/*real 80P80 case*/
		}
	}
}

#if (CFG_SUPPORT_WIFI_6G == 1)
void rlmTransferHe6gOpInfor(IN uint8_t ucChannelNum,
	IN uint8_t ucChannelWidth,
	OUT uint8_t *pucChannelWidth,
	OUT uint8_t *pucCenterFreqS1,
	OUT uint8_t *pucCenterFreqS2,
	OUT enum ENUM_CHNL_EXT *peSco)
{
	int8_t cDelta;
	enum ENUM_CHNL_EXT eAndOneSCO;

	if (ucChannelWidth == HE_OP_CHANNEL_WIDTH_20)
		*pucChannelWidth = (uint8_t)CW_20_40MHZ;
	else if (ucChannelWidth == HE_OP_CHANNEL_WIDTH_40) {
		*pucChannelWidth = (uint8_t)CW_20_40MHZ;
		if ((ucChannelNum + 2) ==
			*pucCenterFreqS1)
			*peSco = CHNL_EXT_SCA;
		else
			*peSco = CHNL_EXT_SCB;
	} else if (ucChannelWidth == HE_OP_CHANNEL_WIDTH_80)
		*pucChannelWidth = (uint8_t)CW_80MHZ;
	else if (
		((*pucCenterFreqS1 - *pucCenterFreqS2) == 8) ||
		((*pucCenterFreqS2 - *pucCenterFreqS1) == 8))
		*pucChannelWidth = (uint8_t)CW_160MHZ;
	else
		*pucChannelWidth = (uint8_t)CW_80P80MHZ;

	/*add IEEE BW160 patch*/
	rlmModifyHE6GBwPara(pucCenterFreqS1,
			pucCenterFreqS2,
			pucChannelWidth);

	/* Calculate SCO */
	if (*pucChannelWidth == CW_80MHZ ||
		*pucChannelWidth == CW_160MHZ) {
		/* P: PriChnl
		 * A: CHNL_EXT_SCA
		 * B: CHNL_EXT_SCB
		 */
		/* --|----|--CenterFreqS1--|----|-- */
		/* --|----|--CenterFreqS1--B----P-- */
		/* --|----|--CenterFreqS1--P----A-- */
		cDelta = ucChannelNum - *pucCenterFreqS1;
		eAndOneSCO = CHNL_EXT_SCB;
		*peSco = CHNL_EXT_SCA;
		if (cDelta < 0) {
			/* --|----|--CenterFreqS1--|----|-- */
			/* --P----A--CenterFreqS1--|----|-- */
			/* --B----P--CenterFreqS1--|----|-- */
			eAndOneSCO = CHNL_EXT_SCA;
			*peSco = CHNL_EXT_SCB;
			cDelta = -cDelta;
		}
		cDelta = cDelta - 2;
		if ((cDelta/4) & 1)
			*peSco = eAndOneSCO;
	}
}

void rlmModifyHE6GBwPara(uint8_t *pucHe6gChannelFrequencyS1,
			uint8_t *pucHe6gChannelFrequencyS2,
			uint8_t *pucHe6gChannelWidth)
{

	uint8_t i = 0, ucTempS = 0;

	if ((*pucHe6gChannelFrequencyS1 != 0) &&
		(*pucHe6gChannelFrequencyS2 != 0)) {

		uint8_t ucBW160Inteval = 8;

		if (((*pucHe6gChannelFrequencyS2 - *pucHe6gChannelFrequencyS1)
		== ucBW160Inteval) ||
		((*pucHe6gChannelFrequencyS1 - *pucHe6gChannelFrequencyS2)
		== ucBW160Inteval)) {
			/*C160 case*/

			/* NEW spec should set central ch of bw80 at S1,
			* set central ch of bw160 at S2
			*/
			for (i = 0; i < 2; i++) {
				if (i == 0)
					ucTempS = *pucHe6gChannelFrequencyS1;
				else
					ucTempS = *pucHe6gChannelFrequencyS2;
				if ((ucTempS == 15) || (ucTempS == 47) ||
					(ucTempS == 79) || (ucTempS == 111) ||
					(ucTempS == 143) || (ucTempS == 175) ||
					(ucTempS == 207))
					break;
			}

			if (ucTempS == 0) {
				DBGLOG(RLM, WARN,
					"@wifi 6g,please check BW160 setting, find central freq fail\n");
				return;
			}

			*pucHe6gChannelFrequencyS1 = ucTempS;
			*pucHe6gChannelFrequencyS2 = 0;
			*pucHe6gChannelWidth = CW_160MHZ;
		} else {
			/*real 80P80 case*/
		}
	}
}
#endif

static void rlmRevisePreferBandwidthNss(struct ADAPTER *prAdapter,
					uint8_t ucBssIndex,
					struct STA_RECORD *prStaRec)
{
	enum ENUM_CHANNEL_WIDTH eChannelWidth = CW_20_40MHZ;
	struct BSS_INFO *prBssInfo;

#define VHT_MCS_TX_RX_MAX_2SS BITS(2, 3)
#define VHT_MCS_TX_RX_MAX_2SS_SHIFT 2

#define AR_STA_2AC_MCS(prStaRec)                                               \
	(((prStaRec)->u2VhtRxMcsMap & VHT_MCS_TX_RX_MAX_2SS) >>                \
	 VHT_MCS_TX_RX_MAX_2SS_SHIFT)

#define AR_IS_STA_2SS_AC(prStaRec) ((AR_STA_2AC_MCS(prStaRec) != BITS(0, 1)))

	prBssInfo = GET_BSS_INFO_BY_INDEX(prAdapter, ucBssIndex);

	eChannelWidth = prBssInfo->ucVhtChannelWidth;

	/*
	 * Prefer setting modification
	 * 80+80 1x1 and 80 2x2 have the same phy rate, choose the 80 2x2
	 */

	if (AR_IS_STA_2SS_AC(prStaRec)) {
		/*
		 * DBGLOG(RLM, WARN, "support 2ss\n");
		 */

		if ((eChannelWidth == CW_80P80MHZ &&
		     prBssInfo->ucVhtChannelFrequencyS2 != 0)) {
			DBGLOG(RLM, WARN, "support (2Nss) and (80+80)\n");
			DBGLOG(RLM, WARN,
			       "choose (2Nss) and (80) for Bss_info\n");
			prBssInfo->ucVhtChannelWidth = CW_80MHZ;
			prBssInfo->ucVhtChannelFrequencyS2 = 0;
		}
	}
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief Revise operating BW by own maximum bandwidth capability
 *
 * \param[in]
 *
 * \return none
 */
/*----------------------------------------------------------------------------*/
void rlmReviseMaxBw(struct ADAPTER *prAdapter, uint8_t ucBssIndex,
		    enum ENUM_CHNL_EXT *peExtend,
		    enum ENUM_CHANNEL_WIDTH *peChannelWidth, uint8_t *pucS1,
		    uint8_t *pucPrimaryCh)
{
	uint8_t ucMaxBandwidth = MAX_BW_80MHZ;
	uint8_t ucCurrentBandwidth = MAX_BW_20MHZ;
	uint8_t ucOffset = (MAX_BW_80MHZ - CW_80MHZ);

#if (CFG_SUPPORT_WIFI_6G == 1)
	struct BSS_INFO *prBssInfo;

	prBssInfo = GET_BSS_INFO_BY_INDEX(prAdapter,
						ucBssIndex);
#endif

#if CFG_SUPPORT_DBDC
	ucMaxBandwidth = cnmGetDbdcBwCapability(prAdapter, ucBssIndex);
#else
	ucMaxBandwidth = cnmGetBssMaxBw(prAdapter, ucBssIndex);
#endif
	if (*peChannelWidth > CW_20_40MHZ) {
		/*case BW > 80 , 160 80P80 */
		ucCurrentBandwidth = (uint8_t)*peChannelWidth + ucOffset;
	} else {
		/*case BW20 BW40 */
		if (*peExtend != CHNL_EXT_SCN) {
			/*case BW40 */
			ucCurrentBandwidth = MAX_BW_40MHZ;
		}
	}

	if (ucCurrentBandwidth > ucMaxBandwidth) {
		DBGLOG(RLM, INFO, "Decreasse the BW to (%d)\n", ucMaxBandwidth);

		if (ucMaxBandwidth <= MAX_BW_40MHZ) {
			/*BW20 * BW40*/
			*peChannelWidth = CW_20_40MHZ;

			if (ucMaxBandwidth == MAX_BW_20MHZ)
				*peExtend = CHNL_EXT_SCN;
		} else {
			/* BW80, BW160, BW80P80
			 * ucMaxBandwidth Must be
			 * MAX_BW_80MHZ,MAX_BW_160MHZ,MAX_BW_80MHZ
			 * peExtend should not change
			 */
			*peChannelWidth = (ucMaxBandwidth - ucOffset);

			if (ucMaxBandwidth == MAX_BW_80MHZ) {
				/* modify S1 for Bandwidth 160 downgrade 80 case
				 */
				if (ucCurrentBandwidth == MAX_BW_160MHZ) {
#if (CFG_SUPPORT_WIFI_6G == 1)
					if (prBssInfo->eBand == BAND_6G) {
					    if ((*pucPrimaryCh >= 1) &&
						     (*pucPrimaryCh <= 13))
						    *pucS1 = 7;
					    else if ((*pucPrimaryCh >= 17) &&
						     (*pucPrimaryCh <= 29))
						    *pucS1 = 23;
					    else if ((*pucPrimaryCh >= 33) &&
						     (*pucPrimaryCh <= 45))
						    *pucS1 = 39;
					    else if ((*pucPrimaryCh >= 49) &&
						     (*pucPrimaryCh <= 61))
						    *pucS1 = 55;
					    else if ((*pucPrimaryCh >= 65) &&
						     (*pucPrimaryCh <= 77))
						    *pucS1 = 71;
					    else if ((*pucPrimaryCh >= 81) &&
						     (*pucPrimaryCh <= 93))
						    *pucS1 = 87;
					    else if ((*pucPrimaryCh >= 97) &&
						     (*pucPrimaryCh <= 109))
						    *pucS1 = 103;
					    else if ((*pucPrimaryCh >= 113) &&
						     (*pucPrimaryCh <= 125))
						    *pucS1 = 119;
					    else if ((*pucPrimaryCh >= 129) &&
						     (*pucPrimaryCh <= 141))
						    *pucS1 = 135;
					    else if ((*pucPrimaryCh >= 145) &&
						     (*pucPrimaryCh <= 157))
						    *pucS1 = 151;
					    else if ((*pucPrimaryCh >= 161) &&
						     (*pucPrimaryCh <= 173))
						    *pucS1 = 167;
					    else if ((*pucPrimaryCh >= 177) &&
						     (*pucPrimaryCh <= 189))
						    *pucS1 = 183;
					    else if ((*pucPrimaryCh >= 193) &&
						     (*pucPrimaryCh <= 205))
						    *pucS1 = 199;
					    else if ((*pucPrimaryCh >= 209) &&
						     (*pucPrimaryCh <= 221))
						    *pucS1 = 215;
					    else
						    DBGLOG(RLM, ERROR,
						       "Check connect 160 downgrde (%d) case\n",
						       ucMaxBandwidth);

					    DBGLOG(RLM, ERROR,
						    "Decreasse the BW160 to BW80, shift S1 to (%d)\n",
						    *pucS1);
					} else
#endif
					{
					    if ((*pucPrimaryCh >= 36) &&
						     (*pucPrimaryCh <= 48))
						    *pucS1 = 42;
					    else if ((*pucPrimaryCh >= 52) &&
						     (*pucPrimaryCh <= 64))
						    *pucS1 = 58;
					    else if ((*pucPrimaryCh >= 100) &&
						     (*pucPrimaryCh <= 112))
						    *pucS1 = 106;
					    else if ((*pucPrimaryCh >= 116) &&
						     (*pucPrimaryCh <= 128))
						    *pucS1 = 122;
					    else if ((*pucPrimaryCh >= 132) &&
						     (*pucPrimaryCh <= 144))
						    /* 160 downgrade should not in
						     * this case
						     */
						    *pucS1 = 138;
					    else if ((*pucPrimaryCh >= 149) &&
						     (*pucPrimaryCh <= 161))
						    /* 160 downgrade should not in
						     * this case
						     */
						    *pucS1 = 155;
					    else
						    DBGLOG(RLM, INFO,
						       "Check connect 160 downgrde (%d) case\n",
						       ucMaxBandwidth);

					    DBGLOG(RLM, INFO,
						    "Decreasse the BW160 to BW80, shift S1 to (%d)\n",
						    *pucS1);
					}
				}
			}
		}

		DBGLOG(RLM, INFO, "Modify ChannelWidth (%d) and Extend (%d)\n",
		       *peChannelWidth, *peExtend);
	}
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief Fill VHT Operation Information(VHT BW, S1, S2) by BSS operating
 *  channel width
 *
 * \param[in]
 *
 * \return none
 */
/*----------------------------------------------------------------------------*/
void rlmFillVhtOpInfoByBssOpBw(struct BSS_INFO *prBssInfo, uint8_t ucBssOpBw)
{
	ASSERT(prBssInfo);

	if (ucBssOpBw < MAX_BW_80MHZ || prBssInfo->eBand == BAND_2G4) {
		prBssInfo->ucVhtChannelWidth = VHT_OP_CHANNEL_WIDTH_20_40;
		prBssInfo->ucVhtChannelFrequencyS1 = 0;
		prBssInfo->ucVhtChannelFrequencyS2 = 0;
	} else if (ucBssOpBw == MAX_BW_80MHZ) {
		prBssInfo->ucVhtChannelWidth = VHT_OP_CHANNEL_WIDTH_80;
		prBssInfo->ucVhtChannelFrequencyS1 = nicGetVhtS1(
			prBssInfo->ucPrimaryChannel, VHT_OP_CHANNEL_WIDTH_80);
		prBssInfo->ucVhtChannelFrequencyS2 = 0;
	} else if (ucBssOpBw == MAX_BW_160MHZ) {
		prBssInfo->ucVhtChannelWidth = VHT_OP_CHANNEL_WIDTH_160;
		prBssInfo->ucVhtChannelFrequencyS1 = nicGetVhtS1(
			prBssInfo->ucPrimaryChannel, VHT_OP_CHANNEL_WIDTH_160);
		prBssInfo->ucVhtChannelFrequencyS2 = 0;
	} else {
		/* 4 TODO: / BW80+80 support */
		DBGLOG(RLM, INFO, "Unsupport BW setting, back to VHT20_40\n");

		prBssInfo->ucVhtChannelWidth = VHT_OP_CHANNEL_WIDTH_20_40;
		prBssInfo->ucVhtChannelFrequencyS1 = 0;
		prBssInfo->ucVhtChannelFrequencyS2 = 0;
	}
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief This function should be invoked to update parameters of associated AP.
 *        (Association response and Beacon)
 *
 * \param[in]
 *
 * \return none
 */
/*----------------------------------------------------------------------------*/
static uint8_t rlmRecIeInfoForClient(struct ADAPTER *prAdapter,
				     struct BSS_INFO *prBssInfo, uint8_t *pucIE,
				     uint16_t u2IELength)
{
	uint16_t u2Offset;
	struct STA_RECORD *prStaRec;
	struct IE_HT_CAP *prHtCap;
	struct IE_HT_OP *prHtOp;
	struct IE_OBSS_SCAN_PARAM *prObssScnParam;
	uint8_t ucERP, ucPrimaryChannel;
	struct WIFI_VAR *prWifiVar = &prAdapter->rWifiVar;
#if CFG_SUPPORT_QUIET && 0
	u_int8_t fgHasQuietIE = FALSE;
#endif
	u_int8_t IsfgHtCapChange = FALSE;

#if CFG_SUPPORT_802_11AC
	struct IE_VHT_OP *prVhtOp;
	struct IE_VHT_CAP *prVhtCap = NULL;
	struct IE_OP_MODE_NOTIFICATION *prOPNotif;
	uint8_t fgHasOPModeIE = FALSE;
	uint8_t fgHasNewOPModeIE = FALSE;
	uint8_t ucVhtOpModeChannelWidth = 0;
	uint8_t ucVhtOpModeRxNss = 0;
	uint8_t ucMaxBwAllowed;
	uint8_t ucInitVhtOpMode = 0;
#endif

#if (CFG_SUPPORT_WIFI_6G == 1)
	struct _IE_HE_6G_BAND_CAP_T *prHe6gBandCap = NULL;
	struct _6G_OPER_INFOR_T *pr6gOperInfor = NULL;
	uint32_t u4Offset;
	uint8_t IsfgHe6gBandCapChange = FALSE;
#endif

#if CFG_SUPPORT_DFS
	u_int8_t fgHasWideBandIE = FALSE;
	u_int8_t fgHasSCOIE = FALSE;
	u_int8_t fgHasChannelSwitchIE = FALSE;
	u_int8_t fgNeedSwitchChannel = FALSE;
	uint8_t ucChannelAnnouncePri;
	enum ENUM_CHNL_EXT eChannelAnnounceSco;
	uint8_t ucChannelAnnounceChannelS1 = 0;
	uint8_t ucChannelAnnounceChannelS2 = 0;
	uint8_t ucChannelAnnounceVhtBw;
	struct IE_CHANNEL_SWITCH *prChannelSwitchAnnounceIE;
	struct IE_SECONDARY_OFFSET *prSecondaryOffsetIE;
	struct IE_WIDE_BAND_CHANNEL *prWideBandChannelIE;
#if CFG_DFS_NEWCH_DFS_FORCE_DISCONNECT
	struct ieee80211_channel *Channel = NULL;
#endif
#endif
	uint8_t *pucDumpIE;
#if (CFG_SUPPORT_TWT_HOTSPOT_AC == 1)
	uint8_t aucMtkOui[] = VENDOR_OUI_MTK;
#endif
	uint8_t ucNss;
	uint8_t ucNssFinal;
	uint8_t fgDomainValid = FALSE;
	enum ENUM_CHANNEL_WIDTH eChannelWidth = CW_20_40MHZ;

	ASSERT(prAdapter);
	ASSERT(prBssInfo);
	ASSERT(pucIE);

	prStaRec = prBssInfo->prStaRecOfAP;
	if (!prStaRec)
		return 0;

	prBssInfo->fgUseShortPreamble = prBssInfo->fgIsShortPreambleAllowed;
	ucPrimaryChannel = 0;
	prObssScnParam = NULL;
	ucMaxBwAllowed = cnmGetBssMaxBw(prAdapter, prBssInfo->ucBssIndex);
	pucDumpIE = pucIE;
	ucNss = wlanGetSupportNss(prAdapter, prBssInfo->ucBssIndex);

	/* Note: HT-related members in staRec may not be zero before, so
	 *       if following IE does not exist, they are still not zero.
	 *       These HT-related parameters are valid only when the
	 * corresponding
	 *       BssInfo supports 802.11n, i.e., RLM_NET_IS_11N()
	 */
	IE_FOR_EACH(pucIE, u2IELength, u2Offset)
	{
		switch (IE_ID(pucIE)) {
		case ELEM_ID_HT_CAP:
#if (CFG_SUPPORT_WIFI_6G == 1)
			if (prBssInfo->eBand == BAND_6G) {
				DBGLOG(SCN, WARN, "Ignore HT CAP IE at 6G\n");
				break;
			}
#endif
			if (!RLM_NET_IS_11N(prBssInfo) ||
			    IE_LEN(pucIE) != (sizeof(struct IE_HT_CAP) - 2))
				break;
			prHtCap = (struct IE_HT_CAP *)pucIE;
			prStaRec->ucMcsSet =
				prHtCap->rSupMcsSet.aucRxMcsBitmask[0];
			prStaRec->fgSupMcs32 =
				(prHtCap->rSupMcsSet.aucRxMcsBitmask[32 / 8] &
				 BIT(0))
					? TRUE
					: FALSE;

			kalMemCopy(
				prStaRec->aucRxMcsBitmask,
				prHtCap->rSupMcsSet.aucRxMcsBitmask,
				/*SUP_MCS_RX_BITMASK_OCTET_NUM */
				sizeof(prStaRec->aucRxMcsBitmask));

			prStaRec->u2RxHighestSupportedRate =
				prHtCap->rSupMcsSet.u2RxHighestSupportedRate;
			prStaRec->u4TxRateInfo =
				prHtCap->rSupMcsSet.u4TxRateInfo;

			if ((prStaRec->u2HtCapInfo &
			     HT_CAP_INFO_SM_POWER_SAVE) !=
			    (prHtCap->u2HtCapInfo & HT_CAP_INFO_SM_POWER_SAVE))
				/* Purpose : To detect SMPS change */
				IsfgHtCapChange = TRUE;

			prStaRec->u2HtCapInfo = prHtCap->u2HtCapInfo;
			/* Set LDPC Tx capability */
			if (IS_FEATURE_FORCE_ENABLED(prWifiVar->ucTxLdpc))
				prStaRec->u2HtCapInfo |= HT_CAP_INFO_LDPC_CAP;
			else if (IS_FEATURE_DISABLED(prWifiVar->ucTxLdpc))
				prStaRec->u2HtCapInfo &= ~HT_CAP_INFO_LDPC_CAP;

			/* Set STBC Tx capability */
			if (IS_FEATURE_FORCE_ENABLED(prWifiVar->ucTxStbc))
				prStaRec->u2HtCapInfo |= HT_CAP_INFO_RX_STBC;
			else if (IS_FEATURE_DISABLED(prWifiVar->ucTxStbc))
				prStaRec->u2HtCapInfo &= ~HT_CAP_INFO_RX_STBC;

			/* Set Short GI Tx capability */
			if (IS_FEATURE_FORCE_ENABLED(prWifiVar->ucTxShortGI)) {
				prStaRec->u2HtCapInfo |=
					HT_CAP_INFO_SHORT_GI_20M;
				prStaRec->u2HtCapInfo |=
					HT_CAP_INFO_SHORT_GI_40M;
			} else if (IS_FEATURE_DISABLED(
					   prWifiVar->ucTxShortGI)) {
				prStaRec->u2HtCapInfo &=
					~HT_CAP_INFO_SHORT_GI_20M;
				prStaRec->u2HtCapInfo &=
					~HT_CAP_INFO_SHORT_GI_40M;
			}

			/* Set HT Greenfield Tx capability */
			if (IS_FEATURE_FORCE_ENABLED(prWifiVar->ucTxGf))
				prStaRec->u2HtCapInfo |= HT_CAP_INFO_HT_GF;
			else if (IS_FEATURE_DISABLED(prWifiVar->ucTxGf))
				prStaRec->u2HtCapInfo &= ~HT_CAP_INFO_HT_GF;

			prStaRec->ucAmpduParam = prHtCap->ucAmpduParam;
			prStaRec->u2HtExtendedCap = prHtCap->u2HtExtendedCap;
			prStaRec->u4TxBeamformingCap =
				prHtCap->u4TxBeamformingCap;
			prStaRec->ucAselCap = prHtCap->ucAselCap;
			break;

		case ELEM_ID_HT_OP:
#if (CFG_SUPPORT_WIFI_6G == 1)
			if (prBssInfo->eBand == BAND_6G) {
				DBGLOG(SCN, WARN, "Ignore HT OP IE at 6G\n");
				break;
			}
#endif

			if (!RLM_NET_IS_11N(prBssInfo) ||
			    IE_LEN(pucIE) != (sizeof(struct IE_HT_OP) - 2))
				break;
			prHtOp = (struct IE_HT_OP *)pucIE;
			/* Workaround that some APs fill primary channel field
			 * by its
			 * secondary channel, but its DS IE is correct 20110610
			 */
			if (ucPrimaryChannel == 0)
				ucPrimaryChannel = prHtOp->ucPrimaryChannel;
			prBssInfo->ucHtOpInfo1 = prHtOp->ucInfo1;
			prBssInfo->u2HtOpInfo2 = prHtOp->u2Info2;
			prBssInfo->u2HtOpInfo3 = prHtOp->u2Info3;

			/*Backup peer HT OP Info*/
			prStaRec->ucHtPeerOpInfo1 = prHtOp->ucInfo1;

			if (!prBssInfo->fg40mBwAllowed)
				prBssInfo->ucHtOpInfo1 &=
					~(HT_OP_INFO1_SCO |
					  HT_OP_INFO1_STA_CHNL_WIDTH);

			if ((prBssInfo->ucHtOpInfo1 & HT_OP_INFO1_SCO) !=
			    CHNL_EXT_RES)
				prBssInfo->eBssSCO = (enum ENUM_CHNL_EXT)(
					prBssInfo->ucHtOpInfo1 &
					HT_OP_INFO1_SCO);

			/* Revise by own OP BW */
			if (prBssInfo->fgIsOpChangeChannelWidth &&
			    prBssInfo->ucOpChangeChannelWidth == MAX_BW_20MHZ) {
				prBssInfo->ucHtOpInfo1 &=
					~(HT_OP_INFO1_SCO |
					  HT_OP_INFO1_STA_CHNL_WIDTH);
				prBssInfo->eBssSCO = CHNL_EXT_SCN;
			}

			prBssInfo->eHtProtectMode = (enum ENUM_HT_PROTECT_MODE)(
				prBssInfo->u2HtOpInfo2 &
				HT_OP_INFO2_HT_PROTECTION);

			/* To do: process regulatory class 16 */
			if ((prBssInfo->u2HtOpInfo2 &
			     HT_OP_INFO2_OBSS_NON_HT_STA_PRESENT) &&
			    0 /* && regulatory class is 16 */)
				prBssInfo->eGfOperationMode =
					GF_MODE_DISALLOWED;
			else if (prBssInfo->u2HtOpInfo2 &
				 HT_OP_INFO2_NON_GF_HT_STA_PRESENT)
				prBssInfo->eGfOperationMode = GF_MODE_PROTECT;
			else
				prBssInfo->eGfOperationMode = GF_MODE_NORMAL;

			prBssInfo->eRifsOperationMode =
				(prBssInfo->ucHtOpInfo1 & HT_OP_INFO1_RIFS_MODE)
					? RIFS_MODE_NORMAL
					: RIFS_MODE_DISALLOWED;

			break;

#if CFG_SUPPORT_802_11AC
		case ELEM_ID_VHT_CAP:
#if (CFG_SUPPORT_WIFI_6G == 1)
			if (prBssInfo->eBand == BAND_6G) {
				DBGLOG(SCN, WARN, "Ignore VHT CAP IE at 6G\n");
				break;
			}
#endif
			if (!RLM_NET_IS_11AC(prBssInfo) ||
			    IE_LEN(pucIE) != (sizeof(struct IE_VHT_CAP) - 2))
				break;

			prVhtCap = (struct IE_VHT_CAP *)pucIE;

			prStaRec->u4VhtCapInfo = prVhtCap->u4VhtCapInfo;
			/* Set Tx LDPC capability */
			if (IS_FEATURE_FORCE_ENABLED(prWifiVar->ucTxLdpc))
				prStaRec->u4VhtCapInfo |= VHT_CAP_INFO_RX_LDPC;
			else if (IS_FEATURE_DISABLED(prWifiVar->ucTxLdpc))
				prStaRec->u4VhtCapInfo &= ~VHT_CAP_INFO_RX_LDPC;

			/* Set Tx STBC capability */
			if (IS_FEATURE_FORCE_ENABLED(prWifiVar->ucTxStbc))
				prStaRec->u4VhtCapInfo |=
					VHT_CAP_INFO_RX_STBC_MASK;
			else if (IS_FEATURE_DISABLED(prWifiVar->ucTxStbc))
				prStaRec->u4VhtCapInfo &=
					~VHT_CAP_INFO_RX_STBC_MASK;

			/* Set Tx TXOP PS capability */
			if (IS_FEATURE_FORCE_ENABLED(prWifiVar->ucTxopPsTx))
				prStaRec->u4VhtCapInfo |=
					VHT_CAP_INFO_VHT_TXOP_PS;
			else if (IS_FEATURE_DISABLED(prWifiVar->ucTxopPsTx))
				prStaRec->u4VhtCapInfo &=
					~VHT_CAP_INFO_VHT_TXOP_PS;

			/* Set Tx Short GI capability */
			if (IS_FEATURE_FORCE_ENABLED(prWifiVar->ucTxShortGI)) {
				prStaRec->u4VhtCapInfo |=
					VHT_CAP_INFO_SHORT_GI_80;
				prStaRec->u4VhtCapInfo |=
					VHT_CAP_INFO_SHORT_GI_160_80P80;
			} else if (IS_FEATURE_DISABLED(
					   prWifiVar->ucTxShortGI)) {
				prStaRec->u4VhtCapInfo &=
					~VHT_CAP_INFO_SHORT_GI_80;
				prStaRec->u4VhtCapInfo &=
					~VHT_CAP_INFO_SHORT_GI_160_80P80;
			}

			prStaRec->u2VhtRxMcsMap =
				prVhtCap->rVhtSupportedMcsSet.u2RxMcsMap;
			prStaRec->u2VhtRxMcsMapAssoc = prStaRec->u2VhtRxMcsMap;

			prStaRec->u2VhtRxHighestSupportedDataRate =
				prVhtCap->rVhtSupportedMcsSet
					.u2RxHighestSupportedDataRate;
			prStaRec->u2VhtTxMcsMap =
				prVhtCap->rVhtSupportedMcsSet.u2TxMcsMap;
			prStaRec->u2VhtTxHighestSupportedDataRate =
				prVhtCap->rVhtSupportedMcsSet
					.u2TxHighestSupportedDataRate;

			break;

		case ELEM_ID_VHT_OP:
#if (CFG_SUPPORT_WIFI_6G == 1)
			if (prBssInfo->eBand == BAND_6G) {
				DBGLOG(SCN, WARN, "Ignore VHT OP IE at 6G\n");
				break;
			}
#endif
			if (!RLM_NET_IS_11AC(prBssInfo) ||
			    IE_LEN(pucIE) != (sizeof(struct IE_VHT_OP) - 2))
				break;

			prVhtOp = (struct IE_VHT_OP *)pucIE;

			/*Backup peer VHT OpInfo*/
			prStaRec->ucVhtOpChannelWidth =
				prVhtOp->ucVhtOperation[0];
			prStaRec->ucVhtOpChannelFrequencyS1 =
				prVhtOp->ucVhtOperation[1];
			prStaRec->ucVhtOpChannelFrequencyS2 =
				prVhtOp->ucVhtOperation[2];

			rlmModifyVhtBwPara(&prStaRec->ucVhtOpChannelFrequencyS1,
					   &prStaRec->ucVhtOpChannelFrequencyS2,
					   &prStaRec->ucVhtOpChannelWidth);

			prBssInfo->ucVhtChannelWidth =
				prVhtOp->ucVhtOperation[0];
			prBssInfo->ucVhtChannelFrequencyS1 =
				prVhtOp->ucVhtOperation[1];
			prBssInfo->ucVhtChannelFrequencyS2 =
				prVhtOp->ucVhtOperation[2];
			prBssInfo->u2VhtBasicMcsSet = prVhtOp->u2VhtBasicMcsSet;

			rlmModifyVhtBwPara(&prBssInfo->ucVhtChannelFrequencyS1,
					   &prBssInfo->ucVhtChannelFrequencyS2,
					   &prBssInfo->ucVhtChannelWidth);

			/* Revise by own OP BW if needed */
			if ((prBssInfo->fgIsOpChangeChannelWidth) &&
			    (rlmGetVhtOpBwByBssOpBw(
				     prBssInfo->ucOpChangeChannelWidth) <
			     prBssInfo->ucVhtChannelWidth)) {
				rlmFillVhtOpInfoByBssOpBw(
					prBssInfo,
					prBssInfo->ucOpChangeChannelWidth);
			}

			break;
		case ELEM_ID_OP_MODE:
			if (!RLM_NET_IS_11AC(prBssInfo) ||
			    IE_LEN(pucIE) !=
				    (sizeof(struct IE_OP_MODE_NOTIFICATION) -
				     2))
				break;
			prOPNotif = (struct IE_OP_MODE_NOTIFICATION *) pucIE;

			/* NOTE: An AP always sets this field to 0,
			 * so break it if this bit is set.
			 */
			if ((prOPNotif->ucOpMode & VHT_OP_MODE_RX_NSS_TYPE)
			    == VHT_OP_MODE_RX_NSS_TYPE) {
				break;
			}
			fgHasOPModeIE = TRUE;

			/* Check BIT(2) is BW80P80_160.
			 * Refactor ucOpMode with ucVhtOpMode format.
			 */
			if (prOPNotif->ucOpMode &
				VHT_OP_MODE_CHANNEL_WIDTH_80P80_160) {
				prOPNotif->ucOpMode &=
					~(VHT_OP_MODE_CHANNEL_WIDTH |
					VHT_OP_MODE_CHANNEL_WIDTH_80P80_160);
				prOPNotif->ucOpMode |=
					(VHT_OP_MODE_CHANNEL_WIDTH_160_80P80 &
					VHT_OP_MODE_CHANNEL_WIDTH);
			}

			/* Same OP mode, no need to update.
			 * Let the further flow not to update VhtOpMode.
			 */
			if (prStaRec->ucVhtOpMode == prOPNotif->ucOpMode) {
				ucInitVhtOpMode = prStaRec->ucVhtOpMode;
				break;
			}

			fgHasNewOPModeIE = TRUE;
			prStaRec->ucVhtOpMode = prOPNotif->ucOpMode;
			ucVhtOpModeChannelWidth =
				(prOPNotif->ucOpMode &
				VHT_OP_MODE_CHANNEL_WIDTH);

			ucVhtOpModeRxNss =
				(prOPNotif->ucOpMode & VHT_OP_MODE_RX_NSS)
				>> VHT_OP_MODE_RX_NSS_OFFSET;

			ucNssFinal = ((ucVhtOpModeRxNss + 1) >= ucNss) ?
				(ucNss) : (ucVhtOpModeRxNss + 1);

			prStaRec->u2VhtRxMcsMap = BITS((ucNssFinal << 1), 15);
			prStaRec->u2VhtRxMcsMap |=
					(prStaRec->u2VhtRxMcsMapAssoc &
					BITS(0, (ucNssFinal << 1) - 1));

			DBGLOG(RLM, INFO,
			       "NSS=%x RxMcsMap:0x%x, McsMapAssoc:0x%x\n",
			       ucVhtOpModeRxNss, prStaRec->u2VhtRxMcsMap,
			       prStaRec->u2VhtRxMcsMapAssoc);
			break;
#if CFG_SUPPORT_DFS
		case ELEM_ID_WIDE_BAND_CHANNEL_SWITCH:
			if (!RLM_NET_IS_11AC(prBssInfo) ||
			    IE_LEN(pucIE) !=
				    (sizeof(struct IE_WIDE_BAND_CHANNEL) - 2))
				break;
			DBGLOG(RLM, INFO,
			       "[Channel Switch] ELEM_ID_WIDE_BAND_CHANNEL_SWITCH, 11AC\n");
			prWideBandChannelIE =
				(struct IE_WIDE_BAND_CHANNEL *)pucIE;
			ucChannelAnnounceVhtBw =
				prWideBandChannelIE->ucNewChannelWidth;
			ucChannelAnnounceChannelS1 =
				prWideBandChannelIE->ucChannelS1;
			ucChannelAnnounceChannelS2 =
				prWideBandChannelIE->ucChannelS2;
			fgHasWideBandIE = TRUE;
			DBGLOG(RLM, INFO, "[Ch] BW=%d, s1=%d, s2=%d\n",
			       ucChannelAnnounceVhtBw,
			       ucChannelAnnounceChannelS1,
			       ucChannelAnnounceChannelS2);
			break;
#endif

#endif
		case ELEM_ID_20_40_BSS_COEXISTENCE:
			if (!RLM_NET_IS_11N(prBssInfo))
				break;
			/* To do: store if scanning exemption grant to BssInfo
			 */
			break;

		case ELEM_ID_OBSS_SCAN_PARAMS:
			if (!RLM_NET_IS_11N(prBssInfo) ||
			    IE_LEN(pucIE) !=
				    (sizeof(struct IE_OBSS_SCAN_PARAM) - 2))
				break;
			/* Store OBSS parameters to BssInfo */
			prObssScnParam = (struct IE_OBSS_SCAN_PARAM *)pucIE;
			break;

		case ELEM_ID_EXTENDED_CAP:
			if (!RLM_NET_IS_11N(prBssInfo))
				break;
			/* To do: store extended capability (PSMP, coexist) to
			 * BssInfo
			 */
			break;

		case ELEM_ID_ERP_INFO:
			if (IE_LEN(pucIE) != (sizeof(struct IE_ERP) - 2) ||
			    prBssInfo->eBand != BAND_2G4)
				break;
			ucERP = ERP_INFO_IE(pucIE)->ucERP;
			prBssInfo->fgErpProtectMode =
				(ucERP & ERP_INFO_USE_PROTECTION) ? TRUE
								  : FALSE;

			if (ucERP & ERP_INFO_BARKER_PREAMBLE_MODE)
				prBssInfo->fgUseShortPreamble = FALSE;
			break;

		case ELEM_ID_DS_PARAM_SET:
			if (IE_LEN(pucIE) == ELEM_MAX_LEN_DS_PARAMETER_SET)
				ucPrimaryChannel =
					DS_PARAM_IE(pucIE)->ucCurrChnl;
			break;
#if CFG_SUPPORT_DFS
		case ELEM_ID_CH_SW_ANNOUNCEMENT:
			if (IE_LEN(pucIE) !=
			    (sizeof(struct IE_CHANNEL_SWITCH) - 2))
				break;

			prChannelSwitchAnnounceIE =
				(struct IE_CHANNEL_SWITCH *)pucIE;

			DBGLOG(RLM, INFO, "[Ch] Count=%d\n",
			       prChannelSwitchAnnounceIE->ucChannelSwitchCount);

			fgHasChannelSwitchIE = TRUE;

			/* Decide the switch new channel */
			if (prChannelSwitchAnnounceIE
				    ->ucChannelSwitchCount <= 3) {
				DBGLOG(RLM, INFO,
				       "[Ch] switch channel [%d]->[%d]\n",
				       prBssInfo->ucPrimaryChannel,
				       prChannelSwitchAnnounceIE
					       ->ucNewChannelNum);
				ucChannelAnnouncePri =
					prChannelSwitchAnnounceIE
						->ucNewChannelNum;

#if CFG_DFS_NEWCH_DFS_FORCE_DISCONNECT
				Channel = ieee80211_get_channel(
						priv_to_wiphy(prAdapter->prGlueInfo),
						ieee80211_channel_to_frequency(ucChannelAnnouncePri, KAL_BAND_5GHZ));
				DBGLOG(RLM, INFO, "[DFS][CSA][CLIENT] Switch to DFS channel: new ChNum = [%d]\n",ucChannelAnnouncePri);
				if (Channel &&
				    (Channel->flags & IEEE80211_CHAN_RADAR)) {
					DBGLOG(RLM, INFO, "[DFS][CSA][CLIENT] New channel is DFS channel!");
					aisBssLinkDown(prAdapter, GET_IOCTL_BSSIDX(prAdapter));
				}
				else
#endif
				{
					fgNeedSwitchChannel = TRUE;
				}
			}

			/* Decide to stop Tx by switch mode */
			if (prChannelSwitchAnnounceIE
					->ucChannelSwitchMode == 1) {

				/* Need to stop data transmission immediately */
				if (!g_fgHasStopTx) {
					g_fgHasStopTx = TRUE;

					/* Stop Tx until channel swtich done */
					qmSetStaRecTxAllowed(prAdapter,
						prStaRec, FALSE);
					DBGLOG(RLM, EVENT,
						"[Ch] TxAllowed = FALSE\n");
				}
			}
#ifdef CFG_DFS_CHSW_FORCE_BW20
			else {
				g_fgHasChannelSwitchIE = TRUE;

				if (RLM_NET_IS_11AC(prBssInfo)) {
					DBGLOG(RLM, INFO,
						   "Send Operation Action Frame");
					rlmSendOpModeNotificationFrame(
						prAdapter, prStaRec,
						VHT_OP_MODE_CHANNEL_WIDTH_20,
						1);
				} else {
					DBGLOG(RLM, INFO,
						"Skip Send Operation Action Frame");
				}
			}
#endif
			break;
		case ELEM_ID_SCO:
			if (IE_LEN(pucIE) !=
			    (sizeof(struct IE_SECONDARY_OFFSET) - 2))
				break;

			prSecondaryOffsetIE =
				(struct IE_SECONDARY_OFFSET *)pucIE;
			DBGLOG(RLM, INFO, "[Channel Switch] SCO [%d]->[%d]\n",
			       prBssInfo->eBssSCO,
			       prSecondaryOffsetIE->ucSecondaryOffset);
			eChannelAnnounceSco =
				(enum ENUM_CHNL_EXT)
					prSecondaryOffsetIE->ucSecondaryOffset;
			fgHasSCOIE = TRUE;
			break;
#endif

#if CFG_SUPPORT_QUIET && 0
		/* Note: RRM code should be moved to independent RRM function by
		 *       component design rule. But we attach it to RLM
		 * temporarily
		 */
		case ELEM_ID_QUIET:
			rrmQuietHandleQuietIE(prBssInfo,
					      (struct IE_QUIET *)pucIE);
			fgHasQuietIE = TRUE;
			break;
#endif

#if (CFG_SUPPORT_802_11AX == 1)
		case ELEM_ID_RESERVED:
			if (IE_ID_EXT(pucIE) == ELEM_EXT_ID_HE_CAP)
				heRlmRecHeCapInfo(prAdapter, prStaRec, pucIE);
#if (CFG_SUPPORT_WIFI_6G == 1)
			else if (IE_ID_EXT(pucIE) == ELEM_EXT_ID_HE_OP) {
				heRlmRecHeOperation(prAdapter,
							prBssInfo, pucIE);

				u4Offset = OFFSET_OF(
					struct _IE_HE_OP_T,
					aucVarInfo[0]);

				if (prBssInfo->fgIsHE6GPresent) {
					if (prBssInfo->fgIsCoHostedBssPresent)
						u4Offset += sizeof(uint8_t);

					pr6gOperInfor =
						(struct _6G_OPER_INFOR_T *)
						(((uint8_t *) pucIE)+
							u4Offset);

					ucPrimaryChannel =
						pr6gOperInfor->
						ucPrimaryChannel;

					prBssInfo->ucVhtChannelFrequencyS1 =
						pr6gOperInfor->
						ucChannelCenterFreqSeg0;

					prBssInfo->ucVhtChannelFrequencyS2 =
						pr6gOperInfor->
						ucChannelCenterFreqSeg1;

					prBssInfo->eBssSCO = CHNL_EXT_SCN;

					rlmTransferHe6gOpInfor(ucPrimaryChannel,
						(uint8_t)pr6gOperInfor->
						rControl.bits.ChannelWidth,
						&prBssInfo->
							ucVhtChannelWidth,
						&prBssInfo->
							ucVhtChannelFrequencyS1,
						&prBssInfo->
							ucVhtChannelFrequencyS2,
						&prBssInfo->eBssSCO);
				}
			} else if (IE_ID_EXT(pucIE) ==
				ELEM_EXT_ID_HE_6G_BAND_CAP) {
				if (!RLM_NET_IS_11AX(prBssInfo) ||
				(prBssInfo->eBand != BAND_6G) ||
				IE_LEN(pucIE) !=
				(sizeof(struct _IE_HE_6G_BAND_CAP_T) - 2))
					break;

				prHe6gBandCap =
					(struct _IE_HE_6G_BAND_CAP_T *)pucIE;

				if ((prStaRec->u2He6gBandCapInfo &
					HT_CAP_INFO_SM_POWER_SAVE) !=
					(prHe6gBandCap->u2CapInfo &
						HT_CAP_INFO_SM_POWER_SAVE))

					/* Purpose : To detect SMPS change */
					IsfgHe6gBandCapChange = TRUE;

				prStaRec->u2He6gBandCapInfo =
					prHe6gBandCap->u2CapInfo;
			}
#else
			else if (IE_ID_EXT(pucIE) == ELEM_EXT_ID_HE_OP)
				heRlmRecHeOperation(prAdapter,
							prBssInfo, pucIE);
#endif /* CFG_SUPPORT_WIFI_6G */
			break;
#endif /* CFG_SUPPORT_802_11AX */

#if (CFG_SUPPORT_TWT_HOTSPOT_AC == 1)
		case ELEM_ID_VENDOR:
			if ((MTK_OUI_IE(pucIE)->aucOui[0] == aucMtkOui[0]) &&
			(MTK_OUI_IE(pucIE)->aucOui[1] == aucMtkOui[1]) &&
			(MTK_OUI_IE(pucIE)->aucOui[2] == aucMtkOui[2]) &&
			(MTK_OUI_IE(pucIE)->ucLength ==
				ELEM_MIN_LEN_MTK_OUI) &&
			((MTK_OUI_IE(pucIE)->aucCapability[0] &
				MTK_SYNERGY_CAP_SUPPORT_TWT_HOTSPOT_AC) ==
				MTK_SYNERGY_CAP_SUPPORT_TWT_HOTSPOT_AC))
				prStaRec->ucTWTHospotSupport = TRUE;

			break;
#endif

		default:
			break;
		} /* end of switch */
	}	 /* end of IE_FOR_EACH */

	if (prStaRec->ucStaState == STA_STATE_3) {
#if (CFG_SUPPORT_WIFI_6G == 1)
		if (IsfgHe6gBandCapChange || IsfgHtCapChange)
#else
		if (IsfgHtCapChange)
#endif
			cnmStaSendUpdateCmd(prAdapter, prStaRec, FALSE);
	}

	/* Some AP will have wrong channel number (255) when running time.
	 * Check if correct channel number information. 20110501
	 */
	if ((prBssInfo->eBand == BAND_2G4 && ucPrimaryChannel > 14) ||
	    (prBssInfo->eBand == BAND_5G &&
	     (ucPrimaryChannel >= 180 || ucPrimaryChannel <= 14)))
		ucPrimaryChannel = 0;
#if CFG_SUPPORT_802_11AC
	/* Check whether the Operation Mode IE is exist or not.
	 *  If exists, then the channel bandwidth of VHT operation field  is
	 * changed
	 *  with the channel bandwidth setting of Operation Mode field.
	 *  The channel bandwidth of OP Mode IE  is  0, represent as 20MHz.
	 *  The channel bandwidth of OP Mode IE  is  1, represent as 40MHz.
	 *  The channel bandwidth of OP Mode IE  is  2, represent as 80MHz.
	 *  The channel bandwidth of OP Mode IE  is  3, represent as
	 * 160/80+80MHz.
	 */
	if (fgHasNewOPModeIE == TRUE) {
		if (prStaRec->ucStaState == STA_STATE_3) {
			/* 1. Modify channel width parameters */
			rlmRecOpModeBwForClient(ucVhtOpModeChannelWidth,
						prBssInfo);

			/* 2. Update StaRec to FW (BssInfo will be updated after
			 * return from this function)
			 */
			DBGLOG(RLM, INFO,
			       "Update OpMode to 0x%x, to FW due to OpMode Notificaition",
			       prStaRec->ucVhtOpMode);
			cnmStaSendUpdateCmd(prAdapter, prStaRec, FALSE);

			/* 3. Revise by own OP BW if needed */
			if ((prBssInfo->fgIsOpChangeChannelWidth)) {
				/* VHT */
				if (rlmGetVhtOpBwByBssOpBw(
					    prBssInfo->ucOpChangeChannelWidth) <
				    prBssInfo->ucVhtChannelWidth)
					rlmFillVhtOpInfoByBssOpBw(
					prBssInfo,
					prBssInfo
					->ucOpChangeChannelWidth);
				/* HT */
				if (prBssInfo->fgIsOpChangeChannelWidth &&
				    prBssInfo->ucOpChangeChannelWidth ==
					    MAX_BW_20MHZ) {
					prBssInfo->ucHtOpInfo1 &=
						~(HT_OP_INFO1_SCO |
						  HT_OP_INFO1_STA_CHNL_WIDTH);
					prBssInfo->eBssSCO = CHNL_EXT_SCN;
				}
			}
		}
	} else { /* Set Default if the VHT OP mode field is not present */
		if (!fgHasOPModeIE) {
			ucInitVhtOpMode |=
				rlmGetOpModeBwByVhtAndHtOpInfo(prBssInfo);
			ucInitVhtOpMode |=
				((rlmGetSupportRxNssInVhtCap(prVhtCap) - 1)
				<< VHT_OP_MODE_RX_NSS_OFFSET) &
				VHT_OP_MODE_RX_NSS;
		}
		if ((prStaRec->ucVhtOpMode != ucInitVhtOpMode) &&
			(prStaRec->ucStaState == STA_STATE_3)) {
			prStaRec->ucVhtOpMode = ucInitVhtOpMode;
			DBGLOG(RLM, INFO, "Update OpMode to 0x%x",
			       prStaRec->ucVhtOpMode);
			DBGLOG(RLM, INFO,
			       "to FW due to NO OpMode Notificaition\n");
			cnmStaSendUpdateCmd(prAdapter, prStaRec, FALSE);
		} else
			prStaRec->ucVhtOpMode = ucInitVhtOpMode;
	}
#endif

#if CFG_SUPPORT_DFS
	/* Check whether Channel Announcement IE, Secondary Offset IE &
	 * Wide Bandwidth Channel Switch IE exist or not. If exist, the
	 * priority is
	 * the highest.
	 */

	if (fgNeedSwitchChannel) {
		struct BSS_DESC *prBssDesc;
		struct PARAM_SSID rSsid;

		memset(&rSsid, 0, sizeof(struct PARAM_SSID));

		prBssInfo->ucPrimaryChannel = ucChannelAnnouncePri;
		prBssInfo->eBand =
			((ucChannelAnnouncePri <= 14) ? BAND_2G4 :
#if (CFG_SUPPORT_WIFI_6G == 1)
			(ucChannelAnnouncePri > 180) ? BAND_6G :
#endif
			BAND_5G);

		/* Change to BW20 for certification issue due to signal sidelope
		 * leakage
		 */
		prBssInfo->ucVhtChannelWidth = 0;
		prBssInfo->ucVhtChannelFrequencyS1 = 0;
		prBssInfo->ucVhtChannelFrequencyS2 = 0;
		prBssInfo->eBssSCO = 0;

		if (fgHasWideBandIE != FALSE) {
			prBssInfo->ucVhtChannelWidth = ucChannelAnnounceVhtBw;
			prBssInfo->ucVhtChannelFrequencyS1 =
				ucChannelAnnounceChannelS1;
			prBssInfo->ucVhtChannelFrequencyS2 =
				ucChannelAnnounceChannelS2;

			/* Revise by own OP BW if needed */
			if ((prBssInfo->fgIsOpChangeChannelWidth) &&
			    (rlmGetVhtOpBwByBssOpBw(
				     prBssInfo->ucOpChangeChannelWidth) <
			     prBssInfo->ucVhtChannelWidth)) {

				DBGLOG(RLM, LOUD,
				       "Change to w:%d s1:%d s2:%d since own changed BW < peer's WideBand BW",
				       prBssInfo->ucVhtChannelWidth,
				       prBssInfo->ucVhtChannelFrequencyS1,
				       prBssInfo->ucVhtChannelFrequencyS2);
				rlmFillVhtOpInfoByBssOpBw(
					prBssInfo,
					prBssInfo->ucOpChangeChannelWidth);
			}
		}
		if (fgHasSCOIE != FALSE)
			prBssInfo->eBssSCO = eChannelAnnounceSco;

		COPY_SSID(rSsid.aucSsid, rSsid.u4SsidLen, prBssInfo->aucSSID,
			  prBssInfo->ucSSIDLen);
		prBssDesc = scanSearchBssDescByBssidAndSsid(
			prAdapter, prBssInfo->aucBSSID, TRUE, &rSsid);

		if (prBssDesc) {
			DBGLOG(RLM, INFO,
			       "DFS: BSS: " MACSTR
			       " Desc found, channel from %u to %u\n ",
			       MAC2STR(prBssInfo->aucBSSID),
			       prBssDesc->ucChannelNum, ucChannelAnnouncePri);
			prBssDesc->ucChannelNum = ucChannelAnnouncePri;
			prBssDesc->eChannelWidth = prBssInfo->ucVhtChannelWidth;
			prBssDesc->ucCenterFreqS1 =
				prBssInfo->ucVhtChannelFrequencyS1;
			prBssDesc->ucCenterFreqS2 =
				prBssInfo->ucVhtChannelFrequencyS2;
			kalIndicateChannelSwitch(
				prAdapter->prGlueInfo,
				prBssInfo->eBssSCO,
#if (CFG_SUPPORT_WIFI_6G == 1)
				prBssDesc->eBand,
#endif
				prBssDesc->ucChannelNum);
		} else {
			DBGLOG(RLM, INFO,
			       "DFS: BSS: " MACSTR " Desc is not found\n ",
			       MAC2STR(prBssInfo->aucBSSID));
		}
	}
#endif

	/* Restore the Tx when receive a beacon on new ch */
	if (!fgHasChannelSwitchIE && g_fgHasStopTx) {
		qmSetStaRecTxAllowed(prAdapter, prStaRec, TRUE);
		DBGLOG(RLM, EVENT, "[Ch] TxAllowed = TRUE\n");
		g_fgHasStopTx = FALSE;
	}

#if CFG_SUPPORT_DFS
#ifdef CFG_DFS_CHSW_FORCE_BW20
	/*DFS Certification for Channel Bandwidth 20MHz */
	DBGLOG(RLM, INFO, "Ch : SwitchIE = %d\n", g_fgHasChannelSwitchIE);
	if (g_fgHasChannelSwitchIE == TRUE) {
		prBssInfo->eBssSCO = CHNL_EXT_SCN;
		prBssInfo->ucVhtChannelWidth = CW_20_40MHZ;
		prBssInfo->ucVhtChannelFrequencyS1 = 0;
		prBssInfo->ucVhtChannelFrequencyS2 = 255;
		prBssInfo->ucHtOpInfo1 &=
			~(HT_OP_INFO1_SCO | HT_OP_INFO1_STA_CHNL_WIDTH);
		DBGLOG(RLM, INFO, "Ch : DFS has Appeared\n");
	}
#endif
#endif

	/* Do not write prBssInfo->ucVhtChannelWidth directly
	 * in rlmReviseMaxBw, otherwise it will cause the following
	 * struct members being overwritten unexpectly.
	 */
	eChannelWidth = (enum ENUM_CHANNEL_WIDTH)
		prBssInfo->ucVhtChannelWidth;
	rlmReviseMaxBw(prAdapter, prBssInfo->ucBssIndex,
		&prBssInfo->eBssSCO,
		&eChannelWidth,
		&prBssInfo->ucVhtChannelFrequencyS1,
		&prBssInfo->ucPrimaryChannel);
	prBssInfo->ucVhtChannelWidth = (uint8_t)eChannelWidth;

	rlmRevisePreferBandwidthNss(prAdapter, prBssInfo->ucBssIndex, prStaRec);

	fgDomainValid = rlmDomainIsValidRfSetting(
		prAdapter,
		prBssInfo->eBand,
		prBssInfo->ucPrimaryChannel,
		prBssInfo->eBssSCO,
		prBssInfo->ucVhtChannelWidth,
		prBssInfo->ucVhtChannelFrequencyS1,
		prBssInfo->ucVhtChannelFrequencyS2);

	if (fgDomainValid == FALSE) {
		/*Dump IE Inforamtion */
		DBGLOG(RLM, WARN, "rlmRecIeInfoForClient IE Information\n");
		DBGLOG(RLM, WARN, "IE Length = %d\n", u2IELength);
		DBGLOG_MEM8(RLM, WARN, pucDumpIE, u2IELength);

		/*Error Handling for Non-predicted IE - Fixed to set 20MHz */
		prBssInfo->ucVhtChannelWidth = CW_20_40MHZ;
		prBssInfo->ucVhtChannelFrequencyS1 = 0;
		prBssInfo->ucVhtChannelFrequencyS2 = 0;
		prBssInfo->eBssSCO = CHNL_EXT_SCN;
		prBssInfo->ucHtOpInfo1 &=
			~(HT_OP_INFO1_SCO | HT_OP_INFO1_STA_CHNL_WIDTH);

		/* Check SAP channel */
		p2pFuncSwitchSapChannel(prAdapter);
	}

#if CFG_SUPPORT_QUIET && 0
	if (!fgHasQuietIE)
		rrmQuietIeNotExist(prAdapter, prBssInfo);
#endif

	/* Check if OBSS scan process will launch */
	if (!prAdapter->fgEnOnlineScan || !prObssScnParam ||
	    !(prStaRec->u2HtCapInfo & HT_CAP_INFO_SUP_CHNL_WIDTH) ||
	    prBssInfo->eBand != BAND_2G4 || !prBssInfo->fg40mBwAllowed) {

		/* Note: it is ok not to stop rObssScanTimer() here */
		prBssInfo->u2ObssScanInterval = 0;
	} else {
		if (prObssScnParam->u2TriggerScanInterval <
		    OBSS_SCAN_MIN_INTERVAL)
			prObssScnParam->u2TriggerScanInterval =
				OBSS_SCAN_MIN_INTERVAL;
		if (prBssInfo->u2ObssScanInterval !=
		    prObssScnParam->u2TriggerScanInterval) {

			prBssInfo->u2ObssScanInterval =
				prObssScnParam->u2TriggerScanInterval;

			/* Start timer to trigger OBSS scanning */
			cnmTimerStartTimer(
				prAdapter, &prBssInfo->rObssScanTimer,
				prBssInfo->u2ObssScanInterval * MSEC_PER_SEC);
		}
	}

	return ucPrimaryChannel;
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief Update parameters from channel width field in OP Mode IE/action frame
 *
 * \param[in]
 *
 * \return none
 */
/*----------------------------------------------------------------------------*/
static void rlmRecOpModeBwForClient(uint8_t ucVhtOpModeChannelWidth,
				    struct BSS_INFO *prBssInfo)
{

	struct STA_RECORD *prStaRec = NULL;

	if (!prBssInfo)
		return;

	prStaRec = prBssInfo->prStaRecOfAP;
	if (!prStaRec)
		return;

	switch (ucVhtOpModeChannelWidth) {
	case VHT_OP_MODE_CHANNEL_WIDTH_20:
		prBssInfo->ucVhtChannelWidth = VHT_OP_CHANNEL_WIDTH_20_40;
		prBssInfo->ucHtOpInfo1 &= ~HT_OP_INFO1_STA_CHNL_WIDTH;
		prStaRec->u2HtCapInfo &= ~HT_CAP_INFO_SUP_CHNL_WIDTH;

#if CFG_OPMODE_CONFLICT_OPINFO
		if (prBssInfo->eBssSCO != CHNL_EXT_SCN) {
			DBGLOG(RLM, WARN,
			       "HT_OP_Info != OPmode_Notifify, follow OPmode_Notify to BW20.\n");
			prBssInfo->eBssSCO = CHNL_EXT_SCN;
		}
#endif
		break;
	case VHT_OP_MODE_CHANNEL_WIDTH_40:
		prBssInfo->ucVhtChannelWidth = VHT_OP_CHANNEL_WIDTH_20_40;
		prBssInfo->ucHtOpInfo1 |= HT_OP_INFO1_STA_CHNL_WIDTH;
		prStaRec->u2HtCapInfo |= HT_CAP_INFO_SUP_CHNL_WIDTH;

#if CFG_OPMODE_CONFLICT_OPINFO
		if (prBssInfo->eBssSCO == CHNL_EXT_SCN) {
			prBssInfo->ucHtOpInfo1 &= ~HT_OP_INFO1_STA_CHNL_WIDTH;
			prStaRec->u2HtCapInfo &= ~HT_CAP_INFO_SUP_CHNL_WIDTH;
			DBGLOG(RLM, WARN,
			       "HT_OP_Info != OPmode_Notifify, follow HT_OP_Info to BW20.\n");
		}
#endif
		break;
	case VHT_OP_MODE_CHANNEL_WIDTH_80:
#if CFG_OPMODE_CONFLICT_OPINFO
		if (prBssInfo->ucVhtChannelWidth !=
		    VHT_OP_MODE_CHANNEL_WIDTH_80) {
			DBGLOG(RLM, WARN,
			       "VHT_OP != OPmode:%d, follow VHT_OP to VHT_OP:%d HT_OP:%d\n",
			       ucVhtOpModeChannelWidth,
			       prBssInfo->ucVhtChannelWidth,
			       (uint8_t)(prBssInfo->ucHtOpInfo1 &
					 HT_OP_INFO1_STA_CHNL_WIDTH) >>
				       HT_OP_INFO1_STA_CHNL_WIDTH_OFFSET);
		} else
#endif
		{
			prBssInfo->ucVhtChannelWidth = VHT_OP_CHANNEL_WIDTH_80;
			prBssInfo->ucHtOpInfo1 |= HT_OP_INFO1_STA_CHNL_WIDTH;
			prStaRec->u2HtCapInfo |= HT_CAP_INFO_SUP_CHNL_WIDTH;
		}
		break;
	case VHT_OP_MODE_CHANNEL_WIDTH_160_80P80:
/* Determine BW160 or BW80+BW80 by VHT OP Info */
#if CFG_OPMODE_CONFLICT_OPINFO
		if ((prBssInfo->ucVhtChannelWidth !=
		     VHT_OP_CHANNEL_WIDTH_160) &&
		    (prBssInfo->ucVhtChannelWidth !=
		     VHT_OP_CHANNEL_WIDTH_80P80)) {
			DBGLOG(RLM, WARN,
			       "VHT_OP != OPmode:%d, follow VHT_OP to VHT_OP:%d HT_OP:%d\n",
			       ucVhtOpModeChannelWidth,
			       prBssInfo->ucVhtChannelWidth,
			       (uint8_t)(prBssInfo->ucHtOpInfo1 &
					 HT_OP_INFO1_STA_CHNL_WIDTH) >>
				       HT_OP_INFO1_STA_CHNL_WIDTH_OFFSET);
		} else
#endif
		{
			prBssInfo->ucHtOpInfo1 |= HT_OP_INFO1_STA_CHNL_WIDTH;
			prStaRec->u2HtCapInfo |= HT_CAP_INFO_SUP_CHNL_WIDTH;
		}
		break;
	default:
		break;
	}
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief Update parameters from Association Response frame
 *
 * \param[in]
 *
 * \return none
 */
/*----------------------------------------------------------------------------*/
static void rlmRecAssocRespIeInfoForClient(struct ADAPTER *prAdapter,
					   struct BSS_INFO *prBssInfo,
					   uint8_t *pucIE, uint16_t u2IELength)
{
	uint16_t u2Offset;
	struct STA_RECORD *prStaRec;
	u_int8_t fgIsHasHtCap = FALSE;
	u_int8_t fgIsHasVhtCap = FALSE;
#if (CFG_SUPPORT_802_11AX == 1)
	u_int8_t fgIsHasHeCap = FALSE;
#endif
	struct BSS_DESC *prBssDesc;
	struct PARAM_SSID rSsid;

	ASSERT(prAdapter);
	ASSERT(prBssInfo);
	ASSERT(pucIE);

	prStaRec = prBssInfo->prStaRecOfAP;

	if (!prStaRec)
		return;
	memset(&rSsid, 0, sizeof(struct PARAM_SSID));
	COPY_SSID(rSsid.aucSsid, rSsid.u4SsidLen, prBssInfo->aucSSID,
		  prBssInfo->ucSSIDLen);
	prBssDesc = scanSearchBssDescByBssidAndSsid(
		prAdapter, prStaRec->aucMacAddr, TRUE, &rSsid);

	IE_FOR_EACH(pucIE, u2IELength, u2Offset)
	{
		switch (IE_ID(pucIE)) {
		case ELEM_ID_HT_CAP:
#if (CFG_SUPPORT_WIFI_6G == 1)
			if (prBssInfo->eBand == BAND_6G) {
				DBGLOG(SCN, WARN, "Ignore HT CAP IE at 6G\n");
				break;
			}
#endif
			if (!RLM_NET_IS_11N(prBssInfo) ||
			    IE_LEN(pucIE) != (sizeof(struct IE_HT_CAP) - 2))
				break;
			fgIsHasHtCap = TRUE;
			break;
#if CFG_SUPPORT_802_11AC
		case ELEM_ID_VHT_CAP:
#if (CFG_SUPPORT_WIFI_6G == 1)
			if (prBssInfo->eBand == BAND_6G) {
				DBGLOG(SCN, WARN, "Ignore VHT CAP IE at 6G\n");
				break;
			}
#endif
			if (!RLM_NET_IS_11AC(prBssInfo) ||
			    IE_LEN(pucIE) != (sizeof(struct IE_VHT_CAP) - 2))
				break;
			fgIsHasVhtCap = TRUE;
			break;
#endif
#if (CFG_SUPPORT_802_11AX == 1)
		case ELEM_ID_RESERVED:
			if (IE_ID_EXT(pucIE) != ELEM_EXT_ID_HE_CAP ||
			    !RLM_NET_IS_11AX(prBssInfo))
				break;
			fgIsHasHeCap = TRUE;
			break;
#endif

		default:
			break;
		} /* end of switch */
	}	 /* end of IE_FOR_EACH */

	if (!fgIsHasHtCap) {
		prStaRec->ucDesiredPhyTypeSet &= ~PHY_TYPE_BIT_HT;
		if (prBssDesc) {
			if (prBssDesc->ucPhyTypeSet & PHY_TYPE_BIT_HT) {
				DBGLOG(RLM, WARN,
				       "PhyTypeSet in Beacon and AssocResp are unsync. ");
				DBGLOG(RLM, WARN,
				       "Follow AssocResp to disable HT.\n");
			}
		}
	}
	if (!fgIsHasVhtCap) {
		prStaRec->ucDesiredPhyTypeSet &= ~PHY_TYPE_BIT_VHT;
		if (prBssDesc) {
			if (prBssDesc->ucPhyTypeSet & PHY_TYPE_BIT_VHT) {
				DBGLOG(RLM, WARN,
				       "PhyTypeSet in Beacon and AssocResp are unsync. ");
				DBGLOG(RLM, WARN,
				       "Follow AssocResp to disable VHT.\n");
			}
		}
	}
#if (CFG_SUPPORT_802_11AX == 1)
	if (!fgIsHasHeCap) {
		prStaRec->ucDesiredPhyTypeSet &= ~PHY_TYPE_BIT_HE;
		if (prBssDesc) {
			if (prBssDesc->ucPhyTypeSet & PHY_TYPE_BIT_HE) {
				DBGLOG(RLM, WARN, "PhyTypeSet are unsync. ");
				DBGLOG(RLM, WARN, "Disable HE per assoc.\n");
			}
		}
	}
#endif
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief AIS or P2P GC.
 *
 * \param[in]
 *
 * \return none
 */
/*----------------------------------------------------------------------------*/
static u_int8_t rlmRecBcnFromNeighborForClient(struct ADAPTER *prAdapter,
					       struct BSS_INFO *prBssInfo,
					       struct SW_RFB *prSwRfb,
					       uint8_t *pucIE,
					       uint16_t u2IELength)
{
	uint16_t u2Offset, i;
	uint8_t ucPriChannel, ucSecChannel;
	enum ENUM_CHNL_EXT eSCO;
	u_int8_t fgHtBss, fg20mReq;
	enum ENUM_BAND eBand = 0;
	struct RX_DESC_OPS_T *prRxDescOps;

	ASSERT(prAdapter);
	ASSERT(prBssInfo && prSwRfb);
	ASSERT(pucIE);
	prRxDescOps = prAdapter->chip_info->prRxDescOps;

	/* Record it to channel list to change 20/40 bandwidth */
	ucPriChannel = 0;
	eSCO = CHNL_EXT_SCN;

	fgHtBss = FALSE;
	fg20mReq = FALSE;

	IE_FOR_EACH(pucIE, u2IELength, u2Offset)
	{
		switch (IE_ID(pucIE)) {
		case ELEM_ID_HT_CAP: {
			struct IE_HT_CAP *prHtCap;

			if (IE_LEN(pucIE) != (sizeof(struct IE_HT_CAP) - 2))
				break;

			prHtCap = (struct IE_HT_CAP *)pucIE;
			if (prHtCap->u2HtCapInfo & HT_CAP_INFO_40M_INTOLERANT)
				fg20mReq = TRUE;
			fgHtBss = TRUE;
			break;
		}
		case ELEM_ID_HT_OP: {
			struct IE_HT_OP *prHtOp;

			if (IE_LEN(pucIE) != (sizeof(struct IE_HT_OP) - 2))
				break;

			prHtOp = (struct IE_HT_OP *)pucIE;
			/* Workaround that some APs fill primary channel field
			 * by its
			 * secondary channel, but its DS IE is correct 20110610
			 */
			if (ucPriChannel == 0)
				ucPriChannel = prHtOp->ucPrimaryChannel;

			if ((prHtOp->ucInfo1 & HT_OP_INFO1_SCO) != CHNL_EXT_RES)
				eSCO = (enum ENUM_CHNL_EXT)(prHtOp->ucInfo1 &
							    HT_OP_INFO1_SCO);
			break;
		}
		case ELEM_ID_20_40_BSS_COEXISTENCE: {
			struct IE_20_40_COEXIST *prCoexist;

			if (IE_LEN(pucIE) !=
			    (sizeof(struct IE_20_40_COEXIST) - 2))
				break;

			prCoexist = (struct IE_20_40_COEXIST *)pucIE;
			if (prCoexist->ucData & BSS_COEXIST_40M_INTOLERANT)
				fg20mReq = TRUE;
			break;
		}
		case ELEM_ID_DS_PARAM_SET:
			if (IE_LEN(pucIE) !=
			    (sizeof(struct IE_DS_PARAM_SET) - 2))
				break;
			ucPriChannel = DS_PARAM_IE(pucIE)->ucCurrChnl;
			break;

		default:
			break;
		}
	}

	/* To do: Update channel list and 5G band. All channel lists have the
	 * same
	 * update procedure. We should give it the entry pointer of desired
	 * channel list.
	 */
	RX_STATUS_GET(prRxDescOps, eBand, get_rf_band, prSwRfb->prRxStatus);
	if (eBand != BAND_2G4)
		return FALSE;

	if (ucPriChannel == 0 || ucPriChannel > 14) {
		RX_STATUS_GET(
			prRxDescOps,
			ucPriChannel,
			get_ch_num,
			prSwRfb->prRxStatus);
	}

	if (fgHtBss) {
		ASSERT(prBssInfo->auc2G_PriChnlList[0] <= CHNL_LIST_SZ_2G);
		for (i = 1; i <= prBssInfo->auc2G_PriChnlList[0] &&
			    i <= CHNL_LIST_SZ_2G;
		     i++) {
			if (prBssInfo->auc2G_PriChnlList[i] == ucPriChannel)
				break;
		}
		if ((i > prBssInfo->auc2G_PriChnlList[0]) &&
		    (i <= CHNL_LIST_SZ_2G)) {
			prBssInfo->auc2G_PriChnlList[i] = ucPriChannel;
			prBssInfo->auc2G_PriChnlList[0]++;
		}

		/* Update secondary channel */
		if (eSCO != CHNL_EXT_SCN) {
			ucSecChannel = (eSCO == CHNL_EXT_SCA)
					       ? (ucPriChannel + 4)
					       : (ucPriChannel - 4);

			ASSERT(prBssInfo->auc2G_SecChnlList[0] <=
			       CHNL_LIST_SZ_2G);
			for (i = 1; i <= prBssInfo->auc2G_SecChnlList[0] &&
				    i <= CHNL_LIST_SZ_2G;
			     i++) {
				if (prBssInfo->auc2G_SecChnlList[i] ==
				    ucSecChannel)
					break;
			}
			if ((i > prBssInfo->auc2G_SecChnlList[0]) &&
			    (i <= CHNL_LIST_SZ_2G)) {
				prBssInfo->auc2G_SecChnlList[i] = ucSecChannel;
				prBssInfo->auc2G_SecChnlList[0]++;
			}
		}

		/* Update 20M bandwidth request channels */
		if (fg20mReq) {
			ASSERT(prBssInfo->auc2G_20mReqChnlList[0] <=
			       CHNL_LIST_SZ_2G);
			for (i = 1; i <= prBssInfo->auc2G_20mReqChnlList[0] &&
				    i <= CHNL_LIST_SZ_2G;
			     i++) {
				if (prBssInfo->auc2G_20mReqChnlList[i] ==
				    ucPriChannel)
					break;
			}
			if ((i > prBssInfo->auc2G_20mReqChnlList[0]) &&
			    (i <= CHNL_LIST_SZ_2G)) {
				prBssInfo->auc2G_20mReqChnlList[i] =
					ucPriChannel;
				prBssInfo->auc2G_20mReqChnlList[0]++;
			}
		}
	} else {
		/* Update non-HT channel list */
		ASSERT(prBssInfo->auc2G_NonHtChnlList[0] <= CHNL_LIST_SZ_2G);
		for (i = 1; i <= prBssInfo->auc2G_NonHtChnlList[0] &&
			    i <= CHNL_LIST_SZ_2G;
		     i++) {
			if (prBssInfo->auc2G_NonHtChnlList[i] == ucPriChannel)
				break;
		}
		if ((i > prBssInfo->auc2G_NonHtChnlList[0]) &&
		    (i <= CHNL_LIST_SZ_2G)) {
			prBssInfo->auc2G_NonHtChnlList[i] = ucPriChannel;
			prBssInfo->auc2G_NonHtChnlList[0]++;
		}
	}

	return FALSE;
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief AIS or P2P GC.
 *
 * \param[in]
 *
 * \return none
 */
/*----------------------------------------------------------------------------*/
static u_int8_t rlmRecBcnInfoForClient(struct ADAPTER *prAdapter,
				       struct BSS_INFO *prBssInfo,
				       struct SW_RFB *prSwRfb, uint8_t *pucIE,
				       uint16_t u2IELength)
{
	/* For checking if syncing params are different from
	 * last syncing and need to sync again
	 */
	struct CMD_SET_BSS_RLM_PARAM rBssRlmParam;
	u_int8_t fgNewParameter = FALSE;

	ASSERT(prAdapter);
	ASSERT(prBssInfo && prSwRfb);
	ASSERT(pucIE);

#if 0 /* SW migration 2010/8/20 */
	/* Note: we shall not update parameters when scanning, otherwise
	 * channel and bandwidth will not be correct or asserted failure
	 * during scanning.
	 * Note: remove channel checking. All received Beacons should be
	 * processed if measurement or other actions are executed in adjacent
	 * channels and Beacon content checking mechanism is not disabled.
	 */
	if (IS_SCAN_ACTIVE()
	    /* || prBssInfo->ucPrimaryChannel != CHNL_NUM_BY_SWRFB(prSwRfb) */
	    ) {
		return FALSE;
	}
#endif

	/* Handle change of slot time */
	prBssInfo->u2CapInfo =
		((struct WLAN_BEACON_FRAME *)(prSwRfb->pvHeader))->u2CapInfo;
	prBssInfo->fgUseShortSlotTime =
		((prBssInfo->u2CapInfo & CAP_INFO_SHORT_SLOT_TIME) ||
		 (prBssInfo->eBand != BAND_2G4))
			? TRUE
			: FALSE;

	/* Check if syncing params are different from last syncing and need to
	 * sync again
	 * If yes, return TRUE and sync with FW; Otherwise, return FALSE.
	 */
	rBssRlmParam.ucRfBand = (u_int8_t)prBssInfo->eBand;
	rBssRlmParam.ucPrimaryChannel = prBssInfo->ucPrimaryChannel;
	rBssRlmParam.ucRfSco = (u_int8_t)prBssInfo->eBssSCO;
	rBssRlmParam.ucErpProtectMode = (u_int8_t)prBssInfo->fgErpProtectMode;
	rBssRlmParam.ucHtProtectMode = (u_int8_t)prBssInfo->eHtProtectMode;
	rBssRlmParam.ucGfOperationMode = (u_int8_t)prBssInfo->eGfOperationMode;
	rBssRlmParam.ucTxRifsMode = (u_int8_t)prBssInfo->eRifsOperationMode;
	rBssRlmParam.u2HtOpInfo3 = prBssInfo->u2HtOpInfo3;
	rBssRlmParam.u2HtOpInfo2 = prBssInfo->u2HtOpInfo2;
	rBssRlmParam.ucHtOpInfo1 = prBssInfo->ucHtOpInfo1;
	rBssRlmParam.ucUseShortPreamble = prBssInfo->fgUseShortPreamble;
	rBssRlmParam.ucUseShortSlotTime = prBssInfo->fgUseShortSlotTime;
	rBssRlmParam.ucVhtChannelWidth = prBssInfo->ucVhtChannelWidth;
	rBssRlmParam.ucVhtChannelFrequencyS1 =
		prBssInfo->ucVhtChannelFrequencyS1;
	rBssRlmParam.ucVhtChannelFrequencyS2 =
		prBssInfo->ucVhtChannelFrequencyS2;
	rBssRlmParam.u2VhtBasicMcsSet = prBssInfo->u2VhtBasicMcsSet;
	rBssRlmParam.ucRxNss = prBssInfo->ucOpRxNss;
	rBssRlmParam.ucTxNss = prBssInfo->ucOpTxNss;

	rlmRecIeInfoForClient(prAdapter, prBssInfo, pucIE, u2IELength);

	if (rBssRlmParam.ucRfBand != prBssInfo->eBand ||
		rBssRlmParam.ucPrimaryChannel != prBssInfo->ucPrimaryChannel ||
		rBssRlmParam.ucRfSco != prBssInfo->eBssSCO ||
		rBssRlmParam.ucErpProtectMode != prBssInfo->fgErpProtectMode ||
		rBssRlmParam.ucHtProtectMode != prBssInfo->eHtProtectMode ||
		rBssRlmParam.ucGfOperationMode != prBssInfo->eGfOperationMode ||
		rBssRlmParam.ucTxRifsMode != prBssInfo->eRifsOperationMode ||
		rBssRlmParam.u2HtOpInfo3 != prBssInfo->u2HtOpInfo3 ||
		rBssRlmParam.u2HtOpInfo2 != prBssInfo->u2HtOpInfo2 ||
		rBssRlmParam.ucHtOpInfo1 != prBssInfo->ucHtOpInfo1 ||
		rBssRlmParam.ucUseShortPreamble !=
			prBssInfo->fgUseShortPreamble ||
		rBssRlmParam.ucUseShortSlotTime !=
			prBssInfo->fgUseShortSlotTime ||
		rBssRlmParam.ucVhtChannelWidth !=
			prBssInfo->ucVhtChannelWidth ||
		rBssRlmParam.ucVhtChannelFrequencyS1 !=
			prBssInfo->ucVhtChannelFrequencyS1 ||
		rBssRlmParam.ucVhtChannelFrequencyS2 !=
			prBssInfo->ucVhtChannelFrequencyS2 ||
		rBssRlmParam.u2VhtBasicMcsSet != prBssInfo->u2VhtBasicMcsSet ||
		rBssRlmParam.ucRxNss != prBssInfo->ucOpRxNss ||
		rBssRlmParam.ucTxNss != prBssInfo->ucOpTxNss)
		fgNewParameter = TRUE;
	else {
		DBGLOG(RLM, TRACE,
		       "prBssInfo's params are all the same! not to sync!\n");
		fgNewParameter = FALSE;
	}

	return fgNewParameter;
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief
 *
 * \param[in]
 *
 * \return none
 */
/*----------------------------------------------------------------------------*/
void rlmProcessBcn(struct ADAPTER *prAdapter, struct SW_RFB *prSwRfb,
		   uint8_t *pucIE, uint16_t u2IELength)
{
	struct BSS_INFO *prBssInfo;
	u_int8_t fgNewParameter;
#if (CFG_SUPPORT_802_11AX == 1)
	u_int8_t fgNewSRParam = FALSE;
#endif
	uint8_t i;

	ASSERT(prAdapter);
	ASSERT(prSwRfb);
	ASSERT(pucIE);

	fgNewParameter = FALSE;

	/* When concurrent networks exist, GO shall have the same handle as
	 * the other BSS, so the Beacon shall be processed for bandwidth and
	 * protection mechanism.
	 * Note1: we do not have 2 AP (GO) cases simultaneously now.
	 * Note2: If we are GO, concurrent AIS AP should detect it and reflect
	 *        action in its Beacon, so AIS STA just follows Beacon from AP.
	 */
	for (i = 0; i < prAdapter->ucHwBssIdNum; i++) {
		prBssInfo = prAdapter->aprBssInfo[i];

		if (IS_BSS_BOW(prBssInfo))
			continue;

		if (IS_BSS_ACTIVE(prBssInfo)) {
			if (prBssInfo->eCurrentOPMode ==
				    OP_MODE_INFRASTRUCTURE &&
			    prBssInfo->eConnectionState ==
				    MEDIA_STATE_CONNECTED) {
				/* P2P client or AIS infra STA */
				if (EQUAL_MAC_ADDR(
					    prBssInfo->aucBSSID,
					    ((struct WLAN_MAC_MGMT_HEADER
						      *)(prSwRfb->pvHeader))
						    ->aucBSSID)) {

					fgNewParameter = rlmRecBcnInfoForClient(
						prAdapter, prBssInfo, prSwRfb,
						pucIE, u2IELength);
#if (CFG_SUPPORT_802_11AX == 1)
					fgNewSRParam = heRlmRecHeSRParams(
						prAdapter, prBssInfo,
						prSwRfb, pucIE, u2IELength);
#endif
				} else {
					fgNewParameter =
						rlmRecBcnFromNeighborForClient(
							prAdapter, prBssInfo,
							prSwRfb, pucIE,
							u2IELength);
				}
			}
#if CFG_ENABLE_WIFI_DIRECT
			else if (prAdapter->fgIsP2PRegistered &&
				 (prBssInfo->eCurrentOPMode ==
					  OP_MODE_ACCESS_POINT ||
				  prBssInfo->eCurrentOPMode ==
					  OP_MODE_P2P_DEVICE)) {
				/* AP scan to check if 20/40M bandwidth is
				 * permitted
				 */
				rlmRecBcnFromNeighborForClient(
					prAdapter, prBssInfo, prSwRfb, pucIE,
					u2IELength);
			}
#endif
			else if (prBssInfo->eCurrentOPMode == OP_MODE_IBSS) {
				/* To do: Nothing */
				/* To do: Ad-hoc */
			}

			/* Appy new parameters if necessary */
			if (fgNewParameter) {
				rlmSyncOperationParams(prAdapter, prBssInfo);
				fgNewParameter = FALSE;
			}
#if (CFG_SUPPORT_802_11AX == 1)
			if (fgNewSRParam) {
				nicRlmUpdateSRParams(prAdapter,
					prBssInfo->ucBssIndex);
				fgNewSRParam = FALSE;
			}
#endif

		} /* end of IS_BSS_ACTIVE() */
	}
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief This function should be invoked after judging successful association.
 *
 * \param[in]
 *
 * \return none
 */
/*----------------------------------------------------------------------------*/
void rlmProcessAssocRsp(struct ADAPTER *prAdapter, struct SW_RFB *prSwRfb,
			uint8_t *pucIE, uint16_t u2IELength)
{
	struct BSS_INFO *prBssInfo;
	struct STA_RECORD *prStaRec;
	uint8_t ucPriChannel;
#if (CFG_SUPPORT_802_11AX == 1)
	uint8_t fgNewSRParam;
#endif

	ASSERT(prAdapter);
	ASSERT(prSwRfb);
	ASSERT(pucIE);

	prStaRec = cnmGetStaRecByIndex(prAdapter, prSwRfb->ucStaRecIdx);
	if (!prStaRec)
		return;

	prBssInfo = GET_BSS_INFO_BY_INDEX(prAdapter, prStaRec->ucBssIndex);
	if (!prBssInfo)
		return;

	if (prStaRec != prBssInfo->prStaRecOfAP)
		return;

	/* To do: the invoked function is used to clear all members. It may be
	 *        done by center mechanism in invoker.
	 */
	rlmBssReset(prAdapter, prBssInfo);

	prBssInfo->fgUseShortSlotTime =
		((prBssInfo->u2CapInfo & CAP_INFO_SHORT_SLOT_TIME) ||
		 (prBssInfo->eBand != BAND_2G4))
			? TRUE
			: FALSE;
	ucPriChannel =
		rlmRecIeInfoForClient(prAdapter, prBssInfo, pucIE, u2IELength);

	/*Update the parameters from Association Response only,
	 *if the parameters need to be updated by both Beacon and Association
	 *Response,
	 *user should use another function, rlmRecIeInfoForClient()
	 */
	rlmRecAssocRespIeInfoForClient(prAdapter, prBssInfo, pucIE, u2IELength);

	if (prBssInfo->ucPrimaryChannel != ucPriChannel) {
		DBGLOG(RLM, INFO,
		       "Use RF pri channel[%u].Pri channel in HT OP IE is :[%u]\n",
		       prBssInfo->ucPrimaryChannel, ucPriChannel);
	}
	/* Avoid wrong primary channel info in HT operation
	 * IE info when accept association response
	 */
#if 0
	if (ucPriChannel > 0)
		prBssInfo->ucPrimaryChannel = ucPriChannel;
#endif

	if (!RLM_NET_IS_11N(prBssInfo) ||
	    !(prStaRec->u2HtCapInfo & HT_CAP_INFO_SUP_CHNL_WIDTH))
		prBssInfo->fg40mBwAllowed = FALSE;

#if (CFG_SUPPORT_802_11AX == 1)
	fgNewSRParam = heRlmRecHeSRParams(prAdapter, prBssInfo,
					prSwRfb, pucIE, u2IELength);
	/* ASSERT(fgNewSRParam); */
	nicRlmUpdateSRParams(prAdapter, prBssInfo->ucBssIndex);
#endif

	/* Note: Update its capabilities to WTBL by cnmStaRecChangeState(),
	 * which
	 *       shall be invoked afterwards.
	 *       Update channel, bandwidth and protection mode by nicUpdateBss()
	 */
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief
 *
 * \param[in]
 *
 * \return none
 */
/*----------------------------------------------------------------------------*/
void rlmProcessHtAction(struct ADAPTER *prAdapter, struct SW_RFB *prSwRfb)
{
	struct ACTION_NOTIFY_CHNL_WIDTH_FRAME *prRxFrame;
	struct ACTION_SM_POWER_SAVE_FRAME *prRxSmpsFrame;
	struct STA_RECORD *prStaRec;
	struct BSS_INFO *prBssInfo;
	uint16_t u2HtCapInfoBitmask = 0;

	ASSERT(prAdapter);
	ASSERT(prSwRfb);

	prRxFrame = (struct ACTION_NOTIFY_CHNL_WIDTH_FRAME *)prSwRfb->pvHeader;
	prStaRec = cnmGetStaRecByIndex(prAdapter, prSwRfb->ucStaRecIdx);

	if (!prStaRec)
		return;

	switch (prRxFrame->ucAction) {
	case ACTION_HT_NOTIFY_CHANNEL_WIDTH:
		if (prStaRec->ucStaState != STA_STATE_3 ||
		    prSwRfb->u2PacketLen <
			    sizeof(struct ACTION_NOTIFY_CHNL_WIDTH_FRAME)) {
			return;
		}

		/* To do: depending regulation class 13 and 14 based on spec
		 * Note: (ucChannelWidth==1) shall restored back to original
		 * capability, not current setting to 40MHz BW here
		 */
		/* 1. Update StaRec for AP/STA mode */
		if (prRxFrame->ucChannelWidth == HT_NOTIFY_CHANNEL_WIDTH_20)
			prStaRec->u2HtCapInfo &= ~HT_CAP_INFO_SUP_CHNL_WIDTH;
		else if (prRxFrame->ucChannelWidth ==
			 HT_NOTIFY_CHANNEL_WIDTH_ANY_SUPPORT_CAHNNAEL_WIDTH)
			prStaRec->u2HtCapInfo |= HT_CAP_INFO_SUP_CHNL_WIDTH;

		cnmStaSendUpdateCmd(prAdapter, prStaRec, FALSE);

		/* 2. Update BssInfo for STA mode */
		prBssInfo = prAdapter->aprBssInfo[prStaRec->ucBssIndex];
		if (prBssInfo->eCurrentOPMode == OP_MODE_INFRASTRUCTURE) {
			if (prRxFrame->ucChannelWidth ==
			    HT_NOTIFY_CHANNEL_WIDTH_20) {
				prBssInfo->ucHtOpInfo1 &=
					~HT_OP_INFO1_STA_CHNL_WIDTH;
				prBssInfo->eBssSCO = CHNL_EXT_SCN;
			} else if (
				prRxFrame->ucChannelWidth ==
	HT_NOTIFY_CHANNEL_WIDTH_ANY_SUPPORT_CAHNNAEL_WIDTH)
				prBssInfo->ucHtOpInfo1 |=
					HT_OP_INFO1_STA_CHNL_WIDTH;

			/* Revise by own OP BW if needed */
			if (prBssInfo->fgIsOpChangeChannelWidth &&
			    prBssInfo->ucOpChangeChannelWidth == MAX_BW_20MHZ) {
				prBssInfo->ucHtOpInfo1 &=
					~(HT_OP_INFO1_SCO |
					  HT_OP_INFO1_STA_CHNL_WIDTH);
				prBssInfo->eBssSCO = CHNL_EXT_SCN;
			}

			/* 3. Update OP BW to FW */
			rlmSyncOperationParams(prAdapter, prBssInfo);
		}
		break;
		/* Support SM power save */ /* TH3_Huang */
	case ACTION_HT_SM_POWER_SAVE:
		prRxSmpsFrame =
			(struct ACTION_SM_POWER_SAVE_FRAME *)prSwRfb->pvHeader;
		if (prStaRec->ucStaState != STA_STATE_3 ||
		    prSwRfb->u2PacketLen <
			    sizeof(struct ACTION_SM_POWER_SAVE_FRAME)) {
			return;
		}

		/* The SM power enable bit is different definition in HtCap and
		 * SMpower IE field
		 */
		if (!(prRxSmpsFrame->ucSmPowerCtrl &
		      (HT_SM_POWER_SAVE_CONTROL_ENABLED |
		       HT_SM_POWER_SAVE_CONTROL_SM_MODE)))
			u2HtCapInfoBitmask |= HT_CAP_INFO_SM_POWER_SAVE;

		/* Support SMPS action frame, TH3_Huang */
		/* Update StaRec if SM power state changed */
		if ((prStaRec->u2HtCapInfo & HT_CAP_INFO_SM_POWER_SAVE) !=
		    u2HtCapInfoBitmask) {
			prStaRec->u2HtCapInfo &= ~HT_CAP_INFO_SM_POWER_SAVE;
			prStaRec->u2HtCapInfo |= u2HtCapInfoBitmask;
			DBGLOG(RLM, INFO,
			       "rlmProcessHtAction -- SMPS change u2HtCapInfo to (%x)\n",
			       prStaRec->u2HtCapInfo);
			cnmStaSendUpdateCmd(prAdapter, prStaRec, FALSE);
		}
		break;
	default:
		break;
	}
}

#if CFG_SUPPORT_802_11AC
/*----------------------------------------------------------------------------*/
/*!
 * \brief
 *
 * \param[in]
 *
 * \return none
 */
/*----------------------------------------------------------------------------*/
void rlmProcessVhtAction(struct ADAPTER *prAdapter, struct SW_RFB *prSwRfb)
{
	struct ACTION_OP_MODE_NOTIFICATION_FRAME *prRxFrame;
	struct STA_RECORD *prStaRec;
	struct BSS_INFO *prBssInfo;
	uint8_t ucVhtOpModeChannelWidth = 0;

	ASSERT(prAdapter);
	ASSERT(prSwRfb);

	prRxFrame =
		(struct ACTION_OP_MODE_NOTIFICATION_FRAME *)prSwRfb->pvHeader;
	prStaRec = cnmGetStaRecByIndex(prAdapter, prSwRfb->ucStaRecIdx);

	if (!prStaRec)
		return;

	prBssInfo = GET_BSS_INFO_BY_INDEX(prAdapter, prStaRec->ucBssIndex);

	if (!prBssInfo)
		return;

	switch (prRxFrame->ucAction) {
	/* Support Operating mode notification action frame, TH3_Huang */
	case ACTION_OPERATING_MODE_NOTIFICATION:
		if (prStaRec->ucStaState != STA_STATE_3 ||
		    prSwRfb->u2PacketLen <
			    sizeof(struct ACTION_OP_MODE_NOTIFICATION_FRAME)) {
			return;
		}

		if (((prRxFrame->ucOperatingMode & VHT_OP_MODE_RX_NSS_TYPE) !=
		     VHT_OP_MODE_RX_NSS_TYPE) &&
		    (prStaRec->ucVhtOpMode != prRxFrame->ucOperatingMode)) {
			/* 1. Fill OP mode notification info */
			prStaRec->ucVhtOpMode = prRxFrame->ucOperatingMode;
			DBGLOG(RLM, INFO,
			       "rlmProcessVhtAction -- Update ucVhtOpMode to 0x%x\n",
			       prStaRec->ucVhtOpMode);

			/* 2. Modify channel width parameters */
			ucVhtOpModeChannelWidth = prRxFrame->ucOperatingMode &
						  VHT_OP_MODE_CHANNEL_WIDTH;
			if (prBssInfo->eCurrentOPMode == OP_MODE_ACCESS_POINT) {
				if (ucVhtOpModeChannelWidth ==
				    VHT_OP_MODE_CHANNEL_WIDTH_20)
					prStaRec->u2HtCapInfo &=
						~HT_CAP_INFO_SUP_CHNL_WIDTH;
				else /* for other 3 VHT cases: 40/80/160 */
					prStaRec->u2HtCapInfo |=
						HT_CAP_INFO_SUP_CHNL_WIDTH;
			} else if (prBssInfo->eCurrentOPMode ==
				   OP_MODE_INFRASTRUCTURE)
				rlmRecOpModeBwForClient(ucVhtOpModeChannelWidth,
							prBssInfo);

			/* 3. Update StaRec to FW */
			/* As defined in spec, 11 means not support this MCS */
			if (((prRxFrame->ucOperatingMode & VHT_OP_MODE_RX_NSS)
				>> VHT_OP_MODE_RX_NSS_OFFSET) ==
				VHT_OP_MODE_NSS_2) {
				prStaRec->u2VhtRxMcsMap = BITS(0, 15) &
					(~(VHT_CAP_INFO_MCS_1SS_MASK |
					VHT_CAP_INFO_MCS_2SS_MASK));

				prStaRec->u2VhtRxMcsMap |=
					(prStaRec->u2VhtRxMcsMapAssoc &
					(VHT_CAP_INFO_MCS_1SS_MASK |
					VHT_CAP_INFO_MCS_2SS_MASK));
				DBGLOG(RLM, INFO,
				       "NSS=2 RxMcsMap:0x%x, McsMapAssoc:0x%x\n",
				       prStaRec->u2VhtRxMcsMap,
				       prStaRec->u2VhtRxMcsMapAssoc);

			} else {
				/* NSS = 1 or others */
				prStaRec->u2VhtRxMcsMap = BITS(0, 15) &
					(~VHT_CAP_INFO_MCS_1SS_MASK);

				prStaRec->u2VhtRxMcsMap |=
					(prStaRec->u2VhtRxMcsMapAssoc &
					VHT_CAP_INFO_MCS_1SS_MASK);
				DBGLOG(RLM, INFO,
				       "NSS=1 RxMcsMap:0x%x, McsMapAssoc:0x%x\n",
				       prStaRec->u2VhtRxMcsMap,
				       prStaRec->u2VhtRxMcsMapAssoc);
			}
			cnmStaSendUpdateCmd(prAdapter, prStaRec, FALSE);

			/* 4. Update BW parameters in BssInfo for STA mode only
			 */
			if (prBssInfo->eCurrentOPMode ==
			    OP_MODE_INFRASTRUCTURE) {
				/* 4.1 Revise by own OP BW if needed for STA
				 * mode only
				 */
				if (prBssInfo->fgIsOpChangeChannelWidth) {
					/* VHT */
					if (rlmGetVhtOpBwByBssOpBw(
					prBssInfo
					->ucOpChangeChannelWidth) <
						prBssInfo->ucVhtChannelWidth)
						rlmFillVhtOpInfoByBssOpBw(
						prBssInfo,
						prBssInfo
						->ucOpChangeChannelWidth);
					/* HT */
					if (prBssInfo
					->fgIsOpChangeChannelWidth &&
					    prBssInfo->ucOpChangeChannelWidth ==
						    MAX_BW_20MHZ) {
						prBssInfo->ucHtOpInfo1 &= ~(
						HT_OP_INFO1_SCO |
						HT_OP_INFO1_STA_CHNL_WIDTH);
						prBssInfo->eBssSCO =
							CHNL_EXT_SCN;
					}
				}

				/* 4.2 Check if OP BW parameter valid */
				if (!rlmDomainIsValidRfSetting(
					    prAdapter, prBssInfo->eBand,
					    prBssInfo->ucPrimaryChannel,
					    prBssInfo->eBssSCO,
					    prBssInfo->ucVhtChannelWidth,
					    prBssInfo->ucVhtChannelFrequencyS1,
				prBssInfo->ucVhtChannelFrequencyS2)) {

					DBGLOG(RLM, WARN,
					       "rlmProcessVhtAction invalid RF settings\n");

					/* Error Handling for Non-predicted IE -
					 * Fixed to set 20MHz
					 */
					prBssInfo->ucVhtChannelWidth =
						CW_20_40MHZ;
					prBssInfo->ucVhtChannelFrequencyS1 = 0;
					prBssInfo->ucVhtChannelFrequencyS2 = 0;
					prBssInfo->eBssSCO = CHNL_EXT_SCN;
					prBssInfo->ucHtOpInfo1 &=
						~(HT_OP_INFO1_SCO |
						  HT_OP_INFO1_STA_CHNL_WIDTH);
				}

				/* 4.3 Update BSS OP BW to FW for STA mode only
				 */
				rlmSyncOperationParams(prAdapter, prBssInfo);
			}
		}
		break;
	default:
		break;
	}
}
#endif

/*----------------------------------------------------------------------------*/
/*!
 * \brief This function should be invoked after judging successful association.
 *
 * \param[in]
 *
 * \return none
 */
/*----------------------------------------------------------------------------*/
void rlmFillSyncCmdParam(struct CMD_SET_BSS_RLM_PARAM *prCmdBody,
			 struct BSS_INFO *prBssInfo)
{
	if (!prCmdBody || !prBssInfo)
		return;

	prCmdBody->ucBssIndex = prBssInfo->ucBssIndex;
	prCmdBody->ucRfBand = (uint8_t)prBssInfo->eBand;
	prCmdBody->ucPrimaryChannel = prBssInfo->ucPrimaryChannel;
	prCmdBody->ucRfSco = (uint8_t)prBssInfo->eBssSCO;
	prCmdBody->ucErpProtectMode = (uint8_t)prBssInfo->fgErpProtectMode;
	prCmdBody->ucHtProtectMode = (uint8_t)prBssInfo->eHtProtectMode;
	prCmdBody->ucGfOperationMode = (uint8_t)prBssInfo->eGfOperationMode;
	prCmdBody->ucTxRifsMode = (uint8_t)prBssInfo->eRifsOperationMode;
	prCmdBody->u2HtOpInfo3 = prBssInfo->u2HtOpInfo3;
	prCmdBody->u2HtOpInfo2 = prBssInfo->u2HtOpInfo2;
	prCmdBody->ucHtOpInfo1 = prBssInfo->ucHtOpInfo1;
	prCmdBody->ucUseShortPreamble = prBssInfo->fgUseShortPreamble;
	prCmdBody->ucUseShortSlotTime = prBssInfo->fgUseShortSlotTime;
	prCmdBody->ucVhtChannelWidth = prBssInfo->ucVhtChannelWidth;
	prCmdBody->ucVhtChannelFrequencyS1 = prBssInfo->ucVhtChannelFrequencyS1;
	prCmdBody->ucVhtChannelFrequencyS2 = prBssInfo->ucVhtChannelFrequencyS2;
	prCmdBody->u2VhtBasicMcsSet = prBssInfo->u2BSSBasicRateSet;
	prCmdBody->ucTxNss = prBssInfo->ucOpTxNss;
	prCmdBody->ucRxNss = prBssInfo->ucOpRxNss;

	if (RLM_NET_PARAM_VALID(prBssInfo)) {
		DBGLOG(RLM, INFO,
		       "N=%d b=%d c=%d s=%d e=%d h=%d I=0x%02x l=%d p=%d w=%d s1=%d s2=%d RxN=%d, TxN=%d\n",
		       prCmdBody->ucBssIndex, prCmdBody->ucRfBand,
		       prCmdBody->ucPrimaryChannel, prCmdBody->ucRfSco,
		       prCmdBody->ucErpProtectMode, prCmdBody->ucHtProtectMode,
		       prCmdBody->ucHtOpInfo1, prCmdBody->ucUseShortSlotTime,
		       prCmdBody->ucUseShortPreamble,
		       prCmdBody->ucVhtChannelWidth,
		       prCmdBody->ucVhtChannelFrequencyS1,
		       prCmdBody->ucVhtChannelFrequencyS2,
		       prCmdBody->ucRxNss,
		       prCmdBody->ucTxNss);
	} else {
		DBGLOG(RLM, INFO, "N=%d closed\n", prCmdBody->ucBssIndex);
	}
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief This function will operation parameters based on situations of
 *        concurrent networks. Channel, bandwidth, protection mode, supported
 *        rate will be modified.
 *
 * \param[in]
 *
 * \return none
 */
/*----------------------------------------------------------------------------*/
void rlmSyncOperationParams(struct ADAPTER *prAdapter,
			    struct BSS_INFO *prBssInfo)
{
	struct CMD_SET_BSS_RLM_PARAM *prCmdBody;
	uint32_t rStatus;

	ASSERT(prAdapter);
	ASSERT(prBssInfo);

	prCmdBody = (struct CMD_SET_BSS_RLM_PARAM *)cnmMemAlloc(
		prAdapter, RAM_TYPE_BUF, sizeof(struct CMD_SET_BSS_RLM_PARAM));

	/* ASSERT(prCmdBody); */
	/* To do: exception handle */
	if (!prCmdBody) {
		DBGLOG(RLM, WARN, "No buf for sync RLM params (Net=%d)\n",
		       prBssInfo->ucBssIndex);
		return;
	}

	rlmFillSyncCmdParam(prCmdBody, prBssInfo);

	rStatus = wlanSendSetQueryCmd(
		prAdapter,			      /* prAdapter */
		CMD_ID_SET_BSS_RLM_PARAM,	     /* ucCID */
		TRUE,				      /* fgSetQuery */
		FALSE,				      /* fgNeedResp */
		FALSE,				      /* fgIsOid */
		NULL,				      /* pfCmdDoneHandler */
		NULL,				      /* pfCmdTimeoutHandler */
		sizeof(struct CMD_SET_BSS_RLM_PARAM), /* u4SetQueryInfoLen */
		(uint8_t *)prCmdBody,		      /* pucInfoBuffer */
		NULL,				      /* pvSetQueryBuffer */
		0				      /* u4SetQueryBufferLen */
		);

	/* ASSERT(rStatus == WLAN_STATUS_PENDING); */
	if (rStatus != WLAN_STATUS_PENDING)
		DBGLOG(RLM, WARN, "rlmSyncOperationParams set cmd fail\n");

	cnmMemFree(prAdapter, prCmdBody);
}

#if CFG_SUPPORT_AAA
/*----------------------------------------------------------------------------*/
/*!
 * \brief This function should be invoked after judging successful association.
 *
 * \param[in]
 *
 * \return none
 */
/*----------------------------------------------------------------------------*/
void rlmProcessAssocReq(struct ADAPTER *prAdapter, struct SW_RFB *prSwRfb,
			uint8_t *pucIE, uint16_t u2IELength)
{
	struct BSS_INFO *prBssInfo;
	struct STA_RECORD *prStaRec;
	uint16_t u2Offset;
	struct IE_HT_CAP *prHtCap;
#if CFG_SUPPORT_802_11AC
	struct IE_VHT_CAP *prVhtCap = NULL;
	struct IE_OP_MODE_NOTIFICATION
		*prOPModeNotification; /* Operation Mode Notification */
	u_int8_t fgHasOPModeIE = FALSE;
#endif

	ASSERT(prAdapter);
	ASSERT(prSwRfb);
	ASSERT(pucIE);

	prStaRec = cnmGetStaRecByIndex(prAdapter, prSwRfb->ucStaRecIdx);
	if (!prStaRec)
		return;
	ASSERT(prStaRec->ucBssIndex <= prAdapter->ucHwBssIdNum);

	prBssInfo = prAdapter->aprBssInfo[prStaRec->ucBssIndex];

	IE_FOR_EACH(pucIE, u2IELength, u2Offset)
	{
		switch (IE_ID(pucIE)) {
		case ELEM_ID_HT_CAP:
			if (!RLM_NET_IS_11N(prBssInfo) ||
			    IE_LEN(pucIE) != (sizeof(struct IE_HT_CAP) - 2))
				break;
			prHtCap = (struct IE_HT_CAP *)pucIE;
			prStaRec->ucMcsSet =
				prHtCap->rSupMcsSet.aucRxMcsBitmask[0];
			prStaRec->fgSupMcs32 =
				(prHtCap->rSupMcsSet.aucRxMcsBitmask[32 / 8] &
				 BIT(0))
					? TRUE
					: FALSE;

			kalMemCopy(
				prStaRec->aucRxMcsBitmask,
				prHtCap->rSupMcsSet.aucRxMcsBitmask,
				/*SUP_MCS_RX_BITMASK_OCTET_NUM */
				sizeof(prStaRec->aucRxMcsBitmask));

			prStaRec->u2HtCapInfo = prHtCap->u2HtCapInfo;

			/* Set Short LDPC Tx capability */
			if (IS_FEATURE_FORCE_ENABLED(
				    prAdapter->rWifiVar.ucTxLdpc))
				prStaRec->u2HtCapInfo |= HT_CAP_INFO_LDPC_CAP;
			else if (IS_FEATURE_DISABLED(
					 prAdapter->rWifiVar.ucTxLdpc))
				prStaRec->u2HtCapInfo &= ~HT_CAP_INFO_LDPC_CAP;

			/* Set STBC Tx capability */
			if (IS_FEATURE_FORCE_ENABLED(
				    prAdapter->rWifiVar.ucTxStbc))
				prStaRec->u2HtCapInfo |= HT_CAP_INFO_TX_STBC;
			else if (IS_FEATURE_DISABLED(
					 prAdapter->rWifiVar.ucTxStbc))
				prStaRec->u2HtCapInfo &= ~HT_CAP_INFO_TX_STBC;
			/* Set Short GI Tx capability */
			if (IS_FEATURE_FORCE_ENABLED(
				    prAdapter->rWifiVar.ucTxShortGI)) {
				prStaRec->u2HtCapInfo |=
					HT_CAP_INFO_SHORT_GI_20M;
				prStaRec->u2HtCapInfo |=
					HT_CAP_INFO_SHORT_GI_40M;
			} else if (IS_FEATURE_DISABLED(
					   prAdapter->rWifiVar.ucTxShortGI)) {
				prStaRec->u2HtCapInfo &=
					~HT_CAP_INFO_SHORT_GI_20M;
				prStaRec->u2HtCapInfo &=
					~HT_CAP_INFO_SHORT_GI_40M;
			}

			/* Set HT Greenfield Tx capability */
			if (IS_FEATURE_FORCE_ENABLED(
				    prAdapter->rWifiVar.ucTxGf))
				prStaRec->u2HtCapInfo |= HT_CAP_INFO_HT_GF;
			else if (IS_FEATURE_DISABLED(
					 prAdapter->rWifiVar.ucTxGf))
				prStaRec->u2HtCapInfo &= ~HT_CAP_INFO_HT_GF;

			prStaRec->ucAmpduParam = prHtCap->ucAmpduParam;
			prStaRec->u2HtExtendedCap = prHtCap->u2HtExtendedCap;
			prStaRec->u4TxBeamformingCap =
				prHtCap->u4TxBeamformingCap;
			prStaRec->ucAselCap = prHtCap->ucAselCap;
			break;

#if CFG_SUPPORT_802_11AC
		case ELEM_ID_VHT_CAP:
			if (!RLM_NET_IS_11AC(prBssInfo) ||
			    IE_LEN(pucIE) != (sizeof(struct IE_VHT_CAP) - 2))
				break;

			prVhtCap = (struct IE_VHT_CAP *)pucIE;

			prStaRec->u4VhtCapInfo = prVhtCap->u4VhtCapInfo;

			/* Set Tx LDPC capability */
			if (IS_FEATURE_FORCE_ENABLED(
				    prAdapter->rWifiVar.ucTxLdpc))
				prStaRec->u4VhtCapInfo |= VHT_CAP_INFO_RX_LDPC;
			else if (IS_FEATURE_DISABLED(
					 prAdapter->rWifiVar.ucTxLdpc))
				prStaRec->u4VhtCapInfo &= ~VHT_CAP_INFO_RX_LDPC;

			/* Set Tx STBC capability */
			if (IS_FEATURE_FORCE_ENABLED(
				    prAdapter->rWifiVar.ucTxStbc))
				prStaRec->u4VhtCapInfo |= VHT_CAP_INFO_TX_STBC;
			else if (IS_FEATURE_DISABLED(
					 prAdapter->rWifiVar.ucTxStbc))
				prStaRec->u4VhtCapInfo &= ~VHT_CAP_INFO_TX_STBC;

			/* Set Tx TXOP PS capability */
			if (IS_FEATURE_FORCE_ENABLED(
				    prAdapter->rWifiVar.ucTxopPsTx))
				prStaRec->u4VhtCapInfo |=
					VHT_CAP_INFO_VHT_TXOP_PS;
			else if (IS_FEATURE_DISABLED(
					 prAdapter->rWifiVar.ucTxopPsTx))
				prStaRec->u4VhtCapInfo &=
					~VHT_CAP_INFO_VHT_TXOP_PS;

			/* Set Tx Short GI capability */
			if (IS_FEATURE_FORCE_ENABLED(
				    prAdapter->rWifiVar.ucTxShortGI)) {
				prStaRec->u4VhtCapInfo |=
					VHT_CAP_INFO_SHORT_GI_80;
				prStaRec->u4VhtCapInfo |=
					VHT_CAP_INFO_SHORT_GI_160_80P80;
			} else if (IS_FEATURE_DISABLED(
					   prAdapter->rWifiVar.ucTxShortGI)) {
				prStaRec->u4VhtCapInfo &=
					~VHT_CAP_INFO_SHORT_GI_80;
				prStaRec->u4VhtCapInfo &=
					~VHT_CAP_INFO_SHORT_GI_160_80P80;
			}

			prStaRec->u2VhtRxMcsMap =
				prVhtCap->rVhtSupportedMcsSet.u2RxMcsMap;
			prStaRec->u2VhtRxMcsMapAssoc =
				prStaRec->u2VhtRxMcsMap;

			prStaRec->u2VhtRxHighestSupportedDataRate =
				prVhtCap->rVhtSupportedMcsSet
					.u2RxHighestSupportedDataRate;
			prStaRec->u2VhtTxMcsMap =
				prVhtCap->rVhtSupportedMcsSet.u2TxMcsMap;
			prStaRec->u2VhtTxHighestSupportedDataRate =
				prVhtCap->rVhtSupportedMcsSet
					.u2TxHighestSupportedDataRate;

			/* Set initial value of VHT OP mode */
			prStaRec->ucVhtOpMode = 0;
			prStaRec->ucVhtOpMode |=
				rlmGetOpModeBwByVhtAndHtOpInfo(prBssInfo);
			prStaRec->ucVhtOpMode |=
				((rlmGetSupportRxNssInVhtCap(prVhtCap) - 1)
				 << VHT_OP_MODE_RX_NSS_OFFSET) &
				VHT_OP_MODE_RX_NSS;

			break;
		case ELEM_ID_OP_MODE:
			if (!RLM_NET_IS_11AC(prBssInfo) ||
			    IE_LEN(pucIE) !=
				    (sizeof(struct IE_OP_MODE_NOTIFICATION) -
				     2))
				break;
			prOPModeNotification =
				(struct IE_OP_MODE_NOTIFICATION *)pucIE;

			if ((prOPModeNotification->ucOpMode &
			     VHT_OP_MODE_RX_NSS_TYPE) !=
			    VHT_OP_MODE_RX_NSS_TYPE) {
				fgHasOPModeIE = TRUE;

				/* Check BIT(2) is BW80P80_160.
				 * Refactor ucOpMode with ucVhtOpMode format.
				 */
				if (prOPModeNotification->ucOpMode &
					VHT_OP_MODE_CHANNEL_WIDTH_80P80_160) {
					prOPModeNotification->ucOpMode &=
					~(VHT_OP_MODE_CHANNEL_WIDTH |
					VHT_OP_MODE_CHANNEL_WIDTH_80P80_160);

					prOPModeNotification->ucOpMode |=
					(VHT_OP_MODE_CHANNEL_WIDTH_160_80P80 &
					VHT_OP_MODE_CHANNEL_WIDTH);
				}
			}

			break;

#endif

#if (CFG_SUPPORT_802_11AX == 1)
		case ELEM_ID_RESERVED:
			if (IE_ID_EXT(pucIE) == ELEM_EXT_ID_HE_CAP)
				heRlmRecHeCapInfo(prAdapter, prStaRec, pucIE);
			break;
#endif

		default:
			break;
		} /* end of switch */
	}	 /* end of IE_FOR_EACH */
#if CFG_SUPPORT_802_11AC
	/*Fill by OP Mode IE after completing parsing all IE to make sure it
	 * won't be overwrite
	 */
	if (fgHasOPModeIE == TRUE)
		prStaRec->ucVhtOpMode = prOPModeNotification->ucOpMode;
#endif
}
#endif /* CFG_SUPPORT_AAA */

/*----------------------------------------------------------------------------*/
/*!
 * \brief It is for both STA and AP modes
 *
 * \param[in]
 *
 * \return none
 */
/*----------------------------------------------------------------------------*/
void rlmBssInitForAPandIbss(struct ADAPTER *prAdapter,
			    struct BSS_INFO *prBssInfo)
{
	ASSERT(prAdapter);
	ASSERT(prBssInfo);

#if CFG_ENABLE_WIFI_DIRECT
	if (prAdapter->fgIsP2PRegistered &&
	    prBssInfo->eCurrentOPMode == OP_MODE_ACCESS_POINT)
		rlmBssInitForAP(prAdapter, prBssInfo);
#endif
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief It is for both STA and AP modes
 *
 * \param[in]
 *
 * \return none
 */
/*----------------------------------------------------------------------------*/
void rlmBssAborted(struct ADAPTER *prAdapter, struct BSS_INFO *prBssInfo)
{
	ASSERT(prAdapter);
	ASSERT(prBssInfo);

	rlmBssReset(prAdapter, prBssInfo);

	prBssInfo->fg40mBwAllowed = FALSE;
	prBssInfo->fgAssoc40mBwAllowed = FALSE;

	/* Assume FW state is updated by CMD_ID_SET_BSS_INFO, so
	 * the sync CMD is not needed here.
	 */
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief All RLM timers will also be stopped.
 *
 * \param[in]
 *
 * \return none
 */
/*----------------------------------------------------------------------------*/
static void rlmBssReset(struct ADAPTER *prAdapter, struct BSS_INFO *prBssInfo)
{
	ASSERT(prAdapter);
	ASSERT(prBssInfo);

	/* HT related parameters */
	prBssInfo->ucHtOpInfo1 = 0; /* RIFS disabled. 20MHz */
	prBssInfo->u2HtOpInfo2 = 0;
	prBssInfo->u2HtOpInfo3 = 0;

#if CFG_SUPPORT_802_11AC
	prBssInfo->ucVhtChannelWidth = 0;       /* VHT_OP_CHANNEL_WIDTH_80; */
	prBssInfo->ucVhtChannelFrequencyS1 = 0; /* 42; */
	prBssInfo->ucVhtChannelFrequencyS2 = 0;
	prBssInfo->u2VhtBasicMcsSet = 0; /* 0xFFFF; */
#endif

	prBssInfo->eBssSCO = 0;
	prBssInfo->fgErpProtectMode = 0;
	prBssInfo->eHtProtectMode = 0;
	prBssInfo->eGfOperationMode = 0;
	prBssInfo->eRifsOperationMode = 0;

	/* OBSS related parameters */
	prBssInfo->auc2G_20mReqChnlList[0] = 0;
	prBssInfo->auc2G_NonHtChnlList[0] = 0;
	prBssInfo->auc2G_PriChnlList[0] = 0;
	prBssInfo->auc2G_SecChnlList[0] = 0;
	prBssInfo->auc5G_20mReqChnlList[0] = 0;
	prBssInfo->auc5G_NonHtChnlList[0] = 0;
	prBssInfo->auc5G_PriChnlList[0] = 0;
	prBssInfo->auc5G_SecChnlList[0] = 0;

	/* All RLM timers will also be stopped */
	cnmTimerStopTimer(prAdapter, &prBssInfo->rObssScanTimer);
	prBssInfo->u2ObssScanInterval = 0;

	prBssInfo->fgObssErpProtectMode = 0;    /* GO only */
	prBssInfo->eObssHtProtectMode = 0;      /* GO only */
	prBssInfo->eObssGfOperationMode = 0;    /* GO only */
	prBssInfo->fgObssRifsOperationMode = 0; /* GO only */
	prBssInfo->fgObssActionForcedTo20M = 0; /* GO only */
	prBssInfo->fgObssBeaconForcedTo20M = 0; /* GO only */

	/* OP mode change control parameters */
	prBssInfo->fgIsOpChangeChannelWidth = FALSE;
	prBssInfo->fgIsOpChangeRxNss = FALSE;
	prBssInfo->fgIsOpChangeTxNss = FALSE;

#if (CFG_SUPPORT_802_11AX == 1)
	/* MU EDCA params */
	prBssInfo->ucMUEdcaUpdateCnt = 0;
	kalMemSet(&prBssInfo->arMUEdcaParams[0], 0,
		sizeof(struct _CMD_MU_EDCA_PARAMS_T) * WMM_AC_INDEX_NUM);

	/* Spatial Reuse params */
	prBssInfo->ucSRControl = 0;
	prBssInfo->ucNonSRGObssPdMaxOffset = 0;
	prBssInfo->ucSRGObssPdMinOffset = 0;
	prBssInfo->ucSRGObssPdMaxOffset = 0;
	prBssInfo->u8SRGBSSColorBitmap = 0;
	prBssInfo->u8SRGPartialBSSIDBitmap = 0;

	/* HE Operation */
	memset(prBssInfo->ucHeOpParams, 0, HE_OP_BYTE_NUM);
#if (CFG_SUPPORT_HE_ER == 1)
	/* set TXOP to 0x3FF (Spec. define default value) */
	prBssInfo->ucHeOpParams[0]
		|= HE_OP_PARAM0_TXOP_DUR_RTS_THRESHOLD_MASK;
	prBssInfo->ucHeOpParams[1]
		|= HE_OP_PARAM1_TXOP_DUR_RTS_THRESHOLD_MASK;
#endif
	prBssInfo->ucBssColorInfo = 0;
	prBssInfo->u2HeBasicMcsSet = 0;
#endif

#ifdef CFG_DFS_CHSW_FORCE_BW20
	g_fgHasChannelSwitchIE = FALSE;
#endif
}

#if CFG_SUPPORT_TDLS
/*----------------------------------------------------------------------------*/
/*!
 * \brief
 *
 * \param[in]
 *
 * \return none
 */
/*----------------------------------------------------------------------------*/
uint32_t rlmFillVhtCapIEByAdapter(struct ADAPTER *prAdapter,
				  struct BSS_INFO *prBssInfo, uint8_t *pOutBuf)
{
	struct IE_VHT_CAP *prVhtCap;
	struct VHT_SUPPORTED_MCS_FIELD *prVhtSupportedMcsSet;
	uint8_t i;

	ASSERT(prAdapter);
	ASSERT(prBssInfo);
	/* ASSERT(prMsduInfo); */

	prVhtCap = (struct IE_VHT_CAP *)pOutBuf;

	prVhtCap->ucId = ELEM_ID_VHT_CAP;
	prVhtCap->ucLength = sizeof(struct IE_VHT_CAP) - ELEM_HDR_LEN;
	prVhtCap->u4VhtCapInfo = VHT_CAP_INFO_DEFAULT_VAL;

	if (IS_FEATURE_ENABLED(prAdapter->rWifiVar.ucRxShortGI))
		prVhtCap->u4VhtCapInfo |= VHT_CAP_INFO_SHORT_GI_80;

	if (IS_FEATURE_ENABLED(prAdapter->rWifiVar.ucRxLdpc))
		prVhtCap->u4VhtCapInfo |= VHT_CAP_INFO_RX_LDPC;

	if (IS_FEATURE_ENABLED(prAdapter->rWifiVar.ucRxStbc))
		prVhtCap->u4VhtCapInfo |= VHT_CAP_INFO_RX_STBC_ONE_STREAM;

	/*set MCS map */
	prVhtSupportedMcsSet = &prVhtCap->rVhtSupportedMcsSet;
	kalMemZero((void *)prVhtSupportedMcsSet,
		   sizeof(struct VHT_SUPPORTED_MCS_FIELD));

	for (i = 0; i < 8; i++) {
		prVhtSupportedMcsSet->u2RxMcsMap |= BITS(2 * i, (2 * i + 1));
		prVhtSupportedMcsSet->u2TxMcsMap |= BITS(2 * i, (2 * i + 1));
	}

	prVhtSupportedMcsSet->u2RxMcsMap &=
		(VHT_CAP_INFO_MCS_MAP_MCS9 << VHT_CAP_INFO_MCS_1SS_OFFSET);
	prVhtSupportedMcsSet->u2TxMcsMap &=
		(VHT_CAP_INFO_MCS_MAP_MCS9 << VHT_CAP_INFO_MCS_1SS_OFFSET);
	prVhtSupportedMcsSet->u2RxHighestSupportedDataRate =
		VHT_CAP_INFO_DEFAULT_HIGHEST_DATA_RATE;
	prVhtSupportedMcsSet->u2TxHighestSupportedDataRate =
		VHT_CAP_INFO_DEFAULT_HIGHEST_DATA_RATE;

	ASSERT(IE_SIZE(prVhtCap) <= (ELEM_HDR_LEN + ELEM_MAX_LEN_VHT_CAP));

	return IE_SIZE(prVhtCap);
}
#endif

#if CFG_SUPPORT_NAN
uint32_t rlmFillNANHTCapIE(struct ADAPTER *prAdapter,
		   struct BSS_INFO *prBssInfo, uint8_t *pOutBuf)
{
	struct IE_HT_CAP *prHtCap;
	struct SUP_MCS_SET_FIELD *prSupMcsSet;
	unsigned char fg40mAllowed = FALSE;
	uint8_t ucIdx;

	if (!prAdapter) {
		DBGLOG(NAN, ERROR, "prAdapter error!\n");
		return 0;
	}
	if (!prBssInfo) {
		DBGLOG(NAN, ERROR, "prBssInfo error!\n");
		return 0;
	}

	if (prAdapter->rWifiVar.ucNanBandwidth >= MAX_BW_40MHZ)
		fg40mAllowed = TRUE;

	prHtCap = (struct IE_HT_CAP *)pOutBuf;

	/* Add HT capabilities IE */
	prHtCap->ucId = ELEM_ID_HT_CAP;
	prHtCap->ucLength = sizeof(struct IE_HT_CAP) - ELEM_HDR_LEN;

	prHtCap->u2HtCapInfo = HT_CAP_INFO_DEFAULT_VAL;

	if (IS_FEATURE_ENABLED(prAdapter->rWifiVar.ucRxShortGI))
		prHtCap->u2HtCapInfo |=
			(HT_CAP_INFO_SHORT_GI_20M | HT_CAP_INFO_SHORT_GI_40M);

	if (IS_FEATURE_ENABLED(prAdapter->rWifiVar.ucRxLdpc))
		prHtCap->u2HtCapInfo |= HT_CAP_INFO_LDPC_CAP;

	if (IS_FEATURE_ENABLED(prAdapter->rWifiVar.ucTxStbc))
		prHtCap->u2HtCapInfo |= HT_CAP_INFO_TX_STBC;

	if (IS_FEATURE_ENABLED(prAdapter->rWifiVar.ucRxStbc)) {
		uint8_t tempRxStbcNss;

		tempRxStbcNss = prAdapter->rWifiVar.ucRxStbcNss;
		tempRxStbcNss =
			(tempRxStbcNss >
			 wlanGetSupportNss(prAdapter, prBssInfo->ucBssIndex))
				? wlanGetSupportNss(prAdapter,
						    prBssInfo->ucBssIndex)
				: (tempRxStbcNss);
		if (tempRxStbcNss != prAdapter->rWifiVar.ucRxStbcNss) {
			DBGLOG(RLM, WARN, "Apply Nss:%d as RxStbcNss in HT Cap",
			       wlanGetSupportNss(prAdapter,
						 prBssInfo->ucBssIndex));
			DBGLOG(RLM, WARN,
			       " due to set RxStbcNss more than Nss is not appropriate.\n");
		}
		if (tempRxStbcNss == 1)
			prHtCap->u2HtCapInfo |= HT_CAP_INFO_RX_STBC_1_SS;
		else if (tempRxStbcNss == 2)
			prHtCap->u2HtCapInfo |= HT_CAP_INFO_RX_STBC_2_SS;
		else if (tempRxStbcNss >= 3)
			prHtCap->u2HtCapInfo |= HT_CAP_INFO_RX_STBC_3_SS;
	}

	if (IS_FEATURE_ENABLED(prAdapter->rWifiVar.ucRxGf))
		prHtCap->u2HtCapInfo |= HT_CAP_INFO_HT_GF;

	if (prAdapter->rWifiVar.ucRxMaxMpduLen > VHT_CAP_INFO_MAX_MPDU_LEN_3K)
		prHtCap->u2HtCapInfo |= HT_CAP_INFO_MAX_AMSDU_LEN;

	if (!fg40mAllowed)
		prHtCap->u2HtCapInfo &= ~(HT_CAP_INFO_SUP_CHNL_WIDTH |
					  HT_CAP_INFO_SHORT_GI_40M |
					  HT_CAP_INFO_DSSS_CCK_IN_40M);

	if (prBssInfo->ucOpRxNss <
	    wlanGetSupportNss(prAdapter, prBssInfo->ucBssIndex))
		prHtCap->u2HtCapInfo &=
			~HT_CAP_INFO_SM_POWER_SAVE; /*Set as static power save*/

	prHtCap->ucAmpduParam = AMPDU_PARAM_DEFAULT_VAL;

	prSupMcsSet = &prHtCap->rSupMcsSet;
	kalMemZero((void *)&prSupMcsSet->aucRxMcsBitmask[0],
		   SUP_MCS_RX_BITMASK_OCTET_NUM);

	for (ucIdx = 0;
	     ucIdx < wlanGetSupportNss(prAdapter, prBssInfo->ucBssIndex);
	     ucIdx++)
		prSupMcsSet->aucRxMcsBitmask[ucIdx] = BITS(0, 7);

	/* prSupMcsSet->aucRxMcsBitmask[0] = BITS(0, 7); */

	if (fg40mAllowed)
	/*whsu:: && IS_FEATURE_ENABLED(prAdapter->rWifiVar.ucMCS32)*/
		prSupMcsSet->aucRxMcsBitmask[32 / 8] = BIT(0); /* MCS32 */
	prSupMcsSet->u2RxHighestSupportedRate = SUP_MCS_RX_DEFAULT_HIGHEST_RATE;
	prSupMcsSet->u4TxRateInfo = SUP_MCS_TX_DEFAULT_VAL;

	prHtCap->u2HtExtendedCap = HT_EXT_CAP_DEFAULT_VAL;
	if (!fg40mAllowed ||
	    prBssInfo->eCurrentOPMode != OP_MODE_INFRASTRUCTURE)
		prHtCap->u2HtExtendedCap &=
			~(HT_EXT_CAP_PCO | HT_EXT_CAP_PCO_TRANS_TIME_NONE);

	prHtCap->u4TxBeamformingCap = TX_BEAMFORMING_CAP_DEFAULT_VAL;

	/* whsu!!
	 * (prAdapter->rWifiVar.ucDbdcMode == DBDC_MODE_DISABLED) ||
	 */
	if (prBssInfo->eBand == BAND_5G) {
		/* HT BFee has IOT issue
		 * only support HT BFee when force mode for testing
		 */
		if (IS_FEATURE_FORCE_ENABLED(prAdapter->rWifiVar.ucStaHtBfee))
			prHtCap->u4TxBeamformingCap = TX_BEAMFORMING_CAP_BFEE;
		if (IS_FEATURE_ENABLED(prAdapter->rWifiVar.ucStaHtBfer))
			prHtCap->u4TxBeamformingCap |= TX_BEAMFORMING_CAP_BFER;
	}

	prHtCap->ucAselCap = ASEL_CAP_DEFAULT_VAL;

	if (IE_SIZE(prHtCap) > (ELEM_HDR_LEN + ELEM_MAX_LEN_HT_CAP)) {
		DBGLOG(NAN, ERROR, "prHtCap size error!\n");
		return 0;
	}

	return IE_SIZE(prHtCap);
}

uint32_t rlmFillNANVHTCapIE(struct ADAPTER *prAdapter,
		   struct BSS_INFO *prBssInfo, uint8_t *pOutBuf)
{
	struct IE_VHT_CAP *prVhtCap;
	struct VHT_SUPPORTED_MCS_FIELD *prVhtSupportedMcsSet;
	uint8_t i;
	uint8_t ucMaxBw;

	if (!prAdapter) {
		DBGLOG(NAN, ERROR, "prAdapter error!\n");
		return 0;
	}
	if (!prBssInfo) {
		DBGLOG(NAN, ERROR, "prBssInfo error!\n");
		return 0;
	}

	prVhtCap = (struct IE_VHT_CAP *)pOutBuf;

	prVhtCap->ucId = ELEM_ID_VHT_CAP;
	prVhtCap->ucLength = sizeof(struct IE_VHT_CAP) - ELEM_HDR_LEN;

	prVhtCap->u4VhtCapInfo = VHT_CAP_INFO_DEFAULT_VAL;

	/* ucMaxBw = cnmGetBssMaxBw(prAdapter, prBssInfo->ucBssIndex); */
	ucMaxBw = prAdapter->rWifiVar.ucNanBandwidth;

	prVhtCap->u4VhtCapInfo |= (prAdapter->rWifiVar.ucRxMaxMpduLen &
				   VHT_CAP_INFO_MAX_MPDU_LEN_MASK);

	if (ucMaxBw == MAX_BW_160MHZ)
		prVhtCap->u4VhtCapInfo |=
			VHT_CAP_INFO_MAX_SUP_CHANNEL_WIDTH_SET_160;
	else if (ucMaxBw == MAX_BW_80_80_MHZ)
		prVhtCap->u4VhtCapInfo |=
			VHT_CAP_INFO_MAX_SUP_CHANNEL_WIDTH_SET_160_80P80;

	/* Fixme: to support BF for NAN */

	if (IS_FEATURE_ENABLED(prAdapter->rWifiVar.ucRxShortGI)) {
		prVhtCap->u4VhtCapInfo |= VHT_CAP_INFO_SHORT_GI_80;

		if (ucMaxBw >= MAX_BW_160MHZ)
			prVhtCap->u4VhtCapInfo |=
				VHT_CAP_INFO_SHORT_GI_160_80P80;
	}

	if (IS_FEATURE_ENABLED(prAdapter->rWifiVar.ucRxLdpc))
		prVhtCap->u4VhtCapInfo |= VHT_CAP_INFO_RX_LDPC;

	if (IS_FEATURE_ENABLED(prAdapter->rWifiVar.ucRxStbc)) {
		uint8_t tempRxStbcNss;

		if (prAdapter->rWifiVar.u4SwTestMode ==
		    ENUM_SW_TEST_MODE_SIGMA_AC) {
			tempRxStbcNss = 1;
			DBGLOG(RLM, INFO,
			       "Set RxStbcNss to 1 for 11ac certification.\n");
		} else {
			tempRxStbcNss = prAdapter->rWifiVar.ucRxStbcNss;
			tempRxStbcNss =
				(tempRxStbcNss >
				 wlanGetSupportNss(prAdapter,
						   prBssInfo->ucBssIndex))
					? wlanGetSupportNss(
						  prAdapter,
						  prBssInfo->ucBssIndex)
					: (tempRxStbcNss);
			if (tempRxStbcNss != prAdapter->rWifiVar.ucRxStbcNss) {
				DBGLOG(RLM, WARN,
				       "Apply Nss:%d as RxStbcNss in VHT Cap",
				       wlanGetSupportNss(
					       prAdapter,
					       prBssInfo->ucBssIndex));
				DBGLOG(RLM, WARN,
				       "due to set RxStbcNss more than Nss is not appropriate.\n");
			}
		}
		prVhtCap->u4VhtCapInfo |=
			((tempRxStbcNss << VHT_CAP_INFO_RX_STBC_OFFSET) &
			 VHT_CAP_INFO_RX_STBC_MASK);
	}

	if (IS_FEATURE_ENABLED(prAdapter->rWifiVar.ucTxStbc))
		prVhtCap->u4VhtCapInfo |= VHT_CAP_INFO_TX_STBC;

	/* set MCS map */
	prVhtSupportedMcsSet = &prVhtCap->rVhtSupportedMcsSet;
	kalMemZero((void *)prVhtSupportedMcsSet,
		   sizeof(struct VHT_SUPPORTED_MCS_FIELD));

	for (i = 0; i < 8; i++) {
		uint8_t ucOffset = i * 2;
		uint8_t ucMcsMap;

		if (i < wlanGetSupportNss(prAdapter, prBssInfo->ucBssIndex))
			ucMcsMap = VHT_CAP_INFO_MCS_MAP_MCS9;
		else
			ucMcsMap = VHT_CAP_INFO_MCS_NOT_SUPPORTED;

		prVhtSupportedMcsSet->u2RxMcsMap |= (ucMcsMap << ucOffset);
		prVhtSupportedMcsSet->u2TxMcsMap |= (ucMcsMap << ucOffset);
	}

	prVhtSupportedMcsSet->u2RxHighestSupportedDataRate =
		VHT_CAP_INFO_DEFAULT_HIGHEST_DATA_RATE;
	prVhtSupportedMcsSet->u2TxHighestSupportedDataRate =
		VHT_CAP_INFO_DEFAULT_HIGHEST_DATA_RATE;

	if (IE_SIZE(prVhtCap) > (ELEM_HDR_LEN + ELEM_MAX_LEN_VHT_CAP)) {
		DBGLOG(NAN, ERROR, "prVhtCap size error!\n");
		return 0;
	}

	return IE_SIZE(prVhtCap);
}
#endif

#if CFG_SUPPORT_TDLS
/*----------------------------------------------------------------------------*/
/*!
 * \brief
 *
 * \param[in]
 *
 * \return none
 */
/*----------------------------------------------------------------------------*/
uint32_t rlmFillHtCapIEByParams(u_int8_t fg40mAllowed,
				u_int8_t fgShortGIDisabled,
				uint8_t u8SupportRxSgi20,
				uint8_t u8SupportRxSgi40, uint8_t u8SupportRxGf,
				enum ENUM_OP_MODE eCurrentOPMode,
				uint8_t *pOutBuf)
{
	struct IE_HT_CAP *prHtCap;
	struct SUP_MCS_SET_FIELD *prSupMcsSet;

	ASSERT(pOutBuf);

	prHtCap = (struct IE_HT_CAP *)pOutBuf;

	/* Add HT capabilities IE */
	prHtCap->ucId = ELEM_ID_HT_CAP;
	prHtCap->ucLength = sizeof(struct IE_HT_CAP) - ELEM_HDR_LEN;

	prHtCap->u2HtCapInfo = HT_CAP_INFO_DEFAULT_VAL;
	if (!fg40mAllowed) {
		prHtCap->u2HtCapInfo &= ~(HT_CAP_INFO_SUP_CHNL_WIDTH |
					  HT_CAP_INFO_SHORT_GI_40M |
					  HT_CAP_INFO_DSSS_CCK_IN_40M);
	}
	if (fgShortGIDisabled)
		prHtCap->u2HtCapInfo &=
			~(HT_CAP_INFO_SHORT_GI_20M | HT_CAP_INFO_SHORT_GI_40M);

	if (u8SupportRxSgi20 == 2)
		prHtCap->u2HtCapInfo &= ~(HT_CAP_INFO_SHORT_GI_20M);
	if (u8SupportRxSgi40 == 2)
		prHtCap->u2HtCapInfo &= ~(HT_CAP_INFO_SHORT_GI_40M);
	if (u8SupportRxGf == 2)
		prHtCap->u2HtCapInfo &= ~(HT_CAP_INFO_HT_GF);

	prHtCap->ucAmpduParam = AMPDU_PARAM_DEFAULT_VAL;

	prSupMcsSet = &prHtCap->rSupMcsSet;
	kalMemZero((void *)&prSupMcsSet->aucRxMcsBitmask[0],
		   SUP_MCS_RX_BITMASK_OCTET_NUM);

	prSupMcsSet->aucRxMcsBitmask[0] = BITS(0, 7);

	if (fg40mAllowed)
		prSupMcsSet->aucRxMcsBitmask[32 / 8] = BIT(0); /* MCS32 */
	prSupMcsSet->u2RxHighestSupportedRate = SUP_MCS_RX_DEFAULT_HIGHEST_RATE;
	prSupMcsSet->u4TxRateInfo = SUP_MCS_TX_DEFAULT_VAL;

	prHtCap->u2HtExtendedCap = HT_EXT_CAP_DEFAULT_VAL;
	if (!fg40mAllowed || eCurrentOPMode != OP_MODE_INFRASTRUCTURE)
		prHtCap->u2HtExtendedCap &=
			~(HT_EXT_CAP_PCO | HT_EXT_CAP_PCO_TRANS_TIME_NONE);

	prHtCap->u4TxBeamformingCap = TX_BEAMFORMING_CAP_DEFAULT_VAL;

	prHtCap->ucAselCap = ASEL_CAP_DEFAULT_VAL;

	ASSERT(IE_SIZE(prHtCap) <= (ELEM_HDR_LEN + ELEM_MAX_LEN_HT_CAP));

	return IE_SIZE(prHtCap);
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief
 *
 * \param[in]
 *
 * \return none
 */
/*----------------------------------------------------------------------------*/
uint32_t rlmFillHtCapIEByAdapter(struct ADAPTER *prAdapter,
				 struct BSS_INFO *prBssInfo, uint8_t *pOutBuf)
{
	struct IE_HT_CAP *prHtCap;
	struct SUP_MCS_SET_FIELD *prSupMcsSet;
	u_int8_t fg40mAllowed;

	ASSERT(prAdapter);
	ASSERT(prBssInfo);
	ASSERT(pOutBuf);

	fg40mAllowed = prBssInfo->fgAssoc40mBwAllowed;

	prHtCap = (struct IE_HT_CAP *)pOutBuf;

	/* Add HT capabilities IE */
	prHtCap->ucId = ELEM_ID_HT_CAP;
	prHtCap->ucLength = sizeof(struct IE_HT_CAP) - ELEM_HDR_LEN;

	prHtCap->u2HtCapInfo = HT_CAP_INFO_DEFAULT_VAL;
	if (!fg40mAllowed) {
		prHtCap->u2HtCapInfo &= ~(HT_CAP_INFO_SUP_CHNL_WIDTH |
					  HT_CAP_INFO_SHORT_GI_40M |
					  HT_CAP_INFO_DSSS_CCK_IN_40M);
	}
	if (IS_FEATURE_ENABLED(prAdapter->rWifiVar.ucRxShortGI))
		prHtCap->u2HtCapInfo |=
			(HT_CAP_INFO_SHORT_GI_20M | HT_CAP_INFO_SHORT_GI_40M);

	if (IS_FEATURE_ENABLED(prAdapter->rWifiVar.ucRxLdpc))
		prHtCap->u2HtCapInfo |= HT_CAP_INFO_LDPC_CAP;

	if (IS_FEATURE_ENABLED(prAdapter->rWifiVar.ucRxStbc))
		prHtCap->u2HtCapInfo |= HT_CAP_INFO_RX_STBC_1_SS;

	prHtCap->ucAmpduParam = AMPDU_PARAM_DEFAULT_VAL;

	prSupMcsSet = &prHtCap->rSupMcsSet;
	kalMemZero((void *)&prSupMcsSet->aucRxMcsBitmask[0],
		   SUP_MCS_RX_BITMASK_OCTET_NUM);

	prSupMcsSet->aucRxMcsBitmask[0] = BITS(0, 7);

	if (fg40mAllowed && IS_FEATURE_ENABLED(prAdapter->rWifiVar.ucMCS32))
		prSupMcsSet->aucRxMcsBitmask[32 / 8] = BIT(0); /* MCS32 */
	prSupMcsSet->u2RxHighestSupportedRate = SUP_MCS_RX_DEFAULT_HIGHEST_RATE;
	prSupMcsSet->u4TxRateInfo = SUP_MCS_TX_DEFAULT_VAL;

	prHtCap->u2HtExtendedCap = HT_EXT_CAP_DEFAULT_VAL;
	if (!fg40mAllowed ||
	    prBssInfo->eCurrentOPMode != OP_MODE_INFRASTRUCTURE)
		prHtCap->u2HtExtendedCap &=
			~(HT_EXT_CAP_PCO | HT_EXT_CAP_PCO_TRANS_TIME_NONE);

	prHtCap->u4TxBeamformingCap = TX_BEAMFORMING_CAP_DEFAULT_VAL;

	prHtCap->ucAselCap = ASEL_CAP_DEFAULT_VAL;

	ASSERT(IE_SIZE(prHtCap) <= (ELEM_HDR_LEN + ELEM_MAX_LEN_HT_CAP));

	return IE_SIZE(prHtCap);
}

#endif

#if CFG_SUPPORT_DFS
/*----------------------------------------------------------------------------*/
/*!
 * @brief This function will compose the TPC Report frame.
 *
 * @param[in] prAdapter              Pointer to the Adapter structure.
 * @param[in] prStaRec               Pointer to the STA_RECORD_T.
 *
 * @return (none)
 */
/*----------------------------------------------------------------------------*/
static void tpcComposeReportFrame(IN struct ADAPTER *prAdapter,
				  IN struct STA_RECORD *prStaRec,
				  IN PFN_TX_DONE_HANDLER pfTxDoneHandler)
{
	struct MSDU_INFO *prMsduInfo;
	struct BSS_INFO *prBssInfo;
	struct ACTION_TPC_REPORT_FRAME *prTxFrame;
	uint16_t u2PayloadLen;

	ASSERT(prAdapter);
	ASSERT(prStaRec);

	prBssInfo = &prAdapter->rWifiVar.arBssInfoPool[prStaRec->ucBssIndex];
	ASSERT(prBssInfo);

	prMsduInfo = (struct MSDU_INFO *)cnmMgtPktAlloc(
		prAdapter, MAC_TX_RESERVED_FIELD + PUBLIC_ACTION_MAX_LEN);

	if (!prMsduInfo)
		return;

	prTxFrame = (struct ACTION_TPC_REPORT_FRAME
			     *)((unsigned long)(prMsduInfo->prPacket) +
				MAC_TX_RESERVED_FIELD);

	prTxFrame->u2FrameCtrl = MAC_FRAME_ACTION;

	COPY_MAC_ADDR(prTxFrame->aucDestAddr, prStaRec->aucMacAddr);
	COPY_MAC_ADDR(prTxFrame->aucSrcAddr, prBssInfo->aucOwnMacAddr);
	COPY_MAC_ADDR(prTxFrame->aucBSSID, prBssInfo->aucBSSID);

	prTxFrame->ucCategory = CATEGORY_SPEC_MGT;
	prTxFrame->ucAction = ACTION_TPC_REPORT;

	/* 3 Compose the frame body's frame. */
	prTxFrame->ucDialogToken = prStaRec->ucSmDialogToken;
	prTxFrame->ucElemId = ELEM_ID_TPC_REPORT;
	prTxFrame->ucLength =
		sizeof(prTxFrame->ucLinkMargin) + sizeof(prTxFrame->ucTransPwr);
	prTxFrame->ucTransPwr = prAdapter->u4GetTxPower;
	prTxFrame->ucLinkMargin =
		prAdapter->rLinkQuality.cRssi - (0 - MIN_RCV_PWR);

	u2PayloadLen = ACTION_SM_TPC_REPORT_LEN;

	/* 4 Update information of MSDU_INFO_T */
	TX_SET_MMPDU(prAdapter, prMsduInfo, prStaRec->ucBssIndex,
		     prStaRec->ucIndex, WLAN_MAC_MGMT_HEADER_LEN,
		     WLAN_MAC_MGMT_HEADER_LEN + u2PayloadLen, pfTxDoneHandler,
		     MSDU_RATE_MODE_AUTO);

	DBGLOG(RLM, TRACE, "ucDialogToken %d ucTransPwr %d ucLinkMargin %d\n",
	       prTxFrame->ucDialogToken, prTxFrame->ucTransPwr,
	       prTxFrame->ucLinkMargin);

	/* 4 Enqueue the frame to send this action frame. */
	nicTxEnqueueMsdu(prAdapter, prMsduInfo);

	return;

} /* end of tpcComposeReportFrame() */

/*----------------------------------------------------------------------------*/
/*!
 * @brief This function will compose the Measurement Report frame.
 *
 * @param[in] prAdapter              Pointer to the Adapter structure.
 * @param[in] prStaRec               Pointer to the STA_RECORD_T.
 *
 * @return (none)
 */
/*----------------------------------------------------------------------------*/
static void msmtComposeReportFrame(IN struct ADAPTER *prAdapter,
				   IN struct STA_RECORD *prStaRec,
				   IN PFN_TX_DONE_HANDLER pfTxDoneHandler)
{
	struct MSDU_INFO *prMsduInfo;
	struct BSS_INFO *prBssInfo;
	struct ACTION_SM_REQ_FRAME *prTxFrame;
	struct IE_MEASUREMENT_REPORT *prMeasurementRepIE;
	uint8_t *pucIE;
	uint16_t u2PayloadLen;

	ASSERT(prAdapter);
	ASSERT(prStaRec);

	prBssInfo = &prAdapter->rWifiVar.arBssInfoPool[prStaRec->ucBssIndex];
	ASSERT(prBssInfo);

	prMsduInfo = (struct MSDU_INFO *)cnmMgtPktAlloc(
		prAdapter, MAC_TX_RESERVED_FIELD + PUBLIC_ACTION_MAX_LEN);

	if (!prMsduInfo)
		return;

	prTxFrame = (struct ACTION_SM_REQ_FRAME
			     *)((unsigned long)(prMsduInfo->prPacket) +
				MAC_TX_RESERVED_FIELD);
	pucIE = prTxFrame->aucInfoElem;
	prMeasurementRepIE = SM_MEASUREMENT_REP_IE(pucIE);

	prTxFrame->u2FrameCtrl = MAC_FRAME_ACTION;

	COPY_MAC_ADDR(prTxFrame->aucDestAddr, prStaRec->aucMacAddr);
	COPY_MAC_ADDR(prTxFrame->aucSrcAddr, prBssInfo->aucOwnMacAddr);
	COPY_MAC_ADDR(prTxFrame->aucBSSID, prBssInfo->aucBSSID);

	prTxFrame->ucCategory = CATEGORY_SPEC_MGT;
	prTxFrame->ucAction = ACTION_MEASUREMENT_REPORT;

	/* 3 Compose the frame body's frame. */
	prTxFrame->ucDialogToken = prStaRec->ucSmDialogToken;
	prMeasurementRepIE->ucId = ELEM_ID_MEASUREMENT_REPORT;

#if 0
	if (prStaRec->ucSmMsmtRequestMode == ELEM_RM_TYPE_BASIC_REQ) {
		prMeasurementRepIE->ucLength =
		     sizeof(struct SM_BASIC_REPORT) + 3;
		u2PayloadLen = ACTION_SM_MEASURE_REPORT_LEN+
		     ACTION_SM_BASIC_REPORT_LEN;
	} else if (prStaRec->ucSmMsmtRequestMode == ELEM_RM_TYPE_CCA_REQ) {
		prMeasurementRepIE->ucLength = sizeof(struct SM_CCA_REPORT) + 3;
		u2PayloadLen = ACTION_SM_MEASURE_REPORT_LEN+
		     ACTION_SM_CCA_REPORT_LEN;
	} else if (prStaRec->ucSmMsmtRequestMode ==
		     ELEM_RM_TYPE_RPI_HISTOGRAM_REQ) {
		prMeasurementRepIE->ucLength = sizeof(struct SM_RPI_REPORT) + 3;
		u2PayloadLen = ACTION_SM_MEASURE_REPORT_LEN+
		     ACTION_SM_PRI_REPORT_LEN;
	} else {
		prMeasurementRepIE->ucLength = 3;
		u2PayloadLen = ACTION_SM_MEASURE_REPORT_LEN;
	}
#else
	prMeasurementRepIE->ucLength = 3;
	u2PayloadLen = ACTION_SM_MEASURE_REPORT_LEN;
	prMeasurementRepIE->ucToken = prStaRec->ucSmMsmtToken;
	prMeasurementRepIE->ucReportMode = BIT(1);
	prMeasurementRepIE->ucMeasurementType = prStaRec->ucSmMsmtRequestMode;
#endif

	/* 4 Update information of MSDU_INFO_T */
	TX_SET_MMPDU(prAdapter, prMsduInfo, prStaRec->ucBssIndex,
		     prStaRec->ucIndex, WLAN_MAC_MGMT_HEADER_LEN,
		     WLAN_MAC_MGMT_HEADER_LEN + u2PayloadLen, pfTxDoneHandler,
		     MSDU_RATE_MODE_AUTO);

	DBGLOG(RLM, TRACE,
	       "ucDialogToken %d ucToken %d ucReportMode %d ucMeasurementType %d\n",
	       prTxFrame->ucDialogToken, prMeasurementRepIE->ucToken,
	       prMeasurementRepIE->ucReportMode,
	       prMeasurementRepIE->ucMeasurementType);

	/* 4 Enqueue the frame to send this action frame. */
	nicTxEnqueueMsdu(prAdapter, prMsduInfo);

	return;

} /* end of msmtComposeReportFrame() */

/*----------------------------------------------------------------------------*/
/*!
 * \brief This function handle spectrum management action frame
 *
 * \param[in]
 *
 * \return none
 */
/*----------------------------------------------------------------------------*/
void rlmProcessSpecMgtAction(struct ADAPTER *prAdapter, struct SW_RFB *prSwRfb)
{
	uint8_t *pucIE;
	struct STA_RECORD *prStaRec;
	struct BSS_INFO *prBssInfo;
	uint16_t u2IELength;
	uint16_t u2Offset = 0;
	struct IE_CHANNEL_SWITCH *prChannelSwitchAnnounceIE;
	struct IE_SECONDARY_OFFSET *prSecondaryOffsetIE = NULL;
	struct IE_WIDE_BAND_CHANNEL *prWideBandChannelIE = NULL;
	struct IE_TPC_REQ *prTpcReqIE;
	struct IE_TPC_REPORT *prTpcRepIE;
	struct IE_MEASUREMENT_REQ *prMeasurementReqIE;
	struct IE_MEASUREMENT_REPORT *prMeasurementRepIE;
	struct ACTION_SM_REQ_FRAME *prRxFrame;
	u_int8_t fgHasWideBandIE = FALSE;
	u_int8_t fgHasSCOIE = FALSE;
	u_int8_t fgHasChannelSwitchIE = FALSE;
	bool fgNeedSwitchChannel = FALSE;
#if CFG_DFS_NEWCH_DFS_FORCE_DISCONNECT
	struct ieee80211_channel *Channel = NULL;
#endif

	DBGLOG(RLM, INFO, "[Mgt Action]rlmProcessSpecMgtAction\n");
	ASSERT(prAdapter);
	ASSERT(prSwRfb);

	u2IELength =
		prSwRfb->u2PacketLen -
		(uint16_t)OFFSET_OF(struct ACTION_SM_REQ_FRAME, aucInfoElem[0]);

	prRxFrame = (struct ACTION_SM_REQ_FRAME *)prSwRfb->pvHeader;
	pucIE = prRxFrame->aucInfoElem;

	prStaRec = cnmGetStaRecByIndex(prAdapter, prSwRfb->ucStaRecIdx);
	if (!prStaRec)
		return;

	if (prStaRec->ucBssIndex > prAdapter->ucHwBssIdNum)
		return;

	prBssInfo = GET_BSS_INFO_BY_INDEX(prAdapter, prStaRec->ucBssIndex);

	prStaRec->ucSmDialogToken = prRxFrame->ucDialogToken;

	DBGLOG_MEM8(RLM, INFO, pucIE, u2IELength);
	switch (prRxFrame->ucAction) {
	case ACTION_MEASUREMENT_REQ:
		DBGLOG(RLM, INFO, "[Mgt Action] Measure Request\n");
		prMeasurementReqIE = SM_MEASUREMENT_REQ_IE(pucIE);
		if (prMeasurementReqIE->ucId == ELEM_ID_MEASUREMENT_REQ) {
			/* Check IE length is valid */
			if (prMeasurementReqIE->ucLength != 0 &&
				(prMeasurementReqIE->ucLength >=
				sizeof(struct IE_MEASUREMENT_REQ) - 2)) {
				prStaRec->ucSmMsmtRequestMode =
					prMeasurementReqIE->ucRequestMode;
				prStaRec->ucSmMsmtToken =
					prMeasurementReqIE->ucToken;
				msmtComposeReportFrame(prAdapter, prStaRec,
							NULL);
			}
		}

		break;
	case ACTION_MEASUREMENT_REPORT:
		DBGLOG(RLM, INFO, "[Mgt Action] Measure Report\n");
		prMeasurementRepIE = SM_MEASUREMENT_REP_IE(pucIE);
		if (prMeasurementRepIE->ucId == ELEM_ID_MEASUREMENT_REPORT)
			DBGLOG(RLM, TRACE,
			       "[Mgt Action] Correct Measurement report IE !!\n");
		break;
	case ACTION_TPC_REQ:
		DBGLOG(RLM, INFO, "[Mgt Action] TPC Request\n");
		prTpcReqIE = SM_TPC_REQ_IE(pucIE);

		if (prTpcReqIE->ucId == ELEM_ID_TPC_REQ)
			tpcComposeReportFrame(prAdapter, prStaRec, NULL);

		break;
	case ACTION_TPC_REPORT:
		DBGLOG(RLM, INFO, "[Mgt Action] TPC Report\n");
		prTpcRepIE = SM_TPC_REP_IE(pucIE);

		if (prTpcRepIE->ucId == ELEM_ID_TPC_REPORT)
			DBGLOG(RLM, TRACE,
			       "[Mgt Action] Correct TPC report IE !!\n");

		break;
	case ACTION_CHNL_SWITCH:
		IE_FOR_EACH(pucIE, u2IELength, u2Offset)
		{
			switch (IE_ID(pucIE)) {

			case ELEM_ID_WIDE_BAND_CHANNEL_SWITCH:
				if (!RLM_NET_IS_11AC(prBssInfo) ||
				    IE_LEN(pucIE) !=
					    (sizeof(struct
						    IE_WIDE_BAND_CHANNEL) -
					     2)) {
					DBGLOG(RLM, INFO,
					       "[Mgt Action] ELEM_ID_WIDE_BAND_CHANNEL_SWITCH, Length\n");
					break;
				}
				DBGLOG(RLM, INFO,
				       "[Mgt Action] ELEM_ID_WIDE_BAND_CHANNEL_SWITCH, 11AC\n");
				prWideBandChannelIE =
					(struct IE_WIDE_BAND_CHANNEL *)pucIE;
				prBssInfo->ucVhtChannelWidth =
					prWideBandChannelIE->ucNewChannelWidth;
				prBssInfo->ucVhtChannelFrequencyS1 =
					prWideBandChannelIE->ucChannelS1;
				prBssInfo->ucVhtChannelFrequencyS2 =
					prWideBandChannelIE->ucChannelS2;

				/* Revise by own OP BW if needed */
				if ((prBssInfo->fgIsOpChangeChannelWidth) &&
				    (rlmGetVhtOpBwByBssOpBw(
					     prBssInfo
						     ->ucOpChangeChannelWidth) <
				     prBssInfo->ucVhtChannelWidth)) {

					DBGLOG(RLM, LOUD,
					"Change to w:%d s1:%d s2:%d since own changed BW < peer's WideBand BW",
					prBssInfo->ucVhtChannelWidth,
					prBssInfo->ucVhtChannelFrequencyS1,
					prBssInfo->ucVhtChannelFrequencyS2);
					rlmFillVhtOpInfoByBssOpBw(
					prBssInfo,
					prBssInfo
					->ucOpChangeChannelWidth);
				}

				fgHasWideBandIE = TRUE;
				break;

			case ELEM_ID_CH_SW_ANNOUNCEMENT:
				if (IE_LEN(pucIE) !=
				    (sizeof(struct IE_CHANNEL_SWITCH) - 2)) {
					DBGLOG(RLM, INFO,
					       "[Mgt Action] ELEM_ID_CH_SW_ANNOUNCEMENT, Length\n");
					break;
				}

				prChannelSwitchAnnounceIE =
					(struct IE_CHANNEL_SWITCH *)pucIE;

				DBGLOG(RLM, INFO, "[Ch] Count=%d\n",
					prChannelSwitchAnnounceIE->
						ucChannelSwitchCount);

				fgHasChannelSwitchIE = TRUE;

				/* Decide the switch new channel */
#if (CFG_CSA_ACTION_FRAME_NO_DELAY == 0)
				if (prChannelSwitchAnnounceIE
					->ucChannelSwitchCount <= 3)
#endif
				{
					DBGLOG(RLM, INFO,
					       "[Mgt Action] switch channel [%d]->[%d]\n",
					       prBssInfo->ucPrimaryChannel,
					       prChannelSwitchAnnounceIE
						       ->ucNewChannelNum);
					prBssInfo->ucPrimaryChannel =
						prChannelSwitchAnnounceIE
							->ucNewChannelNum;

#if CFG_DFS_NEWCH_DFS_FORCE_DISCONNECT
				Channel = ieee80211_get_channel(
				priv_to_wiphy(prAdapter->prGlueInfo),
					ieee80211_channel_to_frequency(prChannelSwitchAnnounceIE->ucNewChannelNum,
					KAL_BAND_5GHZ));
				DBGLOG(RLM, INFO, "[DFS][CSA][CLIENT] Switch to DFS channel: new ChNum = [%d]\n",prChannelSwitchAnnounceIE->ucNewChannelNum);
				if (Channel &&
				    (Channel->flags & IEEE80211_CHAN_RADAR)) {
					DBGLOG(RLM, INFO, "[DFS][CSA][CLIENT] New channel is DFS channel!");
					aisBssLinkDown(prAdapter,
						GET_IOCTL_BSSIDX(prAdapter));
				} else
#endif
					{
						fgNeedSwitchChannel = TRUE;
					}
				}

				/* Decide to stop Tx by switch mode */
				if (prChannelSwitchAnnounceIE
						->ucChannelSwitchMode == 1) {

					/* Stop Tx immediately */
					if (!g_fgHasStopTx) {
						g_fgHasStopTx = TRUE;
						qmSetStaRecTxAllowed(prAdapter,
							prStaRec, FALSE);
						DBGLOG(RLM, EVENT,
							"[Ch] TxAllowed = FALSE\n");
					}
				}
#ifdef CFG_DFS_CHSW_FORCE_BW20
				else {
					g_fgHasChannelSwitchIE = TRUE;

					if (RLM_NET_IS_11AC(prBssInfo)) {
						DBGLOG(RLM, INFO,
							   "Send Operation Action Frame");
						rlmSendOpModeNotificationFrame(
						  prAdapter, prStaRec,
						  VHT_OP_MODE_CHANNEL_WIDTH_20,
						  1);
					} else {
						DBGLOG(RLM, INFO,
							"Skip Send Operation Action Frame");
					}
				}
#endif
				break;
			case ELEM_ID_SCO:
				if (IE_LEN(pucIE) !=
				    (sizeof(struct IE_SECONDARY_OFFSET) - 2)) {
					DBGLOG(RLM, INFO,
					       "[Mgt Action] ELEM_ID_SCO, Length\n");
					break;
				}
				prSecondaryOffsetIE =
					(struct IE_SECONDARY_OFFSET *)pucIE;
				DBGLOG(RLM, INFO,
				       "[Mgt Action] SCO [%d]->[%d]\n",
				       prBssInfo->eBssSCO,
				       prSecondaryOffsetIE->ucSecondaryOffset);
				prBssInfo->eBssSCO =
					prSecondaryOffsetIE->ucSecondaryOffset;
				fgHasSCOIE = TRUE;
				break;
			default:
				break;
			} /*end of switch IE_ID */
		}	 /*end of IE_FOR_EACH */

		if (fgNeedSwitchChannel) {
			struct BSS_DESC *prBssDesc;
			struct PARAM_SSID rSsid;

			memset(&rSsid, 0, sizeof(struct PARAM_SSID));

			prBssInfo->eBand =
				((prBssInfo->ucPrimaryChannel <= 14) ?
					BAND_2G4 :
#if (CFG_SUPPORT_WIFI_6G == 1)
				(prBssInfo->ucPrimaryChannel > 180) ?
					BAND_6G :
#endif
				BAND_5G);

			/* Change to BW20 for certification issue
			 * due to signal sidelope leakage
			 */
			prBssInfo->ucVhtChannelWidth = 0;
			prBssInfo->ucVhtChannelFrequencyS1 = 0;
			prBssInfo->ucVhtChannelFrequencyS2 = 0;
			prBssInfo->eBssSCO = 0;

			if (fgHasWideBandIE != FALSE) {
				prBssInfo->ucVhtChannelWidth =
					prWideBandChannelIE->ucNewChannelWidth;
				prBssInfo->ucVhtChannelFrequencyS1 =
					prWideBandChannelIE->ucChannelS1;
				prBssInfo->ucVhtChannelFrequencyS2 =
					prWideBandChannelIE->ucChannelS2;

				/* Revise by own OP BW if needed */
				if ((prBssInfo->fgIsOpChangeChannelWidth) &&
					(rlmGetVhtOpBwByBssOpBw(
						prBssInfo->
						  ucOpChangeChannelWidth) <
						prBssInfo->ucVhtChannelWidth)) {
					DBGLOG(RLM, LOUD,
						"Change to w:%d s1:%d s2:%d since own changed BW < peer's WideBand BW",
						prBssInfo->ucVhtChannelWidth,
						prBssInfo->
							ucVhtChannelFrequencyS1,
						prBssInfo->
							ucVhtChannelFrequencyS2
					);
					rlmFillVhtOpInfoByBssOpBw(
						prBssInfo,
						prBssInfo->
							ucOpChangeChannelWidth);
				}
			}

			COPY_SSID(rSsid.aucSsid, rSsid.u4SsidLen,
			      prBssInfo->aucSSID, prBssInfo->ucSSIDLen);
			prBssDesc = scanSearchBssDescByBssidAndSsid(
				prAdapter, prBssInfo->aucBSSID, TRUE, &rSsid);

			if (fgHasSCOIE != FALSE)
				prBssInfo->eBssSCO =
					prSecondaryOffsetIE->ucSecondaryOffset;

			/* Check SAP channel */
			p2pFuncSwitchSapChannel(prAdapter);

			if (prBssDesc) {
				DBGLOG(RLM, INFO,
					"DFS: BSS: " MACSTR
					" Desc found, channel from %u to %u\n ",
					MAC2STR(prBssInfo->aucBSSID),
					prBssDesc->ucChannelNum,
					prBssInfo->ucPrimaryChannel);
				prBssDesc->ucChannelNum =
					prBssInfo->ucPrimaryChannel;
				prBssDesc->eChannelWidth =
					prBssInfo->ucVhtChannelWidth;
				prBssDesc->ucCenterFreqS1 =
					prBssInfo->ucVhtChannelFrequencyS1;
				prBssDesc->ucCenterFreqS2 =
					prBssInfo->ucVhtChannelFrequencyS2;
				kalIndicateChannelSwitch(
					prAdapter->prGlueInfo,
					prBssInfo->eBssSCO,
#if (CFG_SUPPORT_WIFI_6G == 1)
					prBssDesc->eBand,
#endif
					prBssDesc->ucChannelNum);
			} else {
				DBGLOG(RLM, WARN,
				"[Mgt Action] BssDesc is not found!\n");
			}
		}
		nicUpdateBss(prAdapter, prBssInfo->ucBssIndex);
		break;
	default:
		break;
	}
}

#endif

/*----------------------------------------------------------------------------*/
/*!
 * \brief Send OpMode Norification frame (VHT action frame)
 *
 * \param[in] ucChannelWidth 0:20MHz, 1:40MHz, 2:80MHz, 3:160MHz or 80+80MHz
 *
 * \return none
 */
/*----------------------------------------------------------------------------*/
uint32_t rlmSendOpModeNotificationFrame(struct ADAPTER *prAdapter,
				    struct STA_RECORD *prStaRec,
				    uint8_t ucChannelWidth, uint8_t ucOpRxNss)
{

	struct MSDU_INFO *prMsduInfo;
	struct ACTION_OP_MODE_NOTIFICATION_FRAME *prTxFrame;
	struct BSS_INFO *prBssInfo;
	uint16_t u2EstimatedFrameLen;
	PFN_TX_DONE_HANDLER pfTxDoneHandler = (PFN_TX_DONE_HANDLER)NULL;

	/* Sanity Check */
	if (!prStaRec)
		return WLAN_STATUS_FAILURE;

	prBssInfo = GET_BSS_INFO_BY_INDEX(prAdapter, prStaRec->ucBssIndex);
	if (!prBssInfo)
		return WLAN_STATUS_FAILURE;

	/* Calculate MSDU buffer length */
	u2EstimatedFrameLen = MAC_TX_RESERVED_FIELD +
			      sizeof(struct ACTION_OP_MODE_NOTIFICATION_FRAME);

	/* Alloc MSDU_INFO */
	prMsduInfo = (struct MSDU_INFO *)cnmMgtPktAlloc(prAdapter,
							u2EstimatedFrameLen);

	if (!prMsduInfo)
		return WLAN_STATUS_FAILURE;

	kalMemZero(prMsduInfo->prPacket, u2EstimatedFrameLen);

	prTxFrame = prMsduInfo->prPacket;

	/* Fill frame ctrl */
	prTxFrame->u2FrameCtrl = MAC_FRAME_ACTION;

	COPY_MAC_ADDR(prTxFrame->aucDestAddr, prStaRec->aucMacAddr);
	COPY_MAC_ADDR(prTxFrame->aucSrcAddr, prBssInfo->aucOwnMacAddr);
	COPY_MAC_ADDR(prTxFrame->aucBSSID, prBssInfo->aucBSSID);

	/* 3 Compose the frame body's frame */
	prTxFrame->ucCategory = CATEGORY_VHT_ACTION;
	prTxFrame->ucAction = ACTION_OPERATING_MODE_NOTIFICATION;

	prTxFrame->ucOperatingMode |=
		(ucChannelWidth & VHT_OP_MODE_CHANNEL_WIDTH);

	if (ucOpRxNss == 0)
		ucOpRxNss = 1;
	prTxFrame->ucOperatingMode |=
		(((ucOpRxNss - 1) << 4) & VHT_OP_MODE_RX_NSS);
	prTxFrame->ucOperatingMode &= ~VHT_OP_MODE_RX_NSS_TYPE;

	if (prBssInfo->pfOpChangeHandler)
		pfTxDoneHandler = rlmNotifyVhtOpModeTxDone;

	/* 4 Update information of MSDU_INFO_T */
	TX_SET_MMPDU(prAdapter, prMsduInfo, prBssInfo->ucBssIndex,
		     prStaRec->ucIndex, WLAN_MAC_MGMT_HEADER_LEN,
		     sizeof(struct ACTION_OP_MODE_NOTIFICATION_FRAME),
		     pfTxDoneHandler, MSDU_RATE_MODE_AUTO);

	/* 4 Enqueue the frame to send this action frame. */
	nicTxEnqueueMsdu(prAdapter, prMsduInfo);

	return WLAN_STATUS_SUCCESS;
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief Send SM Power Save frame (HT action frame)
 *
 * \param[in]
 *
 * \return none
 */
/*----------------------------------------------------------------------------*/
uint32_t rlmSendSmPowerSaveFrame(struct ADAPTER *prAdapter,
			     struct STA_RECORD *prStaRec, uint8_t ucOpRxNss)
{
	struct MSDU_INFO *prMsduInfo;
	struct ACTION_SM_POWER_SAVE_FRAME *prTxFrame;
	struct BSS_INFO *prBssInfo;
	uint16_t u2EstimatedFrameLen;
	PFN_TX_DONE_HANDLER pfTxDoneHandler = (PFN_TX_DONE_HANDLER)NULL;

	/* Sanity Check */
	if (!prStaRec)
		return WLAN_STATUS_FAILURE;

	prBssInfo = GET_BSS_INFO_BY_INDEX(prAdapter, prStaRec->ucBssIndex);
	if (!prBssInfo)
		return WLAN_STATUS_FAILURE;

	/* Calculate MSDU buffer length */
	u2EstimatedFrameLen = MAC_TX_RESERVED_FIELD +
			      sizeof(struct ACTION_SM_POWER_SAVE_FRAME);

	/* Alloc MSDU_INFO */
	prMsduInfo = (struct MSDU_INFO *)cnmMgtPktAlloc(prAdapter,
							u2EstimatedFrameLen);

	if (!prMsduInfo)
		return WLAN_STATUS_FAILURE;

	kalMemZero(prMsduInfo->prPacket, u2EstimatedFrameLen);

	prTxFrame = prMsduInfo->prPacket;

	/* Fill frame ctrl */
	prTxFrame->u2FrameCtrl = MAC_FRAME_ACTION;

	COPY_MAC_ADDR(prTxFrame->aucDestAddr, prStaRec->aucMacAddr);
	COPY_MAC_ADDR(prTxFrame->aucSrcAddr, prBssInfo->aucOwnMacAddr);
	COPY_MAC_ADDR(prTxFrame->aucBSSID, prBssInfo->aucBSSID);

	/* 3 Compose the frame body's frame */
	prTxFrame->ucCategory = CATEGORY_HT_ACTION;
	prTxFrame->ucAction = ACTION_HT_SM_POWER_SAVE;

	if (ucOpRxNss == 1)
		prTxFrame->ucSmPowerCtrl |= HT_SM_POWER_SAVE_CONTROL_ENABLED;
	else if (ucOpRxNss == 2)
		prTxFrame->ucSmPowerCtrl &= ~HT_SM_POWER_SAVE_CONTROL_ENABLED;
	else {
		DBGLOG(RLM, WARN,
		       "Can't switch to RxNss = %d since we don't support.\n",
		       ucOpRxNss);
		return WLAN_STATUS_FAILURE;
	}

	/* Static SM power save mode */
	prTxFrame->ucSmPowerCtrl &=
		(~HT_SM_POWER_SAVE_CONTROL_SM_MODE);

	if (prBssInfo->pfOpChangeHandler)
		pfTxDoneHandler = rlmSmPowerSaveTxDone;

	/* 4 Update information of MSDU_INFO_T */
	TX_SET_MMPDU(prAdapter, prMsduInfo, prBssInfo->ucBssIndex,
		     prStaRec->ucIndex, WLAN_MAC_MGMT_HEADER_LEN,
		     sizeof(struct ACTION_SM_POWER_SAVE_FRAME), pfTxDoneHandler,
		     MSDU_RATE_MODE_AUTO);

	/* 4 Enqueue the frame to send this action frame. */
	nicTxEnqueueMsdu(prAdapter, prMsduInfo);

	return WLAN_STATUS_SUCCESS;
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief Send Notify Channel Width frame (HT action frame)
 *
 * \param[in] ucChannelWidth 0:20MHz, 1:Any channel width
 *  in the STAs Supported Channel Width Set subfield
 *
 * \return none
 */
/*----------------------------------------------------------------------------*/
uint32_t rlmSendNotifyChannelWidthFrame(struct ADAPTER *prAdapter,
				    struct STA_RECORD *prStaRec,
				    uint8_t ucChannelWidth)
{
	struct MSDU_INFO *prMsduInfo;
	struct ACTION_NOTIFY_CHANNEL_WIDTH_FRAME *prTxFrame;
	struct BSS_INFO *prBssInfo;
	uint16_t u2EstimatedFrameLen;
	PFN_TX_DONE_HANDLER pfTxDoneHandler = (PFN_TX_DONE_HANDLER)NULL;

	/* Sanity Check */
	if (!prStaRec)
		return WLAN_STATUS_FAILURE;

	prBssInfo = GET_BSS_INFO_BY_INDEX(prAdapter, prStaRec->ucBssIndex);
	if (!prBssInfo)
		return WLAN_STATUS_FAILURE;

	/* Calculate MSDU buffer length */
	u2EstimatedFrameLen = MAC_TX_RESERVED_FIELD +
			      sizeof(struct ACTION_NOTIFY_CHANNEL_WIDTH_FRAME);

	/* Alloc MSDU_INFO */
	prMsduInfo = (struct MSDU_INFO *)cnmMgtPktAlloc(prAdapter,
							u2EstimatedFrameLen);

	if (!prMsduInfo)
		return WLAN_STATUS_FAILURE;

	kalMemZero(prMsduInfo->prPacket, u2EstimatedFrameLen);

	prTxFrame = prMsduInfo->prPacket;

	/* Fill frame ctrl */
	prTxFrame->u2FrameCtrl = MAC_FRAME_ACTION;

	COPY_MAC_ADDR(prTxFrame->aucDestAddr, prStaRec->aucMacAddr);
	COPY_MAC_ADDR(prTxFrame->aucSrcAddr, prBssInfo->aucOwnMacAddr);
	COPY_MAC_ADDR(prTxFrame->aucBSSID, prBssInfo->aucBSSID);

	/* 3 Compose the frame body's frame */
	prTxFrame->ucCategory = CATEGORY_HT_ACTION;
	prTxFrame->ucAction = ACTION_HT_NOTIFY_CHANNEL_WIDTH;

	prTxFrame->ucChannelWidth = ucChannelWidth;

	if (prBssInfo->pfOpChangeHandler)
		pfTxDoneHandler = rlmNotifyChannelWidthtTxDone;

	/* 4 Update information of MSDU_INFO_T */
	TX_SET_MMPDU(prAdapter, prMsduInfo, prBssInfo->ucBssIndex,
		     prStaRec->ucIndex, WLAN_MAC_MGMT_HEADER_LEN,
		     sizeof(struct ACTION_NOTIFY_CHANNEL_WIDTH_FRAME),
		     pfTxDoneHandler, MSDU_RATE_MODE_AUTO);

	/* 4 Enqueue the frame to send this action frame. */
	nicTxEnqueueMsdu(prAdapter, prMsduInfo);

	return WLAN_STATUS_SUCCESS;
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief
 *
 * \param[in]
 *
 * \return none
 *
 */
/*----------------------------------------------------------------------------*/
uint32_t rlmNotifyVhtOpModeTxDone(IN struct ADAPTER *prAdapter,
				  IN struct MSDU_INFO *prMsduInfo,
				  IN enum ENUM_TX_RESULT_CODE rTxDoneStatus)
{
	u_int8_t fgIsSuccess = FALSE;

	do {
		ASSERT((prAdapter != NULL) && (prMsduInfo != NULL));

		if (rTxDoneStatus == TX_RESULT_SUCCESS)
			fgIsSuccess = TRUE;

	} while (FALSE);

	rlmOpModeTxDoneHandler(prAdapter, prMsduInfo, OP_NOTIFY_TYPE_VHT_NSS_BW,
			       fgIsSuccess);

	return WLAN_STATUS_SUCCESS;
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief
 *
 * \param[in]
 *
 * \return none
 */
/*----------------------------------------------------------------------------*/
uint32_t rlmSmPowerSaveTxDone(IN struct ADAPTER *prAdapter,
			      IN struct MSDU_INFO *prMsduInfo,
			      IN enum ENUM_TX_RESULT_CODE rTxDoneStatus)
{
	u_int8_t fgIsSuccess = FALSE;

	do {
		ASSERT((prAdapter != NULL) && (prMsduInfo != NULL));

		if (rTxDoneStatus == TX_RESULT_SUCCESS)
			fgIsSuccess = TRUE;

	} while (FALSE);

	rlmOpModeTxDoneHandler(prAdapter, prMsduInfo, OP_NOTIFY_TYPE_HT_NSS,
			       fgIsSuccess);

	return WLAN_STATUS_SUCCESS;
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief
 *
 * \param[in]
 *
 * \return none
 */
/*----------------------------------------------------------------------------*/
uint32_t rlmNotifyChannelWidthtTxDone(IN struct ADAPTER *prAdapter,
				      IN struct MSDU_INFO *prMsduInfo,
				      IN enum ENUM_TX_RESULT_CODE rTxDoneStatus)
{
	u_int8_t fgIsSuccess = FALSE;

	do {
		ASSERT((prAdapter != NULL) && (prMsduInfo != NULL));

		if (rTxDoneStatus == TX_RESULT_SUCCESS)
			fgIsSuccess = TRUE;

	} while (FALSE);

	rlmOpModeTxDoneHandler(prAdapter, prMsduInfo, OP_NOTIFY_TYPE_HT_BW,
			       fgIsSuccess);

	return WLAN_STATUS_SUCCESS;
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief Handle TX done for OP mode noritfication frame
 *
 * \param[in]
 *
 * \return none
 */
/*----------------------------------------------------------------------------*/
static void rlmOpModeTxDoneHandler(IN struct ADAPTER *prAdapter,
				   IN struct MSDU_INFO *prMsduInfo,
				   IN uint8_t ucOpChangeType,
				   IN u_int8_t fgIsSuccess)
{
	uint32_t u4Status = WLAN_STATUS_SUCCESS;
	struct BSS_INFO *prBssInfo = NULL;
	struct STA_RECORD *prStaRec = NULL;
	u_int8_t fgIsOpModeChangeSuccess = FALSE; /* OP change result */
	uint8_t ucRelatedFrameType =
		OP_NOTIFY_TYPE_NUM; /* Used for HT notification frame */
		/* Used for HT notification frame */
	uint8_t ucCurrOpState = OP_NOTIFY_STATE_NUM,
		ucRelatedOpState = OP_NOTIFY_STATE_NUM;

	/* Sanity check */
	ASSERT((prAdapter != NULL) && (prMsduInfo != NULL));

	prBssInfo = prAdapter->aprBssInfo[prMsduInfo->ucBssIndex];

	ASSERT(prBssInfo);

	prStaRec = prBssInfo->prStaRecOfAP;

	DBGLOG(RLM, INFO,
	       "OP notification Tx done: BSS[%d] Type[%d] Status[%d] IsSuccess[%d]\n",
	       prBssInfo->ucBssIndex, ucOpChangeType,
	       prBssInfo->aucOpModeChangeState[ucOpChangeType], fgIsSuccess);

	do {
		/* <1>handle abnormal case */
		if ((prBssInfo->aucOpModeChangeState[ucOpChangeType] !=
		     OP_NOTIFY_STATE_KEEP) &&
		    (prBssInfo->aucOpModeChangeState[ucOpChangeType] !=
		     OP_NOTIFY_STATE_SENDING)) {
			DBGLOG(RLM, WARN,
			       "Unexpected BSS[%d] OpModeChangeState[%d]\n",
			       prBssInfo->ucBssIndex,
			       prBssInfo->aucOpModeChangeState[ucOpChangeType]);
			rlmRollbackOpChangeParam(prBssInfo, TRUE, TRUE);
			fgIsOpModeChangeSuccess = FALSE;
			break;
		}

		if (ucOpChangeType >= OP_NOTIFY_TYPE_NUM) {
			DBGLOG(RLM, WARN,
			       "Uxexpected Bss[%d] OpChangeType[%d]\n",
			       prMsduInfo->ucBssIndex, ucOpChangeType);
			rlmRollbackOpChangeParam(prBssInfo, TRUE, TRUE);
			fgIsOpModeChangeSuccess = FALSE;
			break;
		}

		/* <2>Assign Op notification Type/State for HT notification
		 * frame
		 */
		if ((ucOpChangeType == OP_NOTIFY_TYPE_HT_BW) ||
		    (ucOpChangeType == OP_NOTIFY_TYPE_HT_NSS)) {

			ucRelatedFrameType =
				(ucOpChangeType == OP_NOTIFY_TYPE_HT_BW)
					? OP_NOTIFY_TYPE_HT_NSS
					: OP_NOTIFY_TYPE_HT_BW;

			ucCurrOpState = prBssInfo
				->aucOpModeChangeState[ucOpChangeType];

			ucRelatedOpState = prBssInfo
				->aucOpModeChangeState[ucRelatedFrameType];
		}

		/* <3.1>handle TX done - SUCCESS */
		if (fgIsSuccess == TRUE) {

			/* Clear retry count */
			prBssInfo->aucOpModeChangeRetryCnt[ucOpChangeType] = 0;

			if (ucOpChangeType == OP_NOTIFY_TYPE_VHT_NSS_BW) {
				/* VHT notification frame sent */
				fgIsOpModeChangeSuccess = TRUE;
				break;
			}

			/* HT notification frame sent */
			if (ucCurrOpState ==
			    OP_NOTIFY_STATE_SENDING) { /* Change OpMode */

				ucCurrOpState = OP_NOTIFY_STATE_SUCCESS;

				/* Case1: Wait for both HT BW/Nss notification
				 * frame TX done
				 */
				if (ucRelatedOpState ==
				    OP_NOTIFY_STATE_SENDING)
					return;

				/* Case2: Both BW and Nss notification TX done
				 * or only change either BW or Nss
				 */
				if ((ucRelatedOpState ==
				     OP_NOTIFY_STATE_KEEP) ||
				    (ucRelatedOpState ==
				     OP_NOTIFY_STATE_SUCCESS)) {
					fgIsOpModeChangeSuccess = TRUE;

					/* Case3: One of the notification TX
					 * failed,
					 * re-send a notification frame to
					 * rollback the successful one
					 */
				} else if (ucRelatedOpState ==
					   OP_NOTIFY_STATE_FAIL) {
					/*Rollback to keep the original BW/Nss
					 */
					ucCurrOpState = OP_NOTIFY_STATE_KEEP;
					if (ucOpChangeType ==
					    OP_NOTIFY_TYPE_HT_BW)
						u4Status =
						rlmSendNotifyChannelWidthFrame(
						prAdapter, prStaRec,
						rlmGetBssOpBwByVhtAndHtOpInfo(
						prBssInfo));
					else if (ucOpChangeType ==
						 OP_NOTIFY_TYPE_HT_NSS)
						u4Status =
						rlmSendSmPowerSaveFrame(
							prAdapter, prStaRec,
							prBssInfo->ucOpRxNss);

					DBGLOG(RLM, INFO,
						"Bss[%d] OpType[%d] Tx Failed, send OpType",
						prMsduInfo->ucBssIndex,
						ucRelatedFrameType);
					DBGLOG(RLM, INFO,
						"[%d] for roll back to BW[%d] RxNss[%d]\n",
						ucOpChangeType,
						rlmGetBssOpBwByVhtAndHtOpInfo
							(prBssInfo),
						prBssInfo->ucOpRxNss);

					if (u4Status == WLAN_STATUS_SUCCESS)
						return;
				}
			} else if (ucCurrOpState ==
				   OP_NOTIFY_STATE_KEEP) { /* Rollback OpMode */

				/* Case4: Rollback success, keep original OP
				 * BW/Nss
				 */
				if (ucOpChangeType == OP_NOTIFY_TYPE_HT_BW)
					rlmRollbackOpChangeParam(prBssInfo,
								 TRUE, FALSE);
				else if (ucOpChangeType ==
					 OP_NOTIFY_TYPE_HT_NSS)
					rlmRollbackOpChangeParam(prBssInfo,
								 FALSE, TRUE);

				fgIsOpModeChangeSuccess = FALSE;
			}
		} /* End of processing TX success */
		/* <3.2>handle TX done - FAIL */
		else {
			prBssInfo->aucOpModeChangeRetryCnt[ucOpChangeType]++;

			/* Re-send notification frame */
			if (prBssInfo
				    ->aucOpModeChangeRetryCnt[ucOpChangeType] <=
			    OPERATION_NOTICATION_TX_LIMIT) {
				if (ucOpChangeType == OP_NOTIFY_TYPE_VHT_NSS_BW)
					u4Status =
					rlmSendOpModeNotificationFrame(
					prAdapter, prStaRec,
					prBssInfo->ucOpChangeChannelWidth,
					prBssInfo->ucOpChangeRxNss);
				else if (ucOpChangeType ==
					 OP_NOTIFY_TYPE_HT_NSS)
					u4Status = rlmSendSmPowerSaveFrame(
						prAdapter, prStaRec,
						prBssInfo->ucOpChangeRxNss);
				else if (ucOpChangeType == OP_NOTIFY_TYPE_HT_BW)
					u4Status =
					rlmSendNotifyChannelWidthFrame(
						prAdapter, prStaRec,
						prBssInfo
						->ucOpChangeChannelWidth);

				if (u4Status == WLAN_STATUS_SUCCESS)
					return;
			}

			/* Clear retry count when retry count > TX limit */
			prBssInfo->aucOpModeChangeRetryCnt[ucOpChangeType] = 0;

			/* VHT notification frame sent */
			if (ucOpChangeType ==
			    OP_NOTIFY_TYPE_VHT_NSS_BW) {
				/* Change failed, keep original OP BW/Nss */
				rlmRollbackOpChangeParam(prBssInfo, TRUE, TRUE);
				fgIsOpModeChangeSuccess = FALSE;
				break;
			}

			/* HT notification frame sent */
			if (ucCurrOpState ==
			    OP_NOTIFY_STATE_SENDING) { /* Change OpMode */
				ucCurrOpState = OP_NOTIFY_STATE_FAIL;

				/* Change failed, keep original OP BW/Nss */
				if (ucOpChangeType == OP_NOTIFY_TYPE_HT_BW)
					rlmRollbackOpChangeParam(prBssInfo,
								 TRUE, FALSE);
				else if (ucOpChangeType ==
					 OP_NOTIFY_TYPE_HT_NSS)
					rlmRollbackOpChangeParam(prBssInfo,
								 FALSE, TRUE);

				/* Case1: Wait for both HT BW/Nss notification
				 * frame TX done
				 */
				if (ucRelatedOpState ==
				    OP_NOTIFY_STATE_SENDING) {
					return;

					/* Case2: Both BW and Nss notification
					 * TX done
					 * or only change either BW or Nss
					 */
				} else if ((ucRelatedOpState ==
					    OP_NOTIFY_STATE_KEEP) ||
					   (ucRelatedOpState ==
					    OP_NOTIFY_STATE_FAIL)) {
					fgIsOpModeChangeSuccess = FALSE;

					/* Case3: One of the notification TX
					 * failed,
					 * re-send a notification frame to
					 * rollback the successful one
					 */
				} else if (ucRelatedOpState ==
					   OP_NOTIFY_STATE_SUCCESS) {
					/*Rollback to keep the original BW/Nss
					 */
					ucRelatedOpState =
						OP_NOTIFY_STATE_KEEP;

					if (ucRelatedFrameType ==
					    OP_NOTIFY_TYPE_HT_BW) {
						u4Status =
						rlmSendNotifyChannelWidthFrame(
						prAdapter, prStaRec,
						rlmGetBssOpBwByVhtAndHtOpInfo(
						prBssInfo));
					} else if (ucRelatedFrameType ==
						   OP_NOTIFY_TYPE_HT_NSS)
						u4Status =
						rlmSendSmPowerSaveFrame(
							prAdapter, prStaRec,
							prBssInfo->ucOpRxNss);

					DBGLOG(RLM, INFO,
					       "Bss[%d] OpType[%d] Tx Failed, send a OpType[%d] for roll back to BW[%d] RxNss[%d]\n",
					       prMsduInfo->ucBssIndex,
					       ucOpChangeType,
					       ucRelatedFrameType,
					       rlmGetBssOpBwByVhtAndHtOpInfo(
						       prBssInfo),
					       prBssInfo->ucOpRxNss);

					if (u4Status == WLAN_STATUS_SUCCESS)
						return;
				}
			} else if (ucCurrOpState ==
				   OP_NOTIFY_STATE_KEEP) /* Rollback OpMode */
				/* Case4: Rollback failed, keep changing OP
				 * BW/Nss
				 */
				fgIsOpModeChangeSuccess = FALSE;
		} /* End of processing TX failed */

	} while (FALSE);

	/* <4>Change own OP info */
	rlmCompleteOpModeChange(prAdapter, prBssInfo, fgIsOpModeChangeSuccess);
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief
 *
 * \param[in]
 *
 * \return none
 */
/*----------------------------------------------------------------------------*/
static void rlmRollbackOpChangeParam(struct BSS_INFO *prBssInfo,
				     u_int8_t fgIsRollbackBw,
				     u_int8_t fgIsRollbackNss)
{

	ASSERT(prBssInfo);

	if (fgIsRollbackBw == TRUE) {
		prBssInfo->fgIsOpChangeChannelWidth = FALSE;
		prBssInfo->ucOpChangeChannelWidth =
			rlmGetBssOpBwByVhtAndHtOpInfo(prBssInfo);
	}

	if (fgIsRollbackNss == TRUE) {
		prBssInfo->fgIsOpChangeRxNss = FALSE;
		prBssInfo->fgIsOpChangeTxNss = FALSE;
		prBssInfo->ucOpChangeRxNss = prBssInfo->ucOpRxNss;
		prBssInfo->ucOpChangeTxNss = prBssInfo->ucOpTxNss;
	}
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief Get BSS operating channel width by VHT and HT OP Info
 *
 * \param[in]
 *
 * \return ucBssOpBw 0:20MHz, 1:40MHz, 2:80MHz, 3:160MHz 4:80+80MHz
 *
 */
/*----------------------------------------------------------------------------*/
uint8_t rlmGetBssOpBwByVhtAndHtOpInfo(struct BSS_INFO *prBssInfo)
{

	uint8_t ucBssOpBw = MAX_BW_20MHZ;
	uint8_t ucChannelWidth = prBssInfo->ucVhtChannelWidth;

	ASSERT(prBssInfo);

	switch (ucChannelWidth) {
	case VHT_OP_CHANNEL_WIDTH_80P80:
		ucBssOpBw = MAX_BW_80_80_MHZ;
		break;

	case VHT_OP_CHANNEL_WIDTH_160:
		ucBssOpBw = MAX_BW_160MHZ;
		break;

	case VHT_OP_CHANNEL_WIDTH_80:
		ucBssOpBw = MAX_BW_80MHZ;
		break;

	case VHT_OP_CHANNEL_WIDTH_20_40:
		if (prBssInfo->eBssSCO != CHNL_EXT_SCN)
			ucBssOpBw = MAX_BW_40MHZ;
		break;
	default:
		DBGLOG(RLM, WARN, "%s: unexpected VHT channel width: %d\n",
		       __func__, ucChannelWidth);
#if CFG_SUPPORT_802_11AC
		if (RLM_NET_IS_11AC(prBssInfo))
			/*VHT default should support BW 80*/
			ucBssOpBw = MAX_BW_80MHZ;
#endif
		break;
	}

	return ucBssOpBw;
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief
 *
 * \param[in]
 *
 * \return ucVhtOpBw 0:20M/40Hz, 1:80MHz, 2:160MHz, 3:80+80MHz
 *
 */
/*----------------------------------------------------------------------------*/
uint8_t rlmGetVhtOpBwByBssOpBw(uint8_t ucBssOpBw)
{
	uint8_t ucVhtOpBw =
		VHT_OP_CHANNEL_WIDTH_80; /*VHT default should support BW 80*/

	switch (ucBssOpBw) {
	case MAX_BW_20MHZ:
	case MAX_BW_40MHZ:
		ucVhtOpBw = VHT_OP_CHANNEL_WIDTH_20_40;
		break;

	case MAX_BW_80MHZ:
		ucVhtOpBw = VHT_OP_CHANNEL_WIDTH_80;
		break;

	case MAX_BW_160MHZ:
		ucVhtOpBw = VHT_OP_CHANNEL_WIDTH_160;
		break;

	case MAX_BW_80_80_MHZ:
		ucVhtOpBw = VHT_OP_CHANNEL_WIDTH_80P80;
		break;
	default:
		DBGLOG(RLM, WARN, "%s: unexpected Bss OP BW: %d\n", __func__,
		       ucBssOpBw);
		break;
	}

	return ucVhtOpBw;
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief Get operating notification channel width by VHT and HT operating Info
 *
 * \param[in]
 *
 * \return ucOpModeBw 0:20MHz, 1:40MHz, 2:80MHz, 3:160MHz/80+80MHz
 *
 */
/*----------------------------------------------------------------------------*/
static uint8_t rlmGetOpModeBwByVhtAndHtOpInfo(struct BSS_INFO *prBssInfo)
{
	uint8_t ucOpModeBw = VHT_OP_MODE_CHANNEL_WIDTH_20;

	ASSERT(prBssInfo);

	switch (prBssInfo->ucVhtChannelWidth) {
	case VHT_OP_CHANNEL_WIDTH_20_40:
		if (prBssInfo->eBssSCO != CHNL_EXT_SCN)
			ucOpModeBw = VHT_OP_MODE_CHANNEL_WIDTH_40;
		break;
	case VHT_OP_CHANNEL_WIDTH_80:
		ucOpModeBw = VHT_OP_MODE_CHANNEL_WIDTH_80;
		break;
	case VHT_OP_CHANNEL_WIDTH_160:
	case VHT_OP_CHANNEL_WIDTH_80P80:
		ucOpModeBw = VHT_OP_MODE_CHANNEL_WIDTH_160_80P80;
		break;
	default:
		DBGLOG(RLM, WARN, "%s: unexpected VHT channel width: %d\n",
		       __func__, prBssInfo->ucVhtChannelWidth);
		/*VHT default IE should support BW 80*/
		ucOpModeBw = VHT_OP_MODE_CHANNEL_WIDTH_80;
		break;
	}

	return ucOpModeBw;
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief
 *
 * \param[in]
 *
 * \return none
 */
/*----------------------------------------------------------------------------*/
static void rlmChangeOwnOpInfo(struct ADAPTER *prAdapter,
			       struct BSS_INFO *prBssInfo)
{
	struct STA_RECORD *prStaRec;

	ASSERT((prAdapter != NULL) && (prBssInfo != NULL));

	/* Update own operating channel Width */
	if (prBssInfo->fgIsOpChangeChannelWidth) {
		if (prBssInfo->ucPhyTypeSet & PHY_TYPE_BIT_HT) {
#if CFG_SUPPORT_802_11AC
			/* Update VHT OP Info*/
			if (prBssInfo->ucPhyTypeSet & PHY_TYPE_BIT_VHT) {
				rlmFillVhtOpInfoByBssOpBw(
					prBssInfo,
					prBssInfo->ucOpChangeChannelWidth);

				DBGLOG(RLM, INFO,
				       "Update BSS[%d] VHT Channel Width Info to w=%d s1=%d s2=%d\n",
				       prBssInfo->ucBssIndex,
				       prBssInfo->ucVhtChannelWidth,
				       prBssInfo->ucVhtChannelFrequencyS1,
				       prBssInfo->ucVhtChannelFrequencyS2);
			}
#endif

			/* Update HT OP Info*/
			if (prBssInfo->ucOpChangeChannelWidth == MAX_BW_20MHZ) {
				prBssInfo->ucHtOpInfo1 &=
					~HT_OP_INFO1_STA_CHNL_WIDTH;
				prBssInfo->eBssSCO = CHNL_EXT_SCN;
			} else {
				prBssInfo->ucHtOpInfo1 |=
					HT_OP_INFO1_STA_CHNL_WIDTH;

				if (prBssInfo->eCurrentOPMode ==
				    OP_MODE_INFRASTRUCTURE) {
					prStaRec = prBssInfo->prStaRecOfAP;
					if (!prStaRec)
						return;

					if ((prStaRec->ucHtPeerOpInfo1 &
					     HT_OP_INFO1_SCO) != CHNL_EXT_RES)
						prBssInfo->eBssSCO =
						(enum ENUM_CHNL_EXT)(
						prStaRec->ucHtPeerOpInfo1 &
						HT_OP_INFO1_SCO);
				} else if (prBssInfo->eCurrentOPMode ==
					   OP_MODE_ACCESS_POINT) {
					prBssInfo->eBssSCO = rlmDecideScoForAP(
						prAdapter, prBssInfo);
				}
			}

			DBGLOG(RLM, INFO,
			       "Update BSS[%d] HT Channel Width Info to bw=%d sco=%d\n",
			       prBssInfo->ucBssIndex,
			       (uint8_t)((prBssInfo->ucHtOpInfo1 &
					  HT_OP_INFO1_STA_CHNL_WIDTH) >>
					 HT_OP_INFO1_STA_CHNL_WIDTH_OFFSET),
			       prBssInfo->eBssSCO);
		}
	}

	/* Update own operating RxNss */
	if (prBssInfo->fgIsOpChangeRxNss) {
		prBssInfo->ucOpRxNss = prBssInfo->ucOpChangeRxNss;
		DBGLOG(RLM, INFO, "Update OP RxNss[%d]\n",
			prBssInfo->ucOpRxNss);
	}

	/* Update own operating TxNss */
	if (prBssInfo->fgIsOpChangeTxNss) {
		prBssInfo->ucOpTxNss = prBssInfo->ucOpChangeTxNss;
		DBGLOG(RLM, INFO, "Update OP TxNss[%d]\n",
			prBssInfo->ucOpTxNss);
	}
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief
 *
 * \param[in]
 *
 * \return none
 */
/*----------------------------------------------------------------------------*/
static void rlmCompleteOpModeChange(struct ADAPTER *prAdapter,
				    struct BSS_INFO *prBssInfo,
				    u_int8_t fgIsSuccess)
{
	PFN_OPMODE_NOTIFY_DONE_FUNC pfnCallback;

	ASSERT((prAdapter != NULL) && (prBssInfo != NULL));

	if ((prBssInfo->fgIsOpChangeChannelWidth) ||
		(prBssInfo->fgIsOpChangeRxNss) ||
		(prBssInfo->fgIsOpChangeTxNss)) {

		/* <1> Update own OP BW/Nss */
		rlmChangeOwnOpInfo(prAdapter, prBssInfo);

		/* <2> Update OP BW/Nss to FW */
		rlmSyncOperationParams(prAdapter, prBssInfo);

		/* <3> Update BCN/Probe Resp IE to notify peers our OP info is
		 * changed (AP mode)
		 */
		if (prBssInfo->eCurrentOPMode == OP_MODE_ACCESS_POINT)
			bssUpdateBeaconContent(prAdapter,
					       prBssInfo->ucBssIndex);
	}

	DBGLOG(RLM, INFO,
		"Complete BSS[%d] OP Mode change to BW[%d] ",
		prBssInfo->ucBssIndex,
		rlmGetBssOpBwByVhtAndHtOpInfo(prBssInfo));
	DBGLOG(RLM, INFO, "RxNss[%d] TxNss[%d]\n",
		prBssInfo->ucOpRxNss,
		prBssInfo->ucOpTxNss);

	/* <4> Tell OpMode change caller the change result
	 * Allow callback function re-trigger OpModeChange immediately.
	 */
	pfnCallback = prBssInfo->pfOpChangeHandler;
	prBssInfo->pfOpChangeHandler = NULL;
	if (pfnCallback)
		pfnCallback(prAdapter, prBssInfo->ucBssIndex,
					     fgIsSuccess);
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief Change OpMode Nss/Channel Width
 *
 * \param[in] ucChannelWidth 0:20MHz, 1:40MHz, 2:80MHz, 3:160MHz 4:80+80MHz
 *
 * \return fgIsChangeOpMode
 *	TRUE: Can change/Don't need to change operation mode
 *	FALSE: Can't change operation mode
 */
/*----------------------------------------------------------------------------*/
enum ENUM_OP_CHANGE_STATUS_T
rlmChangeOperationMode(
	struct ADAPTER *prAdapter, uint8_t ucBssIndex,
	uint8_t ucChannelWidth, uint8_t ucOpRxNss, uint8_t ucOpTxNss,
	PFN_OPMODE_NOTIFY_DONE_FUNC pfOpChangeHandler)
{
	struct BSS_INFO *prBssInfo;
	struct STA_RECORD *prStaRec = (struct STA_RECORD *)NULL;
	u_int8_t fgIsChangeBw = TRUE,
		 fgIsChangeRxNss = TRUE, /* Indicate if need to change */
		 fgIsChangeTxNss = TRUE;
	uint8_t i;
	uint32_t u4Status = WLAN_STATUS_SUCCESS;

	/* Sanity check */
	if (ucBssIndex >= prAdapter->ucHwBssIdNum)
		return OP_CHANGE_STATUS_INVALID;

	prBssInfo = GET_BSS_INFO_BY_INDEX(prAdapter, ucBssIndex);

	if (!prBssInfo)
		return OP_CHANGE_STATUS_INVALID;

	/* <1>Check if OP change parameter is valid */
	if (rlmCheckOpChangeParamValid(prAdapter, prBssInfo, ucChannelWidth,
				       ucOpRxNss, ucOpTxNss) == FALSE)
		return OP_CHANGE_STATUS_INVALID;

	/* <2>Check if OpMode notification is ongoing, if not, register the call
	 * back function
	 */
	if (prBssInfo->pfOpChangeHandler) {
		DBGLOG(RLM, INFO,
		       "BSS[%d] OpMode change notification is ongoing\n",
		       ucBssIndex);
		return OP_CHANGE_STATUS_INVALID;
	}

	prBssInfo->pfOpChangeHandler = pfOpChangeHandler;

	/* <3>Check if the current operating BW/Nss is the same as the target
	 * one
	 */
	if (ucChannelWidth == rlmGetBssOpBwByVhtAndHtOpInfo(prBssInfo))
		fgIsChangeBw = FALSE;

	if (ucOpRxNss == prBssInfo->ucOpRxNss)
		fgIsChangeRxNss = FALSE;

	if (ucOpTxNss == prBssInfo->ucOpTxNss)
		fgIsChangeTxNss = FALSE;

	if ((!fgIsChangeBw) && (!fgIsChangeRxNss) && (!fgIsChangeTxNss)) {
		if (prBssInfo->pfOpChangeHandler) {
			/* (1) Don't need to call callback at no need to change
			 * OP mode case
			 * (2) Clear callback to NULL when handling OP Mode
			 * change request done
			 */
			prBssInfo->pfOpChangeHandler = NULL;
		}

		DBGLOG(RLM, INFO,
			"BSS[%d] target OpMode BW[%d] RxNss[%d] TxNss[%d] No change, return\n",
			ucBssIndex, ucChannelWidth, ucOpRxNss, ucOpTxNss);
		return OP_CHANGE_STATUS_VALID_NO_CHANGE;
	}

	DBGLOG(RLM, INFO,
		"Intend to change BSS[%d] OP Mode to BW[%d] RxNss[%d] TxNss[%d]\n",
		ucBssIndex, ucChannelWidth, ucOpRxNss, ucOpTxNss);

	/* <4> Fill OP Change Info into BssInfo*/
	if (fgIsChangeBw) {
		prBssInfo->ucOpChangeChannelWidth = ucChannelWidth;
		prBssInfo->fgIsOpChangeChannelWidth = TRUE;
		DBGLOG(RLM, INFO, "Intend to change BSS[%d] to BW[%d]\n",
		       ucBssIndex, ucChannelWidth);
	}
	if (fgIsChangeRxNss) {
		prBssInfo->ucOpChangeRxNss = ucOpRxNss;
		prBssInfo->fgIsOpChangeRxNss = TRUE;
		DBGLOG(RLM, INFO, "Intend to change BSS[%d] to RxNss[%d]\n",
		       ucBssIndex, ucOpRxNss);
	}
	if (fgIsChangeTxNss) {
		prBssInfo->ucOpChangeTxNss = ucOpTxNss;
		prBssInfo->fgIsOpChangeTxNss = TRUE;
		DBGLOG(RLM, INFO, "Intend to change BSS[%d] to TxNss[%d]\n",
			ucBssIndex, ucOpTxNss);
	}

	/* <5>Handling OP Info change for STA/GC */
	if ((prBssInfo->eCurrentOPMode == OP_MODE_INFRASTRUCTURE) &&
	    (prBssInfo->prStaRecOfAP)) {
		prStaRec = prBssInfo->prStaRecOfAP;
		/* <5.1>Initialize OP mode change parameters related to
		 * notification Tx done handler (STA mode)
		 */
		if (prBssInfo->pfOpChangeHandler) {
			for (i = 0; i < OP_NOTIFY_TYPE_NUM; i++) {
				prBssInfo->aucOpModeChangeState[i] =
					OP_NOTIFY_STATE_KEEP;
				prBssInfo->aucOpModeChangeRetryCnt[i] = 0;
			}
		}
		/* <5.2> Send operating mode notification frame (STA mode)
		 * No action frame is needed if we only changed OpTxNss.
		 */
#if CFG_SUPPORT_802_11AC
		if (((RLM_NET_IS_11AC(prBssInfo) &&
			(prStaRec->ucDesiredPhyTypeSet &
			PHY_TYPE_SET_802_11AC))
#if (CFG_SUPPORT_WIFI_6G == 1)
			|| (prBssInfo->eBand == BAND_6G &&
			RLM_NET_IS_11AX(prBssInfo) &&
			(prStaRec->ucDesiredPhyTypeSet &
			PHY_TYPE_SET_802_11AX))
#endif
			) &&
			(fgIsChangeBw || fgIsChangeRxNss)) {
			if (prBssInfo->pfOpChangeHandler)
				prBssInfo->aucOpModeChangeState
					[OP_NOTIFY_TYPE_VHT_NSS_BW] =
					OP_NOTIFY_STATE_SENDING;
			DBGLOG(RLM, INFO,
				"Send VHT OP notification frame: BSS[%d] BW[%d] RxNss[%d]\n",
				ucBssIndex, ucChannelWidth, ucOpRxNss);
			u4Status = rlmSendOpModeNotificationFrame(
				prAdapter, prStaRec,
				ucChannelWidth, ucOpRxNss);
		} else
#endif
		{
			if (RLM_NET_IS_11N(prBssInfo) &&
				(fgIsChangeBw || fgIsChangeRxNss)) {
				if (prBssInfo->pfOpChangeHandler) {
					if (fgIsChangeRxNss)
						prBssInfo->aucOpModeChangeState
						[OP_NOTIFY_TYPE_HT_NSS] =
						OP_NOTIFY_STATE_SENDING;
					if (fgIsChangeBw)
						prBssInfo->aucOpModeChangeState
							[OP_NOTIFY_TYPE_HT_BW] =
							OP_NOTIFY_STATE_SENDING;
				}
				if (fgIsChangeRxNss) {
					u4Status = rlmSendSmPowerSaveFrame(
						prAdapter, prStaRec, ucOpRxNss);
					DBGLOG(RLM, INFO,
						"Send HT SM Power Save frame ");
					DBGLOG(RLM, INFO,
						"BSS[%d] RxNss[%d]\n",
						ucBssIndex, ucOpRxNss);
				}
				if (fgIsChangeBw) {
					u4Status =
					rlmSendNotifyChannelWidthFrame(
						prAdapter, prStaRec,
						ucChannelWidth);
					DBGLOG(RLM, INFO,
						"Send HT Notify Channel Width frame: ");
					DBGLOG(RLM, INFO,
						"BSS[%d] BW[%d]\n",
						ucBssIndex, ucChannelWidth);
				}
			}
		}

		/* Error handling */
		if (u4Status != WLAN_STATUS_SUCCESS) {
			rlmCompleteOpModeChange(prAdapter, prBssInfo, FALSE);
			return OP_CHANGE_STATUS_INVALID;
		}

		/* <5.3> Change OP Info w/o waiting for notification Tx done */
		if (prBssInfo->pfOpChangeHandler == NULL ||
			(!fgIsChangeBw && !fgIsChangeRxNss)) {
			rlmCompleteOpModeChange(prAdapter, prBssInfo, TRUE);
			/* No callback */
			return OP_CHANGE_STATUS_VALID_CHANGE_CALLBACK_DONE;
		}
	}
	/* <6>Handling OP Info change for AP/GO */
	else if (prBssInfo->eCurrentOPMode == OP_MODE_ACCESS_POINT) {
		/* Complete OP Info change after notifying client by beacon */
		rlmCompleteOpModeChange(prAdapter, prBssInfo, TRUE);
		return OP_CHANGE_STATUS_VALID_CHANGE_CALLBACK_DONE;
	}
	return OP_CHANGE_STATUS_VALID_CHANGE_CALLBACK_WAIT;
}


/*----------------------------------------------------------------------------*/
/*!
 * \brief Check our desired BW/RxNss is less or equal than peer's capability to
 *        prevent IOT issue.
 *
 * \param[in] prBssInfo
 * \param[in] ucChannelWidth
 * \param[in] ucOpTxNss
 *
 * \return
 *	TRUE : Can change to desired BW/RxNss
 *	FALSE: Should not change operation mode
 */
/*----------------------------------------------------------------------------*/
static u_int8_t rlmCheckOpChangeParamForClient(struct BSS_INFO *prBssInfo,
					       uint8_t ucChannelWidth,
					       uint8_t ucOpRxNss)
{
	struct STA_RECORD *prStaRec;

	prStaRec = prBssInfo->prStaRecOfAP;

	if (!prStaRec)
		return FALSE;

#if CFG_SUPPORT_802_11AC
	if (RLM_NET_IS_11AC(prBssInfo)) { /* VHT */
		/* Check peer OP Channel Width */
		switch (ucChannelWidth) {
		case MAX_BW_80_80_MHZ:
			if (prStaRec->ucVhtOpChannelWidth !=
			    VHT_OP_CHANNEL_WIDTH_80P80) {
				DBGLOG(RLM, INFO,
				       "Can't change BSS[%d] OP BW to:%d for peer VHT OP BW is:%d\n",
				       prBssInfo->ucBssIndex, ucChannelWidth,
				       prStaRec->ucVhtOpChannelWidth);
				return FALSE;
			}
			break;
		case MAX_BW_160MHZ:
			if (prStaRec->ucVhtOpChannelWidth !=
			    VHT_OP_CHANNEL_WIDTH_160) {
				DBGLOG(RLM, INFO,
				       "Can't change BSS[%d] OP BW to:%d for peer VHT OP BW is:%d\n",
				       prBssInfo->ucBssIndex, ucChannelWidth,
				       prStaRec->ucVhtOpChannelWidth);
				return FALSE;
			}
			break;
		case MAX_BW_80MHZ:
			if (prStaRec->ucVhtOpChannelWidth <
			    VHT_OP_CHANNEL_WIDTH_80) {
				DBGLOG(RLM, INFO,
				       "Can't change BSS[%d] OP BW to:%d for peer VHT OP BW is:%d\n",
				       prBssInfo->ucBssIndex, ucChannelWidth,
				       prStaRec->ucVhtOpChannelWidth);
				return FALSE;
			}
			break;
		case MAX_BW_40MHZ:
			if (!(prStaRec->ucHtPeerOpInfo1 &
			      HT_OP_INFO1_STA_CHNL_WIDTH) ||
			    (!prBssInfo->fg40mBwAllowed)) {
				DBGLOG(RLM, INFO,
				       "Can't change BSS[%d] OP BW to:%d for PeerOpBw:%d fg40mBwAllowed:%d\n",
				       prBssInfo->ucBssIndex, ucChannelWidth,
				       (uint8_t)(prStaRec->ucHtPeerOpInfo1 &
						 HT_OP_INFO1_STA_CHNL_WIDTH),
				       prBssInfo->fg40mBwAllowed);
				return FALSE;
			}
			break;
		case MAX_BW_20MHZ:
			break;
		default:
			DBGLOG(RLM, WARN,
			       "BSS[%d] target OP BW:%d is invalid for VHT OpMode change\n",
			       prBssInfo->ucBssIndex, ucChannelWidth);
			return FALSE;
		}

		/* Check peer Rx Nss Cap */
		if (ucOpRxNss == 2 &&
		    ((prStaRec->u2VhtRxMcsMap & VHT_CAP_INFO_MCS_2SS_MASK) >>
		     VHT_CAP_INFO_MCS_2SS_OFFSET) ==
			    VHT_CAP_INFO_MCS_NOT_SUPPORTED) {
			DBGLOG(RLM, INFO,
			       "Don't change Nss since VHT peer doesn't support 2ss\n");
			return FALSE;
		}

	} else
#endif
	{
		if (RLM_NET_IS_11N(prBssInfo)) { /* HT */
			/* Check peer Channel Width */
			if (ucChannelWidth >= MAX_BW_80MHZ) {
				DBGLOG(RLM, WARN,
				       "BSS[%d] target OP BW:%d is invalid for HT OpMode change\n",
				       prBssInfo->ucBssIndex, ucChannelWidth);
				return FALSE;
			} else if (ucChannelWidth ==
					MAX_BW_40MHZ) {
				if (!(prStaRec->ucHtPeerOpInfo1 &
				      HT_OP_INFO1_STA_CHNL_WIDTH) ||
				    (!prBssInfo->fg40mBwAllowed)) {
					DBGLOG(RLM, INFO,
					       "Can't change BSS[%d] OP BW to:%d for PeerOpBw:%d fg40mBwAllowed:%d\n",
					       prBssInfo->ucBssIndex,
					       ucChannelWidth,
					       (uint8_t)(
					prStaRec->ucHtPeerOpInfo1 &
					HT_OP_INFO1_STA_CHNL_WIDTH),
					prBssInfo->fg40mBwAllowed);
					return FALSE;
				}
			}

			/* Check peer Rx Nss Cap */
			if (ucOpRxNss == 2 &&
				(prStaRec->aucRxMcsBitmask[1] == 0)) {
				DBGLOG(RLM, INFO,
				       "Don't change Nss since HT peer doesn't support 2ss\n");
				return FALSE;
			}
		}
	}
	return TRUE;
}

static u_int8_t rlmCheckOpChangeParamValid(struct ADAPTER *prAdapter,
					   struct BSS_INFO *prBssInfo,
					   uint8_t ucChannelWidth,
					   uint8_t ucOpRxNss,
					   uint8_t ucOpTxNss)
{
	uint8_t ucCapNss;

	ASSERT(prBssInfo);

	/* <1>Check if BSS PHY type is legacy mode */
	if (!RLM_NET_IS_11N(prBssInfo)
#if (CFG_SUPPORT_WIFI_6G == 1)
		&& !(prBssInfo->eBand == BAND_6G &&
			RLM_NET_IS_11AX(prBssInfo))
#endif
	) {
		DBGLOG(RLM, WARN,
		       "Can't change BSS[%d] OP info for legacy BSS\n",
		       prBssInfo->ucBssIndex);
		return FALSE;
	}

	/* <2>Check network type */
	if ((prBssInfo->eCurrentOPMode != OP_MODE_INFRASTRUCTURE) &&
	    (prBssInfo->eCurrentOPMode != OP_MODE_ACCESS_POINT)) {
		DBGLOG(RLM, WARN,
		       "Can't change BSS[%d] OP info for OpMode:%d\n",
		       prBssInfo->ucBssIndex, prBssInfo->eCurrentOPMode);
		return FALSE;
	}

	/* <3>Check if target OP BW/Nss <= Own Cap BW/Nss */
	ucCapNss = wlanGetSupportNss(prAdapter, prBssInfo->ucBssIndex);
	if (ucOpRxNss > ucCapNss || ucOpRxNss == 0 ||
		ucOpTxNss > ucCapNss || ucOpTxNss == 0) {
		DBGLOG(RLM, WARN,
		       "Can't change BSS[%d] OP RxNss[%d]TxNss[%d] due to CapNss[%d]\n",
		       prBssInfo->ucBssIndex, ucOpRxNss, ucOpTxNss, ucCapNss);
		return FALSE;
	}

	if (ucChannelWidth > cnmGetBssMaxBw(prAdapter, prBssInfo->ucBssIndex)) {
		DBGLOG(RLM, WARN,
		       "Can't change BSS[%d] OP BW[%d] due to CapBW[%d]\n",
		       prBssInfo->ucBssIndex, ucChannelWidth,
		       cnmGetBssMaxBw(prAdapter, prBssInfo->ucBssIndex));
		return FALSE;
	}

	/* <4>Check if target OP BW is valid for band and primary channel of
	 * current BSS
	 */
	if (prBssInfo->eBand == BAND_2G4) {
		if ((ucChannelWidth != MAX_BW_20MHZ) &&
		    (ucChannelWidth != MAX_BW_40MHZ)) {
			DBGLOG(RLM, WARN,
			       "Can't change BSS[%d] OP BW to:%d for 2.4G\n",
			       prBssInfo->ucBssIndex, ucChannelWidth);
			return FALSE;
		}
	} else {
		/* It can only use BW20 for CH165 */
		if ((ucChannelWidth != MAX_BW_20MHZ) &&
			(prBssInfo->ucPrimaryChannel == 165)) {
			DBGLOG(RLM, WARN,
			       "Can't change BSS[%d] OP BW to:%d for CH165\n",
			       prBssInfo->ucBssIndex, ucChannelWidth);
			return FALSE;
		}

		if ((ucChannelWidth == MAX_BW_160MHZ) &&
		    ((prBssInfo->ucPrimaryChannel < 36) ||
		     ((prBssInfo->ucPrimaryChannel > 64) &&
		      (prBssInfo->ucPrimaryChannel < 100)) ||
		     (prBssInfo->ucPrimaryChannel > 128))) {
			DBGLOG(RLM, WARN,
			       "Can't change BSS[%d] to OP BW160 for primary CH%d\n",
			       prBssInfo->ucBssIndex,
			       prBssInfo->ucPrimaryChannel);
			return FALSE;
		}
	}

	/* <5>Check if target OP BW/Nss <= peer's BW/Nss (STA mode) */
	if (prBssInfo->eCurrentOPMode == OP_MODE_INFRASTRUCTURE) {
		if (rlmCheckOpChangeParamForClient(prBssInfo, ucChannelWidth,
						   ucOpRxNss) == FALSE)
			return FALSE;
	}

	return TRUE;
}

void rlmDummyChangeOpHandler(struct ADAPTER *prAdapter, uint8_t ucBssIndex,
			     bool fgIsChangeSuccess)
{
	DBGLOG(RLM, INFO, "OP change done for BSS[%d] IsSuccess[%d]\n",
	       ucBssIndex, fgIsChangeSuccess);
}

void rlmSetMaxTxPwrLimit(IN struct ADAPTER *prAdapter, int8_t cLimit,
			 uint8_t ucEnable)
{
	struct CMD_SET_AP_CONSTRAINT_PWR_LIMIT rTxPwrLimit;

	kalMemZero(&rTxPwrLimit, sizeof(rTxPwrLimit));
	rTxPwrLimit.ucCmdVer =  0x1;
	rTxPwrLimit.ucPwrSetEnable =  ucEnable;
	if (ucEnable) {
		if (cLimit > RLM_MAX_TX_PWR) {
			DBGLOG(RLM, INFO,
			       "LM: Target MaxPwr %d Higher than Capability, reset to capability\n",
			       cLimit);
			cLimit = RLM_MAX_TX_PWR;
		}
		if (cLimit < RLM_MIN_TX_PWR) {
			DBGLOG(RLM, INFO,
			       "LM: Target MinPwr %d Lower than Capability, reset to capability\n",
			       cLimit);
			cLimit = RLM_MIN_TX_PWR;
		}
		DBGLOG(RLM, INFO,
		       "LM: Set Max Tx Power Limit %d, Min Limit %d\n", cLimit,
		       RLM_MIN_TX_PWR);
		rTxPwrLimit.cMaxTxPwr =
			cLimit * 2; /* unit of cMaxTxPwr is 0.5 dBm */
		rTxPwrLimit.cMinTxPwr = RLM_MIN_TX_PWR * 2;
	} else
		DBGLOG(RLM, TRACE, "LM: Disable Tx Power Limit\n");
	wlanSendSetQueryCmd(prAdapter, CMD_ID_SET_AP_CONSTRAINT_PWR_LIMIT, TRUE,
			    FALSE, FALSE, nicCmdEventSetCommon,
			    nicOidCmdTimeoutCommon,
			    sizeof(struct CMD_SET_AP_CONSTRAINT_PWR_LIMIT),
			    (uint8_t *)&rTxPwrLimit, NULL, 0);
}

#if (CFG_SUPPORT_802_11AX == 1)
/*----------------------------------------------------------------------------*/
/*!
 * @brief Enable/Disable the SR function according to
      the wifi.cfg SREnable parameter
 *
 * @param prAdapter      a pointer to adapter private data structure.
 * @param fgIsEnableSr  a parameter to decide sr enable or disable.
 *
 * @return -
 */
/*----------------------------------------------------------------------------*/
void rlmSetSrControl(IN struct ADAPTER *prAdapter, bool fgIsEnableSr)
{
	struct _SR_CMD_SR_CAP_T *prCmdSrCap = NULL;

	ASSERT(prAdapter);

	prCmdSrCap = (struct _SR_CMD_SR_CAP_T *)
		kalMemAlloc(sizeof(struct _SR_CMD_SR_CAP_T),
			VIR_MEM_TYPE);
	if (prCmdSrCap == NULL) {
		DBGLOG(RLM, ERROR, "LM: Mem alloc fail for SR_CMD_SR_CAP\n");
		return;
	}

	prCmdSrCap->rSrCmd.u1CmdSubId = SR_CMD_SET_SR_CAP_SREN_CTRL;
	prCmdSrCap->rSrCmd.u1DbdcIdx = 0;
	prCmdSrCap->rSrCap.fgSrEn = fgIsEnableSr;

	wlanSendSetQueryExtCmd(prAdapter,
		CMD_ID_LAYER_0_EXT_MAGIC_NUM,
		EXT_CMD_ID_SR_CTRL,
		TRUE,
		FALSE,
		TRUE,
		NULL,
		nicOidCmdTimeoutCommon,
		sizeof(struct _SR_CMD_SR_CAP_T),
		(uint8_t *) (prCmdSrCap),
		NULL, 0);

	kalMemFree(prCmdSrCap, VIR_MEM_TYPE,
		   sizeof(struct _SR_CMD_SR_CAP_T));
}
#endif

/*----------------------------------------------------------------------------*/
/*!
 * \brief Send channel switch frame
 *
 * \param[in]
 *
 * \return none
 */
/*----------------------------------------------------------------------------*/
uint32_t rlmSendChannelSwitchTxDone(IN struct ADAPTER *prAdapter,
	IN struct MSDU_INFO *prMsduInfo,
	IN enum ENUM_TX_RESULT_CODE rTxDoneStatus)
{
	do {
		ASSERT_BREAK((prAdapter != NULL) && (prMsduInfo != NULL));

		DBGLOG(P2P, INFO,
			"CSA TX Done Status: %d, seqNo: %d\n",
			rTxDoneStatus,
			prMsduInfo->ucTxSeqNum);

	} while (FALSE);

	return WLAN_STATUS_SUCCESS;
}

void rlmSendChannelSwitchFrame(struct ADAPTER *prAdapter,
	IN uint8_t ucBssIndex)
{
	struct MSDU_INFO *prMsduInfo;
	struct ACTION_CHANNEL_SWITCH_FRAME *prTxFrame;
	struct BSS_INFO *prBssInfo;
	uint16_t u2EstimatedFrameLen;
	PFN_TX_DONE_HANDLER pfTxDoneHandler = (PFN_TX_DONE_HANDLER)NULL;
	uint8_t aucBMC[] = BC_MAC_ADDR;

	prBssInfo = GET_BSS_INFO_BY_INDEX(prAdapter, ucBssIndex);
	if (!prBssInfo)
		return;

	/* Calculate MSDU buffer length */
	u2EstimatedFrameLen = MAC_TX_RESERVED_FIELD +
			      sizeof(struct ACTION_CHANNEL_SWITCH_FRAME);

	/* Alloc MSDU_INFO */
	prMsduInfo = (struct MSDU_INFO *)cnmMgtPktAlloc(prAdapter,
							u2EstimatedFrameLen);
	if (!prMsduInfo)
		return;

	kalMemZero(prMsduInfo->prPacket, u2EstimatedFrameLen);

	prTxFrame = prMsduInfo->prPacket;

	/* Fill frame ctrl */
	prTxFrame->u2FrameCtrl = MAC_FRAME_ACTION;

	COPY_MAC_ADDR(prTxFrame->aucDestAddr, aucBMC);
	COPY_MAC_ADDR(prTxFrame->aucSrcAddr, prBssInfo->aucOwnMacAddr);
	COPY_MAC_ADDR(prTxFrame->aucBSSID, prBssInfo->aucBSSID);

	/* 3 Compose the frame body's frame */
	prTxFrame->ucCategory = CATEGORY_SPEC_MGT;
	prTxFrame->ucAction = ACTION_CHNL_SWITCH;

	prTxFrame->aucInfoElem[0] = ELEM_ID_CH_SW_ANNOUNCEMENT;
	prTxFrame->aucInfoElem[1] = 3;
	prTxFrame->aucInfoElem[2]
		= prAdapter->rWifiVar.ucChannelSwitchMode;
	prTxFrame->aucInfoElem[3]
		= prAdapter->rWifiVar.ucNewChannelNumber;
	prTxFrame->aucInfoElem[4]
		= prAdapter->rWifiVar.ucChannelSwitchCount;

	/* 3.2 - Secondary Channel Offset element */
	prTxFrame->aucInfoElem[5] = ELEM_ID_SCO;
	prTxFrame->aucInfoElem[6] = 1;
	prTxFrame->aucInfoElem[7]
		= prAdapter->rWifiVar.ucSecondaryOffset;

	/* 3.3 - Wide Bandwidth Channel Switch element */
	prTxFrame->aucInfoElem[8] = ELEM_ID_WIDE_BAND_CHANNEL_SWITCH;
	prTxFrame->aucInfoElem[9] = 3;
	prTxFrame->aucInfoElem[10]
		= prAdapter->rWifiVar.ucNewChannelWidth;
	prTxFrame->aucInfoElem[11]
		= prAdapter->rWifiVar.ucNewChannelS1;
	prTxFrame->aucInfoElem[12]
		= prAdapter->rWifiVar.ucNewChannelS2;

	pfTxDoneHandler = rlmSendChannelSwitchTxDone;

	/* 4 Update information of MSDU_INFO_T */
	TX_SET_MMPDU(prAdapter, prMsduInfo, prBssInfo->ucBssIndex,
		     STA_REC_INDEX_BMCAST, WLAN_MAC_MGMT_HEADER_LEN,
		     sizeof(struct ACTION_CHANNEL_SWITCH_FRAME),
		     pfTxDoneHandler,
		     MSDU_RATE_MODE_AUTO);

	/* 4 Enqueue the frame to send this action frame. */
	nicTxEnqueueMsdu(prAdapter, prMsduInfo);
}

#if CFG_SUPPORT_BFER
/*----------------------------------------------------------------------------*/
/*!
* \brief
*
* \param[in]
*
* \return none
*/
/*----------------------------------------------------------------------------*/
void rlmBfStaRecPfmuUpdate(struct ADAPTER *prAdapter,
				struct STA_RECORD *prStaRec)
{
	uint8_t ucBFerMaxNr, ucBFeeMaxNr, ucMode = 0;
	struct BSS_INFO *prBssInfo;
	struct CMD_STAREC_BF *prStaRecBF;
	struct CMD_STAREC_UPDATE *prStaRecUpdateInfo;
	uint32_t rWlanStatus = WLAN_STATUS_SUCCESS;
	uint32_t u4SetBufferLen = sizeof(struct CMD_STAREC_BF);

	prBssInfo = GET_BSS_INFO_BY_INDEX(prAdapter, prStaRec->ucBssIndex);
	ASSERT(prBssInfo);

	if (RLM_NET_IS_11AC(prBssInfo) &&
	    IS_FEATURE_ENABLED(prAdapter->rWifiVar.ucStaVhtBfer))
		ucMode = MODE_VHT;
	else if (RLM_NET_IS_11N(prBssInfo) &&
		IS_FEATURE_ENABLED(prAdapter->rWifiVar.ucStaHtBfer))
		ucMode = MODE_HT;
#if (CFG_SUPPORT_802_11AX == 1)
	if (RLM_NET_IS_11AX(prBssInfo) &&
		IS_FEATURE_ENABLED(prAdapter->rWifiVar.ucStaHeSuBfer))
		ucMode = MODE_HE_SU;
#endif

	prStaRecBF =
	    (struct CMD_STAREC_BF *) cnmMemAlloc(prAdapter,
		RAM_TYPE_MSG, u4SetBufferLen);

	if (!prStaRecBF) {
		DBGLOG(RLM, ERROR, "STA Rec memory alloc fail\n");
		return;
	}

	prStaRecUpdateInfo =
	    (struct CMD_STAREC_UPDATE *) cnmMemAlloc(prAdapter,
		RAM_TYPE_MSG, (CMD_STAREC_UPDATE_HDR_SIZE + u4SetBufferLen));

	if (!prStaRecUpdateInfo) {
		cnmMemFree(prAdapter, prStaRecBF);
		DBGLOG(RLM, ERROR, "STA Rec Update Info memory alloc fail\n");
		return;
	}

	prStaRec->rTxBfPfmuStaInfo.u2PfmuId = 0xFFFF;

	switch (ucMode) {
#if (CFG_SUPPORT_802_11AX == 1)
case MODE_HE_SU:
	prStaRec->rTxBfPfmuStaInfo.fgSU_MU = FALSE;
	prStaRec->rTxBfPfmuStaInfo.u1TxBfCap =
			HE_GET_PHY_CAP_SU_BFMEE(prStaRec->ucHePhyCapInfo);

	if (prStaRec->rTxBfPfmuStaInfo.u1TxBfCap) {
		/* OFDM, NDPA/Report Poll/CTS2Self tx mode */
		prStaRec->rTxBfPfmuStaInfo.ucSoundingPhy =
						TX_RATE_MODE_OFDM;

		/* 9: OFDM 24M */
		prStaRec->rTxBfPfmuStaInfo.ucNdpaRate = PHY_RATE_24M;

		/* VHT mode, NDP tx mode */
		prStaRec->rTxBfPfmuStaInfo.ucTxMode = TX_RATE_MODE_HE_SU;

		/* 0: MCS0 */
		prStaRec->rTxBfPfmuStaInfo.ucNdpRate = PHY_RATE_MCS0;

		/* 9: OFDM 24M */
		prStaRec->rTxBfPfmuStaInfo.ucReptPollRate = PHY_RATE_24M;

		switch (prBssInfo->ucVhtChannelWidth) {
		case VHT_OP_CHANNEL_WIDTH_160:
		case VHT_OP_CHANNEL_WIDTH_80P80:
			prStaRec->rTxBfPfmuStaInfo.ucCBW = MAX_BW_160MHZ;
			break;

		case VHT_OP_CHANNEL_WIDTH_80:
			prStaRec->rTxBfPfmuStaInfo.ucCBW = MAX_BW_80MHZ;
			break;

		case VHT_OP_CHANNEL_WIDTH_20_40:
		default:
			prStaRec->rTxBfPfmuStaInfo.ucCBW = MAX_BW_20MHZ;
			if (prBssInfo->eBssSCO != CHNL_EXT_SCN)
				prStaRec->rTxBfPfmuStaInfo.ucCBW =
							MAX_BW_40MHZ;
			break;
		}

		ucBFerMaxNr = 1;
		ucBFeeMaxNr = (prBssInfo->ucVhtChannelWidth <=
			VHT_OP_CHANNEL_WIDTH_80) ?
			HE_GET_PHY_CAP_BFMEE_STS_LT_OR_EQ_80M(
			prStaRec->ucHePhyCapInfo) :
			HE_GET_PHY_CAP_BFMEE_STS_GT_80M(
			prStaRec->ucHePhyCapInfo);
		prStaRec->rTxBfPfmuStaInfo.ucNr =
			(ucBFerMaxNr < ucBFeeMaxNr) ?
				ucBFerMaxNr : ucBFeeMaxNr;

		if (RLM_NET_IS_11AC(prBssInfo)) {
			prStaRec->rTxBfPfmuStaInfo.ucNc =
				((prStaRec->u2VhtRxMcsMap &
				VHT_CAP_INFO_MCS_2SS_MASK) !=
						BITS(2, 3)) ? 1 : 0;
		} else if (RLM_NET_IS_11N(prBssInfo)) {
			prStaRec->rTxBfPfmuStaInfo.ucNc =
				(prStaRec->aucRxMcsBitmask[1] > 0) ? 1 : 0;
		}
	}
	break;

#endif
	case MODE_VHT:
		prStaRec->rTxBfPfmuStaInfo.fgSU_MU = FALSE;
		prStaRec->rTxBfPfmuStaInfo.u1TxBfCap =
				rlmClientSupportsVhtETxBF(prStaRec);

		if (prStaRec->rTxBfPfmuStaInfo.u1TxBfCap) {
			/* OFDM, NDPA/Report Poll/CTS2Self tx mode */
			prStaRec->rTxBfPfmuStaInfo.ucSoundingPhy =
							TX_RATE_MODE_OFDM;

			/* 9: OFDM 24M */
			prStaRec->rTxBfPfmuStaInfo.ucNdpaRate = PHY_RATE_24M;

			/* VHT mode, NDP tx mode */
			prStaRec->rTxBfPfmuStaInfo.ucTxMode = TX_RATE_MODE_VHT;

			/* 0: MCS0 */
			prStaRec->rTxBfPfmuStaInfo.ucNdpRate = PHY_RATE_MCS0;

			/* 9: OFDM 24M */
			prStaRec->rTxBfPfmuStaInfo.ucReptPollRate = PHY_RATE_24M;

			switch (prBssInfo->ucVhtChannelWidth) {
			case VHT_OP_CHANNEL_WIDTH_80:
				prStaRec->rTxBfPfmuStaInfo.ucCBW = MAX_BW_80MHZ;
				break;

			case VHT_OP_CHANNEL_WIDTH_20_40:
			default:
				prStaRec->rTxBfPfmuStaInfo.ucCBW = MAX_BW_20MHZ;
				if (prBssInfo->eBssSCO != CHNL_EXT_SCN)
					prStaRec->rTxBfPfmuStaInfo.ucCBW =
								MAX_BW_40MHZ;
				break;
			}

			ucBFerMaxNr = 1;
			ucBFeeMaxNr = rlmClientSupportsVhtBfeeStsCap(prStaRec);
			prStaRec->rTxBfPfmuStaInfo.ucNr =
				(ucBFerMaxNr < ucBFeeMaxNr) ?
					ucBFerMaxNr : ucBFeeMaxNr;
			prStaRec->rTxBfPfmuStaInfo.ucNc =
				((prStaRec->u2VhtRxMcsMap &
					VHT_CAP_INFO_MCS_2SS_MASK) !=
							BITS(2, 3)) ? 1 : 0;
		}
		break;

	case MODE_HT:
		prStaRec->rTxBfPfmuStaInfo.fgSU_MU = FALSE;
		prStaRec->rTxBfPfmuStaInfo.u1TxBfCap =
				rlmClientSupportsHtETxBF(prStaRec);

		if (prStaRec->rTxBfPfmuStaInfo.u1TxBfCap) {
			/* HT mode, NDPA/NDP tx mode */
			prStaRec->rTxBfPfmuStaInfo.ucTxMode =
						TX_RATE_MODE_HTMIX;

			/* 0: HT MCS0 */
			prStaRec->rTxBfPfmuStaInfo.ucNdpaRate = PHY_RATE_MCS0;

			prStaRec->rTxBfPfmuStaInfo.ucCBW = MAX_BW_20MHZ;
			if (prBssInfo->eBssSCO != CHNL_EXT_SCN)
				prStaRec->rTxBfPfmuStaInfo.ucCBW = MAX_BW_40MHZ;

			ucBFerMaxNr = 1;
			ucBFeeMaxNr =
				(prStaRec->u4TxBeamformingCap &
				TXBF_COMPRESSED_TX_ANTENNANUM_SUPPORTED) >>
				TXBF_COMPRESSED_TX_ANTENNANUM_SUPPORTED_OFFSET;
			prStaRec->rTxBfPfmuStaInfo.ucNr =
				(ucBFerMaxNr < ucBFeeMaxNr) ?
					ucBFerMaxNr : ucBFeeMaxNr;
			prStaRec->rTxBfPfmuStaInfo.ucNc =
				(prStaRec->aucRxMcsBitmask[1] > 0) ? 1 : 0;
			prStaRec->rTxBfPfmuStaInfo.ucNdpRate =
				prStaRec->rTxBfPfmuStaInfo.ucNr * 8;
		}
		break;
	default:
		cnmMemFree(prAdapter, prStaRecBF);
		cnmMemFree(prAdapter, prStaRecUpdateInfo);
		return;
	}

	DBGLOG(RLM, INFO, "ucMode=%d\n", ucMode);
	DBGLOG(RLM, INFO, "rlmClientSupportsVhtETxBF(prStaRec)=%d\n",
				rlmClientSupportsVhtETxBF(prStaRec));
	DBGLOG(RLM, INFO, "rlmClientSupportsVhtBfeeStsCap(prStaRec)=%d\n",
				rlmClientSupportsVhtBfeeStsCap(prStaRec));
	DBGLOG(RLM, INFO, "prStaRec->u2VhtRxMcsMap=%x\n",
				prStaRec->u2VhtRxMcsMap);

	DBGLOG(RLM, INFO,
	    "====================== BF StaRec Info =====================\n");
	DBGLOG(RLM, INFO, "u2PfmuId       =%d\n",
				prStaRec->rTxBfPfmuStaInfo.u2PfmuId);
	DBGLOG(RLM, INFO, "fgSU_MU        =%d\n",
				prStaRec->rTxBfPfmuStaInfo.fgSU_MU);
	DBGLOG(RLM, INFO, "u1TxBfCap     =%d\n",
				prStaRec->rTxBfPfmuStaInfo.u1TxBfCap);
	DBGLOG(RLM, INFO, "ucSoundingPhy  =%d\n",
				prStaRec->rTxBfPfmuStaInfo.ucSoundingPhy);
	DBGLOG(RLM, INFO, "ucNdpaRate     =%d\n",
				prStaRec->rTxBfPfmuStaInfo.ucNdpaRate);
	DBGLOG(RLM, INFO, "ucNdpRate      =%d\n",
				prStaRec->rTxBfPfmuStaInfo.ucNdpRate);
	DBGLOG(RLM, INFO, "ucReptPollRate =%d\n",
				prStaRec->rTxBfPfmuStaInfo.ucReptPollRate);
	DBGLOG(RLM, INFO, "ucTxMode       =%d\n",
				prStaRec->rTxBfPfmuStaInfo.ucTxMode);
	DBGLOG(RLM, INFO, "ucNc           =%d\n",
				prStaRec->rTxBfPfmuStaInfo.ucNc);
	DBGLOG(RLM, INFO, "ucNr           =%d\n",
				prStaRec->rTxBfPfmuStaInfo.ucNr);
	DBGLOG(RLM, INFO, "ucCBW          =%d\n",
				prStaRec->rTxBfPfmuStaInfo.ucCBW);
	DBGLOG(RLM, INFO, "ucTotMemRequire=%d\n",
				prStaRec->rTxBfPfmuStaInfo.ucTotMemRequire);
	DBGLOG(RLM, INFO, "ucMemRequire20M=%d\n",
				prStaRec->rTxBfPfmuStaInfo.ucMemRequire20M);
	DBGLOG(RLM, INFO, "ucMemRow0      =%d\n",
				prStaRec->rTxBfPfmuStaInfo.ucMemRow0);
	DBGLOG(RLM, INFO, "ucMemCol0      =%d\n",
				prStaRec->rTxBfPfmuStaInfo.ucMemCol0);
	DBGLOG(RLM, INFO, "ucMemRow1      =%d\n",
				prStaRec->rTxBfPfmuStaInfo.ucMemRow1);
	DBGLOG(RLM, INFO, "ucMemCol1      =%d\n",
				prStaRec->rTxBfPfmuStaInfo.ucMemCol1);
	DBGLOG(RLM, INFO, "ucMemRow2      =%d\n",
				prStaRec->rTxBfPfmuStaInfo.ucMemRow2);
	DBGLOG(RLM, INFO, "ucMemCol2      =%d\n",
				prStaRec->rTxBfPfmuStaInfo.ucMemCol2);
	DBGLOG(RLM, INFO, "ucMemRow3      =%d\n",
				prStaRec->rTxBfPfmuStaInfo.ucMemRow3);
	DBGLOG(RLM, INFO, "ucMemCol3      =%d\n",
				prStaRec->rTxBfPfmuStaInfo.ucMemCol3);
	DBGLOG(RLM, INFO,
	    "===========================================================\n");



	prStaRecBF->u2Tag = STA_REC_BF;
	prStaRecBF->u2Length = u4SetBufferLen;
	kalMemCopy(&prStaRecBF->rTxBfPfmuInfo,
		&prStaRec->rTxBfPfmuStaInfo, sizeof(struct TXBF_PFMU_STA_INFO));


	prStaRecUpdateInfo->ucBssIndex = prStaRec->ucBssIndex;
	prStaRecUpdateInfo->ucWlanIdx = prStaRec->ucWlanIndex;
	prStaRecUpdateInfo->u2TotalElementNum = 1;
	kalMemCopy(prStaRecUpdateInfo->aucBuffer, prStaRecBF, u4SetBufferLen);


	rWlanStatus = wlanSendSetQueryExtCmd(prAdapter,
			     CMD_ID_LAYER_0_EXT_MAGIC_NUM,
			     EXT_CMD_ID_STAREC_UPDATE,
			     TRUE,
			     FALSE,
			     FALSE,
			     nicCmdEventSetCommon,
			     nicOidCmdTimeoutCommon,
			     (CMD_STAREC_UPDATE_HDR_SIZE + u4SetBufferLen),
			     (uint8_t *) prStaRecUpdateInfo, NULL, 0);

	if (rWlanStatus == WLAN_STATUS_FAILURE)
		DBGLOG(RLM, ERROR, "Send BF sounding cmd fail\n");

	cnmMemFree(prAdapter, prStaRecBF);
	cnmMemFree(prAdapter, prStaRecUpdateInfo);

}

/*----------------------------------------------------------------------------*/
/*!
* \brief
*
* \param[in]
*
* \return none
*/
/*----------------------------------------------------------------------------*/
void rlmETxBfTriggerPeriodicSounding(struct ADAPTER *prAdapter)
{
	uint32_t u4SetBufferLen = sizeof(union PARAM_CUSTOM_TXBF_ACTION_STRUCT);
	union PARAM_CUSTOM_TXBF_ACTION_STRUCT rTxBfActionInfo;
	union CMD_TXBF_ACTION rCmdTxBfActionInfo;
	uint32_t rWlanStatus = WLAN_STATUS_SUCCESS;

	DBGLOG(RLM, INFO, "rlmETxBfTriggerPeriodicSounding\n");

	rTxBfActionInfo.rTxBfSoundingStart.ucCmdCategoryID =
								BF_SOUNDING_ON;

	rTxBfActionInfo.rTxBfSoundingStart.ucSuMuSndMode =
						    AUTO_SU_PERIODIC_SOUNDING;

	rTxBfActionInfo.rTxBfSoundingStart.u4SoundingInterval = 0;

	rTxBfActionInfo.rTxBfSoundingStart.ucStaNum = 0;

	kalMemCopy(&rCmdTxBfActionInfo, &rTxBfActionInfo,
					sizeof(union CMD_TXBF_ACTION));

	rWlanStatus = wlanSendSetQueryExtCmd(prAdapter,
					     CMD_ID_LAYER_0_EXT_MAGIC_NUM,
					     EXT_CMD_ID_BF_ACTION,
					     TRUE,
					     FALSE,
					     FALSE,
					     nicCmdEventSetCommon,
					     nicOidCmdTimeoutCommon,
					     sizeof(union CMD_TXBF_ACTION),
					     (uint8_t *) &rCmdTxBfActionInfo,
					     &rTxBfActionInfo, u4SetBufferLen);

	if (rWlanStatus == WLAN_STATUS_FAILURE)
		DBGLOG(RLM, ERROR, "Send BF sounding cmd fail\n");

}

/*----------------------------------------------------------------------------*/
/*!
* \brief
*
* \param[in]
*
* \return none
*/
/*----------------------------------------------------------------------------*/
bool
rlmClientSupportsVhtETxBF(struct STA_RECORD *prStaRec)
{
	uint8_t ucVhtCapSuBfeeCap;

	ucVhtCapSuBfeeCap =
		(prStaRec->u4VhtCapInfo & VHT_CAP_INFO_SU_BEAMFORMEE_CAPABLE)
		>> VHT_CAP_INFO_SU_BEAMFORMEE_CAPABLE_OFFSET;

	return (ucVhtCapSuBfeeCap) ? TRUE : FALSE;
}

/*----------------------------------------------------------------------------*/
/*!
* \brief
*
* \param[in]
*
* \return none
*/
/*----------------------------------------------------------------------------*/
uint8_t
rlmClientSupportsVhtBfeeStsCap(struct STA_RECORD *prStaRec)
{
	uint8_t ucVhtCapBfeeStsCap;

	ucVhtCapBfeeStsCap =
	    (prStaRec->u4VhtCapInfo &
VHT_CAP_INFO_COMPRESSED_STEERING_NUMBER_OF_BEAMFORMER_ANTENNAS_SUP) >>
VHT_CAP_INFO_COMPRESSED_STEERING_NUMBER_OF_BEAMFORMER_ANTENNAS_SUP_OFF;

	return ucVhtCapBfeeStsCap;
}

/*----------------------------------------------------------------------------*/
/*!
* \brief
*
* \param[in]
*
* \return none
*/
/*----------------------------------------------------------------------------*/
bool
rlmClientSupportsHtETxBF(struct STA_RECORD *prStaRec)
{
	uint32_t u4RxNDPCap, u4ComBfFbkCap;

	u4RxNDPCap = (prStaRec->u4TxBeamformingCap & TXBF_RX_NDP_CAPABLE)
						>> TXBF_RX_NDP_CAPABLE_OFFSET;

	/* Support compress feedback */
	u4ComBfFbkCap = (prStaRec->u4TxBeamformingCap &
			TXBF_EXPLICIT_COMPRESSED_FEEDBACK_IMMEDIATE_CAPABLE)
			>> TXBF_EXPLICIT_COMPRESSED_FEEDBACK_CAPABLE_OFFSET;

	return (u4RxNDPCap == 1) && (u4ComBfFbkCap > 0);
}

#endif

#if CFG_AP_80211K_SUPPORT
static void rlmMulAPAgentFillApRrmCapa(uint8_t *pucCapa)
{
	uint8_t ucIndex = 0;
	uint8_t aucEnabledBits[] = {RRM_CAP_INFO_BEACON_PASSIVE_MEASURE_BIT,
				    RRM_CAP_INFO_BEACON_ACTIVE_MEASURE_BIT,
				    RRM_CAP_INFO_BEACON_TABLE_BIT,
				    RRM_CAP_INFO_CHANNEL_LOAD_MEASURE_BIT,
				    RRM_CAP_INFO_NOISE_HISTOGRAM_MEASURE_BIT,
				    RRM_CAP_INFO_RRM_BIT};

	for (; ucIndex < sizeof(aucEnabledBits); ucIndex++)
		SET_EXT_CAP(pucCapa, ELEM_MAX_LEN_RRM_CAP,
			    aucEnabledBits[ucIndex]);
}

void rlmMulAPAgentGenerateApRRMEnabledCapIE(
				IN struct ADAPTER *prAdapter,
				IN struct MSDU_INFO *prMsduInfo)
{
	struct IE_RRM_ENABLED_CAP *prRrmEnabledCap = NULL;

	if (!prAdapter) {
		DBGLOG(RLM, ERROR, "prAdapter is NULL!\n");
		return;
	}

	if (!prMsduInfo) {
		DBGLOG(RLM, ERROR, "prMsduInfo is NULL!\n");
		return;
	}

	prRrmEnabledCap = (struct IE_RRM_ENABLED_CAP *)
	    (((uint8_t *) prMsduInfo->prPacket) + prMsduInfo->u2FrameLength);
	prRrmEnabledCap->ucId = ELEM_ID_RRM_ENABLED_CAP;
	prRrmEnabledCap->ucLength = ELEM_MAX_LEN_RRM_CAP;
	kalMemZero(&prRrmEnabledCap->aucCap[0], ELEM_MAX_LEN_RRM_CAP);
	rlmMulAPAgentFillApRrmCapa(&prRrmEnabledCap->aucCap[0]);
	prMsduInfo->u2FrameLength += IE_SIZE(prRrmEnabledCap);
}

void rlmMulAPAgentTxMeasurementRequest(
				struct ADAPTER *prAdapter,
				struct STA_RECORD *prStaRec,
				struct SUB_ELEMENT_LIST *prSubIEs)
{
	static uint8_t ucDialogToken = 1;
	struct MSDU_INFO *prMsduInfo = NULL;
	struct BSS_INFO *prBssInfo = NULL;
	uint8_t *pucPayload = NULL;
	struct ACTION_RM_REQ_FRAME *prTxFrame = NULL;
	uint16_t u2TxFrameLen = 500;
	uint16_t u2FrameLen = 0;

	if (!prStaRec)
		return;

	prBssInfo = GET_BSS_INFO_BY_INDEX(prAdapter, prStaRec->ucBssIndex);
	if (!prBssInfo) {
		DBGLOG(RLM, ERROR, "prBssInfo is NULL!\n");
		return;
	}
	/* 1 Allocate MSDU Info */
	prMsduInfo = (struct MSDU_INFO *)cnmMgtPktAlloc(
		prAdapter, MAC_TX_RESERVED_FIELD + u2TxFrameLen);
	if (!prMsduInfo)
		return;
	prTxFrame = (struct ACTION_RM_REQ_FRAME
			     *)((unsigned long)(prMsduInfo->prPacket) +
				MAC_TX_RESERVED_FIELD);

	/* 2 Compose The Mac Header. */
	prTxFrame->u2FrameCtrl = MAC_FRAME_ACTION;
	COPY_MAC_ADDR(prTxFrame->aucDestAddr, prStaRec->aucMacAddr);
	COPY_MAC_ADDR(prTxFrame->aucSrcAddr, prBssInfo->aucOwnMacAddr);
	COPY_MAC_ADDR(prTxFrame->aucBSSID, prBssInfo->aucBSSID);
	prTxFrame->ucCategory = CATEGORY_RM_ACTION;
	prTxFrame->ucAction = RM_ACTION_RM_REQUEST;
	prTxFrame->u2Repetitions = HTONS(prStaRec->u2BcnReqRepetition);
	u2FrameLen = OFFSET_OF(struct ACTION_RM_REQ_FRAME, aucInfoElem);
	/* 3 Compose the frame body's frame. */
	prTxFrame->ucDialogToken = ucDialogToken++;
	u2TxFrameLen -= (sizeof(*prTxFrame) - 1);
	pucPayload = &prTxFrame->aucInfoElem[0];
	while (prSubIEs && u2TxFrameLen >= (prSubIEs->rSubIE.ucLength + 2)) {
		kalMemCopy(pucPayload, &prSubIEs->rSubIE,
			   prSubIEs->rSubIE.ucLength + 2);
		pucPayload += prSubIEs->rSubIE.ucLength + 2;
		u2FrameLen += prSubIEs->rSubIE.ucLength + 2;
		prSubIEs = prSubIEs->prNext;
		if (prSubIEs)
			prTxFrame->u2Repetitions++;
	}
	nicTxSetMngPacket(prAdapter, prMsduInfo, prStaRec->ucBssIndex,
			  prStaRec->ucIndex, WLAN_MAC_MGMT_HEADER_LEN,
			  u2FrameLen, NULL, MSDU_RATE_MODE_AUTO);

	/* 5 Enqueue the frame to send this action frame. */
	nicTxEnqueueMsdu(prAdapter, prMsduInfo);
}

static u_int8_t rlmMulAPAgentRmReportFrameIsValid(
					struct SW_RFB *prSwRfb)
{
	uint16_t u2ElemLen = 0;
	uint16_t u2Offset =
		(uint16_t)OFFSET_OF(struct ACTION_RM_REPORT_FRAME, aucInfoElem);
	uint8_t *pucIE = (uint8_t *)prSwRfb->pvHeader;
	struct IE_MEASUREMENT_REPORT *prCurrMeasElem = NULL;
	uint16_t u2CalcIELen = 0;
	uint16_t u2IELen = 0;

	if (prSwRfb->u2PacketLen <= u2Offset) {
		DBGLOG(RLM, ERROR, "RRM: Rm Packet length %d is too short\n",
		       prSwRfb->u2PacketLen);
		return FALSE;
	}
	pucIE += u2Offset;
	u2ElemLen = prSwRfb->u2PacketLen - u2Offset;
	IE_FOR_EACH(pucIE, u2ElemLen, u2Offset)
	{
		u2IELen = IE_LEN(pucIE);

		/* The minimum value of the Length field is 3 (based on a
		 ** minimum length for the Measurement Request field
		 ** of 0 octets
		 */
		if (u2IELen <= 3) {
			DBGLOG(RLM, ERROR, "RRM: Abnormal RM IE length is %d\n",
			       u2IELen);
			return FALSE;
		}

		/* Check whether the length of each measurment request element
		 ** is reasonable
		 */
		prCurrMeasElem = (struct IE_MEASUREMENT_REPORT *)pucIE;
		switch (prCurrMeasElem->ucMeasurementType) {
		case ELEM_RM_TYPE_BEACON_REPORT:
			if (u2IELen < (3 + OFFSET_OF(struct RM_BCN_REPORT,
						     aucOptElem))) {
				DBGLOG(RLM, ERROR,
				       "RRM: Abnormal Becaon Req IE length is %d\n",
				       u2IELen);
				return FALSE;
			}
			break;
		default:
			DBGLOG(RLM, ERROR,
			       "RRM: Not support: MeasurementType is %d, IE length is %d\n",
				prCurrMeasElem->ucMeasurementType, u2IELen);
			return FALSE;
		}

		u2CalcIELen += IE_SIZE(pucIE);
	}
	if (u2CalcIELen != u2ElemLen) {
		DBGLOG(RLM, ERROR,
		       "RRM: Calculated Total IE len is not equal to received length\n");
		return FALSE;
	}
	return TRUE;
}

void rlmMulAPAgentProcessRadioMeasurementResponse(
		struct ADAPTER *prAdapter, struct SW_RFB *prSwRfb)
{
	struct ACTION_RM_REPORT_FRAME *prRxFrame = NULL;
	uint8_t *pucOptInfo = NULL;
	uint8_t *pucReportElem = NULL;
	uint16_t u2TmpLen = 0;
	struct IE_MEASUREMENT_REPORT *prMeasureReportIE = NULL;
	struct RM_BCN_REPORT *prBeaconReportIE = NULL;
	struct T_MULTI_AP_BEACON_METRICS_RESP *prBcnMeasureReport = NULL;
	int32_t i4Ret = 0;

	if (!prAdapter) {
		DBGLOG(RLM, ERROR, "prAdapter is NULL!\n");
		return;
	}

	if (!prSwRfb) {
		DBGLOG(RLM, ERROR, "prSwRfb is NULL!\n");
		return;
	}

	/*TODO, check it's soft ap mode or not ?*/

	prRxFrame = (struct ACTION_RM_REPORT_FRAME *)prSwRfb->pvHeader;
	if (!rlmMulAPAgentRmReportFrameIsValid(prSwRfb))
		return;

	prBcnMeasureReport = (struct T_MULTI_AP_BEACON_METRICS_RESP *)
			kalMemAlloc(sizeof(
				struct T_MULTI_AP_BEACON_METRICS_RESP),
			VIR_MEM_TYPE);
	if (prBcnMeasureReport == NULL) {
		DBGLOG(INIT, ERROR, "alloc memory fail\n");
		return;
	}

	kalMemZero(prBcnMeasureReport,
		sizeof(struct T_MULTI_AP_BEACON_METRICS_RESP));
	COPY_MAC_ADDR(prBcnMeasureReport->mStaMac, prRxFrame->aucSrcAddr);

	pucOptInfo = &prRxFrame->aucInfoElem[0];
	u2TmpLen = OFFSET_OF(struct ACTION_RM_REPORT_FRAME, aucInfoElem);
	pucReportElem = prBcnMeasureReport->uElem;

	DBGLOG(RLM, INFO,
		"[SAP_Test] u2FrameCtrl = 0x%x\n", prRxFrame->u2FrameCtrl);
	DBGLOG(RLM, INFO,
		"[SAP_Test] u2Duration = %u\n", prRxFrame->u2Duration);
	DBGLOG(RLM, INFO,
		"[SAP_Test] aucDestAddr = " MACSTR "\n",
		MAC2STR(prRxFrame->aucDestAddr));
	DBGLOG(RLM, INFO,
		"[SAP_Test] aucSrcAddr = " MACSTR "\n",
		MAC2STR(prRxFrame->aucSrcAddr));
	DBGLOG(RLM, INFO,
		"[SAP_Test] aucBSSID = " MACSTR "\n",
		MAC2STR(prRxFrame->aucBSSID));
	DBGLOG(RLM, INFO,
		"[SAP_Test] u2SeqCtrl = %u\n", prRxFrame->u2SeqCtrl);
	DBGLOG(RLM, INFO,
		"[SAP_Test] ucCategory = %u\n", prRxFrame->ucCategory);
	DBGLOG(RLM, INFO,
		"[SAP_Test] ucAction = %u\n", prRxFrame->ucAction);
	DBGLOG(RLM, INFO,
		"[SAP_Test] ucDialogToken = %u\n", prRxFrame->ucDialogToken);
	/* Measurement Report Elements */
	while (prSwRfb->u2PacketLen > u2TmpLen) {
		switch (pucOptInfo[0]) {
		case ELEM_ID_MEASUREMENT_REPORT:
			prMeasureReportIE =
				(struct IE_MEASUREMENT_REPORT *) &pucOptInfo[0];
			DBGLOG(RLM, INFO,
				"[SAP_Test] ucId = %u\n",
				prMeasureReportIE->ucId);
			DBGLOG(RLM, INFO,
				"[SAP_Test] ucLength = %u\n",
				prMeasureReportIE->ucLength);
			DBGLOG(RLM, INFO,
				"[SAP_Test] ucToken = %u\n",
				prMeasureReportIE->ucToken);
			DBGLOG(RLM, INFO,
				"[SAP_Test] ucReportMode = 0x%x\n",
				prMeasureReportIE->ucReportMode);
			DBGLOG(RLM, INFO,
				"[SAP_Test] ucMeasurementType = 0x%x\n",
				prMeasureReportIE->ucMeasurementType);
			prBeaconReportIE =
				(struct RM_BCN_REPORT *)
				&prMeasureReportIE->aucReportFields[0];
			DBGLOG(RLM, INFO,
				"[SAP_Test] ucRegulatoryClass = %d\n",
				prBeaconReportIE->ucRegulatoryClass);
			DBGLOG(RLM, INFO,
				"[SAP_Test] ucChannel = %d\n",
				prBeaconReportIE->ucChannel);
			DBGLOG_MEM8(RLM, INFO,
				prBeaconReportIE->aucStartTime, 8);
			DBGLOG(RLM, INFO,
				"[SAP_Test] u2Duration = %d\n",
				prBeaconReportIE->u2Duration);
			DBGLOG(RLM, INFO,
				"[SAP_Test] ucReportInfo = 0x%x\n",
				prBeaconReportIE->ucReportInfo);
			DBGLOG(RLM, INFO,
				"[SAP_Test] ucRCPI = 0x%x\n",
				prBeaconReportIE->ucRCPI);
			DBGLOG(RLM, INFO,
				"[SAP_Test] ucRSNI = 0x%x\n",
				prBeaconReportIE->ucRSNI);
			DBGLOG(RLM, INFO,
				"[SAP_Test] aucBSSID = " MACSTR "\n",
				MAC2STR(prBeaconReportIE->aucBSSID));
			DBGLOG(RLM, INFO,
				"[SAP_Test] ucAntennaID = %d\n",
				prBeaconReportIE->ucAntennaID);
			DBGLOG_MEM8(RLM, INFO,
				prBeaconReportIE->aucParentTSF, 4);
			DBGLOG_MEM8(RLM, INFO,
				prBeaconReportIE->aucOptElem,
				prMeasureReportIE->ucLength - 3 - 26);

			prBcnMeasureReport->u8ElemNum++;
			prBcnMeasureReport->uElemLen +=
				(prMeasureReportIE->ucLength + 2);
			kalMemCopy(pucReportElem,
				&prMeasureReportIE->ucId,
				prMeasureReportIE->ucLength + 2);
			break;
		default:
			DBGLOG(RLM, INFO, "[SAP_Test] not know\n");
			DBGLOG_MEM8(RLM, WARN,
				pucOptInfo, pucOptInfo[1] + 2);
			break;
		}
		pucOptInfo += (pucOptInfo[1] + 2);
		u2TmpLen += (pucOptInfo[1] + 2);
		pucReportElem += prMeasureReportIE->ucLength;
	}

	DBGLOG(RLM, INFO,
		"[SAP_Test] mStaMac = " MACSTR "\n",
		MAC2STR(prRxFrame->aucSrcAddr));
	DBGLOG(RLM, INFO,
		"[SAP_Test] u8ElemNum = %u\n",
		prBcnMeasureReport->u8ElemNum);
	DBGLOG(RLM, INFO,
		"[SAP_Test] uElemLen = %u\n",
		prBcnMeasureReport->uElemLen);
	DBGLOG_MEM8(RLM, INFO,
		prBcnMeasureReport->uElem,
		prBcnMeasureReport->uElemLen);

	i4Ret = MulAPAgentMontorSendMsg(
		EV_WLAN_MULTIAP_BEACON_METRICS_RESPONSE,
		prBcnMeasureReport, sizeof(*prBcnMeasureReport));
	if (i4Ret < 0)
		DBGLOG(AAA, ERROR,
			"EV_WLAN_MULTIAP_BEACON_METRICS_RESPONSE nl send msg failed!\n");

	kalMemFree(prBcnMeasureReport, VIR_MEM_TYPE,
		sizeof(struct T_MULTI_AP_BEACON_METRICS_RESP));
}
#endif /* CFG_AP_80211K_SUPPORT */

#if CFG_SUPPORT_WAC
uint32_t rlmCalculateWacIELen(IN struct ADAPTER *prAdapter,
		IN uint8_t ucBssIdx, IN struct STA_RECORD *prStaRec)
{
	uint32_t u4IELen = 0;

	if (!prAdapter) {
		DBGLOG(RLM, ERROR, "prAdapter is null.\n");
		return 0;
	}

	if (ucBssIdx > BSS_DEFAULT_NUM) {
		DBGLOG(RLM, ERROR, "ucBssIdx:%d is more than max:%d.\n",
			ucBssIdx, BSS_DEFAULT_NUM);
		return 0;
	}

	if (!prAdapter->rWifiVar.fgEnableWACIE) {
		DBGLOG(RLM, ERROR, "WAC IE disabled, return len=0.\n");
		return 0;
	}

	if (!prAdapter->fgIsP2PRegistered)
		return 0;

	/*WAC IE exist in Beacon or Pro Resp Frame */
	if (!p2pFuncIsAPMode((struct P2P_CONNECTION_SETTINGS *)
				prAdapter->rWifiVar.prP2PConnSettings))
		return 0;

	u4IELen = prAdapter->rWifiVar.u2WACIELen;

	DBGLOG(RLM, INFO, "WAC IE len=%d.\n");
	return u4IELen;
}

void rlmGenerateWacIE(IN struct ADAPTER *prAdapter,
			IN struct MSDU_INFO *prMsduInfo)
{
	uint8_t *pucIEBuf = (uint8_t *) NULL;

	if (!prAdapter || !prMsduInfo) {
		DBGLOG(RLM, ERROR, "prAdapter or prMsduInfo is null.\n");
		return;
	}

	if (!prAdapter->rWifiVar.fgEnableWACIE) {
		DBGLOG(RLM, ERROR, "WAC IE disabled, return null.\n");
		return;
	}

	DBGLOG(RLM, INFO, "Generate WAC IE: len=%d\n",
		prAdapter->rWifiVar.u2WACIELen);
	pucIEBuf = (uint8_t *) ((unsigned long) prMsduInfo->prPacket +
			(unsigned long) prMsduInfo->u2FrameLength);
	kalMemCopy(pucIEBuf, prAdapter->rWifiVar.aucWACIECache,
					prAdapter->rWifiVar.u2WACIELen);
	prMsduInfo->u2FrameLength += prAdapter->rWifiVar.u2WACIELen;
}
#endif
