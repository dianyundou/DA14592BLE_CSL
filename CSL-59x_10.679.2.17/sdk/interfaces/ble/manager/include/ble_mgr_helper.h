/**
 * \addtogroup MID_INT_BLE
 * \{
 * \addtogroup MID_INT_BLE_MANAGER
 * \{
 */

/**
 ****************************************************************************************
 *
 * @file ble_mgr_helper.h
 *
 * @brief BLE message creation and handling
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

#ifndef BLE_MGR_HELPER_H_
#define BLE_MGR_HELPER_H_

#include <stdbool.h>
#include "osal.h"
#include "ble_mgr_cmd.h"

/**
 * \brief Allocates new BLE message
 *
 * \param [in] op_code  message op code
 * \param [in] size     message size
 *
 * \return allocated message pointer
 *
 */
void *alloc_ble_msg(uint16_t op_code, uint16_t size);

/**
 * \brief Initialize BLE message
 *
 * Initialize a BLE message with op_code and size. This will allocate a buffer with size equal to
 * \p size and fill it's op_code with \p op_code.
 *
 * \param [in]      op_code  message op code
 * \param [in]      size     message size
 *
 * \return allocated message pointer
 *
 */
void *ble_msg_init(uint16_t op_code, uint16_t size);

/**
 * \brief Initialize BLE event
 *
 * Initialize a BLE event with evt_code and size. This will allocate a buffer with size equal to
 * \p size and fill it's op_code with \p evt_code.
 *
 * \param [in]      evt_code event code
 * \param [in]      size     event size
 *
 * \return allocated event buffer pointer
 */
void *ble_evt_init(uint16_t evt_code, uint16_t size);

/**
 * \brief Free BLE message buffer
 *
 * \param [in]      msg      Message buffer
 *
 */
void ble_msg_free(void *msg);

/**
 * \brief Execute BLE command
 *
 * This calls manager's \p handler with command (if #BLE_MGR_DIRECT_ACCESS is defined to 1) or sends
 * it to manager's command queue (if #BLE_MGR_DIRECT_ACCESS is defined to 0) and waits for response.
 * Buffer pointed by \p cmd is owned by the BLE manager after calling this function and should not
 * be accessed.
 * Buffer returned in \p rsp is owned by caller and should be freed there.
 *
 * \param [in]  cmd      command buffer
 * \param [out] rsp      response buffer
 * \param [in]  handler  command handler
  *
 * \return true if executed successfully, false otherwise
 *
 */
bool ble_cmd_execute(void *cmd, void **rsp, ble_mgr_cmd_handler_t handler);

#endif /* BLE_MGR_HELPER_H_ */
/**
 \}
 \}
 */
