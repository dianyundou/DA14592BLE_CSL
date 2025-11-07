/**
 ****************************************************************************************
 *
 * @file fp_utils.c
 *
 * @brief Google Fast Pair utilities helper module implementation
 *
 * Copyright (C) 2024 Renesas Electronics Corporation and/or its affiliates.
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

#include <stdio.h>
#include <stdbool.h>
#include "osal.h"
#include "sys_timer.h"
#include "fp_core.h"
#include "fp_notifications.h"
#include "fp_utils.h"

#if (FP_UTILS_CORR_OS_TIMERS == 1)
/* Maximum number of timers with interval correction that can be handled */
#ifndef FP_UTILS_CORR_OS_TIMERS_MAX_COUNT
#define FP_UTILS_CORR_OS_TIMERS_MAX_COUNT       (4)
#endif

/* Minimum interval of timer with interval correction that requires correction (in milliseconds) */
#ifndef FP_UTILS_CORR_OS_TIMER_MIN_INTV_MS
#define FP_UTILS_CORR_OS_TIMER_MIN_INTV_MS      (1000)  /* 1 second */
#endif

/* Timer with interval correction command types */
typedef enum {
        CORR_OS_TIMER_CMD_START,
        CORR_OS_TIMER_CMD_CHANGE_PERIOD,
        CORR_OS_TIMER_CMD_STOP,
        CORR_OS_TIMER_CMD_DELETE
} CORR_OS_TIMER_CMD;

/* OS timer used for triggering periodically the correction of timers with interval correction */
__RETAINED static OS_TIMER correction_timer;

/* OS timers the interval of which is corrected */
__RETAINED static struct corr_os_timer_t {
        corr_os_timer_cb_t cb;
        uint32_t start_time_ms;
        uint32_t period_ms;
        bool is_active;
        bool is_periodic;
} corr_os_timers[FP_UTILS_CORR_OS_TIMERS_MAX_COUNT];

/* Returns pointer to free timer instance entry */
static struct corr_os_timer_t *search_for_free_corr_timer(void)
{
        for (int i = 0; i < FP_UTILS_CORR_OS_TIMERS_MAX_COUNT; i++) {
                if (corr_os_timers[i].cb == NULL) {
                        return &corr_os_timers[i];
                }
        }

        return NULL;
}

/* Correction timer callback */
static void correction_timer_cb(OS_TIMER timer)
{
        fp_send_notification(CORR_OS_TIMER_TRIGGER_NOTIF);
}

/* Updates timer with interval correction based on the current status of monitored timers */
static void update_correction_timer(void)
{
        uint32_t now_ms = sys_timer_get_uptime_usec() / 1000;
        uint32_t next_tmr_trigger_in_x_ms = (uint32_t) -1;

        for (int i = 0; i < FP_UTILS_CORR_OS_TIMERS_MAX_COUNT; i++) {
                struct corr_os_timer_t * corr_tmr = &corr_os_timers[i];
                if (corr_os_timer_is_active((corr_os_timer_handle_t) corr_tmr)) {
                        uint32_t elapsed = now_ms - corr_tmr->start_time_ms;
                        uint32_t remaining = (corr_tmr->period_ms > elapsed) ?
                                             (corr_tmr->period_ms - elapsed) : 0;

                        if (remaining < OS_TICK_PERIOD_MS) {
                                uint32_t start_time_ms = corr_tmr->start_time_ms;
                                corr_tmr->is_active = corr_tmr->is_periodic;
                                corr_tmr->cb(corr_tmr);
                                if (corr_tmr->is_active && corr_tmr->is_periodic) {
                                        if (start_time_ms == corr_tmr->start_time_ms) {
                                                corr_tmr->start_time_ms += corr_tmr->period_ms;
                                        }
                                }
                                fp_send_notification(CORR_OS_TIMER_TRIGGER_NOTIF);
                                return;
                        }

                        if (remaining < next_tmr_trigger_in_x_ms) {
                                next_tmr_trigger_in_x_ms = remaining;
                        }
                }
        }

        if (next_tmr_trigger_in_x_ms != (uint32_t) -1) {
                OS_TICK_TIME corr_tmr_ticks;

                /*
                 * Set the interval of the correction timer to half the remaining interval to
                 * deal with clock frequency change.
                 */
                if (next_tmr_trigger_in_x_ms > FP_UTILS_CORR_OS_TIMER_MIN_INTV_MS / 2) {
                        next_tmr_trigger_in_x_ms = next_tmr_trigger_in_x_ms / 2;
                }

                corr_tmr_ticks = OS_MS_2_TICKS(next_tmr_trigger_in_x_ms);

                if (corr_tmr_ticks == 0) {
                        fp_send_notification(CORR_OS_TIMER_TRIGGER_NOTIF);
                        return;
                }

                OS_TIMER_CHANGE_PERIOD(correction_timer, corr_tmr_ticks, OS_TIMER_FOREVER);
        }
}

