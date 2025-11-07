/**
 ****************************************************************************************
 *
 * @file fp_ring_comp.h
 *
 * @brief Google Fast Pair ringing components header file
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

#ifndef FP_RING_COMP_H_
#define FP_RING_COMP_H_

#include <stdint.h>
#include FAST_PAIR_CONFIG_FILE
#include "fp_defaults.h"

#if (FP_FMDN == 1)

/**
 * \brief Fast Pair ringing volume
 */
typedef enum {
        FP_RING_COMP_VOLUME_DEFAULT,
        FP_RING_COMP_VOLUME_LOW,
        FP_RING_COMP_VOLUME_MEDIUM,
        FP_RING_COMP_VOLUME_HIGH
} FP_RING_COMP_VOLUME;

/**
 * \brief Fast Pair ringing components
 */
typedef enum {
#if (FP_FMDN_RING_COMPONENTS_NUM == 1)
        FP_RING_COMP_SINGLE             = 0x1
#else
        FP_RING_COMP_RIGHT_BUD          = 0x1,
        FP_RING_COMP_LEFT_BUD           = 0x2,
        FP_RING_COMP_CASE               = 0x4
#endif
} FP_RING_COMP;

#if (FP_FMDN_RING_COMPONENTS_NUM > 0)
/**
 * \brief Initialize Fast Pair ringing components
 *
 * This function is called by Google Fast Pair framework to initialize ringing components.
 */
void fp_ring_comp_init(void);

/**
 * \brief De-initialize Fast Pair ringing components
 *
 * This function is called by Google Fast Pair framework to release allocated resources for
 * ringing components.
 */
void fp_ring_comp_deinit(void);

/**
 * \brief Set Fast Pair ringing components state
 *
 * \param [in] comp_en_msk bitmask of enabled ringing components (FP_RING_COMP)
 * \param [in] volume ringing volume
 *
 * \return 0 if new ringing components state was applied successfully, other value otherwise
 */
int fp_ring_comp_set_state(uint8_t comp_en_msk, FP_RING_COMP_VOLUME volume);
#endif /* FP_FMDN_RING_COMPONENTS_NUM */

#endif /* FP_FMDN */

#endif /* FP_RING_COMP_H_ */
