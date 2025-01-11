//
// Compute the length of the Unicode string s.
// Return the number of characters that precede the terminating NUL character.
//
#include <fpm/api.h>

size_t fpm_strwlen(const uint16_t *input)
{
    const uint16_t *s;

    for (s = input; *s; ++s)
        continue;

    return s - input;
}

size_t fpm_utf8len(const char *input)
{
    size_t count = 0;

    while (*input) {
        count += ((*input++ & 0xc0) != 0x80);
    }
    return count;
}
