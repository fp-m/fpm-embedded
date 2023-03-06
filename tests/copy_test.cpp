//
// Test FatFS routines.
//
#include <gtest/gtest.h>
#include <rpm/fs.h>
#include <rpm/diskio.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <alloca.h>
#include "util.h"

TEST(cmd, copy_file)
{
    disk_setup();
    write_file("foo.txt", "'Twas brillig, and the slithy toves");

    //TODO: copy foo to bar

    read_file("foo.txt", "'Twas brillig, and the slithy toves");
    read_file("bar.txt", "'Twas brillig, and the slithy toves");
}
