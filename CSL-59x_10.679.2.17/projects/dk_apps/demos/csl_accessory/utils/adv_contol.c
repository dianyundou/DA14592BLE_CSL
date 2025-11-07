/**
 ****************************************************************************************
 *
 * @file adv_control.c
 *
 * @brief Advertising control implementation
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
#include <stdbool.h>
#include <string.h>
#include "sdk_defs.h"
#include "accessory_config.h"
#include "ble_gap.h"
#include "ble_common.h"
#include "osal.h"
#include "sys_timer.h"
#include "notification_bits.h"
#include "fast_pair.h"

#include "adv_control.h"

/* Interval (in msec) at which advertising is periodically enabled in low power mode */
#ifndef ADV_CONTROL_LOW_POWER_INTERVAL_MS
#define ADV_CONTROL_LOW_POWER_INTERVAL_MS       ( 10 * 60000 )  /* 10 minutes */
#endif

/* Timeout interval (in msec) during which advertising is enabled in low power mode */
#ifndef ADV_CONTROL_LOW_POWER_ADV_TIMEOUT_MS
#define ADV_CONTROL_LOW_POWER_ADV_TIMEOUT_MS    ( 10 * 1000 )  /* 10 seconds */
#endif

#if (ADV_CONTROL_LOW_POWER_ADV_TIMEOUT_MS >= ADV_CONTROL_LOW_POWER_INTERVAL_MS)
#error "Low power advertising timeout shall always be greater than low power advertising interval."
#endif

#if (ADV_CONTROL_MULT_EVENTS_ENABLE == 1)
/* Maximum number of different advertising events that can be handled */
#ifndef ADV_CONTROL_EVENT_COUNT_MAX
#define ADV_CONTROL_EVENT_COUNT_MAX             (2)
#endif

/* Minimum interval between advertising events (in milliseconds) */
#ifndef ADV_CONTROL_EVENT_MIN_INTV_MS
#define ADV_CONTROL_EVENT_MIN_INTV_MS           (20)
#endif
#endif /* ADV_CONTROL_MULT_EVENTS_ENABLE */

/* Advertising control notification bits */
#define ADV_CONTROL_LOW_POWER_MODE_NOTIF        (1 << 1)
#define ADV_CONTROL_EVENT_NOTIF                 (1 << 2)

#if (ADV_CONTROL_MULT_EVENTS_ENABLE == 1)
/* Advertising event command types */
typedef enum {
        ADV_EVENT_CMD_DELETE,
        ADV_EVENT_CMD_STOP,
        ADV_EVENT_CMD_START,
        ADV_EVENT_CMD_ADV_COMPLETED
} ADV_EVENT_CMD;

/* Advertising event update reason type */
typedef enum {
        ADV_EVENT_RSN_STOP_EVT,
        ADV_EVENT_RSN_START_EVT,
        ADV_EVENT_RSN_NOTIF
} ADV_EVENT_RSN;

/* Advertising events handled */
__RETAINED static struct adv_event_t {
        adv_control_event_params_t params;
        size_t ad_len;
        uint8_t ad[BLE_ADV_DATA_LEN_MAX];
        size_t sd_len;
        uint8_t sd[BLE_SCAN_RSP_LEN_MAX];
        bool is_valid;
        bool is_active;
        uint16_t last_time_ms;
} adv_events[ADV_CONTROL_EVENT_COUNT_MAX];
#endif /* ADV_CONTROL_MULT_EVENTS_ENABLE */

/* Advertising control mode */
__RETAINED static ADV_CONTROL_MODE adv_control_mode;
/* Low power mode advertising is active (sending advertising frames) */
__RETAINED static bool adv_control_low_power_adv_active;
/* Timer used to handle advertising in low power mode */
__RETAINED static OS_TIMER adv_control_low_power_tim;
/* Advertising control internal notification */
__RETAINED static uint32_t adv_control_notif;
#if (ADV_CONTROL_MULT_EVENTS_ENABLE == 1)
/* Last advertising event */
__RETAINED static struct adv_event_t *adv_control_last_evt;
/* Number of active advertising events */
__RETAINED static uint8_t adv_control_evt_count;
/* Indicate start-advertising request after stopping advertising */
__RETAINED static bool adv_control_adv_start_req;
/* Indicate stop-advertising request pending */
__RETAINED static bool adv_control_adv_stop_req;
/* Timer used for scheduling different advertising events */
__RETAINED static OS_TIMER adv_control_event_tim;
/* Force update last_time_ms for all timers after enabling advertising */
__RETAINED static bool adv_control_update_now;
#endif /* ADV_CONTROL_MULT_EVENTS_ENABLE */
/* Control OS task handle */
__RETAINED static OS_TASK adv_control_task_hdl;

