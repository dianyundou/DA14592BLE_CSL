/**
\addtogroup PLA_DRI_PER_OTHER
\{
\addtogroup HW_CLK HW Clock Driver
\{
\brief Clock Driver
*/

/**
****************************************************************************************
*
* @file hw_clk_da1459x.h
*
* @brief Clock Driver header file.
*
 * Copyright (C) 2020-2023 Renesas Electronics Corporation and/or its affiliates.
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

#ifndef HW_CLK_DA1459x_H_
#define HW_CLK_DA1459x_H_


#if dg_configUSE_HW_CLK

#include "sdk_defs.h"
#include "hw_sys.h"
#if (dg_configHW_FCU_WAIT_CYCLES_MODE)
#include "../src/hw_sys_internal.h"
#endif

#define HW_CLK_DELAY_OVERHEAD_CYCLES   (72)
#define HW_CLK_CYCLES_PER_DELAY_REP    (4)

/**
 * \brief Convert settling time (in usec) to XTAL32M_READY counter cycles (250kHz).
 *
 * \note The XTAL32M_READY IRQ counter is clocked by RC32M divided by either 128 or 1024 (depending
 *       on its configuration). This macro assumes the first case (which results in a clock
 *       frequency of ~250kHz).
 *
 * \note The actual frequency of RC32M might be a little less than dg_configRC32M_FREQ, but we
 *       consider the worst case.
 */
#define XTAL32M_USEC_TO_250K_CYCLES(x)  ((uint16_t)((x * (dg_configRC32M_FREQ/1000000) + 127) / 128))

/**
 * \brief Convert XTAL32M_READY counter cycles (250kHz) to LP clock cycles.
 *
 * \note The XTAL32M_READY IRQ counter is clocked by RC32M divided by either 128 or 1024 (depending
 *       on its configuration). This macro assumes the first case (which results in a clock
 *       frequency of ~250kHz).
 *
 * \note The actual frequency of RC32M might be a little less than dg_configRC32M_FREQ, so we consider
 *       the worst case (dg_configRC32M_FREQ_MIN).
 */
#define XTALRDY_CYCLES_TO_LP_CLK_CYCLES(x, lp_freq) ((((uint32_t)(x)) * lp_freq + dg_configRC32M_FREQ_MIN/(128) - 1) / (dg_configRC32M_FREQ_MIN/128))

/**
 * \addtogroup CLOCK_TYPES
 * \{
 */

/**
 * \brief The type of the system clock
 */
typedef enum sys_clk_is_type {
        SYS_CLK_IS_XTAL32M = 0,
        SYS_CLK_IS_RC32,
        SYS_CLK_IS_LP,
        SYS_CLK_IS_DBLR,
        SYS_CLK_IS_INVALID
} sys_clk_is_t;

/**
 * \}
 */

/**
 * \brief The type of clock to be calibrated
 */
typedef enum cal_clk_sel_type {
        CALIBRATE_RCLP = 0,
        CALIBRATE_RC32M,
        CALIBRATE_XTAL32K,
        CALIBRATE_RCX,
        CALIBRATE_RCOSC,
} cal_clk_t;

/**
 * \brief The reference clock used for calibration
 */
typedef enum cal_ref_clk_sel_type {
        CALIBRATE_REF_DIVN = 0,
        CALIBRATE_REF_RCLP,
        CALIBRATE_REF_RC32M,
        CALIBRATE_REF_XTAL32K,
        CALIBRATE_REF_RCOSC,
        CALIBRATE_REF_EXT,
} cal_ref_clk_t;

/**
 * \brief The system clock type
 *
 * \note Must only be used with functions cm_sys_clk_init/set()
 */
typedef enum sysclk_type {
        sysclk_RC32    = 0,     //!< RC32
        sysclk_XTAL32M = 2,     //!< 32MHz
        sysclk_DBLR64  = 4,     //!< 64MHz
        sysclk_LP      = 255,   //!< not applicable
} sys_clk_t;

/**
 * \brief The RCLP mode
 *
 * \note Must only be used with functions hw_clk_set/get_rclp_mode()
 */
