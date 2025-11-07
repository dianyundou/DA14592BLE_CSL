/**
 * \addtogroup PLA_DRI_MEMORY
 * \{
 * \addtogroup HW_FCU Embedded Flash Controller
 * \{
 * \brief Embedded Flash Controller
 */

/**
 ****************************************************************************************
 *
 * @file hw_fcu.h
 *
 * @brief Definition of API for the FCU Low Level Driver.
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

#ifndef HW_FCU_H_
#define HW_FCU_H_

#if dg_configUSE_HW_FCU

#include <stdbool.h>
#include "sdk_defs.h"
#include "hw_dma.h"

/*
 * The following macros are defined according to FCU properties and its registers default values
 * and size in bits
 */
#define HW_FCU_FLASH_PAGE_SIZE_IN_BYTES (2048)
#define HW_FCU_FLASH_PAGE_SIZE_IN_WORDS (HW_FCU_FLASH_PAGE_SIZE_IN_BYTES / 4)

#define HW_FCU_ERASE_FLASH_PAGE_MIN_TIME (0x13880)
/*
 * FLASH_PTERASE_REG[PTERASE] field is 24 bits but FCU hardware only uses
 * 18 bits, so maximum erase time is 262143 us
 */
#define HW_FCU_ERASE_FLASH_PAGE_MAX_TIME (0x3FFFF)

#define HW_FCU_ERASE_FLASH_BLOCK_MIN_TIME (0x13880)
/*
 * FLASH_PTME_REG[PTME] field is 24 bits but FCU hardware only uses
 * 18 bits, so maximum erase time is 262143 us
 */
#define HW_FCU_ERASE_FLASH_BLOCK_MAX_TIME (0x3FFFF)

#define HW_FCU_ERASE_FLASH_PAGE_SEGMENT_MIN_TIME (0x3E8)
/*
 * FLASH_PTERASE_SEG_REG[PTERASE_SEG] field is 24 bits but FCU hardware only uses
 * 18 bits, so maximum erase time is 262143 us
 */
#define HW_FCU_ERASE_FLASH_PAGE_SEGMENT_MAX_TIME (0x3FFFF)

/**
 * \brief FCU error codes
 */
typedef enum {
        HW_FCU_ERROR_ERASE_IN_PROGRESS = -5,
        HW_FCU_ERROR_WRITE_IN_PROGRESS = -4,
        HW_FCU_ERROR_PROTECTED_AGAINST_ACTION = -3,
        HW_FCU_ERROR_INVALID_ADDRESS = -2,
        HW_FCU_ERROR_INVALID_FUNCTION_INPUT = -1,
        HW_FCU_ERROR_NONE = 0
} HW_FCU_ERROR;

/**
 * \brief FCU erase/suspend modes
 */
typedef enum {
        HW_FCU_ERASE_SUSPEND_MODE_PREEMPTIVE,     /**< When a read operation is issued, erase is
                                                       immediately suspended and erase segment time
                                                       is not subtracted from total erase time */
        HW_FCU_ERASE_SUSPEND_MODE_NONPREEMPTIVE , /**< When a read operation is issued, FCU stall
                                                       the read access until the erase segment is
                                                       completed */
} HW_FCU_ERASE_SUSPEND_MODE;

/**
 * \brief FCU flash read/write/erase mode
 */
typedef enum {
        HW_FCU_FLASH_ACCESS_MODE_READ,        /**< Read mode selected */
        HW_FCU_FLASH_ACCESS_MODE_WRITE_ERASE, /**< Erase/write mode selected */
} HW_FCU_FLASH_ACCESS_MODE;

/**
 * \brief FCU program flash mode
 */
typedef enum {
        HW_FCU_FLASH_PROG_MODE_READ,           /**< No write or erase to flash possible */
        HW_FCU_FLASH_PROG_MODE_WRITE_PAGE,     /**< Write page */
        HW_FCU_FLASH_PROG_MODE_ERASE_PAGE,     /**< Erase page */
        HW_FCU_FLASH_PROG_MODE_ERASE_BLOCK,    /**< Erase one of the flash blocks completely */
} HW_FCU_FLASH_PROG_MODE;

