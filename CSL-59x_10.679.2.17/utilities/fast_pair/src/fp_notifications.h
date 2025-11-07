/**
 ****************************************************************************************
 *
 * @file fp_notification.h
 *
 * @brief Google Fast Pair framework task notifications bits
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

#ifndef FP_NOTIFICATIONS_H_
#define FP_NOTIFICATIONS_H_

#include FAST_PAIR_CONFIG_FILE
#include "fp_defaults.h"

/*
 * Notification bits reservation
 */
#if (FP_UTILS_CORR_OS_TIMERS == 1)
#define CORR_OS_TIMER_TRIGGER_NOTIF     (1 << 1)
#endif
#define KEY_TMO_NOTIF                   (1 << 2)
#if (FP_FMDN == 1)
#define ID_ROTATION_TMO_NOTIF           (1 << 3)
#define ADV_TMO_NOTIF                   (1 << 4)
#define UTPM_TMO_NOTIF                  (1 << 5)
#define FMDN_PROVISIONING_TMO_NOTIF     (1 << 6)
#define USER_CONSENT_TMO_NOTIF          (1 << 7)
#if (FP_FMDN_RING_COMPONENTS_NUM > 0)
#define RING_TMO_NOTIF                  (1 << 8)
#endif
#define MOTION_DETECTOR_ENABLE_NOTIF    (1 << 9)
#define MOTION_DETECTED_NOTIF           (1 << 10)
#define MOTION_DETECTOR_BACKOFF_NOTIF   (1 << 11)
#else
#define RPA_ROTATION_TMO_NOTIF          (1 << 3)
#endif /* FP_FMDN */

#endif /* FP_NOTIFICATIONS_H_ */
