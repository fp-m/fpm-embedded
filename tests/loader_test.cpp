//
// Test dynamic loader.
//
#include <gtest/gtest.h>
#include <fpm/api.h>
#include <fpm/loader.h>
#include <fpm/elf.h>
#include <sys/syscall.h>

TEST(loader, elf_binary)
{
    fpm_executable_t dynobj{};

    // Map ELF file into memory.
    bool load_status = fpm_load(&dynobj, "hello.elf");
    ASSERT_TRUE(load_status);

    fpm_unload(&dynobj);
}

TEST(loader, linked_symbols)
{
    fpm_executable_t dynobj{};

    const int expect_num_links = 1;
    ASSERT_TRUE(fpm_load(&dynobj, "hello.elf"));
    ASSERT_EQ(dynobj.num_links, expect_num_links);

    // Get names of dynamically linked routines.
    const char *symbols[expect_num_links]{};
    fpm_get_symbols(&dynobj, symbols);
    ASSERT_STREQ(symbols[0], "fpm_puts");

    fpm_unload(&dynobj);
}

static std::stringstream puts_result;

static void mock_puts(const char *message)
{
    fputs(message, stdout);
    fflush(stdout);

    // Save output.
    puts_result << message;
}

TEST(loader, run_puts)
{
    fpm_executable_t dynobj{};
    ASSERT_TRUE(fpm_load(&dynobj, "hello.elf"));

    // Export dynamically linked routines.
    static fpm_binding_t linkmap[] = {
        { "", NULL },
        { "fpm_puts", (void*) mock_puts },
        {},
    };
    char filename[] = { "hello" };
    char *argv[] = { filename };

    bool exec_status = fpm_execv(&dynobj, linkmap, 1, argv);
    ASSERT_TRUE(exec_status);
    ASSERT_EQ(dynobj.exit_code, 0);

    // Check output.
    ASSERT_EQ(puts_result.str(), "Hello, World!\r\n");

    fpm_unload(&dynobj);
}
