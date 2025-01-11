//
// Write UTF-8 string to output.
//
#include <fpm/api.h>

void fpm_wputs(const uint16_t *input)
{
    while (*input) {
        fpm_putwch(*input++);
    }
}
