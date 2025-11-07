/**
 ****************************************************************************************
 *
 * @file sys_power_mgr_da1459x.c
 *
 * @brief Power Manager
 *
 * Copyright (C) 2020-2024 Renesas Electronics Corporation and/or its affiliates.
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


#include "hw_clk.h"
#if dg_configUSE_HW_PMU
#       include "hw_bod.h"
#       include "hw_pmu.h"
#endif
#include "eflash_automode.h"
#include "qspi_automode.h"
#include "hw_cpm.h"
#include "hw_fcu.h"
#include "hw_sys.h"
#include "hw_pd.h"
#include "hw_pdc.h"
#include "hw_rtc.h"
#include "hw_watchdog.h"
#include "sys_clock_mgr.h"
#include "sys_clock_mgr_internal.h"
#include "sys_power_mgr.h"
#include "sys_power_mgr_internal.h"
#include "sys_watchdog.h"
#include "sys_watchdog_internal.h"
#if (dg_configUSE_SYS_TCS == 1)
#include "sys_tcs.h"
#endif
#if (dg_configUSE_SYS_DRBG == 1)
        #include "sys_drbg.h"
#endif
#if (dg_configUSE_SYS_ADC)
#include "sys_adc_internal.h"
#endif

#ifdef OS_PRESENT
#       include "osal.h"
#       include "sys_trng.h"
#       include "sys_timer.h"
#       include "sys_timer_internal.h"
#       include "sys_bsr.h"
#       include "hw_dma.h"
#       include "resmgmt.h"
#       if (USE_BLE_SLEEP == 1)
#               include "ad_ble.h"
#       endif
#endif /* OS_PRESENT */
#if dg_configPMU_ADAPTER
#       include "ad_pmu.h"
#       include "../adapters/src/ad_pmu_internal.h"
#endif

#define PM_ENABLE_SLEEP_DIAGNOSTICS     (0)
#define PM_SLEEP_MODE_REQUEST_THRESHOLD (64)

#define WAKEUP_NORMAL_RC32K_CYCLES      (3)
#define WAKEUP_NORMAL_RC512K_CYCLES     (29)

#if (dg_configUSE_LP_CLK == LP_CLK_RCX)
#       define WAKEUP_TIME_2_LP_CYCLES(wkup_time) ((((uint64_t)(wkup_time) * rcx_clock_hz) / 1000000) + 1)
#else
#       define WAKEUP_TIME_2_LP_CYCLES(wkup_time) ((((uint64_t)(wkup_time) * dg_configXTAL32K_FREQ) / 1000000) + 1)
#endif

bool goto_deepsleep(void);

#ifdef OS_PRESENT
#       if (PWR_MGR_DEBUG == 1)
#               pragma message "Power Manager: Debugging mode is on!"
#       endif

#       if ((PWR_MGR_FUNCTIONAL_DEBUG == 1) || (PWR_MGR_USE_TIMING_DEBUG == 1))
#               pragma message "Power Manager: GPIO Debugging is on!"
#       endif

#       if (FLASH_DEBUG == 1)
#               pragma message "Flash: Flash Debugging is on!"
#       endif

#       define GUARD_TIME                              64      // in slots

// Make sure that sleep time can be measured by the sleep timer
#       if (dg_configUSE_LP_CLK == LP_CLK_RCX)
#               define MAX_TIMER_IDLE_COUNT  (LP_CNT_NATIVE_MASK + 1 - GUARD_TIME * 32)
#       else
#               define MAX_TIMER_IDLE_COUNT  (LP_CNT_NATIVE_MASK + 1 - GUARD_TIME * OS_TICK_PERIOD)
#       endif

// Watchdog is active during sleep. The system must wake-up before the watchdog expires to reload its counter
#       if (dg_configUSE_LP_CLK == LP_CLK_RCX)
                // When LP clock is RCX, the watchdog is clocked by RCX clock divided by 320
#               define WDOG_VALUE_2_LP_CLKS(wdg_val) ((wdg_val) * 320)
#       elif (dg_configUSE_LP_CLK == LP_CLK_32768) || (dg_configUSE_LP_CLK == LP_CLK_32000)
                // When LP clock is not RCX, the watchdog is clocked by RCLP (auto-divided, if needed,
                // to 32kHz) divided by 320. Thus, in this case, the IDLE count must be calculated
                // according to dg_configXTAL32K_FREQ and rclp_hz_slow_max.
#               define WDOG_VALUE_2_LP_CLKS(wdg_val) ((wdg_val) * dg_configXTAL32K_FREQ / (rclp_hz_slow_max / 320))
#       else
                #error "Not supported LP clock."
#       endif

#       if (dg_configUSE_LP_CLK != LP_CLK_RCX)
#               define MAX_IDLE_TICKS_ALLOWED        ((MAX_TIMER_IDLE_COUNT / OS_TICK_PERIOD) + 5)
#       endif

/*
 * The watchdog value we got to calculate sleep period may be reduced by 1 until the system goes to
 * sleep. So we define a watchdog margin of 1 watchdog tick + half LP clock tick period (e.g. 11 ms
 * in case of XTAL32K).
 *
 */
#       define WDOG_MARGIN                             (WDOG_VALUE_2_LP_CLKS(1) + (OS_TICK_PERIOD / 2))

#       define PM_MUTEX_CREATE()                        OS_ASSERT(pm_mutex == NULL); \
                                                        OS_MUTEX_CREATE(pm_mutex); \
                                                        OS_ASSERT(pm_mutex)
#       define PM_MUTEX_GET()                           OS_ASSERT(pm_mutex); \
                                                        OS_MUTEX_GET(pm_mutex, OS_MUTEX_FOREVER)
#       define PM_MUTEX_PUT()                           OS_MUTEX_PUT(pm_mutex)

/*
 * Global and / or retained variables
 */

        __RETAINED static periph_init_cb periph_init;
        __RETAINED static OS_MUTEX pm_mutex;
        __RETAINED static bool sleep_is_blocked;

        __RETAINED static sleep_mode_t user_sleep_mode;
        __RETAINED static uint8_t sleep_mode_cnt[pm_mode_sleep_max];
        __RETAINED static adapter_call_backs_t *adapters_cb[dg_configPM_MAX_ADAPTERS_CNT];

#       if (PWR_MGR_DEBUG == 1)
        __RETAINED uint32_t low_power_periods_ret;
        __RETAINED uint32_t sleep_period_ret;
        __RETAINED uint32_t trigger_setting_ret;
        __RETAINED uint32_t lp_time1_ret;  // Power-up: start
        __RETAINED uint32_t lp_time2_ret;  // Power-up: after the activation of the Power Domains
        __RETAINED uint32_t lp_time3_ret;  // Power-up: after the clock setting - finish
#       endif
#else
#       define PM_MUTEX_CREATE()
#       define PM_MUTEX_GET()
#       define PM_MUTEX_PUT()
#endif /* OS_PRESENT */

__RETAINED static sleep_mode_t current_sleep_mode;
__RETAINED static system_state_t system_sleeping;
__RETAINED static bool wakeup_mode_is_XTAL32;   // false: RC32, true: XTAL32M/DBLR

#ifdef OS_PRESENT

/*
 * Local variables
 */

static uint32_t ulTimeSpentSleepingInTicks;                    // Counts in ticks

static bool adapters_wake_up_ind_called = false;
static bool call_adapters_xtal16m_ready_ind = false;

static uint64_t sleep_blocked_until = 0;

#if dg_configENABLE_DEBUGGER
/* Contains the index of the combo trigger PDC LUT entry */
__RETAINED static uint32_t jtag_wkup_combo_pdc_entry_idx;
/* Indicates when sleep blocking due to wake-up from JTAG ends. */
static uint64_t jtag_wkup_sleep_blocked_until = 0;
#endif

/*
 * Forward declarations
 */

/*
 * \brief When resuming the OS, check if we were sleeping and calculate the time we slept.
 *
 * \warning Must be called with interrupts disabled!
 */
__RETAINED_HOT_CODE static void sleep_exit(void);

#if dg_configENABLE_DEBUGGER
static void jtag_wkup_discover_combo_pdc_entry_idx(void);
__RETAINED_CODE static void jtag_wkup_check(bool sys_cpu_entered_sleep);
__STATIC_INLINE bool jtag_wkup_delay_has_expired(void);
#endif

/*
 * Function definitions
 */

static void init_component(const comp_init_tree_t **done, int *done_cnt,
                                                                const comp_init_tree_t *for_init)
{
        int i;

        /* Look in done array to see if for_init component was already initialized. */
        for (i = 0; i < *done_cnt; ++i) {
                if (done[i] == for_init) {
                        return;
                }
        }

        /* If not found, component was not initialized yet */

        /* Initialized dependencies first */
        for (i = 0; for_init->depend && for_init->depend[i]; ++i) {
                init_component(done, done_cnt, for_init->depend[i]);
        }
        /* Mark as initialized and call initialization function */
        done[*done_cnt] = for_init;
        ++(*done_cnt);
        if (for_init->init_fun) {
                for_init->init_fun(for_init->init_arg);
        }
}

