/**
 ****************************************************************************************
 *
 * @file ad_flash.c
 *
 * @brief Flash adapter implementation
 *
 * Copyright (C) 2015-2024 Renesas Electronics Corporation and/or its affiliates.
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

#if dg_configFLASH_ADAPTER

#include <string.h>

#include "ad_flash.h"
#include "qspi_automode.h"

#include "eflash_automode.h"

#ifdef OS_PRESENT
#include "osal.h"
#include "sys_power_mgr.h"
#endif

#include "hw_cache.h"
#include "hw_sys.h"

/**
 * Enable/Disable run-time checks for possible cache incoherence:
 *  - 1 to enable
 *  - 0 to disable
 */
#define DETECT_CACHE_INCOHERENCE_DANGER         0

#define FLASH_PAGE_SIZE   0x0100

/*
 * In case that a user intends to write data to a flash memory providing source data that resides
 * in another address of the same memory, then the use of a temporary buffer on the stack is required.
 * This is because read and write operations cannot be performed simultaneously due to hardware
 * limitations of the corresponding controller.
 */
#define ON_STACK_BUFFER_SIZE 16

__RETAINED static bool initialized;
#ifdef OS_PRESENT
__RETAINED static OS_MUTEX flash_mutex;
#endif
__RETAINED static uint32_t no_cache_flush_base;
__RETAINED static uint32_t no_cache_flush_end;

static bool get_automode_addr(uint32_t addr, const uint8_t **automode_addr);

__STATIC_INLINE bool is_flash_addr_cached(uint32_t addr)
{
        uint32_t cache_len = 0;
        uint32_t cache_base = 0;
        uint32_t cache_offset = 0;
        uint32_t zero_based_addr = 0;
        uint32_t region_size  = 0; // in Bytes
        uint32_t cached_upper_limit  = 0; // in Bytes

# if (dg_configCODE_LOCATION == NON_VOLATILE_IS_EMBEDDED_FLASH)
        if (hw_sys_get_memory_remapping() != HW_SYS_REMAP_ADDRESS_0_TO_EFLASH) {
                return false;
        }
# elif (dg_configCODE_LOCATION == NON_VOLATILE_IS_QSPI_FLASH)
        if (hw_sys_get_memory_remapping() != HW_SYS_REMAP_ADDRESS_0_TO_QSPI_FLASH) {
                return false;
        }
# endif /* dg_configCODE_LOCATION */
        /*
         * Cacheable area is N * 64KB
         *
         * N == 0 --> no caching, the iCache controller is then in bypass mode.
         */

# if (dg_configCODE_LOCATION == NON_VOLATILE_IS_EMBEDDED_FLASH)
        static const uint32 eflash_region_sizes[] = {
             512 * 1024,
             512 * 1024,
             512 * 1024,
             512 * 1024,
             512 * 1024,
             256 * 1024,
             128 * 1024,
             64 * 1024,
        };

        cache_len = hw_cache_get_eflash_cacheable_len() << 16;
        cache_base = hw_cache_eflash_get_region_base() << CACHE_CACHE_EFLASH_REG_EFLASH_REGION_BASE_Pos;
        cache_base -= MEMORY_EFLASH_BASE;
        cache_offset = (hw_cache_eflash_get_region_offset() << 2); //expressed in bytes
        zero_based_addr = addr - EFLASH_MEM1_VIRTUAL_BASE_ADDR;
        region_size  = eflash_region_sizes[hw_cache_eflash_get_region_size()];
# elif (dg_configCODE_LOCATION == NON_VOLATILE_IS_QSPI_FLASH)

        static const uint32 flash_region_sizes[] = {
             32 * 1024 * 1024,
             16 * 1024 * 1024,
             8 * 1024 * 1024,
             4 * 1024 * 1024,
             2 * 1024 * 1024,
             1 * 1024 * 1024,
             512 * 1024,
             256 * 1024,
        };

        cache_len = hw_cache_get_extflash_cacheable_len() << 16;
        cache_base = hw_cache_flash_get_region_base() << CACHE_CACHE_FLASH_REG_FLASH_REGION_BASE_Pos;
        cache_base -= MEMORY_QSPIF_BASE;
        cache_offset = (hw_cache_flash_get_region_offset() << 2); //expressed in bytes
        zero_based_addr = addr - QSPI_MEM1_VIRTUAL_BASE_ADDR;
        region_size = flash_region_sizes[hw_cache_flash_get_region_size()];
# endif /* dg_configCODE_LOCATION */

        if ((cache_offset + cache_len) >= region_size) {
                cached_upper_limit = cache_base + region_size;
        } else {
                cached_upper_limit = cache_base + cache_offset + cache_len;
        }

        return ((zero_based_addr >= cache_base + cache_offset) && (zero_based_addr < cached_upper_limit));
}

