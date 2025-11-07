/**
 ****************************************************************************************
 *
 * @file eflash_automode.c
 *
 * @brief Access EFLASH when running in auto mode
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


#include <string.h>
#if dg_configPMU_ADAPTER
#include "../adapters/src/ad_pmu_internal.h"
#elif dg_configUSE_HW_PMU
#include "hw_pmu.h"
#endif
#include "eflash_automode.h"

#define CS_ADDR_START                           (0x00000000)
#define CS_ADDR_END                             (0x000007BF)
#define KEYS_REVOCATION_ADDR_START              (0x000007C0)
#define KEYS_REVOCATION_ADDR_END                (0x000007FF)
#define USER_APP_KEYS_ADDR_START                (0x00000800)
#define USER_APP_KEYS_ADDR_END                  (0x00000BFF)
#define SIGNATURE_VALID_KEYS_ADDR_START         (0x00000C00)
#define SIGNATURE_VALID_KEYS_ADDR_END           (0x00000FFF)
#define CUSTOMER_APP_AREA_ADDR_START            (0x00002000)
#define CUSTOMER_APP_AREA_ADDR_END              (0x00005FFF)
#define INFO_PAGE_ADDR_START                    (0x00040000)
#define INFO_PAGE_ADDR_END                      (0x000407FF)

enum eflash_protect_bit {
        EFLASH_PROTECT_BIT_ERASE = 1 << 0,
        EFLASH_PROTECT_BIT_WRITE = 1 << 1,
};

struct eflash_protected_range {
        uint32_t start;
        uint32_t end;
        uint32_t reg_addr;
        uint32_t reg_mask;
        enum eflash_protect_bit protect_bits;
};

static const struct eflash_protected_range eflash_protected_ranges[] = {
        {
                .start = EFLASH_MEM1_VIRTUAL_BASE_ADDR + CS_ADDR_START,
                .end = EFLASH_MEM1_VIRTUAL_BASE_ADDR + CS_ADDR_END,
                .reg_addr = CRG_TOP_BASE + offsetof(CRG_TOP_Type, SECURE_BOOT_REG),
                .reg_mask = REG_MSK(CRG_TOP, SECURE_BOOT_REG, PROT_CONFIG_SCRIPT),
                .protect_bits = EFLASH_PROTECT_BIT_WRITE | EFLASH_PROTECT_BIT_ERASE,
        },
        {
                .start = EFLASH_MEM1_VIRTUAL_BASE_ADDR + KEYS_REVOCATION_ADDR_START,
                .end = EFLASH_MEM1_VIRTUAL_BASE_ADDR + KEYS_REVOCATION_ADDR_END,
                .reg_addr = CRG_TOP_BASE + offsetof(CRG_TOP_Type, SECURE_BOOT_REG),
                .reg_mask = REG_MSK(CRG_TOP, SECURE_BOOT_REG, PROT_CONFIG_SCRIPT),
                .protect_bits = EFLASH_PROTECT_BIT_ERASE,
        },
        {
                .start = EFLASH_MEM1_VIRTUAL_BASE_ADDR + USER_APP_KEYS_ADDR_START,
                .end = EFLASH_MEM1_VIRTUAL_BASE_ADDR + USER_APP_KEYS_ADDR_END,
                .reg_addr = CRG_TOP_BASE + offsetof(CRG_TOP_Type, SECURE_BOOT_REG),
                .reg_mask = REG_MSK(CRG_TOP, SECURE_BOOT_REG, PROT_APP_KEY),
                .protect_bits = EFLASH_PROTECT_BIT_ERASE,
        },
        {
                .start = EFLASH_MEM1_VIRTUAL_BASE_ADDR + SIGNATURE_VALID_KEYS_ADDR_START,
                .end = EFLASH_MEM1_VIRTUAL_BASE_ADDR + SIGNATURE_VALID_KEYS_ADDR_END,
                .reg_addr = CRG_TOP_BASE + offsetof(CRG_TOP_Type, SECURE_BOOT_REG),
                .reg_mask = REG_MSK(CRG_TOP, SECURE_BOOT_REG, PROT_VALID_KEY),
                .protect_bits = EFLASH_PROTECT_BIT_ERASE,
        },
        {
                .start = EFLASH_MEM1_VIRTUAL_BASE_ADDR + CUSTOMER_APP_AREA_ADDR_START,
                .end = EFLASH_MEM1_VIRTUAL_BASE_ADDR + CUSTOMER_APP_AREA_ADDR_END,
                .reg_addr = CRG_TOP_BASE + offsetof(CRG_TOP_Type, SECURE_BOOT_REG),
                .reg_mask = REG_MSK(CRG_TOP, SECURE_BOOT_REG, PROT_USER_APP_CODE),
                .protect_bits = EFLASH_PROTECT_BIT_WRITE | EFLASH_PROTECT_BIT_ERASE,
        },
        {
                .start = EFLASH_MEM1_VIRTUAL_BASE_ADDR + INFO_PAGE_ADDR_START,
                .end = EFLASH_MEM1_VIRTUAL_BASE_ADDR + INFO_PAGE_ADDR_END,
                .reg_addr = CRG_TOP_BASE + offsetof(CRG_TOP_Type, SECURE_BOOT_REG),
                .reg_mask = REG_MSK(CRG_TOP, SECURE_BOOT_REG, PROT_INFO_PAGE),
                .protect_bits = EFLASH_PROTECT_BIT_WRITE | EFLASH_PROTECT_BIT_ERASE,
        },
};

#if dg_configUSE_HW_PMU && !dg_configPMU_ADAPTER
struct vdd_settings {
        HW_PMU_VDD_VOLTAGE vdd_voltage;
        HW_PMU_VDCDC_VOLTAGE vdcdc_voltage;
};
static void request_max_vdd_voltage(struct vdd_settings *prev_settings)
{
        HW_PMU_VDD_RAIL_CONFIG vdd_rail_config;
        HW_PMU_VDCDC_RAIL_CONFIG vdcdc_rail_config;

        hw_pmu_get_vdd_active_config(&vdd_rail_config);
        hw_pmu_get_vdcdc_active_config(&vdcdc_rail_config);

        prev_settings->vdd_voltage = vdd_rail_config.voltage;
        prev_settings->vdcdc_voltage = vdcdc_rail_config.voltage;

        if (vdd_rail_config.voltage != HW_PMU_VDD_VOLTAGE_1V20) {
                HW_PMU_ERROR_CODE error_code;

                /* A VDCDC voltage of at least 1.4V is required. */
                error_code = hw_pmu_vdcdc_set_voltage(HW_PMU_VDCDC_VOLTAGE_1V40);
                ASSERT_WARNING(error_code == HW_PMU_ERROR_NOERROR);
                error_code = hw_pmu_vdd_set_voltage(HW_PMU_VDD_VOLTAGE_1V20);
                ASSERT_WARNING(error_code == HW_PMU_ERROR_NOERROR);
        }
}

