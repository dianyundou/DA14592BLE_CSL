/**
 ****************************************************************************************
 *
 * @file asym_suota_config_storage.h
 *
 * @brief Asymmetric SUOTA configuration storage header
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

#ifndef ASYM_SUOTA_CONFIG_STORAGE_H_
#define ASYM_SUOTA_CONFIG_STORAGE_H_

#include "ble_gap.h"
#include "ad.h"

#ifndef ASYM_SUOTA_USE_CONFIG_STORAGE_GENERIC_PART
#define ASYM_SUOTA_USE_CONFIG_STORAGE_GENERIC_PART      ( 0 )
#endif

/**
 * \brief Asymmetric SUOTA configuration status
 */
typedef enum {
        ASUOTA_CONFIG_ERR_NO_ERROR,           /**< Success */
        ASUOTA_CONFIG_ERR_PART_NOT_FOUND,     /**< Partition could not be opened / found */
        ASUOTA_CONFIG_ERR_CONF_UNKNOWN,       /**< Configuration is not known to the NVPARAM */
        ASUOTA_CONFIG_ERR_CONF_NOT_FOUND,     /**< Configuration is not found / valid in storage */
        ASUOTA_CONFIG_ERR_CONF_INVALID_LENGTH,/**< Invalid length was provided */
        ASUOTA_CONFIG_ERR_CONF_INVALID_VALUE, /**< Invalid value was retrieved from storage */
        ASUOTA_CONFIG_ERR_ALLOC_ERROR,        /**< Insufficient space */
} ASUOTA_CONFIG_ERR;

/**
 * \brief Security requirements for connected devices
 */
typedef enum {
        ASUOTA_CONFIG_SECUR_REQ_LVL_1         = 0x00,               /**< Corresponds to GAP_SEC_LEVEL_1 */
        ASUOTA_CONFIG_SECUR_REQ_LVL_2         = 0x01,               /**< Corresponds to GAP_SEC_LEVEL_2 */
        ASUOTA_CONFIG_SECUR_REQ_LVL_3         = 0x02,               /**< Corresponds to GAP_SEC_LEVEL_3 */
        ASUOTA_CONFIG_SECUR_REQ_LVL_4         = 0x03,               /**< Corresponds to GAP_SEC_LEVEL_4 */

        ASUOTA_CONFIG_SECUR_REQ_LVL_MSK       = (1 << 0) | (1 << 1),/**< Security level mask */
        ASUOTA_CONFIG_SECUR_REQ_EN_NON_BONDED = (1 << 7),           /**< Flag that enables non bonded devices to
                                                                         connect if there are no other bonded */
} ASUOTA_CONFIG_SECUR_REQ;

/**
 * \brief Initializes asymmetric SUOTA configuration storage
 */
void asym_suota_config_storage_init(void);

/**
 * \brief Retrieves advertise timeout
 *
 * \param[out] adv_tmo                  Advertise timeout in msec
 *
 * \return Asymmetric SUOTA configuration status
 */
ASUOTA_CONFIG_ERR asym_suota_config_storage_get_adv_timeout(uint32_t *adv_tmo);

/**
 * \brief Sets advertise timeout in configuration storage
 *
 * \param[in] adv_tmo                   Advertise timeout in msec
 *
 * \return Asymmetric SUOTA configuration status
 */
ASUOTA_CONFIG_ERR asym_suota_config_storage_set_adv_timeout(uint32_t adv_tmo);

/**
 * \brief Retrieves advertise interval configuration
 *
 * \param[out] adv_intv_min             Advertise interval minimum
 * \param[out] adv_intv_max             Advertise interval maximum
 *
 * \return Asymmetric SUOTA configuration status
 */
ASUOTA_CONFIG_ERR asym_suota_config_storage_get_adv_intv(uint16_t *adv_intv_min, uint16_t *adv_intv_max);

/**
 * \brief Sets advertise interval configuration in configuration storage
 *
 * \param[in] adv_intv_min              Advertise interval minimum
 * \param[in] adv_intv_max              Advertise interval maximum
 *
 * \return Asymmetric SUOTA configuration status
 */
ASUOTA_CONFIG_ERR asym_suota_config_storage_set_adv_intv(uint16_t adv_intv_min, uint16_t adv_intv_max);

/**
 * \brief Retrieves advertise channel map configuration
 *
 * \param[out] chnl_map                 Channel map
 *
 * \return Asymmetric SUOTA configuration status
 */
ASUOTA_CONFIG_ERR asym_suota_config_storage_get_adv_chnl_map(uint8_t *chnl_map);

/**
 * \brief Sets advertise channel map configuration in configuration storage
 *
 * \param[in] chnl_map                  Channel map
 *
 * \return Asymmetric SUOTA configuration status
 */
ASUOTA_CONFIG_ERR asym_suota_config_storage_set_adv_chnl_map(uint8_t chnl_map);

/**
 * \brief Retrieves advertise data
 *
 * \note Advertise data need to be freed by caller
 *
 * \param[out] adv_data_len             Advertise data length in bytes
 * \param[out] adv_data                 Advertise data
 *
 * \return Asymmetric SUOTA configuration status
 */
ASUOTA_CONFIG_ERR asym_suota_config_storage_get_adv_data(uint8_t *adv_data_len, const uint8_t **adv_data);

/**
 * \brief Sets advertise data in configuration storage
 *
 * \param[in] adv_data_len              Advertise data length in bytes
 * \param[in] adv_data                  Advertise data
 *
 * \return Asymmetric SUOTA configuration status
 */
