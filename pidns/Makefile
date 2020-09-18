obj-m := pidns.o

modules clean:
	make -C /lib/modules/$(shell uname -r)/build M=${PWD} $@
