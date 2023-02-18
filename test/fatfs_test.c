//
// Test FatFS routines.
//
#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <cmocka.h>
#include <rpm/fs.h>

//
// Get date and time (local).
//
void rpm_get_datetime(int *year, int *month, int *day, int *dotw, int *hour, int *min, int *sec)
{
    *year = 2023;
    *month = 2;
    *day = 18;
    *dotw = 6; // Sunday is 0
    *hour = 15;
    *min = 33;
    *sec = 45;
}

static void mkfs(void **unused)
{
    //TODO: assert_int_equal(wday, WED);
}

//
// Run all tests.
//
int main()
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(mkfs),
    };
    return cmocka_run_group_tests(tests, NULL, NULL);
}
