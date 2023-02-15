#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <cmocka.h>
#include <rpm/api.h>

//
// Buffer for stdout messages from rpm_getopt().
//
static char output[256];
static char *out_ptr = output;

void rpm_putchar(char ch)
{
    *out_ptr++ = ch;
    *out_ptr = '\0';
}

void rpm_puts(const char *input)
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
static void option_h_valid(void **unused)
{
    opt_reset();
    char *argv[] = { "prog", "-h", 0 };
    struct rpm_opt opt = {};

    int result = rpm_getopt(2, argv, "h", NULL, &opt);
    assert_int_equal(result, 'h');
    assert_int_equal(opt.ind, 2);
    assert_int_equal(opt.opt, 'h');
    assert_ptr_equal(opt.arg, NULL);
    assert_string_equal(output, "");
}

//
// prog -h --- missing option
//
static void option_h_missing(void **unused)
{
    opt_reset();
    char *argv[] = { "prog", "-h", 0 };
    struct rpm_opt opt = {};

    int result = rpm_getopt(2, argv, "a", NULL, &opt);
    assert_int_equal(result, '?');
    assert_int_equal(opt.opt, '?');
    assert_string_equal(output, "prog: unknown option `-h`\r\n");
}

//
// prog -i 1
//
static void option_i_1(void **unused)
{
    opt_reset();
    char *argv[] = { "prog", "-i", "1", 0 };
    struct rpm_opt opt = {};

    int result = rpm_getopt(3, argv, "i:", NULL, &opt);
    assert_int_equal(result, 'i');
    assert_int_equal(opt.opt, 'i');
    assert_string_equal(opt.arg, "1");
    assert_string_equal(output, "");
}

//
// prog
//
static void no_args(void **unused)
{
    opt_reset();
    char *argv[] = { "prog", 0 };
    struct rpm_opt opt = {};

    int result = rpm_getopt(1, argv, "ab:c:def:j", NULL, &opt);
    assert_int_equal(result, -1);
    assert_int_equal(opt.ind, 1);
    assert_int_equal(opt.opt, 0);
    assert_ptr_equal(opt.arg, NULL);
}

//
// prog x y z
//
static void three_args(void **unused)
{
    opt_reset();
    char *argv[] = { "prog", "x", "y", "z", 0 };
    int argc = 4;
    struct rpm_opt opt = {};

    int result = rpm_getopt(argc, argv, "ab:c:def:j", NULL, &opt);
    assert_int_equal(result, 1);
    assert_int_equal(opt.ind, 2);
    assert_int_equal(opt.opt, 1);
    assert_string_equal(opt.arg, "x");

    result = rpm_getopt(argc, argv, "ab:c:def:j", NULL, &opt);
    assert_int_equal(result, 1);
    assert_int_equal(opt.ind, 3);
    assert_int_equal(opt.opt, 1);
    assert_string_equal(opt.arg, "y");

    result = rpm_getopt(argc, argv, "ab:c:def:j", NULL, &opt);
    assert_int_equal(result, 1);
    assert_int_equal(opt.ind, 4);
    assert_int_equal(opt.opt, 1);
    assert_string_equal(opt.arg, "z");

    result = rpm_getopt(argc, argv, "ab:c:def:j", NULL, &opt);
    assert_int_equal(result, -1);
    assert_int_equal(opt.ind, 4);
    assert_int_equal(opt.opt, 0);
    assert_ptr_equal(opt.arg, NULL);
}

//
// prog -a
//
static void simple_option(void **unused)
{
    opt_reset();
    char *argv[] = { "prog", "-a", 0 };
    int argc = 2;
    struct rpm_opt opt = {};

    int result = rpm_getopt(argc, argv, "ab:c:def:j", NULL, &opt);
    assert_int_equal(result, 'a');
    assert_int_equal(opt.ind, 2);
    assert_int_equal(opt.opt, 'a');
    assert_ptr_equal(opt.arg, NULL);

    result = rpm_getopt(argc, argv, "ab:c:def:j", NULL, &opt);
    assert_int_equal(result, -1);
    assert_int_equal(opt.ind, 2);
    assert_int_equal(opt.opt, 0);
    assert_ptr_equal(opt.arg, NULL);
}

//
// prog -a x y z
//
static void simple_option_with_args(void **unused)
{
    opt_reset();
    char *argv[] = { "prog", "-a", "x", "y", "z", 0 };
    int argc = 5;
    struct rpm_opt opt = {};

    int result = rpm_getopt(argc, argv, "ab:c:def:j", NULL, &opt);
    assert_int_equal(result, 'a');
    assert_int_equal(opt.ind, 2);
    assert_int_equal(opt.opt, 'a');
    assert_ptr_equal(opt.arg, NULL);

    result = rpm_getopt(argc, argv, "ab:c:def:j", NULL, &opt);
    assert_int_equal(result, 1);
    assert_int_equal(opt.ind, 3);
    assert_int_equal(opt.opt, 1);
    assert_string_equal(opt.arg, "x");

    result = rpm_getopt(argc, argv, "ab:c:def:j", NULL, &opt);
    assert_int_equal(result, 1);
    assert_int_equal(opt.ind, 4);
    assert_int_equal(opt.opt, 1);
    assert_string_equal(opt.arg, "y");

    result = rpm_getopt(argc, argv, "ab:c:def:j", NULL, &opt);
    assert_int_equal(result, 1);
    assert_int_equal(opt.ind, 5);
    assert_int_equal(opt.opt, 1);
    assert_string_equal(opt.arg, "z");

    result = rpm_getopt(argc, argv, "ab:c:def:j", NULL, &opt);
    assert_int_equal(result, -1);
    assert_int_equal(opt.ind, 5);
    assert_int_equal(opt.opt, 0);
    assert_ptr_equal(opt.arg, NULL);
}

