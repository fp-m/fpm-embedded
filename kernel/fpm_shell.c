//
// Interactive shell for FP/M.
// It must use only routines defined in fpm/api.h.
//
#include <fpm/api.h>
#include <fpm/fs.h>
#include <fpm/diskio.h>
#include <fpm/internal.h>

//
// Area of system data.
//
jmp_buf fpm_saved_point;
uint16_t fpm_history[FPM_CMDLINE_SIZE];

//
// Build the prompt string.
//
static void build_prompt(char *prompt, unsigned max_length)
{
    prompt[0] = 0;
    strcat(prompt, "\033[0;31m");                // dim red color
    strcat(prompt, disk_name[f_getdrive(NULL)]); // current disk

    // Get current directory.
    char path[4096];
    if (f_getcwd(path, sizeof(path)) == FR_OK) {
        strcat(prompt, ":");

        char *basename = strrchr(path, '/');
        if (basename == 0) {
            strcat(prompt, "?"); // cannot happen
        } else if (basename[-1] != ':') {
            strcat(prompt, basename + 1); // only basename
        } else {
            strcat(prompt, basename); // full path
        }
    }

    strcat(prompt, "\033[1;32m"); // bright green color
    strcat(prompt, " >");
    strcat(prompt, "\033[m"); // default color
    strcat(prompt, " ");
}

//
// Interactive dialog.
//
void fpm_shell()
{
    // TODO: Mount the SD card.

    // TODO: Load the autoexec.bat config file.
    //if (coldBoot > 0) {
    //    source("autoexec.cmd");
    //}

    // Clear history.
    fpm_history[0] = 0;

    // Restart on ^C.
    if (setjmp(fpm_saved_point) != 0) {
        // TODO: Re-initialize internal state.
    }

    // The main loop.
    for (;;) {
        // Create prompt.
        char prompt[FPM_CMDLINE_SIZE];
        build_prompt(prompt, sizeof(prompt));

        // Call the line editor.
        uint16_t buf_unicode[FPM_CMDLINE_SIZE];
        fpm_editline(buf_unicode, sizeof(buf_unicode), 1, prompt, fpm_history);
        fpm_puts("\r\n");

        // Encode as utf8.
        char cmd_line[3 * FPM_CMDLINE_SIZE];
        fpm_strlcpy_to_utf8(cmd_line, buf_unicode, sizeof(cmd_line));

        // Split into argument vector.
        char *argv[FPM_CMDLINE_SIZE / 3];
        int argc;
        const char *error = fpm_tokenize(argv, &argc, cmd_line);
        if (error) {
            fpm_puts(error);
            fpm_puts("\r\n");

            // Save wrong line to the history.
            fpm_strlcpy_unicode(fpm_history, buf_unicode, sizeof(fpm_history)/sizeof(uint16_t));
            continue;
        }

        // Ignore empty commands.
        if (argc == 0) {
            continue;
        }

        // Add the line to the history.
        fpm_strlcpy_unicode(fpm_history, buf_unicode, sizeof(fpm_history)/sizeof(uint16_t));

        if (strcmp(argv[0], "exit") == 0) {
            if (argc > 1) {
                fpm_puts("Usage: exit\r\n\r\n");
                continue;
            }
            return;
        }

        // Execute the command.
        fpm_exec(argc, argv);
    }
}
