/**
****************************************************************************************
*
* @file sys_boot.c
*
* @brief System Boot Handler source file
*
* Copyright (C) 2022-2024 Renesas Electronics Corporation and/or its affiliates.
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

#if dg_configUSE_SYS_BOOT

#ifdef SYS_BOOT_EXPOSE_STATIC_FUNCTIONS
# define __STATIC__
#else
# define __STATIC__      static
#endif

#if defined(USE_PARTITION_TABLE_EFLASH_WITH_SUOTA) || \
    defined(USE_PARTITION_TABLE_1MB_WITH_SUOTA)    || \
    defined(USE_PARTITION_TABLE_4MB_WITH_SUOTA)
#define USE_PARTITION_TABLE_WITH_SUOTA
#endif

#include <string.h>
#include "hw_cache.h"
#include "sys_boot.h"
#if (dg_configCODE_LOCATION == NON_VOLATILE_IS_EMBEDDED_FLASH)
#include "eflash_automode.h"
#elif (dg_configCODE_LOCATION == NON_VOLATILE_IS_QSPI_FLASH)
#include "qspi_automode.h"
#endif
#include "sdk_crc16.h"

// This union typedef is used to handle the bytes of a uint16_t more efficiently
typedef union {
        uint16_t var;
        uint8_t  arr[2];
} uint16_union_t;


// This union typedef is used to handle the bytes of a uint32_t more efficiently
typedef union {
        uint32_t var;
        uint8_t  arr[4];
} uint32_union_t;

typedef __PACKED_STRUCT {
        uint8_t identifier[2];          /**< Identifier (Pp) */
        uint8_t fw_img_active[4];       /**< Active Firmware image address */
        uint8_t fw_img_update[4];       /**< Update Firmware image address (if available,
                                             otherwise equal to Active Firmware image address) */
        uint8_t flash_burstcmda[4];     /**< BURSTCMDA register */
        uint8_t flash_burstcmdb[4];     /**< BURSTCMDB register */
        uint8_t type_flash_conf[2];     /**< Type of Flash Configuration */
        uint8_t flash_conf_len[2];      /**< Length of flash configuration  */
} product_header_fixed_t;

#if (dg_configCODE_LOCATION == NON_VOLATILE_IS_QSPI_FLASH)
__STATIC__ uint16_t get_flash_conf_len(uint32_t product_header_addr)
{
        ASSERT_ERROR((product_header_addr == PRIMARY_PRODUCT_HEADER_BASE) ||
                     (product_header_addr == BACKUP_PRODUCT_HEADER_BASE));

        uint16_union_t flash_config_len;
        uint32_t flash_conf_len_addr = product_header_addr + offsetof(product_header_fixed_t, flash_conf_len);

        qspi_automode_read(flash_conf_len_addr, flash_config_len.arr, sizeof(flash_config_len.arr));

        return (flash_config_len.var);
}
#endif /* dg_configCODE_LOCATION */

__STATIC__ uint32_t get_product_header_len(uint16_t flash_conf_len)
{
        uint32_t ph_len = 0;

        /* Product Header format: Fixed Part + Variable Flash Configuration Part + CRC */
        ph_len = sizeof(product_header_fixed_t) + flash_conf_len + sizeof(uint16_t);

        return (ph_len);
}

__RETAINED_CODE __STATIC__ void restore_product_header(uint32_t src, uint32_t dst, uint32_t len)
{
        uint8_t ph[len];
        size_t offset = 0;

#if (dg_configCODE_LOCATION == NON_VOLATILE_IS_EMBEDDED_FLASH)
        eflash_automode_erase_sector(dst);
        eflash_automode_read(src, ph, len);

        while (offset < len) {
                offset += eflash_automode_write_page((dst + offset), ((const uint8_t *) ph + offset),
                                                     (len - offset));
        }
#elif (dg_configCODE_LOCATION == NON_VOLATILE_IS_QSPI_FLASH)
        qspi_automode_erase_flash_sector(dst);
        qspi_automode_read(src, ph, len);

        while (offset < len) {
                offset += qspi_automode_write_flash_page((dst + offset), ((const uint8_t *) ph + offset),
                                                         (len - offset));
        }
#endif
}

__STATIC__ uint16_t crc16_read(uint32_t addr, uint16_t crc_offset)
{
        ASSERT_ERROR((addr != PRIMARY_PRODUCT_HEADER_BASE) || (addr != BACKUP_PRODUCT_HEADER_BASE));

        uint16_union_t ph_crc;

#if (dg_configCODE_LOCATION == NON_VOLATILE_IS_EMBEDDED_FLASH)
        eflash_automode_read((addr + crc_offset), &ph_crc.arr[0], 1);
        eflash_automode_read((addr + crc_offset + 1), &ph_crc.arr[1], 1);
#elif (dg_configCODE_LOCATION == NON_VOLATILE_IS_QSPI_FLASH)
        qspi_automode_read((addr + crc_offset), &ph_crc.arr[0], 1);
        qspi_automode_read((addr + crc_offset + 1), &ph_crc.arr[1], 1);
#endif
        return ph_crc.var;
}