//
// prog -ade -j x y z
//
static void many_simple_options(void **unused)
{
    opt_reset();
    char *argv[] = { "prog", "-ade", "-j", "x", "y", "z", 0 };
    int argc = 6;
    struct rpm_opt opt = {};

    int result = rpm_getopt(argc, argv, "ab:c:def:j", NULL, &opt);
    assert_int_equal(result, 'a');
    assert_int_equal(opt.ind, 1);
    assert_int_equal(opt.opt, 'a');
    assert_ptr_equal(opt.arg, NULL);

    result = rpm_getopt(argc, argv, "ab:c:def:j", NULL, &opt);
    assert_int_equal(result, 'd');
    assert_int_equal(opt.ind, 1);
    assert_int_equal(opt.opt, 'd');
    assert_ptr_equal(opt.arg, NULL);

    result = rpm_getopt(argc, argv, "ab:c:def:j", NULL, &opt);
    assert_int_equal(result, 'e');
    assert_int_equal(opt.ind, 2);
    assert_int_equal(opt.opt, 'e');
    assert_ptr_equal(opt.arg, NULL);

    result = rpm_getopt(argc, argv, "ab:c:def:j", NULL, &opt);
    assert_int_equal(result, 'j');
    assert_int_equal(opt.ind, 3);
    assert_int_equal(opt.opt, 'j');
    assert_ptr_equal(opt.arg, NULL);

    result = rpm_getopt(argc, argv, "ab:c:def:j", NULL, &opt);
    assert_int_equal(result, 1);
    assert_int_equal(opt.ind, 4);
    assert_int_equal(opt.opt, 1);
    assert_string_equal(opt.arg, "x");

    result = rpm_getopt(argc, argv, "ab:c:def:j", NULL, &opt);
    assert_int_equal(result, 1);
    assert_int_equal(opt.ind, 5);
    assert_int_equal(opt.opt, 1);
    assert_string_equal(opt.arg, "y");

    result = rpm_getopt(argc, argv, "ab:c:def:j", NULL, &opt);
    assert_int_equal(result, 1);
    assert_int_equal(opt.ind, 6);
    assert_int_equal(opt.opt, 1);
    assert_string_equal(opt.arg, "z");

    result = rpm_getopt(argc, argv, "ab:c:def:j", NULL, &opt);
    assert_int_equal(result, -1);
    assert_int_equal(opt.ind, 6);
    assert_int_equal(opt.opt, 0);
    assert_ptr_equal(opt.arg, NULL);
}

//
// prog -abbval -c cval -def fval -j
//
static void option_mix_without_args(void **unused)
{
    opt_reset();
    char *argv[] = { "prog", "-abbval", "-c", "cval", "-def", "fval", "-j", 0 };
    int argc = 7;
    struct rpm_opt opt = {};

    int result = rpm_getopt(argc, argv, "ab:c:def:j", NULL, &opt);
    assert_int_equal(result, 'a');
    assert_int_equal(opt.ind, 1);
    assert_int_equal(opt.opt, 'a');
    assert_ptr_equal(opt.arg, NULL);

    result = rpm_getopt(argc, argv, "ab:c:def:j", NULL, &opt);
    assert_int_equal(result, 'b');
    assert_int_equal(opt.ind, 2);
    assert_int_equal(opt.opt, 'b');
    assert_string_equal(opt.arg, "bval");

    result = rpm_getopt(argc, argv, "ab:c:def:j", NULL, &opt);
    assert_int_equal(result, 'c');
    assert_int_equal(opt.ind, 4);
    assert_int_equal(opt.opt, 'c');
    assert_string_equal(opt.arg, "cval");

    result = rpm_getopt(argc, argv, "ab:c:def:j", NULL, &opt);
    assert_int_equal(result, 'd');
    assert_int_equal(opt.ind, 4);
    assert_int_equal(opt.opt, 'd');
    assert_ptr_equal(opt.arg, NULL);

    result = rpm_getopt(argc, argv, "ab:c:def:j", NULL, &opt);
    assert_int_equal(result, 'e');
    assert_int_equal(opt.ind, 4);
    assert_int_equal(opt.opt, 'e');
    assert_ptr_equal(opt.arg, NULL);

    result = rpm_getopt(argc, argv, "ab:c:def:j", NULL, &opt);
    assert_int_equal(result, 'f');
    assert_int_equal(opt.ind, 6);
    assert_int_equal(opt.opt, 'f');
    assert_string_equal(opt.arg, "fval");

    result = rpm_getopt(argc, argv, "ab:c:def:j", NULL, &opt);
    assert_int_equal(result, 'j');
    assert_int_equal(opt.ind, 7);
    assert_int_equal(opt.opt, 'j');
    assert_ptr_equal(opt.arg, NULL);

    result = rpm_getopt(argc, argv, "ab:c:def:j", NULL, &opt);
    assert_int_equal(result, -1);
    assert_int_equal(opt.ind, 7);
    assert_int_equal(opt.opt, 0);
    assert_ptr_equal(opt.arg, NULL);
}

