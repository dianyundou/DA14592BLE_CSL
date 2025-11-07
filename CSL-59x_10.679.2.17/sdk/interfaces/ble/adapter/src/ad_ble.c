/**
 ****************************************************************************************
 *
 * @file ad_ble.c
 *
 * @brief BLE OS Adapter
 *
 * Copyright (C) 2015-2024 Renesas Electronics Corporation and/or its affiliates.
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
#ifdef CONFIG_USE_BLE
#include <string.h>

#include "co_version.h"
#include "ble_config.h"

#include "osal.h"
#if (BLE_WINDOW_STATISTICS == 1) || (BLE_SLEEP_PERIOD_DEBUG == 1)
#include "logging.h"
#endif

#include "sys_clock_mgr.h"
#include "sys_power_mgr.h"
#include "sys_watchdog.h"

#include "hw_gpio.h"
#include "hw_clk.h"

#include "ad_nvms.h"

#include "ad_nvparam.h"
#include "platform_nvparam.h"

#include "ad_ble.h"
#include "ad_ble_msg.h"
#include "ble_config.h"
#include "ble_common.h"
#include "ble_mgr.h"
#include "ble_mgr_common.h"
#include "ble_mgr_ad_msg.h"
#include "ble_mgr_gtl.h"

#include "rwip_config.h"
#include "gapm_task.h"
#if (dg_configUSE_SYS_TCS == 1)
#include "sys_tcs.h"
#endif
#if (dg_configRF_ENABLE_RECALIBRATION == 1)
#include "../../sys_man/sys_adc_internal.h"
#endif


#include "rwip.h"
#include "gapc.h"


#if (BLE_ADAPTER_DEBUG == 1)
#pragma message "BLE Adapter: GPIO debugging is on!"
#endif

#if (dg_configNVMS_ADAPTER == 0)
#pragma message "NVMS Adapter is disabled. BLE device is going to use default BD_ADDR and IRK."
#endif

/*------------------------------------- Local definitions ----------------------------------------*/

/* Task stack size */
#define mainBLE_TASK_STACK_SIZE         (512)

/* Task priorities */
#define mainBLE_TASK_PRIORITY           ( OS_TASK_PRIORITY_HIGHEST - 3 )

/* BLE manager event group bits */
#define mainBIT_EVENT_QUEUE_TO_MGR      (1 << 1)


typedef enum {
        /* ble_stack_init() not yet called */
        STATE_UNINITIALIZED,
        /* ble_stack_init() has been called, GAPM_DEVICE_READY_IND is pending. */
        STATE_INITIALIZING,
        /* GAPM_DEVICE_READY_IND has been received */
        STATE_READY,
} ble_stack_state_t;

typedef enum {
        GTL_RX_STATE_IDLE,
        GTL_RX_STATE_START,
        GTL_RX_STATE_HDR,
        GTL_RX_STATE_PAYL,
        GTL_RX_STATE_OUT_OF_SYNC,
        GTL_RX_STATE_BLOCKED,
} gtl_rx_state_t;

typedef enum {
        /* Prevent sleep due to ad_ble_stay_active() API call */
        PREVENT_SLEEP_STAYACTIVE,
        /* Prevent sleep because LP clock is not yet available */
        PREVENT_SLEEP_LPCLOCK,
        /* Prevent sleep because GAPM_DEVICE_READY_IND has not been received */
        PREVENT_SLEEP_INITIALIZING
} prevent_sleep_t;

typedef struct ad_ble_gtl_env {
        void (*read) (uint8_t *bufptr, uint32_t size,  void (*callback)(uint8_t));
        void (*write)(uint8_t *bufptr, uint32_t size,  void (*callback)(uint8_t));
        gtl_rx_state_t rx_state;
        uint8_t rx_status;
        ble_stack_msg_type_t rx_msg_type;
        uint8_t rx_hdr_buff[8];
        ble_mgr_common_stack_msg_t* rx_msg;
} ad_ble_gtl_env_t;
/*------------------------------------- Local variables ------------------------------------------*/

__RETAINED static ble_stack_state_t ble_stack_state;

__RETAINED static ad_ble_gtl_env_t ad_ble_gtl_env; /**< BLE Adapter GTL environment */

__RETAINED static uint8_t ble_stack_prevent_sleep;

__RETAINED static bool stay_active; // Disabled by default

__RETAINED static ad_ble_op_code_t current_op;
__RETAINED static ad_ble_operation_t adapter_op;
__RETAINED static OS_MUTEX ad_ble_write_mutex;

__RETAINED static ad_ble_interface_t adapter_if;
__RETAINED static OS_TASK mgr_task;


__RETAINED static uint8_t public_address[BD_ADDR_LEN];

#if dg_configNVPARAM_ADAPTER
/* Global BLE NV-Parameter handle */
__RETAINED static nvparam_t ble_parameters;
#endif

extern __RETAINED uint8_t cmac_system_tcs_length;
extern __RETAINED uint8_t cmac_synth_tcs_length;
extern __RETAINED uint8_t cmac_rfcu_tcs_length;
#if (dg_configRF_ENABLE_RECALIBRATION == 1)
__RETAINED bool ad_ble_temp_meas_enabled;
__RETAINED static uint32_t rf_calibration_info;
#endif /* dg_configRF_ENABLE_RECALIBRATION == 1 */

