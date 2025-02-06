obj-m += btusb.o

obj-m := btmtk.o

all: 
	$(MAKE) -C /lib/modules/`uname -r`/build/ M=`pwd` modules
clean: 
	$(MAKE) -C /lib/modules/`uname -r`/build/ M=`pwd` clean