typedef enum rclp_mode_type {
        RCLP_DEFAULT    = 0,                                                            //!< 32kHz/512kHz
        RCLP_FORCE_SLOW = REG_MSK(CRG_TOP, CLK_RCLP_REG, RCLP_LOW_SPEED_FORCE),         //!< 32kHz
        RCLP_FORCE_FAST = REG_MSK(CRG_TOP, CLK_RCLP_REG, RCLP_HIGH_SPEED_FORCE),        //!< 512kHz
} rclp_mode_t;

/**
 * \brief The CPU clock type (speed)
 *
 */
typedef enum cpu_clk_type {
        cpuclk_2M = 2,          //!< 2 MHz
        cpuclk_4M = 4,          //!< 4 MHz
        cpuclk_8M = 8,          //!< 8 MHz
        cpuclk_16M = 16,        //!< 16 MHz
        cpuclk_32M = 32,        //!< 32 MHz
        cpuclk_64M = 64         //!< 64 MHz
} cpu_clk_t;

/**
 * \brief Check if the RC32M is enabled.
 *
 * \return true if the RC32M is enabled, else false.
 */
__STATIC_INLINE bool hw_clk_check_rc32_status(void)
{
        return REG_GETF(CRG_TOP, CLK_RC32M_REG, RC32M_ENABLE);
}

/**
 * \brief Activate the RC32M.
 */
__STATIC_INLINE void hw_clk_enable_rc32(void)
{
        REG_SET_BIT(CRG_TOP, CLK_RC32M_REG, RC32M_ENABLE);
}

/**
 * \brief Deactivate the RC32M.
 */
__STATIC_FORCEINLINE void hw_clk_disable_rc32(void)
{
        REG_CLR_BIT(CRG_TOP, CLK_RC32M_REG, RC32M_ENABLE);
}

/**
 * \brief Set the XTAL32M settling time.
 *
 * This function configures the start value and the clock (fast or slow) for the XTAL32M_READY
 * IRQ counter.
 *
 * \param cycles Number of clock cycles
 * \param high_clock If true, use 250kHz clock (RC32M divided by 128), else use 31.25kHz clock
 *                   (RC32M divided by 1024).
 */
void hw_clk_set_xtalm_settling_time(uint8_t cycles, bool high_clock);

/**
 * \brief Get the XTAL32M settling time (in 250kHz clock cycles).
 *
 * This function reads the start value of the XTAL32M_READY IRQ counter. If the fast clock
 * configuration is used for the counter, the function returns the read value as is. Otherwise,
 * the function adjusts the read value accordingly (so that it represents the equivalent start
 * value in the fast clock case).
 *
 * \return The number of 250kHz clock cycles required for XTAL32M to settle
 */
__STATIC_FORCEINLINE uint16_t hw_clk_get_xtalm_settling_time(void)
{
        uint32_t val = CRG_XTAL->XTAL32M_IRQ_CTRL_REG;
        uint16_t cycles = REG_GET_FIELD(CRG_XTAL, XTAL32M_IRQ_CTRL_REG, XTAL32M_IRQ_CNT, val);

        if (REG_GET_FIELD(CRG_XTAL, XTAL32M_IRQ_CTRL_REG, XTAL32M_IRQ_CLK, val) == 1) {
                // 31.25kHz clock cycles. Convert them to 250kHz clock cycles.
                cycles *= 8;
        }
        return cycles;
}

/**
 * \brief Check if the XTAL32M is enabled.
 *
 * \return true if the XTAL32M is enabled, else false.
 */
__STATIC_INLINE bool hw_clk_check_xtalm_status(void)
{
        return REG_GETF(CRG_XTAL, XTAL32M_STAT0_REG, XTAL32M_READY) == 1;
}

/**
 * \brief Activate the XTAL32M.
 */
__STATIC_INLINE void hw_clk_enable_xtalm(void)
{
        /* Do nothing if XTAL32M is already up and running. */
        if (REG_GETF(CRG_TOP, CLK_CTRL_REG, RUNNING_AT_XTAL32M)) {
                return;
        }

        // Check if TIM power domain is enabled
        ASSERT_WARNING(REG_GETF(CRG_TOP, SYS_STAT_REG, TIM_IS_UP));

        // Check the power supply

        /* Enable the XTAL oscillator. */
        REG_SET_BIT(CRG_XTAL, XTAL32M_CTRL_REG, XTAL32M_ENABLE);
}

