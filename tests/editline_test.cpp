//
// Test fpm_editline() - command line editor.
//
#include <gtest/gtest.h>
#include <fpm/api.h>
#include <fpm/internal.h>

static const char *input;       // Input stream for the current test, utf-8 encoded
static char output[1000];       // Output from the current test, utf-8 encoded
static unsigned output_ptr;     // Current position
static char result[1000];       // Resulting command line, utf-8 encoded

//
// Get Unicode character from input buffer.
//
char fpm_getchar()
{
    if (input == nullptr || *input == 0) {
        // Should not happen.
        throw std::runtime_error("No input in fpm_getchar()");
    }
    return *input++;
}

//
// Write Unicode character to output buffer.
//
void fpm_putchar(char ch)
{
    // Note extra space for zero byte.
    if (output_ptr >= sizeof(output) - 3) {
        FAIL() << "Too much output";
    }
    output[output_ptr++] = ch;
    output[output_ptr] = 0;
}

//
// Run one test with given input.
//
static void editline_test(const char *prompt, const char *inp)
{
    uint16_t cmd_line[FPM_CMDLINE_SIZE];
    input = inp;
    output_ptr = 0;
    output[0] = 0;
    fpm_editline(cmd_line, sizeof(cmd_line), true, prompt, NULL);
    fpm_strlcpy_to_utf8(result, cmd_line, sizeof(result));
}

static void editline_append(const char *initial, const char *inp)
{
    uint16_t cmd_line[FPM_CMDLINE_SIZE];
    input = inp;
    output_ptr = 0;
    output[0] = 0;
    fpm_strlcpy_from_utf8(cmd_line, initial, sizeof(cmd_line));
    fpm_editline(cmd_line, sizeof(cmd_line), false, ">", NULL);
    fpm_strlcpy_to_utf8(result, cmd_line, sizeof(result));
}

static void editline_history(const char *prev_line, const char *inp)
{
    uint16_t cmd_line[FPM_CMDLINE_SIZE];
    uint16_t history[FPM_CMDLINE_SIZE];
    input = inp;
    output_ptr = 0;
    output[0] = 0;
    fpm_strlcpy_from_utf8(history, prev_line, sizeof(history)/sizeof(uint16_t));
    fpm_editline(cmd_line, sizeof(cmd_line), true, ">", history);
    fpm_strlcpy_to_utf8(result, cmd_line, sizeof(result));
}

TEST(editline, empty_input)
{
    editline_test(">", "\r");
    EXPECT_STREQ(output, ">");
    EXPECT_STREQ(result, "");
}

TEST(editline, ascii_input)
{
    editline_test(">", "foobar\r");
    EXPECT_STREQ(output, ">foobar");
    EXPECT_STREQ(result, "foobar");
}

TEST(editline, append_input)
{
    editline_append("foo", "bar\r");
    EXPECT_STREQ(output, ">foobar");
    EXPECT_STREQ(result, "foobar");
}

TEST(editline, cursor_left_ctrlB)
{
    editline_test(">", "foobar\2\2\2\r");
    EXPECT_STREQ(output, ">foobar\b\b\bbar");
    EXPECT_STREQ(result, "foobar");
}

TEST(editline, cursor_left_arrow)
{
    editline_test(">", "foobar\33[D\33[D\33OD\r");
    EXPECT_STREQ(output, ">foobar\b\b\bbar");
    EXPECT_STREQ(result, "foobar");
}

TEST(editline, cursor_right_ctrlF)
{
    editline_test(">", "foobar\2\6\2\2\6\r");
    EXPECT_STREQ(output, ">foobar\br\b\bar");
    EXPECT_STREQ(result, "foobar");
}

TEST(editline, cursor_right_arrow)
{
    editline_test(">", "foobar\2\33[C\2\2\33OC\r");
    EXPECT_STREQ(output, ">foobar\br\b\bar");
    EXPECT_STREQ(result, "foobar");
}

TEST(editline, input_in_the_middle)
{
    editline_test(">", "foobar\2\2\2xyz\r");
    EXPECT_STREQ(output, ">foobar\b\b\b\33[@x\33[@y\33[@zbar");
    EXPECT_STREQ(result, "fooxyzbar");
}

TEST(editline, backspace_ctrlH_at_the_end)
{
    editline_test(">", "foobar\b\b\b\r");
    EXPECT_STREQ(output, ">foobar\b \b\b \b\b \b");
    EXPECT_STREQ(result, "foo");
}

TEST(editline, backspace_ctrlH_in_the_middle)
{
    editline_test(">", "foobar\2\2\2\b\b\r");
    EXPECT_STREQ(output, ">foobar\b\b\b\b\33[P\b\33[Pbar");
    EXPECT_STREQ(result, "fbar");
}

TEST(editline, backspace_0177_at_the_end)
{
    editline_test(">", "foobar\177\177\177\r");
    EXPECT_STREQ(output, ">foobar\b \b\b \b\b \b");
    EXPECT_STREQ(result, "foo");
}

TEST(editline, backspace_0177_in_the_middle)
{
    editline_test(">", "foobar\2\2\2\177\177\r");
    EXPECT_STREQ(output, ">foobar\b\b\b\b\33[P\b\33[Pbar");
    EXPECT_STREQ(result, "fbar");
}

