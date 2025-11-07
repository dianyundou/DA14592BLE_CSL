/**
 ****************************************************************************************
 *
 * @file sys_watchdog.c
 *
 * @brief Watchdog Service
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

#include "sdk_defs.h"
#include "hw_watchdog.h"

#if dg_configUSE_WDOG

#include "sys_watchdog.h"
#include "sys_watchdog_internal.h"
#include "osal.h"

#if defined(OS_PRESENT)
/* Compile the service with task information available */
#define SYS_WDOG_DEBUG                  (dg_configIMAGE_SETUP == DEVELOPMENT_MODE)

/* mutex to synchronize access to watchdog data */
__RETAINED static OS_MUTEX watchdog_mutex;

#define WATCHDOG_MUTEX_CREATE()         OS_ASSERT(watchdog_mutex == NULL); \
                                        OS_MUTEX_CREATE(watchdog_mutex);   \
                                        OS_ASSERT(watchdog_mutex)
#define WATCHDOG_MUTEX_GET()            OS_ASSERT(watchdog_mutex); \
                                        OS_MUTEX_GET(watchdog_mutex, OS_MUTEX_FOREVER)
#define WATCHDOG_MUTEX_PUT()            OS_MUTEX_PUT(watchdog_mutex)
#else
#define SYS_WDOG_DEBUG                  (0)
#define WATCHDOG_MUTEX_CREATE()
#define WATCHDOG_MUTEX_GET()
#define WATCHDOG_MUTEX_PUT()
#endif /* OS_PRESENT */

#if defined(OS_PRESENT)

#if defined(CONFIG_USE_BLE)
#define BLE_WDOG_TASKS_CNT              (2)
#else
#define BLE_WDOG_TASKS_CNT              (0)
#endif

#define SYSTEM_WDOG_TASKS_CNT (                                         \
                                        ( dg_configUSE_SYS_ADC      * 2 ) +     \
                                        ( dg_configUSE_DGTL             ) +     \
                                        ( dg_configUSE_SYS_DRBG         ) +     \
                                        ( dg_configUSE_CLI              ) +     \
                                        ( dg_configUSE_CONSOLE          ) +     \
                                        ( BLE_WDOG_TASKS_CNT            ) +     \
                                        ( dg_configWDOG_GUARD_IDLE_TASK )       \
                              )
#else
#define SYSTEM_WDOG_TASKS_CNT           (0)
#endif /* OS_PRESENT */

#define TOTAL_TASKS_CNT                 (dg_configWDOG_MAX_TASKS_CNT + SYSTEM_WDOG_TASKS_CNT)
/* number of tasks registered for wdog */
__RETAINED static int8_t max_task_id;

/* bitmask of tasks identifiers which are registered */
__RETAINED volatile static uint32_t registered_mask;

/* bitmask of tasks identifiers which are monitored (registered and not suspended) */
__RETAINED static uint32_t monitored_mask;

/* bitmask of tasks which notified during last period */
__RETAINED volatile static uint32_t notified_mask;

/* allowed latency set by tasks, if any */
__RETAINED volatile static uint8_t tasks_latency[TOTAL_TASKS_CNT];

#if SYS_WDOG_DEBUG
/* handles of registered tasks */
__RETAINED OS_TASK tasks_handle[TOTAL_TASKS_CNT];
#endif

#define SYS_WDOG_UNGUARDED              (-1)
#define SYS_WDOG_GUARDED                (0)

/* the wdog id of the IDLE task */
__RETAINED_RW __UNUSED static int8_t idle_task_id = SYS_WDOG_UNGUARDED;

#if dg_configWDOG_NOTIFY_TRIGGER_TMO
/* bitmask of tasks which requested notification trigger from common timer */
__RETAINED static uint32_t trigger_mask;

/* task handle for tasks which requested notification trigger from common timer */
__RETAINED static OS_TASK trigger_handle[TOTAL_TASKS_CNT];

/* timer handle for common notification trigger */
__RETAINED static OS_TIMER trigger_timer;
#endif

/* Store the last reload value set to the watchdog */
__RETAINED static uint16_t watchdog_reload_value;

#define VALIDATE_ID(id) \
        do { \
                if ((id) < 0 || (id) >= TOTAL_TASKS_CNT) { \
                        OS_ASSERT(0); \
                        return; \
                } \
        } while (0)

#if dg_configUSE_WDOG
__RETAINED_CODE static void reset_watchdog(bool write_busy_wait)
{
        sys_watchdog_set_pos_val(dg_configWDOG_RESET_VALUE);

        if (write_busy_wait) {
                /*
                 *  Wait until the programmed WDOG_VAL is updated in the Watchdog timer.
                 *  If we do not wait then the NMI IRQ will be raised continuously calling
                 *  NMI Handler and watchdog_cb
                 */
                while (hw_watchdog_check_write_busy());
        }
        notified_mask = 0;
}

