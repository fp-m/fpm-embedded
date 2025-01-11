//
// Rename files or directories
//
#include <fpm/api.h>
#include <fpm/fs.h>
#include <fpm/getopt.h>
#include <fpm/internal.h>

//
// Options for renaming.
//
typedef struct {
    bool force;   // do not ask for confirmation
    bool verbose; // show files as they are renamed
} options_t;

//
// Rename one file.
//
static void rename_file(const char *source, const char *destination, const options_t *options)
{
    fs_result_t result;

    // If file exists, ask user if it should be replaced.
    if (!options->force) {
        file_info_t info;
        result = f_stat(destination, &info);
        if (result == FR_OK) {

            // Prompt only if source exist.
            result = f_stat(source, &info);
            if (result != FR_OK) {
                fpm_printf("%s: %s\r\n", source, f_strerror(result));
                return;
            }

            // Ask user.
            char prompt[32 + strlen(destination)];
            uint16_t reply[32];
            fpm_snprintf(prompt, sizeof(prompt), "Overwrite %s? y/n [n] ", destination);
            fpm_editline(reply, sizeof(reply), 1, prompt, 0);
            fpm_puts("\r\n");

            if (reply[0] != 'y' && reply[0] != 'Y') {
                fpm_printf("Not overwritten.\r\n");
                return;
            }
        }
    }

    result = f_rename(source, destination);
    if (result != FR_OK) {
        fpm_printf("%s -> %s: %s\r\n", source, destination, f_strerror(result));
        return;
    }

    // Renamed successfully.
    if (options->verbose)
        fpm_printf("%s -> %s\r\n", source, destination);
}

//
// Move a list of files into destination directory.
//
static void move_files_to_directory(const char *source[], unsigned num_sources,
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
        rename_file(source[i], destination, options);
    }
}

void fpm_cmd_rename(int argc, char *argv[])
{
    static const struct fpm_option long_opts[] = {
        { "help", FPM_NO_ARG, NULL, 'h' },
        {},
    };
    const char *destination = 0;
    const char *source[argc];
    unsigned num_sources = 0;
    options_t options = {};
    struct fpm_opt opt = {};

    while (fpm_getopt(argc, argv, "fhv", long_opts, &opt) >= 0) {
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

        case 'v':
            options.verbose = true;
            break;

        case '?':
            // Unknown option: message already printed.
            fpm_puts("\r\n");
            return;

        case 'h':
usage:      fpm_puts("Usage:\r\n"
                     "    mv [options] filename ...\r\n"
                     "    rename [options] filename ...\r\n"
                     "Options:\r\n"
                     "    -f      Force, do not ask for confirmation to overwrite\r\n"
                     "    -v      Verbose: show files as they are renamed\r\n"
                     "\n");
            return;
        }
    }

    if (destination == 0 || num_sources == 0) {
        goto usage;
    }

    // No disk name is allowed in destination.
    if (strchr(destination, ':') != 0) {
        fpm_printf("%s: cannot move across devices, sorry\r\n\n", destination);
        return;
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
            fpm_printf("%s is not a directory\r\n", destination);
        } else {
            // Rename one file.
            rename_file(source[0], destination, &options);
        }
    } else {
        move_files_to_directory(source, num_sources, destination, &options);
    }
    fpm_puts("\r\n");
}
