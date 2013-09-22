ifneq ($(KERNELRELEASE),)
	obj-m:=list_head.o
else
	KERNELDIR ?= /lib/modules/$(shell uname -r)/build
	PWD := $(shell pwd)
default:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules
clean:
	rm -rf *.ko *.mod.c *.o *.order *.symvers 
install:
	insmod list_head.ko
uninstall:
	rmmod list_head.ko
#	mknod /dev/ c 103 0
endif
