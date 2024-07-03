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

KSYMTAB_FUNC(mt76x0_chip_onoff, "_gpl", "");
KSYMTAB_FUNC(mt76x0_mac_stop, "_gpl", "");
KSYMTAB_FUNC(mt76x0_init_hardware, "_gpl", "");
KSYMTAB_FUNC(mt76x0_register_device, "_gpl", "");
KSYMTAB_FUNC(mt76x0_set_sar_specs, "_gpl", "");
KSYMTAB_FUNC(mt76x0_config, "_gpl", "");
KSYMTAB_FUNC(mt76x0_phy_calibrate, "_gpl", "");

SYMBOL_CRC(mt76x0_chip_onoff, 0x973f48a4, "_gpl");
SYMBOL_CRC(mt76x0_mac_stop, 0x68e38afe, "_gpl");
SYMBOL_CRC(mt76x0_init_hardware, 0xcea359d4, "_gpl");
SYMBOL_CRC(mt76x0_register_device, 0x644f95b6, "_gpl");
SYMBOL_CRC(mt76x0_set_sar_specs, 0x9eb88a3f, "_gpl");
SYMBOL_CRC(mt76x0_config, 0x9133a8e5, "_gpl");
SYMBOL_CRC(mt76x0_phy_calibrate, 0x36d5e5c7, "_gpl");

