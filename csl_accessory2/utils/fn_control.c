/**
 ****************************************************************************************
 *
 * @file fn_control.c
 *
 * @brief Finder network control implementation
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

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "sdk_defs.h"
#include "osal.h"

#include "accessory_config.h"

#include "sys_timer.h"
#include "ble_service.h"
#include "app_params.h"
#include "notification_bits.h"
#include "adv_control.h"

#include "afmn.h"
#include "afmn_os_port.h"
#include "fast_pair.h"

#include "fn_control.h"

/* Finder network control module context */
__RETAINED static struct {
        uint32_t notif;
        OS_TASK task_hdl;

        fn_control_state_cb_t state_cb;
        fn_control_pair_stop_cb_t pair_stop_cb;
        fn_control_reset_db_cb_t db_reset_cb;
        fn_control_get_batt_level_cb_t batt_level_get_cb;
        fn_control_fp_auth_conn_cb_t fp_auth_conn_cb;

        bool bt_addr_change_postponed;

        uint8_t scan_rsp_struct_count;
        const gap_adv_ad_struct_t *scan_rsp_structs;

        /* Timer used to stop pairing mode (Google Fast Pair) */
        OS_TIMER pairing_mode_tim;
        /* Timer used to handle serial number lookup */
        OS_TIMER serial_number_lookup_tim;
#if (FP_FMDN == 1)
        /* Timer used to handle beacon time update in NVM storage */
        OS_TIMER beacon_time_storage_tim;
        /* Beacon time offset read from NVM storage */
        uint32_t beacon_time_offset;
#endif
        FINDER_NETWORK_STATE finder_network_state;
} fn_ctx;

#if (FP_FMDN == 1)
static uint32_t fp_beacon_time_cb(void);
#endif

/* Notify control OS task */
static void notify_control_task(uint32_t notif)
{
        OS_TASK_NOTIFY(fn_ctx.task_hdl, notif, OS_NOTIFY_SET_BITS);
}

#if (FP_FMDN == 1)
/* Beacon time storage timer callback */
static void beacon_time_storage_tim_cb(OS_TIMER timer)
{
        notify_control_task(BEACON_TIME_TMO_NOTIF);
}
#endif /* FP_FMDN */

/* Pairing mode timeout timer callback */
static void pairing_mode_tim_cb(OS_TIMER timer)
{
        notify_control_task(PAIRING_MODE_TMO_NOTIF);
}

/* Serial number lookup timeout timer callback */
static void serial_number_lookup_tim_cb(OS_TIMER timer)
{
        notify_control_task(SERIAL_NUMBER_TMO_NOTIF);
}

#if (FP_FMDN == 1)
/* Initialize beacon time with the value stored in NVM storage */
static void init_beacon_time(void)
{
        uint16_t read_len = 0;

        app_params_t params[] = {
                { .param = APP_PARAMS_BEACON_TIME,
                  .data = &fn_ctx.beacon_time_offset,
                  .len = sizeof(fn_ctx.beacon_time_offset) }
        };
        read_len = app_params_get_params(params, ARRAY_LENGTH(params), APP_PARAMS_TYPE_OTHER);

        if (read_len == 0) {
                fn_ctx.beacon_time_offset = 0;
        }
}
/* Write beacon time to NVM storage */
static void write_beacon_time(void)
{
        uint16_t write_len = 0;
        uint32_t beacon_time = fp_beacon_time_cb();

        app_params_t params[] = {
                { .param = APP_PARAMS_BEACON_TIME,
                  .data = &beacon_time,
                  .len = sizeof(beacon_time) }
        };
        write_len = app_params_set_params(params, ARRAY_LENGTH(params), APP_PARAMS_TYPE_OTHER);

        if (write_len != sizeof(beacon_time)) {
                CSLA_TASK_PRINTF("Problem in writing beacon time to NVM storage\r\n");
                OS_ASSERT(0);
        }
}
#endif /* FP_FMDN */

