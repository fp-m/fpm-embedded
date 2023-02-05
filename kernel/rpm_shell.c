//
// Interactive shell for RP/M.
// It must use only routines defined in rpm_lib.h.
//
#include <rpm/api.h>
#include <rpm/internal.h>

static char cmd_line[128]; // TODO: put into static RP/M memory block.
jmp_buf rpm_saved_point; // TODO: move to the system area

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
        // Display the prompt.
        // TODO: print current disk and directory.
        rpm_puts("c:/> ");

        // Call the line editor.
        int last_char = rpm_editline(cmd_line, sizeof(cmd_line), 1);
        if (last_char != '\r') {
            // Escape - get another line.
            rpm_puts("#\r\n");
            continue;
        }
        rpm_puts("\r\n");

        if (strlen(cmd_line) == 0) {
            // Ignore empty lines.
            continue;
        }

        // TODO: add the line to the history.

        if (strcmp(cmd_line, "exit") == 0) {
            return;
        }

        // TODO: Execute the command.
        rpm_puts(cmd_line);
        rpm_puts("\r\n");
    }
}
