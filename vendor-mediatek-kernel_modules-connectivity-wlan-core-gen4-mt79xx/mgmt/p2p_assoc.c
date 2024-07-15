/* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause */
/*
 * Copyright (c) 2016 MediaTek Inc.
 */
/*
 ** Id: @(#) p2p_assoc.c@@
 */

/*! \file   "p2p_assoc.c"
 *  \brief  This file includes the Wi-Fi Direct association-related functions.
 *
 *  This file includes the association-related functions.
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

/*----------------------------------------------------------------------------*/
/*!
 * @brief This function is used to compose Common Information Elements
 *        for P2P Association Request Frame.
 *
 * @param[in] prMsduInfo     Pointer to the composed MSDU_INFO_T.
 *
 * @return (none)
 */
/*----------------------------------------------------------------------------*/
uint8_t *p2pBuildReAssocReqFrameCommonIEs(IN struct ADAPTER *prAdapter,
		IN struct MSDU_INFO *prMsduInfo,
		IN uint8_t *pucBuffer)
{
	struct BSS_INFO *prP2pBssInfo = (struct BSS_INFO *) NULL;

	prP2pBssInfo = GET_BSS_INFO_BY_INDEX(prAdapter, prMsduInfo->ucBssIndex);

	/* Fill the SSID element. */
	SSID_IE(pucBuffer)->ucId = ELEM_ID_SSID;

	/* NOTE(Kevin): We copy the SSID from CONNECTION_SETTINGS
	 * for the case of Passive Scan and the target BSS didn't broadcast SSID
	 * on its Beacon Frame.
	 */

	COPY_SSID(SSID_IE(pucBuffer)->aucSSID,
		SSID_IE(pucBuffer)->ucLength,
			prP2pBssInfo->aucSSID,
			prP2pBssInfo->ucSSIDLen);

	prMsduInfo->u2FrameLength += IE_SIZE(pucBuffer);
	pucBuffer += IE_SIZE(pucBuffer);
	return pucBuffer;
}