//
// prog -abbval -c cval -def fval -j x y
//
static void option_mix_with_args(void **unused)
{
    opt_reset();
    char *argv[] = { "prog", "-abbval", "-c", "cval", "-def", "fval", "-j", "x", "y", 0 };
    int argc = 9;
    struct rpm_opt opt = {};

    int result = rpm_getopt(argc, argv, "ab:c:def:j", NULL, &opt);
    assert_int_equal(result, 'a');
    assert_int_equal(opt.ind, 1);
    assert_int_equal(opt.opt, 'a');
    assert_ptr_equal(opt.arg, NULL);

    result = rpm_getopt(argc, argv, "ab:c:def:j", NULL, &opt);
    assert_int_equal(result, 'b');
    assert_int_equal(opt.ind, 2);
    assert_int_equal(opt.opt, 'b');
    assert_string_equal(opt.arg, "bval");

    result = rpm_getopt(argc, argv, "ab:c:def:j", NULL, &opt);
    assert_int_equal(result, 'c');
    assert_int_equal(opt.ind, 4);
    assert_int_equal(opt.opt, 'c');
    assert_string_equal(opt.arg, "cval");

    result = rpm_getopt(argc, argv, "ab:c:def:j", NULL, &opt);
    assert_int_equal(result, 'd');
    assert_int_equal(opt.ind, 4);
    assert_int_equal(opt.opt, 'd');
    assert_ptr_equal(opt.arg, NULL);

    result = rpm_getopt(argc, argv, "ab:c:def:j", NULL, &opt);
    assert_int_equal(result, 'e');
    assert_int_equal(opt.ind, 4);
    assert_int_equal(opt.opt, 'e');
    assert_ptr_equal(opt.arg, NULL);

    result = rpm_getopt(argc, argv, "ab:c:def:j", NULL, &opt);
    assert_int_equal(result, 'f');
    assert_int_equal(opt.ind, 6);
    assert_int_equal(opt.opt, 'f');
    assert_string_equal(opt.arg, "fval");

    result = rpm_getopt(argc, argv, "ab:c:def:j", NULL, &opt);
    assert_int_equal(result, 'j');
    assert_int_equal(opt.ind, 7);
    assert_int_equal(opt.opt, 'j');
    assert_ptr_equal(opt.arg, NULL);

    result = rpm_getopt(argc, argv, "ab:c:def:j", NULL, &opt);
    assert_int_equal(result, 1);
    assert_int_equal(opt.ind, 8);
    assert_int_equal(opt.opt, 1);
    assert_string_equal(opt.arg, "x");

    result = rpm_getopt(argc, argv, "ab:c:def:j", NULL, &opt);
    assert_int_equal(result, 1);
    assert_int_equal(opt.ind, 9);
    assert_int_equal(opt.opt, 1);
    assert_string_equal(opt.arg, "y");

    result = rpm_getopt(argc, argv, "ab:c:def:j", NULL, &opt);
    assert_int_equal(result, -1);
    assert_int_equal(opt.ind, 9);
    assert_int_equal(opt.opt, 0);
    assert_ptr_equal(opt.arg, NULL);
}

//
// prog -abbval x -c cval -def fval y -j z
//
static void option_mix_shuffled_args(void **unused)
{
    opt_reset();
    char *argv[] = { "prog", "-abbval", "x", "-c", "cval", "-def", "fval", "y", "-j", "z", 0 };
    int argc = 10;
    struct rpm_opt opt = {};

    int result = rpm_getopt(argc, argv, "ab:c:def:j", NULL, &opt);
    assert_int_equal(result, 'a');
    assert_int_equal(opt.ind, 1);
    assert_int_equal(opt.opt, 'a');
    assert_ptr_equal(opt.arg, NULL);

    result = rpm_getopt(argc, argv, "ab:c:def:j", NULL, &opt);
    assert_int_equal(result, 'b');
    assert_int_equal(opt.ind, 2);
    assert_int_equal(opt.opt, 'b');
    assert_string_equal(opt.arg, "bval");

    result = rpm_getopt(argc, argv, "ab:c:def:j", NULL, &opt);
    assert_int_equal(result, 1);
    assert_int_equal(opt.ind, 3);
    assert_int_equal(opt.opt, 1);
    assert_string_equal(opt.arg, "x");

    result = rpm_getopt(argc, argv, "ab:c:def:j", NULL, &opt);
    assert_int_equal(result, 'c');
    assert_int_equal(opt.ind, 5);
    assert_int_equal(opt.opt, 'c');
    assert_string_equal(opt.arg, "cval");

    result = rpm_getopt(argc, argv, "ab:c:def:j", NULL, &opt);
    assert_int_equal(result, 'd');
    assert_int_equal(opt.ind, 5);
    assert_int_equal(opt.opt, 'd');
    assert_ptr_equal(opt.arg, NULL);

    result = rpm_getopt(argc, argv, "ab:c:def:j", NULL, &opt);
    assert_int_equal(result, 'e');
    assert_int_equal(opt.ind, 5);
    assert_int_equal(opt.opt, 'e');
    assert_ptr_equal(opt.arg, NULL);

    result = rpm_getopt(argc, argv, "ab:c:def:j", NULL, &opt);
    assert_int_equal(result, 'f');
    assert_int_equal(opt.ind, 7);
    assert_int_equal(opt.opt, 'f');
    assert_string_equal(opt.arg, "fval");

    result = rpm_getopt(argc, argv, "ab:c:def:j", NULL, &opt);
    assert_int_equal(result, 1);
    assert_int_equal(opt.ind, 8);
    assert_int_equal(opt.opt, 1);
    assert_string_equal(opt.arg, "y");

    result = rpm_getopt(argc, argv, "ab:c:def:j", NULL, &opt);
    assert_int_equal(result, 'j');
    assert_int_equal(opt.ind, 9);
    assert_int_equal(opt.opt, 'j');
    assert_ptr_equal(opt.arg, NULL);

    result = rpm_getopt(argc, argv, "ab:c:def:j", NULL, &opt);
    assert_int_equal(result, 1);
    assert_int_equal(opt.ind, 10);
    assert_int_equal(opt.opt, 1);
    assert_string_equal(opt.arg, "z");

    result = rpm_getopt(argc, argv, "ab:c:def:j", NULL, &opt);
    assert_int_equal(result, -1);
    assert_int_equal(opt.ind, 10);
    assert_int_equal(opt.opt, 0);
    assert_ptr_equal(opt.arg, NULL);
}

