/* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause */
/*
 * Copyright (c) 2016 MediaTek Inc.
 */
/*
 ** Id: /os/linux/gl_proc.c
 */

/*! \file   "gl_proc.c"
 *  \brief  This file defines the interface which can interact with users
 *          in /proc fs.
 *
 *    Detail description.
 */


/*******************************************************************************
 *                         C O M P I L E R   F L A G S
 *******************************************************************************
 */

/*******************************************************************************
 *                    E X T E R N A L   R E F E R E N C E S
 *******************************************************************************
 */
#include "precomp.h"
#include "gl_os.h"
#include "gl_kal.h"
#include "debug.h"
#include "wlan_lib.h"
#include "debug.h"
#include "wlan_oid.h"

/*******************************************************************************
 *                              C O N S T A N T S
 *******************************************************************************
 */
#define PROC_MCR_ACCESS                         "mcr"
#define PROC_ROOT_NAME							"wlan"

#if CFG_SUPPORT_DEBUG_FS
#define PROC_ROAM_PARAM							"roam_param"
#endif
#define PROC_COUNTRY							"country"
#define PROC_DRV_STATUS                         "status"
#define PROC_RX_STATISTICS                      "rx_statistics"
#define PROC_TX_STATISTICS                      "tx_statistics"
#define PROC_DBG_LEVEL_NAME                     "dbgLevel"
#define PROC_DRIVER_CMD                         "driver"
#define PROC_CFG                                "cfg"
#define PROC_EFUSE_DUMP                         "efuse_dump"
#if CFG_WIFI_TXPWR_TBL_DUMP
#define PROC_GET_TXPWR_TBL                      "get_txpwr_tbl"
#endif
#define PROC_PKT_DELAY_DBG			"pktDelay"
#if CFG_SUPPORT_SET_CAM_BY_PROC
#define PROC_SET_CAM				"setCAM"
#endif
#define PROC_AUTO_PERF_CFG			"autoPerfCfg"
#if CFG_ASSERT_DUMP
#define PROC_CORE_DUMP                     "core_dump"
#endif
#if CFG_SUPPORT_CSI
#define PROC_CSI_DATA_NAME                     "csi_data"
#endif
#if CFG_SUPPORT_PROC_GET_WAKEUP_REASON
#define PROC_WAKEUP_REASON			"wakeup_reason"
#endif
#if CFG_SUPPORT_DYNAMIC_PWR_LIMIT
#define PROC_SAR_CFG_DEBUG			"sarCfgDebug"
#endif

#define PROC_MCR_ACCESS_MAX_USER_INPUT_LEN      20
#define PROC_RX_STATISTICS_MAX_USER_INPUT_LEN   10
#define PROC_TX_STATISTICS_MAX_USER_INPUT_LEN   10
#define PROC_DBG_LEVEL_MAX_USER_INPUT_LEN       20
#define PROC_DBG_LEVEL_MAX_DISPLAY_STR_LEN      30
#define PROC_UID_SHELL							2000
#define PROC_GID_WIFI							1010

/* notice: str only can be an array */
#define SNPRINTF(buf, str, arg)   {buf += \
	snprintf((char *)(buf), sizeof(str)-kalStrLen(str), PRINTF_ARG arg); }

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
static struct GLUE_INFO *g_prGlueInfo_proc;
static uint32_t u4McrOffset;
static struct proc_dir_entry *gprProcRoot;
static uint8_t aucDbModuleName[][PROC_DBG_LEVEL_MAX_DISPLAY_STR_LEN] = {
	"INIT", "HAL", "INTR", "REQ", "TX", "RX", "RFTEST", "EMU", "SW1", "SW2",
	"SW3", "SW4", "HEM", "AIS", "RLM", "MEM", "CNM", "RSN", "BSS", "SCN",
	"SAA", "AAA", "P2P", "QM", "SEC", "BOW", "WAPI", "ROAMING", "TDLS",
	"PF", "OID", "NIC", "NAN"
};

/* This buffer could be overwrite by any proc commands */
static uint8_t g_aucProcBuf[3000];

/* This u32 is only for DriverCmdRead/Write,
 * should not be used by other function
 */
static int32_t g_i4NextDriverReadLen;
/*******************************************************************************
 *                                 M A C R O S
 *******************************************************************************
 */

/*******************************************************************************
 *                   F U N C T I O N   D E C L A R A T I O N S
 *******************************************************************************
 */
#if CFG_SUPPORT_CSI
static int procCSIDataOpen(struct inode *n, struct file *f)
{
	struct CSI_INFO_T *prCSIInfo = NULL;

	if (g_prGlueInfo_proc && g_prGlueInfo_proc->prAdapter) {
		prCSIInfo = &(g_prGlueInfo_proc->prAdapter->rCSIInfo);
		prCSIInfo->bIncomplete = FALSE;
	}

	return 0;
}

static int procCSIDataRelease(struct inode *n, struct file *f)
{
	struct CSI_INFO_T *prCSIInfo = NULL;

	if (g_prGlueInfo_proc && g_prGlueInfo_proc->prAdapter) {
		prCSIInfo = &(g_prGlueInfo_proc->prAdapter->rCSIInfo);
		prCSIInfo->bIncomplete = FALSE;
	}

	return 0;
}

static ssize_t procCSIDataPrepare(
	uint8_t *buf,
	struct CSI_INFO_T *prCSIInfo,
	struct CSI_DATA_T *prCSIData)
{
	int32_t i4Pos = 0;
	uint8_t *tmpBuf = buf;
	uint16_t u2DataSize = prCSIData->u2DataCount * sizeof(int16_t);
	uint16_t u2Rsvd1Size = prCSIData->ucRsvd1Cnt * sizeof(int32_t);
	enum ENUM_CSI_MODULATION_BW_TYPE_T eModulationType = CSI_TYPE_CCK_BW20;

	if (prCSIData->ucBw == 0)
		eModulationType = prCSIData->bIsCck ?
			CSI_TYPE_CCK_BW20 : CSI_TYPE_OFDM_BW20;
	else if (prCSIData->ucBw == 1)
		eModulationType = CSI_TYPE_OFDM_BW40;
	else if (prCSIData->ucBw == 2)
		eModulationType = CSI_TYPE_OFDM_BW80;

	put_unaligned(0xAC, (tmpBuf + i4Pos));
	i4Pos++;

	/* Just bypass total length feild here and update it in the end */
	i4Pos += 2;

	put_unaligned(CSI_DATA_VER, (uint8_t *) (tmpBuf + i4Pos));
	i4Pos++;
	put_unaligned(1, (uint16_t *) (tmpBuf + i4Pos));
	i4Pos += 2;
	put_unaligned(prCSIData->ucFwVer, (uint8_t *) (tmpBuf + i4Pos));
	i4Pos++;

	put_unaligned(CSI_DATA_TYPE, (uint8_t *) (tmpBuf + i4Pos));
	i4Pos++;
	put_unaligned(1, (uint16_t *) (tmpBuf + i4Pos));
	i4Pos += 2;
	put_unaligned(eModulationType, (uint8_t *) (tmpBuf + i4Pos));
	i4Pos++;

	put_unaligned(CSI_DATA_TS, (uint8_t *) (tmpBuf + i4Pos));
	i4Pos++;
	put_unaligned(8, (uint16_t *) (tmpBuf + i4Pos));
	i4Pos += 2;
	put_unaligned(prCSIData->u8TimeStamp, (uint64_t *) (tmpBuf + i4Pos));
	i4Pos += 8;

	put_unaligned(CSI_DATA_RSSI, (uint8_t *) (tmpBuf + i4Pos));
	i4Pos++;
	put_unaligned(1, (uint16_t *) (tmpBuf + i4Pos));
	i4Pos += 2;
	put_unaligned(prCSIData->cRssi, (uint8_t *) (tmpBuf + i4Pos));
	i4Pos++;

	put_unaligned(CSI_DATA_SNR, (tmpBuf + i4Pos));
	i4Pos++;
	put_unaligned(1, (uint16_t *) (tmpBuf + i4Pos));
	i4Pos += 2;
	put_unaligned(prCSIData->ucSNR, (uint8_t *) (tmpBuf + i4Pos));
	i4Pos++;

	put_unaligned(CSI_DATA_DBW, (uint8_t *) (tmpBuf + i4Pos));
	i4Pos++;
	put_unaligned(1, (uint16_t *) (tmpBuf + i4Pos));
	i4Pos += 2;
	put_unaligned(prCSIData->ucDataBw, (uint8_t *) (tmpBuf + i4Pos));
	i4Pos++;

	put_unaligned(CSI_DATA_CH_IDX, (uint8_t *) (tmpBuf + i4Pos));
	i4Pos++;
	put_unaligned(1, (uint16_t *) (tmpBuf + i4Pos));
	i4Pos += 2;
	put_unaligned(prCSIData->ucPrimaryChIdx, (uint8_t *) (tmpBuf + i4Pos));
	i4Pos++;

	put_unaligned(CSI_DATA_TA, (uint8_t *) (tmpBuf + i4Pos));
	i4Pos++;
	put_unaligned(MAC_ADDR_LEN, (uint16_t *) (tmpBuf + i4Pos));
	i4Pos += 2;
	kalMemCopy((tmpBuf + i4Pos), prCSIData->aucTA, MAC_ADDR_LEN);
	i4Pos += MAC_ADDR_LEN;

	put_unaligned(CSI_DATA_EXTRA_INFO, (uint8_t *) (tmpBuf + i4Pos));
	i4Pos++;
	put_unaligned(4, (uint16_t *) (tmpBuf + i4Pos));
	i4Pos += 2;
	put_unaligned(prCSIData->u4ExtraInfo, (uint32_t *) (tmpBuf + i4Pos));
	i4Pos += sizeof(uint32_t);

	put_unaligned(CSI_DATA_I, (uint8_t *) (tmpBuf + i4Pos));
	i4Pos++;
	put_unaligned(u2DataSize, (uint16_t *) (tmpBuf + i4Pos));
	i4Pos += 2;
	kalMemCopy((tmpBuf + i4Pos), prCSIData->ac2IData, u2DataSize);
	i4Pos += u2DataSize;

	put_unaligned(CSI_DATA_Q, (uint8_t *) (tmpBuf + i4Pos));
	i4Pos++;
	put_unaligned(u2DataSize, (uint16_t *) (tmpBuf + i4Pos));
	i4Pos += 2;
	kalMemCopy((tmpBuf + i4Pos), prCSIData->ac2QData, u2DataSize);
	i4Pos += u2DataSize;

	if (prCSIInfo->ucValue1[CSI_CONFIG_INFO] & CSI_INFO_RSVD1) {
		put_unaligned(CSI_DATA_RSVD1, (uint8_t *) (tmpBuf + i4Pos));
		i4Pos++;
		put_unaligned(u2Rsvd1Size, (uint16_t *) (tmpBuf + i4Pos));
		i4Pos += 2;
		kalMemCopy((tmpBuf + i4Pos),
			prCSIData->ai4Rsvd1,
			u2Rsvd1Size);
		i4Pos += u2Rsvd1Size;

		put_unaligned(CSI_DATA_RSVD2, (uint8_t *) (tmpBuf + i4Pos));
		i4Pos++;
		put_unaligned(u2Rsvd1Size, (uint16_t *) (tmpBuf + i4Pos));
		i4Pos += 2;
		kalMemCopy((tmpBuf + i4Pos),
			prCSIData->au4Rsvd2,
			u2Rsvd1Size);
		i4Pos += u2Rsvd1Size;

		put_unaligned(CSI_DATA_RSVD3, (uint8_t *) (tmpBuf + i4Pos));
		i4Pos++;
		put_unaligned(sizeof(int32_t), (int16_t *) (tmpBuf + i4Pos));
		i4Pos += 2;
		put_unaligned(prCSIData->i4Rsvd3,
			(int32_t *) (tmpBuf + i4Pos));
		i4Pos += sizeof(int32_t);
	}

	if (prCSIInfo->ucValue1[CSI_CONFIG_INFO] & CSI_INFO_RSVD2) {
		put_unaligned(CSI_DATA_RSVD4, (uint8_t *) (tmpBuf + i4Pos));
		i4Pos++;
		put_unaligned(sizeof(uint8_t), (int16_t *) (tmpBuf + i4Pos));
		i4Pos += 2;
		put_unaligned(prCSIData->ucRsvd4, (uint8_t *) (tmpBuf + i4Pos));
		i4Pos += sizeof(uint8_t);
	}

	put_unaligned(CSI_DATA_TX_IDX, (uint8_t *) (tmpBuf + i4Pos));
	i4Pos++;
	put_unaligned(sizeof(uint8_t), (int16_t *) (tmpBuf + i4Pos));
	i4Pos += 2;

	put_unaligned(((uint8_t)GET_CSI_TX_IDX(prCSIData->u4TRxIdx)),
		(uint8_t *) (tmpBuf + i4Pos));
	i4Pos += sizeof(uint8_t);

	put_unaligned(CSI_DATA_RX_IDX, (uint8_t *) (tmpBuf + i4Pos));
	i4Pos++;
	put_unaligned(sizeof(uint8_t), (int16_t *) (tmpBuf + i4Pos));
	i4Pos += 2;
	put_unaligned(((uint8_t)GET_CSI_RX_IDX(prCSIData->u4TRxIdx)),
		(uint8_t *) (tmpBuf + i4Pos));
	i4Pos += sizeof(uint8_t);

	put_unaligned(CSI_DATA_FRAME_MODE, (uint8_t *) (tmpBuf + i4Pos));
	i4Pos++;
	put_unaligned(sizeof(uint8_t), (int16_t *) (tmpBuf + i4Pos));
	i4Pos += 2;
	put_unaligned(prCSIData->ucRxMode,
		(uint8_t *) (tmpBuf + i4Pos));
	i4Pos += sizeof(uint8_t);

	/* add antenna pattern*/
	put_unaligned(CSI_DATA_H_IDX, (uint8_t *) (tmpBuf + i4Pos));
	i4Pos++;
	put_unaligned(sizeof(uint32_t), (int16_t *) (tmpBuf + i4Pos));
	i4Pos += 2;
	put_unaligned(prCSIData->Antenna_pattern,
			(uint32_t *) (tmpBuf + i4Pos));
	i4Pos += sizeof(uint32_t);

