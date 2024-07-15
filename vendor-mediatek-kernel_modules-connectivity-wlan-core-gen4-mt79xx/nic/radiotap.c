/* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause */
/*
 * Copyright (c) 2016 MediaTek Inc.
 */

/*! \file   radiotap.c
 *    \brief  Functions that provide many rx-related functions
 *
 *    This file includes the functions used to process RFB and dispatch RFBs to
 *    the appropriate related rx functions for protocols.
 */

#if (CFG_SUPPORT_SNIFFER_RADIOTAP == 1)

/*******************************************************************************
 *                         C O M P I L E R   F L A G S
 *******************************************************************************
 */

/*******************************************************************************
 *                    E X T E R N A L   R E F E R E N C E S
 *******************************************************************************
 */
#include "precomp.h"
#include "radiotap.h"

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

/*******************************************************************************
 *                                 M A C R O S
 *******************************************************************************
 */

/*******************************************************************************
 *                   F U N C T I O N   D E C L A R A T I O N S
 *******************************************************************************
 */

/*******************************************************************************
 *                              F U N C T I O N S
 *******************************************************************************
 */
static void radiotap_fill_vendor(
				struct IEEE80211_RADIOTAP_INFO *p_radiotap_info,
				uint8_t *p_data)
{
	struct VENDOR_NAMESPACE *p_vendor = (struct VENDOR_NAMESPACE *)p_data;
	uint8_t aucMtkOui[] = VENDOR_OUI_MTK;

	p_vendor->aucOUI[0] = aucMtkOui[0];
	p_vendor->aucOUI[1] = aucMtkOui[1];
	p_vendor->aucOUI[2] = aucMtkOui[2];
	p_vendor->ucSubNamespace = p_radiotap_info->ucSubNamespace;
	p_vendor->u2DataLen = p_radiotap_info->u2VendorLen;
}

static void radiotap_fill_he_mu(struct IEEE80211_RADIOTAP_INFO *p_radiotap_info,
				uint8_t *p_data)
{
	struct HE_MU *heMu = (struct HE_MU *)p_data;
	uint16_t flags1 = 0;
	uint16_t flags2 = 0;

	flags1 = p_radiotap_info->u2DataDcm <<
		IEEE80211_RADIOTAP_HE_MU_DCM_SHFT;
	flags1 |= (IEEE80211_RADIOTAP_HE_MU_MCS_KNOWN_MASK |
		IEEE80211_RADIOTAP_HE_MU_DCM_KNOWN_MASK |
		IEEE80211_RADIOTAP_HE_MU_CH1_RU_KNOWN_MASK |
		IEEE80211_RADIOTAP_HE_MU_USER_KNOWN_MASK);

	switch (p_radiotap_info->ucFrMode) {
	case BW_20:
		heMu->aucRuChannel1[0] = p_radiotap_info->ucSigBRU0;
		break;
	case BW_40:
		flags1 |= IEEE80211_RADIOTAP_HE_MU_CH2_RU_KNOWN_MASK;
		heMu->aucRuChannel1[0] = p_radiotap_info->ucSigBRU0;
		heMu->aucRuChannel2[0] = p_radiotap_info->ucSigBRU1;
		break;
	case BW_80:
	case BW_160:
		flags1 |= IEEE80211_RADIOTAP_HE_MU_CH2_RU_KNOWN_MASK;
		heMu->aucRuChannel1[0] = p_radiotap_info->ucSigBRU0;
		heMu->aucRuChannel2[0] = p_radiotap_info->ucSigBRU1;
		heMu->aucRuChannel1[1] = p_radiotap_info->ucSigBRU2;
		heMu->aucRuChannel2[1] = p_radiotap_info->ucSigBRU3;
		break;
	default:
		break;
	}

	flags2 = p_radiotap_info->ucFrMode;
	flags2 |= IEEE80211_RADIOTAP_HE_MU_BW_KNOWN_MASK;
	flags2 |= (IEEE80211_RADIOTAP_HE_MU_USER_MASK &
		   (p_radiotap_info->ucNumUser <<
		    IEEE80211_RADIOTAP_HE_MU_USER_SHFT));

	heMu->u2Flag1 = flags1;
	heMu->u2Flag2 = flags2;
}

