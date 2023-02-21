#include <stdio.h>
#include <rpm/internal.h>
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

    // Initialize chosen serial port.
    stdio_init_all();
    while (!stdio_usb_connected()) {
        sleep_ms(100);
    }

    for (;;) {
        printf("Start RP/M shell.\r\n");
        printf("Use '?' for help.\r\n\r\n");

        // Start interactive dialog.
        rpm_shell();

        printf("Shell has finished.\r\n");
    }
}
