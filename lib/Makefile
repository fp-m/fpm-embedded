ARCH    := $(shell uname -m)
CC      = $(ARCH)-elf-gcc
LD      = $(CC)
SYSINC  = $(shell xcrun --show-sdk-path)/usr/include
INC     = -I../include -I$(SYSINC)
LIBS    = rpm.so

all:    hello.elf

clean:
	rm -f *.o *.so *.elf *.dis *.nm *.readelf *.trace

hello.elf: hello.c $(LIBS)
	$(ARCH)-elf-gcc -fPIC -g -O1 $(INC) -c hello.c -o hello.o
	$(ARCH)-elf-ld -shared -fPIC -g -e main hello.o $(LIBS) -o $@
	$(ARCH)-elf-objdump -d $@ > hello.dis
	$(ARCH)-elf-nm -n $@ > hello.nm
	$(ARCH)-elf-readelf -a $@ > hello.readelf

rpm.so: rpm-stub.c
	$(ARCH)-elf-gcc -fPIC -g -O1 $(INC) -c rpm-stub.c -o rpm-stub.o
	$(ARCH)-elf-ld -shared -fPIC rpm-stub.o -o $@
	$(ARCH)-elf-objdump -d $@ > rpm.dis
	$(ARCH)-elf-nm -n $@ > rpm.nm
	$(ARCH)-elf-readelf -a $@ > rpm.readelf