static const char ____versions[]
__used __section("__versions") =
	"\x1c\x00\x00\x00\x93\x76\x58\xa4"
	"mt76_init_sar_power\0"
	"\x24\x00\x00\x00\x9c\x5c\xbd\x0e"
	"mt76x02_phy_adjust_vga_gain\0"
	"\x24\x00\x00\x00\xde\x12\xc6\x9e"
	"mt76x02_phy_dfs_adjust_agc\0\0"
	"\x1c\x00\x00\x00\x20\x5d\x05\xc3"
	"usleep_range_state\0\0"
	"\x20\x00\x00\x00\x23\x24\x1c\x73"
	"cfg80211_chandef_valid\0\0"
	"\x1c\x00\x00\x00\x2e\x14\x9f\x4a"
	"__dynamic_dev_dbg\0\0\0"
	"\x14\x00\x00\x00\xbb\x6d\xfb\xbd"
	"__fentry__\0\0"
	"\x28\x00\x00\x00\xc7\xe1\x35\xc6"
	"mt76x02_mac_shared_key_setup\0\0\0\0"
	"\x20\x00\x00\x00\xdb\x4b\x46\xf2"
	"mt76x02_ext_pa_enabled\0\0"
	"\x24\x00\x00\x00\x97\x70\x48\x65"
	"__x86_indirect_thunk_rax\0\0\0\0"
	"\x20\x00\x00\x00\x2e\x6b\x60\xff"
	"mt76x02_get_efuse_data\0\0"
	"\x1c\x00\x00\x00\xcb\xf6\xfd\xf0"
	"__stack_chk_fail\0\0\0\0"
	"\x14\x00\x00\x00\x22\x23\xa2\x87"
	"_dev_info\0\0\0"
	"\x28\x00\x00\x00\xb3\x1c\xa2\x87"
	"__ubsan_handle_out_of_bounds\0\0\0\0"
	"\x1c\x00\x00\x00\xa6\x21\x18\x7e"
	"mt76x02_get_rx_gain\0"
	"\x28\x00\x00\x00\x5d\xf8\xfe\x75"
	"mt76x02_add_rate_power_offset\0\0\0"
	"\x28\x00\x00\x00\x20\x77\xa5\x8d"
	"ieee80211_queue_delayed_work\0\0\0\0"
	"\x1c\x00\x00\x00\x2d\x71\xbc\x55"
	"mt76_eeprom_init\0\0\0\0"
	"\x14\x00\x00\x00\x21\x1d\xd8\xd0"
	"_dev_err\0\0\0\0"
	"\x1c\x00\x00\x00\xa7\x74\x61\xac"
	"mt76x02_mac_setaddr\0"
	"\x20\x00\x00\x00\xea\x24\x94\x1a"
	"mt76_register_device\0\0\0\0"
	"\x24\x00\x00\x00\x2e\x5e\x38\x55"
	"__x86_indirect_thunk_r14\0\0\0\0"
	"\x1c\x00\x00\x00\xf0\xb4\x6c\xf3"
	"mt76x02_phy_set_bw\0\0"
	"\x14\x00\x00\x00\x4b\x8d\xfa\x4d"
	"mutex_lock\0\0"
	"\x20\x00\x00\x00\x8b\xb6\x06\x04"
	"mt76x02_phy_set_band\0\0\0\0"
	"\x1c\x00\x00\x00\x22\x23\xc3\x54"
	"mt76_set_channel\0\0\0\0"
	"\x1c\x00\x00\x00\x60\x19\xa2\x8e"
	"mt76x02_init_device\0"
	"\x20\x00\x00\x00\x17\xfb\xea\x35"
	"ieee80211_wake_queues\0\0\0"
	"\x24\x00\x00\x00\xaa\x8a\x4d\xfb"
	"mt76x02_eeprom_parse_hw_cap\0"
	"\x20\x00\x00\x00\x48\x10\x88\x18"
	"mt76x02_phy_set_rxpath\0\0"
	"\x1c\x00\x00\x00\x9b\x16\x69\x7b"
	"mt76x02_edcca_init\0\0"
	"\x20\x00\x00\x00\x4d\xe6\x71\xde"
	"mt76_eeprom_override\0\0\0\0"
	"\x20\x00\x00\x00\x13\x21\x46\xc0"
	"mt76x02_get_lna_gain\0\0\0\0"
	"\x20\x00\x00\x00\xd5\x24\x64\xbf"
	"mt76x02_init_debugfs\0\0\0\0"
	"\x14\x00\x00\x00\x01\x6b\x35\x83"
	"_dev_warn\0\0\0"
	"\x1c\x00\x00\x00\xca\x39\x82\x5b"
	"__x86_return_thunk\0\0"
	"\x20\x00\x00\x00\x63\x77\x71\x78"
	"ieee80211_stop_queues\0\0\0"
	"\x20\x00\x00\x00\x87\x4f\x9d\x8a"
	"mt76x02_mac_cc_reset\0\0\0\0"
	"\x20\x00\x00\x00\x4c\x69\x80\x5c"
	"mt76x02_init_agc_gain\0\0\0"
	"\x18\x00\x00\x00\x38\xf0\x13\x32"
	"mutex_unlock\0\0\0\0"
	"\x24\x00\x00\x00\x4a\x18\xa7\x9f"
	"cancel_delayed_work_sync\0\0\0\0"
	"\x20\x00\x00\x00\xe6\x00\x99\xc3"
	"mt76_txq_schedule_all\0\0\0"
	"\x18\x00\x00\x00\x39\x63\xf4\xc6"
	"init_timer_key\0\0"
	"\x18\x00\x00\x00\xd6\xdf\xe3\xea"
	"__const_udelay\0\0"
	"\x24\x00\x00\x00\xf9\xa4\xcc\x66"
	"__x86_indirect_thunk_rcx\0\0\0\0"
	"\x1c\x00\x00\x00\xea\x59\xfe\xfa"
	"____mt76_poll_msec\0\0"
	"\x20\x00\x00\x00\x4b\x9a\xce\x14"
	"mt76x02_dfs_init_params\0"
	"\x20\x00\x00\x00\x6a\xdf\xee\xff"
	"delayed_work_timer_fn\0\0\0"
	"\x28\x00\x00\x00\xf9\xd0\xab\x58"
	"mt76x02_config_mac_addr_list\0\0\0\0"
	"\x1c\x00\x00\x00\x3e\xb8\x15\x10"
	"mt76_get_sar_power\0\0"
	"\x1c\x00\x00\x00\x42\x4a\x4e\x9a"
	"mt76x02_eeprom_copy\0"
	"\x20\x00\x00\x00\x64\xec\xef\xea"
	"mt76x02_mcu_calibrate\0\0\0"
	"\x24\x00\x00\x00\xd8\x5e\xed\xdb"
	"mt76x02_mcu_function_select\0"
	"\x20\x00\x00\x00\x94\x28\x2c\x78"
	"mt76x02_phy_set_txpower\0"
	"\x24\x00\x00\x00\xa8\x3b\x5d\xad"
	"mt76x02_get_max_rate_power\0\0"
	"\x20\x00\x00\x00\x44\x92\x4c\x83"
	"mt76x02_mac_wcid_setup\0\0"
	"\x2c\x00\x00\x00\xc6\xfa\xb1\x54"
	"__ubsan_handle_load_invalid_value\0\0\0"
	"\x14\x00\x00\x00\xce\x1b\xe1\x88"
	"__mt76_poll\0"
	"\x1c\x00\x00\x00\xe5\x6c\x0f\x35"
	"tasklet_unlock_wait\0"
	"\x20\x00\x00\x00\x03\xc1\x14\x74"
	"mt76x02_phy_set_txdac\0\0\0"
	"\x24\x00\x00\x00\xbc\x79\xc4\x93"
	"mt76x02_limit_rate_power\0\0\0\0"
	"\x20\x00\x00\x00\x9d\xc9\x85\x3c"
	"mt76_get_min_avg_rssi\0\0\0"
	"\x18\x00\x00\x00\xd5\x4f\xd8\x5b"
	"mt76x02_rates\0\0\0"
	"\x10\x00\x00\x00\xf9\x82\xa4\xf9"
	"msleep\0\0"
	"\x18\x00\x00\x00\x72\x3f\x86\xba"
	"module_layout\0\0\0"
	"\x00\x00\x00\x00\x00\x00\x00\x00";

MODULE_INFO(depends, "mt76,mt76x02-lib,cfg80211,mac80211");


MODULE_INFO(srcversion, "CE2DD35AF09A8753CBEF0D2");