//
// prog -ax -bbval y
//
static void simple_option_and_arg_without_spaces(void **unused)
{
    opt_reset();
    char *argv[] = { "prog", "-ax", "-bbval", "y", 0 };
    int argc = 4;
    struct rpm_opt opt = {};

    int result = rpm_getopt(argc, argv, "ab:c:def:j", NULL, &opt);
    assert_int_equal(result, 'a');
    assert_int_equal(opt.ind, 1);
    assert_int_equal(opt.opt, 'a');
    assert_ptr_equal(opt.arg, NULL);

    result = rpm_getopt(argc, argv, "ab:c:def:j", NULL, &opt);
    assert_int_equal(result, '?');
    assert_int_equal(opt.ind, 2);
    assert_int_equal(opt.opt, '?');
    assert_string_equal(output, "prog: unknown option `-x`\r\n");

    result = rpm_getopt(argc, argv, "ab:c:def:j", NULL, &opt);
    assert_int_equal(result, 'b');
    assert_int_equal(opt.ind, 3);
    assert_int_equal(opt.opt, 'b');
    assert_string_equal(opt.arg, "bval");

    result = rpm_getopt(argc, argv, "ab:c:def:j", NULL, &opt);
    assert_int_equal(result, 1);
    assert_int_equal(opt.ind, 4);
    assert_int_equal(opt.opt, 1);
    assert_string_equal(opt.arg, "y");

    result = rpm_getopt(argc, argv, "ab:c:def:j", NULL, &opt);
    assert_int_equal(result, -1);
    assert_int_equal(opt.ind, 4);
    assert_int_equal(opt.opt, 0);
    assert_ptr_equal(opt.arg, NULL);
}

//
// prog -a -bbval -- -f x
//
static void separator_valid(void **unused)
{
    opt_reset();
    char *argv[] = { "prog", "-a", "-bbval", "--", "-f", "x", 0 };
    int argc = 6;
    struct rpm_opt opt = {};

    int result = rpm_getopt(argc, argv, "ab:c:def:j", NULL, &opt);
    assert_int_equal(result, 'a');
    assert_int_equal(opt.ind, 2);
    assert_int_equal(opt.opt, 'a');
    assert_ptr_equal(opt.arg, NULL);

    result = rpm_getopt(argc, argv, "ab:c:def:j", NULL, &opt);
    assert_int_equal(result, 'b');
    assert_int_equal(opt.ind, 3);
    assert_int_equal(opt.opt, 'b');
    assert_string_equal(opt.arg, "bval");

    result = rpm_getopt(argc, argv, "ab:c:def:j", NULL, &opt);
    assert_int_equal(result, -1);
    assert_int_equal(opt.ind, 4);
    assert_int_equal(opt.opt, 0);
    assert_ptr_equal(opt.arg, NULL);
}

//
// prog -a -bbval -f -- x
//
static void separator_as_option_value(void **unused)
{
    opt_reset();
    char *argv[] = { "prog", "-a", "-bbval", "-f", "--", "x", 0 };
    int argc = 6;
    struct rpm_opt opt = {};

    int result = rpm_getopt(argc, argv, "ab:c:def:j", NULL, &opt);
    assert_int_equal(result, 'a');
    assert_int_equal(opt.ind, 2);
    assert_int_equal(opt.opt, 'a');
    assert_ptr_equal(opt.arg, NULL);

    result = rpm_getopt(argc, argv, "ab:c:def:j", NULL, &opt);
    assert_int_equal(result, 'b');
    assert_int_equal(opt.ind, 3);
    assert_int_equal(opt.opt, 'b');
    assert_string_equal(opt.arg, "bval");

    result = rpm_getopt(argc, argv, "ab:c:def:j", NULL, &opt);
    assert_int_equal(result, 'f');
    assert_int_equal(opt.ind, 5);
    assert_int_equal(opt.opt, 'f');
    assert_string_equal(opt.arg, "--");

    result = rpm_getopt(argc, argv, "ab:c:def:j", NULL, &opt);
    assert_int_equal(result, 1);
    assert_int_equal(opt.ind, 6);
    assert_int_equal(opt.opt, 1);
    assert_string_equal(opt.arg, "x");

    result = rpm_getopt(argc, argv, "ab:c:def:j", NULL, &opt);
    assert_int_equal(result, -1);
    assert_int_equal(opt.ind, 6);
    assert_int_equal(opt.opt, 0);
    assert_ptr_equal(opt.arg, NULL);
}

//
// prog -
//
static void orphan_dash(void **unused)
{
    opt_reset();
    char *argv[] = { "prog", "-", 0 };
    int argc = 2;
    struct rpm_opt opt = {};

    int result = rpm_getopt(argc, argv, "ab:c:def:j", NULL, &opt);
    assert_int_equal(result, -1);
    assert_int_equal(opt.ind, 2);
    assert_int_equal(opt.opt, 0);
    assert_ptr_equal(opt.arg, NULL);
}

//
// prog --long
//
static void long_option(void **unused)
{
    opt_reset();
    char *argv[] = { "prog", "--long", 0 };
    int argc = 2;
    struct rpm_option long_opts[] = {
        { "long", RPM_NO_ARG, 0, 42 },
        {},
    };
    struct rpm_opt opt = {};

    int result = rpm_getopt(argc, argv, "", long_opts, &opt);
    assert_int_equal(result, 42);
    assert_int_equal(opt.long_index, 0);
    assert_int_equal(opt.ind, 2);
    assert_int_equal(opt.opt, 0);
    assert_ptr_equal(opt.arg, NULL);

    result = rpm_getopt(argc, argv, "", long_opts, &opt);
    assert_int_equal(result, -1);
    assert_int_equal(opt.long_index, 0);
    assert_int_equal(opt.ind, 2);
    assert_int_equal(opt.opt, 0);
    assert_ptr_equal(opt.arg, NULL);
}

