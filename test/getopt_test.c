#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <cmocka.h>
#include <rpm/api.h>

//
// Buffer for stdout messages from rpm_getopt().
//
static char output[2048];
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
    rpm_optind = 0;
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
    int result = rpm_getopt(2, argv, "h", NULL, NULL);
    assert_int_equal(result, 'h');
    assert_string_equal(output, "");
}

//
// prog -h --- missing option
//
static void option_h_missing(void **unused)
{
    opt_reset();
    char *argv[] = { "prog", "-h", 0 };
    int result = rpm_getopt(2, argv, "a", NULL, NULL);
    assert_int_equal(result, '?');
    assert_string_equal(output, "prog: unknown option `-h`\n");
}

//
// prog -i 1
//
static void option_i_1(void **unused)
{
    opt_reset();
    char *argv[] = { "prog", "-i", "1", 0 };
    int result = rpm_getopt(3, argv, "i:", NULL, NULL);
    assert_int_equal(result, 'i');
    assert_string_equal(rpm_optarg, "1");
    assert_string_equal(output, "");
}

//
// prog
//
static void no_args(void **unused)
{
    opt_reset();
    char *argv[] = { "prog", 0 };
    int result = rpm_getopt(1, argv, "ab:c:def:j", NULL, NULL);
    assert_int_equal(result, -1);
    assert_int_equal(rpm_optind, 1);
    assert_int_equal(rpm_optopt, 0);
    assert_ptr_equal(rpm_optarg, NULL);
}

//
// prog x y z
//
static void three_args(void **unused)
{
    opt_reset();
    char *argv[] = { "prog", "x", "y", "z", 0 };
    int argc = 4;
    int result = rpm_getopt(argc, argv, "ab:c:def:j", NULL, NULL);
    assert_int_equal(result, 1);
    assert_int_equal(rpm_optind, 2);
    assert_int_equal(rpm_optopt, 1);
    assert_string_equal(rpm_optarg, "x");

    result = rpm_getopt(argc, argv, "ab:c:def:j", NULL, NULL);
    assert_int_equal(result, 1);
    assert_int_equal(rpm_optind, 3);
    assert_int_equal(rpm_optopt, 1);
    assert_string_equal(rpm_optarg, "y");

    result = rpm_getopt(argc, argv, "ab:c:def:j", NULL, NULL);
    assert_int_equal(result, 1);
    assert_int_equal(rpm_optind, 4);
    assert_int_equal(rpm_optopt, 1);
    assert_string_equal(rpm_optarg, "z");

    result = rpm_getopt(argc, argv, "ab:c:def:j", NULL, NULL);
    assert_int_equal(result, -1);
    assert_int_equal(rpm_optind, 4);
    assert_int_equal(rpm_optopt, 0);
    assert_ptr_equal(rpm_optarg, NULL);
}

//
// prog -a
//
static void simple_option(void **unused)
{
    opt_reset();
    char *argv[] = { "prog", "-a", 0 };
    int argc = 2;
    int result = rpm_getopt(argc, argv, "ab:c:def:j", NULL, NULL);
    assert_int_equal(result, 'a');
    assert_int_equal(rpm_optind, 2);
    assert_int_equal(rpm_optopt, 'a');
    assert_ptr_equal(rpm_optarg, NULL);

    result = rpm_getopt(argc, argv, "ab:c:def:j", NULL, NULL);
    assert_int_equal(result, -1);
    assert_int_equal(rpm_optind, 2);
    assert_int_equal(rpm_optopt, 0);
    assert_ptr_equal(rpm_optarg, NULL);
}

//
// prog -a x y z
//
static void simple_option_with_args(void **unused)
{
    opt_reset();
    char *argv[] = { "prog", "-a", "x", "y", "z", 0 };
    int argc = 5;
    int result = rpm_getopt(argc, argv, "ab:c:def:j", NULL, NULL);
    assert_int_equal(result, 'a');
    assert_int_equal(rpm_optind, 2);
    assert_int_equal(rpm_optopt, 'a');
    assert_ptr_equal(rpm_optarg, NULL);

    result = rpm_getopt(argc, argv, "ab:c:def:j", NULL, NULL);
    assert_int_equal(result, 1);
    assert_int_equal(rpm_optind, 3);
    assert_int_equal(rpm_optopt, 1);
    assert_string_equal(rpm_optarg, "x");

    result = rpm_getopt(argc, argv, "ab:c:def:j", NULL, NULL);
    assert_int_equal(result, 1);
    assert_int_equal(rpm_optind, 4);
    assert_int_equal(rpm_optopt, 1);
    assert_string_equal(rpm_optarg, "y");

    result = rpm_getopt(argc, argv, "ab:c:def:j", NULL, NULL);
    assert_int_equal(result, 1);
    assert_int_equal(rpm_optind, 5);
    assert_int_equal(rpm_optopt, 1);
    assert_string_equal(rpm_optarg, "z");

    result = rpm_getopt(argc, argv, "ab:c:def:j", NULL, NULL);
    assert_int_equal(result, -1);
    assert_int_equal(rpm_optind, 5);
    assert_int_equal(rpm_optopt, 0);
    assert_ptr_equal(rpm_optarg, NULL);
}

