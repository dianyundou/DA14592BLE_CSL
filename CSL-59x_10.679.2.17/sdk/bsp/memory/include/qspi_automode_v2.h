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
 * @file qspi_automode_v2.h
 *
 * @brief QSPI Memory Abstraction Interface
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

#ifndef QSPI_AUTOMODE_V2_H_
#define QSPI_AUTOMODE_V2_H_

#if dg_configQSPI_AUTOMODE_ENABLE

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <sdk_defs.h>
#include "hw_qspi.h"
#include "hw_clk.h"
#include "qspi_common.h"

/*
 * Defines (generic)
 */
#define QSPI_FLASH_PAGE_SIZE                   (0x100)
#define QSPI_FLASH_SECTOR_SIZE                 (FLASH_SECTOR_SIZE)

/**
 * \brief QSPI memory status
 *
 * During system startup, the JEDEC ID is read to determine the specific part number and configure
 * the QSPIC accordingly. If the JEDEC ID is equal to { 0 0 0 }, then the memory is recognized as
 * absent.
 *
 */
typedef enum {
        QSPI_AUTOMODE_MEMORY_STATUS_ABSENT               = 0,   /**< The memory is absent */
        QSPI_AUTOMODE_MEMORY_STATUS_PRESENT_UNIDENTIFIED = 1,   /**< The memory is present, however
                                                                     it has not been identified and
                                                                     thus the QSPIC could not be
                                                                     configured properly for it. */
        QSPI_AUTOMODE_MEMORY_STATUS_PRESENT_IDENTIFIED   = 2,   /**< The memory is present and has
                                                                     been identified. */
} QSPI_AUTOMODE_MEMORY_STATUS;

/**
 * \brief Write data to flash memory
 *
 * This function writes a specified number of bytes from a source data pointer to a virtual address
 * in flash memory. Before writing to the destination virtual address, the memory at that location
 * should be erased.
 *
 * \param [in] addr The virtual address in flash memory where the data will be written.
 * \param [in] buf  Pointer to the source data.
 * \param [in] size The number of bytes to write.
 *
 * return The number of bytes written to flash memory.
 *
 * \warning The memory at the destination virtual address should be erased before writing to it.
 *
 * \warning The maximum number of bytes to write is limited to the size of a flash memory page.
 *          If the requested number of bytes exceeds the page size, only the data that fit to the
 *          page will be written and the rest will be omitted. It's a caller's responsibility to
 *          check for omitted data and perform additional calls of the function to complete the
 *          write operation.
 *
 * \warning The source data pointer should NOT point to XiP (Execute-In-Place) mapped memory,
 *          since the function switches the QSPIC to manual access mode in order to perform the
 *          write operations. In this mode the memory mapped address area is not accessible, because
 *          it requires the QSPIC in auto access mode, and therefore the read access will fail.
 *
 */
__RETAINED_CODE uint32_t qspi_automode_write_flash_page(uint32_t addr, const uint8_t *buf, uint32_t size);

/**
 * \brief Erase a flash memory sector
 *
 * This function erases the flash sector in a virtual address in flash memory. The erase operation
 * will be performed either in auto or in manual access mode depending on
 * \p dgconfigQSPI_ERASE_IN_AUTOMODE.
 *
 * \param [in] addr The virtual address of the sector to be erased.
 *
 * \return True, if the erase operation was successful.
 *
 * \warning If `addr` is not sector-aligned, the preceding data of the sector where `addr` resides
 *          will also be erased.
 *
 * \sa dgconfigQSPI_ERASE_IN_AUTOMODE
 */
__RETAINED_CODE bool qspi_automode_erase_flash_sector(uint32_t addr);

/**
 * \brief Perform full erase of a specified flash memory.
 *
 * The flash memory to be erased is specified by providing the corresponding virtual start address.
 *
 * \param [in] virtual_base_addr The virtual start address of the memory to be erased.
 */
void qspi_automode_erase_chip(uint32_t virtual_base_addr);

/**
 * \brief Read data from memory
 *
 * This function reads a specified number of bytes from QSPI memory, starting from the virtual
 * address specified in the addr parameter. The contents of the read data will be copied to the
 * destination array pointed by the buf parameter.
 *
 * \param [in] addr The virtual address in memory to read data from.
 * \param [out] buf Pointer to the destination array in System RAM, where the read data will be copied.
 * \param [in] len  The number of bytes to read.
 *
 * \return The number of bytes read from flash memory.
 */
uint32_t qspi_automode_read(uint32_t addr, uint8_t *buf, uint32_t len);

/**
 * \brief Translate a virtual address to a pointer that directly accesses the corresponding physical
 *        flash memory location.
 *
 * \param [in] virtual_addr The virtual address to translate.
 *
 * \return A pointer to the physical memory location corresponding to the given virtual address,
 *         type-casted to a pointer of type void*.
 *
 * \warning Use qspi_automode_is_valid_virtual_addr() to check whether the provided virtual address
 *          is within the acceptable QSPI virtual address space. If an invalid virtual address is
 *          provided to this function, the returned physical address will be invalid as well. The
 *          function doesn't check against invalid input.
 */
const void *qspi_automode_get_physical_addr(uint32_t virtual_addr);

