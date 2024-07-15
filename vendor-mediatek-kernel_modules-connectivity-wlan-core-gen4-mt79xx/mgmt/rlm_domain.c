/* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause */
/*
 * Copyright (c) 2016 MediaTek Inc.
 */
/*
 * Id: //Department/DaVinci/BRANCHES/MT6620_WIFI_DRIVER_V2_3/mgmt/rlm_domain.c#2
 */

/*! \file   "rlm_domain.c"
 *    \brief
 *
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
#include "rlm_txpwr_init.h"

/*******************************************************************************
 *                              C O N S T A N T S
 *******************************************************************************
 */
#if CFG_SUPPORT_DYNAMIC_PWR_LIMIT
/* dynamic tx power control */
char *g_au1TxPwrChlTypeLabel[] = {
	"NORMAL",
	"ALL",
	"RANGE",
	"2G4",
	"5G",
	"BANDEDGE_2G4",
	"BANDEDGE_5G",
	"5GBAND1",
	"5GBAND2",
	"5GBAND3",
	"5GBAND4",
#if (CFG_SUPPORT_WIFI_6G == 1)
	"6G",
	"6GBAND1",
	"6GBAND2",
	"6GBAND3",
	"6GBAND4",
#endif
};

char *g_au1TxPwrAppliedWayLabel[] = {
	"WIFION",
	"IOCTL"
};

char *g_au1TxPwrOperationLabel[] = {
	"LEVEL",
	"OFFSET"
};

#if CFG_SUPPORT_DYNAMIC_PWR_LIMIT_ANT_TAG
static struct TX_PWR_TAG_TABLE g_auTxPwrTagTable[] = {
	{
		"ALL_T",
		(POWER_ANT_BAND_NUM * POWER_ANT_NUM),
		POWER_ANT_ALL_T
	}
};
#endif
#endif

/*******************************************************************************
 *                             D A T A   T Y P E S
 *******************************************************************************
 */


/*******************************************************************************
 *                              P R I V A T E   D A T A
 *******************************************************************************
 */

#define LEGACY_SINGLE_SKU_OFFSET 0

/*******************************************************************************
 *                            P U B L I C   D A T A
 *******************************************************************************
 */
#if CFG_SUPPORT_DYNAMIC_PWR_LIMIT
/* dynamic tx power control */
char *g_au1TxPwrDefaultSetting[] = {
	"__sar;1;2;1;[6,1]",
	"__sar;2;2;1;[6,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15]",
	"__sar;3;2;1;[6-11,1]",
	"__sar;4;2;1;[6-11,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15]",
	"__sar;5;2;1;[ALL,1]",
	"__sar;6;2;1;[ALL,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15]",
	"__sar;7;2;1;[2G4,1]",
	"__sar;8;2;1;[2G4,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15]",
	"__sar;9;2;1;[5G,1]",
	"__sar;10;2;1;[5G,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15]",
	"__sar;11;2;1;[BANDEDGE2G4,1]",
	"__sar;12;2;1;[BANDEDGE2G4,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15]",
	"__sar;13;2;1;[BANDEDGE5G,1]",
	"__sar;14;2;1;[BANDEDGE5G,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15]",
	"__sar;15;2;1;[5GBAND1,1]",
	"__sar;16;2;1;[5GBAND1,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15]",
	"__sar;17;2;1;[5GBAND2,1]",
	"__sar;18;2;1;[5GBAND2,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15]",
	"__sar;19;2;1;[5GBAND3,1]",
	"__sar;20;2;1;[5GBAND3,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15]",
	"__sar;21;2;1;[5GBAND4,1]",
	"__sar;22;2;1;[5GBAND4,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15]",
	"__sar;23;2;1;[1,1][6,2][11,3]",
#if (CFG_SUPPORT_WIFI_6G == 1)
	"__sar;24;2;1;[ALL,10]<ALL_T,-1,-2,-3,-4,-5,-2,-3,-4,-5,-1,-2,-3,-4,-5,-2,-3,-4,-5>"
#else
	"__sar;24;2;1;[ALL,10]<ALL_T,-1,-2,-3,-4,-5,-1,-2,-3,-4,-5>"
#endif
};

enum ENUM_PWR_CTRL_DEBUG_TYPE {
	DEBUG_TYPE_NULL = 0,
	DEBUG_TYPE_DUMP_ALL,
	DEBUG_TYPE_DUMP_ELEMENT,
	DEBUG_TYPE_DUMP_ANT_LIMIT,

	DEBUG_TYPE_NUM
};
static enum ENUM_PWR_CTRL_DEBUG_TYPE g_ePwrCtrlDebugType;
static struct TX_PWR_CTRL_ELEMENT *pcDebugElement;
static u_int8_t fgNewDebugElement = FALSE;

#endif
/* The following country or domain shall be set from host driver.
 * And host driver should pass specified DOMAIN_INFO_ENTRY to MT6620 as
 * the channel list of being a STA to do scanning/searching AP or being an
 * AP to choose an adequate channel if auto-channel is set.
 */

/* Define mapping tables between country code and its channel set
 */
static const uint16_t g_u2CountryGroup0[] = { COUNTRY_CODE_JP };

static const uint16_t g_u2CountryGroup1[] = {
	COUNTRY_CODE_AS, COUNTRY_CODE_AI, COUNTRY_CODE_BM, COUNTRY_CODE_KY,
	COUNTRY_CODE_GU, COUNTRY_CODE_FM, COUNTRY_CODE_PR, COUNTRY_CODE_VI,
	COUNTRY_CODE_AZ, COUNTRY_CODE_BW, COUNTRY_CODE_KH, COUNTRY_CODE_CX,
	COUNTRY_CODE_CO, COUNTRY_CODE_CR, COUNTRY_CODE_GD, COUNTRY_CODE_GT,
	COUNTRY_CODE_KI, COUNTRY_CODE_LB, COUNTRY_CODE_LR, COUNTRY_CODE_MN,
	COUNTRY_CODE_AN, COUNTRY_CODE_NI, COUNTRY_CODE_PW, COUNTRY_CODE_WS,
	COUNTRY_CODE_LK, COUNTRY_CODE_TT, COUNTRY_CODE_MM
};

static const uint16_t g_u2CountryGroup2[] = {
	COUNTRY_CODE_AW, COUNTRY_CODE_LA, COUNTRY_CODE_AE, COUNTRY_CODE_UG
};

static const uint16_t g_u2CountryGroup3[] = {
	COUNTRY_CODE_AR, COUNTRY_CODE_BR, COUNTRY_CODE_HK, COUNTRY_CODE_OM,
	COUNTRY_CODE_PH, COUNTRY_CODE_SA, COUNTRY_CODE_SG, COUNTRY_CODE_ZA,
	COUNTRY_CODE_VN, COUNTRY_CODE_KR, COUNTRY_CODE_DO, COUNTRY_CODE_FK,
	COUNTRY_CODE_KZ, COUNTRY_CODE_MZ, COUNTRY_CODE_NA, COUNTRY_CODE_LC,
	COUNTRY_CODE_VC, COUNTRY_CODE_UA, COUNTRY_CODE_UZ, COUNTRY_CODE_ZW,
	COUNTRY_CODE_MP
};

static const uint16_t g_u2CountryGroup4[] = {
	COUNTRY_CODE_AT, COUNTRY_CODE_BE, COUNTRY_CODE_BG, COUNTRY_CODE_HR,
	COUNTRY_CODE_CZ, COUNTRY_CODE_DK, COUNTRY_CODE_FI, COUNTRY_CODE_FR,
	COUNTRY_CODE_GR, COUNTRY_CODE_HU, COUNTRY_CODE_IS, COUNTRY_CODE_IE,
	COUNTRY_CODE_IT, COUNTRY_CODE_LU, COUNTRY_CODE_NL, COUNTRY_CODE_NO,
	COUNTRY_CODE_PL, COUNTRY_CODE_PT, COUNTRY_CODE_RO, COUNTRY_CODE_SK,
	COUNTRY_CODE_SI, COUNTRY_CODE_ES, COUNTRY_CODE_SE, COUNTRY_CODE_CH,
	COUNTRY_CODE_GB, COUNTRY_CODE_AL, COUNTRY_CODE_AD, COUNTRY_CODE_BY,
	COUNTRY_CODE_BA, COUNTRY_CODE_VG, COUNTRY_CODE_CV, COUNTRY_CODE_CY,
	COUNTRY_CODE_EE, COUNTRY_CODE_ET, COUNTRY_CODE_GF, COUNTRY_CODE_PF,
	COUNTRY_CODE_TF, COUNTRY_CODE_GE, COUNTRY_CODE_DE, COUNTRY_CODE_GH,
	COUNTRY_CODE_GP, COUNTRY_CODE_IQ, COUNTRY_CODE_KE, COUNTRY_CODE_LV,
	COUNTRY_CODE_LS, COUNTRY_CODE_LI, COUNTRY_CODE_LT, COUNTRY_CODE_MK,
	COUNTRY_CODE_MT, COUNTRY_CODE_MQ, COUNTRY_CODE_MR, COUNTRY_CODE_MU,
	COUNTRY_CODE_YT, COUNTRY_CODE_MD, COUNTRY_CODE_MC, COUNTRY_CODE_ME,
	COUNTRY_CODE_MS, COUNTRY_CODE_RE, COUNTRY_CODE_MF, COUNTRY_CODE_SM,
	COUNTRY_CODE_SN, COUNTRY_CODE_RS, COUNTRY_CODE_TR, COUNTRY_CODE_TC,
	COUNTRY_CODE_VA, COUNTRY_CODE_EU, COUNTRY_CODE_DZ
};

static const uint16_t g_u2CountryGroup5[] = {
	COUNTRY_CODE_AU, COUNTRY_CODE_NZ, COUNTRY_CODE_EC, COUNTRY_CODE_PY,
	COUNTRY_CODE_PE, COUNTRY_CODE_TH, COUNTRY_CODE_UY
};

static const uint16_t g_u2CountryGroup6[] = { COUNTRY_CODE_RU };

static const uint16_t g_u2CountryGroup7[] = {
	COUNTRY_CODE_CL, COUNTRY_CODE_EG, COUNTRY_CODE_IN, COUNTRY_CODE_AG,
	COUNTRY_CODE_BS, COUNTRY_CODE_BH, COUNTRY_CODE_BB, COUNTRY_CODE_BN,
	COUNTRY_CODE_MV, COUNTRY_CODE_PA, COUNTRY_CODE_ZM, COUNTRY_CODE_CN
};

static const uint16_t g_u2CountryGroup8[] = { COUNTRY_CODE_MY };

static const uint16_t g_u2CountryGroup9[] = { COUNTRY_CODE_NP };

static const uint16_t g_u2CountryGroup10[] = {
	COUNTRY_CODE_IL, COUNTRY_CODE_AM, COUNTRY_CODE_KW, COUNTRY_CODE_MA,
	COUNTRY_CODE_NE, COUNTRY_CODE_TN
};

static const uint16_t g_u2CountryGroup11[] = {
	COUNTRY_CODE_JO, COUNTRY_CODE_PG
};

static const uint16_t g_u2CountryGroup12[] = { COUNTRY_CODE_AF };

static const uint16_t g_u2CountryGroup13[] = { COUNTRY_CODE_NG };

static const uint16_t g_u2CountryGroup14[] = {
	COUNTRY_CODE_PK, COUNTRY_CODE_QA, COUNTRY_CODE_BF, COUNTRY_CODE_GY,
	COUNTRY_CODE_HT, COUNTRY_CODE_JM, COUNTRY_CODE_MO, COUNTRY_CODE_MW,
	COUNTRY_CODE_RW, COUNTRY_CODE_KN, COUNTRY_CODE_TZ, COUNTRY_CODE_BD
};

static const uint16_t g_u2CountryGroup15[] = { COUNTRY_CODE_ID };

static const uint16_t g_u2CountryGroup16[] = {
	COUNTRY_CODE_AO, COUNTRY_CODE_BZ, COUNTRY_CODE_BJ, COUNTRY_CODE_BT,
	COUNTRY_CODE_BO, COUNTRY_CODE_BI, COUNTRY_CODE_CM, COUNTRY_CODE_CF,
	COUNTRY_CODE_TD, COUNTRY_CODE_KM, COUNTRY_CODE_CD, COUNTRY_CODE_CG,
	COUNTRY_CODE_CI, COUNTRY_CODE_DJ, COUNTRY_CODE_GQ, COUNTRY_CODE_ER,
	COUNTRY_CODE_FJ, COUNTRY_CODE_GA, COUNTRY_CODE_GM, COUNTRY_CODE_GN,
	COUNTRY_CODE_GW, COUNTRY_CODE_RKS, COUNTRY_CODE_KG, COUNTRY_CODE_LY,
	COUNTRY_CODE_MG, COUNTRY_CODE_ML, COUNTRY_CODE_NR, COUNTRY_CODE_NC,
	COUNTRY_CODE_ST, COUNTRY_CODE_SC, COUNTRY_CODE_SL, COUNTRY_CODE_SB,
	COUNTRY_CODE_SO, COUNTRY_CODE_SR, COUNTRY_CODE_SZ, COUNTRY_CODE_TJ,
	COUNTRY_CODE_TG, COUNTRY_CODE_TO, COUNTRY_CODE_TM, COUNTRY_CODE_TV,
	COUNTRY_CODE_VU, COUNTRY_CODE_YE
};

static const uint16_t g_u2CountryGroup17[] = {
	COUNTRY_CODE_US, COUNTRY_CODE_CA, COUNTRY_CODE_TW
};

static const uint16_t g_u2CountryGroup18[] = {
	COUNTRY_CODE_DM, COUNTRY_CODE_SV, COUNTRY_CODE_HN
};

static const uint16_t g_u2CountryGroup19[] = {
	COUNTRY_CODE_MX, COUNTRY_CODE_VE
};

static const uint16_t g_u2CountryGroup20[] = {
	COUNTRY_CODE_CK, COUNTRY_CODE_CU, COUNTRY_CODE_TL, COUNTRY_CODE_FO,
	COUNTRY_CODE_GI, COUNTRY_CODE_GG, COUNTRY_CODE_IR, COUNTRY_CODE_IM,
	COUNTRY_CODE_JE, COUNTRY_CODE_KP, COUNTRY_CODE_MH, COUNTRY_CODE_NU,
	COUNTRY_CODE_NF, COUNTRY_CODE_PS, COUNTRY_CODE_PN, COUNTRY_CODE_PM,
	COUNTRY_CODE_SS, COUNTRY_CODE_SD, COUNTRY_CODE_SY
};

#if (CFG_SUPPORT_SINGLE_SKU == 1)
struct mtk_regd_control g_mtk_regd_control = {
	.en = FALSE,
	.state = REGD_STATE_UNDEFINED
};

#if CFG_SUPPORT_BW160
#define BW_5G 160
#else
#define BW_5G 80
#endif

#if (CFG_SUPPORT_SINGLE_SKU_LOCAL_DB == 1)
#if (CFG_SUPPORT_WIFI_6G == 1)
const struct ieee80211_regdomain default_regdom_ww = {
	.n_reg_rules = 5,
	.alpha2 = "99",
	.reg_rules = {
	/* channels 1..13 */
	REG_RULE_LIGHT(2412-10, 2472+10, 40, 0),
	/* channels 14 */
	REG_RULE_LIGHT(2484-10, 2484+10, 20, 0),
	/* channel 36..64 */
	REG_RULE_LIGHT(5150-10, 5350+10, BW_5G, 0),
	/* channel 100..165 */
	REG_RULE_LIGHT(5470-10, 5850+10, BW_5G, 0),
	/* 6G channel 1..17 */
	REG_RULE_LIGHT(5935-10, 7135+10, BW_5G, 0),
	}
};
#else
const struct ieee80211_regdomain default_regdom_ww = {
	.n_reg_rules = 4,
	.alpha2 = "99",
	.reg_rules = {
	/* channels 1..13 */
	REG_RULE_LIGHT(2412-10, 2472+10, 40, 0),
	/* channels 14 */
	REG_RULE_LIGHT(2484-10, 2484+10, 20, 0),
	/* channel 36..64 */
	REG_RULE_LIGHT(5150-10, 5350+10, BW_5G, 0),
	/* channel 100..165 */
	REG_RULE_LIGHT(5470-10, 5850+10, BW_5G, 0),
	}
};
#endif
#endif

struct TX_PWR_LIMIT_SECTION {
	uint8_t ucSectionNum;
	const char *arSectionNames[TX_PWR_LIMIT_SECTION_NUM];
} gTx_Pwr_Limit_Section[] = {
	{5,
	 {"legacy", "ht20", "ht40", "vht20", "offset"}
	},
	{9,
	 {"cck", "ofdm", "ht20", "ht40", "vht20", "vht40",
	  "vht80", "vht160", "txbf_backoff"}
	},
	{15,
	 {"cck", "ofdm", "ht20", "ht40", "vht20", "vht40", "vht80",
	  "vht160", "ru26", "ru52", "ru106", "ru242",
	  "ru484", "ru996", "ru996x2"}
	},
};

struct TX_LEGACY_PWR_LIMIT_SECTION {
	uint8_t ucLegacySectionNum;
	const char *arLegacySectionNames[TX_LEGACY_PWR_LIMIT_SECTION_NUM];
} gTx_Legacy_Pwr_Limit_Section[] = {
	{4,
	 {"ofdm", "ofdm40", "ofdm80", "ofdm160"}
	},
	{4,
	 {"ofdm", "ofdm40", "ofdm80", "ofdm160"}
	},
	{4,
	 {"ofdm", "ofdm40", "ofdm80", "ofdm160"}
	},
};

const u8 gTx_Pwr_Limit_Element_Num[][TX_PWR_LIMIT_SECTION_NUM] = {
	{7, 6, 7, 7, 5},
	{POWER_LIMIT_SKU_CCK_NUM, POWER_LIMIT_SKU_OFDM_NUM,
	 POWER_LIMIT_SKU_HT20_NUM, POWER_LIMIT_SKU_HT40_NUM,
	 POWER_LIMIT_SKU_VHT20_NUM, POWER_LIMIT_SKU_VHT40_NUM,
	 POWER_LIMIT_SKU_VHT80_NUM, POWER_LIMIT_SKU_VHT160_NUM,
	 POWER_LIMIT_TXBF_BACKOFF_PARAM_NUM},
	{POWER_LIMIT_SKU_CCK_NUM, POWER_LIMIT_SKU_OFDM_NUM,
	 POWER_LIMIT_SKU_HT20_NUM, POWER_LIMIT_SKU_HT40_NUM,
	 POWER_LIMIT_SKU_VHT20_2_NUM, POWER_LIMIT_SKU_VHT40_2_NUM,
	 POWER_LIMIT_SKU_VHT80_2_NUM, POWER_LIMIT_SKU_VHT160_2_NUM,
	 POWER_LIMIT_SKU_RU26_NUM, POWER_LIMIT_SKU_RU52_NUM,
	 POWER_LIMIT_SKU_RU106_NUM, POWER_LIMIT_SKU_RU242_NUM,
	 POWER_LIMIT_SKU_RU484_NUM, POWER_LIMIT_SKU_RU996_NUM,
	 POWER_LIMIT_SKU_RU996X2_NUM},
};


const u8 gTx_Legacy_Pwr_Limit_Element_Num[][TX_LEGACY_PWR_LIMIT_SECTION_NUM] = {
	{POWER_LIMIT_SKU_OFDM_NUM, POWER_LIMIT_SKU_OFDM40_NUM,
	 POWER_LIMIT_SKU_OFDM80_NUM, POWER_LIMIT_SKU_OFDM160_NUM},
	{POWER_LIMIT_SKU_OFDM_NUM, POWER_LIMIT_SKU_OFDM40_NUM,
	 POWER_LIMIT_SKU_OFDM80_NUM, POWER_LIMIT_SKU_OFDM160_NUM},
	{POWER_LIMIT_SKU_OFDM_NUM, POWER_LIMIT_SKU_OFDM40_NUM,
	 POWER_LIMIT_SKU_OFDM80_NUM, POWER_LIMIT_SKU_OFDM160_NUM},
};

const char *gTx_Pwr_Limit_Element[]
	[TX_PWR_LIMIT_SECTION_NUM]
	[TX_PWR_LIMIT_ELEMENT_NUM] = {
	{
		{"cck1_2", "cck_5_11", "ofdm6_9", "ofdm12_18", "ofdm24_36",
		 "ofdm48", "ofdm54"},
		{"mcs0_8", "mcs1_2_9_10", "mcs3_4_11_12", "mcs5_13", "mcs6_14",
		 "mcs7_15"},
		{"mcs0_8", "mcs1_2_9_10", "mcs3_4_11_12", "mcs5_13", "mcs6_14",
		 "mcs7_15", "mcs32"},
		{"mcs0", "mcs1_2", "mcs3_4", "mcs5_6", "mcs7", "mcs8", "mcs9"},
		{"lg40", "lg80", "vht40", "vht80", "vht160nc"},
	},
	{
		{"c1", "c2", "c5", "c11"},
		{"o6", "o9", "o12", "o18",
		 "o24", "o36", "o48", "o54"},
		{"m0", "m1", "m2", "m3", "m4", "m5", "m6", "m7"},
		{"m0", "m1", "m2", "m3", "m4", "m5", "m6", "m7", "m32"},
		{"m0", "m1", "m2", "m3", "m4", "m5", "m6", "m7", "m8", "m9"},
		{"m0", "m1", "m2", "m3", "m4", "m5", "m6", "m7", "m8", "m9"},
		{"m0", "m1", "m2", "m3", "m4", "m5", "m6", "m7", "m8", "m9"},
		{"m0", "m1", "m2", "m3", "m4", "m5", "m6", "m7", "m8", "m9"},
		{"2to1"},
	},
	{
		{"c1", "c2", "c5", "c11"},
		{"o6", "o9", "o12", "o18", "o24", "o36", "o48", "o54"},
		{"m0", "m1", "m2", "m3", "m4", "m5", "m6", "m7"},
		{"m0", "m1", "m2", "m3", "m4", "m5", "m6", "m7", "m32"},
		{"m0", "m1", "m2", "m3", "m4", "m5", "m6", "m7", "m8", "m9",
		 "rsvd", "rsvd"},
		{"m0", "m1", "m2", "m3", "m4", "m5", "m6", "m7", "m8", "m9",
		 "rsvd", "rsvd"},
		{"m0", "m1", "m2", "m3", "m4", "m5", "m6", "m7", "m8", "m9",
		 "rsvd", "rsvd"},
		{"m0", "m1", "m2", "m3", "m4", "m5", "m6", "m7", "m8", "m9",
		 "rsvd", "rsvd"},
		{"m0", "m1", "m2", "m3", "m4", "m5", "m6", "m7", "m8", "m9",
		 "m10", "m11"},
		{"m0", "m1", "m2", "m3", "m4", "m5", "m6", "m7", "m8", "m9",
		 "m10", "m11"},
		{"m0", "m1", "m2", "m3", "m4", "m5", "m6", "m7", "m8", "m9",
		 "m10", "m11"},
		{"m0", "m1", "m2", "m3", "m4", "m5", "m6", "m7", "m8", "m9",
		 "m10", "m11"},
		{"m0", "m1", "m2", "m3", "m4", "m5", "m6", "m7", "m8", "m9",
		 "m10", "m11"},
		{"m0", "m1", "m2", "m3", "m4", "m5", "m6", "m7", "m8", "m9",
		 "m10", "m11"},
		{"m0", "m1", "m2", "m3", "m4", "m5", "m6", "m7", "m8", "m9",
		 "m10", "m11"},
	},
};

const char *gTx_Legacy_Pwr_Limit_Element[]
	[TX_LEGACY_PWR_LIMIT_SECTION_NUM]
	[TX_PWR_LIMIT_ELEMENT_NUM] = {
	{
		{"o6", "o9", "o12", "o18", "o24", "o36", "o48", "o54"},
		{"o6", "o9", "o12", "o18", "o24", "o36", "o48", "o54"},
		{"o6", "o9", "o12", "o18", "o24", "o36", "o48", "o54"},
		{"o6", "o9", "o12", "o18", "o24", "o36", "o48", "o54"},
	},
	{
		{"o6", "o9", "o12", "o18", "o24", "o36", "o48", "o54"},
		{"o6", "o9", "o12", "o18", "o24", "o36", "o48", "o54"},
		{"o6", "o9", "o12", "o18", "o24", "o36", "o48", "o54"},
		{"o6", "o9", "o12", "o18", "o24", "o36", "o48", "o54"},
	},
	{
		{"o6", "o9", "o12", "o18", "o24", "o36", "o48", "o54"},
		{"o6", "o9", "o12", "o18", "o24", "o36", "o48", "o54"},
		{"o6", "o9", "o12", "o18", "o24", "o36", "o48", "o54"},
		{"o6", "o9", "o12", "o18", "o24", "o36", "o48", "o54"},
	},
};

static const int8_t gTx_Pwr_Limit_2g_Ch[] = {
	1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14};
static const int8_t gTx_Pwr_Limit_5g_Ch[] = {
	36, 38, 40, 42, 44, 46, 48, 50, 52, 54, 56, 58, 60, 62, 64, 100, 102,
	104, 106, 108, 110, 112, 114, 116, 118, 120, 122, 124, 126, 128, 132,
	134, 136, 138, 140, 142, 144, 149, 151, 153, 155, 157, 159, 161, 165};
#if (CFG_SUPPORT_WIFI_6G == 1)
static const int8_t gTx_Pwr_Limit_6g_Ch[] = {
	1, 3, 5, 7, 9, 11, 13, 15, 17, 19, 21, 23, 25, 27, 29, 33, 35,
	37, 39,	41, 43, 45, 47, 49, 51, 53, 55, 57, 59, 61, 65, 67, 69,
	71, 73, 75, 77, 79, 81, 83, 85, 87, 89, 91, 93, 97, 99, 101, 103,
	105, 107, 109, 111, 113, 115, 117, 119, 121, 123, 125, 129, 131,
	133, 135, 137, 139, 141, 143, 145, 147, 149, 151, 153, 155, 157,
	161, 163, 165, 167, 169, 171, 173, 175, 177, 179, 181, 183, 185,
	187, 189, 193, 195, 197, 199, 201, 203, 205, 207, 209, 211, 213,
	215, 217, 219, 221, 225, 227, 229, 233};
#endif

#define TX_PWR_LIMIT_2G_CH_NUM (ARRAY_SIZE(gTx_Pwr_Limit_2g_Ch))
#define TX_PWR_LIMIT_5G_CH_NUM (ARRAY_SIZE(gTx_Pwr_Limit_5g_Ch))
#if (CFG_SUPPORT_WIFI_6G == 1)
#define TX_PWR_LIMIT_6G_CH_NUM (ARRAY_SIZE(gTx_Pwr_Limit_6g_Ch))
#endif

/* 5G BAND1(UNII-1): 5150MHz ~ 5250MHz*/
#define TX_PWR_LIMIT_5G_BAND1_L	36
#define TX_PWR_LIMIT_5G_BAND1_H	50
/* 5G BAND2(UNII-2a): 5250MHz ~ 5350MHz*/
#define TX_PWR_LIMIT_5G_BAND2_L	52
#define TX_PWR_LIMIT_5G_BAND2_H	64
/* 5G BAND3(UNII-2c): 5470MHz ~ 5725MHz*/
#define TX_PWR_LIMIT_5G_BAND3_L	100
#define TX_PWR_LIMIT_5G_BAND3_H	144
/* 5G BAND4(UNII-3): 5725MHz ~ 5850MHz*/
#define TX_PWR_LIMIT_5G_BAND4_L	146
#define TX_PWR_LIMIT_5G_BAND4_H	165
#if (CFG_SUPPORT_WIFI_6G == 1)
/* 6G BAND1(UNII-5): 5925MHz ~ 6425MHz*/
#define TX_PWR_LIMIT_6G_BAND1_L	1
#define TX_PWR_LIMIT_6G_BAND1_H	93
/* 6G BAND2(UNII-6): 6425MHz ~ 6525MHz*/
#define TX_PWR_LIMIT_6G_BAND2_L	97
#define TX_PWR_LIMIT_6G_BAND2_H	115
/* 6G BAND3(UNII-7): 6525MHz ~ 6875MHz*/
#define TX_PWR_LIMIT_6G_BAND3_L	117
#define TX_PWR_LIMIT_6G_BAND3_H	185
/* 6G BAND4(UNII-8): 6875MHz ~ 7125MHz*/
#define TX_PWR_LIMIT_6G_BAND4_L	187
#define TX_PWR_LIMIT_6G_BAND4_H	233
#endif

u_int8_t g_bTxBfBackoffExists = FALSE;

#endif

struct DOMAIN_INFO_ENTRY arSupportedRegDomains[] = {
	{
		(uint16_t *) g_u2CountryGroup0, sizeof(g_u2CountryGroup0) / 2,
		{
			{81, BAND_2G4, CHNL_SPAN_5, 1, 13, FALSE}
			,			/* CH_SET_2G4_1_13 */

			{82, BAND_2G4, CHNL_SPAN_5, 14, 1, FALSE}
			,			/* CH_SET_2G4_14_14 */
			{115, BAND_5G, CHNL_SPAN_20, 36, 4, FALSE}
			,			/* CH_SET_UNII_LOW_36_48 */
			{118, BAND_5G, CHNL_SPAN_20, 52, 4, TRUE}
			,			/* CH_SET_UNII_MID_52_64 */
			{121, BAND_5G, CHNL_SPAN_20, 100, 11, TRUE}
			,			/* CH_SET_UNII_WW_100_140 */
			{125, BAND_NULL, 0, 0, 0, FALSE}
				/* CH_SET_UNII_UPPER_NA */
		}
	}
	,
	{
		(uint16_t *) g_u2CountryGroup1, sizeof(g_u2CountryGroup1) / 2,
		{
			{81, BAND_2G4, CHNL_SPAN_5, 1, 13, FALSE}
			,			/* CH_SET_2G4_1_13 */

			{115, BAND_5G, CHNL_SPAN_20, 36, 4, FALSE}
			,			/* CH_SET_UNII_LOW_36_48 */
			{118, BAND_5G, CHNL_SPAN_20, 52, 4, TRUE}
			,			/* CH_SET_UNII_MID_52_64 */
			{121, BAND_5G, CHNL_SPAN_20, 100, 12, TRUE}
			,			/* CH_SET_UNII_WW_100_144 */
			{125, BAND_5G, CHNL_SPAN_20, 149, 5, FALSE}
			,			/* CH_SET_UNII_UPPER_149_165 */
			{0, BAND_NULL, 0, 0, 0, FALSE}
		}
	}
	,
	{
		(uint16_t *) g_u2CountryGroup2, sizeof(g_u2CountryGroup2) / 2,
		{
			{81, BAND_2G4, CHNL_SPAN_5, 1, 13, FALSE}
			,			/* CH_SET_2G4_1_13 */

			{115, BAND_5G, CHNL_SPAN_20, 36, 4, FALSE}
			,			/* CH_SET_UNII_LOW_36_48 */
			{118, BAND_5G, CHNL_SPAN_20, 52, 4, TRUE}
			,			/* CH_SET_UNII_MID_52_64 */
			{121, BAND_5G, CHNL_SPAN_20, 100, 12, TRUE}
			,			/* CH_SET_UNII_WW_100_144 */
			{125, BAND_5G, CHNL_SPAN_20, 149, 4, FALSE}
			,			/* CH_SET_UNII_UPPER_149_161 */
			{0, BAND_NULL, 0, 0, 0, FALSE}
		}
	}
	,
	{
		(uint16_t *) g_u2CountryGroup3, sizeof(g_u2CountryGroup3) / 2,
		{
			{81, BAND_2G4, CHNL_SPAN_5, 1, 13, FALSE}
			,			/* CH_SET_2G4_1_13 */

			{115, BAND_5G, CHNL_SPAN_20, 36, 4, FALSE}
			,			/* CH_SET_UNII_LOW_36_48 */
			{118, BAND_5G, CHNL_SPAN_20, 52, 4, TRUE}
			,			/* CH_SET_UNII_MID_52_64 */
			{121, BAND_5G, CHNL_SPAN_20, 100, 11, TRUE}
			,			/* CH_SET_UNII_WW_100_140 */
			{125, BAND_5G, CHNL_SPAN_20, 149, 5, FALSE}
			,			/* CH_SET_UNII_UPPER_149_165 */
			{0, BAND_NULL, 0, 0, 0, FALSE}
		}
	}
	,
	{
		(uint16_t *) g_u2CountryGroup4, sizeof(g_u2CountryGroup4) / 2,
		{
			{81, BAND_2G4, CHNL_SPAN_5, 1, 13, FALSE}
			,			/* CH_SET_2G4_1_13 */

			{115, BAND_5G, CHNL_SPAN_20, 36, 4, FALSE}
			,			/* CH_SET_UNII_LOW_36_48 */
			{118, BAND_5G, CHNL_SPAN_20, 52, 4, TRUE}
			,			/* CH_SET_UNII_MID_52_64 */
			{121, BAND_5G, CHNL_SPAN_20, 100, 11, TRUE}
			,			/* CH_SET_UNII_WW_100_140 */
			{125, BAND_NULL, 0, 0, 0, FALSE}
			,			/* CH_SET_UNII_UPPER_NA */
			{0, BAND_NULL, 0, 0, 0, FALSE}
		}
	}
	,
	{
		(uint16_t *) g_u2CountryGroup5, sizeof(g_u2CountryGroup5) / 2,
		{
			{81, BAND_2G4, CHNL_SPAN_5, 1, 13, FALSE}
			,			/* CH_SET_2G4_1_13 */

			{115, BAND_5G, CHNL_SPAN_20, 36, 4, FALSE}
			,			/* CH_SET_UNII_LOW_36_48 */
			{118, BAND_5G, CHNL_SPAN_20, 52, 4, TRUE}
			,			/* CH_SET_UNII_MID_52_64 */
			{121, BAND_5G, CHNL_SPAN_20, 100, 5, TRUE}
			,			/* CH_SET_UNII_WW_100_116 */
			{121, BAND_5G, CHNL_SPAN_20, 132, 3, TRUE}
			,			/* CH_SET_UNII_WW_132_140 */
			{125, BAND_5G, CHNL_SPAN_20, 149, 5, FALSE}
				/* CH_SET_UNII_UPPER_149_165 */
		}
	}
	,
	{
		(uint16_t *) g_u2CountryGroup6, sizeof(g_u2CountryGroup6) / 2,
		{
			{81, BAND_2G4, CHNL_SPAN_5, 1, 13, FALSE}
			,			/* CH_SET_2G4_1_13 */

			{115, BAND_5G, CHNL_SPAN_20, 36, 4, FALSE}
			,			/* CH_SET_UNII_LOW_36_48 */
			{118, BAND_5G, CHNL_SPAN_20, 52, 4, TRUE}
			,			/* CH_SET_UNII_MID_52_64 */
			{121, BAND_5G, CHNL_SPAN_20, 132, 3, TRUE}
			,			/* CH_SET_UNII_WW_132_140 */
			{125, BAND_5G, CHNL_SPAN_20, 149, 5, FALSE}
			,			/* CH_SET_UNII_UPPER_149_165 */
			{0, BAND_NULL, 0, 0, 0, FALSE}
		}
	}
	,
	{
		(uint16_t *) g_u2CountryGroup7, sizeof(g_u2CountryGroup7) / 2,
		{
			{81, BAND_2G4, CHNL_SPAN_5, 1, 13, FALSE}
			,			/* CH_SET_2G4_1_13 */

			{115, BAND_5G, CHNL_SPAN_20, 36, 4, FALSE}
			,			/* CH_SET_UNII_LOW_36_48 */
			{118, BAND_5G, CHNL_SPAN_20, 52, 4, TRUE}
			,			/* CH_SET_UNII_MID_52_64 */
			{121, BAND_NULL, 0, 0, 0, FALSE}
			,			/* CH_SET_UNII_WW_NA */
			{125, BAND_5G, CHNL_SPAN_20, 149, 5, FALSE}
			,			/* CH_SET_UNII_UPPER_149_165 */
			{0, BAND_NULL, 0, 0, 0, FALSE}
		}
	}
	,
	{
		(uint16_t *) g_u2CountryGroup8, sizeof(g_u2CountryGroup8) / 2,
		{
			{81, BAND_2G4, CHNL_SPAN_5, 1, 13, FALSE}
			,			/* CH_SET_2G4_1_13 */

			{115, BAND_5G, CHNL_SPAN_20, 36, 4, FALSE}
			,			/* CH_SET_UNII_LOW_36_48 */
			{118, BAND_5G, CHNL_SPAN_20, 52, 4, TRUE}
			,			/* CH_SET_UNII_MID_52_64 */
			{121, BAND_5G, CHNL_SPAN_20, 100, 8, TRUE}
			,			/* CH_SET_UNII_WW_100_128 */
			{125, BAND_5G, CHNL_SPAN_20, 149, 5, FALSE}
			,			/* CH_SET_UNII_UPPER_149_165 */
			{0, BAND_NULL, 0, 0, 0, FALSE}
		}
	}
	,
	{
		(uint16_t *) g_u2CountryGroup9, sizeof(g_u2CountryGroup9) / 2,
		{
			{81, BAND_2G4, CHNL_SPAN_5, 1, 13, FALSE}
			,			/* CH_SET_2G4_1_13 */

			{115, BAND_5G, CHNL_SPAN_20, 36, 4, FALSE}
			,			/* CH_SET_UNII_LOW_36_48 */
			{118, BAND_5G, CHNL_SPAN_20, 52, 4, TRUE}
			,			/* CH_SET_UNII_MID_52_64 */
			{121, BAND_NULL, 0, 0, 0, FALSE}
			,			/* CH_SET_UNII_WW_NA */
			{125, BAND_5G, CHNL_SPAN_20, 149, 4, FALSE}
			,			/* CH_SET_UNII_UPPER_149_161 */
			{0, BAND_NULL, 0, 0, 0, FALSE}
		}
	}
	,
	{
		(uint16_t *) g_u2CountryGroup10, sizeof(g_u2CountryGroup10) / 2,
		{
			{81, BAND_2G4, CHNL_SPAN_5, 1, 13, FALSE}
			,			/* CH_SET_2G4_1_13 */

			{115, BAND_5G, CHNL_SPAN_20, 36, 4, FALSE}
			,			/* CH_SET_UNII_LOW_36_48 */
			{118, BAND_5G, CHNL_SPAN_20, 52, 4, TRUE}
			,			/* CH_SET_UNII_MID_52_64 */
			{121, BAND_NULL, 0, 0, 0, FALSE}
			,			/* CH_SET_UNII_WW_NA */
			{125, BAND_NULL, 0, 0, 0, FALSE}
			,			/* CH_SET_UNII_UPPER_NA */
			{0, BAND_NULL, 0, 0, 0, FALSE}
		}
	}
	,
	{
		(uint16_t *) g_u2CountryGroup11, sizeof(g_u2CountryGroup11) / 2,
		{
			{81, BAND_2G4, CHNL_SPAN_5, 1, 13, FALSE}
			,			/* CH_SET_2G4_1_13 */

			{115, BAND_5G, CHNL_SPAN_20, 36, 4, FALSE}
			,			/* CH_SET_UNII_LOW_36_48 */
			{118, BAND_NULL, 0, 0, 0, FALSE}
			,			/* CH_SET_UNII_MID_NA */
			{121, BAND_NULL, 0, 0, 0, FALSE}
			,			/* CH_SET_UNII_WW_NA */
			{125, BAND_5G, CHNL_SPAN_20, 149, 5, FALSE}
			,			/* CH_SET_UNII_UPPER_149_165 */
			{0, BAND_NULL, 0, 0, 0, FALSE}
		}
	}
	,
	{
		(uint16_t *) g_u2CountryGroup12, sizeof(g_u2CountryGroup12) / 2,
		{
			{81, BAND_2G4, CHNL_SPAN_5, 1, 13, FALSE}
			,			/* CH_SET_2G4_1_13 */

			{115, BAND_5G, CHNL_SPAN_20, 36, 4, FALSE}
			,			/* CH_SET_UNII_LOW_36_48 */
			{118, BAND_NULL, 0, 0, 0, FALSE}
			,			/* CH_SET_UNII_MID_NA */
			{121, BAND_NULL, 0, 0, 0, FALSE}
			,			/* CH_SET_UNII_WW_NA */
			{125, BAND_NULL, 0, 0, 0, FALSE}
			,			/* CH_SET_UNII_UPPER_NA */
			{0, BAND_NULL, 0, 0, 0, FALSE}
		}
	}
	,
	{
		(uint16_t *) g_u2CountryGroup13, sizeof(g_u2CountryGroup13) / 2,
		{
			{81, BAND_2G4, CHNL_SPAN_5, 1, 13, FALSE}
			,			/* CH_SET_2G4_1_13 */

			{115, BAND_NULL, 0, 0, 0, FALSE}
			,			/* CH_SET_UNII_LOW_NA */
			{118, BAND_5G, CHNL_SPAN_20, 52, 4, TRUE}
			,			/* CH_SET_UNII_MID_52_64 */
			{121, BAND_5G, CHNL_SPAN_20, 100, 11, TRUE}
			,			/* CH_SET_UNII_WW_100_140 */
			{125, BAND_5G, CHNL_SPAN_20, 149, 5, FALSE}
			,			/* CH_SET_UNII_UPPER_149_165 */
			{0, BAND_NULL, 0, 0, 0, FALSE}
		}
	}
	,
	{
		(uint16_t *) g_u2CountryGroup14, sizeof(g_u2CountryGroup14) / 2,
		{
			{81, BAND_2G4, CHNL_SPAN_5, 1, 13, FALSE}
			,			/* CH_SET_2G4_1_13 */

			{115, BAND_NULL, 0, 0, 0, FALSE}
			,			/* CH_SET_UNII_LOW_NA */
			{118, BAND_NULL, 0, 0, 0, FALSE}
			,			/* CH_SET_UNII_MID_NA */
			{121, BAND_NULL, 0, 0, 0, FALSE}
			,			/* CH_SET_UNII_WW_NA */
			{125, BAND_5G, CHNL_SPAN_20, 149, 5, FALSE}
			,			/* CH_SET_UNII_UPPER_149_165 */
			{0, BAND_NULL, 0, 0, 0, FALSE}
		}
	}
	,
	{
		(uint16_t *) g_u2CountryGroup15, sizeof(g_u2CountryGroup15) / 2,
		{
			{81, BAND_2G4, CHNL_SPAN_5, 1, 13, FALSE}
			,			/* CH_SET_2G4_1_13 */
			{115, BAND_NULL, 0, 0, 0, FALSE}
			,			/* CH_SET_UNII_LOW_NA */
			{118, BAND_NULL, 0, 0, 0, FALSE}
			,			/* CH_SET_UNII_MID_NA */
			{121, BAND_NULL, 0, 0, 0, FALSE}
			,			/* CH_SET_UNII_WW_NA */
			{125, BAND_5G, CHNL_SPAN_20, 149, 4, FALSE}
			,			/* CH_SET_UNII_UPPER_149_161 */
			{0, BAND_NULL, 0, 0, 0, FALSE}
		}
	}
	,
	{
		(uint16_t *) g_u2CountryGroup16, sizeof(g_u2CountryGroup16) / 2,
		{
			{81, BAND_2G4, CHNL_SPAN_5, 1, 13, FALSE}
			,			/* CH_SET_2G4_1_13 */
			{115, BAND_NULL, 0, 0, 0, FALSE}
			,			/* CH_SET_UNII_LOW_NA */
			{118, BAND_NULL, 0, 0, 0, FALSE}
			,			/* CH_SET_UNII_MID_NA */
			{121, BAND_NULL, 0, 0, 0, FALSE}
			,			/* CH_SET_UNII_WW_NA */
			{125, BAND_NULL, 0, 0, 0, FALSE}
			,			/* CH_SET_UNII_UPPER_NA */
			{0, BAND_NULL, 0, 0, 0, FALSE}
		}
	}
	,
	{
		(uint16_t *) g_u2CountryGroup17, sizeof(g_u2CountryGroup17) / 2,
		{
			{81, BAND_2G4, CHNL_SPAN_5, 1, 11, FALSE}
			,			/* CH_SET_2G4_1_11 */
			{115, BAND_5G, CHNL_SPAN_20, 36, 4, FALSE}
			,			/* CH_SET_UNII_LOW_36_48 */
			{118, BAND_5G, CHNL_SPAN_20, 52, 4, TRUE}
			,			/* CH_SET_UNII_MID_52_64 */
			{121, BAND_5G, CHNL_SPAN_20, 100, 12, TRUE}
			,			/* CH_SET_UNII_WW_100_144 */
			{125, BAND_5G, CHNL_SPAN_20, 149, 5, FALSE}
					/* CH_SET_UNII_UPPER_149_165 */
		}
	}
	,
	{
		(uint16_t *) g_u2CountryGroup18, sizeof(g_u2CountryGroup18) / 2,
		{
			{81, BAND_2G4, CHNL_SPAN_5, 1, 11, FALSE}
			,			/* CH_SET_2G4_1_11 */
			{115, BAND_5G, CHNL_SPAN_20, 36, 4, FALSE}
			,			/* CH_SET_UNII_LOW_36_48 */
			{118, BAND_5G, CHNL_SPAN_20, 52, 4, TRUE}
			,			/* CH_SET_UNII_MID_52_64 */
			{121, BAND_5G, CHNL_SPAN_20, 100, 5, TRUE}
			,			/* CH_SET_UNII_WW_100_116 */
			{121, BAND_5G, CHNL_SPAN_20, 132, 3, TRUE}
			,			/* CH_SET_UNII_WW_132_140 */
			{125, BAND_5G, CHNL_SPAN_20, 149, 5, FALSE}
						/* CH_SET_UNII_UPPER_149_165 */
		}
	}
	,
	{
		(uint16_t *) g_u2CountryGroup19, sizeof(g_u2CountryGroup19) / 2,
		{
			{81, BAND_2G4, CHNL_SPAN_5, 1, 11, FALSE}
			,			/* CH_SET_2G4_1_11 */
			{115, BAND_5G, CHNL_SPAN_20, 36, 4, FALSE}
			,			/* CH_SET_UNII_LOW_36_48 */
			{118, BAND_5G, CHNL_SPAN_20, 52, 4, TRUE}
			,			/* CH_SET_UNII_MID_52_64 */
			{121, BAND_NULL, 0, 0, 0, FALSE}
			,			/* CH_SET_UNII_WW_NA */
			{125, BAND_5G, CHNL_SPAN_20, 149, 5, FALSE}
			,			/* CH_SET_UNII_UPPER_149_165 */
			{0, BAND_NULL, 0, 0, 0, FALSE}
		}
	}
	,
	{
		(uint16_t *) g_u2CountryGroup20, sizeof(g_u2CountryGroup20) / 2,
		{
			{81, BAND_2G4, CHNL_SPAN_5, 1, 13, FALSE}
			,			/* CH_SET_2G4_1_13 */
			{115, BAND_5G, CHNL_SPAN_20, 36, 4, FALSE}
			,			/* CH_SET_UNII_LOW_36_48 */
			{118, BAND_5G, CHNL_SPAN_20, 52, 4, TRUE}
			,			/* CH_SET_UNII_MID_52_64 */
			{121, BAND_5G, CHNL_SPAN_20, 100, 12, TRUE}
			,			/* CH_SET_UNII_WW_100_144 */
			{125, BAND_5G, CHNL_SPAN_20, 149, 5, FALSE}
			,			/* CH_SET_UNII_UPPER_149_165 */
			{0, BAND_NULL, 0, 0, 0, FALSE}
		}
	}
	,
	{
		/* Note: Default group if no matched country code */
		NULL, 0,
		{
			{81, BAND_2G4, CHNL_SPAN_5, 1, 13, FALSE}
			,			/* CH_SET_2G4_1_13 */
			{115, BAND_5G, CHNL_SPAN_20, 36, 4, FALSE}
			,			/* CH_SET_UNII_LOW_36_48 */
			{118, BAND_5G, CHNL_SPAN_20, 52, 4, TRUE}
			,			/* CH_SET_UNII_MID_52_64 */
			{121, BAND_5G, CHNL_SPAN_20, 100, 12, TRUE}
			,			/* CH_SET_UNII_WW_100_144 */
			{125, BAND_5G, CHNL_SPAN_20, 149, 5, FALSE}
			,			/* CH_SET_UNII_UPPER_149_165 */
			{0, BAND_NULL, 0, 0, 0, FALSE}
#if (CFG_SUPPORT_WIFI_6G == 1)
			,
			{131, BAND_6G, CHNL_SPAN_80, 5, 15, FALSE}
						/* 6G_PSC_CH_SET_5_229 */
#endif
		}
	}
};

static const uint16_t g_u2CountryGroup0_Passive[] = {
	COUNTRY_CODE_TW
};

struct DOMAIN_INFO_ENTRY arSupportedRegDomains_Passive[] = {
	{
		(uint16_t *) g_u2CountryGroup0_Passive,
		sizeof(g_u2CountryGroup0_Passive) / 2,
		{
			{81, BAND_2G4, CHNL_SPAN_5, 1, 0, FALSE}
			,			/* CH_SET_2G4_1_14_NA */
			{82, BAND_2G4, CHNL_SPAN_5, 14, 0, FALSE}
			,

			{115, BAND_5G, CHNL_SPAN_20, 36, 0, FALSE}
			,			/* CH_SET_UNII_LOW_NA */
			{118, BAND_5G, CHNL_SPAN_20, 52, 4, TRUE}
			,			/* CH_SET_UNII_MID_52_64 */
			{121, BAND_5G, CHNL_SPAN_20, 100, 11, TRUE}
			,			/* CH_SET_UNII_WW_100_140 */
			{125, BAND_5G, CHNL_SPAN_20, 149, 0, FALSE}
						/* CH_SET_UNII_UPPER_NA */
#if (CFG_SUPPORT_WIFI_6G == 1)
			,
			{131, BAND_6G, CHNL_SPAN_80, 5, 15, FALSE}
						/* 6G_PSC_CH_SET_5_229 */
#endif
		}
	}
	,
	{
		/* Default passive scan channel table: ch52~64, ch100~144 */
		NULL,
		0,
		{
			{81, BAND_2G4, CHNL_SPAN_5, 1, 0, FALSE}
			,			/* CH_SET_2G4_1_14_NA */
			{82, BAND_2G4, CHNL_SPAN_5, 14, 0, FALSE}
			,

			{115, BAND_5G, CHNL_SPAN_20, 36, 0, FALSE}
			,			/* CH_SET_UNII_LOW_NA */
			{118, BAND_5G, CHNL_SPAN_20, 52, 4, TRUE}
			,			/* CH_SET_UNII_MID_52_64 */
			{121, BAND_5G, CHNL_SPAN_20, 100, 12, TRUE}
			,			/* CH_SET_UNII_WW_100_144 */
			{125, BAND_5G, CHNL_SPAN_20, 149, 0, FALSE}
						/* CH_SET_UNII_UPPER_NA */
#if (CFG_SUPPORT_WIFI_6G == 1)
			,
			{131, BAND_6G, CHNL_SPAN_80, 5, 15, FALSE}
						/* 6G_PSC_CH_SET_5_229 */
#endif
		}
	}
};

#if (CFG_SUPPORT_PWR_LIMIT_COUNTRY == 1)
struct SUBBAND_CHANNEL g_rRlmSubBand[] = {

	{BAND_2G4_LOWER_BOUND, BAND_2G4_UPPER_BOUND, 1, 0}
	,			/* 2.4G */
	{UNII1_LOWER_BOUND, UNII1_UPPER_BOUND, 2, 0}
	,			/* ch36,38,40,..,48 */
	{UNII2A_LOWER_BOUND, UNII2A_UPPER_BOUND, 2, 0}
	,			/* ch52,54,56,..,64 */
	{UNII2C_LOWER_BOUND, UNII2C_UPPER_BOUND, 2, 0}
	,			/* ch100,102,104,...,144 */
	{UNII3_LOWER_BOUND, UNII3_UPPER_BOUND, 2, 0}
				/* ch149,151,153,....,165 */
#if (CFG_SUPPORT_WIFI_6G == 1)
	,
	{UNII5_LOWER_BOUND, UNII5_UPPER_BOUND, 2, 0}
	,
	{UNII6_LOWER_BOUND, UNII6_UPPER_BOUND, 2, 0}
	,
	{UNII7_LOWER_BOUND, UNII7_UPPER_BOUND, 2, 0}
	,
	{UNII8_LOWER_BOUND, UNII8_UPPER_BOUND, 2, 0}
#endif
};
#endif
/*******************************************************************************
 *                           P R I V A T E   D A T A
 *******************************************************************************
 */

/*******************************************************************************
 *                                 M A C R O S
 *******************************************************************************
 */
#ifndef MIN
#define MIN(x, y) (((x) < (y)) ? (x) : (y))
#endif

/*******************************************************************************
 *                   F U N C T I O N   D E C L A R A T I O N S
 *******************************************************************************
 */

/*******************************************************************************
 *                              F U N C T I O N S
 *******************************************************************************
 */

/*----------------------------------------------------------------------------*/
/*!
 * \brief
 *
 * \param[in/out]
 *
 * \return none
 */
/*----------------------------------------------------------------------------*/
struct DOMAIN_INFO_ENTRY *rlmDomainGetDomainInfo(struct ADAPTER *prAdapter)
{
#define REG_DOMAIN_GROUP_NUM  \
	(sizeof(arSupportedRegDomains) / sizeof(struct DOMAIN_INFO_ENTRY))
#define REG_DOMAIN_DEF_IDX	(REG_DOMAIN_GROUP_NUM - 1)

	struct DOMAIN_INFO_ENTRY *prDomainInfo;
	struct REG_INFO *prRegInfo;
	uint16_t u2TargetCountryCode;
	uint16_t i, j;

	ASSERT(prAdapter);

	if (prAdapter->prDomainInfo)
		return prAdapter->prDomainInfo;

	prRegInfo = &prAdapter->prGlueInfo->rRegInfo;

	DBGLOG(RLM, TRACE, "eRegChannelListMap=%d, u2CountryCode=0x%04x\n",
			   prRegInfo->eRegChannelListMap,
			   prAdapter->rWifiVar.u2CountryCode);

	/*
	 * Domain info can be specified by given idx of arSupportedRegDomains
	 * table, customized, or searched by country code,
	 * only one is set among these three methods in NVRAM.
	 */
	if (prRegInfo->eRegChannelListMap == REG_CH_MAP_TBL_IDX &&
	    prRegInfo->ucRegChannelListIndex < REG_DOMAIN_GROUP_NUM) {
		/* by given table idx */
		DBGLOG(RLM, TRACE, "ucRegChannelListIndex=%d\n",
		       prRegInfo->ucRegChannelListIndex);
		prDomainInfo = &arSupportedRegDomains
					[prRegInfo->ucRegChannelListIndex];
	} else if (prRegInfo->eRegChannelListMap == REG_CH_MAP_CUSTOMIZED) {
		/* by customized */
		prDomainInfo = &prRegInfo->rDomainInfo;
	} else {
		/* by country code */
		u2TargetCountryCode =
				prAdapter->rWifiVar.u2CountryCode;

		for (i = 0; i < REG_DOMAIN_GROUP_NUM; i++) {
			prDomainInfo = &arSupportedRegDomains[i];

			if ((prDomainInfo->u4CountryNum &&
			     prDomainInfo->pu2CountryGroup) ||
			    prDomainInfo->u4CountryNum == 0) {
				for (j = 0;
				     j < prDomainInfo->u4CountryNum;
				     j++) {
					if (prDomainInfo->pu2CountryGroup[j] ==
							u2TargetCountryCode)
						break;
				}
				if (j < prDomainInfo->u4CountryNum)
					break;	/* Found */
			}
		}

		/* If no matched country code,
		 * use the default regulatory domain
		 */
		if (i >= REG_DOMAIN_GROUP_NUM) {
			DBGLOG(RLM, INFO,
			       "No matched country code, use the default regulatory domain\n");
			prDomainInfo = &arSupportedRegDomains
							[REG_DOMAIN_DEF_IDX];
		}
	}

	prAdapter->prDomainInfo = prDomainInfo;
	return prDomainInfo;
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief Retrieve the supported channel list of specified band
 *
 * \param[in/out] eSpecificBand:   BAND_2G4, BAND_5G or BAND_NULL
 *                                 (both 2.4G and 5G)
 *                fgNoDfs:         whether to exculde DFS channels
 *                ucMaxChannelNum: max array size
 *                pucNumOfChannel: pointer to returned channel number
 *                paucChannelList: pointer to returned channel list array
 *
 * \return none
 */
/*----------------------------------------------------------------------------*/
void
rlmDomainGetChnlList_V2(struct ADAPTER *prAdapter,
			enum ENUM_BAND eSpecificBand, u_int8_t fgNoDfs,
			uint8_t ucMaxChannelNum, uint8_t *pucNumOfChannel,
			struct RF_CHANNEL_INFO *paucChannelList)
{
#if (CFG_SUPPORT_SINGLE_SKU == 1)
	enum ENUM_BAND band;
	uint8_t max_count, i, ucNum;
	struct CMD_DOMAIN_CHANNEL *prCh;

	if (eSpecificBand == BAND_2G4) {
		i = 0;
		max_count = rlmDomainGetActiveChannelCount(KAL_BAND_2GHZ);
	} else if (eSpecificBand == BAND_5G) {
		i = rlmDomainGetActiveChannelCount(KAL_BAND_2GHZ);
		max_count = rlmDomainGetActiveChannelCount(KAL_BAND_5GHZ) +
			rlmDomainGetActiveChannelCount(KAL_BAND_2GHZ);
	}
#if (CFG_SUPPORT_WIFI_6G == 1)
	else if (eSpecificBand == BAND_6G) {
		i = rlmDomainGetActiveChannelCount(KAL_BAND_2GHZ) +
				rlmDomainGetActiveChannelCount(KAL_BAND_5GHZ);
		max_count = rlmDomainGetActiveChannelCount(KAL_BAND_6GHZ) +
			rlmDomainGetActiveChannelCount(KAL_BAND_5GHZ) +
			rlmDomainGetActiveChannelCount(KAL_BAND_2GHZ);
	} else {
		i = 0;
		max_count = rlmDomainGetActiveChannelCount(KAL_BAND_6GHZ) +
			rlmDomainGetActiveChannelCount(KAL_BAND_5GHZ) +
			rlmDomainGetActiveChannelCount(KAL_BAND_2GHZ);
	}
#else
	else {
		i = 0;
		max_count = rlmDomainGetActiveChannelCount(KAL_BAND_5GHZ) +
			rlmDomainGetActiveChannelCount(KAL_BAND_2GHZ);
	}
#endif

	ucNum = 0;
	for (; i < max_count; i++) {
		prCh = rlmDomainGetActiveChannels() + i;
		if (fgNoDfs && (prCh->eFlags & IEEE80211_CHAN_RADAR))
			continue; /*not match*/

		if (i < rlmDomainGetActiveChannelCount(KAL_BAND_2GHZ))
			band = BAND_2G4;
#if (CFG_SUPPORT_WIFI_6G == 1)
		else if (i < rlmDomainGetActiveChannelCount(KAL_BAND_2GHZ) +
			rlmDomainGetActiveChannelCount(KAL_BAND_5GHZ))
			band = BAND_5G;
		else
			band = BAND_6G;
#else
		else
			band = BAND_5G;
#endif

		paucChannelList[ucNum].eBand = band;
		paucChannelList[ucNum].ucChannelNum = prCh->u2ChNum;

		ucNum++;
		if (ucMaxChannelNum == ucNum)
			break;
	}

	*pucNumOfChannel = ucNum;
#else
	*pucNumOfChannel = 0;
#endif
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief Check if Channel supported by HW
 *
 * \param[in/out] eBand:          BAND_2G4, BAND_5G or BAND_NULL
 *                                (both 2.4G and 5G)
 *                ucNumOfChannel: channel number
 *
 * \return TRUE/FALSE
 */
/*----------------------------------------------------------------------------*/
u_int8_t rlmIsValidChnl(struct ADAPTER *prAdapter, uint8_t ucNumOfChannel,
			enum ENUM_BAND eBand)
{
	struct ieee80211_supported_band *channelList;
	int i, chSize;
	struct GLUE_INFO *prGlueInfo;
	struct wiphy *prWiphy;

	prGlueInfo = prAdapter->prGlueInfo;
	prWiphy = priv_to_wiphy(prGlueInfo);

	if (eBand == BAND_5G) {
		channelList = prWiphy->bands[KAL_BAND_5GHZ];
		chSize = channelList->n_channels;
	} else if (eBand == BAND_2G4) {
		channelList = prWiphy->bands[KAL_BAND_2GHZ];
		chSize = channelList->n_channels;
#if (CFG_SUPPORT_WIFI_6G == 1)
	} else if (eBand == BAND_6G) {
		channelList = prWiphy->bands[KAL_BAND_6GHZ];
		chSize = channelList->n_channels;
#endif
	} else
		return FALSE;

	for (i = 0; i < chSize; i++) {
		if ((channelList->channels[i]).hw_value == ucNumOfChannel)
			return TRUE;
	}
	return FALSE;
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief Retrieve the supported channel list of specified band
 *
 * \param[in/out] eSpecificBand:   BAND_2G4, BAND_5G or BAND_NULL
 *                                 (both 2.4G and 5G)
 *                fgNoDfs:         whether to exculde DFS channels
 *                ucMaxChannelNum: max array size
 *                pucNumOfChannel: pointer to returned channel number
 *                paucChannelList: pointer to returned channel list array
 *
 * \return none
 */
/*----------------------------------------------------------------------------*/
void
rlmDomainGetChnlList(struct ADAPTER *prAdapter,
		     enum ENUM_BAND eSpecificBand, u_int8_t fgNoDfs,
		     uint8_t ucMaxChannelNum, uint8_t *pucNumOfChannel,
		     struct RF_CHANNEL_INFO *paucChannelList)
{
	uint8_t i, j, ucNum, ch;
	struct DOMAIN_SUBBAND_INFO *prSubband;
	struct DOMAIN_INFO_ENTRY *prDomainInfo;

	ASSERT(prAdapter);
	ASSERT(paucChannelList);
	ASSERT(pucNumOfChannel);

	if (regd_is_single_sku_en())
		return rlmDomainGetChnlList_V2(prAdapter, eSpecificBand,
					       fgNoDfs, ucMaxChannelNum,
					       pucNumOfChannel,
					       paucChannelList);

	/* If no matched country code, the final one will be used */
	prDomainInfo = rlmDomainGetDomainInfo(prAdapter);
	ASSERT(prDomainInfo);

	ucNum = 0;
	for (i = 0; i < MAX_SUBBAND_NUM; i++) {
		prSubband = &prDomainInfo->rSubBand[i];

		if (prSubband->ucBand == BAND_NULL ||
		    prSubband->ucBand >= BAND_NUM ||
		    (prSubband->ucBand == BAND_5G &&
		     !prAdapter->fgEnable5GBand))
			continue;

#if (CFG_SUPPORT_WIFI_6G == 1)
		if (prSubband->ucBand == BAND_6G && !prAdapter->fgIsHwSupport6G)
			continue;
#endif

		/* repoert to upper layer only non-DFS channel
		 * for ap mode usage
		 */
		if (fgNoDfs == TRUE && prSubband->fgDfs == TRUE)
			continue;

		if (eSpecificBand == BAND_NULL ||
		    prSubband->ucBand == eSpecificBand) {
			for (j = 0; j < prSubband->ucNumChannels; j++) {
				if (ucNum >= ucMaxChannelNum)
					break;

				ch = prSubband->ucFirstChannelNum +
				     j * prSubband->ucChannelSpan;
				if (!rlmIsValidChnl(prAdapter, ch,
						prSubband->ucBand)) {
					DBGLOG(RLM, INFO,
					       "Not support ch%d!\n", ch);
					continue;
				}
				paucChannelList[ucNum].eBand =
							prSubband->ucBand;
				paucChannelList[ucNum].ucChannelNum = ch;
				paucChannelList[ucNum].eDFS = prSubband->fgDfs;
				ucNum++;
			}
		}
	}

	*pucNumOfChannel = ucNum;
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief Retrieve DFS channels from 5G band
 *
 * \param[in/out] ucMaxChannelNum: max array size
 *                pucNumOfChannel: pointer to returned channel number
 *                paucChannelList: pointer to returned channel list array
 *
 * \return none
 */
/*----------------------------------------------------------------------------*/
void rlmDomainGetDfsChnls(struct ADAPTER *prAdapter,
			  uint8_t ucMaxChannelNum, uint8_t *pucNumOfChannel,
			  struct RF_CHANNEL_INFO *paucChannelList)
{
	uint8_t i, j, ucNum, ch;
	struct DOMAIN_SUBBAND_INFO *prSubband;
	struct DOMAIN_INFO_ENTRY *prDomainInfo;

	ASSERT(prAdapter);
	ASSERT(paucChannelList);
	ASSERT(pucNumOfChannel);

	prDomainInfo = rlmDomainGetDomainInfo(prAdapter);
	ASSERT(prDomainInfo);

	ucNum = 0;
	for (i = 0; i < MAX_SUBBAND_NUM; i++) {
		prSubband = &prDomainInfo->rSubBand[i];

		if (prSubband->ucBand == BAND_5G) {
			if (!prAdapter->fgEnable5GBand)
				continue;

			if (prSubband->fgDfs == TRUE) {
				for (j = 0; j < prSubband->ucNumChannels; j++) {
					if (ucNum >= ucMaxChannelNum)
						break;

					ch = prSubband->ucFirstChannelNum +
					     j * prSubband->ucChannelSpan;
					if (!rlmIsValidChnl(prAdapter, ch,
							prSubband->ucBand)) {
						DBGLOG(RLM, INFO,
					       "Not support ch%d!\n", ch);
						continue;
					}

					paucChannelList[ucNum].eBand =
					    prSubband->ucBand;
					paucChannelList[ucNum].ucChannelNum =
					    ch;
					ucNum++;
				}
			}
		}
	}

	*pucNumOfChannel = ucNum;
}

/*----------------------------------------------------------------------------*/
/*!
 * @brief
 *
 * @param[in]
 *
 * @return (none)
 */
/*----------------------------------------------------------------------------*/
void rlmDomainSendCmd(struct ADAPTER *prAdapter)
{

	if (!regd_is_single_sku_en())
		rlmDomainSendPassiveScanInfoCmd(prAdapter);
	rlmDomainSendDomainInfoCmd(prAdapter);
#if CFG_SUPPORT_PWR_LIMIT_COUNTRY
	rlmDomainSendPwrLimitCmd(prAdapter);
#endif
}

#if (CFG_SUPPORT_DYNAMIC_EDCCA == 1)
static bool isEUCountry(struct ADAPTER *prAdapter, uint32_t u4CountryCode)
{
	uint16_t i;
	uint16_t u2TargetCountryCode = 0;

	u2TargetCountryCode =
	    ((u4CountryCode & 0xff) << 8) | ((u4CountryCode & 0xff00) >> 8);

	DBGLOG(RLM, INFO, " Target country code='%c%c'(0x%4x)\n",
	    ((u4CountryCode & 0xff) - 'A'),
		(((u4CountryCode & 0xff00) >> 8) - 'A'),
		u2TargetCountryCode);

	for (i = 0; i < (sizeof(g_u2CountryGroup4) / sizeof(uint16_t)); i++) {
		if (g_u2CountryGroup4[i] == u2TargetCountryCode)
			return TRUE;
	}

	return FALSE;
}

static void rlmSetEd_EU(struct ADAPTER *prAdapter, uint32_t u4CountryCode)
{
	struct WIFI_VAR *prWifiVar = &prAdapter->rWifiVar;

	if (isEUCountry(prAdapter, u4CountryCode)) {
		if ((prWifiVar->i4Ed2GEU != 0) && (prWifiVar->i4Ed5GEU != 0)) {
			wlanSetEd(prAdapter, prWifiVar->i4Ed2GEU,
			    prWifiVar->i4Ed5GEU, 1);
			DBGLOG(RLM, INFO, "Set Ed for EU: 2G=%d, 5G=%d\n",
				prWifiVar->i4Ed2GEU, prWifiVar->i4Ed5GEU);
		}
	} else {
		if ((prWifiVar->i4Ed2GNonEU != 0) &&
			(prWifiVar->i4Ed5GNonEU != 0)) {
			wlanSetEd(prAdapter, prWifiVar->i4Ed2GNonEU,
				prWifiVar->i4Ed5GNonEU, 1);
			DBGLOG(RLM, INFO,
				"Set Ed for non EU: 2G=%d, 5G=%d\n",
				prWifiVar->i4Ed2GNonEU, prWifiVar->i4Ed5GNonEU);
		}
	}
}
#endif

/*----------------------------------------------------------------------------*/
/*!
 * @brief
 *
 * @param[in]
 *
 * @return (none)
 */
/*----------------------------------------------------------------------------*/
void rlmDomainSendDomainInfoCmd_V2(struct ADAPTER *prAdapter)
{
#if (CFG_SUPPORT_SINGLE_SKU == 1)
	u8 max_channel_count = 0;
	u32 buff_max_size, buff_valid_size;
	struct CMD_SET_DOMAIN_INFO_V2 *prCmd;
	struct CMD_DOMAIN_ACTIVE_CHANNEL_LIST *prChs;
	struct wiphy *pWiphy;


	pWiphy = priv_to_wiphy(prAdapter->prGlueInfo);
	if (pWiphy->bands[KAL_BAND_2GHZ] != NULL)
		max_channel_count += pWiphy->bands[KAL_BAND_2GHZ]->n_channels;
	if (pWiphy->bands[KAL_BAND_5GHZ] != NULL)
		max_channel_count += pWiphy->bands[KAL_BAND_5GHZ]->n_channels;
#if (CFG_SUPPORT_WIFI_6G == 1)
	if (pWiphy->bands[KAL_BAND_6GHZ] != NULL)
		max_channel_count += pWiphy->bands[KAL_BAND_6GHZ]->n_channels;
#endif

	if (max_channel_count == 0) {
		DBGLOG(RLM, ERROR, "%s, invalid channel count.\n", __func__);
		ASSERT(0);
	}


	buff_max_size = sizeof(struct CMD_SET_DOMAIN_INFO_V2) +
		max_channel_count * sizeof(struct CMD_DOMAIN_CHANNEL);

	prCmd = cnmMemAlloc(prAdapter, RAM_TYPE_BUF, buff_max_size);
	if (!prCmd) {
		DBGLOG(RLM, ERROR, "Alloc cmd buffer failed\n");
		return;
	}

	kalMemZero(prCmd, buff_max_size);

	prChs = &(prCmd->arActiveChannels);


	/*
	 * Fill in the active channels
	 */
	rlmExtractChannelInfo(max_channel_count, prChs);

	prCmd->u4CountryCode = rlmDomainGetCountryCode();
	prCmd->uc2G4Bandwidth = prAdapter->rWifiVar.uc2G4BandwidthMode;
	prCmd->uc5GBandwidth = prAdapter->rWifiVar.uc5GBandwidthMode;
#if (CFG_SUPPORT_WIFI_6G == 1)
	prCmd->uc6GBandwidth = prAdapter->rWifiVar.uc6GBandwidthMode;
#endif
	prCmd->aucPadding[0] = 0;

#if (CFG_SUPPORT_WIFI_6G == 1)
	buff_valid_size = sizeof(struct CMD_SET_DOMAIN_INFO_V2) +
		(prChs->u1ActiveChNum2g + prChs->u1ActiveChNum5g +
		prChs->u1ActiveChNum6g) *
		sizeof(struct CMD_DOMAIN_CHANNEL);
#else
	buff_valid_size = sizeof(struct CMD_SET_DOMAIN_INFO_V2) +
		(prChs->u1ActiveChNum2g + prChs->u1ActiveChNum5g) *
		sizeof(struct CMD_DOMAIN_CHANNEL);
#endif

	DBGLOG(RLM, INFO,
	       "rlmDomainSendDomainInfoCmd_V2(), buff_valid_size = 0x%x\n",
	       buff_valid_size);

	/* Set domain info to chip */
	wlanSendSetQueryCmd(prAdapter, /* prAdapter */
			    CMD_ID_SET_DOMAIN_INFO, /* ucCID */
			    TRUE,  /* fgSetQuery */
			    FALSE, /* fgNeedResp */
			    FALSE, /* fgIsOid */
			    NULL,  /* pfCmdDoneHandler */
			    NULL,  /* pfCmdTimeoutHandler */
			    buff_valid_size,
			    (uint8_t *) prCmd, /* pucInfoBuffer */
			    NULL,  /* pvSetQueryBuffer */
			    0      /* u4SetQueryBufferLen */
	    );

#if (CFG_SUPPORT_DYNAMIC_EDCCA == 1)
	rlmSetEd_EU(prAdapter, prCmd->u4CountryCode);
#endif

	cnmMemFree(prAdapter, prCmd);
#endif
}

/*----------------------------------------------------------------------------*/
/*!
 * @brief
 *
 * @param[in]
 *
 * @return (none)
 */
/*----------------------------------------------------------------------------*/
void rlmDomainSendDomainInfoCmd(struct ADAPTER *prAdapter)
{
	struct DOMAIN_INFO_ENTRY *prDomainInfo;
	struct CMD_SET_DOMAIN_INFO *prCmd;
	struct DOMAIN_SUBBAND_INFO *prSubBand;
	uint8_t i;

	if (regd_is_single_sku_en())
		return rlmDomainSendDomainInfoCmd_V2(prAdapter);


	prDomainInfo = rlmDomainGetDomainInfo(prAdapter);
	ASSERT(prDomainInfo);

	prCmd = cnmMemAlloc(prAdapter, RAM_TYPE_BUF,
			    sizeof(struct CMD_SET_DOMAIN_INFO));
	if (!prCmd) {
		DBGLOG(RLM, ERROR, "Alloc cmd buffer failed\n");
		return;
	}
	kalMemZero(prCmd, sizeof(struct CMD_SET_DOMAIN_INFO));

	prCmd->u2CountryCode =
		prAdapter->rWifiVar.u2CountryCode;
	prCmd->u2IsSetPassiveScan = 0;
	prCmd->uc2G4Bandwidth =
		prAdapter->rWifiVar.uc2G4BandwidthMode;
	prCmd->uc5GBandwidth =
		prAdapter->rWifiVar.uc5GBandwidthMode;
	prCmd->aucReserved[0] = 0;
	prCmd->aucReserved[1] = 0;

	for (i = 0; i < MAX_SUBBAND_NUM; i++) {
		prSubBand = &prDomainInfo->rSubBand[i];

		prCmd->rSubBand[i].ucRegClass = prSubBand->ucRegClass;
		prCmd->rSubBand[i].ucBand = prSubBand->ucBand;

		if (prSubBand->ucBand != BAND_NULL && prSubBand->ucBand
								< BAND_NUM) {
			prCmd->rSubBand[i].ucChannelSpan
						= prSubBand->ucChannelSpan;
			prCmd->rSubBand[i].ucFirstChannelNum
						= prSubBand->ucFirstChannelNum;
			prCmd->rSubBand[i].ucNumChannels
						= prSubBand->ucNumChannels;
		}
	}

	/* Set domain info to chip */
	wlanSendSetQueryCmd(prAdapter, /* prAdapter */
		CMD_ID_SET_DOMAIN_INFO, /* ucCID */
		TRUE,  /* fgSetQuery */
		FALSE, /* fgNeedResp */
		FALSE, /* fgIsOid */
		NULL,  /* pfCmdDoneHandler */
		NULL,  /* pfCmdTimeoutHandler */
		sizeof(struct CMD_SET_DOMAIN_INFO), /* u4SetQueryInfoLen */
		(uint8_t *) prCmd, /* pucInfoBuffer */
		NULL,  /* pvSetQueryBuffer */
		0      /* u4SetQueryBufferLen */
	    );


#if (CFG_SUPPORT_DYNAMIC_EDCCA == 1)
	rlmSetEd_EU(prAdapter, prCmd->u2CountryCode);
#endif

	cnmMemFree(prAdapter, prCmd);
}

/*----------------------------------------------------------------------------*/
/*!
 * @brief
 *
 * @param[in]
 *
 * @return (none)
 */
/*----------------------------------------------------------------------------*/
void rlmDomainSendPassiveScanInfoCmd(struct ADAPTER *prAdapter)
{
#define REG_DOMAIN_PASSIVE_DEF_IDX	1
#define REG_DOMAIN_PASSIVE_GROUP_NUM \
	(sizeof(arSupportedRegDomains_Passive)	\
	 / sizeof(struct DOMAIN_INFO_ENTRY))

	struct DOMAIN_INFO_ENTRY *prDomainInfo;
	struct CMD_SET_DOMAIN_INFO *prCmd;
	struct DOMAIN_SUBBAND_INFO *prSubBand;
	uint16_t u2TargetCountryCode;
	uint8_t i, j;

	prCmd = cnmMemAlloc(prAdapter, RAM_TYPE_BUF,
			    sizeof(struct CMD_SET_DOMAIN_INFO));
	if (!prCmd) {
		DBGLOG(RLM, ERROR, "Alloc cmd buffer failed\n");
		return;
	}
	kalMemZero(prCmd, sizeof(struct CMD_SET_DOMAIN_INFO));

	prCmd->u2CountryCode = prAdapter->rWifiVar.u2CountryCode;
	prCmd->u2IsSetPassiveScan = 1;
	prCmd->uc2G4Bandwidth =
		prAdapter->rWifiVar.uc2G4BandwidthMode;
	prCmd->uc5GBandwidth =
		prAdapter->rWifiVar.uc5GBandwidthMode;
	prCmd->aucReserved[0] = 0;
	prCmd->aucReserved[1] = 0;

	DBGLOG(RLM, TRACE, "u2CountryCode=0x%04x\n",
	       prAdapter->rWifiVar.u2CountryCode);

	u2TargetCountryCode = prAdapter->rWifiVar.u2CountryCode;

	for (i = 0; i < REG_DOMAIN_PASSIVE_GROUP_NUM; i++) {
		prDomainInfo = &arSupportedRegDomains_Passive[i];

		for (j = 0; j < prDomainInfo->u4CountryNum; j++) {
			if (prDomainInfo->pu2CountryGroup[j] ==
						u2TargetCountryCode)
				break;
		}
		if (j < prDomainInfo->u4CountryNum)
			break;	/* Found */
	}

	if (i >= REG_DOMAIN_PASSIVE_GROUP_NUM)
		prDomainInfo = &arSupportedRegDomains_Passive
					[REG_DOMAIN_PASSIVE_DEF_IDX];

	for (i = 0; i < MAX_SUBBAND_NUM; i++) {
		prSubBand = &prDomainInfo->rSubBand[i];

		prCmd->rSubBand[i].ucRegClass = prSubBand->ucRegClass;
		prCmd->rSubBand[i].ucBand = prSubBand->ucBand;

		if (prSubBand->ucBand != BAND_NULL && prSubBand->ucBand
		    < BAND_NUM) {
			prCmd->rSubBand[i].ucChannelSpan =
						prSubBand->ucChannelSpan;
			prCmd->rSubBand[i].ucFirstChannelNum =
						prSubBand->ucFirstChannelNum;
			prCmd->rSubBand[i].ucNumChannels =
						prSubBand->ucNumChannels;
		}
	}

	/* Set passive scan channel info to chip */
	wlanSendSetQueryCmd(prAdapter, /* prAdapter */
		CMD_ID_SET_DOMAIN_INFO, /* ucCID */
		TRUE,  /* fgSetQuery */
		FALSE, /* fgNeedResp */
		FALSE, /* fgIsOid */
		NULL,  /* pfCmdDoneHandler */
		NULL,  /* pfCmdTimeoutHandler */
		sizeof(struct CMD_SET_DOMAIN_INFO), /* u4SetQueryInfoLen */
		(uint8_t *) prCmd, /* pucInfoBuffer */
		NULL,  /* pvSetQueryBuffer */
		0      /* u4SetQueryBufferLen */
	    );

#if (CFG_SUPPORT_DYNAMIC_EDCCA == 1)
	rlmSetEd_EU(prAdapter, prCmd->u2CountryCode);
#endif

	cnmMemFree(prAdapter, prCmd);
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief
 *
 * \param[in/out]
 *
 * \return TRUE  Legal channel
 *         FALSE Illegal channel for current regulatory domain
 */
/*----------------------------------------------------------------------------*/
u_int8_t rlmDomainIsLegalChannel_V2(struct ADAPTER *prAdapter,
				    enum ENUM_BAND eBand,
				    uint8_t ucChannel)
{
#if (CFG_SUPPORT_SINGLE_SKU == 1)
	uint8_t idx, start_idx, end_idx;
	struct CMD_DOMAIN_CHANNEL *prCh;

#if (CFG_SUPPORT_WIFI_6G == 1)
	if (eBand == BAND_6G) {
		start_idx = rlmDomainGetActiveChannelCount(KAL_BAND_2GHZ) +
				rlmDomainGetActiveChannelCount(KAL_BAND_5GHZ);
		end_idx = rlmDomainGetActiveChannelCount(KAL_BAND_2GHZ) +
				rlmDomainGetActiveChannelCount(KAL_BAND_5GHZ) +
				rlmDomainGetActiveChannelCount(KAL_BAND_6GHZ);
	} else if (eBand == BAND_2G4) {
		start_idx = 0;
		end_idx = rlmDomainGetActiveChannelCount(KAL_BAND_2GHZ);
	} else {
		start_idx = rlmDomainGetActiveChannelCount(KAL_BAND_2GHZ);
		end_idx = rlmDomainGetActiveChannelCount(KAL_BAND_2GHZ) +
				rlmDomainGetActiveChannelCount(KAL_BAND_5GHZ);
	}
#else
	if (eBand == BAND_2G4) {
		start_idx = 0;
		end_idx = rlmDomainGetActiveChannelCount(KAL_BAND_2GHZ);
	} else {
		start_idx = rlmDomainGetActiveChannelCount(KAL_BAND_2GHZ);
		end_idx = rlmDomainGetActiveChannelCount(KAL_BAND_2GHZ) +
				rlmDomainGetActiveChannelCount(KAL_BAND_5GHZ);
	}
#endif

	for (idx = start_idx; idx < end_idx; idx++) {
		prCh = rlmDomainGetActiveChannels() + idx;

		if (prCh->u2ChNum == ucChannel)
			return TRUE;
	}

	return FALSE;
#else
	return FALSE;
#endif
}

u_int8_t rlmDomainIsLegalChannel(struct ADAPTER *prAdapter,
				 enum ENUM_BAND eBand, uint8_t ucChannel)
{
	uint8_t i, j;
	struct DOMAIN_SUBBAND_INFO *prSubband;
	struct DOMAIN_INFO_ENTRY *prDomainInfo;

	if (regd_is_single_sku_en())
		return rlmDomainIsLegalChannel_V2(prAdapter, eBand, ucChannel);


	prDomainInfo = rlmDomainGetDomainInfo(prAdapter);
	ASSERT(prDomainInfo);

	for (i = 0; i < MAX_SUBBAND_NUM; i++) {
		prSubband = &prDomainInfo->rSubBand[i];

		if (prSubband->ucBand == BAND_5G && !prAdapter->fgEnable5GBand)
			continue;

#if (CFG_SUPPORT_WIFI_6G == 1)
		if (prSubband->ucBand == BAND_6G && !prAdapter->fgIsHwSupport6G)
			continue;
#endif

		if (prSubband->ucBand == eBand) {
			for (j = 0; j < prSubband->ucNumChannels; j++) {
				if ((prSubband->ucFirstChannelNum + j *
				    prSubband->ucChannelSpan) == ucChannel) {
					if (!rlmIsValidChnl(prAdapter,
						    ucChannel,
						    prSubband->ucBand)) {
						DBGLOG(RLM, INFO,
						       "Not support ch%d!\n",
						       ucChannel);
						return FALSE;
					} else
						return TRUE;

				}
			}
		}
	}

	return FALSE;
}

u_int8_t rlmDomainIsLegalDfsChannel(struct ADAPTER *prAdapter,
		enum ENUM_BAND eBand, uint8_t ucChannel)
{
	uint8_t i, j;
	struct DOMAIN_SUBBAND_INFO *prSubband;
	struct DOMAIN_INFO_ENTRY *prDomainInfo;

	prDomainInfo = rlmDomainGetDomainInfo(prAdapter);
	ASSERT(prDomainInfo);

	for (i = 0; i < MAX_SUBBAND_NUM; i++) {
		prSubband = &prDomainInfo->rSubBand[i];

		if (prSubband->ucBand == BAND_5G
			&& !prAdapter->fgEnable5GBand)
			continue;

		if (prSubband->ucBand == eBand
			&& prSubband->fgDfs == TRUE) {
			for (j = 0; j < prSubband->ucNumChannels; j++) {
				if ((prSubband->ucFirstChannelNum + j *
					prSubband->ucChannelSpan)
					== ucChannel) {
					return TRUE;
				}
			}
		}
	}

	return FALSE;
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief
 *
 * \param[in/out]
 *
 * \return none
 */
/*----------------------------------------------------------------------------*/

uint32_t rlmDomainSupOperatingClassIeFill(uint8_t *pBuf)
{
	/*
	 *  The Country element should only be included for
	 *  Status Code 0 (Successful).
	 */
	uint32_t u4IeLen;
	uint8_t aucClass[12] = { 0x01, 0x02, 0x03, 0x05, 0x16, 0x17, 0x19, 0x1b,
		0x1c, 0x1e, 0x20, 0x21
	};

	/*
	 * The Supported Operating Classes element is used by a STA to
	 * advertise the operating classes that it is capable of operating
	 * with in this country.
	 * The Country element (see 8.4.2.10) allows a STA to configure its
	 * PHY and MAC for operation when the operating triplet of Operating
	 * Extension Identifier, Operating Class, and Coverage Class fields
	 * is present.
	 */
	SUP_OPERATING_CLASS_IE(pBuf)->ucId = ELEM_ID_SUP_OPERATING_CLASS;
	SUP_OPERATING_CLASS_IE(pBuf)->ucLength = 1 + sizeof(aucClass);
	SUP_OPERATING_CLASS_IE(pBuf)->ucCur = 0x0c;	/* 0x51 */
	kalMemCopy(SUP_OPERATING_CLASS_IE(pBuf)->ucSup, aucClass,
		   sizeof(aucClass));
	u4IeLen = (SUP_OPERATING_CLASS_IE(pBuf)->ucLength + 2);
#if CFG_SUPPORT_802_11D
	pBuf += u4IeLen;

	COUNTRY_IE(pBuf)->ucId = ELEM_ID_COUNTRY_INFO;
	COUNTRY_IE(pBuf)->ucLength = 6;
	COUNTRY_IE(pBuf)->aucCountryStr[0] = 0x55;
	COUNTRY_IE(pBuf)->aucCountryStr[1] = 0x53;
	COUNTRY_IE(pBuf)->aucCountryStr[2] = 0x20;
	COUNTRY_IE(pBuf)->arCountryStr[0].ucFirstChnlNum = 1;
	COUNTRY_IE(pBuf)->arCountryStr[0].ucNumOfChnl = 11;
	COUNTRY_IE(pBuf)->arCountryStr[0].cMaxTxPwrLv = 0x1e;
	u4IeLen += (COUNTRY_IE(pBuf)->ucLength + 2);
#endif
	return u4IeLen;
}

/*----------------------------------------------------------------------------*/
/*!
 * @brief
 *
 * @param[in]
 *
 * @return (fgValid) : 0 -> inValid, 1 -> Valid
 */
/*----------------------------------------------------------------------------*/
u_int8_t rlmDomainCheckChannelEntryValid(struct ADAPTER *prAdapter,
					 uint8_t ucCentralCh)
{
	u_int8_t fgValid = FALSE;
	uint8_t ucTemp = 0xff;
	uint8_t i;
	/*Check Power limit table channel efficient or not */

	/* CH50 is not located in any FCC subbands
	 * but it's a valid central channel for 160C
	 */
	if (ucCentralCh == 50) {
		fgValid = TRUE;
		return fgValid;
	}

	for (i = POWER_LIMIT_2G4; i < POWER_LIMIT_SUBAND_NUM; i++) {
		if ((ucCentralCh >= g_rRlmSubBand[i].ucStartCh) &&
				    (ucCentralCh <= g_rRlmSubBand[i].ucEndCh))
			ucTemp = (ucCentralCh - g_rRlmSubBand[i].ucStartCh) %
				 g_rRlmSubBand[i].ucInterval;
		if (ucTemp == 0)
			break;
	}

	if (ucTemp == 0)
		fgValid = TRUE;
	return fgValid;

}

/*----------------------------------------------------------------------------*/
/*!
 * \brief
 *
 * \param[in]
 *
 * \return
 */
/*----------------------------------------------------------------------------*/
uint8_t rlmDomainGetCenterChannel(enum ENUM_BAND eBand, uint8_t ucPriChannel,
				  enum ENUM_CHNL_EXT eExtend)
{
	uint8_t ucCenterChannel;

	if (eExtend == CHNL_EXT_SCA)
		ucCenterChannel = ucPriChannel + 2;
	else if (eExtend == CHNL_EXT_SCB)
		ucCenterChannel = ucPriChannel - 2;
	else
		ucCenterChannel = ucPriChannel;

	return ucCenterChannel;
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief
 *
 * \param[in]
 *
 * \return
 */
/*----------------------------------------------------------------------------*/
u_int8_t
rlmDomainIsValidRfSetting(struct ADAPTER *prAdapter,
			  enum ENUM_BAND eBand,
			  uint8_t ucPriChannel,
			  enum ENUM_CHNL_EXT eExtend,
			  enum ENUM_CHANNEL_WIDTH eChannelWidth,
			  uint8_t ucChannelS1, uint8_t ucChannelS2)
{
	uint8_t ucCenterCh = 0;
	uint8_t  ucUpperChannel;
	uint8_t  ucLowerChannel;
	u_int8_t fgValidChannel = TRUE;
	u_int8_t fgUpperChannel = TRUE;
	u_int8_t fgLowerChannel = TRUE;
	u_int8_t fgValidBW = TRUE;
	u_int8_t fgValidRfSetting = TRUE;
	uint32_t u4PrimaryOffset;

	/*DBG msg for Channel InValid */
	if (eChannelWidth == CW_20_40MHZ) {
		ucCenterCh = rlmDomainGetCenterChannel(eBand, ucPriChannel,
						       eExtend);

		/* Check Central Channel Valid or Not */
		fgValidChannel = rlmDomainCheckChannelEntryValid(prAdapter,
								 ucCenterCh);
		if (fgValidChannel == FALSE)
			DBGLOG(RLM, WARN, "Rf20: CentralCh=%d\n", ucCenterCh);

		/* Check Upper Channel and Lower Channel */
		switch (eExtend) {
		case CHNL_EXT_SCA:
			ucUpperChannel = ucPriChannel + 4;
			ucLowerChannel = ucPriChannel;
			break;
		case CHNL_EXT_SCB:
			ucUpperChannel = ucPriChannel;
			ucLowerChannel = ucPriChannel - 4;
			break;
		default:
			ucUpperChannel = ucPriChannel;
			ucLowerChannel = ucPriChannel;
			break;
		}

		fgUpperChannel = rlmDomainCheckChannelEntryValid(prAdapter,
								ucUpperChannel);
		if (fgUpperChannel == FALSE)
			DBGLOG(RLM, WARN, "Rf20: UpperCh=%d\n", ucUpperChannel);

		fgLowerChannel = rlmDomainCheckChannelEntryValid(prAdapter,
								ucLowerChannel);
		if (fgLowerChannel == FALSE)
			DBGLOG(RLM, WARN, "Rf20: LowerCh=%d\n", ucLowerChannel);

	} else if ((eChannelWidth == CW_80MHZ) ||
		   (eChannelWidth == CW_160MHZ)) {
		ucCenterCh = ucChannelS1;

		/* Check Central Channel Valid or Not */
		if (eChannelWidth != CW_160MHZ) {
			/* BW not check , ex: primary 36 and
			 * central channel 50 will fail the check
			 */
			fgValidChannel =
				rlmDomainCheckChannelEntryValid(prAdapter,
								ucCenterCh);
		}

		if (fgValidChannel == FALSE)
			DBGLOG(RLM, WARN, "Rf80/160C: CentralCh=%d\n",
			       ucCenterCh);
	} else if (eChannelWidth == CW_80P80MHZ) {
		ucCenterCh = ucChannelS1;

		fgValidChannel = rlmDomainCheckChannelEntryValid(prAdapter,
								 ucCenterCh);

		if (fgValidChannel == FALSE)
			DBGLOG(RLM, WARN, "Rf160NC: CentralCh1=%d\n",
			       ucCenterCh);

		ucCenterCh = ucChannelS2;

		fgValidChannel = rlmDomainCheckChannelEntryValid(prAdapter,
								 ucCenterCh);

		if (fgValidChannel == FALSE)
			DBGLOG(RLM, WARN, "Rf160NC: CentralCh2=%d\n",
			       ucCenterCh);

		/* Check Central Channel Valid or Not */
	} else {
		DBGLOG(RLM, ERROR, "Wrong BW =%d\n", eChannelWidth);
		fgValidChannel = FALSE;
	}

	/* Check BW Setting Correct or Not */
	if (eBand == BAND_2G4) {
		if (eChannelWidth != CW_20_40MHZ) {
			fgValidBW = FALSE;
			DBGLOG(RLM, WARN, "Rf: B=%d, W=%d\n",
			       eBand, eChannelWidth);
		}
	} else {
		if ((eChannelWidth == CW_80MHZ) ||
				(eChannelWidth == CW_80P80MHZ)) {
			u4PrimaryOffset = CAL_CH_OFFSET_80M(ucPriChannel,
							    ucChannelS1);
			if (u4PrimaryOffset >= 4) {
				fgValidBW = FALSE;
				DBGLOG(RLM, WARN, "Rf: PriOffSet=%d, W=%d\n",
				       u4PrimaryOffset, eChannelWidth);
			}
			if (ucPriChannel == 165 && eBand == BAND_5G) {
				fgValidBW = FALSE;
				DBGLOG(RLM, WARN,
				       "Rf: PriOffSet=%d, W=%d C=%d\n",
				       u4PrimaryOffset, eChannelWidth,
				       ucPriChannel);
			}
		} else if (eChannelWidth == CW_160MHZ) {
			u4PrimaryOffset = CAL_CH_OFFSET_160M(ucPriChannel,
							     ucCenterCh);
			if (u4PrimaryOffset >= 8) {
				fgValidBW = FALSE;
				DBGLOG(RLM, WARN,
				       "Rf: PriOffSet=%d, W=%d\n",
				       u4PrimaryOffset, eChannelWidth);
			}
		}
	}

	if ((fgValidBW == FALSE) || (fgValidChannel == FALSE) ||
	    (fgUpperChannel == FALSE) || (fgLowerChannel == FALSE))
		fgValidRfSetting = FALSE;

	return fgValidRfSetting;

}

#if (CFG_SUPPORT_SINGLE_SKU == 1)
/*
 * This function coverts country code from alphabet chars to u32,
 * the caller need to pass country code chars and do size check
 */
u_int32_t rlmDomainAlpha2ToU32(char *pcAlpha2, u_int8_t ucAlpha2Size)
{
	u_int8_t ucIdx;
	u_int32_t u4CountryCode = 0;

	if (ucAlpha2Size > TX_PWR_LIMIT_COUNTRY_STR_MAX_LEN) {
		DBGLOG(RLM, ERROR, "alpha2 size %d is invalid!(max: %d)\n",
			ucAlpha2Size, TX_PWR_LIMIT_COUNTRY_STR_MAX_LEN);
		ucAlpha2Size = TX_PWR_LIMIT_COUNTRY_STR_MAX_LEN;
	}

	for (ucIdx = 0; ucIdx < ucAlpha2Size; ucIdx++)
		u4CountryCode |= (pcAlpha2[ucIdx] << (ucIdx * 8));

	return u4CountryCode;
}



#if (CFG_SUPPORT_SINGLE_SKU_LOCAL_DB == 1)
u_int32_t rlmDomainUpdateRegdomainFromaLocalDataBaseByCountryCode(
	struct wiphy *pWiphy,
	u_int32_t u4CountryCode
	)
{
	const struct ieee80211_regdomain *pRegdom = NULL;
	char acCountryCodeStr[MAX_COUNTRY_CODE_LEN + 1] = {0};
	u_int32_t u4FinalCountryCode = u4CountryCode;

	rlmDomainU32ToAlpha(u4FinalCountryCode, acCountryCodeStr);
	pRegdom =
		rlmDomainSearchRegdomainFromLocalDataBase(acCountryCodeStr);
	if (!pRegdom) {
		DBGLOG(RLM, ERROR,
	       "Cannot find the %s RegDomain. Set to default WW\n",
	       acCountryCodeStr);
		pRegdom = &default_regdom_ww;
		u4FinalCountryCode = COUNTRY_CODE_WW;
	}

	kalApplyCustomRegulatory(pWiphy, pRegdom);

	return u4FinalCountryCode;
}
#else
u_int32_t rlmDomainUpdateRegdomainFromaLocalDataBaseByCountryCode(
	struct wiphy *pWiphy,
	u_int32_t u4CountryCode
	)
{
	return 0;
}
#endif

uint8_t
rlmDomainCountryCodeUpdateSanity(
	struct GLUE_INFO *prGlueInfo,
	struct wiphy *pWiphy,
	struct ADAPTER **prAdapter)
{
	enum regd_state eCurrentState = rlmDomainGetCtrlState();

	/* Always use the wlan GlueInfo as parameter. */
	if (!prGlueInfo) {
		DBGLOG(RLM, ERROR, "prGlueInfo is NULL!\n");
		return FALSE;
	}

	if (!prGlueInfo->prAdapter) {
		DBGLOG(RLM, ERROR, "prAdapter is NULL!\n");
		return FALSE;
	}
	*prAdapter = prGlueInfo->prAdapter;

	if (!pWiphy) {
		DBGLOG(RLM, ERROR, "pWiphy is NULL!\n");
		return FALSE;
	}

	if (eCurrentState == REGD_STATE_INVALID ||
		eCurrentState == REGD_STATE_UNDEFINED) {
		DBGLOG(RLM, ERROR, "regd is in an invalid state\n");
		return FALSE;
	}

	return TRUE;
}

void rlmDomainCountryCodeUpdate(
	struct ADAPTER *prAdapter, struct wiphy *pWiphy,
	u_int32_t u4CountryCode)
{
	u_int32_t u4FinalCountryCode = u4CountryCode;
	char acCountryCodeStr[MAX_COUNTRY_CODE_LEN + 1] = {0};
#ifdef CFG_SUPPORT_BT_SKU
	typedef void (*bt_fn_t) (char *);
	bt_fn_t bt_func = NULL;
	char *bt_func_name = "btmtk_set_country_code_from_wifi";
	void *func_addr = NULL;
#endif
#ifdef CFG_SUPPORT_ZB_SKU
	typedef void (*zb_fn_t) (char *);
	zb_fn_t zb_func = NULL;
	char *zb_func_name = "zbmtk_set_country_code_from_wifi";
	void *zb_func_addr = NULL;
#endif

	if (rlmDomainIsUsingLocalRegDomainDataBase()) {
		u4FinalCountryCode =
			rlmDomainUpdateRegdomainFromaLocalDataBaseByCountryCode(
				pWiphy,
				u4CountryCode);
	}

	rlmDomainU32ToAlpha(u4FinalCountryCode, acCountryCodeStr);

	if (u4FinalCountryCode != u4CountryCode)
		rlmDomainSetCountryCode(acCountryCodeStr,
			MAX_COUNTRY_CODE_LEN);

	DBGLOG(RLM, INFO, "g_mtk_regd_control.alpha2 = %s\n", acCountryCodeStr);
#ifdef CFG_SUPPORT_BT_SKU
#if (CFG_ENABLE_GKI_SUPPORT != 1)
	func_addr = GLUE_SYMBOL_GET(bt_func_name);
#endif
	if (func_addr) {
		bt_func = (bt_fn_t) func_addr;
		bt_func(acCountryCodeStr);
#if (CFG_ENABLE_GKI_SUPPORT != 1)
		GLUE_SYMBOL_PUT(bt_func_name);
#endif
	} else {
		DBGLOG(RLM, ERROR,
		       "Can't find function %s\n",
		       bt_func_name);
	}
#endif

#ifdef CFG_SUPPORT_ZB_SKU
#if (CFG_ENABLE_GKI_SUPPORT != 1)
	zb_func_addr = GLUE_SYMBOL_GET(zb_func_name);
#endif

	if (zb_func_addr) {
		zb_func = (zb_fn_t) zb_func_addr;
		zb_func(acCountryCodeStr);

		DBGLOG(RLM, INFO,
				"Notify ZB country code(%s) done via %s\n",
				acCountryCodeStr, zb_func_name);

#if (CFG_ENABLE_GKI_SUPPORT != 1)
		GLUE_SYMBOL_PUT(zb_func_name);
#endif
	} else {
		DBGLOG(RLM, ERROR,
				"Can't find function %s\n",
				zb_func_name);
	}
#endif

	if (pWiphy)
		rlmDomainParsingChannel(pWiphy);

	if (!regd_is_single_sku_en())
		return;

	prAdapter->rWifiVar.u2CountryCode =
		(uint16_t)rlmDomainGetCountryCode();

	/* Send commands to firmware */
	rlmDomainSendCmd(prAdapter);

}
void
rlmDomainSetCountry(struct ADAPTER *prAdapter)
{
	struct GLUE_INFO *prGlueInfo = rlmDomainGetGlueInfo();
	struct ADAPTER *prBaseAdapter;
	struct wiphy *prBaseWiphy = wlanGetWiphy();

	if (!rlmDomainCountryCodeUpdateSanity(
		prGlueInfo, prBaseWiphy, &prBaseAdapter)) {
		DBGLOG(RLM, WARN, "sanity check failed, skip update\n");
		return;
	}

	rlmDomainCountryCodeUpdate(
		prBaseAdapter, prBaseWiphy,
		rlmDomainGetCountryCode());
}

uint8_t rlmDomainTxPwrLimitGetTableVersion(
	uint8_t *pucBuf, uint32_t u4BufLen)
{
#define TX_PWR_LIMIT_VERSION_STR_LEN 7
#define TX_PWR_LIMIT_MAX_VERSION 2
	uint32_t u4TmpPos = 0;
	uint8_t ucVersion = 0;

	while (u4TmpPos < u4BufLen && pucBuf[u4TmpPos] != '<')
		u4TmpPos++;

	if (u4TmpPos >= (u4BufLen - TX_PWR_LIMIT_VERSION_STR_LEN))
		return ucVersion;

	if (kalStrnCmp(&pucBuf[u4TmpPos + 1], "Ver:", 4) == 0) {
		ucVersion = (pucBuf[u4TmpPos + 5] - '0') * 10 +
			(pucBuf[u4TmpPos + 6] - '0');
	}

	if (ucVersion > TX_PWR_LIMIT_MAX_VERSION)
		ucVersion = 0;

	return ucVersion;
}

/*----------------------------------------------------------------------------*/
/*!
 * \brief Search the tx power limit setting range of the specified in the text
 *        file
 *
 * \param[IN] u4CountryCode The u32 type of the specified country.
 * \param[IN] pucBuf The content of the text file.
 * \param[IN] u4cBufLen End boundary of the text file.
 * \param[OUT] pu4CountryStart Store the start position of the desired country
 *             settings.
 * \param[OUT] pu4CountryEnd Store the end position of the desired country
 *             settings.
 *
 * \retval TRUE Success.
 * \retval FALSE Failure.
 */
/*----------------------------------------------------------------------------*/
u_int8_t rlmDomainTxPwrLimitGetCountryRange(
	uint32_t u4CountryCode, uint8_t *pucBuf, uint32_t u4BufLen,
	uint32_t *pu4CountryStart, uint32_t *pu4CountryEnd)
{
	uint32_t u4TmpPos = 0;
	char pcrCountryStr[TX_PWR_LIMIT_COUNTRY_STR_MAX_LEN + 1] = {0};
	uint8_t cIdx = 0;

	while (1) {
		while (u4TmpPos < u4BufLen && pucBuf[u4TmpPos] != '[')
			u4TmpPos++;

		u4TmpPos++; /* skip the '[' char */

		cIdx = 0;
		while ((u4TmpPos < u4BufLen) &&
			   (cIdx < TX_PWR_LIMIT_COUNTRY_STR_MAX_LEN) &&
			   (pucBuf[u4TmpPos] != ']')) {
			pcrCountryStr[cIdx++] = pucBuf[u4TmpPos];
			u4TmpPos++;
		}

		u4TmpPos++; /* skip the ']' char */

		if ((u4TmpPos >= u4BufLen) ||
		    (cIdx > TX_PWR_LIMIT_COUNTRY_STR_MAX_LEN))
			return FALSE;

		if (u4CountryCode ==
			rlmDomainAlpha2ToU32(pcrCountryStr, cIdx)) {
			DBGLOG(RLM, INFO,
				"Found TxPwrLimit table for CountryCode \"%s\"\n",
				pcrCountryStr);
			*pu4CountryStart = u4TmpPos;
			/* the location after char ']' */
			break;
		}
	}

	while (u4TmpPos < u4BufLen && pucBuf[u4TmpPos] != '[')
		u4TmpPos++;

	*pu4CountryEnd = u4TmpPos;

	return TRUE;
}

u_int8_t rlmDomainTxPwrLimitSearchSection(const char *pSectionName,
	uint8_t *pucBuf, uint32_t *pu4Pos, uint32_t u4BufEnd)
{
	uint32_t u4TmpPos = *pu4Pos;
	uint8_t uSectionNameLen = kalStrLen(pSectionName);

	while (1) {
		while (u4TmpPos < u4BufEnd && pucBuf[u4TmpPos] != '<')
			u4TmpPos++;

		u4TmpPos++; /* skip char '<' */

		if (u4TmpPos + uSectionNameLen >= u4BufEnd)
			return FALSE;

		if (kalStrnCmp(&pucBuf[u4TmpPos],
				pSectionName, uSectionNameLen) == 0) {

			/* Go to the end of section header line */
			while ((u4TmpPos < u4BufEnd) &&
				   (pucBuf[u4TmpPos] != '\n'))
				u4TmpPos++;

			*pu4Pos = u4TmpPos;

			break;
		}
	}

	return TRUE;
}

u_int8_t rlmDomainTxPwrLimitSectionEnd(uint8_t *pucBuf,
	const char *pSectionName, uint32_t *pu4Pos, uint32_t u4BufEnd)
{
	uint32_t u4TmpPos = *pu4Pos;
	char cTmpChar = 0;
	uint8_t uSectionNameLen = kalStrLen(pSectionName);

	while (u4TmpPos < u4BufEnd) {
		cTmpChar = pucBuf[u4TmpPos];

		/* skip blank lines */
		if (cTmpChar == ' ' || cTmpChar == '\t' ||
			cTmpChar == '\n' || cTmpChar == '\r') {
			u4TmpPos++;
			continue;
		}

		break;
	}

	/* 2 means '/' and '>' */
	if (u4TmpPos + uSectionNameLen + 2 >= u4BufEnd) {
		*pu4Pos = u4BufEnd;
		return FALSE;
	}

	if (pucBuf[u4TmpPos] != '<')
		return FALSE;

	if (pucBuf[u4TmpPos + 1] != '/' ||
		pucBuf[u4TmpPos + 2 + uSectionNameLen] != '>' ||
		kalStrnCmp(&pucBuf[u4TmpPos + 2],
			pSectionName, uSectionNameLen)) {

		*pu4Pos = u4TmpPos + uSectionNameLen + 2;
		return FALSE;
	}

	/* 3 means go to the location after '>' */
	*pu4Pos = u4TmpPos + uSectionNameLen + 3;
	return TRUE;
}

int8_t rlmDomainTxPwrLimitGetChIdx(
	struct TX_PWR_LIMIT_DATA *pTxPwrLimit, uint8_t ucChannel)
{
	int8_t cIdx = 0;

	for (cIdx = 0; cIdx < pTxPwrLimit->ucChNum; cIdx++)
		if (ucChannel ==
			pTxPwrLimit->rChannelTxPwrLimit[cIdx].ucChannel)
			return cIdx;

	DBGLOG(RLM, ERROR,
		"Can't find idx of channel %d in TxPwrLimit data\n",
		ucChannel);

	return -1;
}

int8_t rlmDomainTxLegacyPwrLimitGetChIdx(
	struct TX_PWR_LEGACY_LIMIT_DATA *pTxPwrLegacyLimit, uint8_t ucChannel)
{
	int8_t cIdx = 0;

	for (cIdx = 0; cIdx < pTxPwrLegacyLimit->ucChNum; cIdx++)
		if (ucChannel ==
			pTxPwrLegacyLimit->
			    rChannelTxLegacyPwrLimit[cIdx].ucChannel)
			return cIdx;

	DBGLOG(RLM, ERROR,
		"Can't find idx of channel %d in TxPwrLimit data\n",
			ucChannel);

	return -1;
}

u_int8_t rlmDomainTxPwrLimitIsTxBfBackoffSection(
	uint8_t ucVersion, uint8_t ucSectionIdx)
{
	if (ucVersion == 1 && ucSectionIdx == 8)
		return TRUE;

	return FALSE;
}
u_int8_t rlmDomainTxPwrLimitLoadChannelSetting(
	uint8_t ucVersion, uint8_t *pucBuf, uint32_t *pu4Pos, uint32_t u4BufEnd,
	struct TX_PWR_LIMIT_DATA *pTxPwrLimit, uint8_t ucSectionIdx)
{
	uint32_t u4TmpPos = *pu4Pos;
	char cTmpChar = 0;
	struct CHANNEL_TX_PWR_LIMIT *prChTxPwrLimit = NULL;
	u_int8_t bNeg = FALSE;
	int8_t cLimitValue = 0, cChIdx = 0;
	uint8_t ucIdx = 0, ucChannel = 0;
	uint8_t ucElementNum =
		gTx_Pwr_Limit_Element_Num[ucVersion][ucSectionIdx];

	/* skip blank lines */
	while (u4TmpPos < u4BufEnd) {
		cTmpChar = pucBuf[u4TmpPos];

		if (cTmpChar == ' ' || cTmpChar == '\t' ||
			cTmpChar == '\n' || cTmpChar == '\r') {
			u4TmpPos++;
			continue;
		}

		break;
	}

	/* current is at the location of 'c',
	 * check remaining buf length for 'chxxx'
	 */
	if (u4TmpPos + 5 >= u4BufEnd) {
		DBGLOG(RLM, ERROR,
			"Invalid location of ch setting: %u/%u\n",
			u4TmpPos, u4BufEnd);
		return FALSE;
	}

	if (pucBuf[u4TmpPos] == 'c' && pucBuf[u4TmpPos + 1] == 'h') {
		ucChannel = (pucBuf[u4TmpPos + 2] - '0') * 100 +
					(pucBuf[u4TmpPos + 3] - '0') * 10 +
					(pucBuf[u4TmpPos + 4] - '0');
	} else { /* invalid format */
		*pu4Pos = u4TmpPos;
		DBGLOG(RLM, ERROR,
			"Invalid ch setting starting chars: %c%c\n",
			pucBuf[u4TmpPos], pucBuf[u4TmpPos + 1]);

		/* goto next line */
		while (*pu4Pos < u4BufEnd && pucBuf[*pu4Pos] != '\n')
			(*pu4Pos)++;

		return TRUE;
	}

	cChIdx = rlmDomainTxPwrLimitGetChIdx(pTxPwrLimit, ucChannel);

	if (cChIdx == -1) {
		*pu4Pos = u4TmpPos;
		DBGLOG(RLM, ERROR, "Invalid ch %u %c%c%c\n", ucChannel,
			pucBuf[u4TmpPos + 2],
			pucBuf[u4TmpPos + 3], pucBuf[u4TmpPos + 4]);

		/* goto next line */
		while (*pu4Pos < u4BufEnd && pucBuf[*pu4Pos] != '\n')
			(*pu4Pos)++;

		return TRUE;
	}

	u4TmpPos += 5;

	prChTxPwrLimit = &pTxPwrLimit->rChannelTxPwrLimit[cChIdx];

	/* read the channel TxPwrLimit settings */
	for (ucIdx = 0; ucIdx < ucElementNum; ucIdx++) {

		/* skip blank and comma */
		while (u4TmpPos < u4BufEnd) {
			cTmpChar = pucBuf[u4TmpPos];

			if ((cTmpChar == ' ') ||
				(cTmpChar == '\t') ||
				(cTmpChar == ',')) {
				u4TmpPos++;
				continue;
			}
			break;
		}

		if (cTmpChar == '\n')
			break;

		if (u4TmpPos >= u4BufEnd) {
			*pu4Pos = u4BufEnd;
			DBGLOG(RLM, ERROR,
				"Invalid location of ch tx pwr limit val: %u/%u\n",
				u4TmpPos, u4BufEnd);
			return FALSE;
		}

		bNeg = FALSE;

		cTmpChar = pucBuf[u4TmpPos];

		if (cTmpChar == '-') {
			bNeg = TRUE;
			u4TmpPos++;
		} else {
			if (cTmpChar == 'x') {
				if (!rlmDomainTxPwrLimitIsTxBfBackoffSection(
					ucVersion, ucSectionIdx)) {
					prChTxPwrLimit->
						rTxPwrLimitValue
						[ucSectionIdx][ucIdx] =
						TX_PWR_LIMIT_MAX_VAL;
				} else {
					prChTxPwrLimit->rTxBfBackoff[ucIdx] =
						TX_PWR_LIMIT_MAX_VAL;
				}
				u4TmpPos++;
				continue;
			}
		}

		cLimitValue = 0;
		while (u4TmpPos < u4BufEnd) {
			cTmpChar = pucBuf[u4TmpPos];

			if (cTmpChar < '0' || cTmpChar > '9')
				break;

			cLimitValue = (cLimitValue * 10) + (cTmpChar - '0');
			u4TmpPos++;
		}

		if (bNeg)
			cLimitValue = -cLimitValue;
		if (!rlmDomainTxPwrLimitIsTxBfBackoffSection(
			ucVersion, ucSectionIdx)) {
			prChTxPwrLimit->rTxPwrLimitValue[ucSectionIdx][ucIdx] =
				cLimitValue;
		} else {
			prChTxPwrLimit->rTxBfBackoff[ucIdx] =
				cLimitValue;
		}
	}

	*pu4Pos = u4TmpPos;
	return TRUE;
}

u_int8_t rlmDomainLegacyTxPwrLimitLoadChannelSetting(
	uint8_t ucVersion, uint8_t *pucBuf, uint32_t *pu4Pos, uint32_t u4BufEnd,
	struct TX_PWR_LEGACY_LIMIT_DATA *pTxPwrLegacyLimit,
	uint8_t ucSectionIdx)
{
	uint32_t u4TmpPos = *pu4Pos;
	char cTmpChar = 0;
	struct CHANNEL_TX_LEGACY_PWR_LIMIT
		*prChTxLegPwrLimit = NULL;
	u_int8_t bNeg = FALSE;
	int8_t cLimitValue = 0, cChIdx = 0;
	uint8_t ucIdx = 0, ucChannel = 0;
	uint8_t ucLegacyElementNum =
		gTx_Legacy_Pwr_Limit_Element_Num[ucVersion][ucSectionIdx];

	/* skip blank lines */
	while (u4TmpPos < u4BufEnd) {
		cTmpChar = pucBuf[u4TmpPos];

		if (cTmpChar == ' ' || cTmpChar == '\t' ||
			cTmpChar == '\n' || cTmpChar == '\r') {
			u4TmpPos++;
			continue;
		}

		break;
	}

	/* current is at the location of 'c' */
	/* check remaining buf length for 'chxxx' */
	if (u4TmpPos + 5 >= u4BufEnd) {
		DBGLOG(RLM, ERROR,
			"Invalid location of ch setting: %u/%u\n",
			u4TmpPos, u4BufEnd);
		return FALSE;
	}

	if (pucBuf[u4TmpPos] == 'c' && pucBuf[u4TmpPos + 1] == 'h') {
		ucChannel = (pucBuf[u4TmpPos + 2] - '0') * 100 +
					(pucBuf[u4TmpPos + 3] - '0') * 10 +
					(pucBuf[u4TmpPos + 4] - '0');
	} else { /* invalid format */
		*pu4Pos = u4TmpPos;
		DBGLOG(RLM, ERROR,
			"[LEGACY_TXPOWER]:Invalid ch setting starting chars: %c%c\n",
			pucBuf[u4TmpPos], pucBuf[u4TmpPos + 1]);

		/* goto next line */
		while (*pu4Pos < u4BufEnd && pucBuf[*pu4Pos] != '\n')
			(*pu4Pos)++;

		return TRUE;
	}

	cChIdx = rlmDomainTxLegacyPwrLimitGetChIdx(pTxPwrLegacyLimit,
		ucChannel);

	if (cChIdx == -1) {
		*pu4Pos = u4TmpPos;

		/* goto next line */
		while (*pu4Pos < u4BufEnd && pucBuf[*pu4Pos] != '\n')
			(*pu4Pos)++;

		return TRUE;
	}

	u4TmpPos += 5;

	prChTxLegPwrLimit =
		&pTxPwrLegacyLimit->rChannelTxLegacyPwrLimit[cChIdx];

	for (ucIdx = 0; ucIdx < ucLegacyElementNum; ucIdx++) {

		/*  skip blank and comma */
		while (u4TmpPos < u4BufEnd) {
			cTmpChar = pucBuf[u4TmpPos];

			if ((cTmpChar == ' ') ||
				(cTmpChar == '\t') ||
				(cTmpChar == ',')) {
				u4TmpPos++;
				continue;
			}
			break;
		}

		if (cTmpChar == '\n')
			break;

		if (u4TmpPos >= u4BufEnd) {
			*pu4Pos = u4BufEnd;
			DBGLOG(RLM, ERROR,
				"Invalid location of ch tx pwr limit val: %u/%u\n",
				u4TmpPos, u4BufEnd);
			return FALSE;
		}

		bNeg = FALSE;

		cTmpChar = pucBuf[u4TmpPos];

		if (cTmpChar == '-') {
			bNeg = TRUE;
			u4TmpPos++;
		} else {
			if (cTmpChar == 'x') {
				prChTxLegPwrLimit->
					rTxLegacyPwrLimitValue
					[ucSectionIdx][ucIdx] =
					TX_PWR_LIMIT_MAX_VAL;

				u4TmpPos++;
				continue;
			}
		}

		cLimitValue = 0;
		while (u4TmpPos < u4BufEnd) {
			cTmpChar = pucBuf[u4TmpPos];

			if (cTmpChar < '0' || cTmpChar > '9')
				break;

			cLimitValue = (cLimitValue * 10) + (cTmpChar - '0');
			u4TmpPos++;
		}

		if (bNeg)
			cLimitValue = -cLimitValue;
		prChTxLegPwrLimit->rTxLegacyPwrLimitValue[ucSectionIdx][ucIdx] =
			cLimitValue;
	}

	*pu4Pos = u4TmpPos;
	return TRUE;
}

void rlmDomainTxPwrLimitRemoveComments(
	uint8_t *pucBuf, uint32_t u4BufLen)
{
	uint32_t u4TmpPos = 0;
	char cTmpChar = 0;

	while (u4TmpPos < u4BufLen) {
		cTmpChar = pucBuf[u4TmpPos];

		if (cTmpChar == '#') {
			while (cTmpChar != '\n') {
				pucBuf[u4TmpPos] = ' ';

				u4TmpPos++;
				if (u4TmpPos >= u4BufLen)
					break;

				cTmpChar = pucBuf[u4TmpPos];
			}
		}
		u4TmpPos++;
	}
}

u_int8_t rlmDomainTxPwrLimitLoad(
	struct ADAPTER *prAdapter, uint8_t *pucBuf, uint32_t u4BufLen,
	uint8_t ucVersion, uint32_t u4CountryCode,
	struct TX_PWR_LIMIT_DATA *pTxPwrLimitData)
{
	uint8_t uSecIdx = 0;
	uint8_t ucSecNum = gTx_Pwr_Limit_Section[ucVersion].ucSectionNum;
	uint32_t u4CountryStart = 0, u4CountryEnd = 0, u4Pos = 0;
	struct TX_PWR_LIMIT_SECTION *prSection =
		&gTx_Pwr_Limit_Section[ucVersion];

	uint8_t *prFileName = prAdapter->chip_info->prTxPwrLimitFile;


	if (!rlmDomainTxPwrLimitGetCountryRange(u4CountryCode, pucBuf,
		u4BufLen, &u4CountryStart, &u4CountryEnd)) {
		DBGLOG(RLM, ERROR, "Can't find specified table in %s\n",
			prFileName);

		/* Use WW as default country */
		if (!rlmDomainTxPwrLimitGetCountryRange(COUNTRY_CODE_WW, pucBuf,
			u4BufLen, &u4CountryStart, &u4CountryEnd)) {
			DBGLOG(RLM, ERROR,
				"Can't find default table (WW) in %s\n",
				prFileName);
			return FALSE;
		}
	}

	u4Pos = u4CountryStart;

	for (uSecIdx = 0; uSecIdx < ucSecNum; uSecIdx++) {
		const uint8_t *pSecName =
			prSection->arSectionNames[uSecIdx];

		if (!rlmDomainTxPwrLimitSearchSection(
				pSecName, pucBuf, &u4Pos,
				u4CountryEnd)) {
			DBGLOG(RLM, ERROR,
				"Can't find specified section %s in %s\n",
				pSecName,
				prFileName);
			continue;
		}

		DBGLOG(RLM, INFO, "Find specified section %s in %s\n",
			pSecName,
			prFileName);

		while (!rlmDomainTxPwrLimitSectionEnd(pucBuf,
			pSecName,
			&u4Pos, u4CountryEnd) &&
			u4Pos < u4CountryEnd) {
			if (!rlmDomainTxPwrLimitLoadChannelSetting(
				ucVersion, pucBuf, &u4Pos, u4CountryEnd,
				pTxPwrLimitData, uSecIdx))
				return FALSE;
			if (rlmDomainTxPwrLimitIsTxBfBackoffSection(
				ucVersion, uSecIdx))
				g_bTxBfBackoffExists = TRUE;
		}
	}

	DBGLOG(RLM, INFO, "Load %s finished\n", prFileName);
	return TRUE;
}

u_int8_t rlmDomainTxPwrLegacyLimitLoad(
	struct ADAPTER *prAdapter, uint8_t *pucBuf, uint32_t u4BufLen,
	uint8_t ucVersion, uint32_t u4CountryCode,
	struct TX_PWR_LEGACY_LIMIT_DATA *pTxPwrLegacyLimitData)
{
	uint8_t uLegSecIdx;
	uint8_t ucLegacySecNum =
		gTx_Legacy_Pwr_Limit_Section[ucVersion].ucLegacySectionNum;
	uint32_t u4CountryStart = 0, u4CountryEnd = 0, u4Pos = 0;
	struct TX_LEGACY_PWR_LIMIT_SECTION *prLegacySection =
		&gTx_Legacy_Pwr_Limit_Section[ucVersion];

	uint8_t *prFileName = prAdapter->chip_info->prTxPwrLimitFile;

	if (!rlmDomainTxPwrLimitGetCountryRange(u4CountryCode, pucBuf,
		u4BufLen, &u4CountryStart, &u4CountryEnd)) {
		DBGLOG(RLM, ERROR, "Can't find specified table in %s\n",
			prFileName);

		/* Use WW as default country */
		if (!rlmDomainTxPwrLimitGetCountryRange(COUNTRY_CODE_WW, pucBuf,
			u4BufLen, &u4CountryStart, &u4CountryEnd)) {
			DBGLOG(RLM, ERROR,
				"Can't find default table (WW) in %s\n",
				prFileName);
			return FALSE;
		}
	}

	u4Pos = u4CountryStart;

	for (uLegSecIdx = 0; uLegSecIdx < ucLegacySecNum; uLegSecIdx++) {

		const uint8_t *pLegacySecName =
			prLegacySection->arLegacySectionNames[uLegSecIdx];

		if (!rlmDomainTxPwrLimitSearchSection(
				pLegacySecName, pucBuf, &u4Pos,
				u4CountryEnd)) {
			continue;
		}

		while (!rlmDomainTxPwrLimitSectionEnd(pucBuf,
			pLegacySecName,
			&u4Pos, u4CountryEnd) &&
			u4Pos < u4CountryEnd) {

			if (!rlmDomainLegacyTxPwrLimitLoadChannelSetting(
				ucVersion, pucBuf, &u4Pos, u4CountryEnd,
				pTxPwrLegacyLimitData, uLegSecIdx))
				return FALSE;
		}

	}

	DBGLOG(RLM, INFO, "Load %s finished\n", prFileName);
	return TRUE;
}

void rlmDomainTxPwrLimitSetChValues(
	uint8_t ucVersion,
	struct CMD_CHANNEL_POWER_LIMIT_V2 *pCmd,
	struct CHANNEL_TX_PWR_LIMIT *pChTxPwrLimit)
{
	uint8_t section = 0, e = 0;
	uint8_t ucElementNum = 0;

	pCmd->tx_pwr_dsss_cck = pChTxPwrLimit->rTxPwrLimitValue[0][0];
	pCmd->tx_pwr_dsss_bpsk = pChTxPwrLimit->rTxPwrLimitValue[0][1];

	/* 6M, 9M */
	pCmd->tx_pwr_ofdm_bpsk = pChTxPwrLimit->rTxPwrLimitValue[0][2];
	/* 12M, 18M */
	pCmd->tx_pwr_ofdm_qpsk = pChTxPwrLimit->rTxPwrLimitValue[0][3];
	/* 24M, 36M */
	pCmd->tx_pwr_ofdm_16qam = pChTxPwrLimit->rTxPwrLimitValue[0][4];
	pCmd->tx_pwr_ofdm_48m = pChTxPwrLimit->rTxPwrLimitValue[0][5];
	pCmd->tx_pwr_ofdm_54m = pChTxPwrLimit->rTxPwrLimitValue[0][6];

	/* MCS0*/
	pCmd->tx_pwr_ht20_bpsk = pChTxPwrLimit->rTxPwrLimitValue[1][0];
	/* MCS1, MCS2*/
	pCmd->tx_pwr_ht20_qpsk = pChTxPwrLimit->rTxPwrLimitValue[1][1];
	/* MCS3, MCS4*/
	pCmd->tx_pwr_ht20_16qam = pChTxPwrLimit->rTxPwrLimitValue[1][2];
	/* MCS5*/
	pCmd->tx_pwr_ht20_mcs5 = pChTxPwrLimit->rTxPwrLimitValue[1][3];
	/* MCS6*/
	pCmd->tx_pwr_ht20_mcs6 = pChTxPwrLimit->rTxPwrLimitValue[1][4];
	/* MCS7*/
	pCmd->tx_pwr_ht20_mcs7 = pChTxPwrLimit->rTxPwrLimitValue[1][5];

	/* MCS0*/
	pCmd->tx_pwr_ht40_bpsk = pChTxPwrLimit->rTxPwrLimitValue[2][0];
	/* MCS1, MCS2*/
	pCmd->tx_pwr_ht40_qpsk = pChTxPwrLimit->rTxPwrLimitValue[2][1];
	/* MCS3, MCS4*/
	pCmd->tx_pwr_ht40_16qam = pChTxPwrLimit->rTxPwrLimitValue[2][2];
	/* MCS5*/
	pCmd->tx_pwr_ht40_mcs5 = pChTxPwrLimit->rTxPwrLimitValue[2][3];
	/* MCS6*/
	pCmd->tx_pwr_ht40_mcs6 = pChTxPwrLimit->rTxPwrLimitValue[2][4];
	/* MCS7*/
	pCmd->tx_pwr_ht40_mcs7 = pChTxPwrLimit->rTxPwrLimitValue[2][5];
	/* MCS32*/
	pCmd->tx_pwr_ht40_mcs32 = pChTxPwrLimit->rTxPwrLimitValue[2][6];

	/* MCS0*/
	pCmd->tx_pwr_vht20_bpsk = pChTxPwrLimit->rTxPwrLimitValue[3][0];
	/* MCS1, MCS2*/
	pCmd->tx_pwr_vht20_qpsk = pChTxPwrLimit->rTxPwrLimitValue[3][1];
	/* MCS3, MCS4*/
	pCmd->tx_pwr_vht20_16qam = pChTxPwrLimit->rTxPwrLimitValue[3][2];
	/* MCS5, MCS6*/
	pCmd->tx_pwr_vht20_64qam = pChTxPwrLimit->rTxPwrLimitValue[3][3];
	pCmd->tx_pwr_vht20_mcs7 = pChTxPwrLimit->rTxPwrLimitValue[3][4];
	pCmd->tx_pwr_vht20_mcs8 = pChTxPwrLimit->rTxPwrLimitValue[3][5];
	pCmd->tx_pwr_vht20_mcs9 = pChTxPwrLimit->rTxPwrLimitValue[3][6];

	pCmd->tx_pwr_vht_40 = pChTxPwrLimit->rTxPwrLimitValue[4][2];
	pCmd->tx_pwr_vht_80 = pChTxPwrLimit->rTxPwrLimitValue[4][3];
	pCmd->tx_pwr_vht_160c = pChTxPwrLimit->rTxPwrLimitValue[4][5];
	pCmd->tx_pwr_vht_160nc = pChTxPwrLimit->rTxPwrLimitValue[4][4];
	pCmd->tx_pwr_lg_40 = pChTxPwrLimit->rTxPwrLimitValue[4][0];
	pCmd->tx_pwr_lg_80 = pChTxPwrLimit->rTxPwrLimitValue[4][1];


	DBGLOG(RLM, TRACE, "ch %d\n", pCmd->ucCentralCh);
	for (section = 0; section < TX_PWR_LIMIT_SECTION_NUM; section++) {
		struct TX_PWR_LIMIT_SECTION *pSection =
			&gTx_Pwr_Limit_Section[ucVersion];
		ucElementNum = gTx_Pwr_Limit_Element_Num[ucVersion][section];
		for (e = 0; e < ucElementNum; e++)
			DBGLOG(RLM, TRACE, "TxPwrLimit[%s][%s]= %d\n",
				pSection->arSectionNames[section],
				gTx_Pwr_Limit_Element[ucVersion][section][e],
				pChTxPwrLimit->rTxPwrLimitValue[section][e]);
	}
}

void rlmDomainTxPwrLimitPerRateSetChValues(
	uint8_t ucVersion,
	struct CMD_TXPOWER_CHANNEL_POWER_LIMIT_PER_RATE *pCmd,
	struct CHANNEL_TX_PWR_LIMIT *pChTxPwrLimit)
{
	uint8_t section = 0, e = 0, count = 0;
	uint8_t ucElementNum = 0;

	for (section = 0; section < TX_PWR_LIMIT_SECTION_NUM; section++) {
		if (rlmDomainTxPwrLimitIsTxBfBackoffSection(ucVersion, section))
			continue;
		ucElementNum = gTx_Pwr_Limit_Element_Num[ucVersion][section];
		for (e = 0; e < ucElementNum; e++) {
			pCmd->aucTxPwrLimit.i1PwrLimit[count] =
				pChTxPwrLimit->rTxPwrLimitValue[section][e];
			count++;
		}
	}

	DBGLOG(RLM, TRACE, "ch %d\n", pCmd->u1CentralCh);
	count = 0;
	for (section = 0; section < TX_PWR_LIMIT_SECTION_NUM; section++) {
		struct TX_PWR_LIMIT_SECTION *pSection =
			&gTx_Pwr_Limit_Section[ucVersion];
		if (rlmDomainTxPwrLimitIsTxBfBackoffSection(ucVersion, section))
			continue;
		ucElementNum = gTx_Pwr_Limit_Element_Num[ucVersion][section];
		for (e = 0; e < ucElementNum; e++) {
			DBGLOG(RLM, TRACE, "LegacyTxPwrLimit[%s][%s]= %d\n",
				pSection->arSectionNames[section],
				gTx_Pwr_Limit_Element[ucVersion][section][e],
				pCmd->aucTxPwrLimit.i1PwrLimit[count]);
			count++;
		}
	}
}

void rlmDomainTxLegacyPwrLimitPerRateSetChValues(
	uint8_t ucVersion,
	struct CMD_TXPOWER_CHANNEL_LEGACY_POWER_LIMIT_PER_RATE *pCmd,
	struct CHANNEL_TX_LEGACY_PWR_LIMIT *pChTxLegacyPwrLimit)
{
	uint8_t section = 0, e = 0, count = 0;
	uint8_t ucElementNum = 0;

	for (section = 0; section <
		TX_LEGACY_PWR_LIMIT_SECTION_NUM; section++) {
		ucElementNum =
			gTx_Legacy_Pwr_Limit_Element_Num[ucVersion][section];
		for (e = 0; e < ucElementNum; e++) {
			pCmd->aucTxLegacyPwrLimit.i1LegacyPwrLimit[count] =
				pChTxLegacyPwrLimit->
					rTxLegacyPwrLimitValue[section][e];
			count++;
		}
	}

	DBGLOG(RLM, TRACE, "ch %d\n", pCmd->u1CentralCh);
	count = 0;

	for (section = 0; section <
		TX_LEGACY_PWR_LIMIT_SECTION_NUM; section++) {

		struct TX_LEGACY_PWR_LIMIT_SECTION
			*pLegacySection =
				&gTx_Legacy_Pwr_Limit_Section[ucVersion];

		if (rlmDomainTxPwrLimitIsTxBfBackoffSection(ucVersion,
			section))
			continue;

		ucElementNum =
			gTx_Legacy_Pwr_Limit_Element_Num[ucVersion][section];

		for (e = 0; e < ucElementNum; e++) {
			DBGLOG(RLM, TRACE, "LegacyTxPwrLimit[%s][%s]= %d\n",
				pLegacySection->arLegacySectionNames[section],
				gTx_Legacy_Pwr_Limit_Element
					[ucVersion][section][e],
				pCmd->
				aucTxLegacyPwrLimit.i1LegacyPwrLimit[count]
				);
			count++;
		}
	}
}

void rlmDomainTxPwrLimitSetValues(
	uint8_t ucVersion,
	struct CMD_SET_COUNTRY_CHANNEL_POWER_LIMIT_V2 *pSetCmd,
	struct TX_PWR_LIMIT_DATA *pTxPwrLimit)
{
	uint8_t ucIdx = 0;
	int8_t cChIdx = 0;
	struct CMD_CHANNEL_POWER_LIMIT_V2 *pCmd = NULL;
	struct CHANNEL_TX_PWR_LIMIT *pChTxPwrLimit = NULL;

	if (!pSetCmd || !pTxPwrLimit) {
		DBGLOG(RLM, ERROR, "Invalid TxPwrLimit request\n");
		return;
	}

	for (ucIdx = 0; ucIdx < pSetCmd->ucNum; ucIdx++) {
		pCmd = &(pSetCmd->rChannelPowerLimit[ucIdx]);
		cChIdx = rlmDomainTxPwrLimitGetChIdx(pTxPwrLimit,
			pCmd->ucCentralCh);
		if (cChIdx == -1) {
			DBGLOG(RLM, ERROR,
				"Invalid ch idx found while assigning values\n");
			continue;
		}
		pChTxPwrLimit = &pTxPwrLimit->rChannelTxPwrLimit[cChIdx];
		rlmDomainTxPwrLimitSetChValues(ucVersion, pCmd, pChTxPwrLimit);
	}
}

void rlmDomainTxPwrLimitPerRateSetValues(
	uint8_t ucVersion,
	struct CMD_SET_TXPOWER_COUNTRY_TX_POWER_LIMIT_PER_RATE *pSetCmd,
	struct TX_PWR_LIMIT_DATA *pTxPwrLimit)
{
	uint8_t ucIdx = 0;
	int8_t cChIdx = 0;
	struct CMD_TXPOWER_CHANNEL_POWER_LIMIT_PER_RATE *pChPwrLimit = NULL;
	struct CHANNEL_TX_PWR_LIMIT *pChTxPwrLimit = NULL;

	if (pSetCmd == NULL) {
		DBGLOG(RLM, ERROR, "%s pSetCmd is NULL\n", __func__);
		return;
	}

	for (ucIdx = 0; ucIdx < pSetCmd->ucNum; ucIdx++) {
		pChPwrLimit = &(pSetCmd->u.rChannelPowerLimit[ucIdx]);
		cChIdx = rlmDomainTxPwrLimitGetChIdx(pTxPwrLimit,
			pChPwrLimit->u1CentralCh);

		if (cChIdx == -1) {
			DBGLOG(RLM, ERROR,
				"Invalid ch idx found while assigning values\n");
			continue;
		}
		pChTxPwrLimit = &pTxPwrLimit->rChannelTxPwrLimit[cChIdx];
		rlmDomainTxPwrLimitPerRateSetChValues(ucVersion,
			pChPwrLimit, pChTxPwrLimit);
	}
}

void rlmDomainTxLegacyPwrLimitPerRateSetValues(
	uint8_t ucVersion,
	struct CMD_SET_TXPOWER_COUNTRY_TX_LEGACY_POWER_LIMIT_PER_RATE *pSetCmd,
	struct TX_PWR_LEGACY_LIMIT_DATA *pTxPwrLegacyLimit)
{

	uint8_t ucIdx = 0;
	int8_t cChIdx = 0;

	struct CMD_TXPOWER_CHANNEL_LEGACY_POWER_LIMIT_PER_RATE
		*pChLegPwrLimit = NULL;
	struct CHANNEL_TX_LEGACY_PWR_LIMIT *pChTxLegacyPwrLimit = NULL;

	if (pSetCmd == NULL) {
		DBGLOG(RLM, ERROR, "%s pSetCmd is NULL\n", __func__);
		return;
	}

	for (ucIdx = 0; ucIdx < pSetCmd->ucNum; ucIdx++) {
		pChLegPwrLimit = &(pSetCmd->rChannelLegacyPowerLimit[ucIdx]);
		cChIdx = rlmDomainTxLegacyPwrLimitGetChIdx(pTxPwrLegacyLimit,
			pChLegPwrLimit->u1CentralCh);

		if (cChIdx == -1) {
			DBGLOG(RLM, ERROR,
				"Invalid ch idx found while assigning values\n");
			continue;
		}

		pChTxLegacyPwrLimit = &pTxPwrLegacyLimit->
			rChannelTxLegacyPwrLimit[cChIdx];

		rlmDomainTxLegacyPwrLimitPerRateSetChValues(ucVersion,
			pChLegPwrLimit, pChTxLegacyPwrLimit);
	}
}

u_int8_t rlmDomainTxPwrLimitLoadFromFile(
	struct ADAPTER *prAdapter,
	uint8_t *pucConfigBuf, uint32_t *pu4ConfigReadLen)
{
#define TXPWRLIMIT_FILE_LEN 64
	u_int8_t bRet = TRUE;
	uint8_t *prFileName = prAdapter->chip_info->prTxPwrLimitFile;
	uint8_t aucPath[4][TXPWRLIMIT_FILE_LEN];

	if (!prFileName || kalStrLen(prFileName) == 0) {
		bRet = FALSE;
		DBGLOG(RLM, ERROR, "Invalid TxPwrLimit dat file name!!\n");
		goto error;
	}

	kalMemZero(aucPath, sizeof(aucPath));
	kalSnprintf(aucPath[0], TXPWRLIMIT_FILE_LEN, "%s", prFileName);
	kalSnprintf(aucPath[1], TXPWRLIMIT_FILE_LEN,
		"/data/misc/%s", prFileName);
	kalSnprintf(aucPath[2], TXPWRLIMIT_FILE_LEN,
		"/data/misc/wifi/%s", prFileName);
	kalSnprintf(aucPath[3], TXPWRLIMIT_FILE_LEN,
		"/storage/sdcard0/%s", prFileName);

	kalMemZero(pucConfigBuf, WLAN_TX_PWR_LIMIT_FILE_BUF_SIZE);
	*pu4ConfigReadLen = 0;

	if (wlanGetFileContent(
			prAdapter,
			aucPath[0],
			pucConfigBuf,
			WLAN_TX_PWR_LIMIT_FILE_BUF_SIZE,
			pu4ConfigReadLen, TRUE) == 0) {
		/* ToDo:: Nothing */
	} else if (wlanGetFileContent(
				prAdapter,
				aucPath[1],
				pucConfigBuf,
				WLAN_TX_PWR_LIMIT_FILE_BUF_SIZE,
				pu4ConfigReadLen, FALSE) == 0) {
		/* ToDo:: Nothing */
	} else if (wlanGetFileContent(
				prAdapter,
				aucPath[2],
				pucConfigBuf,
				WLAN_TX_PWR_LIMIT_FILE_BUF_SIZE,
				pu4ConfigReadLen, FALSE) == 0) {
		/* ToDo:: Nothing */
	} else if (wlanGetFileContent(
				prAdapter,
				aucPath[3],
				pucConfigBuf,
				WLAN_TX_PWR_LIMIT_FILE_BUF_SIZE,
				pu4ConfigReadLen, FALSE) == 0) {
		/* ToDo:: Nothing */
	} else {
		bRet = FALSE;
		goto error;
	}

	if (pucConfigBuf[0] == '\0' || *pu4ConfigReadLen == 0) {
		bRet = FALSE;
		goto error;
	}

error:

	return bRet;
}

u_int8_t rlmDomainGetTxPwrLimit(
	uint32_t country_code,
	uint8_t *pucVersion,
	struct GLUE_INFO *prGlueInfo,
	struct TX_PWR_LIMIT_DATA *pTxPwrLimitData)
{
	u_int8_t bRet = FALSE;
	uint8_t *pucConfigBuf = NULL;
	uint32_t u4ConfigReadLen = 0;

	pucConfigBuf = (uint8_t *) kalMemAlloc(
		WLAN_TX_PWR_LIMIT_FILE_BUF_SIZE, VIR_MEM_TYPE);

	if (!pucConfigBuf)
		return bRet;

	bRet = rlmDomainTxPwrLimitLoadFromFile(prGlueInfo->prAdapter,
		pucConfigBuf, &u4ConfigReadLen);

	rlmDomainTxPwrLimitRemoveComments(pucConfigBuf, u4ConfigReadLen);
	*pucVersion = rlmDomainTxPwrLimitGetTableVersion(pucConfigBuf,
		u4ConfigReadLen);

	if (!rlmDomainTxPwrLimitLoad(prGlueInfo->prAdapter,
		pucConfigBuf, u4ConfigReadLen, *pucVersion,
		country_code, pTxPwrLimitData)) {
		bRet = FALSE;
		goto error;
	}

error:

	kalMemFree(pucConfigBuf,
		VIR_MEM_TYPE, WLAN_TX_PWR_LIMIT_FILE_BUF_SIZE);

	return bRet;
}

u_int8_t rlmDomainGetTxPwrLegacyLimit(
	uint32_t country_code,
	uint8_t *pucVersion,
	struct GLUE_INFO *prGlueInfo,
	struct TX_PWR_LEGACY_LIMIT_DATA *pTxPwrLegacyLimitData)
{
	u_int8_t bRet = FALSE;
	uint8_t *pucConfigBuf = NULL;
	uint32_t u4ConfigReadLen = 0;

	pucConfigBuf = (uint8_t *) kalMemAlloc(
		WLAN_TX_PWR_LIMIT_FILE_BUF_SIZE, VIR_MEM_TYPE);

	if (!pucConfigBuf)
		return bRet;

	bRet = rlmDomainTxPwrLimitLoadFromFile(prGlueInfo->prAdapter,
		pucConfigBuf, &u4ConfigReadLen);

	rlmDomainTxPwrLimitRemoveComments(pucConfigBuf, u4ConfigReadLen);
	*pucVersion = rlmDomainTxPwrLimitGetTableVersion(pucConfigBuf,
		u4ConfigReadLen);

	if (!rlmDomainTxPwrLegacyLimitLoad(prGlueInfo->prAdapter,
		pucConfigBuf, u4ConfigReadLen, *pucVersion,
		country_code, pTxPwrLegacyLimitData)) {
		bRet = FALSE;
		goto error;
	}

error:

	kalMemFree(pucConfigBuf,
		VIR_MEM_TYPE, WLAN_TX_PWR_LIMIT_FILE_BUF_SIZE);

	return bRet;
}

#if CFG_SUPPORT_DYNAMIC_PWR_LIMIT
static void rlmDomainUpdateChannelTxPwrLimit(
	uint8_t ucVersion,
	u_int8_t fgIs6GBand,
	struct CHANNEL_TX_PWR_LIMIT *prChLimit,
	struct TX_PWR_CTRL_CHANNEL_SETTING *prChSetting)
{
	uint32_t i, j;
	int8_t sarLimit;

	if (prChLimit == NULL || prChSetting == NULL)
		return;

	for (i = 0; i < gTx_Pwr_Limit_Section[ucVersion].ucSectionNum; i++) {
		if (prChSetting->op[i] == PWR_CTRL_TYPE_NO_ACTION)
			continue;

		if ((fgIs6GBand) &&
		    (i != PWR_LIMIT_OFDM) && (i < PWR_LIMIT_RU26))
			continue;

		if (prChSetting->op[i] == PWR_CTRL_TYPE_NEGATIVE)
			sarLimit = 0 - prChSetting->i8PwrLimit[i];
		else
			sarLimit = prChSetting->i8PwrLimit[i];

		for (j = 0; j < gTx_Pwr_Limit_Element_Num[ucVersion][i]; j++) {
			prChLimit->rTxPwrLimitValue[i][j] = MIN(
				sarLimit,
				prChLimit->rTxPwrLimitValue[i][j]);
			DBGLOG(RLM, TRACE,
			       "SAR CH: %d, BAND: %s, SEC: %s, ELE: %s, VAL: %d\n",
			       prChLimit->ucChannel,
			       fgIs6GBand ? "6G" : "2.4G/5G",
			       gTx_Pwr_Limit_Section[ucVersion]
					.arSectionNames[i],
			       gTx_Pwr_Limit_Element[ucVersion][i][j],
			       prChLimit->rTxPwrLimitValue[i][j]);
		}
	}
}

static void rlmDomainUpdateChannelTxLegacyPwrLimit(
	uint8_t ucVersion,
	u_int8_t fgIs6GBand,
	struct CHANNEL_TX_LEGACY_PWR_LIMIT *prChLegacyLimit,
	struct TX_PWR_CTRL_CHANNEL_SETTING *prChSetting)
{
	uint32_t i, j;
	int8_t sarLimit;

	if (prChLegacyLimit == NULL || prChSetting == NULL)
		return;

	if (prChSetting->op[PWR_LIMIT_OFDM] == PWR_CTRL_TYPE_NO_ACTION)
		return;

	if (prChSetting->op[PWR_LIMIT_OFDM] == PWR_CTRL_TYPE_NEGATIVE)
		sarLimit =  0 - prChSetting->i8PwrLimit[PWR_LIMIT_OFDM];
	else
		sarLimit = prChSetting->i8PwrLimit[PWR_LIMIT_OFDM];

	for (i = 0;
	     i < gTx_Legacy_Pwr_Limit_Section[ucVersion].ucLegacySectionNum;
	     i++) {
		for (j = 0;
		     j < gTx_Legacy_Pwr_Limit_Element_Num[ucVersion][i];
		     j++) {
			prChLegacyLimit->rTxLegacyPwrLimitValue[i][j] = MIN(
				sarLimit,
				prChLegacyLimit->rTxLegacyPwrLimitValue[i][j]);
			DBGLOG(RLM, TRACE,
			       "LegacySAR CH: %d, BAND: %s, SEC: %s, ELE: %s, VAL: %d\n",
			       prChLegacyLimit->ucChannel,
			       fgIs6GBand ? "6G" : "2.4G/5G",
			       gTx_Legacy_Pwr_Limit_Section[ucVersion]
					.arLegacySectionNames[i],
			       gTx_Legacy_Pwr_Limit_Element[ucVersion][i][j],
			       prChLegacyLimit->rTxLegacyPwrLimitValue[i][j]);
		}
	}
}

static void rlmDomainUpdateTxPwrLimitData(
	uint8_t ucVersion,
	u_int8_t fgIs6GBand,
	struct TX_PWR_CTRL_ELEMENT *pcElement,
	struct TX_PWR_LIMIT_DATA *pTxPwrLimitData,
	struct TX_PWR_LEGACY_LIMIT_DATA *pTxPwrLegacyLimitData)
{
	uint32_t i, j;
	uint8_t cChIdx, cChIdxMin, cChIdxMax;
	struct TX_PWR_CTRL_CHANNEL_SETTING *prChSetting = NULL;
	struct CHANNEL_TX_PWR_LIMIT *prChLimit = NULL;
	struct CHANNEL_TX_LEGACY_PWR_LIMIT *prLegacyChLimit = NULL;
	enum ENUM_TX_POWER_CTRL_CHANNEL_TYPE eChnlType;

	if (pcElement == NULL ||
	    pTxPwrLimitData == NULL ||
	    pTxPwrLegacyLimitData == NULL)
		return;

	DBGLOG(RLM, TRACE,
	       "Update TxPwrLimitData by sar(%s, %d)\n",
	       pcElement->name, pcElement->index);

	for (i = 0; i < pcElement->settingCount; i++) {
		prChSetting = &pcElement->rChlSettingList[i];
		eChnlType = prChSetting->eChnlType;
		if (fgIs6GBand) {
			if ((eChnlType >= PWR_CTRL_CHNL_TYPE_2G4) &&
			    (eChnlType <= PWR_CTRL_CHNL_TYPE_5G_BAND4))
				continue;
#if (CFG_SUPPORT_WIFI_6G == 1)
		} else {
			if (eChnlType >= PWR_CTRL_CHNL_TYPE_6G)
				continue;
#endif
		}

		switch (eChnlType) {
		case PWR_CTRL_CHNL_TYPE_NORMAL:
			cChIdxMin = prChSetting->channelParam[0];
			cChIdxMax = prChSetting->channelParam[0];
			break;
		case PWR_CTRL_CHNL_TYPE_ALL:
			if (fgIs6GBand) {
#if (CFG_SUPPORT_WIFI_6G == 1)
				cChIdxMin = gTx_Pwr_Limit_6g_Ch[0];
				cChIdxMax = gTx_Pwr_Limit_6g_Ch[
						TX_PWR_LIMIT_6G_CH_NUM - 1];
#else
				cChIdxMin = 0;
				cChIdxMax = 0;
#endif
			} else {
				cChIdxMin = gTx_Pwr_Limit_2g_Ch[0];
				cChIdxMax = gTx_Pwr_Limit_5g_Ch[
						TX_PWR_LIMIT_5G_CH_NUM - 1];
			}
			break;
		case PWR_CTRL_CHNL_TYPE_RANGE:
			cChIdxMin = prChSetting->channelParam[0];
			cChIdxMax = prChSetting->channelParam[1];
			break;
		case PWR_CTRL_CHNL_TYPE_2G4:
			cChIdxMin = gTx_Pwr_Limit_2g_Ch[0];
			cChIdxMax = gTx_Pwr_Limit_2g_Ch[
					TX_PWR_LIMIT_2G_CH_NUM - 1];
			break;
		case PWR_CTRL_CHNL_TYPE_5G:
			cChIdxMin = gTx_Pwr_Limit_5g_Ch[0];
			cChIdxMax = gTx_Pwr_Limit_5g_Ch[
					TX_PWR_LIMIT_5G_CH_NUM - 1];
			break;
		case PWR_CTRL_CHNL_TYPE_BANDEDGE_2G4:
			/* Not support now */
			cChIdxMin = 0;
			cChIdxMax = 0;
			break;
		case PWR_CTRL_CHNL_TYPE_BANDEDGE_5G:
			/* Not support now */
			cChIdxMin = 0;
			cChIdxMax = 0;
			break;
		case PWR_CTRL_CHNL_TYPE_5G_BAND1:
			cChIdxMin = TX_PWR_LIMIT_5G_BAND1_L;
			cChIdxMax = TX_PWR_LIMIT_5G_BAND1_H;
			break;
		case PWR_CTRL_CHNL_TYPE_5G_BAND2:
			cChIdxMin = TX_PWR_LIMIT_5G_BAND2_L;
			cChIdxMax = TX_PWR_LIMIT_5G_BAND2_H;
			break;
		case PWR_CTRL_CHNL_TYPE_5G_BAND3:
			cChIdxMin = TX_PWR_LIMIT_5G_BAND3_L;
			cChIdxMax = TX_PWR_LIMIT_5G_BAND3_H;
			break;
		case PWR_CTRL_CHNL_TYPE_5G_BAND4:
			cChIdxMin = TX_PWR_LIMIT_5G_BAND4_L;
			cChIdxMax = TX_PWR_LIMIT_5G_BAND4_H;
			break;
#if (CFG_SUPPORT_WIFI_6G == 1)
		case PWR_CTRL_CHNL_TYPE_6G:
			cChIdxMin = gTx_Pwr_Limit_6g_Ch[0];
			cChIdxMax = gTx_Pwr_Limit_6g_Ch[
					TX_PWR_LIMIT_6G_CH_NUM - 1];
			break;
		case PWR_CTRL_CHNL_TYPE_6G_BAND1:
			cChIdxMin = TX_PWR_LIMIT_6G_BAND1_L;
			cChIdxMax = TX_PWR_LIMIT_6G_BAND1_H;
			break;
		case PWR_CTRL_CHNL_TYPE_6G_BAND2:
			cChIdxMin = TX_PWR_LIMIT_6G_BAND2_L;
			cChIdxMax = TX_PWR_LIMIT_6G_BAND2_H;
			break;
		case PWR_CTRL_CHNL_TYPE_6G_BAND3:
			cChIdxMin = TX_PWR_LIMIT_6G_BAND3_L;
			cChIdxMax = TX_PWR_LIMIT_6G_BAND3_H;
			break;
		case PWR_CTRL_CHNL_TYPE_6G_BAND4:
			cChIdxMin = TX_PWR_LIMIT_6G_BAND4_L;
			cChIdxMax = TX_PWR_LIMIT_6G_BAND4_H;
			break;
#endif
		default:
			cChIdxMin = 0;
			cChIdxMax = 0;
			break;
		}

		for (j = 0; j < pTxPwrLimitData->ucChNum; j++) {
			prChLimit = &pTxPwrLimitData->rChannelTxPwrLimit[j];
			cChIdx = prChLimit->ucChannel;
			if (cChIdx < cChIdxMin || cChIdx > cChIdxMax)
				continue;
			rlmDomainUpdateChannelTxPwrLimit(ucVersion,
							 fgIs6GBand,
							 prChLimit,
							 prChSetting);
		}

		for (j = 0; j < pTxPwrLegacyLimitData->ucChNum; j++) {
			prLegacyChLimit =
			    &pTxPwrLegacyLimitData->rChannelTxLegacyPwrLimit[j];
			cChIdx = prLegacyChLimit->ucChannel;
			if (cChIdx < cChIdxMin || cChIdx > cChIdxMax)
				continue;
			rlmDomainUpdateChannelTxLegacyPwrLimit(ucVersion,
							       fgIs6GBand,
							       prLegacyChLimit,
							       prChSetting);
		}
	}
}

static u_int8_t rlmDomainApplySarTxPwrLimit(
	uint8_t ucVersion,
	u_int8_t fgIs6GBand,
	struct ADAPTER *prAdapter,
	struct TX_PWR_LIMIT_DATA *pTxPwrLimitData,
	struct TX_PWR_LEGACY_LIMIT_DATA *pTxPwrLegacyLimitData)

{
	struct LINK_ENTRY *prCur, *prNext;
	struct TX_PWR_CTRL_ELEMENT *pcElement = NULL;

	if (prAdapter == NULL ||
	    pTxPwrLimitData == NULL ||
	    pTxPwrLegacyLimitData == NULL)
		return FALSE;

	if (ucVersion != PWR_CFG_PARM_VERSION) {
		DBGLOG(RLM, ERROR,
		       "TxPwrLimit version not match(%d != %d)\n",
		       ucVersion, PWR_CFG_PARM_VERSION);
		return FALSE;
	}

	DBGLOG(RLM, INFO, "ApplySarTxPwrLimit\n");
	LINK_FOR_EACH_SAFE(prCur, prNext, &prAdapter->rTxPwr_DynamicList) {
		pcElement = LINK_ENTRY(prCur, struct TX_PWR_CTRL_ELEMENT, node);
		if (pcElement->fgApplied)
			rlmDomainUpdateTxPwrLimitData(ucVersion,
						      fgIs6GBand,
						      pcElement,
						      pTxPwrLimitData,
						      pTxPwrLegacyLimitData);
	}

	return TRUE;
}

#if CFG_SUPPORT_DYNAMIC_PWR_LIMIT_ANT_TAG
static u_int8_t rlmDomainUpdateAntPwrLimitData(
	struct TX_PWR_CTRL_ELEMENT *pcElement,
	struct CMD_TXPOWER_ANT_POWER_LIMIT *pAntPwrLimitData)
{
	uint32_t i, j;
	struct TX_PWR_CTRL_ANT_SETTING *prPwrant = NULL;
	struct CMD_ANT_TABLE_TYPE *prAntPowerLimit = NULL;

	if (pcElement == NULL ||
	    pAntPwrLimitData == NULL)
		return FALSE;

	DBGLOG(RLM, INFO, "UpdateAntPwrLimitData\n");
	for (i = POWER_ANT_ALL_T; i < POWER_ANT_TAG_NUM; i++) {
		prPwrant = &pcElement->aiPwrAnt[i];
		if (!prPwrant->fgApplied)
			continue;

		if (pAntPwrLimitData->ucAntNum > POWER_ANT_NUM)
			pAntPwrLimitData->ucAntNum = POWER_ANT_NUM;

		for (j = 0; j < pAntPwrLimitData->ucAntNum; j++) {
			prAntPowerLimit = &pAntPwrLimitData->rAntPowerLimit[j];
			prAntPowerLimit->aucAntPowerLimit[0] =
				MIN(prAntPowerLimit->aucAntPowerLimit[0],
				    prPwrant->aiPwrAnt2G4[j]);
			prAntPowerLimit->aucAntPowerLimit[1] =
				MIN(prAntPowerLimit->aucAntPowerLimit[1],
				    prPwrant->aiPwrAnt5GB1[j]);
			prAntPowerLimit->aucAntPowerLimit[2] =
				MIN(prAntPowerLimit->aucAntPowerLimit[2],
				    prPwrant->aiPwrAnt5GB2[j]);
			prAntPowerLimit->aucAntPowerLimit[3] =
				MIN(prAntPowerLimit->aucAntPowerLimit[3],
				    prPwrant->aiPwrAnt5GB3[j]);
			prAntPowerLimit->aucAntPowerLimit[4] =
				MIN(prAntPowerLimit->aucAntPowerLimit[4],
				    prPwrant->aiPwrAnt5GB4[j]);
#if (CFG_SUPPORT_WIFI_6G == 1)

			prAntPowerLimit->aucAntPowerLimit[5] =
				MIN(prAntPowerLimit->aucAntPowerLimit[5],
				    prPwrant->aiPwrAnt6GB1[j]);
			prAntPowerLimit->aucAntPowerLimit[6] =
				MIN(prAntPowerLimit->aucAntPowerLimit[6],
				    prPwrant->aiPwrAnt6GB2[j]);
			prAntPowerLimit->aucAntPowerLimit[7] =
				MIN(prAntPowerLimit->aucAntPowerLimit[7],
				    prPwrant->aiPwrAnt6GB3[j]);
			prAntPowerLimit->aucAntPowerLimit[8] =
				MIN(prAntPowerLimit->aucAntPowerLimit[8],
				    prPwrant->aiPwrAnt6GB4[j]);
#endif
			DBGLOG(RLM, TRACE,
			       "ANT SAR (ANT %d: %d %d %d %d %d %d %d %d %d)\n",
			       j,
			       prAntPowerLimit->aucAntPowerLimit[0],
			       prAntPowerLimit->aucAntPowerLimit[1],
			       prAntPowerLimit->aucAntPowerLimit[2],
			       prAntPowerLimit->aucAntPowerLimit[3],
			       prAntPowerLimit->aucAntPowerLimit[4],
			       prAntPowerLimit->aucAntPowerLimit[5],
			       prAntPowerLimit->aucAntPowerLimit[6],
			       prAntPowerLimit->aucAntPowerLimit[7],
			       prAntPowerLimit->aucAntPowerLimit[8]);
		}
	}
	return TRUE;
}

static u_int8_t rlmDomainApplySarAntPwrLimit(
	struct ADAPTER *prAdapter,
	struct CMD_TXPOWER_ANT_POWER_LIMIT *pAntPwrLimitData)
{
	struct LINK_ENTRY *prCur, *prNext;
	struct TX_PWR_CTRL_ELEMENT *pcElement = NULL;

	if (prAdapter == NULL ||
	    pAntPwrLimitData == NULL)
		return FALSE;

	DBGLOG(RLM, INFO, "ApplySarAntPwrLimit\n");
	LINK_FOR_EACH_SAFE(prCur, prNext, &prAdapter->rTxPwr_DynamicList) {
		pcElement = LINK_ENTRY(prCur, struct TX_PWR_CTRL_ELEMENT, node);
		if (pcElement->fgApplied)
			rlmDomainUpdateAntPwrLimitData(pcElement,
						       pAntPwrLimitData);
	}
	return TRUE;
}
#endif  /* #if CFG_SUPPORT_DYNAMIC_PWR_LIMIT_ANT_TAG */
#endif  /* #if CFG_SUPPORT_DYNAMIC_PWR_LIMIT */
#endif  /* #if (CFG_SUPPORT_SINGLE_SKU == 1) */

#if CFG_SUPPORT_PWR_LIMIT_COUNTRY

/*----------------------------------------------------------------------------*/
/*!
 * @brief Check if power limit setting is in the range [MIN_TX_POWER,
 *        MAX_TX_POWER]
 *
 * @param[in]
 *
 * @return (fgValid) : 0 -> inValid, 1 -> Valid
 */
/*----------------------------------------------------------------------------*/
u_int8_t
rlmDomainCheckPowerLimitValid(struct ADAPTER *prAdapter,
			      struct COUNTRY_POWER_LIMIT_TABLE_CONFIGURATION
						rPowerLimitTableConfiguration,
			      uint8_t ucPwrLimitNum)
{
	uint8_t i;
	u_int8_t fgValid = TRUE;
	int8_t *prPwrLimit;

	prPwrLimit = &rPowerLimitTableConfiguration.aucPwrLimit[0];

	for (i = 0; i < ucPwrLimitNum; i++, prPwrLimit++) {
		if (*prPwrLimit > MAX_TX_POWER || *prPwrLimit < MIN_TX_POWER) {
			fgValid = FALSE;
			break;	/*Find out Wrong Power limit */
		}
	}
	return fgValid;

}

/*----------------------------------------------------------------------------*/
/*!
 * @brief 1.Check if power limit configuration table valid(channel intervel)
 *	2.Check if power limit configuration/default table entry repeat
 *
 * @param[in]
 *
 * @return (none)
 */
/*----------------------------------------------------------------------------*/
void rlmDomainCheckCountryPowerLimitTable(struct ADAPTER *prAdapter)
{
#define PwrLmtConf g_rRlmPowerLimitConfiguration
	uint8_t i, j;
	uint16_t u2CountryCodeTable, u2CountryCodeCheck;
	u_int8_t fgChannelValid = FALSE;
	u_int8_t fgPowerLimitValid = FALSE;
	u_int8_t fgEntryRepetetion = FALSE;
	u_int8_t fgTableValid = TRUE;
	uint8_t limitValueStr[128];

	/*1.Configuration Table Check */
	for (i = 0; i < sizeof(PwrLmtConf) /
	     sizeof(struct COUNTRY_POWER_LIMIT_TABLE_CONFIGURATION); i++) {
		/*Table Country Code */
		WLAN_GET_FIELD_BE16(&PwrLmtConf[i].aucCountryCode[0],
				    &u2CountryCodeTable);

		/*<1>Repetition Entry Check */
		for (j = i + 1; j < sizeof(PwrLmtConf) /
		     sizeof(struct COUNTRY_POWER_LIMIT_TABLE_CONFIGURATION);
		     j++) {

			WLAN_GET_FIELD_BE16(&PwrLmtConf[j].aucCountryCode[0],
					    &u2CountryCodeCheck);
			if (((PwrLmtConf[i].ucCentralCh) ==
			     PwrLmtConf[j].ucCentralCh)
			    && (u2CountryCodeTable == u2CountryCodeCheck)) {
				fgEntryRepetetion = TRUE;
				DBGLOG(RLM, LOUD,
				       "Domain: Configuration Repetition CC=%c%c, Ch=%d\n",
				       PwrLmtConf[i].aucCountryCode[0],
				       PwrLmtConf[i].aucCountryCode[1],
				       PwrLmtConf[i].ucCentralCh);
			}
		}

		/*<2>Channel Number Interval Check */
		fgChannelValid =
		    rlmDomainCheckChannelEntryValid(prAdapter,
						    PwrLmtConf[i].ucCentralCh);

		/*<3>Power Limit Range Check */
		fgPowerLimitValid =
		    rlmDomainCheckPowerLimitValid(prAdapter,
						  PwrLmtConf[i],
						  PWR_LIMIT_NUM);

		if (fgChannelValid == FALSE || fgPowerLimitValid == FALSE) {
			fgTableValid = FALSE;
			kalMemSet(limitValueStr, 0, sizeof(limitValueStr));
			for (j = 0; j < PWR_LIMIT_NUM; j++) {
				kalSnprintf(limitValueStr +
						kalStrLen(limitValueStr),
					sizeof(limitValueStr) -
						kalStrLen(limitValueStr) - 1,
					" %d,", PwrLmtConf[i].aucPwrLimit[j]);
			}
			DBGLOG(RLM, LOUD,
			       "Domain: CC=%c%c, Ch=%d, Limit:%s Valid:%d,%d\n",
			       PwrLmtConf[i].aucCountryCode[0],
			       PwrLmtConf[i].aucCountryCode[1],
			       PwrLmtConf[i].ucCentralCh,
			       limitValueStr,
			       fgChannelValid,
			       fgPowerLimitValid);
		}

		if (u2CountryCodeTable == COUNTRY_CODE_NULL) {
			DBGLOG(RLM, LOUD, "Domain: Full search down\n");
			break;	/*End of country table entry */
		}

	}

	if (fgEntryRepetetion == FALSE)
		DBGLOG(RLM, TRACE,
		       "Domain: Configuration Table no Repetiton.\n");

	/*Configuration Table no error */
	if (fgTableValid == TRUE)
		prAdapter->fgIsPowerLimitTableValid = TRUE;
	else
		prAdapter->fgIsPowerLimitTableValid = FALSE;

	/*2.Default Table Repetition Entry Check */
	fgEntryRepetetion = FALSE;
	for (i = 0; i < sizeof(g_rRlmPowerLimitDefault) /
	     sizeof(struct COUNTRY_POWER_LIMIT_TABLE_DEFAULT); i++) {

		WLAN_GET_FIELD_BE16(&g_rRlmPowerLimitDefault[i].
							aucCountryCode[0],
				    &u2CountryCodeTable);

		for (j = i + 1; j < sizeof(g_rRlmPowerLimitDefault) /
		     sizeof(struct COUNTRY_POWER_LIMIT_TABLE_DEFAULT); j++) {
			WLAN_GET_FIELD_BE16(&g_rRlmPowerLimitDefault[j].
							aucCountryCode[0],
					    &u2CountryCodeCheck);
			if (u2CountryCodeTable == u2CountryCodeCheck) {
				fgEntryRepetetion = TRUE;
				DBGLOG(RLM, LOUD,
				       "Domain: Default Repetition CC=%c%c\n",
				       g_rRlmPowerLimitDefault[j].
							aucCountryCode[0],
				       g_rRlmPowerLimitDefault[j].
							aucCountryCode[1]);
			}
		}
	}
	if (fgEntryRepetetion == FALSE)
		DBGLOG(RLM, TRACE, "Domain: Default Table no Repetiton.\n");
#undef PwrLmtConf
}

/*----------------------------------------------------------------------------*/
/*!
 * @brief
 *
 * @param[in]
 *
 * @return (u2TableIndex) : if  0xFFFF -> No Table Match
 */
/*----------------------------------------------------------------------------*/
uint16_t rlmDomainPwrLimitDefaultTableDecision(struct ADAPTER *prAdapter,
					       uint16_t u2CountryCode)
{

	uint16_t i;
	uint16_t u2CountryCodeTable = COUNTRY_CODE_NULL;
	uint16_t u2TableIndex = POWER_LIMIT_TABLE_NULL;	/* No Table Match */

	/*Default Table Index */
	for (i = 0; i < sizeof(g_rRlmPowerLimitDefault) /
	     sizeof(struct COUNTRY_POWER_LIMIT_TABLE_DEFAULT); i++) {

		WLAN_GET_FIELD_BE16(&g_rRlmPowerLimitDefault[i].
						aucCountryCode[0],
				    &u2CountryCodeTable);

		if (u2CountryCodeTable == u2CountryCode) {
			u2TableIndex = i;
			break;	/*match country code */
		} else if (u2CountryCodeTable == COUNTRY_CODE_NULL) {
			u2TableIndex = i;
			break;	/*find last one country- Default */
		}
	}

	return u2TableIndex;
}

/*----------------------------------------------------------------------------*/
/*!
 * @brief Fill power limit CMD by Power Limit Default Table(regulation)
 *
 * @param[in]
 *
 * @return (none)
 */
/*----------------------------------------------------------------------------*/
void
rlmDomainBuildCmdByDefaultTable(struct CMD_SET_COUNTRY_CHANNEL_POWER_LIMIT
				*prCmd,
				uint16_t u2DefaultTableIndex)
{
	uint8_t i, k;
	struct COUNTRY_POWER_LIMIT_TABLE_DEFAULT *prPwrLimitSubBand;
	struct CMD_CHANNEL_POWER_LIMIT *prCmdPwrLimit;

	prCmdPwrLimit = &prCmd->rChannelPowerLimit[0];
	prPwrLimitSubBand = &g_rRlmPowerLimitDefault[u2DefaultTableIndex];

	/*Build power limit cmd by default table information */

	for (i = POWER_LIMIT_2G4; i < POWER_LIMIT_SUBAND_NUM; i++) {
		if (prPwrLimitSubBand->aucPwrLimitSubBand[i] <= MAX_TX_POWER) {
			for (k = g_rRlmSubBand[i].ucStartCh;
			     k <= g_rRlmSubBand[i].ucEndCh;
			     k += g_rRlmSubBand[i].ucInterval) {
				if ((prPwrLimitSubBand->ucPwrUnit
							& BIT(i)) == 0) {
					prCmdPwrLimit->ucCentralCh = k;
					kalMemSet(&prCmdPwrLimit->cPwrLimitCCK,
						  prPwrLimitSubBand->
							aucPwrLimitSubBand[i],
						  PWR_LIMIT_NUM);
				} else {
					/* ex: 40MHz power limit(mW\MHz)
					 *	= 20MHz power limit(mW\MHz) * 2
					 * ---> 40MHz power limit(dBm)
					 *	= 20MHz power limit(dBm) + 6;
					 */
					prCmdPwrLimit->ucCentralCh = k;
					/* BW20 */
					prCmdPwrLimit->cPwrLimitCCK =
							prPwrLimitSubBand->
							  aucPwrLimitSubBand[i];
					prCmdPwrLimit->cPwrLimit20L =
							prPwrLimitSubBand->
							  aucPwrLimitSubBand[i];
					prCmdPwrLimit->cPwrLimit20H =
							prPwrLimitSubBand->
							  aucPwrLimitSubBand[i];

					/* BW40 */
					if (prPwrLimitSubBand->
						aucPwrLimitSubBand[i] + 6 >
								MAX_TX_POWER) {
						prCmdPwrLimit->cPwrLimit40L =
								   MAX_TX_POWER;
						prCmdPwrLimit->cPwrLimit40H =
								   MAX_TX_POWER;
					} else {
						prCmdPwrLimit->cPwrLimit40L =
							prPwrLimitSubBand->
							aucPwrLimitSubBand[i]
							+ 6;
						prCmdPwrLimit->cPwrLimit40H =
							prPwrLimitSubBand->
							aucPwrLimitSubBand[i]
							+ 6;
					}

					/* BW80 */
					if (prPwrLimitSubBand->
						aucPwrLimitSubBand[i] + 12 >
								MAX_TX_POWER) {
						prCmdPwrLimit->cPwrLimit80L =
								MAX_TX_POWER;
						prCmdPwrLimit->cPwrLimit80H =
								MAX_TX_POWER;
					} else {
						prCmdPwrLimit->cPwrLimit80L =
							prPwrLimitSubBand->
							aucPwrLimitSubBand[i]
							+ 12;
						prCmdPwrLimit->cPwrLimit80H =
							prPwrLimitSubBand->
							aucPwrLimitSubBand[i]
							+ 12;
					}

					/* BW160 */
					if (prPwrLimitSubBand->
						aucPwrLimitSubBand[i] + 18 >
								MAX_TX_POWER) {
						prCmdPwrLimit->cPwrLimit160L =
								MAX_TX_POWER;
						prCmdPwrLimit->cPwrLimit160H =
								MAX_TX_POWER;
					} else {
						prCmdPwrLimit->cPwrLimit160L =
							prPwrLimitSubBand->
							aucPwrLimitSubBand[i]
							+ 18;
						prCmdPwrLimit->cPwrLimit160H =
							prPwrLimitSubBand->
							aucPwrLimitSubBand[i]
							+ 18;
					}

				}
				/* save to power limit array per
				 * subband channel
				 */
				prCmdPwrLimit++;
				prCmd->ucNum++;
			}
		}
	}
}

/*----------------------------------------------------------------------------*/
/*!
 * @brief Fill power limit CMD by Power Limit Configurartion Table
 * (Bandedge and Customization)
 *
 * @param[in]
 *
 * @return (none)
 */
/*----------------------------------------------------------------------------*/
void rlmDomainBuildCmdByConfigTable(struct ADAPTER *prAdapter,
			struct CMD_SET_COUNTRY_CHANNEL_POWER_LIMIT *prCmd)
{
#define PwrLmtConf g_rRlmPowerLimitConfiguration
	uint8_t i, k;
	uint16_t u2CountryCodeTable = COUNTRY_CODE_NULL;
	struct CMD_CHANNEL_POWER_LIMIT *prCmdPwrLimit;
	u_int8_t fgChannelValid;

	/*Build power limit cmd by configuration table information */

	for (i = 0; i < sizeof(PwrLmtConf) /
			sizeof(struct COUNTRY_POWER_LIMIT_TABLE_CONFIGURATION);
	     i++) {

		WLAN_GET_FIELD_BE16(&PwrLmtConf[i].aucCountryCode[0],
				    &u2CountryCodeTable);

		fgChannelValid =
		    rlmDomainCheckChannelEntryValid(prAdapter,
						    PwrLmtConf[i].ucCentralCh);

		if (u2CountryCodeTable == COUNTRY_CODE_NULL) {
			break;	/*end of configuration table */
		} else if ((u2CountryCodeTable == prCmd->u2CountryCode)
				&& (fgChannelValid == TRUE)) {

			prCmdPwrLimit = &prCmd->rChannelPowerLimit[0];

			if (prCmd->ucNum != 0) {
				for (k = 0; k < prCmd->ucNum; k++) {
					if (prCmdPwrLimit->ucCentralCh ==
						PwrLmtConf[i].ucCentralCh) {

						/* Cmd setting (Default table
						 * information) and Conf table
						 * has repetition channel entry,
						 * ex : Default table (ex: 2.4G,
						 *      limit = 20dBm) -->
						 *      ch1~14 limit =20dBm,
						 * Conf table (ex: ch1, limit =
						 *      22dBm) --> ch 1 = 22 dBm
						 * Cmd final setting --> ch1 =
						 *      22dBm, ch2~14 = 20dBm
						 */
						kalMemCopy(&prCmdPwrLimit->
								cPwrLimitCCK,
							   &PwrLmtConf[i].
								aucPwrLimit,
							   PWR_LIMIT_NUM);

						DBGLOG(RLM, LOUD,
						       "Domain: CC=%c%c,ReplaceCh=%d,Limit=%d,%d,%d,%d,%d,%d,%d,%d,%d,Fg=%d\n",
						       ((prCmd->u2CountryCode &
								0xff00) >> 8),
						       (prCmd->u2CountryCode &
									0x00ff),
						       prCmdPwrLimit->
								ucCentralCh,
						       prCmdPwrLimit->
								cPwrLimitCCK,
						       prCmdPwrLimit->
								cPwrLimit20L,
						       prCmdPwrLimit->
								cPwrLimit20H,
						       prCmdPwrLimit->
								cPwrLimit40L,
						       prCmdPwrLimit->
								cPwrLimit40H,
						       prCmdPwrLimit->
								cPwrLimit80L,
						       prCmdPwrLimit->
								cPwrLimit80H,
						       prCmdPwrLimit->
								cPwrLimit160L,
						       prCmdPwrLimit->
								cPwrLimit160H,
						       prCmdPwrLimit->ucFlag);

						break;
					}
					/* To search next entry in
					 * rChannelPowerLimit[k]
					 */
					prCmdPwrLimit++;
				}
				if (k == prCmd->ucNum) {

					/* Full search cmd(Default table
					 * setting) no match channel,
					 *  ex : Default table (ex: 2.4G, limit
					 *       =20dBm) -->ch1~14 limit =20dBm,
					 *  Configuration table(ex: ch36, limit
					 *       =22dBm) -->ch 36 = 22 dBm
					 *  Cmd final setting -->
					 *       ch1~14 = 20dBm, ch36 = 22dBm
					 */
					prCmdPwrLimit->ucCentralCh =
						PwrLmtConf[i].ucCentralCh;
					kalMemCopy(&prCmdPwrLimit->cPwrLimitCCK,
					      &PwrLmtConf[i].aucPwrLimit,
					      PWR_LIMIT_NUM);
					/* Add this channel setting in
					 * rChannelPowerLimit[k]
					 */
					prCmd->ucNum++;

					DBGLOG(RLM, LOUD,
					       "Domain: CC=%c%c,AddCh=%d,Limit=%d,%d,%d,%d,%d,%d,%d,%d,%d,Fg=%d\n",
					       ((prCmd->u2CountryCode & 0xff00)
									>> 8),
					       (prCmd->u2CountryCode & 0x00ff),
					       prCmdPwrLimit->ucCentralCh,
					       prCmdPwrLimit->cPwrLimitCCK,
					       prCmdPwrLimit->cPwrLimit20L,
					       prCmdPwrLimit->cPwrLimit20H,
					       prCmdPwrLimit->cPwrLimit40L,
					       prCmdPwrLimit->cPwrLimit40H,
					       prCmdPwrLimit->cPwrLimit80L,
					       prCmdPwrLimit->cPwrLimit80H,
					       prCmdPwrLimit->cPwrLimit160L,
					       prCmdPwrLimit->cPwrLimit160H,
					       prCmdPwrLimit->ucFlag);

				}
			} else {

				/* Default table power limit value are max on
				 * all subbands --> cmd table no channel entry
				 *  ex : Default table (ex: 2.4G, limit = 63dBm)
				 *  --> no channel entry in cmd,
				 *  Configuration table(ex: ch36, limit = 22dBm)
				 *  --> ch 36 = 22 dBm
				 *  Cmd final setting -->  ch36 = 22dBm
				 */
				prCmdPwrLimit->ucCentralCh =
						PwrLmtConf[i].ucCentralCh;
				kalMemCopy(&prCmdPwrLimit->cPwrLimitCCK,
					   &PwrLmtConf[i].aucPwrLimit,
					   PWR_LIMIT_NUM);
				/* Add this channel setting in
				 * rChannelPowerLimit[k]
				 */
				prCmd->ucNum++;

				DBGLOG(RLM, LOUD,
				       "Domain: Default table power limit value are max on all subbands.\n");
				DBGLOG(RLM, LOUD,
				       "Domain: CC=%c%c,AddCh=%d,Limit=%d,%d,%d,%d,%d,%d,%d,%d,%d,Fg=%d\n",
				       ((prCmd->u2CountryCode & 0xff00) >> 8),
				       (prCmd->u2CountryCode & 0x00ff),
				       prCmdPwrLimit->ucCentralCh,
				       prCmdPwrLimit->cPwrLimitCCK,
				       prCmdPwrLimit->cPwrLimit20L,
				       prCmdPwrLimit->cPwrLimit20H,
				       prCmdPwrLimit->cPwrLimit40L,
				       prCmdPwrLimit->cPwrLimit40H,
				       prCmdPwrLimit->cPwrLimit80L,
				       prCmdPwrLimit->cPwrLimit80H,
				       prCmdPwrLimit->cPwrLimit160L,
				       prCmdPwrLimit->cPwrLimit160H,
				       prCmdPwrLimit->ucFlag);
			}
		}
	}
#undef PwrLmtConf
}

#if (CFG_SUPPORT_SINGLE_SKU == 1)
#if (CFG_SUPPORT_WIFI_6G == 1)
struct TX_PWR_LIMIT_DATA *
rlmDomainInitTxPwrLimitData_6G(struct ADAPTER *prAdapter)
{
	uint8_t ch_cnt = 0;
	uint8_t ch_idx = 0;
	uint8_t ch_num = 0;
	const int8_t *prChannelList = NULL;
	struct TX_PWR_LIMIT_DATA *pTxPwrLimitData = NULL;
	struct CHANNEL_TX_PWR_LIMIT *prChTxPwrLimit = NULL;

	pTxPwrLimitData =
		(struct TX_PWR_LIMIT_DATA *)
		kalMemAlloc(sizeof(struct TX_PWR_LIMIT_DATA),
			VIR_MEM_TYPE);

	if (!pTxPwrLimitData) {
		DBGLOG(RLM, ERROR,
			"Alloc buffer for TxPwrLimit main struct failed\n");
		return NULL;
	}

	pTxPwrLimitData->ucChNum = TX_PWR_LIMIT_6G_CH_NUM;

	pTxPwrLimitData->rChannelTxPwrLimit =
		(struct CHANNEL_TX_PWR_LIMIT *)
		kalMemAlloc(sizeof(struct CHANNEL_TX_PWR_LIMIT) *
			(pTxPwrLimitData->ucChNum), VIR_MEM_TYPE);

	if (!pTxPwrLimitData->rChannelTxPwrLimit) {
		DBGLOG(RLM, ERROR,
			"Alloc buffer for TxPwrLimit ch values failed\n");

		kalMemFree(pTxPwrLimitData, VIR_MEM_TYPE,
			sizeof(struct TX_PWR_LIMIT_DATA));
		return NULL;
	}

	for (ch_idx = 0; ch_idx < pTxPwrLimitData->ucChNum; ch_idx++) {
		prChTxPwrLimit =
			&(pTxPwrLimitData->rChannelTxPwrLimit[ch_idx]);
		kalMemSet(prChTxPwrLimit->rTxPwrLimitValue,
			MAX_TX_POWER,
			sizeof(prChTxPwrLimit->rTxPwrLimitValue));
		kalMemSet(prChTxPwrLimit->rTxBfBackoff,
			MAX_TX_POWER,
			sizeof(prChTxPwrLimit->rTxBfBackoff));
	}

	ch_cnt = 0;

	prChannelList = gTx_Pwr_Limit_6g_Ch;
	ch_num = TX_PWR_LIMIT_6G_CH_NUM;

	for (ch_idx = 0; ch_idx < ch_num; ch_idx++) {
		pTxPwrLimitData->rChannelTxPwrLimit[ch_cnt].ucChannel =
			prChannelList[ch_idx];
		++ch_cnt;
	}

	return pTxPwrLimitData;
}

struct TX_PWR_LEGACY_LIMIT_DATA *
rlmDomainInitTxLegacyPwrLimitData_6G(struct ADAPTER *prAdapter)
{
	uint8_t ch_cnt = 0;
	uint8_t ch_idx = 0;
	uint8_t ch_num = 0;
	const int8_t *prChannelList = NULL;
	struct TX_PWR_LEGACY_LIMIT_DATA *pTxLegacyPwrLimitData = NULL;
	struct CHANNEL_TX_LEGACY_PWR_LIMIT *prChTxLegacyPwrLimit = NULL;

	pTxLegacyPwrLimitData =
		(struct TX_PWR_LEGACY_LIMIT_DATA *)
		kalMemAlloc(sizeof(struct TX_PWR_LEGACY_LIMIT_DATA),
			VIR_MEM_TYPE);

	if (!pTxLegacyPwrLimitData) {
		DBGLOG(RLM, ERROR,
			"Alloc buffer for TxLegacyPwrLimit main struct failed\n");
		return NULL;
	}

	pTxLegacyPwrLimitData->ucChNum = TX_PWR_LIMIT_6G_CH_NUM;

	pTxLegacyPwrLimitData->rChannelTxLegacyPwrLimit =
		(struct CHANNEL_TX_LEGACY_PWR_LIMIT *)
		kalMemAlloc(sizeof(struct CHANNEL_TX_LEGACY_PWR_LIMIT) *
			(pTxLegacyPwrLimitData->ucChNum), VIR_MEM_TYPE);

	if (!pTxLegacyPwrLimitData->rChannelTxLegacyPwrLimit) {
		DBGLOG(RLM, ERROR,
			"Alloc buffer for TxLegacyPwrLimit ch values failed\n");

		kalMemFree(pTxLegacyPwrLimitData, VIR_MEM_TYPE,
			sizeof(struct TX_PWR_LEGACY_LIMIT_DATA));
		return NULL;
	}

	for (ch_idx = 0; ch_idx < pTxLegacyPwrLimitData->ucChNum; ch_idx++) {
		prChTxLegacyPwrLimit =
			&(pTxLegacyPwrLimitData->
			rChannelTxLegacyPwrLimit[ch_idx]);
		kalMemSet(prChTxLegacyPwrLimit->rTxLegacyPwrLimitValue,
			MAX_TX_POWER,
			sizeof(prChTxLegacyPwrLimit->rTxLegacyPwrLimitValue));
	}

	ch_cnt = 0;

	prChannelList = gTx_Pwr_Limit_6g_Ch;
	ch_num = TX_PWR_LIMIT_6G_CH_NUM;

	for (ch_idx = 0; ch_idx < ch_num; ch_idx++) {
		pTxLegacyPwrLimitData->
			rChannelTxLegacyPwrLimit[ch_cnt].ucChannel =
			prChannelList[ch_idx];
		++ch_cnt;
	}

	return pTxLegacyPwrLimitData;
}
#endif /* #if (CFG_SUPPORT_WIFI_6G == 1) */

struct TX_PWR_LIMIT_DATA *
rlmDomainInitTxPwrLimitData(struct ADAPTER *prAdapter)
{
	uint8_t ch_cnt = 0;
	uint8_t ch_idx = 0;
	uint8_t band_idx = 0;
	uint8_t ch_num = 0;
	const int8_t *prChannelList = NULL;
	struct TX_PWR_LIMIT_DATA *pTxPwrLimitData = NULL;
	struct CHANNEL_TX_PWR_LIMIT *prChTxPwrLimit = NULL;

	pTxPwrLimitData =
		(struct TX_PWR_LIMIT_DATA *)
		kalMemAlloc(sizeof(struct TX_PWR_LIMIT_DATA),
			VIR_MEM_TYPE);

	if (!pTxPwrLimitData) {
		DBGLOG(RLM, ERROR,
			"Alloc buffer for TxPwrLimit main struct failed\n");
		return NULL;
	}

	pTxPwrLimitData->ucChNum =
		TX_PWR_LIMIT_2G_CH_NUM + TX_PWR_LIMIT_5G_CH_NUM;

	pTxPwrLimitData->rChannelTxPwrLimit =
		(struct CHANNEL_TX_PWR_LIMIT *)
		kalMemAlloc(sizeof(struct CHANNEL_TX_PWR_LIMIT) *
			(pTxPwrLimitData->ucChNum), VIR_MEM_TYPE);

	if (!pTxPwrLimitData->rChannelTxPwrLimit) {
		DBGLOG(RLM, ERROR,
			"Alloc buffer for TxPwrLimit ch values failed\n");

		kalMemFree(pTxPwrLimitData, VIR_MEM_TYPE,
			sizeof(struct TX_PWR_LIMIT_DATA));
		return NULL;
	}

	for (ch_idx = 0; ch_idx < pTxPwrLimitData->ucChNum; ch_idx++) {
		prChTxPwrLimit =
			&(pTxPwrLimitData->rChannelTxPwrLimit[ch_idx]);
		kalMemSet(prChTxPwrLimit->rTxPwrLimitValue,
			MAX_TX_POWER,
			sizeof(prChTxPwrLimit->rTxPwrLimitValue));
		kalMemSet(prChTxPwrLimit->rTxBfBackoff,
			MAX_TX_POWER,
			sizeof(prChTxPwrLimit->rTxBfBackoff));
	}

	ch_cnt = 0;
	for (band_idx = 0; band_idx < KAL_NUM_BANDS; band_idx++) {
		if (band_idx != KAL_BAND_2GHZ && band_idx != KAL_BAND_5GHZ)
			continue;

		prChannelList = (band_idx == KAL_BAND_2GHZ) ?
			gTx_Pwr_Limit_2g_Ch : gTx_Pwr_Limit_5g_Ch;

		ch_num =
			(band_idx == KAL_BAND_2GHZ) ?
				TX_PWR_LIMIT_2G_CH_NUM :
				TX_PWR_LIMIT_5G_CH_NUM;

		for (ch_idx = 0; ch_idx < ch_num; ch_idx++) {
			pTxPwrLimitData->rChannelTxPwrLimit[ch_cnt].ucChannel =
				prChannelList[ch_idx];
			++ch_cnt;
		}
	}

	return pTxPwrLimitData;
}

struct TX_PWR_LEGACY_LIMIT_DATA *
rlmDomainInitTxPwrLegacyLimitData(struct ADAPTER *prAdapter)
{
	uint8_t ch_cnt = 0;
	uint8_t ch_idx = 0;
	uint8_t band_idx = 0;
	uint8_t ch_num = 0;
	const int8_t *prChannelList = NULL;
	struct TX_PWR_LEGACY_LIMIT_DATA *pTxPwrLegacyLimitData = NULL;
	struct CHANNEL_TX_LEGACY_PWR_LIMIT *prChTxLegPwrLimit = NULL;

	pTxPwrLegacyLimitData =
		(struct TX_PWR_LEGACY_LIMIT_DATA *)
		kalMemAlloc(sizeof(struct TX_PWR_LEGACY_LIMIT_DATA),
			VIR_MEM_TYPE);

	if (!pTxPwrLegacyLimitData) {
		DBGLOG(RLM, ERROR,
			"Alloc buffer for TxPwrLimit main struct failed\n");
		return NULL;
	}

	pTxPwrLegacyLimitData->ucChNum =
		TX_PWR_LIMIT_2G_CH_NUM + TX_PWR_LIMIT_5G_CH_NUM;

	pTxPwrLegacyLimitData->rChannelTxLegacyPwrLimit =
		(struct CHANNEL_TX_LEGACY_PWR_LIMIT *)
		kalMemAlloc(sizeof(struct CHANNEL_TX_LEGACY_PWR_LIMIT) *
			(pTxPwrLegacyLimitData->ucChNum), VIR_MEM_TYPE);

	if (!pTxPwrLegacyLimitData->rChannelTxLegacyPwrLimit) {
		DBGLOG(RLM, ERROR,
			"Alloc buffer for TxPwrLimit ch values failed\n");

		kalMemFree(pTxPwrLegacyLimitData, VIR_MEM_TYPE,
			sizeof(struct TX_PWR_LEGACY_LIMIT_DATA));
		return NULL;
	}

	for (ch_idx = 0; ch_idx < pTxPwrLegacyLimitData->ucChNum; ch_idx++) {
		prChTxLegPwrLimit =
			&(pTxPwrLegacyLimitData->
			    rChannelTxLegacyPwrLimit[ch_idx]);

		kalMemSet(prChTxLegPwrLimit->rTxLegacyPwrLimitValue,
			MAX_TX_POWER,
			sizeof(prChTxLegPwrLimit->rTxLegacyPwrLimitValue));
	}

	ch_cnt = 0;
	for (band_idx = 0; band_idx < KAL_NUM_BANDS; band_idx++) {
		if (band_idx != KAL_BAND_2GHZ && band_idx != KAL_BAND_5GHZ)
			continue;

		prChannelList = (band_idx == KAL_BAND_2GHZ) ?
			gTx_Pwr_Limit_2g_Ch : gTx_Pwr_Limit_5g_Ch;

		ch_num =
			(band_idx == KAL_BAND_2GHZ) ?
				TX_PWR_LIMIT_2G_CH_NUM :
				TX_PWR_LIMIT_5G_CH_NUM;

		for (ch_idx = 0; ch_idx < ch_num; ch_idx++) {
			pTxPwrLegacyLimitData->
				rChannelTxLegacyPwrLimit[ch_cnt].ucChannel =
					prChannelList[ch_idx];

			++ch_cnt;
		}
	}

	return pTxPwrLegacyLimitData;
}

void
rlmDomainSendTxPwrLimitCmd(struct ADAPTER *prAdapter,
	uint8_t ucVersion,
	struct TX_PWR_LIMIT_DATA *pTxPwrLimitData)
{
	uint8_t ch_cnt = 0;
	uint8_t ch_idx = 0;
	uint8_t band_idx = 0;
	const int8_t *prChannelList = NULL;
	uint32_t rStatus;
	uint32_t u4SetQueryInfoLen;
	uint32_t u4SetCmdTableMaxSize[KAL_NUM_BANDS] = {0};
	struct CMD_SET_COUNTRY_CHANNEL_POWER_LIMIT_V2
		*prCmd[KAL_NUM_BANDS] = {0};
	struct CMD_CHANNEL_POWER_LIMIT_V2 *prCmdChPwrLimitV2 = NULL;
	uint32_t u4SetCountryCmdSize =
		sizeof(struct CMD_SET_COUNTRY_CHANNEL_POWER_LIMIT_V2);
	uint32_t u4ChPwrLimitV2Size = sizeof(struct CMD_CHANNEL_POWER_LIMIT_V2);
	const uint8_t ucCmdBatchSize =
		prAdapter->chip_info->ucTxPwrLimitBatchSize;

	for (band_idx = 0; band_idx < KAL_NUM_BANDS; band_idx++) {
		if (band_idx != KAL_BAND_2GHZ && band_idx != KAL_BAND_5GHZ)
			continue;

		prChannelList = (band_idx == KAL_BAND_2GHZ) ?
			gTx_Pwr_Limit_2g_Ch : gTx_Pwr_Limit_5g_Ch;

		ch_cnt = (band_idx == KAL_BAND_2GHZ) ? TX_PWR_LIMIT_2G_CH_NUM :
			TX_PWR_LIMIT_5G_CH_NUM;

		if (!ch_cnt)
			continue;

		u4SetCmdTableMaxSize[band_idx] = u4SetCountryCmdSize +
			ch_cnt * u4ChPwrLimitV2Size;

		prCmd[band_idx] = cnmMemAlloc(prAdapter,
			RAM_TYPE_BUF, u4SetCmdTableMaxSize[band_idx]);

		if (!prCmd[band_idx]) {
			DBGLOG(RLM, ERROR, "Domain: no buf to send cmd\n");
			goto error;
		}

		/*initialize tw pwr table*/
		kalMemSet(prCmd[band_idx], MAX_TX_POWER,
			u4SetCmdTableMaxSize[band_idx]);

		prCmd[band_idx]->ucNum = ch_cnt;
		prCmd[band_idx]->eband =
			(band_idx == KAL_BAND_2GHZ) ?
				BAND_2G4 : BAND_5G;
		prCmd[band_idx]->countryCode = rlmDomainGetCountryCode();

		DBGLOG(RLM, INFO,
			"%s, active n_channels=%d, band=%d\n",
			__func__, ch_cnt, prCmd[band_idx]->eband);

		for (ch_idx = 0; ch_idx < ch_cnt; ch_idx++) {
			prCmdChPwrLimitV2 =
				&(prCmd[band_idx]->rChannelPowerLimit[ch_idx]);
			prCmdChPwrLimitV2->ucCentralCh =
				prChannelList[ch_idx];
		}
	}

	rlmDomainTxPwrLimitSetValues(ucVersion,
		prCmd[KAL_BAND_2GHZ], pTxPwrLimitData);
	rlmDomainTxPwrLimitSetValues(ucVersion,
		prCmd[KAL_BAND_5GHZ], pTxPwrLimitData);

	for (band_idx = 0; band_idx < KAL_NUM_BANDS; band_idx++) {
		uint8_t ucRemainChNum, i, ucTempChNum, prCmdBatchNum;
		uint32_t u4BufSize = 0;
		struct CMD_SET_COUNTRY_CHANNEL_POWER_LIMIT_V2 *prTempCmd = NULL;
		enum ENUM_BAND eBand = (band_idx == KAL_BAND_2GHZ) ?
				BAND_2G4 : BAND_5G;
		uint16_t u2ChIdx = 0;

		if (!prCmd[band_idx])
			continue;

		ucRemainChNum = prCmd[band_idx]->ucNum;
		prCmdBatchNum = (ucRemainChNum +
			ucCmdBatchSize - 1) /
			ucCmdBatchSize;

		for (i = 0; i < prCmdBatchNum; i++) {
			if (i == prCmdBatchNum - 1)
				ucTempChNum = ucRemainChNum;
			else
				ucTempChNum = ucCmdBatchSize;

			u4BufSize = u4SetCountryCmdSize +
				ucTempChNum * u4ChPwrLimitV2Size;

			prTempCmd =
				cnmMemAlloc(prAdapter,
					RAM_TYPE_BUF, u4BufSize);

			if (!prTempCmd) {
				DBGLOG(RLM, ERROR,
					"Domain: no buf to send cmd\n");
				goto error;
			}

			/*copy partial tx pwr limit*/
			prTempCmd->ucNum = ucTempChNum;
			prTempCmd->eband = eBand;
			prTempCmd->countryCode = rlmDomainGetCountryCode();
			u2ChIdx = i * ucCmdBatchSize;
			kalMemCopy(&prTempCmd->rChannelPowerLimit[0],
				&prCmd[band_idx]->rChannelPowerLimit[u2ChIdx],
				ucTempChNum * u4ChPwrLimitV2Size);

			u4SetQueryInfoLen = u4BufSize;
			/* Update tx max. power info to chip */
			rStatus = wlanSendSetQueryCmd(prAdapter,
				CMD_ID_SET_COUNTRY_POWER_LIMIT,
				TRUE,
				FALSE,
				FALSE,
				NULL,
				NULL,
				u4SetQueryInfoLen,
				(uint8_t *) prTempCmd,
				NULL,
				0);

			cnmMemFree(prAdapter, prTempCmd);

			ucRemainChNum -= ucTempChNum;
		}
	}

error:
	for (band_idx = 0; band_idx < KAL_NUM_BANDS; band_idx++) {
		if (prCmd[band_idx])
			cnmMemFree(prAdapter, prCmd[band_idx]);
	}
}

#if (CFG_SUPPORT_WIFI_6G == 1)
void rlmDomainTxPwrLimitSendPerRateCmd_6G(
	struct ADAPTER *prAdapter,
	struct CMD_SET_TXPOWER_COUNTRY_TX_POWER_LIMIT_PER_RATE *prCmd
)
{
	uint32_t rStatus;
	uint32_t u4SetQueryInfoLen;
	uint32_t u4SetCountryTxPwrLimitCmdSize =
		sizeof(struct CMD_SET_TXPOWER_COUNTRY_TX_POWER_LIMIT_PER_RATE);
	uint32_t u4ChPwrLimitSize =
		sizeof(struct CMD_TXPOWER_CHANNEL_POWER_LIMIT_PER_RATE);
	const uint8_t ucCmdBatchSize =
		prAdapter->chip_info->ucTxPwrLimitBatchSize;

	uint8_t ucRemainChNum, i, ucTempChNum, prCmdBatchNum;
	uint32_t u4BufSize = 0;
	struct CMD_SET_TXPOWER_COUNTRY_TX_POWER_LIMIT_PER_RATE
		*prTempCmd = NULL;
	enum ENUM_BAND eBand = 0x3;
	uint16_t u2ChIdx = 0;
	u_int8_t bCmdFinished = FALSE;

	if (!prCmd) {
		DBGLOG(RLM, ERROR, "%s: prCmd is NULL\n", __func__);
		return;
	}

	ucRemainChNum = prCmd->ucNum;
	prCmdBatchNum = (ucRemainChNum +
			ucCmdBatchSize - 1) /
			ucCmdBatchSize;

	for (i = 0; i < prCmdBatchNum; i++) {
		if (i == prCmdBatchNum - 1) {
			ucTempChNum = ucRemainChNum;
			bCmdFinished = TRUE;
		} else {
			ucTempChNum = ucCmdBatchSize;
		}

		u4BufSize = u4SetCountryTxPwrLimitCmdSize +
			ucTempChNum * u4ChPwrLimitSize;

		prTempCmd =
			cnmMemAlloc(prAdapter,
				RAM_TYPE_BUF, u4BufSize);

		if (!prTempCmd) {
			DBGLOG(RLM, ERROR,
				"%s: no buf to send cmd\n", __func__);
			return;
		}

		/*copy partial tx pwr limit*/
		prTempCmd->ucNum = ucTempChNum;
		prTempCmd->eBand = eBand;
		prTempCmd->u4CountryCode =
			rlmDomainGetCountryCode();
		prTempCmd->eLimitType = TXPOWER_LIMIT_FORMAT_CHANNEL;
		prTempCmd->bCmdFinished = bCmdFinished;
		u2ChIdx = i * ucCmdBatchSize;
		kalMemCopy(
			&prTempCmd->u.rChannelPowerLimit[0],
			&prCmd->u.rChannelPowerLimit[u2ChIdx],
			ucTempChNum * u4ChPwrLimitSize);

		u4SetQueryInfoLen = u4BufSize;
		/* Update tx max. power info to chip */
		rStatus = wlanSendSetQueryCmd(prAdapter,
			CMD_ID_SET_COUNTRY_POWER_LIMIT_PER_RATE,
			TRUE,
			FALSE,
			FALSE,
			NULL,
			NULL,
			u4SetQueryInfoLen,
			(uint8_t *) prTempCmd,
			NULL,
			0);

		cnmMemFree(prAdapter, prTempCmd);

		ucRemainChNum -= ucTempChNum;
	}
}

void rlmDomainTxLegacyPwrLimitSendPerRateCmd_6G(
	struct ADAPTER *prAdapter,
	struct CMD_SET_TXPOWER_COUNTRY_TX_LEGACY_POWER_LIMIT_PER_RATE *prCmd
)
{
	uint32_t rStatus;
	uint32_t u4SetQueryInfoLen;
	uint32_t u4SetCountryTxLegacyPwrLimitCmdSize =
		sizeof(struct
		CMD_SET_TXPOWER_COUNTRY_TX_LEGACY_POWER_LIMIT_PER_RATE);
	uint32_t u4ChLegacyPwrLimitSize =
		sizeof(struct
		CMD_TXPOWER_CHANNEL_LEGACY_POWER_LIMIT_PER_RATE);
	const uint8_t ucCmdBatchSize =
		prAdapter->chip_info->ucTxPwrLimitBatchSize;

	uint8_t ucRemainChNum, i, ucTempChNum, prCmdBatchNum;
	uint32_t u4BufSize = 0;
	struct CMD_SET_TXPOWER_COUNTRY_TX_LEGACY_POWER_LIMIT_PER_RATE
		*prTempCmd = NULL;
	enum ENUM_BAND eBand = 0x3;
	uint16_t u2ChIdx = 0;
	u_int8_t bCmdFinished = FALSE;

	if (!prCmd) {
		DBGLOG(RLM, ERROR, "%s: prCmd is NULL\n", __func__);
		return;
	}

	ucRemainChNum = prCmd->ucNum;
	prCmdBatchNum = (ucRemainChNum +
			ucCmdBatchSize - 1) /
			ucCmdBatchSize;

	for (i = 0; i < prCmdBatchNum; i++) {
		if (i == prCmdBatchNum - 1) {
			ucTempChNum = ucRemainChNum;
			bCmdFinished = TRUE;
		} else {
			ucTempChNum = ucCmdBatchSize;
		}

		u4BufSize = u4SetCountryTxLegacyPwrLimitCmdSize +
			ucTempChNum * u4ChLegacyPwrLimitSize;

		prTempCmd =
			cnmMemAlloc(prAdapter,
				RAM_TYPE_BUF, u4BufSize);

		if (!prTempCmd) {
			DBGLOG(RLM, ERROR,
				"%s: no buf to send cmd\n", __func__);
			return;
		}

		/*copy partial tx pwr limit*/
		prTempCmd->ucNum = ucTempChNum;
		prTempCmd->eBand = eBand;
		prTempCmd->u4CountryCode =
			rlmDomainGetCountryCode();
		prTempCmd->bCmdFinished = bCmdFinished;
		u2ChIdx = i * ucCmdBatchSize;
		kalMemCopy(
			&prTempCmd->rChannelLegacyPowerLimit[0],
			&prCmd->rChannelLegacyPowerLimit[u2ChIdx],
			ucTempChNum * u4ChLegacyPwrLimitSize);

		u4SetQueryInfoLen = u4BufSize;
		/* Update tx max. power info to chip */
		rStatus = wlanSendSetQueryCmd(prAdapter,
			CMD_ID_SET_COUNTRY_LEGACY_POWER_LIMIT_PER_RATE,
			TRUE,
			FALSE,
			FALSE,
			NULL,
			NULL,
			u4SetQueryInfoLen,
			(uint8_t *) prTempCmd,
			NULL,
			0);

		cnmMemFree(prAdapter, prTempCmd);

		ucRemainChNum -= ucTempChNum;
	}
}
#endif /* #if (CFG_SUPPORT_WIFI_6G == 1) */

u_int32_t rlmDomainInitTxPwrLimitPerRateCmd(
	struct ADAPTER *prAdapter,
	struct wiphy *prWiphy,
	struct CMD_SET_TXPOWER_COUNTRY_TX_POWER_LIMIT_PER_RATE *prCmd[],
	uint32_t prCmdSize[])
{
	uint8_t ch_cnt = 0;
	uint8_t ch_idx = 0;
	uint8_t band_idx = 0;
	const int8_t *prChannelList = NULL;
	uint32_t u4SetCmdTableMaxSize[KAL_NUM_BANDS] = {0};
	uint32_t u4SetCountryTxPwrLimitCmdSize =
		sizeof(struct CMD_SET_TXPOWER_COUNTRY_TX_POWER_LIMIT_PER_RATE);
	uint32_t u4ChPwrLimitSize =
		sizeof(struct CMD_TXPOWER_CHANNEL_POWER_LIMIT_PER_RATE);
	struct CMD_TXPOWER_CHANNEL_POWER_LIMIT_PER_RATE *prChPwrLimit = NULL;

	for (band_idx = 0; band_idx < KAL_NUM_BANDS; band_idx++) {
		if (band_idx != KAL_BAND_2GHZ && band_idx != KAL_BAND_5GHZ)
			continue;

		prChannelList = (band_idx == KAL_BAND_2GHZ) ?
			gTx_Pwr_Limit_2g_Ch : gTx_Pwr_Limit_5g_Ch;

		ch_cnt = (band_idx == KAL_BAND_2GHZ) ? TX_PWR_LIMIT_2G_CH_NUM :
			TX_PWR_LIMIT_5G_CH_NUM;

		if (!ch_cnt)
			continue;

		u4SetCmdTableMaxSize[band_idx] = u4SetCountryTxPwrLimitCmdSize +
			ch_cnt * u4ChPwrLimitSize;
		prCmdSize[band_idx] = u4SetCmdTableMaxSize[band_idx];

		prCmd[band_idx] = kalMemAlloc(u4SetCmdTableMaxSize[band_idx],
			VIR_MEM_TYPE);
		if (!prCmd[band_idx]) {
			DBGLOG(RLM, ERROR, "Domain: no buf to send cmd\n");
			return WLAN_STATUS_RESOURCES;
		}

		/*initialize tx pwr table*/
		kalMemSet(prCmd[band_idx]->u.rChannelPowerLimit, MAX_TX_POWER,
			ch_cnt * u4ChPwrLimitSize);

		prCmd[band_idx]->ucNum = ch_cnt;
		prCmd[band_idx]->eBand =
			(band_idx == KAL_BAND_2GHZ) ?
				BAND_2G4 : BAND_5G;
		prCmd[band_idx]->eLimitType = TXPOWER_LIMIT_FORMAT_CHANNEL;
		prCmd[band_idx]->u4CountryCode = rlmDomainGetCountryCode();

		DBGLOG(RLM, INFO,
			"%s, active n_channels=%d, band=%d\n",
			__func__, ch_cnt, prCmd[band_idx]->eBand);

		for (ch_idx = 0; ch_idx < ch_cnt; ch_idx++) {
			prChPwrLimit =
				&(prCmd[band_idx]->
				  u.rChannelPowerLimit[ch_idx]);
			prChPwrLimit->u1CentralCh =	prChannelList[ch_idx];
		}

	}

	return WLAN_STATUS_SUCCESS;
}

u_int32_t rlmDomainInitTxLegacyPwrLimitPerRateCmd(
	struct ADAPTER *prAdapter,
	struct wiphy *prWiphy,
	struct CMD_SET_TXPOWER_COUNTRY_TX_LEGACY_POWER_LIMIT_PER_RATE *prCmd[],
	uint32_t prCmdSize[])
{
	uint8_t ch_cnt = 0;
	uint8_t ch_idx = 0;
	uint8_t band_idx = 0;
	const int8_t *prChannelList = NULL;
	uint32_t u4SetCmdTableMaxSize[KAL_NUM_BANDS] = {0};
	uint32_t u4SetCountryTxPwrLimitCmdSize =
		sizeof(struct
		    CMD_SET_TXPOWER_COUNTRY_TX_LEGACY_POWER_LIMIT_PER_RATE);
	uint32_t u4ChPwrLimitSize =
		sizeof(struct CMD_TXPOWER_CHANNEL_LEGACY_POWER_LIMIT_PER_RATE);
	struct CMD_TXPOWER_CHANNEL_LEGACY_POWER_LIMIT_PER_RATE
		*prChPwrLimit = NULL;

	for (band_idx = 0; band_idx < KAL_NUM_BANDS; band_idx++) {
		if (band_idx != KAL_BAND_2GHZ && band_idx != KAL_BAND_5GHZ)
			continue;

		prChannelList = (band_idx == KAL_BAND_2GHZ) ?
			gTx_Pwr_Limit_2g_Ch : gTx_Pwr_Limit_5g_Ch;

		ch_cnt = (band_idx == KAL_BAND_2GHZ) ? TX_PWR_LIMIT_2G_CH_NUM :
			TX_PWR_LIMIT_5G_CH_NUM;

		if (!ch_cnt)
			continue;

		u4SetCmdTableMaxSize[band_idx] = u4SetCountryTxPwrLimitCmdSize +
			ch_cnt * u4ChPwrLimitSize;
		prCmdSize[band_idx] = u4SetCmdTableMaxSize[band_idx];

		prCmd[band_idx] = kalMemAlloc(u4SetCmdTableMaxSize[band_idx],
			VIR_MEM_TYPE);
		if (!prCmd[band_idx]) {
			DBGLOG(RLM, ERROR, "Domain: no buf to send cmd\n");
			return WLAN_STATUS_RESOURCES;
		}

		/*initialize tx pwr table*/
		kalMemSet(prCmd[band_idx]->
			rChannelLegacyPowerLimit, MAX_TX_POWER,
			ch_cnt * u4ChPwrLimitSize);

		prCmd[band_idx]->ucNum = ch_cnt;
		prCmd[band_idx]->eBand =
			(band_idx == KAL_BAND_2GHZ) ?
				BAND_2G4 : BAND_5G;
		prCmd[band_idx]->u4CountryCode = rlmDomainGetCountryCode();

		DBGLOG(RLM, INFO,
			"%s, active n_channels=%d, band=%d\n",
			__func__, ch_cnt, prCmd[band_idx]->eBand);

		for (ch_idx = 0; ch_idx < ch_cnt; ch_idx++) {
			prChPwrLimit =
				&(prCmd[band_idx]->
				  rChannelLegacyPowerLimit[ch_idx]);
			prChPwrLimit->u1CentralCh =	prChannelList[ch_idx];
		}

	}

	return WLAN_STATUS_SUCCESS;
}

void rlmDomainTxPwrLimitSendPerRateCmd(
	struct ADAPTER *prAdapter,
	struct CMD_SET_TXPOWER_COUNTRY_TX_POWER_LIMIT_PER_RATE *prCmd[]
)
{
	uint32_t rStatus;
	uint32_t u4SetQueryInfoLen;
	uint8_t band_idx = 0;
	uint32_t u4SetCountryTxPwrLimitCmdSize =
		sizeof(struct CMD_SET_TXPOWER_COUNTRY_TX_POWER_LIMIT_PER_RATE);
	uint32_t u4ChPwrLimitSize =
		sizeof(struct CMD_TXPOWER_CHANNEL_POWER_LIMIT_PER_RATE);
	const uint8_t ucCmdBatchSize =
		prAdapter->chip_info->ucTxPwrLimitBatchSize;

	for (band_idx = 0; band_idx < KAL_NUM_BANDS; band_idx++) {
		uint8_t ucRemainChNum, i, ucTempChNum, prCmdBatchNum;
		uint32_t u4BufSize = 0;
		struct CMD_SET_TXPOWER_COUNTRY_TX_POWER_LIMIT_PER_RATE
			*prTempCmd = NULL;
		enum ENUM_BAND eBand = (band_idx == KAL_BAND_2GHZ) ?
				BAND_2G4 : BAND_5G;
		uint16_t u2ChIdx = 0;
		u_int8_t bCmdFinished = FALSE;

		if (!prCmd[band_idx])
			continue;

		ucRemainChNum = prCmd[band_idx]->ucNum;
		prCmdBatchNum = (ucRemainChNum +
			ucCmdBatchSize - 1) /
			ucCmdBatchSize;

		for (i = 0; i < prCmdBatchNum; i++) {
			if (i == prCmdBatchNum - 1) {
				ucTempChNum = ucRemainChNum;
				bCmdFinished = TRUE;
			} else {
				ucTempChNum = ucCmdBatchSize;
			}

			u4BufSize = u4SetCountryTxPwrLimitCmdSize +
				ucTempChNum * u4ChPwrLimitSize;

			prTempCmd =
				cnmMemAlloc(prAdapter,
					RAM_TYPE_BUF, u4BufSize);

			if (!prTempCmd) {
				DBGLOG(RLM, ERROR,
					"Domain: no buf to send cmd\n");
				return;
			}

			prTempCmd->aucPadding0[0] &=
				(~BIT(LEGACY_SINGLE_SKU_OFFSET));
			/*copy partial tx pwr limit*/
			prTempCmd->ucNum = ucTempChNum;
			prTempCmd->eBand = eBand;
			prTempCmd->u4CountryCode =
				rlmDomainGetCountryCode();
			prTempCmd->eLimitType = TXPOWER_LIMIT_FORMAT_CHANNEL;
			prTempCmd->bCmdFinished = bCmdFinished;
			u2ChIdx = i * ucCmdBatchSize;
			kalMemCopy(
				&prTempCmd->u.rChannelPowerLimit[0],
				&prCmd[band_idx]->
				 u.rChannelPowerLimit[u2ChIdx],
				ucTempChNum * u4ChPwrLimitSize);

			u4SetQueryInfoLen = u4BufSize;
			/* Update tx max. power info to chip */
			rStatus = wlanSendSetQueryCmd(prAdapter,
				CMD_ID_SET_COUNTRY_POWER_LIMIT_PER_RATE,
				TRUE,
				FALSE,
				FALSE,
				NULL,
				NULL,
				u4SetQueryInfoLen,
				(uint8_t *) prTempCmd,
				NULL,
				0);

			cnmMemFree(prAdapter, prTempCmd);

			ucRemainChNum -= ucTempChNum;
		}
	}
}

void rlmDomainTxLegacyPwrLimitSendPerRateCmd(
	struct ADAPTER *prAdapter,
	struct CMD_SET_TXPOWER_COUNTRY_TX_LEGACY_POWER_LIMIT_PER_RATE *prCmd[]
)
{
	uint32_t rStatus;
	uint32_t u4SetQueryInfoLen;
	uint8_t band_idx = 0;
	uint32_t u4SetCountryTxPwrLimitCmdSize =
		sizeof(struct
			CMD_SET_TXPOWER_COUNTRY_TX_LEGACY_POWER_LIMIT_PER_RATE);
	uint32_t u4ChPwrLimitSize =
		sizeof(struct CMD_TXPOWER_CHANNEL_LEGACY_POWER_LIMIT_PER_RATE);
	const uint8_t ucCmdBatchSize =
		prAdapter->chip_info->ucTxPwrLimitBatchSize;

	for (band_idx = 0; band_idx < KAL_NUM_BANDS; band_idx++) {
		uint8_t ucRemainChNum, i, ucTempChNum, prCmdBatchNum;
		uint32_t u4BufSize = 0;
		struct CMD_SET_TXPOWER_COUNTRY_TX_LEGACY_POWER_LIMIT_PER_RATE
			*prTempCmd = NULL;
		enum ENUM_BAND eBand = (band_idx == KAL_BAND_2GHZ) ?
				BAND_2G4 : BAND_5G;
		uint16_t u2ChIdx = 0;
		u_int8_t bCmdFinished = FALSE;

		if (!prCmd[band_idx])
			continue;

		ucRemainChNum = prCmd[band_idx]->ucNum;
		prCmdBatchNum = (ucRemainChNum +
			ucCmdBatchSize - 1) /
			ucCmdBatchSize;

		for (i = 0; i < prCmdBatchNum; i++) {
			if (i == prCmdBatchNum - 1) {
				ucTempChNum = ucRemainChNum;
				bCmdFinished = TRUE;
			} else {
				ucTempChNum = ucCmdBatchSize;
			}

			u4BufSize = u4SetCountryTxPwrLimitCmdSize +
				ucTempChNum * u4ChPwrLimitSize;

			prTempCmd =
				cnmMemAlloc(prAdapter,
					RAM_TYPE_BUF, u4BufSize);

			if (!prTempCmd) {
				DBGLOG(RLM, ERROR,
					"Domain: no buf to send cmd\n");
				return;
			}

			prTempCmd->aucPadding0[0] |=
				BIT(LEGACY_SINGLE_SKU_OFFSET);
			/*copy partial tx pwr limit*/
			prTempCmd->ucNum = ucTempChNum;
			prTempCmd->eBand = eBand;
			prTempCmd->u4CountryCode =
				rlmDomainGetCountryCode();
			prTempCmd->bCmdFinished = bCmdFinished;
			u2ChIdx = i * ucCmdBatchSize;
			kalMemCopy(
				&prTempCmd->rChannelLegacyPowerLimit[0],
				&prCmd[band_idx]->
				 rChannelLegacyPowerLimit[u2ChIdx],
				ucTempChNum * u4ChPwrLimitSize);

			u4SetQueryInfoLen = u4BufSize;
			/* Update tx max. power info to chip */
			rStatus = wlanSendSetQueryCmd(prAdapter,
				CMD_ID_SET_COUNTRY_LEGACY_POWER_LIMIT_PER_RATE,
				TRUE,
				FALSE,
				FALSE,
				NULL,
				NULL,
				u4SetQueryInfoLen,
				(uint8_t *) prTempCmd,
				NULL,
				0);

			cnmMemFree(prAdapter, prTempCmd);

			ucRemainChNum -= ucTempChNum;
		}
	}
}

u_int32_t rlmDomainInitTxBfBackoffCmd(
	struct ADAPTER *prAdapter,
	struct wiphy *prWiphy,
	struct CMD_TXPWR_TXBF_SET_BACKOFF **prCmd
)
{
	uint8_t ucChNum = TX_PWR_LIMIT_2G_CH_NUM + TX_PWR_LIMIT_5G_CH_NUM;
	uint8_t ucChIdx = 0;
	uint8_t ucChCnt = 0;
	uint8_t ucBandIdx = 0;
	uint8_t ucAisIdx = 0;
	uint8_t ucCnt = 0;
	const int8_t *prChannelList = NULL;
	uint32_t u4SetCmdSize = sizeof(struct CMD_TXPWR_TXBF_SET_BACKOFF);
	struct CMD_TXPWR_TXBF_CHANNEL_BACKOFF *prChTxBfBackoff = NULL;

	if (ucChNum >= CMD_POWER_LIMIT_TABLE_SUPPORT_CHANNEL_NUM) {
		DBGLOG(RLM, ERROR, "ChNum %d should <= %d\n",
			ucChNum, CMD_POWER_LIMIT_TABLE_SUPPORT_CHANNEL_NUM);
		return WLAN_STATUS_FAILURE;
	}

	*prCmd = cnmMemAlloc(prAdapter,
		RAM_TYPE_BUF, u4SetCmdSize);

	if (!*prCmd) {
		DBGLOG(RLM, ERROR, "Domain: no buf to send cmd\n");
		return WLAN_STATUS_RESOURCES;
	}

	/*initialize backoff table*/
	kalMemSet((*prCmd)->rChannelTxBfBackoff, MAX_TX_POWER,
		sizeof((*prCmd)->rChannelTxBfBackoff));

	(*prCmd)->ucNum = ucChNum;
	(*prCmd)->ucBssIdx = prAdapter->prAisBssInfo[ucAisIdx]->ucBssIndex;

	for (ucBandIdx = 0; ucBandIdx < KAL_NUM_BANDS; ucBandIdx++) {
		if (ucBandIdx != KAL_BAND_2GHZ && ucBandIdx != KAL_BAND_5GHZ)
			continue;

		prChannelList = (ucBandIdx == KAL_BAND_2GHZ) ?
			gTx_Pwr_Limit_2g_Ch : gTx_Pwr_Limit_5g_Ch;
		ucChCnt = (ucBandIdx == KAL_BAND_2GHZ) ?
			TX_PWR_LIMIT_2G_CH_NUM : TX_PWR_LIMIT_5G_CH_NUM;

		for (ucChIdx = 0; ucChIdx < ucChCnt; ucChIdx++) {
			prChTxBfBackoff =
				&((*prCmd)->rChannelTxBfBackoff[ucCnt++]);
			prChTxBfBackoff->ucCentralCh =	prChannelList[ucChIdx];
		}
	}

	return WLAN_STATUS_SUCCESS;
}

void rlmDomainTxPwrTxBfBackoffSetValues(
	uint8_t ucVersion,
	struct CMD_TXPWR_TXBF_SET_BACKOFF *prTxBfBackoffCmd,
	struct TX_PWR_LIMIT_DATA *pTxPwrLimitData)
{
	uint8_t ucIdx = 0;
	int8_t cChIdx = 0;
	struct CMD_TXPWR_TXBF_CHANNEL_BACKOFF *pChTxBfBackoff = NULL;
	struct CHANNEL_TX_PWR_LIMIT *pChTxPwrLimit = NULL;

	if (prTxBfBackoffCmd == NULL ||
		pTxPwrLimitData == NULL)
		return;

	for (ucIdx = 0; ucIdx < prTxBfBackoffCmd->ucNum; ucIdx++) {
		pChTxBfBackoff =
			&(prTxBfBackoffCmd->rChannelTxBfBackoff[ucIdx]);
		cChIdx = rlmDomainTxPwrLimitGetChIdx(pTxPwrLimitData,
			pChTxBfBackoff->ucCentralCh);

		if (cChIdx == -1) {
			DBGLOG(RLM, ERROR,
				"Invalid ch idx found while assigning values\n");
			return;
		}
		pChTxPwrLimit = &pTxPwrLimitData->rChannelTxPwrLimit[cChIdx];

		kalMemCopy(&pChTxBfBackoff->acTxBfBackoff,
			pChTxPwrLimit->rTxBfBackoff,
			sizeof(pChTxBfBackoff->acTxBfBackoff));
	}

	for (ucIdx = 0; ucIdx < prTxBfBackoffCmd->ucNum; ucIdx++) {
		pChTxBfBackoff =
			&(prTxBfBackoffCmd->rChannelTxBfBackoff[ucIdx]);

		DBGLOG(RLM, ERROR,
			"ch %d TxBf backoff 2to1 %d\n",
			pChTxBfBackoff->ucCentralCh,
			pChTxBfBackoff->acTxBfBackoff[0]);

	}

}

void rlmDomainTxPwrSendTxBfBackoffCmd(
	struct ADAPTER *prAdapter,
	struct CMD_TXPWR_TXBF_SET_BACKOFF *prCmd)
{
	uint32_t rStatus;
	uint32_t u4SetQueryInfoLen;
	uint32_t u4SetCmdSize = sizeof(struct CMD_TXPWR_TXBF_SET_BACKOFF);

	u4SetQueryInfoLen = u4SetCmdSize;
	/* Update tx max. power info to chip */
	rStatus = wlanSendSetQueryCmd(prAdapter,
		CMD_ID_SET_TXBF_BACKOFF,
		TRUE,
		FALSE,
		FALSE,
		NULL,
		NULL,
		u4SetQueryInfoLen,
		(uint8_t *) prCmd,
		NULL,
		0);

}

#if (CFG_SUPPORT_WIFI_6G == 1)
void
rlmDomainSendTxPwrLimitPerRateCmd_6G(struct ADAPTER *prAdapter,
	uint8_t ucVersion,
	struct TX_PWR_LIMIT_DATA *pTxPwrLimitData)
{
	struct wiphy *wiphy;
	struct CMD_SET_TXPOWER_COUNTRY_TX_POWER_LIMIT_PER_RATE
		*prTxPwrLimitPerRateCmd_6G;
	uint32_t rTxPwrLimitPerRateCmdSize_6G = 0;

	uint32_t u4SetCmdTableMaxSize = 0;
	uint32_t u4SetCountryTxPwrLimitCmdSize =
		sizeof(struct CMD_SET_TXPOWER_COUNTRY_TX_POWER_LIMIT_PER_RATE);
	uint32_t u4ChPwrLimitSize =
		sizeof(struct CMD_TXPOWER_CHANNEL_POWER_LIMIT_PER_RATE);
	uint8_t ch_cnt = TX_PWR_LIMIT_6G_CH_NUM;
	uint8_t ch_idx = 0;
	const int8_t *prChannelList = &gTx_Pwr_Limit_6g_Ch[0];

	wiphy = priv_to_wiphy(prAdapter->prGlueInfo);

	u4SetCmdTableMaxSize = u4SetCountryTxPwrLimitCmdSize +
	ch_cnt * u4ChPwrLimitSize;

	rTxPwrLimitPerRateCmdSize_6G = u4SetCmdTableMaxSize;
	prTxPwrLimitPerRateCmd_6G =
	kalMemAlloc(u4SetCmdTableMaxSize, VIR_MEM_TYPE);

	if (!prTxPwrLimitPerRateCmd_6G) {
		DBGLOG(RLM, ERROR,
		"%s no buf to send cmd\n", __func__);
		return;
	}

	/*initialize tx pwr table*/
	kalMemSet(prTxPwrLimitPerRateCmd_6G->u.rChannelPowerLimit, MAX_TX_POWER,
		ch_cnt * u4ChPwrLimitSize);

	prTxPwrLimitPerRateCmd_6G->ucNum = ch_cnt;
	prTxPwrLimitPerRateCmd_6G->eBand = 0x3;
	prTxPwrLimitPerRateCmd_6G->u4CountryCode = rlmDomainGetCountryCode();

	for (ch_idx = 0; ch_idx < ch_cnt; ch_idx++) {
	prTxPwrLimitPerRateCmd_6G->u.rChannelPowerLimit[ch_idx].u1CentralCh =
	prChannelList[ch_idx];
	}

	rlmDomainTxPwrLimitPerRateSetValues(ucVersion,
		prTxPwrLimitPerRateCmd_6G,
		pTxPwrLimitData);

	rlmDomainTxPwrLimitSendPerRateCmd_6G(prAdapter,
		prTxPwrLimitPerRateCmd_6G);
	kalMemFree(prTxPwrLimitPerRateCmd_6G, VIR_MEM_TYPE,
		u4SetCmdTableMaxSize);
}

void
rlmDomainSendTxLegacyPwrLimitPerRateCmd_6G(struct ADAPTER *prAdapter,
	uint8_t ucVersion,
	struct TX_PWR_LEGACY_LIMIT_DATA *pTxLegacyPwrLimitData)
{
	struct wiphy *wiphy;
	struct CMD_SET_TXPOWER_COUNTRY_TX_LEGACY_POWER_LIMIT_PER_RATE
		*prTxLegacyPwrLimitPerRateCmd_6G;
	uint32_t rTxLegacyPwrLimitPerRateCmdSize_6G = 0;

	uint32_t u4SetCmdTableMaxSize = 0;
	uint32_t u4SetCountryTxLegacyPwrLimitCmdSize =
		sizeof(struct
		CMD_SET_TXPOWER_COUNTRY_TX_LEGACY_POWER_LIMIT_PER_RATE);
	uint32_t u4ChLegacyPwrLimitSize =
		sizeof(struct
		CMD_TXPOWER_CHANNEL_LEGACY_POWER_LIMIT_PER_RATE);
	uint8_t ch_cnt = TX_PWR_LIMIT_6G_CH_NUM;
	uint8_t ch_idx = 0;
	const int8_t *prChannelList = &gTx_Pwr_Limit_6g_Ch[0];

	wiphy = priv_to_wiphy(prAdapter->prGlueInfo);

	u4SetCmdTableMaxSize = u4SetCountryTxLegacyPwrLimitCmdSize +
	ch_cnt * u4ChLegacyPwrLimitSize;

	rTxLegacyPwrLimitPerRateCmdSize_6G = u4SetCmdTableMaxSize;
	prTxLegacyPwrLimitPerRateCmd_6G =
	kalMemAlloc(u4SetCmdTableMaxSize, VIR_MEM_TYPE);

	if (!prTxLegacyPwrLimitPerRateCmd_6G) {
		DBGLOG(RLM, ERROR,
		"%s no buf to send cmd\n", __func__);
		return;
	}

	/*initialize tx pwr table*/
	kalMemSet(prTxLegacyPwrLimitPerRateCmd_6G->
		rChannelLegacyPowerLimit, MAX_TX_POWER,
		ch_cnt * u4ChLegacyPwrLimitSize);

	prTxLegacyPwrLimitPerRateCmd_6G->ucNum = ch_cnt;
	prTxLegacyPwrLimitPerRateCmd_6G->eBand = 0x3;
	prTxLegacyPwrLimitPerRateCmd_6G->
		u4CountryCode = rlmDomainGetCountryCode();

	for (ch_idx = 0; ch_idx < ch_cnt; ch_idx++) {
	prTxLegacyPwrLimitPerRateCmd_6G->
		rChannelLegacyPowerLimit[ch_idx].u1CentralCh =
		prChannelList[ch_idx];
	}

	rlmDomainTxLegacyPwrLimitPerRateSetValues(ucVersion,
		prTxLegacyPwrLimitPerRateCmd_6G,
		pTxLegacyPwrLimitData);

	rlmDomainTxLegacyPwrLimitSendPerRateCmd_6G(prAdapter,
		prTxLegacyPwrLimitPerRateCmd_6G);
	kalMemFree(prTxLegacyPwrLimitPerRateCmd_6G, VIR_MEM_TYPE,
		u4SetCmdTableMaxSize);
}
#endif /* #if (CFG_SUPPORT_WIFI_6G == 1) */

void
rlmDomainSendTxPwrLimitPerRateCmd(struct ADAPTER *prAdapter,
	uint8_t ucVersion,
	struct TX_PWR_LIMIT_DATA *pTxPwrLimitData)
{
	struct wiphy *wiphy;
	uint8_t band_idx = 0;
	struct CMD_SET_TXPOWER_COUNTRY_TX_POWER_LIMIT_PER_RATE
		*prTxPwrLimitPerRateCmd[KAL_NUM_BANDS] = {0};
	uint32_t prTxPwrLimitPerRateCmdSize[KAL_NUM_BANDS] = {0};

	wiphy = priv_to_wiphy(prAdapter->prGlueInfo);

	if (rlmDomainInitTxPwrLimitPerRateCmd(
		prAdapter, wiphy, prTxPwrLimitPerRateCmd,
		prTxPwrLimitPerRateCmdSize) !=
		WLAN_STATUS_SUCCESS)
		goto error;

	rlmDomainTxPwrLimitPerRateSetValues(ucVersion,
		prTxPwrLimitPerRateCmd[KAL_BAND_2GHZ], pTxPwrLimitData);
	rlmDomainTxPwrLimitPerRateSetValues(ucVersion,
		prTxPwrLimitPerRateCmd[KAL_BAND_5GHZ], pTxPwrLimitData);
	rlmDomainTxPwrLimitSendPerRateCmd(prAdapter, prTxPwrLimitPerRateCmd);

error:
	for (band_idx = 0; band_idx < KAL_NUM_BANDS; band_idx++)
		if (prTxPwrLimitPerRateCmd[band_idx])
			kalMemFree(prTxPwrLimitPerRateCmd[band_idx],
				VIR_MEM_TYPE,
				prTxPwrLimitPerRateCmdSize[band_idx]);
}

void
rlmDomainSendTxLegacyPwrLimitPerRateCmd(struct ADAPTER *prAdapter,
	uint8_t ucVersion,
	struct TX_PWR_LEGACY_LIMIT_DATA *pTxPwrLegacyLimitData)
{
	struct wiphy *wiphy;
	uint8_t band_idx = 0;
	struct CMD_SET_TXPOWER_COUNTRY_TX_LEGACY_POWER_LIMIT_PER_RATE
		*prTxLegacyPwrLimitPerRateCmd[KAL_NUM_BANDS] = {0};
	uint32_t prTxLegacyPwrLimitPerRateCmdSize[KAL_NUM_BANDS] = {0};

	wiphy = priv_to_wiphy(prAdapter->prGlueInfo);

	if (rlmDomainInitTxLegacyPwrLimitPerRateCmd(
		prAdapter, wiphy, prTxLegacyPwrLimitPerRateCmd,
		prTxLegacyPwrLimitPerRateCmdSize) !=
		WLAN_STATUS_SUCCESS)
		goto error;

	rlmDomainTxLegacyPwrLimitPerRateSetValues(ucVersion,
		prTxLegacyPwrLimitPerRateCmd[KAL_BAND_2GHZ],
			pTxPwrLegacyLimitData);
	rlmDomainTxLegacyPwrLimitPerRateSetValues(ucVersion,
		prTxLegacyPwrLimitPerRateCmd[KAL_BAND_5GHZ],
			pTxPwrLegacyLimitData);
	rlmDomainTxLegacyPwrLimitSendPerRateCmd(prAdapter,
		prTxLegacyPwrLimitPerRateCmd);

error:
	for (band_idx = 0; band_idx < KAL_NUM_BANDS; band_idx++)
		if (prTxLegacyPwrLimitPerRateCmd[band_idx])
			kalMemFree(prTxLegacyPwrLimitPerRateCmd[band_idx],
				VIR_MEM_TYPE,
				prTxLegacyPwrLimitPerRateCmdSize[band_idx]);
}

void
rlmDomainSendTxBfBackoffCmd(struct ADAPTER *prAdapter,
	uint8_t ucVersion,
	struct TX_PWR_LIMIT_DATA *pTxPwrLimitData)
{
	struct wiphy *wiphy;
	struct CMD_TXPWR_TXBF_SET_BACKOFF
		*prTxBfBackoffCmd = NULL;

	wiphy = priv_to_wiphy(prAdapter->prGlueInfo);

	if (rlmDomainInitTxBfBackoffCmd(
		prAdapter, wiphy, &prTxBfBackoffCmd) !=
		WLAN_STATUS_SUCCESS)
		goto error;

	rlmDomainTxPwrTxBfBackoffSetValues(
		ucVersion, prTxBfBackoffCmd, pTxPwrLimitData);
	rlmDomainTxPwrSendTxBfBackoffCmd(prAdapter, prTxBfBackoffCmd);

error:

	if (prTxBfBackoffCmd)
		cnmMemFree(prAdapter, prTxBfBackoffCmd);
}

#if CFG_SUPPORT_DYNAMIC_PWR_LIMIT_ANT_TAG
void rlmDomainSendAntPwrLimitCmd(struct ADAPTER *prAdapter,
	u_int8_t fgNeedUpdateSkuTable)
{
	uint32_t rStatus;
	struct CMD_SET_TXPOWER_COUNTRY_TX_POWER_LIMIT_PER_RATE
		*prAntLimitCmd = NULL;
	uint32_t u4SetAntPwrLimitCmdSize =
		sizeof(struct CMD_SET_TXPOWER_COUNTRY_TX_POWER_LIMIT_PER_RATE);
	uint32_t u4AntPwrLimitSize =
		sizeof(struct CMD_ANT_TABLE_TYPE);
	uint32_t u4BandNum = (POWER_ANT_BAND_NUM < MIN_ANT_BAND_NUM) ?
				MIN_ANT_BAND_NUM : POWER_ANT_BAND_NUM;
	uint32_t u4BufSize = u4SetAntPwrLimitCmdSize +
		(POWER_ANT_NUM * u4BandNum) * u4AntPwrLimitSize;

	prAntLimitCmd = cnmMemAlloc(prAdapter, RAM_TYPE_BUF, u4BufSize);
	if (!prAntLimitCmd) {
		DBGLOG(RLM, ERROR, "Domain: no buf to send cmd\n");
		return;
	}

	kalMemSet(prAntLimitCmd, 0, u4BufSize);
	prAntLimitCmd->ucNum = 0;
	prAntLimitCmd->eBand = BAND_NULL;
	prAntLimitCmd->bCmdFinished = fgNeedUpdateSkuTable ? FALSE : TRUE;
	prAntLimitCmd->eLimitType = TXPOWER_LIMIT_FORMAT_ANT;
	prAntLimitCmd->u4CountryCode = rlmDomainGetCountryCode();
	prAntLimitCmd->u.rAntPowerLimit.ucAntSetType = ANT_SET_TYPE_ANTSWAP;
	prAntLimitCmd->u.rAntPowerLimit.ucAntNum = POWER_ANT_NUM;
	prAntLimitCmd->u.rAntPowerLimit.ucEntryLength = u4BandNum;

	rlmDomainApplySarAntPwrLimit(prAdapter,
				     &prAntLimitCmd->u.rAntPowerLimit);

	rStatus = wlanSendSetQueryCmd(prAdapter,
				      CMD_ID_SET_COUNTRY_POWER_LIMIT_PER_RATE,
				      TRUE, FALSE, FALSE,
				      NULL, NULL,
				      u4BufSize, (uint8_t *)prAntLimitCmd,
				      NULL, 0);

	if (rStatus != WLAN_STATUS_PENDING)
		DBGLOG(RLM, ERROR, "wlanSendSetQueryCmd error\n");

	cnmMemFree(prAdapter, prAntLimitCmd);
}
#endif
#endif/*CFG_SUPPORT_SINGLE_SKU*/

void rlmDomainSendPwrLimitCmd_V2(struct ADAPTER *prAdapter)
{
#if (CFG_SUPPORT_SINGLE_SKU == 1)
	uint8_t ucVersion = 0;
	struct TX_PWR_LIMIT_DATA *pTxPwrLimitData = NULL;
	struct TX_PWR_LEGACY_LIMIT_DATA *pTxPwrLegacyLimitData = NULL;

	DBGLOG(RLM, INFO, "rlmDomainSendPwrLimitCmd()\n");
	pTxPwrLimitData = rlmDomainInitTxPwrLimitData(prAdapter);
	pTxPwrLegacyLimitData = rlmDomainInitTxPwrLegacyLimitData(prAdapter);

	if (!pTxPwrLimitData) {
		DBGLOG(RLM, ERROR,
			"Init TxPwrLimitData failed\n");
		goto error;
	}

	if (!pTxPwrLegacyLimitData) {
		DBGLOG(RLM, ERROR,
			"Init pTxPwrLegacyLimitData failed\n");
		goto error;
	}

#if CFG_SUPPORT_DYNAMIC_PWR_LIMIT_ANT_TAG
	/* Send SAR AntPwrLimit first */
	rlmDomainSendAntPwrLimitCmd(prAdapter, TRUE);
#endif

	/* Get Max Tx Power from MT_TxPwrLimit.dat */
	if (!rlmDomainGetTxPwrLimit(rlmDomainGetCountryCode(),
		&ucVersion,
		prAdapter->prGlueInfo,
		pTxPwrLimitData)) {
		DBGLOG(RLM, ERROR,
			"Load TxPwrLimitData failed\n");
		goto error;
	}

	if (!rlmDomainGetTxPwrLegacyLimit(rlmDomainGetCountryCode(),
		&ucVersion,
		prAdapter->prGlueInfo,
		pTxPwrLegacyLimitData)) {
		DBGLOG(RLM, ERROR,
			"Load pTxPwrLegacyLimitData failed\n");
		goto error;
	}

#if CFG_SUPPORT_DYNAMIC_PWR_LIMIT
	/* Apply Dynamic TxPwrLimit */
	if (!rlmDomainApplySarTxPwrLimit(ucVersion, FALSE,
		prAdapter, pTxPwrLimitData, pTxPwrLegacyLimitData)) {
		DBGLOG(RLM, INFO,
			"Apply SAR TxPwrLimit failed\n");
	}
#endif

	/* Prepare to send CMD to FW */
	if (ucVersion == 0) {
		rlmDomainSendTxPwrLimitCmd(prAdapter,
			ucVersion, pTxPwrLimitData);
	 } else if (ucVersion == 1 || ucVersion == 2) {

		rlmDomainSendTxLegacyPwrLimitPerRateCmd(prAdapter,
			ucVersion, pTxPwrLegacyLimitData);
		rlmDomainSendTxPwrLimitPerRateCmd(prAdapter,
			ucVersion, pTxPwrLimitData);

		if (g_bTxBfBackoffExists)
			rlmDomainSendTxBfBackoffCmd(prAdapter,
				ucVersion, pTxPwrLimitData);

	} else {
		DBGLOG(RLM, WARN, "Unsupported TxPwrLimit dat version %u\n",
			ucVersion);
	}

#if (CFG_SUPPORT_WIFI_6G == 1)
	if (pTxPwrLimitData && pTxPwrLimitData->rChannelTxPwrLimit)
		kalMemFree(pTxPwrLimitData->rChannelTxPwrLimit, VIR_MEM_TYPE,
			sizeof(struct CHANNEL_TX_PWR_LIMIT) *
			pTxPwrLimitData->ucChNum);

	if (pTxPwrLimitData)
		kalMemFree(pTxPwrLimitData, VIR_MEM_TYPE,
			sizeof(struct TX_PWR_LIMIT_DATA));

	if (pTxPwrLegacyLimitData && pTxPwrLegacyLimitData->
		rChannelTxLegacyPwrLimit)
		kalMemFree(pTxPwrLegacyLimitData->
			rChannelTxLegacyPwrLimit, VIR_MEM_TYPE,
			sizeof(struct CHANNEL_TX_LEGACY_PWR_LIMIT) *
			pTxPwrLegacyLimitData->ucChNum);

	if (pTxPwrLegacyLimitData)
		kalMemFree(pTxPwrLegacyLimitData, VIR_MEM_TYPE,
			sizeof(struct TX_PWR_LEGACY_LIMIT_DATA));

	pTxPwrLimitData = rlmDomainInitTxPwrLimitData_6G(prAdapter);
	pTxPwrLegacyLimitData = rlmDomainInitTxLegacyPwrLimitData_6G(prAdapter);

	if (!pTxPwrLimitData) {
		DBGLOG(RLM, ERROR,
			"Init TxPwrLimitData 6G failed\n");
		goto error;
	}

	if (!pTxPwrLegacyLimitData) {
		DBGLOG(RLM, ERROR,
			"Init TxPwrLegacyLimitData 6G failed\n");
		goto error;
	}

	/* Get Max Tx Power from MT_TxPwrLimit_6G.dat */
	prAdapter->chip_info->prTxPwrLimitFile = "TxPwrLimit6G_MT79x1.dat";
	if (!rlmDomainGetTxPwrLimit(rlmDomainGetCountryCode(),
		&ucVersion,
		prAdapter->prGlueInfo,
		pTxPwrLimitData)) {
		DBGLOG(RLM, ERROR,
			"Load TxPwrLimitData failed\n");
		goto error;
	}

	if (!rlmDomainGetTxPwrLegacyLimit(rlmDomainGetCountryCode(),
		&ucVersion,
		prAdapter->prGlueInfo,
		pTxPwrLegacyLimitData)) {
		DBGLOG(RLM, ERROR,
			"Load TxPwrLegacyLimitData failed\n");
		goto error;
	}

#if CFG_SUPPORT_DYNAMIC_PWR_LIMIT
	/* Apply Dynamic TxPwrLimit for 6G channel */
	if (!rlmDomainApplySarTxPwrLimit(ucVersion, TRUE,
		prAdapter, pTxPwrLimitData, pTxPwrLegacyLimitData)) {
		DBGLOG(RLM, INFO, "Apply SAR TxPwrLimit 6G failed\n");
	}
#endif

	/* Prepare to send CMD to FW */
	if (ucVersion == 2) {
		rlmDomainSendTxPwrLimitPerRateCmd_6G(prAdapter,
			ucVersion, pTxPwrLimitData);
		rlmDomainSendTxLegacyPwrLimitPerRateCmd_6G(prAdapter,
			ucVersion, pTxPwrLegacyLimitData);
	} else {
		DBGLOG(RLM, WARN,
		"Unsupported TxPwrLimit6G_MT7961.dat version %u\n",
		ucVersion);
	}

	/* restore back to default value */
	prAdapter->chip_info->prTxPwrLimitFile = "TxPwrLimit_MT79x1.dat";
#endif /* #if (CFG_SUPPORT_WIFI_6G == 1) */

error:
	if (pTxPwrLimitData && pTxPwrLimitData->rChannelTxPwrLimit)
		kalMemFree(pTxPwrLimitData->rChannelTxPwrLimit, VIR_MEM_TYPE,
			sizeof(struct CHANNEL_TX_PWR_LIMIT) *
			pTxPwrLimitData->ucChNum);

	if (pTxPwrLimitData)
		kalMemFree(pTxPwrLimitData, VIR_MEM_TYPE,
			sizeof(struct TX_PWR_LIMIT_DATA));

	if (pTxPwrLegacyLimitData && pTxPwrLegacyLimitData->
		rChannelTxLegacyPwrLimit)
		kalMemFree(pTxPwrLegacyLimitData->
			rChannelTxLegacyPwrLimit, VIR_MEM_TYPE,
			sizeof(struct CHANNEL_TX_LEGACY_PWR_LIMIT) *
			pTxPwrLegacyLimitData->ucChNum);

	if (pTxPwrLegacyLimitData)
		kalMemFree(pTxPwrLegacyLimitData, VIR_MEM_TYPE,
			sizeof(struct TX_PWR_LEGACY_LIMIT_DATA));

        /* restore back to default value */
        prAdapter->chip_info->prTxPwrLimitFile = "TxPwrLimit_MT79x1.dat";
#endif
}

#if CFG_SUPPORT_DYNAMIC_PWR_LIMIT
/* dynamic tx power control: begin ********************************************/
uint32_t txPwrParseNumber(char **pcContent, char *delim, uint8_t *op,
			  int8_t *value) {
	u_int8_t fgIsNegtive = FALSE;
	char *pcTmp = NULL;
	char *result = NULL;

	if ((!pcContent) || (*pcContent == NULL))
		return -1;

	if (**pcContent == '-') {
		fgIsNegtive = TRUE;
		*pcContent = *pcContent + 1;
	}
	pcTmp = *pcContent;

	result = kalStrSep(pcContent, delim);
	if (result == NULL) {
		return -1;
	} else if ((result != NULL) && (kalStrLen(result) == 0)) {
		if (fgIsNegtive)
			return -1;
		*value = 0;
		*op = 0;
	} else {
		if (kalkStrtou8(pcTmp, 0, value) != 0) {
			DBGLOG(RLM, ERROR,
			       "parse number error: invalid number [%s]\n",
			       pcTmp);
			return -1;
		}
		if (fgIsNegtive)
			*op = 2;
		else
			*op = 1;
	}

	return 0;
}

void txPwrOperate(enum ENUM_TX_POWER_CTRL_TYPE eCtrlType,
		  int8_t *operand1, int8_t *operand2)
{
	switch (eCtrlType) {
	case PWR_CTRL_TYPE_WIFION_POWER_LEVEL:
	case PWR_CTRL_TYPE_IOCTL_POWER_LEVEL:
		if (*operand1 > *operand2)
			*operand1 = *operand2;
		break;
	case PWR_CTRL_TYPE_WIFION_POWER_OFFSET:
	case PWR_CTRL_TYPE_IOCTL_POWER_OFFSET:
		*operand1 += *operand2;
		break;
	default:
		break;
	}

	if (*operand1 > MAX_TX_POWER)
		*operand1 = MAX_TX_POWER;
	else if (*operand1 < MIN_TX_POWER)
		*operand1 = MIN_TX_POWER;
}

uint32_t txPwrArbitrator(enum ENUM_TX_POWER_CTRL_TYPE eCtrlType,
			 struct CMD_CHANNEL_POWER_LIMIT *prCmdPwrLimit,
			 struct TX_PWR_CTRL_CHANNEL_SETTING *prChlSetting)
{
	if (prChlSetting->op[0] != PWR_CTRL_TYPE_NO_ACTION) {
		txPwrOperate(eCtrlType, &prCmdPwrLimit->cPwrLimitCCK,
			     &prChlSetting->i8PwrLimit[0]);
	}
	if (prChlSetting->op[1] != PWR_CTRL_TYPE_NO_ACTION) {
		txPwrOperate(eCtrlType, &prCmdPwrLimit->cPwrLimit20L,
			     &prChlSetting->i8PwrLimit[1]);
	}
	if (prChlSetting->op[2] != PWR_CTRL_TYPE_NO_ACTION) {
		txPwrOperate(eCtrlType, &prCmdPwrLimit->cPwrLimit20H,
			     &prChlSetting->i8PwrLimit[2]);
	}
	if (prChlSetting->op[3] != PWR_CTRL_TYPE_NO_ACTION) {
		txPwrOperate(eCtrlType, &prCmdPwrLimit->cPwrLimit40L,
			     &prChlSetting->i8PwrLimit[3]);
	}
	if (prChlSetting->op[4] != PWR_CTRL_TYPE_NO_ACTION) {
		txPwrOperate(eCtrlType, &prCmdPwrLimit->cPwrLimit40H,
			     &prChlSetting->i8PwrLimit[4]);
	}
	if (prChlSetting->op[5] != PWR_CTRL_TYPE_NO_ACTION) {
		txPwrOperate(eCtrlType, &prCmdPwrLimit->cPwrLimit80L,
			     &prChlSetting->i8PwrLimit[5]);
	}
	if (prChlSetting->op[6] != PWR_CTRL_TYPE_NO_ACTION) {
		txPwrOperate(eCtrlType, &prCmdPwrLimit->cPwrLimit80H,
			     &prChlSetting->i8PwrLimit[6]);
	}
	if (prChlSetting->op[7] != PWR_CTRL_TYPE_NO_ACTION) {
		txPwrOperate(eCtrlType, &prCmdPwrLimit->cPwrLimit160L,
			     &prChlSetting->i8PwrLimit[7]);
	}
	if (prChlSetting->op[8] != PWR_CTRL_TYPE_NO_ACTION) {
		txPwrOperate(eCtrlType, &prCmdPwrLimit->cPwrLimit160H,
			     &prChlSetting->i8PwrLimit[8]);
	}

	return 0;
}

uint32_t txPwrApplyOneSetting(struct CMD_SET_COUNTRY_CHANNEL_POWER_LIMIT *prCmd,
			      struct TX_PWR_CTRL_ELEMENT *prCurElement,
			      uint8_t *bandedgeParam)
{
	struct CMD_CHANNEL_POWER_LIMIT *prCmdPwrLimit;
	struct TX_PWR_CTRL_CHANNEL_SETTING *prChlSetting;
	uint8_t i, j, channel, channel2, channel3;
	u_int8_t fgDoArbitrator;

	prCmdPwrLimit = &prCmd->rChannelPowerLimit[0];
	for (i = 0; i < prCmd->ucNum; i++) {
		channel = prCmdPwrLimit->ucCentralCh;
		for (j = 0; j < prCurElement->settingCount; j++) {
			prChlSetting = &prCurElement->rChlSettingList[j];
			channel2 = prChlSetting->channelParam[0];
			channel3 = prChlSetting->channelParam[1];
			fgDoArbitrator = FALSE;
			switch (prChlSetting->eChnlType) {
				case PWR_CTRL_CHNL_TYPE_NORMAL: {
					if (channel == channel2)
						fgDoArbitrator = TRUE;
					break;
				}
				case PWR_CTRL_CHNL_TYPE_ALL: {
					fgDoArbitrator = TRUE;
					break;
				}
				case PWR_CTRL_CHNL_TYPE_RANGE: {
					if ((channel >= channel2) &&
					    (channel <= channel3))
						fgDoArbitrator = TRUE;
					break;
				}
				case PWR_CTRL_CHNL_TYPE_2G4: {
					if (channel <= 14)
						fgDoArbitrator = TRUE;
					break;
				}
				case PWR_CTRL_CHNL_TYPE_5G: {
					if (channel > 14)
						fgDoArbitrator = TRUE;
					break;
				}
				case PWR_CTRL_CHNL_TYPE_BANDEDGE_2G4: {
					if ((channel == *bandedgeParam) ||
					    (channel == *(bandedgeParam + 1)))
						fgDoArbitrator = TRUE;
					break;
				}
				case PWR_CTRL_CHNL_TYPE_BANDEDGE_5G: {
					if ((channel == *(bandedgeParam + 2)) ||
					    (channel == *(bandedgeParam + 3)))
						fgDoArbitrator = TRUE;
					break;
				}
				case PWR_CTRL_CHNL_TYPE_5G_BAND1: {
					if ((channel >= 30) && (channel <= 50))
						fgDoArbitrator = TRUE;
					break;
				}
				case PWR_CTRL_CHNL_TYPE_5G_BAND2: {
					if ((channel >= 51) && (channel <= 70))
						fgDoArbitrator = TRUE;
					break;
				}
				case PWR_CTRL_CHNL_TYPE_5G_BAND3: {
					if ((channel >= 71) && (channel <= 145))
						fgDoArbitrator = TRUE;
					break;
				}
				case PWR_CTRL_CHNL_TYPE_5G_BAND4: {
					if ((channel >= 146) &&
					    (channel <= 170))
						fgDoArbitrator = TRUE;
					break;
				}
				default:
					break;
			}
			if (fgDoArbitrator)
				txPwrArbitrator(prCurElement->eCtrlType,
						prCmdPwrLimit, prChlSetting);
		}

		prCmdPwrLimit++;
	}

	return 0;
}

uint32_t txPwrCtrlApplySettings(struct ADAPTER *prAdapter,
			struct CMD_SET_COUNTRY_CHANNEL_POWER_LIMIT *prCmd,
			uint8_t *bandedgeParam)
{
	struct LINK_ENTRY *prCur, *prNext;
	struct TX_PWR_CTRL_ELEMENT *element = NULL;
	struct LINK *aryprlist[2] = {
		&prAdapter->rTxPwr_DefaultList,
		&prAdapter->rTxPwr_DynamicList
	};
	int32_t i;

	/* show the tx power ctrl applied list */
	txPwrCtrlShowList(prAdapter, 1, "applied list");

	for (i = 0; i < ARRAY_SIZE(aryprlist); i++) {
		LINK_FOR_EACH_SAFE(prCur, prNext, aryprlist[i]) {
			element = LINK_ENTRY(prCur,
					struct TX_PWR_CTRL_ELEMENT, node);
			if (element->fgApplied == TRUE)
				txPwrApplyOneSetting(
					prCmd, element, bandedgeParam);
		}
	}

	return 0;
}

char *txPwrGetString(char **pcContent, char *delim)
{
	char *result = NULL;

	if (pcContent == NULL)
		return NULL;

	result = kalStrSep(pcContent, delim);
	if ((pcContent == NULL) || (result == NULL) ||
	    ((result != NULL) && (kalStrLen(result) == 0)))
		return NULL;

	return result;
}

#if CFG_SUPPORT_DYNAMIC_PWR_LIMIT_ANT_TAG
void txPwrParseTagDump(struct TX_PWR_CTRL_ELEMENT *pRecord)
{
	uint32_t i = 0, j = 0;

	for (i = 0; i < POWER_ANT_TAG_NUM; i++) {
		if (pRecord->aiPwrAnt[i].fgApplied == FALSE)
			continue;

		DBGLOG(RLM, TRACE, "Tag (%s) id (%d) :",
			g_auTxPwrTagTable[i].arTagNames, i);

		for (j = 0; j < POWER_ANT_NUM; j++) {
			DBGLOG(RLM, TRACE, "2G4: [%d]",
				pRecord->aiPwrAnt[i].aiPwrAnt2G4[j]);
			DBGLOG(RLM, TRACE, "5GB1: [%d]",
				pRecord->aiPwrAnt[i].aiPwrAnt5GB1[j]);
			DBGLOG(RLM, TRACE, "5GB2: [%d]",
				pRecord->aiPwrAnt[i].aiPwrAnt5GB2[j]);
			DBGLOG(RLM, TRACE, "5GB3: [%d]",
				pRecord->aiPwrAnt[i].aiPwrAnt5GB3[j]);
			DBGLOG(RLM, TRACE, "5GB4: [%d]",
				pRecord->aiPwrAnt[i].aiPwrAnt5GB4[j]);
#if (CFG_SUPPORT_WIFI_6G == 1)
			DBGLOG(RLM, TRACE, "6GB1: [%d]",
				pRecord->aiPwrAnt[i].aiPwrAnt6GB1[j]);
			DBGLOG(RLM, TRACE, "6GB2: [%d]",
				pRecord->aiPwrAnt[i].aiPwrAnt6GB2[j]);
			DBGLOG(RLM, TRACE, "6GB3: [%d]",
				pRecord->aiPwrAnt[i].aiPwrAnt6GB3[j]);
			DBGLOG(RLM, TRACE, "6GB4: [%d]",
				pRecord->aiPwrAnt[i].aiPwrAnt6GB4[j]);
#endif
		}
	}
}

int32_t txPwrParseTagXXXT(char *pStart, char *pEnd, uint8_t cTagParaNum,
	struct TX_PWR_CTRL_ELEMENT *pRecord, enum ENUM_POWER_ANT_TAG eTag)
{
	char *pCurent = NULL, *pContent = NULL;
	uint8_t i = 0, j = 0;
	uint8_t ucBId = 0, ucAId = 0;
	uint8_t op = 0, value = 0;
	int8_t backoff = 0;

	if (!pStart || !pEnd || !pRecord)
		return -1;

	DBGLOG(RLM, TRACE, "parse tag Para (%s) to aiPwrAnt[%u]", pStart, eTag);

	pCurent = pStart;
	for (i = 0; i < cTagParaNum; i++) {
		if (!pCurent || pCurent >= pEnd)
			break;

		pContent = txPwrGetString(&pCurent, ",");
		if (!pContent) {
			DBGLOG(RLM, ERROR,
			       "tag parameter format error: %s\n",
			       pStart);
			break;
		}

		if (txPwrParseNumber(&pContent, ",", &op, &value)) {
			DBGLOG(RLM, ERROR,
			       "parse parameter error: %s\n",
			       pContent);
			break;
		}

		backoff = (op == 1) ? value : (0 - value);
		if (backoff < PWR_CFG_LIMIT_MIN)
			backoff = PWR_CFG_LIMIT_MIN;
		if (backoff > PWR_CFG_LIMIT_MAX)
			backoff = PWR_CFG_LIMIT_MAX;

		ucBId = i % POWER_ANT_BAND_NUM;
		ucAId = i / POWER_ANT_BAND_NUM;
		switch (ucBId) {
		case POWER_ANT_2G4_BAND:
			pRecord->aiPwrAnt[eTag].aiPwrAnt2G4[ucAId] = backoff;
			break;
		case POWER_ANT_5G_BAND1:
			pRecord->aiPwrAnt[eTag].aiPwrAnt5GB1[ucAId] = backoff;
			break;
		case POWER_ANT_5G_BAND2:
			pRecord->aiPwrAnt[eTag].aiPwrAnt5GB2[ucAId] = backoff;
			break;
		case POWER_ANT_5G_BAND3:
			pRecord->aiPwrAnt[eTag].aiPwrAnt5GB3[ucAId] = backoff;
			break;
		case POWER_ANT_5G_BAND4:
			pRecord->aiPwrAnt[eTag].aiPwrAnt5GB4[ucAId] = backoff;
			break;
#if (CFG_SUPPORT_WIFI_6G == 1)
		case POWER_ANT_6G_BAND1:
			pRecord->aiPwrAnt[eTag].aiPwrAnt6GB1[ucAId] = backoff;
			break;
		case POWER_ANT_6G_BAND2:
			pRecord->aiPwrAnt[eTag].aiPwrAnt6GB2[ucAId] = backoff;
			break;
		case POWER_ANT_6G_BAND3:
			pRecord->aiPwrAnt[eTag].aiPwrAnt6GB3[ucAId] = backoff;
			break;
		case POWER_ANT_6G_BAND4:
			pRecord->aiPwrAnt[eTag].aiPwrAnt6GB4[ucAId] = backoff;
			break;
#endif
		default:
			DBGLOG(RLM, ERROR, "Never happen: %s\n", pStart);
			return -1;
		}

		if (pCurent >= pEnd)
			break;
	}

	if (i != cTagParaNum) {
		DBGLOG(RLM, ERROR, "parameter number error: %s\n", pStart);
		for (j = 0; j < POWER_ANT_TAG_NUM; j++) {
			kalMemSet(&pRecord->aiPwrAnt[j], 0,
				sizeof(struct TX_PWR_CTRL_ANT_SETTING));
		}
		return -1;
	}
	pRecord->aiPwrAnt[eTag].fgApplied = TRUE;

	DBGLOG(RLM, TRACE, "[Success] Dump aiPwrAnt[%u] para: ", eTag);
	for (j = 0; j < POWER_ANT_NUM; j++)
#if (CFG_SUPPORT_WIFI_6G == 1)
		DBGLOG(RLM, TRACE, "[%d][%d][%d][%d][%d][%d][%d][%d][%d]",
			pRecord->aiPwrAnt[eTag].aiPwrAnt2G4[j],
			pRecord->aiPwrAnt[eTag].aiPwrAnt5GB1[j],
			pRecord->aiPwrAnt[eTag].aiPwrAnt5GB2[j],
			pRecord->aiPwrAnt[eTag].aiPwrAnt5GB3[j],
			pRecord->aiPwrAnt[eTag].aiPwrAnt5GB4[j],
			pRecord->aiPwrAnt[eTag].aiPwrAnt6GB1[j],
			pRecord->aiPwrAnt[eTag].aiPwrAnt6GB2[j],
			pRecord->aiPwrAnt[eTag].aiPwrAnt6GB3[j],
			pRecord->aiPwrAnt[eTag].aiPwrAnt6GB4[j]);

#else
		DBGLOG(RLM, TRACE, "[%d][%d][%d][%d][%d]",
			pRecord->aiPwrAnt[eTag].aiPwrAnt2G4[j],
			pRecord->aiPwrAnt[eTag].aiPwrAnt5GB1[j],
			pRecord->aiPwrAnt[eTag].aiPwrAnt5GB2[j],
			pRecord->aiPwrAnt[eTag].aiPwrAnt5GB3[j],
			pRecord->aiPwrAnt[eTag].aiPwrAnt5GB4[j]);
#endif

	return 0;
}

int32_t txPwrParseTag(char *pTagStart, char *pTagEnd,
	struct TX_PWR_CTRL_ELEMENT *pRecord)
{
	uint8_t i = 0;
	uint8_t cTagParaNum = 0;
	int32_t ret = 0;
	char *pCurent = NULL, *pNext = NULL;

	if (!pTagStart || !pTagEnd || !pRecord)
		return -1;

	DBGLOG(RLM, TRACE, "Parse new tag %s\n", pTagStart);

	for (pCurent = pTagStart; pCurent < pTagEnd; pCurent++) {
		if ((*pCurent) == ',')
			cTagParaNum++;
	}

	pCurent = pTagStart;
	pNext = txPwrGetString(&pCurent, ",");
	if (!pNext) {
		DBGLOG(RLM, INFO, "Parse tag name error, %s\n", pTagStart);
		return -1;
	}

	for (i = 0;
	     i < sizeof(g_auTxPwrTagTable)/sizeof(struct TX_PWR_TAG_TABLE);
	     i++){
		DBGLOG(RLM, TRACE,
		       "Parse tag name [%s] handler name[%s]\n",
		       pNext, g_auTxPwrTagTable[i].arTagNames);

		if (kalStrCmp(pNext, g_auTxPwrTagTable[i].arTagNames) == 0) {
			DBGLOG(RLM, TRACE,
			       "find tag [%s] with %d para\n",
			       g_auTxPwrTagTable[i].arTagNames, cTagParaNum);

			if (cTagParaNum != g_auTxPwrTagTable[i].ucTagParaNum) {
				DBGLOG(RLM, ERROR,
				       "Tag para number error (%d != %d)",
				       cTagParaNum,
				       g_auTxPwrTagTable[i].ucTagParaNum);
				return -1;
			}

			ret = txPwrParseTagXXXT(
					pCurent,
					pTagEnd,
					cTagParaNum,
					pRecord,
					g_auTxPwrTagTable[i].eTag);

			txPwrParseTagDump(pRecord);

			return ret;
		}
	}

	DBGLOG(RLM, INFO, "Undefined tag name [%s]\n", pCurent);

	return -1;
}

int32_t txPwrOnPreParseAppendTag(struct TX_PWR_CTRL_ELEMENT *pRecord)
{
	if (!pRecord)
		return -1;

	kalMemSet(&(pRecord->aiPwrAnt[0]), 0,
		POWER_ANT_TAG_NUM * sizeof(struct TX_PWR_CTRL_ANT_SETTING));

	return 0;
}

int32_t txPwrParseAppendTag(char *pcStart,
	char *pcEnd, struct TX_PWR_CTRL_ELEMENT *pRecord)
{
	char *pcContCur = NULL, *pcContTemp = NULL;
	uint32_t ucTagCount1 = 0, ucTagCount2 = 0;
	uint8_t i = 0;

	if (!pcStart || !pcEnd || !pRecord) {
		DBGLOG(RLM, INFO, "%p-%p-%p!\n", pcStart, pcEnd, pRecord);
		return -1;
	}

	/* parse power limit append tag to list. */
	pcContCur = pcStart;
	pcContTemp = pcContCur;

	while (pcContTemp <= pcEnd) {
		if ((*pcContTemp) == '<')
			ucTagCount1++;
		if ((*pcContTemp) == '>')
			ucTagCount2++;
		pcContTemp++;
	}
	if (ucTagCount1 != ucTagCount2) {
		DBGLOG(RLM, INFO, "Wrong tag append %s!\n", pcStart);
		return -1;
	}
	if (ucTagCount1 == 0)
		return 0;

	DBGLOG(RLM, TRACE, "New config total %u tag append %s !\n",
		ucTagCount1, pcStart);

	for (i = 0; i < ucTagCount1; i++) {
		pcContTemp = txPwrGetString(&pcContCur, "<");
		if (!pcContCur || pcContCur > pcEnd) {
			DBGLOG(RLM, INFO, "No content after '<' !\n");
			return -1;
		}
		pcContTemp = txPwrGetString(&pcContCur, ">");
		if (!pcContTemp) {
			DBGLOG(RLM, INFO, "Can not find content after '<' !\n");
			return -1;
		}
		if (!pcContCur || pcContCur > pcEnd) {
			DBGLOG(RLM, INFO, "No content after '>' !\n");
			return -1;
		}
		if (txPwrParseTag(pcContTemp, pcContCur - 1, pRecord)) {
			DBGLOG(RLM, INFO, "Parse one tag fail !\n");
			return -1;
		}

		DBGLOG(RLM, TRACE, "Parse tag (%u/%u) success!\n",
			i + 1, ucTagCount1);
	}

	return 0;
}
#endif  /* CFG_SUPPORT_DYNAMIC_PWR_LIMIT_ANT_TAG */

struct TX_PWR_CTRL_ELEMENT *txPwrCtrlStringToStruct(char *pcContent,
						    u_int8_t fgSkipHeader)
{
	struct TX_PWR_CTRL_ELEMENT *prCurElement = NULL;
	struct TX_PWR_CTRL_CHANNEL_SETTING *prTmpSetting;
	char acTmpName[MAX_TX_PWR_CTRL_ELEMENT_NAME_SIZE];
	char *pcContCur = NULL, *pcContCur2 = NULL, *pcContEnd = NULL;
	char *pcContTmp = NULL, *pcContNext = NULL, *pcContOld = NULL;
	char carySeperator[2] = { 0, 0 };
	uint32_t u4MemSize = sizeof(struct TX_PWR_CTRL_ELEMENT);
	uint32_t copySize = 0;
	uint8_t i, j, op, ucSettingCount = 0;
	uint8_t value, value2, count = 0;
	uint8_t ucAppliedWay, ucOperation = 0;
	uint8_t ucCommaCount;

	if (!pcContent) {
		DBGLOG(RLM, ERROR, "pcContent is null\n");
		return NULL;
	}

	pcContCur = pcContent;
	pcContEnd = pcContent + kalStrLen(pcContent);

	if (fgSkipHeader == TRUE)
		goto skipLabel;

	/* insert elenemt into prTxPwrCtrlList */
	/* parse scenario name */
	kalMemZero(acTmpName, MAX_TX_PWR_CTRL_ELEMENT_NAME_SIZE);
	pcContOld = pcContCur;
	pcContTmp = txPwrGetString(&pcContCur, ";");
	if (!pcContTmp) {
		DBGLOG(RLM, ERROR, "parse scenario name error: %s\n",
		       pcContOld);
		return NULL;
	}
	copySize = kalStrLen(pcContTmp);
	if (copySize >= MAX_TX_PWR_CTRL_ELEMENT_NAME_SIZE)
		copySize = MAX_TX_PWR_CTRL_ELEMENT_NAME_SIZE - 1;
	kalMemCopy(acTmpName, pcContTmp, copySize);
	acTmpName[copySize] = 0;

	/* parese scenario sub index */
	pcContOld = pcContCur;
	if (txPwrParseNumber(&pcContCur, ";", &op, &value)) {
		DBGLOG(RLM, ERROR, "parse scenario sub index error: %s\n",
		       pcContOld);
		return NULL;
	}
	if (op != 1) {
		DBGLOG(RLM, ERROR,
		       "parse scenario sub index error: op=%u, val=%d\n",
		       op, value);
		return NULL;
	}

	/* parese scenario applied way */
	pcContOld = pcContCur;
	if (txPwrParseNumber(&pcContCur, ";", &op, &ucAppliedWay)) {
		DBGLOG(RLM, ERROR, "parse applied way error: %s\n",
		       pcContOld);
		return NULL;
	}
	if ((ucAppliedWay < PWR_CTRL_TYPE_APPLIED_WAY_WIFION) ||
	    (ucAppliedWay > PWR_CTRL_TYPE_APPLIED_WAY_IOCTL)) {
		DBGLOG(RLM, ERROR,
		       "parse applied way error: value=%u\n",
		       ucAppliedWay);
		return NULL;
	}

	/* parese scenario applied type */
	pcContOld = pcContCur;
	if (txPwrParseNumber(&pcContCur, ";", &op, &ucOperation)) {
		DBGLOG(RLM, ERROR, "parse operation error: %s\n",
		       pcContOld);
		return NULL;
	}
	if ((ucOperation < PWR_CTRL_TYPE_OPERATION_POWER_LEVEL) ||
	    (ucOperation > PWR_CTRL_TYPE_OPERATION_POWER_OFFSET)) {
		DBGLOG(RLM, ERROR,
		       "parse operation error: value=%u\n",
		       ucOperation);
		return NULL;
	}

	switch (ucAppliedWay) {
	case PWR_CTRL_TYPE_APPLIED_WAY_WIFION:
		if (ucOperation == PWR_CTRL_TYPE_OPERATION_POWER_LEVEL)
			value2 = PWR_CTRL_TYPE_WIFION_POWER_LEVEL;
		else
			value2 = PWR_CTRL_TYPE_WIFION_POWER_OFFSET;
		break;
	case PWR_CTRL_TYPE_APPLIED_WAY_IOCTL:
		if (ucOperation == PWR_CTRL_TYPE_OPERATION_POWER_LEVEL)
			value2 = PWR_CTRL_TYPE_IOCTL_POWER_LEVEL;
		else
			value2 = PWR_CTRL_TYPE_IOCTL_POWER_OFFSET;
		break;
	}

skipLabel:
	/* decide how many channel setting */
	pcContOld = pcContCur;
	while (pcContCur <= pcContEnd) {
		if ((*pcContCur) == '[')
			ucSettingCount++;
		pcContCur++;
	}

	if (ucSettingCount == 0) {
		DBGLOG(RLM, ERROR,
		       "power ctrl channel setting is empty\n");
		return NULL;
	}

	/* allocate memory for control element */
	u4MemSize += (ucSettingCount - 1) *
			sizeof(struct TX_PWR_CTRL_CHANNEL_SETTING);
	prCurElement = (struct TX_PWR_CTRL_ELEMENT *)kalMemAlloc(
					u4MemSize, VIR_MEM_TYPE);
	if (!prCurElement) {
		DBGLOG(RLM, ERROR,
		       "alloc power ctrl element failed\n");
		return NULL;
	}

	/* assign values into control element */
	kalMemZero(prCurElement, u4MemSize);
	if (fgSkipHeader == FALSE) {
		kalMemCopy(prCurElement->name, acTmpName, copySize);
		prCurElement->index = (uint8_t)value;
		prCurElement->eCtrlType = (enum ENUM_TX_POWER_CTRL_TYPE)value2;
		if (prCurElement->eCtrlType <=
		    PWR_CTRL_TYPE_WIFION_POWER_OFFSET)
			prCurElement->fgApplied = TRUE;
	}
	prCurElement->settingCount = ucSettingCount;

	/* parse channel setting list */
	pcContCur = pcContOld + 1; /* skip '[' */

	for (i = 0; i < ucSettingCount; i++) {
		if (pcContCur >= pcContEnd) {
			DBGLOG(RLM, ERROR,
			       "parse error: out of bound\n");
			goto clearLabel;
		}

		prTmpSetting = &prCurElement->rChlSettingList[i];

		/* verify there is ] symbol */
		pcContNext = kalStrChr(pcContCur, ']');
		if (!pcContNext) {
			DBGLOG(RLM, ERROR,
			       "parse error: miss symbol ']', %s\n",
			       pcContCur);
			goto clearLabel;
		}

		/* verify this setting segment number */
		pcContTmp = pcContCur;
		count = 0;
		while (pcContTmp < pcContNext) {
			if (*pcContTmp == ',')
				count++;
			pcContTmp++;
		}
		if ((count != PWR_LIMIT_NUM) &&
		    (count != PWR_CFG_PRAM_NUM_ALL_RATE)) {
			DBGLOG(RLM, ERROR,
			       "parse error: not %d segments, %s\n",
			       PWR_LIMIT_NUM, pcContCur);
			goto clearLabel;
		} else {
			ucCommaCount = count;
			if (ucCommaCount == PWR_LIMIT_NUM)
				carySeperator[0] = ',';
			else
				carySeperator[0] = ']';
		}

		/* parse channel setting type */
		pcContOld = pcContCur;
		pcContTmp = txPwrGetString(&pcContCur, ",");
		if (!pcContTmp) {
			DBGLOG(RLM, ERROR,
			       "parse channel setting type error, %s\n",
			       pcContOld);
			goto clearLabel;
		/* "ALL" */
		} else if (kalStrCmp(pcContTmp,
				     PWR_CTRL_CHNL_TYPE_KEY_ALL) == 0)
			prTmpSetting->eChnlType =
				PWR_CTRL_CHNL_TYPE_ALL;
		/* "2G4" */
		else if (kalStrCmp(pcContTmp,
				   PWR_CTRL_CHNL_TYPE_KEY_2G4) == 0)
			prTmpSetting->eChnlType =
				PWR_CTRL_CHNL_TYPE_2G4;
		/* "5G" */
		else if (kalStrCmp(pcContTmp,
				   PWR_CTRL_CHNL_TYPE_KEY_5G) == 0)
			prTmpSetting->eChnlType =
				PWR_CTRL_CHNL_TYPE_5G;
		/* "BANDEDGE2G4" */
		else if (kalStrCmp(pcContTmp,
				   PWR_CTRL_CHNL_TYPE_KEY_BANDEDGE_2G4) == 0)
			prTmpSetting->eChnlType =
				PWR_CTRL_CHNL_TYPE_BANDEDGE_2G4;
		/* "BANDEDGE5G" */
		else if (kalStrCmp(pcContTmp,
				   PWR_CTRL_CHNL_TYPE_KEY_BANDEDGE_5G) == 0)
			prTmpSetting->eChnlType =
				PWR_CTRL_CHNL_TYPE_BANDEDGE_5G;
		/* "5GBAND1" */
		else if (kalStrCmp(pcContTmp,
				   PWR_CTRL_CHNL_TYPE_KEY_5G_BAND1) == 0)
			prTmpSetting->eChnlType =
				PWR_CTRL_CHNL_TYPE_5G_BAND1;
		/* "5GBAND2" */
		else if (kalStrCmp(pcContTmp,
				   PWR_CTRL_CHNL_TYPE_KEY_5G_BAND2) == 0)
			prTmpSetting->eChnlType =
				PWR_CTRL_CHNL_TYPE_5G_BAND2;
		/* "5GBAND3" */
		else if (kalStrCmp(pcContTmp,
				   PWR_CTRL_CHNL_TYPE_KEY_5G_BAND3) == 0)
			prTmpSetting->eChnlType =
				PWR_CTRL_CHNL_TYPE_5G_BAND3;
		/* "5GBAND4" */
		else if (kalStrCmp(pcContTmp,
				   PWR_CTRL_CHNL_TYPE_KEY_5G_BAND4) == 0)
			prTmpSetting->eChnlType =
				PWR_CTRL_CHNL_TYPE_5G_BAND4;
#if (CFG_SUPPORT_WIFI_6G == 1)
		/* "6G" */
		else if (kalStrCmp(pcContTmp,
				   PWR_CTRL_CHNL_TYPE_KEY_6G) == 0)
			prTmpSetting->eChnlType =
				PWR_CTRL_CHNL_TYPE_6G;
		/* "6GBAND1" */
		else if (kalStrCmp(pcContTmp,
				   PWR_CTRL_CHNL_TYPE_KEY_6G_BAND1) == 0)
			prTmpSetting->eChnlType =
				PWR_CTRL_CHNL_TYPE_6G_BAND1;
		/* "6GBAND2" */
		else if (kalStrCmp(pcContTmp,
				   PWR_CTRL_CHNL_TYPE_KEY_6G_BAND2) == 0)
			prTmpSetting->eChnlType =
				PWR_CTRL_CHNL_TYPE_6G_BAND2;
		/* "6GBAND3" */
		else if (kalStrCmp(pcContTmp,
				   PWR_CTRL_CHNL_TYPE_KEY_6G_BAND3) == 0)
			prTmpSetting->eChnlType =
				PWR_CTRL_CHNL_TYPE_6G_BAND3;
		/* "6GBAND4" */
		else if (kalStrCmp(pcContTmp,
				   PWR_CTRL_CHNL_TYPE_KEY_6G_BAND4) == 0)
			prTmpSetting->eChnlType =
				PWR_CTRL_CHNL_TYPE_6G_BAND4;
#endif
		else {
			pcContCur2 = pcContTmp;
			pcContTmp = txPwrGetString(&pcContCur2, "-");
			if (pcContTmp == NULL) {
				DBGLOG(RLM, ERROR,
					"parse channel pcContTmp NULL\n");
				goto clearLabel;
			}

			if (pcContCur2 == NULL) { /* case: normal channel */
				if (kalkStrtou8(pcContTmp, 0, &value) != 0) {
					DBGLOG(RLM, ERROR,
					       "parse channel error: %s\n",
					       pcContTmp);
					goto clearLabel;
				}
				prTmpSetting->channelParam[0] = value;
				prTmpSetting->eChnlType =
						PWR_CTRL_CHNL_TYPE_NORMAL;
			} else { /* case: channel range */
				if (kalkStrtou8(pcContTmp, 0, &value) != 0) {
					DBGLOG(RLM, ERROR,
					       "parse first channel error, %s\n",
					       pcContTmp);
					goto clearLabel;
				}
				if (kalkStrtou8(pcContCur2, 0, &value2) != 0) {
					DBGLOG(RLM, ERROR,
					       "parse second channel error, %s\n",
					       pcContCur2);
					goto clearLabel;
				}
				prTmpSetting->channelParam[0] = value;
				prTmpSetting->channelParam[1] =	value2;
				prTmpSetting->eChnlType =
						PWR_CTRL_CHNL_TYPE_RANGE;
			}
		}

		if (count == PWR_CFG_PRAM_NUM_ALL_RATE) {
			/* parse all rate setting */
			pcContOld = pcContCur;
			if (txPwrParseNumber(&pcContCur, "]", &op, &value)) {
				DBGLOG(RLM, ERROR,
				       "parse all rate limit error, %s\n",
				       pcContOld);
				goto clearLabel;
			}

			for (j = 0; j < PWR_LIMIT_NUM; j++) {
				prTmpSetting->op[j] = op;
				prTmpSetting->i8PwrLimit[j] =
					(op != 2) ? value : (0 - value);
			}
		} else if (count == PWR_LIMIT_NUM) {
			for (j = 0; j < PWR_LIMIT_NUM; j++) {
				if (j == PWR_LIMIT_NUM - 1)
					carySeperator[0] = ']';
				else
					carySeperator[0] = ',';

				pcContOld = pcContCur;
				if (txPwrParseNumber(&pcContCur,
					carySeperator, &op, &value)) {
					DBGLOG(RLM, ERROR,
					       "parse %s error, %s\n",
					       gTx_Pwr_Limit_Section[
							PWR_CFG_PARM_VERSION].
							arSectionNames[j],
					       pcContOld);
					goto clearLabel;
				}
				prTmpSetting->op[j] =
					(enum ENUM_TX_POWER_CTRL_VALUE_SIGN)op;
				prTmpSetting->i8PwrLimit[j] =
					(op != 2) ? value : (0 - value);
				if ((prTmpSetting->op[j] ==
				    PWR_CTRL_TYPE_POSITIVE) &&
				    (ucOperation ==
				    PWR_CTRL_TYPE_OPERATION_POWER_OFFSET)) {
					DBGLOG(RLM, ERROR,
					       "parse %s error, Power_Offset value cannot be positive: %u\n",
					       gTx_Pwr_Limit_Section[
							PWR_CFG_PARM_VERSION].
							arSectionNames[j],
					       value);
					goto clearLabel;
				}
			}
		}

		pcContCur = pcContNext + 2;
	}

#if CFG_SUPPORT_DYNAMIC_PWR_LIMIT_ANT_TAG
		/* parse power limit append tag to list. */
		pcContCur -= 1;

		if (txPwrOnPreParseAppendTag(prCurElement)) {
			DBGLOG(RLM, INFO,
				"txPwrOnPreParseAppendTag fail.");
		}

		if (txPwrParseAppendTag(pcContCur, pcContEnd, prCurElement))
			DBGLOG(RLM, INFO,
			       "txPwrParseAppendTag fail (%s).",
			       pcContent);
#endif

	return prCurElement;

clearLabel:
	if (prCurElement != NULL)
		kalMemFree(prCurElement, VIR_MEM_TYPE, u4MemSize);

	return NULL;
}

/* filterType: 0:no filter, 1:fgEnable is TRUE */
int txPwrCtrlListSize(struct ADAPTER *prAdapter, uint8_t filterType)
{
	struct LINK_ENTRY *prCur, *prNext;
	struct TX_PWR_CTRL_ELEMENT *prCurElement = NULL;
	struct LINK *aryprlist[2] = {
		&prAdapter->rTxPwr_DefaultList,
		&prAdapter->rTxPwr_DynamicList
	};
	int i, count = 0;

	for (i = 0; i < ARRAY_SIZE(aryprlist); i++) {
		LINK_FOR_EACH_SAFE(prCur, prNext, aryprlist[i]) {
			prCurElement = LINK_ENTRY(prCur,
				struct TX_PWR_CTRL_ELEMENT, node);
			if ((filterType == 1) &&
			    (prCurElement->fgApplied != TRUE))
				continue;
			count++;
		}
	}

	return count;
}

/* filterType: 0:no filter, 1:fgApplied is TRUE */
void txPwrCtrlShowList(struct ADAPTER *prAdapter, uint8_t filterType,
		       char *message)
{
	struct LINK_ENTRY *prCur, *prNext;
	struct TX_PWR_CTRL_ELEMENT *prCurElement = NULL;
	struct TX_PWR_CTRL_CHANNEL_SETTING *prChlSettingList;
	struct LINK *aryprlist[2] = {
		&prAdapter->rTxPwr_DefaultList,
		&prAdapter->rTxPwr_DynamicList
	};
	uint8_t ucAppliedWay, ucOperation;
	int i, j, count = 0;

	if (filterType == 1)
		DBGLOG(RLM, INFO, "Tx Power Ctrl List=[%s], Size=[%d]",
		       message, txPwrCtrlListSize(prAdapter, filterType));
	else
		DBGLOG(RLM, TRACE, "Tx Power Ctrl List=[%s], Size=[%d]",
		       message, txPwrCtrlListSize(prAdapter, filterType));

	for (i = 0; i < ARRAY_SIZE(aryprlist); i++) {
		LINK_FOR_EACH_SAFE(prCur, prNext, aryprlist[i]) {
			prCurElement = LINK_ENTRY(prCur,
					struct TX_PWR_CTRL_ELEMENT, node);
			if ((filterType == 1) &&
			    (prCurElement->fgApplied != TRUE))
				continue;

			switch (prCurElement->eCtrlType) {
			case PWR_CTRL_TYPE_WIFION_POWER_LEVEL:
				ucAppliedWay =
					PWR_CTRL_TYPE_APPLIED_WAY_WIFION;
				ucOperation =
					PWR_CTRL_TYPE_OPERATION_POWER_LEVEL;
				break;
			case PWR_CTRL_TYPE_WIFION_POWER_OFFSET:
				ucAppliedWay =
					PWR_CTRL_TYPE_APPLIED_WAY_WIFION;
				ucOperation =
					PWR_CTRL_TYPE_OPERATION_POWER_OFFSET;
				break;
			case PWR_CTRL_TYPE_IOCTL_POWER_LEVEL:
				ucAppliedWay =
					PWR_CTRL_TYPE_APPLIED_WAY_IOCTL;
				ucOperation =
					PWR_CTRL_TYPE_OPERATION_POWER_LEVEL;
				break;
			case PWR_CTRL_TYPE_IOCTL_POWER_OFFSET:
				ucAppliedWay =
					PWR_CTRL_TYPE_APPLIED_WAY_IOCTL;
				ucOperation =
					PWR_CTRL_TYPE_OPERATION_POWER_OFFSET;
				break;
			default:
				ucAppliedWay = 0;
				ucOperation = 0;
				break;
			}

			DBGLOG(RLM, TRACE,
			       "Tx Power Ctrl Element-%u: name=[%s], index=[%u], appliedWay=[%u:%s], operation=[%u:%s], ChlSettingCount=[%u]\n",
			       ++count, prCurElement->name, prCurElement->index,
			       ucAppliedWay,
			       g_au1TxPwrAppliedWayLabel[ucAppliedWay - 1],
			       ucOperation,
			       g_au1TxPwrOperationLabel[ucOperation - 1],
			       prCurElement->settingCount);
			prChlSettingList = &(prCurElement->rChlSettingList[0]);
			for (j = 0; j < prCurElement->settingCount; j++) {
				DBGLOG(RLM, TRACE,
					"Setting-%u:[%s:%u,%u],[%u,%d],[%u,%d],[%u,%d],[%u,%d],[%u,%d],[%u,%d],[%u,%d],[%u,%d],[%u,%d]\n",
					(j + 1),
					g_au1TxPwrChlTypeLabel[
					(uint32_t)prChlSettingList->eChnlType],
					prChlSettingList->channelParam[0],
					prChlSettingList->channelParam[1],
					prChlSettingList->op[0],
					prChlSettingList->i8PwrLimit[0],
					prChlSettingList->op[1],
					prChlSettingList->i8PwrLimit[1],
					prChlSettingList->op[2],
					prChlSettingList->i8PwrLimit[2],
					prChlSettingList->op[3],
					prChlSettingList->i8PwrLimit[3],
					prChlSettingList->op[4],
					prChlSettingList->i8PwrLimit[4],
					prChlSettingList->op[5],
					prChlSettingList->i8PwrLimit[5],
					prChlSettingList->op[6],
					prChlSettingList->i8PwrLimit[6],
					prChlSettingList->op[7],
					prChlSettingList->i8PwrLimit[7],
					prChlSettingList->op[8],
					prChlSettingList->i8PwrLimit[8]
				);
				prChlSettingList++;
			}
		}
	}
}

/* This function used to delete element by specifying name or index
 * if index is 0, deletion only according name
 * if index >= 1, deletion according name and index
 */
void _txPwrCtrlDeleteElement(struct ADAPTER *prAdapter,
			     uint8_t *name,
			     uint32_t index,
			     struct LINK *prTxPwrCtrlList)
{
	struct LINK_ENTRY *prCur, *prNext;
	struct TX_PWR_CTRL_ELEMENT *prCurElement = NULL;
	uint32_t u4MemSize = sizeof(struct TX_PWR_CTRL_ELEMENT);
	uint32_t u4MemSize2;
	uint32_t u4SettingSize = sizeof(struct TX_PWR_CTRL_CHANNEL_SETTING);
	uint8_t ucSettingCount;
	u_int8_t fgFind;

	LINK_FOR_EACH_SAFE(prCur, prNext, prTxPwrCtrlList) {
		fgFind = FALSE;
		prCurElement = LINK_ENTRY(prCur, struct TX_PWR_CTRL_ELEMENT,
					  node);
		if (prCurElement == NULL)
			return;
		if (kalStrCmp(prCurElement->name, name) == 0) {
			if (index == 0)
				fgFind = TRUE;
			else if (prCurElement->index == index)
				fgFind = TRUE;
			if (fgFind) {
				linkDel(prCur);
				ucSettingCount =
					prCurElement->settingCount;
					u4MemSize2 = u4MemSize +
					((ucSettingCount == 1) ? 0 :
					(ucSettingCount - 1) *
					u4SettingSize);
				kalMemFree(prCurElement, VIR_MEM_TYPE,
					u4MemSize2);
			}
		}
	}
}

void txPwrCtrlDeleteElement(struct ADAPTER *prAdapter,
			    uint8_t *name,
			    uint32_t index,
			    enum ENUM_TX_POWER_CTRL_LIST_TYPE eListType)
{
	if ((eListType == PWR_CTRL_TYPE_ALL_LIST) ||
	    (eListType == PWR_CTRL_TYPE_DEFAULT_LIST))
		_txPwrCtrlDeleteElement(prAdapter, name, index,
					&prAdapter->rTxPwr_DefaultList);

	if ((eListType == PWR_CTRL_TYPE_ALL_LIST) ||
	    (eListType == PWR_CTRL_TYPE_DYNAMIC_LIST))
		_txPwrCtrlDeleteElement(prAdapter, name, index,
					&prAdapter->rTxPwr_DynamicList);
}

struct TX_PWR_CTRL_ELEMENT *_txPwrCtrlFindElement(struct ADAPTER *prAdapter,
				 uint8_t *name, uint32_t index,
				 u_int8_t fgCheckIsApplied,
				 struct LINK *prTxPwrCtrlList)
{
	struct LINK_ENTRY *prCur, *prNext;
	struct TX_PWR_CTRL_ELEMENT *prCurElement = NULL;

	LINK_FOR_EACH_SAFE(prCur, prNext, prTxPwrCtrlList) {
		u_int8_t fgFind = FALSE;

		prCurElement = LINK_ENTRY(
			prCur, struct TX_PWR_CTRL_ELEMENT, node);
		if (kalStrCmp(prCurElement->name, name) == 0) {
			if ((!fgCheckIsApplied) ||
			    (fgCheckIsApplied &&
			    (prCurElement->fgApplied == TRUE))) {
				if (index == 0)
					fgFind = TRUE;
				else if (prCurElement->index == index)
					fgFind = TRUE;
			}
			if (fgFind)
				return prCurElement;
		}
	}
	return NULL;
}

struct TX_PWR_CTRL_ELEMENT *txPwrCtrlFindElement(struct ADAPTER *prAdapter,
				 uint8_t *name, uint32_t index,
				 u_int8_t fgCheckIsApplied,
				 enum ENUM_TX_POWER_CTRL_LIST_TYPE eListType)
{
	struct LINK *prTxPwrCtrlList = NULL;

	if (eListType == PWR_CTRL_TYPE_DEFAULT_LIST)
		prTxPwrCtrlList = &prAdapter->rTxPwr_DefaultList;
	if (eListType == PWR_CTRL_TYPE_DYNAMIC_LIST)
		prTxPwrCtrlList = &prAdapter->rTxPwr_DynamicList;
	if (prTxPwrCtrlList == NULL)
		return NULL;

	return _txPwrCtrlFindElement(prAdapter, name, index, fgCheckIsApplied,
				     prTxPwrCtrlList);
}

void txPwrCtrlAddElement(struct ADAPTER *prAdapter,
			 struct TX_PWR_CTRL_ELEMENT *prElement)
{
	struct LINK_ENTRY *prNode = &prElement->node;

	switch (prElement->eCtrlType) {
	case PWR_CTRL_TYPE_WIFION_POWER_LEVEL:
		linkAdd(prNode, &prAdapter->rTxPwr_DefaultList);
		break;
	case PWR_CTRL_TYPE_WIFION_POWER_OFFSET:
		linkAddTail(prNode, &prAdapter->rTxPwr_DefaultList);
		break;
	case PWR_CTRL_TYPE_IOCTL_POWER_LEVEL:
		linkAdd(prNode, &prAdapter->rTxPwr_DynamicList);
		break;
	case PWR_CTRL_TYPE_IOCTL_POWER_OFFSET:
		linkAddTail(prNode, &prAdapter->rTxPwr_DynamicList);
		break;
	}
}

void txPwrCtrlFileBufToList(struct ADAPTER *prAdapter, uint8_t *pucFileBuf)
{
	struct TX_PWR_CTRL_ELEMENT *prNewElement;
	char *oneLine;

	if (pucFileBuf == NULL)
		return;

	while ((oneLine = kalStrSep((char **)(&pucFileBuf), "\r\n"))
	       != NULL) {
		/* skip comment line and empty line */
		if ((oneLine[0] == '#') || (oneLine[0] == 0))
			continue;

		prNewElement = txPwrCtrlStringToStruct(oneLine, FALSE);
		if (prNewElement != NULL) {
			/* delete duplicated element
			 * by checking name and index
			 */
			txPwrCtrlDeleteElement(prAdapter,
				prNewElement->name, prNewElement->index,
				PWR_CTRL_TYPE_ALL_LIST);

			/* append to rTxPwr_List */
			txPwrCtrlAddElement(prAdapter, prNewElement);
		}
	}

	/* show the tx power ctrl list */
	txPwrCtrlShowList(prAdapter, 0, "config list, after loading cfg file");
}

void txPwrCtrlGlobalVariableToList(struct ADAPTER *prAdapter)
{
	struct TX_PWR_CTRL_ELEMENT *pcElement;
	char *ptr;
	int32_t i, u4MemSize;

	for (i = 0; i < ARRAY_SIZE(g_au1TxPwrDefaultSetting); i++) {
		/* skip empty line */
		if (g_au1TxPwrDefaultSetting[i][0] == 0)
			continue;
		u4MemSize = kalStrLen(g_au1TxPwrDefaultSetting[i]) + 1;
		ptr = (char *)kalMemAlloc(u4MemSize, VIR_MEM_TYPE);
		if (ptr == NULL) {
			DBGLOG(RLM, ERROR, "kalMemAlloc fail: %d\n", u4MemSize);
			continue;
		}
		kalMemCopy(ptr, g_au1TxPwrDefaultSetting[i], u4MemSize);
		*(ptr + u4MemSize - 1) = 0;
		pcElement = txPwrCtrlStringToStruct(ptr, FALSE);
		kalMemFree(ptr, VIR_MEM_TYPE, u4MemSize);
		if (pcElement != NULL) {
			/* delete duplicated element
			 * by checking name and index
			 */
			txPwrCtrlDeleteElement(prAdapter,
				pcElement->name, pcElement->index,
				PWR_CTRL_TYPE_ALL_LIST);

			/* append to rTxPwr_List */
			txPwrCtrlAddElement(prAdapter, pcElement);
		}
	}

	/* show the tx power ctrl cfg list */
	txPwrCtrlShowList(prAdapter, 0,
			  "config list, after loadding global variables");
}

void txPwrCtrlCfgFileToList(struct ADAPTER *prAdapter)
{
	uint8_t *pucConfigBuf;
	uint32_t u4ConfigReadLen = 0;

	pucConfigBuf = (uint8_t *)kalMemAlloc(WLAN_CFG_FILE_BUF_SIZE,
					      VIR_MEM_TYPE);
	kalMemZero(pucConfigBuf, WLAN_CFG_FILE_BUF_SIZE);
	if (pucConfigBuf) {
		if (kalRequestFirmware("txpowerctrl.cfg", pucConfigBuf,
		    WLAN_CFG_FILE_BUF_SIZE, &u4ConfigReadLen,
		    prAdapter->prGlueInfo->prDev) == 0) {
			/* ToDo:: Nothing */
		} else if (kalReadToFile("/data/misc/wifi/txpowerctrl.cfg",
			   pucConfigBuf, WLAN_CFG_FILE_BUF_SIZE,
			   &u4ConfigReadLen) == 0) {
			/* ToDo:: Nothing */
		} else if (kalReadToFile("/storage/sdcard0/txpowerctrl.cfg",
			   pucConfigBuf, WLAN_CFG_FILE_BUF_SIZE,
			   &u4ConfigReadLen) == 0) {
			/* ToDo:: Nothing */
		}

		if (pucConfigBuf[0] != '\0' && u4ConfigReadLen > 0)
			txPwrCtrlFileBufToList(prAdapter, pucConfigBuf);
		else
			DBGLOG(RLM, INFO,
			       "no txpowerctrl.cfg or file is empty\n");

		kalMemFree(pucConfigBuf, VIR_MEM_TYPE, WLAN_CFG_FILE_BUF_SIZE);
	}
}

void txPwrCtrlLoadConfig(struct ADAPTER *prAdapter)
{
	/* 1. add records from global tx power ctrl setting into cfg list */
	txPwrCtrlGlobalVariableToList(prAdapter);

	/* 2. update cfg list by txpowerctrl.cfg */
	txPwrCtrlCfgFileToList(prAdapter);

#if CFG_SUPPORT_PWR_LIMIT_COUNTRY
	/* 3. send setting to firmware */
	rlmDomainSendPwrLimitCmd(prAdapter);
#endif
}

void txPwrCtrlInit(struct ADAPTER *prAdapter)
{
	LINK_INITIALIZE(&prAdapter->rTxPwr_DefaultList);
	LINK_INITIALIZE(&prAdapter->rTxPwr_DynamicList);
}

void txPwrCtrlUninit(struct ADAPTER *prAdapter)
{
	struct LINK_ENTRY *prCur, *prNext;
	struct TX_PWR_CTRL_ELEMENT *prCurElement = NULL;
	struct LINK *aryprlist[2] = {
		&prAdapter->rTxPwr_DefaultList,
		&prAdapter->rTxPwr_DynamicList
	};
	uint32_t u4MemSize = sizeof(struct TX_PWR_CTRL_ELEMENT);
	uint32_t u4MemSize2;
	uint32_t u4SettingSize = sizeof(struct TX_PWR_CTRL_CHANNEL_SETTING);
	uint8_t ucSettingCount;
	int32_t i;

	for (i = 0; i < ARRAY_SIZE(aryprlist); i++) {
		LINK_FOR_EACH_SAFE(prCur, prNext, aryprlist[i]) {
			prCurElement = LINK_ENTRY(prCur,
					struct TX_PWR_CTRL_ELEMENT, node);
			linkDel(prCur);
			if (prCurElement) {
				ucSettingCount = prCurElement->settingCount;
					u4MemSize2 = u4MemSize +
					((ucSettingCount <= 1) ? 0 :
					(ucSettingCount - 1) * u4SettingSize);
				kalMemFree(prCurElement,
					VIR_MEM_TYPE, u4MemSize2);
			}
		}
	}
}

static ssize_t debug_dumpElement(struct TX_PWR_CTRL_ELEMENT *pcElement,
	char *buf, ssize_t maxSize)
{
	uint32_t i, j;
	uint8_t temp[256];

	if ((pcElement == NULL) || (buf == NULL) || (maxSize < 1))
		return 0;

	kalSnprintf(buf + kalStrLen(buf), maxSize - kalStrLen(buf),
		"name : %s\n", pcElement->name);
	kalSnprintf(buf + kalStrLen(buf), maxSize - kalStrLen(buf),
		"index: %d\n", pcElement->index);
	kalSnprintf(buf + kalStrLen(buf), maxSize - kalStrLen(buf),
		"ctrl : %s\n",
		g_au1TxPwrAppliedWayLabel[(pcElement->eCtrlType - 1) / 2]);
	kalSnprintf(buf + kalStrLen(buf), maxSize - kalStrLen(buf),
		"op   : %s\n",
		g_au1TxPwrOperationLabel[(pcElement->eCtrlType - 1) % 2]);
	kalSnprintf(buf + kalStrLen(buf), maxSize - kalStrLen(buf),
		"apply: %s\n",
		(pcElement->fgApplied) ? "YES" : "NO");
	for (i = 0; i < pcElement->settingCount; i++) {
		kalSnprintf(buf + kalStrLen(buf), maxSize - kalStrLen(buf),
			"    setting %d/%d:\n", i + 1, pcElement->settingCount);
		kalSnprintf(buf + kalStrLen(buf), maxSize - kalStrLen(buf),
			"        channel type: %s\n",
			g_au1TxPwrChlTypeLabel[
				pcElement->rChlSettingList[i].eChnlType]);
		kalSnprintf(buf + kalStrLen(buf), maxSize - kalStrLen(buf),
			"        channel param: %d - %d\n",
			pcElement->rChlSettingList[i].channelParam[0],
			pcElement->rChlSettingList[i].channelParam[1]);
		kalMemSet(temp, 0, sizeof(temp));
		for (j = 0; j < PWR_LIMIT_NUM; j++) {
			kalSnprintf(temp + kalStrLen(temp),
				sizeof(temp) - kalStrLen(temp) - 1,
				" %-7s",
				gTx_Pwr_Limit_Section[PWR_CFG_PARM_VERSION].
					arSectionNames[j]);
		}
		kalSnprintf(buf + kalStrLen(buf), maxSize - kalStrLen(buf),
			"        section:%s\n", temp);
		kalMemSet(temp, 0, sizeof(temp));
		for (j = 0; j < PWR_LIMIT_NUM; j++) {
			kalSnprintf(temp + kalStrLen(temp),
				sizeof(temp) - kalStrLen(temp) - 1,
				"%4s%-4d",
				(pcElement->rChlSettingList[i].op[j] ==
				PWR_CTRL_TYPE_NEGATIVE) ? "-" : " ",
				pcElement->rChlSettingList[i].i8PwrLimit[j]);
		}
		kalSnprintf(buf + kalStrLen(buf), maxSize - kalStrLen(buf),
				"        limit:%s\n", temp);
	}

#if CFG_SUPPORT_DYNAMIC_PWR_LIMIT_ANT_TAG
	kalSnprintf(buf + kalStrLen(buf), maxSize - kalStrLen(buf), "TAGS:\n");
	for (j = 0; j < POWER_ANT_TAG_NUM; j++) {
		if (pcElement->aiPwrAnt[j].fgApplied == FALSE) {
			kalSnprintf(buf + kalStrLen(buf),
				maxSize - kalStrLen(buf),
				"    %s: not applied\n",
				g_auTxPwrTagTable[j].arTagNames);
			continue;
		}
		kalSnprintf(buf + kalStrLen(buf),
			maxSize - kalStrLen(buf),
			"    %-7s: %d, %d, %d, %d, %d,",
			g_auTxPwrTagTable[j].arTagNames,
			pcElement->aiPwrAnt[j].aiPwrAnt2G4[POWER_ANT_WF0],
			pcElement->aiPwrAnt[j].aiPwrAnt5GB1[POWER_ANT_WF0],
			pcElement->aiPwrAnt[j].aiPwrAnt5GB2[POWER_ANT_WF0],
			pcElement->aiPwrAnt[j].aiPwrAnt5GB3[POWER_ANT_WF0],
			pcElement->aiPwrAnt[j].aiPwrAnt5GB4[POWER_ANT_WF0]);
#if (CFG_SUPPORT_WIFI_6G == 1)
		kalSnprintf(buf + kalStrLen(buf),
			maxSize - kalStrLen(buf),
			" %d, %d, %d, %d,",
			pcElement->aiPwrAnt[j].aiPwrAnt6GB1[POWER_ANT_WF0],
			pcElement->aiPwrAnt[j].aiPwrAnt6GB2[POWER_ANT_WF0],
			pcElement->aiPwrAnt[j].aiPwrAnt6GB3[POWER_ANT_WF0],
			pcElement->aiPwrAnt[j].aiPwrAnt6GB4[POWER_ANT_WF0]);
#endif
		kalSnprintf(buf + kalStrLen(buf),
			maxSize - kalStrLen(buf),
			" %d, %d, %d, %d, %d",
			pcElement->aiPwrAnt[j].aiPwrAnt2G4[POWER_ANT_WF1],
			pcElement->aiPwrAnt[j].aiPwrAnt5GB1[POWER_ANT_WF1],
			pcElement->aiPwrAnt[j].aiPwrAnt5GB2[POWER_ANT_WF1],
			pcElement->aiPwrAnt[j].aiPwrAnt5GB3[POWER_ANT_WF1],
			pcElement->aiPwrAnt[j].aiPwrAnt5GB4[POWER_ANT_WF1]);
#if (CFG_SUPPORT_WIFI_6G == 1)
		kalSnprintf(buf + kalStrLen(buf),
			maxSize - kalStrLen(buf),
			", %d, %d, %d, %d",
			pcElement->aiPwrAnt[j].aiPwrAnt6GB1[POWER_ANT_WF1],
			pcElement->aiPwrAnt[j].aiPwrAnt6GB2[POWER_ANT_WF1],
			pcElement->aiPwrAnt[j].aiPwrAnt6GB3[POWER_ANT_WF1],
			pcElement->aiPwrAnt[j].aiPwrAnt6GB4[POWER_ANT_WF1]);
#endif
		kalSnprintf(buf + kalStrLen(buf),
			maxSize - kalStrLen(buf), "\n");
	}
#endif  /* #if CFG_SUPPORT_DYNAMIC_PWR_LIMIT_ANT_TAG */

	return kalStrLen(buf);
}

static ssize_t debug_readAntLimit(struct GLUE_INFO *prGlueInfo,
	char *buf, ssize_t maxSize)
{
	struct PARAM_CUSTOM_MCR_RW_STRUCT rMcrInfo;
	uint32_t rStatus = WLAN_STATUS_SUCCESS;
	uint32_t u4BufLen, i;
	int8_t acAntPwrLimit[2];
	const uint32_t regAntLimit[2] = {0x8300C794, 0x8301C794};
	const uint32_t regErrReturn = 0xdeaddead;

	if ((prGlueInfo == NULL) || (buf == NULL) || (maxSize < 1))
		return 0;

	for (i = 0; i < 2; i++) {
		rMcrInfo.u4McrData = 0;
		rMcrInfo.u4McrOffset = regAntLimit[i];
		rStatus = kalIoctl(prGlueInfo,
			wlanoidQueryMcrRead, (void *)&rMcrInfo,
			sizeof(rMcrInfo), TRUE, TRUE, TRUE, &u4BufLen);
		if (rStatus != WLAN_STATUS_SUCCESS)
			goto errReturn;
		if (rMcrInfo.u4McrData == regErrReturn)
			rMcrInfo.u4McrData = 0;
		acAntPwrLimit[i] = (int8_t)(rMcrInfo.u4McrData >> 24);
	}

	kalSnprintf(buf + kalStrLen(buf), maxSize - kalStrLen(buf),
		"Ant power limit: %d, %d\n",
		acAntPwrLimit[0], acAntPwrLimit[1]);
	return kalStrLen(buf);

errReturn:
	kalSnprintf(buf + kalStrLen(buf), maxSize - kalStrLen(buf),
		"fail to get antLimit reg data\n");
	return kalStrLen(buf);
}

void debug_write_txPwrCtrlStringToStruct(char *pcContent)
{
	uint32_t u4MemSize = 0;
	uint8_t *name;
	uint8_t id;
	char *temp;
	struct GLUE_INFO *prGlueInfo;
	struct ADAPTER *prAdapter;

	g_ePwrCtrlDebugType = DEBUG_TYPE_NULL;

	/* readAntLimit */
	if (kalStrStr(pcContent, "readAntLimit") == pcContent) {
		g_ePwrCtrlDebugType = DEBUG_TYPE_DUMP_ANT_LIMIT;
		return;
	}

	/* dumpALL */
	if (kalStrStr(pcContent, "dumpAll") == pcContent) {
		g_ePwrCtrlDebugType = DEBUG_TYPE_DUMP_ALL;
		return;
	}

	if (fgNewDebugElement && (pcDebugElement != NULL)) {
		u4MemSize = sizeof(struct TX_PWR_CTRL_ELEMENT) +
			(pcDebugElement->settingCount - 1) *
			sizeof(struct TX_PWR_CTRL_CHANNEL_SETTING);
		kalMemFree(pcDebugElement, VIR_MEM_TYPE, u4MemSize);
	}
	fgNewDebugElement = FALSE;

	/* dumpElement,name,index */
	if (kalStrStr(pcContent, "dumpElement,") == pcContent) {
		g_ePwrCtrlDebugType = DEBUG_TYPE_DUMP_ELEMENT;
		prGlueInfo = wlanGetGlueInfo();
		if (!prGlueInfo)
			return;
		prAdapter = prGlueInfo->prAdapter;
		if (!prAdapter)
			return;
		temp = pcContent + kalStrLen("dumpElement,");
		name = txPwrGetString(&temp, ",");
		if ((!name) || (!temp))
			return;
		if (kalkStrtou8(temp, 0, &id) != 0)
			return;
		pcDebugElement = _txPwrCtrlFindElement(
				prAdapter, name, id, FALSE,
				&prAdapter->rTxPwr_DynamicList);
		return;
	}

	/* name;id;2;1;[...]<...> */
	fgNewDebugElement = TRUE;
	pcDebugElement = txPwrCtrlStringToStruct(pcContent, FALSE);
}

ssize_t debug_read_txPwrCtrlStringToStruct(char *buf, ssize_t maxSize)
{
	struct LINK_ENTRY *prCur, *prNext;
	struct TX_PWR_CTRL_ELEMENT *pcElement = NULL;
	struct GLUE_INFO *prGlueInfo;
	struct ADAPTER *prAdapter;

	prGlueInfo = wlanGetGlueInfo();
	if (!prGlueInfo)
		return 0;
	prAdapter = prGlueInfo->prAdapter;
	if (!prAdapter)
		return 0;

	if (g_ePwrCtrlDebugType == DEBUG_TYPE_DUMP_ANT_LIMIT)
		return debug_readAntLimit(prGlueInfo, buf, maxSize);
	else if (g_ePwrCtrlDebugType == DEBUG_TYPE_DUMP_ELEMENT)
		return debug_dumpElement(pcDebugElement, buf, maxSize);
	else if (g_ePwrCtrlDebugType != DEBUG_TYPE_DUMP_ALL)
		return 0;

	LINK_FOR_EACH_SAFE(prCur, prNext, &prAdapter->rTxPwr_DynamicList) {
		pcElement = LINK_ENTRY(prCur, struct TX_PWR_CTRL_ELEMENT, node);
		kalSnprintf(buf + kalStrLen(buf), maxSize - kalStrLen(buf),
			"    name: %s, index: %d\n",
			pcElement->name, pcElement->index);
	}

	return kalStrLen(buf);
}
/* dynamic tx power control: end **********************************************/
#endif /* CFG_SUPPORT_DYNAMIC_PWR_LIMIT */

void rlmDomainSendPwrLimitCmd(struct ADAPTER *prAdapter)
{
	struct CMD_SET_COUNTRY_CHANNEL_POWER_LIMIT *prCmd = NULL;
	uint32_t rStatus;
	uint8_t i;
	uint16_t u2DefaultTableIndex;
	uint32_t u4SetCmdTableMaxSize;
	uint32_t u4SetQueryInfoLen;
	struct CMD_CHANNEL_POWER_LIMIT *prCmdPwrLimit;	/* for print usage */
	uint8_t bandedgeParam[4] = { 0, 0, 0, 0 };
	struct DOMAIN_INFO_ENTRY *prDomainInfo;
	/* TODO : 5G band edge */

	if (regd_is_single_sku_en())
		return rlmDomainSendPwrLimitCmd_V2(prAdapter);

	prDomainInfo = rlmDomainGetDomainInfo(prAdapter);
	if (prDomainInfo) {
		bandedgeParam[0] = prDomainInfo->rSubBand[0].ucFirstChannelNum;
		bandedgeParam[1] = bandedgeParam[0] +
			prDomainInfo->rSubBand[0].ucNumChannels - 1;
	}

	u4SetCmdTableMaxSize =
	    sizeof(struct CMD_SET_COUNTRY_CHANNEL_POWER_LIMIT) +
	    MAX_CMD_SUPPORT_CHANNEL_NUM *
	    sizeof(struct CMD_CHANNEL_POWER_LIMIT);

	prCmd = cnmMemAlloc(prAdapter, RAM_TYPE_BUF, u4SetCmdTableMaxSize);
	if (!prCmd) {
		DBGLOG(RLM, ERROR, "Domain: Alloc cmd buffer failed\n");
		return;
	}
	kalMemZero(prCmd, u4SetCmdTableMaxSize);

	u2DefaultTableIndex =
	    rlmDomainPwrLimitDefaultTableDecision(prAdapter,
		prAdapter->rWifiVar.u2CountryCode);

	if (u2DefaultTableIndex != POWER_LIMIT_TABLE_NULL) {

		WLAN_GET_FIELD_BE16(&g_rRlmPowerLimitDefault
				    [u2DefaultTableIndex]
				    .aucCountryCode[0],
				    &prCmd->u2CountryCode);

		/* Initialize channel number */
		prCmd->ucNum = 0;

		if (prCmd->u2CountryCode != COUNTRY_CODE_NULL) {
			/*<1>Command - default table information,
			 *fill all subband
			 */
			rlmDomainBuildCmdByDefaultTable(prCmd,
				u2DefaultTableIndex);

			/*<2>Command - configuration table information,
			 * replace specified channel
			 */
			rlmDomainBuildCmdByConfigTable(prAdapter, prCmd);
		}
	}

	DBGLOG(RLM, INFO,
	       "Domain: ValidCC=%c%c, PwrLimitCC=%c%c, PwrLimitChNum=%d\n",
	       (prAdapter->rWifiVar.u2CountryCode & 0xff00) >> 8,
	       (prAdapter->rWifiVar.u2CountryCode & 0x00ff),
	       ((prCmd->u2CountryCode & 0xff00) >> 8),
	       (prCmd->u2CountryCode & 0x00ff),
	       prCmd->ucNum);

	prCmdPwrLimit = &prCmd->rChannelPowerLimit[0];

	for (i = 0; i < prCmd->ucNum; i++) {
		DBGLOG(RLM, TRACE,
			"Old Domain: Ch=%d,Limit=%d,%d,%d,%d,%d,%d,%d,%d,%d,Fg=%d\n",
			prCmdPwrLimit->ucCentralCh,
			prCmdPwrLimit->cPwrLimitCCK,
			prCmdPwrLimit->cPwrLimit20L,
			prCmdPwrLimit->cPwrLimit20H,
			prCmdPwrLimit->cPwrLimit40L,
			prCmdPwrLimit->cPwrLimit40H,
			prCmdPwrLimit->cPwrLimit80L,
			prCmdPwrLimit->cPwrLimit80H,
			prCmdPwrLimit->cPwrLimit160L,
			prCmdPwrLimit->cPwrLimit160H,
			prCmdPwrLimit->ucFlag);
		prCmdPwrLimit++;
	}
#if CFG_SUPPORT_DYNAMIC_PWR_LIMIT
	/* apply each setting into country channel power table */
	txPwrCtrlApplySettings(prAdapter, prCmd, bandedgeParam);
#endif
	/* show tx power table after applying setting */
	prCmdPwrLimit = &prCmd->rChannelPowerLimit[0];
	for (i = 0; i < prCmd->ucNum; i++) {
		DBGLOG(RLM, TRACE,
		       "New Domain: Idx=%d,Ch=%d,Limit=%d,%d,%d,%d,%d,%d,%d,%d,%d,Fg=%d\n",
		       i, prCmdPwrLimit->ucCentralCh,
		       prCmdPwrLimit->cPwrLimitCCK,
		       prCmdPwrLimit->cPwrLimit20L,
		       prCmdPwrLimit->cPwrLimit20H,
		       prCmdPwrLimit->cPwrLimit40L,
		       prCmdPwrLimit->cPwrLimit40H,
		       prCmdPwrLimit->cPwrLimit80L,
		       prCmdPwrLimit->cPwrLimit80H,
		       prCmdPwrLimit->cPwrLimit160L,
		       prCmdPwrLimit->cPwrLimit160H,
		       prCmdPwrLimit->ucFlag);

		prCmdPwrLimit++;
	}

	u4SetQueryInfoLen =
		(sizeof(struct CMD_SET_COUNTRY_CHANNEL_POWER_LIMIT) +
		(prCmd->ucNum) * sizeof(struct CMD_CHANNEL_POWER_LIMIT));

	/* Update domain info to chip */
	if (prCmd->ucNum <= MAX_CMD_SUPPORT_CHANNEL_NUM) {
		rStatus = wlanSendSetQueryCmd(prAdapter,	/* prAdapter */
				CMD_ID_SET_COUNTRY_POWER_LIMIT,	/* ucCID */
				TRUE,	/* fgSetQuery */
				FALSE,	/* fgNeedResp */
				FALSE,	/* fgIsOid */
				NULL,	/* pfCmdDoneHandler */
				NULL,	/* pfCmdTimeoutHandler */
				u4SetQueryInfoLen,	/* u4SetQueryInfoLen */
				(uint8_t *) prCmd,	/* pucInfoBuffer */
				NULL,	/* pvSetQueryBuffer */
				0	/* u4SetQueryBufferLen */
		    );
	} else {
		DBGLOG(RLM, ERROR, "Domain: illegal power limit table\n");
	}

	/* ASSERT(rStatus == WLAN_STATUS_PENDING); */

	cnmMemFree(prAdapter, prCmd);

}
#endif
u_int8_t regd_is_single_sku_en(void)
{
#if (CFG_SUPPORT_SINGLE_SKU == 1)
	return g_mtk_regd_control.en;
#else
	return FALSE;
#endif
}

#if (CFG_SUPPORT_SINGLE_SKU == 1)
enum regd_state rlmDomainGetCtrlState(void)
{
	return g_mtk_regd_control.state;
}


void rlmDomainResetActiveChannel(void)
{
	g_mtk_regd_control.n_channel_active_2g = 0;
	g_mtk_regd_control.n_channel_active_5g = 0;
#if (CFG_SUPPORT_WIFI_6G == 1)
	g_mtk_regd_control.n_channel_active_6g = 0;
#endif
}

void rlmDomainAddActiveChannel(u8 band)

{
	if (band == KAL_BAND_2GHZ)
		g_mtk_regd_control.n_channel_active_2g += 1;
	else if (band == KAL_BAND_5GHZ)
		g_mtk_regd_control.n_channel_active_5g += 1;
#if (CFG_SUPPORT_WIFI_6G == 1)
	else if (band == KAL_BAND_6GHZ)
		g_mtk_regd_control.n_channel_active_6g += 1;
#endif
}

u8 rlmDomainGetActiveChannelCount(u8 band)
{
	if (band == KAL_BAND_2GHZ)
		return g_mtk_regd_control.n_channel_active_2g;
	else if (band == KAL_BAND_5GHZ)
		return g_mtk_regd_control.n_channel_active_5g;
#if (CFG_SUPPORT_WIFI_6G == 1)
	else if (band == KAL_BAND_6GHZ)
		return g_mtk_regd_control.n_channel_active_6g;
#endif
	else
		return 0;
}

struct CMD_DOMAIN_CHANNEL *rlmDomainGetActiveChannels(void)
{
	return g_mtk_regd_control.channels;
}

void rlmDomainSetDefaultCountryCode(void)
{
	g_mtk_regd_control.alpha2 = COUNTRY_CODE_WW;
}

void rlmDomainResetCtrlInfo(u_int8_t force)
{
	if ((g_mtk_regd_control.state == REGD_STATE_UNDEFINED) ||
	    (force == TRUE)) {
		memset(&g_mtk_regd_control, 0, sizeof(struct mtk_regd_control));

		g_mtk_regd_control.state = REGD_STATE_INIT;

		rlmDomainSetDefaultCountryCode();
	}
}

u_int8_t rlmDomainIsUsingLocalRegDomainDataBase(void)
{
#if (CFG_SUPPORT_SINGLE_SKU_LOCAL_DB == 1)
	return TRUE;
#else
	return FALSE;
#endif
}

bool rlmDomainIsSameCountryCode(char *alpha2, u8 size_of_alpha2)
{
	u8 idx;
	u32 alpha2_hex = 0;

	for (idx = 0; idx < size_of_alpha2; idx++)
		alpha2_hex |= (alpha2[idx] << (idx * 8));

	return (rlmDomainGetCountryCode() == alpha2_hex) ? TRUE : FALSE;
}

void rlmDomainSetCountryCode(char *alpha2, u8 size_of_alpha2)
{
	u8 max;
	u8 buf_size;

	buf_size = sizeof(g_mtk_regd_control.alpha2);
	max = (buf_size < size_of_alpha2) ? buf_size : size_of_alpha2;

	g_mtk_regd_control.alpha2 = rlmDomainAlpha2ToU32(alpha2, max);
}
void rlmDomainSetDfsRegion(enum nl80211_dfs_regions dfs_region)
{
	g_mtk_regd_control.dfs_region = dfs_region;
}

enum nl80211_dfs_regions rlmDomainGetDfsRegion(void)
{
	return g_mtk_regd_control.dfs_region;
}

/**
 * rlmDomainChannelFlagString - Transform channel flags to readable string
 *
 * @ flags: the ieee80211_channel->flags for a channel
 * @ buf: string buffer to put the transformed string
 * @ buf_size: size of the buf
 **/
void rlmDomainChannelFlagString(u32 flags, char *buf, size_t buf_size)
{
	int32_t buf_written = 0;

	if (!flags || !buf || !buf_size)
		return;

	if (flags & IEEE80211_CHAN_DISABLED) {
		LOGBUF(buf, ((int32_t)buf_size), buf_written, "DISABLED ");
		/* If DISABLED, don't need to check other flags */
		return;
	}
	if (flags & IEEE80211_CHAN_PASSIVE_FLAG)
		LOGBUF(buf, ((int32_t)buf_size), buf_written,
		       IEEE80211_CHAN_PASSIVE_STR " ");
	if (flags & IEEE80211_CHAN_RADAR)
		LOGBUF(buf, ((int32_t)buf_size), buf_written, "RADAR ");
	if (flags & IEEE80211_CHAN_NO_HT40PLUS)
		LOGBUF(buf, ((int32_t)buf_size), buf_written, "NO_HT40PLUS ");
	if (flags & IEEE80211_CHAN_NO_HT40MINUS)
		LOGBUF(buf, ((int32_t)buf_size), buf_written, "NO_HT40MINUS ");
	if (flags & IEEE80211_CHAN_NO_80MHZ)
		LOGBUF(buf, ((int32_t)buf_size), buf_written, "NO_80MHZ ");
	if (flags & IEEE80211_CHAN_NO_160MHZ)
		LOGBUF(buf, ((int32_t)buf_size), buf_written, "NO_160MHZ ");
}

void rlmDomainParsingChannel(IN struct wiphy *pWiphy)
{
	u32 band_idx, ch_idx;
	u32 ch_count;
	struct ieee80211_supported_band *sband;
	struct ieee80211_channel *chan;
	struct CMD_DOMAIN_CHANNEL *pCh;
	char chan_flag_string[64] = {0};
#if (CFG_SUPPORT_REGD_UPDATE_DISCONNECT_ALLOWED == 1)
	struct GLUE_INFO *prGlueInfo;
	bool fgDisconnection = FALSE;
	uint8_t ucChannelNum = 0;
	uint32_t rStatus, u4BufLen;
#endif

	if (!pWiphy) {
		DBGLOG(RLM, ERROR, "%s():  ERROR. pWiphy = NULL.\n", __func__);
		ASSERT(0);
		return;
	}

#if (CFG_SUPPORT_REGD_UPDATE_DISCONNECT_ALLOWED == 1)
	/* Retrieve connected channel */
	prGlueInfo = rlmDomainGetGlueInfo();
	if (prGlueInfo && kalGetMediaStateIndicated(prGlueInfo) ==
	    PARAM_MEDIA_STATE_CONNECTED) {
		ucChannelNum =
			wlanGetChannelNumberByNetwork(prGlueInfo->prAdapter,
			   prGlueInfo->prAdapter->prAisBssInfo->ucBssIndex);
	}
#endif
	/*
	 * Ready to parse the channel for bands
	 */

	rlmDomainResetActiveChannel();

	ch_count = 0;
	for (band_idx = 0; band_idx < KAL_NUM_BANDS; band_idx++) {
		sband = pWiphy->bands[band_idx];
		if (!sband)
			continue;

		for (ch_idx = 0; ch_idx < sband->n_channels; ch_idx++) {
			chan = &sband->channels[ch_idx];
			pCh = (rlmDomainGetActiveChannels() + ch_count);
			/* Parse flags and get readable string */
			rlmDomainChannelFlagString(chan->flags,
						   chan_flag_string,
						   sizeof(chan_flag_string));

			if (chan->flags & IEEE80211_CHAN_DISABLED) {
				DBGLOG(RLM, INFO,
				       "channels[%d][%d]: ch%d (freq = %d) flags=0x%x [ %s]\n",
				    band_idx, ch_idx, chan->hw_value,
				    chan->center_freq, chan->flags,
				    chan_flag_string);
#if (CFG_SUPPORT_REGD_UPDATE_DISCONNECT_ALLOWED == 1)
				/* Disconnect AP in the end of this function*/
				if (chan->hw_value == ucChannelNum)
					fgDisconnection = TRUE;
#endif
				continue;
			}

			/* Allowable channel */
			if (ch_count == MAXIMUM_OPERATION_CHANNEL_LIST) {
				DBGLOG(RLM, ERROR,
				       "%s(): no buffer to store channel information.\n",
				       __func__);
				break;
			}
                  
			rlmDomainAddActiveChannel(band_idx);

			DBGLOG(RLM, INFO,
			       "channels[%d][%d]: ch%d (freq = %d) flgs=0x%x [%s]\n",
				band_idx, ch_idx, chan->hw_value,
				chan->center_freq, chan->flags,
				chan_flag_string);

			pCh->u2ChNum = chan->hw_value;
			pCh->eFlags = chan->flags;

			ch_count += 1;
		}

	}
#if (CFG_SUPPORT_REGD_UPDATE_DISCONNECT_ALLOWED == 1)
	/* Disconnect with AP if connected channel is disabled in new country */
	if (fgDisconnection) {
		DBGLOG(RLM, STATE, "%s(): Disconnect! CH%d is DISABLED\n",
		    __func__, ucChannelNum);
		rStatus = kalIoctl(prGlueInfo, wlanoidSetDisassociate,
				   NULL, 0, FALSE, FALSE, TRUE, &u4BufLen);

		if (rStatus != WLAN_STATUS_SUCCESS)
			DBGLOG(RLM, WARN, "disassociate error:%lx\n", rStatus);
	}
#endif
}
void rlmExtractChannelInfo(u32 max_ch_count,
			   struct CMD_DOMAIN_ACTIVE_CHANNEL_LIST *prBuff)
{
	u32 ch_count, idx;
	struct CMD_DOMAIN_CHANNEL *pCh;

	prBuff->u1ActiveChNum2g = rlmDomainGetActiveChannelCount(KAL_BAND_2GHZ);
	prBuff->u1ActiveChNum5g = rlmDomainGetActiveChannelCount(KAL_BAND_5GHZ);
#if (CFG_SUPPORT_WIFI_6G == 1)
	prBuff->u1ActiveChNum6g = rlmDomainGetActiveChannelCount(KAL_BAND_6GHZ);

	ch_count = prBuff->u1ActiveChNum2g + prBuff->u1ActiveChNum5g +
		prBuff->u1ActiveChNum6g;
#else
	ch_count = prBuff->u1ActiveChNum2g + prBuff->u1ActiveChNum5g;
#endif
	if (ch_count > max_ch_count) {
		ch_count = max_ch_count;
		DBGLOG(RLM, WARN,
		       "%s(); active channel list is not a complete one.\n",
		       __func__);
	}

	for (idx = 0; idx < ch_count; idx++) {
		pCh = &(prBuff->arChannels[idx]);

		pCh->u2ChNum = (rlmDomainGetActiveChannels() + idx)->u2ChNum;
		pCh->eFlags = (rlmDomainGetActiveChannels() + idx)->eFlags;
	}

}

const struct ieee80211_regdomain
*rlmDomainSearchRegdomainFromLocalDataBase(char *alpha2)
{
#if (CFG_SUPPORT_SINGLE_SKU_LOCAL_DB == 1)
	u8 idx;
	const struct mtk_regdomain *prRegd;

	idx = 0;
	while (g_prRegRuleTable[idx]) {
		prRegd = g_prRegRuleTable[idx];

		if ((prRegd->country_code[0] == alpha2[0]) &&
			(prRegd->country_code[1] == alpha2[1]) &&
			(prRegd->country_code[2] == alpha2[2]) &&
			(prRegd->country_code[3] == alpha2[3]))
			return prRegd->prRegdRules;

		idx++;
	}

	return NULL; /*default world wide*/
#else
	return NULL;
#endif
}


const struct ieee80211_regdomain *rlmDomainGetLocalDefaultRegd(void)
{
#if (CFG_SUPPORT_SINGLE_SKU_LOCAL_DB == 1)
	return &default_regdom_ww;
#else
	return NULL;
#endif
}
struct GLUE_INFO *rlmDomainGetGlueInfo(void)
{
	return g_mtk_regd_control.pGlueInfo;
}

bool rlmDomainIsEfuseUsed(void)
{
	return g_mtk_regd_control.isEfuseCountryCodeUsed;
}

uint8_t rlmDomainGetChannelBw(uint8_t channelNum)
{
	uint32_t ch_idx = 0, start_idx = 0, end_idx = 0;
	uint8_t channelBw = MAX_BW_80_80_MHZ;
	struct CMD_DOMAIN_CHANNEL *pCh;

#if (CFG_SUPPORT_WIFI_6G == 1)
	end_idx = rlmDomainGetActiveChannelCount(KAL_BAND_2GHZ)
			+ rlmDomainGetActiveChannelCount(KAL_BAND_5GHZ)
			+ rlmDomainGetActiveChannelCount(KAL_BAND_6GHZ);
#else
	end_idx = rlmDomainGetActiveChannelCount(KAL_BAND_2GHZ)
			+ rlmDomainGetActiveChannelCount(KAL_BAND_5GHZ);
#endif

	for (ch_idx = start_idx; ch_idx < end_idx; ch_idx++) {
		pCh = (rlmDomainGetActiveChannels() + ch_idx);

		if (pCh->u2ChNum != channelNum)
			continue;

		/* Max BW */
		if ((pCh->eFlags & IEEE80211_CHAN_NO_160MHZ)
						== IEEE80211_CHAN_NO_160MHZ)
			channelBw = MAX_BW_80MHZ;
		if ((pCh->eFlags & IEEE80211_CHAN_NO_80MHZ)
						== IEEE80211_CHAN_NO_80MHZ)
			channelBw = MAX_BW_40MHZ;
		if ((pCh->eFlags & IEEE80211_CHAN_NO_HT40)
						== IEEE80211_CHAN_NO_HT40)
			channelBw = MAX_BW_20MHZ;
	}

	DBGLOG(RLM, INFO, "ch=%d, BW=%d\n", channelNum, channelBw);
	return channelBw;
}
#endif

uint32_t rlmDomainExtractSingleSkuInfoFromFirmware(IN struct ADAPTER *prAdapter,
						   IN uint8_t *pucEventBuf)
{
#if (CFG_SUPPORT_SINGLE_SKU == 1)
	struct SINGLE_SKU_INFO *prSkuInfo =
			(struct SINGLE_SKU_INFO *) pucEventBuf;

	g_mtk_regd_control.en = TRUE;

	if (prSkuInfo->isEfuseValid) {
		if (!rlmDomainIsUsingLocalRegDomainDataBase()) {

			DBGLOG(RLM, ERROR,
				"Error. In efuse mode, must use local data base.\n");

			ASSERT(0);
			/* force using local db if getting
			 * country code from efuse
			 */
			return WLAN_STATUS_NOT_SUPPORTED;
		}

		rlmDomainSetCountryCode(
			(char *) &prSkuInfo->u4EfuseCountryCode,
			sizeof(prSkuInfo->u4EfuseCountryCode));

		g_mtk_regd_control.isEfuseCountryCodeUsed = TRUE;
	}
#endif

	return WLAN_STATUS_SUCCESS;
}

void rlmDomainSendInfoToFirmware(IN struct ADAPTER *prAdapter)
{
#if (CFG_SUPPORT_SINGLE_SKU == 1)
	if (!regd_is_single_sku_en())
		return; /*not support single sku*/

	g_mtk_regd_control.pGlueInfo = prAdapter->prGlueInfo;
	rlmDomainSetCountry(prAdapter);
#endif
}

enum ENUM_CHNL_EXT rlmSelectSecondaryChannelType(struct ADAPTER *prAdapter,
						 enum ENUM_BAND band,
						 u8 primary_ch)
{
	enum ENUM_CHNL_EXT eSCO = CHNL_EXT_SCN;
#if (CFG_SUPPORT_SINGLE_SKU == 1)
	if (band == BAND_5G) {
		switch ((uint32_t)primary_ch) {
		case 36:
		case 44:
		case 52:
		case 60:
		case 100:
		case 108:
		case 116:
		case 124:
		case 132:
		case 140:
		case 149:
		case 157:
			eSCO = CHNL_EXT_SCA;
			break;
		case 40:
		case 48:
		case 56:
		case 64:
		case 104:
		case 112:
		case 120:
		case 128:
		case 136:
		case 144:
		case 153:
		case 161:
			eSCO = CHNL_EXT_SCB;
			break;
		case 165:
		default:
			eSCO = CHNL_EXT_SCN;
			break;
		}
	} else {
		u8 below_ch, above_ch;

		below_ch = primary_ch - CHNL_SPAN_20;
		above_ch = primary_ch + CHNL_SPAN_20;

		if (rlmDomainIsLegalChannel(prAdapter, band, above_ch))
			return CHNL_EXT_SCA;

		if (rlmDomainIsLegalChannel(prAdapter, band, below_ch))
			return CHNL_EXT_SCB;
	}

#endif
	return eSCO;
}

void rlmDomainOidSetCountry(IN struct ADAPTER *prAdapter, char *country,
			    u8 size_of_country)
{
#if (CFG_SUPPORT_SINGLE_SKU == 1)

	if (rlmDomainIsUsingLocalRegDomainDataBase()) {

		if (rlmDomainIsSameCountryCode(country, size_of_country)) {
			char acCountryCodeStr[MAX_COUNTRY_CODE_LEN + 1] = {0};

			rlmDomainU32ToAlpha(
				rlmDomainGetCountryCode(), acCountryCodeStr);
			DBGLOG(RLM, WARN,
				"Same as current country %s, skip!\n",
				acCountryCodeStr);
			return;
		}
		rlmDomainSetCountryCode(country, size_of_country);
		rlmDomainSetCountry(prAdapter);
	} else {
		DBGLOG(RLM, INFO,
		       "%s(): Using driver hint to query CRDA getting regd.\n",
		       __func__);
		regulatory_hint(priv_to_wiphy(prAdapter->prGlueInfo), country);
	}
#endif
}

u32 rlmDomainGetCountryCode(void)
{
#if (CFG_SUPPORT_SINGLE_SKU == 1)
	return g_mtk_regd_control.alpha2;
#else
	return 0;
#endif
}

void rlmDomainAssert(u_int8_t cond)
{
	/* bypass this check because single sku is not enable */
	if (!regd_is_single_sku_en())
		return;

	if (!cond) {
		WARN_ON(1);
		DBGLOG(RLM, ERROR, "[WARNING!!] RLM unexpected case.\n");
	}

}

void rlmDomainU32ToAlpha(u_int32_t u4CountryCode, char *pcAlpha)
{
	u_int8_t ucIdx;

	for (ucIdx = 0; ucIdx < MAX_COUNTRY_CODE_LEN; ucIdx++)
		pcAlpha[ucIdx] = ((u4CountryCode >> (ucIdx * 8)) & 0xff);
}

#if (CFG_SUPPORT_SINGLE_SKU == 1)
void rlm_get_alpha2(char *alpha2)
{
	rlmDomainU32ToAlpha(g_mtk_regd_control.alpha2, alpha2);
}
EXPORT_SYMBOL(rlm_get_alpha2);
#endif
