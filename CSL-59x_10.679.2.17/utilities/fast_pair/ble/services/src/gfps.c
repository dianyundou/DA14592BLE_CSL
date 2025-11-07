/**
 ****************************************************************************************
 *
 * @file gfps.c
 *
 * @brief Google Fast Pair Service implementation
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

#if defined(CONFIG_USE_BLE_SERVICES)

#include <stdio.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include FAST_PAIR_CONFIG_FILE
#include "fp_defaults.h"
#include "osal.h"
#include "ble_att.h"
#include "ble_bufops.h"
#include "ble_common.h"
#include "ble_gatts.h"
#include "ble_uuid.h"
#include "ble_storage.h"
#include "gfps.h"
#include "fast_pair.h"

//#define DEBUG_GFPS

#ifdef DEBUG_GFPS
#define GFPS_PRINTF(fmt, ...) printf(fmt, ##__VA_ARGS__)
#else
#define GFPS_PRINTF(fmt, ...)
#endif

#define UUID_GFP_SERVICE                (0xFE2C)
#define UUID_GFP_MODEL_ID               "FE2C1233-8366-4814-8EB0-01DE32100BEA"
#define UUID_GFP_KEY_BASED_PAIRING      "FE2C1234-8366-4814-8EB0-01DE32100BEA"
#define UUID_GFP_PASS_KEY               "FE2C1235-8366-4814-8EB0-01DE32100BEA"
#define UUID_GFP_ACCOUNT_KEY            "FE2C1236-8366-4814-8EB0-01DE32100BEA"
#define UUID_GFP_ADDITIONAL_DATA        "FE2C1237-8366-4814-8EB0-01DE32100BEA"

#define REQUEST_LENGTH                  (16)
#define PUBLIC_KEY_LENGTH               (64)
#define ADDITIONAL_DATA_LENGTH          (16 + 64)

#if (FP_FMDN == 1)
#define UUID_GFP_BEACON_ACTIONS         "FE2C1238-8366-4814-8EB0-01DE32100BEA"
#define BEACON_ACTIONS_LENGTH           (10 + 32 + 8)
#define BEACON_ACTIONS_NONCE_LENGTH     (8)
#define BEACON_ACTIONS_READ_LENGTH      (1 + BEACON_ACTIONS_NONCE_LENGTH)
#endif /* FP_FMDN */

typedef struct {
        ble_service_t svc;

        // handles
#if (FP_LOCATOR_TAG != 1)
        uint16_t model_id_h;
#endif
        uint16_t key_pairing_h;
        uint16_t key_pairing_ccc_h;
        uint16_t pass_key_h;
        uint16_t pass_key_ccc_h;
        uint16_t account_key_h;
        uint16_t additional_data_h;
        uint16_t additional_data_ccc_h;
#if (FP_FMDN == 1)
        uint16_t beacon_actions_h;
        uint16_t beacon_actions_ccc_h;
#endif

        const fast_pair_callbacks_t *cb;
} gfp_service_t;

static att_error_t do_pairing_write(ble_service_t *svc, uint16_t conn_idx, uint16_t offset,
                uint16_t length, const uint8_t *value)
{
        gfp_service_t *service = (gfp_service_t *) svc;

        GFPS_PRINTF("Key-based pairing write\r\n");

        if (offset) {
                return ATT_ERROR_ATTRIBUTE_NOT_LONG;
        }

        if ((length != REQUEST_LENGTH) &&
            (length != (REQUEST_LENGTH + PUBLIC_KEY_LENGTH))) {
                return ATT_ERROR_APPLICATION_ERROR;
        }

        if (service->cb->pairing_cb) {
                if (length == (REQUEST_LENGTH + PUBLIC_KEY_LENGTH)) {
                        service->cb->pairing_cb(svc, conn_idx, value, &(value[REQUEST_LENGTH]));
                } else {
                        service->cb->pairing_cb(svc, conn_idx, value, NULL);
                }

        }

        return ATT_ERROR_OK;
}

