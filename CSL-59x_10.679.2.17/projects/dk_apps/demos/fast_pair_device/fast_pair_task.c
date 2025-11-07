/**
 ****************************************************************************************
 *
 * @file fast_pair_task.c
 *
 * @brief Google Fast Pair device with Find My Device Network (FMDN) extension application
 * implementation
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

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "sdk_defs.h"
#include "osal.h"
#include "sdk_list.h"

#include "fast_pair_device_config.h"
#include "fast_pair_config.h"

#include "hw_gpio.h"
#include "hw_wkup.h"
#include "hw_pdc.h"
#include "hw_cpm.h"

#include "ble_common.h"
#include "ble_gap.h"
#include "ble_gatts.h"

#include "sys_power_mgr.h"
#include "sys_watchdog.h"
#include "sys_timer.h"

#if (dg_configSUOTA_ASYMMETRIC == 1)
#include "asym_suota_utils.h"
#include "asym_suota.h"
#endif

#if (dg_configSUOTA_SUPPORT == 1)
#include "dlg_suota.h"
#endif

#include "platform_devices.h"
#include "app_params.h"
#include "fp_motion_detector_ext.h"
#include "led_control.h"
#include "battery_monitor.h"
#include "adv_control.h"
#include "notification_bits.h"

#include "fast_pair.h"

#define BYTE_SEQ_LE_U16(value)          (uint8_t)(value), (uint8_t)((value) >> 8)

#define MAX_NAME_LEN                    (BLE_SCAN_RSP_LEN_MAX - 2)

#define LED_BLINK_FOR_10S               (10000 / (LED_CONTROL_BLINK_RATE_DEFAULT * 2))
#define LED_BLINK_FOR_3S                ( 3000 / (LED_CONTROL_BLINK_RATE_DEFAULT * 2))

#define ASYM_SUOTA_UUID_DATA_LEN        (4)   /* Length, type, SUOTA service UUID */
#define ASYM_SUOTA_MANUF_DATA_LEN       (10)  /* Length, type, ID value */
#define ASYM_SUOTA_MANUF_DATA_ID_LEN    (8)   /* ID value: company ID, random value */

#if (USE_CONSOLE == 1)
extern void console_process_notif(uint32_t notif);
extern void console_uart_init(OS_TASK appTaskHandle);
#endif /* USE_CONSOLE */

/* GPIO information */
typedef struct {
        HW_GPIO_PORT port;
        HW_GPIO_PIN pin;
        HW_GPIO_MODE mode;
} gpio_info_t;

/* User button configuration array */
static const gpio_info_t button_array[] = {
        { .port = USER_BUTTON_PORT,  .pin = USER_BUTTON_PIN,  .mode = USER_BUTTON_MODE  },
#if (dg_configUSE_HW_QSPI != 1)
        { .port = USER_BUTTON2_PORT, .pin = USER_BUTTON2_PIN, .mode = USER_BUTTON2_MODE }
#endif
};

#if (dg_configSUOTA_SUPPORT == 1)
/* Store information about ongoing SUOTA. */
__RETAINED static bool suota_ongoing;
#endif
#if (dg_configSUOTA_ASYMMETRIC == 1)
/* Advertising data for Asymmetric SUOTA */
__RETAINED_RW static uint8_t asym_suota_adv_data[] = {
        /* SUOTA service UUID data */
        ASYM_SUOTA_UUID_DATA_LEN - 1, GAP_DATA_TYPE_UUID16_LIST_INC,
        BYTE_SEQ_LE_U16(dg_configBLE_UUID_SUOTA_SERVICE),
        /* Manufacturer specific data - ID value (8 bytes) for Asymmetric SUOTA */
        ASYM_SUOTA_MANUF_DATA_LEN - 1, GAP_DATA_TYPE_MANUFACTURER_SPEC,
        BYTE_SEQ_LE_U16(DLG_COMPANY_ID), 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};
#endif /* dg_configSUOTA_ASYMMETRIC */
/* Timer used to stop pairing mode */
__RETAINED static OS_TIMER pairing_mode_tim;
/* Timer used to handle user button long press */
__RETAINED static OS_TIMER user_long_press_tim;
#if (FP_FMDN == 1)
/* Timer used to handle user button short press */
__RETAINED static OS_TIMER user_short_press_tim;
/* Timer used to handle user button double press */
__RETAINED static OS_TIMER user_double_press_tim;
#endif /* FP_FMDN */
/* Timer used to handle factory reset user button press */
__RETAINED static OS_TIMER factory_reset_press_tim;
#if (FP_FMDN == 1)
/* Timer used to handle advertise stop user button press */
__RETAINED static OS_TIMER adv_stop_press_tim;
/* Timer used to handle beacon time update in NVM storage */
__RETAINED static OS_TIMER beacon_time_storage_tim;
/* Beacon time offset read from NVM storage */
__RETAINED static uint32_t beacon_time_offset;
#if (FP_BATTERIES_COUNT != 0)
/* Battery level is low */
__RETAINED static bool batt_level_low;
#endif /* FP_BATTERIES_COUNT */
#endif /* FP_FMDN */
/* Indication that user intentionally stopped advertising */
__RETAINED static bool user_adv_stopped;
/* Application and Google Fast Pair framework OS task handle */
__RETAINED static OS_TASK fast_pair_task_hdl;

/* Connection status */
typedef struct {
        void *next;
        bool expired;
        uint16_t conn_idx;
        OS_TIMER param_tim;
} conn_dev_t;

/* List of connected devices */
__RETAINED static void *conn_devices;
/* Pointer to next connected device waiting for connection parameters update */
__RETAINED static conn_dev_t *param_update_devices;

static void pairing_mode_stop(void);
static void update_adv_status(void);
#if (FP_FMDN == 1)
static void beacon_time_storage_tim_cb(OS_TIMER timer);
#endif

