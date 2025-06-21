# SPDX-License-Identifier: ISC

obj-m += besra.o

besra-y := pci.o init.o dma.o eeprom.o main.o mcu.o mac.o \
	     debugfs.o mmio.o

# besra-$(CONFIG_NL80211_TESTMODE) += testmode.o


all: 
	$(MAKE) -C /lib/modules/`uname -r`/build/ M=`pwd` modules
clean: 
	$(MAKE) -C /lib/modules/`uname -r`/build/ M=`pwd` clean
