/* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause */
/*
 * Copyright (c) 2019 MediaTek Inc.
 */
/*! \file   hal_wfsys_reset_mt7961.c
*    \brief  WFSYS reset HAL API for MT7961
*
*    This file contains all routines which are exported
     from MediaTek 802.11 Wireless LAN driver stack to GLUE Layer.
*/

#if defined(MT7961) || defined(MT7922) || defined(MT7902)

/*******************************************************************************
*                         C O M P I L E R   F L A G S
********************************************************************************
*/

/*******************************************************************************
*                    E X T E R N A L   R E F E R E N C E S
********************************************************************************
*/
#include "precomp.h"
#include "mt7961.h"
#include "coda/mt7961/cbtop_rgu.h"
#include "coda/mt7961/ssusb_epctl_csr.h"
#include "hal_wfsys_reset_mt7961.h"
#if defined(MT7961) || defined(MT7922)
#include "coda/mt7961/wf_wfdma_host_dma0.h"
#elif defined(MT7902)
#include "coda/mt7902/wf_wfdma_host_dma0.h"
#endif

#if CFG_CHIP_RESET_SUPPORT
/*******************************************************************************
*                              C O N S T A N T S
********************************************************************************
*/

/*******************************************************************************
*                             D A T A   T Y P E S
********************************************************************************
*/

/*******************************************************************************
*                            P U B L I C   D A T A
********************************************************************************
*/

/*******************************************************************************
*                           P R I V A T E   D A T A
********************************************************************************
*/

/*******************************************************************************
*                                 M A C R O S
********************************************************************************
*/

/*******************************************************************************
*                   F U N C T I O N   D E C L A R A T I O N S
********************************************************************************
*/

/*******************************************************************************
*                              F U N C T I O N S
********************************************************************************
*/
#if defined(_HIF_PCIE)

/*----------------------------------------------------------------------------*/
/*!
* @brief assert or de-assert WF subsys reset on CBTOP RGU
*
* @param prAdapter pointer to the Adapter handler
* @param fgAssertRst TRUE means assert reset and FALSE means de-assert reset
*
* @return TRUE means operation successfully and FALSE means fail
*/
/*----------------------------------------------------------------------------*/
u_int8_t mt7961HalCbtopRguWfRst(struct ADAPTER *prAdapter,
				u_int8_t fgAssertRst)
{
	uint32_t u4Val;
	u_int8_t fgIsWfdmaIdle = TRUE;
	u_int8_t ucPollingCount = 0;

	if (fgAssertRst) {

		/* check wfdma idle before reset */
		fgIsWfdmaIdle = mt7961HalPollWfdmaIdle(prAdapter);

		while (fgIsWfdmaIdle == FALSE && ucPollingCount < 10) {
			fgIsWfdmaIdle = mt7961HalPollWfdmaIdle(prAdapter);
			ucPollingCount += 1;
			kalMsleep(100);
		}

		if (ucPollingCount == 10) {
			DBGLOG(HAL, ERROR, "[SER][L0.5]SER Polling WFDMA Idle Fail!\n");
		}

		/* Set PCIE2AP public mapping CR4 */
		u4Val = (CONN_INFRA_BUS_CR_PCIE2AP_REMAP_2_DEFAULT & ~BITS(0, 15)) |
				(CBTOP_RGU_BASE >> 16);
		HAL_MCR_WR(prAdapter, CONN_INFRA_BUS_CR_PCIE2AP_REMAP_2_ADDR, u4Val);

		kalUdelay(2);

		/* Assert WF subsys reset via MMIO */
		u4Val = (CBTOP_RGU_WF_SUBSYS_RST_ADDR & ~BITS(16, 31)) |
				R_PCIE2AP_PUBLIC_REMAPPING_4_BUS_ADDR;
		HAL_MCR_WR(prAdapter, u4Val, 0x00000001);
	} else {
		/* De-assert WF subsys reset via MMIO */
		u4Val = (CBTOP_RGU_WF_SUBSYS_RST_ADDR & ~BITS(16, 31)) |
				R_PCIE2AP_PUBLIC_REMAPPING_4_BUS_ADDR;
		HAL_MCR_WR(prAdapter, u4Val, 0x00000000);

		/* Reset PCIE2AP public mapping CR4 */
		u4Val = CONN_INFRA_BUS_CR_PCIE2AP_REMAP_2_DEFAULT;
		HAL_MCR_WR(prAdapter, CONN_INFRA_BUS_CR_PCIE2AP_REMAP_2_ADDR, u4Val);
	}

	return TRUE;
}

