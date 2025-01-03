//
// Test dynamic loader.
//
#include <gtest/gtest.h>
#include <rpm/api.h>
#include <rpm/loader.h>
#include <rpm/elf.h>

TEST(loader, elf_binary)
{
    const char filename[] = "hello.elf";
    dyn_object_t dynobj{};

    // Map ELF file into memory.
    bool load_status = dyn_load(&dynobj, filename);
    ASSERT_TRUE(load_status);

    dyn_unload(&dynobj);
}

TEST(loader, linked_symbols)
{
    const char filename[] = "hello.elf";
    dyn_object_t dynobj{};

    const int expect_num_links = 1;
    ASSERT_TRUE(dyn_load(&dynobj, filename));
    ASSERT_EQ(dynobj.num_links, expect_num_links);

    // Get names of dynamically linked routines.
    const char *symbols[expect_num_links]{};
    dyn_get_symbols(&dynobj, symbols);
    ASSERT_STREQ(symbols[0], "rpm_puts");

    dyn_unload(&dynobj);
}
