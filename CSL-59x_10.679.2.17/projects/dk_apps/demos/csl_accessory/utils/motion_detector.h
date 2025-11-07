/**
 ****************************************************************************************
 *
 * @file motion_detector.h
 *
 * @brief Motion detector header file
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

#ifndef MOTION_DETECTOR_H_
#define MOTION_DETECTOR_H_

/**
 * \brief Handle of motion detector instance
 */
typedef void *motion_detector_handle_t;

/**
 * \brief Callback called when motion is detected
 */
typedef void (*motion_detector_cb_t)(void);

/**
 * \brief Motion detector configuration
 */
typedef struct {
        motion_detector_cb_t cb;        /**< Motion detected callback */
} motion_detector_config_t;

/**
 * \brief Initialize motion detector
 *
 * This function is called to initialize a motion detector instance.
 *
 * \param [in] cfg configuration of motion detector
 *
 * \return handle of the motion detector instance if it is created successfully, otherwise NULL
 */
motion_detector_handle_t motion_detector_init(const motion_detector_config_t *cfg);

/**
 * \brief De-initialize motion detector
 *
 * This function is called to reset and free resources related to a motion detector instance.
 *
 * \param [in] motion_detector handle of the motion detector instance
 */
void motion_detector_deinit(motion_detector_handle_t motion_detector);

/**
 * \brief Enable motion detector
 *
 * This function is called to enable motion detector.
 *
 * \param [in] motion_detector handle of the motion detector instance
 */
void motion_detector_enable(motion_detector_handle_t motion_detector);

/**
 * \brief Disable motion detector
 *
 * This function is called to disable motion detector.
 *
 * \param [in] motion_detector handle of the motion detector instance
 */
void motion_detector_disable(motion_detector_handle_t motion_detector);

/**
 * \brief Indicate that motion is detected
 *
 * This function shall be called in order to indicate that motion has been detected.
 */
void motion_detector_set_motion_detected(void);

#endif /* MOTION_DETECTOR_H_ */