#if (ADV_CONTROL_MULT_EVENTS_ENABLE == 1)
static ble_error_t update_adv_event(ADV_EVENT_CMD cmd, adv_control_event_t evt);
#endif /* ADV_CONTROL_MULT_EVENTS_ENABLE */

/* Send notification for advertising control */
static void adv_control_send_notifiation(uint32_t notif)
{
        OS_ENTER_CRITICAL_SECTION();
        adv_control_notif |= notif;
        OS_LEAVE_CRITICAL_SECTION();
        OS_TASK_NOTIFY(adv_control_task_hdl, ADV_CONTROL_NOTIF, OS_NOTIFY_SET_BITS);
}

/* Advertising control low power mode timer callback */
static void adv_control_low_power_tim_cb(OS_TIMER timer)
{
        adv_control_send_notifiation(ADV_CONTROL_LOW_POWER_MODE_NOTIF);
}

#if (ADV_CONTROL_MULT_EVENTS_ENABLE == 1)
/* Advertising events control timer callback */
static void adv_control_event_tim_cb(OS_TIMER timer)
{
        adv_control_send_notifiation(ADV_CONTROL_EVENT_NOTIF);
}

/* Check whether advertising is forced disabled */
static bool adv_control_is_adv_disabled(void)
{
        return (adv_control_mode == ADV_CONTROL_MODE_DISABLED) ||
               (adv_control_mode == ADV_CONTROL_MODE_LOW_POWER && !adv_control_low_power_adv_active);
}
#endif /* ADV_CONTROL_MULT_EVENTS_ENABLE */

/* Set low power advertising state */
static void adv_control_set_low_power_adv(bool adv_active)
{
        uint32_t period_ms;

        adv_control_low_power_adv_active = adv_active;

        if (adv_active) {
                period_ms = ADV_CONTROL_LOW_POWER_ADV_TIMEOUT_MS;
                if (fp_is_initialized() && fp_is_advertise_stopped()) {
                        fp_start_advertise();
                }
#if (ADV_CONTROL_MULT_EVENTS_ENABLE == 1)
                adv_control_update_now = true;
                update_adv_event(ADV_EVENT_CMD_ADV_COMPLETED, NULL);
#endif /* ADV_CONTROL_MULT_EVENTS_ENABLE */
        } else {
                period_ms = ADV_CONTROL_LOW_POWER_INTERVAL_MS - ADV_CONTROL_LOW_POWER_ADV_TIMEOUT_MS;
                if (fp_is_initialized()) {
                        fp_stop_advertise();
                }
#if (ADV_CONTROL_MULT_EVENTS_ENABLE == 1)
                OS_TIMER_STOP(adv_control_event_tim, OS_TIMER_FOREVER);
                ble_gap_adv_stop();
                adv_control_adv_start_req = false;
                adv_control_adv_stop_req = false;
                adv_control_last_evt = NULL;
#endif /* ADV_CONTROL_MULT_EVENTS_ENABLE */
        }

        OS_TIMER_CHANGE_PERIOD(adv_control_low_power_tim, OS_MS_2_TICKS(period_ms),
                OS_TIMER_FOREVER);
}

#if (ADV_CONTROL_MULT_EVENTS_ENABLE == 1)
/* Returns pointer to free advertising event entry */
static struct adv_event_t *search_for_free_adv_event(void)
{
        for (int i = 0; i < ADV_CONTROL_EVENT_COUNT_MAX; i++) {
                if (!adv_events[i].is_valid) {
                        return &adv_events[i];
                }
        }

        return NULL;
}