/*----------------------------------------------------------------------------*/
/*!
* @brief polling if WF subsys is SW_INIT_DONE
*
* @param prAdapter pointer to the Adapter handler
*
* @return TRUE means WF subsys is SW_INIT_DONE and ready for driver probe
*         procedure; otherwise, return FALSE
*/
/*----------------------------------------------------------------------------*/
u_int8_t mt7961HalPollWfsysSwInitDone(struct ADAPTER *prAdapter)
{
	uint32_t u4CrValue = 0;
	uint32_t u4ResetTimeCnt = 0, u4ResetTimeTmout = 2;
	u_int8_t fgSwInitDone = TRUE;

	/* Polling until WF WM is done */
	while (TRUE) {

		HAL_MCR_RD(prAdapter, CONN_INFRA_RGU_WFSYS_SW_RST_B_ADDR, &u4CrValue);

		if (u4CrValue == MMIO_READ_FAIL)
			DBGLOG(HAL, ERROR, "[SER][L0.5] MMIO read CR fail\n");
		else if (u4CrValue & CONN_INFRA_RGU_WFSYS_SW_RST_B_WFSYS_SW_INIT_DONE_MASK)
			break;

		if (u4ResetTimeCnt >= u4ResetTimeTmout) {
			DBGLOG(INIT, ERROR, "[SER][L0.5] Poll Sw Init Done FAIL\n");
			fgSwInitDone = FALSE;
			break;
		}

		kalMsleep(100);
		u4ResetTimeCnt++;
	}

	return fgSwInitDone;
}

u_int8_t mt7961HalPollWfdmaIdle(struct ADAPTER *prAdapter)
{
	uint32_t u4WfdmaGloCfgExt2 = 0;
	uint32_t u4WfdmaGloCfg = 0;
	uint32_t u4WfdmaHifBusyInt = 0;
	uint32_t u4WfdmaHifBusyExt = 0;
	u_int8_t fgIsIdle = TRUE;

	HAL_MCR_RD(prAdapter,
		   WF_WFDMA_HOST_DMA0_WPDMA_GLO_CFG_EXT2_ADDR,
		   &u4WfdmaGloCfgExt2);
	u4WfdmaGloCfgExt2 |=
		WF_WFDMA_HOST_DMA0_WPDMA_GLO_CFG_EXT2_CSR_TX_HALT_MODE_MASK;
	HAL_MCR_WR(prAdapter,
		   WF_WFDMA_HOST_DMA0_WPDMA_GLO_CFG_EXT2_ADDR,
		   u4WfdmaGloCfgExt2);

	HAL_MCR_RD(prAdapter,
		WF_WFDMA_HOST_DMA0_WPDMA_GLO_CFG_ADDR, &u4WfdmaGloCfg);
	HAL_MCR_RD(prAdapter,
		WF_WFDMA_INT_WRAP_CSR_WFDMA_HIF_MISC_ADDR,
		&u4WfdmaHifBusyInt);
	HAL_MCR_RD(prAdapter,
		WF_WFDMA_EXT_WRAP_CSR_WFDMA_HIF_MISC_ADDR,
		&u4WfdmaHifBusyExt);

	DBGLOG(HAL, INFO, "[SER][L0.5] HOST_DMA0_WPDMA_GLO_CFG"
		"[0x%08X] = 0x%08X\n",
		WF_WFDMA_HOST_DMA0_WPDMA_GLO_CFG_ADDR, u4WfdmaGloCfg);
	DBGLOG(HAL, INFO, "[SER][L0.5] INT_WRAP_CSR_WFDMA_HIF_MISC"
		"[0x%08X] = 0x%08X\n",
		WF_WFDMA_INT_WRAP_CSR_WFDMA_HIF_MISC_ADDR, u4WfdmaHifBusyInt);
	DBGLOG(HAL, INFO, "[SER][L0.5] EXT_WRAP_CSR_WFDMA_HIF_MISC"
		"[0x%08X] = 0x%08X\n",
		WF_WFDMA_EXT_WRAP_CSR_WFDMA_HIF_MISC_ADDR, u4WfdmaHifBusyExt);

	if (u4WfdmaGloCfg &
		WF_WFDMA_HOST_DMA0_WPDMA_GLO_CFG_TX_DMA_BUSY_MASK ||
		u4WfdmaGloCfg &
		WF_WFDMA_HOST_DMA0_WPDMA_GLO_CFG_RX_DMA_BUSY_MASK ||
		u4WfdmaHifBusyInt &
		WF_WFDMA_INT_WRAP_CSR_WFDMA_HIF_MISC_HIF_BUSY_MASK ||
		u4WfdmaHifBusyExt &
		WF_WFDMA_EXT_WRAP_CSR_WFDMA_HIF_MISC_HIF_BUSY_MASK)
	{
		DBGLOG(HAL, ERROR, "[SER][L0.5] WFDMA BUSY!");
		fgIsIdle = FALSE;
	}

	return fgIsIdle;
}

