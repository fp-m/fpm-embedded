TOP             = ../..
CFLAGS          = -g -Wall -I$(TOP)/include
OBJ             = main_unix.o \
                  $(TOP)/kernel/rpm_shell.o \
                  $(TOP)/kernel/rpm_puts.o \
                  $(TOP)/kernel/rpm_wputs.o \
                  $(TOP)/kernel/rpm_strwlen.o \
                  $(TOP)/kernel/rpm_getwch.o \
                  $(TOP)/kernel/rpm_getkey.o \
                  $(TOP)/kernel/rpm_putwch.o \
                  $(TOP)/kernel/rpm_strlcpy.o \
                  $(TOP)/kernel/rpm_unix.o \
                  $(TOP)/kernel/rpm_tokenize.o \
                  $(TOP)/kernel/rpm_editline.o

all:            shell

clean:
		rm -f *.o $(TOP)/kernel/*.o shell

shell:          $(OBJ)
		$(CC) $(LDFLAGS) $(OBJ) $(LIBS) -o $@
