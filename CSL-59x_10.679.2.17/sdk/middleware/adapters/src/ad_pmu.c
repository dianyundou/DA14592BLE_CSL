/**
 ****************************************************************************************
 *
 * @file ad_pmu.c
 *
 * @brief PMU adapter API implementation
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


#if dg_configPMU_ADAPTER

#ifdef CONFIG_USE_BLE
# include "ble_common.h"
# if (dg_configDEFAULT_RADIO_OP_MODE == HIGH_PERFORMANCE_ALL_PHYS)
/* When using the HP mode there is no need to protect the RF from VDCDC */
#  undef PROTECT_RF_VDCDC
#  define PROTECT_RF_VDCDC            ( 0 )
# else
/* Protect the RF from a VDCDC change while in the middle of an RF frame */
#  ifndef PROTECT_RF_VDCDC
#   define PROTECT_RF_VDCDC           ( 1 )
#  endif /* PROTECT_RF_VDCDC */
# endif /* dg_configDEFAULT_RADIO_OP_MODE */
#else
/* Nothing to protect when BLE is not used */
# undef PROTECT_RF_VDCDC
# define PROTECT_RF_VDCDC            ( 0 )
#endif /* CONFIG_USE_BLE */

#include "ad_pmu.h"
#include "hw_bod.h"

#ifdef CONFIG_USE_BLE
#include "ble_common.h"
#include "ble_config.h"

extern void cmac_update_power_ctrl_reg_values(uint32_t onsleep_value);

# if (PROTECT_RF_VDCDC == 1)
#  include "da14590_config.h"
#  include "cmac_config_tables.h"
# endif /* PROTECT_RF_VDCDC */

#endif /* CONFIG_USE_BLE */

#ifdef OS_PRESENT
#include "sys_power_mgr.h"
#endif


__RETAINED static uint8_t ad_pmu_1v4_vdcdc_acquire_count;
__RETAINED static uint8_t ad_pmu_1v2_vdd_acquire_count;

/*
 * Use initial values that also make sense for baremetal apps,
 * where ad_pmu_init() cannot be called.
 */
__RETAINED_RW static ad_pmu_rail_config_t ad_pmu_vdd_rail_config = {
        .enabled_onwakeup = true,
        .enabled_onsleep = true,
        .rail_vdd = {
                .current_onwakeup = HW_PMU_VDD_MAX_LOAD_20,
                .current_onsleep = HW_PMU_VDD_MAX_LOAD_0_400,
                .voltage_onwakeup = HW_PMU_VDD_VOLTAGE_0V90,
                .voltage_onsleep = HW_PMU_VDD_VOLTAGE_SLEEP_0V75
        }
};

__RETAINED_RW static ad_pmu_rail_config_t ad_pmu_vdcdc_rail_config = {
        .enabled_onwakeup = true,
        .enabled_onsleep = true,
        .rail_vdcdc = {
                .current_onwakeup = HW_PMU_VDCDC_MAX_LDO_LOAD_40,
                .current_onsleep = HW_PMU_VDCDC_MAX_LDO_LOAD_1,
                .voltage_onwakeup = HW_PMU_VDCDC_VOLTAGE_1V10
        }
};

/* Forward declarations */
#ifdef OS_PRESENT

#define AD_PMU_MUTEX_CREATE()   do {                                            \
                                        OS_ASSERT(ad_pmu_mutex == NULL);        \
                                        OS_MUTEX_CREATE(ad_pmu_mutex);          \
                                        OS_ASSERT(ad_pmu_mutex);                \
                                } while (0)
#define AD_PMU_MUTEX_GET()      do {                                                    \
                                        OS_ASSERT(ad_pmu_mutex);                        \
                                        OS_MUTEX_GET(ad_pmu_mutex, OS_MUTEX_FOREVER);   \
                                } while (0)
#define AD_PMU_MUTEX_PUT()      OS_MUTEX_PUT(ad_pmu_mutex)

