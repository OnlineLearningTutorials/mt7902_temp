/* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause */
/*
 * Copyright (c) 2016 MediaTek Inc.
 */

/*! \file  soc3_0.h
*    \brief This file contains the info of soc3_0
*/

#ifdef SOC3_0

#ifndef _SOC3_0_H
#define _SOC3_0_H
#if (CFG_SUPPORT_CONNINFRA == 1)
#include "conninfra.h"
#endif
/*******************************************************************************
*                         C O M P I L E R   F L A G S
********************************************************************************
*/

/*******************************************************************************
*                    E X T E R N A L   R E F E R E N C E S
********************************************************************************
*/
#if (CFG_MTK_ANDROID_EMI == 1)
extern phys_addr_t gConEmiPhyBase;
extern unsigned long long gConEmiSize;
#endif

/*******************************************************************************
*                              C O N S T A N T S
********************************************************************************
*/
#define CONNAC2X_TOP_HCR 0x88000000  /*no use, set HCR = HVR*/
#define CONNAC2X_TOP_HVR 0x88000000
#define CONNAC2X_TOP_FVR 0x88000004
#define CONNAC2x_CONN_CFG_ON_BASE	0x7C060000
#define CONNAC2x_CONN_CFG_ON_CONN_ON_MISC_ADDR \
	(CONNAC2x_CONN_CFG_ON_BASE + 0xF0)
#define CONNAC2x_CONN_CFG_ON_CONN_ON_MISC_DRV_FM_STAT_SYNC_SHFT         0

#define SOC3_0_CHIP_ID                 (0x7915)
#define SOC3_0_SW_SYNC0                CONNAC2x_CONN_CFG_ON_CONN_ON_MISC_ADDR
#define SOC3_0_SW_SYNC0_RDY_OFFSET \
	CONNAC2x_CONN_CFG_ON_CONN_ON_MISC_DRV_FM_STAT_SYNC_SHFT
#define SOC3_0_PATCH_START_ADDR        (0x00100000)
#define SOC3_0_TOP_CFG_BASE			CONN_CFG_BASE
#define SOC3_0_TX_DESC_APPEND_LENGTH   32
#define SOC3_0_RX_DESC_LENGTH   24
#define SOC3_0_ARB_AC_MODE_ADDR (0x820e3020)

#define CONN_HOST_CSR_TOP_BASE_ADDR 0x18060000
#define CONN_INFRA_CFG_BASE_ADDR 0x18001000
#define CONN_INFRA_RGU_BASE_ADDR 0x18000000
#define CONN_INFRA_BRCM_BASE_ADDR 0x1800E000

#define WF_TOP_MISC_OFF_BASE_ADDR 0x184B0000

#define CONN_INFRA_WAKEUP_WF_ADDR (CONN_HOST_CSR_TOP_BASE_ADDR + 0x01A4)
#define CONN_INFRA_ON2OFF_SLP_PROT_ACK_ADDR \
	(CONN_HOST_CSR_TOP_BASE_ADDR + 0x0184)
#define CONN_HW_VER_ADDR (CONN_INFRA_CFG_BASE_ADDR + 0x0000)
#define WFSYS_SW_RST_B_ADDR (CONN_INFRA_RGU_BASE_ADDR + 0x0018)
#define WFSYS_CPU_SW_RST_B_ADDR (CONN_INFRA_RGU_BASE_ADDR + 0x0010)
#define WFSYS_ON_TOP_PWR_CTL_ADDR (CONN_INFRA_RGU_BASE_ADDR + 0x0000)
#define TOP_DBG_DUMMY_3_CONNSYS_PWR_STATUS_ADDR \
	(CONN_HOST_CSR_TOP_BASE_ADDR + 0x02CC)
