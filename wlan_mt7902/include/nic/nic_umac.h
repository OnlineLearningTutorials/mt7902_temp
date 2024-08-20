/* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause */
/*
 * Copyright (c) 2016 MediaTek Inc.
 */
/*
 ** Id: //Department/DaVinci/BRANCHES/
 *      MT6620_WIFI_DRIVER_V2_3/include/nic/nic_umac.h#1
 */

/*! \file   "nic_umac.h"
 *    \brief  The declaration of the nic umac debug functions
 *
 */


#ifndef _NIC_UMAC_H
#define _NIC_UMAC_H

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

#define UMAC_FID_MASK                                       0xFFF

#define UMAC_FID_FAULT                                      0xFFF

#define UMAC_PLE_CFG_POOL_INDEX                             0
#define UMAC_PSE_CFG_POOL_INDEX                             1

#define UMAC_PG_HIF0_GROUP_0                                0
#define UMAC_PG_HIF1_GROUP_1                                1
#define UMAC_PG_CPU_GROUP_2                                 2
#define UMAC_PG_LMAC0_GROUP_3                               3
#define UMAC_PG_LMAC1_GROUP_4                               4
#define UMAC_PG_LMAC2_GROUP_5                               5
#define UMAC_PG_PLE_GROUP_6                                 6

#define UMAC_PBUF_CTRL_TOTAL_PAGE_NUM_MASK                  BITS(0, 11)
#define UMAC_PBUF_CTRL_TOTAL_PAGE_NUM_OFFSET                0

#define UMAC_FREEPG_CNT_FREEPAGE_CNT_MASK                   BITS(0, 11)
#define UMAC_FREEPG_CNT_FREEPAGE_CNT_OFFSET                 0

#define UMAC_FREEPG_CNT_FFA_CNT_MASK                        BITS(16, 27)
#define UMAC_FREEPG_CNT_FFA_CNT_OFFSET                      16

/*******************************************************************************
 *                   F U N C T I O N   D E C L A R A T I O N S
 *******************************************************************************
 */

OUT u_int8_t
halUmacInfoGetMiscStatus(IN struct ADAPTER *prAdapter,
			 IN struct UMAC_STAT2_GET *pUmacStat2Get);

#endif /* _NIC_UMAC_H */
