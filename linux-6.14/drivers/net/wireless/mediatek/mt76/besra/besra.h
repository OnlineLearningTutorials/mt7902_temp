/* SPDX-License-Identifier: ISC */
/* Copyright (C) 2020 MediaTek Inc. */

#ifndef __BESRA_H
#define __BESRA_H

#include <linux/interrupt.h>
#include <linux/ktime.h>
#include "../mt76_connac.h"
#include "regs.h"

#define BESRA_MAX_INTERFACES		19
#define BESRA_MAX_WMM_SETS		4
#define BESRA_WTBL_SIZE			544
#define BESRA_WTBL_RESERVED		201
#define BESRA_WTBL_STA			(BESRA_WTBL_RESERVED - \
					 BESRA_MAX_INTERFACES)

#define BESRA_WATCHDOG_TIME		(HZ / 10)
#define BESRA_RESET_TIMEOUT		(30 * HZ)

#define BESRA_TX_RING_SIZE		2048
#define BESRA_TX_MCU_RING_SIZE		256
#define BESRA_TX_FWDL_RING_SIZE		128

#define BESRA_RX_RING_SIZE		1536
#define BESRA_RX_MCU_RING_SIZE		512

#define MT7902_FIRMWARE_WA		"mediatek/mt7902_wa.bin"
#define MT7902_FIRMWARE_WM		"mediatek/mt7902_wm.bin"
#define MT7902_ROM_PATCH		"mediatek/mt7902_rom_patch.bin"
#define MT7902_FIRMWARE_ROM		"mediatek/mt7902_wf_rom.bin"
#define MT7902_FIRMWARE_ROM_SRAM	"mediatek/mt7902_wf_rom_sram.bin"
#define MT7902_EEPROM_DEFAULT		"mediatek/mt7902_eeprom.bin"

#define BESRA_EEPROM_SIZE		4096
#define BESRA_EEPROM_BLOCK_SIZE		16
#define BESRA_TOKEN_SIZE		8192

#define BESRA_CFEND_RATE_DEFAULT	0x49	/* OFDM 24M */
#define BESRA_CFEND_RATE_11B		0x03	/* 11B LP, 11M */

#define BESRA_THERMAL_THROTTLE_MAX	100
#define BESRA_CDEV_THROTTLE_MAX	99

#define BESRA_SKU_RATE_NUM		161

#define BESRA_MAX_TWT_AGRT		16
#define BESRA_MAX_STA_TWT_AGRT		8
#define BESRA_MAX_QUEUE		(__MT_RXQ_MAX + __MT_MCUQ_MAX + 2)

struct besra_vif;
struct besra_sta;
struct besra_dfs_pulse;
struct besra_dfs_pattern;

enum besra_txq_id {
	BESRA_TXQ_FWDL = 16,
	BESRA_TXQ_MCU_WM,
	BESRA_TXQ_BAND0,
	BESRA_TXQ_BAND1,
	BESRA_TXQ_MCU_WA,
	BESRA_TXQ_BAND2,
};

enum besra_rxq_id {
	BESRA_RXQ_MCU_WM = 0,
	BESRA_RXQ_MCU_WA,
	BESRA_RXQ_MCU_WA_MAIN,
	BESRA_RXQ_MCU_WA_EXT = 2,
	BESRA_RXQ_MCU_WA_TRI = 2,
	BESRA_RXQ_BAND0 = 4,
	BESRA_RXQ_BAND1,
	BESRA_RXQ_BAND2 = 8,
};

struct besra_twt_flow {
	struct list_head list;
	u64 start_tsf;
	u64 tsf;
	u32 duration;
	u16 wcid;
	__le16 mantissa;
	u8 exp;
	u8 table_id;
	u8 id;
	u8 protection:1;
	u8 flowtype:1;
	u8 trigger:1;
	u8 sched:1;
};

struct besra_sta {
	struct mt76_wcid wcid; /* must be first */

	struct besra_vif *vif;