/**
 * \brief FCU wait cycles
 */
typedef enum {
        HW_FCU_WAIT_CYCLES_0,
        HW_FCU_WAIT_CYCLES_1,
        HW_FCU_WAIT_CYCLES_2,
        HW_FCU_WAIT_CYCLES_3,
        HW_FCU_WAIT_CYCLES_4,
        HW_FCU_WAIT_CYCLES_5,
        HW_FCU_WAIT_CYCLES_6,
        HW_FCU_WAIT_CYCLES_7,
        HW_FCU_WAIT_CYCLES_MAX
} HW_FCU_WAIT_CYCLES;

/**
 * \brief FCU VDD level value
 */
typedef enum {
        HW_FCU_VDD_LESS_THAN_1V08,      /**< Should be used if VDD < 1.08 V. */
        HW_FCU_VDD_GREATER_THAN_1V08    /**< Should be used if VDD > 1.08 V. */
} HW_FCU_VDD_LEVEL_VALUE;

/**
 * \brief Callback called upon completion of erase or write (interrupt or DMA)
 *
 * \param [in] user_data pointer to user data
 */
typedef void (*hw_fcu_operation_completed_cb)(void *user_data);

/**
 * \brief Operation parameters structure
 */
struct hw_fcu_operation_params_t {
        hw_fcu_operation_completed_cb cb;       /**< user callback function */
        void *user_data;                        /**< pointer to user data */
#if dg_configUSE_HW_DMA
        HW_DMA_CHANNEL dma_channel;             /**< DMA channel (only valid for read or write) */
#endif
};

/**
 * \brief Set VDD level value
 *
 * \note If VDD_LEVEL_FORCE is set to 0, VDD level value has no effect.
 *
 * \param [in] value VDD level value
 *
 * \sa HW_FCU_VDD_LEVEL_VALUE
 * \sa hw_fcu_enable_vdd_level_overriding
 */
__STATIC_FORCEINLINE void hw_fcu_set_vdd_level_value(HW_FCU_VDD_LEVEL_VALUE value)
{
        REG_SETF(FCU, FLASH_CTRL_REG, VDD_LEVEL_VALUE, value);
}

/**
 * \brief Get VDD level value
 *
 * \note If VDD_LEVEL_FORCE is set to 0, VDD level value has no effect.
 *
 * \return VDD level value
 *
 * \sa HW_FCU_VDD_LEVEL_VALUE
 * \sa hw_fcu_enable_vdd_level_overriding
 */
__STATIC_FORCEINLINE HW_FCU_VDD_LEVEL_VALUE hw_fcu_get_vdd_level_value(void)
{
        return REG_GETF(FCU, FLASH_CTRL_REG, VDD_LEVEL_VALUE);
}

/**
 * \brief Enable overriding of VDD level FCU input signal
 */
__STATIC_FORCEINLINE void hw_fcu_enable_vdd_level_overriding(void)
{
        REG_SET_BIT(FCU, FLASH_CTRL_REG, VDD_LEVEL_FORCE);
}

/**
 * \brief Disable overriding of VDD level FCU input signal
 */
__STATIC_FORCEINLINE void hw_fcu_disable_vdd_level_overriding(void)
{
        REG_CLR_BIT(FCU, FLASH_CTRL_REG, VDD_LEVEL_FORCE);
}

/**
 * \brief Prohibit flash operations
 */
__STATIC_FORCEINLINE void hw_fcu_prohibit_flash_operations(void)
{
        REG_SET_BIT(FCU, FLASH_CTRL_REG, FLASH_PROT);
}

/**
 * \brief Permit flash operations
 */
__STATIC_FORCEINLINE void hw_fcu_permit_flash_operations(void)
{
        REG_CLR_BIT(FCU, FLASH_CTRL_REG, FLASH_PROT);
}

/**
 * \brief Check if flash operations are prohibited
 *
 * \return true if flash operations are prohibited, else false
 */
