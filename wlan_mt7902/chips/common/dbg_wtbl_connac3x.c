/* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause */
/*
 * Copyright (c) 2019 MediaTek Inc.
 */

#if (CFG_SUPPORT_CONNAC3X == 1)
/*******************************************************************************
 *                         C O M P I L E R   F L A G S
 *******************************************************************************
 */

/*******************************************************************************
 *                    E X T E R N A L   R E F E R E N C E S
 *******************************************************************************
 */
#include "precomp.h"
#include "dbg_wtbl_connac3x.h"
#ifdef BELLWETHER
#include "coda/bellwether/wf_wtblon_top.h"
#include "coda/bellwether/wf_uwtbl_top.h"
#include "coda/bellwether/wf_ds_uwtbl.h"
#include "coda/bellwether/wf_ds_lwtbl.h"
#endif
#ifdef MT6639
#include "coda/mt6639/wf_wtblon_top.h"
#include "coda/mt6639/wf_uwtbl_top.h"
#include "coda/mt6639/wf_ds_uwtbl.h"
#include "coda/mt6639/wf_ds_lwtbl.h"
#endif

/*******************************************************************************
 *                              C O N S T A N T S
 *******************************************************************************
 */

/*******************************************************************************
 *                             D A T A   T Y P E S
 *******************************************************************************
 */

/*******************************************************************************
 *                            P U B L I C   D A T A
 *******************************************************************************
 */

/*******************************************************************************
 *                           P R I V A T E   D A T A
 *******************************************************************************
 */
uint32_t halWtblWriteRaw(
	struct ADAPTER *prAdapter,
	uint16_t  u2EntryIdx,
	enum _ENUM_WTBL_TYPE_T eType,
	uint16_t u2DW,
	uint32_t u4Value)
{
	uint32_t u4WtblVmAddr = 0;

	if (eType == WTBL_TYPE_LMAC) {
		HAL_MCR_WR(prAdapter, WF_WTBLON_TOP_WDUCR_ADDR,
			((u2EntryIdx >> 7) & WF_WTBLON_TOP_WDUCR_GROUP_MASK) <<
			WF_WTBLON_TOP_WDUCR_GROUP_SHFT);
		u4WtblVmAddr = LWTBL_IDX2BASE(u2EntryIdx, u2DW);
	} else if (eType == WTBL_TYPE_UMAC) {
		HAL_MCR_WR(prAdapter, WF_UWTBL_TOP_WDUCR_ADDR,
			((u2EntryIdx >> 7) & WF_UWTBL_TOP_WDUCR_GROUP_MASK) <<
			WF_UWTBL_TOP_WDUCR_GROUP_SHFT);
		u4WtblVmAddr = UWTBL_IDX2BASE(u2EntryIdx, u2DW);
	} else if (eType == WTBL_TYPE_KEY) {
		HAL_MCR_WR(prAdapter, WF_UWTBL_TOP_WDUCR_ADDR,
			(WF_UWTBL_TOP_WDUCR_TARGET_MASK |
			 (((u2EntryIdx >> 7) & WF_UWTBL_TOP_WDUCR_GROUP_MASK)
			  << WF_UWTBL_TOP_WDUCR_GROUP_SHFT)));
		u4WtblVmAddr = KEYTBL_IDX2BASE(u2EntryIdx, u2DW);
	} else {
		/*TODO:*/
	}

	HAL_MCR_WR(prAdapter, u4WtblVmAddr, u4Value);

	return 0;
}

uint32_t halWtblReadRaw(
	struct ADAPTER *prAdapter,
	uint16_t  u2EntryIdx,
	enum _ENUM_WTBL_TYPE_T eType,
	uint16_t u2StartDW,
	uint16_t u2LenInDW,
	void *pBuffer)
{
	uint32_t *dest_cpy = (uint32_t *)pBuffer;
	uint32_t sizeInDW = u2LenInDW;
	uint32_t u4SrcAddr = 0;

	if (pBuffer == NULL)
		return 0xFF;

	if (eType == WTBL_TYPE_LMAC) {
		HAL_MCR_WR(prAdapter, WF_WTBLON_TOP_WDUCR_ADDR,
			((u2EntryIdx >> 7) & WF_WTBLON_TOP_WDUCR_GROUP_MASK) <<
			WF_WTBLON_TOP_WDUCR_GROUP_SHFT);
		u4SrcAddr = LWTBL_IDX2BASE(u2EntryIdx, u2StartDW);
	} else if (eType == WTBL_TYPE_UMAC) {
		HAL_MCR_WR(prAdapter, WF_UWTBL_TOP_WDUCR_ADDR,
			((u2EntryIdx >> 7) & WF_UWTBL_TOP_WDUCR_GROUP_MASK) <<
			WF_UWTBL_TOP_WDUCR_GROUP_SHFT);
		u4SrcAddr = UWTBL_IDX2BASE(u2EntryIdx, u2StartDW);
	} else if (eType == WTBL_TYPE_KEY) {
		HAL_MCR_WR(prAdapter, WF_UWTBL_TOP_WDUCR_ADDR,
			(WF_UWTBL_TOP_WDUCR_TARGET_MASK |
			 (((u2EntryIdx >> 7) & WF_UWTBL_TOP_WDUCR_GROUP_MASK)
			  << WF_UWTBL_TOP_WDUCR_GROUP_SHFT)));
		u4SrcAddr = KEYTBL_IDX2BASE(u2EntryIdx, u2StartDW);
	} else {
		/* TODO: */
	}

	while (sizeInDW--) {
		uint32_t u4Value = 0;

		HAL_MCR_RD(prAdapter, u4SrcAddr, &u4Value);
		*dest_cpy++ = u4Value;
		u4SrcAddr += 4;
	}

	return 0;
}

static struct {
	uint8_t *name;
	uint32_t mask;
	uint32_t shift;
	u_int8_t new_line;
} WTBL_LMAC_DW0[] = {
	{"MUAR_IDX",    WF_LWTBL_MUAR_MASK, WF_LWTBL_MUAR_SHIFT,	FALSE},
	{"RCA1",        WF_LWTBL_RCA1_MASK, NO_SHIFT_DEFINE,	FALSE},
	{"KID",         WF_LWTBL_KID_MASK,  WF_LWTBL_KID_SHIFT,	FALSE},
	{"RCID",        WF_LWTBL_RCID_MASK, NO_SHIFT_DEFINE,	FALSE},
	{"BAND",        WF_LWTBL_BAND_MASK, WF_LWTBL_BAND_SHIFT,	FALSE},
	{"RV",          WF_LWTBL_RV_MASK,   NO_SHIFT_DEFINE,	FALSE},
	{"RCA2",        WF_LWTBL_RCA2_MASK, NO_SHIFT_DEFINE,	FALSE},
	{"WPI_FLAG",    WF_LWTBL_WPI_FLAG_MASK, NO_SHIFT_DEFINE,	TRUE},
	{NULL,}
};

static void parse_bmac_lwtbl_DW0_1(struct ADAPTER *prAdapter, uint8_t *lwtbl)
{
	uint32_t *addr = 0;
	uint32_t dw_value = 0;
	uint16_t i = 0;

	DBGLOG(HAL, INFO, "\t\n");
	DBGLOG(HAL, INFO,
	       "LinkAddr: %02x:%02x:%02x:%02x:%02x:%02x(D0[B0~15], D1[B0~31])\n",
		lwtbl[4], lwtbl[5], lwtbl[6], lwtbl[7], lwtbl[0], lwtbl[1]);

	/* LMAC WTBL DW 0 */
	DBGLOG(HAL, INFO, "\t\n");
	DBGLOG(HAL, INFO, "LWTBL DW 0/1\n");
	addr = (uint32_t *)&(lwtbl[WTBL_GROUP_PEER_INFO_DW_0*4]);
	dw_value = *addr;

	while (WTBL_LMAC_DW0[i].name) {

		if (WTBL_LMAC_DW0[i].shift == NO_SHIFT_DEFINE)
			DBGLOG(HAL, INFO, "\t%s:%d\n", WTBL_LMAC_DW0[i].name,
				(dw_value & WTBL_LMAC_DW0[i].mask) ? 1 : 0);
		else
			DBGLOG(HAL, INFO, "\t%s:%u\n", WTBL_LMAC_DW0[i].name,
				(dw_value & WTBL_LMAC_DW0[i].mask) >>
				WTBL_LMAC_DW0[i].shift);
		i++;
	}
}

static struct {
	uint8_t *name;
	uint32_t mask;
	uint32_t shift;
	u_int8_t new_line;
} WTBL_LMAC_DW2[] = {
	{"AID",           WF_LWTBL_AID_MASK,              WF_LWTBL_AID_SHIFT,
		FALSE},
	{"GID_SU",        WF_LWTBL_GID_SU_MASK,           NO_SHIFT_DEFINE,
		FALSE},
	{"SPP_EN",        WF_LWTBL_SPP_EN_MASK,           NO_SHIFT_DEFINE,
		FALSE},
	{"WPI_EVEN",      WF_LWTBL_WPI_EVEN_MASK,         NO_SHIFT_DEFINE,
		FALSE},
	{"AAD_OM",        WF_LWTBL_AAD_OM_MASK,           NO_SHIFT_DEFINE,
		FALSE},
	{"CIPHER_PGTK",   WF_LWTBL_CIPHER_SUIT_PGTK_MASK,
		WF_LWTBL_CIPHER_SUIT_PGTK_SHIFT, TRUE},
	{"FROM_DS",       WF_LWTBL_FD_MASK,               NO_SHIFT_DEFINE,
		FALSE},
	{"TO_DS",         WF_LWTBL_TD_MASK,               NO_SHIFT_DEFINE,
		FALSE},
	{"SW",            WF_LWTBL_SW_MASK,               NO_SHIFT_DEFINE,
		FALSE},
	{"UL",            WF_LWTBL_UL_MASK,               NO_SHIFT_DEFINE,
		FALSE},
	{"TX_POWER_SAVE", WF_LWTBL_TX_PS_MASK,            NO_SHIFT_DEFINE,
		TRUE},
	{"QOS",           WF_LWTBL_QOS_MASK,              NO_SHIFT_DEFINE,
		FALSE},
	{"HT",            WF_LWTBL_HT_MASK,               NO_SHIFT_DEFINE,
		FALSE},
	{"VHT",           WF_LWTBL_VHT_MASK,              NO_SHIFT_DEFINE,
		FALSE},
	{"HE",            WF_LWTBL_HE_MASK,               NO_SHIFT_DEFINE,
		FALSE},
	{"EHT",           WF_LWTBL_EHT_MASK,              NO_SHIFT_DEFINE,
		FALSE},
	{"MESH",          WF_LWTBL_MESH_MASK,             NO_SHIFT_DEFINE,
		TRUE},
	{NULL,}
};

static void parse_bmac_lwtbl_DW2(struct ADAPTER *prAdapter, uint8_t *lwtbl)
{
	uint32_t *addr = 0;
	uint32_t dw_value = 0;
	uint16_t i = 0;

	/* LMAC WTBL DW 2 */
	DBGLOG(HAL, INFO, "\t\n");
	DBGLOG(HAL, INFO, "LWTBL DW 2\n");
	addr = (uint32_t *)&(lwtbl[WTBL_GROUP_TRX_CAP_DW_2*4]);
	dw_value = *addr;

	while (WTBL_LMAC_DW2[i].name) {

		if (WTBL_LMAC_DW2[i].shift == NO_SHIFT_DEFINE)
			DBGLOG(HAL, INFO, "\t%s:%d\n", WTBL_LMAC_DW2[i].name,
				(dw_value & WTBL_LMAC_DW2[i].mask) ? 1 : 0);
		else
			DBGLOG(HAL, INFO, "\t%s:%u\n", WTBL_LMAC_DW2[i].name,
				(dw_value & WTBL_LMAC_DW2[i].mask) >>
				WTBL_LMAC_DW2[i].shift);
		i++;
	}
}

