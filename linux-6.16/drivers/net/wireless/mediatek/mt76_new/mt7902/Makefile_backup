# SPDX-License-Identifier: ISC

obj-m += mt7902e.o

mt7902e-y := pci.o init.o dma.o eeprom.o main.o mcu.o mac.o \
	     debugfs.o mmio.o

mt7902e-$(CONFIG_NL80211_TESTMODE) += testmode.o
mt7902e-$(CONFIG_MT798X_WMAC) += soc.o
mt7902e-$(CONFIG_DEV_COREDUMP) += coredump.o


all:
	$(MAKE) -C /lib/modules/`uname -r`/build/ M=`pwd` modules

clean:
	$(MAKE) -C /lib/modules/`uname -r`/build/ M=`pwd` clean
