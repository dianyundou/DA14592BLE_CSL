/**
****************************************************************************************
*
* @file hw_pmu_da1459x.c
*
* @brief Power Manager Unit for DA1459x
*
* Copyright (C) 2019-2023 Renesas Electronics Corporation and/or its affiliates.
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

#if dg_configUSE_HW_PMU


#include "hw_pmu.h"
#include "hw_bod.h"
#include "hw_clk.h"
#include <string.h>

/*
* dependencies -----------
*                   /
* rail-------------<
*                   \
* dependants -------------
*/

#ifndef HW_PMU_SANITY_CHECKS_ENABLE
#       if dg_configIMAGE_SETUP == DEVELOPMENT_MODE
#               define HW_PMU_SANITY_CHECKS_ENABLE 1
#       else
#               define HW_PMU_SANITY_CHECKS_ENABLE 0
#       endif
#endif

/* 1V8 dependencies check masks */
typedef enum {
        HW_PMU_CHK_1V8_LDO_IO_MSK               = (1 << 0),
        HW_PMU_CHK_1V8_LDO_IO_RET_ACTIVE_MSK    = (1 << 1),
        HW_PMU_CHK_1V8_LDO_IO_RET_SLEEP_MSK     = (1 << 2),
        HW_PMU_CHK_1V8_LDO_IO_BYPASS_MSK        = (1 << 3),
        HW_PMU_CHK_1V8_LDO_IO_RET_BYPASS_MSK    = (1 << 4)
} HW_PMU_DEPENDENCY_CHK_1V8_MSK;

/* VDCDC dependencies check masks */
typedef enum {
        HW_PMU_CHK_VDCDC_LDO_LOW_MSK            = (1 << 0),
        HW_PMU_CHK_VDCDC_LDO_LOW_RET_MSK        = (1 << 1),
        HW_PMU_CHK_VDCDC_DCDC_ACTIVE_MSK        = (1 << 2),
        HW_PMU_CHK_VDCDC_DCDC_SLEEP_MSK         = (1 << 3)
} HW_PMU_DEPENDENCY_CHK_VDCDC_MSK;

/* VDCDC dependants check masks */
typedef enum {
        HW_PMU_CHK_VDCDC_RF_LDO_MSK                     = (1 << 0),
        HW_PMU_CHK_VDCDC_LDO_CORE_MSK                   = (1 << 1),
        HW_PMU_CHK_VDCDC_LDO_CORE_RET_ACTIVE_MSK        = (1 << 2),
        HW_PMU_CHK_VDCDC_LDO_CORE_RET_SLEEP_MSK         = (1 << 3),
        HW_PMU_CHK_VDCDC_LDO_XTAL32M_MSK                = (1 << 4),
        HW_PMU_CHK_VDCDC_LDO_SDADC_MSK                  = (1 << 5),
        HW_PMU_CHK_VDCDC_LDO_GPADC_MSK                  = (1 << 6),
        HW_PMU_CHK_VDCDC_RC32M_MSK                      = (1 << 7),
        HW_PMU_CHK_VDCDC_RCX_MSK                        = (1 << 8),
        HW_PMU_CHK_VDCDC_LDO_CORE_DROPOUT_1V10_MSK      = (1 << 9),
        HW_PMU_CHK_VDCDC_LDO_CORE_DROPOUT_1V15_MSK      = (1 << 10),
        HW_PMU_CHK_VDCDC_LDO_CORE_DROPOUT_1V20_MSK      = (1 << 11),
        HW_PMU_CHK_VDCDC_LDO_CORE_DROPOUT_1V25_MSK      = (1 << 12),
        HW_PMU_CHK_VDCDC_LDO_CORE_DROPOUT_1V30_MSK      = (1 << 13),
        HW_PMU_CHK_VDCDC_LDO_CORE_DROPOUT_1V35_MSK      = (1 << 14),
        HW_PMU_CHK_VDCDC_LDO_CORE_DROPOUT_1V40_MSK      = (1 << 15)
} HW_PMU_DEPENDANT_CHK_VDCDC_MSK;

/* VDD dependencies check masks */
typedef enum {
        HW_PMU_CHK_VDD_LDO_CORE_MSK                     = (1 << 0),
        HW_PMU_CHK_VDD_LDO_CORE_RET_MSK                 = (1 << 1),
        HW_PMU_CHK_VDD_LDO_CORE_DROPOUT_1V25_MSK        = (1 << 2),
        HW_PMU_CHK_VDD_LDO_CORE_DROPOUT_1V20_MSK        = (1 << 3),
        HW_PMU_CHK_VDD_LDO_CORE_DROPOUT_1V15_MSK        = (1 << 4),
        HW_PMU_CHK_VDD_LDO_CORE_DROPOUT_1V10_MSK        = (1 << 5),
        HW_PMU_CHK_VDD_LDO_CORE_DROPOUT_1V05_MSK        = (1 << 6),
        HW_PMU_CHK_VDD_LDO_CORE_DROPOUT_1V00_MSK        = (1 << 7),
        HW_PMU_CHK_VDD_LDO_CORE_DROPOUT_0V95_MSK        = (1 << 8)
} HW_PMU_DEPENDENCY_CHK_VDD_MSK;

/* VDD dependants check masks */
typedef enum {
        HW_PMU_CHK_VDD_XTAL32K_MSK              = (1 << 0),
        HW_PMU_CHK_VDD_FAST_WAKEUP_MSK          = (1 << 1),
        HW_PMU_CHK_VDD_RFHP_MSK                 = (1 << 2),
        HW_PMU_CHK_VDD_XTAL32M_DBLR_MSK         = (1 << 3),
        HW_PMU_CHK_VDD_EFLASH_OPS_MSK           = (1 << 4)
} HW_PMU_DEPENDANT_CHK_VDD_MSK;

__STATIC_INLINE void _1v8_ldo_io_enable(void)
{
        REG_SET_BIT(CRG_TOP, POWER_CTRL_REG, LDO_IO_ENABLE);
}

__STATIC_INLINE void _1v8_ldo_io_disable(void)
{
        REG_CLR_BIT(CRG_TOP, POWER_CTRL_REG, LDO_IO_ENABLE);
}

__STATIC_INLINE bool is_ldo_io_enabled(void)
{
        return REG_GETF(CRG_TOP, POWER_CTRL_REG, LDO_IO_ENABLE);
}

__STATIC_INLINE void _1v8_ldo_io_ret_active_enable(void)
{
        uint32_t tmp;

        tmp = CRG_TOP->POWER_CTRL_REG;
        REG_SET_FIELD(CRG_TOP, POWER_CTRL_REG, LDO_IO_RET_VREF_ENABLE, tmp, 1);
        REG_SET_FIELD(CRG_TOP, POWER_CTRL_REG, LDO_IO_RET_ENABLE_ACTIVE, tmp, 1);
        CRG_TOP->POWER_CTRL_REG = tmp;
}

__STATIC_INLINE void _1v8_ldo_io_ret_active_disable(void)
{
        REG_CLR_BIT(CRG_TOP, POWER_CTRL_REG, LDO_IO_RET_ENABLE_ACTIVE);
}

__STATIC_INLINE bool is_ldo_io_ret_active_enabled(void)
{
        return REG_GETF(CRG_TOP, POWER_CTRL_REG, LDO_IO_RET_ENABLE_ACTIVE);
}

__STATIC_INLINE void _1v8_ldo_io_ret_sleep_enable(void)
{
        uint32_t tmp;

        tmp = CRG_TOP->POWER_CTRL_REG;
        REG_SET_FIELD(CRG_TOP, POWER_CTRL_REG, LDO_IO_RET_VREF_ENABLE, tmp, 1);
        REG_SET_FIELD(CRG_TOP, POWER_CTRL_REG, LDO_IO_RET_ENABLE_SLEEP, tmp, 1);
        CRG_TOP->POWER_CTRL_REG = tmp;
}

__STATIC_INLINE void _1v8_ldo_io_ret_sleep_disable(void)
{
        REG_CLR_BIT(CRG_TOP, POWER_CTRL_REG, LDO_IO_RET_ENABLE_SLEEP);
}

__STATIC_INLINE bool is_ldo_io_ret_sleep_enabled(void)
{
        return REG_GETF(CRG_TOP, POWER_CTRL_REG, LDO_IO_RET_ENABLE_SLEEP);
}

__STATIC_INLINE void _1v8_ldo_io_bypass_active_enable(void)
{
        REG_SET_BIT(CRG_TOP, POWER_CTRL_REG, LDO_IO_BYPASS_ACTIVE);
}

__STATIC_INLINE void _1v8_ldo_io_bypass_active_disable(void)
{
        REG_CLR_BIT(CRG_TOP, POWER_CTRL_REG, LDO_IO_BYPASS_ACTIVE);
}

