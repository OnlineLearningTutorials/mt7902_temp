/* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause */
/*
 * Copyright (c) 2016 MediaTek Inc.
 */
/*! \file   gl_ate_agent.h
 *    \brief  This file includes private ioctl support.
 */

#ifndef _GL_ATE_AGENT_H
#define _GL_ATE_AGENT_H
#if CFG_SUPPORT_QA_TOOL
/*******************************************************************************
 *                         C O M P I L E R   F L A G S
 *******************************************************************************
 */

/*******************************************************************************
 *                    E X T E R N A L   R E F E R E N C E S
 *******************************************************************************
 */

extern uint32_t u4RxStatSeqNum;

#if CFG_SUPPORT_TX_BF
extern union PFMU_PROFILE_TAG1 g_rPfmuTag1;
extern union PFMU_PROFILE_TAG2 g_rPfmuTag2;
extern union PFMU_DATA g_rPfmuData;
#endif

/*******************************************************************************
 *                              C O N S T A N T S
 *******************************************************************************
 */

/*******************************************************************************
 *                                 M A C R O S
 *******************************************************************************
 */

/*******************************************************************************
 *                             D A T A   T Y P E S
 *******************************************************************************
 */

struct STA_REC_BF_UPD_ARGUMENT {
	uint32_t u4WlanId;
	uint32_t u4BssId;
	uint32_t u4PfmuId;
	uint32_t u4SuMu;
	uint32_t u4eTxBfCap;
	uint32_t u4SoundingPhy;
	uint32_t u4NdpaRate;
	uint32_t u4NdpRate;
	uint32_t u4ReptPollRate;
	uint32_t u4TxMode;
	uint32_t u4Nc;
	uint32_t u4Nr;
	uint32_t u4Bw;
	uint32_t u4TotalMemReq;
	uint32_t u4MemReq20M;
	uint32_t au4MemRow[4];
	uint32_t au4MemCol[4];
	uint32_t u4SmartAnt;
	uint32_t u4SpeIdx;
	uint32_t u4iBfTimeOut;
	uint32_t u4iBfDBW;
	uint32_t u4iBfNcol;
	uint32_t u4iBfNrow;
	uint32_t u4RuStartIdx;
	uint32_t u4RuEndIdx;
	uint32_t u4TriggerSu;
	uint32_t u4TriggerMu;
	uint32_t u4Ng16Su;
	uint32_t u4Ng16Mu;
	uint32_t u4Codebook42Su;
	uint32_t u4Codebook75Mu;
	uint32_t u4HeLtf;
};

struct ATE_OPS_T {
	int32_t (*setICapStart)(struct GLUE_INFO *prGlueInfo,
				uint32_t fgTrigger,
				uint32_t fgRingCapEn,
				uint32_t u4Event,
				uint32_t u4Node,
				uint32_t u4Len,
				uint32_t u4StopCycle,
				uint32_t u4BW,
				uint32_t u4MACTriggerEvent,
				uint32_t u4SourceAddrLSB,
				uint32_t u4SourceAddrMSB,
				uint32_t u4Band);
	int32_t (*getICapStatus)(struct GLUE_INFO *prGlueInfo);
	int32_t (*getICapIQData)(struct GLUE_INFO *prGlueInfo,
				 uint8_t *pData,
				 uint32_t u4IQType,
				 uint32_t u4WFNum);
	void (*getRbistDataDumpEvent)(struct ADAPTER *prAdapter,
				      uint8_t *pucEventBuf);
	void (*icapRiseVcoreClockRate)(void);
	void (*icapDownVcoreClockRate)(void);
	uint32_t u4EnBitWidth;/* 0:32bit, 1:96bit, 2:128bit, 3:64bit*/
	uint32_t u4Architech;/* 0:on-chip, 1:on-the-fly */
	uint32_t u4PhyIdx;
	uint32_t u4EmiStartAddress;
	uint32_t u4EmiEndAddress;
	uint32_t u4EmiMsbAddress;
	uint32_t u4CapSource;
};