/* Buffer must have length at least max_len + 1 */
static uint16_t read_name(uint16_t max_len, char *name_buf)
{
        uint16_t read_len = 0;

        app_params_t params[] = {
                { .param = APP_PARAMS_BLE_APP_NAME, .data = name_buf, .len = max_len }
        };
        read_len = app_params_get_params(params, ARRAY_LENGTH(params));

        if (read_len == 0) {
                strcpy(name_buf, DEVICE_DEFAULT_NAME);
                return strlen(DEVICE_DEFAULT_NAME);
        }

        name_buf[read_len] = '\0';

        return read_len;
}

/* Notify Fast Pair OS task */
static void notify_fast_pair_task(uint32_t notif)
{
        OS_TASK_NOTIFY(fast_pair_task_hdl, notif, OS_NOTIFY_SET_BITS);
}

/* Match connection by connection index */
static bool conn_params_match(const void *elem, const void *ud)
{
        conn_dev_t *conn_dev = (conn_dev_t *) elem;
        uint16_t conn_idx = (uint16_t) (uint32_t) ud;

        return conn_dev->conn_idx == conn_idx;
}

/*
 * This timer callback notifies the task that time for discovery, bonding and encryption
 * elapsed, and connection parameters can be changed to the preferred ones.
 */
static void conn_params_tim_cb(OS_TIMER timer)
{
        conn_dev_t *conn_dev = (conn_dev_t *) OS_TIMER_GET_TIMER_ID(timer);

        conn_dev = list_find(param_update_devices, conn_params_match,
                        (const void *) (uint32_t) conn_dev->conn_idx);
        if (conn_dev) {
                conn_dev->expired = true;
                notify_fast_pair_task(UPDATE_CONN_PARAM_NOTIF);
        }
}

/* Update connection parameters */
static void conn_param_update(uint16_t conn_idx)
{
        gap_conn_params_t cp;

        cp.interval_min = defaultBLE_PPCP_INTERVAL_MIN;
        cp.interval_max = defaultBLE_PPCP_INTERVAL_MAX;
        cp.slave_latency = defaultBLE_PPCP_SLAVE_LATENCY;
        cp.sup_timeout = defaultBLE_PPCP_SUP_TIMEOUT;

        ble_gap_conn_param_update(conn_idx, &cp);
}

#if (dg_configSUOTA_SUPPORT == 1)
/* Update connection parameters for SUOTA */
static void conn_param_update_for_suota(uint16_t conn_idx)
{
        gap_conn_params_t cp;

        cp.interval_min = BLE_CONN_INTERVAL_FROM_MS(20);     /* 20ms */
        cp.interval_max = BLE_CONN_INTERVAL_FROM_MS(60);     /* 60ms */
        cp.slave_latency = 0;
        cp.sup_timeout = BLE_SUPERVISION_TMO_FROM_MS(2000);  /* 2000ms */

        ble_gap_conn_param_update(conn_idx, &cp);

#if (dg_configBLE_2MBIT_PHY == 1)
        /* Switch to 2Mbit PHY during SUOTA */
        ble_gap_phy_set(conn_idx, BLE_GAP_PHY_PREF_2M, BLE_GAP_PHY_PREF_2M);
#endif

        /* Advertising is stopped while SUOTA is ongoing */
        fp_stop_advertise();
}
#endif /* dg_configSUOTA_SUPPORT */

/* Handler for BLE_EVT_GAP_CONNECTED event */
static void handle_evt_gap_connected(ble_evt_gap_connected_t *evt)
{
        conn_dev_t *conn_dev;

        /* Add timer that when expired will re-negotiate connection parameters */
        conn_dev = OS_MALLOC(sizeof(*conn_dev));
        if (conn_dev) {
                conn_dev->conn_idx = evt->conn_idx;
                conn_dev->expired = false;
                conn_dev->param_tim = OS_TIMER_CREATE("conn_param", OS_MS_2_TICKS(20000),
                                        OS_TIMER_ONCE, (uint32_t) conn_dev, conn_params_tim_cb);
                list_append(&conn_devices, conn_dev);
                if (param_update_devices == NULL) {
                        param_update_devices = conn_dev;
                }
                OS_TIMER_START(conn_dev->param_tim, OS_TIMER_FOREVER);
                ble_gap_conn_tx_power_set(evt->conn_idx, TX_POWER_CONN);
        }

#if (FP_LOCATOR_TAG != 1)
#if (dg_configSUOTA_ASYMMETRIC == 1)
        ble_gatts_service_changed_ind(evt->conn_idx, 0x0001, 0xFFFF);
#endif
#endif
}

/* Handler for BLE_EVT_GAP_DISCONNECTED event */
static void handle_evt_gap_disconnected(ble_evt_gap_disconnected_t *evt)
{
        conn_dev_t *conn_dev = list_unlink(&conn_devices, conn_params_match,
                                        (const void *) (uint32_t) evt->conn_idx);

        if (param_update_devices == conn_dev) {
                param_update_devices = conn_dev->next;
        }

        /*
         * The device is still in the list if the disconnection happened before the timer expiration.
         * In this case stop the timer and free the associated memory.
         */
        if (conn_dev) {
                if (conn_dev->param_tim) {
                        OS_TIMER_DELETE(conn_dev->param_tim, OS_TIMER_FOREVER);
                }
                OS_FREE(conn_dev);
        }
}

#if (FP_LOCATOR_TAG != 1)
/* Handler for BLE_EVT_GAP_PAIR_REQ event */
static void handle_evt_gap_pair_req(ble_evt_gap_pair_req_t *evt)
{
        FP_TASK_PRINTF("Pair request for conn index %u. Bond: %s\r\n",
                evt->conn_idx, evt->bond ? "true" : "false");

        /* Just accept the pairing request, set bond flag to what peer requested */
        ble_gap_pair_reply(evt->conn_idx, true, evt->bond);
}

