/* SPDX-License-Identifier: ISC */
/* Copyright (C) 2020 MediaTek Inc. */

#ifndef __BESRA_MCU_H
#define __BESRA_MCU_H

#include "../mt76_connac_mcu.h"

struct besra_mcu_txd {
	__le32 txd[8];

	__le16 len;
	__le16 pq_id;

	u8 cid;
	u8 pkt_type;
	u8 set_query; /* FW don't care */
	u8 seq;

	u8 uc_d2b0_rev;
	u8 ext_cid;
	u8 s2d_index;
	u8 ext_cid_ack;

	u32 reserved[5];
} __packed __aligned(4);

/**
 * struct besra_uni_txd - mcu command descriptor for firmware v3
 * @txd: hardware descriptor
 * @len: total length not including txd
 * @cid: command identifier
 * @pkt_type: must be 0xa0 (cmd packet by long format)
 * @frag_n: fragment number
 * @seq: sequence number
 * @checksum: 0 mean there is no checksum
 * @s2d_index: index for command source and destination
 *  Definition              | value | note
 *  CMD_S2D_IDX_H2N         | 0x00  | command from HOST to WM
 *  CMD_S2D_IDX_C2N         | 0x01  | command from WA to WM
 *  CMD_S2D_IDX_H2C         | 0x02  | command from HOST to WA
 *  CMD_S2D_IDX_H2N_AND_H2C | 0x03  | command from HOST to WA and WM
 *
 * @option: command option
 *  BIT[0]: UNI_CMD_OPT_BIT_ACK
 *          set to 1 to request a fw reply
 *          if UNI_CMD_OPT_BIT_0_ACK is set and UNI_CMD_OPT_BIT_2_SET_QUERY
 *          is set, mcu firmware will send response event EID = 0x01
 *          (UNI_EVENT_ID_CMD_RESULT) to the host.
 *  BIT[1]: UNI_CMD_OPT_BIT_UNI_CMD
 *          0: original command
 *          1: unified command
 *  BIT[2]: UNI_CMD_OPT_BIT_SET_QUERY
 *          0: QUERY command
 *          1: SET command
 */
struct besra_uni_txd {
	__le32 txd[8];

	/* DW1 */
	__le16 len;
	__le16 cid;

	/* DW2 */
	u8 reserved;
	u8 pkt_type;
	u8 frag_n;
	u8 seq;

	/* DW3 */
	__le16 checksum;
	u8 s2d_index;
	u8 option;

	/* DW4 */
	u8 reserved2[4];
} __packed __aligned(4);

enum {
	MCU_ATE_SET_TRX = 0x1,
	MCU_ATE_SET_FREQ_OFFSET = 0xa,
	MCU_ATE_SET_SLOT_TIME = 0x13,
	MCU_ATE_CLEAN_TXQUEUE = 0x1c,
};

struct besra_mcu_rxd {
	__le32 rxd[8];

	__le16 len;
	__le16 pkt_type_id;

	u8 eid;
	u8 seq;
	u8 option;
	u8 __rsv;

	u8 ext_eid;
	u8 __rsv1[2];
	u8 s2d_index;
};

struct besra_mcu_uni_event {
	u8 cid;
	u8 __rsv[3];
	__le32 status; /* 0: success, others: fail */
} __packed;

struct besra_mcu_thermal_ctrl {
	u8 ctrl_id;
	u8 band_idx;
	union {
		struct {
			u8 protect_type; /* 1: duty admit, 2: radio off */
			u8 trigger_type; /* 0: low, 1: high */
		} __packed type;
		struct {
			u8 duty_level;	/* level 0~3 */
			u8 duty_cycle;
		} __packed duty;
	};
} __packed;

struct besra_mcu_thermal_notify {
	struct besra_mcu_rxd rxd;

	struct besra_mcu_thermal_ctrl ctrl;
	__le32 temperature;
	u8 rsv[8];
} __packed;

struct besra_mcu_csa_notify {
	struct besra_mcu_rxd rxd;