/*******************************************************************************
 *                  F U N C T I O N   D E C L A R A T I O N S
 *******************************************************************************
 */

int Set_ResetStatCounter_Proc(struct net_device *prNetDev,
			      uint8_t *prInBuf);
int SetATE(struct net_device *prNetDev, uint8_t *prInBuf);
int SetATEDa(struct net_device *prNetDev, uint8_t *prInBuf);
int SetATESa(struct net_device *prNetDev, uint8_t *prInBuf);
int SetATEChannel(struct net_device *prNetDev,
		  uint8_t *prInBuf);
int SetATETxPower0(struct net_device *prNetDev,
		   uint8_t *prInBuf);
int SetATETxGi(struct net_device *prNetDev,
	       uint8_t *prInBuf);
int SetATETxBw(struct net_device *prNetDev,
	       uint8_t *prInBuf);
int SetATETxMode(struct net_device *prNetDev,
		 uint8_t *prInBuf);
int SetATETxLength(struct net_device *prNetDev,
		   uint8_t *prInBuf);
int SetATETxCount(struct net_device *prNetDev,
		  uint8_t *prInBuf);
int SetATETxMcs(struct net_device *prNetDev,
		uint8_t *prInBuf);
int SetATEIpg(struct net_device *prNetDev,
	      uint8_t *prInBuf);
#if CFG_SUPPORT_ANT_SWAP
int SetATEAntSwp(struct net_device *prNetDev,
	      uint8_t *prInBuf);
#endif

#if CFG_SUPPORT_TX_BF
int Set_TxBfProfileTag_Help(struct net_device *prNetDev,
			    uint8_t *prInBuf);
int Set_TxBfProfileTag_InValid(struct net_device *prNetDev,
			       uint8_t *prInBuf);
int Set_TxBfProfileTag_PfmuIdx(struct net_device *prNetDev,
			       uint8_t *prInBuf);
int Set_TxBfProfileTag_BfType(struct net_device *prNetDev,
			      uint8_t *prInBuf);
int Set_TxBfProfileTag_DBW(struct net_device *prNetDev,
			   uint8_t *prInBuf);
int Set_TxBfProfileTag_SuMu(struct net_device *prNetDev,
			    uint8_t *prInBuf);
int Set_TxBfProfileTag_Mem(struct net_device *prNetDev,
			   uint8_t *prInBuf);
int Set_TxBfProfileTag_Matrix(struct net_device *prNetDev,
			      uint8_t *prInBuf);
int Set_TxBfProfileTag_SNR(struct net_device *prNetDev,
			   uint8_t *prInBuf);
int Set_TxBfProfileTag_SmartAnt(struct net_device *prNetDev,
				uint8_t *prInBuf);
int Set_TxBfProfileTag_SeIdx(struct net_device *prNetDev,
			     uint8_t *prInBuf);
int Set_TxBfProfileTag_RmsdThrd(struct net_device *prNetDev,
				uint8_t *prInBuf);
int Set_TxBfProfileTag_McsThrd(struct net_device *prNetDev,
			       uint8_t *prInBuf);
int Set_TxBfProfileTag_TimeOut(struct net_device *prNetDev,
			       uint8_t *prInBuf);
int Set_TxBfProfileTag_DesiredBW(struct net_device
				 *prNetDev, uint8_t *prInBuf);
int Set_TxBfProfileTag_DesiredNc(struct net_device
				 *prNetDev, uint8_t *prInBuf);
int Set_TxBfProfileTag_DesiredNr(struct net_device
				 *prNetDev, uint8_t *prInBuf);
int Set_TxBfProfileTagRead(struct net_device *prNetDev,
			   uint8_t *prInBuf);
int Set_TxBfProfileTagWrite(struct net_device *prNetDev,
			    uint8_t *prInBuf);
int Set_StaRecCmmUpdate(struct net_device *prNetDev,
			uint8_t *prInBuf);