static struct {
	uint8_t *name;
	uint32_t mask;
	uint32_t shift;
	u_int8_t new_line;
} WTBL_LMAC_DW3[] = {
	{"WMM_Q",	WF_LWTBL_WMM_Q_MASK,	WF_LWTBL_WMM_Q_SHIFT,	FALSE},
	{"EHT_SIG_MCS",	WF_LWTBL_EHT_SIG_MCS_MASK,
		WF_LWTBL_EHT_SIG_MCS_SHIFT,	FALSE},
	{"HDRT_MODE",	WF_LWTBL_HDRT_MODE_MASK,	NO_SHIFT_DEFINE,
		FALSE},
	{"BEAM_CHG",	WF_LWTBL_BEAM_CHG_MASK,	NO_SHIFT_DEFINE,	FALSE},
	{"EHT_LTF_SYM_NUM",	WF_LWTBL_EHT_LTF_SYM_NUM_OPT_MASK,
		WF_LWTBL_EHT_LTF_SYM_NUM_OPT_SHIFT,	TRUE},
	{"PFMU_IDX",	WF_LWTBL_PFMU_IDX_MASK,	WF_LWTBL_PFMU_IDX_SHIFT,
		FALSE},
	{"ULPF_IDX",	WF_LWTBL_ULPF_IDX_MASK,	WF_LWTBL_ULPF_IDX_SHIFT,
		FALSE},
	{"RIBF",	WF_LWTBL_RIBF_MASK,	NO_SHIFT_DEFINE,	FALSE},
	{"ULPF",	WF_LWTBL_ULPF_MASK,	NO_SHIFT_DEFINE,	TRUE},
	{"TBF_HT",	WF_LWTBL_TBF_HT_MASK,	NO_SHIFT_DEFINE,	FALSE},
	{"TBF_VHT",	WF_LWTBL_TBF_VHT_MASK,	NO_SHIFT_DEFINE,	FALSE},
	{"TBF_HE",	WF_LWTBL_TBF_HE_MASK,	NO_SHIFT_DEFINE,	FALSE},
	{"TBF_EHT",	WF_LWTBL_TBF_EHT_MASK,	NO_SHIFT_DEFINE,	FALSE},
	{"IGN_FBK",	WF_LWTBL_IGN_FBK_MASK,	NO_SHIFT_DEFINE,	TRUE},
	{NULL,}
};

static void parse_bmac_lwtbl_DW3(struct ADAPTER *prAdapter, uint8_t *lwtbl)
{
	uint32_t *addr = 0;
	uint32_t dw_value = 0;
	uint16_t i = 0;

	/* LMAC WTBL DW 3 */
	DBGLOG(HAL, INFO, "\t\n");
	DBGLOG(HAL, INFO, "LWTBL DW 3\n");
	addr = (uint32_t *)&(lwtbl[WTBL_GROUP_TRX_CAP_DW_3*4]);
	dw_value = *addr;

	while (WTBL_LMAC_DW3[i].name) {

		if (WTBL_LMAC_DW3[i].shift == NO_SHIFT_DEFINE)
			DBGLOG(HAL, INFO, "\t%s:%d\n", WTBL_LMAC_DW3[i].name,
				(dw_value & WTBL_LMAC_DW3[i].mask) ? 1 : 0);
		else
			DBGLOG(HAL, INFO, "\t%s:%u\n", WTBL_LMAC_DW3[i].name,
				(dw_value & WTBL_LMAC_DW3[i].mask) >>
				WTBL_LMAC_DW3[i].shift);
		i++;
	}
}

static struct {
	uint8_t *name;
	uint32_t mask;
	uint32_t shift;
	u_int8_t new_line;
} WTBL_LMAC_DW4[] = {
	{"ANT_ID_STS0",     WF_LWTBL_ANT_ID0_MASK,      WF_LWTBL_ANT_ID0_SHIFT,
		FALSE},
	{"STS1",            WF_LWTBL_ANT_ID1_MASK,      WF_LWTBL_ANT_ID1_SHIFT,
		FALSE},
	{"STS2",            WF_LWTBL_ANT_ID2_MASK,      WF_LWTBL_ANT_ID2_SHIFT,
		FALSE},
	{"STS3",            WF_LWTBL_ANT_ID3_MASK,      WF_LWTBL_ANT_ID3_SHIFT,
		TRUE},
	{"ANT_ID_STS4",     WF_LWTBL_ANT_ID4_MASK,      WF_LWTBL_ANT_ID4_SHIFT,
		FALSE},
	{"STS5",            WF_LWTBL_ANT_ID5_MASK,      WF_LWTBL_ANT_ID5_SHIFT,
		FALSE},
	{"STS6",            WF_LWTBL_ANT_ID6_MASK,      WF_LWTBL_ANT_ID6_SHIFT,
		FALSE},
	{"STS7",            WF_LWTBL_ANT_ID7_MASK,      WF_LWTBL_ANT_ID7_SHIFT,
		TRUE},
	{"PE",              WF_LWTBL_PE_MASK,           WF_LWTBL_PE_SHIFT,
		FALSE},
	{"DIS_RHTR",        WF_LWTBL_DIS_RHTR_MASK,     NO_SHIFT_DEFINE,
		FALSE},
	{"LDPC_HT",         WF_LWTBL_LDPC_HT_MASK,      NO_SHIFT_DEFINE,
		FALSE},
	{"LDPC_VHT",        WF_LWTBL_LDPC_VHT_MASK,     NO_SHIFT_DEFINE,
		FALSE},
	{"LDPC_HE",         WF_LWTBL_LDPC_HE_MASK,      NO_SHIFT_DEFINE,
		FALSE},
	{"LDPC_EHT",        WF_LWTBL_LDPC_EHT_MASK,     NO_SHIFT_DEFINE,
		TRUE},
	{NULL,}
};

static void parse_bmac_lwtbl_DW4(struct ADAPTER *prAdapter, uint8_t *lwtbl)
{
	uint32_t *addr = 0;
	uint32_t dw_value = 0;
	uint16_t i = 0;

	/* LMAC WTBL DW 4 */
	DBGLOG(HAL, INFO, "\t\n");
	DBGLOG(HAL, INFO, "LWTBL DW 4\n");
	addr = (uint32_t *)&(lwtbl[WTBL_GROUP_TRX_CAP_DW_4*4]);
	dw_value = *addr;

	while (WTBL_LMAC_DW4[i].name) {
		if (WTBL_LMAC_DW4[i].shift == NO_SHIFT_DEFINE)
			DBGLOG(HAL, INFO, "\t%s:%d\n", WTBL_LMAC_DW4[i].name,
				(dw_value & WTBL_LMAC_DW4[i].mask) ? 1 : 0);
		else
			DBGLOG(HAL, INFO, "\t%s:%u\n", WTBL_LMAC_DW4[i].name,
				(dw_value & WTBL_LMAC_DW4[i].mask) >>
				WTBL_LMAC_DW4[i].shift);
		i++;
	}
}

static struct {
	uint8_t *name;
	uint32_t mask;
	uint32_t shift;
	u_int8_t new_line;
} WTBL_LMAC_DW5[] = {
	{"AF",                 WF_LWTBL_AF_MASK,           WF_LWTBL_AF_SHIFT,
		FALSE},
	{"AF_HE",              WF_LWTBL_AF_HE_MASK,
		WF_LWTBL_AF_HE_SHIFT, FALSE},
	{"RTS",                WF_LWTBL_RTS_MASK,          NO_SHIFT_DEFINE,
		FALSE},
	{"SMPS",               WF_LWTBL_SMPS_MASK,         NO_SHIFT_DEFINE,
		FALSE},
	{"DYN_BW",             WF_LWTBL_DYN_BW_MASK,       NO_SHIFT_DEFINE,
		TRUE},
	{"MMSS",               WF_LWTBL_MMSS_MASK,         WF_LWTBL_MMSS_SHIFT,
		FALSE},
	{"USR",                WF_LWTBL_USR_MASK,          NO_SHIFT_DEFINE,
		FALSE},
	{"SR_RATE",            WF_LWTBL_SR_R_MASK,         WF_LWTBL_SR_R_SHIFT,
		FALSE},
	{"SR_ABORT",           WF_LWTBL_SR_ABORT_MASK,     NO_SHIFT_DEFINE,
		TRUE},
	{"TX_POWER_OFFSET",    WF_LWTBL_TX_POWER_OFFSET_MASK,
		WF_LWTBL_TX_POWER_OFFSET_SHIFT, FALSE},
	{"LTF_EHT",            WF_LWTBL_LTF_EHT_MASK,
		WF_LWTBL_LTF_EHT_SHIFT, FALSE},
	{"GI_EHT",             WF_LWTBL_GI_EHT_MASK,
		WF_LWTBL_GI_EHT_SHIFT, FALSE},
	{"DOPPL",              WF_LWTBL_DOPPL_MASK,        NO_SHIFT_DEFINE,
		FALSE},
	{"TXOP_PS_CAP",        WF_LWTBL_TXOP_PS_CAP_MASK,  NO_SHIFT_DEFINE,
		FALSE},
	{"DONOT_UPDATE_I_PSM", WF_LWTBL_DU_I_PSM_MASK,     NO_SHIFT_DEFINE,
		TRUE},
	{"I_PSM",              WF_LWTBL_I_PSM_MASK,        NO_SHIFT_DEFINE,
		FALSE},
	{"PSM",                WF_LWTBL_PSM_MASK,          NO_SHIFT_DEFINE,
		FALSE},
	{"SKIP_TX",            WF_LWTBL_SKIP_TX_MASK,      NO_SHIFT_DEFINE,
		TRUE},
	{NULL,}
};

static void parse_bmac_lwtbl_DW5(struct ADAPTER *prAdapter, uint8_t *lwtbl)
{
	uint32_t *addr = 0;
	uint32_t dw_value = 0;
	uint16_t i = 0;

	/* LMAC WTBL DW 5 */
	DBGLOG(HAL, INFO, "\t\n");
	DBGLOG(HAL, INFO, "LWTBL DW 5\n");
	addr = (uint32_t *)&(lwtbl[WTBL_GROUP_TRX_CAP_DW_5*4]);
	dw_value = *addr;

	while (WTBL_LMAC_DW5[i].name) {
		if (WTBL_LMAC_DW5[i].shift == NO_SHIFT_DEFINE)
			DBGLOG(HAL, INFO, "\t%s:%d\n", WTBL_LMAC_DW5[i].name,
				(dw_value & WTBL_LMAC_DW5[i].mask) ? 1 : 0);
		else
			DBGLOG(HAL, INFO, "\t%s:%u\n", WTBL_LMAC_DW5[i].name,
				(dw_value & WTBL_LMAC_DW5[i].mask) >>
				WTBL_LMAC_DW5[i].shift);
		i++;
	}
}

