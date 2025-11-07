/**
 ****************************************************************************************
 *
 * @file afmn_os_port.c
 *
 * @brief Apple FMN OS port implementation
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

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include AFMN_CONFIG_FILE
#include "afmn_defaults.h"
#include "sdk_defs.h"
#include "osal.h"
#include "sys_timer.h"
#include "afmn_os_port.h"

/* Timer notifications */
#define AFMN_OS_TIMER_NOTIF                     (1 << 0)
#define AFMN_OS_CORR_TIMER_NOTIF                (1 << 1)

#if (AFMN_OS_CORR_TIMERS == 1)
#ifdef FAST_PAIR_CONFIG_FILE
/* Number of accurate timers required by Apple FMN and Google Fast Pair framework */
#define AFMN_OS_TIMER_ACCURATE_COUNT_MAX        (5 + 4)
#else
/* Number of accurate timers required by Apple FMN */
#define AFMN_OS_TIMER_ACCURATE_COUNT_MAX        (5)
#endif /* FAST_PAIR_CONFIG_FILE */

/* Maximum number of timers with interval correction that can be handled */
#ifndef AFMN_OS_CORR_TIMER_COUNT_MAX
#define AFMN_OS_CORR_TIMER_COUNT_MAX            (AFMN_OS_TIMER_ACCURATE_COUNT_MAX)
#endif

/* Minimum interval of timer with interval correction that requires correction (in milliseconds) */
#ifndef AFMN_OS_CORR_TIMER_MIN_INTV_MS
#define AFMN_OS_CORR_TIMER_MIN_INTV_MS          (1000)  /* 1 second */
#endif

/* Timer with interval correction command types */
typedef enum {
        CORR_TIMER_CMD_CHANGE_PERIOD,
        CORR_TIMER_CMD_STOP,
        CORR_TIMER_CMD_DELETE
} CORR_TIMER_CMD;

/* Handle of timer with interval correction */
typedef afmn_os_timer_t corr_timer_handle_t;

/* Callback of timer with interval correction */
typedef void (*corr_timer_cb_t)(corr_timer_handle_t timer);
#endif /* AFMN_OS_CORR_TIMERS */

#if (AFMN_OS_CORR_TIMERS == 1)
#ifdef FAST_PAIR_CONFIG_FILE
/* Number of common OS timers required by Apple FMN and Google Fast Pair framework */
#define AFMN_OS_TIMER_COUNT_MAX                 (14 + 4 - AFMN_OS_TIMER_ACCURATE_COUNT_MAX)
#else
/* Number of common OS timers required by Apple FMN */
#define AFMN_OS_TIMER_COUNT_MAX                 (14 - AFMN_OS_TIMER_ACCURATE_COUNT_MAX)
#endif /* FAST_PAIR_CONFIG_FILE */
#else
#define AFMN_OS_TIMER_COUNT_MAX                 (14)
#endif /* AFMN_OS_CORR_TIMERS */

/* Callbacks of timers created for Apple FMN */
__RETAINED static struct {
        afmn_os_timer_cb_t afmn_os_timer_cbs[AFMN_OS_TIMER_COUNT_MAX];
        bool afmn_os_timer_expired[AFMN_OS_TIMER_COUNT_MAX];
        uint8_t notif;
        uint32_t timer_count;
        OS_TASK task_hdl;
} os_port_ctx;

#if (AFMN_OS_CORR_TIMERS == 1)
/* OS timer used for triggering periodically the correction of timers with interval correction */
__RETAINED static OS_TIMER correction_timer;

/* OS timers the interval of which is corrected */
__RETAINED static struct corr_timer_t {
        corr_timer_cb_t cb;
        uint32_t start_time_ms;
        uint32_t period_ms;
        uint32_t timer_id;
        bool is_active;
        bool is_periodic;
} corr_timers[AFMN_OS_CORR_TIMER_COUNT_MAX];
#endif /* AFMN_OS_CORR_TIMERS */

/* Notifies accessory OS task */
static void send_notification(uint8_t notif)
{
        OS_ENTER_CRITICAL_SECTION();
        os_port_ctx.notif |= notif;
        OS_LEAVE_CRITICAL_SECTION();

        OS_TASK_NOTIFY(os_port_ctx.task_hdl, AFMN_OS_TIMER_EXECUTION_NOTIF, OS_NOTIFY_SET_BITS);
}

