/**
 * \addtogroup PLA_DRI_MEMORY
 * \{
 * \addtogroup HW_CACHE iCache Controller
 * \{
 * \brief iCache Controller DA1459x specific LLD API
 */

/**
 *****************************************************************************************
 *
 * @file hw_cache_da1459x.h
 *
 * @brief Definition of DA1459x specific API for the iCache Controller Low Level Driver.
 *
 * Copyright (C) 2020 Renesas Electronics Corporation and/or its affiliates.
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
 *****************************************************************************************
 */

#ifndef HW_CACHE_DA1459X_H_
#define HW_CACHE_DA1459X_H_


#include <sdk_defs.h>

/*
 * ENUMERATION DEFINITIONS
 *****************************************************************************************
 */

/**
 * \brief Cacheable eflash Region Sizes as defined in DA1459x datasheet
 *
 */
typedef enum {
        HW_CACHE_EFLASH_REGION_SZ_512KB = 4,
        HW_CACHE_EFLASH_REGION_SZ_256KB = 5,
        HW_CACHE_EFLASH_REGION_SZ_128KB = 6,
        HW_CACHE_EFLASH_REGION_SZ_64KB = 7,
        HW_CACHE_EFLASH_REGION_SZ_INVALID,              /* Used as iteration terminator */
} HW_CACHE_EFLASH_REGION_SZ;

typedef uint16_t eflash_region_base_t;
typedef uint16_t eflash_region_offset_t;

#define HW_CACHE_EFLASH_DEFAULT_REGION_BASE     0x00A0
#define HW_CACHE_EFLASH_MAX_REGION_BASE         0xFF
#define HW_CACHE_EFLASH_DEFAULT_REGION_OFFSET   0x0
#define HW_CACHE_EFLASH_MAX_REGION_OFFSET       0xFFF
#define HW_CACHE_EFLASH_DEFAULT_REGION_SZ       HW_CACHE_EFLASH_REGION_SZ_128KB
#define HW_CACHE_EFLASH_MIN_REGION_SZ           HW_CACHE_EFLASH_REGION_SZ_64KB
#define HW_CACHE_EFLASH_MAX_REGION_SZ           HW_CACHE_EFLASH_REGION_SZ_512KB

/**
 * \brief Cacheable flash Region Sizes as defined in DA1459x datasheet
 *
 */
typedef enum {
        HW_CACHE_FLASH_REGION_SZ_32MB = 0,
        HW_CACHE_FLASH_REGION_SZ_16MB = 1,
        HW_CACHE_FLASH_REGION_SZ_8MB = 2,
        HW_CACHE_FLASH_REGION_SZ_4MB = 3,
        HW_CACHE_FLASH_REGION_SZ_2MB = 4,
        HW_CACHE_FLASH_REGION_SZ_1MB = 5,
        HW_CACHE_FLASH_REGION_SZ_512KB = 6,
        HW_CACHE_FLASH_REGION_SZ_256KB = 7,
        HW_CACHE_FLASH_REGION_SZ_INVALID,              /* Used as iteration terminator */
} HW_CACHE_FLASH_REGION_SZ;

typedef uint16_t flash_region_base_t;
typedef uint16_t flash_region_offset_t;

#define HW_CACHE_FLASH_DEFAULT_REGION_BASE     0x1600
#define HW_CACHE_FLASH_MAX_REGION_BASE         0x17fc
#define HW_CACHE_FLASH_DEFAULT_REGION_OFFSET   0x0
#define HW_CACHE_FLASH_MAX_REGION_OFFSET       0xFFF
#define HW_CACHE_FLASH_DEFAULT_REGION_SZ       HW_CACHE_FLASH_REGION_SZ_512KB
#define HW_CACHE_FLASH_MIN_REGION_SZ           HW_CACHE_FLASH_REGION_SZ_256KB
#define HW_CACHE_FLASH_MAX_REGION_SZ           HW_CACHE_FLASH_REGION_SZ_32MB

/*
 * FAMILY SPECIFIC GENERIC FUNCTIONALITY DEFINITIONS
 *****************************************************************************************
 */

