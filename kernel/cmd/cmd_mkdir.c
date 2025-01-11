//
// Create a directory
//
#include <fpm/api.h>
#include <fpm/fs.h>
#include <fpm/getopt.h>
#include <fpm/internal.h>

static void create_directory(const char *path)
{
    fs_result_t result = f_mkdir(path);
    if (result != FR_OK) {
        fpm_printf("%s: %s\r\n", path, f_strerror(result));
    }
}

void fpm_cmd_mkdir(int argc, char *argv[])
{
    static const struct fpm_option long_opts[] = {
        { "help", FPM_NO_ARG, NULL, 'h' },
        {},
    };
    struct fpm_opt opt = {};
    unsigned argcount = 0;

    while (fpm_getopt(argc, argv, "h", long_opts, &opt) >= 0) {
        switch (opt.ret) {
        case 1:
            create_directory(opt.arg);
            argcount++;
            break;

        case '?':
            // Unknown option: message already printed.
            fpm_puts("\r\n");
            return;

        case 'h':
usage:      fpm_puts("Usage:\r\n"
                     "    mkdir name ...\r\n"
                     "\n");
            return;
        }
    }

    if (argcount == 0) {
        // Nothing to create.
        goto usage;
    }
    fpm_puts("\r\n");
}
