/* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause */
/*
 * Copyright (c) 2016 MediaTek Inc.
 */
/*
 * Id: //Department/DaVinci/BRANCHES/MT6620_WIFI_DRIVER_V2_3
 *     /include/mgmt/wlan_typedef.h#1
 */

/*! \file   wlan_typedef.h
 *    \brief  Declaration of data type and return values of internal protocol
 *    stack.
 *
 *    In this file we declare the data type and return values which will be
 *    exported to all MGMT Protocol Stack.
 */


#ifndef _WLAN_TYPEDEF_H
#define _WLAN_TYPEDEF_H

/*******************************************************************************
 *                         C O M P I L E R   F L A G S
 *******************************************************************************
 */
#define NIC_TX_ENABLE_SECOND_HW_QUEUE            0

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
/* Type definition for BSS_INFO structure, to describe the
 * attributes used in a common BSS.
 */
struct BSS_INFO;	/* declare BSS_INFO_T */
struct BSS_INFO;	/* declare P2P_DEV_INFO_T */


struct AIS_SPECIFIC_BSS_INFO;	/* declare AIS_SPECIFIC_BSS_INFO_T */
struct P2P_SPECIFIC_BSS_INFO;	/* declare P2P_SPECIFIC_BSS_INFO_T */
struct BOW_SPECIFIC_BSS_INFO;	/* declare BOW_SPECIFIC_BSS_INFO_T */
/* CFG_SUPPORT_WFD */
struct WFD_CFG_SETTINGS;	/* declare WFD_CFG_SETTINGS_T */

/* BSS related structures */
/* Type definition for BSS_DESC structure, to describe parameter
 * sets of a particular BSS
 */
struct BSS_DESC;	/* declare BSS_DESC */

#if CFG_SUPPORT_PASSPOINT
struct HS20_INFO;	/* declare HS20_INFO_T */
#endif /* CFG_SUPPORT_PASSPOINT */

/* Tc Resource index */
enum ENUM_TRAFFIC_CLASS_INDEX {
	/*First HW queue */
	TC0_INDEX = 0,	/* HIF TX: AC0 packets */
	TC1_INDEX,		/* HIF TX: AC1 packets */
	TC2_INDEX,		/* HIF TX: AC2 packets */
	TC3_INDEX,		/* HIF TX: AC3 packets */
	TC4_INDEX,		/* HIF TX: CPU packets */

#if (CFG_TX_RSRC_WMM_ENHANCE == 1)
	TC5_INDEX,		/* HIF TX: AC10 packets */
	TC6_INDEX,		/* HIF TX: AC11 packets */
	TC7_INDEX,		/* HIF TX: AC12 packets */
	TC8_INDEX,		/* HIF TX: AC13 packets */

	TC9_INDEX,		/* HIF TX: AC20 packets */
	TC10_INDEX,		/* HIF TX: AC21 packets */
	TC11_INDEX,		/* HIF TX: AC22 packets */
	TC12_INDEX,		/* HIF TX: AC23 packets */

	TC13_INDEX,		/* HIF TX: AC3X packets */
#endif

#if NIC_TX_ENABLE_SECOND_HW_QUEUE
	/* Second HW queue */
	TC5_INDEX,		/* HIF TX: AC10 packets */
	TC6_INDEX,		/* HIF TX: AC11 packets */
	TC7_INDEX,		/* HIF TX: AC12 packets */
	TC8_INDEX,		/* HIF TX: AC13 packets */
#endif

	TC_NUM			/* Maximum number of Traffic Classes. */
};

#if (CFG_TX_RSRC_WMM_ENHANCE == 1)
#define HIF_WMM_SET_NUM		 HW_WMM_NUM
#endif


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
 *                  F U N C T I O N   D E C L A R A T I O N S
 *******************************************************************************
 */

/*******************************************************************************
 *                              F U N C T I O N S
 *******************************************************************************
 */
#endif /* _WLAN_TYPEDEF_H */
