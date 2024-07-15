/* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause */
/*
 * Copyright (c) 2016 MediaTek Inc.
 */
/*! \file   "hif.h"
 *    \brief  Functions for the driver to register bus and setup the IRQ
 *
 *    Functions for the driver to register bus and setup the IRQ
 */

#ifndef _HIF_H
#define _HIF_H

#include "hif_pdma.h"

#if defined(_HIF_PCIE)
#define HIF_NAME "PCIE"
#else
#error "No HIF defined!"
#endif

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
#if CFG_SUPPORT_PCIE_ASPM
#define ENABLE_ASPM_L1 2
#define DISABLE_ASPM_L1 0

#define  PCI_EXT_CAP_ID_L1PMSS				0x1E
#define  PCI_L1PMSS_CAP						4
#define  PCI_L1PM_CAP_PCIPM_L12				0x00000001
#define  PCI_L1PM_CAP_PCIPM_L11				0x00000002
#define  PCI_L1PM_CAP_ASPM_L12				0x00000004
#define  PCI_L1PM_CAP_ASPM_L11				0x00000008
#define  PCI_L1PM_CAP_L1PM_SS				0x00000010
#define  PCI_L1PM_CAP_PORT_CMR_TIME_MASK	0x0000FF00
#define  PCI_L1PM_CAP_PWR_ON_SCALE_MASK		0x00030000
#define  PCI_L1PM_CAP_PWR_ON_VALUE_MASK		0x00F80000
#define  PCI_L1PMSS_CTR1					8
#define  PCI_L1PM_CTR1_PCIPM_L12_EN			0x00000001
#define  PCI_L1PM_CTR1_PCIPM_L11_EN			0x00000002
#define  PCI_L1PM_CTR1_ASPM_L12_EN			0x00000004
#define  PCI_L1PM_CTR1_ASPM_L11_EN			0x00000008
#define  PCI_L1PMSS_ENABLE_MASK	            (PCI_L1PM_CTR1_PCIPM_L12_EN |   \
					PCI_L1PM_CTR1_PCIPM_L11_EN |   \
					PCI_L1PM_CTR1_ASPM_L12_EN |    \
					PCI_L1PM_CTR1_ASPM_L11_EN)
#define  PCI_L1PM_ENABLE_MASK			0x3

#define PCIE_ASPM_CHECK_L1(reg)	((((reg) & PCI_EXP_LNKCAP_ASPMS) >> 10) & 0x2)

#endif
/*******************************************************************************
 *                             D A T A   T Y P E S
 *******************************************************************************
 */

struct GL_HIF_INFO;

struct HIF_MEM_OPS {
	void (*allocTxDesc)(struct GL_HIF_INFO *prHifInfo,
			    struct RTMP_DMABUF *prDescRing,
			    uint32_t u4Num);
	void (*allocRxDesc)(struct GL_HIF_INFO *prHifInfo,
			    struct RTMP_DMABUF *prDescRing,
			    uint32_t u4Num);
	bool (*allocTxCmdBuf)(struct RTMP_DMABUF *prDmaBuf,
			      uint32_t u4Num, uint32_t u4Idx);
	void (*allocTxDataBuf)(struct MSDU_TOKEN_ENTRY *prToken,
			       uint32_t u4Idx);
	void *(*allocRxBuf)(struct GL_HIF_INFO *prHifInfo,
			    struct RTMP_DMABUF *prDmaBuf,
			    uint32_t u4Num, uint32_t u4Idx);
	void *(*allocRuntimeMem)(uint32_t u4SrcLen);
	bool (*copyCmd)(struct GL_HIF_INFO *prHifInfo,
			struct RTMP_DMACB *prTxCell, void *pucBuf,
			void *pucSrc1, uint32_t u4SrcLen1,
			void *pucSrc2, uint32_t u4SrcLen2);
	bool (*copyEvent)(struct GL_HIF_INFO *prHifInfo,
			  struct RTMP_DMACB *pRxCell,
			  struct RXD_STRUCT *pRxD,
			  struct RTMP_DMABUF *prDmaBuf,
			  uint8_t *pucDst, uint32_t u4Len);
	bool (*copyTxData)(struct MSDU_TOKEN_ENTRY *prToken,
			   void *pucSrc, uint32_t u4Len, uint32_t u4offset);
	bool (*copyRxData)(struct GL_HIF_INFO *prHifInfo,
			   struct RTMP_DMACB *pRxCell,
			   struct RTMP_DMABUF *prDmaBuf,
			   struct SW_RFB *prSwRfb);
	void (*flushCache)(struct GL_HIF_INFO *prHifInfo,
			   void *pucSrc, uint32_t u4Len);
	phys_addr_t (*mapTxBuf)(struct GL_HIF_INFO *prHifInfo,
			  void *pucBuf, uint32_t u4Offset, uint32_t u4Len);
	phys_addr_t (*mapRxBuf)(struct GL_HIF_INFO *prHifInfo,
			  void *pucBuf, uint32_t u4Offset, uint32_t u4Len);
	void (*unmapTxBuf)(struct GL_HIF_INFO *prHifInfo,
			   phys_addr_t rDmaAddr, uint32_t u4Len);
	void (*unmapRxBuf)(struct GL_HIF_INFO *prHifInfo,
			   phys_addr_t rDmaAddr, uint32_t u4Len);
	void (*freeDesc)(struct GL_HIF_INFO *prHifInfo,
			 struct RTMP_DMABUF *prDescRing);
	void (*freeBuf)(void *pucSrc, uint32_t u4Len);
	void (*freePacket)(void *pvPacket);
	void (*dumpTx)(struct GL_HIF_INFO *prHifInfo,
		       struct RTMP_TX_RING *prTxRing,
		       uint32_t u4Idx, uint32_t u4DumpLen);
	void (*dumpRx)(struct GL_HIF_INFO *prHifInfo,
		       struct RTMP_RX_RING *prRxRing,
		       uint32_t u4Idx, uint32_t u4DumpLen);
};

