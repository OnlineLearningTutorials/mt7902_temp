/* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause */
/*
 * Copyright (c) 2016 MediaTek Inc.
 */
/*
 * Id: //Department/DaVinci/BRANCHES/MT6620_WIFI_DRIVER_V2_3/os/linux/include
 *     /gl_rst.h#1
 */

/*! \file   gl_rst.h
 *    \brief  Declaration of functions and finite state machine for
 *	    MT6620 Whole-Chip Reset Mechanism
 */

#ifndef _GL_RST_H
#define _GL_RST_H

/*******************************************************************************
 *                         C O M P I L E R   F L A G S
 *******************************************************************************
 */

/*******************************************************************************
 *                    E X T E R N A L   R E F E R E N C E S
 *******************************************************************************
 */
#include "gl_typedef.h"

#if 0
#include "mtk_porting.h"
#endif
/*******************************************************************************
 *                              C O N S T A N T S
 *******************************************************************************
 */
#if (MTK_WCN_HIF_SDIO == 1) || (MTK_WCN_HIF_AXI == 1)
#define CFG_WMT_RESET_API_SUPPORT   1
#else
#define CFG_WMT_RESET_API_SUPPORT   0
#endif

#define RST_FLAG_DO_CORE_DUMP              BIT(0)
#define RST_FLAG_PREVENT_POWER_OFF         BIT(1)
#define RST_FLAG_DO_WHOLE_RESET            BIT(2)
#define RST_FLAG_DO_L0P5_RESET             BIT(3)
#define RST_FLAG_DO_L1_RESET               BIT(4)

/*******************************************************************************
 *                             D A T A   T Y P E S
 *******************************************************************************
 */
enum ENUM_RESET_STATUS {
	RESET_FAIL,
	RESET_SUCCESS
};

/* TODO: os-related, implementation reference */
#if 0
struct RESET_STRUCT {
	enum ENUM_RESET_STATUS rst_data;
	struct work_struct rst_work;
	struct work_struct rst_trigger_work;
	uint32_t rst_trigger_flag;
};
#endif

/* duplicated from wmt_exp.h for better driver isolation */
enum ENUM_WMTDRV_TYPE {
	WMTDRV_TYPE_BT = 0,
	WMTDRV_TYPE_FM = 1,
	WMTDRV_TYPE_GPS = 2,
	WMTDRV_TYPE_WIFI = 3,
	WMTDRV_TYPE_WMT = 4,
	WMTDRV_TYPE_STP = 5,
	WMTDRV_TYPE_SDIO1 = 6,
	WMTDRV_TYPE_SDIO2 = 7,
	WMTDRV_TYPE_LPBK = 8,
	WMTDRV_TYPE_MAX
};

enum ENUM_WMTMSG_TYPE {
	WMTMSG_TYPE_POWER_ON = 0,
	WMTMSG_TYPE_POWER_OFF = 1,
	WMTMSG_TYPE_RESET = 2,
	WMTMSG_TYPE_STP_RDY = 3,
	WMTMSG_TYPE_HW_FUNC_ON = 4,
	WMTMSG_TYPE_MAX
};

enum ENUM_WMTRSTMSG_TYPE {
	WMTRSTMSG_RESET_START = 0x0,
	WMTRSTMSG_RESET_END = 0x1,
	WMTRSTMSG_RESET_END_FAIL = 0x2,
	WMTRSTMSG_RESET_MAX,
	WMTRSTMSG_RESET_INVALID = 0xff
};

enum _ENUM_CHIP_RESET_REASON_TYPE_T {
	RST_UNKNOWN = 0,
	RST_PROCESS_ABNORMAL_INT,
	RST_DRV_OWN_FAIL,
	RST_FW_ASSERT_DONE,
	RST_FW_ASSERT_TIMEOUT,
	RST_BT_TRIGGER,
	RST_OID_TIMEOUT,
	RST_CMD_TRIGGER,
	RST_CR_ACCESS_FAIL,
	RST_CMD_EVT_FAIL,
	RST_GROUP4_NULL,
	RST_TX_ERROR,
	RST_RX_ERROR,
	RST_WDT,
	RST_SER_L1_FAIL,    /* TODO */
	RST_SER_L0P5_FAIL,  /* TODO */
	RST_REASON_MAX
};

