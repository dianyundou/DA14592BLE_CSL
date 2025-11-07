/**
 ****************************************************************************************
 *
 * @file asym_suota_utils.c
 *
 * @brief Asymmetric SUOTA utilities functions
 *
 * Copyright (C) 2024 Renesas Electronics Corporation and/or its affiliates.
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

#if (dg_configSUOTA_ASYMMETRIC == 1)

#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include "asym_suota_utils.h"
#include "osal.h"
#include "ad_nvms.h"
#include "sdk_crc16.h"
#include "hw_cpm.h"
#include "hw_cache.h"
#include "suota.h"
#if (dg_configSUOTA_SUPPORT == 1) && (ASYM_SUOTA_UTILS_IMG_CRC_CHECK_EN == 1)
#include "dlg_suota.h"
#endif

/*
 * MACRO DEFINITIONS
 *****************************************************************************************
 */
#define PH_BASE_PRIM                            ( PRIMARY_PRODUCT_HEADER_BASE - NVMS_PRODUCT_HEADER_PART_START )
#define PH_BASE_SEC                             ( BACKUP_PRODUCT_HEADER_BASE  - NVMS_PRODUCT_HEADER_PART_START )
#define PH_WRITE_RETRY_TRIES_NUM                ( 5 )

#ifndef DEBUG_PRINT
#define DEBUG_PRINT(_type, _format, ...)
#endif

#ifndef ASUOTA_UTILS_PRINTF
#define ASUOTA_UTILS_PRINTF(_type, ...)         DEBUG_PRINT(PRINT_ ## _type, "ASUOTA_UTILS: " __VA_ARGS__)
#endif

/*
 * TYPE DEFINITIONS
 *****************************************************************************************
 */
typedef __PACKED_STRUCT {
        uint8_t  identifier[2];       /**< Identifier (Pp) */
        uint32_t fw_img_active;       /**< Active Firmware image address */
        uint32_t fw_img_update;       /**< Update Firmware image address (if available,
                                             otherwise equal to Active Firmware image address) */
        uint32_t flash_burstcmda;     /**< BURSTCMDA register */
        uint32_t flash_burstcmdb;     /**< BURSTCMDB register */
        uint16_t type_flash_conf;     /**< Type of Flash Configuration */
        uint16_t flash_conf_len;      /**< Length of flash configuration  */
} product_header_fixed_t;

/*
 * FORWARD DECLARATIONS
 *****************************************************************************************
 */
void cold_reset_hook(void) __attribute__((section("text_reset")));
static uint32_t get_ivt_addr(void);
static bool is_firmware_part(nvms_partition_id_t id);
static nvms_partition_id_t get_executed_fw_part_id(void);
static uint32_t abs_to_rel_address(uint32_t addr);
static ASUOTA_UTILS_ERR get_prod_header(uint32_t offset, const product_header_fixed_t **prod_header, nvms_t *nvms);
static ASUOTA_UTILS_ERR get_img_header(nvms_partition_id_t id, const suota_1_1_image_header_da1469x_t **img_hdr, nvms_t *nvms);
#if (dg_configSUOTA_SUPPORT != 1) && (ASYM_SUOTA_UTILS_IMG_CRC_CHECK_EN == 1)
static uint32_t suota_update_crc(uint32_t crc, const uint8_t *data, size_t len);
#endif
static ASUOTA_UTILS_ERR verify_img_crc(const suota_1_1_image_header_da1469x_t *img_hdr, nvms_t nvms);
static bool safe_product_header_write(nvms_t nvms, size_t offset, const uint8_t *ph, uint16_t ph_size);
static ASUOTA_UTILS_ERR set_active_img_ptr(uint32_t fw_partition_address);
static ASUOTA_UTILS_ERR set_boot_mode(nvms_partition_id_t id, bool reboot);

/*
 * GLOBAL VARIABLES
 *****************************************************************************************
 */