__STATIC__ uint16_t crc16_calc(uint32_t addr, uint16_t crc_offset)
{
        uint8_t buf[crc_offset];
        uint16_t crc;

#if (dg_configCODE_LOCATION == NON_VOLATILE_IS_EMBEDDED_FLASH)
        eflash_automode_read(addr, buf, crc_offset);
#elif (dg_configCODE_LOCATION == NON_VOLATILE_IS_QSPI_FLASH)
        qspi_automode_read(addr, buf, crc_offset);
#endif
        crc = crc16_calculate(buf, crc_offset);

        return crc;
}

#ifdef USE_PARTITION_TABLE_WITH_SUOTA
static uint32_t get_ivt_addr(void)
{
        uint32_t flash_base = 0;
        uint32_t flash_offset = 0;
        uint32_t ivt_address = 0;

#if (dg_configCODE_LOCATION == NON_VOLATILE_IS_EMBEDDED_FLASH)
        flash_base = hw_cache_eflash_get_region_base();
        flash_offset = hw_cache_eflash_get_region_offset();
        ivt_address = ((flash_base << 16) + (flash_offset << 2)) - MEMORY_EFLASH_BASE;
#elif (dg_configCODE_LOCATION == NON_VOLATILE_IS_QSPI_FLASH)
        flash_base = hw_cache_flash_get_region_base();
        flash_offset = hw_cache_flash_get_region_offset();
        ivt_address = ((flash_base << 16) + (flash_offset << 2)) - MEMORY_QSPIF_BASE;
#endif

        return ivt_address;
}
#endif

/*
 * Detect the address of the Executed FW image by comparing the IVT address with
 * NVMS_FW_EXEC_PART_START and NVMS_FW_UPDATE_PART_START.
 */
__STATIC__ uint32_t detect_executed_fw_img_addr(void)
{
#ifdef USE_PARTITION_TABLE_WITH_SUOTA
        uint32_t ivt_addr = get_ivt_addr();

#if (dg_configCODE_LOCATION == NON_VOLATILE_IS_EMBEDDED_FLASH)
        const uint32_t base_offset = EFLASH_MEM1_VIRTUAL_BASE_ADDR;
#elif (dg_configCODE_LOCATION == NON_VOLATILE_IS_QSPI_FLASH)
        const uint32_t base_offset = QSPI_MEM1_VIRTUAL_BASE_ADDR;
#endif

        ivt_addr += base_offset;

#ifdef NVMS_FW_EXEC_PART_START
        if (WITHIN_RANGE(ivt_addr, NVMS_FW_EXEC_PART_START, NVMS_FW_EXEC_PART_START + NVMS_FW_EXEC_PART_SIZE)) {
                return (NVMS_FW_EXEC_PART_START - base_offset);
        }
#endif
#ifdef NVMS_FW_UPDATE_PART_START
        if (WITHIN_RANGE(ivt_addr, NVMS_FW_UPDATE_PART_START, NVMS_FW_UPDATE_PART_START + NVMS_FW_UPDATE_PART_SIZE)) {
                return (NVMS_FW_UPDATE_PART_START - base_offset);
        }
#endif
#ifdef NVMS_FIRMWARE_PART_START
        if (WITHIN_RANGE(ivt_addr, NVMS_FIRMWARE_PART_START, NVMS_FIRMWARE_PART_START + NVMS_FIRMWARE_PART_SIZE)) {
                return (NVMS_FIRMWARE_PART_START - base_offset);
        }
#endif

        ASSERT_WARNING(0);
        return (0);
#else
        return NVMS_FIRMWARE_PART_START;
#endif
}

