//
// Delete a file or set of files
//
#include <rpm/api.h>
#include <rpm/getopt.h>
#include <rpm/internal.h>

void rpm_cmd_remove(int argc, char *argv[])
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
                     "    rm [options] filename ...\r\n"
                     "    erase [options] filename ...\r\n"
                     "Options:\r\n"
                     "    -f      Force removing, do not ask for confirmation\r\n"
                     "    -r      Remove directories and their contents recursively\r\n"
                     "    -v      Verbose: show files as they are removed\r\n"
                     "\n");
            return;
        }
    }

    rpm_puts("Not implemented yet.\r\n\n");
}
