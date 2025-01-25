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
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

ZRESULT zm_hex_to_nybble(char c1);

ZRESULT zm_nybble_to_hex(uint8_t nybble);

//
// *buf MUST have space for exactly two characters!
//
// Returns OK on success, or an error code.
// If an error occues, the buffer will be unchanged.
///
ZRESULT zm_byte_to_hex(uint8_t byte, uint8_t *buf);

ZRESULT zm_hex_to_byte(unsigned char c1, unsigned char c2);

#ifdef __cplusplus
}
#endif
