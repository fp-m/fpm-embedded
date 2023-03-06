TOP             = ..
CFLAGS          = -g -O0 -Wall -I$(TOP)/include
LDFLAGS         = -g
LIBS            = -lcmocka
TESTS           = tokenize_test getopt_test dotw_test fatfs_test
TOKENIZE_OBJ    = tokenize_test.o \
                  rpm_tokenize.o
GETOPT_OBJ      = getopt_test.o \
                  rpm_getopt.o
DOTW_OBJ        = dotw_test.o \
                  rpm_get_dotw.o
FATFS_OBJ       = fatfs_test.o \
                  fatfs.o \
                  unicode.o
vpath %.c $(TOP)/kernel $(TOP)/fatfs

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

all:		$(TESTS)

test:           $(TESTS)
		./tokenize_test
		./getopt_test
		./dotw_test
		./fatfs_test

clean:
		rm -f *_test *.img *.o

tokenize_test:  $(TOKENIZE_OBJ)
		$(CC) $(LDFLAGS) $(TOKENIZE_OBJ) $(LIBS) -o $@

getopt_test:    $(GETOPT_OBJ)
		$(CC) $(LDFLAGS) $(GETOPT_OBJ) $(LIBS) -o $@

dotw_test:      $(DOTW_OBJ)
		$(CC) $(LDFLAGS) $(DOTW_OBJ) $(LIBS) -o $@

fatfs_test:     $(FATFS_OBJ)
		$(CC) $(LDFLAGS) $(FATFS_OBJ) $(LIBS) -o $@
