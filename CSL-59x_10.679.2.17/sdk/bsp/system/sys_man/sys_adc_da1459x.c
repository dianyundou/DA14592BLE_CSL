/**
 ****************************************************************************************
 *
 * @file sys_adc_da1459x.c
 *
 * @brief RF & RCX calibration source file.
 *
 * Copyright (C) 2020-2024 Renesas Electronics Corporation and/or its affiliates.
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


#if (dg_configUSE_SYS_ADC == 1)

#include "sys_adc_internal.h"
#include "sys_clock_mgr_internal.h"
#include "sys_timer.h"
#include "sys_watchdog.h"
#include "osal.h"
#if dg_configRF_ENABLE_RECALIBRATION
#include "ble_config.h"
#include "ad_ble.h"
#endif
#if dg_configRTC_CORRECTION
#include "hw_rtc.h"
#endif

#if dg_configRF_ENABLE_RECALIBRATION
#define SYS_ADC_PERIOD_TICKS            OS_MS_2_TICKS(MIN(dg_configRF_CALIB_TEMP_POLL_INTV, RC_TEMP_POLL_INT))
#else
#define SYS_ADC_PERIOD_TICKS            OS_MS_2_TICKS(RC_TEMP_POLL_INT)
#endif

#define mainBIT_SYS_ADC_TMR_CALL        (1 << 1)
#define mainBIT_SYS_ADC_TRIGGER         (1 << 2)
#define mainBIT_SYS_ADC_RF_CALIBRATION  (1 << 3)
#define SYS_ADC_PRIORITY                (OS_TASK_PRIORITY_NORMAL)
#define SYS_ADC_TIME_THRESHOLD          (SYS_ADC_PERIOD_TICKS / 2)

static void sys_adc_timer_callback(OS_TIMER pxTimer);

static OS_TASK handle_sys_adc;
static OS_TIMER sys_adc_timer;
static OS_TASK_FUNCTION(Sys_ADC, pvParameters);

#if dg_configRTC_CORRECTION
__RETAINED static uint32_t rtc_drift_cnt;
#endif
__RETAINED static OS_TICK_TIME previous_tick;
__RETAINED static int16_t cur_die_temp_x100;
#if (dg_configUSE_LP_CLK == LP_CLK_RCX)
__RETAINED static int16_t rcx_pre_die_temp_x100;
#endif
#if (CM_ENABLE_RCLP_CALIBRATION == 1)
__RETAINED static int16_t rclp_pre_die_temp_x100;
#endif
#if (CM_ENABLE_RC32M_CALIBRATION == 1)
__RETAINED static uint32_t uncond_trigger_cnt;
#endif

static AD_GPADC_ERROR read_adc_value(const ad_gpadc_controller_conf_t *conf, int16_t *temp_x100,
                                     uint16_t *temp_adc_value)
{
        AD_GPADC_ERROR err = AD_GPADC_ERROR_HANDLE_INVALID;
        ad_gpadc_handle_t sys_adc_handle;

        sys_adc_handle = ad_gpadc_open(conf);

        err = ad_gpadc_read_nof_conv(sys_adc_handle, 1, temp_adc_value);

        *temp_x100 = hw_gpadc_convert_to_celsius_x100_util(TEMP_SENSOR_INTERNAL.drv, *temp_adc_value);

        ad_gpadc_close(sys_adc_handle, true);

        return err;
}

void sys_adc_init(void)
{
        OS_BASE_TYPE status;
        uint16_t temp_adc_value;

        /* Initialize all temperature variables before creating the Sys_ADC task */
        OS_ASSERT(read_adc_value(&TEMP_SENSOR_INTERNAL, &cur_die_temp_x100, &temp_adc_value) == AD_GPADC_ERROR_NONE);
#if (CM_ENABLE_RCLP_CALIBRATION == 1)
        rclp_pre_die_temp_x100 = cur_die_temp_x100;
#endif
#if (dg_configUSE_LP_CLK == LP_CLK_RCX)
        rcx_pre_die_temp_x100 = cur_die_temp_x100;
#endif
#if dg_configRF_ENABLE_RECALIBRATION
        ad_ble_set_rf_calibration_info(temp_adc_value);
#endif

        sys_adc_timer = OS_TIMER_CREATE("Sys_adcSet",
                                SYS_ADC_PERIOD_TICKS,
                                OS_TIMER_RELOAD,
                                (void *) 0,                     // Timer id == none
                                sys_adc_timer_callback);        // Call-back
        OS_ASSERT(sys_adc_timer != NULL);

        /* Create Sys_ADC task */
        status = OS_TASK_CREATE("Sys_ADC",              /* The text name assigned to the task, for
                                                           debug only; not used by the kernel. */
                                Sys_ADC,                /* The function that implements the task. */
                                0,                      /* The parameter passed to the task. */
                                OS_MINIMAL_TASK_STACK_SIZE,
                                SYS_ADC_PRIORITY,       /* The priority assigned to the task. */
                                handle_sys_adc);
        OS_ASSERT(status == OS_TASK_CREATE_SUCCESS);

        sys_adc_enable();
}

