//
// Show or change the system date
//
#include <rpm/api.h>
#include <rpm/getopt.h>
#include <rpm/internal.h>

static void set_date(const char *str)
{
    int mm = 0, dd = 0, yyyy = 0;
    if (rpm_sscanf(str, "%d/%d/%d", &mm, &dd, &yyyy) != 3 ||
        mm < 1 || mm > 12 ||
        dd < 1 || dd > 31 ||
        yyyy < 2000 || yyyy > 9999) {
        rpm_puts("The specified date is not correct.\r\n"
                 "\n");
        return;
    }

    // Set date.
    rpm_set_date(yyyy, mm, dd);
    rpm_puts("\r\n");
}

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
            set_date(opt.arg);
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