static struct {
	uint8_t *name;
	uint32_t mask;
	uint32_t shift;
	u_int8_t new_line;
} WTBL_LMAC_DW6[] = {
	{"CBRN",        WF_LWTBL_CBRN_MASK,	WF_LWTBL_CBRN_SHIFT,	FALSE},
	{"DBNSS_EN",    WF_LWTBL_DBNSS_EN_MASK, NO_SHIFT_DEFINE,	FALSE},
	{"BAF_EN",      WF_LWTBL_BAF_EN_MASK,   NO_SHIFT_DEFINE,	FALSE},
	{"RDGBA",       WF_LWTBL_RDGBA_MASK,    NO_SHIFT_DEFINE,	FALSE},
	{"RDG",         WF_LWTBL_R_MASK,        NO_SHIFT_DEFINE,	FALSE},
	{"SPE_IDX",     WF_LWTBL_SPE_IDX_MASK,  WF_LWTBL_SPE_IDX_SHIFT,	TRUE},
	{"G2",          WF_LWTBL_G2_MASK,       NO_SHIFT_DEFINE,	FALSE},
	{"G4",          WF_LWTBL_G4_MASK,       NO_SHIFT_DEFINE,	FALSE},
	{"G8",          WF_LWTBL_G8_MASK,       NO_SHIFT_DEFINE,	FALSE},
	{"G16",         WF_LWTBL_G16_MASK,      NO_SHIFT_DEFINE,	TRUE},
	{"G2_LTF",      WF_LWTBL_G2_LTF_MASK,   WF_LWTBL_G2_LTF_SHIFT,	FALSE},
	{"G4_LTF",      WF_LWTBL_G4_LTF_MASK,   WF_LWTBL_G4_LTF_SHIFT,	FALSE},
	{"G8_LTF",      WF_LWTBL_G8_LTF_MASK,   WF_LWTBL_G8_LTF_SHIFT,	FALSE},
	{"G16_LTF",     WF_LWTBL_G16_LTF_MASK,  WF_LWTBL_G16_LTF_SHIFT,	TRUE},
	{"G2_HE",       WF_LWTBL_G2_HE_MASK,    WF_LWTBL_G2_HE_SHIFT,	FALSE},
	{"G4_HE",       WF_LWTBL_G4_HE_MASK,    WF_LWTBL_G4_HE_SHIFT,	FALSE},
	{"G8_HE",       WF_LWTBL_G8_HE_MASK,    WF_LWTBL_G8_HE_SHIFT,	FALSE},
	{"G16_HE",      WF_LWTBL_G16_HE_MASK,   WF_LWTBL_G16_HE_SHIFT,	TRUE},
	{NULL,}
};

static void parse_bmac_lwtbl_DW6(struct ADAPTER *prAdapter, uint8_t *lwtbl)
{
	uint32_t *addr = 0;
	uint32_t dw_value = 0;
	uint16_t i = 0;

	/* LMAC WTBL DW 6 */
	DBGLOG(HAL, INFO, "\t\n");
	DBGLOG(HAL, INFO, "LWTBL DW 6\n");
	addr = (uint32_t *)&(lwtbl[WTBL_GROUP_TRX_CAP_DW_6*4]);
	dw_value = *addr;

	while (WTBL_LMAC_DW6[i].name) {
		if (WTBL_LMAC_DW6[i].shift == NO_SHIFT_DEFINE)
			DBGLOG(HAL, INFO, "\t%s:%d\n", WTBL_LMAC_DW6[i].name,
				(dw_value & WTBL_LMAC_DW6[i].mask) ? 1 : 0);
		else
			DBGLOG(HAL, INFO, "\t%s:%u\n", WTBL_LMAC_DW6[i].name,
				(dw_value & WTBL_LMAC_DW6[i].mask) >>
				WTBL_LMAC_DW6[i].shift);
		i++;
	}
}

static void parse_bmac_lwtbl_DW7(struct ADAPTER *prAdapter, uint8_t *lwtbl)
{
	uint32_t *addr = 0;
	uint32_t dw_value = 0;
	int i = 0;

	/* LMAC WTBL DW 7 */
	DBGLOG(HAL, INFO, "\t\n");
	DBGLOG(HAL, INFO, "LWTBL DW 7\n");
	addr = (uint32_t *)&(lwtbl[WTBL_GROUP_TRX_CAP_DW_7*4]);
	dw_value = *addr;

	for (i = 0; i < 8; i++)
		DBGLOG(HAL, INFO, "\tBA_WIN_SIZE%u:%lu\n",
		       i, ((dw_value & BITS(i*4, i*4+3)) >> i*4));
}

static struct {
	uint8_t *name;
	uint32_t mask;
	uint32_t shift;
	u_int8_t new_line;
} WTBL_LMAC_DW8[] = {
	{"RTS_FAIL_CNT_AC0",    WF_LWTBL_AC0_RTS_FAIL_CNT_MASK,
		WF_LWTBL_AC0_RTS_FAIL_CNT_SHIFT,	FALSE},
	{"AC1",                 WF_LWTBL_AC1_RTS_FAIL_CNT_MASK,
		WF_LWTBL_AC1_RTS_FAIL_CNT_SHIFT,	FALSE},
	{"AC2",                 WF_LWTBL_AC2_RTS_FAIL_CNT_MASK,
		WF_LWTBL_AC2_RTS_FAIL_CNT_SHIFT,	FALSE},
	{"AC3",                 WF_LWTBL_AC3_RTS_FAIL_CNT_MASK,
		WF_LWTBL_AC3_RTS_FAIL_CNT_SHIFT,	TRUE},
	{"PARTIAL_AID",         WF_LWTBL_PARTIAL_AID_MASK,
		WF_LWTBL_PARTIAL_AID_SHIFT,	FALSE},
	{"CHK_PER",             WF_LWTBL_CHK_PER_MASK,
		NO_SHIFT_DEFINE,	TRUE},
	{NULL,}
};

static void parse_bmac_lwtbl_DW8(struct ADAPTER *prAdapter, uint8_t *lwtbl)
{
	uint32_t *addr = 0;
	uint32_t dw_value = 0;
	uint16_t i = 0;

	/* LMAC WTBL DW 8 */
	DBGLOG(HAL, INFO, "\t\n");
	DBGLOG(HAL, INFO, "LWTBL DW 8\n");
	addr = (uint32_t *)&(lwtbl[WTBL_GROUP_TRX_CAP_DW_8*4]);
	dw_value = *addr;

	while (WTBL_LMAC_DW8[i].name) {
		if (WTBL_LMAC_DW8[i].shift == NO_SHIFT_DEFINE)
			DBGLOG(HAL, INFO, "\t%s:%d\n", WTBL_LMAC_DW8[i].name,
				(dw_value & WTBL_LMAC_DW8[i].mask) ? 1 : 0);
		else
			DBGLOG(HAL, INFO, "\t%s:%u\n", WTBL_LMAC_DW8[i].name,
				(dw_value & WTBL_LMAC_DW8[i].mask) >>
				WTBL_LMAC_DW8[i].shift);
		i++;
	}
}

static struct {
	uint8_t *name;
	uint32_t mask;
	uint32_t shift;
	u_int8_t new_line;
} WTBL_LMAC_DW9[] = {
	{"RX_AVG_MPDU_SIZE", WF_LWTBL_RX_AVG_MPDU_SIZE_MASK,
		WF_LWTBL_RX_AVG_MPDU_SIZE_SHIFT,	FALSE},
	{"PRITX_SW_MODE",    WF_LWTBL_PRITX_SW_MODE_MASK,    NO_SHIFT_DEFINE,
		FALSE},
	{"PRITX_ERSU",       WF_LWTBL_PRITX_ERSU_MASK,       NO_SHIFT_DEFINE,
		FALSE},
	{"PRITX_PLR",        WF_LWTBL_PRITX_PLR_MASK,        NO_SHIFT_DEFINE,
		TRUE},
	{"PRITX_DCM",        WF_LWTBL_PRITX_DCM_MASK,        NO_SHIFT_DEFINE,
		FALSE},
	{"PRITX_ER106T",     WF_LWTBL_PRITX_ER106T_MASK,     NO_SHIFT_DEFINE,
		TRUE},
	/* {"FCAP(0:20 1:~40)",    WTBL_FCAP_20_TO_160_MHZ,
	 * WTBL_FCAP_20_TO_160_MHZ_OFFSET},
	 */
	{"MPDU_FAIL_CNT",    WF_LWTBL_MPDU_FAIL_CNT_MASK,
		WF_LWTBL_MPDU_FAIL_CNT_SHIFT,	FALSE},
	{"MPDU_OK_CNT",      WF_LWTBL_MPDU_OK_CNT_MASK,
		WF_LWTBL_MPDU_OK_CNT_SHIFT,	FALSE},
	{"RATE_IDX",         WF_LWTBL_RATE_IDX_MASK,
		WF_LWTBL_RATE_IDX_SHIFT,	TRUE},
	{NULL,}
};

uint8_t *fcap_name[] = {"20MHz", "20/40MHz", "20/40/80MHz",
	"20/40/80/160/80+80MHz", "20/40/80/160/80+80/320MHz"};

static void parse_bmac_lwtbl_DW9(struct ADAPTER *prAdapter, uint8_t *lwtbl)
{
	uint32_t *addr = 0;
	uint32_t dw_value = 0;
	uint16_t i = 0;

	/* LMAC WTBL DW 9 */
	DBGLOG(HAL, INFO, "\t\n");
	DBGLOG(HAL, INFO, "LWTBL DW 9\n");
	addr = (uint32_t *)&(lwtbl[WTBL_GROUP_TRX_CAP_DW_9*4]);
	dw_value = *addr;

	while (WTBL_LMAC_DW9[i].name) {
		if (WTBL_LMAC_DW9[i].shift == NO_SHIFT_DEFINE)
			DBGLOG(HAL, INFO, "\t%s:%d\n", WTBL_LMAC_DW9[i].name,
				(dw_value & WTBL_LMAC_DW9[i].mask) ? 1 : 0);
		else
			DBGLOG(HAL, INFO, "\t%s:%u\n", WTBL_LMAC_DW9[i].name,
				(dw_value & WTBL_LMAC_DW9[i].mask) >>
				WTBL_LMAC_DW9[i].shift);
		i++;
	}

	/* FCAP parser */
	DBGLOG(HAL, INFO, "\t\n");
	DBGLOG(HAL, INFO, "FCAP:%s\n",
	       fcap_name[(dw_value & WF_LWTBL_FCAP_MASK)
	       >> WF_LWTBL_FCAP_SHIFT]);
}

#define MAX_TX_MODE 16
static char *CONNAC3X_HW_TX_MODE_STR[] = {
				 "CCK", "OFDM", "HT-Mix", "HT-GF", "VHT",
				 "N/A", "N/A", "N/A",
				 "HE_SU", "HE_EXT_SU", "HE_TRIG", "HE_MU",
				 "N/A",
				 "EHT_EXT_SU", "EHT_TRIG", "EHT_MU",
				 "N/A"};
static char *CONNAC3X_HW_TX_RATE_CCK_STR[] = {"1M", "2Mlong", "5.5Mlong",
	"11Mlong", "N/A", "2Mshort", "5.5Mshort", "11Mshort", "N/A"};

static char *hw_rate_str(uint8_t mode, uint16_t rate_idx)
{
	if (mode == 0)
		return rate_idx < 8 ? CONNAC3X_HW_TX_RATE_CCK_STR[rate_idx] :
			CONNAC3X_HW_TX_RATE_CCK_STR[8];
	else if (mode == 1)
		return nicHwRateOfdmStr(rate_idx);
	else
		return "MCS";
}