__STATIC_INLINE bool is_ldo_io_bypass_active_enabled(void)
{
        return REG_GETF(CRG_TOP, POWER_CTRL_REG, LDO_IO_BYPASS_ACTIVE);
}

__STATIC_INLINE void _1v8_ldo_io_bypass_sleep_enable(void)
{
        REG_SET_BIT(CRG_TOP, POWER_CTRL_REG, LDO_IO_BYPASS_SLEEP);
}

__STATIC_INLINE void _1v8_ldo_io_bypass_sleep_disable(void)
{
        REG_CLR_BIT(CRG_TOP, POWER_CTRL_REG, LDO_IO_BYPASS_SLEEP);
}

__STATIC_INLINE bool is_ldo_io_bypass_sleep_enabled(void)
{
        return REG_GETF(CRG_TOP, POWER_CTRL_REG, LDO_IO_BYPASS_SLEEP);
}

__STATIC_INLINE void vdcdc_ldo_low_enable(void)
{
        REG_SET_BIT(CRG_TOP, POWER_CTRL_REG, LDO_LOW_ENABLE_ACTIVE);
}

__STATIC_INLINE void vdcdc_ldo_low_disable(void)
{
        REG_CLR_BIT(CRG_TOP, POWER_CTRL_REG, LDO_LOW_ENABLE_ACTIVE);
}

__STATIC_INLINE bool is_ldo_low_enabled(void)
{
        return REG_GETF(CRG_TOP, POWER_CTRL_REG, LDO_LOW_ENABLE_ACTIVE);
}

__STATIC_INLINE void vdcdc_ldo_low_ret_enable(void)
{
        REG_SET_BIT(CRG_TOP, POWER_CTRL_REG, LDO_LOW_ENABLE_SLEEP);
}

__STATIC_INLINE void vdcdc_ldo_low_ret_disable(void)
{
        REG_CLR_BIT(CRG_TOP, POWER_CTRL_REG, LDO_LOW_ENABLE_SLEEP);
}

__STATIC_INLINE bool is_ldo_low_ret_enabled(void)
{
        return REG_GETF(CRG_TOP, POWER_CTRL_REG, LDO_LOW_ENABLE_SLEEP);
}

__STATIC_INLINE void vdcdc_dcdc_enable(void)
{
        REG_SET_BIT(DCDC, DCDC_CTRL_REG, DCDC_ENABLE);
}

__STATIC_INLINE void vdcdc_dcdc_disable(void)
{
        REG_CLR_BIT(DCDC, DCDC_CTRL_REG, DCDC_ENABLE);
}

__STATIC_INLINE bool is_dcdc_enabled(void)
{
        return REG_GETF(DCDC, DCDC_CTRL_REG, DCDC_ENABLE);
}

__STATIC_INLINE void vdcdc_dcdc_sleep_enable(void)
{
        REG_SET_BIT(DCDC, DCDC_CTRL_REG, DCDC_ENABLE);
        REG_SET_BIT(CRG_TOP, POWER_CTRL_REG, DCDC_ENABLE_SLEEP);
}

__STATIC_INLINE void vdcdc_dcdc_sleep_disable(void)
{
        REG_CLR_BIT(CRG_TOP, POWER_CTRL_REG, DCDC_ENABLE_SLEEP);
}

__STATIC_INLINE bool is_dcdc_sleep_enabled(void)
{
        return (is_dcdc_enabled() && REG_GETF(CRG_TOP, POWER_CTRL_REG, DCDC_ENABLE_SLEEP));
}

__STATIC_INLINE void vdcdc_set_voltage_level(HW_PMU_VDCDC_VOLTAGE voltage)
{
        REG_SETF(CRG_TOP, POWER_LEVEL_REG, VDCDC_LEVEL, voltage);
        // Wait for the rail level change detection and allow DCDC_OK_CLR timeout
        GLOBAL_INT_DISABLE();
        hw_clk_delay_usec(6);
        GLOBAL_INT_RESTORE();
        if (is_dcdc_enabled()) {
                while (RAW_GETF(0x50000308, 0x1UL) == 0);
        }
}

__STATIC_INLINE HW_PMU_VDCDC_VOLTAGE vdcdc_get_voltage_level(void)
{
        return REG_GETF(CRG_TOP, POWER_LEVEL_REG, VDCDC_LEVEL);
}

__STATIC_INLINE void vdd_ldo_core_enable(void)
{
        REG_SET_BIT(CRG_TOP, POWER_CTRL_REG, LDO_CORE_ENABLE);
}

__STATIC_INLINE void vdd_ldo_core_disable(void)
{
        REG_CLR_BIT(CRG_TOP, POWER_CTRL_REG, LDO_CORE_ENABLE);
}

__STATIC_INLINE bool is_ldo_core_enabled(void)
{
        return REG_GETF(CRG_TOP, POWER_CTRL_REG, LDO_CORE_ENABLE);
}

__STATIC_INLINE void vdd_ldo_core_ret_active_enable(void)
{
        uint32_t tmp;

        tmp = CRG_TOP->POWER_CTRL_REG;
        REG_SET_FIELD(CRG_TOP, POWER_CTRL_REG, LDO_CORE_RET_VREF_ENABLE, tmp, 1);
        REG_SET_FIELD(CRG_TOP, POWER_CTRL_REG, LDO_CORE_RET_ENABLE_ACTIVE, tmp, 1);
        CRG_TOP->POWER_CTRL_REG = tmp;
}

__STATIC_INLINE void vdd_ldo_core_ret_active_disable(void)
{
        REG_CLR_BIT(CRG_TOP, POWER_CTRL_REG, LDO_CORE_RET_ENABLE_ACTIVE);
}

__STATIC_INLINE bool is_ldo_core_ret_active_enabled(void)
{
        return REG_GETF(CRG_TOP, POWER_CTRL_REG, LDO_CORE_RET_ENABLE_ACTIVE);
}

__STATIC_INLINE void vdd_ldo_core_ret_sleep_enable(void)
{
        uint32_t tmp;

        tmp = CRG_TOP->POWER_CTRL_REG;
        REG_SET_FIELD(CRG_TOP, POWER_CTRL_REG, LDO_IO_RET_VREF_ENABLE, tmp, 1);
        REG_SET_FIELD(CRG_TOP, POWER_CTRL_REG, LDO_IO_RET_ENABLE_SLEEP, tmp, 1);
        CRG_TOP->POWER_CTRL_REG = tmp;
}

__STATIC_INLINE void vdd_ldo_core_ret_sleep_disable(void)
{
        REG_CLR_BIT(CRG_TOP, POWER_CTRL_REG, LDO_CORE_RET_ENABLE_SLEEP);
}

__STATIC_INLINE bool is_ldo_core_ret_sleep_enabled(void)
{
        return REG_GETF(CRG_TOP, POWER_CTRL_REG, LDO_CORE_RET_ENABLE_SLEEP);
}

__STATIC_INLINE void vdd_set_active_voltage_level(HW_PMU_VDD_VOLTAGE voltage)
{
        REG_SETF(CRG_TOP, POWER_LEVEL_REG, VDD_LEVEL_ACTIVE, voltage);
        // Wait for the rail level change detection and then check the OK bit
        GLOBAL_INT_DISABLE();
        hw_clk_delay_usec(4);
        GLOBAL_INT_RESTORE();
        while (REG_GETF(CRG_TOP, ANA_STATUS_REG, LDO_CORE_OK) == 0);
}

__STATIC_INLINE HW_PMU_VDD_VOLTAGE vdd_get_active_voltage_level(void)
{
       return REG_GETF(CRG_TOP, POWER_LEVEL_REG, VDD_LEVEL_ACTIVE);
}

__STATIC_INLINE void vdd_set_sleep_voltage_level(HW_PMU_VDD_VOLTAGE voltage)
{
        REG_SETF(CRG_TOP, POWER_LEVEL_REG, VDD_LEVEL_SLEEP, voltage);
}

__STATIC_INLINE bool is_xtal32m_ldo_on(void)
{
        return REG_GETF(CRG_XTAL, XTAL32M_STAT0_REG, XTAL32M_LDO_OK);
}

__STATIC_INLINE bool is_sdadc_ldo_on(void)
{
        return REG_GETF(SDADC, SDADC_CTRL_REG, SDADC_LDO_OK);
}

__STATIC_INLINE bool is_gpadc_ldo_on(void)
{
        return REG_GETF(CRG_TOP, ANA_STATUS_REG, LDO_GPADC_OK);
}

__STATIC_INLINE bool is_rc32m_enabled(void)
{
        return (REG_GETF(CRG_TOP, CLK_CTRL_REG, RUNNING_AT_RC32M) ||
                REG_GETF(CRG_TOP, CLK_RC32M_REG, RC32M_ENABLE));
}

