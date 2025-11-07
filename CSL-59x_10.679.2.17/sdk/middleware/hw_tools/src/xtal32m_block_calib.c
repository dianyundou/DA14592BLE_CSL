/**
 ****************************************************************************************
 *
 * @file xtal32m_configure_startup.c
 *
 * @brief xtal32m_configure_startup source file.
 *
 * Copyright (C) 2021-2023 Renesas Electronics Corporation and/or its affiliates.
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
#include "osal.h"
#include "hw_gpio.h"
#include "hw_pd.h"
#include "sys_power_mgr.h"
#include "sys_clock_mgr.h"
#include "Xtal_TRIM.h"
#ifndef OS_PRESENT
#include "hw_timer.h"
#endif /* OS_PRESENT */

#include "hw_clk_da1459x.h"

/*
 *  State of XTAL startup
 */

uint16_t xtal32m_configure_cur_set(void);

#ifdef OS_PRESENT
__RETAINED static OS_TIMER cap_meas_sleep_timer = NULL;
__RETAINED static sleep_mode_t prev_sleep_mode;
static void timer_sleep_cb()
{

}
#else
#include "hw_pdc.h"
#include "hw_watchdog.h"
#define NO_OS_SLEEP_HW_TIMER                    (HW_TIMER2)
#define NO_OS_SLEEP_PDC_ENTRY                   (HW_PDC_PERIPH_TRIG_ID_TIMER2)
#define NO_OS_SLEEP_MS_DURATION                 (100)
static void prepare_no_os_sleep_wakeup(void)
{
        static uint32_t pdc_entry_index;

        pdc_entry_index = hw_pdc_add_entry(HW_PDC_LUT_ENTRY_VAL(
                                                HW_PDC_TRIG_SELECT_PERIPHERAL,
                                                NO_OS_SLEEP_PDC_ENTRY,
                                                HW_PDC_MASTER_CM33,
                                                0));
        hw_pdc_set_pending(pdc_entry_index);
        hw_pdc_acknowledge(pdc_entry_index);

        pm_set_sys_wakeup_mode(pm_sys_wakeup_mode_fast);

        hw_watchdog_set_pos_val(dg_configWDOG_IDLE_RESET_VALUE);

        timer_config timer_cfg = {
                .clk_src = HW_TIMER_CLK_SRC_INT,
                .prescaler = 15-1,
                .mode = HW_TIMER_MODE_TIMER,
                .timer = { .direction = HW_TIMER_DIR_UP,
                           .reload_val = NO_OS_SLEEP_MS_DURATION - 1,
                           .free_run = false },
                .pwm = {
                        .frequency = 0,
                        .duty_cycle = 0
                },
        };

        hw_timer_init(NO_OS_SLEEP_HW_TIMER, &timer_cfg);
        hw_timer_register_int(NO_OS_SLEEP_HW_TIMER, NULL);
        hw_timer_enable(NO_OS_SLEEP_HW_TIMER);
}

static void stop_no_os_sleep_wakeup(void)
{
        hw_watchdog_freeze();                   // Stop watchdog
        hw_timer_disable(NO_OS_SLEEP_HW_TIMER);
}

#endif /* OS_PRESENT */