static void init_components(const comp_init_tree_t **init, int num)
{
        /* Array of already initialized components */
        const comp_init_tree_t *done[num];
        /* Number of already initialized components */
        int done_cnt = 0;

        while (num) {
                init_component(done, &done_cnt, *init);
                ++init;
                --num;
        }
}

/* Symbols generated by linker */
extern const comp_init_tree_t *__start_adapter_init_section;
extern const comp_init_tree_t *__stop_adapter_init_section;
/*
 * In case there are no adapters defined make sure that 0 size linker section is generated.
 */
static const int adapter_init_section_marker[0] __attribute__((section("adapter_init_section")))
                                                                        __USED = {};

static void init_adapters(void)
{
        const comp_init_tree_t **init = &__start_adapter_init_section;
        /* Linker will provide those two symbols so number of adapter can be computed */
        const int32_t num = ((uintptr_t)&__stop_adapter_init_section -
                             (uintptr_t)&__start_adapter_init_section) /
                            sizeof(uint32_t);
        /*
         * Make sure there is no error in configuration. If this assert fires it probably
         * means that there are too many adapters declared with ADAPTER_INIT macros or
         * there is something else put in adapter_init_section.
         */
        ASSERT_ERROR(num <= dg_configPM_MAX_ADAPTERS_CNT);

        init_components(init, num);
}

/* Symbols generated by linker */
extern const comp_init_tree_t *__start_bus_init_section;
extern const comp_init_tree_t *__stop_bus_init_section;
/*
 * In case there are no buses defined make sure that 0 size linker section is generated.
 */
static const int bus_init_section_marker[0] __attribute__((section("bus_init_section")))
                                                                        __USED = {};

static void init_buses(void)
{
        const int32_t num = ((uintptr_t)&__stop_bus_init_section -
                             (uintptr_t)&__start_bus_init_section) /
                            sizeof(uint32_t);
        init_components(&__start_bus_init_section, num);
}

/* Symbols generated by linker */
extern const comp_init_tree_t *__start_device_init_section;
extern const comp_init_tree_t *__stop_device_init_section;
/*
 * In case there are no devices defined make sure that 0 size linker section is generated.
 */
static const int device_init_section_marker[0] __attribute__((section("device_init_section")))
                                                                        __USED = {};

static void init_devices(void)
{
        const int32_t num = ((uintptr_t)&__stop_device_init_section -
                             (uintptr_t)&__start_device_init_section) /
                            sizeof(uint32_t);
        init_components(&__start_device_init_section, num);
}

static void xtalm_ready_cb(void)
{
        if (adapters_wake_up_ind_called) {
                for (int i = 0; i < dg_configPM_MAX_ADAPTERS_CNT; i++) {
                        // Inform Adapters.
                        adapter_call_backs_t *p_Ad = adapters_cb[i];
                        if ((p_Ad != NULL) && (p_Ad->ad_xtalm_ready_ind != NULL)) {
                                p_Ad->ad_xtalm_ready_ind();
                        }
                }
        } else {
                call_adapters_xtal16m_ready_ind = true;
        }
}

#if (PM_ENABLE_SLEEP_DIAGNOSTICS == 1)
static void pm_enable_sleep_diagnostics(void)
{
        /*
         * Enable mapping of sleep diagnostic signals to GPIOs
         *      BANDGAP_ENABLE  -> P0_11
         *      WOKENUP         -> P1_02
         */
        hw_sys_pd_com_enable();

        hw_gpio_set_pin_function(HW_GPIO_PORT_0, HW_GPIO_PIN_11, HW_GPIO_MODE_OUTPUT, HW_GPIO_FUNC_GPIO);
        hw_gpio_set_pin_function(HW_GPIO_PORT_1, HW_GPIO_PIN_2, HW_GPIO_MODE_OUTPUT, HW_GPIO_FUNC_GPIO);

        // Enable sleep diagnostics
        REG_SETF(CRG_TOP, PMU_CTRL_REG, MAP_BANDGAP_EN, 1);

        hw_gpio_pad_latch_enable(HW_GPIO_PORT_0, HW_GPIO_PIN_11);
        hw_gpio_pad_latch_enable(HW_GPIO_PORT_1, HW_GPIO_PIN_2);
        hw_gpio_pad_latch_disable(HW_GPIO_PORT_0, HW_GPIO_PIN_11);
        hw_gpio_pad_latch_disable(HW_GPIO_PORT_1, HW_GPIO_PIN_2);

        hw_sys_pd_com_disable();
}
#endif /* (PM_ENABLE_SLEEP_DIAGNOSTICS == 1) */


void pm_system_init(periph_init_cb peripherals_initialization)
{
        /* Time to copy the image must be less than the minimum sleep time */
        ASSERT_WARNING(dg_configIMAGE_COPY_TIME < dg_configMIN_SLEEP_TIME);

        periph_init = peripherals_initialization;

#if (dg_configPM_ENABLES_PD_COM_WHILE_ACTIVE == 1)
        hw_sys_pd_com_enable();
#if (dg_configUSE_SYS_TCS == 1)
        sys_tcs_apply_reg_pairs(SYS_TCS_GROUP_PD_COMM);
#endif
        hw_sys_set_preferred_values(HW_PD_COM);
#endif

        hw_sys_setup_retmem();                          // Set retmem mode

#ifdef OS_PRESENT
#if (dg_configUSE_SYS_DRBG == 1)
        sys_drbg_create_os_objects();
#endif
#endif /* OS_PRESENT */

#if (PM_ENABLE_SLEEP_DIAGNOSTICS == 1)
        pm_enable_sleep_diagnostics();
#endif /* (PM_ENABLE_SLEEP_DIAGNOSTICS == 1) */

        hw_sys_set_cache_retained();                    // Set cache in retained mode

#if (dg_configUSE_SW_CURSOR == 1)
        hw_sys_setup_sw_cursor();
#endif

        if (periph_init != NULL) {
                periph_init();                          // Power on peripherals and GPIOs
        }

        DBG_CONFIGURE_HIGH(PWR_MGR_FUNCTIONAL_DEBUG, PWRDBG_POWERUP);
        DBG_CONFIGURE_HIGH(PWR_MGR_USE_TIMING_DEBUG, PWRDBG_SLEEP_EXIT);

        PM_MUTEX_CREATE();                              // Create Mutex. Called only once!

        /* Init system BSR */
        sys_sw_bsr_init();

        resource_init();
        init_adapters();
        init_buses();
        init_devices();

#if (dg_configUSE_WDOG == 1)
        sys_watchdog_init();
        hw_watchdog_unfreeze();                 // Start watchdog
#else
        hw_watchdog_freeze();                   // Stop watchdog
        sys_watchdog_set_pos_val(dg_configWDOG_IDLE_RESET_VALUE);
#endif

        cm_register_xtal_ready_callback(xtalm_ready_cb);

#if (dg_configUSE_SYS_ADC == 1)
        cm_rc_clocks_calibration_task_init();
        sys_adc_init();
#endif


#if dg_configENABLE_DEBUGGER
        jtag_wkup_discover_combo_pdc_entry_idx();
#endif
}

void pm_wait_debugger_detach(sleep_mode_t mode)
{
#if (dg_configUSE_WDOG == 1)
        hw_watchdog_freeze();                   // Stop watchdog
#endif
        // Make sure that the debugger is detached else sleep won't be possible
#if (OS_USE_TICKLESS_IDLE != 0)
        if (mode != pm_mode_active) {
                volatile bool loop = true;
                while (loop && hw_sys_is_debugger_attached()) {
                }
        }
#endif

#if (dg_configUSE_WDOG == 1)
        hw_watchdog_unfreeze();                 // Start watchdog
#endif
}
#endif /* OS_PRESENT */

void pm_set_wakeup_mode(bool wait_for_xtalm)
{
        PM_MUTEX_GET();                                 // Block forever

        wakeup_mode_is_XTAL32 = wait_for_xtalm;

        PM_MUTEX_PUT();
}

bool pm_get_wakeup_mode(void)
{
        bool mode;

        PM_MUTEX_GET();                                 // Block forever

        mode = wakeup_mode_is_XTAL32;

        PM_MUTEX_PUT();

        return mode;
}

#ifdef OS_PRESENT
static void pm_sleep_mode_update(void)
{
        /* Select the power mode with the highest priority */
        if ((sleep_mode_cnt[pm_mode_active] > 0) || (user_sleep_mode == pm_mode_active)) {
                current_sleep_mode = pm_mode_active;
        } else if ((sleep_mode_cnt[pm_mode_idle] > 0) || (user_sleep_mode == pm_mode_idle)) {
                current_sleep_mode = pm_mode_idle;
        } else if ((sleep_mode_cnt[pm_mode_extended_sleep] > 0) || (user_sleep_mode == pm_mode_extended_sleep)) {
                current_sleep_mode = pm_mode_extended_sleep;
        } else {
                current_sleep_mode = user_sleep_mode;
        }
}

