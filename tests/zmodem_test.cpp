//
// Test Zmodem implementation.
//
// Copyright (c) 2020 Ross Bamford
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
#include <gtest/gtest.h>
#include <fpm/fs.h>
#include "zmodem.h"
#include "interact.h"
#include "util.h"

std::istream *zm_input;
std::ostream *zm_output;
std::unique_ptr<std::stringstream> zm_input_buffer;

// Set up the fake receive buffer for use in tests
void set_input(const char *data, int len)
{
    zm_input_buffer = std::make_unique<std::stringstream>();
    zm_input_buffer->write(data, len);
    zm_input = zm_input_buffer.get();
}

// recv implementation for use in tests
ZRESULT zm_recv()
{
    int ch = zm_input->get();
    if (ch < 0) {
        return CLOSED;
    }
    return ch;
}

// send implementation for use in tests
ZRESULT zm_send(uint8_t c)
{
    if (zm_output != nullptr) {
        zm_output->put(c);
    }
    return OK;
}

extern "C" {
void fpm_delay_msec(unsigned milliseconds)
{
    // Empty.
}

void fpm_puts(const char *input)
{
    fputs(input, stdout);
    fflush(stdout);
}
};

// Tests of the tests
TEST(rz, recv_buffer)
{
    set_input("abc", 3);

    EXPECT_EQ(zm_recv(), 'a');
    EXPECT_EQ(zm_recv(), 'b');
    EXPECT_EQ(zm_recv(), 'c');

    EXPECT_EQ(zm_recv(), CLOSED);
    EXPECT_EQ(zm_recv(), CLOSED);
}

// The actual tests
TEST(rz, is_error)
{
    EXPECT_FALSE(IS_ERROR(0x0000));
    EXPECT_FALSE(IS_ERROR(0x0001));
    EXPECT_FALSE(IS_ERROR(0x00ff));
    EXPECT_FALSE(IS_ERROR(0x0f00));
    EXPECT_TRUE(IS_ERROR(0xf000));
    EXPECT_TRUE(IS_ERROR(0xff00));

    EXPECT_FALSE(IS_ERROR(OK));
    EXPECT_TRUE(IS_ERROR(BAD_DIGIT));
}

TEST(rz, get_error_code)
{
    EXPECT_EQ(ERROR_CODE(0x0000), 0x0000);
    EXPECT_EQ(ERROR_CODE(0x0001), 0x0000);
    EXPECT_EQ(ERROR_CODE(0x00f0), 0x0000);
    EXPECT_EQ(ERROR_CODE(0x00ff), 0x0000);
    EXPECT_EQ(ERROR_CODE(0x0f00), 0x0000);
    EXPECT_EQ(ERROR_CODE(0xf000), 0xf000);
    EXPECT_EQ(ERROR_CODE(0xff00), 0xf000);
    EXPECT_EQ(ERROR_CODE(0xffc0), 0xf000);
}

TEST(rz, zvalue)
{
    EXPECT_EQ(ZVALUE(0x0000), 0x00);
    EXPECT_EQ(ZVALUE(0x0001), 0x01);
    EXPECT_EQ(ZVALUE(0x1000), 0x00);
    EXPECT_EQ(ZVALUE(0xf00d), 0x0d);

    EXPECT_EQ(ZVALUE(GOT_CRCE), ZCRCE);
    EXPECT_EQ(ZVALUE(GOT_CRCG), ZCRCG);
    EXPECT_EQ(ZVALUE(GOT_CRCQ), ZCRCQ);
    EXPECT_EQ(ZVALUE(GOT_CRCW), ZCRCW);
}

TEST(rz, noncontrol)
{
    for (uint8_t i = 0; i < 0xff; i++) {
        if (i < 32) {
            EXPECT_FALSE(NONCONTROL(i));
        } else {
            EXPECT_TRUE(NONCONTROL(i));
        }
    }
}

TEST(rz, is_fin)
{
    EXPECT_FALSE(IS_FIN(OK));
    EXPECT_FALSE(IS_FIN(BAD_DIGIT));

    EXPECT_TRUE(IS_FIN(GOT_CRCE));
    EXPECT_TRUE(IS_FIN(GOT_CRCG));
    EXPECT_TRUE(IS_FIN(GOT_CRCQ));
    EXPECT_TRUE(IS_FIN(GOT_CRCW));
}

