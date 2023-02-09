//
// Interactive shell for RP/M.
// It must use only routines defined in rpm_lib.h.
//
#include <rpm/api.h>
#include <rpm/internal.h>

//
// Area of system data.
//
jmp_buf rpm_saved_point;
uint16_t rpm_history[RPM_CMDLINE_SIZE];

//
// Debug options.
//
static const bool debug_trace = false;

//
// Show all built-in commands.
//
static void cmd_help(int argc, char *argv[])
{
    rpm_puts("RP/M built-in commands are:\r\n");
    rpm_puts("CLEAR or CLS    Clear the console screen\r\n");
    rpm_puts("DATE            Show or change the system date\r\n");
    rpm_puts("ECHO            Copy text directly to the console output\r\n");
    rpm_puts("HELP or ?       Show all built-in commands\r\n");
    rpm_puts("REBOOT          Restart the RP/M kernel\r\n");
    rpm_puts("TIME            Set or show the current system time\r\n");
    rpm_puts("VER             Show the version of RP/M software\r\n");
    rpm_puts("EXIT            Close down the command interpreter\r\n");
    rpm_puts("\r\n");
}

//
// Clear the console screen
//
static void cmd_clear(int argc, char *argv[])
{
    rpm_puts("\33[H\33[J");
}

//
// Show or change the system date
//
static void cmd_date(int argc, char *argv[])
{
    rpm_puts("Not implemented yet\r\n");
}

//
// Copy text directly to the console output
//
static void cmd_echo(int argc, char *argv[])
{
    rpm_puts("Not implemented yet\r\n");
}

//
// Restart the RP/M kernel
//
static void cmd_reboot(int argc, char *argv[])
{
    rpm_puts("Not implemented yet\r\n");
}

//
// Set or show the current system time
//
static void cmd_time(int argc, char *argv[])
{
    rpm_puts("Not implemented yet\r\n");
}

//
// Show the version of RP/M software
//
static void cmd_ver(int argc, char *argv[])
{
    rpm_puts("Not implemented yet\r\n");
}

//
// Run internal or external command with given arguments.
//
static void run_command(int argc, char *argv[])
{
    // Table of internal commands.
    typedef struct {
        char *name;
        void (*func)(int argc, char *argv[]);
    } command_table_t;
    static command_table_t cmd_tab[] = {
        { "?",      cmd_help },
        { "clear",  cmd_clear },
        { "cls",    cmd_clear },
        { "date",   cmd_date },
        { "echo",   cmd_echo },
        { "help",   cmd_help },
        { "reboot", cmd_reboot },
        { "time",   cmd_time },
        { "ver",    cmd_ver },
        { 0,        0 },
    };

    if (debug_trace) {
        // Print command before execution.
        rpm_printf("[%d] ", argc);
        for (int i=0; i<argc; i++) {
            if (i > 0)
                rpm_putchar(' ');
            rpm_puts(argv[i]);
        }
        rpm_puts("\r\n");
    }

    // Find internal command.
    for (command_table_t *p = cmd_tab; p->name; p++) {
        // Note: command name is case insensitive.
        if (strcasecmp(p->name, argv[0]) == 0) {
            p->func(argc, argv);
            return;
        }
    }

    //TODO: Run external command.elf from filesystem.

    // No such command.
    rpm_puts(argv[0]);
    rpm_puts(": command not found\r\n");
}

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

        // Execute the command.
        run_command(argc, argv);
    }
}

#if 0
//TODO: filesystem commands: mount eject vol ls/dir cat/type cd mv/ren rm/del mkdir pwd pushd popd cp/copy rmdir more
MOUNT
EJECT
VOL             Show the volume label of a disk device
LS or DIR       List the contents of a directory
CAT or TYPE     Type the contents of a text file
CD              Change current default directory
MV or REN       Rename a file, move set of files or directory tree
RM or DEL       Delete a file or set of files
MKDIR           Create a subdirectory
PWD             Show pathname of the current default directory
PUSHD           Change to a new directory, saving the current one
POPD            Restore the directory to the last one saved with PUSHD
COPY            Copy file
CP or COPY      Copy files or directory trees to a destination
RMDIR           Delete a subdirectory
MORE            Display output in pages
CALL            Start a program, or invoke a batch file

//TODO: environment variables and commands
PATH            Set or show the search path
SET             Set or show environment variables
PROMPT          Change the command prompt

//TODO: symbolic links
MKLINK          Create a symbolic link

//TODO: scripts
CHOICE          Wait for an keypress from a selectable list
PAUSE           Suspend execution of a batch file
#endif
