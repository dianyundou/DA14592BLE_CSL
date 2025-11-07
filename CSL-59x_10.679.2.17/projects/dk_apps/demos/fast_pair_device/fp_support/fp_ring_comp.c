/**
 ****************************************************************************************
 *
 * @file fp_ring_comp.c
 *
 * @brief Google Fast Pair ringing components implementation
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

#include <stdint.h>
#include "fast_pair_device_config.h"
#include "led_control.h"
#include "fp_ring_comp.h"

#if (FP_FMDN == 1)

#if (FP_FMDN_RING_COMPONENTS_NUM > 0)
void fp_ring_comp_init(void)
{
        return;
}

void fp_ring_comp_deinit(void)
{
        return;
}

int fp_ring_comp_set_state(uint8_t comp_en_msk, FP_RING_COMP_VOLUME volume)
{
        static const LED_CONTROL_BLINK_RATE led_blink_rates[FP_RING_COMP_VOLUME_HIGH + 1] = {
                LED_CONTROL_BLINK_RATE_DEFAULT,
                LED_CONTROL_BLINK_RATE_LOW,
                LED_CONTROL_BLINK_RATE_MEDIUM,
                LED_CONTROL_BLINK_RATE_HIGH
        };

        /* Start or stop LED blinking */
        led_control_set_mode((comp_en_msk == FP_RING_COMP_SINGLE) ?
                        LED_CONTROL_MODE_START_BLINK : LED_CONTROL_MODE_SET_OFF,
                        led_blink_rates[volume], LED_CONTROL_BLINK_FOREVER);

        return 0;
}
#endif /* FP_FMDN_RING_COMPONENTS_NUM */

#endif /* FP_FMDN */
