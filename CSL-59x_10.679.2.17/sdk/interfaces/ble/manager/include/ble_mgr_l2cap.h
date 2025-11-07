/**
 * \addtogroup MID_INT_BLE
 * \{
 * \addtogroup MID_INT_BLE_MANAGER
 * \{
 */

/**
 ****************************************************************************************
 *
 * @file ble_mgr_l2cap.h
 *
 * @brief BLE manager definitions and handlers for L2CAP
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

#ifndef BLE_MGR_L2CAP_H_
#define BLE_MGR_L2CAP_H_

#include <stdint.h>
#include <stdbool.h>
#include "osal.h"
#include "ble_mgr_cmd.h"
#include "ble_mgr_gtl.h"
#include "ble_l2cap.h"

/** OP codes for L2CAP commands */
enum ble_cmd_l2cap_opcode {
        BLE_MGR_L2CAP_LISTEN_CMD  = BLE_MGR_CMD_CAT_FIRST(BLE_MGR_L2CAP_CMD_CAT),
        BLE_MGR_L2CAP_STOP_LISTEN_CMD,
        BLE_MGR_L2CAP_CONNECTION_CFM_CMD,
        BLE_MGR_L2CAP_CONNECT_CMD,
        BLE_MGR_L2CAP_DISCONNECT_CMD,
        BLE_MGR_L2CAP_ADD_CREDITS_CMD,
        BLE_MGR_L2CAP_SEND_CMD,
        /* Dummy command opcode, needs to be always defined after all commands */
        BLE_MGR_L2CAP_LAST_CMD,
};

typedef struct {
        ble_mgr_msg_hdr_t   hdr;
        uint16_t            conn_idx;
        uint16_t            psm;
        gap_sec_level_t     sec_level;
        uint16_t            initial_credits;
        bool                defer_setup;
} ble_mgr_l2cap_listen_cmd_t;

typedef struct {
        ble_mgr_msg_hdr_t   hdr;
        uint16_t            conn_idx;
        ble_error_t         status;
        uint16_t            scid;
} ble_mgr_l2cap_listen_rsp_t;

void ble_mgr_l2cap_listen_cmd_handler(void *param);

typedef struct {
        ble_mgr_msg_hdr_t   hdr;
        uint16_t            conn_idx;
        uint16_t            scid;
} ble_mgr_l2cap_stop_listen_cmd_t;

typedef struct {
        ble_mgr_msg_hdr_t   hdr;
        uint16_t            conn_idx;
        ble_error_t         status;
        uint16_t            scid;
} ble_mgr_l2cap_stop_listen_rsp_t;

void ble_mgr_l2cap_stop_listen_cmd_handler(void *param);

typedef struct {
        ble_mgr_msg_hdr_t   hdr;
        uint16_t            conn_idx;
        uint16_t            scid;
        uint16_t            status;
} ble_mgr_l2cap_connection_cfm_cmd_t;

typedef struct {
        ble_mgr_msg_hdr_t   hdr;
        ble_error_t         status;
} ble_mgr_l2cap_connection_cfm_rsp_t;

void ble_mgr_l2cap_connection_cfm_cmd_handler(void *param);

typedef struct {
        ble_mgr_msg_hdr_t   hdr;
        uint16_t            conn_idx;
        uint16_t            psm;
        uint16_t            initial_credits;
} ble_mgr_l2cap_connect_cmd_t;

typedef struct {
        ble_mgr_msg_hdr_t   hdr;
        ble_error_t         status;
        uint16_t            scid;
} ble_mgr_l2cap_connect_rsp_t;

void ble_mgr_l2cap_connect_cmd_handler(void *param);

typedef struct {
        ble_mgr_msg_hdr_t   hdr;
        uint16_t            conn_idx;
        uint16_t            scid;
} ble_mgr_l2cap_disconnect_cmd_t;

typedef struct {
        ble_mgr_msg_hdr_t   hdr;
        ble_error_t         status;
        uint16_t            scid;
} ble_mgr_l2cap_disconnect_rsp_t;

void ble_mgr_l2cap_disconnect_cmd_handler(void *param);

typedef struct {
        ble_mgr_msg_hdr_t   hdr;
        uint16_t            conn_idx;
        uint16_t            scid;
        uint16_t            credits;
} ble_mgr_l2cap_add_credits_cmd_t;

typedef struct {
        ble_mgr_msg_hdr_t   hdr;
        ble_error_t         status;
        uint16_t            conn_idx;
        uint16_t            scid;
        uint16_t            credits;
} ble_mgr_l2cap_add_credits_rsp_t;

void ble_mgr_l2cap_add_credits_cmd_handler(void *param);

typedef struct {
        ble_mgr_msg_hdr_t   hdr;
        uint16_t            conn_idx;
        uint16_t            scid;
        uint16_t            length;
        uint8_t             data[0];
} ble_mgr_l2cap_send_cmd_t;

typedef struct {
        ble_mgr_msg_hdr_t   hdr;
        ble_error_t         status;
} ble_mgr_l2cap_send_rsp_t;

void ble_mgr_l2cap_send_cmd_handler(void *param);

/**
 * BLE stack event handlers
 */
void ble_mgr_l2cap_connect_ind_evt_handler(ble_gtl_msg_t *gtl);

void ble_mgr_l2cap_disconnect_ind_evt_handler(ble_gtl_msg_t *gtl);

void ble_mgr_l2cap_connect_req_ind_evt_handler(ble_gtl_msg_t *gtl);

void ble_mgr_l2cap_add_ind_evt_handler(ble_gtl_msg_t *gtl);

void ble_mgr_l2cap_pdu_send_rsp_evt_handler(ble_gtl_msg_t *gtl);

void ble_mgr_l2cap_lecnx_data_recv_ind_evt_handler(ble_gtl_msg_t *gtl);

void ble_mgr_gapc_cmp__le_cb_connection_evt_handler(ble_gtl_msg_t *gtl);

void ble_mgr_l2cap_disconnect_ind(uint16_t conn_idx);

#endif /* BLE_MGR_L2CAP_H_ */
/**
 \}
 \}
 */
