/**
 ****************************************************************************************
 *
 * @file accessory_task.c
 *
 * @brief Apple Find My Network (FMN) accessory application implementation
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

#include "accessory_config.h"
#include "afmn_config.h"

#include "hw_gpio.h"
#include "hw_wkup.h"
#include "hw_pdc.h"
#include "hw_cpm.h"

#include "ble_common.h"
#include "ble_gap.h"
#include "ble_gatts.h"
#include "tps.h"

#include "sys_power_mgr.h"
#include "sys_watchdog.h"

#if (dg_configSUOTA_ASYMMETRIC == 1)
#include "asym_suota_utils.h"
#include "asym_suota.h"
#endif

#if (dg_configSUOTA_SUPPORT == 1)
#include "dlg_suota.h"
#endif

#if (dg_configSUOTA_SUPPORT == 1) || (dg_configSUOTA_ASYMMETRIC == 1)
#include "dis.h"
#endif

#include "platform_devices.h"
#include "app_params.h"
#include "afmn_motion_detector_ext.h"
#include "led_control.h"
#include "battery_monitor.h"
#include "adv_control.h"
#include "notification_bits.h"

#include "afmn.h"
#include "afmn_os_port.h"

#if (dg_configSUOTA_SUPPORT == 1) || (dg_configSUOTA_ASYMMETRIC == 1)
#define SUOTA_AD_STRUCT_SIZE            (2 + 2)
#else
#define SUOTA_AD_STRUCT_SIZE            (0)
#endif

#define BYTE_SEQ_LE_U16(value)          (uint8_t)(value), (uint8_t)((value) >> 8)

#define MAX_NAME_LEN                    (BLE_SCAN_RSP_LEN_MAX - 2 - SUOTA_AD_STRUCT_SIZE)

#define LED_BLINK_FOR_10S               (10000 / (LED_CONTROL_BLINK_RATE_MS * 2))
#define LED_BLINK_FOR_3S                ( 3000 / (LED_CONTROL_BLINK_RATE_MS * 2))

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

#if (dg_configSUOTA_SUPPORT == 1) || (dg_configSUOTA_ASYMMETRIC == 1)
/* Device Information Service instance */
__RETAINED static ble_service_t *dis;
#endif
#if (dg_configSUOTA_ASYMMETRIC == 1)
/* Asymmetric SUOTA Service instance */
__RETAINED static ble_service_t *asuota;
#endif
#if (dg_configSUOTA_SUPPORT == 1)
/* SUOTA Service instance */
__RETAINED static ble_service_t *suota;
/* Store information about ongoing SUOTA. */
__RETAINED static bool suota_ongoing;
#endif /* dg_configSUOTA_SUPPORT */
/* Timer used to handle user button long press */
__RETAINED static OS_TIMER user_long_press_tim;
/* Timer used to handle user button short press */
__RETAINED static OS_TIMER user_short_press_tim;
/* Timer used to handle user button double press */
__RETAINED static OS_TIMER user_double_press_tim;
/* Timer used to handle factory reset user button press */
__RETAINED static OS_TIMER factory_reset_press_tim;
/* Timer used to handle serial number lookup */
__RETAINED static OS_TIMER serial_number_lookup_tim;
/* Apple Find My Network accessory OS task handle */
__RETAINED static OS_TASK accessory_task_hdl;

/* Connection status */
typedef struct {
        void *next;
        bool expired;
        uint16_t conn_idx;
        OS_TIMER param_tim;
#if (dg_configSUOTA_ASYMMETRIC == 1)
        uint8_t addr[BD_ADDR_LEN];
#endif
} conn_dev_t;

/* List of connected devices */
__RETAINED static void *conn_devices;
/* Pointer to next connected device waiting for connection parameters update */
__RETAINED static conn_dev_t *param_update_devices;

/* Notify accessory OS task */
static void notify_accessory_task(uint32_t notif)
{
        OS_TASK_NOTIFY(accessory_task_hdl, notif, OS_NOTIFY_SET_BITS);
}

