#include <stdio.h>
#include <fpm/fs.h>
#include <fpm/internal.h>
#include <fpm/diskio.h>
#include <pico/stdlib.h>
#include "hardware/rtc.h"

void setup_date_time()
{
    // Start the real time clock.
    rtc_init();

    //TODO: get time/date from battery backed RTC or from filesystem
    datetime_t t = {
        .year  = 2023,
        .month = 02,
        .day   = 10,
        .dotw  = 5, // 0 is Sunday, so 5 is Friday
        .hour  = 0,
        .min   = 54,
        .sec   = 0,
    };
    rtc_set_datetime(&t);
    sleep_us(64);
}

int main()
{
    setup_date_time();
    disk_setup();

    // Try to mount flash at startup.
    // It may fail, which is OK.
    f_mount("flash:");

    // Initialize chosen serial port.
    stdio_init_all();

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
