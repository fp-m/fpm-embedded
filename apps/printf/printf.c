//
// Formatted output.
//

/*
 * Copyright (c) 1989, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */
#include <fpm/api.h>
#include <stdlib.h>

typedef struct {
    char **gargv;
} input_t;

static const char *Number = "+-.0123456789";

#define PF(f, func)                                         \
    {                                                       \
        if (fieldwidth)                                     \
            if (precision)                                  \
                fpm_printf(f, fieldwidth, precision, func); \
            else                                            \
                fpm_printf(f, fieldwidth, func);            \
        else if (precision)                                 \
            fpm_printf(f, precision, func);                 \
        else                                                \
            fpm_printf(f, func);                            \
    }

static char *mklong(const char *str, int ch, char buf[])
{
    int len;

    if (ch == 'X') /* XXX */
        ch = 'x';
    len = strlen(str) + 2;
    bcopy(str, buf, len - 3);
    buf[len - 3] = 'l';
    buf[len - 2] = ch;
    buf[len - 1] = '\0';
    return buf;
}

static void escape(char *fmt)
{
    char *store;
    int value, c;

    for (store = fmt; (c = *fmt); ++fmt, ++store) {
        if (c != '\\') {
            *store = c;
            continue;
        }
        switch (*++fmt) {
        case '\0': /* EOS, user error */
            *store = '\\';
            *++store = '\0';
            return;
        case '\\': /* backslash */
        case '\'': /* single quote */
            *store = *fmt;
            break;
        case 'a': /* bell/alert */
            *store = '\7';
            break;
        case 'b': /* backspace */
            *store = '\b';
            break;
        case 'f': /* form-feed */
            *store = '\f';
            break;
        case 'n': /* newline */
            *store = '\n';
            break;
        case 'r': /* carriage-return */
            *store = '\r';
            break;
        case 't': /* horizontal tab */
            *store = '\t';
            break;
        case 'v': /* vertical tab */
            *store = '\13';
            break;
            /* octal constant */
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
            for (c = 3, value = 0; c-- && *fmt >= '0' && *fmt <= '7'; ++fmt) {
                value <<= 3;
                value += *fmt - '0';
            }
            --fmt;
            *store = value;
            break;
        default:
            *store = *fmt;
            break;
        }
    }
    *store = '\0';
}

static int getchr(input_t *input)
{
    if (!*input->gargv)
        return '\0';
    return (int)**input->gargv++;
}

static char *getstr(input_t *input)
{
    if (!*input->gargv)
        return "";
    return *input->gargv++;
}

static int asciicode(input_t *input)
{
    int ch;

    ch = **input->gargv;
    if (ch == '\'' || ch == '"')
        ch = (*input->gargv)[1];
    ++input->gargv;
    return ch;
}

static int getlong(input_t *input, long *lp)
{
    if (!*input->gargv) {
        *lp = 0;
        return 0;
    }
    if (strchr(Number, **input->gargv)) {
        long val;
        char *ep;
        bool err = fpm_strtol(&val, *input->gargv, &ep, 0);
        if (*ep != '\0') {
            fpm_printf("%s: illegal number\n", *input->gargv);
            return 1;
        }
        if (err) {
            fpm_printf("%s: value out of bounds\n", *input->gargv);
            return 1;
        }

        *lp = val;
        ++input->gargv;
        return 0;
    }
    *lp = (long)asciicode(input);
    return 0;
}

static int getint(input_t *input, int *ip)
{
    long val;

    if (getlong(input, &val))
        return 1;
    if (val > 65535) {
        fpm_printf("%s: value out of bounds\n", *input->gargv);
        return 1;
    }
    *ip = val;
    return 0;
}

static double getdouble(input_t *input)
{
    if (!*input->gargv)
        return (double)0;
    if (strchr(Number, **input->gargv))
        return atof(*input->gargv++);
    return (double)asciicode(input);
}

static void usage()
{
    fpm_printf("Usage:\n");
    fpm_printf("    printf format [arg ...]\n");
}

int main(int argc, char **argv)
{
    static const char skip1[] = "#-+ 0";
    static const char skip2[] = "*0123456789";
    int end, fieldwidth, precision;
    char convch, nextch, *format, *start;
    char *fmt;
    input_t input;

    if (argc < 2) {
        usage();
        return 1;
    }

    /*
     * Basic algorithm is to scan the format string for conversion
     * specifications -- once one is found, find out if the field
     * width or precision is a '*'; if it is, gather up value.  Note,
     * format strings are reused as necessary to use up the provided
     * arguments, arguments of zero/null string are provided to use
     * up the format string.
     */
    escape(fmt = format = argv[1]); /* backslash interpretation */
    input.gargv = &argv[2];
    for (;;) {
        end = 0;
        /* find next format specification */
    next:
        for (start = fmt;; ++fmt) {
            if (!*fmt) {
                /* avoid infinite loop */
                if (end == 1) {
                    fpm_printf("missing format character\n", NULL, NULL);
                    return 1;
                }
                end = 1;
                if (fmt > start)
                    fpm_puts(start);
                if (!*input.gargv)
                    return 0;
                fmt = format;
                goto next;
            }
            /* %% prints a % */
            if (*fmt == '%') {
                if (*++fmt != '%')
                    break;
                *fmt++ = '\0';
                fpm_puts(start);
                goto next;
            }
        }

        /* skip to field width */
        for (; strchr(skip1, *fmt); ++fmt)
            ;
        if (*fmt == '*') {
            if (getint(&input, &fieldwidth))
                return 1;
        } else
            fieldwidth = 0;

        /* skip to possible '.', get following precision */
        for (; strchr(skip2, *fmt); ++fmt)
            ;
        if (*fmt == '.')
            ++fmt;
        if (*fmt == '*') {
            if (getint(&input, &precision))
                return 1;
        } else
            precision = 0;

        /* skip to conversion char */
        for (; strchr(skip2, *fmt); ++fmt)
            ;
        if (!*fmt) {
            fpm_printf("missing format character\n");
            return 1;
        }

        convch = *fmt;
        nextch = *++fmt;
        *fmt = '\0';
        switch (convch) {
        case 'c': {
            char p;

            p = getchr(&input);
            PF(start, p);
            break;
        }
        case 's': {
            char *p;

            p = getstr(&input);
            PF(start, p);
            break;
        }
        case 'd':
        case 'i':
        case 'o':
        case 'u':
        case 'x':
        case 'X': {
            char buf[64];
            long p;

            mklong(start, convch, buf);
            if (getlong(&input, &p))
                return 1;
            PF(buf, p);
            break;
        }
        case 'e':
        case 'E':
        case 'f':
        case 'g':
        case 'G': {
            double p;

            p = getdouble(&input);
            PF(start, p);
            break;
        }
        default:
            fpm_printf("illegal format character\n");
            return 1;
        }
        *fmt = nextch;
    }
    /* NOTREACHED */
}
