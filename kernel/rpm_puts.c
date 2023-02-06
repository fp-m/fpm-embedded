//
// Write UTF-8 string to output.
//
#include <rpm/api.h>

void rpm_puts(const char *input)
{
    for (;;) {
        uint8_t c1 = *input++;
        if (c1 == 0)
            break;

        // Decode utf-8 to unicode.
        if (! (c1 & 0x80)) {
            rpm_putwch(c1);
        } else {
            uint8_t c2 = *input++;
            if (! (c1 & 0x20)) {
                rpm_putwch((c1 & 0x1f) << 6 | (c2 & 0x3f));
            } else {
                uint8_t c3 = *input++;
                rpm_putwch((c1 & 0x0f) << 12 | (c2 & 0x3f) << 6 | (c3 & 0x3f));
            }
        }
    }
}
