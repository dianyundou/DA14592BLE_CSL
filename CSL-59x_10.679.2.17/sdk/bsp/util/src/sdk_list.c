/**
 ****************************************************************************************
 *
 * @file sdk_list.c
 *
 * @brief Simple helper to manage single-linked list
 *
 * Copyright (C) 2015-2020 Renesas Electronics Corporation and/or its affiliates.
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
#include "sdk_list.h"
/*
 * osal.h is included using a relative path because sdk_list.{h c} is also used by nortos
 * applications which do not contain this path in their project settings.
 */
#include "../../../middleware/osal/osal.h"

void list_add(void **head, void *elem)
{
        struct list_elem *e = elem;

        e->next = *head;
        *head = e;
}

void *list_pop_back(void **head)
{
        struct list_elem *e = *head;
        struct list_elem *p = NULL;

        if (e) {
                while (e->next) {
                        p = e;
                        e = e->next;
                }

                if (p) {
                        p->next = NULL;
                } else {
                        *head = NULL;
                }
        }

        return e;
}

void *list_peek_back(void **head)
{
        struct list_elem *e = *head;

        if (e) {
                while (e->next) {
                        e = e->next;
                }
        }

        return e;
}

uint8_t list_size(void *head)
{
        uint8_t n = 0;
        struct list_elem *e = head;

        while (e) {
                ++n;
                e = e->next;
        }

        return n;
}

void list_append(void **head, void *elem)
{
        struct list_elem *e = *head;

        if (!e) {
                list_add(head, elem);
                return;
        }

        while (e->next) {
                e = e->next;
        }

        e->next = elem;

        e = elem;
        e->next = NULL;
}

void *list_find(void *head, list_elem_match_t match, const void *ud)
{
        struct list_elem *e = head;

        while (e && !match(e, ud)) {
                e = e->next;
        }

        return e;
}

void *list_unlink(void **head, list_elem_match_t match, const void *ud)
{
        struct list_elem *e = *head;
        struct list_elem *p = NULL;

        while (e && !match(e, ud)) {
                p = e;
                e = e->next;
        }

        if (e) {
                if (p) {
                        p->next = e->next;
                } else {
                        *head = e->next;
                }
        }

        return e;
}

void list_remove(void **head, list_elem_match_t match, const void *ud)
{
        void *e = list_unlink(head, match, ud);

        if (e) {
                OS_FREE(e);
        }
}

void list_filter(void **head, list_elem_match_t match, const void *ud)
{
        struct list_elem *e = *head;
        struct list_elem *p = NULL;

        while (e) {
                if (match(e, ud)) {
                        if (p) {
                                p->next = e->next;
                        } else {
                                *head = e->next;
                        }

                        // p does not change!
                        e = e->next;

                        OS_FREE(e);
                } else {
                        p = e;
                        e = e->next;
                }
        }
}

void list_foreach(void *head, list_elem_cb_t cb, const void *ud)
{
        struct list_elem *e = head;

        while (e) {
                cb(e, ud);
                e = e->next;
        }
}

void list_free(void **head, list_elem_cb_t cb, const void *ud)
{
        while (*head) {
                struct list_elem *e = *head;
                *head = e->next;

                if (cb) {
                        cb(e, ud);
                }

                OS_FREE(e);
        }
}
