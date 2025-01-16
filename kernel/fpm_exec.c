//
// Execute a command or external program.
//
#include <fpm/api.h>
#include <fpm/internal.h>
#include <fpm/fs.h>
#include <fpm/loader.h>
#include <fpm/context.h>
#include <alloca.h>

//
// Debug options.
//
static const bool debug_trace = false;

//
// Export dynamically linked routines.
//
static fpm_binding_t bindings[] = {
    { "", NULL },
    { "fpm_print_version", (void*) fpm_print_version },
    { "fpm_puts", (void*) fpm_puts },
    { "fpm_wputs", (void*) fpm_wputs },
    //TODO: other fpm_xxx() routines.
    {},
};

static const char *find_exe(const char *cmdname, char *buf)
{
    if (strchr(cmdname, '/') != NULL) {
        // Full path name or relative path name - use as is.
        return cmdname;
    }

    if (strchr(cmdname, '.') != NULL) {
        // Extension is explicitly present.
        return cmdname;
    }

    // Copy filename, append extension.
    strcpy(buf, cmdname);
    strcat(buf, ".exe");

    // Check whether file exists.
    file_info_t info;
    fs_result_t result = f_stat(buf, &info);
    if (result == FR_OK) {
        return buf;
    }

    // Look in flash:/bin/ directory.
    strcpy(buf, "flash:/bin/");
    strcat(buf, cmdname);
    strcat(buf, ".exe");

    // Check whether file exists.
    result = f_stat(buf, &info);
    if (result == FR_OK) {
        return buf;
    }

    return NULL;
}

//
// Execute internal command or external program with given arguments.
//
void fpm_exec(int argc, char *argv[])
{
    // Table of internal commands.
    typedef struct {
        const char *name;
        void (*func)(int argc, char *argv[]);
    } command_table_t;
    static const command_table_t cmd_tab[] = {
        { "?",      fpm_cmd_help },   // also HELP
        { "cat",    fpm_cmd_cat },    // also TYPE
        { "cd",     fpm_cmd_cd },     //
        { "clear",  fpm_cmd_clear },  // also CLS
        { "cls",    fpm_cmd_clear },  // also CLEAR
        { "copy",   fpm_cmd_copy },   // also CP
        { "cp",     fpm_cmd_copy },   // also COPY
        { "date",   fpm_cmd_date },   //
        { "dir",    fpm_cmd_dir },    // also LS
        { "echo",   fpm_cmd_echo },   //
        { "eject",  fpm_cmd_eject },  //
        { "erase",  fpm_cmd_remove }, // also RM
        { "format", fpm_cmd_format }, //
        { "help",   fpm_cmd_help },   // also ?
        { "ls",     fpm_cmd_dir },    // also DIR
        { "mkdir",  fpm_cmd_mkdir },  //
        { "mount",  fpm_cmd_mount },  //
        { "mv",     fpm_cmd_rename }, // also RENAME
        { "reboot", fpm_cmd_reboot }, //
        { "rename", fpm_cmd_rename }, // also MV
        { "rm",     fpm_cmd_remove }, // also ERASE
        { "rmdir",  fpm_cmd_rmdir },  //
        { "time",   fpm_cmd_time },   //
        { "type",   fpm_cmd_cat },    // also CAT
        { "ver",    fpm_cmd_ver },    //
        { "vol",    fpm_cmd_vol },    //
        { 0,        0 },
    };

    if (debug_trace) {
        // Print command before execution.
        fpm_printf("[%d] ", argc);
        for (int i=0; i<argc; i++) {
            if (i > 0)
                fpm_putchar(' ');
            fpm_puts(argv[i]);
        }
        fpm_puts("\r\n");
    }

    // Command ends with colon?
    if (argv[0][strlen(argv[0]) - 1] == ':') {
        // Switch to another drive.
        fs_result_t result = f_chdrive(argv[0]);
        if (result != FR_OK) {
            fpm_puts(f_strerror(result));
            fpm_puts("\r\n\n");
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
        fpm_puts(argv[0]);
        fpm_puts(": Command not found\r\n\n");
        return;
    }

    // Load external executable.
    fpm_context_t dynobj;
    memset(&dynobj, 0, sizeof(dynobj));
    if (!fpm_load(&dynobj, path)) {
        // Failed: error message already printed.
        fpm_puts("\r\n");
        return;
    }

    bool success = fpm_execv(&dynobj, bindings, argc, argv);
    fpm_unload(&dynobj);
    if (!success) {
        // Failed: error message already printed.
        fpm_puts("\r\n");
        return;
    }

    // External binary successfully executed.
    //TODO: save dynobj.exit_code somehow.
    fpm_puts("\r\n");
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
