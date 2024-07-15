# ---------------------------------------------------
# Kbuild option
# ---------------------------------------------------
ifeq ($(CONFIG_MTK_COMBO_WIFI_HIF),)
ifneq ($(src),)
ifneq ($(srctree),)
hif=pcie
include $(srctree)/$(src)/Makefile.x86
src:=$(srctree)/$(shell echo ${src})
MTK_COMBO_CHIP=MT7961
$(info Kbuild chip: $(MTK_COMBO_CHIP))
$(info Kbuild hif : $(CONFIG_MTK_COMBO_WIFI_HIF))
endif
endif
endif

# ---------------------------------------------------
# OS option
# ---------------------------------------------------
os=$(CONFIG_MTK_SUPPORT_OS)

ifeq ($(os),)
os=linux
endif

ifeq ($(os), none)
ccflags-y += -I/usr/include/
ccflags-y += -DCFG_VIRTUAL_OS
ccflags-y += -DCFG_REMIND_IMPLEMENT
endif

$(info os option: $(os))

# script for check the kernel version condition
# example check kernel >= 5.4 : $(call kver_ge,5,4)
kver_ge = $(shell \
echo check_ver | awk '{if($(VERSION) < $(1)) {print 0} else { \
if($(VERSION) > $(1)) {print 1} else { \
if($(PATCHLEVEL) < $(2)) {print 0} else {print 1} \
}}}' \
)

# ---------------------------------------------------
# ALPS Setting
# ---------------------------------------------------
ifneq ($(KERNEL_OUT),)
    ccflags-y += -imacros $(KERNEL_OUT)/include/generated/autoconf.h
endif

ifeq ($(KBUILD_MODPOST_FAIL_ON_WARNINGS),)
    # Force build fail on modpost warning
    KBUILD_MODPOST_FAIL_ON_WARNINGS=y
endif

DRIVER_BUILD_DATE=$(shell date +%Y%m%d%H%M%S)
ccflags-y += -DDRIVER_BUILD_DATE='"$(DRIVER_BUILD_DATE)"'
# ---------------------------------------------------
# Compile Options
# ---------------------------------------------------
WLAN_CHIP_LIST:=-UMT6620 -UMT6628 -UMT5931 -UMT6630 -UMT6632 -UMT7663 -UCONNAC -UCONNAC2X2 -UUT_TEST_MODE -UMT7915 -USOC3_0 -UMT7961 -UMT7922 -USOC5_0 -UMT7902 -UMT6639
# '-D' and '-U' options are processed in the order they are given on the command line.
# All '-imacros file' and '-include file' options are processed after all '-D' and '-U' options.
ccflags-y += $(WLAN_CHIP_LIST)

ifeq ($(MTK_COMBO_CHIP),)
MTK_COMBO_CHIP = MT6632
endif