#if (dg_configSUOTA_SUPPORT != 1) && (ASYM_SUOTA_UTILS_IMG_CRC_CHECK_EN == 1)
static const uint32_t crc32_tab[] = {
        0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419, 0x706af48f,
        0xe963a535, 0x9e6495a3, 0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988,
        0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91, 0x1db71064, 0x6ab020f2,
        0xf3b97148, 0x84be41de, 0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,
        0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec, 0x14015c4f, 0x63066cd9,
        0xfa0f3d63, 0x8d080df5, 0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172,
        0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b, 0x35b5a8fa, 0x42b2986c,
        0xdbbbc9d6, 0xacbcf940, 0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59,
        0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116, 0x21b4f4b5, 0x56b3c423,
        0xcfba9599, 0xb8bda50f, 0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924,
        0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d, 0x76dc4190, 0x01db7106,
        0x98d220bc, 0xefd5102a, 0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
        0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818, 0x7f6a0dbb, 0x086d3d2d,
        0x91646c97, 0xe6635c01, 0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e,
        0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457, 0x65b0d9c6, 0x12b7e950,
        0x8bbeb8ea, 0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,
        0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2, 0x4adfa541, 0x3dd895d7,
        0xa4d1c46d, 0xd3d6f4fb, 0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0,
        0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9, 0x5005713c, 0x270241aa,
        0xbe0b1010, 0xc90c2086, 0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
        0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17, 0x2eb40d81,
        0xb7bd5c3b, 0xc0ba6cad, 0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a,
        0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683, 0xe3630b12, 0x94643b84,
        0x0d6d6a3e, 0x7a6a5aa8, 0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
        0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe, 0xf762575d, 0x806567cb,
        0x196c3671, 0x6e6b06e7, 0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc,
        0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5, 0xd6d6a3e8, 0xa1d1937e,
        0x38d8c2c4, 0x4fdff252, 0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,
        0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60, 0xdf60efc3, 0xa867df55,
        0x316e8eef, 0x4669be79, 0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236,
        0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f, 0xc5ba3bbe, 0xb2bd0b28,
        0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d,
        0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a, 0x9c0906a9, 0xeb0e363f,
        0x72076785, 0x05005713, 0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38,
        0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21, 0x86d3d2d4, 0xf1d4e242,
        0x68ddb3f8, 0x1fda836e, 0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
        0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c, 0x8f659eff, 0xf862ae69,
        0x616bffd3, 0x166ccf45, 0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2,
        0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db, 0xaed16a4a, 0xd9d65adc,
        0x40df0b66, 0x37d83bf0, 0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
        0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6, 0xbad03605, 0xcdd70693,
        0x54de5729, 0x23d967bf, 0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94,
        0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d
};
#endif /* dg_configSUOTA_SUPPORT && ASYM_SUOTA_UTILS_IMG_CRC_CHECK_EN */

#ifndef RELEASE_BUILD
__RETAINED_UNINIT uint32_t ph_fw_img_update;
#endif

/*
 * API FUNCTIONS
 *****************************************************************************************
 */
void cold_reset_hook(void)
{
        HW_CACHE_EFLASH_REGION_SZ size;
        eflash_region_base_t base = hw_cache_eflash_get_region_base();
        uint32_t ivt_base = base - HW_CACHE_EFLASH_DEFAULT_REGION_BASE;
        switch (ivt_base) {
        case 0:  size = HW_CACHE_EFLASH_REGION_SZ_256KB; break;
        case 1:  size = HW_CACHE_EFLASH_REGION_SZ_64KB; break;
        case 2:  size = HW_CACHE_EFLASH_REGION_SZ_128KB; break;
        case 3:  size = HW_CACHE_EFLASH_REGION_SZ_64KB; break;
        default: return;
        }

        if (hw_cache_eflash_get_region_size() != size) {
                hw_cache_eflash_set_region_size(size);

                SWRESET;

                while (1);
        }

#ifndef RELEASE_BUILD
        product_header_fixed_t *ph = (product_header_fixed_t *)(PRIMARY_PRODUCT_HEADER_BASE + MEMORY_EFLASH_S_BASE);
        ph_fw_img_update = ph->fw_img_update;
#endif
}

