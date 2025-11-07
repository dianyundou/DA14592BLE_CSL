/**
 ****************************************************************************************
 *
 * @file fp_core.c
 *
 * @brief Google Fast Pair core module implementation
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

#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include FAST_PAIR_CONFIG_FILE
#include "fp_defaults.h"
#include "sdk_defs.h"
#include "osal.h"
#include "ble_common.h"
#include "ble_gap.h"
#include "gap.h"
#include "ble_mgr.h"
#include "ble_mgr_helper.h"
#include "ble_mgr_gtl.h"
#include "ble_mgr_cmd.h"
#include "ble_mgr_gap.h"
#include "ble_mgr_common.h"
#include "ble_service.h"
#include "gfmdn_support.h"
#include "gfps.h"
#include "dis.h"

#include "fp_notifications.h"
#include "fp_procedure.h"
#include "fp_account_keys.h"
#include "fp_fmdn.h"
#include "fp_ano.h"
#include "fp_motion_detection.h"
#include "fp_ble.h"
#include "fp_utils.h"

#include "fp_core.h"
#include "fast_pair.h"

/* Advertise setup
 * every two seconds, 7 Fast Pair advertisements are expected
 * and 1 FMDN advertisement
 * 1 : 7, i.e. 250 msec : 1750 msec
 */
#define DEFAULT_ADV_PERIOD              (2000)  /* msec */
#define MAXIMUM_ADV_PERIOD              (4000)  /* msec */
#define MINIMUM_ADV_PERIOD              (500)   /* msec */
#define ADV_TIME_FMDN                   (250)   /* msec */
#define RPA_ROTATION_TIME_SEC           (1024)  /* sec */
#define RPA_ROTATION_RAND_OFFSET_MIN    (1)     /* sec */
#define RPA_ROTATION_RAND_OFFSET_MAX    (204)   /* sec */
#define ADV_STRUCT_COUNT_MAX            (5)

#define AD_FLAGS_AD_STRUCT_LENGTH       (3)
#define BLOOM_FILTER_MAX_LENGTH         (FP_ACCOUNT_KEYS_COUNT + 3 + FP_ACCOUNT_KEYS_COUNT / 5)
#define SVC_DATA_AD_STRUCT_MAX_LENGTH   (FP_BATTERIES_COUNT + BLOOM_FILTER_MAX_LENGTH + 7)
#if (FP_ADVERTISE_CALIBRATED_TX_POWER_LEVEL == 1)
#define TX_POWER_LEVEL_AD_STRUCT_LENGTH (3)
#else
#define TX_POWER_LEVEL_AD_STRUCT_LENGTH (0)
#endif
#define AVAILABLE_ADV_DATA_SIZE         (BLE_ADV_DATA_LEN_MAX - AD_FLAGS_AD_STRUCT_LENGTH \
                                        - SVC_DATA_AD_STRUCT_MAX_LENGTH \
                                        - TX_POWER_LEVEL_AD_STRUCT_LENGTH)

/* Advertise mode */
typedef enum {
        ADV_DISCOVERABLE,
        ADV_NON_DISCOVERABLE,
        ADV_NON_DISCOVERABLE_FMDN_ONLY
} adv_mode_t;

#if (FP_FMDN == 1)
/* Type of advertise */
typedef enum {
        ADV_TYPE_FAST_PAIR,
        ADV_TYPE_FMDN,
} adv_type_t;
#endif /* FP_FMDN */

/*
 * Advertising and scan response data
 */
static const gap_adv_ad_struct_t adv_data_discoverable[] = {
        GAP_ADV_AD_STRUCT_BYTES(GAP_AD_TYPE_FLAGS,
                                GAP_BR_EDR_NOT_SUPPORTED),
        GAP_ADV_AD_STRUCT_BYTES(GAP_DATA_TYPE_UUID16_SVC_DATA,
                                0x2C, 0xFE,             /* 0xFE2C (Fast Pair Service UUID) */
                                (FP_MODEL_ID >> 16) & 0xFF,
                                (FP_MODEL_ID >> 8) & 0xFF,
                                FP_MODEL_ID & 0xFF),    /* 3 byte-model ID */
#if (FP_ADVERTISE_CALIBRATED_TX_POWER_LEVEL == 1)
        GAP_ADV_AD_STRUCT_BYTES(GAP_DATA_TYPE_TX_POWER_LEVEL, FP_CALIBRATED_TX_POWER_LEVEL)
#endif
};

/*
 * Advertising interval values
 */
static const struct {
        uint16_t min;
        uint16_t max;
} adv_intervals[] = {
        /* ADV_DISCOVERABLE interval values */
        {
                .min = BLE_ADV_INTERVAL_FROM_MS(20),    /* 20ms */
                .max = BLE_ADV_INTERVAL_FROM_MS(100),   /* 100ms */
        },
        /* ADV_NON_DISCOVERABLE interval values */
        {
                .min = BLE_ADV_INTERVAL_FROM_MS(150),   /* 150ms */
                .max = BLE_ADV_INTERVAL_FROM_MS(250),   /* 250ms */
        },
        /* ADV_NON_DISCOVERABLE_FMDN_ONLY interval values */
        {
                .min = BLE_ADV_INTERVAL_FROM_MS(1200),  /* 1200ms */
                .max = BLE_ADV_INTERVAL_FROM_MS(2000),  /* 2000ms */
        }
};

/* Collection of Google Fast Pair service callback functions */
static const fast_pair_callbacks_t fast_pair_callbacks = {
        .pairing_cb = fp_procedure_pairing_cb,
#if (FP_LOCATOR_TAG != 1)
        .passkey_cb = fp_procedure_passkey_cb,
#endif
        .additional_data_cb = fp_procedure_additional_data_cb,
        .accountkey_cb = fp_procedure_account_key_cb,
#if (FP_FMDN == 1)
        .beacon_actions_read_cb = fp_fmdn_beacon_actions_read_cb,
        .beacon_actions_write_cb = fp_fmdn_beacon_actions_write_cb,
#endif
};

/* Google Fast Pair Service information */
static const gfps_info_t gfps_info = {
        .model_id = FP_MODEL_ID,
};

#if (FP_DIS_ENABLE == 1)
/* Device Information Service information */
static const dis_device_info_t dis_info = {
        .manufacturer  = defaultBLE_DIS_MANUFACTURER,
        .model_number  = defaultBLE_DIS_MODEL_NUMBER,
        .serial_number = defaultBLE_DIS_SERIAL_NUMBER,
        .hw_revision   = defaultBLE_DIS_HW_REVISION,
        .fw_revision   = defaultBLE_DIS_FW_REVISION,
        .sw_revision   = defaultBLE_DIS_SW_REVISION,
};
#endif /* FP_DIS_ENABLE */

/* BD address renewal set command message structure */
typedef struct {
        ble_mgr_msg_hdr_t hdr;
        uint8_t status;
} fp_set_rand_addr_renewal_cmd_t;

/* BD address renewal set response message structure */
typedef struct {
        ble_mgr_msg_hdr_t hdr;
        ble_error_t status;
} fp_set_rand_addr_renewal_rsp_t;

