/**
 * \addtogroup MID_INT_BLE_CLIENTS
 * \{
 * \addtogroup BLE_CLI_GATT GATT client
 *
 * \brief GATT client API
 * \{
 */

/**
 ****************************************************************************************
 *
 * @file ble_client.h
 *
 * @brief GATT Client handling routines API
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

#ifndef BLE_CLIENT_H_
#define BLE_CLIENT_H_

#include <stdbool.h>
#include "ble_gattc.h"
#include "ble_gap.h"

typedef struct ble_client ble_client_t;

/**
* \brief Read completed callback
*
* Function to be called when a read request has been completed.
*
* \param [in] client    client instance
* \param [in] evt       read completed event
*
*/
typedef void (* read_completed_evt_t) (ble_client_t *client,
                                                        const ble_evt_gattc_read_completed_t *evt);

/**
* \brief Write completed callback
*
* Function to be called when a write request has been completed.
*
* \param [in] client    client instance
* \param [in] evt       write completed event
*
*/
typedef void (* write_completed_evt_t) (ble_client_t *client,
                                                const ble_evt_gattc_write_completed_t *evt);

/**
* \brief Notification callback
*
* Function to be called when a notification has been received.
*
* \param [in] client    client instance
* \param [in] evt       notification event
*
*/
typedef void (* notification_evt_t) (ble_client_t *client,
                                                        const ble_evt_gattc_notification_t *evt);

/**
* \brief Indication callback
*
* Function to be called when an indication has been received.
*
* \param [in] client    client instance
* \param [in] evt       indication event
*
*/
typedef void (* indication_evt_t) (ble_client_t *client, const ble_evt_gattc_indication_t *evt);

/**
* \brief Disconnected callback
*
* Function to be called when disconnected from a remote device.
*
* \param [in] client    client instance
* \param [in] evt       disconnected event
*
*/
typedef void (* disconnect_evt_t) (ble_client_t *client, const ble_evt_gap_disconnected_t *evt);

/**
* \brief Serialize callback
*
* Function to be called when serialization occurs - pack client's data to the specified buffer.
* \p data must be a buffer with proper length in this case.
*
* If called with NULL as \p data, then \p length will be set to required buffer size. Serialization
* will not be triggered in this case.
*
* \note If client cannot be serialized then \p length is set to 0. Otherwise \p length is set to the
* number of bytes used the serialized client.
*
* \param [in] client    client instance
* \param [out] data     data buffer
* \param [out] length   used/needed buffer size
*
* \sa ble_client_serialize
*
*/
typedef void (* serialize_cb_t) (ble_client_t *client, void *data, size_t *length);

/**
* \brief Attach callback
*
* Function to be called when a client is attached to a new connection index.
*
* \param [in] client    client instance
*
*/
typedef void (* attach_cb_t) (ble_client_t *client);

/**
* \brief Cleanup callback
*
* Function to be called when a client is destroyed.
*
* \param [in] client    client instance
*
*/
typedef void (* cleanup_cb_t) (ble_client_t *client);

/**
 * BLE Client structure
 */
typedef struct ble_client {
        uint16_t conn_idx;                              /**< Connection index */

        uint16_t                start_h;                /**< Service start handle */
        uint16_t                end_h;                  /**< Service end handle */

        read_completed_evt_t    read_completed_evt;     /**< Read completed callback */
        write_completed_evt_t   write_completed_evt;    /**< Write completed callback */
        notification_evt_t      notification_evt;       /**< Notification callback */
        indication_evt_t        indication_evt;         /**< Indication callback */
        disconnect_evt_t        disconnected_evt;       /**< Disconnected callback */

        serialize_cb_t          serialize;              /**< Serialize callback */
        attach_cb_t             attach;                 /**< Attach callback */
        cleanup_cb_t            cleanup;                /**< Cleanup callback */
} ble_client_t;

/**
* \brief Add client
*
* Adds a client to the internal database. It is required in order to receive client callbacks.
*
* \param [in] client    client instance
*
*/
void ble_client_add(ble_client_t *client);

/**
* \brief Remove client
*
* Removes a client from the internal database.
*
* \param [in] client    client instance
*
*/
void ble_client_remove(const ble_client_t *client);

/**
 * \brief Cleanup client
 *
 * Frees the resources allocated for the client.
 *
 * \param [in] client    client instance
 *
 */
void ble_client_cleanup(ble_client_t *client);

/**
 * \brief Cleanup all clients
 *
 * Frees the resources allocated for all added clients.
 *
 */
void ble_clients_cleanup(void);

/**
 * \brief Serialize client
 *
 * Pack client's data to the buffer from which client could be initialized in the future. \p data
 * must be a buffer with proper length in this case.
 *
 * If called with NULL as \p data, then \p length will be set to required buffer size. Serialization
 * will not be triggered in this case.
 *
 * \note If client cannot be serialized then \p length is set to 0. Otherwise \p length is set to a
 * number of bytes used by serialized client.
 *
 * \param [in] client    client instance
 * \param [out] data     data buffer
 * \param [out] length   used/needed buffer size
 *
 */
void ble_client_serialize(ble_client_t *client, void *data, size_t *length);

/**
 * \brief Attach client
 *
 * Attaches a client to a given connection index and adds it to the internal database.
 *
 * \param [in] client    client instance
 * \param [in] conn_idx  new connection index
 *
 */
void ble_client_attach(ble_client_t *client, uint16_t conn_idx);

/**
* \brief Handle BLE event
*
* Handles BLE events and passes them to clients.
*
* \param [in] evt       BLE event
*
*/
void ble_client_handle_event(const ble_evt_hdr_t *evt);

/**
* \brief Check if client is in given handles range
*
* Checks if given client's handles are within given range
*
* \param [in] client    client instance
* \param [in] start_h   start of handle range
* \param [in] end_h     end of handle range
*
* \return true if client in range, false otherwise
*
*/
__STATIC_INLINE bool ble_client_in_range(const ble_client_t *client, uint16_t start_h, uint16_t end_h)
{
        return (client && (client->end_h >= start_h) && (client->start_h <= end_h));
}

#endif /* BLE_CLIENT_H_ */

/**
 \}
 \}
 */
