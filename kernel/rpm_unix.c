//
// API for RP/M, implemented with Posix.
//
#include <rpm/api.h>
#include <rpm/internal.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

int rpm_getchar()
{
    char ch;

    if (read(0, &ch, 1) != 1) {
        // Console closed.
        exit(-1);
    }
    if (ch == '\3') {
        // ^C - kill the process.
        rpm_puts("^C\r\n");
        longjmp(rpm_saved_point, 1);
    }
    return (uint8_t) ch;
}

void rpm_putchar(int ch)
{
    putchar(ch);
    fflush(stdout);
}

void rpm_puts(const char *str)
{
    fputs(str, stdout);
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
