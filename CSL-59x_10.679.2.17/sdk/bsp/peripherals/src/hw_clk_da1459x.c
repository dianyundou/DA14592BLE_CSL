/**
****************************************************************************************
*
* @file hw_clk_da1459x.c
*
* @brief Clock Driver
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

#if dg_configUSE_HW_CLK

#include <stdint.h>
#include "hw_clk.h"

/*
 * Function definitions
 */

__RETAINED_CODE void hw_clk_start_calibration(cal_clk_t clk_type, cal_ref_clk_t clk_ref_type, uint16_t cycles)
{
        uint32_t val = 0;

        /* Must be disabled */
        ASSERT_WARNING(!REG_GETF(ANAMISC_BIF, CLK_REF_SEL_REG, REF_CAL_START));

        ANAMISC_BIF->CLK_REF_CNT_REG = cycles;                      // # of cal clock cycles

        if (clk_ref_type == CALIBRATE_REF_EXT) {
                REG_SET_FIELD(ANAMISC_BIF, CLK_REF_SEL_REG, EXT_CNT_EN_SEL, val, 1);
                REG_SET_FIELD(ANAMISC_BIF, CLK_REF_SEL_REG, CAL_CLK_SEL, val, 0); /* DivN to be calibrated */
        } else {
                REG_SET_FIELD(ANAMISC_BIF, CLK_REF_SEL_REG, CAL_CLK_SEL, val, clk_ref_type);
        }
        REG_SET_FIELD(ANAMISC_BIF, CLK_REF_SEL_REG, REF_CLK_SEL, val, clk_type);
        ANAMISC_BIF->CLK_REF_SEL_REG = val;

        REG_SET_BIT(ANAMISC_BIF, CLK_REF_SEL_REG, REF_CAL_START);
}

uint32_t hw_clk_get_calibration_data(void)
{
        /* Wait until it's finished */
        while (REG_GETF(ANAMISC_BIF, CLK_REF_SEL_REG, REF_CAL_START)) {
        }

        return ANAMISC_BIF->CLK_REF_VAL_REG;
}

__RETAINED_CODE uint32_t hw_clk_get_sysclk_freq(void)
{
        switch (hw_clk_get_sysclk()) {
        case SYS_CLK_IS_XTAL32M:
                return dg_configXTAL32M_FREQ;
        case SYS_CLK_IS_DBLR:
                return dg_configDBLR64M_FREQ;
        default:
                ASSERT_WARNING(0);
                return SYS_CLK_IS_XTAL32M;
        }
}

#define CLK_DELAY_SANITY_CHECKS
#pragma GCC push_options
#pragma GCC optimize ("O3")

void hw_clk_delay_usec(uint32_t usec)
{
        static const uint32_t DIVIDER = 1000000;

#ifdef CLK_DELAY_SANITY_CHECKS
        ASSERT_WARNING((dg_configXTAL32M_FREQ % DIVIDER) == 0);
        ASSERT_WARNING((dg_configDBLR64M_FREQ % DIVIDER) == 0);
        ASSERT_WARNING((HW_CLK_DELAY_OVERHEAD_CYCLES % HW_CLK_CYCLES_PER_DELAY_REP) == 0);
#endif

        static const uint8_t OVERHEAD_REPS = HW_CLK_DELAY_OVERHEAD_CYCLES / HW_CLK_CYCLES_PER_DELAY_REP;
        static volatile const uint32_t sys_freq_table[] = {
                dg_configXTAL32M_FREQ/DIVIDER,  // SYS_CLK_IS_XTAL32M
                dg_configXTAL32M_FREQ/DIVIDER,  // SYS_CLK_IS_RC32: Use XTAL32M frequency. This is the maximum frequency of RC32M
                                                // therefore it is guaranteed that the delay will be greater or equal to the
                                                // requested period.
                0,                              // SYS_CLK_IS_LP is not supported
                dg_configDBLR64M_FREQ/DIVIDER   // SYS_CLK_IS_DBLR
        };

        const uint32_t cycles_per_usec = sys_freq_table[hw_clk_get_sysclk()] >> hw_clk_get_hclk_div(),
                       reps = cycles_per_usec * usec / HW_CLK_CYCLES_PER_DELAY_REP;

#ifdef CLK_DELAY_SANITY_CHECKS
        ASSERT_WARNING(usec <= 0xFFFFFFFF/cycles_per_usec);     // The requested delay is greater than the maximum delay this function can achieve
        ASSERT_WARNING(reps > OVERHEAD_REPS);                   // The requested delay is smaller than the minimum delay this function can achieve.
#endif

        if (reps <= OVERHEAD_REPS) {
                return;
        }

        asm volatile(
                "       nop                             \n"
                "       nop                             \n"
                "       nop                             \n"
                "       nop                             \n"
                "       nop                             \n"
                "loop:  nop                             \n"
                "       subs %[reps], %[reps], #1       \n"
                "       bne loop                        \n"
                :                                       // outputs
                : [reps] "r" (reps - OVERHEAD_REPS)     // inputs
                :                                       // clobbers
        );
}

