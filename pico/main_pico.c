#include <stdio.h>
#include <fpm/api.h>
#include <fpm/fs.h>
#include <fpm/internal.h>
#include <fpm/diskio.h>
#include <fpm/context.h>
#include <pico/stdlib.h>

extern void fpm_init_datetime(void);

int main()
{
    // Initialize chosen serial port.
    stdio_init_all();

    // Setup heap area.
    fpm_context_t context_base;
    extern char end[], __HeapLimit[];
    fpm_heap_init(&context_base, (size_t)&end[0], __HeapLimit - end);

    fpm_init_datetime();
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
