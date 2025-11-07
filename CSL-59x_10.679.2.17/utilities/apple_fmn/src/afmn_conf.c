/**
 ****************************************************************************************
 *
 * @file afmn_conf.c
 *
 * @brief Apple FMN configuration
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

#include AFMN_CONFIG_FILE
#include "afmn_defaults.h"
#include "afmn.h"

/* Apple FMN framework constant configuration structure */
const afmn_const_config_t afmn_const_config = {
        .manufacturer_name      = AFMN_MANUFACTURER_NAME,
        .model_name             = AFMN_MODEL_NAME,
        .accessory_category     = AFMN_ACCESSORY_CATEGORY,
        .product_data           = AFMN_PRODUCT_DATA,
        .fw_version_major       = AFMN_FW_VERSION_MAJOR,
        .fw_version_minor       = AFMN_FW_VERSION_MINOR,
        .fw_version_revision    = AFMN_FW_VERSION_REVISION,
        .accessory_capability   = AFMN_ACCESSORY_CAPABILITY,
        .battery_type           = AFMN_BATTERY_TYPE,
        .battery_level_medium   = AFMN_BATTERY_LEVEL_MEDIUM,
        .battery_level_low      = AFMN_BATTERY_LEVEL_LOW,
        .battery_level_critical = AFMN_BATTERY_LEVEL_CRITICAL,
        .adv_tx_power           = AFMN_ADV_TX_POWER,
        .tx_power_service_level = AFMN_TX_POWER_SERVICE_LEVEL,
        .sound_duration         = AFMN_SOUND_DURATION,
        .max_connections        = dg_configBLE_CONNECTIONS_MAX,
        .config_opt             = AFMN_CONFIG_OPTION
};
