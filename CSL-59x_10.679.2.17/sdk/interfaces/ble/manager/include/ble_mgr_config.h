/**
 * \addtogroup MID_INT_BLE_MANAGER
 * \{
 * \addtogroup BLE_MANAGER_CFG Configuration
 *
 * \brief BLE Manager Configuration
 * \{
 */

/**
 ****************************************************************************************
 *
 * @file ble_mgr_config.h
 *
 * @brief BLE Manager Configuration
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

#ifndef BLE_MGR_CONFIG_H
#define BLE_MGR_CONFIG_H

#include "ble_config.h"

/**
 * \brief Directly call BLE manager handlers
 *
 * When #BLE_MGR_DIRECT_ACCESS is defined to 1, the respective BLE manager command handler will be
 * directly called by the BLE API calling task in the calling task's context and the BLE manager's
 * command queue will not be used to interface with the BLE framework.
 */
#ifndef BLE_MGR_DIRECT_ACCESS
#define BLE_MGR_DIRECT_ACCESS            (1)
#endif

/**
 * \brief Use BLE event list instead of queue
 *
 * Use a dynamic-size list for BLE events instead of a fixed-size queue.
 */
#ifndef BLE_MGR_USE_EVT_LIST
#define BLE_MGR_USE_EVT_LIST             (1)
#endif

/**
 * \brief DOWN queue size
 *
 * Defines the DOWN (client app to BLE manager) queue length, in number of messages
 *
 */
#ifndef BLE_MGR_COMMAND_QUEUE_LENGTH
#define BLE_MGR_COMMAND_QUEUE_LENGTH     (1)
#endif

/**
 * \brief UP queue size
 *
 * Defines the UP (BLE manager to application) queue length, in number of messages
 *
 */
#ifndef BLE_MGR_EVENT_QUEUE_LENGTH
#define BLE_MGR_EVENT_QUEUE_LENGTH       (8)
#endif

/**
 * \brief UP response queue size
 *
 * Defines the UP (BLE manager to application) response queue length, in number of messages
 *
 */
#define BLE_MGR_RESPONSE_QUEUE_LENGTH    (1)

#endif /* BLE_MGR_CONFIG_H */
/**
 \}
 \}
 */
