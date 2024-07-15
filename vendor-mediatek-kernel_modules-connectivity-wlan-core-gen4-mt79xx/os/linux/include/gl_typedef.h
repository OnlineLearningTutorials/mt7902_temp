/* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause */
/*
 * Copyright (c) 2016 MediaTek Inc.
 */
/*
 * Id: //Department/DaVinci/BRANCHES/MT6620_WIFI_DRIVER_V2_3/os/linux/include
 *     /gl_typedef.h#1
 */

/*! \file   gl_typedef.h
 *    \brief  Definition of basic data type(os dependent).
 *
 *    In this file we define the basic data type.
 */


#ifndef _GL_TYPEDEF_H
#define _GL_TYPEDEF_H

#if CFG_ENABLE_EARLY_SUSPEND
#include <linux/earlysuspend.h>
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
/* Define HZ of timer tick for function kalGetTimeTick() */
#define KAL_HZ                  (1000)

/* Miscellaneous Equates */
#ifndef FALSE
#define FALSE               ((u_int8_t) 0)
#define TRUE                ((u_int8_t) 1)
#endif /* FALSE */

#ifndef NULL
#if defined(__cplusplus)
#define NULL            0
#else
#define NULL            ((void *) 0)
#endif
#endif

#if CFG_ENABLE_EARLY_SUSPEND
typedef void (*early_suspend_callback) (struct early_suspend
					*h);
typedef void (*late_resume_callback) (struct early_suspend
				      *h);
#endif

/*******************************************************************************
 *                             D A T A   T Y P E S
 *******************************************************************************
 */
/* Type definition for void */

/* Type definition for Boolean */

/* Type definition for signed integers */

/* Type definition for unsigned integers */


#define OS_SYSTIME uint32_t

/* Type definition of large integer (64bits) union to be comptaible with
 * Windows definition, so we won't apply our own coding style to these data
 * types.
 * NOTE: LARGE_INTEGER must NOT be floating variable.
 * <TODO>: Check for big-endian compatibility.
 */
union LARGE_INTEGER {
	struct {
		uint32_t LowPart;
		int32_t HighPart;
	} u;
	int64_t QuadPart;
};

union ULARGE_INTEGER {
	struct {
		uint32_t LowPart;
		uint32_t HighPart;
	} u;
	uint64_t QuadPart;
};

typedef int32_t(*probe_card) (void *pvData,
			      void *pvDriverData);
typedef void(*remove_card) (void);


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
#define IN			/* volatile */
#define OUT			/* volatile */

#define __KAL_INLINE__                  inline
#define __KAL_ATTRIB_PACKED__           __attribute__((__packed__))
#define __KAL_ATTRIB_ALIGN_4__          __aligned(4)

#ifndef BIT
#define BIT(n)                          ((uint32_t) 1UL << (n))
#endif /* BIT */

#ifndef BITS
/* bits range: for example BITS(16,23) = 0xFF0000
 *   ==>  (BIT(m)-1)   = 0x0000FFFF     ~(BIT(m)-1)   => 0xFFFF0000
 *   ==>  (BIT(n+1)-1) = 0x00FFFFFF
 */
#define BITS(m, n)                       (~(BIT(m)-1) & ((BIT(n) - 1) | BIT(n)))
#endif /* BIT */

/* This macro returns the byte offset of a named field in a known structure
 *   type.
 *   _type - structure name,
 *   _field - field name of the structure
 */
#ifndef OFFSET_OF
#define OFFSET_OF(_type, _field)    ((unsigned long)&(((_type *)0)->_field))
#endif /* OFFSET_OF */

/* This macro returns the base address of an instance of a structure
 * given the type of the structure and the address of a field within the
 * containing structure.
 * _addrOfField - address of current field of the structure,
 * _type - structure name,
 * _field - field name of the structure
 */
#ifndef ENTRY_OF
#define ENTRY_OF(_addrOfField, _type, _field) \
	((_type *)((int8_t *)(_addrOfField) - \
	(int8_t *)OFFSET_OF(_type, _field)))
#endif /* ENTRY_OF */

/* This macro align the input value to the DW boundary.
 * _value - value need to check
 */
#ifndef ALIGN_4
#define ALIGN_4(_value)             (((_value) + 3) & ~3u)
#endif /* ALIGN_4 */

#ifndef ALIGN_8
#define ALIGN_8(_value)             (((_value) + 7) & ~7u)
#endif /* ALIGN_8 */

