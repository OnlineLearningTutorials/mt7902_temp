/* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause */
/*
 * Copyright (c) 2016 MediaTek Inc.
 */
/*
 ** Id: //Department/DaVinci/BRANCHES/MT6620_WIFI_DRIVER_V2_3/os/linux
 *      /gl_init.c#11
 */

/*! \file   gl_init.c
 *    \brief  Main routines of Linux driver
 *
 *    This file contains the main routines of Linux driver for MediaTek Inc.
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
#include "gl_os.h"
#include "debug.h"
#include "wlan_lib.h"
#include "gl_wext.h"
#include "gl_cfg80211.h"
#include "precomp.h"
#if CFG_SUPPORT_AGPS_ASSIST
#include "gl_kal.h"
#endif
#if CFG_TC1_FEATURE
#include <tc1_partition.h>
#endif
#if CFG_CHIP_RESET_SUPPORT
#include "gl_rst.h"
#endif
#include "gl_vendor.h"
#include "gl_hook_api.h"

#if CFG_POWER_OFF_CTRL_SUPPORT
#include <linux/reboot.h>
#endif
#if CFG_SUPPORT_NAN
#include "gl_vendor_ndp.h"
#endif

/*******************************************************************************
 *                              C O N S T A N T S
 *******************************************************************************
 */
/* #define MAX_IOREQ_NUM   10 */
struct semaphore g_halt_sem;
int g_u4HaltFlag;
atomic_t g_wlanRemoving;
static enum ENUM_NVRAM_STATE g_NvramFsm = NVRAM_STATE_INIT;

uint8_t g_aucNvram[MAX_CFG_FILE_WIFI_REC_SIZE];
struct wireless_dev *gprWdev[KAL_AIS_NUM];
#if CFG_REDIRECT_OID_SUPPORT
struct delayed_work oid_workq;
#endif

#if (CFG_SUPPORT_SUPPLICANT_SME == 1)
#if CFG_SUPPORT_CFG80211_QUEUE
struct delayed_work cfg80211_workq;
#endif
#endif

#if CFG_SUPPORT_PERSIST_NETDEV
struct net_device *gprNetdev[KAL_AIS_NUM] = {};
#endif

#if CFG_AP_80211KVR_INTERFACE
#define NETLINK_OSS_KERNEL 25
struct sock *nl_sk;
#endif/* CFG_AP_80211KVR_INTERFACE */

/* fos_change begin */
#if IS_ENABLED(CONFIG_IDME)
#define IDME_OF_MAC_ADDR	"/idme/mac_addr"
#define IDME_OF_BOARD_ID	"/idme/board_id"
static char idme_board_id[17];
#endif
/*******************************************************************************
 *                             D A T A   T Y P E S
 *******************************************************************************
 */
/* Tasklet mechanism is like buttom-half in Linux. We just want to
 * send a signal to OS for interrupt defer processing. All resources
 * are NOT allowed reentry, so txPacket, ISR-DPC and ioctl must avoid preempty.
 */
struct WLANDEV_INFO {
	struct net_device *prDev;
};

/*******************************************************************************
 *                            P U B L I C   D A T A
 *******************************************************************************
 */

MODULE_AUTHOR(NIC_AUTHOR);
MODULE_DESCRIPTION(NIC_DESC);
#if KERNEL_VERSION(5, 12, 0) > LINUX_VERSION_CODE
MODULE_SUPPORTED_DEVICE(NIC_NAME);
#endif

MODULE_LICENSE("Dual BSD/GPL");

#ifdef CFG_DRIVER_INF_NAME_CHANGE
char *gprifnamesta = "";
char *gprifnamep2p = "";
char *gprifnameap = "";
module_param_named(sta, gprifnamesta, charp, 0000);
module_param_named(p2p, gprifnamep2p, charp, 0000);
module_param_named(ap, gprifnameap, charp, 0000);
#endif /* CFG_DRIVER_INF_NAME_CHANGE */

/* NIC interface name */
#define NIC_INF_NAME    "wlan%d"

#ifdef CFG_DRIVER_INF_NAME_CHANGE
/* Kernel IFNAMESIZ is 16, we use 5 in case some protocol might auto gen
 * interface name,
 */
/* in that case, the interface name might have risk of over kernel's IFNAMESIZ
 */
#define CUSTOM_IFNAMESIZ 5
#endif /* CFG_DRIVER_INF_NAME_CHANGE */

uint8_t aucDebugModule[DBG_MODULE_NUM];
uint32_t au4LogLevel[ENUM_WIFI_LOG_MODULE_NUM] = {ENUM_WIFI_LOG_LEVEL_DEFAULT};

/* 4 2007/06/26, mikewu, now we don't use this, we just fix the number of wlan
 *               device to 1
 */
static struct WLANDEV_INFO
	arWlanDevInfo[CFG_MAX_WLAN_DEVICES] = { {0} };

static uint32_t
u4WlanDevNum;	/* How many NICs coexist now */

#if CFG_SUPPORT_CAL_RESULT_BACKUP_TO_HOST
u_int8_t	g_fgIsCalDataBackuped = FALSE;
#endif

/* 20150205 added work queue for sched_scan to avoid cfg80211 stop schedule scan
 *          dead loack
 */
struct delayed_work sched_workq;

#define CFG_EEPRM_FILENAME    "EEPROM"
#define FILE_NAME_MAX     64

#if (CFG_EFUSE_BUFFER_MODE_DELAY_CAL == 1)
static uint8_t *apucEepromName[] = {
	(uint8_t *) CFG_EEPRM_FILENAME "_MT",
	NULL
};
#endif

/* For running on X86 UT environment */
#if defined(UT_TEST_MODE) && defined(CFG_BUILD_X86_PLATFORM)
phys_addr_t gConEmiPhyBase;
EXPORT_SYMBOL(gConEmiPhyBase);

unsigned long long gConEmiSize;
EXPORT_SYMBOL(gConEmiSize);
#endif

/*  For DTV Ref project -> Default enable */
#if CFG_DC_USB_WOW_CALLBACK || CFG_POWER_OFF_CTRL_SUPPORT
/* Register  DC  wow callback */
int kalDcSetWow(void)
{
	struct GLUE_INFO *prGlueInfo = NULL;
	struct BUS_INFO *prBusInfo = NULL;
	struct WIFI_VAR *prWifiVar = NULL;
	uint32_t count = 0;
	int ret = 0;
#if !CFG_ENABLE_WAKE_LOCK
	uint32_t rStatus = WLAN_STATUS_SUCCESS;
	uint32_t u4BufLen;

	GLUE_SPIN_LOCK_DECLARATION();
#endif

	prGlueInfo = (struct GLUE_INFO *) wiphy_priv(wlanGetWiphy());

	if (!prGlueInfo) {
		DBGLOG(INIT, ERROR, "prGlueInfo == NULL\n");
		return -ENODEV;
	}

	if (!prGlueInfo->prAdapter) {
		DBGLOG(INIT, ERROR, "prGlueInfo->prAdapter == NULL\n");
		return -ENODEV;
	}

	if (prGlueInfo && prGlueInfo->prAdapter) {
		prWifiVar = &prGlueInfo->prAdapter->rWifiVar;

#if !CFG_ENABLE_WAKE_LOCK
		if (IS_FEATURE_ENABLED(prWifiVar->ucWow)) {
			GLUE_ACQUIRE_SPIN_LOCK(prGlueInfo, SPIN_LOCK_NET_DEV);
			if (prGlueInfo->prScanRequest) {
				kalCfg80211ScanDone(prGlueInfo->prScanRequest,
					TRUE);
				prGlueInfo->prScanRequest = NULL;
			}
			GLUE_RELEASE_SPIN_LOCK(prGlueInfo, SPIN_LOCK_NET_DEV);

			/* AIS flow: disassociation if wow_en=0 */
			DBGLOG(REQ, INFO, "Enter AIS pre-suspend\n");
			rStatus = kalIoctl(prGlueInfo,
						wlanoidAisPreSuspend,
						NULL,
						0,
						TRUE,
						FALSE,
						TRUE,
						&u4BufLen);

			/* TODO: p2pProcessPreSuspendFlow
			 * In current design, only support AIS connection
			 * during suspend only.
			 * It need to add flow to deactive P2P (GC/GO)
			 * link during suspend flow.
			 * Otherwise, MT7668 would fail to enter deep sleep.
			 */
		}
#endif

		set_bit(SUSPEND_FLAG_FOR_WAKEUP_REASON,
			&prGlueInfo->prAdapter->ulSuspendFlag);
		set_bit(SUSPEND_FLAG_CLEAR_WHEN_RESUME,
			&prGlueInfo->prAdapter->ulSuspendFlag);
	}

	prGlueInfo->fgIsInSuspendMode = TRUE;

	/* Stop upper layers calling the device hard_start_xmit routine. */
	netif_tx_stop_all_queues(prGlueInfo->prDevHandler);

	/* wait wiphy device do cfg80211 suspend done, then start hif suspend */
	if (IS_FEATURE_ENABLED(prGlueInfo->prAdapter->rWifiVar.ucWow))
		wlanWaitCfg80211SuspendDone(prGlueInfo);

	/* change to pre-Suspend state & block cfg80211 ops */
	glUsbSetState(&prGlueInfo->rHifInfo, USB_STATE_PRE_SUSPEND);

#if CFG_CHIP_RESET_SUPPORT
	if (prGlueInfo->prAdapter->chip_info->fgIsSupportL0p5Reset)
		flush_work(&prGlueInfo->rWfsysResetWork);
#endif

	wlanSuspendPmHandle(prGlueInfo);

	prBusInfo = prGlueInfo->prAdapter->chip_info->bus_info;
	if (prBusInfo->asicUsbSuspend) {
		if (prBusInfo->asicUsbSuspend(prGlueInfo->prAdapter,
			prGlueInfo))
			return 0;
		else
			return -1;
	}

	halUSBPreSuspendCmd(prGlueInfo->prAdapter);

	while (prGlueInfo->rHifInfo.state != USB_STATE_SUSPEND) {
		if (count > 250) {
			DBGLOG(HAL, ERROR, "pre_suspend timeout\n");
			ret = -EFAULT;
			break;
		}
		mdelay(2);
		count++;
	}

	glUsbSetState(&prGlueInfo->rHifInfo, USB_STATE_SUSPEND);
	halDisableInterrupt(prGlueInfo->prAdapter);
	halTxCancelAllSending(prGlueInfo->prAdapter);

	/* pending cmd will be kept in queue
	 * and no one to handle it after HIF resume.
	 * In STR, it will result in cmd buf full and then cmd buf alloc fail .
	 */
	if (IS_FEATURE_ENABLED(prGlueInfo->prAdapter->rWifiVar.ucWow))
		wlanReleaseAllTxCmdQueue(prGlueInfo->prAdapter);

	DBGLOG(HAL, STATE, "mtk_usb_suspend() done!\n");
	return ret;
}
#endif


int CFG80211_Suspend(struct wiphy *wiphy,
		     struct cfg80211_wowlan *wow)
{
	DBGLOG(INIT, INFO, "CFG80211 suspend CB\n");

	return 0;
}

int CFG80211_Resume(struct wiphy *wiphy)
{
	DBGLOG(INIT, INFO, "CFG80211 resume CB\n");

	return 0;
}

/*******************************************************************************
 *                           P R I V A T E   D A T A
 *******************************************************************************
 */

#define CHAN2G(_channel, _freq, _flags)		\
{						\
	.band               = KAL_BAND_2GHZ,	\
	.center_freq        = (_freq),		\
	.hw_value           = (_channel),	\
	.flags              = (_flags),		\
	.max_antenna_gain   = 0,		\
	.max_power          = 30,		\
}
static struct ieee80211_channel mtk_2ghz_channels[] = {
	CHAN2G(1, 2412, 0),
	CHAN2G(2, 2417, 0),
	CHAN2G(3, 2422, 0),
	CHAN2G(4, 2427, 0),
	CHAN2G(5, 2432, 0),
	CHAN2G(6, 2437, 0),
	CHAN2G(7, 2442, 0),
	CHAN2G(8, 2447, 0),
	CHAN2G(9, 2452, 0),
	CHAN2G(10, 2457, 0),
	CHAN2G(11, 2462, 0),
	CHAN2G(12, 2467, 0),
	CHAN2G(13, 2472, 0),
	CHAN2G(14, 2484, 0),
};

#define CHAN5G(_channel, _flags)					\
{									\
	.band               = KAL_BAND_5GHZ,				\
	.center_freq        =						\
		(((_channel >= 182) && (_channel <= 196)) ?		\
		(4000 + (5 * (_channel))) : (5000 + (5 * (_channel)))),	\
	.hw_value           = (_channel),				\
	.flags              = (_flags),					\
	.max_antenna_gain   = 0,					\
	.max_power          = 30,					\
}
static struct ieee80211_channel mtk_5ghz_channels[] = {
	/* UNII-1 */
	CHAN5G(36, 0),
	CHAN5G(40, 0),
	CHAN5G(44, 0),
	CHAN5G(48, 0),
	/* UNII-2 */
	CHAN5G(52, IEEE80211_CHAN_RADAR),
	CHAN5G(56, IEEE80211_CHAN_RADAR),
	CHAN5G(60, IEEE80211_CHAN_RADAR),
	CHAN5G(64, IEEE80211_CHAN_RADAR),
	/* UNII-2e */
	CHAN5G(100, IEEE80211_CHAN_RADAR),
	CHAN5G(104, IEEE80211_CHAN_RADAR),
	CHAN5G(108, IEEE80211_CHAN_RADAR),
	CHAN5G(112, IEEE80211_CHAN_RADAR),
	CHAN5G(116, IEEE80211_CHAN_RADAR),
	CHAN5G(120, IEEE80211_CHAN_RADAR),
	CHAN5G(124, IEEE80211_CHAN_RADAR),
	CHAN5G(128, IEEE80211_CHAN_RADAR),
	CHAN5G(132, IEEE80211_CHAN_RADAR),
	CHAN5G(136, IEEE80211_CHAN_RADAR),
	CHAN5G(140, IEEE80211_CHAN_RADAR),
	CHAN5G(144, IEEE80211_CHAN_RADAR),
	/* UNII-3 */
	CHAN5G(149, 0),
	CHAN5G(153, 0),
	CHAN5G(157, 0),
	CHAN5G(161, 0),
	CHAN5G(165, 0)
};

#if (CFG_SUPPORT_WIFI_6G == 1)
#define CHAN6G(_channel, _flags)					\
{									\
	.band               = KAL_BAND_6GHZ,                \
	.center_freq        = (5950 + (5 * (_channel))),    \
	.hw_value           = (_channel),               \
	.flags              = (_flags),                 \
	.max_antenna_gain   = 0,                    \
	.max_power          = 30,                   \
}

static struct ieee80211_channel mtk_6ghz_channels[] = {
	/* UNII-5 */
	CHAN6G(1, 0),
	CHAN6G(5, 0),
	CHAN6G(9, 0),
	CHAN6G(13, 0),
	CHAN6G(17, 0),
	CHAN6G(21, 0),
	CHAN6G(25, 0),
	CHAN6G(29, 0),
	CHAN6G(33, 0),
	CHAN6G(37, 0),
	CHAN6G(41, 0),
	CHAN6G(45, 0),
	CHAN6G(49, 0),
	CHAN6G(53, 0),
	CHAN6G(57, 0),
	CHAN6G(61, 0),
	CHAN6G(65, 0),
	CHAN6G(69, 0),
	CHAN6G(73, 0),
	CHAN6G(77, 0),
	CHAN6G(81, 0),
	CHAN6G(85, 0),
	CHAN6G(89, 0),
	CHAN6G(93, 0),
	/* UNII-6 */
	CHAN6G(97, 0),
	CHAN6G(101, 0),
	CHAN6G(105, 0),
	CHAN6G(109, 0),
	CHAN6G(113, 0),
	CHAN6G(117, 0),
	/* UNII-7 */
	CHAN6G(121, 0),
	CHAN6G(125, 0),
	CHAN6G(129, 0),
	CHAN6G(133, 0),
	CHAN6G(137, 0),
	CHAN6G(141, 0),
	CHAN6G(145, 0),
	CHAN6G(149, 0),
	CHAN6G(153, 0),
	CHAN6G(157, 0),
	CHAN6G(161, 0),
	CHAN6G(165, 0),
	CHAN6G(169, 0),
	CHAN6G(173, 0),
	CHAN6G(177, 0),
	CHAN6G(181, 0),
	CHAN6G(185, 0),
	/* UNII-8 */
	CHAN6G(189, 0),
	CHAN6G(193, 0),
	CHAN6G(197, 0),
	CHAN6G(201, 0),
	CHAN6G(205, 0),
	CHAN6G(209, 0),
	CHAN6G(213, 0),
	CHAN6G(217, 0),
	CHAN6G(221, 0),
	CHAN6G(225, 0),
	CHAN6G(229, 0),
	CHAN6G(233, 0)
};

#if KERNEL_VERSION(4, 19, 0) <= LINUX_VERSION_CODE
#define HE_MCS_SUPP_2SS 0xFFFA
#define HE_MCS_DONOT_SUPP 0xFFFF

#define HE_MCS_NSS_SUPP						\
{								\
	.rx_mcs_80 = CPU_TO_LE16(HE_MCS_SUPP_2SS),		\
	.tx_mcs_80 = CPU_TO_LE16(HE_MCS_SUPP_2SS),		\
	.rx_mcs_160 = CPU_TO_LE16(HE_MCS_DONOT_SUPP),		\
	.tx_mcs_160 = CPU_TO_LE16(HE_MCS_DONOT_SUPP),		\
	.rx_mcs_80p80 = CPU_TO_LE16(HE_MCS_DONOT_SUPP),		\
	.tx_mcs_80p80 = CPU_TO_LE16(HE_MCS_DONOT_SUPP),		\
}

#define WLAN_HE_CAP						\
{								\
	.has_he        = true,					\
	.he_mcs_nss_supp = HE_MCS_NSS_SUPP,			\
}

#define IFTYPE6G(_maskbit)					\
{								\
	.types_mask         = BIT(_maskbit),			\
	.he_cap             = WLAN_HE_CAP,			\
}

static struct ieee80211_sband_iftype_data mtk_6ghz_iftype_data[] = {
	IFTYPE6G(IFTYPE_STATION),
	IFTYPE6G(IFTYPE_AP),
	IFTYPE6G(IFTYPE_P2P_CLIENT),
	IFTYPE6G(IFTYPE_P2P_GO)
};
#endif

#endif

#define RATETAB_ENT(_rate, _rateid, _flags)	\
{						\
	.bitrate    = (_rate),			\
	.hw_value   = (_rateid),		\
	.flags      = (_flags),			\
}

/* for cfg80211 - rate table */
static struct ieee80211_rate mtk_rates[] = {
	RATETAB_ENT(10, 0x1000, 0),
	RATETAB_ENT(20, 0x1001, 0),
	RATETAB_ENT(55, 0x1002, 0),
	RATETAB_ENT(110, 0x1003, 0),	/* 802.11b */
	RATETAB_ENT(60, 0x2000, 0),
	RATETAB_ENT(90, 0x2001, 0),
	RATETAB_ENT(120, 0x2002, 0),
	RATETAB_ENT(180, 0x2003, 0),
	RATETAB_ENT(240, 0x2004, 0),
	RATETAB_ENT(360, 0x2005, 0),
	RATETAB_ENT(480, 0x2006, 0),
	RATETAB_ENT(540, 0x2007, 0),	/* 802.11a/g */
};

#define mtk_a_rates         (mtk_rates + 4)
#define mtk_a_rates_size    (ARRAY_SIZE(mtk_rates) - 4)
#define mtk_g_rates         (mtk_rates + 0)
#define mtk_g_rates_size    (ARRAY_SIZE(mtk_rates) - 0)

#define WLAN_MCS_INFO						\
{								\
	.rx_mask        = {0xff, 0, 0, 0, 0, 0, 0, 0, 0, 0},	\
	.rx_highest     = 0,					\
	.tx_params      = IEEE80211_HT_MCS_TX_DEFINED,		\
}

#define WLAN_VHT_MCS_INFO					\
{								\
	.rx_mcs_map     = 0xFFFA,				\
	.rx_highest     = cpu_to_le16(867),			\
	.tx_mcs_map     = 0xFFFA,				\
	.tx_highest     = cpu_to_le16(867),			\
}


#define WLAN_HT_CAP						\
{								\
	.ht_supported   = true,					\
	.cap            = IEEE80211_HT_CAP_SUP_WIDTH_20_40	\
			| IEEE80211_HT_CAP_SM_PS		\
			| IEEE80211_HT_CAP_GRN_FLD		\
			| IEEE80211_HT_CAP_SGI_20		\
			| IEEE80211_HT_CAP_SGI_40,		\
	.ampdu_factor   = IEEE80211_HT_MAX_AMPDU_64K,		\
	.ampdu_density  = IEEE80211_HT_MPDU_DENSITY_NONE,	\
	.mcs            = WLAN_MCS_INFO,			\
}

#define WLAN_VHT_CAP							\
{									\
	.vht_supported  = true,						\
	.cap            = IEEE80211_VHT_CAP_RXLDPC			\
			| IEEE80211_VHT_CAP_SUPP_CHAN_WIDTH_MASK	\
			| IEEE80211_VHT_CAP_MAX_MPDU_LENGTH_11454	\
			| IEEE80211_VHT_CAP_RXLDPC			\
			| IEEE80211_VHT_CAP_SHORT_GI_80			\
			| IEEE80211_VHT_CAP_TXSTBC			\
			| IEEE80211_VHT_CAP_SU_BEAMFORMER_CAPABLE	\
			| IEEE80211_VHT_CAP_SU_BEAMFORMEE_CAPABLE,	\
	.vht_mcs        = WLAN_VHT_MCS_INFO,				\
}

/* public for both Legacy Wi-Fi / P2P access */
struct ieee80211_supported_band mtk_band_2ghz = {
	.band = KAL_BAND_2GHZ,
	.channels = mtk_2ghz_channels,
	.n_channels = ARRAY_SIZE(mtk_2ghz_channels),
	.bitrates = mtk_g_rates,
	.n_bitrates = mtk_g_rates_size,
	.ht_cap = WLAN_HT_CAP,
};

/* public for both Legacy Wi-Fi / P2P access */
struct ieee80211_supported_band mtk_band_5ghz = {
	.band = KAL_BAND_5GHZ,
	.channels = mtk_5ghz_channels,
	.n_channels = ARRAY_SIZE(mtk_5ghz_channels),
	.bitrates = mtk_a_rates,
	.n_bitrates = mtk_a_rates_size,
	.ht_cap = WLAN_HT_CAP,
	.vht_cap = WLAN_VHT_CAP,
};

#if (CFG_SUPPORT_WIFI_6G == 1)
/* public for both Legacy Wi-Fi / P2P access */
struct ieee80211_supported_band mtk_band_6ghz = {
	.band = KAL_BAND_6GHZ,
	.channels = mtk_6ghz_channels,
	.n_channels = ARRAY_SIZE(mtk_6ghz_channels),
	.bitrates = mtk_a_rates,
	.n_bitrates = mtk_a_rates_size,
#if KERNEL_VERSION(4, 19, 0) <= LINUX_VERSION_CODE
	.n_iftype_data = ARRAY_SIZE(mtk_6ghz_iftype_data),
	.iftype_data = mtk_6ghz_iftype_data,
#endif
};
#endif

const uint32_t mtk_cipher_suites[] = {
	/* keep WEP first, it may be removed below */
	WLAN_CIPHER_SUITE_WEP40,
	WLAN_CIPHER_SUITE_WEP104,
	WLAN_CIPHER_SUITE_TKIP,
	WLAN_CIPHER_SUITE_CCMP,

	/* keep last -- depends on hw flags! */
	WLAN_CIPHER_SUITE_AES_CMAC,
	WLAN_CIPHER_SUITE_GCMP_256,
	WLAN_CIPHER_SUITE_BIP_GMAC_256, /* TODO, HW not support,
					* SW should handle integrity check
					*/
	WLAN_CIPHER_SUITE_NO_GROUP_ADDR
};

#if KERNEL_VERSION(5, 4, 0) <= LINUX_VERSION_CODE
const uint32_t mtk_akm_suites[] = {
#if CFG_SUPPORT_802_11R
	WLAN_AKM_SUITE_FT_8021X,
	WLAN_AKM_SUITE_FT_PSK,
#endif
#if CFG_SUPPORT_802_11W
	WLAN_AKM_SUITE_8021X_SHA256,
	WLAN_AKM_SUITE_PSK_SHA256,
#endif
#if CFG_SUPPORT_OWE
	WLAN_AKM_SUITE_OWE,
#endif
#if CFG_SUPPORT_DPP
	WLAN_AKM_SUITE_DPP,
#endif
#if CFG_SUPPORT_SUPPLICANT_SME
	WLAN_AKM_SUITE_SAE,
#endif
	WLAN_AKM_SUITE_8021X,
	WLAN_AKM_SUITE_PSK,
	WLAN_AKM_SUITE_8021X_SUITE_B
};
#endif

#if (CFG_ENABLE_UNIFY_WIPHY == 0)
static struct cfg80211_ops mtk_wlan_ops = {
	.suspend = mtk_cfg80211_suspend,
	.resume = mtk_cfg80211_resume,
	.change_virtual_intf = mtk_cfg80211_change_iface,
	.add_key = mtk_cfg80211_add_key,
	.get_key = mtk_cfg80211_get_key,
	.del_key = mtk_cfg80211_del_key,
	.set_default_key = mtk_cfg80211_set_default_key,
	.get_station = mtk_cfg80211_get_station,
#if CFG_SUPPORT_TDLS
	.change_station = mtk_cfg80211_change_station,
	.add_station = mtk_cfg80211_add_station,
	.del_station = mtk_cfg80211_del_station,
#endif
	.scan = mtk_cfg80211_scan,
#if KERNEL_VERSION(4, 5, 0) <= CFG80211_VERSION_CODE
	.abort_scan = mtk_cfg80211_abort_scan,
#endif
#if (CFG_SUPPORT_SUPPLICANT_SME == 0)
	.connect = mtk_cfg80211_connect,
	.disconnect = mtk_cfg80211_disconnect,
#endif
#if ((CFG_SUPPORT_SUPPLICANT_SME == 1) ||
	(CFG_ENABLE_WIFI_DIRECT_CFG_80211 != 0))
	.deauth = mtk_cfg80211_deauth,
	.disassoc = mtk_cfg80211_disassoc,
#endif
	.join_ibss = mtk_cfg80211_join_ibss,
	.leave_ibss = mtk_cfg80211_leave_ibss,
	.set_power_mgmt = mtk_cfg80211_set_power_mgmt,
	.set_pmksa = mtk_cfg80211_set_pmksa,
	.del_pmksa = mtk_cfg80211_del_pmksa,
	.flush_pmksa = mtk_cfg80211_flush_pmksa,
#if CONFIG_SUPPORT_GTK_REKEY
	.set_rekey_data = mtk_cfg80211_set_rekey_data,
#endif
#if (CFG_SUPPORT_SUPPLICANT_SME == 1)
	.auth = mtk_cfg80211_auth,
#endif
	.assoc = mtk_cfg80211_assoc,

	/* Action Frame TX/RX */
	.remain_on_channel = mtk_cfg80211_remain_on_channel,
	.cancel_remain_on_channel = mtk_cfg80211_cancel_remain_on_channel,
	.mgmt_tx = mtk_cfg80211_mgmt_tx,
	/* .mgmt_tx_cancel_wait        = mtk_cfg80211_mgmt_tx_cancel_wait, */
#if KERNEL_VERSION(5, 8, 0) <= CFG80211_VERSION_CODE
	.update_mgmt_frame_registrations = mtk_cfg_mgmt_frame_update,
#else
	.mgmt_frame_register = mtk_cfg80211_mgmt_frame_register,
#endif

#ifdef CONFIG_NL80211_TESTMODE
	.testmode_cmd = mtk_cfg80211_testmode_cmd,
#endif
#if CFG_SUPPORT_SCHED_SCAN
	.sched_scan_start = mtk_cfg80211_sched_scan_start,
	.sched_scan_stop = mtk_cfg80211_sched_scan_stop,
#endif /* CFG_SUPPORT_SCHED_SCAN */
#if CFG_SUPPORT_TDLS
	.tdls_oper = mtk_cfg80211_tdls_oper,
	.tdls_mgmt = mtk_cfg80211_tdls_mgmt,
#endif
	.update_ft_ies = mtk_cfg80211_update_ft_ies,
#if CFG_SUPPORT_ROAMING
	.set_cqm_rssi_config = mtk_cfg80211_set_cqm_rssi_config,
#if KERNEL_VERSION(4, 14, 0) <= CFG80211_VERSION_CODE
	.set_cqm_rssi_range_config = mtk_cfg80211_set_cqm_rssi_range_config,
#endif
	.set_cqm_txe_config = mtk_cfg80211_set_cqm_txe_config,
#endif

#if (CFG_SUPPORT_SNIFFER_RADIOTAP == 1)
	.set_monitor_channel = mtk_cfg80211_set_monitor_channel,
#endif
};
#else /* CFG_ENABLE_UNIFY_WIPHY */
static struct cfg80211_ops mtk_cfg_ops = {
	.add_virtual_intf = mtk_cfg_add_iface,
	.del_virtual_intf = mtk_cfg_del_iface,
	.change_virtual_intf = mtk_cfg_change_iface,
	.add_key = mtk_cfg_add_key,
	.get_key = mtk_cfg_get_key,
	.del_key = mtk_cfg_del_key,
	.set_default_mgmt_key = mtk_cfg_set_default_mgmt_key,
	.set_default_key = mtk_cfg_set_default_key,
	.get_station = mtk_cfg_get_station,
#if CFG_SUPPORT_TDLS
	.change_station = mtk_cfg_change_station,
	.add_station = mtk_cfg_add_station,
	.tdls_oper = mtk_cfg_tdls_oper,
	.tdls_mgmt = mtk_cfg_tdls_mgmt,
#endif
	.del_station = mtk_cfg_del_station,	/* AP/P2P use this function */
	.scan = mtk_cfg_scan,
#if KERNEL_VERSION(4, 5, 0) <= CFG80211_VERSION_CODE
	.abort_scan = mtk_cfg_abort_scan,
#endif
#if CFG_SUPPORT_SCHED_SCAN
	.sched_scan_start = mtk_cfg_sched_scan_start,
	.sched_scan_stop = mtk_cfg_sched_scan_stop,
#endif /* CFG_SUPPORT_SCHED_SCAN */
#if (CFG_SUPPORT_SUPPLICANT_SME == 0)
	.connect = mtk_cfg_connect,
	.disconnect = mtk_cfg_disconnect,
#endif
	.deauth = mtk_cfg_deauth,
	.join_ibss = mtk_cfg_join_ibss,
	.leave_ibss = mtk_cfg_leave_ibss,
	.set_power_mgmt = mtk_cfg_set_power_mgmt,
	.set_pmksa = mtk_cfg_set_pmksa,
	.del_pmksa = mtk_cfg_del_pmksa,
	.flush_pmksa = mtk_cfg_flush_pmksa,
#if CONFIG_SUPPORT_GTK_REKEY
	.set_rekey_data = mtk_cfg_set_rekey_data,
#endif
	.suspend = mtk_cfg_suspend,
	.resume = mtk_cfg_resume,
#if (CFG_SUPPORT_SUPPLICANT_SME == 1)
	.auth = mtk_cfg_auth,
#endif
	.assoc = mtk_cfg_assoc,

	/* Action Frame TX/RX */
	.remain_on_channel = mtk_cfg_remain_on_channel,
	.cancel_remain_on_channel = mtk_cfg_cancel_remain_on_channel,
	.mgmt_tx = mtk_cfg_mgmt_tx,
	/* .mgmt_tx_cancel_wait        = mtk_cfg80211_mgmt_tx_cancel_wait, */
#if KERNEL_VERSION(5, 8, 0) <= CFG80211_VERSION_CODE
	.update_mgmt_frame_registrations = mtk_cfg_mgmt_frame_update,
#else
	.mgmt_frame_register = mtk_cfg_mgmt_frame_register,
#endif

#ifdef CONFIG_NL80211_TESTMODE
	.testmode_cmd = mtk_cfg_testmode_cmd,
#endif

#if (CFG_SUPPORT_DFS_MASTER == 1)
	.start_radar_detection = mtk_cfg_start_radar_detection,
#if KERNEL_VERSION(3, 13, 0) <= CFG80211_VERSION_CODE
	.channel_switch = mtk_cfg_channel_switch,
#endif
#endif

#if (CFG_ENABLE_WIFI_DIRECT_CFG_80211 != 0)
	.change_bss = mtk_cfg_change_bss,
	.mgmt_tx_cancel_wait = mtk_cfg_mgmt_tx_cancel_wait,
	.disassoc = mtk_cfg_disassoc,
	.start_ap = mtk_cfg_start_ap,
	.change_beacon = mtk_cfg_change_beacon,
	.stop_ap = mtk_cfg_stop_ap,
	.set_wiphy_params = mtk_cfg_set_wiphy_params,
	.set_bitrate_mask = mtk_cfg_set_bitrate_mask,
	.set_tx_power = mtk_cfg_set_txpower,
	.get_tx_power = mtk_cfg_get_txpower,
#endif
	.update_ft_ies = mtk_cfg80211_update_ft_ies,
#if CFG_SUPPORT_ROAMING
	.set_cqm_rssi_config = mtk_cfg80211_set_cqm_rssi_config,
#if KERNEL_VERSION(4, 14, 0) <= CFG80211_VERSION_CODE
	.set_cqm_rssi_range_config = mtk_cfg80211_set_cqm_rssi_range_config,
#endif
	.set_cqm_txe_config = mtk_cfg80211_set_cqm_txe_config,

#endif
#if (CFG_SUPPORT_SNIFFER_RADIOTAP == 1)
	.set_monitor_channel = mtk_cfg80211_set_monitor_channel,
#endif

};
#endif	/* CFG_ENABLE_UNIFY_WIPHY */

