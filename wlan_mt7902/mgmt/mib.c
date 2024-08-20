/* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause */
/*
 * Copyright (c) 2016 MediaTek Inc.
 */
/*
 * Id: //Department/DaVinci/BRANCHES/MT6620_WIFI_DRIVER_V2_3/mgmt/mib.c#1
 */

/*! \file   "mib.c"
 *    \brief  This file includes the mib default vale and functions.
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

/*******************************************************************************
 *                             D A T A   T Y P E S
 *******************************************************************************
 */

/*******************************************************************************
 *                            P U B L I C   D A T A
 *******************************************************************************
 */
const struct NON_HT_PHY_ATTRIBUTE rNonHTPhyAttributes[] = {
	{RATE_SET_HR_DSSS, TRUE, FALSE}
	,			/* For PHY_TYPE_HR_DSSS_INDEX(0) */
	{RATE_SET_ERP, TRUE, TRUE}
	,			/* For PHY_TYPE_ERP_INDEX(1) */
	{RATE_SET_ERP_P2P, TRUE, TRUE}
	,			/* For PHY_TYPE_ERP_P2P_INDEX(2) */
	{RATE_SET_OFDM, FALSE, FALSE}
	,			/* For PHY_TYPE_OFDM_INDEX(3) */
};

const struct NON_HT_ATTRIBUTE rNonHTAdHocModeAttributes[AD_HOC_MODE_NUM] = {
	{PHY_TYPE_HR_DSSS_INDEX, BASIC_RATE_SET_HR_DSSS}
	,			/* For AD_HOC_MODE_11B(0) */
	{PHY_TYPE_ERP_INDEX, BASIC_RATE_SET_HR_DSSS_ERP}
	,			/* For AD_HOC_MODE_MIXED_11BG(1) */
	{PHY_TYPE_ERP_INDEX, BASIC_RATE_SET_ERP}
	,			/* For AD_HOC_MODE_11G(2) */
	{PHY_TYPE_OFDM_INDEX, BASIC_RATE_SET_OFDM}
	,			/* For AD_HOC_MODE_11A(3) */
};

const struct NON_HT_ATTRIBUTE rNonHTApModeAttributes[AP_MODE_NUM] = {
	{PHY_TYPE_HR_DSSS_INDEX, BASIC_RATE_SET_HR_DSSS}
	,			/* For AP_MODE_11B(0) */
	{PHY_TYPE_ERP_INDEX, BASIC_RATE_SET_HR_DSSS_ERP}
	,			/* For AP_MODE_MIXED_11BG(1) */
	{PHY_TYPE_ERP_INDEX, BASIC_RATE_SET_ERP}
	,			/* For AP_MODE_11G(2) */
	{PHY_TYPE_ERP_P2P_INDEX, BASIC_RATE_SET_ERP_P2P}
	,			/* For AP_MODE_11G_P2P(3) */
	{PHY_TYPE_OFDM_INDEX, BASIC_RATE_SET_OFDM}
	,			/* For AP_MODE_11A(4) */
};

#if CFG_SUPPORT_NAN
struct NON_HT_ADHOC_MODE_ATTRIBUTE rNonHTNanModeAttr[NAN_MODE_NUM] = {
	/* For NAN_MODE_11B(0) */
	{ PHY_TYPE_HR_DSSS_INDEX, BASIC_RATE_SET_HR_DSSS },

	/* For NAN_MODE_MIXED_11BG(1) */
	{ PHY_TYPE_ERP_INDEX, BASIC_RATE_SET_HR_DSSS_ERP },

	/* For NAN_MODE_11G(2) */
	{ PHY_TYPE_ERP_INDEX, BASIC_RATE_SET_ERP },

	/* For NAN_MODE_11A(3) */
	{ PHY_TYPE_OFDM_INDEX, BASIC_RATE_SET_OFDM },
};
#endif /* CFG_SUPPORT_NAN */

/*******************************************************************************
 *                           P R I V A T E   D A T A
 *******************************************************************************
 */

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
