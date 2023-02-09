//
// Parse a command line and split it into tokens (in place).
// Return NULL on success.
// Fill an argument vector.
// On error, return a message.
//
#include <rpm/api.h>

const char *rpm_tokenize(char *argv[], int *argc, char *cmd_line)
{
    char *ptr = cmd_line;
    bool seen_space = true;

    // Scan the line symbol by symbol.
    // Look for delimiters: space, backslash, single quote, double quote.
    *argc = 0;
    for (;; ptr++) {
        char ch = *ptr;

        switch (ch) {
        case '\0':
            // End of line - append extra NUL.
            *++ptr = '\0';
            return 0;

        case ' ':
            // Space - replace by 0.
            seen_space = true;
            *ptr = '\0';
            break;

        default:
            // Ordinary symbol.
            if (seen_space) {
                // Next argument.
                argv[(*argc)++] = ptr;
                seen_space = false;
            }
            break;
        }
    }
}
