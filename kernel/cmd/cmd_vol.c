//
// Show the volume label of a disk device
//
#include <rpm/api.h>
#include <rpm/getopt.h>
#include <rpm/internal.h>
#include <rpm/fs.h>
#include <rpm/diskio.h>
#include <alloca.h>

//
// Print information about given volume.
//
//       Disk Drive: flash or sd
//       Disk Model: identify Flash chip or extract product name from SD card
//        Disk Size: in megabytes
//        Unique Id: read hardware ID from Flash or SD card
// Filesystem Label: read disklabel from filesystem
//    Serial Number: read FATFS serial ID
//
//
static void print_volume_info(const char *drive_name)
{
    // Check the argument.
    const char *path = drive_name;
    int drive = f_getdrive(&path);
    if (drive < 0 || *path != '\0') {
        rpm_printf("%s: Invalid drive name\r\n\n", drive_name);
        return;
    }

    // Get filesystem label.
    // The filesystem may not be formatted, which is OK.
    char label[32] = "";
    uint32_t serial_number;
    fs_result_t result = f_getlabel(drive_name, label, &serial_number);
    if (result != FR_OK && result != FR_NO_FILESYSTEM) {
        rpm_puts(f_strerror(result));
        rpm_puts("\r\n\n");
        return;
    }

    // Print drive name.
    rpm_printf("      Disk Drive: %s\r\n", disk_name[drive]);

    //
    // Print disk name, size and unique id.
    //
    disk_info_t info;
    if (disk_identify(drive, &info) == DISK_OK) {
        rpm_printf("      Disk Model: '%s'\r\n", info.product_name);
        rpm_printf("       Disk Size: %u.%u Mbytes\r\n",
                   (unsigned) (info.num_bytes / 1024 / 1024),
                   (unsigned) (info.num_bytes * 10 / 1024 / 1024 % 10));
        rpm_printf("       Unique Id: %08jx\r\n", (intmax_t) info.serial_number);
    }

    //
    // Print filesystem label and serial number.
    //
    if (result == FR_OK) {
        rpm_printf("Filesystem Label: '%s'\r\n", label);
        rpm_printf("   Serial Number: %08x\r\n", serial_number);
    }
    rpm_puts("\r\n");
}

//
// Return time in microseconds since t0.
//
static uint64_t elapsed_usec(uint64_t t0)
{
    uint64_t usec = rpm_time_usec() - t0;
    if (usec < 1)
        usec = 1;
    return usec;
}

static void print_speed(const char *title, unsigned mbytes, uint64_t usec)
{
    rpm_printf("%s speed: %u Mbytes in %u.%03u seconds = %u kbytes/sec\r\n",
        title, mbytes, (unsigned) (usec / 1000000), (unsigned) (usec / 1000 % 1000),
        (unsigned) (mbytes * 1024000000ULL / usec));
}

