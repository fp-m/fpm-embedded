//
// API for RP/M, implemented with Pico SDK.
//
#include <rpm/api.h>
#include <rpm/internal.h>
#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/stdio_usb.h"
#include "hardware/rtc.h"
#include "hardware/watchdog.h"
#include "pico/bootrom.h"
#include "hardware/structs/ioqspi.h"
#include "hardware/structs/ssi.h"
#include "hardware/sync.h"

//
// Wait for console input.
// Return ASCII keycode.
//
char rpm_getchar()
{
    int ch;

    for (;;) {
        // Make sure console is connected.
        while (!stdio_usb_connected()) {
            sleep_ms(100);
        }

        // Read one byte.
        ch = getchar();
        if (ch >= 0) {
            break;
        }
    }

    // ^C - kill the process.
    if (ch == '\3') {
        rpm_puts("^C\r\n");
        longjmp(rpm_saved_point, 1);
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
void rpm_putchar(char ch)
{
    putchar(ch);
}

//
// Posix-compatible formatted output to console.
//
int rpm_printf(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    int retval = vprintf(format, args);
    va_end(args);
    fflush(stdout);
    return retval;
}

int rpm_vprintf(const char *format, va_list args)
{
    int retval = vprintf(format, args);
    fflush(stdout);
    return retval;
}

//
// Posix-compatible formatted output to string.
//
int rpm_snprintf(char *str, size_t size, const char *format, ...)
{
    va_list args;
    va_start(args, format);
    int retval = vsnprintf(str, size, format, args);
    va_end(args);
    return retval;
}

int rpm_vsnprintf(char *str, size_t size, const char *format , va_list args)
{
    return vsnprintf(str, size, format, args);
}

//
// Posix-compatible formatted input.
//
int rpm_sscanf(const char *str, const char *format, ...)
{
    va_list args;
    va_start(args, format);
    int retval = vsscanf(str, format, args);
    va_end(args);
    return retval;
}

int rpm_vsscanf(const char *str, const char *format, va_list args)
{
    return vsscanf(str, format, args);
}

//
// Perform command on the Flash chip.
//
static void __no_inline_not_in_flash_func(rpm_flash_do_cmd)(const uint8_t* txbuf, uint8_t* rxbuf, size_t count)
{
    uint32_t irqsave = save_and_disable_interrupts();

    // Call routines in Boot ROM.
    rom_connect_internal_flash_fn connect_internal_flash =
        (rom_connect_internal_flash_fn)rom_func_lookup_inline(ROM_FUNC_CONNECT_INTERNAL_FLASH);
    rom_flash_exit_xip_fn flash_exit_xip =
        (rom_flash_exit_xip_fn)rom_func_lookup_inline(ROM_FUNC_FLASH_EXIT_XIP);
    rom_flash_flush_cache_fn flash_flush_cache =
        (rom_flash_flush_cache_fn)rom_func_lookup_inline(ROM_FUNC_FLASH_FLUSH_CACHE);
    rom_flash_enter_cmd_xip_fn flash_enter_cmd_xip =
        (rom_flash_enter_cmd_xip_fn)rom_func_lookup_inline(ROM_FUNC_FLASH_ENTER_CMD_XIP);

    __compiler_memory_barrier();
    connect_internal_flash();
    flash_exit_xip();

    // Flash /CS = low.
    hw_write_masked(&ioqspi_hw->io[1].ctrl,
                    IO_QSPI_GPIO_QSPI_SS_CTRL_OUTOVER_VALUE_LOW << IO_QSPI_GPIO_QSPI_SS_CTRL_OUTOVER_LSB,
                    IO_QSPI_GPIO_QSPI_SS_CTRL_OUTOVER_BITS);

    size_t tx_remaining = count;
    size_t rx_remaining = count;
    const size_t max_in_flight = 16 - 2;
    while (tx_remaining || rx_remaining) {
        uint32_t flags = ssi_hw->sr;
        bool can_put = flags & SSI_SR_TFNF_BITS;
        bool can_get = flags & SSI_SR_RFNE_BITS;
        if (can_put && tx_remaining && rx_remaining - tx_remaining < max_in_flight) {
            ssi_hw->dr0 = *txbuf++;
            --tx_remaining;
        }
        if (can_get && rx_remaining) {
            *rxbuf++ = (uint8_t)ssi_hw->dr0;
            --rx_remaining;
        }
    }

    // Flash /CS = high.
    hw_write_masked(&ioqspi_hw->io[1].ctrl,
                    IO_QSPI_GPIO_QSPI_SS_CTRL_OUTOVER_VALUE_HIGH << IO_QSPI_GPIO_QSPI_SS_CTRL_OUTOVER_LSB,
                    IO_QSPI_GPIO_QSPI_SS_CTRL_OUTOVER_BITS);

    flash_flush_cache();
    flash_enter_cmd_xip();
    restore_interrupts(irqsave);
}

//
// Read Flash unique ID as 64-bit integer value.
//
static uint64_t rpm_flash_read_unique_id()
{
    // Read Unique ID instruction: 4Bh command prefix, 32 dummy bits, 64 data bits.
    uint8_t txbuf[1 + 4 + 8] = { 0x4b };
    uint8_t rxbuf[1 + 4 + 8] = {};
    rpm_flash_do_cmd(txbuf, rxbuf, sizeof(txbuf));

    return (uint64_t)rxbuf[5] << 56 | (uint64_t)rxbuf[6] << 48 |
           (uint64_t)rxbuf[7] << 40 | (uint64_t)rxbuf[8] << 32 |
           (uint64_t)rxbuf[9] << 24 | (uint64_t)rxbuf[10] << 16 |
           (uint64_t)rxbuf[11] << 8 | rxbuf[12];
}

//
// Read Flash manufacturer ID and device ID.
//
static void rpm_flash_read_mf_dev_id(uint8_t *mf_id, uint16_t *dev_id)
{
    // JEDEC ID instruction: 9Fh command prefix, 24 data bits.
    uint8_t txbuf[1 + 3] = { 0x9f };
    uint8_t rxbuf[1 + 3] = {};
    rpm_flash_do_cmd(txbuf, rxbuf, sizeof(txbuf));

    *mf_id = rxbuf[1];
    *dev_id = rxbuf[2] << 8 | rxbuf[3];
}

//
// Print Flash memory size and other info.
//
static void rpm_print_flash_info()
{
    uint64_t unique_id = rpm_flash_read_unique_id();
    uint8_t mf_id;
    uint16_t dev_id;
    rpm_flash_read_mf_dev_id(&mf_id, &dev_id);

    // Manufacturer ID  Chip          ID    Mbytes
    // -------------------------------------------
    //   Winbond    ef  W25Q16JV-IQ   4015  2
    //   Winbond    ef  W25Q128JV-IQ  4018  16
    //   Winbond    ef  W25Q16JV-IM   7015  2
    //   Winbond    ef  W25Q128JV-IM  7018  16
    //   Renesas    1f  AT25SF128A    8901  16
    //
    const char *chip = "Unknown";
    unsigned megabytes = 0;
    switch (mf_id) {
    case 0xef: // Winbond
        switch (dev_id) {
        case 0x4015:
            chip = "W25Q16JV-IQ";
            megabytes = 2;
            break;
        case 0x4018:
            chip = "W25Q128JV-IQ";
            megabytes = 16;
            break;
        case 0x7015:
            chip = "W25Q16JV-IM";
            megabytes = 2;
            break;
        case 0x7018:
            chip = "W25Q128JV-IM";
            megabytes = 16;
            break;
        }
        break;
    case 0x1f: // Renesas
        switch (dev_id) {
        case 0x8901:
            chip = "AT25SF128A";
            megabytes = 16;
            break;
        }
        break;
    }
    rpm_printf("Flash %s, id %016llx, size %u Mbytes\r\n",
                chip, (unsigned long long)unique_id, megabytes);
}

void rpm_print_version()
{
    rpm_puts("RP/M version "RPM_VERSION"."GIT_REVCOUNT"\r\n");
    rpm_puts("Git commit "GIT_COMMIT", built on "__DATE__" at "__TIME__"\r\n");
    rpm_puts("Pico SDK version "PICO_SDK_VERSION_STRING"\r\n");
    rpm_printf("RP2040 chip revision B%d, ROM version %d\r\n", rp2040_chip_version(), rp2040_rom_version());
    rpm_print_flash_info();
}

//
// Get date.
//
void rpm_get_date(int *year, int *month, int *day, int *dotw)
{
    datetime_t now;
    rtc_get_datetime(&now);

    *year = now.year;
    *month = now.month;
    *day = now.day;
    *dotw = now.dotw;
}

//
// Get local time.
//
void rpm_get_time(int *hour, int *min, int *sec)
{
    datetime_t now;
    rtc_get_datetime(&now);

    *hour = now.hour;
    *min = now.min;
    *sec = now.sec;
}

//
// Set date.
//
void rpm_set_date(int year, int month, int day)
{
    datetime_t now;
    rtc_get_datetime(&now);

    now.year = year;
    now.month = month;
    now.day = day;
    now.dotw = rpm_get_dotw(year, month, day);

    rtc_set_datetime(&now);
    sleep_us(64);
}

//
// Set time.
//
void rpm_set_time(int hour, int min, int sec)
{
    datetime_t now;
    rtc_get_datetime(&now);

    now.hour = hour;
    now.min = min;
    now.sec = sec;

    rtc_set_datetime(&now);
    sleep_us(64);
}

//
// Reboot the processor.
//
void rpm_reboot()
{
    watchdog_reboot(0, 0, 0);
}
