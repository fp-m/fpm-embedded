//
// Display the contents of a text file
//
#include <rpm/api.h>
#include <rpm/fs.h>
#include <rpm/getopt.h>
#include <rpm/internal.h>
#include <stdlib.h>

//
// Print string, inserting \r before newline when needed.
//
static void puts_with_cr(const char *str, bool *need_cr)
{
    for (;;) {
        char ch = *str++;
        if (!ch)
            break;

        switch (ch) {
        case '\n':
            if (*need_cr) {
                rpm_putchar('\r');
                *need_cr = false;
            }
            break;
        case '\r':
            *need_cr = false;
            break;
        default:
            *need_cr = true;
            break;
        }
        rpm_putchar(ch);
    }
}

static void display_file(const char *path)
{
    // Open file.
    file_t *fp = alloca(f_sizeof_file_t());
    fs_result_t result = f_open(fp, path, FA_READ);
    if (result != FR_OK) {
        rpm_printf("%s: %s\r\n", path, f_strerror(result));
        return;
    }

    // Read data.
    char buf[1024];
    bool need_cr = false;
    while (f_gets(buf, sizeof(buf), fp)) {
        puts_with_cr(buf, &need_cr);
    }
    if (need_cr) {
        rpm_puts("\r\n");
    }

    // Close the file.
    f_close(fp);
}

void rpm_cmd_cat(int argc, char *argv[])
{
    static const struct rpm_option long_opts[] = {
        { "help", RPM_NO_ARG, NULL, 'h' },
        {},
    };
    struct rpm_opt opt = {};
    unsigned arg_count = 0;

    while (rpm_getopt(argc, argv, "h", long_opts, &opt) >= 0) {
        switch (opt.ret) {
        case 1:
            display_file(opt.arg);
            arg_count++;
            continue;

        case '?':
            // Unknown option: message already printed.
            rpm_puts("\r\n");
            return;

        case 'h':
usage:
            rpm_puts("Usage:\r\n"
                     "    cat filename ...\r\n"
                     "    type filename ...\r\n"
                     "\n");
            return;
        }
    }

    if (arg_count == 0)
        goto usage;

    rpm_puts("\r\n");
}
