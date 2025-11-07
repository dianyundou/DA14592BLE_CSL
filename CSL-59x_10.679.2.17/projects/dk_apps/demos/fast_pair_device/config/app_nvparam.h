/**
 ****************************************************************************************
 *
 * @file app_nvparam.h
 *
 * @brief Configuration of non-volatile parameters for the application
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

#ifndef APP_NVPARAM_H_
#define APP_NVPARAM_H_

#include "fast_pair_config.h"
#include "platform_nvparam.h"

/**
 * Tags definition for 'ble_app' area.
 */
#define TAG_BLE_APP_NAME                        0x01 // up to 29 bytes value

/**
 * Tags definition for 'asym_suota_config' area.
 */
#define TAG_ASYM_SUOTA_ADV_TIMEOUT              0x01
#define TAG_ASYM_SUOTA_ADV_INTERVAL_MIN         0x02
#define TAG_ASYM_SUOTA_ADV_INTERVAL_MAX         0x03
#define TAG_ASYM_SUOTA_ADV_CHANNEL_MAP          0x04
#define TAG_ASYM_SUOTA_ADV_DATA                 0x05
#define TAG_ASYM_SUOTA_ADV_DATA_LEN             0x06
#define TAG_ASYM_SUOTA_SCAN_RESP_DATA           0x07
#define TAG_ASYM_SUOTA_SCAN_RESP_DATA_LEN       0x08
#define TAG_ASYM_SUOTA_DEVICE_NAME              0x09
#define TAG_ASYM_SUOTA_DEVICE_NAME_LEN          0x0A
#define TAG_ASYM_SUOTA_GAP_MTU                  0x0B
#define TAG_ASYM_SUOTA_CONN_INTERVAL_MIN        0x0C
#define TAG_ASYM_SUOTA_CONN_INTERVAL_MAX        0x0D
#define TAG_ASYM_SUOTA_CONN_LATENCY             0x0E
#define TAG_ASYM_SUOTA_CONN_TIMEOUT             0x0F
#define TAG_ASYM_SUOTA_SECURITY_REQ             0x10
#define TAG_ASYM_SUOTA_BD_ADDR_TYPE             0x11
#define TAG_ASYM_SUOTA_BD_ADDR_ADDRESS          0x12
#define TAG_ASYM_SUOTA_BD_ADDR_RENEW_DUR        0x13

/**
 * Tags definition for 'fp_app' area.
 */
#define TAG_FP_APP_PERSONALIZED_NAME            0x01 // up to FP_PERSONALIZED_NAME_MAX_LENGTH bytes
#define TAG_FP_APP_EPHEMERAL_IDENTITY_KEY       0x02 // FP_FMDN_EPHEMERAL_IDENTITY_KEY_LENGTH bytes
#define TAG_FP_APP_ACCOUNT_KEY_LIST             0x03 // up to FP_ACCOUNT_KEYS_COUNT * FP_ACC_KEYS_ACCOUNT_KEY_LENGTH bytes
#define TAG_FP_APP_BEACON_TIME                  0x04 // 4-byte beacon time

/**
 * 'ble_app' area definition
 *
 * Variable-length parameters length need to include 2 extra bytes for parameter header and length.
 *
 * \note It does not follow 'ble_platform' area directly on flash to allow adding new parameters
 *       to 'ble_platform' without need to move 'ble_app'.
 */
NVPARAM_AREA(ble_app, NVMS_PARAM_PART, 0x0100)
        NVPARAM_VARPARAM(TAG_BLE_APP_NAME,                      0x0000, 33) // uint8[31]
NVPARAM_AREA_END()

/**
 * 'asym_suota_config' area definition
 *
 * Parameters length need to include 1 extra byte as validity flag.
 *
 * \note NVPARAM area offset (i.e. 0x0400) in NVMS_PARAM_PART partition is defined by the
 *       Asymmetric SUOTA firmware.
 */