static void release_max_vdd_voltage(struct vdd_settings *prev_settings)
{
        HW_PMU_ERROR_CODE error_code;

        error_code = hw_pmu_vdd_set_voltage(prev_settings->vdd_voltage);
        ASSERT_WARNING(error_code == HW_PMU_ERROR_NOERROR);
        error_code = hw_pmu_vdcdc_set_voltage(prev_settings->vdcdc_voltage);
        ASSERT_WARNING(error_code == HW_PMU_ERROR_NOERROR);
}
#endif

static bool check_for_protected_ranges(uint32_t addr, uint32_t len,
                                                        enum eflash_protect_bit protect_bits)
{
        int i;

        for (i = 0; i < ARRAY_LENGTH(eflash_protected_ranges); i++) {
                if (!(protect_bits & eflash_protected_ranges[i].protect_bits)) {
                        continue;
                }

                if (!RAW_GETF(eflash_protected_ranges[i].reg_addr,
                                                        eflash_protected_ranges[i].reg_mask)) {
                        continue;
                }

                if (addr <= eflash_protected_ranges[i].end &&
                                                (addr + len) > eflash_protected_ranges[i].start) {
                        return false;
                }
        }

        return true;
}

/*
 * Erase whole block address. To erase whole EFLASH memory, address must be between 0x40000-0x40800
 */
#define EFLASH_ERASE_WHOLE_BLOCK_ADDRESS        (MEMORY_EFLASH_SIZE - 0x04)

__RETAINED_CODE const void *eflash_automode_get_physical_addr(uint32_t virtual_addr)
{
        return (const void *) (virtual_addr - EFLASH_MEM1_VIRTUAL_BASE_ADDR + MEMORY_EFLASH_S_BASE);
}

__RETAINED_CODE bool eflash_automode_is_valid_virtual_address_range(uint32_t addr, uint32_t len)
{
        /* cppcheck-suppress unsignedPositive */
        if ((addr >= EFLASH_MEM1_VIRTUAL_BASE_ADDR) &&
            (addr + len - 1) < (EFLASH_MEM1_VIRTUAL_BASE_ADDR + MEMORY_EFLASH_SIZE)) {
                return true;
        }

        return false;
}