__STATIC_INLINE bool is_dblr64m_enabled(void)
{
        return (REG_GETF(CRG_TOP, CLK_CTRL_REG, RUNNING_AT_DBLR64M) ||
                REG_GETF(CRG_XTAL, CLKDBLR_STATUS_REG, OUTPUT_READY));
}

__STATIC_INLINE bool is_rcx_enabled(void)
{
        return REG_GETF(CRG_TOP, CLK_RCX_REG, RCX_ENABLE);
}

__STATIC_INLINE bool is_rcx_lp_clk(void)
{
        return (REG_GETF(CRG_TOP, CLK_CTRL_REG, LP_CLK_SEL) == LP_CLK_IS_RCX);
}

__STATIC_INLINE bool is_xtal32k_enabled(void)
{
        return REG_GETF(CRG_TOP, CLK_XTAL32K_REG, XTAL32K_ENABLE);
}

__STATIC_INLINE bool is_xtal32k_lp_clk(void)
{
        uint8_t lpclk = REG_GETF(CRG_TOP, CLK_CTRL_REG, LP_CLK_SEL);

        return ((lpclk == LP_CLK_IS_XTAL32K ) || (lpclk == LP_CLK_IS_EXTERNAL ));
}

__STATIC_INLINE bool is_eflash_ops_on(void)
{
        uint32_t mask = REG_MSK(FCU, FLASH_CTRL_REG, PROG_ERS) |
                        REG_MSK(FCU, FLASH_CTRL_REG, PROG_WRS) |
                        REG_MSK(FCU, FLASH_CTRL_REG, PROG_RMIN) |
                        REG_MSK(FCU, FLASH_CTRL_REG, ERASE_SUSPEND_STAT);

        return ((FCU->FLASH_CTRL_REG & mask) != 0);
}

__STATIC_INLINE bool is_fast_wakeup_mode_enabled(void)
{
        return REG_GETF(CRG_TOP, PMU_SLEEP_REG, FAST_WAKEUP);
}

__STATIC_INLINE HW_PMU_ERROR_CODE check_1v8_dependencies_active(HW_PMU_DEPENDENCY_CHK_1V8_MSK mask)
{
#if HW_PMU_SANITY_CHECKS_ENABLE == 1
        if (mask & HW_PMU_CHK_1V8_LDO_IO_MSK) {
                if (REG_GETF(CRG_TOP, ANA_STATUS_REG, LDO_IO_OK) || is_ldo_io_enabled()) {
                        /* LDO_IO is enabled or LDO_IO_RET is enabled in active mode. */
                        return HW_PMU_ERROR_NOERROR;
                }
        }

        if (mask & HW_PMU_CHK_1V8_LDO_IO_RET_ACTIVE_MSK) {
                if (REG_GETF(CRG_TOP, ANA_STATUS_REG, LDO_IO_OK) || is_ldo_io_ret_active_enabled()) {
                        /* LDO_IO_RET is enabled in active mode or LDO_IO is enabled. */
                        return HW_PMU_ERROR_NOERROR;
                }
        }

        if (mask & HW_PMU_CHK_1V8_LDO_IO_BYPASS_MSK) {
                if (is_ldo_io_bypass_active_enabled()) {
                        /* LDO_IO is set to bypass. The 1V8 is powered by VBAT rail.
                         * Considering an R on bypass of 6 Ohm and Imax of 20mA,
                         * the expected voltage drop compared to VBAT is around 120mV
                         */
                        return HW_PMU_ERROR_NOERROR;
                }
        }

        return HW_PMU_ERROR_NOT_ENOUGH_POWER;
#else
        return HW_PMU_ERROR_NOERROR;
#endif
}

__STATIC_INLINE HW_PMU_ERROR_CODE check_1v8_dependencies_sleep(HW_PMU_DEPENDENCY_CHK_1V8_MSK mask)
{
#if HW_PMU_SANITY_CHECKS_ENABLE == 1
        if (mask & HW_PMU_CHK_1V8_LDO_IO_RET_SLEEP_MSK) {
                if (is_ldo_io_ret_sleep_enabled()) {
                        /* LDO_VBAT_IO_RET is enabled in sleep mode. */
                        return HW_PMU_ERROR_NOERROR;
                }
        }

        if (mask & HW_PMU_CHK_1V8_LDO_IO_RET_BYPASS_MSK) {
                if (is_ldo_io_bypass_sleep_enabled()) {
                        /* LDO_IO is set to bypass. The 1V8 is powered by VBAT rail.
                         * Considering an R on bypass of 6 Ohm and Imax of 2mA,
                         * the expected voltage drop compared to VBAT is around 12mV
                         */
                        return HW_PMU_ERROR_NOERROR;
                }
        }

        return HW_PMU_ERROR_NOT_ENOUGH_POWER;
#else
        return HW_PMU_ERROR_NOERROR;
#endif
}

__STATIC_INLINE HW_PMU_ERROR_CODE check_vdcdc_dependencies_active(HW_PMU_DEPENDENCY_CHK_VDCDC_MSK mask)
{
#if HW_PMU_SANITY_CHECKS_ENABLE == 1
        if (mask & HW_PMU_CHK_VDCDC_LDO_LOW_MSK) {
                if (REG_GETF(CRG_TOP, ANA_STATUS_REG, LDO_LOW_OK) || is_ldo_low_enabled()) {
                        /* LDO_LOW is enabled. */
                        return HW_PMU_ERROR_NOERROR;
                }
        }

        if (mask & HW_PMU_CHK_VDCDC_DCDC_ACTIVE_MSK) {
                if (is_dcdc_enabled()) {
                        /* DCDC is enabled. */
                        return HW_PMU_ERROR_NOERROR;
                }
        }

        return HW_PMU_ERROR_NOT_ENOUGH_POWER;
#else
        return HW_PMU_ERROR_NOERROR;
#endif
}

__STATIC_INLINE HW_PMU_ERROR_CODE check_vdcdc_dependencies_sleep(HW_PMU_DEPENDENCY_CHK_VDCDC_MSK mask)
{
#if HW_PMU_SANITY_CHECKS_ENABLE == 1
        if (mask & HW_PMU_CHK_VDCDC_LDO_LOW_RET_MSK) {
                if (is_ldo_low_ret_enabled()) {
                        /* LDO_LOW_RET is enabled. */
                        return HW_PMU_ERROR_NOERROR;
                }
        }

        if (mask & HW_PMU_CHK_VDCDC_DCDC_SLEEP_MSK) {
                if (is_dcdc_sleep_enabled()) {
                        /* DCDC is enabled in sleep mode */
                        return HW_PMU_ERROR_NOERROR;
                }
        }

        return HW_PMU_ERROR_NOT_ENOUGH_POWER;
#else
        return HW_PMU_ERROR_NOERROR;
#endif
}

__STATIC_INLINE HW_PMU_ERROR_CODE check_vdd_dependencies_active(HW_PMU_DEPENDENCY_CHK_VDD_MSK mask)
{
#if HW_PMU_SANITY_CHECKS_ENABLE == 1
        if (!REG_GETF(CRG_TOP, ANA_STATUS_REG, LDO_CORE_OK)) {
                /* Something's wrong with LDO_CORE or LDO_CORE_RET. */
                return HW_PMU_ERROR_NOT_ENOUGH_POWER;
        }

        if (mask & HW_PMU_CHK_VDD_LDO_CORE_MSK) {
                if (is_ldo_core_enabled()) {
                        /* LDO_CORE is enabled. */
                        return HW_PMU_ERROR_NOERROR;
                }
        }

        if (mask & HW_PMU_CHK_VDD_LDO_CORE_RET_MSK) {
                if (is_ldo_core_ret_active_enabled()) {
                        /* LDO_CORE_RET is enabled in active mode. */
                        return HW_PMU_ERROR_NOERROR;
                }
        }

        if (mask & HW_PMU_CHK_VDD_LDO_CORE_DROPOUT_1V25_MSK) {
                if (vdcdc_get_voltage_level() > HW_PMU_VDCDC_VOLTAGE_1V40) {
                        /* Enough head room */
                        return HW_PMU_ERROR_NOERROR;
                }
        }

        if (mask & HW_PMU_CHK_VDD_LDO_CORE_DROPOUT_1V20_MSK) {
                if (vdcdc_get_voltage_level() > HW_PMU_VDCDC_VOLTAGE_1V35) {
                        /* Enough head room */
                        return HW_PMU_ERROR_NOERROR;
                }
        }

        if (mask & HW_PMU_CHK_VDD_LDO_CORE_DROPOUT_1V15_MSK) {
                if (vdcdc_get_voltage_level() > HW_PMU_VDCDC_VOLTAGE_1V30) {
                        /* Enough head room */
                        return HW_PMU_ERROR_NOERROR;
                }
        }

        if (mask & HW_PMU_CHK_VDD_LDO_CORE_DROPOUT_1V10_MSK) {
                if (vdcdc_get_voltage_level() > HW_PMU_VDCDC_VOLTAGE_1V25) {
                        /* Enough head room */
                        return HW_PMU_ERROR_NOERROR;
                }
        }

        if (mask & HW_PMU_CHK_VDD_LDO_CORE_DROPOUT_1V05_MSK) {
                if (vdcdc_get_voltage_level() > HW_PMU_VDCDC_VOLTAGE_1V20) {
                        /* Enough head room */
                        return HW_PMU_ERROR_NOERROR;
                }
        }

        if (mask & HW_PMU_CHK_VDD_LDO_CORE_DROPOUT_1V00_MSK) {
                if (vdcdc_get_voltage_level() > HW_PMU_VDCDC_VOLTAGE_1V15) {
                        /* Enough head room */
                        return HW_PMU_ERROR_NOERROR;
                }
        }

        if (mask & HW_PMU_CHK_VDD_LDO_CORE_DROPOUT_0V95_MSK) {
                if (vdcdc_get_voltage_level() > HW_PMU_VDCDC_VOLTAGE_1V10) {
                        /* Enough head room */
                        return HW_PMU_ERROR_NOERROR;
                }
        }

        return HW_PMU_ERROR_NOT_ENOUGH_POWER;
#else
        return HW_PMU_ERROR_NOERROR;
#endif
}

