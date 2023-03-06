#include <gtest/gtest.h>
#include <rpm/fs.h>
#include <rpm/diskio.h>
#include <alloca.h>
#include <string.h>
#include "util.h"

//
// Names of disk volumes.
//
const char *disk_name[DISK_VOLUMES] = { "flash", "sd" };

//
// Disk storage.
//
static char flash_image[2*1024*1024]; // Flash memory - 2 Mbytes
static char sd_image[40*1024*1024];   // SD card - 40 Mbytes

static char *fs_image[DISK_VOLUMES] = {
    flash_image,
    sd_image,
};

//
// Disk metrics.
//
static const unsigned sector_size[DISK_VOLUMES] = {
    4096, // Flash memory - 4 kbytes
    512,  // SD card - half a kbyte
};

static const unsigned disk_size[DISK_VOLUMES] = {
    sizeof(flash_image),
    sizeof(sd_image),
};

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

    memcpy(buf, fs_image[unit] + offset, nbytes);
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

    memcpy(fs_image[unit] + offset, buf, nbytes);
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
        //printf("--- %s(unit = %u, cmd = %u)\r\n", __func__, unit, cmd);
        return DISK_PARERR;
    }
}

extern "C" {

//
// Get date and time (local).
//
void rpm_get_datetime(int *year, int *month, int *day, int *dotw, int *hour, int *min, int *sec)
{
    *year = 2023;
    *month = 2;
    *day = 18;
    *dotw = 6; // Sunday is 0
    *hour = 15;
    *min = 33;
    *sec = 45;
}

};

//
// Create filesystem.
//
static void create_filesystem(const char *drive_name, unsigned format)
{
    char buf[4*1024];
    fs_result_t result = f_mkfs(drive_name, format, buf, sizeof(buf));
    ASSERT_EQ(result, FR_OK);
}

//
// Initialize Flash and SD drives.
//
void disk_setup()
{
    create_filesystem("flash:", FM_FAT | FM_SFD);
    create_filesystem("sd:", FM_FAT32);

    // Mount flash disk.
    fs_result_t result = f_mount("flash:");
    ASSERT_EQ(result, FR_OK);

    // Mount SD card.
    result = f_mount("sd:");
    ASSERT_EQ(result, FR_OK);
}

//
// Create a file with given name and contents.
//
void write_file(const char *filename, const char *contents)
{
    // Create file.
    auto fp = (file_t*) alloca(f_sizeof_file_t());
    auto result = f_open(fp, filename, FA_WRITE | FA_CREATE_ALWAYS);
    ASSERT_EQ(result, FR_OK) << filename;

    // Write data.
    unsigned nbytes = strlen(contents);
    unsigned written = 0;
    result = f_write(fp, contents, nbytes, &written);
    ASSERT_EQ(nbytes, written) << filename;

    // Close the file.
    result = f_close(fp);
    ASSERT_EQ(result, FR_OK) << filename;
}

//
// Check contents of the file.
//
void read_file(const char *filename, const char *contents)
{
    // Open file.
    auto fp = (file_t*) alloca(f_sizeof_file_t());
    auto result = f_open(fp, filename, FA_READ);
    ASSERT_EQ(result, FR_OK) << filename;

    // Read data.
    char buf[128] = {};
    unsigned nbytes_read = 0;
    unsigned nbytes_expected = strlen(contents);
    result = f_read(fp, buf, sizeof(buf), &nbytes_read);
    ASSERT_EQ(nbytes_read, nbytes_expected) << filename;
    ASSERT_STREQ(buf, contents) << filename;

    // Close the file.
    result = f_close(fp);
    ASSERT_EQ(result, FR_OK) << filename;
}
