# SPDX-License-Identifier: ISC

obj-m += mt7921-common.o
obj-m += mt7921e.o

.PHONY: all clean module_compile module_compress module_install

all: module_compile module_compress module_install

MOD_DIR = /lib/modules/$(shell uname -r)/kernel/drivers/net/wireless/mediatek/mt76/mt7921
ZSTD_BINS = mt7921-common.ko.zst mt7921e.ko.zst

mt7921-common-y := mac.o mcu.o main.o init.o debugfs.o
mt7921e-y := pci.o pci_mac.o pci_mcu.o


module_compile: 
	@echo "-- Module compilation"
	$(MAKE) -C /lib/modules/`uname -r`/build/ M=`pwd` modules
	
	
module_compress: $(ZSTD_BINS)
	@echo "-- Module Compression"
	

%.ko.zst: %.ko
	zstd $<


module_install: $(MOD_DIR)
	@echo "-- Copying $(ZSTD_BINS) to $(MOD_DIR) (requires sudo)"
	# Use 'install' command for better handling of permissions/ownership
	sudo install -m 644 $(ZSTD_BINS) $(MOD_DIR)


clean: 
	$(MAKE) -C /lib/modules/`uname -r`/build/ M=`pwd` clean
	rm -f $(ZSTD_BINS)
	
	
