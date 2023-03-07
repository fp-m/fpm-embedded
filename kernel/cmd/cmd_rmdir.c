//
// Remove a directory
//
#include <rpm/api.h>
#include <rpm/getopt.h>
#include <rpm/internal.h>

void rpm_cmd_rmdir(int argc, char *argv[])
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
                     "    rmdir directory ...\r\n"
                     "\n");
            return;
        }
    }

    //TODO: implement rmdir
    rpm_puts("Not implemented yet.\r\n\n");
}