//
// prog -ade -j x y z
//
static void many_simple_options(void **unused)
{
    opt_reset();
    char *argv[] = { "prog", "-ade", "-j", "x", "y", "z", 0 };
    int argc = 6;
    int result = rpm_getopt(argc, argv, "ab:c:def:j", NULL, NULL);
    assert_int_equal(result, 'a');
    assert_int_equal(rpm_optind, 1);
    assert_int_equal(rpm_optopt, 'a');
    assert_ptr_equal(rpm_optarg, NULL);

    result = rpm_getopt(argc, argv, "ab:c:def:j", NULL, NULL);
    assert_int_equal(result, 'd');
    assert_int_equal(rpm_optind, 1);
    assert_int_equal(rpm_optopt, 'd');
    assert_ptr_equal(rpm_optarg, NULL);

    result = rpm_getopt(argc, argv, "ab:c:def:j", NULL, NULL);
    assert_int_equal(result, 'e');
    assert_int_equal(rpm_optind, 2);
    assert_int_equal(rpm_optopt, 'e');
    assert_ptr_equal(rpm_optarg, NULL);

    result = rpm_getopt(argc, argv, "ab:c:def:j", NULL, NULL);
    assert_int_equal(result, 'j');
    assert_int_equal(rpm_optind, 3);
    assert_int_equal(rpm_optopt, 'j');
    assert_ptr_equal(rpm_optarg, NULL);

    result = rpm_getopt(argc, argv, "ab:c:def:j", NULL, NULL);
    assert_int_equal(result, 1);
    assert_int_equal(rpm_optind, 4);
    assert_int_equal(rpm_optopt, 1);
    assert_string_equal(rpm_optarg, "x");

    result = rpm_getopt(argc, argv, "ab:c:def:j", NULL, NULL);
    assert_int_equal(result, 1);
    assert_int_equal(rpm_optind, 5);
    assert_int_equal(rpm_optopt, 1);
    assert_string_equal(rpm_optarg, "y");

    result = rpm_getopt(argc, argv, "ab:c:def:j", NULL, NULL);
    assert_int_equal(result, 1);
    assert_int_equal(rpm_optind, 6);
    assert_int_equal(rpm_optopt, 1);
    assert_string_equal(rpm_optarg, "z");

    result = rpm_getopt(argc, argv, "ab:c:def:j", NULL, NULL);
    assert_int_equal(result, -1);
    assert_int_equal(rpm_optind, 6);
    assert_int_equal(rpm_optopt, 0);
    assert_ptr_equal(rpm_optarg, NULL);
}

//
// prog -abbval -c cval -def fval -j
//
static void option_mix_without_args(void **unused)
{
    opt_reset();
    char *argv[] = { "prog", "-abbval", "-c", "cval", "-def", "fval", "-j", 0 };
    int argc = 7;
    int result = rpm_getopt(argc, argv, "ab:c:def:j", NULL, NULL);
    assert_int_equal(result, 'a');
    assert_int_equal(rpm_optind, 1);
    assert_int_equal(rpm_optopt, 'a');
    assert_ptr_equal(rpm_optarg, NULL);

    result = rpm_getopt(argc, argv, "ab:c:def:j", NULL, NULL);
    assert_int_equal(result, 'b');
    assert_int_equal(rpm_optind, 2);
    assert_int_equal(rpm_optopt, 'b');
    assert_string_equal(rpm_optarg, "bval");

    result = rpm_getopt(argc, argv, "ab:c:def:j", NULL, NULL);
    assert_int_equal(result, 'c');
    assert_int_equal(rpm_optind, 4);
    assert_int_equal(rpm_optopt, 'c');
    assert_string_equal(rpm_optarg, "cval");

    result = rpm_getopt(argc, argv, "ab:c:def:j", NULL, NULL);
    assert_int_equal(result, 'd');
    assert_int_equal(rpm_optind, 4);
    assert_int_equal(rpm_optopt, 'd');
    assert_ptr_equal(rpm_optarg, NULL);

    result = rpm_getopt(argc, argv, "ab:c:def:j", NULL, NULL);
    assert_int_equal(result, 'e');
    assert_int_equal(rpm_optind, 4);
    assert_int_equal(rpm_optopt, 'e');
    assert_ptr_equal(rpm_optarg, NULL);

    result = rpm_getopt(argc, argv, "ab:c:def:j", NULL, NULL);
    assert_int_equal(result, 'f');
    assert_int_equal(rpm_optind, 6);
    assert_int_equal(rpm_optopt, 'f');
    assert_string_equal(rpm_optarg, "fval");

    result = rpm_getopt(argc, argv, "ab:c:def:j", NULL, NULL);
    assert_int_equal(result, 'j');
    assert_int_equal(rpm_optind, 7);
    assert_int_equal(rpm_optopt, 'j');
    assert_ptr_equal(rpm_optarg, NULL);

    result = rpm_getopt(argc, argv, "ab:c:def:j", NULL, NULL);
    assert_int_equal(result, -1);
    assert_int_equal(rpm_optind, 7);
    assert_int_equal(rpm_optopt, 0);
    assert_ptr_equal(rpm_optarg, NULL);
}

