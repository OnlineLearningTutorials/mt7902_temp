# SPDX-License-Identifier: ISC

obj-m += mt7902_2-common.o
obj-m += mt7902_2e.o
obj-m += mt7902_2u.o

mt7902_2-common-y := mac.o mcu.o main.o init.o debugfs.o
mt7902_2e-y := pci.o pci_mac.o pci_mcu.o
mt7902_2u-y := usb.o





all: 
	$(MAKE) -C /lib/modules/`uname -r`/build/ M=`pwd` modules


clean: 
	$(MAKE) -C /lib/modules/`uname -r`/build/ M=`pwd` clean
	
	
