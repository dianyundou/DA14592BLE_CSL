/**
 ****************************************************************************************
 *
 * @file lls.c
 *
 * @brief Link Loss Service sample implementation
 *
 * Copyright (C) 2015-2023 Renesas Electronics Corporation and/or its affiliates.
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
#include "sdk_queue.h"
#include "ble_att.h"
#include "ble_bufops.h"
#include "ble_common.h"
#include "ble_gatts.h"
#include "ble_uuid.h"
#include "lls.h"

#define UUID_ALERT_LEVEL        (0x2A06)

typedef struct {
        ble_service_t svc;

        // handles
        uint16_t al_val_h;

        // callbacks
        lls_alert_cb_t cb;
        queue_t levels;
} ll_service_t;

typedef struct {
        void *next;

        uint16_t conn_idx;
        uint8_t level;
} conn_dev_t;

static bool conn_dev_conn_idx_match(const void *data, const void *match_data)
{
        conn_dev_t *conn_dev = (conn_dev_t *) data;
        uint16_t conn_idx = (*(uint16_t *) match_data);

        return conn_dev->conn_idx == conn_idx;
}

static void handle_disconnected_evt(ble_service_t *svc, const ble_evt_gap_disconnected_t *evt)
{
        ll_service_t *lls = (ll_service_t *) svc;

        if (evt->reason == CO_ERROR_NO_ERROR ||
            evt->reason == CO_ERROR_REMOTE_USER_TERM_CON ||
            evt->reason == CO_ERROR_CON_TERM_BY_LOCAL_HOST) {
                // do not fire callback if disconnection was triggered by either side
                return;
        }

        // fire callback with current Alert Level - app should trigger an alarm
        if (lls->cb) {
                uint8_t level = 0;
                conn_dev_t *conn_dev;

                conn_dev = queue_remove(&lls->levels, conn_dev_conn_idx_match, &evt->conn_idx);

                if (conn_dev) {
                        level = conn_dev->level;
                        OS_FREE(conn_dev);
                }

                lls->cb(evt->conn_idx, &evt->address, level);
        }
}

static void handle_read_req(ble_service_t *svc, const ble_evt_gatts_read_req_t *evt)
{
        ll_service_t *lls = (ll_service_t *) svc;
        conn_dev_t *conn_dev;
        uint8_t level;

        /* Default alert level - 'No Alert' */
        level = 0;

        conn_dev = queue_find(&lls->levels, conn_dev_conn_idx_match, &evt->conn_idx);

        if (conn_dev) {
                level = conn_dev->level;
        }

        ble_gatts_read_cfm(evt->conn_idx, evt->handle, ATT_ERROR_OK, sizeof(level), &level);
}

static void handle_write_req(ble_service_t *svc, const ble_evt_gatts_write_req_t *evt)
{
        ll_service_t *lls = (ll_service_t *) svc;
        att_error_t err = ATT_ERROR_OK;
        uint8_t level = get_u8(evt->value);

        if (evt->length == 1) {
                if (level > 2) {
                        err = ATT_ERROR_APPLICATION_ERROR;
                } else {
                        conn_dev_t *conn_dev = queue_find(&lls->levels, conn_dev_conn_idx_match, &evt->conn_idx);

                        if (!conn_dev) {
                                conn_dev = OS_MALLOC(sizeof(*conn_dev));
                                conn_dev->conn_idx = evt->conn_idx;
                                queue_push_front(&lls->levels, conn_dev);
                        }

                        conn_dev->level = level;
                }
        }

        ble_gatts_write_cfm(evt->conn_idx, evt->handle, err);
}

static void cleanup(ble_service_t *svc)
{
        ll_service_t *lls = (ll_service_t *) svc;

        queue_remove_all(&lls->levels, OS_FREE_FUNC);
        OS_FREE(lls);
}

ble_service_t *lls_init(lls_alert_cb_t alert_cb)
{
        ll_service_t *lls;
        uint16_t num_attr;
        att_uuid_t uuid;

        lls = OS_MALLOC(sizeof(*lls));
        memset(lls, 0, sizeof(*lls));
        queue_init(&lls->levels);

        lls->svc.disconnected_evt = handle_disconnected_evt;
        lls->svc.read_req = handle_read_req;
        lls->svc.write_req = handle_write_req;
        lls->svc.cleanup = cleanup;
        lls->cb = alert_cb;

        num_attr = ble_gatts_get_num_attr(0, 1, 0);

        ble_uuid_create16(UUID_SERVICE_LLS, &uuid);
        ble_gatts_add_service(&uuid, GATT_SERVICE_PRIMARY, num_attr);

        ble_uuid_create16(UUID_ALERT_LEVEL, &uuid);
        ble_gatts_add_characteristic(&uuid, GATT_PROP_READ | GATT_PROP_WRITE, ATT_PERM_RW,
                                                        sizeof(uint8_t), GATTS_FLAG_CHAR_READ_REQ,
                                                        NULL, &lls->al_val_h);

        ble_gatts_register_service(&lls->svc.start_h, &lls->al_val_h, 0);

        lls->svc.end_h = lls->svc.start_h + num_attr;

        ble_service_add(&lls->svc);

        return &lls->svc;
}

#endif /* defined(CONFIG_USE_BLE_SERVICES) */
