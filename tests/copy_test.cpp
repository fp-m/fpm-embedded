//
// Test FatFS routines.
//
#include <gtest/gtest.h>
#include <rpm/fs.h>
#include <rpm/diskio.h>
#include <rpm/internal.h>
#include "util.h"

TEST(cmd, copy_file)
{
    disk_setup();
    write_file("foo.txt", "'Twas brillig, and the slithy toves");

    // Copy foo to bar.
    const char *argv[] = { "cp", "foo.txt", "bar.txt", nullptr };
    rpm_cmd_copy(3, (char**) argv);

    read_file("foo.txt", "'Twas brillig, and the slithy toves");
    read_file("bar.txt", "'Twas brillig, and the slithy toves");
}