ASUOTA_UTILS_ERR asym_suota_utils_handle_boot_from_fw_location(void)
{
        nvms_partition_id_t part_id = get_executed_fw_part_id();

        if (!is_firmware_part(part_id)) {
                uint32_t abs_img_addr, abs_dst_addr;
                nvms_t img_nvms, dst_nvms;
                size_t img_len, ivt_ofs;
                const suota_1_1_image_header_da1469x_t *img_hdr;
                static const size_t img_hdr_len = sizeof(suota_1_1_image_header_da1469x_t);

                ASUOTA_UTILS_PRINTF(INFO, "SUOTA app booted from FW location. Copy and reboot\r\n");

                ASUOTA_UTILS_ERR ret = get_img_header(part_id, &img_hdr, &img_nvms);
                if (ASUOTA_UTILS_ERR_NO_ERROR != ret) {
                        return ret;
                }

                dst_nvms = ad_nvms_open(NVMS_FIRMWARE_PART);
                if (dst_nvms == NULL) {
                        ASUOTA_UTILS_PRINTF(ERROR, "Could not open SUOTA partition\r\n");
                        return ASUOTA_UTILS_ERR_PART_OPEN;
                }

                ivt_ofs = img_hdr->pointer_to_ivt;
                img_len = ivt_ofs + img_hdr->size;

                if (img_len != ad_nvms_get_pointer(img_nvms, 0, img_len, (const void**)&abs_img_addr)) {
                        ASUOTA_UTILS_PRINTF(ERROR, "Could not read image\r\n");
                        return ASUOTA_UTILS_ERR_IMG_READ;
                }

                /* Erase old image header */
                if (!ad_nvms_erase_region(dst_nvms, 0, img_hdr_len)) {
                        ASUOTA_UTILS_PRINTF(ERROR, "Could not erase image header\r\n");
                        return ASUOTA_UTILS_ERR_IMG_HDR_ERASE;
                }

                /* Copy image to running location and reboot */
                if (img_len - ivt_ofs != ad_nvms_write(dst_nvms, ivt_ofs, (uint8_t *)(abs_img_addr + ivt_ofs), img_len - ivt_ofs)) {
                        ASUOTA_UTILS_PRINTF(ERROR, "Could not copy image\r\n");
                        return ASUOTA_UTILS_ERR_IMG_WRITE;
                }

                if (img_len != ad_nvms_get_pointer(dst_nvms, 0, img_len, (const void**)&abs_dst_addr)) {
                        ASUOTA_UTILS_PRINTF(ERROR, "Could not read dst image\r\n");
                        return ASUOTA_UTILS_ERR_IMG_READ;
                }

                /* Verify image was written successfully */
                if (memcmp((void *)(abs_dst_addr + ivt_ofs), (void *)(abs_img_addr + ivt_ofs), img_len - ivt_ofs)) {
                        ASUOTA_UTILS_PRINTF(ERROR, "Image comparison failed\r\n");
                        return ASUOTA_UTILS_ERR_IMG_VERIFY;
                }

                /* Image successfully written, write image header as well */
                if (ivt_ofs != ad_nvms_write(dst_nvms, 0, (uint8_t *)abs_img_addr, ivt_ofs)) {
                        ASUOTA_UTILS_PRINTF(ERROR, "Could not copy image header\r\n");
                        return ASUOTA_UTILS_ERR_IMG_HDR_WRITE;
                }
                /* Verify image header was written successfully */
                if (memcmp((void *)abs_dst_addr, (void *)abs_img_addr, ivt_ofs)) {
                        ASUOTA_UTILS_PRINTF(ERROR, "Image header comparison failed\r\n");
                        return ASUOTA_UTILS_ERR_IMG_HDR_VERIFY;
                }

                ret = set_active_img_ptr(abs_dst_addr);
                if (ASUOTA_UTILS_ERR_NO_ERROR != ret) {
                        ASUOTA_UTILS_PRINTF(ERROR, "Could not set active image pointer\r\n");
                        return ret;
                }

                ASUOTA_UTILS_PRINTF(INFO, "Copy completed! Reboot!\r\n");
                hw_cpm_reboot_system();
        }

        return ASUOTA_UTILS_ERR_NO_ERROR;
}

