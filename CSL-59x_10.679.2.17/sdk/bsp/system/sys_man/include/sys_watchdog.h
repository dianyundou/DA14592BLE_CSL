/**
 * \addtogroup MID_SYS_SERVICES
 * \{
 * \addtogroup WATCHDOG Watchdog Service
 *
 * \brief Watchdog service
 *
 * \{
 */

/**
 ****************************************************************************************
 *
 * @file sys_watchdog.h
 *
 * @brief Watchdog header file.
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

#ifndef SYS_WATCHDOG_H_
#define SYS_WATCHDOG_H_

#include <stdbool.h>
#include <stdint.h>

#define SYS_WATCHDOG_TRIGGER (1 << 31)

/**
 * Initialize sys_watchdog module
 *
 * This should be called before using sys_watchdog module, preferably as early as possible.
 *
 * \note For OS-based applications, the service is initialized by the system.
 *       The application must NOT call this function.
 */
void sys_watchdog_init(void);

/**
 * Register the calling task
 *
 * Returned identifier shall be used in all other calls to sys_watchdog.
 * Once registered, task must notify sys_watchdog periodically using sys_watchdog_notify() to
 * prevent watchdog expiration. Notifications are decided in application level.
 * However, a task can request to be triggered periodically using OS task notify feature.
 * In this case it is expected to notify-back sys_watchdog as a response, calling sys_watchdog_notify().
 *
 * \note
 * \p dg_configWDOG_NOTIFY_TRIGGER_TMO shall be set to non-zero for \p notify_trigger to have any
 * effect.
 *
 * \param [in] notify_trigger   true if task notify should be triggered periodically
 *
 * \return identifier on success, -1 on failure
 *
 * \sa sys_watchdog_notify
 *
 */
int8_t sys_watchdog_register(bool notify_trigger);

/**
 * Stop monitoring a task
 *
 * \param [in] id       identifier from the task's registration to the service
 *
 * \sa sys_watchdog_register
 *
 */
void sys_watchdog_unregister(int8_t id);

/**
 * Suspend a task from being monitored
 *
 * Suspended task is not unregistered entirely but will not be monitored by watchdog until resumed.
 * It's faster than unregistering and registering task again.
 *
 * \param [in] id       identifier from the task's registration to the service
 *
 * \sa sys_watchdog_resume
 *
 */
void sys_watchdog_suspend(int8_t id);

/**
 * Resume a task to be monitored again
 *
 * Resumes a task monitoring, that was previously suspended by sys_watchdog_suspend().
 *
 * \param [in] id       identifier from the task's registration to the service
 *
 * \note This function does not notify the watchdog service for this task. It is possible that
 *       monitor resuming occurs too close to the time that the watchdog expires, before the task
 *       has a chance to explicitly send a notification. This can lead to an unwanted reset.
 *       Therefore, either call sys_watchdog_notify() before calling sys_watchdog_resume(), or use
 *       sys_watchdog_notify_and_resume() instead.
 *
 * \sa sys_watchdog_suspend
 * \sa sys_watchdog_notify_and_resume
 *
 */
void sys_watchdog_resume(int8_t id);

/**
 * Notify sys_watchdog module about a task being alive
 *
 * Registered task must use this periodically to notify sys_watchdog module that it's alive. This
 * should be done frequently enough to fit into hw_watchdog interval set by dg_configWDOG_RESET_VALUE.
 *
 * \param [in] id       identifier from the task's registration to the service
 *
 * \sa sys_watchdog_set_latency
 *
 * \note This function is not designed to be called safely from interrupt context.
 *
 */
__RETAINED_HOT_CODE void sys_watchdog_notify(int8_t id);

/**
 * Notify sys_watchdog module for task with handle \p id and resume its monitoring
 *
 * This function combines the functionality of sys_watchdog_notify() and sys_watchdog_resume().
 *
 * \param [in] id       identifier from the task's registration to the service
 *
 * \sa sys_watchdog_notify()
 * \sa sys_watchdog_resume()
 *
 * \note This function is not designed to be called safely from interrupt context.
 *
 */
void sys_watchdog_notify_and_resume(int8_t id);

/**
 * Set watchdog latency for task
 *
 * This allows task to miss given number of notifications to sys_watchdog without triggering
 * platform reset. Once set, it's allowed that task does not notify sys_watchdog for \p latency
 * consecutive hw_watchdog intervals (as set by dg_configWDOG_RESET_VALUE) which can be used to
 * allow for parts of code which are known to block for long period of time (i.e. computation).
 * This value is set once and does not reload automatically, thus it shall be set every time
 * increased latency is required.
 *
 * \param [in] id       identifier from the task's registration to the service
 * \param [in] latency  latency
 *
 * \note Calling sys_watchdog_notify() / sys_watchdog_notify_and_resume() cancels any latency setting for the given task.
 */
void sys_watchdog_set_latency(int8_t id, uint8_t latency);

/**
 * Notify sys_watchdog module about the idle task being alive
 *
 * The idle task must use this to notify sys_watchdog module that it's alive.
 * This should be done in application level and frequently enough
 * to fit into hw_watchdog interval set by dg_configWDOG_RESET_VALUE.
 * The idle task monitoring is controlled via \sa dg_configWDOG_GUARD_IDLE_TASK
 * When any other registered task notifies the service about its presence,
 * the service assumes that the idle task is also alive.
 * When no task has notified the service during dg_configWDOG_RESET_VALUE period,
 * then it is expected that at least the idle task should be able to notify.
 * In case this fails to happen, the platform is reset.
 *
 * \note When the idle task is monitored, it reserves the service id = 0.
 *
 * \note The idle task is excluded from the timer notification trigger mechanism.
 *
 */
__RETAINED_HOT_CODE void sys_watchdog_idle_task_notify();

#endif /* SYS_WATCHDOG_H_ */

/**
 * \}
 * \}
 */