/*--------------------------------------- Global variables ---------------------------------------*/


/*------------------------------------- Prototypes -----------------------------------------------*/

/* BLE stack's functions */
void ble_stack_init(void);
void ble_stack_reset(void);
bool ble_stack_schedule(uint32_t ulNotifiedValue);
void ble_stack_stay_active(bool status);
bool ble_stack_force_wakeup(void);
void ble_stack_read(uint8_t *bufPtr, uint32_t size, void (*callback)(uint8_t));
void ble_stack_write(uint8_t *bufPtr, uint32_t size, void (*callback)(uint8_t));
void ble_platform_initialization(void);

/**
 * \brief Sets a bit that prevents BLE stack from going to sleep
 */
static void ad_ble_prevent_sleep_set(uint8_t reason_bit);

/**
 * \brief Resets a bit that prevents BLE stack from going to sleep
 */
static void ad_ble_prevent_sleep_reset(uint8_t reason_bit);

/**
 * \brief Function called on the completion of a GTL transmission.
 */
static void ad_ble_write_cb_isr(uint8_t status);

/**
 * \brief Send a message to the BLE stack
 *
 * \param[in] ptr_msg  Pointer to the BLE stack message buffer.
 */
static void ad_ble_write(ble_mgr_common_stack_msg_t *ptr_msg);

/**
 * \brief Function called on the completion of a GTL reception.
 */
static void ble_stack_read_cb_isr(uint8_t status);

/**
 * \brief Function called on the completion of a GTL reception.
 */
static void ble_stack_read_cb(void);

/**
 * \brief Reads a message to the BLE stack
 *
 * \note When a message is received, ad_ble_handle_stack_evt() will get called.
 */
static void ad_ble_read(void);

/**
 * \brief Handle a BLE stack message.
 *
 * \param [in] msg Pointer to the message to be handled.
 */
static void ad_ble_handle_stack_msg(ble_mgr_common_stack_msg_t *msg);

/**
 * \brief Handle a BLE adapter configuration message.
 *
 * \param [in] msg Pointer to the message to be handled.
 */
static void ad_ble_handle_adapter_msg(ad_ble_msg_t *msg);

/**
 * \brief Handles received events from the stack
 *
 * \param [in] msg Pointer to the message to be handled.
 */
static void ad_ble_handle_stack_evt(ble_mgr_common_stack_msg_t* stack_msg);

/**
 * \brief Sends all pending commands to the BLE stack
 */
static void process_cmd_queue(void);

bool ke_mem_is_empty(uint8_t type);

/*--------------------------------------- Local functions  ---------------------------------------*/
static void ad_ble_state_set(ble_stack_state_t state)
{
        ble_stack_state = state;

        switch (ble_stack_state)
        {
        case STATE_UNINITIALIZED:
        {
                ad_ble_prevent_sleep_set(PREVENT_SLEEP_INITIALIZING);

                if (cm_lp_clk_is_avail() == true) {
                        ad_ble_prevent_sleep_reset(PREVENT_SLEEP_LPCLOCK);
                } else {
                        ad_ble_prevent_sleep_set(PREVENT_SLEEP_LPCLOCK);
                }
        }break;
        case STATE_INITIALIZING:
        {

        }break;
        case STATE_READY:
        {
                /* Apply TCS settings */
                ad_ble_sys_tcs_config();
#if (USE_BLE_SLEEP == 1)
                ad_ble_update_wakeup_time();
#endif /* (USE_BLE_SLEEP == 1) */

                ad_ble_prevent_sleep_reset(PREVENT_SLEEP_INITIALIZING);
        }break;
        default:
        {
                OS_ASSERT(0);
        }break;
        }

}

static ble_stack_state_t ad_ble_state_get(void)
{
        return ble_stack_state;
}

static void ad_ble_prevent_sleep_set(prevent_sleep_t reason_bit)
{
       ble_stack_prevent_sleep |= (1 << reason_bit);

#if (USE_BLE_SLEEP == 1)
       if (ad_ble_state_get() == STATE_READY) {
               if (ble_stack_prevent_sleep != 0) {
                       ble_stack_stay_active(true);
                       ble_stack_force_wakeup();
               }
       }
#endif /* (USE_BLE_SLEEP == 1) */
}

static void ad_ble_prevent_sleep_reset(prevent_sleep_t reason_bit)
{
       ble_stack_prevent_sleep &= ~(1 << reason_bit);

#if (USE_BLE_SLEEP == 1)
       if (ad_ble_state_get() == STATE_READY) {
               if (ble_stack_prevent_sleep == 0) {
                       ble_stack_stay_active(false);
                       ble_stack_force_wakeup();
               }
       }
#endif /* (USE_BLE_SLEEP == 1) */
}

static void ble_stack_read_cb_isr(uint8_t status)
{
        ad_ble_gtl_env.rx_status = status;

        OS_ASSERT(status == BLE_STACK_IO_OK);

        if (in_interrupt()) {
                OS_TASK_NOTIFY_FROM_ISR(adapter_if.task, mainBIT_BLE_STACK_READ_CB, OS_NOTIFY_SET_BITS);
        } else {
                OS_TASK_NOTIFY(adapter_if.task, mainBIT_BLE_STACK_READ_CB, OS_NOTIFY_SET_BITS);
        }
}