static void parse_rate(struct ADAPTER *prAdapter, uint16_t rate_idx,
		       uint16_t txrate)
{
	uint16_t txmode, mcs, nss, stbc;

	txmode = HW_TX_RATE_TO_MODE(txrate);
	mcs = HW_TX_RATE_TO_MCS(txrate);
	nss = HW_TX_RATE_TO_NSS(txrate);
	stbc = HW_TX_RATE_TO_STBC(txrate);

	DBGLOG(HAL, INFO,
	       "\tRate%d(0x%x):TxMode=%d(%s), TxRate=%d(%s), Nsts=%d, STBC=%d\n",
		rate_idx + 1,
		txrate,
		txmode,
		(txmode < MAX_TX_MODE ? CONNAC3X_HW_TX_MODE_STR[txmode] :
		 CONNAC3X_HW_TX_MODE_STR[MAX_TX_MODE]),
		mcs,
		hw_rate_str(txmode, mcs),
		nss,
		stbc);
}

static struct {
	uint8_t *name;
	uint32_t mask;
	uint32_t shift;
} WTBL_LMAC_DW10[] = {
	{"RATE1",       WF_LWTBL_RATE1_MASK,        WF_LWTBL_RATE1_SHIFT},
	{"RATE2",       WF_LWTBL_RATE2_MASK,        WF_LWTBL_RATE2_SHIFT},
	{NULL,}
};

static void parse_bmac_lwtbl_DW10(struct ADAPTER *prAdapter, uint8_t *lwtbl)
{
	uint32_t *addr = 0;
	uint32_t dw_value = 0;
	uint16_t i = 0;

	/* LMAC WTBL DW 10 */
	DBGLOG(HAL, INFO, "\t\n");
	DBGLOG(HAL, INFO, "LWTBL DW 10\n");
	addr = (uint32_t *)&(lwtbl[WTBL_GROUP_AUTO_RATE_1_2 * 4]);
	dw_value = *addr;

	while (WTBL_LMAC_DW10[i].name) {
		parse_rate(prAdapter, i, (dw_value & WTBL_LMAC_DW10[i].mask) >>
			   WTBL_LMAC_DW10[i].shift);
		i++;
	}
}

static struct {
	uint8_t *name;
	uint32_t mask;
	uint32_t shift;
} WTBL_LMAC_DW11[] = {
	{"RATE3",       WF_LWTBL_RATE3_MASK,        WF_LWTBL_RATE3_SHIFT},
	{"RATE4",       WF_LWTBL_RATE4_MASK,        WF_LWTBL_RATE4_SHIFT},
	{NULL,}
};

static void parse_bmac_lwtbl_DW11(struct ADAPTER *prAdapter, uint8_t *lwtbl)
{
	uint32_t *addr = 0;
	uint32_t dw_value = 0;
	uint16_t i = 0;

	/* LMAC WTBL DW 11 */
	DBGLOG(HAL, INFO, "\t\n");
	DBGLOG(HAL, INFO, "LWTBL DW 11\n");
	addr = (uint32_t *)&(lwtbl[WTBL_GROUP_AUTO_RATE_3_4 * 4]);
	dw_value = *addr;

	while (WTBL_LMAC_DW11[i].name) {
		parse_rate(prAdapter, i+2, (dw_value & WTBL_LMAC_DW11[i].mask)
			   >> WTBL_LMAC_DW11[i].shift);
		i++;
	}
}

static struct {
	uint8_t *name;
	uint32_t mask;
	uint32_t shift;
} WTBL_LMAC_DW12[] = {
	{"RATE5",       WF_LWTBL_RATE5_MASK,        WF_LWTBL_RATE5_SHIFT},
	{"RATE6",       WF_LWTBL_RATE6_MASK,        WF_LWTBL_RATE6_SHIFT},
	{NULL,}
};

static void parse_bmac_lwtbl_DW12(struct ADAPTER *prAdapter, uint8_t *lwtbl)
{
	uint32_t *addr = 0;
	uint32_t dw_value = 0;
	uint16_t i = 0;

	/* LMAC WTBL DW 12 */
	DBGLOG(HAL, INFO, "\t\n");
	DBGLOG(HAL, INFO, "LWTBL DW 12\n");
	addr = (uint32_t *)&(lwtbl[WTBL_GROUP_AUTO_RATE_5_6 * 4]);
	dw_value = *addr;

	while (WTBL_LMAC_DW12[i].name) {
		parse_rate(prAdapter, i+4, (dw_value & WTBL_LMAC_DW12[i].mask)
			   >> WTBL_LMAC_DW12[i].shift);
		i++;
	}
}

static struct {
	uint8_t *name;
	uint32_t mask;
	uint32_t shift;
} WTBL_LMAC_DW13[] = {
	{"RATE7",       WF_LWTBL_RATE7_MASK,        WF_LWTBL_RATE7_SHIFT},
	{"RATE8",       WF_LWTBL_RATE8_MASK,        WF_LWTBL_RATE8_SHIFT},
	{NULL,}
};

static void parse_bmac_lwtbl_DW13(struct ADAPTER *prAdapter, uint8_t *lwtbl)
{
	uint32_t *addr = 0;
	uint32_t dw_value = 0;
	uint16_t i = 0;

	/* LMAC WTBL DW 13 */
	DBGLOG(HAL, INFO, "\t\n");
	DBGLOG(HAL, INFO, "LWTBL DW 13\n");
	addr = (uint32_t *)&(lwtbl[WTBL_GROUP_AUTO_RATE_7_8 * 4]);
	dw_value = *addr;

	while (WTBL_LMAC_DW13[i].name) {
		parse_rate(prAdapter, i+6, (dw_value & WTBL_LMAC_DW13[i].mask)
			   >> WTBL_LMAC_DW13[i].shift);
		i++;
	}
}

static struct {
	uint8_t *name;
	uint32_t mask;
	uint32_t shift;
	u_int8_t new_line;
} WTBL_LMAC_DW14_BMC[] = {
	{"CIPHER_IGTK",         WF_LWTBL_CIPHER_SUIT_IGTK_MASK,
		WF_LWTBL_CIPHER_SUIT_IGTK_SHIFT,		FALSE},
	{"CIPHER_BIGTK",        WF_LWTBL_CIPHER_SUIT_BIGTK_MASK,
		WF_LWTBL_CIPHER_SUIT_BIGTK_SHIFT,	TRUE},
	{NULL,}
};

static void parse_bmac_lwtbl_DW14(struct ADAPTER *prAdapter, uint8_t *lwtbl)
{
	uint32_t *addr, *muar_addr = 0;
	uint32_t dw_value, muar_dw_value = 0;
	uint16_t i = 0;

	/* DUMP DW14 for BMC entry only */
	muar_addr = (uint32_t *)&(lwtbl[WF_LWTBL_MUAR_DW * 4]);
	muar_dw_value = *muar_addr;
	if (((muar_dw_value & WF_LWTBL_MUAR_MASK) >> WF_LWTBL_MUAR_SHIFT)
		== MUAR_INDEX_OWN_MAC_ADDR_BC_MC) {
		/* LMAC WTBL DW 14 */
		DBGLOG(HAL, INFO, "\t\n");
		DBGLOG(HAL, INFO, "LWTBL DW 14\n");
		addr = (uint32_t *)&(lwtbl[WF_LWTBL_CIPHER_SUIT_IGTK_DW * 4]);
		dw_value = *addr;

		while (WTBL_LMAC_DW14_BMC[i].name) {
			parse_rate(prAdapter, i+6,
				(dw_value & WTBL_LMAC_DW14_BMC[i].mask) >>
					WTBL_LMAC_DW14_BMC[i].shift);
			i++;
		}
	}
}

static struct {
	uint8_t *name;
	uint32_t mask;
	uint32_t shift;
	u_int8_t new_line;
} WTBL_LMAC_DW28[] = {
	{"RELATED_IDX0",	WF_LWTBL_RELATED_IDX0_MASK,
		WF_LWTBL_RELATED_IDX0_SHIFT,	FALSE},
	{"RELATED_BAND0",	WF_LWTBL_RELATED_BAND0_MASK,
		WF_LWTBL_RELATED_BAND0_SHIFT,	FALSE},
	{"PRI_MLD_BAND",    WF_LWTBL_PRIMARY_MLD_BAND_MASK,
		WF_LWTBL_PRIMARY_MLD_BAND_SHIFT,	TRUE},
	{"RELATED_IDX0",	WF_LWTBL_RELATED_IDX1_MASK,
		WF_LWTBL_RELATED_IDX1_SHIFT,	FALSE},
	{"RELATED_BAND1",   WF_LWTBL_RELATED_BAND1_MASK,
		WF_LWTBL_RELATED_BAND1_SHIFT,	FALSE},
	{"SEC_MLD_BAND",	WF_LWTBL_SECONDARY_MLD_BAND_MASK,
		WF_LWTBL_SECONDARY_MLD_BAND_SHIFT,	TRUE},
	{NULL,}
};

static void parse_bmac_lwtbl_DW28(struct ADAPTER *prAdapter, uint8_t *lwtbl)
{
	uint32_t *addr = 0;
	uint32_t dw_value = 0;
	uint16_t i = 0;

	/* LMAC WTBL DW 28 */
	DBGLOG(HAL, INFO, "\t\n");
	DBGLOG(HAL, INFO, "LWTBL DW 28\n");
	addr = (uint32_t *)&(lwtbl[WTBL_GROUP_MLO_INFO_LINE_1 * 4]);
	dw_value = *addr;

	while (WTBL_LMAC_DW28[i].name) {
		if (WTBL_LMAC_DW28[i].shift == NO_SHIFT_DEFINE)
			DBGLOG(HAL, INFO, "\t%s:%d\n", WTBL_LMAC_DW28[i].name,
				(dw_value & WTBL_LMAC_DW28[i].mask) ? 1 : 0);
		else
			DBGLOG(HAL, INFO, "\t%s:%u\n", WTBL_LMAC_DW28[i].name,
				(dw_value & WTBL_LMAC_DW28[i].mask) >>
					WTBL_LMAC_DW28[i].shift);
		i++;
	}
}

