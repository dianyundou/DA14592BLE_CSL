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
 * @file qspi_winbond_v2.h
 *
 * @brief The macros and functions of this header file are utilized by the memory drivers of the
 *        Winbond QSPI flash memories.
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
#ifndef _QSPI_WINBOND_V2_H_
#define _QSPI_WINBOND_V2_H_

#include "qspi_common.h"

#ifndef QSPI_WINBOND_UNLOCK_PROTECTION
#define QSPI_WINBOND_UNLOCK_PROTECTION                  (0)
#endif

#define QSPI_WINBOND_MANUFACTURER_ID                    (0xEF)
#define QSPI_WINBOND_W25QXXXJWIQ_TYPE                   (0x60)
#define QSPI_WINBOND_W25QXXXJWIM_TYPE                   (0x80)

#define QSPI_WINBOND_READ_STATUS_REG2_OPCODE            (0x35)
#define QSPI_WINBOND_READ_STATUS_REG3_OPCODE            (0x15)
#define QSPI_WINBOND_WRITE_STATUS_REG2_OPCODE           (0x31)
#define QSPI_WINBOND_WRITE_STATUS_REG3_OPCODE           (0x11)

#define QSPI_WINBOND_SUSPEND_OPCODE                     (0x75)
#define QSPI_WINBOND_RESUME_OPCODE                      (0x7A)

#define QSPI_WINBOND_STATUS_REG_BP0_POS                 (2)     // Block Protection Bit 0
#define QSPI_WINBOND_STATUS_REG_BP1_POS                 (3)     // Block Protection Bit 1
#define QSPI_WINBOND_STATUS_REG_BP2_POS                 (4)     // Block Protection Bit 2
#define QSPI_WINBOND_STATUS_REG_TB_POS                  (5)     // Top/Bottom Protection Bit
#define QSPI_WINBOND_STATUS_REG_SEC_POS                 (6)     // Sector/Block Protection Bit
#define QSPI_WINBOND_STATUS_REG_SRP_POS                 (7)     // Status Register Protection Bit 0

#define QSPI_WINBOND_STATUS_REG_PROTECTION_MASK         ((1 << QSPI_WINBOND_STATUS_REG_BP0_POS) | \
                                                         (1 << QSPI_WINBOND_STATUS_REG_BP1_POS) | \
                                                         (1 << QSPI_WINBOND_STATUS_REG_BP2_POS) | \
                                                         (1 << QSPI_WINBOND_STATUS_REG_TB_POS)  | \
                                                         (1 << QSPI_WINBOND_STATUS_REG_SEC_POS) | \
                                                         (1 << QSPI_WINBOND_STATUS_REG_SRP_POS))

#define QSPI_WINBOND_STATUS_REG2_SRL_POS                (0)     // Status Register2 Protection Bit 1
#define QSPI_WINBOND_STATUS_REG2_CMP_POS                (6)     // Complement Protect Bit 1

#define QSPI_WINBOND_STATUS_REG2_PROTECTION_MASK        ((1 << QSPI_WINBOND_STATUS_REG2_SRL_POS) | \
                                                         (1 << QSPI_WINBOND_STATUS_REG2_CMP_POS))

#define QSPI_WINBOND_STATUS_REG2_SUSPEND_POS            (7)
#define QSPI_WINBOND_STATUS_REG2_SUSPEND_MASK           (1 << QSPI_WINBOND_STATUS_REG2_SUSPEND_POS)

#define QSPI_WINBOND_STATUS_REG2_QUAD_ENABLE_POS        (1)
#define QSPI_WINBOND_STATUS_REG2_QUAD_ENABLE_MASK       (1 << QSPI_WINBOND_STATUS_REG2_QUAD_ENABLE_POS)

#define QSPI_WINBOND_STATUS_REG3_ADDR_MODE_POS          (0)
#define QSPI_WINBOND_STATUS_REG3_ADDR_MODE_MASK         (1 << QSPI_WINBOND_STATUS_REG3_ADDR_MODE_POS)

