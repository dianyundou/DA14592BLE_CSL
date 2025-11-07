/**
 * \addtogroup MID_INT_DGTL
 * \{
 * \addtogroup DGTL_CFG Configuration
 *
 * \brief DGTL configuration
 *
 * \{
 */

/**
 ****************************************************************************************
 *
 * @file dgtl_config.h
 *
 * @brief Default DGTL configuration
 *
 * Copyright (C) 2016-2019 Renesas Electronics Corporation and/or its affiliates.
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

#ifndef DGTL_CONFIG_H_
#define DGTL_CONFIG_H_

/**
 * \brief Enable application-specific HCI commands
 *
 * When this macro is enabled, HCI commands with an opcode within the vendor-specific
 * address range (0xFE00 - 0xFFFF) will be forwarded to an application-specific callback.
 *
 * Please see \p sdk/middleware/dgtl/include/dgtl.h for more details.
 *
 */
#ifndef DGTL_APP_SPECIFIC_HCI_ENABLE
#define DGTL_APP_SPECIFIC_HCI_ENABLE    (0)
#endif

/**
 * \brief Enable LOG Queue dropped messages counter
 *
 * The LOG queue, in contrast to the rest of the queues, works in
 * non-blocking mode. That means that if there is no more space in the queue
 * left, a message to be pushed into the queue will be silently dropped.
 *
 * When this macro is enabled, DGTL instantiates a counter that counts the dropped
 * messages.
 *
 */
#ifndef DGTL_DROPPED_LOG_QUEUE_COUNTER
#define DGTL_DROPPED_LOG_QUEUE_COUNTER  (0)
#endif

/**
 * \brief Enable DGTL HCI/GTL queue
 *
 * This macro enables the use of the DGTL HCI/GTL queue (packet types 1-5)
 *
 */
#ifndef DGTL_QUEUE_ENABLE_HCI
#define DGTL_QUEUE_ENABLE_HCI           (1)
#endif

/**
 * \brief Enable DGTL APP queue
 *
 * This macro enables the use of the DGTL APP queue (packet types 6-7)
 *
 */
#ifndef DGTL_QUEUE_ENABLE_APP
#define DGTL_QUEUE_ENABLE_APP           (1)
#endif

/**
 * \brief Enable DGTL LOG queue
 *
 * This macro enables the use of the DGTL LOG queue (packet type 8)
 *
 */
#ifndef DGTL_QUEUE_ENABLE_LOG
#define DGTL_QUEUE_ENABLE_LOG           (1)
#endif

/**
 * \brief Enable flow control over DGTL transport by default
 *
 * This macro enables the use of flow control feature on serial interface
 *
 */
#ifndef DGTL_AUTO_FLOW_CONTROL
#define DGTL_AUTO_FLOW_CONTROL          (1)
#endif

#endif /* DGTL_CONFIG_H_ */

/**
 * \}
 * \}
 */
