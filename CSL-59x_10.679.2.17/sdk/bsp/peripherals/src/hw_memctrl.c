/**
 ****************************************************************************************
 *
 * @file hw_memctrl.c
 *
 * @brief Implementation of the Memory Controller Low Level Driver.
 *
 * Copyright (C) 2017-2023 Renesas Electronics Corporation and/or its affiliates.
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
#include "hw_memctrl.h"

void hw_memctrl_reset(void)
{
        MEMCTRL->CMI_CODE_BASE_REG = 0;
        MEMCTRL->CMI_DATA_BASE_REG = 0;
        MEMCTRL->CMI_SHARED_BASE_REG = 0;

}

void hw_memctrl_config_cmac_region(uint32_t code_base_addr, uint32_t data_base_addr, uint32_t shared_base_addr, uint32_t end_addr)
{
        ASSERT_ERROR((code_base_addr % 1024) == 0);
        ASSERT_ERROR((data_base_addr % 4) == 0);
        ASSERT_ERROR((shared_base_addr % 1024) == 0);
        ASSERT_ERROR((end_addr & 0x3FF) == 0x3FF);

        MEMCTRL->CMI_CODE_BASE_REG = code_base_addr;
        MEMCTRL->CMI_DATA_BASE_REG = data_base_addr;
        MEMCTRL->CMI_SHARED_BASE_REG = shared_base_addr;
}



void hw_memctrl_config_master_priorities(MEMCTRL_PRIO syscpu_prio, uint8_t syscpu_max_stall_cycles,
                                         MEMCTRL_PRIO dma_prio, uint8_t dma_max_stall_cycles)
{
        ASSERT_ERROR((syscpu_max_stall_cycles > 0) && (syscpu_max_stall_cycles < 16));
        ASSERT_ERROR((dma_max_stall_cycles > 0) && (dma_max_stall_cycles < 16));

#if (dg_configCODE_LOCATION == NON_VOLATILE_IS_NONE)
        REG_SETF(MEMCTRL, MEM_PRIO_REG, AHB_PRIO, syscpu_prio);
        REG_SETF(MEMCTRL, MEM_STALL_REG, AHB_MAX_STALL, syscpu_max_stall_cycles);
#endif

        REG_SETF(MEMCTRL, MEM_PRIO_REG, AHB2_PRIO, syscpu_prio);
        REG_SETF(MEMCTRL, MEM_STALL_REG, AHB2_MAX_STALL, syscpu_max_stall_cycles);

        REG_SETF(MEMCTRL, MEM_PRIO_REG, AHB3_PRIO, dma_prio);
        REG_SETF(MEMCTRL, MEM_STALL_REG, AHB3_MAX_STALL, dma_max_stall_cycles);
}

