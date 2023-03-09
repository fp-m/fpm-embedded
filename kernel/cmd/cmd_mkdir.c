//
// Create a directory
//
#include <rpm/api.h>
#include <rpm/fs.h>
#include <rpm/getopt.h>
#include <rpm/internal.h>

static void create_directory(const char *path)
{
    fs_result_t result = f_mkdir(path);
    if (result != FR_OK) {
        rpm_printf("%s: %s\r\n", path, f_strerror(result));
    }
}

void rpm_cmd_mkdir(int argc, char *argv[])
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
            create_directory(opt.arg);
            argcount++;
            break;

        case '?':
            // Unknown option: message already printed.
            rpm_puts("\r\n");
            return;

        case 'h':
usage:      rpm_puts("Usage:\r\n"
                     "    mkdir name ...\r\n"
                     "\n");
            return;
        }
    }

    if (argcount == 0) {
        // Nothing to create.
        goto usage;
    }
    rpm_puts("\r\n");
}
