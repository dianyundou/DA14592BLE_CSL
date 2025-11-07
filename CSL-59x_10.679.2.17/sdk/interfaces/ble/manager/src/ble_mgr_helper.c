/**
 ****************************************************************************************
 *
 * @file ble_mgr_helper.c
 *
 * @brief BLE message creation and handling
 *
 * Copyright (C) 2015-2019 Renesas Electronics Corporation and/or its affiliates.
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

#include <string.h>
#include "ble_mgr.h"
#include "ble_mgr_config.h"
#include "ble_mgr_cmd.h"
#include "ble_mgr_helper.h"
#include "ble_common.h"

void *alloc_ble_msg(uint16_t op_code, uint16_t size)
{
        ble_mgr_msg_hdr_t *msg;

        /* Allocate at least the size needed for the base message */
        OS_ASSERT(size >= sizeof(ble_mgr_msg_hdr_t));

        msg = OS_MALLOC(size);
        memset(msg, 0, size);
        msg->op_code  = op_code;
        msg->msg_len = size - sizeof(ble_mgr_msg_hdr_t);

        return msg;
}

static void *alloc_evt(uint16_t evt_code, uint16_t size)
{
        ble_evt_hdr_t *evt;

        /* Allocate at least the size needed for the base message */
        OS_ASSERT(size >= sizeof(*evt));

        evt = OS_MALLOC(size);
        memset(evt, 0, size);
        evt->evt_code = evt_code;
        evt->length = size - sizeof(*evt);

        return evt;
}

void *ble_msg_init(uint16_t op_code, uint16_t size)
{
        /* Allocate at least the size needed for the base message */
        OS_ASSERT(size >= sizeof(ble_mgr_msg_hdr_t));

        return alloc_ble_msg(op_code, size);
}

void *ble_evt_init(uint16_t evt_code, uint16_t size)
{
        /* Allocate at least the size needed for the base message */
        OS_ASSERT(size >= sizeof(ble_evt_hdr_t));

        return alloc_evt(evt_code, size);
}

void ble_msg_free(void *msg)
{
        if (msg) {
                OS_FREE(msg);
        }
}


bool ble_cmd_execute(void *cmd, void **rsp, ble_mgr_cmd_handler_t handler)
{
        uint16_t op_code __UNUSED;
        ble_status_t ble_status = ble_mgr_get_status();

        if ((ble_status == BLE_IS_BUSY) || (ble_status == BLE_IS_RESET)) {
                /* Command is expected to be freed by recipient, so free it right away */
                OS_FREE(cmd);
                return false;
        }

        /* Save opcode for response check */
        op_code = ((ble_mgr_msg_hdr_t *)cmd)->op_code;

        /* Acquire the BLE manager interface */
        ble_mgr_acquire();

#if (BLE_MGR_DIRECT_ACCESS == 1)
        /* Call BLE manager's handler and wait for response */
        handler(cmd);
#else
        /* Send to BLE manager's command queue and wait for response */
        ble_mgr_command_queue_send(&cmd, OS_QUEUE_FOREVER);
#endif /* (BLE_MGR_DIRECT_ACCESS == 1) */

        /* Block and wait for response on the response queue */
        ble_mgr_response_queue_get(rsp, OS_QUEUE_FOREVER);

        /* Release the BLE manager interface */
        ble_mgr_release();

        /* The response op code must be the same as the original command op code */
        OS_ASSERT(((ble_mgr_msg_hdr_t *) *rsp)->op_code == op_code);

        return true;
}
