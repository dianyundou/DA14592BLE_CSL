/**
 ****************************************************************************************
 *
 * @file sys_timer.c
 *
 * @brief System timer
 *
 * Copyright (C) 2017-2023 Renesas Electronics Corporation and/or its affiliates.
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

#if dg_configUSE_HW_TIMER
# define HAVE_SYS_TIMER
#endif

#ifdef HAVE_SYS_TIMER

#include <stdint.h>
#include "hw_timer.h"

#include "sys_timer_internal.h"

#ifdef OS_PRESENT
#include "sys_timer.h"
#include "osal.h"
#include "sys_power_mgr.h"
#include "sys_power_mgr_internal.h"
#endif


/* The new timer reload value cannot be written to the timer until the previous reload value has been
 * synchronized. The synchronization may take up to 2 clock cycles, so a margin of one additional timer value
 * must be allowed.
 */
#define TICK_GUARD_PRESC_LIM                    3

/*
 * Global and / or retained variables
 */

__RETAINED uint32_t lp_last_trigger;               // counts in prescaled LP cycles

#if (SYS_TIM_DEBUG == 1)

__RETAINED uint32_t rt_elapsed_time;
__RETAINED uint32_t rt_elapsed_ticks;
__RETAINED uint32_t trigger_hit_at_ret;

typedef uint8_t trigger_type_t;
typedef uint32_t trigger_value_t ;

struct sys_tim_trigger_mon_st {
        trigger_type_t type;    // 0: lp_last_trigger, 1: trigger
        trigger_value_t value;
};

#define MAX_TRG_MON_SZ          64      // Must be a power of 2

__RETAINED struct sys_tim_trigger_mon_st rt_trigger_mon[MAX_TRG_MON_SZ];
__RETAINED uint32_t rt_trigger_mon_wr;

#endif /* SYS_TIM_DEBUG */


#ifdef OS_PRESENT
__RETAINED static uint32_t current_time;
__RETAINED static uint64_t sys_rtc_time;
#endif


/*
 * Function definitions
 */

#ifdef OS_PRESENT

__RETAINED_CODE static uint32_t sys_timer_advance_time_compute(uint32_t *trigger)
{
        uint32_t elapsed_time;
        uint32_t elapsed_ticks;
        uint32_t tick_offset;
        uint32_t timer_value = hw_timer_get_count(SYS_HW_TIMER);

        hw_timer_unregister_int(SYS_HW_TIMER);

        elapsed_time = (timer_value - lp_last_trigger) & LP_CNT_NATIVE_MASK;

        /* When in Idle mode, the elapsed time period cannot be equal to LP_CNT_NATIVE_MASK!
         * If this happens then it is a "fake" system timer interrupt caused by the 1 LP cycle propagation
         * delay of the internal interrupt of system timer. This interrupt must be ignored!!! */
        if (pm_get_system_sleep_state() != sys_powered_down) {
                if (elapsed_time >= (LP_CNT_NATIVE_MASK - 1)) {
                        *trigger = (lp_last_trigger + OS_TICK_PERIOD) & LP_CNT_NATIVE_MASK;
                        return 0;
                }
        }

        /* We know that we can be within 1 tick period "earlier" than the system time since we may
         * have to intentionally advance the system time by 1 tick during the trigger programming.
         */
        if (elapsed_time >= LP_CNT_NATIVE_MASK - OS_TICK_PERIOD) {
                *trigger = (lp_last_trigger + OS_TICK_PERIOD) & LP_CNT_NATIVE_MASK;
                return 0;
        }

        elapsed_ticks = elapsed_time / OS_TICK_PERIOD;
        tick_offset = elapsed_time - (elapsed_ticks * OS_TICK_PERIOD);  // Offset in current tick (in prescaled LP cycles).

#if (SYS_TIM_DEBUG == 1)
        rt_elapsed_time = elapsed_time;
        rt_elapsed_ticks = elapsed_ticks;
#endif

        // Compute next trigger.
        *trigger = lp_last_trigger + (elapsed_ticks + 1) * OS_TICK_PERIOD;
        if (TICK_GUARD_PRESC_LIM >=
                        (OS_TICK_PERIOD - tick_offset)) {       // Too close in time
                *trigger += OS_TICK_PERIOD;                     // Set trigger at the next tick
                elapsed_ticks++;                                // Report 1 more tick spent sleeping
        }
        *trigger &= LP_CNT_NATIVE_MASK;                         // Make sure it's within limits...

        lp_last_trigger = (*trigger - OS_TICK_PERIOD) & LP_CNT_NATIVE_MASK;

#if (SYS_TIM_DEBUG == 1)
        rt_trigger_mon[rt_trigger_mon_wr].type = 0;
        rt_trigger_mon[rt_trigger_mon_wr].value = lp_last_trigger;
        rt_trigger_mon_wr++;
        rt_trigger_mon_wr %= MAX_TRG_MON_SZ;
#endif
        if ((dg_configUSE_LP_CLK == LP_CLK_32000) || (dg_configUSE_LP_CLK == LP_CLK_32768)) {
                uint32_t test_val = lp_last_trigger + 1;
                /* test_val must be an exact multiple of OS_TICK_PERIOD */
                ASSERT_WARNING((test_val % OS_TICK_PERIOD) == 0);
        }
        return elapsed_ticks;
}

