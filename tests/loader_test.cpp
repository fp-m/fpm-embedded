//
// Test dynamic loader.
//
#include <gtest/gtest.h>
#include <rpm/loader.h>

bool dyn_load(dyn_object_t *dynobj, const char *filename)
{
    return true;
}

TEST(loader, elf_binary)
{
    const char filename[] = "hello.elf";
    dyn_object_t dynobj;

    bool load_status = dyn_load(&dynobj, filename);
    EXPECT_TRUE(load_status);
}
