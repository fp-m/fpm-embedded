//
// Get Unicode character from the console.
//
// Convert from UTF-8 encoding to 16-bit value:
// 0xxxxxxx                     -> 00000000.0xxxxxxx
// 110xxxxx, 10yyyyyy           -> 00000xxx.xxyyyyyy
// 1110xxxx, 10yyyyyy, 10zzzzzz -> xxxxyyyy.yyzzzzzz
//
#include <fpm/api.h>

uint16_t fpm_getwch()
{
    // Read one byte.
    uint8_t c1 = fpm_getchar();
    if (! (c1 & 0x80)) {
        return c1;
    }

    //
    // Decode utf-8 to unicode.
    //

    // Read second byte.
    uint8_t c2 = fpm_getchar();
    if (! (c1 & 0x20)) {
        return (c1 & 0x1f) << 6 | (c2 & 0x3f);
    }

    // Read third byte.
    uint8_t c3 = fpm_getchar();
    return (c1 & 0x0f) << 12 | (c2 & 0x3f) << 6 | (c3 & 0x3f);
}
