#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <cmocka.h>
#include <rpm/api.h>

enum { SUN = 0, MON, TUE, WED, THU, FRI, SAT };

static void arbitrary_wednesday(void **unused)
{
    // Wednesday, February 15, 2023 - when I wrote this test.
    int wday = rpm_get_dotw(2023, 2, 15);
    assert_int_equal(wday, WED);
}

static void space_x_endeavor(void **unused)
{
    // Saturday, May 30, 2020 - Space-X launches astronauts to space station.
    int wday = rpm_get_dotw(2020, 5, 30);
    assert_int_equal(wday, SAT);
}

static void war_on_japan(void **unused)
{
    // Monday, December 8, 1941 - U.S. and Britain declare war on Japan.
    int wday = rpm_get_dotw(1941, 12, 8);
    assert_int_equal(wday, MON);
}

static void emancipation_proclamation(void **unused)
{
    // Thursday, January 1, 1863 - Abraham Lincoln signs the Emancipation Proclamation ending slavery.
    int wday = rpm_get_dotw(1863, 1, 1);
    assert_int_equal(wday, THU);
}

static void u_s_capital(void **unused)
{
    // Friday, December 12, 1800 - Washington, D.C. is established as the U.S. capital.
    int wday = rpm_get_dotw(1800, 12, 12);
    assert_int_equal(wday, FRI);
}

//
// Run all tests.
//
int main()
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(arbitrary_wednesday),
        cmocka_unit_test(space_x_endeavor),
        cmocka_unit_test(war_on_japan),
        cmocka_unit_test(emancipation_proclamation),
        cmocka_unit_test(u_s_capital),
    };
    return cmocka_run_group_tests(tests, NULL, NULL);
}
