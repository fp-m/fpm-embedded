TOP             = ..
CFLAGS          = -g -O -Wall -I$(TOP)/include
LDFLAGS         = -g
LIBS            = -lcmocka
OBJ             = editline_test.o \
                  $(TOP)/kernel/rpm_editline.o

ifneq ($(wildcard /usr/local/include),)
CFLAGS		+= -I/usr/local/include
endif
ifneq ($(wildcard /opt/homebrew/include),)
CFLAGS		+= -I/opt/homebrew/include
endif
ifneq ($(wildcard /usr/local/lib),)
LIBS       += -L/usr/local/lib
endif
ifneq ($(wildcard /opt/homebrew/lib),)
LIBS       += -L/opt/homebrew/lib
endif

all:		unit_tests

test:           unit_tests
		./unit_tests

clean:
		rm -f unit_tests *.o $(TOP)/kernel/*.o

unit_tests:     $(OBJ)
		$(CC) $(LDFLAGS) $(OBJ) $(LIBS) -o $@
