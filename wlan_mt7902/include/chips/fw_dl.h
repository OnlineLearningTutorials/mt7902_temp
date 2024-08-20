/* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause */
/*
 * Copyright (c) 2016 MediaTek Inc.
 */
/*! \file  fw_dl.h
 */

#ifndef _FW_DL_H
#define _FW_DL_H

/*******************************************************************************
 *                              C O N S T A N T S
 *******************************************************************************
 */

/* PDA - Patch Decryption Accelerator */
#define PDA_N9                 0
#define PDA_CR4                1

#define MAX_FWDL_SECTION_NUM   10
#define N9_FWDL_SECTION_NUM    2
#define CR4_FWDL_SECTION_NUM   HIF_CR4_FWDL_SECTION_NUM
#define IMG_DL_STATUS_PORT_IDX HIF_IMG_DL_STATUS_PORT_IDX

#define DOWNLOAD_CONFIG_ENCRYPTION_MODE     BIT(0)
#define DOWNLOAD_CONFIG_KEY_INDEX_MASK		BITS(1, 2)
#define DOWNLOAD_CONFIG_KEY_INDEX_SHFT		(1)
#define DOWNLOAD_CONFIG_RESET_OPTION        BIT(3)
#define DOWNLOAD_CONFIG_WORKING_PDA_OPTION	BIT(4)
#define DOWNLOAD_CONFIG_VALID_RAM_ENTRY	    BIT(5)
#define DOWNLOAD_CONFIG_ENCRY_MODE_SEL	    BIT(6) /* 0 - AES, 1 - SCRAMBLE */
#define DOWNLOAD_CONFIG_EMI			BIT(7)
#define DOWNLOAD_CONFIG_ACK_OPTION          BIT(31)

/*
 * FW feature set
 * bit(0)  : encrypt or not.
 * bit(1,2): encrypt key index.
 * bit(3)  : compressed image or not. (added in CONNAC)
 * bit(4)  : encrypt mode, 1 for scramble, 0 for AES.
 * bit(5)  : replace RAM code starting address with image
 *           destination address or not. (added in CONNAC)
 * bit(7)  : download to EMI or not. (added in CONNAC)
 */
#define FW_FEATURE_SET_ENCRY	BIT(0)
#define FW_FEATURE_SET_KEY_MASK	BITS(1, 2)
#define GET_FW_FEATURE_SET_KEY(p) (((p) & FW_FEATURE_SET_KEY_MASK) >> 1)
#define FW_FEATURE_COMPRESS_IMG	BIT(3)
#define FW_FEATURE_ENCRY_MODE	BIT(4)
#define FW_FEATURE_OVERRIDE_RAM_ADDR	BIT(5)
#define FW_FEATURE_NOT_DOWNLOAD	BIT(6)
#define FW_FEATURE_DL_TO_EMI	BIT(7)

#if CFG_SUPPORT_COMPRESSION_FW_OPTION
#define COMPRESSION_OPTION_OFFSET   4
#define COMPRESSION_OPTION_MASK     BIT(4)
#endif

#define RELEASE_INFO_SEPARATOR_LEN  16

#if CFG_MTK_ANDROID_EMI
#define WIFI_EMI_ADDR_MASK     0xFFFFFF
extern phys_addr_t gConEmiPhyBase;
extern unsigned long long gConEmiSize;
#endif

/*
 * patch format:
 * PATCH_FORMAT_V1 support 7636, 7637, 7615, 7622, CONNAC (p18, 7663)
 * PATCH_FORMAT_V2 support CONNANC2.0 (7915)
 */
/* Magic number, means use this multi-address patch header format*/
#define PATCH_VERSION_MAGIC_NUM 0xffffffff
#define PATCH_SEC_TYPE_MASK	0x0000ffff
#define PATCH_SEC_TYPE_BIN_INFO	0x2

/*
 * Patch Format v2 SectionSpec3 : Security info
 */
#define PATCH_SECINFO_NOT_SUPPORT		(0xFFFFFFFF)

#define PATCH_SECINFO_ENC_TYPE_MASK		(0xFF000000)
#define PATCH_SECINFO_ENC_TYPE_SHFT		(24)
#define PATCH_SECINFO_ENC_TYPE_PLAIN		(0x00)
#define PATCH_SECINFO_ENC_TYPE_AES		(0x01)
#define PATCH_SECINFO_ENC_TYPE_SCRAMBLE		(0x02)
#define PATCH_SECINFO_ENC_SCRAMBLE_INFO_MASK	(0x0000FFFF)
#define PATCH_SECINFO_ENC_AES_KEY_MASK		(0x000000FF)