	struct list_head poll_list;
	struct list_head rc_list;
	u32 airtime_ac[8];

	unsigned long changed;
	unsigned long jiffies;
	unsigned long ampdu_state;

	struct mt76_sta_stats stats;

	struct mt76_connac_sta_key_conf bip;

	struct {
		u8 flowid_mask;
		struct besra_twt_flow flow[BESRA_MAX_STA_TWT_AGRT];
	} twt;
};

struct besra_vif_cap {
	bool ht_ldpc:1;
	bool vht_ldpc:1;
	bool he_ldpc:1;
	bool vht_su_ebfer:1;
	bool vht_su_ebfee:1;
	bool vht_mu_ebfer:1;
	bool vht_mu_ebfee:1;
	bool he_su_ebfer:1;
	bool he_su_ebfee:1;
	bool he_mu_ebfer:1;
};

struct besra_vif {
	struct mt76_vif mt76; /* must be first */

	struct besra_vif_cap cap;
	struct besra_sta sta;
	struct besra_phy *phy;

	struct ieee80211_tx_queue_params queue_params[IEEE80211_NUM_ACS];
	struct cfg80211_bitrate_mask bitrate_mask;
};

/* per-phy stats.  */
struct mib_stats {
	u32 ack_fail_cnt;
	u32 fcs_err_cnt;
	u32 rts_cnt;
	u32 rts_retries_cnt;
	u32 ba_miss_cnt;
	u32 tx_bf_cnt;
	u32 tx_mu_mpdu_cnt;
	u32 tx_mu_acked_mpdu_cnt;
	u32 tx_su_acked_mpdu_cnt;
	u32 tx_bf_ibf_ppdu_cnt;
	u32 tx_bf_ebf_ppdu_cnt;

	u32 tx_bf_rx_fb_all_cnt;
	u32 tx_bf_rx_fb_he_cnt;
	u32 tx_bf_rx_fb_vht_cnt;
	u32 tx_bf_rx_fb_ht_cnt;

	u32 tx_bf_rx_fb_bw; /* value of last sample, not cumulative */
	u32 tx_bf_rx_fb_nc_cnt;
	u32 tx_bf_rx_fb_nr_cnt;
	u32 tx_bf_fb_cpl_cnt;
	u32 tx_bf_fb_trig_cnt;

	u32 tx_ampdu_cnt;
	u32 tx_stop_q_empty_cnt;
	u32 tx_mpdu_attempts_cnt;
	u32 tx_mpdu_success_cnt;
	u32 tx_pkt_ebf_cnt;
	u32 tx_pkt_ibf_cnt;

	u32 tx_rwp_fail_cnt;
	u32 tx_rwp_need_cnt;

	/* rx stats */
	u32 rx_fifo_full_cnt;
	u32 channel_idle_cnt;
	u32 rx_vector_mismatch_cnt;
	u32 rx_delimiter_fail_cnt;
	u32 rx_len_mismatch_cnt;
	u32 rx_mpdu_cnt;
	u32 rx_ampdu_cnt;
	u32 rx_ampdu_bytes_cnt;
	u32 rx_ampdu_valid_subframe_cnt;
	u32 rx_ampdu_valid_subframe_bytes_cnt;
	u32 rx_pfdrop_cnt;
	u32 rx_vec_queue_overflow_drop_cnt;
	u32 rx_ba_cnt;

	u32 tx_amsdu[8];
	u32 tx_amsdu_cnt;
};

struct besra_hif {
	struct list_head list;

	struct device *dev;
	void __iomem *regs;
	int irq;
};

struct besra_phy {
	struct mt76_phy *mt76;
	struct besra_dev *dev;

	struct ieee80211_sband_iftype_data iftype[NUM_NL80211_BANDS][NUM_NL80211_IFTYPES];

	struct ieee80211_vif *monitor_vif;

	struct thermal_cooling_device *cdev;
	u8 cdev_state;
	u8 throttle_state;
	u32 throttle_temp[2]; /* 0: critical high, 1: maximum */

