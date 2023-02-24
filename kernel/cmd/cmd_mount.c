//
// Clear the console screen
//
#include <rpm/api.h>
#include <rpm/getopt.h>
#include <rpm/internal.h>
#include <rpm/fs.h>
#include <rpm/diskio.h>

static void mount(const char *path)
{
    fs_result_t result = f_mount(path, 1);
    if (result != FR_OK) {
        rpm_puts(f_strerror(result));
        rpm_puts("\r\n\n");
    }
}

static void show(int i)
{
    char path[32];
    strcpy(path, disk_name[i]);
    strcat(path, ":");

    fs_size_t free_bytes = 0;
    fs_result_t result = f_getfree(path, &free_bytes);

    rpm_puts(path);
    if (result == FR_OK) {
        rpm_printf(" Available %u kbytes", free_bytes / 1024);
    } else {
        rpm_puts(" ");
        rpm_puts(f_strerror(result));
    }
    rpm_puts("\r\n");
}

void rpm_cmd_mount(int argc, char *argv[])
{
    static const struct rpm_option long_opts[] = {
        { "help", RPM_NO_ARG, NULL, 'h' },
        {},
    };
    struct rpm_opt opt = {};

    if (argc > 2) {
usage:
        rpm_puts("Usage:\r\n"
                 "    mount [sd: | flash:]\r\n"
                 "\n");
        return;
    }

    while (rpm_getopt(argc, argv, "h", long_opts, &opt) >= 0) {
        switch (opt.ret) {
        case 1:
            mount(opt.arg);
            return;

        case '?':
            // Unknown option: message already printed.
            rpm_puts("\r\n");
            return;

        case 'h':
            goto usage;
        }
    }

    // Show mounted volumes.
    for (int i = 0; i < DISK_VOLUMES; i++) {
        extern const char *disk_name[DISK_VOLUMES];
        show(i);
    }
    rpm_puts("\n");
}
