//
// Test fpm_getopt() - parser of CLI options and arguments.
//
#include <gtest/gtest.h>
#include <fpm/api.h>
#include <fpm/getopt.h>

//
// Buffer for stdout messages from fpm_getopt().
//
static char output[256];
static char *out_ptr = output;

void fpm_putchar(char ch)
{
    *out_ptr++ = ch;
    *out_ptr = '\0';
}

void fpm_puts(const char *input)
{
    strcpy(out_ptr, input);
    out_ptr += strlen(input);
}

//
// Reset getopt state for the next test.
//
static void opt_reset()
{
    out_ptr = output;
    *out_ptr = '\0';
}

//
// prog -h --- valid option
//
TEST(getopt, option_h_valid)
{
    opt_reset();
    const char *argv[] = { "prog", "-h", 0 };
    struct fpm_opt opt = {};

    int result = fpm_getopt(2, (char**)argv, "h", nullptr, &opt);
    EXPECT_EQ(result, 'h');
    EXPECT_EQ(opt.ret, result);
    EXPECT_EQ(opt.ind, 2);
    EXPECT_EQ(opt.opt, 'h');
    EXPECT_EQ(opt.arg, nullptr);
    EXPECT_STREQ(output, "");
}

//
// prog -h --- missing option
//
TEST(getopt, option_h_missing)
{
    opt_reset();
    const char *argv[] = { "prog", "-h", 0 };
    struct fpm_opt opt = {};

    int result = fpm_getopt(2, (char**)argv, "a", nullptr, &opt);
    EXPECT_EQ(result, '?');
    EXPECT_EQ(opt.ret, result);
    EXPECT_EQ(opt.opt, '?');
    EXPECT_STREQ(output, "prog: Unknown option `-h`\r\n");
}

//
// prog -i 1
//
TEST(getopt, option_i_1)
{
    opt_reset();
    const char *argv[] = { "prog", "-i", "1", 0 };
    struct fpm_opt opt = {};

    int result = fpm_getopt(3, (char**)argv, "i:", nullptr, &opt);
    EXPECT_EQ(result, 'i');
    EXPECT_EQ(opt.ret, result);
    EXPECT_EQ(opt.opt, 'i');
    EXPECT_STREQ(opt.arg, "1");
    EXPECT_STREQ(output, "");
}

//
// prog
//
TEST(getopt, no_args)
{
    opt_reset();
    const char *argv[] = { "prog", 0 };
    struct fpm_opt opt = {};

    int result = fpm_getopt(1, (char**)argv, "ab:c:def:j", nullptr, &opt);
    EXPECT_EQ(result, -1);
    EXPECT_EQ(opt.ret, result);
    EXPECT_EQ(opt.ind, 1);
    EXPECT_EQ(opt.opt, 0);
    EXPECT_EQ(opt.arg, nullptr);
}

//
// prog x y z
//
TEST(getopt, three_args)
{
    opt_reset();
    const char *argv[] = { "prog", "x", "y", "z", 0 };
    int argc = 4;
    struct fpm_opt opt = {};

    int result = fpm_getopt(argc, (char**)argv, "ab:c:def:j", nullptr, &opt);
    EXPECT_EQ(result, 1);
    EXPECT_EQ(opt.ret, result);
    EXPECT_EQ(opt.ind, 2);
    EXPECT_EQ(opt.opt, 1);
    EXPECT_STREQ(opt.arg, "x");

    result = fpm_getopt(argc, (char**)argv, "ab:c:def:j", nullptr, &opt);
    EXPECT_EQ(result, 1);
    EXPECT_EQ(opt.ret, result);
    EXPECT_EQ(opt.ind, 3);
    EXPECT_EQ(opt.opt, 1);
    EXPECT_STREQ(opt.arg, "y");

    result = fpm_getopt(argc, (char**)argv, "ab:c:def:j", nullptr, &opt);
    EXPECT_EQ(result, 1);
    EXPECT_EQ(opt.ret, result);
    EXPECT_EQ(opt.ind, 4);
    EXPECT_EQ(opt.opt, 1);
    EXPECT_STREQ(opt.arg, "z");

    result = fpm_getopt(argc, (char**)argv, "ab:c:def:j", nullptr, &opt);
    EXPECT_EQ(result, -1);
    EXPECT_EQ(opt.ret, result);
    EXPECT_EQ(opt.ind, 4);
    EXPECT_EQ(opt.opt, 0);
    EXPECT_EQ(opt.arg, nullptr);
}

//
// prog -a
//
TEST(getopt, simple_option)
{
    opt_reset();
    const char *argv[] = { "prog", "-a", 0 };
    int argc = 2;
    struct fpm_opt opt = {};

    int result = fpm_getopt(argc, (char**)argv, "ab:c:def:j", nullptr, &opt);
    EXPECT_EQ(result, 'a');
    EXPECT_EQ(opt.ret, result);
    EXPECT_EQ(opt.ind, 2);
    EXPECT_EQ(opt.opt, 'a');
    EXPECT_EQ(opt.arg, nullptr);

    result = fpm_getopt(argc, (char**)argv, "ab:c:def:j", nullptr, &opt);
    EXPECT_EQ(result, -1);
    EXPECT_EQ(opt.ret, result);
    EXPECT_EQ(opt.ind, 2);
    EXPECT_EQ(opt.opt, 0);
    EXPECT_EQ(opt.arg, nullptr);
}

//
// prog -a x y z
//
TEST(getopt, simple_option_with_args)
{
    opt_reset();
    const char *argv[] = { "prog", "-a", "x", "y", "z", 0 };
    int argc = 5;
    struct fpm_opt opt = {};

    int result = fpm_getopt(argc, (char**)argv, "ab:c:def:j", nullptr, &opt);
    EXPECT_EQ(result, 'a');
    EXPECT_EQ(opt.ret, result);
    EXPECT_EQ(opt.ind, 2);
    EXPECT_EQ(opt.opt, 'a');
    EXPECT_EQ(opt.arg, nullptr);

    result = fpm_getopt(argc, (char**)argv, "ab:c:def:j", nullptr, &opt);
    EXPECT_EQ(result, 1);
    EXPECT_EQ(opt.ret, result);
    EXPECT_EQ(opt.ind, 3);
    EXPECT_EQ(opt.opt, 1);
    EXPECT_STREQ(opt.arg, "x");

    result = fpm_getopt(argc, (char**)argv, "ab:c:def:j", nullptr, &opt);
    EXPECT_EQ(result, 1);
    EXPECT_EQ(opt.ret, result);
    EXPECT_EQ(opt.ind, 4);
    EXPECT_EQ(opt.opt, 1);
    EXPECT_STREQ(opt.arg, "y");

    result = fpm_getopt(argc, (char**)argv, "ab:c:def:j", nullptr, &opt);
    EXPECT_EQ(result, 1);
    EXPECT_EQ(opt.ret, result);
    EXPECT_EQ(opt.ind, 5);
    EXPECT_EQ(opt.opt, 1);
    EXPECT_STREQ(opt.arg, "z");

    result = fpm_getopt(argc, (char**)argv, "ab:c:def:j", nullptr, &opt);
    EXPECT_EQ(result, -1);
    EXPECT_EQ(opt.ret, result);
    EXPECT_EQ(opt.ind, 5);
    EXPECT_EQ(opt.opt, 0);
    EXPECT_EQ(opt.arg, nullptr);
}

