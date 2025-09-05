# Some Data Struct used in the files

 - To load all module in serial
 ```
sudo rmmod mt7902e
sudo rmmod mt7921e
sudo rmmod mt7921_common
sudo rmmod mt792x-lib
sudo rmmod mt76-connac-lib
sudo rmmod mt76

sudo insmod ../mt76_module/mt76.ko
sudo insmod ../mt76_connac_lib_module/mt76-connac-lib.ko
sudo insmod ../mt792x_lib_module/mt792x-lib.ko
sudo insmod mt7902e.ko
```


```
sudo rmmod mt7902e
sudo rmmod mt792x-lib
sudo rmmod mt76-connac-lib
sudo rmmod mt76

sudo insmod mt76.ko
sudo insmod mt76-connac-lib.ko
sudo insmod mt792x-lib.ko
sudo insmod mt7902/mt7902e.ko
```
  -  linux/include/net/mac80211.h
  ```
struct ieee80211_hw {
	struct ieee80211_conf conf;
	struct wiphy *wiphy;
	const char *rate_control_algorithm;
	void *priv;
	unsigned long flags[BITS_TO_LONGS(NUM_IEEE80211_HW_FLAGS)];
	unsigned int extra_tx_headroom;
	unsigned int extra_beacon_tailroom;
	int vif_data_size;
	int sta_data_size;
	int chanctx_data_size;
	int txq_data_size;
	u16 queues;
	u16 max_listen_interval;
	s8 max_signal;
	u8 max_rates;
	u8 max_report_rates;
	u8 max_rate_tries;
	u16 max_rx_aggregation_subframes;
	u16 max_tx_aggregation_subframes;
	u8 max_tx_fragments;
	u8 offchannel_tx_hw_queue;
	u8 radiotap_mcs_details;
	u16 radiotap_vht_details;
	struct {
		int units_pos;
		s16 accuracy;
	} radiotap_timestamp;
	netdev_features_t netdev_features;
	u8 uapsd_queues;
	u8 uapsd_max_sp_len;
	u8 max_nan_de_entries;
	u8 tx_sk_pacing_shift;
	u8 weight_multiplier;
	u32 max_mtu;
	const s8 *tx_power_levels;
	u8 max_txpwr_levels_idx;
};

```


