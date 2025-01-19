//
// API for FP/M.
//
#ifdef __cplusplus
extern "C" {
#endif

#include <stdarg.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>

//
// Size of command line in Unicode characters.
//
#define FPM_CMDLINE_SIZE        128

//
// Unicode symbols.
//
#define FPM_LEFTWARDS_ARROW     0x2190  // ←
#define FPM_UPWARDS_ARROW       0x2191  // ↑
#define FPM_RIGHTWARDS_ARROW    0x2192  // →
#define FPM_DOWNWARDS_ARROW     0x2193  // ↓
#define FPM_LEFTWARDS_TO_BAR    0x21E4  // ⇤
#define FPM_RIGHTWARDS_TO_BAR   0x21E5  // ⇥
#define FPM_DELETE_KEY          0x2421  // ␡

//
// Last command line.
//
extern uint16_t fpm_history[FPM_CMDLINE_SIZE];

//
// Output to the console (USB or Uart).
//
void fpm_putwch(uint16_t);
void fpm_putchar(char);
void fpm_puts(const char *);

int fpm_printf(const char *, ...);
int fpm_snprintf(char *, size_t, const char *, ...);
int fpm_vprintf(const char *, va_list);
int fpm_vsnprintf(char *, size_t, const char *, va_list);

int fpm_sscanf(const char *, const char *, ...);
int fpm_vsscanf(const char *, const char *, va_list);

//
// Get a Unicode character from console.
// Decode escape sequences.
//
uint16_t fpm_getkey(void);

//
// Wait for a keycode character.
// Returns:
// - Unicode symbol, 16 bits
//
uint16_t fpm_getwch(void);

//
// Wait for a keycode character.
// Returns:
// - ASCII keycode
//
char fpm_getchar(void);

//
// Write the Unicode string to the console.
//
void fpm_wputs(const uint16_t *);

//
// The main line edit function
// Parameters:
// - buffer: Pointer to the line edit buffer
// - buffer_length: Size of the buffer in bytes
// - clear: Set to false to not clear, true to clear on entry
// Returns:
// - The exit key pressed (ESC or CR)
//
int fpm_editline(uint16_t *buffer, unsigned buffer_length, bool clear,
                 const char *prompt, uint16_t *history);

//
// Parse a command line and split it into tokens (in place).
// Return NULL on success.
// Fill an argument vector.
// On error, return a message.
//
const char *fpm_tokenize(char *argv[], int *argc, char *cmd_line);

//
// Compute length of Unicode string.
// Return the number of characters that precede the terminating NUL character.
//
size_t fpm_strwlen(const uint16_t *str);

//
// Compute length of UTF-8 string.
// Return the number of characters that precede the terminating NUL character.
//
size_t fpm_utf8len(const char *str);

//
// Size-bounded string copying.
//
size_t fpm_strlcpy(char *dst, const char *src, size_t nitems);
size_t fpm_strlcpy_from_utf8(uint16_t *dst, const char *src, size_t nitems);
size_t fpm_strlcpy_to_utf8(char *dst, const uint16_t *src, size_t nitems);
size_t fpm_strlcpy_unicode(uint16_t *dst, const uint16_t *src, size_t nitems);

//
// Convert string to number.
// Return true when value is out of range.
//
bool fpm_strtol(long *output, const char *str, char **endptr, int base);
bool fpm_strtod(double *output, const char *str, char **endptr);

//
// Print FP/M version.
//
void fpm_print_version(void);

//
// Get/set date and time.
//
void fpm_get_datetime(int *year, int *month, int *day, int *dotw, int *hour, int *min, int *sec);
void fpm_set_datetime(int year, int month, int day, int hour, int min, int sec);

//
// Compute day of the week.
//
int fpm_get_dotw(int year, int month, int day);

//
// Reboot the processor.
//
void fpm_reboot(void);

//
// Execute internal command or external program with given arguments.
//
void fpm_exec(int argc, char *argv[]);

//
// Return the current 64-bit timestamp value in microseconds.
//
uint64_t fpm_time_usec(void);

//
// Busy wait for the given 64-bit number of microseconds.
//
void fpm_delay_usec(uint64_t delay_usec);

//
// Busy wait for the given number of milliseconds.
//
void fpm_delay_msec(unsigned delay_msec);

//
// Memory allocation.
//
void *fpm_alloc(size_t nbytes);
void fpm_free(void *ptr);
void *fpm_realloc(void *ptr, size_t nbytes);
void *fpm_alloc_dirty(size_t nbytes);
void fpm_truncate(void *ptr, size_t nbytes);
size_t fpm_sizeof(void *ptr);
size_t fpm_heap_available(void);
size_t fpm_stack_available(void);

//
// Interactive shell.
//
void fpm_shell(void);

#ifdef __cplusplus
}
#endif