/* Updates BLE advertisement */
static ble_error_t update_adv(struct adv_event_t *adv_evt, bool restart_adv, uint16_t now_ms)
{
        ble_error_t ret;

        if (adv_control_adv_start_req || adv_evt == NULL) {
                return BLE_STATUS_OK;
        }

        if (!adv_control_event_is_active((adv_control_event_t) adv_evt)) {
                return BLE_STATUS_OK;
        }

        if (restart_adv) {
                /* Advertising isn't started, start it now */
                if (ble_gap_adv_stop() != BLE_ERROR_NOT_ALLOWED) {
                        adv_control_adv_start_req = true;
                        return BLE_STATUS_OK;
                }
        }

        ble_gap_adv_mode_set(adv_evt->params.mode);
        ble_gap_adv_intv_set(adv_evt->params.intv_min, adv_evt->params.intv_max);

        ret = ble_gap_tx_power_set(GAP_AIR_OP_ADV | GAP_AIR_OP_SCAN | GAP_AIR_OP_INITIATE,
                                                                        adv_evt->params.tx_power);

        if (ret == BLE_STATUS_OK) {
                ret = ble_gap_adv_data_set(adv_evt->ad_len, adv_evt->ad, adv_evt->sd_len, adv_evt->sd);
        }

        if (ret == BLE_STATUS_OK && !adv_control_is_adv_disabled()) {
                ret = ble_gap_adv_start(adv_evt->params.type);
                adv_evt->last_time_ms = now_ms;
        }

        return ret;
}

/* Updates the state of handled advertising events */
static ble_error_t update_handled_adv_events(ADV_EVENT_RSN rsn, struct adv_event_t *evt)
{
        uint16_t now_ms = (uint16_t) (sys_timer_get_uptime_usec() / 1000);
        uint16_t min_next_adv_in_x_ms = 0, next_adv_in_x_ms = (uint16_t) -1;
        ble_error_t ret = BLE_STATUS_OK;
        uint8_t count_active = 0;
        bool adv_updated = false;
        bool adv_is_disabled = adv_control_is_adv_disabled();

        if (rsn == ADV_EVENT_RSN_START_EVT) {
                evt->last_time_ms = now_ms - BLE_ADV_INTERVAL_TO_MS(evt->params.intv_max);

                /* Reset last time the currently active event has been advertised; if it is the
                 * only one, it is not monitored */
                if (adv_control_evt_count == 1 && adv_control_last_evt &&
                    adv_control_last_evt->is_active) {
                        adv_control_last_evt->last_time_ms = now_ms;
                }
        }

        if (adv_control_last_evt) {
                /* Advance last time the currently active event has been advertised */
                uint16_t intv_ms = BLE_ADV_INTERVAL_TO_MS(adv_control_last_evt->params.intv_max);
                uint16_t elapsed = now_ms - adv_control_last_evt->last_time_ms;
                if (elapsed > 0) {
                        adv_control_last_evt->last_time_ms +=
                                intv_ms * ((elapsed + intv_ms - 1) / intv_ms) - intv_ms;
                }

                /* Calculate min time after last advertised event required before starting next event */
                min_next_adv_in_x_ms =
                        adv_control_last_evt->last_time_ms + ADV_CONTROL_EVENT_MIN_INTV_MS;
                if ((uint16_t) (min_next_adv_in_x_ms - now_ms) < ((uint16_t) -1)/2) {
                        min_next_adv_in_x_ms -= now_ms;
                } else {
                        min_next_adv_in_x_ms = 0;
                }
        }

        /* Update the state of advertising events */
        for (int i = 0; i < ADV_CONTROL_EVENT_COUNT_MAX; i++) {
                struct adv_event_t *adv_evt = &adv_events[i];
                bool is_active = adv_control_event_is_active((adv_control_event_t) adv_evt);

                count_active += is_active;

                if (is_active && !adv_is_disabled) {
                        uint16_t intv_ms = BLE_ADV_INTERVAL_TO_MS(adv_evt->params.intv_max);

                        if (adv_control_update_now) {
                                adv_evt->last_time_ms = now_ms - intv_ms;
                        }

                        uint16_t elapsed = now_ms - adv_evt->last_time_ms;
                        uint16_t remaining = (intv_ms > elapsed) ? (intv_ms - elapsed) : 0;

                        if (remaining < min_next_adv_in_x_ms) {
                                remaining = min_next_adv_in_x_ms;
                        }

                        if (OS_MS_2_TICKS(remaining) == 0 && !adv_updated) {
                                if (adv_control_last_evt != adv_evt) {
                                        ret = update_adv(adv_evt, true, now_ms);
                                }

                                if (ret == BLE_STATUS_OK) {
                                        adv_control_last_evt = adv_evt;
                                        min_next_adv_in_x_ms = ADV_CONTROL_EVENT_MIN_INTV_MS;
                                }

                                adv_updated = true;
                        } else if (remaining < next_adv_in_x_ms) {
                                next_adv_in_x_ms = remaining;
                        }
                }
        }

        if (adv_control_update_now) {
                adv_control_update_now = false;
        }

        adv_control_evt_count = count_active;

        if (rsn == ADV_EVENT_RSN_STOP_EVT) {
                /* Indicate that advertising event has stopped */
                if (ret == BLE_STATUS_OK) {
                        ret = BLE_ERROR_NOT_ALLOWED;
                }

                if (count_active == 0 && !adv_is_disabled) {
                        /* There are no active advertising events, stop advertising */
                        adv_control_last_evt = NULL;
                        OS_TIMER_STOP(adv_control_event_tim, OS_TIMER_FOREVER);
                        adv_control_adv_stop_req = (ble_gap_adv_stop() != BLE_ERROR_NOT_ALLOWED);
                        return ret;
                }
        }

        /* Start timer for advertising next event */
        if (next_adv_in_x_ms != (uint16_t) -1) {
                OS_TICK_TIME tmr_ticks = OS_MS_2_TICKS(next_adv_in_x_ms);

                if (tmr_ticks > 1) {
                        OS_TIMER_CHANGE_PERIOD(adv_control_event_tim, tmr_ticks, OS_TIMER_FOREVER);
                } else {
                        adv_control_send_notifiation(ADV_CONTROL_EVENT_NOTIF);
                }
        }

        return ret;
}

