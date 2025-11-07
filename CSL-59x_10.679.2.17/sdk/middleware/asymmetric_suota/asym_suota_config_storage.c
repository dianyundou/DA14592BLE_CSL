/**
 ****************************************************************************************
 *
 * @file asym_suota_config_storage.c
 *
 * @brief Asymmetric SUOTA configuration storage implementation
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

#if (dg_configSUOTA_ASYMMETRIC == 1)

#if ASYM_SUOTA_USE_CONFIG_STORAGE
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include "asym_suota_config_storage.h"
#include "osal.h"
#include "ad_nvms.h"
#include "ad_nvparam.h"

/*
 * MACRO DEFINITIONS
 *****************************************************************************************
 */
#ifndef ASYM_SUOTA_CONFIG_VERIFY_VALUES
#define ASYM_SUOTA_CONFIG_VERIFY_VALUES                 ( 1 )
#endif

#ifndef DEBUG_PRINT
#define DEBUG_PRINT(_type, _format, ...)
#endif

#ifndef ASUOTA_CONFIG_PRINTF
#define ASUOTA_CONFIG_PRINTF(_type, ...)                DEBUG_PRINT(PRINT_ ## _type, "ASUOTA_CONFIG: " __VA_ARGS__)
#endif

#ifndef ASUOTA_CONFIG_CHECK
#define ASUOTA_CONFIG_CHECK(ret)                        ({if(ret != ASUOTA_CONFIG_ERR_NO_ERROR) {              \
                                                                DEBUG_PRINT((ret == ASUOTA_CONFIG_ERR_CONF_NOT_FOUND ? PRINT_INFO : PRINT_ERROR), \
                                                                        "ASUOTA_CONFIG: %s (%s: %d)\r\n", dbg_asym_suota_conf_error_descr(ret), __func__, __LINE__); \
                                                                return ret;                                 \
                                                        }})
#endif

#define ASUOTA_CONFIG_ADV_INTERVAL_MIN                  32      // 0x20
#define ASUOTA_CONFIG_ADV_INTERVAL_MAX                  16384   // 0x4000
#define ASUOTA_CONFIG_CONN_INTERVAL_MIN                 6       // 0x06  - N * 1.250ms
#define ASUOTA_CONFIG_CONN_INTERVAL_MAX                 3200    // 0xC80 - N * 1.250ms
#define ASUOTA_CONFIG_CONN_LATENCY_MIN                  0       // 0x00  - N * cnx evt
#define ASUOTA_CONFIG_CONN_LATENCY_MAX                  499     // 0x1F3 - N * cnx evt
#define ASUOTA_CONFIG_CONN_SUP_TO_MIN                   10      // 0x0A  - N * 10ms
#define ASUOTA_CONFIG_CONN_SUP_TO_MAX                   3200    // 0xC80 - N * 10ms
#define ASUOTA_CONFIG_BD_ADDR_RENEW_DUR_MIN             1
#define ASUOTA_CONFIG_BD_ADDR_RENEW_DUR_MAX             3600

#define AD_NVPARAM_DEFS_H_

#if dg_configNVPARAM_ADAPTER
#define NVPARAM_CONFIG_INIT(conf, idx, NAME)            conf[idx].param = ad_nvparam_open( # NAME)
#define NVPARAM_AREA(NAME, PARTITION, OFFSET)           enum tag_ ## NAME ##_offset_t {                                 \
                                                                NV_AREA_OFFSET_ ## PARTITION ## _ ## NAME = OFFSET,     \
                                                        };
#define NVPARAM_PARAM(TAG, OFFSET, LENGTH)
#define NVPARAM_VARPARAM(TAG, OFFSET, LENGTH)
#define NVPARAM_AREA_END()
#else
#define NVPARAM_TAG_INVALID                             UINT8_MAX
#define NVPARAM_CONFIG_INIT(conf, idx, NAME)            conf[idx].area = &area_ ## NAME; \
                                                        conf[idx].param = ad_nvms_open(area_ ## NAME.part)


#define NVPARAM_AREA(NAME, PARTITION, OFFSET)           static const parameter_t parameters_ ## NAME[];                 \
                                                        enum tag_ ## NAME ##_offset_t {                                 \
                                                                NV_AREA_OFFSET_ ## PARTITION ## _ ## NAME = OFFSET,     \
                                                        };                                                              \
                                                        static const area_t area_ ## NAME = {                           \
                                                                .part = PARTITION,                                      \
                                                                .offset = OFFSET,                                       \
                                                                .params = parameters_ ## NAME,                          \
                                                        };                                                              \
                                                        static const parameter_t parameters_ ## NAME[] = {

#define NVPARAM_PARAM(TAG, OFFSET, LENGTH)              {                                                               \
                                                                .tag = TAG,                                             \
                                                                .offset = OFFSET,                                       \
                                                                .length = LENGTH,                                       \
                                                        },

#define NVPARAM_VARPARAM(TAG, OFFSET, LENGTH)           {                                                               \
                                                                .tag = TAG,                                             \
                                                                .flags = FLAG_VARIABLE_LEN,                             \
                                                                .offset = OFFSET,                                       \
                                                                .length = LENGTH,                                       \
                                                        },
#define NVPARAM_AREA_END()                              {                                                               \
                                                                .tag = NVPARAM_TAG_INVALID,                             \
                                                                .offset = 0,                                            \
                                                                .length = 0,                                            \
                                                        },\
                                                        };
#endif

#if ASYM_SUOTA_USE_CONFIG_STORAGE_GENERIC_PART
/*
 * Macros used to calculate offset of configuration storage in generic partition
 */
#ifndef CONFIG_BLE_STORAGE_APV_PART_OFFSET
#define CONFIG_BLE_STORAGE_APV_PART_OFFSET              (0x500)
#endif

#ifndef CONFIG_BLE_STORAGE_APV_PART_LENGTH
#define CONFIG_BLE_STORAGE_APV_PART_LENGTH              (1024)
#endif
#endif

/*
 * TYPE DEFINITIONS
 *****************************************************************************************
 */
typedef enum {
#if ASYM_SUOTA_USE_CONFIG_STORAGE_GENERIC_PART
        CONFIG_AREA_STORAGE_DYN,
#endif
        CONFIG_AREA_STORAGE,
        CONFIG_AREA_MAX
} CONFIG_AREA;

