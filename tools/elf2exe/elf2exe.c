#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

static void usage()
{
    printf("Usage:\n");
    printf("    elf2exe [-v] executable.elf\n");
}

int main(int argc, char **argv)
{
    int verbose = 0;

    // Parse arguments.
    for (;;) {
        switch (getopt(argc, argv, "v")) {
        case EOF:
            break;
        case 'v':
            ++verbose;
            continue;
        default:
            fprintf(stderr, "Unrecognized option\n");
            usage();
            exit(EXIT_FAILURE);
        }
        break;
    }
    argc -= optind;
    argv += optind;
    if (argc < 1) {
        usage();
        exit(EXIT_SUCCESS);
    }
    if (argc > 1) {
        fprintf(stderr, "Too many arguments\n");
        usage();
        exit(EXIT_FAILURE);
    }

    //TODO
}
