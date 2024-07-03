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

KSYMTAB_FUNC(mt7925_mac_write_txwi, "_gpl", "");
KSYMTAB_FUNC(mt7925_txwi_free, "_gpl", "");
KSYMTAB_FUNC(mt7925_rx_check, "_gpl", "");
KSYMTAB_FUNC(mt7925_queue_rx_skb, "_gpl", "");
KSYMTAB_FUNC(mt7925_usb_sdio_tx_prepare_skb, "_gpl", "");
KSYMTAB_FUNC(mt7925_usb_sdio_tx_complete_skb, "_gpl", "");
KSYMTAB_FUNC(mt7925_usb_sdio_tx_status_data, "_gpl", "");
KSYMTAB_FUNC(mt7925_mcu_parse_response, "_gpl", "");
KSYMTAB_FUNC(mt7925_mcu_regval, "_gpl", "");
KSYMTAB_FUNC(mt7925_mcu_set_deep_sleep, "_gpl", "");
KSYMTAB_FUNC(mt7925_run_firmware, "_gpl", "");
KSYMTAB_FUNC(mt7925_mcu_set_eeprom, "_gpl", "");
KSYMTAB_FUNC(mt7925_mcu_hw_scan, "_gpl", "");
KSYMTAB_FUNC(mt7925_mcu_sched_scan_req, "_gpl", "");
KSYMTAB_FUNC(mt7925_mcu_cancel_hw_scan, "_gpl", "");
KSYMTAB_FUNC(mt7925_mcu_set_channel_domain, "_gpl", "");
KSYMTAB_FUNC(mt7925_mcu_fill_message, "_gpl", "");
KSYMTAB_FUNC(__mt7925_start, "_gpl", "");
KSYMTAB_FUNC(mt7925_mac_sta_add, "_gpl", "");
KSYMTAB_FUNC(mt7925_mac_sta_assoc, "_gpl", "");
KSYMTAB_FUNC(mt7925_mac_sta_remove, "_gpl", "");
KSYMTAB_DATA(mt7925_ops, "_gpl", "");
KSYMTAB_FUNC(mt7925_mac_init, "_gpl", "");
KSYMTAB_FUNC(mt7925_register_device, "_gpl", "");

SYMBOL_CRC(mt7925_mac_write_txwi, 0xdb7e3b95, "_gpl");
SYMBOL_CRC(mt7925_txwi_free, 0xc073d8d8, "_gpl");
SYMBOL_CRC(mt7925_rx_check, 0xaaa8d284, "_gpl");
SYMBOL_CRC(mt7925_queue_rx_skb, 0x9cdf33af, "_gpl");
SYMBOL_CRC(mt7925_usb_sdio_tx_prepare_skb, 0x938c0907, "_gpl");
SYMBOL_CRC(mt7925_usb_sdio_tx_complete_skb, 0x848b875c, "_gpl");
SYMBOL_CRC(mt7925_usb_sdio_tx_status_data, 0x997adb3e, "_gpl");
SYMBOL_CRC(mt7925_mcu_parse_response, 0x2bbfd982, "_gpl");
SYMBOL_CRC(mt7925_mcu_regval, 0x73511af2, "_gpl");
SYMBOL_CRC(mt7925_mcu_set_deep_sleep, 0x92d7c866, "_gpl");
SYMBOL_CRC(mt7925_run_firmware, 0x609f5500, "_gpl");
SYMBOL_CRC(mt7925_mcu_set_eeprom, 0x094afc27, "_gpl");
SYMBOL_CRC(mt7925_mcu_hw_scan, 0xac2d9d4d, "_gpl");
SYMBOL_CRC(mt7925_mcu_sched_scan_req, 0x11c5a06a, "_gpl");
SYMBOL_CRC(mt7925_mcu_cancel_hw_scan, 0xd9606ff7, "_gpl");
SYMBOL_CRC(mt7925_mcu_set_channel_domain, 0xe8d041c1, "_gpl");
SYMBOL_CRC(mt7925_mcu_fill_message, 0x65a53328, "_gpl");
SYMBOL_CRC(__mt7925_start, 0xc8979cef, "_gpl");
SYMBOL_CRC(mt7925_mac_sta_add, 0x189cb4f9, "_gpl");
SYMBOL_CRC(mt7925_mac_sta_assoc, 0x39b512c1, "_gpl");
SYMBOL_CRC(mt7925_mac_sta_remove, 0x5e7935eb, "_gpl");
SYMBOL_CRC(mt7925_ops, 0x5e29ea37, "_gpl");
SYMBOL_CRC(mt7925_mac_init, 0x13b9319c, "_gpl");
SYMBOL_CRC(mt7925_register_device, 0x540960f4, "_gpl");

