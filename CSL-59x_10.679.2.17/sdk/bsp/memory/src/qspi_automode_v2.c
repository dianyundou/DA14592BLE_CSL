/**
 ****************************************************************************************
 *
 * @file qspi_automode_v2.c
 *
 * @brief QSPI Memory Abstraction Interface
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

#if dg_configQSPI_AUTOMODE_ENABLE

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "sdk_defs.h"

#include "hw_clk.h"
#include "hw_pmu.h"

#include "hw_qspi.h"
#include "hw_cache.h"
#include "qspi_automode.h"
#include "qspi_automode_internal_v2.h"

__RETAINED_RW static HW_QSPI_BUS_MODE manual_access_bus_mode = HW_QSPI_BUS_MODE_SINGLE;

__RETAINED_CODE static void qspi_set_manual_access_bus_mode(HW_QSPIC_ID id, HW_QSPI_BUS_MODE bus_mode, bool forced);
__RETAINED_CODE static void qspi_flash_write_enable(HW_QSPIC_ID id);
__RETAINED_CODE static bool qspi_flash_is_busy(HW_QSPIC_ID id);
__RETAINED_CODE static uint8_t qspi_flash_read_status_register(HW_QSPIC_ID id);
__UNUSED __RETAINED_CODE static void qspi_flash_write_status_register(HW_QSPIC_ID id, uint8_t value);
__RETAINED_CODE static void qspi_enter_manual_access_mode(HW_QSPIC_ID id);
__RETAINED_CODE static void qspi_flash_write_enable(HW_QSPIC_ID id);
__UNUSED __RETAINED_CODE static void qspi_enter_qpi_mode(HW_QSPIC_ID id);
__UNUSED __RETAINED_CODE static bool qspi_exit_qpi(HW_QSPIC_ID id);

#define READ_PIPE_DELAY_0V9             (HW_QSPI_READ_PIPE_DELAY_2)
#define READ_PIPE_DELAY_1V2             (HW_QSPI_READ_PIPE_DELAY_7)

#define QSPIC_INSTANCES                 (1)
#define GET_QSPIC_ID(idx)               (HW_QSPIC)

#ifndef QSPI_AUTOMODE_SANITY_CHECK
#define QSPI_AUTOMODE_SANITY_CHECK      (1)
#endif

#if (dg_configQSPI_FLASH_AUTODETECT == 0)
        #if !defined(dg_configQSPI_FLASH_CONFIG)
        #error Please define dg_configQSPI_FLASH_CONFIG
        #endif
#endif

#if dg_configQSPI_FLASH_AUTODETECT
# define QSPI_GET_PARAM(id, param) (qspi_flash_config.param)
#else
# define QSPI_GET_PARAM(id, param) (dg_configQSPI_FLASH_CONFIG.param)
#endif

#if dg_configQSPI_FLASH_AUTODETECT
# include dg_configQSPI_MEMORY_CONFIG_TABLE_HEADER

__RETAINED static qspi_flash_config_t qspi_flash_config;

#else /* dg_configQSPI_FLASH_AUTODETECT */
# ifndef dg_configQSPI_FLASH_HEADER_FILE
# error dg_configQSPI_FLASH_HEADER_FILE must be defined
# endif
#include dg_configQSPI_FLASH_HEADER_FILE
#endif /* dg_configQSPI_FLASH_AUTODETECT */

/**
 * \brief Get the QSPIC ID based on the virtual address of the data.
 *
 * \param[in] addr The virtual address to access.
 * \param[in] len  The number of bytes to access.
 *
 * \return QSPI controller id
 */
static HW_QSPIC_ID get_hw_qspic_id(uint32_t addr, uint32_t len)
{
        ASSERT_WARNING(qspi_automode_is_valid_virtual_address_range(addr, len));

        return HW_QSPIC;
}

__STATIC_FORCEINLINE uint32_t get_zero_based_addr(uint32_t virtual_addr)
{
        return virtual_addr - QSPI_MEM1_VIRTUAL_BASE_ADDR;
}

/**
 * The read pipe clock delay depends on the voltage level of the 1V2 power rail.
 * According to the hw specifications the optimal settings are:
 * - POWER_RAIL_1V2 = 0V9 --> Read pipe delay = 2
 * - POWER_RAIL_1V2 = 1V2 --> Read pipe delay = 7
 *
 * Moreover, the voltage level of the 1V2 power rail relates to the system clock frequency:
 * - SYS_CLK_FREQ = 32 MHz --> POWER_RAIL_1V2 = 0V9
 * - SYS_CLK_FREQ > 32 MHz --> POWER_RAIL_1V2 = 1V2
 *
 * The read pipe clock delay is set based on the system clock frequency because it's more convenient.
 *
 *  Allowed settings
 * -----------------------------------------------------------------------------|
 * | System clock frequency | 1V2 voltage level | Read pipe delay | Recommended |
 * -----------------------------------------------------------------------------|
 * |        32MHz           |        0V9        |        2        |      Y      |
 * |        32MHz           |        1V2        |        2        |      N      |
 * |        32MHz           |        1V2        |        7        |      N      |
 * |        64MHz           |        1V2        |        7        |      Y      |
 * ------------------------------------------------------------------------------
 *
 *  Forbidden settings
 * ----------------------------------------------------------------
 * | System clock frequency | 1V2 voltage level | Read pipe delay |
 * ----------------------------------------------------------------
 * |          32MHz         |        0V9        |        7        |
 * |          64Mhz         |        0V9        |        x        |
 * |          64Mhz         |        1V2        |        0        |
 * ----------------------------------------------------------------
 *
 * x: don't care
 */
__RETAINED_CODE static void qspi_set_read_pipe_clock_delay(HW_QSPIC_ID id, sys_clk_t sys_clk)
{
        HW_QSPI_READ_PIPE_DELAY read_pipe_delay = READ_PIPE_DELAY_0V9;

        if (sys_clk > sysclk_XTAL32M) {
                read_pipe_delay = READ_PIPE_DELAY_1V2;
        }

        hw_qspi_set_read_pipe_clock_delay(id, read_pipe_delay);
}

/**
 * \brief Check if the device is busy
 *
 * \return bool True if the BUSY bit is set else false.
 *
 * \warning This function checks the value of the BUSY bit in the Status Register 1 of the Flash. It
 *          is the responsibility of the caller to call the function in the right context. The
 *          function must be called with interrupts disabled.
 *
 */
__RETAINED_CODE static bool qspi_flash_is_busy(HW_QSPIC_ID id)
{
        __DBG_QSPI_VOLATILE__ HW_QSPI_BUSY_LEVEL busy_level = QSPI_GET_PARAM(id, read_status_instr_cfg.busy_level);

        return QSPI_GET_PARAM(id, callback.is_busy_cb)(id, busy_level);
}

/**
 * \brief Read the Status Register 1 of the Flash
 *
 * \return uint8_t The value of the Status Register 1 of the Flash.
 */
