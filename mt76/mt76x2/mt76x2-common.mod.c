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

KSYMTAB_FUNC(mt76x2_read_rx_gain, "_gpl", "");
KSYMTAB_FUNC(mt76x2_get_rate_power, "_gpl", "");
KSYMTAB_FUNC(mt76x2_get_power_info, "_gpl", "");
KSYMTAB_FUNC(mt76x2_get_temp_comp, "_gpl", "");
KSYMTAB_FUNC(mt76x2_eeprom_init, "_gpl", "");
KSYMTAB_FUNC(mt76x2_mac_stop, "_gpl", "");
KSYMTAB_FUNC(mt76x2_set_sar_specs, "_gpl", "");
KSYMTAB_FUNC(mt76x2_reset_wlan, "_gpl", "");
KSYMTAB_FUNC(mt76_write_mac_initvals, "_gpl", "");
KSYMTAB_FUNC(mt76x2_init_txpower, "_gpl", "");
KSYMTAB_FUNC(mt76x2_apply_gain_adj, "_gpl", "");
KSYMTAB_FUNC(mt76x2_phy_set_txpower_regs, "_gpl", "");
KSYMTAB_FUNC(mt76x2_phy_set_txpower, "_gpl", "");
KSYMTAB_FUNC(mt76x2_configure_tx_delay, "_gpl", "");
KSYMTAB_FUNC(mt76x2_phy_tssi_compensate, "_gpl", "");
KSYMTAB_FUNC(mt76x2_phy_update_channel_gain, "_gpl", "");
KSYMTAB_FUNC(mt76x2_mcu_set_channel, "_gpl", "");
KSYMTAB_FUNC(mt76x2_mcu_load_cr, "_gpl", "");
KSYMTAB_FUNC(mt76x2_mcu_init_gain, "_gpl", "");
KSYMTAB_FUNC(mt76x2_mcu_tssi_comp, "_gpl", "");

SYMBOL_CRC(mt76x2_read_rx_gain, 0x6d441fcc, "_gpl");
SYMBOL_CRC(mt76x2_get_rate_power, 0xae010e28, "_gpl");
SYMBOL_CRC(mt76x2_get_power_info, 0x8ea229aa, "_gpl");
SYMBOL_CRC(mt76x2_get_temp_comp, 0x6d73773b, "_gpl");
SYMBOL_CRC(mt76x2_eeprom_init, 0x83f5f5d7, "_gpl");
SYMBOL_CRC(mt76x2_mac_stop, 0x140e908d, "_gpl");
SYMBOL_CRC(mt76x2_set_sar_specs, 0x3eb9d541, "_gpl");
SYMBOL_CRC(mt76x2_reset_wlan, 0xd03e4b2d, "_gpl");
SYMBOL_CRC(mt76_write_mac_initvals, 0x04857ba6, "_gpl");
SYMBOL_CRC(mt76x2_init_txpower, 0x527e6621, "_gpl");
SYMBOL_CRC(mt76x2_apply_gain_adj, 0xd4986982, "_gpl");
SYMBOL_CRC(mt76x2_phy_set_txpower_regs, 0x83ee0143, "_gpl");
SYMBOL_CRC(mt76x2_phy_set_txpower, 0x52851242, "_gpl");
SYMBOL_CRC(mt76x2_configure_tx_delay, 0x75c234c2, "_gpl");
SYMBOL_CRC(mt76x2_phy_tssi_compensate, 0xf62049d3, "_gpl");
SYMBOL_CRC(mt76x2_phy_update_channel_gain, 0x650a0bc9, "_gpl");
SYMBOL_CRC(mt76x2_mcu_set_channel, 0xe5b5313a, "_gpl");
SYMBOL_CRC(mt76x2_mcu_load_cr, 0x5f442654, "_gpl");
SYMBOL_CRC(mt76x2_mcu_init_gain, 0x44d0c763, "_gpl");
SYMBOL_CRC(mt76x2_mcu_tssi_comp, 0xfc820e6e, "_gpl");

