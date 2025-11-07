/**
 ****************************************************************************************
 *
 * @file sys_clock_mgr_da1459x.c
 *
 * @brief Clock Manager
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


#if dg_configUSE_CLOCK_MGR

#include <string.h>
#include "sdk_defs.h"
#include "sys_clock_mgr.h"
#include "sys_clock_mgr_internal.h"
#include "sys_power_mgr.h"
#include "sys_power_mgr_internal.h"
#include "sys_watchdog.h"
#include "qspi_automode.h"
#include "hw_pdc.h"
#include "hw_pd.h"
#include "hw_pmu.h"
#include "hw_sys.h"
#include "../peripherals/src/hw_sys_internal.h"
#include "hw_gpio.h"
#include "hw_rtc.h"
#if (dg_configPMU_ADAPTER == 1)
#include "../adapters/src/ad_pmu_internal.h"
#endif

#if (dg_configSYSTEMVIEW)
#include "SEGGER_SYSVIEW_FreeRTOS.h"
#else
#define SEGGER_SYSTEMVIEW_ISR_ENTER()
#define SEGGER_SYSTEMVIEW_ISR_EXIT()
#endif

#ifdef OS_PRESENT
#include "osal.h"
#include "sdk_list.h"

#include "hw_qspi.h"
//#include "sys_tcs.h"

#if (CLK_MGR_USE_TIMING_DEBUG == 1)
#pragma message "Clock manager: GPIO Debugging is on!"
#endif

#ifdef CONFIG_USE_BLE
#include "ad_ble.h"
#endif

#if (dg_configUSE_SYS_ADC == 1)
#include "sys_adc_internal.h"
#endif

#define XTAL32_AVAILABLE                1       // XTAL32M availability
#define LP_CLK_AVAILABLE                2       // LP clock availability
#endif /* OS_PRESENT */

#define RCX_MIN_HZ                      450
#define RCX_MAX_HZ                      550
/* RCX frequency range varies between 13kHz and 17kHz. RCX_MIN/MAX_TICK_CYCLES correspond to the number of
 * min and max RCX cycles respectively in a 2msec duration, which is the optimum OS tick. */
#define RCX_MIN_TICK_CYCLES             26
#define RCX_MAX_TICK_CYCLES             34

/* The number of cycles of the to-be-calibrated clock to be measured using the reference clock. */

/* The RCX clock calibration cycles at power-up */
#define RCX_CALIBRATION_CYCLES_PUP      44

/* The RCX clock calibration cycles at wake-up */
#define RCX_CALIBRATION_CYCLES_WUP      25

/* Total calibration time = N*3 msec. Increase N to get a better estimation of the frequency of
 * RCX. */
#define RCX_REPEAT_CALIBRATION_PUP      10

/* Value to be used in cm_rc_clocks_trigger_calibration. */
#define RCX_TRIG_CAL_VALUE              100

#define MAX_CALIBRATION_COUNT(x)        (dg_configDIVN_FREQ * x)

#if (CM_ENABLE_RCLP_CALIBRATION == 1)
#define RCLP_CALIBRATION_CYCLES         3
/* The number of cycles to measure must not cause a uint32_t overflow */
#if (RCLP_CALIBRATION_CYCLES > 134)
#error "RCLP_CALIBRATION_CYCLES outside limits"
#endif
#define RCLP_TARGET_FREQ                dg_configRC32K_FREQ
#define RCLP_FREQ_MARGIN                2000
#define RCLP_TRIM_LOW                   0x0
#define RCLP_TRIM_HIGH                  (REG_MSK(CRG_TOP, CLK_RCLP_REG, RCLP_TRIM) >> REG_POS(CRG_TOP, CLK_RCLP_REG, RCLP_TRIM))
/* RCLP temperature coefficient
 * The max dF/dT is 0.007Hz/degC */
#define RCLP_TEMPERATURE_DRIFT(x)       ((x * 7 * RCLP_TEMP_DRIFT_CELSIUS_X100) / 100000)
#endif /* CM_ENABLE_RCLP_CALIBRATION */

#if (CM_ENABLE_RC32M_CALIBRATION == 1)
/* ---------- #define the RC32M calibration parameters ---------------------- */
#define RC32M_CALIBRATION_CYCLES        32
#define RC32M_RECOMMENDED_BIAS          0xD
#define RC32M_TARGET_FREQ               30250000 // RC32M target frequency. RC32M allowable frequency should be < 32MHz
#define RC32M_MEAS_TARGET               (MAX_CALIBRATION_COUNT(RC32M_CALIBRATION_CYCLES)/ (RC32M_TARGET_FREQ))
#endif /* CM_ENABLE_RC32M_CALIBRATION */

/*
 * Global and / or retained variables
 */

__RETAINED uint16_t rcx_clock_hz;
__RETAINED uint8_t rcx_tick_period;                             // # of cycles in 1 tick
__RETAINED uint16_t rcx_tick_rate_hz;
__RETAINED static uint32_t rcx_clock_hz_acc;                    // Accurate RCX freq (1/RCX_ACCURACY_LEVEL accuracy)
__RETAINED static uint32_t rcx_clock_period;                    // usec multiplied by 1024 * 1024

__RETAINED uint32_t rclp_hz_slow_max;   // RCLP slow frequency(32kHz) + temperature drift



#if dg_configRTC_CORRECTION

/*
 * RTC compensation variables
 */
#define DAY_IN_USEC                     (24 * 60 * 60 * 1000 * 1000LL)
#define HDAY_IN_USEC                    (12 * 60 * 60 * 1000 * 1000LL)
#define HUNDREDTHS_OF_SEC_us            10000

__RETAINED static uint64_t rtc_usec_prev;
__RETAINED static int32_t rtc_drift_usec;
__RETAINED static uint32_t rtc_freq_acc;
__RETAINED static uint32_t rcx_freq_prev;
#endif

#define CM_SYS_CLK_REQUEST_MAX          (UINT8_MAX)
#define CM_SYS_CLK_NUM                  (3)

__RETAINED_RW static sys_clk_t sysclk = sysclk_LP;      // Invalidate system clock
__RETAINED static ahb_div_t ahbclk;
__RETAINED static apb_div_t apbclk;
/*
 * System clock priority array
 */
__RETAINED_RW static sys_clk_t sys_clk_prio[CM_SYS_CLK_NUM] = { sysclk_DBLR64,
                                                                sysclk_XTAL32M,
                                                                sysclk_RC32 };
__RETAINED static uint8_t sys_clk_reqs[CM_SYS_CLK_NUM];         // system clock requests array
__RETAINED_RW static int8_t default_sys_clk_index = -1;
#if dg_configUSE_HW_PDC
__RETAINED static uint32_t xtal32_pdc_entry;
#endif /* dg_configUSE_HW_PDC */
#if (dg_configPMU_ADAPTER == 0)
__RETAINED static HW_PMU_VDD_VOLTAGE vdd_voltage;
#endif /* dg_configPMU_ADAPTER */

__RETAINED static void (*xtal_ready_callback)(void);

static sys_clk_t sys_clk_next;

static volatile bool xtal32m_settled_notification = false;
static volatile bool xtal32m_settled = false;

#ifdef OS_PRESENT
__RETAINED static OS_MUTEX cm_mutex;
__RETAINED static OS_EVENT_GROUP xEventGroupCM_xtal;
__RETAINED static OS_TIMER xLPSettleTimer;

#if defined(OS_PRESENT)
__RETAINED static OS_TASK xRCClocksCalibTaskHandle;
#endif

#endif /* OS_PRESENT */

#define NUM_OF_CPU_CLK_CONF 5

/*
 * Forward declarations
 */
static cm_sys_clk_status_t sys_clk_set(sys_clk_t type);
static void apb_set_clock_divider(apb_div_t div);
static bool ahb_set_clock_divider(ahb_div_t div);
static int8_t find_prio_list_idx(sys_clk_t type);

#ifdef OS_PRESENT
#define CM_ENTER_CRITICAL_SECTION() OS_ENTER_CRITICAL_SECTION()
#define CM_LEAVE_CRITICAL_SECTION() OS_LEAVE_CRITICAL_SECTION()

#define CM_MUTEX_CREATE()       do {                                    \
                                        OS_ASSERT(cm_mutex == NULL);    \
                                        OS_MUTEX_CREATE(cm_mutex);      \
                                        OS_ASSERT(cm_mutex);            \
                                } while (0)
#define CM_MUTEX_GET()          do {                                                    \
                                        OS_ASSERT(cm_mutex);                            \
                                        OS_MUTEX_GET(cm_mutex, OS_MUTEX_FOREVER);       \
                                } while (0)
#define CM_MUTEX_PUT()          OS_MUTEX_PUT(cm_mutex)

#else
#define CM_ENTER_CRITICAL_SECTION() GLOBAL_INT_DISABLE()
#define CM_LEAVE_CRITICAL_SECTION() GLOBAL_INT_RESTORE()

#define CM_MUTEX_CREATE()       do {} while (0)
#define CM_MUTEX_GET()          do {} while (0)
#define CM_MUTEX_PUT()          do {} while (0)

#endif /* OS_PRESENT */

#define IS_VALID_SYSCLK(clk) (clk == sysclk_RC32 || clk == sysclk_XTAL32M || clk == sysclk_DBLR64)

/*
 * Function definitions
 */

/**
 * \brief Get the CPU clock frequency in MHz
 *
 * \param[in] clk The system clock
 * \param[in] div The HCLK divider
 *
 * \return The clock frequency
 */
static uint32_t get_clk_freq(sys_clk_t clk, ahb_div_t div)
{
        sys_clk_t clock = clk;

        if (clock == sysclk_RC32) {
                clock = sysclk_XTAL32M;
        }

        return ( 16 >> div ) * clock;
}

/**
 * \brief Lower AHB and APB clocks to the minimum frequency.
 *
 * \warning It can be called only at wake-up.
 */
__STATIC_INLINE void lower_amba_clocks(void)
{
        // Lower the AHB clock (fast --> slow clock switch)
        hw_clk_set_hclk_div((uint32_t)ahb_div16);
}

/**
 * \brief Restore AHB and APB clocks to the maximum (default) frequency.
 *
 * \warning It can be called only at wake-up.
 */
