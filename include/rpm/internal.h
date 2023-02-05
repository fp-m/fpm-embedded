#include <setjmp.h>

//
// Interactive shell.
//
void rpm_shell(void);

//
// Resume on ^C.
//
extern jmp_buf rpm_saved_point; // TODO: move to the system area
