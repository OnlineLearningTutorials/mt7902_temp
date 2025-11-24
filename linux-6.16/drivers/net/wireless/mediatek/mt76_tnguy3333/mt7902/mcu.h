/* SPDX-License-Identifier: ISC */
/* Copyright (C) 2020 MediaTek Inc. */

#ifndef __MT7902_MCU_H
#define __MT7902_MCU_H

#include "mt76_connac_mcu.h"

struct mt7902_mcu_tx_done_event {
	u8 pid;
	u8 status;
	__le16 seq;

	u8 wlan_idx;
	u8 tx_cnt;
	__le16 tx_rate;

	u8 flag;
	u8 tid;
	u8 rsp_rate;
	u8 mcs;

	u8 bw;
	u8 tx_pwr;
	u8 reason;
	u8 rsv0[1];

	__le32 delay;
	__le32 timestamp;
	__le32 applied_flag;
	u8 txs[28];

	u8 rsv1[32];
} __packed;

/* ext event table */
enum {
	MCU_EXT_EVENT_RATE_REPORT = 0x87,
};

struct mt7902_mcu_eeprom_info {
	__le32 addr;
	__le32 valid;
	u8 data[MT7902_EEPROM_BLOCK_SIZE];
} __packed;

#define MT_RA_RATE_NSS			GENMASK(8, 6)
#define MT_RA_RATE_MCS			GENMASK(3, 0)
#define MT_RA_RATE_TX_MODE		GENMASK(12, 9)
#define MT_RA_RATE_DCM_EN		BIT(4)
#define MT_RA_RATE_BW			GENMASK(14, 13)

enum {
	MT_EBF = BIT(0),	/* explicit beamforming */
	MT_IBF = BIT(1)		/* implicit beamforming */
};

struct mt7902_mcu_ant_id_config {
	u8 ant_id[4];
} __packed;

struct mt7902_txpwr_req {
	u8 ver;
	u8 action;
	__le16 len;
	u8 dbdc_idx;
	u8 rsv[3];
} __packed;

struct mt7902_txpwr_event {
	u8 ver;
	u8 action;
	__le16 len;
	struct mt7902_txpwr txpwr;
} __packed;

struct mt7902_wf_rf_pin_ctrl_event {
	u8 result;
	u8 value;
} __packed;

enum {
	TM_SWITCH_MODE,
	TM_SET_AT_CMD,
	TM_QUERY_AT_CMD,
};

enum {
	MT7902_TM_NORMAL,
	MT7902_TM_TESTMODE,
	MT7902_TM_ICAP,
	MT7902_TM_ICAP_OVERLAP,
	MT7902_TM_WIFISPECTRUM,
};

struct mt7902_rftest_cmd {
	u8 action;
	u8 rsv[3];
	__le32 param0;
	__le32 param1;
} __packed;

struct mt7902_rftest_evt {
	__le32 param0;
	__le32 param1;
} __packed;

struct mt7902_clc_info_tlv {
	__le16 tag;
	__le16 len;

	u8 chan_conf; /* BIT(0) : Enable UNII-4
		       * BIT(1) : Enable UNII-5
		       * BIT(2) : Enable UNII-6
		       * BIT(3) : Enable UNII-7
		       * BIT(4) : Enable UNII-8
		       */
	u8 rsv[63];
} __packed;


struct bss_info_bmc_rate {
	__le16 tag;
	__le16 len;
	__le16 bc_trans;
	__le16 mc_trans;
	u8 short_preamble;
	u8 rsv[7];
} __packed;

struct bss_info_ra {
	__le16 tag;
	__le16 len;
	u8 op_mode;
	u8 adhoc_en;
	u8 short_preamble;
	u8 tx_streams;
	u8 rx_streams;
	u8 algo;
	u8 force_sgi;
	u8 force_gf;
	u8 ht_mode;
	u8 has_20_sta;		/* Check if any sta support GF. */
	u8 bss_width_trigger_events;
	u8 vht_nss_cap;
	u8 vht_bw_signal;	/* not use */
	u8 vht_force_sgi;	/* not use */
	u8 se_off;
	u8 antenna_idx;
	u8 train_up_rule;
	u8 rsv[3];
	unsigned short train_up_high_thres;
	short train_up_rule_rssi;
	unsigned short low_traffic_thres;
	__le16 max_phyrate;
	__le32 phy_cap;
	__le32 interval;
	__le32 fast_interval;
} __packed;

