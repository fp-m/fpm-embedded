//
// Test fpm_editline() - command line editor.
//
#include <gtest/gtest.h>
#include <fpm/api.h>
#include "util.h"

const char *input;       // Input stream for the current test, utf-8 encoded

//
// Get Unicode character from input buffer.
//
char fpm_getchar()
{
    if (input == nullptr || *input == 0) {
        // Should not happen.
        throw std::runtime_error("No input in fpm_getchar()");
    }
    return *input++;
}

//
// Write Unicode character to output buffer.
//
void fpm_putchar(char ch)
{
    putchar(ch);
    fflush(stdout);
}

//
// Posix-compatible formatted output to string.
//
int fpm_snprintf(char *str, size_t size, const char *format, ...)
{
    va_list args;
    va_start(args, format);
    int retval = vsnprintf(str, size, format, args);
    va_end(args);
    return retval;
}

//
// Posix-compatible formatted output to console.
//
int fpm_printf(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    int retval = vprintf(format, args);
    va_end(args);
    fflush(stdout);
    return retval;
}
