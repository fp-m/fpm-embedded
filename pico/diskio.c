//
// Copyright (c) 2023 Serge Vakulenko
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
#include <fpm/diskio.h> // Declarations of disk functions
#include <stdio.h>      // For debug printfs
#include "sd_card.h"
#include "flash.h"

#if 1
// No trace output
#define TRACE_PRINTF(fmt, args...)
#else
// Enable trace output
#define TRACE_PRINTF printf
#endif

//
// Names of disk volumes.
//
const char *disk_name[DISK_VOLUMES] = { "flash", "sd" };

//
// Hardware Configuration of SPI ports.
// One entry for each supported board.
// Note: multiple SD cards can be driven by one SPI if they use
// different slave selects.
//
static void spi_isr0(void);
static void spi_isr1(void);
static void spi_isr2(void);
static void spi_isr3(void);

static spi_t spi_ports[] = {
    // SparkFun RP2040 Thing Plus board.
    {
        .hw_inst   = spi1,         // Port SPI1
        .miso_gpio = 12,           // DATA 0 - SDO
        .mosi_gpio = 15,           // CMD    - SDI
        .sck_gpio  = 14,           // CLK    - SCK
        .baud_rate = 12500 * 1000, // The limitation here is SPI slew rate
        .dma_isr   = spi_isr0,
    },
    // Challenger RP2040 SD/RTC board.
    {
        .hw_inst   = spi1,         // Port SPI1
        .miso_gpio = 11,           // serial data output SDO
        .mosi_gpio = 12,           // serial data input SDI
        .sck_gpio  = 10,           // serial clock SCK
        .baud_rate = 12500 * 1000, // The limitation here is SPI slew rate
        .dma_isr   = spi_isr1,
    },
    // Waveshare RP2040-PiZero board.
    {
        .hw_inst   = spi0,         // Port SPI0
        .miso_gpio = 20,           // serial data output SDO
        .mosi_gpio = 19,           // serial data input SDI
        .sck_gpio  = 18,           // serial clock SCK
        .baud_rate = 12500 * 1000, // The limitation here is SPI slew rate
        .dma_isr   = spi_isr2,
    },
    // HackyPi board.
    {
        .hw_inst   = spi0,         // Port SPI0
        .miso_gpio = 16,           // serial data output SDO
        .mosi_gpio = 19,           // serial data input SDI
        .sck_gpio  = 18,           // serial clock SCK
        .baud_rate = 12500 * 1000, // The limitation here is SPI slew rate
        .dma_isr   = spi_isr3,
    },
    { 0 }, // Terminate by zero.
};
static void spi_isr0(void) { spi_irq_handler(&spi_ports[0]); }
static void spi_isr1(void) { spi_irq_handler(&spi_ports[1]); }
static void spi_isr2(void) { spi_irq_handler(&spi_ports[2]); }
static void spi_isr3(void) { spi_irq_handler(&spi_ports[3]); }

//
// Hardware Configuration of SD cards.
// One entry for each supported board.
// On first SD card connection, this list will be searched and one
// of these configurations selected, whichever is valid for the current board.
//
static sd_card_t sd_cards[] = {
    {
        .board_name       = "SparkFun Thing Plus RP2040",
        // https://learn.sparkfun.com/tutorials/rp2040-thing-plus-hookup-guide/hardware-overview

        .spi              = &spi_ports[0], // Pointer to the SPI port driving this card
        .ss_gpio          = 9,             // The SPI slave select GPIO for this SD card
        .use_card_detect  = false,         // No card detect GPIO for this card
    },
    {
        .board_name       = "Challenger RP2040 SD/RTC",
        // https://ilabs.se/challenger-rp2040-sd-rtc-datasheet/

        .spi              = &spi_ports[1], // Pointer to the SPI port driving this card
        .ss_gpio          = 9,             // The SPI slave select GPIO for this SD card
        .use_card_detect  = true,          // Card detect GPIO is available
        .card_detect_gpio = 13,            // The card detect GPIO signal
    },
    {
        .board_name       = "Waveshare RP2040-PiZero",

        .spi              = &spi_ports[2], // Pointer to the SPI port driving this card
        .ss_gpio          = 21,            // The SPI slave select GPIO for this SD card
        .use_card_detect  = false,         // Card detect GPIO is available
    },
    {
        .board_name       = "HackyPi",

        .spi              = &spi_ports[3], // Pointer to the SPI port driving this card
        .ss_gpio          = 17,            // The SPI slave select GPIO for this SD card
        .use_card_detect  = false,         // Card detect GPIO is available
    },
    { 0 }, // Terminate by zero.
};

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
    TRACE_PRINTF("--- %s\n", __FUNCTION__);
    if (pdrv == 0) {
        // Flash memory: always present.
        return 0;
    } else {
        // SD card.
        sd_card_t *sd = sd_configure(sd_cards);
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
    TRACE_PRINTF("--- %s\n", __FUNCTION__);
    if (pdrv == 0) {
        // Flash memory: always active.
        return 0;
    } else {
        // SD card.
        sd_card_t *sd = sd_configure(sd_cards);
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
    TRACE_PRINTF("--- %s\n", __FUNCTION__);
    if (pdrv == 0) {
        // Flash memory.
        return flash_read(buff, sector, count);
    } else {
        // SD card.
        sd_card_t *sd = sd_configure(sd_cards);
        if (!sd)
            return DISK_NOTRDY;

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
    TRACE_PRINTF("--- %s\n", __FUNCTION__);
    if (pdrv == 0) {
        // Flash memory.
        return flash_write(buff, sector, count);
    } else {
        // SD card.
        sd_card_t *sd = sd_configure(sd_cards);
        if (!sd)
            return DISK_NOTRDY;

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
    TRACE_PRINTF("--- %s\n", __FUNCTION__);
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
            sd_card_t *sd = sd_configure(sd_cards);
            if (!sd)
                return DISK_NOTRDY;

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
    TRACE_PRINTF("--- %s\n", __FUNCTION__);
    memset(output, 0, sizeof(*output));
    if (pdrv == 0) {
        // Flash memory.
        flash_identify(output);
        return DISK_OK;
    } else {
        // SD card.
        sd_card_t *sd = sd_configure(sd_cards);
        if (!sd)
            return DISK_NOTRDY;

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
