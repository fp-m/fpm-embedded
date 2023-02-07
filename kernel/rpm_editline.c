//
// Interactive line editor.
//
#include <rpm/api.h>

#define CTRL(c) (c & 037)

//
// Unicode symbols.
//
#define LEFTWARDS_ARROW     0x2190  // ←
#define UPWARDS_ARROW       0x2191  // ↑
#define RIGHTWARDS_ARROW    0x2192  // →
#define DOWNWARDS_ARROW     0x2193  // ↓
#define LEFTWARDS_TO_BAR    0x21E4  // ⇤
#define RIGHTWARDS_TO_BAR   0x21E5  // ⇥
#define DELETE_KEY          0x2421  // ␡

//
// Insert a character at current position,
// move the rest of the line to the right.
//
static void insert_character()
{
    rpm_puts("\33[@");
}

//
// Remove a character at the current position,
// move the rest of the line to the left.
//
static void delete_character()
{
    rpm_puts("\33[P");
}

//
// Erase the line starting from current position.
//
static void erase_till_end_of_line()
{
    rpm_puts("\33[K");
}

//
// "Unprint" the current line.
//
static void erase_line(uint16_t *buffer, unsigned *insert_pos)
{
    while (*insert_pos > 0) {
        rpm_putwch('\b');
        (*insert_pos)--;
    }
    if (buffer[0] != 0) {
        erase_till_end_of_line();
        buffer[0] = 0;
    }
}

//
// Decode escape sequence.
//
static uint16_t decode_escape_sequence()
{
    uint16_t key = rpm_getwch();
    if (key != '[' && key != 'O') {
        // Unknown prefix - ignore.
        return 0;
    }

    key = rpm_getwch();
    switch (key) {
    case 'A': // Esc-[-A Esc-O-A
        return UPWARDS_ARROW;

    case 'B': // Esc-[-B Esc-O-B
        return DOWNWARDS_ARROW;

    case 'D': // Esc-[-D Esc-O-D
        return LEFTWARDS_ARROW;

    case 'C': // Esc-[-C Esc-O-C
        return RIGHTWARDS_ARROW;

    case 'H': // Esc-[-H Esc-O-H
        return LEFTWARDS_TO_BAR;

    case 'F': // Esc-[-F Esc-O-F
        return RIGHTWARDS_TO_BAR;

    case '0': case '1': case '2': case '3': case '4':
    case '5': case '6': case '7': case '8': case '9': { // Esc-N-~
        // Read the number.
        unsigned num = 0;
        do {
            num = num*10 + (key - '0');
            key = rpm_getwch();
        } while (key >= '0' && key <= '9');

        if (key != '~') {
            // Unknown terminator - ignore.
            return 0;
        }
        switch (num) {
        case 3: // Esc-[-3-~ - Delete on Linux
            return DELETE_KEY;
        default: // Unknown keycode - ignore.
            return 0;
        }
    }
    default: // Unknown keycode - ignore.
        return 0;
    }
}

