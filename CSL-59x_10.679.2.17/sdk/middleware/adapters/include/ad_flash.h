/**
 * \addtogroup MID_SYS_ADAPTERS
 * \{
 * \addtogroup FLASH_ADAPTER Flash Adapter
 *
 * \brief Flash Adapter
 *
 * \note The Flash Adapter should only be used with virtual addresses. Never pass physical or zero-based address.
 *
 * \{
 */

/**
 ****************************************************************************************
 *
 * \file ad_flash.h
 *
 * \brief Flash adapter API
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

#ifndef AD_FLASH_H_
#define AD_FLASH_H_

#if dg_configFLASH_ADAPTER

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include "qspi_automode.h"
#include "eflash_automode.h"

/*
 * DEFINES
 ****************************************************************************************
 */
# if dg_configQSPI_AUTOMODE_ENABLE
#  define AD_FLASH_GET_SECTOR_SIZE(addr)        ((addr >= EFLASH_MEM1_VIRTUAL_BASE_ADDR &&                       \
                                                  addr < (EFLASH_MEM1_VIRTUAL_BASE_ADDR + MEMORY_EFLASH_SIZE)) ? \
                                                 EFLASH_SECTOR_SIZE : FLASH_SECTOR_SIZE)
#  define AD_FLASH_MAX_SECTOR_SIZE              (MAX(EFLASH_SECTOR_SIZE, FLASH_SECTOR_SIZE))
# else
#  define AD_FLASH_GET_SECTOR_SIZE(addr)        (EFLASH_SECTOR_SIZE)
#  define AD_FLASH_MAX_SECTOR_SIZE              (EFLASH_SECTOR_SIZE)
# endif /* dg_configQSPI_AUTOMODE_ENABLE */

/**
 * \brief Initialize the flash adapter.
 *
 * This function is called by the system on power-up to initialize the flash adapter.
 *
 * \warning It should NOT be called by the application, as it is automatically executed at system start-up.
 *
 */
void ad_flash_init(void);

/**
 * \brief Read data from flash memory
 *
 * This function reads a specified number of bytes from flash memory, starting from the virtual
 * address specified in the addr parameter. The contents of the read data will be copied to the
 * destination array pointed by the buf parameter.
 *
 * \param [in]  addr The virtual address in flash memory to read data from.
 * \param [out] buf  Pointer to the destination array in System RAM, where the read data will be copied.
 * \param [in]  len  The number of bytes to read.
 *
 * \returns The number of bytes read from flash memory.
 */
size_t ad_flash_read(uint32_t addr, uint8_t *buf, size_t len);

/**
 * \brief Write data to flash memory
 *
 * This function writes a specified number of bytes from a source data pointer to a virtual address
 * in flash memory. Before writing to the destination virtual address, the memory at that location
 * should be erased.
 *
 * \param[in] addr The virtual address in flash memory where the data will be written.
 * \param[in] buf  Pointer to the source data.
 * \param[in] size The number of bytes to write.
 *
 * \return The number of bytes written to flash memory.
 *
 * \warning The memory at the destination virtual address should be erased before writing to it.
 *
 * \note The source data pointer can also point to XiP (Execute-In-Place) mapped memory.
 */
size_t ad_flash_write(uint32_t addr, const uint8_t *buf, size_t size);

/**
 * \brief Erase a flash memory region
 *
 * This function erases all sectors from the virtual start address `addr` to `addr + size`.
 *
 * \param [in] addr The virtual start address of the flash region to be erased.
 * \param [in] size The number of bytes to erase.
 *
 * \return True, if the erase operation was successful.
 *
 * \warning  If `addr` is not sector-aligned, the preceding data of the sector where `addr` resides
 *           will also be erased.
 * \warning If `addr + size` is not sector aligned, the whole sector where the `addr + size` resides
 *          will also be erased.
 *
 */
bool ad_flash_erase_region(uint32_t addr, size_t size);

/**
 * \brief Translate a virtual address to a pointer that directly accesses the corresponding physical
 *        flash memory location.
 *
 * \param [in] addr The virtual address to translate.
 *
 * \return A pointer to the physical memory location corresponding to the given virtual address, or
 *         NULL if the virtual address is invalid.
 *
 */