/*
 * Patch Format v2 SectionSpec5 : BINARY TYPE
 * BT uses this field to determind each section' type.
 */
#define FW_SECT_BINARY_TYPE_BT_PATCH			0x00000002
#define FW_SECT_BINARY_TYPE_BT_ILM_TEXT_EX9_DATA	0x00000080
#define FW_SECT_BINARY_TYPE_WF_PATCH			0x00000100
#define FW_SECT_BINARY_TYPE_ZB_FW			0x00001000

/* Used for sanity check, if need can modified it. */
#define FW_MAX_SECTION_NUM 1024

enum ENUM_IMG_DL_IDX_T {
	IMG_DL_IDX_N9_FW,
	IMG_DL_IDX_CR4_FW,
	IMG_DL_IDX_PATCH,
	IMG_DL_IDX_BT_PATCH,
	IMG_DL_IDX_ZB_PATCH
};

#if (CFG_UMAC_GENERATION >= 0x20)
#define LEN_4_BYTE_CRC	(4)

struct TAILER_COMMON_FORMAT_T {
	uint8_t ucChipInfo;
	uint8_t ucEcoCode;
	uint8_t ucRegionNum;
	uint8_t ucFormatVer;
	uint8_t ucFormatFlag;
	uint8_t aucReserved[2];
	uint8_t aucRamVersion[10];
	uint8_t aucRamBuiltDate[15];
	uint32_t u4CRC;
};

struct TAILER_REGION_FORMAT_T {
	uint32_t u4CRC;
	uint32_t u4RealSize;
	uint32_t u4BlockSize;
	uint8_t aucReserved1[4];
	uint32_t u4Addr;
	uint32_t u4Len;
	uint8_t ucFeatureSet;
	uint8_t aucReserved2[15];
};

struct TAILER_FORMAT_T {
	uint32_t addr;
	uint8_t chip_info;
	uint8_t feature_set;
	uint8_t eco_code;
	uint8_t ram_version[10];
	uint8_t ram_built_date[15];
	uint32_t len;
};

struct HEADER_RELEASE_INFO {
	uint16_t u2Len;
	uint8_t ucPaddingLen;
	uint8_t ucTag;
};

#if CFG_SUPPORT_COMPRESSION_FW_OPTION
struct TAILER_FORMAT_T_2 {
	uint32_t crc;
	uint32_t addr;
	uint32_t block_size;
	uint32_t real_size;
	uint8_t  chip_info;
	uint8_t  feature_set;
	uint8_t  eco_code;
	uint8_t  ram_version[10];
	uint8_t  ram_built_date[15];
	uint32_t len;
};

struct FW_IMAGE_TAILER_T_2 {
	struct TAILER_FORMAT_T_2 ilm_info;
	struct TAILER_FORMAT_T_2 dlm_info;
};

struct FW_IMAGE_TAILER_CHECK {
	uint8_t	chip_info;
	uint8_t	feature_set;
	uint8_t	eco_code;
	uint8_t	ram_version[10];
	uint8_t	ram_built_date[15];
	uint32_t len;
};
#endif

struct PATCH_FORMAT_T {
	uint8_t aucBuildDate[16];
	uint8_t aucPlatform[4];
	uint32_t u4SwHwVersion;
	uint32_t u4PatchVersion;
	uint16_t u2CRC;		/* CRC calculated for image only */
	uint8_t ucPatchImage[0];
};

struct PATCH_FORMAT_V2_T {
	uint8_t aucBuildDate[16];
	uint8_t aucPlatform[4];
	uint32_t u4SwHwVersion;
	uint32_t u4PatchVersion;
	uint16_t u2Reserved;
	uint16_t u2CRC;		/* CRC calculated for image only */
};

/* multi-addr patch format */
struct PATCH_GLO_DESC {
	uint32_t patch_ver;
	uint32_t subsys;
	uint32_t feature;
	uint32_t section_num;
	uint32_t crc;
	uint32_t reserved[11];
};

struct PATCH_SEC_MAP {
	uint32_t section_type;
	uint32_t section_offset;
	uint32_t section_size;
	union {
		uint32_t section_spec[13];
		struct {
			uint32_t dl_addr;
			uint32_t dl_size;
			uint32_t sec_info;
			uint32_t align_len;
			uint32_t bin_type; /* BT uses this filed as BIN TYPE */
			uint32_t reserved[8];
		} bin_info_spec;
	};
};

struct patch_dl_buf {
	uint8_t *img_ptr;
	uint32_t img_dest_addr;
	uint32_t img_size;
#if CFG_SUPPORT_WIFI_DL_BT_PATCH || CFG_SUPPORT_WIFI_DL_ZB_PATCH
	uint32_t bin_type;
#endif
	bool check_crc;
};

