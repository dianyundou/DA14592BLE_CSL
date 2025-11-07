/**
 \addtogroup MID_INT_BLE_API
 \{
 */

/**
 ****************************************************************************************
 *
 * @file ble_storage.h
 *
 * @brief BLE persistent storage API
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

#ifndef BLE_STORAGE_H_
#define BLE_STORAGE_H_

#include <stdbool.h>
#include <stdint.h>
#include "ble_common.h"

/* Storage key categories - should not be used directly */
#define BLE_STORAGE_KEYCAT_SRV (0x00000000)
#define BLE_STORAGE_KEYCAT_CLI (0x10000000)
#define BLE_STORAGE_KEYCAT_APP (0x20000000)

/** Storage key for attribute on GATT server (by handle) */
#define BLE_STORAGE_KEY_SRV(handle) (BLE_STORAGE_KEYCAT_SRV | (handle & 0xFFFF))

/** Storage key for attribute on GATT client (by handle) */
#define BLE_STORAGE_KEY_CLI(handle) (BLE_STORAGE_KEYCAT_CLI | (handle & 0xFFFF))

/** Storage key for application-defined values */
#define BLE_STORAGE_KEY_APP(app_id, val_id) (BLE_STORAGE_KEYCAT_APP | ((app_id & 0xFF) << 16) | (val_id & 0xFFFF))

/**
 * \brief Callback to free buffer stored in storage
 *
 * \param [in] ptr  buffer
 *
 */
typedef void (* ble_storage_free_cb_t) (void *ptr);

/**
 * \brief Storage key (for indexing values in storage)
 *
 * Storage keys namespace is divided into different category thus should not be hard-coded as raw
 * number, but appropriate macros should be used to ensure no collision between different users of
 * storage, see STORAGE_KEY_SRV(), STORAGE_KEY_CLI() and STORAGE_KEY_APP().
 *
 */
typedef uint32_t ble_storage_key_t;

/**
 * \brief Store signed integer value in storage
 *
 * \note
 * While \p key can be an arbitrary value, it's recommended to use values within service attribute
 * handles range to avoid collision with other services or applications.
 *
 * \param [in] conn_idx         connection index
 * \param [in] key              storage key (attribute handle)
 * \param [in] value            value to be stored
 * \param [in] persistent       if true, value will be persistent for bonded devices
 *
 * \return error code
 *
 */
ble_error_t ble_storage_put_i32(uint16_t conn_idx, ble_storage_key_t key, int32_t value, bool persistent);

/**
 * \brief Store unsigned integer value in storage
 *
 * \note
 * While \p key can be an arbitrary value, it's recommended to use values within service attribute
 * handles range to avoid collision with other services or applications.
 *
 * \param [in] conn_idx         connection index
 * \param [in] key              storage key (attribute handle)
 * \param [in] value            value to be stored
 * \param [in] persistent       if true, value will be persistent for bonded devices
 *
 * \return error code
 *
 */
ble_error_t ble_storage_put_u32(uint16_t conn_idx, ble_storage_key_t key, uint32_t value, bool persistent);

/**
 * \brief Store data buffer in storage
 *
 * \p length shall be non-zero.
 * Buffer pointed by \p ptr is owned by storage after calling this function and shall not be freed
 * by application. It will be freed automatically when value is removed from storage. Optional \p
 * free_cb callback can be provided if buffer stores data which shall be handled in a special way,
 * i.e. pointers to other buffers.
 *
 * \note
 * While \p key can be an arbitrary value, it's recommended to use values within service attribute
 * handles range to avoid collision with other services or applications.
 *
 * \param [in] conn_idx         connection index
 * \param [in] key              storage key (attribute handle)
 * \param [in] length           length of buffer
 * \param [in] ptr              buffer pointer
 * \param [in] free_cb          callback to free buffer
 * \param [in] persistent       if true, value will be persistent for bonded devices
 *
 * \return error code
 *
 */
ble_error_t ble_storage_put_buffer(uint16_t conn_idx, ble_storage_key_t key, uint16_t length, void *ptr,
                                                ble_storage_free_cb_t free_cb, bool persistent);

/**
 * \brief Store copy of data buffer in storage
 *
 * This is identical to ble_storage_put_buffer() except it makes copy of buffer pointed by \p ptr.
 *
 * \param [in] conn_idx         connection index
 * \param [in] key              storage key (attribute handle)
 * \param [in] length           length of buffer
 * \param [in] ptr              buffer pointer
 * \param [in] free_cb          callback to free buffer
 * \param [in] persistent       if true, value will be persistent for bonded devices
 *
 * \return error code
 *
 */
