//
// Invoke three RP/M calls:
//      rpm_print_version()
//      rpm_puts()
//      rpm_wputs()
//
#include <rpm/api.h>

int main()
{
    rpm_print_version();

    rpm_puts("puts\r\n");

    static const uint16_t wmessage[] = { 'w', 'p', 'u', 't', 's', '\r', '\n' };
    rpm_wputs(wmessage);
}
