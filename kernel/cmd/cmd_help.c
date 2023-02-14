//
// Show all built-in commands.
//
#include <rpm/api.h>
#include <rpm/internal.h>

void rpm_cmd_help(int argc, char *argv[])
{
    static const struct rpm_option long_opts[] = {
        { "help", RPM_NO_ARG, NULL, 'h' },
        {},
    };
    struct rpm_opt opt = {};

int result;
    while ((result = rpm_getopt(argc, argv, "h", long_opts, &opt)) >= 0) {
rpm_printf("result = %d\r\n", result);
        switch (opt.opt) {
        case 'h':
            rpm_puts("Usage: help\r\n");
            return;
        case 1:
            rpm_printf("help: unexpected argument `%s`\r\n", opt.arg);
            return;
        case '?':
            return;
        }
    }

    // TODO: getopt
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
