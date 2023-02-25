//
// Clear the console screen
//
#include <rpm/api.h>
#include <rpm/getopt.h>
#include <rpm/internal.h>
#include <rpm/fs.h>
#include <rpm/diskio.h>

static void print_volume_info(const char *path)
{
    //TODO
    rpm_printf("      Disk Drive: sd:\r\n");
    rpm_printf("      Disk Model: Samsung SDHC\r\n");
    rpm_printf("       Disk Size: 40.0 Mbytes\r\n");
    rpm_printf("   Unique Number: 12345678\r\n");
    rpm_printf("Filesystem Label: mydisk\r\n");
    rpm_printf("   Serial Number: aabbccdd\r\n");
}

static void set_label(const char *label)
{
    fs_result_t result = f_setlabel(label);
    if (result != FR_OK) {
        rpm_puts(f_strerror(result));
        rpm_puts("\r\n\n");
    }
}

void rpm_cmd_vol(int argc, char *argv[])
{
    static const struct rpm_option long_opts[] = {
        { "help", RPM_NO_ARG, NULL, 'h' },
        { "label", RPM_REQUIRED_ARG, NULL, 'l' },
        {},
    };
    struct rpm_opt opt = {};

    while (rpm_getopt(argc, argv, "hl:", long_opts, &opt) >= 0) {
        switch (opt.ret) {
        case 1:
            print_volume_info(opt.arg);
            return;

        case 's':
            set_label(opt.arg);
            return;

        case '?':
            // Unknown option: message already printed.
            rpm_puts("\r\n");
            return;

        case 'h':
            rpm_puts("Usage:\r\n"
                     "    vol [sd: | flash:]\r\n"
                     "    vol -l disk:label\r\n"
                     "Options:\r\n"
                     "    -l, --label   Set the filesystem label\r\n"
                     "\n");
            return;
        }
    }

    const char *drive = disk_name[f_drive()];
    print_volume_info(drive);
}
