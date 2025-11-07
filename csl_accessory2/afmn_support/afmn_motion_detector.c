/**
 ****************************************************************************************
 *
 * @file afmn_motion_detector.c
 *
 * @brief Apple FMN motion detector implementation
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

#include "sdk_defs.h"
#include "osal.h"
#include "motion_detector.h"

#include "afmn_motion_detector.h"

/* Handle of motion detector instance for Apple FMN */
__RETAINED static motion_detector_handle_t afmn_motion_detector_hdl;

void afmn_motion_detector_init(const afmn_motion_detector_config_t *cfg)
{
        const motion_detector_config_t motion_det_cfg = {
                .cb = (motion_detector_cb_t) cfg->cb,
        };
        afmn_motion_detector_hdl = motion_detector_init(&motion_det_cfg);
        OS_ASSERT(afmn_motion_detector_hdl);
}

void afmn_motion_detector_deinit(void)
{
        motion_detector_deinit(afmn_motion_detector_hdl);
}

void afmn_motion_detector_enable(void)
{
        motion_detector_enable(afmn_motion_detector_hdl);
}

void afmn_motion_detector_disable(void)
{
        motion_detector_disable(afmn_motion_detector_hdl);
}