ASUOTA_UTILS_ERR asym_suota_utils_is_fw_valid(void)
{
        ASUOTA_UTILS_ERR ret;
        const suota_1_1_image_header_da1469x_t *fw_hdr, *img_hdr;
        nvms_t fw_nvms, img_nvms;

        ret = get_img_header(NVMS_FW_EXEC_PART, &fw_hdr, &fw_nvms);
        if (ASUOTA_UTILS_ERR_NO_ERROR != ret) {
                return ret;
        }

        ret = verify_img_crc(fw_hdr, fw_nvms);
        if (ASUOTA_UTILS_ERR_NO_ERROR != ret) {
                return ret;
        }

        /* Check against asym SUOTA FW */
        ret = get_img_header(NVMS_FIRMWARE_PART, &img_hdr, &img_nvms);
        if (ASUOTA_UTILS_ERR_NO_ERROR != ret) {
                return ret;
        }

        if (!memcmp(fw_hdr, img_hdr, sizeof(*fw_hdr))) {
                /* Same image headers, FW image has been overwritten */
                ASUOTA_UTILS_PRINTF(ERROR, "FW image is missing\r\n");
                return ASUOTA_UTILS_ERR_FW_IMG_MISSING;
        }

        /* Image headers different, assuming FW is valid */
        return ASUOTA_UTILS_ERR_NO_ERROR;
}

ASUOTA_UTILS_ERR asym_suota_utils_boot_fw_mode(bool reboot)
{
        ASUOTA_UTILS_ERR ret;

        ASUOTA_UTILS_PRINTF(INFO, "Set to boot in app mode%s\r\n", reboot ? " and reboot" : "");

        ret = asym_suota_utils_is_fw_valid();
        if (ASUOTA_UTILS_ERR_NO_ERROR != ret) {
                return ret;
        }

        return set_boot_mode(NVMS_FW_EXEC_PART, reboot);
}

ASUOTA_UTILS_ERR asym_suota_utils_boot_suota_mode(bool reboot)
{
        ASUOTA_UTILS_PRINTF(INFO, "Set to boot in SUOTA mode%s\r\n", reboot ? " and reboot" : "");
        return set_boot_mode(NVMS_FIRMWARE_PART, reboot);
}

const char *asym_suota_utils_get_fw_ver_str(void)
{
        static char version_str[16 + 1] = "";
        if (version_str[0] == '\0') {
                const suota_1_1_image_header_da1469x_t *img_hdr;

                ASUOTA_UTILS_ERR ret = get_img_header(get_executed_fw_part_id(), &img_hdr, NULL);
                if (ASUOTA_UTILS_ERR_NO_ERROR != ret) {
                        return "";
                }

                memcpy(version_str, img_hdr->version_string, sizeof(version_str));
                version_str[sizeof(version_str) - 1] = '\0';
        }

        return version_str;
}

const char *asym_suota_utils_get_fw_addr_str(void)
{
        static char str[] = "0x00000000";
        static const char num[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };
        uint32_t addr;

        ASUOTA_UTILS_ERR ret = get_img_header(get_executed_fw_part_id(), (const suota_1_1_image_header_da1469x_t **)&addr, NULL);
        if (ASUOTA_UTILS_ERR_NO_ERROR == ret) {
                for (int i = 9; i > 1; i--) {
                        str[i] = num[addr & 0xF];
                        addr >>= 4;
                }
        }
        return str;
}

/*
 * STATIC FUNCTIONS
 *****************************************************************************************
 */
static uint32_t get_ivt_addr(void)
{
        HW_SYS_REMAP_ADDRESS_0 remap_addr = hw_sys_get_memory_remapping();
        if (HW_SYS_REMAP_ADDRESS_0_TO_EFLASH == remap_addr) {
                uint32_t flash_base = hw_cache_eflash_get_region_base();
                uint32_t flash_offset = hw_cache_eflash_get_region_offset();
                return ((flash_base << 16) + (flash_offset << 2)) - MEMORY_EFLASH_BASE;
        } else if (HW_SYS_REMAP_ADDRESS_0_TO_QSPI_FLASH == remap_addr) {
                uint32_t flash_base = hw_cache_flash_get_region_base();
                uint32_t flash_offset = hw_cache_flash_get_region_offset();
                return ((flash_base << 16) + (flash_offset << 2)) - MEMORY_QSPIF_BASE;
        }
        return 0;
}

