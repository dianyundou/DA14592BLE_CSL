/**
 ****************************************************************************************
 *
 * @file dgtl_msg.c
 *
 * @brief DGTL message
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

#include <stddef.h>
#include <string.h>
#include <osal.h>
#include "dgtl.h"
#include "dgtl_msg.h"
#include "dgtl_pkt.h"
#include "ble_mgr_common.h"

__STATIC_INLINE size_t get_ext_len(uint8_t pkt_type)
{
        if ((pkt_type >= DGTL_PKT_TYPE_HCI_CMD) && (pkt_type <= DGTL_PKT_TYPE_GTL)) {
                /*
                 * For HCI and GTL packets we return the offset of packet contents from the start
                 * of stack message structure. We also need to include the fact that packet contents
                 * there *does not* include packet type indicator.
                 */
                return offsetof(ble_mgr_common_stack_msg_t, msg) - 1;

        }

        return 0;
}

__STATIC_INLINE void *msg2ptr(dgtl_msg_t *msg)
{
        /* Need to remove const from pkt_type pointer! */
        return (void *) (&msg->pkt_type - get_ext_len(msg->pkt_type));
}

__STATIC_INLINE dgtl_msg_t *ptr2msg(void *ptr, uint8_t pkt_type)
{
        dgtl_msg_t *msg;

        msg = ptr + get_ext_len(pkt_type);

        /* If this does not match then cast is invalid */
        if (msg->pkt_type != pkt_type) {
                OS_ASSERT(0);
                return NULL;
        }

        return msg;
}

__STATIC_INLINE size_t get_pkt_header_length(const dgtl_msg_t *msg)
{
        return dgtl_pkt_get_header_length((const dgtl_pkt_t *) msg);
}

__STATIC_INLINE size_t get_pkt_param_length(const dgtl_msg_t *msg)
{
        return dgtl_pkt_get_param_length((const dgtl_pkt_t *) msg);
}

dgtl_msg_t *dgtl_msg_alloc(uint8_t pkt_type, size_t length)
{
        uint8_t *buf;
        size_t ext_len;

        ext_len = get_ext_len(pkt_type);

        buf = OS_MALLOC(length + ext_len);
        buf[ext_len] = pkt_type;

        return ptr2msg(buf, pkt_type);
}

void dgtl_msg_free(dgtl_msg_t *msg)
{
        void *ptr = msg2ptr(msg);
        OS_FREE(ptr);
}

void *dgtl_msg_get_param_ptr(dgtl_msg_t *msg, size_t *length)
{
        size_t header_len;

        header_len = get_pkt_header_length(msg);

        /* Unknown packet, we don't know where parameters start */
        if (header_len == 0) {
                return NULL;
        }

        if (length) {
                *length = get_pkt_param_length(msg);
        }

        return &msg->data[header_len];
}

void *dgtl_msg_get_ext_ptr(dgtl_msg_t *msg, size_t *length)
{
        if (length) {
                *length = get_ext_len(msg->pkt_type);
        }

        return msg2ptr(msg);
}

dgtl_msg_t *dgtl_msg_prepare_hci_cmd(dgtl_msg_t *msg, uint16_t opcode, uint8_t param_len, void *param)
{
        dgtl_pkt_hci_cmd_t *pkt;

        if (!msg) {
                msg = dgtl_msg_alloc(DGTL_PKT_TYPE_HCI_CMD, sizeof(dgtl_pkt_hci_cmd_t) + param_len);
                if (!msg) {
                        return NULL;
                }
        }

        OS_ASSERT(msg->pkt_type == DGTL_PKT_TYPE_HCI_CMD);

        pkt = (dgtl_pkt_hci_cmd_t *) msg;
        pkt->opcode = opcode;
        pkt->length = param_len;

        if (param) {
                memcpy(pkt->parameters, param, param_len);
        }

        return msg;
}

dgtl_msg_t *dgtl_msg_prepare_hci_evt(dgtl_msg_t *msg, uint8_t code, uint8_t param_len, void *param)
{
        dgtl_pkt_hci_evt_t *pkt;

        if (!msg) {
                msg = dgtl_msg_alloc(DGTL_PKT_TYPE_HCI_EVT, sizeof(dgtl_pkt_hci_evt_t) + param_len);
                if (!msg) {
                        return NULL;
                }
        }

        OS_ASSERT(msg->pkt_type == DGTL_PKT_TYPE_HCI_EVT);

        pkt = (dgtl_pkt_hci_evt_t *) msg;
        pkt->code = code;
        pkt->length = param_len;

        if (param) {
                memcpy(pkt->parameters, param, param_len);
        }

        return msg;
}

void *dgtl_msg_to_raw_ptr(dgtl_msg_t *msg)
{
        return (void *) msg - get_ext_len(msg->pkt_type);

}

dgtl_msg_t *dgtl_msg_from_raw_ptr(void *ptr, uint8_t pkt_type)
{
        dgtl_msg_t *msg = ptr + get_ext_len(pkt_type);

        /* Make sure packet type is set properly - app would need to do this anyway */
        msg->data[0] = pkt_type;

        return msg;
}
