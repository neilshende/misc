#
# Copyright (C) 2012- Maxta, Inc
#

KERNELDIR ?= /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

EXTRA_CFLAGS += $(OPTFLAG) -I$(MXFSINCPATH) -I$(MXFSBUILDDIR)

obj-m := mxfs.o
mxfs-objs := dev.o dir.o file.o inode.o

all:
	make -C $(KERNELDIR) M=$(PWD) modules

clean:
	make -C $(KERNELDIR) M=$(PWD) clean



