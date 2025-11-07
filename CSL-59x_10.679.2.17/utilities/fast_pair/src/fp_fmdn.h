/**
 ****************************************************************************************
 *
 * @file fp_fmdn.h
 *
 * @brief Google Fast Pair Find My Device Network (FMDN) extension module header file
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

#ifndef FP_FMDN_H_
#define FP_FMDN_H_

#include <stdint.h>
#include FAST_PAIR_CONFIG_FILE
#include "fp_defaults.h"

#if (FP_FMDN == 1)
#include "ble_service.h"
#include "gfps.h"
#include "fast_pair.h"
#include "fp_ring_comp.h"

/**
 * \brief FMDN ringing state
 */
typedef enum {
        FP_FMDN_RING_STARTED,
        FP_FMDN_RING_FAILED,
        FP_FMDN_RING_TIMEOUT_STOPPED,
        FP_FMDN_RING_BUTTON_STOPPED,
        FP_FMDN_RING_REQUESTED_STOPPED,
} FP_FMDN_RING_STATE;

/**
 * \brief FMDN module configuration structure
 */
typedef struct {
        fp_beacon_time_cb_t beacon_time_cb;     /**< Beacon time callback */
} fp_fmdn_cfg_t;

/**
* \brief Callback to notify ringing is complete
*/
typedef void (*fp_fmdn_ringing_complete_cb_t)(void);

/**
 * \brief Initialize FMDN module
 *
 * \param [in] cfg configuration structure
 *
 * \return 0 if success, other value otherwise
 */
int fp_fmdn_init(const fp_fmdn_cfg_t *cfg);

/**
 * \brief De-initialize FMDN module
 *
 * This function releases allocated resources for FMDN module.
 */
void fp_fmdn_deinit(void);

/**
 * \brief Check the unwanted tracking protection mode state
 *
 * \return 0 if success, other value otherwise
 */
bool fp_fmdn_is_utpm_active(void);

/**
 * \brief Beacon actions characteristic write callback
 *
 * \param [in] svc BLE service instance
 * \param [in] conn_idx connection index
 * \param [in] request request buffer
 * \param [in] length request buffer length
 *
 * \return Fast Pair characteristic error
 */
fast_pair_error_t fp_fmdn_beacon_actions_write_cb(ble_service_t *svc, uint16_t conn_idx,
        const uint8_t *request, uint8_t length);

/**
 * \brief Beacon actions characteristic read callback
 *
 * \param [in] svc BLE service instance
 * \param [in] conn_idx connection index
 * \param [in] response 8-byte response
 */
void fp_fmdn_beacon_actions_read_cb(ble_service_t *svc, uint16_t conn_idx, uint8_t *response);

/**
 * \brief Generate FMDN advertise structure
 *
 * \return FMDN advertise struct
 */
const gap_adv_ad_struct_t *fp_fmdn_advertise_struct(void);

/**
 * \brief Schedule new advertise structure generation
 *
 * The function should be called whenever data used to generate FMDN advertise structure has changed
 */
void fp_fmdn_schedule_new_advertise_struct(void);

/**
 * \brief Initiate ID rotation
 *
 * \return 0 if success, other value otherwise
 */
int fp_fmdn_rotate_id(void);

/**
 * \brief Perform address rotation for unwanted tracking protection mode
 *
 * \return 0 if success, other value otherwise
 */
int fp_fmdn_rotate_utpm_address(void);

/**
 * \brief Perform ID rotation in progress
 */
void fp_fmdn_perform_id_rotation_in_progress(void);

/**
 * \brief Check if ID rotation is pending
 *
 * \return true if rotation is pending, false otherwise
 */
bool fp_fmdn_is_id_rotation_pending(void);

/**
 * \brief Check if unwanted tracking protection mode address rotation is pending
 *
 * \return true if rotation is pending, false otherwise
 */
bool fp_fmdn_is_utpm_address_rotation_pending(void);

/**
 * \brief Set user consent mode
 *
 * \param [in] enable true to enable user consent mode, false otherwise
 */
void fp_fmdn_set_user_consent(bool enable);

/**
 * \brief Check user consent mode
 *
 * \return true if user consent mode is enabled, false otherwise
 */
bool fp_fmdn_is_user_consent_mode(void);

/**
 * \brief Wipe out stored account keys if needed
 *
 * If ephemeral identity key is cleared, then when Seeker disconnects,
 * account keys must be wiped out.
 */
void fp_fmdn_wipe_out_account_keys(void);

#if (FP_LOCATOR_TAG == 1)
/**
 * \brief Enable/disable provisioning timer
 *
 * Timeout period for provisioning to be completed.
 *
 * \param [in] enable true to enable provisioning timeout timer
 */
void fp_fmdn_set_provisioning_timer(bool enable);
#endif /* FP_LOCATOR_TAG */

#if (FP_FMDN_RING_COMPONENTS_NUM > 0)
/**
 * \brief Stop ringing
 *
 * \param [in] state ringing state
 *
 * \return 0 if success, other value otherwise
 */
int fp_fmdn_stop_ringing(FP_FMDN_RING_STATE state);

/**
 * \brief Start ringing
 *
 * \param [in] dur_ms ringing duration in msec
 * \param [in] volume ringing volume
 * \param [in] cb ringing complete callback
 *
 * \return 0 if success, other value otherwise
 */
int fp_fmdn_start_ringing(uint32_t dur_ms, FP_RING_COMP_VOLUME volume,
        fp_fmdn_ringing_complete_cb_t cb);

/**
 * \brief Check if ringing is enabled
 *
 * \return true if ringing is enabled, false otherwise
 */
bool fp_fmdn_is_ringing(void);
#endif /* FP_FMDN_RING_COMPONENTS_NUM */

/**
 * \brief Remove connection over which ringing request has been sent
 *
 * Having removed the connection, no responses will be sent when ringing state changes.
 *
 * \param [in] conn_idx connection index
 */
void fp_fmdn_remove_ring_connection(uint16_t conn_idx);

/**
 * \brief Get current ephemeral identifier
 *
 * \return pointer to current ephemeral identifier buffer
 */
uint8_t *fp_fmdn_get_eid(void);

/**
 * \brief Get current recovery key
 *
 * \param [out] generated recovery key, 8-byte in length
 * \return true if recovery key is available
 */
bool fp_fmdn_get_recovery_key(uint8_t *recovery_key);
#endif /* FP_FMDN */

#endif /* FP_FMDN_H_ */
