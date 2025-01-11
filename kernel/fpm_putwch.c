//
// Write Unicode character to the console.
//
// Convert to UTF-8 encoding:
// 00000000.0xxxxxxx -> 0xxxxxxx
// 00000xxx.xxyyyyyy -> 110xxxxx, 10yyyyyy
// xxxxyyyy.yyzzzzzz -> 1110xxxx, 10yyyyyy, 10zzzzzz
//
#include <fpm/api.h>

void fpm_putwch(uint16_t ch)
{
    if (ch < 0x80) {
        fpm_putchar(ch);
    } else if (ch < 0x800) {
        fpm_putchar(ch >> 6 | 0xc0);
        fpm_putchar((ch & 0x3f) | 0x80);
    } else {
        fpm_putchar(ch >> 12 | 0xe0);
        fpm_putchar(((ch >> 6) & 0x3f) | 0x80);
        fpm_putchar((ch & 0x3f) | 0x80);
    }
}
