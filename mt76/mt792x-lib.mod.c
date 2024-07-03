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

KSYMTAB_FUNC(mt792x_tx, "_gpl", "");
KSYMTAB_FUNC(mt792x_stop, "_gpl", "");
KSYMTAB_FUNC(mt792x_remove_interface, "_gpl", "");
KSYMTAB_FUNC(mt792x_conf_tx, "_gpl", "");
KSYMTAB_FUNC(mt792x_get_stats, "_gpl", "");
KSYMTAB_FUNC(mt792x_get_tsf, "_gpl", "");
KSYMTAB_FUNC(mt792x_set_tsf, "_gpl", "");
KSYMTAB_FUNC(mt792x_tx_worker, "_gpl", "");
KSYMTAB_FUNC(mt792x_roc_timer, "_gpl", "");
KSYMTAB_FUNC(mt792x_flush, "_gpl", "");
KSYMTAB_FUNC(mt792x_assign_vif_chanctx, "_gpl", "");
KSYMTAB_FUNC(mt792x_unassign_vif_chanctx, "_gpl", "");
KSYMTAB_FUNC(mt792x_set_wakeup, "_gpl", "");
KSYMTAB_FUNC(mt792x_get_et_strings, "_gpl", "");
KSYMTAB_FUNC(mt792x_get_et_sset_count, "_gpl", "");
KSYMTAB_FUNC(mt792x_get_et_stats, "_gpl", "");
KSYMTAB_FUNC(mt792x_sta_statistics, "_gpl", "");
KSYMTAB_FUNC(mt792x_set_coverage_class, "_gpl", "");
KSYMTAB_FUNC(mt792x_init_wiphy, "_gpl", "");
KSYMTAB_FUNC(mt792x_get_mac80211_ops, "_gpl", "");
KSYMTAB_FUNC(mt792x_init_wcid, "_gpl", "");
KSYMTAB_FUNC(mt792x_mcu_drv_pmctrl, "_gpl", "");
KSYMTAB_FUNC(mt792x_mcu_fw_pmctrl, "_gpl", "");
KSYMTAB_FUNC(__mt792xe_mcu_drv_pmctrl, "_gpl", "");
KSYMTAB_FUNC(mt792xe_mcu_drv_pmctrl, "_gpl", "");
KSYMTAB_FUNC(mt792xe_mcu_fw_pmctrl, "_gpl", "");
KSYMTAB_FUNC(mt792x_load_firmware, "_gpl", "");
KSYMTAB_FUNC(mt792x_mac_work, "_gpl", "");
KSYMTAB_FUNC(mt792x_mac_set_timeing, "_gpl", "");
KSYMTAB_FUNC(mt792x_mac_update_mib_stats, "_gpl", "");
KSYMTAB_FUNC(mt792x_rx_get_wcid, "_gpl", "");
KSYMTAB_FUNC(mt792x_mac_assoc_rssi, "_gpl", "");
KSYMTAB_FUNC(mt792x_mac_reset_counters, "_gpl", "");
KSYMTAB_FUNC(mt792x_update_channel, "_gpl", "");
KSYMTAB_FUNC(mt792x_reset, "_gpl", "");
KSYMTAB_FUNC(mt792x_mac_init_band, "_gpl", "");
KSYMTAB_FUNC(mt792x_pm_wake_work, "_gpl", "");
KSYMTAB_FUNC(mt792x_pm_power_save_work, "_gpl", "");
KSYMTAB_DATA(__tracepoint_lp_event, "_gpl", "");
KSYMTAB_FUNC(__traceiter_lp_event, "_gpl", "");
KSYMTAB_DATA(__SCK__tp_func_lp_event, "_gpl", "");
KSYMTAB_FUNC(__SCT__tp_func_lp_event, "_gpl", "");
KSYMTAB_FUNC(mt792x_tx_stats_show, "_gpl", "");
KSYMTAB_FUNC(mt792x_queues_acq, "_gpl", "");
KSYMTAB_FUNC(mt792x_queues_read, "_gpl", "");
KSYMTAB_FUNC(mt792x_pm_stats, "_gpl", "");
KSYMTAB_FUNC(mt792x_pm_idle_timeout_set, "_gpl", "");
KSYMTAB_FUNC(mt792x_pm_idle_timeout_get, "_gpl", "");
KSYMTAB_FUNC(mt792x_irq_handler, "_gpl", "");
KSYMTAB_FUNC(mt792x_irq_tasklet, "_gpl", "");
KSYMTAB_FUNC(mt792x_rx_poll_complete, "_gpl", "");
KSYMTAB_FUNC(mt792x_dma_enable, "_gpl", "");
KSYMTAB_FUNC(mt792x_wpdma_reset, "_gpl", "");
KSYMTAB_FUNC(mt792x_wpdma_reinit_cond, "_gpl", "");
KSYMTAB_FUNC(mt792x_dma_disable, "_gpl", "");
KSYMTAB_FUNC(mt792x_dma_cleanup, "_gpl", "");
KSYMTAB_FUNC(mt792x_poll_tx, "_gpl", "");
KSYMTAB_FUNC(mt792x_poll_rx, "_gpl", "");
KSYMTAB_FUNC(mt792x_wfsys_reset, "_gpl", "");
KSYMTAB_FUNC(mt792x_init_acpi_sar, "_gpl", "");
KSYMTAB_FUNC(mt792x_init_acpi_sar_power, "_gpl", "");
KSYMTAB_FUNC(mt792x_acpi_get_flags, "_gpl", "");
KSYMTAB_FUNC(mt792x_acpi_get_mtcl_conf, "_gpl", "");

