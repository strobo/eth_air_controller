#!/bin/sh -x
make clean
make MCU=atmega328p test_OKworks
echo 
sleep 1
make DUDECPUTYPE=m328p load_test_OKworks
sleep 1
make DUDECPUTYPE=m328p fuse
