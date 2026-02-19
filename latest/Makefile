# SPDX-License-Identifier: ISC


obj-m += mt7921e.o

CFLAGS_trace.o := -I$(src)
CFLAGS_usb_trace.o := -I$(src)
CFLAGS_mt76x02_trace.o := -I$(src)
CFLAGS_mt792x_trace.o := -I$(src)

mt7921e-y += \
	mmio.o util.o trace.o dma.o mac80211.o debugfs.o eeprom.o \
	tx.o agg-rx.o mcu.o wed.o scan.o channel.o pci.o

mt7921e-y += mt76_connac_mcu.o mt76_connac_mac.o mt76_connac3_mac.o

mt7921e-y += mt792x_core.o mt792x_mac.o mt792x_trace.o \
		mt792x_debugfs.o mt792x_dma.o mt792x_acpi_sar.o
		
		
mt7921e-y += mt7921/pci.o mt7921/pci_mac.o mt7921/pci_mcu.o mt7921/init.o mt7921/main.o mt7921/mcu.o mt7921/mac.o \
 mt7921/debugfs.o 

# mt7921e-y += mt7921_orig/pci.o mt7921_orig/pci_mac.o mt7921_orig/pci_mcu.o mt7921_orig/init.o mt7921_orig/main.o mt7921_orig/mcu.o mt7921_orig/mac.o \
#  mt7921_orig/debugfs.o 

# mt7921e-y += mt7921_7915/pci.o mt7921_7915/init.o mt7921_7915/dma.o mt7921_7915/eeprom.o mt7921_7915/main.o mt7921_7915/mcu.o mt7921_7915/mac.o \
# 	     mt7921_7915/debugfs.o mt7921_7915/mmio.o mt7921_7915/coredump.o
 

#  mt7921-common-y := mac.o mcu.o main.o init.o debugfs.o
# mt7921-common-$(CONFIG_NL80211_TESTMODE) += testmode.o
# mt7921e-y := pci.o pci_mac.o pci_mcu.o
# mt7921s-y := sdio.o sdio_mac.o sdio_mcu.o
# mt7921u-y := usb.o


all:
	$(MAKE) -C /lib/modules/`uname -r`/build/ M=`pwd` modules

clean:
	$(MAKE) -C /lib/modules/`uname -r`/build/ M=`pwd` clean



