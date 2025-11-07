/**
 ****************************************************************************************
 *
 * @file ad_crypto.c
 *
 * @brief AES/HASH device access API implementation
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

#if dg_configCRYPTO_ADAPTER

#if dg_configUSE_HW_AES
#include "hw_aes.h"
#endif
#if dg_configUSE_HW_HASH
#include "hw_hash.h"
#endif

#include "hw_aes_hash.h"

#include "ad_crypto.h"
#include "sys_power_mgr.h"
#include "osal.h"

#define CRYPTO_MUTEX_CREATE(mutex)                      do { \
                                                                OS_ASSERT((mutex) == NULL); \
                                                                OS_MUTEX_CREATE(mutex); \
                                                                OS_ASSERT(mutex); \
                                                        } while (0)

#define CRYPTO_MUTEX_GET(mutex)                         do { \
                                                                OS_ASSERT(mutex); \
                                                                OS_MUTEX_GET((mutex), OS_MUTEX_FOREVER); \
                                                        } while (0)

#define CRYPTO_MUTEX_GET_TIMEOUT(mutex, timeout)        ({ \
                                                                OS_BASE_TYPE ret; \
                                                                OS_ASSERT(mutex); \
                                                                ret = OS_MUTEX_GET((mutex), (timeout)); \
                                                                ret; \
                                                        })

#define CRYPTO_MUTEX_PUT(mutex)                         OS_MUTEX_PUT(mutex)

typedef struct {
        const ad_crypto_config_t *cfg;  /**< Crypto adapter current configuration */
        OS_EVENT event;                 /**< Event for asynchronous operations  */
        OS_MUTEX mutex;                 /**< Mutex for thread safety */
} ad_crypto_data_t;

__RETAINED static ad_crypto_data_t aes_hash_data;
__RETAINED static hw_aes_hash_cb aes_hash_user_cb = NULL;

static void aes_hash_irq_cb(uint32_t status)
{

        if (aes_hash_user_cb != NULL) {
                aes_hash_user_cb(status);
        }

        // If the AES/HASH engine is configured to wait more input data, check
        // the CRYPTO_STATUS_REG->CRYPTO_WAIT_FOR_IN to signal the ISR event.
        if (hw_aes_hash_get_input_data_mode()) {
                if (status & HW_AES_HASH_IRQ_MASK_WAITING_FOR_INPUT) {
                        OS_EVENT_SIGNAL_FROM_ISR(aes_hash_data.event);
                }
        // Otherwise check the CRYPTO_STATUS_REG->CRYPTO_IRQ_ST.
        } else {
                if (status & HW_AES_HASH_IRQ_MASK_INACTIVE) {
                        OS_EVENT_SIGNAL_FROM_ISR(aes_hash_data.event);
                }
        }
}

static void ad_crypto_init(void)
{
        CRYPTO_MUTEX_CREATE(aes_hash_data.mutex);
        OS_EVENT_CREATE(aes_hash_data.event);
        OS_ASSERT(aes_hash_data.event);
}

void ad_crypto_configure(ad_crypto_config_t *cfg)
{
        OS_ASSERT(aes_hash_data.mutex != NULL);

        switch (cfg->algo) {
#if dg_configUSE_HW_AES
        case AD_CRYPTO_ALGO_AES:
                ASSERT_WARNING(hw_aes_init(&cfg->engine.aes) == HW_AES_ERROR_NONE);
                break;
#endif

#if dg_configUSE_HW_HASH
        case AD_CRYPTO_ALGO_HASH:
                ASSERT_WARNING(hw_hash_init(&cfg->engine.hash) == HW_HASH_ERROR_NONE);
                break;
#endif
        default:
                ASSERT_WARNING(0);
        }
}

void ad_crypto_configure_for_next_fragment(ad_crypto_config_t *cfg)
{
        OS_ASSERT(aes_hash_data.mutex != NULL);

        switch (cfg->algo) {
#if dg_configUSE_HW_AES
        case AD_CRYPTO_ALGO_AES:
                hw_aes_hash_set_input_data_mode(cfg->engine.aes.wait_more_input);
                hw_aes_hash_set_input_data_addr(cfg->engine.aes.input_data_addr);
                hw_aes_hash_set_input_data_len(cfg->engine.aes.input_data_len);
                hw_aes_set_output_data_mode(cfg->engine.aes.output_data_mode);
                break;
#endif

#if dg_configUSE_HW_HASH
        case AD_CRYPTO_ALGO_HASH:
                hw_aes_hash_set_input_data_mode(cfg->engine.hash.wait_more_input);
                hw_aes_hash_set_input_data_addr(cfg->engine.hash.input_data_addr);
                hw_aes_hash_set_input_data_len(cfg->engine.hash.input_data_len);
                break;
#endif
        default:
                ASSERT_WARNING(0);
        }
}

ad_crypto_handle_t ad_crypto_open(ad_crypto_config_t *cfg, OS_TICK_TIME timeout)
{
        OS_ASSERT(cfg);

        ad_crypto_data_t *crypto = &aes_hash_data;

        crypto->cfg = cfg;

        OS_BASE_TYPE mutex_ret;
        mutex_ret = CRYPTO_MUTEX_GET_TIMEOUT(crypto->mutex, timeout);

        if (mutex_ret == OS_MUTEX_TAKEN) {
                // The crypto engine is expected to be unlocked.
                OS_ASSERT(hw_aes_hash_get_status() == HW_AES_HASH_STATUS_UNLOCKED);
                pm_sleep_mode_request(pm_mode_idle);

                // Clear events
                OS_EVENT_CHECK(aes_hash_data.event);

                // Register the callback passed by the user and set aes_hash_irq_cb() as AES/HASH
                // callback to enable crypto adapter's signaling. The user callback is called by
                // aes_hash_irq_cb().
                switch (cfg->algo) {
#if dg_configUSE_HW_AES
                case AD_CRYPTO_ALGO_AES:
                        aes_hash_user_cb = cfg->engine.aes.callback;
                        cfg->engine.aes.callback = aes_hash_irq_cb;
                        break;
#endif

#if dg_configUSE_HW_HASH
                case AD_CRYPTO_ALGO_HASH:
                        aes_hash_user_cb = cfg->engine.hash.callback;
                        cfg->engine.hash.callback = aes_hash_irq_cb;
                        break;
#endif
                default:
                        ASSERT_WARNING(0);
                }

                ad_crypto_configure(cfg);
        }
        else {
                return NULL;
        }

        return (ad_crypto_handle_t) crypto;
}

void ad_crypto_close(void)
{
        aes_hash_user_cb = NULL;
        hw_aes_hash_deinit();
        // Clear events
        OS_EVENT_CHECK(aes_hash_data.event);

        OS_MUTEX_PUT(aes_hash_data.mutex);

        pm_sleep_mode_release(pm_mode_idle);

}

OS_BASE_TYPE ad_crypto_perform_operation(OS_TICK_TIME timeout)
{
        OS_BASE_TYPE event_status;

        OS_ASSERT(aes_hash_data.mutex != NULL);

        hw_aes_hash_start();

        event_status = OS_EVENT_WAIT(aes_hash_data.event, timeout);

        return event_status;
}

ADAPTER_INIT(crypto_adapter, ad_crypto_init);

#endif /* dg_configCRYPTO_ADAPTER */
