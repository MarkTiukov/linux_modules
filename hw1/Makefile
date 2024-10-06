obj-m += phonebook.o

all:
    make -C /lib/modules/$(shell uname -r)/build M=$(PWD) module
clean:
    make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