	put_unaligned(CSI_DATA_RX_RATE, (uint8_t *) (tmpBuf + i4Pos));
	i4Pos++;
	put_unaligned(sizeof(uint8_t), (int16_t *) (tmpBuf + i4Pos));
	i4Pos += 2;
	put_unaligned(prCSIData->u2RxRate,
		(uint8_t *) (tmpBuf + i4Pos));
	i4Pos += sizeof(uint8_t);

	/*
	 * The lengths of magic number (1 byte) and total length (2 bytes)
	 * fields should not be counted in the total length value
	 */
	put_unaligned(i4Pos - 3, (uint16_t *) (tmpBuf + 1));

	return i4Pos;
}

struct CSI_DATA_T rTmpCSIData;
static ssize_t procCSIDataRead(struct file *filp,
	char __user *buf, size_t count, loff_t *f_pos)
{
	uint8_t *temp = &g_aucProcBuf[0];
	uint32_t u4CopySize = 0;
	uint32_t u4StartIdx = 0;
	int32_t i4Pos = 0;
	struct CSI_INFO_T *prCSIInfo = NULL;

	if (g_prGlueInfo_proc && g_prGlueInfo_proc->prAdapter)
		prCSIInfo = &(g_prGlueInfo_proc->prAdapter->rCSIInfo);
	else
		return 0;

	if (prCSIInfo->bIncomplete == FALSE) {

		wait_event_interruptible(prCSIInfo->waitq,
			prCSIInfo->u4CSIBufferUsed != 0);

		/*
		 * No older CSI data in buffer waiting for reading out,
		 * so prepare a new one for reading.
		 */
		if (wlanPopCSIData(g_prGlueInfo_proc->prAdapter,
			&rTmpCSIData))
			i4Pos = procCSIDataPrepare(temp,
				prCSIInfo, &rTmpCSIData);

		/* The frist run of reading the CSI data */
		u4StartIdx = 0;
		if (i4Pos > count) {
#ifdef CFG_SSVD_SVACE64
			u4CopySize = (uint32_t)count;
#else
			u4CopySize = count;
#endif
			prCSIInfo->u4RemainingDataSize = i4Pos - count;
			prCSIInfo->u4CopiedDataSize = count;
			prCSIInfo->bIncomplete = TRUE;
		} else {
			u4CopySize = i4Pos;
		}
	} else {
		/* Reading the remaining CSI data in the buffer */

		u4StartIdx = prCSIInfo->u4CopiedDataSize;
		if (prCSIInfo->u4RemainingDataSize > count) {
#ifdef CFG_SSVD_SVACE64
			u4CopySize = (uint32_t)count;
#else
			u4CopySize = count;
#endif
			prCSIInfo->u4RemainingDataSize -= count;
			prCSIInfo->u4CopiedDataSize += count;
		} else {
			u4CopySize = prCSIInfo->u4RemainingDataSize;
			prCSIInfo->bIncomplete = FALSE;
		}
	}

	if (copy_to_user(buf, &g_aucProcBuf[u4StartIdx], u4CopySize)) {
		DBGLOG(INIT, ERROR, "[CSI] copy to user failed\n");
		return -EFAULT;
	}

	*f_pos += u4CopySize;

	DBGLOG(INIT, INFO, "[CSI] u4CopySize=%d\n", u4CopySize);

	return (ssize_t)u4CopySize;
}
#endif




#if CFG_ASSERT_DUMP
static ssize_t procCoreDumpRead(struct file *file, char __user *buf,
			size_t count, loff_t *f_pos)
{
	struct GLUE_INFO *prGlueInfo;
	struct ADAPTER *prAdapter;
	struct sk_buff *skb = NULL;
	int copyLen = 0;
	unsigned long ret_len = 0;

	KAL_SPIN_LOCK_DECLARATION();

	prGlueInfo = *((struct GLUE_INFO **)netdev_priv(gPrDev));

	if (!prGlueInfo) {
		DBGLOG(INIT, ERROR, "procCfgRead prGlueInfo is  NULL\n");
		return -EFAULT;
	}

	prAdapter = prGlueInfo->prAdapter;

	if (!prAdapter) {
		DBGLOG(INIT, ERROR, "procCfgRead prAdapter is  NULL\n");
		return -EFAULT;
	}

	KAL_ACQUIRE_SPIN_LOCK(prAdapter, SPIN_LOCK_CORE_DUMP);
	skb = skb_dequeue(&prGlueInfo->rFwDumpSkbQueue);
	KAL_RELEASE_SPIN_LOCK(prAdapter, SPIN_LOCK_CORE_DUMP);

	if (skb == NULL)
		return 0;

	if (skb->len <= count) {
		ret_len = copy_to_user(buf, skb->data, skb->len);
		if (ret_len) {
			DBGLOG(INIT, ERROR,
		"%s: copy_to_user failed, skb->len = %d, ret_len = %ld, count = %zd",
					__func__, skb->len, ret_len, count);
			/* copy_to_user failed, add skb to fw log queue */
			KAL_ACQUIRE_SPIN_LOCK(prAdapter, SPIN_LOCK_CORE_DUMP);
			skb_queue_head(&prGlueInfo->rFwDumpSkbQueue, skb);
			KAL_RELEASE_SPIN_LOCK(prAdapter, SPIN_LOCK_CORE_DUMP);
			copyLen = -EFAULT;
			goto out;
		}
		copyLen = skb->len;
	} else
		DBGLOG(INIT, ERROR,
			"%s: socket buffer length error(count: %d, skb.len: %d)",
			__func__, (int)count, skb->len);

	kfree_skb(skb);

out:
	return copyLen;
}

static unsigned int procCoreDumpPoll(struct file *file, poll_table *wait)
{
	struct GLUE_INFO *prGlueInfo;
	unsigned int mask = 0;

	prGlueInfo = *((struct GLUE_INFO **)netdev_priv(gPrDev));

	if (!prGlueInfo) {
		DBGLOG(INIT, ERROR, "procCoreDumpPoll prGlueInfo is  NULL\n");
		return -EFAULT;
	}

	poll_wait(file, &prGlueInfo->waitq_fwdump, wait);
	if (skb_queue_len(&prGlueInfo->rFwDumpSkbQueue) > 0)
		mask |= POLLIN | POLLRDNORM;

	return mask;
}

#endif

static ssize_t procDbgLevelRead(struct file *filp, char __user *buf,
	size_t count, loff_t *f_pos)
{
	uint8_t *temp = &g_aucProcBuf[0];
	uint8_t *str = NULL;
	uint32_t u4CopySize = 0;
	uint16_t i;
	uint16_t u2ModuleNum = 0;
	uint32_t u4StrLen = 0;
	uint32_t u4Level1, u4Level2;

	/* if *f_ops>0, we should return 0 to make cat command exit */
	if (*f_pos > 0 || buf == NULL)
		return 0;

	str = "\nTEMP|LOUD|INFO|TRACE | EVENT|STATE|WARN|ERROR\n"
	    "bit7|bit6|bit5|bit4 | bit3|bit2|bit1|bit0\n\n"
	    "Usage: Module Index:Module Level, such as 0x00:0xff\n\n"
	    "Debug Module\tIndex\tLevel\tDebug Module\tIndex\tLevel\n\n";
	u4StrLen = kalStrLen(str);
	kalStrnCpy(temp, str, u4StrLen);
	temp[u4StrLen] = '\0';
	temp += u4StrLen;

	u2ModuleNum =
	    (sizeof(aucDbModuleName) /
	     PROC_DBG_LEVEL_MAX_DISPLAY_STR_LEN) & 0xfe;

	for (i = 0; i < u2ModuleNum; i += 2) {
		wlanGetDriverDbgLevel(i, &u4Level1);
		wlanGetDriverDbgLevel(i + 1, &u4Level2);
		SNPRINTF(temp, g_aucProcBuf,
			("DBG_%s_IDX\t(0x%02x):\t0x%02x\t"
			 "DBG_%s_IDX\t(0x%02x):\t0x%02x\n",
			 &aucDbModuleName[i][0], i, (uint8_t) u4Level1,
			 &aucDbModuleName[i + 1][0], i + 1,
			 (uint8_t) u4Level2));
	}

	if ((sizeof(aucDbModuleName) /
	     PROC_DBG_LEVEL_MAX_DISPLAY_STR_LEN) & 0x1) {
		wlanGetDriverDbgLevel(u2ModuleNum, &u4Level1);
		SNPRINTF(temp, g_aucProcBuf,
			 ("DBG_%s_IDX\t(0x%02x):\t0x%02x\n",
			  &aucDbModuleName[u2ModuleNum][0], u2ModuleNum,
			  (uint8_t) u4Level1));
	}

	u4CopySize = kalStrLen(g_aucProcBuf);
	if (u4CopySize > count)
		u4CopySize = count;
	if (copy_to_user(buf, g_aucProcBuf, u4CopySize)) {
		DBGLOG(INIT, ERROR, "copy to user failed\n");
		return -EFAULT;
	}

	*f_pos += u4CopySize;
	return (ssize_t) u4CopySize;
}

#if CFG_SUPPORT_CSI
static DEFINE_PROC_OPS_STRUCT(csidata_ops) = {
	DEFINE_PROC_OPS_OWNER(THIS_MODULE)
	DEFINE_PROC_OPS_READ(procCSIDataRead)
	DEFINE_PROC_OPS_OPEN(procCSIDataOpen)
	DEFINE_PROC_OPS_RELEASE(procCSIDataRelease)
};
#endif

#if WLAN_INCLUDE_PROC
#if	CFG_SUPPORT_EASY_DEBUG

static void *procEfuseDump_start(struct seq_file *s, loff_t *pos)
{
	static unsigned long counter;

	if (*pos == 0)
		counter = *pos;	/* read file init */

	if (counter >= EFUSE_ADDR_MAX)
		return NULL;
	return &counter;
}

static void *procEfuseDump_next(struct seq_file *s, void *v, loff_t *pos)
{
	unsigned long *tmp_v = (unsigned long *)v;

	(*tmp_v) += EFUSE_BLOCK_SIZE;

	if (*tmp_v >= EFUSE_ADDR_MAX)
		return NULL;
	return tmp_v;
}

static void procEfuseDump_stop(struct seq_file *s, void *v)
{
	/* nothing to do, we use a static value in start() */
}

static int procEfuseDump_show(struct seq_file *s, void *v)
{
	uint32_t rStatus = WLAN_STATUS_SUCCESS;
	uint32_t u4BufLen = 0;
	struct GLUE_INFO *prGlueInfo;
	uint32_t idx_addr, idx_value;
	struct PARAM_CUSTOM_ACCESS_EFUSE rAccessEfuseInfo = { };

	prGlueInfo = g_prGlueInfo_proc;

#if  (CFG_EEPROM_PAGE_ACCESS == 1)
	if (prGlueInfo == NULL) {
		seq_puts(s, "prGlueInfo is null\n");
		return -EPERM;
	}

	if (prGlueInfo &&
	    prGlueInfo->prAdapter &&
	    prGlueInfo->prAdapter->chip_info &&
	    !prGlueInfo->prAdapter->chip_info->is_support_efuse) {
		seq_puts(s, "efuse ops is invalid\n");
		return -EPERM; /* return negative value to stop read process */
	}

	idx_addr = *(loff_t *) v;
	rAccessEfuseInfo.u4Address =
		(idx_addr / EFUSE_BLOCK_SIZE) * EFUSE_BLOCK_SIZE;

	rStatus = kalIoctl(prGlueInfo,
		wlanoidQueryProcessAccessEfuseRead,
		&rAccessEfuseInfo,
		sizeof(struct PARAM_CUSTOM_ACCESS_EFUSE), TRUE, TRUE,
		TRUE, &u4BufLen);
	if (rStatus != WLAN_STATUS_SUCCESS) {
		seq_printf(s, "efuse read fail (0x%03X)\n",
			rAccessEfuseInfo.u4Address);
		return 0;
	}

	for (idx_value = 0; idx_value < EFUSE_BLOCK_SIZE; idx_value++)
		seq_printf(s, "0x%03X=0x%02X\n",
			rAccessEfuseInfo.u4Address + idx_value,
			prGlueInfo->prAdapter->aucEepromVaule[idx_value]);
	return 0;
#else
	seq_puts(s, "efuse ops is invalid\n");
	return -EPERM; /* return negative value to stop read process */
#endif
}

static int procEfuseDumpOpen(struct inode *inode, struct file *file)
{
	static const struct seq_operations procEfuseDump_ops = {
		.start = procEfuseDump_start,
		.next = procEfuseDump_next,
		.stop = procEfuseDump_stop,
		.show = procEfuseDump_show
	};

	return seq_open(file, &procEfuseDump_ops);
}