__STATIC_FORCEINLINE bool hw_fcu_are_flash_operations_prohibited(void)
{
        return REG_GETF(FCU, FLASH_CTRL_REG, FLASH_PROT) == 1;
}

/**
 * \brief Check if flash erase is suspended
 *
 * \note If flash erase suspend is true, check FLASH_PTERASE_REG for the remaining time needed
 *       for the erase to complete. If flash erase suspend is false, check PROG_ERS status bit
 *       if an erase is in progress.
 *
 * \return true if flash erase is suspended, else false
 */
__STATIC_FORCEINLINE bool hw_fcu_is_flash_erase_suspended(void)
{
        return REG_GETF(FCU, FLASH_CTRL_REG, ERASE_SUSPEND_STAT) == 1;
}

/**
 * \brief Set erase/suspend mode
 *
 * \param [in] mode erase/suspend mode
 *
 * \sa HW_FCU_ERASE_SUSPEND_MODE
 */
__STATIC_FORCEINLINE void hw_fcu_set_erase_suspend_mode(HW_FCU_ERASE_SUSPEND_MODE mode)
{
        REG_SETF(FCU, FLASH_CTRL_REG, ERASE_SUSPEND_MODE, mode);
}

/**
 * \brief Get erase/suspend mode
 *
 * \return erase/suspend mode
 *
 * \sa HW_FCU_ERASE_SUSPEND_MODE
 */
__STATIC_FORCEINLINE HW_FCU_ERASE_SUSPEND_MODE hw_fcu_get_erase_suspend_mode(void)
{
        return REG_GETF(FCU, FLASH_CTRL_REG, ERASE_SUSPEND_MODE);
}

/**
 * \brief Enable erase/suspend functionality
 *
 * \note HW supports automatic suspend on page erase command, when a read access is occurred on
 *       different page
 */
__STATIC_FORCEINLINE void hw_fcu_enable_erase_suspend(void)
{
        REG_SET_BIT(FCU, FLASH_CTRL_REG, ERASE_SUSPEND_EN);
}

/**
 * \brief Disable erase/suspend functionality
 */
__STATIC_FORCEINLINE void hw_fcu_disable_erase_suspend(void)
{
        REG_CLR_BIT(FCU, FLASH_CTRL_REG, ERASE_SUSPEND_EN);
}

/**
 * \brief Check if erase/suspend functionality is enabled
 *
 * \return true if erase/suspend functionality is enabled, else false
 */
__STATIC_FORCEINLINE bool hw_fcu_is_erase_suspend_enabled(void)
{
        return REG_GETF(FCU, FLASH_CTRL_REG, ERASE_SUSPEND_EN) == 1;
}

/**
 * \brief Check if fcu is in sleeping mode
 *
 * \return true if FCU is in sleeping mode, else false (FCU is in standby mode)
 *
 */
__STATIC_FORCEINLINE bool hw_fcu_is_asleep(void)
{
        return REG_GETF(FCU, FLASH_CTRL_REG, SLEEP) == 1;
}

/**
 * \brief Enable DMA handshake when writing to the FCU
 *
 * \note It also disables IRQ generation.
 */
__STATIC_FORCEINLINE void hw_fcu_enable_dma(void)
{
        REG_SET_BIT(FCU, FLASH_CTRL_REG, DMA_EN);
}

/**
 * \brief Disable DMA handshake when writing to the FCU
 *
 * \note IRQ generation remains enabled.
 */
__STATIC_FORCEINLINE void hw_fcu_disable_dma(void)
{
        REG_CLR_BIT(FCU, FLASH_CTRL_REG, DMA_EN);
}

/**
 * \brief Check if DMA handshake when writing to the FCU is enabled
 *
 * \return true if DMA handshake when writing to the FCU is enabled, else false
 */
__STATIC_FORCEINLINE bool hw_fcu_is_dma_enabled(void)
{
        return REG_GETF(FCU, FLASH_CTRL_REG, DMA_EN) == 1;
}

