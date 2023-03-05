//
// Release removable disk device
//
#include <rpm/api.h>
#include <rpm/getopt.h>
#include <rpm/internal.h>
#include <rpm/fs.h>

static void eject(const char *path)
{
    fs_result_t result = f_unmount(path);
    if (result != FR_OK) {
        rpm_puts(f_strerror(result));
        rpm_puts("\r\n\n");
    }
}

void rpm_cmd_eject(int argc, char *argv[])
{
    static const struct rpm_option long_opts[] = {
        { "help", RPM_NO_ARG, NULL, 'h' },
        {},
    };
    struct rpm_opt opt = {};

    if (argc > 2) {
usage:
        rpm_puts("Usage:\r\n"
                     "    eject [sd: | flash:]\r\n"
                 "\n");
        return;
    }
    while (rpm_getopt(argc, argv, "h", long_opts, &opt) >= 0) {
        switch (opt.ret) {
        case 1:
            eject(opt.arg);
            return;

        case '?':
            // Unknown option: message already printed.
            rpm_puts("\r\n");
            return;

        case 'h':
            goto usage;
        }
    }

    // Eject SD card by default.
    eject("sd:");
}
