//
// API for RP/M.
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
// Compute length of Unicode string.
// Return the number of characters that precede the terminating NUL character.
//
size_t rpm_strwlen(const uint16_t *str);

//
// Compute length of UTF-8 string.
// Return the number of characters that precede the terminating NUL character.
//
size_t rpm_utf8len(const char *str);

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
void rpm_print_version(void);

//
// Get/set date and time.
//
void rpm_get_datetime(int *year, int *month, int *day, int *dotw, int *hour, int *min, int *sec);
void rpm_set_datetime(int year, int month, int day, int hour, int min, int sec);

//
// Compute day of the week.
//
int rpm_get_dotw(int year, int month, int day);

//
// Reboot the processor.
//
void rpm_reboot(void);

//
// Execute internal command or external program with given arguments.
//
void rpm_exec(int argc, char *argv[]);

//
// Return the current 64-bit timestamp value in microseconds.
//
uint64_t rpm_time_usec(void);

//
// Busy wait for the given 64-bit number of microseconds.
//
void rpm_delay_usec(uint64_t delay_usec);

//
// Busy wait for the given number of milliseconds.
//
void rpm_delay_msec(unsigned delay_msec);

//
// Memory allocation.
//
void *rpm_alloc(size_t nbytes);
void rpm_free(void *ptr);
void *rpm_realloc(void *ptr, size_t nbytes);
void *rpm_alloc_dirty(size_t nbytes);
void rpm_alloc_truncate(void *block, size_t nbytes);
size_t rpm_alloc_size(void *block);
size_t rpm_heap_available(void);

#ifdef __cplusplus
}
#endif