static void ad_pmu_init(void);
#endif
static void configure_power_rail(AD_PMU_RAIL rail, const ad_pmu_rail_config_t *cfg);
static void ad_pmu_acquire_1v4_vdcdc_voltage(void);
static void ad_pmu_release_1v4_vdcdc_voltage(void);
static void ad_pmu_acquire_1v2_vdd_voltage(void);
static void ad_pmu_release_1v2_vdd_voltage(void);

#ifdef OS_PRESENT

__RETAINED static OS_MUTEX ad_pmu_mutex;

/**
 * \brief Initialize adapter
 *
 */
static void ad_pmu_init(void)
{
        ad_pmu_rail_config_t ad_pmu_rail_config;

        /* Create Mutex */
        AD_PMU_MUTEX_CREATE();

#if (dg_configUSE_BOD == 1)
        /* Deactivate BOD to configure the rails. */
        hw_bod_deactivate();
#endif
        /*
         * 1V8 rail configuration
         */
        if (dg_configPOWER_1V8_ACTIVE != 2) {
                ad_pmu_rail_config.enabled_onwakeup  = (dg_configPOWER_1V8_ACTIVE == 1);
        } else {
                ad_pmu_rail_config.enabled_onwakeup = true;  /* Default value */
        }

        if (dg_configPOWER_1V8_SLEEP != 2) {
                ad_pmu_rail_config.enabled_onsleep = (dg_configPOWER_1V8_SLEEP == 1);
        } else {
                ad_pmu_rail_config.enabled_onsleep = true; /* Default value */
        }

        ad_pmu_rail_config.rail_1v8.current_onwakeup = HW_PMU_1V8_MAX_LDO_LOAD_20;
        ad_pmu_rail_config.rail_1v8.current_onsleep = HW_PMU_1V8_MAX_LDO_LOAD_2;

        configure_power_rail(PMU_RAIL_1V8, &ad_pmu_rail_config);

       /*
        * VDCDC rail configuration
        */

        ad_pmu_rail_config.enabled_onwakeup = true;
        ad_pmu_rail_config.enabled_onsleep = true;
        ad_pmu_rail_config.rail_vdcdc.current_onwakeup = (dg_configUSE_DCDC == 1) ?
                                                          HW_PMU_VDCDC_MAX_DCDC_LOAD_40 :
                                                          HW_PMU_VDCDC_MAX_LDO_LOAD_40;
        ad_pmu_rail_config.rail_vdcdc.current_onsleep = (dg_configUSE_DCDC == 1) ?
                                                         HW_PMU_VDCDC_MAX_DCDC_LOAD_0_300 :
                                                         HW_PMU_VDCDC_MAX_LDO_LOAD_1;

        ad_pmu_rail_config.rail_vdcdc.voltage_onwakeup =
#ifdef CONFIG_USE_BLE
                                                         (
                                                                 (dg_configDEFAULT_RADIO_OP_MODE == HIGH_PERFORMANCE_ALL_PHYS) ||
#endif
                                                                 (hw_clk_get_sysclk() == SYS_CLK_IS_DBLR)
#ifdef CONFIG_USE_BLE
                                                         )
#endif
                                                          ?
                                                          HW_PMU_VDCDC_VOLTAGE_1V40 :
                                                          HW_PMU_VDCDC_VOLTAGE_1V10;

        configure_power_rail(PMU_RAIL_VDCDC, &ad_pmu_rail_config);


        /*
         * VDD rail configuration
         */

#if (dg_configUSE_BOD == 1)
        uint16_t vdd_bod_voltage_level = (hw_clk_get_sysclk() == SYS_CLK_IS_DBLR) ?
                                         HW_BOD_VDD_LEVEL_ACTIVE_1V05 : HW_BOD_VDD_LEVEL_ACTIVE_0V78;
        hw_bod_set_channel_voltage_level(HW_BOD_CHANNEL_VDD, vdd_bod_voltage_level);
#endif

        ad_pmu_rail_config.enabled_onwakeup = true;
        ad_pmu_rail_config.enabled_onsleep = true;
        ad_pmu_rail_config.rail_vdd.current_onwakeup = HW_PMU_VDD_MAX_LOAD_20;
        ad_pmu_rail_config.rail_vdd.current_onsleep = HW_PMU_VDD_MAX_LOAD_0_400;
        ad_pmu_rail_config.rail_vdd.voltage_onwakeup = (hw_clk_get_sysclk() == SYS_CLK_IS_DBLR) ?
                                                        HW_PMU_VDD_VOLTAGE_1V20 :
                                                        HW_PMU_VDD_VOLTAGE_0V90;
        ad_pmu_rail_config.rail_vdd.voltage_onsleep = HW_PMU_VDD_VOLTAGE_SLEEP_0V75;

        configure_power_rail(PMU_RAIL_VDD, &ad_pmu_rail_config);

#if (dg_configUSE_BOD == 1)
        /* Rails have been configured. Run again BOD  */
        hw_bod_configure();
#endif

}

