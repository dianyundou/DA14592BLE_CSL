/**
 * \addtogroup PLA_BSP_SYSTEM
 * \{
 * \addtogroup PLA_MEMORY
 * \{
 */

/**
 ****************************************************************************************
 *
 * @file qspi_common_v2.h
 *
 * @brief QSPI flash driver common definitions
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
#ifndef _QSPI_COMMON_V2_H_
#define _QSPI_COMMON_V2_H_


#include <stdbool.h>
#include "hw_qspi.h"

#if __DBG_QSPI_ENABLED
#define __DBG_QSPI_VOLATILE__           volatile
#pragma message "qspi_automode.{h c} debugging mode enabled"
#else
#define __DBG_QSPI_VOLATILE__
#endif

#define QSPI_WRITE_STATUS_REG_OPCODE           (0x01)
#define QSPI_WRITE_DISABLE_OPCODE              (0x04)
#define QSPI_READ_STATUS_REG_OPCODE            (0x05)
#define QSPI_WRITE_ENABLE_OPCODE               (0x06)
#define QSPI_RESET_EN_OPCODE                   (0x66)
#define QSPI_RESET_OPCODE                      (0x99)
#define QSPI_READ3B_OPCODE                     (0x03)
#define QSPI_FAST_READ_QUAD_OPCODE             (0xEB)
#define QSPI_BLOCK_ERASE_OPCODE                (0x52)
#define QSPI_CHIP_ERASE_OPCODE                 (0xC7)
#define QSPI_SECTOR_ERASE_OPCODE               (0x20)
#define QSPI_PAGE_PROGRAM_QPI_OPCODE           (0x02)
#define QSPI_PAGE_PROGRAM_QUAD_OPCODE          (0x32)
#define QSPI_READ_JEDEC_ID_OPCODE              (0x9F)
#define QSPI_EXIT_CONTINUOUS_MODE_BYTE         (0xFF)
#define QSPI_EXIT_CONTINUOUS_MODE_WORD         (0xFFFFFFFF)

#define QSPI_RELEASE_POWER_DOWN_OPCODE         (0xAB)
#define QSPI_ENTER_POWER_DOWN_OPCODE           (0xB9)

#define QSPI_ENTER_QPI_OPCODE                  (0x38)
#define QSPI_EXIT_QPI_OPCODE                   (0xFF)

/* Erase/Write in progress */
#define QSPI_STATUS_REG_BUSY_BIT               (0)
#define QSPI_STATUS_REG_BUSY_MASK              (1 << QSPI_STATUS_REG_BUSY_BIT)

/* WE Latch bit */
#define QSPI_STATUS_REG_WEL_BIT                (1)
#define QSPI_STATUS_REG_WEL_MASK               (1 << QSPI_STATUS_REG_WEL_BIT)

#define QSPI_MEMORY_SIZE_1Mbit                 (1024 * 1024)
#define QSPI_MEMORY_SIZE_2Mbits                (2 * QSPI_MEMORY_SIZE_1Mbit)
#define QSPI_MEMORY_SIZE_4Mbits                (4 * QSPI_MEMORY_SIZE_1Mbit)
#define QSPI_MEMORY_SIZE_8Mbits                (8 * QSPI_MEMORY_SIZE_1Mbit)
#define QSPI_MEMORY_SIZE_16Mbits               (16 * QSPI_MEMORY_SIZE_1Mbit)
#define QSPI_MEMORY_SIZE_32Mbits               (32 * QSPI_MEMORY_SIZE_1Mbit)
#define QSPI_MEMORY_SIZE_64Mbits               (64 * QSPI_MEMORY_SIZE_1Mbit)
#define QSPI_MEMORY_SIZE_128Mbits              (128 * QSPI_MEMORY_SIZE_1Mbit)
#define QSPI_MEMORY_SIZE_256Mbits              (256 * QSPI_MEMORY_SIZE_1Mbit)
#define QSPI_MEMORY_SIZE_512Mbits              (512 * QSPI_MEMORY_SIZE_1Mbit)
#define QSPI_MEMORY_SIZE_1Gbit                 (1024 * QSPI_MEMORY_SIZE_1Mbit)

#define PRODUCT_HEADER_STRUCT(_N_) \
__PACKED_STRUCT { \
        uint32_t burstcmdA; \
        uint32_t burstcmdB; \
        uint16_t flash_config_section; \
        uint16_t flash_config_length; \
        uint8_t config_seq[_N_]; \
        uint16_t crc; \
}

