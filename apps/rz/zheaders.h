//
// Routines for working with Zmodem headers.
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

//
// One-shot calculate the CRC for a ZHDR and set the
// crc1 and crc2 fields appropriately.
//
void zm_calc_hdr_crc(ZHDR *hdr);

uint16_t zm_calc_data_crc(uint8_t *buf, uint16_t len);
uint32_t zm_calc_data_crc32(uint8_t *buf, uint16_t len);

//
// Converts ZHDR to wire-format hex header. Expects CRC is already
// computed. Result placed in the supplied buffer.
//
// The encoded header includes the 'B' header-type character and
// trailing CRLF, but does not include other Zmodem control
// characters (e.g. leading ZBUF/ZDLE etc).
//
// Returns actual used length (max 0xff bytes), or OUT_OF_SPACE
// if the supplied buffer is not large enough.
//
ZRESULT zm_to_hex_header(ZHDR *hdr, uint8_t *buf, int max_len);

ZRESULT zm_check_header_crc16(ZHDR *hdr, uint16_t crc);
ZRESULT zm_check_header_crc32(ZHDR *hdr, uint32_t crc);

#ifdef ZDEBUG
// this is wasteful, but only if debugging is on, so, y'know...
static const char *__hdrtypes[]
    __attribute__((unused)) = { "ZRQINIT", "ZRINIT", "ZSINIT",     "ZACK",     "ZFILE",
                                "ZSKIP",   "ZNAK",   "ZABORT",     "ZFIN",     "ZRPOS",
                                "ZDATA",   "ZEOF",   "ZERR",       "ZCRC",     "ZCHALLENGE",
                                "ZCOMPL",  "ZCAN",   "ZFREECOUNT", "ZCOMMAND", "ZSTDERR" };

#define DEBUG_DUMPHDR_F(hdr)                                     \
    DEBUGF("DEBUG: Header type [%s]:\n", __hdrtypes[hdr->type]); \
    DEBUGF("  type: 0x%02x\n", hdr->type);                       \
    DEBUGF("    f0: 0x%02x\n", hdr->flags.f0);                   \
    DEBUGF("    f1: 0x%02x\n", hdr->flags.f1);                   \
    DEBUGF("    f2: 0x%02x\n", hdr->flags.f2);                   \
    DEBUGF("    f3: 0x%02x\n", hdr->flags.f3);                   \
    DEBUGF("  crc1: 0x%02x\n", hdr->crc1);                       \
    DEBUGF("  crc2: 0x%02x\n", hdr->crc2);                       \
    DEBUGF("   RES: 0x%02x\n", hdr->PADDING);                    \
    DEBUGF("\n");

#define DEBUG_DUMPHDR_P(hdr)                                     \
    DEBUGF("DEBUG: Header type [%s]:\n", __hdrtypes[hdr->type]); \
    DEBUGF("  type: 0x%02x\n", hdr->type);                       \
    DEBUGF("    p0: 0x%02x\n", hdr->position.p0);                \
    DEBUGF("    p1: 0x%02x\n", hdr->position.p1);                \
    DEBUGF("    p2: 0x%02x\n", hdr->position.p2);                \
    DEBUGF("    p3: 0x%02x\n", hdr->position.p3);                \
    DEBUGF("  crc1: 0x%02x\n", hdr->crc1);                       \
    DEBUGF("  crc2: 0x%02x\n", hdr->crc2);                       \
    DEBUGF("   RES: 0x%02x\n", hdr->PADDING);                    \
    DEBUGF("\n");

#define DEBUG_DUMPHDR_R(hdr)                                          \
    DEBUGF("DEBUG: Header received  [%s]:\n", __hdrtypes[hdr->type]); \
    DEBUGF("  type: 0x%02x\n", hdr->type);                            \
    DEBUGF("    p0/f3: 0x%02x\n", hdr->position.p0);                  \
    DEBUGF("    p1/f2: 0x%02x\n", hdr->position.p1);                  \
    DEBUGF("    p2/f1: 0x%02x\n", hdr->position.p2);                  \
    DEBUGF("    p3/f0: 0x%02x\n", hdr->position.p3);                  \
    DEBUGF("  crc1: 0x%02x\n", hdr->crc1);                            \
    DEBUGF("  crc2: 0x%02x\n", hdr->crc2);                            \
    DEBUGF("  crc3: 0x%02x\n", hdr->crc3);                            \
    DEBUGF("  crc4: 0x%02x\n", hdr->crc4);                            \
    DEBUGF("   RES: 0x%02x\n", hdr->PADDING);                         \
    DEBUGF("\n");

#define DEBUG_DUMPHDR DEBUG_DUMPHDR_F
#else
#define DEBUG_DUMPHDR_F(hdr)
#define DEBUG_DUMPHDR_P(hdr)
#define DEBUG_DUMPHDR_R(hdr)
#define DEBUG_DUMPHDR(hdr)
#endif

#ifdef __cplusplus
}
#endif
