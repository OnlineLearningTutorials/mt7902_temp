# SPDX-License-Identifier: ISC

obj-m += mt76.o
obj-m += mt76-connac-lib.o
obj-m += mt792x-lib.o
obj-m += mt7921/

.PHONY: all clean module_compile module_compress module_install

all: module_compile module_compress module_install

MOD_DIR = /lib/modules/$(shell uname -r)/kernel/drivers/net/wireless/mediatek/mt76
ZSTD_BINS = mt76.ko.zst mt76-connac-lib.ko.zst mt792x-lib.ko.zst

CFLAGS_trace.o := -I$(src)
CFLAGS_usb_trace.o := -I$(src)
CFLAGS_mt76x02_trace.o := -I$(src)
CFLAGS_mt792x_trace.o := -I$(src)


mt76-y := \
	mmio.o util.o trace.o dma.o mac80211.o debugfs.o eeprom.o \
	tx.o agg-rx.o mcu.o wed.o scan.o channel.o pci.o

mt76-connac-lib-y := mt76_connac_mcu.o mt76_connac_mac.o mt76_connac3_mac.o

mt792x-lib-y := mt792x_core.o mt792x_mac.o mt792x_trace.o \
		mt792x_debugfs.o mt792x_dma.o mt792x_acpi_sar.o


module_compile: 
	@echo "-- Module compilation"
	$(MAKE) -C /lib/modules/`uname -r`/build/ M=`pwd` modules
	
	
module_compress: $(ZSTD_BINS)
	@echo "-- Module Compression"
	@find . -name "*.ko" -exec zstd -f {} \;
	

%.ko.zst: %.ko
	zstd $<


module_install: $(MOD_DIR)
	@echo "-- Installing modules to $(MOD_DIR)"
	# Install the base libs
	sudo install -v -m 644 *.ko.zst $(MOD_DIR)
	# Install the specific chip drivers from subfolders
	sudo install -v -m 644 mt7921/*.ko.zst $(MOD_DIR)/mt7921/ 2>/dev/null || true
	# Refresh module dependencies
	sudo depmod -a


clean: 
	$(MAKE) -C /lib/modules/`uname -r`/build/ M=`pwd` clean
	rm -f $(ZSTD_BINS)
	
	
