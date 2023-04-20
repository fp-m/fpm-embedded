CC      = x86_64-elf-gcc
SYSINC  = $(shell xcrun --show-sdk-path)/usr/include
CFLAGS  = -O1 -g -I../../include -I$(SYSINC)
LDFLAGS = -g -e main
LIBS    = rpm-x86_64.so

all:    hello.elf

clean:
	rm -f *.o *.elf

hello.elf: hello.c
	$(CC) $(CFLAGS) -c -fPIC -fno-plt hello.c -o hello.o
	$(CC) $(LDFLAGS) -shared -fPIC -fno-plt hello.o $(LIBS) -o $@