/* Handler for BLE_EVT_GAP_PAIR_COMPLETED event */
static void handle_evt_gap_pair_completed(ble_evt_gap_pair_completed_t *evt)
{
        FP_TASK_PRINTF("Pair completed for conn index %u. Status: 0x%02X\r\n",
                evt->conn_idx, evt->status);
}
#endif /* !FP_LOCATOR_TAG */

#if (dg_configSUOTA_ASYMMETRIC == 1)
/* Callback indicating that a command has been received */
static void asym_suota_cmd_cb(ble_service_t *svc, uint16_t conn_idx, ASYM_SUOTA_CMD value)
{
        own_address_t addr;
        uint16_t write_len;

        /* Proceed only if a reboot-to-SUOTA-mode command has been received */
        if (value != ASYM_SUOTA_CMD_REBOOT_IN_SUOTA_MODE) {
                return;
        }

        /* Initiate disconnection with peer devices */
        ble_gap_disconnect(conn_idx, BLE_HCI_ERROR_REMOTE_USER_TERM_CON);

        /* Abort rebooting to SUOTA mode if more connections are still active */
        if (list_size(conn_devices) > 1) {
                return;
        }

        /* Get device address type */
        ble_gap_address_get(&addr);

        /* Set address type and random address renewal duration for Asymmetric SUOTA */
        app_params_t params[] = {
                { .param = APP_PARAMS_ASYM_SUOTA_BD_ADDR_TYPE,
                  .data = &addr.addr_type,
                  .len = sizeof(own_addr_type_t) },
                { .param = APP_PARAMS_ASYM_SUOTA_BD_ADDR_RENEW_DUR,
                  .data = (uint8_t[]) { BYTE_SEQ_LE_U16(1024) },  /* 1024 seconds renewal duration */
                  .len = sizeof(uint16_t) }
        };
        write_len = app_params_set_params(params, ARRAY_LENGTH(params));

        if (write_len != sizeof(own_addr_type_t) + sizeof(uint16_t)) {
                FP_TASK_PRINTF("Problem in writing address type and renewal duration to NVM storage"
                               " for Asymmetric SUOTA\r\n");
                OS_ASSERT(0);
        }

        OS_DELAY_MS(1000);

        asym_suota_utils_boot_suota_mode(true);
}

/* Callback indicating that a read ID request has been received */
static att_error_t asym_suota_id_cb(ble_service_t *svc, uint16_t conn_idx, uint8_t **value,
        uint16_t *length)
{
        uint16_t write_len;
        uint8_t *asym_suota_manuf_data =
                &asym_suota_adv_data[sizeof(asym_suota_adv_data) - ASYM_SUOTA_MANUF_DATA_ID_LEN];

        /* Compute random 6 byte value */
        for (int i = sizeof(uint16_t); i < ASYM_SUOTA_MANUF_DATA_ID_LEN; i++) {
                asym_suota_manuf_data[i] = (uint8_t) rand();
        }

        /* Set advertising data for Asymmetric SUOTA */
        app_params_t params[] = {
                { .param = APP_PARAMS_ASYM_SUOTA_ADV_DATA,
                  .data = asym_suota_adv_data,
                  .len = sizeof(asym_suota_adv_data) },
                { .param = APP_PARAMS_ASYM_SUOTA_ADV_DATA_LEN,
                  .data = (uint8_t[]) { sizeof(asym_suota_adv_data) },
                  .len = sizeof(uint8_t) }
        };
        write_len = app_params_set_params(params, ARRAY_LENGTH(params));

        if (write_len != sizeof(asym_suota_adv_data) + sizeof(uint8_t)) {
                FP_TASK_PRINTF("Problem in writing advertising data and length to NVM storage"
                               " for Asymmetric SUOTA\r\n");
                OS_ASSERT(0);
        }

        /* Return ID value and length*/
        *value = asym_suota_manuf_data;
        *length = ASYM_SUOTA_MANUF_DATA_ID_LEN;

        return ATT_ERROR_OK;
}

/* Asymmetric SUOTA callbacks */
static const asym_suota_callbacks_t asym_suota_cb = {
        .cmd_callback = asym_suota_cmd_cb,
        .id_callback = asym_suota_id_cb
};
#endif /* dg_configSUOTA_ASYMMETRIC */

#if (dg_configSUOTA_SUPPORT == 1)
#if (dg_configBLE_2MBIT_PHY == 1)
/* Handler for BLE_EVT_GAP_PHY_SET_COMPLETED event */
static void handle_ble_evt_gap_phy_set_completed(ble_evt_gap_phy_set_completed_t *evt)
{
        FP_TASK_PRINTF("Preferred PHY settings %s set\r\n",
                (evt->status == BLE_STATUS_OK) ? "were successfully" : "failed to be");
}

/* Handler for BLE_EVT_GAP_PHY_CHANGED event */
static void handle_ble_evt_gap_phy_changed(ble_evt_gap_phy_changed_t *evt)
{
        FP_TASK_PRINTF("PHY changed to TX:%s, RX:%s\r\n",
                (evt->tx_phy == BLE_GAP_PHY_1M) ? "1M" : "2M",
                (evt->rx_phy == BLE_GAP_PHY_1M) ? "1M" : "2M");
}
#endif /* (dg_configBLE_2MBIT_PHY == 1) */

/* Callback indicating that SUOTA is ready to start */
static bool suota_ready_cb(uint16_t conn_idx)
{
        /*
         * Application can accept or block SUOTA if 'true' or 'false' is returned, respectively
         */

        suota_ongoing = true;

        /* Update connection parameters for SUOTA (decreased connection interval, increased PHY) */
        conn_param_update_for_suota(conn_idx);

        return true;
}