//
// prog -ade -j x y z
//
TEST(getopt, many_simple_options)
{
    opt_reset();
    const char *argv[] = { "prog", "-ade", "-j", "x", "y", "z", 0 };
    int argc = 6;
    struct fpm_opt opt = {};

    int result = fpm_getopt(argc, (char**)argv, "ab:c:def:j", nullptr, &opt);
    EXPECT_EQ(result, 'a');
    EXPECT_EQ(opt.ret, result);
    EXPECT_EQ(opt.ind, 1);
    EXPECT_EQ(opt.opt, 'a');
    EXPECT_EQ(opt.arg, nullptr);

    result = fpm_getopt(argc, (char**)argv, "ab:c:def:j", nullptr, &opt);
    EXPECT_EQ(result, 'd');
    EXPECT_EQ(opt.ret, result);
    EXPECT_EQ(opt.ind, 1);
    EXPECT_EQ(opt.opt, 'd');
    EXPECT_EQ(opt.arg, nullptr);

    result = fpm_getopt(argc, (char**)argv, "ab:c:def:j", nullptr, &opt);
    EXPECT_EQ(result, 'e');
    EXPECT_EQ(opt.ret, result);
    EXPECT_EQ(opt.ind, 2);
    EXPECT_EQ(opt.opt, 'e');
    EXPECT_EQ(opt.arg, nullptr);

    result = fpm_getopt(argc, (char**)argv, "ab:c:def:j", nullptr, &opt);
    EXPECT_EQ(result, 'j');
    EXPECT_EQ(opt.ret, result);
    EXPECT_EQ(opt.ind, 3);
    EXPECT_EQ(opt.opt, 'j');
    EXPECT_EQ(opt.arg, nullptr);

    result = fpm_getopt(argc, (char**)argv, "ab:c:def:j", nullptr, &opt);
    EXPECT_EQ(result, 1);
    EXPECT_EQ(opt.ret, result);
    EXPECT_EQ(opt.ind, 4);
    EXPECT_EQ(opt.opt, 1);
    EXPECT_STREQ(opt.arg, "x");

    result = fpm_getopt(argc, (char**)argv, "ab:c:def:j", nullptr, &opt);
    EXPECT_EQ(result, 1);
    EXPECT_EQ(opt.ret, result);
    EXPECT_EQ(opt.ind, 5);
    EXPECT_EQ(opt.opt, 1);
    EXPECT_STREQ(opt.arg, "y");

    result = fpm_getopt(argc, (char**)argv, "ab:c:def:j", nullptr, &opt);
    EXPECT_EQ(result, 1);
    EXPECT_EQ(opt.ret, result);
    EXPECT_EQ(opt.ind, 6);
    EXPECT_EQ(opt.opt, 1);
    EXPECT_STREQ(opt.arg, "z");

    result = fpm_getopt(argc, (char**)argv, "ab:c:def:j", nullptr, &opt);
    EXPECT_EQ(result, -1);
    EXPECT_EQ(opt.ret, result);
    EXPECT_EQ(opt.ind, 6);
    EXPECT_EQ(opt.opt, 0);
    EXPECT_EQ(opt.arg, nullptr);
}

//
// prog -abbval -c cval -def fval -j
//
TEST(getopt, option_mix_without_args)
{
    opt_reset();
    const char *argv[] = { "prog", "-abbval", "-c", "cval", "-def", "fval", "-j", 0 };
    int argc = 7;
    struct fpm_opt opt = {};

    int result = fpm_getopt(argc, (char**)argv, "ab:c:def:j", nullptr, &opt);
    EXPECT_EQ(result, 'a');
    EXPECT_EQ(opt.ret, result);
    EXPECT_EQ(opt.ind, 1);
    EXPECT_EQ(opt.opt, 'a');
    EXPECT_EQ(opt.arg, nullptr);

    result = fpm_getopt(argc, (char**)argv, "ab:c:def:j", nullptr, &opt);
    EXPECT_EQ(result, 'b');
    EXPECT_EQ(opt.ret, result);
    EXPECT_EQ(opt.ind, 2);
    EXPECT_EQ(opt.opt, 'b');
    EXPECT_STREQ(opt.arg, "bval");

    result = fpm_getopt(argc, (char**)argv, "ab:c:def:j", nullptr, &opt);
    EXPECT_EQ(result, 'c');
    EXPECT_EQ(opt.ret, result);
    EXPECT_EQ(opt.ind, 4);
    EXPECT_EQ(opt.opt, 'c');
    EXPECT_STREQ(opt.arg, "cval");

    result = fpm_getopt(argc, (char**)argv, "ab:c:def:j", nullptr, &opt);
    EXPECT_EQ(result, 'd');
    EXPECT_EQ(opt.ret, result);
    EXPECT_EQ(opt.ind, 4);
    EXPECT_EQ(opt.opt, 'd');
    EXPECT_EQ(opt.arg, nullptr);

    result = fpm_getopt(argc, (char**)argv, "ab:c:def:j", nullptr, &opt);
    EXPECT_EQ(result, 'e');
    EXPECT_EQ(opt.ret, result);
    EXPECT_EQ(opt.ind, 4);
    EXPECT_EQ(opt.opt, 'e');
    EXPECT_EQ(opt.arg, nullptr);

    result = fpm_getopt(argc, (char**)argv, "ab:c:def:j", nullptr, &opt);
    EXPECT_EQ(result, 'f');
    EXPECT_EQ(opt.ret, result);
    EXPECT_EQ(opt.ind, 6);
    EXPECT_EQ(opt.opt, 'f');
    EXPECT_STREQ(opt.arg, "fval");

    result = fpm_getopt(argc, (char**)argv, "ab:c:def:j", nullptr, &opt);
    EXPECT_EQ(result, 'j');
    EXPECT_EQ(opt.ret, result);
    EXPECT_EQ(opt.ind, 7);
    EXPECT_EQ(opt.opt, 'j');
    EXPECT_EQ(opt.arg, nullptr);

    result = fpm_getopt(argc, (char**)argv, "ab:c:def:j", nullptr, &opt);
    EXPECT_EQ(result, -1);
    EXPECT_EQ(opt.ret, result);
    EXPECT_EQ(opt.ind, 7);
    EXPECT_EQ(opt.opt, 0);
    EXPECT_EQ(opt.arg, nullptr);
}