static void ble_stack_read_cb(void)
{
        __UNUSED uint8_t status = ad_ble_gtl_env.rx_status;

        // Since this function uses OS_MALLOC(), it should not be executed from ISR context
        OS_ASSERT(in_interrupt() == false);

        OS_ASSERT(status == BLE_STACK_IO_OK);

        if (status != BLE_STACK_IO_OK) {
                ad_ble_gtl_env.rx_state = GTL_RX_STATE_OUT_OF_SYNC;
        }

        // Check GTL state to see what was received
        switch (ad_ble_gtl_env.rx_state)
        {
                case GTL_RX_STATE_START:
                {
                     uint8_t hdr_len = 0;

                    // Check received packet indicator
                    if (ad_ble_gtl_env.rx_msg_type == HCI_ACL_MSG) {
                            hdr_len = HCI_ACL_HEADER_LENGTH;
                    } else if (ad_ble_gtl_env.rx_msg_type == HCI_EVT_MSG) {
                            hdr_len = HCI_EVT_HEADER_LENGTH;
                    } else if (ad_ble_gtl_env.rx_msg_type == GTL_MSG) {
                            hdr_len = GTL_MSG_HEADER_LENGTH;
                    } else {
                            ASSERT_ERR(0);
                    }

                    if (hdr_len != 0) {
                            ad_ble_gtl_env.rx_state = GTL_RX_STATE_HDR;
                            ad_ble_gtl_env.read(&ad_ble_gtl_env.rx_hdr_buff[0], hdr_len, ble_stack_read_cb_isr);
                    }
                }
                break;

                case GTL_RX_STATE_HDR:
                {
                     uint8_t hdr_len = 0;
                     uint16_t param_len = 0;

                     // Check received packet indicator
                     if (ad_ble_gtl_env.rx_msg_type == HCI_ACL_MSG) {
                             hdr_len = HCI_ACL_HEADER_LENGTH;
                             param_len = ad_ble_gtl_env.rx_hdr_buff[2] + (ad_ble_gtl_env.rx_hdr_buff[3] << 8);
                     } else if (ad_ble_gtl_env.rx_msg_type == HCI_EVT_MSG) {
                             hdr_len = HCI_EVT_HEADER_LENGTH;
                             param_len = ad_ble_gtl_env.rx_hdr_buff[1];
                     } else if (ad_ble_gtl_env.rx_msg_type == GTL_MSG) {
                             hdr_len = GTL_MSG_HEADER_LENGTH;
                             param_len = ad_ble_gtl_env.rx_hdr_buff[6] + (ad_ble_gtl_env.rx_hdr_buff[7] << 8);
                     } else {
                             ASSERT_ERR(0);
                     }

                    // Allocate the space needed for the message
                    ad_ble_gtl_env.rx_msg = OS_MALLOC(sizeof(ble_mgr_common_stack_msg_t) + param_len);

                    ad_ble_gtl_env.rx_msg->hdr.op_code = BLE_MGR_COMMON_STACK_MSG; // fill message OP code
                    ad_ble_gtl_env.rx_msg->msg_type = ad_ble_gtl_env.rx_msg_type; // fill stack message type
                    ad_ble_gtl_env.rx_msg->hdr.msg_len = hdr_len + param_len; // fill stack message length

                    uint8_t* buf = (uint8_t*) (&ad_ble_gtl_env.rx_msg->msg);

                    // Copy the header
                    memcpy (buf, &ad_ble_gtl_env.rx_hdr_buff[0], hdr_len);

                    if (param_len > 0) {
                            // Read the payload
                            ad_ble_gtl_env.rx_state = GTL_RX_STATE_PAYL;
                            ad_ble_gtl_env.read(&buf[hdr_len], param_len, ble_stack_read_cb_isr);
                    } else {
                            // Handle the event
                            ad_ble_handle_stack_evt(ad_ble_gtl_env.rx_msg);

                            // State is IDLE
                            ad_ble_gtl_env.rx_state = GTL_RX_STATE_IDLE;

                            // Prepare for the next event
                            ad_ble_read();
                    }
                }
                break;

                case GTL_RX_STATE_PAYL:
                {
                        // Handle the event
                        ad_ble_handle_stack_evt(ad_ble_gtl_env.rx_msg);

                        // State is IDLE
                        ad_ble_gtl_env.rx_state = GTL_RX_STATE_IDLE;

                        // Prepare for the next event
                        ad_ble_read();
                }
                break;

                default:
                {
                    ASSERT_ERR(0);
                }
                break;
        }
}

static void ad_ble_read(void)
{
        ASSERT_ERR((ad_ble_gtl_env.rx_state == GTL_RX_STATE_IDLE) ||
                        (ad_ble_gtl_env.rx_state == GTL_RX_STATE_BLOCKED));

        /* Check free space on BLE adapter's event queue. */
        if (OS_QUEUE_SPACES_AVAILABLE(adapter_if.evt_q)) {
                // BLE adapter's event queue is not full, prepare the next Rx
                ad_ble_gtl_env.rx_state = GTL_RX_STATE_START;
                ad_ble_gtl_env.read(&ad_ble_gtl_env.rx_msg_type, 1, ble_stack_read_cb_isr);
        } else {
                // BLE adapter's event queue is full, block until the queue becomes ready
                ad_ble_gtl_env.rx_state = GTL_RX_STATE_BLOCKED;

                /* Notify BLE manager that the adapter has blocked on a full
                 * event queue. BLE manager will notify the adapter when
                 * there is free space in the event queue. */
                ble_mgr_notify_adapter_blocked(true);
        }
}

