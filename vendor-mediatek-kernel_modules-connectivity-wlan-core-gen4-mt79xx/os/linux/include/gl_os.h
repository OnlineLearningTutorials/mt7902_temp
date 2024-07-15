
/* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause */
/*
 * Copyright (c) 2016 MediaTek Inc.
 */
/*
 ** Id: //Department/DaVinci/BRANCHES/MT6620_WIFI_DRIVER_V2_3/os/linux/include
 *      /gl_os.h#4
 */

/*! \file   gl_os.h
 *    \brief  List the external reference to OS for GLUE Layer.
 *
 *    In this file we define the data structure - GLUE_INFO_T to store those
 *    objects
 *    we acquired from OS - e.g. TIMER, SPINLOCK, NET DEVICE ... . And all the
 *    external reference (header file, extern func() ..) to OS for GLUE Layer
 *    should also list down here.
 */


#ifndef _GL_OS_H
#define _GL_OS_H

/*******************************************************************************
 *                         C O M P I L E R   F L A G S
 *******************************************************************************
 */
/*------------------------------------------------------------------------------
 * Flags for LINUX(OS) dependent
 *------------------------------------------------------------------------------
 */
#define CFG_MAX_WLAN_DEVICES 1 /* number of wlan card will coexist */

#define CFG_MAX_TXQ_NUM 4 /* number of tx queue for support multi-queue h/w  */

/* 1: Enable use of SPIN LOCK Bottom Half for LINUX */
/* 0: Disable - use SPIN LOCK IRQ SAVE instead */
#define CFG_USE_SPIN_LOCK_BOTTOM_HALF       0

/* 1: Enable - Drop ethernet packet if it < 14 bytes.
 * And pad ethernet packet with dummy 0 if it < 60 bytes.
 * 0: Disable
 */
#define CFG_TX_PADDING_SMALL_ETH_PACKET     0

#define CFG_TX_STOP_NETIF_QUEUE_THRESHOLD   256	/* packets */

#define CFG_TX_STOP_NETIF_PER_QUEUE_THRESHOLD   256	/* packets */
#define CFG_TX_START_NETIF_PER_QUEUE_THRESHOLD  128	/* packets */

#define CHIP_NAME    "MT6632"

#define DRV_NAME "["CHIP_NAME"]: "

/* Define if target platform is Android.
 * It should already be defined in Android kernel source
 */
#ifndef CONFIG_ANDROID
/* #define CONFIG_ANDROID      0 */

#endif

/* for CFG80211 IE buffering mechanism */
#define	CFG_CFG80211_IE_BUF_LEN		(512)
#define	GLUE_INFO_WSCIE_LENGTH		(500)
/* for non-wfa vendor specific IE buffer */
#define NON_WFA_VENDOR_IE_MAX_LEN	(128)


/*******************************************************************************
 *                    E X T E R N A L   R E F E R E N C E S
 *******************************************************************************
 */
#include <linux/version.h>	/* constant of kernel version */

#include <linux/kernel.h>	/* bitops.h */

#include <linux/timer.h>	/* struct timer_list */
#include <linux/jiffies.h>	/* jiffies */
#include <linux/delay.h>	/* udelay and mdelay macro */
#include <linux/sched.h>
#include <linux/rtc.h>
#include <linux/limits.h>

#ifdef CONFIG_ANDROID
#if (KERNEL_VERSION(4, 9, 0) <= LINUX_VERSION_CODE)
#include <linux/device.h>
#include <linux/pm_wakeup.h>
#else
#include <linux/wakelock.h>
#endif
#endif

#if KERNEL_VERSION(2, 6, 12) < LINUX_VERSION_CODE
#include <linux/irq.h>		/* IRQT_FALLING */
#endif

#include <linux/netdevice.h>	/* struct net_device, struct net_device_stats */
#include <linux/etherdevice.h>	/* for eth_type_trans() function */
#include <linux/wireless.h>	/* struct iw_statistics */
#include <linux/if_arp.h>
#include <linux/inetdevice.h>	/* struct in_device */

#include <linux/ip.h>		/* struct iphdr */

#include <linux/string.h>	/* for memcpy()/memset() function */
#include <linux/stddef.h>	/* for offsetof() macro */

#include <linux/proc_fs.h>	/* The proc filesystem constants/structures */

#include <linux/rtnetlink.h>	/* for rtnl_lock() and rtnl_unlock() */
#include <linux/kthread.h>	/* kthread_should_stop(), kthread_run() */
#include <linux/uaccess.h>	/* for copy_from_user() */
#include <linux/fs.h>		/* for firmware download */
#include <linux/vmalloc.h>

#include <linux/kfifo.h>	/* for kfifo interface */
#include <linux/cdev.h>		/* for cdev interface */

#include <linux/firmware.h>	/* for firmware download */
#include <linux/ctype.h>

#include <linux/interrupt.h>

#if defined(_HIF_USB)
#include <linux/usb.h>
#include <linux/mmc/sdio.h>
#include <linux/mmc/sdio_func.h>
#endif

#if defined(_HIF_PCIE)
#include <linux/pci.h>
#endif

#if defined(_HIF_SDIO)
#include <linux/mmc/card.h>
#include <linux/mmc/sdio.h>
#include <linux/mmc/sdio_func.h>
#endif

#include <linux/random.h>

#include <linux/io.h>		/* readw and writew */

#if WIRELESS_EXT > 12
#include <net/iw_handler.h>
#endif

#ifdef CFG_CFG80211_VERSION
#define CFG80211_VERSION_CODE CFG_CFG80211_VERSION
#else
#define CFG80211_VERSION_CODE LINUX_VERSION_CODE
#endif

#include "version.h"
#include "config.h"

#if CFG_ENABLE_WIFI_DIRECT_CFG_80211
#include <linux/wireless.h>
#include <net/cfg80211.h>
#endif

#include <linux/module.h>
#include <linux/can/netlink.h>
#include <net/netlink.h>

#if IS_ENABLED(CONFIG_IPV6)
#include <linux/ipv6.h>
#include <linux/in6.h>
#include <net/if_inet6.h>
#endif

#if CFG_SUPPORT_PASSPOINT
#include <net/addrconf.h>
#endif /* CFG_SUPPORT_PASSPOINT */

#if KERNEL_VERSION(3, 8, 0) <= CFG80211_VERSION_CODE
#include <uapi/linux/nl80211.h>
#endif

#ifdef UDP_SKT_WIFI
#if (KERNEL_VERSION(4, 4, 0) <= CFG80211_VERSION_CODE)
#include <linux/trace_events.h>
#else
#include <linux/ftrace_event.h>
#endif
#endif
#if (CFG_SUPPORT_TX_TSO_SW == 1)
#include <net/tso.h>
#endif

#include "gl_typedef.h"
#include "typedef.h"
#include "queue.h"
#include "gl_kal.h"
#include "gl_rst.h"
#include "hif.h"

#if CFG_SUPPORT_TDLS
#include "tdls.h"
#endif

#include "debug.h"

#include "wlan_lib.h"
#include "wlan_oid.h"

#if CFG_ENABLE_AEE_MSG
#ifdef CONFIG_ANDROID
#include <mt-plat/aee.h>
#else
#include <linux/aee.h>
#endif
#endif

#if CFG_MET_TAG_SUPPORT
#include <mt-plat/met_drv.h>
#endif
#include <linux/time.h>
#include <linux/fb.h>
#include <linux/capability.h>

#if CFG_SUPPORT_NAN
#include "nan_base.h"
#include "nan_intf.h"
#endif

#if (CONFIG_WLAN_SERVICE == 1)
#include "agent.h"
#endif

