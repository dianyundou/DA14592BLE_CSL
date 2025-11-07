/**
 ****************************************************************************************
 *
 * @file afmn_motion_detector.h
 *
 * @brief Apple FMN motion detector header file
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

#ifndef AFMN_MOTION_DETECTOR_H_
#define AFMN_MOTION_DETECTOR_H_

/**
 * \brief Callback called when motion is detected
 */
typedef void (*afmn_motion_detector_cb)(void);

/**
 * \brief Apple FMN motion detector configuration
 */
typedef struct {
        afmn_motion_detector_cb cb;             /**< Motion detected callback */
} afmn_motion_detector_config_t;

/**
 * \brief Initialize Apple FMN motion detector
 *
 * This function is called by Apple FindMy Network ADK to initialize motion detector.
 *
 * \param [in] cfg motion detector configuration
 */
void afmn_motion_detector_init(const afmn_motion_detector_config_t *cfg);

/**
 * \brief De-initialize Apple FMN motion detector
 *
 * This function is called by Apple FindMy Network ADK to reset and free resources related to
 * motion detector.
 */
void afmn_motion_detector_deinit(void);

/**
 * \brief Enable Apple FMN motion detector
 *
 * This function is called by Apple FindMy Network ADK to enable motion detector.
 */
void afmn_motion_detector_enable(void);

/**
 * \brief Disable Apple FMN motion detector
 *
 * This function is called by Apple FindMy Network ADK to disable motion detector.
 */
void afmn_motion_detector_disable(void);

#endif /* AFMN_MOTION_DETECTOR_H_ */