#if CFG_SUPPORT_CFG_FILE
static ssize_t procCfgRead(struct file *filp, char __user *buf, size_t count,
	loff_t *f_pos)
{
	uint8_t *temp = &g_aucProcBuf[0];
	uint8_t *str = NULL;
	uint8_t *str2 = "\nERROR DUMP CONFIGURATION:\n";
	uint32_t u4CopySize = 0;
	uint32_t i;
	uint32_t u4StrLen = 0;

#define BUFFER_RESERVE_BYTE 50

	struct GLUE_INFO *prGlueInfo;

	struct WLAN_CFG_ENTRY *prWlanCfgEntry;
	struct ADAPTER *prAdapter;

	prGlueInfo = *((struct GLUE_INFO **)netdev_priv(gPrDev));

	if (!prGlueInfo) {
		DBGLOG(INIT, ERROR, "procCfgRead prGlueInfo is  NULL\n");
		return 0;
	}

	prAdapter = prGlueInfo->prAdapter;

	if (!prAdapter) {
		DBGLOG(INIT, ERROR, "procCfgRead prAdapter is  NULL\n");
		return 0;
	}

	/* if *f_ops>0, we should return 0 to make cat command exit */
	if (*f_pos > 0 || buf == NULL)
		return 0;

	str = "\nDUMP CONFIGURATION :\n"
	    "<KEY|VALUE> OR <D:KEY|VALUE>\n"
	    "'D': driver part current setting\n"
	    "===================================\n";
	u4StrLen = kalStrLen(str);
	kalStrnCpy(temp, str, u4StrLen);
	temp[u4StrLen] = '\0';
	temp += u4StrLen;

	for (i = 0; i < WLAN_CFG_ENTRY_NUM_MAX; i++) {
		prWlanCfgEntry = wlanCfgGetEntryByIndex(prAdapter, i, 0);

		if ((!prWlanCfgEntry) || (prWlanCfgEntry->aucKey[0] == '\0'))
			break;

		SNPRINTF(temp, g_aucProcBuf,
			("%s|%s\n", prWlanCfgEntry->aucKey,
			prWlanCfgEntry->aucValue));

		if ((temp - g_aucProcBuf) != kalStrLen(g_aucProcBuf)) {
			DBGLOG(INIT, ERROR,
			       "Dump configuration error: temp offset=%d, buf length=%u, key[%d]=[%u], val[%d]=[%u]\n",
			       (int)(temp - g_aucProcBuf),
			       (uint32_t)kalStrLen(g_aucProcBuf),
			       WLAN_CFG_KEY_LEN_MAX,
			       (uint32_t)prWlanCfgEntry->aucKey[
				WLAN_CFG_KEY_LEN_MAX - 1],
			       WLAN_CFG_VALUE_LEN_MAX,
			       (uint32_t)prWlanCfgEntry->aucValue[
				WLAN_CFG_VALUE_LEN_MAX - 1]);
			kalMemSet(g_aucProcBuf, ' ', u4StrLen);
			kalStrnCpy(g_aucProcBuf, str2, kalStrLen(str2) + 1);
			goto procCfgReadLabel;
		}

		if (kalStrLen(g_aucProcBuf) >
			(sizeof(g_aucProcBuf) - BUFFER_RESERVE_BYTE))
			break;
	}

	for (i = 0; i < WLAN_CFG_REC_ENTRY_NUM_MAX; i++) {
		prWlanCfgEntry = wlanCfgGetEntryByIndex(prAdapter, i, 1);


		if ((!prWlanCfgEntry) || (prWlanCfgEntry->aucKey[0] == '\0'))
			break;

		SNPRINTF(temp, g_aucProcBuf,
			("D:%s|%s\n", prWlanCfgEntry->aucKey,
			prWlanCfgEntry->aucValue));

		if ((temp - g_aucProcBuf) != kalStrLen(g_aucProcBuf)) {
			DBGLOG(INIT, ERROR,
			       "D:Dump configuration error: temp offset=%d, buf length=%u, key[%d]=[%u], val[%d]=[%u]\n",
			       (int)(temp - g_aucProcBuf),
			       (uint32_t)kalStrLen(g_aucProcBuf),
			       WLAN_CFG_KEY_LEN_MAX,
			       (uint32_t)prWlanCfgEntry->aucKey[
				WLAN_CFG_KEY_LEN_MAX - 1],
			       WLAN_CFG_VALUE_LEN_MAX,
			       (uint32_t)prWlanCfgEntry->aucValue[
				WLAN_CFG_VALUE_LEN_MAX - 1]);
			kalMemSet(g_aucProcBuf, ' ', u4StrLen);
			kalStrnCpy(g_aucProcBuf, str2,
				sizeof(g_aucProcBuf) - 1);
			goto procCfgReadLabel;
		}

		if (kalStrLen(g_aucProcBuf) >
			(sizeof(g_aucProcBuf) - BUFFER_RESERVE_BYTE))
			break;
	}

procCfgReadLabel:
	g_aucProcBuf[sizeof(g_aucProcBuf) - 1] = '\0';
	u4CopySize = kalStrLen(g_aucProcBuf);
	if (u4CopySize > count)
		u4CopySize = count;
	if (copy_to_user(buf, g_aucProcBuf, u4CopySize)) {
		DBGLOG(INIT, ERROR,"copy to user failed\n");
		return -EFAULT;
	}

	*f_pos += u4CopySize;
	return (ssize_t) u4CopySize;
}

static ssize_t procCfgWrite(struct file *file, const char __user *buffer,
	size_t count, loff_t *data)
{
	uint32_t u4CopySize = sizeof(g_aucProcBuf)-8;
	struct GLUE_INFO *prGlueInfo;
	uint8_t *pucTmp;
	uint32_t i = 0;

	if (count <= 0) {
		DBGLOG(INIT, ERROR, "wrong copy size\n");
		return -EFAULT;
	}

	kalMemSet(g_aucProcBuf, 0, u4CopySize);
	u4CopySize = (count < u4CopySize) ? count : (u4CopySize - 1);

	pucTmp = g_aucProcBuf;
	SNPRINTF(pucTmp, g_aucProcBuf, ("%s ", "set_cfg"));

	if (copy_from_user(pucTmp, buffer, u4CopySize)) {
		DBGLOG(INIT, ERROR, "error of copy from user\n");
		return -EFAULT;
	}
	g_aucProcBuf[u4CopySize + 8] = '\0';

	for (i = 8 ; i < u4CopySize + 8; i++) {
		if (!isalnum(g_aucProcBuf[i]) && /* alphanumeric */
			g_aucProcBuf[i] != 0x20 && /* space */
			g_aucProcBuf[i] != 0x0a && /* control char */
			g_aucProcBuf[i] != 0x0d) {
			DBGLOG(INIT, ERROR, "wrong char[%d] 0x%x\n",
				i, g_aucProcBuf[i]);
			return -EFAULT;
		}
	}

	prGlueInfo = g_prGlueInfo_proc;
	/* if g_i4NextDriverReadLen >0,
	 * the content for next DriverCmdRead will be
	 * in : g_aucProcBuf with length : g_i4NextDriverReadLen
	 */
	g_i4NextDriverReadLen =
		priv_driver_set_cfg(prGlueInfo->prDevHandler, g_aucProcBuf,
		sizeof(g_aucProcBuf));

	return count;

}
#endif
static ssize_t procDriverCmdRead(struct file *filp, char __user *buf,
	size_t count, loff_t *f_pos)
{
	/* DriverCmd read should only be executed right after
	 * a DriverCmd write because content buffer 'g_aucProcBuf'
	 * is a global buffer for all proc command, otherwise ,
	 * the content could be overwrite by other proc command
	 */
	uint32_t u4CopySize = 0;

	/* if *f_ops>0, we should return 0 to make cat command exit */
	if (*f_pos > 0 || buf == NULL)
		return 0;

	if (g_i4NextDriverReadLen > 0)	/* Detect content to show */
		u4CopySize = g_i4NextDriverReadLen;

	if (u4CopySize > count) {
		DBGLOG(INIT, ERROR, "count is too small: u4CopySize=%u, count=%u\n",
		       u4CopySize, (uint32_t)count);
		return -EFAULT;
	}

	if (copy_to_user(buf, g_aucProcBuf, u4CopySize)) {
		DBGLOG(INIT, ERROR, "copy to user failed\n");
		return -EFAULT;
	}
	g_i4NextDriverReadLen = 0;

	*f_pos += u4CopySize;
	return (ssize_t) u4CopySize;
}



static ssize_t procDriverCmdWrite(struct file *file, const char __user *buffer,
	size_t count, loff_t *data)
{
/*	uint32_t u4DriverCmd, u4DriverValue;
 *	uint8_t *temp = &g_aucProcBuf[0];
 */
	uint32_t u4CopySize = sizeof(g_aucProcBuf);
	struct GLUE_INFO *prGlueInfo;
/*	PARAM_CUSTOM_P2P_SET_STRUCT_T rSetP2P; */

	kalMemSet(g_aucProcBuf, 0, u4CopySize);
	u4CopySize = (count < u4CopySize) ? count : (u4CopySize - 1);

	if (copy_from_user(g_aucProcBuf, buffer, u4CopySize)) {
		DBGLOG(INIT, ERROR,"error of copy from user\n");
		return -EFAULT;
	}
	g_aucProcBuf[u4CopySize] = '\0';


	prGlueInfo = g_prGlueInfo_proc;
	/* if g_u4NextDriverReadLen >0,
	 * the content for next DriverCmdRead will be
	 *  in : g_aucProcBuf with length : g_u4NextDriverReadLen
	 */
	g_i4NextDriverReadLen =
		priv_driver_cmds(prGlueInfo->prDevHandler, g_aucProcBuf,
		sizeof(g_aucProcBuf));

	return count;
}
#endif
#endif

static ssize_t procDbgLevelWrite(struct file *file, const char __user *buffer,
	size_t count, loff_t *data)
{
	uint32_t u4NewDbgModule, u4NewDbgLevel;
	uint8_t *temp = &g_aucProcBuf[0];
	uint32_t u4CopySize = sizeof(g_aucProcBuf);

	kalMemSet(g_aucProcBuf, 0, u4CopySize);
	u4CopySize = (count < u4CopySize) ? count : (u4CopySize - 1);

	if (copy_from_user(g_aucProcBuf, buffer, u4CopySize)) {
		DBGLOG(INIT, ERROR, "error of copy from user\n");
		return -EFAULT;
	}
	g_aucProcBuf[u4CopySize] = '\0';

	/*add chip reset cmd for manual test*/
#if CFG_CHIP_RESET_SUPPORT
	if (temp[0] == 'R') {
		DBGLOG(INIT, INFO, "WIFI trigger reset!!\n");
		if (g_prGlueInfo_proc == NULL ||
			g_prGlueInfo_proc->prAdapter == NULL) {
			DBGLOG(INIT, ERROR,
				"g_prGlueInfo_proc is NULL, skip reset!\n");
			return -EFAULT;
		}
		GL_USER_DEFINE_RESET_TRIGGER(g_prGlueInfo_proc->prAdapter,
			RST_CMD_TRIGGER, RST_FLAG_DO_WHOLE_RESET);
		temp[0] = 'X';
	}
#endif

	while (temp) {
		if (sscanf(temp,
			"0x%x:0x%x", &u4NewDbgModule, &u4NewDbgLevel) != 2) {
			pr_info("debug module and debug level should be one byte in length\n");
			break;
		}
		if (u4NewDbgModule == 0xFF) {
			wlanSetDriverDbgLevel(DBG_ALL_MODULE_IDX,
					(u4NewDbgLevel & DBG_CLASS_MASK));
			break;
		}
		if (u4NewDbgModule >= DBG_MODULE_NUM) {
			pr_info("debug module index should less than %d\n",
				DBG_MODULE_NUM);
			break;
		}
		wlanSetDriverDbgLevel(u4NewDbgModule,
				(u4NewDbgLevel & DBG_CLASS_MASK));
		temp = kalStrChr(temp, ',');
		if (!temp)
			break;
		temp++;		/* skip ',' */
	}
	return count;
}

#if CFG_WIFI_TXPWR_TBL_DUMP
#define TXPWR_TABLE_ENTRY(_siso_mcs, _cdd_mcs, _mimo_mcs, _idx)	\
{								\
	.mcs[STREAM_SISO] = _siso_mcs,				\
	.mcs[STREAM_CDD] = _cdd_mcs,				\
	.mcs[STREAM_MIMO] = _mimo_mcs,				\
	.idx = (_idx),						\
}

static struct txpwr_table_entry dsss[] = {
	TXPWR_TABLE_ENTRY("DSSS1", "", "", MODULATION_SYSTEM_CCK_1M),
	TXPWR_TABLE_ENTRY("DSSS2", "", "", MODULATION_SYSTEM_CCK_2M),
	TXPWR_TABLE_ENTRY("CCK5", "", "", MODULATION_SYSTEM_CCK_5M),
	TXPWR_TABLE_ENTRY("CCK11", "", "", MODULATION_SYSTEM_CCK_11M),
};

static struct txpwr_table_entry ofdm[] = {
	TXPWR_TABLE_ENTRY("OFDM6", "OFDM6", "", MODULATION_SYSTEM_OFDM_6M),
	TXPWR_TABLE_ENTRY("OFDM9", "OFDM9", "", MODULATION_SYSTEM_OFDM_9M),
	TXPWR_TABLE_ENTRY("OFDM12", "OFDM12", "", MODULATION_SYSTEM_OFDM_12M),
	TXPWR_TABLE_ENTRY("OFDM18", "OFDM18", "", MODULATION_SYSTEM_OFDM_18M),
	TXPWR_TABLE_ENTRY("OFDM24", "OFDM24", "", MODULATION_SYSTEM_OFDM_24M),
	TXPWR_TABLE_ENTRY("OFDM36", "OFDM36", "", MODULATION_SYSTEM_OFDM_36M),
	TXPWR_TABLE_ENTRY("OFDM48", "OFDM48", "", MODULATION_SYSTEM_OFDM_48M),
	TXPWR_TABLE_ENTRY("OFDM54", "OFDM54", "", MODULATION_SYSTEM_OFDM_54M),
};

