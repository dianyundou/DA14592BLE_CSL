/**
 ****************************************************************************************
 *
 * @file arch_main.c
 *
 * @brief Initialization of BLE arhitecture.
 *
 * Copyright (C) 2017-2023 Renesas Electronics Corporation and/or its affiliates.
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

#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "co_version.h"
#include "ble_gap.h"   // GAP_TX_POWER_0_dBm
#include "ble_common.h"
#include "ble_config.h"
#include "sdk_defs.h"
#include "arch.h"
#include "cmac_mailbox.h"
#include "osal.h"
#include "core_cm33.h"
#include "hw_memctrl.h"
#include "ad_ble.h"
#include "hw_clk.h"
#include "hw_pd.h"
#include "hw_pdc.h"
#include "platform_nvparam.h"
#include "hw_pmu.h"

#define ODD_TO_NEXT_EVEN(x) ((x) & 0x01 ? x+1 : x)


void lib_ble_stack_init(void);
void lib_ble_stack_reset(uint8_t reset_type);
bool cmac_cpu_wakeup(void);
void sys_temp_meas_disable(void);

/*
 * Last POWER_CTRL_REG on-wakeup/on-sleep values applied for CMAC
 */
__RETAINED static struct {
        uint32_t onwakeup_value;
        uint32_t onsleep_value;
} power_ctrl_reg_values;

__RETAINED static struct {
        uint32_t onwakeup_value;
        uint32_t onsleep_value;
} power_level_reg_values;

__RETAINED uint8_t cmac_system_tcs_length;
__RETAINED uint8_t cmac_synth_tcs_length;
__RETAINED uint8_t cmac_rfcu_tcs_length;

__RETAINED_RW bool nvms_sleep_enable = true;

#if (dg_configRF_ENABLE_RECALIBRATION == 1)
extern bool ad_ble_temp_meas_enabled;
#endif

void ble_platform_initialization(void)
{

}

/*
 * This is the placeholder for CMAC code, data and mailbox sections.
 */
extern uint32_t __cmi_section_end__;

void cmac2sys_notify()
{
        ad_ble_task_notify(mainBIT_BLE_CMAC_IRQ);
}

#if ( dg_configSYSTEMVIEW == 1 )
void cmac2sys_isr_enter()
{
        SEGGER_SYSTEMVIEW_ISR_ENTER();
}

void cmac2sys_isr_exit()
{
        SEGGER_SYSTEMVIEW_ISR_EXIT();
}
#endif /* ( dg_configSYSTEMVIEW == 1 ) */


/**
 ****************************************************************************************
 * @brief Updates the POWER_CTRL_REG on-wakeup/on-sleep values to be applied by CMAC
          accordingly.
 ****************************************************************************************
 */
void cmac_update_power_ctrl_reg_values(uint32_t onsleep_value)
{
        GLOBAL_INT_DISABLE();

        power_level_reg_values.onwakeup_value = onsleep_value;
        power_level_reg_values.onsleep_value =  onsleep_value;
        power_ctrl_reg_values.onwakeup_value = CRG_TOP->POWER_CTRL_REG;
        power_ctrl_reg_values.onsleep_value = CRG_TOP->POWER_CTRL_REG;


        if (cmac_dynamic_config_table_ptr) {
                cmac_dynamic_config_table_ptr->power_ctrl_reg_onwakeup_value = power_ctrl_reg_values.onwakeup_value;
                cmac_dynamic_config_table_ptr->power_ctrl_reg_onsleep_value = power_ctrl_reg_values.onsleep_value;

                cmac_dynamic_config_table_ptr->power_level_reg_onwakeup_value = power_level_reg_values.onwakeup_value;
                cmac_dynamic_config_table_ptr->power_level_reg_onsleep_value = power_level_reg_values.onwakeup_value;

        }

        GLOBAL_INT_RESTORE();
}

/**
 ****************************************************************************************
 * @brief Retrieves the code and end base address of CMAC FW and sets up the memory
 *        controller accordingly.
 *
 * @Note This function gets called during execution of lib_ble_stack_init() and
 *       lib_ble_stack_reset() functions.
 ****************************************************************************************
 */
void cmac_mem_ctrl_setup(uint32_t *cmac_code_base_addr, uint32_t *cmac_end_base_addr)
{
    extern uint32_t cmi_fw_dst_addr;
    extern uint32_t __unused_ram_start__;

    *cmac_code_base_addr = (uint32_t) &cmi_fw_dst_addr;
    *cmac_end_base_addr = __unused_ram_start__ - 1;   // allow CMAC access all RAM so that it can access the heaps
}

/**
 * @brief Enables CMAC memory read only protection from M33
 *
 * This function should be called from libble after CMAC code is copied to RAM by M33.
 * After enabling CMAC memory protection M33 cannot change the contents of memory Cells RAM 9 and RAM 10
 */
