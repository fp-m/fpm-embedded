#include <fpm/api.h>
#include "pico/stdlib.h"
#include "pico/aon_timer.h"

//
// Get date and time (local).
//
void fpm_get_datetime(int *year, int *month, int *day, int *dotw, int *hour, int *min, int *sec)
{
    struct tm timestamp = {};
    aon_timer_get_time_calendar(&timestamp);

    *year = 1900 + timestamp.tm_year;
    *month = 1 + timestamp.tm_mon;
    *day = timestamp.tm_mday;
    *dotw = timestamp.tm_wday;
    *hour = timestamp.tm_hour;
    *min = timestamp.tm_min;
    *sec = timestamp.tm_sec;
}

//
// Set date and time.
//
void fpm_set_datetime(int year, int month, int day, int hour, int min, int sec)
{
    const struct tm timestamp = {
        .tm_sec  = sec,
        .tm_min  = min,
        .tm_hour = hour,
        .tm_mday = day,
        .tm_mon  = month - 1,
        .tm_year = year - 1900,
        .tm_wday = fpm_get_dotw(year, month, day),
    };
    aon_timer_set_time_calendar(&timestamp);
    sleep_us(64);
}

void fpm_init_datetime()
{
    //TODO: get time/date from battery backed RTC or from filesystem
    static const struct tm build_timestamp = {
        .tm_sec  = BUILD_SEC,
        .tm_min  = BUILD_MIN,
        .tm_hour = BUILD_HOUR,
        .tm_mday = BUILD_DAY,
        .tm_mon  = BUILD_MONTH - 1,
        .tm_year = BUILD_YEAR - 1900,
        .tm_wday = BUILD_DOTW, // 0 is Sunday, so 5 is Friday
    };

    aon_timer_start_calendar(&build_timestamp);
    sleep_us(64);
}
