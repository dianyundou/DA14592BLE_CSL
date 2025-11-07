/**
 * \addtogroup MID_SYS_SERVICES
 * \{
 * \addtogroup CLOCK_MANAGER Clock Manager Service
 *
 * \brief Clock Manager
 *
 * \{
 */

/**
 ****************************************************************************************
 *
 * @file sys_clock_mgr.h
 *
 * @brief Clock Manager header file.
 *
 * Copyright (C) 2017-2024 Renesas Electronics Corporation and/or its affiliates.
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

#ifndef SYS_CLOCK_MGR_H_
#define SYS_CLOCK_MGR_H_

#include <stdint.h>
#include <stdbool.h>
#include "hw_clk.h"

typedef enum {
        cm_sysclk_div1_clk_in_use,
        cm_sysclk_ahb_divider_in_use,
        cm_sysclk_dblr_used_by_task,
        cm_sysclk_higher_prio_used,
        cm_sysclk_invalid_clock,
        cm_sysclk_max_requested,
        cm_sysclk_not_requested,
        cm_sysclk_success
} cm_sys_clk_status_t;

/**
 * \brief Initialize clocks after power-up.
 *
 * \param[in] type The clock source to use as the system clock.
 *
 * \warning It must be called with interrupts enabled! It must be called only once, after power-up.
 */
void cm_sys_clk_init(sys_clk_t type);

/**
 * \brief Set the system clock priority.
 *
 * \details The system clock priority indicates which sys_clk prevails against another one
 *          when requesting a new sys_clk using cm_sys_clk_request().
 *
 * \param[in] sys_clk_prio_array: pointer to array containing the system clock priority (in decreasing order)
 *
 *
 * \note The provided array is expected to hold the first three members of the sys_clk_t enum in the order that
 *       denotes the desired priority scheme, with each type appearing only once in the array and the first
 *       element denoting the highest priority sys_clk. In case the array contains more elements, only the first
 *       three are taken into account.
 *
 * \note The default clocks priority in decreasing order is the following:
 *              sysclk_DBLR64
 *              sysclk_XTAL32M
 *              sysclk_RC32
 *
 *       The application can select a different clock priorities order by creating a local temporary priorities
 *       struct instance with the clocks in the desired priority order and then call the cm_sys_clk_set_priority()
 *       like in the example below. If the priorities are not set by the application before the first call of
 *       cm_sys_clk_request() or cm_sys_clk_release(), then the default priorities will be applied.
 *
 *  \code{.c}          sys_clk_t sys_clk_p[] = {
 *                               sysclk_DBLR64,
 *                               sysclk_XTAL32M,
 *                               sysclk_RC32
 *                    };
 *
 *                    cm_sys_clk_set_priority(sys_clk_p);
 * \endcode
 *
 *
 * \warning It has to be called only once, after power-up and before calling cm_sys_clk_init(). If not called the
 *          default clocks priority is used.
 *
 * \sa cm_sys_clk_request()
 * \sa cm_sys_clk_release()
 *
 */
void cm_sys_clk_set_priority(sys_clk_t *sys_clk_prio_array);

/**
 * \brief Request for system clock switch.
 *
 * \details This function can be used for system clock switching.
 *          It requests from the Clock Manager to use a specific clock source as system
 *          clock (sys_clk). If this is possible, the request is accepted and the function switches
 *          the sys_clk setting to the selected clock source and returns cm_sysclk_success.
 *          Otherwise, the request is rejected and the function returns a value that denotes the
 *          reason why the switch was not possible.
 *
 *          The request can be cancelled/released later by calling cm_sys_clk_release().
 *          Until then, the requested sys_clk is protected from any lower priority sys_clk requests
 *          (with the priority as defined by cm_sys_clk_set_priority()).
 *
 *          There are two reasons that make the switching of the system clock not possible.
 *          Either there is at least one peripheral that currently uses the Div1 clock,
 *          or the current system clock has higher priority and is currently in use.
 *          In the second case, a successful switch requires a preceding release operation.
 *
 *          If the request involves enabling the fast Xtal clock (XTAL32M), then, apart from
 *          powering on the XTAL32M, the function will wait for it to settle before setting it as
 *          sys_clk. If the Doubler is requested, the function will also wait for the Doubler to be
 *          available.
 *
 *          The Clock Manager will also take care of automatically restoring the sys_clk to the
 *          requested setting after each wake-up.
 *
 * \param[in] type The clock source to use as sys_clk.
 *
 * \return cm_sysclk_success            if the requested sys_clk switch was applied
 * \return cm_sysclk_div1_clk_in_use    if the sys_clk cannot be switched because a peripheral is
 *                                       clocked by the Div1 clock
 * \return cm_sysclk_higher_prio_used   if a higher priority system clock is still in use
 *
 * \note Requires a preceding call of cm_sys_clk_set_priority().
 *
 * \note Even if the request is rejected, it is still taken into account (until cancelled by a
 *       corresponding cm_sys_clk_release() call), and the switch to the requested clock may take
 *       place later (e.g. as soon as the existing higher priority clock requests are released).
 *
 * \warning It cannot be called from Interrupt Context.
 *
 * \warning It may block.
 *
 * \sa cm_sys_clk_set_priority()
 * \sa cm_sys_clk_release()
 */
