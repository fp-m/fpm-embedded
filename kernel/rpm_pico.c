//
// API for RP/M, implemented with Pico SDK.
//
#include <rpm/api.h>
#include <rpm/internal.h>
#include <stdio.h>
#include "pico/stdlib.h"

//
// Wait for console input.
// Return ASCII keycode.
//
char rpm_getchar()
{
    int ch;

    for (;;) {
        // Make sure console is connected.
        while (!stdio_usb_connected()) {
            sleep_ms(100);
        }

        // Read one byte.
        ch = getchar();
        if (ch >= 0) {
            break;
        }
    }

    // ^C - kill the process.
    if (ch == '\3') {
        rpm_puts("^C\r\n");
        longjmp(rpm_saved_point, 1);
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
void rpm_putchar(char ch)
{
    putchar(ch);
}

//
// Posix-compatible formatted output to console.
//
int rpm_printf(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    int retval = vprintf(format, args);
    va_end(args);
    fflush(stdout);
    return retval;
}

int rpm_vprintf(const char *format, va_list args)
{
    int retval = vprintf(format, args);
    fflush(stdout);
    return retval;
}

//
// Posix-compatible formatted output to string.
//
int rpm_snprintf(char *str, size_t size, const char *format, ...)
{
    va_list args;
    va_start(args, format);
    int retval = vsnprintf(str, size, format, args);
    va_end(args);
    return retval;
}

int rpm_vsnprintf(char *str, size_t size, const char *format , va_list args)
{
    return vsnprintf(str, size, format, args);
}

//
// Posix-compatible formatted input.
//
int rpm_sscanf(const char *str, const char *format, ...)
{
    va_list args;
    va_start(args, format);
    int retval = vsscanf(str, format, args);
    va_end(args);
    return retval;
}

int rpm_vsscanf(const char *str, const char *format, va_list args)
{
    return vsscanf(str, format, args);
}