//
// prog -abbval -c cval -def fval -j x y
//
TEST(getopt, option_mix_with_args)
{
    opt_reset();
    const char *argv[] = { "prog", "-abbval", "-c", "cval", "-def", "fval", "-j", "x", "y", 0 };
    int argc = 9;
    struct fpm_opt opt = {};

    int result = fpm_getopt(argc, (char**)argv, "ab:c:def:j", nullptr, &opt);
    EXPECT_EQ(result, 'a');
    EXPECT_EQ(opt.ret, result);
    EXPECT_EQ(opt.ind, 1);
    EXPECT_EQ(opt.opt, 'a');
    EXPECT_EQ(opt.arg, nullptr);

    result = fpm_getopt(argc, (char**)argv, "ab:c:def:j", nullptr, &opt);
    EXPECT_EQ(result, 'b');
    EXPECT_EQ(opt.ret, result);
    EXPECT_EQ(opt.ind, 2);
    EXPECT_EQ(opt.opt, 'b');
    EXPECT_STREQ(opt.arg, "bval");

    result = fpm_getopt(argc, (char**)argv, "ab:c:def:j", nullptr, &opt);
    EXPECT_EQ(result, 'c');
    EXPECT_EQ(opt.ret, result);
    EXPECT_EQ(opt.ind, 4);
    EXPECT_EQ(opt.opt, 'c');
    EXPECT_STREQ(opt.arg, "cval");

    result = fpm_getopt(argc, (char**)argv, "ab:c:def:j", nullptr, &opt);
    EXPECT_EQ(result, 'd');
    EXPECT_EQ(opt.ret, result);
    EXPECT_EQ(opt.ind, 4);
    EXPECT_EQ(opt.opt, 'd');
    EXPECT_EQ(opt.arg, nullptr);

    result = fpm_getopt(argc, (char**)argv, "ab:c:def:j", nullptr, &opt);
    EXPECT_EQ(result, 'e');
    EXPECT_EQ(opt.ret, result);
    EXPECT_EQ(opt.ind, 4);
    EXPECT_EQ(opt.opt, 'e');
    EXPECT_EQ(opt.arg, nullptr);

    result = fpm_getopt(argc, (char**)argv, "ab:c:def:j", nullptr, &opt);
    EXPECT_EQ(result, 'f');
    EXPECT_EQ(opt.ret, result);
    EXPECT_EQ(opt.ind, 6);
    EXPECT_EQ(opt.opt, 'f');
    EXPECT_STREQ(opt.arg, "fval");

    result = fpm_getopt(argc, (char**)argv, "ab:c:def:j", nullptr, &opt);
    EXPECT_EQ(result, 'j');
    EXPECT_EQ(opt.ret, result);
    EXPECT_EQ(opt.ind, 7);
    EXPECT_EQ(opt.opt, 'j');
    EXPECT_EQ(opt.arg, nullptr);

    result = fpm_getopt(argc, (char**)argv, "ab:c:def:j", nullptr, &opt);
    EXPECT_EQ(result, 1);
    EXPECT_EQ(opt.ret, result);
    EXPECT_EQ(opt.ind, 8);
    EXPECT_EQ(opt.opt, 1);
    EXPECT_STREQ(opt.arg, "x");

    result = fpm_getopt(argc, (char**)argv, "ab:c:def:j", nullptr, &opt);
    EXPECT_EQ(result, 1);
    EXPECT_EQ(opt.ret, result);
    EXPECT_EQ(opt.ind, 9);
    EXPECT_EQ(opt.opt, 1);
    EXPECT_STREQ(opt.arg, "y");

    result = fpm_getopt(argc, (char**)argv, "ab:c:def:j", nullptr, &opt);
    EXPECT_EQ(result, -1);
    EXPECT_EQ(opt.ret, result);
    EXPECT_EQ(opt.ind, 9);
    EXPECT_EQ(opt.opt, 0);
    EXPECT_EQ(opt.arg, nullptr);
}

//
// prog -abbval x -c cval -def fval y -j z
//
TEST(getopt, option_mix_shuffled_args)
{
    opt_reset();
    const char *argv[] = { "prog", "-abbval", "x", "-c", "cval", "-def", "fval", "y", "-j", "z", 0 };
    int argc = 10;
    struct fpm_opt opt = {};

    int result = fpm_getopt(argc, (char**)argv, "ab:c:def:j", nullptr, &opt);
    EXPECT_EQ(result, 'a');
    EXPECT_EQ(opt.ret, result);
    EXPECT_EQ(opt.ind, 1);
    EXPECT_EQ(opt.opt, 'a');
    EXPECT_EQ(opt.arg, nullptr);

    result = fpm_getopt(argc, (char**)argv, "ab:c:def:j", nullptr, &opt);
    EXPECT_EQ(result, 'b');
    EXPECT_EQ(opt.ret, result);
    EXPECT_EQ(opt.ind, 2);
    EXPECT_EQ(opt.opt, 'b');
    EXPECT_STREQ(opt.arg, "bval");

    result = fpm_getopt(argc, (char**)argv, "ab:c:def:j", nullptr, &opt);
    EXPECT_EQ(result, 1);
    EXPECT_EQ(opt.ret, result);
    EXPECT_EQ(opt.ind, 3);
    EXPECT_EQ(opt.opt, 1);
    EXPECT_STREQ(opt.arg, "x");

    result = fpm_getopt(argc, (char**)argv, "ab:c:def:j", nullptr, &opt);
    EXPECT_EQ(result, 'c');
    EXPECT_EQ(opt.ret, result);
    EXPECT_EQ(opt.ind, 5);
    EXPECT_EQ(opt.opt, 'c');
    EXPECT_STREQ(opt.arg, "cval");

    result = fpm_getopt(argc, (char**)argv, "ab:c:def:j", nullptr, &opt);
    EXPECT_EQ(result, 'd');
    EXPECT_EQ(opt.ret, result);
    EXPECT_EQ(opt.ind, 5);
    EXPECT_EQ(opt.opt, 'd');
    EXPECT_EQ(opt.arg, nullptr);

    result = fpm_getopt(argc, (char**)argv, "ab:c:def:j", nullptr, &opt);
    EXPECT_EQ(result, 'e');
    EXPECT_EQ(opt.ret, result);
    EXPECT_EQ(opt.ind, 5);
    EXPECT_EQ(opt.opt, 'e');
    EXPECT_EQ(opt.arg, nullptr);

    result = fpm_getopt(argc, (char**)argv, "ab:c:def:j", nullptr, &opt);
    EXPECT_EQ(result, 'f');
    EXPECT_EQ(opt.ret, result);
    EXPECT_EQ(opt.ind, 7);
    EXPECT_EQ(opt.opt, 'f');
    EXPECT_STREQ(opt.arg, "fval");

    result = fpm_getopt(argc, (char**)argv, "ab:c:def:j", nullptr, &opt);
    EXPECT_EQ(result, 1);
    EXPECT_EQ(opt.ret, result);
    EXPECT_EQ(opt.ind, 8);
    EXPECT_EQ(opt.opt, 1);
    EXPECT_STREQ(opt.arg, "y");

    result = fpm_getopt(argc, (char**)argv, "ab:c:def:j", nullptr, &opt);
    EXPECT_EQ(result, 'j');
    EXPECT_EQ(opt.ret, result);
    EXPECT_EQ(opt.ind, 9);
    EXPECT_EQ(opt.opt, 'j');
    EXPECT_EQ(opt.arg, nullptr);

    result = fpm_getopt(argc, (char**)argv, "ab:c:def:j", nullptr, &opt);
    EXPECT_EQ(result, 1);
    EXPECT_EQ(opt.ret, result);
    EXPECT_EQ(opt.ind, 10);
    EXPECT_EQ(opt.opt, 1);
    EXPECT_STREQ(opt.arg, "z");

    result = fpm_getopt(argc, (char**)argv, "ab:c:def:j", nullptr, &opt);
    EXPECT_EQ(result, -1);
    EXPECT_EQ(opt.ret, result);
    EXPECT_EQ(opt.ind, 10);
    EXPECT_EQ(opt.opt, 0);
    EXPECT_EQ(opt.arg, nullptr);
}