TEST(editline, delete_ctrlD_at_the_end)
{
    editline_test(">", "foobar\4\4\4\r");
    EXPECT_STREQ(output, ">foobar");
    EXPECT_STREQ(result, "foobar");
}

TEST(editline, delete_linux_at_the_end)
{
    editline_test(">", "foobar\33[3~\33[3~\33[3~\r");
    EXPECT_STREQ(output, ">foobar");
    EXPECT_STREQ(result, "foobar");
}

TEST(editline, delete_ctrlD_in_the_middle)
{
    editline_test(">", "foobar\2\2\2\4\4\r");
    EXPECT_STREQ(output, ">foobar\b\b\b\33[P\33[Pr");
    EXPECT_STREQ(result, "foor");
}

TEST(editline, delete_linux_in_the_middle)
{
    editline_test(">", "foobar\2\2\2\33[3~\33[3~\r");
    EXPECT_STREQ(output, ">foobar\b\b\b\33[P\33[Pr");
    EXPECT_STREQ(result, "foor");
}

TEST(editline, beginning_of_line_ctrlA)
{
    editline_test(">", "foobar\1\r");
    EXPECT_STREQ(output, ">foobar\b\b\b\b\b\bfoobar");
    EXPECT_STREQ(result, "foobar");
}

TEST(editline, beginning_of_line_home1)
{
    editline_test(">", "foobar\33[H\r");
    EXPECT_STREQ(output, ">foobar\b\b\b\b\b\bfoobar");
    EXPECT_STREQ(result, "foobar");
}

TEST(editline, beginning_of_line_home2)
{
    editline_test(">", "foobar\33OH\r");
    EXPECT_STREQ(output, ">foobar\b\b\b\b\b\bfoobar");
    EXPECT_STREQ(result, "foobar");
}

TEST(editline, end_of_line_ctrlE)
{
    editline_test(">", "foobar\1x\5y\r");
    EXPECT_STREQ(output, ">foobar\b\b\b\b\b\b\33[@xfoobary");
    EXPECT_STREQ(result, "xfoobary");
}

TEST(editline, end_of_line_home1)
{
    editline_test(">", "foobar\1x\33[Fy\r");
    EXPECT_STREQ(output, ">foobar\b\b\b\b\b\b\33[@xfoobary");
    EXPECT_STREQ(result, "xfoobary");
}

TEST(editline, end_of_line_home2)
{
    editline_test(">", "foobar\1x\33OFy\r");
    EXPECT_STREQ(output, ">foobar\b\b\b\b\b\b\33[@xfoobary");
    EXPECT_STREQ(result, "xfoobary");
}

TEST(editline, erase_the_line_ctrlU)
{
    editline_test(">", "foobar\2\2\2\25abc\r");
    EXPECT_STREQ(output, ">foobar\b\b\b\b\b\b\33[Kabc");
    EXPECT_STREQ(result, "abc");
}

TEST(editline, refresh_the_line_ctrlL)
{
    editline_test(">", "foobar\2\14x\r");
    EXPECT_STREQ(output, ">foobar\b\r\n>foobar\b\33[@xr");
    EXPECT_STREQ(result, "foobaxr");
}

TEST(editline, unicode_input)
{
    editline_test(">", "Γεi\bιά\r");
    EXPECT_STREQ(output, ">Γεi\b \bιά");
    EXPECT_STREQ(result, "Γειά");
}

TEST(editline, arrow_up1)
{
    editline_history("there", "foobar\33[A\r");
    EXPECT_STREQ(output, ">foobar\b\b\b\b\b\b\33[Kthere");
    EXPECT_STREQ(result, "there");
}

TEST(editline, arrow_up2)
{
    editline_history("there", "foobar\33OA2\r");
    EXPECT_STREQ(output, ">foobar\b\b\b\b\b\b\33[Kthere2");
    EXPECT_STREQ(result, "there2");
}

TEST(editline, arrow_down1)
{
    editline_history("there", "foobar\33[A\33[B\r");
    EXPECT_STREQ(output, ">foobar\b\b\b\b\b\b\33[Kthere\b\b\b\b\b\33[Kfoobar");
    EXPECT_STREQ(result, "foobar");
}

TEST(editline, arrow_down2)
{
    editline_history("there", "foobar\33OA\33OB2\r");
    EXPECT_STREQ(output, ">foobar\b\b\b\b\b\b\33[Kthere\b\b\b\b\b\33[Kfoobar2");
    EXPECT_STREQ(result, "foobar2");
}

TEST(editline, prev_line_ctrlP)
{
    editline_history("there", "foobar\20\r");
    EXPECT_STREQ(output, ">foobar\b\b\b\b\b\b\33[Kthere");
    EXPECT_STREQ(result, "there");
}

TEST(editline, next_line_ctrlN)
{
    editline_history("there", "foobar\20\16\r");
    EXPECT_STREQ(output, ">foobar\b\b\b\b\b\b\33[Kthere\b\b\b\b\b\33[Kfoobar");
    EXPECT_STREQ(result, "foobar");
}