#define QSPI_WINBOND_STATUS_REG3_DRV_STRENGTH_POS       (5)
#define QSPI_WINBOND_STATUS_REG3_DRV_STRENGTH_MASK      (3 << QSPI_WINBOND_STATUS_REG3_DRV_STRENGTH_POS)

__RETAINED_CODE static uint8_t qspi_winbond_read_register(HW_QSPIC_ID id, uint8_t opcode,  uint8_t mask)
{
        __DBG_QSPI_VOLATILE__ uint8_t reg_val;

        ASSERT_WARNING((opcode == QSPI_READ_STATUS_REG_OPCODE) ||
                       (opcode == QSPI_WINBOND_READ_STATUS_REG2_OPCODE) ||
                       (opcode == QSPI_WINBOND_READ_STATUS_REG3_OPCODE));

        hw_qspi_cs_enable(id);
        hw_qspi_write8(id, opcode);
        reg_val = hw_qspi_read8(id);
        hw_qspi_cs_disable(id);

        return (reg_val & mask);
}

__RETAINED_CODE static void qspi_winbond_write_register(HW_QSPIC_ID id, uint8_t opcode, uint8_t value)
{
        ASSERT_WARNING((opcode == QSPI_WRITE_STATUS_REG_OPCODE) ||
                       (opcode == QSPI_WINBOND_WRITE_STATUS_REG2_OPCODE) ||
                       (opcode == QSPI_WINBOND_WRITE_STATUS_REG3_OPCODE));

        hw_qspi_cs_enable(id);
        hw_qspi_write8(id, opcode);
        hw_qspi_write8(id, value);
        hw_qspi_cs_disable(id);
}

__RETAINED_CODE static uint8_t qspi_winbond_read_status_reg(HW_QSPIC_ID id)
{
        return qspi_winbond_read_register(id, QSPI_READ_STATUS_REG_OPCODE, 0xFF);
}

__RETAINED_CODE static void qspi_winbond_write_status_reg(HW_QSPIC_ID id, uint8_t status_reg)
{
        qspi_winbond_write_register(id, QSPI_WRITE_STATUS_REG_OPCODE, status_reg);
}

__RETAINED_CODE static uint8_t qspi_winbond_get_dummy_bytes(HW_QSPIC_ID id, sys_clk_t sys_clk)
{
        return 2;
}

__RETAINED_CODE static void qspi_winbond_sys_clock_cfg(HW_QSPIC_ID id, sys_clk_t sys_clk)
{

}

__RETAINED_CODE static bool qspi_winbond_is_suspended(HW_QSPIC_ID id)
{
        __DBG_QSPI_VOLATILE__ uint8_t is_suspended;

        is_suspended = qspi_winbond_read_register(id, QSPI_WINBOND_READ_STATUS_REG2_OPCODE,
                                                  QSPI_WINBOND_STATUS_REG2_SUSPEND_MASK);
        return (bool) is_suspended;
}

__RETAINED_CODE static bool qspi_winbond_is_busy(HW_QSPIC_ID id, HW_QSPI_BUSY_LEVEL busy_level)
{
        __DBG_QSPI_VOLATILE__ HW_QSPI_BUSY_LEVEL is_busy;

        is_busy = (HW_QSPI_BUSY_LEVEL ) (qspi_winbond_read_status_reg(id) & QSPI_STATUS_REG_BUSY_MASK);

        return (bool) is_busy;
}

