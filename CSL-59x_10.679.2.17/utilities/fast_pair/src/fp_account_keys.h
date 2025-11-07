/**
 ****************************************************************************************
 *
 * @file fp_account_keys.h
 *
 * @brief Google Fast Pair account keys module header file
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

#ifndef FP_ACCOUNT_KEYS_H_
#define FP_ACCOUNT_KEYS_H_

#include <stdint.h>
#include FAST_PAIR_CONFIG_FILE
#include "fp_defaults.h"
#include "ble_gap.h"
#include "fast_pair.h"

/**
 * \brief Initialize account keys module
 *
 * \return 0 if success, other value otherwise
 */
int fp_acc_keys_init(void);

/**
 * \brief Clean all account keys
 *
 *  All account keys are wiped out from flash storage.
 *
 * \return 0 if success, other value otherwise
 */
int fp_acc_keys_clean(void);

/**
 * \brief Get advertise structure
 *
 * The structure is generated from available account keys. AD type is GAP_DATA_TYPE_UUID16_SVC_DATA
 *
 * \return generated advertise structure
 */
const gap_adv_ad_struct_t *fp_acc_keys_get_advertise_struct(void);

/**
 * \brief Schedule new advertise structure generation
 *
 * The function should be called whenever data used to generate fast pair advertise structure has changed
 */
void fp_acc_keys_schedule_new_advertise_struct(void);

/**
 * \brief Generate a new salt which will be used in advertise payload and bloom filter
 */
void fp_acc_keys_generate_new_salt(void);

/**
 * \brief Get account key
 *
 * \param [in] index account key index
 *
 * \return pointer to account key or NULL if not available
 */
uint8_t *fp_acc_keys_get_key(uint8_t index);

/**
 * \brief Get number of stored account keys
 *
 * \return number of available account keys
 */
uint8_t fp_acc_keys_get_keys_count(void);

/**
 * \brief Add a new account key to persistent storage
 *
 * If FP_ACCOUNT_KEYS_COUNT is reached the least used key is overwritten.
 *
 * \param [in] key account key to add
 */
void fp_acc_keys_add_key(const uint8_t *key);

/**
 * \brief Update account key usage
 *
 * If account key is used, it is moved up in the stored list and will not be overwritten with
 * fp_acc_keys_add_key().
 *
 * \param [in] index index of the account key
 */
void fp_acc_keys_update_key_usage(uint8_t index);

#if (FP_FMDN == 1)
/**
 * \brief Get ephemeral identity key
 *
 * \return pointer to ephemeral identity key or NULL if not available
 */
uint8_t *fp_acc_keys_get_ephemeral_identity_key(void);

/**
 * \brief Set ephemeral identity key
 *
 * \param [in] identity_key identity key to add to persistent storage
 *
 *
 *
 */
int fp_acc_keys_set_ephemeral_identity_key(const uint8_t *identity_key);

/**
 * \brief Clear stored ephemeral identity key
 *
 * \return 0 if success, other value otherwise
 */
int fp_acc_keys_clear_ephemeral_identity_key(void);

/**
 * \brief Get owner account key
 *
 * \return pointer to account key or NULL if not available
 */
uint8_t *fp_acc_keys_get_owner_key(void);
#endif /* FP_FMDN */

#endif /* FP_ACCOUNT_KEYS_H_ */