__RETAINED_CODE static uint8_t qspi_flash_read_status_register(HW_QSPIC_ID id)
{
        __DBG_QSPI_VOLATILE__ uint8_t status;

        status = QSPI_GET_PARAM(id, callback.read_status_reg_cb)(id);

        return status;
}

/**
 * \brief Write the Status Register 1 of the Flash
 *
 * \param[in] value The value to be written.
 *
 * \note This function blocks until the Flash has processed the command. No verification that the
 *       value has been actually written is done though. It is up to the caller to decide whether
 *       such verification is needed or not and execute it on its own.
 */
__RETAINED_CODE __UNUSED static void qspi_flash_write_status_register(HW_QSPIC_ID id, uint8_t value)
{
        QSPI_GET_PARAM(id, callback.write_status_reg_cb)(id, value);

        /* Wait for the Flash to process the command */
        while (qspi_flash_is_busy(id));
}

/**
 * \brief Write an arbitrary number of bytes to the Flash and then read an arbitrary number of bytes
 *        from the Flash in one transaction
 *
 * \param[in] wbuf Pointer to the beginning of the buffer that contains the data to be written
 * \param[in] wlen The number of bytes to be written
 * \param[in] rbuf Pointer to the beginning of the buffer than the read data are stored
 * \param[in] rlen The number of bytes to be read
 *
 * \note The data are transferred as bytes (8 bits wide). No optimization is done in trying to use
 *       faster access methods (i.e. transfer words instead of bytes whenever it is possible).
 */
__STATIC_INLINE void qspi_flash_transact(HW_QSPIC_ID id, const uint8_t *wbuf, uint32_t wlen,
                                         uint8_t *rbuf, uint32_t rlen)
{
        hw_qspi_cs_enable(id);

        for (uint32_t i = 0; i < wlen; ++i) {
                hw_qspi_write8(id, wbuf[i]);
        }

        for (uint32_t i = 0; i < rlen; ++i) {
                rbuf[i] = hw_qspi_read8(id);
        }

        hw_qspi_cs_disable(id);
}

/*
 * Send flash command
 */
__RETAINED_CODE static void qspi_flash_cmd(HW_QSPIC_ID id, const uint8_t opcode)
{
        hw_qspi_cs_enable(id);
        hw_qspi_write8(id, opcode);
        hw_qspi_cs_disable(id);
}

/**
 * \brief Set WEL (Write Enable Latch) bit of the Status Register of the Flash
 * \details The WEL bit must be set prior to every Page Program, Quad Page Program, Sector Erase,
 *       Block Erase, Chip Erase, Write Status Register and Erase/Program Security Registers
 *       instruction. In the case of Write Status Register command, any status bits will be written
 *       as non-volatile bits.
 *
 *
 * \note This function blocks until the Flash has processed the command and it will be repeated if,
 *       for any reason, the command was not successfully executed by the Flash.
 */
__RETAINED_CODE static void qspi_flash_write_enable(HW_QSPIC_ID id)
{
        __DBG_QSPI_VOLATILE__ uint8_t status;
        __DBG_QSPI_VOLATILE__ uint8_t opcode = QSPI_GET_PARAM(id, write_enable_instr_cfg.opcode);

        do {
                qspi_flash_cmd(id, opcode);
                /* Verify */
                do {
                        status = qspi_flash_read_status_register(id);
                } while (status & QSPI_STATUS_REG_BUSY_MASK);
        } while (!(status & QSPI_STATUS_REG_WEL_MASK));
}

__RETAINED_CODE static void qspi_set_manual_access_bus_mode(HW_QSPIC_ID id, HW_QSPI_BUS_MODE bus_mode, bool forced)
{
        if ((manual_access_bus_mode != bus_mode) || forced) {
                hw_qspi_set_manual_access_bus_mode(id, bus_mode);
                hw_qspi_set_io(id, bus_mode);
                manual_access_bus_mode = bus_mode;
        }
}

/*
 * In order to exit from continuous mode of operation the QSPI_EXIT_CONTINUOUS_MODE_OPCODE must be
 * shifted in the extra byte phase of a read access command.
 */
__RETAINED_CODE static void qspi_flash_exit_continuous_mode_cmd(HW_QSPIC_ID id, HW_QSPI_ADDR_SIZE addr_size)
{
        hw_qspi_cs_enable(id);

        if (addr_size == HW_QSPI_ADDR_SIZE_32) {
                hw_qspi_write16(id, 0xFFFF);
        } else {
                hw_qspi_write8(id, QSPI_EXIT_CONTINUOUS_MODE_BYTE);
        }

        hw_qspi_cs_disable(id);
}

__RETAINED_CODE static void qspi_flash_exit_continuous_mode(HW_QSPIC_ID id)
{
        __DBG_QSPI_VOLATILE__ HW_QSPI_ADDR_SIZE addr_size = QSPI_GET_PARAM(id, address_size);

        qspi_flash_exit_continuous_mode_cmd(id, addr_size);
}

/**
 * Enter QPI mode
 */
__UNUSED __RETAINED_CODE static void qspi_enter_qpi_mode(HW_QSPIC_ID id)
{
        hw_qspi_cs_enable(id);
        hw_qspi_write8(id, QSPI_ENTER_QPI_OPCODE);
        hw_qspi_cs_disable(id);
}

/**
 * Exit QPI mode
 */
__UNUSED __RETAINED_CODE static bool qspi_exit_qpi(HW_QSPIC_ID id)
{
        hw_qspi_cs_enable(id);
        hw_qspi_write8(id, QSPI_EXIT_QPI_OPCODE);
        hw_qspi_cs_disable(id);

        return true;
}

/*
 * Enter Manual Access Mode. This function turns the QSPI Flash memory out of the Continuous Mode
 * of operation, if enabled.
 */
__RETAINED_CODE static void qspi_enter_manual_access_mode(HW_QSPIC_ID id)
{
        __DBG_QSPI_VOLATILE__ HW_QSPI_CONTINUOUS_MODE continuous_mode = QSPI_GET_PARAM(id, read_instr_cfg.continuous_mode);

        if (hw_qspi_get_access_mode(id) == HW_QSPI_ACCESS_MODE_AUTO) {
                hw_qspi_set_access_mode(id, HW_QSPI_ACCESS_MODE_MANUAL);

                if (continuous_mode == HW_QSPI_CONTINUOUS_MODE_ENABLE) {
                        qspi_flash_exit_continuous_mode(id);
                }
        }
}

__RETAINED_CODE void qspi_automode_int_enter_auto_access_mode(HW_QSPIC_ID id)
{
        /*
         * Before switching to Auto Access Mode set the direction of all QSPIC IOs so that they are
         * selected by the controller.
         */
        __DBG_QSPI_VOLATILE__ HW_QSPI_BUS_MODE bus_mode = QSPI_GET_PARAM(id, read_instr_cfg.data_bus_mode);

        hw_qspi_set_io(id, bus_mode);
        hw_qspi_set_access_mode(id, HW_QSPI_ACCESS_MODE_AUTO);
}

