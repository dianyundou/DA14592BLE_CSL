/**
 * \addtogroup MID_INT_BLE_MANAGER
 * \{
 * \addtogroup BLE_MANAGER_CMD Commands
 *
 * \brief BLE manager command definitions
 * \{
 */

/**
 ****************************************************************************************
 *
 * @file ble_mgr_cmd.h
 *
 * @brief BLE manager command definitions
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

#ifndef BLE_MGR_CMD_H_
#define BLE_MGR_CMD_H_

#include <stdbool.h>
#include "osal.h"

typedef void (* ble_mgr_cmd_handler_t) (void *param);

/**
 * Common header for all BLE messages
 */
typedef struct {
        uint16_t        op_code;
        uint16_t        msg_len;
        uint8_t         payload[0];
} ble_mgr_msg_hdr_t;

/**
 * BLE command categories
 */
enum ble_cmd_cat {
        BLE_MGR_COMMON_CMD_CAT  = 0x00,
        BLE_MGR_GAP_CMD_CAT     = 0x01,
        BLE_MGR_GATTS_CMD_CAT   = 0x02,
        BLE_MGR_GATTC_CMD_CAT   = 0x03,
        BLE_MGR_L2CAP_CMD_CAT   = 0x04,
        BLE_MGR_LAST_CMD_CAT,
};

#define BLE_MGR_CMD_CAT_FIRST(CAT) (CAT << 8)

#define BLE_MGR_CMD_GET_CAT(OPCODE) (OPCODE >> 8)
#define BLE_MGR_CMD_GET_IDX(OPCODE) (OPCODE & 0xFF)

bool ble_mgr_cmd_handle(void *cmd);

#endif /* BLE_MGR_CMD_H_ */
/**
 \}
 \}
 */
