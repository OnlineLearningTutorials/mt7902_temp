/* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause */
/*
 * Copyright (c) 2016 MediaTek Inc.
 */
/*
 * Id: include/mgmt/privacy.h#1
 */

/*! \file   privacy.h
 *  \brief This file contains the function declaration for privacy.c.
 */


#ifndef _PRIVACY_H
#define _PRIVACY_H

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
#define MAX_KEY_NUM                             4
#define WEP_40_LEN                              5
#define WEP_104_LEN                             13
#define WEP_128_LEN                             16
#define LEGACY_KEY_MAX_LEN                      16
#define CCMP_KEY_LEN                            16
#define TKIP_KEY_LEN                            32
#define MAX_KEY_LEN                             32
#define MIC_RX_KEY_OFFSET                       16
#define MIC_TX_KEY_OFFSET                       24
#define MIC_KEY_LEN                             8

#define WEP_KEY_ID_FIELD      BITS(0, 29)
#define KEY_ID_FIELD          BITS(0, 7)

#define IS_TRANSMIT_KEY       BIT(31)
#define IS_UNICAST_KEY        BIT(30)
#define IS_AUTHENTICATOR      BIT(28)

/* WTBL cipher selector, sync with HAL RX from hal_hw_def_rom.h */
#define CIPHER_SUITE_NONE               0
#define CIPHER_SUITE_WEP40              1
#define CIPHER_SUITE_TKIP               2
#define CIPHER_SUITE_TKIP_WO_MIC        3
#define CIPHER_SUITE_CCMP               4
#define CIPHER_SUITE_WEP104             5
#define CIPHER_SUITE_BIP                6
#define CIPHER_SUITE_WEP128             7
#define CIPHER_SUITE_WPI                8
#define CIPHER_SUITE_CCMP_W_CCX         9  /* CCMP-128 for DFP or CCX MFP */
#define CIPHER_SUITE_CCMP_256           10
#define CIPHER_SUITE_GCMP_128           11
#define CIPHER_SUITE_GCMP_256           12
#define CIPHER_SUITE_GCM_WPI_128        13

/* Todo:: Move to register */
#if defined(MT6630)
#define WTBL_RESERVED_ENTRY             255
#else
#define WTBL_RESERVED_ENTRY             255
#endif
/* Todo:: By chip capability */
/* Max wlan table size, the max+1 used for probe request,... mgmt frame */
/*sending use basic rate and no security */
#define WTBL_SIZE                       32

#define WTBL_ALLOC_FAIL                 WTBL_RESERVED_ENTRY
#define WTBL_DEFAULT_ENTRY              0

/*******************************************************************************
 *                         D A T A   T Y P E S
 *******************************************************************************
 */

struct IEEE_802_1X_HDR {
	uint8_t ucVersion;
	uint8_t ucType;
	uint16_t u2Length;
	/* followed by length octets of data */
};

struct EAPOL_KEY {
	uint8_t ucType;
	/* Note: key_info, key_length, and key_data_length are unaligned */
	uint8_t aucKeyInfo[2];	/* big endian */
	uint8_t aucKeyLength[2];	/* big endian */
	uint8_t aucReplayCounter[8];
	uint8_t aucKeyNonce[16];
	uint8_t aucKeyIv[16];
	uint8_t aucKeyRsc[8];
	uint8_t aucKeyId[8];	/* Reserved in IEEE 802.11i/RSN */
	uint8_t aucKeyMic[16];
	uint8_t aucKeyDataLength[2];	/* big endian */
	/* followed by key_data_length bytes of key_data */
};

/* WPA2 PMKID candicate structure */
struct PMKID_CANDICATE {
	uint8_t aucBssid[MAC_ADDR_LEN];
	uint32_t u4PreAuthFlags;
};

#if 0
/* WPA2 PMKID cache structure */
struct PMKID_ENTRY {
	struct PARAM_BSSID_INFO rBssidInfo;
	u_int8_t fgPmkidExist;
};
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
/* TRUE if CM error || Unprotected UC frame */
#define IS_INCORRECT_SEC_RX_FRAME(_prSwRfb, _aucDestAddr, u2FrameCtrl) \
	(_prSwRfb->fgIsCipherMS || \
	(IS_UCAST_MAC_ADDR(_aucDestAddr) && \
		!(u2FrameCtrl & MASK_FC_PROTECTED_FRAME)))

/*******************************************************************************
 *                  F U N C T I O N   D E C L A R A T I O N S
 *******************************************************************************
 */

void secInit(IN struct ADAPTER *prAdapter,
	     IN uint8_t ucBssIndex);

void secSetPortBlocked(IN struct ADAPTER *prAdapter,
		       IN struct STA_RECORD *prSta, IN u_int8_t fgPort);

