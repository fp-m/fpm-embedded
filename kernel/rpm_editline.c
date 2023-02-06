//
// Interactive line editor.
//
#include <rpm/api.h>

#define CTRL(c) (c & 037)

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
// The main line edit function
// Parameters:
// - buffer: Pointer to the line edit buffer
// - buffer_length: Size of the buffer in bytes
// - clear: Set to 0 to not clear, 1 to clear on entry
// Returns:
// - The exit key pressed (ESC or CR)
//
int rpm_editline(const char *prompt, char *buffer, unsigned buffer_length, bool clear)
{
    rpm_puts(prompt);
    if (clear) {
        buffer[0] = 0;
    } else {
        rpm_puts(buffer);
    }

    unsigned insert_pos = strlen(buffer);
    for (;;) {
        int key = rpm_getchar();

        switch (key) {
        default:
            if (key >= ' ' && key <= '~') {
                // Insert character into line.
                unsigned len = strlen(buffer);
                if (len < buffer_length - 1) {
                    if (len > insert_pos) {
                        insert_character();
                    }
                    rpm_putchar(key);
                    memmove(&buffer[insert_pos+1], &buffer[insert_pos], len - insert_pos + 1);
                    buffer[insert_pos] = key;
                    insert_pos++;
                }
            }
            break;

        case '\r': // Return
            // Cursor right for the remainder.
            for (;; insert_pos++) {
                int ch = buffer[insert_pos];
                if (ch == 0)
                    break;
                rpm_putchar(ch);
            }
            return key;

        case '\b': // ^H - Backspace on Linux
        case 0x7F: // 0177 - Backspace on Mac
            if (insert_pos > 0) {
                unsigned len = strlen(buffer);
                if (insert_pos < len) {
                    rpm_putchar('\b');
                    delete_character();
                } else {
                    rpm_puts("\b \b");
                }
                insert_pos--;
                memmove(&buffer[insert_pos], &buffer[insert_pos+1], len - insert_pos);
            }
            break;

        case CTRL('d'): { // ^D - Delete
            // TODO: Delete on Linux: Esc[3~
            unsigned len = strlen(buffer);
            if (insert_pos < len) {
                delete_character();
                memmove(&buffer[insert_pos], &buffer[insert_pos+1], len - insert_pos);
            }
            break;
        }
        case CTRL('b'): // ^B - Cursor Left
            // TODO: arrow left: Esc[D Esc[OD
            if (insert_pos > 0) {
                rpm_putchar('\b');
                insert_pos--;
            }
            break;

        case CTRL('f'): { // ^F - Cursor Right
            // TODO: arrow right: Esc[C Esc[OC
            unsigned len = strlen(buffer);
            if (insert_pos < len) {
                rpm_putchar(buffer[insert_pos]);
                insert_pos++;
            }
            break;
        }
        case CTRL('a'): // ^A - Beginning of line
            // TODO: Home: Esc[H EscOH
            while (insert_pos > 0) {
                rpm_putchar('\b');
                insert_pos--;
            }
            break;

        case CTRL('e'): { // ^E - End of line
            // TODO: End: Esc[F EscOF
            unsigned len = strlen(buffer);
            while (insert_pos < len) {
                rpm_putchar(buffer[insert_pos]);
                insert_pos++;
            }
            break;
        }
        case CTRL('u'): // ^U - Erase the line
            while (insert_pos > 0) {
                rpm_putchar('\b');
                insert_pos--;
            }
            if (buffer[0] != 0) {
                erase_till_end_of_line();
                buffer[0] = 0;
            }
            break;

        case CTRL('l'): // ^L - Refresh the line
            rpm_puts("\r\n");
            rpm_puts(prompt);
            rpm_puts(buffer);
            for (int i = strlen(buffer); i > insert_pos; i--) {
                rpm_putchar('\b');
            }
            break;

        case CTRL('p'): // ^P - previous line from history
            // TODO: arrow up: Esc[A EscOA
            break;

        case CTRL('n'): // ^N - next line from history
            // TODO: arrow down: Esc[B EscOB
            break;
        }
    }
}