__STATIC_INLINE HW_PMU_ERROR_CODE check_vdd_dependencies_sleep(HW_PMU_DEPENDENCY_CHK_VDD_MSK mask)
{
#if HW_PMU_SANITY_CHECKS_ENABLE == 1
        if (mask & HW_PMU_CHK_VDD_LDO_CORE_RET_MSK) {
                if (is_ldo_core_ret_sleep_enabled()) {
                        /* LDO_CORE_RET is enabled in sleep mode. */
                        return HW_PMU_ERROR_NOERROR;
                }
        }

        return HW_PMU_ERROR_NOT_ENOUGH_POWER;
#else
        return HW_PMU_ERROR_NOERROR;
#endif
}

__STATIC_INLINE HW_PMU_ERROR_CODE check_vdcdc_dependants_active(HW_PMU_DEPENDANT_CHK_VDCDC_MSK mask)
{
#if HW_PMU_SANITY_CHECKS_ENABLE == 1
        if (mask & HW_PMU_CHK_VDCDC_LDO_CORE_MSK) {
                if (REG_GETF(CRG_TOP, ANA_STATUS_REG, LDO_CORE_OK) &&
                        is_ldo_core_enabled()) {
                        /* LDO_CORE is on */
                        return HW_PMU_ERROR_OTHER_LOADS_DEPENDENCY;
                }
        }

        if (mask & HW_PMU_CHK_VDCDC_LDO_CORE_RET_ACTIVE_MSK) {
                if (REG_GETF(CRG_TOP, ANA_STATUS_REG, LDO_CORE_OK) &&
                        is_ldo_core_ret_active_enabled()) {
                        /* LDO_CORE_RET is on in active mode */
                        return HW_PMU_ERROR_OTHER_LOADS_DEPENDENCY;
                }
        }

        if (mask & HW_PMU_CHK_VDCDC_LDO_XTAL32M_MSK) {
                if (is_xtal32m_ldo_on()) {
                        /* LDO_XTAL is on */
                        return HW_PMU_ERROR_OTHER_LOADS_DEPENDENCY;
                }
        }

        if (mask & HW_PMU_CHK_VDCDC_LDO_SDADC_MSK) {
                if (is_sdadc_ldo_on()) {
                        /* LDO_SDADC is on */
                        return HW_PMU_ERROR_OTHER_LOADS_DEPENDENCY;
                }
        }

        if (mask & HW_PMU_CHK_VDCDC_LDO_GPADC_MSK) {
                if (is_gpadc_ldo_on()) {
                        /* LDO_GPADC is on */
                        return HW_PMU_ERROR_OTHER_LOADS_DEPENDENCY;
                }
        }

        if (mask & HW_PMU_CHK_VDCDC_RC32M_MSK) {
                if (is_rc32m_enabled()) {
                        /* RC32M is on */
                        return HW_PMU_ERROR_RC32M_ON;
                }
        }

        if (mask & HW_PMU_CHK_VDCDC_RCX_MSK) {
                if (is_rcx_enabled()) {
                        /* RCX is on */
                        return HW_PMU_ERROR_RCX_ON;
                }
        }

        if (mask & HW_PMU_CHK_VDCDC_LDO_CORE_DROPOUT_1V10_MSK) {
                if (vdd_get_active_voltage_level() > HW_PMU_VDD_VOLTAGE_0V90) {
                        /* LDO_CORE cannot function properly according to specifications. */
                        return HW_PMU_ERROR_ACTION_NOT_POSSIBLE;
                }
        }

        if (mask & HW_PMU_CHK_VDCDC_LDO_CORE_DROPOUT_1V15_MSK) {
                if (vdd_get_active_voltage_level() > HW_PMU_VDD_VOLTAGE_0V95) {
                        /* LDO_CORE cannot function properly according to specifications. */
                        return HW_PMU_ERROR_ACTION_NOT_POSSIBLE;
                }
        }

        if (mask & HW_PMU_CHK_VDCDC_LDO_CORE_DROPOUT_1V20_MSK) {
                if (vdd_get_active_voltage_level() > HW_PMU_VDD_VOLTAGE_1V00) {
                        /* LDO_CORE cannot function properly according to specifications. */
                        return HW_PMU_ERROR_ACTION_NOT_POSSIBLE;
                }
        }

        if (mask & HW_PMU_CHK_VDCDC_LDO_CORE_DROPOUT_1V25_MSK) {
                if (vdd_get_active_voltage_level() > HW_PMU_VDD_VOLTAGE_1V05) {
                        /* LDO_CORE cannot function properly according to specifications. */
                        return HW_PMU_ERROR_ACTION_NOT_POSSIBLE;
                }
        }

        if (mask & HW_PMU_CHK_VDCDC_LDO_CORE_DROPOUT_1V30_MSK) {
                if (vdd_get_active_voltage_level() > HW_PMU_VDD_VOLTAGE_1V10) {
                        /* LDO_CORE cannot function properly according to specifications. */
                        return HW_PMU_ERROR_ACTION_NOT_POSSIBLE;
                }
        }

        if (mask & HW_PMU_CHK_VDCDC_LDO_CORE_DROPOUT_1V35_MSK) {
                if (vdd_get_active_voltage_level() > HW_PMU_VDD_VOLTAGE_1V15) {
                        /* LDO_CORE cannot function properly according to specifications. */
                        return HW_PMU_ERROR_ACTION_NOT_POSSIBLE;
                }
        }

        if (mask & HW_PMU_CHK_VDCDC_LDO_CORE_DROPOUT_1V40_MSK) {
                if (vdd_get_active_voltage_level() > HW_PMU_VDD_VOLTAGE_1V20) {
                        /* LDO_CORE cannot function properly according to specifications. */
                        return HW_PMU_ERROR_ACTION_NOT_POSSIBLE;
                }
        }

        return HW_PMU_ERROR_NOERROR;
#else
        return HW_PMU_ERROR_NOERROR;
#endif
}

__STATIC_INLINE HW_PMU_ERROR_CODE check_vdcdc_dependants_sleep(HW_PMU_DEPENDANT_CHK_VDCDC_MSK mask)
{
#if HW_PMU_SANITY_CHECKS_ENABLE == 1
        if (mask & HW_PMU_CHK_VDCDC_LDO_CORE_RET_SLEEP_MSK) {
                if (REG_GETF(CRG_TOP, ANA_STATUS_REG, LDO_CORE_OK) &&
                        is_ldo_core_ret_sleep_enabled()) {
                        /* LDO_CORE_RET is on in sleep mode */
                        return HW_PMU_ERROR_OTHER_LOADS_DEPENDENCY;
                }
        }

        if (mask & HW_PMU_CHK_VDCDC_RCX_MSK) {
                if (is_rcx_lp_clk()) {
                        /* RCX set as LP clock */
                        return HW_PMU_ERROR_RCX_LP;
                }
        }
        return HW_PMU_ERROR_NOERROR;
#else
        return HW_PMU_ERROR_NOERROR;
#endif
}

