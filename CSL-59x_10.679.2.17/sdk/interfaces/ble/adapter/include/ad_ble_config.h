/**
 * \addtogroup MID_INT_BLE_ADAPTER
 * \{
 * \addtogroup BLE_ADAPTER_CONFIG Configuration
 *
 * \brief BLE Adapter configuration
 * \{
 */

/**
 ****************************************************************************************
 *
 * @file ad_ble_config.h
 *
 * @brief BLE Adapter configuration
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

#ifndef AD_BLE_CONFIG_H
#define AD_BLE_CONFIG_H

/**
 * \brief UP Queue size
 *
 * Defines the UP (adapter to manager) Queue length, in number of messages, if not defined
 *
 */
#ifndef AD_BLE_EVENT_QUEUE_LENGTH
#define AD_BLE_EVENT_QUEUE_LENGTH    8
#endif

/**
 * \brief DOWN Queue size
 *
 * Defines the DOWN (manager to adapter) Queue length, in number of messages, if not defined
 *
 */
#ifndef AD_BLE_COMMAND_QUEUE_LENGTH
#define AD_BLE_COMMAND_QUEUE_LENGTH  8
#endif


#endif /* AD_BLE_CONFIG_H */
/**
 \}
 \}
 */
