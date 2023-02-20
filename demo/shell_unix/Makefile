PROG            = rpm_shell_unix
TOP             = ../..
CFLAGS          = -g -Wall -Werror -Wmissing-prototypes -I$(TOP)/include
OBJ             = main_unix.o \
                  $(TOP)/kernel/rpm_shell.o \
                  $(TOP)/kernel/rpm_exec.o \
                  $(TOP)/kernel/rpm_puts.o \
                  $(TOP)/kernel/rpm_wputs.o \
                  $(TOP)/kernel/rpm_strwlen.o \
                  $(TOP)/kernel/rpm_getwch.o \
                  $(TOP)/kernel/rpm_getkey.o \
                  $(TOP)/kernel/rpm_putwch.o \
                  $(TOP)/kernel/rpm_strlcpy.o \
                  $(TOP)/kernel/rpm_unix.o \
                  $(TOP)/kernel/rpm_tokenize.o \
                  $(TOP)/kernel/rpm_editline.o \
                  $(TOP)/kernel/rpm_getopt.o \
                  $(TOP)/kernel/cmd/cmd_cat.o \
                  $(TOP)/kernel/cmd/cmd_cd.o \
                  $(TOP)/kernel/cmd/cmd_copy.o \
                  $(TOP)/kernel/cmd/cmd_clear.o \
                  $(TOP)/kernel/cmd/cmd_date.o \
                  $(TOP)/kernel/cmd/cmd_dir.o \
                  $(TOP)/kernel/cmd/cmd_echo.o \
                  $(TOP)/kernel/cmd/cmd_eject.o \
                  $(TOP)/kernel/cmd/cmd_format.o \
                  $(TOP)/kernel/cmd/cmd_help.o \
                  $(TOP)/kernel/cmd/cmd_mkdir.o \
                  $(TOP)/kernel/cmd/cmd_more.o \
                  $(TOP)/kernel/cmd/cmd_mount.o \
                  $(TOP)/kernel/cmd/cmd_pwd.o \
                  $(TOP)/kernel/cmd/cmd_reboot.o \
                  $(TOP)/kernel/cmd/cmd_remove.o \
                  $(TOP)/kernel/cmd/cmd_rename.o \
                  $(TOP)/kernel/cmd/cmd_rmdir.o \
                  $(TOP)/kernel/cmd/cmd_time.o \
                  $(TOP)/kernel/cmd/cmd_ver.o \
                  $(TOP)/kernel/cmd/cmd_vol.o

GIT_REVCOUNT    := $(shell cd $(TOP); git rev-list HEAD --count)
GIT_COMMIT      := $(shell cd $(TOP); git log -1 --format=%h)
CFLAGS          += -DGIT_REVCOUNT=\"$(GIT_REVCOUNT)\" -DGIT_COMMIT=\"$(GIT_COMMIT)\"

all:            $(PROG)

clean:
		rm -f $(PROG) *.o $(TOP)/kernel/*.o $(TOP)/kernel/cmd/*.o

$(PROG):        $(OBJ)
		$(CC) $(LDFLAGS) $(OBJ) $(LIBS) -o $@