__STATIC_INLINE HW_PMU_ERROR_CODE check_vdd_dependants_active(HW_PMU_DEPENDANT_CHK_VDD_MSK mask)
{
#if HW_PMU_SANITY_CHECKS_ENABLE == 1
        if (mask & HW_PMU_CHK_VDD_XTAL32K_MSK) {
                if (is_xtal32k_enabled()) {
                        /* XTAL32K is on */
                        return HW_PMU_ERROR_XTAL32K_ON;
                }
        }

        if (mask & HW_PMU_CHK_VDD_XTAL32M_DBLR_MSK) {
                if (is_dblr64m_enabled()) {
                        /* XTAL32M doubler is on */
                        return HW_PMU_ERROR_XTAL32M_DBLR_ON;
                }
        }

        if (mask & HW_PMU_CHK_VDD_EFLASH_OPS_MSK) {
                if (is_eflash_ops_on()) {
                        /* eFlash performs a write or erase operation */
                        return HW_PMU_ERROR_EFLASH_OPS;
                }
        }

        return HW_PMU_ERROR_NOERROR;
#else
        return HW_PMU_ERROR_NOERROR;
#endif
}

__STATIC_INLINE HW_PMU_ERROR_CODE check_vdd_dependants_sleep(HW_PMU_DEPENDANT_CHK_VDD_MSK mask)
{
#if HW_PMU_SANITY_CHECKS_ENABLE == 1
        if (mask & HW_PMU_CHK_VDD_XTAL32K_MSK) {
                if (is_xtal32k_lp_clk()) {
                        /* XTAL32K set as LP clock */
                        return HW_PMU_ERROR_XTAL32K_LP;
                }
        }

        if (mask & HW_PMU_CHK_VDD_FAST_WAKEUP_MSK) {
                if (is_fast_wakeup_mode_enabled()) {
                        /* Fast wake up is on */
                        return HW_PMU_ERROR_FAST_WAKEUP_ON;
                }
        }

        return HW_PMU_ERROR_NOERROR;
#else
        return HW_PMU_ERROR_NOERROR;
#endif
}

HW_PMU_ERROR_CODE hw_pmu_1v8_onwakeup_enable(HW_PMU_1V8_MAX_LOAD max_load)
{
        HW_PMU_ERROR_CODE res = HW_PMU_ERROR_NOERROR;

        switch (max_load) {
        /* 20mA Max load */
        case HW_PMU_1V8_MAX_LDO_LOAD_20:
        case HW_PMU_1V8_MAX_BYPASS_LOAD_20:
                res = check_1v8_dependencies_active(HW_PMU_CHK_1V8_LDO_IO_MSK |
                                                    HW_PMU_CHK_1V8_LDO_IO_BYPASS_MSK);
                break;

        /* 2mA Max load */
        case HW_PMU_1V8_MAX_LDO_LOAD_2:
                res = check_1v8_dependencies_active(HW_PMU_CHK_1V8_LDO_IO_MSK         |
                                                    HW_PMU_CHK_1V8_LDO_IO_BYPASS_MSK  |
                                                    HW_PMU_CHK_1V8_LDO_IO_RET_ACTIVE_MSK);
                break;
        case HW_PMU_1V8_MAX_BYPASS_LOAD_2:
                /* Applicable only in sleep mode */
                res = HW_PMU_ERROR_INVALID_ARGS;
                break;
        default:
                res = HW_PMU_ERROR_INVALID_ARGS;
        }

        if (res == HW_PMU_ERROR_NOERROR) {
                switch (max_load) {
                case HW_PMU_1V8_MAX_LDO_LOAD_20:
                        _1v8_ldo_io_enable();
                        /* Disable other power sources */
                        _1v8_ldo_io_ret_active_disable();
                        _1v8_ldo_io_bypass_active_disable();
                        break;
                case HW_PMU_1V8_MAX_BYPASS_LOAD_20:
                        _1v8_ldo_io_bypass_active_enable();
                        /* Disable other power sources */
                        _1v8_ldo_io_disable();
                        _1v8_ldo_io_ret_active_disable();
                        break;
                case HW_PMU_1V8_MAX_LDO_LOAD_2:
                        _1v8_ldo_io_ret_active_enable();
                        /* Disable other power sources */
                        _1v8_ldo_io_disable();
                        _1v8_ldo_io_bypass_active_disable();
                        break;
                case HW_PMU_1V8_MAX_BYPASS_LOAD_2:
                        /* Applicable only in sleep mode */
                        res = HW_PMU_ERROR_INVALID_ARGS;
                        break;
                }
        }

        return res;
}

HW_PMU_ERROR_CODE hw_pmu_1v8_onwakeup_disable(void)
{
#if HW_PMU_SANITY_CHECKS_ENABLE == 1

        if (!hw_bod_get_status(HW_BOD_CHANNEL_1V8)) {
                return HW_PMU_ERROR_BOD_IS_ACTIVE;
        }
#endif

        /* Switch off rail LDOs */
        _1v8_ldo_io_ret_active_disable();
        _1v8_ldo_io_disable();

        /* Switch off power supply from VBAT in active mode */
        _1v8_ldo_io_bypass_active_disable();

        return HW_PMU_ERROR_NOERROR;
}

HW_PMU_ERROR_CODE hw_pmu_1v8_onsleep_enable(HW_PMU_1V8_MAX_LOAD max_load)
{
        HW_PMU_ERROR_CODE res = HW_PMU_ERROR_NOERROR;

        switch (max_load) {
                /* 20mA Max load */
                case HW_PMU_1V8_MAX_LDO_LOAD_20:
                case HW_PMU_1V8_MAX_BYPASS_LOAD_20:
                        /* Applicable only in active mode */
                        res = HW_PMU_ERROR_INVALID_ARGS;
                        break;

                /* 2mA Max load */
                case HW_PMU_1V8_MAX_LDO_LOAD_2:
                case HW_PMU_1V8_MAX_BYPASS_LOAD_2:
                        res = check_1v8_dependencies_sleep(HW_PMU_CHK_1V8_LDO_IO_RET_SLEEP_MSK |
                                                           HW_PMU_CHK_1V8_LDO_IO_RET_BYPASS_MSK);
                        break;
                default:
                        res = HW_PMU_ERROR_INVALID_ARGS;
                }

        if (res == HW_PMU_ERROR_NOERROR) {

                switch (max_load) {
                case HW_PMU_1V8_MAX_LDO_LOAD_20:
                case HW_PMU_1V8_MAX_BYPASS_LOAD_20:
                        /* Applicable only in active mode */
                        res = HW_PMU_ERROR_INVALID_ARGS;
                        break;
                case HW_PMU_1V8_MAX_LDO_LOAD_2:
                        _1v8_ldo_io_ret_sleep_enable();
                        /* Disable other power sources */
                        _1v8_ldo_io_bypass_sleep_disable();
                        break;
                case HW_PMU_1V8_MAX_BYPASS_LOAD_2:
                        _1v8_ldo_io_bypass_sleep_enable();
                        /* Disable other power sources */
                        _1v8_ldo_io_ret_sleep_disable();
                        break;
                }

        }

        return res;
}

HW_PMU_ERROR_CODE hw_pmu_1v8_onsleep_disable(void)
{
#if HW_PMU_SANITY_CHECKS_ENABLE == 1

        if (!hw_bod_get_status(HW_BOD_CHANNEL_1V8)) {
                return HW_PMU_ERROR_BOD_IS_ACTIVE;
        }
#endif
        _1v8_ldo_io_ret_sleep_disable();

        /* Switch off power supply from VBAT in sleep mode */
        _1v8_ldo_io_bypass_sleep_disable();

        return HW_PMU_ERROR_NOERROR;
}

HW_PMU_POWER_RAIL_STATE hw_pmu_get_1v8_active_config(HW_PMU_1V8_RAIL_CONFIG *rail_config)
{
        HW_PMU_POWER_RAIL_STATE r_state = POWER_RAIL_DISABLED;

        memset(rail_config, UINT8_MAX, sizeof(HW_PMU_1V8_RAIL_CONFIG));

        rail_config->voltage = HW_PMU_1V8_VOLTAGE_1V8;

        if (is_ldo_io_enabled()) {
                r_state = POWER_RAIL_ENABLED;
                rail_config->current = HW_PMU_1V8_MAX_LDO_LOAD_20;
                rail_config->src_type = HW_PMU_SRC_TYPE_LDO_LOW_RIPPLE;
        } else if (is_ldo_io_ret_active_enabled()) {
                r_state = POWER_RAIL_ENABLED;
                rail_config->current = HW_PMU_1V8_MAX_LDO_LOAD_2;
                rail_config->src_type = HW_PMU_SRC_TYPE_LDO_LOW_RIPPLE;
        }

        /* The LDO bypass mode overrules LDO regulating operation. */
        if (is_ldo_io_bypass_active_enabled()) {
                r_state = POWER_RAIL_ENABLED;
                rail_config->current = HW_PMU_1V8_MAX_BYPASS_LOAD_20;
                rail_config->src_type = HW_PMU_SRC_TYPE_VBAT;
        }

        return r_state;
}