/**
 * \brief Fast copy of a buffer to a FIFO
 * \details Implementation of a fast copy of the contents of a buffer to a FIFO in assembly. All
 *        addresses are word aligned.
 *
 * \param[in] start Pointer to the beginning of the buffer
 * \param[in] end Pointer to the end of the buffer
 * \param[in] Pointer to the FIFO
 *
 * \warning No validity checks are made! It is the responsibility of the caller to make sure that
 *        sane values are passed to this function.
 */
__STATIC_FORCEINLINE void fast_write_to_fifo32(uint32_t start, uint32_t end, uint32_t dest)
{
        asm volatile(   "copy:                                  \n"
                        "       ldmia %[start]!, {r3}           \n"
                        "       str r3, [%[dest]]               \n"
                        "       cmp %[start], %[end]            \n"
                        "       blt copy                        \n"
                        :
                        :                                                         /* output */
                        [start] "l" (start), [end] "r" (end), [dest] "l" (dest) : /* inputs (%0, %1, %2) */
                        "r3");                                              /* registers that are destroyed */
}

__RETAINED_CODE uint32_t qspi_automode_int_flash_write_page(HW_QSPIC_ID id, uint32_t addr, const uint8_t *buf, uint32_t size)
{
        __DBG_QSPI_VOLATILE__ uint32_t i = 0;
        __DBG_QSPI_VOLATILE__ uint32_t odd = ((uint32_t) buf) & 3;
        __DBG_QSPI_VOLATILE__ uint32_t size_aligned32;
        __DBG_QSPI_VOLATILE__ uint32_t page_boundary;
        __DBG_QSPI_VOLATILE__ uint8_t opcode = QSPI_GET_PARAM(id, page_program_instr_cfg.opcode);
        __DBG_QSPI_VOLATILE__ HW_QSPI_BUS_MODE write_enable_bus_mode = QSPI_GET_PARAM(id, write_enable_instr_cfg.opcode_bus_mode);
        __DBG_QSPI_VOLATILE__ HW_QSPI_BUS_MODE opcode_bus_mode = QSPI_GET_PARAM(id, page_program_instr_cfg.opcode_bus_mode);
        __DBG_QSPI_VOLATILE__ HW_QSPI_BUS_MODE addr_bus_mode = QSPI_GET_PARAM(id, page_program_instr_cfg.addr_bus_mode);
        __DBG_QSPI_VOLATILE__ HW_QSPI_BUS_MODE data_bus_mode = QSPI_GET_PARAM(id, page_program_instr_cfg.data_bus_mode);
        __DBG_QSPI_VOLATILE__ HW_QSPI_ADDR_SIZE addr_size = QSPI_GET_PARAM(id, address_size);

        DBG_SET_HIGH(FLASH_DEBUG, FLASHDBG_PAGE_PROG);

        /* Reduce max write size, that can reduce interrupt latency time */
        if (size > dg_configQSPI_FLASH_MAX_WRITE_SIZE) {
                size = dg_configQSPI_FLASH_MAX_WRITE_SIZE;
        }

        /* Make sure write will not cross page boundary */
        page_boundary = QSPI_FLASH_PAGE_SIZE - (addr & 0xFF);
        if (size > page_boundary) {
                size = page_boundary;
        }

        qspi_enter_manual_access_mode(id);
        qspi_set_manual_access_bus_mode(id, write_enable_bus_mode, false);
        qspi_flash_write_enable(id);

        qspi_set_manual_access_bus_mode(id, opcode_bus_mode, false);
        hw_qspi_cs_enable(id);
        hw_qspi_write8(id, opcode);

        qspi_set_manual_access_bus_mode(id, addr_bus_mode, false);

        if (addr_size == HW_QSPI_ADDR_SIZE_32) {
                hw_qspi_write32(id, addr);
        } else {
                hw_qspi_write8(id, (uint8_t) ((addr >> 16) & 0xFF));
                hw_qspi_write8(id, (uint8_t) ((addr >> 8) & 0xFF));
                hw_qspi_write8(id, (uint8_t) (addr & 0xFF));
        }

        qspi_set_manual_access_bus_mode(id, data_bus_mode, false);

        if (odd) {
                odd = 4 - odd;
                for (i = 0; i < odd && i < size; ++i) {
                        hw_qspi_write8(id, buf[i]);
                }
        }

        size_aligned32 = ((size - i) & ~0x3);

        if (size_aligned32) {
                fast_write_to_fifo32((uint32_t)(buf + i), (uint32_t)(buf + i + size_aligned32),
                                     (uint32_t) &(QSPIC->QSPIC_WRITEDATA_REG));
                i += size_aligned32;
        }

        for (; i < size; ++i) {
                hw_qspi_write8(id, buf[i]);
        }

        hw_qspi_cs_disable(id);

        DBG_SET_LOW(FLASH_DEBUG, FLASHDBG_PAGE_PROG);

        return i;
}

#if !dgconfigQSPI_ERASE_IN_AUTOMODE
/*
 * Erase a sector of the Flash in manual mode.
 *
 * Before calling this function you need to disable the interrupts and switch to Manual Access Mode
 * calling the qspi_enter_manual_access_mode(id).
 *
 * This function does not block until the Flash has
 * processed the command! When calling this function the QSPI controller remains to manual mode.
 * The function must be called with interrupts disabled.
 */
__RETAINED_CODE static void qspi_flash_erase_sector_manual(HW_QSPIC_ID id, uint32_t addr)
{
        __DBG_QSPI_VOLATILE__ HW_QSPI_BUS_MODE write_enable_bus_mode = QSPI_GET_PARAM(id, write_enable_instr_cfg.opcode_bus_mode);
        __DBG_QSPI_VOLATILE__ HW_QSPI_BUS_MODE opcode_bus_mode = QSPI_GET_PARAM(id, erase_instr_cfg.opcode_bus_mode);
        __DBG_QSPI_VOLATILE__ HW_QSPI_BUS_MODE addr_bus_mode = QSPI_GET_PARAM(id, erase_instr_cfg.addr_bus_mode);
        __DBG_QSPI_VOLATILE__ HW_QSPI_OPCODE_LEN opcode_len = QSPI_GET_PARAM(id, opcode_len);
        __DBG_QSPI_VOLATILE__ HW_QSPI_ADDR_SIZE addr_size = QSPI_GET_PARAM(id, address_size);
        __DBG_QSPI_VOLATILE__ uint8_t opcode = QSPI_GET_PARAM(id, erase_instr_cfg.opcode);

        qspi_enter_manual_access_mode(id);
        qspi_set_manual_access_bus_mode(id, write_enable_bus_mode, false);
        qspi_flash_write_enable(write_enable_bus_mode);

        qspi_set_manual_access_bus_mode(id, opcode_bus_mode, false);

        hw_qspi_cs_enable(id);

        if (USE_DUAL_BYTE_OPCODE(opcode_len, opcode_bus_mode)) {
                hw_qspi_write16(id, CONVERT_OPCODE_TO_DUAL_BYTE(opcode));
        } else {
                hw_qspi_write8(id, opcode);
        }

        qspi_set_manual_access_bus_mode(id, addr_bus_mode, false);

        if (addr_size == HW_QSPI_ADDR_SIZE_32) {
                hw_qspi_write32(id, addr);
        } else {
                hw_qspi_write8(id, (uint8_t) ((addr >> 16) & 0xFF));
                hw_qspi_write8(id, (uint8_t) ((addr >> 8) & 0xFF));
                hw_qspi_write8(id, (uint8_t) (addr & 0xFF));
        }

        hw_qspi_cs_disable(id);
        /*
         * Flash stays in manual mode.
         */

}
#endif /* !dgconfigQSPI_ERASE_IN_AUTOMODE */

