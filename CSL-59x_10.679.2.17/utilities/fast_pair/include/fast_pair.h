/**
 ****************************************************************************************
 *
 * @file fast_pair.h
 *
 * @brief Google Fast Pair framework header file
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

#ifndef FAST_PAIR_H_
#define FAST_PAIR_H_

#include <stdbool.h>
#include <stdint.h>
#include FAST_PAIR_CONFIG_FILE
#include "fp_defaults.h"
#include "ble_common.h"
#include "ble_gap.h"
#include "ble_service.h"

/**
 * \brief Fast Pair request status
 */
typedef enum {
        FP_PAIR_REQ_STAT_INITIATED,
        FP_PAIR_REQ_STAT_COMPLETED
} FP_PAIR_REQ_STAT;

#if (FP_FMDN == 1)
/**
 * \brief FMDN provisioning status
 */
typedef enum {
        FP_FMDN_PROV_STAT_STOPPED,
        FP_FMDN_PROV_STAT_STARTED
} FP_FMDN_PROV_STAT;
#endif /* FP_FMDN */

/**
 * \brief Fast Pair firmware status
 */
typedef enum {
        FP_FW_NORMAL,
        FP_FW_UPDATING,
        FP_FW_ABNORMAL
} FP_FW_STATUS;

/**
 * \brief Callback to request fp_execution() to be called by application task
 */
typedef void (*fp_execution_cb_t)(void);

/**
 * \brief Callback to get current beacon time
 *
 * \return beacon time in seconds
 */
typedef uint32_t (*fp_beacon_time_cb_t)(void);

#if (FP_LOCATOR_TAG != 1)
/**
 * \brief Callback to notify about Fast Pair pairing status
 *
 * \param [in] conn_idx connection index
 * \param [in] status 0 if success, other value otherwise
 */
typedef void (*fp_pair_status_cb_t)(uint16_t conn_idx, uint8_t stat);
#endif /* !FP_LOCATOR_TAG */

/**
 * \brief Callback to notify about Fast Pair request status
 *
 * This is called once Seeker Fast Pair request is initiated or completed.
 *
 * \param [in] conn_idx connection index
 * \param [in] stat Fast Pair request status
 * \param [in] err Fast Pair request error (0: no error)
 */
typedef void (*fp_pair_req_status_cb_t)(uint16_t conn_idx, FP_PAIR_REQ_STAT stat, uint8_t err);

#if (FP_FMDN == 1)
/**
 * \brief Callback to notify about FMDN provisioning status
 *
 * This is called once device has started or stopped being FMDN provisioned.
 *
 * \param [in] stat FMDN provisioning status
 */
typedef void (*fp_fmdn_prov_status_cb_t)(FP_FMDN_PROV_STAT stat);

/**
 * \brief Callback to notify about new authenticated connection
 *
 * This is called once connection has been authenticated.
 *
 * \param [in] conn_idx connection index
 */
typedef void (*fp_auth_conn_cb_t)(uint16_t conn_idx);
#endif /* FP_FMDN */

/**
 * \brief Callback to notify about error that occurred during Fast Pair framework execution
 *
 * Application is responsible to handle the error.
 *
 * \param [in] error_code error code
 */
typedef void (*fp_error_cb_t)(int error_code);

#if (FP_BATTERIES_COUNT != 0)
/**
 * \brief Battery information structure
 */
typedef struct {
        bool is_charging;       /**< Battery is charging */
        uint8_t level;          /**< Battery level */
} fp_battery_info_t;
#endif

/**
 * \brief Google Fast Pair framework configuration structure
 */