#ifdef CONFIG_RETARGET
static char *afmn_state_str(AFMN_ACCESSORY_STATE state)
{
        switch (state) {
        case AFMN_ACCESSORY_STATE_UNPAIRED:
                return "UNPAIRED";
        case AFMN_ACCESSORY_STATE_CONNECTED:
                return "CONNECTED";
        case AFMN_ACCESSORY_STATE_NEARBY:
                return "NEARBY";
        case AFMN_ACCESSORY_STATE_SEPARATED:
                return "SEPARATED";
        default:
                return "";
        }
}
#endif /* CONFIG_RETARGET */

/* Apple FMN framework execution callback */
static void afmn_execution_cb(void)
{
        notify_control_task(APPLE_FMN_NOTIF);
}

/* Apple FMN accessory state callback */
static void afmn_state_cb(AFMN_ACCESSORY_STATE state)
{
        FINDER_NETWORK_STATE prev_fn_state;

        prev_fn_state = fn_ctx.finder_network_state;

        CSLA_TASK_PRINTF("FindMy is in state: %s\r\n", afmn_state_str(state));

        if (state != AFMN_ACCESSORY_STATE_UNPAIRED) {
                /* Set Apple FMN as active finder network */
                fn_ctx.finder_network_state = FINDER_NETWORK_AFMN;
        } else {
                /* Reset finder network state */
                fn_ctx.finder_network_state = FINDER_NETWORK_NONE;
        }

        if (prev_fn_state != fn_ctx.finder_network_state) {
                notify_control_task(FINDER_NETWORK_STATE_NOTIF);
        }
}

/* Internal attribute database reset callback */
static void afmn_db_reset_cb(void)
{
        ble_services_cleanup();

        afmn_add_services_to_db();
        fp_add_services_to_db();

        if (fn_ctx.db_reset_cb) {
                fn_ctx.db_reset_cb();
        }
}

/* Apple FMN error callback */
static void afmn_error_cb(AFMN_ERROR_LEVEL level, AFMN_ERROR_CATEGORY category, int code)
{
        volatile AFMN_ERROR_LEVEL err_level = level;
        volatile AFMN_ERROR_LEVEL err_category = category;
        volatile AFMN_ERROR_LEVEL err_code = code;

        (void) err_level;
        (void) err_category;
        (void) err_code;

        CSLA_TASK_PRINTF("ERROR: FindMy level = %d, category = %d, code = %d\r\n",
                level, category, code);
        OS_ASSERT(0);
}

/* Apple FMN pairing initiated callback */
static void afmn_pair_init_cb(void)
{
        CSLA_TASK_PRINTF("FindMy pairing initiated\r\n");

        fn_ctx.finder_network_state = FINDER_NETWORK_AFMN_PAIRING;

        fn_control_set_pairing_mode(false);

        /* De-initialize Google Fast Pair framework */
        fp_deinit();

        notify_control_task(FINDER_NETWORK_STATE_NOTIF);
}

/* Google Fast Pair framework execution callback */
static void fp_execution_cb(void)
{
        notify_control_task(GOOGLE_FAST_PAIR_NOTIF);
}

/* Google Fast Pair framework error callback */
static void fp_error_cb(int error_code)
{
        volatile int err_code = error_code;
        (void) err_code;

        CSLA_TASK_PRINTF("ERROR: Google Fast Pair error %d", error_code);
        OS_ASSERT(0);
}

#if (FP_LOCATOR_TAG != 1)
/* Pair status callback */
static void fp_pair_status_cb(uint16_t conn_idx, uint8_t status)
{
        CSLA_TASK_PRINTF("Pairing for conn index %u %s\r\n",
                conn_idx, (status == BLE_STATUS_OK) ? "succeeded" : "failed");
}
#endif /* !FP_LOCATOR_TAG */