static att_error_t do_key_pairing_ccc_write(gfp_service_t *gfps, uint16_t conn_idx, uint16_t offset,
                                                              uint16_t length, const uint8_t *value)
{
        uint16_t ccc;

        if (offset) {
                return ATT_ERROR_ATTRIBUTE_NOT_LONG;
        }

        if (length != sizeof(ccc)) {
                return ATT_ERROR_APPLICATION_ERROR;
        }

        ccc = get_u16(value);
        GFPS_PRINTF("Key-based pairing_ccc write %d\r\n", ccc);
        ble_storage_put_u32(conn_idx, gfps->key_pairing_ccc_h, ccc, true);

        return ATT_ERROR_OK;
}

static att_error_t do_passkey_write(ble_service_t *svc, uint16_t conn_idx, uint16_t offset,
                uint16_t length, const uint8_t *value)
{
        gfp_service_t *service = (gfp_service_t *) svc;

        GFPS_PRINTF("Passkey write\r\n");

        if (offset) {
                return ATT_ERROR_ATTRIBUTE_NOT_LONG;
        }

        if (length != REQUEST_LENGTH) {
                return ATT_ERROR_APPLICATION_ERROR;
        }

        if (service->cb->passkey_cb) {
                service->cb->passkey_cb(svc, conn_idx, value);
        }

        return ATT_ERROR_OK;
}

static att_error_t do_passkey_ccc_write(gfp_service_t *gfps, uint16_t conn_idx,
                                             uint16_t offset, uint16_t length, const uint8_t *value)
{
        uint16_t ccc;

        if (offset) {
                return ATT_ERROR_ATTRIBUTE_NOT_LONG;
        }

        if (length != sizeof(ccc)) {
                return ATT_ERROR_APPLICATION_ERROR;
        }

        ccc = get_u16(value);
        GFPS_PRINTF("Passkey ccc_write %d\r\n", ccc);
        ble_storage_put_u32(conn_idx, gfps->pass_key_ccc_h, ccc, true);

        return ATT_ERROR_OK;
}

static att_error_t do_additional_data_write(gfp_service_t *svc, uint16_t conn_idx, uint16_t offset,
                uint16_t length, const uint8_t *value)
{
        GFPS_PRINTF("Additional data write\r\n");

        if (offset) {
                return ATT_ERROR_ATTRIBUTE_NOT_LONG;
        }

        if (svc->cb->additional_data_cb) {
                svc->cb->additional_data_cb(conn_idx, value, length);
        }

        return ATT_ERROR_OK;
}

static att_error_t do_additional_data_ccc_write(gfp_service_t *gfps, uint16_t conn_idx,
                                             uint16_t offset, uint16_t length, const uint8_t *value)
{
        uint16_t ccc;

        if (offset) {
                return ATT_ERROR_ATTRIBUTE_NOT_LONG;
        }

        if (length != sizeof(ccc)) {
                return ATT_ERROR_APPLICATION_ERROR;
        }

        ccc = get_u16(value);
        GFPS_PRINTF("Additional data ccc write %d\r\n", ccc);
        ble_storage_put_u32(conn_idx, gfps->additional_data_ccc_h, ccc, true);

        return ATT_ERROR_OK;
}

static att_error_t do_accountkey_write(ble_service_t *svc, uint16_t conn_idx, uint16_t length, const uint8_t *keybuffer)
{
        gfp_service_t *gfps = (gfp_service_t *) svc;

        GFPS_PRINTF("Account key write\r\n");

        if (length != REQUEST_LENGTH) {
                return ATT_ERROR_APPLICATION_ERROR;
        }

        if (gfps->cb->accountkey_cb) {
                gfps->cb->accountkey_cb(conn_idx, keybuffer);
        }

        return ATT_ERROR_OK;
}

