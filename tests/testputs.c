//
// Invoke three FP/M calls:
//      fpm_print_version()
//      fpm_puts()
//      fpm_wputs()
//
#include <fpm/api.h>

int main()
{
    fpm_print_version();

    fpm_puts("puts\r\n");

    static const uint16_t wmessage[] = { 'w', 'p', 'u', 't', 's', '\r', '\n' };
    fpm_wputs(wmessage);
}