static void radiotap_fill_he(struct IEEE80211_RADIOTAP_INFO *p_radiotap_info,
			     uint8_t *p_data)
{
	struct HE *he = (struct HE *)p_data;
	uint16_t bw_ru_alloc;
	uint16_t offset_ru_alloc;
	uint16_t spatial_reuse = 0;

	/* Data 1 */
	switch (p_radiotap_info->ucTxMode) {
	case TX_RATE_MODE_HE_SU:
		he->u2Data1 = IEEE80211_RADIOTAP_HE_SU;
		he->u2Data1 |= IEEE80211_RADIOTAP_HE_KNOWN_SPATIAL_REUSE1;
		spatial_reuse = p_radiotap_info->u2SpatialReuse1;
		break;
	case TX_RATE_MODE_EHT_ER:
	case TX_RATE_MODE_HE_ER:
		he->u2Data1 = IEEE80211_RADIOTAP_HE_EXT_SU;
		he->u2Data1 |= IEEE80211_RADIOTAP_HE_KNOWN_SPATIAL_REUSE1;
		spatial_reuse = p_radiotap_info->u2SpatialReuse1;
		break;
	case TX_RATE_MODE_EHT_TRIG:
	case TX_RATE_MODE_HE_TRIG:
		he->u2Data1 = IEEE80211_RADIOTAP_HE_TRIG;
		he->u2Data1 |= (IEEE80211_RADIOTAP_HE_KNOWN_SPATIAL_REUSE1 |
				IEEE80211_RADIOTAP_HE_KNOWN_SPATIAL_REUSE2 |
				IEEE80211_RADIOTAP_HE_KNOWN_SPATIAL_REUSE3 |
				IEEE80211_RADIOTAP_HE_KNOWN_SPATIAL_REUSE4);
		spatial_reuse = (p_radiotap_info->u2SpatialReuse1 |
				(p_radiotap_info->u2SpatialReuse2 <<
				 IEEE80211_RADIOTAP_HE_SPATIAL_REUSE2_SHFT) |
				(p_radiotap_info->u2SpatialReuse3 <<
				 IEEE80211_RADIOTAP_HE_SPATIAL_REUSE3_SHFT) |
				(p_radiotap_info->u2SpatialReuse4 <<
				 IEEE80211_RADIOTAP_HE_SPATIAL_REUSE4_SHFT));
		break;
	case TX_RATE_MODE_EHT_MU:
	case TX_RATE_MODE_HE_MU:
		he->u2Data1 = IEEE80211_RADIOTAP_HE_MU;
		he->u2Data1 |= (IEEE80211_RADIOTAP_HE_KNOWN_SPATIAL_REUSE1 |
					IEEE80211_RADIOTAP_HE_KNOWN_STAID);
		spatial_reuse = (p_radiotap_info->u2SpatialReuse1 |
				(p_radiotap_info->u2VhtPartialAid <<
				 IEEE80211_RADIOTAP_HE_SPATIAL_REUSE2_SHFT));
		break;
	default:
		break;
	}

	he->u2Data1 |= IEEE80211_RADIOTAP_HE_KNOWN_DATA1;

	if (p_radiotap_info->u2RuAllocation <=
	    IEEE80211_RADIOTAP_HE_RU_IDX_26_RU37) {
		bw_ru_alloc = IEEE80211_RADIOTAP_HE_RU_26;
		offset_ru_alloc = p_radiotap_info->u2RuAllocation;
	} else if (p_radiotap_info->u2RuAllocation <=
		   IEEE80211_RADIOTAP_HE_RU_IDX_52_RU16) {
		bw_ru_alloc = IEEE80211_RADIOTAP_HE_RU_52;
		offset_ru_alloc = p_radiotap_info->u2RuAllocation -
			IEEE80211_RADIOTAP_HE_RU_IDX_26_RU37 - 1;
	} else if (p_radiotap_info->u2RuAllocation <=
		   IEEE80211_RADIOTAP_HE_RU_IDX_106_RU8) {
		bw_ru_alloc = IEEE80211_RADIOTAP_HE_RU_106;
		offset_ru_alloc = p_radiotap_info->u2RuAllocation -
			IEEE80211_RADIOTAP_HE_RU_IDX_52_RU16 - 1;
	} else if (p_radiotap_info->u2RuAllocation <=
		   IEEE80211_RADIOTAP_HE_RU_IDX_242_RU4) {
		bw_ru_alloc = IEEE80211_RADIOTAP_HE_RU_242;
		offset_ru_alloc = p_radiotap_info->u2RuAllocation -
			IEEE80211_RADIOTAP_HE_RU_IDX_106_RU8 - 1;
	} else if (p_radiotap_info->u2RuAllocation <=
		   IEEE80211_RADIOTAP_HE_RU_IDX_484_RU2) {
		bw_ru_alloc = IEEE80211_RADIOTAP_HE_RU_484;
		offset_ru_alloc = p_radiotap_info->u2RuAllocation -
			IEEE80211_RADIOTAP_HE_RU_IDX_242_RU4 - 1;
	} else if (p_radiotap_info->u2RuAllocation ==
		   IEEE80211_RADIOTAP_HE_RU_IDX_996_RU1) {
		bw_ru_alloc = IEEE80211_RADIOTAP_HE_RU_996;
		offset_ru_alloc = 0;
	} else if (p_radiotap_info->u2RuAllocation ==
		   IEEE80211_RADIOTAP_HE_RU_IDX_2x_996_RU1) {
		bw_ru_alloc = IEEE80211_RADIOTAP_HE_RU_2x_996;
		offset_ru_alloc = 0;
	} else {
		bw_ru_alloc = p_radiotap_info->ucFrMode;
		offset_ru_alloc = p_radiotap_info->u2RuAllocation;
	}

	/* Data 2 */
	he->u2Data2 = IEEE80211_RADIOTAP_HE_KNOWN_DATA2;
	he->u2Data2 |= ((offset_ru_alloc <<
			 IEEE80211_RADIOTAP_HE_RU_ALLOC_OFFSET_OFFSET) &
		IEEE80211_RADIOTAP_HE_RU_ALLOC_OFFSET_MASK);

	/* Data 3 */
	he->u2Data3 = ((p_radiotap_info->u2BssClr) |
		(p_radiotap_info->u2BeamChange <<
		 IEEE80211_RADIOTAP_HE_BEAM_CHANGE_SHFT) |
		(p_radiotap_info->u2UlDl << IEEE80211_RADIOTAP_HE_UL_DL_SHFT) |
		((p_radiotap_info->ucMcs << IEEE80211_RADIOTAP_HE_DATA_MCS_SHFT)
		 & IEEE80211_RADIOTAP_HE_DATA_MCS_MASK) |
		(p_radiotap_info->u2DataDcm <<
		 IEEE80211_RADIOTAP_HE_DATA_DCM_SHFT) |
		(p_radiotap_info->ucLDPC << IEEE80211_RADIOTAP_HE_CODING_SHFT) |
		(p_radiotap_info->ucLdpcExtraOfdmSym <<
		 IEEE80211_RADIOTAP_HE_LDPC_EXTRA_SHFT) |
		(p_radiotap_info->ucSTBC << IEEE80211_RADIOTAP_HE_STBC_SHFT));

	/* Data 4 */
	he->u2Data4 = spatial_reuse;

	/* Data 5 */

	he->u2Data5 = bw_ru_alloc |
		(p_radiotap_info->ucShortGI << IEEE80211_RADIOTAP_HE_GI_SHFT) |
		(p_radiotap_info->u2Ltf <<
		 IEEE80211_RADIOTAP_HE_LTF_SYMBO_SHFT) |
		(p_radiotap_info->ucBeamFormed <<
		 IEEE80211_RADIOTAP_HE_TX_BF_SHFT) |
		(p_radiotap_info->ucPeDisamb <<
		 IEEE80211_RADIOTAP_HE_PE_DISAMB_SHFT);

	/* Data 6 */
	he->u2Data6 = (p_radiotap_info->ucNsts |
		(p_radiotap_info->u2Doppler <<
		 IEEE80211_RADIOTAP_HE_DOPPLER_SHFT) |
		(p_radiotap_info->u2Txop << IEEE80211_RADIOTAP_HE_TXOP_SHFT));
}

