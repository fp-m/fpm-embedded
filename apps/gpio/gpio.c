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
// Get name of GPIO function.
//
static const char *func_name(int func)
{
    switch (func) {
    case GPIO_FUNC_XIP:  return "XIP";
    case GPIO_FUNC_SPI:  return "SPI";
    case GPIO_FUNC_UART: return "UART";
    case GPIO_FUNC_I2C:  return "I2C";
    case GPIO_FUNC_PWM:  return "PWM";
    case GPIO_FUNC_SIO:  return "SIO";
    case GPIO_FUNC_PIO0: return "PIO0";
    case GPIO_FUNC_PIO1: return "PIO1";
    case GPIO_FUNC_GPCK: return "GPCK";
    case GPIO_FUNC_USB:  return "USB";
    case GPIO_FUNC_NULL: return "NULL";
    default:             return "(unknown)";
    }
}

//
// Show one signal.
//
static void show_signal(unsigned pin)
{
    int dir = gpio_get_dir(pin);
    int val = gpio_get(pin);

    if (dir == GPIO_OUT) {
        fpm_printf(" out %d", val);
    } else {
        fpm_printf(" in %d", val);
        if (gpio_is_pulled_up(pin)) {
            fpm_printf(" pull up");
        } else if (gpio_is_pulled_down(pin)) {
            fpm_printf(" pull down");
        }
    }
}

//
// Show all signals.
//
static void show_all()
{
    for (unsigned pin = 0; pin < NUM_BANK0_GPIOS; pin++) {
        gpio_function_t func = gpio_get_function(pin);
        fpm_printf("gpio %u: ", pin);
        switch (func) {
        case GPIO_FUNC_NULL:
            fpm_printf("unused\r\n");
            break;
        case GPIO_FUNC_SIO:
            // Configured for GPIO.
            show_signal(pin);
            fpm_printf("\r\n");
            break;
        default:
            fpm_printf("function %s\r\n", func_name(func));
            break;
        }
    }
}

//
// When NUM: get value of one signal.
// When NUM-NUM: get values of several signals.
// When NUM=NUM: set value of one signal.
// When NUM-NUM=NUM: set values of several signals.
//
static void get_set(const char *arg)
{
    const char *dash_ptr = strchr(arg, '-');
    const char *eq_ptr   = strchr(arg, '=');
    unsigned long pin;
    if (fpm_strtoul(&pin, arg, NULL, 0)) {
        fpm_printf("\r\nBad pin: %s\r\n", arg);
        return;
    }
    unsigned long limit = pin;
    if (dash_ptr != NULL) {
        if (fpm_strtoul(&limit, dash_ptr + 1, NULL, 0)) {
            fpm_printf("\r\nBad limit: %s\r\n", arg);
            return;
        }
    }
    if (eq_ptr == NULL) {
        // Get signal(s).
        for (; pin <= limit; pin++) {
            fpm_printf(" %d", gpio_get(pin));
        }
    } else {
        // Set signal(s).
        unsigned long value;
        if (fpm_strtoul(&value, eq_ptr + 1, NULL, 0)) {
            fpm_printf("\r\nBad value: %s\r\n", arg);
            return;
        }
        for (; pin <= limit; pin++) {
            gpio_put(pin, value);
        }
    }
}

//
// When NUM: configure one signal.
// When NUM-NUM: configure several signals.
//
static void configure(const char *arg, int mode)
{
    const char *dash_ptr = strchr(arg, '-');
    unsigned long pin;
    if (fpm_strtoul(&pin, arg, NULL, 0)) {
        fpm_printf("\r\nBad pin: %s\r\n", arg);
        return;
    }
    unsigned long limit = pin;
    if (dash_ptr != NULL) {
        if (fpm_strtoul(&limit, dash_ptr + 1, NULL, 0)) {
            fpm_printf("\r\nBad limit: %s\r\n", arg);
            return;
        }
    }
    for (; pin <= limit; pin++) {
        switch (mode) {
        case CMD_OUT:
            gpio_set_dir(pin, GPIO_OUT);
            gpio_put(pin, 0);
            gpio_set_function(pin, GPIO_FUNC_SIO);
            break;
        case CMD_IN:
        case CMD_PU:
        case CMD_PD:
            gpio_set_dir(pin, GPIO_IN);
            gpio_put(pin, 0);
            gpio_set_function(pin, GPIO_FUNC_SIO);
            gpio_disable_pulls(pin);
            if (mode == CMD_PU) {
                gpio_pull_up(pin);
            }
            if (mode == CMD_PD) {
                gpio_pull_down(pin);
            }
            break;
        }
    }
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
