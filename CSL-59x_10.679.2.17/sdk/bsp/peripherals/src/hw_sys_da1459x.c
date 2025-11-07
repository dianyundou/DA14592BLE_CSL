/**
****************************************************************************************
*
* @file hw_sys_da1459x.c
*
* @brief System Driver
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

#if dg_configUSE_HW_SYS

#include <stdint.h>
#include "hw_cpm.h"
#include "hw_clk.h"
#include "hw_pd.h"
#include "hw_sys.h"
#include "hw_gpio.h"
#include "hw_cache.h"
#include "hw_sys_internal.h"
#include "hw_mpu.h"
#if (dg_configUSE_SYS_TCS == 1)
#include "sys_tcs.h"
#endif
#if dg_configHW_FCU_WAIT_CYCLES_MODE
#include "hw_pmu.h"
#include "hw_fcu.h"
#endif
#include "bsp_device_definitions_internal.h"

#if dg_configUSE_HW_MPU
extern uint32_t __Vectors_Size;
#endif

#if (dg_configHW_FCU_WAIT_CYCLES_MODE == 2)
/* Initialize them with invalid values */
__RETAINED_RW static ahb_div_t _old_div = ahb_invalid;
__RETAINED_RW static HW_PMU_VDD_VOLTAGE _old_vdd = HW_PMU_VDD_VOLTAGE_INVALID;
__RETAINED_RW static sys_clk_is_t _old_clk = SYS_CLK_IS_INVALID;
__RETAINED static cpu_clk_t _old_freq;
__RETAINED static int8_t _report_requests;
#endif

#define SW_CURSOR_GPIO                  *(SW_CURSOR_PORT == 0 ? \
                                                (SW_CURSOR_PIN == 0 ? &(GPIO->P0_00_MODE_REG) : \
                                                (SW_CURSOR_PIN == 1 ? &(GPIO->P0_01_MODE_REG) : \
                                                (SW_CURSOR_PIN == 2 ? &(GPIO->P0_02_MODE_REG) : \
                                                (SW_CURSOR_PIN == 3 ? &(GPIO->P0_03_MODE_REG) : \
                                                (SW_CURSOR_PIN == 4 ? &(GPIO->P0_04_MODE_REG) : \
                                                (SW_CURSOR_PIN == 5 ? &(GPIO->P0_05_MODE_REG) : \
                                                (SW_CURSOR_PIN == 6 ? &(GPIO->P0_06_MODE_REG) : \
                                                (SW_CURSOR_PIN == 7 ? &(GPIO->P0_07_MODE_REG) : \
                                                (SW_CURSOR_PIN == 8 ? &(GPIO->P0_08_MODE_REG) : \
                                                (SW_CURSOR_PIN == 9 ? &(GPIO->P0_09_MODE_REG) : \
                                                (SW_CURSOR_PIN == 10 ? &(GPIO->P0_10_MODE_REG) : \
                                                (SW_CURSOR_PIN == 11 ? &(GPIO->P0_11_MODE_REG) : \
                                                (SW_CURSOR_PIN == 12 ? &(GPIO->P0_12_MODE_REG) : \
                                                (SW_CURSOR_PIN == 13 ? &(GPIO->P0_13_MODE_REG) : \
                                                (SW_CURSOR_PIN == 14 ? &(GPIO->P0_14_MODE_REG) : \
                                                (SW_CURSOR_PIN == 15 ? &(GPIO->P0_15_MODE_REG) \
                                                )))))))))))))))))) : \
                                                (SW_CURSOR_PIN == 0 ? &(GPIO->P1_00_MODE_REG) : \
                                                (SW_CURSOR_PIN == 1 ? &(GPIO->P1_01_MODE_REG) : \
                                                (SW_CURSOR_PIN == 2 ? &(GPIO->P1_02_MODE_REG) : \
                                                (SW_CURSOR_PIN == 3 ? &(GPIO->P1_03_MODE_REG) : \
                                                (SW_CURSOR_PIN == 4 ? &(GPIO->P1_04_MODE_REG) : \
                                                (SW_CURSOR_PIN == 5 ? &(GPIO->P1_05_MODE_REG) : \
                                                (SW_CURSOR_PIN == 6 ? &(GPIO->P1_06_MODE_REG) : \
                                                (SW_CURSOR_PIN == 7 ? &(GPIO->P1_07_MODE_REG) : \
                                                (SW_CURSOR_PIN == 8 ? &(GPIO->P1_08_MODE_REG) : \
                                                (SW_CURSOR_PIN == 9 ? &(GPIO->P1_09_MODE_REG) : \
                                                (SW_CURSOR_PIN == 10 ? &(GPIO->P1_10_MODE_REG) : \
                                                (SW_CURSOR_PIN == 11 ? &(GPIO->P1_11_MODE_REG) : \
                                                (SW_CURSOR_PIN == 12 ? &(GPIO->P1_12_MODE_REG) : \
                                                (SW_CURSOR_PIN == 13 ? &(GPIO->P1_13_MODE_REG) : \
                                                (SW_CURSOR_PIN == 14 ? &(GPIO->P1_14_MODE_REG) : \
                                                (SW_CURSOR_PIN == 15 ? &(GPIO->P1_15_MODE_REG) : \
                                                )))))))))))))))))