__RETAINED_CODE bool eflash_automode_is_valid_virtual_addr(uint32_t addr)
{
        /* cppcheck-suppress unsignedPositive */
        return WITHIN_RANGE(addr, EFLASH_MEM1_VIRTUAL_BASE_ADDR,
                            EFLASH_MEM1_VIRTUAL_BASE_ADDR + MEMORY_EFLASH_SIZE);
}

__ALWAYS_RETAINED_CODE void eflash_automode_wakeup(void)
{
        hw_fcu_wakeup();
}

__ALWAYS_RETAINED_CODE void eflash_automode_sleep(void)
{
        if (!hw_fcu_is_asleep()) {
                hw_fcu_sleep();
        }
}

uint32_t eflash_automode_read(uint32_t addr, uint8_t *buf, uint32_t len)
{
        uint32_t mask = REG_MSK(FCU, FLASH_CTRL_REG, FLASH_RPROT) |
                        REG_MSK(FCU, FLASH_CTRL_REG, FLASH_PROT)  |
                        REG_MSK(FCU, FLASH_CTRL_REG, PROG_RMIN);

        if (!len || !buf) {
                return 0;
        }

        if (!eflash_automode_is_valid_virtual_address_range(addr, len) ||
            hw_fcu_is_protected_against_actions(mask)) {
                ASSERT_WARNING(0);
                return 0;
        }

        hw_fcu_enable_read();
        memcpy(buf, eflash_automode_get_physical_addr(addr), len);

        return len;
}

__ALWAYS_RETAINED_CODE uint32_t eflash_automode_write_page(uint32_t addr, const uint8_t *buf, uint32_t len)
{
        uint32_t written = 0;
        uint8_t addr_parity = addr % sizeof(uint32_t);
#if dg_configUSE_HW_PMU && !dg_configPMU_ADAPTER
        struct vdd_settings vdd_settings;
#endif

        if (!len || !buf) {
                return 0;
        } else if (!eflash_automode_is_valid_virtual_address_range(addr, len)) {
                ASSERT_WARNING(0);
                return 0;
        }

        addr -= EFLASH_MEM1_VIRTUAL_BASE_ADDR;

        // Align to page boundary
        if ((addr % HW_FCU_FLASH_PAGE_SIZE_IN_BYTES) + len > HW_FCU_FLASH_PAGE_SIZE_IN_BYTES) {
                len = HW_FCU_FLASH_PAGE_SIZE_IN_BYTES - (addr % HW_FCU_FLASH_PAGE_SIZE_IN_BYTES);
        }

        if (!check_for_protected_ranges(addr, len, EFLASH_PROTECT_BIT_WRITE)) {
                return 0;
        }

#if dg_configPMU_ADAPTER
        ad_pmu_1v2_force_max_voltage_request();
#elif dg_configUSE_HW_PMU
        request_max_vdd_voltage(&vdd_settings);
#endif

        GLOBAL_INT_DISABLE();

        /* If address is not aligned to 32 bit word, read back word from flash and overwrite it with
         * buf, then write to flash again */
        if (addr_parity) {
                uint32_t flash_content;

                /* Align address to word */
                addr -= addr_parity;

                if (hw_fcu_read(addr, &flash_content, 1, NULL) != HW_FCU_ERROR_NONE) {
                        goto done;
                }

                while (written < len && addr_parity < sizeof(uint32_t)) {
                        ((uint8_t *) &flash_content)[addr_parity] = buf[written++];
                        addr_parity++;
                }

                /* Write data to flash */
                if (hw_fcu_write(&flash_content, addr, 1, NULL) != HW_FCU_ERROR_NONE) {
                        written = 0;
                        goto done;
                }

                addr = addr + sizeof(uint32_t);
        }

        /* Write word by word. Note that buf might not be aligned, so here is our own
         * implementation of write instead of hw_fcu_write() */
        if ((len - written) >= sizeof(uint32_t)) {
                uint32_t *ptr = (uint32_t *) (addr + MEMORY_EFLASH_S_BASE);

                if (hw_fcu_are_flash_operations_prohibited() ||
                                                hw_fcu_is_flash_write_protection_enabled()) {
                        goto done;
                }

                hw_fcu_enable_write();
                while (hw_fcu_is_write_in_progress());

                hw_fcu_enable_reset_delay();
                while ((len - written) >= sizeof(uint32_t)) {
                        *ptr = __UNALIGNED_UINT32_READ(&buf[written]);
                        while (hw_fcu_is_write_in_progress());
                        ptr++;
                        written += sizeof(uint32_t);
                }
                hw_fcu_disable_reset_delay();

                addr = ((uint32_t) ptr) - MEMORY_EFLASH_S_BASE;
        }

        /* Write remaining bytes */
        if (len - written > 0) {
                uint32_t flash_content;

                if (hw_fcu_read(addr, &flash_content, 1, NULL) != HW_FCU_ERROR_NONE) {
                        goto done;
                }

                memcpy(&flash_content, &buf[written], len - written);

                if (hw_fcu_write(&flash_content, addr, 1, NULL) != HW_FCU_ERROR_NONE) {
                        goto done;
                }

                written = len;
        }

done:
        hw_fcu_enable_read();
        GLOBAL_INT_RESTORE();

#if dg_configPMU_ADAPTER
        ad_pmu_1v2_force_max_voltage_release();
#elif dg_configUSE_HW_PMU
        release_max_vdd_voltage(&vdd_settings);
#endif

        return written;
}