/**
 * \brief Deactivate the XTAL32M.
 */
__STATIC_INLINE void hw_clk_disable_xtalm(void)
{
        REG_CLR_BIT(CRG_XTAL, XTAL32M_CTRL_REG, XTAL32M_ENABLE);
}

/**
 * \brief Check if the XTAL32M has settled.
 *
 * \return true if the XTAL32M has settled, else false.
 */
__STATIC_INLINE bool hw_clk_is_xtalm_started(void)
{
        return (REG_GETF(CRG_XTAL, XTAL32M_STAT0_REG, XTAL32M_READY) &&
                REG_GETF(CRG_XTAL, XTAL32M_IRQ_STAT_REG, XTAL32M_IRQ_COUNT_STAT) == 0);
}

/**
 * \brief Return the clock used as the system clock.
 *
 * \return The type of the system clock
 */
__STATIC_FORCEINLINE sys_clk_is_t hw_clk_get_sysclk(void)
{
        static const uint32_t freq_msk = CRG_TOP_CLK_CTRL_REG_RUNNING_AT_LP_CLK_Msk |
                                   CRG_TOP_CLK_CTRL_REG_RUNNING_AT_RC32M_Msk |
                                   CRG_TOP_CLK_CTRL_REG_RUNNING_AT_XTAL32M_Msk |
                                   CRG_TOP_CLK_CTRL_REG_RUNNING_AT_DBLR64M_Msk;

        static __RETAINED_CONST_INIT const sys_clk_is_t clocks[] = {
                SYS_CLK_IS_LP,          // 0b000
                SYS_CLK_IS_RC32,        // 0b001
                SYS_CLK_IS_XTAL32M,     // 0b010
                SYS_CLK_IS_INVALID,
                SYS_CLK_IS_DBLR         // 0b100
        };

        // drop bit0 to reduce the size of clocks[]
        uint32_t index = (CRG_TOP->CLK_CTRL_REG & freq_msk) >> (CRG_TOP_CLK_CTRL_REG_RUNNING_AT_LP_CLK_Pos + 1);
        ASSERT_WARNING(index <= 4);

        sys_clk_is_t clk = clocks[index];
        ASSERT_WARNING(clk != SYS_CLK_IS_INVALID);
        return clk;
}

/**
 * \brief Get the current system clock.
 *
 * \return The current system clock.
 *
 * \warning This function returns a sys_clk_t enum, whereas the hw_clk_get_sysclk() a sys_clk_is_t.
 *          Consider calling the right function, based on which type of enumerator is needed.
 */
__RETAINED_CODE sys_clk_t hw_clk_get_system_clock(void);

/**
 * \brief Check whether the XTAL32K is the Low Power clock.
 *
 * \return true if XTAL32K is the LP clock, else false.
 */
__STATIC_INLINE bool hw_clk_lp_is_xtal32k(void)
{
        return REG_GETF(CRG_TOP, CLK_XTAL32K_REG, XTAL32K_ENABLE) &&
              (REG_GETF(CRG_TOP, CLK_CTRL_REG, LP_CLK_SEL) == LP_CLK_IS_XTAL32K);
}

/**
 * \brief Check whether the RCLP is the Low Power clock.
 *
 * \return true if RCLP is the LP clock, else false.
 */
__STATIC_INLINE bool hw_clk_lp_is_rclp(void)
{
        return (!REG_GETF(CRG_TOP, CLK_RCLP_REG, RCLP_DISABLE)) &&
              (REG_GETF(CRG_TOP, CLK_CTRL_REG, LP_CLK_SEL) == LP_CLK_IS_RCLP);
}

/**
 * \brief Check whether the RCX is the Low Power clock.
 *
 * \return true if RCX is the LP clock, else false.
 */
__STATIC_INLINE bool hw_clk_lp_is_rcx(void)
{
        return REG_GETF(CRG_TOP, CLK_RCX_REG, RCX_ENABLE) &&
              (REG_GETF(CRG_TOP, CLK_CTRL_REG, LP_CLK_SEL) == LP_CLK_IS_RCX);
}