#if KERNEL_VERSION(5, 3, 0) <= CFG80211_VERSION_CODE
#define VENDOR_OPS_SET_POLICY(_val) .policy = (_val),
#else
#define VENDOR_OPS_SET_POLICY(_val)
#endif


#if KERNEL_VERSION(3, 18, 0) <= CFG80211_VERSION_CODE

static const struct wiphy_vendor_command
	mtk_wlan_vendor_ops[] = {
	{
		{
			.vendor_id = GOOGLE_OUI,
			.subcmd = WIFI_SUBCMD_GET_CHANNEL_LIST
		},
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |
			 WIPHY_VENDOR_CMD_NEED_NETDEV,
		.doit = mtk_cfg80211_vendor_get_channel_list,
#if KERNEL_VERSION(5, 4, 0) <= LINUX_VERSION_CODE
		.policy = nla_parse_wifi_attribute,
		.maxattr = WIFI_ATTRIBUTE_MAX
#else
		VENDOR_OPS_SET_POLICY(VENDOR_CMD_RAW_DATA)
#endif
	},
	{
		{
			.vendor_id = GOOGLE_OUI,
			.subcmd = WIFI_SUBCMD_SET_COUNTRY_CODE
		},
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |
			 WIPHY_VENDOR_CMD_NEED_NETDEV,
		.doit = mtk_cfg80211_vendor_set_country_code,
#if KERNEL_VERSION(5, 4, 0) <= LINUX_VERSION_CODE
		.policy = nla_parse_wifi_attribute,
		.maxattr = WIFI_ATTRIBUTE_MAX
#else
		VENDOR_OPS_SET_POLICY(VENDOR_CMD_RAW_DATA)
#endif
	},
	{
		{
			.vendor_id = GOOGLE_OUI,
			.subcmd = WIFI_SUBCMD_SET_PNO_RANDOM_MAC_OUI
		},
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV
			| WIPHY_VENDOR_CMD_NEED_NETDEV
			| WIPHY_VENDOR_CMD_NEED_RUNNING,
		.doit = mtk_cfg80211_vendor_set_scan_mac_oui,
#if KERNEL_VERSION(5, 4, 0) <= LINUX_VERSION_CODE
		.policy = nla_parse_wifi_attribute,
		.maxattr = WIFI_ATTRIBUTE_MAX
#else
		VENDOR_OPS_SET_POLICY(VENDOR_CMD_RAW_DATA)
#endif
	},
	{
		{
			.vendor_id = OUI_QCA,
			.subcmd = QCA_NL80211_VENDOR_SUBCMD_SETBAND
		},
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |
			 WIPHY_VENDOR_CMD_NEED_NETDEV,
		.doit = mtk_cfg80211_vendor_set_band,
#if KERNEL_VERSION(5, 4, 0) <= LINUX_VERSION_CODE
		.policy = nal_parse_wifi_setband,
		.maxattr = QCA_WLAN_VENDOR_ATTR_MAX
#else
		VENDOR_OPS_SET_POLICY(VENDOR_CMD_RAW_DATA)
#endif

	},
#if CFG_SUPPORT_MBO
	{
		{
			.vendor_id = OUI_QCA,
			.subcmd = QCA_NL80211_VENDOR_SUBCMD_ROAM
		},
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |
			 WIPHY_VENDOR_CMD_NEED_NETDEV,
		.doit = mtk_cfg80211_vendor_set_roaming_param,
#if KERNEL_VERSION(5, 4, 0) <= LINUX_VERSION_CODE
		.policy = qca_roaming_param_policy,
		.maxattr = QCA_ATTR_ROAMING_PARAM_MAX
#else
		VENDOR_OPS_SET_POLICY(VENDOR_CMD_RAW_DATA)
#endif
	},
#endif
	{
		{
			.vendor_id = OUI_QCA,
			.subcmd = WIFI_SUBCMD_SET_ROAMING
		},
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |
			 WIPHY_VENDOR_CMD_NEED_NETDEV,
		.doit = mtk_cfg80211_vendor_set_roaming_policy,
		VENDOR_OPS_SET_POLICY(VENDOR_CMD_RAW_DATA)

	},
	{
		{
			.vendor_id = GOOGLE_OUI,
			.subcmd = WIFI_SUBCMD_GET_ROAMING_CAPABILITIES
		},
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |
			 WIPHY_VENDOR_CMD_NEED_NETDEV,
		.doit = mtk_cfg80211_vendor_get_roaming_capabilities,
		VENDOR_OPS_SET_POLICY(VENDOR_CMD_RAW_DATA)
	},
	{
		{
			.vendor_id = GOOGLE_OUI,
			.subcmd = WIFI_SUBCMD_CONFIG_ROAMING
		},
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |
			 WIPHY_VENDOR_CMD_NEED_NETDEV,
		.doit = mtk_cfg80211_vendor_config_roaming,
#if KERNEL_VERSION(5, 4, 0) <= LINUX_VERSION_CODE
		.policy = nla_parse_wifi_attribute,
		.maxattr = WIFI_ATTRIBUTE_MAX
#else
		VENDOR_OPS_SET_POLICY(VENDOR_CMD_RAW_DATA)
#endif
	},
	{
		{
			.vendor_id = GOOGLE_OUI,
			.subcmd = WIFI_SUBCMD_ENABLE_ROAMING
		},
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |
			 WIPHY_VENDOR_CMD_NEED_NETDEV,
		.doit = mtk_cfg80211_vendor_enable_roaming,
#if KERNEL_VERSION(5, 4, 0) <= LINUX_VERSION_CODE
		.policy = nla_parse_wifi_attribute,
		.maxattr = WIFI_ATTRIBUTE_MAX
#else
		VENDOR_OPS_SET_POLICY(VENDOR_CMD_RAW_DATA)
#endif
	},
	/* RTT */
	{
		{
			.vendor_id = GOOGLE_OUI,
			.subcmd = RTT_SUBCMD_GETCAPABILITY
		},
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |
		WIPHY_VENDOR_CMD_NEED_NETDEV,
		.doit = mtk_cfg80211_vendor_get_rtt_capabilities,
		VENDOR_OPS_SET_POLICY(VENDOR_CMD_RAW_DATA)
	},
	/* Link Layer Statistics */
	{
		{
			.vendor_id = GOOGLE_OUI,
			.subcmd = LSTATS_SUBCMD_GET_INFO
		},
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |
		WIPHY_VENDOR_CMD_NEED_NETDEV,
		.doit = mtk_cfg80211_vendor_llstats_get_info,
		VENDOR_OPS_SET_POLICY(VENDOR_CMD_RAW_DATA)
	},
	/* RSSI Monitoring */
	{
		{
			.vendor_id = GOOGLE_OUI,
			.subcmd = WIFI_SUBCMD_SET_RSSI_MONITOR
		},
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |
		WIPHY_VENDOR_CMD_NEED_NETDEV,
		.doit = mtk_cfg80211_vendor_set_rssi_monitoring,
#if KERNEL_VERSION(5, 4, 0) <= LINUX_VERSION_CODE
		.policy = nla_parse_wifi_rssi_monitor,
		.maxattr = WIFI_ATTRIBUTE_RSSI_MONITOR_ATTRIBUTE_MAX
#else
		VENDOR_OPS_SET_POLICY(VENDOR_CMD_RAW_DATA)
#endif
	},
	/* Packet Keep Alive */
	{
		{
			.vendor_id = GOOGLE_OUI,
			.subcmd = WIFI_OFFLOAD_START_MKEEP_ALIVE
		},
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |
		WIPHY_VENDOR_CMD_NEED_NETDEV,
		.doit = mtk_cfg80211_vendor_packet_keep_alive_start,
#if KERNEL_VERSION(5, 4, 0) <= LINUX_VERSION_CODE
		.policy = nla_parse_offloading_policy,
		.maxattr = MKEEP_ALIVE_ATTRIBUTE_MAX
#else
		VENDOR_OPS_SET_POLICY(VENDOR_CMD_RAW_DATA)
#endif
	},
	{
		{
			.vendor_id = GOOGLE_OUI,
			.subcmd = WIFI_OFFLOAD_STOP_MKEEP_ALIVE
		},
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |
		WIPHY_VENDOR_CMD_NEED_NETDEV,
		.doit = mtk_cfg80211_vendor_packet_keep_alive_stop,
#if KERNEL_VERSION(5, 4, 0) <= LINUX_VERSION_CODE
		.policy = nla_parse_offloading_policy,
		.maxattr = MKEEP_ALIVE_ATTRIBUTE_MAX
#else
		VENDOR_OPS_SET_POLICY(VENDOR_CMD_RAW_DATA)
#endif
	},
	/* Get Driver Version or Firmware Version */
	{
		{
			.vendor_id = GOOGLE_OUI,
			.subcmd = LOGGER_GET_VER
		},
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |
				WIPHY_VENDOR_CMD_NEED_NETDEV,
		.doit = mtk_cfg80211_vendor_get_version,
#if KERNEL_VERSION(5, 4, 0) <= LINUX_VERSION_CODE
		.policy = nla_get_version_policy,
		.maxattr = LOGGER_ATTRIBUTE_MAX
#else
		VENDOR_OPS_SET_POLICY(VENDOR_CMD_RAW_DATA)
#endif
	},
	/* Get Supported Feature Set */
	{
		{
			.vendor_id = GOOGLE_OUI,
			.subcmd = WIFI_SUBCMD_GET_FEATURE_SET
		},
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |
				WIPHY_VENDOR_CMD_NEED_NETDEV,
		.doit = mtk_cfg80211_vendor_get_supported_feature_set,
		VENDOR_OPS_SET_POLICY(VENDOR_CMD_RAW_DATA)
	},
	/* Set Tx Power Scenario */
	{
		{
			.vendor_id = GOOGLE_OUI,
			.subcmd = WIFI_SUBCMD_SELECT_TX_POWER_SCENARIO
		},
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |
				WIPHY_VENDOR_CMD_NEED_NETDEV,
		.doit = mtk_cfg80211_vendor_set_tx_power_scenario,
#if KERNEL_VERSION(5, 4, 0) <= LINUX_VERSION_CODE
		.policy = nla_parse_wifi_attribute,
		.maxattr = WIFI_ATTRIBUTE_MAX
#else
		VENDOR_OPS_SET_POLICY(VENDOR_CMD_RAW_DATA)
#endif
	},
#if CFG_SUPPORT_P2P_PREFERRED_FREQ_LIST
	/* P2P get preferred freq list */
	{
		{
			.vendor_id = OUI_QCA,
			.subcmd = NL80211_VENDOR_SUBCMD_GET_PREFER_FREQ_LIST
		},
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV
				| WIPHY_VENDOR_CMD_NEED_NETDEV
				| WIPHY_VENDOR_CMD_NEED_RUNNING,
		.doit = mtk_cfg80211_vendor_get_preferred_freq_list,
#if KERNEL_VERSION(5, 4, 0) <= LINUX_VERSION_CODE
		.policy = nla_get_preferred_freq_list_policy,
		.maxattr = WIFI_VENDOR_ATTR_PREFERRED_FREQ_LIST_MAX
#else
		VENDOR_OPS_SET_POLICY(VENDOR_CMD_RAW_DATA)
#endif
	},
#endif /* CFG_SUPPORT_P2P_PREFERRED_FREQ_LIST */
#if CFG_AUTO_CHANNEL_SEL_SUPPORT
	{
		{
			.vendor_id = OUI_QCA,
			.subcmd = NL80211_VENDOR_SUBCMD_ACS
		},
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV
				| WIPHY_VENDOR_CMD_NEED_NETDEV
				| WIPHY_VENDOR_CMD_NEED_RUNNING,
		.doit = mtk_cfg80211_vendor_acs,
#if KERNEL_VERSION(5, 4, 0) <= LINUX_VERSION_CODE
		.policy = nla_get_acs_policy,
		.maxattr = WIFI_VENDOR_ATTR_ACS_MAX
#else
		VENDOR_OPS_SET_POLICY(VENDOR_CMD_RAW_DATA)
#endif
	},
#endif
	{
		{
			.vendor_id = OUI_QCA,
			.subcmd = NL80211_VENDOR_SUBCMD_GET_FEATURES
		},
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV
				| WIPHY_VENDOR_CMD_NEED_NETDEV,
		.doit = mtk_cfg80211_vendor_get_features,
		VENDOR_OPS_SET_POLICY(VENDOR_CMD_RAW_DATA)
	},
	{
		{
			.vendor_id = OUI_MTK,
			.subcmd =  NL80211_VENDOR_SUBCMD_GET_APF_CAPABILITIES
		},
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |
				 WIPHY_VENDOR_CMD_NEED_NETDEV,
		.doit = mtk_cfg80211_vendor_get_apf_capabilities
#if KERNEL_VERSION(5, 4, 0) <= LINUX_VERSION_CODE
		,
		.policy = nla_get_apf_policy,
		.maxattr = APF_ATTRIBUTE_MAX
#else
		VENDOR_OPS_SET_POLICY(VENDOR_CMD_RAW_DATA)
#endif
	},
	/* Get Driver Memory Dump */
	{
		{
			.vendor_id = GOOGLE_OUI,
			.subcmd = LOGGER_DRIVER_MEM_DUMP
		},
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |
				WIPHY_VENDOR_CMD_NEED_NETDEV,
		.doit = mtk_cfg80211_vendor_driver_memory_dump,
		VENDOR_OPS_SET_POLICY(VENDOR_CMD_RAW_DATA)
	},
#if CFG_SUPPORT_NAN
	{
		{
			.vendor_id = OUI_MTK,
			.subcmd = NL80211_VENDOR_SUBCMD_NAN
		},
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |
				WIPHY_VENDOR_CMD_NEED_NETDEV |
				WIPHY_VENDOR_CMD_NEED_RUNNING,
		.doit = mtk_cfg80211_vendor_nan,
#if KERNEL_VERSION(5, 4, 0) <= CFG80211_VERSION_CODE
		VENDOR_OPS_SET_POLICY(VENDOR_CMD_RAW_DATA)
#endif
	},
	{
		{
			.vendor_id = OUI_MTK,
			.subcmd = NL80211_VENDOR_SUBCMD_NDP
		},
		.flags = WIPHY_VENDOR_CMD_NEED_WDEV |
				WIPHY_VENDOR_CMD_NEED_NETDEV |
				WIPHY_VENDOR_CMD_NEED_RUNNING,
		.doit = mtk_cfg80211_vendor_ndp,
#if KERNEL_VERSION(5, 4, 0) <= CFG80211_VERSION_CODE
		.policy = mtk_wlan_vendor_ndp_policy,
		.maxattr = MTK_WLAN_VENDOR_ATTR_NDP_PARAMS_MAX
#endif
	},
#endif
};

static const struct nl80211_vendor_cmd_info
	mtk_wlan_vendor_events[] = {
	{
		.vendor_id = GOOGLE_OUI,
		.subcmd = GSCAN_EVENT_SIGNIFICANT_CHANGE_RESULTS
	},
	{
		.vendor_id = GOOGLE_OUI,
		.subcmd = GSCAN_EVENT_HOTLIST_RESULTS_FOUND
	},
	{
		.vendor_id = GOOGLE_OUI,
		.subcmd = GSCAN_EVENT_SCAN_RESULTS_AVAILABLE
	},
	{
		.vendor_id = GOOGLE_OUI,
		.subcmd = GSCAN_EVENT_FULL_SCAN_RESULTS
	},
	{
		.vendor_id = GOOGLE_OUI,
		.subcmd = RTT_EVENT_COMPLETE
	},
	{
		.vendor_id = GOOGLE_OUI,
		.subcmd = GSCAN_EVENT_COMPLETE_SCAN
	},
	{
		.vendor_id = GOOGLE_OUI,
		.subcmd = GSCAN_EVENT_HOTLIST_RESULTS_LOST
	},
	{
		.vendor_id = GOOGLE_OUI,
		.subcmd = WIFI_EVENT_RSSI_MONITOR
	},
#if CFG_SUPPORT_MAGIC_PKT_VENDOR_EVENT
	{
		.vendor_id = GOOGLE_OUI,
		.subcmd = WIFI_EVENT_MAGIC_PACKET_RECEIVED
	},
#endif
	{
		.vendor_id = OUI_QCA,
		.subcmd = NL80211_VENDOR_SUBCMD_ACS
	},
	{
		.vendor_id = OUI_MTK,
		.subcmd = WIFI_EVENT_DRIVER_ERROR
	},
	{
		.vendor_id = OUI_MTK,
		.subcmd = NL80211_VENDOR_SUBCMD_NAN
	},
	{
		.vendor_id = OUI_MTK,
		.subcmd = NL80211_VENDOR_SUBCMD_NDP
	},

};
#endif

/* There isn't a lot of sense in it, but you can transmit anything you like */
static const struct ieee80211_txrx_stypes
	mtk_cfg80211_ais_default_mgmt_stypes[NUM_NL80211_IFTYPES] = {
	[NL80211_IFTYPE_ADHOC] = {
		.tx = 0xffff,
		.rx = BIT(IEEE80211_STYPE_ACTION >> 4)
	},
	[NL80211_IFTYPE_STATION] = {
		.tx = 0xffff,
		.rx = BIT(IEEE80211_STYPE_ACTION >> 4) |
		      BIT(IEEE80211_STYPE_PROBE_REQ >> 4)
	},
	[NL80211_IFTYPE_AP] = {
		.tx = 0xffff,
		.rx = BIT(IEEE80211_STYPE_PROBE_REQ >> 4)
			| BIT(IEEE80211_STYPE_ACTION >> 4)
#if CFG_SUPPORT_SOFTAP_WPA3
			| BIT(IEEE80211_STYPE_ASSOC_REQ >> 4) |
			  BIT(IEEE80211_STYPE_REASSOC_REQ >> 4) |
			  BIT(IEEE80211_STYPE_DISASSOC >> 4) |
			  BIT(IEEE80211_STYPE_AUTH >> 4) |
			  BIT(IEEE80211_STYPE_DEAUTH >> 4)
#endif
	},
	[NL80211_IFTYPE_AP_VLAN] = {
		/* copy AP */
		.tx = 0xffff,
		.rx = BIT(IEEE80211_STYPE_ASSOC_REQ >> 4) |
		BIT(IEEE80211_STYPE_REASSOC_REQ >> 4) |
		BIT(IEEE80211_STYPE_PROBE_REQ >> 4) |
		BIT(IEEE80211_STYPE_DISASSOC >> 4) |
		BIT(IEEE80211_STYPE_AUTH >> 4) |
		BIT(IEEE80211_STYPE_DEAUTH >> 4) |
		BIT(IEEE80211_STYPE_ACTION >> 4)
	},
	[NL80211_IFTYPE_P2P_CLIENT] = {
		.tx = 0xffff,
		.rx = BIT(IEEE80211_STYPE_ACTION >> 4) |
		      BIT(IEEE80211_STYPE_PROBE_REQ >> 4)
	},
	[NL80211_IFTYPE_P2P_GO] = {
		.tx = 0xffff,
		.rx = BIT(IEEE80211_STYPE_PROBE_REQ >> 4) |
		      BIT(IEEE80211_STYPE_ACTION >> 4)
	}
};

#ifdef CONFIG_PM
static const struct wiphy_wowlan_support mtk_wlan_wowlan_support = {
	.flags = WIPHY_WOWLAN_MAGIC_PKT | WIPHY_WOWLAN_DISCONNECT |
		 WIPHY_WOWLAN_ANY,
};
#endif

#if (CFG_SUPPORT_SUPPLICANT_SME == 1)
/* NL80211_FEATURE_DS_PARAM_SET_IE_IN_PROBES & NL80211_FEATURE_QUIET
 * support in linux kernet version => 3.18
 */
#if KERNEL_VERSION(3, 18, 0) > CFG80211_VERSION_CODE
#define NL80211_FEATURE_DS_PARAM_SET_IE_IN_PROBES BIT(19)
#define NL80211_FEATURE_QUIET BIT(21)
#define NL80211_FEATURE_TX_POWER_INSERTION BIT(22)
#endif
#endif

/*******************************************************************************
 *                                 M A C R O S
 *******************************************************************************
 */

/*******************************************************************************
 *                   F U N C T I O N   D E C L A R A T I O N S
 *******************************************************************************
 */

/*******************************************************************************
 *                              F U N C T I O N S
 *******************************************************************************
 */

#if 0
/*----------------------------------------------------------------------------*/
/*!
 * \brief Override the implementation of select queue
 *
 * \param[in] dev Pointer to struct net_device
 * \param[in] skb Pointer to struct skb_buff
 *
 * \return (none)
 */
/*----------------------------------------------------------------------------*/
unsigned int _cfg80211_classify8021d(struct sk_buff *skb)
{
	unsigned int dscp = 0;

	/* skb->priority values from 256->263 are magic values
	 * directly indicate a specific 802.1d priority.  This is
	 * to allow 802.1d priority to be passed directly in from
	 * tags
	 */

	if (skb->priority >= 256 && skb->priority <= 263)
		return skb->priority - 256;
	switch (skb->protocol) {
	case htons(ETH_P_IP):
		dscp = ip_hdr(skb)->tos & 0xfc;
		break;
	}
	return dscp >> 5;
}
#endif

#if KERNEL_VERSION(5, 2, 0) <= LINUX_VERSION_CODE
u16 wlanSelectQueue(struct net_device *dev,
		    struct sk_buff *skb,
		    struct net_device *sb_dev)
{
	return mtk_wlan_ndev_select_queue(skb);
}
#elif KERNEL_VERSION(4, 19, 0) <= LINUX_VERSION_CODE
u16 wlanSelectQueue(struct net_device *dev,
		    struct sk_buff *skb,
		    struct net_device *sb_dev,
		    select_queue_fallback_t fallback)
{
	return mtk_wlan_ndev_select_queue(skb);
}
#elif KERNEL_VERSION(3, 14, 0) <= LINUX_VERSION_CODE
u16 wlanSelectQueue(struct net_device *dev,
		    struct sk_buff *skb,
		    void *accel_priv, select_queue_fallback_t fallback)
{
	return mtk_wlan_ndev_select_queue(skb);
}
#elif KERNEL_VERSION(3, 13, 0) <= LINUX_VERSION_CODE
u16 wlanSelectQueue(struct net_device *dev,
		    struct sk_buff *skb,
		    void *accel_priv)
{
	return mtk_wlan_ndev_select_queue(skb);
}
#else
u16 wlanSelectQueue(struct net_device *dev,
		    struct sk_buff *skb)
{
	return mtk_wlan_ndev_select_queue(skb);
}
#endif

/*----------------------------------------------------------------------------*/
/*!
 * \brief Load NVRAM data and translate it into REG_INFO_T
 *
 * \param[in]  prGlueInfo Pointer to struct GLUE_INFO_T
 * \param[out] prRegInfo  Pointer to struct REG_INFO_T
 *
 * \return (none)
 */
/*----------------------------------------------------------------------------*/
static void glLoadNvram(struct GLUE_INFO *prGlueInfo,
	struct REG_INFO *prRegInfo)
{
	struct WIFI_CFG_PARAM_STRUCT *prNvramSettings;

	ASSERT(prRegInfo);
	ASSERT(prGlueInfo);

	DBGLOG(INIT, INFO, "g_NvramFsm = %d\n", g_NvramFsm);
	if (g_NvramFsm != NVRAM_STATE_READY) {
		DBGLOG(INIT, WARN, "Nvram not available\n");
		return;
	}

	if (sizeof(struct WIFI_CFG_PARAM_STRUCT) >
					sizeof(g_aucNvram)) {
		DBGLOG(INIT, ERROR,
		"Size WIFI_CFG_PARAM_STRUCT %zu > size aucNvram %zu\n"
		, sizeof(struct WIFI_CFG_PARAM_STRUCT),
		sizeof(g_aucNvram));
		return;
	}

	prGlueInfo->fgNvramAvailable = TRUE;

	prRegInfo->prNvramSettings =
		(struct WIFI_CFG_PARAM_STRUCT *)&g_aucNvram[0];
	prNvramSettings = prRegInfo->prNvramSettings;

#if CFG_TC1_FEATURE
		TC1_FAC_NAME(FacReadWifiMacAddr)(prRegInfo->aucMacAddr);
		DBGLOG(INIT, INFO,
			"MAC address: " MACSTR, MAC2STR(prRegInfo->aucMacAddr));
#else
	/* load MAC Address */
	kalMemCopy(prRegInfo->aucMacAddr,
			prNvramSettings->aucMacAddress,
			PARAM_MAC_ADDR_LEN*sizeof(uint8_t));
#endif
		/* load country code */
		/* cast to wide characters */
		prRegInfo->au2CountryCode[0] =
			(uint16_t) prNvramSettings->aucCountryCode[0];
		prRegInfo->au2CountryCode[1] =
			(uint16_t) prNvramSettings->aucCountryCode[1];

	prRegInfo->ucSupport5GBand =
			prNvramSettings->ucSupport5GBand;

	prRegInfo->ucEnable5GBand = prNvramSettings->ucEnable5GBand;

	/* load regulation subbands */
	prRegInfo->eRegChannelListMap = 0;
	prRegInfo->ucRegChannelListIndex = 0;

	if (prRegInfo->eRegChannelListMap == REG_CH_MAP_CUSTOMIZED) {
		kalMemCopy(prRegInfo->rDomainInfo.rSubBand,
			prNvramSettings->aucRegSubbandInfo,
			MAX_SUBBAND_NUM*sizeof(uint8_t));
	}

	log_dbg(INIT, INFO, "u2Part1OwnVersion = %08x, u2Part1PeerVersion = %08x\n",
				 prNvramSettings->u2Part1OwnVersion,
				 prNvramSettings->u2Part1PeerVersion);
}

static void glTaskletResInit(struct GLUE_INFO *prGlueInfo)
{
#if defined(_HIF_PCIE) || defined(_HIF_AXI)
	prGlueInfo->u4TxMsduRetFifoLen = CFG_TX_MAX_PKT_NUM * sizeof(void *);
	prGlueInfo->prTxMsduRetFifoBuf = kalMemAlloc(
		prGlueInfo->u4TxMsduRetFifoLen,
		PHY_MEM_TYPE);

	if (prGlueInfo->prTxMsduRetFifoBuf) {
		KAL_FIFO_INIT(&prGlueInfo->rTxMsduRetFifo,
			prGlueInfo->prTxMsduRetFifoBuf,
			prGlueInfo->u4TxMsduRetFifoLen);
	} else {
		DBGLOG(INIT, ERROR,
				"Cannot alloc buf(%d) for TxMsduRetFifo\n",
				prGlueInfo->u4TxMsduRetFifoLen);
		prGlueInfo->u4TxMsduRetFifoLen = 0;
	}
#endif
}
static void glTaskletResUninit(struct GLUE_INFO *prGlueInfo)
{
#if defined(_HIF_PCIE) || defined(_HIF_AXI)
	if (prGlueInfo->prTxMsduRetFifoBuf) {
		struct MSDU_INFO *prMsduInfo;

		/* Return pending MSDU */
		while (KAL_FIFO_OUT(&prGlueInfo->rTxMsduRetFifo, prMsduInfo)) {
			if (!prMsduInfo) {
				DBGLOG(RX, ERROR, "prMsduInfo null\n");
				break;
			}
			nicTxReturnMsduInfo(prGlueInfo->prAdapter, prMsduInfo);
		}
		kalMemFree(prGlueInfo->prTxMsduRetFifoBuf, PHY_MEM_TYPE,
			prGlueInfo->u4TxMsduRetFifoLen);
		prGlueInfo->prTxMsduRetFifoBuf = NULL;
		prGlueInfo->u4TxMsduRetFifoLen = 0;
	}
#endif
}

static void glTaskletInit(struct GLUE_INFO *prGlueInfo)
{
	glTaskletResInit(prGlueInfo);

	tasklet_init(&prGlueInfo->rRxTask, halRxTasklet,
			(unsigned long)prGlueInfo);

#if (CFG_SUPPORT_RETURN_TASK == 1)
	tasklet_init(&prGlueInfo->rRxRfbRetTask,
		wlanReturnPacketDelaySetupTasklet,
		(unsigned long)prGlueInfo);
#endif

#if defined(_HIF_PCIE) || defined(_HIF_AXI)
	tasklet_init(&prGlueInfo->rTxMsduRetTask,
			halWpdmaFreeMsduTasklet,
			(unsigned long)prGlueInfo);
#endif

	tasklet_init(&prGlueInfo->rTxCompleteTask,
			halTxCompleteTasklet,
			(unsigned long)prGlueInfo);
}

static void glTaskletUninit(struct GLUE_INFO *prGlueInfo)
{
	tasklet_kill(&prGlueInfo->rTxCompleteTask);

#if defined(_HIF_PCIE) || defined(_HIF_AXI)
	tasklet_kill(&prGlueInfo->rTxMsduRetTask);
#endif

#if (CFG_SUPPORT_RETURN_TASK == 1)
	tasklet_kill(&prGlueInfo->rRxRfbRetTask);
#endif

	tasklet_kill(&prGlueInfo->rRxTask);

	glTaskletResUninit(prGlueInfo);
}

static void wlanFreeNetDev(void)
{
	uint32_t u4Idx = 0;

	for (u4Idx = 0; u4Idx < KAL_AIS_NUM; u4Idx++) {
		if (gprWdev[u4Idx] && gprWdev[u4Idx]->netdev) {
			DBGLOG(INIT, INFO, "free_netdev wlan%d netdev start.\n",
					u4Idx);
			free_netdev(gprWdev[u4Idx]->netdev);
			DBGLOG(INIT, INFO, "free_netdev wlan%d netdev end.\n",
					u4Idx);
		}
	}
}


/*----------------------------------------------------------------------------*/
/*!
 * \brief Release prDev from wlandev_array and free tasklet object related to
 *	  it.
 *
 * \param[in] prDev  Pointer to struct net_device
 *
 * \return (none)
 */
/*----------------------------------------------------------------------------*/
static void wlanClearDevIdx(struct net_device *prDev)
{
	int i;

	ASSERT(prDev);

	for (i = 0; i < CFG_MAX_WLAN_DEVICES; i++) {
		if (arWlanDevInfo[i].prDev == prDev) {
			arWlanDevInfo[i].prDev = NULL;
			u4WlanDevNum--;
		}
	}

}				/* end of wlanClearDevIdx() */

/*----------------------------------------------------------------------------*/
/*!
 * \brief Allocate an unique interface index, net_device::ifindex member for
 *	  this wlan device. Store the net_device in wlandev_array, and
 *	  initialize tasklet object related to it.
 *
 * \param[in] prDev  Pointer to struct net_device
 *
 * \retval >= 0      The device number.
 * \retval -1        Fail to get index.
 */
/*----------------------------------------------------------------------------*/
static int wlanGetDevIdx(struct net_device *prDev)
{
	int i;

	ASSERT(prDev);

	for (i = 0; i < CFG_MAX_WLAN_DEVICES; i++) {
		if (arWlanDevInfo[i].prDev == (struct net_device *)NULL) {
			/* Reserve 2 bytes space to store one digit of
			 * device number and NULL terminator.
			 */
			arWlanDevInfo[i].prDev = prDev;
			u4WlanDevNum++;
			return i;
		}
#if CFG_SUPPORT_PERSIST_NETDEV
		else if (arWlanDevInfo[i].prDev == prDev)
			return i;
#endif
	}

	return -1;
}				/* end of wlanGetDevIdx() */

/*----------------------------------------------------------------------------*/
/*!
 * \brief A method of struct net_device, a primary SOCKET interface to configure
 *        the interface lively. Handle an ioctl call on one of our devices.
 *        Everything Linux ioctl specific is done here. Then we pass the
 *	  contents of the ifr->data to the request message handler.
 *
 * \param[in] prDev      Linux kernel netdevice
 *
 * \param[in] prIFReq    Our private ioctl request structure, typed for the
 *			 generic
 *                       struct ifreq so we can use ptr to function
 *
 * \param[in] cmd        Command ID
 *
 * \retval WLAN_STATUS_SUCCESS The IOCTL command is executed successfully.
 * \retval OTHER The execution of IOCTL command is failed.
 */
/*----------------------------------------------------------------------------*/
int wlanDoIOCTL(struct net_device *prDev,
		struct ifreq *prIfReq, int i4Cmd)
{
	struct GLUE_INFO *prGlueInfo = NULL;
	int ret = 0;

	/* Verify input parameters for the following functions */
	ASSERT(prDev && prIfReq);
	if (!prDev || !prIfReq) {
		DBGLOG(INIT, ERROR, "Invalid input data\n");
		return -EINVAL;
	}

	prGlueInfo = *((struct GLUE_INFO **) netdev_priv(prDev));
	if (!prGlueInfo) {
		DBGLOG(INIT, ERROR, "prGlueInfo is NULL\n");
		return -EFAULT;
	}

	if (prGlueInfo->u4ReadyFlag == 0) {
		DBGLOG(INIT, ERROR, "Adapter is not ready\n");
		return -EINVAL;
	}

	if ((i4Cmd >= SIOCIWFIRST) && (i4Cmd < SIOCIWFIRSTPRIV)) {
		/* 0x8B00 ~ 0x8BDF, wireless extension region */
		ret = wext_support_ioctl(prDev, prIfReq, i4Cmd);
	} else if ((i4Cmd >= SIOCIWFIRSTPRIV)
		   && (i4Cmd < SIOCIWLASTPRIV)) {
		/* 0x8BE0 ~ 0x8BFF, private ioctl region */
		ret = priv_support_ioctl(prDev, prIfReq, i4Cmd);
	} else if (i4Cmd == SIOCDEVPRIVATE + 1) {
#ifdef CFG_ANDROID_AOSP_PRIV_CMD
		ret = android_private_support_driver_cmd(prDev, prIfReq, i4Cmd);
#else
		ret = priv_support_driver_cmd(prDev, prIfReq, i4Cmd);
#endif /* CFG_ANDROID_AOSP_PRIV_CMD */
	} else {
		DBGLOG(INIT, WARN, "Unexpected ioctl command: 0x%04x\n",
		       i4Cmd);
		ret = -EOPNOTSUPP;
	}