/* Updates the state of an advertising event */
static ble_error_t update_adv_event(ADV_EVENT_CMD cmd, adv_control_event_t evt)
{
        struct adv_event_t *adv_evt = (struct adv_event_t *) evt;
        ADV_EVENT_RSN rsn = ADV_EVENT_RSN_STOP_EVT;

        if (cmd != ADV_EVENT_CMD_ADV_COMPLETED && !adv_evt->is_valid) {
                return BLE_ERROR_FAILED;
        }

        switch (cmd) {
        case ADV_EVENT_CMD_DELETE:
                adv_evt->is_valid = false;
                /* no break */
        case ADV_EVENT_CMD_STOP:
                if (!adv_evt->is_active) {
                        return BLE_ERROR_NOT_ALLOWED;
                }

                adv_evt->is_active = false;
                if (adv_control_last_evt == adv_evt) {
                        adv_control_last_evt = NULL;
                }
                rsn = ADV_EVENT_RSN_STOP_EVT;

                break;
        case ADV_EVENT_CMD_START:
                if (adv_evt->is_active) {
                        return BLE_ERROR_IN_PROGRESS;
                }

                adv_evt->is_active = true;
                rsn = ADV_EVENT_RSN_START_EVT;

                break;
        case ADV_EVENT_CMD_ADV_COMPLETED:
                if (adv_control_evt_count && adv_control_adv_start_req) {
                        uint16_t now_ms = (uint16_t) (sys_timer_get_uptime_usec() / 1000);
                        adv_control_adv_start_req = false;
                        adv_control_adv_stop_req = false;
                        return update_adv(adv_control_last_evt, false, now_ms);
                } else {
                        adv_control_adv_start_req = false;

                        if (adv_control_adv_stop_req) {
                                adv_control_adv_stop_req = false;
                                return BLE_STATUS_OK;
                        }
                }

                rsn = ADV_EVENT_RSN_NOTIF;
                adv_control_last_evt = NULL;

                break;
        }

        return update_handled_adv_events(rsn, adv_evt);
}

/* Convert advertising struct format to advertising payload */
static ble_error_t ad_format_serialize(size_t dst_len, uint8_t *dst, size_t src_len,
                                                const gap_adv_ad_struct_t *src, size_t *written)
{
        int i, wr_index;

        for (i = 0, wr_index = 0; i < src_len; i++) {
                if (wr_index >= dst_len) {
                        return BLE_ERROR_INVALID_PARAM;
                }

                /* Serialize gap_adv_ad_struct_t AD object */
                dst[wr_index++] = src[i].len + 1;
                dst[wr_index++] = src[i].type;
                memcpy(&dst[wr_index], src[i].data, src[i].len);
                wr_index += src[i].len;
         }

        *written = wr_index;

        return BLE_STATUS_OK;
}