/**
 * \brief Check whether the RCX is the Low Power clock.
 *
 * \return true if RCX is the LP clock, else false.
 */
__STATIC_INLINE bool hw_clk_lp_is_external(void)
{
        return REG_GETF(CRG_TOP, CLK_CTRL_REG, LP_CLK_SEL) == LP_CLK_IS_EXTERNAL;
}

/**
 * \brief Set RCX as the Low Power clock.
 *
 * \warning The RCX must have been enabled before calling this function!
 *
 * \note Call with interrupts disabled to ensure that CLK_CTRL_REG
 *       read/modify/write operation is not interrupted
 */
__STATIC_INLINE void hw_clk_lp_set_rcx(void)
{
        ASSERT_WARNING(__get_PRIMASK() == 1 || __get_BASEPRI());
        ASSERT_WARNING(REG_GETF(CRG_TOP, CLK_RCX_REG, RCX_ENABLE));

        REG_SETF(CRG_TOP, CLK_CTRL_REG, LP_CLK_SEL, LP_CLK_IS_RCX);
}

/**
 * \brief Set XTAL32K as the Low Power clock.
 *
 * \warning The XTAL32K must have been enabled before calling this function!
 *
 * \note Call with interrupts disabled to ensure that CLK_CTRL_REG
 *       read/modify/write operation is not interrupted
 */
__STATIC_INLINE void hw_clk_lp_set_xtal32k(void)
{
        ASSERT_WARNING(__get_PRIMASK() == 1 || __get_BASEPRI());
        ASSERT_WARNING(REG_GETF(CRG_TOP, CLK_XTAL32K_REG, XTAL32K_ENABLE));

        REG_SETF(CRG_TOP, CLK_CTRL_REG, LP_CLK_SEL, LP_CLK_IS_XTAL32K);
}

/**
 * \brief Set an external digital clock as the Low Power clock.
 *
 * \note Call with interrupts disabled to ensure that CLK_CTRL_REG
 *       read/modify/write operation is not interrupted
 */
__STATIC_INLINE void hw_clk_lp_set_ext32k(void)
{
        ASSERT_WARNING(__get_PRIMASK() == 1 || __get_BASEPRI());

        REG_SETF(CRG_TOP, CLK_CTRL_REG, LP_CLK_SEL, LP_CLK_IS_EXTERNAL);
}

/**
 * \brief Configure RCLP.
 *
 * \param[in] mode The mode of the RCLP.
 */
__STATIC_INLINE void hw_clk_set_rclp_mode(rclp_mode_t mode)
{
        GLOBAL_INT_DISABLE();
        switch (mode) {
        case RCLP_DEFAULT:
                REG_SET_MASKED(CRG_TOP, CLK_RCLP_REG, (RCLP_FORCE_SLOW | RCLP_FORCE_FAST), RCLP_DEFAULT);
                break;
        case RCLP_FORCE_SLOW:
                REG_SET_MASKED(CRG_TOP, CLK_RCLP_REG, (RCLP_FORCE_SLOW | RCLP_FORCE_FAST), RCLP_FORCE_SLOW);
                break;
        case RCLP_FORCE_FAST:
                REG_SET_MASKED(CRG_TOP, CLK_RCLP_REG, (RCLP_FORCE_SLOW | RCLP_FORCE_FAST), RCLP_FORCE_FAST);
                break;
        default:
                ASSERT_WARNING(0);
                break;
        }
        GLOBAL_INT_RESTORE();
}

/**
 * \brief Get RCLP mode of operation.
 *
 * \return The mode of the RCLP.
 */
__STATIC_INLINE rclp_mode_t hw_clk_get_rclp_mode(void)
{
        return (CRG_TOP->CLK_RCLP_REG & (RCLP_FORCE_SLOW | RCLP_FORCE_FAST));
}

/**
 * \brief Enable RCLP.
 */
__STATIC_INLINE void hw_clk_enable_rclp(void)
{
        REG_CLR_BIT(CRG_TOP, CLK_RCLP_REG, RCLP_DISABLE);
}

/**
 * \brief Disable RCLP.
 *
 * \warning RCLP must not be the LP clock.
 */
