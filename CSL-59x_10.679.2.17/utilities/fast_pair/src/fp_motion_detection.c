/**
 ****************************************************************************************
 *
 * @file fp_motion_detector.c
 *
 * @brief Google Fast Pair FMDN motion detection implementation
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
#include <stdbool.h>
#include <stdlib.h>
#include <osal.h>
#include "fp_utils.h"
#include "fp_motion_detector.h"
#include "fp_notifications.h"
#include "fp_core.h"

#include "fp_motion_detection.h"

#if (FP_FMDN == 1)

#define SEPARATED_UT_MOTION_SAMPLING_RATE1_MS   (10 * 1000)        /* 10 seconds */
#define SEPARATED_UT_MOTION_SAMPLING_RATE2_MS   (500)              /* 0.5 seconds */
#define SEPARATED_UT_MOTION_BACKOFF_MS          (6 * 60 * 60000)   /* 6 hours */
#define SEPARATED_UT_TIMEOUT_MIN_MS             (8 * 60 * 60000)   /* 8 hours */
#define SEPARATED_UT_TIMEOUT_MAX_MS             (24 * 60 * 60000)  /* 24 hours */
#define SOUND_PLAYED_COUNT_MAX                  (10)
#define ACTIVE_DETECTION_DURARION_MS            (20 * 1000)        /* 20 seconds */

/* FMDN motion detection module context */
__FP_RETAINED static struct {
        ACCURATE_OS_TIMER enabling_tim;
        OS_TIMER detection_tim;
        OS_TIMER active_detection_tim;
        bool is_motion_detected;
        bool is_motion_detector_enabled;
        uint8_t sound_played_count;
        bool is_detection_period_decreased;
        bool is_active_detection_stopped;
} mos_det_ctx;

/* Motion detector enable callback */
static void enable_cb(ACCURATE_OS_TIMER timer)
{
        fp_send_notification(MOTION_DETECTOR_ENABLE_NOTIF);
}

/* Active motion detection period callback */
static void detection_cb(OS_TIMER timer)
{
        if (mos_det_ctx.is_motion_detected) {
                fp_send_notification(MOTION_DETECTED_NOTIF);
                mos_det_ctx.sound_played_count++;
        }
}

/* Motion detector active detection period end callback */
static void active_detection_cb(OS_TIMER timer)
{
        fp_send_notification(MOTION_DETECTOR_BACKOFF_NOTIF);
}

void fp_motion_detection_disable_detector(void)
{
        FP_FMDN_LOG_PRINTF("Motion detection backoff\r\n");
        mos_det_ctx.is_active_detection_stopped = true;
        OS_TIMER_STOP(mos_det_ctx.detection_tim, OS_TIMER_FOREVER);
        mos_det_ctx.is_detection_period_decreased = false;
        fp_motion_detector_disable();
        ACCURATE_OS_TIMER_CHANGE_PERIOD(mos_det_ctx.enabling_tim, SEPARATED_UT_MOTION_BACKOFF_MS);
}

void fp_motion_detection_restart(void)
{
        if (mos_det_ctx.is_active_detection_stopped == false) {
                if (mos_det_ctx.sound_played_count >= SOUND_PLAYED_COUNT_MAX) {
                        FP_FMDN_LOG_PRINTF("Max sounds played\r\n");
                        OS_TIMER_STOP(mos_det_ctx.active_detection_tim, OS_TIMER_FOREVER);
                        fp_motion_detection_disable_detector();
                } else {
                        mos_det_ctx.is_motion_detected = false;
                        if (!mos_det_ctx.is_detection_period_decreased) {
                                FP_FMDN_LOG_PRINTF("Decreasing motion detection period\r\n");
                                mos_det_ctx.is_detection_period_decreased = true;
                                OS_TIMER_CHANGE_PERIOD(mos_det_ctx.detection_tim,
                                        OS_MS_2_TICKS(SEPARATED_UT_MOTION_SAMPLING_RATE2_MS),
                                        OS_TIMER_FOREVER);
                                OS_TIMER_START(mos_det_ctx.active_detection_tim, OS_TIMER_FOREVER);
                        }
                }
        }
}

