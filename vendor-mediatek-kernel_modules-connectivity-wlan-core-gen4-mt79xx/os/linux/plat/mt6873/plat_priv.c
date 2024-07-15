/* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause */
/*
 * Copyright (c) 2019 MediaTek Inc.
 */

#include "precomp.h"

#ifdef CONFIG_MEDIATEK_EMI
#include <memory/mediatek/emi.h>
#define WIFI_EMI_MEM_OFFSET    0x1D0000
#define WIFI_EMI_MEM_SIZE      0x140000
#define	DOMAIN_AP	0
#define	DOMAIN_CONN	2
#endif

#ifdef CONFIG_MEDIATEK_EMI
void kalSetEmiMpuProtection(phys_addr_t emiPhyBase, bool enable)
{
	struct emimpu_region_t region;
	unsigned long long start = emiPhyBase + WIFI_EMI_MEM_OFFSET;
	unsigned long long end = emiPhyBase + WIFI_EMI_MEM_OFFSET +
			WIFI_EMI_MEM_SIZE - 1;
	int ret;

	DBGLOG(INIT, INFO, "emiPhyBase: 0x%p, enable: %d\n",
				emiPhyBase, enable);

	ret = mtk_emimpu_init_region(&region, 26);
	if (ret) {
		DBGLOG(INIT, ERROR, "mtk_emimpu_init_region failed, ret: %d\n",
				ret);
		return;
	}
	mtk_emimpu_set_addr(&region, start, end);
	mtk_emimpu_set_apc(&region, DOMAIN_AP, MTK_EMIMPU_NO_PROTECTION);
	mtk_emimpu_set_apc(&region, DOMAIN_CONN, MTK_EMIMPU_NO_PROTECTION);
	mtk_emimpu_lock_region(&region,
			enable ? MTK_EMIMPU_LOCK : MTK_EMIMPU_UNLOCK);
	ret = mtk_emimpu_set_protection(&region);
	if (ret)
		DBGLOG(INIT, ERROR,
			"mtk_emimpu_set_protection failed, ret: %d\n",
			ret);
	mtk_emimpu_free_region(&region);
}

void kalSetDrvEmiMpuProtection(phys_addr_t emiPhyBase, uint32_t offset,
			       uint32_t size)
{
	struct emimpu_region_t region;
	unsigned long long start = emiPhyBase + offset;
	unsigned long long end = emiPhyBase + offset + size - 1;
	int ret;

	DBGLOG(INIT, INFO, "emiPhyBase: 0x%p, offset: %d, size: %d\n",
				emiPhyBase, offset, size);

	ret = mtk_emimpu_init_region(&region, 18);
	if (ret) {
		DBGLOG(INIT, ERROR, "mtk_emimpu_init_region failed, ret: %d\n",
				ret);
		return;
	}
	mtk_emimpu_set_addr(&region, start, end);
	mtk_emimpu_set_apc(&region, DOMAIN_AP, MTK_EMIMPU_NO_PROTECTION);
	mtk_emimpu_set_apc(&region, DOMAIN_CONN, MTK_EMIMPU_NO_PROTECTION);
	mtk_emimpu_lock_region(&region, MTK_EMIMPU_LOCK);
	ret = mtk_emimpu_set_protection(&region);
	if (ret)
		DBGLOG(INIT, ERROR,
			"mtk_emimpu_set_protection failed, ret: %d\n",
			ret);
	mtk_emimpu_free_region(&region);
}
#endif

int32_t kalGetFwFlavor(uint8_t *flavor)
{
	*flavor = 'b';
	return 1;
}
