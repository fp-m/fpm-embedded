//
// Interactive line editor.
//
#include <fpm/api.h>

#define CTRL(c) (c & 037)

//
// Insert a character at current position,
// move the rest of the line to the right.
//
static void insert_character()
{
    fpm_puts("\33[@");
}

//
// Remove a character at the current position,
// move the rest of the line to the left.
//
static void delete_character()
{
    fpm_puts("\33[P");
}

//
// Erase the line starting from current position.
//
static void erase_till_end_of_line()
{
    fpm_puts("\33[K");
}

//
// "Unprint" the current line.
//
static void erase_line(uint16_t *buffer, unsigned *insert_pos)
{
    while (*insert_pos > 0) {
        fpm_putwch('\b');
        (*insert_pos)--;
    }
    if (buffer[0] != 0) {
        erase_till_end_of_line();
        buffer[0] = 0;
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
int fpm_editline(uint16_t *buffer, unsigned buffer_length, bool clear, const char *prompt, uint16_t *prev_line)
{
    uint16_t next_line[FPM_CMDLINE_SIZE];
    bool on_prev_line = false;

    fpm_puts(prompt);
    if (clear) {
        buffer[0] = 0;
    } else {
        fpm_wputs(buffer);
    }

    unsigned insert_pos = fpm_strwlen(buffer);
    for (;;) {
        // Get Unicode symbol, with function keys decoded.
        uint16_t key = fpm_getkey();

        switch (key) {
        default:
            if (key >= ' ') {
                // Insert character into line.
                unsigned len = fpm_strwlen(buffer);
                if (len < buffer_length - 1) {
                    if (len > insert_pos) {
                        insert_character();
                    }
                    fpm_putwch(key);
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
                fpm_putwch(ch);
            }
            return key;

        case '\b': // ^H - Backspace on Linux
        case 0x7F: // 0177 - Backspace on Mac
            if (insert_pos > 0) {
                unsigned len = fpm_strwlen(buffer);
                if (insert_pos < len) {
                    fpm_putwch('\b');
                    delete_character();
                } else {
                    fpm_puts("\b \b");
                }
                insert_pos--;
                memmove(&buffer[insert_pos], &buffer[insert_pos+1], (len - insert_pos) * sizeof(uint16_t));
            }
            break;

        case CTRL('d'):        // ^D - Delete
        case FPM_DELETE_KEY: { // Delete on Linux
            unsigned len = fpm_strwlen(buffer);
            if (insert_pos < len) {
                delete_character();
                memmove(&buffer[insert_pos], &buffer[insert_pos+1], (len - insert_pos) * sizeof(uint16_t));
            }
            break;
        }
        case CTRL('b'):           // ^B - Cursor Left
        case FPM_LEFTWARDS_ARROW: // Arrow left
            if (insert_pos > 0) {
                fpm_putwch('\b');
                insert_pos--;
            }
            break;

        case CTRL('f'):              // ^F - Cursor Right
        case FPM_RIGHTWARDS_ARROW: { // Arrow right
            unsigned len = fpm_strwlen(buffer);
            if (insert_pos < len) {
                fpm_putwch(buffer[insert_pos]);
                insert_pos++;
            }
            break;
        }
        case CTRL('a'):            // ^A - Beginning of line
        case FPM_LEFTWARDS_TO_BAR: // Home
            while (insert_pos > 0) {
                fpm_putwch('\b');
                insert_pos--;
            }
            break;

        case CTRL('e'):               // ^E - End of line
        case FPM_RIGHTWARDS_TO_BAR: { // End
            unsigned len = fpm_strwlen(buffer);
            while (insert_pos < len) {
                fpm_putwch(buffer[insert_pos]);
                insert_pos++;
            }
            break;
        }
        case CTRL('u'): // ^U - Erase the line
            erase_line(buffer, &insert_pos);
            break;

        case CTRL('l'): // ^L - Refresh the line
            fpm_puts("\r\n");
            fpm_puts(prompt);
            fpm_wputs(buffer);
            for (int i = fpm_strwlen(buffer); i > insert_pos; i--) {
                fpm_putwch('\b');
            }
            break;

        case CTRL('p'):         // ^P - previous line from history
        case FPM_UPWARDS_ARROW: // Arrow up
            if (!on_prev_line && prev_line != 0) {
                // Save current line.
                fpm_strlcpy_unicode(next_line, buffer, sizeof(next_line)/sizeof(uint16_t));
                erase_line(buffer, &insert_pos);

                // Restore previous line.
                fpm_strlcpy_unicode(buffer, prev_line, sizeof(next_line)/sizeof(uint16_t));
                fpm_wputs(buffer);
                insert_pos = fpm_strwlen(buffer);
                on_prev_line = true;
            }
            break;

        case CTRL('n'):           // ^N - next line from history
        case FPM_DOWNWARDS_ARROW: // Arrow down
            if (on_prev_line && prev_line != 0) {
                // Save current line.
                fpm_strlcpy_unicode(prev_line, buffer, sizeof(next_line)/sizeof(uint16_t));
                erase_line(buffer, &insert_pos);

                // Restore next line.
                fpm_strlcpy_unicode(buffer, next_line, sizeof(next_line)/sizeof(uint16_t));
                fpm_wputs(buffer);
                insert_pos = fpm_strwlen(buffer);
                on_prev_line = false;
            }
            break;
        }
    }
}
