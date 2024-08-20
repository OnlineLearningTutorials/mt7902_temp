/* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause */
/*
 * Copyright (c) 2016 MediaTek Inc.
 */
/*
 * Id: include/mgmt/rsn.h#1
 */

/*! \file   rsn.h
 *  \brief  The wpa/rsn related define, macro and structure are described here.
 */

#ifndef _RSN_H
#define _RSN_H

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
/* ----- Definitions for Cipher Suite Selectors ----- */
#define RSN_CIPHER_SUITE_USE_GROUP_KEY  0x00AC0F00
#define RSN_CIPHER_SUITE_WEP40          0x01AC0F00
#define RSN_CIPHER_SUITE_TKIP           0x02AC0F00
#define RSN_CIPHER_SUITE_CCMP           0x04AC0F00
#define RSN_CIPHER_SUITE_WEP104         0x05AC0F00
#if CFG_SUPPORT_802_11W
#define RSN_CIPHER_SUITE_AES_128_CMAC   0x06AC0F00
#endif
#define RSN_CIPHER_SUITE_GROUP_NOT_USED 0x07AC0F00
#define RSN_CIPHER_SUITE_GCMP           0x08AC0F00
#define RSN_CIPHER_SUITE_GCMP_256       0x09AC0F00
#define RSN_CIPHER_SUITE_CCMP_256       0x0AAC0F00
#define RSN_CIPHER_SUITE_BIP_GMAC_128   0x0BAC0F00
#define RSN_CIPHER_SUITE_BIP_GMAC_256   0x0CAC0F00
#define RSN_CIPHER_SUITE_BIP_CMAC_256   0x0DAC0F00

#define WPA_CIPHER_SUITE_NONE           0x00F25000
#define WPA_CIPHER_SUITE_WEP40          0x01F25000
#define WPA_CIPHER_SUITE_TKIP           0x02F25000
#define WPA_CIPHER_SUITE_CCMP           0x04F25000
#define WPA_CIPHER_SUITE_WEP104         0x05F25000

/* Definitions for Authentication and Key Management Suite Selectors */
#define RSN_AKM_SUITE_NONE              0x00AC0F00
#define RSN_AKM_SUITE_802_1X            0x01AC0F00
#define RSN_AKM_SUITE_PSK               0x02AC0F00
#define RSN_AKM_SUITE_FT_802_1X         0x03AC0F00
#define RSN_AKM_SUITE_FT_PSK            0x04AC0F00
#if KERNEL_VERSION(4, 12, 0) > CFG80211_VERSION_CODE
#define WLAN_AKM_SUITE_FT_8021X         0x000FAC03
#define WLAN_AKM_SUITE_FT_PSK           0x000FAC04
#endif
#if CFG_SUPPORT_802_11W
#define RSN_AKM_SUITE_802_1X_SHA256     0x05AC0F00
#define RSN_AKM_SUITE_PSK_SHA256        0x06AC0F00
#endif
#define RSN_AKM_SUITE_TDLS              0x07AC0F00
#define RSN_AKM_SUITE_SAE               0x08AC0F00
#define RSN_AKM_SUITE_FT_OVER_SAE       0x09AC0F00
#define RSN_AKM_SUITE_8021X_SUITE_B     0x0BAC0F00
#define RSN_AKM_SUITE_8021X_SUITE_B_192 0x0CAC0F00
#define RSN_AKM_SUITE_FILS_SHA256       0x0EAC0F00
#define RSN_AKM_SUITE_FILS_SHA384       0x0FAC0F00
#define RSN_AKM_SUITE_FT_FILS_SHA256    0x10AC0F00
#define RSN_AKM_SUITE_FT_FILS_SHA384    0x11AC0F00
#define RSN_AKM_SUITE_OWE               0x12AC0F00

#define RSN_AKM_SUITE_SAE               0x08AC0F00
#define RSN_AKM_SUITE_8021X_SUITE_B     0x0BAC0F00
#define RSN_AKM_SUITE_8021X_SUITE_B_192 0x0CAC0F00
#define RSN_AKM_SUITE_OWE               0x12AC0F00
#define RSN_AKM_SUITE_DPP               0x029A6F50

#define WPA_AKM_SUITE_NONE              0x00F25000
#define WPA_AKM_SUITE_802_1X            0x01F25000
#define WPA_AKM_SUITE_PSK               0x02F25000

#define WFA_AKM_SUITE_OSEN              0x019A6F50
/* this define should be in ieee80211.h, but kernel didn't update it.
 * so we define here temporary
 */
#define WLAN_AKM_SUITE_OSEN             0x506f9a01
#define WLAN_AKM_SUITE_DPP              0x506F9A02


#define WLAN_CIPHER_SUITE_NO_GROUP_ADDR 0x000fac07

/* The RSN IE len for associate request */
#define ELEM_ID_RSN_LEN_FIXED           20

/* The WPA IE len for associate request */
#define ELEM_ID_WPA_LEN_FIXED           22

