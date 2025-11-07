/**
 ****************************************************************************************
 *
 * @file ble_uuid.c
 *
 * @brief BLE UUID definitions
 *
 * Copyright (C) 2015-2023 Renesas Electronics Corporation and/or its affiliates.
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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "ble_uuid.h"

static uint8_t base_uuid[] = { 0xFB, 0x34, 0x9B, 0x5F, 0x80, 0x00, 0x00, 0x80,
                                                0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

static void compact_uuid(att_uuid_t *uuid)
{
        if (uuid->type == ATT_UUID_16) {
                // nothing to compact
                return;
        }

        if (memcmp(uuid->uuid128, base_uuid, 12) || memcmp(&uuid->uuid128[14], &base_uuid[14], 2)) {
                // not a BT UUID
                return;
        }

        uuid->type = ATT_UUID_16;
        uuid->uuid16 = uuid->uuid128[13] << 8 | uuid->uuid128[12];
}

void ble_uuid_create16(uint16_t uuid16, att_uuid_t *uuid)
{
        uuid->type = ATT_UUID_16;
        uuid->uuid16 = uuid16;
}

void ble_uuid_from_buf(const uint8_t *buf, att_uuid_t *uuid)
{
        uuid->type = ATT_UUID_128;
        memcpy(uuid->uuid128, buf, ATT_UUID_LENGTH);

        compact_uuid(uuid);
}

bool ble_uuid_from_string(const char *str, att_uuid_t *uuid)
{
        uint16_t idx = 0;
        int num_dash = 0;
        uint8_t b = 0;
        char *check_ptr;
        int uuid_length = strlen(str);

        if (uuid_length == 4 || uuid_length == 6) {
                uint16_t uuid16 = strtoul(str, &check_ptr, 16);

                if (*check_ptr == '\0') {
                        ble_uuid_create16(uuid16, uuid);
                        return true;
                } else {
                        return false;
                }

        } else if (uuid_length != 36 || str[8] != '-' || str[13] != '-' || str[18] != '-' || str[23] != '-') {
                return false;
        }

        while (*str) {
                char c = *str++;

                if (c == '-') {
                        num_dash++;
                        continue;
                }

                b <<= 4;

                if (c >= '0' && c <= '9') {
                        b |= c - '0';
                } else if (c >= 'a' && c <= 'f') {
                        b |= c - 'a' + 0x0A;
                } else if (c >= 'A' && c <= 'F') {
                        b |= c - 'A' + 0x0A;
                } else {
                        return false;
                }

                idx++;
                // add full byte if idx is even
                if ((idx % 2) == 0) {
                        uuid->uuid128[ATT_UUID_LENGTH - idx / 2] = b;
                }
        }

        if ((idx != ATT_UUID_LENGTH * 2) || (num_dash != 4)) {
                return false;
        }

        uuid->type = ATT_UUID_128;

        // try to make it UUID16 if possible
        compact_uuid(uuid);

        return true;
}

const char *ble_uuid_to_string(const att_uuid_t *uuid)
{
        /**
         * Example 128-bit UUID: aaaaaaaa-bbbb-cccc-dddd-eeeeeeeeeeee
         * It contains 36 characters so the buffer needs to have at least 37 characters.
         */
        static char buf[37];

        if (uuid->type == ATT_UUID_16) {
                sprintf(buf, "0x%04X", uuid->uuid16);

                return buf;
        }

        sprintf(buf, "%02X%02X%02X%02X-%02X%02X-%02X%02X-%02X%02X-%02X%02X%02X%02X%02X%02X",
                        uuid->uuid128[15], uuid->uuid128[14], uuid->uuid128[13], uuid->uuid128[12],
                        uuid->uuid128[11], uuid->uuid128[10], uuid->uuid128[9], uuid->uuid128[8],
                        uuid->uuid128[7], uuid->uuid128[6], uuid->uuid128[5], uuid->uuid128[4],
                        uuid->uuid128[3], uuid->uuid128[2], uuid->uuid128[1], uuid->uuid128[0]);

        return buf;
}

bool ble_uuid_equal(const att_uuid_t *uuid1, const att_uuid_t *uuid2)
{
        if (uuid1->type != uuid2->type) {
                return false;
        }

        if (uuid1->type == ATT_UUID_16) {
                return uuid1->uuid16 == uuid2->uuid16;
        }

        return !memcmp(uuid1->uuid128, uuid2->uuid128, ATT_UUID_LENGTH);
}
