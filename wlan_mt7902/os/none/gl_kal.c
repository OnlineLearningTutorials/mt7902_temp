/* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause */
/*
 * Copyright (c) 2016 MediaTek Inc.
 */
/****************************************************************************
 *[File]             gl_kal.c
 *[Version]          v1.0
 *[Revision Date]    2019/01/01
 *[Author]
 *[Description]
 *    API implementation for os related logic. eg. threading, packet buf
 *[Copyright]
 *    Copyright (C) 2010 MediaTek Incorporation. All Rights Reserved.
 ****************************************************************************/


/*****************************************************************************
 *                         C O M P I L E R   F L A G S
 *****************************************************************************
 */

/*****************************************************************************
 *                    E X T E R N A L   R E F E R E N C E S
 *****************************************************************************
 */

#include "gl_os.h"

#include "precomp.h"

/*****************************************************************************
 *                              C O N S T A N T S
 *****************************************************************************
 */

/*****************************************************************************
 *                             D A T A   T Y P E S
 *****************************************************************************
 */

/*****************************************************************************
 *                            P U B L I C   D A T A
 *****************************************************************************
 */
/*
 * debug level maintain in os-related part for(?)
 * DBG_MODULE_NUM: include/debug.h
 * ENUM_WIFI_LOG_MODULE_NUM: include/wlan_oid.h
 * ENUM_WIFI_LOG_LEVEL_DEFAULT: include/wlan_oid.h
 *
 * access method in include/debug.h:
 * extern uint8_t aucDebugModule[];
 * extern uint32_t au4LogLevel[];
 */
uint8_t aucDebugModule[DBG_MODULE_NUM];
uint32_t au4LogLevel[ENUM_WIFI_LOG_MODULE_NUM] = {ENUM_WIFI_LOG_LEVEL_DEFAULT};
/*****************************************************************************
 *                           P R I V A T E   D A T A
 *****************************************************************************
 */


/*****************************************************************************
 *                                 M A C R O S
 *****************************************************************************
 */

/*****************************************************************************
 *                   F U N C T I O N   D E C L A R A T I O N S
 *****************************************************************************
 */


/*****************************************************************************
 *                              F U N C T I O N S
 *****************************************************************************
 */
#if CFG_CHIP_RESET_SUPPORT
void kalRemoveProbe(IN struct GLUE_INFO *prGlueInfo)
{
	DBGLOG(INIT, WARN, "[SER][L0] not support..\n");
}
#endif

#if (CFG_SUPPORT_SINGLE_SKU_LOCAL_DB == 1)
void
kalApplyCustomRegulatory(IN struct wiphy *pWiphy,
	IN const struct ieee80211_regdomain *pRegdom) { }
#endif

