/**
 * \addtogroup MID_SYS_ADAPTERS
 * \{
 * \addtogroup CRYPTO_ADAPTER Crypto Engines adapter
 *
 * \brief Cryptographic algorithms (AES/HASH) adapter
 *
 * \{
 */

/**
 ****************************************************************************************
 *
 * @file ad_crypto.h
 *
 * @brief AES/HASH device access API
 *
 * Copyright (C) 2016-2023 Renesas Electronics Corporation and/or its affiliates.
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

#ifndef AD_CRYPTO_H_
#define AD_CRYPTO_H_

#if dg_configCRYPTO_ADAPTER

#if dg_configUSE_HW_AES
#include "hw_aes.h"
#endif
#if dg_configUSE_HW_HASH
#include "hw_hash.h"
#endif

#include "hw_aes_hash.h"

#include <osal.h>

/**
 * \brief Crypto handle returned by ad_crypto_open()
 */
typedef void *ad_crypto_handle_t;

/**
 * \brief       Supported cryptographic algorithms.
 */
typedef enum {
#if dg_configUSE_HW_AES
        AD_CRYPTO_ALGO_AES = 0,
#endif

#if dg_configUSE_HW_HASH
        AD_CRYPTO_ALGO_HASH = 1,
#endif
} AD_CRYPTO_ALGO;

/**
 * \brief       Crypto configuration union
 */
typedef union {
#if dg_configUSE_HW_AES
        hw_aes_config_t aes;            /**< AES engine configuration structure */
#endif

#if dg_configUSE_HW_HASH
        hw_hash_config_t hash;          /**< HASH engine configuration structure */
#endif
} ad_crypto_engine_config_t;

/**
 * \brief       Crypto adapter configuration structure
 */
typedef struct {
        AD_CRYPTO_ALGO algo;                    /**< Cryptographic algorithm */
        ad_crypto_engine_config_t engine;       /**< AES/HASH engine configuration union */
} ad_crypto_config_t;

/**
 * \brief       Configure the crypto engine.
 *
 * A new crypto operation is configured by ad_crypto_open(). Use this function to re-configure
 * the crypto engine.
 *
 * \param [in]  cfg Crypto engine's configuration structure pointer.
 *
 * \warning     Never call this function before having first called the ad_crypto_open().
 *
 * \sa ad_crypto_config_t
 */
void ad_crypto_configure(ad_crypto_config_t *cfg);

/**
 * \brief       Configure crypto engine to perform next fragment of operation.
 *
 * Configure the next essential parameters of the crypto engine in order to proceed to the next
 * fragment of a crypto operation:
 *
 * - Input data mode (AES/HASH).
 * - Input data address (AES/HASH).
 * - Input data length (AES/HASH).
 * - Output data address (AES).
 *
 * \param [in] cfg Crypto engine's configuration structure pointer.
 *
 * \sa ad_crypto_config_t
 */
void ad_crypto_configure_for_next_fragment(ad_crypto_config_t *cfg);

/**
 * \brief       Open crypto engine.
 *
 * This function:
 * - Acquires the necessary resources to perform a crypto operation.
 * - Configures the crypto engine.
 *
 * \param [in] cfg Crypto engine's configuration structure pointer.
 * \param [in] timeout Expected time in OS ticks to acquire the resources and configure the engine.
 *
 * \return Handle to be used in subsequent API calls. Valid: return > 0, Invalid: return = NULL.
 *
 * \sa ad_crypto_config_t
 *
 */
ad_crypto_handle_t ad_crypto_open(ad_crypto_config_t *cfg, OS_TICK_TIME timeout);

/**
 * \brief       Close crypto engine.
 *
 * This function:
 * - Clears the pending crypto interrupts.
 * - Disables the crypto interrupts.
 * - Disables the crypto clock.
 * - Releases the resources reserved to perform operation.
 *
 */
void ad_crypto_close(void);

/**
 * \brief       Perform crypto operation.
 *
 * \param [in] timeout Maximum expected time in OS ticks to perform operation.
 *
 * \return OS_EVENT_SIGNALED, if the operation completed, other value if not.
 */
OS_BASE_TYPE ad_crypto_perform_operation(OS_TICK_TIME timeout);

#endif /* dg_configCRYPTO_ADAPTER */

#endif /* AD_CRYPTO_H_ */

/**
 * \}
 * \}
 */