//
// prog --long1 --long2 --long1 --long1 --long4
//
static void many_long_options(void **unused)
{
    opt_reset();
    char *argv[] = { "prog", "--long1", "--long2", "--long1", "--long1", "--long4", 0 };
    int argc = 6;
    struct rpm_option long_opts[] = {
        { "long1", RPM_NO_ARG, 0, 10 },
        { "long2", RPM_NO_ARG, 0, 11 },
        { "long3", RPM_NO_ARG, 0, 12 },
        {},
    };
    struct rpm_opt opt = {};

    int result = rpm_getopt(argc, argv, "", long_opts, &opt);
    assert_int_equal(result, 10);
    assert_int_equal(opt.long_index, 0);
    assert_int_equal(opt.ind, 2);
    assert_int_equal(opt.opt, 0);
    assert_ptr_equal(opt.arg, NULL);

    result = rpm_getopt(argc, argv, "", long_opts, &opt);
    assert_int_equal(result, 11);
    assert_int_equal(opt.long_index, 1);
    assert_int_equal(opt.ind, 3);
    assert_int_equal(opt.opt, 0);
    assert_ptr_equal(opt.arg, NULL);

    result = rpm_getopt(argc, argv, "", long_opts, &opt);
    assert_int_equal(result, 10);
    assert_int_equal(opt.long_index, 0);
    assert_int_equal(opt.ind, 4);
    assert_int_equal(opt.opt, 0);
    assert_ptr_equal(opt.arg, NULL);

    result = rpm_getopt(argc, argv, "", long_opts, &opt);
    assert_int_equal(result, 10);
    assert_int_equal(opt.long_index, 0);
    assert_int_equal(opt.ind, 5);
    assert_int_equal(opt.opt, 0);
    assert_ptr_equal(opt.arg, NULL);

    result = rpm_getopt(argc, argv, "", long_opts, &opt);
    assert_int_equal(result, '?');
    assert_int_equal(opt.long_index, 0);
    assert_int_equal(opt.ind, 6);
    assert_int_equal(opt.opt, '?');
    assert_string_equal(output, "prog: unknown option `--long4`\r\n");

    result = rpm_getopt(argc, argv, "", long_opts, &opt);
    assert_int_equal(result, -1);
    assert_int_equal(opt.long_index, 0);
    assert_int_equal(opt.ind, 6);
    assert_int_equal(opt.opt, 0);
    assert_ptr_equal(opt.arg, NULL);
}

//
// prog --long1 --long2 x y z
//
static void long_options_with_args(void **unused)
{
    opt_reset();
    char *argv[] = { "prog", "--long1", "--long2", "x", "y", "z", 0 };
    int argc = 6;
    struct rpm_option long_opts[] = {
        { "long1", RPM_NO_ARG, 0, 11 },
        { "long2", RPM_NO_ARG, 0, 22 },
        {},
    };
    struct rpm_opt opt = {};

    int result = rpm_getopt(argc, argv, "", long_opts, &opt);
    assert_int_equal(result, 11);
    assert_int_equal(opt.long_index, 0);
    assert_int_equal(opt.ind, 2);
    assert_int_equal(opt.opt, 0);
    assert_ptr_equal(opt.arg, NULL);

    result = rpm_getopt(argc, argv, "", long_opts, &opt);
    assert_int_equal(result, 22);
    assert_int_equal(opt.long_index, 1);
    assert_int_equal(opt.ind, 3);
    assert_int_equal(opt.opt, 0);
    assert_ptr_equal(opt.arg, NULL);

    result = rpm_getopt(argc, argv, "", long_opts, &opt);
    assert_int_equal(result, 1);
    assert_int_equal(opt.long_index, 1);
    assert_int_equal(opt.ind, 4);
    assert_int_equal(opt.opt, 1);
    assert_string_equal(opt.arg, "x");

    result = rpm_getopt(argc, argv, "", long_opts, &opt);
    assert_int_equal(result, 1);
    assert_int_equal(opt.long_index, 1);
    assert_int_equal(opt.ind, 5);
    assert_int_equal(opt.opt, 1);
    assert_string_equal(opt.arg, "y");

    result = rpm_getopt(argc, argv, "", long_opts, &opt);
    assert_int_equal(result, 1);
    assert_int_equal(opt.long_index, 1);
    assert_int_equal(opt.ind, 6);
    assert_int_equal(opt.opt, 1);
    assert_string_equal(opt.arg, "z");

    result = rpm_getopt(argc, argv, "", long_opts, &opt);
    assert_int_equal(result, -1);
    assert_int_equal(opt.long_index, 1);
    assert_int_equal(opt.ind, 6);
    assert_int_equal(opt.opt, 0);
    assert_ptr_equal(opt.arg, NULL);
}

//
// prog --long1
//
static void flag_ptr(void **unused)
{
    opt_reset();
    char *argv[] = { "prog", "--long1", 0 };
    int argc = 2;
    int flag = 0;
    struct rpm_option long_opts[] = {
        { "long1", RPM_NO_ARG, &flag, 11 },
        {},
    };
    struct rpm_opt opt = {};

    int result = rpm_getopt(argc, argv, "", long_opts, &opt);
    assert_int_equal(result, 0);
    assert_int_equal(opt.ind, 2);
    assert_int_equal(opt.opt, 0);
    assert_ptr_equal(opt.arg, NULL);
    assert_int_equal(flag, 11);

    result = rpm_getopt(argc, argv, "", long_opts, &opt);
    assert_int_equal(result, -1);
    assert_int_equal(opt.ind, 2);
    assert_int_equal(opt.opt, 0);
    assert_ptr_equal(opt.arg, NULL);
}

//
// prog --long2 arg --long3
//
static void mandatory_arg(void **unused)
{
    opt_reset();
    char *argv[] = { "prog", "--long2", "arg", "--long3", 0 };
    int argc = 4;
    struct rpm_option long_opts[] = {
        { "long2", RPM_REQUIRED_ARG, 0, 11 },
        {},
    };
    struct rpm_opt opt = {};

    int result = rpm_getopt(argc, argv, "", long_opts, &opt);
    assert_int_equal(result, 11);
    assert_int_equal(opt.ind, 3);
    assert_int_equal(opt.opt, 0);
    assert_string_equal(opt.arg, "arg");

    result = rpm_getopt(argc, argv, "", long_opts, &opt);
    assert_int_equal(result, '?');
    assert_int_equal(opt.ind, 4);
    assert_int_equal(opt.opt, '?');
    assert_string_equal(output, "prog: unknown option `--long3`\r\n");

    result = rpm_getopt(argc, argv, "", long_opts, &opt);
    assert_int_equal(result, -1);
    assert_int_equal(opt.ind, 4);
    assert_int_equal(opt.opt, 0);
    assert_ptr_equal(opt.arg, NULL);
}

