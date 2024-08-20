/* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause */
/*
 * Copyright (c) 2016 MediaTek Inc.
 */
/*
 * Id: //Department/DaVinci/BRANCHES/MT6620_WIFI_DRIVER_V2_3/
 *							include/mgmt/rate.h#1
 */

/*! \file  rate.h
 *  \brief This file contains the rate utility function of
 *         IEEE 802.11 family for MediaTek 802.11 Wireless LAN Adapters.
 */

#ifndef _RATE_H
#define _RATE_H

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
 *                         D A T A   T Y P E S
 *******************************************************************************
 */

/*******************************************************************************
 *                            P U B L I C   D A T A
 *******************************************************************************
 */
extern const uint8_t aucDataRate[];

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
/*----------------------------------------------------------------------------*/
/* Routines in rate.c                                                         */
/*----------------------------------------------------------------------------*/
void
rateGetRateSetFromIEs(IN struct IE_SUPPORTED_RATE
		      *prIeSupportedRate,
		      IN struct IE_EXT_SUPPORTED_RATE *prIeExtSupportedRate, OUT
		      uint16_t *pu2OperationalRateSet,
		      OUT uint16_t *pu2BSSBasicRateSet,
		      OUT u_int8_t *pfgIsUnknownBSSBasicRate);

void
rateGetDataRatesFromRateSet(IN uint16_t
			    u2OperationalRateSet, IN uint16_t u2BSSBasicRateSet,
			    OUT uint8_t *pucDataRates, OUT
			    uint8_t *pucDataRatesLen);

u_int8_t rateGetHighestRateIndexFromRateSet(
	IN uint16_t u2RateSet, OUT uint8_t *pucHighestRateIndex);

u_int8_t rateGetLowestRateIndexFromRateSet(
	IN uint16_t u2RateSet, OUT uint8_t *pucLowestRateIndex);

/*******************************************************************************
 *                              F U N C T I O N S
 *******************************************************************************
 */

#endif /* _RATE_H */
