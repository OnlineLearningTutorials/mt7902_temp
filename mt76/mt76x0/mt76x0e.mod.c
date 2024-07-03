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
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};

#ifdef CONFIG_RETPOLINE
MODULE_INFO(retpoline, "Y");
#endif



static const char ____versions[]
__used __section("__versions") =
	"\x1c\x00\x00\x00\x19\xe3\xcd\x9a"
	"mt76x02_dma_init\0\0\0\0"
	"\x18\x00\x00\x00\xae\x5e\x41\x48"
	"pci_save_state\0\0"
	"\x1c\x00\x00\x00\xa9\x9a\xd0\xc6"
	"release_firmware\0\0\0\0"
	"\x24\x00\x00\x00\x6d\xdb\x92\x76"
	"devm_request_threaded_irq\0\0\0"
	"\x20\x00\x00\x00\x3f\x8a\xb8\x9e"
	"mt76x0_set_sar_specs\0\0\0\0"
	"\x1c\x00\x00\x00\x92\x46\x05\xff"
	"pci_choose_state\0\0\0\0"
	"\x24\x00\x00\x00\xbe\xcf\x6b\x5a"
	"mt76x02_set_ethtool_fwver\0\0\0"
	"\x18\x00\x00\x00\xff\x16\x13\x6c"
	"mt76x02_set_key\0"
	"\x24\x00\x00\x00\x4b\x29\x81\x6f"
	"mt76x02_mcu_parse_response\0\0"
	"\x28\x00\x00\x00\xa6\x32\xb6\xcd"
	"mt76_release_buffered_frames\0\0\0\0"
	"\x1c\x00\x00\x00\x07\x33\x85\x0f"
	"request_firmware\0\0\0\0"
	"\x1c\x00\x00\x00\x89\xa0\x7c\xc5"
	"mt76_get_txpower\0\0\0\0"
	"\x20\x00\x00\x00\x51\x4e\x78\x6d"
	"__pci_register_driver\0\0\0"
	"\x1c\x00\x00\x00\x6c\x26\xb0\x87"
	"mt76x02_dma_disable\0"
	"\x1c\x00\x00\x00\x4e\x41\x11\x03"
	"mt76_wake_tx_queue\0\0"
	"\x24\x00\x00\x00\xb3\xaa\x68\x32"
	"mt76x02e_init_beacon_config\0"
	"\x1c\x00\x00\x00\x08\xf7\xee\x1c"
	"mt76_get_antenna\0\0\0\0"
	"\x14\x00\x00\x00\xab\x8e\xd3\x43"
	"pcpu_hot\0\0\0\0"
	"\x1c\x00\x00\x00\x20\x5d\x05\xc3"
	"usleep_range_state\0\0"
	"\x1c\x00\x00\x00\x50\x19\x85\x02"
	"pci_set_power_state\0"
	"\x18\x00\x00\x00\x13\x4d\xc9\x5c"
	"mt76x02_sta_ps\0\0"
	"\x24\x00\x00\x00\x1e\xd3\xe0\x77"
	"mt76x02_sta_rate_tbl_update\0"
	"\x18\x00\x00\x00\x03\x9b\x1b\x9e"
	"kthread_park\0\0\0\0"
	"\x1c\x00\x00\x00\x2e\x14\x9f\x4a"
	"__dynamic_dev_dbg\0\0\0"
	"\x1c\x00\x00\x00\x0a\x76\xdf\x82"
	"mt76_dma_cleanup\0\0\0\0"
	"\x20\x00\x00\x00\x4a\xee\xab\xaa"
	"pci_unregister_driver\0\0\0"
	"\x14\x00\x00\x00\xbb\x6d\xfb\xbd"
	"__fentry__\0\0"
	"\x24\x00\x00\x00\x21\x23\xc8\x4c"
	"mt76x02_reconfig_complete\0\0\0"
	"\x1c\x00\x00\x00\xe1\xd7\xaf\xce"
	"mt76_alloc_device\0\0\0"
	"\x18\x00\x00\x00\x06\x45\xbd\xd9"
	"wake_up_process\0"
	"\x24\x00\x00\x00\xfb\x9d\x96\x50"
	"mt76x02_set_coverage_class\0\0"
	"\x24\x00\x00\x00\x97\x70\x48\x65"
	"__x86_indirect_thunk_rax\0\0\0\0"
	"\x1c\x00\x00\x00\xcb\xf6\xfd\xf0"
	"__stack_chk_fail\0\0\0\0"
	"\x20\x00\x00\x00\x69\xea\x4e\x0e"
	"mt76x02_update_channel\0\0"
	"\x18\x00\x00\x00\xc2\x8c\xd8\xf1"
	"__napi_schedule\0"
	"\x20\x00\x00\x00\x4a\x9d\x44\x48"
	"mt76_pci_disable_aspm\0\0\0"
	"\x14\x00\x00\x00\x22\x23\xa2\x87"
	"_dev_info\0\0\0"
	"\x24\x00\x00\x00\x87\x77\x83\x70"
	"mt76x02_configure_filter\0\0\0\0"
	"\x1c\x00\x00\x00\x05\x9e\x4f\x7a"
	"mt76_free_device\0\0\0\0"
	"\x1c\x00\x00\x00\xa4\x48\x3f\x97"
	"mt76x0_chip_onoff\0\0\0"
	"\x28\x00\x00\x00\xb3\x1c\xa2\x87"
	"__ubsan_handle_out_of_bounds\0\0\0\0"
	"\x20\x00\x00\x00\x2b\x33\x5e\x8f"
	"mt76x02_queue_rx_skb\0\0\0\0"
	"\x18\x00\x00\x00\xf7\xb3\xf3\x66"
	"kthread_unpark\0\0"
	"\x1c\x00\x00\x00\x31\xc3\xf7\xc1"
	"pcim_iomap_regions\0\0"
	"\x28\x00\x00\x00\x20\x77\xa5\x8d"
	"ieee80211_queue_delayed_work\0\0\0\0"
	"\x18\x00\x00\x00\xe5\xa8\x33\x91"
	"mt76x0_config\0\0\0"
	"\x20\x00\x00\x00\xb6\x95\x4f\x64"
	"mt76x0_register_device\0\0"
	"\x14\x00\x00\x00\x21\x1d\xd8\xd0"
	"_dev_err\0\0\0\0"
	"\x24\x00\x00\x00\xf2\xe9\xee\xd4"
	"mt76x02_rx_poll_complete\0\0\0\0"
	"\x20\x00\x00\x00\xc7\xe5\xd5\x36"
	"mt76x0_phy_calibrate\0\0\0\0"
	"\x18\x00\x00\x00\x9f\x70\xb6\x90"
	"mt76_set_tim\0\0\0\0"
	"\x24\x00\x00\x00\x9d\xdb\xb4\xe4"
	"mt76x02_sw_scan_complete\0\0\0\0"
	"\x1c\x00\x00\x00\xee\xb9\xd5\x91"
	"mt76x02_irq_handler\0"
	"\x14\x00\x00\x00\xd4\x4f\x79\x02"
	"napi_enable\0"
	"\x14\x00\x00\x00\x0d\xc2\x0d\x53"
	"mt76x02_tx\0\0"
	"\x18\x00\x00\x00\x2b\xed\x4d\xfa"
	"mt76_mmio_init\0\0"
	"\x1c\x00\x00\x00\x48\x94\x1a\xf4"
	"mt76x02_sta_remove\0\0"
	"\x20\x00\x00\x00\x39\xce\x3f\x3c"
	"__local_bh_enable_ip\0\0\0\0"
	"\x18\x00\x00\x00\x9f\x0c\xfb\xce"
	"__mutex_init\0\0\0\0"
	"\x18\x00\x00\x00\xe3\x7b\xa0\x1c"
	"mt76x02_sta_add\0"
	"\x20\x00\x00\x00\xd0\xad\x8c\x88"
	"mt76_unregister_device\0\0"
	"\x1c\x00\x00\x00\xc8\xe8\x07\x02"
	"pci_restore_state\0\0\0"
	"\x18\x00\x00\x00\xfe\x8a\xe3\x68"
	"mt76x0_mac_stop\0"
	"\x18\x00\x00\x00\xb7\x28\xd7\xa4"
	"mt76_get_survey\0"
	"\x14\x00\x00\x00\x01\x6b\x35\x83"
	"_dev_warn\0\0\0"
	"\x18\x00\x00\x00\xeb\xf3\x6c\xdd"
	"pci_set_master\0\0"
	"\x1c\x00\x00\x00\xca\x39\x82\x5b"
	"__x86_return_thunk\0\0"
	"\x24\x00\x00\x00\x52\xce\xd6\x38"
	"mt76x02_set_rts_threshold\0\0\0"
	"\x20\x00\x00\x00\x49\xb6\xd9\x13"
	"mt76x02_tx_complete_skb\0"
	"\x18\x00\x00\x00\x22\xa5\x49\x4c"
	"mt76_sta_state\0\0"
	"\x20\x00\x00\x00\xe7\xc5\xdb\x9e"
	"mt76x02_mcu_msg_send\0\0\0\0"
	"\x20\x00\x00\x00\xd4\x59\xa3\xce"
	"mt76x0_init_hardware\0\0\0\0"
	"\x24\x00\x00\x00\x7f\x05\xf5\x05"
	"mt76x02_bss_info_changed\0\0\0\0"
	"\x18\x00\x00\x00\x4d\x7f\x25\x44"
	"pci_enable_wake\0"
	"\x24\x00\x00\x00\x4a\x18\xa7\x9f"
	"cancel_delayed_work_sync\0\0\0\0"
	"\x20\x00\x00\x00\x4b\x1e\x2f\x9f"
	"mt76x02_add_interface\0\0\0"
	"\x1c\x00\x00\x00\xea\x59\xfe\xfa"
	"____mt76_poll_msec\0\0"
	"\x20\x00\x00\x00\xc4\x2f\x11\xe0"
	"__x86_indirect_thunk_r9\0"
	"\x20\x00\x00\x00\x1b\x32\x90\x51"
	"mt76x02_tx_prepare_skb\0\0"
	"\x1c\x00\x00\x00\x66\x63\xff\x5c"
	"mt76x02_mcu_cleanup\0"
	"\x20\x00\x00\x00\x2d\x9a\x6b\x7e"
	"mt76x02_ampdu_action\0\0\0\0"
	"\x18\x00\x00\x00\xeb\x2c\x36\xf0"
	"mt76x02_conf_tx\0"
	"\x18\x00\x00\x00\x49\xb8\x7d\xef"
	"dma_set_mask\0\0\0\0"
	"\x18\x00\x00\x00\x55\xaf\xb6\xad"
	"mt76_sw_scan\0\0\0\0"
	"\x20\x00\x00\x00\xfe\xcd\x76\xca"
	"mt76_sta_pre_rcu_remove\0"
	"\x24\x00\x00\x00\xa8\xf9\x62\x03"
	"__x86_indirect_thunk_r12\0\0\0\0"
	"\x1c\x00\x00\x00\x5f\x6d\x62\x06"
	"pcim_iomap_table\0\0\0\0"
	"\x1c\x00\x00\x00\x88\xdb\xd5\x56"
	"napi_schedule_prep\0\0"
	"\x18\x00\x00\x00\xf8\x83\xcf\x2a"
	"napi_disable\0\0\0\0"
	"\x14\x00\x00\x00\xce\x1b\xe1\x88"
	"__mt76_poll\0"
	"\x1c\x00\x00\x00\x42\x25\x4d\x79"
	"pcim_enable_device\0\0"
	"\x1c\x00\x00\x00\xe5\x6c\x0f\x35"
	"tasklet_unlock_wait\0"
	"\x1c\x00\x00\x00\xdc\x70\x10\x63"
	"mt76x02_mac_start\0\0\0"
	"\x24\x00\x00\x00\xb5\xeb\xfb\xa7"
	"mt76x02_remove_interface\0\0\0\0"
	"\x18\x00\x00\x00\x72\x3f\x86\xba"
	"module_layout\0\0\0"
	"\x00\x00\x00\x00\x00\x00\x00\x00";

MODULE_INFO(depends, "mt76x02-lib,mt76x0-common,mt76,mac80211");

MODULE_ALIAS("pci:v000014C3d00007610sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v000014C3d00007630sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v000014C3d00007650sv*sd*bc*sc*i*");

MODULE_INFO(srcversion, "89E00C21F06663F58D90809");