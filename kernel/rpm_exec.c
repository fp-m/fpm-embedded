//
// Execute a command or external program.
//
#include <rpm/api.h>
#include <rpm/internal.h>

//
// Debug options.
//
static const bool debug_trace = false;

//
// Execute internal command or external program with given arguments.
//
void rpm_exec(int argc, char *argv[])
{
    // Table of internal commands.
    typedef struct {
        char *name;
        void (*func)(int argc, char *argv[]);
    } command_table_t;
    static const command_table_t cmd_tab[] = {
        { "?",      rpm_cmd_help },
//TODO: { "cat",    rpm_cmd_cat },
//TODO: { "cd",     rpm_cmd_cd },
        { "clear",  rpm_cmd_clear },
        { "cls",    rpm_cmd_clear },
//TODO: { "copy",   rpm_cmd_copy },
//TODO: { "cp",     rpm_cmd_copy },
        { "date",   rpm_cmd_date },
//TODO: { "dir",    rpm_cmd_dir },
        { "echo",   rpm_cmd_echo },
//TODO: { "eject",  rpm_cmd_eject },
//TODO: { "erase",  rpm_cmd_remove },
        { "help",   rpm_cmd_help },
//TODO: { "ls",     rpm_cmd_dir },
//TODO: { "mkdir",  rpm_cmd_mkdir },
//TODO: { "more",   rpm_cmd_more },
//TODO: { "mount",  rpm_cmd_mount },
//TODO: { "mv",     rpm_cmd_rename },
//TODO: { "popd",   rpm_cmd_popd },
//TODO: { "pushd",  rpm_cmd_pushd },
//TODO: { "pwd",    rpm_cmd_pwd },
        { "reboot", rpm_cmd_reboot },
//TODO: { "rename", rpm_cmd_rename },
//TODO: { "rm",     rpm_cmd_remove },
//TODO: { "rmdir",  rpm_cmd_rmdir },
        { "time",   rpm_cmd_time },
//TODO: { "type",   rpm_cmd_cat },
        { "ver",    rpm_cmd_ver },
//TODO: { "vol",    rpm_cmd_vol },
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
    for (const command_table_t *p = cmd_tab; p->name; p++) {
        // Note: command name is case insensitive.
        if (strcasecmp(p->name, argv[0]) == 0) {
            p->func(argc, argv);
            return;
        }
    }

    //TODO: Run external command.elf from filesystem.

    // No such command.
    rpm_puts(argv[0]);
    rpm_puts(": Command not found\r\n\n");
}

#if 0
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
