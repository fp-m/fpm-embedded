#include <iostream>
#include <fpm/diskio.h>
#include <sys/time.h>
#include "extern.h"

//
// Size of filesystem in bytes.
//
unsigned fs_nbytes;

//
// Collection of sectors, ordered by sector number.
//
std::map<unsigned, SectorData> fs_image;

//
// Disk IO data and routines for FatFS.
//
const char *disk_name[DISK_VOLUMES] = { "flash", "sd" };

media_status_t disk_status(uint8_t unit)
{
    return 0;
}

media_status_t disk_initialize(uint8_t unit)
{
    return 0;
}

disk_result_t disk_ioctl(uint8_t unit, uint8_t cmd, void *buf)
{
    switch (cmd) {
    case GET_BLOCK_SIZE:
        // Get erase block size.
        *(uint32_t *)buf = 1;
        return DISK_OK;
    case GET_SECTOR_SIZE:
        *(uint16_t *)buf = SECTOR_SIZE;
        return DISK_OK;
    case GET_SECTOR_COUNT:
        // Get media size.
        *(uint32_t *)buf = fs_nbytes / SECTOR_SIZE;
        return DISK_OK;
    case CTRL_SYNC:
        // Complete pending write process.
        return DISK_OK;
    default:
        return DISK_PARERR;
    }
}

disk_result_t disk_read(uint8_t unit, uint8_t *buf, unsigned sector, unsigned count)
{
    std::cout << "--- read sector " << sector << "\n";
    if (count == 0)
        throw std::runtime_error("Zero count in disk_read()");
    if (sector + count > fs_nbytes / SECTOR_SIZE)
        throw std::runtime_error("Too large count in disk_read()");

    for (; count-- > 0; buf += SECTOR_SIZE, sector++) {
        // Throws if sector not found.
        auto &data = fs_image.at(sector);
        memcpy(buf, &data, SECTOR_SIZE);
    }
    return DISK_OK;
}

disk_result_t disk_write(uint8_t unit, const uint8_t *buf, unsigned sector, unsigned count)
{
    std::cout << "--- write sector " << sector << "\n";
    if (count == 0)
        throw std::runtime_error("Zero count in disk_write()");
    if (sector + count > fs_nbytes / SECTOR_SIZE)
        throw std::runtime_error("Too large count in disk_write()");

    for (; count-- > 0; buf += SECTOR_SIZE, sector++) {
        SectorData data;
        memcpy(&data, buf, SECTOR_SIZE);
        fs_image.insert_or_assign(sector, data);
    }
    return DISK_OK;
}

//
// Get date and time (local).
//
extern "C" {
void fpm_get_datetime(int *year, int *month, int *day, int *dotw, int *hour, int *min, int *sec)
{
    struct timeval tv;
    gettimeofday(&tv, 0);

    time_t now = tv.tv_sec;
    struct tm *info = localtime(&now);

    *year = 1900 + info->tm_year;
    *month = 1 + info->tm_mon;
    *day = info->tm_mday;
    *dotw = info->tm_wday;
    *hour = info->tm_hour;
    *min = info->tm_min;
    *sec = info->tm_sec;
}
};