static struct txpwr_table_entry ht20[] = {
	TXPWR_TABLE_ENTRY("MCS0", "MCS0", "MCS8", MODULATION_SYSTEM_HT20_MCS0),
	TXPWR_TABLE_ENTRY("MCS1", "MCS1", "MCS9", MODULATION_SYSTEM_HT20_MCS1),
	TXPWR_TABLE_ENTRY("MCS2", "MCS2", "MCS10", MODULATION_SYSTEM_HT20_MCS2),
	TXPWR_TABLE_ENTRY("MCS3", "MCS3", "MCS11", MODULATION_SYSTEM_HT20_MCS3),
	TXPWR_TABLE_ENTRY("MCS4", "MCS4", "MCS12", MODULATION_SYSTEM_HT20_MCS4),
	TXPWR_TABLE_ENTRY("MCS5", "MCS5", "MCS13", MODULATION_SYSTEM_HT20_MCS5),
	TXPWR_TABLE_ENTRY("MCS6", "MCS6", "MCS14", MODULATION_SYSTEM_HT20_MCS6),
	TXPWR_TABLE_ENTRY("MCS7", "MCS7", "MCS15", MODULATION_SYSTEM_HT20_MCS7),
};
static struct txpwr_table_entry ht40[] = {
	TXPWR_TABLE_ENTRY("MCS0", "MCS0", "MCS8", MODULATION_SYSTEM_HT40_MCS0),
	TXPWR_TABLE_ENTRY("MCS1", "MCS1", "MCS9", MODULATION_SYSTEM_HT40_MCS1),
	TXPWR_TABLE_ENTRY("MCS2", "MCS2", "MCS10", MODULATION_SYSTEM_HT40_MCS2),
	TXPWR_TABLE_ENTRY("MCS3", "MCS3", "MCS11", MODULATION_SYSTEM_HT40_MCS3),
	TXPWR_TABLE_ENTRY("MCS4", "MCS4", "MCS12", MODULATION_SYSTEM_HT40_MCS4),
	TXPWR_TABLE_ENTRY("MCS5", "MCS5", "MCS13", MODULATION_SYSTEM_HT40_MCS5),
	TXPWR_TABLE_ENTRY("MCS6", "MCS6", "MCS14", MODULATION_SYSTEM_HT40_MCS6),
	TXPWR_TABLE_ENTRY("MCS7", "MCS7", "MCS15", MODULATION_SYSTEM_HT40_MCS7),
	TXPWR_TABLE_ENTRY("MCS32", "MCS32", "MCS32",
		MODULATION_SYSTEM_HT40_MCS32),
};
static struct txpwr_table_entry vht[] = {
	TXPWR_TABLE_ENTRY("MCS0", "MCS0", "MCS0", MODULATION_SYSTEM_VHT20_MCS0),
	TXPWR_TABLE_ENTRY("MCS1", "MCS1", "MCS1", MODULATION_SYSTEM_VHT20_MCS1),
	TXPWR_TABLE_ENTRY("MCS2", "MCS2", "MCS2", MODULATION_SYSTEM_VHT20_MCS2),
	TXPWR_TABLE_ENTRY("MCS3", "MCS3", "MCS3", MODULATION_SYSTEM_VHT20_MCS3),
	TXPWR_TABLE_ENTRY("MCS4", "MCS4", "MCS4", MODULATION_SYSTEM_VHT20_MCS4),
	TXPWR_TABLE_ENTRY("MCS5", "MCS5", "MCS5", MODULATION_SYSTEM_VHT20_MCS5),
	TXPWR_TABLE_ENTRY("MCS6", "MCS6", "MCS6", MODULATION_SYSTEM_VHT20_MCS6),
	TXPWR_TABLE_ENTRY("MCS7", "MCS7", "MCS7", MODULATION_SYSTEM_VHT20_MCS7),
	TXPWR_TABLE_ENTRY("MCS8", "MCS8", "MCS8", MODULATION_SYSTEM_VHT20_MCS8),
	TXPWR_TABLE_ENTRY("MCS9", "MCS9", "MCS9", MODULATION_SYSTEM_VHT20_MCS9),
};

#if (CFG_WIFI_TXPWR_TBL_DUMP_HE == 1)
static struct txpwr_table_entry he[] = {
	TXPWR_TABLE_ENTRY("MCS0", "MCS0", "MCS0", MODULATION_SYSTEM_HE26_MCS0),
	TXPWR_TABLE_ENTRY("MCS1", "MCS1", "MCS1", MODULATION_SYSTEM_HE26_MCS1),
	TXPWR_TABLE_ENTRY("MCS2", "MCS2", "MCS2", MODULATION_SYSTEM_HE26_MCS2),
	TXPWR_TABLE_ENTRY("MCS3", "MCS3", "MCS3", MODULATION_SYSTEM_HE26_MCS3),
	TXPWR_TABLE_ENTRY("MCS4", "MCS4", "MCS4", MODULATION_SYSTEM_HE26_MCS4),
	TXPWR_TABLE_ENTRY("MCS5", "MCS5", "MCS5", MODULATION_SYSTEM_HE26_MCS5),
	TXPWR_TABLE_ENTRY("MCS6", "MCS6", "MCS6", MODULATION_SYSTEM_HE26_MCS6),
	TXPWR_TABLE_ENTRY("MCS7", "MCS7", "MCS7", MODULATION_SYSTEM_HE26_MCS7),
	TXPWR_TABLE_ENTRY("MCS8", "MCS8", "MCS8", MODULATION_SYSTEM_HE26_MCS8),
	TXPWR_TABLE_ENTRY("MCS9", "MCS9", "MCS9", MODULATION_SYSTEM_HE26_MCS9),
	TXPWR_TABLE_ENTRY("MCS10", "MCS10", "MCS10",
		MODULATION_SYSTEM_HE26_MCS10),
	TXPWR_TABLE_ENTRY("MCS11", "MCS11", "MCS11",
		MODULATION_SYSTEM_HE26_MCS11),
};
#endif

static struct txpwr_table txpwr_tables[] = {
	{"Legacy", dsss, ARRAY_SIZE(dsss)},
	{"11g", ofdm, ARRAY_SIZE(ofdm)},
	{"11a", ofdm, ARRAY_SIZE(ofdm)},
	{"HT20", ht20, ARRAY_SIZE(ht20)},
	{"HT40", ht40, ARRAY_SIZE(ht40)},
	{"VHT20", vht, ARRAY_SIZE(vht)},
	{"VHT40", vht, ARRAY_SIZE(vht)},
	{"VHT80", vht, ARRAY_SIZE(vht)},
	{"VHT160", vht, ARRAY_SIZE(vht)},
#if (CFG_WIFI_TXPWR_TBL_DUMP_HE == 1)
	{"HE26", he, ARRAY_SIZE(he)},
	{"HE52", he, ARRAY_SIZE(he)},
	{"HE106", he, ARRAY_SIZE(he)},
	{"HE242", he, ARRAY_SIZE(he)},
	{"HE484", he, ARRAY_SIZE(he)},
	{"HE996", he, ARRAY_SIZE(he)},
	{"HE996X2", he, ARRAY_SIZE(he)},
#endif
};

#define TMP_SZ (1024)
#define CDD_PWR_OFFSET (6)
#define TXPWR_DUMP_SZ (16384)
void print_txpwr_tbl(struct txpwr_table *txpwr_tbl, unsigned char ch,
				unsigned char *tx_pwr[], char pwr_offset[],
				char *stream_buf[], unsigned int stream_pos[])
{
	struct txpwr_table_entry *tmp_tbl = txpwr_tbl->tables;
	unsigned int idx, pwr_idx, stream_idx;
	char pwr[TXPWR_TBL_NUM] = {0}, tmp_pwr = 0;
	char prefix[5], tmp[4];
	char *buf = NULL;
	unsigned int *pos = NULL;
	int i;

	DBGLOG(REQ, INFO, "Enter print_txpwr_tbl\n");

	/* n_tables: MCS number of each modulation */
	for (i = 0; i < txpwr_tbl->n_tables; i++) {
		idx = tmp_tbl[i].idx;

		for (pwr_idx = 0; pwr_idx < TXPWR_TBL_NUM; pwr_idx++) {
			if (!tx_pwr[pwr_idx]) {
				DBGLOG(REQ, WARN,
				       "Power table[%d] is NULL\n", pwr_idx);
				return;
			}
			pwr[pwr_idx] = tx_pwr[pwr_idx][idx] +
				       pwr_offset[pwr_idx];
			pwr[pwr_idx] = (pwr[pwr_idx] > MAX_TX_POWER) ?
				       MAX_TX_POWER : pwr[pwr_idx];
		}

		for (stream_idx = 0; stream_idx < STREAM_NUM; stream_idx++) {
			buf = stream_buf[stream_idx];
			pos = &stream_pos[stream_idx];

			if (tmp_tbl[i].mcs[stream_idx][0] == '\0')
				continue;

			switch (stream_idx) {
			case STREAM_SISO:
				kalStrnCpy(prefix, "siso", sizeof(prefix));
				break;
			/*
			 * CDD offset did not include
			 * The CDD values are the same as the values of SISO
			 */
			case STREAM_CDD:
				kalStrnCpy(prefix, "cdd", sizeof(prefix));
				break;
			case STREAM_MIMO:
				kalStrnCpy(prefix, "mimo", sizeof(prefix));
				break;
			}

			*pos += kalScnprintf(buf + *pos, TMP_SZ - *pos,
				"%s, %d, %s, %s, ",
				prefix, ch,
				txpwr_tbl->phy_mode,
				tmp_tbl[i].mcs[stream_idx]);

			for (pwr_idx = 0; pwr_idx < TXPWR_TBL_NUM; pwr_idx++) {
				tmp_pwr = pwr[pwr_idx];

				tmp_pwr = (tmp_pwr > 0) ? tmp_pwr : 0;

				if (pwr_idx + 1 == TXPWR_TBL_NUM)
					kalStrnCpy(tmp, "\n", sizeof(tmp));
				else
					kalStrnCpy(tmp, ", ", sizeof(tmp));
				*pos += kalScnprintf(buf + *pos, TMP_SZ - *pos,
					"%d.%d%s",
					tmp_pwr / 2,
					tmp_pwr % 2 * 5,
					tmp);
			}
		}
	}
}

char *g_txpwr_tbl_read_buffer;
char *g_txpwr_tbl_read_buffer_head;
unsigned int g_txpwr_tbl_read_residual;

static ssize_t procGetTxpwrTblRead(struct file *filp, char __user *buf,
				   size_t count, loff_t *f_pos)
{
	struct GLUE_INFO *prGlueInfo = NULL;
	struct ADAPTER  *prAdapter = NULL;
	struct BSS_INFO *prBssInfo = NULL;
	unsigned char ucBssIndex;
	struct NETDEV_PRIVATE_GLUE_INFO *prNetDevPrivate = NULL;
	uint32_t status;
	struct PARAM_CMD_GET_TXPWR_TBL pwr_tbl;
	struct POWER_LIMIT *tx_pwr_tbl = pwr_tbl.tx_pwr_tbl;
	char *buffer;
	unsigned int pos = 0, buf_len = TXPWR_DUMP_SZ, oid_len;
	unsigned char i, j;
	char *stream_buf[STREAM_NUM] = {NULL};
	unsigned int stream_pos[STREAM_NUM] = {0};
	unsigned char *tx_pwr[TXPWR_TBL_NUM] =  {NULL};
	char pwr_offset[TXPWR_TBL_NUM] = {0};
	int ret;

	DBGLOG(REQ, INFO, "Enter procGetTxpwrTblRead\n");

	/* Re-entry to the func to print the remaining table */
	if (*f_pos > 0) { /* re-entry */
		pos = g_txpwr_tbl_read_residual;
		buffer = g_txpwr_tbl_read_buffer;
		goto next_entry;
	}

	if (buf == NULL) {
		DBGLOG(REQ, WARN, "empty buf: exit cat");
		return 0;
	}

	prGlueInfo = g_prGlueInfo_proc;
	if (!prGlueInfo) {
		DBGLOG(REQ, WARN, "can't get glue info");
		return -EFAULT;
	}

	prAdapter = prGlueInfo->prAdapter;

	prNetDevPrivate =
		(struct NETDEV_PRIVATE_GLUE_INFO *) netdev_priv(gPrDev);
	if (prNetDevPrivate->prGlueInfo != prGlueInfo) {
		DBGLOG(REQ, WARN, "glue info are not the same");
		return -EFAULT;
	}

	ucBssIndex = prNetDevPrivate->ucBssIdx;
	prBssInfo = prAdapter->aprBssInfo[ucBssIndex];
	if (!prBssInfo) {
		DBGLOG(REQ, WARN, "can't get the BssInfo from adapter");
		return -EFAULT;
	}

	kalMemZero(&pwr_tbl, sizeof(pwr_tbl));


	/* Complete the cmd/event process */
	status = kalIoctl(prGlueInfo,
			  wlanoidGetTxPwrTbl,
			  &pwr_tbl,
			  sizeof(pwr_tbl), TRUE, FALSE, TRUE, &oid_len);

	if (status != WLAN_STATUS_SUCCESS) {
		DBGLOG(REQ, WARN, "Query Tx Power Table fail\n");
		return -EINVAL;
	}
	DBGLOG(REQ, INFO, "Query Tx Power Table success\n");


	buffer = (char *) kalMemAlloc(buf_len, VIR_MEM_TYPE);
	if (!buffer) {
		return -ENOMEM;
		DBGLOG(REQ, WARN, "buffer is empty\n");
	}

	g_txpwr_tbl_read_buffer = buffer;
	g_txpwr_tbl_read_buffer_head = buffer;

	for (i = 0; i < STREAM_NUM; i++) {
		stream_buf[i] = (char *) kalMemAlloc(TMP_SZ, VIR_MEM_TYPE);
		if (!stream_buf[i]) {
			ret = -ENOMEM;
			goto out;
		}
	}
	DBGLOG(REQ, INFO, "stream init\n");


	pos = kalScnprintf(buffer, buf_len,
				"\n%s",
				"spatial stream, Channel, bw, modulation, ");
	pos += kalScnprintf(buffer + pos, buf_len - pos,
				"%s\n",
				"regulatory limit, board limit, target power");

	for (i = 0; i < ARRAY_SIZE(txpwr_tables); i++) {
		for (j = 0; j < STREAM_NUM; j++) {
			kalMemZero(stream_buf[j], TMP_SZ);
			stream_pos[j] = 0;
		}

		for (j = 0; j < TXPWR_TBL_NUM; j++) {
			tx_pwr[j] = NULL;
			pwr_offset[j] = 0;
		}

		/*
		 * In the rate is not supported on this channel,
		 * its limit table value will be the default 127
		*/

		switch (i) {
		case DSSS:
			if (pwr_tbl.ucCenterCh > 14)
				continue;
			DBGLOG(REQ, INFO, "Print DSSS table\n");
			for (j = 0; j < TXPWR_TBL_NUM; j++)
				tx_pwr[j] = tx_pwr_tbl[j].tx_pwr_dsss;
			break;
		case OFDM_24G:
			if (pwr_tbl.ucCenterCh > 14)
				continue;
			DBGLOG(REQ, INFO, "Print OFDM_24G table\n");
			for (j = 0; j < TXPWR_TBL_NUM; j++)
				tx_pwr[j] = tx_pwr_tbl[j].tx_pwr_ofdm;
			break;
		case OFDM_5G:
			if (pwr_tbl.ucCenterCh <= 14)
				continue;
			DBGLOG(REQ, INFO, "Print OFDM_5G table\n");
			for (j = 0; j < TXPWR_TBL_NUM; j++)
				tx_pwr[j] = tx_pwr_tbl[j].tx_pwr_ofdm;
			break;
		case HT20:
			DBGLOG(REQ, INFO, "Print HT20 table\n");
			for (j = 0; j < TXPWR_TBL_NUM; j++)
				tx_pwr[j] = tx_pwr_tbl[j].tx_pwr_ht20;
			break;
		case HT40:
			if (pwr_tbl.ucCenterCh <= 14 ||
					tx_pwr_tbl[0].tx_pwr_ht40[0] >= 127)
				continue;
			DBGLOG(REQ, INFO, "Print HT40 table\n");
			for (j = 0; j < TXPWR_TBL_NUM; j++)
				tx_pwr[j] = tx_pwr_tbl[j].tx_pwr_ht40;
			break;
		case VHT20:
			DBGLOG(REQ, INFO, "Print VHT20 table\n");
			for (j = 0; j < TXPWR_TBL_NUM; j++)
				tx_pwr[j] = tx_pwr_tbl[j].tx_pwr_vht20;
			break;
		case VHT40:
			if (pwr_tbl.ucCenterCh <= 14 ||
					tx_pwr_tbl[0].tx_pwr_vht40[0] >= 127)
				continue;
			DBGLOG(REQ, INFO, "Print VHT40 table\n");
			for (j = 0; j < TXPWR_TBL_NUM; j++)
				tx_pwr[j] = tx_pwr_tbl[j].tx_pwr_vht40;
			break;
		case VHT80:
			if (pwr_tbl.ucCenterCh <= 14 ||
					tx_pwr_tbl[0].tx_pwr_vht80[0] >= 127)
				continue;
			DBGLOG(REQ, INFO, "Print VHT80 table\n");
			for (j = 0; j < TXPWR_TBL_NUM; j++)
				tx_pwr[j] = tx_pwr_tbl[j].tx_pwr_vht80;
			break;
		case VHT160:
			if (pwr_tbl.ucCenterCh <= 14 ||
					tx_pwr_tbl[0].tx_pwr_vht160[0] >= 127)
				continue;
			DBGLOG(REQ, INFO, "Print VHT160 table\n");
			for (j = 0; j < TXPWR_TBL_NUM; j++)
				tx_pwr[j] = tx_pwr_tbl[j].tx_pwr_vht160;
			break;
#if (CFG_WIFI_TXPWR_TBL_DUMP_HE == 1)
		case HE26:
			DBGLOG(REQ, INFO, "Print HE26 table\n");
			for (j = 0; j < TXPWR_TBL_NUM; j++)
				tx_pwr[j] = tx_pwr_tbl[j].tx_pwr_he26;
			break;
		case HE52:
			DBGLOG(REQ, INFO, "Print HE52 table\n");
			for (j = 0; j < TXPWR_TBL_NUM; j++)
				tx_pwr[j] = tx_pwr_tbl[j].tx_pwr_he52;
			break;
		case HE106:
			DBGLOG(REQ, INFO, "Print HE106 table\n");
			for (j = 0; j < TXPWR_TBL_NUM; j++)
				tx_pwr[j] = tx_pwr_tbl[j].tx_pwr_he106;
			break;
		case HE242:
			DBGLOG(REQ, INFO, "Print HE242 table\n");
			for (j = 0; j < TXPWR_TBL_NUM; j++)
				tx_pwr[j] = tx_pwr_tbl[j].tx_pwr_he242;
			break;
		case HE484:
			if (pwr_tbl.ucCenterCh <= 14 ||
					tx_pwr_tbl[0].tx_pwr_he484[0] >= 127)
				continue;
			DBGLOG(REQ, INFO, "Print HE484 table\n");
			for (j = 0; j < TXPWR_TBL_NUM; j++)
				tx_pwr[j] = tx_pwr_tbl[j].tx_pwr_he484;
			break;
		case HE996:
			if (pwr_tbl.ucCenterCh <= 14 ||
					tx_pwr_tbl[0].tx_pwr_he996[0] >= 127)
				continue;
			DBGLOG(REQ, INFO, "Print HE996 table\n");
			for (j = 0; j < TXPWR_TBL_NUM; j++)
				tx_pwr[j] = tx_pwr_tbl[j].tx_pwr_he996;
			break;
		case HE996X2:
			if (pwr_tbl.ucCenterCh <= 14 ||
					tx_pwr_tbl[0].tx_pwr_he996x2[0] >= 127)
				continue;
			DBGLOG(REQ, INFO, "Print HE996X2 table\n");
			for (j = 0; j < TXPWR_TBL_NUM; j++)
				tx_pwr[j] = tx_pwr_tbl[j].tx_pwr_he996x2;
			break;
#endif
		default:
			break;
		}

		print_txpwr_tbl(&txpwr_tables[i], pwr_tbl.ucCenterCh,
				tx_pwr, pwr_offset,
				stream_buf, stream_pos);

		for (j = 0; j < STREAM_NUM; j++) {
			pos += kalScnprintf(buffer + pos, buf_len - pos,
				"%s",
				stream_buf[j]);
		}
	}

	g_txpwr_tbl_read_residual = pos;

next_entry:
	if (pos > count)
		pos = count;

	if (copy_to_user(buf, buffer, pos)) {
		DBGLOG(INIT, WARN, "copy to user failed\n");
		ret = -EFAULT;
		goto out;
	}

	g_txpwr_tbl_read_buffer += pos;
	g_txpwr_tbl_read_residual -= pos;

	*f_pos += pos;
	ret = pos;
out:
	if (ret == 0 || ret == -ENOMEM) {
		for (i = 0; i < STREAM_NUM; i++) {
			if (stream_buf[i])
				kalMemFree(stream_buf[i], VIR_MEM_TYPE, TMP_SZ);
		}
		if (g_txpwr_tbl_read_buffer_head)
			kalMemFree(g_txpwr_tbl_read_buffer_head,
				VIR_MEM_TYPE, buf_len);

		g_txpwr_tbl_read_buffer = NULL;
		g_txpwr_tbl_read_buffer_head = NULL;
		g_txpwr_tbl_read_residual = 0;
	}

	return ret;
}
#endif /* CFG_WIFI_TXPWR_TBL_DUMP */

