//
// API for FP/M, implemented with Pico SDK.
//
#include <fpm/api.h>
#include <fpm/internal.h>
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/watchdog.h"

//
// Wait for console input.
// Return ASCII keycode.
//
char fpm_getchar()
{
    int ch;

    for (;;) {
#if LIB_PICO_STDIO_USB
        // Make sure console is connected.
        while (!stdio_usb_connected()) {
            sleep_ms(100);
        }
#endif
        // Read one byte.
        ch = getchar();
        if (ch >= 0) {
            break;
        }
    }

    // ^C - kill the process.
    if (ch == '\3') {
        fpm_puts("^C\r\n");
        longjmp(fpm_saved_point, 1);
    }
    return ch;
}

//
// Write Unicode character to the console.
//
// Convert to UTF-8 encoding:
// 00000000.0xxxxxxx -> 0xxxxxxx
// 00000xxx.xxyyyyyy -> 110xxxxx, 10yyyyyy
// xxxxyyyy.yyzzzzzz -> 1110xxxx, 10yyyyyy, 10zzzzzz
//
void fpm_putchar(char ch)
{
    putchar(ch);
}

//
// Posix-compatible formatted output to console.
//
int fpm_printf(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    int retval = vprintf(format, args);
    va_end(args);
    fflush(stdout);
    return retval;
}

int fpm_vprintf(const char *format, va_list args)
{
    int retval = vprintf(format, args);
    fflush(stdout);
    return retval;
}

//
// Posix-compatible formatted output to string.
//
int fpm_snprintf(char *str, size_t size, const char *format, ...)
{
    va_list args;
    va_start(args, format);
    int retval = vsnprintf(str, size, format, args);
    va_end(args);
    return retval;
}

int fpm_vsnprintf(char *str, size_t size, const char *format , va_list args)
{
    return vsnprintf(str, size, format, args);
}

//
// Posix-compatible formatted input.
//
int fpm_sscanf(const char *str, const char *format, ...)
{
    va_list args;
    va_start(args, format);
    int retval = vsscanf(str, format, args);
    va_end(args);
    return retval;
}

int fpm_vsscanf(const char *str, const char *format, va_list args)
{
    return vsscanf(str, format, args);
}

void fpm_print_version()
{
    fpm_puts("FP/M version "FPM_VERSION"."GIT_REVCOUNT"\r\n");
    fpm_puts("Git commit "GIT_COMMIT", built on "__DATE__" at "__TIME__"\r\n");
    fpm_puts("Pico SDK version "PICO_SDK_VERSION_STRING"\r\n");
    fpm_printf("RP2040 chip revision B%d, ROM version %d\r\n", rp2040_chip_version(), rp2040_rom_version());
}

//
// Reboot the processor.
//
void fpm_reboot()
{
    watchdog_reboot(0, 0, 0);
}

//
// Return the current 64-bit timestamp value in microseconds.
//
uint64_t fpm_time_usec()
{
    return time_us_64();
}

//
// Busy wait for the given 64-bit number of microseconds.
//
void fpm_delay_usec(uint64_t microseconds)
{
     busy_wait_us(microseconds);
}

//
// Busy wait for the given number of milliseconds.
//
void fpm_delay_msec(unsigned milliseconds)
{
     busy_wait_ms(milliseconds);
}

static void print_pins(unsigned const ss_pin, unsigned const sck_pin,
                       unsigned const mosi_pin, unsigned const miso_pin)
{
    unsigned const ss   = gpio_get(ss_pin);
    unsigned const sck  = gpio_get(sck_pin);
    unsigned const mosi = gpio_get(mosi_pin);
    unsigned const miso = gpio_get(miso_pin);
    fpm_printf("ss = %u, sck = %u, miso = %u, miso = %u\r\n", ss, sck, mosi, miso);
}