/* OP codes for extra GAP commands */
enum fp_ble_cmd_gap_opcode {
        FP_SET_RAND_ADDR_RENEWAL_CMD = BLE_MGR_GAP_LAST_CMD,
};

/* Google Fast Pair core module context */
__FP_RETAINED static struct {
        uint32_t notif;
        bool adv_is_stopped;
        bool adv_restart_req;
        bool max_conns_reached;
        adv_mode_t adv_mode;
        uint16_t adv_period;
        uint16_t auth_conn_idx;
#if (FP_FMDN == 1)
        adv_type_t adv_type;
        FP_FMDN_PROVISIONING_STATE prov_state;
        fp_fmdn_prov_status_cb_t fmdn_prov_status_cb;
        fp_auth_conn_cb_t auth_conn_cb;
        OS_TIMER adv_tim;
#else
        bool rpa_rotation_pending;
        ACCURATE_OS_TIMER rpa_rotation_tim;
#endif /* FP_FMDN */
        fp_execution_cb_t execution_cb;
#if (FP_LOCATOR_TAG != 1)
        fp_pair_status_cb_t pair_status_cb;
#endif
        fp_error_cb_t error_cb;
        gap_adv_ad_struct_t adv_struct[ADV_STRUCT_COUNT_MAX];
        uint8_t adv_struct_count;
        const gap_adv_ad_struct_t *scan_rsp_struct;
        uint8_t scan_rsp_struct_count;
#if (FP_BATTERIES_COUNT != 0)
        fp_battery_info_t info[FP_BATTERIES_COUNT];
#endif
#if (FP_BATTERY_NOTIFICATION == 1) && (FP_BATTERIES_COUNT != 0)
        bool show_battery_ui_indication;
#endif
#if (FP_LOCATOR_TAG != 1)
        bool show_acc_key_filter_ui_indication;
#endif
} core_ctx;

/* Indication that Google Fast Pair framework is initialized */
__RETAINED static bool fp_initialized;

/* Instances for Google Fast Pair framework BLE services */
__RETAINED static ble_service_t *fp_gfps;
#if (FP_DIS_ENABLE == 1)
__RETAINED static ble_service_t *fp_dis;
#endif

static ble_error_t stop_advertise(void);

#if (FP_FMDN != 1)
/* Generates a randomized timer interval in seconds */
static uint32_t randomize_interval(uint32_t intv_sec)
{
        return (intv_sec + ((uint32_t) rand()) % RPA_ROTATION_RAND_OFFSET_MAX +
                RPA_ROTATION_RAND_OFFSET_MIN);
}

/* Controls RPA rotation */
static void control_rpa_rotation(bool enable)
{
        if (enable) {
                /* Generate a randomized time interval for next rotation */
                uint32_t rotation_intv_ms = randomize_interval(RPA_ROTATION_TIME_SEC) * 1000;

                /* Start rotation timer (for next rotation to take place) */
                ACCURATE_OS_TIMER_CHANGE_PERIOD(core_ctx.rpa_rotation_tim, rotation_intv_ms);
        } else {
                ACCURATE_OS_TIMER_STOP(core_ctx.rpa_rotation_tim);
        }
}

/* Performs RPA rotation */
static int rotate_rpa(void)
{
        int err = 0;

        /* No need to change RPA while advertising is stopped */
        core_ctx.rpa_rotation_pending = fp_is_advertise_stopped();

        /* RPA should not change if pairing mode is enabled */
        core_ctx.rpa_rotation_pending = core_ctx.rpa_rotation_pending ||
                                        fp_procedure_is_pairing_mode();

        if (core_ctx.rpa_rotation_pending) {
                return 0;
        }

        /* Trigger random address renewal */
        err = fp_set_rand_addr_renewal(true);
        /* Salt rotation */
        fp_acc_keys_generate_new_salt();

        /* Start rotation timer (for next rotation to take place) */
        control_rpa_rotation(true);

        return err;
}

/* Check if RPA rotation is pending */
static bool is_rpa_rotation_pending(void)
{
        return core_ctx.rpa_rotation_pending;
}
#endif /* FP_FMDN */

/* Triggers pending address rotation */
static void trigger_pending_addr_rotation(void) {
#if (FP_FMDN == 1)
        if (fp_fmdn_is_id_rotation_pending()) {
                fp_send_notification(ID_ROTATION_TMO_NOTIF);
        }
        if (fp_fmdn_is_utpm_address_rotation_pending()) {
                fp_send_notification(UTPM_TMO_NOTIF);
        }
#else
        if (is_rpa_rotation_pending()) {
                fp_send_notification(RPA_ROTATION_TMO_NOTIF);
        }
#endif /* FP_FMDN */
}

/* Update max-connections-reached state */
static void update_max_connections(void)
{
        gap_device_t devices[BLE_GAP_MAX_CONNECTED];
        size_t length = ARRAY_LENGTH(devices);

        ble_gap_get_devices(GAP_DEVICE_FILTER_CONNECTED, NULL, &length, devices);
        core_ctx.max_conns_reached = (length == ARRAY_LENGTH(devices) ? true : false);
}

/* Handler for GAP disconnect event */
static void handle_evt_gap_disconnected(ble_evt_gap_disconnected_t *evt)
{
        bool max_conns_reached_dropped = core_ctx.max_conns_reached;

        if (core_ctx.auth_conn_idx == evt->conn_idx) {
                core_ctx.auth_conn_idx = BLE_CONN_IDX_INVALID;

#if (FP_FMDN == 1)
#if (FP_DIS_ENABLE == 1)
                /* Disable read access authorization to DIS characteristics for the connection */
                dis_set_authorized_read(fp_dis, evt->conn_idx, false);
#endif /* FP_DIS_ENABLE */
#endif /* FP_FMDN */
        }

        update_max_connections();
        fp_procedure_stop(evt->conn_idx);
#if (FP_FMDN == 1)
        fp_fmdn_wipe_out_account_keys();
        fp_fmdn_remove_ring_connection(evt->conn_idx);
#endif /* FP_FMDN */
        if (core_ctx.adv_is_stopped) {
                /* Advertise is intentionally stopped, i.e. fp_stop_advertise has been called.
                 * Do not start advertise */
                return;
        }
        if ((core_ctx.adv_mode == ADV_DISCOVERABLE) ||
            (max_conns_reached_dropped && (core_ctx.adv_mode != ADV_NON_DISCOVERABLE_FMDN_ONLY))) {
                FP_LOG_PRINTF("Connection dropped and advertise is started\r\n");
                fp_start_advertise();
        }
}

/* Handler for GAP connect event */
static void handle_evt_gap_connected(ble_evt_gap_connected_t *evt)
{
        update_max_connections();
}