static void sys_adc_timer_callback(OS_TIMER pxTimer)
{
        OS_TASK_NOTIFY(handle_sys_adc, mainBIT_SYS_ADC_TMR_CALL, OS_NOTIFY_SET_BITS);
}

/**
 * \brief Sys_ADC task
 *
 */
static OS_TASK_FUNCTION(Sys_ADC, pvParameters)
{
        uint32_t ulNotifiedValue;
        int8_t wdog_id;

        DBG_CONFIGURE_LOW(SYS_ADC_DEBUG, SYS_ADC_DBG_READ_TEMPERATURE);
        DBG_CONFIGURE_LOW(SYS_ADC_DEBUG, SYS_ADC_DBG_SET_RF_CALIBRATION_INFO);
        DBG_CONFIGURE_LOW(SYS_ADC_DEBUG, SYS_ADC_DBG_RC_CALIBRATION_NOTIFY);
        DBG_CONFIGURE_LOW(SYS_ADC_DEBUG, SYS_ADC_DBG_TIMER_RESET);
        DBG_CONFIGURE_LOW(SYS_ADC_DEBUG, SYS_ADC_DBG_RC_CLOCK_CALIBRATION);

        /* Register task to be monitored by watchdog */
        wdog_id = sys_watchdog_register(false);

        while (1) {
                OS_BASE_TYPE xResult __UNUSED;

#if ((CM_ENABLE_RCLP_CALIBRATION == 1) || (dg_configUSE_LP_CLK == LP_CLK_RCX) || (CM_ENABLE_RC32M_CALIBRATION == 1))
                uint32_t rc_clocks_to_calibrate = 0;
#endif
                /* Notify watchdog on each loop since there's no other trigger for this */
                sys_watchdog_notify(wdog_id);

                /* Suspend monitoring while task is blocked on OS_TASK_NOTIFY_WAIT() */
                sys_watchdog_suspend(wdog_id);

                /* Wait for any event group bit, and then clear all of them except of
                 * 'mainBIT_SYS_ADC_RF_CALIBRATION'. This bit serves as a switch to enable
                 * or disable RF calibration during runtime and is exclusively modified
                 * by ad_ble through the use of sys_adc_resume_rf_calibration() and
                 * sys_adc_suspend_rf_calibration(). */
                xResult = OS_TASK_NOTIFY_WAIT(OS_TASK_NOTIFY_NONE,
                                              (OS_TASK_NOTIFY_ALL_BITS ^ mainBIT_SYS_ADC_RF_CALIBRATION),
                                              &ulNotifiedValue, OS_TASK_NOTIFY_FOREVER);

                OS_ASSERT(xResult == OS_OK);

                /* Resume watchdog monitoring */
                sys_watchdog_notify_and_resume(wdog_id);

                if (ulNotifiedValue & (mainBIT_SYS_ADC_TMR_CALL | mainBIT_SYS_ADC_TRIGGER |
                                       mainBIT_SYS_ADC_RF_CALIBRATION)) {
                        uint16_t temp_adc_value;

                        DBG_SET_HIGH(SYS_ADC_DEBUG, SYS_ADC_DBG_READ_TEMPERATURE);

                        /* Suspend monitoring while task is blocked on OS_TASK_NOTIFY_WAIT() */
                        sys_watchdog_suspend(wdog_id);

                        /* The read_adc_value() might block, if the ad_gpadc() resource is acquired
                         * by another task. Therefore the watchdog is suspended to prevent it from
                         * expiring in such a case. */
                        OS_ASSERT(read_adc_value(&TEMP_SENSOR_INTERNAL, &cur_die_temp_x100, &temp_adc_value) == AD_GPADC_ERROR_NONE);
                        /* Resume watchdog monitoring */
                        sys_watchdog_notify_and_resume(wdog_id);
                        DBG_SET_LOW(SYS_ADC_DEBUG, SYS_ADC_DBG_READ_TEMPERATURE);

#if dg_configRF_ENABLE_RECALIBRATION
                        if (ulNotifiedValue & mainBIT_SYS_ADC_RF_CALIBRATION) {
                                DBG_SET_HIGH(SYS_ADC_DEBUG, SYS_ADC_DBG_SET_RF_CALIBRATION_INFO);
                                ad_ble_set_rf_calibration_info(temp_adc_value);
                                DBG_SET_LOW(SYS_ADC_DEBUG, SYS_ADC_DBG_SET_RF_CALIBRATION_INFO);
                        }
#endif

#if (dg_configUSE_LP_CLK == LP_CLK_RCX)
# if dg_configRTC_CORRECTION
                        uint32_t rtc_enabled = !HW_RTC_REG_GETF(RTC_CONTROL_REG, RTC_TIME_DISABLE);
                        // Run RTC compensation only if RTC time is running.
                        if (rtc_enabled) {
                                if ((rtc_drift_cnt++ % dg_configRTC_CORRECTION) == 0) {
                                        rc_clocks_to_calibrate |= RCX_RTC_DO_COMPENSATION;
                                }
                        }
# endif /* dg_configRTC_CORRECTION */
                        if ((cur_die_temp_x100 > (rcx_pre_die_temp_x100 + RCX_TEMP_DRIFT_CELSIUS_X100)) ||
                            (cur_die_temp_x100 < (rcx_pre_die_temp_x100 - RCX_TEMP_DRIFT_CELSIUS_X100))) {
                                rcx_pre_die_temp_x100 = cur_die_temp_x100;
                                rc_clocks_to_calibrate |= RCX_DO_CALIBRATION;
# if dg_configRTC_CORRECTION
                                if (rtc_enabled) {
                                        rc_clocks_to_calibrate |= RCX_RTC_DO_COMPENSATION;
                                }
# endif /* dg_configRTC_CORRECTION */
                        }
#endif

#if (CM_ENABLE_RCLP_CALIBRATION == 1)
                        if ((cur_die_temp_x100 > (rclp_pre_die_temp_x100 + RCLP_TEMP_DRIFT_CELSIUS_X100)) ||
                            (cur_die_temp_x100 < (rclp_pre_die_temp_x100 - RCLP_TEMP_DRIFT_CELSIUS_X100))) {
                                rclp_pre_die_temp_x100 = cur_die_temp_x100;
                                rc_clocks_to_calibrate |= RCLP_DO_CALIBRATION;
                        }
#endif

#if (CM_ENABLE_RC32M_CALIBRATION == 1)
                        uncond_trigger_cnt++;
                        if (uncond_trigger_cnt == RC32M_UNCOND_TRIGGER) {
                                uncond_trigger_cnt = 0;
                                rc_clocks_to_calibrate |= RC32M_DO_CALIBRATION;
                        }
#endif

#if ((CM_ENABLE_RCLP_CALIBRATION == 1) || (dg_configUSE_LP_CLK == LP_CLK_RCX) || (CM_ENABLE_RC32M_CALIBRATION == 1))
                        if (rc_clocks_to_calibrate) {
                                DBG_SET_HIGH(SYS_ADC_DEBUG, SYS_ADC_DBG_RC_CALIBRATION_NOTIFY);
                                cm_rc_clocks_calibration_notify(rc_clocks_to_calibrate);
                                DBG_SET_LOW(SYS_ADC_DEBUG, SYS_ADC_DBG_RC_CALIBRATION_NOTIFY);
                        }
#endif
                        if (ulNotifiedValue & mainBIT_SYS_ADC_TRIGGER) {
                                DBG_SET_HIGH(SYS_ADC_DEBUG, SYS_ADC_DBG_TIMER_RESET);
                                OS_TIMER_RESET(sys_adc_timer, OS_TIMER_FOREVER);
                                DBG_SET_LOW(SYS_ADC_DEBUG, SYS_ADC_DBG_TIMER_RESET);
                        }

                        previous_tick = OS_GET_TICK_COUNT();
                }
        }
}