SYMBOL_CRC(mt792x_tx, 0x9b1cd301, "_gpl");
SYMBOL_CRC(mt792x_stop, 0xd85171bf, "_gpl");
SYMBOL_CRC(mt792x_remove_interface, 0x157da5f9, "_gpl");
SYMBOL_CRC(mt792x_conf_tx, 0x3b656049, "_gpl");
SYMBOL_CRC(mt792x_get_stats, 0x856a5887, "_gpl");
SYMBOL_CRC(mt792x_get_tsf, 0x2025f8c4, "_gpl");
SYMBOL_CRC(mt792x_set_tsf, 0x4d3c9761, "_gpl");
SYMBOL_CRC(mt792x_tx_worker, 0xb04726e2, "_gpl");
SYMBOL_CRC(mt792x_roc_timer, 0x9c06d4d5, "_gpl");
SYMBOL_CRC(mt792x_flush, 0x6e285a7e, "_gpl");
SYMBOL_CRC(mt792x_assign_vif_chanctx, 0x11d73532, "_gpl");
SYMBOL_CRC(mt792x_unassign_vif_chanctx, 0xb13807b2, "_gpl");
SYMBOL_CRC(mt792x_set_wakeup, 0x634f1928, "_gpl");
SYMBOL_CRC(mt792x_get_et_strings, 0xbb751425, "_gpl");
SYMBOL_CRC(mt792x_get_et_sset_count, 0x62a09e8c, "_gpl");
SYMBOL_CRC(mt792x_get_et_stats, 0x4a74eeeb, "_gpl");
SYMBOL_CRC(mt792x_sta_statistics, 0x2bc4ff41, "_gpl");
SYMBOL_CRC(mt792x_set_coverage_class, 0x01937fd7, "_gpl");
SYMBOL_CRC(mt792x_init_wiphy, 0x2b60cc2f, "_gpl");
SYMBOL_CRC(mt792x_get_mac80211_ops, 0x1f1d1f1f, "_gpl");
SYMBOL_CRC(mt792x_init_wcid, 0xc2002f9a, "_gpl");
SYMBOL_CRC(mt792x_mcu_drv_pmctrl, 0x9699b8a0, "_gpl");
SYMBOL_CRC(mt792x_mcu_fw_pmctrl, 0x64c211e7, "_gpl");
SYMBOL_CRC(__mt792xe_mcu_drv_pmctrl, 0x8267ea2f, "_gpl");
SYMBOL_CRC(mt792xe_mcu_drv_pmctrl, 0xa94b2a38, "_gpl");
SYMBOL_CRC(mt792xe_mcu_fw_pmctrl, 0x03565441, "_gpl");
SYMBOL_CRC(mt792x_load_firmware, 0x68a7edf8, "_gpl");
SYMBOL_CRC(mt792x_mac_work, 0xe731fec5, "_gpl");
SYMBOL_CRC(mt792x_mac_set_timeing, 0x1e20522f, "_gpl");
SYMBOL_CRC(mt792x_mac_update_mib_stats, 0x858c2af2, "_gpl");
SYMBOL_CRC(mt792x_rx_get_wcid, 0x71eb0c41, "_gpl");
SYMBOL_CRC(mt792x_mac_assoc_rssi, 0xbb61e663, "_gpl");
SYMBOL_CRC(mt792x_mac_reset_counters, 0x9a960022, "_gpl");
SYMBOL_CRC(mt792x_update_channel, 0x4c4e0bc0, "_gpl");
SYMBOL_CRC(mt792x_reset, 0x502d01bb, "_gpl");
SYMBOL_CRC(mt792x_mac_init_band, 0x3a46c448, "_gpl");
SYMBOL_CRC(mt792x_pm_wake_work, 0x0df73a34, "_gpl");
SYMBOL_CRC(mt792x_pm_power_save_work, 0xe4ccb149, "_gpl");
SYMBOL_CRC(__tracepoint_lp_event, 0xcc752571, "_gpl");
SYMBOL_CRC(__traceiter_lp_event, 0xaa1c5b47, "_gpl");
SYMBOL_CRC(__SCK__tp_func_lp_event, 0xd7a93f6f, "_gpl");
SYMBOL_CRC(__SCT__tp_func_lp_event, 0xabff3e0f, "_gpl");
SYMBOL_CRC(mt792x_tx_stats_show, 0xd2d16126, "_gpl");
SYMBOL_CRC(mt792x_queues_acq, 0x481ea579, "_gpl");
SYMBOL_CRC(mt792x_queues_read, 0xb4dd477b, "_gpl");
SYMBOL_CRC(mt792x_pm_stats, 0xb3aada27, "_gpl");
SYMBOL_CRC(mt792x_pm_idle_timeout_set, 0x5a7b5c38, "_gpl");
SYMBOL_CRC(mt792x_pm_idle_timeout_get, 0xc3a0fb62, "_gpl");
SYMBOL_CRC(mt792x_irq_handler, 0xbd22e98c, "_gpl");
SYMBOL_CRC(mt792x_irq_tasklet, 0x8e471e29, "_gpl");
SYMBOL_CRC(mt792x_rx_poll_complete, 0xee138ba3, "_gpl");
SYMBOL_CRC(mt792x_dma_enable, 0xc3e26023, "_gpl");
SYMBOL_CRC(mt792x_wpdma_reset, 0xe3bf630d, "_gpl");
SYMBOL_CRC(mt792x_wpdma_reinit_cond, 0x142b1402, "_gpl");
SYMBOL_CRC(mt792x_dma_disable, 0xcc226572, "_gpl");
SYMBOL_CRC(mt792x_dma_cleanup, 0x8a6ebb83, "_gpl");
SYMBOL_CRC(mt792x_poll_tx, 0x89c1ae20, "_gpl");
SYMBOL_CRC(mt792x_poll_rx, 0xb8c836cd, "_gpl");
SYMBOL_CRC(mt792x_wfsys_reset, 0xc3083004, "_gpl");
SYMBOL_CRC(mt792x_init_acpi_sar, 0x6ed4421c, "_gpl");
SYMBOL_CRC(mt792x_init_acpi_sar_power, 0x78bafa9a, "_gpl");
SYMBOL_CRC(mt792x_acpi_get_flags, 0x5674ea4d, "_gpl");
SYMBOL_CRC(mt792x_acpi_get_mtcl_conf, 0x2e2fb986, "_gpl");

