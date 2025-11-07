/**
 ****************************************************************************************
 *
 * @file afmn_motion_detector.c
 *
 * @brief Apple FMN motion detector implementation
 *
 * Copyright (C) 2024 Renesas Electronics Corporation and/or its affiliates.
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
#include "sdk_defs.h"
#include "accessory_config.h"
#include "osal.h"
#include "led_control.h"

#include "afmn_motion_detector.h"
#include "afmn_motion_detector_ext.h"

/* Pointer to callback called when motion is detected */
__RETAINED static afmn_motion_detector_cb motion_detected_cb;
/* Motion detector is enabled */
__RETAINED static bool motion_detector_enabled;

void afmn_motion_detector_init(const afmn_motion_detector_config_t *cfg)
{
        motion_detected_cb = cfg->cb;
}

void afmn_motion_detector_deinit(void)
{
        return;
}

void afmn_motion_detector_enable(void)
{
        motion_detector_enabled = true;
        return;
}

void afmn_motion_detector_disable(void)
{
        motion_detector_enabled = false;
        return;
}

void afmn_motion_detector_ext_set_motion_detected(void)
{
        if (motion_detector_enabled && (motion_detected_cb != NULL)) {
                FMNA_TASK_PRINTF("Motion detected\r\n");
                if (led_control_get_mode() == LED_CONTROL_MODE_BLINK) {
                        led_control_set_mode(LED_CONTROL_MODE_BLINK, 0);
                        OS_DELAY_MS(SHORT_PRESS_TIMEOUT_MS);
                        led_control_set_mode(LED_CONTROL_MODE_BLINK, 0);
                } else {
                        led_control_set_mode(LED_CONTROL_MODE_START_BLINK, 2);
                }

                motion_detected_cb();
        }
}