	u8 omac_idx;
	u8 csa_count;
	u8 band_idx;
	u8 rsv;
} __packed;

struct besra_mcu_rdd_report {
	struct besra_mcu_rxd rxd;

	u8 band_idx;
	u8 long_detected;
	u8 constant_prf_detected;
	u8 staggered_prf_detected;
	u8 radar_type_idx;
	u8 periodic_pulse_num;
	u8 long_pulse_num;
	u8 hw_pulse_num;

	u8 out_lpn;
	u8 out_spn;
	u8 out_crpn;
	u8 out_crpw;
	u8 out_crbn;
	u8 out_stgpn;
	u8 out_stgpw;

	u8 rsv;

	__le32 out_pri_const;
	__le32 out_pri_stg[3];

	struct {
		__le32 start;
		__le16 pulse_width;
		__le16 pulse_power;
		u8 mdrdy_flag;
		u8 rsv[3];
	} long_pulse[32];

	struct {
		__le32 start;
		__le16 pulse_width;
		__le16 pulse_power;
		u8 mdrdy_flag;
		u8 rsv[3];
	} periodic_pulse[32];

	struct {
		__le32 start;
		__le16 pulse_width;
		__le16 pulse_power;
		u8 sc_pass;
		u8 sw_reset;
		u8 mdrdy_flag;
		u8 tx_active;
	} hw_pulse[32];
} __packed;

struct besra_mcu_background_chain_ctrl {
	u8 chan;		/* primary channel */
	u8 central_chan;	/* central channel */
	u8 bw;
	u8 tx_stream;
	u8 rx_stream;

	u8 monitor_chan;	/* monitor channel */
	u8 monitor_central_chan;/* monitor central channel */
	u8 monitor_bw;
	u8 monitor_tx_stream;
	u8 monitor_rx_stream;

	u8 scan_mode;		/* 0: ScanStop
				 * 1: ScanStart
				 * 2: ScanRunning
				 */
	u8 band_idx;		/* DBDC */
	u8 monitor_scan_type;
	u8 band;		/* 0: 2.4GHz, 1: 5GHz */
	u8 rsv[2];
} __packed;

struct besra_mcu_eeprom {
	u8 _rsv[4];

	__le16 tag;
	__le16 len;
	u8 buffer_mode;
	u8 format;
	__le16 buf_len;
} __packed;

struct besra_mcu_eeprom_info {
	__le32 addr;
	__le32 valid;
	u8 data[16];
} __packed;

struct besra_mcu_phy_rx_info {
	u8 category;
	u8 rate;
	u8 mode;
	u8 nsts;
	u8 gi;
	u8 coding;
	u8 stbc;
	u8 bw;
};

struct besra_mcu_mib {
	__le16 tag;
	__le16 len;
	__le32 offs;
	__le64 data;
} __packed;

enum besra_chan_mib_offs {
	/* besra */
	MIB_BUSY_TIME = 0,
	MIB_TX_TIME = 6,
	MIB_RX_TIME = 8,
	MIB_OBSS_AIRTIME = 499,
};

struct edca {
	__le16 tag;
	__le16 len;

	u8 queue;
	u8 set;
	u8 cw_min;
	u8 cw_max;
	__le16 txop;
	u8 aifs;
	u8 __rsv;
};

struct besra_mcu_muru_stats {
	__le32 event_id;
	struct {
		__le32 cck_cnt;
		__le32 ofdm_cnt;
		__le32 htmix_cnt;
		__le32 htgf_cnt;
		__le32 vht_su_cnt;
		__le32 vht_2mu_cnt;
		__le32 vht_3mu_cnt;
		__le32 vht_4mu_cnt;
		__le32 he_su_cnt;
		__le32 he_ext_su_cnt;
		__le32 he_2ru_cnt;
		__le32 he_2mu_cnt;
		__le32 he_3ru_cnt;
		__le32 he_3mu_cnt;
		__le32 he_4ru_cnt;
		__le32 he_4mu_cnt;
		__le32 he_5to8ru_cnt;
		__le32 he_9to16ru_cnt;
		__le32 he_gtr16ru_cnt;
	} dl;