#define CONN_INFRA_WF_SLP_CTRL_R_ADDR (CONN_INFRA_CFG_BASE_ADDR + 0x0620)
#define CONN_INFRA_WFDMA_SLP_CTRL_R_ADDR (CONN_INFRA_CFG_BASE_ADDR + 0x0624)
#define CONN_INFRA_WFSYS_EMI_REQ_ADDR (CONN_INFRA_CFG_BASE_ADDR + 0x0c14)

#define WF_VDNR_EN_ADDR (CONN_INFRA_BRCM_BASE_ADDR + 0x6C)
#define WFSYS_VERSION_ID_ADDR (WF_TOP_MISC_OFF_BASE_ADDR + 0x10)
#define CONN_CFG_AP2WF_REMAP_1_ADDR (CONN_INFRA_CFG_BASE_ADDR + 0x0120)
#define CONN_MCU_CONFG_HS_BASE 0x89040000
#define CONNSYS_VERSION_ID  0x20010000
#define WF_DYNAMIC_BASE 0x18500000
#define MCU_EMI_ENTRY_OFFSET 0x01DC
#define WF_EMI_ENTRY_OFFSET 0x01E0

#define CONNSYS_ROM_DONE_CHECK  0x00001D1E

#define WF_ROM_CODE_INDEX_ADDR 0x184C1604

#define WF_TRIGGER_AP2CONN_EINT 0x10001F00
#define WF_CONN_INFA_BUS_CLOCK_RATE 0x1000123C


/*******************************************************************************
*                         D A T A   T Y P E S
********************************************************************************
*/

/*******************************************************************************
*                            P U B L I C   D A T A
********************************************************************************
*/
extern struct platform_device *g_prPlatDev;
#if (CFG_SUPPORT_CONNINFRA == 1)
extern u_int8_t g_IsWfsysBusHang;
extern struct completion g_triggerComp;
extern bool g_IsTriggerTimeout;
extern u_int8_t fgIsResetting;
extern u_int8_t g_fgRstRecover;
#endif
/*******************************************************************************
*                           P R I V A T E   D A T A
********************************************************************************
*/

/*******************************************************************************
*                                 M A C R O S
********************************************************************************
*/

/*******************************************************************************
*                  F U N C T I O N   D E C L A R A T I O N S
********************************************************************************
*/
void soc3_0_show_ple_info(
	struct ADAPTER *prAdapter,
	u_int8_t fgDumpTxd);
void soc3_0_show_pse_info(
	struct ADAPTER *prAdapter);

void soc3_0_show_wfdma_info(
	IN struct ADAPTER *prAdapter);

/*******************************************************************************
*                              F U N C T I O N S
********************************************************************************
*/

#if (CFG_DOWNLOAD_DYN_MEMORY_MAP == 1)
uint32_t soc3_0_DownloadByDynMemMap(IN struct ADAPTER *prAdapter,
	IN uint32_t u4Addr, IN uint32_t u4Len,
	IN uint8_t *pucStartPtr, IN enum ENUM_IMG_DL_IDX_T eDlIdx);
#endif
int hifWmmcuPwrOn(void);
int hifWmmcuPwrOff(void);
int wf_ioremap_read(size_t addr, unsigned int *val);

int wf_ioremap_write(phys_addr_t addr, unsigned int val);
int soc3_0_Trigger_fw_assert(void);
#if (CFG_SUPPORT_CONNINFRA == 1)
int soc3_0_Trigger_whole_chip_rst(char *reason);
void soc3_0_Sw_interrupt_handler(struct ADAPTER *prAdapter);
void soc3_0_Conninfra_cb_register(void);
extern void update_driver_reset_status(uint8_t fgIsResetting);
extern int32_t get_wifi_process_status(void);
extern int32_t get_wifi_powered_status(void);
#endif /*#if (CFG_SUPPORT_CONNINFRA == 1)*/
void soc3_0_icapRiseVcoreClockRate(void);
void soc3_0_icapDownVcoreClockRate(void);

#endif /* _SOC3_0_H */

#endif  /* soc3_0 */