#if dg_configUSE_SYS_BACKGROUND_FLASH_OPS
__RETAINED_CODE void qspi_automode_int_resume(HW_QSPIC_ID id)
{
        __DBG_QSPI_VOLATILE__ uint8_t resume_opcode = QSPI_GET_PARAM(id, suspend_resume_instr_cfg.resume_opcode);
        __DBG_QSPI_VOLATILE__ HW_QSPI_BUS_MODE suspended_bus_mode = QSPI_GET_PARAM(id, suspend_resume_instr_cfg.suspend_bus_mode);
        __DBG_QSPI_VOLATILE__ HW_QSPI_BUS_MODE resume_bus_mode = QSPI_GET_PARAM(id, suspend_resume_instr_cfg.resume_bus_mode);
        __DBG_QSPI_VOLATILE__ qspi_is_suspended_cb_t is_suspended = QSPI_GET_PARAM(id, callback.is_suspended_cb);
        __DBG_QSPI_VOLATILE__ uint32_t resume_latency = QSPI_GET_PARAM(id, suspend_resume_instr_cfg.resume_latency_usec);

        DBG_SET_HIGH(FLASH_DEBUG, FLASHDBG_RESUME);

        qspi_enter_manual_access_mode(id);
        qspi_set_manual_access_bus_mode(id, suspended_bus_mode, false);

        if (!is_suspended(suspended_bus_mode)) {
                return;
        }

        do {
                // Send Resume command
                qspi_set_manual_access_bus_mode(id, resume_bus_mode, false);
                qspi_flash_cmd(id, resume_opcode);
                qspi_set_manual_access_bus_mode(id, suspended_bus_mode, false);
        }  while (is_suspended(suspended_bus_mode));

        hw_clk_delay_usec(resume_latency);

        // Flash stays in manual mode.
        DBG_SET_LOW(FLASH_DEBUG, FLASHDBG_RESUME);
}

__RETAINED_CODE void qspi_automode_int_suspend(HW_QSPIC_ID id)
{
        __DBG_QSPI_VOLATILE__ uint8_t opcode = QSPI_GET_PARAM(id, suspend_resume_instr_cfg.suspend_opcode);
        __DBG_QSPI_VOLATILE__ HW_QSPI_BUS_MODE suspend_bus_mode = QSPI_GET_PARAM(id, suspend_resume_instr_cfg.suspend_bus_mode);
        __DBG_QSPI_VOLATILE__ HW_QSPI_BUS_MODE busy_bus_mode = QSPI_GET_PARAM(id, read_status_instr_cfg.opcode_bus_mode);
        __DBG_QSPI_VOLATILE__ uint32_t suspend_latency = QSPI_GET_PARAM(id, suspend_resume_instr_cfg.suspend_latency_usec);

        DBG_SET_HIGH(FLASH_DEBUG, FLASHDBG_SUSPEND_ACTION);

        qspi_enter_manual_access_mode(id);
        qspi_set_manual_access_bus_mode(id, busy_bus_mode, false);

        // Check if an operation is ongoing.
        while (qspi_flash_is_busy(busy_bus_mode)) {
                qspi_set_manual_access_bus_mode(id, suspend_bus_mode, false);
                qspi_flash_cmd(id, opcode);
                qspi_set_manual_access_bus_mode(id, busy_bus_mode, false);

                // Wait for SUS bit to be updated
                hw_clk_delay_usec(suspend_latency);
        }

        // Flash stays in manual mode.
        DBG_SET_LOW(FLASH_DEBUG, FLASHDBG_SUSPEND_ACTION);
}

__RETAINED_CODE bool qspi_automode_int_is_suspended(HW_QSPIC_ID id)
{
        __DBG_QSPI_VOLATILE__ HW_QSPI_BUS_MODE bus_mode = QSPI_GET_PARAM(id, suspend_resume_instr_cfg.suspend_bus_mode);

        return QSPI_GET_PARAM(id, callback.is_suspended_cb)(bus_mode);
}

__RETAINED_CODE bool qspi_automode_int_is_busy(HW_QSPIC_ID id)
{
        __DBG_QSPI_VOLATILE__ HW_QSPI_BUS_MODE bus_mode = QSPI_GET_PARAM(id, read_status_instr_cfg.opcode_bus_mode);

        qspi_enter_manual_access_mode(id);
        qspi_set_manual_access_bus_mode(id, bus_mode, false);

        return qspi_flash_is_busy(bus_mode);
}
#endif /* dg_configUSE_SYS_BACKGROUND_FLASH_OPS */

__RETAINED_CODE bool qspi_automode_is_valid_virtual_address_range(uint32_t addr, uint32_t len)
{
        __DBG_QSPI_VOLATILE__ uint32_t size_bytes = (QSPI_GET_PARAM(id, size_bits) / 8);

        if ((addr >= QSPI_MEM1_VIRTUAL_BASE_ADDR) &&
            (addr + len - 1) < (QSPI_MEM1_VIRTUAL_BASE_ADDR + size_bytes)) {
                return true;
        }

        return false;
}

__RETAINED_CODE bool qspi_automode_is_valid_virtual_addr(uint32_t addr)
{
        __DBG_QSPI_VOLATILE__ uint32_t size_bytes = QSPI_GET_PARAM(id, size_bits) / 8;

        if (WITHIN_RANGE(addr, QSPI_MEM1_VIRTUAL_BASE_ADDR, (QSPI_MEM1_VIRTUAL_BASE_ADDR + size_bytes))) {
                return true;
        }

        return false;
}

uint32_t qspi_automode_get_memory_size(uint32_t addr)
{
        ASSERT_WARNING(addr == QSPI_MEM1_VIRTUAL_BASE_ADDR);

        return (QSPI_GET_PARAM(HW_QSPIC, size_bits) / 8);
}

/**
 * \brief Check if the Flash can accept commands
 *
 * \return bool True if the Flash is not busy else false.
 *
 */
__RETAINED_CODE static bool qspi_flash_is_writable(HW_QSPIC_ID id)
{
        __DBG_QSPI_VOLATILE__ bool writable;

        // Disable the interrupts as long as the QSPIC remains in manual access mode
        GLOBAL_INT_DISABLE();

        qspi_enter_manual_access_mode(id);

        // Check if flash is ready.
        writable = !(qspi_flash_is_busy(id));

        qspi_automode_int_enter_auto_access_mode(id);

        // Re-enable the interrupts since the QSPIC switched back to auto access mode
        GLOBAL_INT_RESTORE();

        return writable;
}