static bool is_firmware_part(nvms_partition_id_t id)
{
        if (id == NVMS_FIRMWARE_PART) {
                return true;
        }
        return false;
}

static nvms_partition_id_t get_executed_fw_part_id(void)
{
        uint32_t ivt_addr = get_ivt_addr();

        size_t parts = ad_nvms_get_partition_count();
        partition_entry_t info;

        for (size_t i = 0; i < parts; ++i) {
                if (ad_nvms_get_partition_info(i, &info)) {
                        if (WITHIN_RANGE(ivt_addr, info.start_address, info.start_address + info.size)) {
                                return info.type;
                        }
                }
        }
        return 0;
}

static uint32_t abs_to_rel_address(uint32_t addr)
{
        if (IS_EFLASH_ADDRESS(addr)) {
                return addr - MEMORY_EFLASH_BASE;
        }

        if (IS_EFLASH_S_ADDRESS(addr)) {
                return addr - MEMORY_EFLASH_S_BASE;
        }

        if (IS_QSPIF_ADDRESS(addr)) {
                return addr - MEMORY_QSPIF_BASE;
        }

        if (IS_QSPIF_S_ADDRESS(addr)) {
                return addr - MEMORY_QSPIF_S_BASE;
        }

        return addr;
}

static ASUOTA_UTILS_ERR get_prod_header(uint32_t offset, const product_header_fixed_t **prod_header, nvms_t *nvms)
{
        nvms_t ph_nvms;
        product_header_fixed_t *ph;
        static const size_t ph_len = sizeof(product_header_fixed_t);

        /* Open product header partition */
        ph_nvms = ad_nvms_open(NVMS_PRODUCT_HEADER_PART);
        if (ph_nvms == NULL) {
                ASUOTA_UTILS_PRINTF(ERROR, "Could not open product header partition\r\n");
                return ASUOTA_UTILS_ERR_PART_OPEN;
        }

        /* Read flash configuration section size */
        if (ph_len != ad_nvms_get_pointer(ph_nvms, offset, ph_len, (const void**)&ph)) {
                ASUOTA_UTILS_PRINTF(ERROR, "Could not read product header\r\n");
                return ASUOTA_UTILS_ERR_PROD_HDR_READ;
        }

        /* Verify PH integrity. PH integrity is normally guaranteed by the sys_boot */
        if ((ph->identifier[0] != SUOTA_1_1_PRODUCT_DA1469x_HEADER_SIGNATURE_B1) ||
                (ph->identifier[1] != SUOTA_1_1_PRODUCT_DA1469x_HEADER_SIGNATURE_B2)) {
                ASUOTA_UTILS_PRINTF(ERROR, "Product header signature does not match\r\n");
                return ASUOTA_UTILS_ERR_PROD_HDR_SIGNATURE;
        }

        *prod_header = ph;
        if (nvms != NULL) {
                *nvms = ph_nvms;
        }
        return ASUOTA_UTILS_ERR_NO_ERROR;
}