#if CFG_SUPPORT_PROC_GET_WAKEUP_REASON
static ssize_t procWakeupReasonRead(struct file *filp,
				    char __user *buf,
				    size_t count,
				    loff_t *f_pos)
{
	struct GLUE_INFO *prGlueInfo = NULL;
	struct WOW_CTRL *prWOW_CTRL = NULL;
	unsigned int pos = 0, buf_len = 128;
	char *temp = &g_aucProcBuf[0];

	if (*f_pos > 0)
		return 0;

	prGlueInfo = g_prGlueInfo_proc;
	if (!prGlueInfo)
		return -EFAULT;

	prWOW_CTRL = &prGlueInfo->prAdapter->rWowCtrl;

	kalMemZero(g_aucProcBuf, sizeof(g_aucProcBuf));

	if (prWOW_CTRL->ucReason != INVALID_WOW_WAKE_UP_REASON) {
		pos = kalScnprintf(temp, buf_len, "%d\n", prWOW_CTRL->ucReason);
	} else {
		DBGLOG(INIT, ERROR, "cat wakeup reason fs: no wakeup reason\n");
		return 0;
	}

	if (copy_to_user(buf, temp, pos)) {
		DBGLOG(INIT, ERROR, "copy to user failed\n");
		return -EFAULT;
	}

	*f_pos += pos;

	return pos;
}

static DEFINE_PROC_OPS_STRUCT(wakeup_reason_ops) = {
	DEFINE_PROC_OPS_OWNER(THIS_MODULE)
	DEFINE_PROC_OPS_READ(procWakeupReasonRead)
};
#endif

static DEFINE_PROC_OPS_STRUCT(dbglevel_ops) = {
	DEFINE_PROC_OPS_OWNER(THIS_MODULE)
	DEFINE_PROC_OPS_READ(procDbgLevelRead)
	DEFINE_PROC_OPS_WRITE(procDbgLevelWrite)
};

#if CFG_ASSERT_DUMP
DEFINE_PROC_OPS_STRUCT(coredump_ops) = {
	DEFINE_PROC_OPS_OWNER(THIS_MODULE)
	DEFINE_PROC_OPS_READ(procCoreDumpRead)
	DEFINE_PROC_OPS_POLL(procCoreDumpPoll)
};
#endif

#if WLAN_INCLUDE_PROC
#if	CFG_SUPPORT_EASY_DEBUG

static DEFINE_PROC_OPS_STRUCT(efusedump_ops) = {
	DEFINE_PROC_OPS_OWNER(THIS_MODULE)
	DEFINE_PROC_OPS_OPEN(procEfuseDumpOpen)
	DEFINE_PROC_OPS_READ(seq_read)
	DEFINE_PROC_OPS_LSEEK(seq_lseek)
	DEFINE_PROC_OPS_RELEASE(seq_release)
};

static DEFINE_PROC_OPS_STRUCT(drivercmd_ops) = {
	DEFINE_PROC_OPS_OWNER(THIS_MODULE)
	DEFINE_PROC_OPS_READ(procDriverCmdRead)
	DEFINE_PROC_OPS_WRITE(procDriverCmdWrite)
};
#if CFG_SUPPORT_CFG_FILE
static DEFINE_PROC_OPS_STRUCT(cfg_ops) = {
	DEFINE_PROC_OPS_OWNER(THIS_MODULE)
	DEFINE_PROC_OPS_READ(procCfgRead)
	DEFINE_PROC_OPS_WRITE(procCfgWrite)
};
#endif
#endif
#endif

#if CFG_WIFI_TXPWR_TBL_DUMP
static DEFINE_PROC_OPS_STRUCT(get_txpwr_tbl_ops) = {
	DEFINE_PROC_OPS_OWNER(THIS_MODULE)
	DEFINE_PROC_OPS_READ(procGetTxpwrTblRead)
};
#endif

/*******************************************************************************
 *                              F U N C T I O N S
 *******************************************************************************
 */
/*----------------------------------------------------------------------------*/
/*!
 * \brief The PROC function for reading MCR register to User Space, the offset
 *        of the MCR is specified in u4McrOffset.
 *
 * \param[in] page       Buffer provided by kernel.
 * \param[in out] start  Start Address to read(3 methods).
 * \param[in] off        Offset.
 * \param[in] count      Allowable number to read.
 * \param[out] eof       End of File indication.
 * \param[in] data       Pointer to the private data structure.
 *
 * \return number of characters print to the buffer from User Space.
 */
/*----------------------------------------------------------------------------*/
static ssize_t procMCRRead(struct file *filp, char __user *buf,
	 size_t count, loff_t *f_pos)
{
	struct GLUE_INFO *prGlueInfo;
	struct PARAM_CUSTOM_MCR_RW_STRUCT rMcrInfo;
	uint32_t u4BufLen;
	uint32_t u4Count;
	uint8_t *temp = &g_aucProcBuf[0];
	uint32_t rStatus = WLAN_STATUS_SUCCESS;

	/* Kevin: Apply PROC read method 1. */
	if (*f_pos > 0)
		return 0;	/* To indicate end of file. */

	prGlueInfo = g_prGlueInfo_proc;
	rMcrInfo.u4McrData = 0;
	rMcrInfo.u4McrOffset = u4McrOffset;

	rStatus = kalIoctl(prGlueInfo,
		wlanoidQueryMcrRead, (void *)&rMcrInfo,
		sizeof(rMcrInfo), TRUE, TRUE, TRUE, &u4BufLen);
	kalMemZero(g_aucProcBuf, sizeof(g_aucProcBuf));
	SNPRINTF(temp, g_aucProcBuf,
		("MCR (0x%08xh): 0x%08x\n", rMcrInfo.u4McrOffset,
		rMcrInfo.u4McrData));

	u4Count = kalStrLen(g_aucProcBuf);
	if (copy_to_user(buf, g_aucProcBuf, u4Count)) {
		DBGLOG(INIT, ERROR, "copy to user failed\n");
		return -EFAULT;
	}

	*f_pos += u4Count;

	return (int)u4Count;

} /* end of procMCRRead() */

/*----------------------------------------------------------------------------*/
/*!
 * \brief The PROC function for writing MCR register to HW or update u4McrOffset
 *        for reading MCR later.
 *
 * \param[in] file   pointer to file.
 * \param[in] buffer Buffer from user space.
 * \param[in] count  Number of characters to write
 * \param[in] data   Pointer to the private data structure.
 *
 * \return number of characters write from User Space.
 */
/*----------------------------------------------------------------------------*/
static ssize_t procMCRWrite(struct file *file, const char __user *buffer,
	size_t count, loff_t *data)
{
	struct GLUE_INFO *prGlueInfo;
	/* + 1 for "\0" */
	char acBuf[PROC_MCR_ACCESS_MAX_USER_INPUT_LEN + 1];
	int i4CopySize;
	struct PARAM_CUSTOM_MCR_RW_STRUCT rMcrInfo;
	uint32_t u4BufLen;
	uint32_t rStatus = WLAN_STATUS_SUCCESS;
	int num = 0;

	ASSERT(data);

	i4CopySize =
	    (count < (sizeof(acBuf) - 1)) ? count : (sizeof(acBuf) - 1);
	if (i4CopySize < 0 ||
		i4CopySize > PROC_MCR_ACCESS_MAX_USER_INPUT_LEN ||
		copy_from_user(acBuf, buffer, i4CopySize))
		return 0;
	acBuf[i4CopySize] = '\0';

	num =
	    sscanf(acBuf, "0x%x 0x%x", &rMcrInfo.u4McrOffset,
		   &rMcrInfo.u4McrData);
	switch (num) {
	case 2:
		/* NOTE: Sometimes we want to test if bus will still be ok,
		 * after accessing the MCR which is not align to DW boundary.
		 */
		/* if (IS_ALIGN_4(rMcrInfo.u4McrOffset)) */
		{
			prGlueInfo = g_prGlueInfo_proc;

			u4McrOffset = rMcrInfo.u4McrOffset;

			/* rMcrInfo.u4McrOffset, rMcrInfo.u4McrData); */

			rStatus = kalIoctl(prGlueInfo,
					   wlanoidSetMcrWrite,
					   (void *)&rMcrInfo, sizeof(rMcrInfo),
					   FALSE, FALSE, TRUE, &u4BufLen);

		}
		break;
	case 1:
		/* if (IS_ALIGN_4(rMcrInfo.u4McrOffset)) */
		{
			u4McrOffset = rMcrInfo.u4McrOffset;
		}
		break;

	default:
		break;
	}

	return count;

}				/* end of procMCRWrite() */

static DEFINE_PROC_OPS_STRUCT(mcr_ops) = {
	DEFINE_PROC_OPS_OWNER(THIS_MODULE)
	DEFINE_PROC_OPS_READ(procMCRRead)
	DEFINE_PROC_OPS_WRITE(procMCRWrite)
};