struct patch_dl_target {
	struct patch_dl_buf *patch_region;
	uint8_t num_of_region;
};

#endif

struct FWDL_OPS_T {
	/* load firmware bin priority */
	void (*constructFirmwarePrio)(struct GLUE_INFO *prGlueInfo,
		uint8_t **apucNameTable, uint8_t **apucName,
		uint8_t *pucNameIdx, uint8_t ucMaxNameIdx);
	void (*constructPatchName)(struct GLUE_INFO *prGlueInfo,
		uint8_t **apucName, uint8_t *pucNameIdx);

	uint32_t (*downloadPatch)(IN struct ADAPTER *prAdapter);
	uint32_t (*downloadFirmware)(IN struct ADAPTER *prAdapter,
		IN enum ENUM_IMG_DL_IDX_T eDlIdx);
	uint32_t (*downloadByDynMemMap)(
		IN struct ADAPTER *prAdapter, IN uint32_t u4Addr,
		IN uint32_t u4Len,	IN uint8_t *pucStartPtr,
		IN enum ENUM_IMG_DL_IDX_T eDlIdx);
	void (*getFwInfo)(IN struct ADAPTER *prAdapter,
		IN uint8_t u4SecIdx, IN enum ENUM_IMG_DL_IDX_T eDlIdx,
		OUT uint32_t *pu4Addr, OUT uint32_t *pu4Len,
		OUT uint32_t *pu4DataMode, OUT u_int8_t *pfgIsEMIDownload,
		OUT u_int8_t *pfgIsNotDownload);
	unsigned int (*getFwDlInfo)(struct ADAPTER *prAdapter,
		char *pcBuf, int i4TotalLen);
#if CFG_SUPPORT_WIFI_DL_BT_PATCH
	void (*constructBtPatchName)(struct GLUE_INFO *prGlueInfo,
		uint8_t **apucName, uint8_t *pucNameIdx);
	uint32_t (*downloadBtPatch)(IN struct ADAPTER *prAdapter);
#endif
#if CFG_SUPPORT_WIFI_DL_ZB_PATCH
	void (*constructZbPatchName)(struct GLUE_INFO *prGlueInfo,
		uint8_t **apucName, uint8_t *pucNameIdx);
	uint32_t (*downloadZbPatch)(IN struct ADAPTER *prAdapter);
#endif
uint32_t (*downloadSection)(IN struct ADAPTER *prAdapter,
			     IN uint32_t u4Addr, IN uint32_t u4Len,
			     IN uint32_t u4DataMode, IN uint8_t *pucStartPtr,
			     IN enum ENUM_IMG_DL_IDX_T eDlIdx);
uint32_t (*downloadSectionV2)(IN struct ADAPTER *prAdapter,
		IN uint32_t u4DataMode,
		IN enum ENUM_IMG_DL_IDX_T eDlIdx,
		struct patch_dl_target *target);

};
/*******************************************************************************
 *                  F U N C T I O N   D E C L A R A T I O N S
 *******************************************************************************
 */

#if CFG_ENABLE_FW_DOWNLOAD
uint32_t wlanGetDataMode(IN struct ADAPTER *prAdapter,
	IN enum ENUM_IMG_DL_IDX_T eDlIdx, IN uint8_t ucFeatureSet);

uint32_t wlanGetPatchDataModeV2(IN struct ADAPTER *prAdapter,
	IN uint32_t u4SecInfo);

void wlanGetHarvardFwInfo(IN struct ADAPTER *prAdapter,
	IN uint8_t u4SecIdx, IN enum ENUM_IMG_DL_IDX_T eDlIdx,
	OUT uint32_t *pu4Addr, OUT uint32_t *pu4Len,
	OUT uint32_t *pu4DataMode, OUT u_int8_t *pfgIsEMIDownload,
	OUT u_int8_t *pfgIsNotDownload);

void wlanGetConnacFwInfo(IN struct ADAPTER *prAdapter,
	IN uint8_t u4SecIdx, IN enum ENUM_IMG_DL_IDX_T eDlIdx,
	OUT uint32_t *pu4Addr, OUT uint32_t *pu4Len,
	OUT uint32_t *pu4DataMode, OUT u_int8_t *pfgIsEMIDownload,
	OUT u_int8_t *pfgIsNotDownload);

#if CFG_SUPPORT_COMPRESSION_FW_OPTION
uint32_t wlanCompressedImageSectionDownloadStage(IN struct ADAPTER *prAdapter,
	IN void *pvFwImageMapFile, IN uint32_t u4FwImageFileLength,
	uint8_t ucSectionNumber, IN enum ENUM_IMG_DL_IDX_T eDlIdx,
	OUT uint8_t *ucIsCompressed,
	OUT struct INIT_CMD_WIFI_DECOMPRESSION_START *prFwImageInFo);
