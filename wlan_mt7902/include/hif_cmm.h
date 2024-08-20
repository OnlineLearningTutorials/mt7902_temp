/* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause */
/*
 * Copyright (c) 2016 MediaTek Inc.
 */
/*! \file   "hif.h"
 *    \brief  Functions for the driver to register bus and setup the IRQ
 *
 *    Functions for the driver to register bus and setup the IRQ
 */


#ifndef _HIF_CMM_H
#define _HIF_CMM_H


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

/* Interface Type */
enum MT_INF_TYPE {
	MT_DEV_INF_UNKNOWN = 0,
	MT_DEV_INF_PCI = 1,
	MT_DEV_INF_USB = 2,
	MT_DEV_INF_RBUS = 4,
	MT_DEV_INF_PCIE = 5,
	MT_DEV_INF_SDIO = 6,
	MT_DEV_INF_EHPI = 7,
	MT_DEV_INF_AXI = 8,
    #ifdef UT_TEST_MODE
	MT_DEV_INF_UT = 10,
    #endif /* UT_TEST_MODE */
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
#define IS_SDIO_INF(__GlueInfo) \
	((__GlueInfo)->u4InfType == MT_DEV_INF_SDIO)
#define IS_USB_INF(__GlueInfo) \
	((__GlueInfo)->u4InfType == MT_DEV_INF_USB)
#define IS_PCIE_INF(__GlueInfo) \
	((__GlueInfo)->u4InfType == MT_DEV_INF_PCIE)
#define IS_EHPI_INF(__GlueInfo)	\
	((__GlueInfo)->u4InfType == MT_DEV_INF_PCIE)

#define HAL_WRITE_HIF_TXD(_prChipInfo, _pucOutputBuf, _u2InfoBufLen, _ucType) \
{ \
	uint16_t _u2DataLen = (uint16_t)(_u2InfoBufLen); \
	uint8_t _ucPacketType = (uint8_t)(_ucType); \
	uint8_t *_prBuf = (_pucOutputBuf); \
	if (_prChipInfo->fillHifTxDesc) \
		_prChipInfo->fillHifTxDesc(&_prBuf, &_u2DataLen, \
		_ucPacketType); \
}

/*******************************************************************************
 *                   F U N C T I O N   D E C L A R A T I O N S
 *******************************************************************************
 */


/*******************************************************************************
 *                              F U N C T I O N S
 *******************************************************************************
 */
#endif /* _HIF_CMM_H */
