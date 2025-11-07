/**
 ****************************************************************************************
 *
 * @file msg_queues.c
 *
 * @brief Message queue API
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

#ifdef OS_PRESENT

#include <stdbool.h>
#include <string.h>
#include <sdk_defs.h>
#include <msg_queues.h>
#include <interrupts.h>


#if CONFIG_MSG_QUEUE_USE_ALLOCATORS
const content_allocator default_os_allocator = {
        .content_alloc = (MSG_ALLOC) MSG_QUEUE_MALLOC,
        .content_free = (MSG_FREE) MSG_QUEUE_FREE
};

#define QUEUE_ALLOC(queue, size) queue->allocator->content_alloc(size)
#define QUEUE_DEALLOCATOR(queue) queue->allocator->content_free

#else

#define QUEUE_ALLOC(queue, size) MSG_QUEUE_MALLOC(size)
#define QUEUE_DEALLOCATOR(queue) MSG_QUEUE_FREE

#endif

void msg_queue_create(msg_queue *queue, int queue_size, content_allocator *allocator)
{
        OS_QUEUE_CREATE(queue->queue, sizeof(msg), queue_size);
#if CONFIG_MSG_QUEUE_USE_ALLOCATORS
        queue->allocator = allocator;
#endif
}

void msg_queue_delete(msg_queue *queue)
{
        OS_QUEUE_DELETE(queue->queue);
}

int msg_queue_put(msg_queue *queue, msg *msg, OS_TICK_TIME timeout)
{
        OS_BASE_TYPE ret = OS_QUEUE_FULL;

        if (in_interrupt()) {
                /* cppcheck-suppress unreadVariable */
                ret = OS_QUEUE_PUT_FROM_ISR(queue->queue, msg);
        } else {
                ret = OS_QUEUE_PUT(queue->queue, msg, timeout);
        }

        return ret;
}

int msg_queue_get(msg_queue *queue, msg *msg, OS_TICK_TIME timeout)
{
        OS_BASE_TYPE ret = OS_QUEUE_EMPTY;

        if (in_interrupt()) {
                /* cppcheck-suppress unreadVariable */
                ret = OS_QUEUE_GET_FROM_ISR(queue->queue, msg);
        } else {
                ret = OS_QUEUE_GET(queue->queue, msg, timeout);
        }

        return ret;
}

void msg_init(msg *msg, MSG_ID id, MSG_TYPE type, void *buf, MSG_SIZE size, MSG_FREE free_cb)
{
        msg->id = id;
        msg->type = type;
        msg->data = buf;
        msg->size = size;
        msg->free_cb = free_cb;
}

void msg_release(msg *msg)
{
        if (msg->free_cb) {
                msg->free_cb(msg->data);
                msg->free_cb = NULL;
        }
}

int msg_queue_init_msg(msg_queue *queue, msg *msg, MSG_ID id, MSG_TYPE type, MSG_SIZE size)
{
        uint8_t *buf;

        buf = QUEUE_ALLOC(queue, size);
        if (buf == NULL) {
                return 0;
        }
        msg_init(msg, id, type, buf, size, QUEUE_DEALLOCATOR(queue));

        return 1;
}

int msg_queue_send(msg_queue *queue, MSG_ID id, MSG_TYPE type, void *buf, MSG_SIZE size,
                                                                        OS_TICK_TIME timeout)
{
        msg msg;

        if (msg_queue_init_msg(queue, &msg, id, type, size) == 0) {
                return OS_QUEUE_FULL;
        }

        memcpy(msg.data, buf, size);
        if (msg_queue_put(queue, &msg, timeout) == OS_QUEUE_FULL) {
                msg_release(&msg);
                return OS_QUEUE_FULL;
        }

        return OS_QUEUE_OK;
}

int msq_queue_send_zero_copy(msg_queue *queue, MSG_ID id, MSG_TYPE type, void *buf,
                                        MSG_SIZE size, OS_TICK_TIME timeout, MSG_FREE free_cb)
{
        msg msg;

        msg_init(&msg, id, type, buf, size, free_cb);

        if (msg_queue_put(queue, &msg, timeout) == OS_QUEUE_FULL) {
                msg_release(&msg);
                return OS_QUEUE_FULL;
        }

        return OS_QUEUE_OK;
}

#endif /* OS_PRESENT */
