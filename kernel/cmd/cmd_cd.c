//
// Show or change current directory
//
#include <rpm/api.h>
#include <rpm/getopt.h>
#include <rpm/internal.h>
#include <rpm/fs.h>

static void chdir(const char *path)
{
    fs_result_t result = f_chdir(path);
    if (result != FR_OK) {
        rpm_printf("%s: %s\r\n\n", path, f_strerror(result));
    }
}

void rpm_cmd_cd(int argc, char *argv[])
{
    static const struct rpm_option long_opts[] = {
        { "help", RPM_NO_ARG, NULL, 'h' },
        {},
    };
    struct rpm_opt opt = {};

    while (rpm_getopt(argc, argv, "h", long_opts, &opt) >= 0) {
        switch (opt.ret) {
        case 1:
            chdir(opt.arg);
            return;

        case '?':
            // Unknown option: message already printed.
            rpm_puts("\r\n");
            return;

        case 'h':
            rpm_puts("Usage:\r\n"
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
        rpm_puts(path);
    } else {
        rpm_puts(f_strerror(result));
    }
    rpm_puts("\r\n\n");
}
