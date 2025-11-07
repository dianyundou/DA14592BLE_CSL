/**
 ****************************************************************************************
 *
 * @file cmac_mailbox.h
 *
 * @brief cmac_mailbox Driver for HCI over cmac_mailbox operation.
 *
 * Copyright (C) 2017-2023 Renesas Electronics Corporation and/or its affiliates.
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


#ifndef _cmac_mailbox_H_
#define _cmac_mailbox_H_

/**
 ****************************************************************************************
 * @defgroup cmac_mailbox cmac_mailbox
 * @ingroup DRIVERS
 *
 * @brief cmac_mailbox driver
 *
 * @{
 *
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include <stdint.h>       // integer definition
#include <stdbool.h>      // boolean definition


/*
 * ENUMERATION DEFINITIONS
 *****************************************************************************************
 */

/// Generic enable/disable enum for cmac_mailbox driver
enum
{
    /// cmac_mailbox disable
    CMAC_MAILBOX_DISABLE = 0,
    /// cmac_mailbox enable
    CMAC_MAILBOX_ENABLE  = 1
};

/// Status bits
enum
{
    /// HCI write
    CMAC_MAILBOX_HCI = 1,
    /// Error
    CMAC_MAILBOX_ERROR = 2,
    /// Flow on/off
    CMAC_MAILBOX_FLOW = 4,
    /// Has been reset
    CMAC_MAILBOX_RESET = 8,
    /// Write Pending
    CMAC_MAILBOX_WRITE_PEND = 0x10,
};

/// return status values
enum
{
    /// status ok
    CMAC_MAILBOX_STATUS_OK,
    /// status not ok
    CMAC_MAILBOX_STATUS_ERROR
};

/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */

#if !(CMAC_CPU)
/**
 ****************************************************************************************
 * @brief Zero-initializes the CMAC mailbox memory.
 *
 * @details     Calling of this function is both safe and required only when
 *              the CMAC image is downloaded manually (not by the host).
 *              It is available only on the host.
 *****************************************************************************************
 */
void cmac_mailbox_init_mem(void);
#endif

/**
 ****************************************************************************************
 * @brief Initializes the cmac_mailbox to default values.
 *****************************************************************************************
 */
void cmac_mailbox_init(void);

/**
 ****************************************************************************************
 * @brief Enable cmac_mailbox flow.
 *****************************************************************************************
 */
void cmac_mailbox_flow_on(void);

/**
 ****************************************************************************************
 * @brief Disable cmac_mailbox flow.
 *****************************************************************************************
 */
bool cmac_mailbox_flow_off(void);

/**
 ****************************************************************************************
 * @brief Finish current cmac_mailbox transfers
 *****************************************************************************************
 */
void cmac_mailbox_finish_transfers(void);

/**
 ****************************************************************************************
 * @brief Starts a data reception.
 *
 * As soon as the end of the data transfer or a buffer overflow is detected,
 * the callback (if not NULL) is executed.
 *
 * @param[in,out]  bufptr   Pointer to the RX buffer
 * @param[in]      size     Size of the expected reception
 * @param[in]      callback The function to call when reading finishes (can be NULL)
 *****************************************************************************************
 */
void cmac_mailbox_read(uint8_t *bufptr, uint32_t size, void (*callback) (uint8_t));

/**
 ****************************************************************************************
 * @brief Starts a data transmission.
 *
 * As soon as the end of the data transfer is detected, the callback (if not NULL) is
 * executed.
 *
 * @param[in]  bufptr   Pointer to the TX buffer
 * @param[in]  size     Size of the transmission
 * @param[in]  callback The function to call when writing finishes (can be NULL)
 *****************************************************************************************
 */
void cmac_mailbox_write(uint8_t *bufptr, uint32_t size, void (*callback) (uint8_t));

/**
 ****************************************************************************************
 * @brief Set status to indicate whether write is pending or not.
 *
 * If write cannot be performed the status will be set to indicate that
 *
 * @param[in]  val 0 write is not pending, other write is pending
 *****************************************************************************************
 */
void cmac_mailbox_write_pend_set(uint32_t val);

/**
 ****************************************************************************************
 * @brief Serves the data transfer interrupt requests.
 *
 * It clears the requests and executes the appropriate callback function.
 *****************************************************************************************
 */
void cmac_mailbox_isr(void);
void cmac_mailbox_set_flow_off_retries_limit(uint32_t );
/// @} cmac_mailbox
#endif /* _cmac_mailbox_H_ */