/**
 * \brief Enables the iCache Controller
 *
 * The iCache Controller is enabled by setting the CACHERAM_MUX to '1'. This action enables
 * the corresponding HW block, letting the RAM memory of the block be visible only to the
 * iCache Controller for caching purposes.
 *
 */
__STATIC_INLINE void hw_cache_enable()
{
        REG_SET_BIT(CRG_TOP, SYS_CTRL_REG, CACHERAM_MUX);
        /* Wait until the CACHERAM_MUX=1 (because of the APB Bridge). */
        while (REG_GETF(CRG_TOP, SYS_CTRL_REG, CACHERAM_MUX) != 1) {}
}

/**
 * \brief Disables the iCache Controller
 *
 * The iCache Controller is disabled by setting the CACHERAM_MUX to '0'. This action disables
 * the corresponding HW block, bypassing the iCache Controller for all read requests
 * and letting the RAM memory of the block be visible in the entire memory space.
 *
 */
__STATIC_INLINE void hw_cache_disable()
{
        REG_CLR_BIT(CRG_TOP, SYS_CTRL_REG, CACHERAM_MUX);
        /* Wait until the CACHERAM_MUX=0 (because of the APB Bridge). */
        while (REG_GETF(CRG_TOP, SYS_CTRL_REG, CACHERAM_MUX) != 0) {}
}

/**
 * \brief Checks if the iCache Controller is enabled
 *
 * \return True if the iCache Controller is enabled, False otherwise.
 *
 *
 */
__STATIC_INLINE bool hw_cache_is_enabled()
{
        return REG_GETF(CRG_TOP, SYS_CTRL_REG, CACHERAM_MUX);
}

/**
 * \brief Set the external (QSPI) flash cacheable memory length
 *
 * \param [in] len The QSPI flash cacheable memory length, in 64KB blocks. The actual
 *                 cacheable memory length will therefore be len * 64KB.
 *                 Valid values: [0, 511].
 *                 A value of 0 sets the iCache Controller in bypass mode for the read
 *                 requests targeting the cacheable QSPI flash memory area. Any value greater
 *                 than zero will set it in caching mode.
 */
__STATIC_INLINE void hw_cache_set_extflash_cacheable_len(uint32_t len)
{
        ASSERT_WARNING((len & ~CACHE_CACHE_CTRL2_REG_CACHE_LEN_Msk) == 0);
        REG_SETF(CACHE, CACHE_CTRL2_REG, CACHE_LEN, len);
}

/**
 * \brief Get the external (QSPI) flash cacheable memory length
 *
 * \return The eflash cacheable memory length, in 64KB blocks. The actual cacheable
 *         memory length will therefore be len * 64KB.
 */
__STATIC_INLINE int hw_cache_get_extflash_cacheable_len(void)
{
        return REG_GETF(CACHE, CACHE_CTRL2_REG, CACHE_LEN);
}


/**
 * \brief Set the eflash cacheable memory length
 *
 * \param [in] len The eflash cacheable memory length, in 64KB blocks. The actual cacheable
 *                 memory length will therefore be len * 64KB.
 *                 Valid values: [0, 511].
 *                 A value of 0 sets the iCache Controller in bypass mode for the read
 *                 requests targeting the cacheable eflash memory area. Any value greater
 *                 than zero will set it in caching mode.
 */
__STATIC_INLINE void hw_cache_set_eflash_cacheable_len(uint32_t len)
{
        ASSERT_WARNING((len & ~(CACHE_CACHE_CTRL2_REG_CACHE_EF_LEN_Msk >>
                CACHE_CACHE_CTRL2_REG_CACHE_EF_LEN_Pos)) == 0);
        REG_SETF(CACHE, CACHE_CTRL2_REG, CACHE_EF_LEN, len);
}

/**
 * \brief Get the eflash cacheable memory length
 *
 * \return The eflash cacheable memory length, in 64KB blocks. The actual cacheable
 *         memory length will therefore be len * 64KB.
 */
