/**
 ****************************************************************************************
 *
 * @file hw_aes.c
 *
 * @brief Implementation of the AES Engine Low Level Driver.
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
#if dg_configUSE_HW_AES

#include "hw_aes.h"
#include "hw_aes_hash.h"
#include "hw_dma.h"
#include "hw_fcu.h"

static bool is_key_address_within_valid_nvm_range(uint32_t key_addr)
{
        if (WITHIN_RANGE(key_addr, (MEMORY_EFLASH_S_BASE + MEMORY_EFLASH_USER_DATA_KEYS_BASE),
                                   (MEMORY_EFLASH_S_BASE + MEMORY_EFLASH_USER_DATA_KEYS_END))) {
                return true;
        }
        return false;
}

static uint8_t calculate_key_words_len(HW_AES_KEY_SIZE key_size, HW_AES_KEY_EXPAND key_exp)
{
        switch (key_exp) {
        case HW_AES_KEY_EXPAND_BY_HW:
                return (key_size == HW_AES_KEY_SIZE_256) ? 8 :
                       (key_size == HW_AES_KEY_SIZE_192) ? 6 : 4;
        case HW_AES_KEY_EXPAND_BY_SW:
                return (key_size == HW_AES_KEY_SIZE_256) ? 60 :
                       (key_size == HW_AES_KEY_SIZE_192) ? 52 : 44;
        default:
                ASSERT_WARNING(0);
                return 0;
        }
}

#if dg_configUSE_HW_DMA
// Transfer the AES Key from NVM to Crypto engine using the secure DMA channel
static void secure_key_transfer_from_nvm(uint32_t key_src_addr, uint8_t key_words)
{
        DMA_setup aes_dma_setup;

        /* Init DMA channel */
        aes_dma_setup.channel_number = HW_DMA_SECURE_DMA_CHANNEL;
        aes_dma_setup.bus_width = HW_DMA_BW_WORD;
        aes_dma_setup.irq_enable = HW_DMA_IRQ_STATE_DISABLED;
        aes_dma_setup.irq_nr_of_trans = 0;
        aes_dma_setup.dreq_mode = HW_DMA_DREQ_START;
        aes_dma_setup.burst_mode = HW_DMA_BURST_MODE_DISABLED;
        aes_dma_setup.a_inc = HW_DMA_AINC_TRUE;
        aes_dma_setup.b_inc = HW_DMA_BINC_TRUE;
        aes_dma_setup.circular = HW_DMA_MODE_NORMAL;
        aes_dma_setup.dma_prio = HW_DMA_PRIO_7;
        aes_dma_setup.dma_idle = HW_DMA_IDLE_BLOCKING_MODE;
        aes_dma_setup.dma_init = HW_DMA_INIT_AX_BX_AY_BY;
        aes_dma_setup.dma_req_mux = HW_DMA_TRIG_NONE;
        aes_dma_setup.src_address = key_src_addr;
        aes_dma_setup.dest_address = (uint32_t) &AES_HASH->CRYPTO_KEYS_START;
        aes_dma_setup.length = (dma_size_t) key_words;
        aes_dma_setup.callback = NULL;
        aes_dma_setup.user_data = NULL;

        /* Transfer key from NVM to CryptoEngine */
        hw_dma_channel_initialization(&aes_dma_setup);
        hw_dma_channel_enable(HW_DMA_SECURE_DMA_CHANNEL, HW_DMA_STATE_ENABLED);

        /* wait for transaction to finish */
        while (hw_dma_is_channel_active(HW_DMA_SECURE_DMA_CHANNEL));
}
#endif

static void non_secure_key_transfer_from_nvm(uint32_t key_src_addr, uint8_t key_words)
{
        uint32 rel_addr = (uint32_t)key_src_addr - MEMORY_EFLASH_S_BASE;
        uint32 *crypto_keys_ptr = (uint32 *) &AES_HASH->CRYPTO_KEYS_START;
        hw_fcu_read(rel_addr, crypto_keys_ptr, (uint32_t)key_words, NULL);
}

