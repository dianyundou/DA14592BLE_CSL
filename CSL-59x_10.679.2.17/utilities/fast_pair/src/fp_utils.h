/**
 ****************************************************************************************
 *
 * @file fp_utils.h
 *
 * @brief Google Fast Pair utilities helper module header file
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

#ifndef FP_UTILS_H_
#define FP_UTILS_H_

#include <stdint.h>
#include FAST_PAIR_CONFIG_FILE
#include "fp_defaults.h"
#include "osal.h"

#ifdef AFMN_CONFIG_FILE
#include AFMN_CONFIG_FILE
#if (AFMN_OS_CORR_TIMERS == 1)
#include "afmn_os_port.h"
#define ACCURATE_OS_TIMER                       afmn_os_timer_t
#define ACCURATE_OS_TIMER_CREATE(period_ms, is_periodic, callback) \
        afmn_os_timer_create((is_periodic) ? AFMN_OS_TIMER_TYPE_ACCURATE_PERIODIC : \
                                             AFMN_OS_TIMER_TYPE_ACCURATE_ONESHOT, \
                             (callback))
#define ACCURATE_OS_TIMER_CHANGE_PERIOD(timer, period_ms) \
        afmn_os_timer_start((timer), OS_MS_2_TICKS(period_ms))
#define ACCURATE_OS_TIMER_STOP(timer)           afmn_os_timer_stop(timer)
#define ACCURATE_OS_TIMER_IS_ACTIVE(timer)      afmn_os_timer_is_active(timer)
#define ACCURATE_OS_TIMER_DELETE(timer)         afmn_os_timer_delete(timer)
#endif /* AFMN_OS_CORR_TIMERS */
#endif /* AFMN_CONFIG_FILE */

#ifndef ACCURATE_OS_TIMER
#if (FP_UTILS_CORR_OS_TIMERS == 1)
#define ACCURATE_OS_TIMER                       corr_os_timer_handle_t
#define ACCURATE_OS_TIMER_CREATE(period_ms, is_periodic, callback) \
        corr_os_timer_create((period_ms), (is_periodic), (callback))
#define ACCURATE_OS_TIMER_START(timer)          corr_os_timer_start(timer)
#define ACCURATE_OS_TIMER_CHANGE_PERIOD(timer, period_ms) \
        corr_os_timer_change_period((timer), (period_ms))
#define ACCURATE_OS_TIMER_STOP(timer)           corr_os_timer_stop(timer)
#define ACCURATE_OS_TIMER_IS_ACTIVE(timer)      corr_os_timer_is_active(timer)
#define ACCURATE_OS_TIMER_DELETE(timer)         corr_os_timer_delete(timer)
#else
#define ACCURATE_OS_TIMER                       OS_TIMER
#define ACCURATE_OS_TIMER_CREATE(period_ms, is_periodic, callback) \
        OS_TIMER_CREATE("acc_tim", OS_MS_2_TICKS(period_ms), \
                ((is_periodic) ? OS_TIMER_RELOAD : OS_TIMER_ONCE), NULL, callback)
#define ACCURATE_OS_TIMER_START(timer)          OS_TIMER_START(timer, OS_TIMER_FOREVER)
#define ACCURATE_OS_TIMER_CHANGE_PERIOD(timer, period_ms) \
        OS_TIMER_CHANGE_PERIOD((timer),         OS_MS_2_TICKS(period_ms), OS_TIMER_FOREVER)
#define ACCURATE_OS_TIMER_STOP(timer)           OS_TIMER_STOP(timer, OS_TIMER_FOREVER)
#define ACCURATE_OS_TIMER_IS_ACTIVE(timer)      OS_TIMER_IS_ACTIVE(timer)
#define ACCURATE_OS_TIMER_DELETE(timer)         OS_TIMER_DELETE(timer, OS_TIMER_FOREVER)
#endif /* FP_UTILS_CORR_OS_TIMERS */
#endif /* ACCURATE_OS_TIMER */

#if (FP_UTILS_CORR_OS_TIMERS == 1)
/**
 * \brief Handle of timer with interval correction
 */
typedef void *corr_os_timer_handle_t;

/**
 * \brief Callback of timer with interval correction
 *
 * \param [in] timer handle of the timer
 */
typedef void (*corr_os_timer_cb_t)(corr_os_timer_handle_t timer);

/**
 * \brief Create timer with interval correction
 *
 * This function creates timer the interval of which is continuously corrected based on the time
 * that has been passed, monitored in shorter time intervals.
 *
 * \param [in] period_ms timer period in milliseconds
 * \param [in] is_periodic timer is periodic
 * \param [in] cb callback called when timer expires
 *
 * \return handle of the timer if timer created successfully, otherwise NULL
 */
corr_os_timer_handle_t corr_os_timer_create(uint32_t period_ms, bool is_periodic,
        corr_os_timer_cb_t cb);

/**
 * \brief Check if timer with interval correction is active
 *
 * \param [in] timer handle of the timer
 *
 * \return OS_TIMER_TRUE if timer is active, OS_TIMER_FALSE otherwise
 */
OS_BASE_TYPE corr_os_timer_is_active(corr_os_timer_handle_t timer);

/**
 * \brief Start timer with interval correction
 *
 * \param [in] timer handle of the timer
 *
 * \return OS_TIMER_SUCCESS if command has been sent successfully
 *         OS_TIMER_FAIL if timeout has occurred
 */
OS_BASE_TYPE corr_os_timer_start(corr_os_timer_handle_t timer);

/**
 * \brief Change the period and start timer with interval correction
 *
 * \param [in] timer handle of the timer
 * \param [in] period_ms new timer's period in milliseconds
 *
 * \return OS_TIMER_SUCCESS if command has been sent successfully
 *         OS_TIMER_FAIL if timeout has occurred
 */
OS_BASE_TYPE corr_os_timer_change_period(corr_os_timer_handle_t timer, uint32_t period_ms);

/**
 * \brief Stop timer with interval correction
 *
 * \param [in] timer handle of the timer
 *
 * \return OS_TIMER_SUCCESS if command has been sent successfully
 *         OS_TIMER_FAIL if timeout has occurred
 */
OS_BASE_TYPE corr_os_timer_stop(corr_os_timer_handle_t timer);

/**
 * \brief Delete timer with interval correction
 *
 * This function deletes previously created timer.
 *
 * \param [in] timer handle of the timer
 *
 * \return OS_TIMER_SUCCESS if command has been sent successfully
 *         OS_TIMER_FAIL if timeout has occurred
 */
OS_BASE_TYPE corr_os_timer_delete(corr_os_timer_handle_t timer);

/**
 * \brief Trigger correction of timers with interval correction
 */
void corr_os_timer_trigger_correction(void);
#endif /* FP_UTILS_CORR_OS_TIMERS */

/**
 * \brief Reverse byte order in a buffer
 *
 * \param [in,out] buffer buffer to be processed
 * \param [in] size size of the buffer
 */
void reverse_byte_order(uint8_t *buffer, uint8_t size);

#endif /* FP_UTILS_H_ */
