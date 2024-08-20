/* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause */
/*
 * Copyright (c) 2016 MediaTek Inc.
 */
/*
 ** Id: @(#) gl_rst.c@@
 */

/*! \file   gl_rst.c
 *    \brief  Main routines for supporintg MT6620 whole-chip reset mechanism
 *
 *    This file contains the support routines of Linux driver for MediaTek Inc.
 *    802.11 Wireless LAN Adapters.
 */


/*******************************************************************************
 *                         C O M P I L E R   F L A G S
 *******************************************************************************
 */

/*******************************************************************************
 *                    E X T E R N A L   R E F E R E N C E S
 *******************************************************************************
 */
#include <linux/kernel.h>
#include <linux/workqueue.h>

#include "precomp.h"
#include "gl_rst.h"

/*******************************************************************************
 *                              C O N S T A N T S
 *******************************************************************************
 */



/*******************************************************************************
 *                            P U B L I C   D A T A
 *******************************************************************************
 */
u_int8_t fgSimplifyResetFlow = FALSE;
uint64_t u8ResetTime;
u_int8_t fgIsRstPreventFwOwn = FALSE;

#if CFG_CHIP_RESET_HANG
u_int8_t fgIsResetHangState = SER_L0_HANG_RST_NONE;
#endif

#if (CFG_SUPPORT_CONNINFRA == 1)
uint32_t g_u4WlanRstThreadPid;
wait_queue_head_t g_waitq_rst;
unsigned long g_ulFlag;/* GLUE_FLAG_XXX */
struct completion g_RstOffComp;
struct completion g_RstOnComp;
struct completion g_triggerComp;
KAL_WAKE_LOCK_T *g_IntrWakeLock = NULL;
struct task_struct *wlan_reset_thread;
static int g_rst_data;
u_int8_t g_IsWholeChipRst = FALSE;
u_int8_t g_SubsysRstCnt;
int g_SubsysRstTotalCnt;
int g_WholeChipRstTotalCnt;
bool g_IsTriggerTimeout = FALSE;
u_int8_t g_IsSubsysRstOverThreshold = FALSE;
u_int8_t g_IsWfsysBusHang = FALSE;
char *g_reason;
enum consys_drv_type g_WholeChipRstType;
char *g_WholeChipRstReason;
u_int8_t g_IsWfsysResetOnFail = FALSE;
u_int8_t g_IsWfsysRstDone = TRUE;
u_int8_t g_fgRstRecover = FALSE;
#endif


/*******************************************************************************
 *                           P R I V A T E   D A T A
 *******************************************************************************
 */
static enum _ENUM_CHIP_RESET_REASON_TYPE_T eResetReason;

#if CFG_CHIP_RESET_SUPPORT
static struct RESET_STRUCT wifi_rst;
u_int8_t fgIsResetting = FALSE;
enum ENUM_BT_RESET_RETURN_STATE {
	BT_RESET_OK = 0,
	BT_RESET_NOT_OK,
	BT_RESET_NOT_FOUND
};
#endif

/*******************************************************************************
 *                   F U N C T I O N   D E C L A R A T I O N S
 *******************************************************************************
 */
#if CFG_CHIP_RESET_SUPPORT
#if (CFG_SUPPORT_CONNINFRA == 0)
static void mtk_wifi_reset(struct work_struct *work);
#endif

#if CFG_WMT_RESET_API_SUPPORT
static void mtk_wifi_trigger_reset(struct work_struct *work);
static void *glResetCallback(enum ENUM_WMTDRV_TYPE eSrcType,
			     enum ENUM_WMTDRV_TYPE eDstType,
			     enum ENUM_WMTMSG_TYPE eMsgType, void *prMsgBody,
			     unsigned int u4MsgLength);
#else
#ifndef CFG_CHIP_RESET_KO_SUPPORT
static u_int8_t is_bt_exist(void);
static u_int8_t rst_L0_notify_step1(void);
static void wait_core_dump_end(void);
#endif /* CFG_CHIP_RESET_KO_SUPPORT */
#endif
#endif

/*******************************************************************************
 *                              F U N C T I O N S
 *******************************************************************************
 */
void glSetRstReason(enum _ENUM_CHIP_RESET_REASON_TYPE_T
		    eReason)
{
	if (kalIsResetting())
		return;

	u8ResetTime = sched_clock();
	eResetReason = eReason;
}

int glGetRstReason(void)
{
	return eResetReason;
}

u_int8_t kalIsRstPreventFwOwn(void)
{
#if CFG_CHIP_RESET_SUPPORT
	return fgIsRstPreventFwOwn;
#else
	return FALSE;
#endif

}

/*----------------------------------------------------------------------------*/
/*!
 * @brief This routine is called for checking if connectivity chip is resetting
 *
 * @param   None
 *
 * @retval  TRUE
 *          FALSE
 */
/*----------------------------------------------------------------------------*/
u_int8_t kalIsResetting(void)
{
#if CFG_CHIP_RESET_SUPPORT
	return fgIsResetting;
#else
	return FALSE;
#endif
}

#if CFG_CHIP_RESET_SUPPORT

/*----------------------------------------------------------------------------*/
/*!
 * @brief This routine is responsible for
 *        1. register wifi reset callback
 *        2. initialize wifi reset work
 *
 * @param none
 *
 * @retval none
 */
/*----------------------------------------------------------------------------*/
void glResetInit(struct GLUE_INFO *prGlueInfo)
{
#if (CFG_SUPPORT_CONNINFRA == 0)
#if CFG_WMT_RESET_API_SUPPORT
	/* 1. Register reset callback */
	mtk_wcn_wmt_msgcb_reg(WMTDRV_TYPE_WIFI,
			      (PF_WMT_CB) glResetCallback);
	/* 2. Initialize reset work */
	INIT_WORK(&(wifi_rst.rst_trigger_work),
		  mtk_wifi_trigger_reset);
#endif /* CFG_WMT_RESET_API_SUPPORT */

#ifdef CFG_CHIP_RESET_KO_SUPPORT
#if defined(_HIF_SDIO)
	struct WIFI_NOTIFY_DESC wifi_notify_desc;

	wifi_notify_desc.BtNotifyWifiSubResetStep1 = NULL;
	if (prGlueInfo == NULL || prGlueInfo->prAdapter == NULL
		|| prGlueInfo->prAdapter->chip_info == NULL) {
		DBGLOG(INIT, ERROR, "[SER][L0] reset init fail!\n");
		return;
	}
	if (prGlueInfo->prAdapter->chip_info->asicSetNoBTFwOwnEn != NULL) {
		wifi_notify_desc.BtNotifyWifiSubResetStep1 = halPreventFwOwnEn;
		register_wifi_notify_callback(&wifi_notify_desc);
	}
#endif /* _HIF_SDIO */
#endif /* CFG_CHIP_RESET_KO_SUPPORT */

	INIT_WORK(&(wifi_rst.rst_work), mtk_wifi_reset);
#endif /* CFG_SUPPORT_CONNINFRA == 0 */

	fgIsResetting = FALSE;
	fgIsRstPreventFwOwn = FALSE;
	wifi_rst.prGlueInfo = prGlueInfo;
#if (CFG_SUPPORT_CONNINFRA == 1)

	update_driver_reset_status(fgIsResetting);
	KAL_WAKE_LOCK_INIT(NULL, g_IntrWakeLock, "WLAN Reset");
	init_waitqueue_head(&g_waitq_rst);
	init_completion(&g_RstOffComp);
	init_completion(&g_RstOnComp);
	init_completion(&g_triggerComp);
	wlan_reset_thread = kthread_run(wlan_reset_thread_main,
					&g_rst_data, "wlan_rst_thread");
	g_SubsysRstCnt = 0;

#endif /* CFG_SUPPORT_CONNINFRA */
}

