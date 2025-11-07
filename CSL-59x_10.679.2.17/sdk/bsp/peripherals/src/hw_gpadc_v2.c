/**
 ****************************************************************************************
 *
 * @file hw_gpadc_v2.c
 *
 * @brief Implementation of the GPADC Low Level Driver.
 *
 * Copyright (C) 2022-2023 Renesas Electronics Corporation and/or its affiliates.
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
#if dg_configUSE_HW_GPADC

#include "hw_gpadc.h"
#include "sys_tcs.h"

/**
 * \brief A macro to define default temperature calibration point for the DIE_TEMP sensor
 */
#define HW_GPADC_TEMP_CALIB_POINT               (2700)

/**
 * \brief A macro to define default adc calibration point for the DIE_TEMP sensor
 */
#define HW_GPADC_ADC_CALIB_POINT                (687 << (HW_GPADC_UNUSED_BITS))

/*
 * DIE_TEMP temperature coefficient
 * as LSB per Celsius degree in the 16-bit resolution scale
 */
const int16_t die_temp_coefficient = 144;  //2.246*64

/*
 * Default calibration point for the DIE_TEMP sensor.
 * Realistic but NOT at all accurate.
 */
#define DIE_TEMP_CALIBRATION_POINT_DEF          {.temp = HW_GPADC_TEMP_CALIB_POINT, .adc = HW_GPADC_ADC_CALIB_POINT}


/*
 * Declaring the variable calibration points
 */
__RETAINED_RW static hw_gpadc_calibration_point_t die_temp_calibration_point = DIE_TEMP_CALIBRATION_POINT_DEF;

void hw_gpadc_check_tcs_custom_values(int16_t se_gain_error, int16_t se_offset_error, int16_t diff_gain_error, int16_t diff_offset_error)
{
        if ((se_gain_error == 0) && (se_offset_error == 0)) {
                sys_tcs_apply_custom_values(SYS_TCS_GROUP_GP_ADC_SINGLE_MODE, sys_tcs_custom_values_system_cb, NULL);
        }
        if ((diff_gain_error == 0) && (diff_offset_error == 0)) {
                sys_tcs_apply_custom_values(SYS_TCS_GROUP_GP_ADC_DIFF_MODE, sys_tcs_custom_values_system_cb, NULL);
        }
        if ((die_temp_calibration_point.temp == HW_GPADC_TEMP_CALIB_POINT) && (die_temp_calibration_point.adc == HW_GPADC_ADC_CALIB_POINT)) {
                sys_tcs_apply_custom_values(SYS_TCS_GROUP_TEMP_SENS_25C, sys_tcs_custom_values_system_cb, NULL);
        }
}

int16_t hw_gpadc_get_voltage(void)
{
        return hw_gpadc_convert_to_millivolt(NULL, hw_gpadc_get_raw_value());
}

/***************************************************************************
 ******************      TEMPERATURE SENSOR functions  *********************
 ***************************************************************************/

void hw_gpadc_store_ambient_calibration_point(uint16_t raw_val, int16_t temp)
{
        hw_gpadc_config_t temp_cfg;
        temp_cfg.positive = HW_GPADC_INP_DIE_TEMP;
        temp_cfg.input_mode = HW_GPADC_INPUT_MODE_SINGLE_ENDED;

        die_temp_calibration_point.temp = temp;
        die_temp_calibration_point.adc  = hw_gpadc_apply_correction(&temp_cfg, raw_val);
}


int16_t hw_gpadc_convert_to_celsius_x100_util(const hw_gpadc_config_t *cfg, uint16_t raw_val)
{
        hw_gpadc_calibration_point_t cp = {0, 0};
        int16_t tc = 0;
        uint16_t corrected;
        HW_GPADC_INPUT_POSITIVE positive = cfg ? cfg->positive : hw_gpadc_get_positive();
        ASSERT_WARNING(positive == HW_GPADC_INP_DIE_TEMP);
        corrected = hw_gpadc_apply_correction(cfg, raw_val);
        cp = die_temp_calibration_point;
        tc = die_temp_coefficient;
        int32_t accurate_ratio = (int32_t)((corrected - cp.adc) * 100) / (int32_t)tc;
        return cp.temp + (int16_t)accurate_ratio;
}

uint16_t hw_gpadc_convert_celsius_x100_to_raw_val_util(const hw_gpadc_config_t *cfg, int16_t temperature)
{
        hw_gpadc_calibration_point_t cp = {0, 0};
        int16_t tc = 0;
        HW_GPADC_INPUT_POSITIVE positive = cfg ? cfg->positive : hw_gpadc_get_positive();
        ASSERT_WARNING(positive == HW_GPADC_INP_DIE_TEMP);
        cp = die_temp_calibration_point;
        tc = die_temp_coefficient;

        int32_t accurate_ratio = ((temperature - cp.temp) * tc) / 100;
        return (uint16_t)((int32_t)cp.adc + accurate_ratio);
}

#endif /* dg_configUSE_HW_GPADC */
