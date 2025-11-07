/**
 * \addtogroup MID_INT_BLE_SERVICES
 * \{
 * \addtogroup BLE_SER_DEBUG Debug Service
 *
 * \brief Debug service API
 * \{
 */

/**
 ****************************************************************************************
 *
 * @file dlg_debug.h
 *
 * @brief Debug service API
 *
 * Copyright (C) 2015-2018 Renesas Electronics Corporation and/or its affiliates.
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

#ifndef DLG_DEBUG_H_
#define DLG_DEBUG_H_

#include "ble_service.h"


/**
 * \brief Create debug handler
 *
 * \param [in] CAT              category
 * \param [in] CMD              command
 * \param [in] CB               application callback
 * \param [in] UD               user data
 *
 */
#define DLGDEBUG_HANDLER(CAT, CMD, CB, UD)      \
        {                                       \
                .cat = (CAT),                   \
                .cmd = (CMD),                   \
                .cb = (CB),                     \
                .ud = (UD)                      \
        }

/**
 * \brief Command handler callback
 *
 * \param [in] conn_idx         connection index
 * \param [in] argc             arguments count
 * \param [in] argv             string arguments
 * \param [in] ud               user data
 *
 */
typedef void (* dlgdebug_call_cb_t) (uint16_t conn_idx, int argc, char **argv, void *ud);

/**
 * Debug handler
 */
typedef struct {
        /** Category */
        const char *cat;
        /** Command */
        const char *cmd;
        /** Callback */
        dlgdebug_call_cb_t cb;
        /** User-data */
        void *ud;
} dlgdebug_handler_t;

/**
 * \brief Register DLG debug instance
 *
 * Function registers DLG debug service with UUID 6b559111-c4df-4660-818e-234f9e17b290.
 *
 * \param [in] cfg              general service configuration
 *
 * \return service instance
 *
 */
ble_service_t *dlgdebug_init(const ble_service_config_t *cfg);

/**
 * \brief Register handler
 *
 * Registers handler when command is written to CP characteristic of Debug Service.
 * Command has format "<cat> <cmd> <...>".
 * Characteristic UUID: 6b559111-c4df-4660-818e-234f9e17b291.
 *
 * \param [in] svc              service instance
 * \param [in] cat              category
 * \param [in] cmd              command
 * \param [in] cb               application callback
 * \param [in] ud               user data
 *
 */
void dlgdebug_register_handler(ble_service_t *svc, const char *cat, const char *cmd,
                                                                dlgdebug_call_cb_t cb, void *ud);

/**
 * \brief Register handlers
 *
 * Registers handlers array. See dlgdebug_register_handler function's description for more details.
 *
 * \param [in] svc              service instance
 * \param [in] len              array length
 * \param [in] handlers         debug handlers array
 *
 */
void dlgdebug_register_handlers(ble_service_t *svc, size_t len, const dlgdebug_handler_t *handlers);

/**
 * \brief Notification strings
 *
 * Send notification to connected peer.
 *
 * \param [in] svc              service instance
 * \param [in] conn_idx         connection index
 * \param [in] fmt              string data
 *
 */
void dlgdebug_notify_str(ble_service_t *svc, uint16_t conn_idx, const char *fmt, ...);

#endif /* DLG_DEBUG_H_ */
/**
 \}
 \}
 */
