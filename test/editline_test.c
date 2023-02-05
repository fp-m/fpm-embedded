#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <cmocka.h>
#include <rpm/api.h>
#include <rpm/internal.h>

static const char *input;
static char output[1000];
static int output_ptr;
static char cmd_line[128];

//
// Get character from input buffer.
//
int rpm_getchar()
{
    if (!input || *input == 0) {
        fail_msg("No input");
    }
    return (uint8_t) *input++;
}

//
// Write character to output buffer.
//
void rpm_putchar(int ch)
{
    // Note extra space for zero byte.
    if (output_ptr >= sizeof(output) - 1) {
        fail_msg("Too much output");
    }
    output[output_ptr++] = ch;
    output[output_ptr] = 0;
}

void rpm_puts(const char *str)
{
    for (;;) {
        int ch = *str++;
        if (ch == 0)
            break;
        rpm_putchar(ch);
    }
}

//
// Run one test with given input.
//
static void editline_test(const char *inp)
{
    input = inp;
    output_ptr = 0;
    output[0] = 0;
    rpm_editline(cmd_line, sizeof(cmd_line), true);
}

static void editline_append(const char *initial, const char *inp)
{
    input = inp;
    output_ptr = 0;
    output[0] = 0;
    strlcpy(cmd_line, initial, sizeof(cmd_line));
    rpm_editline(cmd_line, sizeof(cmd_line), false);
}

static void empty_input(void **unused)
{
    editline_test("\r");
    assert_string_equal(output, "");
    assert_string_equal(cmd_line, "");
}

static void simple_input(void **unused)
{
    editline_test("foobar\r");
    assert_string_equal(output, "foobar");
    assert_string_equal(cmd_line, "foobar");
}

static void append_input(void **unused)
{
    editline_append("foo", "bar\r");
    assert_string_equal(output, "foobar");
    assert_string_equal(cmd_line, "foobar");
}

static void cursor_left_ctrlB(void **unused)
{
    editline_test("foobar\2\2\2\r");
    assert_string_equal(output, "foobar\b\b\bbar");
    assert_string_equal(cmd_line, "foobar");
}

static void cursor_right_ctrlF(void **unused)
{
    editline_test("foobar\2\6\2\2\6\r");
    assert_string_equal(output, "foobar\br\b\bar");
    assert_string_equal(cmd_line, "foobar");
}

static void input_in_the_middle(void **unused)
{
    editline_test("foobar\2\2\2xyz\r");
    assert_string_equal(output, "foobar\b\b\b\33[@x\33[@y\33[@zbar");
    assert_string_equal(cmd_line, "fooxyzbar");
}

static void backspace_ctrlH_at_the_end(void **unused)
{
    editline_test("foobar\b\b\b\r");
    assert_string_equal(output, "foobar\b \b\b \b\b \b");
    assert_string_equal(cmd_line, "foo");
}

static void backspace_ctrlH_in_the_middle(void **unused)
{
    editline_test("foobar\2\2\2\b\b\r");
    assert_string_equal(output, "foobar\b\b\b\b\33[P\b\33[Pbar");
    assert_string_equal(cmd_line, "fbar");
}

static void backspace_0177_at_the_end(void **unused)
{
    editline_test("foobar\177\177\177\r");
    assert_string_equal(output, "foobar\b \b\b \b\b \b");
    assert_string_equal(cmd_line, "foo");
}

static void backspace_0177_in_the_middle(void **unused)
{
    editline_test("foobar\2\2\2\177\177\r");
    assert_string_equal(output, "foobar\b\b\b\b\33[P\b\33[Pbar");
    assert_string_equal(cmd_line, "fbar");
}

static void delete_ctrlD_at_the_end(void **unused)
{
    editline_test("foobar\4\4\4\r");
    assert_string_equal(output, "foobar");
    assert_string_equal(cmd_line, "foobar");
}

static void delete_ctrlD_in_the_middle(void **unused)
{
    editline_test("foobar\2\2\2\4\4\r");
    assert_string_equal(output, "foobar\b\b\b\33[P\33[Pr");
    assert_string_equal(cmd_line, "foor");
}

static void beginning_of_line_ctrlA(void **unused)
{
    editline_test("foobar\1\r");
    assert_string_equal(output, "foobar\b\b\b\b\b\bfoobar");
    assert_string_equal(cmd_line, "foobar");
}

static void end_of_line_ctrlE(void **unused)
{
    editline_test("foobar\1x\5y\r");
    assert_string_equal(output, "foobar\b\b\b\b\b\b\33[@xfoobary");
    assert_string_equal(cmd_line, "xfoobary");
}

static void erase_the_line_ctrlU(void **unused)
{
    editline_test("foobar\2\2\2\25abc\r");
    assert_string_equal(output, "foobar\b\b\b\b\b\b\33[Kabc");
    assert_string_equal(cmd_line, "abc");
}

static void refresh_the_line_ctrlL(void **unused)
{
    editline_test("foobar\2\14x\r");
    assert_string_equal(output, "foobar\b\r\nfoobar\b\33[@xr");
    assert_string_equal(cmd_line, "foobaxr");
}

//
// Run all tests.
//
int main()
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(empty_input),
        cmocka_unit_test(simple_input),
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
        //cmocka_unit_test(),
    };
    return cmocka_run_group_tests(tests, NULL, NULL);
}
