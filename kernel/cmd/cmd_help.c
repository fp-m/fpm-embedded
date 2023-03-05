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
    rpm_puts("cat or type     Display the contents of a text file\r\n");
    rpm_puts("cd              Show or change current directory\r\n");
    rpm_puts("clear or cls    Clear the console screen\r\n");
    rpm_puts("cp or copy      Copy files or directories\r\n");
    rpm_puts("date            Show or change the system date\r\n");
    rpm_puts("echo            Copy text directly to the console output\r\n");
    rpm_puts("eject           Release removable disk device\r\n");
    rpm_puts("format          Create filesystem on a disk device\r\n");
    rpm_puts("help or ?       Show all built-in commands\r\n");
    rpm_puts("ls or dir       List the contents of a directory\r\n");
    rpm_puts("mkdir           Create a directory\r\n");
    rpm_puts("mount           Engage removable disk device\r\n");
    rpm_puts("mv or rename    Rename or move files and directories\r\n");
    rpm_puts("reboot          Restart the RP/M kernel\r\n");
    rpm_puts("rm or erase     Delete a file or set of files\r\n");
    rpm_puts("rmdir           Remove a directory\r\n");
    rpm_puts("time            Set or show the current system time\r\n");
    rpm_puts("ver             Show the version of RP/M software\r\n");
    rpm_puts("vol             Show the volume label of a disk device\r\n");
    rpm_puts("exit            Close down the command interpreter\r\n");
    rpm_puts("\r\n");
    rpm_puts("Enter 'command -h' for more information on any of the above commands.\r\n");
    rpm_puts("\r\n");
}
