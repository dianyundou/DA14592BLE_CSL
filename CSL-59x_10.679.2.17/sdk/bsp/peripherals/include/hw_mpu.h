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
 * @file hw_mpu.h
 *
 * @brief Definition of API for the Memory Protection Unit (MPU) Low Level Driver.
 *
 * The MPU is an optional ARM CM33 feature supported in DA14yyx SoC families that
 * enables protecting loosely defined regions of system RAM memory through enforcing
 * privilege and access rules per region. All MPU LLD terminology is based on the ARM
 * CM33 nomenclature.
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

#ifndef HW_MPU_H_
#define HW_MPU_H_


#if dg_configUSE_HW_MPU

#include <stdbool.h>
#include "sdk_defs.h"

#define MPU_START_ADDRESS_MASK (~MPU_END_ADDRESS_MASK)     /* Due to the 32-byte alignment of MPU-protected segments
                                                            * described in the ARM M33 documentation,
                                                            * all Start Addresses must be ANDed with this Mask */

#define MPU_END_ADDRESS_MASK (0x1F)                        /* Due to the 32-byte alignment of MPU-protected segments
                                                            * and the MPU region size must be a multiple of 32 bytes
                                                            * described in the ARM M33 documentation,
                                                            * all End Addresses must be ORed with this Mask */

/**
 * \brief Region Definitions
 *
 * The MPU divides the memory map into a number of eight regions. Each region has a defined
 * memory type and memory attributes that determine the behavior of accesses to the region.
 * A background (or default) region numbered as -1 exists with the same access attributes as
 * the generic memory map, but is accessible from privileged software only.
 */
typedef enum {
        HW_MPU_REGION_NONE = MPU_REGION_NONE,   /**< MPU Protection Omitted */
        HW_MPU_REGION_0 = MPU_REGION_0,         /**< MPU region 0 */
        HW_MPU_REGION_1 = MPU_REGION_1,         /**< MPU region 1 */
        HW_MPU_REGION_2 = MPU_REGION_2,         /**< MPU region 2 */
        HW_MPU_REGION_3 = MPU_REGION_3,         /**< MPU region 3 */
        HW_MPU_REGION_4 = MPU_REGION_4,         /**< MPU region 4 */
        HW_MPU_REGION_5 = MPU_REGION_5,         /**< MPU region 5 */
        HW_MPU_REGION_6 = MPU_REGION_6,         /**< MPU region 6 */
        HW_MPU_REGION_7 = MPU_REGION_7,         /**< MPU region 7 */

} HW_MPU_REGION_NUM;
/**
 * \brief Executable Region
 *
 * Attribute regarding the code execution from a particular region. The XN (eXecute Never) flag
 * must be zero and there must be read access for the privilege level in order to execute code
 * from the region, otherwise a memory manage (MemManage) fault is generated.
 */
typedef enum {
        HW_MPU_XN_FALSE = 0x00,         /**< Executable region */
        HW_MPU_XN_TRUE = 0x01,          /**< Execute never region */
} HW_MPU_XN;

/**
 * \brief Region Read/Write or Read Only
 *
 * Attribute regarding the access permission (AP) of a particular region with respect to privilege
 * level and read/write capabilities.
 */
typedef enum {
        HW_MPU_APH_RO_RW = 0x00,         /**< Read/write */
        HW_MPU_APH_RO_RO = 0x01,         /**< Read-only */
} HW_MPU_APH_RO;

/**
 * \brief Privileged or Non-Privileged access
 *
 * Attribute to allow an application the privilege of accessing CPU features such as memory, I/O,
 * enable/disable interrupts, setup the NVIC, etc. By system design it can be imperative to restrict
 * an application by defining accordingly the MPU settings for the corresponding region.
 */
typedef enum {
        HW_MPU_APL_NP_PRIVRW = 0x00,     /**< Privileged code only */
        HW_MPU_APL_NP_RW = 0x01,         /**< Any privilege level */
} HW_MPU_APL_NP;

/**
 * \brief Access Permissions
 *
 * Attribute regarding the access permission (AP) of a particular region with respect to privilege
 * level and read/write capabilities. Depending on the privilege configuration an application can
 * access or not CPU features such as memory, I/O, enable/disable interrupts, setup the NVIC, etc.
 * By system design it can be imperative to restrict an application by defining accordingly the
 * MPU settings for the corresponding region.
 * This enumerator is the superposition of HW_MPU_APH_RO and HW_MPU_APL_NP and is provided for
 * cases where the combined AP section is required.
 */