static void radiotap_fill_timestamp(
			    struct IEEE80211_RADIOTAP_INFO *p_radiotap_info,
			    uint8_t *p_data)
{
	struct TIMESTAMP *p_timestamp = (struct TIMESTAMP *)p_data;

	p_timestamp->u8Timestamp = p_radiotap_info->u4Timestamp;
	/* microseconds, matches TSFT field */
	p_timestamp->ucUnit = 0x1;
	/* 32-bit counter */
	p_timestamp->ucFlags = 0x1;
}

static void radiotap_fill_vht(struct IEEE80211_RADIOTAP_INFO *p_radiotap_info,
			      uint8_t *p_data)
{
	struct VHT *vht = (struct VHT *)p_data;
	uint8_t flags = 0;

	if (p_radiotap_info->ucSTBC)
		flags |= IEEE80211_RADIOTAP_VHT_FLAG_STBC;

	if (p_radiotap_info->ucTxopPsNotAllow)
		flags |= IEEE80211_RADIOTAP_VHT_FLAG_TXOP_PS_NA;

	if (p_radiotap_info->ucShortGI)
		flags |= IEEE80211_RADIOTAP_VHT_FLAG_SGI;

	if (p_radiotap_info->ucPeDisamb)
		flags |= IEEE80211_RADIOTAP_VHT_FLAG_SGI_NSYM_M10_9;

	if (p_radiotap_info->ucLdpcExtraOfdmSym)
		flags |= IEEE80211_RADIOTAP_VHT_FLAG_LDPC_EXTRA_OFDM_SYM;

	if (p_radiotap_info->ucBeamFormed)
		flags |= IEEE80211_RADIOTAP_VHT_FLAG_BEAMFORMED;

	vht->u2VhtKnown = IEEE80211_RADIOTAP_VHT_KNOWN_ALL;
	vht->ucVhtFlags = flags;

	switch (p_radiotap_info->ucFrMode) {
	case BW_20:
		vht->ucVhtBandwidth = IEEE80211_RADIOTAP_VHT_BW_20;
		break;
	case BW_40:
		vht->ucVhtBandwidth = IEEE80211_RADIOTAP_VHT_BW_40;
		break;
	case BW_80:
		vht->ucVhtBandwidth = IEEE80211_RADIOTAP_VHT_BW_80;
		break;
	case BW_160:
		vht->ucVhtBandwidth = IEEE80211_RADIOTAP_VHT_BW_160;
		break;
	default:
		break;
	}

	/* STBC = Nsts - Nss */
	vht->aucVhtMcsNss[0] = ((p_radiotap_info->ucMcs << 4) |
		(p_radiotap_info->ucNsts - p_radiotap_info->ucSTBC));
	vht->ucVhtCoding = 0;
	vht->ucVhtGroupId = p_radiotap_info->ucVhtGroupId;
	vht->u2VhtPartialAid = p_radiotap_info->u2VhtPartialAid;
}