	u32 rxfilter;
	u64 omac_mask;
	u8 band_idx;

	u16 noise;

	s16 coverage_class;
	u8 slottime;

	u8 rdd_state;

	u32 rx_ampdu_ts;
	u32 ampdu_ref;

	struct mib_stats mib;
	struct mt76_channel_state state_ts;

#ifdef CONFIG_NL80211_TESTMODE
	struct {
		u32 *reg_backup;

		s32 last_freq_offset;
		u8 last_rcpi[4];
		s8 last_ib_rssi[4];
		s8 last_wb_rssi[4];
		u8 last_snr;

		u8 spe_idx;
	} test;
#endif
};

struct besra_dev {
	union { /* must be first */
		struct mt76_dev mt76;
		struct mt76_phy mphy;
	};

	struct besra_hif *hif2;
	struct besra_reg_desc reg;
	u8 q_id[BESRA_MAX_QUEUE];
	u32 q_int_mask[BESRA_MAX_QUEUE];
	u32 wfdma_mask;

	const struct mt76_bus_ops *bus_ops;
	struct tasklet_struct irq_tasklet;
	struct besra_phy phy;

	/* monitor rx chain configured channel */
	struct cfg80211_chan_def rdd2_chandef;
	struct besra_phy *rdd2_phy;

	u32 chainmask;
	u16 chain_shift_ext;
	u16 chain_shift_tri;
	u32 hif_idx;

	struct work_struct init_work;
	struct work_struct rc_work;
	struct work_struct reset_work;
	wait_queue_head_t reset_wait;
	u32 reset_state;

	struct list_head sta_rc_list;
	struct list_head sta_poll_list;
	struct list_head twt_list;
	spinlock_t sta_poll_lock;

	u32 hw_pattern;

	bool dbdc_support;
	bool tbtc_support;
	bool flash_mode;
	bool muru_debug;
	bool ibf;
	u8 fw_debug_wm;
	u8 fw_debug_wa;
	u8 fw_debug_bin;
	u16 fw_debug_seq;

	struct dentry *debugfs_dir;
	struct rchan *relay_fwlog;

	void *cal;

	struct {
		u8 table_mask;
		u8 n_agrt;
	} twt;
};

enum {
	WFDMA0 = 0x0,
	WFDMA1,
	WFDMA_EXT,
	__MT_WFDMA_MAX,
};

enum {
	MT_CTX0,
	MT_HIF0 = 0x0,

	MT_LMAC_AC00 = 0x0,
	MT_LMAC_AC01,
	MT_LMAC_AC02,
	MT_LMAC_AC03,
	MT_LMAC_ALTX0 = 0x10,
	MT_LMAC_BMC0,
	MT_LMAC_BCN0,
	MT_LMAC_PSMP0,
};

enum {
	MT_RX_SEL0,
	MT_RX_SEL1,
	MT_RX_SEL2, /* monitor chain */
};

enum besra_rdd_cmd {
	RDD_STOP,
	RDD_START,
	RDD_DET_MODE,
	RDD_RADAR_EMULATE,
	RDD_START_TXQ = 20,
	RDD_CAC_START = 50,
	RDD_CAC_END,
	RDD_NORMAL_START,
	RDD_DISABLE_DFS_CAL,
	RDD_PULSE_DBG,
	RDD_READ_PULSE,
	RDD_RESUME_BF,
	RDD_IRQ_OFF,
};

static inline struct besra_phy *
besra_hw_phy(struct ieee80211_hw *hw)
{
	struct mt76_phy *phy = hw->priv;

	return phy->priv;
}

static inline struct besra_dev *
besra_hw_dev(struct ieee80211_hw *hw)
{
	struct mt76_phy *phy = hw->priv;

	return container_of(phy->dev, struct besra_dev, mt76);
}

static inline struct besra_phy *
besra_ext_phy(struct besra_dev *dev)
{
	struct mt76_phy *phy = dev->mt76.phy2;

	if (!phy)
		return NULL;

	return phy->priv;
}

