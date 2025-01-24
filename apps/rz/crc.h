#ifndef _CRC_H_
#define _CRC_H_

#include <stdint.h>

static const uint16_t CRC_START_XMODEM = 0u;
static const uint32_t CRC_START_32     = ~0u;

uint16_t update_crc16_ccitt(uint8_t ch, uint16_t crc);
uint32_t update_crc32(uint8_t ch, uint32_t crc);

#endif // _CRC_H_
