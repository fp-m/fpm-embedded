//
// Emulation of disk I/O functions for Unix demo.
//
#include <rpm/diskio.h>
#include <stdio.h>
#include <string.h>

#define NDISKS 2

static char flash_image[40*1024*1024];
static char sd_image[2*1024*1024];

static const unsigned sector_size[NDISKS] = {
    4096, // Flash memory
    512,  // SD card
};

static const unsigned disk_size[NDISKS] = {
    sizeof(flash_image), // Flash memory
    sizeof(sd_image),    // SD card
};

//
// Get drive status
//
media_status_t disk_status(uint8_t unit)
{
    printf("--- %s(unit = %u)\n", __func__, unit);
    if (unit >= NDISKS)
        return MEDIA_NOINIT;
    return 0;
}

//
// Inidialize the drive
//
media_status_t disk_initialize(uint8_t unit)
{
    printf("--- %s(unit = %u)\n", __func__, unit);
    if (unit >= NDISKS)
        return MEDIA_NOINIT;
    return 0;
}

//
// Read sectors
//
disk_result_t disk_read(uint8_t unit, uint8_t *buf, unsigned sector, unsigned count)
{
    printf("--- %s(unit = %u, sector = %u, count = %u)\n", __func__, unit, sector, count);
    if (unit >= NDISKS || count == 0)
        return DISK_PARERR;

    unsigned offset = sector * sector_size[unit];
    unsigned nbytes = count * sector_size[unit];
    if (offset + nbytes > disk_size[unit])
        return DISK_PARERR;

    char *image = (unit == 0) ? flash_image : sd_image;
    memcpy(buf, &image[offset], nbytes);
    return DISK_OK;
}

//
// Write sectors
//
disk_result_t disk_write(uint8_t unit, const uint8_t *buf, unsigned sector, unsigned count)
{
    printf("--- %s(unit = %u, sector = %u, count = %u)\n", __func__, unit, sector, count);
    if (unit >= NDISKS || count == 0)
        return DISK_PARERR;

    unsigned offset = sector * sector_size[unit];
    unsigned nbytes = count * sector_size[unit];
    if (offset + nbytes > disk_size[unit])
        return DISK_PARERR;

    char *image = (unit == 0) ? flash_image : sd_image;
    memcpy(&image[offset], buf, nbytes);
    return DISK_OK;
}

//
// Miscellaneous functions
//
disk_result_t disk_ioctl(uint8_t unit, uint8_t cmd, void *buf)
{
    if (unit >= NDISKS)
        return DISK_PARERR;

    switch (cmd) {
    case GET_SECTOR_COUNT: {
        printf("--- %s(unit = %u, cmd = GET_SECTOR_COUNT)\n", __func__, unit);
        *(uint32_t *)buf = disk_size[unit] / sector_size[unit];
        return DISK_OK;
    }
    case GET_SECTOR_SIZE:
        printf("--- %s(unit = %u, cmd = GET_SECTOR_SIZE)\n", __func__, unit);
        *(uint16_t *)buf = sector_size[unit];
        return DISK_OK;

    case GET_BLOCK_SIZE:
        printf("--- %s(unit = %u, cmd = GET_BLOCK_SIZE)\n", __func__, unit);
        *(uint32_t *)buf = 1;
        return DISK_OK;

    case CTRL_SYNC:
        printf("--- %s(unit = %u, cmd = CTRL_SYNC)\n", __func__, unit);
        return DISK_OK;

    default:
        printf("--- %s(unit = %u, cmd = %u)\n", __func__, unit, cmd);
        return DISK_PARERR;
    }
}
