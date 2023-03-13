/* glue.c
Copyright 2021 Carl John Kugler III

Licensed under the Apache License, Version 2.0 (the License); you may not use
this file except in compliance with the License. You may obtain a copy of the
License at

   http://www.apache.org/licenses/LICENSE-2.0
Unless required by applicable law or agreed to in writing, software distributed
under the License is distributed on an AS IS BASIS, WITHOUT WARRANTIES OR
CONDITIONS OF ANY KIND, either express or implied. See the License for the
specific language governing permissions and limitations under the License.
*/

/*-----------------------------------------------------------------------*/
/* Low level disk I/O module SKELETON for FatFs     (C)ChaN, 2019        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/
#include <rpm/diskio.h> // Declarations of disk functions
#include <stdio.h>      // For debug printfs
#include "hw_config.h"
#include "sd_card.h"
#include "flash.h"

#define TRACE_PRINTF(fmt, args...)
//#define TRACE_PRINTF printf

//
// Names of disk volumes.
//
const char *disk_name[DISK_VOLUMES] = { "flash", "sd" };

//
// Setup the hardware.
//
void disk_setup()
{
}

//
// Fast check of drive status: whether the media is present or not.
// Argument specifies the physical drive number.
// Drive 0 means Flash memory, drive 1 - SD card.
//
media_status_t disk_status(uint8_t pdrv)
{
    TRACE_PRINTF(">>> %s\n", __FUNCTION__);
    if (pdrv == 0) {
        // Flash memory: always present.
        return 0;
    } else {
        // SD card.
        sd_card_t *sd = sd_get_by_num(0);
        if (!sd)
            return MEDIA_NOINIT;

        // Fast probe: just a GPIO read.
        sd_card_detect(sd);
        return sd->m_Status;
    }
}

//
// Initialize a drive.
//
media_status_t disk_initialize(uint8_t pdrv)
{
    TRACE_PRINTF(">>> %s\n", __FUNCTION__);
    if (pdrv == 0) {
        // Flash memory: always active.
        return 0;
    } else {
        // SD card.
        sd_card_t *sd = sd_get_by_num(0);
        if (!sd)
            return MEDIA_NOINIT;

        return sd_init(sd);
    }
}

static int sdrc2dresult(int sd_rc)
{
    switch (sd_rc) {
    case SD_BLOCK_DEVICE_ERROR_NONE:
        return DISK_OK;

    case SD_BLOCK_DEVICE_ERROR_UNUSABLE:
    case SD_BLOCK_DEVICE_ERROR_NO_RESPONSE:
    case SD_BLOCK_DEVICE_ERROR_NO_INIT:
    case SD_BLOCK_DEVICE_ERROR_NO_DEVICE:
        return DISK_NOTRDY;

    case SD_BLOCK_DEVICE_ERROR_PARAMETER:
    case SD_BLOCK_DEVICE_ERROR_UNSUPPORTED:
        return DISK_PARERR;

    case SD_BLOCK_DEVICE_ERROR_WRITE_PROTECTED:
        return DISK_WRPRT;

    case SD_BLOCK_DEVICE_ERROR_CRC:
    case SD_BLOCK_DEVICE_ERROR_WOULD_BLOCK:
    case SD_BLOCK_DEVICE_ERROR_ERASE:
    case SD_BLOCK_DEVICE_ERROR_WRITE:
    default:
        return DISK_ERROR;
    }
}

/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

disk_result_t disk_read(uint8_t pdrv,    /* Physical drive nmuber to identify the drive */
                        uint8_t *buff,   /* Data buffer to store read data */
                        unsigned sector, /* Start sector in LBA */
                        unsigned count)  /* Number of sectors to read */
{
    TRACE_PRINTF(">>> %s\n", __FUNCTION__);
    if (pdrv == 0) {
        // Flash memory.
        return flash_read(buff, sector, count);
    } else {
        // SD card.
        sd_card_t *sd = sd_get_by_num(0);
        if (!sd)
            return DISK_PARERR;
        int rc = sd_read_blocks(sd, buff, sector, count);
        return sdrc2dresult(rc);
    }
}

/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

#if FF_FS_READONLY == 0

