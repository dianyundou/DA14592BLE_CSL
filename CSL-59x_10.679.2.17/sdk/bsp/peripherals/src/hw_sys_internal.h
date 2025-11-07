/**
 ****************************************************************************************
 *
 * @file hw_sys_internal.h
 *
 * @brief System Driver Internal header file.
 *
 * This file contains system related administration definitions that enable detecting and applying
 * the identification of the target SDK board where the SDK firmware executes on.
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

#ifndef HW_SYS_INTERNAL_H_
#define HW_SYS_INTERNAL_H_


/**
 * \brief       Populate hw_sys_device_info_data with device information retrieved from the
 *              corresponding chip registers (Family, Device Chip ID, Revision and Step)
 *
 * \return      True, if the retrieved information is valid, otherwise false.
 *
 * \warning     The device variant information is not populated by this function, the
 *              hw_sys_dev_variant_init() must be used instead.
 */
bool hw_sys_device_info_init(void);

#if dg_configUSE_SYS_TCS
/**
 * \brief       Populate hw_sys_device_info_data with the device variant information retrieved from the
 *              corresponding TCS entry.
 *
 * \return      True, if the retrieved information is valid, otherwise false.
 */
bool hw_sys_device_variant_init(void);
#endif

/**
 * \brief       Check if a specific device information aspect matches the one of the target device.
 *
 * Use this function to check if a device information attribute equals to specific value using as input
 * arguments the public macros (no prefixed with underscore) defined in "bsp_device_definitions_internal.h".
 *
 * \param[in]   mask The device information attribute mask to be checked.
 * \param[in]   attribute The device information attribute value to compare with.
 *
 * \return      The result of comparison.
 *
 * Examples:
 *
 * \code
 * bool check;
 *
 * check = hw_sys_device_info_check(DEVICE_FAMILY_MASK, DA1468X);
 * check = hw_sys_device_info_check(DEVICE_FAMILY_MASK, DA1469X);
 *
 * check = hw_sys_device_info_check(DEVICE_VARIANT_MASK, DA14695);
 * check = hw_sys_device_info_check(DEVICE_VARIANT_MASK, DA14699);
 *
 * check = hw_sys_device_info_check(DEVICE_CHIP_ID_MASK, DEVICE_CHIP_ID_2522);
 * check = hw_sys_device_info_check(DEVICE_CHIP_ID_MASK, DEVICE_CHIP_ID_3080);
 *
 * check = hw_sys_device_info_check(DEVICE_REVISION_MASK, DEVICE_REV_A);
 * check = hw_sys_device_info_check(DEVICE_REVISION_MASK, DEVICE_REV_B);
 *
 * check = hw_sys_device_info_check(DEVICE_SWC_MASK, DEVICE_SWC_0);
 * check = hw_sys_device_info_check(DEVICE_SWC_MASK, DEVICE_SWC_1);
 *
 * check = hw_sys_device_info_check(DEVICE_STEP_MASK, DEVICE_STEP_A);
 * check = hw_sys_device_info_check(DEVICE_STEP_MASK, DEVICE_STEP_B);
 * check = hw_sys_device_info_check(DEVICE_STEP_MASK, DEVICE_STEP_C);
 * \endcode
 */
bool hw_sys_device_info_check(uint32_t mask, uint32_t attribute);

/**
 * \brief       Get the hw_sys_device_info_data where all device information attributes are
 *              populated.
 *
 * \return      hw_sys_device_info_data
 */
uint32_t hw_sys_get_device_info(void);

/**
 * \brief       Check that the firmware and the chip that it runs on are compatible with each other.
 *
 * \return      True, if the chip version is compliant, otherwise false.
 */
bool hw_sys_is_compatible_chip(void);

/**
 * \brief       Convert a CPU address to a physical address
 *
 * To calculate the physical address, the current remapping (SYS_CTRL_REG.REMAP_ADR0) is used.
 *
 * \warning: The Cache controller is considered enabled
 *
 * \param [in]  addr address seen by CPU
 *
 * \return      physical address (for DMA, AES/HASH etc.) -- can be same as or different to addr.
 *              To be more specific, the physical address is the same as the input address when
 *              the latter is neither a zero-based (remapped) nor a cacheable address from the code memory map region.
 *              In case of XiP memory, always return an address from the uncacheable memory region.
 */
uint32_t hw_sys_get_physical_addr(uint32_t addr);


#if dg_configHW_FCU_WAIT_CYCLES_MODE

/**
 * \brief Set FCU wait cycles to max.
 *
 * \note This function should be called inside FCU change cycles area
 */
__ALWAYS_RETAINED_CODE void hw_sys_fcu_set_max_wait_cycles(void);

/**
 * \brief Set FCU  wait cycles to optimum value. This value depends on
 *        AHB bus frequency and Vdd value
 *
 * \note This function should be called inside FCU change cycles area
 */
__ALWAYS_RETAINED_CODE void hw_sys_fcu_set_optimum_wait_cycles(void);

#endif /* dg_configHW_FCU_WAIT_CYCLES_MODE */


#endif /* HW_SYS_INTERNAL_H_ */

/**
 * \}
 * \}
 */
