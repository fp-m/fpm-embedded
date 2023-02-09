//
// Parse a command line and split it into tokens (in place).
// Return an argument vector.
//
#include <rpm/api.h>

void rpm_tokenize(char *argv[], int *argc, char *cmd_line)
{
    //TODO
    argv[0] = cmd_line;
    *argc = 1;
}
