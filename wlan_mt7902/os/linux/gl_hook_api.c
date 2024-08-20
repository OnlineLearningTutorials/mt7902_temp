/* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause */
/*
 * Copyright (c) 2016 MediaTek Inc.
 */
/*
	Module Name:
	gl_hook_api.c
*/
/*******************************************************************************
 *						C O M P I L E R	 F L A G S
 *******************************************************************************
 */

/*******************************************************************************
 *					E X T E R N A L	R E F E R E N C E S
 *******************************************************************************
 */
#include "precomp.h"
#if CFG_SUPPORT_QA_TOOL
#include "gl_wext.h"
#include "gl_cfg80211.h"
#include "gl_ate_agent.h"
#include "gl_qa_agent.h"
#if KERNEL_VERSION(3, 8, 0) <= CFG80211_VERSION_CODE
#include <uapi/linux/nl80211.h>
#endif

/*******************************************************************************
 *						C O N S T A N T S
 *******************************************************************************
 */
enum {
	ATE_LOG_RXV = 1,
	ATE_LOG_RDD,
	ATE_LOG_RE_CAL,
	ATE_LOG_TYPE_NUM,
	ATE_LOG_RXINFO,
	ATE_LOG_TXDUMP,
	ATE_LOG_TEST,
};

enum {
	ATE_LOG_OFF,
	ATE_LOG_ON,
	ATE_LOG_DUMP,
	ATE_LOG_CTRL_NUM,
};

/* Maximum rxv vectors under 2048-2 bytes */
#define MAX_RXV_DUMP_COUNT			(56)
/*******************************************************************************
 *				F U N C T I O N   D E C L A R A T I O N S
 *******************************************************************************
 */

/*----------------------------------------------------------------------------*/
/*!
 * \brief  Hook API for Enter Test Mode.
 *
 * \param[in] prNetDev		Pointer to the Net Device
 * \param[in] prInBuf
 * \param[out] None
 *
 * \retval 0				On success.
 * \retval -EFAULT			If kalIoctl return nonzero.
 */