/* Pair request status callback */
static void fp_pair_req_status_cb(uint16_t conn_idx, FP_PAIR_REQ_STAT stat, uint8_t err)
{
        if (err == 0) {
                if (stat == FP_PAIR_REQ_STAT_INITIATED) {
                        CSLA_TASK_PRINTF("Fast Pair request initiated for conn index %u\r\n", conn_idx);
                        fn_control_set_pairing_mode(false);

                        /* Indicate that Google Fast Pair pairing has been initiated */
                        fn_ctx.finder_network_state = FINDER_NETWORK_GFP_PAIRING;

                        /* De-initialize Apple FMN framework */
                        afmn_deinit(true);
                } else if (stat == FP_PAIR_REQ_STAT_COMPLETED) {
                        CSLA_TASK_PRINTF("Fast Pair request succeeded for conn index %u\r\n", conn_idx);

                        /* Indicate that Google Fast Pair pairing has been completed */
                        fn_ctx.finder_network_state = FINDER_NETWORK_GFP;
                }
        } else {
                OS_ASSERT(fn_ctx.finder_network_state != FINDER_NETWORK_AFMN);
                /* Reset finder network state */
                fn_ctx.finder_network_state = FINDER_NETWORK_NONE;

                CSLA_TASK_PRINTF("Fast Pair request failed for conn index %u with error code 0x%02X\r\n",
                        conn_idx, err);
        }

        notify_control_task(FINDER_NETWORK_STATE_NOTIF);
}

#if (FP_FMDN == 1)
/* FMDN provisioning status callback */
static void fp_fmdn_prov_status_cb(FP_FMDN_PROV_STAT stat)
{
        switch (stat) {
        case FP_FMDN_PROV_STAT_STOPPED:
                OS_TIMER_STOP(fn_ctx.beacon_time_storage_tim, OS_TIMER_FOREVER);
                CSLA_TASK_PRINTF("Device has stopped being FMDN provisioned\r\n");

                /* Reset finder network state */
                fn_ctx.finder_network_state = FINDER_NETWORK_NONE;
                break;
        case FP_FMDN_PROV_STAT_STARTED:
                /* The device can be provisioned only in one finder network */
                OS_ASSERT(fn_ctx.finder_network_state != FINDER_NETWORK_AFMN);

                /* Set Google FMDN as active finder network */
                fn_ctx.finder_network_state = FINDER_NETWORK_GFP_FMDN;

                /*
                 * Trigger beacon time update in NVM storage and start timer to
                 * periodically update it
                 */
                beacon_time_storage_tim_cb(NULL);
                OS_TIMER_START(fn_ctx.beacon_time_storage_tim, OS_TIMER_FOREVER);
                CSLA_TASK_PRINTF("Device has started being FMDN provisioned\r\n");
                break;
        }

        notify_control_task(FINDER_NETWORK_STATE_NOTIF);
}

/* Authenticated connection indication callback */
static void fp_auth_conn_cb(uint16_t conn_idx)
{
        if (fn_ctx.fp_auth_conn_cb) {
                fn_ctx.fp_auth_conn_cb(conn_idx);
        }
}

/* Beacon time callback */
static uint32_t fp_beacon_time_cb(void)
{
        uint64_t time = sys_timer_get_uptime_usec();
        time /= 1000;
        time /= 1000;
        return (uint32_t) time + fn_ctx.beacon_time_offset;
}
#endif /* FP_FMDN */

/* Apple FMN framework configuration */
static const afmn_config_t afmn_cfg = {
        .execution_cb = afmn_execution_cb,
        .state_cb = afmn_state_cb,
        .db_reset_cb = afmn_db_reset_cb,
        .error_cb = afmn_error_cb,
        .pair_init_cb = afmn_pair_init_cb
};

#if (FP_BATTERIES_COUNT != 0)
/* Google Fast Pair framework battery information */
__FP_RETAINED static fp_battery_info_t fp_batt_info[FP_BATTERIES_COUNT];
#endif

/* Google Fast Pair framework configuration */
const fp_cfg_t fp_cfg = {
        .execution_cb = fp_execution_cb,
#if (FP_FMDN == 1)
        .beacon_time_cb = fp_beacon_time_cb,
#endif
#if (FP_LOCATOR_TAG != 1)
        .pair_status_cb = fp_pair_status_cb,
#endif
        .pair_req_status_cb = fp_pair_req_status_cb,
#if (FP_FMDN == 1)
        .fmdn_prov_status_cb = fp_fmdn_prov_status_cb,
        .auth_conn_cb = fp_auth_conn_cb,
#endif
        .error_cb = fp_error_cb,
#if (FP_BATTERIES_COUNT != 0)
        .batt_info = fp_batt_info
#endif
};

