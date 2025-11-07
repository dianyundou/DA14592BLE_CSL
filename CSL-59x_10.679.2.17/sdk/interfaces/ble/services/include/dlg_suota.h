/**
 * \addtogroup MID_INT_BLE_SERVICES
 * \{
 * \addtogroup BLE_SER_SUOTA SUOTA Service
 *
 * \brief SUOTA service implementation API
 * \{
 */

/**
 ****************************************************************************************
 *
 * @file dlg_suota.h
 *
 * @brief Dialog SUOTA service implementation API
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

#ifndef DLG_SUOTA_H_
#define DLG_SUOTA_H_

#include <stdint.h>
#include "ble_service.h"

/**
 * SUOTA active image enum
 */
typedef enum {
        SUOTA_ACTIVE_IMG_FIRST,
        SUOTA_ACTIVE_IMG_SECOND,
        SUOTA_ACTIVE_IMG_ERROR,
} suota_active_img_t;

/**
 * SUOTA application status
 *
 */
typedef enum {
        SUOTA_START = 1 << 0,
        SUOTA_ONGOING = 1 << 1,
        SUOTA_DONE = 1 << 2,
        SUOTA_ERROR = 1 << 3,
} suota_app_status_t;

typedef void (* suota_status_cb_t) (uint16_t conn_idx, uint8_t status, uint8_t error_code);
typedef bool (* suota_ready_cb_t ) (uint16_t conn_idx);

/**
 * SUOTA application callbacks
 */
typedef struct {
        suota_ready_cb_t suota_ready;
        suota_status_cb_t suota_status;
} suota_callbacks_t;

/**
 * Register SUOTA Service instance
 *
 * \return service instance
 *
 */
ble_service_t *suota_init(const suota_callbacks_t *cb);

/**
 * Get SUOTA active image
 *
 * \return SUOTA active image
 *
 */
suota_active_img_t suota_get_active_img(ble_service_t *svc);

/**
 * Update CRC
 *
 * param [in] crc current value of CRC
 * param [in] data pointer to data to compute CRC over
 * param [in] len number of bytes pointed by data
 *
 * \return new value of CRC
 *
 */
uint32_t suota_update_crc(uint32_t crc, const uint8_t *data, size_t len);

/**
 * Handle L2CAP event
 *
 * This should be called in application main loop to handle L2CAP events. Application does not need
 * to care which events are passed to this function as it only handles events related to SUOTA
 * service and channel.
 *
 * \note This is only applicable with SUOTA 1.2 and L2CAP CoC support enabled
 *
 */
void suota_l2cap_event(ble_service_t *svc, const ble_evt_hdr_t *event);

#endif /* DLG_SUOTA_H_ */
/**
 \}
 \}
 */
