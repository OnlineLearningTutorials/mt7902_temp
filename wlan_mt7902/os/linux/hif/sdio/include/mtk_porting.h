/* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause */
/*
 * Copyright (c) 2016 MediaTek Inc.
 */
/* porting layer */
/* Android */

#ifndef _MTK_PORTING_H_
#define _MTK_PORTING_H_

#include <linux/kernel.h>	/* include stddef.h for NULL */

/* typedef void VOID, *PVOID; */

#ifndef MTK_WCN_BOOL_TRUE
#define MTK_WCN_BOOL_FALSE               ((int) 0)
#define MTK_WCN_BOOL_TRUE                ((int) 1)
#endif



/* system APIs */
/* mutex */
typedef int(*MUTEX_CREATE) (const char *const name);
typedef int32_t(*MUTEX_DESTROY) (int mtx);
typedef int32_t(*MUTEX_LOCK) (int mtx);
typedef int32_t(*MUTEX_UNLOCK) (int mtx, unsigned long flags);
/* debug */
typedef int32_t(*DBG_PRINT) (const char *str, ...);
typedef int32_t(*DBG_ASSERT) (int32_t expr, const char *file, int32_t line);
/* timer */
typedef void (*MTK_WCN_TIMER_CB) (void);
typedef int(*TIMER_CREATE) (const char *const name);
typedef int32_t(*TIMER_DESTROY) (int tmr);
typedef int32_t(*TIMER_START) (int tmr, uint32_t timeout, MTK_WCN_TIMER_CB tmr_cb, void *param);
typedef int32_t(*TIMER_STOP) (int tmr);
/* kernel lib */
typedef void *(*SYS_MEMCPY) (void *dest, const void *src, uint32_t n);
typedef void *(*SYS_MEMSET) (void *s, int32_t c, uint32_t n);
typedef int32_t(*SYS_SPRINTF) (char *str, const char *format, ...);

#endif /* _MTK_PORTING_H_ */
