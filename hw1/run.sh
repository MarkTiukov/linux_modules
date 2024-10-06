#!/bin/bash

rmmod phonebook
sudo rm /dev/my_numbers
sudo mknod /dev/my_numbers c 700 0
sudo chmod a+rw /dev/my_numbers
make
insmod phonebook.ko