/**
 * \brief Enable bus error response
 *
 * \note This bit can only be set and is reset by hardware reset.
 *       The BUS_ERROR status flag isn't affected by setting this bit.
 */
__STATIC_FORCEINLINE void hw_fcu_enable_bus_error(void)
{
        REG_SET_BIT(FCU, FLASH_CTRL_REG, BUS_ERROR_EN);
}

/**
 * \brief Check if bus error response is enabled
 *
 * \return true if bus error response is enabled, else false
 */
__STATIC_FORCEINLINE bool hw_fcu_is_bus_error_enabled(void)
{
        return REG_GETF(FCU, FLASH_CTRL_REG, BUS_ERROR_EN) == 1;
}

/**
 * \brief Get bus error status
 *
 * \return true if bus error occurred in the last FCU AHB access, else false
 *
 * \note Register gets cleared on next FCU AHB access.
 */
__STATIC_FORCEINLINE bool hw_fcu_get_bus_error_status(void)
{
        return REG_GETF(FCU, FLASH_CTRL_REG, BUS_ERROR) == 1;
}

/**
 * \brief Clear FCU interrupt
 */
__STATIC_FORCEINLINE void hw_fcu_clear_interrupt(void)
{
        REG_SET_BIT(FCU, FLASH_CTRL_REG, IRQ_CLEAR);
}

/**
 * \brief Get wait cycles
 *
 * \return wait cycles
 *
 * \sa HW_FCU_WAIT_CYCLES
 */
__STATIC_FORCEINLINE HW_FCU_WAIT_CYCLES hw_fcu_get_wait_cycles(void)
{
        return REG_GETF(FCU, FLASH_CTRL_REG, WAIT_CYCLES);
}

/**
 * \brief Set wait cycles
 *
 * \note Use cases:
 *       HCLK = 64 MHz, VDD = 0.9V -> 4 wait cycles
 *       HCLK = 64 MHz, VDD = 1.2V -> 1 wait cycles
 *       HCLK = 32 MHz, VDD = 0.9V -> 2 wait cycles (default conditions on power up)
 *       HCLK = 32 MHz, VDD = 1.2V -> 0 wait cycles
 *       HCLK = 16 MHz, VDD = 0.9V -> 1 wait cycles
 *       HCLK = 16 MHz, VDD = 1.2V -> 0 wait cycles
 *       HCLK = slower than above -> 0 wait cycles
 *
 * \param [in] wait_cycles (allowed values 0...7)
 *
 * \sa HW_FCU_WAIT_CYCLES
 */
__STATIC_FORCEINLINE void hw_fcu_set_wait_cycles(HW_FCU_WAIT_CYCLES wait_cycles)
{
        ASSERT_WARNING(wait_cycles < HW_FCU_WAIT_CYCLES_MAX);

        REG_SETF(FCU, FLASH_CTRL_REG, WAIT_CYCLES, wait_cycles);
}

/**
 * Check if FCU is protected against specific actions
 *
 * Use the input argument mask by OR-ing one or more of the following arguments to check whether
 * specific FCU protections are enabled.
 *
 * - Read protection:                   REG_MSK(FCU, FLASH_CTRL_REG, FLASH_RPROT)
 * - Write/Erase protection:            REG_MSK(FCU, FLASH_CTRL_REG, FLASH_WPROT)
 * - Read/Write/Erase protection:       REG_MSK(FCU, FLASH_CTRL_REG, FLASH_PROT)
 * - FLASH read mode inhibit:           REG_MSK(FCU, FLASH_CTRL_REG, PROG_RMIN)
 *
 * \param [in] mask Mask of the actions to check
 *
 * return True, if any protection is enabled, otherwise false.
 */
__STATIC_FORCEINLINE bool hw_fcu_is_protected_against_actions(uint32_t mask)
{
     return ((FCU->FLASH_CTRL_REG & mask) != 0);
}

/**
 * \brief Enable program flash read protection
 *
 * \note This bit can only be set and is reset by hardware reset.
 */
