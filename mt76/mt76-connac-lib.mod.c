#include <linux/module.h>
#define INCLUDE_VERMAGIC
#include <linux/build-salt.h>
#include <linux/elfnote-lto.h>
#include <linux/export-internal.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

#ifdef CONFIG_UNWINDER_ORC
#include <asm/orc_header.h>
ORC_HEADER;
#endif

BUILD_SALT;
BUILD_LTO_INFO;

MODULE_INFO(vermagic, VERMAGIC_STRING);
MODULE_INFO(name, KBUILD_MODNAME);

__visible struct module __this_module
__section(".gnu.linkonce.this_module") = {
	.name = KBUILD_MODNAME,
	.arch = MODULE_ARCH_INIT,
};

#ifdef CONFIG_RETPOLINE
MODULE_INFO(retpoline, "Y");
#endif

KSYMTAB_FUNC(mt76_connac_mcu_start_firmware, "_gpl", "");
KSYMTAB_FUNC(mt76_connac_mcu_patch_sem_ctrl, "_gpl", "");
KSYMTAB_FUNC(mt76_connac_mcu_start_patch, "_gpl", "");
KSYMTAB_FUNC(mt76_connac_mcu_init_download, "_gpl", "");
KSYMTAB_FUNC(mt76_connac_mcu_set_channel_domain, "_gpl", "");
KSYMTAB_FUNC(mt76_connac_mcu_set_mac_enable, "_gpl", "");
KSYMTAB_FUNC(mt76_connac_mcu_set_vif_ps, "_gpl", "");
KSYMTAB_FUNC(mt76_connac_mcu_set_rts_thresh, "_gpl", "");
KSYMTAB_FUNC(mt76_connac_mcu_beacon_loss_iter, "_gpl", "");
KSYMTAB_FUNC(mt76_connac_mcu_add_nested_tlv, "_gpl", "");
KSYMTAB_FUNC(__mt76_connac_mcu_alloc_sta_req, "_gpl", "");
KSYMTAB_FUNC(mt76_connac_mcu_alloc_wtbl_req, "_gpl", "");
KSYMTAB_FUNC(mt76_connac_mcu_bss_omac_tlv, "_gpl", "");
KSYMTAB_FUNC(mt76_connac_mcu_sta_basic_tlv, "_gpl", "");
KSYMTAB_FUNC(mt76_connac_mcu_sta_uapsd, "_gpl", "");
KSYMTAB_FUNC(mt76_connac_mcu_wtbl_hdr_trans_tlv, "_gpl", "");
KSYMTAB_FUNC(mt76_connac_mcu_sta_update_hdr_trans, "_gpl", "");
KSYMTAB_FUNC(mt76_connac_mcu_wtbl_update_hdr_trans, "_gpl", "");
KSYMTAB_FUNC(mt76_connac_mcu_wtbl_generic_tlv, "_gpl", "");
KSYMTAB_FUNC(mt76_connac_mcu_sta_he_tlv_v2, "_gpl", "");
KSYMTAB_FUNC(mt76_connac_get_phy_mode_v2, "_gpl", "");
KSYMTAB_FUNC(mt76_connac_mcu_sta_tlv, "_gpl", "");
KSYMTAB_FUNC(mt76_connac_mcu_wtbl_smps_tlv, "_gpl", "");
KSYMTAB_FUNC(mt76_connac_mcu_wtbl_ht_tlv, "_gpl", "");
KSYMTAB_FUNC(mt76_connac_mcu_sta_cmd, "_gpl", "");
KSYMTAB_FUNC(mt76_connac_mcu_wtbl_ba_tlv, "_gpl", "");
KSYMTAB_FUNC(mt76_connac_mcu_uni_add_dev, "_gpl", "");
KSYMTAB_FUNC(mt76_connac_mcu_sta_ba_tlv, "_gpl", "");
KSYMTAB_FUNC(mt76_connac_mcu_sta_wed_update, "_gpl", "");
KSYMTAB_FUNC(mt76_connac_mcu_sta_ba, "_gpl", "");
KSYMTAB_FUNC(mt76_connac_get_phy_mode, "_gpl", "");
KSYMTAB_FUNC(mt76_connac_get_phy_mode_ext, "_gpl", "");
KSYMTAB_FUNC(mt76_connac_get_he_phy_cap, "_gpl", "");
KSYMTAB_FUNC(mt76_connac_get_eht_phy_cap, "_gpl", "");
KSYMTAB_FUNC(mt76_connac_mcu_uni_set_chctx, "_gpl", "");
KSYMTAB_FUNC(mt76_connac_mcu_uni_add_bss, "_gpl", "");
KSYMTAB_FUNC(mt76_connac_mcu_hw_scan, "_gpl", "");
KSYMTAB_FUNC(mt76_connac_mcu_cancel_hw_scan, "_gpl", "");
KSYMTAB_FUNC(mt76_connac_mcu_sched_scan_req, "_gpl", "");
KSYMTAB_FUNC(mt76_connac_mcu_sched_scan_enable, "_gpl", "");
KSYMTAB_FUNC(mt76_connac_mcu_chip_config, "_gpl", "");
KSYMTAB_FUNC(mt76_connac_mcu_set_deep_sleep, "_gpl", "");
KSYMTAB_FUNC(mt76_connac_sta_state_dp, "_gpl", "");
KSYMTAB_FUNC(mt76_connac_mcu_coredump_event, "_gpl", "");
KSYMTAB_FUNC(mt76_connac_get_ch_power, "_gpl", "");
KSYMTAB_FUNC(mt76_connac_mcu_set_rate_txpower, "_gpl", "");
KSYMTAB_FUNC(mt76_connac_mcu_update_arp_filter, "_gpl", "");
KSYMTAB_FUNC(mt76_connac_mcu_set_p2p_oppps, "_gpl", "");
KSYMTAB_DATA(mt76_connac_wowlan_support, "_gpl", "");
KSYMTAB_FUNC(mt76_connac_mcu_update_gtk_rekey, "_gpl", "");
KSYMTAB_FUNC(mt76_connac_mcu_set_gtk_rekey, "_gpl", "");
KSYMTAB_FUNC(mt76_connac_mcu_set_suspend_mode, "_gpl", "");
KSYMTAB_FUNC(mt76_connac_mcu_set_wow_ctrl, "_gpl", "");
KSYMTAB_FUNC(mt76_connac_mcu_set_hif_suspend, "_gpl", "");
KSYMTAB_FUNC(mt76_connac_mcu_set_suspend_iter, "_gpl", "");
KSYMTAB_FUNC(mt76_connac_mcu_reg_rr, "_gpl", "");
KSYMTAB_FUNC(mt76_connac_mcu_reg_wr, "_gpl", "");
KSYMTAB_FUNC(mt76_connac_mcu_add_key, "_gpl", "");
KSYMTAB_FUNC(mt76_connac_mcu_bss_ext_tlv, "_gpl", "");
KSYMTAB_FUNC(mt76_connac_mcu_bss_basic_tlv, "_gpl", "");
KSYMTAB_FUNC(mt76_connac_mcu_set_pm, "_gpl", "");
KSYMTAB_FUNC(mt76_connac_mcu_restart, "_gpl", "");
KSYMTAB_FUNC(mt76_connac_mcu_rdd_cmd, "_gpl", "");
KSYMTAB_FUNC(mt76_connac2_load_ram, "_gpl", "");
KSYMTAB_FUNC(mt76_connac2_load_patch, "_gpl", "");
KSYMTAB_FUNC(mt76_connac2_mcu_fill_message, "_gpl", "");
KSYMTAB_FUNC(mt76_connac_gen_ppe_thresh, "_gpl", "");
KSYMTAB_FUNC(mt76_connac_pm_wake, "_gpl", "");
KSYMTAB_FUNC(mt76_connac_power_save_sched, "_gpl", "");
KSYMTAB_FUNC(mt76_connac_free_pending_tx_skbs, "_gpl", "");
KSYMTAB_FUNC(mt76_connac_pm_queue_skb, "_gpl", "");
KSYMTAB_FUNC(mt76_connac_pm_dequeue_skbs, "_gpl", "");
KSYMTAB_FUNC(mt76_connac_tx_complete_skb, "_gpl", "");
KSYMTAB_FUNC(mt76_connac_write_hw_txp, "_gpl", "");
KSYMTAB_FUNC(mt76_connac_txp_skb_unmap, "_gpl", "");
KSYMTAB_FUNC(mt76_connac_init_tx_queues, "_gpl", "");
KSYMTAB_FUNC(mt76_connac2_mac_tx_rate_val, "_gpl", "");
KSYMTAB_FUNC(mt76_connac2_mac_write_txwi, "_gpl", "");
KSYMTAB_FUNC(mt76_connac2_mac_fill_txs, "_gpl", "");
KSYMTAB_FUNC(mt76_connac2_mac_add_txs_skb, "_gpl", "");
KSYMTAB_FUNC(mt76_connac2_mac_decode_he_radiotap, "_gpl", "");
KSYMTAB_FUNC(mt76_connac2_reverse_frag0_hdr_trans, "_gpl", "");
KSYMTAB_FUNC(mt76_connac2_mac_fill_rx_rate, "_gpl", "");
KSYMTAB_FUNC(mt76_connac2_tx_check_aggr, "_gpl", "");
KSYMTAB_FUNC(mt76_connac2_txwi_free, "_gpl", "");
KSYMTAB_FUNC(mt76_connac2_tx_token_put, "_gpl", "");
KSYMTAB_FUNC(mt76_connac3_mac_decode_he_radiotap, "_gpl", "");