/*----------------------------------------------------------------------------*/
/*!
 * @brief This routine is responsible for
 *        1. deregister wifi reset callback
 *
 * @param none
 *
 * @retval none
 */
/*----------------------------------------------------------------------------*/
void glResetUninit(void)
{
#if CFG_WMT_RESET_API_SUPPORT
	/* 1. Deregister reset callback */
#if (CFG_SUPPORT_CONNINFRA == 0)
	mtk_wcn_wmt_msgcb_unreg(WMTDRV_TYPE_WIFI);
#else /* CFG_SUPPORT_CONNINFRA == 0 */
	set_bit(GLUE_FLAG_HALT_BIT, &g_ulFlag);
	wake_up_interruptible(&g_waitq_rst);
	KAL_WAKE_LOCK_DESTROY(NULL, g_IntrWakeLock);
#endif /* CFG_SUPPORT_CONNINFRA == 0 */
#endif /* CFG_WMT_RESET_API_SUPPORT */

#if (CFG_SUPPORT_CONNINFRA == 0)
#ifdef CFG_CHIP_RESET_KO_SUPPORT
#if defined(_HIF_SDIO)
	unregister_wifi_notify_callback();
#endif /* _HIF_SDIO */
#endif /* CFG_CHIP_RESET_KO_SUPPORT */
#endif /* CFG_SUPPORT_CONNINFRA == 0 */
}
/*----------------------------------------------------------------------------*/
/*!
 * @brief This routine is called for generating reset request to WMT
 *
 * @param   None
 *
 * @retval  None
 */
/*----------------------------------------------------------------------------*/
void glSendResetRequest(void)
{
#if CFG_WMT_RESET_API_SUPPORT

	/* WMT thread would trigger whole chip reset itself */
#endif
}

#if (CFG_WMT_RESET_API_SUPPORT == 1) || (CFG_SUPPORT_CONNINFRA == 1)
/*----------------------------------------------------------------------------*/
/*!
 * @brief For mobile project, decide reset action according to trigger reset
 *        reason.
 *
 * @param   prAdapter
 *
 * @retval  reset action like RST_FLAG_DO_CORE_DUMP,
 *          or RST_FLAG_PREVENT_POWER_OFF, ..., etc.
 */
/*----------------------------------------------------------------------------*/
uint32_t glResetSelectActionMobile(IN struct ADAPTER *prAdapter)
{
	struct mt66xx_chip_info *prChipInfo;
	uint32_t u4RstFlag = RST_FLAG_DO_WHOLE_RESET;

	prChipInfo = prAdapter->chip_info;

	switch (glGetRstReason()) {
	case RST_PROCESS_ABNORMAL_INT:
	case RST_GROUP4_NULL:
	case RST_TX_ERROR:
	case RST_RX_ERROR:
		u4RstFlag = RST_FLAG_DO_CORE_DUMP;
		break;

	case RST_DRV_OWN_FAIL:
#if defined(_HIF_PCIE) || defined(_HIF_AXI)
#if (CFG_SUPPORT_CONNINFRA == 0)
		u4RstFlag = RST_FLAG_DO_WHOLE_RESET;
#else
		u4RstFlag = RST_FLAG_WF_RESET;
#endif
#elif defined(_HIF_SDIO)
		u4RstFlag = RST_FLAG_DO_CORE_DUMP;
#endif
		break;

	case RST_CR_ACCESS_FAIL:
		u4RstFlag = RST_FLAG_DO_CORE_DUMP | RST_FLAG_PREVENT_POWER_OFF;
		break;

	case RST_FW_ASSERT_DONE:
		/* L0.5 reset will be triggered automatically by WDT event. */
		if (prChipInfo->fgIsSupportL0p5Reset)
			u4RstFlag = 0;
		break;

	case RST_FW_ASSERT_TIMEOUT:
	case RST_OID_TIMEOUT:
	case RST_CMD_EVT_FAIL:
	case RST_SER_L1_FAIL:
	case RST_WDT:
		if (prChipInfo->fgIsSupportL0p5Reset)
			u4RstFlag = RST_FLAG_DO_L0P5_RESET;
		break;

	case RST_BT_TRIGGER:
	case RST_CMD_TRIGGER:
	case RST_SER_L0P5_FAIL:
	default:
		u4RstFlag = RST_FLAG_DO_WHOLE_RESET;
		break;
	}

	if ((u4RstFlag & RST_FLAG_DO_WHOLE_RESET) &&
	    !prAdapter->rWifiVar.fgEnableSerL0) {
		DBGLOG(INIT, WARN,
		       "[SER][L0] Bypass L0 reset due to wifi.cfg\n");

		u4RstFlag &= ~RST_FLAG_DO_WHOLE_RESET;
	}

	if ((u4RstFlag & RST_FLAG_DO_L0P5_RESET) &&
	    prAdapter->rWifiVar.eEnableSerL0p5 != FEATURE_OPT_SER_ENABLE) {
		DBGLOG(INIT, WARN,
		       "[SER][L0.5] Bypass L0.5 reset due to wifi.cfg\n");

		u4RstFlag &= ~RST_FLAG_DO_L0P5_RESET;
	}

	return u4RstFlag;
}

/*----------------------------------------------------------------------------*/
/*!
 * @brief Reset trigger MOBILE flow
 *
 * @param   prAddapter
 *          u4RstFlag  specify reset option
 *
 * @retval  none
 */
/*----------------------------------------------------------------------------*/
void glResetTriggerMobile(IN struct ADAPTER *prAdapter, IN uint32_t u4RstFlag)
{
#if (CFG_SUPPORT_CONNINFRA == 1)
	struct mt66xx_chip_info *prChipInfo;
#endif

	fgIsResetting = TRUE;
#if (CFG_SUPPORT_CONNINFRA == 1)
	prChipInfo = prAdapter->chip_info;

	update_driver_reset_status(fgIsResetting);
#endif

#if CFG_WMT_RESET_API_SUPPORT
	if (u4RstFlag & RST_FLAG_DO_CORE_DUMP)
		if (glIsWmtCodeDump())
			DBGLOG(INIT, WARN, "WMT is code dumping !\n");

#if (CFG_SUPPORT_CONNINFRA == 0)
	wifi_rst.rst_trigger_flag = u4RstFlag;
	schedule_work(&(wifi_rst.rst_trigger_work));
#else
	if (u4RstFlag & RST_FLAG_DO_WHOLE_RESET) {
		if (prChipInfo->trigger_wholechiprst)
			prChipInfo->trigger_wholechiprst(g_reason);
	} else {
		if (prChipInfo->triggerfwassert)
			prChipInfo->triggerfwassert();
	}
#endif /*end of CFG_SUPPORT_CONNINFRA == 0*/
#endif
}
#else
/*----------------------------------------------------------------------------*/
/*!
 * @brief For CE project, decide reset action according to trigger reset reason.
 *
 * @param   prAdapter
 *
 * @retval  reset action like RST_FLAG_DO_CORE_DUMP,
 *          or RST_FLAG_PREVENT_POWER_OFF, ..., etc.
 */
