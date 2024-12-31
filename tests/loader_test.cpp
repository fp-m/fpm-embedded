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

    bool load_status = dyn_load(&dynobj, filename);
    ASSERT_TRUE(load_status);

    unsigned const expect_header_size = (sizeof(size_t) == 8) ? sizeof(Elf64_Ehdr) : sizeof(Elf32_Ehdr);
    ASSERT_EQ(dynobj.e_ehsize, expect_header_size);

    dyn_unload(&dynobj);
}