cm_sys_clk_status_t cm_sys_clk_request(sys_clk_t type);

/**
 * \brief Restores the system clock. It terminates a matching request.
 *
 * \details If there are other sys_clk requests that are still active (i.e. not released),
 *          the system will switch to the one that has the highest priority. Otherwise, it will
 *          switch to the initial sys_clk that was specified when cm_sys_clk_init() was called.
 *          No change however will be made if Div1 is in use.
 *
 * \param[in] type The clock source the system was requested to use, when the matching
 *                  cm_sys_clk_request() was called.
 *
 * \return cm_sysclk_success            if the release operation was completed successfully and either
 *                                      triggered a switch to a lower_priority sys_clk or did not
 *                                      trigger any sys_clk_change (either because there are still
 *                                      other active requests for the same sys_clk that have not been
 *                                      released yet or because there are no other active requests and
 *                                      the released sys_clk is also the default one
 *                                      (as specified in cm_sys_clk_init()))
 * \return cm_sysclk_div1_clk_in_use    if the release operation was completed successfully and it should
 *                                      normally trigger a new sys_clk change (based on other active requests)
 *                                      but it didn't because a peripheral is clocked by the Div1 clock
 *
 * \warning This function must be called ONLY for terminating a matching cm_sys_clk_request.
 *          If called alone the system will reach an error-state!
 *
 * \warning It cannot be called from Interrupt Context.
 *
 * \warning It may block.
 *
 * \sa cm_sys_clk_request
 */
cm_sys_clk_status_t cm_sys_clk_release(sys_clk_t type);


/**
 * \brief Change the divider of the AMBA Peripheral Bus clock.
 *
 * \details The frequency of the APB clock is (system_clock / (1 << cm_ahbclk)) / (1 << cm_apbclk).
 *
 * \param[in] div The new value of the APB divider.
 *
 * \warning It may block. It cannot be called from Interrupt Context.
 */
void cm_apb_set_clock_divider(apb_div_t div);

/**
 * \brief Change the divider of the AMBA High speed Bus clock.
 *
 * \details The frequency of the AHB clock is (system_clock / (1 << cm_ahbclk)).
 *          Note: if the SysTick runs then it is the dg_configABORT_IF_SYSTICK_CLK_ERR setting that
 *          controls whether the switch will be aborted or not.
 *
 * \param[in] div The new value of the AHB divider.
 *
 * \return True if the divider was changed to the requested value, else false.
 *
 * \warning It may block. It cannot be called from Interrupt Context.
 */
bool cm_ahb_set_clock_divider(ahb_div_t div);

/**
 * \brief Returns the sys_clk that the system uses at that moment.
 *
 * \return The real sys_clk used by the system.
 *
 * \warning It may block. It cannot be called from Interrupt Context.
 */
sys_clk_t cm_sys_clk_get(void);

/**
 * \brief Returns the sys_clk that the system uses at that moment (interrupt safe version).
 *
 * \return The real sys_clk used by the system.
 */
sys_clk_t cm_sys_clk_get_fromISR(void);

/**
 * \brief Returns the AMBA Peripheral Bus clock divider.
 *
 * \return The pclk being used.
 *
 * \warning It may block. It cannot be called from Interrupt Context.
 */
apb_div_t cm_apb_get_clock_divider(void);

/**
 * \brief Returns the AMBA High speed Bus clock divider.
 *
 * \return The hclk being used.
 *
 * \warning It may block. It cannot be called from Interrupt Context.
 */
ahb_div_t cm_ahb_get_clock_divider(void);

/**
 * \brief Returns the CPU clock frequency.
 *
 * \return The CPU clock being used.
 *
 * \warning Any restrictions of the cm_sys_clk_get() and cm_ahb_get_clock_divider() apply here as
 *          well. It may block. It cannot be called from Interrupt Context.
 */
cpu_clk_t cm_cpu_clk_get(void);

/**
 * \brief Returns the CPU clock frequency (interrupt safe).
 *
 * \return The CPU clock being used.
 *
 * \warning It can be called from Interrupt Context.
 */
cpu_clk_t cm_cpu_clk_get_fromISR(void);
/**
 * \brief Calibrate RCLP.
 */
void cm_calibrate_rclp(void);

/**
 * \brief Calibrate RC32M.
 */
void cm_calibrate_rc32m(void);
#ifdef OS_PRESENT
/**
 * \brief Converts usec to RCX cycles.
 *
 * \return The number of RCX cycles for the given time period.
 *
 * \warning Maximum time period is 4.095msec.
 */
__RETAINED_CODE uint32_t cm_rcx_us_2_lpcycles(uint32_t usec);

