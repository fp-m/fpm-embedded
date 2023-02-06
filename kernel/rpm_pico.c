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

//TODO: int rpm_printf(const char *, ...);
//TODO: int rpm_sprintf(char *, const char *, ...);
//TODO: int rpm_snprintf(char *, size_t, const char *, ...);
//TODO: int rpm_vprintf(const char *, va_list);
//TODO: int rpm_vsprintf(char *, const char *, va_list);
//TODO: int rpm_vsnprintf(char *, size_t, const char *, va_list);

//TODO: int rpm_sscanf(const char *, const char *, ...);
//TODO: int rpm_vsscanf(const char *, const char *, va_list);
