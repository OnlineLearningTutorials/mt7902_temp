obj-m += btusb.o

obj-m += btmtk.o

.PHONY: all clean module_compile module_compress module_install

all: module_compile module_compress module_install

MOD_DIR = /lib/modules/$(shell uname -r)/kernel/drivers/bluetooth
ZSTD_BINS = btusb.ko.zst btmtk.ko.zst

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
