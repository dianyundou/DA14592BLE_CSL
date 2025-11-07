/**
 ****************************************************************************************
 *
 * @file dis.c
 *
 * @brief Device Information Service sample implementation
 *
 * Copyright (C) 2015-2025 Renesas Electronics Corporation and/or its affiliates.
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
#include "dis.h"

#define UUID_MANUFACTURER_NAME_STRING   (0x2A29)
#define UUID_MODEL_NUMBER_STRING        (0x2A24)
#define UUID_SERIAL_NUMBER_STRING       (0x2A25)
#define UUID_HARDWARE_REVISION_STRING   (0x2A27)
#define UUID_FIRMWARE_REVISION_STRING   (0x2A26)
#define UUID_SOFTWARE_REVISION_STRING   (0x2A28)
#define UUID_SYSTEM_ID                  (0x2A23)
#define UUID_IEEE_REGULATORY_CERT_LIST  (0x2A2A)
#define UUID_PNP_ID                     (0x2A50)

typedef struct {
        ble_service_t svc;

        uint16_t manufacturer_name_val_h;
        uint16_t model_number_val_h;
        uint16_t serial_no_val_h;
        uint16_t hw_rev_val_h;
        uint16_t fw_rev_val_h;
        uint16_t sw_rev_val_h;
        uint16_t sys_id_val_h;
        uint16_t cert_val_h;
        uint16_t pnp_id_val_h;

        dis_device_info_t info;

        uint16_t auth_conn[dg_configBLE_CONNECTIONS_MAX]; // authorized connection indexes
        bool auth_read_en;
} dis_service_t;

static uint8_t get_num_chars(const dis_device_info_t *info)
{
        uint8_t ret = 0;

        if (info->manufacturer != NULL) {
                ret++;
        }

        if (info->model_number != NULL) {
                ret++;
        }

        if (info->serial_number != NULL) {
                ret++;
        }

        if (info->hw_revision != NULL) {
                ret++;
        }

        if (info->fw_revision != NULL) {
                ret++;
        }

        if (info->sw_revision != NULL) {
                ret++;
        }

        if (info->system_id != NULL) {
                ret++;
        }

        if (info->reg_cert != NULL && info->reg_cert_length != 0) {
                ret++;
        }

        if (info->pnp_id != NULL) {
                ret++;
        }

        return ret;
}

static uint16_t *find_auth_conn(dis_service_t *dis, uint16_t conn_idx)
{
        for (int i = 0; i < ARRAY_LENGTH(dis->auth_conn); i++) {
                if (conn_idx == dis->auth_conn[i]) {
                        return &dis->auth_conn[i];
                }
        }

        return NULL;
}

static void handle_read_req(ble_service_t *svc, const ble_evt_gatts_read_req_t *evt)
{
        dis_service_t *dis = (dis_service_t *) svc;
        uint16_t handle = evt->handle;
        uint16_t conn_idx = evt->conn_idx;

        /* Check whether the connection is authorized to read characteristic */
        if (dis->auth_read_en && find_auth_conn(dis, conn_idx) == NULL) {
                ble_gatts_read_cfm(conn_idx, handle, ATT_ERROR_INSUFFICIENT_AUTHORIZATION, 0, NULL);
                return;
        }

        if (handle == dis->manufacturer_name_val_h) {
                ble_gatts_read_cfm(conn_idx, handle, ATT_ERROR_OK,
                                        strlen(dis->info.manufacturer), dis->info.manufacturer);
        } else if (handle == dis->model_number_val_h) {
                ble_gatts_read_cfm(conn_idx, handle, ATT_ERROR_OK,
                                        strlen(dis->info.model_number), dis->info.model_number);
        } else if (handle == dis->serial_no_val_h) {
                ble_gatts_read_cfm(conn_idx, handle, ATT_ERROR_OK,
                                        strlen(dis->info.serial_number), dis->info.serial_number);
        } else if (handle == dis->hw_rev_val_h) {
                ble_gatts_read_cfm(conn_idx, handle, ATT_ERROR_OK,
                                        strlen(dis->info.hw_revision), dis->info.hw_revision);
        } else if (handle == dis->fw_rev_val_h) {
                ble_gatts_read_cfm(conn_idx, handle, ATT_ERROR_OK,
                                        strlen(dis->info.fw_revision), dis->info.fw_revision);
        } else if (handle == dis->sw_rev_val_h) {
                ble_gatts_read_cfm(conn_idx, handle, ATT_ERROR_OK,
                                        strlen(dis->info.sw_revision), dis->info.sw_revision);
        } else if (handle == dis->sys_id_val_h) {
                uint8_t buf_sys[8];
                memcpy(buf_sys, dis->info.system_id->manufacturer, 5);
                memcpy(&buf_sys[5], dis->info.system_id->oui, 3);

                ble_gatts_read_cfm(conn_idx, handle, ATT_ERROR_OK, sizeof(buf_sys), buf_sys);
        } else if (handle == dis->cert_val_h) {
                ble_gatts_read_cfm(conn_idx, handle, ATT_ERROR_OK,
                                        dis->info.reg_cert_length, dis->info.reg_cert);
        } else if (handle == dis->pnp_id_val_h) {
                uint8_t buf_pnp[7];
                uint8_t *p = buf_pnp;

                put_u8_inc(&p, dis->info.pnp_id->vid_source);
                put_u16_inc(&p, dis->info.pnp_id->vid);
                put_u16_inc(&p, dis->info.pnp_id->pid);
                put_u16_inc(&p, dis->info.pnp_id->version);

                ble_gatts_read_cfm(conn_idx, handle, ATT_ERROR_OK, sizeof(buf_pnp), buf_pnp);
        } else {
                ble_gatts_read_cfm(conn_idx, handle, ATT_ERROR_READ_NOT_PERMITTED, 0, NULL);
        }
}