__STATIC_INLINE void restore_amba_clocks(void)
{
        // Restore the AHB clock (slow --> fast clock switch)
        hw_clk_set_hclk_div(ahbclk);
}

/**
 * \brief Switch to RC32.
 *
 * \details Set RC32 as the system clock.
 */
static void switch_to_rc32(void)
{
        hw_clk_enable_sysclk(SYS_CLK_IS_RC32);

        // fast --> slow clock switch
        hw_clk_set_sysclk(SYS_CLK_IS_RC32);     // Set RC32 as sys_clk

        /*
         * Disable RC32M. RC32M will remain enabled by the hardware as long as it is used
         * as system clock.
         */
        hw_clk_disable_sysclk(SYS_CLK_IS_RC32);
#if dg_configQSPI_AUTOMODE_ENABLE
        if (sysclk > sysclk_XTAL32M) {
                qspi_automode_sys_clock_cfg(sysclk_RC32);
        }
#endif /* dg_configQSPI_AUTOMODE_ENABLE */
}

/**
 * \brief Switch to XTAL32M.
 *
 * \details Sets the XTAL32M as the system clock.
 *
 * \warning It does not block. It assumes that the caller has made sure that the XTAL32M has
 *          settled.
 */
static void switch_to_xtal32m(void)
{
        if (hw_clk_get_sysclk() != SYS_CLK_IS_XTAL32M) {
                ASSERT_WARNING(hw_clk_is_xtalm_started());

                hw_clk_set_sysclk(SYS_CLK_IS_XTAL32M);          // Set XTAL32 as sys_clk
#if dg_configQSPI_AUTOMODE_ENABLE
                if (sysclk > sysclk_XTAL32M) {                 // slow --> fast clock switch
                        qspi_automode_sys_clock_cfg(sysclk_XTAL32M);
                }
#endif /* dg_configQSPI_AUTOMODE_ENABLE */
        }
}

/**
 * \brief Disable Doubler
 *
 * \details Restore VDD voltage to 0.9V if required.
 */
static void disable_dblr(void)
{
        if (hw_clk_is_enabled_sysclk(SYS_CLK_IS_DBLR)) {
                hw_clk_disable_sysclk(SYS_CLK_IS_DBLR);
                // VDD voltage can be lowered since Doubler is not the system clock anymore
#if (dg_configPMU_ADAPTER == 1)
                ad_pmu_1v2_force_max_voltage_release();
#else
                if (vdd_voltage != HW_PMU_VDD_VOLTAGE_1V20) {
                        HW_PMU_ERROR_CODE error_code;
                        error_code = hw_pmu_vdd_set_voltage(vdd_voltage);
                        ASSERT_WARNING(error_code == HW_PMU_ERROR_NOERROR);
                }
#endif /* dg_configPMU_ADAPTER */
                DBG_SET_LOW(CLK_MGR_USE_TIMING_DEBUG, CLKDBG_DBLR_ON);
        }
}

/**
 * \brief Enable Doubler
 *
 * \details Changes the VDD voltage to 1.2V if required.
 */
static void enable_dblr(void)
{
        if (hw_clk_is_dblr_ready()) {
                return;
        }
        else if (hw_clk_is_enabled_sysclk(SYS_CLK_IS_DBLR) == false) {
#if (dg_configPMU_ADAPTER == 1)
                ad_pmu_1v2_force_max_voltage_request();
#else
                HW_PMU_VDD_RAIL_CONFIG rail_config;
                hw_pmu_get_vdd_active_config(&rail_config);

                // DBLR cannot be powered by retention LDO
                ASSERT_WARNING(rail_config.current == HW_PMU_VDD_MAX_LOAD_20);

                vdd_voltage = rail_config.voltage;
                if (vdd_voltage != HW_PMU_VDD_VOLTAGE_1V20) {
                        /* VDD voltage must be set to 1.2V prior to switching clock to DBLR */
                        HW_PMU_ERROR_CODE error_code;
                        /* A VDCDC voltage of at least 1.4V is required. */
                        error_code = hw_pmu_vdcdc_set_voltage(HW_PMU_VDCDC_VOLTAGE_1V40);
                        ASSERT_WARNING(error_code == HW_PMU_ERROR_NOERROR);
                        error_code = hw_pmu_vdd_set_voltage(HW_PMU_VDD_VOLTAGE_1V20);
                        ASSERT_WARNING(error_code == HW_PMU_ERROR_NOERROR);
                }
#endif /* dg_configPMU_ADAPTER */
                hw_clk_enable_sysclk(SYS_CLK_IS_DBLR);           // Turn on DBLR
                DBG_SET_HIGH(CLK_MGR_USE_TIMING_DEBUG, CLKDBG_DBLR_ON);
        }
}

/**
 * \brief Switch to Doubler.
 *
 * \details Waits until the Doubler has locked and sets it as the system clock.
 */
static void switch_to_dblr(void)
{
        if (hw_clk_get_sysclk() == SYS_CLK_IS_XTAL32M) {
#if dg_configQSPI_AUTOMODE_ENABLE
                // Slow --> fast clock switch
                qspi_automode_sys_clock_cfg(sysclk_DBLR64);
#endif /* dg_configQSPI_AUTOMODE_ENABLE */

                /*
                 * If ultra-fast wake-up mode is used, make sure that the startup state
                 * machine is finished and all power regulation is in order.
                 */
                while (REG_GETF(CRG_TOP, SYS_STAT_REG, POWER_IS_UP) == 0);

                hw_clk_set_sysclk(SYS_CLK_IS_DBLR);                   // Set DBLR as sys_clk
        }
}

#ifdef OS_PRESENT

/**
 * \brief The handler of the XTAL32K LP settling timer.
 */
static void vLPTimerCallback(OS_TIMER pxTimer)
{
        OS_ENTER_CRITICAL_SECTION();                            // Critical section
        if ((dg_configLP_CLK_SOURCE == LP_CLK_IS_ANALOG) &&
                ((dg_configUSE_LP_CLK == LP_CLK_32000) || (dg_configUSE_LP_CLK == LP_CLK_32768))) {
                hw_clk_set_lpclk(LP_CLK_IS_XTAL32K);            // Set XTAL32K as the LP clock
                // Restore the RCLP mode. It was set to 32KHz to allow the RTOS tick to hit at the right interval.
                hw_clk_set_rclp_mode(RCLP_DEFAULT);
        }

#ifdef CONFIG_USE_BLE
        // Inform ble adapter about the availability of the LP clock.
        ad_ble_lpclock_available();
#endif

        OS_LEAVE_CRITICAL_SECTION();                            // Exit critical section

        // Inform (blocked) Tasks about the availability of the LP clock.
        OS_EVENT_GROUP_SET_BITS(xEventGroupCM_xtal, LP_CLK_AVAILABLE);

        // Stop the Timer.
        OS_TIMER_STOP(xLPSettleTimer, OS_TIMER_FOREVER);
}

/**
 * \brief Handle the indication that the XTAL32M has settled.
 *
 */
static OS_BASE_TYPE xtal32m_is_ready(OS_BASE_TYPE *xHigherPriorityTaskWoken)
{
        OS_BASE_TYPE xResult = OS_FAIL;

        if (xtal32m_settled_notification == false) {
                // Do not send the notification twice
                xtal32m_settled_notification = true;

                DBG_SET_HIGH(CLK_MGR_USE_TIMING_DEBUG, CLKDBG_XTAL32M_SETTLED);
                if (xtal_ready_callback) {
                        xtal_ready_callback();
                }

                if (xEventGroupCM_xtal != NULL) {
                        // Inform blocked Tasks
                        *xHigherPriorityTaskWoken = OS_FALSE;           // Must be initialized to OS_FALSE

                        xResult = OS_EVENT_GROUP_SET_BITS_FROM_ISR_NO_YIELD(xEventGroupCM_xtal, XTAL32_AVAILABLE,
                                                            xHigherPriorityTaskWoken);
                }

                DBG_SET_LOW(CLK_MGR_USE_TIMING_DEBUG, CLKDBG_XTAL32M_SETTLED);
        }
        return xResult;
}

#endif /* OS_PRESENT */

/**
 * \brief Calculates the optimum tick rate and the number of LP cycles (RCX) per tick.
 *
 * \param[in] freq The RCX clock frequency (in Hz).
 * \param[out] tick_period The number of LP cycles per tick.
 *
 * \return uint32_t The optimum tick rate.
 */
static uint32_t get_optimum_tick_rate(uint16_t freq, uint8_t *tick_period)
{
        uint32_t optimum_rate = 0;
        int tick;
        int err = 65536;
        int res;

        for (tick = RCX_MIN_TICK_CYCLES; tick <= RCX_MAX_TICK_CYCLES; tick++) {
                uint32_t hz = 2 * freq / tick;
                hz = (hz & 1) ? hz / 2 + 1 : hz / 2;

                if ((hz >= RCX_MIN_HZ) && (hz <= RCX_MAX_HZ)) {
                        res = hz * tick * 65536 / freq;
                        res -= 65536;
                        if (res < 0) {
                                res *= -1;
                        }
                        if (res < err) {
                                err = res;
                                optimum_rate = hz;
                                *tick_period = tick;
                        }
                }
        }

        return optimum_rate;
}

void cm_enable_xtalm_if_required(void)
{
        if (sysclk != sysclk_RC32) {
                cm_enable_xtalm();
        }
}

uint32_t cm_get_xtalm_settling_lpcycles(void)
{
        if (sysclk == sysclk_RC32) {
                return 0;
        }

#if (dg_configUSE_LP_CLK == LP_CLK_RCX)
        return XTALRDY_CYCLES_TO_LP_CLK_CYCLES(hw_clk_get_xtalm_settling_time(), rcx_clock_hz);
#else
        return XTALRDY_CYCLES_TO_LP_CLK_CYCLES(hw_clk_get_xtalm_settling_time(), dg_configXTAL32K_FREQ);
#endif
}

#if dg_configUSE_HW_PDC

