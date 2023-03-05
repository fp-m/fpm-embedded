//
// Copy files or directories
//
#include <rpm/api.h>
#include <rpm/fs.h>
#include <rpm/getopt.h>
#include <rpm/internal.h>
#include <stdlib.h>

//
// Options for copying.
//
typedef struct {
    bool force;     // do not ask for confirmation
    bool recursive; // copy directories recursively
    bool verbose;   // show files as they are copied
} options_t;

//
// Copy one file.
//
static void copy_file(const char *source, const char *destination, const options_t *options)
{
    fs_result_t result;

    // If file exists, ask user if it should be replaced.
    if (!options->force) {
        file_info_t info;
        result = f_stat(destination, &info);
        if (result == FR_OK) {
            char prompt[32 + strlen(destination)];
            uint16_t reply[32];
            rpm_snprintf(prompt, sizeof(prompt), "Overwrite %s? (y/n [n]) ", destination);
            rpm_editline(reply, sizeof(reply), 1, prompt, 0);

            if (reply[0] != 'y' && reply[0] != 'Y') {
                rpm_printf("\r\nNot overwritten.\r\n");
                return;
            }
        }
    }

    // Open source file.
    file_t *fsrc = alloca(f_sizeof_file_t());
    result = f_open(fsrc, source, FA_READ);
    if (result != FR_OK) {
        rpm_printf("%s: %s\r\n", source, f_strerror(result));
        return;
    }

    // Open destination file.
    file_t *fdest = alloca(f_sizeof_file_t());
    result = f_open(fdest, destination, FA_WRITE | FA_CREATE_ALWAYS);
    if (result != FR_OK) {
        rpm_printf("%s: %s\r\n", destination, f_strerror(result));
        f_close(fsrc);
        return;
    }

    // Copy contents.
    char buf[4096];
    for (;;) {
        unsigned nbytes_read = 0;
        result = f_read(fsrc, buf, sizeof(buf), &nbytes_read);
        if (result != FR_OK) {
            // Read error.
            rpm_printf("%s: %s\r\n", source, f_strerror(result));
fatal:      f_close(fsrc);
            f_close(fdest);
            return;
        }
        if (nbytes_read == 0) {
            // End of file.
            break;
        }
        unsigned nbytes_written = 0;
        result = f_write(fdest, buf, nbytes_read, &nbytes_written);
        if (result != FR_OK) {
            // Write error.
            rpm_printf("%s: %s\r\n", destination, f_strerror(result));
            goto fatal;
        }
        if (nbytes_written != nbytes_read) {
            // Out of disk space.
            rpm_printf("%s: Not enough space on device\r\n", destination);
            goto fatal;
        }
    }

    // Copied successfully.
    f_close(fsrc);
    f_close(fdest);
    if (options->verbose)
        rpm_printf("%s -> %s\r\n", source, destination);
}

//
// Copy directory recursively.
//
static void copy_recursive(const char *source, const char *destination, const options_t *options)
{
    rpm_printf("TODO: recursive copy %s -> %s\r\n", source, destination);
}

//
// Copy one file or directory.
//
static void copy_object(const char *source, const char *destination, const options_t *options)
{
    file_info_t info;
    fs_result_t result;

    // Source can be a file or a directory.
    result = f_stat(source, &info);
    if (result != FR_OK) {
        rpm_printf("%s: %s\r\n", source, f_strerror(result));
        return;
    }
    if (info.fattrib & AM_DIR) {
        if (options->recursive) {
            copy_recursive(source, destination, options);
        } else {
            rpm_printf("%s: Cannot copy directory without -r option\r\n", source);
        }
    } else {
        copy_file(source, destination, options);
    }
}

//
// Copy a list of files/directories into destination directory.
//
static void copy_to_directory(const char *source[], unsigned num_sources,
                              const char *dest_dir, const options_t *options)
{
    unsigned baselen = strlen(dest_dir);
    char destination[baselen + FF_LFN_BUF + 2];

    strcpy(destination, dest_dir);
    char *endp = &destination[baselen];
    if (!baselen || endp[-1] != '/') {
        // Add trailing slash.
        *endp++ = '/';
        ++baselen;
    }
    for (unsigned i = 0; i < num_sources; i++) {
        //
        // Find the last component of the source pathname.  It
        // may have trailing slashes.
        //
        const char *p = source[i] + strlen(source[i]);
        while (p != source[i] && p[-1] == '/')
            --p;
        while (p != source[i] && p[-1] != '/')
            --p;

        strcpy(endp, p);
        copy_object(source[i], destination, options);
    }
}

void rpm_cmd_copy(int argc, char *argv[])
{
    static const struct rpm_option long_opts[] = {
        { "help", RPM_NO_ARG, NULL, 'h' },
        {},
    };
    const char *destination = 0;
    const char *source[argc];
    unsigned num_sources = 0;
    options_t options = {};
    struct rpm_opt opt = {};

    while (rpm_getopt(argc, argv, "frvh", long_opts, &opt) >= 0) {
        switch (opt.ret) {
        case 1:
            // Last argument is destination.
            if (destination != 0) {
                source[num_sources++] = destination;
            }
            destination = opt.arg;
            break;

        case 'f':
            options.force = true;
            break;

        case 'r':
            options.recursive = true;
            break;

        case 'v':
            options.verbose = true;
            break;

        case '?':
            // Unknown option: message already printed.
            rpm_puts("\r\n");
            return;

        case 'h':
usage:      rpm_puts("Usage:\r\n"
                     "    cp [options] source ... destination\r\n"
                     "    copy [options] source ... destination\r\n"
                     "Options:\r\n"
                     "    -f      Force, do not ask for confirmation to overwrite\r\n"
                     "    -r      Recursively copy directories and their contents\r\n"
                     "    -v      Verbose: show files as they are copied\r\n"
                     "\n");
            return;
        }
    }

    if (destination == 0 || num_sources == 0) {
        goto usage;
    }

    file_info_t info = {};
    fs_result_t result = f_stat(destination, &info);
    if (result == FR_INVALID_NAME) {
        // Cannot stat current directory - fake it.
        info.fattrib = AM_DIR;
        result = FR_OK;
    }
    if (result != FR_OK || !(info.fattrib & AM_DIR)) {
        // The destination doesn't exist or isn't a directory.
        // More than 2 arguments is an error in this case.
        if (num_sources != 1) {
            rpm_printf("%s is not a directory\r\n", destination);
        } else {
            // Copy one file.
            copy_object(source[0], destination, &options);
        }
    } else {
        copy_to_directory(source, num_sources, destination, &options);
    }
    rpm_puts("\r\n");
}
