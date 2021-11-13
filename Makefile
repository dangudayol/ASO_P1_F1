obj-m+=Fase1.o

all:
	make -C /lib/modules/$(shell uname -r)/build/ M=$(PWD) modules
clean:
	make -C /lib/modules/$(shell uname -r)/build/ M=$(PWD) clean
install:
	insmod Fase1.ko
remove:
	rmmod Fase1.ko
