#include <fpm/api.h>
#include "pico/stdlib.h"
#include "hardware/rtc.h"

//
// Get date and time (local).
//
void fpm_get_datetime(int *year, int *month, int *day, int *dotw, int *hour, int *min, int *sec)
{
    datetime_t now;
    rtc_get_datetime(&now);

    *year = now.year;
    *month = now.month;
    *day = now.day;
    *dotw = now.dotw;
    *hour = now.hour;
    *min = now.min;
    *sec = now.sec;
}

//
// Set date and time.
//
void fpm_set_datetime(int year, int month, int day, int hour, int min, int sec)
{
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
}