//
// prog -ax -bbval y
//
TEST(getopt, simple_option_and_arg_without_spaces)
{
    opt_reset();
    const char *argv[] = { "prog", "-ax", "-bbval", "y", 0 };
    int argc = 4;
    struct fpm_opt opt = {};

    int result = fpm_getopt(argc, (char**)argv, "ab:c:def:j", nullptr, &opt);
    EXPECT_EQ(result, 'a');
    EXPECT_EQ(opt.ret, result);
    EXPECT_EQ(opt.ind, 1);
    EXPECT_EQ(opt.opt, 'a');
    EXPECT_EQ(opt.arg, nullptr);

    result = fpm_getopt(argc, (char**)argv, "ab:c:def:j", nullptr, &opt);
    EXPECT_EQ(result, '?');
    EXPECT_EQ(opt.ret, result);
    EXPECT_EQ(opt.ind, 2);
    EXPECT_EQ(opt.opt, '?');
    EXPECT_STREQ(output, "prog: Unknown option `-x`\r\n");

    result = fpm_getopt(argc, (char**)argv, "ab:c:def:j", nullptr, &opt);
    EXPECT_EQ(result, 'b');
    EXPECT_EQ(opt.ret, result);
    EXPECT_EQ(opt.ind, 3);
    EXPECT_EQ(opt.opt, 'b');
    EXPECT_STREQ(opt.arg, "bval");

    result = fpm_getopt(argc, (char**)argv, "ab:c:def:j", nullptr, &opt);
    EXPECT_EQ(result, 1);
    EXPECT_EQ(opt.ret, result);
    EXPECT_EQ(opt.ind, 4);
    EXPECT_EQ(opt.opt, 1);
    EXPECT_STREQ(opt.arg, "y");

    result = fpm_getopt(argc, (char**)argv, "ab:c:def:j", nullptr, &opt);
    EXPECT_EQ(result, -1);
    EXPECT_EQ(opt.ret, result);
    EXPECT_EQ(opt.ind, 4);
    EXPECT_EQ(opt.opt, 0);
    EXPECT_EQ(opt.arg, nullptr);
}

//
// prog -a -bbval -- -f x
//
TEST(getopt, separator_valid)
{
    opt_reset();
    const char *argv[] = { "prog", "-a", "-bbval", "--", "-f", "x", 0 };
    int argc = 6;
    struct fpm_opt opt = {};

    int result = fpm_getopt(argc, (char**)argv, "ab:c:def:j", nullptr, &opt);
    EXPECT_EQ(result, 'a');
    EXPECT_EQ(opt.ret, result);
    EXPECT_EQ(opt.ind, 2);
    EXPECT_EQ(opt.opt, 'a');
    EXPECT_EQ(opt.arg, nullptr);

    result = fpm_getopt(argc, (char**)argv, "ab:c:def:j", nullptr, &opt);
    EXPECT_EQ(result, 'b');
    EXPECT_EQ(opt.ret, result);
    EXPECT_EQ(opt.ind, 3);
    EXPECT_EQ(opt.opt, 'b');
    EXPECT_STREQ(opt.arg, "bval");

    result = fpm_getopt(argc, (char**)argv, "ab:c:def:j", nullptr, &opt);
    EXPECT_EQ(result, -1);
    EXPECT_EQ(opt.ret, result);
    EXPECT_EQ(opt.ind, 4);
    EXPECT_EQ(opt.opt, 0);
    EXPECT_EQ(opt.arg, nullptr);
}

//
// prog -a -bbval -f -- x
//
TEST(getopt, separator_as_option_value)
{
    opt_reset();
    const char *argv[] = { "prog", "-a", "-bbval", "-f", "--", "x", 0 };
    int argc = 6;
    struct fpm_opt opt = {};

    int result = fpm_getopt(argc, (char**)argv, "ab:c:def:j", nullptr, &opt);
    EXPECT_EQ(result, 'a');
    EXPECT_EQ(opt.ret, result);
    EXPECT_EQ(opt.ind, 2);
    EXPECT_EQ(opt.opt, 'a');
    EXPECT_EQ(opt.arg, nullptr);

    result = fpm_getopt(argc, (char**)argv, "ab:c:def:j", nullptr, &opt);
    EXPECT_EQ(result, 'b');
    EXPECT_EQ(opt.ret, result);
    EXPECT_EQ(opt.ind, 3);
    EXPECT_EQ(opt.opt, 'b');
    EXPECT_STREQ(opt.arg, "bval");

    result = fpm_getopt(argc, (char**)argv, "ab:c:def:j", nullptr, &opt);
    EXPECT_EQ(result, 'f');
    EXPECT_EQ(opt.ret, result);
    EXPECT_EQ(opt.ind, 5);
    EXPECT_EQ(opt.opt, 'f');
    EXPECT_STREQ(opt.arg, "--");

    result = fpm_getopt(argc, (char**)argv, "ab:c:def:j", nullptr, &opt);
    EXPECT_EQ(result, 1);
    EXPECT_EQ(opt.ret, result);
    EXPECT_EQ(opt.ind, 6);
    EXPECT_EQ(opt.opt, 1);
    EXPECT_STREQ(opt.arg, "x");

    result = fpm_getopt(argc, (char**)argv, "ab:c:def:j", nullptr, &opt);
    EXPECT_EQ(result, -1);
    EXPECT_EQ(opt.ret, result);
    EXPECT_EQ(opt.ind, 6);
    EXPECT_EQ(opt.opt, 0);
    EXPECT_EQ(opt.arg, nullptr);
}

//
// prog -
//
TEST(getopt, orphan_dash)
{
    opt_reset();
    const char *argv[] = { "prog", "-", 0 };
    int argc = 2;
    struct fpm_opt opt = {};

    int result = fpm_getopt(argc, (char**)argv, "ab:c:def:j", nullptr, &opt);
    EXPECT_EQ(result, 1);
    EXPECT_EQ(opt.ret, result);
    EXPECT_EQ(opt.ind, 2);
    EXPECT_EQ(opt.opt, 1);
    EXPECT_STREQ(opt.arg, "-");

    result = fpm_getopt(argc, (char**)argv, "ab:c:def:j", nullptr, &opt);
    EXPECT_EQ(result, -1);
    EXPECT_EQ(opt.ret, result);
    EXPECT_EQ(opt.ind, 2);
    EXPECT_EQ(opt.opt, 0);
    EXPECT_EQ(opt.arg, nullptr);
}

