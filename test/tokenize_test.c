#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <cmocka.h>
#include <rpm/api.h>

static void tokenize_test(char *input, int expect_argc, const char *expect_argv[])
{
    char *argv[32] = { "some", "garbage" };
    int argc = 12345;

    rpm_tokenize(argv, &argc, input);
    assert_int_equal(argc, expect_argc);
    for (int i = 0; i < argc && i < expect_argc; i++) {
        assert_string_equal(argv[i], expect_argv[i]);
    }
}

static void empty_input(void **unused)
{
    const char *expect_argv[] = { "" };
    tokenize_test("", 1, expect_argv);
}

//
// Run all tests.
//
int main()
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(empty_input),
    };
    return cmocka_run_group_tests(tests, NULL, NULL);
}
