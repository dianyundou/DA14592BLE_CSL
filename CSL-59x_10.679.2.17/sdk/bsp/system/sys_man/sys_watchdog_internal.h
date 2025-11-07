/**
 ****************************************************************************************
 *
 * @file sys_watchdog_internal.h
 *
 * @brief Watchdog service internal header file.
 *
 * Copyright (C) 2019-2023 Renesas Electronics Corporation and/or its affiliates.
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

#ifndef SYS_WATCHDOG_INTERNAL_H_
#define SYS_WATCHDOG_INTERNAL_H_

#include <stdbool.h>
#include <stdint.h>

/**
 * Find out if the only task currently monitored is the IDLE task.
 *
 * \return true if only the IDLE task is currently monitored, else false. This function is used
 * from CPM in order to know if watchdog should be stopped during sleep.
 */
__RETAINED_CODE bool sys_watchdog_monitor_mask_empty(void);

/**
 * \brief Set positive reload value of the watchdog timer
 *
 * \param [in] value reload value
 *
 * \note Care must be taken not to mix calls to sys_watchdog_set_pos_val() and
 * sys_watchdog_get_val() with call to hw_watchdog_set_pos_val()
 */
__RETAINED_CODE void sys_watchdog_set_pos_val(uint16_t value);

/**
 * \brief Get the value of the watchdog timer
 *
 * \return The watchdog value
 *
 * \note Care must be taken not to mix calls to sys_watchdog_set_pos_val() and
 * sys_watchdog_get_val() with call to hw_watchdog_set_pos_val()
 */
__RETAINED_HOT_CODE uint16_t sys_watchdog_get_val(void);

#endif /* SYS_WATCHDOG_INTERNAL_H_ */