struct bss_info_hw_amsdu {
	__le16 tag;
	__le16 len;
	__le32 cmp_bitmap_0;
	__le32 cmp_bitmap_1;
	__le16 trig_thres;
	u8 enable;
	u8 rsv;
} __packed;

struct bss_info_he {
	__le16 tag;
	__le16 len;
	u8 he_pe_duration;
	u8 vht_op_info_present;
	__le16 he_rts_thres;
	__le16 max_nss_mcs[CMD_HE_MCS_BW_NUM];
	u8 rsv[6];
} __packed;
/*
#define MT7902_BSS_UPDATE_MAX_SIZE	(sizeof(struct sta_req_hdr) +	\
					 sizeof(struct bss_info_omac) +	\
					 sizeof(struct bss_info_basic) +\
					 sizeof(struct bss_info_rf_ch) +\
					 sizeof(struct bss_info_ra) +	\
					 sizeof(struct bss_info_hw_amsdu) +\
					 sizeof(struct bss_info_he) +	\
					 sizeof(struct bss_info_bmc_rate) +\
					 sizeof(struct bss_info_ext_bss))
*/

struct bss_req_hdr {
	u8 bss_idx;
	u8 __rsv[3];
} __packed;


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
/*
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
} __packed; */

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


/*
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


struct bss_sec_tlv {
	__le16 tag;
	__le16 len;
	u8 __rsv1[2];
	u8 cipher;
	u8 __rsv2[1];
} __packed;
struct bss_txcmd_tlv {
	__le16 tag;
	__le16 len;
	u8 txcmd_mode;
	u8 __rsv[3];
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
*/


#define MT7902_BSS_UPDATE_MAX_SIZE	(sizeof(struct bss_req_hdr) +	\
					 sizeof(struct mt76_connac_bss_basic_tlv) +	\
					 sizeof(struct bss_rlm_tlv) +\
					 sizeof(struct bss_ra_tlv) + \
					 sizeof(struct bss_info_uni_he) +	\
					 sizeof(struct bss_rate_tlv) +\
					 sizeof(struct bss_txcmd_tlv) +\
					 sizeof(struct bss_power_save) +\
					 sizeof(struct bss_sec_tlv) +\
					 sizeof(struct bss_mld_tlv))

#define mt7902_BEACON_UPDATE_SIZE	(sizeof(struct bss_req_hdr) +	\
					 sizeof(struct bss_bcn_content_tlv) + \
					 sizeof(struct bss_bcn_cntdwn_tlv) + \
					 sizeof(struct bss_bcn_mbss_tlv))

static int
mt7902_mcu_bss_basic_tlv(struct sk_buff *skb,
			struct ieee80211_vif *vif,
			struct ieee80211_sta *sta,
			struct mt76_phy *phy, u16 wlan_idx,
			bool enable);
static void
mt7902_mcu_bss_rfch_tlv(struct sk_buff *skb, struct ieee80211_vif *vif,
			struct mt792x_phy *phy);
static void
mt7902_mcu_bss_bmc_tlv(struct sk_buff *skb, struct mt792x_phy *phy);
static void
mt7902_mcu_bss_sec_tlv(struct sk_buff *skb, struct ieee80211_vif *vif);
static void
mt7902_mcu_bss_ra_tlv(struct sk_buff *skb, struct ieee80211_vif *vif,
		      struct mt792x_phy *phy);
static void
mt7902_mcu_bss_he_tlv(struct sk_buff *skb, struct ieee80211_vif *vif,
		      struct mt792x_phy *phy);