void fn_control_init(const fn_control_config_t *cfg)
{
        int err = 0;

        /* Save task handle to use it in notifications */
        fn_ctx.task_hdl = OS_GET_CURRENT_TASK();

        fn_ctx.state_cb = cfg->state_cb;
        fn_ctx.pair_stop_cb = cfg->pair_stop_cb;
        fn_ctx.db_reset_cb = cfg->db_reset_cb;
        fn_ctx.batt_level_get_cb = cfg->batt_level_get_cb;
        fn_ctx.fp_auth_conn_cb = cfg->fp_auth_conn_cb;

#if (FP_FMDN == 1)
        /* Initialize beacon time with the value stored in NVM storage */
        init_beacon_time();
#endif

        /* Create timer for Google Fast Pair pairing mode timeout */
        fn_ctx.pairing_mode_tim =
                OS_TIMER_CREATE("pair", OS_MS_2_TICKS(PAIRING_MODE_TIMEOUT_MS),
                                OS_TIMER_ONCE, NULL, pairing_mode_tim_cb);
        /* Create timer for serial number lookup timeout */
        fn_ctx.serial_number_lookup_tim =
                OS_TIMER_CREATE("serial", OS_MS_2_TICKS(SERIAL_NUMBER_LOOKUP_TIMEOUT_MS),
                                OS_TIMER_ONCE, NULL, serial_number_lookup_tim_cb);
#if (FP_FMDN == 1)
        /* Create timer for periodic beacon time update in NVM storage */
        fn_ctx.beacon_time_storage_tim =
                OS_TIMER_CREATE("btime", OS_MS_2_TICKS(BEACON_TIME_STORAGE_INTERVAL_MS),
                                OS_TIMER_RELOAD, NULL, beacon_time_storage_tim_cb);
#endif /* FP_FMDN */

        /* Initialize Apple FMN framework */
        afmn_init(&afmn_cfg);

        /* Check if device is Apple FMN provisioned */
        if (afmn_get_accessory_state() != AFMN_ACCESSORY_STATE_UNPAIRED) {
                /* Set Apple FMN as active finder network */
                fn_ctx.finder_network_state = FINDER_NETWORK_AFMN;
        } else {
                /*
                 * Initialize Google Fast Pair framework
                 */
#if (FP_BATTERIES_COUNT != 0)
                uint8_t batt_level = 100;

                if (fn_ctx.batt_level_get_cb) {
                        batt_level = fn_ctx.batt_level_get_cb();
                }

                /* Set battery information */
                fp_batt_info[0].is_charging = false;
                fp_batt_info[0].level = batt_level;
#endif
                err = fp_init(&fp_cfg);
                OS_ASSERT(err == 0);

#if (FP_LOCATOR_TAG != 1)
                /* Set account key filter UI indication */
                fp_set_acc_key_filter_ui_indication(true);
#endif
#if (FP_BATTERY_NOTIFICATION == 1) && (FP_BATTERIES_COUNT != 0)
                /* Set battery UI indication */
                fp_set_battery_ui_indication(true);
#endif
#if (FP_FMDN == 1)
                /* Check if device is Google FMDN provisioned */
                if (fp_is_fmdn_provisioned()) {
                        /* Set Google FMDN as active finder network */
                        fn_ctx.finder_network_state = FINDER_NETWORK_GFP_FMDN;
                        /* De-initialize Apple FMN framework */
                        afmn_deinit(true);

                        /* Start timer for storing beacon time */
                        OS_TIMER_START(fn_ctx.beacon_time_storage_tim, OS_TIMER_FOREVER);
                }

                /* Start Google Fast Pair and FMDN advertising */
                fp_start_advertise();
#endif /* FP_FMDN */

                /* Initialize BLE services */
                afmn_db_reset_cb();
        }
}

void fn_control_set_scan_response(uint8_t structs_count, const gap_adv_ad_struct_t *structs)
{
        fn_ctx.scan_rsp_struct_count = structs_count;
        fn_ctx.scan_rsp_structs = structs;

        if (afmn_is_initialized()) {
                afmn_set_scan_response(structs_count, structs);
        }
        if (fp_is_initialized()) {
                fp_set_scan_response(structs_count, structs);
        }
}