__ALWAYS_RETAINED_CODE bool eflash_automode_erase_sector(uint32_t addr)
{
        HW_FCU_ERROR err;
#if dg_configUSE_HW_PMU && !dg_configPMU_ADAPTER
        struct vdd_settings vdd_settings;
#endif

        /* First align address to word and make it zero-based */
        addr &= ~0x000000003;
        addr -= EFLASH_MEM1_VIRTUAL_BASE_ADDR;

        if (!check_for_protected_ranges(addr & ~(HW_FCU_FLASH_PAGE_SIZE_IN_BYTES - 1),
                                HW_FCU_FLASH_PAGE_SIZE_IN_BYTES, EFLASH_PROTECT_BIT_ERASE)) {
                return false;
        }

#if dg_configPMU_ADAPTER
        ad_pmu_1v2_force_max_voltage_request();
#elif dg_configUSE_HW_PMU
        request_max_vdd_voltage(&vdd_settings);
#endif

        GLOBAL_INT_DISABLE();
        err = hw_fcu_erase_page(addr, NULL);
        hw_fcu_enable_read();
        GLOBAL_INT_RESTORE();

#if dg_configPMU_ADAPTER
        ad_pmu_1v2_force_max_voltage_release();
#elif dg_configUSE_HW_PMU
        release_max_vdd_voltage(&vdd_settings);
#endif

        return (err == HW_FCU_ERROR_NONE);
}

__RETAINED_CODE bool eflash_automode_erase_chip(void)
{
        HW_FCU_ERROR err;
        uint32_t addr;
#if dg_configUSE_HW_PMU && !dg_configPMU_ADAPTER
        struct vdd_settings vdd_settings;
#endif

        /* If info page is protected, do not erase it */
        if (check_for_protected_ranges(0, MEMORY_EFLASH_SIZE, EFLASH_PROTECT_BIT_ERASE)) {
                addr = EFLASH_ERASE_WHOLE_BLOCK_ADDRESS;
        } else if (check_for_protected_ranges(0, MEMORY_EFLASH_SIZE - HW_FCU_FLASH_PAGE_SIZE_IN_BYTES,
                                                                EFLASH_PROTECT_BIT_ERASE)) {
                addr = EFLASH_MEM1_VIRTUAL_BASE_ADDR;

                for (; addr < EFLASH_MEM1_VIRTUAL_BASE_ADDR + MEMORY_EFLASH_SIZE - HW_FCU_FLASH_PAGE_SIZE_IN_BYTES;
                                                        addr += HW_FCU_FLASH_PAGE_SIZE_IN_BYTES) {
                        eflash_automode_erase_sector(addr);
                }
                return true;
        } else {
                return false;
        }

#if dg_configPMU_ADAPTER
        ad_pmu_1v2_force_max_voltage_request();
#elif dg_configUSE_HW_PMU
        request_max_vdd_voltage(&vdd_settings);
#endif

        GLOBAL_INT_DISABLE();
        err = hw_fcu_erase_block(addr, NULL);
        hw_fcu_enable_read();
        GLOBAL_INT_RESTORE();

#if dg_configPMU_ADAPTER
        ad_pmu_1v2_force_max_voltage_release();
#elif dg_configUSE_HW_PMU
        release_max_vdd_voltage(&vdd_settings);
#endif

        return (err == HW_FCU_ERROR_NONE);
}

