/**
 * \addtogroup MID_INT_BLE_SERVICES
 * \{
 * \addtogroup BLE_SER_BMS Bond Management Service
 *
 * \brief Bond management service sample implementation API
 * \{
 */

/**
 ****************************************************************************************
 *
 * @file bms.h
 *
 * @brief Bond Management Service implementation API
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
#ifndef BMS_H_
#define BMS_H_

#include "ble_service.h"

/**
 * BMS Delete Bond operations
 */
typedef enum {
        /** Delete bond of requesting device */
        BMS_DELETE_BOND_REQ_DEV = 0x01,
        /** Delete bond of requesting device with authorization code */
        BMS_DELETE_BOND_REQ_DEV_AUTH = 0x02,
        /** Delete all bonds on server */
        BMS_DELETE_BOND_ALL_DEV = 0x04,
        /** Delete all bonds on server with authorization code */
        BMS_DELETE_BOND_ALL_DEV_AUTH = 0x08,
        /** Delete bond of all except the requesting device on the server */
        BMS_DELETE_BOND_ALL_EXCEPT_REQ_DEV = 0x10,
        /** Delete bond of all except the requesting device on the server with authorization code */
        BMS_DELETE_BOND_ALL_EXCEPT_REQ_DEV_AUTH = 0x20,
} bms_delete_bond_op_t;

/**
 * BMS Delete Bond status
 */
typedef enum {
        /** Delete Bond status OK */
        BMS_DELETE_BOND_STATUS_OK = 0x00,
        /** Delete Bond status failed */
        BMS_DELETE_BOND_STATUS_FAILED = 0x01,
        /** Delete Bond status insufficient authentication */
        BMS_DELETE_BOND_STATUS_INSUFFICIENT_AUTH = 0x02,
        /** Delete Bond status operation not supported */
        BMS_DELETE_BOND_STATUS_NOT_SUPPORTED = 0x03,
} bms_delete_bond_status_t;

/**
 * BMS config used during initialization
 */
typedef struct {
        bms_delete_bond_op_t supported_delete_bond_op;
} bms_config_t;

typedef void (* delete_bond_cb_t)(bms_delete_bond_op_t op, uint16_t conn_idx, uint16_t length,
                                                                        const uint8_t *auth_code);

/**
 * BMS application callbacks
 */
typedef struct {
        /** Called when remote requests delete bond operation */
        delete_bond_cb_t delete_bond;
} bms_callbacks_t;

/**
 * \brief Register BMS instance
 *
 * Function registers HID Service
 *
 * \param [in] config           general service config
 * \param [in] bms_config       BMS specific config
 * \param [in] callbacks        application callbacks
 *
 * \return service instance
 *
 * \note
 * Mandatory feature of Bond Management Service is BMS_DELETE_BOND_REQ_DEV operation
 * and it will be added to service features even if not declared in bms_config.
 */
ble_service_t *bms_init(const ble_service_config_t *config, const bms_config_t *bms_config,
                                                                const bms_callbacks_t *callbacks);

/**
 * \brief Delete bond confirmation
 *
 * Function sends confirmation to remote device. It should be called once delete_bond
 * callback is called.
 *
 * \param [in] service          BMS service instance
 * \param [in] conn_idx         connection index
 * \param [in] status           operation status
 */
void bms_delete_bond_cfm(ble_service_t *service, uint16_t conn_idx, bms_delete_bond_status_t status);

#endif /* BMS_H_ */
/**
 \}
 \}
 */
