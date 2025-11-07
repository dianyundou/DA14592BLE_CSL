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
 * @file qspi_w25q32jwiq_v2.h
 *
 * @brief QSPI flash driver for Winbond W25Q32JWIQ_V2
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
#ifndef _QSPI_W25Q32JWIQ_V2_H_
#define _QSPI_W25Q32JWIQ_V2_H_

#include "hw_clk.h"
#include "hw_qspi.h"
#include "qspi_common.h"
#include "qspi_winbond_v2.h"

#define QSPI_W25Q32JWIQ_DENSITY                         (0x16)

__RETAINED_CODE static void qspi_w25q32jwiq_initialize(HW_QSPIC_ID id, sys_clk_t sys_clk);

static const qspi_flash_config_t qspi_w25q32jwiq_cfg = {
        .jedec.manufacturer_id                                  = QSPI_WINBOND_MANUFACTURER_ID,
        .jedec.type                                             = QSPI_WINBOND_W25QXXXJWIQ_TYPE,
        .jedec.density                                          = QSPI_W25Q32JWIQ_DENSITY,
        .jedec.density_mask                                     = 0xFF,

        .size_bits                                              = QSPI_MEMORY_SIZE_32Mbits,
        .address_size                                           = HW_QSPI_ADDR_SIZE_24,
        .clk_mode                                               = HW_QSPI_CLK_MODE_LOW,

        .read_instr_cfg.opcode_bus_mode                         = HW_QSPI_BUS_MODE_SINGLE,
        .read_instr_cfg.addr_bus_mode                           = HW_QSPI_BUS_MODE_QUAD,
        .read_instr_cfg.extra_byte_bus_mode                     = HW_QSPI_BUS_MODE_QUAD,
        .read_instr_cfg.dummy_bus_mode                          = HW_QSPI_BUS_MODE_QUAD,
        .read_instr_cfg.data_bus_mode                           = HW_QSPI_BUS_MODE_QUAD,
        // This memory doesn't support continuous mode of operation
        .read_instr_cfg.continuous_mode                         = HW_QSPI_CONTINUOUS_MODE_DISABLE,
        .read_instr_cfg.extra_byte_cfg                          = HW_QSPI_EXTRA_BYTE_ENABLE,
        .read_instr_cfg.extra_byte_half_cfg                     = HW_QSPI_EXTRA_BYTE_HALF_DISABLE,
        .read_instr_cfg.opcode                                  = QSPI_FAST_READ_QUAD_OPCODE,
        .read_instr_cfg.extra_byte_value                        = 0xFF,
        .read_instr_cfg.cs_idle_delay_nsec                      = 10,    // tSHSL1

        .erase_instr_cfg.opcode_bus_mode                        = HW_QSPI_BUS_MODE_SINGLE,
        .erase_instr_cfg.addr_bus_mode                          = HW_QSPI_BUS_MODE_SINGLE,
        .erase_instr_cfg.hclk_cycles                            = 0,
        .erase_instr_cfg.opcode                                 = QSPI_SECTOR_ERASE_OPCODE,
        .erase_instr_cfg.cs_idle_delay_nsec                     = 50,    // tSHSL2

        .read_status_instr_cfg.opcode_bus_mode                  = HW_QSPI_BUS_MODE_SINGLE,
        .read_status_instr_cfg.receive_bus_mode                 = HW_QSPI_BUS_MODE_SINGLE,
        .read_status_instr_cfg.busy_level                       = HW_QSPI_BUSY_LEVEL_HIGH,
        .read_status_instr_cfg.busy_pos                         = QSPI_STATUS_REG_BUSY_BIT,
        .read_status_instr_cfg.opcode                           = QSPI_READ_STATUS_REG_OPCODE,
        .read_status_instr_cfg.delay_nsec                       = 0,

        .write_enable_instr_cfg.opcode_bus_mode                 = HW_QSPI_BUS_MODE_SINGLE,
        .write_enable_instr_cfg.opcode                          = QSPI_WRITE_ENABLE_OPCODE,

        .page_program_instr_cfg.opcode_bus_mode                 = HW_QSPI_BUS_MODE_SINGLE,
        .page_program_instr_cfg.addr_bus_mode                   = HW_QSPI_BUS_MODE_SINGLE,
        .page_program_instr_cfg.data_bus_mode                   = HW_QSPI_BUS_MODE_QUAD,
        .page_program_instr_cfg.opcode                          = QSPI_PAGE_PROGRAM_QUAD_OPCODE,

        .suspend_resume_instr_cfg.suspend_bus_mode              = HW_QSPI_BUS_MODE_SINGLE,
        .suspend_resume_instr_cfg.resume_bus_mode               = HW_QSPI_BUS_MODE_SINGLE,
        .suspend_resume_instr_cfg.suspend_opcode                = QSPI_WINBOND_SUSPEND_OPCODE,
        .suspend_resume_instr_cfg.resume_opcode                 = QSPI_WINBOND_RESUME_OPCODE,
        .suspend_resume_instr_cfg.resume_latency_usec           = 1,    // 200 nsec
        .suspend_resume_instr_cfg.res_sus_latency_usec          = 20,   // tSUS

        .delay.reset_usec                                       = 30,   // tRST
        .delay.power_down_usec                                  = 3,    // tDP
        .delay.release_power_down_usec                          = 30,   // tRES1
        .delay.power_up_usec                                    = 20,   // tVSL

        .callback.initialize_cb                                 = qspi_w25q32jwiq_initialize,
        .callback.sys_clk_cfg_cb                                = qspi_winbond_sys_clock_cfg,
        .callback.exit_qpi_cb                                   = qspi_exit_qpi,
        .callback.get_dummy_bytes_cb                            = qspi_winbond_get_dummy_bytes,
        .callback.is_suspended_cb                               = qspi_winbond_is_suspended,
        .callback.is_busy_cb                                    = qspi_winbond_is_busy,
        .callback.read_status_reg_cb                            = qspi_winbond_read_status_reg,
        .callback.write_status_reg_cb                           = qspi_winbond_write_status_reg,

        .resume_before_writing_regs                             = false,
};

