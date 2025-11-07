/**
 ****************************************************************************************
 *
 * @file crc16.c
 *
 * @brief CRC16 calculation
 *
 * Copyright (C) 2015-2019 Renesas Electronics Corporation and/or its affiliates.
 * All rights reserved. Confidential Information.
 *
 * This software ("Software") is supplied by Renesas Electronics Corporation and/or its
 * affiliates ("Renesas"). Renesas grants you a personal, non-exclusive, non-transferable,
 * revocable, non-sub-licensable right and license to use the Software, solely if used in
 * or together with Renesas products. You may make copies of this Software, provided this
 * copyright notice and disclaimer ("Notice") is included in all such copies. Renesas
 * reserves the right to change or discontinue the Software at any time without notice.
 *
 * THE SOFTWARE IS PROVIDED "AS IS". RENESAS DISCLAIMS ALL WARRANTIES OF ANY KIND,
 * WHETHER EXPRESS, IMPLIED, OR STATUTORY, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. TO THE
 * MAXIMUM EXTENT PERMITTED UNDER LAW, IN NO EVENT SHALL RENESAS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, SPECIAL, INCIDENTAL OR CONSEQUENTIAL DAMAGES ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE, EVEN IF RENESAS HAS BEEN ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGES. USE OF THIS SOFTWARE MAY BE SUBJECT TO TERMS AND CONDITIONS CONTAINED IN
 * AN ADDITIONAL AGREEMENT BETWEEN YOU AND RENESAS. IN CASE OF CONFLICT BETWEEN THE TERMS
 * OF THIS NOTICE AND ANY SUCH ADDITIONAL LICENSE AGREEMENT, THE TERMS OF THE AGREEMENT
 * SHALL TAKE PRECEDENCE. BY CONTINUING TO USE THIS SOFTWARE, YOU AGREE TO THE TERMS OF
 * THIS NOTICE.IF YOU DO NOT AGREE TO THESE TERMS, YOU ARE NOT PERMITTED TO USE THIS
 * SOFTWARE.
 *
 ****************************************************************************************
 */
#include "crc16.h"

void crc16_init(uint16_t *crc16)
{
        *crc16 = 0xFFFF;
}

void crc16_update(uint16_t *crc16, const uint8_t *buf, size_t len)
{
        size_t i, j;

        for (i = 0; i < len; i++) {
                uint8_t b = buf[i];

                for (j = 0; j < 8; j++) {
                        uint16_t need_xor = ((*crc16 & 0x8000) >> 15) ^ ((b & 0x80) >> 7);

                        *crc16 <<= 1;

                        if (need_xor) {
                                *crc16 ^= 0x1021; // CRC16-CCITT polynomial
                        }

                        b <<= 1;
                }
        }
}

uint16_t crc16_calculate(const uint8_t *buf, size_t len)
{
        uint16_t crc;

        crc16_init(&crc);
        crc16_update(&crc, buf, len);

        return crc;
}
