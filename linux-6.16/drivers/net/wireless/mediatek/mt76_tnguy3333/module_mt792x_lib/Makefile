# SPDX-License-Identifier: GPL-2.0-only

obj-m += mt792x-lib.o

CFLAGS_mt792x_trace.o := -I$(src)

mt792x-lib-y := \
	mt792x_core.o \
	mt792x_mac.o \
	mt792x_debugfs.o \
	mt792x_dma.o \
	mt792x_trace.o 

mt792x-lib-$(CONFIG_ACPI) += mt792x_acpi_sar.o


all:
	$(MAKE) -C /lib/modules/`uname -r`/build/ M=`pwd` modules

clean:
	$(MAKE) -C /lib/modules/`uname -r`/build/ M=`pwd` clean
