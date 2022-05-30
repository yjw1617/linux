ifeq ($(KERNELRELEASE),)

ifeq ($(ARCH),arm)
KERNELDIR ?= /home/makeru/Linux_4412/kernel/linux-3.14
ROOTFS ?= /opt/4412/rootfs
else
KERNELDIR ?= /lib/modules/$(shell uname -r)/build
endif
PWD := $(shell pwd)

modules:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules

modules_install:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules INSTALL_MOD_PATH=$(ROOTFS) modules_install

clean:
	rm -rf  *.o  *.ko  .*.cmd  *.mod.*  modules.order  Module.symvers   .tmp_versions

else
#对应myhello.ko
obj-m += myhello.o

#多个.c文件
obj-m += xyz.o
xyz-objs = test.o func.o

#生成testparam.ko
obj-m += testparam.o

#生成testparam.ko
obj-m += mychar.o

#生成testparam.ko
obj-m += multimychar.o
endif

