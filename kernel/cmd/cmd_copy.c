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
            rpm_snprintf(prompt, sizeof(prompt), "Overwrite %s? y/n [n] ", destination);
            rpm_editline(reply, sizeof(reply), 1, prompt, 0);
            rpm_puts("\r\n");

            if (reply[0] != 'y' && reply[0] != 'Y') {
                rpm_puts("Not overwritten.\r\n");
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
    // Allocate from/to paths.
    unsigned src_len = strlen(source);
    unsigned dest_len = strlen(destination);
    char from[src_len + 1 + FF_LFN_BUF + 1];
    char to[dest_len + 1 + FF_LFN_BUF + 1];
    char *from_last = from + src_len;
    char *to_last = to + dest_len;
    strcpy(from, source);
    strcpy(to, destination);
    if (from_last[-1] != '/')
        *from_last++ = '/';
    if (to_last[-1] != '/')
        *to_last++ = '/';

    // Check destination existence and type.
    file_info_t info;
    fs_result_t result = f_stat(destination, &info);
    if (result != FR_OK) {
        // Create destination directory.
        result = f_mkdir(destination);
        if (result != FR_OK) {
            // Cannot create destination directory.
            rpm_printf("%s: %s\r\n", destination, f_strerror(result));
            return;
        }
    } else {
        if (!(info.fattrib & AM_DIR)) {
            // Destination must be a directory.
            rpm_printf("%s: Destination is not a directory, cannot copy\r\n", destination);
            return;
        }
    }

    // Scan the directory.
    directory_t *dir = alloca(f_sizeof_directory_t());
    result = f_opendir(dir, source);
    if (result != FR_OK) {
        rpm_printf("%s: %s\r\n", source, f_strerror(result));
        return;
    }
    for (;;) {
        // Get directory entry.
        result = f_readdir(dir, &info);
        if (result != FR_OK || !info.fname[0]) {
            // End of directory.
            break;
        }

        // Update from/to paths.
        strcpy(from_last, info.fname);
        strcpy(to_last, info.fname);

        if (info.fattrib & AM_DIR) {
            copy_recursive(from, to, options);
        } else {
            copy_file(from, to, options);
        }
    }
    f_closedir(dir);
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
// Path of destination files depends on existence of the target directory,
// and on presence of slash at the end of the source path.
//
// For example, assume we run command:
//      cp -r foo bar
//
// Source  Source      Destination Destination
//         files       exists      non-existent
// --------------------------------------------
// foo     foo/a       bar/foo/a   bar/a
//         foo/b       bar/foo/b   bar/b
//         foo/c       bar/foo/c   bar/c
// --------------------------------------------
// foo/    foo/a       bar/a       bar/a
//         foo/b       bar/b       bar/b
//         foo/c       bar/c       bar/c
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
        // Find the last component of the source pathname.
        // It may have trailing slashes.
        //
        const char *p = source[i] + strlen(source[i]);
        if (p[-1] != '/') {
            // No trailing slash: append last component of the source
            // to the destination path.
            while (p != source[i] && p[-1] == '/')
                --p;
            while (p != source[i] && p[-1] != '/')
                --p;
            strcpy(endp, p);
        }
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
