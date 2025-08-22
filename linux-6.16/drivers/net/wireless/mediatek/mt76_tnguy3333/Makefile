# SPDX-License-Identifier: GPL-2.0-only
obj-m += mt76.o
obj-m += mt76-connac-lib.o
obj-m += mt792x-lib.o

mt76-y := \
	mmio.o util.o trace.o dma.o mac80211.o debugfs.o eeprom.o \
	tx.o agg-rx.o mcu.o wed.o scan.o channel.o

mt76-$(CONFIG_PCI) += pci.o
mt76-$(CONFIG_NL80211_TESTMODE) += testmode.o

CFLAGS_trace.o := -I$(src)
CFLAGS_mt792x_trace.o := -I$(src)

mt76-connac-lib-y := mt76_connac_mcu.o mt76_connac_mac.o mt76_connac3_mac.o

mt792x-lib-y := mt792x_core.o mt792x_mac.o mt792x_trace.o \
		mt792x_debugfs.o mt792x_dma.o
mt792x-lib-$(CONFIG_ACPI) += mt792x_acpi_sar.o

obj-m += mt7902/

all:
	$(MAKE) -C /lib/modules/`uname -r`/build/ M=`pwd` modules

clean:
	$(MAKE) -C /lib/modules/`uname -r`/build/ M=`pwd` clean