uint16_t run_xtal32m_block_calib(void)
{
        uint16_t status = -XTAL_CALIBRATION_ERROR;

        /*
         *  xtal32m_cap_meas expects to be able to close the XTAL in order to measure it.
         *  Since the XTAL is enabled by the PDC,
         *  before performing the test, the system will be forced to sleep in order to close the XTAL.
         *  All peripherals will be closed and CMAC will remain in sleep.
         *  This way the XTAL will not be needed and will not be enabled on wake-up.
         */
#ifdef OS_PRESENT
        /*
         *  Wait until the system is able to go to sleep.
         */
        cm_wait_lp_clk_ready();

        /*
         *  Wait for the other masters to be able to enter sleep.
         */
        hw_pd_wait_power_down_rad();

        /*
         * The following was designed with ST_FW in mind as the host application.
         * Currently, ST_FW has RC32 as highest priority clock and,
         * only one XTAL request for clock (which is the main clock).
         * The request for XTAL will be restored after sleep.
         */
        sys_clk_t prev_clk = cm_sys_clk_get();
        if (prev_clk != sysclk_RC32) {
                cm_sys_clk_release(prev_clk);
        }
#else
        sys_clk_is_t prev_clk = hw_clk_get_sysclk();
        if (prev_clk != SYS_CLK_IS_RC32) {
                hw_clk_set_sysclk(SYS_CLK_IS_RC32);
        }
#endif /* OS_PRESENT */


#ifdef OS_PRESENT
        /* Set sleep mode */
        prev_sleep_mode = pm_sleep_mode_set(pm_mode_extended_sleep);

        /* Start timer to release M33 from sleep. */
        if (!cap_meas_sleep_timer) {
                cap_meas_sleep_timer = OS_TIMER_CREATE("cap_meas_sleep", 1, OS_TIMER_FAIL, NULL, timer_sleep_cb);
        }
        OS_TIMER_CHANGE_PERIOD(cap_meas_sleep_timer, OS_MS_2_TICKS(100), OS_TIMER_FOREVER);
        OS_TIMER_START(cap_meas_sleep_timer, OS_TIMER_FOREVER);

        /*
         * M33 should wait here longer than the sleep time.
         */
        OS_DELAY(OS_MS_2_TICKS(200));

        /* Set to the previous sleep mode */
        pm_sleep_mode_set(prev_sleep_mode);
#else
        /*
         * Prepare a wake-up before entering sleep.
         */
        prepare_no_os_sleep_wakeup();

        /*
         * Enter sleep state.
         */
        pm_sleep_enter_no_os(pm_mode_extended_sleep);

        hw_clk_delay_usec(10000);

        /*
         * Clear any sleep and wake-up related operation.
         */
        stop_no_os_sleep_wakeup();
#endif /* OS_PRESENT */


        status = xtal32m_configure_cur_set();

        /*
         * Wait for XTAL block to settle.
         */
        hw_clk_delay_usec(50000);

        /*
         * Restore system clock.
         */
        cm_sys_clk_status_t cm_sys_clk_set_status = cm_sysclk_invalid_clock;
        if (prev_clk != cm_sys_clk_get()) {
                cm_sys_clk_set_status = cm_sys_clk_request(prev_clk);
        }

        /*
         * Based on the operation status, and the clock switch status,
         * enumerate the overall operation status for the host application.
         */
        if (status == -XTAL_OPERATION_SUCCESS) {
                if (cm_sys_clk_set_status == cm_sysclk_success) {
                        status = -XTAL_OPERATION_SUCCESS;
                } else {
                        status = -XTAL_CALIBRATION_ERROR;
                }
        }

        return status;
}



// ********************************************************************************
// ********************************************************************************
// * Configuration startup
// * Measures xtal parameters and obtains suitable values for:
// * XTAL32M_TRIM_REG.XTAL32M_CUR_SET
// ********************************************************************************
// ********************************************************************************
uint16_t xtal32m_configure_cur_set(void)
{
        uint16_t status = -XTAL_CALIBRATION_ERROR;

        /*
         * System clock should not be based on XTAL.
         */
        sys_clk_is_t sys_clk_is = hw_clk_get_sysclk();
        if (sys_clk_is != SYS_CLK_IS_RC32) {
                return (uint16_t) -XTAL_CALIBRATION_ERROR;
        }

        REG_SETF(CRG_XTAL, XTAL32M_CTRL_REG, XTAL32M_ENABLE, 0);
        for (int i= 0; i < 500; i++) {
                hw_clk_delay_usec(100);
        }
        status = (hw_clk_xtalm_configure_cur_set() >= 0)?(-XTAL_OPERATION_SUCCESS):(-XTAL_CALIBRATION_ERROR);

        return status;
}