void sys_adc_enable(void)
{
        OS_TIMER_START(sys_adc_timer, OS_TIMER_FOREVER);
}

void sys_adc_disable(void)
{
        OS_TIMER_STOP(sys_adc_timer, OS_TIMER_FOREVER);
}

__RETAINED_HOT_CODE void sys_adc_trigger(void)
{
        if (sys_adc_timer && OS_TIMER_IS_ACTIVE(sys_adc_timer)) {
                OS_TICK_TIME current_tick =  OS_GET_TICK_COUNT();
                if ((current_tick - previous_tick) >= SYS_ADC_TIME_THRESHOLD) {

                        OS_TIMER_STOP(sys_adc_timer, OS_TIMER_FOREVER);
                        OS_TASK_NOTIFY(handle_sys_adc, mainBIT_SYS_ADC_TRIGGER, OS_NOTIFY_SET_BITS);
                }
        }
}

void sys_adc_resume_rf_calibration(void)
{
        OS_TASK_NOTIFY(handle_sys_adc, mainBIT_SYS_ADC_RF_CALIBRATION, OS_NOTIFY_SET_BITS);
}

void sys_adc_suspend_rf_calibration(void)
{
        OS_TASK_NOTIFY_VALUE_CLEAR(handle_sys_adc, mainBIT_SYS_ADC_RF_CALIBRATION);
}

int16_t sys_adc_get_die_temp_x100(void)
{
        return cur_die_temp_x100;
}


#endif /* dg_configUSE_SYS_ADC */