TEST(rz, hex_to_nybble)
{
    EXPECT_EQ(zm_hex_to_nybble('0'), 0x0);
    EXPECT_EQ(ERROR_CODE(zm_hex_to_nybble('0')), 0);
    EXPECT_EQ(zm_hex_to_nybble('1'), 0x1);
    EXPECT_EQ(ERROR_CODE(zm_hex_to_nybble('1')), 0);
    EXPECT_EQ(zm_hex_to_nybble('2'), 0x2);
    EXPECT_EQ(ERROR_CODE(zm_hex_to_nybble('2')), 0);
    EXPECT_EQ(zm_hex_to_nybble('3'), 0x3);
    EXPECT_EQ(ERROR_CODE(zm_hex_to_nybble('3')), 0);
    EXPECT_EQ(zm_hex_to_nybble('4'), 0x4);
    EXPECT_EQ(ERROR_CODE(zm_hex_to_nybble('4')), 0);
    EXPECT_EQ(zm_hex_to_nybble('5'), 0x5);
    EXPECT_EQ(ERROR_CODE(zm_hex_to_nybble('5')), 0);
    EXPECT_EQ(zm_hex_to_nybble('6'), 0x6);
    EXPECT_EQ(ERROR_CODE(zm_hex_to_nybble('6')), 0);
    EXPECT_EQ(zm_hex_to_nybble('7'), 0x7);
    EXPECT_EQ(ERROR_CODE(zm_hex_to_nybble('7')), 0);
    EXPECT_EQ(zm_hex_to_nybble('8'), 0x8);
    EXPECT_EQ(ERROR_CODE(zm_hex_to_nybble('8')), 0);
    EXPECT_EQ(zm_hex_to_nybble('9'), 0x9);
    EXPECT_EQ(ERROR_CODE(zm_hex_to_nybble('9')), 0);
    EXPECT_EQ(zm_hex_to_nybble('a'), 0xa);
    EXPECT_EQ(ERROR_CODE(zm_hex_to_nybble('a')), 0);
    EXPECT_EQ(zm_hex_to_nybble('b'), 0xb);
    EXPECT_EQ(ERROR_CODE(zm_hex_to_nybble('b')), 0);
    EXPECT_EQ(zm_hex_to_nybble('c'), 0xc);
    EXPECT_EQ(ERROR_CODE(zm_hex_to_nybble('c')), 0);
    EXPECT_EQ(zm_hex_to_nybble('d'), 0xd);
    EXPECT_EQ(ERROR_CODE(zm_hex_to_nybble('d')), 0);
    EXPECT_EQ(zm_hex_to_nybble('e'), 0xe);
    EXPECT_EQ(ERROR_CODE(zm_hex_to_nybble('e')), 0);
    EXPECT_EQ(zm_hex_to_nybble('f'), 0xf);
    EXPECT_EQ(ERROR_CODE(zm_hex_to_nybble('f')), 0);

    EXPECT_EQ(ERROR_CODE(zm_hex_to_nybble('A')), BAD_DIGIT);
}