#if (FP_FMDN == 1)
static att_error_t do_beacon_actions_write(ble_service_t *svc, uint16_t conn_idx, uint16_t offset,
                uint16_t length, const uint8_t *value)
{
        gfp_service_t *gfps = (gfp_service_t *) svc;
        att_error_t status = ATT_ERROR_OK;

        GFPS_PRINTF("Beacon actions write\r\n");

        if (gfps->cb->beacon_actions_write_cb) {
                status = gfps->cb->beacon_actions_write_cb(svc, conn_idx, value, length);
        }

        return status;
}

static att_error_t do_beacon_actions_ccc_write(gfp_service_t *gfps, uint16_t conn_idx,
                                             uint16_t offset, uint16_t length, const uint8_t *value)
{
        uint16_t ccc;

        if (offset) {
                return ATT_ERROR_ATTRIBUTE_NOT_LONG;
        }

        if (length != sizeof(ccc)) {
                return ATT_ERROR_APPLICATION_ERROR;
        }

        ccc = get_u16(value);
        GFPS_PRINTF("Beacon actions ccc write %d\r\n", ccc);
        ble_storage_put_u32(conn_idx, gfps->beacon_actions_ccc_h, ccc, true);

        return ATT_ERROR_OK;
}

static att_error_t do_beacon_actions_read(ble_service_t *svc, uint16_t conn_idx, uint8_t *response)
{
        gfp_service_t *gfps = (gfp_service_t *) svc;

        if (gfps->cb->beacon_actions_read_cb) {
                gfps->cb->beacon_actions_read_cb(svc, conn_idx, response);
        }

        return ATT_ERROR_OK;
}
#endif /* FP_FMDN */

static void handle_write_req(ble_service_t *svc, const ble_evt_gatts_write_req_t *evt)
{
        gfp_service_t *gfps = (gfp_service_t *) svc;
        att_error_t err = ATT_ERROR_OK;

        if (!fp_is_initialized()) {
                err = ATT_ERROR_WRITE_NOT_PERMITTED;
        } else if (evt->handle == gfps->key_pairing_h) {
                err = do_pairing_write(svc, evt->conn_idx, evt->offset, evt->length, evt->value);
        } else if (evt->handle == gfps->key_pairing_ccc_h) {
                err = do_key_pairing_ccc_write(gfps, evt->conn_idx, evt->offset, evt->length,
                                                                                        evt->value);
        } else if (evt->handle == gfps->pass_key_h) {
                err = do_passkey_write(svc, evt->conn_idx, evt->offset, evt->length, evt->value);
        } else if (evt->handle == gfps->pass_key_ccc_h) {
                err = do_passkey_ccc_write(gfps, evt->conn_idx, evt->offset, evt->length,
                                                                           evt->value);
        } else if (evt->handle == gfps->account_key_h) {
                err = do_accountkey_write(svc, evt->conn_idx, evt->length, evt->value);
        } else if (evt->handle == gfps->additional_data_h) {
                err = do_additional_data_write(gfps, evt->conn_idx, evt->offset, evt->length,
                                                                                        evt->value);
        } else if (evt->handle == gfps->additional_data_ccc_h) {
                err = do_additional_data_ccc_write(gfps, evt->conn_idx, evt->offset, evt->length,
                                                                                        evt->value);
#if (FP_FMDN == 1)
        } else if (evt->handle == gfps->beacon_actions_h) {
                err = do_beacon_actions_write(svc, evt->conn_idx, evt->offset, evt->length, evt->value);
        } else if (evt->handle == gfps->beacon_actions_ccc_h) {
                err = do_beacon_actions_ccc_write(gfps, evt->conn_idx, evt->offset, evt->length,
                                                                                        evt->value);
#endif
        } else {
                err = ATT_ERROR_WRITE_NOT_PERMITTED;
        }

        ble_gatts_write_cfm(evt->conn_idx, evt->handle, err);
}

