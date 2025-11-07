/**
 ****************************************************************************************
 *
 * @file fp_conn_params.h
 *
 * @brief Google Fast Pair connectivity parameters access header file
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

#ifndef FP_CONN_PARAMS_H_
#define FP_CONN_PARAMS_H_

#include <stdint.h>
#include FAST_PAIR_CONFIG_FILE
#include "fp_defaults.h"

/**
 * \brief Fast Pair connectivity parameters enumeration
 */
typedef enum {
#ifdef FP_PERSONALIZED_NAME
        FP_CONN_PARAMS_BLE_PERSONALIZED_NAME,
#endif
#if (FP_FMDN == 1)
        FP_CONN_PARAMS_BLE_EPHEMERAL_IDENTITY_KEY,
#endif
        FP_CONN_PARAMS_BLE_ACCOUNT_KEY_LIST,
        FP_CONN_PARAMS_MAX
} FP_CONN_PARAMS;

/**
 * \brief Multiple connectivity parameters access struct
 */
typedef struct {
        FP_CONN_PARAMS param;           /**< Connectivity parameter type */
        void *data;                     /**< Pointer to stored connectivity parameter data */
        uint16_t len;                   /**< Requested length of connectivity parameter data */
        uint16_t ret_len;               /**< Accessed connectivity parameter data length */
} fp_conn_params_t;

/**
 * \brief Get Fast Pair connectivity parameters
 *
 * \param [in] params set of connectivity parameters to read
 * \param [in] n number of connectivity parameters to read
 *
 * \return length of read data
 */
uint16_t fp_conn_params_get_params(fp_conn_params_t *params, uint8_t n);

/**
 * \brief Set Fast Pair connectivity parameters
 *
 * \param [in] params set of connectivity parameters to write
 * \param [in] n number of connectivity parameters to write
 *
 * \return length of written data
 */
uint16_t fp_conn_params_set_params(fp_conn_params_t *params, uint8_t n);

#endif /* FP_CONN_PARAMS_H_ */
