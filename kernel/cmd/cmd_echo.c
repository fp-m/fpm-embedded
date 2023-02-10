//
// Copy text directly to the console output
//
#include <rpm/api.h>
#include <rpm/internal.h>

void rpm_cmd_echo(int argc, char *argv[])
{
    //TODO: getopt
    for (int i=1; i<argc; i++) {
        if (i > 1)
            rpm_putchar(' ');
        rpm_puts(argv[i]);
    }
    rpm_puts("\r\n");
}