__STATIC_INLINE bool is_base_within_flushable_area(uint32_t base, uint32_t size)
{
        if ((base >= no_cache_flush_base) && ((base + size) <= no_cache_flush_end)) {
                return false;
        }

        return true;
}

void ad_flash_init(void)
{
        if (!initialized) {
                initialized = true;
#ifdef OS_PRESENT
                OS_MUTEX_CREATE(flash_mutex);
                OS_ASSERT(flash_mutex);
#endif

                ad_flash_lock();
                no_cache_flush_base = AD_FLASH_ALWAYS_FLUSH_CACHE;
                no_cache_flush_end = 0;

                ad_flash_unlock();
        }
}

size_t ad_flash_read(uint32_t addr, uint8_t *buf, size_t len)
{
        size_t read = 0;

# if (dg_configCODE_LOCATION == NON_VOLATILE_IS_NONE)
        ASSERT_WARNING(IS_SYSRAM_S_ADDRESS(MEMORY_SYSRAM_S_BASE + buf));
# else
        ASSERT_WARNING(IS_SYSRAM_S_ADDRESS(buf));
# endif

        bool addr_is_in_eflash = eflash_automode_is_valid_virtual_address_range(addr, len);

#if DETECT_CACHE_INCOHERENCE_DANGER
        /* An address within the cacheable area and a read space excluded
         * from being flushed (see ad_flash_skip_cache_flushing()) create a condition
         * for potential cache incoherence.
         */
        OS_ASSERT(!is_flash_addr_cached(addr) || is_base_within_flushable_area(addr, len));
#endif /* DETECT_CACHE_INCOHERENCE_DANGER */

        ad_flash_lock();
        if (addr_is_in_eflash) {
                read = eflash_automode_read(addr, buf, len);
#if (dg_configQSPI_AUTOMODE_ENABLE == 1)
        } else {
                read = qspi_automode_read(addr, buf, len);
#endif
        }
        ad_flash_unlock();
        return read;
}


# if (dg_configCODE_LOCATION == NON_VOLATILE_IS_EMBEDDED_FLASH) || (dg_configCODE_LOCATION == NON_VOLATILE_IS_QSPI_FLASH)
__STATIC_INLINE bool is_src_dst_conflict_address(const void *dst, const void *src)
{
        uint32_t dst_phys = hw_sys_get_physical_addr((uint32_t)dst);
        uint32_t src_phys = hw_sys_get_physical_addr((uint32_t)src);

        if (IS_QSPIF_S_ADDRESS(dst_phys) && IS_QSPIF_S_ADDRESS(src_phys)) {
                return true;
        }

        if (IS_EFLASH_S_ADDRESS(dst_phys) && IS_EFLASH_S_ADDRESS(src_phys)) {
                return true;
        }

        return false;
}
# endif /* (dg_configCODE_LOCATION == NON_VOLATILE_IS_EMBEDDED_FLASH) || (dg_configCODE_LOCATION == NON_VOLATILE_IS_QSPI_FLASH) */




/* typedef's for the two different write internal APIs */
typedef size_t (* fp_write_via_ram)(uint32_t _addr, const uint8_t *_buf, size_t _size);
typedef uint32_t (* fp_write_direct)(uint32_t addr, const uint8_t *buf, uint32_t size);

static fp_write_direct get_write_direct_func(uint32_t addr);