__RETAINED_CODE static void sys_timer_advance_time_apply(uint32_t trigger)
{
        uint32_t dummy __UNUSED;
        uint32_t timer_current_value;
        uint32_t diff;
        uint32_t cs = 0;

        /* Disable irqs until the HW timer is programmed with the calculated reload value.
         * In this way, the execution of any higher priority irqs does not delay the current
         * execution context, while the HW timer is running in the background, leading to
         * a lost OS tick situation.
         */

        if (in_interrupt()) {
                OS_ENTER_CRITICAL_SECTION_FROM_ISR(cs);
        } else {
                OS_ENTER_CRITICAL_SECTION();
        }

        /* Trigger value must be greater than the current timer value, so a margin of one timer value must be allowed.
         * The value of the timer peripheral may change after calling hw_timer_get_count() and before setting the
         * new reload value when calling sys_timer_set_trigger(), so a margin of a slightly higher timer value must be
         * allowed.
         */
        timer_current_value = (hw_timer_get_count(SYS_HW_TIMER) + TICK_GUARD_PRESC_LIM) & LP_CNT_NATIVE_MASK;
        diff = (timer_current_value - trigger) & LP_CNT_NATIVE_MASK;

        while (diff <= LP_CNT_NATIVE_MASK/2) {
                trigger += OS_TICK_PERIOD;           // Set trigger at the next tick
                trigger &= LP_CNT_NATIVE_MASK;       // Make sure it's within limits...

                timer_current_value = (hw_timer_get_count(SYS_HW_TIMER) + TICK_GUARD_PRESC_LIM) & LP_CNT_NATIVE_MASK;
                diff = (timer_current_value - trigger) & LP_CNT_NATIVE_MASK;
        }

        sys_timer_set_trigger(trigger);

        /* The timer value may have changed. Check again that the trigger is greater than the current timer value.
         * If it is not, the timer event will not be generated when expected.
         */
        ASSERT_WARNING(trigger != hw_timer_get_count(SYS_HW_TIMER));


        if (in_interrupt()) {
                OS_LEAVE_CRITICAL_SECTION_FROM_ISR(cs);
        } else {
                OS_LEAVE_CRITICAL_SECTION();
        }

#if (SYS_TIM_DEBUG == 1)
        rt_trigger_mon[rt_trigger_mon_wr].type = 1;
        rt_trigger_mon[rt_trigger_mon_wr].value = trigger;
        rt_trigger_mon_wr++;
        rt_trigger_mon_wr %= MAX_TRG_MON_SZ;
#endif
}

/**
 * \brief Advances time from the previous tick that hit.
 *
 * \details Calculate how many ticks have passed since the last tick.
 *
 * \return uint32_t The number of ticks passed.
 *
 */