HW_PMU_POWER_RAIL_STATE hw_pmu_get_1v8_onwakeup_config(HW_PMU_1V8_RAIL_CONFIG *rail_config)
{
        return hw_pmu_get_1v8_active_config(rail_config);
}

HW_PMU_POWER_RAIL_STATE hw_pmu_get_1v8_onsleep_config(HW_PMU_1V8_RAIL_CONFIG *rail_config)
{
        HW_PMU_POWER_RAIL_STATE r_state = POWER_RAIL_DISABLED;

        memset(rail_config, UINT8_MAX, sizeof(HW_PMU_1V8_RAIL_CONFIG));

        rail_config->voltage = HW_PMU_1V8_VOLTAGE_1V8;


        if (is_ldo_io_ret_sleep_enabled()) {
                r_state = POWER_RAIL_ENABLED;
                rail_config->current = HW_PMU_1V8_MAX_LDO_LOAD_2;
                rail_config->src_type = HW_PMU_SRC_TYPE_LDO_LOW_RIPPLE;
        }

        /* The LDO bypass mode overrules LDO regulating operation. */
        if (is_ldo_io_bypass_sleep_enabled()) {
                r_state = POWER_RAIL_ENABLED;
                rail_config->current = HW_PMU_1V8_MAX_BYPASS_LOAD_2;
                rail_config->src_type = HW_PMU_SRC_TYPE_VBAT;
        }

        return r_state;
}

HW_PMU_ERROR_CODE hw_pmu_vdcdc_set_voltage(HW_PMU_VDCDC_VOLTAGE voltage)
{
#if HW_PMU_SANITY_CHECKS_ENABLE == 1
        HW_PMU_ERROR_CODE res = HW_PMU_ERROR_NOERROR;
#endif

#if HW_PMU_SANITY_CHECKS_ENABLE == 1
        switch (voltage) {
        case HW_PMU_VDCDC_VOLTAGE_1V10: /* NOK VDD > 0V90 */
                res = check_vdcdc_dependants_active(HW_PMU_CHK_VDCDC_LDO_CORE_DROPOUT_1V10_MSK);

                break;
        case HW_PMU_VDCDC_VOLTAGE_1V15: /* NOK VDD > 0V95 */
                res = check_vdcdc_dependants_active(HW_PMU_CHK_VDCDC_LDO_CORE_DROPOUT_1V15_MSK);

                break;
        case HW_PMU_VDCDC_VOLTAGE_1V20: /* NOK VDD > 1V00 */
                res = check_vdcdc_dependants_active(HW_PMU_CHK_VDCDC_LDO_CORE_DROPOUT_1V20_MSK);

                break;
        case HW_PMU_VDCDC_VOLTAGE_1V25: /* NOK VDD > 1V05 */
                res = check_vdcdc_dependants_active(HW_PMU_CHK_VDCDC_LDO_CORE_DROPOUT_1V25_MSK);

                break;
        case HW_PMU_VDCDC_VOLTAGE_1V30: /* NOK VDD > 1V10 */
                res = check_vdcdc_dependants_active(HW_PMU_CHK_VDCDC_LDO_CORE_DROPOUT_1V30_MSK);

                break;
        case HW_PMU_VDCDC_VOLTAGE_1V35: /* NOK VDD > 1V15 */
                res = check_vdcdc_dependants_active(HW_PMU_CHK_VDCDC_LDO_CORE_DROPOUT_1V35_MSK);

                break;
        case HW_PMU_VDCDC_VOLTAGE_1V40: /* NOK VDD > 1V20 */
                res = check_vdcdc_dependants_active(HW_PMU_CHK_VDCDC_LDO_CORE_DROPOUT_1V40_MSK);

                break;
        case HW_PMU_VDCDC_VOLTAGE_1V45: /* OK */
                break;
        default:
                return HW_PMU_ERROR_INVALID_ARGS;
        }

        if (res != HW_PMU_ERROR_NOERROR) {
                return res;
        }
#endif

        switch (voltage) {
        case HW_PMU_VDCDC_VOLTAGE_1V10:
        case HW_PMU_VDCDC_VOLTAGE_1V15:
        case HW_PMU_VDCDC_VOLTAGE_1V20:
        case HW_PMU_VDCDC_VOLTAGE_1V25:
        case HW_PMU_VDCDC_VOLTAGE_1V30:
        case HW_PMU_VDCDC_VOLTAGE_1V35:
#if HW_PMU_SANITY_CHECKS_ENABLE == 1

                res = check_vdd_dependants_active(HW_PMU_CHK_VDD_RFHP_MSK         |
                                                  HW_PMU_CHK_VDD_XTAL32M_DBLR_MSK |
                                                  HW_PMU_CHK_VDD_EFLASH_OPS_MSK);
                if (res != HW_PMU_ERROR_NOERROR) {
                        return res;
                }
#endif
                /* Suppress "No break at the end of case" warning */
        case HW_PMU_VDCDC_VOLTAGE_1V40:
        case HW_PMU_VDCDC_VOLTAGE_1V45:
                vdcdc_set_voltage_level(voltage);
                break;
        default:
                return HW_PMU_ERROR_INVALID_ARGS;
        }

        return HW_PMU_ERROR_NOERROR;
}



HW_PMU_ERROR_CODE hw_pmu_vdcdc_onwakeup_enable(HW_PMU_VDCDC_MAX_LOAD max_load)
{
        HW_PMU_ERROR_CODE res = HW_PMU_ERROR_NOERROR;

        switch (max_load) {
        /* 40mA Max load */
        case HW_PMU_VDCDC_MAX_LDO_LOAD_40:
        case HW_PMU_VDCDC_MAX_DCDC_LOAD_40:
                res = check_vdcdc_dependencies_active(HW_PMU_CHK_VDCDC_LDO_LOW_MSK |
                                                      HW_PMU_CHK_VDCDC_DCDC_ACTIVE_MSK);
                break;
        case HW_PMU_VDCDC_MAX_LDO_LOAD_1:
        case HW_PMU_VDCDC_MAX_DCDC_LOAD_0_300:
                /* Applicable only in sleep mode */
                res = HW_PMU_ERROR_INVALID_ARGS;
                break;
        default:
                res = HW_PMU_ERROR_INVALID_ARGS;
        }

        if (res == HW_PMU_ERROR_NOERROR) {
                switch (max_load) {
                case HW_PMU_VDCDC_MAX_LDO_LOAD_40:
                        vdcdc_ldo_low_enable();
                        /* Disable other power sources */
                        vdcdc_dcdc_disable();
                        break;
                case HW_PMU_VDCDC_MAX_DCDC_LOAD_40:
                        vdcdc_dcdc_enable();
                        /* Disable other power sources */
                        /* Note: When DCDC is enabled, the HW automatically disables
                         * the other drivers of the same rail (namely the LDO_LOW or LDO_LOW_RET)
                         */
                        //vdcdc_ldo_low_disable();
                        break;
                case HW_PMU_VDCDC_MAX_LDO_LOAD_1:
                case HW_PMU_VDCDC_MAX_DCDC_LOAD_0_300:
                        /* Applicable only in sleep mode */
                        res = HW_PMU_ERROR_INVALID_ARGS;
                        break;
                }
        }

        return res;
}

HW_PMU_ERROR_CODE hw_pmu_vdcdc_onwakeup_disable(void)
{
#if HW_PMU_SANITY_CHECKS_ENABLE == 1
        HW_PMU_ERROR_CODE res;
        if (!hw_bod_get_status(HW_BOD_CHANNEL_VDCDC)) {
                return HW_PMU_ERROR_BOD_IS_ACTIVE;
        }

        res = check_vdcdc_dependants_active(HW_PMU_CHK_VDCDC_RF_LDO_MSK                 |
                                           HW_PMU_CHK_VDCDC_LDO_CORE_MSK                |
                                           HW_PMU_CHK_VDCDC_LDO_CORE_RET_ACTIVE_MSK     |
                                           HW_PMU_CHK_VDCDC_LDO_XTAL32M_MSK             |
                                           HW_PMU_CHK_VDCDC_LDO_SDADC_MSK               |
                                           HW_PMU_CHK_VDCDC_LDO_GPADC_MSK |
                                           HW_PMU_CHK_VDCDC_RC32M_MSK |
                                           HW_PMU_CHK_VDCDC_RCX_MSK);

        if (res != HW_PMU_ERROR_NOERROR) {
                return res;
        }
#endif
        /* Switch off rail LDOs */
        vdcdc_ldo_low_disable();

        /* Switch off DCDC */
        vdcdc_dcdc_disable();

        return HW_PMU_ERROR_NOERROR;
}