__RETAINED_CODE static void qspi_winbond_enable_quad_mode(HW_QSPIC_ID id)
{
        __DBG_QSPI_VOLATILE__ uint8_t status_reg2;
        __DBG_QSPI_VOLATILE__ uint8_t verify;

        status_reg2 = qspi_winbond_read_register(id, QSPI_WINBOND_READ_STATUS_REG2_OPCODE, 0xFF);

        if (!(status_reg2 & QSPI_WINBOND_STATUS_REG2_QUAD_ENABLE_MASK)) {
                status_reg2 |= QSPI_WINBOND_STATUS_REG2_QUAD_ENABLE_MASK;
                qspi_flash_write_enable(id);
                qspi_winbond_write_register(id, QSPI_WINBOND_WRITE_STATUS_REG2_OPCODE, status_reg2);
                while (qspi_winbond_is_busy(id, HW_QSPI_BUSY_LEVEL_HIGH));
                verify = qspi_winbond_read_register(id, QSPI_WINBOND_READ_STATUS_REG2_OPCODE, 0xFF);
                ASSERT_WARNING(status_reg2 == verify);
        }
}

__UNUSED __RETAINED_CODE static void qspi_winbond_set_max_drive_strength(HW_QSPIC_ID id)
{
        __DBG_QSPI_VOLATILE__ uint8_t status_reg3;
        __DBG_QSPI_VOLATILE__ uint8_t verify;

        status_reg3 = qspi_winbond_read_register(id, QSPI_WINBOND_READ_STATUS_REG3_OPCODE, 0xFF);

        if ((status_reg3 & QSPI_WINBOND_STATUS_REG3_DRV_STRENGTH_MASK) != 0) {
                status_reg3 &= ~QSPI_WINBOND_STATUS_REG3_DRV_STRENGTH_MASK;
                qspi_flash_write_enable(id);
                qspi_winbond_write_register(id, QSPI_WINBOND_WRITE_STATUS_REG3_OPCODE, status_reg3);
                while (qspi_winbond_is_busy(id, HW_QSPI_BUSY_LEVEL_HIGH));
                verify = qspi_winbond_read_register(id, QSPI_WINBOND_READ_STATUS_REG3_OPCODE, 0xFF);
                ASSERT_WARNING(status_reg3 == verify);
        }
}

__UNUSED __RETAINED_CODE static void qspi_winbond_unlock_protection(HW_QSPIC_ID id)
{
        __DBG_QSPI_VOLATILE__ uint8_t status_reg;
        __DBG_QSPI_VOLATILE__ uint8_t status_reg2;
        __DBG_QSPI_VOLATILE__ uint8_t verify;

        status_reg = qspi_winbond_read_status_reg(id);
        status_reg2 = qspi_winbond_read_register(id, QSPI_WINBOND_READ_STATUS_REG2_OPCODE, 0xFF);

        // Clear Protection Bits [SRP TB BP3 BP2 BP1 BP0]
        if (status_reg & QSPI_WINBOND_STATUS_REG_PROTECTION_MASK) {
                status_reg &= ~QSPI_WINBOND_STATUS_REG_PROTECTION_MASK;
                qspi_flash_write_enable(id);
                qspi_winbond_write_status_reg(id, status_reg);
                while (qspi_winbond_is_busy(id, HW_QSPI_BUSY_LEVEL_HIGH));
                verify = qspi_winbond_read_status_reg(id);
                ASSERT_WARNING(status_reg == verify);
        }

        if (status_reg2 & QSPI_WINBOND_STATUS_REG2_PROTECTION_MASK) {
                status_reg2 &= ~QSPI_WINBOND_STATUS_REG2_PROTECTION_MASK;
                qspi_flash_write_enable(id);
                qspi_winbond_write_register(id, QSPI_WINBOND_WRITE_STATUS_REG2_OPCODE, status_reg2);
                while (qspi_winbond_is_busy(id, HW_QSPI_BUSY_LEVEL_HIGH));
                verify = qspi_winbond_read_register(id, QSPI_WINBOND_READ_STATUS_REG2_OPCODE, 0xFF);
                ASSERT_WARNING(status_reg2 == verify);
        }
}

#endif /* _QSPI_WINBOND_V2_H_ */
/**
 * \}
 * \}
 */
