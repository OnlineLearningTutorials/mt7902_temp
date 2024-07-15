/* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause */
/*
 * Copyright (c) 2016 MediaTek Inc.
 */

/*! \file  soc3_0.h
*    \brief This file contains the info of soc3_0
*/

#ifdef MT7933

#ifndef _MT7933_H
#define _MT7933_H

/*******************************************************************************
*                         C O M P I L E R   F L A G S
********************************************************************************
*/

/*******************************************************************************
*                    E X T E R N A L   R E F E R E N C E S
********************************************************************************
*/

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

#define MT7933_CHIP_ID                 (0x7933)
#define MT7933_SW_SYNC0                CONNAC2x_CONN_CFG_ON_CONN_ON_MISC_ADDR
#define MT7933_SW_SYNC0_RDY_OFFSET \
	CONNAC2x_CONN_CFG_ON_CONN_ON_MISC_DRV_FM_STAT_SYNC_SHFT
#define MT7933_PATCH_START_ADDR        (0x00100000)
#define MT7933_TOP_CFG_BASE			CONN_CFG_BASE
#define MT7933_TX_DESC_APPEND_LENGTH   32
#define MT7933_RX_DESC_LENGTH   24
#define MT7933_ARB_AC_MODE_ADDR (0x820e3020)

#define CONN_HOST_CSR_TOP_BASE_ADDR 0x18060000
#define CONN_INFRA_CFG_BASE_ADDR 0x18001000
#define CONN_INFRA_RGU_BASE_ADDR 0x18000000
#define CONN_INFRA_BRCM_BASE_ADDR 0x1800E000

#define WF_TOP_MISC_OFF_BASE_ADDR 0x184B0000

#define CONN_INFRA_WAKEUP_WF_ADDR (CONN_HOST_CSR_TOP_BASE_ADDR + 0x01A4)
#define CONN_INFRA_ON2OFF_SLP_PROT_ACK_ADDR \
	(CONN_HOST_CSR_TOP_BASE_ADDR + 0x0184)
#define CONN_HW_VER_ADDR (CONN_INFRA_CFG_BASE_ADDR + 0x0000)
#define WFSYS_CPU_SW_RST_B_ADDR (CONN_INFRA_RGU_BASE_ADDR + 0x0010)
#define WFSYS_ON_TOP_PWR_CTL_ADDR (CONN_INFRA_RGU_BASE_ADDR + 0x0000)
#define TOP_DBG_DUMMY_3_CONNSYS_PWR_STATUS_ADDR \
	(CONN_HOST_CSR_TOP_BASE_ADDR + 0x02CC)
#define CONN_INFRA_WF_SLP_CTRL_R_ADDR (CONN_INFRA_CFG_BASE_ADDR + 0x0620)
#define CONN_INFRA_WFDMA_SLP_CTRL_R_ADDR (CONN_INFRA_CFG_BASE_ADDR + 0x0624)
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

#define MT7933_WFDMA_COUNT 1

#define MT7933_HOST_CSR_BASE CONN_HOST_CSR_TOP_BASE

#define MT7933_HOST_CSR_DRIVER_OWN_INFO \
	CONN_HOST_CSR_TOP_WF_BAND0_LPCTL_ADDR

#define MT7933_HOST_CSR_PC_SEL \
	CONN_HOST_CSR_TOP_WF_MCU_PC_LOG_SEL_ADDR

#define MT7933_HOST_CSR_MCU_PORG_COUNT \
	CONN_HOST_CSR_TOP_WM_MCU_PC_DBG_ADDR

#define MT7933_HOST_SET_OWN BIT(2)
/*******************************************************************************
*                         D A T A   T Y P E S
********************************************************************************
*/
enum consys_drv_type {
	CONNDRV_TYPE_BT = 0,
	CONNDRV_TYPE_FM = 1,
	CONNDRV_TYPE_GPS = 2,
	CONNDRV_TYPE_WIFI = 3,
	CONNDRV_TYPE_MAX
};

struct whole_chip_rst_cb {
	int (*pre_whole_chip_rst)(void);
	int (*post_whole_chip_rst)(void);
};

struct pre_calibration_cb {
	int (*pwr_on_cb)(void);
	int (*do_cal_cb)(void);
};

struct sub_drv_ops_cb {
	/* chip reset */
	struct whole_chip_rst_cb rst_cb;

	/* calibration */
	struct pre_calibration_cb pre_cal_cb;

	/* thermal query */
	int (*thermal_qry)(void);

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

/*******************************************************************************
*                  F U N C T I O N   D E C L A R A T I O N S
********************************************************************************
*/

#if (CFG_SUPPORT_CONNINFRA == 1)
extern int conninfra_pwr_on(enum consys_drv_type drv_type);
extern int conninfra_pwr_off(enum consys_drv_type drv_type);
extern int conninfra_sub_drv_ops_register(enum consys_drv_type drv_type,
				struct sub_drv_ops_cb *cb);
extern int conninfra_trigger_whole_chip_rst(enum consys_drv_type who,
				char *reason);
#endif
void mt7933_show_ple_info(
	struct ADAPTER *prAdapter,
	u_int8_t fgDumpTxd);

void mt7933_show_pse_info(
	struct ADAPTER *prAdapter);

void mt7933_show_wfdma_info(
	struct ADAPTER *prAdapter);

bool mt7933HalShowHostCsrInfo(
	struct ADAPTER *prAdapter);
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
#endif /*#if (CFG_SUPPORT_CONNINFRA == 1)*/
void soc3_0_icapRiseVcoreClockRate(void);
void soc3_0_icapDownVcoreClockRate(void);

#endif /* _MT7933_H */

#endif  /* MT7933 */
