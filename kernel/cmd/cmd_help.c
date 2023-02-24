//
// Show all built-in commands.
//
#include <rpm/api.h>
#include <rpm/getopt.h>
#include <rpm/internal.h>

void rpm_cmd_help(int argc, char *argv[])
{
    static const struct rpm_option long_opts[] = {
        { "help", RPM_NO_ARG, NULL, 'h' },
        {},
    };
    struct rpm_opt opt = {};

    while (rpm_getopt(argc, argv, "h", long_opts, &opt) >= 0) {
        switch (opt.ret) {
        case 1:
            rpm_printf("%s: Unexpected argument `%s`\r\n\n", argv[0], opt.arg);
            return;
        case '?':
            // Unknown option: message already printed.
            rpm_puts("\r\n");
            return;
        case 'h':
            rpm_puts("Usage: help\r\n\n");
            return;
        }
    }

    rpm_puts("RP/M built-in commands are:\r\n");
    rpm_puts("CAT or TYPE     Display the contents of a text file\r\n");
    rpm_puts("CD              Show or change current directory\r\n");
    rpm_puts("CLEAR or CLS    Clear the console screen\r\n");
    rpm_puts("CP or COPY      Copy files or directories\r\n");
    rpm_puts("DATE            Show or change the system date\r\n");
    rpm_puts("ECHO            Copy text directly to the console output\r\n");
    rpm_puts("EJECT           Release removable disk device\r\n");
    rpm_puts("FORMAT          Create filesystem on a disk device\r\n");
    rpm_puts("HELP or ?       Show all built-in commands\r\n");
    rpm_puts("LS or DIR       List the contents of a directory\r\n");
    rpm_puts("MKDIR           Create a directory\r\n");
    rpm_puts("MORE            Display output in pages\r\n");
    rpm_puts("MOUNT           Engage removable disk device\r\n");
    rpm_puts("MV or RENAME    Rename a file\r\n");
    rpm_puts("REBOOT          Restart the RP/M kernel\r\n");
    rpm_puts("RM or ERASE     Delete a file or set of files\r\n");
    rpm_puts("RMDIR           Remove a directory\r\n");
    rpm_puts("TIME            Set or show the current system time\r\n");
    rpm_puts("VER             Show the version of RP/M software\r\n");
    rpm_puts("VOL             Show the volume label of a disk device\r\n");
    rpm_puts("EXIT            Close down the command interpreter\r\n");
    rpm_puts("\r\n");
    rpm_puts("Enter COMMAND -h for further information on any of the above commands.\r\n");
    rpm_puts("\r\n");
}