static uint32_t get_pdc_xtal32m_entry(void)
{
        uint32_t entry = HW_PDC_INVALID_LUT_INDEX;
#ifdef OS_PRESENT
        // Search for the RTOS timer entry
        entry = hw_pdc_find_entry(HW_PDC_TRIG_SELECT_PERIPHERAL, HW_PDC_PERIPH_TRIG_ID_TIMER2,
                                        HW_PDC_MASTER_CM33, HW_PDC_LUT_ENTRY_EN_XTAL, 0);
        if (entry != HW_PDC_INVALID_LUT_INDEX) {
                return entry;
        }
#endif
        // Search for any entry that will wake-up M33 and start the XTAL32M
        entry = hw_pdc_find_entry(HW_PDC_FILTER_DONT_CARE, HW_PDC_FILTER_DONT_CARE,
                                        HW_PDC_MASTER_CM33, HW_PDC_LUT_ENTRY_EN_XTAL, 0);
        return entry;
}
#endif

void cm_enable_xtalm(void)
{
        GLOBAL_INT_DISABLE();

#if dg_configUSE_HW_PDC
        if (xtal32_pdc_entry == HW_PDC_INVALID_LUT_INDEX) {
                // Find a PDC entry for enabling XTAL32M
                xtal32_pdc_entry = get_pdc_xtal32m_entry();

                if (xtal32_pdc_entry == HW_PDC_INVALID_LUT_INDEX) {
                        // If no PDC entry exists, add a new entry for enabling the XTAL32M
                        xtal32_pdc_entry = hw_pdc_add_entry( HW_PDC_TRIGGER_FROM_MASTER( HW_PDC_MASTER_CM33,
                                                                                  HW_PDC_LUT_ENTRY_EN_XTAL ) );
                }

                ASSERT_WARNING(xtal32_pdc_entry != HW_PDC_INVALID_LUT_INDEX);

                // XTAL32M may not been started. Use PDC to start it.
                hw_pdc_set_pending(xtal32_pdc_entry);
                hw_pdc_acknowledge(xtal32_pdc_entry);

                // Clear the XTAL32M_XTAL_ENABLE bit to allow PDC to disable XTAL32M when going to sleep.
                hw_clk_disable_sysclk(SYS_CLK_IS_XTAL32M);
        }
#endif

        xtal32m_settled = hw_clk_is_xtalm_started();

        if (xtal32m_settled == false) {
                if (hw_clk_is_enabled_sysclk(SYS_CLK_IS_XTAL32M) == false) {
#if dg_configUSE_HW_PDC
                        // XTAL32M has not been started. Use PDC to start it.
                        hw_pdc_set_pending(xtal32_pdc_entry);
                        hw_pdc_acknowledge(xtal32_pdc_entry);

#else
                        // PDC is not used. Enable XTAL32M by setting XTAL32M_XTAL_ENABLE bit
                        // in XTAL32M_CTRL_REG
                        hw_clk_enable_sysclk(SYS_CLK_IS_XTAL32M);
#endif
                }
        }

        GLOBAL_INT_RESTORE();
}


void cm_clk_init_low_level_internal(void)
{
        NVIC_ClearPendingIRQ(XTAL32M_RDY_IRQn);
        NVIC_EnableIRQ(XTAL32M_RDY_IRQn);                      // Activate XTAL32 Ready IRQ

        hw_clk_xtalm_irq_enable();

        /*
         * Low power clock
         */
        hw_clk_enable_lpclk(LP_CLK_IS_RCLP);
        hw_clk_set_lpclk(LP_CLK_IS_RCLP);

        ASSERT_WARNING(REG_GETF(CRG_TOP, SYS_STAT_REG, TIM_IS_UP));

        if (dg_configXTAL32M_SETTLE_TIME_IN_USEC != 0) {

                uint16_t rdy_cnt = XTAL32M_USEC_TO_250K_CYCLES(dg_configXTAL32M_SETTLE_TIME_IN_USEC);

                hw_clk_set_xtalm_settling_time(rdy_cnt/8, false);
        }

        if (dg_configLP_CLK_SOURCE == LP_CLK_IS_DIGITAL) {
                /* Store PD COM state and restore it after configuring P1_14 */
                bool com_is_up = hw_pd_check_com_status();
                if (!com_is_up) {
                        hw_pd_power_up_com();
                }
                hw_clk_configure_ext32k_pins();                 // Configure Ext32K pins
                hw_gpio_pad_latch_enable(HW_GPIO_PORT_1, HW_GPIO_PIN_14);
                hw_gpio_pad_latch_disable(HW_GPIO_PORT_1, HW_GPIO_PIN_14);
                if (!com_is_up) {
                        hw_pd_power_down_com();
                }
                hw_clk_disable_lpclk(LP_CLK_IS_XTAL32K);        // Disable XTAL32K
                hw_clk_disable_lpclk(LP_CLK_IS_RCX);            // Disable RCX
                hw_clk_set_lpclk(LP_CLK_IS_EXTERNAL);           // Set EXTERNAL as the LP clock
        } else if (dg_configUSE_LP_CLK == LP_CLK_RCX) {
                hw_clk_enable_lpclk(LP_CLK_IS_RCX);             // Enable RCX
                hw_clk_disable_lpclk(LP_CLK_IS_XTAL32K);        // Disable XTAL32K
                // LP clock will be switched to RCX after RCX calibration
        } else if ((dg_configUSE_LP_CLK == LP_CLK_32000) ||
                   (dg_configUSE_LP_CLK == LP_CLK_32768)) {
                // Slow mode of RCLP must be selected to allow the RTOS tick to hit properly
                hw_clk_set_rclp_mode(RCLP_FORCE_SLOW);
                // No need to configure XTAL32K pins. Pins are automatically configured
                // when LP_CLK_IS_XTAL32K is enabled.
                hw_clk_configure_lpclk(LP_CLK_IS_XTAL32K);      // Configure XTAL32K
                hw_clk_enable_lpclk(LP_CLK_IS_XTAL32K);         // Enable XTAL32K
                hw_clk_disable_lpclk(LP_CLK_IS_RCX);            // Disable RCX
                // LP clock cannot be set to XTAL32K here. XTAL32K needs a few seconds to settle after power up.
        } else {
                ASSERT_WARNING(0);                              // Should not be here!
        }

#if dg_configUSE_HW_PDC
        xtal32_pdc_entry = HW_PDC_INVALID_LUT_INDEX;
#endif

}

#if (CM_ENABLE_RCLP_CALIBRATION == 1)
void cm_calibrate_rclp(void)
{
        uint32_t rclp_clock_hz_slow;
        uint8_t  rclp_trim;
        bool     rclp_trim_decreased = false;

        /* Keep RCLP mode */
        rclp_mode_t rclp_mode = hw_clk_get_rclp_mode();
        /* Set RCLP at low speed mode */
        /* Trim value should be calculated at 32kHz */
        hw_clk_set_rclp_mode(RCLP_FORCE_SLOW);
        /* Wait 3 RCLP@32k cycles for frequency to settle properly */
        hw_clk_delay_usec(90);
        /* Keep current trim value */
        rclp_trim = REG_GETF(CRG_TOP, CLK_RCLP_REG, RCLP_TRIM);

        while (1) {
                /* Run calibration process */
                hw_clk_start_calibration(CALIBRATE_RCLP, CALIBRATE_REF_DIVN, RCLP_CALIBRATION_CYCLES);
                uint32_t cal_value = hw_clk_get_calibration_data();

                /* Process calibration results and calculate RCLP frequency */
                rclp_clock_hz_slow = MAX_CALIBRATION_COUNT(RCLP_CALIBRATION_CYCLES) / cal_value;

                if (rclp_clock_hz_slow > RCLP_TARGET_FREQ) {
                        /* Frequency too high, decrease it */
                        if (rclp_trim > RCLP_TRIM_LOW) {
                                rclp_trim--;
                                REG_SETF(CRG_TOP, CLK_RCLP_REG, RCLP_TRIM, rclp_trim);
                        } else {
                                /* Reached the limit. Nothing better is possible */
                                break;
                        }
                        rclp_trim_decreased = true;
                } else if (rclp_clock_hz_slow < (RCLP_TARGET_FREQ - RCLP_FREQ_MARGIN)) {
                        if (!rclp_trim_decreased) {
                                /* Frequency too low, increase it if there was no decrease step */
                                if (rclp_trim < RCLP_TRIM_HIGH) {
                                        rclp_trim++;
                                        REG_SETF(CRG_TOP, CLK_RCLP_REG, RCLP_TRIM, rclp_trim);
                                } else {
                                        /* Reached the limit. Nothing better is possible */
                                        break;
                                }
                        } else {
                                break;
                        }
                } else {
                        /* All done */
                        break;
                }
        }


        /* Calculate rclp slow max frequency */
        uint32_t rclp_slow_drift = RCLP_TEMPERATURE_DRIFT(rclp_clock_hz_slow);
        rclp_hz_slow_max = rclp_clock_hz_slow + rclp_slow_drift;

        /* Restore RCLP mode of operation*/
        hw_clk_set_rclp_mode(rclp_mode);
}
#endif /* CM_ENABLE_RCLP_CALIBRATION */