#if !dg_configNVPARAM_ADAPTER
typedef struct {
        uint8_t tag;                    // unique parameter tag, user has to ensure the unique value
        uint8_t flags;                  // parameter flags
        uint16_t length;                // parameter max length
        uint32_t offset;                // parameter offset inside area
} parameter_t;

typedef struct {
        nvms_partition_id_t part;
        uint32_t offset;
        const parameter_t *params;
} area_t;
#endif

typedef struct {
#if dg_configNVPARAM_ADAPTER
        nvparam_t param;
#else
        nvms_t param;
        const area_t *area;
#endif
} config_t;

/*
 * GLOBAL VARIABLES
 *****************************************************************************************
 */
#include "app_nvparam.h"

__RETAINED static config_t configs[CONFIG_AREA_MAX];

#if ASYM_SUOTA_USE_CONFIG_STORAGE_GENERIC_PART
#if defined( __STDC_VERSION__ ) && __STDC_VERSION__ >= 201112L
_Static_assert(CONFIG_BLE_STORAGE_APV_PART_OFFSET + CONFIG_BLE_STORAGE_APV_PART_LENGTH <=
                                                NV_AREA_OFFSET_NVMS_GENERIC_PART_config_storage_dyn,
        "Offset of config storage in generic partition is not enough for BLE data");
#endif
#endif

/*
 * STATIC FUNCTIONS
 *****************************************************************************************
 */
__UNUSED static const char *dbg_asym_suota_conf_error_descr(ASUOTA_CONFIG_ERR error)
{
        static char error_txt[] = "ASUOTA_CONFIG_ERR_UNKNOWN 0x0000";
        switch (error) {
        case ASUOTA_CONFIG_ERR_NO_ERROR           : return "ASUOTA_CONFIG_ERR_NO_ERROR";
        case ASUOTA_CONFIG_ERR_PART_NOT_FOUND     : return "ASUOTA_CONFIG_ERR_PART_NOT_FOUND";
        case ASUOTA_CONFIG_ERR_CONF_UNKNOWN       : return "ASUOTA_CONFIG_ERR_CONF_UNKNOWN";
        case ASUOTA_CONFIG_ERR_CONF_NOT_FOUND     : return "ASUOTA_CONFIG_ERR_CONF_NOT_FOUND";
        case ASUOTA_CONFIG_ERR_CONF_INVALID_LENGTH: return "ASUOTA_CONFIG_ERR_CONF_INVALID_LENGTH";
        case ASUOTA_CONFIG_ERR_CONF_INVALID_VALUE : return "ASUOTA_CONFIG_ERR_CONF_INVALID_VALUE";
        case ASUOTA_CONFIG_ERR_ALLOC_ERROR        : return "ASUOTA_CONFIG_ERR_ALLOC_ERROR";
        default                                   : snprintf(error_txt + sizeof(error_txt) - 5, 5, "%.4X", error);
                                                    return error_txt;
        };
}

static ASUOTA_CONFIG_ERR asym_suota_config_storage_get_part(const config_t *config, uint8_t tag, void *value, size_t size)
{
        uint16_t param_len, read_len;
        uint8_t valid = 0xFF;

#if dg_configNVPARAM_ADAPTER
        /* Parameter length shall be long enough to store address and validity flag */
        param_len = ad_nvparam_get_length(config->param, tag, NULL);
        if (!param_len) {
                return ASUOTA_CONFIG_ERR_CONF_UNKNOWN;
        }
#else
        uint32_t addr = config->area->offset;

        param_len = 0;
        for (int idx = 0; ; idx++) {
                if (config->area->params[idx].tag == NVPARAM_TAG_INVALID) {
                        break;
                }
                if (config->area->params[idx].tag == tag) {
                        param_len = config->area->params[idx].length;
                        addr += config->area->params[idx].offset;
                }
        }
#endif
        if (param_len < size + sizeof(valid)) {
                return ASUOTA_CONFIG_ERR_CONF_INVALID_LENGTH;
        }

#if dg_configNVPARAM_ADAPTER
        ad_nvparam_read_offset(config->param, tag, param_len - sizeof(valid), sizeof(valid), &valid);
#else
        ad_nvms_read(config->param, addr + param_len - sizeof(valid), &valid, sizeof(valid));
#endif

        /* Read param from nvparam only if validity flag is set to 0x00 and read_len is correct */
        if (valid != 0x00) {
                return ASUOTA_CONFIG_ERR_CONF_NOT_FOUND;
        }

#if dg_configNVPARAM_ADAPTER
        read_len = ad_nvparam_read(config->param, tag, size, value);
#else
        read_len = ad_nvms_read(config->param, addr, value, size);
#endif
        if (read_len != size) {
                return ASUOTA_CONFIG_ERR_CONF_INVALID_LENGTH;
        }

        return ASUOTA_CONFIG_ERR_NO_ERROR;
}

static ASUOTA_CONFIG_ERR asym_suota_config_storage_get(uint8_t tag, void *value, size_t size)
{
        ASUOTA_CONFIG_ERR ret = ASUOTA_CONFIG_ERR_PART_NOT_FOUND;

        for (int idx = 0; idx < ARRAY_LENGTH(configs); ++idx) {
                if (!configs[idx].param) {
                        continue;
                }
                ret = asym_suota_config_storage_get_part(&configs[idx], tag, value, size);
                if (ret == ASUOTA_CONFIG_ERR_NO_ERROR) {
                        return ret;
                }
        }

        return ret;
}