static inline struct besra_phy *
besra_tri_phy(struct besra_dev *dev)
{
	struct mt76_phy *phy = dev->mt76.phy3;

	if (!phy)
		return NULL;

	return phy->priv;
}

static inline u8
besra_get_phy_id(struct besra_phy *phy)
{
	if (phy->mt76 == &phy->dev->mphy)
		return MT_MAIN_PHY;

	if (phy->mt76 == phy->dev->mt76.phy2)
		return MT_EXT_PHY;

	return MT_TRI_PHY;
}

extern const struct ieee80211_ops besra_ops;
extern const struct mt76_testmode_ops besra_testmode_ops;
extern struct pci_driver besra_pci_driver;
extern struct pci_driver besra_hif_driver;

struct besra_dev *besra_mmio_probe(struct device *pdev,
				     void __iomem *mem_base, u32 device_id);
void besra_wfsys_reset(struct besra_dev *dev);
irqreturn_t besra_irq_handler(int irq, void *dev_instance);
u64 __besra_get_tsf(struct ieee80211_hw *hw, struct besra_vif *mvif);
int besra_register_device(struct besra_dev *dev);
void besra_unregister_device(struct besra_dev *dev);
int besra_eeprom_init(struct besra_dev *dev);
void besra_eeprom_parse_hw_cap(struct besra_dev *dev,
				struct besra_phy *phy);
int besra_eeprom_get_target_power(struct besra_dev *dev,
				   struct ieee80211_channel *chan,
				   u8 chain_idx);
s8 besra_eeprom_get_power_delta(struct besra_dev *dev, int band);
int besra_dma_init(struct besra_dev *dev);
void besra_dma_prefetch(struct besra_dev *dev);
void besra_dma_cleanup(struct besra_dev *dev);
int besra_rom_start(struct besra_dev *dev);
int besra_mcu_init(struct besra_dev *dev);
int besra_mcu_twt_agrt_update(struct besra_dev *dev,
			       struct besra_vif *mvif,
			       struct besra_twt_flow *flow,
			       int cmd);
int besra_mcu_add_dev_info(struct besra_phy *phy,
			    struct ieee80211_vif *vif, bool enable);
int besra_mcu_add_bss_info(struct besra_phy *phy,
			    struct ieee80211_vif *vif, int enable);
int besra_mcu_add_sta(struct besra_dev *dev, struct ieee80211_vif *vif,
		       struct ieee80211_sta *sta, bool enable);
int besra_mcu_add_tx_ba(struct besra_dev *dev,
			 struct ieee80211_ampdu_params *params,
			 bool add);
int besra_mcu_add_rx_ba(struct besra_dev *dev,
			 struct ieee80211_ampdu_params *params,
			 bool add);
int besra_mcu_update_bss_color(struct besra_dev *dev, struct ieee80211_vif *vif,
				struct cfg80211_he_bss_color *he_bss_color);
int besra_mcu_add_beacon(struct ieee80211_hw *hw, struct ieee80211_vif *vif,
			  int enable);
int besra_mcu_add_obss_spr(struct besra_dev *dev, struct ieee80211_vif *vif,
                            bool enable);
int besra_mcu_add_rate_ctrl(struct besra_dev *dev, struct ieee80211_vif *vif,
			     struct ieee80211_sta *sta, bool changed);
int besra_mcu_add_smps(struct besra_dev *dev, struct ieee80211_vif *vif,
			struct ieee80211_sta *sta);
int besra_set_channel(struct besra_phy *phy);
int besra_mcu_set_chan_info(struct besra_phy *phy, int tag);
int besra_mcu_set_tx(struct besra_dev *dev, struct ieee80211_vif *vif);
int besra_mcu_update_edca(struct besra_dev *dev, void *req);
int besra_mcu_set_fixed_rate_ctrl(struct besra_dev *dev,
				   struct ieee80211_vif *vif,
				   struct ieee80211_sta *sta,
				   void *data, u32 field);
