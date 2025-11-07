/**
 ****************************************************************************************
 *
 * @file sdk_queue.c
 *
 * @brief Simple helper to manage queue
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

#include <stdbool.h>
#include <stddef.h>
#include "sdk_queue.h"

struct queue_elem {
        struct queue_elem *next;
};

void queue_init(queue_t *q)
{
        q->size = 0;
        q->head = NULL;
        q->tail = NULL;
}

void queue_push_front(queue_t *q, void *data)
{
        struct queue_elem *el = data;

        el->next = q->head;
        q->head = el;

        if (!q->tail) {
                q->tail = el;
        }

        q->size++;
}

void queue_push_back(queue_t *q, void *data)
{
        struct queue_elem *el = data;

        el->next = NULL;

        if (q->tail) {
                q->tail->next = el;
        }
        q->tail = el;

        if (!q->head) {
                q->head = el;
        }

        q->size++;
}


void *queue_pop_front(queue_t *q)
{
        struct queue_elem *el = q->head;

        if (!el) {
                return NULL;
        }

        q->head = el->next;

        if (el == q->tail) {
                q->tail = NULL;
        }

        q->size--;

        return el;
}

void *queue_peek_front(const queue_t *q)
{
        return q->head;
}

void *queue_peek_back(const queue_t *q)
{
        return q->tail;
}

size_t queue_length(const queue_t *q)
{
        return q->size;
}

void queue_foreach(const queue_t *q, queue_foreach_func_t func, void *user_data)
{
        struct queue_elem *el = q->head;

        while (el) {
                func(el, user_data);

                el = el->next;
        }
}

void *queue_find(const queue_t *q, queue_match_func_t func, const void *match_data)
{
        struct queue_elem *el = q->head;

        while (el) {
                if (func(el, match_data)) {
                        return el;
                }

                el = el->next;
        }

        return NULL;
}

static void remove_element(queue_t *q, struct queue_elem *prev_el, struct queue_elem *el)
{
        if (!el) {
                return;
        }

        if (prev_el) {
                prev_el->next = el->next;
        } else {
                // no prev_el means we matched at head
                q->head = el->next;
        }

        if (el == q->tail) {
                // update tail if matched at tail
                q->tail = prev_el;
        }

        q->size--;
}

void *queue_remove(queue_t *q, queue_match_func_t func, const void *match_data)
{
        struct queue_elem *prev_el = NULL;
        struct queue_elem *el = q->head;

        while (el) {
                if (func(el, match_data)) {
                        break;
                }

                prev_el = el;
                el = el->next;
        }

        remove_element(q, prev_el, el);

        return el;
}

void queue_remove_all(queue_t *q, queue_destroy_func_t func)
{
        struct queue_elem *el = q->head;

        while (el) {
                q->head = el->next;

                func(el);

                el = q->head;
        }

        queue_init(q);
}

void queue_filter(queue_t *q, queue_match_func_t m_func, const void *match_data,
                                                                        queue_destroy_func_t d_func)
{
        struct queue_elem *prev_el = NULL;
        struct queue_elem *el = q->head;

        while (el) {
                struct queue_elem *next_el = el->next;

                if (m_func(el, match_data)) {
                        remove_element(q, prev_el, el);

                        if (d_func) {
                                d_func(el);
                        }

                        // prev_el stays the same!
                } else {
                        prev_el = el;
                }

                el = next_el;
        }
}
