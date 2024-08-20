/* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause */
/*
 * Copyright (c) 2016 MediaTek Inc.
 */
/*! \file   "connac_reg.h"
 *   \brief  The common register definition of MT6630
 *
 *   N/A
 */



#ifndef _CONNAC_REG_H
#define _CONNAC_REG_H

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
#define CONN_CFG_BASE		0x80020000

#define CONN_CFG_ON_BASE	0x81021000

#define CONN_CFG_ON_CONN_ON_MISC_ADDR	(CONN_CFG_ON_BASE + 0x140)
#define CONN_CFG_CHIP_ID_ADDR	        (CONN_CFG_BASE + 0x1010)

#define CONN_MCU_CONFG_ON_BASE			0x81030000

#define CONN_MCU_CONFG_ON_HOST_MAILBOX_WF_ADDR \
	(CONN_MCU_CONFG_ON_BASE + 0x100)

/*
 * ============================================================================
 *
 *  ---CONN_ON_MISC (0x81021000 + 0x140)---
 *
 *    HOST_LPCR_FW_OWN[0]          - (W1C)  xxx
 *    DRV_FM_STAT_SYNC[3..1]       - (RW)  xxx
 *    RBIST_MODE[4]                - (RW)  xxx
 *    RESERVED5[31..5]             - (RO) Reserved bits
 *
 * ============================================================================
 */
#define CONN_CFG_ON_CONN_ON_MISC_RBIST_MODE_ADDR \
	CONN_CFG_ON_CONN_ON_MISC_ADDR
#define CONN_CFG_ON_CONN_ON_MISC_RBIST_MODE_MASK \
	0x00000010/*RBIST_MODE[4]*/
#define CONN_CFG_ON_CONN_ON_MISC_RBIST_MODE_SHFT               4
#define CONN_CFG_ON_CONN_ON_MISC_DRV_FM_STAT_SYNC_ADDR \
	CONN_CFG_ON_CONN_ON_MISC_ADDR
#define CONN_CFG_ON_CONN_ON_MISC_DRV_FM_STAT_SYNC_MASK \
	0x0000000E/*DRV_FM_STAT_SYNC[3..1]*/
#define CONN_CFG_ON_CONN_ON_MISC_DRV_FM_STAT_SYNC_SHFT         1
#define CONN_CFG_ON_CONN_ON_MISC_HOST_LPCR_FW_OWN_ADDR \
	CONN_CFG_ON_CONN_ON_MISC_ADDR
#define CONN_CFG_ON_CONN_ON_MISC_HOST_LPCR_FW_OWN_MASK \
	0x00000001/*HOST_LPCR_FW_OWN[0]*/
#define CONN_CFG_ON_CONN_ON_MISC_HOST_LPCR_FW_OWN_SHFT         0

#define CONN_HIF_BASE                           0x7000
#define CONN_HIF_ON_LPCTL                       (CONN_HIF_BASE)
#define CONN_HIF_ON_IRQ_STAT                    (CONN_HIF_BASE + 0x4)
#define CONN_HIF_ON_IRQ_ENA                     (CONN_HIF_BASE + 0x8)
#define CONN_HIF_ON_DBGCR01                     (CONN_HIF_BASE + 0x104)

#if defined(_HIF_PCIE) || defined(_HIF_AXI)

/* MCU Interrupt Event */
#define HOST2MCU_SW_INT_SET			(PCIE_HIF_BASE + 0x0108)

/* MCU2HOST Software interrupt set */
#define MCU2HOST_SW_INT_SET			(PCIE_HIF_BASE + 0x010C)

/* MCU to Host interrupt status */
#define MCU2HOST_SW_INT_STA			(PCIE_HIF_BASE + 0x01F0)

/* MCU to Host interrupt enable */
#define MCU2HOST_SW_INT_ENA			(PCIE_HIF_BASE + 0x01F4)

#define WPDMA_PAUSE_TX_Q			(PCIE_HIF_BASE + 0x0224)

/* Configuraiton Push */
#define PCIE_DOORBELL_PUSH          (0x484)
#define CR_PCIE_CFG_SET_OWN         (0x1 << 0)
#define CR_PCIE_CFG_CLEAR_OWN       (0x1 << 1)
#endif /* _HIF_PCIE */

#if defined(_HIF_USB)
#define CONNAC_UDMA_BASE                         0x7C000000
#define CONNAC_UDMA_TX_QSEL                      (CONNAC_UDMA_BASE + 0x8)
#define CONNAC_UDMA_RESET                        (CONNAC_UDMA_BASE + 0x14)
#define CONNAC_UDMA_WLCFG_1                      (CONNAC_UDMA_BASE + 0xc)
#define CONNAC_UDMA_WLCFG_0                      (CONNAC_UDMA_BASE + 0x18)

#define UDMA_WLCFG_0_TX_BUSY_MASK               (0x1 << 31)
#define UDMA_WLCFG_0_1US_TIMER_EN_MASK          (0x1 << 20)
#define UDMA_WLCFG_0_1US_TIMER_EN(p)            (((p) & 0x1) << 20)
#define UDMA_WLCFG_0_RX_FLUSH_MASK              (0x1 << 19)
#define UDMA_WLCFG_0_TX_TIMEOUT_EN_MASK          (0x1 << 16)

#define UDMA_WLCFG_1_TX_TIMEOUT_LIMIT_MASK      (0xFFFFF << 8)
#define UDMA_WLCFG_1_TX_TIMEOUT_LIMIT(p)        (((p) & 0xFFFFF) << 8)
#define UDMA_TX_TIMEOUT_STATUS_MASK             (0x1 << 13)