//
// prog -abbval -c cval -def fval -j x y
//
static void option_mix_with_args(void **unused)
{
    opt_reset();
    char *argv[] = { "prog", "-abbval", "-c", "cval", "-def", "fval", "-j", "x", "y", 0 };
    int argc = 9;
    int result = rpm_getopt(argc, argv, "ab:c:def:j", NULL, NULL);
    assert_int_equal(result, 'a');
    assert_int_equal(rpm_optind, 1);
    assert_int_equal(rpm_optopt, 'a');
    assert_ptr_equal(rpm_optarg, NULL);

    result = rpm_getopt(argc, argv, "ab:c:def:j", NULL, NULL);
    assert_int_equal(result, 'b');
    assert_int_equal(rpm_optind, 2);
    assert_int_equal(rpm_optopt, 'b');
    assert_string_equal(rpm_optarg, "bval");

    result = rpm_getopt(argc, argv, "ab:c:def:j", NULL, NULL);
    assert_int_equal(result, 'c');
    assert_int_equal(rpm_optind, 4);
    assert_int_equal(rpm_optopt, 'c');
    assert_string_equal(rpm_optarg, "cval");

    result = rpm_getopt(argc, argv, "ab:c:def:j", NULL, NULL);
    assert_int_equal(result, 'd');
    assert_int_equal(rpm_optind, 4);
    assert_int_equal(rpm_optopt, 'd');
    assert_ptr_equal(rpm_optarg, NULL);

    result = rpm_getopt(argc, argv, "ab:c:def:j", NULL, NULL);
    assert_int_equal(result, 'e');
    assert_int_equal(rpm_optind, 4);
    assert_int_equal(rpm_optopt, 'e');
    assert_ptr_equal(rpm_optarg, NULL);

    result = rpm_getopt(argc, argv, "ab:c:def:j", NULL, NULL);
    assert_int_equal(result, 'f');
    assert_int_equal(rpm_optind, 6);
    assert_int_equal(rpm_optopt, 'f');
    assert_string_equal(rpm_optarg, "fval");

    result = rpm_getopt(argc, argv, "ab:c:def:j", NULL, NULL);
    assert_int_equal(result, 'j');
    assert_int_equal(rpm_optind, 7);
    assert_int_equal(rpm_optopt, 'j');
    assert_ptr_equal(rpm_optarg, NULL);

    result = rpm_getopt(argc, argv, "ab:c:def:j", NULL, NULL);
    assert_int_equal(result, 1);
    assert_int_equal(rpm_optind, 8);
    assert_int_equal(rpm_optopt, 1);
    assert_string_equal(rpm_optarg, "x");

    result = rpm_getopt(argc, argv, "ab:c:def:j", NULL, NULL);
    assert_int_equal(result, 1);
    assert_int_equal(rpm_optind, 9);
    assert_int_equal(rpm_optopt, 1);
    assert_string_equal(rpm_optarg, "y");

    result = rpm_getopt(argc, argv, "ab:c:def:j", NULL, NULL);
    assert_int_equal(result, -1);
    assert_int_equal(rpm_optind, 9);
    assert_int_equal(rpm_optopt, 0);
    assert_ptr_equal(rpm_optarg, NULL);
}

