#include <fpm/api.h>
#include <stdlib.h>
#include <errno.h>

//
// Convert string to a long value.
// Return true when value is out of range.
//
bool fpm_strtol(long *output, const char *str, char **endptr, int base)
{
    errno = 0;
    *output = strtol(str, endptr, base);
    return (errno != 0);
}
