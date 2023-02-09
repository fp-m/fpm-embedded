//
// Run shell on Linux or MacOS
//
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <rpm/internal.h>

static struct termios saved_term;

static void restore()
{
    printf("Restore terminal\r\n");
    tcsetattr(0, TCSADRAIN, &saved_term);
}

static void init()
{
    printf("Initialize terminal\n");
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
    // Switch stdin to the raw mode (no line buffering and editing).
    init();

    printf("Start shell on Unix\r\n");
    printf("Use '?' for help or 'exit' to quit.\r\n\r\n");

    // Start interactive dialog.
    rpm_shell();
}