disk_result_t disk_write(uint8_t pdrv,        /* Physical drive nmuber to identify the drive */
                   const uint8_t *buff, /* Data to be written */
                   unsigned sector,     /* Start sector in LBA */
                   unsigned count)      /* Number of sectors to write */
{
    TRACE_PRINTF(">>> %s\n", __FUNCTION__);
    if (pdrv == 0) {
        // Flash memory.
        return flash_write(buff, sector, count);
    } else {
        // SD card.
        sd_card_t *sd = sd_get_by_num(0);
        if (!sd)
            return DISK_PARERR;
        int rc = sd_write_blocks(sd, buff, sector, count);
        return sdrc2dresult(rc);
    }
}

#endif

/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

disk_result_t disk_ioctl(uint8_t pdrv,  /* Physical drive nmuber (0..) */
                   uint8_t cmd,   /* Control code */
                   void *buff) /* Buffer to send/receive control data */
{
    TRACE_PRINTF(">>> %s\n", __FUNCTION__);
    switch (cmd) {
    case GET_SECTOR_COUNT: {
        //
        // Retrieves number of available sectors, the
        // largest allowable LBA + 1, on the drive
        // into the fs_lba_t variable pointed by buff.
        // This command is used by f_mkfs and f_fdisk
        // function to determine the size of
        // volume/partition to be created. It is
        // required when FF_USE_MKFS == 1.
        //
        unsigned num_sectors;
        if (pdrv == 0) {
            // Flash memory.
            num_sectors = flash_block_count();
        } else {
            // SD card.
            sd_card_t *sd = sd_get_by_num(0);
            if (!sd)
                return DISK_PARERR;
            num_sectors = sd_sectors(sd);
        }
        *(uint32_t *)buff = num_sectors;
        if (num_sectors == 0)
            return DISK_ERROR;
        return DISK_OK;
    }
    case GET_SECTOR_SIZE: {
        //
        // Retrieves the sector size on this media (512, 1024, 2048 or 4096).
        // Always set 512 for most systems, generic memory card and harddisk,
        // but a larger value may be required for on-board flash memory and some
        // type of optical media. This command is used by f_mount() and f_mkfs()
        // functions when FF_MAX_SS is larger than FF_MIN_SS.
        //
        unsigned sector_size;
        if (pdrv == 0) {
            // Flash memory.
            sector_size = flash_block_size();
        } else {
            // SD card.
            sector_size = 512;
        }
        *(uint16_t *)buff = sector_size;
        return DISK_OK;
    }
    case GET_BLOCK_SIZE:
        //
        // Retrieves erase block size of the flash
        // memory media in unit of sector into the uint32_t
        // variable pointed by buff. The allowable value
        // is 1 to 32768 in power of 2. Return 1 if the
        // erase block size is unknown or non flash
        // memory media. This command is used by only
        // f_mkfs function and it attempts to align data
        // area on the erase block boundary. It is
        // required when FF_USE_MKFS == 1.
        //
        *(uint32_t *)buff = 1;
        return DISK_OK;

    case CTRL_SYNC:
        return DISK_OK;

    default:
        return DISK_PARERR;
    }
}

//
// Get info from the disk.
//
disk_result_t disk_identify(uint8_t pdrv, disk_info_t *output)
{
    TRACE_PRINTF(">>> %s\n", __FUNCTION__);
    memset(output, 0, sizeof(*output));
    if (pdrv == 0) {
        // Flash memory.
        flash_identify(output);
        return DISK_OK;
    } else {
        // SD card.
        sd_card_t *sd = sd_get_by_num(0);
        if (!sd)
            return DISK_PARERR;

        // Fast probe: just a GPIO read.
        if (!sd_card_detect(sd))
            return DISK_NOTRDY;

        // Full detection.
        // The media may have been reinserted, so reinitialize.
        sd->m_Status |= (MEDIA_NODISK | MEDIA_NOINIT);
        sd->card_type = 0;
        sd_init(sd);
        if (sd->m_Status & (MEDIA_NOINIT | MEDIA_NODISK))
            return DISK_NOTRDY;

        // Size in bytes.
        output->num_bytes = sd->sectors * 512ULL;

        // Serial number: 32 bits or 64 bits.
        output->serial_number = sd->product_serial_number;

        // Product name.
        char *p = output->product_name;
        *p++ = sd->oem_id[0];
        *p++ = sd->oem_id[1];
        *p++ = ' ';
        *p++ = sd->product_name[0];
        *p++ = sd->product_name[1];
        *p++ = sd->product_name[2];
        *p++ = sd->product_name[3];
        *p++ = sd->product_name[4];
        *p++ = 0;
        return DISK_OK;
    }
}
