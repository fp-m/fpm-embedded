//
// Test rpm_editline() - command line editor.
//
#include <gtest/gtest.h>
#include <rpm/api.h>
#include "util.h"

const char *input;       // Input stream for the current test, utf-8 encoded

//
// Get Unicode character from input buffer.
//
char rpm_getchar()
{
    if (input == nullptr || *input == 0) {
        // Should not happen.
        throw std::runtime_error("No input in rpm_getchar()");
    }
    return *input++;
}

//
// Write Unicode character to output buffer.
//
void rpm_putchar(char ch)
{
    putchar(ch);
    fflush(stdout);
}

//
// Posix-compatible formatted output to string.
//
int rpm_snprintf(char *str, size_t size, const char *format, ...)
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
int rpm_printf(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    int retval = vprintf(format, args);
    va_end(args);
    fflush(stdout);
    return retval;
}
