//
// Parse a command line and split it into tokens (in place).
// Return NULL on success.
// Fill an argument vector.
// On error, return a message.
//
#include <rpm/api.h>

const char *rpm_tokenize(char *argv[], int *argc, char *cmd_line)
{
    //TODO
    argv[0] = cmd_line;
    *argc = 1;
    return 0;
}