static void handle_disconnected(ble_service_t *svc, const ble_evt_gap_disconnected_t *evt)
{
        dis_service_t *dis = (dis_service_t *) svc;
        uint16_t *auth_conn;

        if (dis->auth_read_en) {
                auth_conn = find_auth_conn(dis, evt->conn_idx);

                if (auth_conn != NULL) {
                        *auth_conn = BLE_CONN_IDX_INVALID;
                }
        }
}

static void cleanup(ble_service_t *svc)
{
        OS_FREE(svc);
}

ble_service_t *dis_init(const ble_service_config_t *config, const dis_device_info_t *info)
{
        dis_service_t *dis;
        uint16_t num_attr;
        uint8_t num_chars;
        att_perm_t read_perm;
        att_uuid_t uuid;

        if (!info) {
                return NULL;
        }

        num_chars = get_num_chars(info);

        if (num_chars == 0) {
                return NULL;
        }

        dis = OS_MALLOC(sizeof(*dis));
        memset(dis, 0, sizeof(*dis));

        num_attr = ble_service_get_num_attr(config, num_chars, 0);
        read_perm = ble_service_config_elevate_perm(ATT_PERM_READ, config);

        ble_uuid_create16(UUID_SERVICE_DIS, &uuid);
        ble_gatts_add_service(&uuid, GATT_SERVICE_PRIMARY, num_attr);

        ble_service_config_add_includes(config);

        if (info->manufacturer != NULL) {
                ble_uuid_create16(UUID_MANUFACTURER_NAME_STRING, &uuid);
                ble_gatts_add_characteristic(&uuid, GATT_PROP_READ, read_perm,
                                                strlen(info->manufacturer),
                                                GATTS_FLAG_CHAR_READ_REQ, NULL,
                                                &dis->manufacturer_name_val_h);
        }

        if (info->model_number != NULL) {
                ble_uuid_create16(UUID_MODEL_NUMBER_STRING, &uuid);
                ble_gatts_add_characteristic(&uuid, GATT_PROP_READ, read_perm,
                                                strlen(info->model_number),
                                                GATTS_FLAG_CHAR_READ_REQ, NULL,
                                                &dis->model_number_val_h);
        }

        if (info->serial_number != NULL) {
                ble_uuid_create16(UUID_SERIAL_NUMBER_STRING, &uuid);
                ble_gatts_add_characteristic(&uuid, GATT_PROP_READ, read_perm,
                                                strlen(info->serial_number),
                                                GATTS_FLAG_CHAR_READ_REQ, NULL,
                                                &dis->serial_no_val_h);
        }

        if (info->hw_revision != NULL) {
                ble_uuid_create16(UUID_HARDWARE_REVISION_STRING, &uuid);
                ble_gatts_add_characteristic(&uuid, GATT_PROP_READ, read_perm,
                                                strlen(info->hw_revision),
                                                GATTS_FLAG_CHAR_READ_REQ, NULL,
                                                &dis->hw_rev_val_h);
        }

        if (info->fw_revision != NULL) {
                ble_uuid_create16(UUID_FIRMWARE_REVISION_STRING, &uuid);
                ble_gatts_add_characteristic(&uuid, GATT_PROP_READ, read_perm,
                                                strlen(info->fw_revision),
                                                GATTS_FLAG_CHAR_READ_REQ, NULL,
                                                &dis->fw_rev_val_h);
        }

        if (info->sw_revision != NULL) {
                ble_uuid_create16(UUID_SOFTWARE_REVISION_STRING, &uuid);
                ble_gatts_add_characteristic(&uuid, GATT_PROP_READ, read_perm,
                                                strlen(info->sw_revision),
                                                GATTS_FLAG_CHAR_READ_REQ, NULL,
                                                &dis->sw_rev_val_h);
        }

        if (info->system_id != NULL) {
                ble_uuid_create16(UUID_SYSTEM_ID, &uuid);
                ble_gatts_add_characteristic(&uuid, GATT_PROP_READ, read_perm, 8,
                                                GATTS_FLAG_CHAR_READ_REQ, NULL,
                                                &dis->sys_id_val_h);
        }

        if (info->reg_cert != NULL && info->reg_cert_length != 0) {
                ble_uuid_create16(UUID_IEEE_REGULATORY_CERT_LIST, &uuid);
                ble_gatts_add_characteristic(&uuid, GATT_PROP_READ, read_perm,
                                                info->reg_cert_length,
                                                GATTS_FLAG_CHAR_READ_REQ, NULL,
                                                &dis->cert_val_h);
        }

        if (info->pnp_id != NULL) {
                ble_uuid_create16(UUID_PNP_ID, &uuid);
                ble_gatts_add_characteristic(&uuid, GATT_PROP_READ, read_perm, 8,
                                                GATTS_FLAG_CHAR_READ_REQ, NULL,
                                                &dis->pnp_id_val_h);
        }

        ble_gatts_register_service(&dis->svc.start_h, &dis->manufacturer_name_val_h,
                                        &dis->model_number_val_h, &dis->serial_no_val_h,
                                        &dis->hw_rev_val_h, &dis->fw_rev_val_h, &dis->sw_rev_val_h,
                                        &dis->sys_id_val_h, &dis->cert_val_h, &dis->pnp_id_val_h, 0);

        dis->svc.end_h = dis->svc.start_h + num_attr;

        dis->svc.read_req = handle_read_req;
        dis->svc.disconnected_evt = handle_disconnected;
        dis->svc.cleanup = cleanup;

        memcpy(&dis->info, info, sizeof(dis_device_info_t));

        dis_set_authorized_read(&dis->svc, BLE_CONN_IDX_INVALID, false);

        ble_service_add(&dis->svc);

        return &dis->svc;
}

