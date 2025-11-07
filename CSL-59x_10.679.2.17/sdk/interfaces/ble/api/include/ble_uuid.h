/**
 \addtogroup MID_INT_BLE_API
 \{
 */

/**
 ****************************************************************************************
 *
 * @file ble_uuid.h
 *
 * @brief BLE UUID declarations
 *
 * Copyright (C) 2015-2018 Renesas Electronics Corporation and/or its affiliates.
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

#ifndef BLE_UUID_H_
#define BLE_UUID_H_

#include "ble_att.h"

/* Services UUIDs */
#define UUID_SERVICE_GAS                        (0x1800) // Generic Access Service
#define UUID_SERVICE_GATT                       (0x1801) // Generic Attribute
#define UUID_SERVICE_IAS                        (0x1802) // Immediate Alert Service
#define UUID_SERVICE_LLS                        (0x1803) // Link Loss Service
#define UUID_SERVICE_TPS                        (0x1804) // Tx Power Service
#define UUID_SERVICE_CTS                        (0x1805) // Current Time Service
#define UUID_SERVICE_RTUS                       (0x1806) // Reference Time Update Service
#define UUID_SERVICE_NDCS                       (0x1807) // Next DST Change Service
#define UUID_SERVICE_GLS                        (0x1808) // Glucose Service
#define UUID_SERVICE_HTS                        (0x1809) // Health Thermometer Service
#define UUID_SERVICE_DIS                        (0x180A) // Device Information Service
#define UUID_SERVICE_HRS                        (0x180D) // Heart Rate Service
#define UUID_SERVICE_PASS                       (0x180E) // Phone Alert Status Service
#define UUID_SERVICE_BAS                        (0x180F) // Battery Service
#define UUID_SERVICE_BLS                        (0x1810) // Blood Pressure Service
#define UUID_SERVICE_AND                        (0x1811) // Alert Notification Service
#define UUID_SERVICE_HIDS                       (0x1812) // Human Interface Device Service
#define UUID_SERVICE_SCPS                       (0x1813) // Scan Parameters Service
#define UUID_SERVICE_RSCS                       (0x1814) // Running Speed and Cadence Service
#define UUID_SERVICE_CSCS                       (0x1816) // Cycling Speed and Cadence Service
#define UUID_SERVICE_CPS                        (0x1818) // Cycling Power Service
#define UUID_SERVICE_LNS                        (0x1819) // Location and Navigation Service
#define UUID_SERVICE_ESS                        (0x181A) // Environmental Sensing Service
#define UUID_SERVICE_BCS                        (0x181B) // Body Composition Service
#define UUID_SERVICE_UDS                        (0x181C) // User Data Service
#define UUID_SERVICE_WSS                        (0x181D) // Weight Scale Service
#define UUID_SERVICE_BMS                        (0x181E) // Bond Management Service
#define UUID_SERVICE_CGMS                       (0x181F) // Continuous Glucose Monitoring Service
#define UUID_SERVICE_IPSS                       (0x1820) // Internet Protocol Support Service
#define UUID_SERVICE_IPS                        (0x1821) // Indoor Positioning Service

/* GATT UUIDs */
#define UUID_GATT_PRIMARY_SERVICE               (0x2800)
#define UUID_GATT_SECONDARY_SERVICE             (0x2801)
#define UUID_GATT_INCLUDE                       (0x2802)
#define UUID_GATT_CHARACTERISTIC                (0x2803)

/* GATT descriptors UUIDs */
#define UUID_GATT_CHAR_EXT_PROPERTIES           (0x2900)
#define UUID_GATT_CHAR_USER_DESCRIPTION         (0x2901)
#define UUID_GATT_CLIENT_CHAR_CONFIGURATION     (0x2902)
#define UUID_GATT_SERVER_CHAR_CONFIGURATION     (0x2903)
#define UUID_GATT_CHAR_PRESENTATION_FORMAT      (0x2904)
#define UUID_GATT_CHAR_AGGREGATE_FORMAT         (0x2905)

/**
 * \brief Create 16-bit UUID from integer value
 *
 * \param [in]  uuid16  value
 * \param [out] uuid    UUID
 *
 */
void ble_uuid_create16(uint16_t uuid16, att_uuid_t *uuid);

/**
 * \brief Create UUID from buffer
 *
 * \p buf shall have proper length (16 bytes) and endianess (little-endian)
 * Created UUID is 16-bit in case it's Bluetooth UUID or 128-bit otherwise.
 *
 * \param [in]  buf  buffer
 * \param [out] uuid UUID
 *
 */
void ble_uuid_from_buf(const uint8_t *buf, att_uuid_t *uuid);

/**
 * \brief Create UUID from string
 *
 * \p str shall be UUID in canonical form (aaaaaaaa-bbbb-cccc-dddd-eeeeeeeeeeee) or 16-bit
 * Bluetooth UUID.
 * Created UUID is 16-bit in case it is Bluetooth UUID, otherwise it's 128-bit.
 *
 * \param [in]  str  string
 * \param [out] uuid UUID
 *
 * \return true if conversion is successful, false otherwise
 *
 */
bool ble_uuid_from_string(const char *str, att_uuid_t *uuid);

/**
 * \brief Convert UUID to string
 *
 * Convert UUID (16-bit or 128-bit case) to UUID string - (0xaaaa) or canonical form
 * (aaaaaaaa-bbbb-cccc-dddd-eeeeeeeeeeee). The result is pointer to static buffer.
 *
 * \param [in] uuid UUID
 *
 * \return UUID string
 *
 */
const char *ble_uuid_to_string(const att_uuid_t *uuid);

/**
 * \brief Check if two UUIDs are equal
 *
 * \param [in] uuid1 1st uuid
 * \param [in] uuid2 2nd uuid
 *
 * \return true if both UUIDs are equal, false otherwise
 *
 */
bool ble_uuid_equal(const att_uuid_t *uuid1, const att_uuid_t *uuid2);

#endif /* BLE_UUID_H_ */
/**
 \}
 */