```

struct ieee80211_bss_conf {
	struct ieee80211_vif *vif;
	struct cfg80211_bss *bss;

	const u8 *bssid;
	unsigned int link_id;
	u8 addr[ETH_ALEN] __aligned(2);
	u8 htc_trig_based_pkt_ext;
	bool uora_exists;
	u8 uora_ocw_range;
	u16 frame_time_rts_th;
	bool he_support;
	bool twt_requester;
	bool twt_responder;
	bool twt_protected;
	bool twt_broadcast;
	/* erp related data */
	bool use_cts_prot;
	bool use_short_preamble;
	bool use_short_slot;
	bool enable_beacon;
	u8 dtim_period;
	u16 beacon_int;
	u16 assoc_capability;
	u64 sync_tsf;
	u32 sync_device_ts;
	u8 sync_dtim_count;
	u32 basic_rates;
	struct ieee80211_rate *beacon_rate;
	int mcast_rate[NUM_NL80211_BANDS];
	u16 ht_operation_mode;
	s32 cqm_rssi_thold;
	u32 cqm_rssi_hyst;
	s32 cqm_rssi_low;
	s32 cqm_rssi_high;
	struct ieee80211_chan_req chanreq;
	struct ieee80211_mu_group_data mu_group;
	bool qos;
	bool hidden_ssid;
	int txpower;
	enum nl80211_tx_power_setting txpower_type;
	struct ieee80211_p2p_noa_attr p2p_noa_attr;
	bool allow_p2p_go_ps;
	u16 max_idle_period;
	bool protected_keep_alive;
	bool ftm_responder;
	struct ieee80211_ftm_responder_params *ftmr_params;
	/* Multiple BSSID data */
	bool nontransmitted;
	struct ieee80211_bss_conf __rcu *tx_bss_conf;
	u8 transmitter_bssid[ETH_ALEN];
	u8 bssid_index;
	u8 bssid_indicator;
	bool ema_ap;
	u8 profile_periodicity;
	struct {
		u32 params;
		u16 nss_set;
	} he_oper;
	struct ieee80211_he_obss_pd he_obss_pd;
	struct cfg80211_he_bss_color he_bss_color;
	struct ieee80211_fils_discovery fils_discovery;
	u32 unsol_bcast_probe_resp_interval;
	struct cfg80211_bitrate_mask beacon_tx_rate;
	enum ieee80211_ap_reg_power power_type;

	struct ieee80211_parsed_tpe tpe;

	u8 pwr_reduction;
	bool eht_support;
	bool epcs_support;
	bool csa_active;

	bool mu_mimo_owner;
	struct ieee80211_chanctx_conf __rcu *chanctx_conf;

	bool color_change_active;
	u8 color_change_color;

	bool ht_ldpc;
	bool vht_ldpc;
	bool he_ldpc;
	bool vht_su_beamformer;
	bool vht_su_beamformee;
	bool vht_mu_beamformer;
	bool vht_mu_beamformee;
	bool he_su_beamformer;
	bool he_su_beamformee;
	bool he_mu_beamformer;
	bool he_full_ul_mumimo;
	bool eht_su_beamformer;
	bool eht_su_beamformee;
	bool eht_mu_beamformer;
	bool eht_80mhz_full_bw_ul_mumimo;
	bool eht_disable_mcs15;

	u8 bss_param_ch_cnt;
	u8 bss_param_ch_cnt_link_id;

	u8 s1g_long_beacon_period;
};

```
```


struct ieee80211_vif {
	enum nl80211_iftype type;
	struct ieee80211_vif_cfg cfg;
	struct ieee80211_bss_conf bss_conf;
	struct ieee80211_bss_conf __rcu *link_conf[IEEE80211_MLD_MAX_NUM_LINKS];
	u16 valid_links, active_links, dormant_links, suspended_links;
	struct ieee80211_neg_ttlm neg_ttlm;
	u8 addr[ETH_ALEN] __aligned(2);
	bool addr_valid;
	bool p2p;

	u8 cab_queue;
	u8 hw_queue[IEEE80211_NUM_ACS];

	struct ieee80211_txq *txq;

	netdev_features_t netdev_features;
	u32 driver_flags;
	u32 offload_flags;

#ifdef CONFIG_MAC80211_DEBUGFS
	struct dentry *debugfs_dir;
#endif

	bool probe_req_reg;
	bool rx_mcast_action_reg;

	/* must be last */
	u8 drv_priv[] __aligned(sizeof(void *));
};

```
```
struct ieee80211_vif_cfg {
	/* association related data */
	bool assoc, ibss_joined;
	bool ibss_creator;
	bool ps;
	u16 aid;
	u16 eml_cap;
	u16 eml_med_sync_delay;
	u16 mld_capa_op;

	__be32 arp_addr_list[IEEE80211_BSS_ARP_ADDR_LIST_LEN];
	int arp_addr_cnt;
	u8 ssid[IEEE80211_MAX_SSID_LEN];
	size_t ssid_len;
	bool s1g;
	bool idle;
	u8 ap_addr[ETH_ALEN] __aligned(2);
};

```
```
struct ieee80211_conf {
	u32 flags;
	int power_level, dynamic_ps_timeout;

	u16 listen_interval;
	u8 ps_dtim_period;

	u8 long_frame_max_tx_count, short_frame_max_tx_count;

	struct cfg80211_chan_def chandef;
	bool radar_enabled;
	enum ieee80211_smps_mode smps_mode;
};
```
```

struct wiphy {
    struct mutex mtx;
    u8 perm_addr[ETH_ALEN];
    u8 addr_mask[ETH_ALEN];
    struct mac_address *addresses;
    const struct ieee80211_txrx_stypes *mgmt_stypes;
    const struct ieee80211_iface_combination *iface_combinations;
    int n_iface_combinations;
    u16 software_iftypes;
    u16 n_addresses;
    u16 interface_modes;
    u16 max_acl_mac_addrs;
    u32 flags, regulatory_flags, features;
    u8 ext_features[DIV_ROUND_UP(NUM_NL80211_EXT_FEATURES, 8)];
    u32 ap_sme_capa;
    enum cfg80211_signal_type signal_type;
    int bss_priv_size;
    u8 max_scan_ssids;
    u8 max_sched_scan_reqs;
    u8 max_sched_scan_ssids;
    u8 max_match_sets;
    u16 max_scan_ie_len;
    u16 max_sched_scan_ie_len;
    u32 max_sched_scan_plans;
    u32 max_sched_scan_plan_interval;
    u32 max_sched_scan_plan_iterations;
    int n_cipher_suites;
    const u32 *cipher_suites;
    int n_akm_suites;
    const u32 *akm_suites;
    const struct wiphy_iftype_akm_suites *iftype_akm_suites;
    unsigned int num_iftype_akm_suites;
    u8 retry_short;
    u8 retry_long;
    u32 frag_threshold;
    u32 rts_threshold;
    u8 coverage_class;
    char fw_version[ETHTOOL_FWVERS_LEN];
    u32 hw_version;
#ifdef CONFIG_PM;
    const struct wiphy_wowlan_support *wowlan;
    struct cfg80211_wowlan *wowlan_config;
#endif;
    u16 max_remain_on_channel_duration;
    u8 max_num_pmkids;
    u32 available_antennas_tx;
    u32 available_antennas_rx;
    u32 probe_resp_offload;
    const u8 *extended_capabilities, *extended_capabilities_mask;
    u8 extended_capabilities_len;
    const struct wiphy_iftype_ext_capab *iftype_ext_capab;
    unsigned int num_iftype_ext_capab;
    const void *privid;
    struct ieee80211_supported_band *bands[NUM_NL80211_BANDS];
    void (*reg_notifier)(struct wiphy *wiphy, struct regulatory_request *request);
    const struct ieee80211_regdomain __rcu *regd;
    struct device dev;
    bool registered;
    struct dentry *debugfsdir;
    const struct ieee80211_ht_cap *ht_capa_mod_mask;
    const struct ieee80211_vht_cap *vht_capa_mod_mask;
    struct list_head wdev_list;
    possible_net_t _net;
#ifdef CONFIG_CFG80211_WEXT;
    const struct iw_handler_def *wext;
#endif;
    const struct wiphy_coalesce_support *coalesce;
    const struct wiphy_vendor_command *vendor_commands;
    const struct nl80211_vendor_cmd_info *vendor_events;
    int n_vendor_commands, n_vendor_events;
    u16 max_ap_assoc_sta;
    u8 max_num_csa_counters;
    u32 bss_select_support;
    u8 nan_supported_bands;
    u32 txq_limit;
    u32 txq_memory_limit;
    u32 txq_quantum;
    unsigned long tx_queue_len;
    u8 support_mbssid:1, support_only_he_mbssid:1;
    const struct cfg80211_pmsr_capabilities *pmsr_capa;
    struct {
        u64 peer, vif;
        u8 max_retry;
    } tid_config_support;
    u8 max_data_retry_count;
    const struct cfg80211_sar_capa *sar_capa;
    struct rfkill *rfkill;
    u8 mbssid_max_interfaces;
    u8 ema_max_profile_periodicity;
    u16 max_num_akm_suites;
    u16 hw_timestamp_max_peers;
    int n_radio;
    const struct wiphy_radio *radio;
    char priv[] ;
};


```

