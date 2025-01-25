//
// Generic serial routines for Zmodem.
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
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint8_t in_32bit_block;
} zmodem_t;

#define NONCONTROL(c) ((bool)((uint8_t)(c & 0xe0)))

//
// The lib doesn't implement these - they need to be provided.
//
ZRESULT zm_recv();
ZRESULT zm_send(uint8_t chr);

//
// Receive CR/LF (with CR being optional).
//
ZRESULT zm_read_crlf();

//
// Read two ASCII characters and convert them from hex.
//
ZRESULT zm_read_hex_byte();

//
// Read character, taking care of ZMODEM Data Link Escapes (ZDLE)
// and swallowing XON/XOFF.
//
ZRESULT zm_read_escaped();

ZRESULT zm_await_zdle();
ZRESULT zm_await_header(zmodem_t *z, ZHDR *hdr);

ZRESULT zm_read_hex_header(zmodem_t *z, ZHDR *hdr);
ZRESULT zm_read_binary16_header(zmodem_t *z, ZHDR *hdr);
ZRESULT zm_read_binary32_header(zmodem_t *z, ZHDR *hdr);

//
// len specifies the maximum length to read on entry,
// and contains actual length on return.
//
ZRESULT zm_read_data_block(zmodem_t *z, uint8_t *buf, uint16_t *len);

//
// Send a null-terminated string.
//
ZRESULT zm_send_sz(uint8_t *data);

//
// Send the given header as hex, with ZPAD/ZDLE preamble.
//
ZRESULT zm_send_hex_hdr(ZHDR *hdr);

//
// Convenience function to build and send a position header as hex.
//
ZRESULT zm_send_pos_hdr(uint8_t type, uint32_t pos);

//
// Convenience function to build and send a flags header as hex.
//
ZRESULT zm_send_flags_hdr(uint8_t type, uint8_t f0, uint8_t f1, uint8_t f2, uint8_t f3);

#ifdef __cplusplus
}
#endif
