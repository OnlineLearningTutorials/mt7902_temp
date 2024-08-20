/* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause */
/*
 * Copyright (c) 2016 MediaTek Inc.
 */
/*
 * Id: //Department/DaVinci/BRANCHES/MT6620_WIFI_DRIVER_V2_3/
 *						include/mgmt/rlm_protection.h#1
 */

/*! \file   "rlm_protection.h"
 *    \brief
 */

#ifndef _RLM_PROTECTION_H
#define _RLM_PROTECTION_H

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
enum ENUM_SYS_PROTECT_MODE {
	SYS_PROTECT_MODE_NONE = 0,	/* Mode 0 */
	SYS_PROTECT_MODE_ERP,		/* Mode 1 */
	SYS_PROTECT_MODE_NON_HT,	/* Mode 2 */
	SYS_PROTECT_MODE_20M,		/* Mode 3 */

	SYS_PROTECT_MODE_NUM
};

/* This definition follows HT Protection field of HT Operation IE */
enum ENUM_HT_PROTECT_MODE {
	HT_PROTECT_MODE_NONE = 0,
	HT_PROTECT_MODE_NON_MEMBER,
	HT_PROTECT_MODE_20M,
	HT_PROTECT_MODE_NON_HT,

	HT_PROTECT_MODE_NUM
};

enum ENUM_GF_MODE {
	GF_MODE_NORMAL = 0,
	GF_MODE_PROTECT,
	GF_MODE_DISALLOWED,

	GF_MODE_NUM
};

enum ENUM_RIFS_MODE {
	RIFS_MODE_NORMAL = 0,
	RIFS_MODE_DISALLOWED,

	RIFS_MODE_NUM
};

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

/*******************************************************************************
 *                   F U N C T I O N   D E C L A R A T I O N S
 *******************************************************************************
 */

/*******************************************************************************
 *                              F U N C T I O N S
 *******************************************************************************
 */

#endif /* _RLM_PROTECTION_H */
