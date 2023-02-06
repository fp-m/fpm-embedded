//
// Compute the length of the Unicode string s.
// Return the number of characters that precede the terminating NUL character.
//
#include <rpm/api.h>

size_t rpm_strwlen(const uint16_t *input)
{
    const uint16_t *s;

    for (s = input; *s; ++s)
        continue;

    return s - input;
}
