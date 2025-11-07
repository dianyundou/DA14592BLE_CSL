/**
 \addtogroup MID_INT_BLE_API
 \{
 */

/**
 ****************************************************************************************
 *
 * @file ble_gatt.h
 *
 * @brief Common definitions for GATT API
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

#ifndef BLE_GATT_H_
#define BLE_GATT_H_

/** GATT service type */
typedef enum {
        GATT_SERVICE_PRIMARY,
        GATT_SERVICE_SECONDARY,
} gatt_service_t;

/** GATT event type */
typedef enum {
        GATT_EVENT_NOTIFICATION,
        GATT_EVENT_INDICATION,
} gatt_event_t;

/** GATT characteristic properties */
typedef enum {
        GATT_PROP_NONE                          = 0,
        GATT_PROP_BROADCAST                     = 0x0001,
        GATT_PROP_READ                          = 0x0002,
        GATT_PROP_WRITE_NO_RESP                 = 0x0004,
        GATT_PROP_WRITE                         = 0x0008,
        GATT_PROP_NOTIFY                        = 0x0010,
        GATT_PROP_INDICATE                      = 0x0020,
        GATT_PROP_WRITE_SIGNED                  = 0x0040,
        GATT_PROP_EXTENDED                      = 0x0080,
        GATT_PROP_EXTENDED_RELIABLE_WRITE       = 0x0100,
        GATT_PROP_EXTENDED_WRITABLE_AUXILIARIES = 0x0200,
} gatt_prop_t;

/** GATT Client Characteristic Configuration bitmask values */
typedef enum {
        GATT_CCC_NONE           = 0x0000,
        GATT_CCC_NOTIFICATIONS  = 0x0001,
        GATT_CCC_INDICATIONS    = 0x0002,
} gatt_ccc_t;

#endif /* BLE_GATT_H_ */
/**
 \}
 */
