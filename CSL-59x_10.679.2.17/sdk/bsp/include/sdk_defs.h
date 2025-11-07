/**
 * \addtogroup PLA_BSP_SYSTEM
 * \{
 * \addtogroup BSP_DEFINITIONS SDK Platform Definitions
 *
 * \brief Platform definitions
 *
 * \{
 */

/**
 ****************************************************************************************
 *
 * @file sdk_defs.h
 *
 * @brief Central include header file with platform definitions.
 *
 * Copyright (C) 2015-2023 Renesas Electronics Corporation and/or its affiliates.
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

#ifndef __SDK_DEFS_H__
#define __SDK_DEFS_H__

#include <stddef.h>

#ifdef __cplusplus
 extern "C" {
#endif

#ifdef __GNUC__
#  define GCC_VERSION (__GNUC__ * 10000 \
                               + __GNUC_MINOR__ * 100 \
                               + __GNUC_PATCHLEVEL__)
/* assert gcc version is at least 4.9.3 */
#  if GCC_VERSION < 40903
#    error "Please use gcc version 4.9.3 or newer!"
#  endif
#endif

#   include "DA1459x-00.h"

#include "system_DA1459x.h"                         /* DA1459x System */

/**
 *  \brief MTB-related macros
 *
 *  MTB_MASTER_REG:                 Address of MTB_MASTER register
 *  MTB_MASTER_REG_DISABLE_VAL:     Value to be written to MTB_MASTER in order to disable the MTB
 */
#define MTB_MASTER_REG                          ((uint32_t *) 0xE0043004)
#define MTB_MASTER_REG_DISABLE_VAL              (dg_configMTB_MASK)


/************************
 * Memory map
 ************************/

/**
 * \brief Remapped device base address.
 */
#define MEMORY_REMAPPED_BASE    0x00000000UL
 /* The max remapped  region size according to FLASH_REGION_SIZE register field */
#define MEMORY_REMAPPED_END     0x02000000UL

/**
 * \brief Remapped device memory size.
 */
#define MEMORY_REMAPPED_SIZE    (MEMORY_REMAPPED_END - MEMORY_REMAPPED_BASE)

/**
 * \brief ROM base address.
 */
#define MEMORY_ROM_BASE         0x900000UL
#define MEMORY_ROM_END          (MEMORY_ROM_BASE + 0x48000L)

/**
 * \brief ROM memory size.
 */
#define MEMORY_ROM_SIZE         (MEMORY_ROM_END - MEMORY_ROM_BASE)

/**
 * \brief OTP memory base address.
 */

/**
 * \brief OTP memory size.
 */

/**
 * \brief SYSTEM RAM base address.
 */
#define MEMORY_SYSRAM_S_BASE    0x20000000UL
#define MEMORY_SYSRAM_S_END     (MEMORY_SYSRAM_S_BASE + 0x18000UL)

/**
 * \brief SYSTEM RAM size.
 */
#define MEMORY_SYSRAM_S_SIZE    (MEMORY_SYSRAM_S_END - MEMORY_SYSRAM_S_BASE)

/**
* \brief SYSTEM RAM (code) base address.
*/
#define MEMORY_SYSRAM_BASE      0x00800000UL
#define MEMORY_SYSRAM_END       (MEMORY_SYSRAM_BASE + 0x18000UL)

/**
* \brief SYSTEM RAM (code) size.
*/
#define MEMORY_SYSRAM_SIZE      (MEMORY_SYSRAM_END - MEMORY_SYSRAM_BASE)

/**
 * \brief CACHE RAM base address.
 */
#define MEMORY_CACHERAM_BASE    0x36000000UL
#define MEMORY_CACHERAM_END     0x36002000UL
#define MEMORY_CMAC_CACHERAM_BASE       0x40108000UL
#define MEMORY_CMAC_CACHERAM_END        0x4010A000UL

/**
 * \brief CACHE RAM size.
 */
#define MEMORY_CACHERAM_SIZE    (MEMORY_CACHERAM_END - MEMORY_CACHERAM_BASE)
#define MEMORY_CMAC_CACHERAM_SIZE       (MEMORY_CMAC_CACHERAM_END - MEMORY_CMAC_CACHERAM_BASE)

/**
 * \brief QSPI Flash base address.
 */
/**
  * QSPI AHB-C(ode) bus (cached). Accesses through this bus
  * are restricted according to CACHE_FLASH_REG.
  */