#endif /* defined(_HIF_PCIE) */

#if defined(_HIF_USB)

/*----------------------------------------------------------------------------*/
/*!
* @brief assert or de-assert WF subsys reset on CBTOP RGU
*
* @param prAdapter pointer to the Adapter handler
* @param fgAssertRst TRUE means assert reset and FALSE means de-assert reset
*
* @return TRUE means operation successfully and FALSE means fail
*/
/*----------------------------------------------------------------------------*/
u_int8_t mt7961HalCbtopRguWfRst(struct ADAPTER *prAdapter,
				u_int8_t fgAssertRst)
{
	uint32_t u4CrVal;
	u_int8_t fgStatus;

	HAL_UHW_RD(prAdapter, CBTOP_RGU_WF_SUBSYS_RST_ADDR, &u4CrVal,
		   &fgStatus);
	if (!fgStatus) {
		DBGLOG(HAL, ERROR, "UHW read CBTOP RGU CR fail\n");

		goto end;
	}

	if (fgAssertRst) {
		/* CBTOP_RGU_WF_SUBSYS_RST_BYPASS_WFDMA_SLP_PROT_MASK is defined
		 * since MT7922. For prior project like MT7961, it's undefined
		 * and doesn't have any effect if this bit is set.
		 */
		u4CrVal |= CBTOP_RGU_WF_SUBSYS_RST_BYPASS_WFDMA_SLP_PROT_MASK;
		u4CrVal |= CBTOP_RGU_WF_SUBSYS_RST_WF_WHOLE_PATH_RST_MASK;
		HAL_UHW_WR(prAdapter, CBTOP_RGU_WF_SUBSYS_RST_ADDR, u4CrVal,
			   &fgStatus);
	} else {
		u4CrVal &= ~CBTOP_RGU_WF_SUBSYS_RST_WF_WHOLE_PATH_RST_MASK;
		HAL_UHW_WR(prAdapter, CBTOP_RGU_WF_SUBSYS_RST_ADDR, u4CrVal,
			   &fgStatus);
	}

	if (!fgStatus) {
		DBGLOG(HAL, ERROR, "UHW write CBTOP RGU CR fail\n");

		goto end;
	}

end:
	return fgStatus;
}