#if (dg_configCODE_LOCATION == NON_VOLATILE_IS_EMBEDDED_FLASH) || (dg_configCODE_LOCATION == NON_VOLATILE_IS_QSPI_FLASH)
static size_t ad_flash_write_from_flash_with_conflict(uint32_t addr, const uint8_t *xip_buf, size_t size)
{
        size_t offset = 0;
        uint8_t buf[ON_STACK_BUFFER_SIZE];
        fp_write_direct direct_func;

        direct_func = get_write_direct_func(addr);
        xip_buf = (const uint8_t *) hw_sys_get_physical_addr((uint32_t) xip_buf);

        /*
         * If the source data resides in XiP mapped memory, it needs to be copied in a temporary
         * buffer on stack, so that it can be accessed while the memory controller performs the
         * write operations.
         */
        while (offset < size) {
                size_t written;
                size_t chunk = sizeof(buf) > (size - offset) ? (size - offset) : sizeof(buf);
                memcpy(buf, xip_buf + offset, chunk);
                written = direct_func(addr + offset, buf, chunk);
                offset += written;
        }
        return offset;
}
#endif /* (dg_configCODE_LOCATION == NON_VOLATILE_IS_EMBEDDED_FLASH) || (dg_configCODE_LOCATION == NON_VOLATILE_IS_QSPI_FLASH) */

static bool should_flush(uint32_t addr, size_t size)
{
        return is_flash_addr_cached(addr & ~(AD_FLASH_GET_SECTOR_SIZE(addr) - 1))
                                    && is_base_within_flushable_area(addr, size);
}

static void flush_icache(uint32_t addr, size_t size)
{
        if (should_flush(addr, size)) {
                hw_cache_flush();
        }
}

static bool write_src_dst_conflicts(uint32_t addr, const uint8_t *buf, fp_write_via_ram *fp)
{
        const uint8_t *automode_addr;
        if (!get_automode_addr(addr, &automode_addr)) {
                ASSERT_WARNING(0);
                return false;
        }
#if (dg_configCODE_LOCATION == NON_VOLATILE_IS_EMBEDDED_FLASH) || (dg_configCODE_LOCATION == NON_VOLATILE_IS_QSPI_FLASH)
        if (is_src_dst_conflict_address((const void *)automode_addr, buf)) {
                *fp = ad_flash_write_from_flash_with_conflict;
                return true;
        }
#endif
        return false;
}

static fp_write_direct get_write_direct_func(uint32_t addr)
{
        if (eflash_automode_is_valid_virtual_addr(addr)) {
                return eflash_automode_write_page;
# if (dg_configQSPI_AUTOMODE_ENABLE == 1)
        } else if (qspi_automode_is_valid_virtual_addr(addr)) {
                return qspi_automode_write_flash_page;
# endif
        } else {
                ASSERT_WARNING(0);
        }

        return NULL;
}

size_t ad_flash_write(uint32_t addr, const uint8_t *buf, size_t size)
{
        size_t written;
        size_t offset = 0;
        bool buf_conflicts_with_xip;
        union {
                fp_write_via_ram via_ram;
                fp_write_direct direct;
        } write_api;

        ASSERT_WARNING(buf);

        /* assume that buf lies either completely in a xSPI or completely outside any xSPI device */
        buf_conflicts_with_xip = write_src_dst_conflicts(addr, buf, &write_api.via_ram);
        if (!buf_conflicts_with_xip) {
                write_api.direct = get_write_direct_func(addr);

                if (NULL == write_api.direct) {
                        return 0;
                }
        }

        ad_flash_lock();

        while (offset < size) {
                /*
                 * If buf conflicts with the current XIP flash memory, copy source data to RAM first.
                 */
                if (buf_conflicts_with_xip) {
                        written = write_api.via_ram(addr + offset, buf + offset, size - offset);

                } else {
                        /*
                         * Try to write everything, qspi_automode/oqspi_automode will reduce
                         * this value to accommodate page boundary and maximum write size limitation
                         */
                        written = write_api.direct(addr + offset, buf + offset, size - offset);
                }
                offset += written;
        }

        flush_icache(addr, size);

        ad_flash_unlock();

        return size;
}

/* typedef for the internal API for erasing a sector */
typedef bool (* fp_erase_sector)(uint32_t addr);

