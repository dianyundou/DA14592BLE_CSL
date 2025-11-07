/**
 ****************************************************************************************
 *
 * @file anos.h
 *
 * @brief Accessory Non-Owner Service implementation API
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

#ifndef ANOS_H_
#define ANOS_H_

#include <stdint.h>
#include "ble_service.h"
#include FAST_PAIR_CONFIG_FILE
#include "fp_defaults.h"

#if (FP_FMDN == 1)
/**
 * \brief ANOS application write request (to accessory)
 */
typedef void (*write_request_cb_t)(ble_service_t *svc, uint16_t conn_idx, uint16_t opcode);

/**
 * \brief Register Accessory Non-Owner Service instance
 *
 * Function registers ANOS instance.
 * \param [in] cb pointer to Accessory Non-Owner service request write callback
 *
 * \return service instance
 */
ble_service_t *anos_init(write_request_cb_t non_owner_cb);

/**
 * \brief Indicate client with response from accessory
 *
 * \param [in] svc      service instance
 * \param [in] conn_idx connection index
 * \param [in] response response message to accessory
 * \param [in] length   response's length
 *
 * \return result code
 */
ble_error_t anos_indicate_response(ble_service_t *svc, uint16_t conn_idx, uint8_t *response, uint8_t length);
#endif /* FP_FMDN */

#endif /* ANOS_H_ */
