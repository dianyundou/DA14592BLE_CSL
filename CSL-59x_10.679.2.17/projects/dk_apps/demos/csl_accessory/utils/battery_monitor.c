/**
 ****************************************************************************************
 *
 * @file battery_monitor.c
 *
 * @brief Battery level monitoring implementation
 *
 * Copyright (C) 2025 Renesas Electronics Corporation and/or its affiliates.
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
#include <stdbool.h>
#include "sdk_defs.h"
#include "osal.h"
#include "notification_bits.h"
#include "platform_devices.h"
#include "ad_gpadc.h"
#include "accessory_config.h"

#if (BATTERY_MONITOR_INTERVAL_MS > 0)

#include "battery_monitor.h"

/* Battery monitoring sampling interval (in msec) */
#ifndef BATTERY_MONITOR_INTERVAL_MS
#define BATTERY_MONITOR_INTERVAL_MS     ( 5 * 60 * 1000 )
#endif

/* Battery voltage level (in mV) for the battery to be considered fully charged */
#ifndef BATTERY_MONITOR_FULL_LEVEL_MV
#define BATTERY_MONITOR_FULL_LEVEL_MV   ( 3000 )
#endif
/* Battery voltage level (in mV) for the battery to be considered empty */
#ifndef BATTERY_MONITOR_EMPTY_LEVEL_MV
#define BATTERY_MONITOR_EMPTY_LEVEL_MV  ( 1700 )
#endif

/* Timer used for monitoring battery level */
__RETAINED static OS_TIMER batt_monitor_tim;
/* Control OS task handle */
__RETAINED static OS_TASK batt_task_hdl;
/* Battery level (range 0 - 100) */
__RETAINED static uint8_t batt_level;

/* Convert battery voltage level to battery level range 0 - 100 */
static uint8_t batt_conv_mV_to_level(uint16_t voltage)
{
        if (voltage >= BATTERY_MONITOR_FULL_LEVEL_MV) {
                return 100;
        } else if (voltage <= BATTERY_MONITOR_EMPTY_LEVEL_MV) {
                return 0;
        }

        /*
         * For demonstration purposes discharging (Voltage vs. Capacity) is approximated
         * by a linear function. The exact formula depends on the specific battery being used.
         */
        return (uint8_t) ((int) (voltage - BATTERY_MONITOR_EMPTY_LEVEL_MV) * 100 /
                          (BATTERY_MONITOR_FULL_LEVEL_MV - BATTERY_MONITOR_EMPTY_LEVEL_MV));
}

/* Read battery level (range 0 - 100) */
static uint8_t batt_read_level(void)
{
        uint16_t batt_lvl_mV;
        uint16_t value;
        ad_gpadc_handle_t handle;
        const ad_gpadc_driver_conf_t *batt_lvl_drv =
                ((ad_gpadc_controller_conf_t *) BATTERY_LEVEL)->drv;

        handle = ad_gpadc_open(BATTERY_LEVEL);
        ad_gpadc_read_nof_conv(handle, 1, &value);
        batt_lvl_mV = ad_gpadc_conv_raw_to_batt_mvolt(batt_lvl_drv, value);
        ad_gpadc_close(handle, false);

        return batt_conv_mV_to_level(batt_lvl_mV);
}

/* Battery monitoring timer callback */
static void batt_monitor_tim_cb(OS_TIMER timer) {
        OS_TASK_NOTIFY(batt_task_hdl, BATTERY_MONITOR_NOTIF, OS_NOTIFY_SET_BITS);
}

void battery_monitor_init(OS_TASK task_hdl)
{
        OS_ASSERT(task_hdl);

        batt_task_hdl = task_hdl;

        batt_level = batt_read_level();

        /* Create timer to implement periodic battery monitoring */
        batt_monitor_tim = OS_TIMER_CREATE("batt", OS_MS_2_TICKS(BATTERY_MONITOR_INTERVAL_MS),
                                OS_TIMER_RELOAD, NULL, batt_monitor_tim_cb);
        OS_TIMER_START(batt_monitor_tim, OS_TIMER_FOREVER);
}

uint8_t battery_monitor_get_level(void)
{
        return batt_level;
}

void battery_monitor_process_notif(uint32_t notif)
{
    if (notif & BATTERY_MONITOR_NOTIF) {
            batt_level = batt_read_level();
    }
}

#endif /* BATTERY_MONITOR_INTERVAL_MS */