/* Handler for GAP advertise completed event */
static void handle_evt_gap_adv_completed(ble_evt_gap_adv_completed_t *evt)
{
        /*
         * If advertising is completed, just restart it. It's either because a new client connected
         * or it was cancelled in order to change the interval values.
         */

#if (FP_FMDN == 1)
        /* Perform ID rotation if it is in progress */
        fp_fmdn_perform_id_rotation_in_progress();
#endif /* FP_FMDN */

        if (core_ctx.adv_is_stopped) {
                /* Advertise is intentionally stopped, i.e. fp_stop_advertise has been called. */
                return;
        }

        if ((core_ctx.adv_mode != ADV_DISCOVERABLE || core_ctx.adv_restart_req)
                                                                && !fp_ble_adv_is_started()) {
                fp_start_advertise();
        }

        core_ctx.adv_restart_req = false;
}

/* Handler for GAP pairing request */
static bool handle_evt_gap_pair_req(ble_evt_gap_pair_req_t *evt)
{
        ble_error_t ret;
        bool event_handled = false;

#if (FP_LOCATOR_TAG == 1)
        /* It is a locator tag, so reject normal pairing request */
        ret = ble_gap_pair_reply(evt->conn_idx, false, evt->bond);
        FP_CHECK_ERROR(ret);
        event_handled = true;
#else
        if (fp_procedure_is_pairing_connection(evt->conn_idx)) {
                ret = ble_gap_set_io_cap(GAP_IO_CAP_DISP_YES_NO);
                ret += ble_gap_pair_reply(evt->conn_idx, true, evt->bond);
                FP_CHECK_ERROR(ret);
                event_handled = true;
        }
#endif /* FP_LOCATOR_TAG */

        return event_handled;
}

#if (FP_LOCATOR_TAG != 1)
/* Handler for GAP numeric request */
static bool handle_evt_gap_numeric_req(ble_evt_gap_numeric_request_t *evt)
{
        if (fp_procedure_is_pairing_connection(evt->conn_idx)) {
                fp_procedure_save_passkey(evt->num_key);
                return true;
        }

        return false;
}

/* Handler for GAP pairing completed request */
static bool handle_evt_gap_pair_completed_req(ble_evt_gap_pair_completed_t *evt)
{
        if (fp_procedure_is_pairing_connection(evt->conn_idx)) {
                if (evt->status) {
                        fp_procedure_stop(evt->conn_idx);
                } else {
                        ble_gap_set_io_cap(GAP_IO_CAP_NO_INPUT_OUTPUT);
                }
                if (core_ctx.pair_status_cb) {
                        core_ctx.pair_status_cb(evt->conn_idx, evt->status);
                }
                return true;
        }

        return false;
}
#endif /* !FP_LOCATOR_TAG */

/* Sets new advertising interval */
static int set_advertising_interval(adv_mode_t mode)
{
        uint16_t min, max;
        gap_disc_mode_t discov_mode;

        discov_mode = mode == ADV_DISCOVERABLE ? GAP_DISC_MODE_GEN_DISCOVERABLE :
                                                 GAP_DISC_MODE_NON_DISCOVERABLE;

        if ((mode == ADV_NON_DISCOVERABLE_FMDN_ONLY) && (core_ctx.adv_period != DEFAULT_ADV_PERIOD)) {
                min = max = BLE_ADV_INTERVAL_FROM_MS(core_ctx.adv_period);
        } else {
                min = adv_intervals[mode].min;
                max = adv_intervals[mode].max;
        }

        /* Save current advertising setting */
        core_ctx.adv_mode = mode;

        /* Set advertising parameters */
        const fp_ble_adv_params_t adv_params = {
                .mode = discov_mode,
                .type = GAP_CONN_MODE_UNDIRECTED,
                .intv_min = min,
                .intv_max = max,
#if (FP_FMDN == 1)
                .tx_power = (core_ctx.adv_type == ADV_TYPE_FAST_PAIR) ? FP_ADV_TX_POWER :
                                                                        FP_FMDN_ADV_TX_POWER
#else
                .tx_power = FP_ADV_TX_POWER
#endif /* FP_FMDN */
        };

        return fp_ble_adv_set_params(&adv_params);
}

#if (FP_FMDN == 1)
static uint32_t get_fmdn_adv_time()
{
        return (core_ctx.adv_period <= DEFAULT_ADV_PERIOD) ? ADV_TIME_FMDN : (2 * ADV_TIME_FMDN);
}
#endif

/* Updates advertising data during non-discoverable mode */
static int update_non_discoverable_advertise_data(void)
{
        int err = 0;
#if (FP_FMDN == 1)
        FP_FMDN_PROVISIONING_STATE prov_state = fp_get_fmdn_provisioning_state();
        adv_type_t adv_type;
        uint32_t period_ms = 0;

        switch (prov_state) {
        case FP_FMDN_PROVISIONING_STARTED:
#if (FP_LOCATOR_TAG == 1)
                adv_type = ADV_TYPE_FMDN;
                break;
#endif /* FP_LOCATOR_TAG */
        case FP_FMDN_PROVISIONING_INITIATING:
                if (core_ctx.adv_type == ADV_TYPE_FAST_PAIR) {
                        adv_type = ADV_TYPE_FMDN;
                        period_ms = get_fmdn_adv_time();
                } else {
                        adv_type = ADV_TYPE_FAST_PAIR;
                        period_ms = core_ctx.adv_period - get_fmdn_adv_time();
                }
                break;
        default:
                adv_type = ADV_TYPE_FAST_PAIR;
                break;
        }

        core_ctx.adv_type = adv_type;

        err += fp_ble_adv_set_tx_power((adv_type == ADV_TYPE_FAST_PAIR) ? FP_ADV_TX_POWER :
                                                                          FP_FMDN_ADV_TX_POWER);
        err += fp_update_advertise_data(false);
        if (period_ms > 0) {
                OS_TIMER_CHANGE_PERIOD(core_ctx.adv_tim, OS_MS_2_TICKS(period_ms), OS_TIMER_FOREVER);
        }
#else
        err += fp_update_advertise_data(false);
#endif /* FP_FMDN */

        return err;
}

/* Starts advertising in discoverable mode */
static int start_discoverable_advertise(void)
{
        int err = 0;
#if (FP_FMDN == 1)
        core_ctx.adv_type = ADV_TYPE_FAST_PAIR;
#endif
        /* Set advertising parameters */
        err += set_advertising_interval(ADV_DISCOVERABLE);
        /* Set advertising and scan response data */
        err += fp_ble_adv_set_ad_struct(ARRAY_LENGTH(adv_data_discoverable), adv_data_discoverable,
                core_ctx.scan_rsp_struct_count, core_ctx.scan_rsp_struct);
        /* Start advertising */
        err += fp_ble_adv_start();

        return err;
}

/* Starts advertising in non-discoverable mode */
static int start_non_discoverable_advertise(void)
{
        int err = 0;
        adv_mode_t adv_mode = ADV_NON_DISCOVERABLE;

#if (FP_FMDN == 1)
#if (FP_LOCATOR_TAG == 1)
        if (fp_get_fmdn_provisioning_state() == FP_FMDN_PROVISIONING_STARTED) {
                adv_mode = ADV_NON_DISCOVERABLE_FMDN_ONLY;
        }
#endif /* FP_LOCATOR_TAG */
#endif /* FP_FMDN */

        /* Set advertising parameters */
        err += set_advertising_interval(adv_mode);
        /* Set advertising and scan response data */
        err += update_non_discoverable_advertise_data();
        /* Start advertising */
        err += fp_ble_adv_start();

        return err;
}

