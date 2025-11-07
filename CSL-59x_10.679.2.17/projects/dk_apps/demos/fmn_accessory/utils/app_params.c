/**
 ****************************************************************************************
 *
 * @file app_params.c
 *
 * @brief Application parameters access implementation
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

#include <stdio.h>
#include <string.h>
#include "accessory_config.h"
#include "osal.h"
#include "ble_common.h"
#include "ble_gap.h"
#include "ad_nvparam.h"
#include "ad_nvms.h"
#include "app_nvparam.h"
#include "afmn_conn_params.h"

#include "app_params.h"

typedef enum {
        APP_PARAMS_ACCESS_READ,
        APP_PARAMS_ACCESS_WRITE
} APP_PARAMS_ACCESS;

typedef enum {
        APP_PARAMS_NV_AREA_BLE_APP = 0,
        APP_PARAMS_NV_AREA_FMN_APP,
        APP_PARAMS_NV_AREA_FMN_SW_AUTH,
#if (dg_configSUOTA_ASYMMETRIC == 1)
        APP_PARAMS_NV_AREA_ASYM_SUOTA_CONFIG_DYN,
#endif
        APP_PARAMS_NV_AREA_MAX
} APP_PARAMS_NV_AREA;

static const char * const nv_area_names[APP_PARAMS_NV_AREA_MAX] = {
        "ble_app", "fmn_app", "fmn_sw_auth",
#if (dg_configSUOTA_ASYMMETRIC == 1)
        "asym_suota_config_dyn"
#endif
};

/* Access application parameters */
static uint16_t app_params_access_params(APP_PARAMS_ACCESS access, app_params_t *params, uint8_t n)
{
        uint16_t total_access_len = 0;

        APP_PARAMS first_param;
        APP_PARAMS_NV_AREA nv_area;
        nvparam_t nvparam;
#if (dg_configSUOTA_ASYMMETRIC == 1)
        /* Parameter length shall be long enough to store address, address type
         * and validity flag */
        uint8_t asym_suota_param[BD_ADDR_LEN + 1] = { 0 };
#endif

        static const struct {
                APP_PARAMS_NV_AREA area;
                uint8_t tag;
        } nv_area_tags[] = {
                [AFMN_CONN_PARAMS_SERIAL_NUMBER]                = { APP_PARAMS_NV_AREA_BLE_APP,
                                                                    TAG_BLE_SERIAL_NUMBER },
                [AFMN_CONN_PARAMS_SW_AUTH_UUID]                 = { APP_PARAMS_NV_AREA_FMN_SW_AUTH,
                                                                    TAG_FMN_SW_AUTH_UUID },
                [AFMN_CONN_PARAMS_SW_AUTH_TOKEN]                = { APP_PARAMS_NV_AREA_FMN_SW_AUTH,
                                                                    TAG_FMN_SW_AUTH_TOKEN },
                [AFMN_CONN_PARAMS_BLE_IS_PAIRED]                = { APP_PARAMS_NV_AREA_FMN_APP,
                                                                    TAG_FMN_APP_IS_PAIRED },
                [AFMN_CONN_PARAMS_BLE_PRIMARY_KEY]              = { APP_PARAMS_NV_AREA_FMN_APP,
                                                                    TAG_FMN_APP_PRIMARY_KEY },
                [AFMN_CONN_PARAMS_BLE_PRIMARY_KEY_INDEX]        = { APP_PARAMS_NV_AREA_FMN_APP,
                                                                    TAG_FMN_APP_PRIMARY_KEY_INDEX },
                [AFMN_CONN_PARAMS_BLE_LTK]                      = { APP_PARAMS_NV_AREA_FMN_APP,
                                                                    TAG_FMN_APP_LTK },
                [AFMN_CONN_PARAMS_BLE_SECONDARY_KEY]            = { APP_PARAMS_NV_AREA_FMN_APP,
                                                                    TAG_FMN_APP_SECONDARY_KEY },
                [AFMN_CONN_PARAMS_BLE_SECONDARY_KEY_INDEX]      = { APP_PARAMS_NV_AREA_FMN_APP,
                                                                    TAG_FMN_APP_SECONDARY_KEY_INDEX },
                [AFMN_CONN_PARAMS_BLE_P]                        = { APP_PARAMS_NV_AREA_FMN_APP,
                                                                    TAG_FMN_APP_P },
                [AFMN_CONN_PARAMS_BLE_SKN]                      = { APP_PARAMS_NV_AREA_FMN_APP,
                                                                    TAG_FMN_APP_SKN },
                [AFMN_CONN_PARAMS_BLE_SKS]                      = { APP_PARAMS_NV_AREA_FMN_APP,
                                                                    TAG_FMN_APP_SKS },
                [AFMN_CONN_PARAMS_BLE_SHARED_KEY]               = { APP_PARAMS_NV_AREA_FMN_APP,
                                                                    TAG_FMN_APP_SHARED_KEY },
                [AFMN_CONN_PARAMS_BLE_ICLOUD_ID]                = { APP_PARAMS_NV_AREA_FMN_APP,
                                                                    TAG_FMN_APP_ICLOUD_ID },
                [APP_PARAMS_BLE_APP_NAME]                       = { APP_PARAMS_NV_AREA_BLE_APP,
                                                                    TAG_BLE_APP_NAME },
#if (dg_configSUOTA_ASYMMETRIC == 1)
                [APP_PARAMS_ASYM_SUOTA_BD_ADDR_TYPE]            = { APP_PARAMS_NV_AREA_ASYM_SUOTA_CONFIG_DYN,
                                                                    TAG_ASYM_SUOTA_BD_ADDR_TYPE },
                [APP_PARAMS_ASYM_SUOTA_BD_ADDR_ADDRESS]         = { APP_PARAMS_NV_AREA_ASYM_SUOTA_CONFIG_DYN,
                                                                    TAG_ASYM_SUOTA_BD_ADDR_ADDRESS }
#endif /* dg_configSUOTA_ASYMMETRIC */
        };

        OS_ASSERT(params);

        /* First parameter determines the area in which parameter access will be performed */
        first_param = params[0].param;
        nv_area = nv_area_tags[first_param].area;

        nvparam = ad_nvparam_open(nv_area_names[nv_area]);

        for (uint8_t i = 0; i < n; i++) {
                uint16_t param_len, access_len;
                void *param_data;
                uint8_t tag;

                tag = nv_area_tags[params[i].param].tag;
                param_len = params[i].len;
                param_data = params[i].data;

#if (dg_configSUOTA_ASYMMETRIC == 1)
                if (nv_area == APP_PARAMS_NV_AREA_ASYM_SUOTA_CONFIG_DYN) {
                        uint16_t nvparam_len = ad_nvparam_get_length(nvparam, tag, NULL);
                        uint16_t max_param_len = nvparam_len - 1;

                        if (param_len > max_param_len) {
                                param_len = max_param_len;
                        }

                        if (access == APP_PARAMS_ACCESS_READ) {
                                uint8_t valid = 0xFF;
                                ad_nvparam_read_offset(nvparam, tag, max_param_len,
                                        sizeof(valid), &valid);

                                if (valid != 0x00) {
                                        param_len = 0;
                                }
                        } else {
                                /* Initially parameter is considered valid (i.e. validity flag is 0x00) */
                                memset(asym_suota_param, 0x00, nvparam_len);

                                /*
                                 * Invalidate Asymmetric SUOTA configuration entry if given
                                 * parameter length is 0
                                 */
                                if (param_len == 0) {
                                        asym_suota_param[max_param_len] = 0xFF;
                                } else {
                                        memcpy(asym_suota_param, param_data, param_len);
                                }

                                param_data = asym_suota_param;

                                /* Include validity flag */
                                param_len = nvparam_len;
                        }
                }
#endif /* dg_configSUOTA_ASYMMETRIC */

                if (param_len == 0) {
                        access_len = 0;
                } else if (access == APP_PARAMS_ACCESS_READ) {
                        access_len = ad_nvparam_read(nvparam, tag, param_len, param_data);
                } else {
                        access_len = ad_nvparam_write(nvparam, tag, param_len, param_data);

                        if (access_len != param_len) {
                                FMNA_TASK_PRINTF("ERROR: Problem in writing parameter %d (%u bytes) to NVM\r\n",
                                        params[i].param, param_len);
                                OS_ASSERT(0);
                        }
                }

#if (dg_configSUOTA_ASYMMETRIC == 1)
                if (nv_area == APP_PARAMS_NV_AREA_ASYM_SUOTA_CONFIG_DYN) {
                        if (access != APP_PARAMS_ACCESS_READ && access_len > 0) {
                                int err = (access_len != param_len);

                                /* Exclude validity flag */
                                access_len -= 1;

                                /* Indicate failure in writing parameter if access_len != param_len */
                                if (access_len >= params[i].len) {
                                        access_len = params[i].len - err;
                                }
                        }
                }
#endif /* dg_configSUOTA_ASYMMETRIC */

                params[i].ret_len = access_len;
                total_access_len += access_len;
        }

        ad_nvparam_close(nvparam);

        return total_access_len;
}

uint16_t app_params_get_params(app_params_t *params, uint8_t n)
{
        return app_params_access_params(APP_PARAMS_ACCESS_READ, params, n);
}

uint16_t app_params_set_params(app_params_t *params, uint8_t n)
{
        return app_params_access_params(APP_PARAMS_ACCESS_WRITE, (app_params_t *) params, n);
}