/* Disable all advertising events */
static void disable_adv_events(void)
{
        for (int i = 0; i < ADV_CONTROL_EVENT_COUNT_MAX; i++) {
                adv_events[i].is_active = false;
        }
        adv_control_last_evt = NULL;
}

/* Indicate that a connection is established, thus disable advertising */
static void adv_control_connected(void)
{
        adv_control_event_stop_all();
}

/* Indicate that advertising operation is completed */
static ble_error_t adv_control_adv_completed(void)
{
        if (adv_control_is_adv_disabled()) {
                return BLE_STATUS_OK;
        }

        return update_adv_event(ADV_EVENT_CMD_ADV_COMPLETED, NULL);
}
#endif /* ADV_CONTROL_MULT_EVENTS_ENABLE */

void adv_control_init(OS_TASK task_hdl)
{
        OS_ASSERT(task_hdl);

        adv_control_task_hdl = task_hdl;

        /* Create timer to implement low power mode advertising handling */
        adv_control_low_power_tim = OS_TIMER_CREATE("advctrlmode",
                                        OS_MS_2_TICKS(ADV_CONTROL_LOW_POWER_ADV_TIMEOUT_MS),
                                        OS_TIMER_ONCE, NULL, adv_control_low_power_tim_cb);
#if (ADV_CONTROL_MULT_EVENTS_ENABLE == 1)
        /* Create timer to implement advertising events handling */
        adv_control_event_tim =
                OS_TIMER_CREATE("advctrlevt", 1, OS_TIMER_ONCE, NULL, adv_control_event_tim_cb);
#endif /* ADV_CONTROL_MULT_EVENTS_ENABLE */
}

ADV_CONTROL_MODE adv_control_get_mode(void)
{
        return adv_control_mode;
}

void adv_control_set_mode(ADV_CONTROL_MODE mode)
{
        adv_control_mode = mode;

        switch (mode) {
        case ADV_CONTROL_MODE_NORMAL:
                OS_TIMER_STOP(adv_control_low_power_tim, OS_TIMER_FOREVER);
#if (ADV_CONTROL_MULT_EVENTS_ENABLE == 1)
                adv_control_update_now = true;
                update_adv_event(ADV_EVENT_CMD_ADV_COMPLETED, NULL);
#endif /* ADV_CONTROL_MULT_EVENTS_ENABLE */
                break;
        case ADV_CONTROL_MODE_LOW_POWER:
                adv_control_set_low_power_adv(true);
                break;
        case ADV_CONTROL_MODE_DISABLED:
                OS_TIMER_STOP(adv_control_low_power_tim, OS_TIMER_FOREVER);
#if (ADV_CONTROL_MULT_EVENTS_ENABLE == 1)
                OS_TIMER_STOP(adv_control_event_tim, OS_TIMER_FOREVER);
                ble_gap_adv_stop();
                adv_control_adv_start_req = false;
                adv_control_adv_stop_req = false;
                adv_control_last_evt = NULL;
#endif /* ADV_CONTROL_MULT_EVENTS_ENABLE */
                break;
        }
}

#if (ADV_CONTROL_MULT_EVENTS_ENABLE == 1)
adv_control_event_t adv_control_event_create(const adv_control_event_params_t *params)
{
        struct adv_event_t *adv_evt = search_for_free_adv_event();

        if (adv_evt == NULL) {
                return NULL;
        }

        adv_control_event_set_params(adv_evt, params);

        adv_evt->ad_len = 0;
        adv_evt->sd_len = 0;
        adv_evt->is_active = false;
        adv_evt->is_valid = true;

        return adv_evt;
}

ble_error_t adv_control_event_delete(adv_control_event_t evt)
{
        ble_error_t ret = update_adv_event(ADV_EVENT_CMD_DELETE, evt);

        return (ret == BLE_ERROR_NOT_ALLOWED) ? BLE_STATUS_OK : ret;
}