/* Motion detected callback */
static void motion_detected_cb(void)
{
        mos_det_ctx.is_motion_detected = true;
}

int fp_motion_detection_init(void)
{
        memset(&mos_det_ctx, 0, sizeof(mos_det_ctx));

        mos_det_ctx.enabling_tim = ACCURATE_OS_TIMER_CREATE(SEPARATED_UT_TIMEOUT_MIN_MS, false,
                                        enable_cb);
        OS_ASSERT(mos_det_ctx.enabling_tim);
        mos_det_ctx.detection_tim = OS_TIMER_CREATE("detect",
                                OS_MS_2_TICKS(SEPARATED_UT_MOTION_SAMPLING_RATE1_MS),
                                OS_TIMER_RELOAD, NULL, detection_cb);
        OS_ASSERT(mos_det_ctx.detection_tim);
        mos_det_ctx.active_detection_tim = OS_TIMER_CREATE("adetect",
                                OS_MS_2_TICKS(ACTIVE_DETECTION_DURARION_MS),
                                OS_TIMER_ONCE, NULL, active_detection_cb);
        OS_ASSERT(mos_det_ctx.active_detection_tim);

        const fp_motion_detector_config_t cfg = {
                .cb = motion_detected_cb
        };
        fp_motion_detector_init(&cfg);

        return 0;
}

void fp_motion_detection_deinit(void)
{
        OS_TIMER_DELETE(mos_det_ctx.active_detection_tim, OS_TIMER_FOREVER);
        OS_TIMER_DELETE(mos_det_ctx.detection_tim, OS_TIMER_FOREVER);
        ACCURATE_OS_TIMER_DELETE(mos_det_ctx.enabling_tim);

        fp_motion_detector_deinit();
}

void fp_motion_detection_start(void)
{
        uint32_t range_ms = SEPARATED_UT_TIMEOUT_MAX_MS - SEPARATED_UT_TIMEOUT_MIN_MS;
        uint32_t timeout_ms;

        timeout_ms = SEPARATED_UT_TIMEOUT_MIN_MS + (rand() % range_ms);
        ACCURATE_OS_TIMER_CHANGE_PERIOD(mos_det_ctx.enabling_tim, timeout_ms);
}

void fp_motion_detection_stop(void)
{
        OS_TIMER_STOP(mos_det_ctx.active_detection_tim, OS_TIMER_FOREVER);
        OS_TIMER_STOP(mos_det_ctx.detection_tim, OS_TIMER_FOREVER);
        ACCURATE_OS_TIMER_STOP(mos_det_ctx.enabling_tim);

        fp_motion_detector_disable();
}

void fp_motion_detection_enable_detector(void)
{
        mos_det_ctx.is_motion_detector_enabled = true;
        mos_det_ctx.sound_played_count = 0;
        mos_det_ctx.is_detection_period_decreased = false;
        mos_det_ctx.is_active_detection_stopped = false;
        mos_det_ctx.is_motion_detected = false;
        fp_motion_detector_enable();
        OS_TIMER_CHANGE_PERIOD(mos_det_ctx.detection_tim,
                OS_MS_2_TICKS(SEPARATED_UT_MOTION_SAMPLING_RATE1_MS), OS_TIMER_FOREVER);
}

/*
 * Define Google Fast Pair framework motion detector API functions as empty in case no motion
 * detector is needed
 */

__WEAK void fp_motion_detector_init(const fp_motion_detector_config_t *cfg)
{
        (void) cfg;

        return;
}

__WEAK void fp_motion_detector_deinit(void)
{
        return;
}

__WEAK void fp_motion_detector_enable(void)
{
        return;
}

__WEAK void fp_motion_detector_disable(void)
{
        return;
}
#endif /* FP_FMDN */