#if (CM_ENABLE_RC32M_CALIBRATION == 1)
void cm_calibrate_rc32m(void)
{
        uint32_t m_range_0_del_4 = 0;
        uint32_t m_range_1_del_4 = 0;
        uint32_t m_trimmed = 0;
        uint32_t m_last = 0;
        uint32_t d_last = 0;
        uint32_t d_trimmed = 0;

        /* Set the recommended bias setting for the RC32M oscillator */
        REG_SETF(CRG_TOP, CLK_RC32M_REG, RC32M_BIAS, RC32M_RECOMMENDED_BIAS);


        /*      Coarse trimming: RC32M__RANGE (range) = 0, 1, or 2
         *
         *      The fine tuning of the RC32M will be done after the right coarse
         *      band has been selected. Two frequency measurements are done to
         *      determine the proper coarse band. The measurements are done on
         *      the lower two coarse bands while the fine tuning is set to 3/4
         *      of the highest band frequency setting.
         *
         *      Procedure:
         *          Measure the frequency for (range, del) = (0, 4) and (1, 4)
         *          and check whether the frequency is larger than that of the
         *          target frequency.
         *
         *          Next select the proper coarse trimming band.
         *
         */
        REG_SETF(CRG_TOP, CLK_RC32M_REG, RC32M_COSC, 4);
        REG_SETF(CRG_TOP, CLK_RC32M_REG, RC32M_RANGE, 0);

        hw_clk_start_calibration(CALIBRATE_RC32M, CALIBRATE_REF_DIVN, RC32M_CALIBRATION_CYCLES);
        m_range_0_del_4 = hw_clk_get_calibration_data();

        REG_SETF(CRG_TOP, CLK_RC32M_REG, RC32M_RANGE, 1);
        hw_clk_start_calibration(CALIBRATE_RC32M, CALIBRATE_REF_DIVN, RC32M_CALIBRATION_CYCLES);
        m_range_1_del_4 = hw_clk_get_calibration_data();

        /* Select and program the proper coarse trimming */
        uint8_t range = 0;
        if (m_range_0_del_4 > RC32M_MEAS_TARGET) {
                range++;
        }
        if (m_range_1_del_4 > RC32M_MEAS_TARGET) {
                range++;
        }

        REG_SETF(CRG_TOP, CLK_RC32M_REG, RC32M_RANGE, range);

        /*
         *      Fine trimming procedure:
         *
         *      Use binary search to find the frequency closest to the target
         *      frequency. So start in the middle (= 8) of the range.
         *
         *      Note that only if m_trimmed is not equal to RC32M_MEAS_TARGET the
         *      value of RC32M_COSC must be updated and the loop could be exited.
         *      However, for the sake of a fixed pattern length this will not
         *      be done.
         *
         */
        REG_SETF(CRG_TOP, CLK_RC32M_REG, RC32M_COSC, 8);

        uint8_t previous = 0;

        for (int8_t i = 2; i >= 0; i--) {

            previous = REG_GETF(CRG_TOP, CLK_RC32M_REG, RC32M_COSC);
            hw_clk_start_calibration(CALIBRATE_RC32M, CALIBRATE_REF_DIVN, RC32M_CALIBRATION_CYCLES);
            m_trimmed = hw_clk_get_calibration_data();

            if (m_trimmed > RC32M_MEAS_TARGET) {
                    uint8_t tmp = previous - (1 << i);
                    REG_SETF(CRG_TOP, CLK_RC32M_REG, RC32M_COSC, tmp);
            } else if (m_trimmed < RC32M_MEAS_TARGET) {
                    uint8_t tmp = previous + (1 << i);
                    REG_SETF(CRG_TOP, CLK_RC32M_REG, RC32M_COSC, tmp);
            }
        }

        /*
         *      Notice that with the final step in the loop a last measured needs
         *      to be done.
         *
         *      The last step in the loop could have increased the distance from
         *      the target. In that case the previous setting should be selected.
         *      Otherwise, update m_trimmed with the final measurement: m_last.
         */

        hw_clk_start_calibration(CALIBRATE_RC32M, CALIBRATE_REF_DIVN, RC32M_CALIBRATION_CYCLES);
        m_last = hw_clk_get_calibration_data();

        /* Find the distance of the target frequency */
        if (m_last > RC32M_MEAS_TARGET) {
                d_last = m_last - RC32M_MEAS_TARGET;
        } else {
                d_last = RC32M_MEAS_TARGET - m_last;
        }

        /* Find the distance of the target frequency */
        if (m_trimmed > RC32M_MEAS_TARGET) {
                d_trimmed = m_trimmed - RC32M_MEAS_TARGET;
        } else {
                d_trimmed = RC32M_MEAS_TARGET - m_trimmed;
        }

        if (d_last > d_trimmed) {
                REG_SETF(CRG_TOP, CLK_RC32M_REG, RC32M_COSC, previous);
        }
}
#endif /* CM_ENABLE_RC32M_CALIBRATION */

static void cm_calculate_rcx_accurate(uint8_t iterations, uint8_t cycles)
{
        if (iterations > 1) {
                uint32_t hz_value = 0;
                for (int i = 0; i < iterations; i++) {
                        hw_clk_start_calibration(CALIBRATE_RCX, CALIBRATE_REF_DIVN, cycles);
                        uint32_t cal_value = hw_clk_get_calibration_data();

                        // Process calibration results
                        uint64_t max_clk_count = (uint64_t) MAX_CALIBRATION_COUNT(cycles) * RCX_ACCURACY_LEVEL;
                        hz_value += (uint32_t) (max_clk_count / cal_value);
                }
                /* Avoid rounding errors due to division by 2 in the calculation that follows */
                ASSERT_WARNING((iterations & 0x01) == 0);

                /* Accurate frequency */
                rcx_clock_hz_acc = (hz_value + (iterations >> 1)) / iterations;
        } else {
                hw_clk_start_calibration(CALIBRATE_RCX, CALIBRATE_REF_DIVN, cycles);
                uint32_t cal_value = hw_clk_get_calibration_data();
                uint64_t max_clk_count = (uint64_t) MAX_CALIBRATION_COUNT(cycles) * RCX_ACCURACY_LEVEL;

                /* Accurate frequency */
                rcx_clock_hz_acc = (max_clk_count + (cal_value >> 1)) / cal_value;
        }
        rcx_clock_hz     = rcx_clock_hz_acc / RCX_ACCURACY_LEVEL;
        rcx_tick_rate_hz = get_optimum_tick_rate(rcx_clock_hz, &rcx_tick_period);
        rcx_clock_period = (uint32_t)((RC_PERIOD_DIVIDEND * RCX_ACCURACY_LEVEL) / rcx_clock_hz_acc);
}

void cm_rcx_init(void)
{
        // Run a dummy calibration to make sure the clock has settled
        hw_clk_start_calibration(CALIBRATE_RCX, CALIBRATE_REF_DIVN, 25);
        hw_clk_get_calibration_data();

        cm_calculate_rcx_accurate(RCX_REPEAT_CALIBRATION_PUP, RCX_CALIBRATION_CYCLES_PUP);
}

uint32_t cm_get_rcx_clock_hz_acc(void)
{
        return rcx_clock_hz_acc;
}

__RETAINED_CODE uint32_t cm_get_rcx_clock_period(void)
{
        return rcx_clock_period;
}

void cm_update_rtc_divider(uint16_t freq)
{
        uint16_t div_int = freq / 100;
        uint16_t div_frac = 10 * (freq - (div_int * 100));
        hw_rtc_clk_config(RTC_DIV_DENOM_1000, div_int, div_frac);
#if dg_configRTC_CORRECTION
        /* Reconstruct the frequency that created the 100Hz RTC clock */
        rtc_freq_acc = (div_int * 100 + div_frac / 10) * RCX_ACCURACY_LEVEL;
#endif
}

void cm_sys_clk_init(sys_clk_t type)
{
#ifdef OS_PRESENT
        CM_MUTEX_CREATE();                              // Create Mutex

        xEventGroupCM_xtal = OS_EVENT_GROUP_CREATE();   // Create Event Group
        ASSERT_WARNING(xEventGroupCM_xtal != NULL);
#endif
        ahbclk = cm_ahb_get_clock_divider();
        apbclk = cm_apb_get_clock_divider();

        sys_clk_next = type;

        ASSERT_WARNING(IS_VALID_SYSCLK(type));              // Not Applicable!

        HW_PMU_VDD_RAIL_CONFIG rail_config;
        hw_pmu_get_vdd_active_config(&rail_config);
#if (dg_configPMU_ADAPTER == 0)
        vdd_voltage = rail_config.voltage;
#endif /* dg_configPMU_ADAPTER */

        /*
         * Disable RC32M. RC32M will remain enabled by the hardware as long as it is used
         * as system clock.
         */
        hw_clk_disable_sysclk(SYS_CLK_IS_RC32);

        CM_ENTER_CRITICAL_SECTION();
        if (sys_clk_next == sysclk_RC32) {
                if (hw_clk_get_sysclk() != SYS_CLK_IS_RC32) {
                        // RC32 is not the System clock
                        switch_to_rc32();
                }
        }
        else {
                cm_enable_xtalm();

                /*
                 * Note: In case that the LP clock is the XTAL32K then we
                 *       simply set the cm_sysclk to the user setting and skip waiting for the
                 *       XTAL32M to settle. In this case, the system clock will be set to the
                 *       XTAL32M (or the DBLR) when the XTAL32M_RDY_IRQn hits. Every task or Adapter
                 *       must block until the requested system clock is available. Sleep may have to
                 *       be blocked as well.
                 */
                if (cm_poll_xtalm_ready()) {
                        switch_to_xtal32m();

                        hw_clk_disable_sysclk(SYS_CLK_IS_RC32);

                        if (sys_clk_next == sysclk_DBLR64) {
                                if (!hw_clk_is_dblr_ready()) {
                                        // Enable DBLR and switch when ready
                                        enable_dblr();
                                        cm_wait_dblr_ready();
                                }
                                switch_to_dblr();
                        }
                        else {
                                disable_dblr();
                        }
                }
        }

        sysclk = sys_clk_next;

        CM_MUTEX_GET();
        default_sys_clk_index = find_prio_list_idx(sysclk);
        CM_MUTEX_PUT();

        CM_LEAVE_CRITICAL_SECTION();
}

static void cm_sys_enable_xtalm(sys_clk_t type)
{
        if (type >= sysclk_XTAL32M) {
                cm_enable_xtalm();

                // Make sure the XTAL32M has settled
                cm_wait_xtalm_ready();
        }
}

static void sys_enable_dblr(void)
{
        enable_dblr();

        cm_wait_dblr_ready();
}

static int8_t find_prio_list_idx(sys_clk_t type)
{
        uint8_t index;
        for (index = 0; index < CM_SYS_CLK_NUM; index++) {
                if (type == sys_clk_prio[index]) {
                        return index;
                }
        }
        /* Should never reach here */
        ASSERT_WARNING(0);
        return default_sys_clk_index;
}