static void radiotap_fill_ampdu(struct IEEE80211_RADIOTAP_INFO *p_radiotap_info,
				uint8_t *p_data)
{
	struct AMPDU *p_ampdu = (struct AMPDU *)p_data;

	p_ampdu->u4AmpduRefNum = p_radiotap_info->u4AmpduRefNum;
}

static void radiotap_fill_mcs(struct IEEE80211_RADIOTAP_INFO *p_radiotap_info,
			      uint8_t *p_data)
{
	struct MCS *p_mcs = (struct MCS *)p_data;
	uint8_t flags = 0;
	uint8_t known = 0;

	flags = p_radiotap_info->ucFrMode;

	if (p_radiotap_info->ucShortGI)
		flags |= IEEE80211_RADIOTAP_MCS_SGI;

	if (p_radiotap_info->ucTxMode == TX_RATE_MODE_HTGF)
		flags |= IEEE80211_RADIOTAP_MCS_FMT_GF;

	if (p_radiotap_info->ucLDPC)
		flags |= IEEE80211_RADIOTAP_MCS_FEC_LDPC;

	flags |= (p_radiotap_info->ucSTBC << IEEE80211_RADIOTAP_MCS_STBC);

	if (p_radiotap_info->ucNess & BIT(0))
		flags |= IEEE80211_RADIOTAP_MCS_NESS;

	known = IEEE80211_RADIOTAP_MCS_HAVE_ALL;

	if (p_radiotap_info->ucNess & BIT(1))
		known |= IEEE80211_RADIOTAP_MCS_NESS;

	p_mcs->ucMcsKnown = known;
	p_mcs->ucMcsFlags = flags;
	p_mcs->ucMcsMcs = p_radiotap_info->ucMcs;
}

