//
// API for RP/M, implemented with Posix.
//
#include <rpm/api.h>
#include <rpm/internal.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

//
// Read byte from the console.
//
char rpm_getchar()
{
    char ch;
    if (read(0, &ch, 1) != 1) {
        // Console closed.
        exit(-1);
    }

    // ^C - kill the process.
    if (ch == '\3') {
        rpm_puts("^C\r\n");
        longjmp(rpm_saved_point, 1);
    }
    return ch;
}

//
// Write byte to the console.
//
void rpm_putchar(char ch)
{
    putchar(ch);
    fflush(stdout);
}

//TODO: int rpm_printf(const char *, ...);
//TODO: int rpm_sprintf(char *, const char *, ...);
//TODO: int rpm_snprintf(char *, size_t, const char *, ...);
//TODO: int rpm_vprintf(const char *, va_list);
//TODO: int rpm_vsprintf(char *, const char *, va_list);
//TODO: int rpm_vsnprintf(char *, size_t, const char *, va_list);

//TODO: int rpm_sscanf(const char *, const char *, ...);
//TODO: int rpm_vsscanf(const char *, const char *, va_list);