NVPARAM_AREA(asym_suota_config, NVMS_PARAM_PART, 0x0400)
        NVPARAM_PARAM(TAG_ASYM_SUOTA_ADV_TIMEOUT,               0x0000,  4 + 1)
        NVPARAM_PARAM(TAG_ASYM_SUOTA_ADV_INTERVAL_MIN,          0x0005,  2 + 1)
        NVPARAM_PARAM(TAG_ASYM_SUOTA_ADV_INTERVAL_MAX,          0x0008,  2 + 1)
        NVPARAM_PARAM(TAG_ASYM_SUOTA_ADV_CHANNEL_MAP,           0x000B,  1 + 1)
        NVPARAM_PARAM(TAG_ASYM_SUOTA_ADV_DATA,                  0x000D, 31 + 1)
        NVPARAM_PARAM(TAG_ASYM_SUOTA_ADV_DATA_LEN,              0x002D,  1 + 1)
        NVPARAM_PARAM(TAG_ASYM_SUOTA_SCAN_RESP_DATA,            0x002F, 31 + 1)
        NVPARAM_PARAM(TAG_ASYM_SUOTA_SCAN_RESP_DATA_LEN,        0x004F,  1 + 1)
        NVPARAM_PARAM(TAG_ASYM_SUOTA_DEVICE_NAME,               0x0051, 32 + 1)
        NVPARAM_PARAM(TAG_ASYM_SUOTA_DEVICE_NAME_LEN,           0x0072,  1 + 1)
        NVPARAM_PARAM(TAG_ASYM_SUOTA_GAP_MTU,                   0x0074,  2 + 1)
        NVPARAM_PARAM(TAG_ASYM_SUOTA_CONN_INTERVAL_MIN,         0x0077,  2 + 1)
        NVPARAM_PARAM(TAG_ASYM_SUOTA_CONN_INTERVAL_MAX,         0x007A,  2 + 1)
        NVPARAM_PARAM(TAG_ASYM_SUOTA_CONN_LATENCY,              0x007D,  2 + 1)
        NVPARAM_PARAM(TAG_ASYM_SUOTA_CONN_TIMEOUT,              0x0080,  2 + 1)
        NVPARAM_PARAM(TAG_ASYM_SUOTA_SECURITY_REQ,              0x0083,  1 + 1)
        NVPARAM_PARAM(TAG_ASYM_SUOTA_BD_ADDR_TYPE,              0x0085,  1 + 1)
        NVPARAM_PARAM(TAG_ASYM_SUOTA_BD_ADDR_ADDRESS,           0x0087,  6 + 1)
        NVPARAM_PARAM(TAG_ASYM_SUOTA_BD_ADDR_RENEW_DUR,         0x008e,  2 + 1)
NVPARAM_AREA_END()

#ifdef IN_AD_NVPARAM_C
#if (dg_configSUOTA_ASYMMETRIC == 1)
/**
 * 'asym_suota_config_dyn' area definition for dynamically changed parameters
 *
 * Parameters length need to include 1 extra byte as validity flag.
 *
 * \note NVPARAM area offset (i.e. 0x0900) in NVMS_GENERIC_PART partition is defined by the
 *       Asymmetric SUOTA firmware.
 */
