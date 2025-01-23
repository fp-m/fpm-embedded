//
// Export dynamically linked routines.
//
#include <fpm/api.h>
#include <fpm/loader.h>
#include <fpm/fs.h>
#include <fpm/getopt.h>
#include <stdlib.h>
#include <pico/stdlib.h>

int gpio_get_pad(uint gpio); // missing in hardware/gpio.h

fpm_binding_t fpm_bindings[] = {
#include <fpm/bindings.h>

    // GPIO functions.
    FPM_BIND(gpio_acknowledge_irq),
    FPM_BIND(gpio_add_raw_irq_handler_masked),
    FPM_BIND(gpio_add_raw_irq_handler_masked64),
    FPM_BIND(gpio_add_raw_irq_handler_with_order_priority_masked),
    FPM_BIND(gpio_add_raw_irq_handler_with_order_priority_masked64),
    FPM_BIND(gpio_debug_pins_init),
    FPM_BIND(gpio_deinit),
    FPM_BIND(gpio_get_drive_strength),
    FPM_BIND(gpio_get_function),
    FPM_BIND(gpio_get_pad),
    FPM_BIND(gpio_get_slew_rate),
    FPM_BIND(gpio_init),
    FPM_BIND(gpio_init_mask),
    FPM_BIND(gpio_is_input_hysteresis_enabled),
    FPM_BIND(gpio_remove_raw_irq_handler_masked),
    FPM_BIND(gpio_remove_raw_irq_handler_masked64),
    FPM_BIND(gpio_set_dormant_irq_enabled),
    FPM_BIND(gpio_set_drive_strength),
    FPM_BIND(gpio_set_function),
    FPM_BIND(gpio_set_function_masked),
    FPM_BIND(gpio_set_function_masked64),
    FPM_BIND(gpio_set_inover),
    FPM_BIND(gpio_set_input_enabled),
    FPM_BIND(gpio_set_input_hysteresis_enabled),
    FPM_BIND(gpio_set_irq_callback),
    FPM_BIND(gpio_set_irq_enabled),
    FPM_BIND(gpio_set_irq_enabled_with_callback),
    FPM_BIND(gpio_set_irqover),
    FPM_BIND(gpio_set_oeover),
    FPM_BIND(gpio_set_outover),
    FPM_BIND(gpio_set_pulls),
    FPM_BIND(gpio_set_slew_rate),

    {},
};
