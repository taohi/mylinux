ifneq ($(KERNELRELEASE),)
	obj-m:=mmap.o
else
	KERNELDIR ?= /lib/modules/$(shell uname -r)/build
	PWD := $(shell pwd)
default:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules
	gcc test.c -o test.out
clean:
	rm -rf *.ko *.mod.c *.o *.order *.symvers *.out
install:
	insmod mmap.ko
	mknod /dev/mmap_dev c 244 0
uninstall:
	rmmod mmap.ko
	rm /dev/mmap_dev
endif
