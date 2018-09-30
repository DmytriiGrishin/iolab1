obj-m = ch_drv.o
PWD = $(shell pwd)
all:
	make -C /lib/modules/$(shell uname -r)/build M="$(PWD)" modules
clean:
	make -C /lib/modules/$(shell uname -r)/build M="$(PWD)" clean
reload:
	rmmod ch_drv && insmod ch_drv.ko && chmod 777 /dev/var1