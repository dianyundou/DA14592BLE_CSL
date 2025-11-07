/**
 ****************************************************************************************
 *
 * @file app_params.h
 *
 * @brief Application parameters access header file
 *
 * Copyright (C) 2025 Renesas Electronics Corporation and/or its affiliates.
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

#ifndef APP_PARAMS_H_
#define APP_PARAMS_H_

#include <stdint.h>
#include "accessory_config.h"
#include "afmn_conn_params.h"
#include "fp_conn_params.h"

/**
 * \brief Application parameters enumeration
 */
typedef enum {
        APP_PARAMS_SERIAL_NUMBER = AFMN_CONN_PARAMS_SERIAL_NUMBER,
        APP_PARAMS_BLE_APP_NAME  = AFMN_CONN_PARAMS_MAX + FP_CONN_PARAMS_MAX,
#if (FP_FMDN == 1)
        APP_PARAMS_BEACON_TIME,
#endif
#if (dg_configSUOTA_ASYMMETRIC == 1)
        APP_PARAMS_ASYM_SUOTA_ADV_DATA,
        APP_PARAMS_ASYM_SUOTA_ADV_DATA_LEN,
        APP_PARAMS_ASYM_SUOTA_BD_ADDR_TYPE,
        APP_PARAMS_ASYM_SUOTA_BD_ADDR_ADDRESS,
        APP_PARAMS_ASYM_SUOTA_BD_ADDR_RENEW_DUR,
#endif
        APP_PARAMS_MAX
} APP_PARAMS;

/**
 * \brief Application parameter type
 */
typedef enum {
        APP_PARAMS_TYPE_AFMN,
        APP_PARAMS_TYPE_FP,
        APP_PARAMS_TYPE_OTHER
} APP_PARAMS_TYPE;

/**
 * \brief Multiple application parameters access struct
 */
typedef struct {
        APP_PARAMS param;               /**< Parameter type */
        void *data;                     /**< Pointer to stored parameter data */
        uint16_t len;                   /**< Requested length of parameter data */
        uint16_t ret_len;               /**< Accessed parameter data length */
} app_params_t;

/**
 * \brief Get application parameters
 *
 * \param [in] params set of application parameters to read
 * \param [in] n number of application parameters to read
 * \param [in] type technology type
 *
 * \return length of read data
 */
uint16_t app_params_get_params(app_params_t *params, uint8_t n, APP_PARAMS_TYPE type);

/**
 * \brief Set application parameters
 *
 * \param [in] params set of application parameters to write
 * \param [in] n number of application parameters to write
 * \param [in] type technology type
 *
 * \return length of written data
 */
uint16_t app_params_set_params(app_params_t *params, uint8_t n, APP_PARAMS_TYPE type);

#endif /* APP_PARAMS_H_ */
