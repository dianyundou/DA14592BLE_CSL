/**
 * \addtogroup MID_INT_BLE_MANAGER
 * \{
 * \addtogroup BLE_MANAGER_MSG Message Handling
 *
 * \brief Helper library API for BLE adapter message handling in BLE Manager
 * \{
 */

/**
 ****************************************************************************************
 *
 * @file ble_mgr_ad_msg.h
 *
 * @brief Helper library API for BLE adapter message handling in BLE Manager
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

#ifndef BLE_MGR_AD_MSG_H_
#define BLE_MGR_AD_MSG_H_

#include <stdbool.h>
#include "ad_ble.h"

/**
 * \brief Waitqueue callback
 *
 */
typedef void (* ble_ad_msg_wqueue_cb_t) (ad_ble_msg_t *ad_msg, void *param);

/**
 * \brief Allocate buffer for a BLE adapter message
 *
 * Message will be set with passed operation code.
 *
 * \param [in] operation  BLE adapter operation code
 * \param [in] len        message length
 *
 * \return message pointer
 *
 */
void *ble_ad_msg_alloc(ad_ble_operation_t operation, uint16_t len);

/**
 * \brief Send BLE adapter message to adapter
 *
 * \param [in] msg      message pointer
 *
 */
__STATIC_INLINE void ble_ad_msg_send(void *msg)
{
        ad_ble_command_queue_send(&msg, OS_QUEUE_FOREVER);
}

/**
 * \brief Add callback to waitqueue
 *
 * This adds callback for waitqueue associated with specific combination of response's operation
 * code and command's operation code.
 *
 * \param [in] rsp_op
 * \param [in] cmd_op
 * \param [in] cb
 * \param [in] param
 */
void ble_ad_msg_wqueue_add(ad_ble_operation_t rsp_op, ad_ble_operation_t cmd_op,
                           ble_ad_msg_wqueue_cb_t cb, void *param);

/**
 * Match BLE adapter message against waitqueue
 *
 * On positive match, this will remove element from waitqueue and fire associated callback.
 *
 * \param [in] ad_msg      BLE adapter message pointer
 *
 * \return true if matches, false otherwise
 *
 */
bool ble_ad_msg_waitqueue_match(ad_ble_msg_t *ad_msg);

/**
 * Follow-up action on reception of a BLE_ADAPTER_CMP_EVT from the BLE adapter
 *
 * \param [in] ad_msg     BLE adapter message pointer
 * \param [in] param      Pointer to passing parameter
 */
void ble_adapter_cmp_evt_init(ad_ble_msg_t *ad_msg, void *param);

#endif /* BLE_MGR_AD_MSG_H_ */
/**
 \}
 \}
 */