typedef void (* qspi_initialize_cb_t) (HW_QSPIC_ID id, sys_clk_t sys_clk);
typedef void (* qspi_sys_clk_cfg_cb_t) (HW_QSPIC_ID id, sys_clk_t sys_clk);
typedef bool (* qspi_exit_qpi_cb_t) (HW_QSPIC_ID id);
typedef uint8_t (* qspi_get_dummy_bytes_cb_t) (HW_QSPIC_ID id, sys_clk_t sys_clk);
typedef bool (* qspi_is_suspended_cb_t) (HW_QSPIC_ID id);
typedef bool (* qspi_is_busy_cb_t) (HW_QSPIC_ID id, HW_QSPI_BUSY_LEVEL busy_level);
typedef uint8_t (* qspi_read_status_reg_cb_t) (HW_QSPIC_ID id);
typedef void (* qspi_write_status_reg_cb_t) (HW_QSPIC_ID id, uint8_t value);

/**
 * \brief       JEDEC ID struct
 */
typedef struct {
        uint8_t         manufacturer_id;        /**< JEDEC manufacturer ID */
        uint8_t         type;                   /**< JEDEC device type */
        uint8_t         density;                /**< JEDEC device density */
        uint8_t         density_mask;           /**< JEDEC device density mask. Used to mask
                                                     the device density reading, if necessary.
                                                     Otherwise must be set to 0xFF */
} jedec_id_t;

/**
 * \brief       QSPI memory callbacks struct
 */
typedef struct {
        qspi_initialize_cb_t           initialize_cb;          /**< Device initialization callback function */
        qspi_sys_clk_cfg_cb_t          sys_clk_cfg_cb;         /**< Device system clock configuration callback
                                                                    function */
        qspi_exit_qpi_cb_t             exit_qpi_cb;            /**< Callback function that exits the device
                                                                    from QPI mode */
        qspi_get_dummy_bytes_cb_t      get_dummy_bytes_cb;     /**< Callback function that returns the
                                                                    number of dummy bytes */
        qspi_is_suspended_cb_t         is_suspended_cb;        /**< Callback function for checking if the
                                                                    device is in erase/program suspend state */
        qspi_is_busy_cb_t              is_busy_cb;             /**< Callback function for checking if the device is busy */
        qspi_read_status_reg_cb_t      read_status_reg_cb;     /**< Read status register callback function */
        qspi_write_status_reg_cb_t     write_status_reg_cb;    /**< Write status register callback function */
} qspi_callback_t;

/**
 * \brief       QSPI memory delays
 */
typedef struct {
        uint16_t                        reset_usec;             /**< Reset delay (usec) */
        uint16_t                        power_down_usec;        /**< The minimum required delay (usec)
                                                                     to enter power down mode after
                                                                     sending the corresponding command */
        uint16_t                        release_power_down_usec;/**< The minimum required delay (usec)
                                                                     to release from power down mode
                                                                     after sending the corresponding
                                                                     command */
        uint16_t                        power_up_usec;          /**< Power Up delay (usec) */
} qspi_delay_t;

/**
 * \brief       QSPI memory configuration structure
 *
 * This struct is used to define a driver for a specific QSPI memory.
 */
typedef struct {
        jedec_id_t                              jedec;                          /**< JEDEC ID structure */
        uint32_t                                size_bits;                      /**< Memory size (bits) */
        HW_QSPI_ADDR_SIZE                       address_size : 1;               /**< Device address size (24bits or 32-bit) */
        HW_QSPI_CLK_MODE                        clk_mode : 1;                   /**< Clock Mode */
        hw_qspi_read_instr_config_t             read_instr_cfg;                 /**< Read instruction configuration struct */
        hw_qspi_erase_instr_config_t            erase_instr_cfg;                /**< Erase instruction configuration struct */
        hw_qspi_read_status_instr_config_t      read_status_instr_cfg;          /**< Read status register instruction configuration struct */
        hw_qspi_write_enable_instr_config_t     write_enable_instr_cfg;         /**< Write enable instruction configuration struct */
        hw_qspi_page_program_instr_config_t     page_program_instr_cfg;         /**< Page program instruction configuration struct */
        hw_qspi_suspend_resume_instr_config_t   suspend_resume_instr_cfg;       /**< Program and erase suspend/resume
                                                                                     instruction configuration struct */
        qspi_delay_t                            delay;                          /**< QSPI memory delays struct */
        qspi_callback_t                         callback;                       /**< Callbacks struct */
        bool                                    resume_before_writing_regs;     /**< Resume the flash memory before writing the status
                                                                                     register or any other configuration registers.
                                                                                     Some flash memories reject these commands while
                                                                                     being in erase suspend mode, thus a flash erase
                                                                                     resume command must be issued in advance. This setting
                                                                                     is in scope only when the background flash operations
                                                                                     are enabled. Check the manufacturer datasheet and
                                                                                     set this flag accordingly. If the implementation of
                                                                                     the sys_clk_cfg_cb() doesn't make use of the
                                                                                     aforementioned commands this flag must be false */
} qspi_flash_config_t;


#endif /* _QSPI_COMMON_V2_H_ */

/**
 * \}
 * \}
 */
