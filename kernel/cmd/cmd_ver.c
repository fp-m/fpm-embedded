//
// Show the version of RP/M software
//
#include <rpm/api.h>
#include <rpm/internal.h>

void rpm_cmd_ver(int argc, char *argv[])
{
    //TODO: getopt
    rpm_puts("\r\n");
    rpm_print_version();
    rpm_puts("\r\n");
}
