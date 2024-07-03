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

KSYMTAB_FUNC(mt76x02u_mcu_fw_reset, "_gpl", "");
KSYMTAB_FUNC(mt76x02u_mcu_fw_send_data, "_gpl", "");
KSYMTAB_FUNC(mt76x02u_init_mcu, "_gpl", "");
KSYMTAB_FUNC(mt76x02u_tx_complete_skb, "_gpl", "");
KSYMTAB_FUNC(mt76x02u_mac_start, "_gpl", "");
KSYMTAB_FUNC(mt76x02u_tx_prepare_skb, "_gpl", "");
KSYMTAB_FUNC(mt76x02u_init_beacon_config, "_gpl", "");
KSYMTAB_FUNC(mt76x02u_exit_beacon_config, "_gpl", "");

SYMBOL_CRC(mt76x02u_mcu_fw_reset, 0xad7532c4, "_gpl");
SYMBOL_CRC(mt76x02u_mcu_fw_send_data, 0x11ab52c5, "_gpl");
SYMBOL_CRC(mt76x02u_init_mcu, 0xc6817002, "_gpl");
SYMBOL_CRC(mt76x02u_tx_complete_skb, 0x711cef17, "_gpl");
SYMBOL_CRC(mt76x02u_mac_start, 0x84dfc923, "_gpl");
SYMBOL_CRC(mt76x02u_tx_prepare_skb, 0xa4f9dafb, "_gpl");
SYMBOL_CRC(mt76x02u_init_beacon_config, 0x0e6cd042, "_gpl");
SYMBOL_CRC(mt76x02u_exit_beacon_config, 0x51278cff, "_gpl");

