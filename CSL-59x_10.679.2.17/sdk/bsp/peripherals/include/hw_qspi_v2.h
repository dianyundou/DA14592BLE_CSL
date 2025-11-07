/**
 * \addtogroup PLA_DRI_MEMORY
 * \{
 * \addtogroup HW_QSPI QSPI Flash Memory Controller
 * \{
 * \brief QSPI Flash Memory Controller
 */

/**
 *****************************************************************************************
 *
 * @file hw_qspi_v2.h
 *
 * @brief Definition of API for the QSPI Low Level Driver.
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
 *****************************************************************************************
 */

#ifndef HW_QSPI_V2_H_
#define HW_QSPI_V2_H_


#if dg_configUSE_HW_QSPI

#include <stdbool.h>
#include <stdint.h>
#include <sdk_defs.h>

/*
 * ENUMERATION DEFINITIONS
 *****************************************************************************************
 */

/**
 * \brief QSPIC memory access mode
 */
typedef enum {
        HW_QSPI_ACCESS_MODE_MANUAL = 0,        /**< Manual Mode is selected */
        HW_QSPI_ACCESS_MODE_AUTO = 1           /**< Auto Mode is selected */
} HW_QSPI_ACCESS_MODE;

/**
 * \brief QSPIC memory address size
 */
typedef enum {
        HW_QSPI_ADDR_SIZE_24 = 0,      /**< 24 bits address */
        HW_QSPI_ADDR_SIZE_32 = 1       /**< 32 bits address */
} HW_QSPI_ADDR_SIZE;

/**
 * \brief QSPIC bus mode
 */
typedef enum {
        HW_QSPI_BUS_MODE_SINGLE = 0,   /**< Bus mode in single mode */
        HW_QSPI_BUS_MODE_DUAL = 1,     /**< Bus mode in dual mode */
        HW_QSPI_BUS_MODE_QUAD = 2,     /**< Bus mode in quad mode */
} HW_QSPI_BUS_MODE;

/**
 * \brief QSPIC Bus status
 */
typedef enum {
        HW_QSPI_BUS_STATUS_IDLE = 0,           /**< The SPI Bus is idle */
        HW_QSPI_BUS_STATUS_ACTIVE = 1,         /**< The SPI Bus is active. Read data, write data
                                                     or dummy data activity is in progress.*/
} HW_QSPI_BUS_STATUS;

/**
 * \brief QSPIC device busy status setting
 */
typedef enum {
        HW_QSPI_BUSY_LEVEL_LOW = 0,   /**< The QSPI device is busy when the pin level bit is low */
        HW_QSPI_BUSY_LEVEL_HIGH = 1   /**< The QSPI device is busy when the pin level bit is high */
} HW_QSPI_BUSY_LEVEL;

/**
 * \brief QSPIC clock divider
 */
typedef enum {
        HW_QSPI_CLK_DIV_1 = 0,      /**< divide by 1 */
        HW_QSPI_CLK_DIV_2 = 1,      /**< divide by 2 */
        HW_QSPI_CLK_DIV_4 = 2,      /**< divide by 4 */
        HW_QSPI_CLK_DIV_8 = 3       /**< divide by 8 */
} HW_QSPI_CLK_DIV;

/**
 * \brief QSPIC clock mode
 */
typedef enum {
        HW_QSPI_CLK_MODE_LOW = 0,       /**< Mode 0: QSPI_SCK is low when QSPI_CS is high. */
        HW_QSPI_CLK_MODE_HIGH = 1       /**< Mode 3: QSPI_SCK is high when QSPI_CS is high. */
} HW_QSPI_CLK_MODE;

/**
 * \brief QSPIC continuous mode
 */
typedef enum {
        HW_QSPI_CONTINUOUS_MODE_DISABLE = 0,   /**< Disable continuous mode of operation */
        HW_QSPI_CONTINUOUS_MODE_ENABLE = 1     /**< Enable continuous mode of operation */
} HW_QSPI_CONTINUOUS_MODE;

/**
 * \brief QSPIC pads drive current strength
 *
 */
typedef enum {
        HW_QSPI_DRIVE_CURRENT_4 = 0,       /**< 4 mA */
        HW_QSPI_DRIVE_CURRENT_8 = 1,       /**< 8 mA */
        HW_QSPI_DRIVE_CURRENT_12 = 2,      /**< 12 mA */
        HW_QSPI_DRIVE_CURRENT_16 = 3,      /**< 16 mA */
} HW_QSPI_DRIVE_CURRENT;

/**
 * \brief QSPIC extra byte setting in auto access mode
 */
typedef enum {
        HW_QSPI_EXTRA_BYTE_DISABLE = 0,        /**< Disable extra byte phase */
        HW_QSPI_EXTRA_BYTE_ENABLE = 1,         /**< Enable extra byte phase */
} HW_QSPI_EXTRA_BYTE;

/**
 * \brief QSPIC extra byte half setting in auto access mode
 *
 * \note  This setting is out of scope if the extra byte is disabled.
 */
typedef enum {
        HW_QSPI_EXTRA_BYTE_HALF_DISABLE = 0,   /**< Transmit the complete extra byte */
        HW_QSPI_EXTRA_BYTE_HALF_ENABLE = 1,    /**< The output switches to Hi-Z during the
                                                     transmission of the low nibble of the extra byte */
} HW_QSPI_EXTRA_BYTE_HALF;

/**
 * \brief QSPIC HREADY signal mode when accessing the WRITEDATA, READDATA and DUMMYDATA registers
 *
 * \note This configuration is useful when the frequency of the QSPI clock is much lower than
 *       the clock of the AMBA bus, in order to avoid locking the AMBA bus for a long time.
 *       When is set to HW_QSPI_HREADY_MODE_WAIT there is no need to check the QSPIC_BUSY
 *       for detecting completion of the requested access.
 */
typedef enum {
        HW_QSPI_HREADY_MODE_WAIT = 0,          /**< Adds wait states via hready signal when
                                                    accessing the QSPIC_WRITEDATA, QSPIC_READDATA
                                                    and QSPIC_DUMMYDATA registers. */
        HW_QSPI_HREADY_MODE_NO_WAIT = 1        /**< Don't add wait states via the HREADY signal */
} HW_QSPI_HREADY_MODE;

/**
 * \brief QSPIC pad direction
 *
 * \note Set this enum to HW_QSPI_IO_DIR_OUTPUT only when the SPI or Dual SPI mode is enabled in
 *       order to control the /WP signal. When the Quad SPI bus mode is enabled this setting MUST
 *       be set to HW_QSPI_IO_DIR_AUTO_SEL.
 */
typedef enum {
        HW_QSPI_IO_DIR_AUTO_SEL = 0,   /**< The QSPI pad is determined by the controller. */
        HW_QSPI_IO_DIR_OUTPUT = 1      /**< The QSPI pad is output */
} HW_QSPI_IO_DIR;

/**
 * \brief QSPIC IO2/IO3 pad value
 *
 * \note Use this enum to set the value of QSPI_IOx when the corresponding HW_QSPI_IO_DIR is set
 *       to HW_QSPI_IO_DIR_OUTPUT.
 */
typedef enum {
        HW_QSPI_IO_VALUE_LOW = 0,      /**<  Set the level of the QSPI bus IO low */
        HW_QSPI_IO_VALUE_HIGH = 1,     /**<  Set the level of the QSPI bus IO high */
} HW_QSPI_IO_VALUE;

/**
 * \brief QSPIC read pipe setting
 *
 * \note When read pipe is disabled the sampling clock is determined by @ref HW_QSPI_SAMPLING_EDGE
 *       otherwise by @ref HW_QSPI_READ_PIPE_DELAY.
 */
typedef enum {
        HW_QSPI_READ_PIPE_DISABLE = 0,         /**< Disable read pipe delay */
        HW_QSPI_READ_PIPE_ENABLE = 1,          /**< Enable read pipe delay */
} HW_QSPI_READ_PIPE;

/**
 * \brief QSPIC Read pipe clock delay in relation to the falling edge of QSPI_SCK
 *
 * \note The read pipe delay should be set based on the voltage level of the power rail V12.
 *       Recommended values: V12 = 0.9V: HW_QSPI_READ_PIPE_DELAY_0
 *                           V12 = 1.2V: HW_QSPI_READ_PIPE_DELAY_7
 */
typedef enum {
        HW_QSPI_READ_PIPE_DELAY_0 = 0,         /**< Set read pipe delay to 0 */
        HW_QSPI_READ_PIPE_DELAY_1 = 1,         /**< Set read pipe delay to 1 */
        HW_QSPI_READ_PIPE_DELAY_2 = 2,         /**< Set read pipe delay to 2 */
        HW_QSPI_READ_PIPE_DELAY_3 = 3,         /**< Set read pipe delay to 3 */
        HW_QSPI_READ_PIPE_DELAY_4 = 4,         /**< Set read pipe delay to 4 */
        HW_QSPI_READ_PIPE_DELAY_5 = 5,         /**< Set read pipe delay to 5 */
        HW_QSPI_READ_PIPE_DELAY_6 = 6,         /**< Set read pipe delay to 6 */
        HW_QSPI_READ_PIPE_DELAY_7 = 7,         /**< Set read pipe delay to 7 */
} HW_QSPI_READ_PIPE_DELAY;

/**
 * \brief QSPIC clock edge setting for the sampling of the incoming data when the read pipe is
 *        disabled
 */
