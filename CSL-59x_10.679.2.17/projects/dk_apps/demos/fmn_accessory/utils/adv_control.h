/**
 ****************************************************************************************
 *
 * @file adv_control.h
 *
 * @brief Advertising control header file
 *
 * Copyright (C) 2025 Renesas Electronics Corporation and/or its affiliates.
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

#ifndef ADV_CONTROL_H_
#define ADV_CONTROL_H_

#include <stdint.h>
#include <stdbool.h>
#include "accessory_config.h"
#include "osal.h"

/* Enable support of multiple advertising events */
#ifndef ADV_CONTROL_MULT_EVENTS_ENABLE
#define ADV_CONTROL_MULT_EVENTS_ENABLE          (0)
#endif

#if (ADV_CONTROL_MULT_EVENTS_ENABLE == 1)
/**
 * \brief Advertising event handle
 */
typedef void *adv_control_event_t;

/**
 * \brief Advertising event parameters
 */
typedef struct {
        gap_disc_mode_t mode;           /**< Discoverability mode for advertising */
        gap_conn_mode_t type;           /**< Advertising type */
        uint16_t intv_min;              /**< Minimum interval in steps of 0.625ms */
        uint16_t intv_max;              /**< Maximum interval in steps of 0.625ms */
        gap_tx_power_t tx_power;        /**< TX power level for advertising */
} adv_control_event_params_t;

/**
 * \brief Initialize advertising control
 *
 * \param [in] task_hdl control task handle
 */
void adv_control_init(OS_TASK task_hdl);

/**
 * \brief Create advertising event
 *
 * This function creates an advertising event with specific parameters.
 *
 * \param [in] params   advertising parameters
 *
 * \return handle of the advertising event if created successfully, otherwise NULL
 */
adv_control_event_t adv_control_event_create(const adv_control_event_params_t *params);

/**
 * \brief Delete advertising event
 *
 * This function deletes a previously created advertising event.
 *
 * \param [in] evt      advertising event handle
 *
 * \return result code
 */
ble_error_t adv_control_event_delete(adv_control_event_t evt);

/**
 * \brief Set parameters of an advertising event
 *
 * \param [in] evt      advertising event handle
 * \param [in] params   advertising parameters
 *
 * \return result code
 */
ble_error_t adv_control_event_set_params(adv_control_event_t evt,
        const adv_control_event_params_t *params);

/**
 * \brief Set TX power level for an advertising event
 *
 * \param [in] evt      advertising event handle
 * \param [in] tx_power TX power level (see ::gap_tx_power_t for available options)
 *
 * \return result code
 */
ble_error_t adv_control_event_set_tx_power(adv_control_event_t evt, gap_tx_power_t tx_power);

/**
 * \brief Set advertising and scan response data of an advertising event
 *
 * \param [in] evt      advertising event handle
 * \param [in] ad_len   number of advertising data structures
 * \param [in] ad       pointer to advertising data structures
 * \param [in] sd_len   number of scan response data structures
 * \param [in] sd       pointer to scan response data structures
 *
 * \return result code
 */
ble_error_t adv_control_event_set_ad_struct(adv_control_event_t evt,
        size_t ad_len, const gap_adv_ad_struct_t *ad, size_t sd_len, const gap_adv_ad_struct_t *sd);

/**
 * \brief Start advertising event
 *
 * \param [in] evt      advertising event handle
 *
 * \return result code
 */
ble_error_t adv_control_event_start(adv_control_event_t evt);

/**
 * \brief Stop advertising event
 *
 * \param [in] evt      advertising event handle
 *
 * \return result code
 */
ble_error_t adv_control_event_stop(adv_control_event_t evt);

/**
 * \brief Stop advertising
 *
 * This function is used for stopping all advertising events.
 *
 * \return result code
 */
ble_error_t adv_control_event_stop_all(void);

/**
 * \brief Check if advertising event is active
 *
 * \param [in] evt      advertising event handle
 *
 * \return true if advertising event is active, otherwise false
 */
bool adv_control_event_is_active(adv_control_event_t evt);

/**
 * \brief Handle BLE event for advertising control
 *
 * \param [in] evt BLE event
 *
 * \return true if event was handled, false if it needs to be handled by application
 */
bool adv_control_handle_event(ble_evt_hdr_t *evt);

/**
 * \brief Process advertising control task notification
 *
 * \param [in] notif control task notification
 */
void adv_control_process_notif(uint32_t notif);

#endif /* ADV_CONTROL_MULT_EVENTS_ENABLE */

#endif /* ADV_CONTROL_H_ */
