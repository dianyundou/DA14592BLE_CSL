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
 * @file qspi_macronix_v2.h
 *
 * @brief The macros and functions of this header file are utilized by the memory drivers of the
 *        Macronix QSPI flash memories.
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
#ifndef _QSPI_MACRONIX_V2_H_
#define _QSPI_MACRONIX_V2_H_

#include "qspi_common.h"

#define QSPI_MACRONIX_MANUFACTURER_ID                     (0xC2)
#define QSPI_MACRONIX_MX25U_TYPE                          (0x25)
#define QSPI_MACRONIX_MX25R_TYPE                          (0x28)

#define QSPI_MACRONIX_READ_CONFIG_REG_OPCODE              (0x15)
#define QSPI_MACRONIX_READ_SECURITY_REG_OPCODE            (0x2B)

#define QSPI_MACRONIX_PAGE_PROGRAM_4IO_OPCODE             (0x38)

#define QSPI_MACRONIX_SUSPEND_OPCODE                      (0xB0)
#define QSPI_MACRONIX_RESUME_OPCODE                       (0x30)

#define QSPI_MACRONIX_STATUS_REG_QUAD_ENABLE_BIT          (6)
#define QSPI_MACRONIX_STATUS_REG_QUAD_ENABLE_MASK         (1 << QSPI_MACRONIX_STATUS_REG_QUAD_ENABLE_BIT)

#define QSPI_MACRONIX_STATUS_REG3_ADDR_MODE_BIT           (0)
#define QSPI_MACRONIX_STATUS_REG3_ADDR_MODE_MASK          (1 << QSPI_MACRONIX_STATUS_REG3_ADDR_MODE_BIT)

#define QSPI_MACRONIX_STATUS_REG3_DRV_STRENGTH_BITS       (5)
#define QSPI_MACRONIX_STATUS_REG3_DRV_STRENGTH_MASK       (3 << QSPI_MACRONIX_STATUS_REG3_DRV_STRENGTH_BITS)

#define QSPI_MACRONIX_SECURITY_REG_ERASE_SUSPEND_BIT      (3)
#define QSPI_MACRONIX_SECURITY_REG_ERASE_SUSPEND_MASK     (1 << QSPI_MACRONIX_SECURITY_REG_ERASE_SUSPEND_BIT)

#define QSPI_MACRONIX_SECURITY_REG_PROGRAM_SUSPEND_BIT    (2)
#define QSPI_MACRONIX_SECURITY_REG_PROGRAM_SUSPEND_MASK   (1 << QSPI_MACRONIX_SECURITY_REG_PROGRAM_SUSPEND_BIT)

#define QSPI_MACRONIX_SECURITY_REG_SUSPEND_MASK           (QSPI_MACRONIX_SECURITY_REG_ERASE_SUSPEND_MASK | \
                                                           QSPI_MACRONIX_SECURITY_REG_PROGRAM_SUSPEND_MASK)

__UNUSED __RETAINED_CODE static uint8_t qspi_macronix_read_register(HW_QSPIC_ID id, uint8_t opcode,  uint8_t mask)
{
        __DBG_QSPI_VOLATILE__ uint8_t reg_val;

        ASSERT_WARNING((opcode == QSPI_READ_STATUS_REG_OPCODE) ||
                       (opcode == QSPI_MACRONIX_READ_CONFIG_REG_OPCODE) ||
                       (opcode == QSPI_MACRONIX_READ_SECURITY_REG_OPCODE));

        hw_qspi_cs_enable(id);
        hw_qspi_write8(id, opcode);
        reg_val = hw_qspi_read8(id);
        hw_qspi_cs_disable(id);

        return (reg_val & mask);
}

// This function is used by macronix flash memory families where the status register and the
// configuration register are modified simultaneously.
__UNUSED __RETAINED_CODE static void qspi_macronix_write_status_and_config_reg(HW_QSPIC_ID id, uint8_t status_reg, uint8_t config_reg)
{
        // The hw_qspi_write16() swaps the MSB with LSB, therefore the status_reg and config_reg
        // are swapped in regs too.
        __DBG_QSPI_VOLATILE__ uint16_t regs = ((((uint16_t) (status_reg)) << 8) & 0xFF00) | config_reg;

        hw_qspi_cs_enable(id);
        hw_qspi_write8(id, QSPI_WRITE_STATUS_REG_OPCODE);
        hw_qspi_write16(id, regs);
        hw_qspi_cs_disable(id);
}

__UNUSED __RETAINED_CODE static uint8_t qspi_macronix_read_status_reg(HW_QSPIC_ID id)
{
        return qspi_macronix_read_register(id, QSPI_READ_STATUS_REG_OPCODE, 0xFF);
}

__UNUSED __RETAINED_CODE static void qspi_macronix_write_status_reg(HW_QSPIC_ID id, uint8_t status_reg)
{
        __DBG_QSPI_VOLATILE__ uint8_t config_reg;

        config_reg = qspi_macronix_read_register(id, QSPI_MACRONIX_READ_CONFIG_REG_OPCODE, 0xFF);
        qspi_macronix_write_status_and_config_reg(id, status_reg, config_reg);
}

__UNUSED __RETAINED_CODE static bool qspi_macronix_is_suspended(HW_QSPIC_ID id)
{
        __DBG_QSPI_VOLATILE__ uint8_t is_suspended;

        is_suspended = qspi_macronix_read_register(id, QSPI_MACRONIX_READ_SECURITY_REG_OPCODE,
                                                   QSPI_MACRONIX_SECURITY_REG_SUSPEND_MASK);
        return (bool) is_suspended;
}

__UNUSED __RETAINED_CODE static bool qspi_macronix_is_busy(HW_QSPIC_ID id, HW_QSPI_BUSY_LEVEL busy_level)
{
        __DBG_QSPI_VOLATILE__ HW_QSPI_BUSY_LEVEL is_busy;

        is_busy = (HW_QSPI_BUSY_LEVEL) (qspi_macronix_read_status_reg(id) & QSPI_STATUS_REG_BUSY_MASK);

        return (bool) (is_busy == busy_level);
}

#endif /* _QSPI_MACRONIX_V2_H_ */
/**
 * \}
 * \}
 */