bool hw_aes_check_input_data_len_restrictions(void)
{
        HW_AES_MODE aes_mode = hw_aes_get_mode();
        bool wait_more_input = hw_aes_hash_get_input_data_mode();
        uint32_t data_len = hw_aes_hash_get_input_data_len();

        switch (aes_mode) {
        case HW_AES_MODE_ECB:
                if (data_len % 0x10) {
                        ASSERT_WARNING(0);
                        return false;
                }
                break;
        case HW_AES_MODE_CBC:
        case HW_AES_MODE_CTR:
                if (wait_more_input && (data_len % 0x10)) {
                        ASSERT_WARNING(0);
                        return false;
                }
                break;
        default:
                ASSERT_WARNING(0);
                return false;
        }

        return true;
}

void hw_aes_set_init_vector(const uint8_t* iv_cnt_ptr)
{
        AES_HASH->CRYPTO_MREG0_REG = SWAP32(__UNALIGNED_UINT32_READ((const uint8_t *) (iv_cnt_ptr + 12)));
        AES_HASH->CRYPTO_MREG1_REG = SWAP32(__UNALIGNED_UINT32_READ((const uint8_t *) (iv_cnt_ptr + 8)));
        AES_HASH->CRYPTO_MREG2_REG = SWAP32(__UNALIGNED_UINT32_READ((const uint8_t *) (iv_cnt_ptr + 4)));
        AES_HASH->CRYPTO_MREG3_REG = SWAP32(__UNALIGNED_UINT32_READ(iv_cnt_ptr));
}

bool hw_aes_is_key_valid(uint8_t idx)
{
        uint32_t key_index;

        if (idx >= HW_AES_USER_DATA_KEYS_MAX_ENTRIES) {
                return false;
        }

        hw_fcu_read((MEMORY_EFLASH_USER_DATA_KEYS_REVOCATION_BASE + idx * 4), &key_index, 1, NULL);

        if (key_index == 0xFFFFFFFF) {
                return true;
        }

        return false;
}

uint32_t hw_aes_key_address_get(uint8_t key_idx)
{
        if (!hw_aes_is_key_valid(key_idx)) {
                return 0;
        }

        return (MEMORY_EFLASH_S_BASE + MEMORY_EFLASH_USER_DATA_KEYS_BASE +
                ((uint32_t)key_idx * MEMORY_EFLASH_USER_DATA_KEY_SIZE));
}

void hw_aes_load_keys(uint32_t key_src_addr, HW_AES_KEY_SIZE key_size, HW_AES_KEY_EXPAND key_exp)
{
        uint8_t key_wrds;

        key_wrds = calculate_key_words_len(key_size, key_exp);

        if (is_key_address_within_valid_nvm_range(key_src_addr)) {
                /* Key expansion has to be performed by the engine */
                ASSERT_WARNING(key_exp == HW_AES_KEY_EXPAND_BY_HW);

#if dg_configUSE_HW_DMA
                if (hw_dma_is_aes_key_protection_enabled()) {
                        secure_key_transfer_from_nvm(key_src_addr, key_wrds);
                } else {
#endif
                        non_secure_key_transfer_from_nvm(key_src_addr, key_wrds);
#if dg_configUSE_HW_DMA
                }
#endif
        } else {
                volatile uint32_t *key_dst_ptr = &AES_HASH->CRYPTO_KEYS_START;

                do {
                        *key_dst_ptr = SWAP32(__UNALIGNED_UINT32_READ(key_src_addr));
                        ++key_dst_ptr;
                        key_src_addr += sizeof(uint32_t);
                        key_wrds--;
                } while (key_wrds > 0);
        }
}

