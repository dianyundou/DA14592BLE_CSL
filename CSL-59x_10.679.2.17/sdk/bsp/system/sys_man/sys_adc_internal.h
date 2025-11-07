/**
 ****************************************************************************************
 *
 * @file sys_adc_internal.h
 *
 * @brief sys_adc_internal header file.
 *
 * Copyright (C) 2018-2024 Renesas Electronics Corporation and/or its affiliates.
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

#ifndef SYS_ADC_INTERNAL_H_
#define SYS_ADC_INTERNAL_H_

#include "ad_gpadc.h"
///@cond INTERNAL
#include "../adapters/src/sys_platform_devices_internal.h"
///@endcond

/**
 * \brief Sys adc measurement type enum
 *
 */
typedef enum {
        SYS_ADC_TEMP = 0,
#if (dg_configUSE_LP_CLK == LP_CLK_RCX)
        SYS_ADC_BATT,
#endif
        SYS_ADC_MAX_MEASUREMENTS,
} SYS_ADC_MEASUREMENTS;
/**
 * \brief Sys adc measurement value
 *
 */
typedef struct {
        uint16_t adc_val[SYS_ADC_MAX_MEASUREMENTS];
} sys_adc_val_t;



/**
 * \brief Enable sys_adc service
 *
 */
void sys_adc_enable(void);

/**
 * \brief Disable sys_adc service
 *
 */
void sys_adc_disable(void);

/**
 * \brief Initialize sys_adc service
 *
 * This function initializes the SysADC task, which is responsible for measuring the die temperature
 * and notifying other tasks when specified temperature boundaries have been exceeded to take the
 * following actions:

 * - When the RCX is employed as the low power clock, it notifies the rc_clocks_calibration_task()
 *   to perform RCX calibration.
 *
 * - When the RCX is employed as the low power clock and the RTC correction is enabled, the
 *   rc_clocks_calibration_task() is notified to correct the RTC values that have been affected by
 *   the drift of the RCX clock.
 *
 * - If the RF calibration is enabled, an internal variable of the ad_ble is updated, in order for
 *   the ad_ble to perform RF calibration when necessary.
 *
 * \warning To measure the die temperature, the SysADC task relies on the ad_gpadc. If another task
 *          is also utilizing the ad_gpadc and has acquired the GPADC resource, the SysADC task is
 *          forced to wait until the GPADC is released. This blocking of the SysADC task results in
 *          delaying any action triggered by the temperature measurement, including the critical
 *          RCX and RF calibration. This can have a significant impact on various operations of the
 *          application that are time-critical or involve BLE events. Therefore, it is crucial for
 *          the application to handle GPADC arbitration with care.
 *
 */

void sys_adc_init(void);

/**
 * \brief Trigger sys_adc service
 *
 * \note Function triggers the sys adc service in case the time difference
 * between the upcoming and last GPADC measurement exceeds a specified time threshold
 */
__RETAINED_HOT_CODE void sys_adc_trigger(void);

/**
 * \brief Resume RF calibration
 *
 */
void sys_adc_resume_rf_calibration(void);

/**
 * \brief Suspend RF calibration
 *
 */
void sys_adc_suspend_rf_calibration(void);

/**
 * \brief Get the die temperature
 *
 * This function returns the die temperature measurement in hundredth of Celsius degrees,
 * (ex. 2540 = 25.4 C) which is periodically performed by SysADC task.
 *
 * \ return The GPADC temperature measurement in hundredth of Celsius degrees.
 */
int16_t sys_adc_get_die_temp_x100(void);



#endif /* SYS_ADC_INTERNAL_H_ */

#endif /* (dg_configUSE_SYS_ADC == 1) */

