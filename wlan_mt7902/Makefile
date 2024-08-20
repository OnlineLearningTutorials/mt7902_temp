# SPDX-License-Identifier: GPL-2.0-only


$(info inc is [${shell pwd}])
obj-m += wlan_mt7902e.o


export CONFIG_MTK_COMBO_PLATFORM=x86_64
export MTK_COMBO_CHIP=MT7902

PWD=$(shell pwd)
DRIVER_DIR=$(PWD)

export CONFIG_MTK_WIFI_ONLY=m
export CONFIG_MTK_COMBO=m
export CONFIG_MTK_COMBO_WIFI=m
export CONFIG_MTK_COMBO_COMM=m
export CONFIG_MTK_COMBO_COMM_UART=m
export CONFIG_MTK_COMBO_COMM_SDIO=m
export CONFIG_MT_WIFI_CHRDEV=m
  hif=pcie

cflags = -Iinclude/ 





COMMON_DIR  := common/
OS_DIR      := os/linux/
HIF_COMMON_DIR := os/linux/hif/common/
HIF_DIR     := os/linux/hif/pcie/
NIC_DIR     := nic/
MGMT_DIR    := mgmt/
CHIPS       := chips/
CHIPS_CMM   := $(CHIPS)common/
# PLAT_DIR    := os/linux/plat/$(MTK_PLATFORM)/
SYSDVT_DIR  := dvt/

# ---------------------------------------------------
# Chips Objects List
# ---------------------------------------------------


wlan_mt7902e-y := $(CHIPS_CMM)cmm_asic_connac.o	\
	$(CHIPS_CMM)dbg_connac.o  \
	$(CHIPS_CMM)fw_dl.o  \
	$(CHIPS)connac/connac.o \
	$(CHIPS)soc3_0/soc3_0.o  \
	$(CHIPS)soc3_0/dbg_soc3_0.o \
	$(CHIPS)mt7902/mt7902.o \
	$(CHIPS)mt7961/mt7961.o \
	$(CHIPS)mt7961/dbg_mt7961.o \
	$(CHIPS)mt7902/dbg_mt7902.o \
	$(CHIPS)mt7902/hal_dmashdl_mt7902.o \
	$(CHIPS)mt7961/hal_wfsys_reset_mt7961.o 

wlan_mt7902e-y += $(MGMT_DIR)stats.o



wlan_mt7902e-y += 	$(COMMON_DIR)dump.o \
		$(COMMON_DIR)wlan_lib.o \
		$(COMMON_DIR)wlan_oid.o \
		$(COMMON_DIR)wlan_bow.o \
		$(COMMON_DIR)debug.o

wlan_mt7902e-y += 	$(NIC_DIR)nic.o \
		$(NIC_DIR)nic_tx.o \
		$(NIC_DIR)nic_txd_v1.o \
		$(NIC_DIR)nic_rxd_v1.o \
		$(NIC_DIR)nic_rx.o \
		$(NIC_DIR)nic_pwr_mgt.o \
		$(NIC_DIR)nic_rate.o \
		$(NIC_DIR)cmd_buf.o \
		$(NIC_DIR)que_mgt.o \
		$(NIC_DIR)nic_cmd_event.o \
		$(NIC_DIR)nic_umac.o

wlan_mt7902e-y += 	$(OS_DIR)gl_init.o \
		$(OS_DIR)gl_kal.o \
		$(OS_DIR)gl_bow.o \
		$(OS_DIR)gl_wext.o \
		$(OS_DIR)gl_wext_priv.o \
		$(OS_DIR)gl_ate_agent.o \
		$(OS_DIR)gl_qa_agent.o \
		$(OS_DIR)gl_hook_api.o \
		$(OS_DIR)gl_rst.o \
		$(OS_DIR)gl_cfg80211.o \
		$(OS_DIR)gl_proc.o \
		$(OS_DIR)gl_vendor.o \
		$(OS_DIR)platform.o


wlan_mt7902e-y += 	$(MGMT_DIR)ais_fsm.o \
		$(MGMT_DIR)aaa_fsm.o \
		$(MGMT_DIR)assoc.o \
		$(MGMT_DIR)auth.o \
		$(MGMT_DIR)bss.o \
		$(MGMT_DIR)cnm.o \
		$(MGMT_DIR)cnm_timer.o \
		$(MGMT_DIR)cnm_mem.o \
		$(MGMT_DIR)hem_mbox.o \
		$(MGMT_DIR)mib.o \
		$(MGMT_DIR)privacy.o \
		$(MGMT_DIR)rate.o \
		$(MGMT_DIR)rlm.o \
		$(MGMT_DIR)rlm_domain.o \
		$(MGMT_DIR)reg_rule.o \
		$(MGMT_DIR)rlm_obss.o \
		$(MGMT_DIR)rlm_protection.o \
		$(MGMT_DIR)rsn.o \
		$(MGMT_DIR)saa_fsm.o \
		$(MGMT_DIR)scan.o \
		$(MGMT_DIR)scan_fsm.o \
		$(MGMT_DIR)scan_cache.o \
		$(MGMT_DIR)swcr.o \
		$(MGMT_DIR)roaming_fsm.o \
		$(MGMT_DIR)tkip_mic.o \
		$(MGMT_DIR)hs20.o \
		$(MGMT_DIR)tdls.o \
		$(MGMT_DIR)wnm.o \
		$(MGMT_DIR)qosmap.o \
		$(MGMT_DIR)ap_selection.o \
		$(MGMT_DIR)wmm.o
wlan_mt7902e-y += $(MGMT_DIR)p2p_dev_fsm.o\
            $(MGMT_DIR)p2p_dev_state.o\
            $(MGMT_DIR)p2p_role_fsm.o\
            $(MGMT_DIR)p2p_role_state.o\
            $(MGMT_DIR)p2p_func.o\
            $(MGMT_DIR)p2p_scan.o\
            $(MGMT_DIR)p2p_ie.o\
            $(MGMT_DIR)p2p_rlm.o\
            $(MGMT_DIR)p2p_assoc.o\
            $(MGMT_DIR)p2p_bss.o\
            $(MGMT_DIR)p2p_rlm_obss.o\
            $(MGMT_DIR)p2p_fsm.o

wlan_mt7902e-y +=  $(MGMT_DIR)wapi.o




















all: 
	$(MAKE) $(cflags) -C /lib/modules/`uname -r`/build/ M=`pwd` modules


clean: 
	$(MAKE) -C /lib/modules/`uname -r`/build/ M=`pwd` clean
	
