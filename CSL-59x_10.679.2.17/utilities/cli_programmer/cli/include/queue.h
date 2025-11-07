/**
 ****************************************************************************************
 *
 * @file queue.h
 *
 * @brief Simple helper to manage queue
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

#ifndef QUEUE_H_
#define QUEUE_H_

#include <stdbool.h>
#include <stddef.h>

struct queue_elem;

typedef struct {
        size_t size;
        struct queue_elem *head;
        struct queue_elem *tail;
} queue_t;

typedef void (* queue_foreach_func_t) (void *data, void *user_data);

typedef bool (* queue_match_func_t) (const void *data, const void *match_data);

typedef void (* queue_destroy_func_t) (void *data);

/**
 * Initialize queue structure
 *
 * \param [in] q        queue structure
 *
 */
void queue_init(queue_t *q);

/**
 * Add element on the front of queue
 *
 * \param [in] q        queue structure
 * \param [in] data     queue element
 *
 */
void queue_push_front(queue_t *q, void *data);

/**
 * Add element on the back of queue
 *
 * \param [in] q        queue structure
 * \param [in] data     queue element
 *
 */
void queue_push_back(queue_t *q, void *data);

/**
 * Remove element from the front of queue
 *
 * \param [in] q        queue structure
 *
 * \return queue element
 *
 */
void *queue_pop_front(queue_t *q);

/**
 * Get element from the front of queue
 *
 * Element is not removed from queue
 *
 * \param [in] q        queue structure
 *
 * \return queue element
 *
 */
void *queue_peek_front(const queue_t *q);

/**
 * Get element from the back of queue
 *
 * Element is not removed from queue
 *
 * \param [in] q        queue structure
 *
 * \return queue element
 *
 */
void *queue_peek_back(const queue_t *q);

/**
 * Get number of elements on queue
 *
 * \param [in] q        queue structure
 *
 * \return number of elements in queue
 *
 */
size_t queue_length(const queue_t *q);

/**
 * Execute callback for each element of queue
 *
 * \param [in] q        queue structure
 * \param [in] func     callback function
 * \param [in] user_data any pointer passed to callback function
 *
 */
void queue_foreach(const queue_t *q, queue_foreach_func_t func, void *user_data);

/**
 * Find element in queue
 *
 * First element which matches using \p func is returned. Element IS NOT removed from queue
 *
 * \param [in] q        queue structure
 * \param [in] func     callback matching function
 * \param [in] match_data any pointer passed to callback function
 *
 * \return queue element
 *
 */
void *queue_find(const queue_t *q, queue_match_func_t func, const void *match_data);

/**
 * Remove element from queue
 *
 * First element which matches using \p func is returned. Element IS removed from queue
 *
 * \param [in] q        queue structure
 * \param [in] func     callback matching function
 * \param [in] match_data any pointer passed to callback function
 *
 * \return queue element
 *
 */
void *queue_remove(queue_t *q, queue_match_func_t func, const void *match_data);

/**
 * Remove all elements from queue
 *
 * Callback is called for each element and it should be freed there. Queue is empty after this call.
 *
 * \param [in] q        queue structure
 * \param [in] func     callback function to free element
 *
 */
void queue_remove_all(queue_t *q, queue_destroy_func_t func);

/**
 * Remove all matching elements from queue
 *
 * Every element which matches using \p m_func is removed from queue.
 * \p d_func callback is called for each element and it should be freed there.
 *
 * \param [in] q        queue structure
 * \param [in] m_func   callback matching function
 * \param [in] match_data any pointer passed to callback function
 * \param [in] d_func   callback function to free element
 *
 */
void queue_filter(queue_t *q, queue_match_func_t m_func, const void *match_data,
                                                                        queue_destroy_func_t d_func);

#endif /* QUEUE_H_ */
