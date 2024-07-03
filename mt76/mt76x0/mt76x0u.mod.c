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
	"\x24\x00\x00\x00\x17\xef\x1c\x71"
	"mt76x02u_tx_complete_skb\0\0\0\0"
	"\x1c\x00\x00\x00\xa9\x9a\xd0\xc6"
	"release_firmware\0\0\0\0"
	"\x18\x00\x00\x00\x10\x6d\x86\xba"
	"devm_kmalloc\0\0\0\0"
	"\x20\x00\x00\x00\x3f\x8a\xb8\x9e"
	"mt76x0_set_sar_specs\0\0\0\0"
	"\x20\x00\x00\x00\x75\x91\x71\x98"
	"firmware_request_nowarn\0"
	"\x24\x00\x00\x00\xbe\xcf\x6b\x5a"
	"mt76x02_set_ethtool_fwver\0\0\0"
	"\x18\x00\x00\x00\xff\x16\x13\x6c"
	"mt76x02_set_key\0"
	"\x28\x00\x00\x00\xa6\x32\xb6\xcd"
	"mt76_release_buffered_frames\0\0\0\0"
	"\x1c\x00\x00\x00\x07\x33\x85\x0f"
	"request_firmware\0\0\0\0"
	"\x1c\x00\x00\x00\xff\xe0\x58\x5e"
	"usb_register_driver\0"
	"\x1c\x00\x00\x00\x89\xa0\x7c\xc5"
	"mt76_get_txpower\0\0\0\0"
	"\x1c\x00\x00\x00\x4e\x41\x11\x03"
	"mt76_wake_tx_queue\0\0"
	"\x1c\x00\x00\x00\x08\xf7\xee\x1c"
	"mt76_get_antenna\0\0\0\0"
	"\x10\x00\x00\x00\xba\x0c\x7a\x03"
	"kfree\0\0\0"
	"\x1c\x00\x00\x00\x20\x5d\x05\xc3"
	"usleep_range_state\0\0"
	"\x18\x00\x00\x00\x13\x4d\xc9\x5c"
	"mt76x02_sta_ps\0\0"
	"\x18\x00\x00\x00\x72\xea\x6e\xa6"
	"mt76u_resume_rx\0"
	"\x24\x00\x00\x00\x1e\xd3\xe0\x77"
	"mt76x02_sta_rate_tbl_update\0"
	"\x1c\x00\x00\x00\x2e\x14\x9f\x4a"
	"__dynamic_dev_dbg\0\0\0"
	"\x14\x00\x00\x00\xbb\x6d\xfb\xbd"
	"__fentry__\0\0"
	"\x1c\x00\x00\x00\xe1\xd7\xaf\xce"
	"mt76_alloc_device\0\0\0"
	"\x24\x00\x00\x00\x97\x70\x48\x65"
	"__x86_indirect_thunk_rax\0\0\0\0"
	"\x14\x00\x00\x00\xcf\xbf\xfe\x7b"
	"usb_put_dev\0"
	"\x1c\x00\x00\x00\x20\x22\xca\xf8"
	"usb_reset_device\0\0\0\0"
	"\x1c\x00\x00\x00\xcb\xf6\xfd\xf0"
	"__stack_chk_fail\0\0\0\0"
	"\x20\x00\x00\x00\x69\xea\x4e\x0e"
	"mt76x02_update_channel\0\0"
	"\x14\x00\x00\x00\xd4\x34\xf9\x16"
	"usb_get_dev\0"
	"\x14\x00\x00\x00\x22\x23\xa2\x87"
	"_dev_info\0\0\0"
	"\x24\x00\x00\x00\x87\x77\x83\x70"
	"mt76x02_configure_filter\0\0\0\0"
	"\x1c\x00\x00\x00\x05\x9e\x4f\x7a"
	"mt76_free_device\0\0\0\0"
	"\x1c\x00\x00\x00\xa4\x48\x3f\x97"
	"mt76x0_chip_onoff\0\0\0"
	"\x20\x00\x00\x00\x2b\x33\x5e\x8f"
	"mt76x02_queue_rx_skb\0\0\0\0"
	"\x1c\x00\x00\x00\xbf\x89\xf5\xac"
	"mt76u_queues_deinit\0"
	"\x28\x00\x00\x00\x20\x77\xa5\x8d"
	"ieee80211_queue_delayed_work\0\0\0\0"
	"\x18\x00\x00\x00\xe5\xa8\x33\x91"
	"mt76x0_config\0\0\0"
	"\x20\x00\x00\x00\xb6\x95\x4f\x64"
	"mt76x0_register_device\0\0"
	"\x14\x00\x00\x00\x21\x1d\xd8\xd0"
	"_dev_err\0\0\0\0"
	"\x20\x00\x00\x00\xc4\x32\x75\xad"
	"mt76x02u_mcu_fw_reset\0\0\0"
	"\x20\x00\x00\x00\xc7\xe5\xd5\x36"
	"mt76x0_phy_calibrate\0\0\0\0"
	"\x18\x00\x00\x00\x9f\x70\xb6\x90"
	"mt76_set_tim\0\0\0\0"
	"\x24\x00\x00\x00\x9d\xdb\xb4\xe4"
	"mt76x02_sw_scan_complete\0\0\0\0"
	"\x14\x00\x00\x00\x0d\xc2\x0d\x53"
	"mt76x02_tx\0\0"
	"\x1c\x00\x00\x00\x48\x94\x1a\xf4"
	"mt76x02_sta_remove\0\0"
	"\x18\x00\x00\x00\x9f\x0c\xfb\xce"
	"__mutex_init\0\0\0\0"
	"\x18\x00\x00\x00\x07\x08\x9d\x3a"
	"usb_deregister\0\0"
	"\x20\x00\x00\x00\xea\xd0\xc6\x4f"
	"mt76x02_tx_status_data\0\0"
	"\x18\x00\x00\x00\xe3\x7b\xa0\x1c"
	"mt76x02_sta_add\0"
	"\x18\x00\x00\x00\xfe\x8a\xe3\x68"
	"mt76x0_mac_stop\0"
	"\x24\x00\x00\x00\xc5\x52\xab\x11"
	"mt76x02u_mcu_fw_send_data\0\0\0"
	"\x14\x00\x00\x00\x13\x17\x10\x7b"
	"mt76u_init\0\0"
	"\x18\x00\x00\x00\xb7\x28\xd7\xa4"
	"mt76_get_survey\0"
	"\x14\x00\x00\x00\x01\x6b\x35\x83"
	"_dev_warn\0\0\0"
	"\x1c\x00\x00\x00\x02\x70\x81\xc6"
	"mt76x02u_init_mcu\0\0\0"
	"\x20\x00\x00\x00\xfb\xda\xf9\xa4"
	"mt76x02u_tx_prepare_skb\0"
	"\x1c\x00\x00\x00\xca\x39\x82\x5b"
	"__x86_return_thunk\0\0"
	"\x24\x00\x00\x00\x52\xce\xd6\x38"
	"mt76x02_set_rts_threshold\0\0\0"
	"\x18\x00\x00\x00\x22\xa5\x49\x4c"
	"mt76_sta_state\0\0"
	"\x20\x00\x00\x00\xd4\x59\xa3\xce"
	"mt76x0_init_hardware\0\0\0\0"
	"\x1c\x00\x00\x00\x23\xc9\xdf\x84"
	"mt76x02u_mac_start\0\0"
	"\x24\x00\x00\x00\x7f\x05\xf5\x05"
	"mt76x02_bss_info_changed\0\0\0\0"
	"\x24\x00\x00\x00\x4a\x18\xa7\x9f"
	"cancel_delayed_work_sync\0\0\0\0"
	"\x1c\x00\x00\x00\xeb\xda\xc8\xc1"
	"mt76u_alloc_queues\0\0"
	"\x18\x00\x00\x00\x96\x23\x3c\x3a"
	"mt76u_stop_tx\0\0\0"
	"\x24\x00\x00\x00\x42\xd0\x6c\x0e"
	"mt76x02u_init_beacon_config\0"
	"\x20\x00\x00\x00\xac\x35\x97\xb0"
	"ieee80211_unregister_hw\0"
	"\x20\x00\x00\x00\xa9\x3d\xa4\x8b"
	"mt76u_vendor_request\0\0\0\0"
	"\x10\x00\x00\x00\x41\xcc\xb4\x66"
	"kmemdup\0"
	"\x20\x00\x00\x00\x4b\x1e\x2f\x9f"
	"mt76x02_add_interface\0\0\0"
	"\x1c\x00\x00\x00\xea\x59\xfe\xfa"
	"____mt76_poll_msec\0\0"
	"\x24\x00\x00\x00\xff\x8c\x27\x51"
	"mt76x02u_exit_beacon_config\0"
	"\x18\x00\x00\x00\x12\xd9\x14\x58"
	"mt76u_stop_rx\0\0\0"
	"\x20\x00\x00\x00\x2d\x9a\x6b\x7e"
	"mt76x02_ampdu_action\0\0\0\0"
	"\x18\x00\x00\x00\xeb\x2c\x36\xf0"
	"mt76x02_conf_tx\0"
	"\x18\x00\x00\x00\x55\xaf\xb6\xad"
	"mt76_sw_scan\0\0\0\0"
	"\x20\x00\x00\x00\xfe\xcd\x76\xca"
	"mt76_sta_pre_rcu_remove\0"
	"\x2c\x00\x00\x00\xc6\xfa\xb1\x54"
	"__ubsan_handle_load_invalid_value\0\0\0"
	"\x14\x00\x00\x00\xce\x1b\xe1\x88"
	"__mt76_poll\0"
	"\x24\x00\x00\x00\xb5\xeb\xfb\xa7"
	"mt76x02_remove_interface\0\0\0\0"
	"\x18\x00\x00\x00\x72\x3f\x86\xba"
	"module_layout\0\0\0"
	"\x00\x00\x00\x00\x00\x00\x00\x00";

