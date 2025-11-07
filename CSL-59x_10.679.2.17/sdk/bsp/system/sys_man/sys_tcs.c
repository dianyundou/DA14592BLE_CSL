/**
****************************************************************************************
*
* @file sys_tcs.c
*
* @brief TCS Handler
*
* Copyright (C) 2020-2023 Renesas Electronics Corporation and/or its affiliates.
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
#if (dg_configUSE_SYS_TCS == 1)
#include "sys_tcs.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#if (dg_configUSE_HW_GPADC == 1)
#include "hw_gpadc.h"
#endif

#define TCS_DATA sys_tcs_get_tcs_data_ptr()
#define TCS_ATTRIBUTES sys_tcs_get_tcs_attributes_ptr()

uint8_t sys_tcs_get_size(SYS_TCS_GID gid)
{
        ASSERT_WARNING(gid < SYS_TCS_GROUP_MAX);
        return TCS_ATTRIBUTES[gid].size;
}

SYS_TCS_TYPE sys_tcs_get_value_type(SYS_TCS_GID gid)
{
        ASSERT_WARNING(gid < SYS_TCS_GROUP_MAX);
        return TCS_ATTRIBUTES[gid].value_type;
}

void sys_tcs_get_custom_values(SYS_TCS_GID gid, uint32_t **values, uint8_t *size)
{
        ASSERT_WARNING(gid < SYS_TCS_GROUP_MAX);
        ASSERT_WARNING(TCS_ATTRIBUTES[gid].value_type == SYS_TCS_TYPE_TRIM_VAL);

        if (TCS_DATA == NULL) {
                // TCS is not initialize
                return;
        }

        if (size == NULL ) {
                // size is mandatory
                return;
        }

        if (TCS_ATTRIBUTES[gid].start == GID_EMPTY) {
                *size = 0;
        } else {
                *size = TCS_ATTRIBUTES[gid].size;
        }

        if (values) {
                if (*size == 0) {
                        *values = NULL;
                } else {
                        /*if size is not zero then start is different than GID_EMPTY for this GID
                         * so CS parsing for TCS data is done and TCS_DATA is valid */
                        *values = &TCS_DATA[TCS_ATTRIBUTES[gid].start];
                }
        }
}

void sys_tcs_apply_custom_values(SYS_TCS_GID gid, sys_tcs_custom_values_cb cb, void *user_data)
{
        uint32_t* values = NULL;
        uint8_t size = 0;
        if (cb) {
                sys_tcs_get_custom_values(gid, &values, &size);
                if (size != 0) {
                        cb(gid, user_data, values, size);
                }
        }
}

void sys_tcs_get_reg_pairs(SYS_TCS_GID gid, uint32_t **values, uint8_t *size)
{
        ASSERT_WARNING(gid < SYS_TCS_GROUP_MAX);
        ASSERT_WARNING(TCS_ATTRIBUTES[gid].value_type == SYS_TCS_TYPE_REG_PAIR);

        if (TCS_DATA == NULL) {
                // TCS is not initialized
                return;
        }

        if (size == NULL ) {
                // size is mandatory
                return;
        }

        *size = TCS_ATTRIBUTES[gid].size;

        if (values) {
                if (*size == 0) {
                        *values = NULL;
                } else {
                        /*if size is not zero for register pair entry
                         CS parsing for TCS data is done and TCS_DATA is valid */
                        *values = &TCS_DATA[TCS_ATTRIBUTES[gid].start];
                }
        }
}


void sys_tcs_custom_values_system_cb(SYS_TCS_GID gid, void *user_data, const uint32_t *val, uint8_t len)
{
#if (dg_configUSE_HW_GPADC == 1)
        int16_t val_hi = (*val & 0xFFFF0000) >> 16;
        int16_t val_lo = *val & 0xFFFF;

        switch (gid) {
        case SYS_TCS_GROUP_GP_ADC_SINGLE_MODE:
                hw_gpadc_store_se_gain_error(hw_gpadc_calculate_single_ended_gain_error(val_lo, val_hi));
                hw_gpadc_store_se_offset_error(hw_gpadc_calculate_single_ended_offset_error(val_lo, val_hi));
                break;
        case SYS_TCS_GROUP_GP_ADC_DIFF_MODE:
                hw_gpadc_store_diff_gain_error(
                        hw_gpadc_calculate_differential_gain_error(val_lo, val_hi));
                hw_gpadc_store_diff_offset_error(
                        hw_gpadc_calculate_differential_offset_error(val_lo, val_hi));
                break;
        case SYS_TCS_GROUP_TEMP_SENS_25C:
                if (sys_tcs_get_testprogram_version() >= CS_TESTPROGRAM_VERSION_TEMPSENS_THRESHOLD) {
                        hw_gpadc_store_ambient_calibration_point(val_lo, val_hi);
                }
                break;
        default:
                break;
        }
#endif
}
#endif /* (dg_configUSE_SYS_TCS == 1) */