#define MEMORY_QSPIF_BASE       (0x16000000UL)
#define MEMORY_QSPIF_END        (MEMORY_QSPIF_BASE + 0x2000000UL)
/**
  * QSPI AHB-S(system) bus (Not cached). Accesses through this bus
  * are not affected by CACHE_FLASH_REG.
  */
#define MEMORY_QSPIF_S_BASE     (0x32000000UL)
#define MEMORY_QSPIF_S_END      (MEMORY_QSPIF_S_BASE + 0x2000000UL)

/**
 * \brief QSPI Flash memory size.
 */
#define MEMORY_QSPIF_SIZE       (MEMORY_QSPIF_END - MEMORY_QSPIF_BASE)

/**
 * eFlash AHB-C(ode) bus (cached). Accesses through this bus
 * are restricted according to CACHE_FLASH_REG.
 */
#define MEMORY_EFLASH_BASE (0xA00000UL)
#define MEMORY_EFLASH_END  (MEMORY_EFLASH_BASE + 0x40800UL)

/**
 * eFlash AHB-S(system) bus (Not cached). Accesses through this bus
 * are not affected by CACHE_FLASH_REG.
 */
#define MEMORY_EFLASH_S_BASE (0x31000000UL)
#define MEMORY_EFLASH_S_END  (MEMORY_EFLASH_S_BASE + 0x40800UL)

/**
 * \brief eFlash memory size.
 */
#define MEMORY_EFLASH_SIZE (MEMORY_EFLASH_END - MEMORY_EFLASH_BASE)

/**
 * \brief Primary product header base address
 */
#if defined(NVMS_PRODUCT_HEADER_PART_START)
# if (dg_configCODE_LOCATION == NON_VOLATILE_IS_EMBEDDED_FLASH) || \
     (dg_configCODE_LOCATION == NON_VOLATILE_IS_QSPI_FLASH)
#  define PRIMARY_PRODUCT_HEADER_BASE     (NVMS_PRODUCT_HEADER_PART_START)
# else
#  undef PRIMARY_PRODUCT_HEADER_BASE
# endif
#else
# if (dg_configCODE_LOCATION == NON_VOLATILE_IS_EMBEDDED_FLASH)
#  define PRIMARY_PRODUCT_HEADER_BASE     (EFLASH_MEM1_VIRTUAL_BASE_ADDR + 0x1000)
# elif (dg_configCODE_LOCATION == NON_VOLATILE_IS_QSPI_FLASH)
#  define PRIMARY_PRODUCT_HEADER_BASE     (QSPI_MEM1_VIRTUAL_BASE_ADDR + 0x1000)
# else
#  undef PRIMARY_PRODUCT_HEADER_BASE
# endif
#endif

/**
 * \brief Backup product header base address
 */
#if (dg_configCODE_LOCATION == NON_VOLATILE_IS_EMBEDDED_FLASH)
#  define BACKUP_PRODUCT_HEADER_BASE      (PRIMARY_PRODUCT_HEADER_BASE + 0x800)
#elif (dg_configCODE_LOCATION == NON_VOLATILE_IS_QSPI_FLASH)
#  define BACKUP_PRODUCT_HEADER_BASE      (PRIMARY_PRODUCT_HEADER_BASE + 0x1000)
#else
#  undef BACKUP_PRODUCT_HEADER_BASE
#endif


/**
 * \brief TRNG FIFO address
 */



/**
 * \brief OTP User Data Encryption Keys memory base address
 */
#define MEMORY_EFLASH_USER_DATA_KEYS_BASE               (0x00000800UL)
#define MEMORY_EFLASH_USER_DATA_KEYS_END                (0x00000900UL)
#define MEMORY_EFLASH_USER_DATA_KEYS_REVOCATION_BASE    (0x000007C0UL)
#define MEMORY_EFLASH_USER_DATA_KEYS_REVOCATION_END     (0x000007DFUL)
#define MEMORY_EFLASH_USER_DATA_KEY_SIZE                (0x20)


#define WITHIN_RANGE(_a, _s, _e)        (((uint32_t)(_a) >= (uint32_t)(_s)) && ((uint32_t)(_a) < (uint32_t)(_e)))

/**
 * \brief Address is in the remapped memory region
 */
#define IS_REMAPPED_ADDRESS(_a)         WITHIN_RANGE((_a), MEMORY_REMAPPED_BASE, MEMORY_REMAPPED_END)