typedef enum {
        HW_QSPI_SAMPLING_EDGE_POS = 0,    /**< The incoming data sampling is triggered by the
                                               positive edge of QSPIC clock signal */
        HW_QSPI_SAMPLING_EDGE_NEG = 1     /**< The incoming data sampling is triggered by the
                                               negative edge of QSPIC clock signal */
} HW_QSPI_SAMPLING_EDGE;

/**
 * \brief QSPIC pads slew rate
 *
 */
typedef enum {
        HW_QSPI_SLEW_RATE_0 = 0,       /**< Rise = 1.7 V/ns, Fall = 1.9 V/ns (weak) */
        HW_QSPI_SLEW_RATE_1 = 1,       /**< Rise = 2.0 V/ns, Fall = 2.3 V/ns */
        HW_QSPI_SLEW_RATE_2 = 2,       /**< Rise = 2.3 V/ns, Fall = 2.6 V/ns */
        HW_QSPI_SLEW_RATE_3 = 3        /**< Rise = 2.4 V/ns, Fall = 2.7 V/ns (strong) */
} HW_QSPI_SLEW_RATE;

/**
 * \brief The status of sector/block erasing
 */
typedef enum {
        HW_QSPI_ERASE_STATUS_NO = 0,             /**< no erase                           */
        HW_QSPI_ERASE_STATUS_PENDING = 1,        /**< pending erase request              */
        HW_QSPI_ERASE_STATUS_RUNNING = 2,        /**< erase procedure is running         */
        HW_QSPI_ERASE_STATUS_SUSPENDED = 3,      /**< suspended erase procedure          */
        HW_QSPI_ERASE_STATUS_FINISHING = 4       /**< finishing the erase procedure      */
} HW_QSPI_ERASE_STATUS;
/*
 * MACROS DEFINITIONS
 *****************************************************************************************
 */
#define IS_HW_QSPI_ACCESS_MODE(x)              (((x) == HW_QSPI_ACCESS_MODE_MANUAL) || \
                                                ((x) == HW_QSPI_ACCESS_MODE_AUTO))

#define IS_HW_QSPI_ADDR_SIZE(x)                (((x) == HW_QSPI_ADDR_SIZE_24)   || \
                                                ((x) == HW_QSPI_ADDR_SIZE_32))

#define IS_HW_QSPI_BUSY_LEVEL(x)               (((x) == HW_QSPI_BUSY_LEVEL_LOW)   || \
                                                ((x) == HW_QSPI_BUSY_LEVEL_HIGH))

#define IS_HW_QSPI_BUS_MODE(x)                 (((x) >= HW_QSPI_BUS_MODE_SINGLE) && \
                                                ((x) <= HW_QSPI_BUS_MODE_QUAD))

#define IS_HW_QSPI_CLK_DIV(x)                  (((x) >= HW_QSPI_CLK_DIV_1) && \
                                                ((x) <= HW_QSPI_CLK_DIV_8))

#define IS_HW_QSPI_CLK_MODE(x)                 (((x) == HW_QSPI_CLK_MODE_LOW) || \
                                                ((x) == HW_QSPI_CLK_MODE_HIGH))

#define IS_HW_QSPI_CONTINUOUS_MODE(x)          (((x) == HW_QSPI_CONTINUOUS_MODE_DISABLE) || \
                                                ((x) == HW_QSPI_CONTINUOUS_MODE_ENABLE))

#define IS_HW_QSPI_DRIVE_CURRENT(x)            (((x) >= HW_QSPI_DRIVE_CURRENT_4)  && \
                                                ((x) <= HW_QSPI_DRIVE_CURRENT_16))

#define IS_HW_QSPI_EXTRA_BYTE(x)               (((x) == HW_QSPI_EXTRA_BYTE_DISABLE) || \
                                                ((x) == HW_QSPI_EXTRA_BYTE_ENABLE))

#define IS_HW_QSPI_EXTRA_BYTE_HALF(x)          (((x) == HW_QSPI_EXTRA_BYTE_HALF_DISABLE) || \
                                                ((x) == HW_QSPI_EXTRA_BYTE_HALF_ENABLE))

#define IS_HW_QSPI_HREADY_MODE(x)              (((x) == HW_QSPI_HREADY_MODE_WAIT) || \
                                                ((x) == HW_QSPI_HREADY_MODE_NO_WAIT))

#define IS_HW_QSPI_IO_DIR(x)                   (((x) == HW_QSPI_IO_DIR_AUTO_SEL) || \
                                                ((x) == HW_QSPI_IO_DIR_OUTPUT))

#define IS_HW_QSPI_IO_VALUE(x)                 (((x) == HW_QSPI_IO_VALUE_LOW) || \
                                                ((x) == HW_QSPI_IO_VALUE_HIGH))

#define IS_HW_QSPI_READ_PIPE(x)                (((x) == HW_QSPI_READ_PIPE_DISABLE)   || \
                                                ((x) == HW_QSPI_READ_PIPE_ENABLE))

#define IS_HW_QSPI_READ_PIPE_DELAY(x)          (((x) >= HW_QSPI_READ_PIPE_DELAY_0)  && \
                                                ((x) <= HW_QSPI_READ_PIPE_DELAY_7))

#define IS_HW_QSPI_SAMPLING_EDGE(x)            (((x) == HW_QSPI_SAMPLING_EDGE_POS) || \
                                                ((x) == HW_QSPI_SAMPLING_EDGE_NEG))

#define IS_HW_QSPI_SLEW_RATE(x)                (((x) >= HW_QSPI_SLEW_RATE_0)  && \
                                                ((x) <= HW_QSPI_SLEW_RATE_3))

#define SUSPEND_RESUME_COUNTER_FREQ_HZ          (288000)
/*
 * STRUCTURE DEFINITIONS
 *****************************************************************************************
 */
/**
 * \brief This union is used in order to allow different size access when reading/writing to
 *        QSPIC_READDATA_REG, QSPIC_WRITEDATA_REG, QSPIC_DUMMYDATA_REG because
 */
typedef union {
        __IO uint32_t  data32;
        __IO uint16_t  data16;
        __IO uint8_t   data8;
} hw_qspi_data_t;

/**
 * \brief QSPIC configuration structure
 */
typedef struct {
        HW_QSPI_ADDR_SIZE               address_size : 1;       /**< Memory address size */
        HW_QSPI_CLK_DIV                 clk_div : 2;            /**< Clock divider */
        HW_QSPI_CLK_MODE                clock_mode : 1;         /**< Clock mode */
        HW_QSPI_DRIVE_CURRENT           drive_current : 2;      /**< Drive current */
        HW_QSPI_READ_PIPE               read_pipe : 1;          /**< Read pipe enable */
        HW_QSPI_READ_PIPE_DELAY         read_pipe_delay : 3;    /**< Read pipe delay */
        HW_QSPI_SAMPLING_EDGE           sampling_edge :1;       /**< Incoming data sampling edge */
        HW_QSPI_SLEW_RATE               slew_rate : 2;          /**< IOs slew rate */
        HW_QSPI_HREADY_MODE             hready_mode : 1;        /**< HREADY signal mode */
} hw_qspi_config_t;

/**
 * \brief Read instruction configuration structure (auto access mode)
 */
typedef struct {
        HW_QSPI_BUS_MODE                opcode_bus_mode : 2;    /**< Bus mode of the opcode phase */
        HW_QSPI_BUS_MODE                addr_bus_mode : 2;      /**< Bus mode of the address phase */
        HW_QSPI_BUS_MODE                extra_byte_bus_mode : 2;/**< Bus mode of the extra byte phase */
        HW_QSPI_BUS_MODE                dummy_bus_mode : 2;     /**< Bus mode of the dummy phase */
        HW_QSPI_BUS_MODE                data_bus_mode : 2;      /**< Bus mode of the data phase */
        HW_QSPI_CONTINUOUS_MODE         continuous_mode : 1;    /**< Set continuous mode of operation */
        HW_QSPI_EXTRA_BYTE              extra_byte_cfg : 1;     /**< Enable Extra Byte */
        HW_QSPI_EXTRA_BYTE_HALF         extra_byte_half_cfg : 1;/**< Enable Extra Byte Half */
        uint8_t                         opcode;                 /**< Read command opcode */
        uint8_t                         extra_byte_value;       /**< Extra Byte value */
        uint16_t                        cs_idle_delay_nsec;     /**< The minimum CS idle delay in nsec
                                                                     between two consecutive Read commands */
} hw_qspi_read_instr_config_t;

/**
 * \brief QSPIC Erase instruction configuration structure (auto access mode)
 */
typedef struct {
        HW_QSPI_BUS_MODE        opcode_bus_mode : 2;    /**< Bus mode of the opcode phase */
        HW_QSPI_BUS_MODE        addr_bus_mode : 2;      /**< Bus mode of the address phase */
        uint32_t                hclk_cycles : 4;        /**< The number of AMBA AHB hclk cycles
                                                             (0..15) without memory read requests before
                                                             executing an erase or erase resume command.
                                                             Use this setting to delay one of the
                                                             aforementioned commands otherwise keep it 0. */
        uint8_t                 opcode;                 /**< Erase command opcode */
        uint16_t                cs_idle_delay_nsec;     /**< The minimum CS idle delay in nsec
                                                             between a Write Enable, Erase, Erase
                                                             Suspend or Erase Resume command and
                                                             the next consecutive command */
} hw_qspi_erase_instr_config_t;