typedef enum {
        HW_MPU_AP_PRIVRW = ARM_MPU_AP_(HW_MPU_APH_RO_RW, HW_MPU_APL_NP_PRIVRW), /**< Read/write by privileged code only */
        HW_MPU_AP_RW = ARM_MPU_AP_(HW_MPU_APH_RO_RW, HW_MPU_APL_NP_RW),         /**< Read/write by any privilege level */
        HW_MPU_AP_PRIVRO = ARM_MPU_AP_(HW_MPU_APH_RO_RO, HW_MPU_APL_NP_PRIVRW), /**< Read-only by privileged code only */
        HW_MPU_AP_RO = ARM_MPU_AP_(HW_MPU_APH_RO_RO, HW_MPU_APL_NP_RW),         /**< Read-only by any privilege level */
} HW_MPU_AP;

/**
 * \brief Memory attributes
 */
typedef enum {
        HW_MPU_ATTR_INDEX_0,         /**< MPU attribute 0 */
        HW_MPU_ATTR_INDEX_1,         /**< MPU attribute 1 */
        HW_MPU_ATTR_INDEX_2,         /**< MPU attribute 2 */
        HW_MPU_ATTR_INDEX_3,         /**< MPU attribute 3 */
        HW_MPU_ATTR_INDEX_4,         /**< MPU attribute 4 */
        HW_MPU_ATTR_INDEX_5,         /**< MPU attribute 5 */
        HW_MPU_ATTR_INDEX_6,         /**< MPU attribute 6 */
        HW_MPU_ATTR_INDEX_7          /**< MPU attribute 7 */
} HW_MPU_ATTR_INDEX;

/**
 * \brief Shareability
 *
 * Attribute regarding the Shareability status (SH) of a particular region. In our case (ARMv8-M33) the options
 * regarding Shareability are:
 *
 *              [Non-shareable] - This represents memory accessible only by a single processor or other agent,
 *              so memory accesses never need to be synchronized with other processors.
 *              [Inner Shareable] - This represents a shareability domain that can be shared by multiple processors,
 *              but not necessarily all of the agents in the system. A system might have multiple Inner Shareable
 *              domains. An operation that affects one Inner Shareable domain does not affect other Inner Shareable
 *              domains in the system.
 *              [Outer Shareable] - An outer shareable (OSH) domain re-order is shared by multiple agents and can
 *              consist of one or more inner shareable domains. An operation that affects an outer shareable domain
 *              also implicitly affects all inner shareable domains inside it. However, it does not otherwise behave
 *              as an inner shareable operation.
 *
 * CAUTION: The value of HW_MPU_SH must ALWAYS be other-than 0x01. A value of 0x01 will lead to UNPREDICTABLE behavior
 * according to ARMv8 MPU documentation.
 * The most common Shareability status is Non-Shareable.
 */
typedef enum {
        HW_MPU_SH_NS = ARM_MPU_SH_NON,          /**< Non-Shareable */
        HW_MPU_SH_OS = ARM_MPU_SH_OUTER,        /**< Outer Shareable */
        HW_MPU_SH_IS = ARM_MPU_SH_INNER,        /**< Inner Shareable */
} HW_MPU_SH;

/**
 * \brief Memory Type
 *
 * Attribute regarding the memory type of a particular region. According to ARM CM33 nomenclature
 * two memory types are defined: device memory pertains to a memory-mapped region for a peripheral,
 * while normal memory is instead relevant to CPU use. The following enumerator sums up the two most
 * commonly deployed attribute setups:
 * 0x00 - Device Memory,  non-Gathering, non-Re-Ordering, non-Early-Write-Acknowledgement (nGnRnE).
 * 0x44 - Inner Memory normal and non-Cacheable, Outer Memory normal and non-Cacheable.
 */
typedef enum {
        HW_MPU_ATTR_DEVICE = 0x00,      /**< Device Memory, nGnRnE */
        HW_MPU_ATTR_NORMAL = 0x44,      /**< Normal memory, Outer non-cacheable, Inner non-cacheable */
} HW_MPU_ATTR;

/**
 * \brief Memory Region Configuration
 */
typedef struct {
        uint32_t start_addr;            /**< MPU region start address. Address will be rounded to previous 32-byte multiple */
        uint32_t end_addr;              /**< MPU region end address. Address will be rounded to next 32-byte multiple minus 1 */
        HW_MPU_AP access_permissions;   /**< MPU region access permissions */
        HW_MPU_SH shareability;         /**< MPU region Shareability status */
        HW_MPU_XN execute_never;        /**< Defines whether code can be executed from this region */
        HW_MPU_ATTR attributes;         /**< MPU region's memory attributes */
} mpu_region_config;

/**
 * \brief Enables/Disables the operation of MPU during hard fault, NMI, and FAULTMASK handlers.
 *
 * \param [in] hfnmiena Controls (enable/disable) operation of MPU during HardFault and NMI handlers.
 *
 *              When disabled, MPU is disabled during HardFault and NMI handlers, regardless of the
 *              value of the ENABLE bit.
 *              When enabled, the MPU is enabled during HardFault and NMI handlers.
 *
 */
