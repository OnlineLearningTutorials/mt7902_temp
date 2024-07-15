/* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause */
/*
 * Copyright (c) 2017 MediaTek Inc.
 */
/*! \file   reset.c
*   \brief  reset module
*
*    This file contains all implementations of reset module
*/


/*******************************************************************************
*                         C O M P I L E R   F L A G S
********************************************************************************
*/

/*******************************************************************************
*                    E X T E R N A L   R E F E R E N C E S
********************************************************************************
*/
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/gpio.h>
#include <linux/kernel.h>
#include <linux/workqueue.h>
#include <linux/module.h>
#include <linux/mmc/card.h>
#include <linux/mmc/host.h>
#include <linux/mmc/sdio.h>
#include <linux/mmc/sdio_func.h>/* sdio_readl(), etc */
#include <linux/mmc/host.h>		/* mmc_add_host(), etc */
#include <linux/mmc/sdio_ids.h>
#include <linux/errno.h>
#include "precomp.h"

/*******************************************************************************
*                              C O N S T A N T S
********************************************************************************
*/
#define RST_STATUS_BT_PROBE_DONE	BIT(0)
#define RST_STATUS_WIFI_PROBE_DONE	BIT(1)
#define RST_STATUS_BT_DUMP		BIT(0)
#define RST_STATUS_WIFI_DUMP		BIT(1)

/**
* For chip reset pin set low time
*/
#define RESET_PIN_SET_LOW_TIME		500  /* in unit of ms */

/**
 * For chip reset pin number configureation
 */
#define WIFI_DONGLE_RESET_GPIO_PIN	220

/*******************************************************************************
*                             D A T A   T Y P E S
********************************************************************************
*/

enum ENUM_RST_STATE_TYPE_T {
	RST_STATE_UNKNOWN = 0,
	RST_STATE_IDLE,
	RST_STATE_START,
	RST_STATE_GOING,
	RST_STATE_DUMP,
	RST_STATE_MAX
};

struct WHOLE_CHIP_RESET_STRUCT {
	struct mmc_host *prHost;
	struct mutex rResetMutex;
	enum ENUM_RST_STATE_TYPE_T eResetState;
};

/*******************************************************************************
*                            P U B L I C   D A T A
********************************************************************************
*/

MODULE_LICENSE("Dual BSD/GPL");

/*******************************************************************************
*                           P R I V A T E   D A T A
********************************************************************************
*/
static int32_t g_resetFlag;
static int32_t g_dumpFlag;
u_int8_t fgBtProbed = FALSE;
u_int8_t fgWiFiProbed = FALSE;
u_int8_t fgWaitResetDone = FALSE;
static struct WHOLE_CHIP_RESET_STRUCT g_prResetInfo;
/*******************************************************************************
*                                 M A C R O S
********************************************************************************
*/

/*******************************************************************************
*                  F U N C T I O N   D E C L A R A T I O N S
********************************************************************************
*/

/*******************************************************************************
*                              F U N C T I O N S
********************************************************************************
*/

/*----------------------------------------------------------------------------*/
/*!
 * @brief Set L0 reset state
 *
 * @param   state
 *
 * @retval  none
 */
/*----------------------------------------------------------------------------*/
void resetSetState(enum ENUM_RST_STATE_TYPE_T eNextstate)
{
	static const char *const apcState[RST_STATE_MAX] = {
		"RST_STATE_UNKNOWN",
		"RST_STATE_IDLE",
		"RST_STATE_START",
		"RST_STATE_GOING",
		"RST_STATE_DUMP",
	};

	if ((uint32_t)eNextstate >= RST_STATE_MAX) {
		MR_Err("Next state is invalid!\n");
		return;
	}

	MR_Dbg("%s: current_state[%s], next_state[%s]\n", __func__,
		apcState[(uint32_t)g_prResetInfo.eResetState],
		apcState[(uint32_t)eNextstate]);

	mutex_lock(&g_prResetInfo.rResetMutex);
	if (g_prResetInfo.eResetState != eNextstate)
		g_prResetInfo.eResetState = eNextstate;
	mutex_unlock(&g_prResetInfo.rResetMutex);
}


