//
// Clear the console screen
//
#include <rpm/api.h>
#include <rpm/internal.h>

void rpm_cmd_clear(int argc, char *argv[])
{
    //TODO: getopt
    rpm_puts("\33[H\33[J");
}
