#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <cmocka.h>
#include <rpm/api.h>

static void tokenize_test(const char *input, int expect_argc, const char *expect_argv[], const char *expect_error)
{
    char buffer[100];
    char *argv[32] = { "some", "garbage" };
    int argc = 12345;

    strlcpy(buffer, input, sizeof(buffer));
    const char *error = rpm_tokenize(argv, &argc, buffer);

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
// "" -> nothing
//
static void empty_input(void **unused)
{
    tokenize_test("", 0, 0, 0);
}

//
// "   " -> nothing
//
static void spaces_only(void **unused)
{
    tokenize_test("   ", 0, 0, 0);
}

//
// "foo" -> "foo"
//
static void one_word(void **unused)
{
    const char *expect_argv[] = { "foo" };
    tokenize_test("foo", 1, expect_argv, 0);
}

//
// "  foo" -> "foo"
//
static void spaces_at_beginning(void **unused)
{
    const char *expect_argv[] = { "foo" };
    tokenize_test("  foo", 1, expect_argv, 0);
}

//
// "foo   " -> "foo"
//
static void spaces_at_end(void **unused)
{
    const char *expect_argv[] = { "foo" };
    tokenize_test("foo   ", 1, expect_argv, 0);
}

//
// "abra ca dabra" -> "abra", "ca", "dabra"
//
static void three_words(void **unused)
{
    const char *expect_argv[] = { "abra", "ca", "dabra" };
    tokenize_test("abra ca dabra", 3, expect_argv, 0);
}

//
// "abra  ca   dabra" -> "abra", "ca", "dabra"
//
static void extra_spaces(void **unused)
{
    const char *expect_argv[] = { "abra", "ca", "dabra" };
    tokenize_test("abra  ca   dabra", 3, expect_argv, 0);
}

//
// "\" -> error 'Incomplete backslash'
//
static void incomplete_backslash(void **unused)
{
    tokenize_test("\\", 0, 0, "Incomplete backslash");
}

//
// "foo\bar" -> "foobar"
//
static void backslash_char(void **unused)
{
    const char *expect_argv[] = { "foobar" };
    tokenize_test("foo\\bar", 1, expect_argv, 0);
}

//
// " \  \ b" -> " ", " b"
//
static void backslash_space_after_space(void **unused)
{
    const char *expect_argv[] = { " ", " b", };
    tokenize_test(" \\  \\ b", 2, expect_argv, 0);
}

//
// "foo\ bar\ b" -> "foo bar b"
//
static void backslash_space_after_char(void **unused)
{
    const char *expect_argv[] = { "foo bar b", };
    tokenize_test("foo\\ bar\\ b", 1, expect_argv, 0);
}

//
// Run all tests.
//
int main()
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(empty_input),
        cmocka_unit_test(spaces_only),
        cmocka_unit_test(one_word),
        cmocka_unit_test(spaces_at_beginning),
        cmocka_unit_test(spaces_at_end),
        cmocka_unit_test(three_words),
        cmocka_unit_test(extra_spaces),
        cmocka_unit_test(incomplete_backslash),
        cmocka_unit_test(backslash_char),
        cmocka_unit_test(backslash_space_after_space),
        cmocka_unit_test(backslash_space_after_char),
    };
    return cmocka_run_group_tests(tests, NULL, NULL);
}
