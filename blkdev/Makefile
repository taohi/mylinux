ifneq ($(KERNELRELEASE),)
	obj-m:=simp_blkdev.o
else
	KERNELDIR ?= /lib/modules/$(shell uname -r)/build
	PWD := $(shell pwd)
default:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules
clean:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) clean
install:
	insmod simp_blkdev.ko
uninstall:
	rmmod simp_blkdev.ko
endif
