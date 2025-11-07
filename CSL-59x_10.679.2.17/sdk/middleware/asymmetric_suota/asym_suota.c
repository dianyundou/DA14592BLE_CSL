/**
 ****************************************************************************************
 *
 * @file asym_suota.c
 *
 * @brief Asymmetric SUOTA service implementation
 *
 * Copyright (C) 2024 Renesas Electronics Corporation and/or its affiliates.
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
#include "ble_gatts.h"
#include "ble_uuid.h"
#include "svc_defines.h"
#include "asym_suota.h"

/*
 * MACRO DEFINITIONS
 *****************************************************************************************
 */
#define UUID_ASYM_SUOTA                 "17183bb9-a3a7-4a15-b864-979b9d161ebb"
#define UUID_ASYM_SUOTA_CMD_CHAR        "0edb8063-9c6e-4143-9af3-994b32f3547e"
#define UUID_ASYM_SUOTA_ID_CHAR         "106026f5-4d24-44f0-972e-d5c1945d0499"

#define ASYM_SUOTA_ID_CHAR_LEN          ( 32 )

/*
 * TYPE DEFINITIONS
 *****************************************************************************************
 */
typedef struct {
        ble_service_t svc;

        const asym_suota_callbacks_t *cb;

        uint16_t asym_suota_cmd_val_h;
        uint16_t asym_suota_id_val_h;
} sp_service_t;

/*
 * FORWARD DECLARATIONS
 *****************************************************************************************
 */
static void handle_write_req(ble_service_t *svc, const ble_evt_gatts_write_req_t *evt);
static void handle_read_req(ble_service_t *svc, const ble_evt_gatts_read_req_t *evt);
static void handle_cleanup(ble_service_t *svc);

/*
 * GLOBAL VARIABLES
 *****************************************************************************************
 */
static const char asym_suota_cmd_desc[] = "Asymmetric SUOTA command";
static const char asym_suota_id_desc[] = "Asymmetric SUOTA ID";

/*
 * API FUNCTIONS
 *****************************************************************************************
 */
ble_service_t *asym_suota_init(const ble_service_config_t *config, const asym_suota_callbacks_t *cb)
{
        uint16_t num_attr, asym_suota_cmd_desc_h, asym_suota_id_desc_h;
        sp_service_t *asym_suota;
        att_uuid_t uuid;

        asym_suota = OS_MALLOC(sizeof(*asym_suota));
        memset(asym_suota, 0, sizeof(*asym_suota));

        asym_suota->svc.write_req = handle_write_req;
        asym_suota->svc.read_req = handle_read_req;
        asym_suota->svc.cleanup = handle_cleanup;
        asym_suota->cb = cb;

        num_attr = ble_service_get_num_attr(config, 2, 2);

        ble_uuid_from_string(UUID_ASYM_SUOTA, &uuid);
        ble_gatts_add_service(&uuid, GATT_SERVICE_PRIMARY, num_attr);

        ble_service_config_add_includes(config);

        /* Asymmetric SUOTA command characteristic */
        ble_uuid_from_string(UUID_ASYM_SUOTA_CMD_CHAR, &uuid);
        ble_gatts_add_characteristic(&uuid, GATT_PROP_WRITE_NO_RESP,
                                        ble_service_config_elevate_perm(ATT_PERM_WRITE, config),
                                        sizeof(uint8_t), 0, NULL, &asym_suota->asym_suota_cmd_val_h);

        ble_uuid_create16(UUID_GATT_CHAR_USER_DESCRIPTION, &uuid);
        ble_gatts_add_descriptor(&uuid, ATT_PERM_READ, sizeof(asym_suota_cmd_desc), 0, &asym_suota_cmd_desc_h);

        /* Asymmetric SUOTA ID characteristic */
        ble_uuid_from_string(UUID_ASYM_SUOTA_ID_CHAR, &uuid);
        ble_gatts_add_characteristic(&uuid, GATT_PROP_READ,
                                        ble_service_config_elevate_perm(ATT_PERM_READ, config),
                                        ASYM_SUOTA_ID_CHAR_LEN, GATTS_FLAG_CHAR_READ_REQ, NULL, &asym_suota->asym_suota_id_val_h);

        ble_uuid_create16(UUID_GATT_CHAR_USER_DESCRIPTION, &uuid);
        ble_gatts_add_descriptor(&uuid, ATT_PERM_READ, sizeof(asym_suota_id_desc), 0, &asym_suota_id_desc_h);

        /* Register asymmetric SUOTA Service */
        ble_gatts_register_service(&asym_suota->svc.start_h,
                &asym_suota->asym_suota_cmd_val_h, &asym_suota_cmd_desc_h,
                &asym_suota->asym_suota_id_val_h, &asym_suota_id_desc_h,
                0);

        /* Set value of Characteristic Descriptions */
        ble_gatts_set_value(asym_suota_cmd_desc_h, sizeof(asym_suota_cmd_desc), asym_suota_cmd_desc);
        ble_gatts_set_value(asym_suota_id_desc_h, sizeof(asym_suota_id_desc), asym_suota_id_desc);

        asym_suota->svc.end_h = asym_suota->svc.start_h + num_attr;

        ble_service_add(&asym_suota->svc);

        return &asym_suota->svc;
}

/*
 * STATIC FUNCTIONS
 *****************************************************************************************
 */
static void handle_write_req(ble_service_t *svc, const ble_evt_gatts_write_req_t *evt)
{
        sp_service_t *asym_suota = (sp_service_t *)svc;
        att_error_t status = ATT_ERROR_ATTRIBUTE_NOT_FOUND;
        uint16_t handle = evt->handle;

        if (handle == asym_suota->asym_suota_cmd_val_h) {
                status = ATT_ERROR_APPLICATION_ERROR;
                if (asym_suota->cb && asym_suota->cb->cmd_callback) {
                        asym_suota->cb->cmd_callback(&asym_suota->svc, evt->conn_idx, evt->value[0]);
                        status = ATT_ERROR_OK;
                }
        }

        ble_gatts_write_cfm(evt->conn_idx, evt->handle, status);
}

static void handle_read_req(ble_service_t *svc, const ble_evt_gatts_read_req_t *evt)
{
        sp_service_t *asym_suota = (sp_service_t *)svc;
        att_error_t status = ATT_ERROR_READ_NOT_PERMITTED;

        if (evt->handle == asym_suota->asym_suota_id_val_h) {
                status = ATT_ERROR_OK;

                if (asym_suota->cb && asym_suota->cb->id_callback) {
                        uint8_t *value = NULL;
                        uint16_t length = 0;

                        status = asym_suota->cb->id_callback(svc, evt->conn_idx, &value, &length);

                        if (status == ATT_ERROR_OK) {
                                ble_gatts_read_cfm(evt->conn_idx, evt->handle, ATT_ERROR_OK, length, value);
                                return;
                        }
                }
        }

        ble_gatts_read_cfm(evt->conn_idx, evt->handle, status, 0, NULL);
}

static void handle_cleanup(ble_service_t *svc)
{
        sp_service_t *asym_suota = (sp_service_t *)svc;

        OS_FREE(asym_suota);
}

#endif /* defined(CONFIG_USE_BLE_SERVICES) */