__STATIC_INLINE void hw_clk_disable_rclp(void)
{
        ASSERT_WARNING(REG_GETF(CRG_TOP, CLK_CTRL_REG, LP_CLK_SEL) != LP_CLK_IS_RCLP);

        REG_SET_BIT(CRG_TOP, CLK_RCLP_REG, RCLP_DISABLE);
}

/**
 * \brief Set RCLP as the Low Power clock.
 *
 * \warning The RCLP must have been enabled before calling this function!
 *
 * \note Call with interrupts disabled to ensure that CLK_CTRL_REG
 *       read/modify/write operation is not interrupted
 */
__STATIC_INLINE void hw_clk_lp_set_rclp(void)
{
        ASSERT_WARNING(__get_PRIMASK() == 1 || __get_BASEPRI());
        ASSERT_WARNING(!REG_GETF(CRG_TOP, CLK_RCLP_REG, RCLP_DISABLE));

        REG_SETF(CRG_TOP, CLK_CTRL_REG, LP_CLK_SEL, LP_CLK_IS_RCLP);
}

/**
 * \brief Configure RCX. This must be done only once since the register is retained.
 */
__STATIC_INLINE void hw_clk_configure_rcx(void)
{
        // Reset values for CLK_RCX_REG register should be used
}

/**
 * \brief Enable RCX but does not set it as the LP clock.
 */
__STATIC_INLINE void hw_clk_enable_rcx(void)
{
        REG_SET_BIT(CRG_TOP, CLK_RCX_REG, RCX_ENABLE);
}

/**
 * \brief Disable RCX.
 *
 * \warning RCX must not be the LP clock
 */
__STATIC_INLINE void hw_clk_disable_rcx(void)
{
        ASSERT_WARNING(REG_GETF(CRG_TOP, CLK_CTRL_REG, LP_CLK_SEL) != LP_CLK_IS_RCX);

        REG_CLR_BIT(CRG_TOP, CLK_RCX_REG, RCX_ENABLE);
}

/**
 * \brief Configure XTAL32K. This must be done only once since the register is retained.
 */
__STATIC_INLINE void hw_clk_configure_xtal32k(void)
{
        // Configure xtal.
        uint32_t reg = CRG_TOP->CLK_XTAL32K_REG;
        REG_SET_FIELD(CRG_TOP, CLK_XTAL32K_REG, XTAL32K_CUR, reg, 5);
        REG_SET_FIELD(CRG_TOP, CLK_XTAL32K_REG, XTAL32K_RBIAS, reg, 3);

        REG_SET_FIELD(CRG_TOP, CLK_XTAL32K_REG, XTAL32K_DISABLE_AMPREG, reg, dg_configEXT_LP_IS_DIGITAL);

        CRG_TOP->CLK_XTAL32K_REG = reg;
}

/**
 * \brief Enable XTAL32K but do not set it as the LP clock.
 */
__STATIC_INLINE void hw_clk_enable_xtal32k(void)
{
        REG_SET_BIT(CRG_TOP, CLK_XTAL32K_REG, XTAL32K_ENABLE);
}

/**
 * \brief Disable XTAL32K.
 *
 * \warning XTAL32K must not be the LP clock.
 */
__STATIC_INLINE void hw_clk_disable_xtal32k(void)
{
        ASSERT_WARNING(REG_GETF(CRG_TOP, CLK_CTRL_REG, LP_CLK_SEL) != LP_CLK_IS_XTAL32K);
        REG_CLR_BIT(CRG_TOP, CLK_XTAL32K_REG, XTAL32K_ENABLE);
}

/**
 * \brief Enable the clock calibration interrupt.
 *
 */
__STATIC_INLINE void hw_clk_calibration_enable_irq(void)
{
        REG_SET_BIT(ANAMISC_BIF, CLK_CAL_IRQ_REG, CLK_CAL_IRQ_EN);
}

/**
 * \brief Clear the clock calibration interrupt.
 *
 */
__STATIC_INLINE void hw_clk_calibration_clear_irq(void)
{
        REG_SET_BIT(ANAMISC_BIF, CLK_CAL_IRQ_REG, CLK_CAL_IRQ_CLR);
}