/*----------------------------------------------------------------------------*/
uint32_t glResetSelectActionCe(IN struct ADAPTER *prAdapter)
{
	struct mt66xx_chip_info *prChipInfo;
	uint32_t u4RstFlag = 0;

	prChipInfo = prAdapter->chip_info;

	switch (glGetRstReason()) {
	case RST_PROCESS_ABNORMAL_INT:
#if defined(_HIF_PCIE) || defined(_HIF_AXI)
		u4RstFlag = RST_FLAG_DO_L1_RESET;
#elif defined(_HIF_SDIO)
		if (prChipInfo->fgIsSupportL0p5Reset)
			u4RstFlag = RST_FLAG_DO_L0P5_RESET;
		else
			u4RstFlag = RST_FLAG_DO_WHOLE_RESET;
#else
		/* print log but don't execute SER */
		DBGLOG(INIT, WARN,
		       "USB shall not happen abnormal interrupt\n");
#endif
		break;

	case RST_DRV_OWN_FAIL:
#if defined(_HIF_PCIE) || defined(_HIF_AXI) || defined(_HIF_SDIO)
		if (prChipInfo->fgIsSupportL0p5Reset)
			u4RstFlag = RST_FLAG_DO_L0P5_RESET;
		else
			u4RstFlag = RST_FLAG_DO_WHOLE_RESET;
#else
		/* print log but don't execute SER */
		DBGLOG(INIT, WARN,
		       "USB shall not happen set drv own fail\n");
#endif
		break;

	case RST_FW_ASSERT_DONE:
		/* If L0.5 is supported, then L0.5 reset will be triggered
		 * automatically by RST_WDT. Otherwise, execute L0 reset.
		 */
		if (!prChipInfo->fgIsSupportL0p5Reset)
			u4RstFlag = RST_FLAG_DO_WHOLE_RESET;
		break;

	case RST_RX_ERROR:
#if defined(_HIF_SDIO)
		if (prChipInfo->fgIsSupportL0p5Reset)
			u4RstFlag = RST_FLAG_DO_L0P5_RESET;
		else
			u4RstFlag = RST_FLAG_DO_WHOLE_RESET;
#else
		/* When 20201225, HIF other than SDIO doesn't have this error.
		 * If this isn't TRUE in the future, then we need to decide
		 * the appropriate reset at that time.
		 */
#endif
		break;

	case RST_FW_ASSERT_TIMEOUT:
	case RST_OID_TIMEOUT:
	case RST_CMD_EVT_FAIL:
	case RST_WDT:
	case RST_SER_L1_FAIL:
		if (prChipInfo->fgIsSupportL0p5Reset)
			u4RstFlag = RST_FLAG_DO_L0P5_RESET;
		else
			u4RstFlag = RST_FLAG_DO_WHOLE_RESET;
		break;

	case RST_BT_TRIGGER:
	case RST_CR_ACCESS_FAIL:
	case RST_CMD_TRIGGER:
	case RST_SER_L0P5_FAIL:
	default:
		u4RstFlag = RST_FLAG_DO_WHOLE_RESET;
		break;
	}

#if defined(_HIF_PCIE) || defined(_HIF_AXI)
	if ((u4RstFlag & RST_FLAG_DO_L1_RESET) &&
	    prAdapter->rWifiVar.eEnableSerL1 != FEATURE_OPT_SER_ENABLE) {
		DBGLOG(INIT, INFO,
		       "[SER][L1] Bypass L1 reset due to wifi.cfg\n");

		if (prChipInfo->fgIsSupportL0p5Reset) {
			DBGLOG(INIT, INFO,
			       "[SER][L1] Try L0.5 reset instead\n");

			u4RstFlag = RST_FLAG_DO_L0P5_RESET;
		} else {
			DBGLOG(INIT, INFO,
			       "[SER][L1] Try L0 reset instead\n");

			u4RstFlag = RST_FLAG_DO_WHOLE_RESET;
		}
	}
#endif

	if ((u4RstFlag & RST_FLAG_DO_L0P5_RESET) &&
	    prAdapter->rWifiVar.eEnableSerL0p5 != FEATURE_OPT_SER_ENABLE) {
		DBGLOG(INIT, INFO,
		       "[SER][L0.5] Bypass L0.5 reset due to wifi.cfg\n");
		DBGLOG(INIT, INFO,
		       "[SER][L0.5] Try L0 reset instead\n");

		u4RstFlag = RST_FLAG_DO_WHOLE_RESET;
	}

	if ((u4RstFlag & RST_FLAG_DO_WHOLE_RESET) &&
	    !prAdapter->rWifiVar.fgEnableSerL0) {
		DBGLOG(INIT, INFO,
		       "[SER][L0] Bypass L0 reset due to wifi.cfg\n");

		u4RstFlag = 0;
	}

	return u4RstFlag;
}

/*----------------------------------------------------------------------------*/
/*!
 * @brief Reset trigger CE flow
 *
 * @param   prAddapter
 *          u4RstFlag  specify reset option
 *
 * @retval  none
 */
/*----------------------------------------------------------------------------*/
void glResetTriggerCe(IN struct ADAPTER *prAdapter, IN uint32_t u4RstFlag)
{
	if (u4RstFlag & RST_FLAG_DO_WHOLE_RESET) {
		fgIsResetting = TRUE;
		fgSimplifyResetFlow = FALSE;
		wifi_rst.prGlueInfo = prAdapter->prGlueInfo;
		schedule_work(&(wifi_rst.rst_work));
	} else if ((u4RstFlag & RST_FLAG_DO_L0P5_RESET) &&
		   prAdapter->chip_info->fgIsSupportL0p5Reset) {
		spin_lock_bh(&prAdapter->rWfsysResetLock);
		fgIsResetting = TRUE;
		fgIsRstPreventFwOwn = TRUE;

		if (prAdapter->eWfsysResetState == WFSYS_RESET_STATE_IDLE) {
			spin_unlock_bh(&prAdapter->rWfsysResetLock);

			glSetWfsysResetState(prAdapter,
					     WFSYS_RESET_STATE_DETECT);

			schedule_work(&prAdapter->prGlueInfo->rWfsysResetWork);
		} else
			spin_unlock_bh(&prAdapter->rWfsysResetLock);
	} else if (u4RstFlag & RST_FLAG_DO_L1_RESET) {
		wlanoidSerExtCmd(prAdapter, SER_ACTION_RECOVER,
				 SER_SET_L1_RECOVER, 0);
	}
}
#endif

/*----------------------------------------------------------------------------*/
/*!
 * @brief Reset trigger common flow for both MOBILE and CE
 *
 * @param   prAddapter
 *          u4RstFlag  specify reset option
 *          pucFile  reset is triggered at which file
 *          u4Line  reset is triggered at which line
 *
 * @retval  none
 */