/* Callback indicating that SUOTA status has changed */
static void suota_status_changed_cb(uint16_t conn_idx, uint8_t status, uint8_t error_code)
{
        /* Update firmware status */
        switch (status) {
        case SUOTA_START:
        case SUOTA_ONGOING:
                fp_set_firmware_status(NULL, conn_idx, FP_FW_UPDATING);
                break;
        case SUOTA_DONE:
                fp_set_firmware_status(NULL, conn_idx, FP_FW_NORMAL);
                break;
        case SUOTA_ERROR:
                fp_set_firmware_status(NULL, conn_idx, FP_FW_ABNORMAL);
                break;
        }

        /* In case SUOTA finished with an error, default connection parameters are restored. */
        if (status != SUOTA_ERROR) {
                return;
        }

        suota_ongoing = false;

        /* Restart advertising */
        fp_start_advertise();

        conn_param_update(conn_idx);

#if (dg_configBLE_2MBIT_PHY == 1)
        /* Switch to Auto PHY when SUOTA is completed */
        ble_gap_phy_set(conn_idx, BLE_GAP_PHY_PREF_AUTO, BLE_GAP_PHY_PREF_AUTO);
#endif /* (dg_configBLE_2MBIT_PHY == 1) */
}

/* SUOTA callbacks */
static const suota_callbacks_t suota_cb = {
        .suota_ready = suota_ready_cb,
        .suota_status = suota_status_changed_cb
};
#endif /* dg_configSUOTA_SUPPORT */

/* Pairing mode timeout timer callback */
static void pairing_mode_tim_cb(OS_TIMER timer)
{
        notify_fast_pair_task(PAIRING_MODE_TMO_NOTIF);
}

/* Stop pairing mode */
static void pairing_mode_stop(void)
{
        FP_TASK_PRINTF("\r\n>>>>Pairing mode stopped.<<<<\r\n");

        fp_set_pairing_mode(false);

        /* Stop pairing timer */
        OS_TIMER_STOP(pairing_mode_tim, OS_TIMER_FOREVER);

        /* Stop LED blinking */
        led_control_set_mode(LED_CONTROL_MODE_SET_OFF, 0, 0);
}

/* Start pairing mode */
void fast_pair_start_pairing_mode(void)
{
        if (fp_is_pairing_mode()) {
                /* Ignore pairing mode start */
                FP_TASK_PRINTF(">>>>Already in pairing mode.<<<<\r\n");
        } else {
                uint16_t led_blink_dur;

                if (fp_set_pairing_mode(true)) {
                        led_blink_dur = LED_BLINK_FOR_10S;

                        FP_TASK_PRINTF(">>>>Pairing mode started.<<<<\r\n");
                        OS_TIMER_START(pairing_mode_tim, OS_TIMER_FOREVER);
                } else {
                        led_blink_dur = LED_BLINK_FOR_3S;
                        FP_TASK_PRINTF(">>>>Pairing mode cannot be started.<<<<\r\n");
                }

                /* Start LED blinking */
                led_control_set_mode(LED_CONTROL_MODE_START_BLINK, LED_CONTROL_BLINK_RATE_DEFAULT,
                        led_blink_dur);
        }
}

#if (FP_FMDN == 1)
/* Control advertising */
void fast_pair_control_advertising(bool start)
{
        user_adv_stopped = !start;
        update_adv_status();

        FP_TASK_PRINTF(">>>>Advertising %s.<<<<\r\n", (user_adv_stopped) ? "stopped" : "started");
}
#endif /* FP_FMDN */

/* User button press (GPIO interrupt) callback */
static void user_button_interrupt_cb(void)
{
        bool press_timeout_tim_started = false;
        for (int i = 0; i < ARRAY_LENGTH(button_array); i++) {
                if (!press_timeout_tim_started) {
                        OS_TASK_NOTIFY_FROM_ISR(fast_pair_task_hdl, BUTTON_PRESS_NOTIF,
                                OS_NOTIFY_SET_BITS);
                        press_timeout_tim_started = true;
                }
        }
}

/* Initialize all user buttons */
static void user_button_init(void)
{
        hw_wkup_init(NULL);
        hw_wkup_set_key_debounce_time(32);
        hw_wkup_register_key_interrupt(user_button_interrupt_cb, 1);
        hw_wkup_enable_key_irq();

        for (int i = 0; i < ARRAY_LENGTH(button_array); i++) {
                bool pin_status_to_wait;

                hw_gpio_configure_pin(button_array[i].port, button_array[i].pin,
                        button_array[i].mode, HW_GPIO_FUNC_GPIO, false);
                hw_gpio_pad_latch_enable(button_array[i].port, button_array[i].pin);
                hw_gpio_pad_latch_disable(button_array[i].port, button_array[i].pin);

                /* User button mode must always be configured as input with a pull-up */
                OS_ASSERT(button_array[i].mode == HW_GPIO_MODE_INPUT_PULLUP ||
                          button_array[i].mode == HW_GPIO_MODE_INPUT_PULLDOWN);

                pin_status_to_wait =
                        (button_array[i].mode == HW_GPIO_MODE_INPUT_PULLUP) ? false : true;

                while (hw_gpio_get_pin_status(button_array[i].port, button_array[i].pin) ==
                        pin_status_to_wait);

                hw_wkup_set_trigger(button_array[i].port, button_array[i].pin,
                        (button_array[i].mode == HW_GPIO_MODE_INPUT_PULLUP) ?
                                HW_WKUP_TRIG_LEVEL_LO_DEB : HW_WKUP_TRIG_LEVEL_HI_DEB);
        }
}

/* Check if any of buttons is pressed */
static bool user_button_is_pressed(void)
{
        bool status = false;

        for (int i = 0; i < ARRAY_LENGTH(button_array); i++) {
                bool pin_status = hw_gpio_get_pin_status(button_array[i].port, button_array[i].pin);

                if (button_array[i].mode == HW_GPIO_MODE_INPUT_PULLUP) {
                        status = status || !pin_status;
                } else {
                        status = status || pin_status;
                }
        }

        return status;
}

