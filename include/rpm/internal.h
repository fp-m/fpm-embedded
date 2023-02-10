#include <setjmp.h>

#define RPM_VERSION "0.0"

//
// Interactive shell.
//
void rpm_shell(void);

//
// Resume on ^C.
//
extern jmp_buf rpm_saved_point; // TODO: move to the system area

//
// Shell commands.
//
void rpm_cmd_clear(int argc, char *argv[]);
void rpm_cmd_date(int argc, char *argv[]);
void rpm_cmd_echo(int argc, char *argv[]);
void rpm_cmd_help(int argc, char *argv[]);
void rpm_cmd_reboot(int argc, char *argv[]);
void rpm_cmd_time(int argc, char *argv[]);
void rpm_cmd_ver(int argc, char *argv[]);
