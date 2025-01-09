//
// Test dynamic loader.
//
#include <gtest/gtest.h>
#include <rpm/api.h>
#include <rpm/loader.h>
#include <rpm/elf.h>
#include <sys/syscall.h>

TEST(loader, elf_binary)
{
    dyn_object_t dynobj{};

    // Map ELF file into memory.
    bool load_status = dyn_load(&dynobj, "hello.elf");
    ASSERT_TRUE(load_status);

    dyn_unload(&dynobj);
}

TEST(loader, linked_symbols)
{
    dyn_object_t dynobj{};

    const int expect_num_links = 1;
    ASSERT_TRUE(dyn_load(&dynobj, "hello.elf"));
    ASSERT_EQ(dynobj.num_links, expect_num_links);

    // Get names of dynamically linked routines.
    const char *symbols[expect_num_links]{};
    dyn_get_symbols(&dynobj, symbols);
    ASSERT_STREQ(symbols[0], "rpm_puts");

    dyn_unload(&dynobj);
}

static void mock_puts(const char *message)
{
#if __ARM_ARCH_ISA_A64
    // Avoid calling libc here, otherwise the test may fail on arm64 machine.
    const char *end = message;
    while (*end)
        end++;

    register int x0 __asm__("x0") = 1; /* stdout */
    register const char *x1 __asm__("x1") = message;
    register size_t x2 __asm__("x2") = end - message;
    register int x8 __asm__("x8") = SYS_write;
    asm volatile("svc #0" : : "r" (x0), "r" (x1), "r" (x2), "r" (x8) : "memory");
#else
    fputs(message, stdout);
    fflush(stdout);
    // TODO: save output.
#endif
}

TEST(loader, run_puts)
{
    dyn_object_t dynobj{};
    ASSERT_TRUE(dyn_load(&dynobj, "hello.elf"));

    // Export dynamically linked routines.
    static dyn_linkmap_t linkmap[] = {
        { "", NULL },
        { "rpm_puts", (void*) mock_puts },
        {},
    };
    const char *argv[] = { "hello" };

    bool exec_status = dyn_execv(&dynobj, linkmap, 1, argv);
    ASSERT_TRUE(exec_status);
    ASSERT_EQ(dynobj.exit_code, 0);
    // TODO: check output.

    dyn_unload(&dynobj);
}