int dis_set_authorized_read(ble_service_t *svc, uint16_t conn_idx, bool enable)
{
        dis_service_t *dis = (dis_service_t *) svc;
        uint16_t *auth_conn;

        if (conn_idx == BLE_CONN_IDX_INVALID) {
                if (!enable || dis->auth_read_en != enable) {
                        dis->auth_read_en = enable;

                        for (int i = 0; i < ARRAY_LENGTH(dis->auth_conn); i++) {
                                dis->auth_conn[i] = BLE_CONN_IDX_INVALID;
                        }
                }

                return 0;
        }

        if (!dis->auth_read_en && !enable) {
                return 0;
        }

        dis->auth_read_en = true;

        auth_conn = find_auth_conn(dis, conn_idx);

        if (enable) {
                if (auth_conn == NULL) {
                        auth_conn = find_auth_conn(dis, BLE_CONN_IDX_INVALID);
                }
        } else {
                conn_idx = BLE_CONN_IDX_INVALID;
        }

        if (auth_conn != NULL) {
                *auth_conn = conn_idx;
                return 0;
        }

        return -1;
}

bool dis_is_conn_authorized(ble_service_t *svc, uint16_t conn_idx)
{
        dis_service_t *dis = (dis_service_t *) svc;

        return (find_auth_conn(dis, conn_idx) != NULL);
}

ble_error_t dis_set_fw_revision(ble_service_t *svc, const char *fw_revision)
{
        dis_service_t *dis = (dis_service_t *) svc;

        if (dis->fw_rev_val_h) {
                dis->info.fw_revision = fw_revision;
                return BLE_STATUS_OK;
        }

        return BLE_ERROR_FAILED;
}

#endif /* defined(CONFIG_USE_BLE_SERVICES) */