/**
 * \brief QSPIC read status instruction configuration structure (auto access mode)
 */
typedef struct {
        HW_QSPI_BUS_MODE                opcode_bus_mode : 2;    /**< The bus mode of the opcode phase */
        HW_QSPI_BUS_MODE                receive_bus_mode : 2;   /**< The bus mode of the receive data phase */
        HW_QSPI_BUSY_LEVEL              busy_level : 1;         /**< Busy bit level */
        uint32_t                        busy_pos : 3;           /**< The position of the Busy bit in
                                                                     the status register (0 - 7) */
        uint8_t                         opcode;                 /**< Read Status command opcode  */
        uint16_t                        delay_nsec;             /**< The minimum delay in nsec between a
                                                                     Read Status command and the previous
                                                                     Erase command. Usually NOT needed
                                                                     thus is set equal to 0. */
} hw_qspi_read_status_instr_config_t;

/**
 * \brief QSPIC write enable instruction configuration structure (auto access mode)
 */
typedef struct {
        HW_QSPI_BUS_MODE        opcode_bus_mode: 2;     /**< Bus mode of the opcode phase */
        uint8_t                 opcode;                 /**< Write Enable command opcode  */
} hw_qspi_write_enable_instr_config_t;

/**
 * \brief QSPIC Page Program instruction configuration structure (manual access mode)
 */
typedef struct {
        HW_QSPI_BUS_MODE        opcode_bus_mode : 2;    /**< The bus mode of the opcode phase */
        HW_QSPI_BUS_MODE        addr_bus_mode : 2;      /**< The bus mode of the address phase */
        HW_QSPI_BUS_MODE        data_bus_mode : 2;      /**< The bus mode of the address phase */
        uint8_t                 opcode;                 /**< Page Program command opcode  */
} hw_qspi_page_program_instr_config_t;

/**
 * \brief QSPIC Erase suspend/resume instruction structure (auto access mode)
 */
typedef struct {
        HW_QSPI_BUS_MODE                suspend_bus_mode : 2;   /**< Bus mode during the erase suspend
                                                                     command phase */
        HW_QSPI_BUS_MODE                resume_bus_mode : 2;    /**< Bus mode during the erase resume
                                                                     command phase */
        uint8_t                         suspend_opcode;         /**< Erase suspend instruction code */
        uint8_t                         resume_opcode;          /**< Erase resume instruction code */
        uint8_t                         suspend_latency_usec;   /**< The minimum required latency (usec)
                                                                     to suspend an erase operation.
                                                                     The next consecutive read command
                                                                     cannot be issued before this time
                                                                     has elapsed. */
        uint8_t                         resume_latency_usec;    /**< The minimum required latency (usec)
                                                                     to resume an erase operation.
                                                                     Once the resume command is issued,
                                                                     the currently suspended erase
                                                                     operation resumes within this time. */
        uint16_t                        res_sus_latency_usec;   /**< The minimum required latency (usec)
                                                                     between an erase resume and the
                                                                     next consequent erase suspend
                                                                     command */
} hw_qspi_suspend_resume_instr_config_t;

/**
 * \brief QSPI Controller ID
 *
 */
typedef void * HW_QSPIC_ID;
#define HW_QSPIC        ((HW_QSPIC_ID) QSPIC_BASE)

/* QSPI Base Address */
#define QSPIBA(id)    ((QSPIC_Type *) id)

/**
 * \brief Get the value of a field of a QSPIC register.
 *
 * \param [in] id QSPI controller id
 * \param [in] reg is the register to access
 * \param [in] field is the register field to write
 *
 * \return the value of the register field
 *
 */
