# SPDX-License-Identifier: GPL-2.0-only

obj-m += mt76-connac-lib.o

mt76-connac-lib-y := \
	mt76_connac_mcu.o \
	mt76_connac_mac.o \
	mt76_connac3_mac.o


all:
	$(MAKE) -C /lib/modules/`uname -r`/build/ M=`pwd` modules

clean:
	$(MAKE) -C /lib/modules/`uname -r`/build/ M=`pwd` clean