TEST(rz, hex_to_byte)
{
    EXPECT_EQ(zm_hex_to_byte('0', '0'), 0x00);
    EXPECT_EQ(ERROR_CODE(zm_hex_to_byte('0', '0')), 0);
    EXPECT_EQ(zm_hex_to_byte('0', '1'), 0x01);
    EXPECT_EQ(ERROR_CODE(zm_hex_to_byte('0', '1')), 0);
    EXPECT_EQ(zm_hex_to_byte('0', 'e'), 0x0e);
    EXPECT_EQ(ERROR_CODE(zm_hex_to_byte('0', 'e')), 0);
    EXPECT_EQ(zm_hex_to_byte('0', 'f'), 0x0f);
    EXPECT_EQ(ERROR_CODE(zm_hex_to_byte('0', 'f')), 0);
    EXPECT_EQ(zm_hex_to_byte('1', '0'), 0x10);
    EXPECT_EQ(ERROR_CODE(zm_hex_to_byte('1', '0')), 0);
    EXPECT_EQ(zm_hex_to_byte('1', '1'), 0x11);
    EXPECT_EQ(ERROR_CODE(zm_hex_to_byte('1', '1')), 0);
    EXPECT_EQ(zm_hex_to_byte('1', 'e'), 0x1e);
    EXPECT_EQ(ERROR_CODE(zm_hex_to_byte('1', 'e')), 0);
    EXPECT_EQ(zm_hex_to_byte('1', 'f'), 0x1f);
    EXPECT_EQ(ERROR_CODE(zm_hex_to_byte('1', 'f')), 0);
    EXPECT_EQ(zm_hex_to_byte('f', '0'), 0xf0);
    EXPECT_EQ(ERROR_CODE(zm_hex_to_byte('f', '0')), 0);
    EXPECT_EQ(zm_hex_to_byte('f', '1'), 0xf1);
    EXPECT_EQ(ERROR_CODE(zm_hex_to_byte('f', '1')), 0);
    EXPECT_EQ(zm_hex_to_byte('f', 'e'), 0xfe);
    EXPECT_EQ(ERROR_CODE(zm_hex_to_byte('f', 'e')), 0);
    EXPECT_EQ(zm_hex_to_byte('f', 'f'), 0xff);
    EXPECT_EQ(ERROR_CODE(zm_hex_to_byte('f', 'f')), 0);

    EXPECT_EQ(ERROR_CODE(zm_hex_to_byte('0', 'A')), BAD_DIGIT);
    EXPECT_EQ(ERROR_CODE(zm_hex_to_byte('A', '0')), BAD_DIGIT);
    EXPECT_EQ(ERROR_CODE(zm_hex_to_byte('A', 'A')), BAD_DIGIT);
    EXPECT_EQ(ERROR_CODE(zm_hex_to_byte('N', 'O')), BAD_DIGIT);
}

TEST(rz, nybble_to_hex)
{
    EXPECT_EQ(zm_nybble_to_hex(0x0), '0');
    EXPECT_EQ(zm_nybble_to_hex(0x1), '1');
    EXPECT_EQ(zm_nybble_to_hex(0x2), '2');
    EXPECT_EQ(zm_nybble_to_hex(0x3), '3');
    EXPECT_EQ(zm_nybble_to_hex(0x4), '4');
    EXPECT_EQ(zm_nybble_to_hex(0x5), '5');
    EXPECT_EQ(zm_nybble_to_hex(0x6), '6');
    EXPECT_EQ(zm_nybble_to_hex(0x7), '7');
    EXPECT_EQ(zm_nybble_to_hex(0x8), '8');
    EXPECT_EQ(zm_nybble_to_hex(0x9), '9');
    EXPECT_EQ(zm_nybble_to_hex(0xa), 'a');
    EXPECT_EQ(zm_nybble_to_hex(0xb), 'b');
    EXPECT_EQ(zm_nybble_to_hex(0xc), 'c');
    EXPECT_EQ(zm_nybble_to_hex(0xd), 'd');
    EXPECT_EQ(zm_nybble_to_hex(0xe), 'e');
    EXPECT_EQ(zm_nybble_to_hex(0xf), 'f');

    EXPECT_EQ(IS_ERROR(zm_nybble_to_hex(0x10)), true);
    EXPECT_EQ(IS_ERROR(zm_nybble_to_hex(0xFF)), true);
}

TEST(rz, byte_to_hex)
{
    uint8_t buf[2];

    EXPECT_EQ(zm_byte_to_hex(0x00, buf), OK);
    EXPECT_EQ(buf[0], '0');
    EXPECT_EQ(buf[1], '0');

    EXPECT_EQ(zm_byte_to_hex(0x01, buf), OK);
    EXPECT_EQ(buf[0], '0');
    EXPECT_EQ(buf[1], '1');

    EXPECT_EQ(zm_byte_to_hex(0x0f, buf), OK);
    EXPECT_EQ(buf[0], '0');
    EXPECT_EQ(buf[1], 'f');

    EXPECT_EQ(zm_byte_to_hex(0x10, buf), OK);
    EXPECT_EQ(buf[0], '1');
    EXPECT_EQ(buf[1], '0');

    EXPECT_EQ(zm_byte_to_hex(0xff, buf), OK);
    EXPECT_EQ(buf[0], 'f');
    EXPECT_EQ(buf[1], 'f');
}

