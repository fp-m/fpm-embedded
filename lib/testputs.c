//
// Print simple text message.
//
#include <rpm/api.h>

int main()
{
    rpm_print_version();

    rpm_puts("Hello, World!\r\n");

    static const uint16_t wmessage[] = { 'w', 'p', 'u', 't', 's', '\r', '\n' };
    rpm_wputs(wmessage);
}
