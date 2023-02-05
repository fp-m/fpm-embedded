TOP             = ../..
CFLAGS          = -g -Wall -I$(TOP)/include
OBJ             = main_unix.o \
                  $(TOP)/kernel/rpm_shell.o \
                  $(TOP)/kernel/rpm_unix.o \
                  $(TOP)/kernel/rpm_editline.o

all:            shell

clean:
		rm -f *.o $(TOP)/kernel/*.o shell

shell:          $(OBJ)
		$(CC) $(LDFLAGS) $(OBJ) $(LIBS) -o $@