//
// prog --long2 arg --long3
//
static void long_option_no_arg(void **unused)
{
    opt_reset();
    char *argv[] = { "prog", "--long2", "arg", "--long3", 0 };
    int argc = 4;
    struct rpm_option long_opts[] = {
        { "long2", RPM_NO_ARG, 0, 11 },
        {},
    };
    struct rpm_opt opt = {};

    int result = rpm_getopt(argc, argv, "", long_opts, &opt);
    assert_int_equal(result, 11);
    assert_int_equal(opt.ind, 2);
    assert_int_equal(opt.opt, 0);
    assert_ptr_equal(opt.arg, NULL);

    result = rpm_getopt(argc, argv, "", long_opts, &opt);
    assert_int_equal(result, 1);
    assert_int_equal(opt.ind, 3);
    assert_int_equal(opt.opt, 1);
    assert_string_equal(opt.arg, "arg");

    result = rpm_getopt(argc, argv, "", long_opts, &opt);
    assert_int_equal(result, '?');
    assert_int_equal(opt.ind, 4);
    assert_int_equal(opt.opt, '?');
    assert_string_equal(output, "prog: unknown option `--long3`\r\n");

    result = rpm_getopt(argc, argv, "", long_opts, &opt);
    assert_int_equal(result, -1);
    assert_int_equal(opt.ind, 4);
    assert_int_equal(opt.opt, 0);
    assert_ptr_equal(opt.arg, NULL);
}

//
// prog --long1=arg1 --long2=arg2
//
static void long_option_equals_arg(void **unused)
{
    opt_reset();
    char *argv[] = { "prog", "--long1=arg1", "--long2=arg2", 0 };
    int argc = 3;
    struct rpm_option long_opts[] = {
        { "long1", RPM_REQUIRED_ARG, 0, 11 },
        { "long2", RPM_NO_ARG, 0, 22 },
        {},
    };
    struct rpm_opt opt = {};

    int result = rpm_getopt(argc, argv, "", long_opts, &opt);
    assert_int_equal(result, 11);
    assert_int_equal(opt.ind, 2);
    assert_int_equal(opt.opt, 0);
    assert_string_equal(opt.arg, "arg1");

    result = rpm_getopt(argc, argv, "", long_opts, &opt);
    assert_int_equal(result, 22);
    assert_int_equal(opt.ind, 3);
    assert_int_equal(opt.opt, 0);
    assert_ptr_equal(opt.arg, NULL); // arg2 is ignored

    result = rpm_getopt(argc, argv, "", long_opts, &opt);
    assert_int_equal(result, -1);
    assert_int_equal(opt.ind, 3);
    assert_int_equal(opt.opt, 0);
    assert_ptr_equal(opt.arg, NULL);
}

//
// prog --long1 --long2=arg
//
static void optional_args(void **unused)
{
    opt_reset();
    char *argv[] = { "prog", "--long1", "--long2=arg", 0 };
    int argc = 3;
    struct rpm_option long_opts[] = {
        { "long1", RPM_OPTIONAL_ARG, 0, 11 },
        { "long2", RPM_OPTIONAL_ARG, 0, 22 },
        {},
    };
    struct rpm_opt opt = {};
    int result = rpm_getopt(argc, argv, "", long_opts, &opt);
    assert_int_equal(result, 11);
    assert_int_equal(opt.ind, 2);
    assert_int_equal(opt.opt, 0);
    assert_ptr_equal(opt.arg, NULL);

    result = rpm_getopt(argc, argv, "", long_opts, &opt);
    assert_int_equal(result, 22);
    assert_int_equal(opt.ind, 3);
    assert_int_equal(opt.opt, 0);
    assert_string_equal(opt.arg, "arg");

    result = rpm_getopt(argc, argv, "", long_opts, &opt);
    assert_int_equal(result, -1);
    assert_int_equal(opt.ind, 3);
    assert_int_equal(opt.opt, 0);
    assert_ptr_equal(opt.arg, NULL);
}

//
// prog -a -# 6 -H1 -H 2 -ka -l -lv -x path=./foo bar
//
static void short_test1(void **unused)
{
    opt_reset();
    char *argv[] = { "prog", "-a", "-#", "6", "-H1", "-H", "2", "-ka", "-l", "-lv", "-x", "path=./foo", "bar", 0 };
    int argc = 13;
    const char *short_opts = "#:abc::CdDeE::fFgGhH:iIk:l::mMnNo::O:pPqQrRsS:t:u:UvVwW::x:yz";
    struct rpm_opt opt = {};

    int result = rpm_getopt(argc, argv, short_opts, NULL, &opt);
    assert_int_equal(result, 'a');
    assert_int_equal(opt.ind, 2);
    assert_int_equal(opt.opt, 'a');
    assert_ptr_equal(opt.arg, NULL);

    result = rpm_getopt(argc, argv, short_opts, NULL, &opt);
    assert_int_equal(result, '#');
    assert_int_equal(opt.ind, 4);
    assert_int_equal(opt.opt, '#');
    assert_string_equal(opt.arg, "6");

    result = rpm_getopt(argc, argv, short_opts, NULL, &opt);
    assert_int_equal(result, 'H');
    assert_int_equal(opt.ind, 5);
    assert_int_equal(opt.opt, 'H');
    assert_string_equal(opt.arg, "1");

    result = rpm_getopt(argc, argv, short_opts, NULL, &opt);
    assert_int_equal(result, 'H');
    assert_int_equal(opt.ind, 7);
    assert_int_equal(opt.opt, 'H');
    assert_string_equal(opt.arg, "2");

    result = rpm_getopt(argc, argv, short_opts, NULL, &opt);
    assert_int_equal(result, 'k');
    assert_int_equal(opt.ind, 8);
    assert_int_equal(opt.opt, 'k');
    assert_string_equal(opt.arg, "a");

    result = rpm_getopt(argc, argv, short_opts, NULL, &opt);
    assert_int_equal(result, 'l');
    assert_int_equal(opt.ind, 9);
    assert_int_equal(opt.opt, 'l');
    assert_ptr_equal(opt.arg, NULL);

    result = rpm_getopt(argc, argv, short_opts, NULL, &opt);
    assert_int_equal(result, 'l');
    assert_int_equal(opt.ind, 10);
    assert_int_equal(opt.opt, 'l');
    assert_string_equal(opt.arg, "v");

    result = rpm_getopt(argc, argv, short_opts, NULL, &opt);
    assert_int_equal(result, 'x');
    assert_int_equal(opt.ind, 12);
    assert_int_equal(opt.opt, 'x');
    assert_string_equal(opt.arg, "path=./foo");

    result = rpm_getopt(argc, argv, short_opts, NULL, &opt);
    assert_int_equal(result, 1);
    assert_int_equal(opt.ind, 13);
    assert_int_equal(opt.opt, 1);
    assert_string_equal(opt.arg, "bar");

    result = rpm_getopt(argc, argv, short_opts, NULL, &opt);
    assert_int_equal(result, -1);
    assert_int_equal(opt.ind, 13);
    assert_int_equal(opt.opt, 0);
    assert_ptr_equal(opt.arg, NULL);
}