static ASUOTA_UTILS_ERR get_img_header(nvms_partition_id_t id, const suota_1_1_image_header_da1469x_t **img_hdr, nvms_t *nvms)
{
        nvms_t img_nvms;
        suota_1_1_image_header_da1469x_t *hdr;
        static const size_t hdr_len = sizeof(suota_1_1_image_header_da1469x_t);

        img_nvms = ad_nvms_open(id);
        if (img_nvms == NULL) {
                ASUOTA_UTILS_PRINTF(ERROR, "Could not open partition\r\n");
                return ASUOTA_UTILS_ERR_PART_OPEN;
        }

        if (hdr_len != ad_nvms_get_pointer(img_nvms, 0, hdr_len, (const void**)&hdr)) {
                ASUOTA_UTILS_PRINTF(ERROR, "Could not read image header\r\n");
                return ASUOTA_UTILS_ERR_IMG_HDR_READ;
        }

        if ((hdr->image_identifier[0] != SUOTA_1_1_IMAGE_DA1469x_HEADER_SIGNATURE_B1) ||
                (hdr->image_identifier[1] != SUOTA_1_1_IMAGE_DA1469x_HEADER_SIGNATURE_B2)) {
                ASUOTA_UTILS_PRINTF(ERROR, "Image header signature does not match\r\n");
                return ASUOTA_UTILS_ERR_IMG_HDR_SIGNATURE;
        }

        *img_hdr = hdr;
        if (nvms != NULL) {
                *nvms = img_nvms;
        }
        return ASUOTA_UTILS_ERR_NO_ERROR;
}

#if (dg_configSUOTA_SUPPORT != 1) && (ASYM_SUOTA_UTILS_IMG_CRC_CHECK_EN == 1)
static uint32_t suota_update_crc(uint32_t crc, const uint8_t *data, size_t len)
{
        while (len--) {
                crc = crc32_tab[(crc ^ *data++) & 0xff] ^ (crc >> 8);
        }
        return crc;
}
#endif /* dg_configSUOTA_SUPPORT && ASYM_SUOTA_UTILS_IMG_CRC_CHECK_EN */

static ASUOTA_UTILS_ERR verify_img_crc(const suota_1_1_image_header_da1469x_t *img_hdr, nvms_t nvms)
{
#if ASYM_SUOTA_UTILS_IMG_CRC_CHECK_EN
        if (img_hdr->crc != 0) {
                const uint8_t *abs_fw_addr;
                size_t fw_len, ivt_ofs;
                uint32_t crc32 = 0xFFFFFFFF;

                ivt_ofs = img_hdr->pointer_to_ivt;
                fw_len = img_hdr->size;

                if (fw_len != ad_nvms_get_pointer(nvms, ivt_ofs, fw_len, (const void**)&abs_fw_addr)) {
                        ASUOTA_UTILS_PRINTF(ERROR, "Could not read image\r\n");
                        return ASUOTA_UTILS_ERR_IMG_READ;
                }

                crc32 = suota_update_crc(crc32, abs_fw_addr, fw_len);
                crc32 ^= 0xFFFFFFFF;

                if (crc32 != img_hdr->crc) {
                        ASUOTA_UTILS_PRINTF(ERROR, "Image CRC does not match\r\n");
                        return ASUOTA_UTILS_ERR_IMG_CRC;
                }
        } else {
                ASUOTA_UTILS_PRINTF(WARNING, "Image CRC is not available\r\n");
        }
#endif

        return ASUOTA_UTILS_ERR_NO_ERROR;
}

static bool safe_product_header_write(nvms_t nvms, size_t offset, const uint8_t *ph, uint16_t ph_size)
{
        int retry_cnt;
        uint8_t *ph_read;

        for (retry_cnt = 0; retry_cnt < PH_WRITE_RETRY_TRIES_NUM; ++retry_cnt) {
                if (ad_nvms_write(nvms, offset, ph, ph_size) !=  ph_size) {
                        continue;
                }

                if (ad_nvms_get_pointer(nvms, offset, ph_size, (const void**)&ph_read) != ph_size) {
                        continue;
                }

                if (!memcmp(ph_read, ph, ph_size)) {
                        /* Product header was written successfully */
                        break;
                }
        }

        return retry_cnt < PH_WRITE_RETRY_TRIES_NUM;
}