ADAPTER_INIT(ad_pmu_adapter, ad_pmu_init);

#endif /* OS_PRESENT */

#if (PROTECT_RF_VDCDC == 1)
/* This function blocks until the RF is no longer active (transmitting or receiving a frame)
 * and should be called before making any changes to the VDCDC rail.
 * If the system is running with an operating system, it will yield to another task while the RF is busy.
 * Please note that it should not be called from an interrupt context when an operating system is in use.
 */
static void wait_on_rf_active(void)
{
        /*
         * Wait for the RF to exit the active frame
         * and then change the VDCDC.
         * NOTE: This might last in worst case up to 2ms
         */
        if (cmac_info_table_ptr != NULL) {
                while (cmac_info_table_ptr->phy_active == true) {
#ifdef OS_PRESENT
                        if (!in_interrupt()) {
                                OS_TASK_YIELD(); /* ASSERTION will be triggered if called from INT context */
                        } else {
                                OS_TASK_YIELD_FROM_ISR(); /* ASSERTION will be triggered if called from non-INT context */
                        }
#endif
                }
        }
}
#endif /* PROTECT_RF_VDCDC */

static void __ad_pmu_vdcdc_rail_set_voltage_safe(HW_PMU_VDCDC_VOLTAGE voltage)
{
        HW_PMU_ERROR_CODE error_code;
        error_code = hw_pmu_vdcdc_set_voltage(voltage);
        ASSERT_WARNING(error_code == HW_PMU_ERROR_NOERROR);
}

static void ad_pmu_acquire_1v4_vdcdc_voltage(void)
{
        ad_pmu_1v4_vdcdc_acquire_count++;

        if (ad_pmu_1v4_vdcdc_acquire_count == 1) {
                /* This is the first time VDCDC is forced to 1.4V. */
#if (PROTECT_RF_VDCDC == 1)
                wait_on_rf_active();
#endif /* PROTECT_RF_VDCDC */

                __ad_pmu_vdcdc_rail_set_voltage_safe(HW_PMU_VDCDC_VOLTAGE_1V40);
        }
}

static void ad_pmu_release_1v4_vdcdc_voltage(void)
{
        ad_pmu_1v4_vdcdc_acquire_count--;

        if (ad_pmu_1v4_vdcdc_acquire_count == 0) {
                /* This is the last time VDCDC is released from 1.4V.
                 * Restore it's latest configured values set by ad_pmu_init() or
                 * ad_pmu_configure_rail().
                 */
#if (PROTECT_RF_VDCDC == 1)
                wait_on_rf_active();
#endif /* PROTECT_RF_VDCDC */

                __ad_pmu_vdcdc_rail_set_voltage_safe(ad_pmu_vdcdc_rail_config.rail_vdcdc.voltage_onwakeup);
        }
}

static void ad_pmu_vdd_rail_set_voltage_onwakeup_bod_safe(HW_PMU_VDD_VOLTAGE voltage)
{
        HW_PMU_ERROR_CODE error_code;
#if (dg_configUSE_BOD == 1)
        uint16_t bod_lvl = 780;
        if (voltage > HW_PMU_VDD_VOLTAGE_1V15) {
                bod_lvl = 1050;
        }
        hw_bod_deactivate_channel(HW_BOD_CHANNEL_VDD);
#endif
        error_code = hw_pmu_vdd_set_voltage(voltage);
        ASSERT_WARNING(error_code == HW_PMU_ERROR_NOERROR);
#if (dg_configUSE_BOD == 1)
        /* Set the level of the VDD BOD channel */
        hw_bod_set_channel_voltage_level(HW_BOD_CHANNEL_VDD, bod_lvl);
        /* Rail has been configured. Enable BOD on VDD.  */
        hw_bod_activate_channel(HW_BOD_CHANNEL_VDD);
#endif
}