int resetGetState(void)
{
	enum ENUM_RST_STATE_TYPE_T eCurrstate;

	mutex_lock(&g_prResetInfo.rResetMutex);
	eCurrstate = g_prResetInfo.eResetState;
	mutex_unlock(&g_prResetInfo.rResetMutex);

	return eCurrstate;
}


#if defined(_HIF_USB)
void resetUsbTogglePin(void)
{
	uint32_t gpio_num = WIFI_DONGLE_RESET_GPIO_PIN;

#if CFG_CHIP_RESET_USE_DTS_GPIO_NUM
	struct device_node *node;
	int32_t i4Status = 0;

	node = of_find_compatible_node(NULL, NULL, CHIP_RESET_DTS_NODE_NAME);
	if (node) {
		i4Status = of_get_named_gpio(node,
				CHIP_RESET_GPIO_PROPERTY_NAME, 0);
		if (i4Status >= 0) {
			gpio_num = i4Status;
			i4Status = 0;
		} else if (of_property_read_u32(node,
				CHIP_RESET_GPIO_PROPERTY_NAME,
				&gpio_num) == 0){
			i4Status = 0;
		} else {
			i4Status = -1;
			MR_Err("[SER][L0]: Failed to get GPIO num: %s\n",
				CHIP_RESET_GPIO_PROPERTY_NAME);
		}
	} else {
		i4Status = -1;
		MR_Info("[SER][L0]: Failed to find dts node: %s\n",
			CHIP_RESET_DTS_NODE_NAME);
	}
	if (i4Status != 0)
		return;
	i4Status = gpio_request(gpio_num, CHIP_RESET_GPIO_PROPERTY_NAME);
	if (i4Status < 0) {
		MR_Err("[SER][L0]: gpio_request(%d,%s) %d failed\n",
			gpio_num, CHIP_RESET_GPIO_PROPERTY_NAME, i4Status);
		return;
	}
#endif

	MR_Info("[SER][L0]: Use GPIO num: %d\n", gpio_num);

	i4Status = gpio_direction_output(gpio_num, 0);
	MR_Err("[SER][L0]%s: Invoke gpio_direction_output 0(%d,%d)\n", __func__,
			gpio_num, i4Status);
	mdelay(RESET_PIN_SET_LOW_TIME);
	i4Status = gpio_direction_output(gpio_num, 1);
	MR_Err("[SER][L0]%s: Invoke gpio_direction_output 1(%d,%d)\n", __func__,
			gpio_num, i4Status);
	gpio_free(gpio_num);
}
#endif

#if defined(_HIF_SDIO)
void resetSdioTogglePin(void)
{
	struct mmc_host *host;

	host = g_prResetInfo.prHost;
	host->rescan_entered = 0;

	MR_Err("[SER][L0] mmc_remove_host\n");
	mmc_remove_host(host);
	MR_Err("[SER][L0] mmc_add_host\n");
	mmc_add_host(host);
}
#endif

static void resetTogglePin(void)
{
	MR_Dbg("%s: toggle reset pin\n", __func__);

	resetSetState(RST_STATE_GOING);

#if defined(_HIF_USB)
	resetUsbTogglePin();
#elif defined(_HIF_SDIO)
	resetSdioTogglePin();
#endif
}

