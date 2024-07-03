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

KSYMTAB_FUNC(__mt76u_vendor_request, "_gpl", "");
KSYMTAB_FUNC(mt76u_vendor_request, "_gpl", "");
KSYMTAB_FUNC(___mt76u_rr, "_gpl", "");
KSYMTAB_FUNC(___mt76u_wr, "_gpl", "");
KSYMTAB_FUNC(mt76u_read_copy, "_gpl", "");
KSYMTAB_FUNC(mt76u_single_wr, "_gpl", "");
KSYMTAB_FUNC(mt76u_alloc_mcu_queue, "_gpl", "");
KSYMTAB_FUNC(mt76u_stop_rx, "_gpl", "");
KSYMTAB_FUNC(mt76u_resume_rx, "_gpl", "");
KSYMTAB_FUNC(mt76u_stop_tx, "_gpl", "");
KSYMTAB_FUNC(mt76u_queues_deinit, "_gpl", "");
KSYMTAB_FUNC(mt76u_alloc_queues, "_gpl", "");
KSYMTAB_FUNC(__mt76u_init, "_gpl", "");
KSYMTAB_FUNC(mt76u_init, "_gpl", "");

SYMBOL_CRC(__mt76u_vendor_request, 0x2d189a0b, "_gpl");
SYMBOL_CRC(mt76u_vendor_request, 0x8ba43da9, "_gpl");
SYMBOL_CRC(___mt76u_rr, 0x04499df8, "_gpl");
SYMBOL_CRC(___mt76u_wr, 0x4c6432f1, "_gpl");
SYMBOL_CRC(mt76u_read_copy, 0x71b3cb2a, "_gpl");
SYMBOL_CRC(mt76u_single_wr, 0x8eb3a460, "_gpl");
SYMBOL_CRC(mt76u_alloc_mcu_queue, 0xeb041028, "_gpl");
SYMBOL_CRC(mt76u_stop_rx, 0x5814d912, "_gpl");
SYMBOL_CRC(mt76u_resume_rx, 0xa66eea72, "_gpl");
SYMBOL_CRC(mt76u_stop_tx, 0x3a3c2396, "_gpl");
SYMBOL_CRC(mt76u_queues_deinit, 0xacf589bf, "_gpl");
SYMBOL_CRC(mt76u_alloc_queues, 0xc1c8daeb, "_gpl");
SYMBOL_CRC(__mt76u_init, 0xae88ba4a, "_gpl");
SYMBOL_CRC(mt76u_init, 0x7b101713, "_gpl");

