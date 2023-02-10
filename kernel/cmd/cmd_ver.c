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
    //TODO: DOSBox-X version 2022.12.26 (SDL2)
    //TODO: DOSBox-X Git commit 1234567, built on Dec 26, 2022 6:27:42pm
}