//
// prog -a -#pound -b -cfilename
//
static void short_test2(void **unused)
{
    opt_reset();
    char *argv[] = { "prog", "-a", "-#pound", "-b", "-cfilename", 0 };
    int argc = 5;
    const char *short_opts = "#:abc::";
    struct rpm_opt opt = {};

    int result = rpm_getopt(argc, argv, short_opts, NULL, &opt);
    assert_int_equal(result, 'a');
    assert_int_equal(opt.ind, 2);
    assert_int_equal(opt.opt, 'a');
    assert_ptr_equal(opt.arg, NULL);

    result = rpm_getopt(argc, argv, short_opts, NULL, &opt);
    assert_int_equal(result, '#');
    assert_int_equal(opt.ind, 3);
    assert_int_equal(opt.opt, '#');
    assert_string_equal(opt.arg, "pound");

    result = rpm_getopt(argc, argv, short_opts, NULL, &opt);
    assert_int_equal(result, 'b');
    assert_int_equal(opt.ind, 4);
    assert_int_equal(opt.opt, 'b');
    assert_ptr_equal(opt.arg, NULL);

    result = rpm_getopt(argc, argv, short_opts, NULL, &opt);
    assert_int_equal(result, 'c');
    assert_int_equal(opt.ind, 5);
    assert_int_equal(opt.opt, 'c');
    assert_string_equal(opt.arg, "filename");

    result = rpm_getopt(argc, argv, short_opts, NULL, &opt);
    assert_int_equal(result, -1);
    assert_int_equal(opt.ind, 5);
    assert_int_equal(opt.opt, 0);
    assert_ptr_equal(opt.arg, NULL);
}

//
// prog -a -#bx -b -c -cfoo -d bar
//
static void short_test3(void **unused)
{
    opt_reset();
    char *argv[] = { "prog", "-a", "-#bx", "-b", "-c", "-cfoo", "-d", "bar", 0 };
    int argc = 8;
    const char *short_opts = "#:abc::d";
    struct rpm_opt opt = {};

    int result = rpm_getopt(argc, argv, short_opts, NULL, &opt);
    assert_int_equal(result, 'a');
    assert_int_equal(opt.ind, 2);
    assert_int_equal(opt.opt, 'a');
    assert_ptr_equal(opt.arg, NULL);

    result = rpm_getopt(argc, argv, short_opts, NULL, &opt);
    assert_int_equal(result, '#');
    assert_int_equal(opt.ind, 3);
    assert_int_equal(opt.opt, '#');
    assert_string_equal(opt.arg, "bx");

    result = rpm_getopt(argc, argv, short_opts, NULL, &opt);
    assert_int_equal(result, 'b');
    assert_int_equal(opt.ind, 4);
    assert_int_equal(opt.opt, 'b');
    assert_ptr_equal(opt.arg, NULL);

    result = rpm_getopt(argc, argv, short_opts, NULL, &opt);
    assert_int_equal(result, 'c');
    assert_int_equal(opt.ind, 5);
    assert_int_equal(opt.opt, 'c');
    assert_ptr_equal(opt.arg, NULL);

    result = rpm_getopt(argc, argv, short_opts, NULL, &opt);
    assert_int_equal(result, 'c');
    assert_int_equal(opt.ind, 6);
    assert_int_equal(opt.opt, 'c');
    assert_string_equal(opt.arg, "foo");

    result = rpm_getopt(argc, argv, short_opts, NULL, &opt);
    assert_int_equal(result, 'd');
    assert_int_equal(opt.ind, 7);
    assert_int_equal(opt.opt, 'd');
    assert_ptr_equal(opt.arg, NULL);

    result = rpm_getopt(argc, argv, short_opts, NULL, &opt);
    assert_int_equal(result, 1);
    assert_int_equal(opt.ind, 8);
    assert_int_equal(opt.opt, 1);
    assert_string_equal(opt.arg, "bar");

    result = rpm_getopt(argc, argv, short_opts, NULL, &opt);
    assert_int_equal(result, -1);
    assert_int_equal(opt.ind, 8);
    assert_int_equal(opt.opt, 0);
    assert_ptr_equal(opt.arg, NULL);
}

