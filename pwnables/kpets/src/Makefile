obj-m += kpets.o

# Download this directory from http://pwn.partners/linux-5.1.5.zip and update the path accordingly
# You must use this one and not a fresh copy of this folder from kernel.org, because this zip contains
# the relevant config files

KDIR="/home/fuzz/fbctf-2019/pwnables/kpets/linux-5.1.5/"

all:
			make -C $(KDIR) M=$(PWD) modules
			$(CC) exploit.c -o exploit -g -static

clean:
			make -C $(KDIR) M=$(PWD) clean
