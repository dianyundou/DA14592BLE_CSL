/**
 ****************************************************************************************
 *
 * @file fp_motion_detector.c
 *
 * @brief Google Fast Pair FMDN motion detector implementation
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

#include <stdio.h>
#include "fast_pair_device_config.h"

#if (FP_FMDN == 1)

#include "osal.h"
#include "led_control.h"

#include "fp_motion_detector.h"
#include "fp_motion_detector_ext.h"

/* Pointer to callback called when motion is detected */
__FP_RETAINED static fp_motion_detector_cb motion_detected_cb;
/* Motion detector is enabled */
__FP_RETAINED static bool motion_detector_enabled;

void fp_motion_detector_init(const fp_motion_detector_config_t *cfg)
{
        motion_detected_cb = cfg->cb;
}

void fp_motion_detector_deinit(void)
{
        return;
}

void fp_motion_detector_enable(void)
{
        motion_detector_enabled = true;
        return;
}

void fp_motion_detector_disable(void)
{
        motion_detector_enabled = false;
        return;
}

void fp_motion_detector_ext_set_motion_detected(void)
{
        if (motion_detector_enabled && (motion_detected_cb != NULL)) {
                FP_TASK_PRINTF("Motion detected\r\n");
                if (led_control_get_mode() == LED_CONTROL_MODE_BLINK) {
                        led_control_set_mode(LED_CONTROL_MODE_BLINK, 0, 0);
                        OS_DELAY_MS(SHORT_PRESS_TIMEOUT_MS);
                        led_control_set_mode(LED_CONTROL_MODE_BLINK, 0, 0);
                } else {
                        led_control_set_mode(LED_CONTROL_MODE_START_BLINK,
                                LED_CONTROL_BLINK_RATE_DEFAULT, 2);
                }
                motion_detected_cb();
        }
}
#endif /* FP_FMDN */