/* Handler for BD address change */
static void handle_evt_gap_air_op_bdaddr(ble_evt_gap_air_op_bdaddr_t *evt)
{
        /*
         * Disable (periodic) random address renewal, as Google FMD spec requires intervals with
         * randomized offset
         */
        fp_set_rand_addr_renewal(false);

        FP_LOG_PRINTF("Bluetooth device address rotation done: %02X:%02X:%02X:%02X:%02X:%02X\r\n",
                evt->address.addr[5], evt->address.addr[4], evt->address.addr[3],
                evt->address.addr[2], evt->address.addr[1], evt->address.addr[0]);
}

bool fp_handle_event(const ble_evt_hdr_t *evt)
{
        bool event_handled = false;

        if (!(FP_CONFIG_OPTION & FP_CONFIG_OPTION_BLE_DB_CONTROLLED_BY_APP)) {
                event_handled = ble_service_handle_event(evt);
        }

        if (fp_initialized && !event_handled) {
                switch (evt->evt_code) {
                case BLE_EVT_GAP_CONNECTED:
                        handle_evt_gap_connected((ble_evt_gap_connected_t *) evt);
                        break;
                case BLE_EVT_GAP_DISCONNECTED:
                        handle_evt_gap_disconnected((ble_evt_gap_disconnected_t *) evt);
                        break;
                case BLE_EVT_GAP_ADV_COMPLETED:
                        handle_evt_gap_adv_completed((ble_evt_gap_adv_completed_t *) evt);
                        break;
                case BLE_EVT_GAP_PAIR_REQ:
                        event_handled = handle_evt_gap_pair_req((ble_evt_gap_pair_req_t *) evt);
                        break;
#if (FP_LOCATOR_TAG != 1)
                case BLE_EVT_GAP_PAIR_COMPLETED:
                        event_handled = handle_evt_gap_pair_completed_req((ble_evt_gap_pair_completed_t *) evt);
                        break;
                case BLE_EVT_GAP_NUMERIC_REQUEST:
                        event_handled = handle_evt_gap_numeric_req((ble_evt_gap_numeric_request_t *) evt);
                        break;
#endif /* !FP_LOCATOR_TAG */
                case BLE_EVT_GAP_AIR_OP_BDADDR:
                        handle_evt_gap_air_op_bdaddr((ble_evt_gap_air_op_bdaddr_t *) evt);
                        break;
                }
        }

        return event_handled;
}

#if (FP_FMDN == 1)
/* Advertise OS timer callback function */
static void adv_tim_cb(OS_TIMER timer)
{
        fp_send_notification(ADV_TMO_NOTIF);
}
#else
/* RPA Rotation timer callback function */
static void rpa_rotation_tim_cb(ACCURATE_OS_TIMER timer)
{
        fp_send_notification(RPA_ROTATION_TMO_NOTIF);
}
#endif /* FP_FMDN */

#if (FP_LOCATOR_TAG != 1)
bool fp_get_acc_key_filter_ui_indication(void)
{
        return core_ctx.show_acc_key_filter_ui_indication;
}
#endif /* !FP_LOCATOR_TAG */

#if (FP_BATTERIES_COUNT != 0)
fp_battery_info_t *fp_get_battery_information(void)
{
        return core_ctx.info;
}

#if (FP_BATTERY_NOTIFICATION == 1)
bool fp_get_battery_ui_indication(void)
{
        return core_ctx.show_battery_ui_indication;
}
#endif /* FP_BATTERY_NOTIFICATION */
#endif /* FP_BATTERIES_COUNT */

void fp_send_notification(uint32_t notif)
{
        if (!fp_initialized) {
                return;
        }

        OS_ENTER_CRITICAL_SECTION();
        core_ctx.notif |= notif;
        OS_LEAVE_CRITICAL_SECTION();
        core_ctx.execution_cb();
}

int fp_init(const fp_cfg_t *cfg)
{
        int err = 0;

        if (fp_initialized) {
                return -1;
        }

        fp_initialized = true;

        memset(&core_ctx, 0, sizeof(core_ctx));

        OS_ASSERT(cfg->execution_cb);
        OS_ASSERT(cfg->error_cb);
#if (FP_FMDN == 1)
        OS_ASSERT(cfg->beacon_time_cb);
#endif /* FP_FMDN */
#if (FP_BATTERIES_COUNT != 0)
        OS_ASSERT(cfg->batt_info);
#endif
        core_ctx.execution_cb = cfg->execution_cb;
        core_ctx.error_cb = cfg->error_cb;
#if (FP_LOCATOR_TAG != 1)
        core_ctx.pair_status_cb = cfg->pair_status_cb;
#endif
#if (FP_FMDN == 1)
        core_ctx.fmdn_prov_status_cb = cfg->fmdn_prov_status_cb;
        core_ctx.auth_conn_cb = cfg->auth_conn_cb;
#endif

#if (FP_BATTERIES_COUNT != 0)
        memcpy(core_ctx.info, cfg->batt_info, sizeof(fp_battery_info_t) * FP_BATTERIES_COUNT);
#endif
        core_ctx.adv_mode = ADV_NON_DISCOVERABLE;
        core_ctx.adv_period = DEFAULT_ADV_PERIOD;
#if (FP_FMDN == 1)
        core_ctx.adv_type = ADV_TYPE_FAST_PAIR;
#endif
        core_ctx.adv_struct_count = 2;  /* AD Flags + Fast Pair service data */
#if (FP_ADVERTISE_CALIBRATED_TX_POWER_LEVEL == 1)
        core_ctx.adv_struct_count++;    /* Calibrated TX power level */
#endif

        if (!(FP_CONFIG_OPTION & FP_CONFIG_OPTION_BLE_DB_CONTROLLED_BY_APP)) {
                own_address_t addr;
#if (FP_LOCATOR_TAG == 1)
                addr.addr_type = PRIVATE_RANDOM_NONRESOLVABLE_ADDRESS;
#else
                addr.addr_type = PRIVATE_RANDOM_RESOLVABLE_ADDRESS;
#endif
                err += ble_gap_address_set(&addr, RPA_ROTATION_TIME_SEC);
        }

#if (FP_FMDN == 1)
#if (FP_FMDN_RING_COMPONENTS_NUM > 0)
        /* Initialize ringing components control module */
        fp_ring_comp_init();
#endif
        /* Initialize motion detection module */
        err += fp_motion_detection_init();
#endif /* FP_FMDN */

        /* BLE advertising initialization for Google Fast Pair and FMDN */
        fp_ble_adv_init();

        /*
         * Initialize modules and resources of Google Fast Pair framework
         */
#if (FP_FMDN == 1)
        const fp_fmdn_cfg_t fmdn_cfg = {
                .beacon_time_cb = cfg->beacon_time_cb,
        };

        /* Create timer for pairing mode timeout */
        core_ctx.adv_tim = OS_TIMER_CREATE("adv", OS_MS_2_TICKS(ADV_TIME_FMDN), OS_TIMER_ONCE,
                        NULL, adv_tim_cb);
        OS_ASSERT(core_ctx.adv_tim);
#endif /* FP_FMDN */

        err += fp_acc_keys_init();
        fp_procedure_init(cfg->pair_req_status_cb);
#if (FP_FMDN == 1)
        err += fp_fmdn_init(&fmdn_cfg);
#else
        /* Create timer for periodic RPA rotation */
        core_ctx.rpa_rotation_tim = ACCURATE_OS_TIMER_CREATE(RPA_ROTATION_TIME_SEC * 1000,
                                        false, rpa_rotation_tim_cb);
        OS_ASSERT(core_ctx.rpa_rotation_tim);

        /* Enable periodic RPA rotation */
        control_rpa_rotation(true);
#endif /* FP_FMDN */

        if (!(FP_CONFIG_OPTION & FP_CONFIG_OPTION_BLE_DB_CONTROLLED_BY_APP)) {
                /* Build database */
                fp_add_services_to_db();
        }

        /* Update status of connections */
        update_max_connections();

        core_ctx.auth_conn_idx = BLE_CONN_IDX_INVALID;

        /* Set advertising data and scan response, then start advertising. */
        if (cfg->start_adv) {
                err += fp_start_advertise();
        } else {
                err += fp_stop_advertise();
        }

        err += fp_execution();

        return err;
}

