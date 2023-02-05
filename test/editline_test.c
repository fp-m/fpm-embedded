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

//
// Empty input.
//
static void empty(void **unused)
{
    editline_test("\r");
    assert_string_equal(output, "");
    assert_string_equal(cmd_line, "");
}

//
// Run all tests.
//
int main()
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(empty),
    };
    return cmocka_run_group_tests(tests, NULL, NULL);
}
