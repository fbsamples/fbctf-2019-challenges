#!/bin/bash

# Use this to add the kpets kernel module and pack thee initramfs
# $1 should be /path/to/initramfs
# $2 should be /path/to/kpets.ko

[ $# -lt 2 ] && { echo Usage: $1 /path/to/initramfs /path/to/kpets.ko; exit; }

cp $2 $1/lib/modules/5.1.5/
cd $1/bin
./busybox --install .

# Pack the initramfs
cd ..
find . | cpio -H newc -ov -F ../initramfs.cpio 2>/dev/null