#define MASK_RSNIE_CAP_PREAUTH          BIT(0)

#define GET_SELECTOR_TYPE(x)           ((uint8_t)(((x) >> 24) & 0x000000FF))
#define SET_SELECTOR_TYPE(x, y)		(x = (((x) & 0x00FFFFFF) | \
					(((uint32_t)(y) << 24) & 0xFF000000)))

#define AUTH_CIPHER_CCMP                0x00000008

/* Cihpher suite flags */
#define CIPHER_FLAG_NONE                        0x00000000
#define CIPHER_FLAG_WEP40                       0x00000001	/* BIT 1 */
#define CIPHER_FLAG_TKIP                        0x00000002	/* BIT 2 */
#define CIPHER_FLAG_CCMP                        0x00000008	/* BIT 3 */
#define CIPHER_FLAG_WEP104                      0x00000010	/* BIT 4 */
#define CIPHER_FLAG_WEP128                      0x00000020	/* BIT 5 */
#define CIPHER_FLAG_GCMP256                     0x00000080	/* BIT 7 */

#define TKIP_COUNTERMEASURE_SEC                 60	/* seconds */

#if CFG_SUPPORT_802_11W
#define RSN_AUTH_MFP_DISABLED   0	/* MFP disabled */
#define RSN_AUTH_MFP_OPTIONAL   1	/* MFP optional */
#define RSN_AUTH_MFP_REQUIRED   2	/* MFP required */
#endif

#define GTK_REKEY_CMD_MODE_OFFLOAD_ON		0
#define GTK_REKEY_CMD_MODE_OFLOAD_OFF		1
#define GTK_REKEY_CMD_MODE_SET_BCMC_PN		2
#define GTK_REKEY_CMD_MODE_GET_BCMC_PN		3
#define GTK_REKEY_CMD_MODE_RPY_OFFLOAD_ON	4
#define GTK_REKEY_CMD_MODE_RPY_OFFLOAD_OFF	5

#define SA_QUERY_RETRY_TIMEOUT	3000
#define SA_QUERY_TIMEOUT	501

/*******************************************************************************
 *                             D A T A   T Y P E S
 *******************************************************************************
 */

/* Flags for PMKID Candidate list structure */
#define EVENT_PMKID_CANDIDATE_PREAUTH_ENABLED   0x01

#define CONTROL_FLAG_UC_MGMT_NO_ENC             BIT(5)

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
#define RSN_IE(fp)              ((struct RSN_INFO_ELEM *) fp)
#define WPA_IE(fp)              ((struct WPA_INFO_ELEM *) fp)

#define ELEM_MAX_LEN_ASSOC_RSP_WSC_IE          (32 - ELEM_HDR_LEN)
#define ELEM_MAX_LEN_TIMEOUT_IE          (5)

/*******************************************************************************
 *                  F U N C T I O N   D E C L A R A T I O N S
 *******************************************************************************
 */
u_int8_t rsnParseRsnIE(IN struct ADAPTER *prAdapter,
		       IN struct RSN_INFO_ELEM *prInfoElem,
		       OUT struct RSN_INFO *prRsnInfo);

u_int8_t rsnParseWpaIE(IN struct ADAPTER *prAdapter,
		       IN struct WPA_INFO_ELEM *prInfoElem,
		       OUT struct RSN_INFO *prWpaInfo);

u_int8_t rsnSearchSupportedCipher(IN struct ADAPTER
				  *prAdapter,
				  IN uint32_t u4Cipher, OUT uint32_t *pu4Index,
				  IN uint8_t ucBssIndex);

u_int8_t rsnIsSuitableBSS(IN struct ADAPTER *prAdapter,
			  IN struct RSN_INFO *prBssRsnInfo,
			  IN uint8_t ucBssIndex);

u_int8_t rsnSearchAKMSuite(IN struct ADAPTER *prAdapter,
			   IN uint32_t u4AkmSuite, OUT uint32_t *pu4Index,
			   IN uint8_t ucBssIndex);

u_int8_t rsnPerformPolicySelection(IN struct ADAPTER
				   *prAdapter,
				   IN struct BSS_DESC *prBss,
				   IN uint8_t ucBssIndex);

void rsnGenerateWpaNoneIE(IN struct ADAPTER *prAdapter,
			  IN struct MSDU_INFO *prMsduInfo);

void rsnGenerateWPAIE(IN struct ADAPTER *prAdapter,
		      IN struct MSDU_INFO *prMsduInfo);

void rsnGenerateRSNIE(IN struct ADAPTER *prAdapter,
		      IN struct MSDU_INFO *prMsduInfo);

u_int8_t
rsnParseCheckForWFAInfoElem(IN struct ADAPTER *prAdapter,
			    IN uint8_t *pucBuf, OUT uint8_t *pucOuiType,
			    OUT uint16_t *pu2SubTypeVersion);