void fp_deinit(void)
{
        if (!fp_initialized) {
                return;
        }

        fp_stop_advertise();

        /* Enable (periodic) random address renewal */
        fp_set_rand_addr_renewal(true);

#if (FP_FMDN == 1)
        fp_fmdn_deinit();
#else
        ACCURATE_OS_TIMER_DELETE(core_ctx.rpa_rotation_tim);
#endif
        fp_procedure_deinit();
#if (FP_FMDN == 1)
        OS_TIMER_DELETE(core_ctx.adv_tim, OS_TIMER_FOREVER);
#endif

        fp_ble_adv_deinit();
#if (FP_FMDN == 1)
        fp_motion_detection_deinit();
#if (FP_FMDN_RING_COMPONENTS_NUM > 0)
        fp_ring_comp_deinit();
#endif
#endif /* FP_FMDN */

        if (!(FP_CONFIG_OPTION & FP_CONFIG_OPTION_BLE_DB_CONTROLLED_BY_APP)) {
                fp_remove_services_from_db();
        }

        fp_initialized = false;
}

bool fp_is_initialized(void)
{
        return fp_initialized;
}

int fp_execution(void)
{
        uint32_t notif __UNUSED;
        int err = 0;

        if (!fp_initialized) {
                return -1;
        }

        OS_ENTER_CRITICAL_SECTION();
        notif = core_ctx.notif;
        core_ctx.notif = 0;
        OS_LEAVE_CRITICAL_SECTION();

        if (notif & KEY_TMO_NOTIF) {
                /* Procedure for the connection (if any) of current session is completed */
                fp_procedure_stop(BLE_CONN_IDX_INVALID);
        }

#if (FP_UTILS_CORR_OS_TIMERS == 1)
        if (notif & CORR_OS_TIMER_TRIGGER_NOTIF) {
                corr_os_timer_trigger_correction();
        }
#endif /* FP_UTILS_CORR_OS_TIMERS */

#if (FP_FMDN == 1)
        if (notif & ADV_TMO_NOTIF) {
                if (!core_ctx.adv_is_stopped) {
                        err += update_non_discoverable_advertise_data();
                }
        }

        if (notif & ID_ROTATION_TMO_NOTIF) {
                err += fp_fmdn_rotate_id();
        }

        if (notif & UTPM_TMO_NOTIF) {
                err += fp_fmdn_rotate_utpm_address();
        }

        if (notif & FMDN_PROVISIONING_TMO_NOTIF) {
                FP_FMDN_PROVISIONING_STATE prov_state = fp_get_fmdn_provisioning_state();

                if (prov_state < FP_FMDN_PROVISIONING_INITIATING) {
                        /* Device is not FMDN provisioned, so clear all keys */
                        err += fp_factory_reset();
                } else if (prov_state == FP_FMDN_PROVISIONING_INITIATING) {
                        fp_set_fmdn_provisioning_state(FP_FMDN_PROVISIONING_STARTED);
                        /* Stop Fast Pair advertise frames */
                        err += fp_restart_advertise();
                }

                /* Trigger any pending ID rotation */
                trigger_pending_addr_rotation();
        }

        if (notif & USER_CONSENT_TMO_NOTIF) {
                fp_fmdn_set_user_consent(false);
        }
#if (FP_FMDN_RING_COMPONENTS_NUM > 0)
        if (notif & RING_TMO_NOTIF) {
                if (fp_fmdn_is_ringing()) {
                        fp_fmdn_stop_ringing(FP_FMDN_RING_TIMEOUT_STOPPED);
                }
        }
#endif
        if (notif & MOTION_DETECTOR_ENABLE_NOTIF) {
                fp_motion_detection_enable_detector();
        }

        if (notif & MOTION_DETECTED_NOTIF) {
#if (FP_FMDN_RING_COMPONENTS_NUM > 0)
                fp_fmdn_start_ringing(FP_FMDN_MOTION_DETECT_PLAY_SOUND_TIMEOUT_MS,
                        FP_RING_COMP_VOLUME_DEFAULT, fp_motion_detection_restart);
#endif
        }

        if (notif & MOTION_DETECTOR_BACKOFF_NOTIF) {
                fp_motion_detection_disable_detector();
        }
#else
        if (notif & RPA_ROTATION_TMO_NOTIF) {
                err += rotate_rpa();
        }
#endif /* FP_FMDN */

        return err;
}

int fp_factory_reset(void)
{
        int err = 0;

        OS_ASSERT(fp_initialized);

        err += fp_acc_keys_clean();
#if (FP_FMDN == 1)
        err += fp_acc_keys_clear_ephemeral_identity_key();
#endif

        return err;
}

bool fp_set_pairing_mode(bool enable)
{
        bool is_pairing_mode;
        int ret;

        OS_ASSERT(fp_initialized);

#if (FP_FMDN == 1)
#if (FP_LOCATOR_TAG == 1)
        /* If device is FMDN provisioned, prevent pairing mode from being enabled */
        if (enable && fp_get_fmdn_provisioning_state() >= FP_FMDN_PROVISIONING_INITIATING) {
                return false;
        }
#endif /* FP_LOCATOR_TAG */
#endif /* FP_FMDN */

        is_pairing_mode = fp_procedure_is_pairing_mode();

        if (is_pairing_mode != enable) {
                fp_procedure_set_pairing_mode(enable);

                if (!enable) {
                        trigger_pending_addr_rotation();
                }
                ret = fp_restart_advertise();

                if (ret != 0) {
                        return false;
                }
        }

        return true;
}

bool fp_is_pairing_mode(void)
{
        OS_ASSERT(fp_initialized);

        return fp_procedure_is_pairing_mode();
}