#endif
uint32_t wlanImageSectionDownloadStage(IN struct ADAPTER *prAdapter,
	IN void *pvFwImageMapFile,
	IN uint32_t u4FwImageFileLength, IN uint8_t ucSectionNumber,
	IN enum ENUM_IMG_DL_IDX_T eDlIdx);

#if (CFG_DOWNLOAD_DYN_MEMORY_MAP == 1)
uint32_t wlanPatchDynMemMapSendComplete(IN struct ADAPTER *prAdapter);

uint32_t wlanRamCodeDynMemMapSendComplete(IN struct ADAPTER *prAdapter,
	IN u_int8_t fgEnable, IN uint32_t u4StartAddress,
	IN uint8_t ucPDA);
#endif

uint32_t wlanDownloadSection(IN struct ADAPTER *prAdapter,
	IN uint32_t u4Addr, IN uint32_t u4Len,
	IN uint32_t u4DataMode, IN uint8_t *pucStartPtr,
	IN enum ENUM_IMG_DL_IDX_T eDlIdx);

uint32_t wlanDownloadEMISection(IN struct ADAPTER *prAdapter,
	IN uint32_t u4DestAddr,
	IN uint32_t u4Len, IN uint8_t *pucStartPtr);

uint32_t wlanGetHarvardTailerInfo(IN struct ADAPTER *prAdapter,
	IN void *prFwBuffer, IN uint32_t u4FwSize,
	IN uint32_t ucTotSecNum, IN enum ENUM_IMG_DL_IDX_T eDlIdx);

uint32_t wlanGetConnacTailerInfo(IN struct ADAPTER *prAdapter,
	IN void *prFwBuffer,
	IN uint32_t u4FwSize, IN enum ENUM_IMG_DL_IDX_T eDlIdx);

uint32_t wlanImageSectionConfig(IN struct ADAPTER *prAdapter,
	IN uint32_t u4DestAddr, IN uint32_t u4ImgSecSize,
	IN uint32_t u4DataMode,
	IN enum ENUM_IMG_DL_IDX_T eDlIdx);

uint32_t wlanImageSectionDownload(IN struct ADAPTER *prAdapter,
	IN uint32_t u4ImgSecSize, IN uint8_t *pucImgSecBuf);

uint32_t wlanImageQueryStatus(IN struct ADAPTER *prAdapter);

uint32_t wlanConfigWifiFuncStatus(IN struct ADAPTER *prAdapter,
	IN uint8_t ucCmdSeqNum);

uint32_t wlanConfigWifiFunc(IN struct ADAPTER *prAdapter,
	IN u_int8_t fgEnable, IN uint32_t u4StartAddress,
	IN uint8_t ucPDA);

uint32_t wlanCRC32(uint8_t *buf, uint32_t len);

uint32_t wlanDownloadCR4FW(IN struct ADAPTER *prAdapter,
	void *prFwBuffer);

uint32_t wlanDownloadFW(IN struct ADAPTER *prAdapter);

uint32_t wlanDownloadPatch(IN struct ADAPTER *prAdapter);

uint32_t wlanHarvardFormatDownload(IN struct ADAPTER *prAdapter,
	IN enum ENUM_IMG_DL_IDX_T eDlIdx);

uint32_t wlanConnacFormatDownload(IN struct ADAPTER *prAdapter,
	IN enum ENUM_IMG_DL_IDX_T eDlIdx);

uint32_t wlanGetPatchInfo(IN struct ADAPTER *prAdapter);

uint32_t fwDlGetFwdlInfo(struct ADAPTER *prAdapter,
	char *pcBuf, int i4TotalLen);

void fwDlGetReleaseInfoSection(struct ADAPTER *prAdapter, uint8_t *pucStartPtr);
void fwDlGetReleaseManifest(struct ADAPTER *prAdapter,
			    struct HEADER_RELEASE_INFO *prRelInfo,
			    uint8_t *pucStartPtr);

uint32_t wlanDownloadSectionV2(IN struct ADAPTER *prAdapter,
		IN uint32_t u4DataMode,
		IN enum ENUM_IMG_DL_IDX_T eDlIdx,
		struct patch_dl_target *target);
uint32_t wlanPatchSendComplete(IN struct ADAPTER *prAdapter
#if CFG_SUPPORT_WIFI_DL_BT_PATCH || CFG_SUPPORT_WIFI_DL_ZB_PATCH
			       , IN uint8_t ucPatchType
#endif
				);
#endif

#endif /* _FW_DL_H */