#if CFG_SUPPORT_AAA
void rsnParserCheckForRSNCCMPPSK(struct ADAPTER *prAdapter,
				 struct RSN_INFO_ELEM *prIe,
				 struct STA_RECORD *prStaRec,
				 uint16_t *pu2StatusCode);
#endif

void rsnTkipHandleMICFailure(IN struct ADAPTER *prAdapter,
			     IN struct STA_RECORD *prSta,
			     IN u_int8_t fgErrorKeyType);

struct PMKID_ENTRY *rsnSearchPmkidEntry(IN struct ADAPTER *prAdapter,
					IN uint8_t *pucBssid,
					IN uint8_t ucBssIndex);

void rsnCheckPmkidCache(IN struct ADAPTER *prAdapter,
			IN struct BSS_DESC *prBss,
			IN uint8_t ucBssIndex);

void rsnGeneratePmkidIndication(IN struct ADAPTER *prAdapter,
				IN struct PARAM_PMKID_CANDIDATE *prCandi,
				IN uint8_t ucBssIndex);

uint32_t rsnSetPmkid(IN struct ADAPTER *prAdapter,
		     IN struct PARAM_PMKID *prPmkid);

uint32_t rsnDelPmkid(IN struct ADAPTER *prAdapter,
		     IN struct PARAM_PMKID *prPmkid);

uint32_t rsnFlushPmkid(IN struct ADAPTER *prAdapter,
	IN uint8_t ucBssIndex);

#if CFG_SUPPORT_WPS2
void rsnGenerateWSCIE(IN struct ADAPTER *prAdapter,
		      IN struct MSDU_INFO *prMsduInfo);
#endif

#if CFG_SUPPORT_802_11W
uint32_t rsnCheckBipKeyInstalled(IN struct ADAPTER
				 *prAdapter, IN struct STA_RECORD *prStaRec);

uint8_t rsnCheckSaQueryTimeout(
	IN struct ADAPTER *prAdapter, IN uint8_t ucBssIdx);

void rsnStartSaQueryTimer(IN struct ADAPTER *prAdapter,
			  IN unsigned long ulParamPtr);

void rsnStartSaQuery(IN struct ADAPTER *prAdapter,
	IN uint8_t ucBssIdx);

void rsnStopSaQuery(IN struct ADAPTER *prAdapter,
	IN uint8_t ucBssIdx);

void rsnSaQueryRequest(IN struct ADAPTER *prAdapter,
		       IN struct SW_RFB *prSwRfb);

void rsnSaQueryAction(IN struct ADAPTER *prAdapter,
		      IN struct SW_RFB *prSwRfb);

uint16_t rsnPmfCapableValidation(IN struct ADAPTER
				 *prAdapter, IN struct BSS_INFO *prBssInfo,
				 IN struct STA_RECORD *prStaRec);

void rsnPmfGenerateTimeoutIE(struct ADAPTER *prAdapter,
			     struct MSDU_INFO *prMsduInfo);

void rsnApStartSaQuery(IN struct ADAPTER *prAdapter,
		       IN struct STA_RECORD *prStaRec);

void rsnApSaQueryAction(IN struct ADAPTER *prAdapter,
			IN struct SW_RFB *prSwRfb);

#endif /* CFG_SUPPORT_802_11W */

#if CFG_SUPPORT_AAA
void rsnGenerateWSCIEForAssocRsp(struct ADAPTER *prAdapter,
				 struct MSDU_INFO *prMsduInfo);
#endif

u_int8_t rsnParseOsenIE(struct ADAPTER *prAdapter,
			struct IE_WFA_OSEN *prInfoElem,
			struct RSN_INFO *prOsenInfo);

u_int8_t rsnCheckSecurityModeChanged(struct ADAPTER
				     *prAdapter, struct BSS_INFO *prBssInfo,
				     struct BSS_DESC *prBssDesc);

uint32_t rsnCalculateFTIELen(struct ADAPTER *prAdapter, uint8_t ucBssIdx,
			     struct STA_RECORD *prStaRec);

void rsnGenerateFTIE(IN struct ADAPTER *prAdapter,
		     IN OUT struct MSDU_INFO *prMsduInfo);
#if CFG_SUPPORT_OWE
void rsnGenerateOWEIE(IN struct ADAPTER *prAdapter,
		      IN OUT struct MSDU_INFO *prMsduInfo);

uint32_t rsnCalOweIELen(IN struct ADAPTER *prAdapter,
	IN uint8_t ucBssIndex, struct STA_RECORD *prStaRec);

#endif

#if CFG_SUPPORT_WPA3_H2E
void rsnGenerateRSNXE(IN struct ADAPTER *prAdapter,
	IN OUT struct MSDU_INFO *prMsduInfo);
uint32_t rsnCalRSNXELen(IN struct ADAPTER *prAdapter,
	IN uint8_t ucBssIndex, struct STA_RECORD *prStaRec);
#endif

/*******************************************************************************
 *                              F U N C T I O N S
 *******************************************************************************
 */

#endif /* _RSN_H */