__STATIC_FORCEINLINE void hw_fcu_enable_flash_read_protection(void)
{
        REG_SET_BIT(FCU, FLASH_CTRL_REG, FLASH_RPROT);
}

/**
 * \brief Check program flash read protection
 *
 * \return true if the program flash cannot be read, else false
 */
__STATIC_FORCEINLINE bool hw_fcu_is_flash_read_protection_enabled(void)
{
        return REG_GETF(FCU, FLASH_CTRL_REG, FLASH_RPROT) == 1;
}

/**
 * \brief Enable program flash write protection
 *
 * \note This bit can only be set and is reset by hardware reset.
 */
__STATIC_FORCEINLINE void hw_fcu_enable_flash_write_protection(void)
{
        REG_SET_BIT(FCU, FLASH_CTRL_REG, FLASH_WPROT);
}

/**
 * \brief Check program flash write protection
 *
 * \return true if the program flash cannot be written, else false
 */
__STATIC_FORCEINLINE bool hw_fcu_is_flash_write_protection_enabled(void)
{
        return REG_GETF(FCU, FLASH_CTRL_REG, FLASH_WPROT) == 1;
}

/**
 * \brief Check if program flash read mode is inhibited
 *
 * \return true if program flash read mode is inhibited, else false
 */
__STATIC_FORCEINLINE bool hw_fcu_is_flash_read_mode_inhibited(void)
{
        return REG_GETF(FCU, FLASH_CTRL_REG, PROG_RMIN) == 1;
}

/**
 * \brief Check if flash erase is in progress
 *
 * \return true if erase cycle in progress, else false
 *
 */
__STATIC_FORCEINLINE bool hw_fcu_is_erase_in_progress(void)
{
        return REG_GETF(FCU, FLASH_CTRL_REG, PROG_ERS) == 1;
}

/**
 * \brief Resume erase
 *
 * \note It should be called when ERASE_SUSPEND_EN == 1 and ERASE_SUSPEND_STAT == 1 to resume
 *       erasing.
 */
__STATIC_FORCEINLINE void hw_fcu_resume_erase(void)
{
        uint32_t reg_val = FCU->FLASH_CTRL_REG;
        uint32_t mask = REG_MSK(FCU, FLASH_CTRL_REG, ERASE_SUSPEND_EN) |
                        REG_MSK(FCU, FLASH_CTRL_REG, ERASE_SUSPEND_STAT);

        if (mask == (reg_val & mask)) {
                REG_SET_BIT(FCU, FLASH_CTRL_REG, ERASE_RESUME);
                while (!hw_fcu_is_erase_in_progress());
        }
}

/**
 * \brief Check if flash write is in progress
 *
 * \return true if write cycle in progress, else false
 *
 */
__STATIC_FORCEINLINE bool hw_fcu_is_write_in_progress(void)
{
        return REG_GETF(FCU, FLASH_CTRL_REG, PROG_WRS) == 1;
}

/**
 * \brief Set flash read or write/erase mode
 *
 * \param [in] mode flash read/write/erase mode
 *
 * \sa HW_FCU_FLASH_ACCESS_MODE
 */
__STATIC_FORCEINLINE void hw_fcu_set_flash_access_mode(HW_FCU_FLASH_ACCESS_MODE mode)
{
        REG_SETF(FCU, FLASH_CTRL_REG, PROG_SEL, mode);
}

/**
 * \brief Get flash read/write/erase mode
 *
 * \return flash read/write/erase mode
 *
 * \sa HW_FCU_FLASH_ACCESS_MODE
 */
__STATIC_FORCEINLINE HW_FCU_FLASH_ACCESS_MODE hw_fcu_get_flash_access_mode(void)
{
        return REG_GETF(FCU, FLASH_CTRL_REG, PROG_SEL);
}

/**
 * \brief Set program flash mode
 *
 * \param [in] mode program flash mode
 *
 * \sa HW_FCU_FLASH_MODE
 */
__STATIC_FORCEINLINE void hw_fcu_set_flash_programming_mode(HW_FCU_FLASH_PROG_MODE mode)
{
        REG_SETF(FCU, FLASH_CTRL_REG, PROG_MODE, mode);
}