__RETAINED_CODE uint32_t qspi_automode_write_flash_page(uint32_t addr, const uint8_t *buf, uint32_t size)
{
        __DBG_QSPI_VOLATILE__ uint32_t written;
        __DBG_QSPI_VOLATILE__ HW_QSPIC_ID id = get_hw_qspic_id(addr, size);
        __DBG_QSPI_VOLATILE__ HW_QSPI_BUS_MODE busy_bus_mode = QSPI_GET_PARAM(id, read_status_instr_cfg.opcode_bus_mode);

        ASSERT_WARNING(size > 0);

        addr = get_zero_based_addr(addr);

        // Disable the interrupts as long as the QSPIC remains in manual access mode
        GLOBAL_INT_DISABLE();

        // Wait until the flash memory is ready
        qspi_enter_manual_access_mode(id);
        qspi_set_manual_access_bus_mode(id, busy_bus_mode, false);
        while (qspi_flash_is_busy(id));

        written = qspi_automode_int_flash_write_page(id, addr, buf, size);

        /* Wait the write command to be completed */
        qspi_set_manual_access_bus_mode(id, busy_bus_mode, false);
        while (qspi_flash_is_busy(id));

        qspi_automode_int_enter_auto_access_mode(id);

        // Re-enable the interrupts since the QSPIC switched back to auto access mode
        GLOBAL_INT_RESTORE();

        return written;
}

__RETAINED_CODE bool qspi_automode_erase_flash_sector(uint32_t addr)
{
        DBG_SET_HIGH(FLASH_DEBUG, FLASHDBG_SECTOR_ERASE);

        __DBG_QSPI_VOLATILE__ HW_QSPIC_ID id = get_hw_qspic_id(addr, QSPI_FLASH_SECTOR_SIZE);

        addr = get_zero_based_addr(addr);

        while (!qspi_flash_is_writable(id));

#if dgconfigQSPI_ERASE_IN_AUTOMODE
        hw_qspi_erase_block(id, addr);

        while (hw_qspi_get_erase_status(id) != HW_QSPI_ERASE_STATUS_NO);
#else
        __DBG_QSPI_VOLATILE__ HW_QSPI_BUS_MODE busy_bus_mode = QSPI_GET_PARAM(id, read_status_instr_cfg.opcode_bus_mode);

        // Disable the interrupts as long as the QSPIC remains in manual access mode
        GLOBAL_INT_DISABLE();

        qspi_enter_manual_access_mode(id);
        qspi_flash_erase_sector_manual(addr);
        while (qspi_flash_is_busy(busy_bus_mode));
        qspi_automode_int_enter_auto_access_mode(id);

        // Re-enable the interrupts since the QSPIC switched back to auto access mode
        GLOBAL_INT_RESTORE();
#endif
        DBG_SET_LOW(FLASH_DEBUG, FLASHDBG_SECTOR_ERASE);

        return true;
}

void qspi_automode_erase_chip(uint32_t virtual_base_addr)
{
        HW_QSPIC_ID id = get_hw_qspic_id(virtual_base_addr, 0);

        // Disable the interrupts as long as the QSPIC remains in manual access mode
        GLOBAL_INT_DISABLE();

        qspi_enter_manual_access_mode(id);
        qspi_flash_write_enable(id);
        qspi_flash_cmd(id, QSPI_CHIP_ERASE_OPCODE);
        while (qspi_flash_is_busy(id));
        qspi_automode_int_enter_auto_access_mode(id);

        // Re-enable the interrupts since the QSPIC switched back to auto access mode
        GLOBAL_INT_RESTORE();
}

uint32_t qspi_automode_read(uint32_t addr, uint8_t *buf, uint32_t len)
{
        memcpy(buf, qspi_automode_get_physical_addr(addr), len);
        return len;
}

const void *qspi_automode_get_physical_addr(uint32_t virtual_addr)
{
        return (const void *) (MEMORY_QSPIF_S_BASE + get_zero_based_addr(virtual_addr));
}

__RETAINED_CODE static void qspi_flash_init_callback(HW_QSPIC_ID id, HW_QSPI_BUS_MODE bus_mode)
{
        __DBG_QSPI_VOLATILE__ sys_clk_t sys_clk = hw_clk_get_system_clock();

        // Disable the interrupts as long as the QSPIC remains in manual access mode.
        GLOBAL_INT_DISABLE();

        qspi_enter_manual_access_mode(id);
        qspi_set_manual_access_bus_mode(id, bus_mode, true);
        QSPI_GET_PARAM(id, callback.initialize_cb)(id, sys_clk);
        qspi_automode_int_enter_auto_access_mode(id);

        // Re-enable the interrupts, since the QSPIC switched back to auto access mode.
        GLOBAL_INT_RESTORE();
}

__RETAINED_CODE static void qspi_flash_reset_cmd(HW_QSPIC_ID id)
{
        hw_qspi_cs_enable(id);
        hw_qspi_write8(id, QSPI_RESET_EN_OPCODE);
        hw_qspi_cs_disable(id);

        hw_qspi_cs_enable(id);
        hw_qspi_write8(id, QSPI_RESET_OPCODE);

        hw_qspi_cs_disable(id);
}

__RETAINED_CODE static void qspi_flash_release_power_down(HW_QSPIC_ID id, HW_QSPI_BUS_MODE bus_mode)
{
#if dg_configQSPI_FLASH_AUTODETECT
        __DBG_QSPI_VOLATILE__ uint32_t power_down_delay = 20;
#else
        __DBG_QSPI_VOLATILE__ uint32_t power_down_delay = QSPI_GET_PARAM(id, delay.power_down_usec);
#endif
        qspi_set_manual_access_bus_mode(id, bus_mode, false);

        hw_qspi_cs_enable(id);
        hw_qspi_write8(id, QSPI_RELEASE_POWER_DOWN_OPCODE);
        hw_qspi_cs_disable(id);

        hw_clk_delay_usec(power_down_delay);
}

/**
 * Apply all possible QSPI Flash reset sequences to make sure that any type of Flash memory under
 * any possible configuration will be reset successfully
 */