/**
 * \brief Address is in the ROM region
 */
#define IS_ROM_ADDRESS(_a)              WITHIN_RANGE((_a), MEMORY_ROM_BASE, MEMORY_ROM_END)

/**
 * \brief Address is in the OTP memory region
 */
/**
* \brief Address is in the SYSTEM RAM region
*/
#define IS_SYSRAM_ADDRESS(_a)           WITHIN_RANGE((_a), MEMORY_SYSRAM_BASE, MEMORY_SYSRAM_END)

/**
* \brief Address is in the SYSTEM RAM (code) region
*/
#define IS_SYSRAM_S_ADDRESS(_a)         WITHIN_RANGE((_a), MEMORY_SYSRAM_S_BASE, MEMORY_SYSRAM_S_END)

/**
 * \brief Address is in the CACHE RAM region
 */
#define IS_CACHERAM_ADDRESS(_a)         WITHIN_RANGE((_a), MEMORY_CACHERAM_BASE, MEMORY_CACHERAM_END)

/**
 * \brief Address is in the QSPI Flash memory region
 */
#define IS_QSPIF_ADDRESS(_a)            WITHIN_RANGE((_a), MEMORY_QSPIF_BASE, MEMORY_QSPIF_END)

/**
 * \brief Address is in the QSPI AHB-S(ystem) memory region
 */
#define IS_QSPIF_S_ADDRESS(_a)            WITHIN_RANGE((_a), MEMORY_QSPIF_S_BASE, MEMORY_QSPIF_S_END)

/**
 * \brief Address is in eFlash memory code region
 */
#define IS_EFLASH_ADDRESS(_a)      WITHIN_RANGE((_a), MEMORY_EFLASH_BASE, MEMORY_EFLASH_END)

/**
 * \brief Address is in eFlash memory data region
 */
#define IS_EFLASH_S_ADDRESS(_a)      WITHIN_RANGE((_a), MEMORY_EFLASH_S_BASE, MEMORY_EFLASH_S_END)

 /**
  * \brief The Sector Size of QSPI flash memories
  */
#define FLASH_SECTOR_SIZE              (0x1000)




/**
 * \brief The Virtual Base Address to access the Embedded Flash memory
 *
 */
#define EFLASH_MEM1_VIRTUAL_BASE_ADDR               (0x00000000)

/**
 * \brief The Virtual Base Address to access the QSPI Flash memory connected to QSPIC.
 *
 * \note  To ensure proper functioning, the QSPI virtual base address must be aligned with the QSPI
 *        flash sector size. Since the EFLASH size is not aligned with the QSPI flash sector size,
 *        this is handled by the macro.
 */
#define QSPI_MEM1_VIRTUAL_BASE_ADDR                 (((EFLASH_MEM1_VIRTUAL_BASE_ADDR + MEMORY_EFLASH_SIZE) \
                                                     & ~(FLASH_SECTOR_SIZE - 1)) + FLASH_SECTOR_SIZE)

/**
 * \brief Zero-initialized data retained memory attribute
 */
#define __RETAINED                      __attribute__((section("retention_mem_zi")))    // RetRAM0
#define __RETAINED_1                    __attribute__((section("retention_mem_1_zi")))  // RetRAM1

/**
 * \brief Initialized data retained memory attribute
 */
#define __RETAINED_RW                   __attribute__((section("retention_mem_init")))

/**
 * \brief Uninitialized data retained memory attribute. Used for variables that should
 * not be initialized during startup.
 */
#define __RETAINED_UNINIT               __attribute__((section("retention_mem_uninit")))

 /**
  * \brief Constant data retained memory attribute
  */
#define __RETAINED_CONST_INIT           __attribute__((section("retention_mem_const")))

/**
 * \brief Zero-initialized data retained memory attribute visible by CMAC
 */
#ifdef CONFIG_USE_BLE
#define __RETAINED_CMI                  __attribute__((section("retention_mem_cmi_zi")))
#else
#define __RETAINED_CMI                  __RETAINED
#endif /* CONFIG_USE_BLE */

/**
 * \brief Text retained memory attribute
 */