extern u_int8_t fgIsBusAccessFailed;
extern const struct ieee80211_iface_combination
	*p_mtk_iface_combinations_sta;
extern const int32_t mtk_iface_combinations_sta_num;
extern const struct ieee80211_iface_combination
	*p_mtk_iface_combinations_p2p;
extern const int32_t mtk_iface_combinations_p2p_num;
extern uint8_t g_aucNvram[];

#ifdef CONFIG_MTK_CONNSYS_DEDICATED_LOG_PATH
typedef void (*wifi_fwlog_event_func_cb)(int, int);
/* adaptor ko */
extern int  wifi_fwlog_onoff_status(void);
extern void wifi_fwlog_event_func_register(wifi_fwlog_event_func_cb pfFwlog);
#endif
#if CFG_MTK_ANDROID_WMT
extern void update_driver_loaded_status(uint8_t loaded);
#endif

/*******************************************************************************
 *                              C O N S T A N T S
 *******************************************************************************
 */
#define GLUE_FLAG_HALT                  BIT(0)
#define GLUE_FLAG_INT                   BIT(1)
#define GLUE_FLAG_OID                   BIT(2)
#define GLUE_FLAG_TIMEOUT               BIT(3)
#define GLUE_FLAG_TXREQ                 BIT(4)
#define GLUE_FLAG_SUB_MOD_MULTICAST     BIT(7)
#define GLUE_FLAG_FRAME_FILTER          BIT(8)
#define GLUE_FLAG_FRAME_FILTER_AIS      BIT(9)

#define GLUE_FLAG_HALT_BIT              (0)
#define GLUE_FLAG_INT_BIT               (1)
#define GLUE_FLAG_OID_BIT               (2)
#define GLUE_FLAG_TIMEOUT_BIT           (3)
#define GLUE_FLAG_TXREQ_BIT             (4)
#define GLUE_FLAG_SUB_MOD_MULTICAST_BIT (7)
#define GLUE_FLAG_FRAME_FILTER_BIT      (8)
#define GLUE_FLAG_FRAME_FILTER_AIS_BIT  (9)

#if CFG_SUPPORT_MULTITHREAD
#define GLUE_FLAG_RX					BIT(10)
#define GLUE_FLAG_TX_CMD_DONE			BIT(11)
#define GLUE_FLAG_HIF_TX				BIT(12)
#define GLUE_FLAG_HIF_TX_CMD			BIT(13)
#define GLUE_FLAG_RX_TO_OS				BIT(14)
#define GLUE_FLAG_HIF_FW_OWN			BIT(15)

#define GLUE_FLAG_TX_CMD_DONE_BIT			(11)
#define GLUE_FLAG_HIF_TX_BIT				(12)
#define GLUE_FLAG_HIF_TX_CMD_BIT			(13)
#define GLUE_FLAG_RX_TO_OS_BIT				(14)
#define GLUE_FLAG_HIF_FW_OWN_BIT			(15)
#endif
#define GLUE_FLAG_RX_BIT				(10)
#define GLUE_FLAG_HIF_PRT_HIF_DBG_INFO		BIT(16)
#define GLUE_FLAG_HIF_PRT_HIF_DBG_INFO_BIT		(16)
#define GLUE_FLAG_UPDATE_WMM_QUOTA			(17)

#if (CFG_SUPPORT_CONNINFRA == 1)
#define GLUE_FLAG_RST_START BIT(18)
#define GLUE_FLAG_RST_START_BIT 18
#endif
#if CFG_SUPPORT_NAN /* notice the bit differnet with 7668 */
#define GLUE_FLAG_NAN_MULTICAST_BIT (20)
#define GLUE_FLAG_NAN_MULTICAST BIT(20)
#endif

#define GLUE_BOW_KFIFO_DEPTH        (1024)
/* #define GLUE_BOW_DEVICE_NAME        "MT6620 802.11 AMP" */
#define GLUE_BOW_DEVICE_NAME        "ampc0"

#define WAKE_LOCK_RX_TIMEOUT                            300	/* ms */
#define WAKE_LOCK_THREAD_WAKEUP_TIMEOUT                 50	/* ms */

#if KERNEL_VERSION(4, 0, 0) > CFG80211_VERSION_CODE
#define WLAN_CIPHER_SUITE_GCMP_256		0x000FAC09
#define WLAN_CIPHER_SUITE_CCMP_256		0x000FAC0A
#define WLAN_CIPHER_SUITE_BIP_GMAC_128		0x000FAC0B
#define WLAN_CIPHER_SUITE_BIP_GMAC_256		0x000FAC0C
#define WLAN_CIPHER_SUITE_BIP_CMAC_256		0x000FAC0D
#endif

#if KERNEL_VERSION(4, 12, 0) > CFG80211_VERSION_CODE
#define WLAN_AKM_SUITE_8021X_SUITE_B		0x000FAC0B
#define WLAN_AKM_SUITE_8021X_SUITE_B_192	0x000FAC0C
#endif
#if (CFG_SUPPORT_SUPPLICANT_SME == 1)
#ifndef WLAN_AKM_SUITE_SAE
#define WLAN_AKM_SUITE_SAE		0x000FAC08
#endif
#endif
#if CFG_SUPPORT_OWE && \
	KERNEL_VERSION(5, 7, 0) > CFG80211_VERSION_CODE
#define WLAN_AKM_SUITE_OWE			0x000FAC12
#endif

#define IW_AUTH_CIPHER_GCMP256  0x00000080

/* EFUSE Auto Mode Support */
#define LOAD_EFUSE 0
#define LOAD_EEPROM_BIN 1
#define LOAD_AUTO 2

/*******************************************************************************
 *                             D A T A   T Y P E S
 *******************************************************************************
 */
struct GLUE_INFO;

struct GL_WPA_INFO {
	uint32_t u4WpaVersion;
	uint32_t u4KeyMgmt;
	uint32_t u4CipherGroup;
	uint32_t u4CipherPairwise;
	uint32_t u4AuthAlg;
	u_int8_t fgPrivacyInvoke;
#if CFG_SUPPORT_802_11W
	uint32_t u4CipherGroupMgmt;
	uint32_t u4Mfp;
	uint8_t ucRSNMfpCap;
#endif
	uint8_t aucKek[NL80211_KEK_LEN];
	uint8_t aucKck[NL80211_KCK_LEN];
	uint8_t aucReplayCtr[NL80211_REPLAY_CTR_LEN];
};

#if CFG_SUPPORT_REPLAY_DETECTION
struct GL_REPLEY_PN_INFO {
	uint8_t auPN[16];
	u_int8_t fgRekey;
	u_int8_t fgFirstPkt;
};
struct GL_DETECT_REPLAY_INFO {
	uint8_t ucCurKeyId;
	uint8_t ucKeyType;
	struct GL_REPLEY_PN_INFO arReplayPNInfo[4];
	uint32_t u4KeyLength;
	uint8_t aucKeyMaterial[32];
	u_int8_t fgPairwiseInstalled;
	u_int8_t fgKeyRscFresh;
};
#endif

#if CFG_SUPPORT_WOW_EINT
struct WOWLAN_DEV_NODE {
	struct sdio_func *func;
#if CFG_SUPPORT_WOW_EINT_KEYEVENT_WAKEUP
	KAL_WAKE_LOCK_T *pr_eint_wlock;
#endif

	unsigned int wowlan_irq;
	int wowlan_irqlevel;
	atomic_t irq_enable_count;
};
#endif

enum ENUM_NET_DEV_IDX {
	NET_DEV_WLAN_IDX = 0,
	NET_DEV_P2P_IDX,
	NET_DEV_BOW_IDX,
	NET_DEV_NUM
};