static void ad_ble_write_cb_isr(uint8_t status)
{
        OS_ASSERT(status == BLE_STACK_IO_OK);

        if (in_interrupt()) {
                OS_EVENT_SIGNAL_FROM_ISR(ad_ble_write_mutex);
        } else {
                OS_EVENT_SIGNAL(ad_ble_write_mutex);
        }
}

static void ad_ble_write(ble_mgr_common_stack_msg_t *ptr_msg)
{
        ble_stack_msg_type_t msg_type = ptr_msg->msg_type;
        uint16_t msgSize = ptr_msg->hdr.msg_len + sizeof(uint8_t);
        uint8_t *msgPtr = (uint8_t *) &ptr_msg->msg;
        OS_BASE_TYPE xResult __UNUSED;

        /* msgPtr points to the message, which excludes the message type.
         * Since we want to send both the message type and the message
         * in a single step, we need to overwrite the initial ptr_msg */
        msgPtr--;
        *msgPtr = msg_type;
        ad_ble_gtl_env.write(msgPtr, msgSize, ad_ble_write_cb_isr);

        // Wait until message is transmitted
        xResult = OS_EVENT_WAIT(ad_ble_write_mutex, OS_EVENT_FOREVER);
        /* Guaranteed to succeed since we're waiting forever for the notification */
        OS_ASSERT(xResult == OS_OK);
}

/**
 * \brief Reset the BLE stack
 *
 * Create and send a GAPM_RESET_CMD to the BLE stack.
 */
static __UNUSED void ble_stack_reset_cmd(void)
{
        ble_mgr_common_stack_msg_t *msg;
        struct gapm_reset_cmd *cmd;

        msg = ble_gtl_alloc(GAPM_RESET_CMD, TASK_ID_GAPM, sizeof(struct gapm_reset_cmd));
        cmd = (struct gapm_reset_cmd *) msg->msg.gtl.param;

        /* Reset the software stack only */
        cmd->operation = GAPM_RESET;

        /* Send command to stack */
        ad_ble_write(msg);

        OS_FREE(msg);
}


static void read_public_address()
{
        uint8_t default_addr[BD_ADDR_LEN] = defaultBLE_STATIC_ADDRESS;
#if (dg_configUSE_SYS_TCS == 1)
        uint32_t *values;
        uint8_t size = 0;
        sys_tcs_get_custom_values(SYS_TCS_GROUP_BD_ADDR, &values, &size);

        if (size) {
                memcpy(public_address, values, BD_ADDR_LEN);
                return;
        }
#endif /* dg_configUSE_SYS_TCS == 1 */
        bool valid;

        valid = ad_ble_read_nvms_param(public_address, BD_ADDR_LEN, NVPARAM_BLE_PLATFORM_BD_ADDRESS,
                NVPARAM_OFFSET_BLE_PLATFORM_BD_ADDRESS);
        if (!valid) {
                memcpy(public_address, &default_addr, BD_ADDR_LEN);
        }
}

static void process_cmd_queue(void)
{
        ad_ble_hdr_t *received_msg;

        while (OS_QUEUE_MESSAGES_WAITING(adapter_if.cmd_q)) {
                /* The message may have already been read in the while () loop below! */
                if ( OS_QUEUE_GET(adapter_if.cmd_q, &received_msg, 0)) {
                        /* Make sure a valid OP CODE is received */
                        OS_ASSERT(received_msg->op_code < AD_BLE_OP_CODE_LAST);
                        current_op = received_msg->op_code;

                        if (current_op == AD_BLE_OP_CODE_STACK_MSG) {
                                /* Send message to BLE stack */
                                ble_mgr_common_stack_msg_t *stack_msg = (ble_mgr_common_stack_msg_t *) received_msg;
                                        ad_ble_handle_stack_msg(stack_msg);
                        }
                        else if (current_op == AD_BLE_OP_CODE_ADAPTER_MSG) {
                                ad_ble_handle_adapter_msg((ad_ble_msg_t *) received_msg);

                                /* Free previously allocated message buffer. */
                                OS_FREE(received_msg);
                        }
                }
        }
}

static void ad_ble_handle_stack_msg(ble_mgr_common_stack_msg_t *msg)
{
        __UNUSED__ bool send_msg = true;


        if (send_msg) {
                /* Send message to stack. */
                //ble_stack_write(msg_type, msgPtr + 1, msgSize - 1);
                ad_ble_write(msg);

                /* Free previously allocated message buffer. */
                OS_FREE(msg);
        }
}

static void ad_ble_handle_adapter_msg(ad_ble_msg_t *msg)
{
        adapter_op = msg->operation;

        switch (msg->operation) {
        // Only handle initialization command for now
        case AD_BLE_OP_INIT_CMD:
        {
                /* Initialize BLE stack */
                ble_stack_init();

                ad_ble_state_set(STATE_INITIALIZING);

                ad_ble_read();
                break;
        }
        case AD_BLE_OP_RESET_CMD:
        {
                /* Reset BLE stack */
                ble_stack_reset_cmd();
                break;
        }
        default:
                break;
        }
}