ASUOTA_CONFIG_ERR asym_suota_config_storage_set_adv_data(uint8_t adv_data_len, const uint8_t *adv_data);

/**
 * \brief Retrieves scan response data
 *
 * \note Scan response data need to be freed by caller
 *
 * \param[out] scan_rsp_len             Scan response data length in bytes
 * \param[out] scan_rsp                 Scan response data
 *
 * \return Asymmetric SUOTA configuration status
 */
ASUOTA_CONFIG_ERR asym_suota_config_storage_get_scan_rsp(uint8_t *scan_rsp_len, const uint8_t **scan_rsp);

/**
 * \brief Sets scan response data in configuration storage
 *
 * \param[in] scan_rsp_len              Scan response data length in bytes
 * \param[in] scan_rsp                  Scan response data
 *
 * \return Asymmetric SUOTA configuration status
 */
ASUOTA_CONFIG_ERR asym_suota_config_storage_set_scan_rsp(uint8_t scan_rsp_len, const uint8_t *scan_rsp);

/**
 * \brief Retrieves device name
 *
 * \note Device name needs to be freed by caller
 *
 * \param[out] dev_name                 Device name
 *
 * \return Asymmetric SUOTA configuration status
 */
ASUOTA_CONFIG_ERR asym_suota_config_storage_get_device_name(const char **dev_name);

/**
 * \brief Sets device name in configuration storage
 *
 * \param[in] dev_name                  Device name
 *
 * \return Asymmetric SUOTA configuration status
 */
ASUOTA_CONFIG_ERR asym_suota_config_storage_set_device_name(const char *dev_name);

/**
 * \brief Retrieves MTU size configuration
 *
 * \param[out] mtu_size                 MTU size in octets
 *
 * \return Asymmetric SUOTA configuration status
 */
ASUOTA_CONFIG_ERR asym_suota_config_storage_get_mtu_size(uint16_t *mtu_size);

/**
 * \brief sets MTU size configuration in configuration storage
 *
 * \param[in] mtu_size                  MTU size in octets
 *
 * \return Asymmetric SUOTA configuration status
 */
ASUOTA_CONFIG_ERR asym_suota_config_storage_set_mtu_size(uint16_t mtu_size);

/**
 * \brief Retrieves connection parameter settings
 *
 * \param[out] conn_params              GAP connection parameters struct
 *
 * \return Asymmetric SUOTA configuration status
 */
ASUOTA_CONFIG_ERR asym_suota_config_storage_get_conn_param(gap_conn_params_t *conn_params);

/**
 * \brief sets connection parameter settings in configuration storage
 *
 * \param[in] conn_params               GAP connection parameters struct
 *
 * \return Asymmetric SUOTA configuration status
 */
ASUOTA_CONFIG_ERR asym_suota_config_storage_set_conn_param(const gap_conn_params_t *conn_params);

/**
 * \brief Retrieves security requirement setting
 *
 * \param[out] req                      Security requirement
 *
 * \return Asymmetric SUOTA configuration status
 */
ASUOTA_CONFIG_ERR asym_suota_config_storage_get_sec_reqs(ASUOTA_CONFIG_SECUR_REQ *req);

/**
 * \brief Sets security requirement setting in configuration storage
 *
 * \param[in] req                       Security requirement
 *
 * \return Asymmetric SUOTA configuration status
 */
ASUOTA_CONFIG_ERR asym_suota_config_storage_set_sec_reqs(ASUOTA_CONFIG_SECUR_REQ req);

/**
 * \brief Retrieves device's own BD address settings
 *
 * Depending on the address type, the following cases of address and renew duration are used
 *
 *  Address type                               |  Address   |  Renew duration
 * ------------------------------------------- | :--------: | :--------------:
 *  Public Static                              |  -         |  -
 *  Private Static                             |  Mandatory |  -
 *  Private Random Resolvable                  |  Optional  |  Mandatory
 *  Private Random Non-resolvable              |  Optional  |  Mandatory
 *  Private Random Resolvable LE privacy v1.2  |  Optional  |  Mandatory
 *
 * \param[out] addr                     Own address and its type
 * \param[out] renew_dur                Address renewal period in seconds
 *
 * \return Asymmetric SUOTA configuration status
 */
ASUOTA_CONFIG_ERR asym_suota_config_storage_get_own_address(own_address_t *addr, uint16_t *renew_dur);

/**
 * \brief Sets device's own BD address settings
 *
 * Depending on the address type, the following cases of address and renew duration are used
 *
 *  Address type                               |  Address   |  Renew duration
 * ------------------------------------------- | :--------: | :--------------:
 *  Public Static                              |  -         |  -
 *  Private Static                             |  Mandatory |  -
 *  Private Random Resolvable                  |  Optional  |  Mandatory
 *  Private Random Non-resolvable              |  Optional  |  Mandatory
 *  Private Random Resolvable LE privacy v1.2  |  Optional  |  Mandatory
 *
 * \note In cases of Private Random Resolvable and Private Random Non-resolvable, if a non null
 * address (00:00:00:00:00:00) is set, this address is used to advertise. If null, the address is
 * generated by the stack.
 *
 * \param[in] addr                      Own address and its type
 * \param[in] renew_dur                 Address renewal period in seconds
 *
 * \return Asymmetric SUOTA configuration status
 */
ASUOTA_CONFIG_ERR asym_suota_config_storage_set_own_address(const own_address_t *addr, uint16_t renew_dur);

#endif /* ASYM_SUOTA_CONFIG_STORAGE_H_ */