TEST(rz, read_hex_header)
{
    zmodem_t z{};
    ZHDR hdr;

    // All zeros - CRC is zero
    set_input("00000000000000", 14);
    EXPECT_EQ(zm_read_hex_header(&z, &hdr), OK);

    EXPECT_EQ(hdr.type, 0);
    EXPECT_EQ(hdr.flags.f0, 0);
    EXPECT_EQ(hdr.flags.f1, 0);
    EXPECT_EQ(hdr.flags.f2, 0);
    EXPECT_EQ(hdr.flags.f3, 0);
    EXPECT_EQ(hdr.crc1, 0);
    EXPECT_EQ(hdr.crc2, 0);

    // Correct CRC - 01 02 03 04 05 - CRC is 0x8208
    set_input("01020304058208", 14);
    EXPECT_EQ(zm_read_hex_header(&z, &hdr), OK);

    EXPECT_EQ(hdr.type, 0x01);
    EXPECT_EQ(hdr.position.p0, 0x02);
    EXPECT_EQ(hdr.position.p1, 0x03);
    EXPECT_EQ(hdr.position.p2, 0x04);
    EXPECT_EQ(hdr.position.p3, 0x05);
    EXPECT_EQ(hdr.flags.f3, 0x02);
    EXPECT_EQ(hdr.flags.f2, 0x03);
    EXPECT_EQ(hdr.flags.f1, 0x04);
    EXPECT_EQ(hdr.flags.f0, 0x05);
    EXPECT_EQ(hdr.crc1, 0x82);
    EXPECT_EQ(hdr.crc2, 0x08);

    // Incorrect CRC - 01 02 03 04 05 - CRC is 0x8208, but expect 0xc0c0
    // Note that header left intact for debugging
    set_input("0102030405c0c0", 14);
    EXPECT_EQ(zm_read_hex_header(&z, &hdr), BAD_CRC);

    EXPECT_EQ(hdr.type, 0x01);
    EXPECT_EQ(hdr.position.p0, 0x02);
    EXPECT_EQ(hdr.position.p1, 0x03);
    EXPECT_EQ(hdr.position.p2, 0x04);
    EXPECT_EQ(hdr.position.p3, 0x05);
    EXPECT_EQ(hdr.flags.f3, 0x02);
    EXPECT_EQ(hdr.flags.f2, 0x03);
    EXPECT_EQ(hdr.flags.f1, 0x04);
    EXPECT_EQ(hdr.flags.f0, 0x05);
    EXPECT_EQ(hdr.crc1, 0xc0);
    EXPECT_EQ(hdr.crc2, 0xc0);

    // Invalid data - 01 02 0Z 04 05
    // Note that header is undefined
    set_input("01020Z0405c0c0", 14);
    EXPECT_EQ(zm_read_hex_header(&z, &hdr), BAD_DIGIT);
}

TEST(rz, calc_hdr_crc)
{
    ZHDR hdr = { .type = 0x01, .flags = { .f3 = 0x02, .f2 = 0x03, .f1 = 0x04, .f0 = 0x05 } };

    zm_calc_hdr_crc(&hdr);

    EXPECT_EQ(hdr.crc1, 0x82);
    EXPECT_EQ(hdr.crc2, 0x08);

    EXPECT_EQ(CRC(hdr.crc1, hdr.crc2), 0x8208);

    ZHDR real_hdr = { .type = 0x06, .flags = { .f3 = 0x00, .f2 = 0x00, .f1 = 0x00, .f0 = 0x00 } };

    zm_calc_hdr_crc(&real_hdr);

    EXPECT_EQ(real_hdr.crc1, 0xcd);
    EXPECT_EQ(real_hdr.crc2, 0x85);

    EXPECT_EQ(CRC(real_hdr.crc1, real_hdr.crc2), 0xcd85);
}

