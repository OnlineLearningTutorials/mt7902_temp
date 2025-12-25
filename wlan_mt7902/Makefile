# SPDX-License-Identifier: ISC


obj-m += mt7902.o

mt7902-y := \
	chips/common/cmm_asic_connac.o \
	chips/common/dbg_connac.o \
	chips/common/dbg_connac2x.o \
	chips/common/cmm_asic_connac2x.o \
	chips/common/fw_dl.o \
 	chips/mt7902/mt7902.o \
	chips/mt7961/mt7961.o \
	chips/mt7961/dbg_mt7961.o \
	chips/mt7902/dbg_mt7902.o \
	chips/mt7902/hal_dmashdl_mt7902.o \
	chips/mt7961/hal_wfsys_reset_mt7961.o
	
mt7902-y += common/dump.o \
    common/wlan_lib.o \
    common/wlan_oid.o \
    common/wlan_bow.o \
    common/debug.o \
    common/wlan_p2p.o \
    common/wlan_he.o



mt7902-y +=  nic/nic.o \
    nic/nic_tx.o \
    nic/nic_txd_v1.o \
    nic/nic_rxd_v1.o \
    nic/nic_rx.o \
    nic/nic_pwr_mgt.o \
    nic/nic_rate.o \
    nic/cmd_buf.o \
    nic/que_mgt.o \
    nic/nic_cmd_event.o \
    nic/nic_umac.o \
    nic/radiotap.o \
    nic/nic_ext_cmd_event.o \
    nic/nic_txd_v2.o \
    nic/nic_rxd_v2.o \
    nic/p2p_nic.o



mt7902-y += os/linux/gl_init.o \
    os/linux/gl_kal.o \
    os/linux/gl_bow.o \
    os/linux/gl_wext.o \
    os/linux/gl_wext_priv.o \
    os/linux/gl_ate_agent.o \
    os/linux/gl_qa_agent.o \
    os/linux/gl_hook_api.o \
    os/linux/gl_rst.o \
    os/linux/gl_cfg80211.o \
    os/linux/gl_proc.o \
    os/linux/gl_sys.o \
    os/linux/gl_vendor.o \
    os/linux/gl_custom.o \
    os/linux/platform.o \
    os/linux/gl_p2p.o \
    os/linux/gl_p2p_cfg80211.o \
    os/linux/gl_p2p_init.o \
    os/linux/gl_p2p_kal.o \
    os/linux/hif/common/hal_pdma.o \
    os/linux/hif/common/kal_pdma.o \
    os/linux/hif/common/dbg_pdma.o \
    os/linux/hif/pcie/pcie.o



mt7902-y += mgmt/ais_fsm.o \
    mgmt/aaa_fsm.o \
    mgmt/assoc.o \
    mgmt/auth.o \
    mgmt/bss.o \
    mgmt/cnm.o \
    mgmt/cnm_timer.o \
    mgmt/cnm_mem.o \
    mgmt/hem_mbox.o \
    mgmt/mib.o \
    mgmt/privacy.o \
    mgmt/rate.o \
    mgmt/rlm.o \
    mgmt/rlm_domain.o \
    mgmt/reg_rule.o \
    mgmt/rlm_obss.o \
    mgmt/rlm_protection.o \
    mgmt/rrm.o \
    mgmt/rsn.o \
    mgmt/saa_fsm.o \
    mgmt/scan.o \
    mgmt/scan_fsm.o \
    mgmt/scan_cache.o \
    mgmt/swcr.o \
    mgmt/roaming_fsm.o \
    mgmt/tkip_mic.o \
    mgmt/hs20.o \
    mgmt/tdls.o \
    mgmt/wnm.o \
    mgmt/qosmap.o \
    mgmt/ap_selection.o \
    mgmt/wmm.o \
    mgmt/stats.o \
    mgmt/p2p_dev_fsm.o \
    mgmt/p2p_dev_state.o \
    mgmt/p2p_role_fsm.o \
    mgmt/p2p_role_state.o \
    mgmt/p2p_func.o \
    mgmt/p2p_scan.o \
    mgmt/p2p_ie.o \
    mgmt/p2p_rlm.o \
    mgmt/p2p_assoc.o \
    mgmt/p2p_bss.o \
    mgmt/p2p_rlm_obss.o \
    mgmt/p2p_fsm.o \
    mgmt/wapi.o \
    mgmt/he_ie.o \
    mgmt/he_rlm.o \
    mgmt/twt_req_fsm.o \
    mgmt/twt.o \
    mgmt/twt_planner.o
	

ccflags-y += -DDBG=0
ccflags-y += -I$(src)/os -I$(src)/os/linux/include
ccflags-y += -I$(src)/include -I$(src)/include/nic -I$(src)/include/mgmt -I$(src)/include/chips
ccflags-y += -I$(src)/os/linux/hif/pcie/include
ccflags-y += -I$(src)/os/linux/hif/common/include
ccflags-y += -DCONFIG_WLAN_SERVICE=0
ccflags-y += -DCONFIG_TEST_ENGINE_OFFLOAD=0
ccflags-y += -D_HIF_PCIE=1
ccflags-y += -DCFG_SUPPORT_AEE=0
ccflags-y += -DCFG_SUPPORT_CONNAC3X=0
ccflags-y += -DCFG_SUPPORT_SNIFFER_RADIOTAP=0
ccflags-y += -DCFG_SUPPORT_PDMA_SCATTER=0
ccflags-y += -DCFG_SUPPORT_WIFI_6G=1
ccflags-y += -DCFG_SUPPORT_WIFI_DBDC6G=1
ccflags-y += -DCFG_SUPPORT_TX_MGMT_USE_DATAQ=1
ccflags-y += -DCFG_SUPPORT_802_11AX=0



all:
	$(MAKE) -C /lib/modules/`uname -r`/build/ M=`pwd` modules

clean:
	$(MAKE) -C /lib/modules/`uname -r`/build/ M=`pwd` clean





