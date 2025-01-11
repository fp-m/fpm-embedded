//
// API for FP/M, implemented with Pico SDK.
//
#include <fpm/api.h>
#include <fpm/internal.h>
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/watchdog.h"

//
// Wait for console input.
// Return ASCII keycode.
//
char fpm_getchar()
{
    int ch;

    for (;;) {
#if LIB_PICO_STDIO_USB
        // Make sure console is connected.
        while (!stdio_usb_connected()) {
            sleep_ms(100);
        }
#endif
        // Read one byte.
        ch = getchar();
        if (ch >= 0) {
            break;
        }
    }

    // ^C - kill the process.
    if (ch == '\3') {
        fpm_puts("^C\r\n");
        longjmp(fpm_saved_point, 1);
    }
    return ch;
}

//
// Write Unicode character to the console.
//
// Convert to UTF-8 encoding:
// 00000000.0xxxxxxx -> 0xxxxxxx
// 00000xxx.xxyyyyyy -> 110xxxxx, 10yyyyyy
// xxxxyyyy.yyzzzzzz -> 1110xxxx, 10yyyyyy, 10zzzzzz
//
void fpm_putchar(char ch)
{
    putchar(ch);
}

//
// Posix-compatible formatted output to console.
//
int fpm_printf(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    int retval = vprintf(format, args);
    va_end(args);
    fflush(stdout);
    return retval;
}

int fpm_vprintf(const char *format, va_list args)
{
    int retval = vprintf(format, args);
    fflush(stdout);
    return retval;
}

//
// Posix-compatible formatted output to string.
//
int fpm_snprintf(char *str, size_t size, const char *format, ...)
{
    va_list args;
    va_start(args, format);
    int retval = vsnprintf(str, size, format, args);
    va_end(args);
    return retval;
}

int fpm_vsnprintf(char *str, size_t size, const char *format , va_list args)
{
    return vsnprintf(str, size, format, args);
}

//
// Posix-compatible formatted input.
//
int fpm_sscanf(const char *str, const char *format, ...)
{
    va_list args;
    va_start(args, format);
    int retval = vsscanf(str, format, args);
    va_end(args);
    return retval;
}

int fpm_vsscanf(const char *str, const char *format, va_list args)
{
    return vsscanf(str, format, args);
}

void fpm_print_version()
{
    fpm_puts("FP/M version "FPM_VERSION"."GIT_REVCOUNT"\r\n");
    fpm_puts("Git commit "GIT_COMMIT", built on "__DATE__" at "__TIME__"\r\n");
    fpm_puts("Pico SDK version "PICO_SDK_VERSION_STRING"\r\n");
    fpm_printf("RP2040 chip revision B%d, ROM version %d\r\n", rp2040_chip_version(), rp2040_rom_version());
}

//
// Reboot the processor.
//
void fpm_reboot()
{
    watchdog_reboot(0, 0, 0);
}

//
// Return the current 64-bit timestamp value in microseconds.
//
uint64_t fpm_time_usec()
{
    return time_us_64();
}

//
// Busy wait for the given 64-bit number of microseconds.
//
void fpm_delay_usec(uint64_t microseconds)
{
     busy_wait_us(microseconds);
}

//
// Busy wait for the given number of milliseconds.
//
void fpm_delay_msec(unsigned milliseconds)
{
     busy_wait_ms(milliseconds);
}