static struct {
	uint8_t *name;
	uint32_t mask;
	uint32_t shift;
	u_int8_t new_line;
} WTBL_LMAC_DW29[] = {
	{"DISPATCH_POLICY_MLD_TID0", WF_LWTBL_DISPATCH_POLICY0_MASK,
		WF_LWTBL_DISPATCH_POLICY0_SHIFT,	FALSE},
	{"MLD_TID1",	WF_LWTBL_DISPATCH_POLICY1_MASK,
		WF_LWTBL_DISPATCH_POLICY1_SHIFT,	FALSE},
	{"MLD_TID2",	WF_LWTBL_DISPATCH_POLICY2_MASK,
		WF_LWTBL_DISPATCH_POLICY2_SHIFT,	FALSE},
	{"MLD_TID3",	WF_LWTBL_DISPATCH_POLICY3_MASK,
		WF_LWTBL_DISPATCH_POLICY3_SHIFT,	TRUE},
	{"MLD_TID4",	WF_LWTBL_DISPATCH_POLICY4_MASK,
		WF_LWTBL_DISPATCH_POLICY4_SHIFT,	FALSE},
	{"MLD_TID5",	WF_LWTBL_DISPATCH_POLICY5_MASK,
		WF_LWTBL_DISPATCH_POLICY5_SHIFT,	FALSE},
	{"MLD_TID6",	WF_LWTBL_DISPATCH_POLICY6_MASK,
		WF_LWTBL_DISPATCH_POLICY6_SHIFT,	FALSE},
	{"MLD_TID7",	WF_LWTBL_DISPATCH_POLICY7_MASK,
		WF_LWTBL_DISPATCH_POLICY7_SHIFT,	TRUE},
	{"OMLD_ID",		WF_LWTBL_OWN_MLD_ID_MASK,
		WF_LWTBL_OWN_MLD_ID_SHIFT,	FALSE},
	{"EMLSR0",		WF_LWTBL_EMLSR0_MASK,
		NO_SHIFT_DEFINE,	FALSE},
	{"EMLMR0",		WF_LWTBL_EMLMR0_MASK,
		NO_SHIFT_DEFINE,	FALSE},
	{"EMLSR1",		WF_LWTBL_EMLSR1_MASK,
		NO_SHIFT_DEFINE,	FALSE},
	{"EMLMR1",		WF_LWTBL_EMLMR1_MASK,
		NO_SHIFT_DEFINE,	TRUE},
	{"EMLSR2",		WF_LWTBL_EMLSR2_MASK,
		NO_SHIFT_DEFINE,	FALSE},
	{"EMLMR2",		WF_LWTBL_EMLMR2_MASK,
		NO_SHIFT_DEFINE,	FALSE},
	{"STR_BITMAP",	WF_LWTBL_STR_BITMAP_MASK,
		WF_LWTBL_STR_BITMAP_SHIFT,	TRUE},
	{NULL,}
};

static void parse_bmac_lwtbl_DW29(struct ADAPTER *prAdapter, uint8_t *lwtbl)
{
	uint32_t *addr = 0;
	uint32_t dw_value = 0;
	uint16_t i = 0;

	/* LMAC WTBL DW 29 */
	DBGLOG(HAL, INFO, "\t\n");
	DBGLOG(HAL, INFO, "LWTBL DW 29\n");
	addr = (uint32_t *)&(lwtbl[WTBL_GROUP_MLO_INFO_LINE_2 * 4]);
	dw_value = *addr;

	while (WTBL_LMAC_DW29[i].name) {
		if (WTBL_LMAC_DW29[i].shift == NO_SHIFT_DEFINE)
			DBGLOG(HAL, INFO, "\t%s:%d\n", WTBL_LMAC_DW29[i].name,
				(dw_value & WTBL_LMAC_DW29[i].mask) ? 1 : 0);
		else
			DBGLOG(HAL, INFO, "\t%s:%u\n", WTBL_LMAC_DW29[i].name,
				(dw_value & WTBL_LMAC_DW29[i].mask) >>
					WTBL_LMAC_DW29[i].shift);
		i++;
	}
}

static struct {
	uint8_t *name;
	uint32_t mask;
	uint32_t shift;
	u_int8_t new_line;
} WTBL_LMAC_DW30[] = {
	{"DISPATCH_ORDER",	WF_LWTBL_DISPATCH_ORDER_MASK,
		WF_LWTBL_DISPATCH_ORDER_SHIFT,	FALSE},
	{"DISPATCH_RATIO",	WF_LWTBL_DISPATCH_RATIO_MASK,
		WF_LWTBL_DISPATCH_RATIO_SHIFT,	FALSE},
	{"LINK_MGF",		WF_LWTBL_LINK_MGF_MASK,
		WF_LWTBL_LINK_MGF_SHIFT,	TRUE},
	{NULL,}
};

static void parse_bmac_lwtbl_DW30(struct ADAPTER *prAdapter, uint8_t *lwtbl)
{
	uint32_t *addr = 0;
	uint32_t dw_value = 0;
	uint16_t i = 0;

	/* LMAC WTBL DW 30 */
	DBGLOG(HAL, INFO, "\t\n");
	DBGLOG(HAL, INFO, "LWTBL DW 30\n");
	addr = (uint32_t *)&(lwtbl[WTBL_GROUP_MLO_INFO_LINE_3 * 4]);
	dw_value = *addr;


	while (WTBL_LMAC_DW30[i].name) {
		if (WTBL_LMAC_DW30[i].shift == NO_SHIFT_DEFINE)
			DBGLOG(HAL, INFO, "\t%s:%d\n", WTBL_LMAC_DW30[i].name,
				(dw_value & WTBL_LMAC_DW30[i].mask) ? 1 : 0);
		else
			DBGLOG(HAL, INFO, "\t%s:%u\n", WTBL_LMAC_DW30[i].name,
				(dw_value & WTBL_LMAC_DW30[i].mask) >>
				WTBL_LMAC_DW30[i].shift);
		i++;
	}
}

static struct {
	uint8_t *name;
	uint32_t mask;
	uint32_t shift;
	u_int8_t new_line;
} WTBL_LMAC_DW31[] = {
	{"NEGO_WINSIZE0",	WF_LWTBL_NEGOTIATED_WINSIZE0_MASK,
		WF_LWTBL_NEGOTIATED_WINSIZE0_SHIFT,    FALSE},
	{"WINSIZE1",	WF_LWTBL_NEGOTIATED_WINSIZE1_MASK,
		WF_LWTBL_NEGOTIATED_WINSIZE1_SHIFT,    FALSE},
	{"WINSIZE2",	WF_LWTBL_NEGOTIATED_WINSIZE2_MASK,
		WF_LWTBL_NEGOTIATED_WINSIZE2_SHIFT,    FALSE},
	{"WINSIZE3",	WF_LWTBL_NEGOTIATED_WINSIZE3_MASK,
		WF_LWTBL_NEGOTIATED_WINSIZE3_SHIFT,    TRUE},
	{"WINSIZE4",	WF_LWTBL_NEGOTIATED_WINSIZE4_MASK,
		WF_LWTBL_NEGOTIATED_WINSIZE4_SHIFT,    FALSE},
	{"WINSIZE5",	WF_LWTBL_NEGOTIATED_WINSIZE5_MASK,
		WF_LWTBL_NEGOTIATED_WINSIZE5_SHIFT,    FALSE},
	{"WINSIZE6",	WF_LWTBL_NEGOTIATED_WINSIZE6_MASK,
		WF_LWTBL_NEGOTIATED_WINSIZE6_SHIFT,    FALSE},
	{"WINSIZE7",	WF_LWTBL_NEGOTIATED_WINSIZE7_MASK,
		WF_LWTBL_NEGOTIATED_WINSIZE7_SHIFT,    TRUE},
	{"CASCAD",	        WF_LWTBL_CASCAD_MASK,
		NO_SHIFT_DEFINE,    FALSE},
	{"ALL_ACK",	        WF_LWTBL_ALL_ACK_MASK,
		NO_SHIFT_DEFINE,    FALSE},
	{"MPDU_SIZE",	WF_LWTBL_MPDU_SIZE_MASK,
		WF_LWTBL_MPDU_SIZE_SHIFT,  FALSE},
	{"BA_MODE",		WF_LWTBL_BA_MODE_MASK,
		WF_LWTBL_BA_MODE_SHIFT,  TRUE},
	{NULL,}
};

static void parse_bmac_lwtbl_DW31(struct ADAPTER *prAdapter, uint8_t *lwtbl)
{
	uint32_t *addr = 0;
	uint32_t dw_value = 0;
	uint16_t i = 0;

	/* LMAC WTBL DW 31 */
	DBGLOG(HAL, INFO, "\t\n");
	DBGLOG(HAL, INFO, "LWTBL DW 31\n");
	addr = (uint32_t *)&(lwtbl[WTBL_GROUP_RESP_INFO_DW_31 * 4]);
	dw_value = *addr;

	while (WTBL_LMAC_DW31[i].name) {
		if (WTBL_LMAC_DW31[i].shift == NO_SHIFT_DEFINE)
			DBGLOG(HAL, INFO, "\t%s:%d\n", WTBL_LMAC_DW31[i].name,
				(dw_value & WTBL_LMAC_DW31[i].mask) ? 1 : 0);
		else
			DBGLOG(HAL, INFO, "\t%s:%u\n", WTBL_LMAC_DW31[i].name,
				(dw_value & WTBL_LMAC_DW31[i].mask) >>
					WTBL_LMAC_DW31[i].shift);
		i++;
	}
}

static struct {
	uint8_t *name;
	uint32_t mask;
	uint32_t shift;
	u_int8_t new_line;
} WTBL_LMAC_DW32[] = {
	{"OM_INFO",			WF_LWTBL_OM_INFO_MASK,
		WF_LWTBL_OM_INFO_SHIFT,		FALSE},
	{"OM_RXD_DUP_MODE",		WF_LWTBL_RXD_DUP_FOR_OM_CHG_MASK,
		NO_SHIFT_DEFINE,		FALSE},
	{"RXD_DUP_WHITE_LIST",	WF_LWTBL_RXD_DUP_WHITE_LIST_MASK,
		WF_LWTBL_RXD_DUP_WHITE_LIST_SHIFT,	FALSE},
	{"RXD_DUP_MODE",		WF_LWTBL_RXD_DUP_MODE_MASK,
		WF_LWTBL_RXD_DUP_MODE_SHIFT,	FALSE},
	{"DROP",			WF_LWTBL_DROP_MASK,
		NO_SHIFT_DEFINE,		FALSE},
	{"ACK_EN",			WF_LWTBL_ACK_EN_MASK,
		NO_SHIFT_DEFINE,		TRUE},
	{NULL,}
};

static void parse_bmac_lwtbl_DW32(struct ADAPTER *prAdapter, uint8_t *lwtbl)
{
	uint32_t *addr = 0;
	uint32_t dw_value = 0;
	uint16_t i = 0;

	/* LMAC WTBL DW 32 */
	DBGLOG(HAL, INFO, "\t\n");
	DBGLOG(HAL, INFO, "LWTBL DW 32\n");
	addr = (uint32_t *)&(lwtbl[WTBL_GROUP_RX_DUP_INFO_DW_32 * 4]);
	dw_value = *addr;

	while (WTBL_LMAC_DW32[i].name) {
		if (WTBL_LMAC_DW32[i].shift == NO_SHIFT_DEFINE)
			DBGLOG(HAL, INFO, "\t%s:%d\n", WTBL_LMAC_DW32[i].name,
				(dw_value & WTBL_LMAC_DW32[i].mask) ? 1 : 0);
		else
			DBGLOG(HAL, INFO, "\t%s:%u\n", WTBL_LMAC_DW32[i].name,
				(dw_value & WTBL_LMAC_DW32[i].mask) >>
					WTBL_LMAC_DW32[i].shift);
		i++;
	}
}

static struct {
	uint8_t *name;
	uint32_t mask;
	uint32_t shift;
	u_int8_t new_line;
} WTBL_LMAC_DW33[] = {
	{"USER_RSSI",                   WF_LWTBL_USER_RSSI_MASK,
		WF_LWTBL_USER_RSSI_SHIFT,	FALSE},
	{"USER_SNR",                    WF_LWTBL_USER_SNR_MASK,
		WF_LWTBL_USER_SNR_SHIFT,	FALSE},
	{"RAPID_REACTION_RATE",         WF_LWTBL_RAPID_REACTION_RATE_MASK,
		WF_LWTBL_RAPID_REACTION_RATE_SHIFT,	TRUE},
	{"HT_AMSDU(Read Only)",         WF_LWTBL_HT_AMSDU_MASK,
		NO_SHIFT_DEFINE,	FALSE},
	{"AMSDU_CROSS_LG(Read Only)",   WF_LWTBL_AMSDU_CROSS_LG_MASK,
		NO_SHIFT_DEFINE,	TRUE},
	{NULL,}
};