/* host interface's private data structure, which is attached to os glue
 ** layer info structure.
 */

enum pcie_suspend_state {
	PCIE_STATE_PRE_SUSPEND_WAITING, /* Waiting for FW suspend flow status */
	PCIE_STATE_PRE_SUSPEND_DONE,
	PCIE_STATE_PRE_SUSPEND_FAIL,
	PCIE_STATE_SUSPEND_ENTERING,
	PCIE_STATE_PRE_RESUME_DONE,
	PCIE_STATE_SUSPEND
};

struct GL_HIF_INFO {
	struct pci_dev *pdev;
	struct pci_dev *prDmaDev;
	struct HIF_MEM_OPS rMemOps;

	uint32_t u4IrqId;
	int32_t u4HifCnt;

	/* PCI MMIO Base Address, all access will use */
	void *CSRBaseAddress;

	/* Shared memory of all 1st pre-allocated
	 * TxBuf associated with each TXD
	 */
	/* Shared memory for Tx descriptors */
	struct RTMP_DMABUF TxDescRing[NUM_OF_TX_RING];
	/* AC0~3 + HCCA */
	struct RTMP_TX_RING TxRing[NUM_OF_TX_RING];

	/* Shared memory for RX descriptors */
	struct RTMP_DMABUF RxDescRing[NUM_OF_RX_RING];
	struct RTMP_RX_RING RxRing[NUM_OF_RX_RING];

	u_int8_t fgIntReadClear;
	u_int8_t fgMbxReadClear;

	uint32_t u4IntStatus;

	struct MSDU_TOKEN_INFO rTokenInfo;

	struct ERR_RECOVERY_CTRL_T rErrRecoveryCtl;
	struct timer_list rSerTimer;
	struct list_head rTxCmdQ;
	struct list_head rTxDataQ;
	uint32_t u4TxDataQLen;

	bool fgIsPowerOff;
	bool fgIsDumpLog;

	enum pcie_suspend_state eSuspendtate;

	uint32_t u4PcieLTR;
	uint32_t u4PcieASPM;
};

struct BUS_INFO {
	const uint32_t top_cfg_base;	/* TOP_CFG_BASE address */
	const struct PCIE_CHIP_CR_MAPPING *bus2chip;
	const uint32_t bus2chip_tbl_size;
	const uint32_t tx_ring_cmd_idx;
	const uint32_t tx_ring_wa_cmd_idx;
	const uint32_t tx_ring_fwdl_idx;
	const uint32_t tx_ring0_data_idx;
	const uint32_t tx_ring1_data_idx;
	const unsigned int max_static_map_addr;
	const uint32_t fw_own_clear_addr;
	const uint32_t fw_own_clear_bit;
	const bool fgCheckDriverOwnInt;
	const uint32_t u4DmaMask;
	/* host pdma/wfdma0 base address */
	const uint32_t host_dma0_base;
	/* host wfdma1 base address */
	const uint32_t host_dma1_base;
	/* host ext conn hif wrap */
	const uint32_t host_ext_conn_hif_wrap_base;
	const uint32_t host_int_status_addr;
	const uint32_t host_int_txdone_bits;
	const uint32_t host_int_rxdone_bits;

