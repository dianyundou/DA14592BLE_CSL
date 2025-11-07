/**
 * \addtogroup MID_INT_BLE_SERVICES
 * \{
 * \addtogroup BLE_SER_DIS Device Information Service
 *
 * \brief Device information service sample implementation API
 * \{
 */

/**
 ****************************************************************************************
 *
 * @file dis.h
 *
 * @brief Device Information Service sample implementation API
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

#ifndef DIS_H_
#define DIS_H_

#include <stdbool.h>
#include <stdint.h>
#include "ble_service.h"

/**
 * Device Information Service system ID
 */
typedef struct {
        uint8_t manufacturer[5];
        uint8_t oui[3];
} dis_system_id_t;

/**
 * Device Information Service PNP ID
 */
typedef struct {
        uint8_t   vid_source;
        uint16_t  vid;
        uint16_t  pid;
        uint16_t  version;
} dis_pnp_id_t;

/**
 * Device Information Service device information
 */
typedef struct {
        const char              *manufacturer;          /**< Manufacturer Name String */
        const char              *model_number;          /**< Model Number String */
        const char              *serial_number;         /**< Serial Number String */
        const char              *hw_revision;           /**< Hardware Revision String */
        const char              *fw_revision;           /**< Firmware Revision String */
        const char              *sw_revision;           /**< Software Revision String */
        const dis_system_id_t   *system_id;             /**< System ID */
        uint16_t                reg_cert_length;        /**< Regulatory Certification Length */
        const uint8_t           *reg_cert;              /**< Regulatory Certification */
        const dis_pnp_id_t      *pnp_id;                /**< PnP ID */
} dis_device_info_t;

/**
 * \brief Register Device Information Service instance
 *
 * Function registers Device Information Service.
 *
 * Read access authorization mode is by default disabled.
 *
 * \param [in] config           general service configuration
 * \param [in] info             general information about device
 *
 * \return service instance
 *
 */
ble_service_t *dis_init(const ble_service_config_t *config, const dis_device_info_t *info);

/**
 * \brief Authorize read access to service characteristics for connection
 *
 * This function controls read access to service characteristics of a DIS instance for an
 * authorized connection. Authorization mode is enabled for the DIS instance if a connection
 * is authorized for read access.
 *
 * If BLE_CONN_IDX_INVALID is passed to conn_idx argument, read access authorization mode is
 * enabled or disabled for the particular DIS instance. Authorization is removed for all
 * connections.
 *
 * \param [in] svc              service instance
 * \param [in] conn_idx         connection index
 * \param [in] enable           true to enable read access, false otherwise
 *
 * \return 0 if authorized access has been updated successfully, other value otherwise
 */
int dis_set_authorized_read(ble_service_t *svc, uint16_t conn_idx, bool enable);

/**
 * \brief Check if a connection is authorized to read service characteristics
 *
 * \param [in] svc              service instance
 * \param [in] conn_idx         connection index
 */
bool dis_is_conn_authorized(ble_service_t *svc, uint16_t conn_idx);

/**
 * \brief Set firmware revision
 *
 * \param [in] svc              service instance
 * \param [in] fw_revision      firmware revision
 *
 * \return BLE_STATUS_OK if success
 */
ble_error_t dis_set_fw_revision(ble_service_t *svc, const char *fw_revision);

#endif /* DIS_H_ */
/**
 \}
 \}
 */