#define __RETAIN_CODE_ATTR              __attribute__((section("text_retained"))) __attribute__((noinline)) __attribute__((optimize ("no-tree-switch-conversion")))
# if dg_configREDUCE_RETAINED_CODE
// When the code is executed from eFlash if dg_configREDUCE_RETAINED_CODE == 1, the __ALWAYS_RETAINED_CODE attribute is used to selectively
// retain only the necessary functions in SysRAM. When dg_configREDUCE_RETAINED_CODE is set to 0, the system requires more functions to be retained in SysRAM.
// The __RETAINED_CODE and __ALWAYS_RETAINED_CODE attributes determine which functions will be retained.
// When the code is executed from QSPI the dg_configREDUCE_RETAINED_CODE can only be set to 0. The __RETAINED_CODE and __ALWAYS_RETAINED_CODE attributes determine which functions
// will be retained in RAM. Please refer to definition of dg_configREDUCE_RETAINED_CODE for more details.
#  if ((dg_configCODE_LOCATION == NON_VOLATILE_IS_EMBEDDED_FLASH) && \
     (dg_configEXEC_MODE == MODE_IS_CACHED))
#  define __RETAINED_CODE
#  define __ALWAYS_RETAINED_CODE        __RETAIN_CODE_ATTR
#  else /* RAM build */
#  define __RETAINED_CODE
#  define __ALWAYS_RETAINED_CODE
#  endif
# else /* dg_configREDUCE_RETAINED_CODE == 0 */
#  if (((dg_configCODE_LOCATION == NON_VOLATILE_IS_QSPI_FLASH) || \
    (dg_configCODE_LOCATION == NON_VOLATILE_IS_EMBEDDED_FLASH)) && \
    (dg_configEXEC_MODE == MODE_IS_CACHED))
#  define __RETAINED_CODE               __RETAIN_CODE_ATTR
#  define __ALWAYS_RETAINED_CODE        __RETAIN_CODE_ATTR
#  else /* RAM build */
#  define __RETAINED_CODE
#  define __ALWAYS_RETAINED_CODE
#  endif
# endif /* dg_configREDUCE_RETAINED_CODE */


# if dg_configRETAIN_HOT_CODE
# define __RETAINED_HOT_CODE             __RETAINED_CODE
# else
# define __RETAINED_HOT_CODE
# endif

/**
 * \brief Attribute to silence warnings about unused parameters/variables/function
 */
#define __UNUSED                __attribute__((unused))

/**
 * \brief Attribute to tell the compiler to consider a symbol as externally visible (for LTO)
 */
#define __LTO_EXT               __attribute__((externally_visible))

// Forward declaration
__ALWAYS_RETAINED_CODE void hw_sys_assert_trigger_gpio(void);

/**
 * \brief Assert as warning macro
 *
 * \note Active only while in development mode
 */
#if (dg_configIMAGE_SETUP == DEVELOPMENT_MODE)
void assertion_functions_set_to_uninit(void);
void assertion_functions_set_to_init(void);

#define ASSERT_WARNING(a)       if (!(a)) {                                                                     \
                                        extern __NO_RETURN void (*assert_warning_func)(void);                   \
                                        register unsigned r4 __asm__("r4") = (uint32_t)&assert_warning_func;    \
                                        __asm__ volatile (                                                      \
                                                "  .syntax unified  \n"                                         \
                                                "LDR r4, [r4, #0]\n"                                            \
                                                "BX  r4 \n"                                                     \
                                                :                                                               \
                                                : "r" (r4)                                                      \
                                                : "r0","r1","r2","r3" /* force the compiler to avoid R0-R3 */   \
                                        );                                                                      \
                                        __builtin_unreachable();                                                \
                                }
#else
#define ASSERT_WARNING(a)       do { if (!(a)); } while (0);
#endif /* dg_configIMAGE_SETUP */
#define ASSERT_WARNING_UNINIT(a) ASSERT_WARNING(a)

/**
 * \brief Assert as error macro
 *
 */
#if (dg_configIMAGE_SETUP == DEVELOPMENT_MODE)
#define ASSERT_ERROR(a)         ASSERT_WARNING(a)
#else
#define ASSERT_ERROR(a)         if (!(a)) {                                                                     \
                                        __asm__ volatile (                                                      \
                                                "  .syntax unified  \n"                                         \
                                                "CPSID I\n"                                                     \
                                                "BKPT  2\n"                                                     \
                                                :                                                               \
                                                :                                                               \
                                                : "memory"                                                      \
                                        );                                                                      \
                                        __builtin_unreachable();                                                \
                                }
#endif /* dg_configIMAGE_SETUP */

#define ASSERT_ERROR_UNINIT(a)  ASSERT_ERROR(a)