SYMBOL_CRC(mt76_connac_mcu_start_firmware, 0x32d6b359, "_gpl");
SYMBOL_CRC(mt76_connac_mcu_patch_sem_ctrl, 0x60f72384, "_gpl");
SYMBOL_CRC(mt76_connac_mcu_start_patch, 0x172ebf9b, "_gpl");
SYMBOL_CRC(mt76_connac_mcu_init_download, 0x92d9c6a8, "_gpl");
SYMBOL_CRC(mt76_connac_mcu_set_channel_domain, 0x4d5a5d18, "_gpl");
SYMBOL_CRC(mt76_connac_mcu_set_mac_enable, 0x6521eee1, "_gpl");
SYMBOL_CRC(mt76_connac_mcu_set_vif_ps, 0xfa541fba, "_gpl");
SYMBOL_CRC(mt76_connac_mcu_set_rts_thresh, 0xf1a7b5b6, "_gpl");
SYMBOL_CRC(mt76_connac_mcu_beacon_loss_iter, 0x0eb1a1d5, "_gpl");
SYMBOL_CRC(mt76_connac_mcu_add_nested_tlv, 0x89b6e947, "_gpl");
SYMBOL_CRC(__mt76_connac_mcu_alloc_sta_req, 0xae4baf87, "_gpl");
SYMBOL_CRC(mt76_connac_mcu_alloc_wtbl_req, 0x7e92a627, "_gpl");
SYMBOL_CRC(mt76_connac_mcu_bss_omac_tlv, 0xbdc4249f, "_gpl");
SYMBOL_CRC(mt76_connac_mcu_sta_basic_tlv, 0x399b46ad, "_gpl");
SYMBOL_CRC(mt76_connac_mcu_sta_uapsd, 0xd78fba54, "_gpl");
SYMBOL_CRC(mt76_connac_mcu_wtbl_hdr_trans_tlv, 0xcf87e9fb, "_gpl");
SYMBOL_CRC(mt76_connac_mcu_sta_update_hdr_trans, 0x4657ee4c, "_gpl");
SYMBOL_CRC(mt76_connac_mcu_wtbl_update_hdr_trans, 0xb54cf564, "_gpl");
SYMBOL_CRC(mt76_connac_mcu_wtbl_generic_tlv, 0x958bf15d, "_gpl");
SYMBOL_CRC(mt76_connac_mcu_sta_he_tlv_v2, 0x486b15a7, "_gpl");
SYMBOL_CRC(mt76_connac_get_phy_mode_v2, 0x299c8ed7, "_gpl");
SYMBOL_CRC(mt76_connac_mcu_sta_tlv, 0xf74ac990, "_gpl");
SYMBOL_CRC(mt76_connac_mcu_wtbl_smps_tlv, 0x365c715c, "_gpl");
SYMBOL_CRC(mt76_connac_mcu_wtbl_ht_tlv, 0x7d80fe43, "_gpl");
SYMBOL_CRC(mt76_connac_mcu_sta_cmd, 0x31a68dbf, "_gpl");
SYMBOL_CRC(mt76_connac_mcu_wtbl_ba_tlv, 0xbaee4ff3, "_gpl");
SYMBOL_CRC(mt76_connac_mcu_uni_add_dev, 0xfb332414, "_gpl");
SYMBOL_CRC(mt76_connac_mcu_sta_ba_tlv, 0xac84f976, "_gpl");
SYMBOL_CRC(mt76_connac_mcu_sta_wed_update, 0xb5a228f3, "_gpl");
SYMBOL_CRC(mt76_connac_mcu_sta_ba, 0x5c948197, "_gpl");
SYMBOL_CRC(mt76_connac_get_phy_mode, 0x37fee551, "_gpl");
SYMBOL_CRC(mt76_connac_get_phy_mode_ext, 0xa770eeb1, "_gpl");
SYMBOL_CRC(mt76_connac_get_he_phy_cap, 0x673c4f02, "_gpl");
SYMBOL_CRC(mt76_connac_get_eht_phy_cap, 0x8456058b, "_gpl");
SYMBOL_CRC(mt76_connac_mcu_uni_set_chctx, 0x57dfc659, "_gpl");
SYMBOL_CRC(mt76_connac_mcu_uni_add_bss, 0xfb1d0fb5, "_gpl");
SYMBOL_CRC(mt76_connac_mcu_hw_scan, 0x86c23065, "_gpl");
SYMBOL_CRC(mt76_connac_mcu_cancel_hw_scan, 0x2ddd4d1d, "_gpl");
SYMBOL_CRC(mt76_connac_mcu_sched_scan_req, 0xad80f77f, "_gpl");
SYMBOL_CRC(mt76_connac_mcu_sched_scan_enable, 0x95362c24, "_gpl");
SYMBOL_CRC(mt76_connac_mcu_chip_config, 0x001161b3, "_gpl");
SYMBOL_CRC(mt76_connac_mcu_set_deep_sleep, 0x648e42c5, "_gpl");
SYMBOL_CRC(mt76_connac_sta_state_dp, 0x1c02f80f, "_gpl");
SYMBOL_CRC(mt76_connac_mcu_coredump_event, 0xb9bea185, "_gpl");
SYMBOL_CRC(mt76_connac_get_ch_power, 0xac198d17, "_gpl");
SYMBOL_CRC(mt76_connac_mcu_set_rate_txpower, 0xd6266912, "_gpl");
SYMBOL_CRC(mt76_connac_mcu_update_arp_filter, 0x5a0b32c8, "_gpl");
SYMBOL_CRC(mt76_connac_mcu_set_p2p_oppps, 0x2af4fb7c, "_gpl");
SYMBOL_CRC(mt76_connac_wowlan_support, 0x908ca40c, "_gpl");
SYMBOL_CRC(mt76_connac_mcu_update_gtk_rekey, 0x3a3aa3f1, "_gpl");
SYMBOL_CRC(mt76_connac_mcu_set_gtk_rekey, 0x8ddf181c, "_gpl");
SYMBOL_CRC(mt76_connac_mcu_set_suspend_mode, 0x37e5af93, "_gpl");
SYMBOL_CRC(mt76_connac_mcu_set_wow_ctrl, 0x2a6d65d0, "_gpl");
SYMBOL_CRC(mt76_connac_mcu_set_hif_suspend, 0x33da812d, "_gpl");
SYMBOL_CRC(mt76_connac_mcu_set_suspend_iter, 0x8f8ba510, "_gpl");
SYMBOL_CRC(mt76_connac_mcu_reg_rr, 0x7a193fdd, "_gpl");
SYMBOL_CRC(mt76_connac_mcu_reg_wr, 0x067331cb, "_gpl");
SYMBOL_CRC(mt76_connac_mcu_add_key, 0xe2b50b4e, "_gpl");
SYMBOL_CRC(mt76_connac_mcu_bss_ext_tlv, 0x8d31ff00, "_gpl");
SYMBOL_CRC(mt76_connac_mcu_bss_basic_tlv, 0xd17a9caf, "_gpl");
SYMBOL_CRC(mt76_connac_mcu_set_pm, 0xcdb1da8b, "_gpl");
SYMBOL_CRC(mt76_connac_mcu_restart, 0xf9a3ac27, "_gpl");
SYMBOL_CRC(mt76_connac_mcu_rdd_cmd, 0x8a976969, "_gpl");
SYMBOL_CRC(mt76_connac2_load_ram, 0xd5c38789, "_gpl");
SYMBOL_CRC(mt76_connac2_load_patch, 0xed07e767, "_gpl");
SYMBOL_CRC(mt76_connac2_mcu_fill_message, 0x4ab0ce52, "_gpl");
SYMBOL_CRC(mt76_connac_gen_ppe_thresh, 0x0a25337f, "_gpl");
SYMBOL_CRC(mt76_connac_pm_wake, 0x0d2f576a, "_gpl");
SYMBOL_CRC(mt76_connac_power_save_sched, 0x3ee1d99a, "_gpl");
SYMBOL_CRC(mt76_connac_free_pending_tx_skbs, 0x7b545b66, "_gpl");
SYMBOL_CRC(mt76_connac_pm_queue_skb, 0xf635003a, "_gpl");
SYMBOL_CRC(mt76_connac_pm_dequeue_skbs, 0x71d41b72, "_gpl");
SYMBOL_CRC(mt76_connac_tx_complete_skb, 0xd59ceddd, "_gpl");
SYMBOL_CRC(mt76_connac_write_hw_txp, 0xae97f09e, "_gpl");
SYMBOL_CRC(mt76_connac_txp_skb_unmap, 0xb3d96c11, "_gpl");
SYMBOL_CRC(mt76_connac_init_tx_queues, 0x82ac9301, "_gpl");
SYMBOL_CRC(mt76_connac2_mac_tx_rate_val, 0xd9f16186, "_gpl");
SYMBOL_CRC(mt76_connac2_mac_write_txwi, 0x86cb3d20, "_gpl");
SYMBOL_CRC(mt76_connac2_mac_fill_txs, 0xdb94e52b, "_gpl");
SYMBOL_CRC(mt76_connac2_mac_add_txs_skb, 0x162457a7, "_gpl");
SYMBOL_CRC(mt76_connac2_mac_decode_he_radiotap, 0x25ee83a5, "_gpl");
SYMBOL_CRC(mt76_connac2_reverse_frag0_hdr_trans, 0xfff65b25, "_gpl");
SYMBOL_CRC(mt76_connac2_mac_fill_rx_rate, 0xcd7c0e00, "_gpl");
SYMBOL_CRC(mt76_connac2_tx_check_aggr, 0xc06ba219, "_gpl");
SYMBOL_CRC(mt76_connac2_txwi_free, 0x88946893, "_gpl");
SYMBOL_CRC(mt76_connac2_tx_token_put, 0x82cbcb65, "_gpl");
SYMBOL_CRC(mt76_connac3_mac_decode_he_radiotap, 0xd2f2bba9, "_gpl");