static void parse_bmac_lwtbl_DW33(struct ADAPTER *prAdapter, uint8_t *lwtbl)
{
	uint32_t *addr = 0;
	uint32_t dw_value = 0;
	uint16_t i = 0;

	/* LMAC WTBL DW 33 */
	DBGLOG(HAL, INFO, "\t\n");
	DBGLOG(HAL, INFO, "LWTBL DW 33\n");
	addr = (uint32_t *)&(lwtbl[WTBL_GROUP_RX_STAT_CNT_LINE_1 * 4]);
	dw_value = *addr;

	while (WTBL_LMAC_DW33[i].name) {
		if (WTBL_LMAC_DW33[i].shift == NO_SHIFT_DEFINE)
			DBGLOG(HAL, INFO, "\t%s:%d\n", WTBL_LMAC_DW33[i].name,
				(dw_value & WTBL_LMAC_DW33[i].mask) ? 1 : 0);
		else
			DBGLOG(HAL, INFO, "\t%s:%u\n", WTBL_LMAC_DW33[i].name,
				(dw_value & WTBL_LMAC_DW33[i].mask) >>
					WTBL_LMAC_DW33[i].shift);
		i++;
	}
}

static struct {
	uint8_t *name;
	uint32_t mask;
	uint32_t shift;
	u_int8_t new_line;
} WTBL_LMAC_DW34[] = {
	{"RESP_RCPI0",	WF_LWTBL_RESP_RCPI0_MASK,
		WF_LWTBL_RESP_RCPI0_SHIFT,	FALSE},
	{"RCPI1",	WF_LWTBL_RESP_RCPI1_MASK,
		WF_LWTBL_RESP_RCPI1_SHIFT,	FALSE},
	{"RCPI2",	WF_LWTBL_RESP_RCPI2_MASK,
		WF_LWTBL_RESP_RCPI2_SHIFT,	FALSE},
	{"RCPI3",	WF_LWTBL_RESP_RCPI3_MASK,
		WF_LWTBL_RESP_RCPI3_SHIFT,	TRUE},
	{NULL,}
};

static void parse_bmac_lwtbl_DW34(struct ADAPTER *prAdapter, uint8_t *lwtbl)
{
	uint32_t *addr = 0;
	uint32_t dw_value = 0;
	uint16_t i = 0;

	/* LMAC WTBL DW 34 */
	DBGLOG(HAL, INFO, "\t\n");
	DBGLOG(HAL, INFO, "LWTBL DW 34\n");
	addr = (uint32_t *)&(lwtbl[WTBL_GROUP_RX_STAT_CNT_LINE_2 * 4]);
	dw_value = *addr;


	while (WTBL_LMAC_DW34[i].name) {
		if (WTBL_LMAC_DW34[i].shift == NO_SHIFT_DEFINE)
			DBGLOG(HAL, INFO, "\t%s:%d\n", WTBL_LMAC_DW34[i].name,
				(dw_value & WTBL_LMAC_DW34[i].mask) ? 1 : 0);
		else
			DBGLOG(HAL, INFO, "\t%s:%u\n", WTBL_LMAC_DW34[i].name,
				(dw_value & WTBL_LMAC_DW34[i].mask) >>
					WTBL_LMAC_DW34[i].shift);
		i++;
	}
}

static struct {
	uint8_t *name;
	uint32_t mask;
	uint32_t shift;
	u_int8_t new_line;
} WTBL_LMAC_DW35[] = {
	{"SNR 0",	WF_LWTBL_SNR_RX0_MASK,		WF_LWTBL_SNR_RX0_SHIFT,
		FALSE},
	{"SNR 1",	WF_LWTBL_SNR_RX1_MASK,		WF_LWTBL_SNR_RX1_SHIFT,
		FALSE},
	{"SNR 2",	WF_LWTBL_SNR_RX2_MASK,		WF_LWTBL_SNR_RX2_SHIFT,
		FALSE},
	{"SNR 3",	WF_LWTBL_SNR_RX3_MASK,		WF_LWTBL_SNR_RX3_SHIFT,
		TRUE},
	{NULL,}
};

static void parse_bmac_lwtbl_DW35(struct ADAPTER *prAdapter, uint8_t *lwtbl)
{
	uint32_t *addr = 0;
	uint32_t dw_value = 0;
	uint16_t i = 0;

	/* LMAC WTBL DW 35 */
	DBGLOG(HAL, INFO, "\t\n");
	DBGLOG(HAL, INFO, "LWTBL DW 35\n");
	addr = (uint32_t *)&(lwtbl[WTBL_GROUP_RX_STAT_CNT_LINE_3 * 4]);
	dw_value = *addr;


	while (WTBL_LMAC_DW35[i].name) {
		if (WTBL_LMAC_DW35[i].shift == NO_SHIFT_DEFINE)
			DBGLOG(HAL, INFO, "\t%s:%d\n", WTBL_LMAC_DW35[i].name,
				(dw_value & WTBL_LMAC_DW35[i].mask) ? 1 : 0);
		else
			DBGLOG(HAL, INFO, "\t%s:%u\n", WTBL_LMAC_DW35[i].name,
				(dw_value & WTBL_LMAC_DW35[i].mask) >>
					WTBL_LMAC_DW35[i].shift);
		i++;
	}
}

static void parse_bmac_lwtbl_rx_stats(struct ADAPTER *prAdapter,
				      uint8_t *lwtbl)
{
	parse_bmac_lwtbl_DW33(prAdapter, lwtbl);
	parse_bmac_lwtbl_DW34(prAdapter, lwtbl);
	parse_bmac_lwtbl_DW35(prAdapter, lwtbl);
}

static void parse_bmac_lwtbl_mlo_info(struct ADAPTER *prAdapter,
				      uint8_t *lwtbl)
{
	parse_bmac_lwtbl_DW28(prAdapter, lwtbl);
	parse_bmac_lwtbl_DW29(prAdapter, lwtbl);
	parse_bmac_lwtbl_DW30(prAdapter, lwtbl);
}


static struct {
	uint8_t *name;
	uint32_t mask;
	uint32_t shift;
	u_int8_t new_line;
} WTBL_UMAC_DW9[] = {
	{"RELATED_IDX0",	WF_UWTBL_RELATED_IDX0_MASK,
		WF_UWTBL_RELATED_IDX0_SHIFT,	FALSE},
	{"RELATED_BAND0",	WF_UWTBL_RELATED_BAND0_MASK,
		WF_UWTBL_RELATED_BAND0_SHIFT,	FALSE},
	{"PRI_MLD_BAND",    WF_UWTBL_PRIMARY_MLD_BAND_MASK,
		WF_UWTBL_PRIMARY_MLD_BAND_SHIFT,	TRUE},
	{"RELATED_IDX0",	WF_UWTBL_RELATED_IDX1_MASK,
		WF_UWTBL_RELATED_IDX1_SHIFT,	FALSE},
	{"RELATED_BAND1",   WF_UWTBL_RELATED_BAND1_MASK,
		WF_UWTBL_RELATED_BAND1_SHIFT,	FALSE},
	{"SEC_MLD_BAND",	WF_UWTBL_SECONDARY_MLD_BAND_MASK,
		WF_UWTBL_SECONDARY_MLD_BAND_SHIFT,	TRUE},
	{NULL,}
};

static void parse_bmac_uwtbl_mlo_info(struct ADAPTER *prAdapter,
				      uint8_t *uwtbl)
{
	uint32_t *addr = 0;
	uint32_t dw_value = 0;
	uint16_t i = 0;

	DBGLOG(HAL, INFO, "\t\n");
	DBGLOG(HAL, INFO,
	       "MldAddr: %02x:%02x:%02x:%02x:%02x:%02x(D0[B0~15], D1[B0~31])\n",
		uwtbl[4], uwtbl[5], uwtbl[6], uwtbl[7], uwtbl[0], uwtbl[1]);

	/* UMAC WTBL DW 0 */
	DBGLOG(HAL, INFO, "\t\n");
	DBGLOG(HAL, INFO, "UWTBL DW 0\n");
	addr = (uint32_t *)&(uwtbl[WF_UWTBL_OWN_MLD_ID_DW * 4]);
	dw_value = *addr;

	DBGLOG(HAL, INFO, "\t%s:%u\n", "OMLD_ID",
		(dw_value & WF_UWTBL_OWN_MLD_ID_MASK) >>
		WF_UWTBL_OWN_MLD_ID_SHIFT);

	/* UMAC WTBL DW 9 */
	DBGLOG(HAL, INFO, "\t\n");
	DBGLOG(HAL, INFO, "UWTBL DW 9\n");
	addr = (uint32_t *)&(uwtbl[WF_UWTBL_RELATED_IDX0_DW * 4]);
	dw_value = *addr;

	while (WTBL_UMAC_DW9[i].name) {

		if (WTBL_UMAC_DW9[i].shift == NO_SHIFT_DEFINE)
			DBGLOG(HAL, INFO, "\t%s:%d\n", WTBL_UMAC_DW9[i].name,
				(dw_value & WTBL_UMAC_DW9[i].mask) ? 1 : 0);
		else
			DBGLOG(HAL, INFO, "\t%s:%u\n", WTBL_UMAC_DW9[i].name,
				 (dw_value & WTBL_UMAC_DW9[i].mask) >>
					WTBL_UMAC_DW9[i].shift);
		i++;
	}
}


static bool is_wtbl_bigtk_exist(struct ADAPTER *prAdapter, uint8_t *lwtbl)
{
	uint32_t *addr = 0;
	uint32_t dw_value = 0;

	addr = (uint32_t *)&(lwtbl[WF_LWTBL_MUAR_DW*4]);
	dw_value = *addr;
	if (((dw_value & WF_LWTBL_MUAR_MASK) >> WF_LWTBL_MUAR_SHIFT) ==
					MUAR_INDEX_OWN_MAC_ADDR_BC_MC) {
		addr = (uint32_t *)&(lwtbl[WF_LWTBL_CIPHER_SUIT_BIGTK_DW * 4]);
		dw_value = *addr;
		if (((dw_value & WF_LWTBL_CIPHER_SUIT_BIGTK_MASK) >>
		     WF_LWTBL_CIPHER_SUIT_BIGTK_SHIFT) != IGTK_CIPHER_SUIT_NONE)
			return TRUE;
	}

	return FALSE;
}

static struct {
	uint8_t *name;
	uint32_t mask;
	uint32_t shift;
	u_int8_t new_line;
} WTBL_UMAC_DW2[] = {
	{"PN0",		WTBL_PN0_MASK,		WTBL_PN0_OFFSET,	FALSE},
	{"PN1",		WTBL_PN1_MASK,		WTBL_PN1_OFFSET,	FALSE},
	{"PN2",		WTBL_PN2_MASK,		WTBL_PN2_OFFSET,	TRUE},
	{"PN3",		WTBL_PN3_MASK,		WTBL_PN3_OFFSET,	FALSE},
	{NULL,}
};

static struct {
	uint8_t *name;
	uint32_t mask;
	uint32_t shift;
	u_int8_t new_line;
} WTBL_UMAC_DW3[] = {
	{"PN4",     WTBL_PN4_MASK,      WTBL_PN4_OFFSET,	FALSE},
	{"PN5",     WTBL_PN5_MASK,      WTBL_PN5_OFFSET,	TRUE},
	{NULL,}
};

