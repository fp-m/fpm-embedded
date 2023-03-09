//
// Remove a directory
//
#include <rpm/api.h>
#include <rpm/fs.h>
#include <rpm/getopt.h>
#include <rpm/internal.h>

static void remove_directory(const char *path)
{
    file_info_t info;
    fs_result_t result = f_stat(path, &info);
    if (result != FR_OK) {
        rpm_printf("%s: %s\r\n", path, f_strerror(result));
        return;
    }
    if (!(info.fattrib & AM_DIR)) {
        rpm_printf("%s: Not a directory\r\n", path);
        return;
    }
    result = f_unlink(path);
    if (result != FR_OK) {
        rpm_printf("%s: %s\r\n", path, f_strerror(result));
    }
}

void rpm_cmd_rmdir(int argc, char *argv[])
{
    static const struct rpm_option long_opts[] = {
        { "help", RPM_NO_ARG, NULL, 'h' },
        {},
    };
    struct rpm_opt opt = {};
    unsigned argcount = 0;

    while (rpm_getopt(argc, argv, "h", long_opts, &opt) >= 0) {
        switch (opt.ret) {
        case 1:
            remove_directory(opt.arg);
            argcount++;
            break;

        case '?':
            // Unknown option: message already printed.
            rpm_puts("\r\n");
            return;

        case 'h':
usage:      rpm_puts("Usage:\r\n"
                     "    rmdir directory ...\r\n"
                     "\n");
            return;
        }
    }

    if (argcount == 0) {
        // Nothing to remove.
        goto usage;
    }
    rpm_puts("\r\n");
}