static void
mt7902_mcu_bss_txcmd_tlv(struct sk_buff *skb, bool en);
static void
mt7902_mcu_bss_mld_tlv(struct sk_buff *skb);
struct mt76_vif {
	u8 idx;
	u8 omac_idx;
	u8 band_idx;
	u8 wmm_idx;
	u8 scan_seq_num;
	u8 cipher;
};
int mt7902_mcu_add_sta(struct mt792x_dev *dev, struct ieee80211_vif *vif,
		       struct ieee80211_sta *sta, int conn_state, bool newly);
int mt7902_mcu_add_beacon(struct ieee80211_hw *hw, struct ieee80211_vif *vif,
		  int enable);
static int
mt7902_mcu_muar_config(struct mt792x_phy *phy, struct ieee80211_vif *vif,
		       bool bssid, bool enable);
static inline bool
mt7902_is_ebf_supported(struct mt792x_phy *phy, struct ieee80211_vif *vif,
			struct ieee80211_sta *sta, bool bfee);
static void
mt7902_mcu_sta_bfer_he(struct ieee80211_sta *sta, struct ieee80211_vif *vif,
		       struct mt792x_phy *phy, struct sta_rec_bf *bf);
static void
mt7902_mcu_sta_bfer_vht(struct ieee80211_sta *sta, struct mt792x_phy *phy,
			struct sta_rec_bf *bf, bool explicit);
static void
mt7902_mcu_sta_bfer_ht(struct ieee80211_sta *sta, struct mt792x_phy *phy,
		       struct sta_rec_bf *bf);
static void
mt7902_mcu_sta_bfer_tlv(struct mt792x_dev *dev, struct sk_buff *skb,
		       struct ieee80211_vif *vif, struct ieee80211_sta *sta);
static void
mt7902_mcu_sta_ht_tlv(struct sk_buff *skb, struct ieee80211_sta *sta);
static void
mt7902_mcu_sta_vht_tlv(struct sk_buff *skb, struct ieee80211_sta *sta);
static void
mt7902_mcu_sta_amsdu_tlv(struct mt792x_dev *dev, struct sk_buff *skb,
			 struct ieee80211_vif *vif, struct ieee80211_sta *sta);
static void
mt7902_mcu_sta_he_tlv(struct sk_buff *skb, struct ieee80211_sta *sta,
		      struct ieee80211_vif *vif);
static void
mt7902_mcu_sta_he_6g_tlv(struct sk_buff *skb, struct ieee80211_sta *sta,
		      struct ieee80211_vif *vif);
static void
mt7902_mcu_sta_muru_tlv(struct sk_buff *skb, struct ieee80211_sta *sta,
			struct ieee80211_vif *vif);
static void
mt7902_mcu_sta_bfee_tlv(struct mt792x_dev *dev, struct sk_buff *skb,
			struct ieee80211_vif *vif, struct ieee80211_sta *sta);
static void
mt7902_mcu_sta_hdr_trans_tlv(struct mt792x_dev *dev, struct sk_buff *skb,
			 struct ieee80211_vif *vif, struct ieee80211_sta *sta);
static int
mt7902_mcu_add_group(struct mt792x_dev *dev, struct ieee80211_vif *vif,
		     struct ieee80211_sta *sta);

struct sta_rec_hdr_trans {
	__le16 tag;
	__le16 len;
	u8 from_ds;
	u8 to_ds;
	u8 dis_rx_hdr_tran;
	u8 mesh;
} __packed;

struct bss_info_color {
	__le16 tag;
	__le16 len;
	u8 disable;
	u8 color;
	u8 rsv[2];
} __packed;