//
// prog --long
//
TEST(getopt, long_option)
{
    opt_reset();
    const char *argv[] = { "prog", "--long", 0 };
    int argc = 2;
    struct fpm_option long_opts[] = {
        { "long", FPM_NO_ARG, 0, 42 },
        {},
    };
    struct fpm_opt opt = {};

    int result = fpm_getopt(argc, (char**)argv, "", long_opts, &opt);
    EXPECT_EQ(result, 42);
    EXPECT_EQ(opt.ret, result);
    EXPECT_EQ(opt.long_index, 0);
    EXPECT_EQ(opt.ind, 2);
    EXPECT_EQ(opt.opt, 0);
    EXPECT_EQ(opt.arg, nullptr);

    result = fpm_getopt(argc, (char**)argv, "", long_opts, &opt);
    EXPECT_EQ(result, -1);
    EXPECT_EQ(opt.ret, result);
    EXPECT_EQ(opt.ret, result);
    EXPECT_EQ(opt.long_index, 0);
    EXPECT_EQ(opt.ind, 2);
    EXPECT_EQ(opt.opt, 0);
    EXPECT_EQ(opt.arg, nullptr);
}

//
// prog --long1 --long2 --long1 --long1 --long4
//
TEST(getopt, many_long_options)
{
    opt_reset();
    const char *argv[] = { "prog", "--long1", "--long2", "--long1", "--long1", "--long4", 0 };
    int argc = 6;
    struct fpm_option long_opts[] = {
        { "long1", FPM_NO_ARG, 0, 10 },
        { "long2", FPM_NO_ARG, 0, 11 },
        { "long3", FPM_NO_ARG, 0, 12 },
        {},
    };
    struct fpm_opt opt = {};

    int result = fpm_getopt(argc, (char**)argv, "", long_opts, &opt);
    EXPECT_EQ(result, 10);
    EXPECT_EQ(opt.ret, result);
    EXPECT_EQ(opt.ret, result);
    EXPECT_EQ(opt.long_index, 0);
    EXPECT_EQ(opt.ind, 2);
    EXPECT_EQ(opt.opt, 0);
    EXPECT_EQ(opt.arg, nullptr);

    result = fpm_getopt(argc, (char**)argv, "", long_opts, &opt);
    EXPECT_EQ(result, 11);
    EXPECT_EQ(opt.ret, result);
    EXPECT_EQ(opt.long_index, 1);
    EXPECT_EQ(opt.ind, 3);
    EXPECT_EQ(opt.opt, 0);
    EXPECT_EQ(opt.arg, nullptr);

    result = fpm_getopt(argc, (char**)argv, "", long_opts, &opt);
    EXPECT_EQ(result, 10);
    EXPECT_EQ(opt.ret, result);
    EXPECT_EQ(opt.long_index, 0);
    EXPECT_EQ(opt.ind, 4);
    EXPECT_EQ(opt.opt, 0);
    EXPECT_EQ(opt.arg, nullptr);

    result = fpm_getopt(argc, (char**)argv, "", long_opts, &opt);
    EXPECT_EQ(result, 10);
    EXPECT_EQ(opt.ret, result);
    EXPECT_EQ(opt.long_index, 0);
    EXPECT_EQ(opt.ind, 5);
    EXPECT_EQ(opt.opt, 0);
    EXPECT_EQ(opt.arg, nullptr);

    result = fpm_getopt(argc, (char**)argv, "", long_opts, &opt);
    EXPECT_EQ(result, '?');
    EXPECT_EQ(opt.ret, result);
    EXPECT_EQ(opt.long_index, 0);
    EXPECT_EQ(opt.ind, 6);
    EXPECT_EQ(opt.opt, '?');
    EXPECT_STREQ(output, "prog: Unknown option `--long4`\r\n");

    result = fpm_getopt(argc, (char**)argv, "", long_opts, &opt);
    EXPECT_EQ(result, -1);
    EXPECT_EQ(opt.ret, result);
    EXPECT_EQ(opt.long_index, 0);
    EXPECT_EQ(opt.ind, 6);
    EXPECT_EQ(opt.opt, 0);
    EXPECT_EQ(opt.arg, nullptr);
}

//
// prog --long1 --long2 x y z
//
TEST(getopt, long_options_with_args)
{
    opt_reset();
    const char *argv[] = { "prog", "--long1", "--long2", "x", "y", "z", 0 };
    int argc = 6;
    struct fpm_option long_opts[] = {
        { "long1", FPM_NO_ARG, 0, 11 },
        { "long2", FPM_NO_ARG, 0, 22 },
        {},
    };
    struct fpm_opt opt = {};

    int result = fpm_getopt(argc, (char**)argv, "", long_opts, &opt);
    EXPECT_EQ(result, 11);
    EXPECT_EQ(opt.ret, result);
    EXPECT_EQ(opt.long_index, 0);
    EXPECT_EQ(opt.ind, 2);
    EXPECT_EQ(opt.opt, 0);
    EXPECT_EQ(opt.arg, nullptr);

    result = fpm_getopt(argc, (char**)argv, "", long_opts, &opt);
    EXPECT_EQ(result, 22);
    EXPECT_EQ(opt.ret, result);
    EXPECT_EQ(opt.long_index, 1);
    EXPECT_EQ(opt.ind, 3);
    EXPECT_EQ(opt.opt, 0);
    EXPECT_EQ(opt.arg, nullptr);

    result = fpm_getopt(argc, (char**)argv, "", long_opts, &opt);
    EXPECT_EQ(result, 1);
    EXPECT_EQ(opt.ret, result);
    EXPECT_EQ(opt.long_index, 1);
    EXPECT_EQ(opt.ind, 4);
    EXPECT_EQ(opt.opt, 1);
    EXPECT_STREQ(opt.arg, "x");

    result = fpm_getopt(argc, (char**)argv, "", long_opts, &opt);
    EXPECT_EQ(result, 1);
    EXPECT_EQ(opt.ret, result);
    EXPECT_EQ(opt.long_index, 1);
    EXPECT_EQ(opt.ind, 5);
    EXPECT_EQ(opt.opt, 1);
    EXPECT_STREQ(opt.arg, "y");

    result = fpm_getopt(argc, (char**)argv, "", long_opts, &opt);
    EXPECT_EQ(result, 1);
    EXPECT_EQ(opt.ret, result);
    EXPECT_EQ(opt.long_index, 1);
    EXPECT_EQ(opt.ind, 6);
    EXPECT_EQ(opt.opt, 1);
    EXPECT_STREQ(opt.arg, "z");

    result = fpm_getopt(argc, (char**)argv, "", long_opts, &opt);
    EXPECT_EQ(result, -1);
    EXPECT_EQ(opt.ret, result);
    EXPECT_EQ(opt.long_index, 1);
    EXPECT_EQ(opt.ind, 6);
    EXPECT_EQ(opt.opt, 0);
    EXPECT_EQ(opt.arg, nullptr);
}

//
// prog --long1
//
TEST(getopt, flag_ptr)
{
    opt_reset();
    const char *argv[] = { "prog", "--long1", 0 };
    int argc = 2;
    int flag = 0;
    struct fpm_option long_opts[] = {
        { "long1", FPM_NO_ARG, &flag, 11 },
        {},
    };
    struct fpm_opt opt = {};

    int result = fpm_getopt(argc, (char**)argv, "", long_opts, &opt);
    EXPECT_EQ(result, 0);
    EXPECT_EQ(opt.ret, result);
    EXPECT_EQ(opt.ind, 2);
    EXPECT_EQ(opt.opt, 0);
    EXPECT_EQ(opt.arg, nullptr);
    EXPECT_EQ(flag, 11);

    result = fpm_getopt(argc, (char**)argv, "", long_opts, &opt);
    EXPECT_EQ(result, -1);
    EXPECT_EQ(opt.ret, result);
    EXPECT_EQ(opt.ind, 2);
    EXPECT_EQ(opt.opt, 0);
    EXPECT_EQ(opt.arg, nullptr);
}