HW_PMU_ERROR_CODE hw_pmu_vdcdc_onsleep_enable(HW_PMU_VDCDC_MAX_LOAD max_load)
{
        HW_PMU_ERROR_CODE res = HW_PMU_ERROR_NOERROR;

        switch (max_load) {
        /* 40mA Max load */
        case HW_PMU_VDCDC_MAX_LDO_LOAD_40:
        case HW_PMU_VDCDC_MAX_DCDC_LOAD_40:
                /* Applicable only in active mode */
                res = HW_PMU_ERROR_INVALID_ARGS;
                break;
        /* 1mA or 300uA Max load */
        case HW_PMU_VDCDC_MAX_LDO_LOAD_1:
        case HW_PMU_VDCDC_MAX_DCDC_LOAD_0_300:
                res = check_vdcdc_dependencies_sleep(HW_PMU_CHK_VDCDC_LDO_LOW_RET_MSK |
                                                     HW_PMU_CHK_VDCDC_DCDC_SLEEP_MSK);
                break;
        default:
                res = HW_PMU_ERROR_INVALID_ARGS;
        }

        if (res == HW_PMU_ERROR_NOERROR) {
                switch (max_load) {
                case HW_PMU_VDCDC_MAX_LDO_LOAD_40:
                case HW_PMU_VDCDC_MAX_DCDC_LOAD_40:
                        /* Applicable only in active mode */
                        res = HW_PMU_ERROR_INVALID_ARGS;
                        break;
                case HW_PMU_VDCDC_MAX_LDO_LOAD_1:
                        vdcdc_ldo_low_ret_enable();
                        /* Disable other power sources */
                        vdcdc_dcdc_disable();
                        break;
                case HW_PMU_VDCDC_MAX_DCDC_LOAD_0_300:
                        vdcdc_dcdc_sleep_enable();
                        /* Disable other power sources */
                        /* Note: When DCDC is enabled, the HW automatically disables
                         * the other drivers of the same rail (namely the LDO_LOW or LDO_LOW_RET)
                         */
                        //vdcdc_ldo_low_ret_disable();
                        break;
                }
        }

        return res;
}

HW_PMU_ERROR_CODE hw_pmu_vdcdc_onsleep_disable(void)
{
#if HW_PMU_SANITY_CHECKS_ENABLE == 1
        HW_PMU_ERROR_CODE res;
        if (!hw_bod_get_status(HW_BOD_CHANNEL_VDCDC)) {
                return HW_PMU_ERROR_BOD_IS_ACTIVE;
        }

        res = check_vdcdc_dependants_sleep(HW_PMU_CHK_VDCDC_LDO_CORE_RET_SLEEP_MSK |
                                           HW_PMU_CHK_VDCDC_RCX_MSK);

        if (res != HW_PMU_ERROR_NOERROR) {
                return res;
        }
#endif
        /* Switch off rail LDOs */
        vdcdc_ldo_low_ret_disable();

        /* Switch off DCDC */
        vdcdc_dcdc_sleep_disable();

        return HW_PMU_ERROR_NOERROR;
}

HW_PMU_POWER_RAIL_STATE hw_pmu_get_vdcdc_active_config(HW_PMU_VDCDC_RAIL_CONFIG *rail_config)
{
        HW_PMU_POWER_RAIL_STATE r_state = POWER_RAIL_DISABLED;

        memset(rail_config, UINT8_MAX, sizeof(HW_PMU_VDCDC_RAIL_CONFIG));

        rail_config->voltage = vdcdc_get_voltage_level();

        if (is_dcdc_enabled()) {
                r_state = POWER_RAIL_ENABLED;
                rail_config->current = HW_PMU_VDCDC_MAX_DCDC_LOAD_40;
                rail_config->src_type = HW_PMU_SRC_TYPE_DCDC_HIGH_EFFICIENCY;
        } else if (is_ldo_low_enabled()) {
                r_state = POWER_RAIL_ENABLED;
                rail_config->current = HW_PMU_VDCDC_MAX_LDO_LOAD_40;
                rail_config->src_type = HW_PMU_SRC_TYPE_LDO_LOW_RIPPLE;
        }

        return r_state;
}

HW_PMU_POWER_RAIL_STATE hw_pmu_get_vdcdc_onwakeup_config(HW_PMU_VDCDC_RAIL_CONFIG *rail_config)
{
        return hw_pmu_get_vdcdc_active_config(rail_config);
}

HW_PMU_POWER_RAIL_STATE hw_pmu_get_vdcdc_onsleep_config(HW_PMU_VDCDC_RAIL_CONFIG *rail_config)
{
        HW_PMU_POWER_RAIL_STATE r_state = POWER_RAIL_DISABLED;

        memset(rail_config, UINT8_MAX, sizeof(HW_PMU_VDCDC_RAIL_CONFIG));

        rail_config->voltage = HW_PMU_VDCDC_VOLTAGE_1V10;

        if (is_dcdc_sleep_enabled()) {
                r_state = POWER_RAIL_ENABLED;
                rail_config->current = HW_PMU_VDCDC_MAX_DCDC_LOAD_0_300;
                rail_config->src_type = HW_PMU_SRC_TYPE_DCDC_HIGH_EFFICIENCY;
        } else if (is_ldo_low_ret_enabled()) {
                r_state = POWER_RAIL_ENABLED;
                rail_config->current = HW_PMU_VDCDC_MAX_LDO_LOAD_1;
                rail_config->src_type = HW_PMU_SRC_TYPE_LDO_LOW_RIPPLE;
        }

        return r_state;
}

HW_PMU_ERROR_CODE hw_pmu_vdd_set_voltage(HW_PMU_VDD_VOLTAGE voltage)
{

#if HW_PMU_SANITY_CHECKS_ENABLE == 1
        HW_PMU_ERROR_CODE res = HW_PMU_ERROR_NOERROR;
#endif

#if HW_PMU_SANITY_CHECKS_ENABLE == 1
        switch (voltage) {
        case HW_PMU_VDD_VOLTAGE_1V25: /* VDCDC > 1V40 */
                res = check_vdd_dependencies_active(HW_PMU_CHK_VDD_LDO_CORE_DROPOUT_1V25_MSK);

                break;
        case HW_PMU_VDD_VOLTAGE_1V20: /* OK VDCDC > 1V35 */
                res = check_vdd_dependencies_active(HW_PMU_CHK_VDD_LDO_CORE_DROPOUT_1V20_MSK);

                break;
        case HW_PMU_VDD_VOLTAGE_1V15: /* OK VDCDC > 1V30 */
                res = check_vdd_dependencies_active(HW_PMU_CHK_VDD_LDO_CORE_DROPOUT_1V15_MSK);

                break;
        case HW_PMU_VDD_VOLTAGE_1V10: /* OK VDCDC > 1V25 */
                res = check_vdd_dependencies_active(HW_PMU_CHK_VDD_LDO_CORE_DROPOUT_1V10_MSK);

                break;
        case HW_PMU_VDD_VOLTAGE_1V05: /* OK VDCDC > 1V20 */
                res = check_vdd_dependencies_active(HW_PMU_CHK_VDD_LDO_CORE_DROPOUT_1V05_MSK);

                break;
        case HW_PMU_VDD_VOLTAGE_1V00: /* OK VDCDC > 1V15 */
                res = check_vdd_dependencies_active(HW_PMU_CHK_VDD_LDO_CORE_DROPOUT_1V00_MSK);

                break;
        case HW_PMU_VDD_VOLTAGE_0V95: /* OK VDCDC > 1V10 */
                res = check_vdd_dependencies_active(HW_PMU_CHK_VDD_LDO_CORE_DROPOUT_0V95_MSK);

                break;
        case HW_PMU_VDD_VOLTAGE_0V90: /* OK */
        case HW_PMU_VDD_VOLTAGE_SLEEP_0V90:
        case HW_PMU_VDD_VOLTAGE_SLEEP_0V75:
                break;
        default:
                return HW_PMU_ERROR_INVALID_ARGS;
        }

        if (res != HW_PMU_ERROR_NOERROR) {
                return res;
        }
#endif

        switch (voltage) {
        /* VDD voltages in active mode. */
        case HW_PMU_VDD_VOLTAGE_0V90:
        case HW_PMU_VDD_VOLTAGE_0V95:
        case HW_PMU_VDD_VOLTAGE_1V00:
        case HW_PMU_VDD_VOLTAGE_1V05:
#if HW_PMU_SANITY_CHECKS_ENABLE == 1
#if dg_configUSE_BOD
                if (!hw_bod_get_status(HW_BOD_CHANNEL_VDD) &&
                    (hw_bod_get_channel_voltage_level(HW_BOD_CHANNEL_VDD) ==
                            HW_BOD_VDD_LEVEL_ACTIVE_1V05)) {
                        return HW_PMU_ERROR_BOD_THRESHOLD;
                }
#endif
#endif
                /* Suppress "No break at the end of case" warning */
        case HW_PMU_VDD_VOLTAGE_1V10:
        case HW_PMU_VDD_VOLTAGE_1V15:
#if HW_PMU_SANITY_CHECKS_ENABLE == 1
                res = check_vdd_dependants_active(HW_PMU_CHK_VDD_RFHP_MSK         |
                                                  HW_PMU_CHK_VDD_XTAL32M_DBLR_MSK |
                                                  HW_PMU_CHK_VDD_EFLASH_OPS_MSK);
                if (res != HW_PMU_ERROR_NOERROR) {
                        return res;
                }
#endif
                /* Suppress "No break at the end of case" warning */
        case HW_PMU_VDD_VOLTAGE_1V20:
        case HW_PMU_VDD_VOLTAGE_1V25:
#if dg_configHW_FCU_WAIT_CYCLES_MODE
                hw_sys_fcu_set_max_wait_cycles();
                vdd_set_active_voltage_level(voltage);
                hw_sys_fcu_set_optimum_wait_cycles();
#else
                vdd_set_active_voltage_level(voltage);
#endif
                break;

                /* VDD voltages in sleep mode. */
        case HW_PMU_VDD_VOLTAGE_SLEEP_0V75:
#if HW_PMU_SANITY_CHECKS_ENABLE == 1
                res = check_vdd_dependants_sleep(HW_PMU_CHK_VDD_FAST_WAKEUP_MSK);
                if (res != HW_PMU_ERROR_NOERROR) {
                        return res;
                }
#endif
                /* Suppress "No break at the end of case" warning */
        case HW_PMU_VDD_VOLTAGE_SLEEP_0V90:
                vdd_set_sleep_voltage_level(voltage);
                break;
        default:
                return HW_PMU_ERROR_INVALID_ARGS;

        }
        return HW_PMU_ERROR_NOERROR;
}