/**
 * \brief Read the status of the clock calibration interrupt.
 *
 * \return the status of the IRQ bit
 */
__STATIC_INLINE bool hw_clk_calibration_status_irq(void)
{
        return REG_GETF(ANAMISC_BIF, CLK_CAL_IRQ_REG, CLK_CAL_IRQ_STATUS) == 0;
}

/**
 * \brief Check the status of a requested calibration.
 *
 * \return true if the calibration has finished (or never run) else false.
 */
__STATIC_INLINE bool hw_clk_calibration_finished(void)
{
        return REG_GETF(ANAMISC_BIF, CLK_REF_SEL_REG, REF_CAL_START) == 0;
}

/**
 * \brief Start calibration of a clock.
 *
 * \param[in] clk_type The clock to be calibrated. Must be enabled.
 * \param[in] clk_ref_type The reference clock to USE.
 * \param[in] cycles The number of cycles of the to-be-calibrated clock to be measured using the
 *            reference clock.
 *
 * \warning If clk_ref_type == CALIBRATE_REF_EXT, the clk_type is not used. Instead, the value
 *          returned by hw_clk_get_calibration_data() is the number of clock cycles of DIVN counted
 *          during a single pulse of the EXT clock source used.
 *          The EXT clock source must be applied to a pin with a PID equal to HW_GPIO_FUNC_UART_RX
 */
void hw_clk_start_calibration(cal_clk_t clk_type, cal_ref_clk_t clk_ref_type, uint16_t cycles);

/**
 * \brief Return the calibration results.
 *
 * \return The number of cycles of the reference clock corresponding to the programmed
 * (in hw_clk_start_calibration() cycles param) cycles of the clock to be calibrated.
 * In the special case of EXTernal calibration, this function returns the number of cycles of DIVN
 * that correspond to one positive pulse of the EXT source applied.
 */
uint32_t hw_clk_get_calibration_data(void);

/**
 * \brief Set System clock.
 *
 * \param[in] mode The new system clock.
 *
 * \note System clock switch to Doubler is only allowed when current system clock is XTAL32M.
 * System clock switch from Doubler is only allowed when new system clock is XTAL32M.
 */
__STATIC_INLINE void hw_clk_set_sysclk(sys_clk_is_t mode)
{
        /* Make sure a valid sys clock is requested */
        ASSERT_WARNING(mode <= SYS_CLK_IS_DBLR);

        /* Switch to Doubler is only allowed when current system clock is XTAL32M */
        ASSERT_WARNING(mode != SYS_CLK_IS_DBLR ||
                REG_GETF(CRG_TOP, CLK_CTRL_REG, RUNNING_AT_XTAL32M)  ||
                REG_GETF(CRG_TOP, CLK_CTRL_REG, RUNNING_AT_DBLR64M))

        /* Switch to Doubler is only allowed when HDIV and PDIV are 0 */
        ASSERT_WARNING(mode != SYS_CLK_IS_DBLR || (hw_clk_get_hclk_div() == ahb_div1 &&  hw_clk_get_pclk_div() == apb_div1));

        /* Switch from Doubler is only allowed when new system clock is XTAL32M */
        ASSERT_WARNING(!REG_GETF(CRG_TOP, CLK_CTRL_REG, RUNNING_AT_DBLR64M) ||
                mode == SYS_CLK_IS_XTAL32M  ||
                mode == SYS_CLK_IS_DBLR);

#if dg_configHW_FCU_WAIT_CYCLES_MODE
        hw_sys_fcu_set_max_wait_cycles();
#endif
        if (mode == SYS_CLK_IS_XTAL32M && REG_GETF(CRG_TOP, CLK_CTRL_REG, RUNNING_AT_RC32M)) {
                REG_SET_BIT(CRG_TOP, CLK_SWITCH2XTAL_REG, SWITCH2XTAL);
        }
        else {
                GLOBAL_INT_DISABLE();
                REG_SETF(CRG_TOP, CLK_CTRL_REG, SYS_CLK_SEL, mode);
                GLOBAL_INT_RESTORE();
        }

        /* Wait until the switch is done! */
        switch (mode) {
        case SYS_CLK_IS_XTAL32M:
                while (!REG_GETF(CRG_TOP, CLK_CTRL_REG, RUNNING_AT_XTAL32M)) {
                }
                break;

        case SYS_CLK_IS_RC32:
                while (!REG_GETF(CRG_TOP, CLK_CTRL_REG, RUNNING_AT_RC32M)) {
                }
                break;

        case SYS_CLK_IS_LP:
                while (!REG_GETF(CRG_TOP, CLK_CTRL_REG, RUNNING_AT_LP_CLK)) {
                }
                break;

        case SYS_CLK_IS_DBLR:
                while (!REG_GETF(CRG_TOP, CLK_CTRL_REG, RUNNING_AT_DBLR64M)) {
                }
                break;
        default:
                ASSERT_WARNING(0);
        }
#if dg_configHW_FCU_WAIT_CYCLES_MODE
        hw_sys_fcu_set_optimum_wait_cycles();
#endif
}