#define SW_CURSOR_SET                   *(SW_CURSOR_PORT == 0 ? &(GPIO->P0_SET_DATA_REG) :    \
                                                                &(GPIO->P1_SET_DATA_REG))

#define SW_CURSOR_RESET                 *(SW_CURSOR_PORT == 0 ? &(GPIO->P0_RESET_DATA_REG) :  \
                                                                &(GPIO->P1_RESET_DATA_REG))

#define SW_CURSOR_SET_PAD_LATCH         *(SW_CURSOR_PORT == 0 ? &(CRG_TOP->P0_SET_PAD_LATCH_REG) :  \
                                                                &(CRG_TOP->P1_SET_PAD_LATCH_REG))

#define SW_CURSOR_RESET_PAD_LATCH       *(SW_CURSOR_PORT == 0 ? &(CRG_TOP->P0_RESET_PAD_LATCH_REG) :  \
                                                                &(CRG_TOP->P1_RESET_PAD_LATCH_REG))

#define NUM_OF_REG_CONFIG_ENTRIES       5

/**
 * \brief The number of register addresses to check if they are configured in CS
 *
 * \sa hw_sys_apply_default_values
 */
#define NUM_OF_REG_ADDR_IN_CS           2

/*
 * This macro is used to match the value of the CHIP_ID_REG register
 * in order to detect the DEVICE_CHIP_ID.
 */
#define ASCII_2634        0x32363334      // '2' '6' '3' '4'

/**
 * \brief VCO tune voltage low and high levels
 */
#define TUNE_VOLTAGE_LOW_LEVEL          125
#define TUNE_VOLTAGE_HIGH_LEVEL         550


_Static_assert(DEVICE_REVISION_MIN < DEVICE_REVISION_MAX, "Wrong device revision boundaries");
_Static_assert(MAKE_DEVICE_REVISION_ENCODING(0) > DEVICE_REVISION_MIN, "Device revision encoding <= DEVICE_REVISION_MIN");
_Static_assert(MAKE_DEVICE_REVISION_ENCODING(UINT32_MAX) < DEVICE_REVISION_MAX, "Device revision encoding >= DEVICE_REVISION_MAX");

_Static_assert(DEVICE_SWC_MIN < DEVICE_SWC_MAX, "Wrong device swc boundaries");
_Static_assert(MAKE_DEVICE_SWC_ENCODING(0) > DEVICE_SWC_MIN, "Device swc encoding <= DEVICE_SWC_MIN");
_Static_assert(MAKE_DEVICE_SWC_ENCODING(UINT32_MAX) < DEVICE_SWC_MAX, "Device swc encoding >= DEVICE_SWC_MAX");

_Static_assert(DEVICE_STEP_MIN < DEVICE_STEP_MAX, "Wrong device step  boundaries");
_Static_assert(MAKE_DEVICE_STEP_ENCODING(0) > DEVICE_STEP_MIN, "Device step encoding <= DEVICE_STEP_MIN");
_Static_assert(MAKE_DEVICE_STEP_ENCODING(UINT32_MAX) < DEVICE_STEP_MAX, "Device step encoding >= DEVICE_STEP_MAX");

_Static_assert(DEVICE_VARIANT_MIN < DEVICE_VARIANT_MAX, "Wrong device variant boundaries");
_Static_assert(MAKE_DEVICE_VARIANT_ENCODING(0) >= DEVICE_VARIANT_MIN , "Device variant encoding < DEVICE_VARIANT_MIN");
_Static_assert(MAKE_DEVICE_VARIANT_ENCODING(UINT32_MAX) < DEVICE_VARIANT_MAX , "Device variant encoding >= DEVICE_VARIANT_MAX");

/*
 * Indicates which master has currently access to a certain peripheral.
 * hw_sys_sw_bsr[periph_id] == BSR_MASTER_X: Indicates that master X has currently access to that peripheral.
 */
__RETAINED uint32_t hw_sys_sw_bsr[BSR_PERIPH_ID_MAX];
__RETAINED static uint8_t hw_sys_sw_bsr_cnt[BSR_PERIPH_ID_MAX];

__RETAINED static hw_sys_reg_config_t hw_sys_reg_config[NUM_OF_REG_CONFIG_ENTRIES];
__RETAINED static uint32_t hw_sys_reg_num_of_config_entries;

__RETAINED uint32_t hw_sys_pd_com_acquire_cnt;
__RETAINED uint32_t hw_sys_pd_periph_acquire_cnt;
__RETAINED uint32_t hw_sys_pd_audio_acquire_cnt;

__RETAINED static uint32_t hw_sys_device_info_data;

