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

KSYMTAB_DATA(mt7663_usb_sdio_reg_map, "_gpl", "");
KSYMTAB_FUNC(mt7663_usb_sdio_tx_status_data, "_gpl", "");
KSYMTAB_FUNC(mt7663_usb_sdio_tx_complete_skb, "_gpl", "");
KSYMTAB_FUNC(mt7663_usb_sdio_tx_prepare_skb, "_gpl", "");
KSYMTAB_FUNC(mt7663_usb_sdio_register_device, "_gpl", "");

SYMBOL_CRC(mt7663_usb_sdio_reg_map, 0x1506ffca, "_gpl");
SYMBOL_CRC(mt7663_usb_sdio_tx_status_data, 0x79f3d306, "_gpl");
SYMBOL_CRC(mt7663_usb_sdio_tx_complete_skb, 0xf72a46e8, "_gpl");
SYMBOL_CRC(mt7663_usb_sdio_tx_prepare_skb, 0x606ebf61, "_gpl");
SYMBOL_CRC(mt7663_usb_sdio_register_device, 0xd6dbcc9f, "_gpl");

static const char ____versions[]
__used __section("__versions") =
	"\x1c\x00\x00\x00\x2a\x15\x5a\xfb"
	"mt7615_eeprom_init\0\0"
	"\x20\x00\x00\x00\x45\x2b\x91\xb7"
	"mt76_tx_status_skb_add\0\0"
	"\x1c\x00\x00\x00\x6a\x57\x2f\x0d"
	"mt76_connac_pm_wake\0"
	"\x10\x00\x00\x00\xba\x0c\x7a\x03"
	"kfree\0\0\0"
	"\x20\x00\x00\x00\x33\x0d\x8d\x96"
	"__mt76_tx_complete_skb\0\0"
	"\x1c\x00\x00\x00\x8f\x73\x36\x98"
	"mt7615_init_txpower\0"
	"\x14\x00\x00\x00\xbb\x6d\xfb\xbd"
	"__fentry__\0\0"
	"\x28\x00\x00\x00\x9a\xd9\xe1\x3e"
	"mt76_connac_power_save_sched\0\0\0\0"
	"\x24\x00\x00\x00\x97\x70\x48\x65"
	"__x86_indirect_thunk_rax\0\0\0\0"
	"\x1c\x00\x00\x00\xcb\xf6\xfd\xf0"
	"__stack_chk_fail\0\0\0\0"
	"\x1c\x00\x00\x00\xca\x21\x60\xe4"
	"_raw_spin_unlock_bh\0"
	"\x18\x00\x00\x00\xef\x66\x0b\xe4"
	"mt76_wcid_alloc\0"
	"\x14\x00\x00\x00\x5b\xa9\x65\x76"
	"idr_remove\0\0"
	"\x14\x00\x00\x00\x33\xc3\x65\xdf"
	"skb_pull\0\0\0\0"
	"\x20\x00\x00\x00\xea\x24\x94\x1a"
	"mt76_register_device\0\0\0\0"
	"\x14\x00\x00\x00\x4b\x8d\xfa\x4d"
	"mutex_lock\0\0"
	"\x14\x00\x00\x00\x2a\xf0\x64\x8b"
	"skb_push\0\0\0\0"
	"\x1c\x00\x00\x00\x83\xb6\xd5\x11"
	"mt7615_mac_sta_poll\0"
	"\x1c\x00\x00\x00\xca\x39\x82\x5b"
	"__x86_return_thunk\0\0"
	"\x14\x00\x00\x00\xe9\x68\xf5\x17"
	"mt76_rates\0\0"
	"\x18\x00\x00\x00\x38\xf0\x13\x32"
	"mutex_unlock\0\0\0\0"
	"\x20\x00\x00\x00\xd6\xf0\xc1\x37"
	"mt7615_mac_write_txwi\0\0\0"
	"\x1c\x00\x00\x00\xfc\x90\x36\x0c"
	"_raw_spin_lock_bh\0\0\0"
	"\x1c\x00\x00\x00\xd5\x54\x2a\xa9"
	"mt76_skb_adjust_pad\0"
	"\x20\x00\x00\x00\xca\x5f\xfd\x11"
	"ieee80211_queue_work\0\0\0\0"
	"\x1c\x00\x00\x00\xa9\xf6\x5d\x20"
	"mt7615_init_debugfs\0"
	"\x2c\x00\x00\x00\xc6\xfa\xb1\x54"
	"__ubsan_handle_load_invalid_value\0\0\0"
	"\x14\x00\x00\x00\xce\x1b\xe1\x88"
	"__mt76_poll\0"
	"\x20\x00\x00\x00\xa4\xae\xc8\xe5"
	"mt7615_mac_set_rates\0\0\0\0"
	"\x1c\x00\x00\x00\x66\x20\x85\x29"
	"mt7615_init_device\0\0"
	"\x18\x00\x00\x00\x72\x3f\x86\xba"
	"module_layout\0\0\0"
	"\x00\x00\x00\x00\x00\x00\x00\x00";

MODULE_INFO(depends, "mt7615-common,mt76,mt76-connac-lib,mac80211");


MODULE_INFO(srcversion, "685DED10D651B12425F8707");
