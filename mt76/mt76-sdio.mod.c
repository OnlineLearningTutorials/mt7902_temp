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

KSYMTAB_FUNC(mt76s_read_pcr, "_gpl", "");
KSYMTAB_FUNC(mt76s_rr, "_gpl", "");
KSYMTAB_FUNC(mt76s_wr, "_gpl", "");
KSYMTAB_FUNC(mt76s_rmw, "_gpl", "");
KSYMTAB_FUNC(mt76s_write_copy, "_gpl", "");
KSYMTAB_FUNC(mt76s_read_copy, "_gpl", "");
KSYMTAB_FUNC(mt76s_wr_rp, "_gpl", "");
KSYMTAB_FUNC(mt76s_rd_rp, "_gpl", "");
KSYMTAB_FUNC(mt76s_hw_init, "_gpl", "");
KSYMTAB_FUNC(mt76s_alloc_rx_queue, "_gpl", "");
KSYMTAB_FUNC(mt76s_alloc_tx, "_gpl", "");
KSYMTAB_FUNC(mt76s_deinit, "_gpl", "");
KSYMTAB_FUNC(mt76s_init, "_gpl", "");
KSYMTAB_FUNC(mt76s_txrx_worker, "_gpl", "");
KSYMTAB_FUNC(mt76s_sdio_irq, "_gpl", "");
KSYMTAB_FUNC(mt76s_txqs_empty, "_gpl", "");

SYMBOL_CRC(mt76s_read_pcr, 0x3c2a2385, "_gpl");
SYMBOL_CRC(mt76s_rr, 0xdd741596, "_gpl");
SYMBOL_CRC(mt76s_wr, 0xa1542fc0, "_gpl");
SYMBOL_CRC(mt76s_rmw, 0xc650d361, "_gpl");
SYMBOL_CRC(mt76s_write_copy, 0x2a38b209, "_gpl");
SYMBOL_CRC(mt76s_read_copy, 0x83674330, "_gpl");
SYMBOL_CRC(mt76s_wr_rp, 0x120deb76, "_gpl");
SYMBOL_CRC(mt76s_rd_rp, 0xe6ae8aa4, "_gpl");
SYMBOL_CRC(mt76s_hw_init, 0x75ce98bd, "_gpl");
SYMBOL_CRC(mt76s_alloc_rx_queue, 0xf910281a, "_gpl");
SYMBOL_CRC(mt76s_alloc_tx, 0xabe2bdb2, "_gpl");
SYMBOL_CRC(mt76s_deinit, 0xbc9ad4d8, "_gpl");
SYMBOL_CRC(mt76s_init, 0x2e18e985, "_gpl");
SYMBOL_CRC(mt76s_txrx_worker, 0x223caaef, "_gpl");
SYMBOL_CRC(mt76s_sdio_irq, 0xf119b673, "_gpl");
SYMBOL_CRC(mt76s_txqs_empty, 0x72817b17, "_gpl");