/*----------------------------------------------------------------------------*/
void glResetTriggerCommon(struct ADAPTER *prAdapter, uint32_t u4RstFlag,
			  const uint8_t *pucFile, uint32_t u4Line)
{
	struct CHIP_DBG_OPS *prChipDbg;
	uint16_t u2FwOwnVersion;
	uint16_t u2FwPeerVersion;
	static const char *const apcReason[RST_REASON_MAX] = {
		"RST_UNKNOWN",
		"RST_PROCESS_ABNORMAL_INT",
		"RST_DRV_OWN_FAIL",
		"RST_FW_ASSERT_DONE",
		"RST_FW_ASSERT_TIMEOUT",
		"RST_BT_TRIGGER",
		"RST_OID_TIMEOUT",
		"RST_CMD_TRIGGER",
		"RST_CR_ACCESS_FAIL",
		"RST_CMD_EVT_FAIL",
		"RST_GROUP4_NULL",
		"RST_TX_ERROR",
		"RST_RX_ERROR",
		"RST_WDT",
		"RST_SER_L1_FAIL",
		"RST_SER_L0P5_FAIL",
	};
	static const char *const apcAction[] = {
		"RST_FLAG_DO_CORE_DUMP",
		"RST_FLAG_PREVENT_POWER_OFF",
		"RST_FLAG_DO_WHOLE_RESET",
		"RST_FLAG_DO_L0P5_RESET",
		"RST_FLAG_DO_L1_RESET",
	};
	uint32_t i;
	u_int8_t fgDrvOwn;

	if (!prAdapter) {
		if ((uint32_t)eResetReason < RST_REASON_MAX)
			DBGLOG(INIT, ERROR,
			       "Trigger reset in %s line %u reason %s, but prAdapter is null\n",
			       pucFile, u4Line,
			       apcReason[(uint32_t)eResetReason]);
		else
			DBGLOG(INIT, ERROR,
			       "Trigger reset in %s line %u unsupported reason %d and prAdapter is null\n",
			       pucFile, u4Line, (uint32_t)eResetReason);
		return;
	}
	prChipDbg = prAdapter->chip_info->prDebugOps;
	u2FwOwnVersion = prAdapter->rVerInfo.u2FwOwnVersion;
	u2FwPeerVersion = prAdapter->rVerInfo.u2FwPeerVersion;
	fgDrvOwn = TRUE;

	if ((uint32_t)eResetReason < RST_REASON_MAX)
		DBGLOG(INIT, ERROR,
		       "Trigger reset in %s line %u reason %s\n",
		       pucFile, u4Line, apcReason[(uint32_t)eResetReason]);
	else
		DBGLOG(INIT, ERROR,
		       "Trigger reset in %s line %u but unsupported reason %d\n",
		       pucFile, u4Line, eResetReason);
	for (i = 0; i < sizeof(apcAction) / sizeof(char *); i++)
		if (u4RstFlag & BIT(i))
			DBGLOG(INIT, ERROR, "action %s\n", apcAction[i]);
	if (!u4RstFlag)
		DBGLOG(INIT, ERROR, "no action\n");

	DBGLOG(INIT, ERROR,
	       "Chip[%04X E%u] FW Ver DEC[%u.%u] HEX[%x.%x]\n",
	       prAdapter->chip_info->chip_id,
	       wlanGetEcoVersion(prAdapter),
	       (uint16_t)(u2FwOwnVersion >> 8),
	       (uint16_t)(u2FwOwnVersion & BITS(0, 7)),
	       (uint16_t)(u2FwOwnVersion >> 8),
	       (uint16_t)(u2FwOwnVersion & BITS(0, 7)));
	DBGLOG(INIT, ERROR,
	       "Driver Ver[%u.%u]\n",
		(uint16_t)(u2FwPeerVersion >> 8),
		(uint16_t)(u2FwPeerVersion & BITS(0, 7)));

	prAdapter->u4HifDbgFlag |= DEG_HIF_DEFAULT_DUMP;
	halPrintHifDbgInfo(prAdapter);

	if ((u4RstFlag & RST_FLAG_DO_CORE_DUMP)
		&& (prChipDbg->show_mcu_debug_info != NULL)) {
#ifdef CFG_CHIP_RESET_KO_SUPPORT
		if (rstNotifyWholeChipRstStatus(RST_MODULE_WIFI,
						RST_MODULE_STATE_DUMP_START,
						NULL) == RST_MODULE_RET_FAIL)
			return;
#endif
		prChipDbg->show_mcu_debug_info(prAdapter, NULL, 0,
						       DBG_MCU_DBG_ALL, NULL);
#ifdef CFG_CHIP_RESET_KO_SUPPORT
		rstNotifyWholeChipRstStatus(RST_MODULE_WIFI,
					RST_MODULE_STATE_DUMP_END, NULL);
#endif
	}
}

/*----------------------------------------------------------------------------*/
/*!
 * @brief Reset trigger entry point.
 *
 * @param   prAddapter
 *          u4RstFlag  specify reset option
 *          pucFile  reset is triggered at which file
 *          u4Line  reset is triggered at which line
 *
 * @retval  none
 */
/*----------------------------------------------------------------------------*/
void glResetTrigger(struct ADAPTER *prAdapter, uint32_t u4RstFlag,
		    const uint8_t *pucFile, uint32_t u4Line)
{
	dump_stack();

	if (kalIsResetting())
		return;

	if (prAdapter == NULL) {
		if (wifi_rst.prGlueInfo != NULL &&
			wifi_rst.prGlueInfo->prAdapter != NULL)
			prAdapter = wifi_rst.prGlueInfo->prAdapter;
		else {
			DBGLOG(INIT, ERROR, "prAdapter is null, skip reset!\n");
			return;
		}
	}

	if (atomic_read(&g_wlanRemoving)) {
		DBGLOG(INIT, ERROR, "wlanRemove in process, skip reset!\n");
		return;
	}

	glResetTriggerCommon(prAdapter, u4RstFlag, pucFile, u4Line);

#if (CFG_WMT_RESET_API_SUPPORT == 1) || (CFG_SUPPORT_CONNINFRA == 1)
	glResetTriggerMobile(prAdapter, u4RstFlag);
#else
	glResetTriggerCe(prAdapter, u4RstFlag);
#endif
}

/*----------------------------------------------------------------------------*/
/*!
 * @brief Set L0.5 reset state
 *
 * @param   state
 *
 * @retval  none
 */
/*----------------------------------------------------------------------------*/
void glSetWfsysResetState(struct ADAPTER *prAdapter,
			  enum ENUM_WFSYS_RESET_STATE_TYPE_T state)
{
	if (state >= WFSYS_RESET_STATE_MAX) {
		DBGLOG(INIT, WARN,
			"[SER][L0.5] unsupported wfsys reset state\n");
	} else {
		spin_lock_bh(&prAdapter->rWfsysResetLock);

		prAdapter->eWfsysResetState = state;

		spin_unlock_bh(&prAdapter->rWfsysResetLock);
	}
}

/*----------------------------------------------------------------------------*/
/*!
 * @brief Re-schedule L0.5 reset if it's in WFSYS_POSTPONE state.
 *
 * @param   prAdapter
 *
 * @retval  TRUE   if re-schedule L0.5 reset is done
 *          FALSE   otherwise
 */
/*----------------------------------------------------------------------------*/
u_int8_t glReSchWfsysReset(struct ADAPTER *prAdapter)
{
	u_int8_t fgReSch = FALSE;

	spin_lock_bh(&prAdapter->rWfsysResetLock);

	if (prAdapter->eWfsysResetState == WFSYS_RESET_STATE_POSTPONE) {
		fgReSch = TRUE;
		schedule_work(&prAdapter->prGlueInfo->rWfsysResetWork);
	}

	spin_unlock_bh(&prAdapter->rWfsysResetLock);

	return fgReSch;
}

/*----------------------------------------------------------------------------*/
/*!
 * @brief L0.5 reset workhorse.
 *
 * @param   work
 *
 * @retval  none
 */