ble_error_t ble_storage_put_buffer_copy(uint16_t conn_idx, ble_storage_key_t key, uint16_t length, void *ptr,
                                                ble_storage_free_cb_t free_cb, bool persistent);

/**
 * \brief Get int8 value from storage
 *
 * This has unpredictable result when used with \p key which stores unsigned value or buffer, no
 * sanity checks are performed. Value of \p value is not changed if key is not found in storage.
 *
 * \param [in]  conn_idx        connection index
 * \param [in]  key             storage key (attribute handle)
 * \param [out] value           value
 *
 * \return error code
 *
 */
ble_error_t ble_storage_get_i8(uint16_t conn_idx, ble_storage_key_t key, int8_t *value);

/**
 * \brief Get uint8 value from storage
 *
 * This has unpredictable result when used with \p key which stores signed value or buffer, no
 * sanity checks are performed. Value of \p value is not changed if key is not found in storage.
 *
 * \param [in]  conn_idx        connection index
 * \param [in]  key             storage key (attribute handle)
 * \param [out] value           value
 *
 * \return error code
 *
 */
ble_error_t ble_storage_get_u8(uint16_t conn_idx, ble_storage_key_t key, uint8_t *value);

/**
 * \brief Get int16 value from storage
 *
 * This has unpredictable result when used with \p key which stores unsigned value or buffer, no
 * sanity checks are performed. Value of \p value is not changed if key is not found in storage.
 *
 * \param [in]  conn_idx        connection index
 * \param [in]  key             storage key (attribute handle)
 * \param [out] value           value
 *
 * \return error code
 *
 */
ble_error_t ble_storage_get_i16(uint16_t conn_idx, ble_storage_key_t key, int16_t *value);

/**
 * \brief Get uint16 value from storage
 *
 * This has unpredictable result when used with \p key which stores signed value or buffer, no
 * sanity checks are performed. Value of \p value is not changed if key is not found in storage.
 *
 * \param [in]  conn_idx        connection index
 * \param [in]  key             storage key (attribute handle)
 * \param [out] value           value
 *
 * \return error code
 *
 */
ble_error_t ble_storage_get_u16(uint16_t conn_idx, ble_storage_key_t key, uint16_t *value);

/**
 * \brief Get int32 value from storage
 *
 * This has unpredictable result when used with \p key which stores unsigned value or buffer, no
 * sanity checks are performed. Value of \p value is not changed if key is not found in storage.
 *
 * \param [in]  conn_idx        connection index
 * \param [in]  key             storage key (attribute handle)
 * \param [out] value           value
 *
 * \return error code
 *
 */
ble_error_t ble_storage_get_i32(uint16_t conn_idx, ble_storage_key_t key, int32_t *value);

/**
 * \brief Get uint32 value from storage
 *
 * This has unpredictable result when used with \p key which stores signed value or buffer, no
 * sanity checks are performed. Value of \p value is not changed if key is not found in storage.
 *
 * \param [in]  conn_idx        connection index
 * \param [in]  key             storage key (attribute handle)
 * \param [out] value           value
 *
 * \return error code
 *
 */
ble_error_t ble_storage_get_u32(uint16_t conn_idx, ble_storage_key_t key, uint32_t *value);

/**
 * \brief Get buffer value from storage
 *
 * This has unpredictable result when used with \p key which stores signed or unsigned value, no
 * sanity checks are performed. Value of \p value is not changed if key is not found in storage.
 *
 * \param [in]  conn_idx        connection index
 * \param [in]  key             storage key (attribute handle)
 * \param [out] length          length of returned buffer
 * \param [out] ptr             pointer to buffer
 *
 * \return error code
 *
 */
ble_error_t ble_storage_get_buffer(uint16_t conn_idx, ble_storage_key_t key, uint16_t *length, void **ptr);

/**
 * \brief Remove value from storage
 *
 * \param [in] conn_idx         connection index
 * \param [in] key              storage key (attribute handle)
 *
 * \return error code
 *
 */
ble_error_t ble_storage_remove(uint16_t conn_idx, ble_storage_key_t key);

/**
 * \brief Remove value from storage
 *
 * This function removes value in all devices where it is stored.
 *
 * \param [in] key              storage key (attribute handle)
 *
 * \return error code
 *
 */
ble_error_t ble_storage_remove_all(ble_storage_key_t key);

#endif /* BLE_STORAGE_H_ */

/**
 \}
 */
