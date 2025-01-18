PROG    = hello.exe

CC      = arm-none-eabi-gcc
LD      = arm-fpm-ld
OBJDUMP = arm-none-eabi-objdump
NM      = arm-none-eabi-nm
READELF = arm-none-eabi-readelf

CFLAGS  = -mcpu=cortex-m0plus -mthumb -fPIC -O1 -I../include
LDFLAGS = -shared -fPIC -e main

all:    $(PROG)

clean:
	rm -f *.o *.exe *.dis *.nm *.readelf

%.exe:  %.c
	$(CC) $(CFLAGS) -c $< -o $*.o
	$(LD) $(LDFLAGS) $*.o -o $@
	$(OBJDUMP) -d $@ > $*.dis
	$(NM) -n $@ > $*.nm
	$(READELF) -a $@ > $*.readelf