__RETAINED_CODE static void qspi_flash_reset(HW_QSPIC_ID id)
{
        // Disable the interrupts as long as the QSPIC remains in manual access mode
        GLOBAL_INT_DISABLE();
        qspi_enter_manual_access_mode(id);

#if dg_configQSPI_FLASH_AUTODETECT
        hw_qspi_set_access_mode(id, HW_QSPI_ACCESS_MODE_MANUAL);

        // Apply all possible "exit from continuous mode" sequences
        qspi_set_manual_access_bus_mode(id, HW_QSPI_BUS_MODE_QUAD, false);
        qspi_flash_exit_continuous_mode_cmd(id, HW_QSPI_ADDR_SIZE_32);

        qspi_set_manual_access_bus_mode(id, HW_QSPI_BUS_MODE_SINGLE, false);
        qspi_flash_exit_continuous_mode_cmd(id, HW_QSPI_ADDR_SIZE_32);

        // Apply all possible "release from power down" sequences
        qspi_flash_release_power_down(id, HW_QSPI_BUS_MODE_QUAD);
        qspi_flash_release_power_down(id, HW_QSPI_BUS_MODE_SINGLE);

        // Apply all possible reset sequences
        qspi_set_manual_access_bus_mode(id, HW_QSPI_BUS_MODE_QUAD, false);
        qspi_flash_reset_cmd(id);

        qspi_set_manual_access_bus_mode(id, HW_QSPI_BUS_MODE_SINGLE, false);
        qspi_flash_reset_cmd(id);

        hw_clk_delay_usec(dg_configQSPI_FLASH_AUTODETECT_RESET_DELAY);
#else /* dg_configQSPI_FLASH_AUTODETECT */
        __DBG_QSPI_VOLATILE__ HW_QSPI_BUS_MODE opcode_bus_mode = QSPI_GET_PARAM(id, read_instr_cfg.opcode_bus_mode);
        __DBG_QSPI_VOLATILE__ uint32_t reset_delay = QSPI_GET_PARAM(id, delay.reset_usec);

        qspi_flash_release_power_down(id, opcode_bus_mode);

        if (opcode_bus_mode != HW_QSPI_BUS_MODE_SINGLE) {
                qspi_flash_release_power_down(id, HW_QSPI_BUS_MODE_SINGLE);
        }

        qspi_set_manual_access_bus_mode(id, opcode_bus_mode, false);
        qspi_flash_reset_cmd(id);

        if (opcode_bus_mode != HW_QSPI_BUS_MODE_SINGLE) {
                qspi_set_manual_access_bus_mode(id, HW_QSPI_BUS_MODE_SINGLE, false);
                qspi_flash_reset_cmd(id);
        }

        hw_clk_delay_usec(reset_delay);
#endif /* dg_configQSPI_FLASH_AUTODETECT */

        qspi_automode_int_enter_auto_access_mode(id);
        // Re-enable the interrupts since the QSPIC switched back to auto access mode
        GLOBAL_INT_RESTORE();
}

__RETAINED_CODE static bool qspi_match_jedec_id(HW_QSPIC_ID id, jedec_id_t *jedec, const qspi_flash_config_t *flash_cfg)
{
        if ((jedec->manufacturer_id == flash_cfg->jedec.manufacturer_id) &&
            (jedec->type == flash_cfg->jedec.type) &&
            ((jedec->density & flash_cfg->jedec.density_mask) == flash_cfg->jedec.density)) {
                return true;
        }

        return false;
}

#if dg_configQSPI_FLASH_AUTODETECT
/**
 * Searches in qspi_memory_config_table[] whether the input jedec id matches one of the supported memories.
 * If yes, it copies the corresponding qspi_memory_config_table[i] to qspi_flash_config and returns true.
 * Otherwise, it returns false.
 */
__RETAINED_CODE static bool qspi_memory_detect(HW_QSPIC_ID id, jedec_id_t *jedec)
{
        __DBG_QSPI_VOLATILE__ bool jedec_id_matched = false;

        for (uint8_t i = 0; i < ARRAY_LENGTH(qspi_memory_config_table); ++i) {
                jedec_id_matched = qspi_match_jedec_id(id, jedec, qspi_memory_config_table[i]);

                if (jedec_id_matched) {
                        memcpy(&qspi_flash_config, qspi_memory_config_table[i], sizeof(qspi_flash_config_t));
                        break;
                }
        }

        return jedec_id_matched;
}
#endif /* dg_configQSPI_FLASH_AUTODETECT */

__RETAINED_CODE static void read_jedec_id(const HW_QSPIC_ID id, jedec_id_t *jedec)
{
        // Disable the interrupts as long as the QSPIC remains in manual access mode
        GLOBAL_INT_DISABLE();

        // Switch to manual access mode in order to read the jedec id
        qspi_enter_manual_access_mode(id);
        qspi_set_manual_access_bus_mode(id, HW_QSPI_BUS_MODE_SINGLE, false);

        hw_qspi_cs_enable(id);
        hw_qspi_write8(id, QSPI_READ_JEDEC_ID_OPCODE);

        // To prevent getting measurements different than 0 when the memory is absent, it is crucial
        // to wait for adequate time which will allow the internal pull-down resistors to pull the
        // level of the QSPIC IOs at ground. It has been experimentally detected that 100 usec is a
        // safe margin even when the parasitic capacitance of the IOs is extremely high.
        hw_clk_delay_usec(100);

        jedec->manufacturer_id = hw_qspi_read8(id);
        jedec->type = hw_qspi_read8(id);
        jedec->density = hw_qspi_read8(id);

        hw_qspi_cs_disable(id);

        // After reading the JEDEC ID, switch back to auto access mode to detect/match the flash
        // memory. This step is mandatory because the JEDEC ID that was read needs to be compared
        // with data that is stored in the flash memory, such as the qspi_memory_config_table or
        // dg_configQSPI_FLASH_CONFIG. The preliminary QSPIC configuration has already enabled XiP
        // at low performance mode.
        qspi_automode_int_enter_auto_access_mode(id);

        // Re-enable the interrupts since the QSPIC switched back to auto access mode
        GLOBAL_INT_RESTORE();
}