/**
 * \brief Macro to disable all interrupts
 *
 * This macro must always be used with GLOBAL_INT_RESTORE(). E.g.
 *
 * \code{.c}
 * GLOBAL_INT_DISABLE();
 *  ... code to be executed with interrupts disabled ...
 * GLOBAL_INT_RESTORE();
 * \endcode
 *
 * \sa GLOBAL_INT_RESTORE
 */
#define GLOBAL_INT_DISABLE()                                                            \
        do {                                                                            \
                unsigned int __l_irq_rest;                                              \
                __ASM volatile ("mrs   %0, primask  \n\t"                               \
                              "mov   r1, $1     \n\t"                                   \
                              "msr   primask, r1  \n\t"                                 \
                              : "=r" (__l_irq_rest)                                     \
                              :                                                         \
                              : "r1"                                                    \
                              );                                                        \
               DBG_CONFIGURE_HIGH(CMN_TIMING_DEBUG, CMNDBG_CRITICAL_SECTION);

/**
 * \brief Macro to restore all interrupts
 *
 * This macro must always be used after GLOBAL_INT_DISABLE(). E.g.
 *
 * \code{.c}
 * GLOBAL_INT_DISABLE();
 *  ... code to be executed with interrupts disabled ...
 * GLOBAL_INT_RESTORE();
 * \endcode
 *
 * \sa GLOBAL_INT_DISABLE
 */
#define GLOBAL_INT_RESTORE()                                                            \
                if (__l_irq_rest == 0) {                                                \
                        DBG_CONFIGURE_LOW(CMN_TIMING_DEBUG, CMNDBG_CRITICAL_SECTION);   \
                }                                                                       \
                __ASM volatile ("msr   primask, %0  \n\t"                               \
                                                          :                             \
                                                          : "r" (__l_irq_rest)          \
                                                          :                             \
                                                          );                            \
        } while (0)

#define containingoffset(address, type, field) ((type*)((uint8*)(address)-(size_t)(&((type*)0)->field)))

/**
 * \brief Macro the minimum of two values
 *
 * \param[in] a         First value
 * \param[in] b         Second value
 */
#define MIN(a, b)  (((a) < (b)) ? (a) : (b))

/**
 * \brief Macro the maximum of two values
 *
 * \param[in] a         First value
 * \param[in] b         Second value
 */
#define MAX(a, b)  (((a) > (b)) ? (a) : (b))

/**
 * \brief Macro to swap the bytes of a 16-bit variable
 *
 * \param[in] a         The 16-bit variable
 */
#if defined(__GNUC__)
#define SWAP16(a) __builtin_bswap16(a)
#else
#define SWAP16(a) ((a<<8) | (a>>8))
#endif

/**
 * \brief Macro to swap the bytes of a 32-bit variable
 *
 * \param[in] a         The 32-bit variable
 */
#if defined(__GNUC__)
#define SWAP32(a) __builtin_bswap32(a)
#else
#define SWAP32(a) ((a>>24 & 0xff) | (a>>8 & 0xff00) | (a<<8 & 0xff0000) | (a<<24 & 0xff000000))
#endif

#if defined(__GNUC__)
#define DEPRECATED __attribute__((deprecated))
#else
#pragma message "Deprecated macro must be implemented for this compiler"
#define DEPRECATED
#endif

#if defined(__GNUC__)
#define DEPRECATED_MSG(msg) __attribute__((deprecated(msg)))
#else
#pragma message "Deprecated macro must be implemented for this compiler"
#define DEPRECATED_MSG(msg)
#endif

#if defined(__GNUC__)
#define DEPRECATED_MACRO(macro, msg) DEPRECATED_MSG(msg) __STATIC_INLINE void macro(void) {}
#else
#pragma message "Deprecated macro must be implemented for this compiler"
#define DEPRECATED_MACRO(macro, msg)
#endif

/* The following exist in ROM code */
void __aeabi_memcpy(void *dest, const void *src, size_t n);
void __aeabi_memmove(void *dest, const void *src, size_t n);
void __aeabi_memset(void *dest, size_t n, int c);

/**
 * \brief Optimized memcpy
 */
#define OPT_MEMCPY      __aeabi_memcpy

/**
 * \brief Optimized memmove
 */
#define OPT_MEMMOVE     __aeabi_memmove

/**
 * \brief Optimized memset
 */