/**
 * \brief Set the divider of the AMBA High Speed Bus.
 *
 * \param div The AMBA High Speed Bus divider
 */
__STATIC_FORCEINLINE void hw_clk_set_hclk_div(ahb_div_t div)
{
        ASSERT_WARNING(div <= ahb_div16);
        GLOBAL_INT_DISABLE();
#if dg_configHW_FCU_WAIT_CYCLES_MODE
        hw_sys_fcu_set_max_wait_cycles();
        REG_SETF(CRG_TOP, CLK_AMBA_REG, HCLK_DIV, div);
        hw_sys_fcu_set_optimum_wait_cycles();
#else
        REG_SETF(CRG_TOP, CLK_AMBA_REG, HCLK_DIV, div);
#endif
        GLOBAL_INT_RESTORE();
}

/**
 * \brief Enable the Doubler.
 */
__STATIC_FORCEINLINE void hw_clk_dblr_sys_on(void)
{
        GLOBAL_INT_DISABLE();

        /* XTAL32M_LDO_LEVEL voltage must be set to 0.9V prior to enabling DBLR */
        ASSERT_WARNING(REG_GETF(CRG_TOP, POWER_LEVEL_REG, XTAL32M_LDO_LEVEL) > 2);

        /* Release the reset of the Doubler Control Logic */
        REG_SET_BIT(CRG_XTAL, CLKDBLR_CTRL1_REG, RESET_N);

        /* Turn on DBLR. */
        REG_SET_BIT(CRG_XTAL, CLKDBLR_CTRL1_REG, ENABLE);

        GLOBAL_INT_RESTORE();
}

/**
 * \brief Disable the Doubler.
 *
 * \warning The System clock must have been set to XTAL32M before calling this function!
 */
__STATIC_FORCEINLINE void hw_clk_dblr_sys_off(void)
{
        GLOBAL_INT_DISABLE();

        // The DBLR is not the system clk.
        ASSERT_WARNING(!REG_GETF(CRG_TOP, CLK_CTRL_REG, RUNNING_AT_DBLR64M));

        /* Reset the Doubler Control Logic. */
        REG_CLR_BIT(CRG_XTAL, CLKDBLR_CTRL1_REG, RESET_N);

        /* Turn off DBLR. */
        REG_CLR_BIT(CRG_XTAL, CLKDBLR_CTRL1_REG, ENABLE);

        GLOBAL_INT_RESTORE();
}

/**
 * \brief Check if the Doubler is enabled.
 *
 * \return true if the Doubler is enabled, else false.
 */
__STATIC_INLINE bool hw_clk_check_dblr_status(void)
{
        return REG_GETF(CRG_XTAL, CLKDBLR_CTRL1_REG, ENABLE);
}

/**
 * \brief Check if the Doubler is available.
 *
 * \return true if the Doubler is available, else false.
 */
__STATIC_INLINE bool hw_clk_is_dblr_ready(void)
{
        return REG_GETF(CRG_XTAL, CLKDBLR_STATUS_REG , OUTPUT_READY);
}

/**
 * \brief Activate a System clock.
 *
 * \param[in] clk The clock to activate.
 */
