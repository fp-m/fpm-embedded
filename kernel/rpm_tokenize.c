//
// Parse a command line and split it into tokens (in place).
// Return NULL on success.
// Fill an argument vector.
// On error, return a message.
//
#include <rpm/api.h>

const char *rpm_tokenize(char *argv[], int *argc, char *cmd_line)
{
    const char *src = cmd_line;
    char *dest = cmd_line;
    bool seen_space = true;

    // Scan the line symbol by symbol.
    // Look for delimiters: space, backslash, single quote, double quote.
    *argc = 0;
    for (;; src++) {
        char ch = *src;

        switch (ch) {
        case '\0':
            // End of line.
            if (! seen_space) {
                *dest++ = '\0';
            }
            // Append extra NUL.
            *dest = '\0';
            return 0;

        case ' ':
            // Space.
            if (! seen_space) {
                *dest++ = '\0';
                seen_space = true;
            }
            break;

        case '\\':
            // Backslash - quote next char.
            ch = *++src;
            if (ch == '\0') {
                // Error
                return "Incomplete backslash";
            }
            // Fall through...

        case '\'': // TODO
        case '"':  // TODO
        default:
            // Ordinary symbol.
            if (seen_space) {
                // Next argument.
                argv[(*argc)++] = dest;
                seen_space = false;
            }
            *dest++ = ch;
            break;
        }
    }
}
