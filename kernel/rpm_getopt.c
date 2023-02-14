//
// Read command line options.
// Based on public domain sources from Gregory Pietsch.
// See description at the end of file.
//
// Copyright (C) 1997 Gregory Pietsch
//
// This file and the accompanying getopt.h header file are hereby placed in the
// public domain without restrictions.  Just give the author credit, don't
// claim you wrote it or prevent anyone else from using it.
//
#include <rpm/api.h>
#include <stdlib.h>
#include <string.h>

//
// Globally-defined variables.
//
char *rpm_optarg = 0;
int rpm_optind = 0;
int rpm_opterr = 1;
int rpm_optopt = '?';

//
// Static variables.
//
static int rpm_optwhere = 0;

//
// Is this argv-element an option or the end of the option list?
//
static int is_option(char *argv_element)
{
    return ((argv_element == 0) || (argv_element[0] == '-'));
}

int rpm_getopt(int argc, char *const argv[], const char *shortopts,
               const struct rpm_option *longopts, int *longind)
{
    int optindex = 0;
    size_t match_chars = 0;
    char *possible_arg = 0;
    int longopt_match = -1;
    int has_arg = -1;
    char *cp = 0;
    int arg_next = 0;
    int initial_colon = 0;

    // First, deal with silly parameters and easy stuff.
    if (argc == 0 || argv == 0 || (shortopts == 0 && longopts == 0) || rpm_optind > argc) {
        return -1;
    }
    if (argv[rpm_optind] != 0 && strcmp(argv[rpm_optind], "--") == 0) {
        rpm_optind++;
        rpm_optopt = 0;
        rpm_optarg = NULL;
        return -1;
    }

    // If this is our first time through.
    if (rpm_optind == 0) {
        rpm_optind = 1;
        rpm_optwhere = 1;
    }

    // Note that leading ‘-’ or ‘+’ in the optstring is ignored.
    if (shortopts != 0 && (*shortopts == '-' || *shortopts == '+')) {
        shortopts++;
    }

    // Check for initial colon in shortopts.
    if (shortopts != 0 && *shortopts == ':') {
        ++shortopts;
        initial_colon = 1;
    }

    //
    // Find our next option, if we're at the beginning of one.
    //
    if (rpm_optwhere == 1) {
        if (!is_option(argv[rpm_optind])) {
            rpm_optarg = argv[rpm_optind++];
            return (rpm_optopt = 1);
        }
    }
    // End of option list?
    if (argv[rpm_optind] == 0) {
        rpm_optopt = 0;
        rpm_optarg = NULL;
        return -1;
    }

    // We've got an option, so parse it.

    // First, is it a long option?
    if (longopts != 0 &&
        (memcmp(argv[rpm_optind], "--", 2) == 0) &&
        rpm_optwhere == 1) {

        // Handle long options.
        if (memcmp(argv[rpm_optind], "--", 2) == 0) {
            rpm_optwhere = 2;
        }

        longopt_match = -1;
        possible_arg = strchr(argv[rpm_optind] + rpm_optwhere, '=');
        if (possible_arg == 0) {
            // No =, so next argv might be arg.
            match_chars = strlen(argv[rpm_optind]);
            possible_arg = argv[rpm_optind] + match_chars;
            match_chars = match_chars - rpm_optwhere;
        } else {
            match_chars = (possible_arg - argv[rpm_optind]) - rpm_optwhere;
        }

        for (optindex = 0; longopts[optindex].name != 0; ++optindex) {
            if (memcmp(argv[rpm_optind] + rpm_optwhere, longopts[optindex].name, match_chars) ==
                0) {
                // Do we have an exact match?
                if (match_chars == strlen(longopts[optindex].name)) {
                    longopt_match = optindex;
                    break;
                }

                // Do any characters match?
                if (longopt_match >= 0) {
                    // We have ambiguous options.
                    if (rpm_opterr) {
                        rpm_puts(argv[0]);
                        rpm_puts(": option `");
                        rpm_puts(argv[rpm_optind]);
                        rpm_puts("' is ambiguous (could be `--");
                        rpm_puts(longopts[longopt_match].name);
                        rpm_puts("' or `--");
                        rpm_puts(longopts[optindex].name);
                        rpm_puts("')\n");
                    }
                    return (rpm_optopt = '?');
                }

                longopt_match = optindex;
            }
        }
        if (longopt_match >= 0) {
            has_arg = longopts[longopt_match].has_arg;
        } else {
            // Couldn't find long option.
            if (rpm_opterr) {
                rpm_puts(argv[0]);
                rpm_puts(": unknown option `");
                rpm_puts(argv[rpm_optind]);
                rpm_puts("`\n");
            }
            rpm_optind++;
            return (rpm_optopt = '?');
        }
    }

    // If we didn't find a long option, is it a short option?
    if (longopt_match < 0 && shortopts != 0) {
        cp = strchr(shortopts, argv[rpm_optind][rpm_optwhere]);
        if (cp == 0) {
            // Couldn't find option in shortopts.
            if (rpm_opterr) {
                rpm_puts(argv[0]);
                rpm_puts(": unknown option `-");
                rpm_putchar(argv[rpm_optind][rpm_optwhere]);
                rpm_puts("`\n");
            }
            rpm_optwhere++;
            if (argv[rpm_optind][rpm_optwhere] == '\0') {
                rpm_optind++;
                rpm_optwhere = 1;
            }
            return (rpm_optopt = '?');
        }

        if (cp[1] == ':') {
            if (cp[2] == ':') {
                has_arg = RPM_OPTIONAL_ARG;
            } else {
                has_arg = RPM_REQUIRED_ARG;
            }
        } else {
            has_arg = RPM_NO_ARG;
        }

        possible_arg = argv[rpm_optind] + rpm_optwhere + 1;
        rpm_optopt = *cp;
    }

    // Get argument and reset rpm_optwhere.
    arg_next = 0;
    switch (has_arg) {
    case RPM_OPTIONAL_ARG:
        if (*possible_arg == '=') {
            possible_arg++;
        }
        rpm_optarg = (*possible_arg != '\0') ? possible_arg : 0;
        rpm_optwhere = 1;
        break;

    case RPM_REQUIRED_ARG:
        if (*possible_arg == '=') {
            possible_arg++;
        }
        if (*possible_arg != '\0') {
            rpm_optarg = possible_arg;
            rpm_optwhere = 1;
        } else if (rpm_optind + 1 >= argc) {
            if (rpm_opterr) {
                rpm_puts(argv[0]);
                rpm_puts(": argument required for option `-");
                if (longopt_match >= 0) {
                    rpm_putchar('-');
                    rpm_puts(longopts[longopt_match].name);
                    rpm_optopt = initial_colon ? ':' : '\?';
                } else {
                    rpm_putchar(*cp);
                    rpm_optopt = *cp;
                }
                rpm_puts("`\n");
            }
            rpm_optind++;
            return initial_colon ? ':' : '\?';
        } else {
            rpm_optarg = argv[rpm_optind + 1];
            arg_next = 1;
            rpm_optwhere = 1;
        }
        break;

    default: // shouldn't happen
    case RPM_NO_ARG:
        if (longopt_match < 0) {
            rpm_optwhere++;
            if (argv[rpm_optind][rpm_optwhere] == '\0')
                rpm_optwhere = 1;
        } else {
            rpm_optwhere = 1;
        }
        rpm_optarg = 0;
        break;
    }

    // do we have to modify rpm_optind?
    if (rpm_optwhere == 1) {
        rpm_optind = rpm_optind + 1 + arg_next;
    }

    // Finally return.
    if (longopt_match >= 0) {
        if (longind != 0) {
            *longind = longopt_match;
        }

        if (longopts[longopt_match].flag != 0) {
            *(longopts[longopt_match].flag) = longopts[longopt_match].val;
            return 0;
        } else {
            return longopts[longopt_match].val;
        }
    } else {
        return rpm_optopt;
    }
}

