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
    // Check the argument.
    const char *colon = strchr(path, ':');
    if ((colon && colon[1] != '\0') ||
        (!colon && path[0] != '\0')) {
        rpm_printf("%s: Invalid drive name\r\n\n", path);
        return;
    }

    // Get filesystem label.
    // The filesystem may not be formatted, which is OK.
    char label[32] = "";
    uint32_t serial_number;
    fs_result_t result = f_getlabel(path, label, &serial_number);
    if (result != FR_OK && result != FR_NO_FILESYSTEM) {
        rpm_puts(f_strerror(result));
        rpm_puts("\r\n\n");
        return;
    }

    // Print drive name.
    const char *drive;
    int len;
    if (colon) {
        drive = path;
        len = colon - path;
    } else {
        drive = disk_name[f_drive()];
        len = strlen(drive);
    }
    rpm_printf("      Disk Drive: %.*s\r\n", len, drive);

    //
    // Print disk name, size and unique id.
    //
    disk_info_t info;
    if (disk_identify(f_drive(), &info) == DISK_OK) {
        rpm_printf("      Disk Model: '%s'\r\n", info.product_name);
        rpm_printf("       Disk Size: %u.%u Mbytes\r\n",
                   (unsigned) (info.num_bytes / 1024 / 1024),
                   (unsigned) (info.num_bytes * 10 / 1024 / 1024 % 10));
        rpm_printf("       Unique Id: %08jx\r\n", (intmax_t) info.serial_number);
    }

    //
    // Print filesystem label and serial number.
    //
    if (result == FR_OK) {
        rpm_printf("Filesystem Label: '%s'\r\n", label);
        rpm_printf("   Serial Number: %08x\r\n", serial_number);
    }
    rpm_puts("\r\n");
}

//
// Set filesystem label.
//
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

        case 'l':
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

    // Show current drive.
    print_volume_info("");
}