#pragma GCC pop_options

__STATIC_FORCEINLINE void finish_xtal32m_config(void)
{
        /* Apply final preferred settings for SAH fields */
        REG_SET_MASKED(CRG_XTAL, XTAL32M_CTRL_REG, 0x00000033, 0x00000004);
}

int8_t hw_clk_xtalm_configure_cur_set(void)
{
        /* Since this function configures Xtal32M, it cannot be used as sys_clk
         * at the same time */
        if (hw_clk_get_sysclk() != SYS_CLK_IS_RC32) {
                return -1;
        }

        /* The amplitude regulator should not be in HOLD mode, in order to be
         * allowed to change the current setting */
        if (REG_GETF(CRG_XTAL, XTAL32M_CTRL_REG, XTAL32M_AMPREG_SAH) != 1) {
                return -2;
        }

        uint8_t cur_set = 8; // Start with mid-scale setting
        REG_SETF(CRG_XTAL, XTAL32M_TRIM_REG, XTAL32M_CUR_SET, cur_set);

        hw_clk_delay_usec(40); // Wait a short bit

        hw_clk_enable_xtalm(); // Enable Xtal32m

        /* Wait for the Xtal startup FSM to reach the 'RUN' state */
        while (REG_GETF(CRG_XTAL, XTAL32M_STAT0_REG, XTAL32M_STATE) != 0x7);

        /* Set XTAL32M_FSM_APPLY_CONFIG to 1 (temporarily) */
        REG_SETF(CRG_XTAL, XTAL32M_FSM_REG, XTAL32M_FSM_APPLY_CONFIG, 1);

        while (1) {
                uint32_t cnt;

                /* Wait (with timeout) for XTAL32M_CMP_OUT to go high */
                cnt = 0;
                while (!REG_GETF(CRG_XTAL, XTAL32M_STAT0_REG, XTAL32M_CMP_OUT)) {
                        cnt += 1;
                        hw_clk_delay_usec(30);
                        if (cnt > 1000) {
                                /* Break loop if timeout is reached. */
                                break;
                        }
                }

                if (!REG_GETF(CRG_XTAL, XTAL32M_STAT0_REG, XTAL32M_CMP_OUT)) {
                        /* XTAL32M_CMP_OUT didn't go high within timeout.
                         * Optimum setting reached. End sequence. */
                        break;
                }

                if (cur_set > 0) {
                        cur_set -= 1;
                } else {
                        break; // End sequence
                }

                /* Set lowest current */
                REG_SETF(CRG_XTAL, XTAL32M_TRIM_REG, XTAL32M_CUR_SET, 0);

                /* Wait (with timeout) for XTAL32M_CMP_OUT to go back to low */
                cnt = 0;
                while (REG_GETF(CRG_XTAL, XTAL32M_STAT0_REG, XTAL32M_CMP_OUT)) {
                        cnt += 1;
                        hw_clk_delay_usec(30);
                        if (cnt > 1000) {
                                /* Break loop if timeout is reached. */
                                break;
                        }
                }

                REG_SETF(CRG_XTAL, XTAL32M_TRIM_REG, XTAL32M_CUR_SET, cur_set);
        }

        /* Revert XTAL32M_FSM_APPLY_CONFIG setting */
        REG_SETF(CRG_XTAL, XTAL32M_FSM_REG, XTAL32M_FSM_APPLY_CONFIG, 0);

        hw_clk_disable_xtalm(); // Disable Xtal32m

        return cur_set;
}