//
// prog --long2 arg --long3
//
TEST(getopt, mandatory_arg)
{
    opt_reset();
    const char *argv[] = { "prog", "--long2", "arg", "--long3", 0 };
    int argc = 4;
    struct fpm_option long_opts[] = {
        { "long2", FPM_REQUIRED_ARG, 0, 11 },
        {},
    };
    struct fpm_opt opt = {};

    int result = fpm_getopt(argc, (char**)argv, "", long_opts, &opt);
    EXPECT_EQ(result, 11);
    EXPECT_EQ(opt.ret, result);
    EXPECT_EQ(opt.ind, 3);
    EXPECT_EQ(opt.opt, 0);
    EXPECT_STREQ(opt.arg, "arg");

    result = fpm_getopt(argc, (char**)argv, "", long_opts, &opt);
    EXPECT_EQ(result, '?');
    EXPECT_EQ(opt.ret, result);
    EXPECT_EQ(opt.ind, 4);
    EXPECT_EQ(opt.opt, '?');
    EXPECT_STREQ(output, "prog: Unknown option `--long3`\r\n");

    result = fpm_getopt(argc, (char**)argv, "", long_opts, &opt);
    EXPECT_EQ(result, -1);
    EXPECT_EQ(opt.ret, result);
    EXPECT_EQ(opt.ind, 4);
    EXPECT_EQ(opt.opt, 0);
    EXPECT_EQ(opt.arg, nullptr);
}

//
// prog --long2 arg --long3
//
TEST(getopt, long_option_no_arg)
{
    opt_reset();
    const char *argv[] = { "prog", "--long2", "arg", "--long3", 0 };
    int argc = 4;
    struct fpm_option long_opts[] = {
        { "long2", FPM_NO_ARG, 0, 11 },
        {},
    };
    struct fpm_opt opt = {};

    int result = fpm_getopt(argc, (char**)argv, "", long_opts, &opt);
    EXPECT_EQ(result, 11);
    EXPECT_EQ(opt.ret, result);
    EXPECT_EQ(opt.ind, 2);
    EXPECT_EQ(opt.opt, 0);
    EXPECT_EQ(opt.arg, nullptr);

    result = fpm_getopt(argc, (char**)argv, "", long_opts, &opt);
    EXPECT_EQ(result, 1);
    EXPECT_EQ(opt.ret, result);
    EXPECT_EQ(opt.ind, 3);
    EXPECT_EQ(opt.opt, 1);
    EXPECT_STREQ(opt.arg, "arg");

    result = fpm_getopt(argc, (char**)argv, "", long_opts, &opt);
    EXPECT_EQ(result, '?');
    EXPECT_EQ(opt.ret, result);
    EXPECT_EQ(opt.ind, 4);
    EXPECT_EQ(opt.opt, '?');
    EXPECT_STREQ(output, "prog: Unknown option `--long3`\r\n");

    result = fpm_getopt(argc, (char**)argv, "", long_opts, &opt);
    EXPECT_EQ(result, -1);
    EXPECT_EQ(opt.ret, result);
    EXPECT_EQ(opt.ind, 4);
    EXPECT_EQ(opt.opt, 0);
    EXPECT_EQ(opt.arg, nullptr);
}

//
// prog --long1=arg1 --long2=arg2
//
TEST(getopt, long_option_equals_arg)
{
    opt_reset();
    const char *argv[] = { "prog", "--long1=arg1", "--long2=arg2", 0 };
    int argc = 3;
    struct fpm_option long_opts[] = {
        { "long1", FPM_REQUIRED_ARG, 0, 11 },
        { "long2", FPM_NO_ARG, 0, 22 },
        {},
    };
    struct fpm_opt opt = {};

    int result = fpm_getopt(argc, (char**)argv, "", long_opts, &opt);
    EXPECT_EQ(result, 11);
    EXPECT_EQ(opt.ret, result);
    EXPECT_EQ(opt.ind, 2);
    EXPECT_EQ(opt.opt, 0);
    EXPECT_STREQ(opt.arg, "arg1");

    result = fpm_getopt(argc, (char**)argv, "", long_opts, &opt);
    EXPECT_EQ(result, 22);
    EXPECT_EQ(opt.ret, result);
    EXPECT_EQ(opt.ind, 3);
    EXPECT_EQ(opt.opt, 0);
    EXPECT_EQ(opt.arg, nullptr); // arg2 is ignored

    result = fpm_getopt(argc, (char**)argv, "", long_opts, &opt);
    EXPECT_EQ(result, -1);
    EXPECT_EQ(opt.ret, result);
    EXPECT_EQ(opt.ind, 3);
    EXPECT_EQ(opt.opt, 0);
    EXPECT_EQ(opt.arg, nullptr);
}

//
// prog --long1 --long2=arg
//
TEST(getopt, optional_args)
{
    opt_reset();
    const char *argv[] = { "prog", "--long1", "--long2=arg", 0 };
    int argc = 3;
    struct fpm_option long_opts[] = {
        { "long1", FPM_OPTIONAL_ARG, 0, 11 },
        { "long2", FPM_OPTIONAL_ARG, 0, 22 },
        {},
    };
    struct fpm_opt opt = {};
    int result = fpm_getopt(argc, (char**)argv, "", long_opts, &opt);
    EXPECT_EQ(result, 11);
    EXPECT_EQ(opt.ret, result);
    EXPECT_EQ(opt.ind, 2);
    EXPECT_EQ(opt.opt, 0);
    EXPECT_EQ(opt.arg, nullptr);

    result = fpm_getopt(argc, (char**)argv, "", long_opts, &opt);
    EXPECT_EQ(result, 22);
    EXPECT_EQ(opt.ret, result);
    EXPECT_EQ(opt.ind, 3);
    EXPECT_EQ(opt.opt, 0);
    EXPECT_STREQ(opt.arg, "arg");

    result = fpm_getopt(argc, (char**)argv, "", long_opts, &opt);
    EXPECT_EQ(result, -1);
    EXPECT_EQ(opt.ret, result);
    EXPECT_EQ(opt.ind, 3);
    EXPECT_EQ(opt.opt, 0);
    EXPECT_EQ(opt.arg, nullptr);
}