TEST(rz, to_hex_header)
{
    ZHDR hdr = {
        .type = 0x01,
        .flags = {
          .f3 = 0x02,
          .f2 = 0x03,
          .f1 = 0x04,
          .f0 = 0x05,
        },
        .crc1 = 0x0a,
        .crc2 = 0x0b
    };

    uint8_t buf[0xff];
    memset(buf, 0, 0xff);

    // Too small - buffer not modified
    EXPECT_EQ(zm_to_hex_header(&hdr, buf, 0x01), OUT_OF_SPACE);
    EXPECT_EQ(buf[0], 0);

    // Still too small - buffer not modified
    EXPECT_EQ(zm_to_hex_header(&hdr, buf, HEX_HDR_STR_LEN - 1), OUT_OF_SPACE);
    EXPECT_EQ(buf[0], 0);

    // Exactly correct size
    EXPECT_EQ(zm_to_hex_header(&hdr, buf, HEX_HDR_STR_LEN), HEX_HDR_STR_LEN);
    EXPECT_EQ(strcmp("B01020304050a0b\r\x8a", (const char *)buf), 0);

    memset(buf, 0, 0xff);

    // Exactly correct size
    EXPECT_EQ(zm_to_hex_header(&hdr, buf, HEX_HDR_STR_LEN), HEX_HDR_STR_LEN);
    EXPECT_EQ(strcmp("B01020304050a0b\r\x8a", (const char *)buf), 0);

    memset(buf, 0, 0xff);

    // More than enough space
    EXPECT_EQ(zm_to_hex_header(&hdr, buf, 0xff), HEX_HDR_STR_LEN);
    EXPECT_EQ(strcmp("B01020304050a0b\r\x8a", (const char *)buf), 0);
}

TEST(rz, read_escaped)
{
    // simple non-control characters
    set_input("ABC", 3);

    EXPECT_EQ(zm_read_escaped(), 'A');
    EXPECT_EQ(zm_read_escaped(), 'B');
    EXPECT_EQ(zm_read_escaped(), 'C');

    // CLOSED if end of stream
    EXPECT_EQ(zm_read_escaped(), CLOSED);

    // XON/XOFF are skipped
    set_input("\x11\x11\x13Z", 4);

    EXPECT_EQ(zm_read_escaped(), 'Z');
    EXPECT_EQ(zm_read_escaped(), CLOSED);

    // 5x CAN cancels
    set_input("\x18\x18\x18\x18\x18ZYX", 8);
    EXPECT_EQ(zm_read_escaped(), CANCELLED);
    EXPECT_EQ(zm_read_escaped(), 'Z');
}

//
// RZ routine with a C++ interface.
//
void rz_wrapper(std::istream &input, std::ostream &output)
{
    zm_input = &input;
    zm_output = &output;
    zm_receive_file();
    zm_input = nullptr;
    zm_output = nullptr;
}

TEST(rz, receive_abc)
{
    disk_setup();

    Interact session(rz_wrapper);
    EXPECT_EQ(session.receive(), "");

    session.send("**\x18""B00000000000000\x0D\x8A\x11");                 // 21 bytes
    EXPECT_EQ(session.receive(), "**\x18""B0100000023be50\x0D\x8A\x11"); // 21 bytes

    session.send("*\x18""C\x04\x00\x00\x00\x00\xDDQ\xA2""3abc.txt\x004 "
                 "14745364070 100644 0 1 4\x00\x18k\x8F\x82\x82\xCF\x11"); // 54 bytes
    EXPECT_EQ(session.receive(), "**\x18""B0900000000a87c\x0D\x8A\x11");   // 21 bytes

    session.send("*\x18""C\x0A\x00\x00\x00\x00\xBC\xEF\x92\x8C""abc\x0A\x18h"
                 "\xF1\xE2H\x00*\x18""C\x0B\x04\x00\x00\x00[Q\x18\xD0>"); // 35 bytes
    EXPECT_EQ(session.receive(), "**\x18""B0100000023be50\x0D\x8A\x11");  // 21 bytes

    session.send("**\x18""B0800000000022d\x0D\x8A");                 // 20 bytes
    EXPECT_EQ(session.receive(), "**\x18""B0800000000022d\x0D\x8A"); // 20 bytes
}