void hw_clk_xtalm_configure_irq(void)
{
        uint8_t irq_cnt_rst = 0xFF;
        uint8_t irq_val;

        /* Switch to RC32M */
        if (hw_clk_get_sysclk() == SYS_CLK_IS_XTAL32M) {
                hw_clk_enable_sysclk(SYS_CLK_IS_RC32);
                hw_clk_set_sysclk(SYS_CLK_IS_RC32);
        }

        hw_clk_disable_xtalm(); // Disable Xtal32m

        REG_SET_BIT(CRG_TOP, CLK_CTRL_REG, XTAL32M_DISABLE);

        hw_clk_delay_usec(10000); // Wait until oscillation is completely stopped (max. 10ms)

        /* Use reset values for SAH fields (temporarily) */
        REG_SET_MASKED(CRG_XTAL, XTAL32M_CTRL_REG, 0x0000003F, 0x00000015);

        /* Program the start value of the XtalRdy IRQ counter with the reset value (which is the maximum
         * allowed value and should be sufficient in all cases). */
        REG_SETF(CRG_XTAL, XTAL32M_IRQ_CTRL_REG, XTAL32M_IRQ_CNT, irq_cnt_rst);

        REG_CLR_BIT(CRG_TOP, CLK_CTRL_REG, XTAL32M_DISABLE);
        hw_clk_enable_xtalm(); // Enable Xtal32m

        /* Wait for the Xtal32M FSM to reach the RUN state.
         * This will indicate that the Xtal32m has settled. */
        while (REG_GETF(CRG_XTAL, XTAL32M_STAT0_REG, XTAL32M_STATE) != 0x7);

        /* As soon as RUN state is reached, the current value of the XtalRdy IRQ counter at
         * that point is captured in XTAL32M_IRQ_COUNT_CAP. Therefore, read it and subtract it
         * from the XtalRdy IRQ counter start value, in order to get how many cycles (of the XtalRdy
         * counter) it took for the Xtal32M to settle. */
        irq_val = irq_cnt_rst - REG_GETF(CRG_XTAL, XTAL32M_IRQ_STAT_REG, XTAL32M_IRQ_COUNT_CAP);

        /* Add some margin (by multiplying the cycles measured in the previous step by 1.5 and also
         * add 6 more), because we still need some time after reaching the RUN state before
         * the XTAL32M_READY bit is set (which indicates that we can safely use the Xtal32M) and also
         * in order to allow for possible variations in the settling time at the next times the Xtal32M
         * is enabled. */
        irq_val += (irq_val>>1) + 6;

        /* Use the resulting number of cycles as start value for the XtalRdy IRQ counter from now on
         * (instead of the reset value (which is too big)). (The counter will start counting downwards
         * every time the Xtal32M is enabled and will trigger the XTAL32M_RDY IRQ at a certain time
         * (i.e. as soon as it reaches zero). Since, in the previous steps, we have already measured
         * the settling time and also added some margin, the Xtal32M is expected to always have
         * settled (and the XTAL32M_READY bit to have always been set) every time the IRQ will fire.
         * So this start value will represent the maximum (worst case) expected settling time of Xtal32M.
         * Note that this maximum expected time is taken into account by the power manager when
         * calculating the sleep time of the system (i.e. the system will make sure that it wakes
         * up a little sooner than actually required, so that XTAL32M has settled by then).) */
        REG_SETF(CRG_XTAL, XTAL32M_IRQ_CTRL_REG, XTAL32M_IRQ_CNT, irq_val);

        finish_xtal32m_config();
}

void hw_clk_set_xtalm_settling_time(uint8_t cycles, bool high_clock)
{
        uint32_t val = CRG_XTAL->XTAL32M_IRQ_CTRL_REG;
        REG_SET_FIELD(CRG_XTAL, XTAL32M_IRQ_CTRL_REG, XTAL32M_IRQ_CNT, val, cycles);
        REG_SET_FIELD(CRG_XTAL, XTAL32M_IRQ_CTRL_REG, XTAL32M_IRQ_CLK, val, high_clock ? 0 : 1);

        CRG_XTAL->XTAL32M_IRQ_CTRL_REG = val;

        finish_xtal32m_config();
}

__RETAINED_CODE sys_clk_t hw_clk_get_system_clock(void)
{
        switch (hw_clk_get_sysclk()) {
        case SYS_CLK_IS_RC32:
                return sysclk_RC32;
        case SYS_CLK_IS_XTAL32M:
                return sysclk_XTAL32M;
        case SYS_CLK_IS_DBLR:
                return sysclk_DBLR64;
        case SYS_CLK_IS_LP:
                return sysclk_LP;
        default:
                ASSERT_WARNING(0);
                return sysclk_RC32;
        }
}

#endif /* dg_configUSE_HW_CLK */