void enable_cmac_mem_protection(void)
{
        hw_sys_enable_cmac_mem_protection();
}

/**
 * @brief Perfom initializations needed before CMAC starts
 *
 * This function is called during execution of lib_ble_stack_init() for operations that have to be
 * done after the firmware is downloaded but before the CMAC starts executing.
 */
void cmac_early_init(void)
{
        uint32_t jump_table_init(void);
        *cmac_rom_cfg_table_ptr = jump_table_init();
}

/**
 ****************************************************************************************
 * @brief Configures CMAC parameters.
 *
 * @Note This function gets called during execution of lib_ble_stack_init() and
 *       lib_ble_stack_reset() functions, at a point where CMAC is ready to start executing
 *       its main() function. CMAC execution will resume when this function returns.
 ****************************************************************************************
 */
void cmac_config_table_setup(void)
{
        cmac_configuration_table_t tmp_cfg_tbl;

        /* Initialize CMAC configuration table */
        cmac_config_table_ptr->ble_length_exchange_needed = dg_configBLE_DATA_LENGTH_REQ_UPON_CONN;
        cmac_config_table_ptr->ble_rx_buffer_size         = ODD_TO_NEXT_EVEN(dg_configBLE_DATA_LENGTH_RX_MAX + 11);
        cmac_config_table_ptr->ble_tx_buffer_size         = ODD_TO_NEXT_EVEN(dg_configBLE_DATA_LENGTH_TX_MAX + 11);
        cmac_config_table_ptr->initial_tx_power_lvl       = dg_configBLE_INITIAL_TX_POWER;

        if (dg_configDEFAULT_RADIO_OP_MODE == LOW_POWER_ALL_PHYS) {
                cmac_config_table_ptr->use_high_performance_1m  = false;
                cmac_config_table_ptr->use_high_performance_2m  = false;
        } else if (dg_configDEFAULT_RADIO_OP_MODE == HIGH_PERFORMANCE_ALL_PHYS) {
                cmac_config_table_ptr->use_high_performance_1m  = true;
                cmac_config_table_ptr->use_high_performance_2m  = true;
        } else {
                /* Invalid radio mode */
                ASSERT_ERROR(0);
        }

        cmac_config_table_ptr->ble_dup_filter_max         = dg_configBLE_DUPLICATE_FILTER_MAX;

        cmac_system_tcs_length = cmac_config_table_ptr->system_tcs_length;
        cmac_synth_tcs_length  = cmac_config_table_ptr->synth_tcs_length;
        cmac_rfcu_tcs_length   = cmac_config_table_ptr->rfcu_tcs_length;

        cmac_config_table_ptr->system_tcs_length = 0;
        cmac_config_table_ptr->synth_tcs_length  = 0;
        cmac_config_table_ptr->rfcu_tcs_length   = 0;

#if (dg_configUSE_SYS_TCS == 1)
        /* pass TCS settings to RF*/
        ad_ble_tcs_config();
#endif

        // LP clock type (frequency)
#if dg_configUSE_LP_CLK == LP_CLK_32768
        cmac_config_table_ptr->lp_clock_freq    = 0; //  32768Hz LP clock
#elif dg_configUSE_LP_CLK == LP_CLK_32000
        cmac_config_table_ptr->lp_clock_freq    = 1; //  32000Hz LP clock
#elif dg_configUSE_LP_CLK == LP_CLK_RCX
        cmac_config_table_ptr->lp_clock_freq    = 2; //  RCX

#if (USE_BLE_SLEEP == 1)
        // Sleep is enabled using RCX
        ad_ble_update_rcx();
        ASSERT_ERROR(cmac_dynamic_config_table_ptr->rcx_period);
        ASSERT_ERROR(cmac_dynamic_config_table_ptr->rcx_clock_hz_acc);
#endif /* (USE_BLE_SLEEP == 1) */

#else
        #error "The selected dg_configUSE_LP_CLK option is not supported by CMAC"
#endif

        /* Write already fetched public BD address to CMAC configuration table */
        ad_ble_get_public_address((uint8_t *) cmac_config_table_ptr->ble_bd_address);

        cmac_update_power_ctrl_reg_values(CRG_TOP->POWER_LEVEL_REG);
        /* Update POWER_CTRL_REG values */
        ASSERT_ERROR(power_ctrl_reg_values.onwakeup_value != 0
                && power_ctrl_reg_values.onsleep_value != 0);
        ASSERT_ERROR(power_level_reg_values.onwakeup_value != 0
                && power_level_reg_values.onsleep_value != 0);


        GLOBAL_INT_DISABLE();
#if dg_configRF_ENABLE_RECALIBRATION
        /* Write to shared variable the first reading of temp sens value */
        ad_ble_rf_calibration_info();
#else /* dg_configRF_ENABLE_RECALIBRATION */
        cmac_dynamic_config_table_ptr->gpadc_tempsens_val = 0;
#endif /* dg_configRF_ENABLE_RECALIBRATION */

        cmac_dynamic_config_table_ptr->power_ctrl_reg_onwakeup_value =
                power_ctrl_reg_values.onwakeup_value;
        cmac_dynamic_config_table_ptr->power_ctrl_reg_onsleep_value =
                power_ctrl_reg_values.onsleep_value;
        GLOBAL_INT_RESTORE();

        /*
         * Check NVPARAM for valid configuration values and write to proper CMAC configuration table
         */

        /* Static Configuration */
#if dg_configUSE_LP_CLK == LP_CLK_RCX
#if dg_configLP_CLK_DRIFT != 500
#error "500 PPM is the only valid option for dg_configLP_CLK_DRIFT when RCX is the low power clock."
#endif
        cmac_config_table_ptr->lp_clock_drift = dg_configLP_CLK_DRIFT;
#else
        if (ad_ble_read_nvms_param((uint8_t *) &tmp_cfg_tbl.lp_clock_drift, 2,
                                        NVPARAM_BLE_PLATFORM_LPCLK_DRIFT, NVPARAM_OFFSET_BLE_PLATFORM_LPCLK_DRIFT)) {
                cmac_config_table_ptr->lp_clock_drift = tmp_cfg_tbl.lp_clock_drift;
        } else {
                cmac_config_table_ptr->lp_clock_drift = dg_configLP_CLK_DRIFT;
        }
#endif

        if (ad_ble_read_nvms_param((uint8_t *) &tmp_cfg_tbl.ble_chnl_assess_timer, 2,
                                        NVPARAM_BLE_PLATFORM_BLE_CA_TIMER_DUR, NVPARAM_OFFSET_BLE_PLATFORM_BLE_CA_TIMER_DUR)) {
                cmac_config_table_ptr->ble_chnl_assess_timer = tmp_cfg_tbl.ble_chnl_assess_timer;
        }

        if (ad_ble_read_nvms_param((uint8_t *) &tmp_cfg_tbl.ble_chnl_reassess_timer, 1,
                                        NVPARAM_BLE_PLATFORM_BLE_CRA_TIMER_DUR, NVPARAM_OFFSET_BLE_PLATFORM_BLE_CRA_TIMER_DUR)) {
                cmac_config_table_ptr->ble_chnl_reassess_timer = tmp_cfg_tbl.ble_chnl_reassess_timer;
        }

        if (ad_ble_read_nvms_param((uint8_t *) &tmp_cfg_tbl.ble_chnl_assess_min_rssi, 1,
                                        NVPARAM_BLE_PLATFORM_BLE_CA_MIN_RSSI, NVPARAM_OFFSET_BLE_PLATFORM_BLE_CA_MIN_RSSI)) {
                cmac_config_table_ptr->ble_chnl_assess_min_rssi = tmp_cfg_tbl.ble_chnl_assess_min_rssi;
        }

        if (ad_ble_read_nvms_param((uint8_t *) &tmp_cfg_tbl.ble_chnl_assess_nb_pkt, 2,
                                        NVPARAM_BLE_PLATFORM_BLE_CA_NB_PKT, NVPARAM_OFFSET_BLE_PLATFORM_BLE_CA_NB_PKT)) {
                cmac_config_table_ptr->ble_chnl_assess_nb_pkt = tmp_cfg_tbl.ble_chnl_assess_nb_pkt;
        }
        if (ad_ble_read_nvms_param((uint8_t *) &tmp_cfg_tbl.ble_chnl_assess_nb_bad_pkt, 2,
                                        NVPARAM_BLE_PLATFORM_BLE_CA_NB_BAD_PKT, NVPARAM_OFFSET_BLE_PLATFORM_BLE_CA_NB_BAD_PKT)) {
                cmac_config_table_ptr->ble_chnl_assess_nb_bad_pkt = tmp_cfg_tbl.ble_chnl_assess_nb_bad_pkt;
        }

        /* Dynamic Configuration */
        if (ad_ble_read_nvms_param((uint8_t *) &nvms_sleep_enable, 1,
                                        NVPARAM_BLE_PLATFORM_SLEEP_ENABLE, NVPARAM_OFFSET_BLE_PLATFORM_SLEEP_ENABLE)) {
                /* nvms_sleep_enable will be taken into account by ble_stack_stay_active() */
        }


        enable_cmac_mem_protection();
}

