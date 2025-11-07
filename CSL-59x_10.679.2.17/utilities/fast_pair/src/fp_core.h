/**
 ****************************************************************************************
 *
 * @file fp_core.h
 *
 * @brief Google Fast Pair framework core module internal header file
 *
 * Copyright (C) 2024-2025 Renesas Electronics Corporation and/or its affiliates.
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

#ifndef FP_CORE_H_
#define FP_CORE_H_

#include <stdint.h>
#include <stdbool.h>
#include FAST_PAIR_CONFIG_FILE
#include "fp_defaults.h"
#include "fast_pair.h"

/**
 * \brief Check error and handle if necessary
 */
#define FP_CHECK_ERROR(error)      if (error) { fp_error(error); }

/**
 * \brief Restart Fast Pair/FMDN advertise
 *
 * \return 0 if success, other value otherwise
 */
int fp_restart_advertise(void);

/**
 * \brief Update Fast Pair/FMDN advertise data
 *
 * param [in] discoverable change advertise data for discoverable or non-dcoverable mode
 *
 * \return 0 if success, other value otherwise
 */
int fp_update_advertise_data(bool discoverable);

#if (FP_LOCATOR_TAG != 1)
/**
 * \brief Check if account key filter UI indication should be shown
 *
 * \return true to show account key filter UI indication, false otherwise
 */
bool fp_get_acc_key_filter_ui_indication(void);
#endif /* !FP_LOCATOR_TAG */

#if (FP_BATTERIES_COUNT != 0)
/**
 * \brief Get array with battery information
 *
 * \return pointer to battery information array
 */
fp_battery_info_t *fp_get_battery_information(void);

#if (FP_BATTERY_NOTIFICATION == 1)
/**
 * \brief Check if battery UI indication should be shown
 *
 * \return true to show battery UI indication, false otherwise
 */
bool fp_get_battery_ui_indication(void);
#endif /* FP_BATTERY_NOTIFICATION */
#endif /* FP_BATTERIES_COUNT */

/**
 * \brief Send notifications to Google Fast Pair framework
 *
 * \param [in] notif notification bitmap
 */
void fp_send_notification(uint32_t notif);

#if (FP_FMDN == 1)
/**
 * \brief FMDN provisioning state
 */
typedef enum {
        FP_FMDN_PROVISIONING_STOPPED,
        FP_FMDN_PROVISIONING_WAITING,
        FP_FMDN_PROVISIONING_INITIATING,
        FP_FMDN_PROVISIONING_STARTED
} FP_FMDN_PROVISIONING_STATE;

/**
 * \brief Set FMDN provisioning state of Google Fast Pair framework core module
 *
 * \param [in] state FMDN provisioning state
 */
void fp_set_fmdn_provisioning_state(FP_FMDN_PROVISIONING_STATE state);

/**
 * \brief Get FMDN provisioning state of Google Fast Pair framework core module
 *
 * \return FMDN provisioning state
 */
FP_FMDN_PROVISIONING_STATE fp_get_fmdn_provisioning_state(void);

/**
 * \brief Indicate that connection is authenticated
 *
 * \return conn_idx connection index
 */
void fp_set_authenticated_conn(uint16_t conn_idx);
#endif /* FP_FMDN */

/**
 * \brief Set random address renewal for Google Fast Pair framework
 *
 * \param [in] status random address renewal (1: enable, 0: disable)
 *
 * \return result code
 */
ble_error_t fp_set_rand_addr_renewal(uint8_t status);

/**
 * \brief Error handler for Google Fast Pair framework
 *
 * \param [in] error_code error code
 */
void fp_error(int error_code);

#endif /* FP_CORE_H_ */
