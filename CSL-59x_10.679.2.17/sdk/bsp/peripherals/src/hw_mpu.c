/**
 * \addtogroup PLA_DRI_MEMORY
 * \{
 * \addtogroup HW_MPU Memory Protection Unit Low Level Driver
 * \{
 * \brief MPU Driver
 */

/**
 ****************************************************************************************
 *
 * @file hw_mpu.c
 *
 * @brief Implementation of the Memory Protection Unit (MPU) Low Level Driver.
 *
 * Copyright (C) 2023 Renesas Electronics Corporation and/or its affiliates.
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
#if dg_configUSE_HW_MPU


#include <stdbool.h>
#include "sdk_defs.h"
#include "hw_mpu.h"


/**
 * \brief memory_attribute_index
 *
 * Each row of the table corresponds to an MPU region (0-7) and the values are the mapped memory attribute indices.
 */
HW_MPU_ATTR_INDEX memory_attribute_index[] = {
        HW_MPU_ATTR_INDEX_0,
        HW_MPU_ATTR_INDEX_1,
        HW_MPU_ATTR_INDEX_2,
        HW_MPU_ATTR_INDEX_3,
        HW_MPU_ATTR_INDEX_4,
        HW_MPU_ATTR_INDEX_5,
        HW_MPU_ATTR_INDEX_6,
        HW_MPU_ATTR_INDEX_7
};

void hw_mpu_set_attribute_indirection(HW_MPU_REGION_NUM region_num, HW_MPU_ATTR_INDEX attr)
{
        ASSERT_ERROR(region_num > HW_MPU_REGION_NONE && region_num <= HW_MPU_REGION_7);
        ASSERT_ERROR(attr >= HW_MPU_ATTR_INDEX_0 && attr <= HW_MPU_ATTR_INDEX_7);
        memory_attribute_index[region_num] = attr;
}

void hw_mpu_reset_attribute_indirections()
{
        for (HW_MPU_REGION_NUM i = HW_MPU_REGION_0 ; i <= HW_MPU_REGION_7 ; i++)
                memory_attribute_index[i] = (HW_MPU_ATTR_INDEX)i;
}

void hw_mpu_config_region(HW_MPU_REGION_NUM region_num, mpu_region_config *cfg)
{
        ASSERT_ERROR(region_num > HW_MPU_REGION_NONE && region_num <= HW_MPU_REGION_7);
        if (!cfg) {
                GLOBAL_INT_DISABLE();

                ARM_MPU_ClrRegion(region_num);

                GLOBAL_INT_RESTORE();
        } else {
                /* The following assertions check whether the start and end addresses of the region intended
                 * to be protected comply with the 32-byte alignment rule as described in the ARM M33 MPU
                 * documentation. */
                ASSERT_WARNING((cfg->start_addr & MPU_END_ADDRESS_MASK) == 0);
                ASSERT_WARNING((cfg->end_addr & MPU_END_ADDRESS_MASK) == MPU_END_ADDRESS_MASK);
                /* The following assertion checks whether the value of the shareability is other-than 0x01
                 * which will lead to UNPREDICTABLE behavior according to ARMv8 MPU documentation. */
                ASSERT_ERROR(cfg->shareability != 0x01);

                GLOBAL_INT_DISABLE();
                /* Each of the eight M33 MPU regions is configured via a specific 8 bit set in the 32-bit MAIR0 and
                 * MAIR1 registers. The lower four regions are catered by MAIR0 and the upper four by MAIR1
                 * respectively. Please refer to the ARM M33 MPU documentation for a more detailed description. */
                ARM_MPU_SetMemAttr(memory_attribute_index[region_num], cfg->attributes);
                ARM_MPU_SetRegion(region_num,
                        ARM_MPU_RBAR(cfg->start_addr, cfg->shareability, ((cfg->access_permissions >> 1) & 0x01), (cfg->access_permissions & 0x01), cfg->execute_never),
                        ARM_MPU_RLAR(cfg->end_addr, memory_attribute_index[region_num]));
                GLOBAL_INT_RESTORE();
        }
}

void hw_mpu_fetch_region_configuration(HW_MPU_REGION_NUM region_num, mpu_region_config *cfg)
{
        ASSERT_WARNING(cfg != NULL);
        ASSERT_WARNING(memory_attribute_index[region_num] >= HW_MPU_ATTR_INDEX_0 && memory_attribute_index[region_num] <= HW_MPU_ATTR_INDEX_7);
        MPU->RNR = region_num;
        uint32_t rbar = MPU->RBAR;
        cfg->start_addr = rbar & MPU_RBAR_BASE_Msk;                             /*RBAR bits [31:5] - No bit shift since address is ROUNDED*/
        cfg->shareability = (rbar & MPU_RBAR_SH_Msk) >> MPU_RBAR_SH_Pos;        /*RBAR bits [4:3]*/
        cfg->access_permissions = (rbar & MPU_RBAR_AP_Msk) >> MPU_RBAR_AP_Pos;  /*RBAR bits [2:1]*/
        cfg->execute_never = rbar & MPU_RBAR_XN_Msk;                            /*RBAR bit [0]*/
        cfg->end_addr = ((uint32_t) ((REG_GETF(MPU, RLAR, LIMIT) << 5) | MPU_END_ADDRESS_MASK));
        switch (memory_attribute_index[region_num]) {
        case HW_MPU_ATTR_INDEX_0:
                cfg->attributes = ((uint8_t)(REG_GETF(MPU, MAIR0, Attr0)));
                break;
        case HW_MPU_ATTR_INDEX_1:
                cfg->attributes = ((uint8_t)(REG_GETF(MPU, MAIR0, Attr1)));
                break;
        case HW_MPU_ATTR_INDEX_2:
                cfg->attributes = ((uint8_t)(REG_GETF(MPU, MAIR0, Attr2)));
                break;
        case HW_MPU_ATTR_INDEX_3:
                cfg->attributes = ((uint8_t)(REG_GETF(MPU, MAIR0, Attr3)));
                break;
        case HW_MPU_ATTR_INDEX_4:
                cfg->attributes = ((uint8_t)(REG_GETF(MPU, MAIR1, Attr4)));
                break;
        case HW_MPU_ATTR_INDEX_5:
                cfg->attributes = ((uint8_t)(REG_GETF(MPU, MAIR1, Attr5)));
                break;
        case HW_MPU_ATTR_INDEX_6:
                cfg->attributes = ((uint8_t)(REG_GETF(MPU, MAIR1, Attr6)));
                break;
        case HW_MPU_ATTR_INDEX_7:
                cfg->attributes = ((uint8_t)(REG_GETF(MPU, MAIR1, Attr7)));
                break;
        default:
                break;
        }
}

#endif /* dg_configUSE_HW_MPU */
/**
 * \}
 * \}
 * \}
 */
