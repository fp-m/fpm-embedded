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
char rpm_getchar()
{
    if (!input || *input == 0) {
        fail_msg("No input");
    }
    return *input++;
}

//
// Write Unicode character to output buffer.
//
void rpm_putchar(char ch)
{
    // Note extra space for zero byte.
    if (output_ptr >= sizeof(output) - 3) {
        fail_msg("Too much output");
    }
    output[output_ptr++] = ch;
    output[output_ptr] = 0;
}

//
// Run one test with given input.
//
static void editline_test(const char *prompt, const char *inp)
{
    uint16_t cmd_line[RPM_CMDLINE_SIZE];
    input = inp;
    output_ptr = 0;
    output[0] = 0;
    rpm_editline(cmd_line, sizeof(cmd_line), true, prompt, NULL);
    rpm_strlcpy_to_utf8(result, cmd_line, sizeof(result));
}

static void editline_append(const char *initial, const char *inp)
{
    uint16_t cmd_line[RPM_CMDLINE_SIZE];
    input = inp;
    output_ptr = 0;
    output[0] = 0;
    rpm_strlcpy_from_utf8(cmd_line, initial, sizeof(cmd_line));
    rpm_editline(cmd_line, sizeof(cmd_line), false, ">", NULL);
    rpm_strlcpy_to_utf8(result, cmd_line, sizeof(result));
}

