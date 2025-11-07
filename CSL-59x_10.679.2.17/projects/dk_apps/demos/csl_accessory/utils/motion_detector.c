/**
 ****************************************************************************************
 *
 * @file motion_detector.c
 *
 * @brief Motion detector implementation
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
#include "accessory_config.h"
#include "sdk_defs.h"
#include "osal.h"
#include "led_control.h"

#include "motion_detector.h"

/* Maximum number of motion detector instances that can be handled */
#ifndef MOTION_DETECTORS_COUNT_MAX
#define MOTION_DETECTORS_COUNT_MAX       (2)
#endif

/* Motion detector command types */
typedef enum {
        MOTION_DETECTOR_CMD_ENABLE,
        MOTION_DETECTOR_CMD_DISABLE,
        MOTION_DETECTOR_CMD_DELETE
} MOTION_DETECTOR_CMD;

/* Motion detector instances */
__RETAINED static struct motion_detector_t {
        motion_detector_cb_t cb;
        bool enabled;
} motion_detectors[MOTION_DETECTORS_COUNT_MAX];

/* Motion detector is enabled */
__RETAINED static bool motion_detector_enabled;

/* Returns pointer to free motion detector instance entry */
static struct motion_detector_t *search_for_free_motion_detector(void)
{
        for (int i = 0; i < MOTION_DETECTORS_COUNT_MAX; i++) {
                if (motion_detectors[i].cb == NULL) {
                        return &motion_detectors[i];
                }
        }

        return NULL;
}

/* Updates the state of motion detector */
static void update_motion_detector(motion_detector_handle_t motion_detector, MOTION_DETECTOR_CMD cmd)
{
        struct motion_detector_t *motion_det = (struct motion_detector_t *) motion_detector;
        bool motion_det_en = false;

        if (motion_det->cb == NULL) {
                return;
        }

        switch (cmd) {
        case MOTION_DETECTOR_CMD_ENABLE:
                motion_det->enabled = true;
                break;
        case MOTION_DETECTOR_CMD_DISABLE:
                motion_det->enabled = false;
                break;
        case MOTION_DETECTOR_CMD_DELETE:
                motion_det->cb = NULL;
                motion_det->enabled = false;
                break;
        }

        /* Enable motion detector if at least one instance is enabled */
        for (int i = 0; i < MOTION_DETECTORS_COUNT_MAX; i++) {
                motion_det = &motion_detectors[i];
                if (motion_det->cb != NULL && motion_det->enabled) {
                        motion_det_en = true;
                        break;
                }
        }

        motion_detector_enabled = motion_det_en;
}

motion_detector_handle_t motion_detector_init(const motion_detector_config_t *cfg)
{
        struct motion_detector_t *motion_det;

        OS_ASSERT(cfg);
        OS_ASSERT(cfg->cb);

        motion_det = search_for_free_motion_detector();

        if (motion_det == NULL) {
                return NULL;
        }

        motion_det->cb = cfg->cb;
        motion_det->enabled = false;

        return motion_det;
}

void motion_detector_deinit(motion_detector_handle_t motion_detector)
{
        update_motion_detector(motion_detector, MOTION_DETECTOR_CMD_DELETE);
}

void motion_detector_enable(motion_detector_handle_t motion_detector)
{
        update_motion_detector(motion_detector, MOTION_DETECTOR_CMD_ENABLE);
}

void motion_detector_disable(motion_detector_handle_t motion_detector)
{
        update_motion_detector(motion_detector, MOTION_DETECTOR_CMD_DISABLE);
}

void motion_detector_set_motion_detected(void)
{
        if (motion_detector_enabled) {
                CSLA_TASK_PRINTF("Motion detected\r\n");

                /* Indicate that moation has been detected by blinking the LED */
                if (led_control_get_mode() == LED_CONTROL_MODE_BLINK) {
                        led_control_set_mode(LED_CONTROL_MODE_BLINK, 0, 0);
                        OS_DELAY_MS(SHORT_PRESS_TIMEOUT_MS);
                        led_control_set_mode(LED_CONTROL_MODE_BLINK, 0, 0);
                } else {
                        led_control_set_mode(LED_CONTROL_MODE_START_BLINK,
                                LED_CONTROL_BLINK_RATE_DEFAULT, 2);
                }

                /* Trigger the registered callback of each motion detector instance */
                for (int i = 0; i < MOTION_DETECTORS_COUNT_MAX; i++) {
                        struct motion_detector_t *motion_det = &motion_detectors[i];
                        if (motion_det->enabled) {
                                motion_det->cb();
                        }
                }
        }
}