/**
 * \brief Get program flash mode
 *
 * \return program flash mode
 *
 * \sa HW_FCU_FLASH_MODE
 */
__STATIC_FORCEINLINE HW_FCU_FLASH_PROG_MODE hw_fcu_get_flash_programming_mode(void)
{
        return REG_GETF(FCU, FLASH_CTRL_REG, PROG_MODE);
}

/**
 * \brief Puts FCU (eFlash) in sleep mode for lower leakage current
 *
 * \note Automatically wakes up on read access.
 */
__STATIC_FORCEINLINE void hw_fcu_set_sleep_mode(void)
{
        REG_SET_BIT(FCU, FLASH_CTRL_REG, SLEEP_MODE);
}

/**
 * \brief Set program flash NVSTR1 hold time
 *
 * \param [in] hold_time hold time in microseconds. T = PTNVH1 x 1 us
 *
 */
__STATIC_FORCEINLINE void hw_fcu_set_flash_hold_time(uint16_t hold_time)
{
        ASSERT_WARNING(hold_time >= 100);
        REG_SETF(FCU, FLASH_PTNVH1_REG, PTNVH1, hold_time);
}

/**
 * \brief Get program flash NVSTR1 hold time
 *
 * \return program flash NVSTR1 hold time in microseconds
 */
__STATIC_FORCEINLINE uint16_t hw_fcu_get_flash_hold_time(void)
{
        return REG_GETF(FCU, FLASH_PTNVH1_REG, PTNVH1);
}

/**
 * \brief Set program flash programming time
 *
 * \param [in] prog_time programming time in microseconds. T = PTPROG x 1 us
 *
 */
__STATIC_FORCEINLINE void hw_fcu_set_flash_program_time(uint16_t prog_time)
{
        ASSERT_WARNING(prog_time >= 8);
        REG_SETF(FCU, FLASH_PTPROG_REG, PTPROG, prog_time);
}

/**
 * \brief Get program flash programming time
 *
 * \return program flash programming time in microseconds
 */
__STATIC_FORCEINLINE uint16_t hw_fcu_get_flash_program_time(void)
{
        return REG_GETF(FCU, FLASH_PTPROG_REG, PTPROG);
}

/**
 * \brief Set program flash page erase time
 *
 * \param [in] erase_time page erase time in microseconds. T= PTERASE x 1 us
 *
 */
__STATIC_FORCEINLINE void hw_fcu_set_flash_page_erase_time(uint32_t erase_time)
{
        ASSERT_WARNING(erase_time >= HW_FCU_ERASE_FLASH_PAGE_MIN_TIME &&
                       erase_time < HW_FCU_ERASE_FLASH_PAGE_MAX_TIME + 1);
        REG_SETF(FCU, FLASH_PTERASE_REG, PTERASE, erase_time);
}

/**
 * \brief Get program flash page erase time
 *
 * \return program flash page erase time in microseconds
 */
__STATIC_FORCEINLINE uint32_t hw_fcu_get_flash_page_erase_time(void)
{
        return REG_GETF(FCU, FLASH_PTERASE_REG, PTERASE);
}

/**
 * \brief Set program flash mass erase time
 *
 * \param [in] erase_time mass erase time in microseconds. T= PTME x 1 us
 *
 */
__STATIC_FORCEINLINE void hw_fcu_set_flash_mass_erase_time(uint32_t erase_time)
{
        ASSERT_WARNING(erase_time >= HW_FCU_ERASE_FLASH_BLOCK_MIN_TIME &&
                       erase_time < HW_FCU_ERASE_FLASH_BLOCK_MAX_TIME + 1);
        REG_SETF(FCU, FLASH_PTME_REG, PTME, erase_time);
}

/**
 * \brief Get program flash mass erase time
 *
 * \return program flash mass erase time in microseconds
 */
__STATIC_FORCEINLINE uint32_t hw_fcu_get_flash_mass_erase_time(void)
{
        return REG_GETF(FCU, FLASH_PTME_REG, PTME);
}