void hw_sys_set_preferred_values(HW_PD pd)
{
        switch (pd) {
        case HW_PD_AON:
                CRG_TOP->BIAS_VREF_SEL_REG = 0x0037;
                REG_SET_MASKED(CRG_TOP, CLK_XTAL32K_REG, 0x00001E7E, 0x00000E48);
                break;
        case HW_PD_SYS:
                break;
        case HW_PD_TMR:
                REG_SET_MASKED(CRG_XTAL, CLKDBLR_CTRL1_REG, 0x007FFF00, 0x00410801);
                REG_SET_MASKED(CRG_XTAL, CLKDBLR_CTRL2_REG, 0x0000C000, 0x00004000);


                REG_SET_MASKED(CRG_XTAL, XTAL32M_SETTLE_REG, 0x000E0000, 0x000085A0);
                REG_SET_MASKED(CRG_XTAL, XTAL32M_START_REG, 0x00018FFF, 0x00068432);
                REG_SET_MASKED(CRG_XTAL, XTAL32M_TRIM_REG, 0x00007000, 0x0000F4A0);
                break;
        case HW_PD_COM:
                RAW_SET_MASKED(0x50020408, 0x00000010, 0x00007190);
                break;
        default:
                ASSERT_ERROR(pd < HW_PD_MAX);
                break;
        }
}

void hw_sys_setup_sw_cursor(void)
{
        if (dg_configUSE_SW_CURSOR == 1) {
                hw_sys_pd_com_enable();
                hw_gpio_set_pin_function(SW_CURSOR_PORT, SW_CURSOR_PIN, HW_GPIO_MODE_INPUT_PULLDOWN, HW_GPIO_FUNC_GPIO);
                hw_gpio_pad_latch_enable(SW_CURSOR_PORT, SW_CURSOR_PIN);
                hw_gpio_pad_latch_disable(SW_CURSOR_PORT, SW_CURSOR_PIN);
                hw_sys_pd_com_disable();
        }
}

void hw_sys_trigger_sw_cursor(void)
{
        if (dg_configUSE_SW_CURSOR == 1) {
                hw_sys_pd_com_enable();
                hw_gpio_configure_pin(SW_CURSOR_PORT, SW_CURSOR_PIN, HW_GPIO_MODE_OUTPUT, HW_GPIO_FUNC_GPIO, true);
                hw_gpio_pad_latch_enable(SW_CURSOR_PORT, SW_CURSOR_PIN);
                hw_clk_delay_usec(50);
                hw_gpio_set_pin_function(SW_CURSOR_PORT, SW_CURSOR_PIN, HW_GPIO_MODE_INPUT_PULLDOWN, HW_GPIO_FUNC_GPIO);
                hw_gpio_pad_latch_disable(SW_CURSOR_PORT, SW_CURSOR_PIN);
                hw_sys_pd_com_disable();
        }
}

__ALWAYS_RETAINED_CODE void hw_sys_assert_trigger_gpio(void)
{
        if (EXCEPTION_DEBUG == 1) {
                hw_pd_power_up_com();
                if (dg_configLP_CLK_SOURCE == LP_CLK_IS_DIGITAL) {
                        hw_clk_configure_ext32k_pins();
                }
                hw_gpio_pad_latch_enable_all();

                DBG_SET_HIGH(EXCEPTION_DEBUG, EXCEPTIONDBG);
        }
}


bool hw_sys_hw_bsr_try_lock(HW_BSR_MASTER_ID hw_bsr_master_id, HW_BSR_POS pos)
{
        ASSERT_ERROR((hw_bsr_master_id & SW_BSR_HW_BSR_MASK) == hw_bsr_master_id);
        ASSERT_WARNING((pos % 2) == 0);
        ASSERT_WARNING(pos < 31);

        MEMCTRL->BUSY_SET_REG = hw_bsr_master_id << pos;
        if (((MEMCTRL->BUSY_STAT_REG >> pos) & SW_BSR_HW_BSR_MASK) == hw_bsr_master_id) {
                return true;
        } else {
                return false;
        }
}

static HW_BSR_MASTER_ID hw_sys_sw_bsr_to_hw_bsr(SW_BSR_MASTER_ID sw_bsr_master_id) {
        switch (sw_bsr_master_id) {
        case SW_BSR_MASTER_SYSCPU:
                return HW_BSR_MASTER_SYSCPU;
                break;
        case SW_BSR_MASTER_CMAC:
                return HW_BSR_MASTER_CMAC;
                break;
        default:
                ASSERT_ERROR(0);
                return HW_BSR_MASTER_NONE;
                break;
        };
}

void hw_sys_hw_bsr_unlock(HW_BSR_MASTER_ID hw_bsr_master_id, HW_BSR_POS pos)
{
        ASSERT_ERROR((hw_bsr_master_id & SW_BSR_HW_BSR_MASK) == hw_bsr_master_id);
        ASSERT_ERROR(((MEMCTRL->BUSY_STAT_REG >> pos) & SW_BSR_HW_BSR_MASK) == hw_bsr_master_id);
        ASSERT_WARNING((pos % 2) == 0);
        ASSERT_WARNING(pos < 31);

        MEMCTRL->BUSY_RESET_REG = hw_bsr_master_id << pos;
}

void hw_sys_sw_bsr_init(void)
{
        int i;

        for (i = 0; i < sizeof(hw_sys_sw_bsr) / sizeof(hw_sys_sw_bsr[0]); i++) {
                hw_sys_sw_bsr[i] = SW_BSR_MASTER_NONE;
                hw_sys_sw_bsr_cnt[i] = 0;
        }

        MEMCTRL->BUSY_RESET_REG = MEMCTRL->BUSY_STAT_REG;
}