enum ENUM_RSSI_TRIGGER_TYPE {
	ENUM_RSSI_TRIGGER_NONE,
	ENUM_RSSI_TRIGGER_GREATER,
	ENUM_RSSI_TRIGGER_LESS,
	ENUM_RSSI_TRIGGER_TRIGGERED,
	ENUM_RSSI_TRIGGER_NUM
};

#if CFG_ENABLE_WIFI_DIRECT
enum ENUM_NET_REG_STATE {
	ENUM_NET_REG_STATE_UNREGISTERED,
	ENUM_NET_REG_STATE_REGISTERING,
	ENUM_NET_REG_STATE_REGISTERED,
	ENUM_NET_REG_STATE_UNREGISTERING,
	ENUM_NET_REG_STATE_NUM
};
#endif

enum ENUM_PKT_FLAG {
	ENUM_PKT_802_11,	/* 802.11 or non-802.11 */
	ENUM_PKT_802_3,		/* 802.3 or ethernetII */
	ENUM_PKT_1X,		/* 1x frame or not */
	ENUM_PKT_PROTECTED_1X,	/* protected 1x frame */
	ENUM_PKT_NON_PROTECTED_1X,	/* Non protected 1x frame */
	ENUM_PKT_VLAN_EXIST,	/* VLAN tag exist */
	ENUM_PKT_DHCP,		/* DHCP frame */
	ENUM_PKT_ARP,		/* ARP */
	ENUM_PKT_ICMP,		/* ICMP */
	ENUM_PKT_TDLS,		/* TDLS */
	ENUM_PKT_DNS,		/* DNS */
#if CFG_SUPPORT_TX_MGMT_USE_DATAQ
	ENUM_PKT_802_11_MGMT, /* 802.11 MGMT*/
#endif
	ENUM_PKT_TSO,		/* TSO */
#if CFG_SUPPORT_TPENHANCE_MODE
	ENUM_PKT_TCP_ACK,
#endif /* CFG_SUPPORT_TPENHANCE_MODE */

	ENUM_PKT_FLAG_NUM
};

enum ENUM_WLAN_DRV_BUF_TYPE_T {
	ENUM_BUF_TYPE_NVRAM,
	ENUM_BUF_TYPE_DRV_CFG,
	ENUM_BUF_TYPE_FW_CFG,
	ENUM_BUF_TYPE_NUM
};

enum ENUM_NVRAM_STATE {
	NVRAM_STATE_INIT = 0,
	NVRAM_STATE_READY, /*power on or update*/
	NVRAM_STATE_SEND_TO_FW,
	NVRAM_STATE_NUM
};

struct GL_IO_REQ {
	struct QUE_ENTRY rQueEntry;
	/* wait_queue_head_t       cmdwait_q; */
	u_int8_t fgRead;
	u_int8_t fgWaitResp;
	struct ADAPTER *prAdapter;
	PFN_OID_HANDLER_FUNC pfnOidHandler;
	void *pvInfoBuf;
	uint32_t u4InfoBufLen;
	uint32_t *pu4QryInfoLen;
	uint32_t rStatus;
	uint32_t u4Flag;
	uint8_t ucBssIndex;
};

#if CFG_ENABLE_BT_OVER_WIFI
struct GL_BOW_INFO {
	u_int8_t fgIsRegistered;
	dev_t u4DeviceNumber;	/* dynamic device number */
	/* struct kfifo *prKfifo; */ /* for buffering indicated events */
	struct kfifo rKfifo;	/* for buffering indicated events */
	spinlock_t rSpinLock;	/* spin lock for kfifo */
	struct cdev cdev;
	uint32_t u4FreqInKHz;	/* frequency */

	uint8_t aucRole[CFG_BOW_PHYSICAL_LINK_NUM];	/* 0: Responder,
							 * 1: Initiator
							 */
	enum ENUM_BOW_DEVICE_STATE
	aeState[CFG_BOW_PHYSICAL_LINK_NUM];
	uint8_t arPeerAddr[CFG_BOW_PHYSICAL_LINK_NUM][PARAM_MAC_ADDR_LEN];

	wait_queue_head_t outq;

#if CFG_BOW_SEPARATE_DATA_PATH
	/* Device handle */
	struct net_device *prDevHandler;
	u_int8_t fgIsNetRegistered;
#endif

};
#endif

#if CFG_SUPPORT_SCAN_CACHE_RESULT
struct GL_SCAN_CACHE_INFO {
	struct GLUE_INFO *prGlueInfo;

	/* for cfg80211 scan done indication */
	struct cfg80211_scan_request *prRequest;

	/* total number of channels to scan */
	uint32_t n_channels;

	/* Scan period time */
	OS_SYSTIME u4LastScanTime;

	/* Bss index */
	uint8_t ucBssIndex;
};
#endif /* CFG_SUPPORT_SCAN_CACHE_RESULT */

#if CFG_SUPPORT_PERF_IND
	struct GL_PERF_IND_INFO {
		uint32_t u4CurTxBytes[BSSID_NUM]; /* Byte */
		uint32_t u4CurRxBytes[BSSID_NUM]; /* Byte */
		uint16_t u2CurRxRate[BSSID_NUM]; /* Unit 500 Kbps */
		uint8_t ucCurRxRCPI0[BSSID_NUM];
		uint8_t ucCurRxRCPI1[BSSID_NUM];
		uint8_t ucCurRxNss[BSSID_NUM];
	};
#endif /* CFG_SUPPORT_SCAN_CACHE_RESULT */

struct FT_IES {
	uint16_t u2MDID;
	struct IE_MOBILITY_DOMAIN *prMDIE;
	struct IE_FAST_TRANSITION *prFTIE;
	struct IE_TIMEOUT_INTERVAL *prTIE;
	struct RSN_INFO_ELEM *prRsnIE;
	uint8_t *pucIEBuf;
	uint32_t u4IeLength;
};

/*
 * type definition of pointer to p2p structure
 */
struct GL_P2P_INFO;	/* declare GL_P2P_INFO_T */
struct GL_P2P_DEV_INFO;	/* declare GL_P2P_DEV_INFO_T */

struct GLUE_INFO {
	/* Device handle */
	struct net_device *prDevHandler;

	/* Device */
	struct device *prDev;

	/* Device Index(index of arWlanDevInfo[]) */
	int32_t i4DevIdx;

	/* Device statistics */
	/* struct net_device_stats rNetDevStats; */

	/* Wireless statistics struct net_device */
	struct iw_statistics rIwStats;

	/* spinlock to sync power save mechanism */
	spinlock_t rSpinLock[SPIN_LOCK_NUM];

	/* Mutex to protect interruptible section */
	struct mutex arMutex[MUTEX_NUM];

	/* semaphore for ioctl */
	struct semaphore ioctl_sem;

	uint64_t u8Cookie;

	unsigned long ulFlag;		/* GLUE_FLAG_XXX */
	uint32_t u4PendFlag;
	/* UINT_32 u4TimeoutFlag; */
	uint32_t u4OidCompleteFlag;
	uint32_t u4ReadyFlag;	/* check if card is ready */

	uint32_t u4OsMgmtFrameFilter;

	/* Number of pending frames, also used for debuging if any frame is
	 * missing during the process of unloading Driver.
	 *
	 * NOTE(Kevin): In Linux, we also use this variable as the threshold
	 * for manipulating the netif_stop(wake)_queue() func.
	 */
	int32_t ai4TxPendingFrameNumPerQueue[MAX_BSSID_NUM][CFG_MAX_TXQ_NUM];
	int32_t i4TxPendingFrameNum;
	int32_t i4TxPendingSecurityFrameNum;
	int32_t i4TxPendingCmdNum;

