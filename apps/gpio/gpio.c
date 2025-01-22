//
// Control GPIO signals from command line.
//
#include <fpm/api.h>
#include <fpm/getopt.h>
#include <stdlib.h>

enum {
    CMD_GET,
    CMD_SET,
};

//
// Show all signals.
//
void show_all()
{
    //TODO
    fpm_puts("show all signals\r\n");
}

void get_signal(const char *arg)
{
    //TODO
    fpm_printf("show signal %s\r\n", arg);
}

void set_signal(const char *arg, unsigned value)
{
    //TODO
    fpm_printf("set signal %s %u\r\n", arg, value);
}

int main(int argc, char **argv)
{
    struct fpm_opt opt = {};
    unsigned argcount = 0;
    unsigned value = 1;
    int mode = CMD_GET;

    while (fpm_getopt(argc, argv, "h", NULL, &opt) >= 0) {
        switch (opt.ret) {
        case 1:
            if (strcmp(opt.arg, "info") == 0) {
                show_all();
            } else if (strcmp(opt.arg, "get") == 0) {
                mode = CMD_GET;
            } else if (strcmp(opt.arg, "set") == 0) {
                mode = CMD_SET;
                value = 1;
            } else if (strcmp(opt.arg, "clear") == 0) {
                mode = CMD_SET;
                value = 0;
            } else {
                switch (mode) {
                case CMD_GET:
                    get_signal(opt.arg);
                    break;
                case CMD_SET:
                    set_signal(opt.arg, value);
                    break;
                }
            }
            argcount++;
            break;
        case '?':
            // Unknown option: message already printed.
            fpm_puts("\r\n");
            return 0;
        case 'h':
            fpm_puts(
                "Usage:\r\n"
                "    gpio info\r\n"
                "    gpio get 1 2 4-7\r\n"
                "    gpio set 1=0 2=1 4-7=1\r\n"
                "    gpio clear 1 2 4-7\r\n"
                "Commands:\r\n"
                "    info       Show all GPIO signals\r\n"
                "    get        Get input signals\r\n"
                "    set        Set otput signals\r\n"
                "    clear      Clear otput signals\r\n"
                "\n");
            return 0;
        }
    }

    if (argcount == 0) {
        // Called without arguments.
        show_all();
    }
}
