# SPDX-License-Identifier: ISC

obj-m += mt7902-common.o
obj-m += mt7902e.o
obj-m += mt7902s.o
obj-m += mt7902u.o

mt7902-common-y := mac.o mcu.o main.o init.o debugfs.o
# mt7902-common-$(CONFIG_NL80211_TESTMODE) += testmode.o
mt7902e-y := pci.o pci_mac.o pci_mcu.o
mt7902s-y := sdio.o sdio_mac.o sdio_mcu.o
mt7902u-y := usb.o





all: 
	$(MAKE) -C /lib/modules/`uname -r`/build/ M=`pwd` modules


clean: 
	$(MAKE) -C /lib/modules/`uname -r`/build/ M=`pwd` clean
	
	
