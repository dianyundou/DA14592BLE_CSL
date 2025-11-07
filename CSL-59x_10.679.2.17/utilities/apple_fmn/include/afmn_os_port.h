/**
 ****************************************************************************************
 *
 * @file afmn_os_port.h
 *
 * @brief Apple FMN OS port header file
 *
 * Copyright (C) 2024-2025 Renesas Electronics Corporation and/or its affiliates.
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

#ifndef AFMN_OS_PORT_H_
#define AFMN_OS_PORT_H_

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

/**
 * \brief Timeout value for indicating blocking OS function calls
 */
#define AFMN_OS_WAIT_FOREVER                    (0xffffffff)

/**
 * \brief Notification for timers execution
 */
#define AFMN_OS_TIMER_EXECUTION_NOTIF           (1 << 30)

/**
 * \brief OS task handle
 */
typedef void *afmn_os_task_t;

/**
 * \brief OS queue handle
 */
typedef void *afmn_os_queue_t;

/**
 * \brief OS timer type
 */
typedef enum {
        AFMN_OS_TIMER_TYPE_ONESHOT,             /**< One-shot timer */
        AFMN_OS_TIMER_TYPE_PERIODIC,            /**< Periodic timer */
        AFMN_OS_TIMER_TYPE_ACCURATE_ONESHOT,    /**< Accurate one-shot timer */
        AFMN_OS_TIMER_TYPE_ACCURATE_PERIODIC    /**< Accurate periodic timer */
} AFMN_OS_TIMER_TYPE;

/**
 * \brief OS timer handle
 */
typedef void *afmn_os_timer_t;

/**
 * \brief OS timer callback function
 *
 * \param [in] timer    timer handle
 */
typedef void (*afmn_os_timer_cb_t)(afmn_os_timer_t timer);

/**
 * \brief Create OS queue
 *
 * This function creates OS queue that can contain a max number of elements of specified size.
 *
 * \param [in] elem_size        queue element size
 * \param [in] max_elems        max number of elements that queue can store
 *
 * \return handle of the queue if queue created successfully, otherwise NULL
 */
afmn_os_queue_t afmn_os_queue_create(uint32_t elem_size, uint32_t max_elems);

/**
 * \brief Delete OS queue
 *
 * This function deletes OS queue.
 *
 * \param [in] queue    queue handle
 */
void afmn_os_queue_delete(afmn_os_queue_t queue);

/**
 * \brief Put element in OS queue
 *
 * Function adds element into OS queue if there is enough room for it.
 * If there is no room in OS queue for \p timeout ticks, element is not
 * put in OS queue and error is returned.
 *
 * \param [in] queue    queue handle
 * \param [in] elem     pointer to element to enqueue
 * \param [in] timeout  max time in OS ticks to wait for space in queue
 *
 * \return 0 on success, <0 otherwise
 */
int afmn_os_queue_put(afmn_os_queue_t queue, const void * const elem, uint32_t timeout);

/**
 * \brief Get element from OS queue
 *
 * Function gets element from OS queue and removes it.
 * If there is no element in OS queue for \p timeout ticks, error is returned.
 *
 * \param [in] queue    queue handle
 * \param [out] elem    pointer to buffer for storing dequeued element
 * \param [in] timeout  max time in OS ticks to wait for element in queue
 *
 * \return 0 on success, <0 otherwise
 */
int afmn_os_queue_get(afmn_os_queue_t queue, void * const elem, uint32_t timeout);

/**
 * \brief Register current OS task
 *
 * This function is used for registering current OS task to be notified for calling
 * afmn_os_timer_execution().
 *
 * \return 0 on success, <0 otherwise
 */
int afmn_os_register_task(void);

/**
 * \brief Timer execution function
 */
void afmn_os_timer_execution(void);

/**
 * \brief Create OS timer
 *
 * This function creates a periodic or one-shot OS timer.
 *
 * \param [in] type     indicates if callback will be called once or multiple times
 * \param [in] cb       callback called when timer expires
 *
 * \return handle of the timer if timer created successfully, otherwise NULL
 */
afmn_os_timer_t afmn_os_timer_create(AFMN_OS_TIMER_TYPE type, afmn_os_timer_cb_t cb);

/**
 * \brief Start OS timer
 *
 * This function starts OS timer with given period.
 *
 * \param [in] timer    timer handle
 * \param [in] period   timer period in OS ticks
 *
 * \return 0 on success, <0 otherwise
 */
int afmn_os_timer_start(afmn_os_timer_t timer, uint32_t period);

/**
 * \brief Stop OS timer
 *
 * This function stops OS timer.
 *
 * \param [in] timer    timer handle
 *
 * \return 0 on success, <0 otherwise
 */
int afmn_os_timer_stop(afmn_os_timer_t timer);

/**
 * \brief Delete OS timer
 *
 * This function deletes a previously created OS timer.
 *
 * \param [in] timer    timer handle
 *
 * \return 0 on success, <0 otherwise
 */
int afmn_os_timer_delete(afmn_os_timer_t timer);

/**
 * \brief Check if OS timer is active
 *
 * This function checks OS timer status.
 *
 * \param [in] timer    timer handle
 *
 * \return true if timer is active, otherwise false
 */
bool afmn_os_timer_is_active(afmn_os_timer_t timer);

/**
 * \brief Get OS timer ID
 *
 * This function returns OS timer ID assigned in afmn_os_timer_create()
 *
 * \param [in] timer    timer handle
 *
 * \return timer ID
 */
uint32_t afmn_os_timer_get_timer_id(afmn_os_timer_t timer);

/**
 * \brief Convert msec to OS ticks
 *
 * \param [in] ms       milliseconds to convert to OS ticks
 *
 * \return OS ticks
 */
uint32_t afmn_os_ms_to_ticks(uint32_t ms);

/**
 * \brief Allocate memory
 *
 * \param [in] size     size of memory to allocate
 *
 * \return pointer to the allocated memory
 */
void *afmn_os_malloc(size_t size);

/**
 * \brief Free allocated memory
 *
 * \param [in] addr     address of the allocated memory
 */
void afmn_os_free(void *addr);

#endif /* AFMN_OS_PORT_H_ */