sleep_mode_t pm_sleep_mode_set(sleep_mode_t mode)
{
        sleep_mode_t previous_mode;

        ASSERT_ERROR(mode < pm_mode_sleep_max);
        GLOBAL_INT_DISABLE();

        previous_mode = current_sleep_mode;
        user_sleep_mode = mode;
        pm_sleep_mode_update();

        GLOBAL_INT_RESTORE();

        return previous_mode;
}

sleep_mode_t pm_sleep_mode_get(void)
{
        sleep_mode_t mode;

        GLOBAL_INT_DISABLE();

        mode = current_sleep_mode;

        GLOBAL_INT_RESTORE();

        return mode;
}

void pm_sleep_mode_request(sleep_mode_t mode)
{
        ASSERT_ERROR(mode < pm_mode_sleep_max);

        GLOBAL_INT_DISABLE();

        ASSERT_ERROR(sleep_mode_cnt[mode] < PM_SLEEP_MODE_REQUEST_THRESHOLD);

        sleep_mode_cnt[mode]++;
        pm_sleep_mode_update();

        GLOBAL_INT_RESTORE();
}

void pm_sleep_mode_release(sleep_mode_t mode)
{
        ASSERT_ERROR(mode < pm_mode_sleep_max);

        GLOBAL_INT_DISABLE();

        ASSERT_ERROR(sleep_mode_cnt[mode] != 0);

        sleep_mode_cnt[mode]--;
        pm_sleep_mode_update();

        GLOBAL_INT_RESTORE();
}

pm_id_t pm_register_adapter(const adapter_call_backs_t *cb)
{
        pm_id_t ret = -1;
        int i = 0;

        OS_ASSERT(cb != NULL);

        PM_MUTEX_GET();                                 // Block forever

        while ((i < dg_configPM_MAX_ADAPTERS_CNT) && (adapters_cb[i] != NULL)) {
                i++;
        }

        if (i < dg_configPM_MAX_ADAPTERS_CNT) {
                adapters_cb[i] = (adapter_call_backs_t *)cb;
                ret = i;
        }

        OS_ASSERT(ret != -1);                      // Increase dg_configPM_MAX_ADAPTERS_CNT

        PM_MUTEX_PUT();

        return ret;
}

void pm_unregister_adapter(pm_id_t id)
{
        PM_MUTEX_GET();                                 // Block forever

        OS_ASSERT((id >= 0) && (id < dg_configPM_MAX_ADAPTERS_CNT));     // Is id valid?
        OS_ASSERT(adapters_cb[id] != NULL);     // Is it registered?

        adapters_cb[id] = NULL;

        PM_MUTEX_PUT();
}

void pm_defer_sleep_for(pm_id_t id, uint32_t time_in_LP_cycles)
{
        uint64_t rtc_time;
        uint64_t lp_block_time;

        ASSERT_WARNING((id >= 0) && (id < dg_configPM_MAX_ADAPTERS_CNT));
        ASSERT_WARNING(adapters_cb[id] != NULL);                     // Is it registered?
        ASSERT_WARNING(time_in_LP_cycles <= dg_configPM_MAX_ADAPTER_DEFER_TIME);

        /*
         * Update Real Time Clock value
         */
        rtc_time = sys_timer_get_uptime_ticks_fromISR();

        lp_block_time = rtc_time + time_in_LP_cycles;

        if (sleep_is_blocked == false) {
                sleep_blocked_until = lp_block_time;
                sleep_is_blocked = true;
        } else {
                // Update only if the new block time is after the previous one
                if (sleep_blocked_until < lp_block_time) {
                        sleep_blocked_until = lp_block_time;
                }
        }
}

/**
 * \brief Initialize the system after wake-up.
 *
 * \warning Called in ISR context after the settling of the XTAL.
 *
 */
static void sys_init_wake_up(void)
{
         int i;
         uint32_t iser = 0, iser2 = 0;
         adapter_call_backs_t *p_Ad;

 //        if (dg_configLP_CLK_SOURCE == LP_CLK_IS_DIGITAL) {
 //                 hw_clk_configure_ext32k_pins();
 //        }

         // No need to configure pins when XTAL32K is used
         //sys_tcs_apply(tcs_system);

         /*
          * Restore AHB, APB. If sys_clk is not RC32, it will be restored when the XTAL32
          * settles.
          */

         GLOBAL_INT_DISABLE();

         cm_sys_clk_sleep(false);

         bool xtal32_ready = cm_poll_xtalm_ready();

         if (wakeup_mode_is_XTAL32 && !xtal32_ready) {
                 /* Mask interrupts again, since the Adapters or the periph_init() may call
                  * NVIC_EnableIRQ(). This is to ensure that no code other than the power manager
                  * code will be executed until the XTAL32M has settled, as the wake-up mode defines.
                  */
                 iser  = NVIC->ISER[0];                   // Log interrupt enable status (IRQs 00-31)
                 iser2 = NVIC->ISER[1];                   // Log interrupt enable status (IRQs 32-63)
         }

         // Inform Adapters
         adapters_wake_up_ind_called = true;

         if (periph_init != NULL) {
                 periph_init();                          // Power on peripherals and GPIOs
         }

         for (i = 0; i < dg_configPM_MAX_ADAPTERS_CNT; i++) {
                 p_Ad = adapters_cb[i];
                 if ((p_Ad != NULL) && (p_Ad->ad_wake_up_ind != NULL)) {
                         p_Ad->ad_wake_up_ind(false);
                 }
         }

         if (call_adapters_xtal16m_ready_ind) {
                 for (i = 0; i < dg_configPM_MAX_ADAPTERS_CNT; i++) {
                         p_Ad = adapters_cb[i];
                         if ((p_Ad != NULL) && (p_Ad->ad_xtalm_ready_ind != NULL)) {
                                 p_Ad->ad_xtalm_ready_ind();
                         }
                 }
         }

         if (wakeup_mode_is_XTAL32 && !xtal32_ready) {
                 /* Disable all interrupts except for the ones that were active when this function
                  * was entered. No adapter should activate an interrupt that is not normally active
                  * in the ad_wake_up_ind() call-back. All interrupts will be properly restored to
                  * the state they were before the system entered into the sleep mode.
                  */
                 NVIC->ICER[0] = ~iser;
                 NVIC->ICER[1] = ~iser2;
                 NVIC->ISER[0] = iser;
                 NVIC->ISER[1] = iser2;
         }

         GLOBAL_INT_RESTORE();

         DBG_CONFIGURE_HIGH(PWR_MGR_FUNCTIONAL_DEBUG, PWRDBG_TICK);
         DBG_CONFIGURE_LOW(PWR_MGR_FUNCTIONAL_DEBUG, PWRDBG_POWERUP);
         DBG_CONFIGURE_LOW(EXCEPTION_DEBUG, EXCEPTIONDBG);
#ifdef OS_SYS_POST_SLEEP_PROCESSING
         // A user definable macro that allows application code to be added.
         OS_SYS_POST_SLEEP_PROCESSING();
#endif
}

/*
 * \brief When continuing from the WFI(), check if we were sleeping and, if so, power-up the system.
 *
 * \warning Must be called with interrupts disabled!
 *
 */
