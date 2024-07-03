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

KSYMTAB_DATA(mt76x02_rates, "_gpl", "");
KSYMTAB_FUNC(mt76x02_init_device, "_gpl", "");
KSYMTAB_FUNC(mt76x02_configure_filter, "_gpl", "");
KSYMTAB_FUNC(mt76x02_sta_add, "_gpl", "");
KSYMTAB_FUNC(mt76x02_sta_remove, "_gpl", "");
KSYMTAB_FUNC(mt76x02_add_interface, "_gpl", "");
KSYMTAB_FUNC(mt76x02_remove_interface, "_gpl", "");
KSYMTAB_FUNC(mt76x02_ampdu_action, "_gpl", "");
KSYMTAB_FUNC(mt76x02_set_key, "_gpl", "");
KSYMTAB_FUNC(mt76x02_conf_tx, "_gpl", "");
KSYMTAB_FUNC(mt76x02_set_tx_ackto, "_gpl", "");
KSYMTAB_FUNC(mt76x02_set_coverage_class, "_gpl", "");
KSYMTAB_FUNC(mt76x02_set_rts_threshold, "_gpl", "");
KSYMTAB_FUNC(mt76x02_sta_rate_tbl_update, "_gpl", "");
KSYMTAB_FUNC(mt76x02_remove_hdr_pad, "_gpl", "");
KSYMTAB_FUNC(mt76x02_sw_scan_complete, "_gpl", "");
KSYMTAB_FUNC(mt76x02_sta_ps, "_gpl", "");
KSYMTAB_FUNC(mt76x02_bss_info_changed, "_gpl", "");
KSYMTAB_FUNC(mt76x02_config_mac_addr_list, "_gpl", "");
KSYMTAB_FUNC(mt76x02_mac_reset_counters, "_gpl", "");
KSYMTAB_FUNC(mt76x02_mac_shared_key_setup, "_gpl", "");
KSYMTAB_FUNC(mt76x02_mac_wcid_setup, "_gpl", "");
KSYMTAB_FUNC(mt76x02_mac_write_txwi, "_gpl", "");
KSYMTAB_FUNC(mt76x02_mac_setaddr, "_gpl", "");
KSYMTAB_FUNC(mt76x02_tx_complete_skb, "_gpl", "");
KSYMTAB_FUNC(mt76x02_update_channel, "_gpl", "");
KSYMTAB_FUNC(mt76x02_edcca_init, "_gpl", "");
KSYMTAB_FUNC(mt76x02_mac_cc_reset, "_gpl", "");
KSYMTAB_FUNC(mt76x02_mcu_parse_response, "_gpl", "");
KSYMTAB_FUNC(mt76x02_mcu_msg_send, "_gpl", "");
KSYMTAB_FUNC(mt76x02_mcu_function_select, "_gpl", "");
KSYMTAB_FUNC(mt76x02_mcu_set_radio_state, "_gpl", "");
KSYMTAB_FUNC(mt76x02_mcu_calibrate, "_gpl", "");
KSYMTAB_FUNC(mt76x02_mcu_cleanup, "_gpl", "");
KSYMTAB_FUNC(mt76x02_set_ethtool_fwver, "_gpl", "");
KSYMTAB_FUNC(mt76x02_eeprom_copy, "_gpl", "");
KSYMTAB_FUNC(mt76x02_get_efuse_data, "_gpl", "");
KSYMTAB_FUNC(mt76x02_eeprom_parse_hw_cap, "_gpl", "");
KSYMTAB_FUNC(mt76x02_ext_pa_enabled, "_gpl", "");
KSYMTAB_FUNC(mt76x02_get_rx_gain, "_gpl", "");
KSYMTAB_FUNC(mt76x02_get_lna_gain, "_gpl", "");
KSYMTAB_FUNC(mt76x02_phy_set_rxpath, "_gpl", "");
KSYMTAB_FUNC(mt76x02_phy_set_txdac, "_gpl", "");
KSYMTAB_FUNC(mt76x02_get_max_rate_power, "_gpl", "");
KSYMTAB_FUNC(mt76x02_limit_rate_power, "_gpl", "");
KSYMTAB_FUNC(mt76x02_add_rate_power_offset, "_gpl", "");
KSYMTAB_FUNC(mt76x02_phy_set_txpower, "_gpl", "");
KSYMTAB_FUNC(mt76x02_phy_set_bw, "_gpl", "");
KSYMTAB_FUNC(mt76x02_phy_set_band, "_gpl", "");
KSYMTAB_FUNC(mt76x02_phy_adjust_vga_gain, "_gpl", "");
KSYMTAB_FUNC(mt76x02_init_agc_gain, "_gpl", "");
KSYMTAB_FUNC(mt76x02e_init_beacon_config, "_gpl", "");
KSYMTAB_FUNC(mt76x02_dma_init, "_gpl", "");
KSYMTAB_FUNC(mt76x02_rx_poll_complete, "_gpl", "");
KSYMTAB_FUNC(mt76x02_irq_handler, "_gpl", "");
KSYMTAB_FUNC(mt76x02_dma_disable, "_gpl", "");
KSYMTAB_FUNC(mt76x02_mac_start, "_gpl", "");
KSYMTAB_FUNC(mt76x02_reconfig_complete, "_gpl", "");
KSYMTAB_FUNC(mt76x02_tx, "_gpl", "");
KSYMTAB_FUNC(mt76x02_queue_rx_skb, "_gpl", "");
KSYMTAB_FUNC(mt76x02_tx_set_txpwr_auto, "_gpl", "");
KSYMTAB_FUNC(mt76x02_tx_status_data, "_gpl", "");
KSYMTAB_FUNC(mt76x02_tx_prepare_skb, "_gpl", "");
KSYMTAB_FUNC(mt76x02_init_debugfs, "_gpl", "");
KSYMTAB_FUNC(mt76x02_phy_dfs_adjust_agc, "_gpl", "");
KSYMTAB_FUNC(mt76x02_dfs_init_params, "_gpl", "");
KSYMTAB_FUNC(mt76x02_mac_set_beacon, "_gpl", "");
KSYMTAB_FUNC(mt76x02_resync_beacon_timer, "_gpl", "");
KSYMTAB_FUNC(mt76x02_update_beacon_iter, "_gpl", "");
KSYMTAB_FUNC(mt76x02_enqueue_buffered_bc, "_gpl", "");
KSYMTAB_FUNC(mt76x02_init_beacon_config, "_gpl", "");

