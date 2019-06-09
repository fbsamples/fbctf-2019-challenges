CFLAGS=-Wall -Wextra -m32 -s  -fvisibility=hidden -O3

main:
	gcc -Wall -Wextra -m32 -s  -fvisibility=hidden -O3 -o main main.c
	strip -s main
	printf '\x02' | dd conv=notrunc of=./main bs=1 seek=5

check:
	splint main.c