int Set_StaRecBfUpdate(struct net_device *prNetDev,
		       uint8_t *prInBuf);
int Set_StaRecBfHeUpdate(struct net_device *prNetDev,
		       uint8_t *prInBuf);

int Set_DevInfoUpdate(struct net_device *prNetDev,
		      uint8_t *prInBuf);

int Set_BssInfoUpdate(struct net_device *prNetDev,
		      uint8_t *prInBuf);
int Set_TxBfProfileDataRead(struct net_device *prNetDev,
			    uint8_t *prInBuf);
int Set_TxBfProfileDataWrite(struct net_device *prNetDev,
			     uint8_t *prInBuf);
int Set_Trigger_Sounding_Proc(struct net_device *prNetDev,
			      uint8_t *prInBuf);
int Set_Stop_Sounding_Proc(struct net_device *prNetDev,
			   uint8_t *prInBuf);
int Set_TxBfTxApply(struct net_device *prNetDev,
		    uint8_t *prInBuf);
int Set_TxBfProfilePnRead(struct net_device *prNetDev,
			  uint8_t *prInBuf);
int Set_TxBfProfilePnWrite(struct net_device *prNetDev,
			   uint8_t *prInBuf);
int Set_TxBfManualAssoc(struct net_device *prNetDev,
			uint8_t *prInBuf);
int Set_TxBfPfmuMemAlloc(struct net_device *prNetDev,
			 uint8_t *prInBuf);
int Set_TxBfPfmuMemRelease(struct net_device *prNetDev,
			   uint8_t *prInBuf);

#if CFG_SUPPORT_MU_MIMO
int Set_MUGetInitMCS(struct net_device *prNetDev,
		     uint8_t *prInBuf);
int Set_MUCalInitMCS(struct net_device *prNetDev,
		     uint8_t *prInBuf);
int Set_MUCalLQ(struct net_device *prNetDev,
		uint8_t *prInBuf);
int Set_MUGetLQ(struct net_device *prNetDev,
		uint8_t *prInBuf);
int Set_MUSetSNROffset(struct net_device *prNetDev,
		       uint8_t *prInBuf);
int Set_MUSetZeroNss(struct net_device *prNetDev,
		     uint8_t *prInBuf);
int Set_MUSetSpeedUpLQ(struct net_device *prNetDev,
		       uint8_t *prInBuf);
int Set_MUSetMUTable(struct net_device *prNetDev,
		     uint8_t *prInBuf);
int Set_MUSetGroup(struct net_device *prNetDev,
		   uint8_t *prInBuf);
int Set_MUGetQD(struct net_device *prNetDev,
		uint8_t *prInBuf);
int Set_MUSetEnable(struct net_device *prNetDev,
		    uint8_t *prInBuf);
int Set_MUSetGID_UP(struct net_device *prNetDev,
		    uint8_t *prInBuf);
int Set_MUTriggerTx(struct net_device *prNetDev,
		    uint8_t *prInBuf);
#endif

#if CFG_SUPPORT_TX_BF_FPGA
int Set_TxBfProfileSwTagWrite(struct net_device *prNetDev,
			      uint8_t *prInBuf);
#endif
#endif


int WriteEfuse(struct net_device *prNetDev,
	       uint8_t *prInBuf);
int SetTxTargetPower(struct net_device *prNetDev,
		     uint8_t *prInBuf);

#if (CFG_SUPPORT_DFS_MASTER == 1)
int SetRddReport(struct net_device *prNetDev,
		 uint8_t *prInBuf);
int SetByPassCac(struct net_device *prNetDev,
		 uint8_t *prInBuf);
int SetRadarDetectMode(struct net_device *prNetDev,
		       uint8_t *prInBuf);
#endif

int AteCmdSetHandle(struct net_device *prNetDev,
		    uint8_t *prInBuf, uint32_t u4InBufLen);

#endif /*CFG_SUPPORT_QA_TOOL */
#endif /* _GL_ATE_AGENT_H */