__STATIC_INLINE const void *ad_flash_get_ptr(uint32_t addr)
{
        if (eflash_automode_is_valid_virtual_addr(addr)) {
                return eflash_automode_get_physical_addr(addr);
# if (dg_configQSPI_AUTOMODE_ENABLE == 1)
        } else if (qspi_automode_is_valid_virtual_addr(addr)) {
                return qspi_automode_get_physical_addr(addr);
# endif
        }

        return NULL;
}

/**
 * \brief Check if a flash update can be performed without erasing memory
 *
 * This function checks if an update to flash memory can be performed without erasing the memory first.
 * An update is possible only if only "0s" need to be written.
 *
 * \param[in] addr The virtual address in flash memory to be checked.
 * \param[in] data_to_write Pointer to the source data to be written.
 * \param[in] size The number of bytes to check.
 *
 * \return -1 if an erase is required. Otherwise, a non-negative value representing the number of
 *            bytes that do not need to be written (i.e., the data is already the same as the new
 *            data). If the return value is 0, then the write operation should start from offset 0,
 *            but no erase is needed.
 */
int ad_flash_update_possible(uint32_t addr, const uint8_t *data_to_write, size_t size);

/**
 * \brief Get minimum flash erasable size (sector size)
 *
 * This function returns the minimum size that can be erased in flash memory, which corresponds to
 * the sector size.
 *
 * \param [in] addr The virtual address in flash memory.
 *
 * \return The minimum erasable size in bytes (sector size).
 */
size_t ad_flash_erase_size(uint32_t addr);


/**
 * \brief Perform full erase of a specified flash memory.
 *
 * The flash memory to be erased is specified by providing the corresponding virtual start address.
 *
 * \param [in] addr The virtual start address of the memory to be erased.
 *
 * \return True, if the memory was successfully erased, false if the virtual start address is invalid.
 */
bool ad_flash_chip_erase_by_addr(uint32_t addr);

/**
 * \brief Lock the flash adapter to prevent multiple tasks from accessing the same flash memory
 *        simultaneously.
 *
 * This function gets the flash adapter for exclusive usage, ensuring that only one task can access
 * the flash memory at any given time.
 */
void ad_flash_lock(void);

/**
 * \brief Release lock on flash adapter
 *
 * Release exclusive access to the flash adapter that was obtained using \p `ad_flash_lock()`.
 * This allows other tasks to use the flash adapter.
 */
void ad_flash_unlock(void);

/**
 * \brief Special base address used to return to the default cache flushing mode.
 *
 * This constant can be used with the \p `ad_flash_skip_cache_flushing()` function to reset the
 * cache flushing behavior, so that all flash writes and erases will trigger cache flushing.
 * The constant has the value of the maximum 32-bit unsigned integer, which is used as a sentinel
 * value to indicate the special behavior.
 */
#define AD_FLASH_ALWAYS_FLUSH_CACHE     ((uint32_t) - 1)

/**
 * \brief Control cache flushing on modifications to a specified flash region.
 *
 * This function can be used to enable or disable the triggering of cache flushing when modifications
 * (writes or erases) occur in a specific flash region. Only one such flash region can be defined.
 *
 * This feature is useful when the programmer knows in advance that a large flash region is going to
 * be updated (e.g. during firmware update). However, flash reads from that region should be avoided,
 * as they might lead to cache incoherency.
 *
 * \param [in] base  Starting offset of the flash region that should not trigger a cache
 *                   flushing.
 *                   If \sa AD_FLASH_ALWAYS_FLUSH_CACHE is passed, selective cache flushing
 *                   is disabled, regardless of the value of size.
 * \param [in] size  The size of the flash region that should not trigger cache flushing.
 *
 * \warning The effect of this function is limited to ad_flash_* layer and higher. Direct use of
 *          lower level APIs (e.g. hw_qspi_*, qspi_automode_* etc) will not be affected by the use
 *          of this function.
 */
void ad_flash_skip_cache_flushing(uint32_t base, uint32_t size);

#endif /* dg_configFLASH_ADAPTER */

#endif /* AD_FLASH_H_ */
/**
 \}
 \}
 */
