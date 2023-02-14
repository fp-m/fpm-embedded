TOP             = ..
CFLAGS          = -g -O0 -Wall -I$(TOP)/include
LDFLAGS         = -g
LIBS            = -lcmocka
TESTS           = editline_test tokenize_test getopt_test
EDITLINE_OBJ    = editline_test.o \
                  $(TOP)/kernel/rpm_editline.o \
                  $(TOP)/kernel/rpm_puts.o \
                  $(TOP)/kernel/rpm_wputs.o \
                  $(TOP)/kernel/rpm_strwlen.o \
                  $(TOP)/kernel/rpm_getwch.o \
                  $(TOP)/kernel/rpm_getkey.o \
                  $(TOP)/kernel/rpm_putwch.o \
                  $(TOP)/kernel/rpm_strlcpy.o
TOKENIZE_OBJ      = tokenize_test.o \
                  $(TOP)/kernel/rpm_tokenize.o
GETOPT_OBJ        = getopt_test.o \
                  $(TOP)/kernel/rpm_getopt.o

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

clean:
		rm -f *_test *.o $(TOP)/kernel/*.o

editline_test:  $(EDITLINE_OBJ)
		$(CC) $(LDFLAGS) $(EDITLINE_OBJ) $(LIBS) -o $@

tokenize_test:  $(TOKENIZE_OBJ)
		$(CC) $(LDFLAGS) $(TOKENIZE_OBJ) $(LIBS) -o $@

getopt_test:    $(GETOPT_OBJ)
		$(CC) $(LDFLAGS) $(GETOPT_OBJ) $(LIBS) -o $@

###
editline_test.o: editline_test.c ../include/rpm/api.h ../include/rpm/internal.h
tokenize_test.o: tokenize_test.c ../include/rpm/api.h
../kernel/rpm_editline.o: ../kernel/rpm_editline.c ../include/rpm/api.h
../kernel/rpm_getkey.o: ../kernel/rpm_getkey.c ../include/rpm/api.h
../kernel/rpm_getwch.o: ../kernel/rpm_getwch.c ../include/rpm/api.h
../kernel/rpm_puts.o: ../kernel/rpm_puts.c ../include/rpm/api.h
../kernel/rpm_putwch.o: ../kernel/rpm_putwch.c ../include/rpm/api.h
../kernel/rpm_shell.o: ../kernel/rpm_shell.c ../include/rpm/api.h ../include/rpm/internal.h
../kernel/rpm_strlcpy.o: ../kernel/rpm_strlcpy.c ../include/rpm/api.h
../kernel/rpm_strwlen.o: ../kernel/rpm_strwlen.c ../include/rpm/api.h
../kernel/rpm_tokenize.o: ../kernel/rpm_tokenize.c ../include/rpm/api.h
../kernel/rpm_unix.o: ../kernel/rpm_unix.c ../include/rpm/api.h ../include/rpm/internal.h
../kernel/rpm_wputs.o: ../kernel/rpm_wputs.c ../include/rpm/api.h
