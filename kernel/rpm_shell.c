//
// Interactive shell for RP/M.
// It must use only routines defined in rpm_lib.h.
//
#include <rpm/api.h>
#include <rpm/internal.h>

jmp_buf rpm_saved_point; // TODO: move to the system area

uint16_t rpm_history[RPM_CMDLINE_SIZE]; // TODO: move to the system area

//
// Build the prompt string.
// TODO: print current disk and directory.
//
static void build_prompt(char *prompt, unsigned max_length)
{
    prompt[0] = 0;
    strcat(prompt, "\033[0;31m"); // dim red color
    strcat(prompt, "c"); // TODO: current disk
    strcat(prompt, ":");
    strcat(prompt, "/"); // TODO: current directory
    strcat(prompt, "\033[1;32m"); // bright green color
    strcat(prompt, " >");
    strcat(prompt, "\033[m"); // default color
    strcat(prompt, " ");
}

//
// Interactive dialog.
//
void rpm_shell()
{
    // TODO: Mount the SD card.

    // TODO: Load the autoexec.bat config file.
    //if (coldBoot > 0) {
    //    source("autoexec.cmd");
    //}

    // Clear history.
    rpm_history[0] = 0;

    // Restart on ^C.
    if (setjmp(rpm_saved_point) != 0) {
        // TODO: Re-initialize internal state.
    }

    // The main loop.
    for (;;) {
        // Create prompt.
        char prompt[RPM_CMDLINE_SIZE];
        build_prompt(prompt, sizeof(prompt));

        // Call the line editor.
        uint16_t buf_unicode[RPM_CMDLINE_SIZE];
        rpm_editline(buf_unicode, sizeof(buf_unicode), 1, prompt, rpm_history);
        rpm_puts("\r\n");

        // Encode as utf8.
        char cmd_line[3 * RPM_CMDLINE_SIZE];
        rpm_strlcpy_to_utf8(cmd_line, buf_unicode, sizeof(cmd_line));

        // Split into argument vector.
        char *argv[RPM_CMDLINE_SIZE / 3];
        int argc;
        const char *error = rpm_tokenize(argv, &argc, cmd_line);
        if (error) {
            rpm_puts(error);
            rpm_puts("\r\n");

            // Save wrong line to the history.
            rpm_strlcpy_unicode(rpm_history, buf_unicode, sizeof(rpm_history)/sizeof(uint16_t));
            continue;
        }

        // Ignore empty commands.
        if (argc == 0) {
            continue;
        }

        // Add the line to the history.
        rpm_strlcpy_unicode(rpm_history, buf_unicode, sizeof(rpm_history)/sizeof(uint16_t));

        if (strcmp(argv[0], "exit") == 0) {
            return;
        }

        // TODO: Execute the command.
        rpm_printf("[%d] ", argc);
        for (int i=0; i<argc; i++) {
            if (i > 0)
                rpm_putchar(' ');
            rpm_puts(argv[i]);
        }
        rpm_puts("\r\n");
    }
}