static const char ____versions[]
__used __section("__versions") =
	"\x20\x00\x00\x00\x18\xc6\xd2\x5c"
	"mt76_tx_status_unlock\0\0\0"
	"\x1c\x00\x00\x00\xa9\x9a\xd0\xc6"
	"release_firmware\0\0\0\0"
	"\x18\x00\x00\x00\x10\x6d\x86\xba"
	"devm_kmalloc\0\0\0\0"
	"\x10\x00\x00\x00\x0b\x22\x8d\xab"
	"skb_put\0"
	"\x18\x00\x00\x00\x14\x27\x52\x8d"
	"__rcu_read_lock\0"
	"\x14\x00\x00\x00\xe6\xd6\x62\xad"
	"consume_skb\0"
	"\x14\x00\x00\x00\x6e\x4a\x6e\x65"
	"snprintf\0\0\0\0"
	"\x18\x00\x00\x00\x36\xf2\xb6\xc5"
	"queue_work_on\0\0\0"
	"\x20\x00\x00\x00\xcf\xaa\x1e\xb5"
	"mt76_tx_status_skb_get\0\0"
	"\x1c\x00\x00\x00\x69\x1d\x2f\xfa"
	"mt76_tx_status_lock\0"
	"\x14\x00\x00\x00\xbf\x0f\x54\x92"
	"finish_wait\0"
	"\x20\x00\x00\x00\x63\xe8\x9a\x67"
	"dma_unmap_page_attrs\0\0\0\0"
	"\x18\x00\x00\x00\xb9\x34\xc1\x74"
	"__sw_hweight32\0\0"
	"\x1c\x00\x00\x00\x07\x33\x85\x0f"
	"request_firmware\0\0\0\0"
	"\x18\x00\x00\x00\x08\x06\x5a\xf1"
	"mt76_get_rate\0\0\0"
	"\x28\x00\x00\x00\xa7\x7e\x5f\xed"
	"ieee80211_start_tx_ba_session\0\0\0"
	"\x10\x00\x00\x00\x38\xdf\xac\x69"
	"memcpy\0\0"
	"\x20\x00\x00\x00\x2a\xd4\x37\xea"
	"ieee80211_beacon_loss\0\0\0"
	"\x20\x00\x00\x00\x33\x0d\x8d\x96"
	"__mt76_tx_complete_skb\0\0"
	"\x20\x00\x00\x00\x95\xd4\x26\x8c"
	"prepare_to_wait_event\0\0\0"
	"\x28\x00\x00\x00\x50\x3e\x6e\xb0"
	"ieee80211_get_hdrlen_from_skb\0\0\0"
	"\x18\x00\x00\x00\x8c\x89\xd4\xcb"
	"fortify_panic\0\0\0"
	"\x14\x00\x00\x00\xbb\x6d\xfb\xbd"
	"__fentry__\0\0"
	"\x18\x00\x00\x00\x06\x45\xbd\xd9"
	"wake_up_process\0"
	"\x1c\x00\x00\x00\x37\xdf\x59\x5d"
	"ieee80211_find_sta\0\0"
	"\x24\x00\x00\x00\x65\x7b\x87\xbc"
	"mt76_calculate_default_rate\0"
	"\x1c\x00\x00\x00\xad\x8a\xdd\x8d"
	"schedule_timeout\0\0\0\0"
	"\x1c\x00\x00\x00\xcb\xf6\xfd\xf0"
	"__stack_chk_fail\0\0\0\0"
	"\x20\x00\x00\x00\x6d\xb5\xfc\xb2"
	"queue_delayed_work_on\0\0\0"
	"\x1c\x00\x00\x00\xca\x21\x60\xe4"
	"_raw_spin_unlock_bh\0"
	"\x10\x00\x00\x00\x2a\x1a\xfd\xfa"
	"mt76_tx\0"
	"\x18\x00\x00\x00\xa3\x48\x7c\xc5"
	"idr_get_next\0\0\0\0"
	"\x14\x00\x00\x00\x22\x23\xa2\x87"
	"_dev_info\0\0\0"
	"\x28\x00\x00\x00\xb3\x1c\xa2\x87"
	"__ubsan_handle_out_of_bounds\0\0\0\0"
	"\x18\x00\x00\x00\x51\x69\xfc\x7d"
	"mt76_init_queue\0"
	"\x20\x00\x00\x00\xea\x59\x68\x9e"
	"mt76_tx_status_skb_done\0"
	"\x18\x00\x00\x00\x75\x79\x48\xfe"
	"init_wait_entry\0"
	"\x14\x00\x00\x00\xd1\xb1\x84\x0d"
	"devm_kfree\0\0"
	"\x14\x00\x00\x00\x21\x1d\xd8\xd0"
	"_dev_err\0\0\0\0"
	"\x14\x00\x00\x00\x33\xc3\x65\xdf"
	"skb_pull\0\0\0\0"
	"\x28\x00\x00\x00\x91\xb8\x08\x4a"
	"mt76_mcu_skb_send_and_get_msg\0\0\0"
	"\x1c\x00\x00\x00\x0f\x81\x69\x24"
	"__rcu_read_unlock\0\0\0"
	"\x14\x00\x00\x00\x2a\xf0\x64\x8b"
	"skb_push\0\0\0\0"
	"\x20\x00\x00\x00\x17\xfb\xea\x35"
	"ieee80211_wake_queues\0\0\0"
	"\x24\x00\x00\x00\xad\x08\xa0\xa7"
	"mt76_mcu_send_and_get_msg\0\0\0"
	"\x20\x00\x00\x00\x9e\xb7\x0f\x68"
	"ieee80211_iter_keys_rcu\0"
	"\x20\x00\x00\x00\x9b\x6d\xe3\xe0"
	"__mt76_mcu_msg_alloc\0\0\0\0"
	"\x14\x00\x00\x00\xae\xb3\x17\x8e"
	"idr_destroy\0"
	"\x1c\x00\x00\x00\x78\x99\xad\x3d"
	"cancel_delayed_work\0"
	"\x28\x00\x00\x00\x02\xf4\xed\x6b"
	"ieee80211_freq_khz_to_channel\0\0\0"
	"\x18\x00\x00\x00\x43\x8f\xb1\x69"
	"rfc1042_header\0\0"
	"\x24\x00\x00\x00\x90\xa7\xba\xd1"
	"__mt76_mcu_send_firmware\0\0\0\0"
	"\x1c\x00\x00\x00\xca\x39\x82\x5b"
	"__x86_return_thunk\0\0"
	"\x20\x00\x00\x00\x63\x77\x71\x78"
	"ieee80211_stop_queues\0\0\0"
	"\x10\x00\x00\x00\xa6\x50\xba\x15"
	"jiffies\0"
	"\x2c\x00\x00\x00\x61\xe5\x48\xa6"
	"__ubsan_handle_shift_out_of_bounds\0\0"
	"\x24\x00\x00\x00\xae\xb7\x7d\x42"
	"mt76_get_rate_power_limits\0\0"
	"\x24\x00\x00\x00\xae\xcb\x5f\x5d"
	"ieee80211_scan_completed\0\0\0\0"
	"\x24\x00\x00\x00\x4a\x18\xa7\x9f"
	"cancel_delayed_work_sync\0\0\0\0"
	"\x18\x00\x00\x00\x18\x01\x47\x56"
	"__warn_printk\0\0\0"
	"\x1c\x00\x00\x00\x3e\xb8\x15\x10"
	"mt76_get_sar_power\0\0"
	"\x1c\x00\x00\x00\xfc\x90\x36\x0c"
	"_raw_spin_lock_bh\0\0\0"
	"\x18\x00\x00\x00\x3c\x08\xfd\x7b"
	"mt76_put_txwi\0\0\0"
	"\x1c\x00\x00\x00\x09\x37\xed\x41"
	"get_random_bytes\0\0\0\0"
	"\x2c\x00\x00\x00\xc6\xfa\xb1\x54"
	"__ubsan_handle_load_invalid_value\0\0\0"
	"\x20\x00\x00\x00\xbd\xfc\x79\x18"
	"bridge_tunnel_header\0\0\0\0"
	"\x24\x00\x00\x00\xab\xde\xc6\x06"
	"dev_kfree_skb_any_reason\0\0\0\0"
	"\x20\x00\x00\x00\x5d\x7b\xc1\xe2"
	"__SCT__might_resched\0\0\0\0"
	"\x18\x00\x00\x00\x72\x3f\x86\xba"
	"module_layout\0\0\0"
	"\x00\x00\x00\x00\x00\x00\x00\x00";

MODULE_INFO(depends, "mt76,mac80211,cfg80211");


MODULE_INFO(srcversion, "3249D3DBCCE415A4B487A7A");
