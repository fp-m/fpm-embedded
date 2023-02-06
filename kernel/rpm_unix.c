//
// API for RP/M, implemented with Posix.
//
#include <rpm/api.h>
#include <rpm/internal.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

//
// Read Unicode character to the console.
//
uint16_t rpm_getwch()
{
    uint16_t result;

    // Read one byte.
    uint8_t c1;
    if (read(0, (char*)&c1, 1) != 1) {
console_closed:
        exit(-1);
    }

    // Decode utf-8 to unicode.
    if (! (c1 & 0x80)) {
        result = c1;
    } else {
        // Read second byte.
        uint8_t c2;
        if (read(0, (char*)&c2, 1) != 1) {
            goto console_closed;
        }

        if (! (c1 & 0x20)) {
            result = (c1 & 0x1f) << 6 | (c2 & 0x3f);
        } else {
            // Read third byte.
            uint8_t c3;
            if (read(0, (char*)&c3, 1) != 1) {
                goto console_closed;
            }
            result = (c1 & 0x0f) << 12 | (c2 & 0x3f) << 6 | (c3 & 0x3f);
        }
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