/*----------------------------------------------------------------------------*/
/*!
* \brief export function for set reset status for WiFi/BT
*
* \param[in] module connectivity module.
* \param[in] status reset status
* \param[in] SDIO host card information
*
* \retval void
*/
/*----------------------------------------------------------------------------*/
enum ENUM_RST_MODULE_RET_TYPE_T rstNotifyWholeChipRstStatus(
				enum ENUM_RST_MODULE_TYPE_T module,
				enum ENUM_RST_MODULE_STATE_TYPE_T status,
				void *data)
{
	enum ENUM_RST_MODULE_RET_TYPE_T ret = RST_MODULE_RET_SUCCESS;

	static const char *const apcModule[RST_MODULE_MAX] = {
		"BT",
		"WIFI",
	};
	static const char *const apcStatus[RST_MODULE_STATE_MAX] = {
		"RST_MODULE_STATE_PRERESET",
		"RST_MODULE_STATE_KO_INSMOD",
		"RST_MODULE_STATE_KO_RMMOD",
		"RST_MODULE_STATE_PROBE_START",
		"RST_MODULE_STATE_PROBE_DONE",
		"RST_MODULE_STATE_DUMP_START",
		"RST_MODULE_STATE_DUMP_END",
	};

	if ((uint32_t)module >= RST_MODULE_MAX ||
		(uint32_t)status >= RST_MODULE_STATE_MAX) {
		MR_Err("module or status is invalid!\n");
		return ret;
	}
	MR_Dbg("%s: module[%s], status[%s]\n", __func__,
		apcModule[(uint32_t)module], apcStatus[(uint32_t)status]);

#if defined(_HIF_SDIO)
	if (func)
		g_prResetInfo.prHost = func->card->host;
#endif

	switch (status) {
	case RST_MODULE_STATE_PRERESET:
		if (resetGetState() == RST_STATE_IDLE) {
			goto TOGGLE_PIN;
		} else if (fgBtProbed == FALSE
			|| fgWiFiProbed == FALSE) {
			MR_Dbg("WiFi or BT not probe start\n");
		}
		break;

	case RST_MODULE_STATE_KO_INSMOD:
		if (resetGetState() == RST_STATE_UNKNOWN)
			resetSetState(RST_STATE_IDLE);
		break;

	case RST_MODULE_STATE_KO_RMMOD:
		if (module == RST_MODULE_BT)
			fgBtProbed = FALSE;
		else if (module == RST_MODULE_WIFI)
			fgWiFiProbed = FALSE;

		if (fgBtProbed && fgWiFiProbed)
			resetSetState(RST_STATE_IDLE);

		break;

	case RST_MODULE_STATE_PROBE_START:
		if (module == RST_MODULE_BT)
			fgBtProbed = TRUE;
		else if (module == RST_MODULE_WIFI)
			fgWiFiProbed = TRUE;

		break;

	case RST_MODULE_STATE_PROBE_DONE:
		if (module == RST_MODULE_BT)
			g_resetFlag |= RST_STATUS_BT_PROBE_DONE;
		else if (module == RST_MODULE_WIFI)
			g_resetFlag |= RST_STATUS_WIFI_PROBE_DONE;

		if (fgBtProbed && fgWiFiProbed) {
			if (g_resetFlag & RST_STATUS_WIFI_PROBE_DONE
				&& g_resetFlag & RST_STATUS_BT_PROBE_DONE)
				resetSetState(RST_STATE_IDLE);
		} else {
			if (fgBtProbed) {
				if (g_resetFlag & RST_STATUS_BT_PROBE_DONE)
					resetSetState(RST_STATE_IDLE);
			}

			if (fgWiFiProbed) {
				if (g_resetFlag & RST_STATUS_WIFI_PROBE_DONE)
					resetSetState(RST_STATE_IDLE);
			}
		}

		break;

	case RST_MODULE_STATE_DUMP_START:
		if (resetGetState() == RST_STATE_START ||
			resetGetState() == RST_STATE_GOING) {
			ret = RST_MODULE_RET_FAIL;
			MR_Err("resting, dump is not allowed");
		} else {
			if (module == RST_MODULE_BT
				&& g_resetFlag & RST_STATUS_BT_PROBE_DONE
				&& !(g_dumpFlag & RST_STATUS_BT_DUMP)) {
				g_dumpFlag |= RST_STATUS_BT_DUMP;
				g_resetFlag |= RST_STATUS_WIFI_PROBE_DONE;
				resetSetState(RST_STATE_DUMP);
			} else if (module == RST_MODULE_WIFI
				&& g_resetFlag & RST_STATUS_WIFI_PROBE_DONE
				&& !(g_dumpFlag & RST_STATUS_WIFI_DUMP)) {
				g_dumpFlag |= RST_STATUS_WIFI_DUMP;
				resetSetState(RST_STATE_DUMP);
			}
		}
		break;

	case RST_MODULE_STATE_DUMP_END:
		if (module == RST_MODULE_BT)
			g_dumpFlag &= ~RST_STATUS_BT_DUMP;
		else if (module == RST_MODULE_WIFI)
			g_dumpFlag &= ~RST_STATUS_WIFI_DUMP;

		if (!g_dumpFlag && resetGetState() == RST_STATE_DUMP)
			resetSetState(RST_STATE_IDLE);
		break;

	default:
		/* Make sure we have handle all STATEs */
		MR_Err("Invalid State:%d\n", status);
		break;
	}

	return ret;

TOGGLE_PIN:
	g_resetFlag &= ~RST_STATUS_BT_PROBE_DONE;
	g_resetFlag &= ~RST_STATUS_WIFI_PROBE_DONE;
	g_dumpFlag &= ~RST_STATUS_BT_DUMP;
	g_dumpFlag &= ~RST_STATUS_WIFI_DUMP;

	resetSetState(RST_STATE_START);
	resetTogglePin();
	return ret;
}
EXPORT_SYMBOL(rstNotifyWholeChipRstStatus);