//
// Test GPIO signals while SPI is not yet enabled.
//
static void probe_pins(unsigned const ss_pin,   // DAT3/~CS - chip select output
                       unsigned const sck_pin,  // CLK/SCK  - clock output
                       unsigned const mosi_pin, // CMD/SDI  - data output
                       unsigned const miso_pin) // DAT0/SDO - data input
{
    gpio_init(ss_pin);
    gpio_init(sck_pin);
    gpio_init(mosi_pin);
    gpio_init(miso_pin);

    // Save pin directions.
    // Note GPIO_OUT is 1/true and GPIO_IN is 0/false.
    unsigned const save_ss_direction   = gpio_get_dir(ss_pin);
    unsigned const save_sck_direction  = gpio_get_dir(sck_pin);
    unsigned const save_mosi_direction = gpio_get_dir(mosi_pin);
    unsigned const save_miso_direction = gpio_get_dir(miso_pin);

    // Save pin state.
    unsigned const save_ss_state   = save_ss_direction ?   gpio_get(ss_pin) : 0;
    unsigned const save_sck_state  = save_sck_direction ?  gpio_get(sck_pin) : 0;
    unsigned const save_mosi_state = save_mosi_direction ? gpio_get(mosi_pin) : 0;
    unsigned const save_miso_state = save_miso_direction ? gpio_get(miso_pin) : 0;

    // Chip select is active-low, so we initialise it to a driven-high state.
    gpio_put(ss_pin, 1); // Avoid any glitches when enabling output
    gpio_put(sck_pin, 0);
    gpio_put(mosi_pin, 0);
    gpio_set_dir(ss_pin, GPIO_OUT);
    gpio_set_dir(sck_pin, GPIO_OUT);
    gpio_set_dir(mosi_pin, GPIO_OUT);
    gpio_put(ss_pin, 1); // In case set_dir does anything
    gpio_put(sck_pin, 0);
    gpio_put(mosi_pin, 0);

    fpm_printf("Initial state\r\n");
    print_pins(ss_pin, sck_pin, mosi_pin, miso_pin);

    fpm_printf("Enable select\r\n");
    gpio_put(ss_pin, 0);
    print_pins(ss_pin, sck_pin, mosi_pin, miso_pin);

    fpm_printf("Disable select\r\n");
    gpio_put(ss_pin, 1);
    print_pins(ss_pin, sck_pin, mosi_pin, miso_pin);

    fpm_printf("Set SCK, enable select\r\n");
    gpio_put(sck_pin, 1);
    gpio_put(ss_pin, 0);
    print_pins(ss_pin, sck_pin, mosi_pin, miso_pin);

    fpm_printf("Disable select\r\n");
    gpio_put(ss_pin, 1);
    print_pins(ss_pin, sck_pin, mosi_pin, miso_pin);

    fpm_printf("Clear SCK, enable select\r\n");
    gpio_put(sck_pin, 0);
    gpio_put(ss_pin, 0);
    print_pins(ss_pin, sck_pin, mosi_pin, miso_pin);

    fpm_printf("Disable select\r\n");
    gpio_put(ss_pin, 1);
    print_pins(ss_pin, sck_pin, mosi_pin, miso_pin);

    fpm_printf("Set MOSI, enable select\r\n");
    gpio_put(mosi_pin, 1);
    gpio_put(ss_pin, 0);
    print_pins(ss_pin, mosi_pin, mosi_pin, miso_pin);

    fpm_printf("Disable select\r\n");
    gpio_put(ss_pin, 1);
    print_pins(ss_pin, mosi_pin, mosi_pin, miso_pin);

    fpm_printf("Clear MOSI, enable select\r\n");
    gpio_put(mosi_pin, 0);
    gpio_put(ss_pin, 0);
    print_pins(ss_pin, mosi_pin, mosi_pin, miso_pin);

    fpm_printf("Disable select\r\n");
    gpio_put(ss_pin, 1);
    print_pins(ss_pin, mosi_pin, mosi_pin, miso_pin);

    fpm_printf("Set SCK and MOSI, enable select\r\n");
    gpio_put(sck_pin, 1);
    gpio_put(mosi_pin, 1);
    gpio_put(ss_pin, 0);
    print_pins(ss_pin, mosi_pin, mosi_pin, miso_pin);

    fpm_printf("Disable select\r\n");
    gpio_put(ss_pin, 1);
    print_pins(ss_pin, mosi_pin, mosi_pin, miso_pin);

    fpm_printf("Clear SCK and MOSI, enable select\r\n");
    gpio_put(sck_pin, 0);
    gpio_put(mosi_pin, 0);
    gpio_put(ss_pin, 0);
    print_pins(ss_pin, mosi_pin, mosi_pin, miso_pin);

    fpm_printf("Disable select\r\n");
    gpio_put(ss_pin, 1);
    print_pins(ss_pin, mosi_pin, mosi_pin, miso_pin);

    // Restore pin state early, to glitches when enabling output.
    if (save_ss_direction)   gpio_put(ss_pin,   save_ss_state);
    if (save_sck_direction)  gpio_put(sck_pin,  save_sck_state);
    if (save_mosi_direction) gpio_put(mosi_pin, save_mosi_state);
    if (save_miso_direction) gpio_put(miso_pin, save_miso_state);

    // Restore pin directions.
    // Note GPIO_OUT is 1/true and GPIO_IN is 0/false.
    gpio_set_dir(ss_pin, save_ss_direction);
    gpio_set_dir(sck_pin, save_sck_direction);
    gpio_set_dir(mosi_pin, save_mosi_direction);
    gpio_set_dir(miso_pin, save_miso_direction);

    // Restore pin state.
    if (save_ss_direction)   gpio_put(ss_pin,   save_ss_state);
    if (save_sck_direction)  gpio_put(sck_pin,  save_sck_state);
    if (save_mosi_direction) gpio_put(mosi_pin, save_mosi_state);
    if (save_miso_direction) gpio_put(miso_pin, save_miso_state);
}

//
// Find SD connection.
//
void fpm_autodetect()
{
    // SparkFun RP2040 Thing Plus board.
    probe_pins(9, 14, 15, 12);
}