	/* Tx: for NetDev to BSS index mapping */
	struct NET_INTERFACE_INFO arNetInterfaceInfo[MAX_BSSID_NUM];

	/* Rx: for BSS index to NetDev mapping */
	/* P_NET_INTERFACE_INFO_T  aprBssIdxToNetInterfaceInfo[HW_BSSID_NUM]; */

	/* current IO request for kalIoctl */
	struct GL_IO_REQ OidEntry;

	/* registry info */
	struct REG_INFO rRegInfo;

	/* firmware */
	struct firmware *prFw;

	/* Host interface related information */
	/* defined in related hif header file */
	struct GL_HIF_INFO rHifInfo;

	/*! \brief wext wpa related information */
	struct GL_WPA_INFO rWpaInfo[KAL_BSS_NUM];
#if CFG_SUPPORT_REPLAY_DETECTION
	struct GL_DETECT_REPLAY_INFO prDetRplyInfo[KAL_AIS_NUM];
#endif

	/* Pointer to ADAPTER_T - main data structure of internal protocol
	 * stack
	 */
	struct ADAPTER *prAdapter;

#if WLAN_INCLUDE_PROC
	struct proc_dir_entry *pProcRoot;
#endif				/* WLAN_INCLUDE_PROC */

	/* Indicated media state */
	enum ENUM_PARAM_MEDIA_STATE
		eParamMediaStateIndicated[KAL_AIS_NUM];

	/* Device power state D0~D3 */
	enum PARAM_DEVICE_POWER_STATE ePowerState;

	struct completion rScanComp;	/* indicate scan complete */
	struct completion
		rHaltComp;	/* indicate main thread halt complete */
	struct completion
		rPendComp;	/* indicate main thread halt complete */
#if CFG_SUPPORT_MULTITHREAD
	struct completion
		rHifHaltComp;	/* indicate hif_thread halt complete */
	struct completion
		rRxHaltComp;	/* indicate hif_thread halt complete */

	uint32_t u4TxThreadPid;
	uint32_t u4RxThreadPid;
	uint32_t u4HifThreadPid;
#endif

#if CFG_SUPPORT_NCHO
	struct completion
		rAisChGrntComp;	/* indicate Ais channel grant complete */
#endif

	uint32_t rPendStatus;

	struct QUE rTxQueue;

	/* OID related */
	struct QUE rCmdQueue;
	/* PVOID                   pvInformationBuffer; */
	/* UINT_32                 u4InformationBufferLength; */
	/* PVOID                   pvOidEntry; */
	/* PUINT_8                 pucIOReqBuff; */
	/* QUE_T                   rIOReqQueue; */
	/* QUE_T                   rFreeIOReqQueue; */

	wait_queue_head_t waitq;
	struct task_struct *main_thread;

#if CFG_SUPPORT_MULTITHREAD
	wait_queue_head_t waitq_hif;
	struct task_struct *hif_thread;

	wait_queue_head_t waitq_rx;
	struct task_struct *rx_thread;

#endif
	struct tasklet_struct rRxTask;
#if (CFG_SUPPORT_RETURN_TASK == 1)
	struct tasklet_struct rRxRfbRetTask;
#endif
	struct tasklet_struct rTxMsduRetTask;
	struct tasklet_struct rTxCompleteTask;
#if CFG_SUPPORT_MULTITHREAD
	struct work_struct rTxMsduFreeWork;
#endif
	struct delayed_work rRxPktDeAggWork;

	struct timer_list tickfn;

#if CFG_SUPPORT_EXT_CONFIG
	uint16_t au2ExtCfg[256];	/* NVRAM data buffer */
	uint32_t u4ExtCfgLength;	/* 0 means data is NOT valid */
#endif

#if CFG_ENABLE_BT_OVER_WIFI
	struct GL_BOW_INFO rBowInfo;
#endif

#if CFG_ENABLE_WIFI_DIRECT
	struct GL_P2P_DEV_INFO *prP2PDevInfo;
	struct GL_P2P_INFO *prP2PInfo[KAL_P2P_NUM];
#if CFG_SUPPORT_P2P_RSSI_QUERY
	/* Wireless statistics struct net_device */
	struct iw_statistics rP2pIwStats;
#endif
#endif
#if CFG_SUPPORT_NAN
	struct _GL_NAN_INFO_T *aprNANDevInfo[NAN_BSS_INDEX_NUM];
#endif

	/* NVRAM availability */
	u_int8_t fgNvramAvailable;

	u_int8_t fgMcrAccessAllowed;

	/* MAC Address Overridden by IOCTL */
	u_int8_t fgIsMacAddrOverride;
	uint8_t rMacAddrOverride[PARAM_MAC_ADDR_LEN];

	struct SET_TXPWR_CTRL rTxPwr;

	/* for cfg80211 scan done indication */
	struct cfg80211_scan_request *prScanRequest;

#if CFG_SUPPORT_SCHED_SCAN
	struct PARAM_SCHED_SCAN_REQUEST *prSchedScanRequest;
#else
	/* for cfg80211 scheduled scan */
	struct cfg80211_sched_scan_request *prSchedScanRequest;
#endif

	/* to indicate registered or not */
	u_int8_t fgIsRegistered;

#if CFG_SUPPORT_SDIO_READ_WRITE_PATTERN
	u_int8_t fgEnSdioTestPattern;
	u_int8_t fgSdioReadWriteMode;
	u_int8_t fgIsSdioTestInitialized;
	uint8_t aucSdioTestBuffer[256];
#endif

	u_int8_t fgIsInSuspendMode;

#if (CFG_SUPPORT_SUPPLICANT_MBO == 1)
	uint8_t aucSupOpClassIE[200];    /*for Assoc req */
	uint16_t u2SupOpClassIELen;
	uint8_t aucMboIE[200];    /*for Assoc req */
	uint16_t u2MboIELen;
#endif

#if CFG_SUPPORT_PASSPOINT
	u_int8_t fgIsDad;
	uint8_t aucDADipv4[4];
	u_int8_t fgIs6Dad;
	uint8_t aucDADipv6[16];
#endif				/* CFG_SUPPORT_PASSPOINT */

	KAL_WAKE_LOCK_T *prIntrWakeLock;
	KAL_WAKE_LOCK_T *prTimeoutWakeLock;

#if CFG_MET_PACKET_TRACE_SUPPORT
	u_int8_t fgMetProfilingEn;
	uint16_t u2MetUdpPort;
#endif

#if (CFG_SUPPORT_SNIFFER_RADIOTAP == 1)
	uint8_t fgIsEnableMon;
	uint8_t ucPriChannel;
	uint8_t ucChannelS1;
	uint8_t ucChannelS2;
	uint8_t ucBand;
	uint8_t ucChannelWidth;
	uint8_t ucSco;
	uint8_t ucBandIdx;
	uint8_t fgDropFcsErrorFrame;
	uint8_t aucBandIdxEn[CFG_MONITOR_BAND_NUM];
	uint16_t u2Aid;
	uint32_t u4AmpduRefNum;
#endif

	int32_t i4RssiCache;
	uint32_t u4LinkSpeedCache;

#if CFG_AP_80211KVR_INTERFACE
	struct delayed_work rChanNoiseControlWork;
	struct delayed_work rChanNoiseGetInfoWork;
#endif

	uint32_t u4InfType;

	uint32_t IsrCnt;
	uint32_t IsrPassCnt;
	uint32_t TaskIsrCnt;