static const char ____versions[]
__used __section("__versions") =
	"\x24\x00\x00\x00\xc8\xe3\x06\x9e"
	"mt76x02_update_beacon_iter\0\0"
	"\x24\x00\x00\x00\x59\x28\xfd\xc9"
	"mt76x02_enqueue_buffered_bc\0"
	"\x20\x00\x00\x00\xda\xfb\x36\xe4"
	"mt76x02_remove_hdr_pad\0\0"
	"\x10\x00\x00\x00\x0b\x22\x8d\xab"
	"skb_put\0"
	"\x14\x00\x00\x00\xe6\xd6\x62\xad"
	"consume_skb\0"
	"\x10\x00\x00\x00\xeb\x02\xe6\xb0"
	"memmove\0"
	"\x24\x00\x00\x00\xd5\x51\x08\xc0"
	"mt76x02_resync_beacon_timer\0"
	"\x18\x00\x00\x00\x36\xf2\xb6\xc5"
	"queue_work_on\0\0\0"
	"\x24\x00\x00\x00\x4b\x29\x81\x6f"
	"mt76x02_mcu_parse_response\0\0"
	"\x18\x00\x00\x00\xb9\x34\xc1\x74"
	"__sw_hweight32\0\0"
	"\x18\x00\x00\x00\x9b\xdb\xd4\x69"
	"mt76_csa_check\0\0"
	"\x20\x00\x00\x00\x45\x2b\x91\xb7"
	"mt76_tx_status_skb_add\0\0"
	"\x18\x00\x00\x00\x20\x9f\x87\x48"
	"hrtimer_init\0\0\0\0"
	"\x10\x00\x00\x00\x38\xdf\xac\x69"
	"memcpy\0\0"
	"\x10\x00\x00\x00\xba\x0c\x7a\x03"
	"kfree\0\0\0"
	"\x18\x00\x00\x00\x60\xa4\xb3\x8e"
	"mt76u_single_wr\0"
	"\x20\x00\x00\x00\x33\x0d\x8d\x96"
	"__mt76_tx_complete_skb\0\0"
	"\x24\x00\x00\x00\xc8\x50\xe3\x23"
	"mt76x02_mac_reset_counters\0\0"
	"\x1c\x00\x00\x00\x20\x5d\x05\xc3"
	"usleep_range_state\0\0"
	"\x28\x00\x00\x00\x50\x3e\x6e\xb0"
	"ieee80211_get_hdrlen_from_skb\0\0\0"
	"\x1c\x00\x00\x00\x27\xb1\x91\xcd"
	"system_highpri_wq\0\0\0"
	"\x1c\x00\x00\x00\x2e\x14\x9f\x4a"
	"__dynamic_dev_dbg\0\0\0"
	"\x18\x00\x00\x00\x8c\x89\xd4\xcb"
	"fortify_panic\0\0\0"
	"\x14\x00\x00\x00\xbb\x6d\xfb\xbd"
	"__fentry__\0\0"
	"\x24\x00\x00\x00\x97\x70\x48\x65"
	"__x86_indirect_thunk_rax\0\0\0\0"
	"\x18\x00\x00\x00\x77\xae\x56\xf6"
	"usb_bulk_msg\0\0\0\0"
	"\x1c\x00\x00\x00\xcb\xf6\xfd\xf0"
	"__stack_chk_fail\0\0\0\0"
	"\x18\x00\x00\x00\x48\x18\x47\x9b"
	"mt76_csa_finish\0"
	"\x14\x00\x00\x00\x27\xe2\x66\x49"
	"__alloc_skb\0"
	"\x28\x00\x00\x00\xb3\x1c\xa2\x87"
	"__ubsan_handle_out_of_bounds\0\0\0\0"
	"\x14\x00\x00\x00\x5b\xa9\x65\x76"
	"idr_remove\0\0"
	"\x14\x00\x00\x00\x21\x1d\xd8\xd0"
	"_dev_err\0\0\0\0"
	"\x14\x00\x00\x00\x33\xc3\x65\xdf"
	"skb_pull\0\0\0\0"
	"\x14\x00\x00\x00\x4b\x8d\xfa\x4d"
	"mutex_lock\0\0"
	"\x14\x00\x00\x00\x2a\xf0\x64\x8b"
	"skb_push\0\0\0\0"
	"\x24\x00\x00\x00\xd0\x1b\x9c\x6b"
	"mt76x02_init_beacon_config\0\0"
	"\x24\x00\x00\x00\xe9\xc8\x79\x1a"
	"__x86_indirect_thunk_r13\0\0\0\0"
	"\x20\x00\x00\x00\x9b\x6d\xe3\xe0"
	"__mt76_mcu_msg_alloc\0\0\0\0"
	"\x20\x00\x00\x00\xc1\x1d\xff\x7e"
	"mt76x02_mac_write_txwi\0\0"
	"\x20\x00\x00\x00\xa6\xaf\x55\xd9"
	"hrtimer_start_range_ns\0\0"
	"\x1c\x00\x00\x00\xca\x39\x82\x5b"
	"__x86_return_thunk\0\0"
	"\x2c\x00\x00\x00\x61\xe5\x48\xa6"
	"__ubsan_handle_shift_out_of_bounds\0\0"
	"\x34\x00\x00\x00\xd8\x6f\x47\xf3"
	"ieee80211_iterate_active_interfaces_atomic\0\0"
	"\x18\x00\x00\x00\x38\xf0\x13\x32"
	"mutex_unlock\0\0\0\0"
	"\x20\x00\x00\x00\xa9\x3d\xa4\x8b"
	"mt76u_vendor_request\0\0\0\0"
	"\x1c\x00\x00\x00\xfe\x2d\xc1\x03"
	"cancel_work_sync\0\0\0\0"
	"\x1c\x00\x00\x00\xd5\x54\x2a\xa9"
	"mt76_skb_adjust_pad\0"
	"\x18\x00\x00\x00\x56\x6d\x66\x4a"
	"hrtimer_cancel\0\0"
	"\x20\x00\x00\x00\x77\x1b\xcd\xce"
	"mt76x02_mac_set_beacon\0\0"
	"\x18\x00\x00\x00\x2f\xfa\x96\x3c"
	"hrtimer_active\0\0"
	"\x2c\x00\x00\x00\xc6\xfa\xb1\x54"
	"__ubsan_handle_load_invalid_value\0\0\0"
	"\x14\x00\x00\x00\xce\x1b\xe1\x88"
	"__mt76_poll\0"
	"\x14\x00\x00\x00\x45\x3a\x23\xeb"
	"__kmalloc\0\0\0"
	"\x18\x00\x00\x00\x72\x3f\x86\xba"
	"module_layout\0\0\0"
	"\x00\x00\x00\x00\x00\x00\x00\x00";

MODULE_INFO(depends, "mt76x02-lib,mt76,mt76-usb,cfg80211,mac80211");


MODULE_INFO(srcversion, "3A36AF8ED196CFC36DDC3FD");
