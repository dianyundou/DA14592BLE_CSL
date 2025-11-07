/**
 * \addtogroup MID_INT_BLE_SERVICES
 * \{
 * \addtogroup BLE_SER_SCPS Scan Parameters Service
 *
 * \brief Scan parameters service sample implementation API
 * \{
 */

/**
 ****************************************************************************************
 *
 * @file scps.h
 *
 * @brief Scan Parameters Service sample implementation API
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

#ifndef SCPS_H_
#define SCPS_H_

#include <stdint.h>
#include "ble_service.h"

/**
 * \brief Scan Interval Window value updated by client callback
 *
 * \param [in] conn_idx connection index
 * \param [in] interval scan interval
 * \param [in] window   scan window
 *
 */
typedef void (* scps_scan_updated_cb_t) (uint16_t conn_idx, uint16_t interval, uint16_t window);

/**
 * \brief CCC for Scan Interval Window updated by client callback
 *
 * \param [in] conn_idx connection index
 * \param [in] value    value written by client
 *
 */
typedef void (* scps_ccc_changed_cb_t) (uint16_t conn_idx, uint16_t value);

/**
 * \brief Client disconnected callback
 *
 * ScPS client disconnected, last known values of Scan Interval and Scan Window are passed.
 * Application should store these values if required since they are no longer maintained
 * by ScPS instance.
 *
 * \param [in] conn_idx connection index
 * \param [in] interval scan interval
 * \param [in] window   scan window
 *
 */
typedef void (* scps_disconnected_cb_t) (uint16_t conn_idx, uint16_t interval, uint16_t window);

/** ScPS application callbacks */
typedef struct {
        /** Scan Interval Window value updated by client */
        scps_scan_updated_cb_t  scan_updated;

        /** CCC for Scan Interval Window updated by client */
        scps_ccc_changed_cb_t   ccc_changed;

        /** ScPS client disconnected, last known values of Scan Interval Window are passed.
         *  Application should store these values if required since they are no longer maintained
         *  by ScPS instance.
         */
        scps_disconnected_cb_t  disconnected;
} scps_callbacks_t;

/**
 * \brief ScPS initialization
 *
 * \param [in] cb       structure with ScPS callbacks
 *
 */
ble_service_t *scps_init(const scps_callbacks_t *cb);

/**
 * \brief Request Scan Refresh from client
 *
 * \param [in] svc      service instance
 * \param [in] conn_idx connection index
 *
 */
void scps_notify_refresh(ble_service_t *svc, uint16_t conn_idx);

/**
 * \brief Request Scan Refresh from all connected clients
 *
 * \param [in] svc      service instance
 *
 */
void scps_notify_refresh_all(ble_service_t *svc);

#endif /* SCPS_H_ */
/**
 \}
 \}
 */