static __RETAINED_HOT_CODE void system_wake_up(void)
{
        /*
         * Check if it is a wake-up.
         */
        if (system_sleeping == sys_powered_down) {
                uint32_t iser, iser2;

                /*
                 * Disable all IRQs except for Timer2 and XTAL32M_RDY. Other ISRs may call OS routines
                 * that require the system timing to have been restored. Thus, their execution must
                 * be deferred until the call to the OS_TICK_INCREMENT().
                 */
                iser  = NVIC->ISER[0];                   // Log interrupt enable status (IRQs 00-31)
                iser2 = NVIC->ISER[1];                   // Log interrupt enable status (IRQs 32-63)

                /*
                 * WARNING: XTAL32M_RDY_IRQn and TIMER2_IRQn are assumed to be in NVIC->ICER[0].
                 * If not then the following code must be modified accordingly.
                 */

                /* Disable all interrupts except of XTAL32M */
                NVIC->ICER[0] = iser & ~((uint32_t)(1 << XTAL32M_RDY_IRQn));
                NVIC->ICER[1] = iser2;

                /*
                 * If the code stops at this point then the interrupts were enabled while they
                 * shouldn't be so.
                 */
                ASSERT_WARNING(__get_PRIMASK() == 1  || __get_BASEPRI() != 0);

                cm_enable_xtalm_if_required();

                cm_sys_clk_wakeup();

                DBG_SET_LOW(PWR_MGR_FUNCTIONAL_DEBUG, PWRDBG_TICK);

                /*
                 * Determine how long the microcontroller was actually in a low power
                 * state. This time will be less than xExpectedIdleTime if the
                 * microcontroller was brought out of low power mode by an interrupt
                 * other than the sleep timer interrupt.
                 *
                 * This information is provided by the Power Manager.
                 *
                 * Note that the scheduler is suspended before
                 * portSUPPRESS_TICKS_AND_SLEEP() is called, and resumed when
                 * portSUPPRESS_TICKS_AND_SLEEP() returns.  Therefore no other tasks will
                 * execute until this function completes.
                 */

                /*
                 * While we are in critical section, correct the kernel's tick count
                 * to account for the time the microcontroller spent in tickless sleep state.
                 * This will update the OS-TICK but will not allow any task to run */
                sleep_exit();

                /* prevent any task from running until we are done with wakeup process */
                OS_TASK_SUSPEND_ALL();

                /* enable the TIMER2 (OS-TICK timer to hit to keep the OS-TICK updated
                 * through the wakeup process */
                NVIC->ISER[0] |= (uint32_t)(1 << TIMER2_IRQn);

               /* Exit the critical section. */
                __enable_irq();

#if (PWR_MGR_DEBUG == 1)
                sys_timer_get_timestamp_fromCPM(&lp_time1_ret);
#endif

                sys_init_wake_up();

#if (PWR_MGR_DEBUG == 1)
                sys_timer_get_timestamp_fromCPM(&lp_time2_ret);
#endif

                cm_switch_to_xtalm_if_settled();

                if (wakeup_mode_is_XTAL32) {
                        /* Wait for XTAL32 and PLL, if required. */
                        cm_halt_until_sysclk_ready();
                }

#if (PWR_MGR_DEBUG == 1)
                sys_timer_get_timestamp_fromCPM(&lp_time3_ret);
#endif

                DBG_SET_LOW(PWR_MGR_USE_TIMING_DEBUG, PWRDBG_SLEEP_EXIT);

                /* We are done with the Wakeup process.
                 * Restore all interrupts ... */
                NVIC->ISER[0] = iser;
                NVIC->ISER[1] = iser2;

#if (dg_configUSE_SYS_ADC == 1)
                 /* We are in the IDLE task context here.
                  * We need to call the sys_adc_trigger() before resuming the OS_tasks
                  * to make sure it will run as soon as possible after resuming the OS tasks
                  * and not at the next go-to-sleep period when the IDLE task will resume
                  * or next sys_adc_timer expiration. */
                 sys_adc_trigger();
#endif
                /* ... and resume all OS tasks */
                OS_TASK_RESUME_ALL();
        } else {
                /* Correct the kernel's tick count to account for the time the
                 * microcontroller spent in its low power state.
                 */
                sleep_exit();

                DBG_SET_LOW(PWR_MGR_USE_TIMING_DEBUG, PWRDBG_SLEEP_EXIT);

#if (dg_configUSE_SYS_ADC == 1)
                /* We are in the IDLE task context here.
                 * We need to call the sys_adc_trigger() before leaving the
                 * protected code execution (disabled interrupts)
                 * to make sure it will run as soon as possible after the interrupts
                 * are enabled and not at the next go-to-sleep period
                 * when the IDLE task will resume
                 * or next sys_adc_timer expiration */
                 sys_adc_trigger();
#endif

                system_sleeping = sys_active;

                __enable_irq();
        }
}

__RETAINED_HOT_CODE static void sleep_exit(void)
{
        if (system_sleeping != sys_active) {
                /*
                 * Calculate how long we've been idle or sleeping and update OS
                 */
                ulTimeSpentSleepingInTicks = sys_timer_update_slept_time();

#if (dg_configUSE_LP_CLK != LP_CLK_RCX)
                /* No more that MAX_IDLE_TICKS_ALLOWED ticks should have passed */
                ASSERT_WARNING(ulTimeSpentSleepingInTicks < MAX_IDLE_TICKS_ALLOWED);
#endif

                system_sleeping = sys_active;
        }
}

__RETAINED_CODE static bool clk_of_periph_prevents_sleep(void)
{
        bool ret = false;
#if (dg_configPM_ENABLES_PD_COM_WHILE_ACTIVE == 1)
        /* PD COM related registers cannot be taken into account if the corresponding PD is down. */
        ASSERT_WARNING(hw_pd_check_com_status());
        /* Check if any serial peripheral is both enabled and configured to use Div1 as
         * serial input clock. (Check them altogether where possible (for efficiency) by using
         * aggregate masks.) */
        uint32 tmp = CRG_COM->CLK_COM_REG;
        uint32_t clk_sels = tmp &
                            (REG_MSK(CRG_COM, CLK_COM_REG, I2C_CLK_SEL)   |
                             REG_MSK(CRG_COM, CLK_COM_REG, SPI_CLK_SEL)   |
                             REG_MSK(CRG_COM, CLK_COM_REG, UART2_CLK_SEL) |
                             REG_MSK(CRG_COM, CLK_COM_REG, UART_CLK_SEL));
        uint32_t enables = tmp &
                           (REG_MSK(CRG_COM, CLK_COM_REG, I2C_ENABLE)   |
                            REG_MSK(CRG_COM, CLK_COM_REG, SPI_ENABLE)   |
                            REG_MSK(CRG_COM, CLK_COM_REG, UART2_ENABLE) |
                            REG_MSK(CRG_COM, CLK_COM_REG, UART_ENABLE));
        ret = (bool) ((clk_sels >> 1) & enables);
#endif /* dg_configPM_ENABLES_PD_COM_WHILE_ACTIVE */
        if (hw_pd_check_periph_status()) {
                ret |= (REG_GETF(GPADC, GP_ADC_CTRL_REG, GP_ADC_EN) && REG_GETF(CRG_PER, CLK_PER_REG, GPADC_CLK_SEL));
        }
#if (dg_configUSE_HW_PCM == 1)
        if (hw_pd_check_aud_status()) {
                uint32_t pcm_div1_active_msk = REG_MSK(CRG_AUD, PCM_DIV_REG, PCM_SRC_SEL) |
                                               REG_MSK(CRG_AUD, PCM_DIV_REG, CLK_PCM_EN);
                ret |= ((CRG_AUD->PCM_DIV_REG & pcm_div1_active_msk) == pcm_div1_active_msk);
        }
#endif
        return ret;
}
#endif /* OS_PRESENT */

