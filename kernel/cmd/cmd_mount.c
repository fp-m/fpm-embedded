//
// Engage removable disk device
//
#include <rpm/api.h>
#include <rpm/getopt.h>
#include <rpm/internal.h>
#include <rpm/fs.h>
#include <rpm/diskio.h>

//
// Convert statfs returned filesystem size into 1-kbyte units.
// Attempts to avoid overflow for large filesystems.
//
static unsigned long blk_to_kbytes(unsigned long num, unsigned bsize)
{
    if (bsize == 0) {
        return num;
    } else if (bsize < 1024) {
        return num / (1024 / bsize);
    } else {
        return num * (bsize / 1024);
    }
}

static void mount(const char *path)
{
    fs_result_t result = f_mount(path);
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

    fs_info_t info;
    fs_result_t result = f_statfs(path, &info);

    rpm_printf("%5s:  ", disk_name[i]);
    if (result != FR_OK) {
        // Drive not mounted.
        rpm_puts("   ---\r\n");
        return;
    }

    switch (info.f_type) {
    case FS_FAT12: rpm_puts("FAT-12"); break;
    case FS_FAT16: rpm_puts("FAT-16"); break;
    case FS_FAT32: rpm_puts("FAT-32"); break;
    case FS_EXFAT: rpm_puts(" exFAT"); break;
    default:       rpm_puts("Unknwn"); break;
    }

    unsigned used = info.f_blocks - info.f_bfree;
    unsigned avail = info.f_bavail + used;
    rpm_printf(" %10lu", blk_to_kbytes(info.f_blocks, info.f_bsize));
    rpm_printf(" %9lu", blk_to_kbytes(used, info.f_bsize));
    rpm_printf(" %9lu", blk_to_kbytes(info.f_bavail, info.f_bsize));

    if (avail == 0) {
        rpm_puts("   100%");
    } else {
        rpm_printf(" %5u%%", (unsigned) ((used * 200ULL + avail) / avail / 2));
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

    //
    // Show mounted volumes.
    //
    rpm_printf(" Drive    Type  1k-blocks      Used Available Capacity\r\n");
    for (int i = 0; i < DISK_VOLUMES; i++) {
        show(i);
    }
    rpm_puts("\n");
}
