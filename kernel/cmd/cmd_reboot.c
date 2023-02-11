//
// Restart the RP/M kernel
//
#include <rpm/api.h>
#include <rpm/internal.h>

void rpm_cmd_reboot(int argc, char *argv[])
{
    //TODO: getopt
    rpm_puts("Reboot....\r\n\r\n");
    rpm_reboot();
}