/*----------------------------------------------------------------------------*/
/*!
* @brief set USB endpoint reset scope
*
* @param prAdapter pointer to the Adapter handler
* @param fgIsRstScopeIncludeToggleBit TRUE means reset scope including toggle
*                                     bit, sequence number, etc
*                                     FALSE means exclusion
*
* @return TRUE means operation successfully and FALSE means fail
*/
/*----------------------------------------------------------------------------*/
u_int8_t mt7961HalUsbEpctlRstOpt(struct ADAPTER *prAdapter,
			     u_int8_t fgIsRstScopeIncludeToggleBit)
{
	uint32_t u4CrVal;
	u_int8_t fgStatus;

	HAL_UHW_RD(prAdapter, SSUSB_EPCTL_CSR_EP_RST_OPT_ADDR, &u4CrVal,
		   &fgStatus);

	if (!fgStatus) {
		DBGLOG(HAL, ERROR, "UHW read USB CR fail\n");

		return fgStatus;
	}

	if (fgIsRstScopeIncludeToggleBit) {
		/* BITS(4, 9) represents BULK OUT EP4 ~ EP9
		 * BITS(20, 22) represents BULK IN EP4 ~ EP5, INT IN EP6
		 */
		u4CrVal |= (BITS(4, 9) | BITS(20, 22));
		HAL_UHW_WR(prAdapter, SSUSB_EPCTL_CSR_EP_RST_OPT_ADDR, u4CrVal,
			   &fgStatus);
	} else {
		/* BITS(4, 9) represents BULK OUT EP4 ~ EP9
		 * BITS(20, 22) represents BULK IN EP4 ~ EP5, INT IN EP6
		 */
		u4CrVal &= ~(BITS(4, 9) | BITS(20, 22));
		HAL_UHW_WR(prAdapter, SSUSB_EPCTL_CSR_EP_RST_OPT_ADDR, u4CrVal,
			   &fgStatus);
	}

	if (!fgStatus)
		DBGLOG(HAL, ERROR, "UHW write USB CR fail\n");

	return fgStatus;
}

/*----------------------------------------------------------------------------*/
/*!
* @brief polling if WF subsys is SW_INIT_DONE
*
* @param prAdapter pointer to the Adapter handler
*
* @return TRUE means WF subsys is SW_INIT_DONE and ready for driver probe
*         procedure; otherwise, return FALSE
*/
/*----------------------------------------------------------------------------*/
u_int8_t mt7961HalPollWfsysSwInitDone(struct ADAPTER *prAdapter)
{
	struct BUS_INFO *prBusInfo;
	uint32_t u4CrValue;
	uint32_t u4ResetTimeCnt = 0, u4ResetTimeTmout = 2;
	u_int8_t fgStatus;
	u_int8_t fgSwInitDone = TRUE;

#define CONN_INFRA_SYSSTRAP_WFSYS_SW_INIT_DONE_MASK        0x00400000

	prBusInfo = prAdapter->chip_info->bus_info;

	/* select conn_infra sysstrap */
	HAL_UHW_WR(prAdapter, prBusInfo->u4UdmaConnInfraStatusSelAddr,
		   prBusInfo->u4UdmaConnInfraStatusSelVal, &fgStatus);

	if (!fgStatus)
		DBGLOG(HAL, ERROR, "UHW write UDMA CR fail\n");

	/* polling until WF WM is ready */
	u4CrValue = 0;
	while (TRUE) {
		HAL_UHW_RD(prAdapter, prBusInfo->u4UdmaConnInfraStatusAddr,
			   &u4CrValue, &fgStatus);

		if (!fgStatus)
			DBGLOG(HAL, ERROR, "UHW read UDMA CR fail\n");
		else if (u4CrValue &
			 CONN_INFRA_SYSSTRAP_WFSYS_SW_INIT_DONE_MASK)
			break;

		if (u4ResetTimeCnt >= u4ResetTimeTmout) {
			DBGLOG(INIT, ERROR,
			       "L0.5 Reset polling sw init done timeout\n");

			fgSwInitDone = FALSE;

			break;
		}

		kalMsleep(100);
		u4ResetTimeCnt++;
	}

	return fgSwInitDone;
}

#endif /* defined(_HIF_USB) */

#if defined(_HIF_SDIO)

/*----------------------------------------------------------------------------*/
/*!
* @brief assert or de-assert WF subsys reset on CBTOP RGU
*
* @param prAdapter pointer to the Adapter handler
* @param fgAssertRst TRUE means assert reset and FALSE means de-assert reset
*
* @return TRUE means operation successfully and FALSE means fail
*/
/*----------------------------------------------------------------------------*/
u_int8_t mt7961HalCbtopRguWfRst(struct ADAPTER *prAdapter,
				u_int8_t fgAssertRst)
{
	uint32_t u4Val = 0;

	HAL_MCR_RD(prAdapter, MCR_WHCR, &u4Val);

	if (fgAssertRst) {
		/* trigger rst */
		u4Val &= ~WHCR_WF_WHOLE_PATH_RSTB;
		HAL_MCR_WR(prAdapter, MCR_WHCR, u4Val);

	} else {
		/* clear rst */
		u4Val |= WHCR_WF_WHOLE_PATH_RSTB;
		HAL_MCR_WR(prAdapter, MCR_WHCR, u4Val);
	}

	if (fgIsBusAccessFailed == TRUE) {
		DBGLOG(INIT, ERROR,
			"[SER][L0.5] fgIsBusAccessFailed=%d fgAssertRst=%d\n",
				fgIsBusAccessFailed, fgAssertRst);
		return FALSE;
	} else
		return TRUE;

}

