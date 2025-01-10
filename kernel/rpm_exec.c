//
// Execute a command or external program.
//
#include <rpm/api.h>
#include <rpm/internal.h>
#include <rpm/fs.h>
#include <rpm/loader.h>
#include <alloca.h>

//
// Debug options.
//
static const bool debug_trace = false;

//
// Export dynamically linked routines.
//
static rpm_binding_t bindings[] = {
    { "", NULL },
    { "rpm_puts", (void*) rpm_puts },
    //TODO: add all rpm_xxx routines.
    {},
};

static const char *find_exe(const char *cmdname, char *buf)
{
    //TODO: find path
    return NULL;
}

//
// Execute internal command or external program with given arguments.
//
void rpm_exec(int argc, char *argv[])
{
    // Table of internal commands.
    typedef struct {
        const char *name;
        void (*func)(int argc, char *argv[]);
    } command_table_t;
    static const command_table_t cmd_tab[] = {
        { "?",      rpm_cmd_help },   // also HELP
        { "cat",    rpm_cmd_cat },    // also TYPE
        { "cd",     rpm_cmd_cd },     //
        { "clear",  rpm_cmd_clear },  // also CLS
        { "cls",    rpm_cmd_clear },  // also CLEAR
        { "copy",   rpm_cmd_copy },   // also CP
        { "cp",     rpm_cmd_copy },   // also COPY
        { "date",   rpm_cmd_date },   //
        { "dir",    rpm_cmd_dir },    // also LS
        { "echo",   rpm_cmd_echo },   //
        { "eject",  rpm_cmd_eject },  //
        { "erase",  rpm_cmd_remove }, // also RM
        { "format", rpm_cmd_format }, //
        { "help",   rpm_cmd_help },   // also ?
        { "ls",     rpm_cmd_dir },    // also DIR
        { "mkdir",  rpm_cmd_mkdir },  //
        { "mount",  rpm_cmd_mount },  //
        { "mv",     rpm_cmd_rename }, // also RENAME
        { "reboot", rpm_cmd_reboot }, //
        { "rename", rpm_cmd_rename }, // also MV
        { "rm",     rpm_cmd_remove }, // also ERASE
        { "rmdir",  rpm_cmd_rmdir },  //
        { "time",   rpm_cmd_time },   //
        { "type",   rpm_cmd_cat },    // also CAT
        { "ver",    rpm_cmd_ver },    //
        { "vol",    rpm_cmd_vol },    //
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

    // Command ends with colon?
    if (argv[0][strlen(argv[0]) - 1] == ':') {
        // Switch to another drive.
        fs_result_t result = f_chdrive(argv[0]);
        if (result != FR_OK) {
            rpm_puts(f_strerror(result));
            rpm_puts("\r\n\n");
        }
        return;
    }

    // Find internal command.
    for (const command_table_t *p = cmd_tab; p->name; p++) {
        // Note: command name is case insensitive.
        if (strcasecmp(p->name, argv[0]) == 0) {
            p->func(argc, argv);
            return;
        }
    }

    // Find file path of external command.
    // Try .exe extension, search flash:/bin directory.
    char *buf = alloca(strlen(argv[0]) + sizeof(".exe") + sizeof("flash:/bin"));
    const char *path = find_exe(argv[0], buf);
    if (!path) {
        rpm_puts(argv[0]);
        rpm_puts(": Command not found\r\n\n");
        return;
    }

    // Load external executable.
    rpm_executable_t dynobj;
    memset(&dynobj, 0, sizeof(dynobj));
    if (!rpm_load(&dynobj, path)) {
        // Failed: error message already printed.
        return;
    }

    bool success = rpm_execv(&dynobj, bindings, argc, argv);
    rpm_unload(&dynobj);
    if (!success) {
        // Failed: error message already printed.
        return;
    }

    // External binary successfully executed.
    //TODO: save dynobj.exit_code somehow.
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