static const char ____versions[]
__used __section("__versions") =
	"\x2c\x00\x00\x00\xf1\xa3\x3a\x3a"
	"mt76_connac_mcu_update_gtk_rekey\0\0\0\0"
	"\x1c\x00\x00\x00\xd7\x22\x7f\x58"
	"devmap_managed_key\0\0"
	"\x28\x00\x00\x00\x73\x18\xad\x0d"
	"ieee80211_sta_register_airtime\0\0"
	"\x1c\x00\x00\x00\x9a\x2f\x00\xc2"
	"mt792x_init_wcid\0\0\0\0"
	"\x20\x00\x00\x00\x1c\x42\xd4\x6e"
	"mt792x_init_acpi_sar\0\0\0\0"
	"\x1c\x00\x00\x00\x93\x76\x58\xa4"
	"mt76_init_sar_power\0"
	"\x24\x00\x00\x00\xbf\x7d\x4d\xcf"
	"ieee80211_connection_loss\0\0\0"
	"\x20\x00\x00\x00\x18\xc6\xd2\x5c"
	"mt76_tx_status_unlock\0\0\0"
	"\x1c\x00\x00\x00\xa9\x9a\xd0\xc6"
	"release_firmware\0\0\0\0"
	"\x1c\x00\x00\x00\xd4\x69\x87\x4e"
	"simple_attr_open\0\0\0\0"
	"\x28\x00\x00\x00\xac\x40\xce\x7f"
	"ieee80211_iterate_interfaces\0\0\0\0"
	"\x14\x00\x00\x00\xbf\x71\x51\xd8"
	"mt792x_stop\0"
	"\x24\x00\x00\x00\x17\x8d\x19\xac"
	"mt76_connac_get_ch_power\0\0\0\0"
	"\x1c\x00\x00\x00\x4c\x8f\x73\xfe"
	"debugfs_attr_write\0\0"
	"\x18\x00\x00\x00\x10\x6d\x86\xba"
	"devm_kmalloc\0\0\0\0"
	"\x10\x00\x00\x00\x0b\x22\x8d\xab"
	"skb_put\0"
	"\x18\x00\x00\x00\x14\x27\x52\x8d"
	"__rcu_read_lock\0"
	"\x24\x00\x00\x00\xd7\x8e\x9c\x29"
	"mt76_connac_get_phy_mode_v2\0"
	"\x1c\x00\x00\x00\x8f\x18\x02\x7f"
	"__msecs_to_jiffies\0\0"
	"\x14\x00\x00\x00\xe6\xd6\x62\xad"
	"consume_skb\0"
	"\x10\x00\x00\x00\xeb\x02\xe6\xb0"
	"memmove\0"
	"\x14\x00\x00\x00\x6e\x4a\x6e\x65"
	"snprintf\0\0\0\0"
	"\x18\x00\x00\x00\x36\xf2\xb6\xc5"
	"queue_work_on\0\0\0"
	"\x20\x00\x00\x00\xcf\xaa\x1e\xb5"
	"mt76_tx_status_skb_get\0\0"
	"\x1c\x00\x00\x00\xe2\x26\x47\xb0"
	"mt792x_tx_worker\0\0\0\0"
	"\x18\x00\x00\x00\x7e\x5a\x28\x6e"
	"mt792x_flush\0\0\0\0"
	"\x1c\x00\x00\x00\x69\x1d\x2f\xfa"
	"mt76_tx_status_lock\0"
	"\x1c\x00\x00\x00\x41\x0c\xeb\x71"
	"mt792x_rx_get_wcid\0\0"
	"\x14\x00\x00\x00\x88\xb5\x32\x3c"
	"skb_dequeue\0"
	"\x24\x00\x00\x00\x7f\x33\x25\x0a"
	"mt76_connac_gen_ppe_thresh\0\0"
	"\x14\x00\x00\x00\xbf\x0f\x54\x92"
	"finish_wait\0"
	"\x24\x00\x00\x00\x14\x24\x33\xfb"
	"mt76_connac_mcu_uni_add_dev\0"
	"\x18\x00\x00\x00\xb9\x34\xc1\x74"
	"__sw_hweight32\0\0"
	"\x28\x00\x00\x00\xa6\x32\xb6\xcd"
	"mt76_release_buffered_frames\0\0\0\0"
	"\x1c\x00\x00\x00\x07\x33\x85\x0f"
	"request_firmware\0\0\0\0"
	"\x1c\x00\x00\x00\x89\xa0\x7c\xc5"
	"mt76_get_txpower\0\0\0\0"
	"\x24\x00\x00\x00\x54\xba\x8f\xd7"
	"mt76_connac_mcu_sta_uapsd\0\0\0"
	"\x20\x00\x00\x00\x45\x2b\x91\xb7"
	"mt76_tx_status_skb_add\0\0"
	"\x1c\x00\x00\x00\x6a\x57\x2f\x0d"
	"mt76_connac_pm_wake\0"
	"\x18\x00\x00\x00\x08\x06\x5a\xf1"
	"mt76_get_rate\0\0\0"
	"\x1c\x00\x00\x00\x34\x3a\xf7\x0d"
	"mt792x_pm_wake_work\0"
	"\x1c\x00\x00\x00\x4e\x41\x11\x03"
	"mt76_wake_tx_queue\0\0"
	"\x28\x00\x00\x00\xa7\x7e\x5f\xed"
	"ieee80211_start_tx_ba_session\0\0\0"
	"\x10\x00\x00\x00\x38\xdf\xac\x69"
	"memcpy\0\0"
	"\x1c\x00\x00\x00\x08\xf7\xee\x1c"
	"mt76_get_antenna\0\0\0\0"
	"\x24\x00\x00\x00\xd7\x7f\x93\x01"
	"mt792x_set_coverage_class\0\0\0"
	"\x20\x00\x00\x00\x4d\xea\x74\x56"
	"mt792x_acpi_get_flags\0\0\0"
	"\x1c\x00\x00\x00\xad\x82\xb0\xae"
	"_raw_read_unlock_bh\0"
	"\x20\x00\x00\x00\x33\x0d\x8d\x96"
	"__mt76_tx_complete_skb\0\0"
	"\x1c\x00\x00\x00\x2d\x72\xee\x8d"
	"_raw_read_lock_bh\0\0\0"
	"\x14\x00\x00\x00\x85\xf0\xa9\xdc"
	"seq_lseek\0\0\0"
	"\x28\x00\x00\x00\xe7\x06\x26\x9d"
	"__put_devmap_managed_page_refs\0\0"
	"\x20\x00\x00\x00\xc7\xe3\xbc\x7a"
	"mt76_set_stream_caps\0\0\0\0"
	"\x20\x00\x00\x00\x63\xe6\x61\xbb"
	"mt792x_mac_assoc_rssi\0\0\0"
	"\x20\x00\x00\x00\x95\xd4\x26\x8c"
	"prepare_to_wait_event\0\0\0"
	"\x1c\x00\x00\x00\xdc\x90\xee\x82"
	"timer_delete_sync\0\0\0"
	"\x28\x00\x00\x00\x50\x3e\x6e\xb0"
	"ieee80211_get_hdrlen_from_skb\0\0\0"
	"\x14\x00\x00\x00\x44\x43\x96\xe2"
	"__wake_up\0\0\0"
	"\x18\x00\x00\x00\xc5\xfe\x31\xe7"
	"mt792x_mac_work\0"
	"\x1c\x00\x00\x00\x2e\x14\x9f\x4a"
	"__dynamic_dev_dbg\0\0\0"
	"\x1c\x00\x00\x00\x21\xf7\x53\x26"
	"mt76_mcu_rx_event\0\0\0"
	"\x18\x00\x00\x00\x8c\x89\xd4\xcb"
	"fortify_panic\0\0\0"
	"\x14\x00\x00\x00\xbb\x6d\xfb\xbd"
	"__fentry__\0\0"
	"\x18\x00\x00\x00\x62\xbe\x3c\xe9"
	"mt76_wcid_init\0\0"
	"\x18\x00\x00\x00\x06\x45\xbd\xd9"
	"wake_up_process\0"
	"\x2c\x00\x00\x00\x66\x5b\x54\x7b"
	"mt76_connac_free_pending_tx_skbs\0\0\0\0"
	"\x20\x00\x00\x00\x41\xff\xc4\x2b"
	"mt792x_sta_statistics\0\0\0"
	"\x20\x00\x00\x00\x0a\x84\x0b\x92"
	"skb_queue_purge_reason\0\0"
	"\x28\x00\x00\x00\x9a\xd9\xe1\x3e"
	"mt76_connac_power_save_sched\0\0\0\0"
	"\x24\x00\x00\x00\x97\x70\x48\x65"
	"__x86_indirect_thunk_rax\0\0\0\0"
	"\x24\x00\x00\x00\x11\x6c\xd9\xb3"
	"mt76_connac_txp_skb_unmap\0\0\0"
	"\x1c\x00\x00\x00\xad\x8a\xdd\x8d"
	"schedule_timeout\0\0\0\0"
	"\x1c\x00\x00\x00\xcb\xf6\xfd\xf0"
	"__stack_chk_fail\0\0\0\0"
	"\x24\x00\x00\x00\xaf\xe5\xf5\x15"
	"debugfs_create_devm_seqfile\0"
	"\x20\x00\x00\x00\x6d\xb5\xfc\xb2"
	"queue_delayed_work_on\0\0\0"
	"\x1c\x00\x00\x00\xca\x21\x60\xe4"
	"_raw_spin_unlock_bh\0"
	"\x24\x00\x00\x00\x8c\x9e\xa0\x62"
	"mt792x_get_et_sset_count\0\0\0\0"
	"\x18\x00\x00\x00\x61\x97\x3c\x4d"
	"mt792x_set_tsf\0\0"
	"\x18\x00\x00\x00\xef\x66\x0b\xe4"
	"mt76_wcid_alloc\0"
	"\x14\x00\x00\x00\xfc\x11\x89\x61"
	"numa_node\0\0\0"
	"\x24\x00\x00\x00\x9a\xfa\xba\x78"
	"mt792x_init_acpi_sar_power\0\0"
	"\x1c\x00\x00\x00\x28\x19\x4f\x63"
	"mt792x_set_wakeup\0\0\0"
	"\x1c\x00\x00\x00\x29\x41\xe1\x77"
	"mt76_wcid_key_setup\0"
	"\x28\x00\x00\x00\x1c\x18\xdf\x8d"
	"mt76_connac_mcu_set_gtk_rekey\0\0\0"
	"\x10\x00\x00\x00\x49\xb3\xa9\x40"
	"vzalloc\0"
	"\x14\x00\x00\x00\x22\x23\xa2\x87"
	"_dev_info\0\0\0"
	"\x18\x00\x00\x00\xff\x3f\x41\x82"
	"skb_queue_tail\0\0"
	"\x28\x00\x00\x00\xb3\x1c\xa2\x87"
	"__ubsan_handle_out_of_bounds\0\0\0\0"
	"\x28\x00\x00\x00\x62\x2f\xa3\xd3"
	"ieee80211_stop_tx_ba_cb_irqsafe\0"
	"\x1c\x00\x00\x00\x5e\xd7\xd8\x7c"
	"page_offset_base\0\0\0\0"
	"\x10\x00\x00\x00\x9b\x0d\xaa\xc5"
	"mt76_rx\0"
	"\x20\x00\x00\x00\xf8\xed\xa7\x68"
	"mt792x_load_firmware\0\0\0\0"
	"\x1c\x00\x00\x00\xeb\xee\x74\x4a"
	"mt792x_get_et_stats\0"
	"\x24\x00\x00\x00\xb5\x0f\x1d\xfb"
	"mt76_connac_mcu_uni_add_bss\0"
	"\x28\x00\x00\x00\xe4\x6f\xb3\xbc"
	"hugetlb_optimize_vmemmap_key\0\0\0\0"
	"\x14\x00\x00\x00\x5b\xa9\x65\x76"
	"idr_remove\0\0"
	"\x20\x00\x00\x00\xe8\x94\xe2\x60"
	"wiphy_to_ieee80211_hw\0\0\0"
	"\x1c\x00\x00\x00\x5a\xeb\x7d\xe5"
	"debugfs_attr_read\0\0\0"
	"\x18\x00\x00\x00\x75\x79\x48\xfe"
	"init_wait_entry\0"
	"\x20\x00\x00\x00\xea\x59\x68\x9e"
	"mt76_tx_status_skb_done\0"
	"\x28\x00\x00\x00\x20\x77\xa5\x8d"
	"ieee80211_queue_delayed_work\0\0\0\0"
	"\x18\x00\x00\x00\x51\xf9\xcc\x17"
	"devm_kmemdup\0\0\0\0"
	"\x14\x00\x00\x00\xd1\xb1\x84\x0d"
	"devm_kfree\0\0"
	"\x14\x00\x00\x00\x21\x1d\xd8\xd0"
	"_dev_err\0\0\0\0"
	"\x14\x00\x00\x00\x33\xc3\x65\xdf"
	"skb_pull\0\0\0\0"
	"\x20\x00\x00\x00\xea\x24\x94\x1a"
	"mt76_register_device\0\0\0\0"
	"\x1c\x00\x00\x00\x87\x58\x6a\x85"
	"mt792x_get_stats\0\0\0\0"
	"\x28\x00\x00\x00\x91\xb8\x08\x4a"
	"mt76_mcu_skb_send_and_get_msg\0\0\0"
	"\x1c\x00\x00\x00\x7f\xbe\x97\x05"
	"simple_attr_release\0"
	"\x1c\x00\x00\x00\x0f\x81\x69\x24"
	"__rcu_read_unlock\0\0\0"
	"\x14\x00\x00\x00\xb8\x83\x8c\xc3"
	"mod_timer\0\0\0"
	"\x2c\x00\x00\x00\x0a\x8e\x8c\x90"
	"ieee80211_remain_on_channel_expired\0"
	"\x14\x00\x00\x00\x4b\x8d\xfa\x4d"
	"mutex_lock\0\0"
	"\x14\x00\x00\x00\x2a\xf0\x64\x8b"
	"skb_push\0\0\0\0"
	"\x20\x00\x00\x00\x17\xfb\xea\x35"
	"ieee80211_wake_queues\0\0\0"
	"\x24\x00\x00\x00\xad\x08\xa0\xa7"
	"mt76_mcu_send_and_get_msg\0\0\0"
	"\x28\x00\x00\x00\x1d\x2b\xdb\xa4"
	"ieee80211_beacon_get_template\0\0\0"
	"\x18\x00\x00\x00\x27\xda\xaa\xb3"
	"mt792x_pm_stats\0"
	"\x20\x00\x00\x00\x4d\xe6\x71\xde"
	"mt76_eeprom_override\0\0\0\0"
	"\x18\x00\x00\x00\x1c\x61\xed\xdc"
	"dev_coredumpv\0\0\0"
	"\x18\x00\x00\x00\x9f\x0c\xfb\xce"
	"__mutex_init\0\0\0\0"
	"\x20\x00\x00\x00\x9b\x6d\xe3\xe0"
	"__mt76_mcu_msg_alloc\0\0\0\0"
	"\x1c\x00\x00\x00\x70\xfc\xbe\x37"
	"jiffies_to_msecs\0\0\0\0"
	"\x24\x00\x00\x00\x02\x4f\x3c\x67"
	"mt76_connac_get_he_phy_cap\0\0"
	"\x10\x00\x00\x00\xc7\x9a\x08\x11"
	"_ctype\0\0"
	"\x18\x00\x00\x00\xb7\x28\xd7\xa4"
	"mt76_get_survey\0"
	"\x28\x00\x00\x00\x02\xf4\xed\x6b"
	"ieee80211_freq_khz_to_channel\0\0\0"
	"\x18\x00\x00\x00\x43\x8f\xb1\x69"
	"rfc1042_header\0\0"
	"\x18\x00\x00\x00\xed\xed\x8f\x9c"
	"__alloc_pages\0\0\0"
	"\x1c\x00\x00\x00\xa5\xd7\xd8\x68"
	"mt76_token_release\0\0"
	"\x1c\x00\x00\x00\xca\x39\x82\x5b"
	"__x86_return_thunk\0\0"
	"\x18\x00\x00\x00\xc4\xf8\x25\x20"
	"mt792x_get_tsf\0\0"
	"\x20\x00\x00\x00\x54\xea\xa5\xd9"
	"__init_waitqueue_head\0\0\0"
	"\x1c\x00\x00\x00\x6f\xe4\x3a\xe9"
	"__pskb_pull_tail\0\0\0\0"
	"\x24\x00\x00\x00\x38\x5c\x7b\x5a"
	"mt792x_pm_idle_timeout_set\0\0"
	"\x14\x00\x00\x00\x01\xd3\x1c\x9b"
	"mt792x_tx\0\0\0"
	"\x20\x00\x00\x00\x25\x14\x75\xbb"
	"mt792x_get_et_strings\0\0\0"
	"\x18\x00\x00\x00\x22\xa5\x49\x4c"
	"mt76_sta_state\0\0"
	"\x24\x00\x00\x00\xd3\xbd\x0e\xa8"
	"ieee80211_ready_on_channel\0\0"
	"\x20\x00\x00\x00\x63\x77\x71\x78"
	"ieee80211_stop_queues\0\0\0"
	"\x10\x00\x00\x00\xa6\x50\xba\x15"
	"jiffies\0"
	"\x20\x00\x00\x00\x0b\x7e\x43\xd5"
	"cfg80211_reg_can_beacon\0"
	"\x20\x00\x00\x00\xf9\xa5\x7d\x15"
	"mt792x_remove_interface\0"
	"\x14\x00\x00\x00\xc7\x49\x49\x4a"
	"seq_read\0\0\0\0"
	"\x2c\x00\x00\x00\xa9\xbb\xf2\xd2"
	"mt76_connac3_mac_decode_he_radiotap\0"
	"\x18\x00\x00\x00\x6c\x1e\x65\x97"
	"vmemmap_base\0\0\0\0"
	"\x28\x00\x00\x00\x86\x61\xf1\xd9"
	"mt76_connac2_mac_tx_rate_val\0\0\0\0"
	"\x28\x00\x00\x00\x47\xe9\xb6\x89"
	"mt76_connac_mcu_add_nested_tlv\0\0"
	"\x14\x00\x00\x00\xe9\x68\xf5\x17"
	"mt76_rates\0\0"
	"\x2c\x00\x00\x00\x61\xe5\x48\xa6"
	"__ubsan_handle_shift_out_of_bounds\0\0"
	"\x1c\x00\x00\x00\x7b\x47\xdd\xb4"
	"mt792x_queues_read\0\0"
	"\x1c\x00\x00\x00\xff\x29\xdb\xff"
	"debugfs_create_file\0"
	"\x1c\x00\x00\x00\x79\xa5\x1e\x48"
	"mt792x_queues_acq\0\0\0"
	"\x24\x00\x00\x00\xae\xb7\x7d\x42"
	"mt76_get_rate_power_limits\0\0"
	"\x28\x00\x00\x00\xee\x33\x42\xff"
	"ieee80211_sched_scan_results\0\0\0\0"
	"\x24\x00\x00\x00\xae\xcb\x5f\x5d"
	"ieee80211_scan_completed\0\0\0\0"
	"\x34\x00\x00\x00\xd8\x6f\x47\xf3"
	"ieee80211_iterate_active_interfaces_atomic\0\0"
	"\x18\x00\x00\x00\x38\xf0\x13\x32"
	"mutex_unlock\0\0\0\0"
	"\x24\x00\x00\x00\x4a\x18\xa7\x9f"
	"cancel_delayed_work_sync\0\0\0\0"
	"\x20\x00\x00\x00\x4e\x18\x92\xb5"
	"mt76_insert_ccmp_hdr\0\0\0\0"
	"\x18\x00\x00\x00\x39\x63\xf4\xc6"
	"init_timer_key\0\0"
	"\x18\x00\x00\x00\xcc\xb5\x56\x96"
	"param_ops_bool\0\0"
	"\x20\x00\x00\x00\x26\x61\xd1\xd2"
	"mt792x_tx_stats_show\0\0\0\0"
	"\x2c\x00\x00\x00\x93\xaf\xe5\x37"
	"mt76_connac_mcu_set_suspend_mode\0\0\0\0"
	"\x1c\x00\x00\x00\xbb\xd9\x79\xb8"
	"mt76_rx_aggr_start\0\0"
	"\x14\x00\x00\x00\x7d\x97\x53\xb2"
	"__folio_put\0"
	"\x1c\x00\x00\x00\xfe\x2d\xc1\x03"
	"cancel_work_sync\0\0\0\0"
	"\x18\x00\x00\x00\x18\x01\x47\x56"
	"__warn_printk\0\0\0"
	"\x14\x00\x00\x00\x27\xc8\x2d\x2c"
	"seq_printf\0\0"
	"\x24\x00\x00\x00\x62\xfb\xa0\xc3"
	"mt792x_pm_idle_timeout_get\0\0"
	"\x1c\x00\x00\x00\xd5\xd4\x06\x9c"
	"mt792x_roc_timer\0\0\0\0"
	"\x28\x00\x00\x00\x85\xa1\xbe\xb9"
	"mt76_connac_mcu_coredump_event\0\0"
	"\x28\x00\x00\x00\x8a\x44\x84\x55"
	"ieee80211_channel_to_freq_khz\0\0\0"
	"\x20\x00\x00\x00\x6a\xdf\xee\xff"
	"delayed_work_timer_fn\0\0\0"
	"\x1c\x00\x00\x00\x3e\xb8\x15\x10"
	"mt76_get_sar_power\0\0"
	"\x28\x00\x00\x00\xa7\x15\x6b\x48"
	"mt76_connac_mcu_sta_he_tlv_v2\0\0\0"
	"\x1c\x00\x00\x00\xfc\x90\x36\x0c"
	"_raw_spin_lock_bh\0\0\0"
	"\x20\x00\x00\x00\x48\xc4\x46\x3a"
	"mt792x_mac_init_band\0\0\0\0"
	"\x1c\x00\x00\x00\xd5\x54\x2a\xa9"
	"mt76_skb_adjust_pad\0"
	"\x18\x00\x00\x00\x3c\x08\xfd\x7b"
	"mt76_put_txwi\0\0\0"
	"\x1c\x00\x00\x00\x2f\xcc\x60\x2b"
	"mt792x_init_wiphy\0\0\0"
	"\x14\x00\x00\x00\x6d\x6a\x33\x78"
	"seq_puts\0\0\0\0"
	"\x28\x00\x00\x00\x87\xaf\x4b\xae"
	"__mt76_connac_mcu_alloc_sta_req\0"
	"\x18\x00\x00\x00\x7c\x74\x2d\xa0"
	"single_release\0\0"
	"\x1c\x00\x00\x00\x09\x37\xed\x41"
	"get_random_bytes\0\0\0\0"
	"\x1c\x00\x00\x00\x9e\x23\x92\xec"
	"mt76_queues_read\0\0\0\0"
	"\x20\x00\x00\x00\xfe\xcd\x76\xca"
	"mt76_sta_pre_rcu_remove\0"
	"\x24\x00\x00\x00\x1b\x37\xf8\x64"
	"mt76_register_debugfs_fops\0\0"
	"\x18\x00\x00\x00\xbb\x01\x2d\x50"
	"mt792x_reset\0\0\0\0"
	"\x18\x00\x00\x00\x22\x98\x7a\x32"
	"mt76_rx_signal\0\0"
	"\x20\x00\x00\x00\xfd\x2b\x0f\x91"
	"__mt76_set_tx_blocked\0\0\0"
	"\x20\x00\x00\x00\xca\x5f\xfd\x11"
	"ieee80211_queue_work\0\0\0\0"
	"\x24\x00\x00\x00\x22\x00\x96\x9a"
	"mt792x_mac_reset_counters\0\0\0"
	"\x1c\x00\x00\x00\xfe\xcc\x74\xc5"
	"napi_consume_skb\0\0\0\0"
	"\x2c\x00\x00\x00\xc6\xfa\xb1\x54"
	"__ubsan_handle_load_invalid_value\0\0\0"
	"\x10\x00\x00\x00\x9c\x53\x4d\x75"
	"strlen\0\0"
	"\x20\x00\x00\x00\xbd\xfc\x79\x18"
	"bridge_tunnel_header\0\0\0\0"
	"\x14\x00\x00\x00\xce\x1b\xe1\x88"
	"__mt76_poll\0"
	"\x1c\x00\x00\x00\x8d\x9f\x58\xdb"
	"mt76_rx_aggr_stop\0\0\0"
	"\x14\x00\x00\x00\xd4\x9e\xe4\x23"
	"single_open\0"
	"\x24\x00\x00\x00\xb2\x07\x38\xb1"
	"mt792x_unassign_vif_chanctx\0"
	"\x24\x00\x00\x00\x51\xe5\xfe\x37"
	"mt76_connac_get_phy_mode\0\0\0\0"
	"\x28\x00\x00\x00\xad\x46\x9b\x39"
	"mt76_connac_mcu_sta_basic_tlv\0\0\0"
	"\x20\x00\x00\x00\xf6\x88\x7b\x12"
	"ieee80211_disconnect\0\0\0\0"
	"\x20\x00\x00\x00\x5d\x7b\xc1\xe2"
	"__SCT__might_resched\0\0\0\0"
	"\x24\x00\x00\x00\x49\xb1\xcc\xe4"
	"mt792x_pm_power_save_work\0\0\0"
	"\x24\x00\x00\x00\x32\x35\xd7\x11"
	"mt792x_assign_vif_chanctx\0\0\0"
	"\x14\x00\x00\x00\xd3\x85\x33\x2d"
	"system_wq\0\0\0"
	"\x18\x00\x00\x00\x72\x3f\x86\xba"
	"module_layout\0\0\0"
	"\x00\x00\x00\x00\x00\x00\x00\x00";

MODULE_INFO(depends, "mt76-connac-lib,mac80211,mt792x-lib,mt76,cfg80211");


MODULE_INFO(srcversion, "A07BDF767B0073F2EA0C0A5");