//
// Show all built-in commands.
//
#include <fpm/api.h>
#include <fpm/getopt.h>
#include <fpm/internal.h>

void fpm_cmd_help(int argc, char *argv[])
{
    static const struct fpm_option long_opts[] = {
        { "help", FPM_NO_ARG, NULL, 'h' },
        {},
    };
    struct fpm_opt opt = {};

    while (fpm_getopt(argc, argv, "h", long_opts, &opt) >= 0) {
        switch (opt.ret) {
        case 1:
            fpm_printf("%s: Unexpected argument `%s`\r\n\n", argv[0], opt.arg);
            return;
        case '?':
            // Unknown option: message already printed.
            fpm_puts("\r\n");
            return;
        case 'h':
            fpm_puts("Usage: help\r\n\n");
            return;
        }
    }

    fpm_puts("FP/M built-in commands are:\r\n");
    fpm_puts("cat or type     Display the contents of a text file\r\n");
    fpm_puts("cd              Show or change current directory\r\n");
    fpm_puts("clear or cls    Clear the console screen\r\n");
    fpm_puts("cp or copy      Copy files or directories\r\n");
    fpm_puts("date            Show or change the system date\r\n");
    fpm_puts("echo            Copy text directly to the console output\r\n");
    fpm_puts("eject           Release removable disk device\r\n");
    fpm_puts("format          Create filesystem on a disk device\r\n");
    fpm_puts("help or ?       Show all built-in commands\r\n");
    fpm_puts("ls or dir       List the contents of a directory\r\n");
    fpm_puts("mkdir           Create a directory\r\n");
    fpm_puts("mount           Engage removable disk device\r\n");
    fpm_puts("mv or rename    Rename or move files and directories\r\n");
    fpm_puts("reboot          Restart the FP/M kernel\r\n");
    fpm_puts("rm or erase     Delete a file or set of files\r\n");
    fpm_puts("rmdir           Remove a directory\r\n");
    fpm_puts("time            Set or show the current system time\r\n");
    fpm_puts("ver             Show the version of FP/M software\r\n");
    fpm_puts("vol             Show the volume label of a disk device\r\n");
    fpm_puts("exit            Close down the command interpreter\r\n");
    fpm_puts("\r\n");
    fpm_puts("Enter 'command -h' for more information on any of the above commands.\r\n");
    fpm_puts("\r\n");
}
