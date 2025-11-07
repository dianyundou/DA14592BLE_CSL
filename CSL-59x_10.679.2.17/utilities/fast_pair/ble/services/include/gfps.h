/**
 ****************************************************************************************
 *
 * @file gfps.h
 *
 * @brief Google Fast Pair Service implementation API
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

#ifndef GFPS_H_
#define GFPS_H_

#include <stdbool.h>
#include <stdint.h>
#include "ble_service.h"
#include FAST_PAIR_CONFIG_FILE
#include "fp_defaults.h"

#if (FP_FMDN == 1)
/**
 * \brief GATT error codes for beacon actions
 */
typedef enum {
        ATT_ERROR_UNAUTHENTICATED = ATT_ERROR_APPLICATION_ERROR,        /**< Unauthenticated Error */
        ATT_ERROR_INVALID_VALUE,                                        /**< Invalid Value Error */
        ATT_ERROR_NO_USER_CONSENT,                                      /**< No User Consent Error */
} fast_pair_error_t;
#endif /* FP_FMDN */

typedef void (*pairing_cb_t)(ble_service_t *svc, uint16_t conn_idx, const uint8_t *request,
                                                                        const uint8_t *public_key);
typedef void (*passkey_cb_t)(ble_service_t *svc, uint16_t conn_idx, const uint8_t *passkey_block);
typedef void (*additional_data_cb_t)(uint16_t conn_idx, const uint8_t *data, uint8_t length);
typedef void (*accountkey_cb_t)(uint16_t conn_idx, const uint8_t *keybuffer);
#if (FP_FMDN == 1)
typedef void (*beacon_actions_read_cb_t)(ble_service_t *svc, uint16_t conn_idx, uint8_t *response);
typedef fast_pair_error_t (*beacon_actions_write_cb_t)(ble_service_t *svc, uint16_t conn_idx,
                                                        const uint8_t *value, uint8_t length);
#endif /* FP_FMDN */

/**
 * \brief GFPS application callbacks
 */
typedef struct {
        /** Key-based pairing request callback */
        pairing_cb_t pairing_cb;
        /** Passkey write callback */
        passkey_cb_t passkey_cb;
        /** Additional data write callback */
        additional_data_cb_t additional_data_cb;
        /** Account key write callback */
        accountkey_cb_t accountkey_cb;
#if (FP_FMDN == 1)
        /** Beacon actions read callback */
        beacon_actions_read_cb_t beacon_actions_read_cb;
        /** Beacon actions write callback */
        beacon_actions_write_cb_t beacon_actions_write_cb;
#endif
} fast_pair_callbacks_t;

/**
 * \brief GFPS information structure
 */
typedef struct {
        uint32_t model_id;              /**< Google Fast Pair model ID */
} gfps_info_t;


/**
 * \brief Register Google Fast Pair Service instance
 *
 * Function registers GFPS instance.
 * \param [in] pointer to service information structure
 * \param [in] cb pointer to service callbacks
 *
 * \return service instance
 */
ble_service_t *gfps_init(const gfps_info_t *info, const fast_pair_callbacks_t *cb);

/**
 * \brief Notify client with pairing response
 *
 * \param [in] svc      service instance
 * \param [in] conn_idx connection index
 * \param [in] response pairing response message
 *
 * \return result code
 */
ble_error_t gfps_notify_pairing(ble_service_t *svc, uint16_t conn_idx, uint8_t *response);

/**
 * \brief Notify client with passkey block
 *
 * \param [in] svc           service instance
 * \param [in] conn_idx      connection index
 * \param [in] passkey_block encrypted passkey block
 *
 * \return result code
 */
ble_error_t gfps_notify_passkey(ble_service_t *svc, uint16_t conn_idx, uint8_t *passkey_block);

/**
 * \brief Notify client with additional data
 *
 * \param [in] svc           service instance
 * \param [in] conn_idx      connection index
 * \param [in] data          encrypted additional data
 * \param [in] length        data's length
 *
 * \return result code
 */
ble_error_t gfps_notify_additional_data(ble_service_t *svc, uint16_t conn_idx, uint8_t *data,
                                                                                    uint8_t length);

#if (FP_FMDN == 1)
/**
 * \brief Notify client with beacon actions
 *
 * \param [in] svc           service instance
 * \param [in] conn_idx      connection index
 * \param [in] data          data
 * \param [in] length        data's length
 *
 * \return result code
 */
ble_error_t gfps_notify_beacon_actions(ble_service_t *svc, uint16_t conn_idx, uint8_t *data, uint8_t length);
#endif /* FP_FMDN */

#endif /* GFPS_H_ */
