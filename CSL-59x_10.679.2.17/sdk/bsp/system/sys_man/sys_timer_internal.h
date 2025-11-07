/**
 ****************************************************************************************
 *
 * @file sys_timer_internal.h
 *
 * @brief System timer internal header file.
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

#ifndef SYS_TIMER_INTERNAL_H_
#define SYS_TIMER_INTERNAL_H_

#include "sdk_defs.h"

#if dg_configUSE_HW_TIMER
# define HAVE_SYS_TIMER
#endif

#ifdef HAVE_SYS_TIMER

#include "hw_timer.h"

/* System Timer - abstraction layer */

#define LP_CNT_NATIVE_MASK      ( REG_MSK(TIMER2, TIMER2_TIMER_VAL_REG, TIM_TIMER_VALUE) >> REG_POS(TIMER2, TIMER2_TIMER_VAL_REG, TIM_TIMER_VALUE) )

#define SYS_HW_TIMER                    HW_TIMER2
#define SYS_HW_TIMER_IRQ                TIMER2_IRQn




/**
 * \brief Update Real Time Clock value and get current time (in LP cycles).
 *
 * \param[out] timer_value Pointer to the value of the timer
 * \return The current RTC time.
 *
 * \warning This function is used only by the Clock and Power Manager.
 *
 */
__RETAINED_HOT_CODE uint64_t sys_timer_get_timestamp_fromCPM(uint32_t *timer_value);

/**
 * \brief Set an "invalid" trigger value, which refers far away in the future
 *
 */
__STATIC_FORCEINLINE void sys_timer_invalidate_trigger(void)
{
        uint32_t lp_current_time;
        uint32_t trigger;

        lp_current_time = hw_timer_get_count(SYS_HW_TIMER);         // Get current time
        trigger = (lp_current_time - 1) & LP_CNT_NATIVE_MASK;  // Get an "invalid" trigger
        bool sys_timer_irq_en = HW_TIMER_REG_GETF(SYS_HW_TIMER, TIMER_CTRL_REG, TIM_IRQ_EN);
        if (sys_timer_irq_en) {
                HW_TIMER_REG_SETF(SYS_HW_TIMER, TIMER_CTRL_REG, TIM_IRQ_EN, 0);
        }
        hw_timer_set_reload(SYS_HW_TIMER, trigger);
        if (sys_timer_irq_en) {
                HW_TIMER_REG_SETF(SYS_HW_TIMER, TIMER_CTRL_REG, TIM_IRQ_EN, 1);
        }
}

/**
 * \brief Calculate how many ticks have passed since the time the system entered sleep or idle
 *        mode and update OS
 *
 * \return The number of ticks spent while sleeping
 */
__RETAINED_HOT_CODE uint32_t sys_timer_update_slept_time(void);

/**
 * \brief Get the address of the current Real Time Clock time.
 *
 * \return The address of the current RTC time.
 *
 */
uint64_t* sys_timer_get_rtc_time(void);

/**
 * \brief Get the address of the current time.
 *
 * \return The address of the current time.
 *
 */
uint32_t* sys_timer_get_current_time(void);


#endif /* HAVE_SYS_TIMER */

#endif /* SYS_TIMER_INTERNAL_H_ */

