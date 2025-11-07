/**
 \addtogroup MID_INT_BLE_API
 \{
 */

/**
 ****************************************************************************************
 *
 * @file ble_gattc_util.h
 *
 * @brief BLE GATT Client Utilities API
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

#ifndef BLE_GATTC_UTIL_H_
#define BLE_GATTC_UTIL_H_

#include "ble_gattc.h"
#include "ble_uuid.h"

/**
 * \brief Initialize browse event iterators
 *
 * This call will initialize internal structures to iterate over ::BLE_EVT_GATTC_BROWSE_SVC. After
 * this call the application can use ble_gattc_util_find_characteristic() to get a characteristic
 * from the event. \p evt instance should be valid as long as the iterator is used since only weak
 * reference is stored internally.
 *
 * \param [in] evt  instance of event
 *
 */
void ble_gattc_util_find_init(const ble_evt_gattc_browse_svc_t *evt);

/**
 * \brief Find characteristic in browse event
 *
 * This call will return the first characteristic from the event which matches the given UUID (in
 * case \p uuid is \p NULL, the first characteristic will be returned). Subsequent calls with the
 * same given \p uuid will return subsequent characteristics matching the same criteria or \p NULL
 * if no more matching characteristics are found.
 *
 * ble_gattc_util_find_init() must be called prior to calling this function.
 *
 * Subsequent calls with different \p uuid will restart searching from the first characteristic.
 *
 * The returned item will always have the ::GATTC_ITEM_TYPE_CHARACTERISTIC type.
 *
 * \param [in] uuid  optional UUID of characteristic
 *
 * \return found item
 *
 * \sa ble_gattc_util_find_init()
 *
 */
const gattc_item_t *ble_gattc_util_find_characteristic(const att_uuid_t *uuid);

/**
 * \brief Find descriptor in browse event
 *
 * This call will return the first descriptor from the event which matches the given UUID (if
 * \p uuid is \p NULL, the first descriptor will be returned). Subsequent calls with the same
 * given \p uuid will return subsequent descriptors matching the same criteria or \p NULL if no more
 * matching descriptors are found.
 *
 * ble_gattc_util_find_characteristic() must be called and the specified characteristic must be
 * found prior to calling this function.
 *
 * Subsequent calls with different \p uuid will restart searching from the first descriptor.
 *
 * Returned item will always have the ::GATTC_ITEM_TYPE_DESCRIPTOR type.
 *
 * \param [in] uuid  optional UUID of descriptor
 *
 * \return found item
 *
 * \sa ble_gattc_util_find_characteristic()
 *
 */
const gattc_item_t *ble_gattc_util_find_descriptor(const att_uuid_t *uuid);

/**
 * \brief Write value to CCC descriptor
 *
 * This function writes a Client Characteristic Configuration Descriptor value to a given handle.
 *
 * \param [in]  conn_idx        connection index
 * \param [in]  handle          CCC descriptor handle
 * \param [in]  ccc             value to be written
 *
 * \return status of GATT write operation
 */

__STATIC_INLINE ble_error_t ble_gattc_util_write_ccc(uint16_t conn_idx, uint16_t handle,
                                                                                gatt_ccc_t ccc)
{
        uint16_t value = ccc;

        return ble_gattc_write(conn_idx, handle, 0, sizeof(value), (uint8_t *) &value);
}

#endif /* BLE_GATTC_UTIL_H_ */
/**
 \}
 */