__RETAINED_CODE static bool apply_wfi(bool allow_entering_sleep, uint32_t sleep_period)
{
        bool system_entered_sleep = false;
        // XTAL32M_RDY_IRQn is assumed to be in NVIC->ISER[0].
        // If not then the following code must be modified accordingly.
        ASSERT_WARNING(XTAL32M_RDY_IRQn < 32);

        // If an interrupt (other than XTAL32M_RDY_IRQn) is already pending then do not sleep.
        if ((NVIC->ISER[0] & NVIC->ISPR[0] & ~(1UL << XTAL32M_RDY_IRQn)) || (NVIC->ISER[1] & NVIC->ISPR[1])) {
#ifdef OS_PRESENT
                if (allow_entering_sleep) {
                        /*
                         * Inform Adapters about the aborted sleep because pm_system_sleeping
                         * will be left to "sys_idle" and the "wake-up" path will not be followed.
                         */
                        for (int i = dg_configPM_MAX_ADAPTERS_CNT - 1; i >= 0; i--) {
                                adapter_call_backs_t *p_Ad = adapters_cb[i];
                                if ((p_Ad != NULL) && (p_Ad->ad_sleep_canceled != NULL)) {
                                        p_Ad->ad_sleep_canceled();
                                }
                        }
                }
#endif
                DBG_SET_LOW(PWR_MGR_USE_TIMING_DEBUG, PWRDBG_SLEEP_ENTER);
                DBG_SET_HIGH(PWR_MGR_USE_TIMING_DEBUG, PWRDBG_SLEEP_EXIT);

                return false;
        }

        if (allow_entering_sleep) {
#ifdef OS_PRESENT
                /*
                 * Hooks
                 */
                if (sleep_period == 0) {
#ifdef OS_SYS_PRE_STOP_PROCESSING
                        // A user definable macro that allows application code to be added.
                        OS_SYS_PRE_STOP_PROCESSING();
#endif
                } else {
#ifdef OS_SYS_PRE_SLEEP_PROCESSING
                        // A user definable macro that allows application code to be added.
                        OS_SYS_PRE_SLEEP_PROCESSING( sleep_period );
#endif
                }
#endif /* OS_PRESENT */
                /*
                 * Mark that a wake-up interrupt will be treated as such and not as a
                 * typical interrupt.
                 */
                system_sleeping = sys_powered_down;
#if dg_configUSE_CLOCK_MGR
                /*
                 * Switch to 32MHz system clock. AHB/APB dividers are set to 1.
                 */
                cm_sys_clk_sleep(true);
#endif
                DBG_SET_LOW(PWR_MGR_FUNCTIONAL_DEBUG, PWRDBG_POWERUP);

#if (dg_configUSE_WDOG == 1)
#ifdef OS_PRESENT
                // Watchdog is always active during sleep. Check if watchdog will expire during sleep
                ASSERT_WARNING(sleep_period < (WDOG_VALUE_2_LP_CLKS(sys_watchdog_get_val())));
#else
                ASSERT_WARNING(sleep_period < (WDOG_VALUE_2_LP_CLKS(dg_configWDOG_IDLE_RESET_VALUE)));
#endif
#endif

                pm_prepare_sleep(current_sleep_mode);
                pm_set_flashes_to_sleep_mode();
        }
#ifdef OS_PRESENT
        else {

                __UNUSED bool is_any_lld_active = false;
#if (dg_configUSE_HW_DMA == 1)
                is_any_lld_active |= hw_dma_channel_active();
#endif
                /* must not change the sysclk frequency if a periph is using DIV1 */
                is_any_lld_active |= clk_of_periph_prevents_sleep();

                if (!is_any_lld_active) {
                        // Lower the clocks to reduce power consumption.
                        cm_lower_all_clocks();
                }

#ifdef OS_SYS_PRE_IDLE_PROCESSING
                /*
                 * Hook to add any application specific code before entering Idle state.
                 */
                OS_SYS_PRE_IDLE_PROCESSING( sleep_period );
#endif
        }
#endif /* OS_PRESENT */
        DBG_CONFIGURE_LOW(PWR_MGR_USE_TIMING_DEBUG, PWRDBG_SLEEP_ENTER);

#if (dg_configIMAGE_SETUP == DEVELOPMENT_MODE)
        /*
         * If the code stops at this point then the interrupts were enabled while they
         * shouldn't be so.
         */
        ASSERT_WARNING(__get_PRIMASK() == 1 || __get_BASEPRI() != 0);
#endif

        /*
         * Sleep
         */
        if (allow_entering_sleep) {

                if (NVIC_GetPendingIRQ(XTAL32M_RDY_IRQn)) {
                        /*
                         * If there is a pending XTAL32M_RDY_IRQnn clear it now to allow the system
                         * to go to sleep. When the system wakes-up, it will check if XTAL32M is
                         * settled and switch to XTAL32M without waiting for the interrupt.
                         */
                        NVIC_ClearPendingIRQ(XTAL32M_RDY_IRQn);
                }

#if (dg_configPM_ENABLES_PD_COM_WHILE_ACTIVE == 1)
                /* Disable PD COM if it is not needed during sleep */
                hw_sys_pd_com_disable();
#endif /* (dg_configPM_ENABLES_PD_COM_WHILE_ACTIVE == 1) */
                /* Disable PD PERIPH if it is not needed during sleep */
                hw_sys_pd_periph_disable();

#if dg_configUSE_BOD
                hw_bod_deactivate_channel(HW_BOD_CHANNEL_VDD);
#endif /* dg_configUSE_BOD */

                system_entered_sleep = goto_deepsleep();

                if (system_entered_sleep) {
                        /*
                         *  Apply system non-retained register configuration.
                         *  DCDC configuration must be applied immediately after wake-up to reduce
                         *  current consumption.
                         */
                        hw_sys_reg_apply_config();
                }
#ifdef OS_PRESENT
#if dg_configENABLE_DEBUGGER
                /* Detect if woken-up from JTAG and delay sleep if needed. */
                jtag_wkup_check(system_entered_sleep);
#endif
#endif
                /*
                 * If sleep was cancelled and the sleep mode was pm_mode_hibernation or
                 * pm_mode_deep_sleep, the device must be rebooted.
                 */
                if (!system_entered_sleep &&
                    (current_sleep_mode == pm_mode_hibernation ||
                     current_sleep_mode == pm_mode_deep_sleep) ) {
                        if (dg_configENABLE_DEBUGGER) {
                                ENABLE_DEBUGGER;
                        }
                        hw_cpm_reboot_system();
                }
        }
        else {
                __WFI();
                /* Make sure that the code will be executed at the fastest possible speed. */
                hw_clk_set_hclk_div(ahb_div1);
        }
#ifdef OS_PRESENT
        /* Make sure that OS system timer is not programmed to hit within the next few usec. */
        sys_timer_invalidate_trigger();

        if (!allow_entering_sleep) {
                /* Restore clocks that may have been lowered before entering IDLE. */
                cm_restore_all_clocks();
        }
#endif
        if (system_sleeping == sys_powered_down) {
                /* Enable PD PERIPH */
                hw_sys_pd_periph_enable();
#if (dg_configPM_ENABLES_PD_COM_WHILE_ACTIVE == 1)
                /* Enable PD COM */
                hw_sys_pd_com_enable();
#endif /* (dg_configPM_ENABLES_PD_COM_WHILE_ACTIVE == 1) */
        }

        DBG_SET_HIGH(PWR_MGR_USE_TIMING_DEBUG, PWRDBG_SLEEP_EXIT);

        /*
         * Initial wake-up handling of the system.
         */
        if (system_sleeping == sys_powered_down) {
#ifdef OS_PRESENT
                /*
                 * ad_wake_up_ind() and ad_xtalm_ready_ind() have not been called yet
                 */
                adapters_wake_up_ind_called = false;
                call_adapters_xtal16m_ready_ind = false;
#endif
                pm_resume_from_sleep();

#if (dg_configUSE_SYS_TCS == 1)
                /* flash is now on,so you can call TCS */
                sys_tcs_apply_reg_pairs(SYS_TCS_GROUP_PD_PER);
#if (dg_configPM_ENABLES_PD_COM_WHILE_ACTIVE == 1)
                sys_tcs_apply_reg_pairs(SYS_TCS_GROUP_PD_COMM);
#endif
                sys_tcs_apply_reg_pairs(SYS_TCS_GROUP_PD_SYS);
#endif /* dg_configUSE_SYS_TCS */
                hw_sys_set_preferred_values(HW_PD_SYS);
#if (dg_configPM_ENABLES_PD_COM_WHILE_ACTIVE == 1)
                hw_sys_set_preferred_values(HW_PD_COM);
#endif
#if dg_configPMU_ADAPTER
                /*
                 * Restore PMU levels to those chosen by the application.
                 */


                AD_PMU_RESTORE_FOR_WAKEUP();
#endif /* dg_configPMU_ADAPTER */
        }

        return system_entered_sleep;
}

#ifdef OS_PRESENT
__RETAINED_HOT_CODE void pm_execute_wfi(void)
{
        __WFI();
        __enable_irq();
}

