//
// Test rpm_tokenize() - split a command line into arguments.
//
#include <gtest/gtest.h>
#include <rpm/api.h>

static void tokenize_test(const char *input, int expect_argc, const char *expect_argv[], const char *expect_error)
{
    char buffer[100];
    char *argv[32] = { (char*)"some", (char*)"garbage" };
    int argc = 12345;

    strncpy(buffer, input, sizeof(buffer));
    const char *error = rpm_tokenize(argv, &argc, buffer);

    if (expect_error) {
        // Error is expected.
        if (error)
            EXPECT_STREQ(error, expect_error);
        else
            FAIL() << "Error is expected, but got no error";
        return;
    }

    // Check argc, argc.
    EXPECT_EQ(argc, expect_argc);
    for (int i = 0; i < argc && i < expect_argc; i++) {
        EXPECT_STREQ(argv[i], expect_argv[i]);
    }
}

//
// "" -> nothing
//
TEST(tokenize, empty_input)
{
    tokenize_test("", 0, 0, 0);
}

//
// "   " -> nothing
//
TEST(tokenize, spaces_only)
{
    tokenize_test("   ", 0, 0, 0);
}

//
// "foo" -> "foo"
//
TEST(tokenize, one_word)
{
    const char *expect_argv[] = { "foo" };
    tokenize_test("foo", 1, expect_argv, 0);
}

//
// "  foo" -> "foo"
//
TEST(tokenize, spaces_at_beginning)
{
    const char *expect_argv[] = { "foo" };
    tokenize_test("  foo", 1, expect_argv, 0);
}

//
// "foo   " -> "foo"
//
TEST(tokenize, spaces_at_end)
{
    const char *expect_argv[] = { "foo" };
    tokenize_test("foo   ", 1, expect_argv, 0);
}

//
// "abra ca dabra" -> "abra", "ca", "dabra"
//
TEST(tokenize, three_words)
{
    const char *expect_argv[] = { "abra", "ca", "dabra" };
    tokenize_test("abra ca dabra", 3, expect_argv, 0);
}

//
// "abra  ca   dabra" -> "abra", "ca", "dabra"
//
TEST(tokenize, extra_spaces)
{
    const char *expect_argv[] = { "abra", "ca", "dabra" };
    tokenize_test("abra  ca   dabra", 3, expect_argv, 0);
}

//
// "\" -> error 'Incomplete backslash'
//
TEST(tokenize, incomplete_backslash)
{
    tokenize_test("\\", 0, 0, "Incomplete backslash");
}

//
// "foo\bar" -> "foobar"
//
TEST(tokenize, backslash_char)
{
    const char *expect_argv[] = { "foobar" };
    tokenize_test("foo\\bar", 1, expect_argv, 0);
}

//
// " \  \ b" -> " ", " b"
//
TEST(tokenize, backslash_space_after_space)
{
    const char *expect_argv[] = { " ", " b", };
    tokenize_test(" \\  \\ b", 2, expect_argv, 0);
}

//
// "foo\ bar\ b" -> "foo bar b"
//
TEST(tokenize, backslash_space_after_char)
{
    const char *expect_argv[] = { "foo bar b", };
    tokenize_test("foo\\ bar\\ b", 1, expect_argv, 0);
}

//
// "foo'bar" -> error 'Unterminated apostrophe'
//
TEST(tokenize, unterminated_apostrophe)
{
    tokenize_test("foo'bar", 0, 0, "Unterminated apostrophe");
}

//
// 'foo"bar' -> error 'Unterminated quote'
//
TEST(tokenize, unterminated_quote)
{
    tokenize_test("foo\"bar", 0, 0, "Unterminated quote");
}

//
// "foo b'a'r" -> "foo", "bar"
//
TEST(tokenize, valid_apostrophe)
{
    const char *expect_argv[] = { "foo", "bar", };
    tokenize_test("'foo' b'a'r", 2, expect_argv, 0);
}

//
// '"foo" b"a"r' -> "foo", "bar"
//
TEST(tokenize, valid_quote)
{
    const char *expect_argv[] = { "foo", "bar", };
    tokenize_test("\"foo\" b\"a\"r", 2, expect_argv, 0);
}

//
// '"fo'o" b"'a'"r' -> "fo'o", "b'a'r"
//
TEST(tokenize, apostrophe_inside_quotes)
{
    const char *expect_argv[] = { "fo'o", "b'a'r", };
    tokenize_test("\"fo'o\" b\"'a'\"r", 2, expect_argv, 0);
}

//
// "'fo"o' b'"a"'r' -> "fo"o", "b"a"r"
//
TEST(tokenize, quote_inside_apostrophes)
{
    const char *expect_argv[] = { "fo\"o", "b\"a\"r", };
    tokenize_test("'fo\"o' b'\"a\"'r", 2, expect_argv, 0);
}

TEST(tokenize, masked_apostrophe)
{
    const char *expect_argv[] = { "fo'o", "bar'", };
    tokenize_test("fo\\'o bar\\'", 2, expect_argv, 0);
}

TEST(tokenize, masked_quote)
{
    const char *expect_argv[] = { "fo\"o", "bar\"", };
    tokenize_test("fo\\\"o bar\\\"", 2, expect_argv, 0);
}
