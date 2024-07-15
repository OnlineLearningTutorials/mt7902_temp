/* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause */
/*
 * Copyright (c) 2016 MediaTek Inc.
 */
/*! \file   "qosmap.h"
 *    \brief  This file including the qosmap related function.
 *
 *    This file provided the macros and functions library support for the
 *    protocol layer qosmap related function.
 *
 */

#ifndef _QOSMAP_H
#define _QOSMAP_H

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

/*******************************************************************************
 *                           P R I V A T E   D A T A
 *******************************************************************************
 */

/*******************************************************************************
 *                                 M A C R O S
 *******************************************************************************
 */
#define DSCP_SUPPORT 1
/*******************************************************************************
 *                  F U N C T I O N   D E C L A R A T I O N S
 *******************************************************************************
 */
void handleQosMapConf(IN struct ADAPTER *prAdapter,
		      IN struct SW_RFB *prSwRfb);

int qosHandleQosMapConfigure(IN struct ADAPTER *prAdapter,
			     IN struct SW_RFB *prSwRfb);

struct _QOS_MAP_SET *qosParseQosMapSet(IN struct ADAPTER
				       *prAdapter, IN uint8_t *qosMapSet);

uint8_t getUpFromDscp(IN struct GLUE_INFO *prGlueInfo,
		      IN int type, IN int dscp);

void QosMapSetRelease(IN struct STA_RECORD *prStaRec);
/*******************************************************************************
 *                              F U N C T I O N S
 *******************************************************************************
 */

#endif /* _QOSMAP_H */