static void handle_read_req(ble_service_t *svc, const ble_evt_gatts_read_req_t *evt)
{
        gfp_service_t *gfps = (gfp_service_t *) svc;

        if (!fp_is_initialized()) {
                ble_gatts_read_cfm(evt->conn_idx, evt->handle, ATT_ERROR_READ_NOT_PERMITTED, 0,
                                                                                              NULL);
        } else if (evt->handle == gfps->key_pairing_ccc_h) {
                uint16_t ccc = 0x0000;

                ble_storage_get_u16(evt->conn_idx, gfps->key_pairing_ccc_h, &ccc);
                ble_gatts_read_cfm(evt->conn_idx, evt->handle, ATT_ERROR_OK, sizeof(ccc), &ccc);
        } else if (evt->handle == gfps->pass_key_ccc_h) {
                uint16_t ccc = 0x0000;

                ble_storage_get_u16(evt->conn_idx, gfps->pass_key_ccc_h, &ccc);
                ble_gatts_read_cfm(evt->conn_idx, evt->handle, ATT_ERROR_OK, sizeof(ccc), &ccc);
#if (FP_FMDN == 1)
        } else if (evt->handle == gfps->beacon_actions_h) {
                uint8_t response[BEACON_ACTIONS_READ_LENGTH];
                GFPS_PRINTF("Beacon actions read\r\n");
                do_beacon_actions_read(svc, evt->conn_idx, response);
                ble_gatts_read_cfm(evt->conn_idx, evt->handle, ATT_ERROR_OK,
                                                              BEACON_ACTIONS_READ_LENGTH, response);
        } else if (evt->handle == gfps->beacon_actions_ccc_h) {
                uint16_t ccc = 0x0000;
                GFPS_PRINTF("Beacon actions notify check\r\n");
                ble_storage_get_u16(evt->conn_idx, gfps->beacon_actions_ccc_h, &ccc);
                ble_gatts_read_cfm(evt->conn_idx, evt->handle, ATT_ERROR_OK, sizeof(ccc), &ccc);
#endif /* FP_FMDN */
        } else {
                ble_gatts_read_cfm(evt->conn_idx, evt->handle, ATT_ERROR_READ_NOT_PERMITTED, 0,
                                                                                              NULL);
        }
}

static void handle_connected_evt(ble_service_t *svc, const ble_evt_gap_connected_t *evt)
{
        GFPS_PRINTF("Seeker connected\r\n");
}

static void handle_disconnected_evt(ble_service_t *svc, const ble_evt_gap_disconnected_t *evt)
{
        GFPS_PRINTF("Seeker disconnected %u\r\n", evt->reason);
}

static void cleanup(ble_service_t *svc)
{
        gfp_service_t *gfps = (gfp_service_t *) svc;

        ble_storage_remove_all(gfps->key_pairing_ccc_h);
        ble_storage_remove_all(gfps->pass_key_ccc_h);
        ble_storage_remove_all(gfps->additional_data_ccc_h);
#if (FP_FMDN == 1)
        ble_storage_remove_all(gfps->beacon_actions_ccc_h);
#endif
        OS_FREE(svc);
}

ble_error_t gfps_notify_pairing(ble_service_t *svc, uint16_t conn_idx, uint8_t *response)
{
        gfp_service_t *gfps = (gfp_service_t *) svc;
        uint16_t ccc = 0x0000;

        ble_storage_get_u16(conn_idx, gfps->key_pairing_ccc_h, &ccc);

        if (!(ccc & GATT_CCC_NOTIFICATIONS)) {
                return BLE_ERROR_FAILED;
        }

        GFPS_PRINTF("Key-based pairing notify\r\n");
        return ble_gatts_send_event(conn_idx, gfps->key_pairing_h, GATT_EVENT_NOTIFICATION, REQUEST_LENGTH,
                                                                                        response);
}

ble_error_t gfps_notify_passkey(ble_service_t *svc, uint16_t conn_idx, uint8_t *passkey_block)
{
        gfp_service_t *gfps = (gfp_service_t *) svc;
        uint16_t ccc = 0x0000;

        ble_storage_get_u16(conn_idx, gfps->pass_key_ccc_h, &ccc);

        if (!(ccc & GATT_CCC_NOTIFICATIONS)) {
                return BLE_ERROR_FAILED;
        }

        GFPS_PRINTF("Passkey notify\r\n");
        return ble_gatts_send_event(conn_idx, gfps->pass_key_h, GATT_EVENT_NOTIFICATION, REQUEST_LENGTH,
                                                                                     passkey_block);
}