//
// prog -a -# 6 -H1 -H 2 -ka -l -lv -x path=./foo bar
//
TEST(getopt, short_test1)
{
    opt_reset();
    const char *argv[] = { "prog", "-a", "-#", "6", "-H1", "-H", "2", "-ka", "-l", "-lv", "-x", "path=./foo", "bar", 0 };
    int argc = 13;
    const char *short_opts = "#:abc::CdDeE::fFgGhH:iIk:l::mMnNo::O:pPqQrRsS:t:u:UvVwW::x:yz";
    struct fpm_opt opt = {};

    int result = fpm_getopt(argc, (char**)argv, short_opts, nullptr, &opt);
    EXPECT_EQ(result, 'a');
    EXPECT_EQ(opt.ret, result);
    EXPECT_EQ(opt.ind, 2);
    EXPECT_EQ(opt.opt, 'a');
    EXPECT_EQ(opt.arg, nullptr);

    result = fpm_getopt(argc, (char**)argv, short_opts, nullptr, &opt);
    EXPECT_EQ(result, '#');
    EXPECT_EQ(opt.ret, result);
    EXPECT_EQ(opt.ind, 4);
    EXPECT_EQ(opt.opt, '#');
    EXPECT_STREQ(opt.arg, "6");

    result = fpm_getopt(argc, (char**)argv, short_opts, nullptr, &opt);
    EXPECT_EQ(result, 'H');
    EXPECT_EQ(opt.ret, result);
    EXPECT_EQ(opt.ind, 5);
    EXPECT_EQ(opt.opt, 'H');
    EXPECT_STREQ(opt.arg, "1");

    result = fpm_getopt(argc, (char**)argv, short_opts, nullptr, &opt);
    EXPECT_EQ(result, 'H');
    EXPECT_EQ(opt.ret, result);
    EXPECT_EQ(opt.ind, 7);
    EXPECT_EQ(opt.opt, 'H');
    EXPECT_STREQ(opt.arg, "2");

    result = fpm_getopt(argc, (char**)argv, short_opts, nullptr, &opt);
    EXPECT_EQ(result, 'k');
    EXPECT_EQ(opt.ret, result);
    EXPECT_EQ(opt.ind, 8);
    EXPECT_EQ(opt.opt, 'k');
    EXPECT_STREQ(opt.arg, "a");

    result = fpm_getopt(argc, (char**)argv, short_opts, nullptr, &opt);
    EXPECT_EQ(result, 'l');
    EXPECT_EQ(opt.ret, result);
    EXPECT_EQ(opt.ind, 9);
    EXPECT_EQ(opt.opt, 'l');
    EXPECT_EQ(opt.arg, nullptr);

    result = fpm_getopt(argc, (char**)argv, short_opts, nullptr, &opt);
    EXPECT_EQ(result, 'l');
    EXPECT_EQ(opt.ret, result);
    EXPECT_EQ(opt.ind, 10);
    EXPECT_EQ(opt.opt, 'l');
    EXPECT_STREQ(opt.arg, "v");

    result = fpm_getopt(argc, (char**)argv, short_opts, nullptr, &opt);
    EXPECT_EQ(result, 'x');
    EXPECT_EQ(opt.ret, result);
    EXPECT_EQ(opt.ind, 12);
    EXPECT_EQ(opt.opt, 'x');
    EXPECT_STREQ(opt.arg, "path=./foo");

    result = fpm_getopt(argc, (char**)argv, short_opts, nullptr, &opt);
    EXPECT_EQ(result, 1);
    EXPECT_EQ(opt.ret, result);
    EXPECT_EQ(opt.ind, 13);
    EXPECT_EQ(opt.opt, 1);
    EXPECT_STREQ(opt.arg, "bar");

    result = fpm_getopt(argc, (char**)argv, short_opts, nullptr, &opt);
    EXPECT_EQ(result, -1);
    EXPECT_EQ(opt.ret, result);
    EXPECT_EQ(opt.ind, 13);
    EXPECT_EQ(opt.opt, 0);
    EXPECT_EQ(opt.arg, nullptr);
}

//
// prog -a -#pound -b -cfilename
//
TEST(getopt, short_test2)
{
    opt_reset();
    const char *argv[] = { "prog", "-a", "-#pound", "-b", "-cfilename", 0 };
    int argc = 5;
    const char *short_opts = "#:abc::";
    struct fpm_opt opt = {};

    int result = fpm_getopt(argc, (char**)argv, short_opts, nullptr, &opt);
    EXPECT_EQ(result, 'a');
    EXPECT_EQ(opt.ret, result);
    EXPECT_EQ(opt.ind, 2);
    EXPECT_EQ(opt.opt, 'a');
    EXPECT_EQ(opt.arg, nullptr);

    result = fpm_getopt(argc, (char**)argv, short_opts, nullptr, &opt);
    EXPECT_EQ(result, '#');
    EXPECT_EQ(opt.ret, result);
    EXPECT_EQ(opt.ind, 3);
    EXPECT_EQ(opt.opt, '#');
    EXPECT_STREQ(opt.arg, "pound");

    result = fpm_getopt(argc, (char**)argv, short_opts, nullptr, &opt);
    EXPECT_EQ(result, 'b');
    EXPECT_EQ(opt.ret, result);
    EXPECT_EQ(opt.ind, 4);
    EXPECT_EQ(opt.opt, 'b');
    EXPECT_EQ(opt.arg, nullptr);

    result = fpm_getopt(argc, (char**)argv, short_opts, nullptr, &opt);
    EXPECT_EQ(result, 'c');
    EXPECT_EQ(opt.ret, result);
    EXPECT_EQ(opt.ind, 5);
    EXPECT_EQ(opt.opt, 'c');
    EXPECT_STREQ(opt.arg, "filename");

    result = fpm_getopt(argc, (char**)argv, short_opts, nullptr, &opt);
    EXPECT_EQ(result, -1);
    EXPECT_EQ(opt.ret, result);
    EXPECT_EQ(opt.ind, 5);
    EXPECT_EQ(opt.opt, 0);
    EXPECT_EQ(opt.arg, nullptr);
}

//
// prog -a -#bx -b -c -cfoo -d bar
//
TEST(getopt, short_test3)
{
    opt_reset();
    const char *argv[] = { "prog", "-a", "-#bx", "-b", "-c", "-cfoo", "-d", "bar", 0 };
    int argc = 8;
    const char *short_opts = "#:abc::d";
    struct fpm_opt opt = {};

    int result = fpm_getopt(argc, (char**)argv, short_opts, nullptr, &opt);
    EXPECT_EQ(result, 'a');
    EXPECT_EQ(opt.ret, result);
    EXPECT_EQ(opt.ind, 2);
    EXPECT_EQ(opt.opt, 'a');
    EXPECT_EQ(opt.arg, nullptr);

    result = fpm_getopt(argc, (char**)argv, short_opts, nullptr, &opt);
    EXPECT_EQ(result, '#');
    EXPECT_EQ(opt.ret, result);
    EXPECT_EQ(opt.ind, 3);
    EXPECT_EQ(opt.opt, '#');
    EXPECT_STREQ(opt.arg, "bx");

    result = fpm_getopt(argc, (char**)argv, short_opts, nullptr, &opt);
    EXPECT_EQ(result, 'b');
    EXPECT_EQ(opt.ret, result);
    EXPECT_EQ(opt.ind, 4);
    EXPECT_EQ(opt.opt, 'b');
    EXPECT_EQ(opt.arg, nullptr);

    result = fpm_getopt(argc, (char**)argv, short_opts, nullptr, &opt);
    EXPECT_EQ(result, 'c');
    EXPECT_EQ(opt.ret, result);
    EXPECT_EQ(opt.ind, 5);
    EXPECT_EQ(opt.opt, 'c');
    EXPECT_EQ(opt.arg, nullptr);

    result = fpm_getopt(argc, (char**)argv, short_opts, nullptr, &opt);
    EXPECT_EQ(result, 'c');
    EXPECT_EQ(opt.ret, result);
    EXPECT_EQ(opt.ind, 6);
    EXPECT_EQ(opt.opt, 'c');
    EXPECT_STREQ(opt.arg, "foo");

    result = fpm_getopt(argc, (char**)argv, short_opts, nullptr, &opt);
    EXPECT_EQ(result, 'd');
    EXPECT_EQ(opt.ret, result);
    EXPECT_EQ(opt.ind, 7);
    EXPECT_EQ(opt.opt, 'd');
    EXPECT_EQ(opt.arg, nullptr);

    result = fpm_getopt(argc, (char**)argv, short_opts, nullptr, &opt);
    EXPECT_EQ(result, 1);
    EXPECT_EQ(opt.ret, result);
    EXPECT_EQ(opt.ind, 8);
    EXPECT_EQ(opt.opt, 1);
    EXPECT_STREQ(opt.arg, "bar");

    result = fpm_getopt(argc, (char**)argv, short_opts, nullptr, &opt);
    EXPECT_EQ(result, -1);
    EXPECT_EQ(opt.ret, result);
    EXPECT_EQ(opt.ind, 8);
    EXPECT_EQ(opt.opt, 0);
    EXPECT_EQ(opt.arg, nullptr);
}

