/* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause */
/*
 * Copyright (c) 2017 MediaTek Inc.
 */
/*! \file   "reset.h"
*   \brief  This file contains the declairation of reset module
*/

/*******************************************************************************
*                         C O M P I L E R   F L A G S
********************************************************************************
*/

/*******************************************************************************
*                    E X T E R N A L   R E F E R E N C E S
********************************************************************************
*/
#ifndef _RESET_H
#define _RESET_H

/*******************************************************************************
*                              C O N S T A N T S
********************************************************************************
*/

/*******************************************************************************
*                             D A T A   T Y P E S
********************************************************************************
*/
enum ENUM_RST_MODULE_TYPE_T {
	RST_MODULE_BT = 0,
	RST_MODULE_WIFI,
	RST_MODULE_MAX
};

enum ENUM_RST_MODULE_STATE_TYPE_T {
	RST_MODULE_STATE_PRERESET = 0,
	RST_MODULE_STATE_KO_INSMOD,
	RST_MODULE_STATE_KO_RMMOD,
	RST_MODULE_STATE_PROBE_START,
	RST_MODULE_STATE_PROBE_DONE,
	RST_MODULE_STATE_DUMP_START,
	RST_MODULE_STATE_DUMP_END,
	RST_MODULE_STATE_MAX
};

enum ENUM_RST_MODULE_RET_TYPE_T {
	RST_MODULE_RET_SUCCESS = 0,
	RST_MODULE_RET_FAIL,
	RST_MODULE_RET_MAX
};

struct WIFI_NOTIFY_DESC {
	bool (*BtNotifyWifiSubResetStep1)(u_int8_t);
};

struct BT_NOTIFY_DESC {
	int32_t (*WifiNotifyBtSubResetStep1)(int32_t);
	int32_t (*WifiNotifyReadBtMcuPc)(uint32_t *);
	int32_t (*WifiNotifyReadWifiMcuPc)(uint8_t, uint32_t *);
};

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
#define MR_Dbg(_Fmt...)  pr_info("[reset] " _Fmt)
#define MR_Info(_Fmt...)  pr_info("[reset] " _Fmt)
#define MR_Err(_Fmt...) pr_info("[reset] " _Fmt)


/*******************************************************************************
*                   F U N C T I O N   D E C L A R A T I O N S
********************************************************************************
*/
enum ENUM_RST_MODULE_RET_TYPE_T rstNotifyWholeChipRstStatus(
				enum ENUM_RST_MODULE_TYPE_T module,
				enum ENUM_RST_MODULE_STATE_TYPE_T status,
				void *data);
void register_bt_notify_callback(struct BT_NOTIFY_DESC *bt_notify_cb);
void unregister_bt_notify_callback(void);
struct BT_NOTIFY_DESC *get_bt_notify_callback(void);
void register_wifi_notify_callback(struct WIFI_NOTIFY_DESC *wifi_notify_cb);
void unregister_wifi_notify_callback(void);
struct WIFI_NOTIFY_DESC *get_wifi_notify_callback(void);

#endif /* _RESET_H */
