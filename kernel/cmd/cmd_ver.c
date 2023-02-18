//
// Show the version of RP/M software
//
#include <rpm/api.h>
#include <rpm/getopt.h>
#include <rpm/internal.h>

void rpm_cmd_ver(int argc, char *argv[])
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
            rpm_puts("Usage: ver\r\n\n");
            return;
        }
    }

    // Display RP/M version.
    rpm_puts("\r\n");
    rpm_print_version();
    rpm_puts("\r\n");
}
