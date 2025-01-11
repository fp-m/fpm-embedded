//
// Get a Unicode character from console.
// Decode escape sequences.
//
#include <fpm/api.h>

uint16_t fpm_getkey()
{
    for (;;) {
        uint16_t key = fpm_getwch();
        if (key != '\33') {
            return key;
        }

        // Decode escape sequence.
        key = fpm_getwch();
        if (key != '[' && key != 'O') {
            // Unknown prefix - ignore.
            continue;
        }

        key = fpm_getwch();
        switch (key) {
        case 'A': // Esc-[-A Esc-O-A
            return FPM_UPWARDS_ARROW;

        case 'B': // Esc-[-B Esc-O-B
            return FPM_DOWNWARDS_ARROW;

        case 'D': // Esc-[-D Esc-O-D
            return FPM_LEFTWARDS_ARROW;

        case 'C': // Esc-[-C Esc-O-C
            return FPM_RIGHTWARDS_ARROW;

        case 'H': // Esc-[-H Esc-O-H
            return FPM_LEFTWARDS_TO_BAR;

        case 'F': // Esc-[-F Esc-O-F
            return FPM_RIGHTWARDS_TO_BAR;

        case '0': case '1': case '2': case '3': case '4':
        case '5': case '6': case '7': case '8': case '9': { // Esc-N-~
            // Read the number.
            unsigned num = 0;
            do {
                num = num*10 + (key - '0');
                key = fpm_getwch();
            } while (key >= '0' && key <= '9');

            if (key != '~') {
                // Unknown terminator - ignore.
                continue;
            }
            switch (num) {
            case 3: // Esc-[-3-~ - Delete on Linux
                return FPM_DELETE_KEY;
            default: // Unknown keycode - ignore.
                continue;
            }
        }
        default:
            // Unknown keycode - ignore.
            continue;
        }
    }
}