enum ENUM_WFSYS_RESET_STATE_TYPE_T {
	WFSYS_RESET_STATE_IDLE = 0,
	WFSYS_RESET_STATE_RESET,
	WFSYS_RESET_STATE_REINIT,
	WFSYS_RESET_STATE_POSTPONE,
	WFSYS_RESET_STATE_MAX
};

typedef void (*PF_WMT_CB) (enum ENUM_WMTDRV_TYPE, /* Source driver type */
			   enum ENUM_WMTDRV_TYPE, /* Destination driver type */
			   enum ENUM_WMTMSG_TYPE, /* Message type */
			   /* READ-ONLY buffer. Buffer is allocated and
			    * freed by WMT_drv. Client can't touch this
			    * buffer after this function return.
			    */
			   void *,
			   unsigned int); /* Buffer size in unit of byte */


/*******************************************************************************
 *                    E X T E R N A L   F U N C T I O N S
 *******************************************************************************
 */

#if CFG_CHIP_RESET_SUPPORT

extern int mtk_wcn_wmt_assert(enum ENUM_WMTDRV_TYPE type,
			      uint32_t reason);
extern int mtk_wcn_wmt_msgcb_reg(enum ENUM_WMTDRV_TYPE
				 eType, PF_WMT_CB pCb);
extern int mtk_wcn_wmt_msgcb_unreg(enum ENUM_WMTDRV_TYPE
				   eType);
extern int wifi_reset_start(void);
extern int wifi_reset_end(enum ENUM_RESET_STATUS);
#endif

#if CFG_ENABLE_KEYWORD_EXCEPTION_MECHANISM
extern int mtk_wcn_wmt_assert_keyword(enum ENUM_WMTDRV_TYPE type,
	unsigned char *keyword);
#endif

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
#if CFG_CHIP_RESET_SUPPORT
#if (CFG_WMT_RESET_API_SUPPORT == 1) || (CFG_SUPPORT_CONNINFRA == 1)
#define glResetSelectAction(_prAdapter)    glResetSelectActionMobile(_prAdapter)
#else
#define glResetSelectAction(_prAdapter)    glResetSelectActionCe(_prAdapter)
#endif /* CFG_WMT_RESET_API_SUPPORT || CFG_SUPPORT_CONNINFRA */

/* Each reset trigger reason has corresponding default reset action, which is
 * defined in glResetSelectAction(). You can use this macro to trigger default
 * action.
 */
#define GL_DEFAULT_RESET_TRIGGER(_prAdapter, _eReason)		\
{     \
	glSetRstReason(_eReason);    \
	glResetTrigger(_prAdapter, glResetSelectAction(_prAdapter),	\
		       (const uint8_t *)__FILE__, __LINE__);    \
}

/* You can use this macro to trigger user defined reset actions instead of the
 * default ones.
 */
#define GL_USER_DEFINE_RESET_TRIGGER(_prAdapter, _eReason, _u4Flags)    \
{     \
	glSetRstReason(_eReason);    \
	glResetTrigger(_prAdapter, _u4Flags,	\
		       (const uint8_t *)__FILE__, __LINE__);    \
}
#else
#define GL_DEFAULT_RESET_TRIGGER(_prAdapter, _eReason) \
	DBGLOG(INIT, INFO, "DO NOT support chip reset\n");

#define GL_USER_DEFINE_RESET_TRIGGER(_prAdapter, _eReason, _u4Flags)    \
	DBGLOG(INIT, INFO, "DO NOT support chip reset\n");
#endif /* CFG_CHIP_RESET_SUPPORT */

extern uint64_t u8ResetTime;
/*******************************************************************************
 *                  F U N C T I O N   D E C L A R A T I O N S
 *******************************************************************************
 */