/*----------------------------------------------------------------------------*/
void WfsysResetHdlr(struct work_struct *work)
{
	struct GLUE_INFO *prGlueInfo;
	struct ADAPTER *prAdapter;
	struct mt66xx_hif_driver_data *prHifDrvData;

	prGlueInfo = container_of(work, struct GLUE_INFO, rWfsysResetWork);
	prAdapter = prGlueInfo->prAdapter;
	prHifDrvData = container_of(&prAdapter->chip_info,
				    struct mt66xx_hif_driver_data, chip_info);

	DBGLOG(INIT, STATE, "[SER][L0.5] Reset triggered eWfsysResetState=%d\n",
						prAdapter->eWfsysResetState);
	spin_lock_bh(&prAdapter->rWfsysResetLock);

	if (prAdapter->eWfsysResetState != WFSYS_RESET_STATE_POSTPONE) {
		spin_unlock_bh(&prAdapter->rWfsysResetLock);

		if (prAdapter->u2WfsysResetCnt < 0xFFFF)
			prAdapter->u2WfsysResetCnt++;

		glSetWfsysResetState(prAdapter, WFSYS_RESET_STATE_RESET);

		HAL_CANCEL_TX_RX(prAdapter);

#if defined(_HIF_SDIO)
		/* please refer to 7961 sdio ser L0.5 program guide */
		if (prAdapter->chip_info->asicSetNoBTFwOwnEn != NULL)
			if (prAdapter->chip_info->asicSetNoBTFwOwnEn(TRUE)
								== FALSE)
				goto FAIL;
#endif

		if (wlanOffAtReset() != WLAN_STATUS_SUCCESS)
			goto FAIL;

		fgIsResetting = FALSE;
		prAdapter->fgIsFwDownloaded = FALSE;

		if (HAL_TOGGLE_WFSYS_RST(prAdapter) != WLAN_STATUS_SUCCESS)
			goto FAIL;

	} else
		spin_unlock_bh(&prAdapter->rWfsysResetLock);

	if (GL_GET_WFSYS_RESET_POSTPONE(prAdapter))
		goto POSTPONE;

	glSetWfsysResetState(prAdapter, WFSYS_RESET_STATE_REINIT);

	HAL_RESUME_TX_RX(prAdapter);

	if (wlanOnAtReset() != WLAN_STATUS_SUCCESS)
		goto FAIL;

#if defined(_HIF_SDIO)
	/* please refer to 7961 sdio ser L0.5 program guide */
	if (prAdapter->chip_info->asicSetNoBTFwOwnEn != NULL)
		if (prAdapter->chip_info->asicSetNoBTFwOwnEn(FALSE) == FALSE)
			goto FAIL;
#endif

	glSetWfsysResetState(prAdapter, WFSYS_RESET_STATE_IDLE);

	DBGLOG(INIT, INFO, "[SER][L0.5] Reset done\n");

	DBGLOG(INIT, INFO, "[SER][L0.5] eWfsysResetState = %d\n",
					prAdapter->eWfsysResetState);

	DBGLOG(INIT, INFO, "[SER][L0.5] ucSerState = %d\n",
					prAdapter->ucSerState);

	DBGLOG(INIT, INFO, "[SER][L0.5] i4PendingFwdFrameCount=%d\n",
		prAdapter->rTxCtrl.i4PendingFwdFrameCount);

	DBGLOG(INIT, INFO, "[SER][L0.5] i4TxPendingFrameNum=%d\n",
		prAdapter->prGlueInfo->i4TxPendingFrameNum);

	DBGLOG(INIT, INFO, "[SER][L0.5] au4PseCtrlEnMap=%d\n",
		prAdapter->rTxCtrl.rTc.au4PseCtrlEnMap);

	return;

POSTPONE:
	DBGLOG(INIT, WARN, "[SER][L0.5] reset postpone\n");

	glSetWfsysResetState(prAdapter, WFSYS_RESET_STATE_POSTPONE);

	return;

FAIL:
	DBGLOG(INIT, ERROR, "\n[SER][L0.5] Reset fail !!!!\n\n");

	fgIsResetting = FALSE;

	GL_DEFAULT_RESET_TRIGGER(prAdapter, RST_SER_L0P5_FAIL);

	return;
}

/*----------------------------------------------------------------------------*/
/*!
 * @brief This routine is called for wifi reset
 *
 * @param   skb
 *          info
 *
 * @retval  0
 *          nonzero
 */
/*----------------------------------------------------------------------------*/
static void mtk_wifi_reset_main(struct RESET_STRUCT *rst)
{
	u_int8_t fgResult = FALSE;
#if CFG_WMT_RESET_API_SUPPORT
	int32_t ret;
#endif

	if (rst == NULL) {
		DBGLOG(INIT, ERROR, "input value rst is NULL.\n");
		return;
	}

#if CFG_WMT_RESET_API_SUPPORT
	/* wlanOnAtReset(); */
	ret = wifi_reset_end(rst->rst_data);
#if (CFG_SUPPORT_CONNINFRA == 1)
	update_driver_reset_status(fgIsResetting);
	if (g_IsWholeChipRst == TRUE) {
		g_IsWholeChipRst = FALSE;
		g_IsWfsysBusHang = FALSE;
		complete(&g_RstOnComp);
	}
#endif
#else
#ifdef CFG_CHIP_RESET_KO_SUPPORT
#if defined(_HIF_USB)
	rstNotifyWholeChipRstStatus(RST_MODULE_WIFI,
				RST_MODULE_STATE_PRERESET, NULL);
#endif
#if defined(_HIF_SDIO)
	rstNotifyWholeChipRstStatus(RST_MODULE_WIFI,
				RST_MODULE_STATE_PRERESET,
				rst->prGlueInfo->rHifInfo.func);
#endif
#else /* CFG_CHIP_RESET_KO_SUPPORT */
	fgResult = rst_L0_notify_step1();

	if (fgResult == BT_RESET_NOT_OK) {
		DBGLOG(INIT, ERROR, "BT not ready to reset, skip reset.\n");
		fgIsResetting = FALSE;
		return;
	}
	wait_core_dump_end();

	if (rst->prGlueInfo != NULL &&
		rst->prGlueInfo->prAdapter != NULL && 
		rst->prGlueInfo->prAdapter->chip_info != NULL &&
		rst->prGlueInfo->prAdapter->chip_info
			->rst_L0_notify_step2 != NULL)
		fgResult = rst->prGlueInfo->prAdapter
			->chip_info->rst_L0_notify_step2();

#if CFG_CHIP_RESET_HANG
	if (fgIsResetHangState == SER_L0_HANG_RST_NONE)
		fgIsResetHangState = SER_L0_HANG_RST_TRGING;
#endif

	if (is_bt_exist() == FALSE)
		kalRemoveProbe(rst->prGlueInfo);

#endif

#endif  /* CFG_CHIP_RESET_KO_SUPPORT */
	if (fgSimplifyResetFlow) {
		DBGLOG(INIT, INFO, "Force down the reset flag.\n");
		fgSimplifyResetFlow = FALSE;
	}
#if (CFG_SUPPORT_CONNINFRA == 1)
	if (ret != 0) {
		g_IsWfsysResetOnFail = TRUE;
		fgSimplifyResetFlow = TRUE;
		DBGLOG(INIT, STATE,
			"Wi-Fi reset on fail, set flag(%d).\n",
			g_IsWfsysResetOnFail);
	} else {
		g_IsWfsysResetOnFail = FALSE;
		DBGLOG(INIT, STATE,
			"Wi-Fi reset on success, set flag(%d).\n",
			g_IsWfsysResetOnFail);
	}
#endif
	DBGLOG(INIT, STATE, "[SER][L0] flow end, fgResult=%d\n", fgResult);

}
#if (CFG_SUPPORT_CONNINFRA == 0)
/*----------------------------------------------------------------------------*/
/*!
 * @brief This routine is called for wifi reset
 *
 * @param   skb
 *          info
 *
 * @retval  0
 *          nonzero
 */
