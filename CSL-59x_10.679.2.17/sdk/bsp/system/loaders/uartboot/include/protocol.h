/**
 ****************************************************************************************
 *
 * @file protocol.h
 *
 * @brief Common protocol definitions
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

#ifndef PROTOCOL_H
#define PROTOCOL_H

/**************************************************************************************************
 * Special characters used by protocol
 *************************************************************************************************/

/**
 * \brief Start of Header
 *
 * Sent as the beginning of each command. Second byte of the 'Hello message'.
 */
#define SOH '\x01'

/**
 * \brief Start of Text
 *
 * Sent at the beginning 'Hello message'.
 */
#define STX '\x02'

/**
 * \brief Acknowledge
 *
 * Sent by uartboot or the host application when the received command/data is valid.
 */
#define ACK '\x06'

/**
 * \brief Negative Acknowledge
 *
 * Sent by uartboot or the host application when the received command/data is not valid.
 */
#define NAK '\x15'


/**************************************************************************************************
 * Command for basic functions needed by protocol
 *************************************************************************************************/

/**
 * \brief Write data to RAM
 *
 */
#define CMD_WRITE                  0x01

/**
 * \brief Read data from RAM
 *
 */
#define CMD_READ                   0x02

/**
 * \brief Erase a part of the QSPI FLASH memory
 *
 */
#define CMD_COPY_QSPI              0x03

/**
 * \brief Copy data from RAM to QSPI FLASH memory
 *
 */
#define CMD_ERASE_QSPI             0x04

/**
 * \brief Execute a part of the code
 *
 */
#define CMD_RUN                    0x05

/**
 * \brief Write data to the OTP memory
 *
 */
#define CMD_WRITE_OTP              0x06

/**
 * \brief Read data from the OTP memory
 *
 */
#define CMD_READ_OTP               0x07

/**
 * \brief Read data from QSPI memory
 *
 */
#define CMD_READ_QSPI              0x08

/**
 * \brief Perform a customer specific action
 *
 * Handler for this command should be implemented by customer.
 */
#define CMD_CUSTOMER_SPECIFIC      0x09

/**
 * \brief Read partition table from QSPI memory
 *
 */
#define CMD_READ_PARTITION_TABLE   0x0A

/**
 * \brief Get uartboot's version
 *
 */
#define CMD_GET_VERSION            0x0B

/**
 * \brief Erase whole QSPI FLASH memory
 *
 */
#define CMD_CHIP_ERASE_QSPI        0x0C

/**
 * \brief Check that the part of the QSPI FLASH memory is empty
 *
 */
#define CMD_IS_EMPTY_QSPI          0x0D

/**
 * \brief Read data from QSPI FLASH partition
 *
 * This command is available only when dg_configNVMS_ADAPTER is enabled.
 *
 */
#define CMD_READ_PARTITION         0x0E

/**
 * \brief Copy data from RAM to QSPI FLASH partition
 *
 * This command is available only when dg_configNVMS_ADAPTER is enabled.
 *
 */
#define CMD_WRITE_PARTITION        0x0F

/**
 * \brief Check QSPI memory state
 *
 * If the QSPI memory is connected and recognized then returns device identification.
 *
 */
#define CMD_GET_QSPI_STATE         0x10

/**
 * \brief Enable external watchdog triggering
 *
 * Start square wave on QPIO pin: 15ms high state, 2s low state.
 *
 */
#define CMD_GPIO_WD                0x11

/**
 * \brief Write data directly to the QSPI FLASH memory
 *
 */
#define CMD_DIRECT_WRITE_TO_QSPI   0x12

/**
 * \brief Erase whole eFLASH memory
 *
 */
#define CMD_CHIP_ERASE_EFLASH      0x13

/**
 * \brief Read data from eFLASH memory
 *
 */
#define CMD_READ_EFLASH            0x14

/**
 * \brief Write data directly to the eFLASH memory
 *
 */
#define CMD_DIRECT_WRITE_TO_EFLASH 0x15

/**
 * \brief Copy data from RAM to eFLASH
 *
 */
#define CMD_COPY_EFLASH            0x16

/**
 * \brief Erase part of the OQSPI FLASH memory
 *
 */
#define CMD_ERASE_EFLASH           0x17

/**
 * \brief Check that (part of) the eFLASH memory is empty
 *
 */
#define CMD_IS_EMPTY_EFLASH        0x18

/**
 * \brief Get product information
 *
 */
#define CMD_GET_PRODUCT_INFO       0x1B

/**
 * \brief Change communication UART's baudrate
 *
 */
#define CMD_CHANGE_BAUDRATE        0x30

/**
 * \brief Write 'Live' marker to the data buffer
 *
 */
#define CMD_DUMMY                  0xFF

#endif /* PROTOCOL_H */