#define UDMA_TX_TIMEOUT_LIMIT			(50000)

#define UDMA_TX_IDLE_MASK                       0x00003f00

#define PDMA_IF_MISC                            0x500000a8
#define PDMA_IF_MISC_TX_ENABLE_MASK             0x00000001

#define PDMA_HIF_RESET                          0x50000100
#define DPMA_HIF_LOGIC_RESET_MASK               (0x1 << 4)

#define PDMA_DEBUG_EN                           0x50000124
#define PDMA_DEBUG_STATUS                       0x50000128
#define PDMA_DEBUG_TX_STATUS_MASK               0x004c0000 /* 0x00400000 */
#define PDMA_DEBUG_DMASHDL_REQUEST_DONE_MASK    0x00100000

#define PDMA_BUSY_STATUS                        0x50000168
#define PDMA_TX_BUSY_MASK                       0x00000001

#define PDMA_TX_IDLE_WAIT_COUNT                 30
#endif /* _HIF_USB */

#if defined(_HIF_PCIE) || defined(_HIF_AXI)
#define PDMA_DEBUG_EN                           0x50000124
#define PDMA_DEBUG_STATUS                       0x50000128
#define AXI_DEBUG_DEBUG_EN                      0x5000012C
#define CONN_HIF_DEBUG_STATUS                   0x50000130
#define PDMA_DEBUG_HIF_BUSY_STATUS              0x50000138
#define PDMA_DEBUG_BUSY_STATUS                  0x50000168
#define PDMA_DEBUG_REFill                       0x5000A010

#define PDMA_AXI_DEBUG_FLAG                     0x2222
#define GALS_AXI_DEBUG_FLAG                     0x3333
#define MCU_AXI_DEBUG_FLAG                      0x4444
#define RBUS_DEBUG_FLAG                         0x118

#define WPDMA_PAUSE_TX_Q_RINGIDX_OFFSET         16
#define WPDMA_PAUSE_TX_Q_RINGIDX_MASK           0xFFFF0000
#endif /* _HIF_PCIE */


#define PLE_PKT_MAX_SIZE_MASK (0xfff << 0)
#define PLE_PKT_MAX_SIZE_NUM(p) (((p) & 0xfff) << 0)
#define GET_PLE_PKT_MAX_SIZE_NUM(p) (((p) & PLE_PKT_MAX_SIZE_MASK) >> 0)

#define PSE_PKT_MAX_SIZE_MASK (0xfff << 16)
#define PSE_PKT_MAX_SIZE_NUM(p) (((p) & 0xfff) << 16)
#define GET_PSE_PKT_MAX_SIZE_NUM(p) (((p) & PSE_PKT_MAX_SIZE_MASK) >> 16)

#define EXTRA_TXD_SIZE_FOR_TX_BYTE_COUNT         32

#define MCU_INT_PDMA0_STOP_DONE         BIT(0)
#define MCU_INT_PDMA0_INIT_DONE         BIT(1)
#define MCU_INT_SER_TRIGGER_FROM_HOST   BIT(2)
#define MCU_INT_PDMA0_RECOVERY_DONE     BIT(3)
#define MCU_INT_DRIVER_SER              BIT(4)
#define CONNAC_MCU_SW_INT BIT(29)

#define ERROR_DETECT_STOP_PDMA_WITH_FW_RELOAD   BIT(1)
#define ERROR_DETECT_STOP_PDMA                  BIT(2)
#define ERROR_DETECT_RESET_DONE                 BIT(3)
#define ERROR_DETECT_RECOVERY_DONE              BIT(4)
#define ERROR_DETECT_MCU_NORMAL_STATE           BIT(5)
#define ERROR_DETECT_TRIGGER_LP05               BIT(15)

/* Info recorded in CONNAC2X_HOST_MAILBOX_WF_ADDR
 * 1 means L1 reset is triggered when HIF is suspend.
 * 0 means no L1 reset is triggered when HIF is suspend.
 */
#define ERROR_DETECT_L1_TRIGGER_IN_SUSPEND      BIT(30)
/* Info recorded in CONNAC2X_HOST_MAILBOX_WF_ADDR
 * This bit is only meaningful when ERROR_DETECT_L1_TRIGGER_IN_SUSPEND is 1.
 * 1 means L1 reset is done after being triggered when HIF is suspend.
 * 0 means L1 reset is still on-going.
 */
#define ERROR_DETECT_L1_DONE_IN_SUSPEND         BIT(31)
#define CP_LMAC_HANG_WORKAROUND_STEP1           BIT(8)
#define CP_LMAC_HANG_WORKAROUND_STEP2           BIT(9)
#define ERROR_DETECT_LMAC_ERROR                 BIT(24)
#define ERROR_DETECT_PSE_ERROR                  BIT(25)
#define ERROR_DETECT_PLE_ERROR                  BIT(26)
#define ERROR_DETECT_PDMA_ERROR                 BIT(27)
#define ERROR_DETECT_PCIE_ERROR                 BIT(28)

#define ERROR_DETECT_MASK			\
	(ERROR_DETECT_STOP_PDMA			\
	 | ERROR_DETECT_RESET_DONE		\
	 | ERROR_DETECT_RECOVERY_DONE		\
	 | ERROR_DETECT_MCU_NORMAL_STATE)


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
#endif