//
// prog -abbval x -c cval -def fval y -j z
//
static void option_mix_shuffled_args(void **unused)
{
    opt_reset();
    char *argv[] = { "prog", "-abbval", "x", "-c", "cval", "-def", "fval", "y", "-j", "z", 0 };
    int argc = 10;
    int result = rpm_getopt(argc, argv, "ab:c:def:j", NULL, NULL);
    assert_int_equal(result, 'a');
    assert_int_equal(rpm_optind, 1);
    assert_int_equal(rpm_optopt, 'a');
    assert_ptr_equal(rpm_optarg, NULL);

    result = rpm_getopt(argc, argv, "ab:c:def:j", NULL, NULL);
    assert_int_equal(result, 'b');
    assert_int_equal(rpm_optind, 2);
    assert_int_equal(rpm_optopt, 'b');
    assert_string_equal(rpm_optarg, "bval");

    result = rpm_getopt(argc, argv, "ab:c:def:j", NULL, NULL);
    assert_int_equal(result, 1);
    assert_int_equal(rpm_optind, 3);
    assert_int_equal(rpm_optopt, 1);
    assert_string_equal(rpm_optarg, "x");

    result = rpm_getopt(argc, argv, "ab:c:def:j", NULL, NULL);
    assert_int_equal(result, 'c');
    assert_int_equal(rpm_optind, 5);
    assert_int_equal(rpm_optopt, 'c');
    assert_string_equal(rpm_optarg, "cval");

    result = rpm_getopt(argc, argv, "ab:c:def:j", NULL, NULL);
    assert_int_equal(result, 'd');
    assert_int_equal(rpm_optind, 5);
    assert_int_equal(rpm_optopt, 'd');
    assert_ptr_equal(rpm_optarg, NULL);

    result = rpm_getopt(argc, argv, "ab:c:def:j", NULL, NULL);
    assert_int_equal(result, 'e');
    assert_int_equal(rpm_optind, 5);
    assert_int_equal(rpm_optopt, 'e');
    assert_ptr_equal(rpm_optarg, NULL);

    result = rpm_getopt(argc, argv, "ab:c:def:j", NULL, NULL);
    assert_int_equal(result, 'f');
    assert_int_equal(rpm_optind, 7);
    assert_int_equal(rpm_optopt, 'f');
    assert_string_equal(rpm_optarg, "fval");

    result = rpm_getopt(argc, argv, "ab:c:def:j", NULL, NULL);
    assert_int_equal(result, 1);
    assert_int_equal(rpm_optind, 8);
    assert_int_equal(rpm_optopt, 1);
    assert_string_equal(rpm_optarg, "y");

    result = rpm_getopt(argc, argv, "ab:c:def:j", NULL, NULL);
    assert_int_equal(result, 'j');
    assert_int_equal(rpm_optind, 9);
    assert_int_equal(rpm_optopt, 'j');
    assert_ptr_equal(rpm_optarg, NULL);

    result = rpm_getopt(argc, argv, "ab:c:def:j", NULL, NULL);
    assert_int_equal(result, 1);
    assert_int_equal(rpm_optind, 10);
    assert_int_equal(rpm_optopt, 1);
    assert_string_equal(rpm_optarg, "z");

    result = rpm_getopt(argc, argv, "ab:c:def:j", NULL, NULL);
    assert_int_equal(result, -1);
    assert_int_equal(rpm_optind, 10);
    assert_int_equal(rpm_optopt, 0);
    assert_ptr_equal(rpm_optarg, NULL);
}

//
// prog -ax -bbval y
//
static void simple_option_and_arg_without_spaces(void **unused)
{
    opt_reset();
    char *argv[] = { "prog", "-ax", "-bbval", "y", 0 };
    int argc = 4;
    int result = rpm_getopt(argc, argv, "ab:c:def:j", NULL, NULL);
    assert_int_equal(result, 'a');
    assert_int_equal(rpm_optind, 1);
    assert_int_equal(rpm_optopt, 'a');
    assert_ptr_equal(rpm_optarg, NULL);

    result = rpm_getopt(argc, argv, "ab:c:def:j", NULL, NULL);
    assert_int_equal(result, '?');
    assert_int_equal(rpm_optind, 2);
    assert_int_equal(rpm_optopt, '?');
    assert_string_equal(output, "prog: unknown option `-x`\n");

    result = rpm_getopt(argc, argv, "ab:c:def:j", NULL, NULL);
    assert_int_equal(result, 'b');
    assert_int_equal(rpm_optind, 3);
    assert_int_equal(rpm_optopt, 'b');
    assert_string_equal(rpm_optarg, "bval");

    result = rpm_getopt(argc, argv, "ab:c:def:j", NULL, NULL);
    assert_int_equal(result, 1);
    assert_int_equal(rpm_optind, 4);
    assert_int_equal(rpm_optopt, 1);
    assert_string_equal(rpm_optarg, "y");

    result = rpm_getopt(argc, argv, "ab:c:def:j", NULL, NULL);
    assert_int_equal(result, -1);
    assert_int_equal(rpm_optind, 4);
    assert_int_equal(rpm_optopt, 0);
    assert_ptr_equal(rpm_optarg, NULL);
}

//
// prog -a -bbval -- -f x
//
static void separator_valid(void **unused)
{
    opt_reset();
    char *argv[] = { "prog", "-a", "-bbval", "--", "-f", "x", 0 };
    int argc = 6;
    int result = rpm_getopt(argc, argv, "ab:c:def:j", NULL, NULL);
    assert_int_equal(result, 'a');
    assert_int_equal(rpm_optind, 2);
    assert_int_equal(rpm_optopt, 'a');
    assert_ptr_equal(rpm_optarg, NULL);

    result = rpm_getopt(argc, argv, "ab:c:def:j", NULL, NULL);
    assert_int_equal(result, 'b');
    assert_int_equal(rpm_optind, 3);
    assert_int_equal(rpm_optopt, 'b');
    assert_string_equal(rpm_optarg, "bval");

    result = rpm_getopt(argc, argv, "ab:c:def:j", NULL, NULL);
    assert_int_equal(result, -1);
    assert_int_equal(rpm_optind, 4);
    assert_int_equal(rpm_optopt, 0);
    assert_ptr_equal(rpm_optarg, NULL);
}