bool hw_sys_sw_bsr_try_acquire(SW_BSR_MASTER_ID sw_bsr_master_id, uint32_t periph_id)
{
        HW_BSR_MASTER_ID hw_bsr_master_id;
        bool acquired = false;

        ASSERT_ERROR (periph_id < BSR_PERIPH_ID_MAX);

        /*
         * Since hw_sys_hw_bsr_try_lock() / hw_sys_hw_bsr_unlock() sequence
         * can be executed from different contexts, the caller must
         * either disable interrupts or use a mutex to protect against
         * the following scenario:
         * -  Context1: hw_sys_hw_bsr_try_lock()
         * -  Context2: hw_sys_hw_bsr_try_lock()
         * -  Context2: hw_sys_hw_bsr_unlock()
         * -  Context1: HW BSR has been released before calling hw_sys_hw_bsr_unlock()
         */

        hw_bsr_master_id = hw_sys_sw_bsr_to_hw_bsr(sw_bsr_master_id);

        while (!hw_sys_hw_bsr_try_lock(hw_bsr_master_id, HW_BSR_SW_POS)) {}

        if ((hw_sys_sw_bsr[periph_id] == SW_BSR_MASTER_NONE) || (hw_sys_sw_bsr[periph_id] == sw_bsr_master_id)) {
                /* Update SW BSR */
                hw_sys_sw_bsr[periph_id] = sw_bsr_master_id;
                hw_sys_sw_bsr_cnt[periph_id]++;
                acquired = true;
        }

        hw_sys_hw_bsr_unlock(hw_bsr_master_id, HW_BSR_SW_POS);

        return acquired;
}

bool hw_sys_sw_bsr_acquired(SW_BSR_MASTER_ID sw_bsr_master_id, uint32_t periph_id)
{
        HW_BSR_MASTER_ID hw_bsr_master_id;
        bool acquired = false;

        hw_bsr_master_id = hw_sys_sw_bsr_to_hw_bsr(sw_bsr_master_id);

        while (!hw_sys_hw_bsr_try_lock(hw_bsr_master_id, HW_BSR_SW_POS)) {}

        if (hw_sys_sw_bsr[periph_id] == sw_bsr_master_id) {
                acquired = true;
        }

        hw_sys_hw_bsr_unlock(hw_bsr_master_id, HW_BSR_SW_POS);

        return acquired;
}

void hw_sys_sw_bsr_release(SW_BSR_MASTER_ID sw_bsr_master_id, uint32_t periph_id)
{
        HW_BSR_MASTER_ID hw_bsr_master_id;

        hw_bsr_master_id = hw_sys_sw_bsr_to_hw_bsr(sw_bsr_master_id);

        while (!hw_sys_hw_bsr_try_lock(hw_bsr_master_id, HW_BSR_SW_POS)) {}

        ASSERT_ERROR (hw_sys_sw_bsr[periph_id] == sw_bsr_master_id);
        ASSERT_ERROR (hw_sys_sw_bsr_cnt[periph_id]);

        if (--hw_sys_sw_bsr_cnt[periph_id] == 0) {
                hw_sys_sw_bsr[periph_id] = SW_BSR_MASTER_NONE;
        }

        hw_sys_hw_bsr_unlock(hw_bsr_master_id, HW_BSR_SW_POS);
}

__RETAINED_CODE void hw_sys_pd_com_enable(void)
{
        GLOBAL_INT_DISABLE();
        ASSERT_ERROR((!hw_sys_pd_com_acquire_cnt) || !REG_GETF(CRG_TOP, PMU_CTRL_REG, COM_SLEEP));
        ASSERT_ERROR((hw_sys_pd_com_acquire_cnt) || REG_GETF(CRG_TOP, PMU_CTRL_REG, COM_SLEEP));
        if (++hw_sys_pd_com_acquire_cnt == 1) {
                hw_pd_power_up_com();
        }
        GLOBAL_INT_RESTORE();

        ASSERT_ERROR(REG_GETF(CRG_TOP, SYS_STAT_REG, COM_IS_UP));
}

__RETAINED_CODE void hw_sys_pd_com_disable(void)
{
        ASSERT_ERROR(!REG_GETF(CRG_TOP, PMU_CTRL_REG, COM_SLEEP));

        GLOBAL_INT_DISABLE();
        ASSERT_ERROR(hw_sys_pd_com_acquire_cnt);
        if (--hw_sys_pd_com_acquire_cnt == 0) {
                hw_pd_power_down_com();
        }
        GLOBAL_INT_RESTORE();
}

__RETAINED_CODE void hw_sys_pd_periph_enable(void)
{
        GLOBAL_INT_DISABLE();
        if (++hw_sys_pd_periph_acquire_cnt == 1) {
                hw_pd_power_up_periph();
        }
        GLOBAL_INT_RESTORE();

        ASSERT_ERROR(REG_GETF(CRG_TOP, SYS_STAT_REG, PER_IS_UP));
}

__RETAINED_CODE void hw_sys_pd_periph_disable(void)
{
        ASSERT_ERROR(!REG_GETF(CRG_TOP, PMU_CTRL_REG, PERIPH_SLEEP));

        GLOBAL_INT_DISABLE();
        ASSERT_ERROR(hw_sys_pd_periph_acquire_cnt);
        if (--hw_sys_pd_periph_acquire_cnt == 0) {
                hw_pd_power_down_periph();
        }
        GLOBAL_INT_RESTORE();
}

