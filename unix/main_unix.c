//
// Run shell on Linux or MacOS
//
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <fpm/api.h>
#include <fpm/internal.h>
#include <fpm/context.h>
#include <fpm/diskio.h>
#include <fpm/fs.h>

static uint8_t core_memory[1024*1024];

static struct termios saved_term;

static void restore()
{
    //printf("Restore terminal\r\n");
    tcsetattr(0, TCSADRAIN, &saved_term);
}

static void init()
{
    //printf("Initialize terminal\n");
    if (tcgetattr(0, &saved_term) < 0) {
        printf("No terminal on stdin\n");
        exit(1);
    }
    atexit(restore);

    struct termios term = saved_term;
    term.c_iflag &= ~(ICRNL | INLCR | IXON | IXOFF);
    term.c_oflag &= ~(OPOST);
    term.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
    term.c_cc[VMIN] = 1;
    term.c_cc[VTIME] = 0;
    if (tcsetattr(0, TCSADRAIN, &term) < 0) {
        printf("Cannot initialize terminal\n");
        exit(1);
    }
}

int main()
{
    // Setup heap area.
    fpm_context_t context_base;
    fpm_heap_init(&context_base, (size_t)&core_memory[0], sizeof(core_memory));

    // Switch stdin to the raw mode (no line buffering and editing).
    init();
    disk_setup();
    f_mount("flash:");
    f_mount("sd:");

    printf("Start FP/M on Unix\r\n");
    printf("Use '?' for help or 'exit' to quit.\r\n\r\n");

    // Start interactive dialog.
    fpm_shell();
}
