/**
 ****************************************************************************************
 *
 * @file fn_control.h
 *
 * @brief Finder Network control header file
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

#ifndef FN_CONTROL_H_
#define FN_CONTROL_H_

#include <stdint.h>
#include <stdbool.h>
#include "accessory_config.h"

/**
 * \brief Finder network
 */
typedef enum {
        FINDER_NETWORK_NONE,
        FINDER_NETWORK_AFMN_PAIRING,
        FINDER_NETWORK_AFMN,
        FINDER_NETWORK_GFP_PAIRING,
        FINDER_NETWORK_GFP,
        FINDER_NETWORK_GFP_FMDN
} FINDER_NETWORK_STATE;

/**
 * \brief Callback called when accessory state changes in terms of active finder network
 *
 * param [in] fn finder network state
 */
typedef void (*fn_control_state_cb_t)(FINDER_NETWORK_STATE state);

/**
 * \brief Callback called when pairing mode is stopped
 *
 * This callback is called when pairing mode is stopped for a finder network.
 * Google Fast Pair is only supported.
 *
 * param [in] fn finder network state at which pairing mode stopped
 */
typedef void (*fn_control_pair_stop_cb_t)(FINDER_NETWORK_STATE state);

/**
 * \brief Callback called when internal BLE attribute database is reset
 */
typedef void (*fn_control_reset_db_cb_t)(void);

/**
 * \brief Callback called when finder network requests the battery level
 *
 * \return battery level (range 0 - 100)
 */
typedef uint8_t (*fn_control_get_batt_level_cb_t)(void);

/**
 * \brief Callback to notify about new authenticated connection for Google Fast Pair FMDN
 *
 * This is called once connection has been authenticated while being Google FMDN provisioned.
 *
 * \param [in] conn_idx connection index
 */
typedef void (*fn_control_fp_auth_conn_cb_t)(uint16_t conn_idx);

/**
 * \brief Finder network control configuration
 */
typedef struct {
        fn_control_state_cb_t state_cb;                         /**< Accessory state callback */
        fn_control_pair_stop_cb_t pair_stop_cb;                 /**< Pairing mode stopped callback (Google Fast Pair) */
        fn_control_reset_db_cb_t db_reset_cb;                   /**< Internal attribute database reset callback */
        fn_control_get_batt_level_cb_t batt_level_get_cb;       /**< Get battery level callback */
        fn_control_fp_auth_conn_cb_t fp_auth_conn_cb;           /**< Google Fast Pair FMDN authenticated connection indication callback */
} fn_control_config_t;

/**
 * \brief Setting pairing mode error
 */
typedef enum {
        FN_CONTROL_PAIRING_ERROR_NONE,
        FN_CONTROL_PAIRING_ERROR_ALREADY_IN_PAIRING,
        FN_CONTROL_PAIRING_ERROR_ALREADY_PAIRED
} FN_CONTROL_PAIRING_ERROR;

/**
 * \brief Initialize finder network control
 *
 * \param [in] cfg configuration of finder network control
 */
void fn_control_init(const fn_control_config_t *cfg);

/**
 * \brief Add structures to finder network scan response data
 *
 * \param [in] structs_count number of scan response structures to add
 * \param [in] structs array of scan response structures
 */
void fn_control_set_scan_response(uint8_t structs_count, const gap_adv_ad_struct_t *structs);

/**
 * \brief Get finder network state
 *
 * \return finder network state
 */
FINDER_NETWORK_STATE fn_control_get_finder_network_state(void);

/**
 * \brief Handle BLE event for finder networks
 *
 * \param [in] evt BLE event
 *
 * \return true if event was handled, false if it needs to be handled by application
 */
bool fn_control_handle_event(ble_evt_hdr_t *evt);

/**
 * \brief Stop ringing
 *
 * This function stops any active ringing.
 *
 * It is applicable only for Google Fast Pair and FMDN.
 *
 * \return 0 if success, other value otherwise
 */
int fn_control_stop_ringing(void);

/**
 * \brief Perform factory reset
 *
 * \return 0 if success, other value otherwise
 */
int fn_control_factory_reset(void);

/**
 * \brief Set pairing mode
 *
 * This function enables or disables pairing mode.
 * Pairing mode can be disabled only for Google Fast Pair.
 *
 * \param [in] enable true to enable pairing mode, false otherwise
 *
 * \return pairing mode change status
 */
FN_CONTROL_PAIRING_ERROR fn_control_set_pairing_mode(bool enable);

/**
 * \brief Check if pairing mode is enabled for any of the finder networks
 *
 * \return true if pairing mode is enabled, false otherwise
 */
bool fn_control_is_pairing_mode(void);

/**
 * \brief Set user consent mode
 *
 * \param [in] enable true to enable user consent mode, false otherwise
 */
void fn_control_set_user_consent(bool enable);

/**
 * \brief Process finder network control task notification
 *
 * Handles finder network processing.
 *
 * \param [in] notif control task notification
 */
void fn_control_process_notif(uint32_t notif);

#endif /* ADV_CONTROL_H_ */
