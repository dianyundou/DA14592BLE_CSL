/**
 ****************************************************************************************
 *
 * @file hw_hash.c
 *
 * @brief Implementation of the HASH Engine Low Level Driver.
 *
 * Copyright (C) 2023 Renesas Electronics Corporation and/or its affiliates.
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
#if dg_configUSE_HW_HASH

#include "hw_hash.h"
#include "hw_aes_hash.h"

bool hw_hash_check_input_data_len_restrictions(void)
{
        bool wait_more_input = hw_aes_hash_get_input_data_mode();
        uint32_t data_len = hw_aes_hash_get_input_data_len();

        if (wait_more_input && (data_len % 0x08)) {
                ASSERT_WARNING(0);
                return false;
        }

        return true;
}

HW_HASH_ERROR hw_hash_init(const hw_hash_config_t *hash_cfg)
{
        HW_AES_HASH_STATUS status;

        // Critical section to avoid race condition
        GLOBAL_INT_DISABLE();
        status = hw_aes_hash_get_status();

        if (status != HW_AES_HASH_STATUS_LOCKED_BY_AES) {
                // Use direct register access instead of the hw_aes_hash_enable_clock()
                // to avoid nested critical section due to the function call.
                REG_SET_BIT(CRG_TOP, CLK_AMBA_REG, AES_CLK_ENABLE);
                hw_hash_set_type(hash_cfg->type);
        }
        GLOBAL_INT_RESTORE();

        // This check has to take place outside the critical section, because an else condition
        // would end up to return from the function without restoring the interrupts.
        if (status == HW_AES_HASH_STATUS_LOCKED_BY_AES) {
                return HW_HASH_ERROR_CRYPTO_ENGINE_LOCKED;
        }

        hw_aes_hash_set_input_data_mode(hash_cfg->wait_more_input);
        hw_aes_hash_set_input_data_len(hash_cfg->input_data_len);
        hw_hash_set_output_data_len(hash_cfg->type, hash_cfg->output_data_len);
        hw_aes_hash_set_input_data_addr(hash_cfg->input_data_addr);
        hw_aes_hash_set_output_data_addr(hash_cfg->output_data_addr);

        if (hash_cfg->callback == NULL) {
                hw_aes_hash_interrupt_disable();
        } else {
                hw_aes_hash_interrupt_enable(hash_cfg->callback);
        }

        if (!hw_hash_check_input_data_len_restrictions()) {
                return HW_HASH_ERROR_INVALID_INPUT_DATA_LEN;
        }

        return HW_HASH_ERROR_NONE;
}

#endif /* dg_configUSE_HW_HASH */

