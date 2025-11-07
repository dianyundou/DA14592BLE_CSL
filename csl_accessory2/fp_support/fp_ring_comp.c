/**
 ****************************************************************************************
 *
 * @file fp_ring_comp.c
 *
 * @brief Google Fast Pair ringing components implementation
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

#include <stdint.h>
#include "accessory_config.h"
#include "sdk_defs.h"
#include "osal.h"
#include "sound_maker.h"

#include "fp_ring_comp.h"

#if (FP_FMDN == 1)

#if (FP_FMDN_RING_COMPONENTS_NUM > 0)
/* Handle of sound maker instance for Google Fast Pair framework */
__FP_RETAINED static sound_maker_handle_t fp_sound_maker_hdl;

void fp_ring_comp_init(void)
{
        fp_sound_maker_hdl = sound_maker_init();
        OS_ASSERT(fp_sound_maker_hdl);
}

void fp_ring_comp_deinit(void)
{
        sound_maker_deinit(fp_sound_maker_hdl);
}

int fp_ring_comp_set_state(uint8_t comp_en_msk, FP_RING_COMP_VOLUME volume)
{
        static const SOUND_MAKER_VOLUME sound_mkr_vol[FP_RING_COMP_VOLUME_HIGH + 1] = {
                SOUND_MAKER_VOLUME_DEFAULT,
                SOUND_MAKER_VOLUME_LOW,
                SOUND_MAKER_VOLUME_MEDIUM,
                SOUND_MAKER_VOLUME_HIGH
        };

        /* Start or stop sound maker */
        if (comp_en_msk == FP_RING_COMP_SINGLE) {
                sound_maker_enable(fp_sound_maker_hdl, sound_mkr_vol[volume]);
        } else {
                sound_maker_disable(fp_sound_maker_hdl);
        }

        return 0;
}
#endif /* FP_FMDN_RING_COMPONENTS_NUM */

#endif /* FP_FMDN */
