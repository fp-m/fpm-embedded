//
// Set or show the current system time
//
#include <rpm/api.h>
#include <rpm/internal.h>

static void set_time(const char *str)
{
    int hh = 0, mm = 0, ss = 0;
    if (rpm_sscanf(str, "%d:%d:%d", &hh, &mm, &ss) != 3 ||
        hh < 0 || hh > 23 ||
        mm < 0 || mm > 59 ||
        ss < 0 || ss > 59) {
        rpm_puts("The specified time is not correct.\r\n"
                 "\n");
        return;
    }

    // Set time.
    rpm_set_time(hh, mm, ss);
    rpm_puts("\r\n");
}

void rpm_cmd_time(int argc, char *argv[])
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
            set_time(opt.arg);
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
    int hour, min, sec;
    rpm_get_time(&hour, &min, &sec);

    if (terse) {
        rpm_printf("%02d:%02d:%02d\r\n", hour, min, sec);
    } else {
        // Convert 24-hour clock time to 12-hour clock.
        char am_pm = (hour <= 11) ? 'A' : 'P';
        if (hour == 0) {
            hour += 12;
        } else if (hour > 12) {
            hour -= 12;
        }
        rpm_printf("Current Time: %02d:%02d:%02d %cM\r\n", hour, min, sec, am_pm);
        rpm_puts("Type 'time hh:mm:ss' to change.\r\n");
    }
    rpm_puts("\r\n");
}
