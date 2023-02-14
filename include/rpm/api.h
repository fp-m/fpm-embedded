//
// API for RP/M.
//
#include <stdarg.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>

//
// Size of command line in Unicode characters.
//
#define RPM_CMDLINE_SIZE        128

//
// Unicode symbols.
//
#define RPM_LEFTWARDS_ARROW     0x2190  // ←
#define RPM_UPWARDS_ARROW       0x2191  // ↑
#define RPM_RIGHTWARDS_ARROW    0x2192  // →
#define RPM_DOWNWARDS_ARROW     0x2193  // ↓
#define RPM_LEFTWARDS_TO_BAR    0x21E4  // ⇤
#define RPM_RIGHTWARDS_TO_BAR   0x21E5  // ⇥
#define RPM_DELETE_KEY          0x2421  // ␡

//
// Last command line.
//
extern uint16_t rpm_history[RPM_CMDLINE_SIZE];

//
// Output to the console (USB or Uart).
//
void rpm_putwch(uint16_t);
void rpm_putchar(char);
void rpm_puts(const char *);

int rpm_printf(const char *, ...);
int rpm_sprintf(char *, const char *, ...);
int rpm_snprintf(char *, size_t, const char *, ...);
int rpm_vprintf(const char *, va_list);
int rpm_vsprintf(char *, const char *, va_list);
int rpm_vsnprintf(char *, size_t, const char *, va_list);

int rpm_sscanf(const char *, const char *, ...);
int rpm_vsscanf(const char *, const char *, va_list);

//
// Get a Unicode character from console.
// Decode escape sequences.
//
uint16_t rpm_getkey(void);

//
// Wait for a keycode character.
// Returns:
// - Unicode symbol, 16 bits
//
uint16_t rpm_getwch(void);

//
// Wait for a keycode character.
// Returns:
// - ASCII keycode
//
char rpm_getchar(void);

//
// Write the Unicode string to the console.
//
void rpm_wputs(const uint16_t *);

//
// The main line edit function
// Parameters:
// - buffer: Pointer to the line edit buffer
// - buffer_length: Size of the buffer in bytes
// - clear: Set to false to not clear, true to clear on entry
// Returns:
// - The exit key pressed (ESC or CR)
//
int rpm_editline(uint16_t *buffer, unsigned buffer_length, bool clear,
                 const char *prompt, uint16_t *history);

//
// Parse a command line and split it into tokens (in place).
// Return NULL on success.
// Fill an argument vector.
// On error, return a message.
//
const char *rpm_tokenize(char *argv[], int *argc, char *cmd_line);

//
// Compute the length of the Unicode string s.
// Return the number of characters that precede the terminating NUL character.
//
size_t rpm_strwlen(const uint16_t *s);

//
// Size-bounded string copying.
//
size_t rpm_strlcpy(char *dst, const char *src, size_t nitems);
size_t rpm_strlcpy_from_utf8(uint16_t *dst, const char *src, size_t nitems);
size_t rpm_strlcpy_to_utf8(char *dst, const uint16_t *src, size_t nitems);
size_t rpm_strlcpy_unicode(uint16_t *dst, const uint16_t *src, size_t nitems);

//
// Print RP/M version.
//
void rpm_print_version();

//
// Get date and time.
//
void rpm_get_date(int *year, int *month, int *day, int *dotw);
void rpm_get_time(int *hour, int *min, int *sec);

//
// Reboot the processor.
//
void rpm_reboot(void);

//
// Get long options from command line argument list.
//
//
enum {                    // For has_arg:
    RPM_NO_ARG = 0,       // no argument to the option is expected
    RPM_REQUIRED_ARG = 1, // an argument to the option is required
    RPM_OPTIONAL_ARG = 2, // an argument to the option may be presented
};
struct rpm_option {
    const char *name;   // The name of the long option.
    int has_arg;        // One of the above enums.
    int *flag;          // Determines if rpm_getopt() returns a value for a long option.
                        // If it is non-NULL, 0 is returned as a function value and
                        // the value of val is stored in the area pointed to by flag.
                        // Otherwise, val is returned.
    int val;            // Determines the value to return if flag is NULL.

};
struct rpm_opt {
    int opt;        // Current option
    int long_index; // Index of long option
    char *arg;      // Current argument
    int ind;        // Index of next argv
    int silent;     // Suppress error messages
    int where;      // Offset inside current argument
};
int rpm_getopt(int argc, char *const *argv, const char *optstring,
               const struct rpm_option *longopts, struct rpm_opt *opt);

//TODO: sysvar_time

#if 0 // TODO
; MOS high level functions
;
mos_load:		EQU	01h
mos_save:		EQU	02h
mos_cd:			EQU	03h
mos_dir:		EQU	04h
mos_del:		EQU	05h
mos_ren:		EQU	06h
mos_mkdir:		EQU	07h
mos_sysvars:		EQU	08h
mos_fopen:		EQU	0Ah
mos_fclose:		EQU	0Bh
mos_fgetc:		EQU	0Ch
mos_fputc:		EQU	0Dh
mos_feof:		EQU	0Eh
mos_getError:		EQU	0Fh
mos_oscli:		EQU	10h

; FatFS file access functions
;
ffs_fopen:		EQU	80h
ffs_fclose:		EQU	81h
ffs_fread:		EQU	82h
ffs_fwrite:		EQU	83h
ffs_fseek:		EQU	84h
ffs_ftruncate:		EQU	85h
ffs_fsync:		EQU	86h
ffs_fforward:		EQU	87h
ffs_fexpand:		EQU	88h
ffs_fgets:		EQU	89h
ffs_fputc:		EQU	8Ah
ffs_fputs:		EQU	8Bh
ffs_fprintf:		EQU	8Ch
ffs_ftell:		EQU	8Dh
ffs_feof:		EQU	8Eh
ffs_fsize:		EQU	8Fh
ffs_ferror:		EQU	90h

; FatFS directory access functions
;
ffs_dopen:		EQU	91h
ffs_dclose:		EQU	92h
ffs_dread:		EQU	93h
ffs_dfindfirst:		EQU	94h
ffs_dfindnext:		EQU	95h

; FatFS file and directory management functions
;
ffs_stat:		EQU	96h
ffs_unlink:		EQU	97h
ffs_rename:		EQU	98h
ffs_chmod:		EQU	99h
ffs_utime:		EQU	9Ah
ffs_mkdir:		EQU	9Bh
ffs_chdir:		EQU	9Ch
ffs_chdrive:		EQU	9Dh
ffs_getcwd:		EQU	9Eh

; FatFS volume management and system configuration functions
;
ffs_mount:		EQU	9Fh
ffs_mkfs:		EQU	A0h
ffs_fdisk		EQU	A1h
ffs_getfree:		EQU	A2h
ffs_getlabel:		EQU	A3h
ffs_setlabel:		EQU	A4h
ffs_setcp:		EQU	A5h

; File access modes
;
fa_read:		EQU	01h
fa_write:		EQU	02h
fa_open_existing:	EQU	00h
fa_create_new:		EQU	04h
fa_create_always:	EQU	08h
fa_open_always:		EQU	10h
fa_open_append:		EQU	30h

#endif
