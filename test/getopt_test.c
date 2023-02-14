#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <cmocka.h>
#include <rpm/api.h>
#include <stdio.h>

void rpm_putchar(char ch)
{
    putchar(ch);
}

void rpm_puts(const char *input)
{
    fputs(input, stdout);
}

//
// prog -h
//
static void option_h(void **unused)
{
    char *argv[] = { "prog", "-h", 0 };
    int opt = rpm_getopt(2, argv, "h", NULL, NULL);
    assert_int_equal(opt, 'h');
}

//
// Run all tests.
//
int main()
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(option_h),
    };
    return cmocka_run_group_tests(tests, NULL, NULL);
}