static ASUOTA_UTILS_ERR set_active_img_ptr(uint32_t fw_partition_address)
{
        ASUOTA_UTILS_ERR ret;
        nvms_t ph_nvms;
        uint16_t ph_size;
        uint16_t crc16, *ph_crc;
        uint8_t *ph_buf = NULL;
        const product_header_fixed_t *ph;

        do {
                ret = get_prod_header(PH_BASE_PRIM, &ph, &ph_nvms);
                if (ASUOTA_UTILS_ERR_NO_ERROR != ret) {
                        break;
                }

                /* Buffer for product header: static-size data + dynamic-size data + CRC (2 bytes) */
                ph_size = sizeof(product_header_fixed_t) + ph->flash_conf_len + sizeof(*ph_crc);
                ph_buf = OS_MALLOC(ph_size);

                if (ph_buf == NULL) {
                        ret = ASUOTA_UTILS_ERR_ALLOC;
                        ASUOTA_UTILS_PRINTF(ERROR, "Could not allocate product header buffer\r\n");
                        break;
                }

                ph_crc = (uint16_t *)(&ph_buf[ph_size - 2]);

                /* Read whole product header */
                if (ad_nvms_read(ph_nvms, PH_BASE_PRIM, ph_buf, ph_size) != ph_size) {
                        ret = ASUOTA_UTILS_ERR_PROD_HDR_READ;
                        ASUOTA_UTILS_PRINTF(ERROR, "Could not read product header\r\n");
                        break;
                }

                /* Compute CRC16-CCITT */
                crc16 = crc16_calculate(ph_buf, ph_size - sizeof(*ph_crc));
                if (crc16 != *ph_crc) {
                        ret = ASUOTA_UTILS_ERR_PROD_HDR_CRC;
                        ASUOTA_UTILS_PRINTF(ERROR, "Product header CRC does not match\r\n");
                        break;
                }

                fw_partition_address = abs_to_rel_address(fw_partition_address);

                if (((product_header_fixed_t *)ph_buf)->fw_img_update == fw_partition_address) {
                        ASUOTA_UTILS_PRINTF(INFO, "Update image address is already set\r\n");
                        ret = ASUOTA_UTILS_ERR_NO_ERROR;
                        break;
                }

                ((product_header_fixed_t *)ph_buf)->fw_img_update = fw_partition_address;

                /* Compute CRC16-CCITT */
                crc16 = crc16_calculate(ph_buf, ph_size - sizeof(*ph_crc));
                *ph_crc = crc16;

                /* Update primary product header */
                if (!safe_product_header_write(ph_nvms, PH_BASE_PRIM, ph_buf, ph_size)) {
                        ret = ASUOTA_UTILS_ERR_PROD_HDR_WRITE_PRIM;
                        ASUOTA_UTILS_PRINTF(ERROR, "Could not write product header\r\n");
                        break;
                }

                /* Update backup product header */
                if (!safe_product_header_write(ph_nvms, PH_BASE_SEC, ph_buf, ph_size)) {
                        ret = ASUOTA_UTILS_ERR_PROD_HDR_WRITE_SEC;
                        ASUOTA_UTILS_PRINTF(ERROR, "Could not write backup product header\r\n");
                        break;
                }

                ret = ASUOTA_UTILS_ERR_NO_ERROR;
        } while (0);

        if (ph_buf != NULL) {
                OS_FREE(ph_buf);
        }
        return ret;
}

static ASUOTA_UTILS_ERR set_boot_mode(nvms_partition_id_t id, bool reboot)
{
        ASUOTA_UTILS_ERR ret;
        nvms_t img_nvms;
        const suota_1_1_image_header_da1469x_t *img_hdr;

        ret = get_img_header(id, &img_hdr, &img_nvms);
        if (ASUOTA_UTILS_ERR_NO_ERROR != ret) {
                return ret;
        }

        ret = verify_img_crc(img_hdr, img_nvms);
        if (ASUOTA_UTILS_ERR_NO_ERROR != ret) {
                return ret;
        }

        ret = set_active_img_ptr((uint32_t)img_hdr);
        if (ASUOTA_UTILS_ERR_NO_ERROR != ret) {
                ASUOTA_UTILS_PRINTF(ERROR, "Could not set active image pointer\r\n");
                return ret;
        }

        if (reboot) {
                ASUOTA_UTILS_PRINTF(INFO, "Reboot!\r\n");
                hw_cpm_reboot_system();
        }

        return ASUOTA_UTILS_ERR_NO_ERROR;
}

#endif /* dg_configSUOTA_ASYMMETRIC */