static struct {
	uint8_t *name;
	uint32_t mask;
	uint32_t shift;
	u_int8_t new_line;
} WTBL_UMAC_DW4_BIPN[] = {
	{"BIPN0",	WTBL_BIPN0_MASK,	WTBL_BIPN0_OFFSET,	FALSE},
	{"BIPN1",	WTBL_BIPN1_MASK,	WTBL_BIPN1_OFFSET,	FALSE},
	{"BIPN2",	WTBL_BIPN2_MASK,	WTBL_BIPN2_OFFSET,	TRUE},
	{"BIPN3",	WTBL_BIPN3_MASK,	WTBL_BIPN3_OFFSET,	FALSE},
	{NULL,}
};

static struct {
	uint8_t *name;
	uint32_t mask;
	uint32_t shift;
	u_int8_t new_line;
} WTBL_UMAC_DW5_BIPN[] = {
	{"BIPN4",	WTBL_BIPN0_MASK,	WTBL_BIPN0_OFFSET,	FALSE},
	{"BIPN5",	WTBL_BIPN1_MASK,	WTBL_BIPN1_OFFSET,	TRUE},
	{NULL,}
};

static void parse_bmac_uwtbl_pn(struct ADAPTER *prAdapter, uint8_t *uwtbl,
				uint8_t *lwtbl)
{
	uint32_t *addr = 0;
	uint32_t dw_value = 0;
	uint16_t i = 0;

	DBGLOG(HAL, INFO, "\t\n");
	DBGLOG(HAL, INFO, "UWTBL PN\n");

	/* UMAC WTBL DW 2/3 */
	addr = (uint32_t *)&(uwtbl[WF_UWTBL_PN_31_0__DW * 4]);
	dw_value = *addr;

	while (WTBL_UMAC_DW2[i].name) {
		DBGLOG(HAL, INFO, "\t%s:%u\n", WTBL_UMAC_DW2[i].name,
			(dw_value & WTBL_UMAC_DW2[i].mask) >>
				WTBL_UMAC_DW2[i].shift);
		i++;
	}

	i = 0;
	addr = (uint32_t *)&(uwtbl[WF_UWTBL_PN_47_32__DW * 4]);
	dw_value = *addr;

	while (WTBL_UMAC_DW3[i].name) {
		DBGLOG(HAL, INFO, "\t%s:%u\n", WTBL_UMAC_DW3[i].name,
			 (dw_value & WTBL_UMAC_DW3[i].mask) >>
			WTBL_UMAC_DW3[i].shift);
		i++;
	}


	/* UMAC WTBL DW 4/5 for BIGTK */
	if (is_wtbl_bigtk_exist(prAdapter, lwtbl) == TRUE) {
		i = 0;
		addr = (uint32_t *)&(uwtbl[WF_UWTBL_RX_BIPN_31_0__DW * 4]);
		dw_value = *addr;

		while (WTBL_UMAC_DW4_BIPN[i].name) {
			DBGLOG(HAL, INFO, "\t%s:%u\n",
				WTBL_UMAC_DW4_BIPN[i].name,
				(dw_value & WTBL_UMAC_DW4_BIPN[i].mask) >>
					WTBL_UMAC_DW4_BIPN[i].shift);
			i++;
		}

		i = 0;
		addr = (uint32_t *)&(uwtbl[WF_UWTBL_RX_BIPN_47_32__DW * 4]);
		dw_value = *addr;

		while (WTBL_UMAC_DW5_BIPN[i].name) {
			DBGLOG(HAL, INFO, "\t%s:%u\n",
				WTBL_UMAC_DW5_BIPN[i].name,
				(dw_value & WTBL_UMAC_DW5_BIPN[i].mask) >>
				WTBL_UMAC_DW5_BIPN[i].shift);
			i++;
		}
	}
}

static void parse_bmac_uwtbl_sn(struct ADAPTER *prAdapter, uint8_t *uwtbl)
{
	uint32_t *addr = 0;
	uint32_t u2SN = 0;

	/* UMAC WTBL DW SN part */
	DBGLOG(HAL, INFO, "\t\n");
	DBGLOG(HAL, INFO, "UWTBL SN\n");

	addr = (uint32_t *)&(uwtbl[WF_UWTBL_TID0_SN_DW * 4]);
	u2SN = ((*addr) & WF_UWTBL_TID0_SN_MASK) >> WF_UWTBL_TID0_SN_SHIFT;
	DBGLOG(HAL, INFO, "\t%s:%u\n", "TID0_AC0_SN", u2SN);

	addr = (uint32_t *)&(uwtbl[WF_UWTBL_TID1_SN_DW * 4]);
	u2SN = ((*addr) & WF_UWTBL_TID1_SN_MASK) >> WF_UWTBL_TID1_SN_SHIFT;
	DBGLOG(HAL, INFO, "\t%s:%u\n", "TID1_AC1_SN", u2SN);

	addr = (uint32_t *)&(uwtbl[WF_UWTBL_TID2_SN_7_0__DW * 4]);
	u2SN = ((*addr) & WF_UWTBL_TID2_SN_7_0__MASK) >>
				WF_UWTBL_TID2_SN_7_0__SHIFT;
	addr = (uint32_t *)&(uwtbl[WF_UWTBL_TID2_SN_11_8__DW * 4]);
	u2SN |= (((*addr) & WF_UWTBL_TID2_SN_11_8__MASK) >>
			WF_UWTBL_TID2_SN_11_8__SHIFT) << 8;
	DBGLOG(HAL, INFO, "\t%s:%u\n", "TID2_AC2_SN", u2SN);

	addr = (uint32_t *)&(uwtbl[WF_UWTBL_TID3_SN_DW * 4]);
	u2SN = ((*addr) & WF_UWTBL_TID3_SN_MASK) >> WF_UWTBL_TID3_SN_SHIFT;
	DBGLOG(HAL, INFO, "\t%s:%u\n", "TID3_AC3_SN", u2SN);

	addr = (uint32_t *)&(uwtbl[WF_UWTBL_TID4_SN_DW * 4]);
	u2SN = ((*addr) & WF_UWTBL_TID4_SN_MASK) >> WF_UWTBL_TID4_SN_SHIFT;
	DBGLOG(HAL, INFO, "\t%s:%u\n", "TID4_SN", u2SN);

	addr = (uint32_t *)&(uwtbl[WF_UWTBL_TID5_SN_3_0__DW * 4]);
	u2SN = ((*addr) & WF_UWTBL_TID5_SN_3_0__MASK) >>
				WF_UWTBL_TID5_SN_3_0__SHIFT;
	addr = (uint32_t *)&(uwtbl[WF_UWTBL_TID5_SN_11_4__DW * 4]);
	u2SN |= (((*addr) & WF_UWTBL_TID5_SN_11_4__MASK) >>
				WF_UWTBL_TID5_SN_11_4__SHIFT) << 4;
	DBGLOG(HAL, INFO, "\t%s:%u\n", "TID5_SN", u2SN);

	addr = (uint32_t *)&(uwtbl[WF_UWTBL_TID6_SN_DW * 4]);
	u2SN = ((*addr) & WF_UWTBL_TID6_SN_MASK) >> WF_UWTBL_TID6_SN_SHIFT;
	DBGLOG(HAL, INFO, "\t%s:%u\n", "TID6_SN", u2SN);

	addr = (uint32_t *)&(uwtbl[WF_UWTBL_TID7_SN_DW * 4]);
	u2SN = ((*addr) & WF_UWTBL_TID7_SN_MASK) >> WF_UWTBL_TID7_SN_SHIFT;
	DBGLOG(HAL, INFO, "\t%s:%u\n", "TID7_SN", u2SN);

	addr = (uint32_t *)&(uwtbl[WF_UWTBL_COM_SN_DW * 4]);
	u2SN = ((*addr) & WF_UWTBL_COM_SN_MASK) >> WF_UWTBL_COM_SN_SHIFT;
	DBGLOG(HAL, INFO, "\t%s:%u\n", "COM_SN", u2SN);
}

static void dump_key_table(
	struct ADAPTER *prAdapter,
	uint16_t keyloc0,
	uint16_t keyloc1,
	uint16_t keyloc2)
{
	uint8_t keytbl[ONE_KEY_ENTRY_LEN_IN_DW*4] = {0};
	uint16_t x;
	uint32_t u4Value;

	DBGLOG(HAL, INFO, "\t\n");
	DBGLOG(HAL, INFO, "\t%s:%d\n", "keyloc0", keyloc0);
	if (keyloc0 != INVALID_KEY_ENTRY) {

		/* Don't swap below two lines, halWtblReadRaw will
		* write new value WF_WTBLON_TOP_WDUCR_ADDR
		*/
		halWtblReadRaw(prAdapter, keyloc0,
			WTBL_TYPE_KEY, 0, ONE_KEY_ENTRY_LEN_IN_DW, keytbl);
		HAL_MCR_RD(prAdapter, WF_UWTBL_TOP_WDUCR_ADDR, &u4Value);
		DBGLOG(HAL, INFO,
			"\t\tKEY WTBL Addr: group:0x%x=0x%x addr: 0x%x\n",
			WF_UWTBL_TOP_WDUCR_ADDR,
			u4Value,
			KEYTBL_IDX2BASE(keyloc0, 0));
		for (x = 0; x < ONE_KEY_ENTRY_LEN_IN_DW; x++) {
			DBGLOG(HAL, INFO, "\t\tDW%02d: %02x %02x %02x %02x\n",
				x,
				keytbl[x * 4 + 3],
				keytbl[x * 4 + 2],
				keytbl[x * 4 + 1],
				keytbl[x * 4]);
		}
	}

	DBGLOG(HAL, INFO, "\t%s:%d\n", "keyloc1", keyloc1);
	if (keyloc1 != INVALID_KEY_ENTRY) {
		/* Don't swap below two lines, halWtblReadRaw will
		* write new value WF_WTBLON_TOP_WDUCR_ADDR
		*/
		halWtblReadRaw(prAdapter, keyloc1,
			WTBL_TYPE_KEY, 0, ONE_KEY_ENTRY_LEN_IN_DW, keytbl);
		HAL_MCR_RD(prAdapter, WF_UWTBL_TOP_WDUCR_ADDR, &u4Value);
		DBGLOG(HAL, INFO,
			"\t\tKEY WTBL Addr: group:0x%x=0x%x addr: 0x%x\n",
			WF_UWTBL_TOP_WDUCR_ADDR,
			u4Value,
			KEYTBL_IDX2BASE(keyloc1, 0));
		for (x = 0; x < ONE_KEY_ENTRY_LEN_IN_DW; x++) {
			DBGLOG(HAL, INFO, "\t\tDW%02d: %02x %02x %02x %02x\n",
				x,
				keytbl[x * 4 + 3],
				keytbl[x * 4 + 2],
				keytbl[x * 4 + 1],
				keytbl[x * 4]);
		}
	}

	DBGLOG(HAL, INFO, "\t%s:%d\n", "keyloc2", keyloc2);
	if (keyloc2 != INVALID_KEY_ENTRY) {
		/* Don't swap below two lines, halWtblReadRaw will
		* write new value WF_WTBLON_TOP_WDUCR_ADDR
		*/
		halWtblReadRaw(prAdapter, keyloc2,
			WTBL_TYPE_KEY, 0, ONE_KEY_ENTRY_LEN_IN_DW, keytbl);
		HAL_MCR_RD(prAdapter, WF_UWTBL_TOP_WDUCR_ADDR, &u4Value);
		DBGLOG(HAL, INFO,
			"\t\tKEY WTBL Addr: group:0x%x=0x%x addr: 0x%x\n",
			WF_UWTBL_TOP_WDUCR_ADDR,
			u4Value,
			KEYTBL_IDX2BASE(keyloc2, 0));
		for (x = 0; x < ONE_KEY_ENTRY_LEN_IN_DW; x++) {
			DBGLOG(HAL, INFO, "\t\tDW%02d: %02x %02x %02x %02x\n",
				x,
				keytbl[x * 4 + 3],
				keytbl[x * 4 + 2],
				keytbl[x * 4 + 1],
				keytbl[x * 4]);
		}
	}
}

