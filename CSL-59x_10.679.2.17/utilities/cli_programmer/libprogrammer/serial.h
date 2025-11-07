/**
 ****************************************************************************************
 *
 * @file serial.h
 *
 * @brief Serial port API.
 *
 * Copyright (C) 2015-2018 Renesas Electronics Corporation and/or its affiliates.
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

/**
 * \addtogroup UTILITIES
 * \{
 */

#ifndef SERIAL_H_
#define SERIAL_H_

#include <stdint.h>

/**
 * \brief Open serial port
 *
 * \param [in] port Serial port
 * \param [in] baudrate Serial port baudrate
 *
 * \return 1 if port is opened and configured
 *
 */
int serial_open(const char *port, int baudrate);

/**
 * \brief Write to serial port
 *
 * \param [in] buffer Data to send
 * \param [in] length Number of characters to write
 *
 * \return Number of written characters
 *
 */
int serial_write(const uint8_t *buffer, size_t length);

/**
 * \brief Read from serial port
 *
 * \param [out] buffer Received data
 * \param [in] length Number of characters to read
 * \param [in] timeout Time to wait for next character
 *
 * \return Number of read characters
 *
 */
int serial_read(uint8_t *buffer, size_t length, uint32_t timeout);

/**
 * \brief Close serial port
 *
 */
void serial_close(void);

/**
 * \brief Set BAUD rate for an already opened serial.
 *
 * \param [in] baudrate Serial port BAUD rate
 *
 * \return The previously used BAUD rate
 */
int serial_set_baudrate(int baudrate);

#endif /* SERIAL_H_ */

/**
 * \}
 */
