/**
 ****************************************************************************************
 *
 * @file led_control.c
 *
 * @brief LED control implementation
 *
 * Copyright (C) 2024-2025 Renesas Electronics Corporation and/or its affiliates.
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

#include <stdbool.h>
#include "sdk_defs.h"
#include "osal.h"
#include "hw_gpio.h"
#include "notification_bits.h"
#include "fast_pair_device_config.h"

#include "led_control.h"

#define LED_CONTROL_TOGGLES_FOREVER     ( 2 * (LED_CONTROL_BLINK_FOREVER - 1) )

/* LED control mode */
__RETAINED static LED_CONTROL_MODE led_mode;
/* Timer used to blink LED */
__RETAINED static OS_TIMER led_blink_tim;
/* Control OS task handle */
__RETAINED static OS_TASK led_task_hdl;
/* LED blink toggles remaining */
__RETAINED static uint32_t led_blink_toggles;

static void led_blink_tim_cb(OS_TIMER timer)
{
        OS_TASK_NOTIFY(led_task_hdl, LED_CONTROL_NOTIF, OS_NOTIFY_SET_BITS);
}

void led_control_init(OS_TASK task_hdl)
{
        OS_ASSERT(task_hdl);

        led_task_hdl = task_hdl;

        /* Create timer to implement LED blinking */
        led_blink_tim = OS_TIMER_CREATE("led", OS_MS_2_TICKS(LED_CONTROL_BLINK_RATE_DEFAULT),
                                OS_TIMER_ONCE, NULL, led_blink_tim_cb);
}

LED_CONTROL_MODE led_control_get_mode(void)
{
        return led_mode;
}

void led_control_set_mode(LED_CONTROL_MODE mode, LED_CONTROL_BLINK_RATE blink_rate_ms,
        uint16_t blink_cnt)
{
        HW_GPIO_SET_PIN_FUNCTION(LED1);

        switch (mode) {
        case LED_CONTROL_MODE_SET_OFF:
                /* no break */
        case LED_CONTROL_MODE_SET_ON:
                led_mode = mode;

                if (mode == LED_CONTROL_MODE_SET_ON) {
                        hw_gpio_set_active(LED1_PORT, LED1_PIN);
                } else {
                        hw_gpio_set_inactive(LED1_PORT, LED1_PIN);
                }

                /* Stop timer for LED blinking */
                OS_TIMER_STOP(led_blink_tim, OS_TIMER_FOREVER);
                break;
        case LED_CONTROL_MODE_START_BLINK:
                if (blink_cnt == 0) {
                        return;
                }

                if (blink_rate_ms == 0) {
                        blink_rate_ms = LED_CONTROL_BLINK_RATE_DEFAULT;
                }

                led_mode = LED_CONTROL_MODE_BLINK;
                led_blink_toggles = 2 * ((uint32_t) blink_cnt - 1);

                /* Restart corresponding timer for LED blinking */
                OS_TIMER_CHANGE_PERIOD(led_blink_tim, OS_MS_2_TICKS(blink_rate_ms),
                        OS_TIMER_FOREVER);
                /* no break */
        case LED_CONTROL_MODE_BLINK:
                if (led_mode != LED_CONTROL_MODE_BLINK) {
                        return;
                }

                hw_gpio_toggle(LED1_PORT, LED1_PIN);

                if (!OS_TIMER_IS_ACTIVE(led_blink_tim)) {
                        OS_TIMER_START(led_blink_tim, OS_TIMER_FOREVER);
                }
                break;
        default:
                /* Invalid LED control option */
                OS_ASSERT(0);
                break;
        }

        HW_GPIO_PAD_LATCH_ENABLE(LED1);
        HW_GPIO_PAD_LATCH_DISABLE(LED1);
}

void led_control_process_notif(uint32_t notif)
{
    if (notif & LED_CONTROL_NOTIF) {
            if (led_mode == LED_CONTROL_MODE_BLINK) {
                    if (led_blink_toggles == 0) {
                            led_control_set_mode(LED_CONTROL_MODE_SET_OFF, 0, 0);
                    } else {
                            if (led_blink_toggles != LED_CONTROL_TOGGLES_FOREVER) {
                                    led_blink_toggles--;
                            }
                            led_control_set_mode(LED_CONTROL_MODE_BLINK, 0, 0);
                    }
            }
    }
}
