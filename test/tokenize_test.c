#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <cmocka.h>
#include <rpm/api.h>

static void tokenize_test(char *input, int expect_argc, const char *expect_argv[], const char *expect_error)
{
    char *argv[32] = { "some", "garbage" };
    int argc = 12345;
    const char *error = rpm_tokenize(argv, &argc, input);

    if (expect_error) {
        // Error is expected.
        if (error)
            assert_string_equal(error, expect_error);
        else
            fail_msg("Error is expected, but got no error");
        return;
    }

    // Check argc, argc.
    assert_int_equal(argc, expect_argc);
    for (int i = 0; i < argc && i < expect_argc; i++) {
        assert_string_equal(argv[i], expect_argv[i]);
    }
}

//
// "" -> ""
//
static void empty_input(void **unused)
{
    const char *expect_argv[] = { "" };
    tokenize_test("", 1, expect_argv, 0);
}

//
// "\" -> error 'Incomplete backslash'
//
static void incomplete_backslash(void **unused)
{
    const char *expect_argv[] = { "" };
    tokenize_test("", 1, expect_argv, "Incomplete backslash");
}

//
// Run all tests.
//
int main()
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(empty_input),
        cmocka_unit_test(incomplete_backslash),
    };
    return cmocka_run_group_tests(tests, NULL, NULL);
}
