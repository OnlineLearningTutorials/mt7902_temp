/* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause */
/*
 * Copyright (c) 2016 MediaTek Inc.
 */
/*
 * Id: //Department/DaVinci/BRANCHES/MT6620_WIFI_DRIVER_V2_3/
 *							include/mgmt/wapi.h#1
 */

/*! \file  wapi.h
 *    \brief  The wapi related define, macro and structure are described here.
 */


#ifndef _WAPI_H
#define _WAPI_H

#if CFG_SUPPORT_WAPI

/*******************************************************************************
 *                         C O M P I L E R   F L A G S
 *******************************************************************************
 */

/*******************************************************************************
 *                    E X T E R N A L   R E F E R E N C E S
 *******************************************************************************
 */

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

/*******************************************************************************
 *                                 M A C R O S
 *******************************************************************************
 */
#define WAPI_CIPHER_SUITE_WPI           0x01721400	/* WPI_SMS4 */
#define WAPI_AKM_SUITE_802_1X           0x01721400	/* WAI */
#define WAPI_AKM_SUITE_PSK              0x02721400	/* WAI_PSK */

#define ELEM_ID_WAPI                    68	/* WAPI IE */

#define WAPI_IE(fp)                     ((struct WAPI_INFO_ELEM *) fp)

/*******************************************************************************
 *                  F U N C T I O N   D E C L A R A T I O N S
 *******************************************************************************
 */

void wapiGenerateWAPIIE(IN struct ADAPTER *prAdapter,
			IN struct MSDU_INFO *prMsduInfo);

u_int8_t wapiParseWapiIE(IN struct WAPI_INFO_ELEM
			 *prInfoElem, OUT struct WAPI_INFO *prWapiInfo);

u_int8_t wapiPerformPolicySelection(
			IN struct ADAPTER *prAdapter,
			IN struct BSS_DESC *prBss,
			IN uint8_t ucBssIndex);

/* BOOLEAN */
/* wapiUpdateTxKeyIdx ( */
/* IN  P_STA_RECORD_T     prStaRec, */
/* IN  UINT_8             ucWlanIdx */
/* ); */

/*******************************************************************************
 *                              F U N C T I O N S
 *******************************************************************************
 */
#endif
#endif /* _WAPI_H */
