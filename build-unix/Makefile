CFLAGS          = -g -Wall -I../include
OBJ             = main_unix.o \
                  ../kernel/rpm_shell.o \
                  ../kernel/rpm_unix.o \
                  ../kernel/rpm_editline.o

all:            shell

clean:
		rm -f *.o ../kernel/*.o shell

shell:          $(OBJ)
		$(CC) $(LDFLAGS) $(OBJ) $(LIBS) -o $@