HW_AES_ERROR hw_aes_init(const hw_aes_config_t *aes_cfg)
{
        ASSERT_WARNING(aes_cfg);

        ASSERT_WARNING((aes_cfg->operation == HW_AES_OPERATION_DECRYPT) ||
                       (aes_cfg->operation == HW_AES_OPERATION_ENCRYPT));

        ASSERT_WARNING((aes_cfg->key_size == HW_AES_KEY_SIZE_128) || (aes_cfg->key_size == HW_AES_KEY_SIZE_192) ||
                       (aes_cfg->key_size == HW_AES_KEY_SIZE_256));

        ASSERT_WARNING((aes_cfg->key_expand == HW_AES_KEY_EXPAND_BY_SW) ||
                       (aes_cfg->key_expand == HW_AES_KEY_EXPAND_BY_HW));

        ASSERT_WARNING((aes_cfg->output_data_mode == HW_AES_OUTPUT_DATA_MODE_ALL) ||
                       (aes_cfg->output_data_mode == HW_AES_OUTPUT_DATA_MODE_FINAL_BLOCK));

        HW_AES_HASH_STATUS status;
        uint32_t crypto_ctrl_reg;

        // Critical section to avoid race condition
        GLOBAL_INT_DISABLE();
        status = hw_aes_hash_get_status();

        if (status != HW_AES_HASH_STATUS_LOCKED_BY_HASH) {
                // Use direct register access instead of the hw_aes_hash_enable_clock()
                // to avoid nested critical section due to the function call.
                REG_SET_BIT(CRG_TOP, CLK_AMBA_REG, AES_CLK_ENABLE);
                hw_aes_set_mode(aes_cfg->mode);
        }
        GLOBAL_INT_RESTORE();

        // This check has to take place outside the critical section, because an else condition
        // would end up to return from the function without restoring the interrupts.
        if (status == HW_AES_HASH_STATUS_LOCKED_BY_HASH) {
                return HW_AES_ERROR_CRYPTO_ENGINE_LOCKED;
        }

        crypto_ctrl_reg = AES_HASH->CRYPTO_CTRL_REG;

        REG_SET_FIELD(AES_HASH, CRYPTO_CTRL_REG, CRYPTO_ENCDEC, crypto_ctrl_reg, aes_cfg->operation);
        REG_SET_FIELD(AES_HASH, CRYPTO_CTRL_REG, CRYPTO_AES_KEY_SZ, crypto_ctrl_reg, aes_cfg->key_size);
        REG_SET_FIELD(AES_HASH, CRYPTO_CTRL_REG, CRYPTO_AES_KEXP, crypto_ctrl_reg,aes_cfg->key_expand);
        REG_SET_FIELD(AES_HASH, CRYPTO_CTRL_REG, CRYPTO_OUT_MD, crypto_ctrl_reg, aes_cfg->output_data_mode);
        REG_SET_FIELD(AES_HASH, CRYPTO_CTRL_REG, CRYPTO_MORE_IN, crypto_ctrl_reg, aes_cfg->wait_more_input);

        AES_HASH->CRYPTO_CTRL_REG = crypto_ctrl_reg;

        if (aes_cfg->mode == HW_AES_MODE_CBC || aes_cfg->mode == HW_AES_MODE_CTR) {
                hw_aes_set_init_vector(aes_cfg->iv_cnt_ptr);
        }

        hw_aes_load_keys(aes_cfg->keys_addr, aes_cfg->key_size, aes_cfg->key_expand);
        hw_aes_hash_set_input_data_addr(aes_cfg->input_data_addr);
        hw_aes_hash_set_output_data_addr(aes_cfg->output_data_addr);
        hw_aes_hash_set_input_data_len(aes_cfg->input_data_len);

        if (aes_cfg->callback == NULL) {
                hw_aes_hash_interrupt_disable();
        } else {
                hw_aes_hash_interrupt_enable(aes_cfg->callback);
        }

        if (!hw_aes_check_input_data_len_restrictions()) {
                return HW_AES_ERROR_INVALID_INPUT_DATA_LEN;
        }

        return HW_AES_ERROR_NONE;
}

#endif /* dg_configUSE_HW_AES */
