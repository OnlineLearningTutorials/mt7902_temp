# SPDX-License-Identifier: ISC

obj-$(CONFIG_BESRA) += besra.o

besra-y := pci.o init.o dma.o eeprom.o main.o mcu.o mac.o \
	     debugfs.o mmio.o

besra-$(CONFIG_NL80211_TESTMODE) += testmode.o