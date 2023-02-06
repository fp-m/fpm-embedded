#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <cmocka.h>
#include <rpm/api.h>
#include <rpm/internal.h>

static const char *input;       // Input stream for the current test, utf-8 encoded
static char output[1000];       // Output from the current test, utf-8 encoded
static int output_ptr;          // Current position
static char result[1000];       // Resulting command line, utf-8 encoded

//
// Get Unicode character from input buffer.
//
uint16_t rpm_getwch()
{
    if (!input || *input == 0) {
        fail_msg("No input");
    }

    // Decode utf-8 to unicode.
    uint8_t c1 = *input++;
    if (c1 < 0 || ! (c1 & 0x80))
        return c1;

    uint8_t c2 = *input++;
    if (! (c1 & 0x20))
        return (c1 & 0x1f) << 6 | (c2 & 0x3f);

    uint8_t c3 = *input++;
    return (c1 & 0x0f) << 12 | (c2 & 0x3f) << 6 | (c3 & 0x3f);
}

//
// Write Unicode character to output buffer.
//
void rpm_putwch(uint16_t ch)
{
    // Note extra space for zero byte.
    if (output_ptr >= sizeof(output) - 3) {
        fail_msg("Too much output");
    }

    // Convert to UTF-8 encoding.
    if (ch < 0x80) {
        output[output_ptr++] = ch;
    } else if (ch < 0x800) {
        output[output_ptr++] = ch >> 6 | 0xc0;
        output[output_ptr++] = (ch & 0x3f) | 0x80;
    } else {
        output[output_ptr++] = ch >> 12 | 0xe0;
        output[output_ptr++] = ((ch >> 6) & 0x3f) | 0x80;
        output[output_ptr++] = (ch & 0x3f) | 0x80;
    }
    output[output_ptr] = 0;
}

//
// Run one test with given input.
//
static void editline_test(const char *prompt, const char *inp)
{
    uint16_t cmd_line[128];
    input = inp;
    output_ptr = 0;
    output[0] = 0;
    rpm_editline(prompt, cmd_line, sizeof(cmd_line), true);
    rpm_strlcpy_to_utf8(result, cmd_line, sizeof(result));
}

static void editline_append(const char *initial, const char *inp)
{
    uint16_t cmd_line[128];
    input = inp;
    output_ptr = 0;
    output[0] = 0;
    rpm_strlcpy_from_utf8(cmd_line, initial, sizeof(cmd_line));
    rpm_editline(">", cmd_line, sizeof(cmd_line), false);
    rpm_strlcpy_to_utf8(result, cmd_line, sizeof(result));
}

static void empty_input(void **unused)
{
    editline_test(">", "\r");
    assert_string_equal(output, ">");
    assert_string_equal(result, "");
}

static void ascii_input(void **unused)
{
    editline_test(">", "foobar\r");
    assert_string_equal(output, ">foobar");
    assert_string_equal(result, "foobar");
}

static void append_input(void **unused)
{
    editline_append("foo", "bar\r");
    assert_string_equal(output, ">foobar");
    assert_string_equal(result, "foobar");
}

static void cursor_left_ctrlB(void **unused)
{
    editline_test(">", "foobar\2\2\2\r");
    assert_string_equal(output, ">foobar\b\b\bbar");
    assert_string_equal(result, "foobar");
}

static void cursor_right_ctrlF(void **unused)
{
    editline_test(">", "foobar\2\6\2\2\6\r");
    assert_string_equal(output, ">foobar\br\b\bar");
    assert_string_equal(result, "foobar");
}

static void input_in_the_middle(void **unused)
{
    editline_test(">", "foobar\2\2\2xyz\r");
    assert_string_equal(output, ">foobar\b\b\b\33[@x\33[@y\33[@zbar");
    assert_string_equal(result, "fooxyzbar");
}

static void backspace_ctrlH_at_the_end(void **unused)
{
    editline_test(">", "foobar\b\b\b\r");
    assert_string_equal(output, ">foobar\b \b\b \b\b \b");
    assert_string_equal(result, "foo");
}

static void backspace_ctrlH_in_the_middle(void **unused)
{
    editline_test(">", "foobar\2\2\2\b\b\r");
    assert_string_equal(output, ">foobar\b\b\b\b\33[P\b\33[Pbar");
    assert_string_equal(result, "fbar");
}

static void backspace_0177_at_the_end(void **unused)
{
    editline_test(">", "foobar\177\177\177\r");
    assert_string_equal(output, ">foobar\b \b\b \b\b \b");
    assert_string_equal(result, "foo");
}

static void backspace_0177_in_the_middle(void **unused)
{
    editline_test(">", "foobar\2\2\2\177\177\r");
    assert_string_equal(output, ">foobar\b\b\b\b\33[P\b\33[Pbar");
    assert_string_equal(result, "fbar");
}

static void delete_ctrlD_at_the_end(void **unused)
{
    editline_test(">", "foobar\4\4\4\r");
    assert_string_equal(output, ">foobar");
    assert_string_equal(result, "foobar");
}

static void delete_ctrlD_in_the_middle(void **unused)
{
    editline_test(">", "foobar\2\2\2\4\4\r");
    assert_string_equal(output, ">foobar\b\b\b\33[P\33[Pr");
    assert_string_equal(result, "foor");
}

static void beginning_of_line_ctrlA(void **unused)
{
    editline_test(">", "foobar\1\r");
    assert_string_equal(output, ">foobar\b\b\b\b\b\bfoobar");
    assert_string_equal(result, "foobar");
}

static void end_of_line_ctrlE(void **unused)
{
    editline_test(">", "foobar\1x\5y\r");
    assert_string_equal(output, ">foobar\b\b\b\b\b\b\33[@xfoobary");
    assert_string_equal(result, "xfoobary");
}

static void erase_the_line_ctrlU(void **unused)
{
    editline_test(">", "foobar\2\2\2\25abc\r");
    assert_string_equal(output, ">foobar\b\b\b\b\b\b\33[Kabc");
    assert_string_equal(result, "abc");
}

static void refresh_the_line_ctrlL(void **unused)
{
    editline_test(">", "foobar\2\14x\r");
    assert_string_equal(output, ">foobar\b\r\n>foobar\b\33[@xr");
    assert_string_equal(result, "foobaxr");
}

static void unicode_input(void **unused)
{
    editline_test(">", "Γεi\bιά\r");
    assert_string_equal(output, ">Γεi\b \bιά");
    assert_string_equal(result, "Γειά");
}

//
// Run all tests.
//
int main()
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(empty_input),
        cmocka_unit_test(ascii_input),
        cmocka_unit_test(append_input),
        cmocka_unit_test(cursor_left_ctrlB),
        cmocka_unit_test(cursor_right_ctrlF),
        cmocka_unit_test(input_in_the_middle),
        cmocka_unit_test(backspace_ctrlH_at_the_end),
        cmocka_unit_test(backspace_ctrlH_in_the_middle),
        cmocka_unit_test(backspace_0177_at_the_end),
        cmocka_unit_test(backspace_0177_in_the_middle),
        cmocka_unit_test(delete_ctrlD_at_the_end),
        cmocka_unit_test(delete_ctrlD_in_the_middle),
        cmocka_unit_test(beginning_of_line_ctrlA),
        cmocka_unit_test(end_of_line_ctrlE),
        cmocka_unit_test(erase_the_line_ctrlU),
        cmocka_unit_test(refresh_the_line_ctrlL),
        cmocka_unit_test(unicode_input),
        //cmocka_unit_test(),
    };
    return cmocka_run_group_tests(tests, NULL, NULL);
}