	/* tx pdma/wfdma ring base address */
	const uint32_t host_tx_ring_base;
	/* tx pdma/wfdma ring ext control base address */
	const uint32_t host_tx_ring_ext_ctrl_base;
	/* tx pdma/wfdma ring cpu index address */
	const uint32_t host_tx_ring_cidx_addr;
	/* tx pdma/wfdma ring dma index address */
	const uint32_t host_tx_ring_didx_addr;
	/* tx pdma/wfdma ring count address */
	const uint32_t host_tx_ring_cnt_addr;

	/* rx pdma/wfdma ring base address */
	const uint32_t host_rx_ring_base;
	/* rx pdma/wfdma ring ext control base address */
	const uint32_t host_rx_ring_ext_ctrl_base;
	/* rx pdma/wfdma ring cpu index address */
	const uint32_t host_rx_ring_cidx_addr;
	/* rx pdma/wfdma ring dma index address */
	const uint32_t host_rx_ring_didx_addr;
	/* rx pdma/wfdma ring count address */
	const uint32_t host_rx_ring_cnt_addr;
	/*skip tx ring index*/
	const uint32_t skip_tx_ring;

#if (CFG_SUPPORT_CONNAC2X == 1)
	/* rx wfdma_1 ring base address */
	const uint32_t host_wfdma1_rx_ring_base;
	/* rx wfdma_1 ring cpu index address */
	const uint32_t host_wfdma1_rx_ring_cidx_addr;
	/* rx wfdma_1 ring dma index address */
	const uint32_t host_wfdma1_rx_ring_didx_addr;
	/* rx wfdma_1 ring count address */
	const uint32_t host_wfdma1_rx_ring_cnt_addr;
	/* rx wfdma_1 ring ext control base address */
	const uint32_t host_wfdma1_rx_ring_ext_ctrl_base;
	/* rx event ring buffer size */
	const uint32_t rx_evt_ring_buf_size;
#endif /* CFG_SUPPORT_CONNAC2X == 1 */

#if (CFG_SUPPORT_CONNAC3X == 1)
	const uint32_t pcie2ap_remap_2;
	const uint32_t ap2wf_remap_1;
	struct wfdma_group_info *wfmda_host_tx_group;
	const uint32_t wfmda_host_tx_group_len;
	struct wfdma_group_info *wfmda_host_rx_group;
	const uint32_t wfmda_host_rx_group_len;
	struct wfdma_group_info *wfmda_wm_tx_group;
	const uint32_t wfmda_wm_tx_group_len;
	struct wfdma_group_info *wfmda_wm_rx_group;
	const uint32_t wfmda_wm_rx_group_len;

	struct DMASHDL_CFG *prDmashdlCfg;
	struct PLE_TOP_CR *prPleTopCr;
	struct PSE_TOP_CR *prPseTopCr;
	struct PP_TOP_CR *prPpTopCr;
	struct pse_group_info *prPseGroup;
	const uint32_t u4PseGroupLen;
#endif