typedef struct {
        fp_execution_cb_t execution_cb;                 /**< Execution callback */
#if (FP_FMDN == 1)
        fp_beacon_time_cb_t beacon_time_cb;             /**< Beacon time callback */
#endif
#if (FP_LOCATOR_TAG != 1)
        fp_pair_status_cb_t pair_status_cb;             /**< Fast Pair pairing status callback
                                                             (applicable for non-locator tag device) */
#endif
        fp_pair_req_status_cb_t pair_req_status_cb;     /**< Fast Pair request status callback */
#if (FP_FMDN == 1)
        fp_fmdn_prov_status_cb_t fmdn_prov_status_cb;   /**< FMDN provisioning status callback */
        fp_auth_conn_cb_t auth_conn_cb;                 /**< Authenticated connection indication callback */
#endif
        fp_error_cb_t error_cb;                         /**< Error callback */
#if (FP_BATTERIES_COUNT != 0)
        const fp_battery_info_t *batt_info;             /**< Battery information */
#endif
        bool start_adv;                                 /**< Start advertising */
} fp_cfg_t;

/**
 * \brief Initialize Google Fast Pair framework
 *
 * execution_cb and beacon_time_cb callbacks in configuration structure are mandatory and cannot
 * be set to NULL. This  function registers the following services:
 * 1) Google Fast Pair service (0xFE2C)
 * 2) Accessory Non-owne service (15190001-12F4-C226-88ED-2AC5579F2A85)
 * 3) Device Information service (0x180A)
 *
 * \param [in] cfg configuration structure
 *
 * \return 0 if success, other value otherwise
 */
int fp_init(const fp_cfg_t *cfg);

/**
 * \brief De-initialize Google Fast Pair framework
 *
 * This function releases allocated resources for Google Fast Pair framework.
 */
void fp_deinit(void);

/**
 * \brief Check if Google Fast Pair framework is initialized
 *
 * \return true if the framework is initialized, false otherwise
 */
bool fp_is_initialized(void);

/**
 * \brief Google Fast Pair framework execution function
 *
 * \return 0 if success, other value otherwise
 */
int fp_execution(void);

/**
 * \brief Perform factory reset
 *
 * All stored keys are wiped out.
 *
 * \return 0 if success, other value otherwise
 */
int fp_factory_reset(void);

/**
 * \brief Set pairing mode
 *
 * This function enables or disables pairing mode. In case of a locator tag device, it will return
 * false if FMDN is activated for the device.
 *
 * \param [in] enable true to enable pairing mode, false otherwise
 *
 * \return true if new pairing mode setting is applied, false otherwise
 */
bool fp_set_pairing_mode(bool enable);

/**
 * \brief Check if pairing mode is enabled
 *
 * \return true if pairing mode is enabled, false otherwise
 */
bool fp_is_pairing_mode(void);

/**
 * \brief Handle BLE event for the Google Fast Pair framework
 *
 * \param [in] evt BLE event
 *
 * \return true if event was handled, false if it needs to be handled by application
 */
bool fp_handle_event(const ble_evt_hdr_t *evt);

/**
 * \brief Start Fast Pair/FMDN advertise
 *
 * \return 0 if success, other value otherwise
 */
int fp_start_advertise(void);

/**
 * \brief Stop Fast Pair/FMDN advertise
 *
 * \return 0 if success, other value otherwise
 */
int fp_stop_advertise(void);

/**
 * \brief Check if Fast Pair/FMDN advertise is stopped
 *
 * \return true if it is stopped, false otherwise
 */
bool fp_is_advertise_stopped(void);

#if (FP_FMDN == 1)
/**
 * \brief Set advertise interval
 *
 * Advertise interval can be set a value from 500ms to 4000ms. After the call, advertise is
 * restarted with the new settings.
 *
 * \param [in] interval advertise interval in ms, value must be multiple of 250ms
 *
 * \return 0 if success, other value otherwise
 */
int fp_set_advertise_interval(uint16_t interval);
#endif

/**
 * \brief Add custom advertise structures to Fast Pair advertise data
 *
 * \param [in] structs_count number of advertise structures to add
 * \param [in] structs array of advertise structures
 *
 * \return 0 if advertise structures added successfully, other value otherwise
 */
int fp_add_custom_advertise(uint8_t structs_count, const gap_adv_ad_struct_t *structs);

/**
 * \brief Add structures to Fast Pair scan response data
 *
 * \param [in] structs_count number of scan response structures to add
 * \param [in] structs array of scan response structures
 *
 * \return 0 if success, other value otherwise
 */