HW_PMU_ERROR_CODE hw_pmu_vdd_onwakeup_enable(HW_PMU_VDD_MAX_LOAD max_load)
{
        HW_PMU_ERROR_CODE res = HW_PMU_ERROR_NOERROR;

        switch (max_load) {
        /* 20mA Max load */
        case HW_PMU_VDD_MAX_LOAD_20:
                res = check_vdd_dependencies_active(HW_PMU_CHK_VDD_LDO_CORE_MSK);
                break;

        /* 400uA Max load */
        case HW_PMU_VDD_MAX_LOAD_0_400:
                res = check_vdd_dependencies_active(HW_PMU_CHK_VDD_LDO_CORE_MSK |
                                                    HW_PMU_CHK_VDD_LDO_CORE_RET_MSK);
                break;
        default:
                res = HW_PMU_ERROR_INVALID_ARGS;
        }

        if (res == HW_PMU_ERROR_NOERROR) {
                switch (max_load) {
                case HW_PMU_VDD_MAX_LOAD_20:
                        vdd_ldo_core_enable();
                        /* Disable other power sources */
                        vdd_ldo_core_ret_active_disable();
                        break;
                case HW_PMU_VDD_MAX_LOAD_0_400:
                        vdd_ldo_core_ret_active_enable();
                        /* Disable other power sources */
                        vdd_ldo_core_disable();
                        break;
                }
        }

        return res;
}

HW_PMU_ERROR_CODE hw_pmu_vdd_onwakeup_disable(void)
{

#if HW_PMU_SANITY_CHECKS_ENABLE == 1
        HW_PMU_ERROR_CODE res;
        if (!hw_bod_get_status(HW_BOD_CHANNEL_VDD)) {
                return HW_PMU_ERROR_BOD_IS_ACTIVE;
        }

        res = check_vdd_dependants_active(HW_PMU_CHK_VDD_XTAL32K_MSK      |
                                          HW_PMU_CHK_VDD_RFHP_MSK         |
                                          HW_PMU_CHK_VDD_XTAL32M_DBLR_MSK |
                                          HW_PMU_CHK_VDD_EFLASH_OPS_MSK);

        if (res != HW_PMU_ERROR_NOERROR) {
                return res;
        }
#endif
        /* Switch off rail LDOs */
        vdd_ldo_core_ret_active_disable();
        vdd_ldo_core_disable();

        return HW_PMU_ERROR_NOERROR;
}

HW_PMU_ERROR_CODE hw_pmu_vdd_onsleep_enable(HW_PMU_VDD_MAX_LOAD max_load)
{
        HW_PMU_ERROR_CODE res = HW_PMU_ERROR_NOERROR;

        switch (max_load) {
        /* 20mA Max load */
        case HW_PMU_VDD_MAX_LOAD_20:
                /* Applicable only in active mode */
                res = HW_PMU_ERROR_INVALID_ARGS;
                break;

                /* 2mA Max load */
        case HW_PMU_VDD_MAX_LOAD_0_400:
                res = check_vdd_dependencies_sleep(HW_PMU_CHK_VDD_LDO_CORE_RET_MSK);
                break;
        default:
                res = HW_PMU_ERROR_INVALID_ARGS;
        }

        if (res == HW_PMU_ERROR_NOERROR) {

                switch (max_load) {
                case HW_PMU_VDD_MAX_LOAD_20:
                        /* Applicable only in active mode */
                        res = HW_PMU_ERROR_INVALID_ARGS;
                        break;
                case HW_PMU_VDD_MAX_LOAD_0_400:
                        vdd_ldo_core_ret_sleep_enable();
                        /* Disable other power sources */
                        /* No other power supply in sleep mode. */
                        break;
                }
        }

        return res;
}

HW_PMU_ERROR_CODE hw_pmu_vdd_onsleep_disable(void)
{

#if HW_PMU_SANITY_CHECKS_ENABLE == 1
        HW_PMU_ERROR_CODE res;
        if (!hw_bod_get_status(HW_BOD_CHANNEL_VDD)) {
                return HW_PMU_ERROR_BOD_IS_ACTIVE;
        }

        res = check_vdd_dependants_sleep(HW_PMU_CHK_VDD_XTAL32K_MSK |
                                         HW_PMU_CHK_VDD_FAST_WAKEUP_MSK);

        if (res != HW_PMU_ERROR_NOERROR) {
                return res;
        }
#endif
        /* Switch off rail LDOs */
        vdd_ldo_core_ret_sleep_disable();

        return HW_PMU_ERROR_NOERROR;
}


HW_PMU_POWER_RAIL_STATE hw_pmu_get_vdd_active_config(HW_PMU_VDD_RAIL_CONFIG *rail_config)
{
        HW_PMU_POWER_RAIL_STATE r_state = POWER_RAIL_DISABLED;

        memset(rail_config, UINT8_MAX, sizeof(HW_PMU_VDD_RAIL_CONFIG));

        rail_config->voltage = vdd_get_active_voltage_level();

        if (is_ldo_core_enabled()) {
                r_state = POWER_RAIL_ENABLED;
                rail_config->current = HW_PMU_VDD_MAX_LOAD_20;
                rail_config->src_type = HW_PMU_SRC_TYPE_LDO_LOW_RIPPLE;
        } else if (is_ldo_core_ret_active_enabled()) {
                r_state = POWER_RAIL_ENABLED;
                rail_config->current = HW_PMU_VDD_MAX_LOAD_0_400;
                rail_config->src_type = HW_PMU_SRC_TYPE_LDO_LOW_RIPPLE;
        }

        return r_state;
}

HW_PMU_POWER_RAIL_STATE hw_pmu_get_vdd_onwakeup_config(HW_PMU_VDD_RAIL_CONFIG *rail_config)
{
        return hw_pmu_get_vdd_active_config(rail_config);
}

HW_PMU_POWER_RAIL_STATE hw_pmu_get_vdd_onsleep_config(HW_PMU_VDD_RAIL_CONFIG *rail_config)
{
        HW_PMU_POWER_RAIL_STATE r_state = POWER_RAIL_DISABLED;

        memset(rail_config, UINT8_MAX, sizeof(HW_PMU_VDD_RAIL_CONFIG));

        rail_config->voltage = HW_PMU_VDD_VOLTAGE_SLEEP_0V75 +
                               REG_GETF(CRG_TOP, POWER_LEVEL_REG, VDD_LEVEL_SLEEP);

        if (is_ldo_core_ret_sleep_enabled()) {
                r_state = POWER_RAIL_ENABLED;
                rail_config->current = HW_PMU_VDD_MAX_LOAD_0_400;
                rail_config->src_type = HW_PMU_SRC_TYPE_LDO_LOW_RIPPLE;
        }

        return r_state;
}


#endif /* dg_configUSE_HW_PMU */
