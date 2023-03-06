//
// Test FatFS routines.
//
#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <cmocka.h>
#include <rpm/fs.h>
#include <rpm/diskio.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <alloca.h>
#include "util.h"

static void copy_file(void **unused)
{
    disk_setup();
    write_file("foo.txt", "'Twas brillig, and the slithy toves");

    //TODO: copy foo to bar

    read_file("foo.txt", "'Twas brillig, and the slithy toves");
    read_file("bar.txt", "'Twas brillig, and the slithy toves");
}

//
// Run all tests.
//
int main()
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(copy_file),
    };
    return cmocka_run_group_tests(tests, NULL, NULL);
}