static void radiotap_fill_antenna(
				struct IEEE80211_RADIOTAP_INFO *p_radiotap_info,
				uint8_t *p_data)
{
	struct ANTENNA *p_antenna = (struct ANTENNA *)p_data;

	p_antenna->ucAntIdx = 0;
}

static void radiotap_fill_ant_signal(
				struct IEEE80211_RADIOTAP_INFO *p_radiotap_info,
				uint8_t *p_data)
{
	struct ANT_SIGNAL *p_ant_signal = (struct ANT_SIGNAL *)p_data;

	p_ant_signal->i1AntennaSignal =
		(int8_t)RCPI_TO_dBm(p_radiotap_info->ucRcpi0);
}

static void radiotap_fill_channel(
				struct IEEE80211_RADIOTAP_INFO *p_radiotap_info,
				uint8_t *p_data)
{
	struct CHANNEL *p_channel = (struct CHANNEL *)p_data;
	enum ENUM_BAND eBand = (enum ENUM_BAND)p_radiotap_info->ucRfBand;
	uint8_t ucChanNum = p_radiotap_info->ucChanNum;
	uint16_t flags = 0;
	uint32_t freq = 0;

	if (p_radiotap_info->ucTxMode == TX_RATE_MODE_CCK)
		flags |= IEEE80211_CHAN_CCK;
	else
		flags |= IEEE80211_CHAN_OFDM;

#if (CFG_SUPPORT_WIFI_6G == 1)
	nicRxdChNumTranslate(eBand, &ucChanNum);
#endif
	if (eBand == BAND_2G4)
		freq = (ucChanNum * 5 + 2407);
	else if (eBand == BAND_5G)
		freq = (ucChanNum * 5 + 5000);
#if (CFG_SUPPORT_WIFI_6G == 1)
	else if (eBand == BAND_6G)
		freq = (ucChanNum * 5 + 5950);
#endif
	if (eBand <= BAND_2G4)
		flags |= IEEE80211_CHAN_2GHZ;
	else
		flags |= IEEE80211_CHAN_5GHZ;

	p_channel->u2ChFrequency = freq;
	p_channel->u2ChFlags = flags;
}

static void radiotap_fill_rate(struct IEEE80211_RADIOTAP_INFO *p_radiotap_info,
			       uint8_t *p_data)
{
	struct RATE *p_rate = (struct RATE *)p_data;

	p_rate->ucRate = nicGetHwRateByPhyRate(p_radiotap_info->ucMcs);
}

