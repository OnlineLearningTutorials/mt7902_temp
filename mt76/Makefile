# SPDX-License-Identifier: GPL-2.0-only
obj-m += mt7902_pci.o

mt7902_pci-y := mmio.o util.o trace.o dma.o mac80211.o debugfs.o eeprom.o tx.o agg-rx.o mcu.o pci.o  \
	 mt76_connac_mcu.o mt76_connac_mac.o mt76_connac3_mac.o \
	 mt792x_core.o mt792x_mac.o mt792x_trace.o mt792x_debugfs.o mt792x_dma.o mt792x_acpi_sar.o
	 
mt7902_pci-y += mt7902/mac.o mt7902/mcu.o mt7902/main.o mt7902/init.o mt7902/debugfs.o mt7902/pci.o mt7902/pci_mac.o mt7902/pci_mcu.o



CFLAGS_trace.o := -I$(src)
CFLAGS_usb_trace.o := -I$(src)
CFLAGS_mt76x02_trace.o := -I$(src)
CFLAGS_mt792x_trace.o := -I$(src)




all: 
	$(MAKE) -C /lib/modules/`uname -r`/build/ M=`pwd` modules


clean: 
	$(MAKE) -C /lib/modules/`uname -r`/build/ M=`pwd` clean
	