/**
 ****************************************************************************************
 * @brief Initializes the BLE stack
 *
 * @Note cmac_mem_ctrl_setup() and cmac_config_table_setup() will get called while
 *       executing lib_ble_stack_init() to configure the memory controller and the
 *       CMAC parameters.
 ****************************************************************************************
 */
void ble_stack_init(void)
{
        /* Make sure that LP clock is enabled */
        //uint8_t lp_clk_sel = REG_GETF(CRG_TOP, CLK_CTRL_REG, LP_CLK_SEL);
        lp_clk_is_t lp_clk_sel =  hw_clk_get_lpclk();

        switch (lp_clk_sel)
        {
        case LP_CLK_IS_RCLP: /* RCLP */
                //ASSERT_WARNING(REG_GETF(CRG_TOP, CLK_RC32K_REG, RC32K_ENABLE));
                //ASSERT_WARNING(0);
                break;
        case LP_CLK_IS_RCX: /* RCX */
                ASSERT_WARNING(REG_GETF(CRG_TOP, CLK_RCX_REG, RCX_ENABLE));
                break;
        case LP_CLK_IS_XTAL32K: /* XTAL32K through the oscillator with an external Crystal. */
                ASSERT_WARNING(REG_GETF(CRG_TOP, CLK_XTAL32K_REG , XTAL32K_ENABLE));
                break;
        case LP_CLK_IS_EXTERNAL: /* XTAL32K through an external square wave generator */
                break;
        default:
                ASSERT_WARNING(0);
                break;
        }

        /* Initialize BLE stack (Controller and Host) */
        lib_ble_stack_init();
}

