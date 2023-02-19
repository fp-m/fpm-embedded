//
// Test FatFS routines.
//
#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <cmocka.h>
#include <rpm/fs.h>
#include <rpm/diskio.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

static const unsigned SECTOR_SIZE = 512;

static char fs_image[1024*1024]; // 1 Mbyte

media_status_t disk_status(uint8_t unit)
{
    printf("--- %s(unit = %u)\n", __func__, unit);
    return 0;
}

media_status_t disk_initialize(uint8_t unit)
{
    printf("--- %s(unit = %u)\n", __func__, unit);
    return 0;
}

disk_result_t disk_ioctl(uint8_t unit, uint8_t cmd, void *buf)
{
    switch (cmd) {
    case GET_BLOCK_SIZE:
        // Get erase block size.
        printf("--- %s(unit = %u, cmd = GET_BLOCK_SIZE)\n", __func__, unit);
        *(uint32_t *)buf = 1;
        return DISK_OK;

    case GET_SECTOR_COUNT:
        // Get media size.
        printf("--- %s(unit = %u, cmd = GET_SECTOR_COUNT)\n", __func__, unit);
        *(uint32_t *)buf = sizeof(fs_image) / SECTOR_SIZE; // number of sectors
        return DISK_OK;

    case CTRL_SYNC:
        // Complete pending write process.
        printf("--- %s(unit = %u, cmd = CTRL_SYNC)\n", __func__, unit);
        return DISK_OK;

    default:
        printf("--- %s(unit = %u, cmd = %u)\n", __func__, unit, cmd);
        return DISK_PARERR;
    }
}

disk_result_t disk_read(uint8_t unit, uint8_t *buf, unsigned sector, unsigned count)
{
    printf("--- %s(unit = %u, sector = %u, count = %u)\n", __func__, unit, sector, count);
    assert_true(count > 0);
    assert_true(sector + count <= sizeof(fs_image) / SECTOR_SIZE);

    memcpy(buf, &fs_image[sector * SECTOR_SIZE], count * SECTOR_SIZE);
    return DISK_OK;
}

disk_result_t disk_write(uint8_t unit, const uint8_t *buf, unsigned sector, unsigned count)
{
    printf("--- %s(unit = %u, sector = %u, count = %u)\n", __func__, unit, sector, count);
    assert_true(count > 0);
    assert_true(sector + count <= sizeof(fs_image) / SECTOR_SIZE);

    memcpy(&fs_image[sector * SECTOR_SIZE], buf, count * SECTOR_SIZE);
    return DISK_OK;
}

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

static void mkfs(void **unused)
{
    // Create a FAT volume.
    const char *filename = "fs.img";
    char buf[4*1024];
    fs_result_t mkfs_result = f_mkfs(filename, buf, sizeof(buf));
    assert_int_equal(mkfs_result, FR_OK);

    // Save to file.
    int fd = open(filename, O_CREAT | O_TRUNC | O_WRONLY, 0664);
    assert_true(fd >= 0);
    int write_result = write(fd, fs_image, sizeof(fs_image));
    close(fd);
    assert_int_equal(write_result, sizeof(fs_image));
}

//
// Run all tests.
//
int main()
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(mkfs),
    };
    return cmocka_run_group_tests(tests, NULL, NULL);
}
