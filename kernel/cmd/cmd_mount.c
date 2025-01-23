//
// Engage removable disk device
//
#include <fpm/api.h>
#include <fpm/getopt.h>
#include <fpm/internal.h>
#include <fpm/fs.h>
#include <fpm/diskio.h>

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
        fpm_puts(f_strerror(result));
        fpm_puts("\r\n\n");
        return;
    }

    // Print media info.
    const char *filename = path;
    int drive = f_getdrive(&filename);
    if (drive < 0) {
        return;
    }
    disk_info_t disk_info;
    if (disk_identify(drive, &disk_info) != DISK_OK) {
        return;
    }
    fpm_printf("Media '%s' mounted as %s", disk_info.product_name, disk_name[drive]);

    fs_info_t fs_info;
    if (f_statfs(path, &fs_info) != FR_OK) {
        fpm_puts("\r\n\n");
        return;
    }
    fpm_printf(": %lu kbytes used, %lu free\r\n\n",
        blk_to_kbytes(fs_info.f_blocks - fs_info.f_bfree, fs_info.f_bsize),
        blk_to_kbytes(fs_info.f_bavail, fs_info.f_bsize));
}

static void show(int i)
{
    char path[32];
    strcpy(path, disk_name[i]);
    strcat(path, ":");

    fs_info_t info;
    fs_result_t result = f_statfs(path, &info);

    fpm_printf("%5s:  ", disk_name[i]);
    if (result != FR_OK) {
        // Drive not mounted.
        fpm_puts("   ---\r\n");
        return;
    }

    switch (info.f_type) {
    case FS_FAT12: fpm_puts("FAT-12"); break;
    case FS_FAT16: fpm_puts("FAT-16"); break;
    case FS_FAT32: fpm_puts("FAT-32"); break;
    case FS_EXFAT: fpm_puts(" exFAT"); break;
    default:       fpm_puts("Unknwn"); break;
    }

    unsigned used = info.f_blocks - info.f_bfree;
    unsigned avail = info.f_bavail + used;
    fpm_printf(" %10lu", blk_to_kbytes(info.f_blocks, info.f_bsize));
    fpm_printf(" %9lu", blk_to_kbytes(used, info.f_bsize));
    fpm_printf(" %9lu", blk_to_kbytes(info.f_bavail, info.f_bsize));

    if (avail == 0) {
        fpm_puts("   100%");
    } else {
        fpm_printf(" %5u%%", (unsigned) ((used * 200ULL + avail) / avail / 2));
    }
    fpm_puts("\r\n");
}

void fpm_cmd_mount(int argc, char *argv[])
{
    static const struct fpm_option long_opts[] = {
        { "help", FPM_NO_ARG, NULL, 'h' },
        {},
    };
    struct fpm_opt opt = {};

    if (argc > 2) {
usage:
        fpm_puts("Usage:\r\n"
                 "    mount [sd: | flash:]\r\n"
                 "\n");
        return;
    }

    while (fpm_getopt(argc, argv, "h", long_opts, &opt) >= 0) {
        switch (opt.ret) {
        case 1:
            mount(opt.arg);
            return;

        case '?':
            // Unknown option: message already printed.
            fpm_puts("\r\n");
            return;

        case 'h':
            goto usage;
        }
    }

    //
    // Show mounted volumes.
    //
    fpm_printf(" Drive    Type  1k-blocks      Used Available Capacity\r\n");
    for (int i = 0; i < DISK_VOLUMES; i++) {
        show(i);
    }
    fpm_puts("\n");
}
