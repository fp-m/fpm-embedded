//
// Show or change the system date
//
#include <rpm/api.h>
#include <rpm/internal.h>

void rpm_cmd_date(int argc, char *argv[])
{
    //TODO: getopt
    int year, month, day, dotw;
    rpm_get_date(&year, &month, &day, &dotw);

    rpm_printf("Current Date is %d/%d/%04d\r\n", month, day, year);
    rpm_puts("Type 'date MM/DD/YYYY' to change.\r\n");
    rpm_puts("\r\n");
}
