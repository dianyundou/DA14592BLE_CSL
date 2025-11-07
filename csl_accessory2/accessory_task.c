/**
 ****************************************************************************************
 *
 * @file accessory_task.c
 *
 * @brief Renesas CSL accessory application implementation
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
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "sdk_defs.h"
#include "osal.h"
#include "sdk_list.h"

#include "accessory_config.h"
#include "fast_pair_config.h"

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
#include "sys_timer.h"

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
#include "motion_detector.h"
#include "led_control.h"
#include "battery_monitor.h"
#include "adv_control.h"
#include "fn_control.h"
#include "notification_bits.h"

#include "fast_pair.h"

#if (dg_configSUOTA_SUPPORT == 1) || (dg_configSUOTA_ASYMMETRIC == 1)
#define SUOTA_AD_STRUCT_SIZE            (2 + 2)
#else
#define SUOTA_AD_STRUCT_SIZE            (0)
#endif

#define BYTE_SEQ_LE_U16(value)          (uint8_t)(value), (uint8_t)((value) >> 8)

#define MAX_NAME_LEN                    (BLE_SCAN_RSP_LEN_MAX - 2 - SUOTA_AD_STRUCT_SIZE)

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

#if (dg_configSUOTA_SUPPORT == 1) || (dg_configSUOTA_ASYMMETRIC == 1)
/* Device Information Service instance */
__RETAINED static ble_service_t *dis;
#endif
#if (dg_configSUOTA_ASYMMETRIC == 1)
/* Asymmetric SUOTA Service instance */
__RETAINED static ble_service_t *asuota;
/* Advertising data for Asymmetric SUOTA */
__RETAINED_RW static uint8_t asym_suota_adv_data[] = {
        /* SUOTA service UUID data */
        ASYM_SUOTA_UUID_DATA_LEN - 1, GAP_DATA_TYPE_UUID16_LIST_INC,
        BYTE_SEQ_LE_U16(dg_configBLE_UUID_SUOTA_SERVICE),
        /* Manufacturer specific data - ID value (8 bytes) for Asymmetric SUOTA */
        ASYM_SUOTA_MANUF_DATA_LEN - 1, GAP_DATA_TYPE_MANUFACTURER_SPEC,
        BYTE_SEQ_LE_U16(DLG_COMPANY_ID), 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};
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
/* Timer used to handle advertise stop user button press */
__RETAINED static OS_TIMER adv_stop_press_tim;
#if (FP_FMDN == 1)
#if (FP_BATTERIES_COUNT != 0)
/* Battery level is low */
__RETAINED static bool batt_level_low;
#endif /* FP_BATTERIES_COUNT */
#endif /* FP_FMDN */
/* Indication that user intentionally stopped advertising */
__RETAINED static bool user_adv_stopped;
/* Accessory OS task handle */
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

#if (FP_BATTERIES_COUNT != 0)
static void update_battery_status(void);
#endif
static void update_adv_status(void);

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

/* Advertise stop user button timeout timer callback */
static void adv_stop_press_tim_cb(OS_TIMER timer)
{
        notify_accessory_task(ADV_STOP_PRESS_TMO_NOTIF);
}