__STATIC_INLINE int hw_cache_get_eflash_cacheable_len(void)
{
        return REG_GETF(CACHE, CACHE_CTRL2_REG, CACHE_EF_LEN);
}

/**
 * \brief Set the cacheable memory length. Backwards compatibility wrapper.
 *
 * \param [in] len See hw_cache_set_eflash_cacheable_len for details.
 *
 * \deprecated API no longer supported, use hw_cache_set_eflash_cacheable_len.
 */
DEPRECATED_MSG("API no longer supported, use hw_cache_set_eflash_cacheable_len.")
__STATIC_INLINE void hw_cache_set_len(uint32_t len)
{
        hw_cache_set_eflash_cacheable_len(len);
}

/**
 * \brief Get the cacheable memory length. Backwards compatibility wrapper.
 *
 * \return See hw_cache_get_eflash_cacheable_len for details.
 * \deprecated API no longer supported, use hw_cache_get_eflash_cacheable_len.
 */
DEPRECATED_MSG("API no longer supported, use hw_cache_get_eflash_cacheable_len.")
__STATIC_INLINE int hw_cache_get_len(void)
{
        return hw_cache_get_eflash_cacheable_len();
}

/**
 * \brief Enable flushing the iCache Controller (cache RAM cells) contents. For debugging only.
 */
__STATIC_INLINE void hw_cache_enable_flushing(void)
{
        REG_CLR_BIT(CACHE, CACHE_CTRL2_REG, CACHE_FLUSH_DISABLE);
}

/**
 * \brief Disable flushing the iCache Controller (cache RAM cells) contents. For debugging only.
 */
__STATIC_INLINE void hw_cache_disable_flushing(void)
{
        REG_SET_BIT(CACHE, CACHE_CTRL2_REG, CACHE_FLUSH_DISABLE);
}

/**
 * \brief Checks if the iCache Controller flushing is disabled. For debugging only.
 *
 * \return True if the iCache Controller flushing is disabled, False otherwise.
 *
 *
 */
__STATIC_INLINE bool hw_cache_is_flushing_disabled()
{
        return REG_GETF(CACHE, CACHE_CTRL2_REG, CACHE_FLUSH_DISABLE);
}

/**
 * \brief Flush the cache contents
 *
 * Note: The very first flushing occurred after power on reset when the iCache Controller
 * is enabled for the first time by the booter.
 */
__STATIC_INLINE void hw_cache_flush(void)
{
        if (!hw_cache_is_flushing_disabled()) {
                GLOBAL_INT_DISABLE();
                hw_cache_disable();
                hw_cache_enable();
                GLOBAL_INT_RESTORE();
        }
}

/*
 * CACHEABLE EFLASH RELATED FUNCTIONALITY DEFINITIONS
 *****************************************************************************************
 */

/**
 * \brief Set the eflash region base
 *
 * \param [in] base The eflash region base corresponds to the eflash address bits
 *             [31:16]. Default value is '0x00A0'. Bits [31:24] are fixed to '0x00'.
 *             Max value is thus 0x00FF and min 0x0000.
 *
 * \note This should be aligned to the region size value (hw_cache_eflash_set_region_size()).
 * \note The updated value takes effect only after a software reset.
 */
__STATIC_INLINE void hw_cache_eflash_set_region_base(eflash_region_base_t base)
{
        ASSERT_WARNING(base <= HW_CACHE_EFLASH_MAX_REGION_BASE);
        REG_SETF(CACHE, CACHE_EFLASH_REG, EFLASH_REGION_BASE, base);
}

/**
 * \brief Get the eflash region base
 *
 * \return The eflash region base to use with the cache
 */
__STATIC_INLINE eflash_region_base_t hw_cache_eflash_get_region_base(void)
{
        return REG_GETF(CACHE, CACHE_EFLASH_REG, EFLASH_REGION_BASE);
}

