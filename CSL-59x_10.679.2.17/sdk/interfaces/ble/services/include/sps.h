/**
 * \addtogroup MID_INT_BLE_SERVICES
 * \{
 * \addtogroup BLE_SER_SPS Serial Port Service
 *
 * \brief Serial port service sample implementation API
 * \{
 */

/**
 ****************************************************************************************
 *
 * @file sps.h
 *
 * @brief Serial Port Service sample implementation API
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

#ifndef SPS_H_
#define SPS_H_

#include "ble_service.h"

/**
 * SPS Flow Control flags values
 */
typedef enum {
        SPS_FLOW_CONTROL_ON = 0x01,
        SPS_FLOW_CONTROL_OFF = 0x02,
} sps_flow_control_t;

typedef void (* sps_set_flow_control_cb_t) (ble_service_t *svc, uint16_t conn_idx, sps_flow_control_t value);

typedef void (* sps_rx_data_cb_t) (ble_service_t *svc, uint16_t conn_idx, const uint8_t *value,
                                                                                uint16_t length);

typedef void (* sps_tx_done_cb_t) (ble_service_t *svc, uint16_t conn_idx, uint16_t length);

/**
 * SPS application callbacks
 */
typedef struct {
        /** Remote client wrote new value of flow control characteristic */
        sps_set_flow_control_cb_t set_flow_control;
        /** Data received from remote client */
        sps_rx_data_cb_t          rx_data;
        /** Service finished TX transaction */
        sps_tx_done_cb_t          tx_done;
} sps_callbacks_t;

/**
 * \brief Register Serial Port Service instance
 *
 * Function registers SPS instance
 *
 * \param [in] cb               application callbacks
 *
 * \return service instance
 *
 */
ble_service_t *sps_init(sps_callbacks_t *cb);

/**
 * \brief Set flow control value
 *
 * Function updates flow control value.
 *
 * \param [in] svc              service instance
 * \param [in] conn_idx         connection index
 * \param [in] value            flow control value
 *
 */
void sps_set_flow_control(ble_service_t *svc, uint16_t conn_idx, sps_flow_control_t value);

/**
 * \brief TX data available
 *
 * Function notifies new data is available for client. After sending data, service
 * will call tx_done callback.
 *
 * \param [in] svc              service instance
 * \param [in] conn_idx         connection index
 * \param [in] data             tx data
 * \param [in] length           tx data length
 *
 */
void sps_tx_data(ble_service_t *svc, uint16_t conn_idx, uint8_t *data, uint16_t length);

#endif /* SPS_H_ */
/**
 \}
 \}
 */
