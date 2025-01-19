//
// Test dynamic loader.
//
#include <gtest/gtest.h>
#include <fpm/api.h>
#include <fpm/loader.h>
#include <fpm/context.h>
#include <fpm/elf.h>
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
    fpm_context_t ctx{};
    ASSERT_TRUE(fpm_load(&ctx, "testputs.exe"));

    // Export dynamically linked routines.
    static fpm_binding_t linkmap[] = {
        { "", NULL },
        { "fpm_puts", (void*) mock_puts },
        { "fpm_wputs", (void*) mock_wputs },
        { "fpm_print_version", (void*) mock_print_version },
        {},
    };
    char filename[] = { "hello" };
    char *argv[] = { filename };

    bool exec_status = fpm_invoke(&ctx, linkmap, 1, argv);

#if __APPLE__ && __x86_64__
    // Cannot set %gs register on MacOS.
    ASSERT_FALSE(exec_status);
#else
    ASSERT_TRUE(exec_status);
    ASSERT_EQ(ctx.exit_code, 0);

    // Check output.
    ASSERT_EQ(puts_result.str(),
        "Loader Test\r\n"
        "puts\r\n"
        "wputs\r\n"
    );
#endif

    fpm_unload(&ctx);
}
