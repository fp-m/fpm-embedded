//
// Show or change the system date
//
#include <rpm/api.h>
#include <rpm/internal.h>

void rpm_cmd_date(int argc, char *argv[])
{
    static const struct rpm_option long_opts[] = {
        { "help", RPM_NO_ARG, NULL, 'h' },
        {},
    };
    struct rpm_opt opt = {};
    bool terse = false;

    while (rpm_getopt(argc, argv, "ht", long_opts, &opt) >= 0) {
        switch (opt.ret) {
        case 1:
            //TODO: set the date: opt.arg is MM/DD/YYYY
            rpm_puts("Not implemented yet.\r\n"
                     "\n");
            return;

        case '?':
            // Unknown option: message already printed.
            rpm_puts("\r\n");
            return;

        case 't':
            terse = true;
            continue;

        case 'h':
            rpm_puts("Usage:\r\n"
                     "    date [-t]\r\n"
                     "    date MM/DD/YYYY\r\n"
                     "\n"
                     "    -t            Only display date\r\n"
                     "    MM/DD/YYYY    New date to set\r\n"
                     "\n");
            return;
        }
    }

    // Display current date.
    static const char *dotw_name[7] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
    int year, month, day, dotw;
    rpm_get_date(&year, &month, &day, &dotw);

    if (terse) {
        rpm_printf("%02d/%02d/%04d\r\n", month, day, year);
    } else {
        rpm_printf("Current Date: %s %d/%d/%04d\r\n", dotw_name[dotw], month, day, year);
        rpm_puts("Type 'date MM/DD/YYYY' to change.\r\n");
    }
    rpm_puts("\r\n");
}
