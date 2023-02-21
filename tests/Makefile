TOP             = ..
CFLAGS          = -g -O0 -Wall -I$(TOP)/include
LDFLAGS         = -g
LIBS            = -lcmocka
TESTS           = editline_test tokenize_test getopt_test dotw_test fatfs_test
EDITLINE_OBJ    = editline_test.o \
                  rpm_editline.o \
                  rpm_puts.o \
                  rpm_wputs.o \
                  rpm_strwlen.o \
                  rpm_getwch.o \
                  rpm_getkey.o \
                  rpm_putwch.o \
                  rpm_strlcpy.o
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
		./editline_test
		./tokenize_test
		./getopt_test
		./dotw_test
		./fatfs_test

clean:
		rm -f *_test *.img *.o

editline_test:  $(EDITLINE_OBJ)
		$(CC) $(LDFLAGS) $(EDITLINE_OBJ) $(LIBS) -o $@

tokenize_test:  $(TOKENIZE_OBJ)
		$(CC) $(LDFLAGS) $(TOKENIZE_OBJ) $(LIBS) -o $@

getopt_test:    $(GETOPT_OBJ)
		$(CC) $(LDFLAGS) $(GETOPT_OBJ) $(LIBS) -o $@

dotw_test:      $(DOTW_OBJ)
		$(CC) $(LDFLAGS) $(DOTW_OBJ) $(LIBS) -o $@

fatfs_test:     $(FATFS_OBJ)
		$(CC) $(LDFLAGS) $(FATFS_OBJ) $(LIBS) -o $@

###
rpm_editline.o: ../kernel/rpm_editline.c ../include/rpm/api.h
rpm_puts.o: ../kernel/rpm_puts.c ../include/rpm/api.h
rpm_wputs.o: ../kernel/rpm_wputs.c ../include/rpm/api.h
rpm_strwlen.o: ../kernel/rpm_strwlen.c ../include/rpm/api.h
rpm_getwch.o: ../kernel/rpm_getwch.c ../include/rpm/api.h
rpm_getkey.o: ../kernel/rpm_getkey.c ../include/rpm/api.h
rpm_putwch.o: ../kernel/rpm_putwch.c ../include/rpm/api.h
rpm_strlcpy.o: ../kernel/rpm_strlcpy.c ../include/rpm/api.h
tokenize_test.o: tokenize_test.c /usr/local/include/cmocka.h ../include/rpm/api.h
rpm_tokenize.o: ../kernel/rpm_tokenize.c ../include/rpm/api.h
getopt_test.o: getopt_test.c /usr/local/include/cmocka.h ../include/rpm/api.h ../include/rpm/getopt.h
rpm_getopt.o: ../kernel/rpm_getopt.c ../include/rpm/api.h ../include/rpm/getopt.h
dotw_test.o: dotw_test.c /usr/local/include/cmocka.h ../include/rpm/api.h
rpm_get_dotw.o: ../kernel/rpm_get_dotw.c ../include/rpm/api.h
fatfs_test.o: fatfs_test.c /usr/local/include/cmocka.h ../include/rpm/fs.h ../include/rpm/diskio.h
fatfs.o: ../fatfs/fatfs.c ../include/rpm/diskio.h ../include/rpm/api.h ../fatfs/fatfs.h ../include/rpm/fs.h ../fatfs/ffconf.h
unicode.o: ../fatfs/unicode.c ../fatfs/fatfs.h ../include/rpm/fs.h ../fatfs/ffconf.h
