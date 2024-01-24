ifneq ($(KERNELRELEASE),)
# kbuild part of makefile
obj-m  := afs.o
afs-y  := afs_main.o afs_fs.o afs_inode.o afs_client.o

ccflags-y := -I$(src)/..

else
# normal makefile
KDIR ?= /lib/modules/`uname -r`/build

all:
	$(MAKE) -C $(KDIR) M=$$PWD modules
 
clean:
	$(MAKE) -C $(KDIR) M=$$PWD clean

endif