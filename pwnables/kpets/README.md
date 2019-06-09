## Summary
 - category: pwnables
 - name: kpets
 - difficulty: easy-medium
 - flag: See "flag" file


## Vulnerability
 - Players can add pets to the kernel by writing data in a certain format to /dev/kpets
 - There is a double fetch bug when reading the pet header
 - Players can modify the size field of the pet and overflow the kernel heap to add a new pet of type dragon
 - When reading from /dev/kpets if there is a pet of type dragon then the flag will be returned

## Requirements
 - `socat`
 - `qemu-system`
 - `kvm` - AWS EC2 doesn't support nested virtualisation, so a DigitalOcean box will be needed for this challenge
 - python2
 - gcc

## Build
 - `cd src; make`
 - `cd ..`
 - `./pack_initramfs.sh qemu/initramfs/ src/kpets.ko`

## Run
 - `./run_challenge.sh`, this will listen on 0.0.0.0:1337
