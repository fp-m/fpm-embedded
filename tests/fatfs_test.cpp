//
// Test FatFS routines.
//
#include <gtest/gtest.h>
#include <fpm/fs.h>
#include <fpm/diskio.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <alloca.h>

//
// Names of disk volumes.
//
const char *disk_name[DISK_VOLUMES] = { "flash", "sd" };

static unsigned sector_size;
static unsigned block_size = 1;

//
// Reserve 40 Mbytes for FS image.
//
static char fs_image[40*1024*1024];
static unsigned fs_nbytes = sizeof(fs_image); // or 1 Mbyte for extFAT

media_status_t disk_status(uint8_t unit)
{
    //printf("--- %s(unit = %u)\n", __func__, unit);
    return 0;
}

media_status_t disk_initialize(uint8_t unit)
{
    //printf("--- %s(unit = %u)\n", __func__, unit);
    return 0;
}

disk_result_t disk_ioctl(uint8_t unit, uint8_t cmd, void *buf)
{
    switch (cmd) {
    case GET_BLOCK_SIZE:
        // Get erase block size.
        //printf("--- %s(unit = %u, cmd = GET_BLOCK_SIZE)\n", __func__, unit);
        *(uint32_t *)buf = block_size;
        return DISK_OK;

    case GET_SECTOR_SIZE:
        //printf("--- %s(unit = %u, cmd = GET_SECTOR_SIZE)\n", __func__, unit);
        *(uint16_t *)buf = sector_size;
        return DISK_OK;

    case GET_SECTOR_COUNT:
        // Get media size.
        //printf("--- %s(unit = %u, cmd = GET_SECTOR_COUNT)\n", __func__, unit);
        *(uint32_t *)buf = fs_nbytes / sector_size; // number of sectors
        return DISK_OK;

    case CTRL_SYNC:
        // Complete pending write process.
        //printf("--- %s(unit = %u, cmd = CTRL_SYNC)\n", __func__, unit);
        return DISK_OK;

    default:
        printf("--- %s(unit = %u, cmd = %u)\n", __func__, unit, cmd);
        return DISK_PARERR;
    }
}

disk_result_t disk_read(uint8_t unit, uint8_t *buf, unsigned sector, unsigned count)
{
    //printf("--- %s(unit = %u, sector = %u, count = %u)\n", __func__, unit, sector, count);
    if (count == 0)
        throw std::runtime_error("Zero count in disk_read()");

    if (sector + count > fs_nbytes / sector_size)
        throw std::runtime_error("Too large count in disk_read()");

    memcpy(buf, &fs_image[sector * sector_size], count * sector_size);
    return DISK_OK;
}

disk_result_t disk_write(uint8_t unit, const uint8_t *buf, unsigned sector, unsigned count)
{
    //printf("--- %s(unit = %u, sector = %u, count = %u)\n", __func__, unit, sector, count);
    if (count == 0)
        throw std::runtime_error("Zero count in disk_write()");
    if (sector + count > fs_nbytes / sector_size)
        throw std::runtime_error("Too large count in disk_write()");

    memcpy(&fs_image[sector * sector_size], buf, count * sector_size);
    return DISK_OK;
}