/* Wifi module complex the func, BT call */
struct WIFI_NOTIFY_DESC fg_wifi_notify_desc = {
	.BtNotifyWifiSubResetStep1 = NULL,
};

/* BT module complex the func, Wifi call */
struct BT_NOTIFY_DESC fg_bt_notify_desc = {
	.WifiNotifyBtSubResetStep1 = NULL,
	.WifiNotifyReadBtMcuPc = NULL,
	.WifiNotifyReadWifiMcuPc = NULL,
};

void register_bt_notify_callback(struct BT_NOTIFY_DESC *bt_notify_cb)
{
	fg_bt_notify_desc.WifiNotifyBtSubResetStep1 =
		bt_notify_cb->WifiNotifyBtSubResetStep1;
	fg_bt_notify_desc.WifiNotifyReadBtMcuPc =
		bt_notify_cb->WifiNotifyReadBtMcuPc;
	fg_bt_notify_desc.WifiNotifyReadWifiMcuPc =
		bt_notify_cb->WifiNotifyReadWifiMcuPc;
}
EXPORT_SYMBOL(register_bt_notify_callback);

void unregister_bt_notify_callback(void)
{
	fg_bt_notify_desc.WifiNotifyBtSubResetStep1 = NULL;
	fg_bt_notify_desc.WifiNotifyReadBtMcuPc = NULL;
	fg_bt_notify_desc.WifiNotifyReadWifiMcuPc = NULL;
}
EXPORT_SYMBOL(unregister_bt_notify_callback);

struct BT_NOTIFY_DESC *get_bt_notify_callback(void)
{
	return &fg_bt_notify_desc;
}
EXPORT_SYMBOL(get_bt_notify_callback);

void register_wifi_notify_callback(struct WIFI_NOTIFY_DESC *wifi_notify_cb)
{
	fg_wifi_notify_desc.BtNotifyWifiSubResetStep1 =
		wifi_notify_cb->BtNotifyWifiSubResetStep1;
}
EXPORT_SYMBOL(register_wifi_notify_callback);

void unregister_wifi_notify_callback(void)
{
	fg_wifi_notify_desc.BtNotifyWifiSubResetStep1 = NULL;
}
EXPORT_SYMBOL(unregister_wifi_notify_callback);

struct WIFI_NOTIFY_DESC *get_wifi_notify_callback(void)
{
	return &fg_wifi_notify_desc;
}
EXPORT_SYMBOL(get_wifi_notify_callback);

static int __init resetInit(void)
{
	MR_Err("%s\n", __func__);

	mutex_init(&g_prResetInfo.rResetMutex);
	g_prResetInfo.eResetState = RST_STATE_UNKNOWN;

	return 0;
}

static void __exit resetExit(void)
{
	MR_Dbg("%s\n", __func__);
}

module_init(resetInit);
module_exit(resetExit);
