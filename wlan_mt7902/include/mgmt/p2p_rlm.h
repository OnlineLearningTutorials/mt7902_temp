/* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause */
/*
 * Copyright (c) 2016 MediaTek Inc.
 */

/*! \file   "rlm.h"
 *  \brief
 */

#ifndef _P2P_RLM_H
#define _P2P_RLM_H

/******************************************************************************
 *                         C O M P I L E R   F L A G S
 ******************************************************************************
 */

/******************************************************************************
 *                    E X T E R N A L   R E F E R E N C E S
 ******************************************************************************
 */

/******************************************************************************
 *                              C O N S T A N T S
 ******************************************************************************
 */

#define CHANNEL_SPAN_20 20

/******************************************************************************
 *                                 M A C R O S
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
 *                  F U N C T I O N   D E C L A R A T I O N S
 ******************************************************************************
 */

void rlmBssInitForAP(struct ADAPTER *prAdapter, struct BSS_INFO *prBssInfo);

u_int8_t rlmUpdateBwByChListForAP(struct ADAPTER *prAdapter,
		struct BSS_INFO *prBssInfo);

void rlmUpdateParamsForAP(struct ADAPTER *prAdapter,
		struct BSS_INFO *prBssInfo, u_int8_t fgUpdateBeacon);

void rlmFuncInitialChannelList(IN struct ADAPTER *prAdapter);

void
rlmFuncCommonChannelList(IN struct ADAPTER *prAdapter,
		IN struct CHANNEL_ENTRY_FIELD *prChannelEntryII,
		IN uint8_t ucChannelListSize);

uint8_t rlmFuncFindOperatingClass(IN struct ADAPTER *prAdapter,
		IN uint8_t ucChannelNum);

u_int8_t
rlmFuncFindAvailableChannel(IN struct ADAPTER *prAdapter,
		IN uint8_t ucCheckChnl,
		IN uint8_t *pucSuggestChannel,
		IN u_int8_t fgIsSocialChannel, IN u_int8_t fgIsDefaultChannel);

enum ENUM_CHNL_EXT rlmDecideScoForAP(struct ADAPTER *prAdapter,
		struct BSS_INFO *prBssInfo);

enum ENUM_CHNL_EXT rlmGetScoForAP(struct ADAPTER *prAdapter,
		struct BSS_INFO *prBssInfo);

uint8_t rlmGetVhtS1ForAP(struct ADAPTER *prAdapter,
		struct BSS_INFO *prBssInfo);


#endif
