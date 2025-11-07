/**
 ****************************************************************************************
 *
 * @file hw_pdm.c
 *
 * @brief Implementation of the PDM/Audio Low Level Driver.
 *
 * Copyright (C) 2020-2023 Renesas Electronics Corporation and/or its affiliates.
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

#if dg_configUSE_HW_PDM
#include "hw_pdm.h"

uint32_t hw_pdm_clk_init(uint32_t frequency)
{
        uint32_t div;

        /* Translate main clk frequency and requested frequency to proper divider */
        div = (dg_configDIVN_FREQ / frequency);

        /* Calculate the achievable frequency */
        if (dg_configXTAL32M_FREQ % frequency) {
                frequency = (dg_configXTAL32M_FREQ / div);
        }

        /* PDM_CLK frequency according to specification is in the range of 62.5 kHz - 4 MHz */
        ASSERT_WARNING((frequency >= 62500) && (frequency <= 4000000));

        ASSERT_WARNING((div & ~(HW_PDM_CRG_REG_FIELD_MASK(DIV, PDM_DIV) >>
                                HW_PDM_CRG_REG_FIELD_POS(DIV, PDM_DIV))) == 0);

        HW_PDM_CRG_REG_SETF(DIV, PDM_DIV, div);

        return frequency;
}

void hw_pdm_init(HW_SRC_ID id, hw_pdm_config_t *config)
{
        uint32_t src1_ctrl_reg = SRCBA(id)->SRC1_CTRL_REG;

        if (config->data_direction == PDM_DIRECTION_INPUT) {
                HW_SRC_REG_SET_FIELD(SRC1, CTRL_REG, SRC_PDM_DI_DEL, src1_ctrl_reg, config->in_delay);
                if (config->swap_channel == false) {
                        REG_SET_FIELD(SRC1, SRC1_CTRL_REG, SRC_PDM_IN_INV, src1_ctrl_reg, true);

                } else {
                        REG_CLR_FIELD(SRC1, SRC1_CTRL_REG, SRC_PDM_IN_INV, src1_ctrl_reg);
                }
        } else {
                HW_SRC_REG_SET_FIELD(SRC1, CTRL_REG, SRC_PDM_DO_DEL, src1_ctrl_reg, config->out_delay);
                if (config->swap_channel == true) {
                        REG_SET_FIELD(SRC1, SRC1_CTRL_REG, SRC_PDM_OUT_INV, src1_ctrl_reg, true);

                } else {
                        REG_CLR_FIELD(SRC1, SRC1_CTRL_REG, SRC_PDM_OUT_INV, src1_ctrl_reg);
                }
        }

        SRCBA(id)->SRC1_CTRL_REG = src1_ctrl_reg;

        hw_pdm_set_mode(config->config_mode);
}
#endif /* dg_configUSE_HW_PDM */
