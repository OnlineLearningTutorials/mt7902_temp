obj-m :=  mt7902.o


mt7902-y := mac.o mcu.o main.o init.o debugfs.o pci.o pci_mac.o pci_mcu.o


all: 
	$(MAKE) -C /lib/modules/`uname -r`/build/ M=`pwd` modules


clean: 
	$(MAKE) -C /lib/modules/`uname -r`/build/ M=`pwd` clean
