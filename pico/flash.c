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
#include <fpm/api.h>    // Declarations of disk functions
#include <fpm/diskio.h> // Declarations of disk functions
#include <fpm/context.h>
#include <fpm/internal.h>
#include <fpm/fs.h>
#include <stdio.h>      // For debug printfs
#include "pico/stdlib.h"
#include "flash.h"
#include "hardware/sync.h"
#include "hardware/flash.h"

static char *flash_disk_image = 0;
static unsigned flash_base_offset;
static disk_info_t flash_info;

// All supported Flash chips have 4-kbyte block size.
static unsigned flash_bytes_per_block = 4096;

extern char __flash_binary_start[];
extern char __flash_binary_end[];

//
// Read Flash unique ID as 64-bit integer value.
//
static uint64_t flash_read_unique_id()
{
    // Read Unique ID instruction: 4Bh command prefix, 32 dummy bits, 64 data bits.
    uint8_t txbuf[1 + 4 + 8] = { 0x4b };
    uint8_t rxbuf[1 + 4 + 8] = {};
    uint32_t irqsave = save_and_disable_interrupts();
    flash_do_cmd(txbuf, rxbuf, sizeof(txbuf));
    restore_interrupts(irqsave);

    return (uint64_t)rxbuf[5] << 56 | (uint64_t)rxbuf[6] << 48 |
           (uint64_t)rxbuf[7] << 40 | (uint64_t)rxbuf[8] << 32 |
           (uint64_t)rxbuf[9] << 24 | (uint64_t)rxbuf[10] << 16 |
           (uint64_t)rxbuf[11] << 8 | rxbuf[12];
}

//
// Read Flash manufacturer ID and device ID.
//
static void flash_read_mf_dev_id(uint8_t *mf_id, uint16_t *dev_id)
{
    // JEDEC ID instruction: 9Fh command prefix, 24 data bits.
    uint8_t txbuf[1 + 3] = { 0x9f };
    uint8_t rxbuf[1 + 3] = {};
    uint32_t irqsave = save_and_disable_interrupts();
    flash_do_cmd(txbuf, rxbuf, sizeof(txbuf));
    restore_interrupts(irqsave);

    *mf_id = rxbuf[1];
    *dev_id = rxbuf[2] << 8 | rxbuf[3];
}

//
// Print Flash memory size and other info.
// Set flash_disk_image pointer and fill flash_info.
//
static void flash_probe()
{
    uint8_t mf_id;
    uint16_t dev_id;
    flash_read_mf_dev_id(&mf_id, &dev_id);

    // Serial number: 64 bits.
    flash_info.serial_number = flash_read_unique_id();

    // Assume 2 Mbytes by default.
    flash_info.num_bytes = 2 * 1024 * 1024;

    fpm_snprintf(flash_info.product_name, sizeof(flash_info.product_name), "mf%02x-dev%04x", mf_id, dev_id);

    switch (mf_id) {
    case 0xef: // Winbond
        switch (dev_id) {
        // clang-format off
        case 0x4015: strcpy(flash_info.product_name, "W25Q16JV-IQ");  flash_info.num_bytes = 2*1024*1024;  break;
        case 0x4016: strcpy(flash_info.product_name, "W25Q32JV-IQ");  flash_info.num_bytes = 4*1024*1024;  break;
        case 0x4017: strcpy(flash_info.product_name, "W25Q64JV-IQ");  flash_info.num_bytes = 8*1024*1024;  break;
        case 0x4018: strcpy(flash_info.product_name, "W25Q128JV-IQ"); flash_info.num_bytes = 16*1024*1024; break;
        case 0x7015: strcpy(flash_info.product_name, "W25Q16JV-IM");  flash_info.num_bytes = 2*1024*1024;  break;
        case 0x7016: strcpy(flash_info.product_name, "W25Q32JV-IM");  flash_info.num_bytes = 4*1024*1024;  break;
        case 0x7017: strcpy(flash_info.product_name, "W25Q64JV-IM");  flash_info.num_bytes = 8*1024*1024;  break;
        case 0x7018: strcpy(flash_info.product_name, "W25Q128JV-IM"); flash_info.num_bytes = 16*1024*1024; break;
        // clang-format on
        }
        break;
    case 0x1f: // Renesas
        switch (dev_id) {
        // clang-format off
        case 0x8601: strcpy(flash_info.product_name, "AT25SF161B"); flash_info.num_bytes = 2*1024*1024;  break;
        case 0x8701: strcpy(flash_info.product_name, "AT25SF321B"); flash_info.num_bytes = 4*1024*1024;  break;
        case 0x8801: strcpy(flash_info.product_name, "AT25SF641B"); flash_info.num_bytes = 8*1024*1024;  break;
        case 0x8901: strcpy(flash_info.product_name, "AT25SF128A"); flash_info.num_bytes = 16*1024*1024; break;
        // clang-format on
        }
        break;
    }

    // Subtract area occupied by the code.
    flash_base_offset = __flash_binary_end - __flash_binary_start;

    // Align to 64 kbytes.
    flash_base_offset = (flash_base_offset + 0xffff) & ~0xffff;

    flash_disk_image = &__flash_binary_start[flash_base_offset];
    flash_info.num_bytes -= flash_base_offset;
}

