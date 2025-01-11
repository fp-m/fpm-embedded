//
// Copy text directly to the console output
//
#include <fpm/api.h>
#include <fpm/getopt.h>
#include <fpm/internal.h>

void fpm_cmd_echo(int argc, char *argv[])
{
    static const struct fpm_option long_opts[] = {
        { "help", FPM_NO_ARG, NULL, 'h' },
        {},
    };
    struct fpm_opt opt = {};
    bool no_newline = false;
    int count = 0;

    while (fpm_getopt(argc, argv, "hn", long_opts, &opt) >= 0) {
        switch (opt.ret) {
        case 1:
            // Display arguments.
            if (count > 0)
                fpm_putchar(' ');
            fpm_puts(opt.arg);
            count++;
            continue;

        case '?':
            // Unknown option: message already printed.
            continue;

        case 'n':
            no_newline = true;
            continue;

        case 'h':
            fpm_puts("Usage:\r\n"
                     "    echo [-n] [string ...]\r\n"
                     "\n"
                     "    -n            Do not output the trailing newline\r\n"
                     "\n");
            return;
        }
    }

    if (!no_newline) {
        fpm_puts("\r\n");
    }
}