static const char ____versions[]
__used __section("__versions") =
	"\x1c\x00\x00\x00\xd7\x22\x7f\x58"
	"devmap_managed_key\0\0"
	"\x1c\x00\x00\x00\x58\xc2\xe0\xb3"
	"sdio_release_host\0\0\0"
	"\x1c\x00\x00\x00\xec\x12\xa5\xbf"
	"sdio_disable_func\0\0\0"
	"\x1c\x00\x00\x00\x5c\x44\x73\x2b"
	"sdio_set_block_size\0"
	"\x18\x00\x00\x00\x10\x6d\x86\xba"
	"devm_kmalloc\0\0\0\0"
	"\x10\x00\x00\x00\x0b\x22\x8d\xab"
	"skb_put\0"
	"\x18\x00\x00\x00\x14\x27\x52\x8d"
	"__rcu_read_lock\0"
	"\x18\x00\x00\x00\xc7\x9a\xa3\x2f"
	"sdio_claim_host\0"
	"\x14\x00\x00\x00\xe6\xd6\x62\xad"
	"consume_skb\0"
	"\x18\x00\x00\x00\x30\xaf\xd3\x09"
	"skb_add_rx_frag\0"
	"\x10\x00\x00\x00\x38\xdf\xac\x69"
	"memcpy\0\0"
	"\x14\x00\x00\x00\xab\x8e\xd3\x43"
	"pcpu_hot\0\0\0\0"
	"\x28\x00\x00\x00\xe7\x06\x26\x9d"
	"__put_devmap_managed_page_refs\0\0"
	"\x1c\x00\x00\x00\x20\x5d\x05\xc3"
	"usleep_range_state\0\0"
	"\x14\x00\x00\x00\x44\x43\x96\xe2"
	"__wake_up\0\0\0"
	"\x1c\x00\x00\x00\x56\x66\x7d\xf5"
	"sched_set_fifo_low\0\0"
	"\x14\x00\x00\x00\xbb\x6d\xfb\xbd"
	"__fentry__\0\0"
	"\x1c\x00\x00\x00\x39\x16\xa8\xec"
	"pskb_expand_head\0\0\0\0"
	"\x20\x00\x00\x00\x57\xd9\xc4\x06"
	"mt76_tx_status_check\0\0\0\0"
	"\x18\x00\x00\x00\x06\x45\xbd\xd9"
	"wake_up_process\0"
	"\x24\x00\x00\x00\x97\x70\x48\x65"
	"__x86_indirect_thunk_rax\0\0\0\0"
	"\x1c\x00\x00\x00\xcb\xf6\xfd\xf0"
	"__stack_chk_fail\0\0\0\0"
	"\x1c\x00\x00\x00\xf8\x14\xc1\x8e"
	"sdio_release_irq\0\0\0\0"
	"\x1c\x00\x00\x00\xca\x21\x60\xe4"
	"_raw_spin_unlock_bh\0"
	"\x14\x00\x00\x00\x27\xe2\x66\x49"
	"__alloc_skb\0"
	"\x14\x00\x00\x00\xfc\x11\x89\x61"
	"numa_node\0\0\0"
	"\x28\x00\x00\x00\xb3\x1c\xa2\x87"
	"__ubsan_handle_out_of_bounds\0\0\0\0"
	"\x1c\x00\x00\x00\x5e\xd7\xd8\x7c"
	"page_offset_base\0\0\0\0"
	"\x28\x00\x00\x00\xe4\x6f\xb3\xbc"
	"hugetlb_optimize_vmemmap_key\0\0\0\0"
	"\x14\x00\x00\x00\x3d\x6d\x8e\x13"
	"sdio_readsb\0"
	"\x14\x00\x00\x00\x21\x1d\xd8\xd0"
	"_dev_err\0\0\0\0"
	"\x1c\x00\x00\x00\x0f\x81\x69\x24"
	"__rcu_read_unlock\0\0\0"
	"\x20\x00\x00\x00\xad\xc4\xf9\x36"
	"__SCK__tp_func_dev_irq\0\0"
	"\x14\x00\x00\x00\x74\xd0\xcd\xff"
	"sdio_readl\0\0"
	"\x14\x00\x00\x00\xb0\x28\x9d\x4c"
	"phys_base\0\0\0"
	"\x1c\x00\x00\x00\x71\x22\x5a\x5a"
	"__cpu_online_mask\0\0\0"
	"\x20\x00\x00\x00\x39\xce\x3f\x3c"
	"__local_bh_enable_ip\0\0\0\0"
	"\x18\x00\x00\x00\xd6\xa3\x98\x7a"
	"kthread_stop\0\0\0\0"
	"\x18\x00\x00\x00\xed\xed\x8f\x9c"
	"__alloc_pages\0\0\0"
	"\x14\x00\x00\x00\xb3\x84\xf5\x42"
	"sdio_writel\0"
	"\x1c\x00\x00\x00\xca\x39\x82\x5b"
	"__x86_return_thunk\0\0"
	"\x20\x00\x00\x00\xe1\x8a\xb6\xde"
	"mt76_queue_tx_complete\0\0"
	"\x20\x00\x00\x00\x9d\x83\x1b\x93"
	"kthread_create_on_node\0\0"
	"\x20\x00\x00\x00\xf8\xa1\x46\xeb"
	"__tracepoint_dev_irq\0\0\0\0"
	"\x18\x00\x00\x00\x6c\x1e\x65\x97"
	"vmemmap_base\0\0\0\0"
	"\x20\x00\x00\x00\x72\x0e\xc7\xf1"
	"mt76_rx_poll_complete\0\0\0"
	"\x20\x00\x00\x00\xcb\x28\x56\xe4"
	"__SCT__tp_func_dev_irq\0\0"
	"\x24\x00\x00\x00\xf9\xa4\xcc\x66"
	"__x86_indirect_thunk_rcx\0\0\0\0"
	"\x14\x00\x00\x00\x7d\x97\x53\xb2"
	"__folio_put\0"
	"\x14\x00\x00\x00\x65\x93\x3f\xb4"
	"ktime_get\0\0\0"
	"\x1c\x00\x00\x00\x4f\x7b\xc5\x1e"
	"__mt76_worker_fn\0\0\0\0"
	"\x1c\x00\x00\x00\xfc\x90\x36\x0c"
	"_raw_spin_lock_bh\0\0\0"
	"\x18\x00\x00\x00\x83\x3a\x0b\x44"
	"sdio_claim_irq\0\0"
	"\x1c\x00\x00\x00\xd5\x54\x2a\xa9"
	"mt76_skb_adjust_pad\0"
	"\x1c\x00\x00\x00\x70\x13\xb6\x4d"
	"sdio_enable_func\0\0\0\0"
	"\x18\x00\x00\x00\xe4\xd0\x1b\x90"
	"sdio_writesb\0\0\0\0"
	"\x28\x00\x00\x00\xe1\x72\x8e\x68"
	"__SCT__preempt_schedule_notrace\0"
	"\x20\x00\x00\x00\x5d\x7b\xc1\xe2"
	"__SCT__might_resched\0\0\0\0"
	"\x18\x00\x00\x00\x72\x3f\x86\xba"
	"module_layout\0\0\0"
	"\x00\x00\x00\x00\x00\x00\x00\x00";

MODULE_INFO(depends, "mt76");


MODULE_INFO(srcversion, "7AD9943EEFFC185D200FE59");
