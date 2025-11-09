# SPDX-License-Identifier: ISC

obj-m += mt7902e.o

mt7902e-y := pci.o init.o dma.o eeprom.o main.o mcu.o mac.o \
	     debugfs.o mmio.o coredump.o
# mac.o mcu.o main.o init.o debugfs.o pci.o pci_mac.o pci_mcu.o


all:
	$(MAKE) -C /lib/modules/`uname -r`/build/ M=`pwd` modules

clean:
	$(MAKE) -C /lib/modules/`uname -r`/build/ M=`pwd` clean
