/**
 ****************************************************************************************
 *
 * @file hw_qspi_v2.c
 *
 * @brief Implementation of the QSPI Low Level Driver.
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
#if dg_configUSE_HW_QSPI

#include <stdint.h>
#include "hw_qspi.h"

__RETAINED_CODE void hw_qspi_init(HW_QSPIC_ID id, const hw_qspi_config_t *cfg)
{
        if (cfg) {
                ASSERT_WARNING(IS_HW_QSPI_ADDR_SIZE(cfg->address_size));
                ASSERT_WARNING(IS_HW_QSPI_CLK_DIV(cfg->clk_div));
                ASSERT_WARNING(IS_HW_QSPI_CLK_MODE(cfg->clock_mode));
                ASSERT_WARNING(IS_HW_QSPI_DRIVE_CURRENT(cfg->drive_current));
                ASSERT_WARNING(IS_HW_QSPI_READ_PIPE(cfg->read_pipe));
                ASSERT_WARNING(IS_HW_QSPI_READ_PIPE_DELAY(cfg->read_pipe_delay));
                ASSERT_WARNING(IS_HW_QSPI_SAMPLING_EDGE(cfg->sampling_edge));
                ASSERT_WARNING(IS_HW_QSPI_SLEW_RATE(cfg->slew_rate));
                ASSERT_WARNING(IS_HW_QSPI_HREADY_MODE(cfg->hready_mode));

                hw_qspi_set_div(id, cfg->clk_div);
                hw_qspi_clock_enable(id);

                uint32_t ctrlmode_reg = QSPIBA(id)->QSPIC_CTRLMODE_REG;
                uint32_t gp_reg = QSPIBA(id)->QSPIC_GP_REG;

                REG_SET_FIELD(QSPIC, QSPIC_CTRLMODE_REG, QSPIC_USE_32BA, ctrlmode_reg, cfg->address_size);
                REG_SET_FIELD(QSPIC, QSPIC_CTRLMODE_REG, QSPIC_CLK_MD, ctrlmode_reg, cfg->clock_mode);
                REG_SET_FIELD(QSPIC, QSPIC_CTRLMODE_REG, QSPIC_RXD_NEG, ctrlmode_reg, cfg->sampling_edge);
                REG_SET_FIELD(QSPIC, QSPIC_CTRLMODE_REG, QSPIC_RPIPE_EN, ctrlmode_reg, cfg->read_pipe);
                REG_SET_FIELD(QSPIC, QSPIC_CTRLMODE_REG, QSPIC_PCLK_MD, ctrlmode_reg, cfg->read_pipe_delay);
                REG_SET_FIELD(QSPIC, QSPIC_CTRLMODE_REG, QSPIC_HRDY_MD, ctrlmode_reg, cfg->hready_mode);

                REG_SET_FIELD(QSPIC, QSPIC_GP_REG, QSPIC_PADS_DRV, gp_reg, cfg->drive_current);
                REG_SET_FIELD(QSPIC, QSPIC_GP_REG, QSPIC_PADS_SLEW , gp_reg, cfg->slew_rate);

                QSPIBA(id)->QSPIC_CTRLMODE_REG = ctrlmode_reg;
                QSPIBA(id)->QSPIC_GP_REG = gp_reg;
        }
}

__RETAINED_CODE void hw_qspi_erase_block(HW_QSPIC_ID id, uint32_t addr)
{
        uint32_t block_sector = addr / FLASH_SECTOR_SIZE;

        // Wait for previous erase to end
        while (hw_qspi_get_erase_status(id) != HW_QSPI_ERASE_STATUS_NO) {
        }

        if (hw_qspi_get_access_mode(id) != HW_QSPI_ACCESS_MODE_AUTO) {
                hw_qspi_set_access_mode(id, HW_QSPI_ACCESS_MODE_AUTO);
        }

        /*
         * In order to to calculate the sector number and set the proper value for the QSPIC_ERS_ADDR
         * field in the QSPIC_ERASECTRL_REG, the given address needs to be divided by the sector size.
         * However, when using 24 bits addressing size, the sector number needs to be stored in the
         * higher 12 bits of the field, as the remaining bits are disregarded. As a result, the sector
         * number must be shifted left by 8 bits to account for this.
         *
         *                          QSPIC_ERASECTRL_REG[QSPIC_ERS_ADDR]
         *     =====================================================================
         *              19 18 17 16 15 14 13 12 11 10 09 08 07 06 05 04 03 02 01 00
         *     =====================================================================
         *     24 bits:  v  v  v  v  v  v  v  v  v  v  v  v  x  x  x  x  x  x  x  x
         *     32 bits:  v  v  v  v  v  v  v  v  v  v  v  v  v  v  v  v  v  v  v  v
         */

        switch (hw_qspi_get_address_size(id)) {
        case HW_QSPI_ADDR_SIZE_24:
                ASSERT_WARNING(addr < 0x01000000);
                block_sector <<= 8;
                break;
        case HW_QSPI_ADDR_SIZE_32:
                ASSERT_WARNING(addr < MEMORY_QSPIF_SIZE);
                break;
        default:
                ASSERT_WARNING(0);
        }

        hw_qspi_set_erase_address(id, block_sector);
        hw_qspi_trigger_erase(id);
}

#endif /* dg_configUSE_HW_QSPI */
