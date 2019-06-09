#CC           = avr-gcc
#CFLAGS       = -Wall -mmcu=atmega16 -Os -Wl,-Map,test.map
#OBJCOPY      = avr-objcopy
CC           = g++
LD           = g++
AR           = ar
ARFLAGS      = rcs
CFLAGS       = -std=c++11 -Wall -Os -c
LDFLAGS      = -std=c++11 -Wall -Os -static -W

OBJCOPYFLAGS = -j .text -O ihex
OBJCOPY      = objcopy

# include path to AVR library
INCLUDE_PATH = /usr/lib/avr/include
# splint static check
SPLINT       = splint main.cpp utils.cpp aes.c -I$(INCLUDE_PATH) +charindex -unrecog

default: main

.SILENT:
.PHONY:  lint clean

main.o : main.cpp aes.h utils.h aes.o utils.o
	echo [CC] $@ $(CFLAGS)
	$(CC) $(CFLAGS) -o  $@ $<

aes.o : aes.c aes.h
	echo [CC] $@ $(CFLAGS)
	$(CC) $(CFLAGS) -o $@ $<

utils.o : utils.cpp utils.h
	echo [CC] $@ $(CFLAGS)
	$(CC) $(CFLAGS) -o $@ $<

main : aes.o main.o utils.o
	echo [LD] $@
	$(LD) $(LDFLAGS) -o $@ $^
	strip -s main

aes.a : aes.o
	echo [AR] $@
	$(AR) $(ARFLAGS) $@ $^

utils.a :utils.o
	echo [AR] $@
	$(AR) $(ARFLAGS) $@ $^

lib : aes.a utils.a

clean:
	rm -f *.OBJ *.LST *.o *.gch *.out *.hex *.map *.a main temp.bin

lint:
	$(call SPLINT)