To add BSS (Basic Service Set) information for a STA (Station) using the Linux mac80211 subsystem, you need to configure the STA's Virtual Interface (VIF) by setting parameters in its operating mode to reflect its connection to a BSS, such as joining an existing BSS, creating an ad-hoc BSS, or handling BSS transitions. BSS information is shared through beacon frames and managed by the AP, with the STA responding to and utilizing this information to maintain connectivity and participate in the network. 



## Understanding the Terms

    BSS (Basic Service Set):
    A collection of wireless stations that can communicate, potentially including an Access Point (AP) that connects to a wired network. 

### STA (Station):
A wireless device, such as a laptop or smartphone, that participates in the network. 
### VIF (Virtual Interface):
A logical interface that manages a wireless connection, with a STA VIF typically handling station-related operations like connecting to an AP. 
### mac80211:
A Linux kernel subsystem that handles the functions of a wireless LAN driver, providing APIs to configure and manage Wi-Fi devices and connections. 


## How to Add BSS Information to a STA VIF

### 1. Create and Configure the STA VIF:
        In a Linux environment using mac80211, you would use helpers like the WifiHelper and WifiMacHelper in simulation environments, or driver-level APIs to create a new STA-mode VIF. 

wifi.Install(phy, mac, wifiStaNodes) from ns-3 is an example of installing a STA device. 

### 2. Join a BSS:

    For a STA to join a BSS managed by an AP, it sends association requests. 

The STA will listen for Beacon frames from the AP, which contain BSS information like the SSID (network name) and security settings. 
The STA's VIF then uses this information to authenticate and associate with the AP's BSS. 

### 3. Participate in an Ad-Hoc BSS:

    In an IBSS (Independent Basic Service Set) or ad-hoc mode, stations form a network without an AP. 

The STA VIF needs to perform functions normally done by an AP, such as generating beacon frames to announce its presence and synchronize with other stations in the ad-hoc BSS. 

### 4. Manage BSS Transitions (Roaming):

    When a STA needs to move to a new AP in the same or a different BSS, it uses the BSS Transition Management (BTM) protocol (IEEE 802.11v). 

A STA can send a BTM Query to an AP to get recommendations on where to roam, or the AP can send a BTM Request to the STA. 
The STA's VIF then uses this information to request a transition to the target AP's BSS. 

## Key Configuration Considerations

###  Beacon Management:
    For ad-hoc networks, the VIF must generate beacon frames containing the BSS information. 

### Association Parameters:
The VIF needs to be set to the appropriate operating mode (e.g., StaWifiMac in ns-3) to handle association and authentication with the chosen BSS. 

### Parameter Negotiation:
In a BSS, the STA uses information from the AP's beacons to set operational parameters, such as the BSS Max Idle Period for power saving. 