/*----------------------------------------------------------------------------*/
int32_t MT_ATEStart(struct net_device *prNetDev,
		    uint8_t *prInBuf)
{
	uint32_t u4BufLen = 0;
	int32_t i4Status;
	struct GLUE_INFO *prGlueInfo = NULL;

	DBGLOG(RFTEST, INFO, "QA_ATE_HOOK SetATE = %s\n", prInBuf);

	prGlueInfo = *((struct GLUE_INFO **) netdev_priv(prNetDev));

	i4Status = kalIoctl(prGlueInfo,	/* prGlueInfo */
			    wlanoidRftestSetTestMode,	/* pfnOidHandler */
			    NULL,	/* pvInfoBuf */
			    0,	/* u4InfoBufLen */
			    FALSE,	/* fgRead */
			    FALSE,	/* fgWaitResp */
			    TRUE,	/* fgCmd */
			    &u4BufLen);	/* pu4QryInfoLen */

	if (i4Status != WLAN_STATUS_SUCCESS)
		return -EFAULT;

	return i4Status;
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief  Hook API for Enter ICAP Mode.
 *
 * \param[in] prNetDev		Pointer to the Net Device
 * \param[in] prInBuf
 * \param[out] None
 *
 * \retval 0				On success.
 * \retval -EFAULT			If kalIoctl return nonzero.
 */
/*----------------------------------------------------------------------------*/
int32_t MT_ICAPStart(struct net_device *prNetDev,
		     uint8_t *prInBuf)
{
	uint32_t u4BufLen = 0;
	int32_t i4Status;
	struct GLUE_INFO *prGlueInfo = NULL;

	DBGLOG(RFTEST, INFO, "QA_ATE_HOOK SetATE = %s\n", prInBuf);

	prGlueInfo = *((struct GLUE_INFO **) netdev_priv(prNetDev));

	i4Status = kalIoctl(prGlueInfo,	/* prGlueInfo */
		    wlanoidRftestSetTestIcapMode, /* pfnOidHandler */
		    NULL,	/* pvInfoBuf */
		    0,	/* u4InfoBufLen */
		    FALSE,	/* fgRead */
		    FALSE,	/* fgWaitResp */
		    TRUE,	/* fgCmd */
		    &u4BufLen);	/* pu4QryInfoLen */

	if (i4Status != WLAN_STATUS_SUCCESS)
		return -EFAULT;

	return i4Status;
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief  Hook API for Enter ICAP Mode.
 *
 * \param[in] prNetDev		Pointer to the Net Device
 * \param[in] prInBuf
 * \param[out] None
 *
 * \retval 0				On success.
 * \retval -EFAULT			If kalIoctl return nonzero.
 */
/*----------------------------------------------------------------------------*/
int32_t MT_ICAPCommand(struct net_device *prNetDev,
		       uint8_t *prInBuf)
{
	int32_t i4Status = 0;
	struct GLUE_INFO *prGlueInfo = NULL;
	struct mt66xx_chip_info *prChipInfo = NULL;
	struct ATE_OPS_T *prAteOps = NULL;
	uint32_t *buf = NULL;

	DBGLOG(RFTEST, INFO, "QA_ATE_HOOK SetATE = %s\n", prInBuf);

	prGlueInfo = *((struct GLUE_INFO **) netdev_priv(prNetDev));
	prChipInfo = prGlueInfo->prAdapter->chip_info;
	ASSERT(prChipInfo);
	prAteOps = prChipInfo->prAteOps;
	ASSERT(prAteOps);

	if (prInBuf[0] == '0') {
		if (prAteOps->setICapStart)
			i4Status = prAteOps->setICapStart(prGlueInfo,
							  0, /*stop icap*/
							  0,
							  0,
							  0x14000000,
							  0,
							  10,
							  0,
							  0x2000,
							  0xefefefef,
							  0x0001efef,
							  0);
		else
			i4Status = 1;
	} else if (prInBuf[0] == '1') {
		if (prAteOps->setICapStart)
			i4Status = prAteOps->setICapStart(prGlueInfo,
							  1, /*enable icap*/
							  0,
							  0,
							  0x14000000,
							  0,
							  10,
							  0,
							  0x2000,
							  0xefefefef,
							  0x0001efef,
							  0);
		else
			i4Status = 1;
	} else if (prInBuf[0] == '2') {
		if (prAteOps->getICapStatus)
			i4Status = prAteOps->getICapStatus(prGlueInfo);
		else
			i4Status = 1;
	} else if (prInBuf[0] == '3') {
		if (prAteOps->getICapIQData) {
			buf = kalMemAlloc(1024, VIR_MEM_TYPE);
			if (buf) {
				i4Status = prAteOps->getICapIQData(prGlueInfo,
						(uint8_t *) buf, CAP_I_TYPE, 0);
				dumpMemory32((uint32_t *)buf, i4Status);
				kalMemFree(buf, VIR_MEM_TYPE, 1024);
			}
		} else
			i4Status = 1;
	} else if (prInBuf[0] == '4') {
		if (prAteOps->getICapIQData) {
			buf = (uint32_t *) kalMemAlloc(1024, VIR_MEM_TYPE);
			if (buf) {
				i4Status = prAteOps->getICapIQData(prGlueInfo,
						(uint8_t *) buf, CAP_Q_TYPE, 0);
				dumpMemory32((uint32_t *)buf, i4Status);
				kalMemFree(buf, VIR_MEM_TYPE, 1024);
			}
		} else
			i4Status = 1;
	} else if (prInBuf[0] == '5') {
		if (prAteOps->getICapIQData) {
			buf = (uint32_t *) kalMemAlloc(1024, VIR_MEM_TYPE);
			if (buf) {
				i4Status = prAteOps->getICapIQData(prGlueInfo,
						(uint8_t *) buf, CAP_I_TYPE, 1);
				dumpMemory32((uint32_t *)buf, i4Status);
				kalMemFree(buf, VIR_MEM_TYPE, 1024);
			}
		} else
			i4Status = 1;
	} else if (prInBuf[0] == '6') {
		if (prAteOps->getICapIQData) {
			buf = (uint32_t *) kalMemAlloc(1024, VIR_MEM_TYPE);
			if (buf) {
				i4Status = prAteOps->getICapIQData(prGlueInfo,
						(uint8_t *) buf, CAP_Q_TYPE, 1);
				dumpMemory32((uint32_t *)buf, i4Status);
				kalMemFree(buf, VIR_MEM_TYPE, 1024);
			}
		} else
			i4Status = 1;
	}

	return i4Status;
}


/*----------------------------------------------------------------------------*/
/*!
 * \brief  Hook API for Abort Test Mode.
 *
 * \param[in] prNetDev		Pointer to the Net Device
 * \param[in] prInBuf
 * \param[out] None
 *
 * \retval 0				On success.
 * \retval -EFAULT			If kalIoctl return nonzero.
 */
/*----------------------------------------------------------------------------*/
int32_t MT_ATEStop(struct net_device *prNetDev,
		   uint8_t *prInBuf)
{
	uint32_t u4BufLen = 0;
	int32_t i4Status;
	struct GLUE_INFO *prGlueInfo = NULL;

	DBGLOG(RFTEST, INFO, "QA_ATE_HOOK SetATE = %s\n", prInBuf);

	prGlueInfo = *((struct GLUE_INFO **) netdev_priv(prNetDev));

	i4Status = kalIoctl(prGlueInfo,	/* prGlueInfo */
		    wlanoidRftestSetAbortTestMode, /* pfnOidHandler */
		    NULL,	/* pvInfoBuf */
		    0,	/* u4InfoBufLen */
		    FALSE,	/* fgRead */
		    FALSE,	/* fgWaitResp */
		    TRUE,	/* fgCmd */
		    &u4BufLen);	/* pu4QryInfoLen */

	if (i4Status != WLAN_STATUS_SUCCESS)
		return -EFAULT;

	return i4Status;
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief  Hook API for Start auto Tx test in packet format and the driver will
 *	   enter auto Tx test mode.
 *
 * \param[in] prNetDev		Pointer to the Net Device
 * \param[in] prInBuf
 * \param[out] None
 *
 * \retval 0				On success.
 * \retval -EFAULT			If kalIoctl return nonzero.
 */
/*----------------------------------------------------------------------------*/
int32_t MT_ATEStartTX(struct net_device *prNetDev,
		      uint8_t *prInBuf)
{
	uint32_t u4BufLen = 0;
	int32_t i4Status;
	struct GLUE_INFO *prGlueInfo = NULL;
	struct PARAM_MTK_WIFI_TEST_STRUCT rRfATInfo;

	DBGLOG(RFTEST, INFO, "QA_ATE_HOOK SetATE = %s\n", prInBuf);

	prGlueInfo = *((struct GLUE_INFO **) netdev_priv(prNetDev));

	rRfATInfo.u4FuncIndex = RF_AT_FUNCID_COMMAND;
	rRfATInfo.u4FuncData = RF_AT_COMMAND_STARTTX;

	i4Status = kalIoctl(prGlueInfo,	/* prGlueInfo */
			    wlanoidRftestSetAutoTest,	/* pfnOidHandler */
			    &rRfATInfo,	/* pvInfoBuf */
			    sizeof(rRfATInfo),	/* u4InfoBufLen */
			    FALSE,	/* fgRead */
			    FALSE,	/* fgWaitResp */
			    TRUE,	/* fgCmd */
			    &u4BufLen);	/* pu4QryInfoLen */

	if (i4Status != WLAN_STATUS_SUCCESS)
		return -EFAULT;

	return i4Status;
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief  Hook API for Stop TX/RX test action if the driver is in any test
 *	   mode.
 *
 * \param[in] prNetDev		Pointer to the Net Device
 * \param[in] prInBuf
 * \param[out] None
 *
 * \retval 0				On success.
 * \retval -EFAULT			If kalIoctl return nonzero.
 */
/*----------------------------------------------------------------------------*/
int32_t MT_ATEStopTX(struct net_device *prNetDev,
		     uint8_t *prInBuf)
{
	uint32_t u4BufLen = 0;
	int32_t i4Status;
	struct GLUE_INFO *prGlueInfo = NULL;
	struct PARAM_MTK_WIFI_TEST_STRUCT rRfATInfo;

	DBGLOG(RFTEST, INFO, "QA_ATE_HOOK SetATE = %s\n", prInBuf);

	prGlueInfo = *((struct GLUE_INFO **) netdev_priv(prNetDev));

	rRfATInfo.u4FuncIndex = RF_AT_FUNCID_COMMAND;
	rRfATInfo.u4FuncData = RF_AT_COMMAND_STOPTEST;

	i4Status = kalIoctl(prGlueInfo,	/* prGlueInfo */
			    wlanoidRftestSetAutoTest,	/* pfnOidHandler */
			    &rRfATInfo,	/* pvInfoBuf */
			    sizeof(rRfATInfo),	/* u4InfoBufLen */
			    FALSE,	/* fgRead */
			    FALSE,	/* fgWaitResp */
			    TRUE,	/* fgCmd */
			    &u4BufLen);	/* pu4QryInfoLen */

	if (i4Status != WLAN_STATUS_SUCCESS)
		return -EFAULT;

	return i4Status;
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief  Hook API for Start auto Rx test and the driver will enter auto Rx
 *	   test mode.
 *
 * \param[in] prNetDev		Pointer to the Net Device
 * \param[in] prInBuf
 * \param[out] None
 *
 * \retval 0				On success.
 * \retval -EFAULT			If kalIoctl return nonzero.
 */
/*----------------------------------------------------------------------------*/
int32_t MT_ATEStartRX(struct net_device *prNetDev,
		      uint8_t *prInBuf)
{
	uint32_t u4BufLen = 0;
	int32_t i4Status;
	struct GLUE_INFO *prGlueInfo = NULL;
	struct PARAM_MTK_WIFI_TEST_STRUCT rRfATInfo;

	prGlueInfo = *((struct GLUE_INFO **) netdev_priv(prNetDev));

	DBGLOG(RFTEST, INFO, "QA_ATE_HOOK SetATE = %s\n", prInBuf);

	rRfATInfo.u4FuncIndex = RF_AT_FUNCID_COMMAND;
	rRfATInfo.u4FuncData = RF_AT_COMMAND_STARTRX;

	i4Status = kalIoctl(prGlueInfo,	/* prGlueInfo */
			    wlanoidRftestSetAutoTest,	/* pfnOidHandler */
			    &rRfATInfo,	/* pvInfoBuf */
			    sizeof(rRfATInfo),	/* u4InfoBufLen */
			    FALSE,	/* fgRead */
			    FALSE,	/* fgWaitResp */
			    TRUE,	/* fgCmd */
			    &u4BufLen);	/* pu4QryInfoLen */

	if (i4Status != WLAN_STATUS_SUCCESS)
		return -EFAULT;

	return i4Status;
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief  Hook API for Stop TX/RX test action if the driver is in any test
 *	   mode.
 *
 * \param[in] prNetDev		Pointer to the Net Device
 * \param[in] prInBuf
 * \param[out] None
 *
 * \retval 0				On success.
 * \retval -EFAULT			If kalIoctl return nonzero.
 */
/*----------------------------------------------------------------------------*/
int32_t MT_ATEStopRX(struct net_device *prNetDev,
		     uint8_t *prInBuf)
{
	uint32_t u4BufLen = 0;
	int32_t i4Status;
	struct GLUE_INFO *prGlueInfo = NULL;
	struct PARAM_MTK_WIFI_TEST_STRUCT rRfATInfo;

	prGlueInfo = *((struct GLUE_INFO **) netdev_priv(prNetDev));

	DBGLOG(RFTEST, INFO, "QA_ATE_HOOK SetATE = %s\n", prInBuf);

	rRfATInfo.u4FuncIndex = RF_AT_FUNCID_COMMAND;
	rRfATInfo.u4FuncData = RF_AT_COMMAND_STOPTEST;

	i4Status = kalIoctl(prGlueInfo,	/* prGlueInfo */
			    wlanoidRftestSetAutoTest,	/* pfnOidHandler */
			    &rRfATInfo,	/* pvInfoBuf */
			    sizeof(rRfATInfo),	/* u4InfoBufLen */
			    FALSE,	/* fgRead */
			    FALSE,	/* fgWaitResp */
			    TRUE,	/* fgCmd */
			    &u4BufLen);	/* pu4QryInfoLen */

	if (i4Status != WLAN_STATUS_SUCCESS)
		return -EFAULT;

	return i4Status;
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief  Hook API for Set Channel Frequency.
 *
 * \param[in] prNetDev		Pointer to the Net Device
 * \param[in] u4SetFreq		Center frequency in unit of KHz
 * \param[out] None
 *
 * \retval 0				On success.
 * \retval -EFAULT			If kalIoctl return nonzero.
 * \retval -EINVAL			If invalid argument.
 */
/*----------------------------------------------------------------------------*/
int32_t MT_ATESetChannel(struct net_device *prNetDev,
			 uint32_t u4SXIdx, uint32_t u4SetFreq)
{
	uint32_t u4BufLen = 0;
	uint32_t i4SetChan;
	int32_t i4Status;
	struct GLUE_INFO *prGlueInfo = NULL;
	struct PARAM_MTK_WIFI_TEST_STRUCT rRfATInfo;

	prGlueInfo = *((struct GLUE_INFO **) netdev_priv(prNetDev));

	i4SetChan = nicFreq2ChannelNum(u4SetFreq);

	DBGLOG(RFTEST, INFO, "QA_ATE_HOOK SetChannel=%d, Freq=%d\n",
	       i4SetChan, u4SetFreq);

	if (u4SetFreq == 0)
		return -EINVAL;

	if (u4SXIdx == 0) {
		rRfATInfo.u4FuncIndex = RF_AT_FUNCID_CHNL_FREQ;
		rRfATInfo.u4FuncData = u4SetFreq;
	} else {
		rRfATInfo.u4FuncIndex = RF_AT_FUNCID_CHNL_FREQ | BIT(16);
		rRfATInfo.u4FuncData = u4SetFreq;
	}

	i4Status = kalIoctl(prGlueInfo,	/* prGlueInfo */
			    wlanoidRftestSetAutoTest,	/* pfnOidHandler */
			    &rRfATInfo,	/* pvInfoBuf */
			    sizeof(rRfATInfo),	/* u4InfoBufLen */
			    FALSE,	/* fgRead */
			    FALSE,	/* fgWaitResp */
			    TRUE,	/* fgCmd */
			    &u4BufLen);	/* pu4QryInfoLen */

	if (i4Status != WLAN_STATUS_SUCCESS)
		return -EFAULT;

	return i4Status;
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief  Hook API for Set Preamble.
 *
 * \param[in] prNetDev		Pointer to the Net Device
 * \param[in] u4Mode		depends on Rate. 0--> normal,
 *						 1--> CCK short preamble,
 *						 2: 11n MM,
 *						 3: 11n GF
 *						 4: 11ac VHT
 * \param[out] None
 *
 * \retval 0				On success.
 * \retval -EFAULT			If kalIoctl return nonzero.
 * \retval -EINVAL			If invalid argument.
 */
/*----------------------------------------------------------------------------*/
int32_t MT_ATESetPreamble(struct net_device *prNetDev,
			  uint32_t u4Mode)
{
	uint32_t u4BufLen = 0;
	int32_t i4Status;
	struct GLUE_INFO *prGlueInfo = NULL;
	struct PARAM_MTK_WIFI_TEST_STRUCT rRfATInfo;

	prGlueInfo = *((struct GLUE_INFO **) netdev_priv(prNetDev));

	DBGLOG(RFTEST, INFO, "QA_ATE_HOOK SetPreamble=%d\n",
	       u4Mode);

	if (u4Mode > 4)
		return -EINVAL;

	rRfATInfo.u4FuncIndex = RF_AT_FUNCID_PREAMBLE;
	rRfATInfo.u4FuncData = u4Mode;

	i4Status = kalIoctl(prGlueInfo,	/* prGlueInfo */
			    wlanoidRftestSetAutoTest,	/* pfnOidHandler */
			    &rRfATInfo,	/* pvInfoBuf */
			    sizeof(rRfATInfo),	/* u4InfoBufLen */
			    FALSE,	/* fgRead */
			    FALSE,	/* fgWaitResp */
			    TRUE,	/* fgCmd */
			    &u4BufLen);	/* pu4QryInfoLen */

	if (i4Status != WLAN_STATUS_SUCCESS)
		return -EFAULT;

	return i4Status;
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief  Hook API for Set Channel Bandwidth.
 *
 * \param[in] prNetDev		Pointer to the Net Device
 * \param[in] u4BW		Choose Channel Bandwidth
 *				0: 20 / 1: 40 / 2: 80 / 3: 160
 * \param[out] None
 *
 * \retval 0			On success.
 * \retval -EFAULT		If kalIoctl return nonzero.
 * \retval -EINVAL		If invalid argument.
 */
/*----------------------------------------------------------------------------*/
int32_t MT_ATESetSystemBW(struct net_device *prNetDev,
			  uint32_t u4BW)
{
	uint32_t u4BufLen = 0;
	int32_t i4Status;
	struct GLUE_INFO *prGlueInfo = NULL;
	struct PARAM_MTK_WIFI_TEST_STRUCT rRfATInfo;
	uint32_t u4BWMapping = 0;

	prGlueInfo = *((struct GLUE_INFO **) netdev_priv(prNetDev));

	DBGLOG(RFTEST, INFO, "QA_ATE_HOOK SetSystemBW=%d\n", u4BW);

	if (u4BW > 6)
		return -EINVAL;

	/* BW Mapping in QA Tool
	 * 0: BW20
	 * 1: BW40
	 * 2: BW80
	 * 3: BW10
	 * 4: BW5
	 * 5: BW160C
	 * 6: BW160NC
	 */
	/* BW Mapping in FW
	 * 0: BW20
	 * 1: BW40
	 * 2: BW80
	 * 3: BW160C
	 * 4: BW160NC
	 * 5: BW5
	 * 6: BW10
	 */
	if (u4BW == 0)
		u4BWMapping = 0;
	else if (u4BW == 1)
		u4BWMapping = 1;
	else if (u4BW == 2)
		u4BWMapping = 2;
	else if (u4BW == 3)
		u4BWMapping = 6;
	else if (u4BW == 4)
		u4BWMapping = 5;
	else if (u4BW == 5)
		u4BWMapping = 3;
	else if (u4BW == 6)
		u4BWMapping = 4;

	rRfATInfo.u4FuncIndex = RF_AT_FUNCID_SET_CBW;
	rRfATInfo.u4FuncData = u4BWMapping;

	i4Status = kalIoctl(prGlueInfo,	/* prGlueInfo */
			    wlanoidRftestSetAutoTest,	/* pfnOidHandler */
			    &rRfATInfo,	/* pvInfoBuf */
			    sizeof(rRfATInfo),	/* u4InfoBufLen */
			    FALSE,	/* fgRead */
			    FALSE,	/* fgWaitResp */
			    TRUE,	/* fgCmd */
			    &u4BufLen);	/* pu4QryInfoLen */

	if (i4Status != WLAN_STATUS_SUCCESS)
		return -EFAULT;

	return i4Status;
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief  Hook API for Set TX Length.
 *
 * \param[in] prNetDev		Pointer to the Net Device
 * \param[in] u4TxLength	Packet length (MPDU)
 * \param[out] None
 *
 * \retval 0			On success.
 * \retval -EFAULT		If kalIoctl return nonzero.
 * \retval -EINVAL		If invalid argument.
 */
/*----------------------------------------------------------------------------*/
int32_t MT_ATESetTxLength(struct net_device *prNetDev,
			  uint32_t u4TxLength)
{
	uint32_t u4BufLen = 0;
	int32_t i4Status;
	struct GLUE_INFO *prGlueInfo = NULL;
	struct PARAM_MTK_WIFI_TEST_STRUCT rRfATInfo;

	prGlueInfo = *((struct GLUE_INFO **) netdev_priv(prNetDev));

	DBGLOG(RFTEST, INFO, "QA_ATE_HOOK SetTxLength=%d\n",
	       u4TxLength);

	rRfATInfo.u4FuncIndex = RF_AT_FUNCID_PKTLEN;
	rRfATInfo.u4FuncData = u4TxLength;

	i4Status = kalIoctl(prGlueInfo,	/* prGlueInfo */
			    wlanoidRftestSetAutoTest,	/* pfnOidHandler */
			    &rRfATInfo,	/* pvInfoBuf */
			    sizeof(rRfATInfo),	/* u4InfoBufLen */
			    FALSE,	/* fgRead */
			    FALSE,	/* fgWaitResp */
			    TRUE,	/* fgCmd */
			    &u4BufLen);	/* pu4QryInfoLen */

	if (i4Status != WLAN_STATUS_SUCCESS)
		return -EFAULT;

	return i4Status;
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief  Hook API for Set TX Count.
 *
 * \param[in] prNetDev		Pointer to the Net Device
 * \param[in] u4TxCount		Total packet count to send. 0 : unlimited,
 *				until stopped
 * \param[out] None
 *
 * \retval 0			On success.
 * \retval -EFAULT		If kalIoctl return nonzero.
 * \retval -EINVAL		If invalid argument.
 */
/*----------------------------------------------------------------------------*/
int32_t MT_ATESetTxCount(struct net_device *prNetDev,
			 uint32_t u4TxCount)
{
	uint32_t u4BufLen = 0;
	int32_t i4Status;
	struct GLUE_INFO *prGlueInfo = NULL;
	struct PARAM_MTK_WIFI_TEST_STRUCT rRfATInfo;

	prGlueInfo = *((struct GLUE_INFO **) netdev_priv(prNetDev));

	DBGLOG(RFTEST, INFO, "QA_ATE_HOOK SetTxCount=%d\n",
	       u4TxCount);

	rRfATInfo.u4FuncIndex = RF_AT_FUNCID_PKTCNT;
	rRfATInfo.u4FuncData = u4TxCount;

	i4Status = kalIoctl(prGlueInfo,	/* prGlueInfo */
			    wlanoidRftestSetAutoTest,	/* pfnOidHandler */
			    &rRfATInfo,	/* pvInfoBuf */
			    sizeof(rRfATInfo),	/* u4InfoBufLen */
			    FALSE,	/* fgRead */
			    FALSE,	/* fgWaitResp */
			    TRUE,	/* fgCmd */
			    &u4BufLen);	/* pu4QryInfoLen */

	if (i4Status != WLAN_STATUS_SUCCESS)
		return -EFAULT;

	return i4Status;
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief  Hook API for Set TX Inter-Packet Guard.
 *
 * \param[in] prNetDev		Pointer to the Net Device
 * \param[in] u4TxIPG		In unit of us. The min value is 19us and max
 *				value is 2314us.
 * \				It will be round-up to (19+9n) us.
 * \param[out] None
 *
 * \retval 0			On success.
 * \retval -EFAULT		If kalIoctl return nonzero.
 * \retval -EINVAL		If invalid argument.
 */
/*----------------------------------------------------------------------------*/
int32_t MT_ATESetTxIPG(struct net_device *prNetDev,
		       uint32_t u4TxIPG)
{
	uint32_t u4BufLen = 0;
	int32_t i4Status;
	struct GLUE_INFO *prGlueInfo = NULL;
	struct PARAM_MTK_WIFI_TEST_STRUCT rRfATInfo;

	prGlueInfo = *((struct GLUE_INFO **) netdev_priv(prNetDev));

	DBGLOG(RFTEST, INFO, "QA_ATE_HOOK SetTxIPG=%d\n", u4TxIPG);

	if (u4TxIPG > 2314 || u4TxIPG < 19)
		return -EINVAL;

	rRfATInfo.u4FuncIndex = RF_AT_FUNCID_PKTINTERVAL;
	rRfATInfo.u4FuncData = u4TxIPG;

	i4Status = kalIoctl(prGlueInfo,	/* prGlueInfo */
			    wlanoidRftestSetAutoTest,	/* pfnOidHandler */
			    &rRfATInfo,	/* pvInfoBuf */
			    sizeof(rRfATInfo),	/* u4InfoBufLen */
			    FALSE,	/* fgRead */
			    FALSE,	/* fgWaitResp */
			    TRUE,	/* fgCmd */
			    &u4BufLen);	/* pu4QryInfoLen */

	if (i4Status != WLAN_STATUS_SUCCESS)
		return -EFAULT;

	return i4Status;
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief  Hook API for Set WF0 TX Power.
 *
 * \param[in] prNetDev		Pointer to the Net Device
 * \param[in] u4TxPower0	Tx Gain of RF. The value is signed absolute
 *				power
 *            (2's complement representation) in unit of 0.5 dBm.
 * \param[out] None
 *
 * \retval 0			On success.
 * \retval -EFAULT		If kalIoctl return nonzero.
 */
/*----------------------------------------------------------------------------*/
int32_t MT_ATESetTxPower0(struct net_device *prNetDev,
			  uint32_t u4TxPower0)
{
	uint32_t u4BufLen = 0;
	int32_t i4Status;
	struct GLUE_INFO *prGlueInfo = NULL;
	struct PARAM_MTK_WIFI_TEST_STRUCT rRfATInfo;

	prGlueInfo = *((struct GLUE_INFO **) netdev_priv(prNetDev));

	DBGLOG(RFTEST, INFO, "QA_ATE_HOOK SetTxPower0=0x%02x\n",
	       u4TxPower0);

	if (u4TxPower0 > 0x3F) {
		u4TxPower0 += 128;
		DBGLOG(RFTEST, INFO, "QA_ATE_HOOK Negative Power =0x%02x\n",
		       u4TxPower0);
	}

	rRfATInfo.u4FuncIndex = RF_AT_FUNCID_POWER;
	rRfATInfo.u4FuncData = u4TxPower0;

	i4Status = kalIoctl(prGlueInfo,	/* prGlueInfo */
			    wlanoidRftestSetAutoTest,	/* pfnOidHandler */
			    &rRfATInfo,	/* pvInfoBuf */
			    sizeof(rRfATInfo),	/* u4InfoBufLen */
			    FALSE,	/* fgRead */
			    FALSE,	/* fgWaitResp */
			    TRUE,	/* fgCmd */
			    &u4BufLen);	/* pu4QryInfoLen */

	if (i4Status != WLAN_STATUS_SUCCESS)
		return -EFAULT;

	return i4Status;
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief  Hook API for Set Per Packet BW.
 *
 * \param[in] prNetDev		Pointer to the Net Device
 * \param[in] u4BW			0: 20 / 1: 40 / 2: 80 / 3: 160
 * \param[out] None
 *
 * \retval 0				On success.
 * \retval -EFAULT			If kalIoctl return nonzero.
 * \retval -EINVAL			If invalid argument.
 */
/*----------------------------------------------------------------------------*/
int32_t MT_ATESetPerPacketBW(struct net_device *prNetDev,
			     uint32_t u4BW)
{
	uint32_t u4BufLen = 0;
	int32_t i4Status;
	struct GLUE_INFO *prGlueInfo = NULL;
	struct PARAM_MTK_WIFI_TEST_STRUCT rRfATInfo;
	uint32_t u4BWMapping = 0;

	prGlueInfo = *((struct GLUE_INFO **) netdev_priv(prNetDev));

	DBGLOG(RFTEST, INFO, "QA_ATE_HOOK SetPerPacketBW=%d\n",
	       u4BW);

	if (u4BW > 6)
		return -EINVAL;

	/* BW Mapping in QA Tool
	 * 0: BW20
	 * 1: BW40
	 * 2: BW80
	 * 3: BW10
	 * 4: BW5
	 * 5: BW160C
	 * 6: BW160NC
	 */
	/* BW Mapping in FW
	 * 0: BW20
	 * 1: BW40
	 * 2: BW80
	 * 3: BW160C
	 * 4: BW160NC
	 * 5: BW5
	 * 6: BW10
	 */
	if (u4BW == 0)
		u4BWMapping = 0;
	else if (u4BW == 1)
		u4BWMapping = 1;
	else if (u4BW == 2)
		u4BWMapping = 2;
	else if (u4BW == 3)
		u4BWMapping = 6;
	else if (u4BW == 4)
		u4BWMapping = 5;
	else if (u4BW == 5)
		u4BWMapping = 3;
	else if (u4BW == 6)
		u4BWMapping = 4;

	rRfATInfo.u4FuncIndex = RF_AT_FUNCID_SET_DBW;
	rRfATInfo.u4FuncData = u4BWMapping;

	i4Status = kalIoctl(prGlueInfo,	/* prGlueInfo */
			    wlanoidRftestSetAutoTest,	/* pfnOidHandler */
			    &rRfATInfo,	/* pvInfoBuf */
			    sizeof(rRfATInfo),	/* u4InfoBufLen */
			    FALSE,	/* fgRead */
			    FALSE,	/* fgWaitResp */
			    TRUE,	/* fgCmd */
			    &u4BufLen);	/* pu4QryInfoLen */

	if (i4Status != WLAN_STATUS_SUCCESS)
		return -EFAULT;

	return i4Status;
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief  Hook API for Primary Channel Setting.
 *
 * \param[in] prNetDev		Pointer to the Net Device
 * \param[in] u4PrimaryCh	The range is from 0~7
 * \param[out] None
 *
 * \retval 0				On success.
 * \retval -EFAULT			If kalIoctl return nonzero.
 * \retval -EINVAL			If invalid argument.
 */
/*----------------------------------------------------------------------------*/
int32_t MT_ATEPrimarySetting(struct net_device *prNetDev,
			     uint32_t u4PrimaryCh)
{
	uint32_t u4BufLen = 0;
	int32_t i4Status;
	struct GLUE_INFO *prGlueInfo = NULL;
	struct PARAM_MTK_WIFI_TEST_STRUCT rRfATInfo;

	prGlueInfo = *((struct GLUE_INFO **) netdev_priv(prNetDev));

	DBGLOG(RFTEST, INFO, "QA_ATE_HOOK PrimarySetting=%d\n",
	       u4PrimaryCh);

	if (u4PrimaryCh > 7)
		return -EINVAL;

	rRfATInfo.u4FuncIndex = RF_AT_FUNCID_SET_PRIMARY_CH;
	rRfATInfo.u4FuncData = u4PrimaryCh;

	i4Status = kalIoctl(prGlueInfo,	/* prGlueInfo */
			    wlanoidRftestSetAutoTest,	/* pfnOidHandler */
			    &rRfATInfo,	/* pvInfoBuf */
			    sizeof(rRfATInfo),	/* u4InfoBufLen */
			    FALSE,	/* fgRead */
			    FALSE,	/* fgWaitResp */
			    TRUE,	/* fgCmd */
			    &u4BufLen);	/* pu4QryInfoLen */

	if (i4Status != WLAN_STATUS_SUCCESS)
		return -EFAULT;

	return i4Status;
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief  Hook API for Set TX Guard Interval.
 *
 * \param[in] prNetDev		Pointer to the Net Device
 * \param[in] u4SetTxGi		0: Normal GI, 1: Short GI
 * \param[out] None
 *
 * \retval 0				On success.
 * \retval -EFAULT			If kalIoctl return nonzero.
 * \retval -EINVAL			If invalid argument.
 */
/*----------------------------------------------------------------------------*/
int32_t MT_ATESetTxGi(struct net_device *prNetDev,
		      uint32_t u4SetTxGi)
{
	uint32_t u4BufLen = 0;
	int32_t i4Status;
	struct GLUE_INFO *prGlueInfo = NULL;
	struct PARAM_MTK_WIFI_TEST_STRUCT rRfATInfo;

	prGlueInfo = *((struct GLUE_INFO **) netdev_priv(prNetDev));

	DBGLOG(RFTEST, INFO, "QA_ATE_HOOK SetTxGi=%d\n", u4SetTxGi);

	if (u4SetTxGi != 0 && u4SetTxGi != 1)
		return -EINVAL;

	rRfATInfo.u4FuncIndex = RF_AT_FUNCID_GI;
	rRfATInfo.u4FuncData = u4SetTxGi;

	i4Status = kalIoctl(prGlueInfo,	/* prGlueInfo */
			    wlanoidRftestSetAutoTest,	/* pfnOidHandler */
			    &rRfATInfo,	/* pvInfoBuf */
			    sizeof(rRfATInfo),	/* u4InfoBufLen */
			    FALSE,	/* fgRead */
			    FALSE,	/* fgWaitResp */
			    TRUE,	/* fgCmd */
			    &u4BufLen);	/* pu4QryInfoLen */

	if (i4Status != WLAN_STATUS_SUCCESS)
		return -EFAULT;

	return i4Status;
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief  Hook API for Set TX Path.
 *
 * \param[in] prNetDev		Pointer to the Net Device
 * \param[in] u4Tx_path		0: All Tx, 1: WF0, 2: WF1, 3: WF0+WF1
 * \param[out] None
 *
 * \retval 0				On success.
 * \retval -EFAULT			If kalIoctl return nonzero.
 * \retval -EINVAL			If invalid argument.
 */
/*----------------------------------------------------------------------------*/
int32_t MT_ATESetTxPath(struct net_device *prNetDev,
			uint32_t u4Tx_path)
{
	uint32_t u4BufLen = 0;
	int32_t i4Status;
	struct GLUE_INFO *prGlueInfo = NULL;
	struct PARAM_MTK_WIFI_TEST_STRUCT rRfATInfo;

	prGlueInfo = *((struct GLUE_INFO **) netdev_priv(prNetDev));

	DBGLOG(RFTEST, INFO, "QA_ATE_HOOK u4Tx_path=%d\n",
	       u4Tx_path);

	rRfATInfo.u4FuncIndex = RF_AT_FUNCID_SET_TX_PATH;
	rRfATInfo.u4FuncData = u4Tx_path;

	i4Status = kalIoctl(prGlueInfo,	/* prGlueInfo */
			    wlanoidRftestSetAutoTest,	/* pfnOidHandler */
			    &rRfATInfo,	/* pvInfoBuf */
			    sizeof(rRfATInfo),	/* u4InfoBufLen */
			    FALSE,	/* fgRead */
			    FALSE,	/* fgWaitResp */
			    TRUE,	/* fgCmd */
			    &u4BufLen);	/* pu4QryInfoLen */

	if (i4Status != WLAN_STATUS_SUCCESS)
		return -EFAULT;

	return i4Status;
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief  Hook API for Set TX Payload Fix/Random.
 *
 * \param[in] prNetDev		Pointer to the Net Device
 * \param[in] u4Stbc	       0: Disable , 1 : Enable
 * \param[out] None
 *
 * \retval 0				On success.
 * \retval -EFAULT			If kalIoctl return nonzero.
 * \retval -EINVAL			If invalid argument.
 */
/*----------------------------------------------------------------------------*/
int32_t MT_ATESetTxPayLoad(struct net_device *prNetDev,
			   uint32_t u4Gen_payload_rule, uint8_t ucPayload)
{
	uint32_t u4BufLen = 0;
	int32_t i4Status;
	struct GLUE_INFO *prGlueInfo = NULL;
	struct PARAM_MTK_WIFI_TEST_STRUCT rRfATInfo;

	prGlueInfo = *((struct GLUE_INFO **) netdev_priv(prNetDev));

	DBGLOG(RFTEST, INFO, "QA_ATE_HOOK rule=%d, len =0x%x\n",
	       u4Gen_payload_rule, ucPayload);

	rRfATInfo.u4FuncIndex = RF_AT_FUNCID_SET_PAYLOAD;
	rRfATInfo.u4FuncData = ((u4Gen_payload_rule << 16) |
				ucPayload);

	i4Status = kalIoctl(prGlueInfo,	/* prGlueInfo */
			    wlanoidRftestSetAutoTest,	/* pfnOidHandler */
			    &rRfATInfo,	/* pvInfoBuf */
			    sizeof(rRfATInfo),	/* u4InfoBufLen */
			    FALSE,	/* fgRead */
			    FALSE,	/* fgWaitResp */
			    TRUE,	/* fgCmd */
			    &u4BufLen);	/* pu4QryInfoLen */

	if (i4Status != WLAN_STATUS_SUCCESS)
		return -EFAULT;

	return i4Status;
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief  Hook API for Set TX STBC.
 *
 * \param[in] prNetDev		Pointer to the Net Device
 * \param[in] u4Stbc	       0: Disable , 1 : Enable
 * \param[out] None
 *
 * \retval 0				On success.
 * \retval -EFAULT			If kalIoctl return nonzero.
 * \retval -EINVAL			If invalid argument.
 */
/*----------------------------------------------------------------------------*/
int32_t MT_ATESetTxSTBC(struct net_device *prNetDev,
			uint32_t u4Stbc)
{
	uint32_t u4BufLen = 0;
	int32_t i4Status;
	struct GLUE_INFO *prGlueInfo = NULL;
	struct PARAM_MTK_WIFI_TEST_STRUCT rRfATInfo;

	prGlueInfo = *((struct GLUE_INFO **) netdev_priv(prNetDev));

	DBGLOG(RFTEST, INFO, "QA_ATE_HOOK u4Stbc=%d\n", u4Stbc);

	if (u4Stbc > 1)
		return -EINVAL;

	rRfATInfo.u4FuncIndex = RF_AT_FUNCID_STBC;
	rRfATInfo.u4FuncData = u4Stbc;

	i4Status = kalIoctl(prGlueInfo,	/* prGlueInfo */
			    wlanoidRftestSetAutoTest,	/* pfnOidHandler */
			    &rRfATInfo,	/* pvInfoBuf */
			    sizeof(rRfATInfo),	/* u4InfoBufLen */
			    FALSE,	/* fgRead */
			    FALSE,	/* fgWaitResp */
			    TRUE,	/* fgCmd */
			    &u4BufLen);	/* pu4QryInfoLen */

	if (i4Status != WLAN_STATUS_SUCCESS)
		return -EFAULT;

	return i4Status;
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief  Hook API for Set TX Nss.
 *
 * \param[in] prNetDev		Pointer to the Net Device
 * \param[in] u4Nss		       1/2
 * \param[out] None
 *
 * \retval 0				On success.
 * \retval -EFAULT			If kalIoctl return nonzero.
 * \retval -EINVAL			If invalid argument.
 */
/*----------------------------------------------------------------------------*/
int32_t MT_ATESetTxVhtNss(struct net_device *prNetDev,
			  uint32_t u4VhtNss)
{
	uint32_t u4BufLen = 0;
	int32_t i4Status;
	struct GLUE_INFO *prGlueInfo = NULL;
	struct PARAM_MTK_WIFI_TEST_STRUCT rRfATInfo;

	prGlueInfo = *((struct GLUE_INFO **) netdev_priv(prNetDev));

	DBGLOG(RFTEST, INFO, "QA_ATE_HOOK u4Nss=%d\n", u4VhtNss);

	rRfATInfo.u4FuncIndex = RF_AT_FUNCID_SET_NSS;
	rRfATInfo.u4FuncData = u4VhtNss;

	i4Status = kalIoctl(prGlueInfo,	/* prGlueInfo */
			    wlanoidRftestSetAutoTest,	/* pfnOidHandler */
			    &rRfATInfo,	/* pvInfoBuf */
			    sizeof(rRfATInfo),	/* u4InfoBufLen */
			    FALSE,	/* fgRead */
			    FALSE,	/* fgWaitResp */
			    TRUE,	/* fgCmd */
			    &u4BufLen);	/* pu4QryInfoLen */

	if (i4Status != WLAN_STATUS_SUCCESS)
		return -EFAULT;

	return i4Status;
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief  Hook API for Set Rate.
 *
 * \param[in] prNetDev		Pointer to the Net Device
 * \param[in] u4Rate		Rate
 * \param[out] None
 *
 * \retval 0				On success.
 * \retval -EFAULT			If kalIoctl return nonzero.
 */
/*----------------------------------------------------------------------------*/
int32_t MT_ATESetRate(struct net_device *prNetDev,
		      uint32_t u4Rate)
{
	uint32_t u4BufLen = 0;
	int32_t i4Status;
	struct GLUE_INFO *prGlueInfo = NULL;
	struct PARAM_MTK_WIFI_TEST_STRUCT rRfATInfo;

	prGlueInfo = *((struct GLUE_INFO **) netdev_priv(prNetDev));

	DBGLOG(RFTEST, INFO, "QA_ATE_HOOK SetRate=0x%08x\n",
	       u4Rate);

	rRfATInfo.u4FuncIndex = RF_AT_FUNCID_RATE;
	rRfATInfo.u4FuncData = u4Rate;

	i4Status = kalIoctl(prGlueInfo,	/* prGlueInfo */
			    wlanoidRftestSetAutoTest,	/* pfnOidHandler */
			    &rRfATInfo,	/* pvInfoBuf */
			    sizeof(rRfATInfo),	/* u4InfoBufLen */
			    FALSE,	/* fgRead */
			    FALSE,	/* fgWaitResp */
			    TRUE,	/* fgCmd */
			    &u4BufLen);	/* pu4QryInfoLen */

	if (i4Status != WLAN_STATUS_SUCCESS)
		return -EFAULT;

	return i4Status;
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief  Hook API for Set Encode Mode.
 *
 * \param[in] prNetDev		Pointer to the Net Device
 * \param[in] u4Ldpc		0: BCC / 1: LDPC
 * \param[out] None
 *
 * \retval 0				On success.
 * \retval -EFAULT			If kalIoctl return nonzero.
 * \retval -EINVAL			If invalid argument.
 */
/*----------------------------------------------------------------------------*/
int32_t MT_ATESetEncodeMode(struct net_device *prNetDev,
			    uint32_t u4Ldpc)
{
	uint32_t u4BufLen = 0;
	int32_t i4Status;
	struct GLUE_INFO *prGlueInfo = NULL;
	struct PARAM_MTK_WIFI_TEST_STRUCT rRfATInfo;

	prGlueInfo = *((struct GLUE_INFO **) netdev_priv(prNetDev));

	DBGLOG(RFTEST, INFO, "QA_ATE_HOOK SetEncodeMode=%d\n",
	       u4Ldpc);

	if (u4Ldpc != 0 && u4Ldpc != 1)
		return -EINVAL;

	rRfATInfo.u4FuncIndex = RF_AT_FUNCID_SET_ENCODE_MODE;
	rRfATInfo.u4FuncData = u4Ldpc;

	i4Status = kalIoctl(prGlueInfo,	/* prGlueInfo */
			    wlanoidRftestSetAutoTest,	/* pfnOidHandler */
			    &rRfATInfo,	/* pvInfoBuf */
			    sizeof(rRfATInfo),	/* u4InfoBufLen */
			    FALSE,	/* fgRead */
			    FALSE,	/* fgWaitResp */
			    TRUE,	/* fgCmd */
			    &u4BufLen);	/* pu4QryInfoLen */

	if (i4Status != WLAN_STATUS_SUCCESS)
		return -EFAULT;

	return i4Status;
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief  Hook API for Set iBF Enable.
 *
 * \param[in] prNetDev		Pointer to the Net Device
 * \param[in] u4iBF		0: disable / 1: enable
 * \param[out] None
 *
 * \retval 0				On success.
 * \retval -EFAULT			If kalIoctl return nonzero.
 * \retval -EINVAL			If invalid argument.
 */
/*----------------------------------------------------------------------------*/
int32_t MT_ATESetiBFEnable(struct net_device *prNetDev,
			   uint32_t u4iBF)
{
	uint32_t u4BufLen = 0;
	int32_t i4Status;
	struct GLUE_INFO *prGlueInfo = NULL;
	struct PARAM_MTK_WIFI_TEST_STRUCT rRfATInfo;

	prGlueInfo = *((struct GLUE_INFO **) netdev_priv(prNetDev));

	DBGLOG(RFTEST, INFO, "QA_ATE_HOOK SetiBFEnable=%d\n",
	       u4iBF);

	if (u4iBF != 0 && u4iBF != 1)
		return -EINVAL;

	rRfATInfo.u4FuncIndex = RF_AT_FUNCID_SET_IBF_ENABLE;
	rRfATInfo.u4FuncData = u4iBF;

	i4Status = kalIoctl(prGlueInfo,	/* prGlueInfo */
			    wlanoidRftestSetAutoTest,	/* pfnOidHandler */
			    &rRfATInfo,	/* pvInfoBuf */
			    sizeof(rRfATInfo),	/* u4InfoBufLen */
			    FALSE,	/* fgRead */
			    FALSE,	/* fgWaitResp */
			    TRUE,	/* fgCmd */
			    &u4BufLen);	/* pu4QryInfoLen */

	if (i4Status != WLAN_STATUS_SUCCESS)
		return -EFAULT;

	return i4Status;
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief  Hook API for Set eBF Enable.
 *
 * \param[in] prNetDev		Pointer to the Net Device
 * \param[in] u4eBF		0: disable / 1: enable
 * \param[out] None
 *
 * \retval 0				On success.
 * \retval -EFAULT			If kalIoctl return nonzero.
 * \retval -EINVAL			If invalid argument.
 */
/*----------------------------------------------------------------------------*/
int32_t MT_ATESeteBFEnable(struct net_device *prNetDev,
			   uint32_t u4eBF)
{
	uint32_t u4BufLen = 0;
	int32_t i4Status;
	struct GLUE_INFO *prGlueInfo = NULL;
	struct PARAM_MTK_WIFI_TEST_STRUCT rRfATInfo;

	prGlueInfo = *((struct GLUE_INFO **) netdev_priv(prNetDev));

	DBGLOG(RFTEST, INFO, "QA_ATE_HOOK SeteBFEnable=%d\n",
	       u4eBF);

	if (u4eBF != 0 && u4eBF != 1)
		return -EINVAL;

	rRfATInfo.u4FuncIndex = RF_AT_FUNCID_SET_EBF_ENABLE;
	rRfATInfo.u4FuncData = u4eBF;

	i4Status = kalIoctl(prGlueInfo,	/* prGlueInfo */
			    wlanoidRftestSetAutoTest,	/* pfnOidHandler */
			    &rRfATInfo,	/* pvInfoBuf */
			    sizeof(rRfATInfo),	/* u4InfoBufLen */
			    FALSE,	/* fgRead */
			    FALSE,	/* fgWaitResp */
			    TRUE,	/* fgCmd */
			    &u4BufLen);	/* pu4QryInfoLen */

	if (i4Status != WLAN_STATUS_SUCCESS)
		return -EFAULT;

	return i4Status;
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief  Hook API for Set MAC Address.
 *
 * \param[in] prNetDev		Pointer to the Net Device
 * \param[in] u4Type		Address type
 * \param[in] ucAddr		Address ready to set
 * \param[out] None
 *
 * \retval 0				On success.
 * \retval -EFAULT			If kalIoctl return nonzero.
 */
/*----------------------------------------------------------------------------*/
int32_t MT_ATESetMACAddress(struct net_device *prNetDev,
			    uint32_t u4Type, uint8_t *ucAddr)
{
	uint32_t u4BufLen = 0;
	int32_t i4Status;
	struct GLUE_INFO *prGlueInfo = NULL;
	struct PARAM_MTK_WIFI_TEST_STRUCT rRfATInfo;

	prGlueInfo = *((struct GLUE_INFO **) netdev_priv(prNetDev));

	DBGLOG(RFTEST, ERROR,
	       "QA_ATE_HOOK SetMACAddress Type = %d, Addr = %02x:%02x:%02x:%02x:%02x:%02x\n",
	       u4Type, ucAddr[0], ucAddr[1], ucAddr[2], ucAddr[3],
	       ucAddr[4], ucAddr[5]);

#if 1
	rRfATInfo.u4FuncIndex = u4Type;
	memcpy(&rRfATInfo.u4FuncData, ucAddr, 4);

	i4Status = kalIoctl(prGlueInfo,	/* prGlueInfo */
			    wlanoidRftestSetAutoTest,	/* pfnOidHandler */
			    &rRfATInfo,	/* pvInfoBuf */
			    sizeof(rRfATInfo),	/* u4InfoBufLen */
			    FALSE,	/* fgRead */
			    FALSE,	/* fgWaitResp */
			    TRUE,	/* fgCmd */
			    &u4BufLen);	/* pu4QryInfoLen */
	if (i4Status != WLAN_STATUS_SUCCESS)
		return -EFAULT;
#endif
	rRfATInfo.u4FuncIndex = u4Type | BIT(18);
	memset(&rRfATInfo.u4FuncData, 0,
	       sizeof(rRfATInfo.u4FuncData));
	memcpy(&rRfATInfo.u4FuncData, ucAddr + 4, 2);

	i4Status = kalIoctl(prGlueInfo,	/* prGlueInfo */
			    wlanoidRftestSetAutoTest,	/* pfnOidHandler */
			    &rRfATInfo,	/* pvInfoBuf */
			    sizeof(rRfATInfo),	/* u4InfoBufLen */
			    FALSE,	/* fgRead */
			    FALSE,	/* fgWaitResp */
			    TRUE,	/* fgCmd */
			    &u4BufLen);	/* pu4QryInfoLen */

	if (i4Status != WLAN_STATUS_SUCCESS)
		return -EFAULT;

	return i4Status;
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief  Hook API for RX Vector Dump.
 *
 * \param[in] prNetDev		Pointer to the Net Device
 * \param[in] u4Type
 * \param[in] u4On_off
 * \param[in] u4Size
 * \param[out] None
 *
 * \retval 0				On success.
 * \retval -EFAULT			If kalIoctl return nonzero.
 */
/*----------------------------------------------------------------------------*/
int32_t MT_ATELogOnOff(struct net_device *prNetDev,
		       uint32_t u4Type, uint32_t u4On_off, uint32_t u4Size)
{
	int32_t i4Status = 0, i, i4TargetLength = 0,
		i4MaxDumpRXVCnt = 500;
	uint32_t u4BufLen = 0, rxv;
	struct GLUE_INFO *prGlueInfo = NULL;
	struct PARAM_MTK_WIFI_TEST_STRUCT rRfATInfo;

	prGlueInfo = *((struct GLUE_INFO **) netdev_priv(prNetDev));

	DBGLOG(RFTEST, INFO, "QA_ATE_HOOK MT_ATELogOnOff\n");

	switch (u4Type) {
	case ATE_LOG_RXV:
		DBGLOG(RFTEST, INFO,
		       "QA_ATE_HOOK MT_ATELogOnOff : ATE_LOG_RXV\n\n");
		break;
	case ATE_LOG_RDD:
		DBGLOG(RFTEST, INFO,
		       "QA_ATE_HOOK MT_ATELogOnOff : ATE_LOG_RDD\n\n");
		break;
	case ATE_LOG_RE_CAL:
		DBGLOG(RFTEST, INFO,
		       "QA_ATE_HOOK MT_ATELogOnOff : ATE_LOG_RE_CAL\n\n");
		break;
	case ATE_LOG_RXINFO:
		DBGLOG(RFTEST, INFO,
		       "QA_ATE_HOOK MT_ATELogOnOff : ATE_LOG_RXINFO\n\n");
		break;
	case ATE_LOG_TXDUMP:
		DBGLOG(RFTEST, INFO,
		       "QA_ATE_HOOK MT_ATELogOnOff : ATE_LOG_TXDUMP\n\n");
		break;
	case ATE_LOG_TEST:
		DBGLOG(RFTEST, INFO,
		       "QA_ATE_HOOK MT_ATELogOnOff : ATE_LOG_TEST\n\n");
		break;
	default:
		DBGLOG(RFTEST, INFO,
		       "QA_ATE_HOOK log type %d not supported\n\n", u4Type);
	}

	if ((u4On_off == ATE_LOG_DUMP) && (u4Type == ATE_LOG_RXV)) {
		rRfATInfo.u4FuncIndex = RF_AT_FUNCID_RESULT_INFO;
		rRfATInfo.u4FuncData = RF_AT_FUNCID_RXV_DUMP;

		i4Status = kalIoctl(prGlueInfo, wlanoidRftestQueryAutoTest,
				    &rRfATInfo, sizeof(rRfATInfo),
				    TRUE, TRUE, TRUE, &u4BufLen);

		if (i4Status == 0) {
			i4TargetLength = rRfATInfo.u4FuncData * 36;
			DBGLOG(RFTEST, ERROR,
			       "QA_ATE_HOOK Get RX Vector Total size = %d\n",
			       i4TargetLength);

			if (i4TargetLength >= (i4MaxDumpRXVCnt * 36))
				i4TargetLength = (i4MaxDumpRXVCnt * 36);
		} else {
			DBGLOG(RFTEST, ERROR,
			       "QA_ATE_HOOK Get RX Vector Total Size Error!!!!\n\n");
		}

		TOOL_PRINTLOG(RFTEST, ERROR, "[LOG DUMP START]\n");

		for (i = 0; i < i4TargetLength; i += 4) {
			rRfATInfo.u4FuncIndex = RF_AT_FUNCID_RXV_DUMP;
			rRfATInfo.u4FuncData = i;

			i4Status = kalIoctl(prGlueInfo,
					wlanoidRftestQueryAutoTest,
					&rRfATInfo, sizeof(rRfATInfo),
					TRUE, TRUE, TRUE, &u4BufLen);

			if (i4Status == 0) {
				rxv = rRfATInfo.u4FuncData;

				if (i % 36 == 0)
					TOOL_PRINTLOG(RFTEST, ERROR,
						      "%%[RXV DUMP START][%d]\n",
						      (i / 36) + 1);

				TOOL_PRINTLOG(RFTEST, ERROR, "[RXVD%d]%08x\n",
					      ((i % 36) / 4) + 1, rxv);

				if (((i % 36) / 4) + 1 == 9)
					TOOL_PRINTLOG(RFTEST, ERROR,
						      "[RXV DUMP END]\n");
			}
		}

		TOOL_PRINTLOG(RFTEST, ERROR, "[LOG DUMP END]\n");
	}

	return i4Status;
}

/*----------------------------------------------------------------------------*/
/*!
* \brief  Hook API for Get RX Vector Dump.
*
* \param[in] prNetDev		Pointer to the Net Device
* \param[out] pucData		Pointer to the output data buffer
* \param[out] pCount		Pointer to the length of data
*
* \retval 0				On success.
* \retval -EFAULT			If kalIoctl return nonzero.
*/
/*----------------------------------------------------------------------------*/
int32_t MT_ATEGetDumpRXV(struct net_device *prNetDev,
			 uint8_t *pucData,
			 int32_t *pCount)
{
	int32_t i4Status = 0;
	struct GLUE_INFO *prGlueInfo = NULL;
	struct PARAM_MTK_WIFI_TEST_STRUCT rRfATInfo;
	uint32_t i = 0, u4BufLen = 0, u4Value = 0;
	uint32_t u4RespLen = 2, i4TargetLength = 0;

	rRfATInfo.u4FuncIndex = RF_AT_FUNCID_RESULT_INFO;
	rRfATInfo.u4FuncData = RF_AT_FUNCID_RXV_DUMP;

	prGlueInfo = *((struct GLUE_INFO **) netdev_priv(prNetDev));
	ASSERT(prGlueInfo);

	i4Status = kalIoctl(prGlueInfo,
			    wlanoidRftestQueryAutoTest,
			    &rRfATInfo,
			    sizeof(rRfATInfo),
			    TRUE,
			    TRUE,
			    TRUE,
			    &u4BufLen);

	if (i4Status == 0) {
		DBGLOG(RFTEST, INFO, "Get RX Vector Total count = %d\n",
				     rRfATInfo.u4FuncData);
		if (rRfATInfo.u4FuncData > MAX_RXV_DUMP_COUNT)
			rRfATInfo.u4FuncData = MAX_RXV_DUMP_COUNT;

		i4TargetLength = rRfATInfo.u4FuncData * 36;
		u4Value = ntohl(rRfATInfo.u4FuncData);
		kalMemCopy(pucData + u4RespLen,
			   (uint8_t *)&u4Value,
			   sizeof(u4Value));
		u4RespLen += sizeof(u4Value);
	} else {
		DBGLOG(RFTEST, ERROR, "Get RX Vector Total Size Error!!\n");
		return -EFAULT;
	}

	for (i = 0; i < i4TargetLength; i += 4) {
		rRfATInfo.u4FuncIndex = RF_AT_FUNCID_RXV_DUMP;
		rRfATInfo.u4FuncData = i;

		i4Status = kalIoctl(prGlueInfo,
					wlanoidRftestQueryAutoTest,
					&rRfATInfo,
					sizeof(rRfATInfo),
					TRUE,
					TRUE,
					TRUE,
					&u4BufLen);

		if (i4Status == 0) {
			u4Value = ntohl(rRfATInfo.u4FuncData);
			kalMemCopy(pucData + u4RespLen,
				   (uint8_t *)&u4Value,
				   sizeof(u4Value));
			u4RespLen += sizeof(u4Value);
		} else {
			DBGLOG(RFTEST, ERROR,
			      "Error getting index[%d]'s RXV dump data!!\n", i);
			return -EFAULT;
		}
	}

	*pCount = i4TargetLength;
	/* dumpMemory32((PUINT_32)pucData, i4TargetLength + 8); */

	return 0;
}


/*----------------------------------------------------------------------------*/
/*!
 * \brief  Hook API for Reset Counter.
 *
 * \param[in] prNetDev		Pointer to the Net Device
 * \param[out] None
 *
 * \retval 0				On success.
 * \retval -EFAULT			If kalIoctl return nonzero.
 */
/*----------------------------------------------------------------------------*/
int32_t MT_ATEResetTXRXCounter(struct net_device *prNetDev)
{
	uint32_t u4BufLen = 0;
	int32_t i4Status;
	struct GLUE_INFO *prGlueInfo = NULL;
	struct PARAM_MTK_WIFI_TEST_STRUCT rRfATInfo;

	prGlueInfo = *((struct GLUE_INFO **) netdev_priv(prNetDev));

	DBGLOG(RFTEST, INFO,
	       "QA_ATE_HOOK MT_ATEResetTXRXCounter\n");

	rRfATInfo.u4FuncIndex = RF_AT_FUNCID_RESETTXRXCOUNTER;
	rRfATInfo.u4FuncData = 0;

	i4Status = kalIoctl(prGlueInfo,	/* prGlueInfo */
			    wlanoidRftestSetAutoTest,	/* pfnOidHandler */
			    &rRfATInfo,	/* pvInfoBuf */
			    sizeof(rRfATInfo),	/* u4InfoBufLen */
			    FALSE,	/* fgRead */
			    FALSE,	/* fgWaitResp */
			    TRUE,	/* fgCmd */
			    &u4BufLen);	/* pu4QryInfoLen */

	if (i4Status != WLAN_STATUS_SUCCESS)
		return -EFAULT;

	return i4Status;
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief  Hook API for Set DBDC Band Index.
 *
 * \param[in] prNetDev		Pointer to the Net Device
 * \param[in] u4BandIdx       Band Index Number ready to set
 * \param[out] None
 *
 * \retval 0				On success.
 * \retval -EFAULT			If kalIoctl return nonzero.
 */
/*----------------------------------------------------------------------------*/
int32_t MT_ATESetDBDCBandIndex(struct net_device *prNetDev,
			       uint32_t u4BandIdx)
{
	uint32_t u4BufLen = 0;
	int32_t i4Status;
	struct GLUE_INFO *prGlueInfo = NULL;
	struct PARAM_MTK_WIFI_TEST_STRUCT rRfATInfo;

	prGlueInfo = *((struct GLUE_INFO **) netdev_priv(prNetDev));

	DBGLOG(RFTEST, INFO,
	       "QA_ATE_HOOK MT_ATESetDBDCBandIndex\n");

	rRfATInfo.u4FuncIndex = RF_AT_FUNCID_SET_DBDC_BAND_IDX;
	rRfATInfo.u4FuncData = u4BandIdx;

	i4Status = kalIoctl(prGlueInfo,	/* prGlueInfo */
			    wlanoidRftestSetAutoTest,	/* pfnOidHandler */
			    &rRfATInfo,	/* pvInfoBuf */
			    sizeof(rRfATInfo),	/* u4InfoBufLen */
			    FALSE,	/* fgRead */
			    FALSE,	/* fgWaitResp */
			    TRUE,	/* fgCmd */
			    &u4BufLen);	/* pu4QryInfoLen */

	if (i4Status != WLAN_STATUS_SUCCESS)
		return -EFAULT;

	return i4Status;
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief  Hook API for Set Band. (2G or 5G)
 *
 * \param[in] prNetDev		Pointer to the Net Device
 * \param[in] i4Band		Band to set
 * \param[out] None
 *
 * \retval 0				On success.
 * \retval -EFAULT			If kalIoctl return nonzero.
 */
/*----------------------------------------------------------------------------*/
int32_t MT_ATESetBand(struct net_device *prNetDev,
		      int32_t i4Band)
{
	uint32_t u4BufLen = 0;
	int32_t i4Status;
	struct GLUE_INFO *prGlueInfo = NULL;
	struct PARAM_MTK_WIFI_TEST_STRUCT rRfATInfo;

	prGlueInfo = *((struct GLUE_INFO **) netdev_priv(prNetDev));

	DBGLOG(RFTEST, INFO, "QA_ATE_HOOK MT_ATESetBand\n");

	rRfATInfo.u4FuncIndex = RF_AT_FUNCID_SET_BAND;
	rRfATInfo.u4FuncData = i4Band;

	i4Status = kalIoctl(prGlueInfo,	/* prGlueInfo */
			    wlanoidRftestSetAutoTest,	/* pfnOidHandler */
			    &rRfATInfo,	/* pvInfoBuf */
			    sizeof(rRfATInfo),	/* u4InfoBufLen */
			    FALSE,	/* fgRead */
			    FALSE,	/* fgWaitResp */
			    TRUE,	/* fgCmd */
			    &u4BufLen);	/* pu4QryInfoLen */

	if (i4Status != WLAN_STATUS_SUCCESS)
		return -EFAULT;

	return i4Status;
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief  Hook API for Set Tx Tone Type. (2G or 5G)
 *
 * \param[in] prNetDev		Pointer to the Net Device
 * \param[in] i4ToneType	Set Single or Two Tone.
 * \param[out] None
 *
 * \retval 0				On success.
 * \retval -EFAULT			If kalIoctl return nonzero.
 */
/*----------------------------------------------------------------------------*/
int32_t MT_ATESetTxToneType(struct net_device *prNetDev,
			    int32_t i4ToneType)
{
	uint32_t u4BufLen = 0;
	int32_t i4Status;
	struct GLUE_INFO *prGlueInfo = NULL;
	struct PARAM_MTK_WIFI_TEST_STRUCT rRfATInfo;

	prGlueInfo = *((struct GLUE_INFO **) netdev_priv(prNetDev));

	DBGLOG(RFTEST, INFO, "QA_ATE_HOOK MT_ATESetTxToneType\n");

	rRfATInfo.u4FuncIndex = RF_AT_FUNCID_SET_TONE_TYPE;
	rRfATInfo.u4FuncData = i4ToneType;

	i4Status = kalIoctl(prGlueInfo,	/* prGlueInfo */
			    wlanoidRftestSetAutoTest,	/* pfnOidHandler */
			    &rRfATInfo,	/* pvInfoBuf */
			    sizeof(rRfATInfo),	/* u4InfoBufLen */
			    FALSE,	/* fgRead */
			    FALSE,	/* fgWaitResp */
			    TRUE,	/* fgCmd */
			    &u4BufLen);	/* pu4QryInfoLen */

	if (i4Status != WLAN_STATUS_SUCCESS)
		return -EFAULT;

	return i4Status;
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief  Hook API for Set Tx Tone Frequency. (DC/5M/10M/20M/40M)
 *
 * \param[in] prNetDev		Pointer to the Net Device
 * \param[in] i4ToneFreq	Set Tx Tone Frequency.
 * \param[out] None
 *
 * \retval 0				On success.
 * \retval -EFAULT			If kalIoctl return nonzero.
 */
/*----------------------------------------------------------------------------*/
int32_t MT_ATESetTxToneBW(struct net_device *prNetDev,
			  int32_t i4ToneFreq)
{
	uint32_t u4BufLen = 0;
	int32_t i4Status;
	struct GLUE_INFO *prGlueInfo = NULL;
	struct PARAM_MTK_WIFI_TEST_STRUCT rRfATInfo;

	prGlueInfo = *((struct GLUE_INFO **) netdev_priv(prNetDev));

	DBGLOG(RFTEST, INFO, "QA_ATE_HOOK MT_ATESetTxToneBW\n");

	rRfATInfo.u4FuncIndex = RF_AT_FUNCID_SET_TONE_BW;
	rRfATInfo.u4FuncData = i4ToneFreq;

	i4Status = kalIoctl(prGlueInfo,	/* prGlueInfo */
			    wlanoidRftestSetAutoTest,	/* pfnOidHandler */
			    &rRfATInfo,	/* pvInfoBuf */
			    sizeof(rRfATInfo),	/* u4InfoBufLen */
			    FALSE,	/* fgRead */
			    FALSE,	/* fgWaitResp */
			    TRUE,	/* fgCmd */
			    &u4BufLen);	/* pu4QryInfoLen */

	if (i4Status != WLAN_STATUS_SUCCESS)
		return -EFAULT;

	return i4Status;
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief  Hook API for Set Tx Tone DC Offset. (DC Offset I / DC Offset Q)
 *
 * \param[in] prNetDev		Pointer to the Net Device
 * \param[in] i4DcOffsetI	Set Tx Tone DC Offset I.
 * \param[in] i4DcOffsetQ	Set Tx Tone DC Offset Q.
 * \param[out] None
 *
 * \retval 0				On success.
 * \retval -EFAULT			If kalIoctl return nonzero.
 */
/*----------------------------------------------------------------------------*/
int32_t MT_ATESetTxToneDCOffset(struct net_device *prNetDev,
				int32_t i4DcOffsetI, int32_t i4DcOffsetQ)
{
	uint32_t u4BufLen = 0;
	int32_t i4Status;
	struct GLUE_INFO *prGlueInfo = NULL;
	struct PARAM_MTK_WIFI_TEST_STRUCT rRfATInfo;

	prGlueInfo = *((struct GLUE_INFO **) netdev_priv(prNetDev));

	DBGLOG(RFTEST, INFO,
	       "QA_ATE_HOOK MT_ATESetTxToneDCOffset\n");

	rRfATInfo.u4FuncIndex = RF_AT_FUNCID_SET_TONE_DC_OFFSET;
	rRfATInfo.u4FuncData = i4DcOffsetQ << 16 | i4DcOffsetI;

	i4Status = kalIoctl(prGlueInfo,	/* prGlueInfo */
			    wlanoidRftestSetAutoTest,	/* pfnOidHandler */
			    &rRfATInfo,	/* pvInfoBuf */
			    sizeof(rRfATInfo),	/* u4InfoBufLen */
			    FALSE,	/* fgRead */
			    FALSE,	/* fgWaitResp */
			    TRUE,	/* fgCmd */
			    &u4BufLen);	/* pu4QryInfoLen */

	if (i4Status != WLAN_STATUS_SUCCESS)
		return -EFAULT;

	return i4Status;
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief  Hook API for Set Tx Tone Power. (RF and Digital)
 *
 * \param[in] prNetDev		Pointer to the Net Device
 * \param[in] i4AntIndex
 * \param[in] i4RF_Power
 * \param[in] i4Digi_Power
 * \param[out] None
 *
 * \retval 0				On success.
 * \retval -EFAULT			If kalIoctl return nonzero.
 */
/*----------------------------------------------------------------------------*/
int32_t MT_ATESetDBDCTxTonePower(struct net_device *prNetDev,
		int32_t i4AntIndex, int32_t i4RF_Power, int32_t i4Digi_Power)
{
	uint32_t u4BufLen = 0;
	int32_t i4Status;
	struct GLUE_INFO *prGlueInfo = NULL;
	struct PARAM_MTK_WIFI_TEST_STRUCT rRfATInfo;

	prGlueInfo = *((struct GLUE_INFO **) netdev_priv(prNetDev));

	DBGLOG(RFTEST, INFO,
	       "QA_ATE_HOOK MT_ATESetDBDCTxTonePower\n");

	rRfATInfo.u4FuncIndex = RF_AT_FUNCID_SET_TONE_RF_GAIN;
	rRfATInfo.u4FuncData = i4AntIndex << 16 | i4RF_Power;

	i4Status = kalIoctl(prGlueInfo,	/* prGlueInfo */
			    wlanoidRftestSetAutoTest,	/* pfnOidHandler */
			    &rRfATInfo,	/* pvInfoBuf */
			    sizeof(rRfATInfo),	/* u4InfoBufLen */
			    FALSE,	/* fgRead */
			    FALSE,	/* fgWaitResp */
			    TRUE,	/* fgCmd */
			    &u4BufLen);	/* pu4QryInfoLen */

	if (i4Status != WLAN_STATUS_SUCCESS)
		return -EFAULT;

	rRfATInfo.u4FuncIndex = RF_AT_FUNCID_SET_TONE_DIGITAL_GAIN;
	rRfATInfo.u4FuncData = i4AntIndex << 16 | i4Digi_Power;

	i4Status = kalIoctl(prGlueInfo,	/* prGlueInfo */
			    wlanoidRftestSetAutoTest,	/* pfnOidHandler */
			    &rRfATInfo,	/* pvInfoBuf */
			    sizeof(rRfATInfo),	/* u4InfoBufLen */
			    FALSE,	/* fgRead */
			    FALSE,	/* fgWaitResp */
			    TRUE,	/* fgCmd */
			    &u4BufLen);	/* pu4QryInfoLen */

	if (i4Status != WLAN_STATUS_SUCCESS)
		return -EFAULT;

	return i4Status;
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief  Hook API for Start Tx Tone.
 *
 * \param[in] prNetDev		Pointer to the Net Device
 * \param[in] i4Control		Start or Stop TX Tone.
 * \param[out] None
 *
 * \retval 0				On success.
 * \retval -EFAULT			If kalIoctl return nonzero.
 */
/*----------------------------------------------------------------------------*/
int32_t MT_ATEDBDCTxTone(struct net_device *prNetDev,
			 int32_t i4Control)
{
	uint32_t u4BufLen = 0;
	int32_t i4Status;
	struct GLUE_INFO *prGlueInfo = NULL;
	struct PARAM_MTK_WIFI_TEST_STRUCT rRfATInfo;

	DBGLOG(RFTEST, INFO, "QA_ATE_HOOK MT_ATEDBDCTxTone\n");

	prGlueInfo = *((struct GLUE_INFO **) netdev_priv(prNetDev));

	if (i4Control) {
		rRfATInfo.u4FuncIndex = RF_AT_FUNCID_COMMAND;
		rRfATInfo.u4FuncData = RF_AT_COMMAND_SINGLE_TONE;
	} else {
		rRfATInfo.u4FuncIndex = RF_AT_FUNCID_COMMAND;
		rRfATInfo.u4FuncData = RF_AT_COMMAND_STOPTEST;
	}

	i4Status = kalIoctl(prGlueInfo,	/* prGlueInfo */
			    wlanoidRftestSetAutoTest,	/* pfnOidHandler */
			    &rRfATInfo,	/* pvInfoBuf */
			    sizeof(rRfATInfo),	/* u4InfoBufLen */
			    FALSE,	/* fgRead */
			    FALSE,	/* fgWaitResp */
			    TRUE,	/* fgCmd */
			    &u4BufLen);	/* pu4QryInfoLen */

	if (i4Status != WLAN_STATUS_SUCCESS)
		return -EFAULT;

	return i4Status;
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief  Hook API for Set TX Mac Header.
 *
 * \param[in] prNetDev		Pointer to the Net Device
 * \param[in] u4BandIdx       Band Index Number ready to set
 * \param[out] None
 *
 * \retval 0				On success.
 * \retval -EFAULT			If kalIoctl return nonzero.
 */
/*----------------------------------------------------------------------------*/
int32_t MT_ATESetMacHeader(struct net_device *prNetDev,
			   uint32_t u4FrameCtrl, uint32_t u4DurationID,
			   uint32_t u4SeqCtrl)
{
	uint32_t u4BufLen = 0;
	int32_t i4Status;
	struct GLUE_INFO *prGlueInfo = NULL;
	struct PARAM_MTK_WIFI_TEST_STRUCT rRfATInfo;

	DBGLOG(RFTEST, INFO, "QA_ATE_HOOK MT_ATESetMacHeader\n");

	prGlueInfo = *((struct GLUE_INFO **) netdev_priv(prNetDev));

	rRfATInfo.u4FuncIndex = RF_AT_FUNCID_SET_MAC_HEADER;
	rRfATInfo.u4FuncData = u4FrameCtrl || (u4DurationID << 16);

	i4Status = kalIoctl(prGlueInfo,	/* prGlueInfo */
			    wlanoidRftestSetAutoTest,	/* pfnOidHandler */
			    &rRfATInfo,	/* pvInfoBuf */
			    sizeof(rRfATInfo),	/* u4InfoBufLen */
			    FALSE,	/* fgRead */
			    FALSE,	/* fgWaitResp */
			    TRUE,	/* fgCmd */
			    &u4BufLen);	/* pu4QryInfoLen */

	if (i4Status != WLAN_STATUS_SUCCESS)
		return -EFAULT;

	rRfATInfo.u4FuncIndex = RF_AT_FUNCID_SET_SEQ_CTRL;
	rRfATInfo.u4FuncData = u4SeqCtrl;

	i4Status = kalIoctl(prGlueInfo,	/* prGlueInfo */
			    wlanoidRftestSetAutoTest,	/* pfnOidHandler */
			    &rRfATInfo,	/* pvInfoBuf */
			    sizeof(rRfATInfo),	/* u4InfoBufLen */
			    FALSE,	/* fgRead */
			    FALSE,	/* fgWaitResp */
			    TRUE,	/* fgCmd */
			    &u4BufLen);	/* pu4QryInfoLen */

	if (i4Status != WLAN_STATUS_SUCCESS)
		return -EFAULT;

	return i4Status;
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief  Hook API for IRR Set ADC. (RF_AT_FUNCID_SET_ADC)
 *
 * \param[in] prNetDev		Pointer to the Net Device
 *
 * \retval 0				On success.
 * \retval -EFAULT			If kalIoctl return nonzero.
 */
/*----------------------------------------------------------------------------*/
int32_t MT_ATE_IRRSetADC(struct net_device *prNetDev,
			 uint32_t u4WFIdx,
			 uint32_t u4ChFreq,
			 uint32_t u4BW, uint32_t u4Sx, uint32_t u4Band,
			 uint32_t u4RunType, uint32_t u4FType)
{
	uint32_t u4BufLen = 0, i = 0;
	int32_t i4Status;
	struct GLUE_INFO *prGlueInfo = NULL;
	struct PARAM_MTK_WIFI_TEST_STRUCT rRfATInfo;
	uint32_t au4Param[7];

	prGlueInfo = *((struct GLUE_INFO **) netdev_priv(prNetDev));

	DBGLOG(RFTEST, INFO, "QA_ATE_HOOK MT_ATE_IRRSetADC\n");

	if (u4BW == 3 || u4BW == 4 || u4BW > 5)
		return -EINVAL;

	if (u4BW == 5)		/* For BW160, UI will pass "5" */
		u4BW = 3;

	au4Param[0] = u4ChFreq;
	au4Param[1] = u4WFIdx;
	au4Param[2] = u4BW;
	au4Param[3] = u4Sx;
	au4Param[4] = u4Band;
	au4Param[5] = u4RunType;
	au4Param[6] = u4FType;

	for (i = 0; i < 8; i++) {
		rRfATInfo.u4FuncIndex = RF_AT_FUNCID_SET_ADC | (i << 16);
		if (i < 7)
			rRfATInfo.u4FuncData = au4Param[i];
		else
			rRfATInfo.u4FuncData = 0;

		i4Status = kalIoctl(prGlueInfo,	/* prGlueInfo */
				wlanoidRftestSetAutoTest, /* pfnOidHandler */
				&rRfATInfo,	/* pvInfoBuf */
				sizeof(rRfATInfo),	/* u4InfoBufLen */
				FALSE,	/* fgRead */
				FALSE,	/* fgWaitResp */
				TRUE,	/* fgCmd */
				&u4BufLen);	/* pu4QryInfoLen */

		if (i4Status != WLAN_STATUS_SUCCESS)
			return -EFAULT;
	}

	return i4Status;
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief  Hook API for IRR Set RX Gain. (RF_AT_FUNCID_SET_RX_GAIN)
 *
 * \param[in] prNetDev		Pointer to the Net Device
 *
 * \retval 0				On success.
 * \retval -EFAULT			If kalIoctl return nonzero.
 */
/*----------------------------------------------------------------------------*/
int32_t MT_ATE_IRRSetRxGain(struct net_device *prNetDev,
			    uint32_t u4PgaLpfg, uint32_t u4Lna, uint32_t u4Band,
			    uint32_t u4WF_inx, uint32_t u4Rfdgc)
{
	uint32_t u4BufLen = 0, i = 0;
	int32_t i4Status;
	struct GLUE_INFO *prGlueInfo = NULL;
	struct PARAM_MTK_WIFI_TEST_STRUCT rRfATInfo;
	uint32_t au4Param[5];

	prGlueInfo = *((struct GLUE_INFO **) netdev_priv(prNetDev));

	DBGLOG(RFTEST, INFO, "QA_ATE_HOOK MT_ATE_IRRSetRxGain\n");

	au4Param[0] = u4PgaLpfg;
	au4Param[1] = u4Lna;
	au4Param[2] = u4Band;
	au4Param[3] = u4WF_inx;
	au4Param[4] = u4Rfdgc;

	for (i = 0; i < 6; i++) {
		rRfATInfo.u4FuncIndex = RF_AT_FUNCID_SET_RX_GAIN |
					(i << 16);
		if (i < 5)
			rRfATInfo.u4FuncData = au4Param[i];
		else
			rRfATInfo.u4FuncData = 0;

		i4Status = kalIoctl(prGlueInfo,	/* prGlueInfo */
				wlanoidRftestSetAutoTest, /* pfnOidHandler */
				&rRfATInfo,	/* pvInfoBuf */
				sizeof(rRfATInfo),	/* u4InfoBufLen */
				FALSE,	/* fgRead */
				FALSE,	/* fgWaitResp */
				TRUE,	/* fgCmd */
				&u4BufLen);	/* pu4QryInfoLen */

		if (i4Status != WLAN_STATUS_SUCCESS)
			return -EFAULT;
	}

	return i4Status;
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief  Hook API for IRR Set TTG. (RF_AT_FUNCID_SET_TTG)
 *
 * \param[in] prNetDev		Pointer to the Net Device
 *
 * \retval 0				On success.
 * \retval -EFAULT			If kalIoctl return nonzero.
 */
/*----------------------------------------------------------------------------*/
int32_t MT_ATE_IRRSetTTG(struct net_device *prNetDev,
			 uint32_t u4TTGPwrIdx, uint32_t u4ChFreq,
			 uint32_t u4FIToneFreq, uint32_t u4Band)
{
	uint32_t u4BufLen = 0, i = 0;
	int32_t i4Status;
	struct GLUE_INFO *prGlueInfo = NULL;
	struct PARAM_MTK_WIFI_TEST_STRUCT rRfATInfo;
	uint32_t au4Param[4];

	prGlueInfo = *((struct GLUE_INFO **) netdev_priv(prNetDev));

	DBGLOG(RFTEST, INFO, "QA_ATE_HOOK MT_ATE_IRRSetTTG\n");

	au4Param[0] = u4ChFreq;
	au4Param[1] = u4FIToneFreq;
	au4Param[2] = u4TTGPwrIdx;
	au4Param[3] = u4Band;

	for (i = 0; i < 5; i++) {
		rRfATInfo.u4FuncIndex = RF_AT_FUNCID_SET_TTG | (i << 16);
		if (i < 4)
			rRfATInfo.u4FuncData = au4Param[i];
		else
			rRfATInfo.u4FuncData = 0;

		i4Status = kalIoctl(prGlueInfo,	/* prGlueInfo */
			    wlanoidRftestSetAutoTest,	/* pfnOidHandler */
			    &rRfATInfo,	/* pvInfoBuf */
			    sizeof(rRfATInfo),	/* u4InfoBufLen */
			    FALSE,	/* fgRead */
			    FALSE,	/* fgWaitResp */
			    TRUE,	/* fgCmd */
			    &u4BufLen);	/* pu4QryInfoLen */

		if (i4Status != WLAN_STATUS_SUCCESS)
			return -EFAULT;
	}

	return i4Status;
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief  Hook API for IRR Set TTG On/Off. (RF_AT_FUNCID_TTG_ON_OFF)
 *
 * \param[in] prNetDev		Pointer to the Net Device
 *
 * \retval 0				On success.
 * \retval -EFAULT			If kalIoctl return nonzero.
 */
/*----------------------------------------------------------------------------*/
int32_t MT_ATE_IRRSetTrunOnTTG(struct net_device *prNetDev,
			uint32_t u4TTGOnOff, uint32_t u4Band, uint32_t u4WF_inx)
{
	uint32_t u4BufLen = 0, i = 0;
	int32_t i4Status;
	struct GLUE_INFO *prGlueInfo = NULL;
	struct PARAM_MTK_WIFI_TEST_STRUCT rRfATInfo;
	uint32_t au4Param[3];

	prGlueInfo = *((struct GLUE_INFO **) netdev_priv(prNetDev));

	DBGLOG(RFTEST, INFO,
	       "QA_ATE_HOOK MT_ATE_IRRSetTrunOnTTG\n");

	au4Param[0] = u4TTGOnOff;
	au4Param[1] = u4Band;
	au4Param[2] = u4WF_inx;

	for (i = 0; i < 4; i++) {
		rRfATInfo.u4FuncIndex = RF_AT_FUNCID_TTG_ON_OFF | (i << 16);
		if (i < 3)
			rRfATInfo.u4FuncData = au4Param[i];
		else
			rRfATInfo.u4FuncData = 0;

		i4Status = kalIoctl(prGlueInfo,	/* prGlueInfo */
			    wlanoidRftestSetAutoTest,	/* pfnOidHandler */
			    &rRfATInfo,	/* pvInfoBuf */
			    sizeof(rRfATInfo),	/* u4InfoBufLen */
			    FALSE,	/* fgRead */
			    FALSE,	/* fgWaitResp */
			    TRUE,	/* fgCmd */
			    &u4BufLen);	/* pu4QryInfoLen */

		if (i4Status != WLAN_STATUS_SUCCESS)
			return -EFAULT;
	}

	return i4Status;
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief  Hook API for IRR Set TTG On/Off.
 *
 * \param[in] prNetDev		Pointer to the Net Device
 *
 * \retval 0				On success.
 * \retval -EFAULT			If kalIoctl return nonzero.
 */
/*----------------------------------------------------------------------------*/
int32_t MT_ATE_TMRSetting(struct net_device *prNetDev, uint32_t u4Setting,
		uint32_t u4Version, uint32_t u4MPThres, uint32_t u4MPIter)
{
	uint32_t u4BufLen = 0;
	int32_t i4Status;
	struct GLUE_INFO *prGlueInfo = NULL;
	struct PARAM_MTK_WIFI_TEST_STRUCT rRfATInfo;

	prGlueInfo = *((struct GLUE_INFO **) netdev_priv(prNetDev));

	DBGLOG(RFTEST, INFO, "QA_ATE_HOOK MT_ATE_TMRSetting\n");

	rRfATInfo.u4FuncIndex = RF_AT_FUNCID_SET_TMR_ROLE;
	rRfATInfo.u4FuncData = u4Setting;

	i4Status = kalIoctl(prGlueInfo,	/* prGlueInfo */
			    wlanoidRftestSetAutoTest,	/* pfnOidHandler */
			    &rRfATInfo,	/* pvInfoBuf */
			    sizeof(rRfATInfo),	/* u4InfoBufLen */
			    FALSE,	/* fgRead */
			    FALSE,	/* fgWaitResp */
			    TRUE,	/* fgCmd */
			    &u4BufLen);	/* pu4QryInfoLen */

	if (i4Status != WLAN_STATUS_SUCCESS)
		return -EFAULT;

	rRfATInfo.u4FuncIndex = RF_AT_FUNCID_SET_TMR_MODULE;
	rRfATInfo.u4FuncData = u4Version;

	i4Status = kalIoctl(prGlueInfo,	/* prGlueInfo */
			    wlanoidRftestSetAutoTest,	/* pfnOidHandler */
			    &rRfATInfo,	/* pvInfoBuf */
			    sizeof(rRfATInfo),	/* u4InfoBufLen */
			    FALSE,	/* fgRead */
			    FALSE,	/* fgWaitResp */
			    TRUE,	/* fgCmd */
			    &u4BufLen);	/* pu4QryInfoLen */

	if (i4Status != WLAN_STATUS_SUCCESS)
		return -EFAULT;

	rRfATInfo.u4FuncIndex = RF_AT_FUNCID_SET_TMR_DBM;
	rRfATInfo.u4FuncData = u4MPThres;

	i4Status = kalIoctl(prGlueInfo,	/* prGlueInfo */
			    wlanoidRftestSetAutoTest,	/* pfnOidHandler */
			    &rRfATInfo,	/* pvInfoBuf */
			    sizeof(rRfATInfo),	/* u4InfoBufLen */
			    FALSE,	/* fgRead */
			    FALSE,	/* fgWaitResp */
			    TRUE,	/* fgCmd */
			    &u4BufLen);	/* pu4QryInfoLen */

	if (i4Status != WLAN_STATUS_SUCCESS)
		return -EFAULT;

	rRfATInfo.u4FuncIndex = RF_AT_FUNCID_SET_TMR_ITER;
	rRfATInfo.u4FuncData = u4MPIter;

	i4Status = kalIoctl(prGlueInfo,	/* prGlueInfo */
			    wlanoidRftestSetAutoTest,	/* pfnOidHandler */
			    &rRfATInfo,	/* pvInfoBuf */
			    sizeof(rRfATInfo),	/* u4InfoBufLen */
			    FALSE,	/* fgRead */
			    FALSE,	/* fgWaitResp */
			    TRUE,	/* fgCmd */
			    &u4BufLen);	/* pu4QryInfoLen */

	if (i4Status != WLAN_STATUS_SUCCESS)
		return -EFAULT;

	return i4Status;
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief  Hook API for MPS Setting. (Set Seq Data)
 *
 * \param[in] prNetDev		Pointer to the Net Device
 *
 * \retval 0				On success.
 * \retval -EFAULT			If kalIoctl return nonzero.
 */
/*----------------------------------------------------------------------------*/
int32_t MT_ATEMPSSetSeqData(struct net_device *prNetDev,
		    uint32_t u4TestNum, uint32_t *pu4Phy, uint32_t u4Band)
{
	uint32_t u4BufLen = 0, i;
	int32_t i4Status;
	struct GLUE_INFO *prGlueInfo = NULL;
	struct PARAM_MTK_WIFI_TEST_STRUCT rRfATInfo;

	DBGLOG(RFTEST, INFO, "QA_ATE_HOOK MT_ATEMPSSetSeqData\n");

	prGlueInfo = *((struct GLUE_INFO **) netdev_priv(prNetDev));

	rRfATInfo.u4FuncIndex = RF_AT_FUNCID_SET_MPS_SIZE;
	rRfATInfo.u4FuncData = u4TestNum;

	i4Status = kalIoctl(prGlueInfo,	/* prGlueInfo */
			    wlanoidRftestSetAutoTest,	/* pfnOidHandler */
			    &rRfATInfo,	/* pvInfoBuf */
			    sizeof(rRfATInfo),	/* u4InfoBufLen */
			    FALSE,	/* fgRead */
			    FALSE,	/* fgWaitResp */
			    TRUE,	/* fgCmd */
			    &u4BufLen);	/* pu4QryInfoLen */

	if (i4Status != WLAN_STATUS_SUCCESS)
		return -EFAULT;

	for (i = 0 ; i < u4TestNum ; i++) {
		rRfATInfo.u4FuncIndex = RF_AT_FUNCID_SET_MPS_SEQ_DATA |
					(i << 16);
		rRfATInfo.u4FuncData = pu4Phy[i];

		i4Status = kalIoctl(prGlueInfo,	/* prGlueInfo */
			    wlanoidRftestSetAutoTest,	/* pfnOidHandler */
			    &rRfATInfo,	/* pvInfoBuf */
			    sizeof(rRfATInfo),	/* u4InfoBufLen */
			    FALSE,	/* fgRead */
			    FALSE,	/* fgWaitResp */
			    TRUE,	/* fgCmd */
			    &u4BufLen);	/* pu4QryInfoLen */

		if (i4Status != WLAN_STATUS_SUCCESS)
			return -EFAULT;
	}

	return i4Status;
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief  Hook API for MPS Setting. (Set Payload Length)
 *
 * \param[in] prNetDev		Pointer to the Net Device
 *
 * \retval 0				On success.
 * \retval -EFAULT			If kalIoctl return nonzero.
 */
/*----------------------------------------------------------------------------*/
int32_t MT_ATEMPSSetPayloadLength(struct net_device *prNetDev,
		  uint32_t u4TestNum, uint32_t *pu4Length, uint32_t u4Band)
{
	uint32_t u4BufLen = 0, i;
	int32_t i4Status = 0;
	struct GLUE_INFO *prGlueInfo = NULL;
	struct PARAM_MTK_WIFI_TEST_STRUCT rRfATInfo;

	DBGLOG(RFTEST, INFO,
	       "QA_ATE_HOOK MT_ATEMPSSetPayloadLength\n");

	prGlueInfo = *((struct GLUE_INFO **) netdev_priv(prNetDev));

	for (i = 0 ; i < u4TestNum ; i++) {
		rRfATInfo.u4FuncIndex = RF_AT_FUNCID_SET_MPS_PAYLOAD_LEN |
					(i << 16);
		rRfATInfo.u4FuncData = pu4Length[i];

		i4Status = kalIoctl(prGlueInfo,	/* prGlueInfo */
			    wlanoidRftestSetAutoTest,	/* pfnOidHandler */
			    &rRfATInfo,	/* pvInfoBuf */
			    sizeof(rRfATInfo),	/* u4InfoBufLen */
			    FALSE,	/* fgRead */
			    FALSE,	/* fgWaitResp */
			    TRUE,	/* fgCmd */
			    &u4BufLen);	/* pu4QryInfoLen */

		if (i4Status != WLAN_STATUS_SUCCESS)
			return -EFAULT;
	}

	return i4Status;
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief  Hook API for MPS Setting. (Set Packet Count)
 *
 * \param[in] prNetDev		Pointer to the Net Device
 *
 * \retval 0				On success.
 * \retval -EFAULT			If kalIoctl return nonzero.
 */
/*----------------------------------------------------------------------------*/
int32_t MT_ATEMPSSetPacketCount(struct net_device *prNetDev,
		uint32_t u4TestNum, uint32_t *pu4PktCnt, uint32_t u4Band)
{
	uint32_t u4BufLen = 0, i;
	int32_t i4Status = 0;
	struct GLUE_INFO *prGlueInfo = NULL;
	struct PARAM_MTK_WIFI_TEST_STRUCT rRfATInfo;

	DBGLOG(RFTEST, INFO,
	       "QA_ATE_HOOK MT_ATEMPSSetPacketCount\n");

	prGlueInfo = *((struct GLUE_INFO **) netdev_priv(prNetDev));

	for (i = 0 ; i < u4TestNum ; i++) {
		rRfATInfo.u4FuncIndex = RF_AT_FUNCID_SET_MPS_PKT_CNT |
					(i << 16);
		rRfATInfo.u4FuncData = pu4PktCnt[i];

		i4Status = kalIoctl(prGlueInfo,	/* prGlueInfo */
			    wlanoidRftestSetAutoTest,	/* pfnOidHandler */
			    &rRfATInfo,	/* pvInfoBuf */
			    sizeof(rRfATInfo),	/* u4InfoBufLen */
			    FALSE,	/* fgRead */
			    FALSE,	/* fgWaitResp */
			    TRUE,	/* fgCmd */
			    &u4BufLen);	/* pu4QryInfoLen */

		if (i4Status != WLAN_STATUS_SUCCESS)
			return -EFAULT;
	}

	return i4Status;
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief  Hook API for MPS Setting. (Set Power Gain)
 *
 * \param[in] prNetDev		Pointer to the Net Device
 *
 * \retval 0				On success.
 * \retval -EFAULT			If kalIoctl return nonzero.
 */
/*----------------------------------------------------------------------------*/
int32_t MT_ATEMPSSetPowerGain(struct net_device *prNetDev,
		      uint32_t u4TestNum, uint32_t *pu4PwrGain, uint32_t u4Band)
{
	uint32_t u4BufLen = 0, i;
	int32_t i4Status = 0;
	struct GLUE_INFO *prGlueInfo = NULL;
	struct PARAM_MTK_WIFI_TEST_STRUCT rRfATInfo;

	DBGLOG(RFTEST, INFO, "QA_ATE_HOOK MT_ATEMPSSetPowerGain\n");

	prGlueInfo = *((struct GLUE_INFO **) netdev_priv(prNetDev));

	for (i = 0 ; i < u4TestNum ; i++) {
		rRfATInfo.u4FuncIndex = RF_AT_FUNCID_SET_MPS_PWR_GAIN |
					(i << 16);
		rRfATInfo.u4FuncData = pu4PwrGain[i];

		i4Status = kalIoctl(prGlueInfo,	/* prGlueInfo */
			    wlanoidRftestSetAutoTest,	/* pfnOidHandler */
			    &rRfATInfo,	/* pvInfoBuf */
			    sizeof(rRfATInfo),	/* u4InfoBufLen */
			    FALSE,	/* fgRead */
			    FALSE,	/* fgWaitResp */
			    TRUE,	/* fgCmd */
			    &u4BufLen);	/* pu4QryInfoLen */

		if (i4Status != WLAN_STATUS_SUCCESS)
			return -EFAULT;
	}

	return i4Status;
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief  Hook API for MPS Setting. (Set NSS)
 *
 * \param[in] prNetDev		Pointer to the Net Device
 *
 * \retval 0				On success.
 * \retval -EFAULT			If kalIoctl return nonzero.
 */
/*----------------------------------------------------------------------------*/
int32_t MT_ATEMPSSetNss(struct net_device *prNetDev,
			uint32_t u4TestNum, uint32_t *pu4Nss, uint32_t u4Band)
{
	uint32_t u4BufLen = 0, i;
	int32_t i4Status = 0;
	struct GLUE_INFO *prGlueInfo = NULL;
	struct PARAM_MTK_WIFI_TEST_STRUCT rRfATInfo;

	DBGLOG(RFTEST, INFO, "QA_ATE_HOOK MT_ATEMPSSetNss\n");

	prGlueInfo = *((struct GLUE_INFO **) netdev_priv(prNetDev));

	for (i = 0 ; i < u4TestNum ; i++) {
		rRfATInfo.u4FuncIndex = RF_AT_FUNCID_SET_MPS_NSS |
					(i << 16);
		rRfATInfo.u4FuncData = pu4Nss[i];

		i4Status = kalIoctl(prGlueInfo,	/* prGlueInfo */
			    wlanoidRftestSetAutoTest,	/* pfnOidHandler */
			    &rRfATInfo,	/* pvInfoBuf */
			    sizeof(rRfATInfo),	/* u4InfoBufLen */
			    FALSE,	/* fgRead */
			    FALSE,	/* fgWaitResp */
			    TRUE,	/* fgCmd */
			    &u4BufLen);	/* pu4QryInfoLen */

		if (i4Status != WLAN_STATUS_SUCCESS)
			return -EFAULT;
	}

	return i4Status;
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief  Hook API for MPS Setting. (Set NSS)
 *
 * \param[in] prNetDev		Pointer to the Net Device
 *
 * \retval 0				On success.
 * \retval -EFAULT			If kalIoctl return nonzero.
 */
/*----------------------------------------------------------------------------*/
int32_t MT_ATEMPSSetPerpacketBW(struct net_device *prNetDev,
		uint32_t u4TestNum, uint32_t *pu4PerPktBW, uint32_t u4Band)
{
	uint32_t u4BufLen = 0, i;
	int32_t i4Status = 0;
	struct GLUE_INFO *prGlueInfo = NULL;
	struct PARAM_MTK_WIFI_TEST_STRUCT rRfATInfo;

	DBGLOG(RFTEST, INFO,
	       "QA_ATE_HOOK MT_ATEMPSSetPerpacketBW\n");

	prGlueInfo = *((struct GLUE_INFO **) netdev_priv(prNetDev));

	for (i = 0 ; i < u4TestNum ; i++) {
		rRfATInfo.u4FuncIndex = RF_AT_FUNCID_SET_MPS_PACKAGE_BW |
					(i << 16);
		rRfATInfo.u4FuncData = pu4PerPktBW[i];

		i4Status = kalIoctl(prGlueInfo,	/* prGlueInfo */
			    wlanoidRftestSetAutoTest, /* pfnOidHandler */
			    &rRfATInfo,	/* pvInfoBuf */
			    sizeof(rRfATInfo),	/* u4InfoBufLen */
			    FALSE,	/* fgRead */
			    FALSE,	/* fgWaitResp */
			    TRUE,	/* fgCmd */
			    &u4BufLen);	/* pu4QryInfoLen */

		if (i4Status != WLAN_STATUS_SUCCESS)
			return -EFAULT;
	}

	return i4Status;
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief  Hook API for Start RDD.
 *
 * \param[in] prNetDev		Pointer to the Net Device
 *
 * \retval 0				On success.
 * \retval -EFAULT			If kalIoctl return nonzero.
 */
/*----------------------------------------------------------------------------*/
int32_t MT_ATERDDStart(struct net_device *prNetDev,
		       uint8_t *prInBuf)
{
	uint32_t u4BufLen = 0;
	int32_t i4Status = 0;
	struct GLUE_INFO *prGlueInfo = NULL;
	struct PARAM_MTK_WIFI_TEST_STRUCT rRfATInfo;

	prGlueInfo = *((struct GLUE_INFO **) netdev_priv(prNetDev));

	DBGLOG(RFTEST, INFO, "QA_ATE_HOOK SetATE = %s\n", prInBuf);

	rRfATInfo.u4FuncIndex = RF_AT_FUNCID_COMMAND;
	rRfATInfo.u4FuncData = RF_AT_COMMAND_RDD;

	i4Status = kalIoctl(prGlueInfo, /* prGlueInfo */
			    wlanoidRftestSetAutoTest,   /* pfnOidHandler */
			    &rRfATInfo, /* pvInfoBuf */
			    sizeof(rRfATInfo),  /* u4InfoBufLen */
			    FALSE,  /* fgRead */
			    FALSE,  /* fgWaitResp */
			    TRUE,   /* fgCmd */
			    &u4BufLen); /* pu4QryInfoLen */

	if (i4Status != WLAN_STATUS_SUCCESS)
		return -EFAULT;

	return i4Status;
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief  Hook API for Stop RDD.
 *
 * \param[in] prNetDev		Pointer to the Net Device
 *
 * \retval 0				On success.
 * \retval -EFAULT			If kalIoctl return nonzero.
 */
/*----------------------------------------------------------------------------*/
int32_t MT_ATERDDStop(struct net_device *prNetDev,
		      uint8_t *prInBuf)
{
	uint32_t u4BufLen = 0;
	int32_t i4Status = 0;
	struct GLUE_INFO *prGlueInfo = NULL;
	struct PARAM_MTK_WIFI_TEST_STRUCT rRfATInfo;

	prGlueInfo = *((struct GLUE_INFO **) netdev_priv(prNetDev));

	DBGLOG(RFTEST, INFO, "QA_ATE_HOOK SetATE = %s\n", prInBuf);

	rRfATInfo.u4FuncIndex = RF_AT_FUNCID_COMMAND;
	rRfATInfo.u4FuncData = RF_AT_COMMAND_RDD_OFF;

	i4Status = kalIoctl(prGlueInfo, /* prGlueInfo */
			    wlanoidRftestSetAutoTest,   /* pfnOidHandler */
			    &rRfATInfo, /* pvInfoBuf */
			    sizeof(rRfATInfo),  /* u4InfoBufLen */
			    FALSE,  /* fgRead */
			    FALSE,  /* fgWaitResp */
			    TRUE,   /* fgCmd */
			    &u4BufLen); /* pu4QryInfoLen */

	if (i4Status != WLAN_STATUS_SUCCESS)
		return -EFAULT;

	return i4Status;

}


/*----------------------------------------------------------------------------*/
/*!
 * \brief  Hook API for Write Efuse.
 *
 * \param[in] prNetDev		Pointer to the Net Device
 * \param[in] u2Offset		Efuse offset
 * \param[in] u2Content		Efuse content
 * \param[out] None
 *
 * \retval 0				On success.
 * \retval -EFAULT			If kalIoctl return nonzero.
 * \retval -EINVAL			If invalid argument.
 */
/*----------------------------------------------------------------------------*/
int32_t MT_ATEWriteEfuse(struct net_device *prNetDev,
			 uint16_t u2Offset, uint16_t u2Content)
{
	uint32_t u4BufLen = 0;
	struct PARAM_CUSTOM_ACCESS_EFUSE rAccessEfuseInfoRead,
		       rAccessEfuseInfoWrite;
	struct GLUE_INFO *prGlueInfo = NULL;
	uint32_t i4Status = WLAN_STATUS_SUCCESS;
	uint8_t  u4Index = 0, u4Loop = 0;

	prGlueInfo = *((struct GLUE_INFO **) netdev_priv(prNetDev));

	if (!prGlueInfo) {
		log_dbg(RFTEST, ERROR, "prGlueInfo is NULL\n");
		return -EFAULT;
	}

	if (prGlueInfo->prAdapter == NULL) {
		DBGLOG(RFTEST, ERROR, "prAdapter is NULL\n");
		return -EFAULT;
	}

	kalMemSet(&rAccessEfuseInfoRead, 0,
		  sizeof(struct PARAM_CUSTOM_ACCESS_EFUSE));
	kalMemSet(&rAccessEfuseInfoWrite, 0,
		  sizeof(struct PARAM_CUSTOM_ACCESS_EFUSE));

	if (prGlueInfo &&
	    prGlueInfo->prAdapter &&
	    prGlueInfo->prAdapter->chip_info &&
	    !prGlueInfo->prAdapter->chip_info->is_support_efuse) {
		log_dbg(RFTEST, WARN, "Efuse not support\n");
		return -EFAULT;
	}

	/* Read */
	DBGLOG(INIT, INFO, "QA_AGENT HQA_WriteBulkEEPROM  Read\n");
	kalMemSet(&rAccessEfuseInfoRead, 0,
		  sizeof(struct PARAM_CUSTOM_ACCESS_EFUSE));
	rAccessEfuseInfoRead.u4Address = (u2Offset /
					  EFUSE_BLOCK_SIZE) * EFUSE_BLOCK_SIZE;
	i4Status = kalIoctl(prGlueInfo,
			    wlanoidQueryProcessAccessEfuseRead,
			    &rAccessEfuseInfoRead,
			    sizeof(struct PARAM_CUSTOM_ACCESS_EFUSE),
			    TRUE, TRUE, TRUE, &u4BufLen);
	if (i4Status != WLAN_STATUS_SUCCESS)
		return -EFAULT;

	/* Write */
	kalMemSet(&rAccessEfuseInfoWrite, 0,
		  sizeof(struct PARAM_CUSTOM_ACCESS_EFUSE));
	u4Index = u2Offset % EFUSE_BLOCK_SIZE;

	if (u4Index >= EFUSE_BLOCK_SIZE - 1) {
		DBGLOG(INIT, INFO, "u4Index [%d] overrun\n", u4Index);
		return -EFAULT;
	}

	prGlueInfo->prAdapter->aucEepromVaule[u4Index] = u2Content;
	prGlueInfo->prAdapter->aucEepromVaule[u4Index + 1] =
		u2Content >> 8 & 0xff;

	kalMemCopy(rAccessEfuseInfoWrite.aucData,
		   prGlueInfo->prAdapter->aucEepromVaule, 16);

	for (u4Loop = 0; u4Loop < (EFUSE_BLOCK_SIZE); u4Loop++) {
		DBGLOG(INIT, INFO,
		       "QA_AGENT aucEepromVaule u4Loop=%d  u4Value=%x\n",
		       u4Loop, prGlueInfo->prAdapter->aucEepromVaule[u4Loop]);

		DBGLOG(INIT, INFO,
		       "QA_AGENT rAccessEfuseInfoWrite.aucData u4Loop=%d  u4Value=%x\n",
		       u4Loop, rAccessEfuseInfoWrite.aucData[u4Loop]);
	}

	rAccessEfuseInfoWrite.u4Address = (u2Offset /
					   EFUSE_BLOCK_SIZE) * EFUSE_BLOCK_SIZE;

	i4Status = kalIoctl(prGlueInfo,
			    wlanoidQueryProcessAccessEfuseWrite,
			    &rAccessEfuseInfoWrite,
			    sizeof(struct PARAM_CUSTOM_ACCESS_EFUSE),
			    FALSE, TRUE, TRUE, &u4BufLen);

	if (i4Status != WLAN_STATUS_SUCCESS)
		return -EFAULT;

	return i4Status;
}


/*----------------------------------------------------------------------------*/
/*!
 * \brief  Hook API for Set Tx Target Power.
 *
 * \param[in] prNetDev		 Pointer to the Net Device
 * \param[in] u2TxTargetPower TxTarget Power
 * \param[out] None
 *
 * \retval 0				On success.
 * \retval -EFAULT			If kalIoctl return nonzero.
 * \retval -EINVAL			If invalid argument.
 */
/*----------------------------------------------------------------------------*/
int32_t MT_ATESetTxTargetPower(struct net_device *prNetDev,
			       uint8_t ucTxTargetPower)
{
	uint32_t u4BufLen = 0;
	struct PARAM_CUSTOM_SET_TX_TARGET_POWER rSetTxTargetPwr;
	struct GLUE_INFO *prGlueInfo = NULL;
	uint32_t i4Status = WLAN_STATUS_SUCCESS;

	prGlueInfo = *((struct GLUE_INFO **) netdev_priv(prNetDev));
	kalMemSet(&rSetTxTargetPwr, 0,
		  sizeof(struct PARAM_CUSTOM_SET_TX_TARGET_POWER));


	/* Set Target Power Base */
	DBGLOG(INIT, INFO, "QA_AGENT Set Tx Target Power= %x dbm\n",
	       ucTxTargetPower);
	rSetTxTargetPwr.ucTxTargetPwr = ucTxTargetPower;

	i4Status = kalIoctl(prGlueInfo,
			    wlanoidQuerySetTxTargetPower,
			    &rSetTxTargetPwr,
			    sizeof(struct PARAM_CUSTOM_SET_TX_TARGET_POWER),
			    FALSE, FALSE, TRUE, &u4BufLen);

	if (i4Status != WLAN_STATUS_SUCCESS)
		return -EFAULT;

	return i4Status;
}

#if CFG_SUPPORT_ANT_SWAP
/*----------------------------------------------------------------------------*/
/*!
 * \brief  Hook API for Set Antenna swap
 *
 * \param[in] prNetDev		 Pointer to the Net Device
 * \param[in] u4Ant		 The antenna to choose (0 for main, 1 for aux)
 * \param[out] None
 *
 * \retval 0				On success.
 * \retval -EFAULT			If kalIoctl return nonzero.
 * \retval -EINVAL			If invalid argument.
 */
/*----------------------------------------------------------------------------*/
int32_t MT_ATESetAntSwap(struct net_device *prNetDev,
					     uint32_t u4Ant)
{
	struct GLUE_INFO *prGlueInfo = NULL;
	struct PARAM_MTK_WIFI_TEST_STRUCT rRfATInfo;
	uint32_t u4BufLen = 0;
	int32_t i4Status = 0;

	prGlueInfo = *((struct GLUE_INFO **) netdev_priv(prNetDev));
	if (!prGlueInfo) {
		DBGLOG(RFTEST, ERROR, "prGlueInfo is NULL\n");
		return WLAN_STATUS_ADAPTER_NOT_READY;
	}

	DBGLOG(RFTEST, INFO,
	       "QA_AGENT MT_ATESetAntSwap u4Ant : %d\n", u4Ant);

	rRfATInfo.u4FuncIndex = RF_AT_FUNCID_SET_ANT_SWP;
	rRfATInfo.u4FuncData = (uint32_t) u4Ant;

	i4Status = kalIoctl(prGlueInfo,	/* prGlueInfo */
			 wlanoidRftestSetAutoTest,	/* pfnOidHandler */
			 &rRfATInfo,	/* pvInfoBuf */
			 sizeof(rRfATInfo),	/* u4InfoBufLen */
			 FALSE,	/* fgRead */
			 FALSE,	/* fgWaitResp */
			 TRUE,	/* fgCmd */
			 &u4BufLen);	/* pu4QryInfoLen */

	if (i4Status != WLAN_STATUS_SUCCESS)
		return -EFAULT;

	return i4Status;

}
#endif /* CFG_SUPPORT_ANT_SWAP */

#if (CFG_SUPPORT_DFS_MASTER == 1)
/*----------------------------------------------------------------------------*/
/*!
 * \brief  Hook API for Set Rdd Report.
 *
 * \param[in] prNetDev		 Pointer to the Net Device
 * \param[in] ucDbdcIdx         Dbdc Index
 * \param[out] None
 *
 * \retval 0				On success.
 * \retval -EFAULT			If kalIoctl return nonzero.
 * \retval -EINVAL			If invalid argument.
 */
/*----------------------------------------------------------------------------*/
int32_t MT_ATESetRddReport(struct net_device *prNetDev,
			   uint8_t ucDbdcIdx)
{
	uint32_t u4BufLen = 0;
	struct PARAM_CUSTOM_SET_RDD_REPORT rSetRddReport;
	struct GLUE_INFO *prGlueInfo = NULL;
	uint32_t i4Status = WLAN_STATUS_SUCCESS;

	prGlueInfo = *((struct GLUE_INFO **) netdev_priv(prNetDev));
	kalMemSet(&rSetRddReport, 0,
		  sizeof(struct PARAM_CUSTOM_SET_RDD_REPORT));

	/* Set Rdd Report */
	DBGLOG(INIT, INFO, "QA_AGENT Set RDD Report - Band: %d\n",
	       ucDbdcIdx);
	rSetRddReport.ucDbdcIdx = ucDbdcIdx;

	i4Status = kalIoctl(prGlueInfo,
			    wlanoidQuerySetRddReport,
			    &rSetRddReport,
			    sizeof(struct PARAM_CUSTOM_SET_RDD_REPORT),
			    FALSE, FALSE, TRUE, &u4BufLen);

	if (i4Status != WLAN_STATUS_SUCCESS)
		return -EFAULT;

	return i4Status;
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief  Hook API for Set Radar Detect Mode.
 *
 * \param[in] prNetDev				 Pointer to the Net Device
 * \param[in] ucRadarDetectMode         Radar Detect Mode
 * \param[out] None
 *
 * \retval 0				On success.
 * \retval -EFAULT			If kalIoctl return nonzero.
 * \retval -EINVAL			If invalid argument.
 */
/*----------------------------------------------------------------------------*/
int32_t MT_ATESetRadarDetectMode(struct net_device
				 *prNetDev, uint8_t ucRadarDetectMode)
{
	uint32_t u4BufLen = 0;
	struct PARAM_CUSTOM_SET_RADAR_DETECT_MODE
		rSetRadarDetectMode;
	struct GLUE_INFO *prGlueInfo = NULL;
	uint32_t i4Status = WLAN_STATUS_SUCCESS;

	prGlueInfo = *((struct GLUE_INFO **) netdev_priv(prNetDev));
	kalMemSet(&rSetRadarDetectMode, 0,
		  sizeof(struct PARAM_CUSTOM_SET_RADAR_DETECT_MODE));

	/* Set Rdd Report */
	DBGLOG(INIT, INFO, "QA_AGENT Set Radar Detect Mode: %d\n",
	       ucRadarDetectMode);
	rSetRadarDetectMode.ucRadarDetectMode = ucRadarDetectMode;

	i4Status = kalIoctl(prGlueInfo,
			    wlanoidQuerySetRadarDetectMode,
			    &rSetRadarDetectMode,
			    sizeof(struct PARAM_CUSTOM_SET_RADAR_DETECT_MODE),
			    FALSE, FALSE, TRUE, &u4BufLen);

	if (i4Status != WLAN_STATUS_SUCCESS)
		return -EFAULT;

	return i4Status;
}

#endif

#if CFG_SUPPORT_TX_BF
int32_t TxBfProfileTag_InValid(struct net_device *prNetDev,
			       union PFMU_PROFILE_TAG1 *prPfmuTag1,
			       uint8_t ucInValid)
{
	int32_t i4Status = 0;

	prPfmuTag1->rField.ucInvalidProf = ucInValid;

	return i4Status;
}

int32_t TxBfProfileTag_PfmuIdx(struct net_device *prNetDev,
			       union PFMU_PROFILE_TAG1 *prPfmuTag1,
			       uint8_t ucProfileIdx)
{
	int32_t i4Status = 0;

	prPfmuTag1->rField.ucProfileID = ucProfileIdx;

	return i4Status;
}

int32_t TxBfProfileTag_TxBfType(struct net_device *prNetDev,
				union PFMU_PROFILE_TAG1 *prPfmuTag1,
				uint8_t ucBFType)
{
	int32_t i4Status = 0;

	prPfmuTag1->rField.ucTxBf = ucBFType;

	return i4Status;
}

int32_t TxBfProfileTag_DBW(struct net_device *prNetDev,
			   union PFMU_PROFILE_TAG1 *prPfmuTag1, uint8_t ucBW)
{
	int32_t i4Status = 0;

	prPfmuTag1->rField.ucDBW = ucBW;

	return i4Status;
}

int32_t TxBfProfileTag_SuMu(struct net_device *prNetDev,
			    union PFMU_PROFILE_TAG1 *prPfmuTag1, uint8_t ucSuMu)
{
	int32_t i4Status = 0;

	prPfmuTag1->rField.ucSU_MU = ucSuMu;

	return i4Status;
}

int32_t TxBfProfileTag_Mem(struct net_device *prNetDev,
			   union PFMU_PROFILE_TAG1 *prPfmuTag1,
			   uint8_t *aucMemAddrColIdx, uint8_t *aucMemAddrRowIdx)
{
	int32_t i4Status = 0;

	prPfmuTag1->rField.ucMemAddr1ColIdx = aucMemAddrColIdx[0];
	prPfmuTag1->rField.ucMemAddr1RowIdx = aucMemAddrRowIdx[0];
	prPfmuTag1->rField.ucMemAddr2ColIdx = aucMemAddrColIdx[1];
	prPfmuTag1->rField.ucMemAddr2RowIdx = aucMemAddrRowIdx[1];
	prPfmuTag1->rField.ucMemAddr3ColIdx = aucMemAddrColIdx[2];
	prPfmuTag1->rField.ucMemAddr3RowIdx = aucMemAddrRowIdx[2];
	prPfmuTag1->rField.ucMemAddr4ColIdx = aucMemAddrColIdx[3];
	prPfmuTag1->rField.ucMemAddr4RowIdx = aucMemAddrRowIdx[3];

	return i4Status;
}

int32_t TxBfProfileTag_Matrix(struct net_device *prNetDev,
			      union PFMU_PROFILE_TAG1 *prPfmuTag1,
			      uint8_t ucNrow,
			      uint8_t ucNcol, uint8_t ucNgroup, uint8_t ucLM,
			      uint8_t ucCodeBook, uint8_t ucHtcExist)
{
	int32_t i4Status = 0;

	prPfmuTag1->rField.ucNrow = ucNrow;
	prPfmuTag1->rField.ucNcol = ucNcol;
	prPfmuTag1->rField.ucNgroup = ucNgroup;
	prPfmuTag1->rField.ucLM = ucLM;
	prPfmuTag1->rField.ucCodeBook = ucCodeBook;

	return i4Status;
}

int32_t TxBfProfileTag_SNR(struct net_device *prNetDev,
			   union PFMU_PROFILE_TAG1 *prPfmuTag1,
			   uint8_t ucSNR_STS0, uint8_t ucSNR_STS1,
			   uint8_t ucSNR_STS2, uint8_t ucSNR_STS3)
{
	int32_t i4Status = 0;

	prPfmuTag1->rField.ucSNR_STS0 = ucSNR_STS0;
	prPfmuTag1->rField.ucSNR_STS1 = ucSNR_STS1;
	prPfmuTag1->rField.ucSNR_STS2 = ucSNR_STS2;
	prPfmuTag1->rField.ucSNR_STS3 = ucSNR_STS3;

	return i4Status;
}

int32_t TxBfProfileTag_SmtAnt(struct net_device *prNetDev,
			      union PFMU_PROFILE_TAG2 *prPfmuTag2,
			      uint8_t ucSmartAnt)
{
	int32_t i4Status = 0;

	prPfmuTag2->rField.u2SmartAnt = ucSmartAnt;

	return i4Status;
}

int32_t TxBfProfileTag_SeIdx(struct net_device *prNetDev,
			     union PFMU_PROFILE_TAG2 *prPfmuTag2,
			     uint8_t ucSeIdx)
{
	int32_t i4Status = 0;

	prPfmuTag2->rField.ucSEIdx = ucSeIdx;

	return i4Status;
}

int32_t TxBfProfileTag_RmsdThd(struct net_device *prNetDev,
			       union PFMU_PROFILE_TAG2 *prPfmuTag2,
			       uint8_t ucRmsdThrd)
{
	int32_t i4Status = 0;

	prPfmuTag2->rField.ucRMSDThd = ucRmsdThrd;

	return i4Status;
}

int32_t TxBfProfileTag_McsThd(struct net_device *prNetDev,
			      union PFMU_PROFILE_TAG2 *prPfmuTag2,
			      uint8_t *pMCSThLSS, uint8_t *pMCSThSSS)
{
	int32_t i4Status = 0;

	/* connac 1.0 setting

	* prPfmuTag2->rField.ucMCSThL1SS = pMCSThLSS[0];
	* prPfmuTag2->rField.ucMCSThS1SS = pMCSThSSS[0];
	* prPfmuTag2->rField.ucMCSThL2SS = pMCSThLSS[1];
	* prPfmuTag2->rField.ucMCSThS2SS = pMCSThSSS[1];
	* prPfmuTag2->rField.ucMCSThL3SS = pMCSThLSS[2];
	* prPfmuTag2->rField.ucMCSThS3SS = pMCSThSSS[2];
	*/
	return i4Status;
}

int32_t TxBfProfileTag_TimeOut(struct net_device *prNetDev,
			union PFMU_PROFILE_TAG2 *prPfmuTag2, uint8_t ucTimeOut)
{
	int32_t i4Status = 0;

	prPfmuTag2->rField.uciBfTimeOut = ucTimeOut;

	return i4Status;
}

int32_t TxBfProfileTag_DesiredBW(struct net_device
				 *prNetDev, union PFMU_PROFILE_TAG2 *prPfmuTag2,
				 uint8_t ucDesiredBW)
{
	int32_t i4Status = 0;

	prPfmuTag2->rField.uciBfDBW = ucDesiredBW;

	return i4Status;
}

int32_t TxBfProfileTag_DesiredNc(struct net_device
				 *prNetDev, union PFMU_PROFILE_TAG2 *prPfmuTag2,
				 uint8_t ucDesiredNc)
{
	int32_t i4Status = 0;

	prPfmuTag2->rField.uciBfNcol = ucDesiredNc;

	return i4Status;
}

int32_t TxBfProfileTag_DesiredNr(struct net_device
				 *prNetDev, union PFMU_PROFILE_TAG2 *prPfmuTag2,
				 uint8_t ucDesiredNr)
{
	int32_t i4Status = 0;

	prPfmuTag2->rField.uciBfNrow = ucDesiredNr;

	return i4Status;
}

int32_t TxBfProfileTagWrite(struct net_device *prNetDev,
			    union PFMU_PROFILE_TAG1 *prPfmuTag1,
			    union PFMU_PROFILE_TAG2 *prPfmuTag2,
			    uint8_t profileIdx)
{
	int32_t i4Status = 0;
	struct GLUE_INFO *prGlueInfo = NULL;
	struct ADAPTER *prAdapter = NULL;
	uint32_t u4BufLen = 0;
	union PARAM_CUSTOM_TXBF_ACTION_STRUCT rTxBfActionInfo;

	kalMemZero(&rTxBfActionInfo, sizeof(rTxBfActionInfo));

	ASSERT(prNetDev);
	prGlueInfo = *((struct GLUE_INFO **) netdev_priv(prNetDev));
	prAdapter = prGlueInfo->prAdapter;

	DBGLOG(RFTEST, ERROR,
	       "prPfmuTag1 : au4RawData[0] = 0x%08x\n",
	       prPfmuTag1->au4RawData[0]);
	DBGLOG(RFTEST, ERROR,
	       "prPfmuTag1 : au4RawData[1] = 0x%08x\n",
	       prPfmuTag1->au4RawData[1]);
	DBGLOG(RFTEST, ERROR,
	       "prPfmuTag1 : au4RawData[2] = 0x%08x\n",
	       prPfmuTag1->au4RawData[2]);
	DBGLOG(RFTEST, ERROR,
	       "prPfmuTag1 : au4RawData[3] = 0x%08x\n",
	       prPfmuTag1->au4RawData[3]);
	DBGLOG(RFTEST, ERROR,
	       "prPfmuTag1 : au4RawData[4] = 0x%08x\n",
	       prPfmuTag1->au4RawData[4]);
	DBGLOG(RFTEST, ERROR,
	       "prPfmuTag1 : au4RawData[5] = 0x%08x\n",
	       prPfmuTag1->au4RawData[5]);
	DBGLOG(RFTEST, ERROR,
	       "prPfmuTag1 : au4RawData[6] = 0x%08x\n",
	       prPfmuTag1->au4RawData[6]);

	DBGLOG(RFTEST, ERROR,
	       "prPfmuTag2 : au4RawData[0] = 0x%08x\n",
	       prPfmuTag2->au4RawData[0]);
	DBGLOG(RFTEST, ERROR,
	       "prPfmuTag2 : au4RawData[1] = 0x%08x\n",
	       prPfmuTag2->au4RawData[1]);
	DBGLOG(RFTEST, ERROR,
	       "prPfmuTag2 : au4RawData[2] = 0x%08x\n",
	       prPfmuTag2->au4RawData[2]);
	DBGLOG(RFTEST, ERROR,
	       "prPfmuTag2 : au4RawData[3] = 0x%08x\n",
	       prPfmuTag2->au4RawData[3]);
	DBGLOG(RFTEST, ERROR,
	       "prPfmuTag2 : au4RawData[4] = 0x%08x\n",
	       prPfmuTag2->au4RawData[4]);
	DBGLOG(RFTEST, ERROR,
	       "prPfmuTag2 : au4RawData[5] = 0x%08x\n",
	       prPfmuTag2->au4RawData[5]);
	DBGLOG(RFTEST, ERROR,
	       "prPfmuTag2 : au4RawData[6] = 0x%08x\n",
	       prPfmuTag2->au4RawData[6]);

	DBGLOG(RFTEST, ERROR,
	       "prPfmuTag1 : prPfmuTag1->rField.ucProfileID= %d\n",
	       prPfmuTag1->rField.ucProfileID);
	DBGLOG(RFTEST, ERROR,
	       "prPfmuTag1 : prPfmuTag1->rField.ucTxBf= %d\n",
	       prPfmuTag1->rField.ucTxBf);
	DBGLOG(RFTEST, ERROR,
	       "prPfmuTag1 : prPfmuTag1->rField.ucDBW= %d\n",
	       prPfmuTag1->rField.ucDBW);
	DBGLOG(RFTEST, ERROR,
	       "prPfmuTag1 : prPfmuTag1->rField.ucSU_MU= %d\n",
	       prPfmuTag1->rField.ucSU_MU);
	DBGLOG(RFTEST, ERROR,
	       "prPfmuTag1 : prPfmuTag1->rField.ucInvalidProf= %d\n",
	       prPfmuTag1->rField.ucInvalidProf);
	DBGLOG(RFTEST, ERROR,
	       "prPfmuTag1 : prPfmuTag1->rField.ucRMSD= %d\n",
	       prPfmuTag1->rField.ucRMSD);
	DBGLOG(RFTEST, ERROR,
	       "prPfmuTag1 : prPfmuTag1->rField.ucMemAddr1ColIdx= %d\n",
	       prPfmuTag1->rField.ucMemAddr1ColIdx);
	DBGLOG(RFTEST, ERROR,
	       "prPfmuTag1 : prPfmuTag1->rField.ucMemAddr1RowIdx= %d\n",
	       prPfmuTag1->rField.ucMemAddr1RowIdx);
	DBGLOG(RFTEST, ERROR,
	       "prPfmuTag1 : prPfmuTag1->rField.ucMemAddr2ColIdx= %d\n",
	       prPfmuTag1->rField.ucMemAddr2ColIdx);
	DBGLOG(RFTEST, ERROR,
	       "prPfmuTag1 : prPfmuTag1->rField.ucMemAddr2RowIdx= %d\n",
	       prPfmuTag1->rField.ucMemAddr2RowIdx);
	DBGLOG(RFTEST, ERROR,
	       "prPfmuTag1 : prPfmuTag1->rField.ucMemAddr3ColIdx= %d\n",
	       prPfmuTag1->rField.ucMemAddr3ColIdx);
	DBGLOG(RFTEST, ERROR,
	       "prPfmuTag1 : prPfmuTag1->rField.ucMemAddr3RowIdx= %d\n",
	       prPfmuTag1->rField.ucMemAddr3RowIdx);
	DBGLOG(RFTEST, ERROR,
	       "prPfmuTag1 : prPfmuTag1->rField.ucMemAddr4ColIdx= %d\n",
	       prPfmuTag1->rField.ucMemAddr4ColIdx);
	DBGLOG(RFTEST, ERROR,
	       "prPfmuTag1 : prPfmuTag1->rField.ucMemAddr4RowIdx= %d\n",
	       prPfmuTag1->rField.ucMemAddr4RowIdx);
	DBGLOG(RFTEST, ERROR,
	       "prPfmuTag1 : prPfmuTag1->rField.ucNrow= %d\n",
	       prPfmuTag1->rField.ucNrow);
	DBGLOG(RFTEST, ERROR,
	       "prPfmuTag1 : prPfmuTag1->rField.ucNcol= %d\n",
	       prPfmuTag1->rField.ucNcol);
	DBGLOG(RFTEST, ERROR,
	       "prPfmuTag1 : prPfmuTag1->rField.ucNgroup= %d\n",
	       prPfmuTag1->rField.ucNgroup);
	DBGLOG(RFTEST, ERROR,
	       "prPfmuTag1 : prPfmuTag1->rField.ucLM= %d\n",
	       prPfmuTag1->rField.ucLM);
	DBGLOG(RFTEST, ERROR,
	       "prPfmuTag1 : prPfmuTag1->rField.ucCodeBook= %d\n",
	       prPfmuTag1->rField.ucCodeBook);
	DBGLOG(RFTEST, ERROR,
	       "prPfmuTag1 : prPfmuTag1->rField.ucSNR_STS0= %d\n",
	       prPfmuTag1->rField.ucSNR_STS0);
	DBGLOG(RFTEST, ERROR,
	       "prPfmuTag1 : prPfmuTag1->rField.ucSNR_STS1= %d\n",
	       prPfmuTag1->rField.ucSNR_STS1);
	DBGLOG(RFTEST, ERROR,
	       "prPfmuTag1 : prPfmuTag1->rField.ucSNR_STS2= %d\n",
	       prPfmuTag1->rField.ucSNR_STS2);
	DBGLOG(RFTEST, ERROR,
	       "prPfmuTag1 : prPfmuTag1->rField.ucSNR_STS3= %d\n",
	       prPfmuTag1->rField.ucSNR_STS3);

	DBGLOG(RFTEST, ERROR,
	       "prPfmuTag2 : prPfmuTag2->rField.u2SmartAnt = %d\n",
	       prPfmuTag2->rField.u2SmartAnt);
	DBGLOG(RFTEST, ERROR,
	       "prPfmuTag2 : prPfmuTag2->rField.ucSEIdx = %d\n",
	       prPfmuTag2->rField.ucSEIdx);
	DBGLOG(RFTEST, ERROR,
	       "prPfmuTag2 : prPfmuTag2->rField.ucRMSDThd = %d\n",
	       prPfmuTag2->rField.ucRMSDThd);
	DBGLOG(RFTEST, ERROR,
	       "prPfmuTag2 : prPfmuTag2->rField.uciBfTimeOut = %d\n",
	       prPfmuTag2->rField.uciBfTimeOut);
	DBGLOG(RFTEST, ERROR,
	       "prPfmuTag2 : prPfmuTag2->rField.uciBfDBW = %d\n",
	       prPfmuTag2->rField.uciBfDBW);
	DBGLOG(RFTEST, ERROR,
	       "prPfmuTag2 : prPfmuTag2->rField.uciBfNcol = %d\n",
	       prPfmuTag2->rField.uciBfNcol);
	DBGLOG(RFTEST, ERROR,
	       "prPfmuTag2 : prPfmuTag2->rField.uciBfNrow = %d\n",
	       prPfmuTag2->rField.uciBfNrow);

	rTxBfActionInfo.rProfileTagWrite.ucTxBfCategory =
		BF_PFMU_TAG_WRITE;
	rTxBfActionInfo.rProfileTagWrite.ucPfmuId = profileIdx;
	rTxBfActionInfo.rProfileTagWrite.fgBFer = TRUE;
	rTxBfActionInfo.rProfileTagWrite.ucBandIdx = ENUM_BAND_0;
	memcpy(&rTxBfActionInfo.rProfileTagWrite.ucBuffer,
	       prPfmuTag1, sizeof(union PFMU_PROFILE_TAG1));
	memcpy(&rTxBfActionInfo.rProfileTagWrite.ucBuffer[28],
	       prPfmuTag2, sizeof(union PFMU_PROFILE_TAG2));

	i4Status = kalIoctl(prGlueInfo, wlanoidTxBfAction, &rTxBfActionInfo,
			    sizeof(rTxBfActionInfo),
			    FALSE, FALSE, TRUE, &u4BufLen);

	return i4Status;
}

int32_t TxBfProfileTagRead(struct net_device *prNetDev,
			   uint8_t profileIdx, uint8_t fgBFer)
{
	int32_t i4Status = 0;
	struct GLUE_INFO *prGlueInfo = NULL;
	struct ADAPTER *prAdapter = NULL;
	uint32_t u4BufLen = 0;
	union PARAM_CUSTOM_TXBF_ACTION_STRUCT rTxBfActionInfo;

	kalMemZero(&rTxBfActionInfo, sizeof(rTxBfActionInfo));

	ASSERT(prNetDev);
	prGlueInfo = *((struct GLUE_INFO **) netdev_priv(prNetDev));
	prAdapter = prGlueInfo->prAdapter;

	DBGLOG(RFTEST, ERROR,
	       "TxBfProfileTagRead : profileIdx = 0x%08x\n", profileIdx);
	DBGLOG(RFTEST, ERROR,
	       "TxBfProfileTagRead : fgBFer = 0x%08x\n", fgBFer);

	rTxBfActionInfo.rProfileTagRead.ucTxBfCategory =
		BF_PFMU_TAG_READ;
	rTxBfActionInfo.rProfileTagRead.ucProfileIdx = profileIdx;
	rTxBfActionInfo.rProfileTagRead.fgBfer = fgBFer;
	rTxBfActionInfo.rProfileTagRead.ucBandIdx = ENUM_BAND_0;

	i4Status = kalIoctl(prGlueInfo, wlanoidTxBfAction, &rTxBfActionInfo,
			    sizeof(rTxBfActionInfo),
			    TRUE, TRUE, TRUE, &u4BufLen);

	return i4Status;
}

int32_t StaRecCmmUpdate(struct net_device *prNetDev,
			uint8_t ucWlanId, uint8_t ucBssId, uint8_t u4Aid,
			uint8_t aucMacAddr[MAC_ADDR_LEN]
		       )
{
	struct STAREC_COMMON rStaRecCmm;
	struct GLUE_INFO *prGlueInfo = NULL;
	struct ADAPTER *prAdapter = NULL;
	uint32_t u4BufLen = 0;
	int32_t i4Status = 0;

	ASSERT(prNetDev);
	prGlueInfo = *((struct GLUE_INFO **) netdev_priv(prNetDev));
	prAdapter = prGlueInfo->prAdapter;

	kalMemZero(&rStaRecCmm, sizeof(struct STAREC_COMMON));
	/* Tag assignment */
	rStaRecCmm.u2Tag = STA_REC_BASIC;
	rStaRecCmm.u2Length = sizeof(struct STAREC_COMMON);

	/* content */
	kalMemCopy(rStaRecCmm.aucPeerMacAddr, aucMacAddr,
		   MAC_ADDR_LEN);
	rStaRecCmm.ucConnectionState = STATE_CONNECTED;
	rStaRecCmm.u4ConnectionType = EXTCMD_CONNECTION_INFRA_STA;
	rStaRecCmm.u2AID = u4Aid;
	rStaRecCmm.u2ExtraInfo = STAREC_COMMON_EXTRAINFO_V2 |
						STAREC_COMMON_EXTRAINFO_NEWSTAREC | ucWlanId << 8;

	DBGLOG(RFTEST, ERROR, "ucWlanId = 0x%08x\n", ucWlanId);

	i4Status = kalIoctl(prGlueInfo, wlanoidStaRecUpdate, &rStaRecCmm,
			    sizeof(struct STAREC_COMMON),
			    FALSE, FALSE, TRUE, &u4BufLen);

	return i4Status;
}

int32_t StaRecBfUpdate(struct net_device *prNetDev,
		       struct STA_REC_BF_UPD_ARGUMENT *prStaRecBfUpdArg,
		       uint8_t aucMemRow[4], uint8_t aucMemCol[4]
		      )
{
	struct CMD_STAREC_BF rStaRecBF;
	/* PARAM_CUSTOM_STA_REC_UPD_STRUCT_T rStaRecUpdateInfo = {0}; */
	/* P_STA_RECORD_T                        prStaRec; */
	struct GLUE_INFO *prGlueInfo = NULL;
	struct ADAPTER *prAdapter = NULL;
	uint32_t u4BufLen = 0;
	int32_t i4Status = 0;

	ASSERT(prNetDev);
	prGlueInfo = *((struct GLUE_INFO **) netdev_priv(prNetDev));
	prAdapter = prGlueInfo->prAdapter;

	kalMemZero(&rStaRecBF, sizeof(struct CMD_STAREC_BF));
	/* Tag assignment */
	rStaRecBF.u2Tag = STA_REC_BF;
	rStaRecBF.u2Length = sizeof(struct CMD_STAREC_BF);
	rStaRecBF.ucReserved[0] = prStaRecBfUpdArg->u4BssId;
	rStaRecBF.ucReserved[1] = prStaRecBfUpdArg->u4WlanId;
	/* content */
	rStaRecBF.rTxBfPfmuInfo.u2PfmuId = prStaRecBfUpdArg->u4PfmuId;
	rStaRecBF.rTxBfPfmuInfo.ucTotMemRequire =
		prStaRecBfUpdArg->u4TotalMemReq;
	rStaRecBF.rTxBfPfmuInfo.ucMemRequire20M =
		prStaRecBfUpdArg->u4MemReq20M;
	rStaRecBF.rTxBfPfmuInfo.ucMemRow0 = aucMemRow[0];
	rStaRecBF.rTxBfPfmuInfo.ucMemCol0 = aucMemCol[0];
	rStaRecBF.rTxBfPfmuInfo.ucMemRow1 = aucMemRow[1];
	rStaRecBF.rTxBfPfmuInfo.ucMemCol1 = aucMemCol[1];
	rStaRecBF.rTxBfPfmuInfo.ucMemRow2 = aucMemRow[2];
	rStaRecBF.rTxBfPfmuInfo.ucMemCol2 = aucMemCol[2];
	rStaRecBF.rTxBfPfmuInfo.ucMemRow3 = aucMemRow[3];
	rStaRecBF.rTxBfPfmuInfo.ucMemCol3 = aucMemCol[3];
	/* 0 : SU, 1 : MU */
	rStaRecBF.rTxBfPfmuInfo.fgSU_MU = prStaRecBfUpdArg->u4SuMu;
	/* 0: iBF, 1: eBF */
	rStaRecBF.rTxBfPfmuInfo.u1TxBfCap =
		prStaRecBfUpdArg->u4eTxBfCap;
	/* 0: legacy, 1: OFDM, 2: HT, 4: VHT */
	rStaRecBF.rTxBfPfmuInfo.ucSoundingPhy = 1;
	rStaRecBF.rTxBfPfmuInfo.ucNdpaRate =
		prStaRecBfUpdArg->u4NdpaRate;
	rStaRecBF.rTxBfPfmuInfo.ucNdpRate =
		prStaRecBfUpdArg->u4NdpRate;
	rStaRecBF.rTxBfPfmuInfo.ucReptPollRate =
		prStaRecBfUpdArg->u4ReptPollRate;
	/* 0: legacy, 1: OFDM, 2: HT, 4: VHT */
	rStaRecBF.rTxBfPfmuInfo.ucTxMode = prStaRecBfUpdArg->u4TxMode;
	rStaRecBF.rTxBfPfmuInfo.ucNc = prStaRecBfUpdArg->u4Nc;
	rStaRecBF.rTxBfPfmuInfo.ucNr = prStaRecBfUpdArg->u4Nr;
	/* 0 : 20M, 1 : 40M, 2 : 80M, 3 : 80 + 80M */
	rStaRecBF.rTxBfPfmuInfo.ucCBW = prStaRecBfUpdArg->u4Bw;
	rStaRecBF.rTxBfPfmuInfo.ucSEIdx = prStaRecBfUpdArg->u4SpeIdx;
	/* Default setting */
	rStaRecBF.rTxBfPfmuInfo.u2SmartAnt = prStaRecBfUpdArg->u4SmartAnt;
	/* 0: legacy, 1: OFDM, 2: HT, 4: VHT */
	rStaRecBF.rTxBfPfmuInfo.ucSoundingPhy = prStaRecBfUpdArg->u4SoundingPhy;
	rStaRecBF.rTxBfPfmuInfo.uciBfTimeOut = prStaRecBfUpdArg->u4iBfTimeOut;
	rStaRecBF.rTxBfPfmuInfo.uciBfDBW = prStaRecBfUpdArg->u4iBfDBW;
	rStaRecBF.rTxBfPfmuInfo.uciBfNcol = prStaRecBfUpdArg->u4iBfNcol;
	rStaRecBF.rTxBfPfmuInfo.uciBfNrow = prStaRecBfUpdArg->u4iBfNrow;
	rStaRecBF.rTxBfPfmuInfo.u1RuStartIdx = prStaRecBfUpdArg->u4RuStartIdx;
	rStaRecBF.rTxBfPfmuInfo.u1RuEndIdx = prStaRecBfUpdArg->u4RuEndIdx;
	rStaRecBF.rTxBfPfmuInfo.fgTriggerSu = prStaRecBfUpdArg->u4TriggerSu;
	rStaRecBF.rTxBfPfmuInfo.fgTriggerMu = prStaRecBfUpdArg->u4TriggerMu;
	rStaRecBF.rTxBfPfmuInfo.fgNg16Su = prStaRecBfUpdArg->u4Ng16Su;
	rStaRecBF.rTxBfPfmuInfo.fgNg16Mu = prStaRecBfUpdArg->u4Ng16Mu;
	rStaRecBF.rTxBfPfmuInfo.fgCodebook42Su =
		prStaRecBfUpdArg->u4Codebook42Su;
	rStaRecBF.rTxBfPfmuInfo.fgCodebook75Mu =
		prStaRecBfUpdArg->u4Codebook75Mu;
	rStaRecBF.rTxBfPfmuInfo.u1HeLtf = prStaRecBfUpdArg->u4HeLtf;

	i4Status = kalIoctl(prGlueInfo,
			    wlanoidStaRecBFUpdate, &rStaRecBF,
			    sizeof(struct CMD_STAREC_BF), FALSE, FALSE, TRUE,
			    &u4BufLen);

	return i4Status;
}

int32_t StaRecBfHeUpdate(struct net_device *prNetDev,
			struct PFMU_HE_INFO *prPfmuHeInfo, uint32_t u4Config,
			uint8_t ucSuMu, uint8_t ucRuStartIdx,
			uint8_t ucRuEndIdx, uint8_t ucTriggerSu,
			uint8_t ucTriggerMu, uint8_t ucNg16Su,
			uint8_t ucNg16Mu, uint8_t ucCodebook42Su,
			uint8_t ucCodebook75Mu, uint8_t ucHeLtf,
			uint8_t uciBfNcol, uint8_t uciBfNrow)
{
	int32_t i4Status = 0;

	prPfmuHeInfo->u4Config = u4Config;
	prPfmuHeInfo->fgSU_MU = ucSuMu;
	prPfmuHeInfo->u1RuStartIdx = ucRuStartIdx;
	prPfmuHeInfo->u1RuEndIdx = ucRuEndIdx;
	prPfmuHeInfo->fgTriggerSu = ucTriggerSu;
	prPfmuHeInfo->fgTriggerMu = ucTriggerMu;
	prPfmuHeInfo->fgNg16Su = ucNg16Su;
	prPfmuHeInfo->fgNg16Mu = ucNg16Mu;
	prPfmuHeInfo->fgCodebook42Su = ucCodebook42Su;
	prPfmuHeInfo->fgCodebook75Mu = ucCodebook75Mu;
	prPfmuHeInfo->u1HeLtf = ucHeLtf;
	prPfmuHeInfo->uciBfNcol = uciBfNcol;
	prPfmuHeInfo->uciBfNrow = uciBfNrow;

	return i4Status;
}

int32_t DevInfoUpdate(struct net_device *prNetDev,
		      uint8_t ucOwnMacIdx, uint8_t fgBand,
		      uint8_t aucMacAddr[MAC_ADDR_LEN])
{
	struct CMD_DEVINFO_ACTIVE rDevInfo;
	struct GLUE_INFO *prGlueInfo = NULL;
	struct ADAPTER *prAdapter = NULL;
	uint32_t u4BufLen = 0;
	int32_t i4Status = 0;

	ASSERT(prNetDev);
	prGlueInfo = *((struct GLUE_INFO **) netdev_priv(prNetDev));
	prAdapter = prGlueInfo->prAdapter;

	kalMemZero(&rDevInfo, sizeof(struct CMD_DEVINFO_ACTIVE));
	/* Tag assignment */
	rDevInfo.u2Tag = DEV_INFO_ACTIVE;
	rDevInfo.u2Length = sizeof(struct CMD_DEVINFO_ACTIVE);
	/* content */
	kalMemCopy(rDevInfo.aucOwnMacAddr, aucMacAddr,
		   MAC_ADDR_LEN);
	rDevInfo.ucActive = TRUE;
	rDevInfo.ucBandNum = fgBand;
	rDevInfo.ucOwnMacIdx = ucOwnMacIdx;

	i4Status = kalIoctl(prGlueInfo, wlanoidDevInfoActive, &rDevInfo,
			    sizeof(struct CMD_DEVINFO_ACTIVE),
			    FALSE, FALSE, TRUE, &u4BufLen);

	return i4Status;
}

int32_t BssInfoUpdate(struct net_device *prNetDev,
		      uint8_t ucOwnMacIdx, uint8_t ucBssIdx,
		      uint8_t ucBssId[MAC_ADDR_LEN])
{
	struct BSSINFO_BASIC rBssInfo;
	struct GLUE_INFO *prGlueInfo = NULL;
	struct ADAPTER *prAdapter = NULL;
	uint32_t u4BufLen = 0;
	int32_t i4Status = 0;

	ASSERT(prNetDev);
	prGlueInfo = *((struct GLUE_INFO **) netdev_priv(prNetDev));
	prAdapter = prGlueInfo->prAdapter;

	kalMemZero(&rBssInfo, sizeof(struct BSSINFO_BASIC));
	/* Tag assignment */
	rBssInfo.u2Tag = BSS_INFO_BASIC;
	rBssInfo.u2Length = sizeof(struct BSSINFO_BASIC);
	/* content */
	kalMemCopy(rBssInfo.aucBSSID, ucBssId, MAC_ADDR_LEN);
	rBssInfo.ucBcMcWlanidx = ucBssIdx;
	rBssInfo.ucActive = TRUE;
	rBssInfo.u4NetworkType = NETWORK_TYPE_AIS;
	rBssInfo.u2BcnInterval = 100;
	rBssInfo.ucDtimPeriod = 1;

	i4Status = kalIoctl(prGlueInfo,
			    wlanoidBssInfoBasic, &rBssInfo,
			    sizeof(struct BSSINFO_BASIC), FALSE, FALSE, TRUE,
			    &u4BufLen);

	return i4Status;
}

int32_t BssInfoConnectOwnDev(struct net_device *prNetDev,
		      uint8_t ucOwnMacIdx, uint8_t ucBssIdx,
		      uint8_t ucBandIdx)
{
	struct BSSINFO_CONNECT_OWN_DEV rBssInfoConOwnDev;
	struct GLUE_INFO *prGlueInfo = NULL;
	struct ADAPTER *prAdapter = NULL;
	uint32_t u4BufLen = 0;
	int32_t i4Status = 0;

	ASSERT(prNetDev);
	prGlueInfo = *((struct GLUE_INFO **) netdev_priv(prNetDev));
	prAdapter = prGlueInfo->prAdapter;

	kalMemZero(&rBssInfoConOwnDev, sizeof(struct BSSINFO_CONNECT_OWN_DEV));
	/* Tag assignment */
	rBssInfoConOwnDev.u2Tag = BSS_INFO_OWN_MAC;
	rBssInfoConOwnDev.u2Length = sizeof(struct BSSINFO_CONNECT_OWN_DEV);
	/* content */
	rBssInfoConOwnDev.ucHwBSSIndex = ucBssIdx;
	rBssInfoConOwnDev.ucOwnMacIdx = ucOwnMacIdx;
	rBssInfoConOwnDev.ucDbdcIdx = ucBandIdx;
	rBssInfoConOwnDev.u4ConnectionType = OP_MODE_INFRASTRUCTURE;

	i4Status = kalIoctl(prGlueInfo,
			    wlanoidBssInfoConOwnDev, &rBssInfoConOwnDev,
			    sizeof(struct BSSINFO_CONNECT_OWN_DEV), FALSE, FALSE, TRUE,
			    &u4BufLen);

	return i4Status;
}

int32_t TxBfProfileDataRead(struct net_device *prNetDev,
			    uint8_t profileIdx, uint8_t fgBFer,
			    uint8_t ucSubCarrIdxMsb, uint8_t ucSubCarrIdxLsb)
{
	int32_t i4Status = 0;
	struct GLUE_INFO *prGlueInfo = NULL;
	struct ADAPTER *prAdapter = NULL;
	uint32_t u4BufLen = 0;
	union PARAM_CUSTOM_TXBF_ACTION_STRUCT rTxBfActionInfo;

	kalMemZero(&rTxBfActionInfo, sizeof(rTxBfActionInfo));

	ASSERT(prNetDev);
	prGlueInfo = *((struct GLUE_INFO **) netdev_priv(prNetDev));
	prAdapter = prGlueInfo->prAdapter;

	DBGLOG(RFTEST, ERROR,
	       "TxBfProfileDataRead : ucPfmuIdx = 0x%08x\n", profileIdx);
	DBGLOG(RFTEST, ERROR,
	       "TxBfProfileDataRead : fgBFer = 0x%08x\n", fgBFer);
	DBGLOG(RFTEST, ERROR,
	       "TxBfProfileDataRead : ucSubCarrIdxMsb = 0x%08x\n",
	       ucSubCarrIdxMsb);
	DBGLOG(RFTEST, ERROR,
	       "TxBfProfileDataRead : ucSubCarrIdxLsb = 0x%08x\n",
	       ucSubCarrIdxLsb);

	rTxBfActionInfo.rProfileDataRead.ucTxBfCategory =
		BF_PROFILE_READ;
	rTxBfActionInfo.rProfileDataRead.ucPfmuIdx = profileIdx;
	rTxBfActionInfo.rProfileDataRead.fgBFer = fgBFer;
	rTxBfActionInfo.rProfileDataRead.u2SubCarIdx =
		CPU_TO_LE16(ucSubCarrIdxMsb << 8 | ucSubCarrIdxLsb);
	rTxBfActionInfo.rProfileDataRead.ucBandIdx = ENUM_BAND_0;

	i4Status = kalIoctl(prGlueInfo, wlanoidTxBfAction, &rTxBfActionInfo,
			    sizeof(rTxBfActionInfo),
			    TRUE, TRUE, TRUE, &u4BufLen);

	return i4Status;
}

int32_t TxBfProfileDataWrite(struct net_device *prNetDev,
			     uint8_t profileIdx,
			     uint16_t u2SubCarrIdx, uint16_t au2Phi[6],
			     uint8_t aucPsi[6], uint8_t aucDSnr[4]
			    )
{
	int32_t i4Status = 0;
	struct GLUE_INFO *prGlueInfo = NULL;
	struct ADAPTER *prAdapter = NULL;
	uint32_t u4BufLen = 0;
	union PARAM_CUSTOM_TXBF_ACTION_STRUCT rTxBfActionInfo;

	kalMemZero(&rTxBfActionInfo, sizeof(rTxBfActionInfo));

	ASSERT(prNetDev);
	prGlueInfo = *((struct GLUE_INFO **) netdev_priv(prNetDev));
	prAdapter = prGlueInfo->prAdapter;

	DBGLOG(RFTEST, ERROR,
	       "TxBfProfileDataWrite : ucPfmuIdx = 0x%08x\n", profileIdx);
	DBGLOG(RFTEST, ERROR,
	       "TxBfProfileDataWrite : u2SubCarrIdx = 0x%08x\n",
	       u2SubCarrIdx);

	DBGLOG(RFTEST, ERROR,
	       "TxBfProfileDataWrite : au2Phi[0] = 0x%08x\n", au2Phi[0]);
	DBGLOG(RFTEST, ERROR,
	       "TxBfProfileDataWrite : au2Phi[1] = 0x%08x\n", au2Phi[1]);
	DBGLOG(RFTEST, ERROR,
	       "TxBfProfileDataWrite : au2Phi[2] = 0x%08x\n", au2Phi[2]);
	DBGLOG(RFTEST, ERROR,
	       "TxBfProfileDataWrite : au2Phi[3] = 0x%08x\n", au2Phi[3]);
	DBGLOG(RFTEST, ERROR,
	       "TxBfProfileDataWrite : au2Phi[4] = 0x%08x\n", au2Phi[4]);
	DBGLOG(RFTEST, ERROR,
	       "TxBfProfileDataWrite : au2Phi[5] = 0x%08x\n", au2Phi[5]);

	DBGLOG(RFTEST, ERROR,
	       "TxBfProfileDataWrite : aucPsi[0] = 0x%08x\n", aucPsi[0]);
	DBGLOG(RFTEST, ERROR,
	       "TxBfProfileDataWrite : aucPsi[1] = 0x%08x\n", aucPsi[1]);
	DBGLOG(RFTEST, ERROR,
	       "TxBfProfileDataWrite : aucPsi[2] = 0x%08x\n", aucPsi[2]);
	DBGLOG(RFTEST, ERROR,
	       "TxBfProfileDataWrite : aucPsi[3] = 0x%08x\n", aucPsi[3]);
	DBGLOG(RFTEST, ERROR,
	       "TxBfProfileDataWrite : aucPsi[4] = 0x%08x\n", aucPsi[4]);
	DBGLOG(RFTEST, ERROR,
	       "TxBfProfileDataWrite : aucPsi[5] = 0x%08x\n", aucPsi[5]);

	DBGLOG(RFTEST, ERROR,
	       "TxBfProfileDataWrite : aucDSnr[0] = 0x%x\n", aucDSnr[0]);
	DBGLOG(RFTEST, ERROR,
	       "TxBfProfileDataWrite : aucDSnr[1] = 0x%x\n", aucDSnr[1]);
	DBGLOG(RFTEST, ERROR,
	       "TxBfProfileDataWrite : aucDSnr[2] = 0x%x\n", aucDSnr[2]);
	DBGLOG(RFTEST, ERROR,
	       "TxBfProfileDataWrite : aucDSnr[3] = 0x%x\n", aucDSnr[3]);

	rTxBfActionInfo.rProfileDataWrite.ucTxBfCategory =
		BF_PROFILE_WRITE;
	rTxBfActionInfo.rProfileDataWrite.ucPfmuIdx = profileIdx;
	rTxBfActionInfo.rProfileDataWrite.u2SubCarrIdxLsb =
		u2SubCarrIdx;
	rTxBfActionInfo.rProfileDataWrite.u2SubCarrIdxMsb =
		u2SubCarrIdx >> 8;
	rTxBfActionInfo.rProfileDataWrite.rTxBfPfmuData.rField.u2Phi11
		= au2Phi[0];
	rTxBfActionInfo.rProfileDataWrite.rTxBfPfmuData.rField.u2Phi21
		= au2Phi[1];
	rTxBfActionInfo.rProfileDataWrite.rTxBfPfmuData.rField.u2Phi31
		= au2Phi[2];
	rTxBfActionInfo.rProfileDataWrite.rTxBfPfmuData.rField.u2Phi22
		= au2Phi[3];
	rTxBfActionInfo.rProfileDataWrite.rTxBfPfmuData.rField.u2Phi32
		= au2Phi[4];
	rTxBfActionInfo.rProfileDataWrite.rTxBfPfmuData.rField.u2Phi33
		= au2Phi[5];
	rTxBfActionInfo.rProfileDataWrite.rTxBfPfmuData.rField.ucPsi21
		= aucPsi[0];
	rTxBfActionInfo.rProfileDataWrite.rTxBfPfmuData.rField.ucPsi31
		= aucPsi[1];
	rTxBfActionInfo.rProfileDataWrite.rTxBfPfmuData.rField.ucPsi41
		= aucPsi[2];
	rTxBfActionInfo.rProfileDataWrite.rTxBfPfmuData.rField.ucPsi32
		= aucPsi[3];
	rTxBfActionInfo.rProfileDataWrite.rTxBfPfmuData.rField.ucPsi42
		= aucPsi[4];
	rTxBfActionInfo.rProfileDataWrite.rTxBfPfmuData.rField.ucPsi43
		= aucPsi[5];
	rTxBfActionInfo.rProfileDataWrite.rTxBfPfmuData.rField.u2dSNR00
		= aucDSnr[0];
	rTxBfActionInfo.rProfileDataWrite.rTxBfPfmuData.rField.u2dSNR01
		= aucDSnr[1];
	rTxBfActionInfo.rProfileDataWrite.rTxBfPfmuData.rField.u2dSNR02
		= aucDSnr[2];
	rTxBfActionInfo.rProfileDataWrite.rTxBfPfmuData.rField.u2dSNR03
		= aucDSnr[3];

	i4Status = kalIoctl(prGlueInfo, wlanoidTxBfAction, &rTxBfActionInfo,
			    sizeof(rTxBfActionInfo),
			    FALSE, FALSE, TRUE, &u4BufLen);

	return i4Status;
}

int32_t TxBfProfilePnRead(struct net_device *prNetDev,
			  uint8_t profileIdx)
{
	int32_t i4Status = 0;
	struct GLUE_INFO *prGlueInfo = NULL;
	struct ADAPTER *prAdapter = NULL;
	uint32_t u4BufLen = 0;
	union PARAM_CUSTOM_TXBF_ACTION_STRUCT rTxBfActionInfo;

	kalMemZero(&rTxBfActionInfo, sizeof(rTxBfActionInfo));

	ASSERT(prNetDev);
	prGlueInfo = *((struct GLUE_INFO **) netdev_priv(prNetDev));
	prAdapter = prGlueInfo->prAdapter;

	DBGLOG(RFTEST, ERROR,
	       "TxBfProfilePnRead : ucPfmuIdx = 0x%08x\n", profileIdx);

	rTxBfActionInfo.rProfilePnRead.ucTxBfCategory = BF_PN_READ;
	rTxBfActionInfo.rProfilePnRead.ucPfmuIdx = profileIdx;

	i4Status = kalIoctl(prGlueInfo, wlanoidTxBfAction, &rTxBfActionInfo,
			    sizeof(rTxBfActionInfo),
			    FALSE, FALSE, TRUE, &u4BufLen);

	return i4Status;
}

int32_t TxBfProfilePnWrite(struct net_device *prNetDev,
		   uint8_t profileIdx, uint16_t u2bw, uint16_t au2XSTS[12])
{
	int32_t i4Status = 0;
	struct GLUE_INFO *prGlueInfo = NULL;
	struct ADAPTER *prAdapter = NULL;
	uint32_t u4BufLen = 0;
	union PARAM_CUSTOM_TXBF_ACTION_STRUCT rTxBfActionInfo;

	kalMemZero(&rTxBfActionInfo, sizeof(rTxBfActionInfo));

	ASSERT(prNetDev);
	prGlueInfo = *((struct GLUE_INFO **) netdev_priv(prNetDev));
	prAdapter = prGlueInfo->prAdapter;

	DBGLOG(RFTEST, ERROR,
	       "TxBfProfilePnWrite : ucPfmuIdx = 0x%08x\n", profileIdx);
	DBGLOG(RFTEST, ERROR,
	       "TxBfProfilePnWrite : u2bw = 0x%08x\n", u2bw);
	DBGLOG(RFTEST, ERROR,
	       "TxBfProfilePnWrite : au2XSTS[0] = 0x%08x\n", au2XSTS[0]);
	DBGLOG(RFTEST, ERROR,
	       "TxBfProfilePnWrite : au2XSTS[1] = 0x%08x\n", au2XSTS[1]);
	DBGLOG(RFTEST, ERROR,
	       "TxBfProfilePnWrite : au2XSTS[2] = 0x%08x\n", au2XSTS[2]);
	DBGLOG(RFTEST, ERROR,
	       "TxBfProfilePnWrite : au2XSTS[3] = 0x%08x\n", au2XSTS[3]);
	DBGLOG(RFTEST, ERROR,
	       "TxBfProfilePnWrite : au2XSTS[4] = 0x%08x\n", au2XSTS[4]);
	DBGLOG(RFTEST, ERROR,
	       "TxBfProfilePnWrite : au2XSTS[5] = 0x%08x\n", au2XSTS[5]);
	DBGLOG(RFTEST, ERROR,
	       "TxBfProfilePnWrite : au2XSTS[6] = 0x%08x\n", au2XSTS[6]);
	DBGLOG(RFTEST, ERROR,
	       "TxBfProfilePnWrite : au2XSTS[7] = 0x%08x\n", au2XSTS[7]);
	DBGLOG(RFTEST, ERROR,
	       "TxBfProfilePnWrite : au2XSTS[8] = 0x%08x\n", au2XSTS[8]);
	DBGLOG(RFTEST, ERROR,
	       "TxBfProfilePnWrite : au2XSTS[9] = 0x%08x\n", au2XSTS[9]);
	DBGLOG(RFTEST, ERROR,
	       "TxBfProfilePnWrite : au2XSTS[10] = 0x%08x\n", au2XSTS[10]);
	DBGLOG(RFTEST, ERROR,
	       "TxBfProfilePnWrite : au2XSTS[11] = 0x%08x\n", au2XSTS[11]);

	rTxBfActionInfo.rProfilePnWrite.ucTxBfCategory =
		BF_PN_WRITE;
	rTxBfActionInfo.rProfilePnWrite.ucPfmuIdx = profileIdx;
	rTxBfActionInfo.rProfilePnWrite.u2bw = u2bw;
	memcpy(&rTxBfActionInfo.rProfilePnWrite.ucBuf[0], &au2XSTS[0],
	       sizeof(uint16_t) * 12);

	i4Status = kalIoctl(prGlueInfo, wlanoidTxBfAction, &rTxBfActionInfo,
			sizeof(rTxBfActionInfo),
			FALSE, FALSE, TRUE, &u4BufLen);

	return i4Status;
}

int32_t TxBfSounding(struct net_device *prNetDev,
		     uint8_t ucSuMu,	/* 0/1/2/3 */
		     uint8_t ucNumSta,	/* 00~04 */
		     uint8_t ucSndInterval,	/* 00~FF */
		     uint8_t ucWLan0,	/* 00~7F */
		     uint8_t ucWLan1,	/* 00~7F */
		     uint8_t ucWLan2,	/* 00~7F */

		     uint8_t ucWLan3	/* 00~7F */
		    )
{
	int32_t i4Status = 0;
	struct GLUE_INFO *prGlueInfo = NULL;
	struct ADAPTER *prAdapter = NULL;
	uint32_t u4BufLen = 0;
	union PARAM_CUSTOM_TXBF_ACTION_STRUCT rTxBfActionInfo;

	kalMemZero(&rTxBfActionInfo, sizeof(rTxBfActionInfo));

	ASSERT(prNetDev);
	prGlueInfo = *((struct GLUE_INFO **) netdev_priv(prNetDev));
	prAdapter = prGlueInfo->prAdapter;

	DBGLOG(RFTEST, ERROR, "TxBfSounding : ucSuMu = 0x%08x\n",
	       ucSuMu);
	DBGLOG(RFTEST, ERROR, "TxBfSounding : ucNumSta = 0x%08x\n",
	       ucNumSta);
	DBGLOG(RFTEST, ERROR,
	       "TxBfSounding : ucSndInterval = 0x%08x\n", ucSndInterval);
	DBGLOG(RFTEST, ERROR, "TxBfSounding : ucWLan0 = 0x%08x\n",
	       ucWLan0);
	DBGLOG(RFTEST, ERROR, "TxBfSounding : ucWLan1 = 0x%08x\n",
	       ucWLan1);
	DBGLOG(RFTEST, ERROR, "TxBfSounding : ucWLan2 = 0x%08x\n",
	       ucWLan2);
	DBGLOG(RFTEST, ERROR, "TxBfSounding : ucWLan3 = 0x%08x\n",
	       ucWLan3);

	if (ucSuMu < SOUNDING_MAX) {
		rTxBfActionInfo.rTxBfSoundingStart.ucCmdCategoryID =
								BF_SOUNDING_ON;
		rTxBfActionInfo.rTxBfSoundingStart.ucSuMuSndMode = ucSuMu;
		rTxBfActionInfo.rTxBfSoundingStart.ucStaNum = ucNumSta;
		rTxBfActionInfo.rTxBfSoundingStart.u4SoundingInterval =
								ucSndInterval;
		rTxBfActionInfo.rTxBfSoundingStart.ucWlanId[0] = ucWLan0;
		rTxBfActionInfo.rTxBfSoundingStart.ucWlanId[1] = ucWLan1;
		rTxBfActionInfo.rTxBfSoundingStart.ucWlanId[2] = ucWLan2;
		rTxBfActionInfo.rTxBfSoundingStart.ucWlanId[3] = ucWLan3;
	} else {
		DBGLOG(RFTEST, ERROR, "TxBfSounding Wrong Sounding Mode\n");
		return WLAN_STATUS_INVALID_DATA;
	}

	i4Status = kalIoctl(prGlueInfo, wlanoidTxBfAction, &rTxBfActionInfo,
			    sizeof(rTxBfActionInfo),
			    FALSE, FALSE, TRUE, &u4BufLen);

	return i4Status;
}

int32_t TxBfSoundingStop(struct net_device *prNetDev)
{
	int32_t i4Status = 0;
	struct GLUE_INFO *prGlueInfo = NULL;
	struct ADAPTER *prAdapter = NULL;
	uint32_t u4BufLen = 0;
	union PARAM_CUSTOM_TXBF_ACTION_STRUCT rTxBfActionInfo;

	kalMemZero(&rTxBfActionInfo, sizeof(rTxBfActionInfo));

	ASSERT(prNetDev);
	prGlueInfo = *((struct GLUE_INFO **) netdev_priv(prNetDev));
	prAdapter = prGlueInfo->prAdapter;

	DBGLOG(RFTEST, ERROR, "TxBfSoundingStop\n");

	rTxBfActionInfo.rTxBfSoundingStop.ucTxBfCategory =
		BF_SOUNDING_OFF;
	rTxBfActionInfo.rTxBfSoundingStop.ucSndgStop = 1;

	i4Status = kalIoctl(prGlueInfo, wlanoidTxBfAction, &rTxBfActionInfo,
			    sizeof(rTxBfActionInfo),
			    FALSE, FALSE, TRUE, &u4BufLen);

	return i4Status;
}

int32_t TxBfTxApply(struct net_device *prNetDev,
		    uint8_t ucWlanId, uint8_t fgETxBf, uint8_t fgITxBf,
		    uint8_t fgMuTxBf)
{
	int32_t i4Status = 0;
	struct GLUE_INFO *prGlueInfo = NULL;
	struct ADAPTER *prAdapter = NULL;
	uint32_t u4BufLen = 0;
	union PARAM_CUSTOM_TXBF_ACTION_STRUCT rTxBfActionInfo;

	kalMemZero(&rTxBfActionInfo, sizeof(rTxBfActionInfo));

	ASSERT(prNetDev);
	prGlueInfo = *((struct GLUE_INFO **) netdev_priv(prNetDev));
	prAdapter = prGlueInfo->prAdapter;

	DBGLOG(RFTEST, ERROR,
	       "TxBfTxApply : ucWlanId = 0x%08x, fgETxBf = 0x%08x,fgITxBf = 0x%08x,fgMuTxBf = 0x%08x\n",
	       ucWlanId, fgETxBf, fgITxBf, fgMuTxBf);

	rTxBfActionInfo.rTxBfTxApply.ucTxBfCategory =
		BF_DATA_PACKET_APPLY;
	rTxBfActionInfo.rTxBfTxApply.ucWlanId = ucWlanId;
	rTxBfActionInfo.rTxBfTxApply.fgETxBf = fgETxBf;
	rTxBfActionInfo.rTxBfTxApply.fgITxBf = fgITxBf;
	rTxBfActionInfo.rTxBfTxApply.fgMuTxBf = fgMuTxBf;

	i4Status = kalIoctl(prGlueInfo, wlanoidTxBfAction, &rTxBfActionInfo,
			    sizeof(rTxBfActionInfo),
			    FALSE, FALSE, TRUE, &u4BufLen);

	return i4Status;
}

int32_t TxBfPfmuMemAlloc(struct net_device *prNetDev,
			 uint8_t ucSuMuMode, uint8_t ucWlanIdx)
{
	int32_t i4Status = 0;
	struct GLUE_INFO *prGlueInfo = NULL;
	struct ADAPTER *prAdapter = NULL;
	uint32_t u4BufLen = 0;
	union PARAM_CUSTOM_TXBF_ACTION_STRUCT rTxBfActionInfo;

	kalMemZero(&rTxBfActionInfo, sizeof(rTxBfActionInfo));

	ASSERT(prNetDev);
	prGlueInfo = *((struct GLUE_INFO **) netdev_priv(prNetDev));
	prAdapter = prGlueInfo->prAdapter;

	DBGLOG(RFTEST, ERROR,
	       "TxBfPfmuMemAlloc : ucSuMuMode = 0x%08x, ucWlanIdx = 0x%08x\n",
	       ucSuMuMode, ucWlanIdx);

	rTxBfActionInfo.rTxBfPfmuMemAlloc.ucTxBfCategory =
		BF_PFMU_MEM_ALLOCATE;
	rTxBfActionInfo.rTxBfPfmuMemAlloc.ucSuMuMode = ucSuMuMode;
	rTxBfActionInfo.rTxBfPfmuMemAlloc.ucWlanIdx = ucWlanIdx;

	i4Status = kalIoctl(prGlueInfo, wlanoidTxBfAction, &rTxBfActionInfo,
			    sizeof(rTxBfActionInfo),
			    FALSE, FALSE, TRUE, &u4BufLen);

	return i4Status;
}

int32_t TxBfPfmuMemRelease(struct net_device *prNetDev,
			   uint8_t ucWlanId)
{
	int32_t i4Status = 0;
	struct GLUE_INFO *prGlueInfo = NULL;
	struct ADAPTER *prAdapter = NULL;
	uint32_t u4BufLen = 0;
	union PARAM_CUSTOM_TXBF_ACTION_STRUCT rTxBfActionInfo;

	kalMemZero(&rTxBfActionInfo, sizeof(rTxBfActionInfo));

	ASSERT(prNetDev);
	prGlueInfo = *((struct GLUE_INFO **) netdev_priv(prNetDev));
	prAdapter = prGlueInfo->prAdapter;

	DBGLOG(RFTEST, ERROR,
	       "TxBfPfmuMemRelease : ucWlanId = 0x%08x\n", ucWlanId);

	rTxBfActionInfo.rTxBfPfmuMemRls.ucTxBfCategory =
		BF_PFMU_MEM_RELEASE;
	rTxBfActionInfo.rTxBfPfmuMemRls.ucWlanId = ucWlanId;

	i4Status = kalIoctl(prGlueInfo, wlanoidTxBfAction, &rTxBfActionInfo,
			    sizeof(rTxBfActionInfo),
			    FALSE, FALSE, TRUE, &u4BufLen);

	return i4Status;
}

int32_t TxBfBssInfoUpdate(struct net_device *prNetDev,
			  uint8_t ucOwnMacIdx, uint8_t ucBssIdx,
			  uint8_t ucBssId[MAC_ADDR_LEN])
{
	int32_t i4Status = 0;
	struct GLUE_INFO *prGlueInfo = NULL;
	struct ADAPTER *prAdapter = NULL;
	/* UINT_32 u4BufLen = 0; */
	union PARAM_CUSTOM_TXBF_ACTION_STRUCT rTxBfActionInfo;
	struct BSS_INFO *prBssInfo;

	kalMemZero(&rTxBfActionInfo, sizeof(rTxBfActionInfo));

	ASSERT(prNetDev);
	prGlueInfo = *((struct GLUE_INFO **) netdev_priv(prNetDev));
	prAdapter = prGlueInfo->prAdapter;

	DBGLOG(RFTEST, ERROR,
	       "TxBfBssInfoUpdate : ucOwnMacIdx = 0x%08x\n", ucOwnMacIdx);
	DBGLOG(RFTEST, ERROR,
	       "TxBfBssInfoUpdate : ucBssIdx = 0x%08x\n", ucBssIdx);
	DBGLOG(RFTEST, ERROR,
	       "TxBfBssInfoUpdate : ucBssId[0] = 0x%08x\n", ucBssId[0]);
	DBGLOG(RFTEST, ERROR,
	       "TxBfBssInfoUpdate : ucBssId[1] = 0x%08x\n", ucBssId[1]);
	DBGLOG(RFTEST, ERROR,
	       "TxBfBssInfoUpdate : ucBssId[2] = 0x%08x\n", ucBssId[2]);
	DBGLOG(RFTEST, ERROR,
	       "TxBfBssInfoUpdate : ucBssId[3] = 0x%08x\n", ucBssId[3]);
	DBGLOG(RFTEST, ERROR,
	       "TxBfBssInfoUpdate : ucBssId[4] = 0x%08x\n", ucBssId[4]);
	DBGLOG(RFTEST, ERROR,
	       "TxBfBssInfoUpdate : ucBssId[5] = 0x%08x\n", ucBssId[5]);

	prBssInfo = prAdapter->aprBssInfo[ucBssIdx];

	if (!prBssInfo)
		return WLAN_STATUS_FAILURE;
	prBssInfo->ucOwnMacIndex = ucOwnMacIdx;
	memcpy(&prBssInfo->aucBSSID, &ucBssId[0], MAC_ADDR_LEN);

	nicUpdateBss(prAdapter, prBssInfo->ucBssIndex);

	return i4Status;
}

/* iwpriv ra0 set assoc=[mac:hh:hh:hh:hh:hh:hh]-[wtbl:dd]-
 * [ownmac:dd]-[type:xx]-[mode:mmm]-[bw:dd]-[nss:ss]-[maxrate:kkk_dd]
 */
int32_t TxBfManualAssoc(struct net_device *prNetDev,
			uint8_t aucMac[MAC_ADDR_LEN],
			uint8_t ucType,
			uint8_t ucWtbl,
			uint8_t ucOwnmac,
			uint8_t ucMode,
			uint8_t ucBw,
			uint8_t ucNss, uint8_t ucPfmuId, uint8_t ucMarate,
			uint8_t ucSpeIdx, uint8_t ucRca2)
{
	struct CMD_MANUAL_ASSOC_STRUCT rManualAssoc;
	/* P_STA_RECORD_T prStaRec; */
	struct GLUE_INFO *prGlueInfo = NULL;
	struct ADAPTER *prAdapter = NULL;
	uint32_t u4BufLen = 0;
	int32_t i4Status = 0;
	/* uint8_t ucNsts;
	 * uint32_t i;
	 */

	ASSERT(prNetDev);
	prGlueInfo = *((struct GLUE_INFO **) netdev_priv(prNetDev));
	prAdapter = prGlueInfo->prAdapter;

	kalMemZero(&rManualAssoc,
		   sizeof(struct CMD_MANUAL_ASSOC_STRUCT));
	/* Tag assignment */
	rManualAssoc.u2Tag = STA_REC_MAUNAL_ASSOC;
	rManualAssoc.u2Length = sizeof(struct
				       CMD_MANUAL_ASSOC_STRUCT);
	/* content */
	kalMemCopy(rManualAssoc.aucMac, aucMac, MAC_ADDR_LEN);
	rManualAssoc.ucType = ucType;
	rManualAssoc.ucWtbl = ucWtbl;
	rManualAssoc.ucOwnmac = ucOwnmac;
	rManualAssoc.ucMode = ucMode;
	rManualAssoc.ucBw = ucBw;
	rManualAssoc.ucNss = ucNss;
	rManualAssoc.ucPfmuId = ucPfmuId;
	rManualAssoc.ucMarate = ucMarate;
	rManualAssoc.ucSpeIdx = ucSpeIdx;
	rManualAssoc.ucaid = ucRca2;

#if 0
	switch (ucMode) {
	case 0:		/* abggnanac */
		prStaRec->ucDesiredPhyTypeSet =
			aucPhyCfg2PhyTypeSet[PHY_TYPE_SET_802_11ABGNAC];
		break;
	case 1:		/* bggnan */
		prStaRec->ucDesiredPhyTypeSet =
			aucPhyCfg2PhyTypeSet[PHY_TYPE_SET_802_11ABGN];
		break;
	case 2:		/* aanac */
		prStaRec->ucDesiredPhyTypeSet =
			aucPhyCfg2PhyTypeSet[PHY_TYPE_SET_802_11ANAC];
		break;
	default:
		prStaRec->ucDesiredPhyTypeSet =
			aucPhyCfg2PhyTypeSet[PHY_TYPE_SET_802_11ABGNAC];
		break;
	}

	prStaRec->rTxBfPfmuStaInfo.u2PfmuId = ucPfmuId;

	memcpy(prStaRec->aucMacAddr, aucMac, MAC_ADDR_LEN);

	i4Status = kalIoctl(prGlueInfo, wlanoidStaRecUpdate, &rStaRecUpdateInfo,
			    sizeof(struct PARAM_CUSTOM_STA_REC_UPD_STRUCT),
			    FALSE, FALSE, TRUE, &u4BufLen);
#endif

	i4Status = kalIoctl(prGlueInfo, wlanoidManualAssoc, &rManualAssoc,
			    sizeof(struct CMD_MANUAL_ASSOC_STRUCT),
			    FALSE, FALSE, TRUE, &u4BufLen);

	return i4Status;
}

#if CFG_SUPPORT_TX_BF_FPGA
int32_t TxBfPseudoTagUpdate(struct net_device *prNetDev,
			    uint8_t ucLm, uint8_t ucNr,
			    uint8_t ucNc, uint8_t ucBw, uint8_t ucCodeBook,
			    uint8_t ucGroup)
{
	int32_t i4Status = 0;
	struct GLUE_INFO *prGlueInfo = NULL;
	uint32_t u4BufLen = 0;
	union PARAM_CUSTOM_TXBF_ACTION_STRUCT rTxBfActionInfo;

	kalMemZero(&rTxBfActionInfo, sizeof(rTxBfActionInfo));

	ASSERT(prNetDev);
	prGlueInfo = *((struct GLUE_INFO **) netdev_priv(prNetDev));

	DBGLOG(RFTEST, ERROR,
	       "TxBfPseudoTagUpdate : ucLm = 0x%08x, ucNr = 0x%08x, ucNc = 0x%08x, ucBw = 0x%08x, ucCodeBook = 0x%08x, ucGroup = 0x%08x\n",
	       ucLm, ucNr, ucNc, ucBw, ucCodeBook, ucGroup);

	rTxBfActionInfo.rTxBfProfileSwTagWrite.ucTxBfCategory =
		BF_PFMU_SW_TAG_WRITE;
	rTxBfActionInfo.rTxBfProfileSwTagWrite.ucLm = ucLm;
	rTxBfActionInfo.rTxBfProfileSwTagWrite.ucNr = ucNr;
	rTxBfActionInfo.rTxBfProfileSwTagWrite.ucNc = ucNc;
	rTxBfActionInfo.rTxBfProfileSwTagWrite.ucBw = ucBw;
	rTxBfActionInfo.rTxBfProfileSwTagWrite.ucCodebook =
		ucCodeBook;
	rTxBfActionInfo.rTxBfProfileSwTagWrite.ucgroup = ucGroup;
	rTxBfActionInfo.rTxBfProfileSwTagWrite.ucTxBf = ENUM_BAND_0;

	i4Status = kalIoctl(prGlueInfo, wlanoidTxBfAction, &rTxBfActionInfo,
			    sizeof(rTxBfActionInfo),
			    FALSE, FALSE, TRUE, &u4BufLen);

	return i4Status;
}
#endif

#endif
#endif /*CFG_SUPPORT_QA_TOOL */
#if (CONFIG_WLAN_SERVICE == 1)
uint32_t ServiceRfTestInit(void *winfos)
{

	uint32_t u4SetInfoLen = 0;
	uint32_t rStatus = WLAN_STATUS_SUCCESS;
	uint8_t ucBssIndex = 0;

	struct GLUE_INFO *prGlueInfo = NULL;
	struct ADAPTER *prAdapter = NULL;
	struct test_wlan_info *prTestWinfo;


	ASSERT(winfos);
	prTestWinfo = (struct test_wlan_info *)winfos;
	prGlueInfo = *((struct GLUE_INFO **) netdev_priv(prTestWinfo->net_dev));
	ASSERT(prGlueInfo);
	prAdapter = prGlueInfo->prAdapter;
	ASSERT(prAdapter);

	/*1. do abort Scan, reset AIS FSM to IDLE*/

	ucBssIndex = wlanGetBssIdx(prTestWinfo->net_dev);

	if (!IS_BSS_INDEX_VALID(ucBssIndex)) {
		DBGLOG(REQ, WARN, "Test Abort SCAN invalid BssIndex\n");
		return WLAN_STATUS_FAILURE;
	}

	DBGLOG(REQ, WARN, "Test Abort SCAN ucBssIndex = %d\n", ucBssIndex);

	rStatus = kalIoctlByBssIdx(prGlueInfo,
			   wlanoidAbortScan,
			   NULL, 1, FALSE, FALSE, TRUE, &u4SetInfoLen,
			   ucBssIndex);

	if (rStatus != WLAN_STATUS_SUCCESS)
		DBGLOG(REQ, ERROR, "wlanoidAbortScan fail 0x%x\n", rStatus);

	return rStatus;

}
uint32_t ServiceIcapInit(struct ADAPTER *prAdapter)
{
	struct mt66xx_chip_info *prChipInfo = NULL;
	struct ATE_OPS_T *prAteOps = NULL;
	struct ICAP_INFO_T *prIcapInfo = NULL;
	uint32_t u4IQArrayLen = 0;
	uint32_t u4Status = WLAN_STATUS_SUCCESS;


	ASSERT(prAdapter);
	prChipInfo = prAdapter->chip_info;
	ASSERT(prChipInfo);
	prAteOps = prChipInfo->prAteOps;
	ASSERT(prAteOps);
	prIcapInfo = &prAdapter->rIcapInfo;
	ASSERT(prIcapInfo);

	u4IQArrayLen =
		MAX_ICAP_IQ_DATA_CNT * sizeof(struct _RBIST_IQ_DATA_T);

	/*init IQ data array*/
	if (!prIcapInfo->prIQArray) {
		prIcapInfo->prIQArray =
				kalMemAlloc(u4IQArrayLen, VIR_MEM_TYPE);
		if (!prIcapInfo->prIQArray) {
			DBGLOG(RFTEST, ERROR,
				"Not enough memory for IQ_Array\n");
			return WLAN_STATUS_NOT_ACCEPTED;
		}
	}
	prIcapInfo->u4IQArrayIndex = 0;
	prIcapInfo->u4ICapEventCnt = 0;

	kalMemZero(prIcapInfo->au4ICapDumpIndex,
		sizeof(prIcapInfo->au4ICapDumpIndex));
	kalMemZero(prIcapInfo->prIQArray, u4IQArrayLen);


	if (prIcapInfo->eIcapState == ICAP_STATE_INIT) {

		if (prAteOps->icapRiseVcoreClockRate)
			prAteOps->icapRiseVcoreClockRate();

		u4Status = WLAN_STATUS_SUCCESS;
	} else {
		DBGLOG(RFTEST, ERROR, "icap start ignore state in %d\n",
			prIcapInfo->eIcapState);
		u4Status = WLAN_STATUS_NOT_ACCEPTED;
	}

	DBGLOG(RFTEST, STATE, "%s done\n", __func__);

	return u4Status;
}
uint32_t ServiceIcapDeInit(struct ADAPTER *prAdapter)
{
	struct mt66xx_chip_info *prChipInfo = NULL;
	struct ATE_OPS_T *prAteOps = NULL;
	struct ICAP_INFO_T *prIcapInfo = NULL;
	uint32_t u4Status = WLAN_STATUS_SUCCESS;

	ASSERT(prAdapter);
	prChipInfo = prAdapter->chip_info;
	ASSERT(prChipInfo);
	prAteOps = prChipInfo->prAteOps;
	ASSERT(prAteOps);
	prIcapInfo = &prAdapter->rIcapInfo;
	ASSERT(prIcapInfo);

	if (prAteOps->icapDownVcoreClockRate)
		prAteOps->icapDownVcoreClockRate();

	if (prIcapInfo->prIQArray != NULL)
		kalMemFree(prIcapInfo->prIQArray,
			   VIR_MEM_TYPE,
			   0);

	prIcapInfo->u4IQArrayIndex = 0;
	prIcapInfo->u4ICapEventCnt = 0;
	prIcapInfo->prIQArray = NULL;

	return u4Status;
}
uint32_t ServiceWlanOid(void *winfos,
	 enum op_wlan_oid oidType,
	 void *param,
	 uint32_t paramLen,
	 uint32_t *u4BufLen,
	 void *rsp_data)
{
	int32_t i4Status = 0;
	uint32_t u4BufLen2;
	uint32_t *resp;
	struct GLUE_INFO *prGlueInfo = NULL;
	struct ADAPTER *prAdapter = NULL;
	struct RECAL_INFO_T *prReCalInfo = NULL;
	boolean fgRead, fgWaitResp, fgCmd;
	PFN_OID_HANDLER_FUNC pfnOidHandler = NULL;
	struct test_wlan_info *prTestWinfo;
	struct hqa_m_rx_stat *prStatsData = NULL;
#if CFG_SUPPORT_ANT_SWAP
	struct mt66xx_chip_info *prChipInfo = NULL;
#endif
	struct ICAP_INFO_T *prIcapInfo = NULL;

	ASSERT(winfos);

    /* Avoid assert caused by u4BufLen = NULL. */
	if (u4BufLen == NULL)
		u4BufLen = &u4BufLen2;

	prTestWinfo = (struct test_wlan_info *)winfos;

	prGlueInfo = *((struct GLUE_INFO **) netdev_priv(prTestWinfo->net_dev));
	ASSERT(prGlueInfo);
	prAdapter = prGlueInfo->prAdapter;
	ASSERT(prAdapter);
	prReCalInfo = &prAdapter->rReCalInfo;
#if CFG_SUPPORT_ANT_SWAP
	prChipInfo = prAdapter->chip_info;
	ASSERT(prChipInfo);
#endif
	prIcapInfo = &prAdapter->rIcapInfo;
	ASSERT(prIcapInfo);

	/* Normal set */
	fgRead = FALSE;
	fgWaitResp = FALSE;
	fgCmd = TRUE;

	switch (oidType) {
	case OP_WLAN_OID_SET_TEST_MODE_START:
		ServiceRfTestInit(winfos);
		pfnOidHandler = wlanoidRftestSetTestMode;
		break;
	case OP_WLAN_OID_SET_TEST_MODE_ABORT:
		pfnOidHandler = wlanoidRftestSetAbortTestMode;
		break;
	case OP_WLAN_OID_RFTEST_SET_AUTO_TEST:
		pfnOidHandler = wlanoidRftestSetAutoTest;
		break;
	case OP_WLAN_OID_QUERY_RX_STATISTICS:
		prStatsData = (struct hqa_m_rx_stat *)rsp_data;
		pfnOidHandler = wlanoidQueryRxStatistics;
		fgRead = TRUE;
		fgWaitResp = TRUE;
		fgCmd = TRUE;
		break;
	/* ICAP Operation Function -- Start*/
	case OP_WLAN_OID_SET_TEST_ICAP_MODE:
		pfnOidHandler = wlanoidRftestSetTestIcapMode;
		break;
	case OP_WLAN_OID_SET_TEST_ICAP_START:
		ServiceIcapInit(prAdapter);
		pfnOidHandler = wlanoidExtRfTestICapStart;
		break;
	case OP_WLAN_OID_SET_TEST_ICAP_ABORT:
		i4Status = ServiceIcapDeInit(prAdapter);
		pfnOidHandler = wlanoidExtRfTestICapStart;
		break;
	case OP_WLAN_OID_SET_TEST_ICAP_STATUS:

		if (!rsp_data)
			return WLAN_STATUS_INVALID_DATA;

		resp = (uint32_t *)rsp_data;

		if (prIcapInfo->eIcapState == ICAP_STATE_FW_DUMP_DONE) {
			DBGLOG(RFTEST, INFO, "icap capture done!\n");
			*resp = 0; /*response QA TOOL CAPTURE success*/
			 return WLAN_STATUS_SUCCESS;
		} else if (prIcapInfo->eIcapState == ICAP_STATE_FW_DUMPING) {
			DBGLOG(RFTEST, INFO, "icap fw dumping !!!\n");
			*resp = 1; /*response QA TOOL CAPTURE wait*/
			return WLAN_STATUS_SUCCESS;
		}

		pfnOidHandler = wlanoidExtRfTestICapStatus;
		*resp = 1; /*response QA TOOL CAPTURE wait*/

		break;
	case OP_WLAN_OID_GET_TEST_ICAP_MAX_DATA_LEN:
		/* Maximum 1KB = ICAP_EVENT_DATA_SAMPLE (256) slots */
		if (!rsp_data)
			return WLAN_STATUS_INVALID_DATA;

		resp = (uint32_t *)rsp_data;
		*resp = ICAP_EVENT_DATA_SAMPLE * sizeof(uint32_t);
		return WLAN_STATUS_SUCCESS;
	case OP_WLAN_OID_GET_TEST_ICAP_DATA:
		if ((prIcapInfo->eIcapState != ICAP_STATE_QA_TOOL_CAPTURE) &&
			(prIcapInfo->eIcapState != ICAP_STATE_FW_DUMP_DONE)) {
			DBGLOG(RFTEST, ERROR, "ICAP State = %d don't support\n",
				prIcapInfo->eIcapState);
			return WLAN_STATUS_NOT_SUPPORTED;
		}
		pfnOidHandler = wlanoidRfTestICapGetIQData;
		fgRead = TRUE;
		fgWaitResp = FALSE;
		fgCmd = FALSE;
		break;
	/* ICAP Operation Function -- END*/
	case OP_WLAN_OID_RFTEST_QUERY_AUTO_TEST:
		pfnOidHandler = wlanoidRftestQueryAutoTest;
		fgRead = TRUE;
		fgWaitResp = TRUE;
		fgCmd = TRUE;
		break;
	case OP_WLAN_OID_SET_MCR_WRITE:
		pfnOidHandler = wlanoidSetMcrWrite;
		fgRead = TRUE;
		fgWaitResp = TRUE;
		fgCmd = TRUE;
		break;
	case OP_WLAN_OID_QUERY_MCR_READ:
		pfnOidHandler = wlanoidQueryMcrRead;
		break;
	case OP_WLAN_OID_GET_RECAL_COUNT:
		*u4BufLen = prReCalInfo->u4Count;

		return WLAN_STATUS_SUCCESS;
	case OP_WLAN_OID_GET_RECAL_CONTENT:
		if (prReCalInfo->u4Count > 0) {
			kalMemCopy(u4BufLen, &prReCalInfo->prCalArray[0],
			(prReCalInfo->u4Count * sizeof(struct RECAL_DATA_T)));
		}

		return WLAN_STATUS_SUCCESS;
	case OP_WLAN_OID_GET_ANTSWAP_CAPBILITY:
#if CFG_SUPPORT_ANT_SWAP
		if (!prChipInfo) {
			DBGLOG(RFTEST, ERROR, "prChipInfo is NULL\n");
			return -EFAULT;
		}

		DBGLOG(RFTEST, INFO, "HQA_GetAntSwapCapability [%d]\n",
				prGlueInfo->prAdapter->fgIsSupportAntSwp);

		DBGLOG(RFTEST, INFO, "ucMaxSwapAntenna = [%d]\n",
					prChipInfo->ucMaxSwapAntenna);

		if (prGlueInfo->prAdapter->fgIsSupportAntSwp)
			*u4BufLen = prChipInfo->ucMaxSwapAntenna;
		else
			*u4BufLen = 0;

		return WLAN_STATUS_SUCCESS;
#endif
	case OP_WLAN_OID_RESET_RECAL_COUNT:
		kalMemSet(&prReCalInfo->prCalArray[0], 0,
		(prReCalInfo->u4Count * sizeof(struct RECAL_DATA_T)));

		prReCalInfo->u4Count = 0;

		return WLAN_STATUS_SUCCESS;

	case OP_WLAN_OID_NUM:
	default:
		return WLAN_STATUS_FAILURE;
	}

	i4Status = kalIoctl(prGlueInfo, /* prGlueInfo */
		pfnOidHandler,  /* pfnOidHandler */
		param, /* pvInfoBuf */
		paramLen, /* u4InfoBufLen */
		fgRead, /* fgRead */
		fgWaitResp, /* fgWaitResp */
		fgCmd, /* fgCmd */
		u4BufLen); /* pu4QryInfoLen */

	if ((prStatsData) &&
		(oidType == OP_WLAN_OID_QUERY_RX_STATISTICS)) {

		/* 264 = 66 items * 4 bytes */
		kalMemCopy(&prStatsData->mac_rx_fcs_err_cnt,
		&(g_HqaRxStat.MAC_FCS_Err), 264);
	}

	return i4Status;
}
#endif
