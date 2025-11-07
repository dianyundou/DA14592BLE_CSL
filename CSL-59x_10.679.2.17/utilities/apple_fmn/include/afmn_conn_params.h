/**
 ****************************************************************************************
 *
 * @file afmn_conn_params.h
 *
 * @brief Apple FMN connectivity parameters access header file
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

#ifndef AFMN_CONN_PARAMS_H_
#define AFMN_CONN_PARAMS_H_

#include <stdint.h>

/**
 * \brief Apple FMN connectivity parameters enumeration
 */
typedef enum {
        AFMN_CONN_PARAMS_SERIAL_NUMBER,
        AFMN_CONN_PARAMS_SW_AUTH_UUID,
        AFMN_CONN_PARAMS_SW_AUTH_TOKEN,
        AFMN_CONN_PARAMS_BLE_IS_PAIRED,
        AFMN_CONN_PARAMS_BLE_PRIMARY_KEY,
        AFMN_CONN_PARAMS_BLE_PRIMARY_KEY_INDEX,
        AFMN_CONN_PARAMS_BLE_LTK,
        AFMN_CONN_PARAMS_BLE_SECONDARY_KEY,
        AFMN_CONN_PARAMS_BLE_SECONDARY_KEY_INDEX,
        AFMN_CONN_PARAMS_BLE_P,
        AFMN_CONN_PARAMS_BLE_SKN,
        AFMN_CONN_PARAMS_BLE_SKS,
        AFMN_CONN_PARAMS_BLE_SHARED_KEY,
        AFMN_CONN_PARAMS_BLE_ICLOUD_ID,
        AFMN_CONN_PARAMS_MAX
} AFMN_CONN_PARAMS;

/**
 * \brief Multiple connectivity parameters access struct
 */
typedef struct {
        AFMN_CONN_PARAMS param;         /**< Connectivity parameter type */
        void *data;                     /**< Pointer to stored connectivity parameter data */
        uint16_t len;                   /**< Requested length of connectivity parameter data */
        uint16_t ret_len;               /**< Accessed connectivity parameter data length */
} afmn_conn_params_t;

/**
 * \brief Get Apple FMN connectivity parameters
 *
 * \param [in] params set of connectivity parameters to read
 * \param [in] n number of connectivity parameters to read
 *
 * \return length of read data
 */
uint16_t afmn_conn_params_get_params(afmn_conn_params_t *params, uint8_t n);

/**
 * \brief Set Apple FMN connectivity parameters
 *
 * \param [in] params set of connectivity parameters to write
 * \param [in] n number of connectivity parameters to write
 *
 * \return length of written data
 */
uint16_t afmn_conn_params_set_params(afmn_conn_params_t *params, uint8_t n);

#endif /* AFMN_CONN_PARAMS_H_ */
