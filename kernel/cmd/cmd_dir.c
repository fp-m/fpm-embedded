//
// Clear the console screen
//
#include <rpm/api.h>
#include <rpm/getopt.h>
#include <rpm/internal.h>

void rpm_cmd_dir(int argc, char *argv[])
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
            rpm_puts("Usage:\r\n"
                     "    ls [options] [file ...]\r\n"
                     "    dir [options] [file ...]\r\n"
                     "Options:\n"
                     "    -a      Include hidden files and directories\r\n"
                     "    -l      List files in the long format\r\n"
                     "    -R      Recursively list subdirectories encountered\r\n"
                     "    -r      Reverse the order of the sort\r\n"
                     "    -1      Force output to be one entry per line\r\n"
                     "\n");
            return;
        }
    }

    rpm_puts("Not implemented yet.\r\n\n");
}
