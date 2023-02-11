//
// Show or change the system date
//
#include <rpm/api.h>
#include <rpm/internal.h>

void rpm_cmd_date(int argc, char *argv[])
{
    //TODO: getopt
    static const char *dotw_name[7] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
    int year, month, day, dotw;
    rpm_get_date(&year, &month, &day, &dotw);

    rpm_printf("Current Date: %s %d/%d/%04d\r\n", dotw_name[dotw], month, day, year);
    rpm_puts("Type 'date MM/DD/YYYY' to change.\r\n");
    rpm_puts("\r\n");
}