	uint32_t IsrAbnormalCnt;
	uint32_t IsrSoftWareCnt;
	uint32_t IsrTxCnt;
	uint32_t IsrRxCnt;
	uint64_t u8HifIntTime;

	/* save partial scan channel information */
	/* PARTIAL_SCAN_INFO rScanChannelInfo; */
	uint8_t *pucScanChannel;

#if CFG_SUPPORT_SCAN_CACHE_RESULT
	struct GL_SCAN_CACHE_INFO scanCache;
#endif /* CFG_SUPPORT_SCAN_CACHE_RESULT */
#if (CFG_SUPPORT_PERF_IND == 1)
	struct GL_PERF_IND_INFO PerfIndCache;
#endif

	/* Full2Partial */
	OS_SYSTIME u4LastFullScanTime;
	/* full scan or partial scan */
	uint8_t ucTrScanType;
	/* UINT_8 aucChannelNum[FULL_SCAN_MAX_CHANNEL_NUM]; */
	/* PARTIAL_SCAN_INFO rFullScanApChannel; */
	uint8_t *pucFullScan2PartialChannel;

	uint32_t u4RoamFailCnt;
	uint64_t u8RoamFailTime;
	u_int8_t fgTxDoneDelayIsARP;
	uint32_t u4ArriveDrvTick;
	uint32_t u4EnQueTick;
	uint32_t u4DeQueTick;
	uint32_t u4LeaveDrvTick;
	uint32_t u4CurrTick;
	uint64_t u8CurrTime;

	/* FW Roaming */
	/* store the FW roaming enable state which FWK determines */
	/* if it's = 0, ignore the black/whitelists settings from FWK */
	uint32_t u4FWRoamingEnable;

	/*service for test mode*/
#if (CONFIG_WLAN_SERVICE == 1)
	struct service rService;
#endif

#if CFG_SUPPORT_RX_GRO
	spinlock_t napi_spinlock;
#endif

#if CFG_CHIP_RESET_SUPPORT
	struct work_struct rWfsysResetWork;    /* work for Wfsys L0.5 reset  */
#endif

#if CFG_ASSERT_DUMP
	wait_queue_head_t waitq_fwdump;
	struct sk_buff_head rFwDumpSkbQueue;
#endif
#if CFG_SUPPORT_RX_NAPI
	struct napi_struct *prRxDirectNapi;
	struct kfifo rRxKfifoQ;
	uint8_t *prRxKfifoBuf;
	uint32_t u4RxKfifoBufLen;
#endif
	struct kfifo rTxMsduRetFifo;
	uint8_t *prTxMsduRetFifoBuf;
	uint32_t u4TxMsduRetFifoLen;

#if (CFG_KEEPFULLPOWER_BEFORE_TWT_NEGO == 1)
	uint8_t ucKeepFullPwrBackup;
#endif

#if CFG_SUPPORT_NAN
	struct sock *NetLinkSK;
#endif

#if CFG_SUPPORT_TPENHANCE_MODE
	/* Tp Enhance */
	struct QUE rTpeAckQueue;
	uint32_t u4TpeMaxPktNum;
	uint64_t u8TpeTimestamp;
	uint32_t u4TpeTimeout;
	struct timer_list rTpeTimer;
#endif /* CFG_SUPPORT_TPENHANCE_MODE */
};

typedef irqreturn_t(*PFN_WLANISR) (int irq, void *dev_id,
				   struct pt_regs *regs);

typedef void (*PFN_LINUX_TIMER_FUNC) (unsigned long);

/* generic sub module init/exit handler
 *   now, we only have one sub module, p2p
 */
#if CFG_ENABLE_WIFI_DIRECT
typedef u_int8_t(*SUB_MODULE_INIT) (struct GLUE_INFO
				    *prGlueInfo);
typedef u_int8_t(*SUB_MODULE_EXIT) (struct GLUE_INFO
				    *prGlueInfo);

struct SUB_MODULE_HANDLER {
	SUB_MODULE_INIT subModInit;
	SUB_MODULE_EXIT subModExit;
	u_int8_t fgIsInited;
};

#endif

#ifdef CONFIG_NL80211_TESTMODE

enum TestModeCmdType {
	TESTMODE_CMD_ID_SW_CMD = 1,
	TESTMODE_CMD_ID_WAPI = 2,
	TESTMODE_CMD_ID_HS20 = 3,

	/* Hotspot managerment testmode command */
	TESTMODE_CMD_ID_HS_CONFIG = 51,

	TESTMODE_CMD_ID_STR_CMD = 102,
	NUM_OF_TESTMODE_CMD_ID
};

#if CFG_SUPPORT_PASSPOINT
enum Hs20CmdType {
	HS20_CMD_ID_SET_BSSID_POOL = 0,
	NUM_OF_HS20_CMD_ID
};
#endif /* CFG_SUPPORT_PASSPOINT */

struct NL80211_DRIVER_TEST_MODE_PARAMS {
	uint32_t index;
	uint32_t buflen;
};

/*SW CMD */
struct NL80211_DRIVER_SW_CMD_PARAMS {
	struct NL80211_DRIVER_TEST_MODE_PARAMS hdr;
	uint8_t set;
	uint32_t adr;
	uint32_t data;
};

struct iw_encode_exts {
	__u32 ext_flags;	/*!< IW_ENCODE_EXT_* */
	__u8 tx_seq[IW_ENCODE_SEQ_MAX_SIZE];	/*!< LSB first */
	__u8 rx_seq[IW_ENCODE_SEQ_MAX_SIZE];	/*!< LSB first */
	/*!< ff:ff:ff:ff:ff:ff for broadcast/multicast
	 *   (group) keys or unicast address for
	 *   individual keys
	 */
	__u8 addr[MAC_ADDR_LEN];
	__u16 alg;		/*!< IW_ENCODE_ALG_* */
	__u16 key_len;
	__u8 key[32];
};

/*SET KEY EXT */
struct NL80211_DRIVER_SET_KEY_EXTS {
	struct NL80211_DRIVER_TEST_MODE_PARAMS hdr;
	uint8_t key_index;
	uint8_t key_len;
	struct iw_encode_exts ext;
};

#if CFG_SUPPORT_PASSPOINT

struct param_hs20_set_bssid_pool {
	uint8_t fgBssidPoolIsEnable;
	uint8_t ucNumBssidPool;
	uint8_t arBssidPool[8][ETH_ALEN];
};

struct wpa_driver_hs20_data_s {
	struct NL80211_DRIVER_TEST_MODE_PARAMS hdr;
	enum Hs20CmdType CmdType;
	struct param_hs20_set_bssid_pool hs20_set_bssid_pool;
};

#endif /* CFG_SUPPORT_PASSPOINT */

#endif

struct NETDEV_PRIVATE_GLUE_INFO {
	struct GLUE_INFO *prGlueInfo;
	uint8_t ucBssIdx;
#if CFG_ENABLE_UNIFY_WIPHY
	u_int8_t ucIsP2p;
#endif
#if CFG_SUPPORT_RX_GRO
	struct napi_struct napi;
	OS_SYSTIME tmGROFlushTimeout;
	struct sk_buff_head rRxNapiSkbQ;
#endif
#if CFG_SUPPORT_NAN
	unsigned char ucIsNan;
#endif
};

struct PACKET_PRIVATE_DATA {
	/* tx/rx both use cb */
	struct QUE_ENTRY rQueEntry;  /* 16byte total:16 */

