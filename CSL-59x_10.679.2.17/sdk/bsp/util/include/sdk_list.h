/**
 ****************************************************************************************
 *
 * @file sdk_list.h
 *
 * @brief Simple helper to manage single-linked list
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

#ifndef SDK_LIST_H_
#define SDK_LIST_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/**
 * List element
 *
 * Each list element can be of any user defined type as long as its first member is pointer to
 * structure of the same type.
 *
 */
struct list_elem {
        struct list_elem *next;
};

/**
 * Callback for elements
 *
 * \param [in] elem  evaluated element
 * \param [in] ud    user data
 *
 */
typedef void (* list_elem_cb_t) (const void *elem, const void *ud);

/**
 * Callback for matching elements
 *
 * \param [in] elem  evaluated element
 * \param [in] ud    user data
 *
 * \return true if element is matches, false otherwise
 *
 */
typedef bool (* list_elem_match_t) (const void *elem, const void *ud);

/**
 * Add element to list
 *
 * Element is added to the beginning of list.
 *
 * \param [in,out] head   list head
 * \param [in]     elem   new element
 *
 */
void list_add(void **head, void *elem);

/**
 * Delete element from the end of the list
 *
 * \param [in,out] head   list head
 *
 * \return last element in list
 *
 */
void *list_pop_back(void **head);

/**
 * Peek element from the end of the list
 *
 * \param [in,out] head   list head
 *
 * \return last element in list
 *
 */
void *list_peek_back(void **head);

/**
 * Get number of elements in list
 *
 * \param [in] head   list head
 *
 * \return number of elements in list
 *
 */
uint8_t list_size(void *head);

/**
 * Append element to list
 *
 * It's recommended to use list_add() whenever possible since it works in constant time.
 *
 * \param [in,out] head   list head
 * \param [in]     elem   new element
 *
 */
void list_append(void **head, void *elem);

/**
 * Find element in list
 *
 * \param [in]     head   list head
 * \param [in]     match  callback to match element
 * \param [in]     ud     user data
 *
 * \return found element or NULL if not found
 *
 */
void *list_find(void *head, list_elem_match_t match, const void *ud);

/**
 * Unlink element from list
 *
 * \param [in,out] head   list head
 * \param [in]     match  callback to match element
 * \param [in]     ud     user data
 *
 * \return unlinked element or NULL if not found
 *
 */
void *list_unlink(void **head, list_elem_match_t match, const void *ud);

/**
 * Remove element from list
 *
 * \param [in,out] head   list head
 * \param [in]     match  callback to match element
 * \param [in]     ud     user data
 *
 */
void list_remove(void **head, list_elem_match_t match, const void *ud);

/**
 * Removes all matched elements from list
 *
 * \param [in,out] head   list head
 * \param [in]     match  callback to match element
 * \param [in]     ud     user data
 *
 */
void list_filter(void **head, list_elem_match_t match, const void *ud);

/**
 * Iterates over entire list
 *
 * \param [in]     head   list head
 * \param [in]     cb     callback to be called for each element
 * \param [in]     ud     user data
 *
 */
void list_foreach(void *head, list_elem_cb_t cb, const void *ud);

/**
 * Remove all elements from list
 *
 * \param [in,out] head   list head
 * \param [in]     cb     callback to be called for each element before removing it
 * \param [in]     ud     user data
 *
 */
void list_free(void **head, list_elem_cb_t cb, const void *ud);

#endif /* SDK_LIST_H_ */