/**
 * \brief Set the eflash region offset
 *
 * This value (expressed in words) is added to eflash region base
 * (see hw_cache_eflash_set/get_region_base()) to calculate the
 * the starting address within the eflash memory area that will be cacheable
 * (remapped to 0x0) and XiPed.
 *
 * \param [in] offset eflash region offset in 32-bit words. Max: 0xFFF
 *              since the corresponding register bit field area is 3 nibbles
 *              in length.
 *
 * \note The updated value takes effect only after a software reset.
 */
__STATIC_INLINE void hw_cache_eflash_set_region_offset(eflash_region_offset_t offset)
{
        ASSERT_WARNING(offset <= HW_CACHE_EFLASH_MAX_REGION_OFFSET);
        REG_SETF(CACHE, CACHE_EFLASH_REG, EFLASH_REGION_OFFSET, offset);
}

/**
 * \brief Get the eflash region offset
 *
 * \return The region offset to be used in conjunction with the region base to indicate
 * the starting address within the eflash memory area that will be cacheable
 */
__STATIC_INLINE eflash_region_offset_t hw_cache_eflash_get_region_offset(void)
{
        return REG_GETF(CACHE, CACHE_EFLASH_REG, EFLASH_REGION_OFFSET);
}

/**
 * \brief Set the eflash region size
 *
 * \param [in] sz The eflash region size to use with the cache
 *
 * This is the size of the eflash memory that will be cached. The size starts from eflash
 * region base (see hw_cache_eflash_set_region_base()) plus eflash region offset
 * (see hw_cache_eflash_set_region_offset()).
 *
 * \note The updated value takes effect only after a software reset.
 */
__STATIC_INLINE void hw_cache_eflash_set_region_size(HW_CACHE_EFLASH_REGION_SZ sz)
{
        ASSERT_WARNING(sz >= HW_CACHE_EFLASH_MAX_REGION_SZ && sz <= HW_CACHE_EFLASH_MIN_REGION_SZ);
        REG_SETF(CACHE, CACHE_EFLASH_REG, EFLASH_REGION_SIZE, sz);
}

/**
 * \brief Get the eflash region size
 *
 * \return The eflash region size to use with the cache
 */
__STATIC_INLINE HW_CACHE_EFLASH_REGION_SZ hw_cache_eflash_get_region_size(void)
{
        return REG_GETF(CACHE, CACHE_EFLASH_REG, EFLASH_REGION_SIZE);
}

/**
 * \brief Configure the eflash memory region that will be cacheable
 *
 * This is an alternative API to hw_cache_eflash_set_region_base()/_size()/_offset(). It automatically
 * configures the entire eflash region in one call.
 *
 * See the relevant called functions for input parameter definition.
 *
 * \note The updated value takes effect only after a software reset.
 */
__STATIC_INLINE void hw_cache_eflash_configure_region(eflash_region_base_t base, eflash_region_offset_t offset,
        HW_CACHE_EFLASH_REGION_SZ sz)
{
        hw_cache_eflash_set_region_base(base);
        hw_cache_eflash_set_region_offset(offset);
        hw_cache_eflash_set_region_size(sz);
}

/*
 * CACHEABLE FLASH RELATED FUNCTIONALITY DEFINITIONS
 *****************************************************************************************
 */

/**
 * \brief Set the flash region base
 *
 * \param [in] base The Flash region base corresponds to the flash address bits
 *             [31:16]. Bits [31:25] should be fixed to '0xb' and bits [17:16]
 *             should be fixed to '0x0'. Therefore, valid values are from 0x1600 to
 *             0x17fc. This address should be 'size'-param aligned.
 *
 * \note This should be aligned to the region size value (hw_cache_flash_set_region_size()).
 * \note The updated value takes effect only after a software reset.
 */
__STATIC_INLINE void hw_cache_flash_set_region_base(flash_region_base_t base)
{
        ASSERT_WARNING((base & 0xfe03) == HW_CACHE_FLASH_DEFAULT_REGION_BASE);
        REG_SETF(CACHE, CACHE_FLASH_REG, FLASH_REGION_BASE, base);
}

/**
 * \brief Get the flash region base
 *
 * \return The flash region base to use with the cache
 */
__STATIC_INLINE flash_region_base_t hw_cache_flash_get_region_base(void)
{
        return REG_GETF(CACHE, CACHE_FLASH_REG, FLASH_REGION_BASE);
}