__RETAINED_CODE static void qspi_w25q32jwiq_initialize(HW_QSPIC_ID id, sys_clk_t sys_clk)
{
#if QSPI_WINBOND_UNLOCK_PROTECTION
        qspi_winbond_unlock_protection(id);
#endif

        // The Quad Enable (QE) bit of this flash memory is enabled by default as a fixed setting
        // from the factory. Nevertheless, the function qspi_winbond_enable_quad_mode() is invoked
        // to ensure that the QE bit is enabled.
        qspi_winbond_enable_quad_mode(id);
}

#if (dg_configCODE_LOCATION == NON_VOLATILE_IS_QSPI_FLASH) & (dg_configQSPI_FLASH_AUTODETECT == 0)
__attribute__((used, __section__("__product_header_primary__")))
static const PRODUCT_HEADER_STRUCT(3) ph_primary = {
        .burstcmdA = 0xA8FF00EB,
        .burstcmdB = 0x00001026,
        .flash_config_section = 0x11AA,
        .flash_config_length = 0x3,
        .config_seq = {0x31, 0x02, 0x02},
        .crc = 0xE264
};

__attribute__((used, __section__("__product_header_backup__")))
static const PRODUCT_HEADER_STRUCT(3) ph_backup = {
        .burstcmdA = 0xA8FF00EB,
        .burstcmdB = 0x00001026,
        .flash_config_section = 0x11AA,
        .flash_config_length = 0x3,
        .config_seq = {0x31, 0x02, 0x02},
        .crc = 0xE264
};
#endif /* (dg_configUSE_SEGGER_FLASH_LOADER == 1) &&  (dg_configFLASH_AUTODETECT == 0) */

#endif /* _QSPI_W25Q32JWIQ_V2_H_ */
/**
 * \}
 * \}
 */
