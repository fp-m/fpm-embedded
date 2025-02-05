//
// Numeric-related routines for Zmodem implementation.
//
// Copyright (c) 2020 Ross Bamford
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
#include <stdbool.h>

#include "ztypes.h"
#include "znumbers.h"

ZRESULT zm_hex_to_nybble(char c1)
{
    switch (c1) {
    case '0':
        return 0x00;
    case '1':
        return 0x01;
    case '2':
        return 0x02;
    case '3':
        return 0x03;
    case '4':
        return 0x04;
    case '5':
        return 0x05;
    case '6':
        return 0x06;
    case '7':
        return 0x07;
    case '8':
        return 0x08;
    case '9':
        return 0x09;
    case 'a':
        return 0x0a;
    case 'b':
        return 0x0b;
    case 'c':
        return 0x0c;
    case 'd':
        return 0x0d;
    case 'e':
        return 0x0e;
    case 'f':
        return 0x0f;
    default:
        return BAD_DIGIT;
    }
}

ZRESULT zm_nybble_to_hex(uint8_t nybble)
{
    switch (nybble) {
    case 0x0:
        return '0';
    case 0x1:
        return '1';
    case 0x2:
        return '2';
    case 0x3:
        return '3';
    case 0x4:
        return '4';
    case 0x5:
        return '5';
    case 0x6:
        return '6';
    case 0x7:
        return '7';
    case 0x8:
        return '8';
    case 0x9:
        return '9';
    case 0xa:
        return 'a';
    case 0xb:
        return 'b';
    case 0xc:
        return 'c';
    case 0xd:
        return 'd';
    case 0xe:
        return 'e';
    case 0xf:
        return 'f';
    default:
        return OUT_OF_RANGE;
    }
}

ZRESULT zm_byte_to_hex(uint8_t byte, uint8_t *buf)
{
    uint16_t h1 = zm_nybble_to_hex(BMSN(byte));

    if (IS_ERROR(h1)) {
        return h1;
    } else {
        uint16_t h2 = zm_nybble_to_hex(BLSN(byte));

        if (IS_ERROR(h2)) {
            return h2;
        } else {
            *buf++ = h1;
            *buf = h2;

            return OK;
        }
    }
}

ZRESULT zm_hex_to_byte(unsigned char c1, unsigned char c2)
{
    uint16_t n1, n2;

    n1 = zm_hex_to_nybble(c1);
    n2 = zm_hex_to_nybble(c2);

    if (n1 == BAD_DIGIT || n2 == BAD_DIGIT) {
        DEBUGF("Got bad digit: [0x%02x, 0x%02x]\n", c1, c2);
        return BAD_DIGIT;
    } else {
        return NTOB(n1, n2);
    }
}