ble_error_t gfps_notify_additional_data(ble_service_t *svc, uint16_t conn_idx, uint8_t *data,
                                                                                     uint8_t length)
{
        gfp_service_t *gfps = (gfp_service_t *) svc;
        uint16_t ccc = 0x0000;

        ble_storage_get_u16(conn_idx, gfps->additional_data_ccc_h, &ccc);

        if (!(ccc & GATT_CCC_NOTIFICATIONS)) {
                return BLE_ERROR_FAILED;
        }

        GFPS_PRINTF("Additional data notify\r\n");
        return ble_gatts_send_event(conn_idx, gfps->additional_data_h, GATT_EVENT_NOTIFICATION, length, data);
}

#if (FP_FMDN == 1)
ble_error_t gfps_notify_beacon_actions(ble_service_t *svc, uint16_t conn_idx, uint8_t *data, uint8_t length)
{
        gfp_service_t *gfps = (gfp_service_t *) svc;
        uint16_t ccc = 0x0000;

        ble_storage_get_u16(conn_idx, gfps->beacon_actions_ccc_h, &ccc);

        if (!(ccc & GATT_CCC_NOTIFICATIONS)) {
                return BLE_ERROR_FAILED;
        }

        GFPS_PRINTF("Beacon actions notify\r\n");
        return ble_gatts_send_event(conn_idx, gfps->beacon_actions_h, GATT_EVENT_NOTIFICATION, length, data);
}
#endif /* FP_FMDN */