/* OS timer callback */
static void os_timer_cb(OS_TIMER timer)
{
        uint32_t timer_nr = (uint32_t) OS_TIMER_GET_TIMER_ID(timer);
        os_port_ctx.afmn_os_timer_expired[timer_nr] = true;
        send_notification(AFMN_OS_TIMER_NOTIF);
}

#if (AFMN_OS_CORR_TIMERS == 1)
static OS_BASE_TYPE corr_timer_is_active(corr_timer_handle_t timer);

/* Returns pointer to free timer instance entry */
static struct corr_timer_t *search_for_free_corr_timer(void)
{
        for (int i = 0; i < AFMN_OS_CORR_TIMER_COUNT_MAX; i++) {
                if (corr_timers[i].cb == NULL) {
                        return &corr_timers[i];
                }
        }

        return NULL;
}

/* Correction timer callback */
static void correction_timer_cb(OS_TIMER timer)
{
        send_notification(AFMN_OS_CORR_TIMER_NOTIF);
}

/* Updates timer with interval correction based on the current status of monitored timers */
static void update_correction_timer(void)
{
        uint32_t now_ms = sys_timer_get_uptime_usec() / 1000;
        uint32_t next_tmr_trigger_in_x_ms = (uint32_t) -1;

        for (int i = 0; i < AFMN_OS_CORR_TIMER_COUNT_MAX; i++) {
                struct corr_timer_t * corr_tmr = &corr_timers[i];
                if (corr_timer_is_active((corr_timer_handle_t) corr_tmr)) {
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
                                send_notification(AFMN_OS_CORR_TIMER_NOTIF);
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
                if (next_tmr_trigger_in_x_ms > AFMN_OS_CORR_TIMER_MIN_INTV_MS / 2) {
                        next_tmr_trigger_in_x_ms = next_tmr_trigger_in_x_ms / 2;
                }

                corr_tmr_ticks = OS_MS_2_TICKS(next_tmr_trigger_in_x_ms);

                if (corr_tmr_ticks == 0) {
                        send_notification(AFMN_OS_CORR_TIMER_NOTIF);
                        return;
                }

                OS_TIMER_CHANGE_PERIOD(correction_timer, corr_tmr_ticks, OS_TIMER_FOREVER);
        }
}

/* Updates the state of timer with interval correction */
static OS_BASE_TYPE update_timer(CORR_TIMER_CMD cmd, corr_timer_handle_t timer, uint32_t period_ms)
{
        struct corr_timer_t *corr_tmr = (struct corr_timer_t *) timer;

        if (corr_tmr->cb == NULL) {
                return OS_TIMER_FAIL;
        }

        switch (cmd) {
        case CORR_TIMER_CMD_CHANGE_PERIOD:
                corr_tmr->start_time_ms = sys_timer_get_uptime_usec() / 1000;
                corr_tmr->period_ms = period_ms;
                corr_tmr->is_active = true;
                break;
        case CORR_TIMER_CMD_STOP:
                corr_tmr->is_active = false;
                break;
        case CORR_TIMER_CMD_DELETE:
                corr_tmr->cb = NULL;
                break;
        }

        update_correction_timer();

        return OS_TIMER_SUCCESS;
}

/* Creates timer with interval correction */
static corr_timer_handle_t corr_timer_create(bool is_periodic, corr_timer_cb_t cb)
{
        struct corr_timer_t *corr_tmr = search_for_free_corr_timer();

        if (corr_tmr == NULL) {
                return NULL;
        }

        corr_tmr->is_active = false;
        corr_tmr->is_periodic = is_periodic;
        corr_tmr->period_ms = AFMN_OS_CORR_TIMER_MIN_INTV_MS;
        corr_tmr->timer_id = AFMN_OS_TIMER_COUNT_MAX + (corr_tmr - &corr_timers[0]);
        corr_tmr->cb = cb;

        if (correction_timer == NULL) {
                correction_timer =
                        OS_TIMER_CREATE("corr_tim", OS_MS_2_TICKS(AFMN_OS_CORR_TIMER_MIN_INTV_MS),
                                OS_TIMER_ONCE, NULL, correction_timer_cb);
                OS_ASSERT(correction_timer);
        }

        return corr_tmr;
}

/* Checks if timer with interval correction is active */
static OS_BASE_TYPE corr_timer_is_active(corr_timer_handle_t timer)
{
        struct corr_timer_t *corr_tmr = (struct corr_timer_t *) timer;

        return (corr_tmr->cb > 0 && corr_tmr->is_active) ? OS_TRUE : OS_FALSE;
}

/* Gets timer ID of timer with interval correction */
static uint32_t corr_timer_get_timer_id(corr_timer_handle_t timer)
{
        struct corr_timer_t *corr_tmr = (struct corr_timer_t *) timer;

        return (corr_tmr->cb > 0) ? corr_tmr->timer_id : -1;
}

/* Changes the period and starts timer with interval correction */
static OS_BASE_TYPE corr_timer_change_period(corr_timer_handle_t timer, uint32_t period_ms)
{
        return update_timer(CORR_TIMER_CMD_CHANGE_PERIOD, timer, period_ms);
}

/* Stops timer with interval correction */
static OS_BASE_TYPE corr_timer_stop(corr_timer_handle_t timer)
{
        return update_timer(CORR_TIMER_CMD_STOP, timer, 0 /* don't care */);
}

/* Deletes timer with interval correction */
static OS_BASE_TYPE corr_timer_delete(corr_timer_handle_t timer)
{
        return update_timer(CORR_TIMER_CMD_DELETE, timer, 0 /* don't care */);
}

/* Triggers correction of timers with interval correction */
static void corr_timer_trigger_correction(void)
{
        update_correction_timer();
}

/* Checks if timer is with interval correction or not */
static bool timer_is_corr_timer(afmn_os_timer_t timer)
{
        return (timer >= (void *) &corr_timers[0] &&
                timer < (void *) &corr_timers[AFMN_OS_CORR_TIMER_COUNT_MAX]);
}
#endif /* AFMN_OS_CORR_TIMERS */

afmn_os_queue_t afmn_os_queue_create(uint32_t elem_size, uint32_t max_elems)
{
        afmn_os_queue_t queue;

        OS_QUEUE_CREATE(queue, elem_size, max_elems);

        return queue;
}

void afmn_os_queue_delete(afmn_os_queue_t queue)
{
        OS_QUEUE_DELETE(queue);
}

int afmn_os_queue_put(afmn_os_queue_t queue, const void * const elem, uint32_t timeout)
{
        return !(OS_QUEUE_PUT(queue, elem, timeout) == OS_QUEUE_OK);
}

int afmn_os_queue_get(afmn_os_queue_t queue, void * const elem, uint32_t timeout)
{
        return !(OS_QUEUE_GET(queue, elem, timeout) == OS_QUEUE_OK);
}

int afmn_os_register_task(void)
{
        os_port_ctx.task_hdl = OS_GET_CURRENT_TASK();

        return 0;
}

/* Returns id of free timer instance entry */
static uint32_t search_for_free_timer_id(void)
{
        for (uint32_t i = 0; i < AFMN_OS_TIMER_COUNT_MAX; i++) {
                if (os_port_ctx.afmn_os_timer_cbs[i] == NULL) {
                        return i;
                }
        }

        return AFMN_OS_TIMER_COUNT_MAX;
}

void afmn_os_timer_execution(void)
{
        uint8_t notif;

        OS_ENTER_CRITICAL_SECTION();
        notif = os_port_ctx.notif;
        os_port_ctx.notif = 0;
        OS_LEAVE_CRITICAL_SECTION();

        if (notif & AFMN_OS_TIMER_NOTIF) {
                for (uint32_t i = 0; i < AFMN_OS_TIMER_COUNT_MAX; i++) {
                        if (os_port_ctx.afmn_os_timer_expired[i]) {
                                os_port_ctx.afmn_os_timer_expired[i] = false;
                                if (os_port_ctx.afmn_os_timer_cbs[i]) {
                                        os_port_ctx.afmn_os_timer_cbs[i]((void *) i);
                                }
                        }
                }
        }

#if (AFMN_OS_CORR_TIMERS == 1)
        if (notif & AFMN_OS_CORR_TIMER_NOTIF) {
                corr_timer_trigger_correction();
        }
#endif /* AFMN_OS_CORR_TIMERS */
}

afmn_os_timer_t afmn_os_timer_create(AFMN_OS_TIMER_TYPE type, afmn_os_timer_cb_t cb)
{
        afmn_os_timer_t ret = NULL;

        OS_ASSERT(type <= AFMN_OS_TIMER_TYPE_ACCURATE_PERIODIC);

#if (AFMN_OS_CORR_TIMERS == 1)
        if (type > AFMN_OS_TIMER_TYPE_PERIODIC) {
                ret = corr_timer_create((type == AFMN_OS_TIMER_TYPE_ACCURATE_PERIODIC),
                        (afmn_os_timer_cb_t) cb);
        } else
#endif /* AFMN_OS_CORR_TIMERS */
        if (os_port_ctx.timer_count < AFMN_OS_TIMER_COUNT_MAX) {
                uint32_t timer_id = search_for_free_timer_id();

                if (timer_id != AFMN_OS_TIMER_COUNT_MAX) {
                        os_port_ctx.afmn_os_timer_cbs[timer_id] = cb;
                        os_port_ctx.afmn_os_timer_expired[timer_id] = false;
                        ret = OS_TIMER_CREATE("ADK_TIM", OS_MAX_DELAY,
                                ((type == AFMN_OS_TIMER_TYPE_PERIODIC) ||
                                 (type == AFMN_OS_TIMER_TYPE_ACCURATE_PERIODIC)) ? OS_TIMER_RELOAD
                                                                                 : OS_TIMER_ONCE,
                                (void *) timer_id, os_timer_cb);
                }
                os_port_ctx.timer_count++;
        }

        return ret;
}

int afmn_os_timer_start(afmn_os_timer_t timer, uint32_t period)
{
#if (AFMN_OS_CORR_TIMERS == 1)
        if (timer_is_corr_timer(timer)) {
                return !corr_timer_change_period(timer, OS_TICKS_2_MS(period));
        }
#endif
        return !(OS_TIMER_CHANGE_PERIOD(timer, period, OS_TIMER_FOREVER) == OS_TIMER_SUCCESS);
}

int afmn_os_timer_stop(afmn_os_timer_t timer)
{
#if (AFMN_OS_CORR_TIMERS == 1)
        if (timer_is_corr_timer(timer)) {
                return !corr_timer_stop(timer);
        }
#endif
        return !(OS_TIMER_STOP(timer, OS_TIMER_FOREVER) == OS_TIMER_SUCCESS);
}

int afmn_os_timer_delete(afmn_os_timer_t timer)
{
#if (AFMN_OS_CORR_TIMERS == 1)
        if (timer_is_corr_timer(timer)) {
                return !corr_timer_delete(timer);
        }
#endif
        OS_ASSERT(os_port_ctx.timer_count > 0);

        uint32_t timer_id = (uint32_t) OS_TIMER_GET_TIMER_ID(timer);

        if (os_port_ctx.timer_count > 0 &&
            OS_TIMER_DELETE(timer, OS_TIMER_FOREVER) == OS_TIMER_SUCCESS) {
                os_port_ctx.afmn_os_timer_cbs[timer_id] = NULL;
                os_port_ctx.timer_count--;
                return 0;
        }

        return 1;
}

bool afmn_os_timer_is_active(afmn_os_timer_t timer)
{
#if (AFMN_OS_CORR_TIMERS == 1)
        if (timer_is_corr_timer(timer)) {
                return corr_timer_is_active(timer);
        }
#endif
        return OS_TIMER_IS_ACTIVE(timer);
}

uint32_t afmn_os_timer_get_timer_id(afmn_os_timer_t timer)
{
#if (AFMN_OS_CORR_TIMERS == 1)
        if (timer_is_corr_timer(timer)) {
                return corr_timer_get_timer_id(timer);
        }
#endif
        return (uint32_t) OS_TIMER_GET_TIMER_ID(timer);
}

uint32_t afmn_os_ms_to_ticks(uint32_t ms)
{
        return OS_MS_2_TICKS(ms);
}

void *afmn_os_malloc(size_t size)
{
        return OS_MALLOC(size);
}

void afmn_os_free(void *addr)
{
        OS_FREE(addr);
}
