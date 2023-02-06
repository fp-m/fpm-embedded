//
// API for RP/M, implemented with Pico SDK.
//
#include <rpm/api.h>
#include <rpm/internal.h>
#include <stdio.h>
#include "pico/stdlib.h"
//#include <stdlib.h>

uint16_t rpm_getwch()
{
    uint16_t result;

    for (;;) {
        // Make sure console is connected.
        while (!stdio_usb_connected()) {
            sleep_ms(100);
        }

        // Read one byte.
        int c1 = getchar();
        if (c1 < 0) {
            continue;
        }

        // Decode utf-8 to unicode.
        if (! (c1 & 0x80)) {
            result = c1;
        } else {
            // Read second byte.
            int c2 = getchar();
            if (c2 < 0) {
                continue;
            }
            if (! (c1 & 0x20)) {
                result = (c1 & 0x1f) << 6 | (c2 & 0x3f);
            } else {
                // Read third byte.
                int c3 = getchar();
                if (c3 < 0) {
                    continue;
                }
                result = (c1 & 0x0f) << 12 | (c2 & 0x3f) << 6 | (c3 & 0x3f);
            }
        }
        break;
    }

    // ^C - kill the process.
    if (result == '\3') {
        rpm_puts("^C\r\n");
        longjmp(rpm_saved_point, 1);
    }
    return result;
}

//
// Write Unicode character to the console.
//
// Convert to UTF-8 encoding:
// 00000000.0xxxxxxx -> 0xxxxxxx
// 00000xxx.xxyyyyyy -> 110xxxxx, 10yyyyyy
// xxxxyyyy.yyzzzzzz -> 1110xxxx, 10yyyyyy, 10zzzzzz
//
void rpm_putwch(uint16_t ch)
{
    if (ch < 0x80) {
        putchar(ch);
    } else if (ch < 0x800) {
        putchar(ch >> 6 | 0xc0);
        putchar((ch & 0x3f) | 0x80);
    } else {
        putchar(ch >> 12 | 0xe0);
        putchar(((ch >> 6) & 0x3f) | 0x80);
        putchar((ch & 0x3f) | 0x80);
    }
    fflush(stdout);
}

//TODO: int rpm_printf(const char *, ...);
//TODO: int rpm_sprintf(char *, const char *, ...);
//TODO: int rpm_snprintf(char *, size_t, const char *, ...);
//TODO: int rpm_vprintf(const char *, va_list);
//TODO: int rpm_vsprintf(char *, const char *, va_list);
//TODO: int rpm_vsnprintf(char *, size_t, const char *, va_list);

//TODO: int rpm_sscanf(const char *, const char *, ...);
//TODO: int rpm_vsscanf(const char *, const char *, va_list);