#if ASYM_SUOTA_USE_CONFIG_STORAGE_GENERIC_PART
static ASUOTA_CONFIG_ERR asym_suota_config_storage_set_part(const config_t *config, uint8_t tag, const void *value, size_t size)
{
        uint16_t param_len, write_len;
        uint8_t *buf;

#if dg_configNVPARAM_ADAPTER
        /* Parameter length shall be long enough to store address and validity flag */
        param_len = ad_nvparam_get_length(config->param, tag, NULL);
        if (!param_len) {
                return ASUOTA_CONFIG_ERR_CONF_UNKNOWN;
        }
#else
        uint32_t addr= config->area->offset;

        param_len = 0;
        for (int idx = 0; ; idx++) {
                if (config->area->params[idx].tag == NVPARAM_TAG_INVALID) {
                        break;
                }
                if (config->area->params[idx].tag == tag) {
                        param_len = config->area->params[idx].length;
                        addr += config->area->params[idx].offset;
                }
        }
#endif
        if (param_len < size + sizeof(uint8_t)) {
                return ASUOTA_CONFIG_ERR_CONF_INVALID_LENGTH;
        }

        buf = OS_MALLOC(param_len);
        if (buf == NULL) {
                return ASUOTA_CONFIG_ERR_ALLOC_ERROR;
        }

        if (size) {
                memmove(buf, value, size);
                memset(&buf[size], 0xFF, param_len - size - sizeof(uint8_t));
                buf[param_len - sizeof(uint8_t)] = 0x00;
        } else {
                memset(buf, 0xFF, param_len);
        }

#if dg_configNVPARAM_ADAPTER
        write_len = ad_nvparam_write(config->param, tag, param_len, buf);
#else
        write_len = ad_nvms_write(config->param, addr, buf, param_len);
#endif

        OS_FREE(buf);

        if (write_len != param_len) {
                return ASUOTA_CONFIG_ERR_CONF_INVALID_LENGTH;
        }

        return ASUOTA_CONFIG_ERR_NO_ERROR;
}
#endif

static ASUOTA_CONFIG_ERR asym_suota_config_storage_set(uint8_t tag, const void *value, size_t size)
{
        ASUOTA_CONFIG_ERR ret = ASUOTA_CONFIG_ERR_PART_NOT_FOUND;

#if ASYM_SUOTA_USE_CONFIG_STORAGE_GENERIC_PART
        if (!configs[CONFIG_AREA_STORAGE_DYN].param) {
                return ret;
        }

        ret = asym_suota_config_storage_set_part(&configs[CONFIG_AREA_STORAGE_DYN], tag, value, size);
#endif

        return ret;
}

#if ASYM_SUOTA_CONFIG_VERIFY_VALUES
static uint32_t calc_total_interval(uint16_t latency, uint16_t intv_max)
{
        return (uint32_t)((((1 + latency) * intv_max * 2 * 5) / 4) + 1);
}

static ASUOTA_CONFIG_ERR asym_suota_config_storage_verify_ci_min(const void *param)
{
        uint16_t val = *(uint16_t *)param;

        if ((val < ASUOTA_CONFIG_CONN_INTERVAL_MIN) || (val > ASUOTA_CONFIG_CONN_INTERVAL_MAX)) {
                return ASUOTA_CONFIG_ERR_CONF_INVALID_VALUE;
        }
        return ASUOTA_CONFIG_ERR_NO_ERROR;
}

static ASUOTA_CONFIG_ERR asym_suota_config_storage_verify_ci_max(const void *param)
{
        uint16_t val = *(uint16_t *)param;

        if ((val < ASUOTA_CONFIG_CONN_INTERVAL_MIN) || (val > ASUOTA_CONFIG_CONN_INTERVAL_MAX)) {
                return ASUOTA_CONFIG_ERR_CONF_INVALID_VALUE;
        }
        return ASUOTA_CONFIG_ERR_NO_ERROR;
}

static ASUOTA_CONFIG_ERR asym_suota_config_storage_verify_sl(const void *param)
{
        uint16_t val = *(uint16_t *)param;

        if ((val < ASUOTA_CONFIG_CONN_LATENCY_MIN) || (val > ASUOTA_CONFIG_CONN_LATENCY_MAX)) {
                return ASUOTA_CONFIG_ERR_CONF_INVALID_VALUE;
        }
        return ASUOTA_CONFIG_ERR_NO_ERROR;
}

static ASUOTA_CONFIG_ERR asym_suota_config_storage_verify_sup_to(const void *param)
{
        uint16_t val = *(uint16_t *)param;

        if ((val < ASUOTA_CONFIG_CONN_SUP_TO_MIN || val > ASUOTA_CONFIG_CONN_SUP_TO_MAX)) {
                return ASUOTA_CONFIG_ERR_CONF_INVALID_VALUE;
        }
        return ASUOTA_CONFIG_ERR_NO_ERROR;
}

static ASUOTA_CONFIG_ERR asym_suota_config_storage_verify_mtu(const void *param)
{
        uint16_t val = *(uint16_t *)param;

        if (val < defaultBLE_MIN_MTU_SIZE || val > defaultBLE_MAX_MTU_SIZE) {
                return ASUOTA_CONFIG_ERR_CONF_INVALID_VALUE;
        }
        return ASUOTA_CONFIG_ERR_NO_ERROR;
}

#if dg_configBLE_PERIPHERAL
static ASUOTA_CONFIG_ERR asym_suota_config_storage_verify_adv_int_min(const void *param)
{
        uint16_t val = *(uint16_t *)param;

        if (val < ASUOTA_CONFIG_ADV_INTERVAL_MIN || val > ASUOTA_CONFIG_ADV_INTERVAL_MAX) {
                return ASUOTA_CONFIG_ERR_CONF_INVALID_VALUE;
        }
        return ASUOTA_CONFIG_ERR_NO_ERROR;
}

static ASUOTA_CONFIG_ERR asym_suota_config_storage_verify_adv_int_max(const void *param)
{
        uint16_t val = *(uint16_t *)param;

        if (val < ASUOTA_CONFIG_ADV_INTERVAL_MIN || val > ASUOTA_CONFIG_ADV_INTERVAL_MAX) {
                return ASUOTA_CONFIG_ERR_CONF_INVALID_VALUE;
        }
        return ASUOTA_CONFIG_ERR_NO_ERROR;
}

static ASUOTA_CONFIG_ERR asym_suota_config_storage_verify_adv_channel_map(const void *param)
{
        uint8_t val = *(uint8_t *)param;
        if (val > ADV_ALL_CHNLS_EN) {
                return ASUOTA_CONFIG_ERR_CONF_INVALID_VALUE;
        }
        return ASUOTA_CONFIG_ERR_NO_ERROR;
}
#endif /* dg_configBLE_PERIPHERAL */

static ASUOTA_CONFIG_ERR asym_suota_config_storage_verify_con_params(uint16_t intv_min, uint16_t intv_max,
        uint16_t slave_latency, uint16_t sup_timeout)
{
        uint32_t interval = calc_total_interval(slave_latency, intv_max);

        if (intv_min > intv_max) {
                return ASUOTA_CONFIG_ERR_CONF_INVALID_VALUE;
        }

        if ((interval >= (sup_timeout * 10))) {
                return ASUOTA_CONFIG_ERR_CONF_INVALID_VALUE;
        }

        return ASUOTA_CONFIG_ERR_NO_ERROR;
}