__RETAINED_CODE void hw_sys_pd_audio_enable(void)
{
        GLOBAL_INT_DISABLE();
        if (++hw_sys_pd_audio_acquire_cnt == 1) {
                hw_pd_power_up_aud();
        }
        GLOBAL_INT_RESTORE();

        ASSERT_ERROR(REG_GETF(CRG_TOP, SYS_STAT_REG, AUD_IS_UP));
}

__RETAINED_CODE void hw_sys_pd_audio_disable(void)
{
        ASSERT_ERROR(!REG_GETF(CRG_TOP, PMU_CTRL_REG, AUD_SLEEP));

        GLOBAL_INT_DISABLE();
        ASSERT_ERROR(hw_sys_pd_audio_acquire_cnt);
        if (--hw_sys_pd_audio_acquire_cnt == 0) {
                hw_pd_power_down_aud();
        }
        GLOBAL_INT_RESTORE();
}

uint32_t hw_sys_reg_add_config(const hw_sys_reg_config_t *config, uint32_t num_of_entries)
{
       ASSERT_ERROR(hw_sys_reg_num_of_config_entries + num_of_entries <= NUM_OF_REG_CONFIG_ENTRIES);

       uint32_t ret = hw_sys_reg_num_of_config_entries;

       for (uint32_t i = 0; i < num_of_entries; i++) {
               hw_sys_reg_config[hw_sys_reg_num_of_config_entries + i] = config[i];
       }
       hw_sys_reg_num_of_config_entries += num_of_entries;

       return ret;
}

hw_sys_reg_config_t *hw_sys_reg_get_config(uint32_t index)
{
        ASSERT_WARNING(index == 0 || index < hw_sys_reg_num_of_config_entries);

        return &hw_sys_reg_config[index];
}

void hw_sys_reg_modify_config(uint32_t index, __IO uint32_t *addr, uint32_t value)
{
        ASSERT_ERROR(index < hw_sys_reg_num_of_config_entries);

        hw_sys_reg_config[index].value = value;

        // Address must be written after value to prevent race condition with other hosts
        hw_sys_reg_config[index].addr = addr;
}

uint32_t *hw_sys_reg_get_num_of_config_entries(void)
{
        return &hw_sys_reg_num_of_config_entries;
}

__RETAINED_CODE void hw_sys_reg_apply_config(void)
{
        uint32_t *p = (uint32_t *)&hw_sys_reg_config;

        while (p < (uint32_t *)&hw_sys_reg_config + 2 * hw_sys_reg_num_of_config_entries) {
                *(uint32_t *)(*p) = *(p+1);
                p += 2;
        }
}

void hw_sys_apply_default_values(void)
{
        bool is_reg_trimmed[NUM_OF_REG_ADDR_IN_CS] = {false};

#if (dg_configUSE_SYS_TCS == 1)
        const uint32_t reg_in_cs[NUM_OF_REG_ADDR_IN_CS] = {
                (uint32_t)&CRG_TOP->CLK_RC32M_REG,
                (uint32_t)&CRG_XTAL->XTAL32M_TRIM_REG
        };

        // Check for plain register entries
        if (sys_tcs_reg_pairs_in_cs(reg_in_cs, NUM_OF_REG_ADDR_IN_CS, is_reg_trimmed)) {
                return;
        }
#endif

        if (!is_reg_trimmed[0]) {
                REG_SETF(CRG_TOP, CLK_RC32M_REG, RC32M_BIAS, dg_configDEFAULT_CLK_RC32M_REG_RC32M_BIAS__VALUE);
                REG_SETF(CRG_TOP, CLK_RC32M_REG, RC32M_RANGE, dg_configDEFAULT_CLK_RC32M_REG_RC32M_RANGE__VALUE);
                REG_SETF(CRG_TOP, CLK_RC32M_REG, RC32M_COSC, dg_configDEFAULT_CLK_RC32M_REG_RC32M_COSC__VALUE);
        }

        if (!is_reg_trimmed[1]) {
                REG_SETF(CRG_XTAL, XTAL32M_TRIM_REG, XTAL32M_TRIM,
                        dg_configDEFAULT_XTAL32M_TRIM_REG__XTAL32M_TRIM__VALUE);
        }

}

#if dg_configUSE_SYS_TCS
/* Get device variant ID as stored in TCS. */
static bool get_device_variant(uint32_t *variant)
{
        uint32_t *variantID = NULL;
        uint8_t size = 0;

        sys_tcs_get_custom_values(SYS_TCS_GROUP_CHIP_ID, &variantID, &size);

        // If the Device Variant entry has been successfully retrieved
        if ((size == 1) && (variantID != NULL)) {
                *variant = MAKE_DEVICE_VARIANT_ENCODING(variantID[0]);
                return true;
        }

        return false;
}
#endif /* dg_configUSE_SYS_TCS */