int besra_mcu_set_eeprom(struct besra_dev *dev);
int besra_mcu_get_eeprom(struct besra_dev *dev, u32 offset);
int besra_mcu_get_eeprom_free_block(struct besra_dev *dev, u8 *block_num);
int besra_mcu_set_test_param(struct besra_dev *dev, u8 param, bool test_mode,
			      u8 en);
int besra_mcu_set_ser(struct besra_dev *dev, u8 action, u8 set, u8 band);
int besra_mcu_set_sku_en(struct besra_phy *phy, bool enable);
int besra_mcu_set_txpower_sku(struct besra_phy *phy);
int besra_mcu_get_txpower_sku(struct besra_phy *phy, s8 *txpower, int len);
int besra_mcu_set_txbf(struct besra_dev *dev, u8 action);
int besra_mcu_set_fcc5_lpn(struct besra_dev *dev, int val);
int besra_mcu_set_pulse_th(struct besra_dev *dev,
			    const struct besra_dfs_pulse *pulse);
int besra_mcu_set_radar_th(struct besra_dev *dev, int index,
			    const struct besra_dfs_pattern *pattern);
int besra_mcu_set_radio_en(struct besra_phy *phy, bool enable);
void besra_mcu_set_pm(void *priv, u8 *mac, struct ieee80211_vif *vif);
int besra_mcu_set_rts_thresh(struct besra_phy *phy, u32 val);
int besra_mcu_set_edcca_thresh(struct besra_phy *phy);
int besra_mcu_set_edcca_en(struct besra_phy *phy, bool enable);
int besra_mcu_set_muru_ctrl(struct besra_dev *dev, u32 cmd, u32 val);
int besra_mcu_apply_group_cal(struct besra_dev *dev);
int besra_mcu_apply_tx_dpd(struct besra_phy *phy);
int besra_mcu_get_chan_mib_info(struct besra_phy *phy, bool chan_switch);
int besra_mcu_get_temperature(struct besra_phy *phy);
int besra_mcu_set_thermal_throttling(struct besra_phy *phy, u8 state);
int besra_mcu_get_rx_rate(struct besra_phy *phy, struct ieee80211_vif *vif,
			   struct ieee80211_sta *sta, struct rate_info *rate);
int besra_mcu_rdd_cmd(struct besra_dev *dev, int cmd, u8 index,
		      u8 rx_sel, u8 val);
int besra_mcu_rdd_background_enable(struct besra_phy *phy,
				     struct cfg80211_chan_def *chandef);
int besra_mcu_rf_regval(struct besra_dev *dev, u32 regidx, u32 *val, bool set);
int besra_mcu_wa_cmd(struct besra_dev *dev, int cmd, u32 a1, u32 a2, u32 a3);
int besra_mcu_fw_log_2_host(struct besra_dev *dev, u8 type, u8 ctrl);
int besra_mcu_fw_dbg_ctrl(struct besra_dev *dev, u32 module, u8 level);
void besra_mcu_rx_event(struct besra_dev *dev, struct sk_buff *skb);
void besra_mcu_exit(struct besra_dev *dev);
int besra_mcu_set_hdr_trans(struct besra_dev *dev, bool hdr_trans);

void besra_dual_hif_set_irq_mask(struct besra_dev *dev, bool write_reg,
				  u32 clear, u32 set);

static inline void besra_irq_enable(struct besra_dev *dev, u32 mask)
{
	if (dev->hif2)
		besra_dual_hif_set_irq_mask(dev, false, 0, mask);
	else
		mt76_set_irq_mask(&dev->mt76, 0, 0, mask);

	tasklet_schedule(&dev->irq_tasklet);
}

static inline void besra_irq_disable(struct besra_dev *dev, u32 mask)
{
	if (dev->hif2)
		besra_dual_hif_set_irq_mask(dev, true, mask, 0);
	else
		mt76_set_irq_mask(&dev->mt76, MT_INT_MASK_CSR, mask, 0);
}

