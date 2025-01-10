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
    rpm_executable_t dynobj{};

    // Map ELF file into memory.
    bool load_status = rpm_load(&dynobj, "hello.elf");
    ASSERT_TRUE(load_status);

    rpm_unload(&dynobj);
}

TEST(loader, linked_symbols)
{
    rpm_executable_t dynobj{};

    const int expect_num_links = 1;
    ASSERT_TRUE(rpm_load(&dynobj, "hello.elf"));
    ASSERT_EQ(dynobj.num_links, expect_num_links);

    // Get names of dynamically linked routines.
    const char *symbols[expect_num_links]{};
    rpm_get_symbols(&dynobj, symbols);
    ASSERT_STREQ(symbols[0], "rpm_puts");

    rpm_unload(&dynobj);
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
    rpm_executable_t dynobj{};
    ASSERT_TRUE(rpm_load(&dynobj, "hello.elf"));

    // Export dynamically linked routines.
    static rpm_binding_t linkmap[] = {
        { "", NULL },
        { "rpm_puts", (void*) mock_puts },
        {},
    };
    char filename[] = { "hello" };
    char *argv[] = { filename };

    bool exec_status = rpm_execv(&dynobj, linkmap, 1, argv);
    ASSERT_TRUE(exec_status);
    ASSERT_EQ(dynobj.exit_code, 0);

    // Check output.
    ASSERT_EQ(puts_result.str(), "Hello, World!\r\n");

    rpm_unload(&dynobj);
}