/**
 * \brief Converts time to RCX cycles.
 *
 * \return The number of RCX cycles for the given time period.
 *
 * \warning This is a low accuracy function. To have good accuracy, the minimum time period should
 *        be 1msec and the maximum 200msec. Above 200msec, the function calculates more RCX cycles
 *        than necessary.
 */
uint32_t cm_rcx_us_2_lpcycles_low_acc(uint32_t usec);

#endif

/**
 * \brief Block until the fast XTAL clock (XTALxxM) is ready. If the fast XTAL clock (XTALxxM) is
 *        running then the function exits immediately.
 *
 * \warning It may block. It cannot be called from Interrupt Context.
 */
void cm_wait_xtalm_ready(void);

#ifdef OS_PRESENT
/**
 * \brief Initialize the RCX calibration task.
 */
void cm_rc_clocks_calibration_task_init(void);


/**
 * \brief Initialize the Low Power clock.
 *
 * \details It initializes and sets as LP clock either the RCX or the XTAL32K. Since the XTAL32K
 *          settling takes a long time, the system is kept in active mode until this completes.
 */
void cm_lp_clk_init(void);

/**
 * \brief Check if the Low Power clock is available.
 *
 * \return true if the LP clock is available, else false.
 *
 * \warning It does not block. It cannot be called from Interrupt Context.
 */
bool cm_lp_clk_is_avail(void);

/**
 * \brief Check if the Low Power clock is available, interrupt safe version.
 *
 * \return true if the LP clock is available, else false.
 *
 * \warning It does not block. It can be called from Interrupt Context.
 */
bool cm_lp_clk_is_avail_fromISR(void);

/**
 * \brief Wait until the Low Power clock is available.
 *
 * \warning It may block. It cannot be called from Interrupt Context.
 */
void cm_wait_lp_clk_ready(void);

/**
 * \brief   Clear the flag that indicates that the Low Power clock is available.
 *
 * \details It is called when the system wakes up from a "forced" deep sleep state and the XTAL32K
 *          is used as the LP clock so that the system won't enter into sleep until the crystal has
 *          settled.
 *
 * \warning It may block. It cannot be called from Interrupt Context.
 */
__RETAINED_CODE void cm_lp_clk_wakeup(void);

#endif /* OS_PRESENT */


/**
 * \brief Block until the Doubler is ready.
 *
 * \warning It may block. It cannot be called from Interrupt Context.
 */
void cm_wait_dblr_ready(void);


/**
 * \brief Check if the fast XTAL clock (XTALxxM) is ready.
 *
 * \return True if the fast XTAL clock (XTALxxM) has settled, else false.
 */
__RETAINED_CODE bool cm_poll_xtalm_ready(void);

/**
 * \brief Start the fast XTAL clock (XTALxxM)
 *
 * \details Checks if the fast XTAL clock (XTALxxM) is started. If not, it checks if there is a PDC
 * entry for starting the fast XTAL clock (XTALxxM). If there is, it uses PDC to start the fast
 * XTAL clock (XTALxxM). Otherwise, it enables the fast XTAL clock (XTALxxM) using
 * hw_clk_enable_sysclk().
 */
void cm_enable_xtalm(void);


/* ---------------------------------------------------------------------------------------------- */

/*
 * Functions intended to be used only by the Clock and Power Manager.
 */

/**
 * \brief Set the system clock (unprotected).
 *
 * \details It attempts to:
 *              - Prepare the system clock for sleep : called when the system is entering power-down mode.
 *                        The system clock settings of the application are kept in order to be able to
 *                        restore them. If the PLL is active it will be turned off.
 *                        (It is called with the scheduler stopped and all interrupts disabled in
 *                        this case.)
 *              - Restore the previous setting : called when the fast XTAL clock (XTALxxM) settles.
 *                        (It is called from ISR context with all interrupts disabled in this case.)
 *
 * \param[in] entering_sleep true if the system is going to sleep, else false.
 *
 * \warning It must be called from Interrupt Context and/or with all interrupts disabled.
 *          The function is internal to the clock and power managers and should not be used externally!
 */
__RETAINED_HOT_CODE void cm_sys_clk_sleep(bool entering_sleep);

/**
 * \brief Halt until the fast XTAL clock (XTALxxM) has settled.
 *
 * \details It executes a WFI() call waiting for the fast XTAL clock (XTALxxM) Ready interrupt.
 *          Any other interrupts that hit are served.
 */
__RETAINED_CODE void cm_halt_until_xtalm_ready(void);

/**
 * \brief Register a callback function to be called then XTAL32M is ready
 *
 * \details cb pointer to the callback function
 */
void cm_register_xtal_ready_callback(void (*cb)(void));

/**
 * \brief Halt until system clock (either PLL or XTAL32M) is ready.
 *
 * \details It executes a WFI() call waiting for the XTALxxM Ready interrupt and PLL LOCK interrupt if needed.
 */
__RETAINED_HOT_CODE void cm_halt_until_sysclk_ready(void);

#endif /* SYS_CLOCK_MGR_H_ */

/**
 \}
 \}
 */