/* Long press user button timeout timer callback */
static void user_long_press_tim_cb(OS_TIMER timer)
{
        notify_fast_pair_task(LONG_PRESS_TMO_NOTIF);
}

#if (FP_FMDN == 1)
/* Short press user button timeout timer callback */
static void user_short_press_tim_cb(OS_TIMER timer)
{
        notify_fast_pair_task(SHORT_PRESS_TMO_NOTIF);
}

/* Double press user button timeout timer callback */
static void user_double_press_tim_cb(OS_TIMER timer)
{
        notify_fast_pair_task(DOUBLE_PRESS_TMO_NOTIF);
}
#endif /* FP_FMDN */

/* Factory reset user button timeout timer callback */
static void factory_reset_press_tim_cb(OS_TIMER timer)
{
        notify_fast_pair_task(FACTORY_RESET_TMO_NOTIF);
}

#if (FP_FMDN == 1)
/* Advertise stop user button timeout timer callback */
static void adv_stop_press_tim_cb(OS_TIMER timer)
{
        notify_fast_pair_task(ADV_STOP_PRESS_TMO_NOTIF);
}
#endif /* FP_FMDN */

/* Google Fast Pair framework execution callback */
static void fast_pair_execution_cb(void)
{
        notify_fast_pair_task(GOOGLE_FAST_PAIR_NOTIF);
}

/* Google Fast Pair framework error callback */
static void fast_pair_error_cb(int error_code)
{
        volatile int err_code = error_code;
        (void) err_code;

        FP_TASK_PRINTF("Google Fast Pair error %d", error_code);
        OS_ASSERT(0);
}

#if (FP_LOCATOR_TAG != 1)
/* Pair status callback */
static void pair_status_cb(uint16_t conn_idx, uint8_t status)
{
        FP_TASK_PRINTF("Pairing for conn index %u %s\r\n",
                conn_idx, (status == BLE_STATUS_OK) ? "succeeded" : "failed");
}
#endif /* !FP_LOCATOR_TAG */

/* Pair request status callback */
static void pair_req_status_cb(uint16_t conn_idx, FP_PAIR_REQ_STAT stat, uint8_t err)
{
        if (err == 0) {
                if (stat == FP_PAIR_REQ_STAT_INITIATED) {
                        FP_TASK_PRINTF("Fast Pair request initiated for conn index %u\r\n", conn_idx);
                        if (fp_is_pairing_mode()) {
                                pairing_mode_stop();
                        }
                } else if (stat == FP_PAIR_REQ_STAT_COMPLETED) {
                        FP_TASK_PRINTF("Fast Pair request succeeded for conn index %u\r\n", conn_idx);
                }
        } else {
                FP_TASK_PRINTF("Fast Pair request failed for conn index %u with error code 0x%02X\r\n",
                        conn_idx, err);
        }
}

#if (FP_FMDN == 1)
/* FMDN provisioning status callback */
static void fmdn_prov_status_cb(FP_FMDN_PROV_STAT stat)
{
        switch (stat) {
        case FP_FMDN_PROV_STAT_STOPPED:
                OS_TIMER_STOP(beacon_time_storage_tim, OS_TIMER_FOREVER);
                FP_TASK_PRINTF("Device has stopped being FMDN provisioned\r\n");
                break;
        case FP_FMDN_PROV_STAT_STARTED:
                /*
                 * Trigger beacon time update in NVM storage and start timer to
                 * periodically update it
                 */
                beacon_time_storage_tim_cb(NULL);
                OS_TIMER_START(beacon_time_storage_tim, OS_TIMER_FOREVER);
                FP_TASK_PRINTF("Device has started being FMDN provisioned\r\n");
                break;
        }
}
#endif /* FP_FMDN */

/* Factory reset handler for button very long press */
static void factory_reset(void)
{
        /* Set LED on */
        led_control_set_mode(LED_CONTROL_MODE_SET_ON, 0, 0);

        OS_DELAY(OS_MS_2_TICKS(2000));
        fp_factory_reset();
        hw_cpm_reboot_system();
}

#if (FP_BATTERIES_COUNT != 0)
/* Update device battery status */
static void update_battery_status(void)
{
        /* Update battery status information for Google Fast Pair framework */
        fp_battery_info_t batt_info[FP_BATTERIES_COUNT] = {
                [0] = { .is_charging = false, .level = battery_monitor_get_level() }
        };
        fp_set_battery_information(batt_info);
}
#endif /* FP_BATTERIES_COUNT */

/* Update advertising status */
static void update_adv_status(void)
{
        if (user_adv_stopped) {
                adv_control_set_mode(ADV_CONTROL_MODE_DISABLED);
        } else {
                ADV_CONTROL_MODE mode = ADV_CONTROL_MODE_NORMAL;
#if (FP_BATTERIES_COUNT != 0)
#if (FP_FMDN == 1)
                /*
                 * Set advertising to low power mode if device is FMDN provisioned and
                 * the battery level is low
                 */
                if (fp_is_fmdn_provisioned() && batt_level_low) {
                        mode = ADV_CONTROL_MODE_LOW_POWER;
                }
#endif /* FP_FMDN */
#endif /* FP_BATTERIES_COUNT */
                adv_control_set_mode(mode);
        }
}

#if (FP_FMDN == 1)
/* Beacon time callback */
static uint32_t beacon_time_cb(void)
{
        uint64_t time = sys_timer_get_uptime_usec();
        time /= 1000;
        time /= 1000;
        return (uint32_t) time + beacon_time_offset;
}

/* Beacon time storage timer callback */
static void beacon_time_storage_tim_cb(OS_TIMER timer)
{
        notify_fast_pair_task(BEACON_TIME_TMO_NOTIF);
}