//
// prog -a -bbval -f -- x
//
static void separator_as_option_value(void **unused)
{
    opt_reset();
    char *argv[] = { "prog", "-a", "-bbval", "-f", "--", "x", 0 };
    int argc = 6;
    int result = rpm_getopt(argc, argv, "ab:c:def:j", NULL, NULL);
    assert_int_equal(result, 'a');
    assert_int_equal(rpm_optind, 2);
    assert_int_equal(rpm_optopt, 'a');
    assert_ptr_equal(rpm_optarg, NULL);

    result = rpm_getopt(argc, argv, "ab:c:def:j", NULL, NULL);
    assert_int_equal(result, 'b');
    assert_int_equal(rpm_optind, 3);
    assert_int_equal(rpm_optopt, 'b');
    assert_string_equal(rpm_optarg, "bval");

    result = rpm_getopt(argc, argv, "ab:c:def:j", NULL, NULL);
    assert_int_equal(result, 'f');
    assert_int_equal(rpm_optind, 5);
    assert_int_equal(rpm_optopt, 'f');
    assert_string_equal(rpm_optarg, "--");

    result = rpm_getopt(argc, argv, "ab:c:def:j", NULL, NULL);
    assert_int_equal(result, 1);
    assert_int_equal(rpm_optind, 6);
    assert_int_equal(rpm_optopt, 1);
    assert_string_equal(rpm_optarg, "x");

    result = rpm_getopt(argc, argv, "ab:c:def:j", NULL, NULL);
    assert_int_equal(result, -1);
    assert_int_equal(rpm_optind, 6);
    assert_int_equal(rpm_optopt, 0);
    assert_ptr_equal(rpm_optarg, NULL);
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
    int longindex = -123;
    int result = rpm_getopt(argc, argv, "", long_opts, &longindex);
    assert_int_equal(result, 42);
    assert_int_equal(longindex, 0);
    assert_int_equal(rpm_optind, 2);
    assert_int_equal(rpm_optopt, 0);
    assert_ptr_equal(rpm_optarg, NULL);

    result = rpm_getopt(argc, argv, "", long_opts, &longindex);
    assert_int_equal(result, -1);
    assert_int_equal(longindex, 0);
    assert_int_equal(rpm_optind, 2);
    assert_int_equal(rpm_optopt, 0);
    assert_ptr_equal(rpm_optarg, NULL);
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
    int longindex = -123;
    int result = rpm_getopt(argc, argv, "", long_opts, &longindex);
    assert_int_equal(result, 10);
    assert_int_equal(longindex, 0);
    assert_int_equal(rpm_optind, 2);
    assert_int_equal(rpm_optopt, 0);
    assert_ptr_equal(rpm_optarg, NULL);

    result = rpm_getopt(argc, argv, "", long_opts, &longindex);
    assert_int_equal(result, 11);
    assert_int_equal(longindex, 1);
    assert_int_equal(rpm_optind, 3);
    assert_int_equal(rpm_optopt, 0);
    assert_ptr_equal(rpm_optarg, NULL);

    result = rpm_getopt(argc, argv, "", long_opts, &longindex);
    assert_int_equal(result, 10);
    assert_int_equal(longindex, 0);
    assert_int_equal(rpm_optind, 4);
    assert_int_equal(rpm_optopt, 0);
    assert_ptr_equal(rpm_optarg, NULL);

    result = rpm_getopt(argc, argv, "", long_opts, &longindex);
    assert_int_equal(result, 10);
    assert_int_equal(longindex, 0);
    assert_int_equal(rpm_optind, 5);
    assert_int_equal(rpm_optopt, 0);
    assert_ptr_equal(rpm_optarg, NULL);

    result = rpm_getopt(argc, argv, "", long_opts, &longindex);
    assert_int_equal(result, '?');
    assert_int_equal(longindex, 0);
    assert_int_equal(rpm_optind, 6);
    assert_int_equal(rpm_optopt, '?');
    assert_string_equal(output, "prog: unknown option `--long4`\n");

    result = rpm_getopt(argc, argv, "", long_opts, &longindex);
    assert_int_equal(result, -1);
    assert_int_equal(longindex, 0);
    assert_int_equal(rpm_optind, 6);
    assert_int_equal(rpm_optopt, 0);
    assert_ptr_equal(rpm_optarg, NULL);
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
    int longindex = -123;
    int result = rpm_getopt(argc, argv, "", long_opts, &longindex);
    assert_int_equal(result, 11);
    assert_int_equal(longindex, 0);
    assert_int_equal(rpm_optind, 2);
    assert_int_equal(rpm_optopt, 0);
    assert_ptr_equal(rpm_optarg, NULL);

    result = rpm_getopt(argc, argv, "", long_opts, &longindex);
    assert_int_equal(result, 22);
    assert_int_equal(longindex, 1);
    assert_int_equal(rpm_optind, 3);
    assert_int_equal(rpm_optopt, 0);
    assert_ptr_equal(rpm_optarg, NULL);

    result = rpm_getopt(argc, argv, "", long_opts, &longindex);
    assert_int_equal(result, 1);
    assert_int_equal(longindex, 1);
    assert_int_equal(rpm_optind, 4);
    assert_int_equal(rpm_optopt, 1);
    assert_string_equal(rpm_optarg, "x");

    result = rpm_getopt(argc, argv, "", long_opts, &longindex);
    assert_int_equal(result, 1);
    assert_int_equal(longindex, 1);
    assert_int_equal(rpm_optind, 5);
    assert_int_equal(rpm_optopt, 1);
    assert_string_equal(rpm_optarg, "y");

    result = rpm_getopt(argc, argv, "", long_opts, &longindex);
    assert_int_equal(result, 1);
    assert_int_equal(longindex, 1);
    assert_int_equal(rpm_optind, 6);
    assert_int_equal(rpm_optopt, 1);
    assert_string_equal(rpm_optarg, "z");

    result = rpm_getopt(argc, argv, "", long_opts, &longindex);
    assert_int_equal(result, -1);
    assert_int_equal(longindex, 1);
    assert_int_equal(rpm_optind, 6);
    assert_int_equal(rpm_optopt, 0);
    assert_ptr_equal(rpm_optarg, NULL);
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
    int result = rpm_getopt(argc, argv, "", long_opts, NULL);
    assert_int_equal(result, 0);
    assert_int_equal(rpm_optind, 2);
    assert_int_equal(rpm_optopt, 0);
    assert_ptr_equal(rpm_optarg, NULL);
    assert_int_equal(flag, 11);

    result = rpm_getopt(argc, argv, "", long_opts, NULL);
    assert_int_equal(result, -1);
    assert_int_equal(rpm_optind, 2);
    assert_int_equal(rpm_optopt, 0);
    assert_ptr_equal(rpm_optarg, NULL);
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
    int result = rpm_getopt(argc, argv, "", long_opts, NULL);
    assert_int_equal(result, 11);
    assert_int_equal(rpm_optind, 3);
    assert_int_equal(rpm_optopt, 0);
    assert_string_equal(rpm_optarg, "arg");

    result = rpm_getopt(argc, argv, "", long_opts, NULL);
    assert_int_equal(result, '?');
    assert_int_equal(rpm_optind, 4);
    assert_int_equal(rpm_optopt, '?');
    assert_string_equal(output, "prog: unknown option `--long3`\n");

    result = rpm_getopt(argc, argv, "", long_opts, NULL);
    assert_int_equal(result, -1);
    assert_int_equal(rpm_optind, 4);
    assert_int_equal(rpm_optopt, 0);
    assert_ptr_equal(rpm_optarg, NULL);
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
    int result = rpm_getopt(argc, argv, "", long_opts, NULL);
    assert_int_equal(result, 11);
    assert_int_equal(rpm_optind, 2);
    assert_int_equal(rpm_optopt, 0);
    assert_ptr_equal(rpm_optarg, NULL);

    result = rpm_getopt(argc, argv, "", long_opts, NULL);
    assert_int_equal(result, 1);
    assert_int_equal(rpm_optind, 3);
    assert_int_equal(rpm_optopt, 1);
    assert_string_equal(rpm_optarg, "arg");

    result = rpm_getopt(argc, argv, "", long_opts, NULL);
    assert_int_equal(result, '?');
    assert_int_equal(rpm_optind, 4);
    assert_int_equal(rpm_optopt, '?');
    assert_string_equal(output, "prog: unknown option `--long3`\n");

    result = rpm_getopt(argc, argv, "", long_opts, NULL);
    assert_int_equal(result, -1);
    assert_int_equal(rpm_optind, 4);
    assert_int_equal(rpm_optopt, 0);
    assert_ptr_equal(rpm_optarg, NULL);
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
    int result = rpm_getopt(argc, argv, "", long_opts, NULL);
    assert_int_equal(result, 11);
    assert_int_equal(rpm_optind, 2);
    assert_int_equal(rpm_optopt, 0);
    assert_string_equal(rpm_optarg, "arg1");

    result = rpm_getopt(argc, argv, "", long_opts, NULL);
    assert_int_equal(result, 22);
    assert_int_equal(rpm_optind, 3);
    assert_int_equal(rpm_optopt, 0);
    assert_ptr_equal(rpm_optarg, NULL); // arg2 is ignored

    result = rpm_getopt(argc, argv, "", long_opts, NULL);
    assert_int_equal(result, -1);
    assert_int_equal(rpm_optind, 3);
    assert_int_equal(rpm_optopt, 0);
    assert_ptr_equal(rpm_optarg, NULL);
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
    int result = rpm_getopt(argc, argv, "", long_opts, NULL);
    assert_int_equal(result, 11);
    assert_int_equal(rpm_optind, 2);
    assert_int_equal(rpm_optopt, 0);
    assert_ptr_equal(rpm_optarg, NULL);

    result = rpm_getopt(argc, argv, "", long_opts, NULL);
    assert_int_equal(result, 22);
    assert_int_equal(rpm_optind, 3);
    assert_int_equal(rpm_optopt, 0);
    assert_string_equal(rpm_optarg, "arg");

    result = rpm_getopt(argc, argv, "", long_opts, NULL);
    assert_int_equal(result, -1);
    assert_int_equal(rpm_optind, 3);
    assert_int_equal(rpm_optopt, 0);
    assert_ptr_equal(rpm_optarg, NULL);
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

    int result = rpm_getopt(argc, argv, short_opts, NULL, NULL);
    assert_int_equal(result, 'a');
    assert_int_equal(rpm_optind, 2);
    assert_int_equal(rpm_optopt, 'a');
    assert_ptr_equal(rpm_optarg, NULL);

    result = rpm_getopt(argc, argv, short_opts, NULL, NULL);
    assert_int_equal(result, '#');
    assert_int_equal(rpm_optind, 4);
    assert_int_equal(rpm_optopt, '#');
    assert_string_equal(rpm_optarg, "6");

    result = rpm_getopt(argc, argv, short_opts, NULL, NULL);
    assert_int_equal(result, 'H');
    assert_int_equal(rpm_optind, 5);
    assert_int_equal(rpm_optopt, 'H');
    assert_string_equal(rpm_optarg, "1");

    result = rpm_getopt(argc, argv, short_opts, NULL, NULL);
    assert_int_equal(result, 'H');
    assert_int_equal(rpm_optind, 7);
    assert_int_equal(rpm_optopt, 'H');
    assert_string_equal(rpm_optarg, "2");

    result = rpm_getopt(argc, argv, short_opts, NULL, NULL);
    assert_int_equal(result, 'k');
    assert_int_equal(rpm_optind, 8);
    assert_int_equal(rpm_optopt, 'k');
    assert_string_equal(rpm_optarg, "a");

    result = rpm_getopt(argc, argv, short_opts, NULL, NULL);
    assert_int_equal(result, 'l');
    assert_int_equal(rpm_optind, 9);
    assert_int_equal(rpm_optopt, 'l');
    assert_ptr_equal(rpm_optarg, NULL);

    result = rpm_getopt(argc, argv, short_opts, NULL, NULL);
    assert_int_equal(result, 'l');
    assert_int_equal(rpm_optind, 10);
    assert_int_equal(rpm_optopt, 'l');
    assert_string_equal(rpm_optarg, "v");

    result = rpm_getopt(argc, argv, short_opts, NULL, NULL);
    assert_int_equal(result, 'x');
    assert_int_equal(rpm_optind, 12);
    assert_int_equal(rpm_optopt, 'x');
    assert_string_equal(rpm_optarg, "path=./foo");

    result = rpm_getopt(argc, argv, short_opts, NULL, NULL);
    assert_int_equal(result, 1);
    assert_int_equal(rpm_optind, 13);
    assert_int_equal(rpm_optopt, 1);
    assert_string_equal(rpm_optarg, "bar");

    result = rpm_getopt(argc, argv, short_opts, NULL, NULL);
    assert_int_equal(result, -1);
    assert_int_equal(rpm_optind, 13);
    assert_int_equal(rpm_optopt, 0);
    assert_ptr_equal(rpm_optarg, NULL);
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

    int result = rpm_getopt(argc, argv, short_opts, NULL, NULL);
    assert_int_equal(result, 'a');
    assert_int_equal(rpm_optind, 2);
    assert_int_equal(rpm_optopt, 'a');
    assert_ptr_equal(rpm_optarg, NULL);

    result = rpm_getopt(argc, argv, short_opts, NULL, NULL);
    assert_int_equal(result, '#');
    assert_int_equal(rpm_optind, 3);
    assert_int_equal(rpm_optopt, '#');
    assert_string_equal(rpm_optarg, "pound");

    result = rpm_getopt(argc, argv, short_opts, NULL, NULL);
    assert_int_equal(result, 'b');
    assert_int_equal(rpm_optind, 4);
    assert_int_equal(rpm_optopt, 'b');
    assert_ptr_equal(rpm_optarg, NULL);

    result = rpm_getopt(argc, argv, short_opts, NULL, NULL);
    assert_int_equal(result, 'c');
    assert_int_equal(rpm_optind, 5);
    assert_int_equal(rpm_optopt, 'c');
    assert_string_equal(rpm_optarg, "filename");

    result = rpm_getopt(argc, argv, short_opts, NULL, NULL);
    assert_int_equal(result, -1);
    assert_int_equal(rpm_optind, 5);
    assert_int_equal(rpm_optopt, 0);
    assert_ptr_equal(rpm_optarg, NULL);
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

    int result = rpm_getopt(argc, argv, short_opts, NULL, NULL);
    assert_int_equal(result, 'a');
    assert_int_equal(rpm_optind, 2);
    assert_int_equal(rpm_optopt, 'a');
    assert_ptr_equal(rpm_optarg, NULL);

    result = rpm_getopt(argc, argv, short_opts, NULL, NULL);
    assert_int_equal(result, '#');
    assert_int_equal(rpm_optind, 3);
    assert_int_equal(rpm_optopt, '#');
    assert_string_equal(rpm_optarg, "bx");

    result = rpm_getopt(argc, argv, short_opts, NULL, NULL);
    assert_int_equal(result, 'b');
    assert_int_equal(rpm_optind, 4);
    assert_int_equal(rpm_optopt, 'b');
    assert_ptr_equal(rpm_optarg, NULL);

    result = rpm_getopt(argc, argv, short_opts, NULL, NULL);
    assert_int_equal(result, 'c');
    assert_int_equal(rpm_optind, 5);
    assert_int_equal(rpm_optopt, 'c');
    assert_ptr_equal(rpm_optarg, NULL);

    result = rpm_getopt(argc, argv, short_opts, NULL, NULL);
    assert_int_equal(result, 'c');
    assert_int_equal(rpm_optind, 6);
    assert_int_equal(rpm_optopt, 'c');
    assert_string_equal(rpm_optarg, "foo");

    result = rpm_getopt(argc, argv, short_opts, NULL, NULL);
    assert_int_equal(result, 'd');
    assert_int_equal(rpm_optind, 7);
    assert_int_equal(rpm_optopt, 'd');
    assert_ptr_equal(rpm_optarg, NULL);

    result = rpm_getopt(argc, argv, short_opts, NULL, NULL);
    assert_int_equal(result, 1);
    assert_int_equal(rpm_optind, 8);
    assert_int_equal(rpm_optopt, 1);
    assert_string_equal(rpm_optarg, "bar");

    result = rpm_getopt(argc, argv, short_opts, NULL, NULL);
    assert_int_equal(result, -1);
    assert_int_equal(rpm_optind, 8);
    assert_int_equal(rpm_optopt, 0);
    assert_ptr_equal(rpm_optarg, NULL);
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

    int result = rpm_getopt(argc, argv, short_opts, NULL, NULL);
    assert_int_equal(result, 'a');
    assert_int_equal(rpm_optind, 1);
    assert_int_equal(rpm_optopt, 'a');
    assert_ptr_equal(rpm_optarg, NULL);

    result = rpm_getopt(argc, argv, short_opts, NULL, NULL);
    assert_int_equal(result, 'b');
    assert_int_equal(rpm_optind, 2);
    assert_int_equal(rpm_optopt, 'b');
    assert_ptr_equal(rpm_optarg, NULL);

    result = rpm_getopt(argc, argv, short_opts, NULL, NULL);
    assert_int_equal(result, 'a');
    assert_int_equal(rpm_optind, 3);
    assert_int_equal(rpm_optopt, 'a');
    assert_ptr_equal(rpm_optarg, NULL);

    result = rpm_getopt(argc, argv, short_opts, NULL, NULL);
    assert_int_equal(result, 'c');
    assert_int_equal(rpm_optind, 4);
    assert_int_equal(rpm_optopt, 'c');
    assert_string_equal(rpm_optarg, "x");

    result = rpm_getopt(argc, argv, short_opts, NULL, NULL);
    assert_int_equal(result, 'c');
    assert_int_equal(rpm_optind, 6);
    assert_int_equal(rpm_optopt, 'c');
    assert_string_equal(rpm_optarg, "y");

    result = rpm_getopt(argc, argv, short_opts, NULL, NULL);
    assert_int_equal(result, 'd');
    assert_int_equal(rpm_optind, 7);
    assert_int_equal(rpm_optopt, 'd');
    assert_ptr_equal(rpm_optarg, NULL);

    result = rpm_getopt(argc, argv, short_opts, NULL, NULL);
    assert_int_equal(result, 'd');
    assert_int_equal(rpm_optind, 8);
    assert_int_equal(rpm_optopt, 'd');
    assert_string_equal(rpm_optarg, "a");

    result = rpm_getopt(argc, argv, short_opts, NULL, NULL);
    assert_int_equal(result, -1);
    assert_int_equal(rpm_optind, 8);
    assert_int_equal(rpm_optopt, 0);
    assert_ptr_equal(rpm_optarg, NULL);
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

    int result = rpm_getopt(argc, argv, short_opts, NULL, NULL);
    assert_int_equal(result, '?');
    assert_int_equal(rpm_optind, 2);
    assert_int_equal(rpm_optopt, 'H');
    assert_string_equal(output, "prog: argument required for option `-H`\n");

    result = rpm_getopt(argc, argv, short_opts, NULL, NULL);
    assert_int_equal(result, -1);
    assert_int_equal(rpm_optind, 2);
    assert_int_equal(rpm_optopt, 0);
    assert_ptr_equal(rpm_optarg, NULL);
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

    int result = rpm_getopt(argc, argv, short_opts, NULL, NULL);
    assert_int_equal(result, ':');
    assert_int_equal(rpm_optind, 2);
    assert_int_equal(rpm_optopt, 'H');
    assert_string_equal(output, "prog: argument required for option `-H`\n");

    result = rpm_getopt(argc, argv, short_opts, NULL, NULL);
    assert_int_equal(result, -1);
    assert_int_equal(rpm_optind, 2);
    assert_int_equal(rpm_optopt, 0);
    assert_ptr_equal(rpm_optarg, NULL);
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

    int result = rpm_getopt(argc, argv, short_opts, NULL, NULL);
    assert_int_equal(result, '?');
    assert_int_equal(rpm_optind, 2);
    assert_int_equal(rpm_optopt, '?');
    assert_string_equal(output, "prog: unknown option `-x`\n");

    result = rpm_getopt(argc, argv, short_opts, NULL, NULL);
    assert_int_equal(result, -1);
    assert_int_equal(rpm_optind, 2);
    assert_int_equal(rpm_optopt, 0);
    assert_ptr_equal(rpm_optarg, NULL);
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