/*----------------------------------------------------------------------------*/
/*!
* @brief polling if WF subsys is SW_INIT_DONE
*
* @param prAdapter pointer to the Adapter handler
*
* @return TRUE means WF subsys is SW_INIT_DONE and ready for driver probe
*         procedure; otherwise, return FALSE
*/
/*----------------------------------------------------------------------------*/
u_int8_t mt7961HalPollWfsysSwInitDone(struct ADAPTER *prAdapter)
{
#define SER_L05_RST_DONE_TIMEOUT        (2000)

	uint32_t u4CrValue1 = 0, u4Tick;
	u_int8_t fgSwInitDone = TRUE;


	HAL_MCR_RD(prAdapter, MCR_WHCR, &u4CrValue1);
	/* polling until WF WM is ready */
	u4Tick = kalGetTimeTick();
	while ((u4CrValue1 & WHCR_WF_RST_DONE) == FALSE) {
		kalMsleep(50);
		HAL_MCR_RD(prAdapter, MCR_WHCR, &u4CrValue1);

		/* timeout handle */
		if ((CHECK_FOR_TIMEOUT(kalGetTimeTick(), u4Tick,
				MSEC_TO_SYSTIME(SER_L05_RST_DONE_TIMEOUT)))) {
			fgSwInitDone = FALSE;
			DBGLOG(INIT, ERROR,
				"[SER] L05 - polling sw init done timeout\n");
			break;
		}
	}

	return fgSwInitDone;
}

u_int8_t mt7961HalSetNoBTFwOwnEn(IN int32_t i4Enable)
{
	typedef int32_t (*p_bt_fun_type) (int32_t);
	p_bt_fun_type bt_func = NULL;
	char *bt_func_name = "btmtk_sdio_set_driver_own_for_subsys_reset";
	uint32_t u4Res = 0;
        void *pvAddr = NULL;

#if	(CFG_ENABLE_GKI_SUPPORT != 1)
	DBGLOG(INIT, STATE, "[SER][L0.5] %s, i4Enable=%d\n",
						bt_func_name, i4Enable);
	pvAddr = GLUE_SYMBOL_GET(bt_func_name);
#else
#ifdef CFG_CHIP_RESET_KO_SUPPORT
	struct BT_NOTIFY_DESC *bt_notify_desc = NULL;

	bt_notify_desc = get_bt_notify_callback();
	pvAddr = bt_notify_desc->WifiNotifyBtSubResetStep1;
#endif /* CFG_CHIP_RESET_KO_SUPPORT */
#endif
	if (pvAddr) {
		bt_func = (p_bt_fun_type) pvAddr;
		u4Res = bt_func(i4Enable);
#if	(CFG_ENABLE_GKI_SUPPORT != 1)
		GLUE_SYMBOL_PUT(bt_func_name);
#endif
		if (u4Res != 0) {
			DBGLOG(INIT, ERROR, "[SER][L0.5] %s fail u4Res=%d\n",
							bt_func_name, u4Res);
			return FALSE;
		}
	} else {
		DBGLOG(INIT, ERROR,
			"[SER][L0.5] %s does not exist\n", bt_func_name);
		return FALSE;
	}

	return TRUE;
}

#endif /* defined(_HIF_SDIO) */


/* Provide dummy functions for none-OS build */
#if !defined(_HIF_PCIE) && !defined(_HIF_USB) && !defined(_HIF_SDIO)
u_int8_t mt7961HalCbtopRguWfRst(struct ADAPTER *prAdapter,
				u_int8_t fgAssertRst)
{
	return TRUE;
}
u_int8_t mt7961HalPollWfsysSwInitDone(struct ADAPTER *prAdapter)
{
	return TRUE;
}
#endif

#endif /* CFG_CHIP_RESET_SUPPORT */
#endif /* defined(MT7961) || defined(MT7922) || defined(MT7902) */
