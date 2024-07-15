/* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause */
/*
 * Copyright (c) 2009 MediaTek Inc.
 */

#ifndef _WF_CR_SW_DEF_H
#define _WF_CR_SW_DEF_H

/*******************************************************************************
*                                 M A C R O S
********************************************************************************
*/


//****************************************************************************
//
//                     MCU_SYSRAM SW CR Definitions
//
//****************************************************************************
extern UINT_32 __sw_def_cr_begin;

#define WF_SW_DEF_CR_SER_STATUS_ADDR            ((volatile kal_uint32 *)&__sw_def_cr_begin) 	// 0x00401A00
#define WF_SW_DEF_CR_PLE_STATUS_ADDR            ((volatile kal_uint32 *)&__sw_def_cr_begin+4)
#define WF_SW_DEF_CR_PLE1_STATUS_ADDR           ((volatile kal_uint32 *)&__sw_def_cr_begin+8)
#define WF_SW_DEF_CR_PLE_AMSDU_STATUS_ADDR      ((volatile kal_uint32 *)&__sw_def_cr_begin+12)
#define WF_SW_DEF_CR_PSE_STATUS_ADDR            ((volatile kal_uint32 *)&__sw_def_cr_begin+16)
#define WF_SW_DEF_CR_PSE1_STATUS_ADDR           ((volatile kal_uint32 *)&__sw_def_cr_begin+20)
#define WF_SW_DEF_CR_LAMC_WISR6_BN0_STATUS_ADDR ((volatile kal_uint32 *)&__sw_def_cr_begin+24)
#define WF_SW_DEF_CR_LAMC_WISR6_BN1_STATUS_ADDR ((volatile kal_uint32 *)&__sw_def_cr_begin+28)
#define WF_SW_DEF_CR_LAMC_WISR7_BN0_STATUS_ADDR ((volatile kal_uint32 *)&__sw_def_cr_begin+32)
#define WF_SW_DEF_CR_LAMC_WISR7_BN1_STATUS_ADDR ((volatile kal_uint32 *)&__sw_def_cr_begin+36)
#define WF_SW_DEF_CR_USB_MCU_EVENT_ADD          ((volatile kal_uint32 *)&__sw_def_cr_begin+40)
#define WF_SW_DEF_CR_USB_HOST_ACK_ADDR          ((volatile kal_uint32 *)&__sw_def_cr_begin+44)
#define WF_SW_DEF_CR_ICAP_SPECTRUM_MODE_ADDR    ((volatile kal_uint32 *)&__sw_def_cr_begin+48)

#endif /* _WF_CR_SW_DEF_H */