/* User button press (GPIO interrupt) callback */
static void user_button_interrupt_cb(void)
{
        bool press_timeout_tim_started = false;
        for (int i = 0; i < ARRAY_LENGTH(button_array); i++) {
                if (!press_timeout_tim_started) {
                        OS_TASK_NOTIFY_FROM_ISR(accessory_task_hdl, BUTTON_PRESS_NOTIF,
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
        notify_accessory_task(LONG_PRESS_TMO_NOTIF);
}

/* Short press user button timeout timer callback */
static void user_short_press_tim_cb(OS_TIMER timer)
{
        notify_accessory_task(SHORT_PRESS_TMO_NOTIF);
}

/* Double press user button timeout timer callback */
static void user_double_press_tim_cb(OS_TIMER timer)
{
        notify_accessory_task(DOUBLE_PRESS_TMO_NOTIF);
}

/* Factory reset user button timeout timer callback */
static void factory_reset_press_tim_cb(OS_TIMER timer)
{
        notify_accessory_task(FACTORY_RESET_TMO_NOTIF);
}

/* Factory reset handler for button very long press */
static void factory_reset(void)
{
        /* Stop LED blinking and set it on */
        led_control_set_mode(LED_CONTROL_MODE_SET_ON, 0);

        OS_DELAY(OS_MS_2_TICKS(2000));
        afmn_factory_reset();
        hw_cpm_reboot_system();
}

/* Serial number lookup timeout timer callback */
static void serial_number_lookup_tim_cb(OS_TIMER timer)
{
        notify_accessory_task(SERIAL_NUMBER_TMO_NOTIF);
}

/* Buffer must have length at least max_len + 1 */
static uint16_t read_name(uint16_t max_len, char *name_buf)
{
        uint16_t read_len = 0;

        app_params_t params[] = {
                { .param = APP_PARAMS_BLE_APP_NAME, .data = name_buf, .len = max_len }
        };
        read_len = app_params_get_params(params, ARRAY_LENGTH(params));

        if (read_len == 0) {
                strcpy(name_buf, ACCESSORY_DEFAULT_NAME);
                return strlen(ACCESSORY_DEFAULT_NAME);
        }

        name_buf[read_len] = '\0';

        return read_len;
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
                notify_accessory_task(UPDATE_CONN_PARAM_NOTIF);
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

        cp.interval_min = BLE_CONN_INTERVAL_FROM_MS(15);     /* 15ms */
        cp.interval_max = BLE_CONN_INTERVAL_FROM_MS(60);     /* 60ms */
        cp.slave_latency = 0;
        cp.sup_timeout = BLE_SUPERVISION_TMO_FROM_MS(2000);  /* 2000ms */

        ble_gap_conn_param_update(conn_idx, &cp);

#if (dg_configBLE_2MBIT_PHY == 1)
        /* Switch to 2Mbit PHY during SUOTA */
        ble_gap_phy_set(conn_idx, BLE_GAP_PHY_PREF_2M, BLE_GAP_PHY_PREF_2M);
#endif
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
#if (dg_configSUOTA_ASYMMETRIC == 1)
                memcpy(conn_dev->addr, evt->own_addr.addr, sizeof(conn_dev->addr));
#endif
                conn_dev->param_tim = OS_TIMER_CREATE("conn_param", OS_MS_2_TICKS(20000),
                                        OS_TIMER_ONCE, (uint32_t) conn_dev, conn_params_tim_cb);
                list_append(&conn_devices, conn_dev);
                if (param_update_devices == NULL) {
                        param_update_devices = conn_dev;
                }
                OS_TIMER_START(conn_dev->param_tim, OS_TIMER_FOREVER);
                ble_gap_conn_tx_power_set(evt->conn_idx, TX_POWER_CONN);
        }

#if (dg_configSUOTA_ASYMMETRIC == 1)
        ble_gatts_service_changed_ind(evt->conn_idx, 0x0001, 0xFFFF);
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

#if (dg_configSUOTA_ASYMMETRIC == 1)
/* Callback indicating that a command has been received */
static void asym_suota_cmd_cb(ble_service_t *svc, uint16_t conn_idx, ASYM_SUOTA_CMD value)
{
        conn_dev_t *conn_dev;
        own_address_t addr;
        uint16_t write_len;

        conn_dev = list_find(conn_devices, conn_params_match,
                        (const void *) (uint32_t) conn_idx);

        /* Abort rebooting to SUOTA mode if connection is not active */
        if (conn_dev == NULL) {
                return;
        }

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

        /* Set address of the device when connection was established */
        app_params_t params[] = {
                { .param = APP_PARAMS_ASYM_SUOTA_BD_ADDR_TYPE,
                  .data = &addr.addr_type,
                  .len = sizeof(own_addr_type_t) },
                { .param = APP_PARAMS_ASYM_SUOTA_BD_ADDR_ADDRESS,
                  .data = conn_dev->addr,
                  .len = sizeof(conn_dev->addr) }
        };
        write_len = app_params_set_params(params, ARRAY_LENGTH(params));

        if (write_len != sizeof(own_addr_type_t) + BD_ADDR_LEN) {
                FMNA_TASK_PRINTF("Problem in writing BD address to NVM storage for Asymmetric SUOTA\r\n");
                OS_ASSERT(0);
        }

        OS_DELAY_MS(1000);

        asym_suota_utils_boot_suota_mode(true);
}

/* Callback indicating that a read ID request has been received */
static att_error_t asym_suota_id_cb(ble_service_t *svc, uint16_t conn_idx, uint8_t **value,
        uint16_t *length)
{
        /* Return NULL ID value and 0 length, indicating BD address matching reconnection */
        *value = NULL;
        *length = 0;

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
        FMNA_TASK_PRINTF("Preferred PHY settings %s set\r\n",
                (evt->status == BLE_STATUS_OK) ? "were successfully" : "failed to be");
}

/* Handler for BLE_EVT_GAP_PHY_CHANGED event */
static void handle_ble_evt_gap_phy_changed(ble_evt_gap_phy_changed_t *evt)
{
        FMNA_TASK_PRINTF("PHY changed to TX:%s, RX:%s\r\n",
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
        /* In case SUOTA finished with an error, default connection parameters are restored. */
        if (status != SUOTA_ERROR) {
                return;
        }

        suota_ongoing = false;

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

#if (dg_configSUOTA_SUPPORT == 1) || (dg_configSUOTA_ASYMMETRIC == 1)
/* Device Information Service data */
static const dis_device_info_t dis_info = {
        .manufacturer  = defaultBLE_DIS_MANUFACTURER,
        .model_number  = defaultBLE_DIS_MODEL_NUMBER,
        .serial_number = defaultBLE_DIS_SERIAL_NUMBER,
        .hw_revision   = defaultBLE_DIS_HW_REVISION,
        .fw_revision   = defaultBLE_DIS_FW_REVISION,
        .sw_revision   = defaultBLE_DIS_SW_REVISION
};
#endif /* dg_configSUOTA_SUPPORT || dg_configSUOTA_ASYMMETRIC */

/* Enable serial number lookup */
void accessory_enable_serial_number_lookup(void)
{
        afmn_set_serial_number_lookup(true);

        FMNA_TASK_PRINTF("Serial number lookup by BLE enabled\r\n");

        OS_TIMER_START(serial_number_lookup_tim, OS_TIMER_FOREVER);
        if (led_control_get_mode() != LED_CONTROL_MODE_BLINK) {
                led_control_set_mode(LED_CONTROL_MODE_START_BLINK, 2);
        }
}

/* Read paired state from NVM storage */
static bool read_paired_state(void)
{
        uint16_t read_len;
        uint8_t is_paired = 0;
        afmn_conn_params_t params[] = {
                { .param = AFMN_CONN_PARAMS_BLE_IS_PAIRED, .data = &is_paired, .len = 1 }
        };

        read_len = afmn_conn_params_get_params(params, ARRAY_LENGTH(params));
        if (read_len == 0) {
                is_paired = false;
        }

        return is_paired ? true : false;
}

/* Initialize BLE services */
static void initialize_ble_services(void)
{
#if (dg_configSUOTA_ASYMMETRIC == 1)
        /* Add Asymmetric SUOTA Service */
        asuota = asym_suota_init(NULL, &asym_suota_cb);
        OS_ASSERT(asuota != NULL);
#endif
#if (dg_configSUOTA_SUPPORT == 1)
        /* Add SUOTA Service */
        suota = suota_init(&suota_cb);
        OS_ASSERT(suota != NULL);
#endif /* dg_configSUOTA_SUPPORT */
#if (dg_configSUOTA_SUPPORT == 1) || (dg_configSUOTA_ASYMMETRIC == 1)
        /* Add Device Information Service */
        dis = dis_init(NULL, &dis_info);
        OS_ASSERT(dis != NULL);
#endif /* dg_configSUOTA_SUPPORT || dg_configSUOTA_ASYMMETRIC */
}

#ifdef CONFIG_RETARGET
static char *accessory_state_str(AFMN_ACCESSORY_STATE state)
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
static void accessory_afmn_execution_cb(void)
{
        notify_accessory_task(APPLE_FMN_NOTIF);
}

/* Apple FMN accessory state callback */
static void accessory_afmn_state_cb(AFMN_ACCESSORY_STATE state)
{
        FMNA_TASK_PRINTF(">>>>AFMN is in state: %s\r\n", accessory_state_str(state));
}

/* Internal attribute database reset callback */
static void accessory_afmn_db_reset_cb(void)
{
        initialize_ble_services();
}

/* Apple FMN error callback */
static void accessory_afmn_error_cb(AFMN_ERROR_LEVEL level, AFMN_ERROR_CATEGORY category, int code)
{
        volatile AFMN_ERROR_LEVEL err_level = level;
        volatile AFMN_ERROR_LEVEL err_category = category;
        volatile AFMN_ERROR_LEVEL err_code = code;

        (void) err_level;
        (void) err_category;
        (void) err_code;

        FMNA_TASK_PRINTF("ERROR: FindMy level = %d, category = %d, code = %d\r\n",
                level, category, code);
        OS_ASSERT(0);
}

/* Apple FMN pairing initiated callback */
static void accessory_afmn_pair_init_cb(void)
{
        FMNA_TASK_PRINTF("INFO: Pairing initiated\r\n");
}

/* Start pair mode */
void accessory_start_pair_mode(void)
{
        bool is_paired = read_paired_state();
        uint16_t led_blink_dur;

        if (is_paired) {
                /* Ignore; another pairing is not allowed */
                FMNA_TASK_PRINTF(">>>>Already paired.<<<<\r\n");
                return;
        }

        if (afmn_start_pair_mode()) {
                led_blink_dur = LED_BLINK_FOR_10S;
                FMNA_TASK_PRINTF(">>>>Pairing mode started.<<<<\r\n");
        } else {
                led_blink_dur = LED_BLINK_FOR_3S;
                FMNA_TASK_PRINTF(">>>>Already in pairing mode.<<<<\r\n");
        }

        /* Start LED blinking */
        led_control_set_mode(LED_CONTROL_MODE_START_BLINK, led_blink_dur);
}

/* Apple Find My Network accessory application task */
OS_TASK_FUNCTION(accessory_task, params)
{
#if (dg_configUSE_WDOG == 1)
        int8_t wdog_id;
#endif
        char local_name_buf[MAX_NAME_LEN + 1];          /* 1 byte for '\0' character */
        uint16_t local_name_len;
        uint8_t button_press_counter = 0;

        /* Save task handle to use it in accessory_afmn_execution_cb() */
        accessory_task_hdl = OS_GET_CURRENT_TASK();

#if (dg_configUSE_WDOG == 1)
        /* Register accessory_task to be monitored by watchdog */
        wdog_id = sys_watchdog_register(false);
#endif
#if (USE_CONSOLE == 1)
        /* Initialize UART console */
        console_uart_init(OS_GET_CURRENT_TASK());
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
        /* Create timer for serial number lookup timeout */
        serial_number_lookup_tim = OS_TIMER_CREATE("serial", OS_MS_2_TICKS(SERIAL_NUMBER_LOOKUP_TIMEOUT_MS),
                                OS_TIMER_ONCE, NULL, serial_number_lookup_tim_cb);

        /* Create timers for user button handling */
        user_long_press_tim = OS_TIMER_CREATE("longpress", OS_MS_2_TICKS(LONG_PRESS_TIMEOUT_MS),
                                OS_TIMER_ONCE, NULL, user_long_press_tim_cb);
        user_short_press_tim = OS_TIMER_CREATE("shortpress", OS_MS_2_TICKS(SHORT_PRESS_TIMEOUT_MS),
                                OS_TIMER_ONCE, NULL, user_short_press_tim_cb);
        user_double_press_tim = OS_TIMER_CREATE("dpress", OS_MS_2_TICKS(DOUBLE_PRESS_TIMEOUT_MS),
                                OS_TIMER_ONCE, NULL, user_double_press_tim_cb);
        factory_reset_press_tim = OS_TIMER_CREATE("factoryrst", OS_MS_2_TICKS(VERY_LONG_PRESS_TIMEOUT_MS),
                                OS_TIMER_ONCE, NULL, factory_reset_press_tim_cb);

        /* Initialize LED control */
        led_control_init(OS_GET_CURRENT_TASK());

        /* Initialize battery monitoring */
        battery_monitor_init(OS_GET_CURRENT_TASK());
#if (ADV_CONTROL_MULT_EVENTS_ENABLE == 1)
        /* Initialize advertising control */
        adv_control_init(OS_GET_CURRENT_TASK());
#endif

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
        FMNA_TASK_PRINTF(">>>>>Starting Find My Accessory<<<<<\r\n");
#endif /* CONFIG_RETARGET */

        /* Initialize Apple FMN framework */
        const afmn_config_t cfg = {
                .execution_cb = accessory_afmn_execution_cb,
                .state_cb = accessory_afmn_state_cb,
                .db_reset_cb = accessory_afmn_db_reset_cb,
                .error_cb = accessory_afmn_error_cb,
                .pair_init_cb = accessory_afmn_pair_init_cb
        };
        afmn_init(&cfg);

        /* Set scan response populated with <Complete Local Name> AD type */
        gap_adv_ad_struct_t scan_rsp_data[] = {
                GAP_ADV_AD_STRUCT(GAP_DATA_TYPE_LOCAL_NAME, local_name_len, local_name_buf),
#if (dg_configSUOTA_SUPPORT == 1) || (dg_configSUOTA_ASYMMETRIC == 1)
                GAP_ADV_AD_STRUCT_BYTES(GAP_DATA_TYPE_UUID16_LIST_INC,
                        BYTE_SEQ_LE_U16(dg_configBLE_UUID_SUOTA_SERVICE))
#endif
        };
        afmn_set_scan_response(ARRAY_LENGTH(scan_rsp_data), scan_rsp_data);

#ifdef CONFIG_RETARGET
        own_address_t addr;
        ble_gap_address_get(&addr);
        FMNA_TASK_PRINTF("Bluetooth device address: %02X:%02X:%02X:%02X:%02X:%02X\r\n",
                addr.addr[5], addr.addr[4], addr.addr[3],
                addr.addr[2], addr.addr[1], addr.addr[0]);
#endif /* CONFIG_RETARGET */

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
                         * ble_service and Apple FMN framework. If it is not handled,
                         * the application may handle it by defining a case for it in the
                         * `switch ()` statement below. If the event is not handled by the
                         * application either, it is handled by the default event handler.
                         */
                        if (!afmn_handle_event(evt)) {
                                switch (evt->evt_code) {
                                case BLE_EVT_GAP_CONNECTED:
                                        handle_evt_gap_connected((ble_evt_gap_connected_t *) evt);
                                        break;
                                case BLE_EVT_GAP_DISCONNECTED:
                                        handle_evt_gap_disconnected((ble_evt_gap_disconnected_t *) evt);
                                        break;
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
                                notify_accessory_task(BLE_APP_NOTIFY_MASK);
                        }
                }

                /* Process any LED control specific notifications */
                led_control_process_notif(notif);

                /* Process any battery monitoring specific notifications */
                battery_monitor_process_notif(notif);
#if (ADV_CONTROL_MULT_EVENTS_ENABLE == 1)
                /* Process any advertising control specific notifications */
                adv_control_process_notif(notif);
#endif

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
                                        notify_accessory_task(UPDATE_CONN_PARAM_NOTIF);
                                }
                        }
                }

                /* Long press user button timeout timer expired, check button to start pair mode */
                if (notif & LONG_PRESS_TMO_NOTIF) {
                        if (user_button_is_pressed()) {
                                accessory_start_pair_mode();
                        }
                }

                /* Short press user button timeout timer expired, proceed with further operations
                 * based on button press counter. */
                if (notif & SHORT_PRESS_TMO_NOTIF) {
                        if (!user_button_is_pressed()) {
                                if (!button_press_counter) {
                                        OS_TIMER_START(user_double_press_tim, OS_TIMER_FOREVER);
                                }
                                button_press_counter++;
                        }
                }

                /* Double press user button timeout timer expired, check button to enable serial
                 * number lookup or indicate that a motion has been detected. */
                if (notif & DOUBLE_PRESS_TMO_NOTIF) {
                        switch (button_press_counter) {
                        case 1:
                                accessory_enable_serial_number_lookup();
                                break;
                        case 2:
                                afmn_motion_detector_ext_set_motion_detected();
                                break;
                        }
                        button_press_counter = 0;
                }

                /* Factory reset user button timeout timer expired, check button to perform factory reset */
                if (notif & FACTORY_RESET_TMO_NOTIF) {
                        if (user_button_is_pressed()) {
                                factory_reset();
                        }
                }

                /* Apple FMN framework execution triggered */
                if (notif & APPLE_FMN_NOTIF) {
                        afmn_execution();
                }

                /* OS timer execution triggered */
                if (notif & AFMN_OS_TIMER_EXECUTION_NOTIF) {
                        afmn_os_timer_execution();
                }

                /* Serial number lookup timeout timer expired */
                if (notif & SERIAL_NUMBER_TMO_NOTIF) {
                        afmn_set_serial_number_lookup(false);
                }

                /* User button pressed notification */
                if (notif & BUTTON_PRESS_NOTIF) {
                        OS_TIMER_START(user_long_press_tim, OS_TIMER_FOREVER);
                        OS_TIMER_START(user_short_press_tim, OS_TIMER_FOREVER);
                        OS_TIMER_START(factory_reset_press_tim, OS_TIMER_FOREVER);
                }

#if (USE_CONSOLE == 1)
                /* Process any serial console specific notifications */
                console_process_notif(notif);
#endif
        }
}