u_int8_t secCheckClassError(IN struct ADAPTER *prAdapter,
			    IN struct SW_RFB *prSwRfb,
			    IN struct STA_RECORD *prStaRec);

u_int8_t secTxPortControlCheck(IN struct ADAPTER *prAdapter,
			       IN struct MSDU_INFO *prMsduInfo,
			       IN struct STA_RECORD *prStaRec);

u_int8_t secRxPortControlCheck(IN struct ADAPTER *prAdapter,
			       IN struct SW_RFB *prSWRfb);

void secSetCipherSuite(IN struct ADAPTER *prAdapter,
		       IN uint32_t u4CipherSuitesFlags,
		       IN uint8_t ucBssIndex);

u_int8_t secIsProtectedFrame(IN struct ADAPTER *prAdapter,
			     IN struct MSDU_INFO *prMsdu,
			     IN struct STA_RECORD *prStaRec);

u_int8_t secRsnKeyHandshakeEnabled(IN struct ADAPTER
				   *prAdapter);

uint8_t secGetBmcWlanIndex(IN struct ADAPTER *prAdapter,
			   IN enum ENUM_NETWORK_TYPE eNetType,
			   IN struct STA_RECORD *prStaRec);

u_int8_t secTransmitKeyExist(IN struct ADAPTER *prAdapter,
			     IN struct STA_RECORD *prSta);

u_int8_t secEnabledInAis(IN struct ADAPTER *prAdapter,
		IN uint8_t ucBssIndex);

u_int8_t secPrivacySeekForEntry(IN struct ADAPTER
				*prAdapter, IN struct STA_RECORD *prSta);

void secPrivacyFreeForEntry(IN struct ADAPTER *prAdapter,
			    IN uint8_t ucEntry);

void secPrivacyFreeSta(IN struct ADAPTER *prAdapter,
		       IN struct STA_RECORD *prStaRec);

void secRemoveBssBcEntry(IN struct ADAPTER *prAdapter,
			 IN struct BSS_INFO *prBssInfo, IN u_int8_t fgRoam);

uint8_t
secPrivacySeekForBcEntry(IN struct ADAPTER *prAdapter,
			 IN uint8_t ucBssIndex,
			 IN uint8_t *pucAddr, IN uint8_t ucStaIdx,
			 IN uint8_t ucAlg, IN uint8_t ucKeyId);

uint8_t secGetStaIdxByWlanIdx(IN struct ADAPTER *prAdapter,
			      IN uint8_t ucWlanIdx);

#ifdef CFG_SUPPORT_UNIFIED_COMMAND
uint8_t secGetWlanIdxByStaIdx(struct ADAPTER *prAdapter,
				uint8_t ucStaIndex);
#endif

uint8_t secGetBssIdxByWlanIdx(IN struct ADAPTER *prAdapter,
			      IN uint8_t ucWlanIdx);

uint8_t secGetBssIdxByRfb(IN struct ADAPTER *prAdapter,
	IN struct SW_RFB *prSwRfb);

uint8_t secLookupStaRecIndexFromTA(struct ADAPTER
				   *prAdapter, uint8_t *pucMacAddress);

void secPrivacyDumpWTBL(IN struct ADAPTER *prAdapter);

u_int8_t secCheckWTBLAssign(IN struct ADAPTER *prAdapter);

u_int8_t secIsProtected1xFrame(IN struct ADAPTER *prAdapter,
			       IN struct STA_RECORD *prStaRec);

u_int8_t secIsProtectedBss(IN struct ADAPTER *prAdapter,
			   IN struct BSS_INFO *prBssInfo);

u_int8_t secIsRobustActionFrame(IN struct ADAPTER *prAdapter,
			   IN void *prPacket);

u_int8_t secIsRobustMgmtFrame(IN struct ADAPTER *prAdapter,
			IN void *prPacket);

u_int8_t secIsWepBss(IN struct ADAPTER *prAdapter,
			IN struct BSS_INFO *prBssInfo);

u_int8_t tkipMicDecapsulate(IN struct SW_RFB *prSwRfb,
			    IN uint8_t *pucMicKey);

u_int8_t tkipMicDecapsulateInRxHdrTransMode(
	IN struct SW_RFB *prSwRfb, IN uint8_t *pucMicKey);

void secPostUpdateAddr(IN struct ADAPTER *prAdapter,
		       IN struct BSS_INFO *prBssInfo);

enum ENUM_EAPOL_KEY_TYPE_T secGetEapolKeyType(
	uint8_t *pucPacket);

void secHandleNoWtbl(IN struct ADAPTER *prAdapter,
	IN struct SW_RFB *prSwRfb);

/*******************************************************************************
 *                              F U N C T I O N S
 *******************************************************************************
 */

#endif /* _PRIVACY_H */