/* Buffer must have length at least max_len + 1 */
static uint16_t read_name(uint16_t max_len, char *name_buf)
{
        uint16_t read_len = 0;

        app_params_t params[] = {
                { .param = APP_PARAMS_BLE_APP_NAME, .data = name_buf, .len = max_len }
        };
        read_len = app_params_get_params(params, ARRAY_LENGTH(params), APP_PARAMS_TYPE_OTHER);

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

#if (FP_LOCATOR_TAG != 1)
/* Handler for BLE_EVT_GAP_PAIR_REQ event */
static void handle_evt_gap_pair_req(ble_evt_gap_pair_req_t *evt)
{
        CSLA_TASK_PRINTF("Pair request for conn index %u. Bond: %s\r\n",
                evt->conn_idx, evt->bond ? "true" : "false");

        /* Just accept the pairing request, set bond flag to what peer requested */
        ble_gap_pair_reply(evt->conn_idx, true, evt->bond);
}

/* Handler for BLE_EVT_GAP_PAIR_COMPLETED event */
static void handle_evt_gap_pair_completed(ble_evt_gap_pair_completed_t *evt)
{
        CSLA_TASK_PRINTF("Pair completed for conn index %u. Status: 0x%02X\r\n",
                evt->conn_idx, evt->status);
}
#endif /* !FP_LOCATOR_TAG */

#if (dg_configSUOTA_ASYMMETRIC == 1)
/* Callback indicating that a command has been received */
static void asym_suota_cmd_cb(ble_service_t *svc, uint16_t conn_idx, ASYM_SUOTA_CMD value)
{
        conn_dev_t *conn_dev;
        own_address_t addr;
        uint16_t write_len;
        FINDER_NETWORK_STATE fn_state;

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

        fn_state = fn_control_get_finder_network_state();

        if (fn_state == FINDER_NETWORK_AFMN) {
                /* Set address of the device when connection was established */
                app_params_t params[] = {
                        { .param = APP_PARAMS_ASYM_SUOTA_BD_ADDR_TYPE,
                          .data = &addr.addr_type,
                          .len = sizeof(own_addr_type_t) },
                        { .param = APP_PARAMS_ASYM_SUOTA_BD_ADDR_ADDRESS,
                          .data = conn_dev->addr,
                          .len = sizeof(conn_dev->addr) }
                };
                write_len = app_params_set_params(params, ARRAY_LENGTH(params), APP_PARAMS_TYPE_OTHER);

                if (write_len != sizeof(own_addr_type_t) + BD_ADDR_LEN) {
                        CSLA_TASK_PRINTF("Problem in writing BD address to NVM storage for Asymmetric SUOTA\r\n");
                        OS_ASSERT(0);
                }
        } else if (fn_state == FINDER_NETWORK_NONE || fn_state == FINDER_NETWORK_GFP_FMDN) {
                /* Set address type and random address renewal duration for Asymmetric SUOTA */
                app_params_t params[] = {
                        { .param = APP_PARAMS_ASYM_SUOTA_BD_ADDR_TYPE,
                          .data = &addr.addr_type,
                          .len = sizeof(own_addr_type_t) },
                        { .param = APP_PARAMS_ASYM_SUOTA_BD_ADDR_RENEW_DUR,
                          .data = (uint8_t[]) { BYTE_SEQ_LE_U16(1024) },  /* 1024 seconds renewal duration */
                          .len = sizeof(uint16_t) }
                };
                write_len = app_params_set_params(params, ARRAY_LENGTH(params), APP_PARAMS_TYPE_OTHER);

                if (write_len != sizeof(own_addr_type_t) + sizeof(uint16_t)) {
                        CSLA_TASK_PRINTF("Problem in writing address type and renewal duration to NVM storage"
                                         " for Asymmetric SUOTA\r\n");
                        OS_ASSERT(0);
                }
        }

        OS_DELAY_MS(1000);

        asym_suota_utils_boot_suota_mode(true);
}

/* Callback indicating that a read ID request has been received */
static att_error_t asym_suota_id_cb(ble_service_t *svc, uint16_t conn_idx, uint8_t **value,
        uint16_t *length)
{
        FINDER_NETWORK_STATE fn_state = fn_control_get_finder_network_state();

        if (fn_state == FINDER_NETWORK_AFMN) {
                /* Return NULL ID value and 0 length, indicating BD address matching reconnection */
                *value = NULL;
                *length = 0;
        } else if (fn_state == FINDER_NETWORK_NONE || fn_state == FINDER_NETWORK_GFP ||
                fn_state == FINDER_NETWORK_GFP_FMDN) {
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
                write_len = app_params_set_params(params, ARRAY_LENGTH(params), APP_PARAMS_TYPE_OTHER);

                if (write_len != sizeof(asym_suota_adv_data) + sizeof(uint8_t)) {
                        CSLA_TASK_PRINTF("Problem in writing advertising data and length to NVM "
                                         "storage for Asymmetric SUOTA\r\n");
                        OS_ASSERT(0);
                }

                /* Return ID value and length*/
                *value = asym_suota_manuf_data;
                *length = ASYM_SUOTA_MANUF_DATA_ID_LEN;
        }

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
        CSLA_TASK_PRINTF("Preferred PHY settings %s set\r\n",
                (evt->status == BLE_STATUS_OK) ? "were successfully" : "failed to be");
}

/* Handler for BLE_EVT_GAP_PHY_CHANGED event */
static void handle_ble_evt_gap_phy_changed(ble_evt_gap_phy_changed_t *evt)
{
        CSLA_TASK_PRINTF("PHY changed to TX:%s, RX:%s\r\n",
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

#if (FP_FMDN == 1)
        /* Authorize read access to DIS characteristics accordingly */
        if (fn_control_get_finder_network_state() == FINDER_NETWORK_GFP_FMDN) {
                dis_set_authorized_read(dis, BLE_CONN_IDX_INVALID, true);
        }
#endif /* FP_FMDN */
#endif /* dg_configSUOTA_SUPPORT || dg_configSUOTA_ASYMMETRIC */
}

#ifdef CONFIG_RETARGET
static char *accessory_state_str(FINDER_NETWORK_STATE state)
{
        switch (state) {
        case FINDER_NETWORK_NONE:
                return "UNPAIRED";
        case FINDER_NETWORK_AFMN_PAIRING:
                return "AFMN pairing";
        case FINDER_NETWORK_AFMN:
                return "AFMN provisioned";
        case FINDER_NETWORK_GFP_PAIRING:
                return "GFP pairing";
        case FINDER_NETWORK_GFP:
                return "GFP completed";
        case FINDER_NETWORK_GFP_FMDN:
                return "GFMDN provisioned";
        }

        return "";
}
#endif /* CONFIG_RETARGET */

/* Accessory state change callback */
static void accessory_state_cb(FINDER_NETWORK_STATE state)
{
        CSLA_TASK_PRINTF(">>>>Accessory is in state: %s\r\n", accessory_state_str(state));

        if (state == FINDER_NETWORK_NONE) {
#if (FP_BATTERIES_COUNT != 0)
                update_battery_status();
#endif
                update_adv_status();
#if (dg_configSUOTA_SUPPORT == 1) || (dg_configSUOTA_ASYMMETRIC == 1)
                /* Disable read access authorization for DIS characteristics */
                dis_set_authorized_read(dis, BLE_CONN_IDX_INVALID, false);
#endif
        } else if (state == FINDER_NETWORK_GFP_FMDN) {
#if (dg_configSUOTA_SUPPORT == 1) || (dg_configSUOTA_ASYMMETRIC == 1)
                /* Disable read access authorization for DIS characteristics */
                dis_set_authorized_read(dis, BLE_CONN_IDX_INVALID, true);
#endif
        }
}

/* Finder network pairing mode stopped callback */
static void accessory_pair_stop_cb(FINDER_NETWORK_STATE state)
{
        CSLA_TASK_PRINTF("\r\n>>>>Fast Pair pairing mode stopped.<<<<\r\n");

        /* Stop LED blinking */
        led_control_set_mode(LED_CONTROL_MODE_SET_OFF, 0, 0);
}

/* Internal attribute database reset callback */
static void accessory_db_reset_cb(void)
{
        initialize_ble_services();
}

/* Get battery level callback */
static uint8_t accessory_batt_level_get_cb(void)
{
#if (BATTERY_MONITOR_INTERVAL_MS > 0)
        return battery_monitor_get_level();
#else
        return 100;
#endif
}

/* Authenticated (for Google Fast Pair FMDN) connection indication callback */
static void accessory_fp_auth_conn_cb(uint16_t conn_idx)
{
#if (dg_configSUOTA_SUPPORT == 1) || (dg_configSUOTA_ASYMMETRIC == 1)
        /* Authorize read access to DIS characteristics for the connection */
        dis_set_authorized_read(dis, conn_idx, true);
#endif
}

/* Get current finder network */
FINDER_NETWORK_STATE accessory_get_finder_network_state(void)
{
        return fn_control_get_finder_network_state();
}

/* Start pairing mode */
void accessory_start_pairing_mode(void)
{
        FN_CONTROL_PAIRING_ERROR ret;
        uint16_t led_blink_dur = 0;

        ret = fn_control_set_pairing_mode(true);

        switch (ret) {
        case FN_CONTROL_PAIRING_ERROR_NONE:
                led_blink_dur = LED_BLINK_FOR_10S;
                CSLA_TASK_PRINTF(">>>>Pairing mode started.<<<<\r\n");
                break;
        case FN_CONTROL_PAIRING_ERROR_ALREADY_IN_PAIRING:
                led_blink_dur = LED_BLINK_FOR_3S;
                CSLA_TASK_PRINTF(">>>>Already in pairing mode.<<<<\r\n");
                break;
        case FN_CONTROL_PAIRING_ERROR_ALREADY_PAIRED:
                /* Ignore; another pairing is not allowed */
                CSLA_TASK_PRINTF(">>>>Already paired.<<<<\r\n");
                return;
        }

        /* Start LED blinking */
        led_control_set_mode(LED_CONTROL_MODE_START_BLINK, LED_CONTROL_BLINK_RATE_HIGH,
                led_blink_dur);
}

/* Enable user consent mode */
void accessory_enable_user_consent(void)
{
        fn_control_set_user_consent(true);

        CSLA_TASK_PRINTF("User consent mode enabled\r\n");

        if (led_control_get_mode() != LED_CONTROL_MODE_BLINK) {
                led_control_set_mode(LED_CONTROL_MODE_START_BLINK, LED_CONTROL_BLINK_RATE_HIGH, 2);
        }
}

/* Factory reset device */
void accessory_factory_reset(void)
{
        /* Stop LED blinking and set it on */
        led_control_set_mode(LED_CONTROL_MODE_SET_ON, 0, 0);

        CSLA_TASK_PRINTF("Factory reset\r\n");

        OS_DELAY(OS_MS_2_TICKS(2000));

        /* Perform factory reset for finder networks */
        fn_control_factory_reset();

        /* Reboot device */
        hw_cpm_reboot_system();
}

/* Control advertising */
void accessory_control_advertising(bool start)
{
        user_adv_stopped = !start;
        update_adv_status();

        CSLA_TASK_PRINTF(">>>>Advertising %s.<<<<\r\n", (user_adv_stopped) ? "stopped" : "started");
}

#if (FP_BATTERIES_COUNT != 0)
/* Update device battery status */
static void update_battery_status(void)
{
        if (fp_is_initialized()) {
                /* Update battery status information for Google Fast Pair framework */
                fp_battery_info_t batt_info[FP_BATTERIES_COUNT] = {
                        [0] = { .is_charging = false, .level = battery_monitor_get_level() }
                };
                fp_set_battery_information(batt_info);
        }
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
                if (fn_control_get_finder_network_state() == FINDER_NETWORK_GFP_FMDN && batt_level_low) {
                        mode = ADV_CONTROL_MODE_LOW_POWER;
                }
#endif /* FP_FMDN */
#endif /* FP_BATTERIES_COUNT */
                adv_control_set_mode(mode);
        }
}

/* Renesas CSL accessory application task */
OS_TASK_FUNCTION(accessory_task, params)
{
#if (dg_configUSE_WDOG == 1)
        int8_t wdog_id;
#endif
        char local_name_buf[MAX_NAME_LEN + 1];          /* 1 byte for '\0' character */
        uint16_t local_name_len;
        uint8_t button_press_counter = 0;

        /* Save task handle to use it in notifications */
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

        own_address_t addr;
#if (FP_LOCATOR_TAG == 1)
        addr.addr_type = PRIVATE_RANDOM_NONRESOLVABLE_ADDRESS;
#else
        addr.addr_type = PRIVATE_RANDOM_RESOLVABLE_ADDRESS;
#endif
        ble_gap_address_set(&addr, 1024);

        /*
         * Initialize timers
         */
        /* Create timers for user button handling */
        user_long_press_tim = OS_TIMER_CREATE("longpress", OS_MS_2_TICKS(LONG_PRESS_TIMEOUT_MS),
                                OS_TIMER_ONCE, NULL, user_long_press_tim_cb);
        user_short_press_tim = OS_TIMER_CREATE("shortpress", OS_MS_2_TICKS(SHORT_PRESS_TIMEOUT_MS),
                                OS_TIMER_ONCE, NULL, user_short_press_tim_cb);
        user_double_press_tim = OS_TIMER_CREATE("dpress", OS_MS_2_TICKS(DOUBLE_PRESS_TIMEOUT_MS),
                                OS_TIMER_ONCE, NULL, user_double_press_tim_cb);
        factory_reset_press_tim = OS_TIMER_CREATE("factoryrst", OS_MS_2_TICKS(VERY_LONG_PRESS_TIMEOUT_MS),
                                OS_TIMER_ONCE, NULL, factory_reset_press_tim_cb);
        adv_stop_press_tim = OS_TIMER_CREATE("advstop", OS_MS_2_TICKS(ADV_STOP_PRESS_TIMEOUT_MS),
                                OS_TIMER_ONCE, NULL, adv_stop_press_tim_cb);

        /* Initialize LED control */
        led_control_init(OS_GET_CURRENT_TASK());

#if (BATTERY_MONITOR_INTERVAL_MS > 0)
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
        CSLA_TASK_PRINTF(">>>>>Starting Renesas CSL Accessory<<<<<\r\n");
#endif /* CONFIG_RETARGET */

        /* Finder network control initialization */
        const fn_control_config_t fn_control_cfg = {
                .state_cb = accessory_state_cb,
                .pair_stop_cb = accessory_pair_stop_cb,
                .db_reset_cb = accessory_db_reset_cb,
                .batt_level_get_cb = accessory_batt_level_get_cb,
                .fp_auth_conn_cb = accessory_fp_auth_conn_cb
        };
        fn_control_init(&fn_control_cfg);

        /* Set scan response populated with <Complete Local Name> AD type */
        gap_adv_ad_struct_t scan_rsp_data[] = {
                GAP_ADV_AD_STRUCT(GAP_DATA_TYPE_LOCAL_NAME, local_name_len, local_name_buf),
#if (dg_configSUOTA_SUPPORT == 1) || (dg_configSUOTA_ASYMMETRIC == 1)
                GAP_ADV_AD_STRUCT_BYTES(GAP_DATA_TYPE_UUID16_LIST_INC,
                        BYTE_SEQ_LE_U16(dg_configBLE_UUID_SUOTA_SERVICE))
#endif
        };
        fn_control_set_scan_response(ARRAY_LENGTH(scan_rsp_data), scan_rsp_data);

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
                        bool evt_handled;

                        evt = ble_get_event(false);
                        if (!evt) {
                                goto no_event;
                        }

                        /* First update advertising control module status */
                        adv_control_handle_event(evt);

                        /*
                         * First, the application needs to check if the event is handled by the
                         * ble_service and Apple FMN or Google Fast Pair framework. If it is not handled,
                         * the application may handle it by defining a case for it in the
                         * `switch ()` statement below. If the event is not handled by the
                         * application either, it is handled by the default event handler.
                         */
                        evt_handled = fn_control_handle_event(evt);

                        if (!evt_handled) {
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
                                notify_accessory_task(BLE_APP_NOTIFY_MASK);
                        }
                }

                /* Process any LED control specific notifications */
                led_control_process_notif(notif);

#if (ACCESSORY_BATTERIES_COUNT != 0)
                /* Notified from battery level monitoring timer */
                if (notif & BATTERY_MONITOR_NOTIF) {
                        /* Process any battery monitoring specific notifications */
                        battery_monitor_process_notif(notif);
#if (FP_BATTERIES_COUNT != 0)
                        /* Update device battery status */
                        update_battery_status();
#if (FP_FMDN == 1)
                        /* Update device advertising status */
                        batt_level_low = (battery_monitor_get_level() <= LOW_POWER_MODE_BATTERY_LEVEL);
#endif
                        update_adv_status();
#endif /* FP_BATTERIES_COUNT */
                }
#endif /* ACCESSORY_BATTERIES_COUNT */

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
                                        notify_accessory_task(UPDATE_CONN_PARAM_NOTIF);
                                }
                        }
                }

                /* Long press user button timeout timer expired, check button to start pairing mode */
                if (notif & LONG_PRESS_TMO_NOTIF) {
                        if (user_button_is_pressed()) {
                                accessory_start_pairing_mode();
                        }
                }

                /* Short press user button timeout timer expired, proceed with further operations
                 * based on button press counter. */
                if (notif & SHORT_PRESS_TMO_NOTIF) {
                        if (!user_button_is_pressed()) {
                                if (!fn_control_stop_ringing()) {
                                        /* Button handled, so skip further processing */
                                        OS_TIMER_STOP(adv_stop_press_tim, OS_TIMER_FOREVER);
                                } else {
                                        if (!button_press_counter) {
                                                OS_TIMER_START(user_double_press_tim, OS_TIMER_FOREVER);
                                        }
                                        button_press_counter++;
                                }
                        }
                }

                /* Double press user button timeout timer expired, check button to enable user
                 * consent mode, serial number lookup or indicate that a motion has been detected. */
                if (notif & DOUBLE_PRESS_TMO_NOTIF) {
                        switch (button_press_counter) {
                        case 1:
                                accessory_enable_user_consent();
                                break;
                        case 2:
                                motion_detector_set_motion_detected();
                                break;
                        }
                        /* Short press combination detected, so skip button press timeout for advertise stop */
                        OS_TIMER_STOP(adv_stop_press_tim, OS_TIMER_FOREVER);
                        button_press_counter = 0;
                }

                /* Factory reset user button timeout timer expired, check button to perform factory reset */
                if (notif & FACTORY_RESET_TMO_NOTIF) {
                        if (user_button_is_pressed()) {
                                accessory_factory_reset();
                        }
                }

                /* Advertise stop user button timeout timer expired, check button to stop advertise */
                if (notif & ADV_STOP_PRESS_TMO_NOTIF) {
                        if (!user_button_is_pressed()) {
                                accessory_control_advertising(user_adv_stopped);
                        }
                }

                /* Process any finder network control specific notifications */
                fn_control_process_notif(notif);

                /* User button pressed notification */
                if (notif & BUTTON_PRESS_NOTIF) {
                        OS_TIMER_START(user_long_press_tim, OS_TIMER_FOREVER);
                        OS_TIMER_START(user_short_press_tim, OS_TIMER_FOREVER);
                        OS_TIMER_START(factory_reset_press_tim, OS_TIMER_FOREVER);
                        OS_TIMER_START(adv_stop_press_tim, OS_TIMER_FOREVER);
                }

#if (USE_CONSOLE == 1)
                /* Process any serial console specific notifications */
                console_process_notif(notif);
#endif
        }
}