__RETAINED_CODE static uint32_t sys_timer_advance_time(void)
{
        uint32_t elapsed_ticks;
        uint32_t trigger;

        elapsed_ticks = sys_timer_advance_time_compute(&trigger);
        sys_timer_advance_time_apply(trigger);

        return elapsed_ticks;
}

__RETAINED_HOT_CODE uint32_t sys_timer_update_slept_time(void)
{
        uint32_t trigger;
        uint32_t elapsed_ticks;
        /*
         * Update Real Time Clock value and calculate the time spent sleeping.
         * lp_prescaled_time - lp_last_trigger : sleep time in lp cycles
         * lp_previous_time : lp timer offset at sleep entry
         */
#if (SYS_TIM_DEBUG == 1)
        trigger_hit_at_ret = hw_timer_get_count(SYS_HW_TIMER);
#endif

        // Calculate time spent sleeping in ticks and the offset in this tick period.
        elapsed_ticks = sys_timer_advance_time_compute(&trigger);

        // Advance time
        if (elapsed_ticks > 0) {
                OS_TICK_INCREMENT( elapsed_ticks - 1 );
        }

        sys_timer_advance_time_apply(trigger);

        return elapsed_ticks;
}

#endif /* OS_PRESENT */

/**
 * \brief Interrupt handler of system timer
 *
 */
__RETAINED_CODE void OS_TICK_Handler(void)
{
#ifdef OS_PRESENT
        uint32_t ulPreviousMask;

        OS_ENTER_CRITICAL_SECTION_FROM_ISR(ulPreviousMask);

        if (pm_get_system_sleep_state() == sys_active)
        {
                uint32_t elapsed_ticks;

                DBG_SET_HIGH(PWR_MGR_FUNCTIONAL_DEBUG, PWRDBG_TICK);

                elapsed_ticks = sys_timer_advance_time();
                while (elapsed_ticks) {
                        OS_TICK_ADVANCE();
                        elapsed_ticks--;
                }
                DBG_SET_LOW(PWR_MGR_FUNCTIONAL_DEBUG, PWRDBG_TICK);
        }

        OS_LEAVE_CRITICAL_SECTION_FROM_ISR(ulPreviousMask);
#endif
}


void sys_timer_start(uint32_t period)
{
        timer_config timer_cfg = {
                .clk_src = HW_TIMER_CLK_SRC_INT,
                .prescaler = 0,
                .timer = { .direction = HW_TIMER_DIR_UP,
                           .reload_val = period-1,
                           .free_run = true },
                .pwm = { .frequency = 0, .duty_cycle = 0},
        };

        lp_last_trigger = LP_CNT_NATIVE_MASK;

        hw_timer_init(SYS_HW_TIMER, &timer_cfg);
        hw_timer_register_int(SYS_HW_TIMER, OS_TICK_Handler);
        hw_timer_enable(SYS_HW_TIMER);
}

void sys_timer_stop(void)
{
        hw_timer_disable(SYS_HW_TIMER);
}

__RETAINED_CODE void sys_timer_set_trigger(uint32_t trigger)
{
        TBA(SYS_HW_TIMER)->TIMER_CLEAR_IRQ_REG = 1;
        hw_timer_set_reload(SYS_HW_TIMER, trigger);
        NVIC_ClearPendingIRQ(SYS_HW_TIMER_IRQ);                // Clear any pending IRQ from this source
        hw_timer_register_int(SYS_HW_TIMER, OS_TICK_Handler);  // Enable interrupt

#if (SYS_TIM_DEBUG == 1)
        rt_trigger_mon[rt_trigger_mon_wr].type = 1;
        rt_trigger_mon[rt_trigger_mon_wr].value = trigger;
        rt_trigger_mon_wr++;
        rt_trigger_mon_wr %= MAX_TRG_MON_SZ;
#endif
}

