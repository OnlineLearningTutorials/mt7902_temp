# SPDX-License-Identifier: ISC

obj-$(CONFIG_mt7902) += mt7902.o

mt7902-y := pci.o init.o dma.o eeprom.o main.o mcu.o mac.o \
	     debugfs.o mmio.o

mt7902-$(CONFIG_NL80211_TESTMODE) += testmode.o