static cm_sys_clk_status_t cm_sys_clk_update(void)
{
        uint8_t clk_next_index = 0;

        for (clk_next_index = 0; clk_next_index < CM_SYS_CLK_NUM; clk_next_index++) {
                if (sys_clk_reqs[clk_next_index] > 0) {
                        break;
                }
        }

        if (clk_next_index == CM_SYS_CLK_NUM) {
                clk_next_index = default_sys_clk_index;
        }

        cm_sys_enable_xtalm(sys_clk_prio[clk_next_index]);

        if (sys_clk_prio[clk_next_index] == sysclk_DBLR64) {
                sys_enable_dblr();
        }

        cm_sys_clk_status_t ret = sys_clk_set(sys_clk_prio[clk_next_index]);

        if (sysclk != sysclk_DBLR64) {
                disable_dblr();
        }

        return ret;

}
void cm_sys_clk_set_priority(sys_clk_t *sys_clk_prio_array)
{
        /* In order to change the system clocks priorities cm_sys_clk_set_priority()
         * should be called prior to cm_sys_clk_init() */
        ASSERT_WARNING(default_sys_clk_index < 0);

        for (uint8_t i = 0; i < CM_SYS_CLK_NUM; i++) {
                /* elements should be < sysclk_LP */
                ASSERT_WARNING(IS_VALID_SYSCLK(sys_clk_prio_array[i]));
                for (uint8_t j = 0; j < CM_SYS_CLK_NUM; j++) {
                        /* elements should be unique */
                        ASSERT_WARNING((sys_clk_prio_array[i] != sys_clk_prio_array[j]) || (i == j));
                }
        }
        memcpy(sys_clk_prio, sys_clk_prio_array, sizeof(sys_clk_prio));
}

cm_sys_clk_status_t cm_sys_clk_request(sys_clk_t type)
{
        if (!IS_VALID_SYSCLK(type)) {
                return cm_sysclk_invalid_clock;
        }

        int8_t clk = find_prio_list_idx(type);

        if (sys_clk_reqs[clk] > CM_SYS_CLK_REQUEST_MAX) {
                return cm_sysclk_max_requested;
        }

        CM_MUTEX_GET();

        sys_clk_reqs[clk]++;
        cm_sys_clk_status_t ret = cm_sys_clk_update();

        if ((ret == cm_sysclk_success) && sysclk != type) {
                ret = cm_sysclk_higher_prio_used;
        }
        CM_MUTEX_PUT();

        return ret;
}

cm_sys_clk_status_t cm_sys_clk_release(sys_clk_t type)
{
        if (!IS_VALID_SYSCLK(type)) {
                return cm_sysclk_invalid_clock;
        }

        int8_t clk = find_prio_list_idx(type);

        if (sys_clk_reqs[clk] == 0) {
                return cm_sysclk_not_requested;
        }

        CM_MUTEX_GET();

        sys_clk_reqs[clk]--;
        cm_sys_clk_status_t ret = cm_sys_clk_update();

        CM_MUTEX_PUT();

        return ret;
}

