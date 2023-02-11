//
// Set or show the current system time
//
#include <rpm/api.h>
#include <rpm/internal.h>

void rpm_cmd_time(int argc, char *argv[])
{
    //TODO: getopt
    int hour, min, sec;
    rpm_get_time(&hour, &min, &sec);

    // Convert 24-hour clock time to 12-hour clock.
    char am_pm = (hour <= 11) ? 'A' : 'P';
    if (hour == 0) {
        hour += 12;
    } else if (hour > 12) {
        hour -= 12;
    }

    rpm_printf("Current Time: %02d:%02d:%02d %cM\r\n", hour, min, sec, am_pm);
    rpm_puts("Type 'time hh/mm/ss' to change.\r\n");
    rpm_puts("\r\n");
}
