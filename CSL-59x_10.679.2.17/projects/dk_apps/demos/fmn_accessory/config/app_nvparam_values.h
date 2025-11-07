/**
 ****************************************************************************************
 *
 * @file app_nvparam_values.h
 *
 * @brief Non-volatile parameters description for create_nvparam script
 *
 * Copyright (C) 2024 Renesas Electronics Corporation and/or its affiliates.
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

#include "platform_nvparam_values.h"

/**
 * \note
 * This file is not used in regular build. It is only used by create_nvparam script to create flash
 * image to populate parameters partition with default parameter values.
 * See utilities/nvparam for more information.
 *
 */

#ifndef U16
#define U16(VALUE)      (uint8_t)(VALUE), (uint8_t)(VALUE >> 8)
#endif
#ifndef U32
#define U32(VALUE)      (uint8_t)(VALUE), (uint8_t)(VALUE >> 8), \
                        (uint8_t)(VALUE >> 16), (uint8_t)(VALUE >> 24)
#endif

/**
 * BLE application name
 *
 * \note Make sure to update TAG_ASYM_SUOTA_SCAN_RESP_DATA and TAG_ASYM_SUOTA_DEVICE_NAME param
 *       values when changing BLE_APP_NAME
 * \note BLE_APP_NAME length shall be less than 30 bytes, as it is used in scan response
 */
#define BLE_APP_NAME    'R', 'e', 'n', 'e', 's', 'a', 's', ' ', 'F', 'M', 'N', ' ', \
                        'A', 'c', 'c', 'e', 's', 's', 'o', 'r', 'y'

/**
 * 'ble_app' area parameter values
 */
NVPARAM_PARAM_VALUE(TAG_BLE_APP_NAME, char, BLE_APP_NAME)
NVPARAM_PARAM_VALUE(TAG_BLE_SERIAL_NUMBER, char, '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', \
                                                 'a', 'b', 'c', 'd', 'e', 'f')

/**
 * 'asym_suota_config' area parameter values
 *
 * \note In case of connectable advertising, maximum advertising data length
 *       (TAG_ASYM_SUOTA_ADV_DATA_LEN) is BLE_ADV_DATA_LEN_MAX bytes.
 *       Refer to ble_gap_adv_data_set() for further details.
 */
//                                                                ,-- parameter value          'validity' flag --
//                                                                |                                              |
//                                                                V                                              V
NVPARAM_PARAM_VALUE(TAG_ASYM_SUOTA_ADV_TIMEOUT,        uint8_t, U32(0x0000EA60),                                0x00) // 60000ms
NVPARAM_PARAM_VALUE(TAG_ASYM_SUOTA_ADV_INTERVAL_MIN,   uint8_t, U16(0x044C),                                    0x00) // 687.5ms
NVPARAM_PARAM_VALUE(TAG_ASYM_SUOTA_ADV_INTERVAL_MAX,   uint8_t, U16(0x044C),                                    0x00) // 687.5ms
NVPARAM_PARAM_VALUE(TAG_ASYM_SUOTA_ADV_CHANNEL_MAP,    uint8_t, 0x07,                                           0x00) // 37, 38, 39
NVPARAM_PARAM_VALUE(TAG_ASYM_SUOTA_ADV_DATA,           uint8_t, 0x03, 0x02, 0xF5, 0xFE, 0xFF, 0xFF, 0xFF, 0xFF,
                                                                0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
                                                                0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
                                                                0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,       0x00)
NVPARAM_PARAM_VALUE(TAG_ASYM_SUOTA_ADV_DATA_LEN,       uint8_t, 0x04,                                           0x00) // 4 bytes
NVPARAM_PARAM_VALUE(TAG_ASYM_SUOTA_SCAN_RESP_DATA,     uint8_t, 0x16, 0x09, BLE_APP_NAME,
                                                                0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00)
NVPARAM_PARAM_VALUE(TAG_ASYM_SUOTA_SCAN_RESP_DATA_LEN, uint8_t, 0x17,                                           0x00) // 23 bytes
NVPARAM_PARAM_VALUE(TAG_ASYM_SUOTA_DEVICE_NAME,        uint8_t, BLE_APP_NAME, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
                                                                0xFF, 0xFF, 0xFF, 0xFF, 0xFF,                   0x00)
NVPARAM_PARAM_VALUE(TAG_ASYM_SUOTA_DEVICE_NAME_LEN,    uint8_t, 0x15,                                           0x00) // 21 bytes
NVPARAM_PARAM_VALUE(TAG_ASYM_SUOTA_GAP_MTU,            uint8_t, U16(0x00FB),                                    0x00) // 251 bytes
NVPARAM_PARAM_VALUE(TAG_ASYM_SUOTA_CONN_INTERVAL_MIN,  uint8_t, U16(0x08),                                      0x00) // 10ms -> 8
NVPARAM_PARAM_VALUE(TAG_ASYM_SUOTA_CONN_INTERVAL_MAX,  uint8_t, U16(0x10),                                      0x00) // 20ms -> 16
NVPARAM_PARAM_VALUE(TAG_ASYM_SUOTA_CONN_LATENCY,       uint8_t, U16(0x00),                                      0x00) // 0
NVPARAM_PARAM_VALUE(TAG_ASYM_SUOTA_CONN_TIMEOUT,       uint8_t, U16(0x64),                                      0x00) // 1s -> 100
NVPARAM_PARAM_VALUE(TAG_ASYM_SUOTA_SECURITY_REQ,       uint8_t, 0x00,                                           0x00) // SECUR_REQ_LVL_1
NVPARAM_PARAM_VALUE(TAG_ASYM_SUOTA_BD_ADDR_TYPE,       uint8_t, 0x03,                                           0x00) // PRIVATE_RANDOM_NONRESOLVABLE_ADDRESS
NVPARAM_PARAM_VALUE(TAG_ASYM_SUOTA_BD_ADDR_ADDRESS,    uint8_t, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,             0xFF) // undefined
NVPARAM_PARAM_VALUE(TAG_ASYM_SUOTA_BD_ADDR_RENEW_DUR,  uint8_t, U16(1024),                                      0x00) // 1024s random address renew duration
