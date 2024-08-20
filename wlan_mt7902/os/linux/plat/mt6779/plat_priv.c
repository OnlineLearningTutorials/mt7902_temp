/* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause */
/*
 * Copyright (c) 2019 MediaTek Inc.
 */

#include "precomp.h"

#ifdef CONFIG_MTK_EMI
#include <mt_emi_api.h>
#define WIFI_EMI_MEM_OFFSET    0x1D0000
#define WIFI_EMI_MEM_SIZE      0x140000
#endif

#ifdef CONFIG_MTK_EMI
void kalSetEmiMpuProtection(phys_addr_t emiPhyBase, bool enable)
{
	struct emi_region_info_t region_info;

	/*set MPU for EMI share Memory */
	region_info.start = emiPhyBase + WIFI_EMI_MEM_OFFSET;
	region_info.end = emiPhyBase + WIFI_EMI_MEM_OFFSET
		+ WIFI_EMI_MEM_SIZE - 1;
	region_info.region = 26;

	SET_ACCESS_PERMISSION(region_info.apc, enable ? LOCK : UNLOCK,
			      FORBIDDEN, FORBIDDEN,
			      FORBIDDEN, FORBIDDEN, FORBIDDEN, FORBIDDEN,
			      FORBIDDEN, FORBIDDEN, FORBIDDEN, FORBIDDEN,
			      FORBIDDEN, FORBIDDEN, FORBIDDEN, NO_PROTECTION,
			      FORBIDDEN, enable ? FORBIDDEN : NO_PROTECTION);
	emi_mpu_set_protection(&region_info);
}

void kalSetDrvEmiMpuProtection(phys_addr_t emiPhyBase, uint32_t offset,
			       uint32_t size)
{
	struct emi_region_info_t region_info;

	/*set MPU for EMI share Memory */
	region_info.start = emiPhyBase + offset;
	region_info.end = emiPhyBase + offset + size - 1;
	region_info.region = 18;
	SET_ACCESS_PERMISSION(region_info.apc, LOCK, FORBIDDEN, FORBIDDEN,
			      FORBIDDEN, FORBIDDEN, FORBIDDEN, FORBIDDEN,
			      FORBIDDEN, FORBIDDEN, FORBIDDEN, FORBIDDEN,
			      FORBIDDEN, FORBIDDEN, FORBIDDEN, NO_PROTECTION,
			      FORBIDDEN, NO_PROTECTION);
	emi_mpu_set_protection(&region_info);
}
#endif

#if (CFG_FLAVOR_FIRMWARE)
int32_t kalGetFwFlavor(uint8_t *flavor)
{
	*flavor = 'a';
	return 1;
}
#endif
