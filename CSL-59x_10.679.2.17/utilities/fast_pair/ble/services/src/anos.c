/**
 ****************************************************************************************
 *
 * @file anos.c
 *
 * @brief Accessory Non-Owner Service implementation
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
#include "ble_uuid.h"
#include "ble_storage.h"
#include "ble_bufops.h"
#include "fast_pair.h"

#include "anos.h"

#if (FP_FMDN == 1)

//#define DEBUG_ANOS

#ifdef DEBUG_ANOS
#define ANOS_PRINTF(fmt, ...) printf(fmt, ##__VA_ARGS__)
#else
#define ANOS_PRINTF(fmt, ...)
#endif

#define UUID_ANO_SERVICE                "15190001-12F4-C226-88ED-2AC5579F2A85"
#define UUID_ANO_NON_ONWER              "8E0C0001-1D68-FB92-BF61-48377421680E"

#define REQUEST_LENGTH                  (16)

typedef struct {
        ble_service_t svc;

        // handles
        uint16_t non_owner_h;
        uint16_t non_owner_ccc_h;
        write_request_cb_t non_owner_cb;
} ano_service_t;

static att_error_t do_non_owner_write(ble_service_t *svc, uint16_t conn_idx, uint16_t offset,
                uint16_t length, const uint8_t *value)
{
        ano_service_t *service = (ano_service_t *) svc;

        ANOS_PRINTF("Accessory Non-Owner write\r\n");

        if (offset) {
                return ATT_ERROR_ATTRIBUTE_NOT_LONG;
        }

        if (length != sizeof(uint16_t)) {
                return ATT_ERROR_INVALID_VALUE_LENGTH;
        }

        if (service->non_owner_cb) {
                service->non_owner_cb(svc, conn_idx, *((uint16_t*)value));
        }

        return ATT_ERROR_OK;
}

static att_error_t do_non_owner_ccc_write(ano_service_t *anos, uint16_t conn_idx, uint16_t offset,
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
        ANOS_PRINTF("Accessory Non-Owner ccc write %d\r\n", ccc);
        ble_storage_put_u32(conn_idx, anos->non_owner_ccc_h, ccc, true);

        return ATT_ERROR_OK;
}

static void handle_write_req(ble_service_t *svc, const ble_evt_gatts_write_req_t *evt)
{
        ano_service_t *anos = (ano_service_t *) svc;
        att_error_t err = ATT_ERROR_OK;

        if (!fp_is_initialized()) {
                err = ATT_ERROR_WRITE_NOT_PERMITTED;
        } else if (evt->handle == anos->non_owner_h) {
                err = do_non_owner_write(svc, evt->conn_idx, evt->offset, evt->length, evt->value);
        } else if (evt->handle == anos->non_owner_ccc_h) {
                err = do_non_owner_ccc_write(anos, evt->conn_idx, evt->offset, evt->length,
                                                                                        evt->value);
        } else {
                err = ATT_ERROR_WRITE_NOT_PERMITTED;
        }

        ble_gatts_write_cfm(evt->conn_idx, evt->handle, err);
}

static void handle_connected_evt(ble_service_t *svc, const ble_evt_gap_connected_t *evt)
{
        ANOS_PRINTF("Client connected\r\n");
}

static void handle_disconnected_evt(ble_service_t *svc, const ble_evt_gap_disconnected_t *evt)
{
        ANOS_PRINTF("Client disconnected %u\r\n", evt->reason);
}

static void cleanup(ble_service_t *svc)
{
        ano_service_t *anos = (ano_service_t *) svc;
        ble_storage_remove_all(anos->non_owner_ccc_h);
        OS_FREE(svc);
}

ble_error_t anos_indicate_response(ble_service_t *svc, uint16_t conn_idx, uint8_t *response, uint8_t length)
{
        ano_service_t *anos = (ano_service_t *) svc;
        uint16_t ccc = 0x0000;

        ble_storage_get_u16(conn_idx, anos->non_owner_ccc_h, &ccc);

        if (!(ccc & GATT_CCC_INDICATIONS)) {
                return BLE_ERROR_FAILED;
        }

        ANOS_PRINTF("Accessory Non-Owner indicate\r\n");
        return ble_gatts_send_event(conn_idx, anos->non_owner_h, GATT_EVENT_INDICATION, length, response);
}

ble_service_t *anos_init(write_request_cb_t non_owner_cb)
{
        ano_service_t *anos;
        uint16_t num_attr;
        att_uuid_t uuid;

        anos = OS_MALLOC(sizeof(*anos));
        memset(anos, 0, sizeof(*anos));

        anos->svc.connected_evt = handle_connected_evt;
        anos->svc.disconnected_evt = handle_disconnected_evt;
        anos->svc.write_req = handle_write_req;
        anos->svc.cleanup = cleanup;
        anos->non_owner_cb = non_owner_cb;

        num_attr = ble_gatts_get_num_attr(0, 1, 1);

        ble_uuid_from_string(UUID_ANO_SERVICE, &uuid);          // Accessory Non-Owner Service
        ble_gatts_add_service(&uuid, GATT_SERVICE_PRIMARY, num_attr);

        ble_uuid_from_string(UUID_ANO_NON_ONWER, &uuid);        // Non-Owner characteristic write
        ble_gatts_add_characteristic(&uuid, GATT_PROP_WRITE | GATT_PROP_INDICATE, ATT_PERM_WRITE,
                                                       REQUEST_LENGTH, 0, NULL, &anos->non_owner_h);

        ble_uuid_create16(UUID_GATT_CLIENT_CHAR_CONFIGURATION, &uuid);
        ble_gatts_add_descriptor(&uuid, ATT_PERM_RW, sizeof(uint16_t), 0, &anos->non_owner_ccc_h);

        ble_gatts_register_service(&anos->svc.start_h, &anos->non_owner_h, &anos->non_owner_ccc_h,
                                                                                                 0);

        anos->svc.end_h = anos->svc.start_h + num_attr;

        ble_service_add(&anos->svc);

        return &anos->svc;
}
#endif /* FP_FMDN */

#endif /* defined(CONFIG_USE_BLE_SERVICES) */
