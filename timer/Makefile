ifneq ($(KERNELRELEASE),)
	obj-m:=mytimer.o
else
	KERNELDIR ?= /lib/modules/$(shell uname -r)/build
	PWD := $(shell pwd)
default:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules
#gcc test.c -o test.out
clean:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) clean
install:
	insmod mytimer.ko
uninstall:
	rmmod mytimer.ko
endif
