CFLAGS          = -g -Wall -I..
OBJ             = main_unix.o \
                  shell.o \
                  ../rpm/rpm_unix.o \
                  ../rpm/rpm_editline.o

all:            shell

clean:
		rm -f *.o ../rpm/*.o shell

shell:          $(OBJ)
		$(CC) $(LDFLAGS) $(OBJ) $(LIBS) -o $@
