/**
 ****************************************************************************************
 *
 * @file ble_mgr_ad_msg.c
 *
 * @brief Helper library for BLE adapter message handling in BLE Manager
 *
 * Copyright (C) 2015-2023 Renesas Electronics Corporation and/or its affiliates.
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
#include "osal.h"
#include "ble_config.h"
#include "ble_common.h"
#include "ble_mgr_helper.h"
#include "ble_mgr.h"
#include "ble_mgr_ad_msg.h"
#include "ble_mgr_gtl.h"
#include "ble_mgr_common.h"

#include "ad_ble_msg.h"
#include "gapm_task.h"

#define AD_MSG_WQUEUE_MAXLEN (5)

typedef struct {
        ad_ble_operation_t      rsp_op;
        ad_ble_operation_t      cmd_op;
        ble_ad_msg_wqueue_cb_t  cb;
        void                    *param;
} ad_msg_wqueue_element_t;

__RETAINED static struct {
        ad_msg_wqueue_element_t  queue[AD_MSG_WQUEUE_MAXLEN];
        uint8_t                  len;
} ad_msg_wqueue;

void *ble_ad_msg_alloc(ad_ble_operation_t operation, uint16_t len)
{
        ad_ble_msg_t *ad_msg = OS_MALLOC(sizeof(ad_ble_msg_t) + len);

        ad_msg->op_code   = AD_BLE_OP_CODE_ADAPTER_MSG;
        ad_msg->msg_size  = len;
        ad_msg->operation = operation;

        memset(ad_msg->param, 0, len);

        return ad_msg;
}

void ble_ad_msg_wqueue_add(ad_ble_operation_t rsp_op, ad_ble_operation_t cmd_op,
                           ble_ad_msg_wqueue_cb_t cb, void *param)
{
        ad_msg_wqueue_element_t *elem;

        /* There should be still room in the queue before calling this function */
        OS_ASSERT(ad_msg_wqueue.len < AD_MSG_WQUEUE_MAXLEN);

        elem = &ad_msg_wqueue.queue[ad_msg_wqueue.len++];

        elem->rsp_op = rsp_op;
        elem->cmd_op = cmd_op;
        elem->cb = cb;
        elem->param = param;
}

bool ble_ad_msg_waitqueue_match(ad_ble_msg_t *ad_msg)
{
        uint8_t idx;

        for (idx = 0; idx < ad_msg_wqueue.len; idx++) {
                bool match = true;
                ad_msg_wqueue_element_t *elem = &ad_msg_wqueue.queue[idx];

                match = (elem->rsp_op == ad_msg->operation);
                if (!match) {
                        continue;
                }

                switch (elem->rsp_op) {
                case AD_BLE_OP_CMP_EVT:
                {
                        ad_ble_cmp_evt_t *evt = (void *) ad_msg->param;
                        match = (evt->op_req == elem->cmd_op);
                        break;
                }
                /* Add more events if other commands need more fine-grained matching */
                default:
                        break;
                }

                if (match) {
                        ble_ad_msg_wqueue_cb_t cb = elem->cb;
                        void *param = elem->param;

                        /* Remove from queue by moving remaining elements up in queue */
                        ad_msg_wqueue.len--;
                        memmove(elem, elem + 1, sizeof(ad_msg_wqueue_element_t) * (ad_msg_wqueue.len - idx));

                        /* Fire associated callback */
                        cb(ad_msg, param);

                        return true;
                }
        }

        return false;
}
