# SPDX-License-Identifier: GPL-2.0
#
# Makefile for the Linux Bluetooth HCI device drivers.
#

obj-$(CONFIG_BT_HCIBTUSB)	+= btusb.o

PWD := $(CURDIR)

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