	struct {
		__le32 hetrig_su_cnt;
		__le32 hetrig_2ru_cnt;
		__le32 hetrig_3ru_cnt;
		__le32 hetrig_4ru_cnt;
		__le32 hetrig_5to8ru_cnt;
		__le32 hetrig_9to16ru_cnt;
		__le32 hetrig_gtr16ru_cnt;
		__le32 hetrig_2mu_cnt;
		__le32 hetrig_3mu_cnt;
		__le32 hetrig_4mu_cnt;
	} ul;
};

#define WMM_AIFS_SET		BIT(0)
#define WMM_CW_MIN_SET		BIT(1)
#define WMM_CW_MAX_SET		BIT(2)
#define WMM_TXOP_SET		BIT(3)
#define WMM_PARAM_SET		GENMASK(3, 0)

#define MCU_PQ_ID(p, q)			(((p) << 15) | ((q) << 10))
#define MCU_PKT_ID			0xa0

enum {
	MCU_FW_LOG_WM,
	MCU_FW_LOG_WA,
	MCU_FW_LOG_TO_HOST,
	MCU_FW_LOG_RELAY = 16
};

enum {
	MCU_TWT_AGRT_ADD,
	MCU_TWT_AGRT_MODIFY,
	MCU_TWT_AGRT_DELETE,
	MCU_TWT_AGRT_TEARDOWN,
	MCU_TWT_AGRT_GET_TSF,
};

enum {
	MCU_WA_PARAM_CMD_QUERY,
	MCU_WA_PARAM_CMD_SET,
	MCU_WA_PARAM_CMD_CAPABILITY,
	MCU_WA_PARAM_CMD_DEBUG,
};

enum {
	MCU_WA_PARAM_PDMA_RX = 0x04,
	MCU_WA_PARAM_CPU_UTIL = 0x0b,
	MCU_WA_PARAM_RED = 0x0e,
};

enum mcu_mmps_mode {
	MCU_MMPS_STATIC,
	MCU_MMPS_DYNAMIC,
	MCU_MMPS_RSV,
	MCU_MMPS_DISABLE,
};

struct bss_rate_tlv {
	__le16 tag;
	__le16 len;
	u8 __rsv1[4];
	__le16 bc_trans;
	__le16 mc_trans;
	u8 short_preamble;
	u8 bc_fixed_rate;
	u8 mc_fixed_rate;
	u8 __rsv2[1];
} __packed;

struct bss_ra_tlv {
	__le16 tag;
	__le16 len;
	u8 short_preamble;
	u8 force_sgi;
	u8 force_gf;
	u8 ht_mode;
	u8 se_off;
	u8 antenna_idx;
	__le16 max_phyrate;
	u8 force_tx_streams;
	u8 __rsv[3];
} __packed;

struct bss_rlm_tlv {
	__le16 tag;
	__le16 len;
	u8 control_channel;
	u8 center_chan;
	u8 center_chan2;
	u8 bw;
	u8 tx_streams;
	u8 rx_streams;
	u8 ht_op_info;
	u8 sco;
	u8 band;
	u8 __rsv[3];
} __packed;

struct bss_color_tlv {
	__le16 tag;
	__le16 len;
	u8 enable;
	u8 color;
	u8 rsv[2];
} __packed;

#define MAX_BEACON_SIZE 512
struct bss_bcn_content_tlv {
	__le16 tag;
	__le16 len;
	__le16 tim_ie_pos;
	__le16 csa_ie_pos;
	__le16 bcc_ie_pos;
	u8 enable;
	u8 type;
	__le16 pkt_len;
	u8 pkt[MAX_BEACON_SIZE];
} __packed;

struct bss_bcn_cntdwn_tlv {
	__le16 tag;
	__le16 len;
	u8 cnt;
	u8 rsv[3];
} __packed;