static void ad_pmu_acquire_1v2_vdd_voltage(void)
{
        ad_pmu_1v2_vdd_acquire_count++;

        if (ad_pmu_1v2_vdd_acquire_count == 1) {
                /* This is the first time VDD is forced to 1.2V. */
                ad_pmu_vdd_rail_set_voltage_onwakeup_bod_safe(HW_PMU_VDD_VOLTAGE_1V20);
        }
}

static void ad_pmu_release_1v2_vdd_voltage(void)
{
        ad_pmu_1v2_vdd_acquire_count--;

        if (ad_pmu_1v2_vdd_acquire_count == 0) {
                /* This is the last time VDD is released from 1.2V.
                 * Restore it's latest configured values set by ad_pmu_init() or
                 * ad_pmu_configure_rail().
                 */
                ad_pmu_vdd_rail_set_voltage_onwakeup_bod_safe(ad_pmu_vdd_rail_config.rail_vdd.voltage_onwakeup);
        }
}

static void configure_power_rail(AD_PMU_RAIL rail, const ad_pmu_rail_config_t *cfg)
{
        HW_PMU_ERROR_CODE error_code = HW_PMU_ERROR_NOERROR;

        if (cfg->enabled_onwakeup) {
                /* Power up rail in active mode. */
                switch (rail) {
                case PMU_RAIL_1V8:
                        error_code = hw_pmu_1v8_onwakeup_enable(cfg->rail_1v8.current_onwakeup);
                        break;
                case PMU_RAIL_VDCDC:
                        error_code = hw_pmu_vdcdc_onwakeup_enable(cfg->rail_vdcdc.current_onwakeup);
                        break;
                case PMU_RAIL_VDD:
                        error_code = hw_pmu_vdd_onwakeup_enable(cfg->rail_vdd.current_onwakeup);
                        break;
                default:
                        ASSERT_WARNING(0);
                        break;
                }

                ASSERT_WARNING(error_code == HW_PMU_ERROR_NOERROR);

                /* Set rail voltage in active mode. */
                switch (rail) {
                case PMU_RAIL_1V8:
                        /* Nothing to do */
                        break;
                case PMU_RAIL_VDCDC:
                        error_code = hw_pmu_vdcdc_set_voltage(cfg->rail_vdcdc.voltage_onwakeup);
                        break;
                case PMU_RAIL_VDD:
                        error_code = hw_pmu_vdd_set_voltage(cfg->rail_vdd.voltage_onwakeup);
                        break;
                }

                ASSERT_WARNING(error_code == HW_PMU_ERROR_NOERROR);

        } else {
                /* Power down rail in active mode. */
                switch (rail) {
                case PMU_RAIL_1V8:
                        error_code = hw_pmu_1v8_onwakeup_disable();
                        break;
                case PMU_RAIL_VDCDC:
                        error_code = hw_pmu_vdcdc_onwakeup_disable();
                        break;
                case PMU_RAIL_VDD:
                        error_code = hw_pmu_vdd_onwakeup_disable();
                        break;
                default:
                        ASSERT_WARNING(0);
                        break;
                }

                ASSERT_WARNING(error_code == HW_PMU_ERROR_NOERROR);
        }

        if (cfg->enabled_onsleep) {
                /* Power up rail in sleep mode. */
                switch (rail) {
                case PMU_RAIL_1V8:
                        error_code = hw_pmu_1v8_onsleep_enable(cfg->rail_1v8.current_onsleep);
                        break;
                case PMU_RAIL_VDCDC:
                        error_code = hw_pmu_vdcdc_onsleep_enable(cfg->rail_vdcdc.current_onsleep);
                        break;
                case PMU_RAIL_VDD:
                        error_code = hw_pmu_vdd_onsleep_enable(cfg->rail_vdd.current_onsleep);
                        break;
                default:
                        ASSERT_WARNING(0);
                        break;
                }

                ASSERT_WARNING(error_code == HW_PMU_ERROR_NOERROR);

                /* Set rail voltage in sleep mode. */
                switch (rail) {
                case PMU_RAIL_1V8:
                        /* Nothing to do */
                        break;
                case PMU_RAIL_VDCDC:
                        /* Nothing to do */
                        break;
                case PMU_RAIL_VDD:
                        error_code = hw_pmu_vdd_set_voltage(cfg->rail_vdd.voltage_onsleep);
                        break;
                }

                ASSERT_WARNING(error_code == HW_PMU_ERROR_NOERROR);

        } else {
                /* Power down rail in sleep mode. */
                switch (rail) {
                case PMU_RAIL_1V8:
                        error_code = hw_pmu_1v8_onsleep_disable();
                        break;
                case PMU_RAIL_VDCDC:
                        error_code = hw_pmu_vdcdc_onsleep_disable();
                        break;
                case PMU_RAIL_VDD:
                        error_code = hw_pmu_vdd_onsleep_disable();
                        break;
                default:
                        ASSERT_WARNING(0);
                        break;
                }

                ASSERT_WARNING(error_code == HW_PMU_ERROR_NOERROR);
        }

        /* Update configuration */

        switch (rail) {
        case PMU_RAIL_1V8:
                /* Nothing to do */
                break;
        case PMU_RAIL_VDCDC:
                ad_pmu_vdcdc_rail_config = *cfg;
                break;
        case PMU_RAIL_VDD:
                ad_pmu_vdd_rail_config = *cfg;
                break;
        }
}

