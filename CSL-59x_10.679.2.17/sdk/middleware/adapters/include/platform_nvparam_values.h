/**
 ****************************************************************************************
 *
 * @file platform_nvparam_values.h
 *
 * @brief Non-volatile parameters description for create_nvparam script
 *
 * Copyright (C) 2016-2020 Renesas Electronics Corporation and/or its affiliates.
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
 * \note
 * This file is not used in regular build. It is only used by create_nvparam script to create flash
 * image to populate parameters partition with default parameter values.
 * See utilities/nvparam for more information.
 *
 ****************************************************************************************
 */

#define U16(VALUE) (uint8_t)(VALUE), (uint8_t)(VALUE >> 8)

//                                                                        ,-- parameter value           'validity' flag --
//                                                                        |                                               |
//                                                                        V                                               V
NVPARAM_PARAM_VALUE( NVPARAM_BLE_PLATFORM_BD_ADDRESS,         uint8_t,  0x06, 0x00, 0xF4, 0x35, 0x23, 0x48,              0x00)
NVPARAM_PARAM_VALUE( NVPARAM_BLE_PLATFORM_LPCLK_DRIFT,        uint8_t,  U16(0xFFFF),                                     0xFF)
NVPARAM_PARAM_VALUE( NVPARAM_BLE_PLATFORM_EXT_WAKEUP_TIME,    uint8_t,  U16(0xFFFF),                                     0xFF)
NVPARAM_PARAM_VALUE( NVPARAM_BLE_PLATFORM_OSC_WAKEUP_TIME,    uint8_t,  U16(0xFFFF),                                     0xFF)
NVPARAM_PARAM_VALUE( NVPARAM_BLE_PLATFORM_RM_WAKEUP_TIME,     uint8_t,  U16(0xFFFF),                                     0xFF)
NVPARAM_PARAM_VALUE( NVPARAM_BLE_PLATFORM_SLEEP_ENABLE,       uint8_t,  0xFF,                                            0xFF)
NVPARAM_PARAM_VALUE( NVPARAM_BLE_PLATFORM_EXT_WAKEUP_ENABLE,  uint8_t,  0xFF,                                            0xFF)
NVPARAM_PARAM_VALUE( NVPARAM_BLE_PLATFORM_BLE_CA_TIMER_DUR,   uint8_t,  U16(0xFFFF),                                     0xFF)
NVPARAM_PARAM_VALUE( NVPARAM_BLE_PLATFORM_BLE_CRA_TIMER_DUR,  uint8_t,  0xFF,                                            0xFF)
NVPARAM_PARAM_VALUE( NVPARAM_BLE_PLATFORM_BLE_CA_MIN_RSSI,    uint8_t,  0xFF,                                            0xFF)
NVPARAM_PARAM_VALUE( NVPARAM_BLE_PLATFORM_BLE_CA_NB_PKT,      uint8_t,  0xFF,                                            0xFF)
NVPARAM_PARAM_VALUE( NVPARAM_BLE_PLATFORM_BLE_CA_NB_BAD_PKT,  uint8_t,  0xFF,                                            0xFF)
NVPARAM_PARAM_VALUE( NVPARAM_BLE_PLATFORM_IRK,                uint8_t,  0xEF, 0xCD, 0xAB, 0x89, 0x67, 0x45, 0x23, 0x01, \
                                                                        0xEF, 0xCD, 0xAB, 0x89, 0x67, 0x45, 0x23, 0x01,  0x00)
