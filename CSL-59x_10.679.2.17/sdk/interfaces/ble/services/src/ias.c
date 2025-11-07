/**
 ****************************************************************************************
 *
 * @file ias.c
 *
 * @brief Immediate Alert Service sample implementation
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
#include <string.h>
#include "osal.h"
#include "ble_att.h"
#include "ble_bufops.h"
#include "ble_common.h"
#include "ble_gatts.h"
#include "ble_uuid.h"
#include "ias.h"

#define UUID_ALERT_LEVEL        (0x2A06)

typedef struct {
        ble_service_t svc;

        // handles
        uint16_t al_val_h;

        // callbacks
        ias_alert_level_cb_t cb;
} ia_service_t;

static void handle_write_req(ble_service_t *svc, const ble_evt_gatts_write_req_t *evt)
{
        ia_service_t *ias = (ia_service_t *) svc;
        att_error_t err = ATT_ERROR_OK;

        if (evt->length == 1) {
                uint8_t level;

                /*
                * This is write-only characteristic so we don't need to store value written
                * anywhere, can just handle it here and reply.
                */

                level = get_u8(evt->value);

                if (level > 2) {
                        err = ATT_ERROR_APPLICATION_ERROR;
                } else if (ias->cb) {
                        ias->cb(evt->conn_idx, level);
                }
        }

        ble_gatts_write_cfm(evt->conn_idx, evt->handle, err);
}

static void handle_disconnected_evt(ble_service_t *svc, const ble_evt_gap_disconnected_t *evt)
{
        ia_service_t *ias = (ia_service_t *) svc;

        if (ias->cb) {
                ias->cb(evt->conn_idx, 0);
        }
}

static void cleanup(ble_service_t *svc)
{
        OS_FREE(svc);
}

ble_service_t *ias_init(ias_alert_level_cb_t alert_level_cb)
{
        ia_service_t *ias;
        uint16_t num_attr;
        att_uuid_t uuid;

        ias = OS_MALLOC(sizeof(*ias));
        memset(ias, 0, sizeof(*ias));

        ias->svc.write_req = handle_write_req;
        ias->svc.disconnected_evt = handle_disconnected_evt;
        ias->svc.cleanup = cleanup;
        ias->cb = alert_level_cb;

        num_attr = ble_gatts_get_num_attr(0, 1, 0);

        ble_uuid_create16(UUID_SERVICE_IAS, &uuid);
        ble_gatts_add_service(&uuid, GATT_SERVICE_PRIMARY, num_attr);

        ble_uuid_create16(UUID_ALERT_LEVEL, &uuid);
        ble_gatts_add_characteristic(&uuid, GATT_PROP_WRITE_NO_RESP, ATT_PERM_WRITE,
                                                        sizeof(uint8_t), 0, NULL, &ias->al_val_h);

        ble_gatts_register_service(&ias->svc.start_h, &ias->al_val_h, 0);

        ias->svc.end_h = ias->svc.start_h + num_attr;

        ble_service_add(&ias->svc);

        return &ias->svc;
}

#endif /* defined(CONFIG_USE_BLE_SERVICES) */
