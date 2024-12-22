ARCH    := $(shell uname -m)
OS      := $(shell uname -s)
LIBS    = rpm.so

ifeq ($(OS), Linux)
    PREFIX  =
    INC     = -I../include
endif
ifeq ($(OS), Darwin)
    PREFIX  = $(ARCH)-elf-
    SYSINC  = $(shell xcrun --show-sdk-path)/usr/include
    INC     = -I../include -I$(SYSINC)
endif

all:    hello.elf

clean:
	rm -f *.o *.so *.elf *.dis *.nm *.readelf *.trace

hello.elf: hello.c $(LIBS)
	$(PREFIX)gcc -fPIC -g -O1 $(INC) -c hello.c -o hello.o
	$(PREFIX)ld -shared -fPIC -g -e main hello.o $(LIBS) -o $@
	$(PREFIX)objdump -d $@ > hello.dis
	$(PREFIX)nm -n $@ > hello.nm
	$(PREFIX)readelf -a $@ > hello.readelf

rpm.so: rpm-stub.c
	$(PREFIX)gcc -fPIC -g -O1 $(INC) -c rpm-stub.c -o rpm-stub.o
	$(PREFIX)ld -shared -fPIC rpm-stub.o -o $@
	$(PREFIX)objdump -d $@ > rpm.dis
	$(PREFIX)nm -n $@ > rpm.nm
	$(PREFIX)readelf -a $@ > rpm.readelf
