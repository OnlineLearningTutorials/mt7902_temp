LOCAL_PATH := $(call my-dir)

MT7961_CHIPS := MT7920 MT7921
ifeq ($(MTK_WLAN_SUPPORT), yes)
ifeq ($(WLAN_BUILD_COMMON), true)
    # for layer decoupling 2.0, we have to build all configurations
    MTK_PLATFORM := mt6789
    WIFI_CHIP := MT7902
    WIFI_HIF := sdio
    WIFI_WMT := n
    WIFI_EMI := n
    WIFI_GKI := y
    WIFI_NAME := wlan_mt7902_sdio_mt6789
    include $(LOCAL_PATH)/build_wlan_drv.mk
else
ifneq (MT7921, $(MTK_COMBO_CHIP)),)
    include $(CLEAR_VARS)
    LOCAL_MODULE := wlan_drv_gen4_mt7921.ko
    LOCAL_PROPRIETARY_MODULE := true
    LOCAL_MODULE_OWNER := mtk
    LOCAL_INIT_RC := init.wlan_mt79xx_drv.rc
    LOCAL_SRC_FILES := $(patsubst $(LOCAL_PATH)/%,%,$(shell find $(LOCAL_PATH) -type f -name '*.[cho]')) Makefile
    include $(MTK_KERNEL_MODULE)

    WIFI_OPTS := CONFIG_MTK_COMBO_WIFI_HIF=pcie
    WIFI_OPTS += MTK_COMBO_CHIP=MT7961
    $(linked_module): OPTS += $(WIFI_OPTS)
endif
ifeq (MT7902, $(strip $(MTK_COMBO_CHIP)))
    include $(CLEAR_VARS)
    LOCAL_MODULE := wlan_drv_gen4_mt7902.ko
    LOCAL_PROPRIETARY_MODULE := true
    LOCAL_MODULE_OWNER := mtk
    LOCAL_INIT_RC := init.wlan_mt79xx_drv.rc
    LOCAL_SRC_FILES := $(patsubst $(LOCAL_PATH)/%,%,$(shell find $(LOCAL_PATH) -type f -name '*.[cho]')) Makefile
    include $(MTK_KERNEL_MODULE)

    WIFI_OPTS := CONFIG_MTK_COMBO_WIFI_HIF=sdio
    WIFI_OPTS += MTK_COMBO_CHIP=MT7902
    $(linked_module): OPTS += $(WIFI_OPTS)
endif
endif
endif