#define CHECK_PER_DIV1_CLK(val, per) ((val & REG_MSK(CRG_COM, CLK_COM_REG, per ## _ENABLE)) && \
                                      (val & REG_MSK(CRG_COM, CLK_COM_REG, per ## _CLK_SEL)))

/**
 * \brief Check if div1 clock is used by a peripheral
 *
 * \return true if div1 is used by a peripheral
 */
static bool sys_clk_check_div1(void)
{
        uint32_t tmp;

        // Check if SysTick is ON and if it is affected
        if (dg_configABORT_IF_SYSTICK_CLK_ERR) {
                if (SysTick->CTRL & SysTick_CTRL_ENABLE_Msk) {
                        return true;
                }
        }

        // Check if peripherals are clocked by DIV1 clock

        if (hw_pd_check_com_status()) {
                tmp = CRG_COM->CLK_COM_REG;

                // Check SPI clock
                if (CHECK_PER_DIV1_CLK(tmp, SPI)) {
                        return true;
                }

                // Check I2C clock
                if (CHECK_PER_DIV1_CLK(tmp, I2C)) {
                        return true;
                }

                // Check UART clock
                if (CHECK_PER_DIV1_CLK(tmp, UART)) {
                        return true;
                }

                // Check UART2 clock
                if (CHECK_PER_DIV1_CLK(tmp, UART2)) {
                        return true;
                }
        }

        if (hw_pd_check_periph_status()) {
                // Check GPADC
                if (REG_GETF(GPADC, GP_ADC_CTRL_REG, GP_ADC_EN) && REG_GETF(CRG_PER, CLK_PER_REG, GPADC_CLK_SEL)) {
                        return true;
                }

#if (dg_configUSE_HW_PCM == 1) /* Checking for compile time option to use HW PCM (Audio) block.
                                  No need to check for the dg_configSYS_AUDIO_MGR too.
                                  If the Audio manager is enabled it will require the dg_configUSE_HW_PCM
                                  to be enabled too */

                /* Check if the PCM (Audio) subsystem is powered-up */
                if (hw_pd_check_aud_status()) {
                        /* Check PCM clock if the AUdio block is powered-up */
                        tmp = CRG_AUD->PCM_DIV_REG;
                        if ((tmp & REG_MSK(CRG_AUD, PCM_DIV_REG, CLK_PCM_EN)) &&
                                        (tmp & REG_MSK(CRG_AUD, PCM_DIV_REG, PCM_SRC_SEL))) {
                                /* The Audio block is using DIV1.
                                 * Return true to prevent switching
                                 * of the system clock
                                 */
                                return true;
                        }
                }
#endif
        }

        if (hw_pd_check_tim_status()) {
                // Check CMAC
                tmp = CRG_TOP->CLK_RADIO_REG;
                if ((tmp & REG_MSK(CRG_TOP, CLK_RADIO_REG, CMAC_CLK_ENABLE)) &&
                                 (tmp & REG_MSK(CRG_TOP, CLK_RADIO_REG, CMAC_CLK_SEL))) {
                        return true;
                }
        }
        return false;
}


static cm_sys_clk_status_t sys_clk_set(sys_clk_t type)
{
        cm_sys_clk_status_t ret;

        CM_ENTER_CRITICAL_SECTION();

        if (type != sysclk && sys_clk_check_div1()) {
                ret = cm_sysclk_div1_clk_in_use;
        }
        else {
                ret = cm_sysclk_success;

                if (type != sysclk) {
                        sys_clk_next = type;

                        switch (sys_clk_next) {
                        case sysclk_DBLR64:
                                if (sysclk == sysclk_RC32) {
                                        // Transition from RC32M to DBLR is not allowed.
                                        // Switch to XTAL32M first.
                                        switch_to_xtal32m();
                                }
                                switch_to_dblr();
                                break;
                        case sysclk_RC32:
                                if (sysclk == sysclk_DBLR64) {
                                        // Transition from DBLR to RC32 is not allowed.
                                        // Switch to XTAL32M first.
                                        switch_to_xtal32m();
                                }
                                switch_to_rc32();
                                break;
                        case sysclk_XTAL32M:
                                switch_to_xtal32m();
                                break;
                        default:
                                ASSERT_WARNING(0);
                                break;
                        }
                        sysclk = sys_clk_next;
                }
        }

        CM_LEAVE_CRITICAL_SECTION();

        return ret;
}

void cm_apb_set_clock_divider(apb_div_t div)
{
        CM_MUTEX_GET();
        apb_set_clock_divider(div);
        CM_MUTEX_PUT();
}

static void apb_set_clock_divider(apb_div_t div)
{
        hw_clk_set_pclk_div(div);
        apbclk = div;
}

bool cm_ahb_set_clock_divider(ahb_div_t div)
{
        CM_MUTEX_GET();
        bool ret = ahb_set_clock_divider(div);
        CM_MUTEX_PUT();

        return ret;
}

static bool ahb_set_clock_divider(ahb_div_t div)
{
        bool ret = true;

        CM_ENTER_CRITICAL_SECTION();

        do {
                if (ahbclk == div) {
                        break;
                }

                // Check if SysTick is ON and if it is affected
                if (dg_configABORT_IF_SYSTICK_CLK_ERR) {
                        if (SysTick->CTRL & SysTick_CTRL_ENABLE_Msk) {
                                ret = false;
                                break;
                        }
                }

                hw_clk_set_hclk_div(div);
                ahbclk = div;

        } while (0);

        CM_LEAVE_CRITICAL_SECTION();

        return ret;
}

sys_clk_t cm_sys_clk_get(void)
{
        sys_clk_t clk;

        CM_MUTEX_GET();
        CM_ENTER_CRITICAL_SECTION();

        clk = cm_sys_clk_get_fromISR();

        CM_LEAVE_CRITICAL_SECTION();
        CM_MUTEX_PUT();

        return clk;
}

sys_clk_t cm_sys_clk_get_fromISR(void)
{
        switch (hw_clk_get_sysclk()) {
        case SYS_CLK_IS_RC32:
                return sysclk_RC32;

        case SYS_CLK_IS_XTAL32M:
                return sysclk_XTAL32M;

        case SYS_CLK_IS_DBLR:
                return sysclk_DBLR64;
        default:
                ASSERT_WARNING(0);
                return sysclk_RC32;
        }
}

apb_div_t cm_apb_get_clock_divider(void)
{
        CM_MUTEX_GET();
        apb_div_t clk = (apb_div_t)hw_clk_get_pclk_div();
        CM_MUTEX_PUT();

        return clk;
}

ahb_div_t cm_ahb_get_clock_divider(void)
{
        ahb_div_t clk;

        CM_MUTEX_GET();
        CM_ENTER_CRITICAL_SECTION();                            // Critical section

        clk = (ahb_div_t)hw_clk_get_hclk_div();

        CM_LEAVE_CRITICAL_SECTION();                            // Exit critical section
        CM_MUTEX_PUT();
        return clk;
}

cpu_clk_t cm_cpu_clk_get(void)
{
        sys_clk_t curr_sysclk = cm_sys_clk_get();
        ahb_div_t curr_ahbclk = cm_ahb_get_clock_divider();

        return (cpu_clk_t)get_clk_freq(curr_sysclk, curr_ahbclk);
}

#ifdef OS_PRESENT

cpu_clk_t cm_cpu_clk_get_fromISR(void)
{
        sys_clk_t curr_sysclk = cm_sys_clk_get_fromISR();
        ahb_div_t curr_ahbclk = hw_clk_get_hclk_div();

        return (cpu_clk_t)get_clk_freq(curr_sysclk, curr_ahbclk);
}
#endif /* OS_PRESENT */

/**
 * \brief Interrupt handler of the XTAL32M_RDY_IRQn.
 *
 */
void XTAL32M_Ready_Handler(void)
{
        SEGGER_SYSTEMVIEW_ISR_ENTER();

        DBG_SET_HIGH(CLK_MGR_USE_TIMING_DEBUG, CLKDBG_XTAL32M_READY);

        /* The settling time for XTAL32M may vary from time to time, depending on the crystal used,
         * the off time (how long the XTAL32M has remained disabled before enabling it) and also on
         * other conditions (environment etc). Normally these variations are taken into account by
         * configuring a high enough maximum expected settling time, but there might be cases, were
         * these expectations are exceeded. Thus, wait a bit to make sure that XTAL32M has settled
         * before proceeding with switching the sys_clk. */
        while (!hw_clk_is_xtalm_started()) {
                /* If the delay is too long, there will be a problem. Thus, we should normally not
                 * enter here. If we happen to do so, increasing the maximum expected settling time
                 * should be considered. This means that: */
                if (dg_configXTAL32M_SETTLE_TIME_IN_USEC == 0) {
                        /* The margin added for the maximum expected settling time in
                         * hw_clk_xtalm_configure_irq() should be increased. Alternatively, a fixed
                         * maximum expected settling time can be applied by setting
                         * dg_configXTAL32M_SETTLE_TIME_IN_USEC to a sufficiently high value. */
                } else {
                        /* dg_configXTAL32M_SETTLE_TIME_IN_USEC should be set to a higher value. */
                }
                ASSERT_WARNING(0);
        }

        xtal32m_settled = true;

        if (sysclk != sysclk_LP) {
                // Restore system clocks. xtal32m_rdy_cnt is updated in cm_sys_clk_sleep()
                GLOBAL_INT_DISABLE();
                cm_sys_clk_sleep(false);
                GLOBAL_INT_RESTORE();

#ifdef OS_PRESENT
                if (xEventGroupCM_xtal != NULL) {
                        OS_BASE_TYPE xHigherPriorityTaskWoken, xResult;

                        xResult = xtal32m_is_ready(&xHigherPriorityTaskWoken);

                        if (xResult != OS_FAIL) {
                                /*
                                 * If xHigherPriorityTaskWoken is now set to pdTRUE then a context
                                 * switch should be requested.
                                 */
                                if ( xHigherPriorityTaskWoken != OS_FALSE ) {
                                        OS_TASK_YIELD_FROM_ISR();
                                }
                        }
                }
#endif /* OS_PRESENT */
        }

        DBG_SET_LOW(CLK_MGR_USE_TIMING_DEBUG, CLKDBG_XTAL32M_READY);

        SEGGER_SYSTEMVIEW_ISR_EXIT();
}

void cm_wait_xtalm_ready(void)
{
#ifdef OS_PRESENT
        ASSERT_WARNING(xEventGroupCM_xtal != NULL);

        if (!xtal32m_settled) {
                // Do not go to sleep while waiting for XTAL32M to settle.
                pm_sleep_mode_request(pm_mode_idle);
                OS_EVENT_GROUP_WAIT_BITS(xEventGroupCM_xtal,
                                XTAL32_AVAILABLE,
                                OS_EVENT_GROUP_FAIL,            // Don't clear bit after ret
                                OS_EVENT_GROUP_OK,              // Wait for all bits
                                OS_EVENT_GROUP_FOREVER);        // Block forever

                /* If we get here, XTAL32 must have settled */
                ASSERT_WARNING(xtal32m_settled == true);
                pm_sleep_mode_release(pm_mode_idle);
        }
#else
        cm_halt_until_xtalm_ready();
#endif /* OS_PRESENT */
}

void cm_wait_dblr_ready(void)
{
        while (!hw_clk_is_dblr_ready());
}

__RETAINED_HOT_CODE void cm_halt_until_sysclk_ready(void)
{
        if (sysclk != sysclk_RC32) {
                cm_halt_until_xtalm_ready();
        }

        if (sysclk == sysclk_DBLR64) {
                cm_wait_dblr_ready();
                switch_to_dblr();
        }
}

#ifdef OS_PRESENT

uint32_t cm_rcx_us_2_lpcycles(uint32_t usec)
{
        /* Can only convert up to 4095 usec */
        ASSERT_WARNING(usec < 4096);

        return ((usec << 20) / rcx_clock_period) + 1;
}

uint32_t cm_rcx_us_2_lpcycles_low_acc(uint32_t usec)
{
        return ((1 << 20) / (rcx_clock_period / usec)) + 1;
}

void cm_rc_clocks_calibration_notify(uint32_t rc_do_cal_mask)
{
        OS_TASK_NOTIFY(xRCClocksCalibTaskHandle, rc_do_cal_mask, OS_NOTIFY_SET_BITS);
}

#if dg_configRTC_CORRECTION

extern void hw_rtc_register_correction_cb(void (*cb)(const hw_rtc_time_t *time));

static uint64_t cm_get_usec_from_rtc_time(const hw_rtc_time_t *time)
{
        uint64_t usec;

        usec = ((((time->hour * 60 + time->minute) * 60) + time->sec) * 1000 + time->hsec *10)*1000LL;
        if (time->hour_mode && time->pm_flag) {
                usec += HDAY_IN_USEC;
        }
        return usec;
}

static void cm_rtc_callback(const hw_rtc_time_t *time)
{
        rtc_usec_prev = cm_get_usec_from_rtc_time(time);
        rcx_freq_prev = rcx_clock_hz_acc;
        rtc_drift_usec = 0;
        cm_update_rtc_divider(rcx_clock_hz);
}
/**
 * \brief Apply compensation value to RTC.
 *
 * \p This function takes as input the new hundredths-of-seconds value.
 *
 * \param [in] new_hos value for the field hundredths-of-seconds of RTC.
 *
 * \note This function deals only with hundredths of seconds, nothing bigger than that.
 *
 */
static void cm_apply_rtc_compensation_hos(uint8_t new_hos)
{
        hw_rtc_time_stop();
        uint32_t reg = RTC->RTC_TIME_REG;
        REG_SET_FIELD(RTC, RTC_TIME_REG, RTC_TIME_H_U, reg, (new_hos % 10));
        REG_SET_FIELD(RTC, RTC_TIME_REG, RTC_TIME_H_T, reg, (new_hos / 10));
        RTC->RTC_TIME_REG = reg;
        hw_rtc_time_start();
}

/**
 * \brief Calculate RTC's compensation value and apply it.
 *        It accumulates the RTC drift in usec in the global RETAINED `rtc_drift_usec`
 *
 * \parampin]     rtc_freq    the accurate frequency stemming from the 100HZ clock divisor
 * \param[in]     start_freq  the accurate RCX clock frequency in the beginning of the drift window
 * \param[in]     end_freq  the accurate RCX clock frequency in the end of the drift window
 * \param[in/out] start_usec  the interval starting point - updated after each call
 *
 * \note This function compensates up to hundredths of seconds.
 *
 * \warning Must be called with interrupts disabled.
 *
 */
static void cm_rtc_compensation(uint32_t rtc_freq, uint32_t start_freq, uint32_t end_freq, uint64_t *start_usec)
{
        hw_rtc_time_t current_time;
        uint64_t curr_usec;
        uint32_t usec_delta_rtc, usec_delta_act, abs_usec_correction;
        uint32_t mean_freq;
        uint8_t hos_drift, rtc_time_hundredths, new_rtc_time_hundredths;
        int8_t hos_overflown = 0;
        bool rtc_is_rushing;

        /*
         * Synchronize compensation process with RCX's rising edge.
         * Wait until Timer2 val changes. This happens in every RCX's rising edge.
         */
        uint32_t val = REG_GETF(TIMER2, TIMER2_TIMER_VAL_REG , TIM_TIMER_VALUE);
        while ( REG_GETF(TIMER2, TIMER2_TIMER_VAL_REG , TIM_TIMER_VALUE) == val );

        hw_rtc_get_time_clndr(&current_time, NULL);
        curr_usec = cm_get_usec_from_rtc_time(&current_time);

        if (curr_usec >= *start_usec) {
                usec_delta_rtc = curr_usec - *start_usec;
        } else {
                usec_delta_rtc = (DAY_IN_USEC + curr_usec) - *start_usec;
        }

        /* Renew the drift-window starting point */
        *start_usec = curr_usec;

        mean_freq = (start_freq + end_freq) / 2;
        /* Equal frequencies: no new drift */
        if (rtc_freq == mean_freq) {
                return;
        }

        usec_delta_act = (uint32_t)(((uint64_t)usec_delta_rtc * (uint64_t)mean_freq) / (uint64_t)rtc_freq);

        /* Cumulative drift */
        rtc_drift_usec += (int32_t)usec_delta_act - (int32_t)usec_delta_rtc; // RCX time - RTC time

        if (rtc_drift_usec > 0 ) {
                rtc_is_rushing = true;
                abs_usec_correction = (uint32_t) rtc_drift_usec;
        } else {
                rtc_is_rushing = false;
                abs_usec_correction = (uint32_t) ((-1) * rtc_drift_usec);
        }

        /* The accumulated drift is too small, wait for the next interval */
        if (abs_usec_correction < HUNDREDTHS_OF_SEC_us) {
                return;
        }

        hos_drift = abs_usec_correction / HUNDREDTHS_OF_SEC_us;
        /* RTC's hos should not have changed yet */
        rtc_time_hundredths = current_time.hsec;

        if (rtc_is_rushing) {
                /* Must reduce HOS in RTC to compensate */
                if (rtc_time_hundredths < hos_drift) {
                        /* Renew HOS is only possible without affecting higher order values */
                        new_rtc_time_hundredths = 0;
                        /* Positive overflow */
                        hos_overflown = hos_drift - rtc_time_hundredths;
                } else {
                        new_rtc_time_hundredths = rtc_time_hundredths - hos_drift;
                }
        } else {
                /* Must advance HOS in RTC to compensate */
                if (rtc_time_hundredths + hos_drift > 99) {
                        /* Renew HOS is only possible without affecting higher order values */
                        new_rtc_time_hundredths = 99;
                        /* Negative overflow */
                        hos_overflown = 99 - (rtc_time_hundredths + hos_drift);
                } else {
                        new_rtc_time_hundredths = rtc_time_hundredths + hos_drift;
                }
        }

        /* Adjust RTC time */
        cm_apply_rtc_compensation_hos(new_rtc_time_hundredths);

        /* Reset the drift-window starting point */
        current_time.hsec = new_rtc_time_hundredths;
        *start_usec = cm_get_usec_from_rtc_time(&current_time);

        /* Keep the remaining usec for next interval */
        rtc_drift_usec = rtc_drift_usec % HUNDREDTHS_OF_SEC_us + hos_overflown * HUNDREDTHS_OF_SEC_us;
}

#endif /* dg_configRTC_CORRECTION */

#if (dg_configUSE_SYS_ADC == 1)
/**
 * \brief RC clocks Calibration Task function.
 *
 * \param [in] pvParameters ignored.
 */
static OS_TASK_FUNCTION(rc_clocks_calibration_task, pvParameters )
{

        uint32_t ulNotifiedValue;
        OS_BASE_TYPE xResult __UNUSED;
        int8_t wdog_id;


#if dg_configRTC_CORRECTION
        /* Initialize RTC correction */
        hw_rtc_register_correction_cb(cm_rtc_callback);
#endif

        /* Register task to be monitored by watchdog */
        wdog_id = sys_watchdog_register(false);

        while (1) {
                /* Notify watch dog on each loop since there's no other trigger for this */
                sys_watchdog_notify(wdog_id);

                /* Suspend monitoring while task is blocked on OS_TASK_NOTIFY_WAIT() */
                sys_watchdog_suspend(wdog_id);

                // Wait for the internal notifications.
                xResult = OS_TASK_NOTIFY_WAIT(OS_TASK_NOTIFY_NONE, OS_TASK_NOTIFY_ALL_BITS,
                                              &ulNotifiedValue, OS_TASK_NOTIFY_FOREVER);
                OS_ASSERT(xResult == OS_OK);

                /* Resume watch dog monitoring */
                sys_watchdog_notify_and_resume(wdog_id);

#if (dg_configUSE_LP_CLK == LP_CLK_RCX)
                if (ulNotifiedValue & RCX_DO_CALIBRATION) {

                        DBG_SET_HIGH(SYS_ADC_DEBUG, SYS_ADC_DBG_RC_CLOCK_CALIBRATION);
                        OS_ENTER_CRITICAL_SECTION();

                        cm_calculate_rcx_accurate(1, RCX_CALIBRATION_CYCLES_WUP);
#ifdef CONFIG_USE_BLE
#if (USE_BLE_SLEEP == 1)
                        /*
                         * Notify CMAC about the new values of:
                         *   rcx_clock_period
                         *   rcx_clock_hz_acc
                         */
                        ad_ble_update_rcx();
#endif /* (USE_BLE_SLEEP == 1) */
#endif /* CONFIG_USE_BLE */

                        OS_LEAVE_CRITICAL_SECTION();
                        DBG_SET_LOW(SYS_ADC_DEBUG, SYS_ADC_DBG_RC_CLOCK_CALIBRATION);
                }

# if dg_configRTC_CORRECTION
                if (ulNotifiedValue & RCX_RTC_DO_COMPENSATION) {
                        OS_ENTER_CRITICAL_SECTION();

                        cm_rtc_compensation(rtc_freq_acc, rcx_freq_prev, rcx_clock_hz_acc, &rtc_usec_prev);
                        rcx_freq_prev = rcx_clock_hz_acc;
                        if (rtc_freq_acc / RCX_ACCURACY_LEVEL != rcx_clock_hz) {
                                cm_update_rtc_divider(rcx_clock_hz);
                        }

                        OS_LEAVE_CRITICAL_SECTION();
                }
# endif /* dg_configRTC_CORRECTION */
#endif /* (dg_configUSE_LP_CLK == LP_CLK_RCX) */
#if (CM_ENABLE_RCLP_CALIBRATION == 1)
                if (ulNotifiedValue & RCLP_DO_CALIBRATION) {

                        DBG_SET_HIGH(SYS_ADC_DEBUG, SYS_ADC_DBG_RC_CLOCK_CALIBRATION);
                        OS_ENTER_CRITICAL_SECTION();
                        cm_calibrate_rclp();
                        OS_LEAVE_CRITICAL_SECTION();
                        DBG_SET_LOW(SYS_ADC_DEBUG, SYS_ADC_DBG_RC_CLOCK_CALIBRATION);
                }
#endif
#if (CM_ENABLE_RC32M_CALIBRATION == 1)
                if (ulNotifiedValue & RC32M_DO_CALIBRATION) {

                        DBG_SET_HIGH(SYS_ADC_DEBUG, SYS_ADC_DBG_RC_CLOCK_CALIBRATION);
                        OS_ENTER_CRITICAL_SECTION();
                        cm_calibrate_rc32m();
                        OS_LEAVE_CRITICAL_SECTION();
                        DBG_SET_LOW(SYS_ADC_DEBUG, SYS_ADC_DBG_RC_CLOCK_CALIBRATION);
                }
#endif
        }
}

void cm_rc_clocks_calibration_task_init(void)
{
        /* Start the task that will handle the calibration calculations,
         * which require ~80usec@32MHz to complete. */

        OS_BASE_TYPE status;

        // Create the RC clocks calibration task
        status = OS_TASK_CREATE("RC_clocks_cal",                // The text name of the task.
                                rc_clocks_calibration_task,     // The function that implements the task.
                                ( void * ) NULL,                // No parameter is passed to the task.
                                OS_MINIMAL_TASK_STACK_SIZE,     // The size of the stack to allocate.
                                OS_TASK_PRIORITY_NORMAL,        // The priority assigned to the task.
                                xRCClocksCalibTaskHandle);      // The task handle is required.

        OS_ASSERT(status == OS_OK);
}
#endif /* dg_configUSE_SYS_ADC */

/**
 * \brief Create and start the timer and block sleep while the low power clock is settling.
 *
 * \details It starts the timer that blocks system from sleeping for
 *          dg_configXTAL32K_SETTLE_TIME. This is needed when the XTAL32K is used to make sure
 *          that the clock has settled properly before going back to sleep again.
 */
static void lp_clk_timer_create_start(void)
{
        /* Create the timer. */
        xLPSettleTimer = OS_TIMER_CREATE("LPSet",
                                OS_MS_2_TICKS(dg_configXTAL32K_SETTLE_TIME),
                                OS_TIMER_FAIL,          // Run once
                                (void *) 0,             // Timer id == none
                                vLPTimerCallback);      // Call-back
        OS_ASSERT(xLPSettleTimer != NULL);

        /* Start the timer.  No block time is specified, and even if one was
         it would be ignored because the RTOS scheduler has not yet been
         started. */
        if (OS_TIMER_START(xLPSettleTimer, 0) != OS_TIMER_SUCCESS) {
                // The timer could not be set into the Active state.
                OS_ASSERT(0);
        }
}

void cm_lp_clk_init(void)
{
        CM_MUTEX_GET();

        if (dg_configUSE_LP_CLK == LP_CLK_32000 || dg_configUSE_LP_CLK == LP_CLK_32768) {
                lp_clk_timer_create_start();
        } else {
                // No need to wait for LP clock
                OS_EVENT_GROUP_SET_BITS(xEventGroupCM_xtal, LP_CLK_AVAILABLE);
        }

        CM_MUTEX_PUT();
}

bool cm_lp_clk_is_avail(void)
{
        ASSERT_WARNING(xEventGroupCM_xtal != NULL);

        return (OS_EVENT_GROUP_GET_BITS(xEventGroupCM_xtal) & LP_CLK_AVAILABLE);
}

__RETAINED_CODE bool cm_lp_clk_is_avail_fromISR(void)
{
        ASSERT_WARNING(xEventGroupCM_xtal != NULL);

        return (OS_EVENT_GROUP_GET_BITS_FROM_ISR(xEventGroupCM_xtal) & LP_CLK_AVAILABLE);
}

void cm_wait_lp_clk_ready(void)
{
        ASSERT_WARNING(xEventGroupCM_xtal != NULL);

        OS_EVENT_GROUP_WAIT_BITS(xEventGroupCM_xtal,
                LP_CLK_AVAILABLE,
                OS_EVENT_GROUP_FAIL,                            // Don't clear bit after ret
                OS_EVENT_GROUP_OK,                              // Wait for all bits
                OS_EVENT_GROUP_FOREVER);                        // Block forever
}

void cm_lp_clk_wakeup(void)
{
        ASSERT_WARNING(xEventGroupCM_xtal != NULL);

        OS_EVENT_GROUP_CLEAR_BITS_FROM_ISR(xEventGroupCM_xtal, LP_CLK_AVAILABLE);
}
#endif /* OS_PRESENT */

/*
 * Functions intended to be used only by the Clock and Power Manager or in hooks.
 */
__RETAINED_CODE static void apply_lowered_clocks(sys_clk_t new_sysclk, ahb_div_t new_ahbclk)
{

        // First the system clock
        if (new_sysclk != sysclk) {
                sys_clk_next = new_sysclk;

                // fast --> slow clock switch
                hw_clk_set_sysclk(SYS_CLK_IS_XTAL32M);                  // Set XTAL32 as sys_clk
        }
        // else cm_sysclk is RC32 as in all other cases it is set to XTAL32M.

        // Then the AHB clock
        if (new_ahbclk != ahbclk) {
                hw_clk_set_hclk_div(new_ahbclk);
        }
}

__RETAINED_CODE void cm_lower_all_clocks(void)
{
        DBG_SET_HIGH(CLK_MGR_USE_TIMING_DEBUG, CLKDBG_LOWER_CLOCKS);

        sys_clk_t new_sysclk;
        ahb_div_t new_ahbclk = ahb_div1;

#ifdef OS_PRESENT
        // Cannot lower clocks if the first calibration has not been completed.
        if ((dg_configLP_CLK_SOURCE == LP_CLK_IS_ANALOG) && (dg_configUSE_LP_CLK == LP_CLK_RCX) && !cm_lp_clk_is_avail_fromISR()) {
                return;
        }
#endif /* OS_PRESENT */

        // Check which is the lowest system clock that can be used.
        do {
                new_sysclk = sysclk;

                // Check XTAL32 has settled.
                if (!xtal32m_settled) {
                        break;
                }

                switch (sysclk) {
                case sysclk_RC32:
                        // fall-through
                case sysclk_XTAL32M:
                        // unchanged: new_sysclk = cm_sysclk
                        break;
                case sysclk_DBLR64:
                        new_sysclk = sysclk_XTAL32M;
                        break;

                case sysclk_LP:
                        // fall-through
                default:
                        // should never reach this point
                        ASSERT_WARNING(0);
                }
        } while (0);

        if (!xtal32m_settled) {
                new_ahbclk = ahb_div16;                               // Use 2MHz AHB clock.
        } else {
                new_ahbclk = ahb_div8;                                // Use 4Mhz AHB clock.
        }

        // Check if the SysTick is ON and if it is affected
        if ((dg_configABORT_IF_SYSTICK_CLK_ERR) && (SysTick->CTRL & SysTick_CTRL_ENABLE_Msk)) {
                if ((new_sysclk != sysclk) || (new_ahbclk != ahbclk)) {
                        /*
                         * This is an application error! The SysTick should not run with any of the
                         * sleep modes active! This is because the OS may decide to go to sleep
                         * because all tasks are blocked and nothing is pending, although the
                         * SysTick is running.
                         */
                        new_sysclk = sysclk;
                        new_ahbclk = ahbclk;
                }
        }

        apply_lowered_clocks(new_sysclk, new_ahbclk);
}

__RETAINED_CODE void cm_restore_all_clocks(void)
{
#ifdef OS_PRESENT
        if ((dg_configLP_CLK_SOURCE == LP_CLK_IS_ANALOG) && (dg_configUSE_LP_CLK == LP_CLK_RCX) && !cm_lp_clk_is_avail_fromISR()) {
                return;
        }
#endif /* OS_PRESENT */
        // Set the AMBA High speed Bus clock (slow --> fast clock switch)
        if (ahbclk != (ahb_div_t)hw_clk_get_hclk_div()) {
                hw_clk_set_hclk_div(ahbclk);
        }

        // Set the system clock (slow --> fast clock switch)
        if (xtal32m_settled && (sysclk != sysclk_RC32)) {
                sys_clk_next = sysclk;

                if (sysclk >= sysclk_DBLR64) {
                        hw_clk_set_sysclk(SYS_CLK_IS_DBLR);           // Set DBLR as sys_clk
                } else {
                        hw_clk_set_sysclk(SYS_CLK_IS_XTAL32M);       // Set XTAL32 as sys_clk
                }
        }
        DBG_SET_LOW(CLK_MGR_USE_TIMING_DEBUG, CLKDBG_LOWER_CLOCKS);
}

#ifdef OS_PRESENT
void cm_wait_xtalm_ready_fromISR(void)
{
        if (!xtal32m_settled) {
                while (NVIC_GetPendingIRQ(XTAL32M_RDY_IRQn) == 0);
                xtal32m_settled = true;
                cm_switch_to_xtalm_if_settled();
                NVIC_ClearPendingIRQ(XTAL32M_RDY_IRQn);
        }
}

#endif /* OS_PRESENT */

__RETAINED_CODE bool cm_poll_xtalm_ready(void)
{
        return xtal32m_settled;
}

void cm_halt_until_xtalm_ready(void)
{
        while (!xtal32m_settled) {
                GLOBAL_INT_DISABLE();
                /* System is waking up.
                 * We need to protect the __WFI from the case where the XTAL_RDY IRQ
                 * happened and served after entering the loop and before disabling the interrupts.
                 * Not doing so, the __WFI remains blocked, and if / when it wakes up from
                 * the HW_TIMER2 IRQ (or attaching the debugger), it will be too late and the missing
                 * OS event will cause an assertion.
                 */
                DBG_CONFIGURE_LOW(CMN_TIMING_DEBUG, CMNDBG_CRITICAL_SECTION);
                if (!xtal32m_settled) {
                        lower_amba_clocks();
                        __WFI();
                        restore_amba_clocks();
                }
                GLOBAL_INT_RESTORE();
        }
}

void cm_register_xtal_ready_callback(void (*cb)(void))
{
        xtal_ready_callback = cb;
}

/**
 * \brief Switch to XTAL32M - Interrupt Safe version.
 *
 * \detail Waits until the XTAL32M has settled and sets it as the system clock.
 *
 * \warning It is called from Interrupt Context.
 */
static void switch_to_xtal_safe(void)
{
        cm_halt_until_xtalm_ready();
        hw_clk_set_sysclk(SYS_CLK_IS_XTAL32M);  // Set XTAL32 as sys_clk
}

__RETAINED_HOT_CODE void cm_sys_clk_sleep(bool entering_sleep)
{
        if (entering_sleep) {
                // Sleep entry : No need to switch to RC32. PDC will do it.

                if (sysclk == sysclk_DBLR64) {
                        if (hw_clk_get_sysclk() == SYS_CLK_IS_DBLR) {
                                // Transition from DBLR to RC32 is not allowed.
                                // Switch to XTAL32M first.
                                switch_to_xtal32m();
                        }
                        // No need to disable RC32M. It is already disabled.
                        disable_dblr();
                }

                if (sysclk != sysclk_RC32) {
#if dg_configUSE_HW_PDC
                        if (xtal32_pdc_entry == HW_PDC_INVALID_LUT_INDEX) {
                                switch_to_rc32();
                                hw_clk_disable_sysclk(SYS_CLK_IS_XTAL32M);
                        }
#else
                        switch_to_rc32();
                        hw_clk_disable_sysclk(SYS_CLK_IS_XTAL32M);
#endif
                }

                // Make sure that the AHB and APB busses are clocked at 32MHz.
                if (ahbclk != ahb_div1) {
                        // slow --> fast clock switch
                        hw_clk_set_hclk_div(ahb_div1);                  // cm_ahbclk is not altered!
                }
                hw_clk_set_pclk_div(apb_div1);                          // cm_apbclk is not altered!
        }
        else {
                /*
                 * XTAL32M ready: transition to the cm_sysclk, cm_ahbclk and cm_apbclk that were set
                 * by the user.
                 *
                 * Note that when the system wakes up the system clock is RC32 and the AHB / APB are
                 * clocked at highest frequency (because this is what the setting was just before
                 * sleep entry).
                 */

                sys_clk_t tmp_sys_clk;

#if (USE_BLE_SLEEP == 1)

#endif
                if ((sysclk != sysclk_RC32) && xtal32m_settled) {
                        tmp_sys_clk = sysclk;

                        if (hw_clk_get_sysclk() == SYS_CLK_IS_RC32) {
                                sys_clk_next = sysclk_XTAL32M;
                                sysclk = sysclk_RC32;               // Current clock is RC32
                                switch_to_xtal_safe();
                                sysclk = sys_clk_next;

                                sys_clk_next = tmp_sys_clk;
                        }

                        if (sys_clk_next == sysclk_DBLR64) {
                                if (hw_clk_is_dblr_ready()) {
                                        switch_to_dblr();
                                }
                                else {
                                        // Enable DBLR and switch when ready
                                        enable_dblr();
                                        cm_wait_dblr_ready();
                                        switch_to_dblr();
                                }
                        }
                        sysclk = sys_clk_next;
                } else {
                        // If the user uses RC32 as the system clock then there's nothing to be done!
                }

                if (ahbclk != ahb_div1) {
                        // fast --> slow clock switch
                        hw_clk_set_hclk_div(ahbclk);                 // cm_ahbclk is not altered!
                }
                // else cm_ahbclk == ahb_div1 and nothing has to be done!

                if (apbclk != apb_div1) {
                        hw_clk_set_pclk_div(apbclk);
                }
                // else cm_apbclk == apb_div1 and nothing has to be done!
        }
}

void cm_sys_restore_sysclk(sys_clk_t prev_sysclk)
{
        ASSERT_ERROR(prev_sysclk == sysclk_DBLR64);

        sys_enable_dblr();
        sys_clk_next = prev_sysclk;
        switch_to_dblr();
}

#ifdef OS_PRESENT
void cm_sys_clk_wakeup(void)
{
        /*
         * Clear the "XTAL32_AVAILABLE" flag in the Event Group of the Clock Manager. It will be
         * set again to 1 when the XTAL32M has settled.
         * Note: this translates to a message in a queue that unblocks the Timer task in order to
         * service it. This will be done when the OS scheduler is resumed. Even if the
         * XTAL32M_RDY_IRQn hits while still in this function (pm_sys_wakeup_mode_is_XTAL32 is true), this
         * will result to a second message being added to the same queue. When the OS scheduler is
         * resumed, the first task that will be executed is the Timer task. This will first process
         * the first message of the queue (clear Event Group bits) and then the second (set Event
         * Group bits), which is the desired operation.
         */

        /* Timer task must have the highest priority so that it runs first
         * as soon as the OS scheduler is unblocked.
         * See caller (system_wake_up()) */
        ASSERT_WARNING(configTIMER_TASK_PRIORITY == (configMAX_PRIORITIES - 1));

        xtal32m_settled_notification = false;
        xtal32m_settled = hw_clk_is_xtalm_started();
        if (!xtal32m_settled) {
                OS_EVENT_GROUP_CLEAR_BITS_FROM_ISR(xEventGroupCM_xtal, XTAL32_AVAILABLE);
        }
}
#endif /* OS_PRESENT */

__RETAINED_HOT_CODE void cm_switch_to_xtalm_if_settled(void)
{
        if (xtal32m_settled) {
                // Restore system clocks
                GLOBAL_INT_DISABLE();
                cm_sys_clk_sleep(false);
                GLOBAL_INT_RESTORE();
#ifdef OS_PRESENT
                OS_BASE_TYPE xHigherPriorityTaskWoken;

                xtal32m_is_ready(&xHigherPriorityTaskWoken);
#endif /* OS_PRESENT */
        }
}

#endif /* dg_configUSE_CLOCK_MGR */

