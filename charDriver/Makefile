obj-m += char_device.o

KDIR = /lib/modules/$(shell uname -r)/build

all:
	make -C $(KDIR) M=$(PWD) modules

app:
	gcc app.c -o app.o

clean:
	make -C $(KDIR) M=$(PWD) clean
