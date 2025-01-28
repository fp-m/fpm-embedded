//
// Receive files with ZMODEM/YMODEM/XMODEM protocol.
//
// Copyright (c) 2025 Serge Vakulenko
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
#include <fpm/api.h>
#include <fpm/getopt.h>
#include <fpm/fs.h>
#include <alloca.h>

#include "zmodem.h"

//
// Implementation-defined receive character function.
//
ZRESULT zm_recv()
{
    // TODO: ignore ^C in FP/M somehow.
    return fpm_getchar();
}

//
// Implementation-defined send character function.
//
ZRESULT zm_send(uint8_t chr)
{
    fpm_putchar(chr);
    return OK;
}

int main(int argc, char **argv)
{
    static const struct fpm_option long_opts[] = {
        { "help", FPM_NO_ARG, NULL, 'h' },
        {},
    };
    struct fpm_opt opt = {};

    while (fpm_getopt(argc, argv, "h", long_opts, &opt) >= 0) {
        fpm_puts("Usage: rz\r\n");
        return 0;
    }

    // Invoked without arguments: receive file.
    zm_receive_file();
}