NVPARAM_AREA(asym_suota_config_dyn, NVMS_GENERIC_PART, 0x0900)
        NVPARAM_PARAM(TAG_ASYM_SUOTA_ADV_TIMEOUT,               0x0000,  4 + 1)
        NVPARAM_PARAM(TAG_ASYM_SUOTA_ADV_INTERVAL_MIN,          0x0005,  2 + 1)
        NVPARAM_PARAM(TAG_ASYM_SUOTA_ADV_INTERVAL_MAX,          0x0008,  2 + 1)
        NVPARAM_PARAM(TAG_ASYM_SUOTA_ADV_CHANNEL_MAP,           0x000B,  1 + 1)
        NVPARAM_PARAM(TAG_ASYM_SUOTA_ADV_DATA,                  0x000D, 31 + 1)
        NVPARAM_PARAM(TAG_ASYM_SUOTA_ADV_DATA_LEN,              0x002D,  1 + 1)
        NVPARAM_PARAM(TAG_ASYM_SUOTA_SCAN_RESP_DATA,            0x002F, 31 + 1)
        NVPARAM_PARAM(TAG_ASYM_SUOTA_SCAN_RESP_DATA_LEN,        0x004F,  1 + 1)
        NVPARAM_PARAM(TAG_ASYM_SUOTA_DEVICE_NAME,               0x0051, 32 + 1)
        NVPARAM_PARAM(TAG_ASYM_SUOTA_DEVICE_NAME_LEN,           0x0072,  1 + 1)
        NVPARAM_PARAM(TAG_ASYM_SUOTA_GAP_MTU,                   0x0074,  2 + 1)
        NVPARAM_PARAM(TAG_ASYM_SUOTA_CONN_INTERVAL_MIN,         0x0077,  2 + 1)
        NVPARAM_PARAM(TAG_ASYM_SUOTA_CONN_INTERVAL_MAX,         0x007A,  2 + 1)
        NVPARAM_PARAM(TAG_ASYM_SUOTA_CONN_LATENCY,              0x007D,  2 + 1)
        NVPARAM_PARAM(TAG_ASYM_SUOTA_CONN_TIMEOUT,              0x0080,  2 + 1)
        NVPARAM_PARAM(TAG_ASYM_SUOTA_SECURITY_REQ,              0x0083,  1 + 1)
        NVPARAM_PARAM(TAG_ASYM_SUOTA_BD_ADDR_TYPE,              0x0085,  1 + 1)
        NVPARAM_PARAM(TAG_ASYM_SUOTA_BD_ADDR_ADDRESS,           0x0087,  6 + 1)
        NVPARAM_PARAM(TAG_ASYM_SUOTA_BD_ADDR_RENEW_DUR,         0x008e,  2 + 1)
NVPARAM_AREA_END()
#endif /* dg_configSUOTA_ASYMMETRIC */

/**
 * 'fp_app' area definition
 *
 * Parameters length need to include 2 extra bytes for parameter header.
 *
 * \note Make sure to update app_nvparam_values.h when changing FP_ACCOUNT_KEYS_COUNT and
 *       FP_PERSONALIZED_NAME_MAX_LENGTH
 */
#if (dg_configSUOTA_ASYMMETRIC == 1)
NVPARAM_AREA(fp_app, NVMS_GENERIC_PART, 0x0A00)
#else
NVPARAM_AREA(fp_app, NVMS_GENERIC_PART, 0x0900)
#endif /* dg_configSUOTA_ASYMMETRIC */
        NVPARAM_VARPARAM(TAG_FP_APP_PERSONALIZED_NAME,          0x0000, FP_PERSONALIZED_NAME_MAX_LENGTH + 2)
#if (FP_FMDN == 1)
        NVPARAM_VARPARAM(TAG_FP_APP_EPHEMERAL_IDENTITY_KEY,     0x0042, FP_FMDN_EPHEMERAL_IDENTITY_KEY_LENGTH + 2)
#endif
        NVPARAM_VARPARAM(TAG_FP_APP_ACCOUNT_KEY_LIST,           0x0064, (FP_ACCOUNT_KEYS_COUNT * FP_ACC_KEYS_ACCOUNT_KEY_LENGTH) + 2 )
#if (FP_FMDN == 1)
        NVPARAM_VARPARAM(TAG_FP_APP_BEACON_TIME,                0x00B6, 4 + 2 ) // time is kept as uint32_t
#endif
NVPARAM_AREA_END()
#endif

#endif /* APP_NVPARAM_H_ */