/* Updates the state of timer with interval correction */
static OS_BASE_TYPE update_timer(CORR_OS_TIMER_CMD cmd, corr_os_timer_handle_t timer,
        uint32_t period_ms)
{
        struct corr_os_timer_t *corr_tmr = (struct corr_os_timer_t *) timer;

        if (corr_tmr->cb == NULL) {
                return OS_TIMER_FAIL;
        }

        switch (cmd) {
        case CORR_OS_TIMER_CMD_START:
                period_ms = corr_tmr->period_ms;
                /* no break */
        case CORR_OS_TIMER_CMD_CHANGE_PERIOD:
                corr_tmr->start_time_ms = sys_timer_get_uptime_usec() / 1000;
                corr_tmr->period_ms = period_ms;
                corr_tmr->is_active = true;
                break;
        case CORR_OS_TIMER_CMD_STOP:
                corr_tmr->is_active = false;
                break;
        case CORR_OS_TIMER_CMD_DELETE:
                corr_tmr->cb = NULL;
                break;
        }

        update_correction_timer();

        return OS_TIMER_SUCCESS;
}

/* Creates timer with interval correction */
corr_os_timer_handle_t corr_os_timer_create(uint32_t period_ms, bool is_periodic,
        corr_os_timer_cb_t cb)
{
        struct corr_os_timer_t *corr_tmr = search_for_free_corr_timer();

        if (corr_tmr == NULL) {
                return NULL;
        }

        corr_tmr->is_active = false;
        corr_tmr->is_periodic = is_periodic;
        corr_tmr->period_ms = period_ms;
        corr_tmr->cb = cb;

        if (correction_timer == NULL) {
                correction_timer =
                        OS_TIMER_CREATE("corr_tim", OS_MS_2_TICKS(FP_UTILS_CORR_OS_TIMER_MIN_INTV_MS),
                                OS_TIMER_ONCE, NULL, correction_timer_cb);
                OS_ASSERT(correction_timer);
        }

        return corr_tmr;
}

OS_BASE_TYPE corr_os_timer_is_active(corr_os_timer_handle_t timer)
{
        struct corr_os_timer_t *corr_tmr = (struct corr_os_timer_t *) timer;

        return (corr_tmr->cb > 0 && corr_tmr->is_active) ? OS_TRUE : OS_FALSE;
}

OS_BASE_TYPE corr_os_timer_start(corr_os_timer_handle_t timer)
{
        return update_timer(CORR_OS_TIMER_CMD_START, timer, 0 /* don't care */);
}

OS_BASE_TYPE corr_os_timer_change_period(corr_os_timer_handle_t timer, uint32_t period_ms)
{
        return update_timer(CORR_OS_TIMER_CMD_CHANGE_PERIOD, timer, period_ms);
}

OS_BASE_TYPE corr_os_timer_stop(corr_os_timer_handle_t timer)
{
        return update_timer(CORR_OS_TIMER_CMD_STOP, timer, 0 /* don't care */);
}

OS_BASE_TYPE corr_os_timer_delete(corr_os_timer_handle_t timer)
{
        return update_timer(CORR_OS_TIMER_CMD_DELETE, timer, 0 /* don't care */);
}

void corr_os_timer_trigger_correction(void)
{
        update_correction_timer();
}
#endif /* FP_UTILS_CORR_OS_TIMERS */

void reverse_byte_order(uint8_t *buffer, uint8_t size)
{
        uint8_t i;
        uint8_t temp;
        uint8_t last = size - 1;

        for (i = 0; i < (size / 2); i++) {
                temp = buffer[i];
                buffer[i] = buffer[last - i];
                buffer[last - i] = temp;
        }
}
