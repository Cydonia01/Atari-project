#!/bin/bash

sudo umount mount

rm -f ./device_file

LOOP_DEVICE=$(sudo losetup -j storage_vgc.img | cut -d ':' -f 1) 
sudo losetup -d "$LOOP_DEVICE"