/**
 * \brief Set the flash region offset
 *
 * This value (expressed in words) is added to flash region base
 * (see hw_cache_flash_set/get_region_base()) to calculate the
 * the starting address within the flash memory area that will be cacheable
 * (remapped to 0x0) and XiPed.
 *
 * \param [in] offset flash region offset in 32-bit words. Max: 0xFFF
 *              since the corresponding register bit field area is 3 nibbles
 *              in length.
 *
 * \note The updated value takes effect only after a software reset.
 */
__STATIC_INLINE void hw_cache_flash_set_region_offset(flash_region_offset_t offset)
{
        ASSERT_WARNING(offset <= HW_CACHE_FLASH_MAX_REGION_OFFSET);
        REG_SETF(CACHE, CACHE_FLASH_REG, FLASH_REGION_OFFSET, offset);
}

/**
 * \brief Get the flash region offset
 *
 * \return The region offset to be used in conjunction with the region base to indicate
 * the starting address within the flash memory area that will be cacheable
 */
__STATIC_INLINE flash_region_offset_t hw_cache_flash_get_region_offset(void)
{
        return REG_GETF(CACHE, CACHE_FLASH_REG, FLASH_REGION_OFFSET);
}

/**
 * \brief Set the flash region size
 *
 * \param [in] sz The flash region size to use with the cache
 *
 * This is the size of the flash memory that will be cached. The size starts from flash
 * region base (see hw_cache_flash_set_region_base()) plus flash region offset
 * (see hw_cache_flash_set_region_offset()).
 *
 * \note The updated value takes effect only after a software reset.
 */
__STATIC_INLINE void hw_cache_flash_set_region_size(HW_CACHE_FLASH_REGION_SZ sz)
{
        ASSERT_WARNING(sz >= HW_CACHE_FLASH_MAX_REGION_SZ && sz <= HW_CACHE_FLASH_MIN_REGION_SZ);
        REG_SETF(CACHE, CACHE_FLASH_REG, FLASH_REGION_SIZE, sz);
}

/**
 * \brief Get the flash region size
 *
 * \return The flash region size to use with the cache
 */
__STATIC_INLINE HW_CACHE_FLASH_REGION_SZ hw_cache_flash_get_region_size(void)
{
        return REG_GETF(CACHE, CACHE_FLASH_REG, FLASH_REGION_SIZE);
}

/**
 * \brief Configure the flash memory region that will be cacheable
 *
 * This is an alternative API to hw_cache_flash_set_region_base()/_size()/_offset(). It automatically
 * configures the entire flash region in one call.
 *
 * See the relevant called functions for input parameter definition.
 *
 * \note The updated value takes effect only after a software reset.
 */
__STATIC_INLINE void hw_cache_flash_configure_region(flash_region_base_t base, flash_region_offset_t offset,
        HW_CACHE_FLASH_REGION_SZ sz)
{
        hw_cache_flash_set_region_base(base);
        hw_cache_flash_set_region_offset(offset);
        hw_cache_flash_set_region_size(sz);
}

/*
 * MRM RELATED FUNCTIONALITY DEFINITIONS
 *****************************************************************************************
 */
/**
 * \brief Get the cache MRM hits with 1 Wait State number
 *
 * \return The number of hits with 1 Wait State
 *
 */
__STATIC_INLINE uint32_t hw_cache_mrm_get_hits_with_one_wait_state(void)
{
        return CACHE->CACHE_MRM_HITS1WS_REG;
}

/**
 * \brief Set the cache MRM hits with 1 Wait State number
 *
 * This is primarily intended for clearing the register
 *
 * \param[in] hits The number of cache hits with 1 Wait State
 *
 */
__STATIC_INLINE void hw_cache_mrm_set_hits_with_one_wait_state(uint32_t hits)
{
        CACHE->CACHE_MRM_HITS1WS_REG = hits;
}


#endif /* HW_CACHE_DA1459X_H_ */

/**
 * \}
 * \}
 */