SYMBOL_CRC(mt76x02_rates, 0x5bd84fd5, "_gpl");
SYMBOL_CRC(mt76x02_init_device, 0x8ea21960, "_gpl");
SYMBOL_CRC(mt76x02_configure_filter, 0x70837787, "_gpl");
SYMBOL_CRC(mt76x02_sta_add, 0x1ca07be3, "_gpl");
SYMBOL_CRC(mt76x02_sta_remove, 0xf41a9448, "_gpl");
SYMBOL_CRC(mt76x02_add_interface, 0x9f2f1e4b, "_gpl");
SYMBOL_CRC(mt76x02_remove_interface, 0xa7fbebb5, "_gpl");
SYMBOL_CRC(mt76x02_ampdu_action, 0x7e6b9a2d, "_gpl");
SYMBOL_CRC(mt76x02_set_key, 0x6c1316ff, "_gpl");
SYMBOL_CRC(mt76x02_conf_tx, 0xf0362ceb, "_gpl");
SYMBOL_CRC(mt76x02_set_tx_ackto, 0x439caa1f, "_gpl");
SYMBOL_CRC(mt76x02_set_coverage_class, 0x50969dfb, "_gpl");
SYMBOL_CRC(mt76x02_set_rts_threshold, 0x38d6ce52, "_gpl");
SYMBOL_CRC(mt76x02_sta_rate_tbl_update, 0x77e0d31e, "_gpl");
SYMBOL_CRC(mt76x02_remove_hdr_pad, 0xe436fbda, "_gpl");
SYMBOL_CRC(mt76x02_sw_scan_complete, 0xe4b4db9d, "_gpl");
SYMBOL_CRC(mt76x02_sta_ps, 0x5cc94d13, "_gpl");
SYMBOL_CRC(mt76x02_bss_info_changed, 0x05f5057f, "_gpl");
SYMBOL_CRC(mt76x02_config_mac_addr_list, 0x58abd0f9, "_gpl");
SYMBOL_CRC(mt76x02_mac_reset_counters, 0x23e350c8, "_gpl");
SYMBOL_CRC(mt76x02_mac_shared_key_setup, 0xc635e1c7, "_gpl");
SYMBOL_CRC(mt76x02_mac_wcid_setup, 0x834c9244, "_gpl");
SYMBOL_CRC(mt76x02_mac_write_txwi, 0x7eff1dc1, "_gpl");
SYMBOL_CRC(mt76x02_mac_setaddr, 0xac6174a7, "_gpl");
SYMBOL_CRC(mt76x02_tx_complete_skb, 0x13d9b649, "_gpl");
SYMBOL_CRC(mt76x02_update_channel, 0x0e4eea69, "_gpl");
SYMBOL_CRC(mt76x02_edcca_init, 0x7b69169b, "_gpl");
SYMBOL_CRC(mt76x02_mac_cc_reset, 0x8a9d4f87, "_gpl");
SYMBOL_CRC(mt76x02_mcu_parse_response, 0x6f81294b, "_gpl");
SYMBOL_CRC(mt76x02_mcu_msg_send, 0x9edbc5e7, "_gpl");
SYMBOL_CRC(mt76x02_mcu_function_select, 0xdbed5ed8, "_gpl");
SYMBOL_CRC(mt76x02_mcu_set_radio_state, 0x415ea237, "_gpl");
SYMBOL_CRC(mt76x02_mcu_calibrate, 0xeaefec64, "_gpl");
SYMBOL_CRC(mt76x02_mcu_cleanup, 0x5cff6366, "_gpl");
SYMBOL_CRC(mt76x02_set_ethtool_fwver, 0x5a6bcfbe, "_gpl");
SYMBOL_CRC(mt76x02_eeprom_copy, 0x9a4e4a42, "_gpl");
SYMBOL_CRC(mt76x02_get_efuse_data, 0xff606b2e, "_gpl");
SYMBOL_CRC(mt76x02_eeprom_parse_hw_cap, 0xfb4d8aaa, "_gpl");
SYMBOL_CRC(mt76x02_ext_pa_enabled, 0xf2464bdb, "_gpl");
SYMBOL_CRC(mt76x02_get_rx_gain, 0x7e1821a6, "_gpl");
SYMBOL_CRC(mt76x02_get_lna_gain, 0xc0462113, "_gpl");
SYMBOL_CRC(mt76x02_phy_set_rxpath, 0x18881048, "_gpl");
SYMBOL_CRC(mt76x02_phy_set_txdac, 0x7414c103, "_gpl");
SYMBOL_CRC(mt76x02_get_max_rate_power, 0xad5d3ba8, "_gpl");
SYMBOL_CRC(mt76x02_limit_rate_power, 0x93c479bc, "_gpl");
SYMBOL_CRC(mt76x02_add_rate_power_offset, 0x75fef85d, "_gpl");
SYMBOL_CRC(mt76x02_phy_set_txpower, 0x782c2894, "_gpl");
SYMBOL_CRC(mt76x02_phy_set_bw, 0xf36cb4f0, "_gpl");
SYMBOL_CRC(mt76x02_phy_set_band, 0x0406b68b, "_gpl");
SYMBOL_CRC(mt76x02_phy_adjust_vga_gain, 0x0ebd5c9c, "_gpl");
SYMBOL_CRC(mt76x02_init_agc_gain, 0x5c80694c, "_gpl");
SYMBOL_CRC(mt76x02e_init_beacon_config, 0x3268aab3, "_gpl");
SYMBOL_CRC(mt76x02_dma_init, 0x9acde319, "_gpl");
SYMBOL_CRC(mt76x02_rx_poll_complete, 0xd4eee9f2, "_gpl");
SYMBOL_CRC(mt76x02_irq_handler, 0x91d5b9ee, "_gpl");
SYMBOL_CRC(mt76x02_dma_disable, 0x87b0266c, "_gpl");
SYMBOL_CRC(mt76x02_mac_start, 0x631070dc, "_gpl");
SYMBOL_CRC(mt76x02_reconfig_complete, 0x4cc82321, "_gpl");
SYMBOL_CRC(mt76x02_tx, 0x530dc20d, "_gpl");
SYMBOL_CRC(mt76x02_queue_rx_skb, 0x8f5e332b, "_gpl");
SYMBOL_CRC(mt76x02_tx_set_txpwr_auto, 0x87e85792, "_gpl");
SYMBOL_CRC(mt76x02_tx_status_data, 0x4fc6d0ea, "_gpl");
SYMBOL_CRC(mt76x02_tx_prepare_skb, 0x5190321b, "_gpl");
SYMBOL_CRC(mt76x02_init_debugfs, 0xbf6424d5, "_gpl");
SYMBOL_CRC(mt76x02_phy_dfs_adjust_agc, 0x9ec612de, "_gpl");
SYMBOL_CRC(mt76x02_dfs_init_params, 0x14ce9a4b, "_gpl");
SYMBOL_CRC(mt76x02_mac_set_beacon, 0xcecd1b77, "_gpl");
SYMBOL_CRC(mt76x02_resync_beacon_timer, 0xc00851d5, "_gpl");
SYMBOL_CRC(mt76x02_update_beacon_iter, 0x9e06e3c8, "_gpl");
SYMBOL_CRC(mt76x02_enqueue_buffered_bc, 0xc9fd2859, "_gpl");
SYMBOL_CRC(mt76x02_init_beacon_config, 0x6b9c1bd0, "_gpl");

