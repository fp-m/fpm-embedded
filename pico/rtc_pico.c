#include <fpm/api.h>
#include "pico/stdlib.h"
#if HAS_RP2040_RTC
#   include "hardware/rtc.h"
#endif

//
// Get date and time (local).
//
void fpm_get_datetime(int *year, int *month, int *day, int *dotw, int *hour, int *min, int *sec)
{
#if HAS_RP2040_RTC
    datetime_t now = {};
    rtc_get_datetime(&now);

    *year = now.year;
    *month = now.month;
    *day = now.day;
    *dotw = now.dotw;
    *hour = now.hour;
    *min = now.min;
    *sec = now.sec;
#endif
}

//
// Set date and time.
//
void fpm_set_datetime(int year, int month, int day, int hour, int min, int sec)
{
#if HAS_RP2040_RTC
    datetime_t now = {};

    now.year = year;
    now.month = month;
    now.day = day;
    now.dotw = fpm_get_dotw(year, month, day);
    now.hour = hour;
    now.min = min;
    now.sec = sec;

    rtc_set_datetime(&now);
    sleep_us(64);
#endif
}