//
// prog -ab -a -cx -c y -d -da
//
TEST(getopt, short_test5)
{
    opt_reset();
    const char *argv[] = { "prog", "-ab", "-a", "-cx", "-c", "y", "-d", "-da", 0 };
    int argc = 8;
    const char *short_opts = "abc:d::";
    struct fpm_opt opt = {};

    int result = fpm_getopt(argc, (char**)argv, short_opts, nullptr, &opt);
    EXPECT_EQ(result, 'a');
    EXPECT_EQ(opt.ret, result);
    EXPECT_EQ(opt.ind, 1);
    EXPECT_EQ(opt.opt, 'a');
    EXPECT_EQ(opt.arg, nullptr);

    result = fpm_getopt(argc, (char**)argv, short_opts, nullptr, &opt);
    EXPECT_EQ(result, 'b');
    EXPECT_EQ(opt.ret, result);
    EXPECT_EQ(opt.ind, 2);
    EXPECT_EQ(opt.opt, 'b');
    EXPECT_EQ(opt.arg, nullptr);

    result = fpm_getopt(argc, (char**)argv, short_opts, nullptr, &opt);
    EXPECT_EQ(result, 'a');
    EXPECT_EQ(opt.ret, result);
    EXPECT_EQ(opt.ind, 3);
    EXPECT_EQ(opt.opt, 'a');
    EXPECT_EQ(opt.arg, nullptr);

    result = fpm_getopt(argc, (char**)argv, short_opts, nullptr, &opt);
    EXPECT_EQ(result, 'c');
    EXPECT_EQ(opt.ret, result);
    EXPECT_EQ(opt.ind, 4);
    EXPECT_EQ(opt.opt, 'c');
    EXPECT_STREQ(opt.arg, "x");

    result = fpm_getopt(argc, (char**)argv, short_opts, nullptr, &opt);
    EXPECT_EQ(result, 'c');
    EXPECT_EQ(opt.ret, result);
    EXPECT_EQ(opt.ind, 6);
    EXPECT_EQ(opt.opt, 'c');
    EXPECT_STREQ(opt.arg, "y");

    result = fpm_getopt(argc, (char**)argv, short_opts, nullptr, &opt);
    EXPECT_EQ(result, 'd');
    EXPECT_EQ(opt.ret, result);
    EXPECT_EQ(opt.ind, 7);
    EXPECT_EQ(opt.opt, 'd');
    EXPECT_EQ(opt.arg, nullptr);

    result = fpm_getopt(argc, (char**)argv, short_opts, nullptr, &opt);
    EXPECT_EQ(result, 'd');
    EXPECT_EQ(opt.ret, result);
    EXPECT_EQ(opt.ind, 8);
    EXPECT_EQ(opt.opt, 'd');
    EXPECT_STREQ(opt.arg, "a");

    result = fpm_getopt(argc, (char**)argv, short_opts, nullptr, &opt);
    EXPECT_EQ(result, -1);
    EXPECT_EQ(opt.ret, result);
    EXPECT_EQ(opt.ind, 8);
    EXPECT_EQ(opt.opt, 0);
    EXPECT_EQ(opt.arg, nullptr);
}

//
// prog -H
//
TEST(getopt, short_test6)
{
    opt_reset();
    const char *argv[] = { "prog", "-H", 0 };
    int argc = 2;
    const char *short_opts = "abH:d::";
    struct fpm_opt opt = {};

    int result = fpm_getopt(argc, (char**)argv, short_opts, nullptr, &opt);
    EXPECT_EQ(result, '?');
    EXPECT_EQ(opt.ret, result);
    EXPECT_EQ(opt.ind, 2);
    EXPECT_EQ(opt.opt, 'H');
    EXPECT_STREQ(output, "prog: Argument required for option `-H`\r\n");

    result = fpm_getopt(argc, (char**)argv, short_opts, nullptr, &opt);
    EXPECT_EQ(result, -1);
    EXPECT_EQ(opt.ret, result);
    EXPECT_EQ(opt.ind, 2);
    EXPECT_EQ(opt.opt, 0);
    EXPECT_EQ(opt.arg, nullptr);
}

//
// prog -H
//
TEST(getopt, short_test7)
{
    opt_reset();
    const char *argv[] = { "prog", "-H", 0 };
    int argc = 2;
    const char *short_opts = ":abH:d::"; // Leading ':' in opt string
    struct fpm_opt opt = {};

    int result = fpm_getopt(argc, (char**)argv, short_opts, nullptr, &opt);
    EXPECT_EQ(result, ':');
    EXPECT_EQ(opt.ret, result);
    EXPECT_EQ(opt.ind, 2);
    EXPECT_EQ(opt.opt, 'H');
    EXPECT_STREQ(output, "prog: Argument required for option `-H`\r\n");

    result = fpm_getopt(argc, (char**)argv, short_opts, nullptr, &opt);
    EXPECT_EQ(result, -1);
    EXPECT_EQ(opt.ret, result);
    EXPECT_EQ(opt.ind, 2);
    EXPECT_EQ(opt.opt, 0);
    EXPECT_EQ(opt.arg, nullptr);
}

//
// prog -x
//
TEST(getopt, short_test8)
{
    opt_reset();
    const char *argv[] = { "prog", "-x", 0 };
    int argc = 2;
    const char *short_opts = "abH:d::";
    struct fpm_opt opt = {};

    int result = fpm_getopt(argc, (char**)argv, short_opts, nullptr, &opt);
    EXPECT_EQ(result, '?');
    EXPECT_EQ(opt.ret, result);
    EXPECT_EQ(opt.ind, 2);
    EXPECT_EQ(opt.opt, '?');
    EXPECT_STREQ(output, "prog: Unknown option `-x`\r\n");

    result = fpm_getopt(argc, (char**)argv, short_opts, nullptr, &opt);
    EXPECT_EQ(result, -1);
    EXPECT_EQ(opt.ret, result);
    EXPECT_EQ(opt.ind, 2);
    EXPECT_EQ(opt.opt, 0);
    EXPECT_EQ(opt.arg, nullptr);
}