/* Initialize beacon time with the value stored in NVM storage */
static void init_beacon_time(void)
{
        uint16_t read_len = 0;

        app_params_t params[] = {
                { .param = APP_PARAMS_BEACON_TIME,
                  .data = &beacon_time_offset,
                  .len = sizeof(beacon_time_offset) }
        };
        read_len = app_params_get_params(params, ARRAY_LENGTH(params));

        if (read_len == 0) {
                beacon_time_offset = 0;
        }
}
/* Write beacon time to NVM storage */
static void write_beacon_time(void)
{
        uint16_t write_len = 0;
        uint32_t beacon_time = beacon_time_cb();

        app_params_t params[] = {
                { .param = APP_PARAMS_BEACON_TIME,
                  .data = &beacon_time,
                  .len = sizeof(beacon_time) }
        };
        write_len = app_params_set_params(params, ARRAY_LENGTH(params));

        if (write_len != sizeof(beacon_time)) {
                FP_TASK_PRINTF("Problem in writing beacon time to NVM storage\r\n");
                OS_ASSERT(0);
        }
}
#endif /* FP_FMDN */

/* Application and Google Fast Pair framework OS task */
OS_TASK_FUNCTION(fast_pair_task, params)
{
        int err = 0;

#if (dg_configUSE_WDOG == 1)
        int8_t wdog_id;
#endif
#if (dg_configSUOTA_ASYMMETRIC == 1)
        ble_service_t *asuota;
#endif
#if (dg_configSUOTA_SUPPORT == 1)
        ble_service_t *suota;
#endif
        char local_name_buf[MAX_NAME_LEN + 1];          /* 1 byte for '\0' character */
        uint16_t local_name_len;
#if (FP_FMDN == 1)
        uint8_t button_press_counter = 0;
#endif

        /* Save task handle to use it in fast_pair_execution_cb() */
        fast_pair_task_hdl = OS_GET_CURRENT_TASK();

#if (dg_configUSE_WDOG == 1)
        /* Register fast_pair_task to be monitored by watchdog */
        wdog_id = sys_watchdog_register(false);
#endif
#if (USE_CONSOLE == 1)
        /* Initialize UART console */
        console_uart_init(OS_GET_CURRENT_TASK());
#endif

#if (FP_FMDN == 1)
        /* Initialize beacon time with the value stored in NVM storage */
        init_beacon_time();
#endif

        /*
         * Initialize BLE
         */
        /* Start BLE device as peripheral */
        ble_peripheral_start();

        /* Register task to BLE framework to receive BLE event notifications */
        ble_register_app();

        /* Get device name from NVM storage if valid or use default otherwise */
        local_name_len = read_name(MAX_NAME_LEN, local_name_buf);

        /* Set device name */
        ble_gap_device_name_set(local_name_buf, ATT_PERM_READ);

        /*
         * Initialize timers
         */
        /* Create timer for pairing mode timeout */
        pairing_mode_tim = OS_TIMER_CREATE("pair", OS_MS_2_TICKS(PAIRING_MODE_TIMEOUT_MS),
                                OS_TIMER_ONCE, NULL, pairing_mode_tim_cb);

        /* Create timers for user button handling */
        user_long_press_tim = OS_TIMER_CREATE("longpress", OS_MS_2_TICKS(LONG_PRESS_TIMEOUT_MS),
                                OS_TIMER_ONCE, NULL, user_long_press_tim_cb);
#if (FP_FMDN == 1)
        user_short_press_tim = OS_TIMER_CREATE("shortpress", OS_MS_2_TICKS(SHORT_PRESS_TIMEOUT_MS),
                                OS_TIMER_ONCE, NULL, user_short_press_tim_cb);
        user_double_press_tim = OS_TIMER_CREATE("dpress", OS_MS_2_TICKS(DOUBLE_PRESS_TIMEOUT_MS),
                                OS_TIMER_ONCE, NULL, user_double_press_tim_cb);
#endif
        factory_reset_press_tim = OS_TIMER_CREATE("factoryrst", OS_MS_2_TICKS(VERY_LONG_PRESS_TIMEOUT_MS),
                                OS_TIMER_ONCE, NULL, factory_reset_press_tim_cb);
#if (FP_FMDN == 1)
        adv_stop_press_tim = OS_TIMER_CREATE("advstop", OS_MS_2_TICKS(ADV_STOP_PRESS_TIMEOUT_MS),
                                OS_TIMER_ONCE, NULL, adv_stop_press_tim_cb);
#endif

#if (FP_FMDN == 1)
        /* Create timer for periodic beacon time update in NVM storage */
        beacon_time_storage_tim = OS_TIMER_CREATE("btime", OS_MS_2_TICKS(BEACON_TIME_STORAGE_INTERVAL_MS),
                                OS_TIMER_RELOAD, NULL, beacon_time_storage_tim_cb);
#endif /* FP_FMDN */

        /* Initialize LED control */
        led_control_init(OS_GET_CURRENT_TASK());
#if (FP_BATTERIES_COUNT != 0)
        /* Initialize battery monitoring */
        battery_monitor_init(OS_GET_CURRENT_TASK());
#endif
        /* Initialize advertising control */
        adv_control_init(OS_GET_CURRENT_TASK());

#if (dg_configUSE_WDOG == 1)
        /* Suspend watchdog monitoring when initializing the buttons */
        sys_watchdog_suspend(wdog_id);
#endif
        /* Initialize user buttons */
        user_button_init();
#if (dg_configUSE_WDOG == 1)
        /* Resume watchdog monitoring */
        sys_watchdog_notify_and_resume(wdog_id);
#endif

#ifdef CONFIG_RETARGET
        FP_TASK_PRINTF(">>>>>Starting Fast Pair device<<<<<\r\n");
#endif /* CONFIG_RETARGET */

        /*
         * Initialize Google Fast Pair framework
         */
#if (FP_BATTERIES_COUNT != 0)
        /* Set battery information */
        const fp_battery_info_t batt_info[FP_BATTERIES_COUNT] = {
                [0] = { .is_charging = false, .level = battery_monitor_get_level() }
        };
#endif
        /* Set Google Fast Pair framework configuration */
        const fp_cfg_t cfg = {
                .execution_cb = fast_pair_execution_cb,
#if (FP_FMDN == 1)
                .beacon_time_cb = beacon_time_cb,
#endif
#if (FP_LOCATOR_TAG != 1)
                .pair_status_cb = pair_status_cb,
#endif
                .pair_req_status_cb = pair_req_status_cb,
#if (FP_FMDN == 1)
                .fmdn_prov_status_cb = fmdn_prov_status_cb,
#endif
                .error_cb = fast_pair_error_cb,
#if (FP_BATTERIES_COUNT != 0)
                .batt_info = batt_info,
#endif
                .start_adv = false
        };
        err = fp_init(&cfg);
        OS_ASSERT(err == 0);

        /* Set scan response object populated with <Complete Local Name> AD type */
        gap_adv_ad_struct_t *scan_rsp = GAP_ADV_AD_STRUCT_DECLARE(GAP_DATA_TYPE_LOCAL_NAME,
                                                local_name_len, local_name_buf);
        err = fp_set_scan_response(1, scan_rsp);
        OS_ASSERT(err == 0);

#if (dg_configSUOTA_ASYMMETRIC == 1)
        /* Add Asymmetric SUOTA Service */
        asuota = asym_suota_init(NULL, &asym_suota_cb);
        OS_ASSERT(asuota != NULL);
#endif

#if (dg_configSUOTA_SUPPORT == 1)
        /* Add SUOTA Service */
        suota = suota_init(&suota_cb);
        OS_ASSERT(suota != NULL);
#endif

#if (dg_configSUOTA_SUPPORT == 1) || (dg_configSUOTA_ASYMMETRIC == 1)
        /* Add advertise structure for SUOTA */
        gap_adv_ad_struct_t suota_adv_data = GAP_ADV_AD_STRUCT_BYTES(GAP_DATA_TYPE_UUID16_LIST_INC,
                                                BYTE_SEQ_LE_U16(dg_configBLE_UUID_SUOTA_SERVICE));
        err = fp_add_custom_advertise(1, &suota_adv_data);
        OS_ASSERT(err == 0);
#endif /* dg_configSUOTA_SUPPORT || dg_configSUOTA_ASYMMETRIC */

#if (FP_LOCATOR_TAG != 1)
        /* Set account key filter UI indication */
        fp_set_acc_key_filter_ui_indication(true);
#endif
#if (FP_BATTERY_NOTIFICATION == 1) && (FP_BATTERIES_COUNT != 0)
        /* Set battery UI indication */
        fp_set_battery_ui_indication(true);
#endif

#if (FP_FMDN == 1)
        /* Start timer if device is FMDN provisioned */
        if (fp_is_fmdn_provisioned()) {
                OS_TIMER_START(beacon_time_storage_tim, OS_TIMER_FOREVER);
        }
#endif /* FP_FMDN */

        /* Start advertising */
        update_adv_status();

        for (;;) {
                OS_BASE_TYPE ret __UNUSED;
                uint32_t notif;

#if (dg_configUSE_WDOG == 1)
                /* Notify watchdog on each loop */
                sys_watchdog_notify(wdog_id);

                /* Suspend watchdog while blocking on OS_TASK_NOTIFY_WAIT() */
                sys_watchdog_suspend(wdog_id);
#endif /* dg_configUSE_WDOG */

                /*
                 * Wait on any of the notification bits, then clear them all
                 */
                ret = OS_TASK_NOTIFY_WAIT(0, OS_TASK_NOTIFY_ALL_BITS, &notif, OS_TASK_NOTIFY_FOREVER);
                /* Blocks forever waiting for the task notification. Therefore, the return value must
                 * always be OS_OK
                 */
                OS_ASSERT(ret == OS_OK);

#if (dg_configUSE_WDOG == 1)
                /* Resume watchdog */
                sys_watchdog_notify_and_resume(wdog_id);
#endif

                /* Notified from BLE manager */
                if (notif & BLE_APP_NOTIFY_MASK) {
                        ble_evt_hdr_t *evt;

                        evt = ble_get_event(false);
                        if (!evt) {
                                goto no_event;
                        }
#if (ADV_CONTROL_MULT_EVENTS_ENABLE == 1)
                        /* First update advertising control module status */
                        adv_control_handle_event(evt);
#endif
                        /*
                         * First, the application needs to check if the event is handled by the
                         * ble_service and Google Fast Pair framework. If it is not handled,
                         * the application may handle it by defining a case for it in the
                         * `switch ()` statement below. If the event is not handled by the
                         * application either, it is handled by the default event handler.
                         */
                        if (!fp_handle_event(evt)) {
                                switch (evt->evt_code) {
                                case BLE_EVT_GAP_CONNECTED:
                                        handle_evt_gap_connected((ble_evt_gap_connected_t *) evt);
                                        break;
                                case BLE_EVT_GAP_DISCONNECTED:
                                        handle_evt_gap_disconnected((ble_evt_gap_disconnected_t *) evt);
                                        break;
#if (FP_LOCATOR_TAG != 1)
                                case BLE_EVT_GAP_PAIR_REQ:
                                        handle_evt_gap_pair_req((ble_evt_gap_pair_req_t *) evt);
                                        break;
                                case BLE_EVT_GAP_PAIR_COMPLETED:
                                        handle_evt_gap_pair_completed((ble_evt_gap_pair_completed_t *) evt);
                                        break;
#endif /* !FP_LOCATOR_TAG */
#if (dg_configSUOTA_SUPPORT == 1)
#if (dg_configBLE_2MBIT_PHY == 1)
                                case BLE_EVT_GAP_PHY_SET_COMPLETED:
                                        handle_ble_evt_gap_phy_set_completed((ble_evt_gap_phy_set_completed_t *) evt);
                                        break;
                                case BLE_EVT_GAP_PHY_CHANGED:
                                        handle_ble_evt_gap_phy_changed((ble_evt_gap_phy_changed_t *) evt);
                                        break;
#endif /* (dg_configBLE_2MBIT_PHY == 1) */
#endif /* (dg_configSUOTA_SUPPORT == 1) */
                                default:
                                        ble_handle_event_default(evt);
                                        break;
                                }
                        }

                        /* Free event buffer (it's not needed anymore) */
                        OS_FREE(evt);

no_event:
                        /*
                         * If there are more events waiting in queue, application should process
                         * them now.
                         */
                        if (ble_has_event()) {
                                notify_fast_pair_task(BLE_APP_NOTIFY_MASK);
                        }
                }

                /* Process any LED control specific notifications */
                led_control_process_notif(notif);

#if (FP_BATTERIES_COUNT != 0)
                /* Notified from battery level monitoring timer */
                if (notif & BATTERY_MONITOR_NOTIF) {
                        /* Process any battery monitoring specific notifications */
                        battery_monitor_process_notif(notif);

                        /* Update device battery status */
                        update_battery_status();
#if (FP_FMDN == 1)
                        /* Update device advertising status */
                        batt_level_low = (battery_monitor_get_level() <= LOW_POWER_MODE_BATTERY_LEVEL);
#endif
                        update_adv_status();
                }
#endif /* FP_BATTERIES_COUNT */

                /* Process any advertising control specific notifications */
                adv_control_process_notif(notif);

                /* Fast connection timer expired, try to set reduced power connection parameters */
                if (notif & UPDATE_CONN_PARAM_NOTIF) {
                        conn_dev_t *conn_dev = param_update_devices;

                        if (conn_dev && conn_dev->expired) {
                                param_update_devices = conn_dev->next;

#if (dg_configSUOTA_SUPPORT == 1)
                                /* Ignore this if SUOTA is ongoing */
                                if (!suota_ongoing) {
#endif
                                        conn_param_update(conn_dev->conn_idx);
#if (dg_configSUOTA_SUPPORT == 1)
                                }
#endif /* dg_configSUOTA_SUPPORT */

                                OS_TIMER_DELETE(conn_dev->param_tim, OS_TIMER_FOREVER);
                                conn_dev->param_tim = NULL;

                                /*
                                 * If the queue is not empty, reset bit and check if timer expired
                                 * next time.
                                 */
                                if (param_update_devices) {
                                        notify_fast_pair_task(UPDATE_CONN_PARAM_NOTIF);
                                }
                        }
                }

                /* Pairing mode timeout timer expired, stop pairing mode */
                if (notif & PAIRING_MODE_TMO_NOTIF) {
                        pairing_mode_stop();
                }

                /* Long press user button timeout timer expired, check button to start pairing mode */
                if (notif & LONG_PRESS_TMO_NOTIF) {
                        if (user_button_is_pressed()) {
                                fast_pair_start_pairing_mode();
                        }
                }
#if (FP_FMDN == 1)
                /* Short press user button timeout timer expired, check button to stop ringing or
                 * proceed with further operations based on button press counter. */
                if (notif & SHORT_PRESS_TMO_NOTIF) {
                        if (!user_button_is_pressed()) {
#if (FP_FMDN_RING_COMPONENTS_NUM > 0)
                                if (fp_is_ringing()) {
                                        fp_stop_ringing();
                                        /* button handled so skip further processing */
                                        OS_TIMER_STOP(adv_stop_press_tim, OS_TIMER_FOREVER);
                                } else
#endif
                                {
                                        if (!button_press_counter) {
                                                OS_TIMER_START(user_double_press_tim, OS_TIMER_FOREVER);
                                        }
                                        button_press_counter++;
                                }
                        }
                }

                /* Double press user button timeout timer expired, check button to enable user
                 * consent mode or indicate that a motion has been detected. */
                if (notif & DOUBLE_PRESS_TMO_NOTIF) {
                        switch (button_press_counter) {
                        case 1:
                                fp_set_user_consent(true);
                                if (led_control_get_mode() != LED_CONTROL_MODE_BLINK) {
                                        led_control_set_mode(LED_CONTROL_MODE_START_BLINK,
                                                LED_CONTROL_BLINK_RATE_DEFAULT, 2);
                                }
                                break;
                        case 2:
                                fp_motion_detector_ext_set_motion_detected();
                                break;
                        }
                        /* Short press combination detected, so skip button press timeout for advertise stop */
                        OS_TIMER_STOP(adv_stop_press_tim, OS_TIMER_FOREVER);
                        button_press_counter = 0;
                }
#endif /* FP_FMDN */
                /* Factory reset user button timeout timer expired, check button to perform factory reset */
                if (notif & FACTORY_RESET_TMO_NOTIF) {
                        if (user_button_is_pressed()) {
                                factory_reset();
                        }
                }

#if (FP_FMDN == 1)
                /* Advertise stop user button timeout timer expired, check button to stop advertise */
                if (notif & ADV_STOP_PRESS_TMO_NOTIF) {
                        if (!user_button_is_pressed()) {
                                fast_pair_control_advertising(user_adv_stopped);
                        }
                }

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

                /* User button pressed notification */
                if (notif & BUTTON_PRESS_NOTIF) {
                        OS_TIMER_START(user_long_press_tim, OS_TIMER_FOREVER);
#if (FP_FMDN == 1)
                        OS_TIMER_START(user_short_press_tim, OS_TIMER_FOREVER);
#endif
                        OS_TIMER_START(factory_reset_press_tim, OS_TIMER_FOREVER);
#if (FP_FMDN == 1)
                        OS_TIMER_START(adv_stop_press_tim, OS_TIMER_FOREVER);
#endif
                }

#if (USE_CONSOLE == 1)
                /* Process any serial console specific notifications */
                console_process_notif(notif);
#endif
        }
}
