/**
 * \addtogroup MID_INT_BLE_SERVICES
 * \{
 * \addtogroup BLE_SER_ASYM_SUOTA Asymmetric SUOTA service
 *
 * \brief Asymmetric SUOTA service implementation API
 * \{
 */

/**
 ****************************************************************************************
 *
 * @file asym_suota.h
 *
 * @brief Asymmetric SUOTA service implementation API
 *
 * Copyright (C) 2024 Renesas Electronics Corporation and/or its affiliates.
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

#ifndef ASYM_SUOTA_H_
#define ASYM_SUOTA_H_

#include "ble_service.h"

/*
 * TYPE DEFINITIONS
 *****************************************************************************************
 */
/**
 * \brief Asymmetric SUOTA service command
 */
typedef enum {
        ASYM_SUOTA_CMD_REBOOT_IN_SUOTA_MODE = 0x01,/**< Command to reboot in SUOTA mode */
} ASYM_SUOTA_CMD;

/**
 * \brief Command callback is called when a command is received in the service
 *
 * \param [in] svc              service instance
 * \param [in] conn_idx         connection index
 * \param [in] value            command value
 */
typedef void (* asym_suota_cmd_cb_t) (ble_service_t *svc, uint16_t conn_idx, ASYM_SUOTA_CMD value);

/**
 * \brief ID read callback is called when the ID is read by the peer device
 *
 * \note Returning a zero length ID is considered as a fall-back to BD address matching reconnect
 *
 * \param [in] svc              service instance
 * \param [in] conn_idx         connection index
 * \param [out] value           ID value to be sent to peer device
 * \param [out] length          length of ID value
 *
 * \return application status to be sent to peer device
 */
typedef att_error_t (* asym_suota_id_cb_t) (ble_service_t *svc, uint16_t conn_idx, uint8_t **value, uint16_t *length);

/**
 * ASYM_SUOTA application callbacks
 */
typedef struct {
        /** Data received from remote client */
        asym_suota_cmd_cb_t     cmd_callback;
        asym_suota_id_cb_t      id_callback;
} asym_suota_callbacks_t;

/*
 * API FUNCTION DECLARATIONS
 *****************************************************************************************
 */
/**
 * \brief Register Asymmetric SUOTA service instance
 *
 * Function registers service instance
 *
 * \param [in] config           service configuration
 * \param [in] cb               application callback
 *
 * \return service instance
 *
 */
ble_service_t *asym_suota_init(const ble_service_config_t *config, const asym_suota_callbacks_t *cb);

#endif /* ASYM_SUOTA_H_ */
/**
 \}
 \}
 */