/**
 ****************************************************************************************
 * @brief Resets the BLE stack
 *
 * @Note Currently only BLE controller reset is performed.
 *
 * @Note cmac_mem_ctrl_setup() and cmac_config_table_setup() will get called while
 *       executing lib_ble_stack_reset() to configure the memory controller and the
 *       CMAC parameters.
 ****************************************************************************************
 */
void ble_stack_reset(void)
{
        bool sleep_enable = cmac_dynamic_config_table_ptr->sleep_enable;

#if (dg_configRF_ENABLE_RECALIBRATION == 1)
        /* Disable temperature monitoring for calibration if enabled */
        if (ad_ble_temp_meas_enabled) {
                sys_temp_meas_disable();
                ad_ble_temp_meas_enabled = false;
        }
#endif

        /* Reset the controller */
        lib_ble_stack_reset(0);

        /* Restore CMAC sleep enable value */
        cmac_dynamic_config_table_ptr->sleep_enable = sleep_enable;
}

/**
 ****************************************************************************************
 * @brief Runs the internal BLE stack scheduler, if needed
 *
 * param ulNotifiedValue Keeps track of any events signaled using ad_ble_task_notify()
 *
 * retval false if the BLE stack scheduler has pending actions, else true
 ****************************************************************************************
 */
bool ble_stack_schedule(uint32_t ulNotifiedValue)
{
        return true;
}

/**
 * @brief       Wakes the HW BLE block via an external request.
 *
 * @return      bool
 *
 * @retval      The status of the requested operation.
 *              <ul>
 *                  <li> false, if the BLE core is not sleeping
 *                  <li> true, if the BLE core was woken-up successfully
 *              </ul>
 */
bool ble_stack_force_wakeup(void)
{
#if (USE_BLE_SLEEP == 1)
        bool retval;
        retval = cmac_cpu_wakeup();
        return retval;
#else
        return false;
#endif /* (USE_BLE_SLEEP == 1) */
}

/**
 ****************************************************************************************
 * @brief Disables the sleep of the HW BLE block
 *
 * param status Defines if BLE sleep in allowed or not
 ****************************************************************************************
 */
void ble_stack_stay_active(bool status)
{
#if (USE_BLE_SLEEP == 1)
        if ((nvms_sleep_enable) && (status == false)) {
                cmac_dynamic_config_table_ptr->sleep_enable = true;
        } else {
                cmac_dynamic_config_table_ptr->sleep_enable = false;
        }
#endif /* (USE_BLE_SLEEP == 1) */
}

/**
 ****************************************************************************************
 * @brief BLE adapter uses this function to read data from the BLE stack
 ****************************************************************************************
 */
void ble_stack_read(uint8_t *bufPtr, uint32_t size, void (*callback)(uint8_t))
{
        cmac_mailbox_read(bufPtr, size, callback);
}

/**
 ****************************************************************************************
 * @brief BLE adapter uses this function to write a new message to the BLE stack
 ****************************************************************************************
 */
void ble_stack_write(uint8_t* msgPtr, uint32_t msgSize, void (*callback) (uint8_t))
{
        cmac_mailbox_write(msgPtr, msgSize, callback);
}

__WEAK void ble_controller_error(void)
{
        ASSERT_ERROR(0);
}

void sys_cmac_on_error_handler(void)
{
        ble_controller_error();
}

void sys_temp_meas_enable(void)
{
}

void sys_temp_meas_disable(void)
{
}

#endif /* CONFIG_USE_BLE */