#if CFG_WMT_RESET_API_SUPPORT
extern int mtk_wcn_set_connsys_power_off_flag(int value);
extern int mtk_wcn_wmt_assert_timeout(enum ENUM_WMTDRV_TYPE
				      type, uint32_t reason, int timeout);
extern int mtk_wcn_wmt_do_reset(enum ENUM_WMTDRV_TYPE type);
#endif

/* WMT Core Dump Support */
extern u_int8_t mtk_wcn_stp_coredump_start_get(void);

/*******************************************************************************
 *                              F U N C T I O N S
 *******************************************************************************
 */
void glResetInit(void);

void glResetUninit(void);

void glSendResetRequest(void);

u_int8_t glIsWmtCodeDump(void);

#ifdef CFG_REMIND_IMPLEMENT
#define glSetWfsysResetState(_prAdapter, _state) \
	KAL_NEED_IMPLEMENT(__FILE__, __func__, __LINE__)

#define glReSchWfsysReset(_prAdapter) \
	KAL_NEED_IMPLEMENT(__FILE__, __func__, __LINE__)

#define WfsysResetHdlr(_work) \
	KAL_NEED_IMPLEMENT(__FILE__, __func__, __LINE__)

#define glSetRstReason(_eReason) \
	KAL_NEED_IMPLEMENT(__FILE__, __func__, __LINE__)

#define glGetRstReason() \
	KAL_NEED_IMPLEMENT(__FILE__, __func__, __LINE__)

#if (CFG_WMT_RESET_API_SUPPORT == 1) || (CFG_SUPPORT_CONNINFRA == 1)
#define glResetSelectActionMobile(prAdapter) \
	KAL_NEED_IMPLEMENT(__FILE__, __func__, __LINE__)

#define glResetTriggerMobile(prAdapter, u4RstFlag) \
	KAL_NEED_IMPLEMENT(__FILE__, __func__, __LINE__)
#else
#define glResetSelectActionCe(prAdapter) \
	KAL_NEED_IMPLEMENT(__FILE__, __func__, __LINE__)n

#define glResetTriggerCe(prAdapter, u4RstFlag) \
	KAL_NEED_IMPLEMENT(__FILE__, __func__, __LINE__)n
#endif /* CFG_WMT_RESET_API_SUPPORT || CFG_SUPPORT_CONNINFRA */

#define glResetTriggerCommon(prAdapter, u4RstFlag, pucFile, u4Line) \
	KAL_NEED_IMPLEMENT(__FILE__, __func__, __LINE__)

#define glResetTrigger(prAdapter, u4RstFlag, pucFile, u4Line) \
	KAL_NEED_IMPLEMENT(__FILE__, __func__, __LINE__)

#else

void glSetWfsysResetState(struct ADAPTER *prAdapter,
			  enum ENUM_WFSYS_RESET_STATE_TYPE_T state);

u_int8_t glReSchWfsysReset(struct ADAPTER *prAdapter);

void WfsysResetHdlr(struct work_struct *work);

void glSetRstReason(enum _ENUM_CHIP_RESET_REASON_TYPE_T
		    eReason);

#if (CFG_WMT_RESET_API_SUPPORT == 1) || (CFG_SUPPORT_CONNINFRA == 1)
uint32_t glResetSelectActionMobile(IN struct ADAPTER *prAdapter);
void glResetTriggerMobile(IN struct ADAPTER *prAdapter, IN uint32_t u4RstFlag);
#else
uint32_t glResetSelectActionCe(IN struct ADAPTER *prAdapter);
void glResetTriggerCe(IN struct ADAPTER *prAdapter, IN uint32_t u4RstFlag);
#endif /* CFG_WMT_RESET_API_SUPPORT || CFG_SUPPORT_CONNINFRA */

void glResetTriggerCommon(struct ADAPTER *prAdapter, uint32_t u4RstFlag,
			  const uint8_t *pucFile, uint32_t u4Line);

u_int8_t glResetTrigger(struct ADAPTER *prAdapter,
			uint32_t u4RstFlag, const uint8_t *pucFile,
			uint32_t u4Line);
#endif

#endif /* _GL_RST_H */
