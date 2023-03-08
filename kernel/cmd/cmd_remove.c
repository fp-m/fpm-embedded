//
// Delete a file or set of files
//
#include <rpm/api.h>
#include <rpm/fs.h>
#include <rpm/getopt.h>
#include <rpm/internal.h>
#include <stdlib.h>

//
// Options for removing files.
//
typedef struct {
    bool force;     // do not ask for confirmation
    bool recursive; // copy directories recursively
    bool verbose;   // show files as they are copied
} options_t;

//
// Remove regular file.
//
static void remove_file(const char *filename, const options_t *options)
{
    if (!options->force) {
        // Ask user.
        char prompt[48 + strlen(filename)];
        uint16_t reply[32];
        rpm_snprintf(prompt, sizeof(prompt), "Remove file %s? y/n [n] ", filename);
        rpm_editline(reply, sizeof(reply), 1, prompt, 0);
        rpm_puts("\r\n");
        if (reply[0] != 'y' && reply[0] != 'Y') {
            rpm_printf("Not removed.\r\n");
            return;
        }
    }
    fs_result_t result = f_unlink(filename);
    if (result != FR_OK) {
        if (!options->force) {
            rpm_printf("%s: %s\r\n", filename, f_strerror(result));
        }
        return;
    }

    if (options->verbose) {
        rpm_printf("%s\r\n", filename);
    }
}

//
// Remove directory recursively.
//
static fs_result_t remove_recursive(const char *dirname, const options_t *options)
{
    // Try to remove directory, assuming it's empty.
    fs_result_t result = f_unlink(dirname);
    if (result == FR_OK) {
        // Succeeded.
        if (options->verbose) {
            rpm_printf("%s\r\n", dirname);
        }
        return FR_OK;
    }

    // Scan directory and remove files and sub-directories recursively.
    directory_t *dir = alloca(f_sizeof_directory_t());
    result = f_opendir(dir, dirname);
    if (result != FR_OK) {
        rpm_printf("%s: %s\r\n", dirname, f_strerror(result));
        return result;
    }

    // Allocate path for child.
    unsigned baselen = strlen(dirname);
    char child[baselen + FF_LFN_BUF + 2];
    strcpy(child, dirname);
    char *child_last = child + baselen;
    *child_last++ = '/';

    for (;;) {
        file_info_t info;
        result = f_readdir(dir, &info); /* Get a directory item */
        if (result != FR_OK || !info.fname[0]) {
            // End of directory.
            break;
        }

        strcpy(child_last, info.fname);
        if (info.fattrib & AM_DIR) {
            // Delete sub-directory.
            result = remove_recursive(child, options);
        } else {
            // Delete file.
            result = f_unlink(child);
            if (result != FR_OK) {
                rpm_printf("%s: %s\r\n", child, f_strerror(result));
            } else if (options->verbose) {
                rpm_printf("%s\r\n", child);
            }
        }

        if (result != FR_OK)
            break;
    }
    f_closedir(dir);

    if (result == FR_OK) {
        // Now the directory is empty: delete it.
        result = f_unlink(dirname);
        if (result != FR_OK) {
            rpm_printf("%s: %s\r\n", dirname, f_strerror(result));
        } else if (options->verbose) {
            rpm_printf("%s\r\n", child);
        }
    }
    return result;
}

//
// Remove directory.
//
static void remove_directory(const char *path, const options_t *options)
{
    if (!options->recursive) {
        if (!options->force) {
            rpm_printf("%s: Cannot remove directory without -r option\r\n", path);
        }
        return;
    }

    // Ask user.
    char prompt[48 + strlen(path)];
    uint16_t reply[32];
    rpm_snprintf(prompt, sizeof(prompt), "Remove directory %s? y/n [n] ", path);
    rpm_editline(reply, sizeof(reply), 1, prompt, 0);
    rpm_puts("\r\n");
    if (reply[0] != 'y' && reply[0] != 'Y') {
        rpm_printf("Not removed.\r\n");
        return;
    }

    remove_recursive(path, options);
}

//
// Remove one file or directory.
//
static void remove(const char *path, const options_t *options)
{
    file_info_t info;
    fs_result_t result;

    result = f_stat(path, &info);
    if (result != FR_OK) {
        if (!options->force) {
            rpm_printf("%s: %s\r\n", path, f_strerror(result));
        }
        return;
    }
    if (info.fattrib & AM_DIR) {
        remove_directory(path, options);
    } else {
        remove_file(path, options);
    }
}

void rpm_cmd_remove(int argc, char *argv[])
{
    static const struct rpm_option long_opts[] = {
        { "help", RPM_NO_ARG, NULL, 'h' },
        {},
    };
    options_t options = {};
    struct rpm_opt opt = {};
    unsigned argcount = 0;

    while (rpm_getopt(argc, argv, "frvh", long_opts, &opt) >= 0) {
        switch (opt.ret) {
        case 1:
            remove(opt.arg, &options);
            argcount++;
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
                     "    rm [options] filename ...\r\n"
                     "    erase [options] filename ...\r\n"
                     "Options:\r\n"
                     "    -f      Force removing, do not ask for confirmation\r\n"
                     "    -r      Remove directories and their contents recursively\r\n"
                     "    -v      Verbose: show files as they are removed\r\n"
                     "\n");
            return;
        }
    }

    if (argcount == 0) {
        // Nothing to remove.
        goto usage;
    }
    rpm_puts("\r\n");
}