static const char ____versions[]
__used __section("__versions") =
	"\x1c\x00\x00\x00\x93\x76\x58\xa4"
	"mt76_init_sar_power\0"
	"\x24\x00\x00\x00\x9c\x5c\xbd\x0e"
	"mt76x02_phy_adjust_vga_gain\0"
	"\x24\x00\x00\x00\xde\x12\xc6\x9e"
	"mt76x02_phy_dfs_adjust_agc\0\0"
	"\x18\x00\x00\x00\x10\x6d\x86\xba"
	"devm_kmalloc\0\0\0\0"
	"\x1c\x00\x00\x00\x20\x5d\x05\xc3"
	"usleep_range_state\0\0"
	"\x20\x00\x00\x00\x23\x24\x1c\x73"
	"cfg80211_chandef_valid\0\0"
	"\x14\x00\x00\x00\xbb\x6d\xfb\xbd"
	"__fentry__\0\0"
	"\x20\x00\x00\x00\xdb\x4b\x46\xf2"
	"mt76x02_ext_pa_enabled\0\0"
	"\x24\x00\x00\x00\x97\x70\x48\x65"
	"__x86_indirect_thunk_rax\0\0\0\0"
	"\x20\x00\x00\x00\x2e\x6b\x60\xff"
	"mt76x02_get_efuse_data\0\0"
	"\x1c\x00\x00\x00\xcb\xf6\xfd\xf0"
	"__stack_chk_fail\0\0\0\0"
	"\x28\x00\x00\x00\xb3\x1c\xa2\x87"
	"__ubsan_handle_out_of_bounds\0\0\0\0"
	"\x1c\x00\x00\x00\xa6\x21\x18\x7e"
	"mt76x02_get_rx_gain\0"
	"\x28\x00\x00\x00\x5d\xf8\xfe\x75"
	"mt76x02_add_rate_power_offset\0\0\0"
	"\x1c\x00\x00\x00\x2d\x71\xbc\x55"
	"mt76_eeprom_init\0\0\0\0"
	"\x14\x00\x00\x00\x21\x1d\xd8\xd0"
	"_dev_err\0\0\0\0"
	"\x14\x00\x00\x00\x4b\x8d\xfa\x4d"
	"mutex_lock\0\0"
	"\x24\x00\x00\x00\xaa\x8a\x4d\xfb"
	"mt76x02_eeprom_parse_hw_cap\0"
	"\x24\x00\x00\x00\xad\x08\xa0\xa7"
	"mt76_mcu_send_and_get_msg\0\0\0"
	"\x20\x00\x00\x00\x4d\xe6\x71\xde"
	"mt76_eeprom_override\0\0\0\0"
	"\x20\x00\x00\x00\x13\x21\x46\xc0"
	"mt76x02_get_lna_gain\0\0\0\0"
	"\x1c\x00\x00\x00\xca\x39\x82\x5b"
	"__x86_return_thunk\0\0"
	"\x18\x00\x00\x00\x38\xf0\x13\x32"
	"mutex_unlock\0\0\0\0"
	"\x18\x00\x00\x00\xd6\xdf\xe3\xea"
	"__const_udelay\0\0"
	"\x1c\x00\x00\x00\x3e\xb8\x15\x10"
	"mt76_get_sar_power\0\0"
	"\x1c\x00\x00\x00\x42\x4a\x4e\x9a"
	"mt76x02_eeprom_copy\0"
	"\x20\x00\x00\x00\x64\xec\xef\xea"
	"mt76x02_mcu_calibrate\0\0\0"
	"\x20\x00\x00\x00\x94\x28\x2c\x78"
	"mt76x02_phy_set_txpower\0"
	"\x24\x00\x00\x00\xa8\x3b\x5d\xad"
	"mt76x02_get_max_rate_power\0\0"
	"\x2c\x00\x00\x00\xc6\xfa\xb1\x54"
	"__ubsan_handle_load_invalid_value\0\0\0"
	"\x24\x00\x00\x00\xbc\x79\xc4\x93"
	"mt76x02_limit_rate_power\0\0\0\0"
	"\x20\x00\x00\x00\x9d\xc9\x85\x3c"
	"mt76_get_min_avg_rssi\0\0\0"
	"\x18\x00\x00\x00\x72\x3f\x86\xba"
	"module_layout\0\0\0"
	"\x00\x00\x00\x00\x00\x00\x00\x00";

MODULE_INFO(depends, "mt76,mt76x02-lib,cfg80211");


MODULE_INFO(srcversion, "F614E833AE49F1C231FFF8A");
