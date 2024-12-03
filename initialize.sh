#!/bin/bash

# Remove existing image if it exists
if [ -f storage_vgc.img ]; then
    rm -f storage_vgc.img
fi

# Create a new 100MB disk image
dd if=/dev/zero of=storage_vgc.img bs=1M count=1

# Format the image as ext4
sudo mkfs.ext4 storage_vgc.img