static void parse_bmac_uwtbl_key_info(struct ADAPTER *prAdapter, uint8_t *uwtbl,
				      uint8_t *lwtbl)
{
	uint32_t *addr = 0;
	uint32_t dw_value = 0;
	uint16_t keyloc0 = INVALID_KEY_ENTRY;
	uint16_t keyloc1 = INVALID_KEY_ENTRY;
	uint16_t keyloc2 = INVALID_KEY_ENTRY;

	/* UMAC WTBL DW 7 */
	DBGLOG(HAL, INFO, "\t\n");
	DBGLOG(HAL, INFO, "UWTBL key info\n");

	addr = (uint32_t *)&(uwtbl[WF_UWTBL_KEY_LOC0_DW * 4]);
	dw_value = *addr;
	keyloc0 = (dw_value & WF_UWTBL_KEY_LOC0_MASK) >>
		WF_UWTBL_KEY_LOC0_SHIFT;
	keyloc1 = (dw_value & WF_UWTBL_KEY_LOC1_MASK) >>
		WF_UWTBL_KEY_LOC1_SHIFT;

	DBGLOG(HAL, INFO, "\t%s:%u/%u\n", "Key Loc 0/1", keyloc0, keyloc1);

	/* UMAC WTBL DW 6 for BIGTK */
	if (is_wtbl_bigtk_exist(prAdapter, lwtbl) == TRUE) {
		keyloc2 = (dw_value & WF_UWTBL_KEY_LOC2_MASK) >>
			WF_UWTBL_KEY_LOC2_SHIFT;
		DBGLOG(HAL, INFO, "\t%s:%u\n", "Key Loc 2", keyloc2);
	}

	/* Parse KEY link */
	dump_key_table(prAdapter, keyloc0, keyloc1, keyloc2);
}

static struct {
	uint8_t *name;
	uint32_t mask;
	uint32_t shift;
	u_int8_t new_line;
} WTBL_UMAC_DW8[] = {
	{"UWTBL_WMM_Q",		WF_UWTBL_WMM_Q_MASK,
		WF_UWTBL_WMM_Q_SHIFT,	FALSE},
	{"UWTBL_QOS",		WF_UWTBL_QOS_MASK,
		NO_SHIFT_DEFINE,	FALSE},
	{"UWTBL_HT_VHT_HE",	WF_UWTBL_HT_MASK,
		NO_SHIFT_DEFINE,	FALSE},
	{"UWTBL_HDRT_MODE",	WF_UWTBL_HDRT_MODE_MASK,
		NO_SHIFT_DEFINE,	TRUE},
	{NULL,}
};

static void parse_bmac_uwtbl_msdu_info(struct ADAPTER *prAdapter,
				       uint8_t *uwtbl)
{
	uint32_t *addr = 0;
	uint32_t dw_value = 0;
	uint32_t amsdu_len = 0;
	uint16_t i = 0;

	/* UMAC WTBL DW 8 */
	DBGLOG(HAL, INFO, "\t\n");
	DBGLOG(HAL, INFO, "UWTBL DW8\n");

	addr = (uint32_t *)&(uwtbl[WF_UWTBL_AMSDU_CFG_DW * 4]);
	dw_value = *addr;

	while (WTBL_UMAC_DW8[i].name) {

		if (WTBL_UMAC_DW8[i].shift == NO_SHIFT_DEFINE)
			DBGLOG(HAL, INFO, "\t%s:%d\n", WTBL_UMAC_DW8[i].name,
				(dw_value & WTBL_UMAC_DW8[i].mask) ? 1 : 0);
		else
			DBGLOG(HAL, INFO, "\t%s:%u\n", WTBL_UMAC_DW8[i].name,
				(dw_value & WTBL_UMAC_DW8[i].mask) >>
					WTBL_UMAC_DW8[i].shift);
		i++;
	}

	/* UMAC WTBL DW 8 - AMSDU_CFG */
	DBGLOG(HAL, INFO, "\t%s:%d\n", "HW AMSDU Enable",
		(dw_value & WTBL_AMSDU_EN_MASK) ? 1 : 0);

	amsdu_len = (dw_value & WTBL_AMSDU_LEN_MASK) >> WTBL_AMSDU_LEN_OFFSET;
	if (amsdu_len == 0)
		DBGLOG(HAL, INFO, "\t%s:invalid (WTBL value=0x%x)\n",
		       "HW AMSDU Len",
			amsdu_len);
	else if (amsdu_len == 1)
		DBGLOG(HAL, INFO, "\t%s:%d~%d (WTBL value=0x%x)\n",
		       "HW AMSDU Len",
			1,
			255,
			amsdu_len);
	else if (amsdu_len == 2)
		DBGLOG(HAL, INFO, "\t%s:%d~%d (WTBL value=0x%x)\n",
		       "HW AMSDU Len",
			256,
			511,
			amsdu_len);
	else if (amsdu_len == 3)
		DBGLOG(HAL, INFO, "\t%s:%d~%d (WTBL value=0x%x)\n",
		       "HW AMSDU Len",
			512,
			767,
			amsdu_len);
	else
		DBGLOG(HAL, INFO, "\t%s:%d~%d (WTBL value=0x%x)\n",
		       "HW AMSDU Len",
			256 * (amsdu_len - 1),
			256 * (amsdu_len - 1) + 255,
			amsdu_len);

	DBGLOG(HAL, INFO, "\t%s:%lu (WTBL value=0x%lx)\n", "HW AMSDU Num",
		((dw_value & WTBL_AMSDU_NUM_MASK) >> WTBL_AMSDU_NUM_OFFSET) + 1,
		(dw_value & WTBL_AMSDU_NUM_MASK) >> WTBL_AMSDU_NUM_OFFSET);
}

static void dump_bmac_wtbl_info(struct ADAPTER *prAdapter, uint8_t *lwtbl,
				uint8_t *uwtbl)
{
	/* Parse LWTBL */
	parse_bmac_lwtbl_DW0_1(prAdapter, lwtbl);
	parse_bmac_lwtbl_DW2(prAdapter, lwtbl);
	parse_bmac_lwtbl_DW3(prAdapter, lwtbl);
	parse_bmac_lwtbl_DW4(prAdapter, lwtbl);
	parse_bmac_lwtbl_DW5(prAdapter, lwtbl);
	parse_bmac_lwtbl_DW6(prAdapter, lwtbl);
	parse_bmac_lwtbl_DW7(prAdapter, lwtbl);
	parse_bmac_lwtbl_DW8(prAdapter, lwtbl);
	parse_bmac_lwtbl_DW9(prAdapter, lwtbl);
	parse_bmac_lwtbl_DW10(prAdapter, lwtbl);
	parse_bmac_lwtbl_DW11(prAdapter, lwtbl);
	parse_bmac_lwtbl_DW12(prAdapter, lwtbl);
	parse_bmac_lwtbl_DW13(prAdapter, lwtbl);
	parse_bmac_lwtbl_DW14(prAdapter, lwtbl);
	parse_bmac_lwtbl_mlo_info(prAdapter, lwtbl);
	parse_bmac_lwtbl_DW31(prAdapter, lwtbl);
	parse_bmac_lwtbl_DW32(prAdapter, lwtbl);
	parse_bmac_lwtbl_rx_stats(prAdapter, lwtbl);

	/* Parse UWTBL */
	parse_bmac_uwtbl_mlo_info(prAdapter, uwtbl);
	parse_bmac_uwtbl_pn(prAdapter, uwtbl, lwtbl);
	parse_bmac_uwtbl_sn(prAdapter, uwtbl);
	parse_bmac_uwtbl_key_info(prAdapter, uwtbl, lwtbl);
	parse_bmac_uwtbl_msdu_info(prAdapter, uwtbl);
}

int32_t connac3x_show_wtbl_info(
	struct ADAPTER *prAdapter,
	uint32_t u4Index,
	char *pcCommand,
	int i4TotalLen)
{
	int32_t i4BytesWritten = 0;
	uint8_t lwtbl[LWTBL_LEN_IN_DW * 4] = {0};
	uint8_t uwtbl[UWTBL_LEN_IN_DW * 4] = {0};
	int x;
	uint8_t real_lwtbl_size = 0;
	uint32_t u4Value = 0;

	real_lwtbl_size = LWTBL_LEN_IN_DW;

	/* Don't swap below two lines,
	 * halWtblReadRaw will write new value WF_WTBLON_TOP_WDUCR_ADDR
	 */
	halWtblReadRaw(prAdapter, u4Index, WTBL_TYPE_LMAC, 0, real_lwtbl_size,
		       lwtbl);
	HAL_MCR_RD(prAdapter, WF_WTBLON_TOP_WDUCR_ADDR, &u4Value);
	DBGLOG(HAL, INFO, "Dump WTBL info of WLAN_IDX:%d\n", u4Index);
	DBGLOG(HAL, INFO, "LMAC WTBL Addr: group:0x%x=0x%x addr: 0x%x\n",
		WF_WTBLON_TOP_WDUCR_ADDR,
		u4Value,
		LWTBL_IDX2BASE(u4Index, 0));
	for (x = 0; x < real_lwtbl_size; x++) {
		DBGLOG(HAL, INFO, "DW%02d: %02x %02x %02x %02x\n",
			x,
			lwtbl[x * 4 + 3],
			lwtbl[x * 4 + 2],
			lwtbl[x * 4 + 1],
			lwtbl[x * 4]);
	}

	/* Don't swap below two lines,
	 * halWtblReadRaw will write new value WF_WTBLON_TOP_WDUCR_ADDR
	 */
	halWtblReadRaw(prAdapter, u4Index, WTBL_TYPE_UMAC, 0, UWTBL_LEN_IN_DW,
		       uwtbl);
	HAL_MCR_RD(prAdapter, WF_UWTBL_TOP_WDUCR_ADDR, &u4Value);
	DBGLOG(HAL, INFO, "UMAC WTBL Addr: group:0x%x=0x%x addr: 0x%x\n",
		WF_UWTBL_TOP_WDUCR_ADDR,
		u4Value,
		UWTBL_IDX2BASE(u4Index, 0));
	for (x = 0; x < UWTBL_LEN_IN_DW; x++) {
		DBGLOG(HAL, INFO, "DW%02d: %02x %02x %02x %02x\n",
			x,
			uwtbl[x * 4 + 3],
			uwtbl[x * 4 + 2],
			uwtbl[x * 4 + 1],
			uwtbl[x * 4]);
	}

	dump_bmac_wtbl_info(prAdapter, lwtbl, uwtbl);

	return i4BytesWritten;
}

#endif /* CFG_SUPPORT_CONNAC3X */
