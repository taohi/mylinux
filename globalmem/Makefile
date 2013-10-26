ifneq ($(KERNELRELEASE),)
	obj-m:=globalmem.o
else
	KERNELDIR ?= /lib/modules/$(shell uname -r)/build
	PWD := $(shell pwd)
default:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules
#	gcc test.c -o test.out
clean:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) clean
install:
	insmod globalmem.ko
	mknod /dev/globalmem c 250 0
uninstall:
	rmmod globalmem.ko
	rm /dev/globalmem
endif
