/*---------------------------------------------------------------------*/
/* Raw Read/Write Throughput Checker                                   */
/*---------------------------------------------------------------------*/

#include <stdio.h>
#include <systimer.h>

#include "diskio.h"
#include "fatfs.h"

int test_raw_speed(uint8_t pdrv,    /* Physical drive number */
                   uint32_t lba,    /* Start LBA for read/write test */
                   uint32_t len,    /* Number of bytes to read/write (must be multiple of sz_buff) */
                   void *buff,   /* Read/write buffer */
                   unsigned sz_buff) /* Size of read/write buffer (must be multiple of FF_MAX_SS) */
{
    uint16_t ss;
    uint32_t ofs, tmr;

#if FF_MIN_SS != FF_MAX_SS
    if (disk_ioctl(pdrv, GET_SECTOR_SIZE, &ss) != DISK_OK) {
        printf("\ndisk_ioctl() failed.\n");
        return 0;
    }
#else
    ss = FF_MAX_SS;
#endif

    printf("Starting raw write test at sector %lu in %u bytes of data chunks...", lba, sz_buff);
    tmr = systimer();
    for (ofs = 0; ofs < len / ss; ofs += sz_buff / ss) {
        if (disk_write(pdrv, buff, lba + ofs, sz_buff / ss) != DISK_OK) {
            printf("\ndisk_write() failed.\n");
            return 0;
        }
    }
    if (disk_ioctl(pdrv, CTRL_SYNC, 0) != DISK_OK) {
        printf("\ndisk_ioctl() failed.\n");
        return 0;
    }
    tmr = systimer() - tmr;
    printf("\n%lu bytes written and it took %lu timer ticks.\n", len, tmr);

    printf("Starting raw read test at sector %lu in %u bytes of data chunks...", lba, sz_buff);
    tmr = systimer();
    for (ofs = 0; ofs < len / ss; ofs += sz_buff / ss) {
        if (disk_read(pdrv, buff, lba + ofs, sz_buff / ss) != DISK_OK) {
            printf("\ndisk_read() failed.\n");
            return 0;
        }
    }
    tmr = systimer() - tmr;
    printf("\n%lu bytes read and it took %lu timer ticks.\n", len, tmr);

    printf("Test completed.\n");
    return 1;
}