int fp_start_advertise(void)
{
        int err = 0;

        OS_ASSERT(fp_initialized);

        if (core_ctx.max_conns_reached && (core_ctx.adv_mode != ADV_NON_DISCOVERABLE_FMDN_ONLY)) {
                /* Maximum number of connections reached so do not restart advertise. */
                FP_LOG_PRINTF("Max connections reached. Advertise will not restart.\r\n");
                return -1;
        }

        if (core_ctx.adv_is_stopped) {
                trigger_pending_addr_rotation();
                core_ctx.adv_is_stopped = false;
        }

#if (FP_FMDN == 1)
        OS_TIMER_STOP(core_ctx.adv_tim, OS_TIMER_FOREVER);
#endif /* FP_FMDN */

        if (fp_procedure_is_pairing_mode()) {
                err += start_discoverable_advertise();
        } else {
                err += start_non_discoverable_advertise();
        }

        return err;
}

/* Stops advertising */
static ble_error_t stop_advertise(void)
{
#if (FP_FMDN == 1)
        OS_TIMER_STOP(core_ctx.adv_tim, OS_TIMER_FOREVER);
#endif
        return fp_ble_adv_stop();
}

int fp_stop_advertise(void)
{
        int err = 0;

        OS_ASSERT(fp_initialized);

        core_ctx.adv_restart_req = false;

        if (!core_ctx.adv_is_stopped) {
                ble_error_t ret = stop_advertise();
                err += (ret != BLE_STATUS_OK && ret != BLE_ERROR_NOT_ALLOWED);
                core_ctx.adv_is_stopped = true;
        }

        return err;
}

bool fp_is_advertise_stopped(void)
{
        OS_ASSERT(fp_initialized);

        return core_ctx.adv_is_stopped;
}

#if (FP_FMDN == 1)
int fp_set_advertise_interval(uint16_t interval)
{
        OS_ASSERT(fp_initialized);

        OS_ASSERT(interval <= MAXIMUM_ADV_PERIOD);
        OS_ASSERT((interval % 250) == 0);
        OS_ASSERT(interval >= MINIMUM_ADV_PERIOD);

        core_ctx.adv_period = interval;

        return fp_restart_advertise();
}
#endif /* FP_FMDN */

int fp_restart_advertise(void)
{
        int err = 0;

        if (core_ctx.adv_is_stopped) {
                /* Advertise is intentionally stopped, i.e. fp_stop_advertise has been called. */
                return 0;
        }

        /* Advertising isn't started, start it now */
        if (stop_advertise() == BLE_ERROR_NOT_ALLOWED) {
                core_ctx.adv_restart_req = false;
                err += fp_start_advertise();
        } else {
                core_ctx.adv_restart_req = true;
        }

        return err;
}

int fp_update_advertise_data(bool discoverable)
{
        int err = 0;

        if (core_ctx.adv_is_stopped) {
                /* Advertise is intentionally stopped, i.e. fp_stop_advertise has been called. */
                return 0;
        }

        if (fp_procedure_is_pairing_mode() && discoverable) {
                err += fp_ble_adv_set_ad_struct(ARRAY_LENGTH(adv_data_discoverable),
                        adv_data_discoverable, core_ctx.scan_rsp_struct_count,
                        core_ctx.scan_rsp_struct);
        } else if (!fp_procedure_is_pairing_mode() && !discoverable) {
                core_ctx.adv_struct[0].len = 1;
                core_ctx.adv_struct[0].type = GAP_AD_TYPE_FLAGS;
                core_ctx.adv_struct[0].data = (uint8_t []){ GAP_BR_EDR_NOT_SUPPORTED };
#if (FP_FMDN == 1)
                if (core_ctx.adv_type == ADV_TYPE_FAST_PAIR) {
                        const gap_adv_ad_struct_t *adv_struct_fast_pair =
                                fp_acc_keys_get_advertise_struct();

                        memcpy(&core_ctx.adv_struct[1], adv_struct_fast_pair,
                                sizeof(gap_adv_ad_struct_t));
#if (FP_ADVERTISE_CALIBRATED_TX_POWER_LEVEL == 1)
                        core_ctx.adv_struct[2].len = 1;
                        core_ctx.adv_struct[2].type = GAP_DATA_TYPE_TX_POWER_LEVEL;
                        core_ctx.adv_struct[2].data = (uint8_t []){ FP_CALIBRATED_TX_POWER_LEVEL };
#endif
                        err += fp_ble_adv_set_ad_struct(core_ctx.adv_struct_count,
                                core_ctx.adv_struct, core_ctx.scan_rsp_struct_count,
                                core_ctx.scan_rsp_struct);
                } else {
                        const gap_adv_ad_struct_t *adv_struct_fmdn = fp_fmdn_advertise_struct();

                        memcpy(&core_ctx.adv_struct[1], adv_struct_fmdn,
                                sizeof(gap_adv_ad_struct_t));

                        err += fp_ble_adv_set_ad_struct(2, core_ctx.adv_struct,
                                core_ctx.scan_rsp_struct_count, core_ctx.scan_rsp_struct);
                }
#else
                const gap_adv_ad_struct_t *adv_struct_fast_pair = fp_acc_keys_get_advertise_struct();

                memcpy(&core_ctx.adv_struct[1], adv_struct_fast_pair, sizeof(gap_adv_ad_struct_t));
#if (FP_ADVERTISE_CALIBRATED_TX_POWER_LEVEL == 1)
                core_ctx.adv_struct[2].len = 1;
                core_ctx.adv_struct[2].type = GAP_DATA_TYPE_TX_POWER_LEVEL;
                core_ctx.adv_struct[2].data = (uint8_t []){ FP_CALIBRATED_TX_POWER_LEVEL };
#endif /* FP_ADVERTISE_CALIBRATED_TX_POWER_LEVEL */
                err += fp_ble_adv_set_ad_struct(core_ctx.adv_struct_count, core_ctx.adv_struct,
                        core_ctx.scan_rsp_struct_count, core_ctx.scan_rsp_struct);
#endif /* FP_FMDN */
        }

        return err;
}