#if CFG_SUPPORT_SET_CAM_BY_PROC
static ssize_t procSetCamCfgWrite(struct file *file, const char __user *buffer,
	size_t count, loff_t *data)
{
#define MODULE_NAME_LEN_1 5

	uint32_t u4CopySize = sizeof(g_aucProcBuf);
	uint8_t *temp = &g_aucProcBuf[0];
	u_int8_t fgSetCamCfg = FALSE;
	uint8_t aucModule[MODULE_NAME_LEN_1];
	uint32_t u4Enabled;
	uint8_t aucModuleArray[MODULE_NAME_LEN_1] = "CAM";
	u_int8_t fgParamValue = TRUE;
	struct GLUE_INFO *prGlueInfo = NULL;
	struct ADAPTER *prAdapter = NULL;

	kalMemSet(g_aucProcBuf, 0, u4CopySize);
	u4CopySize = (count < u4CopySize) ? count : (u4CopySize - 1);

	if (copy_from_user(g_aucProcBuf, buffer, u4CopySize)) {
		DBGLOG(INIT, ERROR, "error of copy from user\n");
		return -EFAULT;
	}
	g_aucProcBuf[u4CopySize] = '\0';
	temp = &g_aucProcBuf[0];
	while (temp) {
		kalMemSet(aucModule, 0, MODULE_NAME_LEN_1);

		/* pick up a string and teminated after meet : */
		if (sscanf(temp, "%4s %d", aucModule, &u4Enabled) != 2) {
			pr_info("read param fail, aucModule=%s\n", aucModule);
			fgParamValue = FALSE;
			break;
		}

		if (kalStrnCmp
			(aucModule, aucModuleArray, MODULE_NAME_LEN_1) == 0) {
			if (u4Enabled)
				fgSetCamCfg = TRUE;
			else
				fgSetCamCfg = FALSE;
		}
		temp = kalStrChr(temp, ',');
		if (!temp)
			break;
		temp++;		/* skip ',' */
	}

	if (fgParamValue) {
		uint8_t i;

		prGlueInfo = wlanGetGlueInfo();
		if (!prGlueInfo)
			return count;

		prAdapter = prGlueInfo->prAdapter;
		if (!prAdapter)
			return count;

		for (i = 0; i < KAL_AIS_NUM; i++) {
			nicConfigProcSetCamCfgWrite(prAdapter,
				fgSetCamCfg,
				i);
		}
	}

	return count;
}

static DEFINE_PROC_OPS_STRUCT(proc_set_cam_ops) = {
	DEFINE_PROC_OPS_OWNER(THIS_MODULE)
	DEFINE_PROC_OPS_WRITE(procSetCamCfgWrite)
};
#endif /*CFG_SUPPORT_SET_CAM_BY_PROC */

static ssize_t procPktDelayDbgCfgRead(struct file *filp, char __user *buf,
	size_t count, loff_t *f_pos)
{
	uint8_t *temp = &g_aucProcBuf[0];
	uint8_t *str = NULL;
	uint32_t u4CopySize = 0;
	uint8_t ucTxRxFlag;
	uint8_t ucTxIpProto;
	uint16_t u2TxUdpPort;
	uint32_t u4TxDelayThreshold;
	uint8_t ucRxIpProto;
	uint16_t u2RxUdpPort;
	uint32_t u4RxDelayThreshold;
	uint32_t u4StrLen = 0;

	/* if *f_ops>0, we should return 0 to make cat command exit */
	if (*f_pos > 0 || buf == NULL)
		return 0;

	str = "\nUsage: txLog/rxLog/reset 1(ICMP)/6(TCP)/11(UDP) Dst/SrcPortNum DelayThreshold(us)\n"
		"Print tx delay log,                                   such as: echo txLog 0 0 0 > pktDelay\n"
		"Print tx UDP delay log,                               such as: echo txLog 11 0 0 > pktDelay\n"
		"Print tx UDP dst port19305 delay log,                 such as: echo txLog 11 19305 0 > pktDelay\n"
		"Print rx UDP src port19305 delay more than 500us log, such as: echo rxLog 11 19305 500 > pktDelay\n"
		"Print tx TCP delay more than 500us log,               such as: echo txLog 6 0 500 > pktDelay\n"
		"Close log,                                            such as: echo reset 0 0 0 > pktDelay\n\n";
	u4StrLen = kalStrLen(str);
	kalStrnCpy(temp, str, u4StrLen);
	temp[u4StrLen] = '\0';
	temp += u4StrLen;

	StatsEnvGetPktDelay(&ucTxRxFlag, &ucTxIpProto, &u2TxUdpPort,
			&u4TxDelayThreshold, &ucRxIpProto, &u2RxUdpPort,
			&u4RxDelayThreshold);

	if (ucTxRxFlag & BIT(0)) {
		SNPRINTF(temp, g_aucProcBuf,
			("txLog %x %d %d\n", ucTxIpProto, u2TxUdpPort,
			u4TxDelayThreshold));
		temp += kalStrLen(temp);
	}
	if (ucTxRxFlag & BIT(1)) {
		SNPRINTF(temp, g_aucProcBuf,
			("rxLog %x %d %d\n", ucRxIpProto, u2RxUdpPort,
			u4RxDelayThreshold));
		temp += kalStrLen(temp);
	}
	if (ucTxRxFlag == 0)
		SNPRINTF(temp, g_aucProcBuf,
			("reset 0 0 0, there is no tx/rx delay log\n"));

	u4CopySize = kalStrLen(g_aucProcBuf);
	if (u4CopySize > count)
		u4CopySize = count;
	if (copy_to_user(buf, g_aucProcBuf, u4CopySize)) {
		DBGLOG(INIT, ERROR, "copy to user failed\n");
		return -EFAULT;
	}

	*f_pos += u4CopySize;
	return (ssize_t) u4CopySize;
}

static ssize_t procPktDelayDbgCfgWrite(struct file *file, const char *buffer,
	size_t count, loff_t *data)
{
#define MODULE_NAME_LENGTH 7
#define MODULE_RESET 0
#define MODULE_TX 1
#define MODULE_RX 2

	uint32_t u4CopySize = sizeof(g_aucProcBuf);
	uint8_t *temp = &g_aucProcBuf[0];
	uint8_t aucModule[MODULE_NAME_LENGTH];
	uint32_t u4DelayThreshold = 0;
	uint32_t u4PortNum = 0;
	uint32_t u4IpProto = 0;
	uint8_t aucResetArray[MODULE_NAME_LENGTH] = "reset";
	uint8_t aucTxArray[MODULE_NAME_LENGTH] = "txLog";
	uint8_t aucRxArray[MODULE_NAME_LENGTH] = "rxLog";
	uint8_t ucTxOrRx = 0;

	kalMemSet(g_aucProcBuf, 0, u4CopySize);
	u4CopySize = (count < u4CopySize) ? count : (u4CopySize - 1);

	if (copy_from_user(g_aucProcBuf, buffer, u4CopySize)) {
		DBGLOG(INIT, ERROR, "error of copy from user\n");
		return -EFAULT;
	}
	g_aucProcBuf[u4CopySize] = '\0';

	while (temp) {
		kalMemSet(aucModule, 0, MODULE_NAME_LENGTH);

		/* pick up a string and teminated after meet : */
		if (sscanf
		    (temp, "%6s %x %d %d", aucModule, &u4IpProto, &u4PortNum,
		     &u4DelayThreshold) != 4) {
			pr_info("read param fail, aucModule=%s\n", aucModule);
			break;
		}

		if (kalStrnCmp
			(aucModule, aucResetArray, MODULE_NAME_LENGTH) == 0) {
			ucTxOrRx = MODULE_RESET;
		} else if (kalStrnCmp
			(aucModule, aucTxArray, MODULE_NAME_LENGTH) == 0) {
			ucTxOrRx = MODULE_TX;
		} else if (kalStrnCmp
			(aucModule, aucRxArray, MODULE_NAME_LENGTH) == 0) {
			ucTxOrRx = MODULE_RX;
		} else {
			pr_info("input module error!\n");
			break;
		}

		temp = kalStrChr(temp, ',');
		if (!temp)
			break;
		temp++;		/* skip ',' */
	}

	StatsEnvSetPktDelay(ucTxOrRx, (uint8_t) u4IpProto, (uint16_t) u4PortNum,
		u4DelayThreshold);

	return count;
}

static DEFINE_PROC_OPS_STRUCT(proc_pkt_delay_dbg_ops) = {
	DEFINE_PROC_OPS_OWNER(THIS_MODULE)
	DEFINE_PROC_OPS_READ(procPktDelayDbgCfgRead)
	DEFINE_PROC_OPS_WRITE(procPktDelayDbgCfgWrite)
};

#if CFG_SUPPORT_DEBUG_FS
static ssize_t procRoamRead(struct file *filp, char __user *buf,
	size_t count, loff_t *f_pos)
{
	uint32_t u4CopySize;
	uint32_t rStatus;
	uint32_t u4BufLen;

	/* if *f_pos > 0, it means has read successed last time,
	 * don't try again
	 */
	if (*f_pos > 0 || buf == NULL)
		return 0;

	rStatus =
	    kalIoctl(g_prGlueInfo_proc, wlanoidGetRoamParams, g_aucProcBuf,
		     sizeof(g_aucProcBuf), TRUE, FALSE, TRUE, &u4BufLen);
	if (rStatus != WLAN_STATUS_SUCCESS) {
		DBGLOG(INIT, INFO, "failed to read roam params\n");
		return -EINVAL;
	}

	u4CopySize = kalStrLen(g_aucProcBuf);
	if (copy_to_user(buf, g_aucProcBuf, u4CopySize)) {
		DBGLOG(INIT, ERROR, "copy to user failed\n");
		return -EFAULT;
	}
	*f_pos += u4CopySize;

	return (int32_t) u4CopySize;
}

static ssize_t procRoamWrite(struct file *file, const char __user *buffer,
	size_t count, loff_t *data)
{
	uint32_t rStatus;
	uint32_t u4BufLen = 0;
	uint32_t u4CopySize = sizeof(g_aucProcBuf);

	kalMemSet(g_aucProcBuf, 0, u4CopySize);
	u4CopySize = (count < u4CopySize) ? count : (u4CopySize - 1);

	if (copy_from_user(g_aucProcBuf, buffer, u4CopySize)) {
		DBGLOG(INIT, ERROR, "error of copy from user\n");
		return -EFAULT;
	}
	g_aucProcBuf[u4CopySize] = '\0';

	if (kalStrnCmp(g_aucProcBuf, "force_roam", 10) == 0)
		rStatus =
		    kalIoctl(g_prGlueInfo_proc, wlanoidSetForceRoam, NULL, 0,
			     FALSE, FALSE, TRUE, &u4BufLen);
	else
		rStatus =
		    kalIoctl(g_prGlueInfo_proc, wlanoidSetRoamParams,
			     g_aucProcBuf, kalStrLen(g_aucProcBuf), FALSE,
			     FALSE, TRUE, &u4BufLen);

	if (rStatus != WLAN_STATUS_SUCCESS) {
		DBGLOG(INIT, INFO, "failed to set roam params: %s\n",
		       g_aucProcBuf);
		return -EINVAL;
	}
	return count;
}

static const struct file_operations roam_ops = {
	.owner = THIS_MODULE,
	.read = procRoamRead,
	.write = procRoamWrite,
};
#endif

static ssize_t procCountryRead(struct file *filp, char __user *buf,
	size_t count, loff_t *f_pos)
{
	uint32_t u4CopySize;
	uint32_t country = 0;
	char acCountryStr[MAX_COUNTRY_CODE_LEN + 1] = {0};

	/* if *f_pos > 0, it means has read successed last time */
	if (*f_pos > 0)
		return 0;

	country = rlmDomainGetCountryCode();
	rlmDomainU32ToAlpha(country, acCountryStr);

	kalMemZero(g_aucProcBuf, sizeof(g_aucProcBuf));
	if (country)
		kalSnprintf(g_aucProcBuf, sizeof(g_aucProcBuf),
			"Current Country Code: %s\n", acCountryStr);
	else
		kalSnprintf(g_aucProcBuf, sizeof(g_aucProcBuf),
			"Current Country Code: NULL\n");

	u4CopySize = kalStrLen(g_aucProcBuf);
	if (copy_to_user(buf, g_aucProcBuf, u4CopySize)) {
		DBGLOG(INIT, ERROR, "copy to user failed\n");
		return -EFAULT;
	}
	*f_pos += u4CopySize;

	return (int32_t) u4CopySize;
}

static ssize_t procCountryWrite(struct file *file, const char __user *buffer,
	size_t count, loff_t *data)
{
	uint32_t u4BufLen = 0;
	uint32_t rStatus;
	uint32_t u4CopySize = sizeof(g_aucProcBuf);

	kalMemSet(g_aucProcBuf, 0, u4CopySize);
	u4CopySize = (count < u4CopySize) ? count : (u4CopySize - 1);

	if (copy_from_user(g_aucProcBuf, buffer, u4CopySize)) {
		DBGLOG(INIT, ERROR, "error of copy from user\n");
		return -EFAULT;
	}
	g_aucProcBuf[u4CopySize] = '\0';

	rStatus = kalIoctl(g_prGlueInfo_proc, wlanoidSetCountryCode,
			   &g_aucProcBuf[0], 2, FALSE, FALSE, TRUE, &u4BufLen);
	if (rStatus != WLAN_STATUS_SUCCESS) {
		DBGLOG(INIT, INFO, "failed set country code: %s\n",
			g_aucProcBuf);
		return -EINVAL;
	}
	return count;
}

static DEFINE_PROC_OPS_STRUCT(country_ops) = {
	DEFINE_PROC_OPS_OWNER(THIS_MODULE)
	DEFINE_PROC_OPS_READ(procCountryRead)
	DEFINE_PROC_OPS_WRITE(procCountryWrite)
};