static fp_erase_sector get_erase_sector_func(uint32_t addr)
{
        if (eflash_automode_is_valid_virtual_addr(addr)) {
                return eflash_automode_erase_sector;
        }
# if (dg_configQSPI_AUTOMODE_ENABLE == 1)
        else if (qspi_automode_is_valid_virtual_addr(addr)) {
                return qspi_automode_erase_flash_sector;
        }
# endif
        return NULL;

        return NULL;
}

bool ad_flash_erase_region(uint32_t addr, size_t size)
{
        uint32_t flash_sector_address = addr & ~(AD_FLASH_GET_SECTOR_SIZE(addr) - 1);
        fp_erase_sector erase_sector = get_erase_sector_func(addr);
        if (NULL == erase_sector) {
                return false;
        }

        ad_flash_lock();

        while (flash_sector_address < addr + size) {
                erase_sector(flash_sector_address);
                flash_sector_address += AD_FLASH_GET_SECTOR_SIZE(addr);
        }

        flush_icache(addr, size);

        ad_flash_unlock();

        return true;
}


bool ad_flash_chip_erase_by_addr(uint32_t addr)
{
        if (addr == EFLASH_MEM1_VIRTUAL_BASE_ADDR) {
                ad_flash_lock();
                eflash_automode_erase_chip();
                ad_flash_unlock();

                return true;
        }
# if dg_configQSPI_AUTOMODE_ENABLE
        else if (addr == QSPI_MEM1_VIRTUAL_BASE_ADDR) {
                ad_flash_lock();
                qspi_automode_erase_chip(addr);
                ad_flash_unlock();

                return true;
        }
# endif /* dg_configQSPI_AUTOMODE_ENABLE */


        /* Wrong start address */
        return false;
}

static bool get_automode_addr(uint32_t addr, const uint8_t **automode_addr)
{
        if (eflash_automode_is_valid_virtual_addr(addr)) {
                *automode_addr = eflash_automode_get_physical_addr(addr);
                return true;
# if (dg_configQSPI_AUTOMODE_ENABLE == 1)
        } else if (qspi_automode_is_valid_virtual_addr(addr)) {
                *automode_addr = qspi_automode_get_physical_addr(addr);
                return true;
# endif
        }

        return false;
}

int ad_flash_update_possible(uint32_t addr, const uint8_t *data_to_write, size_t size)
{
        int i;
        int same;
        const uint8_t *old;

        if (!get_automode_addr(addr, &old)) {
                ASSERT_WARNING(0);
                return -1;
        }

        ASSERT_WARNING(data_to_write);

        /* Check if new data is same as old one, in which case no write will be needed */
        for (i = 0; i < size && old[i] == data_to_write[i]; ++i) {
        }

        /* This much did not change */
        same = i;

        /* Check if new data can be stored by clearing bits only */
        for (; i < size ; ++i) {
                if ((old[i] & data_to_write[i]) != data_to_write[i])
                        /*
                         * Found byte that needs to have at least one bit set and it was cleared,
                         * erase will be needed.
                         */
                        return -1;
        }
        return same;
}

size_t ad_flash_erase_size(uint32_t addr)
{
        return AD_FLASH_GET_SECTOR_SIZE(addr);
}

void ad_flash_lock(void)
{
#ifdef OS_PRESENT
        OS_MUTEX_GET(flash_mutex, OS_MUTEX_FOREVER);
#endif
}

void ad_flash_unlock(void)
{
#ifdef OS_PRESENT
        OS_MUTEX_PUT(flash_mutex);
#endif
}

void ad_flash_skip_cache_flushing(uint32_t base, uint32_t size)
{
        /* cppcheck-suppress unsignedPositive */
        ASSERT_WARNING((base == AD_FLASH_ALWAYS_FLUSH_CACHE) || IS_REMAPPED_ADDRESS(base));

        no_cache_flush_base = base;
        no_cache_flush_end = base + size;
        if (no_cache_flush_end < no_cache_flush_base)
                no_cache_flush_end = 0;
}

#ifdef OS_PRESENT
ADAPTER_INIT(ad_flash_adapter, ad_flash_init)
#endif

#endif
