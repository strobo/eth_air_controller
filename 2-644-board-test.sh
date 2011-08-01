#!/bin/sh -x
make clean
make MCU=atmega644 test_OKworks
echo 
sleep 1
make DUDECPUTYPE=m644 load_test_OKworks
sleep 1
make DUDECPUTYPE=m644 fuse