#if (CFG_SUPPORT_PERMON == 1)
static ssize_t procAutoPerfCfgRead(struct file *filp, char __user *buf,
	size_t count, loff_t *f_pos)
{
	uint8_t *temp = &g_aucProcBuf[0];
	uint8_t *str = NULL;
	uint32_t u4CopySize = 0;
	uint32_t u4StrLen = 0;

	/* if *f_ops>0, we should return 0 to make cat command exit */
	if (*f_pos > 0)
		return 0;

	str = "Auto Performance Configure Usage:\n"
	    "\n"
	    "echo ForceEnable:0 or 1 > /proc/net/wlan/autoPerfCfg\n"
	    "     1: always enable performance monitor\n"
	    "     0: restore performance monitor's default strategy\n";
	u4StrLen = kalStrLen(str);
	kalStrnCpy(temp, str, u4StrLen + 1);

	u4CopySize = kalStrLen(g_aucProcBuf);
	if (u4CopySize > count)
		u4CopySize = count;

	if (copy_to_user(buf, g_aucProcBuf, u4CopySize)) {
		DBGLOG(INIT, WARN, "copy_to_user error\n");
		return -EFAULT;
	}

	*f_pos += u4CopySize;
	return (ssize_t) u4CopySize;
}

static ssize_t procAutoPerfCfgWrite(struct file *file, const char *buffer,
	size_t count, loff_t *data)
{
	uint32_t u4CoreNum = 0;
	uint32_t u4CoreFreq = 0;
	uint8_t *temp = &g_aucProcBuf[0];
	uint32_t u4CopySize = count;
	uint8_t i = 0;
	uint32_t u4ForceEnable = 0;
	uint8_t aucBuf[32];

	if (u4CopySize >= sizeof(g_aucProcBuf))
		u4CopySize = sizeof(g_aucProcBuf) - 1;

	kalMemSet(g_aucProcBuf, 0, u4CopySize);

	if (copy_from_user(g_aucProcBuf, buffer, u4CopySize)) {
		DBGLOG(INIT, WARN, "copy_from_user error\n");
		return -EFAULT;
	}

	g_aucProcBuf[u4CopySize] = '\0';

	i = sscanf(temp, "%d:%d", &u4CoreNum, &u4CoreFreq);
	if (i == 2) {
		DBGLOG(INIT, INFO, "u4CoreNum:%d, u4CoreFreq:%d\n", u4CoreNum,
			u4CoreFreq);
		kalSetCpuNumFreq(u4CoreNum, u4CoreFreq);
		return u4CopySize;
	}

	if (strlen(temp) > sizeof(aucBuf)) {
		DBGLOG(INIT, WARN,
			"input string(%s) len is too long, over %d\n",
			g_aucProcBuf, (uint32_t) sizeof(aucBuf));
		return -EFAULT;
	}

	i = sscanf(temp, "%11s:%d", aucBuf, &u4ForceEnable);

	if ((i == 2) && strstr(aucBuf, "ForceEnable")) {
		kalPerMonSetForceEnableFlag(u4ForceEnable);
		return u4CopySize;
	}

	DBGLOG(INIT, WARN, "parameter format should be ForceEnable:0 or 1\n");

	return -EFAULT;
}

static DEFINE_PROC_OPS_STRUCT(auto_perf_ops) = {
	DEFINE_PROC_OPS_OWNER(THIS_MODULE)
	DEFINE_PROC_OPS_READ(procAutoPerfCfgRead)
	DEFINE_PROC_OPS_WRITE(procAutoPerfCfgWrite)
};
#endif

#if CFG_SUPPORT_DYNAMIC_PWR_LIMIT
static ssize_t procSarCfgDebugRead(struct file *filp,
	char __user *buf, size_t count, loff_t *f_pos)
{
	uint8_t *str = NULL;
	uint8_t *temp = &g_aucProcBuf[0];
	uint32_t u4CopySize = sizeof(g_aucProcBuf);

	/* if *f_ops>0, we should return 0 to make cat command exit */
	if (*f_pos > 0 || buf == NULL)
		return 0;

	kalMemSet(temp, 0, u4CopySize);

	u4CopySize = debug_read_txPwrCtrlStringToStruct(temp, u4CopySize - 1);

	if (u4CopySize == 0) {
		str = "    cfg element is empty.\n"
		      "    write cfg string to build an new element,\n"
		      "    write 'dumpAll' to dump element list,\n"
		      "    write 'dumpElement,name,id' to dump an element.\n";
		u4CopySize = kalStrLen(str);
		kalStrnCpy(temp, str, u4CopySize);
	}
	u4CopySize++;

	if (copy_to_user(buf, temp, u4CopySize))
		return -EFAULT;

	*f_pos += u4CopySize;
	return (ssize_t) u4CopySize;
}

static ssize_t procSarCfgDebugWrite(struct file *file,
	const char __user *buffer, size_t count, loff_t *data)
{
	uint32_t u4CopySize = sizeof(g_aucProcBuf);

	kalMemSet(g_aucProcBuf, 0, u4CopySize);
	u4CopySize = (count < u4CopySize) ? count : (u4CopySize - 1);

	if (copy_from_user(g_aucProcBuf, buffer, u4CopySize))
		return -EFAULT;
	g_aucProcBuf[u4CopySize] = '\0';

	debug_write_txPwrCtrlStringToStruct(g_aucProcBuf);

	return count;
}

static DEFINE_PROC_OPS_STRUCT(sardebug_ops) = {
	DEFINE_PROC_OPS_OWNER(THIS_MODULE)
	DEFINE_PROC_OPS_READ(procSarCfgDebugRead)
	DEFINE_PROC_OPS_WRITE(procSarCfgDebugWrite)
};
#endif

int32_t procInitFs(void)
{
	struct proc_dir_entry *prEntry;

	g_i4NextDriverReadLen = 0;

	if (init_net.proc_net == (struct proc_dir_entry *)NULL) {
		DBGLOG(INIT, ERROR, "init proc fs fail: proc_net == NULL\n");
		return -ENOENT;
	}

	/*
	 * Directory: Root (/proc/net/wlan0)
	 */

	gprProcRoot = proc_mkdir(PROC_ROOT_NAME, init_net.proc_net);
	if (!gprProcRoot) {
		DBGLOG(INIT, ERROR, "gprProcRoot == NULL\n");
		return -ENOENT;
	}
	proc_set_user(gprProcRoot, KUIDT_INIT(PROC_UID_SHELL),
		      KGIDT_INIT(PROC_GID_WIFI));

	prEntry =
	    proc_create(PROC_DBG_LEVEL_NAME, 0664, gprProcRoot, &dbglevel_ops);
	if (prEntry == NULL) {
		DBGLOG(INIT, ERROR, "Unable to create /proc entry dbgLevel\n\r");
		return -1;
	}
	proc_set_user(prEntry, KUIDT_INIT(PROC_UID_SHELL),
		      KGIDT_INIT(PROC_GID_WIFI));

#if (CFG_SUPPORT_PERMON == 1)
	prEntry =
	    proc_create(PROC_AUTO_PERF_CFG, 0664, gprProcRoot, &auto_perf_ops);
	if (prEntry == NULL) {
		DBGLOG(INIT, ERROR, "Unable to create /proc entry %s/n",
		       PROC_AUTO_PERF_CFG);
		return -1;
	}
	proc_set_user(prEntry, KUIDT_INIT(PROC_UID_SHELL),
		      KGIDT_INIT(PROC_GID_WIFI));
#endif

#if CFG_SUPPORT_DYNAMIC_PWR_LIMIT
	prEntry =
	    proc_create(PROC_SAR_CFG_DEBUG, 0664, gprProcRoot, &sardebug_ops);
	if (prEntry == NULL) {
		DBGLOG(INIT, ERROR, "Unable to create /proc entry %s/n",
		       PROC_SAR_CFG_DEBUG);
		return -1;
	}
	proc_set_user(prEntry, KUIDT_INIT(PROC_UID_SHELL),
		      KGIDT_INIT(PROC_GID_WIFI));
#endif

	return 0;
}				/* end of procInitProcfs() */

