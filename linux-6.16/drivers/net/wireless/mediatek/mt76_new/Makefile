# SPDX-License-Identifier: ISC


obj-m += mt7902.o

CFLAGS_trace.o := -I$(src)
CFLAGS_usb_trace.o := -I$(src)
CFLAGS_mt76x02_trace.o := -I$(src)
CFLAGS_mt792x_trace.o := -I$(src)

mt7902-y := \
	mmio.o util.o trace.o dma.o mac80211.o debugfs.o eeprom.o \
	tx.o agg-rx.o mcu.o wed.o scan.o channel.o pci.o

mt7902-y += mt76_connac_mcu.o mt76_connac_mac.o mt76_connac3_mac.o

mt7902-y += mt792x_core.o mt792x_mac.o mt792x_trace.o \
		mt792x_debugfs.o mt792x_dma.o mt792x_acpi_sar.o

mt7902-y += mt7902_mt7921/mac.o mt7902_mt7921/mcu.o mt7902_mt7921/main.o mt7902_mt7921/init.o mt7902_mt7921/debugfs.o
# mt7902e-y += testmode.o
mt7902-y += mt7902_mt7921/pci.o mt7902_mt7921/pci_mac.o mt7902_mt7921/pci_mcu.o



all:
	$(MAKE) -C /lib/modules/`uname -r`/build/ M=`pwd` modules

clean:
	$(MAKE) -C /lib/modules/`uname -r`/build/ M=`pwd` clean