	return ret;
}				/* end of wlanDoIOCTL() */

/*----------------------------------------------------------------------------*/
/*!
 * \brief Export wlan GLUE_INFO_T pointer to p2p module
 *
 * \param[in]  prGlueInfo Pointer to struct GLUE_INFO_T
 *
 * \return TRUE: get GlueInfo pointer successfully
 *            FALSE: wlan is not started yet
 */
/*---------------------------------------------------------------------------*/
struct GLUE_INFO *wlanGetGlueInfo(void)
{
	struct net_device *prDev = NULL;
	struct GLUE_INFO *prGlueInfo = NULL;

	if (u4WlanDevNum == 0)
		return NULL;

	prDev = arWlanDevInfo[u4WlanDevNum - 1].prDev;
	if (prDev == NULL)
		return NULL;

	prGlueInfo = *((struct GLUE_INFO **) netdev_priv(prDev));

	return prGlueInfo;
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief This function is to set multicast list and set rx mode.
 *
 * \param[in] prDev  Pointer to struct net_device
 *
 * \return (none)
 */
/*----------------------------------------------------------------------------*/

static struct delayed_work workq;
struct net_device *gPrDev;

static void wlanSetMulticastList(struct net_device *prDev)
{
	struct GLUE_INFO *prGlueInfo;

	if (!prDev)
		return;

	prGlueInfo = *((struct GLUE_INFO **) netdev_priv(prDev));

	if (!prGlueInfo || !prGlueInfo->u4ReadyFlag) {
		DBGLOG(REQ, WARN, "driver is not ready\n");
		return;
	}

	/* Allow to receive all multicast for WOW */
	DBGLOG(INIT, TRACE, "flags: 0x%x\n", prDev->flags);
	prDev->flags |= (IFF_MULTICAST | IFF_ALLMULTI);
	gPrDev = prDev;
	schedule_delayed_work(&workq, 0);
}



/* FIXME: Since we cannot sleep in the wlanSetMulticastList, we arrange
 * another workqueue for sleeping. We don't want to block
 * main_thread, so we can't let tx_thread to do this
 */

static void wlanSetMulticastListWorkQueue(
	struct work_struct *work)
{

	struct GLUE_INFO *prGlueInfo = NULL;
	uint32_t u4PacketFilter = 0;
	uint32_t u4SetInfoLen;
	struct net_device *prDev = gPrDev;
	uint8_t ucBssIndex = 0;
	uint32_t rStatus = WLAN_STATUS_SUCCESS;

	ucBssIndex = wlanGetBssIdx(prDev);
	if (!IS_BSS_INDEX_VALID(ucBssIndex))
		return;

	DBGLOG(INIT, INFO, "ucBssIndex = %d\n", ucBssIndex);

	down(&g_halt_sem);
	if (g_u4HaltFlag) {
		up(&g_halt_sem);
		return;
	}

	prGlueInfo = (prDev != NULL) ? *((struct GLUE_INFO **)
					 netdev_priv(prDev)) : NULL;
	ASSERT(prDev);
	ASSERT(prGlueInfo);
	if (!prDev || !prGlueInfo) {
		DBGLOG(INIT, WARN,
		       "abnormal dev or skb: prDev(0x%p), prGlueInfo(0x%p)\n",
		       prDev, prGlueInfo);
		up(&g_halt_sem);
		return;
	}

	if (!prGlueInfo->u4ReadyFlag) {
		DBGLOG(REQ, WARN, "driver is not ready\n");
		up(&g_halt_sem);
		return;
	}

	DBGLOG(INIT, INFO,
	       "wlanSetMulticastListWorkQueue prDev->flags:0x%x\n",
	       prDev->flags);

	if (prDev->flags & IFF_PROMISC)
		u4PacketFilter |= PARAM_PACKET_FILTER_PROMISCUOUS;

	if (prDev->flags & IFF_BROADCAST)
		u4PacketFilter |= PARAM_PACKET_FILTER_BROADCAST;

	if (prDev->flags & IFF_MULTICAST) {
		if ((prDev->flags & IFF_ALLMULTI)
		    || (netdev_mc_count(prDev) > MAX_NUM_GROUP_ADDR))
			u4PacketFilter |= PARAM_PACKET_FILTER_ALL_MULTICAST;
		else
			u4PacketFilter |= PARAM_PACKET_FILTER_MULTICAST;
	}

	up(&g_halt_sem);

	if (kalIoctl(prGlueInfo,
		     wlanoidSetCurrentPacketFilter,
		     &u4PacketFilter,
		     sizeof(u4PacketFilter), FALSE, FALSE, TRUE,
		     &u4SetInfoLen) != WLAN_STATUS_SUCCESS) {
		return;
	}

	if (u4PacketFilter & PARAM_PACKET_FILTER_MULTICAST) {
		/* Prepare multicast address list */
		struct netdev_hw_addr *ha;
		uint8_t *prMCAddrList = NULL;
		uint32_t i = 0;

		down(&g_halt_sem);
		if (g_u4HaltFlag) {
			up(&g_halt_sem);
			return;
		}

		prMCAddrList = kalMemAlloc(MAX_NUM_GROUP_ADDR * ETH_ALEN,
					   VIR_MEM_TYPE);
		if (!prMCAddrList) {
			DBGLOG(INIT, WARN, "prMCAddrList memory alloc fail!\n");
			up(&g_halt_sem);
			return;
		}

		/* Avoid race condition with kernel net subsystem */
		netif_addr_lock_bh(prDev);

		netdev_for_each_mc_addr(ha, prDev) {
			if (i < MAX_NUM_GROUP_ADDR) {
				kalMemCopy((prMCAddrList + i * ETH_ALEN),
					   GET_ADDR(ha), ETH_ALEN);
				DBGLOG(INIT, LOUD, "%u MAC: "MACSTR"\n",
					i, MAC2STR(GET_ADDR(ha)));
				i++;
			}
		}

		netif_addr_unlock_bh(prDev);

		up(&g_halt_sem);

		rStatus = kalIoctlByBssIdx(prGlueInfo,
			 wlanoidSetMulticastList, prMCAddrList, (i * ETH_ALEN),
			 FALSE, FALSE, TRUE, &u4SetInfoLen,
			 ucBssIndex);

		kalMemFree(prMCAddrList, VIR_MEM_TYPE,
			   MAX_NUM_GROUP_ADDR * ETH_ALEN);
	} else if (u4PacketFilter & PARAM_PACKET_FILTER_ALL_MULTICAST) {
		DBGLOG(INIT, INFO,
			"Clear previous MAR settings to rx all mc pkt\n");
		kalIoctlByBssIdx(prGlueInfo,
			 wlanoidSetMulticastList, NULL, 0,
			 FALSE, FALSE, TRUE, &u4SetInfoLen,
			 ucBssIndex);
	}

}				/* end of wlanSetMulticastList() */

/*----------------------------------------------------------------------------*/
/*!
 * \brief    To indicate scheduled scan has been stopped
 *
 * \param[in]
 *           prGlueInfo
 *
 * \return
 *           None
 */
/*----------------------------------------------------------------------------*/
void wlanSchedScanStoppedWorkQueue(struct work_struct *work)
{
	struct GLUE_INFO *prGlueInfo = NULL;
	struct net_device *prDev = gPrDev;

	DBGLOG(SCN, INFO, "wlanSchedScanStoppedWorkQueue\n");
	prGlueInfo = (prDev != NULL) ? *((struct GLUE_INFO **)
					 netdev_priv(prDev)) : NULL;
	if (!prGlueInfo) {
		DBGLOG(SCN, INFO, "prGlueInfo == NULL unexpected\n");
		return;
	}

	/* 2. indication to cfg80211 */
	/* 20150205 change cfg80211_sched_scan_stopped to work queue due to
	 * sched_scan_mtx dead lock issue
	 */
#if KERNEL_VERSION(4, 12, 0) <= CFG80211_VERSION_CODE
	cfg80211_sched_scan_stopped(priv_to_wiphy(prGlueInfo), 0);
#else
	cfg80211_sched_scan_stopped(priv_to_wiphy(prGlueInfo));
#endif
	DBGLOG(SCN, INFO,
	       "cfg80211_sched_scan_stopped event send done WorkQueue thread return from wlanSchedScanStoppedWorkQueue\n");
	return;

}

/* FIXME: Since we cannot sleep in the wlanSetMulticastList, we arrange
 * another workqueue for sleeping. We don't want to block
 * main_thread, so we can't let tx_thread to do this
 */

void p2pSetMulticastListWorkQueueWrapper(struct GLUE_INFO
		*prGlueInfo)
{


	if (!prGlueInfo) {
		DBGLOG(INIT, WARN,
		       "abnormal dev or skb: prGlueInfo(0x%p)\n", prGlueInfo);
		return;
	}
#if CFG_ENABLE_WIFI_DIRECT
	if (prGlueInfo->prAdapter->fgIsP2PRegistered)
		mtk_p2p_wext_set_Multicastlist(prGlueInfo);
#endif

} /* end of p2pSetMulticastListWorkQueueWrapper() */

/*----------------------------------------------------------------------------*/
/*
 * \brief This function is TX entry point of NET DEVICE.
 *
 * \param[in] prSkb  Pointer of the sk_buff to be sent
 * \param[in] prDev  Pointer to struct net_device
 *
 * \retval NETDEV_TX_OK - on success.
 * \retval NETDEV_TX_BUSY - on failure, packet will be discarded by upper layer.
 */
/*----------------------------------------------------------------------------*/
netdev_tx_t wlanHardStartXmit(struct sk_buff *prSkb,
		      struct net_device *prDev)
{
	struct NETDEV_PRIVATE_GLUE_INFO *prNetDevPrivate =
		(struct NETDEV_PRIVATE_GLUE_INFO *) NULL;
	struct GLUE_INFO *prGlueInfo = *((struct GLUE_INFO **)
					 netdev_priv(prDev));
	uint8_t ucBssIndex;

	ASSERT(prSkb);
	ASSERT(prDev);
	ASSERT(prGlueInfo);

	prNetDevPrivate = (struct NETDEV_PRIVATE_GLUE_INFO *)
			  netdev_priv(prDev);
	ASSERT(prNetDevPrivate->prGlueInfo == prGlueInfo);
	ucBssIndex = prNetDevPrivate->ucBssIdx;

#if CFG_SUPPORT_PASSPOINT
	if (prGlueInfo->fgIsDad) {
		/* kalPrint("[Passpoint R2] Due to ipv4_dad...TX is forbidden\n"
		 *         );
		 */
		dev_kfree_skb(prSkb);
		return NETDEV_TX_OK;
	}
	if (prGlueInfo->fgIs6Dad) {
		/* kalPrint("[Passpoint R2] Due to ipv6_dad...TX is forbidden\n"
		 *          );
		 */
		dev_kfree_skb(prSkb);
		return NETDEV_TX_OK;
	}
#endif /* CFG_SUPPORT_PASSPOINT */

#if CFG_CHIP_RESET_SUPPORT
	if (!wlanIsDriverReady(prGlueInfo,
		(WLAN_DRV_READY_CHCECK_RESET | WLAN_DRV_READY_CHCECK_WLAN_ON))) {
		DBGLOG(INIT, WARN,
		"u4ReadyFlag:%u, kalIsResetting():%d, dropping the packet\n",
		prGlueInfo->u4ReadyFlag, kalIsResetting());
		dev_kfree_skb(prSkb);
		return NETDEV_TX_OK;
	}
#endif
	kalResetPacket(prGlueInfo, (void *) prSkb);

	STATS_TX_TIME_ARRIVE(prSkb);

#if (CFG_SUPPORT_TX_TSO_SW == 1)
	if (kalSwTsoXmit(&prSkb, prDev, prGlueInfo, ucBssIndex)
		== WLAN_STATUS_SUCCESS)
		return NETDEV_TX_OK;
#endif

#if (CFG_SUPPORT_TX_SG == 1) && (defined(_HIF_PCIE) || defined(_HIF_AXI)) && \
	(CFG_HIF_TX_PREALLOC_DATA_BUFFER == 0)
	if (skb_is_nonlinear(prSkb) &&
	    skb_shinfo(prSkb)->nr_frags > HIF_TX_MAX_FRAG &&
	    __skb_linearize(prSkb)) {
		DBGLOG(INIT, WARN,
			"unable handle skb with frags:%d\n",
			skb_shinfo(prSkb)->nr_frags);
		dev_kfree_skb(prSkb);
		return NETDEV_TX_OK;
	}
#endif

	if (kalHardStartXmit(prSkb, prDev, prGlueInfo,
			     ucBssIndex) == WLAN_STATUS_SUCCESS) {
		/* Successfully enqueue to Tx queue */
		/* Successfully enqueue to Tx queue */
#if (CFG_SUPPORT_PERMON == 1)
		if (netif_carrier_ok(prDev))
			kalPerMonStart(prGlueInfo);
#endif
	}

	/* For Linux, we'll always return OK FLAG, because we'll free this skb
	 * by ourself
	 */
	return NETDEV_TX_OK;
}				/* end of wlanHardStartXmit() */

/*----------------------------------------------------------------------------*/
/*!
 * \brief A method of struct net_device, to get the network interface
 *        statistical information.
 *
 * Whenever an application needs to get statistics for the interface, this
 * method is called. This happens, for example, when ifconfig or netstat -i
 * is run.
 *
 * \param[in] prDev      Pointer to struct net_device.
 *
 * \return net_device_stats buffer pointer.
 */
/*----------------------------------------------------------------------------*/
struct net_device_stats *wlanGetStats(IN struct net_device
				      *prDev)
{
	return (struct net_device_stats *)kalGetStats(prDev);
}				/* end of wlanGetStats() */

void wlanDebugInit(void)
{
	/* Set the initial debug level of each module */
#if DBG
	/* enable all */
	wlanSetDriverDbgLevel(DBG_ALL_MODULE_IDX, DBG_CLASS_MASK);
#else
#ifdef CFG_DEFAULT_DBG_LEVEL
	wlanSetDriverDbgLevel(DBG_ALL_MODULE_IDX,
			      CFG_DEFAULT_DBG_LEVEL);
#else
	wlanSetDriverDbgLevel(DBG_ALL_MODULE_IDX,
			      DBG_LOG_LEVEL_DEFAULT);
#endif
#endif /* DBG */

	LOG_FUNC("Reset ALL DBG module log level to DEFAULT!");

}

/*----------------------------------------------------------------------------*/
/*!
 * \brief A function for prDev->init
 *
 * \param[in] prDev      Pointer to struct net_device.
 *
 * \retval 0         The execution of wlanInit succeeds.
 * \retval -ENXIO    No such device.
 */
/*----------------------------------------------------------------------------*/
static int wlanInit(struct net_device *prDev)
{
	struct GLUE_INFO *prGlueInfo = NULL;

	if (!prDev)
		return -ENXIO;

	prGlueInfo = *((struct GLUE_INFO **) netdev_priv(prDev));
	INIT_DELAYED_WORK(&workq, wlanSetMulticastListWorkQueue);

	/* 20150205 work queue for sched_scan */
	INIT_DELAYED_WORK(&sched_workq,
			  wlanSchedScanStoppedWorkQueue);

#if CFG_REDIRECT_OID_SUPPORT
	INIT_DELAYED_WORK(&oid_workq,
				wlanSchedOidWorkQueue);
#endif

#if (CFG_SUPPORT_SUPPLICANT_SME == 1)
#if CFG_SUPPORT_CFG80211_QUEUE
	INIT_DELAYED_WORK(&cfg80211_workq,
				wlanSchedCfg80211WorkQueue);
#endif
#endif

	/* 20161024 init wow port setting */
#if CFG_WOW_SUPPORT
	kalWowInit(prGlueInfo);
#endif

#if CFG_SUPPORT_RX_GRO
	spin_lock_init(&prGlueInfo->napi_spinlock);
	kalNapiInit(prDev);
	kalNapiRxDirectInit(prDev);
#endif
	return 0;		/* success */
}				/* end of wlanInit() */

/*----------------------------------------------------------------------------*/
/*!
 * \brief A function for prDev->uninit
 *
 * \param[in] prDev      Pointer to struct net_device.
 *
 * \return (none)
 */
/*----------------------------------------------------------------------------*/
static void wlanUninit(struct net_device *prDev)
{
}				/* end of wlanUninit() */

/*----------------------------------------------------------------------------*/
/*!
 * \brief A method of struct net_device, to set the randomized mac address
 *
 * This method is called before Wifi Framework requests a new conenction with
 * enabled feature "Connected Random Mac".
 *
 * \param[in] ndev	Pointer to struct net_device.
 * \param[in] addr	Randomized Mac address passed from WIFI framework.
 *
 * \return int.
 */
/*----------------------------------------------------------------------------*/
static int wlanSetMacAddress(struct net_device *ndev, void *addr)
{
	struct ADAPTER *prAdapter = NULL;
	struct GLUE_INFO *prGlueInfo = NULL;
	struct sockaddr *sa = NULL;
	struct BSS_INFO *prAisBssInfo = NULL;

	/**********************************************************************
	 * Check if kernel passes valid data to us                            *
	 **********************************************************************
	 */
	if (!ndev || !addr) {
		DBGLOG(INIT, ERROR, "Set macaddr with ndev(%d) and addr(%d)\n",
		       (ndev == NULL) ? 0 : 1, (addr == NULL) ? 0 : 1);
		return -EINVAL;
	}

	/**********************************************************************
	 * 1. Change OwnMacAddr which will be updated to FW through           *
	 *    rlmActivateNetwork later.                                       *
	 * 2. Change dev_addr stored in kernel to notify framework that the   *
	 *    mac addr has been changed and what the new value is.            *
	 **********************************************************************
	 */
	sa = (struct sockaddr *)addr;
	prGlueInfo = *((struct GLUE_INFO **) netdev_priv(ndev));
	prAdapter = prGlueInfo->prAdapter;

	prAisBssInfo = aisGetAisBssInfo(prAdapter,
		wlanGetBssIdx(ndev));

	COPY_MAC_ADDR(prAisBssInfo->aucOwnMacAddr, sa->sa_data);
	kal_eth_hw_addr_set(ndev, sa->sa_data);
	DBGLOG(INIT, INFO,
		"[wlan%d] Set connect random macaddr to " MACSTR ".\n",
		prAisBssInfo->ucBssIndex,
		MAC2STR(prAisBssInfo->aucOwnMacAddr));

	return WLAN_STATUS_SUCCESS;
}				/* end of wlanSetMacAddr() */

/*----------------------------------------------------------------------------*/
/*!
 * \brief A function for prDev->open
 *
 * \param[in] prDev      Pointer to struct net_device.
 *
 * \retval 0     The execution of wlanOpen succeeds.
 * \retval < 0   The execution of wlanOpen failed.
 */
/*----------------------------------------------------------------------------*/
static int wlanOpen(struct net_device *prDev)
{
	ASSERT(prDev);

#if CFG_SUPPORT_RX_GRO
	kalNapiEnable(prDev);
#endif

	netif_tx_start_all_queues(prDev);

	return 0;		/* success */
}				/* end of wlanOpen() */

/*----------------------------------------------------------------------------*/
/*!
 * \brief A function for prDev->stop
 *
 * \param[in] prDev      Pointer to struct net_device.
 *
 * \retval 0     The execution of wlanStop succeeds.
 * \retval < 0   The execution of wlanStop failed.
 */
/*----------------------------------------------------------------------------*/
static int wlanStop(struct net_device *prDev)
{
	struct GLUE_INFO *prGlueInfo = NULL;

	GLUE_SPIN_LOCK_DECLARATION();

	ASSERT(prDev);

	prGlueInfo = *((struct GLUE_INFO **) netdev_priv(prDev));

	/* CFG80211 down */
	GLUE_ACQUIRE_SPIN_LOCK(prGlueInfo, SPIN_LOCK_NET_DEV);
	if (prGlueInfo->prScanRequest) {
		kalCfg80211ScanDone(prGlueInfo->prScanRequest, TRUE);
		prGlueInfo->prScanRequest = NULL;
	}
	GLUE_RELEASE_SPIN_LOCK(prGlueInfo, SPIN_LOCK_NET_DEV);

	netif_tx_stop_all_queues(prDev);

#if CFG_SUPPORT_RX_GRO
	kalNapiRxDirectUninit(prDev);
	kalNapiDisable(prDev);
#endif

	return 0;		/* success */
}				/* end of wlanStop() */

/*----------------------------------------------------------------------------*/
/*!
 * \brief Update Channel table for cfg80211 for Wi-Fi Direct based on current
 *        country code
 *
 * \param[in] prGlueInfo      Pointer to glue info
 *
 * \return   none
 */
/*----------------------------------------------------------------------------*/
void wlanUpdateChannelTable(struct GLUE_INFO *prGlueInfo)
{
	uint8_t i, j;
	uint8_t ucNumOfChannel;
	uint32_t u4BandIdx;

	struct RF_CHANNEL_INFO aucChannelList[MAX_PER_BAND_CHN_NUM];

	kalMemZero(aucChannelList,
		sizeof(struct RF_CHANNEL_INFO) * MAX_PER_BAND_CHN_NUM);

	/* 1. Disable all channels */
	for (i = 0; i < ARRAY_SIZE(mtk_2ghz_channels); i++) {
		mtk_2ghz_channels[i].flags |= IEEE80211_CHAN_DISABLED;
		mtk_2ghz_channels[i].orig_flags |= IEEE80211_CHAN_DISABLED;
	}

	for (i = 0; i < ARRAY_SIZE(mtk_5ghz_channels); i++) {
		mtk_5ghz_channels[i].flags |= IEEE80211_CHAN_DISABLED;
		mtk_5ghz_channels[i].orig_flags |= IEEE80211_CHAN_DISABLED;
	}

#if (CFG_SUPPORT_WIFI_6G == 1)
	for (i = 0; i < ARRAY_SIZE(mtk_6ghz_channels); i++) {
		mtk_6ghz_channels[i].flags |= IEEE80211_CHAN_DISABLED;
		mtk_6ghz_channels[i].orig_flags |= IEEE80211_CHAN_DISABLED;
	}
#endif

	for (u4BandIdx = BAND_2G4; u4BandIdx < BAND_NUM; u4BandIdx++) {
		/* 2. Get current domain channel list */
		rlmDomainGetChnlList(prGlueInfo->prAdapter,
				     u4BandIdx, FALSE,
				     MAX_PER_BAND_CHN_NUM,
				     &ucNumOfChannel, aucChannelList);

		/* 3. Enable specific channel based on domain channel list */
		for (i = 0; i < ucNumOfChannel; i++) {
			switch (aucChannelList[i].eBand) {
			case BAND_2G4:
				for (j = 0; j <
					ARRAY_SIZE(mtk_2ghz_channels); j++) {
					if (mtk_2ghz_channels[j].hw_value ==
					    aucChannelList[i].ucChannelNum) {
						mtk_2ghz_channels[j].flags &=
						~IEEE80211_CHAN_DISABLED;
						mtk_2ghz_channels[j].orig_flags
						&= ~IEEE80211_CHAN_DISABLED;
						break;
					}
				}
				break;

			case BAND_5G:
				for (j = 0; j <
					ARRAY_SIZE(mtk_5ghz_channels); j++) {
					if (mtk_5ghz_channels[j].hw_value ==
					    aucChannelList[i].ucChannelNum) {
						mtk_5ghz_channels[j].flags &=
						~IEEE80211_CHAN_DISABLED;
						mtk_5ghz_channels[j].orig_flags
						&= ~IEEE80211_CHAN_DISABLED;
						mtk_5ghz_channels[j].dfs_state =
						(aucChannelList[i].eDFS != 0) ?
							NL80211_DFS_USABLE :
							NL80211_DFS_UNAVAILABLE;
						break;
					}
				}
				break;

#if (CFG_SUPPORT_WIFI_6G == 1)
			case BAND_6G:
				for (j = 0; j <
					ARRAY_SIZE(mtk_6ghz_channels); j++) {
					if (mtk_6ghz_channels[j].hw_value ==
					aucChannelList[i].ucChannelNum) {
						mtk_6ghz_channels[j].flags &=
						~IEEE80211_CHAN_DISABLED;
						mtk_6ghz_channels[j].orig_flags
						&= ~IEEE80211_CHAN_DISABLED;
						mtk_6ghz_channels[j].dfs_state =
						(aucChannelList[i].eDFS != 0) ?
							NL80211_DFS_USABLE :
							NL80211_DFS_UNAVAILABLE;
						break;
					}
				}
				break;
#endif

			default:
				DBGLOG(INIT, WARN, "Unknown band %d\n",
				       aucChannelList[i].eBand);
				break;
			}
		}
	}
}

#if CFG_SUPPORT_SAP_DFS_CHANNEL
static u_int8_t wlanIsAdjacentChnl(struct GL_P2P_INFO *prGlueP2pInfo,
		uint32_t u4CenterFreq, uint8_t ucBandWidth,
		enum ENUM_CHNL_EXT eBssSCO, uint8_t ucAdjacentChannel,
		enum ENUM_BAND eBand)
{
	uint32_t u4AdjacentFreq = 0;
	uint32_t u4BandWidth;
	uint32_t u4StartFreq, u4EndFreq;
	struct ieee80211_channel *chnl = NULL;

	u4AdjacentFreq = nicChannelNum2Freq(ucAdjacentChannel, eBand) / 1000;

	DBGLOG(INIT, TRACE,
		"p2p: %p, center_freq: %d, bw: %d, sco: %d, ad_freq: %d",
		prGlueP2pInfo, u4CenterFreq, ucBandWidth, eBssSCO,
		u4AdjacentFreq);

	if (!prGlueP2pInfo)
		return FALSE;

	if (ucBandWidth == VHT_OP_CHANNEL_WIDTH_20_40 &&
			eBssSCO == CHNL_EXT_SCN)
		return FALSE;

	if (!u4CenterFreq)
		return FALSE;

	if (!u4AdjacentFreq)
		return FALSE;

	switch (ucBandWidth) {
	case VHT_OP_CHANNEL_WIDTH_20_40:
		u4BandWidth = 40;
		break;
	case VHT_OP_CHANNEL_WIDTH_80:
		u4BandWidth = 80;
		break;
	default:
		DBGLOG(INIT, WARN, "unsupported bandwidth!");
		return FALSE;
	}
	u4StartFreq = u4CenterFreq - u4BandWidth / 2 + 10;
	u4EndFreq = u4CenterFreq + u4BandWidth / 2 - 10;
	DBGLOG(INIT, TRACE, "bw: %d, s_freq: %d, e_freq: %d",
			u4BandWidth, u4StartFreq, u4EndFreq);
	if (u4AdjacentFreq < u4StartFreq || u4AdjacentFreq > u4EndFreq)
		return FALSE;

	/* check valid channel */
	chnl = ieee80211_get_channel(prGlueP2pInfo->prWdev->wiphy,
			u4AdjacentFreq);
	if (!chnl) {
		DBGLOG(INIT, WARN, "invalid channel for freq: %d",
				u4AdjacentFreq);
		return FALSE;
	}
	return TRUE;
}

void wlanUpdateDfsChannelTable(struct GLUE_INFO *prGlueInfo,
		uint8_t ucRoleIdx, uint8_t ucChannel, uint8_t ucBandWidth,
		enum ENUM_CHNL_EXT eBssSCO, uint32_t u4CenterFreq,
		enum ENUM_BAND eBand)
{
	struct GL_P2P_INFO *prGlueP2pInfo = NULL;
	uint8_t i, j;
	uint8_t ucNumOfChannel;
	struct RF_CHANNEL_INFO aucChannelList[
			ARRAY_SIZE(mtk_5ghz_channels)] = {};

	DBGLOG(INIT, INFO, "r: %d, chnl %u, b: %d, s: %d, freq: %d\n",
			ucRoleIdx, ucChannel, ucBandWidth, eBssSCO,
			u4CenterFreq);

	/* 1. Get current domain DFS channel list */
	rlmDomainGetDfsChnls(prGlueInfo->prAdapter,
		ARRAY_SIZE(mtk_5ghz_channels),
		&ucNumOfChannel, aucChannelList);

	if (ucRoleIdx < KAL_P2P_NUM)
		prGlueP2pInfo = prGlueInfo->prP2PInfo[ucRoleIdx];