int32_t procUninitProcFs(void)
{
#if CFG_SUPPORT_DYNAMIC_PWR_LIMIT
	remove_proc_subtree(PROC_SAR_CFG_DEBUG, gprProcRoot);
#endif
#if KERNEL_VERSION(3, 9, 0) <= LINUX_VERSION_CODE
#if (CFG_SUPPORT_PERMON == 1)
	remove_proc_subtree(PROC_AUTO_PERF_CFG, gprProcRoot);
#endif
	remove_proc_subtree(PROC_DBG_LEVEL_NAME, gprProcRoot);

	/*
	 * move PROC_ROOT_NAME to last since it's root directory of the others
	 * incorrect sequence would cause use-after-free error
	 */
	remove_proc_subtree(PROC_ROOT_NAME, init_net.proc_net);
#else
#if (CFG_SUPPORT_PERMON == 1)
	remove_proc_entry(PROC_AUTO_PERF_CFG, gprProcRoot);
#endif
	remove_proc_entry(PROC_DBG_LEVEL_NAME, gprProcRoot);

	/*
	 * move PROC_ROOT_NAME to last since it's root directory of the others
	 * incorrect sequence would cause use-after-free error
	 */
	remove_proc_entry(PROC_ROOT_NAME, init_net.proc_net);
#endif

	return 0;
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief This function clean up a PROC fs created by procInitProcfs().
 *
 * \param[in] prDev      Pointer to the struct net_device.
 * \param[in] pucDevName Pointer to the name of net_device.
 *
 * \return N/A
 */
/*----------------------------------------------------------------------------*/
int32_t procRemoveProcfs(void)
{
#if CFG_ASSERT_DUMP
	remove_proc_entry(PROC_CORE_DUMP, gprProcRoot);
#endif
	remove_proc_entry(PROC_MCR_ACCESS, gprProcRoot);
	remove_proc_entry(PROC_DRIVER_CMD, gprProcRoot);
	remove_proc_entry(PROC_CFG, gprProcRoot);
	remove_proc_entry(PROC_EFUSE_DUMP, gprProcRoot);
#if CFG_WIFI_TXPWR_TBL_DUMP
	remove_proc_entry(PROC_GET_TXPWR_TBL, gprProcRoot);
#endif
	remove_proc_entry(PROC_PKT_DELAY_DBG, gprProcRoot);
#if CFG_SUPPORT_SET_CAM_BY_PROC
	remove_proc_entry(PROC_SET_CAM, gprProcRoot);
#endif
#if CFG_SUPPORT_DEBUG_FS
	remove_proc_entry(PROC_ROAM_PARAM, gprProcRoot);
#endif
#if CFG_SUPPORT_CSI
	remove_proc_entry(PROC_CSI_DATA_NAME, gprProcRoot);
#endif
#if CFG_SUPPORT_PROC_GET_WAKEUP_REASON
	remove_proc_entry(PROC_WAKEUP_REASON, gprProcRoot);
#endif
	remove_proc_entry(PROC_COUNTRY, gprProcRoot);
	g_prGlueInfo_proc = NULL;
	return 0;
} /* end of procRemoveProcfs() */

int32_t procCreateFsEntry(struct GLUE_INFO *prGlueInfo)
{
	struct proc_dir_entry *prEntry;

	DBGLOG(INIT, INFO, "[%s]\n", __func__);
	g_prGlueInfo_proc = prGlueInfo;

	prEntry = proc_create(PROC_MCR_ACCESS, 0664, gprProcRoot, &mcr_ops);
	if (prEntry == NULL) {
		DBGLOG(INIT, ERROR, "Unable to create /proc entry mcr\n");
		return -1;
	}

	prEntry =
	    proc_create(PROC_PKT_DELAY_DBG, 0664, gprProcRoot,
			&proc_pkt_delay_dbg_ops);
	if (prEntry == NULL) {
		DBGLOG(INIT, ERROR,
		       "Unable to create /proc entry pktDelay\n");
		return -1;
	}
	proc_set_user(prEntry, KUIDT_INIT(PROC_UID_SHELL),
		      KGIDT_INIT(PROC_GID_WIFI));

#if CFG_SUPPORT_SET_CAM_BY_PROC
	prEntry =
	    proc_create(PROC_SET_CAM, 0664, gprProcRoot, &proc_set_cam_ops);
	if (prEntry == NULL) {
		DBGLOG(INIT, ERROR, "Unable to create /proc entry SetCAM\n");
		return -1;
	}
	proc_set_user(prEntry, KUIDT_INIT(PROC_UID_SHELL),
		      KGIDT_INIT(PROC_GID_WIFI));
#endif
#if CFG_SUPPORT_DEBUG_FS
	prEntry = proc_create(PROC_ROAM_PARAM, 0664, gprProcRoot, &roam_ops);
	if (prEntry == NULL) {
		DBGLOG(INIT, ERROR,
		       "Unable to create /proc entry roam_param\n");
		return -1;
	}
#endif
	prEntry = proc_create(PROC_COUNTRY, 0664, gprProcRoot, &country_ops);
	if (prEntry == NULL) {
		DBGLOG(INIT, ERROR, "Unable to create /proc entry country\n");
		return -1;
	}

#if	CFG_SUPPORT_EASY_DEBUG

	prEntry =
		proc_create(PROC_DRIVER_CMD, 0664, gprProcRoot, &drivercmd_ops);
	if (prEntry == NULL) {
		DBGLOG(INIT, ERROR, "Unable to create /proc entry for driver command\n");
		return -1;
	}
#if CFG_SUPPORT_CFG_FILE
	prEntry = proc_create(PROC_CFG, 0664, gprProcRoot, &cfg_ops);
	if (prEntry == NULL) {
		DBGLOG(INIT, ERROR, "Unable to create /proc entry for driver cfg\n");
		return -1;
	}
#endif
	prEntry =
		proc_create(PROC_EFUSE_DUMP, 0664, gprProcRoot, &efusedump_ops);
	if (prEntry == NULL) {
		DBGLOG(INIT, ERROR, "Unable to create /proc entry efuse\n");
		return -1;
	}
#endif
#if CFG_WIFI_TXPWR_TBL_DUMP
	prEntry = proc_create(PROC_GET_TXPWR_TBL, 0664, gprProcRoot,
			      &get_txpwr_tbl_ops);
	if (prEntry == NULL) {
		DBGLOG(INIT, ERROR,
			"Unable to create /proc entry TXPWR Table\n");
		return -1;
	}
#endif

#if CFG_ASSERT_DUMP
	prEntry =
		proc_create(PROC_CORE_DUMP, 0664, gprProcRoot, &coredump_ops);
	if (prEntry == NULL) {
		DBGLOG(INIT, ERROR, "Unable to create /proc entry core_dump\n");
		return -1;
	}
#endif

#if CFG_SUPPORT_CSI
	prEntry =
		proc_create(PROC_CSI_DATA_NAME, 0664,
					gprProcRoot, &csidata_ops);
	if (prEntry == NULL) {
		DBGLOG(INIT, ERROR,
			"[CSI] Unable to create /proc entry csidata\n");
		return -1;
	}
#endif

#if CFG_SUPPORT_PROC_GET_WAKEUP_REASON
	prEntry = proc_create(PROC_WAKEUP_REASON, 0664, gprProcRoot,
			&wakeup_reason_ops);
	if (prEntry == NULL) {
		DBGLOG(INIT, ERROR,
			"Unable to create /proc entry wakeup_reason\n");
		return -1;
	}
#endif

	return 0;
}

#if 0
/*----------------------------------------------------------------------------*/
/*!
 * \brief The PROC function for reading Driver Status to User Space.
 *
 * \param[in] page       Buffer provided by kernel.
 * \param[in out] start  Start Address to read(3 methods).
 * \param[in] off        Offset.
 * \param[in] count      Allowable number to read.
 * \param[out] eof       End of File indication.
 * \param[in] data       Pointer to the private data structure.
 *
 * \return number of characters print to the buffer from User Space.
 */
/*----------------------------------------------------------------------------*/
static int procDrvStatusRead(char *page, char **start, off_t off, int count,
	int *eof, void *data)
{
	struct GLUE_INFO *prGlueInfo = ((struct net_device *)data)->priv;
	char *p = page;
	uint32_t u4Count;

	GLUE_SPIN_LOCK_DECLARATION();

	ASSERT(data);

	/* Kevin: Apply PROC read method 1. */
	if (off != 0)
		return 0;	/* To indicate end of file. */

	SNPRINTF(p, page, ("GLUE LAYER STATUS:"));
	SNPRINTF(p, page, ("\n=================="));

	SNPRINTF(p, page,
		("\n* Number of Pending Frames: %ld\n",
		prGlueInfo->u4TxPendingFrameNum));

	GLUE_ACQUIRE_SPIN_LOCK(prGlueInfo, SPIN_LOCK_FSM);

	wlanoidQueryDrvStatusForLinuxProc(prGlueInfo->prAdapter, p, &u4Count);

	GLUE_RELEASE_SPIN_LOCK(prGlueInfo, SPIN_LOCK_FSM);

	u4Count += (uint32_t) (p - page);

	*eof = 1;

	return (int)u4Count;

} /* end of procDrvStatusRead() */

/*----------------------------------------------------------------------------*/
/*!
 * \brief The PROC function for reading Driver RX Statistic Counters
 *        to User Space.
 *
 * \param[in] page       Buffer provided by kernel.
 * \param[in out] start  Start Address to read(3 methods).
 * \param[in] off        Offset.
 * \param[in] count      Allowable number to read.
 * \param[out] eof       End of File indication.
 * \param[in] data       Pointer to the private data structure.
 *
 * \return number of characters print to the buffer from User Space.
 */
/*----------------------------------------------------------------------------*/
static int procRxStatisticsRead(char *page, char **start, off_t off, int count,
	int *eof, void *data)
{
	struct GLUE_INFO *prGlueInfo = ((struct net_device *)data)->priv;
	char *p = page;
	uint32_t u4Count;

	GLUE_SPIN_LOCK_DECLARATION();

	ASSERT(data);

	/* Kevin: Apply PROC read method 1. */
	if (off != 0)
		return 0;	/* To indicate end of file. */

	SNPRINTF(p, page, ("RX STATISTICS (Write 1 to clear):"));
	SNPRINTF(p, page, ("\n=================================\n"));

	GLUE_ACQUIRE_SPIN_LOCK(prGlueInfo, SPIN_LOCK_FSM);

	wlanoidQueryRxStatisticsForLinuxProc(prGlueInfo->prAdapter, p,
		&u4Count);

	GLUE_RELEASE_SPIN_LOCK(prGlueInfo, SPIN_LOCK_FSM);

	u4Count += (uint32_t) (p - page);

	*eof = 1;

	return (int)u4Count;

} /* end of procRxStatisticsRead() */

/*----------------------------------------------------------------------------*/
/*!
 * \brief The PROC function for reset Driver RX Statistic Counters.
 *
 * \param[in] file   pointer to file.
 * \param[in] buffer Buffer from user space.
 * \param[in] count  Number of characters to write
 * \param[in] data   Pointer to the private data structure.
 *
 * \return number of characters write from User Space.
 */
/*----------------------------------------------------------------------------*/
static int procRxStatisticsWrite(struct file *file, const char *buffer,
	unsigned long count, void *data)
{
	struct GLUE_INFO *prGlueInfo = ((struct net_device *)data)->priv;
	/* + 1 for "\0" */
	char acBuf[PROC_RX_STATISTICS_MAX_USER_INPUT_LEN + 1];
	uint32_t u4CopySize;
	uint32_t u4ClearCounter;
	int32_t rv;

	GLUE_SPIN_LOCK_DECLARATION();

	ASSERT(data);

	u4CopySize =
		(count < (sizeof(acBuf) - 1)) ? count : (sizeof(acBuf) - 1);
	copy_from_user(acBuf, buffer, u4CopySize);
	acBuf[u4CopySize] = '\0';

	rv = kstrtoint(acBuf, 0, &u4ClearCounter);
	if (rv == 1) {
		if (u4ClearCounter == 1) {
			GLUE_ACQUIRE_SPIN_LOCK(prGlueInfo, SPIN_LOCK_FSM);

			wlanoidSetRxStatisticsForLinuxProc(prGlueInfo->
				prAdapter);

			GLUE_RELEASE_SPIN_LOCK(prGlueInfo, SPIN_LOCK_FSM);
		}
	}

	return count;

} /* end of procRxStatisticsWrite() */

/*----------------------------------------------------------------------------*/
/*!
 * \brief The PROC function for reading Driver TX Statistic Counters
 *        to User Space.
 *
 * \param[in] page       Buffer provided by kernel.
 * \param[in out] start  Start Address to read(3 methods).
 * \param[in] off        Offset.
 * \param[in] count      Allowable number to read.
 * \param[out] eof       End of File indication.
 * \param[in] data       Pointer to the private data structure.
 *
 * \return number of characters print to the buffer from User Space.
 */
/*----------------------------------------------------------------------------*/
static int procTxStatisticsRead(char *page, char **start, off_t off, int count,
	int *eof, void *data)
{
	struct GLUE_INFO *prGlueInfo = ((struct net_device *)data)->priv;
	char *p = page;
	uint32_t u4Count;

	GLUE_SPIN_LOCK_DECLARATION();

	ASSERT(data);

	/* Kevin: Apply PROC read method 1. */
	if (off != 0)
		return 0;	/* To indicate end of file. */

	SNPRINTF(p, page, ("TX STATISTICS (Write 1 to clear):"));
	SNPRINTF(p, page, ("\n=================================\n"));

	GLUE_ACQUIRE_SPIN_LOCK(prGlueInfo, SPIN_LOCK_FSM);

	wlanoidQueryTxStatisticsForLinuxProc(prGlueInfo->prAdapter, p,
		&u4Count);

	GLUE_RELEASE_SPIN_LOCK(prGlueInfo, SPIN_LOCK_FSM);

	u4Count += (uint32_t) (p - page);

	*eof = 1;

	return (int)u4Count;

} /* end of procTxStatisticsRead() */

/*----------------------------------------------------------------------------*/
/*!
 * \brief The PROC function for reset Driver TX Statistic Counters.
 *
 * \param[in] file   pointer to file.
 * \param[in] buffer Buffer from user space.
 * \param[in] count  Number of characters to write
 * \param[in] data   Pointer to the private data structure.
 *
 * \return number of characters write from User Space.
 */
/*----------------------------------------------------------------------------*/
static int procTxStatisticsWrite(struct file *file, const char *buffer,
	unsigned long count, void *data)
{
	struct GLUE_INFO *prGlueInfo = ((struct net_device *)data)->priv;
	/* + 1 for "\0" */
	char acBuf[PROC_RX_STATISTICS_MAX_USER_INPUT_LEN + 1];
	uint32_t u4CopySize;
	uint32_t u4ClearCounter;
	int32_t rv;

	GLUE_SPIN_LOCK_DECLARATION();

	ASSERT(data);

	u4CopySize =
		(count < (sizeof(acBuf) - 1)) ? count : (sizeof(acBuf) - 1);
	copy_from_user(acBuf, buffer, u4CopySize);
	acBuf[u4CopySize] = '\0';

	rv = kstrtoint(acBuf, 0, &u4ClearCounter);
	if (rv == 1) {
		if (u4ClearCounter == 1) {
			GLUE_ACQUIRE_SPIN_LOCK(prGlueInfo, SPIN_LOCK_FSM);

			wlanoidSetTxStatisticsForLinuxProc(prGlueInfo->
				prAdapter);

			GLUE_RELEASE_SPIN_LOCK(prGlueInfo, SPIN_LOCK_FSM);
		}
	}

	return count;

} /* end of procTxStatisticsWrite() */
#endif

#ifdef FW_CFG_SUPPORT
#define MAX_CFG_OUTPUT_BUF_LENGTH 1024
static uint8_t aucCfgBuf[CMD_FORMAT_V1_LENGTH];
static uint8_t aucCfgQueryKey[MAX_CMD_NAME_MAX_LENGTH];
static uint8_t aucCfgOutputBuf[MAX_CFG_OUTPUT_BUF_LENGTH];

static ssize_t cfgRead(struct file *filp, char __user *buf, size_t count,
	loff_t *f_pos)
{
	uint32_t rStatus = WLAN_STATUS_FAILURE;
	uint8_t *temp = &aucCfgOutputBuf[0];
	uint32_t u4CopySize = 0;

	struct CMD_HEADER cmdV1Header;
	struct CMD_FORMAT_V1 *pr_cmd_v1 =
		(struct CMD_FORMAT_V1 *)cmdV1Header.buffer;

	/* if *f_pos >  0, we should return 0 to make cat command exit */
	if (*f_pos > 0 || gprGlueInfo == NULL)
		return 0;
	if (!kalStrLen(aucCfgQueryKey))
		return 0;

	kalMemSet(aucCfgOutputBuf, 0, MAX_CFG_OUTPUT_BUF_LENGTH);

	SNPRINTF(temp, aucCfgOutputBuf,
		("\nprocCfgRead() %s:\n", aucCfgQueryKey));

	/* send to FW */
	cmdV1Header.cmdVersion = CMD_VER_1;
	cmdV1Header.cmdType = CMD_TYPE_QUERY;
	cmdV1Header.itemNum = 1;
	cmdV1Header.cmdBufferLen = sizeof(struct CMD_FORMAT_V1);
	kalMemSet(cmdV1Header.buffer, 0, MAX_CMD_BUFFER_LENGTH);

	pr_cmd_v1->itemStringLength = kalStrLen(aucCfgQueryKey);

	kalMemCopy(pr_cmd_v1->itemString, aucCfgQueryKey,
		kalStrLen(aucCfgQueryKey));

	rStatus = kalIoctl(gprGlueInfo,
		wlanoidQueryCfgRead,
		(void *)&cmdV1Header,
		sizeof(cmdV1Header), TRUE, TRUE, TRUE, &u4CopySize);
	if (rStatus == WLAN_STATUS_FAILURE)
		DBGLOG(INIT, ERROR,
			"kalIoctl wlanoidQueryCfgRead fail 0x%x\n",
			rStatus);

	SNPRINTF(temp, aucCfgOutputBuf,
		("%s\n", cmdV1Header.buffer));

	u4CopySize = kalStrLen(aucCfgOutputBuf);
	if (u4CopySize > count)
		u4CopySize = count;

	if (copy_to_user(buf, aucCfgOutputBuf, u4CopySize))
		DBGLOG(INIT, ERROR, "copy to user failed\n");

	*f_pos += u4CopySize;
	return (ssize_t) u4CopySize;
}

static ssize_t cfgWrite(struct file *filp, const char __user *buf,
	size_t count, loff_t *f_pos)
{
	/* echo xxx xxx > /proc/net/wlan/cfg */
	uint8_t i = 0;
	uint32_t u4CopySize = sizeof(aucCfgBuf);
	uint8_t token_num = 1;

	if (count <= 0) {
		DBGLOG(INIT, ERROR, "wrong copy size\n");
		return -EFAULT;
	}

	kalMemSet(aucCfgBuf, 0, u4CopySize);
	u4CopySize = (count < u4CopySize) ? count : (u4CopySize - 1);

	if (copy_from_user(aucCfgBuf, buf, u4CopySize)) {
		DBGLOG(INIT, ERROR, "copy from user failed\n");
		return -EFAULT;
	}
	aucCfgBuf[u4CopySize] = '\0';
	for (i = 0; i < u4CopySize; i++) {
		if (aucCfgBuf[i] == ' ') {
			token_num++;
			break;
		}
	}

	if (token_num == 1) {
		kalMemSet(aucCfgQueryKey, 0, sizeof(aucCfgQueryKey));
		u4CopySize = (u4CopySize < sizeof(aucCfgQueryKey)) ?
			u4CopySize : sizeof(aucCfgQueryKey);

		/* remove the 0x0a */
		memcpy(aucCfgQueryKey, aucCfgBuf, u4CopySize);
		if (aucCfgQueryKey[u4CopySize - 1] == 0x0a)
			aucCfgQueryKey[u4CopySize - 1] = '\0';
	} else {
		wlanFwCfgParse(gprGlueInfo->prAdapter, aucCfgBuf);
	}

	return count;
}

static const struct file_operations fwcfg_ops = {
	.owner = THIS_MODULE,
	.read = cfgRead,
	.write = cfgWrite,
};

int32_t cfgRemoveProcEntry(void)
{
	remove_proc_entry(PROC_CFG_NAME, gprProcRoot);
	return 0;
}

int32_t cfgCreateProcEntry(struct GLUE_INFO *prGlueInfo)
{
	struct proc_dir_entry *prEntry;

	prGlueInfo->pProcRoot = gprProcRoot;
	gprGlueInfo = prGlueInfo;

	prEntry = proc_create(PROC_CFG_NAME, 0664, gprProcRoot, &fwcfg_ops);
	if (prEntry == NULL) {
		DBGLOG(INIT, ERROR, "Unable to create /proc entry cfg\n");
		return -1;
	}
	proc_set_user(prEntry, KUIDT_INIT(PROC_UID_SHELL),
		KGIDT_INIT(PROC_GID_WIFI));

	return 0;
}
#endif