__RETAINED_HOT_CODE void pm_sleep_enter(uint32_t low_power_periods)
{
        uint64_t rtc_time;
        uint32_t lp_current_time;
        uint32_t trigger;
        uint32_t sleep_period;
        bool allow_stopping_tick = false;
        bool allow_entering_sleep = false;
        bool abort_sleep = false;

 #if (PWR_MGR_DEBUG == 1)
        low_power_periods_ret = low_power_periods;
 #endif

#if dg_configUSE_HW_TRNG
        /* Check that TRNG service is not in the process of generating random numbers */
        if (sys_trng_producing_numbers() != 0) {
                abort_sleep = true;
        }
#endif
        /* Do not go to sleep with both LDO_IO and BYPASS_ACTIVE disabled. */
        if ( REG_GETF(CRG_TOP, POWER_CTRL_REG, LDO_IO_ENABLE) == 0 && REG_GETF(CRG_TOP, POWER_CTRL_REG, LDO_IO_BYPASS_ACTIVE) == 0 ) {
#if (dg_configIMAGE_SETUP == DEVELOPMENT_MODE)
                ASSERT_WARNING(0);
#endif
                abort_sleep = true;
        }
#if dg_configENABLE_DEBUGGER
        /* If the debugger is attached then sleep is not allowed */
        if (hw_sys_is_debugger_attached() || !jtag_wkup_delay_has_expired()) {
                abort_sleep = true;
        }
#endif /* dg_configENABLE_DEBUGGER */

        /*
         * Update Real Time Clock value
         */
        rtc_time = sys_timer_get_timestamp_fromCPM(&lp_current_time);

        /*
         * Check if sleep is allowed.
         */
        if (!abort_sleep && (current_sleep_mode != pm_mode_active)) {
                uint32_t lp_tick_offset;
                uint32_t os_sleep_time = 0;
                uint32_t rtc_offset;
                uint32_t sleep_time_reduction = 0;
                bool is_infinite = true;
                uint32_t wakeup_time = pm_get_sys_wakeup_cycles();

                if (wakeup_mode_is_XTAL32) {
                        wakeup_time += cm_get_xtalm_settling_lpcycles();
                }

                /* We plan to stop Tick */
                allow_stopping_tick = true;

                if (clk_of_periph_prevents_sleep()) {
                        allow_entering_sleep = false;
                } else if (current_sleep_mode != pm_mode_idle) {
                        /* We plan to enter sleep */
                        allow_entering_sleep = true;
                }

                if ((dg_configUSE_WDOG == 1) && !(sys_watchdog_monitor_mask_empty())) {
                        // At least one task registered to WDOG, limit low_power_periods to guarantee that WDOG will not expire during sleep
                        if (allow_entering_sleep) {
                                uint32_t wdog_period_lp_clks = WDOG_VALUE_2_LP_CLKS(sys_watchdog_get_val());

                                // Add a margin so that the system wakes up before WDOG expiration
                                if (wdog_period_lp_clks > WDOG_MARGIN) {
                                        wdog_period_lp_clks -= WDOG_MARGIN;
                                } else {
                                        wdog_period_lp_clks = 0;
                                }

                                if (wdog_period_lp_clks == 0) {
                                        // WDOG will expire soon, abort sleep
                                        allow_entering_sleep = false;
                                } else {
                                        // Limit low_power_periods
                                        if (low_power_periods > 0) {
                                                // Non-Infinite sleep request
                                                if (low_power_periods > wdog_period_lp_clks) {
                                                        low_power_periods = wdog_period_lp_clks;
                                                }
                                        } else {
                                                // Infinite sleep request
                                                low_power_periods = wdog_period_lp_clks;
                                        }
                                }
                        }
                }

                /*
                 * Compute when the earliest wake-up time is.
                 *
                 * lp_prescaled_time - lp_last_trigger : offset in this tick period (in prescaled LP
                 *         cycles, for the calculation of the sleep_period of the OS tasks, this
                 *         offset must be subtracted).
                 * rtc_time : absolute time (in LP cycles)
                 *
                 * sleep_period holds the result (in prescaled LP cycles).
                 */

                // 1. Check OS first!
                if (low_power_periods) {
                        // 1a. Offset in this tick period.
                        lp_tick_offset = sys_timer_get_tick_offset();

                        // 1c. Calculate time of wake-up as an offset of the current RTC value.
                        if (lp_tick_offset > low_power_periods) {
                                // We are already late! The tick interrupt may already be pending...
                                allow_entering_sleep = false;
                                allow_stopping_tick = false;
                        }
                        else {
                                os_sleep_time = low_power_periods - lp_tick_offset;
                        }
                        // 1d. Subtract power-up time.
                        if (allow_entering_sleep) {
                                if (os_sleep_time > wakeup_time) {
                                        sleep_time_reduction = wakeup_time;
                                        os_sleep_time -= sleep_time_reduction;
                                }
                                else {
                                        allow_entering_sleep = false;
                                }
                        }
                        // 1e. Initially, wake-up time is set for the OS case.
                        sleep_period = os_sleep_time;
                        is_infinite = false;
                }
                else {
                        // 1f. Sleep period is infinite for the OS.
                        sleep_period = -1;
                }

                if (allow_entering_sleep) {                     // Power-down is possible.
                        do {
                                // 2. Check if sleep is blocked for some short period
                                if (sleep_is_blocked) {
                                        rtc_offset = (uint32_t)(sleep_blocked_until - rtc_time);
                                        if (rtc_offset < dg_configPM_MAX_ADAPTER_DEFER_TIME) {
                                                // Still valid ==> Block is ON.
                                                allow_entering_sleep = false;
                                                break;
                                        }
                                        else {
                                                // Time has passed! Reset flag!
                                                sleep_is_blocked = false;
                                        }
                                }

                                // 3. Calculate the overhead added from the Adapters.
                                if (!is_infinite) {
                                        for (int i = 0; i < dg_configPM_MAX_ADAPTERS_CNT; i++) {
                                                adapter_call_backs_t *p_Ad = adapters_cb[i];
                                                if (p_Ad != NULL) {
                                                        if (sleep_period > p_Ad->ad_sleep_preparation_time) {
                                                                sleep_time_reduction += p_Ad->ad_sleep_preparation_time;
                                                                sleep_period -= p_Ad->ad_sleep_preparation_time;
                                                        }
                                                        else {
                                                                sleep_period = 0;
                                                                allow_entering_sleep = false;
                                                                break;
                                                        }
                                                }
                                        }
                                }
                        } while (0);
                }

                // 4. Calculate sleep_period and check if power-down is possible.
                if (is_infinite) {
                        sleep_period = 0;
                }
                else {
                         // 4b. Abort power down if sleep_period is too short!
                         if (sleep_period < dg_configMIN_SLEEP_TIME) {
                                 allow_entering_sleep = false;
                         }

                         // 4c. Restore sleep time if power-down is not possible.
                         if (!allow_entering_sleep) {
                                 /* Since the system will not be powered-down, restore the sleep time
                                  * by inverting any reductions that have been made to account for
                                  * delays relevant to power-down / wake-up.
                                  */
                                 sleep_period += sleep_time_reduction;

                                 /* If the CPU clock is too slow, wake-up earlier in order to be able
                                  * to resume the OS in time.
                                  */
                                 if (cm_cpu_clk_get_fromISR() < cpuclk_16M) {
                                         sleep_period -= pm_get_sys_wakeup_cycles();

                                 }
                         }

                         // 4d. Check if sleep period is too small!
                         if (sleep_period <= (OS_TICK_PERIOD)) {
                                 allow_stopping_tick = false;
                         }
                 }

#if (PWR_MGR_DEBUG == 1)
                 sleep_period_ret = sleep_period;
#endif
         }

         if (allow_stopping_tick) {
                 /*
                  * Mark that a wake-up interrupt will be treated as such and not as a typical interrupt
                  */
                 system_sleeping = sys_idle;

                 if (((current_sleep_mode == pm_mode_hibernation) || current_sleep_mode == pm_mode_deep_sleep)
                                 && (sleep_period == 0)) {
                         /*
                          * Interrupt is already disabled!
                          * No LP clock will be available during sleep and no wake up has been
                          * requested! The system will wake up only from an external event!
                          * No trigger will be programmed.
                          */
                 } else {
                         uint32_t lp_latest_time;
                         uint32_t max_sleep_period;

                         /*
                          * If no "deep sleep" mode and sleep_period is infinite then wake up
                          * periodically to implement RTC.
                          * If "deep sleep" and wake-up has been requested after a sleep_period then
                          * schedule the wake up.
                          * In both cases, the lp clock will stay alive to allow the timer to wake
                          * the system up in time. The lp_last_trigger already holds the last trigger
                          * that hit since the prvStopTickInterrupt() has been called!
                          */
                         if (dg_configUSE_WDOG == 1) {
                                 if (sys_watchdog_monitor_mask_empty()) {
                                         /*
                                          * Only IDLE task is registered, WDOG already counting down, set the WDOG
                                          * value to the maximum possible value to prevent it from expiring during
                                          * sleep or while waiting for an interrupt.
                                          */
                                         sys_watchdog_set_pos_val(dg_configWDOG_IDLE_RESET_VALUE);

                                         max_sleep_period = WDOG_VALUE_2_LP_CLKS(sys_watchdog_get_val()) - WDOG_MARGIN;
                                 } else {
                                         /*
                                          * At least one task registered, WDOG already counting down,
                                          * do not change the WDOG value.
                                          */
                                         max_sleep_period = sleep_period;        // Margin has already been removed.
                                 }
                         } else {
                                 /* WDG is frozen but will be activated when the PD SYS is off.
                                  * It is set to dg_configWDOG_IDLE_RESET_VALUE:
                                  * - In pm_system_init(), during system initialization
                                  * - In pm_resume_from_sleep(), during sleep exit
                                  * Therefore, it will start counting down from this value during sleep.
                                  */
                                 max_sleep_period = WDOG_VALUE_2_LP_CLKS(dg_configWDOG_IDLE_RESET_VALUE) - WDOG_MARGIN;
                         }

                         if (sleep_period == 0 || sleep_period > max_sleep_period) {
                                 sleep_period = max_sleep_period;
                         } else {
                                 /*
                                  * Configure OS timer to raise interrupt at the requested time.
                                  */
                                 rtc_time = sys_timer_get_timestamp_fromCPM(&lp_latest_time);

                                 uint32_t computational_delay = (lp_latest_time - lp_current_time) & LP_CNT_NATIVE_MASK;

                                 /* Make sure computational_delay is less than 10 prescaled cycles (else is too big!). */
                                 ASSERT_WARNING(computational_delay < 10);

                                 sleep_period -= computational_delay;
                         }

                         // Set wake-up trigger (minus any computational delays).
                         trigger = (lp_current_time + sleep_period) & LP_CNT_NATIVE_MASK;

 #if (PWR_MGR_DEBUG == 1)
                         trigger_setting_ret = trigger;
 #endif
                         sys_timer_set_trigger(trigger);
                 }

                 /**********************************************************************************/
#if dg_configUSE_HW_DMA
                 if (allow_entering_sleep) {
                         if (hw_dma_channel_active()) {
                                 allow_entering_sleep = false;
                         }
                 }
#endif

                 if (allow_entering_sleep) {
                         int i;
                         adapter_call_backs_t *p_Ad;

                         /*
                          * Inquiry Adapters about the forthcoming sleep entry
                          */

                         // 1. Inquiry Adapters
                         for (i = dg_configPM_MAX_ADAPTERS_CNT - 1; i >= 0; i--) {
                                 p_Ad = adapters_cb[i];
                                 if ((p_Ad != NULL) && (p_Ad->ad_prepare_for_sleep != NULL)) {
                                         if (!p_Ad->ad_prepare_for_sleep()) {
                                                 break;
                                         }
                                 }
                         }

                         // 2. If an Adapter rejected sleep, resume any Adapters that have already accepted it.
                         if (i >= 0) {
                                 allow_entering_sleep = false;   // Sleep has been canceled.

                                 i++;
                                 while (i < dg_configPM_MAX_ADAPTERS_CNT) {
                                         p_Ad = adapters_cb[i];
                                         if ((p_Ad != NULL) && (p_Ad->ad_sleep_canceled != NULL)) {
                                                 p_Ad->ad_sleep_canceled();
                                         }
                                         i++;
                                 }
                         }
                 }

                 apply_wfi(allow_entering_sleep, sleep_period);

                 if (system_sleeping == sys_powered_down) {
                         if (dg_configIMAGE_SETUP == DEVELOPMENT_MODE) {
                                 rtc_time = sys_timer_get_uptime_ticks_fromISR();
                         }
                 }

                 if (!allow_entering_sleep) {
#ifdef OS_SYS_POST_IDLE_PROCESSING
                         /*
                          * Hook to add any application specific code after exiting Idle state.
                          * Note: the System and the Peripheral Power Domains are active.
                          */
                         OS_SYS_POST_IDLE_PROCESSING( sleep_period );
#endif
                 }
         } else {
                 /*
                  * Make sure that it's not OS_TICK_INCREMENT() that advances time but the OS timer
                  * ISR, in this case!
                  */
                ulTimeSpentSleepingInTicks = 0;

                /*
                 * Wait for an interrupt
                 *
                 * Any interrupt will cause an exit from WFI(). This is not a problem since even
                 * if an interrupt other that the tick interrupt occurs before the next tick comes,
                 * the only thing that should be done is to resume the scheduler. Since no tick has
                 * occurred, the OS time will be the same.
                 */
                __WFI();
         }

         ASSERT_WARNING(__get_PRIMASK() == 1);

         /* Wake-up! */
         system_wake_up();
}