/*----------------------------------------------------------------------------*/
static void mtk_wifi_reset(struct work_struct *work)
{
	struct RESET_STRUCT *rst = container_of(work,
						struct RESET_STRUCT, rst_work);
	struct ADAPTER *prAdapter;
	struct CHIP_DBG_OPS *prChipDbg;
	u_int8_t fgDrvOwn;

	prAdapter = rst->prGlueInfo->prAdapter;
	prChipDbg = prAdapter->chip_info->prDebugOps;
	fgDrvOwn = TRUE;

	/* Although we've already executed show_mcu_debug_info() in
	 * glResetTriggerCommon(), it may not be a successful operation at that
	 * time in USB mode due to the context is soft_irq which forbids the
	 * execution of usb_control_msg(). Since we don't prefer to refactor it
	 * at this stage, this is a workaround by performing it again here
	 * where the context is kernel thread from workqueue.
	 */
	if (prChipDbg->show_mcu_debug_info) {
#ifdef CFG_CHIP_RESET_KO_SUPPORT
		if (rstNotifyWholeChipRstStatus(RST_MODULE_WIFI,
						RST_MODULE_STATE_DUMP_START,
						NULL) == RST_MODULE_RET_FAIL)
			return;
#endif
		prChipDbg->show_mcu_debug_info(prAdapter, NULL, 0,
						       DBG_MCU_DBG_ALL, NULL);
#ifdef CFG_CHIP_RESET_KO_SUPPORT
		rstNotifyWholeChipRstStatus(RST_MODULE_WIFI,
					RST_MODULE_STATE_DUMP_END, NULL);
#endif
	}

	mtk_wifi_reset_main(rst);
}
#endif

#if CFG_WMT_RESET_API_SUPPORT
#if (CFG_SUPPORT_CONNINFRA == 0)
static void mtk_wifi_trigger_reset(struct work_struct *work)
{
	u_int8_t fgResult = FALSE;
	struct RESET_STRUCT *rst = container_of(work,
					struct RESET_STRUCT, rst_trigger_work);

	fgIsResetting = TRUE;
	/* Set the power off flag to FALSE in WMT to prevent chip power off
	 * after wlanProbe return failure, because we need to do core dump
	 * afterward.
	 */
	if (rst->rst_trigger_flag & RST_FLAG_PREVENT_POWER_OFF)
		mtk_wcn_set_connsys_power_off_flag(FALSE);

	fgResult = mtk_wcn_wmt_assert_timeout(WMTDRV_TYPE_WIFI, 0x40, 0);
	DBGLOG(INIT, INFO, "reset result %d, trigger flag 0x%x\n",
				fgResult, rst->rst_trigger_flag);
}
#endif
/* Weak reference for those platform doesn't support wmt functions */
u_int8_t __weak mtk_wcn_stp_coredump_start_get(void)
{
	return FALSE;
}


/*0= f/w assert flag is not set, others=f/w assert flag is set */
u_int8_t glIsWmtCodeDump(void)
{
	return mtk_wcn_stp_coredump_start_get();
}

/*----------------------------------------------------------------------------*/
/*!
 * @brief This routine is invoked when there is reset messages indicated
 *
 * @param   eSrcType
 *          eDstType
 *          eMsgType
 *          prMsgBody
 *          u4MsgLength
 *
 * @retval
 */
/*----------------------------------------------------------------------------*/
#if (CFG_SUPPORT_CONNINFRA == 0)
static void *glResetCallback(enum ENUM_WMTDRV_TYPE eSrcType,
			     enum ENUM_WMTDRV_TYPE eDstType,
			     enum ENUM_WMTMSG_TYPE eMsgType, void *prMsgBody,
			     unsigned int u4MsgLength)
{
	switch (eMsgType) {
	case WMTMSG_TYPE_RESET:
		if (u4MsgLength == sizeof(enum ENUM_WMTRSTMSG_TYPE)) {
			enum ENUM_WMTRSTMSG_TYPE *prRstMsg =
					(enum ENUM_WMTRSTMSG_TYPE *) prMsgBody;

			switch (*prRstMsg) {
			case WMTRSTMSG_RESET_START:
				DBGLOG(INIT, WARN, "Whole chip reset start!\n");
				fgIsResetting = TRUE;
				fgSimplifyResetFlow = TRUE;
				wifi_reset_start();
				break;

			case WMTRSTMSG_RESET_END:
				DBGLOG(INIT, WARN, "Whole chip reset end!\n");
				wifi_rst.rst_data = RESET_SUCCESS;
				fgIsResetting = FALSE;
				schedule_work(&(wifi_rst.rst_work));
				break;

			case WMTRSTMSG_RESET_END_FAIL:
				DBGLOG(INIT, WARN, "Whole chip reset fail!\n");
				fgIsResetting = FALSE;
				wifi_rst.rst_data = RESET_FAIL;
				schedule_work(&(wifi_rst.rst_work));
				break;

			default:
				break;
			}
		}
		break;

	default:
		break;
	}

	return NULL;
}
#else

void glSetRstReasonString(char *reason)
{
	g_reason = reason;
}


static u_int8_t glResetMsgHandler(enum ENUM_WMTMSG_TYPE eMsgType,
				  enum ENUM_WMTRSTMSG_TYPE MsgBody)
{
	switch (eMsgType) {
	case WMTMSG_TYPE_RESET:

			switch (MsgBody) {
			case WMTRSTMSG_RESET_START:
				DBGLOG(INIT, WARN, "Whole chip reset start!\n");
				fgIsResetting = TRUE;
				fgSimplifyResetFlow = TRUE;
				wifi_reset_start();
				hifAxiRemove();
				complete(&g_RstOffComp);
				break;

			case WMTRSTMSG_RESET_END:
				DBGLOG(INIT, WARN, "WF reset end!\n");
				fgIsResetting = FALSE;
				wifi_rst.rst_data = RESET_SUCCESS;
				mtk_wifi_reset_main(&wifi_rst);
				break;

			case WMTRSTMSG_RESET_END_FAIL:
				DBGLOG(INIT, WARN, "Whole chip reset fail!\n");
				fgIsResetting = FALSE;
				wifi_rst.rst_data = RESET_FAIL;
				schedule_work(&(wifi_rst.rst_work));
				break;
			case WMTRSTMSG_0P5RESET_START:
				DBGLOG(INIT, WARN, "WF chip reset start!\n");
				fgIsResetting = TRUE;
				fgSimplifyResetFlow = TRUE;
				wifi_reset_start();
				hifAxiRemove();
				break;
			default:
				break;
			}
		break;

	default:
		break;
	}

	return TRUE;
}
bool glRstCheckRstCriteria(void)
{
	/*
	 * for those cases which need to trigger whole chip reset
	 * when fgIsResetting = TRUE
	 */
	if (g_IsSubsysRstOverThreshold || g_IsWfsysBusHang)
		return FALSE;
	else
		return TRUE;
}
void glRstWholeChipRstParamInit(void)
{
	g_IsSubsysRstOverThreshold = FALSE;
	g_SubsysRstCnt = 0;
	g_IsTriggerTimeout = FALSE;
	g_WholeChipRstTotalCnt++;
}
void glRstSetRstEndEvent(void)
{
	KAL_WAKE_LOCK(NULL, g_IntrWakeLock);

	set_bit(GLUE_FLAG_RST_END_BIT, &g_ulFlag);

	/* when we got interrupt, we wake up servie thread */
	wake_up_interruptible(&g_waitq_rst);

}