bool hw_sys_device_info_init(void)
{
        union {
                uint8_t  arr[4];
                uint32_t value;
        } device_chip_id;

        uint32_t revision = MAKE_DEVICE_REVISION_ENCODING((REG_GETF(CHIP_VERSION, CHIP_REVISION_REG, CHIP_REVISION) - 'A'));
        uint32_t swc = MAKE_DEVICE_SWC_ENCODING(REG_GETF(CHIP_VERSION, CHIP_SWC_REG, CHIP_SWC));
        uint32_t step = MAKE_DEVICE_STEP_ENCODING((REG_GETF(CHIP_VERSION, CHIP_TEST1_REG, CHIP_LAYOUT_REVISION) - 'A'));

        device_chip_id.arr[3] = REG_GETF(CHIP_VERSION, CHIP_ID1_REG, CHIP_ID1);
        device_chip_id.arr[2] = REG_GETF(CHIP_VERSION, CHIP_ID2_REG, CHIP_ID2);
        device_chip_id.arr[1] = REG_GETF(CHIP_VERSION, CHIP_ID3_REG, CHIP_ID3);
        device_chip_id.arr[0] = REG_GETF(CHIP_VERSION, CHIP_ID4_REG, CHIP_ID4);

        switch (device_chip_id.value) {
        case ASCII_2634:
                device_chip_id.value = DEVICE_CHIP_ID_2634;
                break;
        default:
                return false;
        }


        hw_sys_device_info_data = DA1459X | device_chip_id.value | revision | swc | step;
        return true;
}

#if dg_configUSE_SYS_TCS
bool hw_sys_device_variant_init(void)
{
        uint32_t variant = 0;

        if (!get_device_variant(&variant)) {
                return false;
        }

        hw_sys_device_info_data |= variant;
        return true;
}
#endif

bool hw_sys_device_info_check(uint32_t mask, uint32_t attribute)
{
        uint32_t attribute_masked = attribute & mask;
        uint32_t hw_sys_device_info_data_masked = hw_sys_device_info_data & mask;

        return (hw_sys_device_info_data_masked == attribute_masked);
}

uint32_t hw_sys_get_device_info(void)
{
        return hw_sys_device_info_data;
}

bool hw_sys_is_compatible_chip(void)
{
        if (hw_sys_device_info_check(DEVICE_CHIP_ID_MASK, DEVICE_CHIP_ID_2634)) {
                if (hw_sys_device_info_check(DEVICE_REVISION_MASK, DEVICE_REV_A) &&
                        hw_sys_device_info_check(DEVICE_SWC_MASK, DEVICE_SWC_2)) {
                        return true;
                }
        }

        return false;
}


/*
 * The re-mapped region is [Base + Offset, Base + Region_Size)
 * Therefore, from the above, the re-map region size = Region_Size - Offset
 * Note that according to the registers file the Base address depends on the Region_Size (base address is aligned to the region size)
 * e.g. the Base address cannot be 0x00A20000 (eFlash middle) and the region size be 256kB = 0x00040000 (eFlash full size)
 *
 * Since the re-mapped region is [Base + Offset, Base + Region_Size):
 * ---------------
 * Base + Offset         maps to address: 0
 * ...
 * Base + Region_Size    maps to address: Region_Size - Offset = (Base + Region_Size) - (Base + Offset), which is the upper boundary of the re-map region
 */
uint32_t hw_sys_get_physical_addr(uint32_t addr)
{
        uint32_t phy_addr = 0;
        HW_SYS_REMAP_ADDRESS_0 remap_addr0 = hw_sys_get_memory_remapping();

#if (dg_configUSE_HW_QSPI == 1)
        static const uint32 flash_region_sizes[] = {
             32 * 1024 * 1024,
             16 * 1024 * 1024,
             8 * 1024 * 1024,
             4 * 1024 * 1024,
             2 * 1024 * 1024,
             1 * 1024 * 1024,
             512 * 1024,
             256 * 1024,
        };
#endif

        static const uint32 eflash_region_sizes[] = {
             512 * 1024,
             512 * 1024,
             512 * 1024,
             512 * 1024,
             512 * 1024,
             256 * 1024,
             128 * 1024,
             64 * 1024,
        };

        switch (remap_addr0) {
        case HW_SYS_REMAP_ADDRESS_0_TO_EFLASH:
                {
                        /* Cache controller is considered enabled */
                        /* Take into account flash region base, offset and size */
                        uint32_t flash_region_base_offset = hw_cache_eflash_get_region_base() << REG_POS(CACHE, CACHE_EFLASH_REG, EFLASH_REGION_BASE);
                        uint32_t flash_region_offset = hw_cache_eflash_get_region_offset() << 2;
                        flash_region_base_offset += flash_region_offset;

                        uint32_t flash_region_size = eflash_region_sizes[hw_cache_eflash_get_region_size()];
                        uint32_t flash_remap_size = flash_region_size - flash_region_offset;

                        if (addr < flash_remap_size) {
                                /* Zero-based address */
                                phy_addr = flash_region_base_offset + addr;
                        } else {
                                phy_addr = addr;
                        }
                }
                break;
#if (dg_configUSE_HW_QSPI == 1)
        case HW_SYS_REMAP_ADDRESS_0_TO_QSPI_FLASH:
                {
                        /* Cache controller is considered enabled */
                        /* Take into account flash region base, offset and size */
                        uint32_t flash_region_base_offset = hw_cache_flash_get_region_base() << REG_POS(CACHE, CACHE_FLASH_REG, FLASH_REGION_BASE);
                        uint32_t flash_region_offset = hw_cache_flash_get_region_offset() << 2;
                        flash_region_base_offset += flash_region_offset;

                        uint32_t flash_region_size = flash_region_sizes[hw_cache_flash_get_region_size()];
                        uint32_t flash_remap_size = flash_region_size - flash_region_offset;

                        if (addr < flash_remap_size) {
                                /* Zero-based address */
                                phy_addr = flash_region_base_offset + addr;
                        } else {
                                phy_addr = addr;
                        }
                }
                break;
#endif
        case HW_SYS_REMAP_ADDRESS_0_TO_RAM:
                if (addr < MEMORY_SYSRAM_SIZE) {
                        phy_addr = MEMORY_SYSRAM_S_BASE + addr;
                } else {
                        phy_addr = addr;
                }
                break;
        default:
                ASSERT_ERROR(0);
        }

        // Return uncached address
        if (IS_EFLASH_ADDRESS(phy_addr)) {
                phy_addr += (MEMORY_EFLASH_S_BASE - MEMORY_EFLASH_BASE);
        }
#if (dg_configUSE_HW_QSPI == 1)
        else if (IS_QSPIF_ADDRESS(phy_addr)) {
                phy_addr += (MEMORY_QSPIF_S_BASE - MEMORY_QSPIF_BASE);
        }
#endif
        else if (IS_SYSRAM_ADDRESS(phy_addr)) {
                phy_addr += (MEMORY_SYSRAM_S_BASE - MEMORY_SYSRAM_BASE);
        }
        return phy_addr;
}

