#!/usr/bin/bash

if [ ! -d mount ]; then
    mkdir mount
fi
LOOP_DEVICE=$(sudo losetup -f --show "storage_vgc.img");
ln -s "$LOOP_DEVICE" ./device_file
sudo mount "$LOOP_DEVICE" mount