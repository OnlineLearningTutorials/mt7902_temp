/* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause */
/*
 * Copyright (c) 2016 MediaTek Inc.
 */

/*******************************************************************************
 *                         C O M P I L E R   F L A G S
 *******************************************************************************
 */

/*******************************************************************************
 *                    E X T E R N A L   R E F E R E N C E S
 *******************************************************************************
 */

#include "precomp.h"

#if (CFG_SUPPORT_SINGLE_SKU == 1)
#if (CFG_SUPPORT_SINGLE_SKU_LOCAL_DB == 1)
#include "rlm_domain.h"

/***************************************************
 * Here to describe the regulatory rules of yours.
 ***************************************************
 */

/*
 * Step1. Decclare struct ieee80211_regdomain
 */
const struct ieee80211_regdomain regdom_us01 = {
	.n_reg_rules = 6,
	.reg_rules = {
	/* channels 1..11 */
	REG_RULE_LIGHT(2412-10, 2462+10, 40, 0),
	/* channels 36..48 */
	REG_RULE_LIGHT(5180-10, 5240+10, 40, 0),
	/* channels 56..64 */
	REG_RULE_LIGHT(5260-10, 5320+10, 40, KAL_RRF_DFS),
	/* channels 100..118 */
	REG_RULE_LIGHT(5500-10, 5590+10, 40, KAL_RRF_DFS),
	/* channels 132..140 */
	REG_RULE_LIGHT(5660-10, 5700+10, 40, KAL_RRF_DFS),
	/* channels 149..165 */
	REG_RULE_LIGHT(5745-10, 5825+10, 40, 0) }
};

const struct ieee80211_regdomain regdom_us = {
	.n_reg_rules = 5,
	.dfs_region = NL80211_DFS_FCC,
	.reg_rules = {
	/* channels 1..11 */
	REG_RULE_LIGHT(2412-10, 2462+10, 40, 0),
	/* channels 36..48 */
	REG_RULE_LIGHT(5180-10, 5240+10, 80, KAL_RRF_AUTO_BW),
	/* channels 52..64 */
	REG_RULE_LIGHT(5260-10, 5320+10, 80, KAL_RRF_DFS | KAL_RRF_AUTO_BW),
	/* channels 100..140 */
	REG_RULE_LIGHT(5500-10, 5720+10, 160, KAL_RRF_DFS),
	/* channels 149..165 */
	REG_RULE_LIGHT(5745-10, 5825+10, 80, 0) }
};

const struct ieee80211_regdomain regdom_cn = {
	.n_reg_rules = 4,
	.dfs_region = NL80211_DFS_FCC,
	.reg_rules = {
	/* channels 1..13 */
	REG_RULE_LIGHT(2412-10, 2472+10, 40, 0),
	/* channels 36..48 */
	REG_RULE_LIGHT(5180-10, 5240+10, 80, KAL_RRF_AUTO_BW),
	/* channels 52..64 */
	REG_RULE_LIGHT(5260-10, 5320+10, 80, KAL_RRF_DFS | KAL_RRF_AUTO_BW),
	/* channels 149..165 */
	REG_RULE_LIGHT(5745-10, 5825+10, 80, 0) }
};

const struct ieee80211_regdomain regdom_cz_nl = {
	.n_reg_rules = 5,
	.reg_rules = {
	/* channels 1..11 */
	REG_RULE_LIGHT(2412-10, 2462+10, 40, 0),
	/* channels 12,13 */
	REG_RULE_LIGHT(2467-10, 2472+10, 40, 0),
	/* channels 36..48 */
	REG_RULE_LIGHT(5180-10, 5240+10, 80, 0),
	/* channels 52..64 */
	REG_RULE_LIGHT(5260-10, 5320+10, 80, KAL_RRF_DFS),
	/* channels 100..140 */
	REG_RULE_LIGHT(5500-10, 5700+10, 160, KAL_RRF_DFS) }
};

const struct ieee80211_regdomain regdom_jp = {
	.n_reg_rules = 7,
	.dfs_region = NL80211_DFS_JP,
	.reg_rules = {
	/* channels 1..13 */
	REG_RULE_LIGHT(2412-10, 2472+10, 40, 0),
	/* channels 14 */
	REG_RULE_LIGHT(2484-10, 2484+10, 20, KAL_RRF_NO_OFDM),
	/* channels 184..196 */
	REG_RULE_LIGHT(4920-10, 4980+10, 40, 0),
	/* channels 8..16 */
	REG_RULE_LIGHT(5040-10, 5080+10, 40, 0),
	/* channels 36..48 */
	REG_RULE_LIGHT(5180-10, 5240+10, 80, KAL_RRF_AUTO_BW),
	/* channels 52..64 */
	REG_RULE_LIGHT(5260-10, 5320+10, 80, KAL_RRF_DFS | KAL_RRF_AUTO_BW),
	/* channels 100..140 */
	REG_RULE_LIGHT(5500-10, 5700+10, 160, KAL_RRF_DFS) }
};

const struct ieee80211_regdomain regdom_tr = {
	.n_reg_rules = 4,
	.dfs_region = NL80211_DFS_ETSI,
	.reg_rules = {
	/* channels 1..13 */
	REG_RULE_LIGHT(2412-10, 2472+10, 40, 0),
	/* channels 36..48 */
	REG_RULE_LIGHT(5180-10, 5240+10, 80, KAL_RRF_AUTO_BW),
	/* channels 52..64 */
	REG_RULE_LIGHT(5260-10, 5320+10, 80, KAL_RRF_DFS | KAL_RRF_AUTO_BW),
	/* channels 100..140 */
	REG_RULE_LIGHT(5500-10, 5700+10, 160, KAL_RRF_DFS) }
};

/*
 * Step2. Decclare struct mtk_regdomain
 */
const struct mtk_regdomain my_regdom_us01 = {
	.country_code = "US01",
	.prRegdRules = &regdom_us01
};

const struct mtk_regdomain my_regdom_us = {
	.country_code = "US",
	.prRegdRules = &regdom_us
};

const struct mtk_regdomain my_regdom_cn = {
	.country_code = "CN",
	.prRegdRules = &regdom_cn
};

const struct mtk_regdomain my_regdom_nl = {
	.country_code = "NL",
	.prRegdRules = &regdom_cz_nl
};

const struct mtk_regdomain my_regdom_cz = {
	.country_code = "CZ",
	.prRegdRules = &regdom_cz_nl
};

const struct mtk_regdomain my_regdom_jp = {
	.country_code = "JP",
	.prRegdRules = &regdom_jp
};

const struct mtk_regdomain my_regdom_tr = {
	.country_code = "TR",
	.prRegdRules = &regdom_tr
};

/*
 * Step3. Register to table
 */
const struct mtk_regdomain *g_prRegRuleTable[] = {
	&my_regdom_us01,
	&my_regdom_us,
	&my_regdom_cn,
	&my_regdom_nl,
	&my_regdom_cz,
	&my_regdom_jp,
	&my_regdom_tr,
	NULL /* this NULL SHOULD be at the end of the array */
};

#endif
#endif

