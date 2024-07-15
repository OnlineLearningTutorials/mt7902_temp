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

#if defined(_HIF_NONE)
#define HIF_NAME "NONE"
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

/*******************************************************************************
 *                             D A T A   T Y P E S
 *******************************************************************************
 */

struct GL_HIF_INFO;


/* host interface's private data structure, which is attached to os glue
 ** layer info structure.
 */
struct GL_HIF_INFO {
};

struct BUS_INFO {
};


/*******************************************************************************
 *                            P U B L I C   D A T A
 *******************************************************************************
 */
/* Common data type */
#ifndef HIF_NUM_OF_QM_RX_PKT_NUM
#define HIF_NUM_OF_QM_RX_PKT_NUM        512
#endif

/* chip dependent? used in wlanHarvardFormatDownload */
#define HIF_CR4_FWDL_SECTION_NUM            2
#define HIF_IMG_DL_STATUS_PORT_IDX          0

/* enable/disable TX resource control */
#define HIF_TX_RESOURCE_CTRL                1

/* enable/disable TX resource control PLE */
#define HIF_TX_RESOURCE_CTRL_PLE            0

#define HIF_IST_LOOP_COUNT              128

/* Min msdu count to trigger Tx during INT polling state */
#define HIF_IST_TX_THRESHOLD            32

#define HIF_TX_BUFF_COUNT_TC0               3
#define HIF_TX_BUFF_COUNT_TC1               3
#define HIF_TX_BUFF_COUNT_TC2               3
#define HIF_TX_BUFF_COUNT_TC3               3
#define HIF_TX_BUFF_COUNT_TC4               2
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
/* kal internal use */
#define glRegisterBus(_pfProbe, _pfRemove)

#define glUnregisterBus(_pfRemove)

#define glSetHifInfo(_prGlueInfo, _ulCookie)

#define glClearHifInfo(_prGlueInfo)

#define glResetHifInfo(_prGlueInfo)

#define glBusInit(_pvData)

#define glBusRelease(_pData)

#define glBusSetIrq(_pvData, _pfnIsr, _pvCookie)

#define glBusFreeIrq(_pvData, _pvCookie)

#define glSetPowerState(_prGlueInfo, _ePowerMode)

#define glGetDev(_prCtx, _ppDev)

#define glGetHifDev(_prHif, _ppDev)

#define HAL_WAKE_UP_WIFI(_prAdapter)

#define halWpdmaInitRing(_glueinfo) \
	KAL_NEED_IMPLEMENT(__FILE__, __func__, __LINE__)

uint8_t halTxRingDataSelect(IN struct ADAPTER *prAdapter,
	IN struct MSDU_INFO *prMsduInfo);

/*******************************************************************************
 *                              F U N C T I O N S
 *******************************************************************************
 */

/* for nic/hal.h */
/*
 * kal_virt_write_tx_port: Write frame to data port
 * @ad: structure for adapter private data
 * @pid: port id to write data, eg.NIC_TX_INIT_CMD_PORT
 * @len: data len to write
 * @buf: data buf pointer
 * @buf_size: maximum size for data buffer, buf_size >= len
 *
 * not: implementation for different HIF may refer to nic/hal.h
 */
void kal_virt_write_tx_port(struct ADAPTER *ad,
	uint16_t pid, uint32_t len, uint8_t *buf, uint32_t buf_size);

/*
 * kal_virt_get_wifi_func_stat: check HW status when check ready fail
 * @ad: structure for adapter private data
 * @res: status for return
 *
 * not: implementation for different HIF may refer to nic/hal.h
 */
void kal_virt_get_wifi_func_stat(struct ADAPTER *ad, uint32_t *res);

/*
 * kal_virt_chk_wifi_func_off: check HW status for function OFF
 * @ad: structure for adapter private data
 * @ready_bits: asserted bit if function is off correctly
 * @res: status for return
 *
 * not: implementation for different HIF may refer to nic/hal.h
 */
void kal_virt_chk_wifi_func_off(struct ADAPTER *ad, uint32_t ready_bits,
	uint8_t *res);

/*
 * kal_virt_chk_wifi_func_off: check HW status for function ready
 * @ad: structure for adapter private data
 * @ready_bits: asserted bit if function is ready
 * @res: status for return
 *
 * not: implementation for different HIF may refer to nic/hal.h
 */
void kal_virt_chk_wifi_func_ready(struct ADAPTER *ad, uint32_t ready_bits,
	uint8_t *res);

/*
 * kal_virt_set_mailbox_readclear: set read clear on hw mailbox (?)
 * @ad: structure for adapter private data
 * @enable: enable read clear or not
 *
 * not: implementation for different HIF may refer to nic/hal.h
 */
void kal_virt_set_mailbox_readclear(struct ADAPTER *ad, bool enable);

/*
 * kal_virt_set_int_stat_readclear: set hardware interrupt read clear
 * @ad: structure for adapter private data
 *
 * not: no disable command
 * not: implementation for different HIF may refer to nic/hal.h
 */
void kal_virt_set_int_stat_readclear(struct ADAPTER *ad);

/*
 * kal_virt_init_hif: do device related initialization
 * @ad: structure for adapter private data
 *
 * not: implementation for different HIF may refer to nic/hal.h
 */
void kal_virt_init_hif(struct ADAPTER *ad);

/*
 * kal_virt_enable_fwdl: enable firmware download
 * @ad: structure for adapter private data
 *
 * not: implementation for different HIF may refer to nic/hal.h
 */
void kal_virt_enable_fwdl(struct ADAPTER *ad, bool enable);

/*
 * kal_virt_get_int_status: read interrupt status
 * @ad: structure for adapter private data
 * @status: return value for interrupt status
 *
 * not: implementation for different HIF may refer to nic/hal.h
 */
void kal_virt_get_int_status(struct ADAPTER *ad, uint32_t *status);

/*
 * kal_virt_uhw_rd: read chip CR via USB UHW.
 * @ad: structure for adapter private data
 * @u4Offset: CR address
 * @pu4Value: return CR value
 * @pfgSts: return TRUE if IO operation is successful; otherwise, return FALSE
 *
 * note: implementation for different HIF may refer to nic/hal.h
 */
void kal_virt_uhw_rd(struct ADAPTER *ad, uint32_t u4Offset, uint32_t *pu4Value,
		     u_int8_t *pfgSts);

/*
 * kal_virt_uhw_wr: write chip CR via USB UHW.
 * @ad: structure for adapter private data
 * @u4Offset: CR address
 * @u4Value: CR value
 * @pfgSts: return TRUE if IO operation is successful; otherwise, return FALSE
 *
 * note: implementation for different HIF may refer to nic/hal.h
 */
void kal_virt_uhw_wr(struct ADAPTER *ad, uint32_t u4Offset, uint32_t u4Value,
		     u_int8_t *pfgSts);
#endif /* _HIF_H */
