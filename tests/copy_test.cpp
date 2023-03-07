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
    const char *argv[] = { "cp", "-v", "foo.txt", "bar.txt", nullptr };
    rpm_cmd_copy(4, (char**) argv);

    read_file("foo.txt", "'Twas brillig, and the slithy toves");
    read_file("bar.txt", "'Twas brillig, and the slithy toves");
}

TEST(cmd, copy_recursive_existing_target)
{
    disk_setup();
    create_directory("a");
    create_directory("a/b");
    create_directory("x");
    write_file("a/b/c", "foobar");

    // Copy directory recursively to existing directory.
    const char *argv[] = { "cp", "-v", "-r", "a/b", "x", nullptr };
    rpm_cmd_copy(5, (char**) argv);

    check_directory("x/b");
    read_file("x/b/c", "foobar");
}

TEST(cmd, copy_recursive_nonexisting_target)
{
    disk_setup();
    create_directory("a");
    create_directory("a/b");
    create_directory("x");
    write_file("a/b/c", "foobar");

    // Copy directory recursively to non-existing directory.
    const char *argv[] = { "cp", "-v", "-r", "a/b", "y", nullptr };
    rpm_cmd_copy(5, (char**) argv);

    check_directory("y");
    read_file("y/c", "foobar");
}

TEST(cmd, copy_recursive_trailing_slash_existing_target)
{
    disk_setup();
    create_directory("a");
    create_directory("a/b");
    create_directory("x");
    write_file("a/b/c", "foobar");

    // Copy directory recursively to existing directory.
    // Note trailing slash in the source path.
    const char *argv[] = { "cp", "-v", "-r", "a/b/", "x", nullptr };
    rpm_cmd_copy(5, (char**) argv);

    read_file("x/c", "foobar");
}

TEST(cmd, copy_recursive_trailing_slash_nonexisting_target)
{
    disk_setup();
    create_directory("a");
    create_directory("a/b");
    create_directory("x");
    write_file("a/b/c", "foobar");

    // Copy directory recursively to non-existing directory.
    // Note trailing slash in the source path.
    const char *argv[] = { "cp", "-v", "-r", "a/b/", "y", nullptr };
    rpm_cmd_copy(5, (char**) argv);

    check_directory("y");
    read_file("y/c", "foobar");
}
