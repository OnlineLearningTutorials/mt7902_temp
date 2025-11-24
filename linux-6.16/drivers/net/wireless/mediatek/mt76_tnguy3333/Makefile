# SPDX-License-Identifier: ISC

obj-m += mt7902e.o

mt7902e-y := \
	mt76_module/mmio.o \
	mt76_module/util.o \
	mt76_module/dma.o \
	mt76_module/mac80211.o \
	mt76_module/debugfs.o \
	mt76_module/eeprom.o \
	mt76_module/tx.o \
	mt76_module/agg-rx.o \
	mt76_module/mcu.o \
	mt76_module/wed.o \
	mt76_module/scan.o \
	mt76_module/channel.o \
	mt76_module/trace.o 


mt7902e-$(CONFIG_PCI) += mt76_module/pci.o
mt7902e-$(CONFIG_NL80211_TESTMODE) += mt76_module/testmode.o

CFLAGS_trace.o := -I$(mt76_module/src)
CFLAGS_mt792x_trace.o := -I$(mt76_module/src)

mt7902e-y += \
	mt76_connac_lib_module/mt76_connac_mcu.o \
	mt76_connac_lib_module/mt76_connac_mac.o \
	mt76_connac_lib_module/mt76_connac3_mac.o

mt7902e-y += \
	mt792x_lib_module/mt792x_core.o \
	mt792x_lib_module/mt792x_mac.o \
	mt792x_lib_module/mt792x_debugfs.o \
	mt792x_lib_module/mt792x_dma.o \
	mt792x_lib_module/mt792x_trace.o 

mt7902e-$(CONFIG_ACPI) += mt792x_lib_module/mt792x_acpi_sar.o


mt7902e-y += mt7902/mac.o mt7902/mcu.o mt7902/main.o mt7902/init.o \
			mt7902/debugfs.o mt7902/pci.o mt7902/pci_mac.o mt7902/pci_mcu.o


all:
	$(MAKE) -C /lib/modules/`uname -r`/build/ M=`pwd` modules

clean:
	$(MAKE) -C /lib/modules/`uname -r`/build/ M=`pwd` clean