struct bss_bcn_mbss_tlv {
#define MAX_BEACON_NUM	32
	__le16 tag;
	__le16 len;
	__le32 bitmap;
	__le16 offset[MAX_BEACON_NUM];
} __packed __aligned(4);

struct bss_txcmd_tlv {
	__le16 tag;
	__le16 len;
	u8 txcmd_mode;
	u8 __rsv[3];
} __packed;

struct bss_sec_tlv {
	__le16 tag;
	__le16 len;
	u8 __rsv1[2];
	u8 cipher;
	u8 __rsv2[1];
} __packed;

struct bss_power_save {
	__le16 tag;
	__le16 len;
	u8 profile;
	u8 _rsv[3];
} __packed;

struct bss_mld_tlv {
	__le16 tag;
	__le16 len;
	u8 group_mld_id;
	u8 own_mld_id;
	u8 mac_addr[ETH_ALEN];
	u8 remap_idx;
	u8 __rsv[3];
} __packed;

struct hdr_trans_en {
	__le16 tag;
	__le16 len;
	u8 enable;
	u8 check_bssid;
	u8 mode;
	u8 __rsv;
} __packed;

struct hdr_trans_vlan {
	__le16 tag;
	__le16 len;
	u8 insert_vlan;
	u8 remove_vlan;
	u8 tid;
	u8 __rsv;
} __packed;

struct hdr_trans_blacklist {
	__le16 tag;
	__le16 len;
	u8 idx;
	u8 enable;
	__le16 type;
} __packed;

#define BESRA_HDR_TRANS_MAX_SIZE	(sizeof(struct hdr_trans_en) + \
					 sizeof(struct hdr_trans_vlan) + \
					 sizeof(struct hdr_trans_blacklist))

enum {
	UNI_HDR_TRANS_EN,
	UNI_HDR_TRANS_VLAN,
	UNI_HDR_TRANS_BLACKLIST,
};

enum {
	RATE_PARAM_FIXED = 3,
	RATE_PARAM_MMPS_UPDATE = 5,
	RATE_PARAM_FIXED_HE_LTF = 7,
	RATE_PARAM_FIXED_MCS,
	RATE_PARAM_FIXED_GI = 11,
	RATE_PARAM_AUTO = 20,
};

#define RATE_CFG_MCS			GENMASK(3, 0)
#define RATE_CFG_NSS			GENMASK(7, 4)
#define RATE_CFG_GI			GENMASK(11, 8)
#define RATE_CFG_BW			GENMASK(15, 12)
#define RATE_CFG_STBC			GENMASK(19, 16)
#define RATE_CFG_LDPC			GENMASK(23, 20)
#define RATE_CFG_PHY_TYPE		GENMASK(27, 24)
#define RATE_CFG_HE_LTF			GENMASK(31, 28)

enum {
	THERMAL_PROTECT_PARAMETER_CTRL,
	THERMAL_PROTECT_BASIC_INFO,
	THERMAL_PROTECT_ENABLE,
	THERMAL_PROTECT_DISABLE,
	THERMAL_PROTECT_DUTY_CONFIG,
	THERMAL_PROTECT_MECH_INFO,
	THERMAL_PROTECT_DUTY_INFO,
	THERMAL_PROTECT_STATE_ACT,
};

enum {
	MT_BF_SOUNDING_ON = 1,
	MT_BF_TYPE_UPDATE = 20,
	MT_BF_MODULE_UPDATE = 25
};

enum {
	MURU_SET_ARB_OP_MODE = 14,
	MURU_SET_PLATFORM_TYPE = 25,
};

enum {
	MURU_PLATFORM_TYPE_PERF_LEVEL_1 = 1,
	MURU_PLATFORM_TYPE_PERF_LEVEL_2,
};

/* tx cmd tx statistics */
enum {
	MURU_SET_TXC_TX_STATS_EN = 150,
	MURU_GET_TXC_TX_STATS = 151,
};

enum {
	CMD_BAND_NONE,
	CMD_BAND_24G,
	CMD_BAND_5G,
	CMD_BAND_6G,
};

