//
// Show all built-in commands.
//
#include <rpm/api.h>
#include <rpm/internal.h>

void rpm_cmd_help(int argc, char *argv[])
{
    //TODO: getopt
    rpm_puts("RP/M built-in commands are:\r\n");
    rpm_puts("CLEAR or CLS    Clear the console screen\r\n");
    rpm_puts("DATE            Show or change the system date\r\n");
    rpm_puts("ECHO            Copy text directly to the console output\r\n");
    rpm_puts("HELP or ?       Show all built-in commands\r\n");
    rpm_puts("REBOOT          Restart the RP/M kernel\r\n");
    rpm_puts("TIME            Set or show the current system time\r\n");
    rpm_puts("VER             Show the version of RP/M software\r\n");
    rpm_puts("EXIT            Close down the command interpreter\r\n");
    rpm_puts("\r\n");
    rpm_puts("Enter COMMAND -h for further information on any of the above commands.\r\n");
    rpm_puts("\r\n");
}