	uint8_t ucBssIdx;	/* 1byte */
	/* only rx use cb */
	u_int8_t fgIsIndependentPkt; /* 1byte */
	/* only tx use cb */
	uint8_t ucTid;		/* 1byte */
	uint8_t ucHeaderLen;	/* 1byte */
	uint8_t ucProfilingFlag;	/* 1byte */
	uint8_t ucSeqNo;		/* 1byte */
	uint16_t u2Flag;		/* 2byte total:24 */

	uint16_t u2IpId;		/* 2byte */
	uint16_t u2FrameLen;	/* 2byte */
	OS_SYSTIME rArrivalTime;/* 4byte total:32 */

	uint64_t u8ArriveTime;	/* 8byte total:40 */
#if CFG_SUPPORT_TX_MGMT_USE_DATAQ
	uint64_t u8Cookie;		/* 8byte total:48 */
#endif
};

struct PACKET_PRIVATE_RX_DATA {
	uint64_t u8IntTime;	/* 8byte */
	uint64_t u8RxTime;	/* 8byte */
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
/*----------------------------------------------------------------------------*/
/* Macros of SPIN LOCK operations for using in Glue Layer                     */
/*----------------------------------------------------------------------------*/
#if CFG_USE_SPIN_LOCK_BOTTOM_HALF
#define GLUE_SPIN_LOCK_DECLARATION()
#define GLUE_ACQUIRE_SPIN_LOCK(prGlueInfo, rLockCategory)   \
	{ \
		if (rLockCategory < SPIN_LOCK_NUM) \
			spin_lock_bh(&(prGlueInfo->rSpinLock[rLockCategory])); \
	}
#define GLUE_RELEASE_SPIN_LOCK(prGlueInfo, rLockCategory)   \
	{ \
		if (rLockCategory < SPIN_LOCK_NUM) \
			spin_unlock_bh(\
			&(prGlueInfo->rSpinLock[rLockCategory])); \
	}
#else /* !CFG_USE_SPIN_LOCK_BOTTOM_HALF */
#define GLUE_SPIN_LOCK_DECLARATION() unsigned long __ulFlags = 0
#define GLUE_ACQUIRE_SPIN_LOCK(prGlueInfo, rLockCategory)   \
	{ \
		if (rLockCategory < SPIN_LOCK_NUM) \
			spin_lock_irqsave(\
			&(prGlueInfo)->rSpinLock[rLockCategory], __ulFlags); \
	}
#define GLUE_RELEASE_SPIN_LOCK(prGlueInfo, rLockCategory)   \
	{ \
		if (rLockCategory < SPIN_LOCK_NUM) \
			spin_unlock_irqrestore(\
			&(prGlueInfo->rSpinLock[rLockCategory]), __ulFlags); \
	}
#endif /* !CFG_USE_SPIN_LOCK_BOTTOM_HALF */

/*----------------------------------------------------------------------------*/
/* Macros for accessing Reserved Fields of native packet                      */
/*----------------------------------------------------------------------------*/

#define GLUE_GET_PKT_PRIVATE_DATA(_p) \
	((struct PACKET_PRIVATE_DATA *)(&(((struct sk_buff *)(_p))->cb[0])))

#define GLUE_GET_PKT_QUEUE_ENTRY(_p)    \
	    (&(GLUE_GET_PKT_PRIVATE_DATA(_p)->rQueEntry))

#define GLUE_GET_PKT_DESCRIPTOR(_prQueueEntry)  \
	    ((void *) (((unsigned long)_prQueueEntry) \
	    - offsetof(struct sk_buff, cb[0])))

#define GLUE_SET_PKT_TID(_p, _tid) \
	    (GLUE_GET_PKT_PRIVATE_DATA(_p)->ucTid = (uint8_t)(_tid))

#define GLUE_GET_PKT_TID(_p) \
	    (GLUE_GET_PKT_PRIVATE_DATA(_p)->ucTid)

#define GLUE_SET_PKT_FLAG(_p, _flag) \
	(GLUE_GET_PKT_PRIVATE_DATA(_p)->u2Flag |= BIT(_flag))

#define GLUE_TEST_PKT_FLAG(_p, _flag) \
	(GLUE_GET_PKT_PRIVATE_DATA(_p)->u2Flag & BIT(_flag))

#define GLUE_IS_PKT_FLAG_SET(_p) \
	(GLUE_GET_PKT_PRIVATE_DATA(_p)->u2Flag)

#define GLUE_SET_PKT_BSS_IDX(_p, _ucBssIndex) \
	    (GLUE_GET_PKT_PRIVATE_DATA(_p)->ucBssIdx = (uint8_t)(_ucBssIndex))

#define GLUE_GET_PKT_BSS_IDX(_p) \
	    (GLUE_GET_PKT_PRIVATE_DATA(_p)->ucBssIdx)

#define GLUE_SET_PKT_HEADER_LEN(_p, _ucMacHeaderLen) \
	    (GLUE_GET_PKT_PRIVATE_DATA(_p)->ucHeaderLen = \
	    (uint8_t)(_ucMacHeaderLen))

#define GLUE_GET_PKT_HEADER_LEN(_p) \
	    (GLUE_GET_PKT_PRIVATE_DATA(_p)->ucHeaderLen)

#define GLUE_SET_PKT_FRAME_LEN(_p, _u2PayloadLen) \
	(GLUE_GET_PKT_PRIVATE_DATA(_p)->u2FrameLen = (uint16_t)(_u2PayloadLen))

#define GLUE_GET_PKT_FRAME_LEN(_p) \
	    (GLUE_GET_PKT_PRIVATE_DATA(_p)->u2FrameLen)

#define GLUE_SET_PKT_ARRIVAL_TIME(_p, _rSysTime) \
	(GLUE_GET_PKT_PRIVATE_DATA(_p)->rArrivalTime = (OS_SYSTIME)(_rSysTime))

#define GLUE_GET_PKT_ARRIVAL_TIME(_p)    \
	    (GLUE_GET_PKT_PRIVATE_DATA(_p)->rArrivalTime)

#define GLUE_SET_PKT_IP_ID(_p, _u2IpId) \
	(GLUE_GET_PKT_PRIVATE_DATA(_p)->u2IpId = (uint16_t)(_u2IpId))

#define GLUE_GET_PKT_IP_ID(_p) \
	    (GLUE_GET_PKT_PRIVATE_DATA(_p)->u2IpId)

#if CFG_SUPPORT_TX_MGMT_USE_DATAQ
#define GLUE_SET_PKT_COOKIE(_p, _u8Cookie) \
	(GLUE_GET_PKT_PRIVATE_DATA(_p)->u8Cookie = (uint64_t)(_u8Cookie))

#define GLUE_GET_PKT_COOKIE(_p) \
	    (GLUE_GET_PKT_PRIVATE_DATA(_p)->u8Cookie)
#endif

#define GLUE_SET_PKT_SEQ_NO(_p, _ucSeqNo) \
	(GLUE_GET_PKT_PRIVATE_DATA(_p)->ucSeqNo = (uint8_t)(_ucSeqNo))

#define GLUE_GET_PKT_SEQ_NO(_p) \
	    (GLUE_GET_PKT_PRIVATE_DATA(_p)->ucSeqNo)

#define GLUE_SET_PKT_FLAG_PROF_MET(_p) \
	    (GLUE_GET_PKT_PRIVATE_DATA(_p)->ucProfilingFlag |= BIT(0))

#define GLUE_GET_PKT_IS_PROF_MET(_p) \
	    (GLUE_GET_PKT_PRIVATE_DATA(_p)->ucProfilingFlag & BIT(0))

#define GLUE_SET_PKT_XTIME(_p, _rSysTime) \
	(GLUE_GET_PKT_PRIVATE_DATA(_p)->u8ArriveTime = (uint64_t)(_rSysTime))

#define GLUE_GET_PKT_XTIME(_p)    \
	(GLUE_GET_PKT_PRIVATE_DATA(_p)->u8ArriveTime)

#define GLUE_GET_PKT_PRIVATE_RX_DATA(_p) \
	((struct PACKET_PRIVATE_RX_DATA *)(&(((struct sk_buff *)(_p))->cb[24])))

#define GLUE_RX_SET_PKT_INT_TIME(_p, _rTime) \
	(GLUE_GET_PKT_PRIVATE_RX_DATA(_p)->u8IntTime = (uint64_t)(_rTime))

#define GLUE_RX_GET_PKT_INT_TIME(_p) \
	(GLUE_GET_PKT_PRIVATE_RX_DATA(_p)->u8IntTime)

#define GLUE_RX_SET_PKT_RX_TIME(_p, _rTime) \
	(GLUE_GET_PKT_PRIVATE_RX_DATA(_p)->u8RxTime = (uint64_t)(_rTime))

#define GLUE_RX_GET_PKT_RX_TIME(_p) \
	(GLUE_GET_PKT_PRIVATE_RX_DATA(_p)->u8RxTime)

#define GLUE_GET_PKT_ETHER_DEST_ADDR(_p)    \
	    ((uint8_t *)&(((struct sk_buff *)(_p))->data))

/* Check validity of prDev, private data, and pointers */
#define GLUE_CHK_DEV(prDev) \
	((prDev && *((struct GLUE_INFO **) netdev_priv(prDev))) ? TRUE : FALSE)

#define GLUE_CHK_PR2(prDev, pr2) \
	((GLUE_CHK_DEV(prDev) && pr2) ? TRUE : FALSE)

#define GLUE_CHK_PR3(prDev, pr2, pr3) \
	((GLUE_CHK_PR2(prDev, pr2) && pr3) ? TRUE : FALSE)

#define GLUE_CHK_PR4(prDev, pr2, pr3, pr4) \
	((GLUE_CHK_PR3(prDev, pr2, pr3) && pr4) ? TRUE : FALSE)

#define GLUE_SET_EVENT(pr) \
	kalSetEvent(pr)

/* For legacy kernel, create new MACRO by ourself
 * Otherwise, may use __atomic_op_fence()
 */
#define GLUE_FENCE_ATOMIC(op, args...) \
({ \
	typeof(op(args)) __ret;	 \
	KAL_MB_RW(); \
	__ret = op(args); \
	KAL_MB_RW(); \
	__ret; \
})
#define GLUE_INC_REF_CNT(_refCount) \
	GLUE_FENCE_ATOMIC(atomic_add_return, 1, ((atomic_t *)&(_refCount)))
#define GLUE_DEC_REF_CNT(_refCount) \
	GLUE_FENCE_ATOMIC(atomic_sub_return, 1, ((atomic_t *)&(_refCount)))
#define GLUE_GET_REF_CNT(_refCount)  \
	GLUE_FENCE_ATOMIC(atomic_read, ((atomic_t *)&(_refCount)))

#define DbgPrint(...)

#if (CFG_SUPPORT_CONNAC3X == 1)

#if KERNEL_VERSION(5, 7, 0) <= LINUX_VERSION_CODE
#define GLUE_LOOKUP_FUN(fun_name)       NULL
#else
#ifdef CONFIG_KALLSYMS
#define GLUE_LOOKUP_FUN(fun_name)       kallsyms_lookup_name
#else
#define GLUE_LOOKUP_FUN(fun_name)       NULL
#endif
#endif

#endif /* (CFG_SUPPORT_CONNAC3X == 1) */

#define GLUE_SYMBOL_GET(fun_name)	__symbol_get(fun_name)
#define GLUE_SYMBOL_PUT(fun_name)	__symbol_put(fun_name)

#if CFG_MET_TAG_SUPPORT
#define GL_MET_TAG_START(_id, _name)	met_tag_start(_id, _name)
#define GL_MET_TAG_END(_id, _name)	met_tag_end(_id, _name)
#define GL_MET_TAG_ONESHOT(_id, _name, _value) \
	met_tag_oneshot(_id, _name, _value)
#define GL_MET_TAG_DISABLE(_id)		met_tag_disable(_id)
#define GL_MET_TAG_ENABLE(_id)		met_tag_enable(_id)
#define GL_MET_TAG_REC_ON()		met_tag_record_on()
#define GL_MET_TAG_REC_OFF()		met_tag_record_off()
#define GL_MET_TAG_INIT()		met_tag_init()
#define GL_MET_TAG_UNINIT()		met_tag_uninit()
#else
#define GL_MET_TAG_START(_id, _name)
#define GL_MET_TAG_END(_id, _name)
#define GL_MET_TAG_ONESHOT(_id, _name, _value)
#define GL_MET_TAG_DISABLE(_id)
#define GL_MET_TAG_ENABLE(_id)
#define GL_MET_TAG_REC_ON()
#define GL_MET_TAG_REC_OFF()
#define GL_MET_TAG_INIT()
#define GL_MET_TAG_UNINIT()
#endif

#define MET_TAG_ID	0

/*----------------------------------------------------------------------------*/
/* Macros of Data Type Check                                                  */
/*----------------------------------------------------------------------------*/
/* Kevin: we don't have to call following function to inspect the data
 * structure.
 * It will check automatically while at compile time.
 */
static __KAL_INLINE__ void glPacketDataTypeCheck(void)
{
	DATA_STRUCT_INSPECTING_ASSERT(sizeof(struct
		PACKET_PRIVATE_DATA) <= sizeof(((struct sk_buff *) 0)->cb));
}

static inline u16 mtk_wlan_ndev_select_queue(
	struct sk_buff *skb)
{
	static u16 ieee8021d_to_queue[8] = { 1, 0, 0, 1, 2, 2, 3, 3 };