int fp_set_scan_response(uint8_t structs_count, const gap_adv_ad_struct_t *structs);

#if (FP_LOCATOR_TAG != 1)
/**
 * \brief Set account key filter UI indication
 *
 * \param [in] enable true to show account key filter UI indication, false otherwise
 */
void fp_set_acc_key_filter_ui_indication(bool enable);
#endif /* !FP_LOCATOR_TAG */

#if (FP_BATTERIES_COUNT != 0)
/**
 * \brief Set current battery information
 *
 * \param [in] info array of battery information for each battery
 *
 * \note The battery, the information of which is located in the first entry of the array,
 *       is considered as the main battery.
 *
 * \return 0 if success, other value otherwise
 */
int fp_set_battery_information(const fp_battery_info_t info[FP_BATTERIES_COUNT]);

#if (FP_BATTERY_NOTIFICATION == 1)
/**
 * \brief Set battery UI indication
 *
 * \param [in] enable true to show battery UI indication, false otherwise
 */
void fp_set_battery_ui_indication(bool enable);
#endif /* FP_BATTERY_NOTIFICATION */
#endif /* FP_BATTERIES_COUNT */

#if (FP_FMDN == 1)
#if (FP_FMDN_RING_COMPONENTS_NUM > 0)
/**
 * \brief Stop ringing initiated by a button press
 *
 * This function should only be called if ringing is stopped via button press.
 *
 * \return 0 if success, other value otherwise
 */
int fp_stop_ringing(void);

/**
 * \brief Check if ringing is enabled
 *
 * \return true if ringing is enabled, false otherwise
 */
bool fp_is_ringing(void);
#endif /* FP_FMDN_RING_COMPONENTS_NUM */

/**
 * \brief Set user consent mode
 *
 * \param [in] enable true to enable user consent mode, false otherwise
 */
void fp_set_user_consent(bool enable);

/**
 * \brief Check FMDN provisioning state
 *
 * \return true if provisioned, false otherwise
 */
bool fp_is_fmdn_provisioned(void);

/**
 * \brief Check if connection is authenticated
 *
 * \return true if authenticated, false otherwise
 */
bool fp_is_conn_authenticated(uint16_t conn_idx);
#endif /* FP_FMDN */

/**
 * \brief Set firmware status
 *
 * This function updates the firmware revision string in the registered Device Information Service
 * (DIS).
 *
 * If FP_DIS_ENABLE is set to 1, dis argument is ignored and the DIS instance which has been
 * registered by the Google Fast Pair framework will be used instead. Authorized read access to
 * DIS characteristics is always granted for the specified connection.
 *
 * \param [in] dis Device Information Service (DIS) instance
 * \param [in] conn_idx connection index
 * \param [in] status current status of the firmware
 *
 * \return 0 if success, other value otherwise
 */
int fp_set_firmware_status(ble_service_t *dis, uint16_t conn_idx, FP_FW_STATUS status);

/**
 * \brief Add Google Fast Pair BLE services to internal attribute database
 *
 * This function creates and adds instances for all Google Fast Pair BLE services to the internal
 * attribute database.
 *
 * It can be used when FP_CONFIG_OPTION_BLE_DB_CONTROLLED_BY_APP is set as configuration option
 * for Google Fast Pair framework.
 *
 * This function can be called even if the framework is not initialized.
 */
void fp_add_services_to_db(void);

/**
 * \brief Remove Google Fast Pair BLE services from internal attribute database
 *
 * This function removes all instances of Google Fast Pair BLE services from the internal attribute
 * database and frees all resources allocated by the services.
 *
 * It can be used when FP_CONFIG_OPTION_BLE_DB_CONTROLLED_BY_APP is set as configuration option
 * for Google Fast Pair framework.
 *
 * It should not be called if internal attribute database is cleaned up with ble_services_cleanup().
 *
 * This function can be called even if the framework is not initialized.
 */
void fp_remove_services_from_db(void);

#endif /* FAST_PAIR_H_ */
