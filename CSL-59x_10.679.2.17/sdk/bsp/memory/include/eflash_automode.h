/**
 * \addtogroup PLA_BSP_SYSTEM
 * \{
 * \addtogroup PLA_MEMORY
 *
 * \{
 */

/**
 ****************************************************************************************
 *
 * @file eflash_automode.h
 *
 * @brief Access EFLASH device when running in auto mode
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

#ifndef EFLASH_AUTOMODE_H_
#define EFLASH_AUTOMODE_H_


#include "hw_fcu.h"

/*
 * EFLASH sector size
 */
#define EFLASH_SECTOR_SIZE      (HW_FCU_FLASH_PAGE_SIZE_IN_BYTES)

#include <sdk_defs.h>

/**
 * \brief Get the physical address of eFlash memory for the given virtual address
 *
 * \param [in] virtual_addr Virtual address
 *
 * \warning Use eflash_automode_is_valid_virtual_addr() to check whether the given virtual address
 *          is within the acceptable eFlash virtual address space area. If an invalid virtual address
 *          is passed to this function, the returned physical address will be invalid as well. The
 *          function doesn't check against invalid input.
 *
 * \return Physical address in CPU address space where data is located
 */
__RETAINED_CODE const void *eflash_automode_get_physical_addr(uint32_t virtual_addr);

/**
 * \brief Check if the virtual address range from `addr` to `addr` + `len` is valid.
 *
 * \warning This function requires virtual destination address. Do not provide physical or zero-based
 *          address.
 *
 * \param [in] addr     Virtual start address to be checked.
 * \param [in] len      Length of bytes to access relative to the virtual start address,
 *                      which determines the virtual end address to be checked.
 *
 * \return True, if the input address virtual range is valid, otherwise false.
 */
__RETAINED_CODE bool eflash_automode_is_valid_virtual_address_range(uint32_t addr, uint32_t len);

/**
 * \brief Verify if given virtual address points to EFLASH memory
 *
 * \param [in] addr Virtual address.
 *
 * \return true if virtual address points to EFLASH memory range, false otherwise
 */
__RETAINED_CODE bool eflash_automode_is_valid_virtual_addr(uint32_t addr);

/**
 * \brief Wakeup EFLASH
 *
 * Call this function prior to eflash_automode_read(), eflash_automode_write_page(),
 * eflash_automode_erase_sector() and eflash_automode_erase_chip().
 */
__ALWAYS_RETAINED_CODE void eflash_automode_wakeup(void);

/**
 * \brief Sef EFLASH to sleep mode
 *
 * This function put EFLASH into sleep mode.
 */
__ALWAYS_RETAINED_CODE void eflash_automode_sleep(void);

/**
 * \brief Read data from EFLASH memory
 *
 * \param [in] addr address pointing to EFLASH memory
 * \param [in] buf buffer to read data to
 * \param [in] len number of bytes to read
 *
 * \returns number of bytes read
 *
 * \sa eflash_automode_wakeup() eflash_automode_sleep()
 */
uint32_t eflash_automode_read(uint32_t addr, uint8_t *buf, uint32_t len);

/**
 * \brief Write data to EFLASH memory page
 *
 * This function writes data do EFLASH memory. Note that EFLASH hardware supports writes of
 * 32 bit words only, and if buffer or address is not word-aligned, flash content will be read,
 * overwritten and written back to flash. Therefore word-aligned memory area should be erased
 * before. Note that page boundary cannot be crossed and, as a result up to page size may be
 * written.
 *
 * \param [in] addr address pointing to EFLASH memory
 * \param [in] buf buffer to write data into
 * \param [in] len number of bytes to write
 *
 * \returns number of written bytes
 *
 * \sa eflash_automode_wakeup() eflash_automode_sleep()
 */
__ALWAYS_RETAINED_CODE uint32_t eflash_automode_write_page(uint32_t addr, const uint8_t *buf, uint32_t len);

/**
 * \brief Erase EFLASH sector
 *
 * Erase EFLASH sector. Note that size of EFLASH sector is 0x800 bytes.
 *
 * \param [in] addr sector address
 *
 * \returns true if sector was erased successfully, false otherwise
 *
 * \sa eflash_automode_wakeup() eflash_automode_sleep()
 */
__ALWAYS_RETAINED_CODE bool eflash_automode_erase_sector(uint32_t addr);

/**
 * \brief Erase chip
 *
 * \returns true if chip was erased successfully, false otherwise
 *
 * \note If info page protection is enabled, it won't be erased.
 *
 * \sa eflash_automode_wakeup() eflash_automode_sleep()
 */
__RETAINED_CODE bool eflash_automode_erase_chip(void);


#endif /* EFLASH_AUTOMODE_H_ */
/**
 * \}
 * \}
 */