static void ad_ble_handle_stack_evt(ble_mgr_common_stack_msg_t* stack_msg)
{
        uint8_t* bufPtr = (uint8_t*)  &stack_msg->msg;

        /* Get msg id - bufPtr points to a packed message */
        uint16_t stack_msg_id = *( bufPtr + 0 ) + ( *( bufPtr + 1 ) << 8 );

        // Interception of calibration-related events
        if (stack_msg->msg.gtl.msg_id == GAPM_TEMP_MEAS_REQ_IND) {
#if (dg_configRF_ENABLE_RECALIBRATION == 1)
                struct gapm_temp_meas_req_ind *ind = (struct gapm_temp_meas_req_ind *) stack_msg->msg.gtl.param;
                if (ind->enable) {
                        ad_ble_task_notify(mainBIT_TEMP_MONITOR_ENABLE);
                } else {
                        ad_ble_task_notify(mainBIT_TEMP_MONITOR_DISABLE);
                }
#endif /* (dg_configRF_ENABLE_RECALIBRATION == 1) */
                /* Free the event */
                OS_FREE(stack_msg);

                return;
        } else if (stack_msg->msg.gtl.msg_id == GAPM_CMP_EVT) {
                struct gapm_cmp_evt *evt = (struct gapm_cmp_evt *) stack_msg->msg.gtl.param;

                if (evt->operation == GAPM_PERFORM_RF_CALIB) {
                        /* Free the event */
                        OS_FREE(stack_msg);

                        return;
                }
        }

#ifndef BLE_STACK_PASSTHROUGH_MODE
        if (current_op == AD_BLE_OP_CODE_ADAPTER_MSG)
        {
                switch (stack_msg_id) {
                case GAPM_DEVICE_READY_IND:
                {
                        /* BLE stack is ready to accept commands */
                        ad_ble_state_set(STATE_READY);

                        /* Send GAPM_RESET_CMD to properly initialize the stack */
                        ble_stack_reset_cmd();

                        /* Free the event */
                        OS_FREE(stack_msg);
                        break;
                }
                case GAPM_CMP_EVT:
                {
                        ad_ble_msg_t *ad_msg;
                        ad_ble_cmp_evt_t *ad_evt;

                        /* Make sure the reset was completed successfully */
                        OS_ASSERT( * ( bufPtr + 8 ) == GAPM_RESET );
                        OS_ASSERT( * ( bufPtr + 9 ) == GAP_ERR_NO_ERROR );

                        /* Create and send an AD_BLE_CMP_EVT */
                        ad_msg = ble_ad_msg_alloc(AD_BLE_OP_CMP_EVT, sizeof(ad_ble_cmp_evt_t));
                        ad_evt = (ad_ble_cmp_evt_t *) ad_msg->param;
                        ad_evt->op_req = adapter_op;
                        ad_evt->status = AD_BLE_STATUS_NO_ERROR;

                        ad_ble_event_queue_send(&ad_msg, OS_QUEUE_FOREVER);

                        /* Free the event */
                        OS_FREE(stack_msg);

                        break;
                }
                default:
                        break;
                }
        }
        else if (current_op == AD_BLE_OP_CODE_STACK_MSG)
#else
        if (stack_msg_id == GAPM_DEVICE_READY_IND) {
                /* BLE stack is ready to accept commands */
                ad_ble_state_set(STATE_READY);
        }
#endif /* BLE_STACK_PASSTHROUGH_MODE */
        {
                OS_BASE_TYPE status;

                status = ad_ble_event_queue_send(&stack_msg, 0);
                ASSERT_ERROR(status == OS_OK);
        }
}

/**
 * \brief Main BLE Interrupt and event queue handling task
 */