static void radiotap_fill_flags(struct IEEE80211_RADIOTAP_INFO *p_radiotap_info,
				uint8_t *p_data)
{
	struct FLAGS *p_flags = (struct FLAGS *)p_data;
	uint8_t flags = 0;

	if (p_radiotap_info->ucFrag)
		flags |= IEEE80211_RADIOTAP_F_FRAG;

	if (p_radiotap_info->ucFcsErr)
		flags |= IEEE80211_RADIOTAP_F_BADFCS;

	if (p_radiotap_info->ucShortGI)
		flags |= IEEE80211_RADIOTAP_F_SHORTGI;

	p_flags->ucFlags = flags;
}

void radiotapFillRadiotap(IN struct ADAPTER *prAdapter,
			    IN OUT struct SW_RFB *prSwRfb)
{
	struct RX_CTRL *prRxCtrl = &prAdapter->rRxCtrl;
	struct sk_buff *prSkb = (struct sk_buff *)(prSwRfb->pvPacket);
	struct IEEE80211_RADIOTAP_INFO radiotapInfo;
	struct IEEE80211_RADIOTAP_HEADER *header;
	struct IEEE80211_RADIOTAP_FIELD_FUNC
		radiotap_fill_func[IEEE80211_RADIOTAP_SUPPORT_NUM];
	struct RX_DESC_OPS_T *prRxDescOps = prAdapter->chip_info->prRxDescOps;
	uint8_t ucFillRadiotap = FALSE;
	uint8_t *p_base;
	uint8_t *p_data;
	uint8_t func_idx;
	uint8_t func_num = 0;
	uint16_t radiotap_len = sizeof(struct IEEE80211_RADIOTAP_HEADER);
	uint16_t padding_len = 0;
	uint32_t present;

	if (prRxDescOps->nic_rxd_fill_radiotap) {
		prSwRfb->prRadiotapInfo = &radiotapInfo;
		ucFillRadiotap = prRxDescOps->nic_rxd_fill_radiotap(prAdapter,
								    prSwRfb);
	}

	if (ucFillRadiotap == FALSE) {
		DBGLOG(RX, ERROR, "fill radiotap info fail!\n");
		goto bypass;
	}

	switch (radiotapInfo.ucTxMode) {
	case TX_RATE_MODE_CCK:
	case TX_RATE_MODE_OFDM:
		present = IEEE80211_RADIOTAP_FIELD_PRESENT_LEGACY;
		break;
	case TX_RATE_MODE_HTMIX:
	case TX_RATE_MODE_HTGF:
		present = IEEE80211_RADIOTAP_FIELD_PRESENT_HT;
		break;
	case TX_RATE_MODE_VHT:
		present = IEEE80211_RADIOTAP_FIELD_PRESENT_VHT;
		break;
	case TX_RATE_MODE_EHT_ER:
	case TX_RATE_MODE_EHT_TRIG:
	case TX_RATE_MODE_HE_SU:
	case TX_RATE_MODE_HE_ER:
	case TX_RATE_MODE_HE_TRIG:
		present = IEEE80211_RADIOTAP_FIELD_PRESENT_HE;
		break;
	case TX_RATE_MODE_EHT_MU:
	case TX_RATE_MODE_HE_MU:
		present = IEEE80211_RADIOTAP_FIELD_PRESENT_HE_MU;
		break;
	default:
		present = IEEE80211_RADIOTAP_FIELD_VENDOR;
		break;
	}

	/* Bit Number 1 FLAGS */
	if (present & IEEE80211_RADIOTAP_FIELD_FLAGS) {
		radiotap_fill_func[func_num].offset = radiotap_len;
		radiotap_fill_func[func_num].radiotap_fill_func =
			radiotap_fill_flags;
		radiotap_len += sizeof(struct FLAGS);
		func_num++;
	}

	/* Bit Number 2 RATE */
	if (present & IEEE80211_RADIOTAP_FIELD_RATE) {
		radiotap_fill_func[func_num].offset = radiotap_len;
		radiotap_fill_func[func_num].radiotap_fill_func =
			radiotap_fill_rate;
		radiotap_len += sizeof(struct RATE);
		func_num++;
	}

	/* Bit Number 3 CHANNEL */
	if (present & IEEE80211_RADIOTAP_FIELD_CHANNEL) {
		/* Required Alignment 2 bytes */
		padding_len = radiotap_len % 2;
		radiotap_len += padding_len;
		radiotap_fill_func[func_num].offset = radiotap_len;
		radiotap_fill_func[func_num].radiotap_fill_func =
			radiotap_fill_channel;
		radiotap_len += sizeof(struct CHANNEL);
		func_num++;
	}

	/* Bit Number 5 ANT SIGNAL */
	if (present & IEEE80211_RADIOTAP_FIELD_ANT_SIGNAL) {
		radiotap_fill_func[func_num].offset = radiotap_len;
		radiotap_fill_func[func_num].radiotap_fill_func =
			radiotap_fill_ant_signal;
		radiotap_len += sizeof(struct ANT_SIGNAL);
		func_num++;
	}

	/* Bit Number 11 ANTENNA */
	if (present & IEEE80211_RADIOTAP_FIELD_ANTENNA) {
		radiotap_fill_func[func_num].offset = radiotap_len;
		radiotap_fill_func[func_num].radiotap_fill_func =
			radiotap_fill_antenna;
		radiotap_len += sizeof(struct ANTENNA);
		func_num++;
	}

	/* Bit Number 19 MCS */
	if (present & IEEE80211_RADIOTAP_FIELD_MCS) {
		radiotap_fill_func[func_num].offset = radiotap_len;
		radiotap_fill_func[func_num].radiotap_fill_func =
			radiotap_fill_mcs;
		radiotap_len += sizeof(struct MCS);
		func_num++;
	}

	/* Bit Number 20 A-MPDU */
	if (present & IEEE80211_RADIOTAP_FIELD_AMPDU) {
		/* Required Alignment 4 bytes */
		padding_len = ((radiotap_len % 4) == 0) ? 0 :
				(4 - (radiotap_len % 4));
		radiotap_len += padding_len;
		radiotap_fill_func[func_num].offset = radiotap_len;
		radiotap_fill_func[func_num].radiotap_fill_func =
			radiotap_fill_ampdu;
		radiotap_len += sizeof(struct AMPDU);
		func_num++;
	}

	/* Bit Number 21 VHT */
	if (present & IEEE80211_RADIOTAP_FIELD_VHT) {
		/* Required Alignment 2 bytes */
		padding_len = radiotap_len % 2;
		radiotap_len += padding_len;
		radiotap_fill_func[func_num].offset = radiotap_len;
		radiotap_fill_func[func_num].radiotap_fill_func =
			radiotap_fill_vht;
		radiotap_len += sizeof(struct VHT);
		func_num++;
	}

	/* Bit Number 22 TIMESTAMP */
	if (present & IEEE80211_RADIOTAP_FIELD_TIMESTAMP) {
		/* Required Alignment 8 bytes */
		padding_len = ((radiotap_len % 8) == 0) ? 0 :
				(8 - (radiotap_len % 8));
		radiotap_len += padding_len;
		radiotap_fill_func[func_num].offset = radiotap_len;
		radiotap_fill_func[func_num].radiotap_fill_func =
			radiotap_fill_timestamp;
		radiotap_len += sizeof(struct TIMESTAMP);
		func_num++;
	}

	/* Bit Number 23 HE */
	if (present & IEEE80211_RADIOTAP_FIELD_HE) {
		/* Required Alignment 2 bytes */
		padding_len = radiotap_len % 2;
		radiotap_len += padding_len;
		radiotap_fill_func[func_num].offset = radiotap_len;
		radiotap_fill_func[func_num].radiotap_fill_func =
			radiotap_fill_he;
		radiotap_len += sizeof(struct HE);
		func_num++;
	}

	/* Bit Number 24 HE-MU */
	if (present & IEEE80211_RADIOTAP_FIELD_HE_MU) {
		/* Required Alignment 2 bytes */
		padding_len = radiotap_len % 2;
		radiotap_len += padding_len;
		radiotap_fill_func[func_num].offset = radiotap_len;
		radiotap_fill_func[func_num].radiotap_fill_func =
			radiotap_fill_he_mu;
		radiotap_len += sizeof(struct HE_MU);
		func_num++;
	}

	/* Bit Number 30 Vendor Namespace */
	if (present & IEEE80211_RADIOTAP_FIELD_VENDOR) {
		/* Required Alignment 2 bytes */
		padding_len = radiotap_len % 2;
		radiotap_len += padding_len;
		radiotap_fill_func[func_num].offset = radiotap_len;
		radiotap_fill_func[func_num].radiotap_fill_func =
			radiotap_fill_vendor;
		radiotap_len += sizeof(struct VENDOR_NAMESPACE);
		func_num++;
	}

	/* exceed skb headroom the kernel will panic */
	if (skb_headroom(prSkb) < radiotap_len) {
		DBGLOG(RX, ERROR, "radiotap exceed skb headroom!\n");
		goto bypass;
	}

	skb_push(prSkb, radiotap_len);
	p_base = (uint8_t *)(prSkb->data);
	kalMemZero(p_base, radiotap_len);

	skb_reset_tail_pointer(prSkb);
	skb_trim(prSkb, 0);
	skb_put(prSkb, (radiotap_len + prSwRfb->u2RxByteCount));

	header = (struct IEEE80211_RADIOTAP_HEADER *)p_base;
	header->ucItVersion = PKTHDR_RADIOTAP_VERSION;
	radiotap_len += radiotapInfo.u2VendorLen;
	header->u2ItLen = cpu_to_le16(radiotap_len);
	header->u4ItPresent = present;

	for (func_idx = 0; func_idx < func_num; func_idx++)	{
		p_data = p_base + radiotap_fill_func[func_idx].offset;
		radiotap_fill_func[func_idx].radiotap_fill_func(&radiotapInfo,
								p_data);
	}

#if CFG_SUPPORT_MULTITHREAD
	if (HAL_IS_RX_DIRECT(prAdapter)) {
		kalRxIndicateOnePkt(prAdapter->prGlueInfo,
			(void *) GLUE_GET_PKT_DESCRIPTOR(
				GLUE_GET_PKT_QUEUE_ENTRY(prSwRfb->pvPacket)));
		RX_ADD_CNT(prRxCtrl, RX_DATA_INDICATION_COUNT, 1);
	} else {
		KAL_SPIN_LOCK_DECLARATION();

		KAL_ACQUIRE_SPIN_LOCK(prAdapter, SPIN_LOCK_RX_TO_OS_QUE);
		QUEUE_INSERT_TAIL(&(prAdapter->rRxQueue),
			(struct QUE_ENTRY *) GLUE_GET_PKT_QUEUE_ENTRY(
				prSwRfb->pvPacket));
		KAL_RELEASE_SPIN_LOCK(prAdapter, SPIN_LOCK_RX_TO_OS_QUE);

		prRxCtrl->ucNumIndPacket++;
		kalSetTxEvent2Rx(prAdapter->prGlueInfo);
	}

	prSwRfb->pvPacket = NULL;

	if (nicRxSetupRFB(prAdapter, prSwRfb)) {
		DBGLOG(RX, WARN,
		       "Cannot allocate packet buffer for SwRfb!\n");
		if (!timerPendingTimer(
			    &prAdapter->rPacketDelaySetupTimer)) {
			DBGLOG(RX, WARN,
				"Start ReturnIndicatedRfb Timer (%u)\n",
			  RX_RETURN_INDICATED_RFB_TIMEOUT_SEC);
			cnmTimerStartTimer(prAdapter,
				&prAdapter->rPacketDelaySetupTimer,
				SEC_TO_MSEC(
					RX_RETURN_INDICATED_RFB_TIMEOUT_SEC));
		}
	}
#endif
bypass:
	nicRxReturnRFB(prAdapter, prSwRfb);
}
#endif
