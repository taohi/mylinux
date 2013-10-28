ifneq ($(KERNELRELEASE),)
	obj-m:=globalfifo.o
else
	KERNELDIR ?= /lib/modules/$(shell uname -r)/build
	PWD := $(shell pwd)
default:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules
#	gcc test.c -o test.out
clean:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) clean
install:
	insmod globalfifo.ko
	mknod /dev/globalfifo c 250 0
uninstall:
	rmmod globalfifo.ko
	rm /dev/globalfifo
endif