void hw_sys_enable_cmac_mem_protection(void)
{
#if defined(CONFIG_USE_BLE)
#if dg_configUSE_HW_MPU

        GLOBAL_INT_DISABLE();

        /*
         * Apply MPU configuration for CMAC code region only if MPU is not enabled,
         * that is, MPU has been already configured before reaching here.
         */
        if (!hw_mpu_is_enabled()) {
                mpu_region_config region_cfg;

                /* Set ro_region to CMAC code section. CMAC code location is described by CMI_CODE_BASE_ADDR.
                 * */
                region_cfg.start_addr = REG_GETF(MEMCTRL, CMI_CODE_BASE_REG, CMI_CODE_BASE_ADDR) & ~((uint32_t) MPU_END_ADDRESS_MASK);

                region_cfg.end_addr = (region_cfg.start_addr + REG_GETF(MEMCTRL, CMI_DATA_BASE_REG, CMI_DATA_BASE_ADDR) - 1)
                        | MPU_END_ADDRESS_MASK;
                region_cfg.shareability = HW_MPU_SH_NS;
                region_cfg.access_permissions = HW_MPU_AP_RO;
                region_cfg.attributes = HW_MPU_ATTR_NORMAL;
                region_cfg.execute_never = HW_MPU_XN_TRUE;
                hw_mpu_config_region(dg_configCMAC_PROTECT_REGION, &region_cfg);
                hw_mpu_enable(true);
        }

        GLOBAL_INT_RESTORE();
#endif /* dg_configUSE_HW_MPU */
#endif /* CONFIG_USE_BLE */

        return;
}

void hw_sys_enable_ivt_mem_protection(void)
{
#if dg_configUSE_HW_MPU

        GLOBAL_INT_DISABLE();

        /*
         * Apply MPU configuration for IVT region only if MPU is not enabled,
         * that is, MPU has been already configured before reaching here.
         */
        if (!hw_mpu_is_enabled()) {
                mpu_region_config region_cfg;

                /* Set ro_region to IVT section. IVT code starts at the beginning of RAM cell
                 * The MPU is an optional ARM CM33 feature supported in DA14yyx SoC families that
                 * enables protecting loosely defined regions of system RAM memory through enforcing
                 * privilege and access rules per region. All MPU LLD terminology is based on the ARM
                 * CM33 nomenclature.0 and stops at length
                 * of 0xFF.
                 * as MPU region that allows
                 * any RO access (privileged & unprivileged).
                 * RAM cells    RAM Size (KB)     Main Use      IVT start       IVT end
                 * RAM 0            8           IVT & Others       0x0            0xFF
                 * */
                region_cfg.start_addr = (0x0) & ~((uint32_t) MPU_END_ADDRESS_MASK);

                region_cfg.end_addr = (region_cfg.start_addr + ((uint32_t)&__Vectors_Size) -1) | MPU_END_ADDRESS_MASK;
                region_cfg.shareability = HW_MPU_SH_NS;
                region_cfg.access_permissions = HW_MPU_AP_RO;
                region_cfg.attributes = HW_MPU_ATTR_NORMAL;
                region_cfg.execute_never = HW_MPU_XN_TRUE;
                hw_mpu_config_region(dg_configIVT_PROTECT_REGION, &region_cfg);
                hw_mpu_enable(true);
        }

        GLOBAL_INT_RESTORE();
#endif /* dg_configUSE_HW_MPU */
        return;
}

#if dg_configHW_FCU_WAIT_CYCLES_MODE