ble_error_t adv_control_event_set_params(adv_control_event_t evt,
        const adv_control_event_params_t *params)
{
        struct adv_event_t *adv_evt = (struct adv_event_t *) evt;

        if (params == NULL) {
                return BLE_ERROR_FAILED;
        }

        memcpy(&adv_evt->params, params, sizeof(adv_control_event_params_t));

        return BLE_STATUS_OK;
}

ble_error_t adv_control_event_set_tx_power(adv_control_event_t evt, gap_tx_power_t tx_power)
{
        struct adv_event_t *adv_evt = (struct adv_event_t *) evt;

        if (tx_power > GAP_TX_POWER_MAX) {
                return BLE_ERROR_INVALID_PARAM;
        }

        adv_evt->params.tx_power = tx_power;

        if (!adv_control_is_adv_disabled() &&
            adv_evt->is_active && adv_control_last_evt == adv_evt) {
                return ble_gap_tx_power_set(GAP_AIR_OP_ADV | GAP_AIR_OP_SCAN | GAP_AIR_OP_INITIATE,
                                                                                        tx_power);
        }

        return BLE_STATUS_OK;
}

ble_error_t adv_control_event_set_ad_struct(adv_control_event_t evt,
        size_t ad_len, const gap_adv_ad_struct_t *ad, size_t sd_len, const gap_adv_ad_struct_t *sd)
{
        struct adv_event_t *adv_evt = (struct adv_event_t *) evt;
        ble_error_t ret;

        ret = ad_format_serialize(BLE_ADV_DATA_LEN_MAX, adv_evt->ad, ad_len, ad, &adv_evt->ad_len);
        if (ret) {
                return ret;
        }

        if (sd) {
                ret = ad_format_serialize(BLE_SCAN_RSP_LEN_MAX, adv_evt->sd, sd_len, sd,
                                                                                &adv_evt->sd_len);
                if (ret) {
                        return ret;
                }
        } else {
                adv_evt->sd_len = 0;
        }

        if (!adv_control_is_adv_disabled() &&
            adv_evt->is_active && adv_control_last_evt == adv_evt) {
                return ble_gap_adv_data_set(adv_evt->ad_len, adv_evt->ad,
                                            adv_evt->sd_len, adv_evt->sd);
        }

        return BLE_STATUS_OK;
}

ble_error_t adv_control_event_start(adv_control_event_t evt)
{
        return update_adv_event(ADV_EVENT_CMD_START, evt);
}

ble_error_t adv_control_event_stop(adv_control_event_t evt)
{
        return update_adv_event(ADV_EVENT_CMD_STOP, evt);
}

ble_error_t adv_control_event_stop_all(void)
{
        disable_adv_events();

        return ble_gap_adv_stop();
}

bool adv_control_event_is_active(adv_control_event_t evt)
{
        struct adv_event_t *adv_evt = (struct adv_event_t *) evt;

        return (adv_evt->is_valid && adv_evt->is_active);
}

bool adv_control_handle_event(ble_evt_hdr_t *evt)
{
        ble_error_t ret __UNUSED;

        switch (evt->evt_code) {
        case BLE_EVT_GAP_CONNECTED:
                /* Indicate to advertising control that a connection is established */
                adv_control_connected();
                break;
        case BLE_EVT_GAP_ADV_COMPLETED:
                /* Indicate to advertising control that advertising operation is completed */
                adv_control_adv_completed();
                break;
        default:
                break;
        }

        return false;
}
#endif /* ADV_CONTROL_MULT_EVENTS_ENABLE */

void adv_control_process_notif(uint32_t notif)
{
        if (notif & ADV_CONTROL_NOTIF) {
                uint32_t notif;

                OS_ENTER_CRITICAL_SECTION();
                notif = adv_control_notif;
                adv_control_notif = 0;
                OS_LEAVE_CRITICAL_SECTION();

                if (notif & ADV_CONTROL_LOW_POWER_MODE_NOTIF) {
                        if (adv_control_mode == ADV_CONTROL_MODE_LOW_POWER) {
                                adv_control_set_low_power_adv(!adv_control_low_power_adv_active);
                        }
                }
#if (ADV_CONTROL_MULT_EVENTS_ENABLE == 1)
                if (notif & ADV_CONTROL_EVENT_NOTIF) {
                        update_handled_adv_events(ADV_EVENT_RSN_NOTIF, NULL);
                }
#endif /* ADV_CONTROL_MULT_EVENTS_ENABLE */
        }
}