__STATIC__ bool equalize_image_pointers(uint32_t ph_len)
{
        uint32_t active = offsetof(product_header_fixed_t, fw_img_active);
        uint32_t update = offsetof(product_header_fixed_t, fw_img_update);
        uint8_t ph[ph_len];
        uint16_union_t crc;

#if (dg_configCODE_LOCATION == NON_VOLATILE_IS_EMBEDDED_FLASH)
        eflash_automode_read(PRIMARY_PRODUCT_HEADER_BASE, ph, ph_len);
#elif (dg_configCODE_LOCATION == NON_VOLATILE_IS_QSPI_FLASH)
        qspi_automode_read(PRIMARY_PRODUCT_HEADER_BASE, ph, ph_len);
#endif

        // If the Active Image Address is not equal to Update Image Address
        if (memcmp((const uint8_t *) (ph + active), (const uint8_t *) (ph + update), 4)) {
                uint32_union_t exec_fw_addr;

                exec_fw_addr.var = detect_executed_fw_img_addr();
                // Set both the active and the update image pointer equal to the executed FW address.
                memcpy((uint8_t *) (ph + active), (const uint8_t *) exec_fw_addr.arr, sizeof(exec_fw_addr));
                memcpy((uint8_t *) (ph + update), (const uint8_t *) exec_fw_addr.arr, sizeof(exec_fw_addr));

                // Re-calculate the CRC of the new product header
                crc.var = crc16_calculate(ph, (ph_len - sizeof(crc)));
                ph[ph_len - 2] = crc.arr[0];
                ph[ph_len - 1] = crc.arr[1];

                // Update both Primary and Backup product header with ph[]
#if (dg_configCODE_LOCATION == NON_VOLATILE_IS_EMBEDDED_FLASH)
                eflash_automode_erase_sector(PRIMARY_PRODUCT_HEADER_BASE);
                eflash_automode_write_page(PRIMARY_PRODUCT_HEADER_BASE, ph, ph_len);

                eflash_automode_erase_sector(BACKUP_PRODUCT_HEADER_BASE);
                eflash_automode_write_page(BACKUP_PRODUCT_HEADER_BASE, ph, ph_len);
#elif (dg_configCODE_LOCATION == NON_VOLATILE_IS_QSPI_FLASH)
                qspi_automode_erase_flash_sector(PRIMARY_PRODUCT_HEADER_BASE);
                qspi_automode_write_flash_page(PRIMARY_PRODUCT_HEADER_BASE, ph, ph_len);

                qspi_automode_erase_flash_sector(BACKUP_PRODUCT_HEADER_BASE);
                qspi_automode_write_flash_page(BACKUP_PRODUCT_HEADER_BASE, ph, ph_len);
#endif
        }

        return true;
}


void sys_boot_restore_product_headers(void)
{
        bool pph_repaired = false;
        uint16_t crc_offset;
        uint32_t ph_len;

        uint16_t pph_crc = 0xFFFF;
        uint16_t pph_crc_calc = 0xF5F5;

#if (dg_configCODE_LOCATION == NON_VOLATILE_IS_EMBEDDED_FLASH)
        crc_offset = sizeof(product_header_fixed_t);
        ph_len = get_product_header_len(0);
#elif (dg_configCODE_LOCATION == NON_VOLATILE_IS_QSPI_FLASH)
        uint16_t flash_conf_len;

        flash_conf_len = get_flash_conf_len(PRIMARY_PRODUCT_HEADER_BASE);
        crc_offset = sizeof(product_header_fixed_t) + flash_conf_len;
        ph_len = get_product_header_len(flash_conf_len);
#endif

        // Read and calculate Primary Product Header's CRC
        pph_crc = crc16_read(PRIMARY_PRODUCT_HEADER_BASE, crc_offset);
        pph_crc_calc = crc16_calc(PRIMARY_PRODUCT_HEADER_BASE, crc_offset);

        // If the Primary Product Header is corrupted, repair it by copying the Backup Product Header
        // on it. Afterwards check again the CRC of the repaired Primary Product Header.
        while (pph_crc != pph_crc_calc) {
#if (dg_configCODE_LOCATION == NON_VOLATILE_IS_QSPI_FLASH)
                // Use the Flash config length of the Backup Product Header
                flash_conf_len = get_flash_conf_len(BACKUP_PRODUCT_HEADER_BASE);
                crc_offset = sizeof(product_header_fixed_t) + flash_conf_len;
                ph_len = get_product_header_len(flash_conf_len);
#endif
                restore_product_header(BACKUP_PRODUCT_HEADER_BASE,
                                       PRIMARY_PRODUCT_HEADER_BASE, ph_len);

                pph_crc = crc16_read(PRIMARY_PRODUCT_HEADER_BASE, crc_offset);
                pph_crc_calc = crc16_calc(PRIMARY_PRODUCT_HEADER_BASE, crc_offset);
                pph_repaired = true;
        }

        if (!pph_repaired) {
                uint16_t bph_crc = 0xFFFF;
                uint16_t bph_crc_calc = 0xF5F5;

                // Read and calculate Backup Product Header's CRC
                bph_crc = crc16_read(BACKUP_PRODUCT_HEADER_BASE, crc_offset);
                bph_crc_calc = crc16_calc(BACKUP_PRODUCT_HEADER_BASE, crc_offset);

                // If the Backup Product Header is corrupted or NOT equal to the CRC of the Primary
                // product header, repair it by copying the Primary Product Header on it. Afterwards
                // check again the CRC of the repaired Backup Product Header.
                while (bph_crc != bph_crc_calc || bph_crc != pph_crc) {
                        restore_product_header(PRIMARY_PRODUCT_HEADER_BASE,
                                               BACKUP_PRODUCT_HEADER_BASE, ph_len);

                        bph_crc = crc16_read(BACKUP_PRODUCT_HEADER_BASE, crc_offset);
                        bph_crc_calc = crc16_calc(BACKUP_PRODUCT_HEADER_BASE, crc_offset);
                }
        }

        ASSERT_WARNING(equalize_image_pointers(ph_len));
}

#endif /* dg_configUSE_SYS_BOOT */
