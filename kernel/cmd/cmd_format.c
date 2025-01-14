//
// Create filesystem on a disk device
//
#include <fpm/api.h>
#include <fpm/fs.h>
#include <fpm/diskio.h>
#include <fpm/getopt.h>
#include <fpm/internal.h>

//
// Find disk unit number by name.
//
static int find_drive_unit(const char *drive_name)
{
    const char *path = drive_name;
    int disk_unit = f_getdrive(&path);
    if (disk_unit < 0 || *path != '\0') {
        fpm_printf("%s: Invalid drive name\r\n", drive_name);
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
        //fpm_printf("%s Cannot identify\r\n", drive_name);
        return;
    }

    // Ask user.
    uint16_t reply[32];
    fpm_printf("Do you really want to format drive %s?\r\n", disk_name[disk_unit]);
    fpm_printf("Disk '%s', size %u.%u Mbytes.\r\n", info.product_name,
               (unsigned) (info.num_bytes / 1024 / 1024),
               (unsigned) (info.num_bytes * 10 / 1024 / 1024 % 10));
    fpm_printf("All data on the disk will be lost.\r\n");
    fpm_editline(reply, sizeof(reply), 1, "Confirm? y/n [n] ", 0);
    fpm_puts("\r\n");
    if (reply[0] != 'y' && reply[0] != 'Y') {
        fpm_printf("Not formatted.\r\n");
        return;
    }

    char buf[4*1024];
    fs_result_t result = f_mkfs(drive_name, disk_fmt[disk_unit], buf, sizeof(buf));
    if (result != FR_OK) {
        fpm_printf("%s: %s\r\n", drive_name, f_strerror(result));
        return;
    }

    // Done.
    fpm_printf("Disk %s formatted.\r\n", disk_name[disk_unit]);
}

void fpm_cmd_format(int argc, char *argv[])
{
    static const struct fpm_option long_opts[] = {
        { "help", FPM_NO_ARG, NULL, 'h' },
        {},
    };
    struct fpm_opt opt = {};
    const char *drive_name = 0;

    while (fpm_getopt(argc, argv, "h", long_opts, &opt) >= 0) {
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
            fpm_puts("\r\n");
            return;

        case 'h':
usage:      fpm_puts("Usage:\r\n"
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
    fpm_puts("\r\n");
}
