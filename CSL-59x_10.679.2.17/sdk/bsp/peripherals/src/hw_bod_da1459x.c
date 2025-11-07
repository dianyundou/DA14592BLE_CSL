/**
\addtogroup BSP
\{
\addtogroup DEVICES
\{
\addtogroup BOD
\{
*/

/**
****************************************************************************************
*
* @file hw_bod_da1459x.c
*
* @brief BOD LLD
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

#include "hw_bod.h"
#include "hw_pmu.h"

#if dg_configUSE_BOD

typedef enum  {
        BOD_VDD_COMPARATOR_LEVEL_700    = 0x3,
        BOD_VDD_COMPARATOR_LEVEL_780    = 0x0,
        BOD_VDD_COMPARATOR_LEVEL_1050   = 0x1,
        BOD_VDD_COMPARATOR_LEVEL_UNDEF  = 0x2
} HW_BOD_VDD_COMP_LVL;

static void hw_bod_activate_on_wakeup(void)
{
        hw_bod_deactivate_channel(HW_BOD_CHANNEL_VDD);
        hw_bod_deactivate_channel(HW_BOD_CHANNEL_VDCDC);
        hw_bod_deactivate_channel(HW_BOD_CHANNEL_1V8);

        HW_PMU_VDD_RAIL_CONFIG vdd_rail_config;
        if (hw_pmu_get_vdd_onwakeup_config(&vdd_rail_config) == POWER_RAIL_ENABLED) {
                hw_bod_activate_channel(HW_BOD_CHANNEL_VDD);
        }

        HW_PMU_VDCDC_RAIL_CONFIG vdcdc_rail_config;
        if (hw_pmu_get_vdcdc_onwakeup_config(&vdcdc_rail_config) == POWER_RAIL_ENABLED) {
                hw_bod_activate_channel(HW_BOD_CHANNEL_VDCDC);
        }

        HW_PMU_1V8_RAIL_CONFIG v18_rail_config;
        if (hw_pmu_get_1v8_onwakeup_config(&v18_rail_config) == POWER_RAIL_ENABLED) {
                hw_bod_activate_channel(HW_BOD_CHANNEL_1V8);
        }
}

void hw_bod_configure(void)
{
        hw_bod_activate_on_wakeup();


        /* Generate Reset on a BOD event */
        uint32_t mask = REG_MSK(CRG_TOP, BOD_CTRL_REG, BOD_DIS_VDDIO_COMP) |
                        REG_MSK(CRG_TOP, BOD_CTRL_REG, BOD_DIS_VDCDC_COMP) |
                        REG_MSK(CRG_TOP, BOD_CTRL_REG, BOD_DIS_VDD_COMP);

        uint32 mask_startup_status_reg = REG_MSK(CRG_TOP, STARTUP_STATUS_REG, BOD_VDDIO_OK_SYNC_RD)|
                                         REG_MSK(CRG_TOP, STARTUP_STATUS_REG, BOD_VDDD_OK_SYNC_RD) |
                                         REG_MSK(CRG_TOP, STARTUP_STATUS_REG, BOD_VDCDC_OK_SYNC_RD);

        REG_SET_MASKED(CRG_TOP, BOD_CTRL_REG, mask, false);

        while ((CRG_TOP->STARTUP_STATUS_REG & mask_startup_status_reg) != mask_startup_status_reg);
}

void hw_bod_set_channel_voltage_level(HW_BOD_CHANNEL channel, HW_BOD_VDD_LVL level)
{
        uint16_t voltage = BOD_VDD_COMPARATOR_LEVEL_UNDEF;

        switch (level) {
        case HW_BOD_VDD_LEVEL_SLEEP_0V70:
                voltage = BOD_VDD_COMPARATOR_LEVEL_700;
                break;
        case HW_BOD_VDD_LEVEL_ACTIVE_0V78:
                voltage = BOD_VDD_COMPARATOR_LEVEL_780;
                break;
        case HW_BOD_VDD_LEVEL_ACTIVE_1V05:
                voltage = BOD_VDD_COMPARATOR_LEVEL_1050;
                break;
        default:
                /* Invalid voltage, we should not reach here. */
                ASSERT_WARNING(0);
        }

        switch (channel) {
        case HW_BOD_CHANNEL_VDD:
                REG_SETF(CRG_TOP, BOD_CTRL_REG, BOD_SEL_VDD_LVL, voltage);
                while (REG_GETF(CRG_TOP, STARTUP_STATUS_REG, BOD_VDDD_LVL_RD) != voltage);
                break;
        case HW_BOD_CHANNEL_1V8:
        case HW_BOD_CHANNEL_VDCDC:
        default:
                /* Invalid channel, we should not reach here. */
                ASSERT_WARNING(0);
        }
}

