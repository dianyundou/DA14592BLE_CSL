/**
 \addtogroup MID_INT_BLE_API
 \{
 */

/**
 ****************************************************************************************
 *
 * @file ble_attribdb.h
 *
 * @brief Helper to manage complex attributes database
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

#ifndef BLE_ATTRIBDB_H_
#define BLE_ATTRIBDB_H_

#include <stdint.h>

/**
 * NOTE:
 * THIS API IS NO LONGER SUPPORTED AND WILL BE REMOVED - use ble_storage.h instead
 */

typedef struct {
        uint16_t length;
        union {
                int  i32;
                void *ptr;
        };
} ble_attribdb_value_t;

typedef void (* ble_attribdb_foreach_cb_t) (uint16_t conn_idx, const ble_attribdb_value_t *val, void *ud);

void ble_attribdb_put_int(uint16_t conn_idx, uint16_t handle, int value);

void ble_attribdb_put_buffer(uint16_t conn_idx, uint16_t handle, uint16_t length, void *buffer);

int ble_attribdb_get_int(uint16_t conn_idx, uint16_t handle, int def_value);

void *ble_attribdb_get_buffer(uint16_t conn_idx, uint16_t handle, uint16_t *length);

void ble_attribdb_remove(uint16_t conn_idx, uint16_t handle, bool free);

void ble_attribdb_foreach_conn(uint16_t handle, ble_attribdb_foreach_cb_t cb, void *ud);

#endif /* BLE_ATTRIBDB_H_ */
/**
 \}
 */
