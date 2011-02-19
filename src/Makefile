ifneq ($(KERNELRELEASE),)
# Wenn die make-Variable "KERNELRELEASE" definiert ist,
# wurde dieses Makefile vom Kernel-Build-System aufgerufen.
# Dann reicht die folgende Zeile aus, um das Modul zu bauen.
obj-m    := mcs9815.o hldrv.o
mcs9815-objs := mcs9815_main.o mcs9815_ops.o
else
# Hierher kommt man, wenn make direkt aus dem
# Quellverzeichnis aufgerufen wurde. Dann wird make
# rekursiv mit den gezeigten Parametern aufgerufen und
# damit das Kernel-Build-System aktiviert.
KDIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)
default:
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) modules
clean:
	rm *.o *.ko

test:
	rm test -f
	gcc -o test test.c --std=c99
endif