#if (FP_FMDN == 1)
void fp_set_fmdn_provisioning_state(FP_FMDN_PROVISIONING_STATE state)
{
        FP_FMDN_PROVISIONING_STATE prov_state;

        switch (state) {
        case FP_FMDN_PROVISIONING_STOPPED:
        case FP_FMDN_PROVISIONING_INITIATING:
        case FP_FMDN_PROVISIONING_STARTED:
                prov_state = core_ctx.prov_state;
                core_ctx.prov_state = state;
                core_ctx.adv_type = ADV_TYPE_FAST_PAIR;
#if (FP_LOCATOR_TAG == 1)
                {
                        bool start_prov_timer = (state == FP_FMDN_PROVISIONING_INITIATING) &&
                                                (FP_FMDN_PROVISIONING_INIT_TIMEOUT_MS > 0);
                        /* Control timer for monitoring FMDN provisioning timeout */
                        fp_fmdn_set_provisioning_timer(start_prov_timer);
                }
#endif /* FP_LOCATOR_TAG */
#if (FP_DIS_ENABLE == 1)
                if (prov_state >= FP_FMDN_PROVISIONING_INITIATING) {
                        if (state == FP_FMDN_PROVISIONING_STOPPED) {
                                /* Disable read access authorization for DIS characteristics */
                                dis_set_authorized_read(fp_dis, BLE_CONN_IDX_INVALID, false);
                        }
                } else {
                        if (state > FP_FMDN_PROVISIONING_INITIATING) {
                                /* Enable read access authorization for DIS characteristics */
                                dis_set_authorized_read(fp_dis, BLE_CONN_IDX_INVALID, true);
                        }
                }
#endif /* FP_DIS_ENABLE */
                /* Trigger any pending ID rotation */
                trigger_pending_addr_rotation();

                if (core_ctx.fmdn_prov_status_cb && state != prov_state) {
                        if (state == FP_FMDN_PROVISIONING_STARTED &&
                            prov_state != FP_FMDN_PROVISIONING_INITIATING) {
                                core_ctx.fmdn_prov_status_cb(FP_FMDN_PROV_STAT_STARTED);
                        } else if (state == FP_FMDN_PROVISIONING_STOPPED) {
                                core_ctx.fmdn_prov_status_cb(FP_FMDN_PROV_STAT_STOPPED);
                        }
                }
                break;
        case FP_FMDN_PROVISIONING_WAITING:
                if (fp_get_fmdn_provisioning_state() < FP_FMDN_PROVISIONING_INITIATING) {
                        core_ctx.prov_state = FP_FMDN_PROVISIONING_WAITING;
#if (FP_LOCATOR_TAG == 1)
                        /*
                         * If device is not already FMDN provisioned, start timer for monitoring
                         * provisioning timeout
                         */
                        fp_fmdn_set_provisioning_timer(true);
#endif /* FP_LOCATOR_TAG */
                }
                break;
        }
}

FP_FMDN_PROVISIONING_STATE fp_get_fmdn_provisioning_state(void)
{
        return core_ctx.prov_state;
}

void fp_set_authenticated_conn(uint16_t conn_idx)
{
        bool trigger_cb = (core_ctx.auth_conn_idx != conn_idx);

        core_ctx.auth_conn_idx = conn_idx;

#if (FP_DIS_ENABLE == 1)
        /* Authorize read access to DIS characteristics for the connection */
        dis_set_authorized_read(fp_dis, conn_idx, true);
#endif /* FP_DIS_ENABLE */

        if (core_ctx.auth_conn_cb && trigger_cb) {
                core_ctx.auth_conn_cb(conn_idx);
        }
}
#endif /* FP_FMDN */

/* Command handler for setting random address renewal for Fast Pair/FMDN */
static void fp_set_rand_addr_renewal_cmd_handler(void *param)
{
        const fp_set_rand_addr_renewal_cmd_t *cmd = param;
        fp_set_rand_addr_renewal_rsp_t *rsp;
        ble_mgr_common_stack_msg_t *gmsg;
        struct app_gfmdn_set_rand_addr_renewal_cmd *gcmd;

        gmsg = ble_gtl_alloc(APP_GFMDN_SET_RAND_ADDR_RENEWAL_CMD, TASK_ID_APP, sizeof(*gcmd));
        gcmd = (struct app_gfmdn_set_rand_addr_renewal_cmd *) gmsg->msg.gtl.param;
        gcmd->operation = APP_GFMDN_SET_RAND_ADDR_RENEWAL;
        gcmd->status = cmd->status;

        ble_gtl_send(gmsg);

        ble_msg_free(param);
        rsp = ble_msg_init(FP_SET_RAND_ADDR_RENEWAL_CMD, sizeof(*rsp));

        rsp->status = BLE_STATUS_OK;
        ble_mgr_response_queue_send(&rsp, OS_QUEUE_FOREVER);
}

ble_error_t fp_set_rand_addr_renewal(uint8_t status)
{
        fp_set_rand_addr_renewal_cmd_t *cmd;
        fp_set_rand_addr_renewal_rsp_t *rsp;
        ble_error_t ret = BLE_ERROR_FAILED;

        /* Create new command and fill it */
        cmd = alloc_ble_msg(FP_SET_RAND_ADDR_RENEWAL_CMD, sizeof(*cmd));
        cmd->status = status;

        if (!ble_cmd_execute(cmd, (void **) &rsp, fp_set_rand_addr_renewal_cmd_handler)) {
                return BLE_ERROR_BUSY;
        }

        ret = rsp->status;
        OS_FREE(rsp);

        return ret;
}

int fp_add_custom_advertise(uint8_t structs_count, const gap_adv_ad_struct_t *structs)
{
        uint8_t i;
        uint8_t structs_len = 0;
        uint8_t avail_adv_data_size = AVAILABLE_ADV_DATA_SIZE;

        /* Adapt default number of advertise structs if calibrated TX power level is included */
        uint8_t default_adv_structs_count = (FP_ADVERTISE_CALIBRATED_TX_POWER_LEVEL == 1) ? 3 : 2;

        OS_ASSERT(fp_initialized);

        /* Return if maximum number of advertise structs is reached */
        if (core_ctx.adv_struct_count + structs_count > ADV_STRUCT_COUNT_MAX) {
                return 1;
        }

        for (i = default_adv_structs_count; i < core_ctx.adv_struct_count; i++) {
                avail_adv_data_size -= core_ctx.adv_struct[i].len + 2;  /* 2 bytes for type/length fields */
        }
        for (i = 0; i < structs_count; i++) {
                structs_len += structs[i].len + 2;  /* 2 bytes for type/length fields */
        }

        /* Return if there is no space available in advertise payload to add custom data */
        if (structs_len > avail_adv_data_size) {
                return 2;
        }

        /* Copy advertise structures */
        for (i = 0 ; i < structs_count; i++) {
                uint8_t adv_i = i + core_ctx.adv_struct_count;

                core_ctx.adv_struct[adv_i].data = structs[i].data;
                core_ctx.adv_struct[adv_i].len = structs[i].len;
                core_ctx.adv_struct[adv_i].type = structs[i].type;
        }

        core_ctx.adv_struct_count += structs_count;

        return 0;
}

int fp_set_scan_response(uint8_t structs_count, const gap_adv_ad_struct_t *structs)
{
        OS_ASSERT(fp_initialized);

        core_ctx.scan_rsp_struct_count = structs_count;
        core_ctx.scan_rsp_struct = structs;

        return fp_update_advertise_data(fp_procedure_is_pairing_mode());
}

#if (FP_LOCATOR_TAG != 1)
void fp_set_acc_key_filter_ui_indication(bool enable)
{
        bool update_adv_data;

        OS_ASSERT(fp_initialized);

        update_adv_data = (core_ctx.show_acc_key_filter_ui_indication != enable);

        if (update_adv_data) {
                core_ctx.show_acc_key_filter_ui_indication = enable;

                fp_acc_keys_schedule_new_advertise_struct();
                fp_update_advertise_data(false);
        }
}
#endif /* FP_LOCATOR_TAG */