FINDER_NETWORK_STATE fn_control_get_finder_network_state(void)
{
        return fn_ctx.finder_network_state;
}

static ble_error_t update_bd_address(void)
{
        ble_error_t err;

        own_address_t addr;
#if (FP_LOCATOR_TAG == 1)
        addr.addr_type = PRIVATE_RANDOM_NONRESOLVABLE_ADDRESS;
#else
        addr.addr_type = PRIVATE_RANDOM_RESOLVABLE_ADDRESS;
#endif
        err = ble_gap_address_set(&addr, 1024);

        if (err == BLE_STATUS_OK) {
                afmn_db_reset_cb();
        }

        return err;
}

static void handle_postponed_bd_address_update(void)
{
        if (fn_ctx.bt_addr_change_postponed && update_bd_address() == BLE_STATUS_OK) {
                fn_ctx.bt_addr_change_postponed = false;
                notify_control_task(FINDER_NETWORK_STATE_NOTIF);
        }
}

bool fn_control_handle_event(ble_evt_hdr_t *evt)
{
        bool evt_handled = ble_service_handle_event(evt);

        if (!evt_handled) {
        switch (evt->evt_code) {
        case BLE_EVT_GAP_ADV_COMPLETED:
        case BLE_EVT_GAP_DISCONNECTED:
                handle_postponed_bd_address_update();
                break;
        default:
                break;
        }
        }

        if (!evt_handled) {
                evt_handled = afmn_handle_event(evt);
        }
        if (!evt_handled) {
                evt_handled = fp_handle_event(evt);
        }

        return evt_handled;
}

int fn_control_stop_ringing(void)
{
        int ret = -1;

#if (FP_FMDN_RING_COMPONENTS_NUM > 0)
        if (fn_ctx.finder_network_state == FINDER_NETWORK_GFP_FMDN && fp_is_ringing()) {
                ret = fp_stop_ringing();
        }
#endif
        return ret;
}

int fn_control_factory_reset(void)
{
        int ret = 0;;

        if (afmn_is_initialized()) {
                afmn_factory_reset();
        }
        if (fp_is_initialized()) {
                ret = fp_factory_reset();
        }

        return ret;
}

FN_CONTROL_PAIRING_ERROR fn_control_set_pairing_mode(bool enable)
{
        FN_CONTROL_PAIRING_ERROR ret = FN_CONTROL_PAIRING_ERROR_NONE;

        /* Pairing mode can be enabled only when the device is not provisioned in a finder network */
        if (fn_ctx.finder_network_state == FINDER_NETWORK_AFMN ||
            fn_ctx.finder_network_state == FINDER_NETWORK_GFP ||
            fn_ctx.finder_network_state == FINDER_NETWORK_GFP_FMDN) {
                return FN_CONTROL_PAIRING_ERROR_ALREADY_PAIRED;
        }

        if (afmn_is_initialized()) {
                if (enable) {
                        if (!afmn_start_pair_mode()) {
                                ret = FN_CONTROL_PAIRING_ERROR_ALREADY_IN_PAIRING;
                        }
                } else {
                        /* Cannot stop pairing mode in case of Apple FMN */
                        if (afmn_is_pair_mode()) {
                                ret = FN_CONTROL_PAIRING_ERROR_ALREADY_IN_PAIRING;
                        }
                }
        }
        if (fp_is_initialized()) {
                if (enable) {
                        /* Start pairing mode for a specific period */
                        if (!fp_is_pairing_mode() && fp_set_pairing_mode(true)) {
                                OS_TIMER_START(fn_ctx.pairing_mode_tim, OS_TIMER_FOREVER);
                                ret = FN_CONTROL_PAIRING_ERROR_NONE;
                        }
                } else {
                        if (fp_is_pairing_mode()) {
                                fp_set_pairing_mode(false);

                                /* Stop pairing timer */
                                OS_TIMER_STOP(fn_ctx.pairing_mode_tim, OS_TIMER_FOREVER);

                                ret = FN_CONTROL_PAIRING_ERROR_NONE;
                        }
                }
        }

        return ret;
}

