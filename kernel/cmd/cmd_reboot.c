//
// Restart the FP/M kernel
//
#include <fpm/api.h>
#include <fpm/getopt.h>
#include <fpm/internal.h>

void fpm_cmd_reboot(int argc, char *argv[])
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
            fpm_puts("Usage: reboot\r\n\n");
            return;
        }
    }

    fpm_puts("Reboot....\r\n\r\n");
    fpm_reboot();
}