__RETAINED_CODE static void watchdog_cb(unsigned long *exception_args)
{
        uint32_t tmp_mask = monitored_mask;
        uint32_t latency_mask = 0;
        int i;

        /*
         * watchdog is reset immediately when we detect that all tasks notified during period so
         * no need to check this here
         *
         * but if we're here, then check if some tasks have non-zero latency and remove them from
         * notify mask check and also decrease latency for each task
         */
        for (i = 0; i <= max_task_id; i++) {
                if (tasks_latency[i] == 0) {
                        continue;
                }

                tasks_latency[i]--;

                latency_mask |= (1 << i);
        }

        /*
         * check if all remaining tasks notified and reset hw_watchdog in such case
         */
        tmp_mask &= ~latency_mask;
        if ((notified_mask & tmp_mask) == tmp_mask) {
                reset_watchdog(true);
                return;
        }

        /*
         * latency for all tasks expired and some of them still did not notify sys_watchdog
         * we'll let watchdog reset the system
         *
         * note that hw_watchdog_handle_int() never returns
         */
        hw_watchdog_handle_int(exception_args);

        reset_watchdog(true);
}
#endif

#if dg_configWDOG_NOTIFY_TRIGGER_TMO
static void watchdog_auto_notify_cb(OS_TIMER timer)
{
        int i;

        WATCHDOG_MUTEX_GET();

        for (i = 0; i <= max_task_id; i++) {
                if (trigger_handle[i]) {
                        OS_TASK_NOTIFY(trigger_handle[i], SYS_WATCHDOG_TRIGGER, OS_NOTIFY_SET_BITS);
                }
        }

        WATCHDOG_MUTEX_PUT();
}
#endif

#endif

void sys_watchdog_init(void)
{
#if dg_configUSE_WDOG
        max_task_id = 0;
        notified_mask = 0;

        sys_watchdog_set_pos_val(dg_configWDOG_RESET_VALUE);

        WATCHDOG_MUTEX_CREATE();

#if dg_configWDOG_NOTIFY_TRIGGER_TMO
        trigger_timer = OS_TIMER_CREATE("wdog",
                        OS_MS_2_TICKS(dg_configWDOG_NOTIFY_TRIGGER_TMO),
                        OS_TIMER_RELOAD, NULL, watchdog_auto_notify_cb);
#endif
#endif
}

int8_t sys_watchdog_register(bool notify_trigger)
{
#if dg_configUSE_WDOG
        int8_t id = 0;

        WATCHDOG_MUTEX_GET();

#if defined(OS_PRESENT)
        ASSERT_WARNING(OS_GET_TASK_SCHEDULER_STATE() == OS_SCHEDULER_RUNNING);
#endif

#if dg_configWDOG_GUARD_IDLE_TASK
        /* The idle task registers when the first task registers the service */
        if (idle_task_id == SYS_WDOG_UNGUARDED) {
                registered_mask = 1;
                monitored_mask = 1;
#if SYS_WDOG_DEBUG
                tasks_handle[0] = OS_GET_IDLE_TASK_HANDLE();
#endif
                hw_watchdog_register_int(watchdog_cb);
                idle_task_id = SYS_WDOG_GUARDED;
        }
#endif /* dg_configWDOG_GUARD_IDLE_TASK */

        while (registered_mask & (1 << id)) {
                id++;
        }

        if (id >= TOTAL_TASKS_CNT) {
                /* Don't allow registration of more than TOTAL_TASKS_CNT */
                OS_ASSERT(0);
                return -1;
        }

        registered_mask |= (1 << id);
        monitored_mask |= (1 << id);

#if SYS_WDOG_DEBUG
        tasks_handle[id] = OS_GET_CURRENT_TASK();
#endif

        if (id > max_task_id) {
                max_task_id = id;
        }

#if !dg_configWDOG_GUARD_IDLE_TASK
        if (id == 0) {
                hw_watchdog_register_int(watchdog_cb);
        }
#endif

#if dg_configWDOG_NOTIFY_TRIGGER_TMO
        if (notify_trigger) {
                /* this is first task to request trigger - start timer */
                if (!trigger_mask && (OS_TIMER_IS_ACTIVE(trigger_timer) == OS_FALSE)) {
                        OS_TIMER_START(trigger_timer, OS_TIMER_FOREVER);
                }
                trigger_mask |= (1 << id);
                trigger_handle[id] = OS_GET_CURRENT_TASK();
        }
#endif

        WATCHDOG_MUTEX_PUT();

        return id;
#else
        return -1;
#endif /* dg_configUSE_WDOG */
}

void sys_watchdog_unregister(int8_t id)
{
#if dg_configUSE_WDOG
        uint32_t tmp_mask;
        int8_t new_max = 0;

        VALIDATE_ID(id);

        WATCHDOG_MUTEX_GET();

#if dg_configWDOG_GUARD_IDLE_TASK
        /* Unregistering the idle task is not recommended */
        ASSERT_WARNING(id != 0);
#endif
        registered_mask &= ~(1 << id);
        monitored_mask &= ~(1 << id);
        tasks_latency[id] = 0;

#if SYS_WDOG_DEBUG
        tasks_handle[id] = NULL;
#endif

#if dg_configWDOG_NOTIFY_TRIGGER_TMO
        /* this was last task to have requested trigger - stop timer */
        if (trigger_mask == (1 << id)) {
                OS_TIMER_STOP(trigger_timer, OS_TIMER_FOREVER);
        }
        trigger_mask &= ~(1 << id);
        trigger_handle[id] = 0;
#endif

        /* recalculate max task id */
        tmp_mask = registered_mask;
        while (tmp_mask) {
                tmp_mask >>= 1;
                new_max++;
        }

        max_task_id = new_max;

        WATCHDOG_MUTEX_PUT();
#endif
}