//
// prog -ab -a -cx -c y -d -da
//
static void short_test5(void **unused)
{
    opt_reset();
    char *argv[] = { "prog", "-ab", "-a", "-cx", "-c", "y", "-d", "-da", 0 };
    int argc = 8;
    const char *short_opts = "abc:d::";
    struct rpm_opt opt = {};

    int result = rpm_getopt(argc, argv, short_opts, NULL, &opt);
    assert_int_equal(result, 'a');
    assert_int_equal(opt.ind, 1);
    assert_int_equal(opt.opt, 'a');
    assert_ptr_equal(opt.arg, NULL);

    result = rpm_getopt(argc, argv, short_opts, NULL, &opt);
    assert_int_equal(result, 'b');
    assert_int_equal(opt.ind, 2);
    assert_int_equal(opt.opt, 'b');
    assert_ptr_equal(opt.arg, NULL);

    result = rpm_getopt(argc, argv, short_opts, NULL, &opt);
    assert_int_equal(result, 'a');
    assert_int_equal(opt.ind, 3);
    assert_int_equal(opt.opt, 'a');
    assert_ptr_equal(opt.arg, NULL);

    result = rpm_getopt(argc, argv, short_opts, NULL, &opt);
    assert_int_equal(result, 'c');
    assert_int_equal(opt.ind, 4);
    assert_int_equal(opt.opt, 'c');
    assert_string_equal(opt.arg, "x");

    result = rpm_getopt(argc, argv, short_opts, NULL, &opt);
    assert_int_equal(result, 'c');
    assert_int_equal(opt.ind, 6);
    assert_int_equal(opt.opt, 'c');
    assert_string_equal(opt.arg, "y");

    result = rpm_getopt(argc, argv, short_opts, NULL, &opt);
    assert_int_equal(result, 'd');
    assert_int_equal(opt.ind, 7);
    assert_int_equal(opt.opt, 'd');
    assert_ptr_equal(opt.arg, NULL);

    result = rpm_getopt(argc, argv, short_opts, NULL, &opt);
    assert_int_equal(result, 'd');
    assert_int_equal(opt.ind, 8);
    assert_int_equal(opt.opt, 'd');
    assert_string_equal(opt.arg, "a");

    result = rpm_getopt(argc, argv, short_opts, NULL, &opt);
    assert_int_equal(result, -1);
    assert_int_equal(opt.ind, 8);
    assert_int_equal(opt.opt, 0);
    assert_ptr_equal(opt.arg, NULL);
}

//
// prog -H
//
static void short_test6(void **unused)
{
    opt_reset();
    char *argv[] = { "prog", "-H", 0 };
    int argc = 2;
    const char *short_opts = "abH:d::";
    struct rpm_opt opt = {};

    int result = rpm_getopt(argc, argv, short_opts, NULL, &opt);
    assert_int_equal(result, '?');
    assert_int_equal(opt.ind, 2);
    assert_int_equal(opt.opt, 'H');
    assert_string_equal(output, "prog: argument required for option `-H`\r\n");

    result = rpm_getopt(argc, argv, short_opts, NULL, &opt);
    assert_int_equal(result, -1);
    assert_int_equal(opt.ind, 2);
    assert_int_equal(opt.opt, 0);
    assert_ptr_equal(opt.arg, NULL);
}

//
// prog -H
//
static void short_test7(void **unused)
{
    opt_reset();
    char *argv[] = { "prog", "-H", 0 };
    int argc = 2;
    const char *short_opts = ":abH:d::"; // Leading ':' in opt string
    struct rpm_opt opt = {};

    int result = rpm_getopt(argc, argv, short_opts, NULL, &opt);
    assert_int_equal(result, ':');
    assert_int_equal(opt.ind, 2);
    assert_int_equal(opt.opt, 'H');
    assert_string_equal(output, "prog: argument required for option `-H`\r\n");

    result = rpm_getopt(argc, argv, short_opts, NULL, &opt);
    assert_int_equal(result, -1);
    assert_int_equal(opt.ind, 2);
    assert_int_equal(opt.opt, 0);
    assert_ptr_equal(opt.arg, NULL);
}

//
// prog -x
//
static void short_test8(void **unused)
{
    opt_reset();
    char *argv[] = { "prog", "-x", 0 };
    int argc = 2;
    const char *short_opts = "abH:d::";
    struct rpm_opt opt = {};

    int result = rpm_getopt(argc, argv, short_opts, NULL, &opt);
    assert_int_equal(result, '?');
    assert_int_equal(opt.ind, 2);
    assert_int_equal(opt.opt, '?');
    assert_string_equal(output, "prog: unknown option `-x`\r\n");

    result = rpm_getopt(argc, argv, short_opts, NULL, &opt);
    assert_int_equal(result, -1);
    assert_int_equal(opt.ind, 2);
    assert_int_equal(opt.opt, 0);
    assert_ptr_equal(opt.arg, NULL);
}

//
// Run all tests.
//
int main()
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(option_h_valid),
        cmocka_unit_test(option_h_missing),
        cmocka_unit_test(option_i_1),
        cmocka_unit_test(no_args),
        cmocka_unit_test(three_args),
        cmocka_unit_test(simple_option),
        cmocka_unit_test(simple_option_with_args),
        cmocka_unit_test(many_simple_options),
        cmocka_unit_test(option_mix_without_args),
        cmocka_unit_test(option_mix_with_args),
        cmocka_unit_test(option_mix_shuffled_args),
        cmocka_unit_test(simple_option_and_arg_without_spaces),
        cmocka_unit_test(separator_valid),
        cmocka_unit_test(separator_as_option_value),
        cmocka_unit_test(orphan_dash),
        cmocka_unit_test(long_option),
        cmocka_unit_test(many_long_options),
        cmocka_unit_test(long_options_with_args),
        cmocka_unit_test(flag_ptr),
        cmocka_unit_test(mandatory_arg),
        cmocka_unit_test(long_option_no_arg),
        cmocka_unit_test(long_option_equals_arg),
        cmocka_unit_test(optional_args),
        cmocka_unit_test(short_test1),
        cmocka_unit_test(short_test2),
        cmocka_unit_test(short_test3),
        cmocka_unit_test(short_test5),
        cmocka_unit_test(short_test6),
        cmocka_unit_test(short_test7),
        cmocka_unit_test(short_test8),
    };
    return cmocka_run_group_tests(tests, NULL, NULL);
}