	void (*pdmaSetup)(struct GLUE_INFO *prGlueInfo, u_int8_t enable);
	uint32_t (*updateTxRingMaxQuota)(struct ADAPTER *prAdapter,
		uint8_t ucWmmIndex, uint32_t u4MaxQuota);
	void (*pdmaStop)(struct GLUE_INFO *prGlueInfo, u_int8_t enable);
	u_int8_t (*pdmaPollingIdle)(struct GLUE_INFO *prGlueInfo);
	void (*enableInterrupt)(struct ADAPTER *prAdapter);
	void (*disableInterrupt)(struct ADAPTER *prAdapter);
	void (*processTxInterrupt)(struct ADAPTER *prAdapter);
	void (*processRxInterrupt)(struct ADAPTER *prAdapter);
	void (*lowPowerOwnRead)(struct ADAPTER *prAdapter, u_int8_t *pfgResult);
	void (*lowPowerOwnSet)(struct ADAPTER *prAdapter, u_int8_t *pfgResult);
	void (*lowPowerOwnClear)(struct ADAPTER *prAdapter,
		u_int8_t *pfgResult);
	void (*wakeUpWiFi)(struct ADAPTER *prAdapter);
	bool (*isValidRegAccess)(struct ADAPTER *prAdapter,
				 uint32_t u4Register);
	void (*getMailboxStatus)(struct ADAPTER *prAdapter, uint32_t *pu4Val);
	void (*setDummyReg)(struct GLUE_INFO *prGlueInfo);
	void (*checkDummyReg)(struct GLUE_INFO *prGlueInfo);
	void (*tx_ring_ext_ctrl)(struct GLUE_INFO *prGlueInfo,
		struct RTMP_TX_RING *tx_ring, uint32_t index);
	void (*rx_ring_ext_ctrl)(struct GLUE_INFO *prGlueInfo,
		struct RTMP_RX_RING *rx_ring, uint32_t index);
	void (*wfdmaManualPrefetch)(struct GLUE_INFO *prGlueInfo);
	uint32_t (*getSoftwareInterrupt)(IN struct ADAPTER *prAdapter);
	void (*processSoftwareInterrupt)(IN struct ADAPTER *prAdapter);
	void (*softwareInterruptMcu)(IN struct ADAPTER *prAdapter,
		u_int32_t intrBitMask);
	void (*hifRst)(struct GLUE_INFO *prGlueInfo);
	void (*initPcieInt)(struct GLUE_INFO *prGlueInfo);
	void (*configPcieASPM)(struct GLUE_INFO *prGlueInfo, bool fgActivate);
	void (*setCTSbyRate)(struct GLUE_INFO *prGlueInfo,
		struct MSDU_INFO *prMsduInfo, void *prTxDesc);
	void (*devReadIntStatus)(struct ADAPTER *prAdapter,
		OUT uint32_t *pu4IntStatus);
	/* Do DMASDHL init when WIFISYS is initialized at probe, L0.5 reset,
	 * etc.
	 */
	void (*DmaShdlInit)(IN struct ADAPTER *prAdapter);
	/* Although DMASHDL was init, we need to reinit it again due to falcon
	 * L1 reset, etc. Take MT7961 as example. The difference between
	 * mt7961DmashdlInit and mt7961DmashdlReInit is that we don't init CRs
	 * such as refill, min_quota, max_quota in mt7961DmashdlReInit, which
	 * are backup and restored in fw. The reason why some DMASHDL CRs are
	 * reinit by driver and some by fw is
	 *     1. Some DMASHDL CRs shall be inited before fw releases UMAC reset
	 *        in L1 procedure. Then, these CRs are backup and restored by fw
	 *     2. However, the backup and restore of each DMASHDL CR in fw needs
	 *        wm DLM space. So, we save DLM space by reinit the remaining
	 *        DMASHDL CRs in driver.
	 */
	void (*DmaShdlReInit)(IN struct ADAPTER *prAdapter);
	uint8_t (*setRxRingHwAddr)(struct RTMP_RX_RING *prRxRing,
		struct BUS_INFO *prBusInfo,
		uint32_t u4SwRingIdx);
	bool (*wfdmaAllocRxRing)(
		struct GLUE_INFO *prGlueInfo,
		bool fgAllocMem);
#if CFG_SUPPORT_HOST_RX_WM_EVENT_FROM_PSE
	u_int8_t (*checkPortForRxEventFromPse)(struct ADAPTER *prAdapter,
		uint8_t u2Port);
#endif
#if (CFG_COALESCING_INTERRUPT == 1)
	uint32_t (*setWfdmaCoalescingInt)(struct ADAPTER *prAdapter,
		u_int8_t fgEnable);
#endif
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

/*******************************************************************************
 *                   F U N C T I O N   D E C L A R A T I O N S
 *******************************************************************************
 */

uint32_t glRegisterBus(probe_card pfProbe, remove_card pfRemove);

void glUnregisterBus(remove_card pfRemove);

void glSetHifInfo(struct GLUE_INFO *prGlueInfo, unsigned long ulCookie);

void glClearHifInfo(struct GLUE_INFO *prGlueInfo);

void glResetHifInfo(struct GLUE_INFO *prGlueInfo);

u_int8_t glBusInit(void *pvData);

void glBusRelease(void *pData);

int32_t glBusSetIrq(void *pvData, void *pfnIsr, void *pvCookie);

void glBusFreeIrq(void *pvData, void *pvCookie);

void glSetPowerState(IN struct GLUE_INFO *prGlueInfo, IN uint32_t ePowerMode);

void glGetDev(void *ctx, struct device **dev);

void glGetHifDev(struct GL_HIF_INFO *prHif, struct device **dev);

void halPciePreSuspendDone(IN struct ADAPTER *prAdapter,
	IN struct CMD_INFO *prCmdInfo, IN uint8_t *pucEventBuf);
void halPciePreSuspendTimeout(IN struct ADAPTER *prAdapter,
	IN struct CMD_INFO *prCmdInfo);
void halPciePreSuspendCmd(IN struct ADAPTER *prAdapter);
void halPcieResumeCmd(IN struct ADAPTER *prAdapter);
#if CFG_SUPPORT_PCIE_ASPM
bool glBusConfigASPM(struct pci_dev *dev, int val);
bool glBusConfigASPML1SS(struct pci_dev *dev, int enable);
#endif

/*******************************************************************************
 *                              F U N C T I O N S
 *******************************************************************************
 */
#endif /* _HIF_H */
