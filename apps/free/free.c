//
// Print amount of available memory on the heap.
//
#include <fpm/api.h>

int main()
{
    fpm_printf("Free memory: %u kbytes\r\n", fpm_heap_available() / 1024);
}
