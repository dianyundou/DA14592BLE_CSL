/**
 ****************************************************************************************
 *
 * @file fp_ble.h
 *
 * @brief Google Fast Pair and FMDN BLE operations header file
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

#ifndef FP_BLE_H_
#define FP_BLE_H_

#include <stdint.h>
#include "sdk_defs.h"
#include "ble_gap.h"
#include "ble_common.h"

/**
 * \brief BLE GAP advertising parameters for Google Fast Pair and FMDN
 */
typedef struct {
        gap_disc_mode_t mode;           /**< Discoverability mode for advertising */
        gap_conn_mode_t type;           /**< Advertising type */
        uint16_t intv_min;              /**< Minimum interval in steps of 0.625ms */
        uint16_t intv_max;              /**< Maximum interval in steps of 0.625ms */
        gap_tx_power_t tx_power;        /**< TX power level for advertising */
} fp_ble_adv_params_t;

/**
 * \brief Initialize advertising for Google Fast Pair and FMDN
 *
 * This function is called by Google Fast Pair framework to initialize resources for advertising.
 */
void fp_ble_adv_init(void);

/**
 * \brief De-initialize advertising for Google Fast Pair and FMDN
 *
 * This function is called by Google Fast Pair framework to release allocated resources for
 * Google Fast Pair and FMDN advertising.
 */
void fp_ble_adv_deinit(void);

/**
 * \brief Set advertising parameters for Google Fast Pair and FMDN
 *
 * This function is called by Google Fast Pair framework to set advertising parameters before
 * starting advertising.
 *
 * \param [in] params advertising parameters
 *
 * \return result code
 */
ble_error_t fp_ble_adv_set_params(const fp_ble_adv_params_t *params);

/**
 * \brief Set advertising TX power for Google Fast Pair and FMDN
 *
 * This function is called by Google Fast Pair framework to set the TX power level for advertising.
 *
 * \param [in] tx_power TX power level (see ::gap_tx_power_t for available options)
 *
 * \return result code
 */
ble_error_t fp_ble_adv_set_tx_power(gap_tx_power_t tx_power);

/**
 * \brief Set advertising and scan response data for Google Fast Pair and FMDN
 *
 * This function is called by Google Fast Pair framework to set advertising and scan response data.
 *
 * \param [in] ad_len   number of advertising data structures
 * \param [in] ad       pointer to advertising data structures
 * \param [in] sd_len   number of scan response data structures
 * \param [in] sd       pointer to scan response data structures
 *
 * \return result code
 */
ble_error_t fp_ble_adv_set_ad_struct(size_t ad_len, const gap_adv_ad_struct_t *ad,
        size_t sd_len, const gap_adv_ad_struct_t *sd);

/**
 * \brief Start advertising for Google Fast Pair and FMDN
 *
 * This function is called by Google Fast Pair framework to start advertising.
 *
 * \return result code
 */
ble_error_t fp_ble_adv_start(void);

/**
 * \brief Check if advertising for Google Fast Pair and FMDN is started
 *
 * This function is called by Google Fast Pair framework for checking advertising status.
 *
 * \return true if advertising is started, false otherwise
 */
bool fp_ble_adv_is_started(void);

/**
 * \brief Stop advertising for Google Fast Pair and FMDN
 *
 * This function is called by Google Fast Pair framework to stop advertising.
 *
 * \return result code
 */
ble_error_t fp_ble_adv_stop(void);

/**
 * \brief Stop advertising
 *
 * This function is called by Google Fast Pair framework to stop all advertising events.
 *
 * \return result code
 */
ble_error_t fp_ble_adv_stop_all(void);

#endif /* FP_BLE_H_ */
