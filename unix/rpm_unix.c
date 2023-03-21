//
// API for RP/M, implemented with Posix.
//
#include <rpm/api.h>
#include <rpm/internal.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/utsname.h>
#include <sys/time.h>

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

int rpm_vprintf(const char *format, va_list args)
{
    int retval = vprintf(format, args);
    fflush(stdout);
    return retval;
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

int rpm_vsnprintf(char *str, size_t size, const char *format , va_list args)
{
    return vsnprintf(str, size, format, args);
}

//
// Posix-compatible formatted input.
//
int rpm_sscanf(const char *str, const char *format, ...)
{
    va_list args;
    va_start(args, format);
    int retval = vsscanf(str, format, args);
    va_end(args);
    return retval;
}

int rpm_vsscanf(const char *str, const char *format, va_list args)
{
    return vsscanf(str, format, args);
}

void rpm_print_version()
{
    rpm_puts("RP/M version "RPM_VERSION"."GIT_REVCOUNT"\r\n");
    rpm_puts("Git commit "GIT_COMMIT", built on "__DATE__" at "__TIME__"\r\n");

    struct utsname u;
    if (uname(&u) == 0) {
        rpm_printf("Unix %s %s version %s\r\n", u.sysname, u.machine, u.release);
    }
}

//
// Get date and time (local).
//
void rpm_get_datetime(int *year, int *month, int *day, int *dotw, int *hour, int *min, int *sec)
{
    struct timeval tv;
    gettimeofday(&tv, 0);

    time_t now = tv.tv_sec;
    struct tm *info = localtime(&now);

    *year = 1900 + info->tm_year;
    *month = 1 + info->tm_mon;
    *day = info->tm_mday;
    *dotw = info->tm_wday;
    *hour = info->tm_hour;
    *min = info->tm_min;
    *sec = info->tm_sec;
}

//
// Set date and time.
//
void rpm_set_datetime(int year, int month, int day, int hour, int min, int sec)
{
    // Ignore.
}

//
// Reboot the processor.
//
void rpm_reboot()
{
    rpm_puts("Cannot reboot Unix, sorry.\r\n\r\n");
}

//
// Return the current 64-bit timestamp value in microseconds.
//
uint64_t rpm_time_usec()
{
    struct timeval t;

    gettimeofday(&t, 0);
    return t.tv_sec * 1000000ULL + t.tv_usec;
}

//
// Busy wait for the given 64-bit number of microseconds.
//
void rpm_delay_usec(uint64_t microseconds)
{
     usleep(microseconds);
}

//
// Busy wait for the given number of milliseconds.
//
void rpm_delay_msec(unsigned milliseconds)
{
     usleep(milliseconds * 1000ULL);
}