__STATIC_INLINE void hw_clk_enable_sysclk(sys_clk_is_t clk)
{
        switch (clk) {
        case SYS_CLK_IS_XTAL32M:
                hw_clk_enable_xtalm();
                return;
        case SYS_CLK_IS_RC32:
                hw_clk_enable_rc32();
                return;
        case SYS_CLK_IS_DBLR:
                hw_clk_dblr_sys_on();
                return;
        default:
                /* An invalid clock is requested */
                ASSERT_WARNING(0);
        }
}

/**
 * \brief Deactivate a System clock.
 *
 * \param[in] clk The clock to deactivate.
 */
__STATIC_INLINE void hw_clk_disable_sysclk(sys_clk_is_t clk)
{
        switch (clk) {
        case SYS_CLK_IS_XTAL32M:
                hw_clk_disable_xtalm();
                return;
        case SYS_CLK_IS_RC32:
                hw_clk_disable_rc32();
                return;
        case SYS_CLK_IS_DBLR:
                hw_clk_dblr_sys_off();
                return;
        default:
                /* An invalid clock is requested */
                ASSERT_WARNING(0);
        }
}

/**
 * \brief Check if a System clock is enabled.
 *
 * \return true if the System clock is enabled, else false.
 */
__STATIC_INLINE bool hw_clk_is_enabled_sysclk(sys_clk_is_t clk)
{
        switch (clk) {
        case SYS_CLK_IS_XTAL32M:
                return hw_clk_check_xtalm_status();
        case SYS_CLK_IS_RC32:
                return hw_clk_check_rc32_status();
        case SYS_CLK_IS_DBLR:
                return hw_clk_check_dblr_status();
        default:
                /* An invalid clock is requested */
                ASSERT_WARNING(0);
                return false;
        }
}

/**
 * \brief Configure pin to connect an external digital clock.
 */
__STATIC_FORCEINLINE void hw_clk_configure_ext32k_pins(void)
{
        GPIO-> P1_14_MODE_REG = 0;
}

/**
 * \brief Configure XTAL32M current setting
 *
 * Find and apply optimum value for XTAL32M_TRIM_REG[XTAL32M_CUR_SET] (i.e. current setting during
 * normal operation) (depends on crystal loss). This minimizes phase noise and ensures that the
 * oscillation stays stable.
 *
 * \return If >= 0 , the optimum current setting, else error (see warnings below)
 *
 * \warning The sys_clk should be set to RC32M (i.e. XTAL32M should not be used) when calling this
 *          function (hw_clk_disable_xtalm() is also called before returning). Otherwise, the
 *          function returns -1.
 *
 * \warning The amplitude regulator should not be in HOLD mode when calling this function.
 *          Otherwise, the function returns -2.
 *
 * \note This function should be called at startup before using XTAL32M, in case there is no
 *       optimum value for CUR_SET already stored in the Configuration Script.
 *
 */
int8_t hw_clk_xtalm_configure_cur_set(void);

/**
 * \brief Configure XTAL32M IRQ counter start value.
 *
 * \note At startup, the XTAL32M IRQ counter must be calculated and stored
 * in the respective register XTAL32M_IRQ_CTRL_REG->XTAL32M_IRQ_CNT.
 * The calculation of the correct start value is based on the IRQ counter captured
 * upon XTAL32M settling (i.e XTAL32M_IRQ_STAT_REG->XTAL32M_IRQ_COUNT_CAP),
 * after trim and preferred settings have been applied.
 * Given that the 32M crystal is enabled by the booter, the SDK must first disable it,
 * then wait 10ms for the oscillation to stop completely, and finally re-enable it to
 * achieve a valid IRQ counter calculation.
 * If the XTAL32M is configured as the system clock, the function switches the system clock
 * to RC32M before stopping the crystal.
 * The function leaves the system clock switched to RC32M and XTAL32M settled.
 *
 */

void hw_clk_xtalm_configure_irq(void);

/**
 * \brief Enable XTAL32M interrupt generation.
 *
 */
__STATIC_INLINE void hw_clk_xtalm_irq_enable(void)
{
        REG_SET_BIT(CRG_XTAL, XTAL32M_IRQ_CTRL_REG, XTAL32M_IRQ_ENABLE);
}

#endif /* dg_configUSE_HW_CLK */


#endif /* HW_CLK_DA1459x_H_ */

/**
\}
\}
*/