	/* 2. Enable specific channel based on domain channel list */
	for (i = 0; i < ucNumOfChannel; i++) {
		for (j = 0; j < ARRAY_SIZE(mtk_5ghz_channels); j++) {
			if (aucChannelList[i].ucChannelNum !=
				mtk_5ghz_channels[j].hw_value)
				continue;

			if ((aucChannelList[i].ucChannelNum == ucChannel) ||
				wlanIsAdjacentChnl(prGlueP2pInfo,
					u4CenterFreq,
					ucBandWidth,
					eBssSCO,
					aucChannelList[i].ucChannelNum,
					eBand)) {
				mtk_5ghz_channels[j].dfs_state
					= NL80211_DFS_AVAILABLE;
				mtk_5ghz_channels[j].flags &=
					~IEEE80211_CHAN_RADAR;
				mtk_5ghz_channels[j].orig_flags &=
					~IEEE80211_CHAN_RADAR;
				DBGLOG(INIT, INFO,
					"ch (%d), force NL80211_DFS_AVAILABLE.\n",
					aucChannelList[i].ucChannelNum);
			} else {
				mtk_5ghz_channels[j].dfs_state
					= NL80211_DFS_USABLE;
				mtk_5ghz_channels[j].flags |=
					IEEE80211_CHAN_RADAR;
				mtk_5ghz_channels[j].orig_flags |=
					IEEE80211_CHAN_RADAR;
				DBGLOG(INIT, TRACE,
					"ch (%d), force NL80211_DFS_USABLE.\n",
					aucChannelList[i].ucChannelNum);
			}
		}
	}
}
#endif

/*----------------------------------------------------------------------------*/
/*!
 * \brief Register the device to the kernel and return the index.
 *
 * \param[in] prDev      Pointer to struct net_device.
 *
 * \retval 0     The execution of wlanNetRegister succeeds.
 * \retval < 0   The execution of wlanNetRegister failed.
 */
/*----------------------------------------------------------------------------*/
static int32_t wlanNetRegister(struct wireless_dev *prWdev)
{
	struct GLUE_INFO *prGlueInfo;
	int32_t i4DevIdx = -1;
	struct NETDEV_PRIVATE_GLUE_INFO *prNetDevPrivate =
		(struct NETDEV_PRIVATE_GLUE_INFO *) NULL;
	struct ADAPTER *prAdapter = NULL;
	uint32_t u4Idx = 0;

	ASSERT(prWdev);

	do {
		if (!prWdev)
			break;

		prGlueInfo = (struct GLUE_INFO *) wiphy_priv(prWdev->wiphy);
		prAdapter = prGlueInfo->prAdapter;
		i4DevIdx = wlanGetDevIdx(prWdev->netdev);
		if (i4DevIdx < 0) {
			DBGLOG(INIT, ERROR, "net_device number exceeds!\n");
			break;
		}

		if (prAdapter && prAdapter->rWifiVar.ucWow)
			kalInitDevWakeup(prGlueInfo->prAdapter,
				wiphy_dev(prWdev->wiphy));

		for (u4Idx = 0; u4Idx < KAL_AIS_NUM; u4Idx++) {
			prNetDevPrivate = (struct NETDEV_PRIVATE_GLUE_INFO *)
				netdev_priv(gprWdev[u4Idx]->netdev);
			ASSERT(prNetDevPrivate->prGlueInfo == prGlueInfo);
			prNetDevPrivate->ucBssIdx = u4Idx;
#if CFG_ENABLE_UNIFY_WIPHY
			prNetDevPrivate->ucIsP2p = FALSE;
#endif
			wlanBindBssIdxToNetInterface(prGlueInfo,
				     u4Idx,
				     (void *)gprWdev[u4Idx]->netdev);
#if CFG_SUPPORT_PERSIST_NETDEV
			if (gprNetdev[u4Idx]->reg_state == NETREG_REGISTERED)
				continue;
#endif
			if (register_netdev(gprWdev[u4Idx]->netdev)
				< 0) {
				DBGLOG(INIT, ERROR,
					"Register net_device %d failed\n",
					u4Idx);
				wlanClearDevIdx(
					gprWdev[u4Idx]->netdev);
				i4DevIdx = -1;
			}
		}

		if (i4DevIdx != -1)
			prGlueInfo->fgIsRegistered = TRUE;
	} while (FALSE);

	return i4DevIdx;	/* success */
}				/* end of wlanNetRegister() */


/*----------------------------------------------------------------------------*/
/*!
 * \brief Unregister the device from the kernel
 *
 * \param[in] prWdev      Pointer to struct net_device.
 *
 * \return (none)
 */
/*----------------------------------------------------------------------------*/
static void wlanNetUnregister(struct wireless_dev *prWdev)
{
	struct GLUE_INFO *prGlueInfo;

	if (!prWdev) {
		DBGLOG(INIT, ERROR, "The device context is NULL\n");
		return;
	}

	prGlueInfo = (struct GLUE_INFO *) wiphy_priv(prWdev->wiphy);

#if !CFG_SUPPORT_PERSIST_NETDEV
	{
		uint32_t u4Idx = 0;

		for (u4Idx = 0; u4Idx < KAL_AIS_NUM; u4Idx++) {
			if (gprWdev[u4Idx] && gprWdev[u4Idx]->netdev) {
				wlanClearDevIdx(gprWdev[u4Idx]->netdev);
				unregister_netdev(gprWdev[u4Idx]->netdev);
			}
		}

		prGlueInfo->fgIsRegistered = FALSE;
	}
#endif

}				/* end of wlanNetUnregister() */

static const struct net_device_ops wlan_netdev_ops = {
	.ndo_open = wlanOpen,
	.ndo_stop = wlanStop,
	.ndo_set_rx_mode = wlanSetMulticastList,
	.ndo_get_stats = wlanGetStats,
	.ndo_do_ioctl = wlanDoIOCTL,
	.ndo_start_xmit = wlanHardStartXmit,
	.ndo_init = wlanInit,
	.ndo_uninit = wlanUninit,
	.ndo_select_queue = wlanSelectQueue,
	.ndo_set_mac_address = wlanSetMacAddress,
};

#if CFG_ENABLE_UNIFY_WIPHY
const struct net_device_ops *wlanGetNdevOps(void)
{
	return &wlan_netdev_ops;
}
#endif
void wlanNvramSetState(enum ENUM_NVRAM_STATE state)
{
	g_NvramFsm = state;
}
enum ENUM_NVRAM_STATE wlanNvramGetState(void)
{
	return g_NvramFsm;
}
#if CFG_WLAN_ASSISTANT_NVRAM
static void wlanNvramUpdateOnTestMode(void)
{
	struct GLUE_INFO *prGlueInfo = NULL;
	enum ENUM_NVRAM_STATE nvrmState;
	struct REG_INFO *prRegInfo = NULL;
	struct ADAPTER *prAdapter = NULL;


	/* <1> Sanity Check */
	if ((u4WlanDevNum == 0)
		&& (u4WlanDevNum > CFG_MAX_WLAN_DEVICES)) {
		DBGLOG(INIT, ERROR,
			   "wlanNvramUpdateOnTestMode invalid!!\n");
		return;
	}

	prGlueInfo = wlanGetGlueInfo();
	if (prGlueInfo == NULL) {
		DBGLOG(INIT, ERROR,
			   "prGlueInfo invalid!!\n");
		return;
	}
	prAdapter = prGlueInfo->prAdapter;
	if (prAdapter == NULL) {
		DBGLOG(INIT, ERROR,
			   "prAdapter invalid!!\n");
		return;
	}
	prRegInfo = &prGlueInfo->rRegInfo;
	if (prRegInfo == NULL) {
		DBGLOG(INIT, ERROR,
			   "prRegInfo invalid!!\n");
		return;
	}

	if (prAdapter->fgTestMode == FALSE) {
		DBGLOG(INIT, ERROR,
			   "by-pass on Normal mode\n");
		return;
	}

	nvrmState = wlanNvramGetState();

	if (nvrmState == NVRAM_STATE_READY) {
		DBGLOG(RFTEST, INFO,
		"update nvram to fw on test mode!\n");

		if (kalIsConfigurationExist(prGlueInfo) == TRUE)
			wlanLoadManufactureData(prAdapter, prRegInfo);
		else
			DBGLOG(INIT, WARN, "%s: load manufacture data fail\n",
					   __func__);
	}


}
static uint8_t wlanNvramBufHandler(void *ctx,
			const char *buf,
			uint16_t length)
{

	DBGLOG(INIT, ERROR, "buf = %p, length = %u\n", buf, length);
	if (buf == NULL || length <= 0)
		return -EFAULT;

	if (length > sizeof(g_aucNvram)) {
		DBGLOG(INIT, ERROR, "is over nvrm size %d\n",
			sizeof(g_aucNvram));
		return -EINVAL;
	}

	kalMemZero(&g_aucNvram, sizeof(g_aucNvram));
	if (copy_from_user(g_aucNvram, buf, length)) {
		DBGLOG(INIT, ERROR, "copy nvram fail\n");
		g_NvramFsm = NVRAM_STATE_INIT;
		return -EINVAL;
	}

	g_NvramFsm = NVRAM_STATE_READY;

	/*do nvram update on test mode then driver sent new NVRAM to FW*/
	wlanNvramUpdateOnTestMode();

	return 0;
}

#endif

static void wlanCreateWirelessDevice(void)
{
	struct wiphy *prWiphy = NULL;
	struct wireless_dev *prWdev[KAL_AIS_NUM] = {NULL};
	unsigned int u4SupportSchedScanFlag = 0;
	uint32_t u4Idx = 0;

	/* 4 <1.1> Create wireless_dev */
	for (u4Idx = 0; u4Idx < KAL_AIS_NUM; u4Idx++) {
		prWdev[u4Idx] = kzalloc(sizeof(struct wireless_dev),
			GFP_KERNEL);
		if (!prWdev[u4Idx]) {
			DBGLOG(INIT, ERROR,
				"Allocating memory to wireless_dev context failed\n");
			return;
		}
		prWdev[u4Idx]->iftype = NL80211_IFTYPE_STATION;
	}
	/* 4 <1.2> Create wiphy */
#if CFG_ENABLE_UNIFY_WIPHY
	prWiphy = wiphy_new(&mtk_cfg_ops, sizeof(struct GLUE_INFO));
#else
	prWiphy = wiphy_new(&mtk_wlan_ops,
			    sizeof(struct GLUE_INFO));
#endif

	if (!prWiphy) {
		DBGLOG(INIT, ERROR,
		       "Allocating memory to wiphy device failed\n");
		goto free_wdev;
	}

	/* 4 <1.3> configure wireless_dev & wiphy */
	prWiphy->iface_combinations = p_mtk_iface_combinations_sta;
	prWiphy->n_iface_combinations =
		mtk_iface_combinations_sta_num;
	prWiphy->max_scan_ssids = SCN_SSID_MAX_NUM +
				  1; /* include one wildcard ssid */
	prWiphy->max_scan_ie_len = 512;
#if CFG_SUPPORT_SCHED_SCAN
	prWiphy->max_sched_scan_ssids     =
		CFG_SCAN_HIDDEN_SSID_MAX_NUM;
	prWiphy->max_match_sets           =
		CFG_SCAN_SSID_MATCH_MAX_NUM;
	prWiphy->max_sched_scan_ie_len    = CFG_CFG80211_IE_BUF_LEN;
#if KERNEL_VERSION(4, 12, 0) <= CFG80211_VERSION_CODE
	/* In kernel 4.12 or newer,
	 * this is obsoletes - WIPHY_FLAG_SUPPORTS_SCHED_SCAN
	 */
	prWiphy->max_sched_scan_reqs = 1;
#else
	u4SupportSchedScanFlag            =
		WIPHY_FLAG_SUPPORTS_SCHED_SCAN;
#endif
#endif /* CFG_SUPPORT_SCHED_SCAN */
	prWiphy->interface_modes = BIT(NL80211_IFTYPE_STATION) |
				   BIT(NL80211_IFTYPE_ADHOC);
	prWiphy->bands[KAL_BAND_2GHZ] = &mtk_band_2ghz;
	/* always assign 5Ghz bands here, if the chip is not support 5Ghz,
	 *  bands[KAL_BAND_5GHZ] will be assign to NULL
	 */
	prWiphy->bands[KAL_BAND_5GHZ] = &mtk_band_5ghz;
#if (CFG_SUPPORT_WIFI_6G == 1)
	prWiphy->bands[KAL_BAND_6GHZ] = &mtk_band_6ghz;
	DBGLOG(INIT, INFO, "Support 6G\n");
#endif
	prWiphy->signal_type = CFG80211_SIGNAL_TYPE_MBM;
	prWiphy->cipher_suites = (const u32 *)mtk_cipher_suites;
	prWiphy->n_cipher_suites = ARRAY_SIZE(mtk_cipher_suites);
#if KERNEL_VERSION(5, 4, 0) <= LINUX_VERSION_CODE
	prWiphy->akm_suites = (const u32 *)mtk_akm_suites;
	prWiphy->n_akm_suites = ARRAY_SIZE(mtk_akm_suites);
#endif
	prWiphy->flags = WIPHY_FLAG_HAS_REMAIN_ON_CHANNEL
			| u4SupportSchedScanFlag;

#if (CFG_SUPPORT_ROAMING == 1) && (CFG_SUPPORT_SUPPLICANT_SME == 0)
	prWiphy->flags |= WIPHY_FLAG_SUPPORTS_FW_ROAM;
#endif /* CFG_SUPPORT_ROAMING */

#if KERNEL_VERSION(3, 14, 0) > CFG80211_VERSION_CODE
	prWiphy->flags |= WIPHY_FLAG_CUSTOM_REGULATORY;
#else
	prWiphy->regulatory_flags |= REGULATORY_CUSTOM_REG;
#if (CFG_SUPPORT_DFS_MASTER == 1)
	prWiphy->flags |= WIPHY_FLAG_HAS_CHANNEL_SWITCH;
#if KERNEL_VERSION(3, 16, 0) <= CFG80211_VERSION_CODE
	prWiphy->max_num_csa_counters = 2;
#endif
#endif /* CFG_SUPPORT_DFS_MASTER */
#endif

#if (CFG_SUPPORT_SUPPLICANT_SME == 1)
	prWiphy->features |= NL80211_FEATURE_SAE;
#endif

#if KERNEL_VERSION(3, 14, 0) < CFG80211_VERSION_CODE
	prWiphy->max_ap_assoc_sta = P2P_MAXIMUM_CLIENT_COUNT;
#endif

	cfg80211_regd_set_wiphy(prWiphy);

#if (CFG_SUPPORT_TDLS == 1)
	TDLSEX_WIPHY_FLAGS_INIT(prWiphy->flags);
#endif /* CFG_SUPPORT_TDLS */
	prWiphy->max_remain_on_channel_duration = 5000;
	prWiphy->mgmt_stypes = mtk_cfg80211_ais_default_mgmt_stypes;

#if (CFG_SUPPORT_SCAN_RANDOM_MAC && \
	(KERNEL_VERSION(3, 19, 0) <= CFG80211_VERSION_CODE))
	prWiphy->features |= NL80211_FEATURE_SCAN_RANDOM_MAC_ADDR;
	prWiphy->features |= NL80211_FEATURE_SCHED_SCAN_RANDOM_MAC_ADDR;
#endif

	prWiphy->features |= NL80211_FEATURE_INACTIVITY_TIMER;
#if KERNEL_VERSION(3, 18, 0) <= CFG80211_VERSION_CODE
	prWiphy->vendor_commands = mtk_wlan_vendor_ops;
	prWiphy->n_vendor_commands = sizeof(mtk_wlan_vendor_ops) /
				     sizeof(struct wiphy_vendor_command);
	prWiphy->vendor_events = mtk_wlan_vendor_events;
	prWiphy->n_vendor_events = ARRAY_SIZE(
					   mtk_wlan_vendor_events);
#endif
	/* 4 <1.4> wowlan support */
#ifdef CONFIG_PM
#if KERNEL_VERSION(3, 11, 0) <= CFG80211_VERSION_CODE
	prWiphy->wowlan = &mtk_wlan_wowlan_support;
#else
	kalMemCopy(&prWiphy->wowlan, &mtk_wlan_wowlan_support,
		   sizeof(struct wiphy_wowlan_support));
#endif
#endif

#ifdef CONFIG_CFG80211_WEXT
	/* 4 <1.5> Use wireless extension to replace IOCTL */

#if CFG_ENABLE_UNIFY_WIPHY
	prWiphy->wext = NULL;
#else
	prWiphy->wext = &wext_handler_def;
#endif
#endif
	/* initialize semaphore for halt control */
	sema_init(&g_halt_sem, 1);

#if CFG_ENABLE_UNIFY_WIPHY
	prWiphy->iface_combinations = p_mtk_iface_combinations_p2p;
	prWiphy->n_iface_combinations =
		mtk_iface_combinations_p2p_num;

	prWiphy->interface_modes |= BIT(NL80211_IFTYPE_AP) |
				    BIT(NL80211_IFTYPE_P2P_CLIENT) |
#if (CFG_SUPPORT_SNIFFER_RADIOTAP == 1)
				    BIT(NL80211_IFTYPE_MONITOR) |
#endif
				    BIT(NL80211_IFTYPE_P2P_GO) |
				    BIT(NL80211_IFTYPE_STATION);
	prWiphy->software_iftypes |= BIT(NL80211_IFTYPE_P2P_DEVICE);
	prWiphy->flags |= WIPHY_FLAG_HAS_REMAIN_ON_CHANNEL |
			  WIPHY_FLAG_HAVE_AP_SME;
	prWiphy->ap_sme_capa = 1;
#endif
/*[TODO] remove this CFG when SAE is implemented
* on driver-SME arch and default enabled
*/
#if (CFG_SUPPORT_SUPPLICANT_SME == 1)
	/*Support SAE*/
	prWiphy->features |= NL80211_FEATURE_SAE;
#if CFG_SUPPORT_802_11R
	prWiphy->features |= NL80211_FEATURE_DS_PARAM_SET_IE_IN_PROBES;
	prWiphy->features |= NL80211_FEATURE_QUIET;
	prWiphy->features |= NL80211_FEATURE_TX_POWER_INSERTION;
#endif
#endif
#if CFG_ENABLE_OFFCHANNEL_TX
	prWiphy->flags |= WIPHY_FLAG_OFFCHAN_TX;
#endif /* CFG_ENABLE_OFFCHANNEL_TX */

#if CFG_SUPPORT_RM_BEACON_REPORT_BY_SUPPLICANT
	/* Enable following to indicate supplicant
	 * to support Beacon report feature
	 */
	prWiphy->features |= NL80211_FEATURE_DS_PARAM_SET_IE_IN_PROBES;
	prWiphy->features |= NL80211_FEATURE_QUIET;
#endif

