/**
 ****************************************************************************************
 *
 * @file tps.c
 *
 * @brief Tx Power Service sample implementation
 *
 * Copyright (C) 2015-2018 Renesas Electronics Corporation and/or its affiliates.
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
#if defined(CONFIG_USE_BLE_SERVICES)

#include <stdbool.h>
#include <stddef.h>
#include "osal.h"
#include "ble_att.h"
#include "ble_bufops.h"
#include "ble_common.h"
#include "ble_gatts.h"
#include "ble_uuid.h"
#include "tps.h"

#define UUID_TX_POWER_LEVEL     (0x2A07)

typedef struct {
        ble_service_t svc;
} tp_service_t;

static void cleanup(ble_service_t *svc)
{
        OS_FREE(svc);
}

ble_service_t *tps_init(int8_t level)
{
        tp_service_t *tps;
        uint16_t num_attr;
        uint16_t tpl_val_h;
        att_uuid_t uuid;

        tps = OS_MALLOC(sizeof(*tps));
        memset(tps, 0, sizeof(*tps));

        num_attr = ble_gatts_get_num_attr(0, 1, 0);

        ble_uuid_create16(UUID_SERVICE_TPS, &uuid);
        ble_gatts_add_service(&uuid, GATT_SERVICE_PRIMARY, num_attr);

        ble_uuid_create16(UUID_TX_POWER_LEVEL, &uuid);
        ble_gatts_add_characteristic(&uuid, GATT_PROP_READ, ATT_PERM_READ, sizeof(int8_t), 0,
                                                                                NULL, &tpl_val_h);

        ble_gatts_register_service(&tps->svc.start_h, &tpl_val_h, 0);

        ble_gatts_set_value(tpl_val_h, sizeof(level), &level);

        tps->svc.cleanup = cleanup;
        tps->svc.end_h = tps->svc.start_h + num_attr;

        ble_service_add(&tps->svc);

        return &tps->svc;
}

#endif /* defined(CONFIG_USE_BLE_SERVICES) */