static OS_TASK_FUNCTION(ad_ble_task, pvParameters)
{
        uint32_t ulNotifiedValue;
        OS_BASE_TYPE xResult __UNUSED;
        int8_t wdog_id;

        ad_ble_state_set(STATE_UNINITIALIZED);

        ad_ble_gtl_env.read = ble_stack_read;
        ad_ble_gtl_env.write = ble_stack_write;

#ifdef BLE_STACK_PASSTHROUGH_MODE
        /* Initialize BLE stack */
        ble_stack_init();

        ad_ble_state_set(STATE_INITIALIZING);

        ad_ble_read();
#endif /* BLE_STACK_PASSTHROUGH_MODE */

        /* Register task to be monitored by watch dog. */
        wdog_id = sys_watchdog_register(false);

        DBG_SET_HIGH(BLE_ADAPTER_DEBUG, BLEBDG_ADAPTER); // Debug LED active (i.e. not sleeping)

        for (;;) {
                /* Notify watch dog on each loop since there's no other trigger for this. */
                sys_watchdog_notify(wdog_id);

                /* Suspend monitoring while task is blocked on OS_TASK_NOTIFY_WAIT(). */
                sys_watchdog_suspend(wdog_id);

                /*
                 * Wait on any of the event group bits, then clear them all.
                 */
                xResult = OS_TASK_NOTIFY_WAIT(OS_TASK_NOTIFY_NONE, OS_TASK_NOTIFY_ALL_BITS, &ulNotifiedValue,
                                                                            OS_TASK_NOTIFY_FOREVER);
                /* Guaranteed to succeed since we're waiting forever for the notification */
                OS_ASSERT(xResult == OS_OK);

                /* Resume watch dog monitoring. */
                sys_watchdog_notify_and_resume(wdog_id);

#if ((dg_configUSE_SYS_ADC == 1) && (dg_configRF_ENABLE_RECALIBRATION == 1))
                if (ulNotifiedValue & mainBIT_TEMP_MONITOR_ENABLE) {
                        /* Enable temperature monitoring */
                        ad_ble_temp_meas_enabled = true;
                        sys_adc_resume_rf_calibration();
                }

                if (ulNotifiedValue & mainBIT_TEMP_MONITOR_DISABLE) {
                        /* Disable temperature monitoring */
                        sys_adc_suspend_rf_calibration();
                        ad_ble_temp_meas_enabled = false;
                }
#endif /* ((dg_configUSE_SYS_ADC == 1) && (dg_configRF_ENABLE_RECALIBRATION == 1)) */

                /* Check whether any pending Rx has been completed */
                if (ulNotifiedValue & mainBIT_BLE_STACK_READ_CB) {
                        ble_stack_read_cb();
                }

                /* Check if Rx should be resumed */
                if (ulNotifiedValue & mainBIT_EVENT_QUEUE_AVAIL) {
                        // Continue Rx
                        ad_ble_read();

                        ble_mgr_notify_adapter_blocked(false);
                }


                if (ulNotifiedValue & mainBIT_COMMAND_QUEUE) {
                        process_cmd_queue();
                }

                if (ulNotifiedValue & mainBIT_EVENT_LPCLOCK_AVAIL) {
                        ad_ble_prevent_sleep_reset(PREVENT_SLEEP_LPCLOCK);
                }

                if (ulNotifiedValue & mainBIT_STAY_ACTIVE_UPDATED) {
                        if (stay_active) {
                                ad_ble_prevent_sleep_set(PREVENT_SLEEP_STAYACTIVE);
                        } else {
                                ad_ble_prevent_sleep_reset(PREVENT_SLEEP_STAYACTIVE);
                        }
                }

                /* Run BLE stack's internal scheduler */
                if (ad_ble_state_get() >= STATE_INITIALIZING) {
                        while (ble_stack_schedule(ulNotifiedValue) == false) {
                                /* Reset notification value */
                                ulNotifiedValue = 0;

                                /* Now is a good time to notify the watch dog. */
                                sys_watchdog_notify(wdog_id);
                        }
                }
        }
}

/*--------------------------------------- Global functions  ---------------------------------------*/
void ad_ble_notify(uint32_t ulNotifiedValue)
{
        OS_TASK_NOTIFY_FROM_ISR(adapter_if.task, ulNotifiedValue, OS_NOTIFY_SET_BITS);
}


void ad_ble_lpclock_available(void)
{
        if (adapter_if.task) {
               OS_TASK_NOTIFY(adapter_if.task, mainBIT_EVENT_LPCLOCK_AVAIL, OS_NOTIFY_SET_BITS);
        }
}

OS_BASE_TYPE ad_ble_command_queue_send( const void *item, OS_TICK_TIME wait_ticks)
{
        if (OS_QUEUE_PUT(adapter_if.cmd_q, item, wait_ticks) != OS_OK) {
                return OS_FAIL;
        }
        OS_TASK_NOTIFY(adapter_if.task, mainBIT_COMMAND_QUEUE, OS_NOTIFY_SET_BITS);

        return OS_OK;
}

OS_BASE_TYPE ad_ble_event_queue_send( const void *item, OS_TICK_TIME wait_ticks)
{
        if (OS_QUEUE_PUT(adapter_if.evt_q, item, wait_ticks) != OS_OK) {
                return OS_FAIL;
        }
        OS_TASK_NOTIFY(mgr_task, mainBIT_EVENT_QUEUE_TO_MGR, OS_NOTIFY_SET_BITS);

        return OS_OK;
}

void ad_ble_notify_event_queue_avail(void)
{
        OS_TASK_NOTIFY(adapter_if.task, mainBIT_EVENT_QUEUE_AVAIL, OS_NOTIFY_SET_BITS);
}

void ad_ble_task_notify(uint32_t value)
{
        if (in_interrupt()) {
                OS_TASK_NOTIFY_FROM_ISR(adapter_if.task, value, OS_NOTIFY_SET_BITS);
        } else {
                OS_TASK_NOTIFY(adapter_if.task, value, OS_NOTIFY_SET_BITS);
        }
}
bool ad_ble_non_retention_heap_in_use(void)
{
        if (ad_ble_state_get() == STATE_READY) {
                return !ke_mem_is_empty(KE_MEM_NON_RETENTION);
        } else {
                return false;
        }
}