	if (wiphy_register(prWiphy) < 0) {
		DBGLOG(INIT, ERROR, "wiphy_register error\n");
		goto free_wiphy;
	}
	for (u4Idx = 0; u4Idx < KAL_AIS_NUM; u4Idx++) {
		prWdev[u4Idx]->wiphy = prWiphy;
		gprWdev[u4Idx] = prWdev[u4Idx];
	}
#if CFG_WLAN_ASSISTANT_NVRAM
	register_file_buf_handler(wlanNvramBufHandler, (void *)NULL,
			ENUM_BUF_TYPE_NVRAM);
#endif
	DBGLOG(INIT, INFO, "Create wireless device success\n");
	return;

free_wiphy:
	wiphy_free(prWiphy);
free_wdev:
	for (u4Idx = 0; u4Idx < KAL_AIS_NUM; u4Idx++)
		kfree(prWdev[u4Idx]);
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief Destroy all wdev (including the P2P device), and unregister wiphy
 *
 * \param (none)
 *
 * \return (none)
 *
 */
/*----------------------------------------------------------------------------*/
static void wlanDestroyAllWdev(void)
{
#if CFG_ENABLE_UNIFY_WIPHY
	/* There is only one wiphy, avoid that double free the wiphy */
	struct wiphy *wiphy = NULL;
#endif
#if CFG_ENABLE_WIFI_DIRECT
	int i = 0;
#endif

#if CFG_ENABLE_WIFI_DIRECT
	/* free P2P wdev */
	for (i = 0; i < KAL_P2P_NUM; i++) {
		if (gprP2pRoleWdev[i] == NULL)
			continue;
#if CFG_ENABLE_UNIFY_WIPHY
		if (wlanIsAisDev(gprP2pRoleWdev[i]->netdev)) {
			/* This is AIS/AP Interface */
			gprP2pRoleWdev[i] = NULL;
			continue;
		}
#endif
		/* Do wiphy_unregister here. Take care the case that the
		 * gprP2pRoleWdev[i] is created by the cfg80211 add iface ops,
		 * And the base P2P dev is in the gprP2pWdev.
		 * Expect that new created gprP2pRoleWdev[i] is freed in
		 * unregister_netdev/mtk_vif_destructor. And gprP2pRoleWdev[i]
		 * is reset as gprP2pWdev in mtk_vif_destructor.
		 */
		if (gprP2pRoleWdev[i] == gprP2pWdev)
			gprP2pWdev = NULL;

#if CFG_ENABLE_UNIFY_WIPHY
		wiphy = gprP2pRoleWdev[i]->wiphy;
#else
		set_wiphy_dev(gprP2pRoleWdev[i]->wiphy, NULL);
		wiphy_unregister(gprP2pRoleWdev[i]->wiphy);
		wiphy_free(gprP2pRoleWdev[i]->wiphy);
#endif

		kfree(gprP2pRoleWdev[i]);
		gprP2pRoleWdev[i] = NULL;
	}

	if (gprP2pWdev != NULL) {
		/* This case is that gprP2pWdev isn't equal to gprP2pRoleWdev[0]
		 * . The gprP2pRoleWdev[0] is created in the p2p cfg80211 add
		 * iface ops. The two wdev use the same wiphy. Don't double
		 * free the same wiphy.
		 * This part isn't expect occur. Because p2pNetUnregister should
		 * unregister_netdev the new created wdev, and gprP2pRoleWdev[0]
		 * is reset as gprP2pWdev.
		 */
#if CFG_ENABLE_UNIFY_WIPHY
		wiphy = gprP2pWdev->wiphy;
#endif

		kfree(gprP2pWdev);
		gprP2pWdev = NULL;
	}
#endif	/* CFG_ENABLE_WIFI_DIRECT */

	/* free AIS wdev */
	if (gprWdev[0]) {
#if CFG_ENABLE_UNIFY_WIPHY
		wiphy = wlanGetWiphy();
#else
		/* trunk doesn't do set_wiphy_dev, but trunk-ce1 does. */
		/* set_wiphy_dev(gprWdev->wiphy, NULL); */
		wiphy_unregister(gprWdev[0]->wiphy);
		wiphy_free(gprWdev[0]->wiphy);
#endif

		for (i = 0; i < KAL_AIS_NUM; i++) {
			kfree(gprWdev[i]);
			gprWdev[i] = NULL;
		}
	}

#if CFG_ENABLE_UNIFY_WIPHY
	/* unregister & free wiphy */
	if (wiphy) {
		/* set_wiphy_dev(wiphy, NULL): set the wiphy->dev->parent = NULL
		 * The trunk-ce1 does this, but the trunk seems not.
		 */
		/* set_wiphy_dev(wiphy, NULL); */
		wiphy_unregister(wiphy);
		wiphy_free(wiphy);
	}
#endif
}

void wlanWakeLockInit(struct GLUE_INFO *prGlueInfo)
{
#if CFG_ENABLE_WAKE_LOCK
	KAL_WAKE_LOCK_INIT(NULL, prGlueInfo->prIntrWakeLock,
			   "WLAN interrupt");
	KAL_WAKE_LOCK_INIT(NULL, prGlueInfo->prTimeoutWakeLock,
			   "WLAN timeout");
#endif
}

void wlanWakeLockUninit(struct GLUE_INFO *prGlueInfo)
{
#if CFG_ENABLE_WAKE_LOCK
	if (KAL_WAKE_LOCK_ACTIVE(NULL, prGlueInfo->prIntrWakeLock))
		KAL_WAKE_UNLOCK(NULL, prGlueInfo->prIntrWakeLock);
	KAL_WAKE_LOCK_DESTROY(NULL, prGlueInfo->prIntrWakeLock);

	if (KAL_WAKE_LOCK_ACTIVE(NULL,
				 prGlueInfo->prTimeoutWakeLock))
		KAL_WAKE_UNLOCK(NULL, prGlueInfo->prTimeoutWakeLock);
	KAL_WAKE_LOCK_DESTROY(NULL, prGlueInfo->prTimeoutWakeLock);
#endif
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief A method for creating Linux NET4 struct net_device object and the
 *        private data(prGlueInfo and prAdapter). Setup the IO address to the
 *        HIF.
 *        Assign the function pointer to the net_device object
 *
 * \param[in] pvData     Memory address for the device
 *
 * \retval Not null      The wireless_dev object.
 * \retval NULL          Fail to create wireless_dev object
 */
/*----------------------------------------------------------------------------*/
static struct lock_class_key rSpinKey[SPIN_LOCK_NUM];
static struct wireless_dev *wlanNetCreate(void *pvData,
		void *pvDriverData)
{
	struct wireless_dev *prWdev = gprWdev[0];
	struct GLUE_INFO *prGlueInfo = NULL;
	struct ADAPTER *prAdapter = NULL;
	uint32_t i;
	struct device *prDev;
	struct NETDEV_PRIVATE_GLUE_INFO *prNetDevPrivate =
		(struct NETDEV_PRIVATE_GLUE_INFO *) NULL;
	struct mt66xx_chip_info *prChipInfo;
#if CFG_ENABLE_UNIFY_WIPHY
	struct wiphy *prWiphy = NULL;
#endif

	uint8_t *prInfName = NULL;

	if (prWdev == NULL) {
		DBGLOG(INIT, ERROR,
		       "No wireless dev exist, abort power on\n");
		return NULL;
	}

#if CFG_ENABLE_UNIFY_WIPHY
	/* The gprWdev is created at initWlan() and isn't reset when the
	 * disconnection occur. That cause some issue.
	 */
	prWiphy = prWdev->wiphy;
	for (i = 0; i < KAL_AIS_NUM; i++) {
		if (gprWdev[i]
#if CFG_SUPPORT_PERSIST_NETDEV
			&& !gprNetdev[i]
#endif
			) {
			memset(gprWdev[i], 0, sizeof(struct wireless_dev));
			gprWdev[i]->wiphy = prWiphy;
			gprWdev[i]->iftype = NL80211_IFTYPE_STATION;
		}
	}

#if (CFG_SUPPORT_SINGLE_SKU == 1)
	/* XXX: ref from cfg80211_regd_set_wiphy().
	 * The error case: Sometimes after unplug/plug usb, the wlan0 STA can't
	 * scan correctly (FW doesn't do scan). The usb_probe message:
	 * "mtk_reg_notify:(RLM ERROR) Invalid REG state happened. state = 0x6".
	 */
	if (rlmDomainGetCtrlState() == REGD_STATE_INVALID)
		rlmDomainResetCtrlInfo(TRUE);
#endif
#endif	/* CFG_ENABLE_UNIFY_WIPHY */

	/* 4 <1.3> co-relate wiphy & prDev */
	glGetDev(pvData, &prDev);
	if (!prDev)
		DBGLOG(INIT, ERROR, "unable to get struct dev for wlan\n");
	/* Some kernel API (ex: cfg80211_get_drvinfo) will use wiphy_dev().
	 * Without set_wiphy_dev(prWdev->wiphy, prDev), those API will crash.
	 */
	set_wiphy_dev(prWdev->wiphy, prDev);

	/* 4 <2> Create Glue structure */
	prGlueInfo = (struct GLUE_INFO *) wiphy_priv(prWdev->wiphy);
	kalMemZero(prGlueInfo, sizeof(struct GLUE_INFO));

	/* 4 <2.1> Create Adapter structure */
	prAdapter = (struct ADAPTER *) wlanAdapterCreate(
			    prGlueInfo);

	if (!prAdapter) {
		DBGLOG(INIT, ERROR,
		       "Allocating memory to adapter failed\n");
		glClearHifInfo(prGlueInfo);
		return NULL;
	}

	prChipInfo = ((struct mt66xx_hif_driver_data *)
		      pvDriverData)->chip_info;
	prAdapter->chip_info = prChipInfo;
	prGlueInfo->prAdapter = prAdapter;

	/* 4 <3> Initialize Glue structure */
	/* 4 <3.1> Create net device */

#ifdef CFG_DRIVER_INF_NAME_CHANGE

	if (kalStrLen(gprifnamesta) > 0) {
		prInfName = kalStrCat(gprifnamesta, "%d");
		DBGLOG(INIT, WARN, "Station ifname customized, use %s\n",
		       prInfName);
	} else
#endif /* CFG_DRIVER_INF_NAME_CHANGE */
		prInfName = NIC_INF_NAME;

	for (i = 0; i < KAL_AIS_NUM; i++) {
		struct net_device *prDevHandler;

#if CFG_SUPPORT_PERSIST_NETDEV
		if (!gprNetdev[i]) {
			prDevHandler = alloc_netdev_mq(
				sizeof(struct NETDEV_PRIVATE_GLUE_INFO),
				prInfName,
#if KERNEL_VERSION(3, 18, 0) <= CFG80211_VERSION_CODE
				NET_NAME_PREDICTABLE,
#endif
				ether_setup,
				CFG_MAX_TXQ_NUM);
			gprNetdev[i] = prDevHandler;
		} else
			prDevHandler = gprNetdev[i];
#else
		prDevHandler = alloc_netdev_mq(
			sizeof(struct NETDEV_PRIVATE_GLUE_INFO),
			prInfName,
#if KERNEL_VERSION(3, 18, 0) <= CFG80211_VERSION_CODE
			NET_NAME_PREDICTABLE,
#endif
			ether_setup,
			CFG_MAX_TXQ_NUM);
#endif /* end of CFG_SUPPORT_PERSIST_NETDEV */

		if (!prDevHandler) {
			DBGLOG(INIT, ERROR,
				"Allocating memory to net_device context failed\n");
			goto netcreate_err;
		}

		/* Device can help us to save at most 3000 packets,
		 * after we stopped queue
		 */
		prDevHandler->tx_queue_len = 3000;
		DBGLOG(INIT, INFO, "net_device prDev(0x%p) allocated\n",
			prDevHandler);

		/* 4 <3.1.1> Initialize net device varaiables */
		prNetDevPrivate = (struct NETDEV_PRIVATE_GLUE_INFO *)
			netdev_priv(prDevHandler);
		prNetDevPrivate->prGlueInfo = prGlueInfo;

		prDevHandler->needed_headroom =
			NIC_TX_DESC_AND_PADDING_LENGTH +
			prChipInfo->txd_append_size;
		prDevHandler->netdev_ops = &wlan_netdev_ops;
#ifdef CONFIG_WIRELESS_EXT
		prDevHandler->wireless_handlers =
			&wext_handler_def;
#endif
		netif_carrier_off(prDevHandler);
		netif_tx_stop_all_queues(prDevHandler);
		kalResetStats(prDevHandler);

		/* 4 <3.1.2> co-relate with wiphy bi-directionally */
		prDevHandler->ieee80211_ptr = gprWdev[i];

		gprWdev[i]->netdev = prDevHandler;

		/* 4 <3.1.3> co-relate net device & prDev */
		SET_NETDEV_DEV(prDevHandler,
				wiphy_dev(prWdev->wiphy));

		/* 4 <3.1.4> set device to glue */
		prGlueInfo->prDev = prDev;

		/* 4 <3.2> Initialize glue variables */
		kalSetMediaStateIndicated(prGlueInfo,
			MEDIA_STATE_DISCONNECTED,
			i);
	}

	prGlueInfo->prDevHandler = gprWdev[0]->netdev;

	prGlueInfo->ePowerState = ParamDeviceStateD0;
#if !CFG_SUPPORT_PERSIST_NETDEV
	prGlueInfo->fgIsRegistered = FALSE;
#endif
	prGlueInfo->prScanRequest = NULL;
	prGlueInfo->prSchedScanRequest = NULL;


#if CFG_SUPPORT_PASSPOINT
	/* Init DAD */
	prGlueInfo->fgIsDad = FALSE;
	prGlueInfo->fgIs6Dad = FALSE;
	kalMemZero(prGlueInfo->aucDADipv4, 4);
	kalMemZero(prGlueInfo->aucDADipv6, 16);
#endif /* CFG_SUPPORT_PASSPOINT */

	init_completion(&prGlueInfo->rScanComp);
	init_completion(&prGlueInfo->rHaltComp);
	init_completion(&prGlueInfo->rPendComp);

#if CFG_SUPPORT_MULTITHREAD
	init_completion(&prGlueInfo->rHifHaltComp);
	init_completion(&prGlueInfo->rRxHaltComp);
#endif

#if CFG_SUPPORT_NCHO
	init_completion(&prGlueInfo->rAisChGrntComp);
#endif

	/* initialize timer for OID timeout checker */
	kalOsTimerInitialize(prGlueInfo, kalTimeoutHandler);

	for (i = 0; i < SPIN_LOCK_NUM; i++) {
		spin_lock_init(&prGlueInfo->rSpinLock[i]);
		lockdep_set_class(&prGlueInfo->rSpinLock[i], &rSpinKey[i]);
	}

	for (i = 0; i < MUTEX_NUM; i++) {
		mutex_init(&prGlueInfo->arMutex[i]);
		lockdep_set_subclass(&prGlueInfo->arMutex[i], i);
	}

	/* initialize semaphore for ioctl */
	sema_init(&prGlueInfo->ioctl_sem, 1);

#if CFG_SUPPORT_SDIO_READ_WRITE_PATTERN
	/* initialize SDIO read-write pattern control */
	prGlueInfo->fgEnSdioTestPattern = FALSE;
	prGlueInfo->fgIsSdioTestInitialized = FALSE;
#endif

	/* 4 <8> Init Queues */
	init_waitqueue_head(&prGlueInfo->waitq);
	QUEUE_INITIALIZE(&prGlueInfo->rCmdQueue);
	prGlueInfo->i4TxPendingCmdNum = 0;
	QUEUE_INITIALIZE(&prGlueInfo->rTxQueue);
	glSetHifInfo(prGlueInfo, (unsigned long) pvData);

	/* Init wakelock */
	wlanWakeLockInit(prGlueInfo);

	/* main thread is created in this function */
#if CFG_SUPPORT_MULTITHREAD
	init_waitqueue_head(&prGlueInfo->waitq_rx);
	init_waitqueue_head(&prGlueInfo->waitq_hif);

	prGlueInfo->u4TxThreadPid = 0xffffffff;
	prGlueInfo->u4RxThreadPid = 0xffffffff;
	prGlueInfo->u4HifThreadPid = 0xffffffff;
#endif

#if CFG_SUPPORT_CSI
	/* init CSI wait queue  */
	init_waitqueue_head(&(prGlueInfo->prAdapter->rCSIInfo.waitq));
#endif


#if CFG_ASSERT_DUMP
	init_waitqueue_head(&(prGlueInfo->waitq_fwdump));
	skb_queue_head_init(&(prGlueInfo->rFwDumpSkbQueue));
#endif

	return prWdev;

netcreate_err:
	if (prAdapter != NULL) {
		wlanAdapterDestroy(prAdapter);
		prGlueInfo->prAdapter = NULL;
	}
	prGlueInfo->prDevHandler = NULL;

	return prWdev;
}				/* end of wlanNetCreate() */

/*----------------------------------------------------------------------------*/
/*!
 * \brief Destroying the struct net_device object and the private data.
 *
 * \param[in] prWdev      Pointer to struct wireless_dev.
 *
 * \return (none)
 */
/*----------------------------------------------------------------------------*/
static void wlanNetDestroy(struct wireless_dev *prWdev)
{
	struct GLUE_INFO *prGlueInfo = NULL;

	ASSERT(prWdev);

	if (!prWdev) {
		DBGLOG(INIT, ERROR, "The device context is NULL\n");
		return;
	}

	/* prGlueInfo is allocated with net_device */
	prGlueInfo = (struct GLUE_INFO *) wiphy_priv(prWdev->wiphy);
	ASSERT(prGlueInfo);

#if CFG_ASSERT_DUMP
	skb_queue_purge(&(prGlueInfo->rFwDumpSkbQueue));
#endif

	/* prWdev: base AIS dev
	 * Because the interface dev (ex: usb_device) would be free
	 * after un-plug event. Should set the wiphy->dev->parent which
	 * pointer to the interface dev to NULL. Otherwise, the corresponding
	 * system operation (poweroff, suspend) might reference it.
	 * set_wiphy_dev(wiphy, NULL): set the wiphy->dev->parent = NULL
	 * The set_wiphy_dev(prWdev->wiphy, prDev) is done in wlanNetCreate.
	 * But that is after wiphy_register, and will cause exception in
	 * wiphy_unregister(), if do not set_wiphy_dev(wiphy, NULL).
	 */
	set_wiphy_dev(prWdev->wiphy, NULL);

	/* destroy kal OS timer */
	kalCancelTimer(prGlueInfo);

	glClearHifInfo(prGlueInfo);

	wlanAdapterDestroy(prGlueInfo->prAdapter);
	prGlueInfo->prAdapter = NULL;

	/* Free net_device and private data, which are allocated by
	 * alloc_netdev().
	 */
#if CFG_SUPPORT_PERSIST_NETDEV
#ifdef CONFIG_WIRELESS_EXT
	{
		uint32_t u4Idx = 0;

		for (u4Idx = 0; u4Idx < KAL_AIS_NUM; u4Idx++) {
			if (gprWdev[u4Idx] && gprWdev[u4Idx]->netdev) {
				rtnl_lock();
				gprWdev[u4Idx]->netdev->wireless_handlers =
					NULL;
				rtnl_unlock();
			}
		}
	}
#endif
#else
	wlanFreeNetDev();
#endif
	/* gPrDev is assigned by prGlueInfo->prDevHandler,
	 * set NULL to this global variable.
	 */
	gPrDev = NULL;

}				/* end of wlanNetDestroy() */

void wlanSetSuspendMode(struct GLUE_INFO *prGlueInfo,
			u_int8_t fgEnable)
{
	struct net_device *prDev = NULL;
	uint32_t u4PacketFilter = 0;
	uint32_t u4SetInfoLen = 0;
	uint32_t u4Idx = 0;

	if (!prGlueInfo)
		return;


	for (u4Idx = 0; u4Idx < KAL_AIS_NUM; u4Idx++) {
		prDev = wlanGetNetDev(prGlueInfo, u4Idx);
		if (!prDev)
			continue;

		/* new filter should not include p2p mask */
#if CFG_ENABLE_WIFI_DIRECT_CFG_80211
		u4PacketFilter =
			prGlueInfo->prAdapter->u4OsPacketFilter &
			(~PARAM_PACKET_FILTER_P2P_MASK);
#endif
		if (kalIoctl(prGlueInfo,
			wlanoidSetCurrentPacketFilter,
			&u4PacketFilter,
			sizeof(u4PacketFilter), FALSE, FALSE, TRUE,
			&u4SetInfoLen) != WLAN_STATUS_SUCCESS)
			DBGLOG(INIT, ERROR, "set packet filter failed.\n");

		kalSetNetAddressFromInterface(prGlueInfo, prDev, fgEnable);
		wlanNotifyFwSuspend(prGlueInfo, prDev, fgEnable);
	}
}

#if CFG_ENABLE_EARLY_SUSPEND
static struct early_suspend wlan_early_suspend_desc = {
	.level = EARLY_SUSPEND_LEVEL_BLANK_SCREEN,
};

static void wlan_early_suspend(struct early_suspend *h)
{
	struct net_device *prDev = NULL;
	struct GLUE_INFO *prGlueInfo = NULL;

	/* 4 <1> Sanity Check */
	if ((u4WlanDevNum == 0)
	    && (u4WlanDevNum > CFG_MAX_WLAN_DEVICES)) {
		DBGLOG(INIT, ERROR,
		       "wlanLateResume u4WlanDevNum==0 invalid!!\n");
		return;
	}

	prDev = arWlanDevInfo[u4WlanDevNum - 1].prDev;
	if (!prDev)
		return;

	prGlueInfo = *((struct GLUE_INFO **) netdev_priv(prDev));
	if (!prGlueInfo)
		return;

	DBGLOG(INIT, INFO, "********<%s>********\n", __func__);

	if (prGlueInfo->fgIsInSuspendMode == TRUE) {
		DBGLOG(INIT, INFO, "%s: Already in suspend mode, SKIP!\n",
		       __func__);
		return;
	}

	prGlueInfo->fgIsInSuspendMode = TRUE;

	wlanSetSuspendMode(prGlueInfo, TRUE);
	p2pSetSuspendMode(prGlueInfo, TRUE);
}

static void wlan_late_resume(struct early_suspend *h)
{
	struct net_device *prDev = NULL;
	struct GLUE_INFO *prGlueInfo = NULL;

	/* 4 <1> Sanity Check */
	if ((u4WlanDevNum == 0)
	    && (u4WlanDevNum > CFG_MAX_WLAN_DEVICES)) {
		DBGLOG(INIT, ERROR,
		       "wlanLateResume u4WlanDevNum==0 invalid!!\n");
		return;
	}

	prDev = arWlanDevInfo[u4WlanDevNum - 1].prDev;
	if (!prDev)
		return;

	prGlueInfo = *((struct GLUE_INFO **) netdev_priv(prDev));
	if (!prGlueInfo)
		return;

	DBGLOG(INIT, INFO, "********<%s>********\n", __func__);

	if (prGlueInfo->fgIsInSuspendMode == FALSE) {
		DBGLOG(INIT, INFO, "%s: Not in suspend mode, SKIP!\n",
		       __func__);
		return;
	}

	prGlueInfo->fgIsInSuspendMode = FALSE;

	/* 4 <2> Set suspend mode for each network */
	wlanSetSuspendMode(prGlueInfo, FALSE);
	p2pSetSuspendMode(prGlueInfo, FALSE);
}
#endif

#if (CFG_MTK_ANDROID_WMT || WLAN_INCLUDE_PROC)

void reset_p2p_mode(struct GLUE_INFO *prGlueInfo)
{
	struct PARAM_CUSTOM_P2P_SET_STRUCT rSetP2P;
	uint32_t rWlanStatus = WLAN_STATUS_SUCCESS;
	uint32_t u4BufLen = 0;

	if (!prGlueInfo)
		return;

	rSetP2P.u4Enable = 0;
	rSetP2P.u4Mode = 0;

	p2pNetUnregister(prGlueInfo, FALSE);

	rWlanStatus = kalIoctl(prGlueInfo, wlanoidSetP2pMode,
			(void *) &rSetP2P,
			sizeof(struct PARAM_CUSTOM_P2P_SET_STRUCT),
			FALSE, FALSE, FALSE, &u4BufLen);

	if (rWlanStatus != WLAN_STATUS_SUCCESS)
		prGlueInfo->prAdapter->fgIsP2PRegistered = FALSE;

	DBGLOG(INIT, INFO,
			"ret = 0x%08x\n", (uint32_t) rWlanStatus);
}

int set_p2p_mode_handler(struct net_device *netdev,
			 struct PARAM_CUSTOM_P2P_SET_STRUCT p2pmode)
{
	struct GLUE_INFO *prGlueInfo = *((struct GLUE_INFO **)
					 netdev_priv(netdev));
	struct PARAM_CUSTOM_P2P_SET_STRUCT rSetP2P;
	uint32_t rWlanStatus = WLAN_STATUS_SUCCESS;
	uint32_t u4BufLen = 0;

	if (!prGlueInfo)
		return -1;

#if (CFG_MTK_ANDROID_WMT)
	if (prGlueInfo->u4ReadyFlag == 0) {
		DBGLOG(INIT, ERROR, "adapter is not ready\n");
		return -1;
	}
#endif /*CFG_MTK_ANDROID_WMT*/

	/* Remember original ifindex for reset case */
	if (kalIsResetting()) {
		struct GL_P2P_INFO *prP2PInfo = NULL;
		int i = 0;

		for (i = 0 ; i < KAL_P2P_NUM; i++) {
			prP2PInfo = prGlueInfo->prP2PInfo[i];

			if (!prP2PInfo || !prP2PInfo->prDevHandler)
				continue;

			g_u4DevIdx[i] =
				prP2PInfo->prDevHandler->ifindex;
		}
	}

	/* Resetting p2p mode if registered to avoid launch KE */
	if (p2pmode.u4Enable
		&& prGlueInfo->prAdapter->fgIsP2PRegistered
		&& !kalIsResetting()) {
		DBGLOG(INIT, WARN, "Resetting p2p mode\n");
		reset_p2p_mode(prGlueInfo);
	}

	rSetP2P.u4Enable = p2pmode.u4Enable;
	rSetP2P.u4Mode = p2pmode.u4Mode;

	if ((!rSetP2P.u4Enable) && (kalIsResetting() == FALSE))
		p2pNetUnregister(prGlueInfo, FALSE);

	rWlanStatus = kalIoctl(prGlueInfo, wlanoidSetP2pMode,
			(void *) &rSetP2P,
			sizeof(struct PARAM_CUSTOM_P2P_SET_STRUCT),
			FALSE, FALSE, FALSE, &u4BufLen);

	DBGLOG(INIT, INFO,
			"ret = 0x%08x, p2p reg = %d, resetting = %d\n",
			(uint32_t) rWlanStatus,
			prGlueInfo->prAdapter->fgIsP2PRegistered,
			kalIsResetting());


	/* Need to check fgIsP2PRegistered, in case of whole chip reset.
	 * in this case, kalIOCTL return success always,
	 * and prGlueInfo->prP2PInfo[0] may be NULL
	 */
	if ((rSetP2P.u4Enable)
	    && (prGlueInfo->prAdapter->fgIsP2PRegistered)
	    && (kalIsResetting() == FALSE))
		p2pNetRegister(prGlueInfo, FALSE);

	return 0;
}

#endif

#if CFG_SUPPORT_NAN
int set_nan_handler(struct net_device *netdev, uint32_t ucEnable)
{
	struct GLUE_INFO *prGlueInfo =
		*((struct GLUE_INFO **)netdev_priv(netdev));
	uint32_t rWlanStatus = WLAN_STATUS_SUCCESS;
	uint32_t u4BufLen = 0;

	if ((!ucEnable) && (kalIsResetting() == FALSE))
		nanNetUnregister(prGlueInfo, FALSE);

	rWlanStatus = kalIoctl(prGlueInfo, wlanoidSetNANMode, (void *)&ucEnable,
			       sizeof(uint32_t), FALSE, FALSE, TRUE, &u4BufLen);

	DBGLOG(INIT, INFO, "set_nan_handler ret = 0x%08lx\n",
	       (uint32_t)rWlanStatus);

	/* Need to check fgIsNANRegistered, in case of whole chip reset.
	 * in this case, kalIOCTL return success always,
	 * and prGlueInfo->prP2PInfo[0] may be NULL
	 */
	if ((ucEnable) && (prGlueInfo->prAdapter->fgIsNANRegistered) &&
	    (kalIsResetting() == FALSE))
		nanNetRegister(prGlueInfo, FALSE); /* Fixme: error handling */

	return 0;
}
#endif

#if CFG_SUPPORT_CFG_FILE
/*----------------------------------------------------------------------------*/
/*!
 * \brief get config from wifi.cfg
 *
 * \param[in] prAdapter
 *
 * \retval VOID
 */
/*----------------------------------------------------------------------------*/
void wlanGetConfig(struct ADAPTER *prAdapter)
{
	uint8_t *pucConfigBuf;
	uint32_t u4ConfigReadLen;
#ifndef CFG_WIFI_CFG_FN
#define WIFI_CFG_FN	"wifi.cfg"
#else
#define WIFI_CFG_FN	CFG_WIFI_CFG_FN
#endif
	wlanCfgInit(prAdapter, NULL, 0, 0);
	pucConfigBuf = (uint8_t *) kalMemAlloc(
			       WLAN_CFG_FILE_BUF_SIZE, VIR_MEM_TYPE);
	u4ConfigReadLen = 0;
	if (pucConfigBuf) {
		kalMemZero(pucConfigBuf, WLAN_CFG_FILE_BUF_SIZE);
		if (kalRequestFirmware(WIFI_CFG_FN, pucConfigBuf,
			   WLAN_CFG_FILE_BUF_SIZE, &u4ConfigReadLen,
			   prAdapter->prGlueInfo->prDev) == 0) {
			/* ToDo:: Nothing */
		} else if (kalReadToFile("/data/misc/wifi/wifi.cfg",
			   pucConfigBuf, WLAN_CFG_FILE_BUF_SIZE,
			   &u4ConfigReadLen) == 0) {
			/* ToDo:: Nothing */
		} else if (kalReadToFile("/storage/sdcard0/wifi.cfg",
			   pucConfigBuf, WLAN_CFG_FILE_BUF_SIZE,
			   &u4ConfigReadLen) == 0) {
			/* ToDo:: Nothing */
		}

		if (pucConfigBuf[0] != '\0' && u4ConfigReadLen > 0)
			wlanCfgInit(prAdapter,
				pucConfigBuf, u4ConfigReadLen, 0);

		kalMemFree(pucConfigBuf, VIR_MEM_TYPE,
			   WLAN_CFG_FILE_BUF_SIZE);
	}			/* pucConfigBuf */
}

#endif /* CFG_SUPPORT_CFG_FILE */

/*----------------------------------------------------------------------------*/
/*!
 * \brief this function send buffer bin EEPROB_MTxxxx.bin to FW.
 *
 * \param[in] prAdapter
 *
 * \retval WLAN_STATUS_SUCCESS Success
 * \retval WLAN_STATUS_FAILURE Failed
 */
/*----------------------------------------------------------------------------*/
uint32_t wlanDownloadBufferBin(struct ADAPTER *prAdapter)
{
	struct GLUE_INFO *prGlueInfo = NULL;
#if (CFG_FW_Report_Efuse_Address)
	uint16_t u2InitAddr = prAdapter->u4EfuseStartAddress;
#else
	uint16_t u2InitAddr = EFUSE_CONTENT_BUFFER_START;
#endif
	uint32_t u4BufLen = 0;
	uint32_t rStatus = WLAN_STATUS_SUCCESS;
	struct PARAM_CUSTOM_EFUSE_BUFFER_MODE *prSetEfuseBufModeInfo
			= NULL;
	uint32_t u4ContentLen;
	uint8_t *pucConfigBuf = NULL;
	struct mt66xx_chip_info *prChipInfo;
	uint32_t chip_id;
	uint8_t aucEeprom[32];
	uint32_t retWlanStat = WLAN_STATUS_FAILURE;
	int ret;

#if CFG_EFUSE_AUTO_MODE_SUPPORT
	uint32_t u4Efuse_addr = 0;
	struct PARAM_CUSTOM_ACCESS_EFUSE *prAccessEfuseInfo
			= NULL;
#endif

	if (prAdapter->fgIsSupportPowerOnSendBufferModeCMD ==
	    TRUE) {
		DBGLOG(INIT, INFO, "Start Efuse Buffer Mode ..\n");
		DBGLOG(INIT, INFO, "ucEfuseBUfferModeCal is %x\n",
		       prAdapter->rWifiVar.ucEfuseBufferModeCal);

		prChipInfo = prAdapter->chip_info;
		chip_id = prChipInfo->chip_id;
		prGlueInfo = prAdapter->prGlueInfo;

		if (prGlueInfo == NULL || prGlueInfo->prDev == NULL)
			goto label_exit;

		/* allocate memory for buffer mode info */
		prSetEfuseBufModeInfo =
			(struct PARAM_CUSTOM_EFUSE_BUFFER_MODE *)
			kalMemAlloc(sizeof(
				    struct PARAM_CUSTOM_EFUSE_BUFFER_MODE),
				    VIR_MEM_TYPE);
		if (prSetEfuseBufModeInfo == NULL)
			goto label_exit;
		kalMemZero(prSetEfuseBufModeInfo,
			   sizeof(struct PARAM_CUSTOM_EFUSE_BUFFER_MODE));

#if CFG_EFUSE_AUTO_MODE_SUPPORT
		/* allocate memory for Access Efuse Info */
		prAccessEfuseInfo =
			(struct PARAM_CUSTOM_ACCESS_EFUSE *)
			kalMemAlloc(sizeof(
				    struct PARAM_CUSTOM_ACCESS_EFUSE),
				    VIR_MEM_TYPE);
		if (prAccessEfuseInfo == NULL)
			goto label_exit;
		kalMemZero(prAccessEfuseInfo,
			   sizeof(struct PARAM_CUSTOM_ACCESS_EFUSE));

		if (prAdapter->rWifiVar.ucEfuseBufferModeCal == LOAD_AUTO) {
			prAccessEfuseInfo->u4Address = (u4Efuse_addr /
				EFUSE_BLOCK_SIZE) * EFUSE_BLOCK_SIZE;
			rStatus = kalIoctl(prGlueInfo,
				wlanoidQueryProcessAccessEfuseRead,
				prAccessEfuseInfo,
				sizeof(struct PARAM_CUSTOM_ACCESS_EFUSE),
				TRUE, TRUE, TRUE, &u4BufLen);
			if (prGlueInfo->prAdapter->aucEepromVaule[1]
				== (chip_id>>8)) {
				prAdapter->rWifiVar.ucEfuseBufferModeCal
					= LOAD_EFUSE;
				DBGLOG(INIT, STATE,
					"[EFUSE AUTO] EFUSE Mode\n");
			} else {
				prAdapter->rWifiVar.ucEfuseBufferModeCal
					= LOAD_EEPROM_BIN;
				DBGLOG(INIT, STATE,
					"[EFUSE AUTO] Buffer Mode\n");
			}
		}
#endif
		if (prAdapter->rWifiVar.ucEfuseBufferModeCal
			== LOAD_EEPROM_BIN) {
			/* Buffer mode */
			/* Only in buffer mode need to access bin file */
			/* 1 <1> Load bin file*/
			pucConfigBuf = (uint8_t *) kalMemAlloc(
					MAX_EEPROM_BUFFER_SIZE, VIR_MEM_TYPE);
			if (pucConfigBuf == NULL)
				goto label_exit;

			kalMemZero(pucConfigBuf, MAX_EEPROM_BUFFER_SIZE);

			/* 1 <2> Construct EEPROM binary name */
			kalMemZero(aucEeprom, sizeof(aucEeprom));

			ret = kalSnprintf(aucEeprom,
					sizeof(aucEeprom), "%s%x.bin",
					apucEepromName[0], chip_id);
			if (ret == 0 || ret >= sizeof(aucEeprom)) {
				DBGLOG(INIT, ERROR,
						"[%u] snprintf failed, ret: %d\n",
						__LINE__, ret);
				goto label_exit;
			}

			/* 1 <3> Request buffer bin */
			if (kalRequestFirmware(aucEeprom, pucConfigBuf,
			    MAX_EEPROM_BUFFER_SIZE, &u4ContentLen,
			    prGlueInfo->prDev) == 0) {
				DBGLOG(INIT, INFO, "request file done\n");
			} else {
				DBGLOG(INIT, INFO, "can't find file\n");
				goto label_exit;
			}

			/* 1 <4> Send CMD with bin file content */
			prGlueInfo = prAdapter->prGlueInfo;

			/* Update contents in local table */
			kalMemCopy(uacEEPROMImage, pucConfigBuf,
				   MAX_EEPROM_BUFFER_SIZE);

			/* copy to the command buffer */
#if (CFG_FW_Report_Efuse_Address)
			u4ContentLen = (prAdapter->u4EfuseEndAddress) -
				       (prAdapter->u4EfuseStartAddress) + 1;
#else
			u4ContentLen = EFUSE_CONTENT_BUFFER_SIZE;
#endif
			if (u4ContentLen > BUFFER_BIN_PAGE_SIZE)
				goto label_exit;
			kalMemCopy(prSetEfuseBufModeInfo->aBinContent,
				   &pucConfigBuf[u2InitAddr], u4ContentLen);

			prSetEfuseBufModeInfo->ucSourceMode = 1;
		} else {
			/* eFuse mode */
			/* Only need to tell FW the content from, contents are
			 * directly from efuse
			 */
			prSetEfuseBufModeInfo->ucSourceMode = 0;
		}
		prSetEfuseBufModeInfo->ucCmdType = 0x1 |
				   (prAdapter->rWifiVar.ucCalTimingCtrl << 4);
		prSetEfuseBufModeInfo->ucCount   =
			0xFF; /* ucCmdType 1 don't care the ucCount */

		rStatus = kalIoctl(prGlueInfo, wlanoidSetEfusBufferMode,
				(void *)prSetEfuseBufModeInfo,
				sizeof(struct PARAM_CUSTOM_EFUSE_BUFFER_MODE),
				FALSE, TRUE, TRUE, &u4BufLen);
	}

	retWlanStat = WLAN_STATUS_SUCCESS;

label_exit:

	/* free memory */
	if (prSetEfuseBufModeInfo != NULL)
		kalMemFree(prSetEfuseBufModeInfo, VIR_MEM_TYPE,
			   sizeof(struct PARAM_CUSTOM_EFUSE_BUFFER_MODE));

	if (pucConfigBuf != NULL)
		kalMemFree(pucConfigBuf, VIR_MEM_TYPE,
			   MAX_EEPROM_BUFFER_SIZE);

#if CFG_EFUSE_AUTO_MODE_SUPPORT
	if (prAccessEfuseInfo != NULL)
		kalMemFree(prAccessEfuseInfo, VIR_MEM_TYPE,
			sizeof(struct PARAM_CUSTOM_ACCESS_EFUSE));
#endif

	return retWlanStat;
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief this function send buffer bin EEPROB_MTxxxx.bin to FW.
 *
 * \param[in] prAdapter
 *
 * \retval WLAN_STATUS_SUCCESS Success
 * \retval WLAN_STATUS_FAILURE Failed
 */
/*----------------------------------------------------------------------------*/
uint32_t wlanConnacDownloadBufferBin(struct ADAPTER
				     *prAdapter)
{
	struct GLUE_INFO *prGlueInfo = NULL;
	uint32_t u4BufLen = 0;
	uint32_t rStatus = WLAN_STATUS_SUCCESS;
	struct PARAM_CUSTOM_EFUSE_BUFFER_MODE_CONNAC_T
		*prSetEfuseBufModeInfo = NULL;
	uint32_t u4ContentLen;
	uint8_t *pucConfigBuf = NULL;
	struct mt66xx_chip_info *prChipInfo;
	uint32_t chip_id;
	uint8_t aucEeprom[32];
	uint32_t retWlanStat = WLAN_STATUS_FAILURE;
	int ret;

#if CFG_EFUSE_AUTO_MODE_SUPPORT
	uint32_t u4Efuse_addr = 0;
	struct PARAM_CUSTOM_ACCESS_EFUSE *prAccessEfuseInfo
			= NULL;
#endif

	if (prAdapter->fgIsSupportPowerOnSendBufferModeCMD == FALSE)
		return WLAN_STATUS_SUCCESS;

	DBGLOG(INIT, INFO, "Start Efuse Buffer Mode ..\n");
	DBGLOG(INIT, INFO, "ucEfuseBUfferModeCal is %x\n",
	       prAdapter->rWifiVar.ucEfuseBufferModeCal);

	prChipInfo = prAdapter->chip_info;
	chip_id = prChipInfo->chip_id;
	prGlueInfo = prAdapter->prGlueInfo;

	if (prGlueInfo == NULL || prGlueInfo->prDev == NULL)
		goto label_exit;

	/* allocate memory for buffer mode info */
	prSetEfuseBufModeInfo =
		(struct PARAM_CUSTOM_EFUSE_BUFFER_MODE_CONNAC_T *)
		kalMemAlloc(sizeof(
			    struct PARAM_CUSTOM_EFUSE_BUFFER_MODE_CONNAC_T),
			    VIR_MEM_TYPE);
	if (prSetEfuseBufModeInfo == NULL)
		goto label_exit;
	kalMemZero(prSetEfuseBufModeInfo,
		   sizeof(struct PARAM_CUSTOM_EFUSE_BUFFER_MODE_CONNAC_T));

#if CFG_EFUSE_AUTO_MODE_SUPPORT
	/* allocate memory for Access Efuse Info */
	prAccessEfuseInfo =
		(struct PARAM_CUSTOM_ACCESS_EFUSE *)
			kalMemAlloc(sizeof(
				struct PARAM_CUSTOM_ACCESS_EFUSE),
					VIR_MEM_TYPE);
	if (prAccessEfuseInfo == NULL)
		goto label_exit;
	kalMemZero(prAccessEfuseInfo,
		sizeof(struct PARAM_CUSTOM_ACCESS_EFUSE));

	if (prAdapter->rWifiVar.ucEfuseBufferModeCal == LOAD_AUTO) {
		prAccessEfuseInfo->u4Address = (u4Efuse_addr /
			EFUSE_BLOCK_SIZE) * EFUSE_BLOCK_SIZE;
		rStatus = kalIoctl(prGlueInfo,
			wlanoidQueryProcessAccessEfuseRead,
			prAccessEfuseInfo,
			sizeof(struct PARAM_CUSTOM_ACCESS_EFUSE),
			TRUE, TRUE, TRUE, &u4BufLen);
		if (prGlueInfo->prAdapter->aucEepromVaule[1]
			== (chip_id>>8)) {
			prAdapter->rWifiVar.ucEfuseBufferModeCal
				= LOAD_EFUSE;
			DBGLOG(INIT, STATE,
				"[EFUSE AUTO] EFUSE Mode\n");
		} else {
			prAdapter->rWifiVar.ucEfuseBufferModeCal
				= LOAD_EEPROM_BIN;
			DBGLOG(INIT, STATE,
				"[EFUSE AUTO] Buffer Mode\n");
		}
	}
#endif

	if (prAdapter->rWifiVar.ucEfuseBufferModeCal
		== LOAD_EEPROM_BIN) {
		/* Buffer mode */
		/* Only in buffer mode need to access bin file */
		/* 1 <1> Load bin file*/
		pucConfigBuf = (uint8_t *) kalMemAlloc(
				       MAX_EEPROM_BUFFER_SIZE, VIR_MEM_TYPE);
		if (pucConfigBuf == NULL)
			goto label_exit;

		kalMemZero(pucConfigBuf, MAX_EEPROM_BUFFER_SIZE);

		/* 1 <2> Construct EEPROM binary name */
		kalMemZero(aucEeprom, sizeof(aucEeprom));

		ret = kalSnprintf(aucEeprom, sizeof(aucEeprom), "%s%x.bin",
			 apucEepromName[0], chip_id);
		if (ret == 0 || ret >= sizeof(aucEeprom)) {
			DBGLOG(INIT, ERROR,
					"[%u] kalSnprintf failed, ret: %d\n",
					__LINE__, ret);
			goto label_exit;
		}

		/* 1 <3> Request buffer bin */
		if (kalRequestFirmware(aucEeprom, pucConfigBuf,
		    MAX_EEPROM_BUFFER_SIZE, &u4ContentLen, prGlueInfo->prDev)
		    == 0) {
			DBGLOG(INIT, INFO, "request file done\n");
		} else {
			DBGLOG(INIT, INFO, "can't find file\n");
			goto label_exit;
		}
		DBGLOG(INIT, INFO, "u4ContentLen = %d\n", u4ContentLen);

		/* 1 <4> Send CMD with bin file content */
		prGlueInfo = prAdapter->prGlueInfo;

		/* Update contents in local table */
		kalMemCopy(uacEEPROMImage, pucConfigBuf,
			   MAX_EEPROM_BUFFER_SIZE);

		if (u4ContentLen > BUFFER_BIN_PAGE_SIZE)
			goto label_exit;

		kalMemCopy(prSetEfuseBufModeInfo->aBinContent, pucConfigBuf,
			   u4ContentLen);

		prSetEfuseBufModeInfo->ucSourceMode = 1;
	} else {
		/* eFuse mode */
		/* Only need to tell FW the content from, contents are directly
		 * from efuse
		 */
		prSetEfuseBufModeInfo->ucSourceMode = 0;
		u4ContentLen = 0;
	}
	prSetEfuseBufModeInfo->ucContentFormat = 0x1 |
			(prAdapter->rWifiVar.ucCalTimingCtrl << 4);
	prSetEfuseBufModeInfo->u2Count = u4ContentLen;

	rStatus = kalIoctl(prGlueInfo, wlanoidConnacSetEfusBufferMode,
		(void *)prSetEfuseBufModeInfo,
		OFFSET_OF(struct PARAM_CUSTOM_EFUSE_BUFFER_MODE_CONNAC_T,
		aBinContent) + u4ContentLen,
		FALSE, TRUE, TRUE, &u4BufLen);

	retWlanStat = WLAN_STATUS_SUCCESS;

label_exit:

	/* free memory */
	if (prSetEfuseBufModeInfo != NULL)
		kalMemFree(prSetEfuseBufModeInfo, VIR_MEM_TYPE,
			sizeof(struct PARAM_CUSTOM_EFUSE_BUFFER_MODE_CONNAC_T));

	if (pucConfigBuf != NULL)
		kalMemFree(pucConfigBuf, VIR_MEM_TYPE,
			   MAX_EEPROM_BUFFER_SIZE);

#if CFG_EFUSE_AUTO_MODE_SUPPORT
	if (prAccessEfuseInfo != NULL)
		kalMemFree(prAccessEfuseInfo, VIR_MEM_TYPE,
			sizeof(struct PARAM_CUSTOM_ACCESS_EFUSE));
#endif

	return retWlanStat;
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief this function send buffer bin EEPROB_MTxxxx.bin to FW.
 *
 * \param[in] prAdapter
 *
 * \retval WLAN_STATUS_SUCCESS Success
 * \retval WLAN_STATUS_FAILURE Failed
 */
/*----------------------------------------------------------------------------*/
uint32_t wlanConnac2XDownloadBufferBin(struct ADAPTER *prAdapter)
{
	struct mt66xx_chip_info *prChipInfo = NULL;
	uint32_t chip_id = 0;
	struct GLUE_INFO *prGlueInfo = NULL;
	struct PARAM_CUSTOM_EFUSE_BUFFER_MODE_CONNAC_T
		*prSetEfuseBufModeInfo = NULL;
	uint8_t *pucConfigBuf = NULL;
	uint8_t aucEeprom[32];
	uint32_t u4ContentLen = 0;
	uint8_t uTotalPage = 0;
	uint8_t uPageIdx = 0;
	uint32_t rStatus = WLAN_STATUS_SUCCESS;
	uint32_t u4BufLen = 0;
	uint32_t retWlanStat = WLAN_STATUS_FAILURE;

#if CFG_EFUSE_AUTO_MODE_SUPPORT
	uint32_t u4Efuse_addr = 0;
	struct PARAM_CUSTOM_ACCESS_EFUSE *prAccessEfuseInfo
			= NULL;
#endif

	if (prAdapter->fgIsSupportPowerOnSendBufferModeCMD == FALSE)
		return WLAN_STATUS_SUCCESS;

	DBGLOG(INIT, INFO, "Start Efuse Buffer Mode ..\n");
	DBGLOG(INIT, INFO, "ucEfuseBUfferModeCal is %x\n",
		prAdapter->rWifiVar.ucEfuseBufferModeCal);

	prChipInfo = prAdapter->chip_info;
	chip_id = prChipInfo->chip_id;
	prGlueInfo = prAdapter->prGlueInfo;
	if (prGlueInfo == NULL || prGlueInfo->prDev == NULL)
		goto label_exit;

	/* allocate memory for buffer mode info */
	prSetEfuseBufModeInfo =
		(struct PARAM_CUSTOM_EFUSE_BUFFER_MODE_CONNAC_T *)
		kalMemAlloc(sizeof(
			struct PARAM_CUSTOM_EFUSE_BUFFER_MODE_CONNAC_T),
			VIR_MEM_TYPE);
	if (prSetEfuseBufModeInfo == NULL)
		goto label_exit;
	kalMemZero(prSetEfuseBufModeInfo,
		sizeof(struct PARAM_CUSTOM_EFUSE_BUFFER_MODE_CONNAC_T));

#if CFG_EFUSE_AUTO_MODE_SUPPORT
		/* allocate memory for Access Efuse Info */
		prAccessEfuseInfo =
			(struct PARAM_CUSTOM_ACCESS_EFUSE *)
			kalMemAlloc(sizeof(
				    struct PARAM_CUSTOM_ACCESS_EFUSE),
				    VIR_MEM_TYPE);
		if (prAccessEfuseInfo == NULL)
			goto label_exit;
		kalMemZero(prAccessEfuseInfo,
			   sizeof(struct PARAM_CUSTOM_ACCESS_EFUSE));

		if (prAdapter->rWifiVar.ucEfuseBufferModeCal == LOAD_AUTO) {
			prAccessEfuseInfo->u4Address = (u4Efuse_addr /
				EFUSE_BLOCK_SIZE) * EFUSE_BLOCK_SIZE;
			rStatus = kalIoctl(prGlueInfo,
				wlanoidQueryProcessAccessEfuseRead,
				prAccessEfuseInfo,
				sizeof(struct PARAM_CUSTOM_ACCESS_EFUSE),
				TRUE, TRUE, TRUE, &u4BufLen);
			if (prGlueInfo->prAdapter->aucEepromVaule[1]
				== (chip_id>>8)) {
				prAdapter->rWifiVar.ucEfuseBufferModeCal
					= LOAD_EFUSE;
				DBGLOG(INIT, STATE,
					"[EFUSE AUTO] EFUSE Mode\n");
			} else {
				prAdapter->rWifiVar.ucEfuseBufferModeCal
					= LOAD_EEPROM_BIN;
				DBGLOG(INIT, STATE,
					"[EFUSE AUTO] Buffer Mode\n");
			}
		}
#endif

	if (prAdapter->rWifiVar.ucEfuseBufferModeCal
		== LOAD_EEPROM_BIN) {
		/* Buffer mode */
		/* Only in buffer mode need to access bin file */
		/* 1 <1> Load bin file*/
		pucConfigBuf = (uint8_t *)
			kalMemAlloc(MAX_EEPROM_BUFFER_SIZE, VIR_MEM_TYPE);
		if (pucConfigBuf == NULL)
			goto label_exit;
		kalMemZero(pucConfigBuf, MAX_EEPROM_BUFFER_SIZE);

		/* 1 <2> Construct EEPROM binary name */
		kalMemZero(aucEeprom, sizeof(aucEeprom));
		if (prChipInfo->constructBufferBinFileName == NULL) {
			if (snprintf(aucEeprom, 32, "%s%x.bin",
				 apucEepromName[0], chip_id) < 0) {
				DBGLOG(INIT, ERROR, "gen BIN file name fail\n");
				goto label_exit;
			}
		} else {
			if (prChipInfo->constructBufferBinFileName(
			    prAdapter, aucEeprom) != WLAN_STATUS_SUCCESS) {
				DBGLOG(INIT, ERROR, "gen BIN file name fail\n");
				goto label_exit;
			}
		}

		/* 1 <3> Request buffer bin */
		if (kalRequestFirmware(aucEeprom, pucConfigBuf,
				MAX_EEPROM_BUFFER_SIZE, &u4ContentLen,
				prGlueInfo->prDev) == 0) {
			DBGLOG(INIT, INFO, "request file done\n");
		} else {
			DBGLOG(INIT, INFO, "can't find file\n");
			goto label_exit;
		}
		DBGLOG(INIT, INFO, "u4ContentLen = %d\n", u4ContentLen);

		/* 1 <4> Send CMD with bin file content */
		if (u4ContentLen == 0 || u4ContentLen > MAX_EEPROM_BUFFER_SIZE)
			goto label_exit;

		/* Update contents in local table */
		kalMemCopy(uacEEPROMImage, pucConfigBuf,
			MAX_EEPROM_BUFFER_SIZE);

		uTotalPage = u4ContentLen / BUFFER_BIN_PAGE_SIZE;
		if ((u4ContentLen % BUFFER_BIN_PAGE_SIZE) == 0)
			uTotalPage--;

		prSetEfuseBufModeInfo->ucSourceMode = 1;
	} else {
		/* eFuse mode */
		/* Only need to tell FW the content from, contents are directly
		 * from efuse
		 */
		prSetEfuseBufModeInfo->ucSourceMode = 0;
		u4ContentLen = 0;
		uTotalPage = 0;
	}

	for (uPageIdx = 0; uPageIdx <= uTotalPage; uPageIdx++) {
		/* set format */
		prSetEfuseBufModeInfo->ucContentFormat = (
			CONTENT_FORMAT_WHOLE_CONTENT |
			((uTotalPage << BUFFER_BIN_TOTAL_PAGE_SHIFT)
				& BUFFER_BIN_TOTAL_PAGE_MASK) |
			((uPageIdx << BUFFER_BIN_PAGE_INDEX_SHIFT)
				& BUFFER_BIN_PAGE_INDEX_MASK)
		);
		/* set buffer size */
		prSetEfuseBufModeInfo->u2Count =
			(u4ContentLen < BUFFER_BIN_PAGE_SIZE ?
				u4ContentLen : BUFFER_BIN_PAGE_SIZE);
		/* set buffer */
		kalMemZero(prSetEfuseBufModeInfo->aBinContent,
			BUFFER_BIN_PAGE_SIZE);
		if (prSetEfuseBufModeInfo->u2Count != 0)
			kalMemCopy(prSetEfuseBufModeInfo->aBinContent,
				pucConfigBuf + uPageIdx * BUFFER_BIN_PAGE_SIZE,
				prSetEfuseBufModeInfo->u2Count);
		/* send buffer */
		DBGLOG(INIT, INFO, "[%d/%d] load buffer size: 0x%x\n",
			uPageIdx, uTotalPage, prSetEfuseBufModeInfo->u2Count);
		rStatus = kalIoctl(prGlueInfo, wlanoidConnacSetEfusBufferMode,
			(void *) prSetEfuseBufModeInfo, OFFSET_OF(
				struct PARAM_CUSTOM_EFUSE_BUFFER_MODE_CONNAC_T,
				aBinContent) + prSetEfuseBufModeInfo->u2Count,
			FALSE, TRUE, TRUE, &u4BufLen);

		if (rStatus != WLAN_STATUS_SUCCESS) {
			DBGLOG(INIT, ERROR,
			       "kalIoctl EXT_CMD_ID_EFUSE_BUFFER_MODE 0x%X\n",
			       rStatus);
			goto label_exit;
		}

		/* update remain size */
		u4ContentLen -= prSetEfuseBufModeInfo->u2Count;
	}
	retWlanStat = WLAN_STATUS_SUCCESS;

label_exit:
	/* free memory */
	if (prSetEfuseBufModeInfo != NULL)
		kalMemFree(prSetEfuseBufModeInfo, VIR_MEM_TYPE,
			sizeof(struct PARAM_CUSTOM_EFUSE_BUFFER_MODE_CONNAC_T));
	if (pucConfigBuf != NULL)
		kalMemFree(pucConfigBuf, VIR_MEM_TYPE, MAX_EEPROM_BUFFER_SIZE);

#if CFG_EFUSE_AUTO_MODE_SUPPORT
	if (prAccessEfuseInfo != NULL)
		kalMemFree(prAccessEfuseInfo, VIR_MEM_TYPE,
			sizeof(struct PARAM_CUSTOM_ACCESS_EFUSE));
#endif

	return retWlanStat;
}

#if (CONFIG_WLAN_SERVICE == 1)
uint32_t wlanServiceInit(struct GLUE_INFO *prGlueInfo)
{

	struct service_test *prServiceTest;
	struct test_wlan_info *winfos;
	struct mt66xx_chip_info *prChipInfo;
	uint32_t rStatus = WLAN_STATUS_SUCCESS;

	DBGLOG(INIT, TRACE, "%s enter!\n", __func__);

	if (prGlueInfo == NULL)
		return WLAN_STATUS_FAILURE;

	prChipInfo = prGlueInfo->prAdapter->chip_info;
	prGlueInfo->rService.serv_id = SERV_HANDLE_TEST;
	prGlueInfo->rService.serv_handle
		= kalMemAlloc(sizeof(struct service_test), VIR_MEM_TYPE);
	if (prGlueInfo->rService.serv_handle == NULL) {
		DBGLOG(INIT, WARN,
			"prGlueInfo->rService.serv_handle memory alloc fail!\n");
			return WLAN_STATUS_FAILURE;
	}

	prServiceTest = (struct service_test *)prGlueInfo->rService.serv_handle;
	prServiceTest->test_winfo
		= kalMemAlloc(sizeof(struct test_wlan_info), VIR_MEM_TYPE);
	if (prServiceTest->test_winfo == NULL) {
		DBGLOG(INIT, WARN,
			"prServiceTest->test_winfo memory alloc fail!\n");
			goto label_exit;
	}
	winfos = prServiceTest->test_winfo;

	prServiceTest->test_winfo->net_dev = gPrDev;

	if (prChipInfo->asicGetChipID)
		prServiceTest->test_winfo->chip_id =
			prChipInfo->asicGetChipID(prGlueInfo->prAdapter);
	else
		prServiceTest->test_winfo->chip_id = prChipInfo->chip_id;

	DBGLOG(INIT, WARN,
			"%s chip_id = 0x%x\n", __func__,
			prServiceTest->test_winfo->chip_id);

#if (CFG_MTK_ANDROID_EMI == 1)
	prServiceTest->test_winfo->emi_phy_base = gConEmiPhyBase;
	prServiceTest->test_winfo->emi_phy_size = gConEmiSize;
#else
	DBGLOG(RFTEST, WARN, "Platform doesn't support EMI address\n");
#endif

	prServiceTest->test_op
		= kalMemAlloc(sizeof(struct test_operation), VIR_MEM_TYPE);
	if (prServiceTest->test_op == NULL) {
		DBGLOG(INIT, WARN,
			"prServiceTest->test_op memory alloc fail!\n");
			goto label_exit;
	}

	prServiceTest->engine_offload = true;
	winfos->oid_funcptr = (wlan_oid_handler_t) ServiceWlanOid;

	rStatus = mt_agent_init_service(&prGlueInfo->rService);
	if (rStatus != WLAN_STATUS_SUCCESS)
		DBGLOG(INIT, WARN, "%s init fail err:%d\n", __func__, rStatus);

	return rStatus;

label_exit:

	/* free memory */
	if (prGlueInfo->rService.serv_handle != NULL)
		kalMemFree(prGlueInfo->rService.serv_handle, VIR_MEM_TYPE,
			sizeof(struct service_test));

	if (prServiceTest->test_winfo != NULL)
		kalMemFree(prServiceTest->test_winfo, VIR_MEM_TYPE,
			   sizeof(struct test_wlan_info));

	if (prServiceTest->test_op != NULL)
		kalMemFree(prServiceTest->test_op, VIR_MEM_TYPE,
			   sizeof(struct test_operation));

	return WLAN_STATUS_FAILURE;
}
uint32_t wlanServiceExit(struct GLUE_INFO *prGlueInfo)
{
	uint32_t rStatus = WLAN_STATUS_SUCCESS;
	struct service_test *prServiceTest;

	DBGLOG(INIT, TRACE, "%s enter\n", __func__);

	if (prGlueInfo == NULL)
		return WLAN_STATUS_FAILURE;

	rStatus = mt_agent_exit_service(&prGlueInfo->rService);

	prServiceTest = (struct service_test *)prGlueInfo->rService.serv_handle;

	if (rStatus != WLAN_STATUS_SUCCESS)
		DBGLOG(INIT, WARN, "wlanServiceExit fail err:%d\n", rStatus);

	if (prServiceTest->test_winfo)
		kalMemFree(prServiceTest->test_winfo,
		VIR_MEM_TYPE, sizeof(struct test_wlan_info));

	if (prServiceTest->test_op)
		kalMemFree(prServiceTest->test_op,
		VIR_MEM_TYPE, sizeof(struct test_operation));

	if (prGlueInfo->rService.serv_handle)
		kalMemFree(prGlueInfo->rService.serv_handle,
		VIR_MEM_TYPE, sizeof(struct service_test));

	prGlueInfo->rService.serv_id = 0;

	return rStatus;
}
#endif

#ifdef CONFIG_MTK_CONNSYS_DEDICATED_LOG_PATH

#define FW_LOG_CMD_ON_OFF        0
#define FW_LOG_CMD_SET_LEVEL     1
static uint32_t u4LogOnOffCache = -1;

struct CMD_CONNSYS_FW_LOG {
	int32_t fgCmd;
	int32_t fgValue;
};

uint32_t
connsysFwLogControl(struct ADAPTER *prAdapter, void *pvSetBuffer,
	uint32_t u4SetBufferLen, uint32_t *pu4SetInfoLen)
{
	struct CMD_CONNSYS_FW_LOG *prCmd;
	struct CMD_HEADER rCmdV1Header;
	struct CMD_FORMAT_V1 rCmd_v1;
	uint32_t rStatus = WLAN_STATUS_SUCCESS;

	if ((prAdapter == NULL) || (pvSetBuffer == NULL)
		|| (pu4SetInfoLen == NULL))
		return WLAN_STATUS_FAILURE;

	/* init */
	*pu4SetInfoLen = sizeof(struct CMD_CONNSYS_FW_LOG);
	prCmd = (struct CMD_CONNSYS_FW_LOG *) pvSetBuffer;

	if (prCmd->fgCmd == FW_LOG_CMD_ON_OFF) {

		/*EvtDrvnLogEn 0/1*/
		uint8_t onoff[1] = {'0'};

		DBGLOG(INIT, TRACE, "FW_LOG_CMD_ON_OFF\n");

		rCmdV1Header.cmdType = CMD_TYPE_SET;
		rCmdV1Header.cmdVersion = CMD_VER_1;
		rCmdV1Header.cmdBufferLen = 0;
		rCmdV1Header.itemNum = 0;

		kalMemSet(rCmdV1Header.buffer, 0, MAX_CMD_BUFFER_LENGTH);
		kalMemSet(&rCmd_v1, 0, sizeof(struct CMD_FORMAT_V1));

		rCmd_v1.itemType = ITEM_TYPE_STR;

		/*send string format to firmware */
		rCmd_v1.itemStringLength = kalStrLen("EnableDbgLog");
		kalMemZero(rCmd_v1.itemString, MAX_CMD_NAME_MAX_LENGTH);
		kalMemCopy(rCmd_v1.itemString, "EnableDbgLog",
			rCmd_v1.itemStringLength);

		if (prCmd->fgValue == 1) /* other cases, send 'OFF=0' */
			onoff[0] = '1';
		rCmd_v1.itemValueLength = 1;
		kalMemZero(rCmd_v1.itemValue, MAX_CMD_VALUE_MAX_LENGTH);
		kalMemCopy(rCmd_v1.itemValue, &onoff, 1);

		DBGLOG(INIT, INFO, "Send key word (%s) WITH (%s) to firmware\n",
				rCmd_v1.itemString, rCmd_v1.itemValue);

		kalMemCopy(((struct CMD_FORMAT_V1 *)rCmdV1Header.buffer),
				&rCmd_v1,  sizeof(struct CMD_FORMAT_V1));

		rCmdV1Header.cmdBufferLen += sizeof(struct CMD_FORMAT_V1);
		rCmdV1Header.itemNum = 1;

		rStatus = wlanSendSetQueryCmd(
				prAdapter, /* prAdapter */
				CMD_ID_GET_SET_CUSTOMER_CFG, /* 0x70 */
				TRUE,  /* fgSetQuery */
				FALSE, /* fgNeedResp */
				FALSE, /* fgIsOid */
				NULL,  /* pfCmdDoneHandler*/
				NULL,  /* pfCmdTimeoutHandler */
				sizeof(struct CMD_HEADER),
				(uint8_t *)&rCmdV1Header, /* pucInfoBuffer */
				NULL,  /* pvSetQueryBuffer */
				0      /* u4SetQueryBufferLen */
			);

		/* keep in cache */
		u4LogOnOffCache = prCmd->fgValue;
	} else if (prCmd->fgCmd == FW_LOG_CMD_SET_LEVEL) {
		/*ENG_LOAD_OFFSET 1*/
		/*USERDEBUG_LOAD_OFFSET 2 */
		/*USER_LOAD_OFFSET 3 */
		DBGLOG(INIT, INFO, "FW_LOG_CMD_SET_LEVEL\n");
	} else {
		DBGLOG(INIT, INFO, "command can not parse\n");
	}
	return WLAN_STATUS_SUCCESS;
}

static void consys_log_event_notification(int cmd, int value)
{
	struct CMD_CONNSYS_FW_LOG rFwLogCmd;
	struct GLUE_INFO *prGlueInfo = NULL;
	struct ADAPTER *prAdapter = NULL;
	struct net_device *prDev = gPrDev;
	uint32_t rStatus = WLAN_STATUS_FAILURE;
	uint32_t u4BufLen;

	DBGLOG(INIT, INFO, "gPrDev=%p, cmd=%d, value=%d\n",
		gPrDev, cmd, value);

	if (kalIsHalted()) { /* power-off */
		u4LogOnOffCache = value;
		DBGLOG(INIT, INFO,
			"Power off return, u4LogOnOffCache=%d\n",
				u4LogOnOffCache);
		return;
	}

	prGlueInfo = (prDev != NULL) ?
		*((struct GLUE_INFO **) netdev_priv(prDev)) : NULL;
	DBGLOG(INIT, TRACE, "prGlueInfo=%p\n", prGlueInfo);
	if (!prGlueInfo) {
		u4LogOnOffCache = value;
		DBGLOG(INIT, INFO,
			"prGlueInfo == NULL return, u4LogOnOffCache=%d\n",
				u4LogOnOffCache);
		return;
	}
	prAdapter = prGlueInfo->prAdapter;
	DBGLOG(INIT, TRACE, "prAdapter=%p\n", prAdapter);
	if (!prAdapter) {
		u4LogOnOffCache = value;
		DBGLOG(INIT, INFO,
			"prAdapter == NULL return, u4LogOnOffCache=%d\n",
				u4LogOnOffCache);
		return;
	}

	kalMemZero(&rFwLogCmd, sizeof(rFwLogCmd));
	rFwLogCmd.fgCmd = cmd;
	rFwLogCmd.fgValue = value;

	rStatus = kalIoctl(prGlueInfo,
				   connsysFwLogControl,
				   &rFwLogCmd,
				   sizeof(struct CMD_CONNSYS_FW_LOG),
				   FALSE, FALSE, TRUE,
				   &u4BufLen);
}
#endif

/* fos_change begin */
#if IS_ENABLED(CONFIG_IDME)
static void idme_get_mac_addr(OUT struct REG_INFO *prRegInfo)
{
	struct device_node *ap;
	int32_t len = 0;
	int32_t i, ret;
	char buf[3] = {0};

	ap = of_find_node_by_path(IDME_OF_MAC_ADDR);
	if (likely(ap)) {
		const char *mac_addr = of_get_property(ap, "value", &len);

		if (likely(len >= 12)) {
			for (i = 0; i < 12; i += 2) {
				buf[0] = mac_addr[i];
				buf[1] = mac_addr[i + 1];
				ret = kstrtou8(buf, 16,
					&prRegInfo->aucMacAddr[i >> 1]);
				if (ret)
					DBGLOG(INIT, WARN,
					"kstrtou8 failed, i=%d\n", i);
			}
		}
	}
}

static void idme_get_board_id(void)
{
	struct device_node *ap;
	uint32_t len;

	ap = of_find_node_by_path(IDME_OF_BOARD_ID);
	if (likely(ap)) {
		const char *board_id = of_get_property(ap, "value", &len);

		if (board_id != NULL && likely(len >= 16)) {
			memcpy(idme_board_id, board_id,
				sizeof(idme_board_id) - 1);
			idme_board_id[sizeof(idme_board_id) - 1] = '\0';
		}
	}
}
#endif /* fos_change end */

static
void wlanOnPreAdapterStart(struct GLUE_INFO *prGlueInfo,
	struct ADAPTER *prAdapter,
	struct REG_INFO **pprRegInfo,
	struct mt66xx_chip_info **pprChipInfo)
{
	uint32_t u4Idx = 0;

	DBGLOG(INIT, TRACE, "start.\n");
	prGlueInfo->u4ReadyFlag = 0;

#if CFG_TCP_IP_CHKSUM_OFFLOAD
	prAdapter->fgIsSupportCsumOffload = FALSE;
	prAdapter->u4CSUMFlags = CSUM_OFFLOAD_EN_ALL;
#endif

#if CFG_SUPPORT_CFG_FILE
	wlanGetConfig(prAdapter);
#endif

	prAdapter->CurNoResSeqID = 0;

	/* Init Chip Capability */
	*pprChipInfo = prAdapter->chip_info;
	if ((*pprChipInfo)->asicCapInit)
		(*pprChipInfo)->asicCapInit(prAdapter);

	/* Default support 2.4/5G MIMO */
	prAdapter->rWifiFemCfg.u2WifiPath = (
			WLAN_FLAG_2G4_WF0 | WLAN_FLAG_5G_WF0 |
			WLAN_FLAG_2G4_WF1 | WLAN_FLAG_5G_WF1);

#if (CFG_SUPPORT_WIFI_6G == 1)
	/* Default support 6G MIMO */
	prAdapter->rWifiFemCfg.u2WifiPath6G =
			(WLAN_FLAG_6G_WF0 | WLAN_FLAG_6G_WF1);
#endif

#if (MTK_WCN_HIF_SDIO && CFG_WMT_WIFI_PATH_SUPPORT)
	i4RetVal = mtk_wcn_wmt_wifi_fem_cfg_report((
				void *)&prAdapter->rWifiFemCfg);
	if (i4RetVal)
		DBGLOG(INIT, ERROR, "Get WifiPath from WMT drv fail\n");
	else
		DBGLOG(INIT, INFO,
		       "Get WifiPath from WMT drv success, WifiPath=0x%x\n",
		       prAdapter->rWifiFemCfg.u2WifiPath);
#endif
	/* 4 <5> Start Device */
	*pprRegInfo = &prGlueInfo->rRegInfo;

	/* P_REG_INFO_T prRegInfo = (P_REG_INFO_T) kmalloc(
	 *				sizeof(REG_INFO_T), GFP_KERNEL);
	 */
	kalMemSet(*pprRegInfo, 0, sizeof(struct REG_INFO));

	/* Trigger the action of switching Pwr state to drv_own */
	prAdapter->fgIsFwOwn = TRUE;

	/* Load NVRAM content to REG_INFO_T */
	glLoadNvram(prGlueInfo, *pprRegInfo);

	/* kalMemCopy(&prGlueInfo->rRegInfo, prRegInfo,
	 *            sizeof(REG_INFO_T));
	 */

	/* fos_change begin */
#if IS_ENABLED(CONFIG_IDME)
	idme_get_mac_addr(&prGlueInfo->rRegInfo);
	idme_get_board_id();
#endif /* fos_change end */

	(*pprRegInfo)->u4PowerMode = CFG_INIT_POWER_SAVE_PROF;
#if 0
		prRegInfo->fgEnArpFilter = TRUE;
#endif

	/* The Init value of u4WpaVersion/u4AuthAlg shall be
	 * DISABLE/OPEN, not zero!
	 */
	/* The Init value of u4CipherGroup/u4CipherPairwise shall be
	 * NONE, not zero!
	 */
	for (u4Idx = 0; u4Idx < KAL_AIS_NUM; u4Idx++) {
		struct GL_WPA_INFO *prWpaInfo =
			aisGetWpaInfo(prAdapter, u4Idx);

		if (!prWpaInfo)
			continue;

		prWpaInfo->u4WpaVersion =
			IW_AUTH_WPA_VERSION_DISABLED;
		prWpaInfo->u4AuthAlg = IW_AUTH_ALG_OPEN_SYSTEM;
		prWpaInfo->u4CipherGroup = IW_AUTH_CIPHER_NONE;
		prWpaInfo->u4CipherPairwise = IW_AUTH_CIPHER_NONE;
	}

	glTaskletInit(prGlueInfo);

#if CFG_SUPPORT_NAN
	prAdapter->fgIsNANfromHAL = TRUE;
	prAdapter->ucNanPubNum = 0;
	prAdapter->ucNanSubNum = 0;
	DBGLOG(INIT, WARN, "NAN fgIsNANfromHAL init %u\n",
	       prAdapter->fgIsNANfromHAL);
#endif
}

static
void wlanOnPostAdapterStart(struct ADAPTER *prAdapter,
	struct GLUE_INFO *prGlueInfo)
{
	DBGLOG(INIT, TRACE, "start.\n");
	if (HAL_IS_TX_DIRECT(prAdapter)) {
		if (!prAdapter->fgTxDirectInited) {
			skb_queue_head_init(
					&prAdapter->rTxDirectSkbQueue);

#if KERNEL_VERSION(4, 15, 0) <= LINUX_VERSION_CODE
			timer_setup(&prAdapter->rTxDirectSkbTimer,
					nicTxDirectTimerCheckSkbQ, 0);

			timer_setup(&prAdapter->rTxDirectHifTimer,
					nicTxDirectTimerCheckHifQ, 0);
#else
			init_timer(&prAdapter->rTxDirectSkbTimer);
			prAdapter->rTxDirectSkbTimer.data =
					(unsigned long)prGlueInfo;
			prAdapter->rTxDirectSkbTimer.function =
					nicTxDirectTimerCheckSkbQ;

			init_timer(&prAdapter->rTxDirectHifTimer);
			prAdapter->rTxDirectHifTimer.data =
					(unsigned long)prGlueInfo;
			prAdapter->rTxDirectHifTimer.function =
				nicTxDirectTimerCheckHifQ;
#endif
			prAdapter->fgTxDirectInited = TRUE;
		}
	}

#if (CFG_SUPPORT_TX_TSO_SW == 1)
	skb_queue_head_init(&prAdapter->rTsoQueue);
#endif
}

static int32_t wlanOnPreNetRegister(struct GLUE_INFO *prGlueInfo,
	struct ADAPTER *prAdapter,
	struct mt66xx_chip_info *prChipInfo,
	struct WIFI_VAR *prWifiVar,
	const u_int8_t bAtResetFlow)
{
	uint32_t i;
#if (CFG_SUPPORT_WFDMA_REALLOC == 1)
	uint32_t u4Status;
#endif /* CFG_SUPPORT_WFDMA_REALLOC */

	DBGLOG(INIT, TRACE, "start.\n");

	if (!bAtResetFlow) {
		/* change net device mtu from feature option */
		if (prWifiVar->u4MTU > 0 && prWifiVar->u4MTU <= ETH_DATA_LEN) {
			for (i = 0; i < KAL_AIS_NUM; i++)
				gprWdev[i]->netdev->mtu = prWifiVar->u4MTU;
		}
		INIT_DELAYED_WORK(&prGlueInfo->rRxPktDeAggWork,
				halDeAggRxPktWorker);
	}
	prGlueInfo->main_thread = kthread_run(main_thread,
		prGlueInfo->prDevHandler, "main_thread");
#if CFG_SUPPORT_MULTITHREAD
	INIT_WORK(&prGlueInfo->rTxMsduFreeWork, kalFreeTxMsduWorker);
	prGlueInfo->hif_thread = kthread_run(hif_thread,
			prGlueInfo->prDevHandler, "hif_thread");
	prGlueInfo->rx_thread = kthread_run(rx_thread,
			prGlueInfo->prDevHandler, "rx_thread");
#endif

	if (!bAtResetFlow)
		g_u4HaltFlag = 0;

#if (CFG_SUPPORT_WFDMA_REALLOC == 1)
	u4Status = wlanWfdmaRealloc(prAdapter);

	if (u4Status != WLAN_STATUS_NOT_SUPPORTED &&
	    u4Status != WLAN_STATUS_SUCCESS)
		return -1;
#endif /* CFG_SUPPORT_WFDMA_REALLOC */

#if CFG_SUPPORT_BUFFER_MODE && (CFG_EFUSE_BUFFER_MODE_DELAY_CAL == 1)

#if CFG_MTK_ANDROID_WMT
	if (!bAtResetFlow) {
#else
	{
#endif /* CFG_MTK_ANDROID_WMT */
		if (prChipInfo->downloadBufferBin) {
			if (prChipInfo->downloadBufferBin(prAdapter) !=
					WLAN_STATUS_SUCCESS)
				return -1;
		}
	}
#endif /* CFG_SUPPORT_BUFFER_MODE && CFG_EFUSE_BUFFER_MODE_DELAY_CAL */

#if CFG_SUPPORT_NAN
	if (!bAtResetFlow)
		kalCreateUserSock(prAdapter->prGlueInfo);
#endif

#if CFG_SUPPORT_DBDC
	if (!bAtResetFlow)
		/* Update DBDC default setting */
		cnmInitDbdcSetting(prAdapter);
#endif /*CFG_SUPPORT_DBDC*/

	/* send regulatory information to firmware */
	rlmDomainSendInfoToFirmware(prAdapter);

	/* set MAC address */
	if (!bAtResetFlow) {
		uint32_t rStatus = WLAN_STATUS_FAILURE;
		struct sockaddr MacAddr;
		uint32_t u4SetInfoLen = 0;
		struct net_device *prDevHandler;

		rStatus = kalIoctl(prGlueInfo, wlanoidQueryCurrentAddr,
				&MacAddr.sa_data, PARAM_MAC_ADDR_LEN,
				TRUE, TRUE, FALSE, &u4SetInfoLen);

		if (rStatus != WLAN_STATUS_SUCCESS) {
			DBGLOG(INIT, WARN, "set MAC addr fail 0x%x\n",
							rStatus);
		} else {
			kal_eth_hw_addr_set(prGlueInfo->prDevHandler,
					    MacAddr.sa_data);
			kalMemCopy(prGlueInfo->prDevHandler->perm_addr,
					prGlueInfo->prDevHandler->dev_addr,
					ETH_ALEN);

#if CFG_SHOW_MACADDR_SOURCE
			DBGLOG(INIT, INFO, "MAC address: " MACSTR,
			MAC2STR(prAdapter->rWifiVar.aucMacAddress));
#endif
		}

		/* wlan1 */
		if (KAL_AIS_NUM > 1) {
			prDevHandler = wlanGetNetDev(prGlueInfo, 1);
			kal_eth_hw_addr_set(prDevHandler,
					    prWifiVar->aucMacAddress1);
			kalMemCopy(prDevHandler->perm_addr,
				prDevHandler->dev_addr,
				ETH_ALEN);
#if CFG_SHOW_MACADDR_SOURCE
			DBGLOG(INIT, INFO,
				"MAC1 address: " MACSTR,
				MAC2STR(prDevHandler->dev_addr));
#endif
		}
	}

#if CFG_TCP_IP_CHKSUM_OFFLOAD
	/* set HW checksum offload */
	if (!bAtResetFlow && prAdapter->fgIsSupportCsumOffload) {
		uint32_t rStatus;
		uint32_t u4CSUMFlags = CSUM_OFFLOAD_EN_ALL;
		uint32_t u4SetInfoLen = 0;

		rStatus = kalIoctl(prGlueInfo, wlanoidSetCSUMOffload,
				   (void *) &u4CSUMFlags,
				   sizeof(uint32_t),
				   FALSE, FALSE, TRUE, &u4SetInfoLen);

		if (rStatus == WLAN_STATUS_SUCCESS) {
			for (i = 0; i < KAL_AIS_NUM; i++)
				gprWdev[i]->netdev->features |=
					NETIF_F_IP_CSUM |
					NETIF_F_IPV6_CSUM |
					NETIF_F_RXCSUM;
		} else {
			DBGLOG(INIT, WARN,
			       "set HW checksum offload fail 0x%x\n",
			       rStatus);
			prAdapter->fgIsSupportCsumOffload = FALSE;
		}
	}
#endif

#if (CFG_SUPPORT_TX_TSO_SW == 1)
	for (i = 0; i < KAL_AIS_NUM; i++) {
		gprWdev[i]->netdev->features |=
			NETIF_F_TSO |
			NETIF_F_TSO6 |
			NETIF_F_GSO |
			NETIF_F_SG;
	}
#endif

#if (CFG_SUPPORT_TX_SG == 1)
	for (i = 0; i < KAL_AIS_NUM; i++) {
		if (!gprWdev[i] || !gprWdev[i]->netdev)
			continue;
		gprWdev[i]->netdev->features |=
			NETIF_F_SG;
	}
#endif

#if CFG_SUPPORT_802_11K
	{
		uint32_t u4Idx = 0;

		for (u4Idx = 0; u4Idx < KAL_AIS_NUM; u4Idx++) {
			uint32_t rStatus;
			uint32_t u4SetInfoLen = 0;

			rStatus = kalIoctlByBssIdx(prGlueInfo,
					wlanoidSync11kCapabilities, NULL, 0,
					FALSE, FALSE, TRUE, &u4SetInfoLen,
					u4Idx);
			if (rStatus != WLAN_STATUS_SUCCESS)
				DBGLOG(INIT, WARN,
					"[%d] Set 11k Capabilities fail 0x%x\n",
					u4Idx, rStatus);
		}
	}
#endif
	return 0;
}

static void wlanOnPostNetRegister(void)
{
	DBGLOG(INIT, TRACE, "start.\n");
	/* 4 <4> Register early suspend callback */
#if CFG_ENABLE_EARLY_SUSPEND
	glRegisterEarlySuspend(&wlan_early_suspend_desc,
			       wlan_early_suspend, wlan_late_resume);
#endif
	/* 4 <5> Register Notifier callback */
	wlanRegisterInetAddrNotifier();
}

static
void wlanOnP2pRegistration(struct GLUE_INFO *prGlueInfo,
	struct ADAPTER *prAdapter,
	struct wireless_dev *prWdev)
{
	DBGLOG(INIT, TRACE, "start.\n");

#if (CFG_ENABLE_WIFI_DIRECT && CFG_MTK_ANDROID_WMT)
	register_set_p2p_mode_handler(set_p2p_mode_handler);
#endif

#if CFG_ENABLE_WIFI_DIRECT
	if (prAdapter->rWifiVar.u4RegP2pIfAtProbe) {
		struct PARAM_CUSTOM_P2P_SET_STRUCT rSetP2P;

		rSetP2P.u4Enable = 1;

#ifdef CFG_DRIVER_INITIAL_RUNNING_MODE
		rSetP2P.u4Mode = CFG_DRIVER_INITIAL_RUNNING_MODE;
#else
		rSetP2P.u4Mode = RUNNING_P2P_MODE;
#endif /* CFG_DRIVER_RUNNING_MODE */
		if (set_p2p_mode_handler(prWdev->netdev, rSetP2P) == 0)
			DBGLOG(INIT, INFO,
				"%s: p2p device registered\n",
				__func__);
		else
			DBGLOG(INIT, ERROR,
				"%s: Failed to register p2p device\n",
				__func__);
	}
#endif
}

static
int32_t wlanOnWhenProbeSuccess(struct GLUE_INFO *prGlueInfo,
	struct ADAPTER *prAdapter,
	const u_int8_t bAtResetFlow)
{
	int32_t u4LogLevel = ENUM_WIFI_LOG_LEVEL_DEFAULT;

	DBGLOG(INIT, TRACE, "start.\n");
#if CFG_SUPPORT_CFG_FILE
#if CFG_SUPPORT_EASY_DEBUG
	/* move before reading file
	 * wlanLoadDefaultCustomerSetting(prAdapter);
	 */
	wlanFeatureToFw(prGlueInfo->prAdapter);
#endif
#endif

#if CFG_SUPPORT_IOT_AP_BLACKLIST
	wlanCfgLoadIotApRule(prAdapter);
	wlanCfgDumpIotApRule(prAdapter);
#endif
	if (!bAtResetFlow) {
#if CFG_SUPPORT_AGPS_ASSIST
		kalIndicateAgpsNotify(prAdapter, AGPS_EVENT_WLAN_ON, NULL,
				0);
#endif
#if CFG_SUPPORT_CFG_FILE
		wlanCfgSetSwCtrl(prGlueInfo->prAdapter);
		wlanCfgSetChip(prGlueInfo->prAdapter);
		wlanCfgSetCountryCode(prGlueInfo->prAdapter);
#endif
#if (CFG_SUPPORT_PERMON == 1)
		kalPerMonInit(prGlueInfo);
#endif
#if CFG_MET_TAG_SUPPORT
		if (met_tag_init() != 0)
			DBGLOG(INIT, ERROR, "MET_TAG_INIT error!\n");
#endif
	}

#if CFG_SUPPORT_CAL_RESULT_BACKUP_TO_HOST
	/* Calibration Backup Flow */
	if (!g_fgIsCalDataBackuped) {
		if (rlmTriggerCalBackup(prGlueInfo->prAdapter,
		    g_fgIsCalDataBackuped) == WLAN_STATUS_FAILURE) {
			DBGLOG(RFTEST, INFO,
			       "Error : Boot Time Wi-Fi Enable Fail........\n");
			return -1;
		}

		g_fgIsCalDataBackuped = TRUE;
	} else {
		if (rlmTriggerCalBackup(prGlueInfo->prAdapter,
		    g_fgIsCalDataBackuped) == WLAN_STATUS_FAILURE) {
			DBGLOG(RFTEST, INFO,
			       "Error : Normal Wi-Fi Enable Fail........\n");
			return -1;
		}
	}
#endif

	/* card is ready */
	prGlueInfo->u4ReadyFlag = 1;
#if CFG_MTK_ANDROID_WMT
	update_driver_loaded_status(prGlueInfo->u4ReadyFlag);
#endif

	kalSetHalted(FALSE);

	wlanDbgGetGlobalLogLevel(ENUM_WIFI_LOG_MODULE_FW,
				 &u4LogLevel);
	if (u4LogLevel > ENUM_WIFI_LOG_LEVEL_DEFAULT)
		wlanDbgSetLogLevelImpl(prAdapter,
				       ENUM_WIFI_LOG_LEVEL_VERSION_V1,
				       ENUM_WIFI_LOG_MODULE_FW,
				       u4LogLevel);

#ifdef CONFIG_MTK_CONNSYS_DEDICATED_LOG_PATH
	/* sync log status with firmware */
	if (u4LogOnOffCache != -1) /* -1: connsysD does not set */
		consys_log_event_notification((int)FW_LOG_CMD_ON_OFF,
			u4LogOnOffCache);
#endif

#if CFG_CHIP_RESET_HANG
	if (fgIsResetHangState == SER_L0_HANG_RST_TRGING) {
		DBGLOG(INIT, STATE, "[SER][L0] SET hang!\n");
			fgIsResetHangState = SER_L0_HANG_RST_HANG;
			fgIsResetting = TRUE;
	}
	DBGLOG(INIT, STATE, "[SER][L0] PASS!!\n");
#endif

	return 0;
}

#if (CFG_POWER_ON_DOWNLOAD_EMI_ROM_PATCH == 1)
/* WiFi power on download MCU/WiFi ROM EMI + ROM patch */
/*----------------------------------------------------------------------------*/
/*!
 * \brief Wlan power on init function. This function do the job in the
 *  power on stage, they are:
 *  [1]Power on sequence
 *  [2]Download MCU + WiFi ROM EMI
 *  [3]Download ROM patch
 *
 *  It is to simulate wlanProbe() with the minimum effort to complete
 *  ROM EMI + ROM patch download.
 *
 * \retval 0 Success
 * \retval negative value Failed
 */
/*----------------------------------------------------------------------------*/
static int32_t wlanPowerOnInit(void)
{
/*
*	pvData     data passed by bus driver init function
*		_HIF_EHPI: NULL
*		_HIF_SDIO: sdio bus driver handle
*	see hifAxiProbe() for more detail...
*	pfWlanProbe((void *)prPlatDev,
*			(void *)prPlatDev->id_entry->driver_data)
*/
#if defined(SOC3_0)
#if defined(_HIF_AXI)
/* prPlatDev is already created by initWlan()->glRegisterBus()::axi.c */
	void *pvData = (void *)prPlatDev;
	void *pvDriverData = (void *)prPlatDev->id_entry->driver_data;
#endif

#if defined(_HIF_PCIE)
	void *pvData = NULL;
	void *pvDriverData = (void *)&mt66xx_driver_data_soc3_0;
#endif
#endif

	int32_t i4Status = 0;
	enum ENUM_POWER_ON_INIT_FAIL_REASON {
		BUS_INIT_FAIL,
		NET_CREATE_FAIL,
		BUS_SET_IRQ_FAIL,
		ALLOC_ADAPTER_MEM_FAIL,
		DRIVER_OWN_FAIL,
		INIT_ADAPTER_FAIL,
		INIT_HIFINFO_FAIL,
		ROM_PATCH_DOWNLOAD_FAIL,
		POWER_ON_INIT_DONE,
		FAIL_REASON_NUM
	} eFailReason;
	uint32_t i = 0;
	int32_t i4DevIdx = 0;
	struct wireless_dev *prWdev = NULL;
	struct GLUE_INFO *prGlueInfo = NULL;
	struct ADAPTER *prAdapter = NULL;
	struct mt66xx_chip_info *prChipInfo;
	struct FWDL_OPS_T *prFwDlOps;

	DBGLOG(INIT, INFO, "wlanPowerOnInit::begin\n");

	eFailReason = FAIL_REASON_NUM;

	do {
#if defined(_HIF_AXI)
		/* AXI goes over here, to be conti... */
		prChipInfo = ((struct mt66xx_hif_driver_data *)pvDriverData)
					->chip_info;

		pvData = (void *)prChipInfo->pdev;

		/* we should call axi_enable_device(
		*        (struct platform_device *)pvData);
		* since it is invoked from within hifAxiProbe()::axi.c,
		* before calling pfWlanProbe(), to be conti...
		*/
#endif

#if defined(_HIF_PCIE)
		prChipInfo = ((struct mt66xx_hif_driver_data *)pvDriverData)
					->chip_info;

		pvData = (void *)prChipInfo->pdev;

		/* no need pci_enable_device(dev),
		 * it has already been done in PCI driver's probe() function
		 */
#endif

		/* [1]Copy from wlanProbe()::Begin */
		/* Initialize the IO port of the interface */
		/* GeorgeKuo: pData has different meaning for _HIF_XXX:
		* _HIF_EHPI: pointer to memory base variable, which will be
		*      initialized by glBusInit().
		* _HIF_SDIO: bus driver handle
		*/

		/* Remember to call glBusRelease() in wlanPowerOnDeinit() */
		if (glBusInit(pvData) == FALSE) {
			DBGLOG(INIT, ERROR,
				"[Wi-Fi PWR On] glBusInit() fail\n");

			i4Status = -EIO;

			eFailReason = BUS_INIT_FAIL;

			break;
		}

#if (CFG_SUPPORT_TRACE_TC4 == 1)
		wlanDebugTC4Init();
#endif

		/* Create network device, Adapter, KalInfo,
		*       prDevHandler(netdev)
		*/
		prWdev = wlanNetCreate(pvData, pvDriverData);

		if (prWdev == NULL) {
			DBGLOG(INIT, ERROR,
				"[Wi-Fi PWR On] No memory for dev and its private\n");

			i4Status = -ENOMEM;

			eFailReason = NET_CREATE_FAIL;

			break;
		}

		/* Set the ioaddr to HIF Info */
		prGlueInfo = (struct GLUE_INFO *) wiphy_priv(prWdev->wiphy);

		/* Should we need this??? to be conti... */
		gPrDev = prGlueInfo->prDevHandler;

		/* Setup IRQ */
		i4Status = glBusSetIrq(prWdev->netdev, NULL, prGlueInfo);

		if (i4Status != WLAN_STATUS_SUCCESS) {
			DBGLOG(INIT, ERROR, "[Wi-Fi PWR On] Set IRQ error\n");

			eFailReason = BUS_SET_IRQ_FAIL;

			break;
		}

		prGlueInfo->i4DevIdx = i4DevIdx;

		prAdapter = prGlueInfo->prAdapter;
		/* [1]Copy from wlanProbe()::End */

		/* [2]Copy from wlanProbe()->wlanOnPreAdapterStart()::Begin */
		/* Init Chip Capability */
		prChipInfo = prAdapter->chip_info;

		if (prChipInfo->asicCapInit)
			prChipInfo->asicCapInit(prAdapter);

		/* Trigger the action of switching Pwr state to drv_own */
		prAdapter->fgIsFwOwn = TRUE;

		nicpmWakeUpWiFi(prAdapter);
		/* [2]Copy from wlanProbe()->wlanOnPreAdapterStart()::End */

		/* [3]Copy from
		* wlanProbe()
		*	->wlanAdapterStart()
		*		->wlanOnPreAllocAdapterMem()::Begin
		*/
#if 0  /* Sample's gen4m code base doesn't support */
		prAdapter->u4HifDbgFlag = 0;
		prAdapter->u4HifChkFlag = 0;
		prAdapter->u4TxHangFlag = 0;
		prAdapter->u4NoMoreRfb = 0;
#endif

		prAdapter->u4OwnFailedCount = 0;
		prAdapter->u4OwnFailedLogCount = 0;

#if 0  /* Sample's gen4m code base doesn't support */
		prAdapter->fgEnHifDbgInfo = TRUE;
#endif

		/* Additional with chip reset optimize*/
		prAdapter->ucCmdSeqNum = 0;
		prAdapter->u4PwrCtrlBlockCnt = 0;

		QUEUE_INITIALIZE(&(prAdapter->rPendingCmdQueue));
#if CFG_SUPPORT_MULTITHREAD
		QUEUE_INITIALIZE(&prAdapter->rTxCmdQueue);
		QUEUE_INITIALIZE(&prAdapter->rTxCmdDoneQueue);
#if CFG_FIX_2_TX_PORT
		QUEUE_INITIALIZE(&prAdapter->rTxP0Queue);
		QUEUE_INITIALIZE(&prAdapter->rTxP1Queue);
#else
		for (i = 0; i < TX_PORT_NUM; i++)
			QUEUE_INITIALIZE(&prAdapter->rTxPQueue[i]);
#endif
		QUEUE_INITIALIZE(&prAdapter->rRxQueue);
		QUEUE_INITIALIZE(&prAdapter->rTxDataDoneQueue);
#endif

		/* reset fgIsBusAccessFailed */
		fgIsBusAccessFailed = FALSE;

		/* Allocate mandatory resource for TX/RX */
		if (nicAllocateAdapterMemory(prAdapter)
			!= WLAN_STATUS_SUCCESS) {
			DBGLOG(INIT, ERROR,
				"[Wi-Fi PWR On] nicAllocateAdapterMemory Error!\n");

			i4Status = -ENOMEM;

			eFailReason = ALLOC_ADAPTER_MEM_FAIL;
/*
*#if CFG_ENABLE_KEYWORD_EXCEPTION_MECHANISM & 0
*			mtk_wcn_wmt_assert_keyword(WMTDRV_TYPE_WIFI,
*			"[Wi-Fi PWR On] nicAllocateAdapterMemory Error!");
*#endif
*/
			break;
		}

		/* should we need this?  to be conti... */
		prAdapter->u4OsPacketFilter = PARAM_PACKET_FILTER_SUPPORTED;

		/* WLAN driver acquire LP own */
		DBGLOG(INIT, TRACE, "[Wi-Fi PWR On] Acquiring LP-OWN\n");

		ACQUIRE_POWER_CONTROL_FROM_PM(prAdapter);

		DBGLOG(INIT, TRACE, "[Wi-Fi PWR On] Acquiring LP-OWN-end\n");

		if (prAdapter->fgIsFwOwn == TRUE) {
			DBGLOG(INIT, ERROR,
				"[Wi-Fi PWR On] nicpmSetDriverOwn() failed!\n");

			eFailReason = DRIVER_OWN_FAIL;
/*
*#if CFG_ENABLE_KEYWORD_EXCEPTION_MECHANISM & 0
*			mtk_wcn_wmt_assert_keyword(WMTDRV_TYPE_WIFI,
*				"[Wi-Fi PWR On] nicpmSetDriverOwn() failed!");
*#endif
*/
			break;
		}

		/* Initialize the Adapter:
		*       verify chipset ID, HIF init...
		*       the code snippet just do the copy thing
		*/
		if (nicInitializeAdapter(prAdapter) != WLAN_STATUS_SUCCESS) {
			DBGLOG(INIT, ERROR,
				"[Wi-Fi PWR On] nicInitializeAdapter failed!\n");

			eFailReason = INIT_ADAPTER_FAIL;

			break;
		}

		/* Do the post NIC init adapter:
		* copy only the mandatory task
		* in wlanOnPostNicInitAdapter(prAdapter, FALSE)::Begin
		*/
		nicInitSystemService(prAdapter, FALSE);

		/* Initialize Tx */
		nicTxInitialize(prAdapter);

		/* Initialize Rx */
		nicRxInitialize(prAdapter);
		/* Do the post NIC init adapter:
		* copy only the mandatory task
		* in wlanOnPostNicInitAdapter(prAdapter, FALSE)::End
		*/

		/* HIF SW info initialize */
		if (!halHifSwInfoInit(prAdapter)) {
			DBGLOG(INIT, ERROR,
				"[Wi-Fi PWR On] halHifSwInfoInit failed!\n");

			eFailReason = INIT_HIFINFO_FAIL;

			break;
		}

		/* Enable HIF  cut-through to N9 mode */
		HAL_ENABLE_FWDL(prAdapter, TRUE);

		wlanSetChipEcoInfo(prAdapter);

		/* should we open it, to be conti */
		/* wlanOnPostInitHifInfo(prAdapter); */

		/* Disable interrupt, download is done by polling mode only */
		nicDisableInterrupt(prAdapter);

		/* Initialize Tx Resource to fw download state */
		nicTxInitResetResource(prAdapter);

		/* MCU ROM EMI +
		* WiFi ROM EMI + ROM patch download goes over here::Begin
		*/
		/* assiggned in wlanNetCreate() */
		prChipInfo = prAdapter->chip_info;

		/* It is configured in mt66xx_chip_info_connac2x2.fw_dl_ops */
		prFwDlOps = prChipInfo->fw_dl_ops;

		/* No need to check F/W ready bit,
		* since we are downloading MCU ROM EMI
		* + WiFi ROM EMI + ROM patch
		*/
		/*
		*DBGLOG(INIT, INFO,
		*	"wlanDownloadFW:: Check ready_bits(=0x%x)\n",
		*	prChipInfo->sw_ready_bits);
		*
		*HAL_WIFI_FUNC_READY_CHECK(prAdapter,
		*	prChipInfo->sw_ready_bits, &fgReady);
		*/

		if (prFwDlOps->downloadPatch) {
			HAL_ENABLE_FWDL(prAdapter, TRUE);

			DBGLOG_LIMITED(INIT, INFO,
				"[Wi-Fi PWR On] download Start\n");

			if (prFwDlOps->downloadPatch(prAdapter)
				!= WLAN_STATUS_SUCCESS) {
				eFailReason = ROM_PATCH_DOWNLOAD_FAIL;

				HAL_ENABLE_FWDL(prAdapter, FALSE);

				break;
			}

			DBGLOG_LIMITED(INIT, INFO,
				"[Wi-Fi PWR On] download End\n");

			HAL_ENABLE_FWDL(prAdapter, FALSE);

			/* This is only to download ROM patch in power on stage,
			* have to find a better way, to be conti...
			*/
			prFwDlOps->downloadPatch = NULL;
		}
		/* MCU ROM EMI + WiFi ROM EMI
		* + ROM patch download goes over here::End
		*/

		eFailReason = POWER_ON_INIT_DONE;
		/* [3]Copy from wlanProbe()
		*		->wlanAdapterStart()
		*			->wlanOnPreAllocAdapterMem()::End
		*/
	} while (FALSE);

	switch (eFailReason) {
	case BUS_INIT_FAIL:
		break;

	case NET_CREATE_FAIL:
#if (CFG_SUPPORT_TRACE_TC4 == 1)
		wlanDebugTC4Uninit();  /* Uninit for TC4 debug */
#endif

		/* We should call this, although nothing is inside */
		glBusRelease(pvData);

		break;

	case BUS_SET_IRQ_FAIL:
#if (CFG_SUPPORT_TRACE_TC4 == 1)
		wlanDebugTC4Uninit();  /* Uninit for TC4 debug */
#endif

		/* We should call this, although nothing is inside */
		glBusRelease(pvData);

		wlanWakeLockUninit(prGlueInfo);

		wlanNetDestroy(prWdev);

		break;

	case ALLOC_ADAPTER_MEM_FAIL:
	case DRIVER_OWN_FAIL:
	case INIT_ADAPTER_FAIL:
		/* Should we set Onwership to F/W for advanced debug???
		* to be conti...
		*/
		/* nicpmSetFWOwn(prAdapter, FALSE); */

		glBusFreeIrq(prWdev->netdev,
			*((struct GLUE_INFO **)netdev_priv(prWdev->netdev)));

#if (CFG_SUPPORT_TRACE_TC4 == 1)
		wlanDebugTC4Uninit();  /* Uninit for TC4 debug */
#endif

		/* We should call this, although nothing is inside */
		glBusRelease(pvData);

		wlanWakeLockUninit(prGlueInfo);

		wlanNetDestroy(prWdev);

		break;

	case INIT_HIFINFO_FAIL:
		nicRxUninitialize(prAdapter);

		nicTxRelease(prAdapter, FALSE);

		/* System Service Uninitialization */
		nicUninitSystemService(prAdapter);

		/* Should we set Onwership to F/W for advanced debug???
		* to be conti...
		*/
		/* nicpmSetFWOwn(prAdapter, FALSE); */

		glBusFreeIrq(prWdev->netdev,
			*((struct GLUE_INFO **)netdev_priv(prWdev->netdev)));

#if (CFG_SUPPORT_TRACE_TC4 == 1)
		wlanDebugTC4Uninit();  /* Uninit for TC4 debug */
#endif

		/* We should call this, although nothing is inside */
		glBusRelease(pvData);

		wlanWakeLockUninit(prGlueInfo);

		wlanNetDestroy(prWdev);

		break;

	case ROM_PATCH_DOWNLOAD_FAIL:
	case POWER_ON_INIT_DONE:
		HAL_ENABLE_FWDL(prAdapter, FALSE);

		nicRxUninitialize(prAdapter);

		nicTxRelease(prAdapter, FALSE);

		/* System Service Uninitialization */
		nicUninitSystemService(prAdapter);

		/* Should we set Onwership to F/W for advanced debug???
		* to be conti...
		*/
		/* nicpmSetFWOwn(prAdapter, FALSE); */

		glBusFreeIrq(prWdev->netdev,
			*((struct GLUE_INFO **)netdev_priv(prWdev->netdev)));

#if (CFG_SUPPORT_TRACE_TC4 == 1)
		wlanDebugTC4Uninit();  /* Uninit for TC4 debug */
#endif

		/* We should call this, although nothing is inside */
		glBusRelease(pvData);

		wlanWakeLockUninit(prGlueInfo);

		wlanNetDestroy(prWdev);

		break;

	default:
		break;
	}

	DBGLOG(INIT, INFO, "wlanPowerOnInit::end\n");

	return i4Status;
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief Wlan power on deinit function. This function revert whatever
 * has been altered in the
 * power on stage to restore to the most original state.
 *
 * \param[in] void
 *
 * \return (none)
 */
/*----------------------------------------------------------------------------*/
/*
*static void wlanPowerOnDeinit(void)
*{
*
*}
*/
#endif


void wlanOffStopWlanThreads(IN struct GLUE_INFO *prGlueInfo)
{
	DBGLOG(INIT, TRACE, "start.\n");

	if (prGlueInfo->main_thread == NULL
#if CFG_SUPPORT_MULTITHREAD
		&& prGlueInfo->hif_thread == NULL
		&& prGlueInfo->rx_thread == NULL
#endif
		) {

		DBGLOG(INIT, INFO,
			"Threads are already NULL, skip stop and free\n");
		return;
	}

#if CFG_SUPPORT_MULTITHREAD
	wake_up_interruptible(&prGlueInfo->waitq_hif);
	wait_for_completion_interruptible(
		&prGlueInfo->rHifHaltComp);
	wake_up_interruptible(&prGlueInfo->waitq_rx);
	wait_for_completion_interruptible(&prGlueInfo->rRxHaltComp);
#endif

	/* wake up main thread */
	wake_up_interruptible(&prGlueInfo->waitq);
	/* wait main thread stops */
	wait_for_completion_interruptible(&prGlueInfo->rHaltComp);

	DBGLOG(INIT, INFO, "wlan thread stopped\n");

	/* prGlueInfo->rHifInfo.main_thread = NULL; */
	prGlueInfo->main_thread = NULL;
#if CFG_SUPPORT_MULTITHREAD
	prGlueInfo->hif_thread = NULL;
	prGlueInfo->rx_thread = NULL;

	prGlueInfo->u4TxThreadPid = 0xffffffff;
	prGlueInfo->u4HifThreadPid = 0xffffffff;
#endif

	if (test_and_clear_bit(GLUE_FLAG_OID_BIT, &prGlueInfo->ulFlag) &&
			!completion_done(&prGlueInfo->rPendComp)) {
		struct GL_IO_REQ *prIoReq;

		DBGLOG(INIT, INFO, "Complete on-going ioctl as failure.\n");
		prIoReq = &(prGlueInfo->OidEntry);
		prIoReq->rStatus = WLAN_STATUS_FAILURE;
		complete(&prGlueInfo->rPendComp);
	}
}


#if CFG_CHIP_RESET_SUPPORT
/*----------------------------------------------------------------------------*/
/*!
 * \brief slight off procedure for chip reset
 *
 * \return
 * WLAN_STATUS_FAILURE - reset off fail
 * WLAN_STATUS_SUCCESS - reset off success
 */
/*----------------------------------------------------------------------------*/
int32_t wlanOffAtReset(void)
{
	struct ADAPTER *prAdapter = NULL;
	struct net_device *prDev = NULL;
	struct GLUE_INFO *prGlueInfo = NULL;

	DBGLOG(INIT, STATE, "[SER] Driver Off during Reset\n");
	kalSetHalted(TRUE);
	if (u4WlanDevNum > 0
		&& u4WlanDevNum <= CFG_MAX_WLAN_DEVICES) {
		prDev = arWlanDevInfo[u4WlanDevNum - 1].prDev;
	}

	if (prDev == NULL) {
		DBGLOG(INIT, ERROR, "prDev is NULL\n");
		return WLAN_STATUS_FAILURE;
	}

	prGlueInfo = *((struct GLUE_INFO **) netdev_priv(prDev));
	if (prGlueInfo == NULL) {
		DBGLOG(INIT, INFO, "prGlueInfo is NULL\n");
		wlanFreeNetDev();
		return WLAN_STATUS_FAILURE;
	}

	prAdapter = prGlueInfo->prAdapter;
	if (prAdapter == NULL) {
		DBGLOG(INIT, INFO, "prAdapter is NULL\n");
		wlanFreeNetDev();
		return WLAN_STATUS_FAILURE;
	}

	/* to avoid that wpa_supplicant/hostapd triogger new cfg80211 command */
	prGlueInfo->u4ReadyFlag = 0;
#if CFG_MTK_ANDROID_WMT
	update_driver_loaded_status(prGlueInfo->u4ReadyFlag);
#endif
#if (CFG_SUPPORT_PERMON == 1)
	kalPerMonDestroy(prGlueInfo);
#endif

#if CFG_SUPPORT_MULTITHREAD
	/* Stop works */
	flush_work(&prGlueInfo->rTxMsduFreeWork);
#endif
	/* 4 <2> Mark HALT, notify main thread to stop, and clean up queued
	 *	 requests
	 */
	set_bit(GLUE_FLAG_HALT_BIT, &prGlueInfo->ulFlag);
	wlanOffStopWlanThreads(prGlueInfo);
	if (HAL_IS_TX_DIRECT(prAdapter)) {
		if (prAdapter->fgTxDirectInited) {
			del_timer_sync(&prAdapter->rTxDirectSkbTimer);
			del_timer_sync(&prAdapter->rTxDirectHifTimer);
		}
	}

/* wlanAdapterStop Section Start */
	wlanOffClearAllQueues(prAdapter);

	wlanOffUninitNicModule(prAdapter, TRUE);
/* wlanAdapterStop Section End */

	/* 4 <x> Stopping handling interrupt and free IRQ */
	glBusFreeIrq(prDev, prGlueInfo);

#if (CFG_SUPPORT_TRACE_TC4 == 1)
	wlanDebugTC4Uninit();
#endif
	fgSimplifyResetFlow = TRUE;

	return WLAN_STATUS_SUCCESS;
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief A slight wlan on for chip reset
 *
 * \return
 * WLAN_STATUS_FAILURE - reset on fail
 * WLAN_STATUS_SUCCESS - reset on success
 */
/*----------------------------------------------------------------------------*/
int32_t wlanOnAtReset(void)
{
	struct net_device *prDev = NULL;
	struct GLUE_INFO *prGlueInfo = NULL;
	struct ADAPTER *prAdapter = NULL;
	uint32_t rStatus = WLAN_STATUS_SUCCESS;
	uint32_t u4BufLen = 0;
	uint32_t u4DisconnectReason = DISCONNECT_REASON_CODE_CHIPRESET;
	uint32_t u4Idx = 0;

	enum ENUM_PROBE_FAIL_REASON {
		BUS_INIT_FAIL,
		NET_CREATE_FAIL,
		BUS_SET_IRQ_FAIL,
		ADAPTER_START_FAIL,
		NET_REGISTER_FAIL,
		PROC_INIT_FAIL,
		FAIL_MET_INIT_PROCFS,
		FAIL_REASON_NUM
	} eFailReason = FAIL_REASON_NUM;

	DBGLOG(INIT, STATE, "[SER] Driver On during Reset\n");

	if (u4WlanDevNum > 0
		&& u4WlanDevNum <= CFG_MAX_WLAN_DEVICES) {
		prDev = arWlanDevInfo[u4WlanDevNum - 1].prDev;
	}

	if (prDev == NULL) {
		DBGLOG(INIT, ERROR, "prDev is NULL\n");
		return WLAN_STATUS_FAILURE;
	}

	prGlueInfo = *((struct GLUE_INFO **) netdev_priv(prDev));
	if (prGlueInfo == NULL) {
		DBGLOG(INIT, INFO, "prGlueInfo is NULL\n");
		wlanFreeNetDev();
		return WLAN_STATUS_FAILURE;
	}

	prAdapter = prGlueInfo->prAdapter;
	if (prAdapter == NULL) {
		DBGLOG(INIT, INFO, "prAdapter is NULL\n");
		return WLAN_STATUS_FAILURE;
	}

	prGlueInfo->ulFlag = 0;
	fgSimplifyResetFlow = FALSE;
	do {
#if (CFG_SUPPORT_TRACE_TC4 == 1)
		wlanDebugTC4Init();
#endif
		/* wlanNetCreate partial process */
		QUEUE_INITIALIZE(&prGlueInfo->rCmdQueue);
		prGlueInfo->i4TxPendingCmdNum = 0;
		QUEUE_INITIALIZE(&prGlueInfo->rTxQueue);

		glResetHifInfo(prGlueInfo);

		rStatus = glBusSetIrq(prDev, NULL, prGlueInfo);
		if (rStatus != WLAN_STATUS_SUCCESS) {
			DBGLOG(INIT, ERROR, "Set IRQ error\n");
			eFailReason = BUS_SET_IRQ_FAIL;
			break;
		}

		/* Trigger the action of switching Pwr state to drv_own */
		prAdapter->fgIsFwOwn = TRUE;

		/* wlanAdapterStart Section Start */
		rStatus = wlanAdapterStart(prAdapter,
					   &prGlueInfo->rRegInfo,
					   TRUE);
		if (rStatus != WLAN_STATUS_SUCCESS) {
			eFailReason = ADAPTER_START_FAIL;
			break;
		}

		if (wlanOnPreNetRegister(prGlueInfo, prAdapter,
					 prAdapter->chip_info,
					 &prAdapter->rWifiVar,
					 TRUE)) {
			rStatus = WLAN_STATUS_FAILURE;
			eFailReason = NET_REGISTER_FAIL;
			break;
		}

		/* Resend schedule scan */
		prAdapter->rWifiVar.rScanInfo.fgSchedScanning = FALSE;
#if CFG_SUPPORT_SCHED_SCAN
		if (prGlueInfo->prSchedScanRequest) {
			rStatus = kalIoctl(prGlueInfo, wlanoidSetStartSchedScan,
					prGlueInfo->prSchedScanRequest,
					sizeof(struct PARAM_SCHED_SCAN_REQUEST),
					false, FALSE, TRUE, &u4BufLen);
			if (rStatus != WLAN_STATUS_SUCCESS)
				DBGLOG(INIT, WARN,
				"SCN: Start sched scan failed after chip reset 0x%x\n",
					rStatus);
		}
#endif

		for (u4Idx = 0; u4Idx < KAL_AIS_NUM; u4Idx++) {
			struct FT_IES *prFtIEs =
				aisGetFtIe(prAdapter, u4Idx);
			struct CONNECTION_SETTINGS *prConnSettings =
				aisGetConnSettings(prAdapter, u4Idx);

			kalMemZero(prFtIEs,
				sizeof(*prFtIEs));
			prConnSettings->fgIsScanReqIssued = FALSE;
		}

	} while (FALSE);

	if (rStatus == WLAN_STATUS_SUCCESS) {
		wlanOnWhenProbeSuccess(prGlueInfo, prAdapter, TRUE);
		DBGLOG(INIT, INFO, "reset success\n");

		/* Clear pending request (SCAN). */
		scnFreeAllPendingScanRquests(prAdapter);

		/* Send disconnect */
		for (u4Idx = 0; u4Idx < KAL_AIS_NUM; u4Idx++) {
			/* Clear pending request (AIS). */
			aisFsmFlushRequest(prAdapter, u4Idx);

			/* If scan state is SCAN_STATE_SCANNING, means
			 * that have scan req not done before SER.
			 * Abort this request to prevent scan fail
			 * (scan state back to IDLE).
			 */
			if (prAdapter->rWifiVar.rScanInfo.eCurrentState
				== SCAN_STATE_SCANNING) {
				aisFsmStateAbort_SCAN(prAdapter, u4Idx);
			};

			rStatus = kalIoctlByBssIdx(prGlueInfo,
				wlanoidSetDisassociate,
				&u4DisconnectReason,
				0, FALSE, FALSE, TRUE, &u4BufLen,
				u4Idx);

			if (rStatus != WLAN_STATUS_SUCCESS) {
				DBGLOG(REQ, WARN,
					"disassociate error:%x\n", rStatus);
				continue;
			}
			DBGLOG(INIT, INFO,
				"%d inform disconnected\n", u4Idx);
		}
	} else {
		prAdapter->u4HifDbgFlag |= DEG_HIF_DEFAULT_DUMP;
		halPrintHifDbgInfo(prAdapter);
		DBGLOG(INIT, WARN, "Fail reason: %d\n", eFailReason);

		/* Remove error handling here, leave it to coming wlanRemove
		 * for full clean.
		 *
		 * If WMT being removed in the future, you should invoke
		 * wlanRemove directly from here
		 */
#if 0
		switch (eFailReason) {
		case ADAPTER_START_FAIL:
			glBusFreeIrq(prDev,
				*((struct GLUE_INFO **)
						netdev_priv(prDev)));
			kal_fallthrough;
		case BUS_SET_IRQ_FAIL:
			wlanWakeLockUninit(prGlueInfo);
			wlanNetDestroy(prDev->ieee80211_ptr);
			/* prGlueInfo->prAdapter is released in
			 * wlanNetDestroy
			 */
			/* Set NULL value for local prAdapter as well */
			prAdapter = NULL;
			break;
		default:
			break;
		}
#endif
	}
	return rStatus;
}
#endif

/*----------------------------------------------------------------------------*/
/*!
 * \brief Wlan probe function. This function probes and initializes the device.
 *
 * \param[in] pvData     data passed by bus driver init function
 *                           _HIF_EHPI: NULL
 *                           _HIF_SDIO: sdio bus driver handle
 *
 * \retval 0 Success
 * \retval negative value Failed
 */
/*----------------------------------------------------------------------------*/
static int32_t wlanProbe(void *pvData, void *pvDriverData)
{
	struct wireless_dev *prWdev = NULL;
	enum ENUM_PROBE_FAIL_REASON {
		BUS_INIT_FAIL,
		NET_CREATE_FAIL,
		BUS_SET_IRQ_FAIL,
		ADAPTER_START_FAIL,
		NET_REGISTER_FAIL,
		PROC_INIT_FAIL,
		FAIL_MET_INIT_PROCFS,
		FAIL_REASON_NUM
	} eFailReason;
	struct WLANDEV_INFO *prWlandevInfo = NULL;
	int32_t i4DevIdx = 0;
	struct GLUE_INFO *prGlueInfo = NULL;
	struct ADAPTER *prAdapter = NULL;
	int32_t i4Status = 0;
	u_int8_t bRet = FALSE;
	u_int8_t i = 0;
	struct REG_INFO *prRegInfo;
	struct mt66xx_chip_info *prChipInfo;
	struct WIFI_VAR *prWifiVar;
#if (MTK_WCN_HIF_SDIO && CFG_WMT_WIFI_PATH_SUPPORT)
	int32_t i4RetVal = 0;
#endif
	uint32_t u4Idx = 0;

#if CFG_CHIP_RESET_SUPPORT
	if (fgSimplifyResetFlow)
		return wlanOnAtReset();
#endif

#if 0
	uint8_t *pucConfigBuf = NULL, pucCfgBuf = NULL;
	uint32_t u4ConfigReadLen = 0;
#endif
	eFailReason = FAIL_REASON_NUM;
	do {
		/* 4 <1> Initialize the IO port of the interface */
		/*  GeorgeKuo: pData has different meaning for _HIF_XXX:
		 * _HIF_EHPI: pointer to memory base variable, which will be
		 *      initialized by glBusInit().
		 * _HIF_SDIO: bus driver handle
		 */
#ifdef CFG_CHIP_RESET_KO_SUPPORT
		rstNotifyWholeChipRstStatus(RST_MODULE_WIFI,
			RST_MODULE_STATE_PROBE_START, NULL);
#endif

		DBGLOG(INIT, INFO, "enter wlanProbe\n");

		bRet = glBusInit(pvData);

#if (CFG_SUPPORT_TRACE_TC4 == 1)
		wlanDebugTC4Init();
#endif
		/* Cannot get IO address from interface */
		if (bRet == FALSE) {
			DBGLOG(INIT, ERROR, "wlanProbe: glBusInit() fail\n");
			i4Status = -EIO;
			eFailReason = BUS_INIT_FAIL;
			break;
		}
		/* 4 <2> Create network device, Adapter, KalInfo,
		 *       prDevHandler(netdev)
		 */
		prWdev = wlanNetCreate(pvData, pvDriverData);
		if (prWdev == NULL) {
			DBGLOG(INIT, ERROR,
			       "wlanProbe: No memory for dev and its private\n");
			i4Status = -ENOMEM;
			eFailReason = NET_CREATE_FAIL;
			break;
		}
		/* 4 <2.5> Set the ioaddr to HIF Info */
		prGlueInfo = (struct GLUE_INFO *) wiphy_priv(prWdev->wiphy);
		gPrDev = prGlueInfo->prDevHandler;

		/* 4 <4> Setup IRQ */
		prWlandevInfo = &arWlanDevInfo[i4DevIdx];

		i4Status = glBusSetIrq(prWdev->netdev, NULL, prGlueInfo);

		if (i4Status != WLAN_STATUS_SUCCESS) {
			DBGLOG(INIT, ERROR, "wlanProbe: Set IRQ error\n");
			eFailReason = BUS_SET_IRQ_FAIL;
			break;
		}

		prGlueInfo->i4DevIdx = i4DevIdx;

		prAdapter = prGlueInfo->prAdapter;
		prWifiVar = &prAdapter->rWifiVar;

		wlanOnPreAdapterStart(prGlueInfo,
			prAdapter,
			&prRegInfo,
			&prChipInfo);

		if (wlanAdapterStart(prAdapter,
				     prRegInfo, FALSE) != WLAN_STATUS_SUCCESS)
			i4Status = -EIO;

		wlanOnPostAdapterStart(prAdapter, prGlueInfo);

		/* kfree(prRegInfo); */

		if (i4Status < 0) {
			eFailReason = ADAPTER_START_FAIL;
			break;
		}

		if (wlanOnPreNetRegister(prGlueInfo, prAdapter, prChipInfo,
					 prWifiVar, FALSE)) {
			i4Status = -EIO;
			eFailReason = NET_REGISTER_FAIL;
			break;
		}

		/* 4 <3> Register the card */
		i4DevIdx = wlanNetRegister(prWdev);
		if (i4DevIdx < 0) {
			i4Status = -ENXIO;
			DBGLOG(INIT, ERROR,
			       "wlanProbe: Cannot register the net_device context to the kernel\n");
			eFailReason = NET_REGISTER_FAIL;
			break;
		}

		wlanOnPostNetRegister();

		/* 4 <6> Initialize /proc filesystem */
#if WLAN_INCLUDE_PROC
		i4Status = procCreateFsEntry(prGlueInfo);
		if (i4Status < 0) {
			DBGLOG(INIT, ERROR, "wlanProbe: init procfs failed\n");
			eFailReason = PROC_INIT_FAIL;
			break;
		}
#endif /* WLAN_INCLUDE_PROC */
#if WLAN_INCLUDE_SYS
		i4Status = sysCreateFsEntry(prGlueInfo);
		if (i4Status < 0) {
			DBGLOG(INIT, ERROR, "wlanProbe: init sysfs failed\n");
			eFailReason = PROC_INIT_FAIL;
			break;
		}
#endif /* WLAN_INCLUDE_SYS */

#if (CFG_SUPPORT_SNIFFER_RADIOTAP == 1)
		prGlueInfo->fgIsEnableMon = FALSE;
		sysCreateMonDbgFs(prGlueInfo);
#endif

#if CFG_MET_PACKET_TRACE_SUPPORT
	kalMetInit(prGlueInfo);
#endif

#if CFG_ENABLE_BT_OVER_WIFI
	prGlueInfo->rBowInfo.fgIsNetRegistered = FALSE;
	prGlueInfo->rBowInfo.fgIsRegistered = FALSE;
	glRegisterAmpc(prGlueInfo);
#endif

#if (CONFIG_WLAN_SERVICE == 1)
	wlanServiceInit(prGlueInfo);
#endif

#if (CFG_MET_PACKET_TRACE_SUPPORT == 1)
		DBGLOG(INIT, TRACE, "init MET procfs...\n");
		i4Status = kalMetInitProcfs(prGlueInfo);
		if (i4Status < 0) {
			DBGLOG(INIT, ERROR,
			       "wlanProbe: init MET procfs failed\n");
			eFailReason = FAIL_MET_INIT_PROCFS;
			break;
		}
#endif

		for (u4Idx = 0; u4Idx < KAL_AIS_NUM; u4Idx++) {
			struct FT_IES *prFtIEs =
				aisGetFtIe(prAdapter, u4Idx);

			kalMemZero(prFtIEs,
				sizeof(*prFtIEs));
		}

		/* Configure 5G band for registered wiphy */
		if (prAdapter->fgEnable5GBand)
			prWdev->wiphy->bands[KAL_BAND_5GHZ] = &mtk_band_5ghz;
		else
			prWdev->wiphy->bands[KAL_BAND_5GHZ] = NULL;

		for (i = 0 ; i < KAL_P2P_NUM; i++) {
			if (gprP2pRoleWdev[i] == NULL)
				continue;

			if (prAdapter->fgEnable5GBand)
				gprP2pRoleWdev[i]->wiphy->bands[KAL_BAND_5GHZ] =
				&mtk_band_5ghz;
			else
				gprP2pRoleWdev[i]->wiphy->bands[KAL_BAND_5GHZ] =
				NULL;
		}

#if (CFG_SUPPORT_WIFI_6G == 1)
		/* Configure 6G band for registered wiphy */
		if (prAdapter->fgIsHwSupport6G)
			prWdev->wiphy->bands[KAL_BAND_6GHZ] = &mtk_band_6ghz;
		else
			prWdev->wiphy->bands[KAL_BAND_6GHZ] = NULL;

		for (i = 0 ; i < KAL_P2P_NUM; i++) {
			if (gprP2pRoleWdev[i] == NULL)
				continue;

			if (prAdapter->fgIsHwSupport6G)
				gprP2pRoleWdev[i]->wiphy->bands[KAL_BAND_6GHZ] =
				&mtk_band_6ghz;
			else
				gprP2pRoleWdev[i]->wiphy->bands[KAL_BAND_6GHZ] =
				NULL;
		}
#endif
	} while (FALSE);

	if (i4Status == 0) {
		wlanOnWhenProbeSuccess(prGlueInfo, prAdapter, FALSE);
		DBGLOG(INIT, INFO,
		       "wlanProbe: probe success, feature set: 0x%llx, persistNetdev: %d\n",
		       wlanGetSupportedFeatureSet(prGlueInfo),
		       CFG_SUPPORT_PERSIST_NETDEV);
		wlanOnP2pRegistration(prGlueInfo, prAdapter, prWdev);
#if CFG_SUPPORT_TPENHANCE_MODE
		wlanTpeInit(prGlueInfo);
#endif /* CFG_SUPPORT_TPENHANCE_MODE */
	} else {
		DBGLOG(INIT, ERROR, "wlanProbe: probe failed, reason:%d\n",
		       eFailReason);
		switch (eFailReason) {
#if CFG_MET_PACKET_TRACE_SUPPORT
		case FAIL_MET_INIT_PROCFS:
			kalMetRemoveProcfs();
#endif
			kal_fallthrough;
		case PROC_INIT_FAIL:
			wlanNetUnregister(prWdev);
			kal_fallthrough;
		case NET_REGISTER_FAIL:
			set_bit(GLUE_FLAG_HALT_BIT, &prGlueInfo->ulFlag);
			/* wake up main thread */
			wake_up_interruptible(&prGlueInfo->waitq);
			/* wait main thread stops */
			wait_for_completion_interruptible(
							&prGlueInfo->rHaltComp);
			wlanAdapterStop(prAdapter);
			kal_fallthrough;
		case ADAPTER_START_FAIL:
			glBusFreeIrq(prWdev->netdev,
				*((struct GLUE_INFO **)
						netdev_priv(prWdev->netdev)));
			kal_fallthrough;
		case BUS_SET_IRQ_FAIL:
			wlanWakeLockUninit(prGlueInfo);
			wlanNetDestroy(prWdev);
			/* prGlueInfo->prAdapter is released in
			 * wlanNetDestroy
			 */
			break;
		case NET_CREATE_FAIL:
			break;
		case BUS_INIT_FAIL:
			break;
		default:
			break;
		}
	}

#ifdef CFG_CHIP_RESET_KO_SUPPORT
	rstNotifyWholeChipRstStatus(RST_MODULE_WIFI,
		RST_MODULE_STATE_PROBE_DONE, NULL);
#endif

	return i4Status;
}				/* end of wlanProbe() */

void
wlanOffNotifyCfg80211Disconnect(IN struct GLUE_INFO *prGlueInfo)
{
	uint32_t u4Idx = 0;
	u_int8_t bNotify = FALSE;

	DBGLOG(INIT, TRACE, "start.\n");

	for (u4Idx = 0; u4Idx < KAL_AIS_NUM; u4Idx++) {
		if (kalGetMediaStateIndicated(prGlueInfo,
			u4Idx) ==
		    MEDIA_STATE_CONNECTED) {
			struct net_device *prDevHandler =
				wlanGetNetDev(prGlueInfo, u4Idx);
			if (!prDevHandler)
				continue;
#if CFG_WPS_DISCONNECT || (KERNEL_VERSION(4, 2, 0) <= CFG80211_VERSION_CODE)
			cfg80211_disconnected(
				prDevHandler, 0, NULL, 0,
				TRUE, GFP_KERNEL);
#else
			cfg80211_disconnected(
				prDevHandler, 0, NULL, 0,
				GFP_KERNEL);
#endif
			bNotify = TRUE;
		}
	}
	if (bNotify)
		kalMsleep(500);
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief A method to stop driver operation and release all resources. Following
 *        this call, no frame should go up or down through this interface.
 *
 * \return (none)
 */
/*----------------------------------------------------------------------------*/
static void wlanRemove(void)
{
	struct net_device *prDev = NULL;
	struct WLANDEV_INFO *prWlandevInfo = NULL;
	struct GLUE_INFO *prGlueInfo = NULL;
	struct ADAPTER *prAdapter = NULL;
	u_int8_t fgResult = FALSE;

	DBGLOG(INIT, INFO, "Remove wlan!\n");

	if (atomic_read(&g_wlanRemoving)) {
		DBGLOG(INIT, ERROR, "wlanRemove in process\n");
		return;
	}
	atomic_set(&g_wlanRemoving, 1);

	/*reset NVRAM State to ready for the next wifi-no*/
	if (g_NvramFsm == NVRAM_STATE_SEND_TO_FW)
		g_NvramFsm = NVRAM_STATE_READY;

#if CFG_CHIP_RESET_SUPPORT
	/* During chip reset, use simplify remove flow first
	 * if anything goes wrong in wlanOffAtReset then goes to normal flow
	 */
	if (fgSimplifyResetFlow) {
		if (wlanOffAtReset() == WLAN_STATUS_SUCCESS) {
			atomic_set(&g_wlanRemoving, 0);
			return;
		}
	}
#endif

	kalSetHalted(TRUE);

	/* 4 <0> Sanity check */
	ASSERT(u4WlanDevNum <= CFG_MAX_WLAN_DEVICES);
	if (u4WlanDevNum == 0) {
		DBGLOG(INIT, ERROR, "u4WlanDevNum = 0\n");
		atomic_set(&g_wlanRemoving, 0);
		return;
	}
#if (CFG_ENABLE_WIFI_DIRECT && CFG_MTK_ANDROID_WMT)
	register_set_p2p_mode_handler(NULL);
#endif
	if (u4WlanDevNum > 0
	    && u4WlanDevNum <= CFG_MAX_WLAN_DEVICES) {
		prDev = arWlanDevInfo[u4WlanDevNum - 1].prDev;
		prWlandevInfo = &arWlanDevInfo[u4WlanDevNum - 1];
	}

	ASSERT(prDev);
	if (prDev == NULL) {
		DBGLOG(INIT, ERROR, "prDev is NULL\n");
		atomic_set(&g_wlanRemoving, 0);
		return;
	}

	prGlueInfo = *((struct GLUE_INFO **) netdev_priv(prDev));
	ASSERT(prGlueInfo);
	if (prGlueInfo == NULL) {
		DBGLOG(INIT, INFO, "prGlueInfo is NULL\n");
		wlanFreeNetDev();
		atomic_set(&g_wlanRemoving, 0);
		return;
	}

#if (CONFIG_WLAN_SERVICE == 1)
	wlanServiceExit(prGlueInfo);
#endif

	/* Hold mutex in order to asynchronously check whether the net */
	/* interface is available					*/
	KAL_ACQUIRE_MUTEX(prGlueInfo->prAdapter, MUTEX_DEL_INF);
	/* to avoid that wpa_supplicant/hostapd triogger new cfg80211 command */
	prGlueInfo->u4ReadyFlag = 0;
	KAL_RELEASE_MUTEX(prGlueInfo->prAdapter, MUTEX_DEL_INF);
#if CFG_MTK_ANDROID_WMT
	update_driver_loaded_status(prGlueInfo->u4ReadyFlag);
#endif

	/* Have tried to do scan done here, but the exception occurs for */
	/* the P2P scan. Keep the original design that scan done in the	 */
	/* p2pStop/wlanStop.						 */

#if WLAN_INCLUDE_PROC
	procRemoveProcfs();
#endif /* WLAN_INCLUDE_PROC */
#if WLAN_INCLUDE_SYS
	sysRemoveSysfs();
#endif /* WLAN_INCLUDE_SYS */
#if (CFG_SUPPORT_SNIFFER_RADIOTAP == 1)
	sysRemoveMonDbgFs();
#endif
	prAdapter = prGlueInfo->prAdapter;
#if (CFG_SUPPORT_PERMON == 1)
	kalPerMonDestroy(prGlueInfo);
#endif

#if CFG_SUPPORT_TPENHANCE_MODE
	wlanTpeUninit(prGlueInfo);
#endif /* CFG_SUPPORT_TPENHANCE_MODE */

	/* Need to get A-DIE ver anytime when device plug in,
	 * or will fail on the case with insert different A-DIE card.
	 */
	prAdapter->chip_info->u4ADieVer = 0xFFFFFFFF;

	prAdapter->CurNoResSeqID = 0;

	/* complete possible pending oid, which may block wlanRemove some time
	 * and then whole chip reset may failed
	 */
	if (kalIsResetting())
		wlanReleasePendingOid(prGlueInfo->prAdapter, 1);

#if CFG_ENABLE_BT_OVER_WIFI
	if (prGlueInfo->rBowInfo.fgIsNetRegistered) {
		bowNotifyAllLinkDisconnected(prGlueInfo->prAdapter);
		/* wait 300ms for BoW module to send deauth */
		kalMsleep(300);
	}
#endif

	wlanOffNotifyCfg80211Disconnect(prGlueInfo);

	/* 20150205 work queue for sched_scan */

	flush_delayed_work(&sched_workq);

#if CFG_REDIRECT_OID_SUPPORT
	flush_delayed_work(&oid_workq);
#endif

#if (CFG_SUPPORT_SUPPLICANT_SME == 1)
#if CFG_SUPPORT_CFG80211_QUEUE
	flush_delayed_work(&cfg80211_workq);
#endif
#endif

#if CFG_AP_80211KVR_INTERFACE
	cancel_delayed_work_sync(&prAdapter->prGlueInfo->rChanNoiseControlWork);
	cancel_delayed_work_sync(&prAdapter->prGlueInfo->rChanNoiseGetInfoWork);
#endif

	down(&g_halt_sem);
	g_u4HaltFlag = 1;
	up(&g_halt_sem);

	/* 4 <2> Mark HALT, notify main thread to stop, and clean up queued
	 *       requests
	 */
	set_bit(GLUE_FLAG_HALT_BIT, &prGlueInfo->ulFlag);

	/* Stop works */
#if CFG_SUPPORT_MULTITHREAD
	flush_work(&prGlueInfo->rTxMsduFreeWork);
#endif
	cancel_delayed_work_sync(&prGlueInfo->rRxPktDeAggWork);

	nicSerDeInit(prGlueInfo->prAdapter);

	wlanOffStopWlanThreads(prGlueInfo);

	if (HAL_IS_TX_DIRECT(prAdapter)) {
		if (prAdapter->fgTxDirectInited) {
			del_timer_sync(&prAdapter->rTxDirectSkbTimer);
			del_timer_sync(&prAdapter->rTxDirectHifTimer);
		}
	}

	/* Destroy wakelock */
	wlanWakeLockUninit(prGlueInfo);

	kalMemSet(&(prGlueInfo->prAdapter->rWlanInfo), 0,
		  sizeof(struct WLAN_INFO));

#if CFG_ENABLE_WIFI_DIRECT
	if (prGlueInfo->prAdapter->fgIsP2PRegistered) {
		DBGLOG(INIT, INFO, "p2pNetUnregister...\n");
		p2pNetUnregister(prGlueInfo, FALSE);
		DBGLOG(INIT, INFO, "p2pRemove...\n");
		/*p2pRemove must before wlanAdapterStop */
		p2pRemove(prGlueInfo);
	}
#endif

#if CFG_SUPPORT_NAN
	if (prGlueInfo->prAdapter->fgIsNANRegistered) {
		DBGLOG(INIT, INFO, "NANNetUnregister...\n");
		nanNetUnregister(prGlueInfo, FALSE);
		DBGLOG(INIT, INFO, "nanRemove...\n");
		/* nanRemove must before wlanAdapterStop */
		nanRemove(prGlueInfo);
	}
	kalReleaseUserSock(prGlueInfo);
#endif

#if CFG_ENABLE_BT_OVER_WIFI
	if (prGlueInfo->rBowInfo.fgIsRegistered)
		glUnregisterAmpc(prGlueInfo);
#endif

#if (CFG_MET_PACKET_TRACE_SUPPORT == 1)
	kalMetRemoveProcfs();
#endif

#if CFG_MET_TAG_SUPPORT
	if (GL_MET_TAG_UNINIT() != 0)
		DBGLOG(INIT, ERROR, "MET_TAG_UNINIT error!\n");
#endif

	/* 4 <4> wlanAdapterStop */
#if CFG_SUPPORT_AGPS_ASSIST
	kalIndicateAgpsNotify(prAdapter, AGPS_EVENT_WLAN_OFF, NULL,
			      0);
#endif

	wlanAdapterStop(prAdapter);

	HAL_LP_OWN_SET(prAdapter, &fgResult);
	DBGLOG(INIT, INFO, "HAL_LP_OWN_SET(%d)\n",
	       (uint32_t) fgResult);

	/* 4 <x> Stopping handling interrupt and free IRQ */
	glBusFreeIrq(prDev, prGlueInfo);

	/* 4 <5> Release the Bus */
	glBusRelease(prDev);

#if (CFG_SUPPORT_TRACE_TC4 == 1)
	wlanDebugTC4Uninit();
#endif
	/* 4 <6> Unregister the card */
	wlanNetUnregister(prDev->ieee80211_ptr);

	flush_delayed_work(&workq);

	/* 4 <7> Destroy the device */
	wlanNetDestroy(prDev->ieee80211_ptr);

	glTaskletUninit(prGlueInfo);

	/* 4 <8> Unregister early suspend callback */
#if CFG_ENABLE_EARLY_SUSPEND
	glUnregisterEarlySuspend(&wlan_early_suspend_desc);
#endif

#if !CFG_SUPPORT_PERSIST_NETDEV
	{
		uint32_t u4Idx = 0;

		for (u4Idx = 0; u4Idx < KAL_AIS_NUM; u4Idx++) {
			if (gprWdev[u4Idx] && gprWdev[u4Idx]->netdev)
				gprWdev[u4Idx]->netdev = NULL;
		}
	}
#endif

	/* 4 <9> Unregister notifier callback */
	wlanUnregisterInetAddrNotifier();

#if CFG_CHIP_RESET_SUPPORT & !CFG_WMT_RESET_API_SUPPORT
	fgIsResetting = FALSE;
#if (CFG_SUPPORT_CONNINFRA == 1)
	update_driver_reset_status(fgIsResetting);
#endif
#endif
	atomic_set(&g_wlanRemoving, 0);
}				/* end of wlanRemove() */

/*----------------------------------------------------------------------------*/
/*!
 * \brief Driver entry point when the driver is configured as a Linux Module,
 *        and is called once at module load time, by the user-level modutils
 *        application: insmod or modprobe.
 *
 * \retval 0     Success
 */
/*----------------------------------------------------------------------------*/
/* 1 Module Entry Point */
static int initWlan(void)
{
	int ret = 0;
	struct GLUE_INFO *prGlueInfo = NULL;

#if (CFG_POWER_ON_DOWNLOAD_EMI_ROM_PATCH == 1)
#if defined(SOC3_0)
#if defined(_HIF_AXI)
/* prPlatDev is already created by initWlan()->glRegisterBus()::axi.c */
	void *pvData = (void *)prPlatDev;
	void *pvDriverData = (void *)prPlatDev->id_entry->driver_data;
#endif

#if defined(_HIF_PCIE)
	void *pvData = NULL;
	void *pvDriverData = (void *)&mt66xx_driver_data_soc3_0;
#endif
#endif

	struct mt66xx_chip_info *prChipInfo;
#endif

#if defined(UT_TEST_MODE) && defined(CFG_BUILD_X86_PLATFORM)
	/* Refer 6765 dts setting */
	char *ptr = NULL;

	gConEmiSize = 0x400000;
	ptr = kmalloc(gConEmiSize, GFP_KERNEL);
	if (!ptr) {
		DBGLOG(INIT, INFO,
		       "initWlan try to allocate 0x%x bytes memory error\n",
		       gConEmiSize);
		return -EINVAL;
	}
	memset(ptr, 0, gConEmiSize);
	gConEmiPhyBase = (phys_addr_t)ptr;
#endif



	DBGLOG(INIT, INFO, "initWlan\n");
	atomic_set(&g_wlanRemoving, 0);

#ifdef CFG_CHIP_RESET_KO_SUPPORT
	rstNotifyWholeChipRstStatus(RST_MODULE_WIFI,
		RST_MODULE_STATE_KO_INSMOD, NULL);
#endif

#ifdef CFG_DRIVER_INF_NAME_CHANGE

	if (kalStrLen(gprifnamesta) > CUSTOM_IFNAMESIZ ||
	    kalStrLen(gprifnamep2p) > CUSTOM_IFNAMESIZ ||
	    kalStrLen(gprifnameap) > CUSTOM_IFNAMESIZ) {
		DBGLOG(INIT, ERROR, "custom infname len illegal > %d\n",
		       CUSTOM_IFNAMESIZ);
		return -EINVAL;
	}

#endif /*  CFG_DRIVER_INF_NAME_CHANGE */

	wlanDebugInit();

	/* memory pre-allocation */
#if CFG_PRE_ALLOCATION_IO_BUFFER
	kalInitIOBuffer(TRUE);
#else
	kalInitIOBuffer(FALSE);
#endif


#if WLAN_INCLUDE_PROC
	procInitFs();
#endif
#if WLAN_INCLUDE_SYS
	sysInitFs();
#endif

	wlanCreateWirelessDevice();
	if (gprWdev[0] == NULL)
		return -ENOMEM;

	prGlueInfo = (struct GLUE_INFO *) wiphy_priv(
			     wlanGetWiphy());
	if (gprWdev[0]) {
		/* P2PDev and P2PRole[0] share the same Wdev */
		if (glP2pCreateWirelessDevice(prGlueInfo) == TRUE)
			gprP2pWdev = gprP2pRoleWdev[0];
	}
	gPrDev = NULL;

#if CFG_DC_USB_WOW_CALLBACK
	/* register system DC wow enable/disable callback function */
	do {
		typedef void (*func_ptr) (int (*f) (void));
		char *func_name = "WifiRegisterPdwncCallback";
		func_ptr pFunc =
			(func_ptr)GLUE_SYMBOL_GET(func_name);
		char *func_name2 = "RegisterPdwncCallbackWifi";
		func_ptr pFunc2 =
			(func_ptr)GLUE_SYMBOL_GET(func_name2);

		if (pFunc) {
			DBGLOG(INIT, ERROR,
			"Register Wow callback1 success\n");
				pFunc(&kalDcSetWow);
			GLUE_SYMBOL_PUT(func_name);
		} else if (pFunc2) {
			DBGLOG(INIT, ERROR,
			"Register Wow callback2 success\n");
			pFunc2(&kalDcSetWow);
			GLUE_SYMBOL_PUT(pFunc2);
		} else
			DBGLOG(INIT, ERROR,
			"No Exported Func Found:%s\n",
			func_name);
	} while (0);
#endif

	ret = ((glRegisterBus(wlanProbe,
			      wlanRemove) == WLAN_STATUS_SUCCESS) ? 0 : -EIO);

	if (ret == -EIO) {
		kalUninitIOBuffer();
		return ret;
	}
#if (CFG_CHIP_RESET_SUPPORT)
	glResetInit(prGlueInfo);
#endif
	kalFbNotifierReg((struct GLUE_INFO *) wiphy_priv(
				 wlanGetWiphy()));
	wlanRegisterNetdevNotifier();

#ifdef CONFIG_MTK_CONNSYS_DEDICATED_LOG_PATH
	wifi_fwlog_event_func_register(consys_log_event_notification);
#endif

#if CFG_MTK_ANDROID_EMI
	/* Set WIFI EMI protection to consys permitted on system boot up */
	kalSetEmiMpuProtection(gConEmiPhyBase, true);
#endif

#if (CFG_POWER_ON_DOWNLOAD_EMI_ROM_PATCH == 1)
/* #pragma message("initWlan(2)::CFG_POWER_ON_DOWNLOAD_EMI_ROM_PATCH == 1") */
/* Power on download MCU ROM EMI + WiFi ROM EMI + ROM patch */
	wlanPowerOnInit();

#if defined(_HIF_AXI) && defined(SOC3_0)
/* AXI goes over here, to be conti... */
#endif

#if defined(_HIF_PCIE) && defined(SOC3_0)
	prChipInfo = ((struct mt66xx_hif_driver_data *)pvDriverData)->chip_info;

	pvData = (void *)prChipInfo->pdev;
#endif

	wlanProbe(pvData, pvDriverData);
#endif

#if CFG_POWER_OFF_CTRL_SUPPORT
	wlanRegisterRebootNotifier();
#endif
#if CFG_AP_80211KVR_INTERFACE
	nl_sk = netlink_kernel_create(&init_net, NETLINK_OSS_KERNEL, NULL);
	if (!nl_sk) {
		DBGLOG(INIT, ERROR, "netlink create failed!\n");
		return -EBUSY;
	}
#endif /* CFG_AP_80211KVR_INTERFACE */


	DBGLOG(INIT, INFO, "initWlan::End\n");

	return ret;
}				/* end of initWlan() */

/*----------------------------------------------------------------------------*/
/*!
 * \brief Driver exit point when the driver as a Linux Module is removed. Called
 *        at module unload time, by the user level modutils application: rmmod.
 *        This is our last chance to clean up after ourselves.
 *
 * \return (none)
 */
/*----------------------------------------------------------------------------*/
/* 1 Module Leave Point */
static void exitWlan(void)
{
#if defined(_HIF_USB) || CFG_SUPPORT_PERSIST_NETDEV
	struct GLUE_INFO *prGlueInfo = NULL;
#endif

#if CFG_AP_80211KVR_INTERFACE
	if (nl_sk != NULL)
		netlink_kernel_release(nl_sk);
#endif /* CFG_AP_80211KVR_INTERFACE */

#if CFG_SUPPORT_PERSIST_NETDEV
	uint32_t u4Idx = 0;
	struct wiphy *wiphy = NULL;

	wiphy = wlanGetWiphy();
	prGlueInfo = (struct GLUE_INFO *) wiphy_priv(wiphy);

	for (u4Idx = 0; u4Idx < KAL_AIS_NUM; u4Idx++) {
		if (gprNetdev[u4Idx]) {
			wlanClearDevIdx(gprWdev[u4Idx]->netdev);
			DBGLOG(INIT, INFO, "Unregister wlan%d netdev start.\n",
					u4Idx);
			unregister_netdev(gprWdev[u4Idx]->netdev);
			DBGLOG(INIT, INFO, "Unregister wlan%d netdev end.\n",
					u4Idx);
			gprWdev[u4Idx]->netdev = gprNetdev[u4Idx] = NULL;
		}
	}

	prGlueInfo->fgIsRegistered = FALSE;

	DBGLOG(INIT, INFO, "Free wlan device..\n");
	wlanFreeNetDev();
#endif

#ifdef CFG_CHIP_RESET_KO_SUPPORT
	rstNotifyWholeChipRstStatus(RST_MODULE_WIFI,
		RST_MODULE_STATE_KO_RMMOD, NULL);
#endif

	kalFbNotifierUnReg();

	wlanUnregisterNetdevNotifier();

#if CFG_CHIP_RESET_SUPPORT
	glResetUninit();
#endif

#if defined(_HIF_USB)
	/* for USB remove ko case, Power off Wifi CMD need to be DONE
	 * before unregister bus, or connsys cannot enter deep sleep
	 * after rmmod
	 */
	prGlueInfo = wlanGetGlueInfo();
	if (prGlueInfo != NULL)
		wlanPowerOffWifi(prGlueInfo->prAdapter);
#endif

	glUnregisterBus(wlanRemove);

#if CFG_DC_USB_WOW_CALLBACK
	/*Unregister system DC wow enable/disable callback function */
	do {
		typedef void (*func_ptr) (int (*f) (void));
		char *func_name = "WifiRegisterPdwncCallback";
		func_ptr pFunc =
			(func_ptr)GLUE_SYMBOL_GET(func_name);
		char *func_name2 = "RegisterPdwncCallbackWifi";
		func_ptr pFunc2 =
			(func_ptr)GLUE_SYMBOL_GET(func_name2);

		if (pFunc) {
			DBGLOG(INIT, STATE,
			"Unregister Wow callback1 success\n");
			pFunc(NULL);
			GLUE_SYMBOL_PUT(func_name);
		} else if (pFunc2) {
			DBGLOG(INIT, STATE,
			"Unregister Wow callback2 success\n");
			pFunc2(NULL);
			GLUE_SYMBOL_PUT(func_name2);
		} else
			DBGLOG(INIT, ERROR,
			"No Exported Func Found:%s\n",
			func_name);
		} while (0);
#endif

	/* free pre-allocated memory */
	kalUninitIOBuffer();

	/* For single wiphy case, it's hardly to free wdev & wiphy in 2 func.
	 * So that, use wlanDestroyAllWdev to replace wlanDestroyWirelessDevice
	 * and glP2pDestroyWirelessDevice.
	 */
	wlanDestroyAllWdev();

#if WLAN_INCLUDE_PROC
	procUninitProcFs();
#endif
#if WLAN_INCLUDE_SYS
	sysUninitSysFs();
#endif
#if defined(UT_TEST_MODE) && defined(CFG_BUILD_X86_PLATFORM)
	kfree((const void *)gConEmiPhyBase);
#endif
#if CFG_POWER_OFF_CTRL_SUPPORT
	wlanUnregisterRebootNotifier();
#endif

	DBGLOG(INIT, INFO, "exitWlan\n");

}				/* end of exitWlan() */

#if CFG_POWER_OFF_CTRL_SUPPORT
static int wf_pdwnc_notify(struct notifier_block *nb,
		unsigned long event, void *unused)
{
	if (event == SYS_RESTART) {
		DBGLOG(HAL, STATE, "wf_pdwnc_notify()\n");
#if CFG_SUPPORT_PERSIST_NETDEV
		uint32_t u4Idx = 0;
		struct GLUE_INFO *prGlueInfo = NULL;
		struct wiphy *wiphy = NULL;

		wiphy = wlanGetWiphy();
		prGlueInfo = (struct GLUE_INFO *) wiphy_priv(wiphy);

		for (u4Idx = 0; u4Idx < KAL_AIS_NUM; u4Idx++) {
			if (gprNetdev[u4Idx]) {
				wlanClearDevIdx(gprWdev[u4Idx]->netdev);
				DBGLOG(INIT, INFO,
					"Unregister wlan%d netdev start.\n",
						u4Idx);
				unregister_netdev(gprWdev[u4Idx]->netdev);
				DBGLOG(INIT, INFO,
					"Unregister wlan%d netdev end.\n",
						u4Idx);
				gprWdev[u4Idx]->netdev = NULL;
				gprNetdev[u4Idx] = NULL;
			}
		}

		prGlueInfo->fgIsRegistered = FALSE;

		DBGLOG(INIT, INFO, "Free wlan device..\n");
		wlanFreeNetDev();
#endif
		kalFbNotifierUnReg();

		wlanUnregisterNetdevNotifier();

#if CFG_CHIP_RESET_SUPPORT
		glResetUninit();
#endif

		glUnregisterBus(wlanRemove);

		/* free pre-allocated memory */
		kalUninitIOBuffer();

		/* For single wiphy case, it's hardly
		 * to free wdev & wiphy in 2 func.
		 * So that, use wlanDestroyAllWdev
		 * to replace wlanDestroyWirelessDevice
		 * and glP2pDestroyWirelessDevice.
		 */
		wlanDestroyAllWdev();

#if WLAN_INCLUDE_PROC
		procUninitProcFs();
#endif
#if WLAN_INCLUDE_SYS
		sysUninitSysFs();
#endif
#if defined(UT_TEST_MODE) && defined(CFG_BUILD_X86_PLATFORM)
		kfree((const void *)gConEmiPhyBase);
#endif

		DBGLOG(HAL, STATE, "wf_pdwnc_notify() done\n");
	}

	if (event == SYS_POWER_OFF || event == SYS_HALT) {
		DBGLOG(HAL, STATE, "DC Set WoW\n");
		kalDcSetWow();
	}
	return 0;
}

static struct notifier_block wf_pdwnc_notifier = {
	.notifier_call = wf_pdwnc_notify,
	.next = NULL,
	.priority = 0,
};

void wlanRegisterRebootNotifier(void)
{
	DBGLOG(HAL, STATE, "wlanRegisterRebootNotifier()\n");
	register_reboot_notifier(&wf_pdwnc_notifier);
	DBGLOG(HAL, STATE, "wlanRegisterRebootNotifier() done\n");
}

void wlanUnregisterRebootNotifier(void)
{
	DBGLOG(HAL, STATE, "wlanUnregisterRebootNotifier()\n");
	unregister_reboot_notifier(&wf_pdwnc_notifier);
	DBGLOG(HAL, STATE, "wlanUnregisterRebootNotifier() done\n");
}

#endif

#if ((MTK_WCN_HIF_SDIO == 1) && (CFG_BUILT_IN_DRIVER == 1)) || \
	((MTK_WCN_HIF_AXI == 1) && (CFG_BUILT_IN_DRIVER == 1))

int mtk_wcn_wlan_gen4_init(void)
{
	return initWlan();
}
EXPORT_SYMBOL(mtk_wcn_wlan_gen4_init);

void mtk_wcn_wlan_gen4_exit(void)
{
	return exitWlan();
}
EXPORT_SYMBOL(mtk_wcn_wlan_gen4_exit);

#elif ((MTK_WCN_HIF_SDIO == 0) && (CFG_BUILT_IN_DRIVER == 1))

device_initcall(initWlan);

#else

module_init(initWlan);
module_exit(exitWlan);

#endif
