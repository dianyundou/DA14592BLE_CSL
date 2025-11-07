/**
 * \addtogroup MID_INT_BLE_MANAGER
 * \{
 * \addtogroup BLE_MANAGER_COMON Common
 *
 * \brief BLE Manager common definitions and handlers
 * \{
 */

/**
 ****************************************************************************************
 *
 * @file ble_mgr_common.h
 *
 * @brief BLE manager common definitions and handlers
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

#ifndef BLE_MGR_COMMON_H_
#define BLE_MGR_COMMON_H_

#include "osal.h"
#include "ble_mgr.h"
#include "ble_mgr_cmd.h"

enum ble_mgr_common_cmd_opcode {
        BLE_MGR_COMMON_STACK_MSG = BLE_MGR_CMD_CAT_FIRST(BLE_MGR_COMMON_CMD_CAT),
        BLE_MGR_COMMON_REGISTER_CMD,
        BLE_MGR_COMMON_ENABLE_CMD,
        BLE_MGR_COMMON_RESET_CMD,
        BLE_MGR_COMMON_READ_TX_POWER_CMD,
        /* Dummy command opcode, needs to be always defined after all commands */
        BLE_MGR_COMMON_LAST_CMD,
};

/** Definition of BLE stack message */
typedef struct ble_msg {
        ble_mgr_msg_hdr_t     hdr;        /**< Message header (op_code and msg_len) */
        ble_stack_msg_type_t  msg_type;   /**< Stack message type (GTL, HCI CMD, HCI ACL,
                                                                   HCI SCO or HCI EVT) */
        ble_stack_msg_t       msg;        /**< Stack message place holder */
} ble_mgr_common_stack_msg_t;

void ble_mgr_common_stack_msg_handler(void *param);

typedef struct {
        ble_mgr_msg_hdr_t   hdr;
        OS_TASK             task;
} ble_mgr_common_register_cmd_t;

typedef struct {
        ble_mgr_msg_hdr_t   hdr;
        ble_error_t         status;
} ble_mgr_common_register_rsp_t;

void ble_mgr_common_register_cmd_handler(void *param);

typedef struct {
        ble_mgr_msg_hdr_t   hdr;
} ble_mgr_common_enable_cmd_t;

typedef struct {
        ble_mgr_msg_hdr_t   hdr;
        ad_ble_status_t     status;
} ble_mgr_common_enable_rsp_t;

void ble_mgr_common_enable_cmd_handler(void *param);

typedef struct {
        ble_mgr_msg_hdr_t   hdr;
} ble_mgr_common_reset_cmd_t;

typedef struct {
        ble_mgr_msg_hdr_t   hdr;
        ad_ble_status_t     status;
} ble_mgr_common_reset_rsp_t;

void ble_mgr_common_reset_cmd_handler(void *param);

typedef struct {
        ble_mgr_msg_hdr_t   hdr;
        uint16_t            conn_idx;
        tx_power_level_type_t type;
} ble_mgr_common_read_tx_power_cmd_t;

typedef struct {
        ble_mgr_msg_hdr_t   hdr;
        ble_error_t         status;
        uint8_t             tx_power_level;
} ble_mgr_common_read_tx_power_rsp_t;

void ble_mgr_common_read_tx_power_cmd_handler(void *param);

#endif /* BLE_MGR_COMMON_H_ */
/**
 \}
 \}
 */
