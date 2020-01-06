obj-m += block_device.o

KDIR = /usr/src/linux-headers-5.3.0-kali3-amd64

all:
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) modules

clean:
	rm -rf -v *.o *.ko *.mod.* *.symvers *.order