ble_service_t *gfps_init(const gfps_info_t *info, const fast_pair_callbacks_t *cb)
{
        gfp_service_t *gfps;
        uint16_t num_attr;
        att_uuid_t uuid;
#if (FP_LOCATOR_TAG != 1)
        uint8_t model_id[3];

        model_id[0] = (info->model_id >> 16) & 0xff;
        model_id[1] = (info->model_id >> 8) & 0xff;
        model_id[2] = info->model_id & 0xff;
#endif /* !FP_LOCATOR_TAG */

        gfps = OS_MALLOC(sizeof(*gfps));
        memset(gfps, 0, sizeof(*gfps));

        gfps->svc.connected_evt = handle_connected_evt;
        gfps->svc.disconnected_evt = handle_disconnected_evt;
        gfps->svc.write_req = handle_write_req;
        gfps->svc.read_req = handle_read_req;
        gfps->svc.cleanup = cleanup;
        gfps->cb = cb;

#if (FP_FMDN == 1)
        num_attr = ble_gatts_get_num_attr(0, 6, 4);
#else
        num_attr = ble_gatts_get_num_attr(0, 5, 3);
#endif

        ble_uuid_create16(UUID_GFP_SERVICE, &uuid);                     // Fast Pair Service
        ble_gatts_add_service(&uuid, GATT_SERVICE_PRIMARY, num_attr);
#if (FP_LOCATOR_TAG != 1)
        ble_uuid_from_string(UUID_GFP_MODEL_ID, &uuid);                 // Model ID
        ble_gatts_add_characteristic(&uuid, GATT_PROP_READ, ATT_PERM_READ,
                3 * sizeof(uint8_t), GATTS_FLAG_CHAR_READ_REQ, NULL, &gfps->model_id_h);
#endif
        ble_uuid_from_string(UUID_GFP_KEY_BASED_PAIRING, &uuid);        // Key-based Pairing
        ble_gatts_add_characteristic(&uuid, GATT_PROP_WRITE | GATT_PROP_NOTIFY, ATT_PERM_WRITE,
                REQUEST_LENGTH + PUBLIC_KEY_LENGTH, 0, NULL, &gfps->key_pairing_h);

        ble_uuid_create16(UUID_GATT_CLIENT_CHAR_CONFIGURATION, &uuid);
        ble_gatts_add_descriptor(&uuid, ATT_PERM_RW, sizeof(uint16_t), 0, &gfps->key_pairing_ccc_h);

        ble_uuid_from_string(UUID_GFP_PASS_KEY, &uuid);                 // Pass key
        ble_gatts_add_characteristic(&uuid, GATT_PROP_WRITE | GATT_PROP_NOTIFY, ATT_PERM_WRITE,
                16 * sizeof(uint8_t), 0, NULL, &gfps->pass_key_h);

        ble_uuid_create16(UUID_GATT_CLIENT_CHAR_CONFIGURATION, &uuid);
        ble_gatts_add_descriptor(&uuid, ATT_PERM_RW, sizeof(uint16_t), 0, &gfps->pass_key_ccc_h);

        ble_uuid_from_string(UUID_GFP_ACCOUNT_KEY, &uuid);              // Account Key
        ble_gatts_add_characteristic(&uuid, GATT_PROP_WRITE, ATT_PERM_WRITE,
                16 * sizeof(uint8_t), 0, NULL, &gfps->account_key_h);

        ble_uuid_from_string(UUID_GFP_ADDITIONAL_DATA, &uuid);          // Additional data
        ble_gatts_add_characteristic(&uuid, GATT_PROP_WRITE | GATT_PROP_NOTIFY, ATT_PERM_WRITE,
                ADDITIONAL_DATA_LENGTH, 0, NULL, &gfps->additional_data_h);

        ble_uuid_create16(UUID_GATT_CLIENT_CHAR_CONFIGURATION, &uuid);
        ble_gatts_add_descriptor(&uuid, ATT_PERM_RW, sizeof(uint16_t), 0,
                                                                      &gfps->additional_data_ccc_h);

#if (FP_FMDN == 1)
        ble_uuid_from_string(UUID_GFP_BEACON_ACTIONS, &uuid);           // Beacon actions
        ble_gatts_add_characteristic(&uuid, GATT_PROP_READ | GATT_PROP_WRITE | GATT_PROP_NOTIFY,
                ATT_PERM_RW, BEACON_ACTIONS_LENGTH, GATTS_FLAG_CHAR_READ_REQ,
                NULL, &gfps->beacon_actions_h);

        ble_uuid_create16(UUID_GATT_CLIENT_CHAR_CONFIGURATION, &uuid);
        ble_gatts_add_descriptor(&uuid, ATT_PERM_RW, sizeof(uint16_t), 0, &gfps->beacon_actions_ccc_h);

        ble_gatts_register_service(&gfps->svc.start_h,
#if (FP_LOCATOR_TAG != 1)
                &gfps->model_id_h,
#endif
                &gfps->key_pairing_h, &gfps->key_pairing_ccc_h, &gfps->pass_key_h,
                &gfps->pass_key_ccc_h, &gfps->account_key_h, &gfps->additional_data_h,
                &gfps->additional_data_ccc_h, &gfps->beacon_actions_h,
                &gfps->beacon_actions_ccc_h, 0);
#else
        ble_gatts_register_service(&gfps->svc.start_h, &gfps->model_id_h, &gfps->key_pairing_h,
                &gfps->key_pairing_ccc_h, &gfps->pass_key_h, &gfps->pass_key_ccc_h,
                &gfps->account_key_h, &gfps->additional_data_h, &gfps->additional_data_ccc_h, 0);
#endif /* FP_FMDN */

#if (FP_LOCATOR_TAG != 1)
        /* Set model ID. */
        ble_gatts_set_value(gfps->model_id_h, sizeof(model_id), model_id);
#endif

        gfps->svc.end_h = gfps->svc.start_h + num_attr;

        ble_service_add(&gfps->svc);

        return &gfps->svc;
}

#endif /* defined(CONFIG_USE_BLE_SERVICES) */