void ad_pmu_1v2_force_max_voltage_request(void)
{
        GLOBAL_INT_DISABLE();
#ifdef CONFIG_USE_BLE
        while (hw_sys_hw_bsr_try_lock(HW_BSR_MASTER_SYSCPU, HW_BSR_POWER_CTRL_POS) == false);
#endif /* CONFIG_USE_BLE */


        ad_pmu_acquire_1v4_vdcdc_voltage();
        ad_pmu_acquire_1v2_vdd_voltage();

#ifdef CONFIG_USE_BLE
        cmac_update_power_ctrl_reg_values(CRG_TOP->POWER_LEVEL_REG);
#endif /* CONFIG_USE_BLE */

#ifdef CONFIG_USE_BLE
        hw_sys_hw_bsr_unlock(HW_BSR_MASTER_SYSCPU, HW_BSR_POWER_CTRL_POS);
#endif /* CONFIG_USE_BLE */
        GLOBAL_INT_RESTORE();
}

void ad_pmu_1v2_force_max_voltage_release(void)
{
        GLOBAL_INT_DISABLE();
#ifdef CONFIG_USE_BLE
        while (hw_sys_hw_bsr_try_lock(HW_BSR_MASTER_SYSCPU, HW_BSR_POWER_CTRL_POS) == false);
#endif /* CONFIG_USE_BLE */

        ad_pmu_release_1v2_vdd_voltage();
        ad_pmu_release_1v4_vdcdc_voltage();

#ifdef CONFIG_USE_BLE
        cmac_update_power_ctrl_reg_values(CRG_TOP->POWER_LEVEL_REG);
#endif /* CONFIG_USE_BLE */

#ifdef CONFIG_USE_BLE
        hw_sys_hw_bsr_unlock(HW_BSR_MASTER_SYSCPU, HW_BSR_POWER_CTRL_POS);
#endif /* CONFIG_USE_BLE */
        GLOBAL_INT_RESTORE();
}