MTK_PLATFORM := $(subst ",,$(CONFIG_MTK_PLATFORM))
$(info $$MTK_PLATFORM is [${MTK_PLATFORM}])

ifeq ($(WLAN_CHIP_ID),)
WLAN_CHIP_ID=$(word 1, $(MTK_COMBO_CHIP))
endif
$(info $$WLAN_CHIP_ID is [${WLAN_CHIP_ID}])

ccflags-y += -DCFG_SUPPORT_DEBUG_FS=0
ccflags-y += -DWLAN_INCLUDE_PROC
ccflags-y += -DCFG_SUPPORT_AGPS_ASSIST=0
ccflags-y += -DCFG_SUPPORT_TSF_USING_BOOTTIME=1
ccflags-y += -DARP_MONITER_ENABLE=1
ccflags-y += -Werror
#ccflags-y:=$(filter-out -U$(WLAN_CHIP_ID),$(ccflags-y))
#ccflags-y += -DLINUX -D$(WLAN_CHIP_ID)
#workaround: also needed for none LINUX system
# because some of common part code is surrounded with this flag
ccflags-y += -DLINUX

ifneq ($(filter MT6632,$(MTK_COMBO_CHIP)),)
ccflags-y:=$(filter-out -UMT6632,$(ccflags-y))
ccflags-y += -DMT6632
endif

ifneq ($(filter MT7668,$(MTK_COMBO_CHIP)),)
ccflags-y:=$(filter-out -UMT7668,$(ccflags-y))
ccflags-y += -DMT7668
endif

ifneq ($(filter MT7663,$(MTK_COMBO_CHIP)),)
ccflags-y:=$(filter-out -UMT7663,$(ccflags-y))
ccflags-y += -DMT7663
endif

ifneq ($(filter CONNAC,$(MTK_COMBO_CHIP)),)
ccflags-y:=$(filter-out -UCONNAC,$(ccflags-y))
ccflags-y += -DCONNAC
endif

ifneq ($(filter CONNAC2X2,$(MTK_COMBO_CHIP)),)
ccflags-y:=$(filter-out -UCONNAC2X2,$(ccflags-y))
ccflags-y += -DCONNAC2X2
endif

ifneq ($(findstring MT7915,$(MTK_COMBO_CHIP)),)
ccflags-y:=$(filter-out -UMT7915,$(ccflags-y))
ccflags-y += -DMT7915
CONFIG_MTK_WIFI_CONNAC2X=y
CONFIG_MTK_WIFI_11AX_SUPPORT=y
CONFIG_MTK_WIFI_TWT_SUPPORT=y
CONFIG_MTK_WIFI_TWT_HOTSPOT_SUPPORT=n
CONFIG_MTK_WIFI_TWT_HOTSPOT_AC_SUPPORT=n
endif

ifneq ($(findstring 3_0,$(MTK_COMBO_CHIP)),)
ccflags-y:=$(filter-out -USOC3_0,$(ccflags-y))
ccflags-y += -DSOC3_0
CONFIG_MTK_WIFI_CONNAC2X=y
CONFIG_MTK_WIFI_11AX_SUPPORT=y
CONFIG_MTK_WIFI_TWT_SUPPORT=y
CONFIG_WIFI_TWT_STAPS_DISABLE_SCAN=y
CONFIG_MTK_WIFI_TWT_HOTSPOT_SUPPORT=n
CONFIG_MTK_WIFI_TWT_HOTSPOT_AC_SUPPORT=n
endif

ifneq ($(findstring MT7961,$(MTK_COMBO_CHIP)),)
ccflags-y:=$(filter-out -UMT7961,$(ccflags-y))
ccflags-y += -DMT7961
ccflags-y += -DCFG_SDIO_INTR_ENHANCE_FORMAT=2
ccflags-y += -DCFG_SUPPORT_SCHED_SCAN=1
ccflags-y += -DCFG_SUPPORT_MDNS_OFFLOAD=1
# MT7961's max cmd tx resource = 3
# so QM_CMD_RESERVED_THRESHOL must be less than 3
ccflags-y += -DQM_CMD_RESERVED_THRESHOLD=1
CONFIG_SUPPORT_WFDMA_REALLOC=y
CONFIG_MTK_WIFI_CONNAC2X=y
CONFIG_MTK_WIFI_11AX_SUPPORT=y
# unless patch wifi 6G setting in kernel before kernel version 5.4,
# it will build fail when enable wifi 6G flag
ifeq ($(call kver_ge,5,4),1)
CONFIG_MTK_WIFI_6G_SUPPORT=y
else
CONFIG_MTK_WIFI_6G_SUPPORT=n
endif
CONFIG_MTK_WIFI_TWT_SUPPORT=y
CONFIG_MTK_WIFI_TWT_HOTSPOT_SUPPORT=y
CONFIG_MTK_WIFI_TWT_HOTSPOT_AC_SUPPORT=n
CONFIG_WIFI_TWT_STAPS_DISABLE_SCAN=n
CONIFG_WIFI_TWT_WIFI_6E_CERTIFICATION=n
CONFIG_SUPPORT_WIFI_DL_BT_PATCH=y
CONFIG_SUPPORT_PCIE_ASPM=y
CONFIG_SUPPORT_PCIE_ASPM_IMPROVE=n
CONFIG_SUPPORT_CMD_OVER_WFDMA=y
CFG_SUPPORT_HOST_RX_WM_EVENT_FROM_PSE=y
CONFIG_MTK_WIFI_NAN=n
CONFIG_MTK_DBDC_SW_FOR_P2P_LISTEN=n
# AP KVR
ifeq ($(CFG_MTK_AP_80211KVR_INTERFACE), y)
	CONFIG_WIFI_SUPPORT_GET_NOISE=y
	CFG_WIFI_SUPPORT_NOISE_HISTOGRAM=y
	ccflags-y += -DCFG_AP_80211KVR_INTERFACE=1
	ccflags-y += -DCFG_AP_80211K_SUPPORT=1
	ccflags-y += -DCFG_AP_80211V_SUPPORT=1
endif

ifeq ($(CONFIG_WIFI_SUPPORT_GET_NOISE), y)
	ccflags-y += -DCONFIG_WIFI_SUPPORT_GET_NOISE=1
else
	ccflags-y += -DCONFIG_WIFI_SUPPORT_GET_NOISE=0
endif
ifeq ($(CFG_WIFI_SUPPORT_NOISE_HISTOGRAM), y)
	ccflags-y += -DCFG_WIFI_SUPPORT_NOISE_HISTOGRAM=1
else
	ccflags-y += -DCFG_WIFI_SUPPORT_NOISE_HISTOGRAM=0
endif
ifeq ($(CFG_IPI_2CHAIN_SUPPORT), y)
	ccflags-y += -DCFG_IPI_2CHAIN_SUPPORT=1
else
	ccflags-y += -DCFG_IPI_2CHAIN_SUPPORT=0
endif
ifeq ($(CFG_WIFI_PHY_SUPPORT_GET_BCNTH), y)
	ccflags-y += -DCFG_WIFI_PHY_SUPPORT_GET_BCNTH=1
else
	ccflags-y += -DCFG_WIFI_PHY_SUPPORT_GET_BCNTH=0
endif
ifeq ($(CFG_GET_CNM_INFO_BC), y)
    ccflags-y += -DCFG_GET_CNM_INFO_BC=1
else
    ccflags-y += -DCFG_GET_CNM_INFO_BC=0
endif
ifeq ($(CFG_WIFI_GET_MCS_INFO), y)
	ccflags-y += -DCFG_WIFI_GET_MCS_INFO=1
else
	ccflags-y += -DCFG_WIFI_GET_MCS_INFO=0
endif
ifeq ($(CFG_ENABLE_PS_INTV_CTRL), y)
    ccflags-y += -DCFG_ENABLE_PS_INTV_CTRL=1
else
    ccflags-y += -DCFG_ENABLE_PS_INTV_CTRL=0
endif
ifeq ($(CFG_SUPPORT_COEX_NON_COTX), y)
	ccflags-y += -DCFG_SUPPORT_COEX_NON_COTX=1
else
	ccflags-y += -DCFG_SUPPORT_COEX_NON_COTX=0
endif
ifeq ($(CFG_ADM_CTRL), y)
	ccflags-y += -DCFG_ADM_CTRL=1
else
	ccflags-y += -DCFG_ADM_CTRL=0
endif
ifeq ($(CONFIG_MTK_COMBO_WIFI_HIF), sdio)
CONFIG_TX_RSRC_WMM_ENHANCE=y
CONFIG_SUPPORT_MAILBOX_ACK=y
endif
ifeq ($(CONFIG_MTK_COMBO_WIFI_HIF), pcie)
CFG_COALESCING_INTERRUPT=y
endif
# For Buzzard, only PCIE & USB support
ifneq ($(CONFIG_MTK_COMBO_WIFI_HIF), sdio)
CONFIG_SUPPORT_DEBUG_SOP=y
endif
ccflags-y += -DCFG_SUPPORT_WOW_EINT=1
ccflags-y += -DCFG_SUPPORT_SUPPLICANT_SME=1
ccflags-y += -DCFG_SUPPORT_OWE=1
ccflags-y += -DCFG_REDIRECT_OID_SUPPORT=1
ccflags-y += -DCFG_SUPPORT_CFG80211_QUEUE=1
ccflags-y += -DCFG_RX_REPORT_FORMAT=1
ccflags-y += -DCFG_SUPPORT_HE_ER=0
ccflags-y += -DCFG_WIFI_FWDL_UMAC_RESERVE_SIZE_PARA=128
ccflags-y += -DCFG_WIFI_TX_ETH_CHK_EMPTY_PAYLOAD=1
# Default disable feature notify country code to bt
CONFIG_SUPPORT_BT_SKU=n
# For WAC module API
# 1: enable, 0: disable
ccflags-y += -DCFG_SUPPORT_WAC=0
endif

ifneq ($(findstring MT7922,$(MTK_COMBO_CHIP)),)
ccflags-y:=$(filter-out -UMT7922,$(ccflags-y))
ccflags-y += -DMT7922
ccflags-y += -DCFG_SDIO_INTR_ENHANCE_FORMAT=2
# MT7922's max cmd tx resource = 3
# so QM_CMD_RESERVED_THRESHOL must be less than 3
ccflags-y += -DQM_CMD_RESERVED_THRESHOLD=1
CONFIG_MTK_WIFI_CONNAC2X=y
CONFIG_MTK_WIFI_11AX_SUPPORT=y
CONFIG_MTK_WIFI_TWT_SUPPORT=y
CONFIG_MTK_WIFI_TWT_HOTSPOT_SUPPORT=n
CONFIG_MTK_WIFI_TWT_HOTSPOT_AC_SUPPORT=n
CONFIG_SUPPORT_WIFI_DL_BT_PATCH=n
CONFIG_SUPPORT_PCIE_ASPM=y
CONFIG_SUPPORT_PCIE_ASPM_IMPROVE=n
CONFIG_SUPPORT_CMD_OVER_WFDMA=y
CONFIG_SUPPORT_RETURN_TASK=y
CFG_SUPPORT_HOST_RX_WM_EVENT_FROM_PSE=y
CONFIG_SUPPORT_DEBUG_SOP=y
CONFIG_CONTROL_ASPM_BY_FW=y
ifeq ($(CONFIG_MTK_COMBO_WIFI_HIF), pcie)
CFG_COALESCING_INTERRUPT=y
endif
ccflags-y += -DCFG_SUPPORT_SUPPLICANT_SME=1
ccflags-y += -DCFG_SUPPORT_OWE=1
ccflags-y += -DCFG_REDIRECT_OID_SUPPORT=1
ccflags-y += -DCFG_SUPPORT_CFG80211_QUEUE=1
ccflags-y += -DCFG_RX_REPORT_FORMAT=2
ccflags-y += -DCFG_SUPPORT_BW160=1
ccflags-y += -DCFG_WIFI_TX_ETH_CHK_EMPTY_PAYLOAD=1
endif

ifneq ($(findstring MT7902,$(MTK_COMBO_CHIP)),)
ccflags-y:=$(filter-out -UMT7902,$(ccflags-y))
ccflags-y += -DMT7902
ccflags-y += -DCFG_SDIO_INTR_ENHANCE_FORMAT=2
ccflags-y += -DCFG_SUPPORT_SCHED_SCAN=1
# ccflags-y += -DQM_CMD_RESERVED_THRESHOLD=1
CONFIG_MTK_WIFI_CONNAC2X=y
CONFIG_MTK_WIFI_11AX_SUPPORT=y
ifeq ($(call kver_ge,5,4),1)
CONFIG_MTK_WIFI_6G_SUPPORT=y
else
CONFIG_MTK_WIFI_6G_SUPPORT=n
endif
CONFIG_MTK_WIFI_TWT_SUPPORT=y
CONFIG_MTK_WIFI_TWT_HOTSPOT_SUPPORT=y
CONFIG_MTK_WIFI_TWT_HOTSPOT_AC_SUPPORT=n
CONFIG_WIFI_TWT_STAPS_DISABLE_SCAN=n
CONFIG_SUPPORT_WIFI_DL_BT_PATCH=y
CONFIG_SUPPORT_WIFI_DL_ZB_PATCH=y
CONFIG_SUPPORT_PCIE_ASPM=y
CONFIG_SUPPORT_PCIE_ASPM_IMPROVE=n
CONFIG_SUPPORT_WIFI_DL_TEST_MODE=y
CONFIG_SUPPORT_DTIM_SKIP=n
# CONFIG_SUPPORT_CMD_OVER_WFDMA=y
# CFG_SUPPORT_HOST_RX_WM_EVENT_FROM_PSE=y
CONFIG_WIFI_RAM_MQM_BA_DELAY_SUPPORT=n
CONFIG_SUPPORT_DYNAMIC_PWR_LIMIT=y
# DYNAMIC_PWR_LIMIT_ANT_TAG depend on DYNAMIC_PWR_LIMIT
CONFIG_SUPPORT_DYNAMIC_PWR_LIMIT_ANT_TAG=y
ifeq ($(CONFIG_MTK_COMBO_WIFI_HIF), sdio)
CONFIG_TX_RSRC_WMM_ENHANCE=y
CONFIG_SUPPORT_MAILBOX_ACK=n
endif
ifeq ($(CONFIG_MTK_COMBO_WIFI_HIF), pcie)
CFG_COALESCING_INTERRUPT=y
endif
CONFIG_SUPPORT_DEBUG_SOP=y
CONFIG_NOT_CLR_FREE_MSDU_IN_ACTIVE_NETWORK=y
# Default disable feature notify country code to zb
CONFIG_SUPPORT_ZB_SKU=n

ccflags-y += -DCFG_SUPPORT_WOW_EINT=1
ccflags-y += -DCFG_SUPPORT_SUPPLICANT_SME=1
ccflags-y += -DCFG_SUPPORT_OWE=1
ccflags-y += -DCFG_REDIRECT_OID_SUPPORT=1
ccflags-y += -DCFG_SUPPORT_CFG80211_QUEUE=1
ccflags-y += -DCFG_RX_REPORT_FORMAT=2
ccflags-y += -DCFG_SUPPORT_HE_ER=1
ccflags-y += -DCFG_SUPPORT_DBDC=0
ccflags-y += -DCFG_SUPPORT_BW160=1
ccflags-y += -DCFG_WIFI_TX_ETH_CHK_EMPTY_PAYLOAD=1
ccflags-y += -DCFG_SUPPORT_SINGLE_SKU_LOCAL_DB=0
ccflags-y += -DCFG_SUPPORT_WOW_EINT_KEYEVENT_WAKEUP=1
endif

ifneq ($(findstring MT7933,$(MTK_COMBO_CHIP)),)
ccflags-y:=$(filter-out -UMT7961,$(ccflags-y))
ccflags-y += -DMT7933
CONFIG_MTK_WIFI_CONNAC2X=y
CONFIG_MTK_WIFI_11AX_SUPPORT=y
CONFIG_MTK_WIFI_TWT_SUPPORT=y
CONFIG_MTK_WIFI_TWT_HOTSPOT_SUPPORT=n
CONFIG_MTK_WIFI_TWT_HOTSPOT_AC_SUPPORT=n
# for defeature quality control
ccflags-y += -DCFG_SUPPORT_TDLS=0
ccflags-y += -DCFG_SUPPORT_SPE_IDX_CONTROL=0
ccflags-y += -DCFG_SUPPORT_DFS_MASTER=0
ccflags-y += -DCFG_SUPPORT_MULTITHREAD=0
ccflags-y += -DCFG_SUPPORT_802_11D=0
ccflags-y += -DCFG_SUPPORT_WAPI=0
ccflags-y += -DCFG_SUPPORT_DBDC=0
ccflags-y += -DCFG_SUPPORT_IDC_CH_SWITCH=0
ccflags-y += -DCFG_SUPPORT_LINK_QUALITY_MONITOR=0
ccflags-y += -DCFG_SUPPORT_CFG_FILE=0
ccflags-y += -DCFG_SUPPORT_SINGLE_SKU=0
ccflags-y += -DCFG_SUPPORT_SINGLE_SKU_LOCAL_DB=0
ccflags-y += -DCFG_WIFI_TX_ETH_CHK_EMPTY_PAYLOAD=1
endif

ifneq ($(findstring 5_0,$(MTK_COMBO_CHIP)),)
ccflags-y:=$(filter-out -USOC5_0,$(ccflags-y))
ccflags-y += -DSOC5_0
CONFIG_MTK_WIFI_CONNAC2X=y
CONFIG_MTK_WIFI_11AX_SUPPORT=y
CONFIG_MTK_WIFI_TWT_SUPPORT=y
CONFIG_MTK_WIFI_TWT_HOTSPOT_SUPPORT=n
CONFIG_MTK_WIFI_TWT_HOTSPOT_AC_SUPPORT=n
CONFIG_NUM_OF_WFDMA_RX_RING=3
CONFIG_NUM_OF_WFDMA_TX_RING=0
CONFIG_MTK_WIFI_CONNAC2X_2x2=y
#CONFIG_MTK_WIFI_CONNINFRA_SUPPORT=y
endif

ifneq ($(filter MT6639,$(MTK_COMBO_CHIP)),)
ccflags-y:=$(filter-out -UMT6639,$(ccflags-y))
ccflags-y += -DMT6639
CONFIG_MTK_WIFI_11AX_SUPPORT=y
CONFIG_MTK_WIFI_11BE_SUPPORT=y
CONFIG_MTK_WIFI_CONNAC3X=y
CONFIG_NUM_OF_WFDMA_RX_RING=2
CONFIG_NUM_OF_WFDMA_TX_RING=0
CONFIG_MTK_WIFI_UNIFIED_COMMND_SUPPORT=y
CONFIG_MTK_WIFI_TWT_SUPPORT=y
endif

ifeq ($(CONFIG_MTK_WIFI_CONNAC2X), y)
    ccflags-y += -DCFG_SUPPORT_CONNAC2X=1
else
    ccflags-y += -DCFG_SUPPORT_CONNAC2X=0
endif

ifeq ($(CONFIG_MTK_WIFI_CONNAC3X), y)
    ccflags-y += -DCFG_SUPPORT_CONNAC3X=1
else
    ccflags-y += -DCFG_SUPPORT_CONNAC3X=0
endif

ifeq ($(CONFIG_MTK_WIFI_11AX_SUPPORT), y)
    ccflags-y += -DCFG_SUPPORT_802_11AX=1
else
    ccflags-y += -DCFG_SUPPORT_802_11AX=0
endif

ifeq ($(CONFIG_MTK_WIFI_11BE_SUPPORT), y)
    ccflags-y += -DCFG_SUPPORT_802_11BE=1
else
    ccflags-y += -DCFG_SUPPORT_802_11BE=0
endif

ifeq ($(CONFIG_MTK_WIFI_11AX_SUPPORT), y)
ifeq ($(CONFIG_MTK_WIFI_6G_SUPPORT), y)
    ccflags-y += -DCFG_SUPPORT_WIFI_6G=1
    ccflags-y += -DCFG_SUPPORT_WIFI_DBDC6G=1
    ccflags-y += -DCFG_SUPPORT_TX_MGMT_USE_DATAQ=1
else
    ccflags-y += -DCFG_SUPPORT_WIFI_6G=0
    ccflags-y += -DCFG_SUPPORT_WIFI_DBDC6G=0
    ccflags-y += -DCFG_SUPPORT_TX_MGMT_USE_DATAQ=0
endif
else
    ccflags-y += -DCFG_SUPPORT_WIFI_6G=0
    ccflags-y += -DCFG_SUPPORT_WIFI_DBDC6G=0
    ccflags-y += -DCFG_SUPPORT_TX_MGMT_USE_DATAQ=0
endif

ifeq ($(CONFIG_MTK_DBDC_SW_FOR_P2P_LISTEN), y)
    ccflags-y += -DCFG_DBDC_SW_FOR_P2P_LISTEN=1
endif

ifeq ($(CONFIG_MTK_WIFI_TWT_SUPPORT), y)
    ccflags-y += -DCFG_SUPPORT_TWT=1
    ifeq ($(CONFIG_WIFI_TWT_STAPS_DISABLE_SCAN), y)
        ccflags-y += -DCFG_DISABLE_SCAN_UNDER_TWT=1
    else
        ccflags-y += -DCFG_DISABLE_SCAN_UNDER_TWT=0
    endif
    ifeq ($(CONFIG_MTK_WIFI_TWT_HOTSPOT_SUPPORT), y)
        ccflags-y += -DCFG_SUPPORT_TWT_HOTSPOT=1
    else
        ccflags-y += -DCFG_SUPPORT_TWT_HOTSPOT=0
    endif
    ifeq ($(CONFIG_MTK_WIFI_TWT_HOTSPOT_AC_SUPPORT), y)
        ccflags-y += -DCFG_SUPPORT_TWT_HOTSPOT_AC=1
    else
        ccflags-y += -DCFG_SUPPORT_TWT_HOTSPOT_AC=0
    endif
    ifeq ($(CONIFG_WIFI_TWT_WIFI_6E_CERTIFICATION), y)
        ccflags-y += -DCFG_KEEPFULLPOWER_BEFORE_TWT_NEGO=1
    else
        ccflags-y += -DCFG_KEEPFULLPOWER_BEFORE_TWT_NEGO=0
    endif
else
    ccflags-y += -DCFG_SUPPORT_TWT=0
    ccflags-y += -DCFG_SUPPORT_TWT_HOTSPOT=0
    ccflags-y += -DCFG_SUPPORT_TWT_HOTSPOT_AC=0
    ccflags-y += -DCFG_KEEPFULLPOWER_BEFORE_TWT_NEGO=0
endif

ifneq ($(CONFIG_MTK_COMBO_WIFI_HIF), pcie)
CONFIG_SUPPORT_PCIE_ASPM=n
endif
ifeq ($(CONFIG_SUPPORT_PCIE_ASPM), y)
    ccflags-y += -DCFG_SUPPORT_PCIE_ASPM=1
else
    ccflags-y += -DCFG_SUPPORT_PCIE_ASPM=0
endif

ifeq ($(CONFIG_SUPPORT_PCIE_ASPM_IMPROVE), y)
    ccflags-y += -DCFG_SUPPORT_PCIE_ASPM_IMPROVE=1
else
    ccflags-y += -DCFG_SUPPORT_PCIE_ASPM_IMPROVE=0
endif

ifeq ($(CONFIG_CONTROL_ASPM_BY_FW), y)
    ccflags-y += -DCFG_CONTROL_ASPM_BY_FW=1
else
    ccflags-y += -DCFG_CONTROL_ASPM_BY_FW=0
endif

ifeq ($(WIFI_ENABLE_GCOV), y)
GCOV_PROFILE := y
endif

ifeq ($(CONFIG_SUPPORT_SNIFFER_RADIOTAP), y)
    ccflags-y += -DCFG_SUPPORT_SNIFFER_RADIOTAP=1
    ccflags-y += -DCFG_SUPPORT_PDMA_SCATTER=1
else
    ccflags-y += -DCFG_SUPPORT_SNIFFER_RADIOTAP=0
    ccflags-y += -DCFG_SUPPORT_PDMA_SCATTER=0
endif

ccflags-y += -DCFG_DRIVER_INITIAL_RUNNING_MODE=3

ifneq ($(filter 6765, $(WLAN_CHIP_ID)),)
    ccflags-y += -DCFG_SUPPORT_DUAL_STA=0
else ifeq ($(CONFIG_MTK_WIFI_NAN), y)
    ccflags-y += -DCFG_SUPPORT_DUAL_STA=0
else ifeq ($(CONFIG_MTK_TC10_FEATURE), y)
    ccflags-y += -DCFG_SUPPORT_DUAL_STA=0
else
    ccflags-y += -DCFG_SUPPORT_DUAL_STA=1
endif

ifeq ($(MTK_ANDROID_WMT), y)
    ccflags-y += -DCFG_MTK_ANDROID_WMT=1
else ifneq ($(filter MT6632,$(MTK_COMBO_CHIP)),)
    ccflags-y += -DCFG_MTK_ANDROID_WMT=1
else
    ccflags-y += -DCFG_MTK_ANDROID_WMT=0
endif

ifeq ($(MTK_ANDROID_EMI), y)
    ccflags-y += -DCFG_MTK_ANDROID_EMI=1
else
    ccflags-y += -DCFG_MTK_ANDROID_EMI=0
endif

ifneq ($(WIFI_IP_SET),)
    ccflags-y += -DCFG_WIFI_IP_SET=$(WIFI_IP_SET)
else
    ccflags-y += -DCFG_WIFI_IP_SET=1
endif

ifneq ($(filter MTK_WCN_REMOVE_KERNEL_MODULE,$(KBUILD_SUBDIR_CCFLAGS)),)
    ccflags-y += -DCFG_BUILT_IN_DRIVER=1
else
    ccflags-y += -DCFG_BUILT_IN_DRIVER=0
endif

ifneq ($(findstring UT_TEST_MODE,$(MTK_COMBO_CHIP)),)
ccflags-y:=$(filter-out -UUT_TEST_MODE,$(ccflags-y))
ccflags-y += -DUT_TEST_MODE
endif

CONFIG_MTK_WIFI_MCC_SUPPORT=y
ifeq ($(CONFIG_MTK_WIFI_MCC_SUPPORT), y)
    ccflags-y += -DCFG_SUPPORT_CHNL_CONFLICT_REVISE=0
else
    ccflags-y += -DCFG_SUPPORT_CHNL_CONFLICT_REVISE=1
endif

ifeq ($(CONFIG_MTK_AEE_FEATURE), y)
    ccflags-y += -DCFG_SUPPORT_AEE=1
else
    ccflags-y += -DCFG_SUPPORT_AEE=0
endif

ifeq ($(CONFIG_MTK_COMBO_WIFI),y)
    ccflags-y += -DCFG_WPS_DISCONNECT=1
endif

ifeq ($(CONFIG_MTK_COMBO_WIFI_HIF), sdio)
    ccflags-y += -D_HIF_SDIO=1
else ifeq ($(CONFIG_MTK_COMBO_WIFI_HIF), pcie)
    ccflags-y += -D_HIF_PCIE=1
else ifeq ($(CONFIG_MTK_COMBO_WIFI_HIF), usb)
    ccflags-y += -D_HIF_USB=1
else ifeq ($(CONFIG_MTK_COMBO_WIFI_HIF), axi)
    ccflags-y += -D_HIF_AXI=1
else ifeq ($(CONFIG_MTK_COMBO_WIFI_HIF), ut)
    # Increase frame size to 2048 because of 'cfg80211_connect_result' exceed stack size
    ccflags-y += -D_HIF_UT=1 -Wno-unused-function -Wno-unused-variable -Wframe-larger-than=2048
else ifeq ($(CONFIG_MTK_COMBO_WIFI_HIF), none)
	ccflags-y += -D_HIF_NONE=1
else
    $(error Unsuppoted HIF=$(CONFIG_MTK_COMBO_WIFI_HIF)!!)
endif

ifeq ($(CONFIG_MTK_WIFI_POWER_ON_DOWNLOAD_EMI_ROM_PATCH), y)
    ccflags-y += -DCFG_POWER_ON_DOWNLOAD_EMI_ROM_PATCH=1
else
    ccflags-y += -DCFG_POWER_ON_DOWNLOAD_EMI_ROM_PATCH=0
endif

ifeq ($(CONFIG_MTK_WIFI_DOWNLOAD_DYN_MEMORY_MAP), y)
    ccflags-y += -DCFG_DOWNLOAD_DYN_MEMORY_MAP=1
else
    ccflags-y += -DCFG_DOWNLOAD_DYN_MEMORY_MAP=0
endif

ifeq ($(CONFIG_MTK_WIFI_ROM_PATCH_NO_SEM_CTRL), y)
    ccflags-y += -DCFG_ROM_PATCH_NO_SEM_CTRL=1
else
    ccflags-y += -DCFG_ROM_PATCH_NO_SEM_CTRL=0
endif

ifneq ($(CFG_CFG80211_VERSION),)
VERSION_STR = $(subst \",,$(subst ., , $(subst -, ,$(subst v,,$(CFG_CFG80211_VERSION)))))
$(info VERSION_STR=$(VERSION_STR))
X = $(firstword $(VERSION_STR))
Y = $(word 2 ,$(VERSION_STR))
Z = $(word 3 ,$(VERSION_STR))
VERSION := $(shell echo "$$(( $X * 65536 + $Y * 256 + $Z))" )
ccflags-y += -DCFG_CFG80211_VERSION=$(VERSION)
$(info DCFG_CFG80211_VERSION=$(VERSION))
endif


ifeq ($(CONFIG_MTK_PASSPOINT_R2_SUPPORT), y)
    ccflags-y += -DCFG_SUPPORT_PASSPOINT=1
    ccflags-y += -DCFG_HS20_DEBUG=1
    ccflags-y += -DCFG_ENABLE_GTK_FRAME_FILTER=1
else
    ccflags-y += -DCFG_SUPPORT_PASSPOINT=0
    ccflags-y += -DCFG_HS20_DEBUG=0
    ccflags-y += -DCFG_ENABLE_GTK_FRAME_FILTER=0
endif

MTK_MET_PROFILING_SUPPORT = yes
ifeq ($(MTK_MET_PROFILING_SUPPORT), yes)
    ccflags-y += -DCFG_MET_PACKET_TRACE_SUPPORT=1
else
    ccflags-y += -DCFG_MET_PACKET_TRACE_SUPPORT=0
endif

MTK_MET_TAG_SUPPORT = no
ifeq ($(MTK_MET_TAG_SUPPORT), yes)
    ccflags-y += -DMET_USER_EVENT_SUPPORT
    ccflags-y += -DCFG_MET_TAG_SUPPORT=1
else
    ccflags-y += -DCFG_MET_TAG_SUPPORT=0
endif

ifeq ($(CONFIG_MTK_TC10_FEATURE), y)
    ccflags-y += -DCFG_TC10_FEATURE=1
else
    ccflags-y += -DCFG_TC10_FEATURE=0
endif

ifeq ($(CONFIG_MTK_TC1_FEATURE), y)
    ccflags-y += -I$(srctree)/drivers/misc/mediatek/tc1_interface
    ccflags-y += -DCFG_TC1_FEATURE=1
else
    ccflags-y += -DCFG_TC1_FEATURE=0
endif

ifeq ($(CONFIG_MTK_WIFI_NAN), y)
ccflags-y += -DCFG_SUPPORT_NAN=1
else
ccflags-y += -DCFG_SUPPORT_NAN=0
endif

ifneq ($(filter 6779, $(WLAN_CHIP_ID)),)
    ifneq ($(filter MT6631, $(MTK_CONSYS_ADIE)),)
        ccflags-y += -DCFG_FLAVOR_FIRMWARE=1
    else
        ccflags-y += -DCFG_FLAVOR_FIRMWARE=0
    endif
else
    ccflags-y += -DCFG_FLAVOR_FIRMWARE=0
endif

ifeq ($(CONFIG_SUPPORT_WIFI_DL_BT_PATCH), y)
    ccflags-y += -DCFG_SUPPORT_WIFI_DL_BT_PATCH=1
else
    ccflags-y += -DCFG_SUPPORT_WIFI_DL_BT_PATCH=0
endif

ifeq ($(CONFIG_SUPPORT_WIFI_DL_ZB_PATCH), y)
    ccflags-y += -DCFG_SUPPORT_WIFI_DL_ZB_PATCH=1
else
    ccflags-y += -DCFG_SUPPORT_WIFI_DL_ZB_PATCH=0
endif

ifeq ($(CONFIG_SUPPORT_WIFI_DL_TEST_MODE), y)
    ccflags-y += -DCFG_SUPPORT_WIFI_DL_TEST_MODE=1
else
    ccflags-y += -DCFG_SUPPORT_WIFI_DL_TEST_MODE=0
endif

ifeq ($(CFG_SUPPORT_HOST_RX_WM_EVENT_FROM_PSE), y)
    ccflags-y += -DCFG_SUPPORT_HOST_RX_WM_EVENT_FROM_PSE=1
else
    ccflags-y += -DCFG_SUPPORT_HOST_RX_WM_EVENT_FROM_PSE=0
endif

ifeq ($(CONFIG_TX_RSRC_WMM_ENHANCE), y)
    ccflags-y += -DCFG_TX_RSRC_WMM_ENHANCE=1
else
    ccflags-y += -DCFG_TX_RSRC_WMM_ENHANCE=0
endif

ifeq ($(CONFIG_SUPPORT_MAILBOX_ACK), y)
    ccflags-y += -DCFG_SUPPORT_MAILBOX_ACK=1
else
    ccflags-y += -DCFG_SUPPORT_MAILBOX_ACK=0
endif

ifeq ($(CFG_COALESCING_INTERRUPT), y)
    ccflags-y += -DCFG_COALESCING_INTERRUPT=1
else
    ccflags-y += -DCFG_COALESCING_INTERRUPT=0
endif

ifeq ($(CONFIG_SUPPORT_DEBUG_SOP), y)
    ccflags-y += -DCFG_SUPPORT_DEBUG_SOP=1
else
    ccflags-y += -DCFG_SUPPORT_DEBUG_SOP=0
endif

ifeq ($(CONFIG_SUPPORT_CMD_OVER_WFDMA), y)
    ccflags-y += -DCFG_SUPPORT_CMD_OVER_WFDMA=1
else
    ccflags-y += -DCFG_SUPPORT_CMD_OVER_WFDMA=0
endif

ifeq ($(CONFIG_SUPPORT_WFDMA_REALLOC), y)
    ccflags-y += -DCFG_SUPPORT_WFDMA_REALLOC=1
else
    ccflags-y += -DCFG_SUPPORT_WFDMA_REALLOC=0
endif

ifeq ($(CONFIG_SUPPORT_RETURN_TASK), y)
    ccflags-y += -DCFG_SUPPORT_RETURN_TASK=1
else
    ccflags-y += -DCFG_SUPPORT_RETURN_TASK=0
endif

ifeq ($(CONFIG_SUPPORT_TX_TSO_SW), y)
    ccflags-y += -DCFG_SUPPORT_TX_TSO_SW=1
else
    ccflags-y += -DCFG_SUPPORT_TX_TSO_SW=0
endif

ifeq ($(CONFIG_SUPPORT_RX_NAPI), y)
    ccflags-y += -DCFG_SUPPORT_RX_NAPI=1
else
    ccflags-y += -DCFG_SUPPORT_RX_NAPI=0
endif

ifeq ($(CFG_SUPPORT_HIDDEN_SW_AP), y)
    ccflags-y += -DCFG_SUPPORT_HIDDEN_SW_AP=1
else
    ccflags-y += -DCFG_SUPPORT_HIDDEN_SW_AP=0
endif

ifeq ($(CONFIG_NOT_CLR_FREE_MSDU_IN_ACTIVE_NETWORK), y)
    ccflags-y += -DCFG_NOT_CLR_FREE_MSDU_IN_ACTIVE_NETWORK=1
else
    ccflags-y += -DCFG_NOT_CLR_FREE_MSDU_IN_ACTIVE_NETWORK=0
endif

# Tp Enhance
ifeq ($(CONFIG_MTK_TPENHANCE_MODE), y)
    ccflags-y += -DCFG_SUPPORT_TPENHANCE_MODE=1
    ccflags-y += -DCFG_FORCE_ENABLE_PERF_MONITOR=1
else
    ccflags-y += -DCFG_SUPPORT_TPENHANCE_MODE=0
endif

# GKI Support(can't use filp_open/close,kernel_read/write in GKI kernel, can't use kallsyms_lookup_name)
CONFIG_GKI_SUPPORT=y
ifeq ($(CONFIG_GKI_SUPPORT), y)
    ccflags-y += -DCFG_ENABLE_GKI_SUPPORT=1
    ccflags-y += -DCFG_DC_USB_WOW_CALLBACK=0
else
    ccflags-y += -DCFG_ENABLE_GKI_SUPPORT=0
endif

ifeq ($(CONFIG_SUPPORT_BT_SKU), y)
    ccflags-y += -DCFG_SUPPORT_BT_SKU
endif

ifeq ($(CONFIG_SUPPORT_ZB_SKU), y)
    ccflags-y += -DCFG_SUPPORT_ZB_SKU
endif

# Unified Command Support
ifeq ($(CONFIG_MTK_WIFI_UNIFIED_COMMND_SUPPORT), y)
ccflags-y += -DCFG_SUPPORT_UNIFIED_COMMAND
endif

# Debug Config for userdebug and eng load
ifneq ($(filter userdebug eng, $(TARGET_BUILD_VARIANT)),)
    ccflags-y += -DBUILD_QA_DBG=1
else
    ccflags-y += -DBUILD_QA_DBG=0
endif

ifeq ($(CONFIG_WIFI_RAM_MQM_BA_DELAY_SUPPORT), y)
ccflags-y += -DCONFIG_WIFI_RAM_MQM_BA_DELAY_SUPPORT=1
endif

# For Android Vendor Hal
# kernel >= 5.4: SHALL use 0, which means you must use the ALPS Android S vendor hal or patch your old hal with CL:5188442
# kernel < 5.4: may use 0 or 1, it depends on your vendor hal version:
#        a) if your hal is ALPS Android S hal, use 0;
#        b) if your hal version is before ALPS Android S hal, use 1;
ifeq ($(CFG_SUPPORT_OLD_VENDOR_HAL), y)
ccflags-y += -DCFG_SUPPORT_OLD_VENDOR_HAL=1
else
ccflags-y += -DCFG_SUPPORT_OLD_VENDOR_HAL=0
endif

ifeq ($(CONFIG_SUPPORT_DTIM_SKIP), y)
ccflags-y += -DCFG_SUPPORT_DTIM_SKIP=1
ccflags-y += -DMAX_SLEEP_TIME=600
endif

ifeq ($(CONFIG_SUPPORT_DYNAMIC_PWR_LIMIT), y)
ccflags-y += -DCFG_SUPPORT_DYNAMIC_PWR_LIMIT=1
endif

ifeq ($(CONFIG_SUPPORT_DYNAMIC_PWR_LIMIT_ANT_TAG), y)
ccflags-y += -DCFG_SUPPORT_DYNAMIC_PWR_LIMIT_ANT_TAG=1
endif

ifeq ($(MODULE_NAME),)
	MODULE_NAME := wlan_$(shell echo $(strip $(WLAN_CHIP_ID)) | tr A-Z a-z)_$(CONFIG_MTK_COMBO_WIFI_HIF)
endif

ccflags-y += -DDBG=0
ccflags-y += -I$(src)/os -I$(src)/os/$(os)/include
ccflags-y += -I$(src)/include -I$(src)/include/nic -I$(src)/include/mgmt -I$(src)/include/chips
ifeq ($(CONFIG_MTK_WIFI_NAN), y)
ccflags-y += -I$(src)/include/nan -I$(src)/include/nan/wpa_supp
endif
ifeq ($(CFG_SUPPORT_WIFI_SYSDVT), 1)
ccflags-y += -I$(src)/include/dvt
endif
ccflags-y += -I$(srctree)/drivers/misc/mediatek/base/power/include/
ccflags-y += -I$(srctree)/drivers/misc/mediatek/include/mt-plat/
ccflags-y += -I$(srctree)/drivers/misc/mediatek/performance/include/
ccflags-y += -I$(srctree)/drivers/misc/mediatek/emi/$(MTK_PLATFORM)
ccflags-y += -I$(srctree)/drivers/misc/mediatek/emi/submodule
ccflags-y += -I$(srctree)/drivers/devfreq/
ccflags-y += -I$(srctree)/net

ifeq ($(CONFIG_MTK_COMBO_WIFI_HIF), sdio)
ccflags-y += -I$(src)/os/$(os)/hif/sdio/include
else ifeq ($(CONFIG_MTK_COMBO_WIFI_HIF), pcie)
ccflags-y += -I$(src)/os/$(os)/hif/common/include
ccflags-y += -I$(src)/os/$(os)/hif/pcie/include
ifneq ($(findstring 3_0,$(MTK_COMBO_CHIP)),)
ccflags-y += -I$(src)/include/chips/coda/soc3_0
endif
else ifeq ($(CONFIG_MTK_COMBO_WIFI_HIF), axi)
ccflags-y += -I$(src)/os/$(os)/hif/common/include
ccflags-y += -I$(src)/os/$(os)/hif/axi/include
else ifeq ($(CONFIG_MTK_COMBO_WIFI_HIF), usb)
ccflags-y += -I$(src)/os/$(os)/hif/usb/include
else ifeq ($(CONFIG_MTK_COMBO_WIFI_HIF), ut)
ccflags-y += -I$(src)/test -I$(src)/test/lib/include -I$(src)/test/testcases -I$(src)/test/lib/hif
else ifeq ($(CONFIG_MTK_COMBO_WIFI_HIF), none)
ccflags-y += -I$(src)/os/$(os)/hif/none/include
endif

ifneq ($(PLATFORM_FLAGS), )
    ccflags-y += $(PLATFORM_FLAGS)
endif

ifeq ($(CONFIG_MTK_WIFI_ONLY),$(filter $(CONFIG_MTK_WIFI_ONLY),m y))
obj-$(CONFIG_MTK_WIFI_ONLY) += $(MODULE_NAME).o
else
obj-$(CONFIG_MTK_COMBO_WIFI) += $(MODULE_NAME).o
#obj-y += $(MODULE_NAME).o
endif

ifeq ($(CONFIG_WLAN_DRV_BUILD_IN),y)
$(warning $(MODULE_NAME) build-in boot.img)
obj-y += $(MODULE_NAME).o
else
$(warning $(MODULE_NAME) is kernel module)
obj-m += $(MODULE_NAME).o
endif

# ---------------------------------------------------
# Directory List
# ---------------------------------------------------
COMMON_DIR  := common/
OS_DIR      := os/$(os)/
HIF_COMMON_DIR := $(OS_DIR)hif/common/
ifeq ($(CONFIG_MTK_COMBO_WIFI_HIF), sdio)
HIF_DIR	    := os/$(os)/hif/sdio/
else ifeq ($(CONFIG_MTK_COMBO_WIFI_HIF), pcie)
HIF_DIR     := os/$(os)/hif/pcie/
else ifeq ($(CONFIG_MTK_COMBO_WIFI_HIF), axi)
HIF_DIR	    := os/$(os)/hif/axi/
else ifeq ($(CONFIG_MTK_COMBO_WIFI_HIF), usb)
HIF_DIR	    := os/$(os)/hif/usb/
else ifeq ($(CONFIG_MTK_COMBO_WIFI_HIF), ut)
HIF_DIR	    := test/lib/hif/
else ifeq ($(CONFIG_MTK_COMBO_WIFI_HIF), none)
HIF_DIR	    := os/$(os)/hif/none/
endif
NIC_DIR     := nic/
MGMT_DIR    := mgmt/
NAN_DIR     := nan/
CHIPS       := chips/
CHIPS_CMM   := $(CHIPS)common/

ifneq ($(MTK_PLATFORM),)
PLAT_DIR    := os/$(os)/plat/$(MTK_PLATFORM)/
endif
SYSDVT_DIR  := dvt/

# ---------------------------------------------------
# Objects List
# ---------------------------------------------------

COMMON_OBJS := 	$(COMMON_DIR)dump.o \
		$(COMMON_DIR)wlan_lib.o \
		$(COMMON_DIR)wlan_oid.o \
		$(COMMON_DIR)wlan_bow.o \
		$(COMMON_DIR)debug.o

NIC_OBJS := 	$(NIC_DIR)nic.o \
		$(NIC_DIR)nic_tx.o \
		$(NIC_DIR)nic_txd_v1.o \
		$(NIC_DIR)nic_rxd_v1.o \
		$(NIC_DIR)nic_rx.o \
		$(NIC_DIR)nic_pwr_mgt.o \
		$(NIC_DIR)nic_rate.o \
		$(NIC_DIR)cmd_buf.o \
		$(NIC_DIR)que_mgt.o \
		$(NIC_DIR)nic_cmd_event.o \
		$(NIC_DIR)nic_umac.o \
		$(NIC_DIR)radiotap.o

ifeq ($(os), none)
OS_OBJS := 	$(OS_DIR)gl_dependent.o \
		$(OS_DIR)gl_init.o \
		$(OS_DIR)gl_kal.o \
		$(OS_DIR)gl_ate_agent.o \
		$(OS_DIR)gl_qa_agent.o
else
OS_OBJS := 	$(OS_DIR)gl_init.o \
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
		$(OS_DIR)gl_sys.o \
		$(OS_DIR)gl_vendor.o \
		$(OS_DIR)gl_custom.o \
		$(OS_DIR)platform.o
endif

MGMT_OBJS := 	$(MGMT_DIR)ais_fsm.o \
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
		$(MGMT_DIR)rrm.o \
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

# ---------------------------------------------------
# Chips Objects List
# ---------------------------------------------------
MGMT_OBJS += $(MGMT_DIR)stats.o


CHIPS_OBJS += $(CHIPS_CMM)cmm_asic_connac.o
CHIPS_OBJS +=  $(CHIPS_CMM)dbg_connac.o
ifeq ($(CONFIG_MTK_WIFI_CONNAC2X), y)
CHIPS_OBJS +=  $(CHIPS_CMM)dbg_connac2x.o
endif

ifeq ($(CONFIG_MTK_WIFI_CONNAC2X), y)
CHIPS_OBJS += $(CHIPS_CMM)cmm_asic_connac2x.o
NIC_OBJS += $(NIC_DIR)nic_ext_cmd_event.o \
			$(NIC_DIR)nic_txd_v2.o \
			$(NIC_DIR)nic_rxd_v2.o
endif
ifeq ($(CONFIG_MTK_WIFI_CONNAC3X), y)
CHIPS_OBJS += $(CHIPS_CMM)cmm_asic_common.o
CHIPS_OBJS += $(CHIPS_CMM)cmm_asic_connac3x.o \
              $(CHIPS_CMM)dbg_connac3x.o \
              $(CHIPS_CMM)dbg_wtbl_connac3x.o
NIC_OBJS += $(NIC_DIR)nic_ext_cmd_event.o \
			$(NIC_DIR)nic_txd_v3.o \
			$(NIC_DIR)nic_rxd_v3.o
endif
CHIPS_OBJS += $(CHIPS_CMM)fw_dl.o

ifeq ($(CONFIG_MTK_WIFI_UNIFIED_COMMND_SUPPORT), y)
NIC_OBJS += $(NIC_DIR)nic_uni_cmd_event.o
endif

ifneq ($(filter MT6632,$(MTK_COMBO_CHIP)),)
CHIPS_OBJS += $(CHIPS)mt6632/mt6632.o
CHIPS_OBJS += $(CHIPS)mt6632/dbg_mt6632.o
endif
ifneq ($(filter MT7668,$(MTK_COMBO_CHIP)),)
CHIPS_OBJS += $(CHIPS)mt7668/mt7668.o
CHIPS_OBJS += $(CHIPS)mt7668/dbg_mt7668.o
endif
ifneq ($(filter MT7663,$(MTK_COMBO_CHIP)),)
CHIPS_OBJS += $(CHIPS)mt7663/mt7663.o
CHIPS_OBJS += $(CHIPS)mt7663/dbg_mt7663.o
endif
ifneq ($(filter CONNAC,$(MTK_COMBO_CHIP)),)
CHIPS_OBJS += $(CHIPS)connac/connac.o
endif
ifneq ($(filter CONNAC2X2,$(MTK_COMBO_CHIP)),)
CHIPS_OBJS += $(CHIPS)connac2x2/connac2x2.o
endif
ifneq ($(findstring MT7915,$(MTK_COMBO_CHIP)),)
CHIPS_OBJS += $(CHIPS)mt7915/mt7915.o
CHIPS_OBJS +=  $(CHIPS)mt7915/dbg_mt7915.o
endif
ifneq ($(findstring 3_0,$(MTK_COMBO_CHIP)),)
CHIPS_OBJS += $(CHIPS)soc3_0/soc3_0.o
CHIPS_OBJS += $(CHIPS)soc3_0/dbg_soc3_0.o
endif
ifneq ($(findstring MT7961,$(MTK_COMBO_CHIP)),)
CHIPS_OBJS += $(CHIPS)mt7961/mt7961.o
CHIPS_OBJS += $(CHIPS)mt7961/dbg_mt7961.o
CHIPS_OBJS += $(CHIPS)mt7961/hal_dmashdl_mt7961.o
CHIPS_OBJS += $(CHIPS)mt7961/hal_wfsys_reset_mt7961.o
endif
ifneq ($(findstring MT7933,$(MTK_COMBO_CHIP)),)
CHIPS_OBJS += $(CHIPS)mt7933/mt7933.o
CHIPS_OBJS += $(CHIPS)mt7933/dbg_mt7933.o
#CHIPS_OBJS += $(CHIPS)mt7933/hal_dmashdl_mt7933.o
#CHIPS_OBJS += $(CHIPS)mt7933/hal_wfsys_reset_mt7933.o
endif
ifneq ($(findstring MT7922,$(MTK_COMBO_CHIP)),)
CHIPS_OBJS += $(CHIPS)mt7922/mt7922.o
CHIPS_OBJS += $(CHIPS)mt7961/mt7961.o
CHIPS_OBJS += $(CHIPS)mt7961/dbg_mt7961.o
CHIPS_OBJS += $(CHIPS)mt7922/dbg_mt7922.o
CHIPS_OBJS += $(CHIPS)mt7961/hal_dmashdl_mt7961.o
CHIPS_OBJS += $(CHIPS)mt7961/hal_wfsys_reset_mt7961.o
endif
ifneq ($(findstring 5_0,$(MTK_COMBO_CHIP)),)
CHIPS_OBJS += $(CHIPS)soc5_0/soc5_0.o
CHIPS_OBJS += $(CHIPS)soc5_0/dbg_soc5_0.o
CHIPS_OBJS += $(CHIPS)soc5_0/hal_dmashdl_soc5_0.o
endif
ifneq ($(findstring MT7902,$(MTK_COMBO_CHIP)),)
CHIPS_OBJS += $(CHIPS)mt7902/mt7902.o
CHIPS_OBJS += $(CHIPS)mt7961/mt7961.o
CHIPS_OBJS += $(CHIPS)mt7961/dbg_mt7961.o
CHIPS_OBJS += $(CHIPS)mt7902/dbg_mt7902.o
CHIPS_OBJS += $(CHIPS)mt7902/hal_dmashdl_mt7902.o
CHIPS_OBJS += $(CHIPS)mt7961/hal_wfsys_reset_mt7961.o
endif
ifneq ($(filter MT6639,$(MTK_COMBO_CHIP)),)
CHIPS_OBJS += $(CHIPS)mt6639/mt6639.o
CHIPS_OBJS += $(CHIPS)mt6639/dbg_mt6639.o
CHIPS_OBJS += $(CHIPS)mt6639/hal_dmashdl_mt6639.o
endif

# ---------------------------------------------------
# P2P Objects List
# ---------------------------------------------------

COMMON_OBJS += $(COMMON_DIR)wlan_p2p.o

NIC_OBJS += $(NIC_DIR)p2p_nic.o

ifneq ($(os), none)
OS_OBJS += $(OS_DIR)gl_p2p.o \
           $(OS_DIR)gl_p2p_cfg80211.o \
           $(OS_DIR)gl_p2p_init.o \
           $(OS_DIR)gl_p2p_kal.o
endif

MGMT_OBJS += $(MGMT_DIR)p2p_dev_fsm.o\
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

MGMT_OBJS += $(MGMT_DIR)wapi.o

# ---------------------------------------------------
# NAN Objects List
# ---------------------------------------------------
ifeq ($(CONFIG_MTK_WIFI_NAN), y)
OS_OBJS  += $(OS_DIR)gl_nan.o \
            $(OS_DIR)gl_vendor_nan.o \
            $(OS_DIR)gl_vendor_ndp.o
NAN_OBJS := $(NAN_DIR)nan_dev.o \
            $(NAN_DIR)nanDiscovery.o\
            $(NAN_DIR)nanScheduler.o\
            $(NAN_DIR)nanReg.o\
            $(NAN_DIR)nan_data_engine.o\
            $(NAN_DIR)nan_data_engine_util.o\
            $(NAN_DIR)nan_ranging.o\
            $(NAN_DIR)nan_txm.o

NAN_SEC_OBJS := $(NAN_DIR)nan_sec.o\
                $(NAN_DIR)wpa_supp/FourWayHandShake.o\
                $(NAN_DIR)wpa_supp/src/ap/wpa_auth.o\
                $(NAN_DIR)wpa_supp/src/crypto/sha1.o\
                $(NAN_DIR)wpa_supp/src/crypto/sha1-internal.o\
                $(NAN_DIR)wpa_supp/src/crypto/sha1-prf.o\
                $(NAN_DIR)wpa_supp/src/common/wpa_common.o\
                $(NAN_DIR)wpa_supp/src/crypto/aes-wrap.o\
                $(NAN_DIR)wpa_supp/src/crypto/aes-internal.o\
                $(NAN_DIR)wpa_supp/src/utils/common.o\
                $(NAN_DIR)wpa_supp/src/rsn_supp/wpa.o\
                $(NAN_DIR)wpa_supp/src/crypto/aes-unwrap.o\
                $(NAN_DIR)wpa_supp/src/crypto/aes-internal-enc.o\
                $(NAN_DIR)wpa_supp/src/crypto/aes-internal-dec.o\
                $(NAN_DIR)wpa_supp/src/crypto/sha256.o\
                $(NAN_DIR)wpa_supp/src/crypto/sha256-prf.o\
                $(NAN_DIR)wpa_supp/src/crypto/sha256-internal.o\
                $(NAN_DIR)wpa_supp/wpa_supplicant/wpas_glue.o\
                $(NAN_DIR)wpa_supp/wpa_supplicant/wpa_supplicant.o\
                $(NAN_DIR)wpa_supp/src/ap/wpa_auth_glue.o\
                $(NAN_DIR)wpa_supp/src/crypto/pbkdf2-sha256.o\
                $(NAN_DIR)wpa_supp/src/crypto/sha384-internal.o\
                $(NAN_DIR)wpa_supp/src/crypto/sha512-internal.o\
                $(NAN_DIR)wpa_supp/src/crypto/sha384-prf.o\
                $(NAN_DIR)wpa_supp/src/crypto/sha384.o
endif

# ---------------------------------------------------
# HE Objects List
# ---------------------------------------------------

COMMON_OBJS += $(COMMON_DIR)wlan_he.o

ifeq ($(CONFIG_MTK_WIFI_11AX_SUPPORT), y)
MGMT_OBJS += $(MGMT_DIR)he_ie.o \
             $(MGMT_DIR)he_rlm.o
endif

ifeq ($(CONFIG_MTK_WIFI_TWT_SUPPORT), y)
MGMT_OBJS += $(MGMT_DIR)twt_req_fsm.o \
             $(MGMT_DIR)twt.o \
             $(MGMT_DIR)twt_planner.o
endif

ifeq ($(CONFIG_MTK_COMBO_WIFI_HIF), sdio)
HIF_OBJS :=  $(HIF_DIR)arm.o \
             $(HIF_DIR)sdio.o \
             $(HIF_DIR)hal_api.o \
             $(HIF_DIR)sdio_test_driver_core.o \
             $(HIF_DIR)sdio_test_driver_ops.o
else ifeq ($(CONFIG_MTK_COMBO_WIFI_HIF), pcie)
HIF_OBJS :=  $(HIF_COMMON_DIR)hal_pdma.o \
             $(HIF_COMMON_DIR)kal_pdma.o \
             $(HIF_COMMON_DIR)dbg_pdma.o \
             $(HIF_DIR)pcie.o
else ifeq ($(CONFIG_MTK_COMBO_WIFI_HIF), axi)
HIF_OBJS :=  $(HIF_COMMON_DIR)hal_pdma.o \
             $(HIF_COMMON_DIR)kal_pdma.o \
             $(HIF_COMMON_DIR)dbg_pdma.o \
             $(HIF_DIR)axi.o
else ifeq ($(CONFIG_MTK_COMBO_WIFI_HIF), usb)
HIF_OBJS :=  $(HIF_DIR)usb.o \
             $(HIF_DIR)hal_api.o
else ifeq ($(CONFIG_MTK_COMBO_WIFI_HIF), ut)
HIF_OBJS :=  $(HIF_DIR)ut.o \
             $(HIF_DIR)hal_api.o
else ifeq ($(CONFIG_MTK_COMBO_WIFI_HIF), none)
HIF_OBJS :=  $(HIF_DIR)none.o
endif
# ---------------------------------------------------
# Platform Objects List
# ---------------------------------------------------
ifneq ($(MTK_PLATFORM),)

PLAT_PRIV_C = $(src)/$(PLAT_DIR)plat_priv.c

# search path (out of kernel tree)
IS_EXIST_PLAT_PRIV_C := $(wildcard $(PLAT_PRIV_C))
# search path (build-in kernel tree)
IS_EXIST_PLAT_PRIV_C += $(wildcard $(srctree)/$(PLAT_PRIV_C))

ifneq ($(strip $(IS_EXIST_PLAT_PRIV_C)),)
PLAT_OBJS := $(PLAT_DIR)plat_priv.o
$(MODULE_NAME)-objs  += $(PLAT_OBJS)
endif
endif

# ---------------------------------------------------
# System Dvt Objects List
# ---------------------------------------------------
ifeq ($(CFG_SUPPORT_WIFI_SYSDVT), 1)
SYSDVT_OBJS += $(SYSDVT_DIR)dvt_common.o

ifeq ($(CFG_SUPPORT_DMASHDL_SYSDVT), 1)
SYSDVT_OBJS += $(SYSDVT_DIR)dvt_dmashdl.o
endif
endif

# ---------------------------------------------------
# Service git List
# ---------------------------------------------------
SERVICE_DIR  := wlan_service/

ifneq ($(findstring wlan_service,$(MTK_WLAN_SERVICE_PATH)),)
MTK_WLAN_SERVICE=yes
SERVICE_DIR  := $(MTK_WLAN_SERVICE_PATH)
$(info SERVICE_DIR is [{$(MTK_WLAN_SERVICE_PATH)}])
endif

ifeq ($(MTK_WLAN_SERVICE), yes)
ccflags-y += -DCONFIG_WLAN_SERVICE=1
ccflags-y += -DCONFIG_TEST_ENGINE_OFFLOAD=1
ccflags-y += -I$(src)/$(SERVICE_DIR)include
ccflags-y += -I$(src)/$(SERVICE_DIR)service/include
ccflags-y += -I$(src)/$(SERVICE_DIR)glue/osal/include
ccflags-y += -I$(src)/$(SERVICE_DIR)glue/hal/include
$(info $$CCFLAG is [{$(ccflags-y)}])
SERVICE_OBJS := $(SERVICE_DIR)agent/agent.o \
                $(SERVICE_DIR)service/service_test.o \
                $(SERVICE_DIR)service/test_engine.o \
                $(SERVICE_DIR)glue/osal/gen4m/sys_adaption_gen4m.o \
                $(SERVICE_DIR)glue/osal/gen4m/net_adaption_gen4m.o \
                $(SERVICE_DIR)glue/hal/gen4m/operation_gen4m.o
$(MODULE_NAME)-objs  += $(SERVICE_OBJS)
$(info $$MTK_WLAN_SERVICE is [{$(SERVICE_OBJS)}])
else
ccflags-y += -DCONFIG_WLAN_SERVICE=0
ccflags-y += -DCONFIG_TEST_ENGINE_OFFLOAD=0
endif

$(MODULE_NAME)-objs  += $(COMMON_OBJS)
$(MODULE_NAME)-objs  += $(NIC_OBJS)
$(MODULE_NAME)-objs  += $(OS_OBJS)
$(MODULE_NAME)-objs  += $(HIF_OBJS)
$(MODULE_NAME)-objs  += $(MGMT_OBJS)
$(MODULE_NAME)-objs  += $(CHIPS_OBJS)
$(MODULE_NAME)-objs  += $(SYSDVT_OBJS)
$(MODULE_NAME)-objs  += $(NAN_OBJS)
$(MODULE_NAME)-objs  += $(NAN_SEC_OBJS)

ifneq ($(findstring UT_TEST_MODE,$(MTK_COMBO_CHIP)),)
include $(src)/test/ut.make
endif

#
# mtprealloc
#
ifeq ($(CONFIG_MTK_PREALLOC_MEMORY), y)
ccflags-y += -DCFG_PREALLOC_MEMORY
ccflags-y += -I$(src)/prealloc/include
MODULE_NAME_PREALLOC = $(MODULE_NAME)_prealloc
PREALLOC_OBJS := prealloc/prealloc.o
$(MODULE_NAME_PREALLOC)-objs += $(PREALLOC_OBJS)
obj-m += $(MODULE_NAME_PREALLOC).o
endif

#
# mtreset
#
ifeq ($(CONFIG_GKI_SUPPORT), y)
ccflags-y += -DCFG_CHIP_RESET_KO_SUPPORT
ccflags-y += -I$(src)/reset/include
ifeq ($(CONFIG_MTK_COMBO_WIFI_HIF), sdio)
MODULE_NAME_RESET = wlan_sdio_reset
endif
RESET_OBJS := reset/reset.o
$(MODULE_NAME_RESET)-objs += $(RESET_OBJS)
obj-m += $(MODULE_NAME_RESET).o
endif
