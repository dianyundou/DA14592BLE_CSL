/**
 * \addtogroup PLA_BSP_SYSTEM
 * \{
 * \addtogroup PLA_MEMORY
 *
 * \{
 */

/**
 ****************************************************************************************
 *
 * @file qspi_mx25u3235_v2.h
 *
 * @brief QSPI flash driver for Macronix MX25U3235_V2
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
#ifndef _QSPI_MX25U3235_V2_H_
#define _QSPI_MX25U3235_V2_H_

#include "hw_clk.h"
#include "hw_qspi.h"
#include "qspi_common.h"
#include "qspi_macronix_v2.h"

#define QSPI_MX25U3235_DENSITY                         (0x36)

__RETAINED_CODE static void qspi_mx25u3235_initialize(HW_QSPIC_ID id, sys_clk_t sys_clk);
__RETAINED_CODE static void qspi_mx25u3235_sys_clock_cfg(HW_QSPIC_ID id, sys_clk_t sys_clk);
__RETAINED_CODE static uint8_t qspi_mx25u3235_get_dummy_bytes(HW_QSPIC_ID id, sys_clk_t sys_clk);
__RETAINED_CODE static void qspi_mx25u3235_write_status_reg(HW_QSPIC_ID id, uint8_t status_reg);

static const qspi_flash_config_t qspi_mx25u3235_cfg = {
        .jedec.manufacturer_id                                  = QSPI_MACRONIX_MANUFACTURER_ID,
        .jedec.type                                             = QSPI_MACRONIX_MX25U_TYPE,
        .jedec.density                                          = QSPI_MX25U3235_DENSITY,
        .jedec.density_mask                                     = 0xFF,

        .size_bits                                              = QSPI_MEMORY_SIZE_32Mbits,
        .address_size                                           = HW_QSPI_ADDR_SIZE_24,
        .clk_mode                                               = HW_QSPI_CLK_MODE_LOW,

        .read_instr_cfg.opcode_bus_mode                         = HW_QSPI_BUS_MODE_SINGLE,
        .read_instr_cfg.addr_bus_mode                           = HW_QSPI_BUS_MODE_QUAD,
        .read_instr_cfg.extra_byte_bus_mode                     = HW_QSPI_BUS_MODE_QUAD,
        .read_instr_cfg.dummy_bus_mode                          = HW_QSPI_BUS_MODE_QUAD,
        .read_instr_cfg.data_bus_mode                           = HW_QSPI_BUS_MODE_QUAD,
        .read_instr_cfg.continuous_mode                         = HW_QSPI_CONTINUOUS_MODE_ENABLE,
        .read_instr_cfg.extra_byte_cfg                          = HW_QSPI_EXTRA_BYTE_ENABLE,
        .read_instr_cfg.extra_byte_half_cfg                     = HW_QSPI_EXTRA_BYTE_HALF_DISABLE,
        .read_instr_cfg.opcode                                  = QSPI_FAST_READ_QUAD_OPCODE,
        .read_instr_cfg.extra_byte_value                        = 0xA5,
        .read_instr_cfg.cs_idle_delay_nsec                      = 5,    // tSHSL (read)

        .erase_instr_cfg.opcode_bus_mode                        = HW_QSPI_BUS_MODE_SINGLE,
        .erase_instr_cfg.addr_bus_mode                          = HW_QSPI_BUS_MODE_SINGLE,
        .erase_instr_cfg.hclk_cycles                            = 0,
        .erase_instr_cfg.opcode                                 = QSPI_SECTOR_ERASE_OPCODE,
        .erase_instr_cfg.cs_idle_delay_nsec                     = 30,   // tSHSL (erase)

        .read_status_instr_cfg.opcode_bus_mode                  = HW_QSPI_BUS_MODE_SINGLE,
        .read_status_instr_cfg.receive_bus_mode                 = HW_QSPI_BUS_MODE_SINGLE,
        .read_status_instr_cfg.busy_level                       = HW_QSPI_BUSY_LEVEL_HIGH,
        .read_status_instr_cfg.busy_pos                         = QSPI_STATUS_REG_BUSY_BIT,
        .read_status_instr_cfg.opcode                           = QSPI_READ_STATUS_REG_OPCODE,
        .read_status_instr_cfg.delay_nsec                       = 0,

        .write_enable_instr_cfg.opcode_bus_mode                 = HW_QSPI_BUS_MODE_SINGLE,
        .write_enable_instr_cfg.opcode                          = QSPI_WRITE_ENABLE_OPCODE,

        .page_program_instr_cfg.opcode_bus_mode                 = HW_QSPI_BUS_MODE_SINGLE,
        .page_program_instr_cfg.addr_bus_mode                   = HW_QSPI_BUS_MODE_QUAD,
        .page_program_instr_cfg.data_bus_mode                   = HW_QSPI_BUS_MODE_QUAD,
        .page_program_instr_cfg.opcode                          = QSPI_MACRONIX_PAGE_PROGRAM_4IO_OPCODE,

        .suspend_resume_instr_cfg.suspend_bus_mode              = HW_QSPI_BUS_MODE_SINGLE,
        .suspend_resume_instr_cfg.resume_bus_mode               = HW_QSPI_BUS_MODE_SINGLE,
        .suspend_resume_instr_cfg.suspend_opcode                = QSPI_MACRONIX_SUSPEND_OPCODE,
        .suspend_resume_instr_cfg.resume_opcode                 = QSPI_MACRONIX_RESUME_OPCODE,
        .suspend_resume_instr_cfg.suspend_latency_usec          = 25,   // tESL
        .suspend_resume_instr_cfg.resume_latency_usec           = 1,    // no latency
        .suspend_resume_instr_cfg.res_sus_latency_usec          = 100,  // tERS

        .delay.reset_usec                                       = 12000, // tREADY2
        .delay.power_down_usec                                  = 10,    // tDP
        .delay.release_power_down_usec                          = 30,    // tRDP
        .delay.power_up_usec                                    = 800,   // tVSL

        .callback.initialize_cb                                 = qspi_mx25u3235_initialize,
        .callback.sys_clk_cfg_cb                                = qspi_mx25u3235_sys_clock_cfg,
        .callback.exit_qpi_cb                                   = qspi_exit_qpi,
        .callback.get_dummy_bytes_cb                            = qspi_mx25u3235_get_dummy_bytes,
        .callback.is_suspended_cb                               = qspi_macronix_is_suspended,
        .callback.is_busy_cb                                    = qspi_macronix_is_busy,
        .callback.read_status_reg_cb                            = qspi_macronix_read_status_reg,
        .callback.write_status_reg_cb                           = qspi_mx25u3235_write_status_reg,

        .resume_before_writing_regs                             = false,
};

__RETAINED_CODE static uint8_t qspi_mx25u3235_get_dummy_bytes(HW_QSPIC_ID id, sys_clk_t sys_clk)
{
        return 2;
}

__RETAINED_CODE static void qspi_mx25u3235_write_status_reg(HW_QSPIC_ID id, uint8_t status_reg)
{
        hw_qspi_cs_enable(id);
        hw_qspi_write8(id, QSPI_WRITE_STATUS_REG_OPCODE);
        hw_qspi_write8(id, status_reg);
        hw_qspi_cs_disable(id);
}

__RETAINED_CODE static void qspi_mx25u3235_enable_quad_mode(HW_QSPIC_ID id)
{
        __DBG_QSPI_VOLATILE__ uint8_t status_reg;
        __DBG_QSPI_VOLATILE__ uint8_t verify;

        status_reg = qspi_flash_read_status_register(id);

        if (!(status_reg & QSPI_MACRONIX_STATUS_REG_QUAD_ENABLE_MASK)) {
                status_reg |= QSPI_MACRONIX_STATUS_REG_QUAD_ENABLE_MASK;
                qspi_flash_write_enable(id);
                qspi_flash_write_status_register(id, status_reg);
                while (qspi_macronix_is_busy(id, HW_QSPI_BUSY_LEVEL_HIGH));
                verify = qspi_flash_read_status_register(id);
                ASSERT_WARNING((status_reg & QSPI_MACRONIX_STATUS_REG_QUAD_ENABLE_MASK) ==
                                (verify & QSPI_MACRONIX_STATUS_REG_QUAD_ENABLE_MASK));
        }
}

__RETAINED_CODE static void qspi_mx25u3235_initialize(HW_QSPIC_ID id, sys_clk_t sys_clk)
{
                qspi_mx25u3235_enable_quad_mode(id);
}

__RETAINED_CODE static void qspi_mx25u3235_sys_clock_cfg(HW_QSPIC_ID id, sys_clk_t sys_clk)
{

}

#if (dg_configCODE_LOCATION == NON_VOLATILE_IS_QSPI_FLASH) & (dg_configQSPI_FLASH_AUTODETECT == 0)
__attribute__((used, __section__("__product_header_primary__")))
static const PRODUCT_HEADER_STRUCT(3) ph_primary = {
        .burstcmdA = 0xA8A500EB,
        .burstcmdB = 0x00000066,
        .flash_config_section = 0x11AA,
        .flash_config_length = 0x3,
        .config_seq = {0x01, 0x40, 0x02},
        .crc = 0x51D4
};

__attribute__((used, __section__("__product_header_backup__")))
static const PRODUCT_HEADER_STRUCT(3) ph_backup = {
        .burstcmdA = 0xA8A500EB,
        .burstcmdB = 0x00000066,
        .flash_config_section = 0x11AA,
        .flash_config_length = 0x3,
        .config_seq = {0x01, 0x40, 0x02},
        .crc = 0x51D4
};
#endif /* (dg_configUSE_SEGGER_FLASH_LOADER == 1) &&  (dg_configFLASH_AUTODETECT == 0) */

#endif /* _QSPI_MX25U3235_V2_H_ */
/**
 * \}
 * \}
 */