//
// Measure speed of disk IO.
//
static void test_speed(const char *drive_name)
{
    // Check the argument.
    const char *path = drive_name;
    int drive_index = f_getdrive(&path);
    if (drive_index < 0 || *path != '\0') {
        rpm_printf("%s: Invalid drive name\r\n\n", drive_name);
        return;
    }

    // Get free space.
    fs_info_t info;
    fs_result_t result = f_statfs(drive_name, &info);
    if (result != FR_OK) {
        // Drive not mounted.
        rpm_puts(f_strerror(result));
        rpm_puts("\r\n\n");
        return;
    }

    // Check free space.
    unsigned free_mbytes = (uint64_t) info.f_bavail * info.f_bsize / 1024 / 1024;
    if (free_mbytes < 1) {
        rpm_printf("%s: Insufficient free space\r\n\n", drive_name);
        return;
    }

    // Use 8 megabytes if available.
    unsigned datasize_mbytes = 8;
    if (free_mbytes < datasize_mbytes) {
        datasize_mbytes = free_mbytes;
    }

    // Use 4-kbyte block size.
    const unsigned BLOCKSIZE_KBYTES = 4;
    unsigned const bytes_per_block = BLOCKSIZE_KBYTES * 1024;
    unsigned nblocks = datasize_mbytes * 1024 / BLOCKSIZE_KBYTES;
    rpm_printf("Testing %u-kbyte block size.\r\n", BLOCKSIZE_KBYTES);

    // Build path of temporary file.
    char filename[FF_LFN_BUF+1];
    strcpy(filename, disk_name[drive_index]);
    strcat(filename, ":/diskspeed.data");

    // Open temporary file.
    file_t *fd = alloca(f_sizeof_file_t());
    result = f_open(fd, filename, FA_WRITE | FA_READ | FA_CREATE_ALWAYS);
    if (result != FR_OK) {
        rpm_printf("Cannot create temporary file.\r\n\r\n");
        rpm_printf("%s: %s\r\n\n", filename, f_strerror(result));
        return;
    }

    // Fill buffer with some data.
    char buf[BLOCKSIZE_KBYTES * 1024];
    unsigned n;
    for (n = 0; n < sizeof(buf); n++) {
        buf[n] = ~n;
    }

    // Write data to file.
    rpm_delay_msec(500);
    uint64_t t0 = rpm_time_usec();
    for (n = 0; n < nblocks; n++) {
        unsigned nbytes_written = 0;
        result = f_write(fd, buf, bytes_per_block, &nbytes_written);
        if (result != FR_OK || nbytes_written != bytes_per_block) {
            rpm_printf("Write error at block %u.\r\n\n", n);
            return;
        }
    }
    print_speed("Write", datasize_mbytes, elapsed_usec(t0));

    // Rewind.
    result = f_rewind(fd);
    if (result != FR_OK) {
        rpm_printf("Rewind error.\r\n\n");
        return;
    }

    // Read data from file.
    rpm_delay_msec(500);
    t0 = rpm_time_usec();
    for (n = 0; n < nblocks; n++) {
        unsigned nbytes_read = 0;
        result = f_read(fd, buf, bytes_per_block, &nbytes_read);
        if (result != FR_OK || nbytes_read != bytes_per_block) {
            rpm_printf("Read error at block %u.\r\n\n", n);
            return;
        }
    }
    print_speed(" Read", datasize_mbytes, elapsed_usec(t0));

    // Remove temporary file.
    f_close(fd);
    result = f_unlink(filename);
    if (result != FR_OK) {
        rpm_printf("%s: %s\r\n\n", filename, f_strerror(result));
        return;
    }
    rpm_puts("\r\n");
}

//
// Set filesystem label.
//
static void set_label(const char *label)
{
    fs_result_t result = f_setlabel(label);
    if (result != FR_OK) {
        rpm_puts(f_strerror(result));
        rpm_puts("\r\n\n");
    }
}

void rpm_cmd_vol(int argc, char *argv[])
{
    static const struct rpm_option long_opts[] = {
        { "help", RPM_NO_ARG, NULL, 'h' },
        { "label", RPM_REQUIRED_ARG, NULL, 'l' },
        {},
    };
    struct rpm_opt opt = {};
    const char *path = "";
    bool test_flag = false;

    while (rpm_getopt(argc, argv, "hl:t", long_opts, &opt) >= 0) {
        switch (opt.ret) {
        case 1:
            path = opt.arg;
            break;

        case 'l':
            set_label(opt.arg);
            return;

        case 't':
            test_flag = true;
            break;

        case '?':
            // Unknown option: message already printed.
            rpm_puts("\r\n");
            return;

        case 'h':
            rpm_puts("Usage:\r\n"
                     "    vol [-t] [sd: | flash:]\r\n"
                     "    vol -l disk:label\r\n"
                     "Options:\r\n"
                     "    -l, --label   Set the filesystem label\r\n"
                     "    -t, --test    Test disk speed\r\n"
                     "\n");
            return;
        }
    }

    if (test_flag) {
        // Measure disk speed.
        test_speed(path);
    } else {
        // Show current drive.
        print_volume_info(path);
    }
}
