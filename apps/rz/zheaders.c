/*
 *------------------------------------------------------------
 *                                  ___ ___ _
 *  ___ ___ ___ ___ ___       _____|  _| . | |_
 * |  _| . |_ -|  _| . |     |     | . | . | '_|
 * |_| |___|___|___|___|_____|_|_|_|___|___|_,_|
 *                     |_____|       firmware v1
 * ------------------------------------------------------------
 * Copyright (c)2020 Ross Bamford
 * See top-level LICENSE.md for licence information.
 *
 * Routines for working with Zmodem headers
 * ------------------------------------------------------------
 */
#include <string.h>
#include "crc.h"
#include "zheaders.h"
#include "znumbers.h"

void zm_calc_hdr_crc(ZHDR *hdr)
{
    uint16_t crc = update_crc16_ccitt(hdr->type, CRC_START_XMODEM);
    crc = update_crc16_ccitt(hdr->flags.f3, crc);
    crc = update_crc16_ccitt(hdr->flags.f2, crc);
    crc = update_crc16_ccitt(hdr->flags.f1, crc);
    crc = update_crc16_ccitt(hdr->flags.f0, crc);

    hdr->crc1 = CRC_MSB(crc);
    hdr->crc2 = CRC_LSB(crc);
}

uint16_t zm_calc_data_crc(uint8_t *buf, uint16_t len)
{
    uint16_t crc = CRC_START_XMODEM;

    for (int i = 0; i < len; i++) {
        crc = update_crc16_ccitt(buf[i], crc);
    }
    return crc;
}

//
// Returns CRC-32 of sequence of bytes (binary or ASCIIZ).
// Pass len of 0 to auto-determine ASCIIZ string length.
// or non-zero for arbitrary binary data/
//
uint32_t zm_calc_data_crc32(uint8_t *buf, uint16_t len)
{
    uint32_t crc = CRC_START_32;

    if (buf != 0) {
        if (len == 0) {
            len = strlen((const char *)buf);
        }
        for (unsigned l = 0; l < len; l++) {
            crc = update_crc32(buf[l], crc);
        }
    }
    return ~crc;
}

ZRESULT zm_to_hex_header(ZHDR *hdr, uint8_t *buf, int max_len)
{
    if (max_len < HEX_HDR_STR_LEN) {
        return OUT_OF_SPACE;
    } else {
        *buf++ = 'B'; // 01

        zm_byte_to_hex(hdr->type, buf); // 03
        buf += 2;
        zm_byte_to_hex(hdr->flags.f3, buf); // 05
        buf += 2;
        zm_byte_to_hex(hdr->flags.f2, buf); // 07
        buf += 2;
        zm_byte_to_hex(hdr->flags.f1, buf); // 09
        buf += 2;
        zm_byte_to_hex(hdr->flags.f0, buf); // 0b
        buf += 2;
        zm_byte_to_hex(hdr->crc1, buf); // 0d
        buf += 2;
        zm_byte_to_hex(hdr->crc2, buf); // 0f
        buf += 2;
        *buf++ = CR;        // 10
        *buf++ = LF | 0x80; // 11

        return HEX_HDR_STR_LEN;
    }
}

ZRESULT zm_check_header_crc16(ZHDR *hdr, uint16_t crc)
{
    if (CRC(hdr->crc1, hdr->crc2) == crc) {
        return OK;
    } else {
        return BAD_CRC;
    }
}

ZRESULT zm_check_header_crc32(ZHDR *hdr, uint32_t crc)
{
    if (CRC32(hdr->crc1, hdr->crc2, hdr->crc3, hdr->crc4) == crc) {
        return OK;
    } else {
        return BAD_CRC;
    }
}