#define HW_QSPIC_REG_GETF(id, reg, field) \
        ((QSPIBA(id)->QSPIC_##reg##_REG & QSPIC_QSPIC_##reg##_REG_##QSPIC_##field##_Msk) >> \
         QSPIC_QSPIC_##reg##_REG_##QSPIC_##field##_Pos)

/**
 * \brief Set the value of a field of a QSPIC register.
 *
 * \param [in] id QSPI controller id
 * \param [in] reg is the register to access
 * \param [in] field is the register field to write
 * \param [in] new_val is the value to write
 *
 */
#define HW_QSPIC_REG_SETF(id, reg, field, new_val) \
        QSPIBA(id)->QSPIC_##reg##_REG = ((QSPIBA(id)->QSPIC_##reg##_REG & \
                                         ~QSPIC_QSPIC_##reg##_REG_##QSPIC_##field##_Msk) | \
                                        (QSPIC_QSPIC_##reg##_REG_##QSPIC_##field##_Msk & \
                                         ((new_val) << QSPIC_QSPIC_##reg##_REG_##QSPIC_##field##_Pos)))

/**
 * \brief Set a bit of a QSPIC register.
 *
 * \param [in] id QSPI controller id
 * \param [in] reg is the register to access
 * \param [in] field is the register bit to set
 *
 */
#define HW_QSPIC_REG_SET_BIT(id, reg, field) \
        QSPIBA(id)->QSPIC_##reg##_REG |= (1 << QSPIC_QSPIC_##reg##_REG_##QSPIC_##field##_Pos)

/**
 * \brief Clear a bit of a QSPIC register.
 *
 * \param [in] id QSPI controller id
 * \param [in] reg is the register to access
 * \param [in] field is the register bit to clear
 *
 */
#define HW_QSPIC_REG_CLR_BIT(id, reg, field) \
        QSPIBA(id)->QSPIC_##reg##_REG &= ~QSPIC_QSPIC_##reg##_REG_##QSPIC_##field##_Msk

/**
 * \brief Enable QSPI controller clock
 *
 * \param [in] id QSPI controller id
 */
__STATIC_FORCEINLINE void hw_qspi_clock_enable(HW_QSPIC_ID id)
{
        GLOBAL_INT_DISABLE();
        REG_SET_BIT(CRG_TOP, CLK_AMBA_REG, QSPI_ENABLE);
        GLOBAL_INT_RESTORE();
}

/**
 * \brief Disable QSPI controller clock
 *
 * \param [in] id QSPI controller id
 */
__STATIC_FORCEINLINE void hw_qspi_clock_disable(HW_QSPIC_ID id)
{
        GLOBAL_INT_DISABLE();
        REG_CLR_BIT(CRG_TOP, CLK_AMBA_REG, QSPI_ENABLE);
        GLOBAL_INT_RESTORE();
}

/**
 * \brief Enable CS on QSPI bus in manual access mode
 *
 * \param [in] id QSPI controller id
 */
__STATIC_FORCEINLINE void hw_qspi_cs_enable(HW_QSPIC_ID id)
{
        QSPIBA(id)->QSPIC_CTRLBUS_REG = REG_MSK(QSPIC, QSPIC_CTRLBUS_REG, QSPIC_EN_CS);
}

/**
 * \brief Disable CS on QSPI bus in manual access mode.
 *
 * \param [in] id QSPI controller id
 */
__STATIC_FORCEINLINE void hw_qspi_cs_disable(HW_QSPIC_ID id)
{
        QSPIBA(id)->QSPIC_CTRLBUS_REG = REG_MSK(QSPIC, QSPIC_CTRLBUS_REG, QSPIC_DIS_CS);
}

/**
 * \brief Get QSPIC Bus status
 *
 * \param [in] id QSPI controller id
 *
 * \sa HW_QSPI_BUS_STATUS
 */
__STATIC_FORCEINLINE HW_QSPI_BUS_STATUS hw_qspi_get_bus_status(HW_QSPIC_ID id)
{
        return (HW_QSPI_BUS_STATUS) HW_QSPIC_REG_GETF(id, STATUS, BUSY);
}

/**
 * \brief Set QSPIC clock divider
 *
 * \param [in] id QSPI controller id
 * \param [in] div QSPIC clock divider
 *
 * \sa HW_QSPI_CLK_DIV
 */
__STATIC_FORCEINLINE void hw_qspi_set_div(HW_QSPIC_ID id, HW_QSPI_CLK_DIV div)
{
        ASSERT_WARNING(IS_HW_QSPI_CLK_DIV(div));

        GLOBAL_INT_DISABLE();
        REG_SETF(CRG_TOP, CLK_AMBA_REG, QSPI_DIV, div);
        GLOBAL_INT_RESTORE();
}

/**
 * \brief Get QSPIC clock divider
 *
 * \param [in] id QSPI controller id
 *
 * \return QSPIC clock divider setting
 *
 * \sa HW_QSPI_CLK_DIV
 */
__STATIC_FORCEINLINE HW_QSPI_CLK_DIV hw_qspi_get_div(HW_QSPIC_ID id)
{
        return (HW_QSPI_CLK_DIV) REG_GETF(CRG_TOP, CLK_AMBA_REG, QSPI_DIV);
}

/**
 * \brief       Set QSPIC bus mode in manual access mode
 *
 * \param [in]  id QSPI controller id
 * \param [in]  bus_mode QSPIC bus mode in manual access mode
 *
 * \sa          HW_QSPI_BUS_MODE
 */
__STATIC_FORCEINLINE void hw_qspi_set_manual_access_bus_mode(HW_QSPIC_ID id, HW_QSPI_BUS_MODE bus_mode)
{
        ASSERT_WARNING(IS_HW_QSPI_BUS_MODE(bus_mode));

        QSPIBA(id)->QSPIC_CTRLBUS_REG = 1 << bus_mode;
}

/**
 * \brief       Set QSPIC access mode
 *
 * \param [in]  id QSPI controller id
 * \param [in]  access_mode QSPIC access mode

 * \sa          HW_QSPI_ACCESS_MODE
 */
__STATIC_FORCEINLINE void hw_qspi_set_access_mode(HW_QSPIC_ID id, HW_QSPI_ACCESS_MODE access_mode)
{
        ASSERT_WARNING(IS_HW_QSPI_ACCESS_MODE(access_mode));
        // During erasing where QSPIC_ERASE_EN = 1, QSPIC_AUTO_MD switches in read only mode
        ASSERT_WARNING(HW_QSPIC_REG_GETF(id, ERASECTRL, ERASE_EN) == 0);

        HW_QSPIC_REG_SETF(id, CTRLMODE, AUTO_MD, access_mode);
}

/**
 * \brief       Get QSPIC access mode
 *
 * \param [in]  id QSPI controller id
 * \return      QSPIC access mode
 *
 * \sa          HW_QSPI_ACCESS_MODE
 */
__STATIC_FORCEINLINE HW_QSPI_ACCESS_MODE hw_qspi_get_access_mode(HW_QSPIC_ID id)
{
        return (HW_QSPI_ACCESS_MODE) HW_QSPIC_REG_GETF(id, CTRLMODE, AUTO_MD);
}

/**
 * \brief       Set QSPIC clock mode
 *
 * \param [in]  id QSPI controller id
 * \param [in]  clk_mode QSPIC clock mode
 *
 * \sa          HW_QSPI_CLK_MODE
 */
__STATIC_FORCEINLINE void hw_qspi_set_clock_mode(HW_QSPIC_ID id, HW_QSPI_CLK_MODE clk_mode)
{
        ASSERT_WARNING(IS_HW_QSPI_CLK_MODE(clk_mode));

        HW_QSPIC_REG_SETF(id, CTRLMODE, CLK_MD, clk_mode);
}

/**
 * \brief       Get QSPIC clock mode
 *
 * \param [in]  id QSPI controller id
 * \return      QSPIC clock mode
 *
 * \sa          HW_QSPI_CLK_MODE
 */
__STATIC_FORCEINLINE HW_QSPI_CLK_MODE hw_qspi_get_clock_mode(HW_QSPIC_ID id)
{
        return (HW_QSPI_CLK_MODE) HW_QSPIC_REG_GETF(id, CTRLMODE, CLK_MD);
}

/**
 * \brief       Set QSPI_IO2 direction
 *
 * \param [in]  id QSPI controller id
 * \param [in]  dir QSPI_IO2 direction
 *
 * \note        Set QSPI_IO2 direction to HW_QSPI_IO_DIR_OUTPUT only in Single or Dual SPI mode
 *              to control the /WP signal. When the Quad SPI is enabled, dir MUST be set to
 *              HW_QSPI_IO_DIR_AUTO_SEL.
 *
 * \sa          HW_QSPI_IO_DIR
 */
__STATIC_FORCEINLINE void hw_qspi_set_io2_direction(HW_QSPIC_ID id, HW_QSPI_IO_DIR dir)
{
        ASSERT_WARNING(IS_HW_QSPI_IO_DIR(dir));

        HW_QSPIC_REG_SETF(id, CTRLMODE, IO2_OEN, dir);
}

/**
 * \brief       Get QSPI_IO2 direction
 *
 * \param [in]  id QSPI controller id
 *
 * \return      QSPI_IO2 direction
 *
 * \sa          HW_QSPI_IO_DIR
 */
__STATIC_FORCEINLINE HW_QSPI_IO_DIR hw_qspi_get_io2_direction(HW_QSPIC_ID id)
{
        return (HW_QSPI_IO_DIR) HW_QSPIC_REG_GETF(id, CTRLMODE, IO2_OEN);
}

/**
 * \brief       Set QSPI_IO3 direction
 *
 * \param [in]  id QSPI controller id
 * \param [in]  dir QSPI_IO3 direction
 *
 * \note        Set QSPI_IO3 direction to HW_QSPI_IO_DIR_OUTPUT only in Single or Dual SPI mode
 *              to control the /WP signal. When the Quad SPI is enabled, dir MUST be set to
 *              HW_QSPI_IO_DIR_AUTO_SEL.
 *
 * \sa          HW_QSPI_IO_DIR
 */
__STATIC_FORCEINLINE void hw_qspi_set_io3_direction(HW_QSPIC_ID id, HW_QSPI_IO_DIR dir)
{
        ASSERT_WARNING(IS_HW_QSPI_IO_DIR(dir));

        HW_QSPIC_REG_SETF(id, CTRLMODE, IO3_OEN, dir);
}

/**
 * \brief       Get QSPI_IO3 direction
 *
 * \param [in]  id QSPI controller id
 *
 * \return      QSPI_IO3 direction
 *
 * \sa          HW_QSPI_IO_DIR
 */
__STATIC_FORCEINLINE HW_QSPI_IO_DIR hw_qspi_get_io3_direction(HW_QSPIC_ID id)
{
        return (HW_QSPI_IO_DIR) HW_QSPIC_REG_GETF(id, CTRLMODE, IO3_OEN);
}

/**
 * \brief       Set the value of QSPI_IO2 pad when QSPI_IO2 direction is output
 *
 * \param [in]  id QSPI controller id
 * \param [in]  value The value of QSPI_IO2 pad
 *
 * \sa          HW_QSPI_IO_VALUE
 */
__STATIC_FORCEINLINE void hw_qspi_set_io2_value(HW_QSPIC_ID id, HW_QSPI_IO_VALUE value)
{
        ASSERT_WARNING(IS_HW_QSPI_IO_VALUE(value));

        HW_QSPIC_REG_SETF(id, CTRLMODE, IO2_DAT, (uint32_t) value);
}

/**
 * \brief       Get the value of QSPI_IO2 pad when QSPI_IO2 direction is output
 *
 * \param [in]  id QSPI controller id
 *
 * \return      The value of QSPI_IO2 pad
 *
 * \sa          HW_QSPI_IO_VALUE
 */
__STATIC_FORCEINLINE HW_QSPI_IO_VALUE hw_qspi_get_io2_value(HW_QSPIC_ID id)
{
        return (HW_QSPI_IO_VALUE) HW_QSPIC_REG_GETF(id, CTRLMODE, IO2_DAT);
}

/**
 * \brief       Set the value of QSPI_IO3 pad when QSPI_IO3 direction is output
 *
 * \param [in]  id QSPI controller id
 *
 * \param [in]  value The value of QSPI_IO3 pad
 *
 * \sa          HW_QSPI_IO_VALUE
 */
__STATIC_FORCEINLINE void hw_qspi_set_io3_value(HW_QSPIC_ID id, HW_QSPI_IO_VALUE value)
{
        ASSERT_WARNING(IS_HW_QSPI_IO_VALUE(value));

        HW_QSPIC_REG_SETF(id, CTRLMODE, IO3_DAT, (uint32_t) value);
}

/**
 * \brief       Get the value of QSPI_IO3 pad when QSPI_IO3 direction is output
 *
 * \param [in]  id QSPI controller id
 *
 * \return      The value of QSPI_IO3 pad
 *
 * \sa          HW_QSPI_IO_VALUE
 */
__STATIC_FORCEINLINE HW_QSPI_IO_VALUE hw_qspi_get_io3_value(HW_QSPIC_ID id)
{
        return (HW_QSPI_IO_VALUE) HW_QSPIC_REG_GETF(id, CTRLMODE, IO3_DAT);
}

/**
 * \brief       Set the direction and the level of QSPIC IOs based on the Bus Mode
 *
 * \param [in]  id QSPI controller id
 * \param [in]  bus_mode The QSPIC Bus Mode
 *
 * \sa          bus_mode
 */
__STATIC_FORCEINLINE void hw_qspi_set_io(HW_QSPIC_ID id, HW_QSPI_BUS_MODE bus_mode)
{
        ASSERT_WARNING(IS_HW_QSPI_BUS_MODE(bus_mode));

        uint32_t ctrlmode_reg = QSPIBA(id)->QSPIC_CTRLMODE_REG;

        if (bus_mode == HW_QSPI_BUS_MODE_SINGLE) {
                REG_SET_FIELD(QSPIC, QSPIC_CTRLMODE_REG, QSPIC_IO2_OEN, ctrlmode_reg, 1);
                REG_SET_FIELD(QSPIC, QSPIC_CTRLMODE_REG, QSPIC_IO2_DAT, ctrlmode_reg, 1);
                REG_SET_FIELD(QSPIC, QSPIC_CTRLMODE_REG, QSPIC_IO3_OEN, ctrlmode_reg, 1);
                REG_SET_FIELD(QSPIC, QSPIC_CTRLMODE_REG, QSPIC_IO3_DAT, ctrlmode_reg, 1);
        } else {
                REG_SET_FIELD(QSPIC, QSPIC_CTRLMODE_REG, QSPIC_IO2_OEN, ctrlmode_reg, 0);
                REG_SET_FIELD(QSPIC, QSPIC_CTRLMODE_REG, QSPIC_IO2_DAT, ctrlmode_reg, 0);
                REG_SET_FIELD(QSPIC, QSPIC_CTRLMODE_REG, QSPIC_IO3_OEN, ctrlmode_reg, 0);
                REG_SET_FIELD(QSPIC, QSPIC_CTRLMODE_REG, QSPIC_IO3_DAT, ctrlmode_reg, 0);
        }

        QSPIBA(id)->QSPIC_CTRLMODE_REG = ctrlmode_reg;
}

/**
 * \brief       Set QSPIC HReady signal mode
 *
 * \param [in]  id QSPI controller id
 * \param [in]  mode HReady signal mode
 *
 * \sa          HW_QSPI_HREADY_MODE
 */
__STATIC_FORCEINLINE void hw_qspi_set_hready_mode(HW_QSPIC_ID id, HW_QSPI_HREADY_MODE mode)
{
        ASSERT_WARNING(IS_HW_QSPI_HREADY_MODE(mode));

        HW_QSPIC_REG_SETF(id, CTRLMODE, HRDY_MD, mode);
}

/**
 * \brief       Get QSPIC HReady signal mode
 *
 * \param [in]  id QSPI controller id
 *
 * \return      HReady signal mode
 *
 * \sa          HW_QSPI_HREADY_MODE
 */
__STATIC_FORCEINLINE HW_QSPI_HREADY_MODE hw_qspi_get_hready_mode(HW_QSPIC_ID id)
{
        return (HW_QSPI_HREADY_MODE) HW_QSPIC_REG_GETF(id, CTRLMODE, HRDY_MD);
}

/**
 * \brief       Set QSPIC read sampling edge
 *
 * \param [in]  id QSPI controller id
 * \param [in]  edge Read sampling edge
 *
 * \sa          HW_QSPI_SAMPLING_EDGE
 */
__STATIC_FORCEINLINE void hw_qspi_set_read_sampling_edge(HW_QSPIC_ID id, HW_QSPI_SAMPLING_EDGE edge)
{
        ASSERT_WARNING(IS_HW_QSPI_SAMPLING_EDGE(edge));

        HW_QSPIC_REG_SETF(id, CTRLMODE, RXD_NEG, edge);
}

/**
 * \brief       Get QSPIC read sampling edge
 *
 * \param [in]  id QSPI controller id
 *
 * \return      Read sampling edge
 *
 * \sa          HW_QSPI_SAMPLING_EDGE
 */
__STATIC_FORCEINLINE HW_QSPI_SAMPLING_EDGE hw_qspi_get_read_sampling_edge(HW_QSPIC_ID id)
{
        return (HW_QSPI_SAMPLING_EDGE) HW_QSPIC_REG_GETF(id, CTRLMODE, RXD_NEG);
}

/**
 * \brief       Set QSPIC data read pipe status
 *
 * \param [in]  id QSPI controller id
 * \param [in]  read_pipe Status of data read pipe
 *
 * \sa          HW_QSPI_READ_PIPE
 */
__STATIC_FORCEINLINE void hw_qspi_set_read_pipe(HW_QSPIC_ID id, HW_QSPI_READ_PIPE read_pipe)
{
        ASSERT_WARNING(IS_HW_QSPI_READ_PIPE(read_pipe));

        HW_QSPIC_REG_SETF(id, CTRLMODE, RPIPE_EN, read_pipe);
}

/**
 * \brief       Get QSPIC read pipe status
 *
 * \param [in]  id QSPI controller id
 *
 * \return      Status of data read pipe
 *
 * \sa          HW_QSPI_READ_PIPE
 */
__STATIC_FORCEINLINE HW_QSPI_READ_PIPE hw_qspi_get_read_pipe(HW_QSPIC_ID id)
{
        return (HW_QSPI_READ_PIPE) HW_QSPIC_REG_GETF(id, CTRLMODE, RPIPE_EN);
}

/**
 * \brief       Set the QSPIC read pipe clock delay
 *
 * \param [in]  id QSPI controller id
 * \param [in]  delay Read pipe clock delay
 *
 * \sa          HW_QSPI_READ_PIPE_DELAY
 */
__STATIC_FORCEINLINE void hw_qspi_set_read_pipe_clock_delay(HW_QSPIC_ID id, HW_QSPI_READ_PIPE_DELAY delay)
{
        ASSERT_WARNING(IS_HW_QSPI_READ_PIPE_DELAY(delay));

        HW_QSPIC_REG_SETF(id, CTRLMODE, PCLK_MD, delay);
}

/**
 * \brief       Get QSPIC read pipe clock delay
 *
 * \param [in]  id QSPI controller id
 *
 * \return      Read pipe clock delay
 *
 * \sa          HW_QSPI_READ_PIPE_DELAY
 */
__STATIC_FORCEINLINE HW_QSPI_READ_PIPE_DELAY hw_qspi_get_read_pipe_clock_delay(HW_QSPIC_ID id)
{
        return (HW_QSPI_READ_PIPE_DELAY) HW_QSPIC_REG_GETF(id, CTRLMODE, PCLK_MD);
}

/**
 * \brief       Set QSPIC address size
 *
 * \param [in]  id QSPI controller id
 *
 * \param [in]  addr_size QSPIC address size
 *
 * \sa          HW_QSPI_ADDR_SIZE
 */
__STATIC_FORCEINLINE void hw_qspi_set_address_size(HW_QSPIC_ID id, HW_QSPI_ADDR_SIZE addr_size)
{
        ASSERT_WARNING(IS_HW_QSPI_ADDR_SIZE(addr_size));

        HW_QSPIC_REG_SETF(id, CTRLMODE, USE_32BA, addr_size);
}

/**
 * \brief       Get QSPIC address size
 *
 * \param [in]  id QSPI controller id
 *
 * \return      QSPIC address size
 *
 * \sa          HW_QSPI_ADDR_SIZE
 */
__STATIC_FORCEINLINE HW_QSPI_ADDR_SIZE hw_qspi_get_address_size(HW_QSPIC_ID id)
{
        return (HW_QSPI_ADDR_SIZE) HW_QSPIC_REG_GETF(id, CTRLMODE, USE_32BA);
}

/**
 * \brief       Set slew rate of QSPIC pads
 *
 * \param [in]  id QSPI controller id
 * \param [in]  slew_rate QSPIC pads slew rate
 *
 * \sa          HW_QSPI_SLEW_RATE
 */
__STATIC_FORCEINLINE void hw_qspi_set_slew_rate(HW_QSPIC_ID id, HW_QSPI_SLEW_RATE slew_rate)
{
        ASSERT_WARNING(IS_HW_QSPI_SLEW_RATE(slew_rate));

        HW_QSPIC_REG_SETF(id, GP, PADS_SLEW , slew_rate);
}

/**
 * \brief       Get slew rate of QSPIC pads
 *
 * \param [in]  id QSPI controller id
 *
 * \return      Slew rate of QSPIC pads
 *
 * \sa          HW_QSPI_SLEW_RATE
 */
__STATIC_FORCEINLINE HW_QSPI_SLEW_RATE hw_qspi_get_slew_rate(HW_QSPIC_ID id)
{
        return (HW_QSPI_SLEW_RATE) HW_QSPIC_REG_GETF(id, GP, PADS_SLEW);
}

/**
 * \brief       Set drive current of QSPIC pads
 *
 * \param [in]  id QSPI controller id
 * \param [in]  drive_current QSPIC pads drive current
 *
 * \sa          HW_QSPI_DRIVE_CURRENT
 */
__STATIC_FORCEINLINE void hw_qspi_set_drive_current(HW_QSPIC_ID id, HW_QSPI_DRIVE_CURRENT drive_current)
{
        ASSERT_WARNING(IS_HW_QSPI_DRIVE_CURRENT(drive_current));

        HW_QSPIC_REG_SETF(id, GP, PADS_DRV, drive_current);
}

/**
 * \brief       Get drive current of QSPIC pads
 *
 * \param [in]  id QSPI controller id
 *
 * \return      Drive current of QSPIC pads
 *
 * \sa          HW_QSPI_DRIVE_CURRENT
 */
__STATIC_FORCEINLINE HW_QSPI_DRIVE_CURRENT hw_qspi_get_drive_current(HW_QSPIC_ID id)
{
        return (HW_QSPI_DRIVE_CURRENT) HW_QSPIC_REG_GETF(id, GP, PADS_DRV);
}

/**
 * \brief       Set the number of dummy bytes in auto access mode
 *
 * \param [in]  id QSPI controller id
 * \param [in]  dummy_bytes Number of dummy bytes (0 - 4)
 */
__STATIC_FORCEINLINE void hw_qspi_set_dummy_bytes(HW_QSPIC_ID id, uint8_t dummy_bytes)
{
        ASSERT_WARNING(dummy_bytes < 5);

        if (dummy_bytes == 3) {
                HW_QSPIC_REG_SET_BIT(id, BURSTCMDB, DMY_FORCE);
        } else {
                HW_QSPIC_REG_CLR_BIT(id, BURSTCMDB, DMY_FORCE);
                HW_QSPIC_REG_SETF(id, BURSTCMDB, DMY_NUM, ((dummy_bytes == 4) ? 3 : dummy_bytes));
        }
}

/**
 * \brief       Get the number of dummy bytes in auto access mode
 *
 * \param [in]  id QSPI controller id
 *
 * \return      Number of dummy bytes (0 - 4)
 */
__STATIC_FORCEINLINE uint8_t hw_qspi_get_dummy_bytes(HW_QSPIC_ID id)
{
        uint8_t dummy_bytes;

        if (HW_QSPIC_REG_GETF(id, BURSTCMDB, DMY_FORCE)) {
                return 3;
        }

        dummy_bytes = HW_QSPIC_REG_GETF(id, BURSTCMDB, DMY_NUM);

        return (dummy_bytes == 3) ? 4 : dummy_bytes;
}

/**
 * \brief Set the minimum number of clocks cycles that CS stays in idle mode, between two
 *        consecutive read commands
 *
 * \param [in] id                 QSPI controller id
 * \param [in] cs_idle_delay_nsec The minimum time in nsec that the CS signal stays idle
 * \param [in] clk_freq_hz        The QSPI controller clock frequency (in Hz)
 *
 */
__STATIC_FORCEINLINE void hw_qspi_set_read_cs_idle_delay(HW_QSPIC_ID id, uint16_t cs_idle_delay_nsec,
                                                         uint32_t clk_freq_hz)
{
        uint32_t cs_idle_delay_clk;

        cs_idle_delay_clk = NSEC_TO_CLK_CYCLES(cs_idle_delay_nsec, clk_freq_hz);

        ASSERT_WARNING(cs_idle_delay_clk < 8);
        HW_QSPIC_REG_SETF(id, BURSTCMDB, CS_HIGH_MIN, cs_idle_delay_clk);
}

/**
 * \brief Set the minimum number of clocks cycles that CS stays in idle mode, between a write enable,
 *        erase, erase suspend and erase resume instruction and the next consecutive command.
 *
 * \param [in]  id                QSPI controller id
 * \param [in] cs_idle_delay_nsec The minimum time in nsec that the CS signal stays idle
 * \param [in] clk_freq_hz        The QSPI controller clock frequency (in Hz)
 *
 */
__STATIC_FORCEINLINE void hw_qspi_set_erase_cs_idle_delay(HW_QSPIC_ID id, uint16_t cs_idle_delay_nsec,
                                                          uint32_t clk_freq_hz)
{
        uint32_t cs_idle_delay_clk;

        cs_idle_delay_clk = NSEC_TO_CLK_CYCLES(cs_idle_delay_nsec, clk_freq_hz);

        ASSERT_WARNING(cs_idle_delay_clk < 32);
        HW_QSPIC_REG_SETF(id, ERASECMDB, ERS_CS_HI, cs_idle_delay_clk);
}

/**
 * \brief       Generate 32 bits data transfer from the external device to the QSPIC (manual mode)
 *
 * \param [in]  id QSPI controller id
 *
 * \return      32 bits value read from the bus
 */
__STATIC_FORCEINLINE uint32_t hw_qspi_read32(HW_QSPIC_ID id)
{
        hw_qspi_data_t *tmp = (hw_qspi_data_t *) &(QSPIBA(id)->QSPIC_READDATA_REG);

        return tmp->data32;
}

/**
 * \brief       Generate 16 bits data transfer from the external device to the QSPIC (manual mode)
 *
 * \param [in]  id QSPI controller id
 *
 * \return      16 bits value read from the bus
 */
__STATIC_FORCEINLINE uint16_t hw_qspi_read16(HW_QSPIC_ID id)
{
        hw_qspi_data_t *tmp = (hw_qspi_data_t *) &(QSPIBA(id)->QSPIC_READDATA_REG);

        return tmp->data16;
}

/**
 * \brief       Generate 8 bits data transfer from the external device to the QSPIC (manual mode)
 *
 * \param [in]  id QSPI controller id
 *
 * \return      8 bits value read from the bus
 */
__STATIC_FORCEINLINE uint8_t hw_qspi_read8(HW_QSPIC_ID id)
{
        hw_qspi_data_t *tmp = (hw_qspi_data_t *) &(QSPIBA(id)->QSPIC_READDATA_REG);

        return tmp->data8;
}

/**
 * \brief       Generate 32 bits data transfer from the QSPIC to the external device (manual mode)
 *
 * \param [in]  id QSPI controller id
 * \param [in]  data 32 bits value to be written on the device
 *
 */
__STATIC_FORCEINLINE void hw_qspi_write32(HW_QSPIC_ID id, uint32_t data)
{
        hw_qspi_data_t *tmp = (hw_qspi_data_t *) &(QSPIBA(id)->QSPIC_WRITEDATA_REG);

        tmp->data32 = SWAP32(data);
}

/**
 * \brief       Generate 16 bits data transfer from the QSPIC to the external device (manual mode)
 *
 * \param [in]  id QSPI controller id
 * \param [in]  data 16 bits value to be written on the device
 */
__STATIC_FORCEINLINE void hw_qspi_write16(HW_QSPIC_ID id, uint16_t data)
{
        hw_qspi_data_t *tmp = (hw_qspi_data_t *) &(QSPIBA(id)->QSPIC_WRITEDATA_REG);

        tmp->data16 = SWAP16(data);
}

/**
 * \brief       Generate 8 bits data transfer from the QSPIC to the external device (manual mode)
 *
 * \param [in]  id QSPI controller id
 * \param [in]  data 8 bits value to be written on the device
 */
__STATIC_FORCEINLINE void hw_qspi_write8(HW_QSPIC_ID id, uint8_t data)
{
        hw_qspi_data_t *tmp = (hw_qspi_data_t *) &(QSPIBA(id)->QSPIC_WRITEDATA_REG);

        tmp->data8 = data;
}

/**
 * \brief       Generate clock pulses on the SPI bus for a 32-bit transfer
 *
 * \param [in]  id QSPI controller id
 *
 * \note        During the last clock of this activity in the SPI bus, the QSPI_IOx data pads are
 *              in hi-z state. The number of generated pulses is equal to: (size of AHB bus access)
 *              / (size of SPI bus). The size of SPI bus can be 1, 2 or 4 for Single, Dual, Quad SPI
 *              bus mode respectively.
 */
__STATIC_FORCEINLINE void hw_qspi_dummy32(HW_QSPIC_ID id)
{
        hw_qspi_data_t *tmp = (hw_qspi_data_t *) &(QSPIBA(id)->QSPIC_DUMMYDATA_REG);

        tmp->data32 = 0;
}

/**
 * \brief       Generate clock pulses on the SPI bus for a 16-bit transfer
 *
 * \param [in]  id QSPI controller id
 *
 * \note        During the last clock of this activity in the SPI bus, the QSPI_IOx data pads are
 *              in hi-z state. The number of generated pulses is equal to: (size of AHB bus access)
 *              / (size of SPI bus). The size of SPI bus can be 1, 2 or 4 for Single, Dual, Quad SPI
 *              mode respectively.
 */
__STATIC_FORCEINLINE void hw_qspi_dummy16(HW_QSPIC_ID id)
{
        hw_qspi_data_t *tmp = (hw_qspi_data_t *) &(QSPIBA(id)->QSPIC_DUMMYDATA_REG);

        tmp->data16 = 0;
}

/**
 * \brief       Generate clock pulses on the SPI bus for an 8-bit transfer
 *
 * \param [in]  id QSPI controller id
 *
 * \note        During the last clock of this activity in the SPI bus, the QSPI_IOx data pads are
 *              in hi-z state. The number of generated pulses is equal to: (size of AHB bus access)
 *              / (size of SPI bus). The size of SPI bus can be 1, 2 or 4 for Single, Dual, Quad SPI
 *              mode respectively.
 */
__STATIC_FORCEINLINE void hw_qspi_dummy8(HW_QSPIC_ID id)
{
        hw_qspi_data_t *tmp = (hw_qspi_data_t *) &(QSPIBA(id)->QSPIC_DUMMYDATA_REG);

        tmp->data8 = 0;
}

/**
 * \brief       Initialize the QSPI controller (QSPIC)
 *
 * \param [in]  id      QSPI controller id
 * \param [in]  cfg     Pointer to QSPIC configuration structure.
 *
 * \sa          hw_qspi_config_t
 */
__RETAINED_CODE void hw_qspi_init(HW_QSPIC_ID id, const hw_qspi_config_t *cfg);

/**
 * \brief       Initialize the read instruction of the QSPIC
 *
 * \param [in]  id              QSPI controller id
 * \param [in]  cfg             Pointer to configuration structure of the read instruction.
 * \param [in]  dummy_bytes     The number of dummy bytes.
 * \param [in]  sys_clk_freq_hz The system clock frequency in Hz, which is used to calculate the
 *                              minimum QSPI bus clock cycles that the Chip Select (CS) signal must
 *                              remain high between two consecutive read instructions.
 *
 * \sa          hw_qspi_read_instr_config_t
 */
__STATIC_FORCEINLINE void hw_qspi_read_instr_init(HW_QSPIC_ID id, const hw_qspi_read_instr_config_t *cfg,
                                                  uint8_t dummy_bytes, uint32_t sys_clk_freq_hz)
{
        uint32_t qspi_clk_freq_hz = sys_clk_freq_hz >> (uint32_t) hw_qspi_get_div(id);
        uint32_t delay_clk_cycles = NSEC_TO_CLK_CYCLES(cfg->cs_idle_delay_nsec, qspi_clk_freq_hz);
        uint32_t burstcmda_reg = QSPIBA(id)->QSPIC_BURSTCMDA_REG;
        uint32_t burstcmdb_reg = QSPIBA(id)->QSPIC_BURSTCMDB_REG;

        ASSERT_WARNING(IS_HW_QSPI_BUS_MODE(cfg->opcode_bus_mode));
        ASSERT_WARNING(IS_HW_QSPI_BUS_MODE(cfg->addr_bus_mode));
        ASSERT_WARNING(IS_HW_QSPI_BUS_MODE(cfg->extra_byte_bus_mode));
        ASSERT_WARNING(IS_HW_QSPI_BUS_MODE(cfg->dummy_bus_mode));
        ASSERT_WARNING(IS_HW_QSPI_BUS_MODE(cfg->data_bus_mode));
        ASSERT_WARNING(IS_HW_QSPI_EXTRA_BYTE(cfg->extra_byte_cfg));
        ASSERT_WARNING(IS_HW_QSPI_EXTRA_BYTE_HALF(cfg->extra_byte_half_cfg));
        ASSERT_WARNING(IS_HW_QSPI_CONTINUOUS_MODE(cfg->continuous_mode));
        ASSERT_WARNING(dummy_bytes < 5);
        ASSERT_WARNING(delay_clk_cycles < 8);

        REG_SET_FIELD(QSPIC, QSPIC_BURSTCMDA_REG, QSPIC_INST, burstcmda_reg, cfg->opcode);
        REG_SET_FIELD(QSPIC, QSPIC_BURSTCMDA_REG, QSPIC_EXT_BYTE, burstcmda_reg, cfg->extra_byte_value);
        REG_SET_FIELD(QSPIC, QSPIC_BURSTCMDA_REG, QSPIC_INST_TX_MD, burstcmda_reg, cfg->opcode_bus_mode);
        REG_SET_FIELD(QSPIC, QSPIC_BURSTCMDA_REG, QSPIC_ADR_TX_MD, burstcmda_reg, cfg->addr_bus_mode);
        REG_SET_FIELD(QSPIC, QSPIC_BURSTCMDA_REG, QSPIC_EXT_TX_MD, burstcmda_reg, cfg->extra_byte_bus_mode);
        REG_SET_FIELD(QSPIC, QSPIC_BURSTCMDA_REG, QSPIC_DMY_TX_MD, burstcmda_reg, cfg->dummy_bus_mode);

        REG_SET_FIELD(QSPIC, QSPIC_BURSTCMDB_REG, QSPIC_DAT_RX_MD, burstcmdb_reg, cfg->data_bus_mode);
        REG_SET_FIELD(QSPIC, QSPIC_BURSTCMDB_REG, QSPIC_EXT_BYTE_EN, burstcmdb_reg, cfg->extra_byte_cfg);
        REG_SET_FIELD(QSPIC, QSPIC_BURSTCMDB_REG, QSPIC_EXT_HF_DS, burstcmdb_reg, cfg->extra_byte_half_cfg);
        REG_SET_FIELD(QSPIC, QSPIC_BURSTCMDB_REG, QSPIC_INST_MD, burstcmdb_reg, cfg->continuous_mode);

        if (dummy_bytes == 3) {
                REG_SET_FIELD(QSPIC, QSPIC_BURSTCMDB_REG, QSPIC_DMY_FORCE, burstcmdb_reg, 1);
                REG_SET_FIELD(QSPIC, QSPIC_BURSTCMDB_REG, QSPIC_DMY_NUM, burstcmdb_reg, 0);
        } else {
                REG_SET_FIELD(QSPIC, QSPIC_BURSTCMDB_REG, QSPIC_DMY_FORCE, burstcmdb_reg, 0);
                REG_SET_FIELD(QSPIC, QSPIC_BURSTCMDB_REG, QSPIC_DMY_NUM, burstcmdb_reg,
                              ((dummy_bytes == 4) ? 3 : dummy_bytes));
        }

        REG_SET_FIELD(QSPIC, QSPIC_BURSTCMDB_REG, QSPIC_INST_MD, burstcmdb_reg, cfg->continuous_mode);
        REG_SET_FIELD(QSPIC, QSPIC_BURSTCMDB_REG, QSPIC_CS_HIGH_MIN, burstcmdb_reg, delay_clk_cycles);

        QSPIBA(id)->QSPIC_BURSTCMDA_REG = burstcmda_reg;
        QSPIBA(id)->QSPIC_BURSTCMDB_REG = burstcmdb_reg;
}

/**
 * \brief       Initialize the erase instruction of the QSPIC
 *
 * \param [in]  id              QSPI controller id
 * \param [in]  cfg             Pointer to configuration structure of the erase instruction.
 * \param [in]  sys_clk_freq_hz The system clock frequency in Hz, which is used to calculate the
 *                              minimum QSPI bus clock cycles that the Chip Select (CS) signal
 *                              remain must high between an erase instruction and the next
 *                              consecutive instruction.
 *
 * \sa          hw_qspi_erase_instr_config_t
 */
__STATIC_FORCEINLINE void hw_qspi_erase_instr_init(HW_QSPIC_ID id, const hw_qspi_erase_instr_config_t *cfg,
                                                   uint32_t sys_clk_freq_hz)
{
        uint32_t qspi_clk_freq_hz = sys_clk_freq_hz >> (uint32_t) hw_qspi_get_div(id);
        uint32_t delay_clk_cycles = NSEC_TO_CLK_CYCLES(cfg->cs_idle_delay_nsec, qspi_clk_freq_hz);
        uint32_t erasecmdb_reg = QSPIBA(id)->QSPIC_ERASECMDB_REG;

        ASSERT_WARNING(IS_HW_QSPI_BUS_MODE(cfg->opcode_bus_mode));
        ASSERT_WARNING(IS_HW_QSPI_BUS_MODE(cfg->addr_bus_mode));
        ASSERT_WARNING(cfg->hclk_cycles < 16);
        ASSERT_WARNING(delay_clk_cycles < 32);

        HW_QSPIC_REG_SETF(id, ERASECMDA, ERS_INST, cfg->opcode);

        REG_SET_FIELD(QSPIC, QSPIC_ERASECMDB_REG, QSPIC_ERS_TX_MD, erasecmdb_reg, cfg->opcode_bus_mode);
        REG_SET_FIELD(QSPIC, QSPIC_ERASECMDB_REG, QSPIC_EAD_TX_MD, erasecmdb_reg, cfg->addr_bus_mode);
        REG_SET_FIELD(QSPIC, QSPIC_ERASECMDB_REG, QSPIC_ERSRES_HLD, erasecmdb_reg, cfg->hclk_cycles);
        REG_SET_FIELD(QSPIC, QSPIC_ERASECMDB_REG, QSPIC_ERS_CS_HI, erasecmdb_reg, delay_clk_cycles);

        QSPIBA(id)->QSPIC_ERASECMDB_REG = erasecmdb_reg;
}

/**
 * \brief       Initialize the read status register instruction of the QSPIC
 *
 * \param [in]  id              QSPI controller id
 * \param [in]  cfg             Pointer to configuration structure of the read status register
 *                              instruction.
 * \param [in]  sys_clk_freq_hz The system clock frequency in Hz, which is used to calculate the
 *                              minimum required delay, in QSPI bus clock cycles, between an
 *                              erase or erase resume instruction and the next consecutive
 *                              read status register instruction.
 *
 * \sa          hw_qspi_read_status_instr_config_t
 */
__STATIC_FORCEINLINE void hw_qspi_read_status_instr_init(HW_QSPIC_ID id, const hw_qspi_read_status_instr_config_t *cfg,
                                                         uint32_t sys_clk_freq_hz)
{
        uint32_t qspi_clk_freq_hz = sys_clk_freq_hz >> (uint32_t) hw_qspi_get_div(id);
        uint32_t delay_clk_cycles = NSEC_TO_CLK_CYCLES(cfg->delay_nsec, qspi_clk_freq_hz);
        uint32_t statuscmd_reg = QSPIBA(id)->QSPIC_STATUSCMD_REG;

        ASSERT_WARNING(IS_HW_QSPI_BUS_MODE(cfg->opcode_bus_mode));
        ASSERT_WARNING(IS_HW_QSPI_BUS_MODE(cfg->receive_bus_mode));
        ASSERT_WARNING(IS_HW_QSPI_BUSY_LEVEL(cfg->busy_level));
        ASSERT_WARNING(cfg->busy_pos < 8);
        ASSERT_WARNING(delay_clk_cycles < 64);

        REG_SET_FIELD(QSPIC, QSPIC_STATUSCMD_REG, QSPIC_RSTAT_INST, statuscmd_reg, cfg->opcode);
        REG_SET_FIELD(QSPIC, QSPIC_STATUSCMD_REG, QSPIC_RSTAT_TX_MD, statuscmd_reg, cfg->opcode_bus_mode);
        REG_SET_FIELD(QSPIC, QSPIC_STATUSCMD_REG, QSPIC_RSTAT_RX_MD, statuscmd_reg, cfg->receive_bus_mode);
        REG_SET_FIELD(QSPIC, QSPIC_STATUSCMD_REG, QSPIC_BUSY_POS, statuscmd_reg, cfg->busy_pos);
        REG_SET_FIELD(QSPIC, QSPIC_STATUSCMD_REG, QSPIC_BUSY_VAL, statuscmd_reg, cfg->busy_level);
        REG_SET_FIELD(QSPIC, QSPIC_STATUSCMD_REG, QSPIC_RESSTS_DLY, statuscmd_reg, delay_clk_cycles);
        REG_SET_FIELD(QSPIC, QSPIC_STATUSCMD_REG, QSPIC_STSDLY_SEL, statuscmd_reg, 0);

        QSPIBA(id)->QSPIC_STATUSCMD_REG = statuscmd_reg;
}

/**
 * \brief       Initialize the write enable instruction of the QSPIC
 *
 * \param [in]  id      QSPI controller id
 * \param [in]  cfg     Pointer to configuration structure of the write enable instruction.
 *
 * \sa          hw_qspi_write_enable_instr_config_t
 */
__STATIC_FORCEINLINE void hw_qspi_write_enable_instr_init(HW_QSPIC_ID id, const hw_qspi_write_enable_instr_config_t *cfg)
{
        ASSERT_WARNING(IS_HW_QSPI_BUS_MODE(cfg->opcode_bus_mode));

        HW_QSPIC_REG_SETF(id, ERASECMDA, WEN_INST, cfg->opcode);
        HW_QSPIC_REG_SETF(id, ERASECMDB, WEN_TX_MD, cfg->opcode_bus_mode);
}

/**
 * \brief       Initialize the program and erase suspend/resume instruction of the QSPIC
 *
 * \param [in]  id      QSPI controller id
 * \param [in]  cfg     Pointer to configuration structure of the program and erase suspend/resume
 *                      instruction.
 *
 * \sa          hw_qspi_suspend_resume_instr_config_t
 */
__STATIC_FORCEINLINE void hw_qspi_suspend_resume_instr_init(HW_QSPIC_ID id, const hw_qspi_suspend_resume_instr_config_t *cfg)
{
        uint32_t res_sus_latency_clk_cycles = NSEC_TO_CLK_CYCLES((1000 * cfg->res_sus_latency_usec), SUSPEND_RESUME_COUNTER_FREQ_HZ);

        uint32_t erasecmda_reg = QSPIBA(id)->QSPIC_ERASECMDA_REG;
        uint32_t erasecmdb_reg = QSPIBA(id)->QSPIC_ERASECMDB_REG;

        ASSERT_WARNING(IS_HW_QSPI_BUS_MODE(cfg->suspend_bus_mode));
        ASSERT_WARNING(IS_HW_QSPI_BUS_MODE(cfg->resume_bus_mode));
        ASSERT_WARNING(res_sus_latency_clk_cycles < 64);

        REG_SET_FIELD(QSPIC, QSPIC_ERASECMDA_REG, QSPIC_SUS_INST, erasecmda_reg, cfg->suspend_opcode);

        REG_SET_FIELD(QSPIC, QSPIC_ERASECMDB_REG, QSPIC_SUS_TX_MD, erasecmdb_reg, cfg->suspend_bus_mode);
        REG_SET_FIELD(QSPIC, QSPIC_ERASECMDB_REG, QSPIC_RES_TX_MD, erasecmdb_reg, cfg->resume_bus_mode);
        REG_SET_FIELD(QSPIC, QSPIC_ERASECMDB_REG, QSPIC_RESSUS_DLY, erasecmdb_reg, res_sus_latency_clk_cycles);

        QSPIBA(id)->QSPIC_ERASECMDA_REG = erasecmda_reg;
        QSPIBA(id)->QSPIC_ERASECMDB_REG = erasecmdb_reg;
}

/**
 * \brief       Initialize the exit from continuous mode instruction of the QSPIC
 *
 * \param [in]  id        QSPI controller id
 * \param [in]  mode      Enable/Disable continuous mode of operation.
 * \param [in]  addr_size The address size which determines the length of the sequence to exit the
 *                        connected memory from continuous mode of operation. If the address size is
 *                        32 bits, the length of the command sequence is 2 bytes. Otherwise, for
 *                        address size 24 bits only 1 byte is required.
 *
 * \sa          HW_QSPI_CONTINUOUS_MODE
 * \sa          HW_QSPI_ADDR_SIZE
 */
__STATIC_FORCEINLINE void hw_qspi_exit_continuous_mode_instr_init(HW_QSPIC_ID id, HW_QSPI_CONTINUOUS_MODE mode,
                                                                  HW_QSPI_ADDR_SIZE addr_size)
{
        uint32_t burstbrk_reg = QSPIBA(id)->QSPIC_BURSTBRK_REG;

        REG_SET_FIELD(QSPIC, QSPIC_BURSTBRK_REG, QSPIC_BRK_WRD, burstbrk_reg, 0xFFFF);
        REG_SET_FIELD(QSPIC, QSPIC_BURSTBRK_REG, QSPIC_BRK_TX_MD, burstbrk_reg, HW_QSPI_BUS_MODE_SINGLE);
        REG_SET_FIELD(QSPIC, QSPIC_BURSTBRK_REG, QSPIC_SEC_HF_DS, burstbrk_reg, 0);
        REG_SET_FIELD(QSPIC, QSPIC_BURSTBRK_REG, QSPIC_BRK_EN, burstbrk_reg, mode);
        REG_SET_FIELD(QSPIC, QSPIC_BURSTBRK_REG, QSPIC_BRK_SZ, burstbrk_reg, ((addr_size == HW_QSPI_ADDR_SIZE_32) ? 1 : 0));

        QSPIBA(id)->QSPIC_BURSTBRK_REG = burstbrk_reg;
}

/**
 * \brief       Set the address of the block/sector that is requested to be erased.
 *
 * \param [in]  id         QSPI controller id
 * \param [in]  erase_addr Address to erase.
 *
 */
__STATIC_FORCEINLINE void hw_qspi_set_erase_address(HW_QSPIC_ID id, uint32_t erase_addr)
{
        HW_QSPIC_REG_SETF(id, ERASECTRL, ERS_ADDR, erase_addr);
}

/**
 * \brief       Trigger erase block/sector
 *
 * \param [in]  id QSPI controller id
 *
 */
__STATIC_FORCEINLINE void hw_qspi_trigger_erase(HW_QSPIC_ID id)
{
        HW_QSPIC_REG_SET_BIT(id, ERASECTRL, ERASE_EN);
}

/**
 * \brief       Get erase status
 *
 * \param [in]  id QSPI controller id
 *
 * \return      The status of sector/block erasing
 *
 * \sa          HW_QSPI_ERASE_STATUS
 */
__STATIC_FORCEINLINE HW_QSPI_ERASE_STATUS hw_qspi_get_erase_status(HW_QSPIC_ID id)
{
        // Dummy access to QSPIC_CHCKERASE_REG in order to trigger a read status command
        HW_QSPIC_REG_SETF(id, CHCKERASE, CHCKERASE, 0);
        return (HW_QSPI_ERASE_STATUS) HW_QSPIC_REG_GETF(id, ERASECTRL, ERS_STATE);
}

/**
 * \brief       Erase block/sector of flash memory
 *
 * \note        Before erasing the flash memory, it is mandatory to set up the erase instructions
 *              first by calling hw_qspi_erase_instr_init().
 *
 * \note        Call hw_qspi_get_erase_status() to check whether the erase operation has finished.
 *
 * \note        Before switching the QSPI controller to manual mode check that
 *              hw_qspi_get_erase_status() == HW_QSPI_ERASE_STATUS_NO.
 *
 * \param [in]  id   QSPI controller id
 * \param [in]  addr memory address of the block/sector to be erased
 *
 * \sa          hw_qspi_erase_instr_init
 * \sa          hw_qspi_get_erase_status
 */
__RETAINED_CODE void hw_qspi_erase_block(HW_QSPIC_ID id, uint32_t addr);

/**
 * \brief Set an extra byte to use with read instructions
 *
 * \param [in] id         QSPI controller id
 * \param [in] extra_byte an extra byte transferred after the address asking memory to
 *                        stay in continuous read mode or wait for a normal instruction
 *                        after CS goes inactive
 * \param [in] bus_mode the mode of the SPI bus during the extra byte phase.
 * \param [in] half_disable_out true  - disable (hi-z) output during the transmission
 *                                      of bits [3:0] of extra byte
 *                              false - transmit the complete extra byte
 *
 * \sa hw_qspi_set_read_instruction
 *
 */
__STATIC_FORCEINLINE void hw_qspi_set_extra_byte(HW_QSPIC_ID id, uint8_t extra_byte,
                                                 HW_QSPI_BUS_MODE bus_mode, bool half_disable_out)
{
        HW_QSPIC_REG_SETF(id, BURSTCMDA, EXT_BYTE, extra_byte);
        HW_QSPIC_REG_SETF(id, BURSTCMDA, EXT_TX_MD, bus_mode);

        HW_QSPIC_REG_SETF(id, BURSTCMDB, EXT_BYTE_EN, 1);
        HW_QSPIC_REG_SETF(id, BURSTCMDB, EXT_HF_DS, half_disable_out);
}

/**
 * \brief Enable the 'exit from continuous read mode' sequence in automode
 *
 * \param [in] id QSPI controller id
 *
 */
__STATIC_FORCEINLINE void hw_qspi_exit_continuous_mode_sequence_enable(HW_QSPIC_ID id)
{
        HW_QSPIC_REG_SET_BIT(id, BURSTBRK, BRK_EN);
}

/**
 * \brief Disable the 'exit from continuous read mode' sequence in automode
 *
 * \param [in] id QSPI controller id
 *
 */
__STATIC_FORCEINLINE void hw_qspi_exit_continuous_mode_sequence_disable(HW_QSPIC_ID id)
{
        HW_QSPIC_REG_CLR_BIT(id, BURSTBRK, BRK_EN);
}
#endif /* dg_configUSE_HW_QSPI */


#endif /* HW_QSPI_V2_H_ */

/**
 * \}
 * \}
 */
