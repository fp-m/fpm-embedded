//
// Print amount of available memory on the heap.
//
#include <fpm/api.h>

static void print_value(const char *name, size_t size)
{
    if (size < 1000) {
        fpm_printf("%s: %u bytes\r\n", name, size);
    } else {
        fpm_printf("%s: %u,%03u bytes\r\n", name, size / 1000, size % 1000);
    }
}

int main()
{
    print_value(" Free heap", fpm_heap_available());
    print_value("Free stack", fpm_stack_available());
}