//
// Return size of the Flash memory in blocks.
//
unsigned flash_block_count(void)
{
    if (!flash_disk_image) {
        flash_probe();
    }
    return flash_info.num_bytes / flash_bytes_per_block;
}

//
// Return block size in bytes.
//
unsigned flash_block_size(void)
{
    return flash_bytes_per_block;
}

disk_result_t flash_read(uint8_t *buf, unsigned block, unsigned count)
{
    //printf("--- %s(block = %u, count = %u)\r\n", __func__, block, count);
    if (!flash_disk_image) {
        flash_probe();
    }
    if (count == 0)
        return DISK_PARERR;

    unsigned offset = block * flash_bytes_per_block;
    unsigned nbytes = count * flash_bytes_per_block;
    if (offset + nbytes > flash_info.num_bytes)
        return DISK_PARERR;

    memcpy(buf, &flash_disk_image[offset], nbytes);
    return DISK_OK;
}

disk_result_t flash_write(const uint8_t *buf, unsigned block, unsigned count)
{
    //printf("--- %s(block = %u, count = %u)\r\n", __func__, block, count);
    if (!flash_disk_image) {
        flash_probe();
    }

    unsigned offset = block * flash_bytes_per_block;
    unsigned nbytes = count * flash_bytes_per_block;
    if (nbytes == 0 || offset + nbytes > flash_info.num_bytes)
        return DISK_PARERR;

    // Erase flash.
    uint32_t irqsave = save_and_disable_interrupts();
    flash_range_erase(flash_base_offset + offset, nbytes);
    restore_interrupts(irqsave);

    // Write to flash.
    irqsave = save_and_disable_interrupts();
    flash_range_program(flash_base_offset + offset, buf, nbytes);
    restore_interrupts(irqsave);
    return DISK_OK;
}

//
// Get info about Flash memory.
//
void flash_identify(disk_info_t *output)
{
    if (!flash_disk_image) {
        flash_probe();
    }
    *output = flash_info;
}

//
// Load dynamic binary.
// Return true on success.
//
bool fpm_load_arch(fpm_context_t *dynobj, const char *filename)
{
    // FP/M on RP2040 microcontroller.
    file_info_t file_info;
    fs_result_t result = f_stat(filename, &file_info);
    if (result != FR_OK) {
        fpm_printf("%s: Cannot open\r\n", filename);
        return false;
    }

    fs_info_t fs_info;
    result = f_statfs(filename, &fs_info);
    if (result != FR_OK) {
        fpm_printf("%s: Cannot get filesystem status\r\n", filename);
        return false;
    }

    if (file_info.fstartblk == 0) {
        fpm_printf("%s: File is fragmented\r\n", filename);
        return false;
    }

    // Compute address of file contents.
    dynobj->base = &flash_disk_image[file_info.fstartblk * fs_info.f_bsize];
    return true;
}

//
// Unmap ELF binary from memory.
//
void fpm_unload_arch(fpm_context_t *dynobj)
{
    dynobj->base = NULL;
}
