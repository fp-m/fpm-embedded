//
// Copy text directly to the console output
//
#include <rpm/api.h>
#include <rpm/getopt.h>
#include <rpm/internal.h>

void rpm_cmd_echo(int argc, char *argv[])
{
    static const struct rpm_option long_opts[] = {
        { "help", RPM_NO_ARG, NULL, 'h' },
        {},
    };
    struct rpm_opt opt = {};
    bool no_newline = false;
    int count = 0;

    while (rpm_getopt(argc, argv, "hn", long_opts, &opt) >= 0) {
        switch (opt.ret) {
        case 1:
            // Display arguments.
            if (count > 0)
                rpm_putchar(' ');
            rpm_puts(opt.arg);
            count++;
            continue;

        case '?':
            // Unknown option: message already printed.
            continue;

        case 'n':
            no_newline = true;
            continue;

        case 'h':
            rpm_puts("Usage:\r\n"
                     "    echo [-n] [string ...]\r\n"
                     "\n"
                     "    -n            Do not output the trailing newline\r\n"
                     "\n");
            return;
        }
    }

    if (!no_newline) {
        rpm_puts("\r\n");
    }
}