int glRstwlanPreWholeChipReset(enum consys_drv_type type, char *reason)
{
	bool bRet = TRUE;
	struct GLUE_INFO *prGlueInfo;

	prGlueInfo = (struct GLUE_INFO *) wiphy_priv(wlanGetWiphy());
	DBGLOG(INIT, INFO,
			"Enter glRstwlanPreWholeChipReset.\n");
	while (get_wifi_process_status() == 1) {
		DBGLOG(REQ, WARN,
			"Wi-Fi on/off process is ongoing, wait here.\n");
		msleep(100);
	}
	if (!get_wifi_powered_status()) {
		DBGLOG(REQ, WARN, "wifi driver is off now\n");
		return bRet;
	}
	g_WholeChipRstType = type;
	g_WholeChipRstReason = reason;

	if (glRstCheckRstCriteria()) {
		while (kalIsResetting()) {
			DBGLOG(REQ, WARN, "wifi driver is resetting\n");
			msleep(100);
		}
		while ((!prGlueInfo) ||
			(prGlueInfo->u4ReadyFlag == 0) ||
			(g_IsWfsysRstDone == FALSE)) {
			prGlueInfo =
				(struct GLUE_INFO *) wiphy_priv(wlanGetWiphy());
			DBGLOG(REQ, WARN, "wifi driver is not ready\n");
			if (g_IsWfsysResetOnFail == TRUE) {
				DBGLOG(REQ, WARN,
					"wifi driver reset fail, need whole chip reset.\n");
				g_IsWholeChipRst = TRUE;
				return bRet;
			}
			msleep(100);
		}
		g_IsWholeChipRst = TRUE;
		DBGLOG(INIT, INFO,
				"Wi-Fi Driver processes whole chip reset start.\n");
		GL_USER_DEFINE_RESET_TRIGGER(prGlueInfo->prAdapter,
					     RST_UNKNOWN,
					     RST_FLAG_WF_RESET);
	} else {
		if (g_IsSubsysRstOverThreshold)
			DBGLOG(INIT, INFO, "Reach subsys reset threshold!!!\n");
		else if (g_IsWfsysBusHang)
			DBGLOG(INIT, INFO, "WFSYS bus hang!!!\n");
	g_IsWholeChipRst = TRUE;
		kalSetRstEvent();
	}
	wait_for_completion(&g_RstOffComp);
		DBGLOG(INIT, INFO, "Wi-Fi is off successfully.\n");

	return bRet;
}

int glRstwlanPostWholeChipReset(void)
{
	while (get_wifi_process_status() == 1) {
		DBGLOG(REQ, WARN,
			"Wi-Fi on/off process is ongoing, wait here.\n");
		msleep(100);
	}
	if (!get_wifi_powered_status()) {
		DBGLOG(REQ, WARN, "wifi driver is off now\n");
		return 0;
	}
	glRstSetRstEndEvent();
	DBGLOG(INIT, INFO, "Wait Wi-Fi state recover.\n");
	wait_for_completion(&g_RstOnComp);

	DBGLOG(INIT, INFO,
		"Leave glRstwlanPostWholeChipReset (%d).\n",
		g_IsWholeChipRst);
	return 0;
}
u_int8_t kalIsWholeChipResetting(void)
{
#if CFG_CHIP_RESET_SUPPORT
	return g_IsWholeChipRst;
#else
	return FALSE;
#endif
}
void glReset_timeinit(struct timeval *rNowTs, struct timeval *rLastTs)
{
	rNowTs->tv_sec = 0;
	rNowTs->tv_usec = 0;
	rLastTs->tv_sec = 0;
	rLastTs->tv_usec = 0;
}

bool IsOverRstTimeThreshold(struct timeval *rNowTs, struct timeval *rLastTs)
{
	struct timeval rTimeout, rTime;
	bool fgIsTimeout = FALSE;

	rTimeout.tv_sec = 30;
	rTimeout.tv_usec = 0;
	do_gettimeofday(rNowTs);
	DBGLOG(INIT, INFO,
		"Reset happen time :%d.%d, last happen time :%d.%d\n",
		rNowTs->tv_sec,
		rNowTs->tv_usec,
		rLastTs->tv_sec,
		rLastTs->tv_usec);
	if (rLastTs->tv_sec != 0) {
		/* Ignore now time < token time */
		if (halTimeCompare(rNowTs, rLastTs) > 0) {
			rTime.tv_sec = rNowTs->tv_sec - rLastTs->tv_sec;
			rTime.tv_usec = rNowTs->tv_usec;
			if (rLastTs->tv_usec > rNowTs->tv_usec) {
				rTime.tv_sec -= 1;
				rTime.tv_usec += SEC_TO_USEC(1);
			}
			rTime.tv_usec -= rLastTs->tv_usec;
			if (halTimeCompare(&rTime, &rTimeout) >= 0)
				fgIsTimeout = TRUE;
			else
				fgIsTimeout = FALSE;
		}
		DBGLOG(INIT, INFO,
			"Reset rTimeout :%d.%d, calculate time :%d.%d\n",
			rTimeout.tv_sec,
			rTimeout.tv_usec,
			rTime.tv_sec,
			rTime.tv_usec);
	}
	return fgIsTimeout;
}
void glResetSubsysRstProcedure(
	struct ADAPTER *prAdapter,
	struct timeval *rNowTs,
	struct timeval *rLastTs)
{
	bool fgIsTimeout;
	struct mt66xx_chip_info *prChipInfo;
	struct WIFI_VAR *prWifiVar = &prAdapter->rWifiVar;

