//
// Write UTF-8 string to output.
//
#include <rpm/api.h>

void rpm_wputs(const uint16_t *input)
{
    while (*input) {
        rpm_putwch(*input++);
    }
}
