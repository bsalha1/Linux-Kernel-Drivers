obj-m += currentmod.o

KDIR = /usr/src/linux-headers-5.2.0-kali3-amd64

all:
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) modules

clean:
	rm -rf *.o *.ko *.mod.* *.symvers *.order