/**
  * \brief Wake up the memory(ies) connected to QSPI controller(s) from power down mode.
  */
__RETAINED_CODE void qspi_automode_flash_power_up(void);

/**
 * \brief Put the memory(ies) connected to QSPI controller(s) into power down mode
 */
__RETAINED_CODE void qspi_automode_flash_power_down(void);

/**
 * \brief Initialize the QSPI controller(s)
 *
 * \return true if the QSPIC initialization succeeds, otherwise false.
 */
__RETAINED_CODE bool qspi_automode_init(void);

/**
 * \brief Configure the QSPI controller and the connected memory for specified system clock frequency.
 *
 * This function is called by the System Clock Manager, every time the system clock frequency changes
 * to reconfigure the QSPI controller and connected memory for proper operation at the new frequency.
 *
 * \param [in] sys_clk The new system clock frequency in Hz.
 *
 */
__RETAINED_CODE void qspi_automode_sys_clock_cfg(sys_clk_t sys_clk);

/**
 * \brief Check if the virtual address range from `addr` to `addr + len` is valid and within
 *        the range of the connected memory.
 *
 * \param [in] addr The virtual start address to check.
 * \param [in] len  The length of bytes to access relative to the virtual start address,
 *                  which determines the virtual end address to check.
 *
 * \return True, if the input address range is valid and within the range of the connected memory,
 *         otherwise false.
 *
 * \warning This function requires virtual destination address. Do not provide physical or zero-based
 *          address.
 */
__RETAINED_CODE bool qspi_automode_is_valid_virtual_address_range(uint32_t addr, uint32_t len);

/**
 * \brief Check if the virtual address is valid and within the range of the connected memory.
 *
 * \param [in] addr The virtual address to check.
 *
 * \return True, if the input virtual address is valid and within the range of the connected memory,
 *         otherwise false.
 */
__RETAINED_CODE bool qspi_automode_is_valid_virtual_addr(uint32_t addr);

/**
 * \brief Get the size of the memory which is connected to the specified QSPI controller.
 *
 * \param [in] addr The virtual base address of the memory.
 *
 * \return The memory size in bytes connected to the specified QSPI controller.
 */
uint32_t qspi_automode_get_memory_size(uint32_t addr);

/**
 * \brief Read the JEDEC ID
 *
 * The function reads the JEDEC ID (manufacturer ID, type, density) of the memory that is connected
 * to the QSPI controller corresponding to the provided virtual address and returns the memory status.
 * There are three possible conditions:
 *
 * (a) The memory is absent.
 * (b) The memory is present but not identified.
 * (c) The memory is present and identified.
 *
 * If the JEDEC ID is {0 0 0}, it indicates that the memory is not present, and therefore, the pull
 * down resistors of the QSPIC IOs have pulled the level at ground. To prevent getting measurements
 * different than 0 when the memory is absent, the function waits for adequate time after sending
 * the read JEDEC ID opcode in order to allow the internal pull-down resistors to pull the level of
 * the QSPIC IOs at ground. It has been experimentally detected that 100 usec is a safe margin, even
 * when the parasitic capacitance of the IOs is extremely high.
 *
 * If the memory is present, the function will attempt to identify it. When the memory auto-detection
 * feature is enabled (dg_configQSPI_FLASH_AUTODETECT = 1), the function will compare the read JEDEC
 * ID with the JEDEC IDs of all supported memories listed in the qspi_memory_config_table[]. Otherwise,
 * will compare the read JEDEC ID with the JEDEC ID of the memory driver that the application is built
 * for. If there is a match, the memory will be considered as present and identified. If not, it will
 * be considered present but unidentified.
 *
 * \param [in]  addr  The virtual base address of the memory.
 * \param [out] jedec Pointer to a structure jedec_id_t where the JEDEC parameters will be saved.
 *
 * \return The status of the connected memory.
 *
 * \warning This function resets the connected QSPI flash memory to make sure that the memory
 *          is in single bus mode.
 *
 * \warning The maximum allowed QSPIC clock frequency of the Read JEDEC ID command may differ among
 *          various memories. Hence, if the initial attempt to read the JEDEC ID fails, this function
 *          will change the QSPIC clock divider to HW_QSPI_CLK_DIV_8 and try again. Nonetheless,
 *          there is no assurance that the second attempt will be successful. Therefore, the user
 *          must ensure that the QSPIC clock frequency is within the manufacturer's specified limits
 *          for the connected memory.
 *
 * \sa QSPI_AUTOMODE_MEMORY_STATUS
 *
 */
__RETAINED_CODE QSPI_AUTOMODE_MEMORY_STATUS qspi_automode_read_jedec_id(uint32_t addr, jedec_id_t *jedec);

/**
 * \brief Check that connected device is an external RAM
 *
 * \param [in] addr The virtual base address of the memory.
 *
 * \return True if the connected device is RAM, otherwise false.
 *
 */
__RETAINED_CODE bool qspi_automode_is_ram(uint32_t addr);


#endif /* dg_configQSPI_AUTOMODE_ENABLE */

#endif /* QSPI_AUTOMODE_V2_H_ */
/**
 * \}
 * \}
 */
