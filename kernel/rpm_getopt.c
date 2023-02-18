//
// Read command line options.
// Based on public domain sources from Gregory Pietsch.
// See description at the end of file.
//
// Copyright (C) 1997 Gregory Pietsch
// Copyright (C) 2023 Serge Vakulenko
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
#include <rpm/api.h>
#include <rpm/getopt.h>
#include <stdlib.h>
#include <string.h>

int rpm_getopt(int argc, char *const argv[], const char *shortopts,
               const struct rpm_option *longopts, struct rpm_opt *opt)
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
    if (!opt) {
        return -1;
    }
    if (argc == 0 || argv == 0 || (shortopts == 0 && longopts == 0) || opt->ind > argc) {
        return (opt->ret = -1);
    }
    if (argv[opt->ind] != 0 && strcmp(argv[opt->ind], "--") == 0) {
        opt->ind++;
        opt->opt = 0;
        opt->arg = NULL;
        return (opt->ret = -1);
    }

    // If this is our first time through.
    if (opt->ind == 0) {
        opt->ind = 1;
        opt->where = 1;
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
    if (opt->where == 1 && argv[opt->ind] != 0) {
        if (argv[opt->ind][0] != '-' || argv[opt->ind][1] == '\0') {
            // Regular argument.
            // Treat single dash as argument.
            opt->arg = argv[opt->ind++];
            return (opt->ret = opt->opt = 1);
        }
    }

    // End of option list?
    if (argv[opt->ind] == 0) {
        opt->opt = 0;
        opt->arg = NULL;
        return (opt->ret = -1);
    }

    // We've got an option, so parse it.

    // First, is it a long option?
    if (longopts != 0 &&
        (memcmp(argv[opt->ind], "--", 2) == 0) &&
        opt->where == 1) {

        // Handle long options.
        if (memcmp(argv[opt->ind], "--", 2) == 0) {
            opt->where = 2;
        }

        longopt_match = -1;
        possible_arg = strchr(argv[opt->ind] + opt->where, '=');
        if (possible_arg == 0) {
            // No =, so next argv might be arg.
            match_chars = strlen(argv[opt->ind]);
            possible_arg = argv[opt->ind] + match_chars;
            match_chars = match_chars - opt->where;
        } else {
            match_chars = (possible_arg - argv[opt->ind]) - opt->where;
        }

        for (optindex = 0; longopts[optindex].name != 0; ++optindex) {
            if (memcmp(argv[opt->ind] + opt->where, longopts[optindex].name, match_chars) ==
                0) {
                // Do we have an exact match?
                if (match_chars == strlen(longopts[optindex].name)) {
                    longopt_match = optindex;
                    break;
                }

                // Do any characters match?
                if (longopt_match >= 0) {
                    // We have ambiguous options.
                    if (!opt->silent) {
                        rpm_puts(argv[0]);
                        rpm_puts(": Option `");
                        rpm_puts(argv[opt->ind]);
                        rpm_puts("' is ambiguous (could be `--");
                        rpm_puts(longopts[longopt_match].name);
                        rpm_puts("' or `--");
                        rpm_puts(longopts[optindex].name);
                        rpm_puts("')\r\n");
                    }
                    return (opt->ret = opt->opt = '?');
                }

                longopt_match = optindex;
            }
        }
        if (longopt_match >= 0) {
            has_arg = longopts[longopt_match].has_arg;
        } else {
            // Couldn't find long option.
            if (!opt->silent) {
                rpm_puts(argv[0]);
                rpm_puts(": Unknown option `");
                rpm_puts(argv[opt->ind]);
                rpm_puts("`\r\n");
            }
            opt->ind++;
            return (opt->ret = opt->opt = '?');
        }
    }

    // If we didn't find a long option, is it a short option?
    if (longopt_match < 0 && shortopts != 0) {
        cp = strchr(shortopts, argv[opt->ind][opt->where]);
        if (cp == 0) {
            // Couldn't find option in shortopts.
            if (!opt->silent) {
                rpm_puts(argv[0]);
                rpm_puts(": Unknown option `-");
                rpm_putchar(argv[opt->ind][opt->where]);
                rpm_puts("`\r\n");
            }
            opt->where++;
            if (argv[opt->ind][opt->where] == '\0') {
                opt->ind++;
                opt->where = 1;
            }
            return (opt->ret = opt->opt = '?');
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

        possible_arg = argv[opt->ind] + opt->where + 1;
        opt->opt = *cp;
    }

    // Get argument and reset opt->where.
    arg_next = 0;
    switch (has_arg) {
    case RPM_OPTIONAL_ARG:
        if (*possible_arg == '=') {
            possible_arg++;
        }
        opt->arg = (*possible_arg != '\0') ? possible_arg : 0;
        opt->where = 1;
        break;

    case RPM_REQUIRED_ARG:
        if (*possible_arg == '=') {
            possible_arg++;
        }
        if (*possible_arg != '\0') {
            opt->arg = possible_arg;
            opt->where = 1;
        } else if (opt->ind + 1 >= argc) {
            if (!opt->silent) {
                rpm_puts(argv[0]);
                rpm_puts(": Argument required for option `-");
                if (longopt_match >= 0) {
                    rpm_putchar('-');
                    rpm_puts(longopts[longopt_match].name);
                    opt->opt = initial_colon ? ':' : '\?';
                } else {
                    rpm_putchar(*cp);
                    opt->opt = *cp;
                }
                rpm_puts("`\r\n");
            }
            opt->ind++;
            return (opt->ret = initial_colon ? ':' : '\?');
        } else {
            opt->arg = argv[opt->ind + 1];
            arg_next = 1;
            opt->where = 1;
        }
        break;

    default: // shouldn't happen
    case RPM_NO_ARG:
        if (longopt_match < 0) {
            opt->where++;
            if (argv[opt->ind][opt->where] == '\0')
                opt->where = 1;
        } else {
            opt->where = 1;
        }
        opt->arg = 0;
        break;
    }

    // do we have to modify opt->ind?
    if (opt->where == 1) {
        opt->ind = opt->ind + 1 + arg_next;
    }

    // Finally return.
    if (longopt_match >= 0) {
        opt->long_index = longopt_match;

        if (longopts[longopt_match].flag != 0) {
            *(longopts[longopt_match].flag) = longopts[longopt_match].val;
            return (opt->ret = 0);
        } else {
            return (opt->ret = longopts[longopt_match].val);
        }
    } else {
        return (opt->ret = opt->opt);
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
// The long_index field shows the index of the current long option relative
// to longopts.
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