__STATIC_INLINE HW_BOD_VDD_LVL get_bod_vdd_lvl(HW_BOD_VDD_COMP_LVL level)
{
        switch (level) {
        case BOD_VDD_COMPARATOR_LEVEL_700:
                return HW_BOD_VDD_LEVEL_SLEEP_0V70;

        case BOD_VDD_COMPARATOR_LEVEL_780:
                return HW_BOD_VDD_LEVEL_ACTIVE_0V78;

        case BOD_VDD_COMPARATOR_LEVEL_1050:
                return HW_BOD_VDD_LEVEL_ACTIVE_1V05;

        default:
                /* Invalid level, we should not reach here. */
                ASSERT_WARNING(0);
        }

        return HW_BOD_VDD_LEVEL_UNDEF;
}

HW_BOD_VDD_LVL hw_bod_get_channel_voltage_level(HW_BOD_CHANNEL channel)
{
        switch (channel) {
        case HW_BOD_CHANNEL_VDD:
                return get_bod_vdd_lvl(REG_GETF(CRG_TOP, BOD_CTRL_REG, BOD_SEL_VDD_LVL));
        case HW_BOD_CHANNEL_1V8:
        case HW_BOD_CHANNEL_VDCDC:
        default:
                /* Invalid channel, we should not reach here. */
                ASSERT_WARNING(0);
        }

        return HW_BOD_VDD_LEVEL_UNDEF;
}

#endif /* dg_configUSE_BOD */

bool hw_bod_get_status(HW_BOD_CHANNEL channel)
{
        bool ret = false;

        switch (channel) {
        case HW_BOD_CHANNEL_1V8:
                ret = REG_GETF(CRG_TOP, STARTUP_STATUS_REG, BOD_VDDIO_MASK_SYNC_RD);
                break;
        case HW_BOD_CHANNEL_VDCDC:
                ret = REG_GETF(CRG_TOP, STARTUP_STATUS_REG, BOD_VDCDC_MASK_SYNC_RD);
                break;
        case HW_BOD_CHANNEL_VDD:
                ret = REG_GETF(CRG_TOP, STARTUP_STATUS_REG, BOD_VDDD_MASK_SYNC_RD);
                break;
        default:
                /* Invalid channel, we should not reach here. */
                ASSERT_WARNING(0);
        }

        return ret;
}

void hw_bod_deactivate(void)
{

        uint32_t mask = REG_MSK(CRG_TOP, BOD_CTRL_REG, BOD_VDDIO_MASK)  |
                        REG_MSK(CRG_TOP, BOD_CTRL_REG, BOD_VDCDC_MASK)  |
                        REG_MSK(CRG_TOP, BOD_CTRL_REG, BOD_VDD_MASK);

        uint32 mask_startup_status_reg = REG_MSK(CRG_TOP, STARTUP_STATUS_REG, BOD_VDDIO_MASK_SYNC_RD) |
                                         REG_MSK(CRG_TOP, STARTUP_STATUS_REG, BOD_VDCDC_MASK_SYNC_RD) |
                                         REG_MSK(CRG_TOP, STARTUP_STATUS_REG, BOD_VDDD_MASK_SYNC_RD);

        REG_SET_MASKED(CRG_TOP, BOD_CTRL_REG, mask, UINT32_MAX);

        while ((CRG_TOP->STARTUP_STATUS_REG & mask_startup_status_reg) != mask_startup_status_reg);
}


/**
\}
\}
\}
*/