__STATIC_FORCEINLINE cpu_clk_t get_cpu_freq(sys_clk_is_t clk, ahb_div_t div)
{
        uint8_t adapted_clk = 1;

        if (clk == SYS_CLK_IS_DBLR) { // doubler frequency is 64Mhz, twice the freq of other sys_clks
                adapted_clk = 2;
        }

        return ( 32 >> div ) * adapted_clk;
}

__STATIC_FORCEINLINE HW_PMU_VDD_VOLTAGE get_active_vdd_voltage_level(void)
{
       return REG_GETF(CRG_TOP, POWER_LEVEL_REG, VDD_LEVEL_ACTIVE);
}

#if (dg_configHW_FCU_WAIT_CYCLES_MODE == 2)
__STATIC_FORCEINLINE void hw_sys_fcu_wait_cycles_monitor_start(sys_clk_is_t clk, ahb_div_t div, HW_PMU_VDD_VOLTAGE vdd, cpu_clk_t freq)
{
        ASSERT_WARNING(_report_requests >= 0);
        _report_requests++;

        if (_old_vdd == HW_PMU_VDD_VOLTAGE_INVALID) {
                _old_vdd = vdd;
        }

        if (_old_div == ahb_invalid) {
                _old_div = div;
        }

        /* track clock for debugging purposes */
        if (_old_clk == SYS_CLK_IS_INVALID) {
                _old_clk = clk;
        }

        if (_old_freq == 0) {
                _old_freq = freq;
        }

        ASSERT_WARNING((div == _old_div) && (vdd == _old_vdd) && (freq == _old_freq));
}

__STATIC_FORCEINLINE void hw_sys_fcu_wait_cycles_monitor_stop(sys_clk_is_t clk, ahb_div_t div, HW_PMU_VDD_VOLTAGE vdd, cpu_clk_t cpu_freq)
{
        ASSERT_WARNING(_report_requests >= 1);
        _report_requests--;

        _old_clk = clk;
        _old_div = div;
        _old_vdd = vdd;
        _old_freq = cpu_freq;
}

#endif /* #if (dg_configHW_FCU_WAIT_CYCLES_MODE == 2 */



__ALWAYS_RETAINED_CODE void hw_sys_fcu_set_max_wait_cycles(void)
{
        GLOBAL_INT_DISABLE();

#if (dg_configHW_FCU_WAIT_CYCLES_MODE == 2)
        HW_PMU_VDD_VOLTAGE vdd = get_active_vdd_voltage_level();
        ahb_div_t div = hw_clk_get_hclk_div();
        sys_clk_is_t clk = hw_clk_get_sysclk();
        cpu_clk_t cpu_freq = get_cpu_freq(clk, div);

        hw_sys_fcu_wait_cycles_monitor_start(clk, div, vdd, cpu_freq);
#endif
        hw_fcu_set_wait_cycles(HW_FCU_WAIT_CYCLES_7);

        GLOBAL_INT_RESTORE();
}

__ALWAYS_RETAINED_CODE void hw_sys_fcu_set_optimum_wait_cycles(void)
{
        /*
         * HCLK = 64 MHz, VDD = 1.2V -> 1 wait cycles
         * HCLK = 32 MHz, VDD = 1.2V -> 0 wait cycles
         * HCLK = 32 MHz, VDD = 0.9V -> 2 wait cycles (default conditions on power up)
         * HCLK = 16 MHz, VDD = 1.2V -> 0 wait cycles
         * HCLK = 16 MHz, VDD = 0.9V -> 1 wait cycles
         * HCLK = slower than above -> 0 wait cycles
         */

        GLOBAL_INT_DISABLE();

        ahb_div_t div = hw_clk_get_hclk_div();
        HW_PMU_VDD_VOLTAGE vdd = get_active_vdd_voltage_level();
        sys_clk_is_t clk = hw_clk_get_sysclk();
        cpu_clk_t cpu_freq = get_cpu_freq(clk, div);
        HW_FCU_WAIT_CYCLES wait_cycles = HW_FCU_WAIT_CYCLES_7;

#if (dg_configHW_FCU_WAIT_CYCLES_MODE == 2)
        hw_sys_fcu_wait_cycles_monitor_stop(clk, div, vdd, cpu_freq);
#endif

        if (vdd >= HW_PMU_VDD_VOLTAGE_1V20) {
                if (cpu_freq == cpuclk_64M) {
                        wait_cycles = HW_FCU_WAIT_CYCLES_1;
                } else {
                        wait_cycles = HW_FCU_WAIT_CYCLES_0;
                }

        } else {
                if (cpu_freq == cpuclk_32M) {
                        wait_cycles = HW_FCU_WAIT_CYCLES_2;
                } else if (cpu_freq == cpuclk_16M) {
                        wait_cycles = HW_FCU_WAIT_CYCLES_1;
                } else {
                        wait_cycles = HW_FCU_WAIT_CYCLES_0;
                }
        }

        hw_fcu_set_wait_cycles(wait_cycles);

        GLOBAL_INT_RESTORE();
}

#endif /* dg_configHW_FCU_WAIT_CYCLES_MODE */
#endif /* dg_configUSE_HW_SYS */

