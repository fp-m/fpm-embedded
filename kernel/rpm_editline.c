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
    rpm_puts("\33[1@");
}

//
// Remove a character at the current position,
// move the rest of the line to the left.
//
static void delete_character()
{
    rpm_puts("\33[1P");
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
int rpm_editline(char *buffer, unsigned buffer_length, bool clear)
{
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

        case '\r':
            // Return
            // Cursor right for the remainder
            for (;; insert_pos++) {
                int ch = buffer[insert_pos];
                if (ch == 0)
                    break;
                rpm_putchar(ch);
            }
            return key;

        case '\33':
            // Escape
            return key;

        case '\b':
        case 0x7F:
            // Backspace
            if (insert_pos > 0) {
                unsigned len = strlen(buffer);
                rpm_putchar('\b');
                delete_character();
                insert_pos--;
                memmove(&buffer[insert_pos], &buffer[insert_pos+1], len - insert_pos);
            }
            break;

        case CTRL('d'): {
            // Delete
            unsigned len = strlen(buffer);
            if (insert_pos < len) {
                delete_character();
                memmove(&buffer[insert_pos], &buffer[insert_pos+1], len - insert_pos);
            }
            break;
        }
        case CTRL('b'):
            // Cursor Left
            if (insert_pos > 0) {
                rpm_putchar('\b');
                insert_pos--;
            }
            break;

        case CTRL('f'): {
            // Cursor Right
            unsigned len = strlen(buffer);
            if (insert_pos < len) {
                rpm_putchar(buffer[insert_pos]);
                insert_pos++;
            }
            break;
        }
        case CTRL('a'):
            // Home
            // TODO: Begin of line
            break;

        case CTRL('e'):
            // End
            // TODO: End of line
            break;

        case CTRL('p'):
            // Cursor Up
            // TODO: previous line from history
            break;

        case CTRL('n'):
            // Cursor Down
            // TODO: next line from history
            break;
        }
    }
}