static const char ____versions[]
__used __section("__versions") =
	"\x24\x00\x00\x00\xa7\x08\x70\x65"
	"device_set_wakeup_enable\0\0\0\0"
	"\x1c\x00\x00\x00\xa5\x40\x82\xd1"
	"mt76_dma_rx_poll\0\0\0\0"
	"\x1c\x00\x00\x00\xa9\x9a\xd0\xc6"
	"release_firmware\0\0\0\0"
	"\x1c\x00\x00\x00\xe4\xb2\x61\xd0"
	"is_acpi_device_node\0"
	"\x18\x00\x00\x00\x10\x6d\x86\xba"
	"devm_kmalloc\0\0\0\0"
	"\x1c\x00\x00\x00\x8f\x18\x02\x7f"
	"__msecs_to_jiffies\0\0"
	"\x18\x00\x00\x00\x36\xf2\xb6\xc5"
	"queue_work_on\0\0\0"
	"\x20\x00\x00\x00\x58\x89\xb8\xc4"
	"trace_raw_output_prep\0\0\0"
	"\x28\x00\x00\x00\x09\xd7\x89\xab"
	"__trace_trigger_soft_disabled\0\0\0"
	"\x14\x00\x00\x00\xbf\x0f\x54\x92"
	"finish_wait\0"
	"\x24\x00\x00\x00\x14\x24\x33\xfb"
	"mt76_connac_mcu_uni_add_dev\0"
	"\x1c\x00\x00\x00\x07\x33\x85\x0f"
	"request_firmware\0\0\0\0"
	"\x1c\x00\x00\x00\xc8\xa5\xc3\xa0"
	"trace_event_printf\0\0"
	"\x18\x00\x00\x00\x07\x97\x56\x53"
	"this_cpu_off\0\0\0\0"
	"\x24\x00\x00\x00\x6f\x6f\x23\x4c"
	"__x86_indirect_thunk_r15\0\0\0\0"
	"\x2c\x00\x00\x00\xee\x4e\xa9\x7d"
	"ieee80211_iterate_stations_atomic\0\0\0"
	"\x1c\x00\x00\x00\x6a\x57\x2f\x0d"
	"mt76_connac_pm_wake\0"
	"\x20\x00\x00\x00\x00\xf1\xb9\x75"
	"trace_event_raw_init\0\0\0\0"
	"\x10\x00\x00\x00\xba\x0c\x7a\x03"
	"kfree\0\0\0"
	"\x1c\x00\x00\x00\xba\x2b\xca\x7e"
	"mt76_has_tx_pending\0"
	"\x14\x00\x00\x00\xab\x8e\xd3\x43"
	"pcpu_hot\0\0\0\0"
	"\x18\x00\x00\x00\x84\x33\x0c\x55"
	"bpf_trace_run2\0\0"
	"\x20\x00\x00\x00\x95\xd4\x26\x8c"
	"prepare_to_wait_event\0\0\0"
	"\x14\x00\x00\x00\x44\x43\x96\xe2"
	"__wake_up\0\0\0"
	"\x1c\x00\x00\x00\x2e\x14\x9f\x4a"
	"__dynamic_dev_dbg\0\0\0"
	"\x1c\x00\x00\x00\x0a\x76\xdf\x82"
	"mt76_dma_cleanup\0\0\0\0"
	"\x18\x00\x00\x00\x8c\x89\xd4\xcb"
	"fortify_panic\0\0\0"
	"\x24\x00\x00\x00\x3a\x00\x35\xf6"
	"mt76_connac_pm_queue_skb\0\0\0\0"
	"\x14\x00\x00\x00\xbb\x6d\xfb\xbd"
	"__fentry__\0\0"
	"\x20\x00\x00\x00\x57\xd9\xc4\x06"
	"mt76_tx_status_check\0\0\0\0"
	"\x18\x00\x00\x00\x06\x45\xbd\xd9"
	"wake_up_process\0"
	"\x2c\x00\x00\x00\x66\x5b\x54\x7b"
	"mt76_connac_free_pending_tx_skbs\0\0\0\0"
	"\x28\x00\x00\x00\x9a\xd9\xe1\x3e"
	"mt76_connac_power_save_sched\0\0\0\0"
	"\x24\x00\x00\x00\x3d\xed\x44\xeb"
	"trace_event_buffer_commit\0\0\0"
	"\x2c\x00\x00\x00\xd7\xce\xf8\x53"
	"page_pool_ethtool_stats_get_strings\0"
	"\x24\x00\x00\x00\x97\x70\x48\x65"
	"__x86_indirect_thunk_rax\0\0\0\0"
	"\x1c\x00\x00\x00\xd4\x8c\xb2\xcd"
	"napi_complete_done\0\0"
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
	"\x18\x00\x00\x00\xc2\x8c\xd8\xf1"
	"__napi_schedule\0"
	"\x10\x00\x00\x00\x94\xb6\x16\xa9"
	"strnlen\0"
	"\x28\x00\x00\x00\xe1\xee\x21\x65"
	"mt76_connac_mcu_set_mac_enable\0\0"
	"\x18\x00\x00\x00\xef\x66\x0b\xe4"
	"mt76_wcid_alloc\0"
	"\x2c\x00\x00\x00\xa6\xd7\xf1\x23"
	"page_pool_ethtool_stats_get_count\0\0\0"
	"\x28\x00\x00\x00\xb3\x1c\xa2\x87"
	"__ubsan_handle_out_of_bounds\0\0\0\0"
	"\x18\x00\x00\x00\x3e\x66\x22\x53"
	"acpi_get_handle\0"
	"\x18\x00\x00\x00\x75\x79\x48\xfe"
	"init_wait_entry\0"
	"\x28\x00\x00\x00\x20\x77\xa5\x8d"
	"ieee80211_queue_delayed_work\0\0\0\0"
	"\x18\x00\x00\x00\x51\xf9\xcc\x17"
	"devm_kmemdup\0\0\0\0"
	"\x20\x00\x00\x00\x17\x00\x5f\xad"
	"perf_trace_buf_alloc\0\0\0\0"
	"\x24\x00\x00\x00\x33\x96\x73\x1f"
	"perf_trace_run_bpf_submit\0\0\0"
	"\x14\x00\x00\x00\xd1\xb1\x84\x0d"
	"devm_kfree\0\0"
	"\x14\x00\x00\x00\x21\x1d\xd8\xd0"
	"_dev_err\0\0\0\0"
	"\x20\x00\x00\x00\x89\x87\xc3\xd5"
	"mt76_connac2_load_ram\0\0\0"
	"\x20\x00\x00\x00\xad\xc4\xf9\x36"
	"__SCK__tp_func_dev_irq\0\0"
	"\x14\x00\x00\x00\x4b\x8d\xfa\x4d"
	"mutex_lock\0\0"
	"\x18\x00\x00\x00\x8b\x0d\x87\x07"
	"trace_event_reg\0"
	"\x18\x00\x00\x00\xad\x23\x4c\x36"
	"mutex_is_locked\0"
	"\x1c\x00\x00\x00\x77\x86\x3a\x3b"
	"mt76_update_survey\0\0"
	"\x1c\x00\x00\x00\xac\xb8\x2a\x9d"
	"__tasklet_schedule\0\0"
	"\x24\x00\x00\x00\xe9\xc8\x79\x1a"
	"__x86_indirect_thunk_r13\0\0\0\0"
	"\x20\x00\x00\x00\x17\xfb\xea\x35"
	"ieee80211_wake_queues\0\0\0"
	"\x1c\x00\x00\x00\x71\x22\x5a\x5a"
	"__cpu_online_mask\0\0\0"
	"\x20\x00\x00\x00\x39\xce\x3f\x3c"
	"__local_bh_enable_ip\0\0\0\0"
	"\x1c\x00\x00\x00\x70\xfc\xbe\x37"
	"jiffies_to_msecs\0\0\0\0"
	"\x24\x00\x00\x00\x0c\xa4\x8c\x90"
	"mt76_connac_wowlan_support\0\0"
	"\x1c\x00\x00\x00\x02\x2b\xec\xc7"
	"mt76_set_irq_mask\0\0\0"
	"\x24\x00\x00\x00\x72\x1b\xd4\x71"
	"mt76_connac_pm_dequeue_skbs\0"
	"\x1c\x00\x00\x00\xca\x39\x82\x5b"
	"__x86_return_thunk\0\0"
	"\x28\x00\x00\x00\x76\x8b\x2b\xd6"
	"mt76_ethtool_page_pool_stats\0\0\0\0"
	"\x10\x00\x00\x00\xa6\x50\xba\x15"
	"jiffies\0"
	"\x20\x00\x00\x00\xf8\xa1\x46\xeb"
	"__tracepoint_dev_irq\0\0\0\0"
	"\x10\x00\x00\x00\x39\xe6\x64\xdd"
	"strscpy\0"
	"\x2c\x00\x00\x00\x61\xe5\x48\xa6"
	"__ubsan_handle_shift_out_of_bounds\0\0"
	"\x20\x00\x00\x00\xcb\x28\x56\xe4"
	"__SCT__tp_func_dev_irq\0\0"
	"\x24\x00\x00\x00\x07\x6e\x40\xdb"
	"trace_event_buffer_reserve\0\0"
	"\x34\x00\x00\x00\xd8\x6f\x47\xf3"
	"ieee80211_iterate_active_interfaces_atomic\0\0"
	"\x18\x00\x00\x00\x38\xf0\x13\x32"
	"mutex_unlock\0\0\0\0"
	"\x24\x00\x00\x00\x4a\x18\xa7\x9f"
	"cancel_delayed_work_sync\0\0\0\0"
	"\x20\x00\x00\x00\xe6\x00\x99\xc3"
	"mt76_txq_schedule_all\0\0\0"
	"\x18\x00\x00\x00\xd6\xdf\xe3\xea"
	"__const_udelay\0\0"
	"\x20\x00\x00\x00\x67\xe7\x07\xed"
	"mt76_connac2_load_patch\0"
	"\x20\x00\x00\x00\x6b\xeb\x2d\xd9"
	"acpi_evaluate_object\0\0\0\0"
	"\x24\x00\x00\x00\xf9\xa4\xcc\x66"
	"__x86_indirect_thunk_rcx\0\0\0\0"
	"\x1c\x00\x00\x00\xea\x59\xfe\xfa"
	"____mt76_poll_msec\0\0"
	"\x1c\x00\x00\x00\xfe\x2d\xc1\x03"
	"cancel_work_sync\0\0\0\0"
	"\x14\x00\x00\x00\x27\xc8\x2d\x2c"
	"seq_printf\0\0"
	"\x1c\x00\x00\x00\xfc\x90\x36\x0c"
	"_raw_spin_lock_bh\0\0\0"
	"\x1c\x00\x00\x00\x5d\xce\xef\x86"
	"mt76_wcid_cleanup\0\0\0"
	"\x14\x00\x00\x00\x6d\x6a\x33\x78"
	"seq_puts\0\0\0\0"
	"\x20\x00\x00\x00\xca\x5f\xfd\x11"
	"ieee80211_queue_work\0\0\0\0"
	"\x1c\x00\x00\x00\x88\xdb\xd5\x56"
	"napi_schedule_prep\0\0"
	"\x28\x00\x00\x00\xe1\x72\x8e\x68"
	"__SCT__preempt_schedule_notrace\0"
	"\x1c\x00\x00\x00\xe3\x20\xd1\x85"
	"trace_handle_return\0"
	"\x1c\x00\x00\x00\xb8\x88\x9d\xab"
	"mt76_ethtool_worker\0"
	"\x20\x00\x00\x00\x85\x1e\x0a\xf9"
	"__x86_indirect_thunk_r8\0"
	"\x10\x00\x00\x00\xf9\x82\xa4\xf9"
	"msleep\0\0"
	"\x20\x00\x00\x00\x12\xda\xf0\xc4"
	"ktime_get_with_offset\0\0\0"
	"\x20\x00\x00\x00\x5d\x7b\xc1\xe2"
	"__SCT__might_resched\0\0\0\0"
	"\x18\x00\x00\x00\x72\x3f\x86\xba"
	"module_layout\0\0\0"
	"\x00\x00\x00\x00\x00\x00\x00\x00";

MODULE_INFO(depends, "mt76,mt76-connac-lib,mac80211");


MODULE_INFO(srcversion, "75E480489CCD9DD3DD1AB61");
