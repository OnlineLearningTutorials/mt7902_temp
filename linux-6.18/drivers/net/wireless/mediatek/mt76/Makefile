# SPDX-License-Identifier: ISC


obj-m += mt7902e.o

CFLAGS_trace.o := -I$(src)
CFLAGS_usb_trace.o := -I$(src)
CFLAGS_mt76x02_trace.o := -I$(src)
CFLAGS_mt792x_trace.o := -I$(src)

mt7902e-y := \
	mmio.o util.o trace.o dma.o mac80211.o debugfs.o eeprom.o \
	tx.o agg-rx.o mcu.o wed.o scan.o channel.o pci.o

mt7902e-y += mt76_connac_mcu.o mt76_connac_mac.o mt76_connac3_mac.o

mt7902e-y += mt792x_core.o mt792x_mac.o mt792x_trace.o \
		mt792x_debugfs.o mt792x_dma.o mt792x_acpi_sar.o
		
		
mt7902e-y += mt7902/pci.o mt7902/pci_mac.o mt7902/pci_mcu.o mt7902/init.o mt7902/main.o mt7902/mcu.o mt7902/mac.o \
 mt7902/debugfs.o 

# mt7902e-y += mt7902_orig/pci.o mt7902_orig/pci_mac.o mt7902_orig/pci_mcu.o mt7902_orig/init.o mt7902_orig/main.o mt7902_orig/mcu.o mt7902_orig/mac.o \
#  mt7902_orig/debugfs.o 

# mt7902e-y += mt7902_7915/pci.o mt7902_7915/init.o mt7902_7915/dma.o mt7902_7915/eeprom.o mt7902_7915/main.o mt7902_7915/mcu.o mt7902_7915/mac.o \
# 	     mt7902_7915/debugfs.o mt7902_7915/mmio.o mt7902_7915/coredump.o
 

#  mt7902-common-y := mac.o mcu.o main.o init.o debugfs.o
# mt7902-common-$(CONFIG_NL80211_TESTMODE) += testmode.o
# mt7902e-y := pci.o pci_mac.o pci_mcu.o
# mt7902s-y := sdio.o sdio_mac.o sdio_mcu.o
# mt7902u-y := usb.o


all:
	$(MAKE) -C /lib/modules/`uname -r`/build/ M=`pwd` modules

clean:
	$(MAKE) -C /lib/modules/`uname -r`/build/ M=`pwd` clean