	/* cfg80211_classify8021d returns 0~7 */
#if KERNEL_VERSION(3, 14, 0) > CFG80211_VERSION_CODE
	skb->priority = cfg80211_classify8021d(skb);
#else
	skb->priority = cfg80211_classify8021d(skb, NULL);
#endif
	return ieee8021d_to_queue[skb->priority];
}

#if KERNEL_VERSION(2, 6, 34) > LINUX_VERSION_CODE
#define netdev_for_each_mc_addr(mclist, dev) \
	for (mclist = dev->mc_list; mclist; mclist = mclist->next)
#endif

#if KERNEL_VERSION(2, 6, 34) > LINUX_VERSION_CODE
#define GET_ADDR(ha) (ha->da_addr)
#else
#define GET_ADDR(ha) (ha->addr)
#endif

#if KERNEL_VERSION(2, 6, 35) <= LINUX_VERSION_CODE
#define LIST_FOR_EACH_IPV6_ADDR(_prIfa, _ip6_ptr) \
	list_for_each_entry(_prIfa, &((struct inet6_dev *) \
	_ip6_ptr)->addr_list, if_list)
#else
#define LIST_FOR_EACH_IPV6_ADDR(_prIfa, _ip6_ptr) \
	for (_prIfa = ((struct inet6_dev *) _ip6_ptr)->addr_list; _prIfa; \
	_prIfa = _prIfa->if_next)
#endif

/*******************************************************************************
 *                  F U N C T I O N   D E C L A R A T I O N S
 *******************************************************************************
 */
#if WLAN_INCLUDE_PROC
int32_t procCreateFsEntry(struct GLUE_INFO *prGlueInfo);
int32_t procRemoveProcfs(void);


int32_t procInitFs(void);
int32_t procUninitProcFs(void);



int32_t procInitProcfs(struct net_device *prDev,
		       char *pucDevName);
#endif /* WLAN_INCLUDE_PROC */

#if WLAN_INCLUDE_SYS
int32_t sysCreateFsEntry(struct GLUE_INFO *prGlueInfo);
int32_t sysRemoveSysfs(void);
int32_t sysInitFs(void);
int32_t sysUninitSysFs(void);
void sysMacAddrOverride(uint8_t *prMacAddr);
#endif /* WLAN_INCLUDE_SYS */

#if CFG_ENABLE_BT_OVER_WIFI
u_int8_t glRegisterAmpc(struct GLUE_INFO *prGlueInfo);

u_int8_t glUnregisterAmpc(struct GLUE_INFO *prGlueInfo);
#endif

#if CFG_ENABLE_WIFI_DIRECT
void p2pSetMulticastListWorkQueueWrapper(struct GLUE_INFO
		*prGlueInfo);
#endif

struct GLUE_INFO *wlanGetGlueInfo(void);

#if KERNEL_VERSION(5, 2, 0) <= LINUX_VERSION_CODE
u16 wlanSelectQueue(struct net_device *dev,
		    struct sk_buff *skb,
		    struct net_device *sb_dev);
#elif KERNEL_VERSION(4, 19, 0) <= LINUX_VERSION_CODE
u16 wlanSelectQueue(struct net_device *dev,
		    struct sk_buff *skb,
		    struct net_device *sb_dev,
		    select_queue_fallback_t fallback);
#elif KERNEL_VERSION(3, 14, 0) <= LINUX_VERSION_CODE
u16 wlanSelectQueue(struct net_device *dev,
		    struct sk_buff *skb,
		    void *accel_priv, select_queue_fallback_t fallback);
#elif KERNEL_VERSION(3, 13, 0) <= LINUX_VERSION_CODE
u16 wlanSelectQueue(struct net_device *dev,
		    struct sk_buff *skb,
		    void *accel_priv);
#else
u16 wlanSelectQueue(struct net_device *dev,
		    struct sk_buff *skb);
#endif

void wlanDebugInit(void);

uint32_t wlanSetDriverDbgLevel(IN uint32_t u4DbgIdx,
			       IN uint32_t u4DbgMask);

uint32_t wlanGetDriverDbgLevel(IN uint32_t u4DbgIdx,
			       OUT uint32_t *pu4DbgMask);

void wlanSetSuspendMode(struct GLUE_INFO *prGlueInfo,
			u_int8_t fgEnable);
#if CFG_SUPPORT_CFG_FILE
void wlanGetConfig(struct ADAPTER *prAdapter);
#endif

uint32_t wlanDownloadBufferBin(struct ADAPTER *prAdapter);

uint32_t wlanConnacDownloadBufferBin(struct ADAPTER
				     *prAdapter);

uint32_t wlanConnac2XDownloadBufferBin(struct ADAPTER
					*prAdapter);

#if CFG_CHIP_RESET_SUPPORT
int32_t wlanOffAtReset(void);

int32_t wlanOnAtReset(void);
#endif

/*******************************************************************************
 *			 E X T E R N A L   F U N C T I O N S / V A R I A B L E
 *******************************************************************************
 */
extern struct net_device *gPrP2pDev[KAL_P2P_NUM];
extern struct net_device *gPrDev;
extern struct wireless_dev *gprWdev[KAL_AIS_NUM];
extern uint32_t g_u4DevIdx[KAL_P2P_NUM];

#ifdef CFG_DRIVER_INF_NAME_CHANGE
extern char *gprifnameap;
extern char *gprifnamep2p;
extern char *gprifnamesta;
#endif /* CFG_DRIVER_INF_NAME_CHANGE */

void wlanRegisterInetAddrNotifier(void);
void wlanUnregisterInetAddrNotifier(void);
void wlanRegisterNetdevNotifier(void);
void wlanUnregisterNetdevNotifier(void);
#if CFG_POWER_OFF_CTRL_SUPPORT
extern void wlanRegisterRebootNotifier(void);
extern void wlanUnregisterRebootNotifier(void);
#endif

#if CFG_MTK_ANDROID_WMT
typedef int (*set_p2p_mode) (struct net_device *netdev,
			     struct PARAM_CUSTOM_P2P_SET_STRUCT p2pmode);
extern void register_set_p2p_mode_handler(
	set_p2p_mode handler);
#endif

#if CFG_ENABLE_EARLY_SUSPEND
extern int glRegisterEarlySuspend(struct early_suspend
				  *prDesc,
				  early_suspend_callback wlanSuspend,
				  late_resume_callback wlanResume);

extern int glUnregisterEarlySuspend(struct early_suspend
				    *prDesc);
#endif

#if CFG_MET_PACKET_TRACE_SUPPORT
void kalMetTagPacket(IN struct GLUE_INFO *prGlueInfo,
		     IN void *prPacket, IN enum ENUM_TX_PROFILING_TAG eTag);

void kalMetInit(IN struct GLUE_INFO *prGlueInfo);
#endif

void wlanUpdateChannelTable(struct GLUE_INFO *prGlueInfo);

#if CFG_SUPPORT_SAP_DFS_CHANNEL
void wlanUpdateDfsChannelTable(struct GLUE_INFO *prGlueInfo,
		uint8_t ucRoleIdx, uint8_t ucChannel, uint8_t ucBandWidth,
		enum ENUM_CHNL_EXT eBssSCO, uint32_t u4CenterFreq,
		enum ENUM_BAND eBand);
#endif

#if (CFG_MTK_ANDROID_WMT || WLAN_INCLUDE_PROC)
int set_p2p_mode_handler(struct net_device *netdev,
			 struct PARAM_CUSTOM_P2P_SET_STRUCT p2pmode);
#endif

#if CFG_SUPPORT_NAN
int set_nan_handler(struct net_device *netdev, uint32_t ucEnable);
#endif

#if CFG_ENABLE_UNIFY_WIPHY
const struct net_device_ops *wlanGetNdevOps(void);
#endif

#if CFG_MTK_ANDROID_WMT
extern void connectivity_flush_dcache_area(void *addr, size_t len);
extern void connectivity_arch_setup_dma_ops(
	struct device *dev, u64 dma_base,
	u64 size, struct iommu_ops *iommu,
	bool coherent);
extern void connectivity_export_show_stack(struct task_struct *tsk,
	unsigned long *sp);
#endif

netdev_tx_t wlanHardStartXmit(struct sk_buff *prSkb, struct net_device *prDev);

typedef uint8_t (*file_buf_handler) (void *ctx,
			const char __user *buf,
			uint16_t length);

#if CFG_WLAN_ASSISTANT_NVRAM
extern void register_file_buf_handler(file_buf_handler handler,
			void *ctx,
			uint8_t ucType);
#endif

#if CFG_SUPPORT_CUSTOM_NETLINK
extern void glCustomGenlInit(void);
extern void glCustomGenlDeinit(void);
#endif

extern void WfsysResetHdlr(struct work_struct *work);

extern const uint8_t *kalFindIeMatchMask(uint8_t eid,
				const uint8_t *ies, int len,
				const uint8_t *match,
				int match_len, int match_offset,
				const uint8_t *match_mask);


void wlanNvramSetState(enum ENUM_NVRAM_STATE state);
enum ENUM_NVRAM_STATE wlanNvramGetState(void);

#if (CFG_SUPPORT_SNIFFER_RADIOTAP == 1)
int32_t sysCreateMonDbgFs(struct GLUE_INFO *prGlueInfo);
void sysRemoveMonDbgFs(void);
#endif
#endif /* _GL_OS_H */