MODULE_INFO(depends, "mt76x02-usb,mt76x0-common,mt76x02-lib,mt76,mt76-usb,mac80211");

MODULE_ALIAS("usb:v148Fp7610d*dc*dsc*dp*ic*isc*ip*in*");
MODULE_ALIAS("usb:v13B1p003Ed*dc*dsc*dp*ic*isc*ip*in*");
MODULE_ALIAS("usb:v0E8Dp7610d*dc*dsc*dp*ic*isc*ip*in*");
MODULE_ALIAS("usb:v7392pA711d*dc*dsc*dp*ic*isc*ip*in*");
MODULE_ALIAS("usb:v7392pB711d*dc*dsc*dp*ic*isc*ip*in*");
MODULE_ALIAS("usb:v148Fp761Ad*dc*dsc*dp*ic*isc*ip*in*");
MODULE_ALIAS("usb:v148Fp760Ad*dc*dsc*dp*ic*isc*ip*in*");
MODULE_ALIAS("usb:v0B05p17D1d*dc*dsc*dp*ic*isc*ip*in*");
MODULE_ALIAS("usb:v0B05p17DBd*dc*dsc*dp*ic*isc*ip*in*");
MODULE_ALIAS("usb:v0DF6p0075d*dc*dsc*dp*ic*isc*ip*in*");
MODULE_ALIAS("usb:v2019pAB31d*dc*dsc*dp*ic*isc*ip*in*");
MODULE_ALIAS("usb:v2001p3D02d*dc*dsc*dp*ic*isc*ip*in*");
MODULE_ALIAS("usb:v0586p3425d*dc*dsc*dp*ic*isc*ip*in*");
MODULE_ALIAS("usb:v07B8p7610d*dc*dsc*dp*ic*isc*ip*in*");
MODULE_ALIAS("usb:v04BBp0951d*dc*dsc*dp*ic*isc*ip*in*");
MODULE_ALIAS("usb:v057Cp8502d*dc*dsc*dp*ic*isc*ip*in*");
MODULE_ALIAS("usb:v293Cp5702d*dc*dsc*dp*ic*isc*ip*in*");
MODULE_ALIAS("usb:v20F4p806Bd*dc*dsc*dp*ic*isc*ip*in*");
MODULE_ALIAS("usb:v7392pC711d*dc*dsc*dp*ic*isc*ip*in*");
MODULE_ALIAS("usb:v0DF6p0079d*dc*dsc*dp*ic*isc*ip*in*");
MODULE_ALIAS("usb:v2357p0123d*dc*dsc*dp*ic*isc*ip*in*");
MODULE_ALIAS("usb:v2357p010Bd*dc*dsc*dp*ic*isc*ip*in*");
MODULE_ALIAS("usb:v2357p0105d*dc*dsc*dp*ic*isc*ip*in*");
MODULE_ALIAS("usb:v0E8Dp7630d*dc*dsc*dp*icFFisc02ipFFin*");
MODULE_ALIAS("usb:v0E8Dp7650d*dc*dsc*dp*icFFisc02ipFFin*");

MODULE_INFO(srcversion, "3F876D49879E6C24E2C91DE");