// The rpm_getopt() function parses the command line arguments.  Its arguments argc
// and argv are the argument count and array as passed to the main() function
// on program invocation.  The argument optstring is a list of available option
// characters.  If such a character is followed by a colon (`:'), the option
// takes an argument, which is placed in optarg.  If such a character is
// followed by two colons, the option takes an optional argument, which is
// placed in optarg.  If the option does not take an argument, optarg is NULL.
//
// The external variable optind is the index of the next array element of argv
// to be processed; it communicates from one call to the next which element to
// process.
//
// The rpm_getopt() function also accepts long options started
// by two dashes `--'. If these take values, it is either in the form
//
//      --arg=value
//
//  or
//
//      --arg value
//
// It takes the additional arguments longopts which is a pointer to the first
// element of an array of type struct rpm_option.  The last element of the array
// has to be filled with NULL for the name field.
//
// The longind pointer points to the index of the current long option relative
// to longopts if it is non-NULL.
//
// The rpm_getopt() function returns the option character if the option was found
// successfully, `:' if there was a missing parameter for one of the options,
// `?' for an unknown option character, and -1 for the end of the option list.
//
// The program should expect options and other argv-elements in any order and
// care about the ordering of the two. We describe each non-option argv-element
// as if it were the argument of an option with character code 1.
//
// The special argument `--' forces an end of option-scanning.
// Only `--' can cause rpm_getopt() to return -1 with optind != argc.
//
// 2012-08-26: Tried to make the error handling more sus4-like. The functions
// return a colon if rpm_getopt() and friends detect a missing argument and the
// first character of shortopts/optstring starts with a colon (`:'). If rpm_getopt()
// and friends detect a missing argument and shortopts/optstring does not start
// with a colon, the function returns a question mark (`?'). If it was a missing
// argument to a short option, optopt is set to the character in question. The
// colon goes after the ordering character (`+' or `-').