struct bss_req_hdr {
	u8 bss_idx;
	u8 __rsv[3];
} __packed;

enum {
	UNI_CHANNEL_SWITCH,
	UNI_CHANNEL_RX_PATH,
};
#define BESRA_BSS_UPDATE_MAX_SIZE	(sizeof(struct bss_req_hdr) +	\
					 sizeof(struct mt76_connac_bss_basic_tlv) +	\
					 sizeof(struct bss_rlm_tlv) +\
					 sizeof(struct bss_ra_tlv) + \
					 sizeof(struct bss_info_uni_he) +	\
					 sizeof(struct bss_rate_tlv) +\
					 sizeof(struct bss_txcmd_tlv) +\
					 sizeof(struct bss_power_save) +\
					 sizeof(struct bss_sec_tlv) +\
					 sizeof(struct bss_mld_tlv))

#define BESRA_BEACON_UPDATE_SIZE	(sizeof(struct bss_req_hdr) +	\
					 sizeof(struct bss_bcn_content_tlv) + \
					 sizeof(struct bss_bcn_cntdwn_tlv) + \
					 sizeof(struct bss_bcn_mbss_tlv))

enum {
	UNI_BAND_CONFIG_RADIO_ENABLE,
	UNI_BAND_CONFIG_EDCCA_ENABLE = 0x5,
	UNI_BAND_CONFIG_EDCCA_THRESHOLD = 0x6,
	UNI_BAND_CONFIG_RTS_THRESHOLD = 0x8,
};

enum {
	UNI_WSYS_CONFIG_FW_LOG_CTRL,
	UNI_WSYS_CONFIG_FW_DBG_CTRL,
};

enum {
	UNI_RDD_CTRL_PARM,
};

enum {
	UNI_TXPOWER_SHOW_INFO = 0x7,
};

enum {
	UNI_EFUSE_ACCESS = 1,
	UNI_EFUSE_BUFFER_MODE,
	UNI_EFUSE_FREE_BLOCK,
	UNI_EFUSE_BUFFER_RD,
};

enum {
	UNI_VOW_DRR_CTRL,
	UNI_VOW_FEATURE_CTRL,
	UNI_VOW_BSSGROUP_CTRL_1_GROUP,
	UNI_VOW_BSSGROUP_TOKEN_CFG,
	UNI_VOW_BSSGROUP_CTRL_ALL_GROUP,
	UNI_VOW_BSSGROUP_BW_GROUP_QUANTUM,
	UNI_VOW_BSSGROUP_BW_GROUP_QUANTUM_ALL,
	UNI_VOW_AT_PROC_EST_FEATURE,
	UNI_VOW_AT_PROC_EST_MONITOR_PERIOD,
	UNI_VOW_AT_PROC_EST_GROUP_RATIO,
	UNI_VOW_AT_PROC_EST_GROUP_TO_BAND_MAPPING,
	UNI_VOW_RX_AT_AIRTIME_EN,
	UNI_VOW_RX_AT_MIBTIME_EN,
	UNI_VOW_RX_AT_EARLYEND_EN,
	UNI_VOW_RX_AT_AIRTIME_CLR_EN,
	UNI_VOW_RX_AT_STA_WMM_CTRL,
	UNI_VOW_RX_AT_MBSS_WMM_CTRL,
	UNI_VOW_RX_AT_ED_OFFSET,
	UNI_VOW_RX_AT_SW_TIMER,
	UNI_VOW_RX_AT_BACKOFF_TIMER,
	UNI_VOW_RX_AT_REPORT_RX_NONWIFI_TIME,
	UNI_VOW_RX_AT_REPORT_RX_OBSS_TIME,
	UNI_VOW_RX_AT_REPORT_MIB_OBSS_TIME,
	UNI_VOW_RX_AT_REPORT_PER_STA_RX_TIME,
	UNI_VOW_RED_ENABLE,
	UNI_VOW_RED_TX_RPT,
};

enum {
	UNI_CMD_MIB_DATA,
};

#endif
