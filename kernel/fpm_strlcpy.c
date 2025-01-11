//
// Copyright (c) 1998 Todd C. Miller <Todd.Miller@courtesan.com>
// Copyright (c) 2023 Serge Vakulenko <serge.vakulenko@gmail.com>
//
// Permission to use, copy, modify, and distribute this software for any
// purpose with or without fee is hereby granted, provided that the above
// copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
// WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
// ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
// WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
// ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
// OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
//
#include <fpm/api.h>

//
// Decode UTF-8 string.
//
size_t fpm_strlcpy_from_utf8(uint16_t *dst, const char *src, size_t nitems)
{
    uint16_t *d = dst;
    const char *s = src;
    size_t n = nitems;

    // Copy as many bytes as will fit.
    if (n != 0) {
        while (--n != 0) {
            uint8_t c1 = *s++;
            if (c1 == '\0') {
                *d++ = 0;
                break;
            }

            // Decode utf-8 to unicode.
            if (! (c1 & 0x80)) {
                *d++ = c1;
            } else {
                // Read second byte.
                uint8_t c2 = *s++;
                if (! (c1 & 0x20)) {
                    *d++ = (c1 & 0x1f) << 6 | (c2 & 0x3f);
                } else {
                    // Read third byte.
                    uint8_t c3 = *s++;
                    *d++ = (c1 & 0x0f) << 12 | (c2 & 0x3f) << 6 | (c3 & 0x3f);
                }
            }
        }
    }

    // Not enough room in dst, add NUL and traverse rest of src.
    if (n == 0) {
        if (nitems != 0) {
            *d = '\0'; // NUL-terminate dst
        }
        while (*s++)
            ;
    }

    return (s - src - 1); // count does not include NUL
}

//
// Encode as UTF-8 string.
//
size_t fpm_strlcpy_to_utf8(char *dst, const uint16_t *src, size_t nitems)
{
    char *d = dst;
    const uint16_t *s = src;
    size_t n = nitems;

    // Copy as many bytes as will fit.
    if (n != 0) {
        while (--n != 0) {
            uint16_t ch = *s++;
            if (ch == '\0') {
                *d++ = 0;
                break;
            }

            // Convert to UTF-8 encoding.
            if (ch < 0x80) {
                *d++ = ch;
            } else if (ch < 0x800) {
                if (n < 2) {
                    *d++ = 0;
                    break;
                }
                *d++ = ch >> 6 | 0xc0;
                *d++ = (ch & 0x3f) | 0x80;
            } else {
                if (n < 3) {
                    *d++ = 0;
                    break;
                }
                *d++ = ch >> 12 | 0xe0;
                *d++ = ((ch >> 6) & 0x3f) | 0x80;
                *d++ = (ch & 0x3f) | 0x80;
            }
        }
    }

    // Not enough room in dst, add NUL and traverse rest of src.
    if (n == 0) {
        if (nitems != 0) {
            *d = '\0'; // NUL-terminate dst
        }
        while (*s++)
            ;
    }

    return (s - src - 1); // count does not include NUL
}

//
// Copy src to string dst of size nitems.  At most nitems-1 characters
// will be copied.  Always NUL terminates (unless nitems == 0).
// Returns strwlen(src); if retval >= nitems, truncation occurred.
//
size_t fpm_strlcpy_unicode(uint16_t *dst, const uint16_t *src, size_t nitems)
{
    uint16_t *d = dst;
    const uint16_t *s = src;
    size_t n = nitems;

    // Copy as many bytes as will fit.
    if (n != 0) {
        while (--n != 0) {
            if ((*d++ = *s++) == '\0') {
                break;
            }
        }
    }

    // Not enough room in dst, add NUL and traverse rest of src.
    if (n == 0) {
        if (nitems != 0) {
            *d = '\0'; // NUL-terminate dst
        }
        while (*s++)
            ;
    }

    return (s - src - 1); // count does not include NUL
}

//
// Copy src to string dst of size nitems.  At most nitems-1 characters
// will be copied.  Always NUL terminates (unless nitems == 0).
// Returns strlen(src); if retval >= nitems, truncation occurred.
//
size_t fpm_strlcpy(char *dst, const char *src, size_t nitems)
{
    char *d = dst;
    const char *s = src;
    size_t n = nitems;

    // Copy as many bytes as will fit.
    if (n != 0) {
        while (--n != 0) {
            if ((*d++ = *s++) == '\0') {
                break;
            }
        }
    }

    // Not enough room in dst, add NUL and traverse rest of src.
    if (n == 0) {
        if (nitems != 0) {
            *d = '\0'; // NUL-terminate dst
        }
        while (*s++)
            ;
    }

    return (s - src - 1); // count does not include NUL
}