//
// The main line edit function
// Parameters:
// - buffer: Pointer to the line edit buffer
// - buffer_length: Size of the buffer in bytes
// - clear: Set to 0 to not clear, 1 to clear on entry
// - prompt: Print before input
// - history: Pointer to the line edit buffer
// Returns:
// - The exit key pressed (ESC or CR)
//
int rpm_editline(uint16_t *buffer, unsigned buffer_length, bool clear, const char *prompt, uint16_t *prev_line)
{
    uint16_t next_line[RPM_CMDLINE_SIZE];
    bool on_prev_line = false;

    rpm_puts(prompt);
    if (clear) {
        buffer[0] = 0;
    } else {
        rpm_wputs(buffer);
    }

    unsigned insert_pos = rpm_strwlen(buffer);
    for (;;) {
        // Get Unicode symbol.
        uint16_t key = rpm_getwch();

        if (key == '\33') {
            key = decode_escape_sequence();
            if (key == 0) {
                // Unknown keycode - ignore.
                continue;
            }
        }

        switch (key) {
        default:
            if (key >= ' ') {
                // Insert character into line.
                unsigned len = rpm_strwlen(buffer);
                if (len < buffer_length - 1) {
                    if (len > insert_pos) {
                        insert_character();
                    }
                    rpm_putwch(key);
                    memmove(&buffer[insert_pos+1], &buffer[insert_pos], (len - insert_pos + 1) * sizeof(uint16_t));
                    buffer[insert_pos] = key;
                    insert_pos++;
                }
            }
            break;

        case '\r': // Return
            // Cursor right for the remainder.
            for (;; insert_pos++) {
                uint16_t ch = buffer[insert_pos];
                if (ch == 0)
                    break;
                rpm_putwch(ch);
            }
            return key;

        case '\b': // ^H - Backspace on Linux
        case 0x7F: // 0177 - Backspace on Mac
            if (insert_pos > 0) {
                unsigned len = rpm_strwlen(buffer);
                if (insert_pos < len) {
                    rpm_putwch('\b');
                    delete_character();
                } else {
                    rpm_puts("\b \b");
                }
                insert_pos--;
                memmove(&buffer[insert_pos], &buffer[insert_pos+1], (len - insert_pos) * sizeof(uint16_t));
            }
            break;

        case CTRL('d'):    // ^D - Delete
        case DELETE_KEY: { // Delete on Linux
            unsigned len = rpm_strwlen(buffer);
            if (insert_pos < len) {
                delete_character();
                memmove(&buffer[insert_pos], &buffer[insert_pos+1], (len - insert_pos) * sizeof(uint16_t));
            }
            break;
        }
        case CTRL('b'):       // ^B - Cursor Left
        case LEFTWARDS_ARROW: // Arrow left
            if (insert_pos > 0) {
                rpm_putwch('\b');
                insert_pos--;
            }
            break;

        case CTRL('f'):          // ^F - Cursor Right
        case RIGHTWARDS_ARROW: { // Arrow right
            unsigned len = rpm_strwlen(buffer);
            if (insert_pos < len) {
                rpm_putwch(buffer[insert_pos]);
                insert_pos++;
            }
            break;
        }
        case CTRL('a'):        // ^A - Beginning of line
        case LEFTWARDS_TO_BAR: // Home
            while (insert_pos > 0) {
                rpm_putwch('\b');
                insert_pos--;
            }
            break;

        case CTRL('e'):           // ^E - End of line
        case RIGHTWARDS_TO_BAR: { // End
            unsigned len = rpm_strwlen(buffer);
            while (insert_pos < len) {
                rpm_putwch(buffer[insert_pos]);
                insert_pos++;
            }
            break;
        }
        case CTRL('u'): // ^U - Erase the line
            erase_line(buffer, &insert_pos);
            break;

        case CTRL('l'): // ^L - Refresh the line
            rpm_puts("\r\n");
            rpm_puts(prompt);
            rpm_wputs(buffer);
            for (int i = rpm_strwlen(buffer); i > insert_pos; i--) {
                rpm_putwch('\b');
            }
            break;

        case CTRL('p'):     // ^P - previous line from history
        case UPWARDS_ARROW: // Arrow up
            if (!on_prev_line && prev_line != 0) {
                // Save current line.
                rpm_strlcpy_unicode(next_line, buffer, sizeof(next_line)/sizeof(uint16_t));
                erase_line(buffer, &insert_pos);

                // Restore previous line.
                rpm_strlcpy_unicode(buffer, prev_line, sizeof(next_line)/sizeof(uint16_t));
                rpm_wputs(buffer);
                insert_pos = rpm_strwlen(buffer);
                on_prev_line = true;
            }
            break;

        case CTRL('n'):       // ^N - next line from history
        case DOWNWARDS_ARROW: // Arrow down
            if (on_prev_line) {
                // Save current line.
                rpm_strlcpy_unicode(prev_line, buffer, sizeof(next_line)/sizeof(uint16_t));
                erase_line(buffer, &insert_pos);

                // Restore next line.
                rpm_strlcpy_unicode(buffer, next_line, sizeof(next_line)/sizeof(uint16_t));
                rpm_wputs(buffer);
                insert_pos = rpm_strwlen(buffer);
                on_prev_line = false;
            }
            break;
        }
    }
}
