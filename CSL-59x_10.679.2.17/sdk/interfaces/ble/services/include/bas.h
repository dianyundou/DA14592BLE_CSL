/**
 * \addtogroup MID_INT_BLE_SERVICES
 * \{
 * \addtogroup BLE_SER_BAS Battery Service
 *
 * \brief Battery service sample implementation API
 * \{
 */

/**
 ****************************************************************************************
 *
 * @file bas.h
 *
 * @brief Battery Service sample implementation API
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

#ifndef BAS_H_
#define BAS_H_

#include <stdint.h>
#include "ble_service.h"

/**
 * Battery instance information
 *
 * This corresponds to contents of Content Presentation Format descriptor for Battery Level
 * characteristic. Other fields of this descriptor are set internally to proper values.
 */
typedef struct {
        uint8_t         namespace;      /**< namespace */
        uint16_t        descriptor;     /**< descriptor */
} bas_battery_info_t;

/**
 * Register Battery Service instance
 *
 * For application with single instance of BAS, \p info parameter is optional. Otherwise \p info
 * is mandatory as per BAS 1.0 specification since this will add Content Presentation Format
 * descriptor to Battery Level characteristic.
 *
 * \param [in] config   service configuration
 * \param [in] info     battery instance information
 *
 * \return service instance
 *
 */
ble_service_t *bas_init(const ble_service_config_t *config, const bas_battery_info_t *info);

/**
 * Notify client with battery level
 *
 * \param [in] svc      service instance
 * \param [in] conn_idx connection index
 *
 */
void bas_notify_level(ble_service_t *svc, uint16_t conn_idx);

/**
 * Set reported battery level
 *
 * \param [in] svc      service instance
 * \param [in] level    new battery level
 * \param [in] notify   true if clients shall be notified, false otherwise
 *
 */
void bas_set_level(ble_service_t *svc, uint8_t level, bool notify);

#endif /* BAS_H_ */
/**
 \}
 \}
 */