system_state_t pm_get_system_sleep_state(void)
{
        return system_sleeping;
}

#if dg_configENABLE_DEBUGGER


__STATIC_INLINE bool jtag_wkup_is_combo_trigger(uint32_t idx)
{
        const uint32_t MASK = HW_PDC_LUT_ENTRY_FIELD_MASK(TRIG_SELECT)
                            | HW_PDC_LUT_ENTRY_FIELD_MASK(TRIG_ID)
                            | HW_PDC_LUT_ENTRY_FIELD_MASK(PDC_MASTER);

        const uint32_t VALUE = HW_PDC_LUT_ENTRY_VAL(HW_PDC_TRIG_SELECT_PERIPHERAL,
                                                    HW_PDC_PERIPH_TRIG_ID_COMBO,
                                                    HW_PDC_MASTER_CM33, 0);

        return (hw_pdc_read_entry(idx) & MASK) == (VALUE & MASK);
}

static void jtag_wkup_discover_combo_pdc_entry_idx(void)
{
        jtag_wkup_combo_pdc_entry_idx = HW_PDC_INVALID_LUT_INDEX;

        for (int i = 0; i < HW_PDC_LUT_SIZE; ++i) {
                if (jtag_wkup_is_combo_trigger(i)) {
                        jtag_wkup_combo_pdc_entry_idx = i;
                        break;
                }
        }
}

__STATIC_FORCEINLINE bool jtag_wkup_combo_source_irq_is_pending(void)
{
        _Static_assert(CMAC2SYS_IRQn < 32, "CMAC2SYS_IRQn >= 32");
        _Static_assert(KEY_WKUP_GPIO_IRQn < 32, "KEY_WKUP_GPIO_IRQn >= 32");

        uint32_t non_jtag_combo_irq_pending = NVIC->ISPR[0]
                                                 & ( (1 << CMAC2SYS_IRQn)
                                                   | (1 << KEY_WKUP_GPIO_IRQn)
                                                   );
        return non_jtag_combo_irq_pending != 0;
}

__STATIC_FORCEINLINE bool jtag_wkup_detect(void)
{
        const bool combo_pdc_entry_exists = (jtag_wkup_combo_pdc_entry_idx
                                                != HW_PDC_INVALID_LUT_INDEX);

        const bool combo_is_pending = (combo_pdc_entry_exists
                                        && hw_pdc_is_pending(jtag_wkup_combo_pdc_entry_idx));

        return combo_is_pending && !jtag_wkup_combo_source_irq_is_pending();
}

__STATIC_FORCEINLINE void jtag_wkup_disable_sleep_delay(void)
{
        jtag_wkup_sleep_blocked_until = 0;
}

__STATIC_FORCEINLINE void jtag_wkup_set_sleep_delay_ms(uint32_t millis)
{
        const uint32_t delay_sleep_lp_cycles = (millis * OS_TICK_CLOCK_HZ) / 1000;
        const uint64_t now = sys_timer_get_uptime_ticks_fromISR();

        jtag_wkup_sleep_blocked_until = now + delay_sleep_lp_cycles;
}

__RETAINED_CODE static void jtag_wkup_check(bool sys_cpu_entered_sleep)
{
        jtag_wkup_disable_sleep_delay();

        /* If the system CPU (CM33) did not enter sleep then we are done. */
        if (!sys_cpu_entered_sleep) {
                return;
        }

        if (jtag_wkup_detect()) {
                jtag_wkup_set_sleep_delay_ms(50);
        }
}

__STATIC_FORCEINLINE bool jtag_wkup_delay_has_expired(void)
{
        uint32_t unused;
        const uint64_t now = sys_timer_get_timestamp_fromCPM(&unused);

        return (now > jtag_wkup_sleep_blocked_until);
}

#endif /* dg_configENABLE_DEBUGGER */
#else /* OS_PRESENT */
bool pm_sleep_enter_no_os(sleep_mode_t sleep_mode)
{
        bool system_slept = false;
        bool allow_entering_sleep = true;
        __disable_irq();

        DBG_SET_HIGH(PWR_MGR_USE_TIMING_DEBUG, PWRDBG_SLEEP_ENTER);

        current_sleep_mode = sleep_mode;
#if (dg_configUSE_HW_DMA == 1)
        if (hw_dma_channel_active()) {
                allow_entering_sleep = false;
        }
#endif
        system_slept = apply_wfi(allow_entering_sleep, UINT32_MAX);

        DBG_SET_LOW(PWR_MGR_USE_TIMING_DEBUG, PWRDBG_SLEEP_EXIT);

#if dg_configUSE_CLOCK_MGR
        if (system_slept) {
                uint32_t iser, iser2;

                /*
                 * Disable all IRQs except the one of XTAL32M_RDY.
                 */
                iser  = NVIC->ISER[0];                   // Log interrupt enable status (IRQs 00-31)
                iser2 = NVIC->ISER[1];                   // Log interrupt enable status (IRQs 32-63)

                /*
                 * Disable all interrupts except the one of XTAL32M
                 * XTAL32M_RDY_IRQn is assumed to be in NVIC->ICER[0].
                 * If not then the following code must be modified accordingly.
                 */
                ASSERT_WARNING(XTAL32M_RDY_IRQn < 32);

                NVIC->ICER[0] = iser & ~((uint32_t)(1 << XTAL32M_RDY_IRQn));
                NVIC->ICER[1] = iser2;

                /*
                 * If the code stops at this point then the interrupts were enabled while they
                 * shouldn't be so.
                 */
                ASSERT_WARNING(__get_PRIMASK() == 1  || __get_BASEPRI() != 0);

                cm_enable_xtalm_if_required();

                __enable_irq();

                /*
                 * Restore AHB, APB. If sys_clk is not RC32, it will be restored when the XTAL32
                 * settles.
                 */

                GLOBAL_INT_DISABLE();

                cm_sys_clk_sleep(false);

                GLOBAL_INT_RESTORE();

                cm_switch_to_xtalm_if_settled();

                if (wakeup_mode_is_XTAL32) {
                        /* Wait for XTAL32 and DBLR, if required. */
                        cm_halt_until_sysclk_ready();
                }

                /* Restore all interrupts */
                NVIC->ISER[0] = iser;
                NVIC->ISER[1] = iser2;
        }
#else
        __enable_irq();
#endif /* dg_configUSE_CLOCK_MGR */
        return system_slept;
}
#endif /* OS_PRESENT */

void pm_set_sys_wakeup_mode(sys_wakeup_mode_t mode)
{
        uint32_t val = CRG_TOP->PMU_SLEEP_REG;
        switch (mode) {
        case pm_sys_wakeup_mode_normal:
                REG_SET_FIELD(CRG_TOP, PMU_SLEEP_REG, FAST_WAKEUP, val, 0);
                break;
        case pm_sys_wakeup_mode_fast:
                /* Fast wake up mode is not supported yet */
                ASSERT_WARNING(0);
                break;
        default:
                ASSERT_WARNING(0);
        }

        CRG_TOP->PMU_SLEEP_REG = val;

#if (USE_BLE_SLEEP == 1)
        ad_ble_update_wakeup_time();
#endif
}

sys_wakeup_mode_t pm_get_sys_wakeup_mode(void)
{
        uint32_t val = CRG_TOP->PMU_SLEEP_REG;

        if (REG_GET_FIELD(CRG_TOP, PMU_SLEEP_REG, FAST_WAKEUP, val) == 1) {
                return pm_sys_wakeup_mode_fast;
        }

        return pm_sys_wakeup_mode_normal;
}

