//
// Set or show the current system time
//
#include <fpm/api.h>
#include <fpm/getopt.h>
#include <fpm/internal.h>

static void set_time(const char *str)
{
    int year, month, day, dotw, hour, min, sec;
    fpm_get_datetime(&year, &month, &day, &dotw, &hour, &min, &sec);

    if (fpm_sscanf(str, "%d:%d:%d", &hour, &min, &sec) != 3 ||
        hour < 0 || hour > 23 ||
        min < 0 || min > 59 ||
        sec < 0 || sec > 59) {
        fpm_puts("The specified time is not correct.\r\n"
                 "\n");
        return;
    }

    // Set time.
    fpm_set_datetime(year, month, day, hour, min, sec);
    fpm_puts("\r\n");
}

void fpm_cmd_time(int argc, char *argv[])
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
            set_time(opt.arg);
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
                     "    time [-t]\r\n"
                     "    time hh:mm:ss\r\n"
                     "\n"
                     "    -t            Display simple time\r\n"
                     "    hh:mm:ss      New time to set\r\n"
                     "\n");
            return;
        }
    }

    // Display current time.
    int year, month, day, dotw, hour, min, sec;
    fpm_get_datetime(&year, &month, &day, &dotw, &hour, &min, &sec);

    if (terse) {
        fpm_printf("%02d:%02d:%02d\r\n", hour, min, sec);
    } else {
        // Convert 24-hour clock time to 12-hour clock.
        char am_pm = (hour <= 11) ? 'A' : 'P';
        if (hour == 0) {
            hour += 12;
        } else if (hour > 12) {
            hour -= 12;
        }
        fpm_printf("Current Time: %02d:%02d:%02d %cM\r\n", hour, min, sec, am_pm);
        fpm_puts("Type 'time hh:mm:ss' to change.\r\n");
    }
    fpm_puts("\r\n");
}
