# SPDX-License-Identifier: GPL-2.0-only
obj-m += mt76.o

mt76-y := \
	mmio.o \
	util.o \
	dma.o \
	mac80211.o \
	debugfs.o \
	eeprom.o \
	tx.o \
	agg-rx.o \
	mcu.o \
	wed.o \
	scan.o \
	channel.o \
	trace.o 


mt76-$(CONFIG_PCI) += pci.o
mt76-$(CONFIG_NL80211_TESTMODE) += testmode.o

CFLAGS_trace.o := -I$(src)



all:
	$(MAKE) -C /lib/modules/`uname -r`/build/ M=`pwd` modules

clean:
	$(MAKE) -C /lib/modules/`uname -r`/build/ M=`pwd` clean