/*
struct bss_info_bmc_rate {
	__le16 tag;
	__le16 len;
	__le16 bc_trans;
	__le16 mc_trans;
	u8 short_preamble;
	u8 rsv[7];
} __packed; 

struct bss_info_ra {
	__le16 tag;
	__le16 len;
	u8 op_mode;
	u8 adhoc_en;
	u8 short_preamble;
	u8 tx_streams;
	u8 rx_streams;
	u8 algo;
	u8 force_sgi;
	u8 force_gf;
	u8 ht_mode;
	u8 has_20_sta;		/* Check if any sta support GF. * /
	u8 bss_width_trigger_events;
	u8 vht_nss_cap;
	u8 vht_bw_signal;	/* not use * /
	u8 vht_force_sgi;	/* not use * /
	u8 se_off;
	u8 antenna_idx;
	u8 train_up_rule;
	u8 rsv[3];
	unsigned short train_up_high_thres;
	short train_up_rule_rssi;
	unsigned short low_traffic_thres;
	__le16 max_phyrate;
	__le32 phy_cap;
	__le32 interval;
	__le32 fast_interval;
} __packed; 

struct bss_info_hw_amsdu {
	__le16 tag;
	__le16 len;
	__le32 cmp_bitmap_0;
	__le32 cmp_bitmap_1;
	__le16 trig_thres;
	u8 enable;
	u8 rsv;
} __packed; 

struct bss_info_color {
	__le16 tag;
	__le16 len;
	u8 disable;
	u8 color;
	u8 rsv[2];
} __packed;

struct bss_info_he {
	__le16 tag;
	__le16 len;
	u8 he_pe_duration;
	u8 vht_op_info_present;
	__le16 he_rts_thres;
	__le16 max_nss_mcs[CMD_HE_MCS_BW_NUM];
	u8 rsv[6];
} __packed; */

struct bss_info_bcn {
	__le16 tag;
	__le16 len;
	u8 ver;
	u8 enable;
	__le16 sub_ntlv;
} __packed __aligned(4);

struct bss_info_bcn_cntdwn {
	__le16 tag;
	__le16 len;
	u8 cnt;
	u8 rsv[3];
} __packed __aligned(4);

struct bss_info_bcn_mbss {
#define MAX_BEACON_NUM	32
	__le16 tag;
	__le16 len;
	__le32 bitmap;
	__le16 offset[MAX_BEACON_NUM];
	u8 rsv[8];
} __packed __aligned(4);

struct bss_info_bcn_cont {
	__le16 tag;
	__le16 len;
	__le16 tim_ofs;
	__le16 csa_ofs;
	__le16 bcc_ofs;
	__le16 pkt_len;
} __packed __aligned(4);

struct bss_info_inband_discovery {
	__le16 tag;
	__le16 len;
	u8 tx_type;
	u8 tx_mode;
	u8 tx_interval;
	u8 enable;
	__le16 rsv;
	__le16 prob_rsp_len;
} __packed __aligned(4);

enum {
	BSS_INFO_BCN_CSA,
	BSS_INFO_BCN_BCC,
	BSS_INFO_BCN_MBSSID,
	BSS_INFO_BCN_CONTENT,
	BSS_INFO_BCN_DISCOV,
	BSS_INFO_BCN_MAX
};


#define MT7902_MAX_BEACON_SIZE		1308
#define MT7902_BEACON_UPDATE_SIZE	(sizeof(struct sta_req_hdr) +	\
					 sizeof(struct bss_info_bcn) +	\
					 sizeof(struct bss_info_bcn_cntdwn) +	\
					 sizeof(struct bss_info_bcn_mbss) +	\
					 MT_TXD_SIZE +	\
					 sizeof(struct bss_info_bcn_cont))
#define MT7902_MAX_BSS_OFFLOAD_SIZE	(MT7902_MAX_BEACON_SIZE +	\
					 MT7902_BEACON_UPDATE_SIZE)

#define MT7902_BSS_UPDATE_MAX_SIZE	(sizeof(struct sta_req_hdr) +	\
					 sizeof(struct bss_info_omac) +	\
					 sizeof(struct bss_info_basic) +\
					 sizeof(struct bss_info_rf_ch) +\
					 sizeof(struct bss_info_ra) +	\
					 sizeof(struct bss_info_hw_amsdu) +\
					 sizeof(struct bss_info_he) +	\
					 sizeof(struct bss_info_bmc_rate) +\
					 sizeof(struct bss_info_ext_bss))


#endif

enum {
	UNI_BAND_CONFIG_RADIO_ENABLE,
	UNI_BAND_CONFIG_EDCCA_ENABLE = 0x5,
	UNI_BAND_CONFIG_EDCCA_THRESHOLD = 0x6,
	UNI_BAND_CONFIG_RTS_THRESHOLD = 0x8,
};
