/**
 ****************************************************************************************
 *
 * @file afmn.h
 *
 * @brief Apple FMN main header file
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

#ifndef AFMN_H_
#define AFMN_H_

#include <stdbool.h>
#include <stdint.h>
#include "sdk_defs.h"
#include "ble_common.h"
#include "ble_gap.h"
#include "afmn_errors.h"

/**
 * \brief Apple FMN product data length
 */
#define AFMN_PRODUCT_DATA_LEN           ( 8 )

/**
 * \brief AFMN accessory state
 */
typedef enum {
        AFMN_ACCESSORY_STATE_UNPAIRED,
        AFMN_ACCESSORY_STATE_CONNECTED,
        AFMN_ACCESSORY_STATE_NEARBY,
        AFMN_ACCESSORY_STATE_SEPARATED
} AFMN_ACCESSORY_STATE;

/**
 * \brief Callback to request afmn_execution() to be called by application task
 */
typedef void (*afmn_execution_cb_t)(void);

/**
 * \brief Callback to inform application task about accessory state
 *
 * \param [in] state current accessory state
 */
typedef void (*afmn_state_cb_t)(AFMN_ACCESSORY_STATE state);

/**
 * \brief Callback to indicate reset of internal attribute database
 */
typedef void (*afmn_db_reset_cb_t)(void);

/**
 * \brief Callback to notify about error that occurred inside Apple FMN framework
 *
 * Application is responsible to handle the error.
 *
 * \param [in] level error criticality level
 * \param [in] category error category
 * \param [in] code error code
 */
typedef void (*afmn_error_cb_t)(AFMN_ERROR_LEVEL level, AFMN_ERROR_CATEGORY category, int code);

/**
 * \brief Callback to indicate Apple FMN pairing process initiated
 *
 * \param [in] level error criticality level
 * \param [in] category error category
 * \param [in] code error code
 */
typedef void (*afmn_pair_init_cb_t)(void);

/**
 * \brief Callback to indicate new UTC time has been set
 *
 * \param [in] val UTC time in msec
 */
typedef void (*afmn_utc_cb_t)(uint64_t time_ms);

/**
 * \brief Apple FMN framework constant configuration structure
 */
typedef struct {
        const char *manufacturer_name;                          /**< Manufacturer name */
        const char *model_name;                                 /**< Model name */
        const uint8_t accessory_category;                       /**< Accessory category */
        const uint8_t product_data[AFMN_PRODUCT_DATA_LEN];      /**< Product data assigned to
                                                                     MFI product plan */
        const uint16_t fw_version_major;                        /**< Firmware major version number */
        const uint8_t fw_version_minor;                         /**< Firmware minor version number */
        const uint8_t fw_version_revision;                      /**< Firmware revision version number */
        const uint32_t accessory_capability;                    /**< Accessory capability */
        const uint8_t battery_type;                             /**< Battery type */
        const uint8_t battery_level_medium;                     /**< Battery level medium */
        const uint8_t battery_level_low;                        /**< Battery level low */
        const uint8_t battery_level_critical;                   /**< Battery level critical */
        const uint8_t adv_tx_power;                             /**< Advertise TX power */
        const int8_t tx_power_service_level;                    /**< TX power service level */
        const uint8_t sound_duration;                           /**< Play sound duration in seconds */
        const uint8_t max_connections;                          /**< Maximum supported connections */
        const uint8_t config_opt;                               /**< Configuration option */
} afmn_const_config_t;

/**
 * \brief Apple FMN framework configuration structure
 */
typedef struct {
        afmn_execution_cb_t execution_cb;               /**< Execution callback */
        afmn_state_cb_t state_cb;                       /**< State callback */
        afmn_db_reset_cb_t db_reset_cb;                 /**< Internal attribute database reset callback */
        afmn_error_cb_t error_cb;                       /**< Error callback */
        afmn_pair_init_cb_t pair_init_cb;               /**< Pairing initiated callback */
        afmn_utc_cb_t utc_cb;                           /**< UTC time set callback */
} afmn_config_t;

/**
 * \brief Initialize Apple FMN framework
 *
 * \param [in] cfg configuration structure
 */
void afmn_init(const afmn_config_t *cfg);

/**
 * \brief De-initialize Apple FMN framework
 *
 * This function releases allocated resources for Apple FMN framework.
 *
 * If suspend is set to true, part of framework's state is maintained, so that it can be resumed
 * when calling again afmn_init().
 * If suspend is set to false, then all resources are released and the framework cannot be
 * initialized again.
 *
 * \param [in] suspend true if framework is suspended
 */
void afmn_deinit(bool suspend);

/**
 * \brief Check if Apple FMN framework is initialized
 *
 * \return true if the framework is initialized, false otherwise
 */
bool afmn_is_initialized(void);

/**
 * \brief Apple FMN framework execution function
 */
void afmn_execution(void);

/**
 * \brief Perform factory reset
 */
void afmn_factory_reset(void);

/**
 * \brief Handle BLE event for the Apple FMN framework
 *
 * \param [in] evt BLE event
 *
 * \return true if event was handled, false if it needs to be handled by application
 */
bool afmn_handle_event(ble_evt_hdr_t *evt);

/**
 * \brief Add structures to Apple FMN scan response data
 *
 * \param [in] structs_count number of scan response structures to add
 * \param [in] structs array of scan response structures
 */
void afmn_set_scan_response(uint8_t structs_count, const gap_adv_ad_struct_t *structs);

/**
 * \brief Set serial number lookup
 *
 * This function enables or disables serial number lookup.
 *
 * \param [in] enable true to enable serial number lookup, false otherwise
 */
void afmn_set_serial_number_lookup(bool enable);

/**
 * \brief Check if serial number lookup is enabled
 *
 * \return true if serial number lookup is enabled, false otherwise
 */
bool afmn_is_serial_number_lookup_enabled(void);

/**
 * \brief Start pair mode
 *
 * This function starts pair mode. If accessory is already in pair mode or paired returns false.
 *
 * \return true if pair mode is entered, false otherwise
 */
bool afmn_start_pair_mode(void);

/**
 * \brief Check if pair mode is enabled
 *
 * \return true if pair mode is enabled, false otherwise
 */
bool afmn_is_pair_mode(void);

/**
 * \brief Get current accessory state
 *
 * \return accessory state
 */
AFMN_ACCESSORY_STATE afmn_get_accessory_state(void);

/**
 * \brief Add Apple FMN BLE services to internal attribute database
 *
 * This function creates and adds all instances of Apple FMN BLE services to the internal attribute
 * database.
 *
 * It can be used when AFMN_CONFIG_OPTION_BLE_DB_CONTROLLED_BY_APP is set as configuration option
 * for Apple FMN framework.
 *
 * This function can be called even if the framework is not initialized.
 */
void afmn_add_services_to_db(void);

/**
 * \brief Remove Apple FMN BLE services from internal attribute database
 *
 * This function removes all instances of Apple FMN BLE services from the internal attribute
 * database and frees all resources allocated by the services.
 *
 * It can be used when AFMN_CONFIG_OPTION_BLE_DB_CONTROLLED_BY_APP is set as configuration option
 * for Apple FMN framework.
 *
 * It should not be called if internal attribute database is cleaned up with ble_services_cleanup().
 *
 * This function can be called even if the framework is not initialized.
 */
void afmn_remove_services_from_db(void);

#endif /* AFMN_H_ */
