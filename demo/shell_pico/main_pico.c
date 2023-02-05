#include <stdio.h>
#include <pico/stdlib.h>
#include <rpm/internal.h>

int main()
{
    // Initialize chosen serial port
    stdio_init_all();
again:
    while (!stdio_usb_connected()) {
        sleep_ms(1000);
    }

    // Wait for user to press 'enter' to continue
    printf("\r\nSD card test. Press 'enter' to start.\r\n");
    for (;;) {
        int ch = getchar();
        if (ch < 0) {
            goto again;
        }
        if (ch == '\r' || ch == '\n') {
            break;
        }
    }

    printf("Start shell on Pico\r\n");
    printf("Use 'exit' to quit.\r\n\r\n");

    // Start interactive dialog.
    rpm_shell();

    printf("Shell has finished\r\n");

    // Loop forever doing nothing
    for (;;) {
        sleep_ms(1000);
    }
}
