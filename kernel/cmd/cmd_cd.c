//
// Show or change current directory
//
#include <fpm/api.h>
#include <fpm/getopt.h>
#include <fpm/internal.h>
#include <fpm/fs.h>

static void chdir(const char *path)
{
    fs_result_t result = f_chdir(path);
    if (result != FR_OK) {
        fpm_printf("%s: %s\r\n\n", path, f_strerror(result));
    }
}

void fpm_cmd_cd(int argc, char *argv[])
{
    static const struct fpm_option long_opts[] = {
        { "help", FPM_NO_ARG, NULL, 'h' },
        {},
    };
    struct fpm_opt opt = {};

    while (fpm_getopt(argc, argv, "h", long_opts, &opt) >= 0) {
        switch (opt.ret) {
        case 1:
            chdir(opt.arg);
            return;

        case '?':
            // Unknown option: message already printed.
            fpm_puts("\r\n");
            return;

        case 'h':
            fpm_puts("Usage:\r\n"
                     "    cd\r\n"
                     "    cd path\r\n"
                     "    cd ..\r\n"
                     "\n");
            return;
        }
    }

    char path[4096];
    fs_result_t result = f_getcwd(path, sizeof(path));
    if (result == FR_OK) {
        fpm_puts(path);
    } else {
        fpm_puts(f_strerror(result));
    }
    fpm_puts("\r\n\n");
}