__RETAINED_CODE static QSPI_AUTOMODE_MEMORY_STATUS identify_jedec_id(const HW_QSPIC_ID id, jedec_id_t *jedec)
{
        __DBG_QSPI_VOLATILE__ HW_QSPI_CLK_DIV div = hw_qspi_get_div(id);

         qspi_flash_reset(id);

         // Some memories have a limitation on the maximum frequency at which the read JEDEC ID can
         // be performed. To ensure successful reading of the JEDEC ID, the QSPIC clock divider is
         // switched to 8.
         hw_qspi_set_div(id, HW_QSPI_CLK_DIV_8);
         read_jedec_id(id, jedec);
         // Restore the QSPIC clock divider
         hw_qspi_set_div(id, div);

         // If the JEDEC ID is {0 0 0}, it indicates that the memory is not present, and
         // therefore, the pull-down resistors of the QSPIC IOs have pulled the level at ground.
         if (jedec->manufacturer_id == 0x00 && jedec->type == 0x00 && jedec->density == 0x00) {
                 return QSPI_AUTOMODE_MEMORY_STATUS_ABSENT;
         }

#if dg_configQSPI_FLASH_AUTODETECT
         if (qspi_memory_detect(id, jedec)) {
#else
         if (qspi_match_jedec_id(id, jedec, &dg_configQSPI_FLASH_CONFIG)) {
#endif
                 return QSPI_AUTOMODE_MEMORY_STATUS_PRESENT_IDENTIFIED;
         }

         return QSPI_AUTOMODE_MEMORY_STATUS_PRESENT_UNIDENTIFIED;
}

__RETAINED_CODE QSPI_AUTOMODE_MEMORY_STATUS qspi_automode_read_jedec_id(uint32_t addr, jedec_id_t *jedec)
{
        __DBG_QSPI_VOLATILE__ HW_QSPIC_ID id = get_hw_qspic_id(addr, 0);

        return identify_jedec_id(id, jedec);
}

/**
 * Initialize the QSPI Controller with a preliminary setup which is applicable to all flash memories.
 */
__RETAINED_CODE static void qspi_controller_preliminary_init(HW_QSPIC_ID id)
{
        hw_qspi_config_t qspic_cfg;
        hw_qspi_read_instr_config_t qspic_read_instr_cfg;
        sys_clk_t sys_clk = hw_clk_get_system_clock();
        uint32_t sys_clk_freq = hw_clk_get_sys_clk_freq(sys_clk);

        qspic_cfg.address_size                                 = HW_QSPI_ADDR_SIZE_24;
        qspic_cfg.clk_div                                      = HW_QSPI_CLK_DIV_1;
        qspic_cfg.clock_mode                                   = HW_QSPI_CLK_MODE_LOW;
        qspic_cfg.drive_current                                = dg_configQSPI_DRIVE_CURRENT;
        qspic_cfg.read_pipe                                    = HW_QSPI_READ_PIPE_ENABLE;
        qspic_cfg.read_pipe_delay                              = READ_PIPE_DELAY_0V9;
        qspic_cfg.sampling_edge                                = HW_QSPI_SAMPLING_EDGE_POS;
        qspic_cfg.slew_rate                                    = dg_configQSPI_SLEW_RATE;
        qspic_cfg.hready_mode                                  = HW_QSPI_HREADY_MODE_WAIT;

        qspic_read_instr_cfg.opcode_bus_mode                   = HW_QSPI_BUS_MODE_SINGLE,
        qspic_read_instr_cfg.addr_bus_mode                     = HW_QSPI_BUS_MODE_SINGLE,
        qspic_read_instr_cfg.extra_byte_bus_mode               = HW_QSPI_BUS_MODE_SINGLE,
        qspic_read_instr_cfg.dummy_bus_mode                    = HW_QSPI_BUS_MODE_SINGLE,
        qspic_read_instr_cfg.data_bus_mode                     = HW_QSPI_BUS_MODE_SINGLE,
        qspic_read_instr_cfg.continuous_mode                   = HW_QSPI_CONTINUOUS_MODE_DISABLE,
        qspic_read_instr_cfg.extra_byte_cfg                    = HW_QSPI_EXTRA_BYTE_DISABLE,
        qspic_read_instr_cfg.extra_byte_half_cfg               = HW_QSPI_EXTRA_BYTE_HALF_DISABLE,
        qspic_read_instr_cfg.opcode                            = QSPI_READ3B_OPCODE,
        qspic_read_instr_cfg.extra_byte_value                  = 0xFF,
        qspic_read_instr_cfg.cs_idle_delay_nsec                = 10,

        hw_qspi_init(id, &qspic_cfg);
        hw_qspi_read_instr_init(id, &qspic_read_instr_cfg, 0, sys_clk_freq);
        qspi_set_manual_access_bus_mode(id, HW_QSPI_BUS_MODE_SINGLE, true);
}

/**
 * Initialize the QSPI controller based on the QSPI flash driver.
 */
__RETAINED_CODE static void qspi_controller_init(HW_QSPIC_ID id)
{
        hw_qspi_config_t qspic_cfg;
        sys_clk_t sys_clk = hw_clk_get_system_clock();
        uint32_t sys_clk_freq = hw_clk_get_sys_clk_freq(sys_clk);
        uint8_t dummy_bytes = QSPI_GET_PARAM(id, callback.get_dummy_bytes_cb)(id, sys_clk);

        qspic_cfg.address_size = QSPI_GET_PARAM(id, address_size);
        qspic_cfg.clk_div = HW_QSPI_CLK_DIV_1;
        qspic_cfg.clock_mode = QSPI_GET_PARAM(id, clk_mode);
        qspic_cfg.drive_current = dg_configQSPI_DRIVE_CURRENT;
        qspic_cfg.read_pipe = HW_QSPI_READ_PIPE_ENABLE;
        qspic_cfg.read_pipe_delay = READ_PIPE_DELAY_0V9;
        qspic_cfg.sampling_edge = HW_QSPI_SAMPLING_EDGE_POS;
        qspic_cfg.slew_rate = dg_configQSPI_SLEW_RATE;
        qspic_cfg.hready_mode = HW_QSPI_HREADY_MODE_WAIT;

        hw_qspi_init(id, &qspic_cfg);
        hw_qspi_read_instr_init(id, &QSPI_GET_PARAM(id, read_instr_cfg), dummy_bytes, sys_clk_freq);
        hw_qspi_read_status_instr_init(id, &QSPI_GET_PARAM(id, read_status_instr_cfg), sys_clk_freq);
        hw_qspi_write_enable_instr_init(id, &QSPI_GET_PARAM(id, write_enable_instr_cfg));
#if dgconfigQSPI_ERASE_IN_AUTOMODE
        hw_qspi_erase_instr_init(id, &QSPI_GET_PARAM(id, erase_instr_cfg), sys_clk_freq);
        hw_qspi_exit_continuous_mode_instr_init(id, HW_QSPI_CONTINUOUS_MODE_ENABLE,
                                                QSPI_GET_PARAM(id, address_size));
#endif

#if dg_configUSE_SYS_BACKGROUND_FLASH_OPS
        hw_qspi_suspend_resume_instr_init(id, &QSPI_GET_PARAM(id, suspend_resume_instr_cfg));
#endif
}

#if (dg_configQSPI_FLASH_CONFIG_VERIFY || dg_configQSPI_FLASH_AUTODETECT)
/**
 * Initializes the QSPI controller when either dg_configQSPI_FLASH_AUTODETECT or
 * dg_configQSPI_FLASH_CONFIG_VERIFY is enabled.
 */
__RETAINED_CODE static QSPI_AUTOMODE_MEMORY_STATUS qspi_memory_detect_init(HW_QSPIC_ID id)
{
        QSPI_AUTOMODE_MEMORY_STATUS status;
        jedec_id_t jedec;

        qspi_controller_preliminary_init(id);

        /*
         * If the memory is identified, initialize the QSPIC for high performance based on the
         * flash driver's configuration structure. Otherwise, the connected memory is considered as
         * unknown and the QSPIC configuration remains as applied by qspi_controller_preliminary_init().
         * The latter maintains a low performance reliable functionality in single SPI bus mode,
         * which is applicable to all memories.
         */
        status = identify_jedec_id(id, &jedec);

        if (status == QSPI_AUTOMODE_MEMORY_STATUS_PRESENT_IDENTIFIED) {
                qspi_flash_init_callback(id, HW_QSPI_BUS_MODE_SINGLE);
                qspi_controller_init(id);
        }

        return status;
}

#else /* (dg_configQSPI_FLASH_CONFIG_VERIFY || dg_configQSPI_FLASH_AUTODETECT) */

__RETAINED_CODE static void qspi_memory_no_detect_init(HW_QSPIC_ID id)
{
        /*
         * Since the QSPI memory may be in an undetermined state, the first step is to execute a
         * reset sequence to ensure that the memory is switched to single SPI bus mode. Subsequently,
         * the initialization callback is invoked.
         */
        qspi_controller_preliminary_init(id);
        qspi_flash_reset(id);
        qspi_flash_init_callback(id, HW_QSPI_BUS_MODE_SINGLE);
        qspi_controller_init(id);
}

#endif /* (dg_configQSPI_FLASH_CONFIG_VERIFY) || (dg_configQSPI_FLASH_AUTODETECT) */

__RETAINED_CODE void qspi_automode_flash_power_up(void)
{
        __DBG_QSPI_VOLATILE__ HW_QSPI_BUS_MODE opcode_bus_mode = QSPI_GET_PARAM(id, read_instr_cfg.opcode_bus_mode);

        for (uint8_t idx = 0; idx < QSPIC_INSTANCES; ++idx) {
                HW_QSPIC_ID id = GET_QSPIC_ID(idx);

                // Disable the interrupts as long as the QSPIC remains in manual access mode.
                GLOBAL_INT_DISABLE();

                hw_qspi_clock_enable(id);
                qspi_controller_init(id);

                // The bus mode is not retained during sleep mode and has to be configured when the system
                // wakes up, otherwise the release from power down command will be sent in single SPI bus mode.
                qspi_set_manual_access_bus_mode(id, opcode_bus_mode, true);

#if dg_configQSPI_FLASH_POWER_DOWN
                __DBG_QSPI_VOLATILE__ uint32_t release_pd_delay = QSPI_GET_PARAM(id, delay.release_power_down_usec);

                qspi_enter_manual_access_mode(id);
                qspi_flash_cmd(id, QSPI_RELEASE_POWER_DOWN_OPCODE);
                hw_clk_delay_usec(release_pd_delay);
#endif

                qspi_automode_int_enter_auto_access_mode(id);
                // Re-enable the interrupts, since the QSPIC switched back to auto access mode.
                GLOBAL_INT_RESTORE();
        }
}

__RETAINED_CODE void qspi_automode_flash_power_down(void)
{
        for (uint8_t idx = 0; idx < QSPIC_INSTANCES; ++idx) {
                HW_QSPIC_ID id = GET_QSPIC_ID(idx);

                qspi_enter_manual_access_mode(id);

#if dg_configQSPI_FLASH_POWER_DOWN
                __DBG_QSPI_VOLATILE__ uint32_t power_down_delay = QSPI_GET_PARAM(id, delay.power_down_usec);

                qspi_flash_cmd(id, QSPI_ENTER_POWER_DOWN_OPCODE);
                hw_clk_delay_usec(power_down_delay);
#endif
                // Disable QSPI clock to save power
                hw_qspi_clock_disable(id);
        }
}

__RETAINED_CODE bool qspi_automode_init(void)
{
#if ((dg_configCODE_LOCATION == NON_VOLATILE_IS_QSPI_FLASH) && (dg_configEXEC_MODE == MODE_IS_CACHED) && (dg_configUSE_HW_CACHE == 1))
        ASSERT_WARNING((hw_cache_get_extflash_cacheable_len() != 0) || (hw_cache_is_enabled() == 1));
#endif

        for (uint8_t idx = 0; idx < QSPIC_INSTANCES; ++idx) {
                HW_QSPIC_ID id = GET_QSPIC_ID(idx);

#if (dg_configQSPI_FLASH_CONFIG_VERIFY || dg_configQSPI_FLASH_AUTODETECT)
                // Return false only when the memory is present but unidentified. If it is absent
                // the system doesn't care for proper QSPIC configuration.
                if (qspi_memory_detect_init(id) == QSPI_AUTOMODE_MEMORY_STATUS_PRESENT_UNIDENTIFIED) {
# if QSPI_AUTOMODE_SANITY_CHECK
                        ASSERT_WARNING(0);
# endif
                        return false;
                }
#else
                qspi_memory_no_detect_init(id);
#endif
        }

        return true;
}

__RETAINED_CODE void qspi_automode_sys_clock_cfg(sys_clk_t sys_clk)
{
        __DBG_QSPI_VOLATILE__ uint16_t read_cs_idle_delay;
        __DBG_QSPI_VOLATILE__ uint16_t erase_cs_idle_delay;
        __DBG_QSPI_VOLATILE__ uint32_t qspic_clk_freq = hw_clk_get_sys_clk_freq(sys_clk);

        for (uint8_t idx = 0; idx < QSPIC_INSTANCES; ++idx) {
                HW_QSPIC_ID id = GET_QSPIC_ID(idx);

                read_cs_idle_delay = QSPI_GET_PARAM(id, read_instr_cfg.cs_idle_delay_nsec);
                erase_cs_idle_delay = QSPI_GET_PARAM(id, erase_instr_cfg.cs_idle_delay_nsec);

                qspi_set_read_pipe_clock_delay(id, sys_clk);
                hw_qspi_set_read_cs_idle_delay(id, read_cs_idle_delay, qspic_clk_freq);
                hw_qspi_set_erase_cs_idle_delay(id, erase_cs_idle_delay, qspic_clk_freq);

                GLOBAL_INT_DISABLE();
#if dg_configUSE_SYS_BACKGROUND_FLASH_OPS
                __DBG_QSPI_VOLATILE__ bool resume_before_writing_regs = QSPI_GET_PARAM(id, resume_before_writing_regs);
                /*
                 * When the system clock switches, the XiP flash might be suspended due to an ongoing flash
                 * erase operation. Some flash memories reject commands such as `write status register`,
                 * `write config register` etc while being in erase suspend mode, thus a flash erase resume
                 * command must be issued in advance. Otherwise, the sys_clk_cfg_cb() will fail to update the
                 * corresponding memory settings and the execution will end up to a bus fault, because the
                 * QSPIC won't be able to access the memory.
                 */
                if (resume_before_writing_regs) {
                        qspi_automode_int_resume();
                        qspi_automode_int_enter_auto_access_mode(id);
                }
#endif

                /*
                 * The sys_clk_cfg_cb() might switch the QSPIC to manual access mode, where the flash
                 * memory is not available for XiP, therefore the interrupts are disable during its call.
                 */
                QSPI_GET_PARAM(id, callback.sys_clk_cfg_cb)(id, sys_clk);
                GLOBAL_INT_RESTORE();
        }
}

__RETAINED_CODE bool qspi_automode_is_ram(uint32_t addr)
{
        ASSERT_WARNING(qspi_automode_is_valid_virtual_addr(addr));

        return false;
}

#endif /* dg_configQSPI_AUTOMODE_ENABLE */
