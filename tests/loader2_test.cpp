//
// Test dynamic loader.
//
#include <gtest/gtest.h>
#include <rpm/api.h>
#include <rpm/loader.h>
#include <rpm/elf.h>
#include <sys/syscall.h>

static std::stringstream puts_result;

static void mock_puts(const char *message)
{
    fputs(message, stdout);
    fflush(stdout);

    // Save output.
    puts_result << message;
}

static void mock_wputs(const uint16_t *message)
{
    for (;;) {
        unsigned ch = *message++;
        if (!ch)
            break;
        putchar(ch);
        puts_result << (char) ch;
    }
    fflush(stdout);
}

static void mock_print_version()
{
    mock_puts("Loader Test\r\n");
}

TEST(loader, print_version_puts_wputs)
{
    dyn_object_t dynobj{};
    ASSERT_TRUE(dyn_load(&dynobj, "testputs.elf"));

    // Export dynamically linked routines.
    static dyn_linkmap_t linkmap[] = {
        { "", NULL },
        { "rpm_puts", (void*) mock_puts },
        { "rpm_wputs", (void*) mock_wputs },
        { "rpm_print_version", (void*) mock_print_version },
        {},
    };
    const char *argv[] = { "hello" };

    bool exec_status = dyn_execv(&dynobj, linkmap, 1, argv);
    ASSERT_TRUE(exec_status);
    ASSERT_EQ(dynobj.exit_code, 0);

    // Check output.
    ASSERT_EQ(puts_result.str(),
        "Loader Test\r\n"
        "puts\r\n"
        "wputs\r\n"
    );

    dyn_unload(&dynobj);
}