#ifndef ALIGN_16
#define ALIGN_16(_value)             (((_value) + 15) & ~15u)
#endif /* ALIGN_16 */

/* This macro check the DW alignment of the input value.
 * _value - value of address need to check
 */
#ifndef IS_ALIGN_4
#define IS_ALIGN_4(_value)          (((_value) & 0x3) ? FALSE : TRUE)
#endif /* IS_ALIGN_4 */

#ifndef IS_NOT_ALIGN_4
#define IS_NOT_ALIGN_4(_value)      (((_value) & 0x3) ? TRUE : FALSE)
#endif /* IS_NOT_ALIGN_4 */

/* This macro evaluate the input length in unit of Double Word(4 Bytes).
 * _value - value in unit of Byte, output will round up to DW boundary.
 */
#ifndef BYTE_TO_DWORD
#define BYTE_TO_DWORD(_value)       ((_value + 3) >> 2)
#endif /* BYTE_TO_DWORD */

/* This macro evaluate the input length in unit of Byte.
 * _value - value in unit of DW, output is in unit of Byte.
 */
#ifndef DWORD_TO_BYTE
#define DWORD_TO_BYTE(_value)       ((_value) << 2)
#endif /* DWORD_TO_BYTE */

#if 1				/* Little-Endian */
#define CONST_NTOHS(_x)     ntohs(_x)

#define CONST_HTONS(_x)     htons(_x)

#define NTOHS(_x)           ntohs(_x)

#define HTONS(_x)           htons(_x)

#define NTOHL(_x)           ntohl(_x)

#define HTONL(_x)           htonl(_x)

#else /* Big-Endian */

#define CONST_NTOHS(_x)

#define CONST_HTONS(_x)

#define NTOHS(_x)

#define HTONS(_x)

#endif

#define CPU_TO_LE16 cpu_to_le16
#define CPU_TO_LE32 cpu_to_le32
#define CPU_TO_LE64 cpu_to_le64

#define LE16_TO_CPU le16_to_cpu
#define LE32_TO_CPU le32_to_cpu
#define LE64_TO_CPU le64_to_cpu

#define SWAP32(x) \
	((uint32_t) (\
	(((uint32_t) (x) & (uint32_t) 0x000000ffUL) << 24) | \
	(((uint32_t) (x) & (uint32_t) 0x0000ff00UL) << 8) | \
	(((uint32_t) (x) & (uint32_t) 0x00ff0000UL) >> 8) | \
	(((uint32_t) (x) & (uint32_t) 0xff000000UL) >> 24)))

/* Endian byte swapping codes */
#ifdef __LITTLE_ENDIAN
#define LE48_TO_CPU(x) (x)
#define CPU_TO_LE48(x) (x)
#define cpu2le32(x) ((uint32_t)(x))
#define le2cpu32(x) ((uint32_t)(x))
#define cpu2be32(x) SWAP32((x))
#define be2cpu32(x) SWAP32((x))

#else


#define SWAP48(x) \
	((uint64_t)( \
	(uint64_t)(((UINT_64)(x) & (uint64_t) 0x0000000000ffULL) << 40) | \
	(uint64_t)(((UINT_64)(x) & (uint64_t) 0x00000000ff00ULL) << 24) | \
	(uint64_t)(((UINT_64)(x) & (uint64_t) 0x000000ff0000ULL) << 8) | \
	(uint64_t)(((UINT_64)(x) & (uint64_t) 0x0000ff000000ULL) >> 8) | \
	(uint64_t)(((UINT_64)(x) & (uint64_t) 0x00ff00000000ULL) >> 24) | \
	(uint64_t)(((UINT_64)(x) & (uint64_t) 0xff0000000000ULL) >> 40)))
#define LE48_TO_CPU(x) SWAP48(x)
#define CPU_TO_LE48(x) SWAP48(x)
#define cpu2le32(x) SWAP32((x))
#define le2cpu32(x) SWAP32((x))
#define cpu2be32(x) ((uint32_t)(x))
#define be2cpu32(x) ((uint32_t)(x))


#endif

/*******************************************************************************
 *                  F U N C T I O N   D E C L A R A T I O N S
 *******************************************************************************
 */

/*******************************************************************************
 *                              F U N C T I O N S
 *******************************************************************************
 */

#endif /* _GL_TYPEDEF_H */
