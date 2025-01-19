//
// Execute a command or external program.
//
#include <fpm/api.h>
#include <fpm/internal.h>
#include <fpm/fs.h>
#include <fpm/loader.h>
#include <fpm/context.h>
#include <fpm/getopt.h>
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

    // Kernel routines.
    { "fpm_alloc", (void*) fpm_alloc },
    { "fpm_alloc_dirty", (void*) fpm_alloc_dirty },
    { "fpm_atof", (void*) fpm_atof },
    { "fpm_delay_msec", (void*) fpm_delay_msec },
    { "fpm_delay_usec", (void*) fpm_delay_usec },
    { "fpm_editline", (void*) fpm_editline },
    { "fpm_exec", (void*) fpm_exec },
    { "fpm_free", (void*) fpm_free },
    { "fpm_get_datetime", (void*) fpm_get_datetime },
    { "fpm_get_dotw", (void*) fpm_get_dotw },
    { "fpm_getchar", (void*) fpm_getchar },
    { "fpm_getkey", (void*) fpm_getkey },
    { "fpm_getopt", (void*) fpm_getopt },
    { "fpm_getwch", (void*) fpm_getwch },
    { "fpm_heap_available", (void*) fpm_heap_available },
    { "fpm_print_version", (void*) fpm_print_version },
    { "fpm_printf", (void*) fpm_printf },
    { "fpm_putchar", (void*) fpm_putchar },
    { "fpm_puts", (void*) fpm_puts },
    { "fpm_putwch", (void*) fpm_putwch },
    { "fpm_realloc", (void*) fpm_realloc },
    { "fpm_reboot", (void*) fpm_reboot },
    { "fpm_set_datetime", (void*) fpm_set_datetime },
    { "fpm_shell", (void*) fpm_shell },
    { "fpm_sizeof", (void*) fpm_sizeof },
    { "fpm_snprintf", (void*) fpm_snprintf },
    { "fpm_sscanf", (void*) fpm_sscanf },
    { "fpm_strlcpy", (void*) fpm_strlcpy },
    { "fpm_strlcpy_from_utf8", (void*) fpm_strlcpy_from_utf8 },
    { "fpm_strlcpy_to_utf8", (void*) fpm_strlcpy_to_utf8 },
    { "fpm_strlcpy_unicode", (void*) fpm_strlcpy_unicode },
    { "fpm_strtol", (void*) fpm_strtol },
    { "fpm_strwlen", (void*) fpm_strwlen },
    { "fpm_time_usec", (void*) fpm_time_usec },
    { "fpm_tokenize", (void*) fpm_tokenize },
    { "fpm_truncate", (void*) fpm_truncate },
    { "fpm_utf8len", (void*) fpm_utf8len },
    { "fpm_vprintf", (void*) fpm_vprintf },
    { "fpm_vsnprintf", (void*) fpm_vsnprintf },
    { "fpm_vsscanf", (void*) fpm_vsscanf },
    { "fpm_wputs", (void*) fpm_wputs },

    // FIlesystem routines.
    { "f_chdir", (void*) f_chdir },
    { "f_chdrive", (void*) f_chdrive },
    { "f_chmod", (void*) f_chmod },
    { "f_close", (void*) f_close },
    { "f_closedir", (void*) f_closedir },
    { "f_eof", (void*) f_eof },
    { "f_error", (void*) f_error },
    { "f_expand", (void*) f_expand },
    { "f_findfirst", (void*) f_findfirst },
    { "f_findnext", (void*) f_findnext },
    { "f_forward", (void*) f_forward },
    { "f_getcwd", (void*) f_getcwd },
    { "f_getdrive", (void*) f_getdrive },
    { "f_getlabel", (void*) f_getlabel },
    { "f_gets", (void*) f_gets },
    { "f_lseek", (void*) f_lseek },
    { "f_mkdir", (void*) f_mkdir },
    { "f_mkfs", (void*) f_mkfs },
    { "f_mount", (void*) f_mount },
    { "f_open", (void*) f_open },
    { "f_opendir", (void*) f_opendir },
    { "f_printf", (void*) f_printf },
    { "f_putc", (void*) f_putc },
    { "f_puts", (void*) f_puts },
    { "f_read", (void*) f_read },
    { "f_readdir", (void*) f_readdir },
    { "f_rename", (void*) f_rename },
    { "f_setlabel", (void*) f_setlabel },
    { "f_size", (void*) f_size },
    { "f_sizeof_directory_t", (void*) f_sizeof_directory_t },
    { "f_sizeof_file_t", (void*) f_sizeof_file_t },
    { "f_stat", (void*) f_stat },
    { "f_statfs", (void*) f_statfs },
    { "f_strerror", (void*) f_strerror },
    { "f_sync", (void*) f_sync },
    { "f_tell", (void*) f_tell },
    { "f_truncate", (void*) f_truncate },
    { "f_unlink", (void*) f_unlink },
    { "f_unmount", (void*) f_unmount },
    { "f_utime", (void*) f_utime },
    { "f_write", (void*) f_write },

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

    // Allocate program context.
    fpm_context_t ctx;
    memset(&ctx, 0, sizeof(ctx));
    if (!fpm_load(&ctx, path)) {
        // Failed: error message already printed.
        fpm_puts("\r\n");
        return;
    }
    fpm_context_push(&ctx);

    // Load external executable.
    bool success = fpm_invoke(&ctx, bindings, argc, argv);
    fpm_unload(&ctx);
    fpm_context_pop();

    if (!success) {
        // Failed: error message already printed.
        fpm_puts("\r\n");
        return;
    }

    // External binary successfully executed.
    //TODO: save ctx.exit_code somehow.
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
