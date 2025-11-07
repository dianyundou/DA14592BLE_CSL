/**
 ****************************************************************************************
 *
 * @file fp_motion_detection.h
 *
 * @brief Google Fast Pair FMDN motion detection header file
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

#ifndef FP_MOTION_DETECTION_H_
#define FP_MOTION_DETECTION_H_

#include <stdint.h>
#include FAST_PAIR_CONFIG_FILE
#include "fp_defaults.h"

#if (FP_FMDN == 1)
/**
 * \brief Initialize FMDN motion detection module
 *
 * \return 0 if success, other value otherwise
 */
int fp_motion_detection_init(void);

/**
 * \brief De-initialize FMDN motion detection module
 *
 * This function releases allocated resources for FMDN motion detection module.
 */
void fp_motion_detection_deinit(void);

/**
 * \brief Start motion detection
 */
void fp_motion_detection_start(void);

/**
 * \brief Stop motion detection
 */
void fp_motion_detection_stop(void);

/**
 * \brief Enable motion detector
 */
void fp_motion_detection_enable_detector(void);

/**
 * \brief Disable motion detector
 */
void fp_motion_detection_disable_detector(void);

/**
 * \brief Restart motion detection
 *
 * This function is called when motion has been detected and playing sound is complete.
 */
void fp_motion_detection_restart(void);
#endif /* FP_FMDN */

#endif /* FP_MOTION_DETECTION_H_ */