static const char ____versions[]
__used __section("__versions") =
	"\x18\x00\x00\x00\x74\x87\x6d\x94"
	"usb_free_urb\0\0\0\0"
	"\x18\x00\x00\x00\x10\x6d\x86\xba"
	"devm_kmalloc\0\0\0\0"
	"\x10\x00\x00\x00\x0b\x22\x8d\xab"
	"skb_put\0"
	"\x18\x00\x00\x00\x14\x27\x52\x8d"
	"__rcu_read_lock\0"
	"\x20\x00\x00\x00\xc1\x16\xf2\x30"
	"page_pool_alloc_frag\0\0\0\0"
	"\x18\x00\x00\x00\x36\xf2\xb6\xc5"
	"queue_work_on\0\0\0"
	"\x20\x00\x00\x00\x58\x89\xb8\xc4"
	"trace_raw_output_prep\0\0\0"
	"\x28\x00\x00\x00\x09\xd7\x89\xab"
	"__trace_trigger_soft_disabled\0\0\0"
	"\x14\x00\x00\x00\xbf\x0f\x54\x92"
	"finish_wait\0"
	"\x1c\x00\x00\x00\xc8\xa5\xc3\xa0"
	"trace_event_printf\0\0"
	"\x18\x00\x00\x00\x07\x97\x56\x53"
	"this_cpu_off\0\0\0\0"
	"\x18\x00\x00\x00\x30\xaf\xd3\x09"
	"skb_add_rx_frag\0"
	"\x20\x00\x00\x00\x00\xf1\xb9\x75"
	"trace_event_raw_init\0\0\0\0"
	"\x10\x00\x00\x00\x38\xdf\xac\x69"
	"memcpy\0\0"
	"\x1c\x00\x00\x00\xba\x2b\xca\x7e"
	"mt76_has_tx_pending\0"
	"\x14\x00\x00\x00\xab\x8e\xd3\x43"
	"pcpu_hot\0\0\0\0"
	"\x1c\x00\x00\x00\x33\xef\xa1\xb3"
	"page_pool_destroy\0\0\0"
	"\x1c\x00\x00\x00\x20\x5d\x05\xc3"
	"usleep_range_state\0\0"
	"\x18\x00\x00\x00\x84\x33\x0c\x55"
	"bpf_trace_run2\0\0"
	"\x20\x00\x00\x00\x95\xd4\x26\x8c"
	"prepare_to_wait_event\0\0\0"
	"\x14\x00\x00\x00\x44\x43\x96\xe2"
	"__wake_up\0\0\0"
	"\x20\x00\x00\x00\x0b\x05\xdb\x34"
	"_raw_spin_lock_irqsave\0\0"
	"\x18\x00\x00\x00\x03\x9b\x1b\x9e"
	"kthread_park\0\0\0\0"
	"\x1c\x00\x00\x00\x56\x66\x7d\xf5"
	"sched_set_fifo_low\0\0"
	"\x18\x00\x00\x00\x8c\x89\xd4\xcb"
	"fortify_panic\0\0\0"
	"\x14\x00\x00\x00\xbb\x6d\xfb\xbd"
	"__fentry__\0\0"
	"\x20\x00\x00\x00\x57\xd9\xc4\x06"
	"mt76_tx_status_check\0\0\0\0"
	"\x18\x00\x00\x00\x06\x45\xbd\xd9"
	"wake_up_process\0"
	"\x24\x00\x00\x00\x3d\xed\x44\xeb"
	"trace_event_buffer_commit\0\0\0"
	"\x24\x00\x00\x00\x97\x70\x48\x65"
	"__x86_indirect_thunk_rax\0\0\0\0"
	"\x18\x00\x00\x00\x81\xc8\x24\x1d"
	"___ratelimit\0\0\0\0"
	"\x1c\x00\x00\x00\xad\x8a\xdd\x8d"
	"schedule_timeout\0\0\0\0"
	"\x1c\x00\x00\x00\xcb\xf6\xfd\xf0"
	"__stack_chk_fail\0\0\0\0"
	"\x10\x00\x00\x00\x94\xb6\x16\xa9"
	"strnlen\0"
	"\x14\x00\x00\x00\x27\xe2\x66\x49"
	"__alloc_skb\0"
	"\x18\x00\x00\x00\xe5\x6a\xdf\x90"
	"usb_submit_urb\0\0"
	"\x28\x00\x00\x00\xb3\x1c\xa2\x87"
	"__ubsan_handle_out_of_bounds\0\0\0\0"
	"\x1c\x00\x00\x00\x5e\xd7\xd8\x7c"
	"page_offset_base\0\0\0\0"
	"\x18\x00\x00\x00\xf7\xb3\xf3\x66"
	"kthread_unpark\0\0"
	"\x28\x00\x00\x00\xe4\x6f\xb3\xbc"
	"hugetlb_optimize_vmemmap_key\0\0\0\0"
	"\x14\x00\x00\x00\x53\xb8\x22\x36"
	"build_skb\0\0\0"
	"\x18\x00\x00\x00\x75\x79\x48\xfe"
	"init_wait_entry\0"
	"\x20\x00\x00\x00\x17\x00\x5f\xad"
	"perf_trace_buf_alloc\0\0\0\0"
	"\x24\x00\x00\x00\x33\x96\x73\x1f"
	"perf_trace_run_bpf_submit\0\0\0"
	"\x14\x00\x00\x00\x21\x1d\xd8\xd0"
	"_dev_err\0\0\0\0"
	"\x1c\x00\x00\x00\x0f\x81\x69\x24"
	"__rcu_read_unlock\0\0\0"
	"\x14\x00\x00\x00\x4b\x8d\xfa\x4d"
	"mutex_lock\0\0"
	"\x18\x00\x00\x00\x32\x66\x57\xdb"
	"usb_control_msg\0"
	"\x18\x00\x00\x00\x05\x81\x87\xb7"
	"usb_poison_urb\0\0"
	"\x18\x00\x00\x00\x8b\x0d\x87\x07"
	"trace_event_reg\0"
	"\x1c\x00\x00\x00\x2b\x71\x58\xe6"
	"usb_unpoison_urb\0\0\0\0"
	"\x18\x00\x00\x00\x15\x43\x63\xc6"
	"mt76_ac_to_hwq\0\0"
	"\x14\x00\x00\x00\xb0\x28\x9d\x4c"
	"phys_base\0\0\0"
	"\x1c\x00\x00\x00\x71\x22\x5a\x5a"
	"__cpu_online_mask\0\0\0"
	"\x20\x00\x00\x00\x39\xce\x3f\x3c"
	"__local_bh_enable_ip\0\0\0\0"
	"\x18\x00\x00\x00\xd6\xa3\x98\x7a"
	"kthread_stop\0\0\0\0"
	"\x18\x00\x00\x00\x9f\x0c\xfb\xce"
	"__mutex_init\0\0\0\0"
	"\x24\x00\x00\x00\x70\xce\x5c\xd3"
	"_raw_spin_unlock_irqrestore\0"
	"\x1c\x00\x00\x00\xca\x39\x82\x5b"
	"__x86_return_thunk\0\0"
	"\x20\x00\x00\x00\xe1\x8a\xb6\xde"
	"mt76_queue_tx_complete\0\0"
	"\x20\x00\x00\x00\x9d\x83\x1b\x93"
	"kthread_create_on_node\0\0"
	"\x18\x00\x00\x00\x9f\xdd\xf9\xdd"
	"bpf_trace_run3\0\0"
	"\x18\x00\x00\x00\x6c\x1e\x65\x97"
	"vmemmap_base\0\0\0\0"
	"\x20\x00\x00\x00\x72\x0e\xc7\xf1"
	"mt76_rx_poll_complete\0\0\0"
	"\x10\x00\x00\x00\x39\xe6\x64\xdd"
	"strscpy\0"
	"\x24\x00\x00\x00\x07\x6e\x40\xdb"
	"trace_event_buffer_reserve\0\0"
	"\x18\x00\x00\x00\x38\xf0\x13\x32"
	"mutex_unlock\0\0\0\0"
	"\x18\x00\x00\x00\xcc\xb5\x56\x96"
	"param_ops_bool\0\0"
	"\x24\x00\x00\x00\xf9\xa4\xcc\x66"
	"__x86_indirect_thunk_rcx\0\0\0\0"
	"\x18\x00\x00\x00\xc0\x8c\x7c\x2a"
	"usb_kill_urb\0\0\0\0"
	"\x1c\x00\x00\x00\xfe\x2d\xc1\x03"
	"cancel_work_sync\0\0\0\0"
	"\x18\x00\x00\x00\x18\x01\x47\x56"
	"__warn_printk\0\0\0"
	"\x1c\x00\x00\x00\x4f\x7b\xc5\x1e"
	"__mt76_worker_fn\0\0\0\0"
	"\x18\x00\x00\x00\xd9\x14\xbf\xd7"
	"usb_init_urb\0\0\0\0"
	"\x2c\x00\x00\x00\xc6\xfa\xb1\x54"
	"__ubsan_handle_load_invalid_value\0\0\0"
	"\x28\x00\x00\x00\xe1\x72\x8e\x68"
	"__SCT__preempt_schedule_notrace\0"
	"\x20\x00\x00\x00\x30\x7a\xe4\xc8"
	"mt76_create_page_pool\0\0\0"
	"\x1c\x00\x00\x00\xe3\x20\xd1\x85"
	"trace_handle_return\0"
	"\x18\x00\x00\x00\x63\xe9\xa4\xa4"
	"skb_to_sgvec\0\0\0\0"
	"\x18\x00\x00\x00\x10\xc9\xf0\x2c"
	"sg_init_table\0\0\0"
	"\x14\x00\x00\x00\x45\x3a\x23\xeb"
	"__kmalloc\0\0\0"
	"\x20\x00\x00\x00\x5d\x7b\xc1\xe2"
	"__SCT__might_resched\0\0\0\0"
	"\x24\x00\x00\x00\x5c\x35\xad\x3c"
	"page_pool_put_unrefed_page\0\0"
	"\x18\x00\x00\x00\x72\x3f\x86\xba"
	"module_layout\0\0\0"
	"\x00\x00\x00\x00\x00\x00\x00\x00";

MODULE_INFO(depends, "mt76");


MODULE_INFO(srcversion, "24CEE758FE0C39B8A55FF20");