void sys_watchdog_suspend(int8_t id)
{
#if dg_configUSE_WDOG
        VALIDATE_ID(id);

        WATCHDOG_MUTEX_GET();

        monitored_mask &= ~(1 << id);

        WATCHDOG_MUTEX_PUT();
#endif
}

#if dg_configUSE_WDOG
__STATIC_INLINE void resume_monitoring(int8_t id)
{
        monitored_mask |= (1 << id);
        monitored_mask &= registered_mask;
}
#endif

void sys_watchdog_resume(int8_t id)
{
#if dg_configUSE_WDOG
        VALIDATE_ID(id);

        WATCHDOG_MUTEX_GET();

        resume_monitoring(id);

        WATCHDOG_MUTEX_PUT();
#endif
}

#if dg_configUSE_WDOG
__STATIC_INLINE void notify_about_task(int8_t id, bool resume)
{
        /* Make sure that the requested task is one of the watched tasks */
        OS_ASSERT(registered_mask & (1 << id));

        uint32_t idle_notif_mask = 0;

#if dg_configWDOG_GUARD_IDLE_TASK
        if (idle_task_id == SYS_WDOG_GUARDED) {
                idle_notif_mask = 1;
        } else {
                idle_notif_mask = 0;
        }
#endif
        notified_mask |= ((1 << id) | idle_notif_mask);

        if (resume) {
                resume_monitoring(id);
        }

        /*
         * we also reset latency here because it's ok for app to notify before latency
         * expired, but it should start with zero latency for next notification interval
         */
        tasks_latency[id] = 0;

        if ((notified_mask & monitored_mask) == monitored_mask) {
                reset_watchdog(false);
        }
}
#endif

__RETAINED_HOT_CODE void sys_watchdog_notify(int8_t id)
{
#if dg_configUSE_WDOG
        VALIDATE_ID(id);

        WATCHDOG_MUTEX_GET();

        notify_about_task(id, false);

        WATCHDOG_MUTEX_PUT();
#endif
}

void sys_watchdog_notify_and_resume(int8_t id)
{
#if dg_configUSE_WDOG
        VALIDATE_ID(id);

        WATCHDOG_MUTEX_GET();

        /* Notify and resume */
        notify_about_task(id, true);

        WATCHDOG_MUTEX_PUT();
#endif
}

void sys_watchdog_set_latency(int8_t id, uint8_t latency)
{
#if dg_configUSE_WDOG
        VALIDATE_ID(id);

        WATCHDOG_MUTEX_GET();

        tasks_latency[id] = latency;

        WATCHDOG_MUTEX_PUT();
#endif
}

__RETAINED_CODE bool sys_watchdog_monitor_mask_empty()
{
#if dg_configUSE_WDOG
#if dg_configWDOG_GUARD_IDLE_TASK
        return (monitored_mask == 1);
#else
        return (monitored_mask == 0);
#endif /* dg_configWDOG_GUARD_IDLE_TASK */
#else
        return true;
#endif
}

__RETAINED_CODE void sys_watchdog_set_pos_val(uint16_t value)
{
#if dg_configUSE_WDOG
        if (hw_watchdog_check_write_busy()) {
                // WATCHDOG is still busy writing the previous value

                // Previous value was written using sys_watchdog_set_pos_val()
                ASSERT_WARNING(watchdog_reload_value != 0);

                if (watchdog_reload_value == value) {
                       /* The previous value was the same. No need to wait for
                          WRITE_BUSY and write it again. */
                        return;
                }
        }

        watchdog_reload_value = value;
#endif
        hw_watchdog_set_pos_val(value);
}

__RETAINED_HOT_CODE uint16_t sys_watchdog_get_val(void)
{
#if dg_configUSE_WDOG
        if (hw_watchdog_check_write_busy()) {
                // WATCHDOG is still busy writing the previous value

                // Previous value was written using sys_watchdog_set_pos_val()
                ASSERT_WARNING(watchdog_reload_value != 0);

                return watchdog_reload_value;
        }
#else
        while (hw_watchdog_check_write_busy());
#endif
        return hw_watchdog_get_val();
}

__RETAINED_HOT_CODE void sys_watchdog_idle_task_notify()
{
#if dg_configUSE_WDOG && dg_configWDOG_GUARD_IDLE_TASK
        WATCHDOG_MUTEX_GET();

        notify_about_task(0, false);

        WATCHDOG_MUTEX_PUT();
#endif
}


