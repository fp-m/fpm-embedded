//
// Control GPIO signals from command line.
//
#include <fpm/api.h>
#include <fpm/getopt.h>
#include <stdlib.h>
#include <pico/stdlib.h>

enum {
    CMD_GETSET,
    CMD_OUT,
    CMD_IN,
    CMD_PU,
    CMD_PD,
};

//
// Show all signals.
//
static void show_all()
{
    for (unsigned pin = 0; pin < NUM_BANK0_GPIOS; pin++) {
        gpio_function_t func = gpio_get_function(pin);
        fpm_printf("gpio% function %u\r\n", pin, func);
    //val = gpio_get(pin);
    //val = gpio_get_dir(pin); // 1 for out, 0 for in
    }
}

static void get_set(const char *arg)
{
    //TODO
    fpm_printf("get/set signal %s\r\n", arg);
    //val = gpio_get(pin);
    //gpio_put(pin, 1);
}

static void configure(const char *arg, int mode)
{
    //TODO
    fpm_printf("configure signal %s in mode %d\r\n", arg, mode);
    //gpio_set_dir(pin, GPIO_IN);
    //gpio_set_dir(pin, GPIO_OUT);
    //gpio_put(pin, 1);
}

int main(int argc, char **argv)
{
    struct fpm_opt opt = {};
    unsigned argcount = 0;
    int mode = CMD_GETSET;

    while (fpm_getopt(argc, argv, "h", NULL, &opt) >= 0) {
        switch (opt.ret) {
        case 1:
            if (strcmp(opt.arg, "info") == 0) {
                show_all();
            } else if (strcmp(opt.arg, "out") == 0) {
                mode = CMD_OUT;
            } else if (strcmp(opt.arg, "in") == 0) {
                mode = CMD_IN;
            } else if (strcmp(opt.arg, "pu") == 0) {
                mode = CMD_PU;
            } else if (strcmp(opt.arg, "pd") == 0) {
                mode = CMD_PD;
            } else {
                switch (mode) {
                case CMD_GETSET:
                    get_set(opt.arg);
                    break;
                default:
                    configure(opt.arg, mode);
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
                "    gpio info      - show configuration of all signals\r\n"
                "    gpio 1 3-5     - show signals gpio1 and gpio3...gpio5\r\n"
                "    gpio 1=0 3-5=1 - clear gpio to 0, set gpio3...gpio5 to 1\r\n"
                "    gpio out 1 2   - configure gpio1 and gpio2 as outputs\r\n"
                "    gpio in 3      - configure gpio3 as input (with high impedance)\r\n"
                "    gpio pu 4      - configure gpio4 as input with pull-up\r\n"
                "    gpio pd 5-7    - configure gpio5...gpio7 as inputs with pull-down\r\n"
                "\n");
            return 0;
        }
    }

    if (argcount == 0) {
        // Called without arguments.
        show_all();
    }
}