__STATIC_FORCEINLINE void hw_mpu_hardfault_nmi_handlers_enable(bool hfnmiena)
{
        REG_SETF(MPU, CTRL, HFNMIENA, hfnmiena);
}

/**
 * \brief Enables/Disables Privileged Background Access
 * \param [in] privdefena Controls (enable/disable) privileged access to the background region.
 *
 *              When disabled, any access to the background region will cause a memory manage fault.
 *              When enabled, privileged accesses to the background region are allowed.
 *
 *              In handler mode, execution is always privileged. In thread mode
 *              privilege level can be set using the 'nPRIV' field of the control register.
 *              For manipulating nPRIV, check __set_CONTROL() and __get_CONTROL() CMSIS API calls.
 *              Hard fault and NMI handlers always operate with MPU disabled, accessing the
 *              default memory map as normal. The same can be true when FAULTMASK is set to 1,
 *              effectively masking Hard Fault exceptions by raising the current priority level to -1.
 *              FAULTMASK can only be set in privileged mode except from within NMI and HardFault
 *              Handlers (in which cases lockup state will be entered).
 *
 */
__STATIC_FORCEINLINE void hw_mpu_privileged_background_access_enable(bool privdefena)
{
        REG_SETF(MPU, CTRL, PRIVDEFENA, privdefena);
}

/**
 * \brief Initializes the MPU by disabling its operation during faults, defining the background region
 *              privilege access and finally by enabling the actual HW block.
 * \param [in] privdefena Controls (enable/disable) privileged access to the background region.
 *
 *              When disabled, any access to the background region will cause a memory manage fault.
 *              When enabled, privileged accesses to the background region are allowed.
 *
 *              In handler mode, execution is always privileged. In thread mode
 *              privilege level can be set using the 'nPRIV' field of the control register.
 *              For manipulating nPRIV, check __set_CONTROL() and __get_CONTROL() CMSIS API calls.
 *              Hard fault and NMI handlers always operate with MPU disabled, accessing the
 *              default memory map as normal. The same can be true when FAULTMASK is set to 1,
 *              effectively masking Hard Fault exceptions by raising the current priority level to -1.
 *              FAULTMASK can only be set in privileged mode except from within NMI and HardFault
 *              Handlers (in which cases lockup state will be entered).
 */
__STATIC_FORCEINLINE void hw_mpu_enable(bool privdefena)
{
        uint32_t MPU_Control = (privdefena ? REG_MSK(MPU, CTRL, PRIVDEFENA) : 0);
        ARM_MPU_Enable(MPU_Control);
}

/**
 * \brief Disables the MPU
 *
 */
__STATIC_FORCEINLINE void hw_mpu_disable(void)
{
        ARM_MPU_Disable();
        __ISB();
}

/**
 * \brief Checks if MPU is enabled
 *
 * \return true if enabled, false otherwise
 */
__STATIC_FORCEINLINE bool hw_mpu_is_enabled(void)
{
        return (!!REG_GETF(MPU, CTRL, ENABLE));
}

/**
 * \brief Configures an MPU attribute indirection.
 *
 * \param [in] region_num Region number.
 * \param [in] attr The region number selects one from the available memory attributes.
 *
*/
void hw_mpu_set_attribute_indirection(HW_MPU_REGION_NUM region_num, HW_MPU_ATTR_INDEX attr);

/**
 * \brief Resets all MPU attribute indirections. MPU Attribute Index pattern is returned to one-to-one configuration.
*/
void hw_mpu_reset_attribute_indirections();

/**
 * \brief Configures an MPU region.
 * Region's start and end addresses will be aligned to 32 byte boundary. The start address
 * is ANDed with 0xFFFFFFE0 whereas the end address is ORed with 0x1F.
 *
 * The following accesses will generate a hard fault:
 * - An access to an address that matches in more than one region.
 * - An access that does not match all the access conditions for that region.
 * - An access to the background region, depending on the privilege mode and the value of
 *      the 'privdefena' parameter when MPU is enabled.
 *
 * \param [in] region_num Region number
 * \param [in] cfg Region configuration. When cfg is NULL the particular region is disabled.
 *
 * \note The regions intended for protection will be rounded to increments of 32 bytes in any case.
 *       This is a result of the fact that the 5 low bits of RLAR and RBAR registers are reserved
 *       for other purposes. The first two assertions serve as a reminder of that detail.
*/
void hw_mpu_config_region(HW_MPU_REGION_NUM region_num, mpu_region_config *cfg);

/**
 * \brief Fetches the configuration of an MPU region.
 *
 * \param [in] region_num Region number.
 * \param [in] *cfg Region configuration.
*/
void hw_mpu_fetch_region_configuration(HW_MPU_REGION_NUM region_num, mpu_region_config *cfg);

#endif /* dg_configUSE_HW_MPU */


#endif /* HW_MPU_H_ */

/**
 * \}
 * \}
 */