	if (prWifiVar->fgRstRecover == 1)
		g_fgRstRecover = TRUE;
#if 0
	if (prAdapter->chip_info->checkbushang)
		prAdapter->chip_info->checkbushang(FALSE);
#endif
	fgIsTimeout = IsOverRstTimeThreshold(rNowTs, rLastTs);
	if (g_IsWfsysBusHang == TRUE) {
		/* dump host cr */
		if (prAdapter->chip_info->dumpBusHangCr)
			prAdapter->chip_info->dumpBusHangCr(prAdapter);
		glSetRstReasonString(
			"fw detect bus hang");
		prChipInfo = prAdapter->chip_info;
		if (prChipInfo->trigger_wholechiprst)
			prChipInfo->trigger_wholechiprst(g_reason);
		g_IsTriggerTimeout = FALSE;
		return;
	}
	if (g_SubsysRstCnt > 3) {
		if (fgIsTimeout == TRUE) {
		/*
		 * g_SubsysRstCnt > 3, > 30 sec,
		 * need to update rLastTs, still do wfsys reset
		 */

			glResetMsgHandler(WMTMSG_TYPE_RESET,
					  WMTRSTMSG_0P5RESET_START);
			glResetMsgHandler(WMTMSG_TYPE_RESET,
					  WMTRSTMSG_RESET_END);
			g_SubsysRstTotalCnt++;
			g_SubsysRstCnt = 1;
		} else {
			/*g_SubsysRstCnt > 3, < 30 sec, do whole chip reset */
			g_IsSubsysRstOverThreshold = TRUE;
			/*coredump is done, no need do again*/
			g_IsTriggerTimeout = TRUE;
			glSetRstReasonString(
				"subsys reset more than 3 times");
			prChipInfo = prAdapter->chip_info;
			if (prChipInfo->trigger_wholechiprst)
				prChipInfo->trigger_wholechiprst(g_reason);
		}
	} else {

		glResetMsgHandler(WMTMSG_TYPE_RESET,
				  WMTRSTMSG_0P5RESET_START);
		glResetMsgHandler(WMTMSG_TYPE_RESET,
				  WMTRSTMSG_RESET_END);
		g_SubsysRstTotalCnt++;
		/*g_SubsysRstCnt < 3, but >30 sec,need to update rLastTs*/
		if (fgIsTimeout == TRUE)
			g_SubsysRstCnt = 1;
	}
	if (g_SubsysRstCnt == 1) {
		rLastTs->tv_sec = rNowTs->tv_sec;
		rLastTs->tv_usec = rNowTs->tv_usec;
	}
	g_IsTriggerTimeout = FALSE;
#if (CFG_ANDORID_CONNINFRA_COREDUMP_SUPPORT == 1)
	g_eWfRstSource = WF_RST_SOURCE_NONE;
#endif
}
int wlan_reset_thread_main(void *data)
{
	int ret = 0;
	struct GLUE_INFO *prGlueInfo = NULL;
	struct timeval rNowTs, rLastTs;

#if defined(CONFIG_ANDROID) && (CFG_ENABLE_WAKE_LOCK)
	KAL_WAKE_LOCK_T *prWlanRstThreadWakeLock;

	KAL_WAKE_LOCK_INIT(NULL,
			   prWlanRstThreadWakeLock, "WLAN rst_thread");
	KAL_WAKE_LOCK(NULL, prWlanRstThreadWakeLock);
#endif

	glReset_timeinit(&rNowTs, &rLastTs);

	DBGLOG(INIT, INFO, "%s:%u starts running...\n",
	       KAL_GET_CURRENT_THREAD_NAME(), KAL_GET_CURRENT_THREAD_ID());

	g_u4WlanRstThreadPid = KAL_GET_CURRENT_THREAD_ID();

	while (TRUE) {
		/* Unlock wakelock if hif_thread going to idle */
		KAL_WAKE_UNLOCK(NULL, prWlanRstThreadWakeLock);
		/*
		 * sleep on waitqueue if no events occurred. Event contain
		 * (1) GLUE_FLAG_HALT (2) GLUE_FLAG_RST
		 *
		 */
		do {
			ret = wait_event_interruptible(g_waitq_rst,
				((g_ulFlag & GLUE_FLAG_RST_PROCESS)
				!= 0));
		} while (ret != 0);
#if defined(CONFIG_ANDROID) && (CFG_ENABLE_WAKE_LOCK)
		if (!KAL_WAKE_LOCK_ACTIVE(NULL,
					  prWlanRstThreadWakeLock))
			KAL_WAKE_LOCK(NULL,
				      prWlanRstThreadWakeLock);
#endif
		prGlueInfo = (struct GLUE_INFO *) wiphy_priv(wlanGetWiphy());
		if (test_and_clear_bit(GLUE_FLAG_RST_START_BIT, &g_ulFlag) &&
			 ((prGlueInfo) && (prGlueInfo->u4ReadyFlag))) {
			if (KAL_WAKE_LOCK_ACTIVE(NULL, g_IntrWakeLock))
				KAL_WAKE_UNLOCK(NULL, g_IntrWakeLock);

			if (g_IsWholeChipRst) {
#if (CFG_ANDORID_CONNINFRA_COREDUMP_SUPPORT == 1)
				if (eResetReason >= RST_REASON_MAX)
					eResetReason = 0;
				fw_log_connsys_coredump_start(
					g_WholeChipRstType,
					g_WholeChipRstReason);
#endif
				glResetMsgHandler(WMTMSG_TYPE_RESET,
							WMTRSTMSG_RESET_START);
				glRstWholeChipRstParamInit();
				glReset_timeinit(&rNowTs, &rLastTs);
			} else {
				/*wfsys reset start*/
				g_IsWfsysRstDone = FALSE;
					g_SubsysRstCnt++;
					DBGLOG(INIT, INFO,
					"WF reset count = %d.\n",
						g_SubsysRstCnt);
				glResetSubsysRstProcedure(prGlueInfo->prAdapter,
							 &rNowTs,
							 &rLastTs);
				/*wfsys reset done*/
				g_IsWfsysRstDone = TRUE;
			}
			DBGLOG(INIT, INFO,
			"Whole Chip rst count /WF reset total count = (%d)/(%d).\n",
				g_WholeChipRstTotalCnt,
				g_SubsysRstTotalCnt);
		}
		if (test_and_clear_bit(GLUE_FLAG_RST_END_BIT, &g_ulFlag)) {
			if (KAL_WAKE_LOCK_ACTIVE(NULL, g_IntrWakeLock))
				KAL_WAKE_UNLOCK(NULL, g_IntrWakeLock);
			DBGLOG(INIT, INFO, "Whole chip reset end start\n");
					glResetMsgHandler(WMTMSG_TYPE_RESET,
							WMTRSTMSG_RESET_END);
			}
		if (test_and_clear_bit(GLUE_FLAG_HALT_BIT, &g_ulFlag)) {
			DBGLOG(INIT, INFO, "rst_thread should stop now...\n");
			break;
		}
	}

#if defined(CONFIG_ANDROID) && (CFG_ENABLE_WAKE_LOCK)
	if (KAL_WAKE_LOCK_ACTIVE(NULL,
				 prWlanRstThreadWakeLock))
		KAL_WAKE_UNLOCK(NULL, prWlanRstThreadWakeLock);
	KAL_WAKE_LOCK_DESTROY(NULL,
			      prWlanRstThreadWakeLock);
#endif

	DBGLOG(INIT, TRACE, "%s:%u stopped!\n",
	       KAL_GET_CURRENT_THREAD_NAME(), KAL_GET_CURRENT_THREAD_ID());

	return 0;
}
#endif
#else
#ifndef CFG_CHIP_RESET_KO_SUPPORT
static u_int8_t is_bt_exist(void)
{
	char *bt_func_name = "btmtk_sdio_whole_reset";
	void *pvAddr = NULL;

#if	(CFG_ENABLE_GKI_SUPPORT != 1)
	pvAddr = GLUE_SYMBOL_GET(bt_func_name);
#endif
	if (!pvAddr)
		DBGLOG(INIT, ERROR, "[SER][L0] %s does not exist\n",
			bt_func_name);
	else {
#if	(CFG_ENABLE_GKI_SUPPORT != 1)
		GLUE_SYMBOL_PUT(bt_func_name);
#endif
		return TRUE;
	}

	return FALSE;

}

static u_int8_t rst_L0_notify_step1(void)
{
	if (eResetReason != RST_BT_TRIGGER) {
		typedef int (*p_bt_fun_type) (void *);
		p_bt_fun_type bt_func;
		char *bt_func_name = "btmtk_sdio_whole_reset";
		void *pvAddr = NULL;
		int ret = -1;

		DBGLOG(INIT, STATE, "[SER][L0] %s\n", bt_func_name);
#if	(CFG_ENABLE_GKI_SUPPORT != 1)
		pvAddr = GLUE_SYMBOL_GET(bt_func_name);
#endif
		if (pvAddr) {
			bt_func = (p_bt_fun_type) pvAddr;
			ret = bt_func(NULL);
#if	(CFG_ENABLE_GKI_SUPPORT != 1)
			GLUE_SYMBOL_PUT(bt_func_name);
#endif
			if (ret == 0)
				return BT_RESET_OK;
			else
				return BT_RESET_NOT_OK;
		} else {
			DBGLOG(INIT, ERROR,
				"[SER][L0] %s does not exist\n", bt_func_name);
			return BT_RESET_NOT_FOUND;
		}
	}

	return BT_RESET_OK;
}

static void wait_core_dump_end(void)
{
#ifdef CFG_SUPPORT_CONNAC2X
	if (eResetReason == RST_OID_TIMEOUT)
		return;
	DBGLOG(INIT, WARN, "[SER][L0] not support..\n");
#endif
}

int32_t BT_rst_L0_notify_WF_step1(int32_t reserved)
{
	glSetRstReason(RST_BT_TRIGGER);
	GL_DEFAULT_RESET_TRIGGER(NULL, RST_BT_TRIGGER);

	return 0;
}
EXPORT_SYMBOL(BT_rst_L0_notify_WF_step1);

int32_t BT_rst_L0_notify_WF_2(int32_t reserved)
{
	DBGLOG(INIT, WARN, "[SER][L0] not support...\n");

	return 0;
}
EXPORT_SYMBOL(BT_rst_L0_notify_WF_2);

#endif
#endif

#endif
