//
// Create filesystem on a disk device
//
#include <rpm/api.h>
#include <rpm/fs.h>
#include <rpm/diskio.h>
#include <rpm/getopt.h>
#include <rpm/internal.h>

//
// Find disk unit number by name.
//
static int find_drive_unit(const char *drive_name)
{
    const char *path = drive_name;
    int disk_unit = f_getdrive(&path);
    if (disk_unit < 0 || *path != '\0') {
        rpm_printf("%s: Invalid drive name\r\n", drive_name);
        return -1;
    }
    return disk_unit;
}

static void format_drive(const char *drive_name)
{
    static unsigned const disk_fmt[DISK_VOLUMES] = {
        FM_FAT | FM_SFD, // Drive 0 - Flash memory - FAT12/16 non-partitioned
        FM_FAT32,        // Drive 1 - SD card - FAT32 with partition table
    };

    // Find disk number by name.
    int disk_unit = find_drive_unit(drive_name);
    if (disk_unit < 0) {
        return;
    }

    // Get disk size.
    disk_info_t info;
    if (disk_identify(disk_unit, &info) != DISK_OK) {
        rpm_printf("%s: Cannot identify\r\n", drive_name);
        return;
    }

    // Ask user.
    uint16_t reply[32];
    rpm_printf("Do you really want to format drive %s?\r\n", disk_name[disk_unit]);
    rpm_printf("Disk '%s', size %u.%u Mbytes.\r\n", info.product_name,
               (unsigned) (info.num_bytes / 1024 / 1024),
               (unsigned) (info.num_bytes * 10 / 1024 / 1024 % 10));
    rpm_printf("All data on the disk will be lost.\r\n");
    rpm_editline(reply, sizeof(reply), 1, "Confirm? y/n [n] ", 0);
    rpm_puts("\r\n");
    if (reply[0] != 'y' && reply[0] != 'Y') {
        rpm_printf("Not formatted.\r\n");
        return;
    }

    char buf[4*1024];
    fs_result_t result = f_mkfs(drive_name, disk_fmt[disk_unit], buf, sizeof(buf));
    if (result != FR_OK) {
        rpm_printf("%s: %s\r\n", drive_name, f_strerror(result));
        return;
    }

    // Done.
    rpm_printf("Disk %s formatted.\r\n", disk_name[disk_unit]);
}

void rpm_cmd_format(int argc, char *argv[])
{
    static const struct rpm_option long_opts[] = {
        { "help", RPM_NO_ARG, NULL, 'h' },
        {},
    };
    struct rpm_opt opt = {};
    const char *drive_name = 0;

    while (rpm_getopt(argc, argv, "h", long_opts, &opt) >= 0) {
        switch (opt.ret) {
        case 1:
            if (drive_name != 0) {
                // To many arguments.
                goto usage;
            }
            drive_name = opt.arg;
            break;

        case '?':
            // Unknown option: message already printed.
            rpm_puts("\r\n");
            return;

        case 'h':
usage:      rpm_puts("Usage:\r\n"
                     "    format sd:\r\n"
                     "    format flash:\r\n"
                     "\n");
            return;
        }
    }

    if (drive_name == 0) {
        // No arguments.
        goto usage;
    }
    format_drive(drive_name);
    rpm_puts("\r\n");
}
