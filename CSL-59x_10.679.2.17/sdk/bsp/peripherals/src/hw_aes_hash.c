/**
 ****************************************************************************************
 *
 * @file hw_aes_hash.c
 *
 * @brief Implementation of the AES/Hash Engine Low Level Driver.
 *
 * Copyright (C) 2015-2023 Renesas Electronics Corporation and/or its affiliates.
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

#if dg_configUSE_HW_AES || dg_configUSE_HW_HASH

#include "hw_aes_hash.h"
#include "hw_sys_internal.h"

__ALWAYS_RETAINED_CODE bool hw_aes_hash_is_active(void)
{
        return REG_GETF(AES_HASH, CRYPTO_STATUS_REG, CRYPTO_INACTIVE) == 0;
}

void hw_aes_hash_set_input_data_addr(uint32_t inp_data_addr)
{
        uint32_t inp_phy_addr = hw_sys_get_physical_addr(inp_data_addr);

        AES_HASH->CRYPTO_FETCH_ADDR_REG = inp_phy_addr;
}

void hw_aes_hash_set_output_data_addr(uint32_t out_data_addr)
{
        uint32_t out_phy_addr = hw_sys_get_physical_addr(out_data_addr);

        AES_HASH->CRYPTO_DEST_ADDR_REG = out_phy_addr;
}

HW_AES_HASH_STATUS hw_aes_hash_get_status(void)
{
        bool clk_enabled = hw_aes_hash_clock_is_enabled();
        bool hash_enabled = (bool) REG_GETF(AES_HASH, CRYPTO_CTRL_REG, CRYPTO_HASH_SEL);

        if (clk_enabled && hash_enabled) {
                return HW_AES_HASH_STATUS_LOCKED_BY_HASH;
        } else if (clk_enabled && !hash_enabled) {
                return HW_AES_HASH_STATUS_LOCKED_BY_AES;
        }

        return HW_AES_HASH_STATUS_UNLOCKED;
}

void hw_aes_hash_deinit(void)
{
        hw_aes_hash_interrupt_disable();
        /* Clear AES/HASH interrupt source */
        AES_HASH->CRYPTO_CLRIRQ_REG = 0x1;
        hw_aes_hash_disable_clock();
}

#if (dg_configSYSTEMVIEW)
#  include "SEGGER_SYSVIEW_FreeRTOS.h"
#else
#  define SEGGER_SYSTEMVIEW_ISR_ENTER()
#  define SEGGER_SYSTEMVIEW_ISR_EXIT()
#endif

__RETAINED static hw_aes_hash_cb aes_hash_cb;

void hw_aes_hash_interrupt_enable(hw_aes_hash_cb cb)
{
        ASSERT_WARNING(cb);

        aes_hash_cb = cb;
        REG_SET_BIT(AES_HASH, CRYPTO_CTRL_REG, CRYPTO_IRQ_EN);
        NVIC_ClearPendingIRQ(CRYPTO_IRQn);
        NVIC_EnableIRQ(CRYPTO_IRQn);
}

void hw_aes_hash_interrupt_disable(void)
{
        aes_hash_cb = NULL;
        REG_CLR_BIT(AES_HASH, CRYPTO_CTRL_REG, CRYPTO_IRQ_EN);
        NVIC_ClearPendingIRQ(CRYPTO_IRQn);
        NVIC_DisableIRQ(CRYPTO_IRQn);
}

void Crypto_Handler(void)
{
        SEGGER_SYSTEMVIEW_ISR_ENTER();

        uint32_t status = AES_HASH->CRYPTO_STATUS_REG;
        /* Clear AES/HASH interrupt source */
        AES_HASH->CRYPTO_CLRIRQ_REG = 0x1;

        if (aes_hash_cb != NULL) {
                aes_hash_cb(status);
        }

        SEGGER_SYSTEMVIEW_ISR_EXIT();
}

#endif /* dg_configUSE_HW_AES || dg_configUSE_HW_HASH */