//
// Get date and time (local).
//
extern "C" {
void fpm_get_datetime(int *year, int *month, int *day, int *dotw, int *hour, int *min, int *sec)
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
// Create a file with given name and contents.
//
static void write_file(const char *filename, const char *contents)
{
    // Create file.
    auto fp = (file_t*) alloca(f_sizeof_file_t());
    auto result = f_open(fp, filename, FA_WRITE | FA_CREATE_ALWAYS);
    EXPECT_EQ(result, FR_OK);

    // Check current position.
    EXPECT_EQ(f_tell(fp), 0);

    // Write data.
    unsigned nbytes = strlen(contents);
    unsigned written = 0;
    result = f_write(fp, contents, nbytes, &written);
    EXPECT_EQ(nbytes, written);

    // Check position again.
    EXPECT_EQ(f_tell(fp), nbytes);

    // Close the file.
    result = f_close(fp);
    EXPECT_EQ(result, FR_OK);
}

//
// Check contents of the file.
//
static void read_file(const char *filename, const char *contents)
{
    // Open file.
    auto fp = (file_t*) alloca(f_sizeof_file_t());
    auto result = f_open(fp, filename, FA_READ);
    EXPECT_EQ(result, FR_OK);

    // Check current position.
    EXPECT_EQ(f_tell(fp), 0);

    // Read data.
    char buf[128] = {};
    unsigned nbytes_read = 0;
    unsigned nbytes_expected = strlen(contents);
    result = f_read(fp, buf, sizeof(buf), &nbytes_read);
    EXPECT_EQ(nbytes_read, nbytes_expected);
    EXPECT_STREQ(buf, contents);

    // Check position again.
    EXPECT_EQ(f_tell(fp), nbytes_read);

    // Close the file.
    result = f_close(fp);
    EXPECT_EQ(result, FR_OK);
}

//
// Test filesystem in specified format: either FM_FAT32 or FM_EXFAT
//
static void test_mkfs_write_read_delete(unsigned fmt)
{
    const char *filename = (fmt & FM_FAT32) ? "fat32.img" :
                             (fmt & FM_FAT) ? "fat16.img" :
                                              "exfat.img";
    char buf[4*1024];

    // Use 512 byte sector size for FAT32 volume,
    // and 4096 bytes for exFAT and FAT16.
    // We are going to use exFAT for Flash memory,
    // which typically has erase block size 4 kbytes.
    sector_size = (fmt & FM_FAT32) ? 512 : 4096;

    // Create FAT32 volume, non-partitioned.
    // Minimal size for FAT32 volume is 33 Mbytes,
    // for exFAT - 128 kbytes.
    // Let's create 40 Mbytes for FAT32 and 1 Mbyte for exFAT.
    fs_nbytes = (fmt & FM_FAT32) ? sizeof(fs_image) : 1*1024*1024;
    memset(fs_image, 0xff, fs_nbytes);
    fs_result_t result = f_mkfs(filename, fmt, buf, sizeof(buf));
    EXPECT_EQ(result, FR_OK);

    // Mount drive.
    result = f_mount("0:");
    EXPECT_EQ(result, FR_OK);

    // Check free space on the drive.
    unsigned const expect_free_clusters = (fmt & FM_FAT32) ? 81184 :
                                            (fmt & FM_FAT) ? 250 :
                                                             24;
    fs_info_t fsinfo;
    result = f_statfs("", &fsinfo);
    EXPECT_EQ(result, FR_OK);
    EXPECT_EQ(fsinfo.f_bavail, expect_free_clusters);

    // Set disk label.
    const char *disk_label = "mydisklabel";
    result = f_setlabel(disk_label);
    EXPECT_EQ(result, FR_OK);

    write_file("Foo.txt", "'Twas brillig, and the slithy toves");

    // Create directory.
    result = f_mkdir("Bar");
    EXPECT_EQ(result, FR_OK);

    write_file("Bar/Long-file-name.txt", "Did gyre and gimble in the wabe");

    write_file("Αβρακαδαβρα.txt", "Kαυσπροῦντος ἤδη, γλοῖσχρα διὰ περισκιᾶς");

    // Unmount.
    result = f_unmount("0:");
    EXPECT_EQ(result, FR_OK);

    // Save FS image to file.
    int fd = open(filename, O_CREAT | O_TRUNC | O_WRONLY, 0664);
    ASSERT_TRUE(fd >= 0);
    int nbytes = write(fd, fs_image, fs_nbytes);
    close(fd);
    EXPECT_EQ(nbytes, fs_nbytes);

    // Mount the drive again.
    result = f_mount("0:");
    EXPECT_EQ(result, FR_OK);

    // Get disk label.
    char label[128];
    uint32_t serial_number = 0;
    result = f_getlabel("0:", label, &serial_number);
    EXPECT_EQ(result, FR_OK);
    //printf("--- %s() volume label = '%s', serial number = %08x\n", __func__, label, serial_number);
    EXPECT_STREQ(label, (fmt & FM_EXFAT) ? "mydisklabel" : "MYDISKLABEL");
    ASSERT_TRUE(serial_number != 0);

    read_file("Foo.txt", "'Twas brillig, and the slithy toves");

    read_file("Bar/Long-file-name.txt", "Did gyre and gimble in the wabe");

    read_file("Αβρακαδαβρα.txt", "Kαυσπροῦντος ἤδη, γλοῖσχρα διὰ περισκιᾶς");

    // Check directory.
    file_info_t info = {};
    result = f_stat("Bar", &info);
    EXPECT_EQ(result, FR_OK);
    EXPECT_EQ(info.fattrib, AM_DIR);   // File attribute
    EXPECT_EQ(info.fsize, 0);          // File size
    EXPECT_EQ(info.fdate, 0x5652);     // Modified date
    EXPECT_EQ(info.ftime, 0x7c36);     // Modified time
    EXPECT_STREQ(info.fname, "Bar");   // Primary file name
    EXPECT_STREQ(info.altname, (fmt & FM_EXFAT) ? "" : "BAR"); // Alternative file name

    // Check file with unicode name.
    result = f_stat("Αβρακαδαβρα.txt", &info);
    EXPECT_EQ(result, FR_OK);
    EXPECT_EQ(info.fattrib, AM_ARC);             // File attribute
    EXPECT_EQ(info.fsize, 79);                   // File size
    EXPECT_EQ(info.fdate, 0x5652);               // Modified date
    EXPECT_EQ(info.ftime, 0x7c36);               // Modified time
    EXPECT_STREQ(info.fname, "Αβρακαδαβρα.txt"); // Primary file name
    EXPECT_STREQ(info.altname, (fmt & FM_EXFAT) ? "" : "______~1.TXT"); // Alternative file name

    // Delete file.
    result = f_unlink("Foo.txt");
    EXPECT_EQ(result, FR_OK);

    // Try to delete it again.
    result = f_unlink("Foo.txt");
    EXPECT_EQ(result, FR_NO_FILE);

    // Try to delete non-empty directory.
    result = f_unlink("Bar");
    EXPECT_EQ(result, FR_DENIED);

    // Delete file in the directorry.
    result = f_unlink("Bar/Long-file-name.txt");
    EXPECT_EQ(result, FR_OK);

    // Delete the directory.
    result = f_unlink("Bar");
    EXPECT_EQ(result, FR_OK);

    // Delete file with unicode name.
    result = f_unlink("Αβρακαδαβρα.txt");
    EXPECT_EQ(result, FR_OK);

    // Unmount.
    result = f_unmount("0:");
    EXPECT_EQ(result, FR_OK);
}

TEST(fatfs, fat32)
{
    test_mkfs_write_read_delete(FM_FAT32);
}

TEST(fatfs, exfat)
{
    test_mkfs_write_read_delete(FM_EXFAT | FM_SFD);
}

TEST(fatfs, fat16)
{
    test_mkfs_write_read_delete(FM_FAT | FM_SFD);
}