int ad_pmu_configure_rail(AD_PMU_RAIL rail, const ad_pmu_rail_config_t *config)
{
#ifdef OS_PRESENT
        AD_PMU_MUTEX_GET();
#else
        GLOBAL_INT_DISABLE();
#endif

#ifdef CONFIG_USE_BLE
        while (hw_sys_hw_bsr_try_lock(HW_BSR_MASTER_SYSCPU, HW_BSR_POWER_CTRL_POS) == false);
#endif /* CONFIG_USE_BLE */

#if (dg_configUSE_BOD == 1)
        /* Deactivate BOD to configure the rails. */
        hw_bod_deactivate();
#endif

        configure_power_rail(rail, config);

#ifdef CONFIG_USE_BLE
        cmac_update_power_ctrl_reg_values(CRG_TOP->POWER_LEVEL_REG);
#endif /* CONFIG_USE_BLE */

#if (dg_configUSE_BOD == 1)
        /* Rails have been configured. Run again BOD.  */
        hw_bod_configure();
#endif

#ifdef CONFIG_USE_BLE
        hw_sys_hw_bsr_unlock(HW_BSR_MASTER_SYSCPU, HW_BSR_POWER_CTRL_POS);
#endif /* CONFIG_USE_BLE */


#ifdef OS_PRESENT
        AD_PMU_MUTEX_PUT();
#else
        GLOBAL_INT_RESTORE();
#endif
        return 0;
}

__STATIC_INLINE HW_PMU_VDCDC_VOLTAGE __vdcdc_get_voltage_level()
{
        return REG_GETF(CRG_TOP, POWER_LEVEL_REG, VDCDC_LEVEL);
}

static void ad_pmu_vdcdc_rail_set_voltage_level(HW_PMU_VDCDC_VOLTAGE voltage)
{
#ifdef CONFIG_USE_BLE
        if (dg_configDEFAULT_RADIO_OP_MODE == HIGH_PERFORMANCE_ALL_PHYS) {
                /* Do nothing, the voltage level should have already been taken care of. */
                ASSERT_WARNING(__vdcdc_get_voltage_level() == HW_PMU_VDCDC_VOLTAGE_1V40);
        } else
#endif
        {
                __ad_pmu_vdcdc_rail_set_voltage_safe(voltage);
        }
}

void ad_pmu_prepare_for_sleep(void)
{
        GLOBAL_INT_DISABLE();
#ifdef CONFIG_USE_BLE
        while (hw_sys_hw_bsr_try_lock(HW_BSR_MASTER_SYSCPU, HW_BSR_POWER_CTRL_POS) == false);
#endif /* CONFIG_USE_BLE */

        ad_pmu_vdd_rail_set_voltage_onwakeup_bod_safe(HW_PMU_VDD_VOLTAGE_0V90);
        ad_pmu_vdcdc_rail_set_voltage_level(HW_PMU_VDCDC_VOLTAGE_1V10);

#ifdef CONFIG_USE_BLE
        /* Communicate power rail levels to CMAC */
        cmac_update_power_ctrl_reg_values(CRG_TOP->POWER_LEVEL_REG);

        hw_sys_hw_bsr_unlock(HW_BSR_MASTER_SYSCPU, HW_BSR_POWER_CTRL_POS);
#endif /* CONFIG_USE_BLE */
        GLOBAL_INT_RESTORE();
}

void ad_pmu_restore_for_wake_up(void)
{
        GLOBAL_INT_DISABLE();
#ifdef CONFIG_USE_BLE
        while (hw_sys_hw_bsr_try_lock(HW_BSR_MASTER_SYSCPU, HW_BSR_POWER_CTRL_POS) == false);
#endif /* CONFIG_USE_BLE */

        ad_pmu_vdcdc_rail_set_voltage_level(ad_pmu_vdcdc_rail_config.rail_vdcdc.voltage_onwakeup);
        ad_pmu_vdd_rail_set_voltage_onwakeup_bod_safe(ad_pmu_vdd_rail_config.rail_vdd.voltage_onwakeup);

#ifdef CONFIG_USE_BLE
        /* Communicate power rail levels to CMAC */
        cmac_update_power_ctrl_reg_values(CRG_TOP->POWER_LEVEL_REG);

        hw_sys_hw_bsr_unlock(HW_BSR_MASTER_SYSCPU, HW_BSR_POWER_CTRL_POS);
#endif /* CONFIG_USE_BLE */
        GLOBAL_INT_RESTORE();
}

#endif /* dg_configPMU_ADAPTER */

