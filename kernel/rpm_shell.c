//
// Interactive shell for RP/M.
// It must use only routines defined in rpm_lib.h.
//
#include <rpm/api.h>
#include <rpm/internal.h>

jmp_buf rpm_saved_point; // TODO: move to the system area

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

    // Restart on ^C.
    if (setjmp(rpm_saved_point) != 0) {
        // TODO: Re-initialize internal state.
    }

    // The main loop.
    for (;;) {
        // Create prompt.
        char prompt[128];
        build_prompt(prompt, sizeof(prompt));

        // Call the line editor.
        uint16_t buf_unicode[128];
        rpm_editline(prompt, buf_unicode, sizeof(buf_unicode), 1);
        rpm_puts("\r\n");

        if (buf_unicode[0] == 0) {
            // Ignore empty lines.
            continue;
        }

        // Encode as utf8.
        char cmd_line[500];
        rpm_strlcpy_to_utf8(cmd_line, buf_unicode, sizeof(cmd_line));

        // TODO: add the line to the history.

        if (strcmp(cmd_line, "exit") == 0) {
            return;
        }

        // TODO: Execute the command.
        rpm_puts(cmd_line);
        rpm_puts("\r\n");
    }
}