bool ad_ble_read_nvms_param(uint8_t* param, uint8_t len, uint8_t nvparam_tag, uint32_t nvms_addr)
{
#if (dg_configNVMS_ADAPTER == 1)
#if (dg_configNVPARAM_ADAPTER == 1)
        uint16_t param_len;
        uint8_t valid;

        /* Parameter length shall be long enough to store address and validity flag */
        param_len = ad_nvparam_get_length(ble_parameters, nvparam_tag, NULL);
        if (param_len == len + sizeof(valid)) {
                ad_nvparam_read_offset(ble_parameters, nvparam_tag,
                                                len, sizeof(valid), &valid);

                /* Read param from nvparam only if validity flag is set to 0x00 and read_len is correct */
                if (valid == 0x00) {
                        uint16_t read_len = ad_nvparam_read(ble_parameters, nvparam_tag,
                                len, param);
                        if (read_len == len) {
                                return true; /* Success */
                        }
                }
        }
#else
        nvms_t nvms;
        int i;

        nvms = ad_nvms_open(NVMS_PARAM_PART);
        if (!nvms) {
                return false;
        }

        if (len != ad_nvms_read(nvms, nvms_addr, (uint8_t *) param, len)) {
                return false;
        }

        for (i = 0; i < len; i++) {
                if (param[i] != 0xFF) {
                        return true; /* Success */
                }
        }
#endif /* (dg_configNVPARAM_ADAPTER == 1) */
#endif /* (dg_configNVMS_ADAPTER == 1) */

        return false; /* Failure */
}

/**
 * \brief Initialization function of BLE adapter
 */
void ad_ble_init(void)
{
        // BLE ROM variables initialization
        ble_platform_initialization();

        OS_QUEUE_CREATE(adapter_if.cmd_q, sizeof(ble_mgr_common_stack_msg_t *), AD_BLE_COMMAND_QUEUE_LENGTH);
        OS_QUEUE_CREATE(adapter_if.evt_q, sizeof(ble_mgr_common_stack_msg_t *), AD_BLE_EVENT_QUEUE_LENGTH);
        OS_EVENT_CREATE(ad_ble_write_mutex);

        OS_ASSERT(adapter_if.cmd_q);
        OS_ASSERT(adapter_if.evt_q);

#if dg_configNVPARAM_ADAPTER
        /* Open BLE NV-Parameters - area name is defined in platform_nvparam.h */
        ble_parameters = ad_nvparam_open("ble_platform");
#endif

        // Create OS task
        OS_TASK_CREATE("bleA",                     // Text name assigned to the task
                       ad_ble_task,                // Function implementing the task
                       NULL,                       // No parameter passed
                       mainBLE_TASK_STACK_SIZE,    // Size of the stack to allocate to task
                       mainBLE_TASK_PRIORITY,      // Priority of the task
                       adapter_if.task);           // No task handle

        OS_ASSERT(adapter_if.task);

        DBG_CONFIGURE_LOW(BLE_ADAPTER_DEBUG, BLEBDG_ADAPTER); /* led (on: active, off: sleeping) */

        read_public_address();
}

const ad_ble_interface_t *ad_ble_get_interface()
{
        return &adapter_if;
}

OS_BASE_TYPE ad_ble_event_queue_register(const OS_TASK task_handle)
{
        // Set event queue task handle
        mgr_task = task_handle;

        return OS_OK;
}

void ad_ble_get_public_address(uint8_t address[BD_ADDR_LEN])
{
        memcpy(address, public_address, BD_ADDR_LEN);
}

void ad_ble_get_irk(uint8_t irk[KEY_LEN])
{
        uint8_t default_irk[KEY_LEN] = defaultBLE_IRK;

        bool valid;

        valid = ad_ble_read_nvms_param(irk, KEY_LEN, NVPARAM_BLE_PLATFORM_IRK,
                                                        NVPARAM_OFFSET_BLE_PLATFORM_IRK);
        if (!valid) {
                memcpy(irk, &default_irk, KEY_LEN);
        }
}

#if (dg_configNVPARAM_ADAPTER == 1)

nvparam_t ad_ble_get_nvparam_handle(void)
{
        return ble_parameters;
}
#if (dg_configPMU_ADAPTER == 1)
ADAPTER_INIT_DEP2(ad_ble_adapter, ad_ble_init, ad_pmu_adapter, ad_nvparam_adapter);
#else
ADAPTER_INIT_DEP1(ad_ble_adapter, ad_ble_init, ad_nvparam_adapter);
#endif /* dg_configPMU_ADAPTER */

#elif (dg_configNVMS_ADAPTER == 1)

#if (dg_configPMU_ADAPTER == 1)
ADAPTER_INIT_DEP2(ad_ble_adapter, ad_ble_init, ad_pmu_adapter, ad_nvms_adapter);
#else
ADAPTER_INIT_DEP1(ad_ble_adapter, ad_ble_init, ad_nvms_adapter);
#endif /* dg_configPMU_ADAPTER */

#else

#if (dg_configPMU_ADAPTER == 1)
ADAPTER_INIT_DEP1(ad_ble_adapter, ad_ble_init, ad_pmu_adapter);
#else
ADAPTER_INIT(ad_ble_adapter, ad_ble_init);
#endif /* dg_configPMU_ADAPTER */

#endif /* dg_configNVPARAM_ADAPTER */

void ad_ble_stay_active(bool status)
{
        stay_active = status;
        OS_TASK_NOTIFY(adapter_if.task, mainBIT_STAY_ACTIVE_UPDATED, OS_NOTIFY_SET_BITS);
}
#endif /* CONFIG_USE_BLE */

