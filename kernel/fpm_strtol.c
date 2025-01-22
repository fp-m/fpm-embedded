// Cannot include <fpm/api.h> here.
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>

//
// Convert a string value to a long integer.
// Return true when value is out of range.
//
bool fpm_strtol(long *output, const char *str, char **endptr, int base)
{
    errno = 0;
    *output = strtol(str, endptr, base);
    return (errno != 0);
}

//
// Convert a string value to an unsigned long integer.
// Return true when value is out of range.
//
bool fpm_strtoul(unsigned long *output, const char *str, char **endptr, int base)
{
    errno = 0;
    *output = strtoul(str, endptr, base);
    return (errno != 0);
}

//
// Convert string to floating point.
// Return true when value is out of range.
//
bool fpm_strtod(double *output, const char *str, char **endptr)
{
    errno = 0;
    *output = strtod(str, endptr);
    return (errno != 0);
}
