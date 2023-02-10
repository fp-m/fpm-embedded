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