bool fn_control_is_pairing_mode(void)
{
        return (afmn_is_pair_mode() || fp_is_pairing_mode());
}

void fn_control_set_user_consent(bool enable)
{
        if (afmn_is_initialized()) {
                afmn_set_serial_number_lookup(enable);
                if (enable) {
                        OS_TIMER_START(fn_ctx.serial_number_lookup_tim, OS_TIMER_FOREVER);
                } else {
                        OS_TIMER_STOP(fn_ctx.serial_number_lookup_tim, OS_TIMER_FOREVER);
                }
        }

#if (FP_FMDN == 1)
        if (fp_is_initialized()) {
                fp_set_user_consent(enable);
        }
#endif
}

void fn_control_process_notif(uint32_t notif)
{
        int err = 0;

        if (fp_is_initialized()) {
#if (FP_FMDN == 1)
                /* Beacon time storage timeout timer expired. Update time in NVM storage. */
                if (notif & BEACON_TIME_TMO_NOTIF) {
                        /* Skip beacon time update in NVM storage if not FMDN provisioned */
                        if (fp_is_fmdn_provisioned()) {
                                write_beacon_time();
                        }
                }
#endif /* FP_FMDN */

                /* Google Fast Pair framework core module execution triggered */
                if (notif & GOOGLE_FAST_PAIR_NOTIF) {
                        fp_execution();
                }
        }

        if (afmn_is_initialized()) {
                /* Apple FMN framework execution triggered */
                if (notif & APPLE_FMN_NOTIF) {
                        afmn_execution();
                }

                /* Serial number lookup timeout timer expired */
                if (notif & SERIAL_NUMBER_TMO_NOTIF) {
                        afmn_set_serial_number_lookup(false);
                }
        }

        /* OS timer execution triggered */
        if (notif & AFMN_OS_TIMER_EXECUTION_NOTIF) {
                afmn_os_timer_execution();
        }

        /* Pairing mode timeout timer expired, stop pairing mode */
        if (notif & PAIRING_MODE_TMO_NOTIF) {
                fn_control_set_pairing_mode(false);

                /* Only Google Fast Pair pairing mode can be stopped */
                if (fn_ctx.pair_stop_cb) {
                        fn_ctx.pair_stop_cb(FINDER_NETWORK_GFP_PAIRING);
                }
        }

        /* Finder Network state has changed */
        if (notif & FINDER_NETWORK_STATE_NOTIF) {
                if (fn_ctx.finder_network_state == FINDER_NETWORK_NONE) {
                        if (!afmn_is_initialized()) {
                                afmn_init(&afmn_cfg);

                                /* Set scan response populated with <Complete Local Name> AD type */
                                afmn_set_scan_response(fn_ctx.scan_rsp_struct_count,
                                        fn_ctx.scan_rsp_structs);
                        }

                        if (!fp_is_initialized()) {
                                err = fp_init(&fp_cfg);
                                OS_ASSERT(err == 0);

                                /* Set scan response populated with <Complete Local Name> AD type */
                                err = fp_set_scan_response(fn_ctx.scan_rsp_struct_count,
                                        fn_ctx.scan_rsp_structs);
                                OS_ASSERT(err == 0);

#if (FP_LOCATOR_TAG != 1)
                                /* Set account key filter UI indication */
                                fp_set_acc_key_filter_ui_indication(true);
#endif
#if (FP_BATTERY_NOTIFICATION == 1) && (FP_BATTERIES_COUNT != 0)
                                /* Set battery UI indication */
                                fp_set_battery_ui_indication(true);
#endif

                                fp_start_advertise();

                                /* Disable advertising to trigger BD address change */
                                adv_control_set_mode(ADV_CONTROL_MODE_DISABLED);

                                if (update_bd_address() != BLE_STATUS_OK) {
                                        fn_ctx.bt_addr_change_postponed = true;
                                }
                        }
                }

                if (!fn_ctx.bt_addr_change_postponed && fn_ctx.state_cb) {
                        fn_ctx.state_cb(fn_ctx.finder_network_state);
                }
        }
}
