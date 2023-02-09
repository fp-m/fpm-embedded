#include <stdio.h>
#include <pico/stdlib.h>
#include <rpm/internal.h>

int main()
{
    // Initialize chosen serial port.
    stdio_init_all();
    while (!stdio_usb_connected()) {
        sleep_ms(100);
    }

    for (;;) {
        printf("Start shell on Pico\r\n");
        printf("Use '?' for help.\r\n\r\n");

        // Start interactive dialog.
        rpm_shell();

        printf("Shell has finished\r\n");
    }
}
