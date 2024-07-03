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

KSYMTAB_FUNC(mt792xu_rr, "_gpl", "");
KSYMTAB_FUNC(mt792xu_wr, "_gpl", "");
KSYMTAB_FUNC(mt792xu_rmw, "_gpl", "");
KSYMTAB_FUNC(mt792xu_copy, "_gpl", "");
KSYMTAB_FUNC(mt792xu_mcu_power_on, "_gpl", "");
KSYMTAB_FUNC(mt792xu_dma_init, "_gpl", "");
KSYMTAB_FUNC(mt792xu_wfsys_reset, "_gpl", "");
KSYMTAB_FUNC(mt792xu_init_reset, "_gpl", "");
KSYMTAB_FUNC(mt792xu_stop, "_gpl", "");
KSYMTAB_FUNC(mt792xu_disconnect, "_gpl", "");

SYMBOL_CRC(mt792xu_rr, 0x0bbb2b51, "_gpl");
SYMBOL_CRC(mt792xu_wr, 0xa0d65ac2, "_gpl");
SYMBOL_CRC(mt792xu_rmw, 0xd83d187b, "_gpl");
SYMBOL_CRC(mt792xu_copy, 0x7cbc812e, "_gpl");
SYMBOL_CRC(mt792xu_mcu_power_on, 0x5b9aff3f, "_gpl");
SYMBOL_CRC(mt792xu_dma_init, 0x81798723, "_gpl");
SYMBOL_CRC(mt792xu_wfsys_reset, 0xc1aaa895, "_gpl");
SYMBOL_CRC(mt792xu_init_reset, 0x51fc144a, "_gpl");
SYMBOL_CRC(mt792xu_stop, 0x97525b18, "_gpl");
SYMBOL_CRC(mt792xu_disconnect, 0xb349e1ad, "_gpl");

static const char ____versions[]
__used __section("__versions") =
	"\x14\x00\x00\x00\xbf\x71\x51\xd8"
	"mt792x_stop\0"
	"\x20\x00\x00\x00\x0b\x9a\x18\x2d"
	"__mt76u_vendor_request\0\0"
	"\x14\x00\x00\x00\xf1\x32\x64\x4c"
	"___mt76u_wr\0"
	"\x10\x00\x00\x00\x38\xdf\xac\x69"
	"memcpy\0\0"
	"\x1c\x00\x00\x00\x20\x5d\x05\xc3"
	"usleep_range_state\0\0"
	"\x14\x00\x00\x00\x44\x43\x96\xe2"
	"__wake_up\0\0\0"
	"\x18\x00\x00\x00\x72\xea\x6e\xa6"
	"mt76u_resume_rx\0"
	"\x14\x00\x00\x00\xbb\x6d\xfb\xbd"
	"__fentry__\0\0"
	"\x20\x00\x00\x00\x0a\x84\x0b\x92"
	"skb_queue_purge_reason\0\0"
	"\x24\x00\x00\x00\x97\x70\x48\x65"
	"__x86_indirect_thunk_rax\0\0\0\0"
	"\x14\x00\x00\x00\xcf\xbf\xfe\x7b"
	"usb_put_dev\0"
	"\x1c\x00\x00\x00\x05\x9e\x4f\x7a"
	"mt76_free_device\0\0\0\0"
	"\x1c\x00\x00\x00\xbf\x89\xf5\xac"
	"mt76u_queues_deinit\0"
	"\x14\x00\x00\x00\x21\x1d\xd8\xd0"
	"_dev_err\0\0\0\0"
	"\x14\x00\x00\x00\x4b\x8d\xfa\x4d"
	"mutex_lock\0\0"
	"\x20\x00\x00\x00\xd0\xad\x8c\x88"
	"mt76_unregister_device\0\0"
	"\x1c\x00\x00\x00\xca\x39\x82\x5b"
	"__x86_return_thunk\0\0"
	"\x18\x00\x00\x00\x38\xf0\x13\x32"
	"mutex_unlock\0\0\0\0"
	"\x18\x00\x00\x00\x96\x23\x3c\x3a"
	"mt76u_stop_tx\0\0\0"
	"\x20\x00\x00\x00\xa9\x3d\xa4\x8b"
	"mt76u_vendor_request\0\0\0\0"
	"\x1c\x00\x00\x00\xea\x59\xfe\xfa"
	"____mt76_poll_msec\0\0"
	"\x1c\x00\x00\x00\xfe\x2d\xc1\x03"
	"cancel_work_sync\0\0\0\0"
	"\x18\x00\x00\x00\x12\xd9\x14\x58"
	"mt76u_stop_rx\0\0\0"
	"\x14\x00\x00\x00\xf8\x9d\x49\x04"
	"___mt76u_rr\0"
	"\x14\x00\x00\x00\xce\x1b\xe1\x88"
	"__mt76_poll\0"
	"\x10\x00\x00\x00\xf9\x82\xa4\xf9"
	"msleep\0\0"
	"\x18\x00\x00\x00\x72\x3f\x86\xba"
	"module_layout\0\0\0"
	"\x00\x00\x00\x00\x00\x00\x00\x00";

MODULE_INFO(depends, "mt792x-lib,mt76-usb,mt76");


MODULE_INFO(srcversion, "111A008F6FCD169D8E95D51");
