//
// Show or change the system date
//
#include <fpm/api.h>
#include <fpm/getopt.h>
#include <fpm/internal.h>

static void set_date(const char *str)
{
    int year, month, day, dotw, hour, min, sec;
    fpm_get_datetime(&year, &month, &day, &dotw, &hour, &min, &sec);

    if (fpm_sscanf(str, "%d/%d/%d", &month, &day, &year) != 3 ||
        month < 1 || month > 12 ||
        day < 1 || day > 31 ||
        year < 2000 || year > 9999) {
        fpm_puts("The specified date is not correct.\r\n"
                 "\n");
        return;
    }

    // Set date.
    fpm_set_datetime(year, month, day, hour, min, sec);
    fpm_puts("\r\n");
}

void fpm_cmd_date(int argc, char *argv[])
{
    static const struct fpm_option long_opts[] = {
        { "help", FPM_NO_ARG, NULL, 'h' },
        {},
    };
    struct fpm_opt opt = {};
    bool terse = false;

    while (fpm_getopt(argc, argv, "ht", long_opts, &opt) >= 0) {
        switch (opt.ret) {
        case 1:
            set_date(opt.arg);
            return;

        case '?':
            // Unknown option: message already printed.
            fpm_puts("\r\n");
            return;

        case 't':
            terse = true;
            continue;

        case 'h':
            fpm_puts("Usage:\r\n"
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
    int year, month, day, dotw, hour, min, sec;
    fpm_get_datetime(&year, &month, &day, &dotw, &hour, &min, &sec);

    if (terse) {
        fpm_printf("%02d/%02d/%04d\r\n", month, day, year);
    } else {
        fpm_printf("Current Date: %s %d/%d/%04d\r\n", dotw_name[dotw], month, day, year);
        fpm_puts("Type 'date MM/DD/YYYY' to change.\r\n");
    }
    fpm_puts("\r\n");
}
