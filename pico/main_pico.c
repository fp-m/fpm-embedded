#include <stdio.h>
#include <fpm/api.h>
#include <fpm/fs.h>
#include <fpm/internal.h>
#include <fpm/diskio.h>
#include <fpm/context.h>
#include <pico/stdlib.h>
#if HAS_RP2040_RTC
#   include "hardware/rtc.h"
#endif

void setup_date_time()
{
#if HAS_RP2040_RTC
    // Start the real time clock.
    rtc_init();

    //TODO: get time/date from battery backed RTC or from filesystem
    datetime_t t = {
        .year  = BUILD_YEAR,
        .month = BUILD_MONTH,
        .day   = BUILD_DAY,
        .dotw  = BUILD_DOTW, // 0 is Sunday, so 5 is Friday
        .hour  = BUILD_HOUR,
        .min   = BUILD_MIN,
        .sec   = BUILD_SEC,
    };
    rtc_set_datetime(&t);
    sleep_us(64);
#endif
}

int main()
{
    // Initialize chosen serial port.
    stdio_init_all();

    // Setup heap area.
    fpm_context_t context_base;
    extern char end[], __HeapLimit[];
    fpm_heap_init(&context_base, (size_t)&end[0], __HeapLimit - end);

    setup_date_time();
    disk_setup();

    // Try to mount flash at startup.
    // It may fail, which is OK.
    f_mount("flash:");

    // Start interactive dialog.
    for (;;) {
#if LIB_PICO_STDIO_USB
        // Make sure console is connected.
        while (!stdio_usb_connected()) {
            sleep_ms(100);
        }
#endif
        fpm_shell();
    }
}