static const char ____versions[]
__used __section("__versions") =
	"\x28\x00\x00\x00\x73\x18\xad\x0d"
	"ieee80211_sta_register_airtime\0\0"
	"\x1c\x00\x00\x00\xa5\x40\x82\xd1"
	"mt76_dma_rx_poll\0\0\0\0"
	"\x20\x00\x00\x00\x18\xc6\xd2\x5c"
	"mt76_tx_status_unlock\0\0\0"
	"\x1c\x00\x00\x00\xd4\x69\x87\x4e"
	"simple_attr_open\0\0\0\0"
	"\x1c\x00\x00\x00\x4c\x8f\x73\xfe"
	"debugfs_attr_write\0\0"
	"\x18\x00\x00\x00\x10\x6d\x86\xba"
	"devm_kmalloc\0\0\0\0"
	"\x18\x00\x00\x00\x14\x27\x52\x8d"
	"__rcu_read_lock\0"
	"\x14\x00\x00\x00\xe6\xd6\x62\xad"
	"consume_skb\0"
	"\x10\x00\x00\x00\xeb\x02\xe6\xb0"
	"memmove\0"
	"\x20\x00\x00\x00\x0d\xee\xd6\x1f"
	"netif_napi_add_weight\0\0\0"
	"\x14\x00\x00\x00\x6e\x4a\x6e\x65"
	"snprintf\0\0\0\0"
	"\x20\x00\x00\x00\xcf\xaa\x1e\xb5"
	"mt76_tx_status_skb_get\0\0"
	"\x20\x00\x00\x00\x58\x89\xb8\xc4"
	"trace_raw_output_prep\0\0\0"
	"\x1c\x00\x00\x00\x69\x1d\x2f\xfa"
	"mt76_tx_status_lock\0"
	"\x14\x00\x00\x00\x88\xb5\x32\x3c"
	"skb_dequeue\0"
	"\x28\x00\x00\x00\x09\xd7\x89\xab"
	"__trace_trigger_soft_disabled\0\0\0"
	"\x1c\x00\x00\x00\xc8\xa5\xc3\xa0"
	"trace_event_printf\0\0"
	"\x18\x00\x00\x00\x07\x97\x56\x53"
	"this_cpu_off\0\0\0\0"
	"\x24\x00\x00\x00\x6f\x6f\x23\x4c"
	"__x86_indirect_thunk_r15\0\0\0\0"
	"\x18\x00\x00\x00\x9b\xdb\xd4\x69"
	"mt76_csa_check\0\0"
	"\x20\x00\x00\x00\x45\x2b\x91\xb7"
	"mt76_tx_status_skb_add\0\0"
	"\x20\x00\x00\x00\xac\xdc\x8c\x55"
	"__tracepoint_mac_txdone\0"
	"\x20\x00\x00\x00\x98\xcd\xfc\x12"
	"mt76_tx_check_agg_ssn\0\0\0"
	"\x20\x00\x00\x00\x1a\x2d\x88\xc8"
	"ieee80211_tx_status_ext\0"
	"\x1c\x00\x00\x00\xae\xa9\xe0\xf1"
	"mt76_stop_tx_queues\0"
	"\x18\x00\x00\x00\xcb\xf8\xa8\xec"
	"___pskb_trim\0\0\0\0"
	"\x20\x00\x00\x00\x00\xf1\xb9\x75"
	"trace_event_raw_init\0\0\0\0"
	"\x10\x00\x00\x00\x38\xdf\xac\x69"
	"memcpy\0\0"
	"\x20\x00\x00\x00\x33\x0d\x8d\x96"
	"__mt76_tx_complete_skb\0\0"
	"\x14\x00\x00\x00\xab\x8e\xd3\x43"
	"pcpu_hot\0\0\0\0"
	"\x14\x00\x00\x00\x85\xf0\xa9\xdc"
	"seq_lseek\0\0\0"
	"\x1c\x00\x00\x00\x20\x5d\x05\xc3"
	"usleep_range_state\0\0"
	"\x20\x00\x00\x00\x15\x1e\x28\x65"
	"ieee80211_restart_hw\0\0\0\0"
	"\x18\x00\x00\x00\x84\x33\x0c\x55"
	"bpf_trace_run2\0\0"
	"\x28\x00\x00\x00\x50\x3e\x6e\xb0"
	"ieee80211_get_hdrlen_from_skb\0\0\0"
	"\x18\x00\x00\x00\x03\x9b\x1b\x9e"
	"kthread_park\0\0\0\0"
	"\x1c\x00\x00\x00\x21\xf7\x53\x26"
	"mt76_mcu_rx_event\0\0\0"
	"\x18\x00\x00\x00\x64\xbd\x8f\xba"
	"_raw_spin_lock\0\0"
	"\x18\x00\x00\x00\x8c\x89\xd4\xcb"
	"fortify_panic\0\0\0"
	"\x14\x00\x00\x00\xbb\x6d\xfb\xbd"
	"__fentry__\0\0"
	"\x18\x00\x00\x00\x62\xbe\x3c\xe9"
	"mt76_wcid_init\0\0"
	"\x20\x00\x00\x00\x57\xd9\xc4\x06"
	"mt76_tx_status_check\0\0\0\0"
	"\x18\x00\x00\x00\x06\x45\xbd\xd9"
	"wake_up_process\0"
	"\x24\x00\x00\x00\x3d\xed\x44\xeb"
	"trace_event_buffer_commit\0\0\0"
	"\x24\x00\x00\x00\x30\x74\xd2\xbe"
	"ieee80211_get_buffered_bc\0\0\0"
	"\x24\x00\x00\x00\x97\x70\x48\x65"
	"__x86_indirect_thunk_rax\0\0\0\0"
	"\x1c\x00\x00\x00\xd4\x8c\xb2\xcd"
	"napi_complete_done\0\0"
	"\x20\x00\x00\x00\x78\x27\x7d\xb5"
	"ieee80211_get_tx_rates\0\0"
	"\x1c\x00\x00\x00\xd7\x2b\x9e\x14"
	"ieee80211_send_bar\0\0"
	"\x1c\x00\x00\x00\xcb\xf6\xfd\xf0"
	"__stack_chk_fail\0\0\0\0"
	"\x24\x00\x00\x00\xaf\xe5\xf5\x15"
	"debugfs_create_devm_seqfile\0"
	"\x1c\x00\x00\x00\xca\x21\x60\xe4"
	"_raw_spin_unlock_bh\0"
	"\x10\x00\x00\x00\x2a\x1a\xfd\xfa"
	"mt76_tx\0"
	"\x18\x00\x00\x00\xc2\x8c\xd8\xf1"
	"__napi_schedule\0"
	"\x18\x00\x00\x00\x48\x18\x47\x9b"
	"mt76_csa_finish\0"
	"\x10\x00\x00\x00\x94\xb6\x16\xa9"
	"strnlen\0"
	"\x18\x00\x00\x00\xef\x66\x0b\xe4"
	"mt76_wcid_alloc\0"
	"\x24\x00\x00\x00\x7c\xb2\x83\x63"
	"__x86_indirect_thunk_rdx\0\0\0\0"
	"\x1c\x00\x00\x00\x29\x41\xe1\x77"
	"mt76_wcid_key_setup\0"
	"\x14\x00\x00\x00\x22\x23\xa2\x87"
	"_dev_info\0\0\0"
	"\x28\x00\x00\x00\xb3\x1c\xa2\x87"
	"__ubsan_handle_out_of_bounds\0\0\0\0"
	"\x28\x00\x00\x00\x62\x2f\xa3\xd3"
	"ieee80211_stop_tx_ba_cb_irqsafe\0"
	"\x10\x00\x00\x00\x9b\x0d\xaa\xc5"
	"mt76_rx\0"
	"\x18\x00\x00\x00\xf7\xb3\xf3\x66"
	"kthread_unpark\0\0"
	"\x1c\x00\x00\x00\x7f\x32\x8a\xb6"
	"mt76_phy_dfs_state\0\0"
	"\x20\x00\x00\x00\xe8\x94\xe2\x60"
	"wiphy_to_ieee80211_hw\0\0\0"
	"\x18\x00\x00\x00\x51\x69\xfc\x7d"
	"mt76_init_queue\0"
	"\x1c\x00\x00\x00\x5a\xeb\x7d\xe5"
	"debugfs_attr_read\0\0\0"
	"\x1c\x00\x00\x00\x61\xa3\x35\x3b"
	"__mt76_sta_remove\0\0\0"
	"\x20\x00\x00\x00\xea\x59\x68\x9e"
	"mt76_tx_status_skb_done\0"
	"\x28\x00\x00\x00\x20\x77\xa5\x8d"
	"ieee80211_queue_delayed_work\0\0\0\0"
	"\x20\x00\x00\x00\x17\x00\x5f\xad"
	"perf_trace_buf_alloc\0\0\0\0"
	"\x24\x00\x00\x00\x33\x96\x73\x1f"
	"perf_trace_run_bpf_submit\0\0\0"
	"\x14\x00\x00\x00\x21\x1d\xd8\xd0"
	"_dev_err\0\0\0\0"
	"\x1c\x00\x00\x00\xab\xef\xb2\x42"
	"debugfs_create_bool\0"
	"\x14\x00\x00\x00\x33\xc3\x65\xdf"
	"skb_pull\0\0\0\0"
	"\x1c\x00\x00\x00\x7f\xbe\x97\x05"
	"simple_attr_release\0"
	"\x1c\x00\x00\x00\x0f\x81\x69\x24"
	"__rcu_read_unlock\0\0\0"
	"\x24\x00\x00\x00\x2e\x5e\x38\x55"
	"__x86_indirect_thunk_r14\0\0\0\0"
	"\x20\x00\x00\x00\xad\xc4\xf9\x36"
	"__SCK__tp_func_dev_irq\0\0"
	"\x14\x00\x00\x00\x4b\x8d\xfa\x4d"
	"mutex_lock\0\0"
	"\x14\x00\x00\x00\x2a\xf0\x64\x8b"
	"skb_push\0\0\0\0"
	"\x24\x00\x00\x00\xce\xce\x0e\x67"
	"__x86_indirect_thunk_rbx\0\0\0\0"
	"\x14\x00\x00\x00\xd4\x4f\x79\x02"
	"napi_enable\0"
	"\x18\x00\x00\x00\x8b\x0d\x87\x07"
	"trace_event_reg\0"
	"\x1c\x00\x00\x00\x77\x86\x3a\x3b"
	"mt76_update_survey\0\0"
	"\x1c\x00\x00\x00\xac\xb8\x2a\x9d"
	"__tasklet_schedule\0\0"
	"\x18\x00\x00\x00\x15\x43\x63\xc6"
	"mt76_ac_to_hwq\0\0"
	"\x20\x00\x00\x00\x17\xfb\xea\x35"
	"ieee80211_wake_queues\0\0\0"
	"\x24\x00\x00\x00\xad\x08\xa0\xa7"
	"mt76_mcu_send_and_get_msg\0\0\0"
	"\x1c\x00\x00\x00\x71\x22\x5a\x5a"
	"__cpu_online_mask\0\0\0"
	"\x20\x00\x00\x00\x9e\xb7\x0f\x68"
	"ieee80211_iter_keys_rcu\0"
	"\x20\x00\x00\x00\x39\xce\x3f\x3c"
	"__local_bh_enable_ip\0\0\0\0"
	"\x1c\x00\x00\x00\xeb\x16\xf2\xfe"
	"_raw_spin_trylock\0\0\0"
	"\x20\x00\x00\x00\x9b\x6d\xe3\xe0"
	"__mt76_mcu_msg_alloc\0\0\0\0"
	"\x1c\x00\x00\x00\x02\x2b\xec\xc7"
	"mt76_set_irq_mask\0\0\0"
	"\x18\x00\x00\x00\x83\x5b\x76\x7b"
	"bpf_trace_run1\0\0"
	"\x24\x00\x00\x00\x2a\x9b\x54\x31"
	"__x86_indirect_thunk_r10\0\0\0\0"
	"\x24\x00\x00\x00\x40\xe7\xb9\x0b"
	"ieee80211_calc_tx_airtime\0\0\0"
	"\x1c\x00\x00\x00\xca\x39\x82\x5b"
	"__x86_return_thunk\0\0"
	"\x1c\x00\x00\x00\xd2\xf9\x6a\x1f"
	"debugfs_create_u32\0\0"
	"\x24\x00\x00\x00\xd5\x01\xaf\x7c"
	"__SCK__tp_func_mac_txdone\0\0\0"
	"\x1c\x00\x00\x00\x10\x6c\xfb\x8a"
	"mt76_seq_puts_array\0"
	"\x20\x00\x00\x00\x63\x77\x71\x78"
	"ieee80211_stop_queues\0\0\0"
	"\x18\x00\x00\x00\x55\x2b\x46\xbd"
	"__kfifo_init\0\0\0\0"
	"\x10\x00\x00\x00\xa6\x50\xba\x15"
	"jiffies\0"
	"\x20\x00\x00\x00\xf8\xa1\x46\xeb"
	"__tracepoint_dev_irq\0\0\0\0"
	"\x14\x00\x00\x00\xc7\x49\x49\x4a"
	"seq_read\0\0\0\0"
	"\x10\x00\x00\x00\x39\xe6\x64\xdd"
	"strscpy\0"
	"\x2c\x00\x00\x00\x61\xe5\x48\xa6"
	"__ubsan_handle_shift_out_of_bounds\0\0"
	"\x20\x00\x00\x00\x82\xbf\x3c\x23"
	"mt76_mcu_get_response\0\0\0"
	"\x1c\x00\x00\x00\xff\x29\xdb\xff"
	"debugfs_create_file\0"
	"\x20\x00\x00\x00\xcb\x28\x56\xe4"
	"__SCT__tp_func_dev_irq\0\0"
	"\x24\x00\x00\x00\x07\x6e\x40\xdb"
	"trace_event_buffer_reserve\0\0"
	"\x34\x00\x00\x00\xd8\x6f\x47\xf3"
	"ieee80211_iterate_active_interfaces_atomic\0\0"
	"\x18\x00\x00\x00\x38\xf0\x13\x32"
	"mutex_unlock\0\0\0\0"
	"\x20\x00\x00\x00\xe6\x00\x99\xc3"
	"mt76_txq_schedule_all\0\0\0"
	"\x18\x00\x00\x00\x39\x63\xf4\xc6"
	"init_timer_key\0\0"
	"\x18\x00\x00\x00\xd6\xdf\xe3\xea"
	"__const_udelay\0\0"
	"\x1c\x00\x00\x00\xbb\xd9\x79\xb8"
	"mt76_rx_aggr_start\0\0"
	"\x24\x00\x00\x00\xf9\xa4\xcc\x66"
	"__x86_indirect_thunk_rcx\0\0\0\0"
	"\x1c\x00\x00\x00\xea\x59\xfe\xfa"
	"____mt76_poll_msec\0\0"
	"\x14\x00\x00\x00\x27\xc8\x2d\x2c"
	"seq_printf\0\0"
	"\x24\x00\x00\x00\xcc\xdf\x71\x68"
	"ieee80211_beacon_get_tim\0\0\0\0"
	"\x20\x00\x00\x00\x6a\xdf\xee\xff"
	"delayed_work_timer_fn\0\0\0"
	"\x1c\x00\x00\x00\x23\x98\xf3\x7e"
	"ieee80211_hdrlen\0\0\0\0"
	"\x1c\x00\x00\x00\xfc\x90\x36\x0c"
	"_raw_spin_lock_bh\0\0\0"
	"\x1c\x00\x00\x00\x5d\xce\xef\x86"
	"mt76_wcid_cleanup\0\0\0"
	"\x14\x00\x00\x00\x6d\x6a\x33\x78"
	"seq_puts\0\0\0\0"
	"\x24\x00\x00\x00\x44\x81\xd0\x86"
	"ieee80211_radar_detected\0\0\0\0"
	"\x18\x00\x00\x00\x7c\x74\x2d\xa0"
	"single_release\0\0"
	"\x1c\x00\x00\x00\x09\x37\xed\x41"
	"get_random_bytes\0\0\0\0"
	"\x1c\x00\x00\x00\x9e\x23\x92\xec"
	"mt76_queues_read\0\0\0\0"
	"\x24\x00\x00\x00\x1b\x37\xf8\x64"
	"mt76_register_debugfs_fops\0\0"
	"\x24\x00\x00\x00\xa8\xf9\x62\x03"
	"__x86_indirect_thunk_r12\0\0\0\0"
	"\x18\x00\x00\x00\x3c\x1b\x7d\xa0"
	"tasklet_setup\0\0\0"
	"\x1c\x00\x00\x00\x88\xdb\xd5\x56"
	"napi_schedule_prep\0\0"
	"\x2c\x00\x00\x00\xc6\xfa\xb1\x54"
	"__ubsan_handle_load_invalid_value\0\0\0"
	"\x18\x00\x00\x00\xf8\x83\xcf\x2a"
	"napi_disable\0\0\0\0"
	"\x24\x00\x00\x00\xab\xde\xc6\x06"
	"dev_kfree_skb_any_reason\0\0\0\0"
	"\x14\x00\x00\x00\xce\x1b\xe1\x88"
	"__mt76_poll\0"
	"\x1c\x00\x00\x00\x8d\x9f\x58\xdb"
	"mt76_rx_aggr_stop\0\0\0"
	"\x28\x00\x00\x00\xe1\x72\x8e\x68"
	"__SCT__preempt_schedule_notrace\0"
	"\x1c\x00\x00\x00\xe5\xb2\x80\x4d"
	"debugfs_create_u8\0\0\0"
	"\x14\x00\x00\x00\xd4\x9e\xe4\x23"
	"single_open\0"
	"\x1c\x00\x00\x00\xe5\x6c\x0f\x35"
	"tasklet_unlock_wait\0"
	"\x1c\x00\x00\x00\x34\x4b\xb5\xb5"
	"_raw_spin_unlock\0\0\0\0"
	"\x18\x00\x00\x00\xbe\x13\x5a\xc7"
	"mt76_dma_attach\0"
	"\x1c\x00\x00\x00\xe3\x20\xd1\x85"
	"trace_handle_return\0"
	"\x20\x00\x00\x00\x85\x1e\x0a\xf9"
	"__x86_indirect_thunk_r8\0"
	"\x24\x00\x00\x00\x66\xa2\x31\xbb"
	"__SCT__tp_func_mac_txdone\0\0\0"
	"\x20\x00\x00\x00\x12\xda\xf0\xc4"
	"ktime_get_with_offset\0\0\0"
	"\x18\x00\x00\x00\x72\x3f\x86\xba"
	"module_layout\0\0\0"
	"\x00\x00\x00\x00\x00\x00\x00\x00";

MODULE_INFO(depends, "mac80211,mt76,cfg80211");


MODULE_INFO(srcversion, "F286DACD5DF288548A91A5D");