/**
 * \brief Set program flash sleep to standby wake-up time
 *
 * \param [in] wakeup_time sleep to standby wake-up time in microseconds. T = PTWK_SP x 1us
 */
__STATIC_FORCEINLINE void hw_fcu_set_flash_sleep_to_standby_time(uint8_t wakeup_time)
{
        ASSERT_WARNING(wakeup_time >= 3);
        REG_SETF(FCU, FLASH_PTWK_SP_REG, PTWK_SP, wakeup_time);
}

/**
 * \brief Get program flash sleep to standby wake-up time
 *
 * \return program flash sleep to standby wake-up time in microseconds
 */
__STATIC_FORCEINLINE uint8_t hw_fcu_get_flash_sleep_to_standby_time(void)
{
        return REG_GETF(FCU, FLASH_PTWK_SP_REG, PTWK_SP);
}

/**
 * \brief Set program flash page segment erase time for suspend erase
 *
 * \param [in] erase_time page segment erase time for suspend erase in microseconds.
 */
__STATIC_FORCEINLINE void hw_fcu_set_flash_page_segment_erase_time(uint32_t erase_time)
{
        ASSERT_WARNING(erase_time >= HW_FCU_ERASE_FLASH_PAGE_SEGMENT_MIN_TIME &&
                       erase_time < HW_FCU_ERASE_FLASH_PAGE_SEGMENT_MAX_TIME + 1);
        REG_SETF(FCU, FLASH_PTERASE_SEG_REG, PTERASE_SEG, erase_time);
}

/**
 * \brief Get program flash page segment erase time for suspend erase
 *
 * \return program flash page segment erase time for suspend erase in microseconds
 */
__STATIC_FORCEINLINE uint32_t hw_fcu_get_flash_page_segment_erase_time(void)
{
        return REG_GETF(FCU, FLASH_PTERASE_SEG_REG, PTERASE_SEG);
}

/**
 * \brief Get total erase time counter value
 *
 * \return total erase time counter value in microseconds
 */
__STATIC_FORCEINLINE uint32_t hw_fcu_get_total_erase_counter(void)
{
        return REG_GETF(FCU, FLASH_RTERASE_TOT_CNT_REG, RTERASE_TOT_CNT);
}

/**
 * \brief Get segment erase time counter value
 *
 * \return segment erase time counter value in microseconds
 */
__STATIC_FORCEINLINE uint32_t hw_fcu_get_segment_erase_counter(void)
{
        return REG_GETF(FCU, FLASH_RTERASE_SEG_CNT_REG, RTERASE_SEG_CNT);
}

/**
 * \brief Enables reset delay when FCU write/erase begins.
 */
__STATIC_FORCEINLINE void hw_fcu_enable_reset_delay(void)
{
        REG_SET_BIT(CRG_TOP, RST_CTRL_REG, GATE_RST_WITH_FCU);
}

/**
 * \brief Disables reset delay when FCU write/erase has finished.
 */
__STATIC_FORCEINLINE void hw_fcu_disable_reset_delay(void)
{
        REG_CLR_BIT(CRG_TOP, RST_CTRL_REG, GATE_RST_WITH_FCU);
}

/**
 * \brief Wakeup FCU
 */
__ALWAYS_RETAINED_CODE void hw_fcu_wakeup(void);

/**
 * \brief Set FCU to sleep mode
 */
__ALWAYS_RETAINED_CODE void hw_fcu_sleep(void);

/**
 * \brief Put FCU in erase mode
 *
 * \param [in] mode flash page/block erase mode
 */
__ALWAYS_RETAINED_CODE void hw_fcu_enable_erase(HW_FCU_FLASH_PROG_MODE mode);

/**
 * \brief Erase flash block's page
 *
 * \note  If there is no user callback registered, erasing will be performed in blocking mode.
 *
 * \param [in] address page address
 * \param [in] params pointer to operation parameters structure
 *
 * \return HW_FCU_ERROR_NONE on success, else error
 *
 * \sa HW_FCU_ERROR
 * \sa hw_fcu_register_user_cb
 */