#if dg_configBLE_PERIPHERAL
static ASUOTA_CONFIG_ERR asym_suota_config_storage_verify_adv_params(uint16_t intv_min, uint16_t intv_max)
{
        if (intv_min > intv_max) {
                return ASUOTA_CONFIG_ERR_CONF_INVALID_VALUE;
        }

        return ASUOTA_CONFIG_ERR_NO_ERROR;
}
#endif /* dg_configBLE_PERIPHERAL */

static ASUOTA_CONFIG_ERR asym_suota_config_storage_verify_sec_reqs(const void *param)
{
        ASUOTA_CONFIG_SECUR_REQ val = *(ASUOTA_CONFIG_SECUR_REQ *)param;
        if ((val & (ASUOTA_CONFIG_SECUR_REQ_LVL_MSK | ASUOTA_CONFIG_SECUR_REQ_EN_NON_BONDED)) != val) {
                return ASUOTA_CONFIG_ERR_CONF_INVALID_VALUE;
        }
        return ASUOTA_CONFIG_ERR_NO_ERROR;
}

static ASUOTA_CONFIG_ERR asym_suota_config_storage_verify_own_address(const own_address_t *addr, uint16_t renew_dur)
{
        static const uint8_t null_addr[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

        switch (addr->addr_type) {
        case PUBLIC_STATIC_ADDRESS:
                break;
        case PRIVATE_STATIC_ADDRESS:
                if (!memcmp(addr->addr, null_addr, sizeof(addr->addr))) {
                        return ASUOTA_CONFIG_ERR_CONF_INVALID_VALUE;
                }
                if ((addr->addr[sizeof(addr->addr) - 1] & LE_RAND_BD_ADDR_TYPE_MASK) != RAND_STATIC_ADDR) {
                        return ASUOTA_CONFIG_ERR_CONF_INVALID_VALUE;
                }
                break;
        case PRIVATE_RANDOM_RESOLVABLE_ADDRESS:
        case PRIVATE_RANDOM_NONRESOLVABLE_ADDRESS:
#if (dg_configBLE_PRIVACY_1_2 == 1)
        case PRIVATE_CNTL:
#endif /* (dg_configBLE_PRIVACY_1_2 == 1) */
                if (renew_dur < ASUOTA_CONFIG_BD_ADDR_RENEW_DUR_MIN || renew_dur > ASUOTA_CONFIG_BD_ADDR_RENEW_DUR_MAX) {
                        return ASUOTA_CONFIG_ERR_CONF_INVALID_VALUE;
                }
                break;
        default:
                return ASUOTA_CONFIG_ERR_CONF_INVALID_VALUE;
        }

        return ASUOTA_CONFIG_ERR_NO_ERROR;
}
#endif /* ASYM_SUOTA_CONFIG_VERIFY_VALUES */

/*
 * API FUNCTIONS
 *****************************************************************************************
 */
void asym_suota_config_storage_init(void)
{
        if (configs[0].param) {
                return;
        }

#if ASYM_SUOTA_USE_CONFIG_STORAGE_GENERIC_PART
        NVPARAM_CONFIG_INIT(configs, CONFIG_AREA_STORAGE_DYN, config_storage_dyn);
#endif
        NVPARAM_CONFIG_INIT(configs, CONFIG_AREA_STORAGE, config_storage);
}

#if dg_configBLE_PERIPHERAL
ASUOTA_CONFIG_ERR asym_suota_config_storage_get_adv_timeout(uint32_t *adv_tmo)
{
        ASUOTA_CONFIG_ERR ret;
        uint32_t tmo = 0;

        ret = asym_suota_config_storage_get(NVPARAM_CONFIG_STORAGE_ADV_TIMEOUT, &tmo, sizeof(tmo));
        ASUOTA_CONFIG_CHECK(ret);

        *adv_tmo = tmo;
        ASUOTA_CONFIG_PRINTF(INFO, "Adv timeout got %ldms\r\n", tmo);
        return ret;
}

ASUOTA_CONFIG_ERR asym_suota_config_storage_set_adv_timeout(uint32_t adv_tmo)
{
        ASUOTA_CONFIG_ERR ret;

        ret = asym_suota_config_storage_set(NVPARAM_CONFIG_STORAGE_ADV_TIMEOUT, &adv_tmo, sizeof(adv_tmo));
        ASUOTA_CONFIG_CHECK(ret);

        ASUOTA_CONFIG_PRINTF(INFO, "Adv timeout set %ldms\r\n", adv_tmo);
        return ret;
}

ASUOTA_CONFIG_ERR asym_suota_config_storage_get_adv_intv(uint16_t *adv_intv_min, uint16_t *adv_intv_max)
{
        ASUOTA_CONFIG_ERR ret;
        uint16_t intv_min = 0, intv_max = 0;

        ret = asym_suota_config_storage_get(NVPARAM_CONFIG_STORAGE_ADV_INTERVAL_MIN, &intv_min, sizeof(intv_min));
        ASUOTA_CONFIG_CHECK(ret);

#if ASYM_SUOTA_CONFIG_VERIFY_VALUES
        ret = asym_suota_config_storage_verify_adv_int_min(&intv_min);
        ASUOTA_CONFIG_CHECK(ret);
#endif

        ret = asym_suota_config_storage_get(NVPARAM_CONFIG_STORAGE_ADV_INTERVAL_MAX, &intv_max, sizeof(intv_max));
        ASUOTA_CONFIG_CHECK(ret);

#if ASYM_SUOTA_CONFIG_VERIFY_VALUES
        ret = asym_suota_config_storage_verify_adv_int_max(&intv_max);
        ASUOTA_CONFIG_CHECK(ret);

        ret = asym_suota_config_storage_verify_adv_params(intv_min, intv_max);
        ASUOTA_CONFIG_CHECK(ret);
#endif

        *adv_intv_min = intv_min;
        *adv_intv_max = intv_max;
        ASUOTA_CONFIG_PRINTF(INFO, "Adv intv got %dms - %dms\r\n", BLE_ADV_INTERVAL_TO_MS(intv_min),
                BLE_ADV_INTERVAL_TO_MS(intv_max));
        return ret;
}

ASUOTA_CONFIG_ERR asym_suota_config_storage_set_adv_intv(uint16_t adv_intv_min, uint16_t adv_intv_max)
{
        ASUOTA_CONFIG_ERR ret;

#if ASYM_SUOTA_CONFIG_VERIFY_VALUES
        ret = asym_suota_config_storage_verify_adv_int_min(&adv_intv_min);
        ASUOTA_CONFIG_CHECK(ret);

        ret = asym_suota_config_storage_verify_adv_int_max(&adv_intv_max);
        ASUOTA_CONFIG_CHECK(ret);

        ret = asym_suota_config_storage_verify_adv_params(adv_intv_min, adv_intv_max);
        ASUOTA_CONFIG_CHECK(ret);
#endif

        ret = asym_suota_config_storage_set(NVPARAM_CONFIG_STORAGE_ADV_INTERVAL_MIN, &adv_intv_min, sizeof(adv_intv_min));
        ASUOTA_CONFIG_CHECK(ret);

        ret = asym_suota_config_storage_set(NVPARAM_CONFIG_STORAGE_ADV_INTERVAL_MAX, &adv_intv_max, sizeof(adv_intv_max));
        ASUOTA_CONFIG_CHECK(ret);

        ASUOTA_CONFIG_PRINTF(INFO, "Adv intv set %dms - %dms\r\n", BLE_ADV_INTERVAL_TO_MS(adv_intv_min),
                BLE_ADV_INTERVAL_TO_MS(adv_intv_max));
        return ret;
}

ASUOTA_CONFIG_ERR asym_suota_config_storage_get_adv_chnl_map(uint8_t *chnl_map)
{
        ASUOTA_CONFIG_ERR ret;
        __UNUSED static const char *adv_chnl_map_descr[] = {
                "-", "37", "38", "37-38", "39", "37-39", "38-39", "37-38-39",
        };
        uint8_t map;

        ret = asym_suota_config_storage_get(NVPARAM_CONFIG_STORAGE_ADV_CHANNEL_MAP, &map, sizeof(map));
        ASUOTA_CONFIG_CHECK(ret);

#if ASYM_SUOTA_CONFIG_VERIFY_VALUES
        ret = asym_suota_config_storage_verify_adv_channel_map(&map);
        ASUOTA_CONFIG_CHECK(ret);
#endif

        *chnl_map = map;
        ASUOTA_CONFIG_PRINTF(INFO, "Adv channel map got %s\r\n", adv_chnl_map_descr[map]);
        return ret;
}

ASUOTA_CONFIG_ERR asym_suota_config_storage_set_adv_chnl_map(uint8_t chnl_map)
{
        ASUOTA_CONFIG_ERR ret;
        __UNUSED static const char *adv_chnl_map_descr[] = {
                "-", "37", "38", "37-38", "39", "37-39", "38-39", "37-38-39",
        };

#if ASYM_SUOTA_CONFIG_VERIFY_VALUES
        ret = asym_suota_config_storage_verify_adv_channel_map(&chnl_map);
        ASUOTA_CONFIG_CHECK(ret);
#endif

        ret = asym_suota_config_storage_set(NVPARAM_CONFIG_STORAGE_ADV_CHANNEL_MAP, &chnl_map, sizeof(chnl_map));
        ASUOTA_CONFIG_CHECK(ret);

        ASUOTA_CONFIG_PRINTF(INFO, "Adv channel map set %s\r\n", adv_chnl_map_descr[chnl_map]);
        return ret;
}
#endif /* dg_configBLE_PERIPHERAL */

ASUOTA_CONFIG_ERR asym_suota_config_storage_get_adv_data(uint8_t *adv_data_len, const uint8_t **adv_data)
{
        ASUOTA_CONFIG_ERR ret;
        uint8_t len = 0;
        __UNUSED char adv_data_tmp[BLE_ADV_DATA_LEN_MAX * 3 + 1];

        ret = asym_suota_config_storage_get(NVPARAM_CONFIG_STORAGE_ADV_DATA_LEN, &len, sizeof(len));
        ASUOTA_CONFIG_CHECK(ret);

        if (len == 0) {
                return ret;
        }

        uint8_t *ad = OS_MALLOC(len);
        if (!ad) {
                ret = ASUOTA_CONFIG_ERR_ALLOC_ERROR;
                ASUOTA_CONFIG_CHECK(ret);
        }

        ret = asym_suota_config_storage_get(NVPARAM_CONFIG_STORAGE_ADV_DATA, ad, len);
        if (ret != ASUOTA_CONFIG_ERR_NO_ERROR) {
                OS_FREE(ad);
                ASUOTA_CONFIG_CHECK(ret);
        }

        *adv_data = ad;
        *adv_data_len = len;

#if defined(CONFIG_RETARGET) || defined(CONFIG_RTT)
        adv_data_tmp[0] = '\0';
        for (int i = 0, pos = 0; i < len; i++, pos +=3) {
                snprintf(&adv_data_tmp[pos], sizeof(adv_data_tmp) - pos, " %.2X", ad[i]);
        }
#endif
        ASUOTA_CONFIG_PRINTF(INFO, "Adv data got [%d]%s\r\n", len, adv_data_tmp);
        return ret;
}

ASUOTA_CONFIG_ERR asym_suota_config_storage_set_adv_data(uint8_t adv_data_len, const uint8_t *adv_data)
{
        ASUOTA_CONFIG_ERR ret;
        __UNUSED char adv_data_tmp[BLE_ADV_DATA_LEN_MAX * 3 + 1];

        ret = asym_suota_config_storage_set(NVPARAM_CONFIG_STORAGE_ADV_DATA, adv_data, adv_data_len);
        ASUOTA_CONFIG_CHECK(ret);

        ret = asym_suota_config_storage_set(NVPARAM_CONFIG_STORAGE_ADV_DATA_LEN, &adv_data_len, sizeof(adv_data_len));
        ASUOTA_CONFIG_CHECK(ret);

#if defined(CONFIG_RETARGET) || defined(CONFIG_RTT)
        adv_data_tmp[0] = '\0';
        for (int i = 0, pos = 0; i < adv_data_len; i++, pos +=3) {
                snprintf(&adv_data_tmp[pos], sizeof(adv_data_tmp) - pos, " %.2X", adv_data[i]);
        }
#endif
        ASUOTA_CONFIG_PRINTF(INFO, "Adv data set [%d]%s\r\n", adv_data_len, adv_data_tmp);
        return ret;
}

#if dg_configBLE_PERIPHERAL
ASUOTA_CONFIG_ERR asym_suota_config_storage_get_scan_rsp(uint8_t *scan_rsp_len, const uint8_t **scan_rsp)
{
        ASUOTA_CONFIG_ERR ret;
        uint8_t len = 0;
        __UNUSED char scan_resp_tmp[BLE_SCAN_RSP_LEN_MAX * 3 + 1];

        ret = asym_suota_config_storage_get(NVPARAM_CONFIG_STORAGE_SCAN_RESP_DATA_LEN, &len, sizeof(len));
        ASUOTA_CONFIG_CHECK(ret);

        if (len == 0) {
                return ret;
        }

        uint8_t *sr = OS_MALLOC(len);
        if (!sr) {
                ret = ASUOTA_CONFIG_ERR_ALLOC_ERROR;
                ASUOTA_CONFIG_CHECK(ret);
        }

        ret = asym_suota_config_storage_get(NVPARAM_CONFIG_STORAGE_SCAN_RESP_DATA, sr, len);
        if (ret != ASUOTA_CONFIG_ERR_NO_ERROR) {
                OS_FREE(sr);
                ASUOTA_CONFIG_CHECK(ret);
        }

        *scan_rsp = sr;
        *scan_rsp_len = len;
#if defined(CONFIG_RETARGET) || defined(CONFIG_RTT)
        scan_resp_tmp[0] = '\0';
        for (int i = 0, pos = 0; i < len; i++, pos +=3) {
                snprintf(&scan_resp_tmp[pos], sizeof(scan_resp_tmp) - pos, " %.2X", sr[i]);
        }
#endif
        ASUOTA_CONFIG_PRINTF(INFO, "Scan data got [%d]%s\r\n", len, scan_resp_tmp);
        return ret;
}

ASUOTA_CONFIG_ERR asym_suota_config_storage_set_scan_rsp(uint8_t scan_rsp_len, const uint8_t *scan_rsp)
{
        ASUOTA_CONFIG_ERR ret;
        __UNUSED char scan_resp_tmp[BLE_SCAN_RSP_LEN_MAX * 3 + 1];

        ret = asym_suota_config_storage_set(NVPARAM_CONFIG_STORAGE_SCAN_RESP_DATA, scan_rsp, scan_rsp_len);
        ASUOTA_CONFIG_CHECK(ret);

        ret = asym_suota_config_storage_set(NVPARAM_CONFIG_STORAGE_SCAN_RESP_DATA_LEN, &scan_rsp_len, sizeof(scan_rsp_len));
        ASUOTA_CONFIG_CHECK(ret);

#if defined(CONFIG_RETARGET) || defined(CONFIG_RTT)
        scan_resp_tmp[0] = '\0';
        for (int i = 0, pos = 0; i < scan_rsp_len; i++, pos +=3) {
                snprintf(&scan_resp_tmp[pos], sizeof(scan_resp_tmp) - pos, " %.2X", scan_rsp[i]);
        }
#endif
        ASUOTA_CONFIG_PRINTF(INFO, "Scan data set [%d]%s\r\n", scan_rsp_len, scan_resp_tmp);
        return ret;
}
#endif /* dg_configBLE_PERIPHERAL */

ASUOTA_CONFIG_ERR asym_suota_config_storage_get_device_name(const char **dev_name)
{
        ASUOTA_CONFIG_ERR ret;
        uint8_t dev_name_len = 0;

        ret = asym_suota_config_storage_get(NVPARAM_CONFIG_STORAGE_DEVICE_NAME_LEN, &dev_name_len, sizeof(dev_name_len));
        ASUOTA_CONFIG_CHECK(ret);

        if (dev_name_len == 0) {
                return ret;
        }

        char *dn = OS_MALLOC(dev_name_len + 1);
        if (!dn) {
                ret = ASUOTA_CONFIG_ERR_ALLOC_ERROR;
                ASUOTA_CONFIG_CHECK(ret);
        }

        ret = asym_suota_config_storage_get(NVPARAM_CONFIG_STORAGE_DEVICE_NAME, dn, dev_name_len);
        if (ret != ASUOTA_CONFIG_ERR_NO_ERROR) {
                OS_FREE(dn);
                ASUOTA_CONFIG_CHECK(ret);
        }

        dn[dev_name_len] = '\0';
        *dev_name = dn;
        ASUOTA_CONFIG_PRINTF(INFO, "Dev name got [%d] \"%s\"\r\n", dev_name_len, dn);
        return ret;
}

ASUOTA_CONFIG_ERR asym_suota_config_storage_set_device_name(const char *dev_name)
{
        ASUOTA_CONFIG_ERR ret;
        uint8_t dev_name_len = strlen(dev_name);

        ret = asym_suota_config_storage_set(NVPARAM_CONFIG_STORAGE_DEVICE_NAME, dev_name, dev_name_len);
        ASUOTA_CONFIG_CHECK(ret);

        ret = asym_suota_config_storage_set(NVPARAM_CONFIG_STORAGE_DEVICE_NAME_LEN, &dev_name_len, sizeof(dev_name_len));
        ASUOTA_CONFIG_CHECK(ret);

        ASUOTA_CONFIG_PRINTF(INFO, "Dev name set [%d] \"%s\"\r\n", dev_name_len, dev_name);
        return ret;
}

ASUOTA_CONFIG_ERR asym_suota_config_storage_get_mtu_size(uint16_t *mtu_size)
{
        ASUOTA_CONFIG_ERR ret;
        uint16_t mtu;

        ret = asym_suota_config_storage_get(NVPARAM_CONFIG_STORAGE_GAP_MTU, &mtu, sizeof(mtu));
        ASUOTA_CONFIG_CHECK(ret);

#if ASYM_SUOTA_CONFIG_VERIFY_VALUES
        ret = asym_suota_config_storage_verify_mtu(&mtu);
        ASUOTA_CONFIG_CHECK(ret);
#endif

        *mtu_size = mtu;
        ASUOTA_CONFIG_PRINTF(INFO, "MTU size got %d\r\n", mtu);
        return ret;
}

ASUOTA_CONFIG_ERR asym_suota_config_storage_set_mtu_size(uint16_t mtu_size)
{
        ASUOTA_CONFIG_ERR ret;

#if ASYM_SUOTA_CONFIG_VERIFY_VALUES
        ret = asym_suota_config_storage_verify_mtu(&mtu_size);
        ASUOTA_CONFIG_CHECK(ret);
#endif

        ret = asym_suota_config_storage_set(NVPARAM_CONFIG_STORAGE_GAP_MTU, &mtu_size, sizeof(mtu_size));
        ASUOTA_CONFIG_CHECK(ret);

        ASUOTA_CONFIG_PRINTF(INFO, "MTU size set %d\r\n", mtu_size);
        return ret;
}

ASUOTA_CONFIG_ERR asym_suota_config_storage_get_conn_param(gap_conn_params_t *conn_params)
{
        ASUOTA_CONFIG_ERR ret;
        uint16_t intv_min;
        uint16_t intv_max;
        uint16_t slave_latency;
        uint16_t sup_timeout;

        ret = asym_suota_config_storage_get(NVPARAM_CONFIG_STORAGE_CONN_INTERVAL_MIN, &intv_min, sizeof(intv_min));
        ASUOTA_CONFIG_CHECK(ret);

#if ASYM_SUOTA_CONFIG_VERIFY_VALUES
        ret = asym_suota_config_storage_verify_ci_min(&intv_min);
        ASUOTA_CONFIG_CHECK(ret);
#endif

        ret = asym_suota_config_storage_get(NVPARAM_CONFIG_STORAGE_CONN_INTERVAL_MAX, &intv_max, sizeof(intv_max));
        ASUOTA_CONFIG_CHECK(ret);

#if ASYM_SUOTA_CONFIG_VERIFY_VALUES
        ret = asym_suota_config_storage_verify_ci_max(&intv_max);
        ASUOTA_CONFIG_CHECK(ret);
#endif

        ret = asym_suota_config_storage_get(NVPARAM_CONFIG_STORAGE_CONN_LATENCY, &slave_latency, sizeof(slave_latency));
        ASUOTA_CONFIG_CHECK(ret);

#if ASYM_SUOTA_CONFIG_VERIFY_VALUES
        ret = asym_suota_config_storage_verify_sl(&slave_latency);
        ASUOTA_CONFIG_CHECK(ret);
#endif

        ret = asym_suota_config_storage_get(NVPARAM_CONFIG_STORAGE_CONN_TIME_OUT, &sup_timeout, sizeof(sup_timeout));
        ASUOTA_CONFIG_CHECK(ret);
#if ASYM_SUOTA_CONFIG_VERIFY_VALUES
        ret = asym_suota_config_storage_verify_sup_to(&sup_timeout);
        ASUOTA_CONFIG_CHECK(ret);

        ret = asym_suota_config_storage_verify_con_params(intv_min, intv_max, slave_latency, sup_timeout);
        ASUOTA_CONFIG_CHECK(ret);
#endif

        conn_params->interval_min = intv_min;
        conn_params->interval_max = intv_max;
        conn_params->slave_latency = slave_latency;
        conn_params->sup_timeout = sup_timeout;

        ASUOTA_CONFIG_PRINTF(INFO, "Conn params got INTV_MIN %d, INTV_MAX %d, SL %d, SUP_TO %d\r\n", intv_min, intv_max, slave_latency, sup_timeout);
        return ret;
}

ASUOTA_CONFIG_ERR asym_suota_config_storage_set_conn_param(const gap_conn_params_t *conn_params)
{
        ASUOTA_CONFIG_ERR ret;

#if ASYM_SUOTA_CONFIG_VERIFY_VALUES
        ret = asym_suota_config_storage_verify_ci_min(&conn_params->interval_min);
        ASUOTA_CONFIG_CHECK(ret);

        ret = asym_suota_config_storage_verify_ci_max(&conn_params->interval_max);
        ASUOTA_CONFIG_CHECK(ret);

        ret = asym_suota_config_storage_verify_sl(&conn_params->slave_latency);
        ASUOTA_CONFIG_CHECK(ret);

        ret = asym_suota_config_storage_verify_sup_to(&conn_params->sup_timeout);
        ASUOTA_CONFIG_CHECK(ret);

        ret = asym_suota_config_storage_verify_con_params(conn_params->interval_min, conn_params->interval_max,
                conn_params->slave_latency, conn_params->sup_timeout);
        ASUOTA_CONFIG_CHECK(ret);
#endif

        ret = asym_suota_config_storage_set(NVPARAM_CONFIG_STORAGE_CONN_INTERVAL_MIN,
                &conn_params->interval_min, sizeof(conn_params->interval_min));
        ASUOTA_CONFIG_CHECK(ret);

        ret = asym_suota_config_storage_set(NVPARAM_CONFIG_STORAGE_CONN_INTERVAL_MAX,
                &conn_params->interval_max, sizeof(conn_params->interval_max));
        ASUOTA_CONFIG_CHECK(ret);

        ret = asym_suota_config_storage_set(NVPARAM_CONFIG_STORAGE_CONN_LATENCY,
                &conn_params->slave_latency, sizeof(conn_params->slave_latency));
        ASUOTA_CONFIG_CHECK(ret);

        ret = asym_suota_config_storage_set(NVPARAM_CONFIG_STORAGE_CONN_TIME_OUT,
                &conn_params->sup_timeout, sizeof(conn_params->sup_timeout));
        ASUOTA_CONFIG_CHECK(ret);

        ASUOTA_CONFIG_PRINTF(INFO, "Conn params set INTV_MIN %d, INTV_MAX %d, SL %d, SUP_TO %d\r\n",
                conn_params->interval_min, conn_params->interval_max,
                conn_params->slave_latency, conn_params->sup_timeout);
        return ret;
}

ASUOTA_CONFIG_ERR asym_suota_config_storage_get_sec_reqs(ASUOTA_CONFIG_SECUR_REQ *req)
{
        ASUOTA_CONFIG_ERR ret;
        uint8_t sec_req;

        ret = asym_suota_config_storage_get(NVPARAM_CONFIG_STORAGE_SECURITY_REQ, &sec_req, sizeof(sec_req));
        ASUOTA_CONFIG_CHECK(ret);

#if ASYM_SUOTA_CONFIG_VERIFY_VALUES
        ret = asym_suota_config_storage_verify_sec_reqs(&sec_req);
        ASUOTA_CONFIG_CHECK(ret);
#endif

        *req = sec_req;

        ASUOTA_CONFIG_PRINTF(INFO, "Security req got 0x%.2X\r\n", sec_req);
        return ret;
}

ASUOTA_CONFIG_ERR asym_suota_config_storage_set_sec_reqs(ASUOTA_CONFIG_SECUR_REQ req)
{
        ASUOTA_CONFIG_ERR ret;

#if ASYM_SUOTA_CONFIG_VERIFY_VALUES
        ret = asym_suota_config_storage_verify_sec_reqs(&req);
        ASUOTA_CONFIG_CHECK(ret);
#endif

        ret = asym_suota_config_storage_set(NVPARAM_CONFIG_STORAGE_SECURITY_REQ, &req, sizeof(req));
        ASUOTA_CONFIG_CHECK(ret);

        ASUOTA_CONFIG_PRINTF(INFO, "Security req set 0x%.2X\r\n", req);
        return ret;
}

ASUOTA_CONFIG_ERR asym_suota_config_storage_get_own_address(own_address_t *addr, uint16_t *renew_dur)
{
        ASUOTA_CONFIG_ERR ret;
        own_address_t address = { 0 };
        uint16_t duration = 0;

        ret = asym_suota_config_storage_get(NVPARAM_CONFIG_STORAGE_BD_ADDR_TYPE, &address.addr_type, sizeof(address.addr_type));
        ASUOTA_CONFIG_CHECK(ret);

        switch (address.addr_type) {
        case PRIVATE_STATIC_ADDRESS:
                ret = asym_suota_config_storage_get(NVPARAM_CONFIG_STORAGE_BD_ADDR_ADDRESS, &address.addr, sizeof(address.addr));
                ASUOTA_CONFIG_CHECK(ret);
                break;
        case PRIVATE_RANDOM_RESOLVABLE_ADDRESS:
        case PRIVATE_RANDOM_NONRESOLVABLE_ADDRESS:
#if (dg_configBLE_PRIVACY_1_2 == 1)
        case PRIVATE_CNTL:
#endif /* (dg_configBLE_PRIVACY_1_2 == 1) */
                ret = asym_suota_config_storage_get(NVPARAM_CONFIG_STORAGE_BD_ADDR_RENEW_DUR, &duration, sizeof(duration));
                ASUOTA_CONFIG_CHECK(ret);
                break;
        default:
                break;
        }

#if ASYM_SUOTA_CONFIG_VERIFY_VALUES
        ret = asym_suota_config_storage_verify_own_address(&address, duration);
        ASUOTA_CONFIG_CHECK(ret);
#endif

        *addr = address;
        *renew_dur = duration;

#if defined(CONFIG_RETARGET) || defined(CONFIG_RTT)
        bd_address_t bd_addr;
        memcpy(bd_addr.addr, addr->addr, sizeof(bd_addr.addr));
#endif
        ASUOTA_CONFIG_PRINTF(INFO, "BD address set type: %s, addr: %s (%d sec)\r\n",
                addr->addr_type == PUBLIC_STATIC_ADDRESS                ? "public static"                  :
                addr->addr_type == PRIVATE_STATIC_ADDRESS               ? "private static"                 :
                addr->addr_type == PRIVATE_RANDOM_RESOLVABLE_ADDRESS    ? "random resolvable"              :
                addr->addr_type == PRIVATE_RANDOM_NONRESOLVABLE_ADDRESS ? "random non resolvable"          :
#if (dg_configBLE_PRIVACY_1_2 == 1)
                addr->addr_type == PRIVATE_CNTL                         ? "random resolvable LE priv v1.2" :
#endif /* (dg_configBLE_PRIVACY_1_2 == 1) */
                                                                          "unknown",
                ble_address_to_string(&bd_addr), duration);

        return ret;
}

ASUOTA_CONFIG_ERR asym_suota_config_storage_set_own_address(const own_address_t *addr, uint16_t renew_dur)
{
        ASUOTA_CONFIG_ERR ret;

#if ASYM_SUOTA_CONFIG_VERIFY_VALUES
        ret = asym_suota_config_storage_verify_own_address(addr, renew_dur);
        ASUOTA_CONFIG_CHECK(ret);
#endif

        ret = asym_suota_config_storage_set(NVPARAM_CONFIG_STORAGE_BD_ADDR_ADDRESS, &addr->addr, sizeof(addr->addr));
        ASUOTA_CONFIG_CHECK(ret);

        ret = asym_suota_config_storage_set(NVPARAM_CONFIG_STORAGE_BD_ADDR_RENEW_DUR, &renew_dur, sizeof(renew_dur));
        ASUOTA_CONFIG_CHECK(ret);

        ret = asym_suota_config_storage_set(NVPARAM_CONFIG_STORAGE_BD_ADDR_TYPE, &addr->addr_type, sizeof(addr->addr_type));
        ASUOTA_CONFIG_CHECK(ret);

#if defined(CONFIG_RETARGET) || defined(CONFIG_RTT)
        bd_address_t bd_addr;
        memcpy(bd_addr.addr, addr->addr, sizeof(bd_addr.addr));
#endif
        ASUOTA_CONFIG_PRINTF(INFO, "BD address set type: %s, addr: %s (%d sec)\r\n",
                addr->addr_type == PUBLIC_STATIC_ADDRESS                ? "public static"                  :
                addr->addr_type == PRIVATE_STATIC_ADDRESS               ? "private static"                 :
                addr->addr_type == PRIVATE_RANDOM_RESOLVABLE_ADDRESS    ? "random resolvable"              :
                addr->addr_type == PRIVATE_RANDOM_NONRESOLVABLE_ADDRESS ? "random non resolvable"          :
#if (dg_configBLE_PRIVACY_1_2 == 1)
                addr->addr_type == PRIVATE_CNTL                         ? "random resolvable LE priv v1.2" :
#endif /* (dg_configBLE_PRIVACY_1_2 == 1) */
                                                                          "unknown",
                ble_address_to_string(&bd_addr), renew_dur);

        return ret;
}

#endif /* ASYM_SUOTA_USE_CONFIG_STORAGE */

#endif /* dg_configSUOTA_ASYMMETRIC */