#if (FP_BATTERIES_COUNT != 0)
int fp_set_battery_information(const fp_battery_info_t info[FP_BATTERIES_COUNT])
{
        int err = 0;
        bool update_adv_data = false;
        bool update_batt_info = true;

        OS_ASSERT(fp_initialized);

#if (FP_BATTERY_NOTIFICATION == 1) || (FP_FMDN == 1)
        update_adv_data = memcmp(core_ctx.info, info, sizeof(core_ctx.info));
        update_batt_info = update_adv_data;
#endif

        if (update_batt_info) {
                memcpy(core_ctx.info, info, sizeof(fp_battery_info_t) * FP_BATTERIES_COUNT);
        }

        if (update_adv_data) {
#if (FP_BATTERY_NOTIFICATION == 1)
                fp_acc_keys_schedule_new_advertise_struct();
#endif
#if (FP_FMDN == 1)
                fp_fmdn_schedule_new_advertise_struct();
#endif
                err += fp_update_advertise_data(false);
        }

        return err;
}

#if (FP_BATTERY_NOTIFICATION == 1)
void fp_set_battery_ui_indication(bool enable)
{
        bool update_adv_data;

        OS_ASSERT(fp_initialized);

        update_adv_data = (core_ctx.show_battery_ui_indication != enable);

        if (update_adv_data) {
                core_ctx.show_battery_ui_indication = enable;

                fp_acc_keys_schedule_new_advertise_struct();
                fp_update_advertise_data(false);
        }
}
#endif /* FP_BATTERY_NOTIFICATION */
#endif /* FP_BATTERIES_COUNT */

#if (FP_FMDN == 1)
#if (FP_FMDN_RING_COMPONENTS_NUM > 0)
int fp_stop_ringing(void)
{
        int err = 0;

        OS_ASSERT(fp_initialized);

        if (fp_fmdn_is_ringing()) {
                err += fp_fmdn_stop_ringing(FP_FMDN_RING_BUTTON_STOPPED);
        }

        return err;
}

bool fp_is_ringing(void)
{
        OS_ASSERT(fp_initialized);

        return fp_fmdn_is_ringing();
}
#endif /* FP_FMDN_RING_COMPONENTS_NUM */

void fp_set_user_consent(bool enable)
{
        OS_ASSERT(fp_initialized);

        fp_fmdn_set_user_consent(enable);
}

bool fp_is_fmdn_provisioned(void)
{
        OS_ASSERT(fp_initialized);

        return (core_ctx.prov_state >= FP_FMDN_PROVISIONING_INITIATING);
}

bool fp_is_conn_authenticated(uint16_t conn_idx)
{
        OS_ASSERT(fp_initialized);

        return (core_ctx.auth_conn_idx == conn_idx);
}
#endif /* FP_FMDN */

int fp_set_firmware_status(ble_service_t *dis, uint16_t conn_idx, FP_FW_STATUS status)
{
        int ret = -1;
#if (FP_DIS_ENABLE == 1)
        dis = fp_dis;
#endif
        OS_ASSERT(fp_initialized);
        OS_ASSERT(dis);

        (void) conn_idx;

#if (FP_DIS_ENABLE == 1)
#if (FP_FMDN == 1)
        /* Authorize read access to DIS characteristics accordingly */
        if (fp_is_fmdn_provisioned()) {
                dis_set_authorized_read(dis, conn_idx, true);
        }
#endif /* FP_FMDN */
#endif /* FP_DIS_ENABLE */

        switch (status) {
        case FP_FW_NORMAL:
                ret = dis_set_fw_revision(dis, defaultBLE_DIS_FW_REVISION);
                break;
        case FP_FW_UPDATING:
                ret = dis_set_fw_revision(dis, "status-updating");
                break;
        case FP_FW_ABNORMAL:
                ret = dis_set_fw_revision(dis, "status-abnormal");
                break;
        }

        return ret;
}

void fp_add_services_to_db(void)
{
        /* Add Google Fast Pair Service */
        fp_gfps = gfps_init(&gfps_info, &fast_pair_callbacks);
#if (FP_DIS_ENABLE == 1)
        /* Add Device Information Service */
        fp_dis = dis_init(NULL, &dis_info);

#if (FP_FMDN == 1)
        /* Authorize read access to DIS characteristics accordingly */
        if (fp_is_fmdn_provisioned()) {
                dis_set_authorized_read(fp_dis, BLE_CONN_IDX_INVALID, true);
        }
#endif /* FP_FMDN */
#endif /* FP_DIS_ENABLE */
#if (FP_FMDN == 1)
        /* Add Accessory Non-Owner Service */
        fp_ano_init();
#endif /* FP_FMDN */
}

void fp_remove_services_from_db(void)
{
        ble_service_remove(fp_gfps);
        ble_service_cleanup(fp_gfps);
        fp_gfps = NULL;
#if (FP_DIS_ENABLE == 1)
        ble_service_remove(fp_dis);
        ble_service_cleanup(fp_dis);
        fp_dis = NULL;
#endif /* FP_DIS_ENABLE */
#if (FP_FMDN == 1)
        fp_ano_deinit();
#endif
}

void fp_error(int error_code)
{
        FP_LOG_PRINTF("Google Fast Pair error: %d\r\n", error_code);
        core_ctx.error_cb(error_code);
}

/*
 * Define default implementation of Google Fast Pair framework BLE advertising API functions
 * addressing the case where only Fast Pair or FMDN frames are advertised
 */

__WEAK void fp_ble_adv_init(void)
{
        return;
}

__WEAK void fp_ble_adv_deinit(void)
{
        return;
}

__WEAK ble_error_t fp_ble_adv_set_params(const fp_ble_adv_params_t *params)
{
        ble_gap_adv_mode_set(params->mode);
        ble_gap_adv_intv_set(params->intv_min, params->intv_max);

        return ble_gap_tx_power_set(GAP_AIR_OP_ADV | GAP_AIR_OP_SCAN | GAP_AIR_OP_INITIATE,
                                                                                params->tx_power);
}

__WEAK ble_error_t fp_ble_adv_set_tx_power(gap_tx_power_t tx_power)
{
        return ble_gap_tx_power_set(GAP_AIR_OP_ADV | GAP_AIR_OP_SCAN | GAP_AIR_OP_INITIATE, tx_power);
}

__WEAK ble_error_t fp_ble_adv_set_ad_struct(size_t ad_len, const gap_adv_ad_struct_t *ad,
        size_t sd_len, const gap_adv_ad_struct_t *sd)
{
        return ble_gap_adv_ad_struct_set(ad_len, ad, sd_len, sd);
}

__WEAK ble_error_t fp_ble_adv_start(void)
{
        return ble_gap_adv_start(GAP_CONN_MODE_UNDIRECTED);
}

__WEAK bool fp_ble_adv_is_started(void)
{
        return false;
}

__WEAK ble_error_t fp_ble_adv_stop(void)
{
        return ble_gap_adv_stop();
}

__WEAK ble_error_t fp_ble_adv_stop_all(void)
{
        return ble_gap_adv_stop();
}
