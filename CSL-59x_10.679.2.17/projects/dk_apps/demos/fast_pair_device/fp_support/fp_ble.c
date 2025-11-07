/**
 ****************************************************************************************
 *
 * @file fp_ble.c
 *
 * @brief Google Fast Pair and FMDN BLE operations implementation
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
#include <string.h>
#include "fast_pair_device_config.h"

#if (ADV_CONTROL_MULT_EVENTS_ENABLE == 1)
#include "sdk_defs.h"
#include "osal.h"
#include "ble_gap.h"
#include "ble_common.h"
#include "adv_control.h"

#include "fp_ble.h"

/* Advertising event used for Google Fast Pair and FMDN */
__FP_RETAINED static adv_control_event_t fp_adv_evt;

void fp_ble_adv_init(void)
{
        fp_adv_evt = adv_control_event_create(NULL);
        OS_ASSERT(fp_adv_evt);
}

void fp_ble_adv_deinit(void)
{
        ble_error_t ret = adv_control_event_delete(fp_adv_evt);
        OS_ASSERT(ret == BLE_STATUS_OK);

        fp_adv_evt = NULL;
}

ble_error_t fp_ble_adv_set_params(const fp_ble_adv_params_t *params)
{
        adv_control_event_params_t adv_params = {
                .mode = params->mode,
                .type = params->type,
                .intv_min = params->intv_min,
                .intv_max = params->intv_max,
                .tx_power = params->tx_power
        };

        return adv_control_event_set_params(fp_adv_evt, &adv_params);
}

ble_error_t fp_ble_adv_set_tx_power(gap_tx_power_t tx_power)
{
        return adv_control_event_set_tx_power(fp_adv_evt, tx_power);
}

ble_error_t fp_ble_adv_set_ad_struct(size_t ad_len, const gap_adv_ad_struct_t *ad,
        size_t sd_len, const gap_adv_ad_struct_t *sd)
{
        return adv_control_event_set_ad_struct(fp_adv_evt, ad_len, ad, sd_len, sd);
}

ble_error_t fp_ble_adv_start(void)
{
        return adv_control_event_start(fp_adv_evt);
}

bool fp_ble_adv_is_started(void)
{
        return adv_control_event_is_active(fp_adv_evt);
}

ble_error_t fp_ble_adv_stop(void)
{
        return adv_control_event_stop(fp_adv_evt);
}

ble_error_t fp_ble_adv_stop_all(void)
{
        return adv_control_event_stop_all();
}
#endif /* ADV_CONTROL_MULT_EVENTS_ENABLE */