static void editline_history(const char *prev_line, const char *inp)
{
    uint16_t cmd_line[RPM_CMDLINE_SIZE];
    uint16_t history[RPM_CMDLINE_SIZE];
    input = inp;
    output_ptr = 0;
    output[0] = 0;
    rpm_strlcpy_from_utf8(history, prev_line, sizeof(history)/sizeof(uint16_t));
    rpm_editline(cmd_line, sizeof(cmd_line), true, ">", history);
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

static void cursor_left_arrow(void **unused)
{
    editline_test(">", "foobar\33[D\33[D\33OD\r");
    assert_string_equal(output, ">foobar\b\b\bbar");
    assert_string_equal(result, "foobar");
}

static void cursor_right_ctrlF(void **unused)
{
    editline_test(">", "foobar\2\6\2\2\6\r");
    assert_string_equal(output, ">foobar\br\b\bar");
    assert_string_equal(result, "foobar");
}

static void cursor_right_arrow(void **unused)
{
    editline_test(">", "foobar\2\33[C\2\2\33OC\r");
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

static void delete_linux_at_the_end(void **unused)
{
    editline_test(">", "foobar\33[3~\33[3~\33[3~\r");
    assert_string_equal(output, ">foobar");
    assert_string_equal(result, "foobar");
}

static void delete_ctrlD_in_the_middle(void **unused)
{
    editline_test(">", "foobar\2\2\2\4\4\r");
    assert_string_equal(output, ">foobar\b\b\b\33[P\33[Pr");
    assert_string_equal(result, "foor");
}

static void delete_linux_in_the_middle(void **unused)
{
    editline_test(">", "foobar\2\2\2\33[3~\33[3~\r");
    assert_string_equal(output, ">foobar\b\b\b\33[P\33[Pr");
    assert_string_equal(result, "foor");
}

static void beginning_of_line_ctrlA(void **unused)
{
    editline_test(">", "foobar\1\r");
    assert_string_equal(output, ">foobar\b\b\b\b\b\bfoobar");
    assert_string_equal(result, "foobar");
}

static void beginning_of_line_home1(void **unused)
{
    editline_test(">", "foobar\33[H\r");
    assert_string_equal(output, ">foobar\b\b\b\b\b\bfoobar");
    assert_string_equal(result, "foobar");
}

static void beginning_of_line_home2(void **unused)
{
    editline_test(">", "foobar\33OH\r");
    assert_string_equal(output, ">foobar\b\b\b\b\b\bfoobar");
    assert_string_equal(result, "foobar");
}

static void end_of_line_ctrlE(void **unused)
{
    editline_test(">", "foobar\1x\5y\r");
    assert_string_equal(output, ">foobar\b\b\b\b\b\b\33[@xfoobary");
    assert_string_equal(result, "xfoobary");
}

static void end_of_line_home1(void **unused)
{
    editline_test(">", "foobar\1x\33[Fy\r");
    assert_string_equal(output, ">foobar\b\b\b\b\b\b\33[@xfoobary");
    assert_string_equal(result, "xfoobary");
}

static void end_of_line_home2(void **unused)
{
    editline_test(">", "foobar\1x\33OFy\r");
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

static void arrow_up1(void **unused)
{
    editline_history("there", "foobar\33[A\r");
    assert_string_equal(output, ">foobar\b\b\b\b\b\b\33[Kthere");
    assert_string_equal(result, "there");
}

static void arrow_up2(void **unused)
{
    editline_history("there", "foobar\33OA2\r");
    assert_string_equal(output, ">foobar\b\b\b\b\b\b\33[Kthere2");
    assert_string_equal(result, "there2");
}

static void arrow_down1(void **unused)
{
    editline_history("there", "foobar\33[A\33[B\r");
    assert_string_equal(output, ">foobar\b\b\b\b\b\b\33[Kthere\b\b\b\b\b\33[Kfoobar");
    assert_string_equal(result, "foobar");
}

static void arrow_down2(void **unused)
{
    editline_history("there", "foobar\33OA\33OB2\r");
    assert_string_equal(output, ">foobar\b\b\b\b\b\b\33[Kthere\b\b\b\b\b\33[Kfoobar2");
    assert_string_equal(result, "foobar2");
}

static void prev_line_ctrlP(void **unused)
{
    editline_history("there", "foobar\20\r");
    assert_string_equal(output, ">foobar\b\b\b\b\b\b\33[Kthere");
    assert_string_equal(result, "there");
}

static void next_line_ctrlN(void **unused)
{
    editline_history("there", "foobar\20\16\r");
    assert_string_equal(output, ">foobar\b\b\b\b\b\b\33[Kthere\b\b\b\b\b\33[Kfoobar");
    assert_string_equal(result, "foobar");
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
        cmocka_unit_test(cursor_left_arrow),
        cmocka_unit_test(cursor_right_ctrlF),
        cmocka_unit_test(cursor_right_arrow),
        cmocka_unit_test(input_in_the_middle),
        cmocka_unit_test(backspace_ctrlH_at_the_end),
        cmocka_unit_test(backspace_ctrlH_in_the_middle),
        cmocka_unit_test(backspace_0177_at_the_end),
        cmocka_unit_test(backspace_0177_in_the_middle),
        cmocka_unit_test(delete_ctrlD_at_the_end),
        cmocka_unit_test(delete_linux_at_the_end),
        cmocka_unit_test(delete_ctrlD_in_the_middle),
        cmocka_unit_test(delete_linux_in_the_middle),
        cmocka_unit_test(beginning_of_line_ctrlA),
        cmocka_unit_test(beginning_of_line_home1),
        cmocka_unit_test(beginning_of_line_home2),
        cmocka_unit_test(end_of_line_ctrlE),
        cmocka_unit_test(end_of_line_home1),
        cmocka_unit_test(end_of_line_home2),
        cmocka_unit_test(erase_the_line_ctrlU),
        cmocka_unit_test(refresh_the_line_ctrlL),
        cmocka_unit_test(unicode_input),
        cmocka_unit_test(arrow_up1),
        cmocka_unit_test(arrow_up2),
        cmocka_unit_test(arrow_down1),
        cmocka_unit_test(arrow_down2),
        cmocka_unit_test(prev_line_ctrlP),
        cmocka_unit_test(next_line_ctrlN),
    };
    return cmocka_run_group_tests(tests, NULL, NULL);
}
