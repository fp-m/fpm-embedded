#include <setjmp.h>

#ifdef __cplusplus
extern "C" {
#endif

#define RPM_VERSION "0.0"

//
// Interactive shell.
//
void rpm_shell(void);

//
// Resume on ^C.
//
extern jmp_buf rpm_saved_point; // TODO: move to the system area

//
// Program Area Descriptor (PAD): layout of the current program.
//
typedef struct {
    // Total amount of free memory.
    size_t free_size;

    // A linked list of memory gaps, sorted by address in ascending order.
    void *free_list;
} rpm_pad_t;

extern rpm_pad_t rpm_pad;

//
// Initialize region of memory for dynamic allocation.
//
void rpm_heap_init(size_t start, size_t nbytes);
void rpm_heap_print_free_list(void);

//
// Shell commands.
//
void rpm_cmd_cat(int argc, char *argv[]);
void rpm_cmd_cd(int argc, char *argv[]);
void rpm_cmd_clear(int argc, char *argv[]);
void rpm_cmd_copy(int argc, char *argv[]);
void rpm_cmd_date(int argc, char *argv[]);
void rpm_cmd_dir(int argc, char *argv[]);
void rpm_cmd_echo(int argc, char *argv[]);
void rpm_cmd_eject(int argc, char *argv[]);
void rpm_cmd_format(int argc, char *argv[]);
void rpm_cmd_help(int argc, char *argv[]);
void rpm_cmd_mkdir(int argc, char *argv[]);
void rpm_cmd_mount(int argc, char *argv[]);
void rpm_cmd_pwd(int argc, char *argv[]);
void rpm_cmd_reboot(int argc, char *argv[]);
void rpm_cmd_remove(int argc, char *argv[]);
void rpm_cmd_rename(int argc, char *argv[]);
void rpm_cmd_rmdir(int argc, char *argv[]);
void rpm_cmd_time(int argc, char *argv[]);
void rpm_cmd_ver(int argc, char *argv[]);
void rpm_cmd_vol(int argc, char *argv[]);

#ifdef __cplusplus
}
#endif