__ALWAYS_RETAINED_CODE HW_FCU_ERROR hw_fcu_erase_page(uint32_t address,
                                               struct hw_fcu_operation_params_t *params);

/**
 * \brief Configure page erase suspend/resume
 *
 * \param [in] mode mode of erase suspend operation
 * \param [in] page_erase_time flash page erase time in microseconds
 * \param [in] segment_erase_time flash page segment erase time in microseconds
 *
 * \return HW_FCU_ERROR_NONE on success, else error
 *
 * \sa HW_FCU_ERROR
 */
__ALWAYS_RETAINED_CODE HW_FCU_ERROR hw_fcu_configure_erase_page_suspend(HW_FCU_ERASE_SUSPEND_MODE mode,
                                                                 uint32_t page_erase_time,
                                                                 uint32_t segment_erase_time);

/**
 * \brief Erase flash block
 *
 * \note  If there is no user callback registered, erasing will be performed in blocking mode.
 *
 * \param [in] address flash address.
 * \param [in] params pointer to operation parameters structuree
 *
 * \note If eFLASH address parameter is between 0x0 and 0x40000, eFLASH space between 0x40000 and
 *       0x40800 won't be erased. If address parameter is between 0x40000 and 0x40800, all eFLASH
 *       space (0x0 - 0x40800) will be erased.
 *
 * \return HW_FCU_ERROR_NONE on success, else error
 *
 * \sa HW_FCU_ERROR
 * \sa hw_fcu_register_user_cb
 */
__ALWAYS_RETAINED_CODE HW_FCU_ERROR hw_fcu_erase_block(uint32_t address,
                                                struct hw_fcu_operation_params_t *params);

/**
 * \brief Put FCU in write mode
 */
__ALWAYS_RETAINED_CODE void hw_fcu_enable_write(void);

/**
 * \brief Write a buffer to flash address (first flash page should be erased)
 *
 * \note  If there is no user callback registered, writing will be performed in blocking mode.
 *
 * \param [in] src buffer to be written (avoid casting uint8_t* or uint16_t* to uint32_t*)
 * \param [in] address flash address
 * \param [in] len buffer size in 32bit words
 * \param [in] params pointer to operation parameters structure
 *
 * \return HW_FCU_ERROR_NONE on success, else error
 *
 * \sa HW_FCU_ERROR
 * \sa hw_fcu_register_user_cb
 */
__ALWAYS_RETAINED_CODE HW_FCU_ERROR hw_fcu_write(uint32_t *src, uint32_t address, uint32_t len,
                                          struct hw_fcu_operation_params_t *params);

/**
 * \brief Put FCU in read mode
 */
__ALWAYS_RETAINED_CODE void hw_fcu_enable_read(void);

/**
 * \brief Read from flash address to buffer
 *
 * \note  If there is no user callback registered or a valid DMA channel, reading will be performed
 *        in blocking mode.
 *
 * \note  If FCU is in erase suspend state, then the state is reset, see \sa hw_fcu_enable_read.
 *
 * \param [in] address flash address
 * \param [in] dst buffer to store data (avoid casting uint8_t* or uint16_t* to uint32_t*)
 * \param [in] len buffer size in 32bit words
 * \param [in] params pointer to operation parameters structure
 *
 * \return HW_FCU_ERROR_NONE on success, else error
 *
 * \sa HW_FCU_ERROR
 * \sa hw_fcu_register_user_cb
 */
__ALWAYS_RETAINED_CODE HW_FCU_ERROR hw_fcu_read(uint32_t address, uint32_t *dst, uint32_t len,
                                         struct hw_fcu_operation_params_t *params);

/**
 * \brief Check if FCU is not busy or in sleeping mode
 *
 * \return true if FCU is available, else false
 */
__ALWAYS_RETAINED_CODE bool hw_fcu_is_available(void);

#endif /* dg_configUSE_HW_FCU */
#endif /* HW_FCU_H_ */

/**
 * \}
 * \}
 */
