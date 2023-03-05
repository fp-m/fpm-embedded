//
// Copy files or directories
//
#include <rpm/api.h>
#include <rpm/fs.h>
#include <rpm/getopt.h>
#include <rpm/internal.h>

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

    //
    // Check access.  If file exists, ask user if it should be replaced.
    // Otherwise if file exists but isn't writable make sure the user wants to clobber it.
    //
    if (!options->force) {
        file_info_t info;
        result = f_stat(destination, &info);
        if (result == FR_OK) {

            // Prompt only if source exist.
            result = f_stat(source, &info);
            if (result != FR_OK) {
                rpm_printf("%s: %s\r\n", source, f_strerror(result));
                return;
            }

            // Ask user.
            char prompt[32 + strlen(destination)];
            uint16_t reply[32];
            rpm_snprintf(prompt, sizeof(prompt), "Overwrite %s? (y/n [n]) ", destination);
            rpm_editline(reply, sizeof(reply), 1, prompt, 0);

            if (reply[0] != 'y' && reply[0] != 'Y') {
                rpm_printf("Not overwritten.\r\n");
                return;
            }
        }
    }

    //TODO
    //result = copy(source, destination);
    //if (result != FR_OK) {
    //    rpm_printf("%s -> %s: %s\r\n", source, destination, f_strerror(result));
    //    return;
    //}

    // Copied successfully.
    if (options->verbose)
        rpm_printf("%s -> %s\r\n", source, destination);
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
        copy_file(source[i], destination, options);
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
                     "Options:\n"
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
            copy_file(source[0], destination, &options);
        }
    } else {
        copy_to_directory(source, num_sources, destination, &options);
    }
    rpm_puts("\r\n");
}
