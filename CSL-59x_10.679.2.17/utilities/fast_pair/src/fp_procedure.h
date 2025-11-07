/**
 ****************************************************************************************
 *
 * @file fp_procedure.h
 *
 * @brief Google Fast Pair procedure module header file
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

#ifndef FP_PROCEDURE_H_
#define FP_PROCEDURE_H_

#include <stdint.h>
#include <stdbool.h>
#include FAST_PAIR_CONFIG_FILE
#include "fp_defaults.h"
#include "ble_common.h"
#include "ble_service.h"
#include "fast_pair.h"

/**
 * \brief Callback to notify Fast Pair procedure status
 *
 * This is called when Fast Pair procedure is initiated or completed.
 *
 * \param [in] conn_idx connection index
 * \param [in] stat Fast Pair procedure status
 * \param [in] err Fast Pair procedure error (0: no error)
 */
typedef void (*fp_procedure_status_cb_t)(uint16_t conn_idx, FP_PAIR_REQ_STAT stat, uint8_t err);

/**
 * \brief Initialize Fast Pair procedure module
 *
 * \param [in] procedure_status_cb callback to notify about Fast Pair procedure status
 */
void fp_procedure_init(fp_procedure_status_cb_t procedure_status_cb);

/**
 * \brief De-initialize Fast Pair procedure module
 *
 * This function releases allocated resources for procedure module.
 */
void fp_procedure_deinit(void);

/**
 * \brief Stop current Fast Pair procedure for a connection
 *
 * \param [in] conn_idx connection index
 *
 * \note If BLE_CONN_IDX_INVALID is given as connection index, the connection of the current
 *       procedure session (if any) is implied.
 */
void fp_procedure_stop(uint16_t conn_idx);

/**
 * \brief Set pairing mode
 *
 * \param [in] enable true to enable, false otherwise
 */
void fp_procedure_set_pairing_mode(bool enable);

/**
 * \brief Check if pairing mode is enabled
 *
 * \return true if pairing mode is enabled, false otherwise
 */
bool fp_procedure_is_pairing_mode(void);

/**
 * \brief Account key characteristic write callback
 *
 * Account key characteristic is used for writing account key to Provider's storage.
 *
 * \param [in] conn_idx connection index
 * \param [in] keybuffer account key to be stored
 */
void fp_procedure_account_key_cb(uint16_t conn_idx, const uint8_t *keybuffer);

/**
 * \brief Key-based paring characteristic write callback
 *
 * Key-based paring characteristic is used for starting Fast Pair procedure.
 *
 * \param [in] svc BLE service instance
 * \param [in] conn_idx connection index
 * \param [in] request request buffer (16-bytes)
 * \param [in] public_key public key to be used to authenticate
 */
void fp_procedure_pairing_cb(ble_service_t *svc, uint16_t conn_idx, const uint8_t *request,
        const uint8_t *public_key);

#if (FP_LOCATOR_TAG != 1)
/**
 * \brief Passkey characteristic write callback
 *
 * Passkey characteristic is used for providing passkey for comparison.
 *
 * \param [in] svc BLE service instance
 * \param [in] conn_idx connection index
 * \param [in] passkey_block passkey block
 */
void fp_procedure_passkey_cb(ble_service_t *svc, uint16_t conn_idx, const uint8_t *passkey_block);
#endif /* !FP_LOCATOR_TAG */

/**
 * \brief Additional data characteristic write callback
 *
 * Additional data characteristic is used for additional data, like personalized name.
 *
 * \param [in] conn_idx connection index
 * \param [in] data buffer containing additional data
 * \param [in] length length of data buffer
 */
void fp_procedure_additional_data_cb(uint16_t conn_idx, const uint8_t *data, uint8_t length);

#if (FP_LOCATOR_TAG != 1)
/**
 * \brief Save the received passkey
 *
 * This passkey is going to be compared with passkey block received later on.
 *
 * \param [in] passkey passkey
 */
void fp_procedure_save_passkey(uint32_t passkey);
#endif /* !FP_LOCATOR_TAG */

/**
 * \brief Verify if a connection is the one that has started pairing procedure
 *
 * \param [in] conn_idx connection index
 *
 * \return true if pairing procedure has been started by the indicated connection
 */
bool fp_procedure_is_pairing_connection(uint16_t conn_idx);

#endif /* 1FP_PROCEDURE_H_ */
