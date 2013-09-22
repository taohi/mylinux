ifneq ($(KERNELRELEASE),)
	obj-m:=kerfile.o
else
	KERNELDIR ?= /lib/modules/$(shell uname -r)/build
	PWD := $(shell pwd)
default:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules
clean:
	rm -rf *.ko *.mod.c *.o *.order *.symvers 
install:
	insmod kerfile.ko
uninstall:
	rmmod kerfile.ko
#	mknod /dev/ c 103 0
endif
