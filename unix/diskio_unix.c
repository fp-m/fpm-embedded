//
// Emulation of disk I/O functions for Unix demo.
//
#include <rpm/fs.h>
#include <rpm/diskio.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/errno.h>

//
// Names of disk volumes.
//
const char *disk_name[DISK_VOLUMES] = { "flash", "sd" };

//
// Disk metrics.
//
static const unsigned sector_size[DISK_VOLUMES] = {
    4096, // Flash memory - 4 kbytes
    512,  // SD card - half a kbyte
};

static const unsigned disk_size[DISK_VOLUMES] = {
    2*1024*1024,  // Flash memory - 2 Mbytes
    40*1024*1024, // SD card - 40 Mbytes
};

static int disk_fd[DISK_VOLUMES] = { -1, -1 };

//
// Open disk image.
// On first run, create file with required size.
//
static void open_disk_image(unsigned unit, const char *dirname, const char *filename)
{
    char path[4096];
    strcpy(path, dirname);
    strcat(path, "/");
    strcat(path, filename);

    disk_fd[unit] = open(path, O_RDWR);
    if (disk_fd[unit] < 0) {
        // No such file - create it.
        disk_fd[unit] = open(path, O_CREAT | O_RDWR, 0640);
        if (disk_fd[unit] < 0) {
            printf("%s: Cannot create file, aborted\r\n", path);
            exit(-1);
        }

        // Set file size.
        lseek(disk_fd[unit], disk_size[unit] - 1, SEEK_SET);
        if (write(disk_fd[unit], "", 1) != 1) {
            printf("%s: Write error, aborted\r\n", path);
            exit(-1);
        }
        lseek(disk_fd[unit], 0, SEEK_SET);

        // Create filesystem.
        char buf[4*1024];
        const char *drive_name = (unit == 0) ? "0:" : "1:";
        unsigned fmt = (unit == 0) ? (FM_FAT | FM_SFD) : FM_FAT32;
        fs_result_t result = f_mkfs(drive_name, fmt, buf, sizeof(buf));
        if (result != FR_OK) {
            printf("%s: Cannot create filesystem: %s\r\n", path, f_strerror(result));
        }

        printf("Create file %s - size %u Mbytes\r\n", path, disk_size[unit] / 1024 / 1024);
    }
}

//
// Open files.
// On first run, create files in ~/.rp-m/ directory.
//
void disk_setup()
{
    const char *home = getenv("HOME");
    if (!home) {
        home = "";
    }

    // Create directory ~/.rp-m.
    char path[4096];
    strcpy(path, home);
    strcat(path, "/.rp-m");
    if (mkdir(path, 0750) < 0 && errno != EEXIST) {
        printf("%s: Cannot create directory, aborted\r\n", path);
        exit(-1);
    }

    open_disk_image(0, path, "flash.img");
    open_disk_image(1, path, "sd.img");
}

//
// Get drive status
//
media_status_t disk_status(uint8_t unit)
{
    //printf("--- %s(unit = %u)\r\n", __func__, unit);
    if (unit >= DISK_VOLUMES)
        return MEDIA_NOINIT;
    return 0;
}

//
// Inidialize the drive
//
media_status_t disk_initialize(uint8_t unit)
{
    //printf("--- %s(unit = %u)\r\n", __func__, unit);
    if (unit >= DISK_VOLUMES)
        return MEDIA_NOINIT;
    return 0;
}

//
// Read sectors
//
disk_result_t disk_read(uint8_t unit, uint8_t *buf, unsigned sector, unsigned count)
{
    //printf("--- %s(unit = %u, sector = %u, count = %u)\r\n", __func__, unit, sector, count);
    if (unit >= DISK_VOLUMES || count == 0)
        return DISK_PARERR;

    unsigned offset = sector * sector_size[unit];
    unsigned nbytes = count * sector_size[unit];
    if (offset + nbytes > disk_size[unit])
        return DISK_PARERR;

    int status = lseek(disk_fd[unit], offset, SEEK_SET);
    if (status < 0)
        return DISK_ERROR;

    status = read(disk_fd[unit], buf, nbytes);
    if (status < 0)
        return DISK_ERROR;

    return DISK_OK;
}

//
// Write sectors
//
disk_result_t disk_write(uint8_t unit, const uint8_t *buf, unsigned sector, unsigned count)
{
    //printf("--- %s(unit = %u, sector = %u, count = %u)\r\n", __func__, unit, sector, count);
    if (unit >= DISK_VOLUMES || count == 0)
        return DISK_PARERR;

    unsigned offset = sector * sector_size[unit];
    unsigned nbytes = count * sector_size[unit];
    if (offset + nbytes > disk_size[unit])
        return DISK_PARERR;

    int status = lseek(disk_fd[unit], offset, SEEK_SET);
    if (status < 0)
        return DISK_ERROR;

    status = write(disk_fd[unit], buf, nbytes);
    if (status < 0)
        return DISK_ERROR;

    return DISK_OK;
}

//
// Miscellaneous functions
//
disk_result_t disk_ioctl(uint8_t unit, uint8_t cmd, void *buf)
{
    if (unit >= DISK_VOLUMES)
        return DISK_PARERR;

    switch (cmd) {
    case GET_SECTOR_COUNT: {
        //printf("--- %s(unit = %u, cmd = GET_SECTOR_COUNT)\r\n", __func__, unit);
        *(uint32_t *)buf = disk_size[unit] / sector_size[unit];
        return DISK_OK;
    }
    case GET_SECTOR_SIZE:
        //printf("--- %s(unit = %u, cmd = GET_SECTOR_SIZE)\r\n", __func__, unit);
        *(uint16_t *)buf = sector_size[unit];
        return DISK_OK;

    case GET_BLOCK_SIZE:
        //printf("--- %s(unit = %u, cmd = GET_BLOCK_SIZE)\r\n", __func__, unit);
        *(uint32_t *)buf = 1;
        return DISK_OK;

    case CTRL_SYNC:
        //printf("--- %s(unit = %u, cmd = CTRL_SYNC)\r\n", __func__, unit);
        return DISK_OK;

    default:
        printf("--- %s(unit = %u, cmd = %u)\r\n", __func__, unit, cmd);
        return DISK_PARERR;
    }
}

//
// Get info from the disk.
//
disk_result_t disk_identify(uint8_t unit, disk_info_t *output)
{
    if (unit >= DISK_VOLUMES)
        return DISK_PARERR;

    memset(output, 0, sizeof(*output));

    // Size in bytes.
    output->num_bytes = disk_size[unit];

    // Serial number: 32 bits or 64 bits.
    struct stat st;
    fstat(disk_fd[unit], &st);
    output->serial_number = (st.st_dev << 16) + st.st_ino;

    // Product name.
    strcpy(output->product_name, unit==0 ? "flash.img" : "sd.img");
    return DISK_OK;
}
