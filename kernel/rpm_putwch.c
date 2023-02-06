//
// Write Unicode character to the console.
//
// Convert to UTF-8 encoding:
// 00000000.0xxxxxxx -> 0xxxxxxx
// 00000xxx.xxyyyyyy -> 110xxxxx, 10yyyyyy
// xxxxyyyy.yyzzzzzz -> 1110xxxx, 10yyyyyy, 10zzzzzz
//
#include <rpm/api.h>

void rpm_putwch(uint16_t ch)
{
    if (ch < 0x80) {
        rpm_putchar(ch);
    } else if (ch < 0x800) {
        rpm_putchar(ch >> 6 | 0xc0);
        rpm_putchar((ch & 0x3f) | 0x80);
    } else {
        rpm_putchar(ch >> 12 | 0xe0);
        rpm_putchar(((ch >> 6) & 0x3f) | 0x80);
        rpm_putchar((ch & 0x3f) | 0x80);
    }
}
