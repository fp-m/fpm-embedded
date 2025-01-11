//
// Release removable disk device
//
#include <fpm/api.h>
#include <fpm/getopt.h>
#include <fpm/internal.h>
#include <fpm/fs.h>

static void eject(const char *path)
{
    fs_result_t result = f_unmount(path);
    if (result != FR_OK) {
        fpm_puts(f_strerror(result));
        fpm_puts("\r\n\n");
    }
}

void fpm_cmd_eject(int argc, char *argv[])
{
    static const struct fpm_option long_opts[] = {
        { "help", FPM_NO_ARG, NULL, 'h' },
        {},
    };
    struct fpm_opt opt = {};

    if (argc > 2) {
usage:
        fpm_puts("Usage:\r\n"
                     "    eject [sd: | flash:]\r\n"
                 "\n");
        return;
    }
    while (fpm_getopt(argc, argv, "h", long_opts, &opt) >= 0) {
        switch (opt.ret) {
        case 1:
            eject(opt.arg);
            return;

        case '?':
            // Unknown option: message already printed.
            fpm_puts("\r\n");
            return;

        case 'h':
            goto usage;
        }
    }

    // Eject SD card by default.
    eject("sd:");
}