#define OPT_MEMSET(s, c, n)      __aeabi_memset(s, n, c)

/**
 * \brief Access register field mask.
 *
 * Returns a register field mask (aimed to be used with local variables).
 * e.g.
 * \code
 * uint32_t tmp;
 *
 * tmp = XX_BASE->YY_REG;
 *
 * if (tmp & REG_MSK(XX_BASE, YY_REG, FIELD_Z)) {
 * ...
 * \endcode
 */
#define REG_MSK(base, reg, field) \
        (base ## _ ## reg ## _ ## field ## _Msk)

/**
 * \brief Access register field position.
 *
 * Returns a register field position (aimed to be used with local variables).
 */
#define REG_POS(base, reg, field) \
        (base ## _ ## reg ## _ ## field ## _Pos)

/**
 * \brief Access register field value.
 *
 * Returns a register field value (aimed to be used with local variables).
 * e.g.
 * \code
 * uint32_t tmp;
 * int counter;
 * tmp = XX_BASE->YY_REG;
 * counter = REG_GET_FIELD(XX_BASE, YY_REG, FIELD_Z, tmp);
 * ...
 * \endcode
 */
#define REG_GET_FIELD(base, reg, field, var) \
        ((var & (base ## _ ## reg ## _ ## field ## _Msk)) >> \
                (base ## _ ## reg ## _ ## field ## _Pos))

/**
 * \brief Set register field value.
 *
 * Sets a register field value (aimed to be used with local variables).
 * e.g.
 * \code
 * uint32_t tmp;
 *
 * tmp = XX_BASE->YY_REG;
 * REG_SET_FIELD(XX_BASE, YY_REG, FIELD_A, tmp, 10);
 * REG_SET_FIELD(XX_BASE, YY_REG, FIELD_B, tmp, 2);
 * XX_BASE->YY_REG = tmp;
 * ...
 * \endcode
 */
#define REG_SET_FIELD(base, reg, field, var, val) \
        var = ((var & ~((base ## _ ## reg ## _ ## field ## _Msk))) | \
                (((val) << (base ## _ ## reg ## _ ## field ## _Pos)) & \
                (base ## _ ## reg ## _ ## field ## _Msk)))

/**
 * \brief Set memory field value.
 *
 * Sets a memory field value using a mask (aimed to be used with local variables).
 * e.g.
 * \code
 * uint32_t tmp = *(volatile uint32_t *)0x50000000;
 *
 * RAW_SET_FIELD(tmp, 0x1UL, 1);
 * ...
 * \endcode
 */
#define RAW_SET_FIELD(mem, mask, val) \
        mem = ((mem & ~((mask))) | (((val) << (__builtin_ctz(mask))) & (mask)))

/**
 * \brief Clear register field value.
 *
 * Clears a register field value (aimed to be used with local variables).
 * e.g.
 * \code
 * uint32_t tmp;
 *
 * tmp = XX_BASE->YY_REG;
 * REG_CLR_FIELD(XX_BASE, YY_REG, FIELD_A, tmp);
 * REG_CLR_FIELD(XX_BASE, YY_REG, FIELD_B, tmp);
 * XX_BASE->YY_REG = tmp;
 * ...
 * \endcode
 */
#define REG_CLR_FIELD(base, reg, field, var) \
        var &= ~(base ## _ ## reg ## _ ## field ## _Msk)

/**
 * \brief Get the address of a register value by index (provided a register interval)
 *
 * \note The register interval should be an exact multiple of the register's base size. For example,
 * if the register size is 32-bit, then the interval should be 0x4, 0x8, etc. Otherwise, the result
 * will be undefined. The interval value must be in bytes. The index value (0,1,2...) is multiplied by
 * the interval value (in bytes) to find the actual offset of the register.
 *
 * Returns a register address value by index
 */
#define REG_GET_ADDR_INDEXED(base, reg, interval, index) \
        ((&base->reg) + (((intptr_t) index) * ((interval) / sizeof(base->reg))))

/**
 * \brief Return the value of a register field by index (provided a register interval).
 *
 * e.g.
 * \code
 * uint32_t val;
 * uint16_t index = 2
 *
 * val = REG_GETF_INDEXED(XX_BASE, YY_REG, FIELD_Z, 0x10, index)
 *
 * ...
 * \endcode
 *
 * \note The register interval should be an exact multiple of the register's base size. For example,
 * if the register size is 32-bit, then the interval should be 0x4, 0x8, etc. Otherwise, the result
 * will be undefined. The interval value must be in bytes. The index value (0,1,2...) is multiplied by
 * the interval value (in bytes) to find the actual offset of the register.
 *
 */
#define REG_GETF_INDEXED(base, reg, field, interval, index) \
        (((*REG_GET_ADDR_INDEXED(base, reg, interval, index)) & \
            (base ## _ ## reg ## _ ## field ## _Msk)) >> (base ## _ ## reg ## _ ## field ## _Pos))

/**
 * \brief Return the value of a register field.
 *
 * e.g.
 * \code
 * uint32_t val;
 *
 * val = REG_GETF(XX_BASE, YY_REG, FIELD_Z);
 * ...
 * \endcode
 */
#define REG_GETF(base, reg, field) \
        (((base->reg) & (base##_##reg##_##field##_Msk)) >> (base##_##reg##_##field##_Pos))

/**
 * \brief Return the value of a memory field using a mask.
 *
 * e.g.
 * \code
 * uint32_t val;
 *
 * val = RAW_GETF(0x50000000, 0x1UL);
 * ...
 * \endcode
 */
#define RAW_GETF(addr, mask) \
        ((*(volatile uint32_t *)(addr) & (mask)) >> (__builtin_ctz(mask)))

/**
 * \brief Set the value of a register field.
 *
 * e.g.
 * \code
 *
 * REG_SETF(XX_BASE, YY_REG, FIELD_Z, new_value);
 * ...
 * \endcode
 */
#define REG_SETF(base, reg, field, new_val) \
                base->reg = ((base->reg & ~(base##_##reg##_##field##_Msk)) | \
                ((base##_##reg##_##field##_Msk) & ((new_val) << (base##_##reg##_##field##_Pos))))

/**
 * \brief Set the value of a memory field.
 *
 * e.g.
 * \code
 *
 * RAW_SETF(0x50000000, 0x1UL, 1);
 * ...
 * \endcode
 */
#define RAW_SETF(addr, mask, val) \
                (*(volatile uint32_t *)(addr)) = ((*(volatile uint32_t *)(addr) & ~(mask)) | \
                ((mask) & ((val) << __builtin_ctz(mask))))

/**
 * \brief Set a bit of a register.
 *
 * e.g.
 * \code
 *
 * REG_SET_BIT(XX_BASE, YY_REG, FIELD_Z);
 * ...
 * \endcode
 */
#define REG_SET_BIT(base, reg, field) \
        do { \
                base->reg |= (1 << (base##_##reg##_##field##_Pos)); \
         } while (0)

/**
 * \brief Clear a bit of a register.
 *
 * e.g.
 * \code
 *
 * REG_CLR_BIT(XX_BASE, YY_REG, FIELD_Z);
 * ...
 * \endcode
 */
#define REG_CLR_BIT(base, reg, field) \
        do { \
                base->reg &= ~(base##_##reg##_##field##_Msk); \
         } while (0)

/**
 * \brief Sets register bits, indicated by the mask, to a value.
 *
 * e.g.
 * \code
 * REG_SET_MASKED(XX_BASE, YY_REG, 0xFF00, 0x1818);
 * \endcode
 */
#define REG_SET_MASKED(base, reg, mask, value) \
        do { \
                base->reg = (base->reg & ~(mask)) | ((value) & (mask)); \
        } while (0)

/**
 * \brief Sets memory bits, indicated by the mask, to a value.
 *
 * e.g.
 * \code
 * RAW_SET_MASKED(0x50000000, 0xFF00, 0x1818);
 * \endcode
 */
#define RAW_SET_MASKED(addr, mask, value) \
        do { \
                (*(volatile uint32_t *)(addr)) = ((*(volatile uint32_t *)(addr)) & ~(mask)) | ((value) & (mask)); \
        } while (0)

/**
 * \brief Sets 16-bit wide register bits, indicated by the field, to a value v.
 */
#define BITS16(base, reg, field, v) \
        ((uint16_t) (((uint16_t) (v) << (base ## _ ## reg ## _ ## field ## _Pos)) & \
                (base ## _ ## reg ## _ ## field ## _Msk)))

/**
 * \brief Sets 32-bit wide register bits, indicated by the field, to a value v.
 */
#define BITS32(base, reg, field, v) \
        ((uint32_t) (((uint32_t) (v) << (base ## _ ## reg ## _ ## field ## _Pos)) & \
                (base ## _ ## reg ## _ ## field ## _Msk)))

/**
 * \brief Reads 16-bit wide register bits, indicated by the field, to a variable v.
 */
#define GETBITS16(base, reg, v, field) \
        ((uint16_t) (((uint16_t) (v)) & (base ## _ ## reg ## _ ## field ## _Msk)) >> \
                (base ## _ ## reg ## _ ## field ## _Pos))

/**
 * \brief Reads 32-bit wide register bits, indicated by the field, to a variable v.
 */
#define GETBITS32(base, reg, v, field) \
        ((uint32_t) (((uint32_t) (v)) & (base ## _ ## reg ## _ ## field ## _Msk)) >> \
                (base ## _ ## reg ## _ ## field ## _Pos))

/**
 * \brief Macro to enable the debugger
 *
 */
#define ENABLE_DEBUGGER \
        do { \
                REG_SET_BIT(CRG_TOP, SYS_CTRL_REG, DEBUGGER_ENABLE); \
        } while (0)

/**
 * \brief Macro to enable the CMAC debugger
 *
 */
#define ENABLE_CMAC_DEBUGGER \
        do { \
                CRG_TOP->SYS_CTRL_REG |= (1 << (6UL)); \
        } while (0)

/**
 * \brief Macro to disable the debugger
 *
 */
#define DISABLE_DEBUGGER \
        do { \
                REG_CLR_BIT(CRG_TOP, SYS_CTRL_REG, DEBUGGER_ENABLE); \
        } while (0)

/**
 * \brief Macro to disable the CMAC debugger
 *
 */
#define DISABLE_CMAC_DEBUGGER \
        do { \
                CRG_TOP->SYS_CTRL_REG &= ~(0x40UL); \
        } while (0)

/**
 * \brief Macro to cause a software reset
 *
 */
#define SWRESET \
        do { \
                REG_SET_BIT(CRG_TOP, SYS_CTRL_REG, SW_RESET); \
        } while (0)

#define BIT0        0x01
#define BIT1        0x02
#define BIT2        0x04
#define BIT3        0x08
#define BIT4        0x10
#define BIT5        0x20
#define BIT6        0x40
#define BIT7        0x80

#define BIT8      0x0100
#define BIT9      0x0200
#define BIT10     0x0400
#define BIT11     0x0800
#define BIT12     0x1000
#define BIT13     0x2000
#define BIT14     0x4000
#define BIT15     0x8000

#define BIT16 0x00010000
#define BIT17 0x00020000
#define BIT18 0x00040000
#define BIT19 0x00080000
#define BIT20 0x00100000
#define BIT21 0x00200000
#define BIT22 0x00400000
#define BIT23 0x00800000

#define BIT24 0x01000000
#define BIT25 0x02000000
#define BIT26 0x04000000
#define BIT27 0x08000000
#define BIT28 0x10000000
#define BIT29 0x20000000
#define BIT30 0x40000000
#define BIT31 0x80000000

typedef unsigned char      uint8;   //  8 bits
typedef char               int8;    //  8 bits
typedef unsigned short     uint16;  // 16 bits
typedef short              int16;   // 16 bits
typedef unsigned long      uint32;  // 32 bits
typedef long               int32;   // 32 bits
typedef unsigned long long uint64;  // 64 bits
typedef long long          int64;   // 64 bits

/* See also "Data Types" on pag. 21 of the (Doulos) Cortex-M0 / SoC 1.0 training documentation. */
typedef unsigned char      BYTE;     //  8 bits = Byte
typedef unsigned short     HWORD;    // 16 bits = Halfword
typedef unsigned long      WORD;     // 32 bits = Word
typedef long long          DWORD;    // 64 bits = Doubleword

/**
 * \brief Macro to get the number of elements in an array
 *
 * \param[in] array     The array
 */
#define ARRAY_LENGTH(array) (sizeof((array))/sizeof((array)[0]))

/**
 * \brief Macro to convert time in nsec to clock cycles
 *
 * \param[in] nsec              Time in nsec
 * \param[in] clk_freq_hz       Clock Frequency in Hz
 */
#define NSEC_TO_CLK_CYCLES(nsec, clk_freq_hz)   ((((nsec) * ((clk_freq_hz) / 10000)) + 99999) / 100000)


#ifdef __cplusplus
}
#endif

#endif  /* __SDK_DEFS_H__ */

/**
 * \}
 * \}
 */