#if (USE_BLE_SLEEP == 1)
void ad_ble_update_wakeup_time(void)
{
        if (ad_ble_state_get() == STATE_READY) {
                uint16_t value, prev_value;
                value = pm_get_sys_wakeup_cycles();

                GLOBAL_INT_DISABLE();
                while (hw_sys_hw_bsr_try_lock(HW_BSR_MASTER_SYSCPU, HW_BSR_WAKEUP_CONFIG_POS) == false);
                prev_value = cmac_dynamic_config_table_ptr->wakeup_time;
#if (dg_configUSE_LP_CLK == LP_CLK_RCX)
                value += XTALRDY_CYCLES_TO_LP_CLK_CYCLES(hw_clk_get_xtalm_settling_time(), rcx_clock_hz);
#else
                value += XTALRDY_CYCLES_TO_LP_CLK_CYCLES(hw_clk_get_xtalm_settling_time(), dg_configXTAL32K_FREQ);
#endif
                if (value != prev_value) {
                        cmac_dynamic_config_table_ptr->wakeup_time = value;
                }
                hw_sys_hw_bsr_unlock(HW_BSR_MASTER_SYSCPU, HW_BSR_WAKEUP_CONFIG_POS);
                GLOBAL_INT_RESTORE();
                if (value  > prev_value) {
                        /*
                         * The wake-up time has been increased. Wake-up CMAC to re-calculate the sleep time.
                         */
                        ble_stack_force_wakeup();
                }
        }
}
#endif /* USE_BLE_SLEEP */

void ad_ble_sys_tcs_config(void)
{
        if (ad_ble_state_get() == STATE_READY) {
                uint8_t i;
                hw_sys_reg_config_t *source;
                hw_sys_reg_config_t *dest = (hw_sys_reg_config_t *)cmac_sys_tcs_table_ptr;
                uint32_t num_of_entries = *hw_sys_reg_get_num_of_config_entries();

                ASSERT_ERROR(num_of_entries <= cmac_system_tcs_length);

                cmac_config_table_ptr->system_tcs_length = 0;

                for (i = 0; i < num_of_entries && i < cmac_system_tcs_length; i++) {

                        source = hw_sys_reg_get_config(i);
                        dest[i].value = source->value;

                        /* Address must be written after value to prevent race condition */
                        dest[i].addr = source->addr;
                }
                cmac_config_table_ptr->system_tcs_length = num_of_entries;
        }
}

#if (dg_configUSE_SYS_TCS == 1)
void ad_ble_tcs_config(void)
{
        if (cmac_tcs_table_ptr != NULL) {
                cmac_tcs_table_ptr->tcs_attributes_size = MAX_SUPPORTED_TCS_GID;
                cmac_tcs_table_ptr->tcs_attributes_ptr = (uint32_t*) sys_tcs_get_tcs_attributes_ptr();

                cmac_tcs_table_ptr->tcs_data_size = sys_tcs_get_tcs_data_size();
                cmac_tcs_table_ptr->tcs_data_ptr = sys_tcs_get_tcs_data_ptr();
        }
}
#endif

#if (dg_configRF_ENABLE_RECALIBRATION == 1)
void ad_ble_rf_calibration_info(void)
{
        cmac_dynamic_config_table_ptr->gpadc_tempsens_val = rf_calibration_info;
}

void ad_ble_set_rf_calibration_info(const uint32_t value)
{
        rf_calibration_info = value;
        ad_ble_rf_calibration_info();
}
#endif

void ad_ble_get_lld_stats(struct ad_ble_lld_stats *stats)
{
        if (stats) {
                memset(stats, 0, sizeof(struct ad_ble_lld_stats));

                if (cmac_info_table_ptr) {
                        for (int conhdl = 0; conhdl < BLE_CONNECTION_MAX_USER; conhdl++) {
                                 uint16_t conidx = gapc_get_conidx(conhdl);
                                 if ((conidx != GAP_INVALID_CONIDX) && (conidx < BLE_CONNECTION_MAX_USER)) {
                                         stats->conn_evt_counter_non_apfm[conidx] = cmac_info_table_ptr->ble_conn_evt_counter_non_apfm[conhdl];
                                         stats->conn_evt_counter[conidx] = cmac_info_table_ptr->ble_conn_evt_counter[conhdl];
                                 }
                        }

                        stats->adv_evt_counter_non_apfm = cmac_info_table_ptr->ble_adv_evt_counter_non_apfm;
                        stats->adv_evt_counter = cmac_info_table_ptr->ble_adv_evt_counter;
                }
        }
}

#if (USE_BLE_SLEEP == 1)
#if (dg_configUSE_LP_CLK == LP_CLK_RCX)
void ad_ble_update_rcx(void)
{
        if (cmac_dynamic_config_table_ptr != NULL) {
                /* Put the new RCX values into the dynamic configuration table */
                cmac_dynamic_config_table_ptr->rcx_period = cm_get_rcx_clock_period();
                cmac_dynamic_config_table_ptr->rcx_clock_hz_acc = cm_get_rcx_clock_hz_acc();

                if (ad_ble_state_get() == STATE_READY) {
                        /*
                         * Wake up CMAC to pick up the new values in case:
                         *   - it is sleeping or
                         *   - it is on its way to enter sleep mode
                         */
                        ble_stack_force_wakeup();
                }
        }
}
#endif /* (USE_BLE_SLEEP == 1) */
#endif /* (dg_configUSE_LP_CLK == LP_CLK_RCX) */