uint8_t pm_get_sys_wakeup_cycles(void)
{
        uint8_t wakeup_cycles = 0;
        uint32_t wakeup_time = 0;

        switch (pm_get_sys_wakeup_mode()) {
        case pm_sys_wakeup_mode_normal:
                wakeup_time = dg_configWAKEUP_NORMAL;
                break;
        case pm_sys_wakeup_mode_fast:
                wakeup_time = dg_configWAKEUP_FAST;
                break;
        default:
                ASSERT_WARNING(0);
        }

        wakeup_cycles = WAKEUP_TIME_2_LP_CYCLES(wakeup_time);

        return wakeup_cycles;
}

static void reset_pdc_lut_entries(sleep_mode_t mode)
{
        if (mode == pm_mode_deep_sleep) {
                hw_pdc_entry_t keep_triggers[] = {
                        {HW_PDC_TRIG_SELECT_P0_GPIO, HW_PDC_FILTER_DONT_CARE, HW_PDC_MASTER_CM33, HW_PDC_FILTER_DONT_CARE},
                        {HW_PDC_TRIG_SELECT_P1_GPIO, HW_PDC_FILTER_DONT_CARE, HW_PDC_MASTER_CM33, HW_PDC_FILTER_DONT_CARE},
                        {HW_PDC_TRIG_SELECT_PERIPHERAL, HW_PDC_PERIPH_TRIG_ID_FAST_WKUP, HW_PDC_MASTER_CM33, HW_PDC_FILTER_DONT_CARE},
                        {HW_PDC_TRIG_SELECT_PERIPHERAL, HW_PDC_PERIPH_TRIG_ID_RTC_ALARM, HW_PDC_MASTER_CM33, HW_PDC_FILTER_DONT_CARE},
                        {HW_PDC_TRIG_SELECT_PERIPHERAL, HW_PDC_PERIPH_TRIG_ID_RTC_TIMER, HW_PDC_MASTER_CM33, HW_PDC_FILTER_DONT_CARE},
                };

                hw_pdc_lut_keep_t keep_entries = {
                        .num = sizeof(keep_triggers)/sizeof(keep_triggers[0]),
                        .keep = (hw_pdc_entry_t* )keep_triggers,
                };

                hw_pdc_lut_keep(&keep_entries);
        } else if (mode == pm_mode_hibernation) {
                hw_pdc_lut_reset();
        }

}

__RETAINED_HOT_CODE void pm_prepare_sleep(sleep_mode_t sleep_mode)
{
        if (sleep_mode == pm_mode_hibernation || sleep_mode == pm_mode_deep_sleep) {
#ifdef OS_PRESENT
                // Hibernation Sleep mode! Wake-up only from external button!
                // Deep sleep mode! Wake-up only from external button! Keep RTC.
                sys_timer_stop();            // Stop OS timer
#endif

                // Switch system clock to RC32M
                /* RC32M_ENABLE does not need to be set when the sys_clk is switched to RC32M. */
                hw_clk_set_sysclk(SYS_CLK_IS_RC32);

                /*
                 * Clear all PDC entries and make sure SYS_SLEEP is 0
                 */
                REG_CLR_BIT(CRG_TOP, PMU_CTRL_REG, SYS_SLEEP);

                // reset PDC entries based on sleep mode. This function is not retained but FLASH is still active accessible in this point.
                reset_pdc_lut_entries(sleep_mode);


                // Add one PDC entry for M33 to wake-up from GPIO and set SYS_SLEEP to 1
                uint32_t pdc_entry_index = hw_pdc_add_entry( HW_PDC_LUT_ENTRY_VAL(HW_PDC_TRIG_SELECT_PERIPHERAL,
                                                                                  HW_PDC_PERIPH_TRIG_ID_COMBO,
                                                                                  HW_PDC_MASTER_CM33, 0));

                ASSERT_WARNING(pdc_entry_index != HW_PDC_INVALID_LUT_INDEX);
                hw_pdc_set_pending(pdc_entry_index);
                hw_pdc_acknowledge(pdc_entry_index);
                REG_SET_BIT(CRG_TOP, PMU_CTRL_REG, SYS_SLEEP);

                // Turn off LP clocks. Leave LP clock unchanged in pm_mode_deep_sleep for RTC
                /* RCLP must be enabled for the HW FSM to work properly. */
                hw_clk_enable_lpclk(LP_CLK_IS_RCLP);

                if (sleep_mode == pm_mode_hibernation) {
#if dg_configUSE_HW_RTC
                        hw_rtc_stop();
#endif
                        hw_clk_set_lpclk(LP_CLK_IS_RCLP);

                        if ((dg_configUSE_LP_CLK == LP_CLK_32000) ||
                            (dg_configUSE_LP_CLK == LP_CLK_32768)) {
                                hw_clk_disable_lpclk(LP_CLK_IS_XTAL32K);
                        } else if (dg_configUSE_LP_CLK == LP_CLK_RCX) {
                                hw_clk_disable_lpclk(LP_CLK_IS_RCX);
                        }
                }
                else {
                        /* Only XTAL32K or RCX can be used as LP clock in deep sleep mode */
                        ASSERT_WARNING((dg_configUSE_LP_CLK == LP_CLK_32000) ||
                                       (dg_configUSE_LP_CLK == LP_CLK_32768) ||
                                       (dg_configUSE_LP_CLK == LP_CLK_RCX));

                        if ((dg_configUSE_LP_CLK == LP_CLK_32000) ||
                            (dg_configUSE_LP_CLK == LP_CLK_32768)) {
                                hw_clk_disable_lpclk(LP_CLK_IS_RCX);
                        } else if (dg_configUSE_LP_CLK == LP_CLK_RCX) {
                                hw_clk_disable_lpclk(LP_CLK_IS_XTAL32K);
                        }
                }

                hw_sys_no_retmem();
                hw_sys_enable_reset_on_wup();

                hw_pd_power_down_tim();

                REG_CLR_BIT(CRG_TOP, PMU_SLEEP_REG, FAST_WAKEUP);

#if dg_configUSE_HW_PMU
                hw_bod_deactivate();

                if (sleep_mode == pm_mode_hibernation) {
                        /* During hibernation the rails are adjusted by the HW. */
                        hw_sys_enable_hibernation();
                } else {
                        HW_PMU_ERROR_CODE error_code = hw_pmu_1v8_onsleep_enable(HW_PMU_1V8_MAX_LDO_LOAD_2);
                        ASSERT_WARNING(error_code == HW_PMU_ERROR_NOERROR);
                        hw_pmu_vdcdc_onsleep_enable((dg_configUSE_DCDC == 1) ?
                                                     HW_PMU_VDCDC_MAX_DCDC_LOAD_0_300 :
                                                     HW_PMU_VDCDC_MAX_LDO_LOAD_1);
                        error_code = hw_pmu_vdd_onsleep_enable(HW_PMU_VDD_MAX_LOAD_0_400);
                        ASSERT_WARNING(error_code == HW_PMU_ERROR_NOERROR);
                        error_code = hw_pmu_vdd_set_voltage(HW_PMU_VDD_VOLTAGE_SLEEP_0V75);
                        ASSERT_WARNING(error_code == HW_PMU_ERROR_NOERROR);
                }
#endif
                if (dg_configENABLE_DEBUGGER) {
                        DISABLE_DEBUGGER;
                }
                CRG_TOP->SYS_CTRL_REG &= ~(0x40UL);
        }
#if dg_configPMU_ADAPTER
        else {
                AD_PMU_PREPARE_FOR_SLEEP();
        }
#endif /* dg_configPMU_ADAPTER */
}

__RETAINED_CODE void pm_set_flashes_to_sleep_mode(void)
{
#if dg_configQSPI_AUTOMODE_ENABLE
        /*
         * Put the Flash in Power-Down.
         */
        qspi_automode_flash_power_down();
#endif /* dg_configQSPI_AUTOMODE_ENABLE */

#if (dg_configREDUCE_RETAINED_CODE == 0)
        eflash_automode_sleep();
#endif
}

__RETAINED_CODE void pm_resume_from_sleep(void)
{

#if dg_configQSPI_AUTOMODE_ENABLE
        /*
         * Re-initialize QSPIC/QSPI Flash if dg_configQSPI_FLASH_POWER_DOWN is 1
         */
        qspi_automode_flash_power_up();
#endif /* dg_configQSPI_AUTOMODE_ENABLE */

#if (dg_configREDUCE_RETAINED_CODE == 0)
        /*
         * Wakeup EFLASH
         */
        eflash_automode_wakeup();
#endif
        hw_fcu_set_flash_mass_erase_time(dg_configEFLASH_MASS_ERASE_TIME);
        hw_fcu_set_flash_page_erase_time(dg_configEFLASH_PAGE_ERASE_TIME);
        hw_fcu_set_flash_program_time(dg_configEFLASH_WORD_WRITE_TIME);

#if (dg_configUSE_WDOG == 0)
        hw_watchdog_freeze();                   // Stop watchdog
        /*
         * Reload the watchdog counter to prevent it from expiring during sleep.
         */
        sys_watchdog_set_pos_val(dg_configWDOG_IDLE_RESET_VALUE);
#else
        // Restore watchdog reset value, if needed
        if (sys_watchdog_monitor_mask_empty()) {
                sys_watchdog_set_pos_val(dg_configWDOG_RESET_VALUE);
        }
#endif
}