u32 besra_mac_wtbl_lmac_addr(struct besra_dev *dev, u16 wcid, u8 dw);
bool besra_mac_wtbl_update(struct besra_dev *dev, int idx, u32 mask);
void besra_mac_reset_counters(struct besra_phy *phy);
void besra_mac_cca_stats_reset(struct besra_phy *phy);
void besra_mac_enable_nf(struct besra_dev *dev, u8 band);
void besra_mac_write_txwi(struct besra_dev *dev, __le32 *txwi,
			   struct sk_buff *skb, struct mt76_wcid *wcid, int pid,
			   struct ieee80211_key_conf *key, bool beacon);
void besra_mac_set_timing(struct besra_phy *phy);
int besra_mac_sta_add(struct mt76_dev *mdev, struct ieee80211_vif *vif,
		       struct ieee80211_sta *sta);
void besra_mac_sta_remove(struct mt76_dev *mdev, struct ieee80211_vif *vif,
			   struct ieee80211_sta *sta);
void besra_mac_work(struct work_struct *work);
void besra_mac_reset_work(struct work_struct *work);
void besra_mac_sta_rc_work(struct work_struct *work);
void besra_mac_update_stats(struct besra_phy *phy);
void besra_mac_twt_teardown_flow(struct besra_dev *dev,
				  struct besra_sta *msta,
				  u8 flowid);
void besra_mac_add_twt_setup(struct ieee80211_hw *hw,
			      struct ieee80211_sta *sta,
			      struct ieee80211_twt_setup *twt);
int besra_tx_prepare_skb(struct mt76_dev *mdev, void *txwi_ptr,
			  enum mt76_txq_id qid, struct mt76_wcid *wcid,
			  struct ieee80211_sta *sta,
			  struct mt76_tx_info *tx_info);
void besra_tx_complete_skb(struct mt76_dev *mdev, struct mt76_queue_entry *e);
void besra_tx_token_put(struct besra_dev *dev);
int besra_init_tx_queues(struct besra_phy *phy, int idx, int n_desc, int ring_base);
void besra_queue_rx_skb(struct mt76_dev *mdev, enum mt76_rxq_id q,
			 struct sk_buff *skb);
bool besra_rx_check(struct mt76_dev *mdev, enum mt76_rxq_id q, void *data, int len);
void besra_sta_ps(struct mt76_dev *mdev, struct ieee80211_sta *sta, bool ps);
void besra_stats_work(struct work_struct *work);
int mt76_dfs_start_rdd(struct besra_dev *dev, bool force);
int besra_dfs_init_radar_detector(struct besra_phy *phy);
void besra_set_stream_he_caps(struct besra_phy *phy);
void besra_set_stream_vht_txbf_caps(struct besra_phy *phy);
void besra_update_channel(struct mt76_phy *mphy);
int besra_mcu_muru_debug_set(struct besra_dev *dev, bool enable);
int besra_mcu_muru_debug_get(struct besra_phy *phy, void *ms);
int besra_init_debugfs(struct besra_phy *phy);
void besra_debugfs_rx_fw_monitor(struct besra_dev *dev, const void *data, int len);
bool besra_debugfs_rx_log(struct besra_dev *dev, const void *data, int len);
int besra_mcu_add_key(struct mt76_dev *dev, struct ieee80211_vif *vif,
		      struct mt76_connac_sta_key_conf *sta_key_conf,
		      struct ieee80211_key_conf *key, int mcu_cmd,
		      struct mt76_wcid *wcid, enum set_key_cmd cmd);
int besra_mcu_wtbl_update_hdr_trans(struct besra_dev *dev,
			struct ieee80211_vif *vif, struct ieee80211_sta *sta);
#ifdef CONFIG_MAC80211_DEBUGFS
void besra_sta_add_debugfs(struct ieee80211_hw *hw, struct ieee80211_vif *vif,
			    struct ieee80211_sta *sta, struct dentry *dir);
#endif

#endif
