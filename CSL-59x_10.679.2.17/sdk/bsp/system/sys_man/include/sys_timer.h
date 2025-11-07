/**
 * \addtogroup MID_SYS_SERVICES
 * \{
 * \addtogroup SYS_TIMER System Timer
 *
 * \brief System timer
 *
 * \{
 */

/**
 ****************************************************************************************
 *
 * @file sys_timer.h
 *
 * @brief System timer header file.
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

#ifndef SYS_TIMER_H_
#define SYS_TIMER_H_


#include "sdk_defs.h"

/**
 * \brief Start the OS timer.
 *
 * \param period The timer period in LP clock cycles
 *
 */
void sys_timer_start(uint32_t period);

/**
 * \brief Stop the OS timer.
 *
 */
void sys_timer_stop(void);

/**
 * \brief Set OS timer trigger
 *
 * \param[in] trigger The new timer trigger
 */
__RETAINED_CODE void sys_timer_set_trigger(uint32_t trigger);

/**
 * \brief Get OS timer offset in the tick period
 *
 * \return The offset in the tick period
 */
__RETAINED_HOT_CODE uint32_t sys_timer_get_tick_offset(void);


/**
 * \brief Get uptime ticks value.
 *
 * \return The uptime ticks. Each tick will be 1000000 / OS_TICK_CLOCK_HZ
 *         (e.g. 30.5us if XTAL32K is set as LP clock)
 *
 * \warning This function is called only from OS Tasks.
 *
 */
uint64_t sys_timer_get_uptime_ticks(void);

/**
 * \brief Get uptime ticks value.
 *
 * \return The uptime ticks. Each tick will be 1000000 / OS_TICK_CLOCK_HZ
 *         (e.g. 30.5us if XTAL32K is set as LP clock)
 *
 * \warning This function is called only when interrupts are disabled.
 *
 */
__RETAINED_CODE uint64_t sys_timer_get_uptime_ticks_fromISR(void);

/**
 * \brief Get uptime in usec.
 *
 * \return The uptime in usec.
 *
 * \warning This function is called only from OS Tasks.
 *
 */
uint64_t sys_timer_get_uptime_usec(void);

/**
 * \brief Get uptime in usec.
 *
 * \return The uptime in usec.
 *
 * \warning This function is called only when interrupts are disabled.
 *
 */
__RETAINED_HOT_CODE uint64_t sys_timer_get_uptime_usec_fromISR(void);

/**
 * \brief Get timestamp value.
 *
 * \return The current timestamp. This is expressed in ticks of the clock that clocks OS timer
 *         (e.g. XTAL32K). For example, if XTAL32K drives OS timer, each timestamp tick will be
 *         1000000 / 32768 = 30.5uS
 *
 * \warning This function is called only from OS Tasks.
 * \deprecated This function is deprecated. User shall call sys_timer_get_uptime_ticks() instead.
 */
DEPRECATED_MSG("API no longer supported, use sys_timer_get_uptime_ticks() instead.")
__STATIC_INLINE uint64_t sys_timer_get_timestamp(void)
{
        return sys_timer_get_uptime_ticks();
}
/**
 * \brief Get timestamp value.
 *
 * \return The current timestamp. This is expressed in ticks of the clock that clocks OS timer
 *         (e.g. XTAL32K). For, example, if XTAL32K drives OS timer, each timestamp tick will be
 *         1000000 / 32768 = 30.5uS
 *
 * \warning This function is called only when interrupts are disabled.
 * \deprecated This function is deprecated. User shall call sys_timer_get_uptime_ticks_fromISR() instead.
 */
DEPRECATED_MSG("API no longer supported, use sys_timer_get_uptime_ticks_fromISR() instead.")
__STATIC_INLINE uint64_t sys_timer_get_timestamp_fromISR(void)
{
        return sys_timer_get_uptime_ticks_fromISR();
}
#endif /* SYS_TIMER_H_ */

/**
\}
\}
*/