__RETAINED_HOT_CODE uint32_t sys_timer_get_tick_offset(void)
{
        uint32_t lp_tick_offset, lp_current_time;

        lp_current_time = hw_timer_get_count(SYS_HW_TIMER);

        lp_tick_offset = lp_current_time - lp_last_trigger;
         // 1b. Set within valid limits.
         lp_tick_offset &= LP_CNT_NATIVE_MASK;
         if (lp_tick_offset > (LP_CNT_NATIVE_MASK / 2)) {
                 lp_tick_offset = 0;
         }
         return lp_tick_offset;
}


#ifdef OS_PRESENT
#if (dg_configUSE_LP_CLK == LP_CLK_RCX)
__RETAINED static uint64_t sys_rtc_clock_time_fine;         // counts time in usec*1024*1024
__RETAINED static uint64_t sys_rtc_clock_time;         // counts time in usec
#endif

static __RETAINED_CODE void update_timestamp_values(void)
{
        uint32_t prev_time = current_time;

        current_time = hw_timer_get_count(SYS_HW_TIMER);
        uint32_t rtc_tick = (current_time - prev_time) & LP_CNT_NATIVE_MASK;

#if (dg_configUSE_LP_CLK == LP_CLK_RCX)
        uint64_t rcx_time_advance = 0;
        rcx_time_advance = rtc_tick * (uint64_t)cm_get_rcx_clock_period();
        if ((sys_rtc_clock_time_fine + rcx_time_advance) < sys_rtc_clock_time_fine) {
                sys_rtc_clock_time +=  (sys_rtc_clock_time_fine >> 20);
                /* Keep fractional part */
                sys_rtc_clock_time_fine &= ((1 << 20) - 1);
        }
        sys_rtc_clock_time_fine += rcx_time_advance;
#endif
        sys_rtc_time += rtc_tick;
}

uint64_t sys_timer_get_uptime_ticks(void)
{
        OS_ENTER_CRITICAL_SECTION();
        update_timestamp_values();
        OS_LEAVE_CRITICAL_SECTION();

        return sys_rtc_time;
}

__RETAINED_CODE uint64_t sys_timer_get_uptime_ticks_fromISR(void)
{
        uint32_t ulPreviousMask;

        OS_ENTER_CRITICAL_SECTION_FROM_ISR(ulPreviousMask);
        update_timestamp_values();
        OS_LEAVE_CRITICAL_SECTION_FROM_ISR(ulPreviousMask);

        return sys_rtc_time;
}

uint64_t sys_timer_get_uptime_usec(void)
{
        OS_ENTER_CRITICAL_SECTION();
        update_timestamp_values();
        OS_LEAVE_CRITICAL_SECTION();

#if (dg_configUSE_LP_CLK == LP_CLK_RCX)
        return sys_rtc_clock_time + (sys_rtc_clock_time_fine >> 20);
#else
        return (sys_rtc_time * 1000000) / OS_TICK_CLOCK_HZ;
#endif
}

__RETAINED_HOT_CODE uint64_t sys_timer_get_uptime_usec_fromISR(void)
{
        uint32_t ulPreviousMask;

        OS_ENTER_CRITICAL_SECTION_FROM_ISR(ulPreviousMask);
        update_timestamp_values();
        OS_LEAVE_CRITICAL_SECTION_FROM_ISR(ulPreviousMask);

#if (dg_configUSE_LP_CLK == LP_CLK_RCX)
        return sys_rtc_clock_time + (sys_rtc_clock_time_fine >> 20);
#else
        return (sys_rtc_time * 1000000) / OS_TICK_CLOCK_HZ;
#endif
}


__RETAINED_HOT_CODE uint64_t sys_timer_get_timestamp_fromCPM(uint32_t* timer_value)
{
        update_timestamp_values();
        *timer_value = current_time;

        return sys_rtc_time;
}

uint64_t* sys_timer_get_rtc_time(void)
{
        return &sys_rtc_time;
}

uint32_t* sys_timer_get_current_time(void)
{
        return &current_time;
}


#endif /* OS_PRESENT */

#endif /* HAVE_SYS_TIMER */
