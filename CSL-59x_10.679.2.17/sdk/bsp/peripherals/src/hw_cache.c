/**
 * \addtogroup BSP
 * \{
 * \addtogroup DEVICES
 * \{
 * \addtogroup CACHE
 * \{
 * \brief iCache Controller LLD
 */

/**
 *****************************************************************************************
 *
 * @file hw_cache.c
 *
 * @brief Implementation of the iCache Controller Low Level Driver.
 *
 * Copyright (C) 2015-2020 Renesas Electronics Corporation and/or its affiliates.
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
 *****************************************************************************************
 */


#if dg_configUSE_HW_CACHE

#include "hw_cache.h"

__RETAINED static hw_cache_mrm_cb_t hw_cache_mrm_cb;

void hw_cache_mrm_enable_interrupt(hw_cache_mrm_cb_t cb)
{
        ASSERT_WARNING(cb);
        hw_cache_mrm_cb = cb;
        REG_SET_BIT(CACHE, CACHE_MRM_CTRL_REG, MRM_IRQ_MASK);
        NVIC_ClearPendingIRQ(MRM_IRQn);
        NVIC_EnableIRQ(MRM_IRQn);
}

void hw_cache_mrm_disable_interrupt(void)
{
        REG_CLR_BIT(CACHE, CACHE_MRM_CTRL_REG, MRM_IRQ_MASK);
        NVIC_DisableIRQ(MRM_IRQn);
        NVIC_ClearPendingIRQ(MRM_IRQn);
        hw_cache_mrm_cb = NULL;
}

__RETAINED_CODE void MRM_Handler(void)
{
        if (hw_cache_mrm_cb) {
                hw_cache_mrm_cb();
        }
}

#endif /* dg_configUSE_HW_CACHE */


/**
 * \}
 * \}
 * \}
 */
