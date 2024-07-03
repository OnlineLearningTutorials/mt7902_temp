# SPDX-License-Identifier: GPL-2.0-only
obj-$(CONFIG_MT7601U)	+= mt7601u/
obj-$(CONFIG_MT76_CORE)	+= mt76/



all: 
	$(MAKE) -C /lib/modules/`uname -r`/build/ M=`pwd` modules


clean: 
	$(MAKE) -C /lib/modules/`uname -r`/build/ M=`pwd` clean
	
	
