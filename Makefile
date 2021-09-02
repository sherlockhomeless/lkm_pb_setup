obj-m+= base_module.o

all:
	make -C /lib/modules/$(shell uname -r)/build/ M=$(PWD) modules
clean:
	make -C /lib/modules/$(shell uname -r)/build/ M=$(PWD) clean
in:
	sudo insmod base_module.ko
out:
	sudo rmmod base_module
